//
//  PDFObjectBuilder.h
//  Ampede
//
//  Created by Eric Ocean on Thu Jul 15 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#import "PDFParsingProtocol.h"


// This is an abstract class, used primarily to provide default implementations of the PDFParsing protocol.
// It also provides method logging facilities (see the .m for the define that turns it on and off)

@interface PDFObjectBuilder : NSObject < PDFParsing >
{
	
}

@end
