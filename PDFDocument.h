//
//  PDFDocument.h
//  Ampede
//
//  Created by Eric Ocean on Thu Jul 15 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@class PDFView;

@interface PDFDocument : NSDocument 
{
	IBOutlet PDFView *pdfView;
	size_t currentPage;
	size_t pageCount;
	CGPDFDocumentRef pdf;
	CGPDFPageRef page;
	int width; // in pixels
	int height; // in pixels
}

////////// Handling the default CFMessagePort /////////////////////////////////////////////////////////////////////////

- (NSData *)
handleMessage:(SInt32)msgid
withData:(NSData *)data;

- (void)
messagePortInvalidated:(CFMessagePortRef)ms;

@end