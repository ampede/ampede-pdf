//
//  PDFParsingProtocolDebug.h
//  Ampede
//
//  Created by Eric Ocean on Fri Jul 16 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

// comment out the LOG_PDFPARSING_CALLBACKS define below to prevent logging
#define LOG_PDFPARSING_CALLBACKS


void logPDFParsing( SEL theSelector, id theObject );

#ifdef LOG_PDFPARSING_CALLBACKS

	#warning LOG_PDFPARSING_CALLBACKS is enabled
	void logPDFParsing( SEL theSelector, id theObject )
	{
		NSLog(@"%@ called on %@", NSStringFromSelector( theSelector ), NSStringFromClass( [theObject class] ) );
	}

#else

	void logPDFParsing( SEL theSelector, id theObject ) { }

#endif

#define LOG_PDFPARSING_CALLBACKS_IMP logPDFParsing( _cmd, self );

