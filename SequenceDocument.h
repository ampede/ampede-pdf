//
//  SequenceDocument.h
//  Ampede
//
//  Created by Eric Ocean on Sun Jul 25 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "AmpedeDocument.h"
#import "SequenceWindowController.h"


@class UKKQueue;

@interface SequenceDocument : AmpedeDocument
{
	////////// DO NOT MODIFY OR REARRANGE THIS DATA //////////
	size_t currentPage;
	int pageRotation;
	int boxType;
	int shouldDrawBackground;
	int shouldPreserveAspectRatio;
	int fileGeneration; // incremented each time a new file is chosen, relinked, etc.
	float red;
	float green;
	float blue;
	float alpha;
	////////// DO NOT MODIFY OR REARRANGE THIS DATA //////////
		
	NSMutableData *drawCache;
	BOOL drawCacheIsValid;
	NSData *fileCache;
	BOOL fileCacheIsValid;
	BOOL usingDefaultPDF;
	
	size_t pageCount;
	NSColor *backgroundColor;
	NSString *pdfPath;
	CGPDFDocumentRef pdf;
	CGPDFPageRef page;
	SequenceWindowController *wc;
	BOOL shouldRequestPDF;
	BOOL requestedFirstPDF;
	UKKQueue *kqueue;
	BOOL monitorFileForChanges;
	NSImage *fileIcon;
	BOOL shouldReconnectOnLoad;
	NSString *backupPdfPath;
	int backupPdfPage;
	
	int cachedBoxType;
	
	NSString *profileName;
}

// Subclass overrides from AmpedeDocument
- (NSData *)returnDrawData:(NSData *)data;
- (NSData *)showOptionsWithData:(NSData *)theData;
//- (NSData *)cacheImageData:(NSData *)theData;
//- (NSData *)returnCacheForData:(NSData *)theData;
- (BOOL)drawDataIsValid;

- (int)minimumPageValue;
- (BOOL)usingDefaultPDF;

- (void)setPdf:(CGPDFDocumentRef)thePDF;
- (CGPDFPageRef)page;
- (void)setPage:(CGPDFPageRef)thePage;
- (void)setDrawCache:(NSMutableData *)theData;
- (void)setFileCache:(NSData *)theData;
- (void)setPdfPath:(NSString *)thePath;
- (NSString *)pdfPath;
- (void)setBackgroundColor:(NSColor *)theColor;
- (void)setBackupPdfPath:(NSString *)aString;

- (void)useDefaultPdf;
- (void)setPdfPath:(NSString *)thePath;
- (void)setUsingDefaultPDF:(BOOL)yn;
- (void)setMonitorFileForChanges:(BOOL)yn;
- (void)setFileIcon:(NSImage *)icon;
- (void)setCurrentPage:(size_t)pageNum;
- (NSString *)backupPdfPath;

- (void)monitorNewPath:(NSString *)newPath;

- (void)colorProfileDidChange:(NSNotification *)notification;
- (void)setProfileName:(NSString *)theName;

- (void)presentFileMissingAlert:(NSTimer *)theTimer;
- (void)presentFileMissingAlert;
- (void)presentFileModifiedAlert:(NSTimer *)theTimer;
- (void)presentFileModifiedAlert;
- (void)revertToUsingDefaultPDF;
- (void)presentFileOutOfDateAlert;

@end
