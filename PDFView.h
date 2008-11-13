//
//  PDFView.h
//  Ampede
//
//  Created by Eric Ocean on Mon Jul 19 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SequenceWindowController.h"


@interface PDFView : NSView
{
	CGPDFPageRef page;
	BOOL antialias;
	CGColorRenderingIntent intent;
	CGInterpolationQuality quality;
	float flatness;
	BOOL shouldSmoothFonts;
	int pageRotation;

	IBOutlet SequenceWindowController *wc;
}

- (void)setPage:(CGPDFPageRef)thePage;

@end

CGAffineTransform
GetDrawingTransformToCenterAndScaleSourceRectInDestinationRect(
		CGRect sourceRect,
		CGRect destinationRect,
		int preserveAspectRatio
	);
	
#define NSRectToCGRect( aRect ) CGRectMake(aRect.origin.x, aRect.origin.y, aRect.size.width, aRect.size.height)
#define CGRectToNSRect( aRect ) NSMakeRect(aRect.origin.x, aRect.origin.y, aRect.size.width, aRect.size.height)


