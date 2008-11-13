//
//  PDFParser.h
//  Ampede
//
//  Created by Eric Ocean on Thu Jul 15 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "PDFParsingProtocol.h"


// This class is responsible for parsing PDF data. The data need not be complete.
// It interacts principally with PDFLexer and objects that conform to the PDFParsing protocol.
// It defines specialized methods to parse different aspects of a PDF file, such as the cross-reference table.
// Usually, you will use it to parse an indirect object.
//
// It it always the caller's repsonsibility to insure that the NSData passed contains enough data to complete the
// desired parse, and that the data starts at the correct position to begin that parse.
// It is acceptable to send too much data; the parser will only use as much as it needs.
//
// All parsing methods return nil on error.

@class PDFLexer, PDFIndirectObject, PDFStreamObject, PDFTrailer, PDFXref, PDFDocument;

@interface PDFParser : NSObject
{
	NSData *data;
	PDFLexer *lexer;
	NSMutableArray *stack;
	PDFDocument *doc;
}

// convenience methods
+ (PDFIndirectObject *)parseIndirectObjectFromData:(NSData *)theData;
+ (PDFStreamObject *)parseStreamDataFromData:(NSData *)theData;

+ (NSString *)parseHeaderFromData:(NSData *)theData;
+ (PDFTrailer *)parseTrailerFromData:(NSData *)theData;
+ (PDFXref *)parseXrefFromData:(NSData *)theData;

// designated initializer
- initWithData:(NSData *)theData forDocument:(PDFDocument *)pdfDoc;

// These methods determine what kind of parsing object PDFParser will instantiate to handle the parse
// That object becomes the first (and initially the only) object on the parsing stack.
- (PDFIndirectObject *)parseIndirectObject;
- (PDFStreamObject *)parseStreamData;
		// this allows you to parse just the data stream of a stream object
		// it's useful when you already have the stream, and just want to do something with it, such as transforming it
		
- (NSString *)parseHeader;
- (PDFTrailer *)parseTrailer;
- (PDFXref *)parseXref;

// PDFParser manages a stack of objects implementing the PDFParsing protocol.
// The object on the top is the current parser, and is passed to the lexer to receive callbacks for the next token.
// A PDFLexer object is only called directly by PDFParser, which manages the progress of the parse.
- (id <PDFParsing>)top;
- (id <PDFParsing>)pop;
- (void)push:(id <PDFParsing>)theParser;

@end


