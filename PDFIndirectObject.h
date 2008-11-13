//
//  PDFIndirectObject.h
//  Ampede
//
//  Created by Eric Ocean on Thu Jul 15 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface PDFIndirectObject : NSObject
{
	NSString *tocEntry; // i.e. the 20 bytes in the xref
	int number;
	int generation;
	id value;
	char status; // either 'n', in-use, or 'f' freed
}

@end
