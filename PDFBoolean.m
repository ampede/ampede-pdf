//
//  PDFBoolean.m
//  Ampede
//
//  Created by Eric Ocean on Thu Jul 15 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "PDFBoolean.h"


@implementation PDFBoolean

+ (PDFBoolean *)booleanWithBool:(BOOL)boolValue;
{
	return [[[self alloc] initWithBool:boolValue] autorelease];
}

- initWithBool:(BOOL)boolValue;
{
	value = boolValue;
	return self;
}

- (BOOL)value; { return value; }

@end
