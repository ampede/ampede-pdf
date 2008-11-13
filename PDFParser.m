//
//  PDFParser.m
//  Ampede
//
//  Created by Eric Ocean on Thu Jul 15 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "PDFParser.h"
#import "PDFLexer.h"
#import "PDFDocument.h"



@implementation PDFParser

// designated initializer
- initWithData:(NSData *)theData forDocument:(PDFDocument *)pdfDoc;
{
	if ( self = [super init] ) {
		data = [theData retain];
		lexer = [[PDFLexer alloc] initWithData:data];
		stack = [[NSMutableArray alloc] initWithCapacity:10];
		doc = pdfDoc; // weak retain
	}
	return self;
}

- (void)
dealloc;
{
	[data release]; date = nil;
	[lexer release]; lexer = nil;
	[stack release]; stack = nil;
	[super dealloc];
}

@end
