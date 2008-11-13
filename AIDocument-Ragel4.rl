//
//  AIDocument-Ragel4.m
//  Ampede
//
//  Created by Eric Ocean on Wed Jul 14 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "AIDocument-Ragel4.h"


@implementation AIDocument (Ragel4)

#define IDENT_BUFLEN 64

%%  machine4
	alphtype unsigned char;

	# The data that is to go into the fsm structure.
	struct {
		char identBuf[IDENT_BUFLEN+1];
		int identLen;
	};

	# Initialization code that will go into the Num_layersInit routine.
	init {
		fsm->identLen = 0;
	}

	# Function to buffer a character.
	action bufChar {
		if ( fsm->identLen < IDENT_BUFLEN ) {
			fsm->identBuf[fsm->identLen] = fc;
			fsm->identLen += 1;
		}
	}

	# Function to clear the buffer.
	action clearBuf {
		fsm->identLen = 0;
	}

	action int {
		fsm->identBuf[fsm->identLen] = 0; // add the Null character
		// NSLog(@"startxref number is: %s", fsm->identBuf);
	}

	# Match an integer. Upon entering the machine clear the buf, buffer
	# characters on every trans and dump the int upon leaving.
	int = digit+ >clearBuf $bufChar %int;
	
	# Leave the catch-all machine on the last character of the token
	token = (any* $0) . ( '/Size ' @1 );
	
	# Find the token, read the decimal integer, finish
	main := ( token int ) . space?;
%%

- (int)numberOfObjectsFromData:(NSData *)theData;
{
	void *buf = (void *)[theData bytes];
	int buffer_size = [theData length];
	struct machine4 fsm;
	
	machine4_init( &fsm );
	machine4_execute( &fsm, buf, buffer_size );
	
	if ( fsm.identLen > 0 )
		return atoi( (const char *)&fsm.identBuf );
	else
	{
		NSLog(@"Ampede error: couldn't determine the number of object from the data given.");
		return 0;
	}
}

@end
