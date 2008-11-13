//
//  AmpedeDocument.m
//  Ampede
//
//  Created by Eric Ocean on Fri Jul 30 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "AmpedeDocument.h"
#import "SequenceWindowController.h"
#import "Ampede.h"

//#define DEBUG_CALLING_SEQUENCE


CFDataRef sequenceDocumentDefaultPortCallback2( CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info );
CFDataRef sequenceDocumentDefaultPortCallback2( CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info )
{
	return (CFDataRef)[(AmpedeDocument *)info handleMessage:msgid withData:(NSData *)data];
}

@implementation AmpedeDocument

- (void)
setMessagePort:(CFMessagePortRef)mp;
{
#ifdef DEBUG_CALLING_SEQUENCE
		CFShow( CFSTR("-[AmpedeDocument setMessagePort:] called") );
#endif		
	if ( _messagePort ) NSLog(@"AmpedeUI error: -[AmpedeDocument setMessagePort:] called multiple times on the same object");
	_messagePort = mp;
}

- (void)
setRunLoopSource:(CFRunLoopSourceRef)rls;
{
#ifdef DEBUG_CALLING_SEQUENCE
		CFShow( CFSTR("-[AmpedeDocument setRunLoopSource:] called") );
#endif		
	if ( _runLoopSource ) NSLog(@"AmpedeUI error: -[AmpedeDocument setRunLoopSource:] called multiple times on the same object");
	_runLoopSource = rls;
}

- (CFStringRef)
uuidString;
{
#ifdef DEBUG_CALLING_SEQUENCE
		CFShow( CFSTR("-[AmpedeDocument uuidString] called") );
#endif		
	return uuidString;
}

+ (void)
detachThreadForDocument:(AmpedeDocument *)doc;
{
#ifdef DEBUG_CALLING_SEQUENCE
		CFShow( CFSTR("-[AmpedeDocument detachThreadForDocument:] called") );
#endif		
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	CFMessagePortContext defaultPortContext = { 0, doc, NULL, NULL, NULL };
	CFRunLoopSourceRef runLoopSource = NULL;
	CFMessagePortRef messagePort = CFMessagePortCreateLocal(	kCFAllocatorDefault,
																[doc uuidString],
																sequenceDocumentDefaultPortCallback2,
																&defaultPortContext,
																NULL	);
	if ( messagePort != NULL )
	{
		runLoopSource = CFMessagePortCreateRunLoopSource( kCFAllocatorDefault, messagePort, 0 );

		if ( runLoopSource != NULL )
		{
			CFRunLoopAddSource( [[NSRunLoop currentRunLoop] getCFRunLoop], runLoopSource, kCFRunLoopCommonModes );
		}
		else
		{
			CFRelease( messagePort ); messagePort = NULL;
			CFShow( CFSTR("AmpedeUI error: failed to create run loop source for an Ampede document message port.") );				
		}
	}
	else
	{
		CFShow( CFSTR("AmpedeUI error: Failed to create an Ampede document message port.") );
	}
	
	[doc setMessagePort:messagePort];
	[doc setRunLoopSource:runLoopSource];
	
	[[NSRunLoop currentRunLoop] run];
	
	[pool release];
}

- (void)
dealloc
{
#ifdef DEBUG_CALLING_SEQUENCE
		CFShow( CFSTR("-[AmpedeDocument dealloc] called") );
#endif		
	if ( _messagePort )
	{
		CFMessagePortInvalidate( _messagePort );
		CFRelease( _messagePort );
	}
	if ( _runLoopSource ) CFRelease( _runLoopSource );
	
	[uuidData release]; uuidData = nil;
	[self removeObserver:self forKeyPath:@"dataCacheIsValid"];
	[super dealloc];
}

- (NSData *)retainedUuidData;
{
#ifdef DEBUG_CALLING_SEQUENCE
		CFShow( CFSTR("-[AmpedeDocument retainedUuidData] called") );
#endif		
	return [uuidData retain];
}

+ (void)
initialize
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("+[AmpedeDocument initialize] called") );
#endif
	static BOOL classDidInitialize = NO; if ( classDidInitialize ) return; classDidInitialize = YES;
	
    [self setKeys:[NSArray arrayWithObjects:
									@"antialias",
									@"shouldSmoothFonts",
									@"shouldPreserveAspectRatio",
									@"shouldDrawPageBackground",
									@"intent",
									@"quality",
									@"drawStyle",
									@"superSampling",
									@"width",
									@"height",
									@"pixelAspectRatio",
									@"scaleMultiple",
									@"flatness", 
									@"shouldColorManage",
									@"doSubpixelRendering", nil]
			triggerChangeNotificationsForDependentKey:@"dataCacheIsValid"];
}

+ (BOOL)
automaticallyNotifiesObserversForKey:(NSString *)theKey
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("+[AmpedeDocument automaticallyNotifiesObserversForKey:] called") );
#endif
	BOOL automatic = YES;
    if (	[theKey isEqualToString:@"width"] ||
			[theKey isEqualToString:@"height"] ||
			[theKey isEqualToString:@"pixelAspectRatio"]	)
	{
        automatic = NO;
    }
	else automatic = [super automaticallyNotifiesObserversForKey:theKey];
    return automatic;
}

- (int)
maximumSuperSampling;
{
	return 16;
}

- (int)
minimumSuperSampling;
{
	return 2;
}

- init
{
	if ( self = [super init] )
	{
#ifdef DEBUG_CALLING_SEQUENCE
		CFShow( CFSTR("-[AmpedeDocument init] called") );
#endif		
		dataCache = nil;

		CFUUIDRef ref = CFUUIDCreate( kCFAllocatorDefault );
		CFUUIDBytes bytes = CFUUIDGetUUIDBytes( ref );
		uuidData = [[NSData alloc] initWithBytes:(const void *)&bytes length:16];
		uuidString = CFUUIDCreateString( kCFAllocatorDefault, ref );
		
		quality = kCGInterpolationDefault;
		antialias = YES;
		intent = kCGRenderingIntentDefault;
		flatness = 1.0;
		shouldSmoothFonts = 0;
		drawStyle = 0;
		
		doSubpixelRendering = 0;
		shouldColorManage = 0;
		
		superSampling = 2;
		forceRefresh = 1; // never changed
		
		dataCacheIsValid = NO;
		
		CFRelease( ref );
		
		[NSThread detachNewThreadSelector:@selector(detachThreadForDocument:) toTarget:[AmpedeDocument class] withObject:self];

		// this insures that we receive notifications when dataCacheIsValid "changes"
		// which is whenever one of its dependent keys is updated
		[self	addObserver:self
				forKeyPath:@"dataCacheIsValid" 
				options:NSKeyValueObservingOptionNew
				context:NULL];
	}
	return self;
}

- (void)
observeValueForKeyPath:(NSString *)keyPath
ofObject:(id)object 
change:(NSDictionary *)change
context:(void *)context;
{
#ifdef DEBUG_CALLING_SEQUENCE
		CFShow( CFSTR("[AmpedeDocument observeValueForKeyPath:ofObject:change:context:] called") );
#endif
    if ( [keyPath isEqualToString:@"dataCacheIsValid"] )
	{
		// this is triggered if any of our dependent keys changes--see above
		dataCacheIsValid = NO;
#ifdef DEBUG_CALLING_SEQUENCE
		CFShow( CFSTR("dataCacheIsValid observer notification called") );
#endif
	}
}

- (NSData *)
handleMessage:(SInt32)msgid
withData:(NSData *)data;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument handleMessage:withData:] called") );
#endif
	if ( msgid == 'DATA' )
	{
		int *ints = (int *)[data bytes];
		float *floats = (float *)[data bytes];

		[self setWidth:*ints];
		[self setHeight:*(ints+1)];
		[self setPixelAspectRatio:*(floats+2)];
		
		if ( !dataCacheIsValid ) [self recomputeDataCache];
		else
		{
			int *ints = (int *)[dataCache bytes];
			ints[9] = 0;
		}

		return [dataCache retain]; // CFMessagePort will release it
	}
	else if ( msgid == 'DRAW' ) return [[self returnDrawData:data] retain];			// CFMessagePort will release it
	else if ( msgid == 'FILE' ) return [[self returnFileData:data] retain];			// CFMessagePort will release it
	else if ( msgid == 'INFO' )
	{
		if ( !TRIAL_TIMED_OUT ) return [[self showOptionsWithData:data] retain];	// CFMessagePort will release it
		else
		{
			NSAlert *alert = [[NSAlert alloc] init];
			[alert addButtonWithTitle:@"Purchase"];
			[alert addButtonWithTitle:@"Cancel"];
			[alert setAlertStyle:NSInformationalAlertStyle];
			[alert setMessageText:@"Please purchase Ampede PDF today!"];
			[alert setInformativeText:@"Your trial period has expired, and your 10 minute demo limit has been reached. We hope you like Ampede PDF. If you use it, please pay for it so we can make it better!"];
			
			[NSApp activateIgnoringOtherApps:YES]; // put us on top
			int returnButton = [alert runModal];
			
			if ( returnButton == NSAlertFirstButtonReturn )
			{
				// user wants to purchase Ampede PDF
				[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://www.ampede.com/pdf/buy.html"]];
			}
			else if ( returnButton == NSAlertSecondButtonReturn )
			{
				; // user canceled
			}
			
			return NULL;
		}
	}
	else if ( msgid == 'SAVE' ) return [[self cacheImageData:data] retain];			// CFMessagePort will release it
	else if ( msgid == 'RSTR' ) return [[self returnCacheForData:data] retain];		// CFMessagePort will release it
	else if ( msgid == 'FLAT' )
	{
		id flattenedData = [[self returnFlattenedData:data] retain];
//		NSData *readableData = [NSData dataWithData:flattenedData];
//		NSLog(@"flattenedData is %@", readableData);
		return flattenedData;	// CFMessagePort will release it
	}
	else if ( msgid == 'LOAD' )
	{
//		NSLog(@"flattenedData for LOAD is %@", data);
		[self loadFlattenedData:data];
		return nil;
	}
	else
	{
		NSAssert( NO, @"Amped error: an unknown msgid was used." );
		return NULL;
	}
}

- (void)invalidate;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument invalidate] called") );
#endif
	dataCacheIsValid = NO;
}

- (void)
recomputeDataCache;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument recomputeDataCache] called") );
#endif
	NSAssert( dataCacheIsValid == NO, @"recomputeDataCache called when dataCacheIsValid == YES" );
	
	int drawDataIsValid = [self drawDataIsValid];
	NSMutableData *tmp = [NSMutableData dataWithCapacity:44]; // Create a new instance containing all of our ivars.
	char *rawBytes = (char *)&antialias; // This is the beginning of our ivars.
	
	// this gets the raw data for all of the ivars we want to send to the plug-in
	[tmp appendBytes:rawBytes length:40];
	[tmp appendBytes:&drawDataIsValid length:4];
	
	[self setDataCache:tmp];
	dataCacheIsValid = YES;
}

- (NSData *)
returnDrawData:(NSData *)data;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument returnDrawData:] called") );
#endif
	NSString *pdfPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"default" ofType:@"pdf"];
	CGPDFDocumentRef pdf = NULL;
	CGPDFPageRef page = NULL;

	if ( pdfPath )
	{
		pdf = CGPDFDocumentCreateWithURL( (CFURLRef)[NSURL fileURLWithPath:pdfPath] );
		if ( pdf) page = CGPDFDocumentGetPage( pdf, 1 );
		NSData *returnData = [self renderPageToPDFData:page];
		CGPDFDocumentRelease( pdf );
		return returnData;
	}
	return NULL;
}

- (NSData *)
returnFileData:(NSData *)data;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument returnFileData:] called") );
#endif
	return nil;
}

- (NSData *)
showOptionsWithData:(NSData *)theData;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument showOptionsWithData:] called") );
#endif
	return NULL;
}

- (NSData *)
cacheImageData:(NSData *)theData;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument cacheImageData:] called") );
#endif
	return NULL;
}

- (NSData *)
returnCacheForData:(NSData *)theData;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument returnCacheForData:] called") );
#endif
	return NULL;
}

- (NSData *)
returnFlattenedData:(NSData *)theData;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument returnFlattenedData:] called") );
#endif
	return [self dataRepresentationOfType:nil];
}

- (void)
loadFlattenedData:(NSData *)theData;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument loadFlattenedData:] called") );
#endif
	[self loadDataRepresentation:theData ofType:nil];
}

size_t
PutBytesInNSMutableData( void *mutableData, const void *buffer, size_t count )
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("AmpedeDocument.h PutBytesInNSMutableData() called") );
#endif
	[(NSMutableData *)mutableData appendBytes:buffer length:count];
	return count;
}

- (NSData *)
renderPageToPDFData:(CGPDFPageRef)thePage
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument renderPageToPDFData:] called") );
#endif
	NSMutableData *returnData = [[NSMutableData alloc] init];
	CGDataConsumerCallbacks cb = { PutBytesInNSMutableData, NULL }; // we don't want the data consumer to release the memory
	CGDataConsumerRef dc = CGDataConsumerCreate( returnData, &cb );
	CGRect mb = CGRectMake( 0, 0, width, height );
	CGContextRef pdfContext = CGPDFContextCreate( dc, &mb, NULL );

	CGContextBeginPage( pdfContext, &mb );
	[self drawPage:thePage inContext:pdfContext];
	CGContextEndPage( pdfContext );
	
	CGContextRelease( pdfContext );
	CGDataConsumerRelease( dc );
	
	[returnData writeToFile:@"/Users/bizman/Desktop/testPDFOutput.txt" atomically:NO];
	CFShow( CFSTR("wrote a new testPDFOutput.txt file") );
	return [returnData autorelease];
}

- (void)
drawPage:(CGPDFPageRef)thePage
inContext:(CGContextRef)theContext;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument drawPage:inContext:] called") );
#endif
	CGAffineTransform tr = CGPDFPageGetDrawingTransform(	thePage,
															kCGPDFMediaBox,
															CGRectMake( 0, 0, width, height ),
															0,
															YES		);
	CGRect mb = CGPDFPageGetBoxRect( thePage, kCGPDFMediaBox );
	CGContextConcatCTM( theContext, tr );
	CGContextSetRGBFillColor( theContext, 1.0, 1.0, 1.0, 1.0 );
	CGContextFillRect( theContext, mb );
	CGContextDrawPDFPage( theContext, thePage );
}

// subclasses over-ride this based on the state of their drawing data
- (BOOL)drawDataIsValid;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument drawDataIsValid] called") );
#endif
	// it would be better if we could force subclasses to override this somehow
	return YES;
}


////////// Internal Methods //////////
- (void)
setDataCache:(NSData *)theData;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument setDataCache:] called") );
#endif
	id myData = [theData retain];
	[dataCache release];
	dataCache = myData;
}

- (void)
setWidth:(int)theWidth;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument setWidth:] called") );
#endif
    if ( width != theWidth )
	{
        [self willChangeValueForKey:@"width"];
        width = theWidth;
        [self didChangeValueForKey:@"width"];
    }
}

- (void)
setHeight:(int)theHeight;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument setHeight:] called") );
#endif
    if ( height != theHeight )
	{
        [self willChangeValueForKey:@"height"];
        height = theHeight;
        [self didChangeValueForKey:@"height"];
    }
}

- (void)
setPixelAspectRatio:(float)theRatio;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument setPixelAspectRatio:] called") );
#endif
    if ( pixelAspectRatio != theRatio )
	{
        [self willChangeValueForKey:@"pixelAspectRatio"];
        pixelAspectRatio = theRatio;
        [self didChangeValueForKey:@"pixelAspectRatio"];
    }
}

- (int)antialias;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument antialias] called") );
#endif
	return antialias;
}

- (void)setAntialias:(int)yn;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument setAntialias:] called") );
#endif
	antialias = yn;
	dataCacheIsValid = NO;
}

- (int)shouldSmoothFonts;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument shouldSmoothFonts] called") );
#endif
	return shouldSmoothFonts;
}

- (void)setShouldSmoothFonts:(int)yn;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument setShouldSmoothFonts:] called") );
#endif
	shouldSmoothFonts = yn;
	dataCacheIsValid = NO;
}

- (int)shouldColorManage;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument shouldColorManage] called") );
#endif
	return shouldColorManage;
}

- (void)setShouldColorManage:(int)yn;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument setShouldColorManage:] called") );
#endif
	shouldColorManage = yn;
	dataCacheIsValid = NO;
}

- (int)doSubpixelRendering;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument doSubpixelRendering] called") );
#endif
	return doSubpixelRendering;
}

- (void)setDoSubpixelRendering:(int)yn;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument setDoSubpixelRendering:] called") );
#endif
	doSubpixelRendering = yn;
	dataCacheIsValid = NO;
}

- (int)intent;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument intent] called") );
#endif
	return intent;
}

- (void)setIntent:(int)tag;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument setIntent:] called") );
#endif
	intent = tag;
	dataCacheIsValid = NO;
}

- (int)quality;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument quality] called") );
#endif
	return quality;
}

- (void)setQuality:(int)tag;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument setQuality:] called") );
#endif
	quality = tag;
	dataCacheIsValid = NO;
}

- (int)drawStyle;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument drawStyle] called") );
#endif
	return drawStyle;
}

- (void)setDrawStyle:(int)tag;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument setDrawStyle:] called") );
#endif
	drawStyle = tag;
	dataCacheIsValid = NO;
}

- (int)superSampling;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument superSampling] called") );
#endif
	return superSampling;
}

- (void)setSuperSampling:(int)tag;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument setSuperSampling:] called") );
#endif
	superSampling = tag;
	dataCacheIsValid = NO;
}

- (float)flatness;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument flatness] called") );
#endif
	return flatness;
}

- (void)setFlatness:(float)f;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[AmpedeDocument setFlatness:] called") );
#endif
	flatness = f;
	dataCacheIsValid = NO;
}


//- (NSData *)
//dataRepresentationOfType:(NSString *)type
//{
//#ifdef DEBUG_CALLING_SEQUENCE
//	CFShow( CFSTR("-[AmpedeDocument dataRepresentationOfType:] called") );
//#endif
//	// Implement to provide a persistent data representation of your document OR remove this and implement the
//	// file-wrapper or file path based save methods.
//    return nil;
//}
//
//- (BOOL)
//loadDataRepresentation:(NSData *)data
//ofType:(NSString *)type
//{
//#ifdef DEBUG_CALLING_SEQUENCE
//	CFShow( CFSTR("-[AmpedeDocument loadDataRepresentation:ofType:] called") );
//#endif
//    // Implement to load a persistent data representation of your document OR remove this and implement the
//	// file-wrapper or file path based load methods.
//    return YES;
//}
//
@end
