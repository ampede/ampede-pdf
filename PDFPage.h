//
//  PDFPage.h
//  Ampede
//
//  Created by Eric Ocean on Thu Aug 05 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import <Cocoa/Cocoa.h>


// a simple wrapper

@interface PDFPage : NSObject
{
	CGPDFPageRef page;
}

- initWithPage:(CGPDFPageRef)thePage;

- (CGPDFPageRef)page;

@end
