//
//  AIDocument-Ragel1.m
//  Ampede
//
//  Created by Eric Ocean on Wed Jul 14 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "AIDocument-Ragel1.h"


@implementation AIDocument (Ragel1)

%%  machine1
	alphtype unsigned char;

	# The data that is to go into the fsm structure.
	struct {
		int loc;
		int layerStart;
	};

	# Initialization code that will go into the Locate_layer_init routine.
	init {
		fsm->loc = 0;
		fsm->layerStart = 0;
	}
	
	action incrementLoc {
		fsm->loc += 1;
		// NSLog(@"current loc is %d, current char is %c", fsm->loc, fc);
	}

	action layerStart {
		fsm->layerStart = fsm->loc;
		// NSLog(@"layer start called and is %d", fsm->layerStart);
	}

	# Match the Layer name and exit
	layer = (any* $0) . ("/Layer" %1) . (any >2 >{ fgoto end; });
	
	# record the start of the layer by incrementing on every character and recording when a '/' is encountered
	# when the main machine quits, this will have been the location of the '/' in '/Layer'
	layerStart = ( any | ('/' >0 >layerStart) )* $1 $incrementLoc;
	
	end := empty >{NSLog(@"end machine entered");};
	
	# Find the token, read the decimal integer, finish
	main := layer | layerStart ;
%%

- (int)locateLayerAfterByte:(int)start;
{
	void *buf = (void *)[streamData bytes] + start;
	int buffer_size = [streamData length] - start;
	struct machine1 fsm;
	
	machine1_init( &fsm );
	machine1_execute( &fsm, buf, buffer_size );
	
	return fsm.layerStart;
}

@end
