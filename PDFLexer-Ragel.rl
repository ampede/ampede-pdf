//
//  PDFLexer-Ragel.rl
//  Ampede
//
//  Created by Eric Ocean on Thu Jul 15 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import <Cocoa/Cocoa.h>


unsigned char hexToUChar( unsigned char hex1, unsigned char hex2 );
unsigned char octalToUChar( unsigned char octal1, unsigned char octal2, unsigned char octal3 );

@class PDFLexer;

%% PDFParsingMachine
	alphtype unsigned char;
	
	#*************************************************************************************************************#
	# global information #
	
	struct {
		PDFLexer *self;
	};
	
	#*************************************************************************************************************#
	# the whitespace, eol, delimiter, regular, and comment machines #
	
	#*************************************************************************************************************#
	# the boolean machine #
	
	struct {
		BOOL boolVal;
	};
	
	#*************************************************************************************************************#
	# the number machine #
	
	struct {
		NSMutableData *numChars;
		unsigned char nullByte;
	};
	
	#*************************************************************************************************************#
	# the hexString machine #
	
	struct {
		NSMutableData *hexChars;
		unsigned char hexByte;
		unsigned char hexChar1;
		unsigned char hexChar2;
		BOOL shouldSetFirstHexChar;
	};
	
	#*************************************************************************************************************#
	# the name machine #

	struct {
		NSMutableData *nameChars;
		unsigned char nameHexByte;
		unsigned char nameHexChar1;
	};
		
	#*************************************************************************************************************#
	# the literalString machine #
	
	struct {
		NSMutableData *literalChars;
		unsigned char charBuf;
		unsigned char octalByte;
		unsigned char octal1;
		unsigned char octal2;
		unsigned char octal3;
		int octalCharToBuf;
		unsigned char appendByte;
	};
	
	struct {
		int leftParenCount;
	};

	#*************************************************************************************************************#
	# the array machines #

	#*************************************************************************************************************#
	# the dictionary and pdfNull machines #
		
	#*************************************************************************************************************#
	# the stream machine #
	
	struct {
		NSMutableData *streamBytes;
	};
	
	#*************************************************************************************************************#
	# the indirectObject machines #
	
	#*************************************************************************************************************#
	# the indirectObjectReference machine #
	
	#*************************************************************************************************************#
	# the PDFParsingMachine machine #
	
%%
