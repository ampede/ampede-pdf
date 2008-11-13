//
//  AIDocument-Ragel.m
//  Ampede
//
//  Created by Eric Ocean on Wed Jul 14 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "AIDocument-Ragel.h"


@implementation AIDocument (Ragel)

#define IDENT_BUFLEN 64

%%  machine
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
		NSLog(@"\%AI5_NumLayers: %s", fsm->identBuf);
	}

	# Match an integer. Upon entering the machine clear the buf, buffer
	# characters on every trans and dump the int upon leaving.
	int = digit+ >clearBuf $bufChar %int;
	
	# Leave the catch-all machine on the last character of the token
	token = (any* $0) . ( '%AI5_NumLayers: ' @1 );

	# Find the token, read the decimal integer, finish
	main := ( token . int ) . space?;
%%

- (int)numberOfLayers;
{
	void *buf = (void *)[fileData bytes];
	int buffer_size = [fileData length];
	struct machine fsm;
	
	machine_init( &fsm );
	machine_execute( &fsm, buf, buffer_size );
	
	if ( fsm.identLen > 0 )
		return atoi( (const char *)&fsm.identBuf );
	else
	{
		NSLog(@"Ampede error: couldn't determine the number of layers in the Adobe Illustrator file.");
		return 0;
	}
}

@end
