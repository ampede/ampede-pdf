//
//  PDFDocument.m
//  Ampede
//
//  Created by Eric Ocean on Thu Jul 15 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "PDFDocument.h"
#import "PDFView.h"

#import <Accelerate/Accelerate.h>


@implementation PDFDocument

- (NSData *)
handleMessage:(SInt32)msgid
withData:(NSData *)data;
{
	CFDataRef returnData;
	double scale;

	if ( msgid == 'DRAW' )
	{
		//
		// calculate drawing info and return results
		//
		
		if ( data )
		{
			int *dims = (int *)[data bytes];
			width = *dims;
			height = *(dims+1);
			double *scales = (double *)[data bytes];
			scale = *(scales+1); // scale is 8 bytes in
		}
		else return NULL;
		
		CGContextRef    myContext = NULL;
		CGColorSpaceRef colorSpace;
		char *          bitmapData;
		char *          rBitmapData;
		int             bitmapByteCount;
		int             bitmapBytesPerRow;
		
#define SUPER_SAMPLE 2

		bitmapBytesPerRow   = (width * 4); 
		bitmapByteCount     = (bitmapBytesPerRow * height);

		colorSpace = CGColorSpaceCreateDeviceRGB();
		
		bitmapData = malloc( bitmapByteCount*SUPER_SAMPLE*SUPER_SAMPLE );
		rBitmapData = malloc( bitmapByteCount );
		
		if( bitmapData != NULL && rBitmapData != NULL )
		{
			myContext = CGBitmapContextCreate (		bitmapData, 
													width*SUPER_SAMPLE,
													height*SUPER_SAMPLE,
													8,      // bits per component
													bitmapBytesPerRow*SUPER_SAMPLE,
													colorSpace,
													kCGImageAlphaPremultipliedFirst		);
			if (myContext != NULL)
			{
				page = CGPDFDocumentGetPage( pdf, currentPage );

				CGRect cgRect = CGRectMake( 0, 0, width*SUPER_SAMPLE, height*SUPER_SAMPLE );
				
//				CGContextSetInterpolationQuality( myContext, quality );
//				CGContextSetShouldAntialias( myContext, antialias );
//				CGContextSetRenderingIntent( myContext, intent );
//				CGContextSetShouldSmoothFonts( myContext, true );
//				CGContextSetFlatness( myContext, 1.0 );
				
				CGAffineTransform myTransform = CGPDFPageGetDrawingTransform(   page, // the PDF page
																				kCGPDFMediaBox, // the box on the page
																				cgRect, // the box to scale it to
																				0, // an optional rotation
																				true	); // whether to preserve the aspect ratio
				CGRect mb = CGPDFPageGetBoxRect( page, kCGPDFMediaBox );
				CGContextConcatCTM( myContext, myTransform );
				CGContextTranslateCTM( myContext, (mb.size.width/2), (mb.size.height/2) );
				CGContextScaleCTM( myContext, scale, scale );
				CGContextTranslateCTM( myContext, -(mb.size.width/2), -(mb.size.height/2) );
				CGContextSetRGBFillColor( myContext, 1.0, 1.0, 1.0, 1.0 );
				CGContextFillRect( myContext, mb );
				CGContextDrawPDFPage( myContext, page );
				
				///// Now create the supersampled image at the correct size
				vImage_Error imgErr;
				vImage_Buffer srcBuf;
				vImage_Buffer dstBuf;
				
				srcBuf.data = bitmapData;
				srcBuf.height = height*SUPER_SAMPLE;
				srcBuf.width = width*SUPER_SAMPLE;
				srcBuf.rowBytes = bitmapBytesPerRow*SUPER_SAMPLE;
				
				dstBuf.data = rBitmapData;
				dstBuf.height = height;
				dstBuf.width = width;
				dstBuf.rowBytes = bitmapBytesPerRow;
				
				imgErr = vImageScale_ARGB8888( &srcBuf, &dstBuf, NULL, kvImageEdgeExtend | kvImageHighQualityResampling );
				
				if ( imgErr )
				{
					free( bitmapData );
					free( rBitmapData );
					return NULL;
				}

				// create the CFData to return
				returnData = CFDataCreateWithBytesNoCopy( kCFAllocatorDefault, rBitmapData, bitmapByteCount, kCFAllocatorDefault );
				
				// don't need to free the rBitmapData we allocated, because returnData will do it
				free( bitmapData );
			}
			else
			{
				free( bitmapData );
				free( rBitmapData );
				return NULL;
			}
		}
		else
		{
			if ( bitmapData ) free( bitmapData );
			if ( rBitmapData ) free( rBitmapData );
		}
		
		CGColorSpaceRelease( colorSpace ); 
		CGContextRelease( myContext );
		
		return (NSData *)returnData;
	}
	else if ( msgid == 'INFO' )
	{
		[NSApp activateIgnoringOtherApps:YES];
		[NSApp orderFrontStandardAboutPanel:nil]; 
	}
	return NULL;
}

- (void)
messagePortInvalidated:(CFMessagePortRef)ms;
{
	NSRunCriticalAlertPanel(	@"Message Port Invalidated.", // title
								@"Probably should do something about that here. Write code.", // message
								@"Continue", // default button
								nil,
								nil		);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- init
{
	if ( self = [super init] ) {
		pdf = nil;
		page = nil;
		currentPage = 3;
	}
	return self;
}

- (void)
dealloc;
{
	if ( pdf ) CGPDFDocumentRelease( pdf );
	// I believe that pages are retained by their documents
//	if ( page ) CGPDFPageRelease( page );
	[super dealloc];
}

- (NSString *)windowNibName
{
    // Implement this to return a nib to load OR implement -makeWindowControllers to manually create your controllers.
    return @"PDFDocument";
}

- (NSData *)
dataRepresentationOfType:(NSString *)type
{
    // Implement to provide a persistent data representation of your document OR remove this and implement the
	// file-wrapper or file path based save methods.
    return nil;
}

- (BOOL)
readFromFile:(NSString *)fileName
ofType:(NSString *)docType;
{
	pdf = CGPDFDocumentCreateWithURL( (CFURLRef)[NSURL fileURLWithPath:fileName] );
	if ( pdf)
	{
		page = CGPDFDocumentGetPage( pdf, currentPage );
		pageCount = CGPDFDocumentGetNumberOfPages( pdf );
	}
	return ( pdf ) ? YES : NO;
}

- (void)setCurrentPage:(size_t)newPage;
{
	if ( currentPage != newPage )
	{
		currentPage = newPage;
		[pdfView setPage:CGPDFDocumentGetPage( pdf, currentPage )];
	}
}

- (void)
windowControllerDidLoadNib:(NSWindowController *)wc;
{
	[pdfView setPage:page];
}

//- (BOOL)
//loadDataRepresentation:(NSData *)data
//ofType:(NSString *)type
//{
//    // Implement to load a persistent data representation of your document OR remove this and implement the
//	// file-wrapper or file path based load methods.
//	fileData = [data retain];
//	
//	PDFLexer *lexer = [[PDFLexer alloc] initWithData:fileData];
//	id tok = nil;
//	NSError *err = nil;
//	NSString *str = nil;
//		// this is only here to force err to be set to nil; I don't why the compiler doesn't do it normally
//	if ( err != nil ) NSLog(@"err variable improperly initialized to a value not nil in %s %d", __FILE__, __LINE__);
//	
//	
//	do {
//		tok = [lexer nextTokenWithError:&err];
//	} while ( tok );
//		
//	if ( err )
//	{
//		NSLog(@"%@", err);
//		[err release];
//	}
//	else NSLog(@"lexer finished normally");
//	
//    return YES;
//}

@end
