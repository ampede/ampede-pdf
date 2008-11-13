//
//  SequenceWindowController.h
//  Ampede
//
//  Created by Eric Ocean on Wed Jul 28 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class PDFView;

typedef enum {
	kFileInfoPaneTag = 0,
	kPDFPaneTag,
	kQuartzPaneTag,
	kColorsyncPane,
	kPreferencesPane,
	kSoftwareUpdatePane,
	kLicensingPane
} SequencePaneTag;

typedef enum {
	kAboutAmpedePDF = 0,
	kQuickStartTutorial = 1,
	kFilterControls = 2,
	kTheFilePane = 3,
	kThePDFPane = 4,
	kTheQuartzPane = 5,
	kTheColorSyncPane = 6,
	kTheSoftwareUpdatePane = 7,
	kTheLicensingPane = 8,
	kTipsAndTricks = 9
} HelpTag;

@interface SequenceWindowController : NSWindowController
{
	NSToolbar *toolbar;
	
	NSView *blankView;
	IBOutlet NSView *fileInfoPane;
	IBOutlet NSView *pdfPane;
	IBOutlet NSView *quartzPane;
	IBOutlet NSView *softwareUpdatePane;
	IBOutlet NSView *preferencesPane;
	IBOutlet NSView *colorsyncPane;
	IBOutlet NSView *licensingPane;
	SequencePaneTag currentPane;
	NSSize fileInfoPaneSize, pdfPaneSize, quartzPaneSize, softwareUpdatePaneSize,
			preferencesPaneSize, colorsyncPaneSize, licensingPaneSize;
			
	IBOutlet PDFView *pageView;
//	IBOutlet PDFBoxView *backgroundView;

	BOOL shouldDestroyWindowHidingTimer;
}

- (IBAction)openHelp:(id)sender;

- (IBAction)choosePDF:(id)sender;
- (IBAction)openInDefaultViewer:(id)sender;
- (IBAction)reconnect:(id)sender;
- (IBAction)launchLicensing:(id)sender;
- (IBAction)launchInfoWebPage:(id)sender;
- (IBAction)launchPurchaseWebPage:(id)sender;

- (void)
selectPane:(NSView *)thePane
withTag:(int)theTag
size:(NSSize)theSize;

- (void)
setupToolbar;

- (int)
tagForItemIdentifier:(NSString *)itemIdent;

- (void)
resizeWindowToSize:(NSSize)newSize;

- (PDFView *)pageView;

- (IBAction)launchColorSyncUtility:(id)sender;

@end
