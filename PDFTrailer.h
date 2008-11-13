//
//  PDFTrailer.h
//  Ampede
//
//  Created by Eric Ocean on Thu Jul 15 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface PDFTrailer : NSObject
{
	NSDictionary *trailer;
	int startxref;
}

- initWithData:(NSData *)theData;

@end
