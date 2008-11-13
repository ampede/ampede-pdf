//
//  PDFView.m
//  Ampede
//
//  Created by Eric Ocean on Mon Jul 19 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "PDFView.h"
#import "SequenceDocument.h"


@implementation PDFView

+ (void)
initialize
{
	static BOOL classDidInitialize = NO; if ( classDidInitialize ) return; classDidInitialize = YES;
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("+[PDFView initialize] called") );
#endif
    [self setKeys:[NSArray arrayWithObjects:@"wc.document.pdfPath", @"wc.document.currentPage", nil] triggerChangeNotificationsForDependentKey:@"page"];
	
	[self exposeBinding:@"antialias"];
	[self exposeBinding:@"intent"];
	[self exposeBinding:@"quality"];
	[self exposeBinding:@"flatness"];
	[self exposeBinding:@"shouldSmoothFonts"];
	[self exposeBinding:@"pageRotation"];
}

//- (void)
//observeValueForKeyPath:(NSString *)keyPath
//ofObject:(id)object 
//change:(NSDictionary *)change
//context:(void *)context;
//{
//#ifdef DEBUG_CALLING_SEQUENCE
//	CFShow( CFSTR("-[PDFView observeValueForKeyPath:ofObject:change:context:] called") );
//#endif
//    if ( [keyPath isEqualToString:@"wc.document.pdfPath"] || [keyPath isEqualToString:@"wc.document.currentPage"] )
//	{
//		// this is triggered if any of our dependent keys changes--see above
//		[self setPage:[(SequenceDocument *)[wc document] page]];
//	}
//}
//
- initWithFrame:(NSRect)frame
{
    if ( self = [super initWithFrame:frame] ) {
		quality = kCGInterpolationNone; //kCGInterpolationDefault;
		antialias = YES;
		intent = kCGRenderingIntentDefault;
		flatness = 1.0;
		shouldSmoothFonts = YES;
    }
    return self;
}

- (void)
awakeFromNib;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[PDFView awakeFromNib] called") );
#endif
	[self   bind:@"antialias"
			toObject:[wc document]
			withKeyPath:@"antialias"
			options:nil];

	[self   bind:@"intent"
			toObject:[wc document]
			withKeyPath:@"intent"
			options:nil];

	[self   bind:@"quality"
			toObject:[wc document]
			withKeyPath:@"quality"
			options:nil];

	[self   bind:@"flatness"
			toObject:[wc document]
			withKeyPath:@"flatness"
			options:nil];

	[self   bind:@"shouldSmoothFonts"
			toObject:[wc document]
			withKeyPath:@"shouldSmoothFonts"
			options:nil];
			
	[self   bind:@"pageRotation"
			toObject:[wc document]
			withKeyPath:@"pageRotation"
			options:nil];
			
	[self setPage:[(SequenceDocument *)[wc document] page]];
	
//	[self setFrame:[[self enclosingScrollView] documentVisibleRect]];
}

- (void)
drawRect:(NSRect)aRect;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[PDFView drawRect:] called") );
#endif
	CGRect cgRect = NSRectToCGRect( [self bounds] );
	
	CGContextRef gContext = [[NSGraphicsContext currentContext] graphicsPort];

	CGContextSaveGState( gContext );
	CGContextSetInterpolationQuality( gContext, quality );
	CGContextSetShouldAntialias( gContext, antialias );
	CGContextSetRenderingIntent( gContext, intent );
	CGContextSetShouldSmoothFonts( gContext, shouldSmoothFonts );
	CGContextSetFlatness( gContext, flatness );
	
	CGRect boxRect = CGPDFPageGetBoxRect(page, kCGPDFMediaBox);

	CGAffineTransform myTransform = GetDrawingTransformToCenterAndScaleSourceRectInDestinationRect(	boxRect, cgRect, true );
	
	CGContextClearRect( gContext, cgRect );
	CGContextConcatCTM( gContext, myTransform );
		
	CGContextClipToRect ( gContext, boxRect );
	NSEraseRect( CGRectToNSRect( boxRect ) );
	CGContextDrawPDFPage ( gContext, page );
	
	CGContextRestoreGState( gContext );
}

- (void)
setPage:(CGPDFPageRef)thePage;
{
	page = thePage;
	[self setNeedsDisplay:YES];
}

- (void)
setPageRotation:(int)theRotation;
{
	pageRotation = theRotation;
	[self setNeedsDisplay:YES];
}

- (void)setFlatness:(float)f;
{
	flatness = f;
	[self setNeedsDisplay:YES];
}

- (void)setAntialias:(BOOL)yn;
{
	antialias = yn;
	[self setNeedsDisplay:YES];
}

- (void)setShouldSmoothFonts:(BOOL)yn;
{
	shouldSmoothFonts = yn;
	[self setNeedsDisplay:YES];
}

@end

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

