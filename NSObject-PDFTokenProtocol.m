//
//  NSObject-PDFTokenProtocol.m
//  Ampede
//
//  Created by Eric Ocean on Mon Jul 19 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "PDFParsingProtocol.h"
#import "PDFBoolean.h"


@implementation NSObject (PDFTokenProtocol)

- (int)pdfTokenID;
{
	return 0; // not a valid token
}

@end

typedef enum {
	pdfBeginDictionaryID = 1,
	pdfEndDictionaryID = 2,
	pdfBeginArrayID = 3,
	pdfEndArrayID = 4,
	pdfStringOrStreamID = 5,
	pdfNumberID = 6,
	pdfBooleanID = 7,
	pdfNullID = 8,
	pdfBeginStreamID = 9,
	pdfEndStreamID = 10,
	pdfIndirectObjectReferenceID = 11,
	pdfBeginIndirectObjectID = 12,
	pdfEndIndirectObjectID = 13,
	pdfXrefID = 14,
	pdfStartxrefID = 15,
	pdfTrailerID = 16,
	pdfFreeID = 17,
	pdfInUseID = 18,
	pdfNameID = 19
} PDFTokenID;

@implementation NSString (PDFTokenProtocol)

- (int)pdfTokenID; { return pdfNameID; }

@end

@implementation NSDecimalNumber (PDFTokenProtocol)

- (int)pdfTokenID; { return pdfNumberID; }

@end

@implementation PDFBoolean (PDFTokenProtocol)

- (int)pdfTokenID; { return pdfNumberID; }

@end

