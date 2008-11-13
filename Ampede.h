//
//  Ampede.h
//  Ampede
//
//  Created by Eric Ocean on Sun Jul 11 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface Ampede : NSApplication
{
	NSWindow *aboutWindow;
	BOOL aboutWindowIsVisible;
	
	BOOL isHidden;
	NSMutableSet *windowsToHide;
	
	BOOL shouldShowTrialAlert;
}

//- (void)closeAboutPanel;
- (IBAction)launchLicensingApp:(id)sender;

- (NSData *)
handleMessage:(SInt32)msgid
withData:(NSData *)data;

- (void)
messagePortInvalidated:(CFMessagePortRef)ms;

- (void)
addWindowToHide:(NSWindow *)aWindow;

@end

extern BOOL TRIAL_EXPIRED;
extern BOOL TRIAL_TIMED_OUT;