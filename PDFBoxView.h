//
//  PDFBoxView.h
//  Ampede
//
//  Created by Eric Ocean on 8/8/04.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import"PDFView.h"


@interface PDFBoxView : PDFView
{
	int boxType;
	int shouldDrawBackground;
	NSColor *backgroundColor;
	int shouldPreserveAspectRatio;
	int shouldColorManage;
}

- (void)setBackgroundColor:(NSColor *)theColor;

@end
