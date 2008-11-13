//
//  PDFParsingProtocol.h
//  Ampede
//
//  Created by Eric Ocean on Thu Jul 15 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import <Cocoa/Cocoa.h>


typedef struct {
	int number;
	int generation;
} PDFIndirectObjectStruct;

// These are the callbacks that objects managed by the PDFParser class (and used by the PDFLexer) must manage.
// This is not an informal protocol because I want each parsing object to handle the full range of problems that can
// occur during a parse and give appropriate, context sensitive error messages when things go awry.

@class PDFBoolean, PDFStream;

@protocol PDFParsing

- (void)handleParsedNameToken:(NSString *)theName;
- (void)handleParsedIndirectObjectLabelToken:(PDFIndirectObjectStruct)theStruct;
- (void)handleParsedObjectReferenceToken:(PDFIndirectObjectStruct)theStruct;
- (void)handleParsedStringToken:(NSData *)theString;
- (void)handleParsedNumberToken:(NSDecimalNumber *)theNumber;
- (void)handleParsedBooleanToken:(PDFBoolean *)theBoolean;
- (void)handleParsedObjectBeginToken;
- (void)handleParsedObjectEndToken;
- (void)handleParsedArrayBeginToken;
- (void)handleParsedArrayEndToken;
- (void)handleParsedDictionaryBeginToken;
- (void)handleParsedDictionaryEndToken;
- (void)handleParsedStreamBeginToken;
- (void)handleParsedStreamEndToken;

@end


// These are callbacks that are only of interest to stream parsers.

@protocol PDFStreamParsing < PDFParsing >

@end


// parsers use the return value to determine what kind of token they're dealing with
// class-based wouldn't work, because I haven't defined unique classes for each token type

@interface NSObject (PDFTokenProcotol)

- (int)pdfTokenID;

@end
