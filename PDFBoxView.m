//
//  PDFBoxView.m
//  Ampede
//
//  Created by Eric Ocean on 8/8/04.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "PDFBoxView.h"


@implementation PDFBoxView

+ (void)
initialize
{
	static BOOL classDidInitialize = NO; if ( classDidInitialize ) return; classDidInitialize = YES;
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("+[PDFBoxView initialize] called") );
#endif
	[self exposeBinding:@"boxType"];
	[self exposeBinding:@"shouldDrawBackground"];
	[self exposeBinding:@"backgroundColor"];
}

- initWithFrame:(NSRect)frame
{
    if ( self = [super initWithFrame:frame] ) {
		boxType = 0;
		shouldDrawBackground = 1;
		[self setBackgroundColor:[NSColor whiteColor]];
    }
    return self;
}

- (void)
awakeFromNib;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[PDFBoxView awakeFromNib] called") );
#endif
	[self   bind:@"boxType"
			toObject:[wc document]
			withKeyPath:@"boxType"
			options:nil];

	[self   bind:@"shouldDrawBackground"
			toObject:[wc document]
			withKeyPath:@"shouldDrawBackground"
			options:nil];
	
	[self   bind:@"backgroundColor"
			toObject:[wc document]
			withKeyPath:@"backgroundColor"
			options:nil];
	
	[self   bind:@"shouldPreserveAspectRatio"
			toObject:[wc document]
			withKeyPath:@"shouldPreserveAspectRatio"
			options:nil];
	
	[self   bind:@"shouldColorManage"
			toObject:[wc document]
			withKeyPath:@"shouldColorManage"
			options:nil];
	
	[super awakeFromNib];
}

- (void)
setShouldColorManage:(int)yn;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[PDFBoxView setShouldColorManage:] called") );
#endif
	shouldColorManage = yn;
	[self setNeedsDisplay:YES];
}

- (void)
setBoxType:(int)theBox;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[PDFBoxView setBoxType:] called") );
#endif
	boxType = theBox;
	[self setNeedsDisplay:YES];
}

- (void)
setShouldDrawBackground:(int)yn;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[PDFBoxView setShouldDrawBackground:] called") );
#endif
	shouldDrawBackground = yn;
	[self setNeedsDisplay:YES];
}

- (void)
setBackgroundColor:(NSColor *)theColor;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[PDFBoxView setBackgroundColor:] called") );
#endif
	[theColor retain];
	[backgroundColor autorelease];
	backgroundColor = theColor;
	[self setNeedsDisplay:YES];
}

- (void)
setShouldPreserveAspectRatio:(int)yn;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[PDFBoxView setShouldPreserveAspectRatio:] called") );
#endif
	shouldPreserveAspectRatio = yn;
	[self setNeedsDisplay:YES];
}

void
DrawCell ( void * info, CGContextRef context )
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("DrawCell called") );
#endif
	CGRect ur = { {20, 20}, {20, 20} };
	CGRect ll = { {0, 0}, {20, 20} };
	const float dgA[4] = { 0.5411764705882353, 0.5411764705882353, 0.5411764705882353, 1.0 };
	
	CGContextSetFillColor( context, dgA );
	CGContextFillRect( context, ur );
	CGContextFillRect( context, ll );
}

- (void)
drawRect:(NSRect)aRect;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[PDFBoxView drawRect:] called") );
#endif
	CGRect cgRect = NSRectToCGRect( [self bounds] );
	
	CGContextRef gContext = [[NSGraphicsContext currentContext] graphicsPort];

	CGContextSaveGState( gContext );
	CGContextSetInterpolationQuality( gContext, quality );
	CGContextSetShouldAntialias( gContext, antialias );
	CGContextSetRenderingIntent( gContext, intent );
	CGContextSetShouldSmoothFonts( gContext, shouldSmoothFonts );
	CGContextSetFlatness( gContext, flatness );
	
	CGRect mediaBox = CGPDFPageGetBoxRect(page, kCGPDFMediaBox);;
	CGAffineTransform myTransform;
	
	if ( shouldPreserveAspectRatio ) myTransform = GetDrawingTransformToCenterAndScaleSourceRectInDestinationRect(	mediaBox, cgRect, true );
	else myTransform = GetDrawingTransformToCenterAndScaleSourceRectInDestinationRect(	mediaBox, cgRect, false );
	
	CGRect boxRect;
	if ( boxType == -1 )
	{
		CGPoint pt = CGPointApplyAffineTransform( CGPointMake( cgRect.origin.x, cgRect.origin.y ), CGAffineTransformInvert( myTransform ) );
		CGSize sz = CGSizeApplyAffineTransform( CGSizeMake( cgRect.size.width, cgRect.size.height ), CGAffineTransformInvert( myTransform ) );
		boxRect = CGRectMake( pt.x, pt.y, sz.width, sz.height );
	}
	else boxRect = CGPDFPageGetBoxRect(page, boxType);

	[[NSColor colorWithDeviceRed:0.7372549019607844 green:0.7372549019607844 blue:0.7372549019607844 alpha:1.0 ] set];
	NSRectFill( CGRectToNSRect( cgRect ) );
	
	// draw background pattern
	CGPatternCallbacks cb = { 0, &DrawCell, NULL };
	
	CGPatternRef pattern =  CGPatternCreate (	NULL,
												CGRectMake( 0, 0, 40, 40 ),
												CGAffineTransformMake( 1, 0, 0, 1, 0, 0 ), // identity matrix
												40,
												40,
												kCGPatternTilingConstantSpacing,
												true,
												&cb );
	CGColorSpaceRef fs = CGColorSpaceCreatePattern( NULL );
	
	float alpha = 1.0;
	CGContextSetFillColorSpace( gContext, fs );
	CGContextSetFillPattern( gContext, pattern, &alpha );
	CGContextFillRect( gContext, cgRect );
	
	CGColorSpaceRelease( fs );
	CGPatternRelease( pattern );
	
	// draw pdf page
	CGContextConcatCTM( gContext, myTransform );
	
//	CGColorSpaceRef colorSpace;
//
//	if ( shouldColorManage )
//	{
//		CMProfileRef aProfile;
//
//		CMGetDefaultProfileBySpace(cmRGBData, &aProfile);
//		if ( aProfile == NULL ) return PF_Err_OUT_OF_MEMORY;
//		
//		colorSpace = CGColorSpaceCreateWithPlatformColorSpace( aProfile );
//
//		CMCloseProfile( aProfile );
//	}
//	else colorSpace = CGColorSpaceCreateDeviceRGB_p();
	
	

	if ( shouldDrawBackground )
	{
		CGContextClipToRect ( gContext, boxRect );
		[backgroundColor set];
		NSRectFill( CGRectToNSRect( boxRect ) );
	}
	
	CGContextDrawPDFPage ( gContext, page );

	CGContextRestoreGState( gContext ); // 6
}

@end
