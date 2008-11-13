//
//  SequenceWindowController.m
//  Ampede
//
//  Created by Eric Ocean on Wed Jul 28 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "SequenceWindowController.h"
#import "SequenceDocument.h"
#import "PDFView.h"
#import "Ampede.h"


#ifdef LICENSING_CONTROL_ON
#import <LicenseControl/LicenseControl.h>
#import <LicenseControl/LicensingLauncherC.h>
#endif


//static NSToolbar *toolbar = nil;
static NSDictionary *toolbarList = nil;

@interface NSString (AmpedeAdditions)
 - (int)ampedeTagCompare:(NSString *)other;
 - (int)tagForItemIdentifier;
@end

@implementation SequenceWindowController

- (IBAction)choosePDF:(id)sender;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController choosePDF:] called") );
#endif	
	// I might want do this differently and use a sheet on the main Ampede window. This has the advantage of being
	// at the correct window level. It also looks cooler.
	
	[[NSOpenPanel openPanel]
			beginSheetForDirectory:nil
			file:nil
			types:[NSArray arrayWithObjects:@"pdf", @"ai", nil]
			modalForWindow:[self window]
			modalDelegate:self
			didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
			contextInfo:nil];
}

- (IBAction)reconnect:(id)sender;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController reconnect:] called") );
#endif	
	[NSApp activateIgnoringOtherApps:YES]; // make sure we're on top
	if ( ![[self document] usingDefaultPDF] )
	{
		if ( [NSFileHandle fileHandleForReadingAtPath:[[self document] pdfPath]] )
		{
			// the file exists
			[[NSOpenPanel openPanel]
					beginSheetForDirectory:[[[self document] pdfPath] stringByDeletingLastPathComponent]
					file:[[[self document] pdfPath] lastPathComponent]
					types:[NSArray arrayWithObjects:@"pdf", @"ai", nil]
					modalForWindow:[self window]
					modalDelegate:self
					didEndSelector:@selector(openPanelDidEndReconnect:returnCode:contextInfo:)
					contextInfo:nil];
		}
		else
		{
			[[NSOpenPanel openPanel]
					beginSheetForDirectory:[[[self document] pdfPath] stringByDeletingLastPathComponent]
					file:nil
					types:[NSArray arrayWithObjects:@"pdf", @"ai", nil]
					modalForWindow:[self window]
					modalDelegate:self
					didEndSelector:@selector(openPanelDidEndReconnect:returnCode:contextInfo:)
					contextInfo:nil];
		}
	}
	else
	{
		if ( [NSFileHandle fileHandleForReadingAtPath:[[self document] backupPdfPath]] )
		{
			// the file exists
			[[NSOpenPanel openPanel]
					beginSheetForDirectory:[[[self document] backupPdfPath] stringByDeletingLastPathComponent]
					file:[[[self document] backupPdfPath] lastPathComponent]
					types:[NSArray arrayWithObjects:@"pdf", @"ai", nil]
					modalForWindow:[self window]
					modalDelegate:self
					didEndSelector:@selector(openPanelDidEndReconnect:returnCode:contextInfo:)
					contextInfo:nil];
		}
		else
		{
			[[NSOpenPanel openPanel]
					beginSheetForDirectory:[[[self document] backupPdfPath] stringByDeletingLastPathComponent]
					file:nil
					types:[NSArray arrayWithObjects:@"pdf", @"ai", nil]
					modalForWindow:[self window]
					modalDelegate:self
					didEndSelector:@selector(openPanelDidEndReconnect:returnCode:contextInfo:)
					contextInfo:nil];
		}
	}
}

- (PDFView *)pageView;
{
	return pageView;
}

- (IBAction)openInDefaultViewer:(id)sender;
{
	[[NSWorkspace sharedWorkspace] openFile:[(SequenceDocument *)[self document] pdfPath]];
}

- (void)
openPanelDidEnd:(NSOpenPanel *)sheet
returnCode:(int)returnCode
contextInfo:(void *)contextInfo;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController openPanelDidEnd:returnCode:contextInfo:] called") );
#endif
	NSString *path = [sheet filename];
	
	if ( path ) 
	{
		[(SequenceDocument *)[self document] setPdfPath:path];
	}
}

- (void)
openPanelDidEndReconnect:(NSOpenPanel *)sheet
returnCode:(int)returnCode
contextInfo:(void *)contextInfo;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController openPanelDidEndReconnect:returnCode:contextInfo:] called") );
#endif
	// this has the right reconnect behavior
	[[self document] openPanelDidEnd:sheet returnCode:returnCode contextInfo:contextInfo];
}

- (void)
alertDidEnd:(NSAlert *)alert
returnCode:(int)returnCode
contextInfo:(void *)contextInfo;
{
	// don't need to do anything here
}

//+ (void)
//initialize
//{
//#ifdef DEBUG_CALLING_SEQUENCE
//	CFShow( CFSTR("+[SequenceWindowController initialize] called") );
//#endif
//	if ( toolbar ) return;
//	
//	toolbar = [[NSToolbar alloc] initWithIdentifier:@"com.ampede.ef.sequence.toolbar"];
//	
//	[toolbar setAllowsUserCustomization:NO];
//    [toolbar setAutosavesConfiguration:NO];
//    [toolbar setDisplayMode:NSToolbarDisplayModeIconAndLabel];
//	[toolbar setSizeMode:NSToolbarSizeModeRegular];
//	
//	toolbarList = [[NSDictionary alloc] initWithObjects:[NSArray arrayWithObjects:  // labels
//																					@"File",
//																					@"PDF",
//																					@"Quartz",
//																					@"ColorSync",
//																					@"Preferences",
//																					@"Software Update",
//																					@"Licensing", nil]
//										forKeys:[NSArray arrayWithObjects:  // identifiers
//																			@"com.ampede.ef.sequence.toolbar.file_info",
//																			@"com.ampede.ef.sequence.toolbar.pdf",
//																			@"com.ampede.ef.sequence.toolbar.quartz",
//																			@"com.ampede.ef.sequence.toolbar.colorsync",
//																			@"com.ampede.ef.sequence.toolbar.preferences",
//																			@"com.ampede.ef.sequence.toolbar.software_update",
//																			@"com.ampede.ef.sequence.toolbar.licensing", nil]];
//}

- initWithWindow:(NSWindow *)theWindow;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController initWithWindow:] called") );
#endif
	if ( self = [super initWithWindow:theWindow] )
	{
		blankView = [[NSView alloc] init];
		currentPane = kFileInfoPaneTag;
		[theWindow setLevel:NSModalPanelWindowLevel];
		
		toolbar = [[NSToolbar alloc] initWithIdentifier:@"com.ampede.ef.sequence.toolbar"];
		
		[toolbar setAllowsUserCustomization:NO];
		[toolbar setAutosavesConfiguration:NO];
		[toolbar setDisplayMode:NSToolbarDisplayModeIconAndLabel];
		[toolbar setSizeMode:NSToolbarSizeModeRegular];
		
		toolbarList = [[NSDictionary alloc] initWithObjects:[NSArray arrayWithObjects:  // labels
																						@"File",
																						@"PDF",
																						@"Quartz",
																						@"ColorSync",
//																						@"Preferences",
																						@"Software Update",
																						@"Licensing", nil]
											forKeys:[NSArray arrayWithObjects:  // identifiers
																				@"com.ampede.ef.sequence.toolbar.file_info",
																				@"com.ampede.ef.sequence.toolbar.pdf",
																				@"com.ampede.ef.sequence.toolbar.quartz",
																				@"com.ampede.ef.sequence.toolbar.colorsync",
//																				@"com.ampede.ef.sequence.toolbar.preferences",
																				@"com.ampede.ef.sequence.toolbar.software_update",
																				@"com.ampede.ef.sequence.toolbar.licensing", nil]];
		}
		return self;
}

- (void)
dealloc;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController dealloc] called") );
#endif
	[blankView release];
	[toolbar release];
	[super dealloc];
}


- (void)
windowDidLoad;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController windowDidLoad] called") );
#endif
	fileInfoPaneSize = [fileInfoPane frame].size;
	pdfPaneSize = [pdfPane frame].size;
	quartzPaneSize = [quartzPane frame].size;
	softwareUpdatePaneSize = [softwareUpdatePane frame].size;
	preferencesPaneSize = [preferencesPane frame].size;
	colorsyncPaneSize = [colorsyncPane frame].size;
	licensingPaneSize = [licensingPane frame].size;
	
	[self setupToolbar];
	[self selectPane:fileInfoPane withTag:kFileInfoPaneTag size:fileInfoPaneSize];
	[[[self window] toolbar] setSelectedItemIdentifier:@"com.ampede.ef.sequence.toolbar.file_info"];

	// this might not be high enough
	[[self window] setLevel:NSFloatingWindowLevel];
	[[self window] setHidesOnDeactivate:NO]; // can't set this in IB
}

- (IBAction)showWindow:(id)sender;
{
	[super showWindow:sender];
	[(Ampede *)NSApp addWindowToHide:[self window]];
}

//- (void)
//windowWillLoad;
//{
//#ifdef DEBUG_CALLING_SEQUENCE
//	CFShow( CFSTR("-[SequenceWindowController windowWillLoad] called") );
//#endif
//	// create our window hiding timer here
//	shouldDestroyWindowHidingTimer = NO;
//	[self performSelectorOnMainThread:@selector(createWindowHidingTimer) withObject:nil waitUntilDone:NO];
//}

//- (void)
//close;
//{
//#ifdef DEBUG_CALLING_SEQUENCE
//	CFShow( CFSTR("-[SequenceWindowController close] called") );
//#endif
////	// remove our window hiding timer here
////	shouldDestroyWindowHidingTimer = YES;
//	[(Ampede *)NSApp removeWindowToHide:[self window]];
//	[super close];
//}
//
//- (void)createWindowHidingTimer;
//{
//#ifdef DEBUG_CALLING_SEQUENCE
//	CFShow( CFSTR("-[SequenceWindowController createWindowHidingTimer:] called") );
//#endif
//	[NSTimer	scheduledTimerWithTimeInterval:.2 // fires every 200 milliseconds
//				target:self
//				selector:@selector(handleWindowBehavior:)
//				userInfo:nil
//				repeats:YES];
//}

//- (void)
//handleWindowBehavior:(NSTimer *)theTimer;
//{
//#ifdef DEBUG_CALLING_SEQUENCE
//	CFShow( CFSTR("-[SequenceWindowController handleWindowBehavior:] called") );
//#endif
//	NSString *appName = (NSString *)[[[NSWorkspace sharedWorkspace] activeApplication] objectForKey:@"NSApplicationName"];
//	NSLog(@"active app is %@", appName);
//	if ( [appName isEqualToString:@"Final Cut Pro HD"] || [appName isEqualToString:@"Final Cut Pro"] )
//	{
//		[[self window] orderFront:nil];
//	}
//	else if ( appName == NULL ) // this could be us or another non-activating app
//	{
//		[[self window] orderFront:nil];
//	}
//	else // another activating app is currently active
//	{
//		[[self window] orderOut:nil];
//	}
//	if ( shouldDestroyWindowHidingTimer )
//	{
//		// our window was closed
//		[theTimer invalidate];
//		shouldDestroyWindowHidingTimer = NO;
//	}
//}

- (void)
setupToolbar;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController setupToolbar] called") );
#endif
	[toolbar setDelegate:self];
	[[self window] setToolbar:toolbar];
}

- (NSArray*)
toolbarSelectableItemIdentifiers:(NSToolbar*)toolbar;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController toolbarSelectableItemIdentifiers:] called") );
#endif
	return [toolbarList allKeys];
}

- (NSArray *)
toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController toolbarDefaultItemIdentifiers:] called") );
#endif
	NSMutableArray *allowedItems = [[[toolbarList allKeys] mutableCopy] autorelease];
	
	allowedItems = [[[allowedItems sortedArrayUsingSelector:@selector(ampedeTagCompare:)] mutableCopy] autorelease];
	[allowedItems insertObject:NSToolbarFlexibleSpaceItemIdentifier atIndex:4];
	[allowedItems insertObject:NSToolbarSeparatorItemIdentifier atIndex:5];
	
	return allowedItems;
}

- (NSArray *)
toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController toolbarAllowedItemIdentifiers:] called") );
#endif
    NSMutableArray *allowedItems = [[toolbarList allKeys] mutableCopy];
	
	[allowedItems addObjectsFromArray:
					[NSArray arrayWithObjects:
									NSToolbarSeparatorItemIdentifier,
									NSToolbarSpaceItemIdentifier,
									NSToolbarFlexibleSpaceItemIdentifier,
									NSToolbarCustomizeToolbarItemIdentifier, nil]];
	return allowedItems;
}

- (NSToolbarItem *)
toolbar:(NSToolbar *)toolbar
itemForItemIdentifier:(NSString *)itemIdent
willBeInsertedIntoToolbar:(BOOL)willBeInserted;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController toolbar:itemForItemIdentifier:willBeInsertedIntoToolbar:] called") );
#endif
    NSToolbarItem *toolbarItem = [[[NSToolbarItem alloc] initWithItemIdentifier:itemIdent] autorelease];
    NSString *itemLabel = nil;
	
    if ( itemLabel = [toolbarList objectForKey:itemIdent] )
	{
		[toolbarItem setLabel:itemLabel];
		[toolbarItem setTag:[self tagForItemIdentifier:itemIdent]];
//		if ( [toolbarItem tag] == kFileInfoPaneTag )
//		{
//			// get default document icon, a blank page
//			[toolbarItem setImage:[[NSWorkspace sharedWorkspace] iconForFile:nil]]; // [[self document] fileName]
//		}
		/* else */[toolbarItem setImage:[NSImage imageNamed:itemIdent]];
		[toolbarItem setTarget:self];
		[toolbarItem setAction:@selector(changePanes:)];
    }
	else
	{
		// Returning nil will inform the toolbar this kind of item is not supported 
		toolbarItem = nil;
    }
	
    return toolbarItem;
}

- (int)
tagForItemIdentifier:(NSString *)itemIdent;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController tagForItemIdentifier:] called") );
#endif
	if ( [itemIdent isEqualToString:@"com.ampede.ef.sequence.toolbar.file_info"] ) return kFileInfoPaneTag;
	if ( [itemIdent isEqualToString:@"com.ampede.ef.sequence.toolbar.pdf"] ) return kPDFPaneTag;
	if ( [itemIdent isEqualToString:@"com.ampede.ef.sequence.toolbar.quartz"] ) return kQuartzPaneTag;
	if ( [itemIdent isEqualToString:@"com.ampede.ef.sequence.toolbar.colorsync"] ) return kColorsyncPane;
	if ( [itemIdent isEqualToString:@"com.ampede.ef.sequence.toolbar.preferences"] ) return kPreferencesPane;
	if ( [itemIdent isEqualToString:@"com.ampede.ef.sequence.toolbar.software_update"] ) return kSoftwareUpdatePane;
	if ( [itemIdent isEqualToString:@"com.ampede.ef.sequence.toolbar.licensing"] ) return kLicensingPane;
	return kFileInfoPaneTag; // the default
}

- (IBAction)changePanes:(id)sender;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController changePanes:] called") );
#endif
	SequencePaneTag tag = [sender tag];
	
	switch ( tag ) {
	case kFileInfoPaneTag: [self selectPane:fileInfoPane withTag:tag size:fileInfoPaneSize]; break;
	case kPDFPaneTag: [self selectPane:pdfPane withTag:tag size:pdfPaneSize]; break;
	case kQuartzPaneTag: [self selectPane:quartzPane withTag:tag size:quartzPaneSize]; break;
	case kSoftwareUpdatePane: [self selectPane:softwareUpdatePane withTag:tag size:softwareUpdatePaneSize]; break;
	case kPreferencesPane: [self selectPane:preferencesPane withTag:tag size:preferencesPaneSize]; break;
	case kColorsyncPane: [self selectPane:colorsyncPane withTag:tag size:colorsyncPaneSize]; break;
	case kLicensingPane: [self selectPane:licensingPane withTag:tag size:licensingPaneSize]; break;
	}
}

- (void)
selectPane:(NSView *)thePane
withTag:(int)theTag
size:(NSSize)theSize;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController selectPane:withTag:size:] called") );
#endif
    [[self window] setContentView:blankView];
    [self resizeWindowToSize:theSize];
    [[self window] setContentView:thePane];
	// check view retain count
	currentPane = theTag;
}

- (void)
resizeWindowToSize:(NSSize)newSize;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController resizeWindowToSize:] called") );
#endif
    NSRect aFrame;
    
    float newHeight = newSize.height;
    float newWidth = newSize.width;

    aFrame = [[self window] contentRectForFrameRect:[[self window] frame]];
    
    aFrame.origin.y += aFrame.size.height;
    aFrame.origin.y -= newHeight;
    aFrame.size.height = newHeight;
    aFrame.size.width = newWidth;
    
    aFrame = [[self window] frameRectForContentRect:aFrame];
    
    [[self window] setFrame:aFrame display:YES animate:YES];
}

- (IBAction)launchColorSyncUtility:(id)sender;
{
	CMLaunchControlPanel( 0 );
}

- (IBAction)launchLicensing:(id)sender;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[Ampede launchLicensingApp:] called") );
#endif
#ifdef LICENSING_CONTROL_ON
	lauchLicensing(); 
#endif
}

- (IBAction)launchInfoWebPage:(id)sender;
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://www.ampede.com/pdf/"]];
}

- (IBAction)launchPurchaseWebPage:(id)sender;
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://www.ampede.com/pdf/buy.html"]];
}

- (int)licensingState;
{
#ifdef LICENSING_CONTROL_ON
	int lic = licensingLevelCheck().opMode;
	
//	NSLog(@"lic is %d", lic);
	
	if ( lic != OpModeLicensed )
	{
//		NSLog(@"returning 0 from licensingState");
		return 0;
	}
	else 
	{
//		NSLog(@"returning 1 from licensingState");
		return 1;
	}
	// return (OpModeLicensed == licensingLevelCheck().opMode); // ? 1 : 0;
#else
	return 1;
#endif
}

OSStatus
MyGotoHelpAnchor( CFStringRef anchorName )
{
    CFBundleRef myApplicationBundle = NULL;
    CFTypeRef myBookName = NULL;
    OSStatus err = noErr;

    myApplicationBundle = CFBundleGetMainBundle(); 

    if ( myApplicationBundle == NULL ) { err = fnfErr; goto bail; }

    myBookName = CFBundleGetValueForInfoDictionaryKey( myApplicationBundle, CFSTR("CFBundleHelpBookName") );

    if ( myBookName == NULL ) { err = fnfErr; goto bail; }

    if ( CFGetTypeID( myBookName ) != CFStringGetTypeID() ) err = paramErr;
	
    if ( err == noErr ) [[NSHelpManager sharedHelpManager]	openHelpAnchor:(NSString *)anchorName
															inBook:(NSString *)myBookName];



bail:
    return err;
}

- (IBAction)openHelp:(id)sender;
{
	HelpTag tag = [sender tag];
	
	switch (tag) {
	case kTheFilePane:
		MyGotoHelpAnchor( CFSTR( "file_pane" ) );
		break;
	case kThePDFPane:
		MyGotoHelpAnchor( CFSTR( "pdf_pane" ) );
		break;
	case kTheQuartzPane:
		MyGotoHelpAnchor( (CFStringRef)@"quartz_pane" );
		break;
	case kTheColorSyncPane:
		MyGotoHelpAnchor( (CFStringRef)@"colorsync_pane" );
		break;
	case kTheSoftwareUpdatePane:
		MyGotoHelpAnchor( (CFStringRef)@"software_update_pane" );
		break;
	case kTheLicensingPane:
		MyGotoHelpAnchor( (CFStringRef)@"licensing_pane" );
		break;
	case kTipsAndTricks:
	case kAboutAmpedePDF:
	case kQuickStartTutorial:
	case kFilterControls:
		break;
	}
}

@end

@implementation NSString ( AmpedeAdditions )

 - (int)
 ampedeTagCompare:(NSString *)other;
 {
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController ampedeTagCompare:] called") );
#endif
	int myTag = [self tagForItemIdentifier];
	int otherTag = [other tagForItemIdentifier];
	
	if ( myTag < otherTag ) return NSOrderedAscending;
	else if ( myTag == otherTag ) return NSOrderedSame;
	else return NSOrderedDescending;
 }
 
- (int)
tagForItemIdentifier;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceWindowController tagForItemIdentifier] called") );
#endif
	if ( [self isEqualToString:@"com.ampede.ef.sequence.toolbar.file_info"] ) return kFileInfoPaneTag;
	if ( [self isEqualToString:@"com.ampede.ef.sequence.toolbar.pdf"] ) return kPDFPaneTag;
	if ( [self isEqualToString:@"com.ampede.ef.sequence.toolbar.quartz"] ) return kQuartzPaneTag;
	if ( [self isEqualToString:@"com.ampede.ef.sequence.toolbar.colorsync"] ) return kColorsyncPane;
	if ( [self isEqualToString:@"com.ampede.ef.sequence.toolbar.preferences"] ) return kPreferencesPane;
	if ( [self isEqualToString:@"com.ampede.ef.sequence.toolbar.software_update"] ) return kSoftwareUpdatePane;
	if ( [self isEqualToString:@"com.ampede.ef.sequence.toolbar.licensing"] ) return kLicensingPane;
	return kFileInfoPaneTag; // the default
}

@end

