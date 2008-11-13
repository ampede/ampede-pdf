//
//  SequenceDocument.m
//  Ampede
//
//  Created by Eric Ocean on Sun Jul 25 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "SequenceDocument.h"
#import <unistd.h>
#import "UKKQueue.h"
#import "PDFView.h"
#import "Ampede.h"

#ifdef LICENSING_CONTROL_ON
#import <LicenseControl/LicenseControl.h>
#import <LicenseControl/LicensingLauncherC.h>
#endif


@implementation SequenceDocument

+ (void)
initialize
{
	static BOOL classDidInitialize = NO; if ( classDidInitialize ) return; classDidInitialize = YES;
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("+[SequenceDocument initialize] called") );
#endif
    [self   setKeys:[NSArray arrayWithObjects:	@"fileGeneration", // this can be updated independently of changing the path in a relink situation
												@"pdfPath",
												@"currentPage",
												@"pageRotation",
												@"shouldPreserveAspectRatio",
												@"shouldDrawBackground",
												@"backgroundColor",
												@"boxType", nil]
			triggerChangeNotificationsForDependentKey:@"drawCacheIsValid"];

    [self   setKeys:[NSArray arrayWithObjects:@"pdfPath", nil]
			triggerChangeNotificationsForDependentKey:@"displayName"];
}

+ (BOOL)
automaticallyNotifiesObserversForKey:(NSString *)theKey
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("+[SequenceDocument automaticallyNotifiesObserversForKey:] called") );
#endif
	BOOL automatic = YES;
    if (	[theKey isEqualToString:@"pdf"] ||
			[theKey isEqualToString:@"page"] ||
			[theKey isEqualToString:@"pdfPath"] ||
			[theKey isEqualToString:@"backgroundColor"] )
	{
        automatic = NO;
    }
	else automatic = [super automaticallyNotifiesObserversForKey:theKey];
    return automatic;
}

- (void)
observeValueForKeyPath:(NSString *)keyPath
ofObject:(id)object 
change:(NSDictionary *)change
context:(void *)context;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument observeValueForKeyPath:ofObject:change:context:] called") );
#endif
    if ( [keyPath isEqualToString:@"drawCacheIsValid"] )
	{
		// this is triggered if any of our dependent keys changes--see above
		drawCacheIsValid = NO;
		[self invalidate]; // this causes AmpedeDocument (super) to recompute it's data cache which is how the plug-in knows we've changed
	}
    if ( [keyPath isEqualToString:@"displayName"] )
	{
		CFShow( CFSTR("-[SequenceDocument displayName observeValueForKeyPath: called") );
		// this is triggered if any of our dependent keys changes--see above
		[[wc window] setTitle:[self displayName]];
	}
    if ( [keyPath isEqualToString:@"usingDefaultPDF"] )
	{
		CFShow( CFSTR("-[SequenceDocument usingDefaultPDF observeValueForKeyPath: called") );
		// this is triggered if any of our dependent keys changes--see above
		[[wc window] setTitle:[self displayName]];
	}
	else
	{
		[super  observeValueForKeyPath:keyPath
				ofObject:object
				change:change
				context:context];
	}
}

- (void)
useDefaultPdf;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument useDefaultPdf] called") );
#endif
	[self setUsingDefaultPDF:YES];
	[self setPdfPath:[[NSBundle bundleForClass:[self class]] pathForResource:@"default" ofType:@"pdf"]];
	[self setUsingDefaultPDF:YES]; // hack
	[self setMonitorFileForChanges:NO];
	[self setBackupPdfPath:nil]; // hack
	[self setFileIcon:nil];
}

- (void)
setUsingDefaultPDF:(BOOL)yn;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument setUsingDefaultPDF:] called") );
#endif
	usingDefaultPDF = yn;
}

- (void)
setMonitorFileForChanges:(BOOL)yn;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument setMonitorFileForChanges:] called") );
#endif
	monitorFileForChanges = yn;
	if (!usingDefaultPDF) [self monitorNewPath:pdfPath];
}

- init;
{
	if ( self = [super init] )
	{
#ifdef DEBUG_CALLING_SEQUENCE
		CFShow( CFSTR("-[SequenceDocument init] called") );
#endif
		pdf = nil;
		page = nil;
		fileGeneration = 1; // the plug-in starts at zero; this causes it to issue the 'FILE' command
		currentPage = 1;
		pageRotation = 0; // don't change; screws up movement in Final Cut Pro
		shouldPreserveAspectRatio = YES;
		shouldDrawBackground = YES;
		backgroundColor = [[NSColor whiteColor] retain];
		boxType = kCGPDFMediaBox;
		
		wc = nil;
		shouldRequestPDF = YES;
		requestedFirstPDF = NO;
		
		drawCache = nil;
		drawCacheIsValid = NO;
		fileCache = nil;
		fileCacheIsValid = NO;
		
		red = 1.0;
		green = 1.0;
		blue = 1.0;
		alpha = 1.0;
		
		kqueue = [[UKKQueue alloc] init];
		monitorFileForChanges = NO;
		shouldReconnectOnLoad = YES;
		backupPdfPage = 0;
		backupPdfPath = nil;
		pdfPath = nil;
		
		cachedBoxType = boxType;
		
		[self useDefaultPdf];
				
		// this insures that we receive notifications when drawingDataIsValid "changes"
		// which is whenever one of its dependent keys is updated
		[self	addObserver:self
				forKeyPath:@"drawCacheIsValid" 
				options:NSKeyValueChangeSetting
				context:NULL];
		
		[[NSDistributedNotificationCenter defaultCenter]	addObserver:self
															selector:@selector(colorProfileDidChange:)
															name:(NSString *)kCMPrefsChangedNotification
															object:nil];
															
		[[NSNotificationCenter defaultCenter]	addObserver:self
												selector:@selector(handleTrialQuit:)
												name:@"AmpedeTrialQuit"
												object:nil];
															
		[self colorProfileDidChange:nil]; // this sets up our initial profile name
		
		[[[NSWorkspace sharedWorkspace] notificationCenter]	addObserver:self
															selector:@selector(handleFileRenamed:)
															name:UKKQueueFileRenamedNotification
															object:nil];
											
		[[[NSWorkspace sharedWorkspace] notificationCenter]	addObserver:self
															selector:@selector(handleFileWrittenTo:)
															name:UKKQueueFileWrittenToNotification
															object:nil];
											
		[[[NSWorkspace sharedWorkspace] notificationCenter]	addObserver:self
															selector:@selector(handleFileDeleted:)
															name:UKKQueueFileDeletedNotification
															object:nil];
											
//		[[[NSWorkspace sharedWorkspace] notificationCenter]	addObserver:self
//															selector:@selector(handleFileSizeIncreased:)
//															name:UKKQueueFileDeletedNotification
//															object:nil];
	}
	return self;
}

- (void)
handleTrialQuit:(NSNotification *)note;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument handleTrialQuit:] called") );
#endif
	[self revertToUsingDefaultPDF];
}

- (void)
handleFileRenamed:(NSNotification *)note;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument handleFileRenamed:] called") );
#endif
	NSString *fp = [note object];
	if ( [fp isEqualToString:pdfPath] )
	{
		backupPdfPage = currentPage;
		NSString *appName = (NSString *)[[[NSWorkspace sharedWorkspace] activeApplication] objectForKey:@"NSApplicationName"];
		if ( monitorFileForChanges )
		{
			if ( [appName isEqualToString:@"Final Cut Pro HD"] || [appName isEqualToString:@"Final Cut Pro"] || [appName isEqualToString:@"Ampede LCed"] )
			{
				[self presentFileMissingAlert];
			}
			else
			{
				// Begin a timer that checks for Final Cut Pro's activation, when it activates, we'll present our alert panel
				[self performSelectorOnMainThread:@selector(createFileMissingTimer) withObject:nil waitUntilDone:NO];
				[self revertToUsingDefaultPDF];
				shouldReconnectOnLoad = NO;
			}
		}
		else
		{
			// don't alert the user that anything has changed
			[self revertToUsingDefaultPDF];
			shouldReconnectOnLoad = NO;
		}
	}
}

- (void)
createFileMissingTimer;
{
	[NSTimer	scheduledTimerWithTimeInterval:1 // fires every second
				target:self
				selector:@selector(presentFileMissingAlert:)
				userInfo:nil
				repeats:YES];
}

- (void)
handleFileDeleted:(NSNotification *)note;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument handleFileDeleted:] called") );
#endif
	[self handleFileRenamed:note];
}

- (void)testMe:(NSTimer *)theTimer;
{
	CFShow( CFSTR("the timer fired") );
}

- (void)
presentFileMissingAlert:(NSTimer *)theTimer;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument presentFileModifiedAlert:] called") );
#endif
	NSString *appName = (NSString *)[[[NSWorkspace sharedWorkspace] activeApplication] objectForKey:@"NSApplicationName"];
	if ( [appName isEqualToString:@"Final Cut Pro HD"] || [appName isEqualToString:@"Final Cut Pro"] || [appName isEqualToString:@"Ampede LCed"] )
	{
		[theTimer invalidate];
		[self presentFileMissingAlert];
	}
}

- (void)
presentFileMissingAlert;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument presentFileMissingAlert] called") );
#endif

	NSAlert *alert = [[NSAlert alloc] init];
	[alert addButtonWithTitle:@"OK"];
	[alert addButtonWithTitle:@"Reconnect..."];
	[alert setAlertStyle:NSWarningAlertStyle];
	[alert setMessageText:@"The following file is missing:"];
	[alert setInformativeText:backupPdfPath];
	
	[NSApp activateIgnoringOtherApps:YES]; // put us on top
	int returnButton = [alert runModal];
	
	if ( returnButton == NSAlertFirstButtonReturn )
	{
		// user doesn't want to reconnect right now
		[self revertToUsingDefaultPDF];
			// revertToUsingDefaultPDF might have already been called; it's harmless to call it twice
			// this won't change the backupPdfPath;
		shouldReconnectOnLoad = NO;
	}
	else if ( returnButton == NSAlertSecondButtonReturn )
	{
		// user wants to reconnect right now
		[[NSOpenPanel openPanel]
				beginForDirectory:[backupPdfPath stringByDeletingLastPathComponent]
				file:[backupPdfPath lastPathComponent]
				types:[NSArray arrayWithObjects:@"pdf", @"ai", nil]
				modelessDelegate:self
				didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
				contextInfo:nil];
	}
}

- (void)
handleFileWrittenTo:(NSNotification *)note;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument handleFileWrittenTo:] called") );
#endif
	NSString *fp = [note object];
	if ( [fp isEqualToString:pdfPath] )
	{
		backupPdfPage = currentPage;
		NSString *appName = (NSString *)[[[NSWorkspace sharedWorkspace] activeApplication] objectForKey:@"NSApplicationName"];
		if ( monitorFileForChanges )
		{
			if ( [appName isEqualToString:@"Final Cut Pro HD"] || [appName isEqualToString:@"Final Cut Pro"] || [appName isEqualToString:@"Ampede LCed"] )
			{
				[self presentFileModifiedAlert];
			}
			else
			{
				// Begin a timer that checks for Final Cut Pro's activation, when it activates, we'll present our alert panel
				[self performSelectorOnMainThread:@selector(createFileModifiedTimer) withObject:nil waitUntilDone:NO];
				[self revertToUsingDefaultPDF];
				shouldReconnectOnLoad = NO;
			}
		}
		else
		{
			// don't alert the user that anything has changed
			[self revertToUsingDefaultPDF];
			shouldReconnectOnLoad = NO;
		}
	}
}

- (void)
createFileModifiedTimer;
{
	[NSTimer	scheduledTimerWithTimeInterval:1 // fires every second
				target:self
				selector:@selector(presentFileModifiedAlert:)
				userInfo:nil
				repeats:YES];
}

- (void)
presentFileModifiedAlert:(NSTimer *)theTimer;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument presentFileModifiedAlert] called") );
#endif
	NSString *appName = (NSString *)[[[NSWorkspace sharedWorkspace] activeApplication] objectForKey:@"NSApplicationName"];
	if ( [appName isEqualToString:@"Final Cut Pro HD"] || [appName isEqualToString:@"Final Cut Pro"] || [appName isEqualToString:@"Ampede LCed"] )
	{
		[theTimer invalidate];
		[self presentFileModifiedAlert];
	}
}

- (void)
presentFileModifiedAlert;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument presentFileModifiedAlert] called") );
#endif

	NSAlert *alert = [[NSAlert alloc] init];
	[alert addButtonWithTitle:@"Okay"];
	[alert addButtonWithTitle:@"Reconnect..."];
	[alert setAlertStyle:NSWarningAlertStyle];
	[alert setMessageText:@"The following file was modified:"];
	[alert setInformativeText:backupPdfPath];
	
	[NSApp activateIgnoringOtherApps:YES]; // put us on top
	int returnButton = [alert runModal];
	
	if ( returnButton == NSAlertFirstButtonReturn )
	{
		// user doesn't want to reconnect right now
		[self revertToUsingDefaultPDF];
			// revertToUsingDefaultPDF might have already been called; it's harmless to call it twice
			// this won't change the backupPdfPath
		shouldReconnectOnLoad = NO;
	}
	else if ( returnButton == NSAlertSecondButtonReturn )
	{
		// user wants to reconnect right now
		[[NSOpenPanel openPanel]
				beginForDirectory:[backupPdfPath stringByDeletingLastPathComponent]
				file:[backupPdfPath lastPathComponent]
				types:[NSArray arrayWithObjects:@"pdf", @"ai", nil]
				modelessDelegate:self
				didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
				contextInfo:nil];
	}
}

- (void)
presentFileOutOfDateAlert;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument presentFileMissingAlert] called") );
#endif
	// this apparently can happen
	if ( !backupPdfPath ) return;
	
	NSAlert *alert = [[NSAlert alloc] init];
	[alert addButtonWithTitle:@"Okay"];
	[alert addButtonWithTitle:@"Reconnect..."];
	[alert setAlertStyle:NSWarningAlertStyle];
	[alert setMessageText:@"The following file is out of date:"];
	[alert setInformativeText:backupPdfPath];
	
	[NSApp activateIgnoringOtherApps:YES]; // put us on top
	int returnButton = [alert runModal];
	
	if ( returnButton == NSAlertFirstButtonReturn )
	{
		// user doesn't want to reconnect right now
		[self revertToUsingDefaultPDF];
			// revertToUsingDefaultPDF might have already been called; it's harmless to call it twice
			// this won't change the backupPdfPath;
		shouldReconnectOnLoad = NO;
	}
	else if ( returnButton == NSAlertSecondButtonReturn )
	{
		// user wants to reconnect right now
		if ( !wc ) [self makeWindowControllers];
		else [self addWindowController:wc];
		[[wc window] makeKeyAndOrderFront:nil];

		if ( [NSFileHandle fileHandleForReadingAtPath:backupPdfPath] )
		{
			// the file exists
			[[NSOpenPanel openPanel]
					beginSheetForDirectory:[backupPdfPath stringByDeletingLastPathComponent]
					file:[backupPdfPath lastPathComponent]
					types:[NSArray arrayWithObjects:@"pdf", @"ai", nil]
					modalForWindow:[wc window]
					modalDelegate:self
					didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
					contextInfo:nil];
		}
		else
		{
			[[NSOpenPanel openPanel]
					beginSheetForDirectory:[backupPdfPath stringByDeletingLastPathComponent]
					file:nil
					types:[NSArray arrayWithObjects:@"pdf", @"ai", nil]
					modalForWindow:[wc window]
					modalDelegate:self
					didEndSelector:@selector(openPanelDidEnd:returnCode:contextInfo:)
					contextInfo:nil];
		}
	}
}

- (NSString *)backupPdfPath;
{
	return [[backupPdfPath copy] autorelease];
}

- (void)
openPanelDidEnd:(NSOpenPanel *)sheet
returnCode:(int)returnCode
contextInfo:(void *)contextInfo;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument openPanelDidEnd:returnCode:contextInfo:] called") );
#endif
	NSString *path = [sheet filename];
	
	if ( path ) 
	{
		[self setPdfPath:path];
		[self setCurrentPage:backupPdfPage];
		shouldReconnectOnLoad = YES;
	}
}

- (void)
revertToUsingDefaultPDF;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument revertToUsingDefaultPDF] called") );
#endif
	usingDefaultPDF = YES;
	[self setPdfPath:nil];
	// don't remove the backupPdfPath; we need that when we reconnect
	pdfPath = [[[NSBundle bundleForClass:[self class]] pathForResource:@"default" ofType:@"pdf"] retain];
	[self setPdf:CGPDFDocumentCreateWithURL( (CFURLRef)[NSURL fileURLWithPath:pdfPath] )];
	if ( pdf )
	{
		// this is kind of odd; we're not changing the current page (we want to retain that)
		// but the plug-in tests for a page not found, and substitutes one instead, which is what we want
		[self setCurrentPage:1];
		[self setValue:[NSNumber numberWithInt:CGPDFDocumentGetNumberOfPages( pdf )] forKey:@"pageCount"];
	}
	usingDefaultPDF = YES;
}

//- (void)
//handleFileSizeIncreased:(NSNotification *)note;

- (void)
dealloc;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument dealloc] called") );
#endif
	if ( pdf ) CGPDFDocumentRelease( pdf ); pdf = NULL;
	[wc release]; wc = nil;
	[backgroundColor release]; backgroundColor = nil;
	[super dealloc];
}

- (NSData *)
dataRepresentationOfType:(NSString *)type
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument dataRepresentationOfType:] called") );
#endif
	// Implement to provide a persistent data representation of your document OR remove this and implement the
	// file-wrapper or file path based save methods.
	NSMutableData *mdata = [NSMutableData data];
	NSKeyedArchiver *karchive = [[NSKeyedArchiver alloc] initForWritingWithMutableData:mdata];
	
	// we encode AmpedeDocument's ivars
	[karchive encodeInt:antialias forKey:@"EFAntialias"];
	[karchive encodeInt:shouldSmoothFonts forKey:@"EFShouldSmoothFonts"];
	[karchive encodeInt:intent forKey:@"EFIntent"];
	[karchive encodeInt:quality forKey:@"EFQuality"];
	[karchive encodeInt:drawStyle forKey:@"EFDrawStyle"];
	[karchive encodeInt:superSampling forKey:@"EFSuperSampling"];
	[karchive encodeFloat:flatness forKey:@"EFFlatness"];
	[karchive encodeInt:shouldColorManage forKey:@"EFShouldColorManage"];
	[karchive encodeInt:doSubpixelRendering forKey:@"EFDoSubpixelRendering"];
	
	// and now our own
	[karchive encodeInt:currentPage forKey:@"EFCurrentPage"];
	[karchive encodeInt:pageRotation forKey:@"EFPageRotation"];
	[karchive encodeInt:boxType forKey:@"EFBoxType"];
	[karchive encodeInt:shouldDrawBackground forKey:@"EFShouldDrawBackground"];
	[karchive encodeInt:shouldPreserveAspectRatio forKey:@"EFShouldPreserveAspectRatio"];
	// don't need to encode fileGeneration
	// don't encode individual color components; encode NSColor instead
	[karchive encodeObject:backgroundColor forKey:@"EFBackgroundColor"];
	[karchive encodeBool:shouldReconnectOnLoad forKey:@"EFShouldReconnectOnLoad"];
	[karchive encodeObject:backupPdfPath forKey:@"EFBackupPdfPath"];
	[karchive encodeInt:backupPdfPage forKey:@"EFBackupPdfPage"];
	
	// only encode file name if it's not the default filename
	if ( !usingDefaultPDF ) [karchive encodeObject:pdfPath forKey:@"EFPdfPath"];
	
	[karchive finishEncoding];
    return mdata;
}

- (BOOL)
loadDataRepresentation:(NSData *)data
ofType:(NSString *)type
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument loadDataRepresentation:ofType:] called") );
#endif
    // Implement to load a persistent data representation of your document OR remove this and implement the
	// file-wrapper or file path based load methods.
	NSKeyedUnarchiver *karchive = [[NSKeyedUnarchiver alloc] initForReadingWithData:data];
	
	// decode AmpedeDocument's ivars
	antialias = [karchive decodeIntForKey:@"EFAntialias"];
	shouldSmoothFonts = [karchive decodeIntForKey:@"EFShouldSmoothFonts"];
	intent = [karchive decodeIntForKey:@"EFIntent"];
	quality = [karchive decodeIntForKey:@"EFQuality"];
	drawStyle = [karchive decodeIntForKey:@"EFDrawStyle"];
	superSampling = [karchive decodeIntForKey:@"EFSuperSampling"];
	flatness = [karchive decodeFloatForKey:@"EFFlatness"];
	shouldColorManage = [karchive decodeIntForKey:@"EFShouldColorManage"];
	doSubpixelRendering = [karchive decodeIntForKey:@"EFDoSubpixelRendering"];
	
	// and now decode our own
	currentPage = [karchive decodeIntForKey:@"EFCurrentPage"];
	pageRotation = [karchive decodeIntForKey:@"EFPageRotation"];
	boxType = [karchive decodeIntForKey:@"EFBoxType"];
	shouldDrawBackground = [karchive decodeIntForKey:@"EFShouldDrawBackground"];
	shouldPreserveAspectRatio = [karchive decodeIntForKey:@"EFShouldPreserveAspectRatio"];
	
	// this sets the color components correctly
	[self setBackgroundColor:[karchive decodeObjectForKey:@"EFBackgroundColor"]];
	
	if ( !TRIAL_EXPIRED
#ifdef LICENSING_CONTROL_ON
	|| ( licensingLevelCheck().opMode == OpModeLicensed )
#endif
	)
	{
#ifndef LICENSING_CONTROL_ON
		NSLog(@"TRIAL_EXPIRED is NO");
#endif
		shouldReconnectOnLoad = [karchive decodeBoolForKey:@"EFShouldReconnectOnLoad"];
		backupPdfPath = [[karchive decodeObjectForKey:@"EFBackupPdfPath"] retain];
		backupPdfPage = [karchive decodeIntForKey:@"EFBackupPdfPage"];
		
		// note: currentPage *must* be unarchived and set before setPdfPath: is called
		NSString *archivedPath = [karchive decodeObjectForKey:@"EFPdfPath"];
		if ( shouldReconnectOnLoad )
		{
#ifndef LICENSING_CONTROL_ON
			CFShow( CFSTR("Ampede: reconnecting to media...") );
#endif
			id test = [NSFileHandle fileHandleForReadingAtPath:archivedPath];
			
			if ( test ) [self setPdfPath:archivedPath];
			else
			{
				// the file has moved
				[self presentFileOutOfDateAlert];
			}
		}
		else
		{
			if ( backupPdfPath )
			{
				// begin a reconnect session (which uses backupPdfPath)
				[self presentFileOutOfDateAlert];
			}
		}
	}
	// else leave pdf path set at default
	
	[karchive finishDecoding];
	
	[self invalidate]; // this ensure the plug-in sees our changes
		
    return YES;
}

// this method must be reentrant
- (NSData *)returnDrawData:(NSData *)data;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument returnPDFWithData:] called") );
#endif	
	if ( !drawCacheIsValid )
	{
		[NSThread detachNewThreadSelector:@selector(recomputeDrawCache:) toTarget:self withObject:nil];
		// this could spawn a few extra threads, but at least it will give the correct results and not hurt anything
		
		// we don't want to return right away; otherwise, we'll loop too quickly and end up spawning lots of threads.
		// this way, the time-out is what causes us to be reinvoked if we're not done in time (though I'm not sure why we wouldn't be)
		for (;;)
		{
			if ( drawCacheIsValid ) return drawCache;
			usleep(25);
		}
		return nil; // not reached--keep compiler happy
	}

	return drawCache;
}

- (void)recomputeDrawCache:(id)sender;
{
	// begin thread
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument recomputeDrawCache:] called") );
#endif
	NSMutableData *tmp = [NSMutableData dataWithCapacity:40]; // Create a new instance containing all of our ivars.
	char *rawBytes = (char *)&currentPage; // This is the beginning of our ivars.
	
	// this gets the raw data for all of the ivars we want to send to the plug-in
	[tmp appendBytes:rawBytes length:40];

	[self setDrawCache:tmp];
	drawCacheIsValid = YES;
	
	// end thread
	[pool release];
}

- (void)
setDrawCache:(NSMutableData *)theData;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument setDrawCache:] called") );
#endif
	if ( drawCache != theData )
	{
		id tmp = [theData retain];
		[drawCache autorelease];
		drawCache = tmp;
	}
}

// this method must be reentrant
- (NSData *)returnFileData:(NSData *)data;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument returnFileData:] called") );
#endif	
	if ( !fileCacheIsValid )
	{
		[NSThread detachNewThreadSelector:@selector(recomputeFileCache:) toTarget:self withObject:nil];
		// this could spawn a few extra threads, but at least it will give the correct results and not hurt anything
		
		// we don't want to return right away; otherwise, will loop too quickly and end up spawning lots of threads.
		// this way, the time-out is what causes us to be reinvoked if we're not done in time (though I'm not sure why we wouldn't be)
		for (;;) { if (fileCacheIsValid) return fileCache; usleep(250); }
		return nil; // not reached--keep compiler happy
	}
	else return fileCache;
}

- (void)recomputeFileCache:(id)sender;
{
	// begin thread
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument recomputeFileCache:] called") );
#endif	
	if ( pdfPath )
	{
		if ( fileCacheIsValid ) return; // another thread (running the same method) already filled it
		
		CFDataRef tmp = CFStringCreateExternalRepresentation( NULL, (CFStringRef)pdfPath, kCFStringEncodingUTF8, 0 );

		[self setFileCache:(NSData *)tmp];
		fileCacheIsValid = YES;
		CFRelease( tmp );
	}

	[pool release];
}

- (void)
setFileCache:(NSData *)theData;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument setFileCache:] called") );
#endif
	if ( fileCache != theData )
	{
		id tmp = [theData retain];
		[fileCache autorelease];
		fileCache = tmp;
	}
}

- (void)
setBackupPdfPath:(NSString *)aString;
{
	[backupPdfPath autorelease];
	backupPdfPath = [aString copy];
}

- (NSData *)
showOptionsWithData:(NSData *)theData;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument showOptionsWithData:] called") );
#endif
	[NSApp activateIgnoringOtherApps:YES];
	if ( !wc ) [self makeWindowControllers];
	else [self addWindowController:wc];
	[wc showWindow:nil];
	
	return NULL;
}

- (void)
makeWindowControllers;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument makeWindowControllers] called") );
#endif
	wc = [[SequenceWindowController alloc] initWithWindowNibName:@"SequenceDocument"];
	[self addWindowController:wc];
	[wc window];
}

- (void)
monitorNewPath:(NSString *)newPath;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument monitorNewPath:] called") );
#endif
	if ( newPath ) [kqueue addPathToQueue:newPath];
}

- (BOOL)usingDefaultPDF;
{
	return usingDefaultPDF;
}

- (NSString *)
displayName;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument displayName] called") );
#endif
	if ( usingDefaultPDF ) return @"Ampede PDF";
	else
	{
		NSString *displayName = [pdfPath lastPathComponent];
		return [@"Ampede PDF: " stringByAppendingString:displayName];
	}
}

- (void)
setDisplayName:(NSString *)name;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument setDisplayName:] called") );
#endif
	// here only so that automatic notification of changes works
}

- (NSString *)pdfPath;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument pdfPath] called") );
#endif
	if ( usingDefaultPDF && !backupPdfPath ) return @"Please choose an Adobe Illustrator or PDF file to rasterize.";
	else if ( usingDefaultPDF )
	{
		NSDictionary *attributes = [NSDictionary dictionaryWithObject:[NSColor redColor] forKey:NSForegroundColorAttributeName];
		return [[[NSAttributedString alloc] initWithString:backupPdfPath attributes:attributes] autorelease];
	}
	else return pdfPath;
}

- (void)
setPdfPath:(NSString *)thePath;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument setPdfPath:] called") );
#endif
	if ( ![pdfPath isEqualToString:thePath] )
	{
        [self willChangeValueForKey:@"pdfPath"];

		id tmp = [thePath retain];
		id old = [pdfPath autorelease];
		pdfPath = tmp;
		
		fileGeneration++; // this must be incremented each time we change the file
		
		if ( pdfPath )
		{
			[self setPdf:CGPDFDocumentCreateWithURL( (CFURLRef)[NSURL fileURLWithPath:pdfPath] )];
			if ( pdf )
			{
				CGPDFPageRef thePage = CGPDFDocumentGetPage( pdf, currentPage );
				if (thePage) [self setPage:thePage];
				else 
				{
					[self setCurrentPage:1]; // should always work
				}
				[self setValue:[NSNumber numberWithInt:CGPDFDocumentGetNumberOfPages( pdf )] forKey:@"pageCount"];
			}
			
			if ( usingDefaultPDF )
			{
				[self setUsingDefaultPDF:NO];
				if ( !backupPdfPath) monitorFileForChanges = YES; // we monitor by default, but we don't want to monitor the default pdf, which is the default, and we don't want to force monitoring if we've already passed beyond the default
				[self setBackupPdfPath:pdfPath];
			}
			// only update the backupPdfPath whene we have both a new pdfPath and it's not the default pdf path
			else [self setBackupPdfPath:pdfPath];

			if ( old ) [kqueue removePathFromQueue:old]; // note: it's okay if "old" wasn't being monitored
			[self setMonitorFileForChanges:monitorFileForChanges]; // this will begin monitoring if monitorFileForChanges is on
			NSImage *icon = [[NSWorkspace sharedWorkspace] iconForFile:pdfPath];
			[icon setSize:NSMakeSize( 64, 64 )];
			[self setFileIcon:icon];
		}
		else
		{
			if ( old ) [kqueue removePathFromQueue:old]; // hack to get the old path unmonitored. setMonitorFileForChanges:
														 // will begin monitoring the new path but it has no way to
														 // demonitor the old path
														 // note: it's okay if "old" wasn't being monitored
			[self setFileIcon:nil];
		}
		
		fileCacheIsValid = NO;
		
        [self didChangeValueForKey:@"pdfPath"];
	}
}

- (void)
setFileIcon:(NSImage *)icon;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument setFileIcon:] called") );
#endif
	// don't remove the icon if we're set to nil; we want to keep it for the user
	if ( icon )
	{
		id tmp = [icon retain];
		[fileIcon release];
		fileIcon = tmp;
	}
}

- (void)
setPdf:(CGPDFDocumentRef)thePDF;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument setPdf:] called") );
#endif
    if ( pdf != thePDF )
	{
        [self willChangeValueForKey:@"pdf"];

		CGPDFDocumentRef tmp = NULL;
		if ( thePDF ) CFRetain( thePDF );
		tmp = thePDF;
        if ( pdf ) CFRelease( pdf );
		pdf = thePDF;

        [self didChangeValueForKey:@"pdf"];
    }
}

- (void)
setCurrentPage:(size_t)pageNum;
{
	currentPage = pageNum;
	[self setPage:CGPDFDocumentGetPage( pdf, currentPage )];
}

- (void)
setPage:(CGPDFPageRef)thePage;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument setPage:] called") );
#endif
    if ( page != thePage )
	{
        [self willChangeValueForKey:@"page"];

		CGPDFPageRef tmp = NULL;
		if ( thePage ) CFRetain( thePage );
		tmp = thePage;
        if ( page ) CFRelease( page );
		page = thePage;
		
		[[wc pageView] setPage:page];

        [self didChangeValueForKey:@"page"];
    }
}

- (CGPDFPageRef)page;
{
	return page;
}

- (void)
setBackgroundColor:(NSColor *)theColor;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument setBackgroundColor:] called") );
#endif
	if ( ![theColor isEqual:backgroundColor] )
	{
        [self willChangeValueForKey:@"backgroundColor"];

		id tmp = [theColor retain];
        [backgroundColor release];
		backgroundColor = tmp;
		
		// update the color values we send to the plugin
		
		id rgbColor = [backgroundColor colorUsingColorSpaceName:NSDeviceRGBColorSpace];
		if ( rgbColor ) [rgbColor getRed:&red green:&green blue:&blue alpha:&alpha];
		else CFShow( CFSTR("Ampede warning: unable to convert chosen color to RGB values. Please choose another color.") );
		
        [self didChangeValueForKey:@"backgroundColor"];
	}
}

- (void)
setBoxType:(int)type
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument setBoxType] called") );
#endif	
	boxType = type;
}

- (void)
setShouldPreserveAspectRatio:(int)yn;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument setShouldDrawBackground] called") );
#endif
	if ( shouldPreserveAspectRatio == 0 && yn )
	{
		shouldPreserveAspectRatio = yn;
		[self setBoxType:cachedBoxType];
		return;
	}
	shouldPreserveAspectRatio = yn;
	if ( !yn )
	{
		cachedBoxType = boxType;
		[self setBoxType:-1];
	}
}

- (BOOL)drawDataIsValid;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument drawingDataIsValid] called") );
#endif	
	return drawCacheIsValid;
}

- (int)
minimumPageValue;
{
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("-[SequenceDocument minimumPageValue] called") );
#endif
	return 1;
}

//////////// ColorSync ////////////

- (void)
colorProfileDidChange:(NSNotification *)notification;
{
	CFStringRef string = nil;
	CMProfileRef rgbProfile;
	
    CMGetDefaultProfileBySpace(cmRGBData, (CMProfileRef *)&rgbProfile);
	
    if ( rgbProfile) 
	{
		CMError error;
		
		error = CMCopyProfileLocalizedString((CMProfileRef)rgbProfile, cmProfileDescriptionTag, 0, 0, &string);
		if (error != noErr) {
			error = CMCopyProfileLocalizedString((CMProfileRef)rgbProfile, cmProfileDescriptionMLTag, 0,0, &string);
			if (error != noErr) {
				Str255 pName;
				ScriptCode code;
				
				CMGetScriptProfileDescription((CMProfileRef)rgbProfile, pName, &code);
				string = CFStringCreateWithPascalString(0, pName, code);
			}
		}
		CMCloseProfile(rgbProfile);
	}
    [self setProfileName:[(NSString *)string autorelease]];
}

- (void)
setProfileName:(NSString *)theName;
{
	[profileName autorelease];
	profileName = [theName copy];
}

- (int)
trialType;
{
	return ( TRIAL_EXPIRED ) ? 2 : 1;
}

@end
