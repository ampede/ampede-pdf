//
//  PDFLexer.h
//  Ampede
//
//  Created by Eric Ocean on Thu Jul 15 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "PDFLexer-Ragel.h"


@interface PDFLexer : NSObject 
{
	@public // for speed
	int loc; // the current byte location in the data; this is updated by the state machine
	int line; // the current line in the data; this is updated by the state machine
	int errorLoc; // these are set upon entering the error state to facilitate better error messages
	int errorLine;
	
	@private
	NSData *data;
	NSMutableArray *tokenArray;
	int nextTokenIndex;
	
	void *buf;
	int buffer_size;
	int machineState;
	BOOL moreData;
	struct PDFParsingMachine *fsm;
}

// designated initializer
- initWithData:(NSData *)theData;

// nextTokenWithError: is the main method. It returns the next next token from the data passed to the parser.
// If it returns nil, and error is nil, there are no more tokens to return. If error is non-nil, there
// was a lexer error. See the NSError object for the error ( e.g. [error localizedDescription] );
// The object returned is autoreleased.
// If an error is returned, the caller is responsible for releasing it. (error is not autoreleased)
//
// If you send incomplete PDF data, and you parse to the end of the data, you may receive a final error object
// if the machine did not end in an accepting state. If the data was supposed to contain a complete file,
// you'll want to notify the user. Otherwise, just release the error object and ignore the error.
// In general, it's more efficient to stop parsing when you've received all the tokens you need.

- (id)nextTokenWithError:(NSError **)error;
	// if the token is an NSError object, it's sent a retain message, assigned by reference to error, and nil is returned.
	// otherwise, the token is returned unmodifed to the caller and error remains set to nil.
	// if there are no more objects, return nil, leaving error unmodified.

/////////////// private API /////////////////

- (void)addToken:(id)theToken;
	// This is called by the machine to append a newly discovered token.
	// 
	// When an NSError token is added, it should contain the byte and line number from where the error occurred
	// in the error object's user-info dictionary.
	
- (id)nextTokenForLexer:(PDFLexer *)lexer;
	// note: this method verifies that lexer == self. It should only be called by the lexer object.
	// this should return nil if there are no more tokens in the buffer, and there were no more tokens after a call to
	// bufferTokens;
	
- (void)bufferTokens;
	// this buffers more tokens, if there are any
	// if the machine doesn't produce any more tokens and the machine was in a non-accepting state or an error state
	// this method constructs an error object to that affect and places it in the buffer
	// if the machine ended normally, then it does nothing

@end
