//
//  AIDocument.m
//  Ampede
//
//  Created by Eric Ocean on Tue Jul 13 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "AIDocument.h"
#import "AIDocument-Ragel.h"
#import "AIDocument-Ragel1.h"
#import "AIDocument-Ragel2.h"
#import "AIDocument-Ragel3.h"
#import "AIDocument-Ragel4.h"


@implementation AIDocument

- init
{
	if ( self = [super init] ) {
		layers = [[NSMutableArray alloc] init];
	}
	return self;
}

- (void) dealloc;
{
	[layers release]; layers = nil;
	[fileData release]; fileData = nil;
	[super dealloc];
}

- (NSString *)windowNibName
{
    // Implement this to return a nib to load OR implement -makeWindowControllers to manually create your controllers.
    return @"PDFLayerViewer";
}

- (NSData *)
dataRepresentationOfType:(NSString *)type
{
    // Implement to provide a persistent data representation of your document OR remove this and implement the
	// file-wrapper or file path based save methods.
    return nil;
}

- (BOOL)
loadDataRepresentation:(NSData *)data
ofType:(NSString *)type
{
    // Implement to load a persistent data representation of your document OR remove this and implement the
	// file-wrapper or file path based load methods.
	fileData = [data retain];
	
	pdf = CGPDFDocumentCreateWithURL(   CFURLCreateWithFileSystemPath(	kCFAllocatorDefault,
																		(CFStringRef)[self fileName],
																		kCFURLPOSIXPathStyle,
																		NO  )   );
	
	NSLog(@"%@", (NSString *)CFCopyDescription( pdf ));
	catalog = CGPDFDocumentGetCatalog( pdf ); CFRetain( catalog );
	NSLog(@"count of catalog is %d", CGPDFDictionaryGetCount( catalog ));
	// CFShow( catalog ); // crashes
	page = CGPDFDocumentGetPage( pdf , 1 ); // AI documents only have one page
	// CFShow( page ); // crashes
	pageDict = CGPDFPageGetDictionary( page );
	NSLog(@"count of page dictionary is %d", CGPDFDictionaryGetCount( pageDict ));
	// CFShow( pageDict ); // crashes
	
	const char *keyName = "Contents";
	BOOL gotStream = NO;
	gotStream = CGPDFDictionaryGetStream(   pageDict,
											keyName,
											&stream );
	if ( gotStream )
	{
		CGPDFDataFormat format;
		// CFShow( stream ); // crashes
		streamDict = CGPDFStreamGetDictionary( stream );
		NSLog(@"count of stream dictionary is %d", CGPDFDictionaryGetCount( streamDict ));
		streamData = (NSData *)CGPDFStreamCopyData( stream, &format );
		NSLog(@"streamData length is %d", [streamData length]);
		// NSLog(@"\nstreamData is %@", [streamData description]);
	}
	else
		NSLog(@"didn't get a stream");
		
	// find first layer marker and save location
	int loc = [self locateLayerAfterByte:0];
	NSLog(@"the first byte of the first layer is at base + %d", loc);
	
	int loc2 = [self locateLayerAfterByte:(loc+6)] + loc + 6; // skip past the first /Layer
	NSLog(@"the first byte of the second layer is at base + %d", loc2);
	
	NSData *layer1 = [NSData dataWithBytes:([streamData bytes]+loc) length:(loc2-loc)];
	NSData *layer2 = [NSData dataWithBytes:([streamData bytes]+loc2) length:([streamData length]-loc2)];
	
//	NSLog(@"layer1 length is %d", [layer1 length]);
//	NSLog(@"\nlayer1 is %@", [layer1 description]);
//
//	NSLog(@"layer2 length is %d", [layer2 length]);
//	NSLog(@"\nlayer2 is %@", [layer2 description]);
	
	NSMutableData *layer1FileData = [NSMutableData data];
	[layer1FileData appendData:fileData];
	
	int contentObjectNum = [self contentObjectNumber];
	NSLog(@"page content number is: %d", contentObjectNum);
	
	NSData *endOfPDF = [fileData subdataWithRange:NSMakeRange( [fileData length] - 64, 64 )];
		// this gives us the last 64 bytes, enough to determine the location of the pdf's cross-reference table
	
	int original_startxref = [self startxrefFromData:endOfPDF];
	NSLog(@"original startxref number is: %d", original_startxref);
	
	NSData *xrefForPDF = [fileData subdataWithRange:NSMakeRange( original_startxref, [fileData length] - original_startxref )];
		// this gives us the original xref table to the end of the file
		// we actually are going to read from the trailer, but this insures we have enough, but not too much, data to do so
		
	int numberOfObjects = [self numberOfObjectsFromData:xrefForPDF];
	NSLog(@"the number of objects in the PDF is: %d", numberOfObjects);
	
	NSString *startObjectString = [NSString stringWithFormat:@"%d 0 obj\r<<\r/Length %d\r>>\rstream\r\n",
										contentObjectNum,
										[layer1 length]];
	NSData *startObjectData = [startObjectString dataUsingEncoding:NSASCIIStringEncoding];
	NSLog(@"%@", [startObjectData description]);
	
	NSString *endObjectString = @"endstream\rendobject\r";
	NSData *endObjectData = [endObjectString dataUsingEncoding:NSASCIIStringEncoding];
	
	[layer1FileData appendData:startObjectData]; // [startObjectData subdataWithRange:NSMakeRange( 0, [startObjectData length]-1 )]];
	[layer1FileData appendData:layer1];
	[layer1FileData appendData:endObjectData]; // [endObjectData subdataWithRange:NSMakeRange( 0, [endObjectData length]-1 )]];
	
	int xref_location = [layer1FileData length];
	
	NSString *xrefString = [NSString stringWithFormat:@"xref\r0 1\r0000000000 65535 f \r%d 1\r%010d 00000 n \r",
										contentObjectNum,
										( [fileData length] + 1 )];
	NSData *xrefData = [xrefString dataUsingEncoding:NSASCIIStringEncoding];
										
	[layer1FileData appendData:xrefData]; // [xrefData subdataWithRange:NSMakeRange( 0, [xrefData length]-1 )]];

	NSString *trailerString = [NSString stringWithFormat:@"trailer\r<<\r/Size %d\r/Root 1 0 R\r/Prev %d\r>>\r",
										numberOfObjects,
										original_startxref];
	
	NSData *trailerData = [trailerString dataUsingEncoding:NSASCIIStringEncoding];
										
	[layer1FileData appendData:trailerData]; // [trailerData subdataWithRange:NSMakeRange( 0, [trailerData length]-1 )]];

	NSString *endString = [NSString stringWithFormat:@"startxref\r%d\r",
										xref_location]; 

	NSData *endData = [endString dataUsingEncoding:NSASCIIStringEncoding];
										
	[layer1FileData appendData:endData]; // [endData subdataWithRange:NSMakeRange( 0, [endData length]-1 )]];
	
	NSString *eolString = [@"\x25\x25" stringByAppendingString:@"EOF\r"]; 

	NSData *eolData = [eolString dataUsingEncoding:NSASCIIStringEncoding];
										
	[layer1FileData appendData:eolData]; // [endData subdataWithRange:NSMakeRange( 0, [endData length]-1 )]];
	
	// now it's time to write the data out to a file
	NSString *tmpPath = [@"~/Desktop/AmpedeTest.pdf" stringByStandardizingPath];
	
	[layer1FileData writeToFile:tmpPath atomically:NO];
	
//************************************************************* Layer 2 *********************************************//

	NSMutableData *layer2FileData = [NSMutableData data];
	[layer2FileData appendData:fileData];
	
	contentObjectNum = [self contentObjectNumber];
	NSLog(@"page content number is: %d", contentObjectNum);
	
	endOfPDF = [fileData subdataWithRange:NSMakeRange( [fileData length] - 64, 64 )];
		// this gives us the last 64 bytes, enough to determine the location of the pdf's cross-reference table
	
	original_startxref = [self startxrefFromData:endOfPDF];
	NSLog(@"original startxref number is: %d", original_startxref);
	
	xrefForPDF = [fileData subdataWithRange:NSMakeRange( original_startxref, [fileData length] - original_startxref )];
		// this gives us the original xref table to the end of the file
		// we actually are going to read from the trailer, but this insures we have enough, but not too much, data to do so
		
	numberOfObjects = [self numberOfObjectsFromData:xrefForPDF];
	NSLog(@"the number of objects in the PDF is: %d", numberOfObjects);
	
	startObjectString = [NSString stringWithFormat:@"%d 0 obj\r<<\r/Length %d\r>>\rstream\r\n",
										contentObjectNum,
										[layer2 length]];
	startObjectData = [startObjectString dataUsingEncoding:NSASCIIStringEncoding];
	NSLog(@"%@", [startObjectData description]);
	
	endObjectString = @"endstream\rendobject\r";
	endObjectData = [endObjectString dataUsingEncoding:NSASCIIStringEncoding];
	
	[layer2FileData appendData:startObjectData]; // [startObjectData subdataWithRange:NSMakeRange( 0, [startObjectData length]-1 )]];
	[layer2FileData appendData:layer2];
	[layer2FileData appendData:endObjectData]; // [endObjectData subdataWithRange:NSMakeRange( 0, [endObjectData length]-1 )]];
	
	xref_location = [layer2FileData length];
	
	xrefString = [NSString stringWithFormat:@"xref\r0 1\r0000000000 65535 f \r%d 1\r%010d 00000 n \r",
										contentObjectNum,
										( [fileData length] + 1 )];
	xrefData = [xrefString dataUsingEncoding:NSASCIIStringEncoding];
										
	[layer2FileData appendData:xrefData]; // [xrefData subdataWithRange:NSMakeRange( 0, [xrefData length]-1 )]];

	trailerString = [NSString stringWithFormat:@"trailer\r<<\r/Size %d\r/Root 1 0 R\r/Prev %d\r>>\r",
										numberOfObjects,
										original_startxref];
	
	trailerData = [trailerString dataUsingEncoding:NSASCIIStringEncoding];
										
	[layer2FileData appendData:trailerData]; // [trailerData subdataWithRange:NSMakeRange( 0, [trailerData length]-1 )]];

	endString = [NSString stringWithFormat:@"startxref\r%d\r",
										xref_location]; 

	endData = [endString dataUsingEncoding:NSASCIIStringEncoding];
										
	[layer2FileData appendData:endData]; // [endData subdataWithRange:NSMakeRange( 0, [endData length]-1 )]];
	
	eolString = [@"\x25\x25" stringByAppendingString:@"EOF\r"]; 

	eolData = [eolString dataUsingEncoding:NSASCIIStringEncoding];
										
	[layer2FileData appendData:eolData]; // [endData subdataWithRange:NSMakeRange( 0, [endData length]-1 )]];
	
	// now it's time to write the data out to a file
	tmpPath = [@"~/Desktop/AmpedeTest2.pdf" stringByStandardizingPath];
	
	[layer2FileData writeToFile:tmpPath atomically:NO];
	
    return ( [self numberOfLayers] ) ? YES : NO;
}

@end
