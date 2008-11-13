//
//  EFTextField.h
//  Ampede
//
//  Created by Eric Ocean on Thu Aug 5 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "EFTextField.h"


@implementation EFTextField

- initWithCoder:(NSCoder *)coder;
{
    if ( self = [super initWithCoder: coder] ) {
		shouldDrawDisabledText = YES;
	}
    return self;
}

- (void)
setEnabled:(BOOL)yn;
{
	NSColor *color;

	shouldDrawDisabledText = (yn) ? NO : YES;
	
	if ( shouldDrawDisabledText ) color = [NSColor disabledControlTextColor];
	else color = [NSColor controlTextColor];

	[self setTextColor:color];
}

- (BOOL)
isEnabled;
{
	return ( shouldDrawDisabledText ) ? NO : YES;
}

@end

