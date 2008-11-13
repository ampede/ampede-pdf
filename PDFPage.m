//
//  PDFPage.m
//  Ampede
//
//  Created by Eric Ocean on Thu Aug 05 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "PDFPage.h"


@implementation PDFPage

- initWithPage:(CGPDFPageRef)thePage;
{
	if ( self = [super init] ) {
		page = CGPDFPageRetain( thePage );
	}
	return self;
}

- (void)
dealloc;
{
	if (page) CFRelease( page ); page = NULL;
	[super dealloc];
}

- (CGPDFPageRef)
page;
{
	return page;
}

@end
