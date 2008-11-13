//
//  AIDocument.h
//  Ampede
//
//  Created by Eric Ocean on Tue Jul 13 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "PDFDocument.h"


@interface AIDocument : NSDocument
{
	NSData *fileData;
	NSMutableArray *layers;
	CGPDFDocumentRef pdf;
	CGPDFDictionaryRef catalog;
	CGPDFPageRef page;
	CGPDFDictionaryRef pageDict;
	CGPDFStreamRef stream;
	CGPDFDictionaryRef streamDict;
	NSData *streamData;
}

@end
