//
//  PDFToken.h
//  Ampede
//
//  Created by Eric Ocean on Fri Jul 16 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import <Cocoa/Cocoa.h>

// This class represents tokens that are not objects themselves, but serve to delimit actual objects

typedef enum {
	arrayBegin = 0,
	arrayEnd,
	dictionaryBegin,
	dictionaryEnd,
	indirectObjectBegin,
	indirectObjectEnd,
	objectReference,
	pdfNull,
	streamBegin,
	streamEnd,
	xref,
	trailer,
	startxref,
	pdfFree,
	inUse
} PDFTokenType;

@interface PDFToken : NSObject
{
	PDFTokenType type;
}

+ (id)pdfNull;
+ (id)objectReference;
+ (id)beginIndirectObject;
+ (id)endIndirectObject;
+ (id)beginArray;
+ (id)endArray;
+ (id)beginDict;
+ (id)endDict;
+ (id)beginStream;
+ (id)endStream;
+ (id)xref;
+ (id)trailer;
+ (id)startxref;
+ (id)trailer;
+ (id)startxref;
+ (id)pdfFree;
+ (id)inUse;

- initWithTokenType:(PDFTokenType)theType;

@end
