//
//  PDFXref.h
//  Ampede
//
//  Created by Eric Ocean on Thu Jul 15 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface PDFXref : NSObject
{
	// This should probably be a standard, malloc'd array, not an NSArray. I'm just not sure how to do that.
	NSArray *toc; // an array of PDFIndirectObject; there is one entry for each possible object number
}

- initWithData:(NSData *)theData;
	// this should completely build the xref, including all previous xref entries
	// it's not necessary to do this for the first version, though, because AI files only have one


@end
