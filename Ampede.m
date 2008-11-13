//
//  Ampede.m
//  Ampede
//
//  Created by Eric Ocean on Sun Jul 11 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "Ampede.h"
#import "AboutView.h"
#import "AmpedeDocument.h"


#ifdef LICENSING_CONTROL_ON
#import <LicenseControl/LicenseControl.h>
#import <LicenseControl/LicensingLauncherC.h>
#endif

#define TRIAL_LENGTH 1296000 // 15 days

BOOL TRIAL_EXPIRED = YES;
BOOL TRIAL_TIMED_OUT = NO;

#define DEBUG_CALLING_SEQUENCE

static CFMessagePortRef defaultPort;
static CFRunLoopSourceRef runLoopSource;

CFDataRef defaultPortCallback( CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info )
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("Ampede.m defaultPortCallback() called") );
#endif
	return (CFDataRef)[(Ampede *)NSApp handleMessage:msgid withData:(NSData *)data];
}

void defaultPortInvalidationCallback( CFMessagePortRef ms, void *info )
{
	[(Ampede *)NSApp messagePortInvalidated:ms];
}

@implementation Ampede

+ (NSApplication *)
sharedApplication;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("+[Ampede sharedApplication] called") );
#endif
	CFMessagePortContext defaultPortContext = { 0, self, NULL, NULL, NULL };
	
	defaultPort = CFMessagePortCreateLocal( kCFAllocatorDefault,
											(CFStringRef)@"AmpedeMessagePort",
											defaultPortCallback,
											&defaultPortContext,
											NULL );
	if ( defaultPort != NULL )
	{
		CFMessagePortSetInvalidationCallBack( defaultPort, defaultPortInvalidationCallback );

		runLoopSource = CFMessagePortCreateRunLoopSource( kCFAllocatorDefault, defaultPort, 0 );

		if ( runLoopSource != NULL )
		{
			CFRunLoopAddSource( [[NSRunLoop currentRunLoop] getCFRunLoop], runLoopSource, kCFRunLoopCommonModes );
		}
		else
		{
			CFRelease( defaultPort ); defaultPort = NULL;
			CFShow( CFSTR("AmpedeUI error: failed to create run loop source for AmpedeUI.") );				
			CFShow( CFSTR("AmpedeUI error: failed to create a message port for AmpedeUI.") );
		}
	}
	else
	{
		CFShow( CFSTR("AmpedeUI error: failed to create a message port for Ampede.") );
		exit(0); // we don't want the app to keep running
	}

	return [super sharedApplication];
}

#define CONTENT_WIDTH 600
#define CONTENT_HEIGHT 350

- init
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[Ampede init] called") );
#endif
	if ( self = [super init] ) {
		NSRect screenRect = [[NSScreen mainScreen] frame];
		
		float bottomLeftX = ( screenRect.size.width / 2 ) - ( CONTENT_WIDTH / 2 );
		float bottomLeftY = ( screenRect.size.height * 0.635 ) - ( CONTENT_HEIGHT / 2 );
		
		NSRect contentRect = NSMakeRect( bottomLeftX, bottomLeftY, CONTENT_WIDTH, CONTENT_HEIGHT );
		
		aboutWindow = [[NSWindow alloc]
								initWithContentRect:contentRect
                                styleMask:NSBorderlessWindowMask
                                backing:NSBackingStoreBuffered
                                defer:YES screen:[NSScreen mainScreen]];
		[aboutWindow setHasShadow:YES];
		
		AboutView *av = [[AboutView alloc] initWithFrame:contentRect];
		[aboutWindow setContentView:av];
		[av setImage:[NSImage imageNamed:@"about"]];
		[self setDelegate:self];
		[aboutWindow setLevel:CGShieldingWindowLevel()];

		[[NSNotificationCenter defaultCenter]
					addObserver:self
					selector:@selector(closeAboutPanel:)
					name:NSApplicationWillResignActiveNotification
					object:nil];
					
		//////// Trial mode code
		
		shouldShowTrialAlert = YES;
		
#ifdef LICENSING_CONTROL_ON
		if ( OpModeLicensed == licensingLevelCheck().opMode ) shouldShowTrialAlert = NO;
#endif

		NSFileManager *dfm = [NSFileManager defaultManager];
		if ( ![dfm fileExistsAtPath:@"/Library/Application Support/Final Cut Pro System Support/.Final Cut Pro System ID"] )
		{
			// this is the first time the application has been run
#ifndef LICENSING_CONTROL_ON
			NSLog(@"Ampede first run; creating trial hidden file.");
#endif
			double startTime = [[NSDate  date] timeIntervalSince1970];
			id data = [NSMutableData data];
			[data appendBytes:&startTime length:8];
			
			BOOL success = [dfm	createFileAtPath:@"/Library/Application Support/Final Cut Pro System Support/.Final Cut Pro System ID"
								contents:data
								attributes:nil];
									
			if ( success ) TRIAL_EXPIRED = NO;
			else TRIAL_EXPIRED = YES;
		}
		else
		{
#ifndef LICENSING_CONTROL_ON
			NSLog(@"Ampede has been run before; the trial hidden file is present.");
#endif
			// get time interval since startTime
			id fh = [NSFileHandle fileHandleForReadingAtPath:@"/Library/Application Support/Final Cut Pro System Support/.Final Cut Pro System ID"];
			
			NSData *fileContents = [fh availableData];
			double *startTime = (double *)[fileContents bytes];
			id startDate = [NSDate dateWithTimeIntervalSince1970:*startTime];
			double timeExpired = [[NSDate date] timeIntervalSinceDate:startDate];
			
			if ( timeExpired > TRIAL_LENGTH ) TRIAL_EXPIRED = YES;
			else TRIAL_EXPIRED = NO;
			
		}
	}
	return self;
}

- (void)closeAboutPanel:(NSNotification *)note;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[Ampede closeAboutPanel:] called") );
#endif
	if ( aboutWindowIsVisible )
	{
		aboutWindowIsVisible = NO;
		[aboutWindow orderOut:nil];
	}
}

//- (void)closeAboutPanel;
//{
//#ifdef DEBUG_CALLING_SEQUENCE
//	CFShow( CFSTR("-[Ampede closeAboutPanel:] called") );
//#endif
//	[aboutWindow orderOut:nil];
//}
//
- (void)
sendEvent:(NSEvent *)anEvent;
{
	if ( aboutWindowIsVisible )
	{
		int type = [anEvent type];
		
		switch ( type ) {
		case NSLeftMouseDown:
		case NSRightMouseDown:
		case NSOtherMouseDown:
		case NSScrollWheel:
			aboutWindowIsVisible = NO;
			[aboutWindow orderOut:nil];
		}
	}
	[super sendEvent:anEvent];
}

- (NSData *)
handleMessage:(SInt32)msgid
withData:(NSData *)data;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[Ampede handleMessage:withData:] called") );
#endif
	if ( msgid == 'INFO' )
	{
		[NSApp activateIgnoringOtherApps:YES];
		[aboutWindow makeKeyAndOrderFront:nil];
		aboutWindowIsVisible = YES;
	}
	else if ( msgid == 'QUIT' )
	{
		[NSApp terminate:nil];
	}
	else if ( msgid == 'SSET' )
	{
		// setup a new sequence document
		id doc = [[NSDocumentController sharedDocumentController] makeUntitledDocumentOfType:@"FCP Sequence"];
		[[NSDocumentController sharedDocumentController] addDocument:doc];
		return [doc retainedUuidData];
	}
	return NULL;
}

- (void)
messagePortInvalidated:(CFMessagePortRef)ms;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[Ampede messagePortInvalidated:] called") );
#endif
	[NSApp activateIgnoringOtherApps:YES];
	NSRunCriticalAlertPanel(	@"Ampede EF encountered a fatal error.", // title
								@"Please restart Final Cut Pro. If the problem continues, contact EF support at ef@ampede.com.", // message
								@"Continue", // default button
								nil,
								nil		);
}

- (BOOL)
applicationShouldOpenUntitledFile:(NSApplication *)sender; 
{ 
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[Ampede applicationShouldOpenUntitledFile:] called") );
#endif
	return NO;
}

- (IBAction)launchLicensingApp:(id)sender;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[Ampede launchLicensingApp:] called") );
#endif
#ifdef LICENSING_CONTROL_ON
	lauchLicensing(); 
#endif
}

- (IBAction)orderFrontColorPanel:(id)sender;
{
	NSWindow *cp  = [NSColorPanel sharedColorPanel];
	[cp setHidesOnDeactivate:NO];
	[cp orderFront:nil];
	[self addWindowToHide:cp];
}

- (void)
managelWindowBehavior:(NSTimer *)theTimer;
{
#ifdef DEBUG_CALLING_SEQUENCE
//	CFShow( CFSTR("-[Ampede managelWindowBehavior:] called") );
#endif
	NSString *appName = (NSString *)[[[NSWorkspace sharedWorkspace] activeApplication] objectForKey:@"NSApplicationName"];
//	NSLog(@"active app is %@", appName);
	if ( [appName isEqualToString:@"Final Cut Pro HD"] || [appName isEqualToString:@"Final Cut Pro"] )
	{
		if ( isHidden )
		{
//			CFShow( CFSTR("isHidden was YES") );
//			NSLog(@"%@", windowsToHide);
			id e = [windowsToHide objectEnumerator];
			id i;
			while ( i = [e nextObject] )
			{
				[i orderFront:nil];
			}
			isHidden = NO;
		}
	}
	else if ( appName == NULL ) // this could be us or another non-activating app
	{
		// do nothing (we're already visible)
//		CFShow( CFSTR("[self unhide:nil] called") );
//		[self unhide:nil];
	}
	else // another activating app is currently active
	{
		id e = [windowsToHide objectEnumerator];
		id i;
		while ( i = [e nextObject] )
		{
			[i orderOut:nil];
		}
		isHidden = YES;
	}
}

- (void)
finishLaunching;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[Ampede applicationWillFinishLaunching] called") );
#endif
	isHidden = NO;
	windowsToHide = [[NSMutableSet alloc] init];

	[NSTimer	scheduledTimerWithTimeInterval:.2 // fires every 200 milliseconds
				target:self
				selector:@selector(managelWindowBehavior:)
				userInfo:nil
				repeats:YES];
				
	[[NSNotificationCenter defaultCenter]	addObserver:self
											selector:@selector(handleWindowClosed:)
											name:NSWindowWillCloseNotification
											object:nil];
	
	if ( shouldShowTrialAlert && TRIAL_EXPIRED )
	{
		// show alert here
		NSAlert *alert = [[NSAlert alloc] init];
		[alert addButtonWithTitle:@"Purchase"];
		[alert addButtonWithTitle:@"OK"];
		[alert addButtonWithTitle:@"Enter Serial Number"];
		[alert setAlertStyle:NSInformationalAlertStyle];
		[alert setMessageText:@"Please purchase Ampede PDF today!"];
		[alert setInformativeText:@"Your trial period has expired. Your session will be limited to 10 minutes, and your media will not reconnect automatically. If you like the PDF plugin, please purchase it for unrestricted use!"];
		
		[NSApp activateIgnoringOtherApps:YES]; // put us on top
		int returnButton = [alert runModal];
		
		if ( returnButton == NSAlertFirstButtonReturn )
		{
			// user wants to purchase Ampede PDF
			[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://www.ampede.com/pdf/buy.html"]];
		}
		else if ( returnButton == NSAlertSecondButtonReturn )
		{
			// user wants to use plugin
			;
		}
		else if ( returnButton == NSAlertThirdButtonReturn )
		{
			// user wants to enter serial number
			#ifdef LICENSING_CONTROL_ON
				lauchLicensing(); 
			#endif
		}

		[self performSelectorOnMainThread:@selector(createTrialTimer) withObject:nil waitUntilDone:NO];
	}
}

- (void)
createTrialTimer;
{
	// set 10 minute timer
	[NSTimer	scheduledTimerWithTimeInterval:600 // fires after 10 minutes
				target:self
				selector:@selector(trialTimedOut:)
				userInfo:nil
				repeats:NO];
}

- (void)
quitTrial;
{
	NSAlert *alert = [[NSAlert alloc] init];
	[alert addButtonWithTitle:@"Purchase"];
	[alert addButtonWithTitle:@"Unload"];
	[alert setAlertStyle:NSInformationalAlertStyle];
	[alert setMessageText:@"Please purchase Ampede PDF today!"];
	[alert setInformativeText:@"Your trial period has expired, and your 10 minute demo limit has been reached. We hope you like Ampede PDF. If you use it, please pay for it so we can make it better!"];
	
	[NSApp activateIgnoringOtherApps:YES]; // put us on top
	int returnButton = [alert runModal];
	
	if ( returnButton == NSAlertFirstButtonReturn )
	{
		// user wants to purchase Ampede PDF
		[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://www.ampede.com/pdf/buy.html"]];
	}
	// else( returnButton == NSAlertSecondButtonReturn ) --user wants to unload the plugin

	[[NSNotificationCenter defaultCenter] postNotificationName:@"AmpedeTrialQuit" object:nil];
}

- (void)
trialTimedOut:(NSTimer *)theTimer;
{
	TRIAL_TIMED_OUT = YES;
	
	[theTimer invalidate];
	
	NSString *appName = (NSString *)[[[NSWorkspace sharedWorkspace] activeApplication] objectForKey:@"NSApplicationName"];
	if ( [appName isEqualToString:@"Final Cut Pro HD"] || [appName isEqualToString:@"Final Cut Pro"] )
	{
		[self quitTrial];
	}
	else [self performSelectorOnMainThread:@selector(createQuitTimer) withObject:nil waitUntilDone:NO];
}

- (void)
createQuitTimer;
{
	[NSTimer	scheduledTimerWithTimeInterval:1 // fires every second
				target:self
				selector:@selector(quitTrial:)
				userInfo:nil
				repeats:YES];
}

- (void)
quitTrial:(NSTimer *)theTimer;
{
	NSString *appName = (NSString *)[[[NSWorkspace sharedWorkspace] activeApplication] objectForKey:@"NSApplicationName"];
	if ( [appName isEqualToString:@"Final Cut Pro HD"] || [appName isEqualToString:@"Final Cut Pro"] )
	{
		[theTimer invalidate];
		[self quitTrial];
	}
}

- (void)
addWindowToHide:(NSWindow *)aWindow;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[Ampede addWindowToHide:] called") );
#endif
	[windowsToHide addObject:aWindow];
}

- (void)
handleWindowClosed:(NSNotification *)note;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[Ampede handleWindowClosed:] called") );
#endif
	[windowsToHide removeObject:[note object]];
}

@end
