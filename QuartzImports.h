/*
 *  QuartzImports.h
 *  Ampede
 *
 *  Created by Eric Ocean on Sun Aug 01 2004.
 *  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
 *
 */

////////// CGDataProviderDirectAccessCallbacks //////////

size_t
MyProviderGetBytesWithOffset (
	   void * info,    // A CFData instance.
	   void * buffer,    
	   size_t offset,    
	   size_t count
	)
{
	CFRange loc = CFRangeMake(offset, count);
	
	size_t length = CFDataGetLength( (CFDataRef)info );
	
	if ( (offset + count) < length )
	{	
		CFDataGetBytes( (CFDataRef)info, loc, buffer );
		return count;
	}
	else
	{
		if ( offset >= length ) return 0;
		else
		{
			size_t newCount = length - offset;
			if ( !( (offset + count) < length ) ) CFShow( CFSTR("Ampede error: range math is wrong (QuartzImport.h)") );
			
			loc = CFRangeMake(offset, newCount);
			CFDataGetBytes( (CFDataRef)info, loc, buffer );
			return 0;
		}
	}
}

void *
MyProviderGetBytePointer (
	   void * info    // A CFData instance.
	)
{
	return (void *)CFDataGetBytePtr( (CFDataRef)info );
}

void
MyProviderReleaseBytePointer (
	   void * info,    // A CFData instance.
	   const void * pointer
	)
{
#pragma unused ( info, pointer )
}

void
MyProviderReleaseProvider (
	   void * info,    // A CFData instance.
	)
{
	CFRelease( (CFDataRef)info );
}

typedef size_t (*MyProviderGetBytesWithOffsetCallback)( void * info, void * buffer, size_t offset, size_t count );
typedef void *(*MyProviderGetBytePointerCallback)( void * info );
typedef void (*MyProviderReleaseBytePointerCallback)( void * info, const void * pointer );
typedef void (*MyProviderReleaseProviderCallback)( void * info );

static MyProviderGetBytesWithOffsetCallback MyProviderGetBytesWithOffsetPtr;
static MyProviderGetBytePointerCallback MyProviderGetBytePointerPtr;
static MyProviderReleaseBytePointerCallback MyProviderReleaseBytePointerPtr;
static MyProviderReleaseProviderCallback MyProviderReleaseProviderPtr;

////////// Mach-O function pointers to load in //////////

static CFBundleRef cgBundle;
static CFBundleRef vBundle;

typedef CGColorSpaceRef (*CGColorSpaceCreateDeviceRGBPtr)(void);
typedef CGColorSpaceRef (*CGColorSpaceCreateWithPlatformColorSpacePtr)(void *platformColorSpaceReference);
typedef CGContextRef (*CGBitmapContextCreatePtr)(void *data, size_t width, size_t height, size_t bitsPerComponent, size_t bytesPerRow, CGColorSpaceRef colorspace, CGImageAlphaInfo alphaInfo);
typedef CGPDFPageRef (*CGPDFDocumentGetPagePtr)(CGPDFDocumentRef document, size_t pageNumber);
typedef void (*CGContextReleasePtr)(CGContextRef c);
typedef void (*CGPDFPageReleasePtr)(CGPDFPageRef page);
typedef void (*CGPDFDocumentReleasePtr)(CGPDFDocumentRef document);
typedef CGDataProviderRef (*CGDataProviderCreateWithDataPtr)(void *info, const void *data, size_t size, void (*releaseData)(void *info, const void *data, size_t size));
typedef CGPDFDocumentRef (*CGPDFDocumentCreateWithProviderPtr)(CGDataProviderRef provider);
typedef void (*CGDataProviderReleasePtr)(CGDataProviderRef provider);
typedef void (*CGContextDrawPDFPagePtr)(CGContextRef c, CGPDFPageRef page);
typedef void *(*CGBitmapContextGetDataPtr)(CGContextRef c);
typedef size_t (*CGBitmapContextGetWidthPtr)(CGContextRef c);
typedef size_t (*CGBitmapContextGetHeightPtr)(CGContextRef c);
typedef void (*CGContextDrawPDFDocumentPtr)(CGContextRef c, CGRect rect, CGPDFDocumentRef document, int page);
typedef CGPDFDocumentRef (*CGPDFDocumentCreateWithURLPtr)(CFURLRef url);
typedef CGDataProviderRef (*CGDataProviderCreateDirectAccessPtr)(void *info, size_t size, const CGDataProviderDirectAccessCallbacks *callbacks);
typedef size_t (*CGBitmapContextGetBytesPerRowPtr)(CGContextRef c);
typedef void (*CGContextSaveGStatePtr)(CGContextRef c);
typedef void (*CGContextRestoreGStatePtr)(CGContextRef c);
typedef void (*CGContextScaleCTMPtr)(CGContextRef c, float sx, float sy);
typedef void (*CGContextTranslateCTMPtr)(CGContextRef c, float tx, float ty);
typedef void (*CGContextRotateCTMPtr)(CGContextRef c, float angle);
typedef void (*CGContextClearRectPtr)(CGContextRef c, CGRect rect);
typedef void (*CGContextSetRenderingIntentPtr)(CGContextRef c, CGColorRenderingIntent intent);
typedef void (*CGContextSetInterpolationQualityPtr)(CGContextRef c, CGInterpolationQuality quality);
typedef void (*CGContextSetShouldAntialiasPtr)(CGContextRef c, int shouldAntialias);
typedef void (*CGContextSetShouldSmoothFontsPtr)(CGContextRef c, int shouldSmoothFonts);
typedef void (*CGContextSetFlatnessPtr)(CGContextRef c, float flatness);
typedef CGRect (*CGPDFPageGetBoxRectPtr)( CGPDFPageRef page, CGPDFBox box );
typedef CGColorRef (*CGColorCreatePtr)( CGColorSpaceRef colorspace,  const float components[] );
typedef CGAffineTransform (*CGPDFPageGetDrawingTransformPtr)( CGPDFPageRef page, CGPDFBox box, CGRect rect, int rotate, int preserveAspectRatio );
typedef CGAffineTransform (*CGAffineTransformScalePtr)( CGAffineTransform t, float sx, float sy );
typedef void (*CGContextSaveGStatePtr)( CGContextRef context );
typedef void (*CGContextRestoreGStatePtr)( CGContextRef context );
typedef void (*CGContextClearRectPtr)( CGContextRef context, CGRect rect );
typedef CGAffineTransform (*CGAffineTransformInvertPtr)( CGAffineTransform t);
typedef CGPoint (*CGPointApplyAffineTransformPtr)( CGPoint point, CGAffineTransform t );
typedef CGSize (*CGSizeApplyAffineTransformPtr)( CGSize size, CGAffineTransform t );
typedef void (*CGContextConcatCTMPtr)( CGContextRef context, CGAffineTransform transform );
typedef void (*CGContextRotateCTMPtr)( CGContextRef context, float angle );
typedef void (*CGContextScaleCTMPtr)( CGContextRef context, float sx, float sy );
typedef void (*CGContextTranslateCTMPtr)( CGContextRef context, float tx, float ty );
typedef CGAffineTransform (*CGAffineTransformScalePtr)( CGAffineTransform t, float sx, float sy );
typedef void (*CGContextSetFillColorWithColorPtr)( CGContextRef context, CGColorRef color );
typedef void (*CGContextFillRectPtr)( CGContextRef context, CGRect rect );
typedef size_t (*CGBitmapContextGetBytesPerRowPtr)( CGContextRef context );

typedef vImage_Error (*vImageScale_ARGB8888Ptr)( const vImage_Buffer *src, const vImage_Buffer *dest, void *tempBuffer, vImage_Flags flags );
typedef size_t (*vImageGetMinimumGeometryTempBufferSizePtr)( const vImage_Buffer *src, const vImage_Buffer *dest, size_t bytesPerPixel, vImage_Flags flags );


static CGColorSpaceCreateWithPlatformColorSpacePtr CGColorSpaceCreateWithPlatformColorSpace;
static CGBitmapContextCreatePtr CGBitmapContextCreate;
static CGPDFDocumentGetPagePtr CGPDFDocumentGetPage;
static CGColorSpaceCreateDeviceRGBPtr CGColorSpaceCreateDeviceRGB_p;
static CGContextReleasePtr CGContextRelease_p;
static CGPDFPageReleasePtr CGPDFPageRelease;
static CGPDFDocumentReleasePtr CGPDFDocumentRelease_p;
static CGDataProviderCreateWithDataPtr CGDataProviderCreateWithData_p;
static CGPDFDocumentCreateWithProviderPtr CGPDFDocumentCreateWithProvider_p;
static CGDataProviderReleasePtr CGDataProviderRelease_p;
static CGContextDrawPDFPagePtr CGContextDrawPDFPage;
static CGBitmapContextGetDataPtr CGBitmapContextGetData;
static CGBitmapContextGetWidthPtr CGBitmapContextGetWidth;
static CGBitmapContextGetHeightPtr CGBitmapContextGetHeight;
static CGContextDrawPDFDocumentPtr CGContextDrawPDFDocument_p;
static CGPDFDocumentCreateWithURLPtr CGPDFDocumentCreateWithURL_p;
static CGDataProviderCreateDirectAccessPtr CGDataProviderCreateDirectAccess_p;
static CGBitmapContextGetBytesPerRowPtr CGBitmapContextGetBytesPerRow_p;
static CGContextSaveGStatePtr CGContextSaveGState_p;
static CGContextRestoreGStatePtr CGContextRestoreGState_p;
static CGContextScaleCTMPtr CGContextScaleCTM_p;
static CGContextTranslateCTMPtr CGContextTranslateCTM_p;
static CGContextRotateCTMPtr CGContextRotateCTM_p;
static CGContextClearRectPtr CGContextClearRect_p;
static CGContextSetRenderingIntentPtr CGContextSetRenderingIntent_p;
static CGContextSetInterpolationQualityPtr CGContextSetInterpolationQuality_p;
static CGContextSetShouldAntialiasPtr CGContextSetShouldAntialias_p;
static CGContextSetShouldSmoothFontsPtr CGContextSetShouldSmoothFonts_p;
static CGContextSetFlatnessPtr CGContextSetFlatness_p;
static CGPDFPageGetBoxRectPtr CGPDFPageGetBoxRect_p;
static CGColorCreatePtr CGColorCreate_p;
static CGPDFPageGetDrawingTransformPtr CGPDFPageGetDrawingTransform_p;
static CGAffineTransformScalePtr CGAffineTransformScale_p;
static CGContextSaveGStatePtr CGContextSaveGState_p;
static CGContextRestoreGStatePtr CGContextRestoreGState_p;
static CGContextClearRectPtr CGContextClearRect_p;
static CGAffineTransformInvertPtr CGAffineTransformInvert_p;
static CGPointApplyAffineTransformPtr CGPointApplyAffineTransform_p;
static CGSizeApplyAffineTransformPtr CGSizeApplyAffineTransform_p;
static CGContextConcatCTMPtr CGContextConcatCTM_p;
static CGContextRotateCTMPtr CGContextRotateCTM_p;
static CGContextScaleCTMPtr CGContextScaleCTM_p;
static CGContextTranslateCTMPtr CGContextTranslateCTM_p;
static CGAffineTransformScalePtr CGAffineTransformScale_p;
static CGContextSetFillColorWithColorPtr CGContextSetFillColorWithColor_p;
static CGContextFillRectPtr CGContextFillRect_p;
static CGBitmapContextGetBytesPerRowPtr CGBitmapContextGetBytesPerRow_p;

static vImageScale_ARGB8888Ptr vImageScale_ARGB8888_p;
static vImageGetMinimumGeometryTempBufferSizePtr vImageGetMinimumGeometryTempBufferSize_p;


OSStatus LoadCoreGraphicsPointers()
{
	OSStatus err = noErr;
	
	MyProviderGetBytesWithOffsetPtr = NULL;
	MyProviderGetBytePointerPtr = NULL;
	MyProviderReleaseBytePointerPtr = NULL;
	MyProviderReleaseProviderPtr = NULL;

	// make mach-O to CFM glue
	MyProviderGetBytesWithOffsetPtr = MachOFunctionPointerForCFMFunctionPointer( MyProviderGetBytesWithOffset );
	MyProviderGetBytePointerPtr = MachOFunctionPointerForCFMFunctionPointer( MyProviderGetBytePointer );
	MyProviderReleaseBytePointerPtr = MachOFunctionPointerForCFMFunctionPointer( MyProviderReleaseBytePointer );
	MyProviderReleaseProviderPtr = MachOFunctionPointerForCFMFunctionPointer( MyProviderReleaseProvider );

	err = LoadFrameworkBundle(CFSTR("Accelerate.framework"), &vBundle);
	if (err != noErr) CFShow( CFSTR("Ampede error: LoadFrameworkBundle(CFSTR(\"Accelerate.framework\")) failed.") );

	if ( err == noErr )
	{
		vImageGetMinimumGeometryTempBufferSize_p = (vImageGetMinimumGeometryTempBufferSizePtr) CFBundleGetFunctionPointerForName( vBundle, CFSTR("vImageGetMinimumGeometryTempBufferSize") );
		if (vImageGetMinimumGeometryTempBufferSize_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: vImageGetMinimumGeometryTempBufferSize_p failed.") );

		vImageScale_ARGB8888_p = (vImageScale_ARGB8888Ptr) CFBundleGetFunctionPointerForName( vBundle, CFSTR("vImageScale_ARGB8888") );
		if (vImageScale_ARGB8888_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: vImageScale_ARGB8888_p failed.") );
	}
	else return err;

	err = LoadFrameworkBundle(CFSTR("ApplicationServices.framework"), &cgBundle);
	if (err != noErr) CFShow( CFSTR("Ampede error: LoadFrameworkBundle(CFSTR(\"ApplicationServices.framework\")) failed.") );

	if ( err == noErr )
	{
		CGBitmapContextGetBytesPerRow_p = (CGBitmapContextGetBytesPerRowPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGBitmapContextGetBytesPerRow") );
		if (CGBitmapContextGetBytesPerRow_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGBitmapContextGetBytesPerRow_p failed.") );

		CGContextFillRect_p = (CGContextFillRectPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextFillRect") );
		if (CGContextFillRect_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextFillRect_p failed.") );

		CGContextSetFillColorWithColor_p = (CGContextSetFillColorWithColorPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextSetFillColorWithColor") );
		if (CGContextSetFillColorWithColor_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextSetFillColorWithColor_p failed.") );

		CGAffineTransformScale_p = (CGAffineTransformScalePtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGAffineTransformScale") );
		if (CGAffineTransformScale_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGAffineTransformScale_p failed.") );

		CGContextConcatCTM_p = (CGContextConcatCTMPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextConcatCTM") );
		if (CGContextConcatCTM_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextConcatCTM_p failed.") );

		CGContextRotateCTM_p = (CGContextRotateCTMPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextRotateCTM") );
		if (CGContextRotateCTM_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextRotateCTM_p failed.") );

		CGContextScaleCTM_p = (CGContextScaleCTMPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextScaleCTM") );
		if (CGContextScaleCTM_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextScaleCTM_p failed.") );

		CGContextTranslateCTM_p = (CGContextTranslateCTMPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextTranslateCTM") );
		if (CGContextTranslateCTM_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextTranslateCTM_p failed.") );

		CGPointApplyAffineTransform_p = (CGPointApplyAffineTransformPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGPointApplyAffineTransform") );
		if (CGPointApplyAffineTransform_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGPointApplyAffineTransform_p failed.") );

		CGSizeApplyAffineTransform_p = (CGSizeApplyAffineTransformPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGSizeApplyAffineTransform") );
		if (CGSizeApplyAffineTransform_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGSizeApplyAffineTransform_p failed.") );

		CGAffineTransformInvert_p = (CGAffineTransformInvertPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGAffineTransformInvert") );
		if (CGAffineTransformInvert_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGAffineTransformInvert_p failed.") );

		CGContextClearRect_p = (CGContextClearRectPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextClearRect") );
		if (CGContextClearRect_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextClearRect_p failed.") );

		CGContextSaveGState_p = (CGContextSaveGStatePtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextSaveGState") );
		if (CGContextSaveGState_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextSaveGState_p failed.") );

		CGContextRestoreGState_p = (CGContextRestoreGStatePtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextRestoreGState") );
		if (CGContextRestoreGState_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextRestoreGState_p failed.") );

		CGAffineTransformScale_p = (CGAffineTransformScalePtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGAffineTransformScale") );
		if (CGAffineTransformScale_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGAffineTransformScale_p failed.") );

		CGPDFPageGetDrawingTransform_p = (CGPDFPageGetDrawingTransformPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGPDFPageGetDrawingTransform") );
		if (CGPDFPageGetDrawingTransform_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGPDFPageGetDrawingTransform_p failed.") );

		CGColorCreate_p = (CGColorCreatePtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGColorCreate") );
		if (CGColorCreate_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGColorCreate_p failed.") );

		CGPDFPageGetBoxRect_p = (CGPDFPageGetBoxRectPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGPDFPageGetBoxRect") );
		if (CGPDFPageGetBoxRect_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGPDFPageGetBoxRect_p failed.") );

		CGContextSetFlatness_p = (CGContextSetFlatnessPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextSetFlatness") );
		if (CGContextSetFlatness_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextSetFlatness_p failed.") );

		CGContextSetShouldSmoothFonts_p = (CGContextSetShouldSmoothFontsPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextSetShouldSmoothFonts") );
		if (CGContextSetShouldSmoothFonts_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextSetShouldSmoothFonts_p failed.") );

		CGContextSetShouldAntialias_p = (CGContextSetShouldAntialiasPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextSetShouldAntialias") );
		if (CGContextSetShouldAntialias_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextSetShouldAntialias_p failed.") );

		CGContextSetInterpolationQuality_p = (CGContextSetInterpolationQualityPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextSetInterpolationQuality") );
		if (CGContextSetInterpolationQuality_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextSetInterpolationQuality_p failed.") );

		CGContextSetRenderingIntent_p = (CGContextSetRenderingIntentPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextSetRenderingIntent") );
		if (CGContextSetRenderingIntent_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextSetRenderingIntent_p failed.") );

		CGContextClearRect_p = (CGContextClearRectPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextClearRect") );
		if (CGContextClearRect_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextClearRect_p failed.") );

		CGContextRotateCTM_p = (CGContextRotateCTMPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextRotateCTM") );
		if (CGContextRotateCTM_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextRotateCTM_p failed.") );

		CGContextTranslateCTM_p = (CGContextTranslateCTMPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextTranslateCTM") );
		if (CGContextTranslateCTM_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextTranslateCTM_p failed.") );

		CGContextScaleCTM_p = (CGContextScaleCTMPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextScaleCTM") );
		if (CGContextScaleCTM_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextScaleCTM_p failed.") );

		CGContextRestoreGState_p = (CGContextRestoreGStatePtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextRestoreGState") );
		if (CGContextRestoreGState_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextRestoreGState_p failed.") );

		CGContextSaveGState_p = (CGContextSaveGStatePtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextSaveGState") );
		if (CGContextSaveGState_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextSaveGState_p failed.") );

		CGBitmapContextGetBytesPerRow_p = (CGBitmapContextGetBytesPerRowPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGBitmapContextGetBytesPerRow") );
		if (CGBitmapContextGetBytesPerRow_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGBitmapContextGetBytesPerRow_p failed.") );

		CGDataProviderCreateDirectAccess_p = (CGDataProviderCreateDirectAccessPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGDataProviderCreateDirectAccess") );
		if (CGDataProviderCreateDirectAccess_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGDataProviderCreateDirectAccess_p failed.") );

		CGPDFDocumentCreateWithURL_p = (CGPDFDocumentCreateWithURLPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGPDFDocumentCreateWithURL") );
		if (CGPDFDocumentCreateWithURL_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGPDFDocumentCreateWithURL_p failed.") );

		CGContextDrawPDFDocument_p = (CGContextDrawPDFDocumentPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextDrawPDFDocument") );
		if (CGContextDrawPDFDocument_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextDrawPDFDocument_p failed.") );

		CGBitmapContextGetWidth = (CGBitmapContextGetWidthPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGBitmapContextGetWidth") );
		if (CGBitmapContextGetWidth == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGBitmapContextGetWidth failed.") );

		CGBitmapContextGetHeight = (CGBitmapContextGetHeightPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGBitmapContextGetHeight") );
		if (CGBitmapContextGetHeight == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGBitmapContextGetHeight failed.") );

		CGColorSpaceCreateWithPlatformColorSpace = (CGColorSpaceCreateWithPlatformColorSpacePtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGColorSpaceCreateWithPlatformColorSpace") );
		if (CGColorSpaceCreateWithPlatformColorSpace == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGColorSpaceCreateWithPlatformColorSpace failed.") );

		CGBitmapContextCreate = (CGBitmapContextCreatePtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGBitmapContextCreate") );
		if (CGBitmapContextCreate == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGBitmapContextCreate failed.") );

		CGPDFDocumentGetPage = (CGPDFDocumentGetPagePtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGPDFDocumentGetPage") );
		if (CGPDFDocumentGetPage == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGPDFDocumentGetPage failed.") );

		CGColorSpaceCreateDeviceRGB_p = (CGColorSpaceCreateDeviceRGBPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGColorSpaceCreateDeviceRGB") );
		if (CGColorSpaceCreateDeviceRGB_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGColorSpaceCreateDeviceRGB_p failed.") );
		
		CGContextRelease_p = (CGContextReleasePtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextRelease") );
		if (CGContextRelease_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextRelease_p failed.") );
		
		CGPDFPageRelease = (CGPDFPageReleasePtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGPDFPageRelease") );
		if (CGPDFPageRelease == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGPDFPageRelease failed.") );
		
		CGPDFDocumentRelease_p = (CGPDFDocumentReleasePtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGPDFDocumentRelease") );
		if (CGPDFDocumentRelease_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGPDFDocumentRelease_p failed.") );
		
		CGDataProviderCreateWithData_p = (CGDataProviderCreateWithDataPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGDataProviderCreateWithData") );
		if (CGDataProviderCreateWithData_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGDataProviderCreateWithData_p failed.") );
		
		CGPDFDocumentCreateWithProvider_p = (CGPDFDocumentCreateWithProviderPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGPDFDocumentCreateWithProvider") );
		if (CGPDFDocumentCreateWithProvider_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGPDFDocumentCreateWithProvider_p failed.") );
		
		CGDataProviderRelease_p = (CGDataProviderReleasePtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGDataProviderRelease") );
		if (CGDataProviderRelease_p == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGDataProviderRelease_p failed.") );

		CGContextDrawPDFPage = (CGContextDrawPDFPagePtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGContextDrawPDFPage") );
		if (CGContextDrawPDFPage == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGContextDrawPDFPage failed.") );

		CGBitmapContextGetData = (CGBitmapContextGetDataPtr) CFBundleGetFunctionPointerForName( cgBundle, CFSTR("CGBitmapContextGetData") );
		if (CGBitmapContextGetData == NULL)  err = cfragNoSymbolErr;
		if (err == cfragNoSymbolErr) CFShow( CFSTR("Ampede error: CGBitmapContextGetData failed.") );
	}
	
	if (err != noErr) CFShow( CFSTR("Ampede error: LoadCoreGraphicsPointers() failed.") );
	
	return err;
}

CGRect
CGRectMake(float x, float y, float width, float height)
{
    CGRect rect;
    rect.origin.x = x; rect.origin.y = y;
    rect.size.width = width; rect.size.height = height;
    return rect;
}

CGPoint
CGPointMake(float x, float y)
{
    CGPoint p; p.x = x; p.y = y; return p;
}

CGSize
CGSizeMake(float width, float height)
{
    CGSize size; size.width = width; size.height = height; return size;
}

CGAffineTransform
GetDrawingTransformToCenterAndScaleSourceRectInDestinationRect(
		CGRect src,
		CGRect dst,
		int preserveAspectRatio
	)
{
	CGAffineTransform transform = { 1, 0, 0, 1, 0, 0 }; // the identity transform
	CGPoint srcCenter  = CGPointMake( src.origin.x + (src.size.width/2), src.origin.y + (src.size.height/2) );
	CGPoint dstCenter = CGPointMake( dst.origin.x + (dst.size.width/2), dst.origin.y + (dst.size.height/2) );
	float xScale;
	float yScale;
	
	if ( src.size.width != 0.0 ) xScale = dst.size.width / src.size.width; else return transform;
	if ( src.size.height != 0.0 ) yScale = dst.size.height / src.size.height; else return transform;
	
	if ( preserveAspectRatio )
	{
		// first, figure out which dimension of dst is closest to src
		if ( fabs( xScale ) < fabs( yScale ) )
		{
			// width is closest, so we'll scale the width as the "control" scale
			transform.a = xScale;
			transform.d = xScale;
			transform.tx = dstCenter.x - (srcCenter.x * xScale);
			transform.ty = dstCenter.y - (srcCenter.y * xScale);
		}
		else 
		{
			// height is closest, so we'll scale the height as the "control" scale
			transform.a = yScale;
			transform.d = yScale;
			transform.tx = dstCenter.x - (srcCenter.x * yScale);
			transform.ty = dstCenter.y - (srcCenter.y * yScale);
		}
	}
	else
	{
		// scale each dimension independently
		transform.a = xScale;
		transform.d = yScale;
	}
	
	return transform;
}

