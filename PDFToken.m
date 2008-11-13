//
//  PDFToken.m
//  Ampede
//
//  Created by Eric Ocean on Fri Jul 16 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "PDFToken.h"


@implementation PDFToken

+ (id)pdfNull;
{
	return [[[self alloc] initWithTokenType:pdfNull] autorelease];
}

+ (id)objectReference;
{
	return [[[self alloc] initWithTokenType:objectReference] autorelease];
}

+ (id)beginIndirectObject;
{
	return [[[self alloc] initWithTokenType:indirectObjectBegin] autorelease];
}

+ (id)endIndirectObject;
{
	return [[[self alloc] initWithTokenType:indirectObjectEnd] autorelease];
}

+ (id)beginArray;
{
	return [[[self alloc] initWithTokenType:arrayBegin] autorelease];
}

+ (id)endArray;
{
	return [[[self alloc] initWithTokenType:arrayEnd] autorelease];
}

+ (id)beginDict;
{
	return [[[self alloc] initWithTokenType:dictionaryBegin] autorelease];
}

+ (id)endDict;
{
	return [[[self alloc] initWithTokenType:dictionaryEnd] autorelease];
}

+ (id)beginStream;
{
	return [[[self alloc] initWithTokenType:streamBegin] autorelease];
}

+ (id)endStream;
{
	return [[[self alloc] initWithTokenType:streamEnd] autorelease];
}

+ (id)xref;
{
	return [[[self alloc] initWithTokenType:xref] autorelease];
}

+ (id)trailer;
{
	return [[[self alloc] initWithTokenType:trailer] autorelease];
}

+ (id)startxref;
{
	return [[[self alloc] initWithTokenType:startxref] autorelease];
}

+ (id)pdfFree;
{
	return [[[self alloc] initWithTokenType:pdfFree] autorelease];
}

+ (id)inUse;
{
	return [[[self alloc] initWithTokenType:inUse] autorelease];
}

- initWithTokenType:(PDFTokenType)theType;
{
	if ( self = [super init] ) {
		type = theType;
	}
	return self;
}

- (NSString *)description;
{
	NSString *typeString;
	
	switch ( type ) {
	case arrayBegin: { typeString = @"arrayBegin"; break; }
	case arrayEnd: { typeString = @"arrayEnd"; break; }
	case dictionaryBegin: { typeString = @"dictionaryBegin"; break; }
	case dictionaryEnd: { typeString = @"dictionaryEnd"; break; }
	case indirectObjectBegin: { typeString = @"indirectObjectBegin"; break; }
	case indirectObjectEnd: { typeString = @"indirectObjectEnd"; break; }
	case objectReference: { typeString = @"objectReference"; break; }
	case pdfNull: { typeString = @"pdfNull"; break; }
	case streamBegin: { typeString = @"streamBegin"; break; }
	case streamEnd: { typeString = @"streamEnd"; break; }
	case xref: { typeString = @"xref"; break; }
	case trailer: { typeString = @"trailer"; break; }
	case startxref: { typeString = @"startxref"; break; }
	case pdfFree: { typeString = @"pdfFree"; break; }
	case inUse: { typeString = @"inUse"; break; }
	}
	
	return [NSString stringWithFormat:@"%@ token", typeString];
}

@end

