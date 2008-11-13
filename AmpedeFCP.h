//
//  AmpedeFCP.h
//  Ampede
//
//  Created by Eric Ocean on Sun Jul 25 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//


#ifndef AMPEDE_FCP_H
#define AMPEDE_FCP_H

#pragma once

#include "AE_Effect.h"
#include "A.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "DrawingEnums.h"

// From CallMachOFramework sample code
#include "MoreSetup.h"

#include <MacTypes.h>
#include <CFBundle.h>
#include <Gestalt.h>
#include <Folders.h>

// #include "Statechart_MACROS.h"

#define	MAJOR_VERSION		2
#define	MINOR_VERSION		0
#define	BUG_VERSION			0
#define	STAGE_VERSION		PF_Stage_DEVELOP
#define	BUILD_VERSION		1

#define	NAME "Ampede EF"
#define DESCRIPTION	\
"Yeah, it works."

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Launch Services defines
//

// type definitions from LSInfo.h, LSOpen.h
typedef OptionBits LSLaunchFlags;enum {
	kLSLaunchDefaults = 0x00000001,
	kLSLaunchAndPrint = 0x00000002,
	kLSLaunchReserved2 = 0x00000004,
	kLSLaunchReserved3 = 0x00000008,
	kLSLaunchReserved4 = 0x00000010,
	kLSLaunchReserved5 = 0x00000020,
	kLSLaunchReserved6 = 0x00000040,
	kLSLaunchInhibitBGOnly = 0x00000080,
	kLSLaunchDontAddToRecents = 0x00000100,
	kLSLaunchDontSwitch = 0x00000200,
	kLSLaunchNoParams = 0x00000800,
	kLSLaunchAsync = 0x00010000,
	kLSLaunchStartClassic = 0x00020000,
	kLSLaunchInClassic = 0x00040000,
	kLSLaunchNewInstance = 0x00080000,
	kLSLaunchAndHide = 0x00100000,
	kLSLaunchAndHideOthers = 0x00200000
};
struct LSLaunchURLSpec {
	CFURLRef appURL;
	CFArrayRef itemURLs;
//  const AEDesc *passThruParams;
	void *passThruParams; // for some reason, the above won't get past the compiler
	LSLaunchFlags launchFlags;
	void *asyncRefCon;
};
typedef struct LSLaunchURLSpec LSLaunchURLSpec;
enum {
	kLSUnknownType = 0,
	kLSUnknownCreator = 0
};

// pointer-to-function definitions for our LaunchServices functions
typedef OSStatus (*LSFindApplicationForInfoPtr)( OSType, CFStringRef, CFStringRef, FSRef *, CFURLRef * );
typedef OSStatus (*LSOpenFromURLSpecPtr)( const LSLaunchURLSpec *, CFURLRef * );

//
//	CFMessagePort defines
//

// type definitions from CFMessagePort.h
typedef double CFTimeInterval;
typedef struct __CFMessagePort *CFMessagePortRef;
typedef struct __CFRunLoopSource *CFRunLoopSourceRef;
typedef void (*CFMessagePortInvalidationCallBack)( CFMessagePortRef ms, void *info );

// pointer-to-function definitions for our Mach-O CFMessagePort functions
typedef CFMessagePortRef (*CFMessagePortCreateRemotePtr)( CFAllocatorRef, CFStringRef );
typedef void (*CFMessagePortSetInvalidationCallBackPtr)( CFMessagePortRef, CFMessagePortInvalidationCallBack );
typedef SInt32 (*CFMessagePortSendRequestPtr)( CFMessagePortRef, SInt32, CFDataRef, CFTimeInterval, CFTimeInterval, CFStringRef, CFDataRef* );
typedef Boolean (*CFMessagePortIsValidPtr)( CFMessagePortRef );

typedef enum {
   kCFMessagePortSuccess = 0,
   kCFMessagePortSendTimeout = -1,
   kCFMessagePortReceiveTimeout = -2,
   kCFMessagePortIsInvalid = -3,
   kCFMessagePortTransportError = -4
} MessagePortError;

//
//	BSD defines
//

typedef int (*usleepPtr)(unsigned int microseconds);
void *malloc(size_t size);

//
//	vImage defines
//

typedef struct vImage_Buffer {
    void        *data;      /* A pointer to the top left pixel of the buffer */
    u_int32_t   height;     /* The height (in pixels) of the buffer */
    u_int32_t   width;      /* The width (in pixels) of the buffer */
    u_int32_t   rowBytes;   /* The number of bytes in a pixel row */
} vImage_Buffer;

/* vImage errors */
typedef int32_t vImage_Error;
enum {
   kvImageNoError                  =   0,
   kvImageRoiLargerThanInputBuffer =   -21766,
   kvImageInvalidKernelSize        =   -21767,
   kvImageNoEdgeStyleSpecified     =   -21768,
   kvImageInvalidOffset_X          =   -21769,
   kvImageInvalidOffset_Y          =   -21770,
   kvImageMemoryAllocationError    =   -21771,
   kvImageNullPointerArgument      =   -21772,
   kvImageInvalidParameter         =   -21773,
   kvImageBufferSizeMismatch       =   -21774
};

/* vImage flags */
typedef u_int32_t vImage_Flags;
enum {
    kvImageNoFlags              =   0,
    kvImageLeaveAlphaUnchanged  =   1,
		/* Operate on red, green and blue channels only. Alpha is copied from source to destination. For Interleaved formats only. */
    kvImageCopyInPlace          =   2,  /* Copy edge pixels */
    kvImageBackgroundColorFill  =   4,  /* Use a background color */
    kvImageEdgeExtend           =   8,  /* Extend border data elements */
    kvImageDoNotTile            =   16,
		/* Pass to turn off internal tiling. Use this if you want to do your own tiling, or to use the Min/Max filters in place. */
    kvImageHighQualityResampling =  32
		/* Use a higher quality, slower resampling filter for Geometry operations (shear, scale, rotate, affine transform, etc.) */
};

//
// ColorSync defines
//

typedef struct OpaqueCMProfileRef * CMProfileRef;

enum {
   cmXYZData = 'XYZ ',
   cmLabData = 'Lab ',
   cmLuvData = 'Luv ',
   cmYCbCrData = 'YCbr',
   cmYxyData = 'Yxy ',
   cmRGBData = 'RGB ',
   cmSRGBData = 'sRGB',
   cmGrayData = 'GRAY',
   cmHSVData = 'HSV ',
   cmHLSData = 'HLS ',
   cmCMYKData = 'CMYK',
   cmCMYData = 'CMY ',
   cmMCH5Data = 'MCH5',
   cmMCH6Data = 'MCH6',
   cmMCH7Data = 'MCH7',
   cmMCH8Data = 'MCH8',
   cm3CLRData = '3CLR',
   cm4CLRData = '4CLR',
   cm5CLRData = '5CLR',
   cm6CLRData = '6CLR',
   cm7CLRData = '7CLR',
   cm8CLRData = '8CLR',
   cm9CLRData = '9CLR',
   cm10CLRData = 'ACLR',
   cm11CLRData = 'BCLR',
   cm12CLRData = 'CCLR',
   cm13CLRData = 'DCLR',
   cm14CLRData = 'ECLR',
   cm15CLRData = 'FCLR',
   cmNamedData = 'NAME'
};

//
// CoreGraphics function prototypes
//

enum CGInterpolationQuality {
   kCGInterpolationDefault,
   kCGInterpolationNone,
   kCGInterpolationLow,
   kCGInterpolationHigh
};
typedef enum CGInterpolationQuality CGInterpolationQuality;

enum CGPDFBox {
    kCGPDFMediaBox = 0,
    kCGPDFCropBox = 1,
    kCGPDFBleedBox = 2,
    kCGPDFTrimBox = 3,
    kCGPDFArtBox = 4
};
typedef enum CGPDFBox CGPDFBox;

typedef struct CGContext * CGContextRef;
typedef struct CGColorSpace *CGColorSpaceRef;
typedef struct CGPDFDocument *CGPDFDocumentRef;
typedef struct CGPDFPage *CGPDFPageRef;
typedef struct CGColor *CGColorRef;

CGRect CGPDFPageGetBoxRect ( CGPDFPageRef page, CGPDFBox box );
CGColorRef CGColorCreate ( CGColorSpaceRef colorspace, const float components[] );
CGAffineTransform CGPDFPageGetDrawingTransform ( CGPDFPageRef page, CGPDFBox box, CGRect rect, int rotate, int preserveAspectRatio );


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum {
	BASE = 0,	// default input layer
	BASIC_VECTOR_MOTION_TOPIC,
	AMPEDE_SCALE,
	AMPEDE_SCALE_MULTIPLIER,
	AMPEDE_ROTATION,
	AMPEDE_CENTER,
	AMPEDE_ANCHOR_POINT,
	AMPEDE_FORCE_RERENDER,
	AMPEDE_NUM_PARAMS
};

enum {
	SCALE_DISK_ID = 1,
	SCALE_MULTIPLIER_DISK_ID,
	ROTATION_DISK_ID,
	CENTER_DISK_ID,
	ANCHOR_POINT_DISK_ID,
	BASIC_VECTOR_MOTION_TOPIC_DISK_ID,
	FORCE_UPDATE_DISK_ID
};

#define	SCALE_MIN		(1)	
#define	SCALE_MAX		(1000)
#define	SCALE_BIG_MAX	(10000)
#define	SCALE_DFLT		(100)

#define	SCALE_MULTIPLIER_MIN		(6553)	// 0.1 fixed
#define	SCALE_MULTIPLIER_MAX		(10L << 16)
#define	SCALE_MULTIPLIER_BIG_MAX	(100L << 16)
#define	SCALE_MULTIPLIER_DFLT		(1L << 16)

#define	ROTATION_DFLT   (0)

#define CENTER_X_DFLT			(50)
#define CENTER_Y_DFLT			(50)
#define CENTER_RESTRICT_BOUNDS  (0)

#define ANCHOR_POINT_X_DFLT				(50)
#define ANCHOR_POINT_Y_DFLT				(50)
#define ANCHOR_POINT_RESTRICT_BOUNDS	(0)

#define FORCE_UPDATE_DFLT	(0)

PF_Err 
main (
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output );

typedef struct {
	size_t currentPage;
	int pageRotation;
	int boxType;
	int shouldDrawPageBackground;
	int shouldPreserveAspectRatio;
	int fileGeneration; // incremented each time a new file is chosen, relinked, etc.
	float red;
	float green;
	float blue;
	float alpha;
} AmpedeDRAW;

typedef struct {
	int antialias;
	int shouldSmoothFonts;
	int intent;
	int quality;
	int drawStyle;
	int superSampling;
	float flatness;
	int shouldColorManage;
	int doSubpixelRendering;
	int forceRerender;
	int pdfDataFresh;
} AmpedeDATA;

typedef struct {
	int magic; // set to NULL if sequenced data is unflattened data; set to 0xBABECAFE if set to flattened data

	int width;
	int height;
	float scale;
	float scaleMultiplier;
	float rotation;
	float centerX;
	float centerY;
	float anchorX;
	float anchorY;
	float pixelAspectRatio;
	
	CGContextRef bitmapContext;
	CGContextRef frameCache;
	vImage_Buffer sourceBuffer;
	CGPDFDocumentRef pdf;
	CGPDFPageRef pdfPage;
	void *tmpBuf;
	
	// these are from AmpedeDocument
	int antialias;
	int shouldSmoothFonts;
	int intent;
	int quality;
	int drawStyle;
	int superSampling;
	float scaleMultiple;
	float flatness;
	int shouldColorManage;
	int doSubpixelRendering;

	// these are from SequenceDocument
	size_t currentPage;
	int pageRotation;
	int boxType;
	int shouldDrawPageBackground;
	int shouldPreserveAspectRatio;
	int fileGeneration;
	float red;
	float green;
	float blue;
	float alpha;
	
} AmpedeFrameInfo;

typedef struct {
	AmpedeFrameInfo frameInfo;	
	CFMessagePortRef messagePort;	
} AmpedeSequenceData;


static PF_Err 
DoDialog (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output );

static PF_Err 
About (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output );

static PF_Err 
GlobalSetup (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output );

static PF_Err 
ParamsSetup (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output );

static PF_Err 
SequenceSetup (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output );

static PF_Err 
SequenceSetdown (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output );

static PF_Err 
SequenceResetup (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output );

static PF_Err 
FrameSetup (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output );

static PF_Err 
Render (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output );

static PF_Err 
SequenceFlatten (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output );

static PF_Err 
GlobalSetdown (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output );
	
vImage_Error InitvImage_Buffer( vImage_Buffer *result, int height, int width, size_t bytesPerPixel );
void FreevImage_Buffer( vImage_Buffer *buffer );

static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr);
void *MachOFunctionPointerForCFMFunctionPointer( void *cfmfp );

typedef const struct __CFNumber *CFNumberRef;

enum CFNumberType {
   kCFNumberSInt8Type = 1,
   kCFNumberSInt16Type = 2,
   kCFNumberSInt32Type = 3,
   kCFNumberSInt64Type = 4,
   kCFNumberFloat32Type = 5,
   kCFNumberFloat64Type = 6,
   kCFNumberCharType = 7,
   kCFNumberShortType = 8,
   kCFNumberIntType = 9,
   kCFNumberLongType = 10,
   kCFNumberLongLongType = 11,
   kCFNumberFloatType = 12,
   kCFNumberDoubleType = 13,
   kCFNumberCFIndexType = 14,
   kCFNumberMaxType = 14
};
typedef enum CFNumberType CFNumberType;

CFNumberRef CFNumberCreate (
   CFAllocatorRef allocator,
   CFNumberType theType,
   const void *valuePtr
);

#endif // AMPEDE_FCP_H