//
//  PDFObjectBuilder.m
//  Ampede
//
//  Created by Eric Ocean on Thu Jul 15 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "PDFObjectBuilder.h"
#import "PDFParsingProtocolDebug.h"


@implementation PDFObjectBuilder

- (void)
handleParsedNameToken:(NSString *)theName;
{
	LOG_PDFPARSING_CALLBACKS_IMP
}

- (void)
handleParsedIndirectObjectLabelToken:(PDFIndirectObjectStruct)theStruct;
{
	LOG_PDFPARSING_CALLBACKS_IMP
}

- (void)
handleParsedObjectReferenceToken:(PDFIndirectObjectStruct)theStruct;
{
	LOG_PDFPARSING_CALLBACKS_IMP
}

- (void)
handleParsedStringToken:(NSData *)theString;
{
	LOG_PDFPARSING_CALLBACKS_IMP
}

- (void)
handleParsedNumberToken:(NSDecimalNumber *)theNumber;
{
	LOG_PDFPARSING_CALLBACKS_IMP
}

- (void)
handleParsedBooleanToken:(PDFBoolean *)theBoolean;
{
	LOG_PDFPARSING_CALLBACKS_IMP
}

- (void)
handleParsedObjectBeginToken;
{
	LOG_PDFPARSING_CALLBACKS_IMP
}

- (void)
handleParsedObjectEndToken;
{
	LOG_PDFPARSING_CALLBACKS_IMP
}

- (void)
handleParsedArrayBeginToken;
{
	LOG_PDFPARSING_CALLBACKS_IMP
}

- (void)
handleParsedArrayEndToken;
{
	LOG_PDFPARSING_CALLBACKS_IMP
}

- (void)
handleParsedDictionaryBeginToken;
{
	LOG_PDFPARSING_CALLBACKS_IMP
}

- (void)
handleParsedDictionaryEndToken;
{
	LOG_PDFPARSING_CALLBACKS_IMP
}

- (void)
handleParsedStreamBeginToken;
{
	LOG_PDFPARSING_CALLBACKS_IMP
}

- (void)
handleParsedStreamEndToken;
{
	LOG_PDFPARSING_CALLBACKS_IMP
}

@end
