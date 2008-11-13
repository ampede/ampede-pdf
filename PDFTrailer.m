//
//  PDFTrailer.m
//  Ampede
//
//  Created by Eric Ocean on Thu Jul 15 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "PDFTrailer.h"
#import "PDFTrailer-Ragel.h"


@implementation PDFTrailer

- initWithData:(NSData *)theData;
{
	if ( self = [super init] ) {
		NSData *endOfPDF = [theData subdataWithRange:NSMakeRange( [theData length] - 64, 64 )];
			// this gives us the last 64 bytes, enough to determine the location of the pdf's cross-reference table
		
		startxref = [self startxrefFromData:endOfPDF];
		NSLog(@"startxref number is: %d", startxref);
		
		trailer = nil;
	}
	return self;
}

- (NSDictionary *)trailer;
{
	if ( trailer == nil )
	{
		// need to construct the trailer here
		return nil;
	}
	else
		return trailer;
}

@end
