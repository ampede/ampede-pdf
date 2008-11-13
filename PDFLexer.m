//
//  PDFLexer.m
//  Ampede
//
//  Created by Eric Ocean on Thu Jul 15 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "PDFLexer.h"

static NSString *PDFLexerDomain = @"PDFLexerDomain";

#define BUFFER_STRIDE 4096

@implementation PDFLexer

//+ (void)
//initialize;
//{
//	static BOOL hasBeenInitialized = NO;
//	if ( hasBeenInitialized ) return;
//	hasBeenInitialized = YES;
//	
//}

// designated initializer

- initWithData:(NSData *)theData;
{
	if ( self = [super init] ) {
		data = [theData retain];
		loc = 0;
		line = 1;
		errorLoc = loc;
		errorLine = line;
		
		tokenArray = [[NSMutableArray alloc] initWithCapacity:128];
		nextTokenIndex = -1;
		
		fsm = malloc( sizeof( struct PDFParsingMachine ) );
		buf = (void *)[data bytes];
		buffer_size = [data length];
		PDFParsingMachine_init( fsm );
		fsm->self = self;
		moreData = YES;
	}
	return self;
}

- (void)dealloc;
{
	// free( fsm );
	[data release]; data = nil;
	[tokenArray release]; tokenArray = nil;
	[super dealloc];
}

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

- (id)
nextTokenWithError:(NSError **)error;
	// if the token is an NSError object, it's sent a retain message, assigned by reference to error, and nil is returned.
	// otherwise, the token is returned unmodifed to the caller and error remains set to nil.
	// if there are no more object, return nil, leaving error unmodified.
{
	id token = [self nextTokenForLexer:self];
	
	if ( [token isKindOfClass:[NSError class]] )
	{
		*error = [token retain];
		return nil;
	}
	else *error = nil;
	
	return token;
}

/////////////// private API /////////////////

- (void)
addToken:(id)theToken;
	// This is called by the machine to append a newly discovered token.
	// 
	// When an NSError token is added, it should contain the byte and line number from where the error occurred
	// in the error object's user-info dictionary.
{
	// NS_DURING
	[tokenArray addObject:theToken];
//	NS_HANDLER
//	[tokenArray addObject:
//		[NSError
//			errorWithDomain:PDFLexerDomain
//			code:0 // unused
//			userInfo:[NSDictionary
//						dictionaryWithObject:@"PDFLexer error: state machine tried to add a nil object to the token array"
//						forKey:NSLocalizedDescriptionKey]]];
//	NS_ENDHANDLER
}

- (id)
nextTokenForLexer:(PDFLexer *)lexer;
	// note: this method verifies that lexer == self. It should only be called by the lexer object.
	// this should return nil if there are no more tokens in the buffer, and there were no more tokens after a call to
	// bufferTokens;
{
	id token = nil;
	
	if ( self != lexer ) NSLog(@"-[PDFLexer nextTokenForLexer:] must be called by lexer == self");
	else if ( nextTokenIndex < [tokenArray count] )
	{
		token = [tokenArray objectAtIndex:nextTokenIndex];
		nextTokenIndex++;
	}
	else
	{
		[self bufferTokens];
		if ( nextTokenIndex < [tokenArray count] )
		{
			token = [tokenArray objectAtIndex:nextTokenIndex];
			nextTokenIndex++;
		}
	}
	
	return token;
}

- (void)
bufferTokens;
	// this buffers more tokens, if there are any
	// if the machine doesn't produce any more tokens and the machine was in a non-accepting state or an error state
	// this method constructs an error object to that affect and places it in the buffer
	// if the machine ended normally, then it does nothing
{
	[tokenArray removeAllObjects];
	nextTokenIndex = -1;
	
//	NSLog(@"%@", self);
//	NSLog(@"%d", loc);
//	NSLog(@"%@", tokenArray);
	
	if ( machineState != -1 && moreData )
	// we're not in an error state and there is more data to parse
	{
		do {
			// if there's only enough data for one more parse, goto finalBuf
			if ( buffer_size - loc <= BUFFER_STRIDE ) goto finalBuf;
			
			machineState = PDFParsingMachine_execute( fsm, buf + loc, BUFFER_STRIDE );
			loc += BUFFER_STRIDE;
			
			// check for error state
			if ( machineState == -1 )
			{
				// machine is in an error state and can never accept
				[tokenArray addObject:
					[NSError
						errorWithDomain:PDFLexerDomain
						code:0 // unused
						userInfo:[NSDictionary
									dictionaryWithObject:
										[NSString
											stringWithFormat:@"PDFLexer error: state machine is in an error state at line: %d byte: %d",
												errorLine, errorLoc]
									forKey:NSLocalizedDescriptionKey]]];
				nextTokenIndex = 0;
return; // <------------------------- EARLY EXIT ON ERROR ----------------------///////////
			}
		} while ( [tokenArray count] == 0 ); // keep lexing until we accept at least one token
		
		// this is only reached if a token is found before the final buffer is lexed (see below)
		nextTokenIndex = 0;
	}
	return;
	
finalBuf:
	
	if ( buffer_size == loc )
	{
		// there's no more data in the buffer
		machineState = PDFParsingMachine_finish( fsm );
	}
	else
	{
		machineState = PDFParsingMachine_execute( fsm, buf + loc, buffer_size - loc );
	}
	
	machineState = PDFParsingMachine_finish( fsm );
	
	// check for error state
	if ( machineState == -1 )
	{
		// machine is in an error state and can never accept
		// check to see if we had an error **before** the end of the data
		if ( errorLoc != 0 && errorLoc != loc )
		{
			[tokenArray addObject:
				[NSError
					errorWithDomain:PDFLexerDomain
					code:0 // unused
					userInfo:[NSDictionary
								dictionaryWithObject:
									[NSString
										stringWithFormat:@"PDFLexer error: state machine is in an error state at line: %d byte: %d",
											errorLine, errorLoc]
								forKey:NSLocalizedDescriptionKey]]];
			nextTokenIndex = 0;
		}
		else
		{
			// machine was in an error state at EOF
			moreData = NO;
			[tokenArray addObject:
				[NSError
					errorWithDomain:PDFLexerDomain
					code:0 // unused
					userInfo:[NSDictionary
								dictionaryWithObject:@"PDFLexer error: state machine ended in an error state."
								forKey:NSLocalizedDescriptionKey]]];
			nextTokenIndex = 0;
		}
	}
	else if ( machineState == 0 )
	{
		// machine was in a non-accepting state at EOF
		moreData = NO;
		[tokenArray addObject:
			[NSError
				errorWithDomain:PDFLexerDomain
				code:0 // unused
				userInfo:[NSDictionary
							dictionaryWithObject:@"PDFLexer error: state machine ended in a non-accepting state."
							forKey:NSLocalizedDescriptionKey]]];
		nextTokenIndex = 0;
		return;
	}
	else // machineState == 1
	{
		// this is necessary if the token finished at EOF, apparently
		if ( [tokenArray count] != 0 ) nextTokenIndex = 0;
		// machine was in an accepting state at EOF
		moreData = NO;
		NSLog(@"PDFLexer consumed all of the data without incident, ending at an accepting state.");
	}
}

@end

unsigned char
hexToUChar( unsigned char hex1, unsigned char hex2 )
{
	// hex1 and hex2 are guaranteed to be [0-9a-fA-Z]
	unsigned char hexByte = 0;
	
	if ( hex1 < 'A' ) hexByte += ( hex1 - 48 ) << 4;		// hex is an ASCII decimal number
	else if ( hex1 < 'a' ) hexByte += ( hex1 - 55 ) << 4;	// hex is an UPPERCASE ASCII alpha
	else hexByte += ( hex1 - 87) << 4;						// hex is a lowercase ASCII alpha

	if ( hex2 < 'A' ) hexByte += ( hex2 - 48 );			// hex is an ASCII decimal number
	else if ( hex2 < 'a' ) hexByte += ( hex2 - 55 );	// hex is an UPPERCASE ASCII alpha
	else hexByte += ( hex2 - 87);						// hex is a lowercase ASCII alpha
	
	return hexByte;
}

unsigned char
octalToUChar( unsigned char octal1, unsigned char octal2, unsigned char octal3 )
{
	// octal1, octal2, and octal3 are guaranteed to be [0-7]
	// the resulting conversion may overflow, though, in which case the value is clamped to 255
	unsigned int octalInt = 0; // won't overflow
	
	octalInt = ((octal1 - 48) * 64) + ((octal2 - 48) * 8) + (octal3 - 48);
	
	if ( octalInt > 255 ) return 255;
	else return octalInt; // converted to uchar on return
}


