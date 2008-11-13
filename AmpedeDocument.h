//
//  AmpedeDocument.h
//  Ampede
//
//  Created by Eric Ocean on Fri Jul 30 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "DrawingEnums.h"


@interface AmpedeDocument : NSDocument
{
	NSData *uuidData;
	CFStringRef uuidString; // makes it easier to recreate in +detachThreadForDocument:
	CFMessagePortRef _messagePort;
	CFRunLoopSourceRef _runLoopSource;

	////////// DO NOT MODIFY OR REARRANGE THIS DATA //////////
	int antialias;
	int shouldSmoothFonts;
	int intent;
	int quality;
	int drawStyle;
	int superSampling;
	float flatness;
	int shouldColorManage;
	int doSubpixelRendering;
	int forceRefresh;
	////////// DO NOT MODIFY OR REARRANGE THIS DATA //////////

	int width;
	int height;
	float pixelAspectRatio;
	
	BOOL dataCacheIsValid;
	NSMutableData *dataCache;
}

- (void)invalidate;

- (NSData *)returnDrawData:(NSData *)data;
- (NSData *)returnFileData:(NSData *)data;
- (NSData *)showOptionsWithData:(NSData *)theData;
- (NSData *)cacheImageData:(NSData *)theData;
- (NSData *)returnCacheForData:(NSData *)theData;
- (NSData *)returnFlattenedData:(NSData *)theData;
- (void)loadFlattenedData:(NSData *)theData;
- (NSData *)renderPageToPDFData:(CGPDFPageRef)thePage;
- (void)drawPage:(CGPDFPageRef)thePage inContext:(CGContextRef)theContext;
- (void)recomputeDataCache;
- (BOOL)drawDataIsValid;

////////// Accessors //////////
- (void)setDataCache:(NSData *)theData;
- (void)setWidth:(int)theWidth;
- (void)setHeight:(int)theHeight;
- (void)setPixelAspectRatio:(float)theRatio;
- (NSData *)retainedUuidData;
- (CFStringRef)uuidString;

- (int)antialias;
- (void)setAntialias:(int)yn;
- (int)shouldSmoothFonts;
- (void)setShouldSmoothFonts:(int)yn;
- (int)shouldColorManage;
- (void)setShouldColorManage:(int)yn;
- (int)doSubpixelRendering;
- (void)setDoSubpixelRendering:(int)yn;

- (int)intent;
- (void)setIntent:(int)tag;
- (int)quality;
- (void)setQuality:(int)tag;
- (int)drawStyle;
- (void)setDrawStyle:(int)tag;
- (int)superSampling;
- (void)setSuperSampling:(int)tag;

- (float)flatness;
- (void)setFlatness:(float)f;

- (void)setMessagePort:(CFMessagePortRef)mp;
- (void)setRunLoopSource:(CFRunLoopSourceRef)rls;

////////// CFMessagePort //////////
- (NSData *)
handleMessage:(SInt32)msgid
withData:(NSData *)data;

+ (void)detachThreadForDocument:(AmpedeDocument *)doc;

@end
