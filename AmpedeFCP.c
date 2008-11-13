//
//  AmpedeFCP.c
//  Ampede
//
//  Created by Eric Ocean on Sun Jul 25 2004.
//  Copyright (c) 2004 Eric Daniel Ocean. All rights reserved.
//

#include "AmpedeFCP.h"
#include <CFUUID.h>
#include "QuartzImports.h"

//#define DEBUG_VALUES
//#define DEBUG_CALLING_SEQUENCE
//#define DEBUG_ABORTS
//#define DEBUG_FORCE_RENDER

#define LICENSING_CONTROL_ON

// From CFM_MachO_CFM.c
//
//	This function allocates a block of CFM glue code which contains the instructions to call CFM routines
//

UInt32 template[6] = {0x3D800000, 0x618C0000, 0x800C0000, 0x804C0004, 0x7C0903A6, 0x4E800420};

void *MachOFunctionPointerForCFMFunctionPointer( void *cfmfp )
{
    UInt32	*mfp = (UInt32*) NewPtr( sizeof(template) ); //	Must later dispose of allocated memory
    mfp[0] = template[0] | ((UInt32)cfmfp >> 16);
    mfp[1] = template[1] | ((UInt32)cfmfp & 0xFFFF);
    mfp[2] = template[2];
    mfp[3] = template[3];
    mfp[4] = template[4];
    mfp[5] = template[5];
    MakeDataExecutable( mfp, sizeof(template) );
    return( mfp );
}


// From CallMachOFramework.c
//

static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr)
	// This routine finds a the named framework and creates a CFBundle 
	// object for it.  It looks for the framework in the frameworks folder, 
	// as defined by the Folder Manager.  Currently this is 
	// "/System/Library/Frameworks", but we recommend that you avoid hard coded 
	// paths to ensure future compatibility.
	//
	// You might think that you could use CFBundleGetBundleWithIdentifier but 
	// that only finds bundles that are already loaded into your context. 
	// That would work in the case of the System framework but it wouldn't 
	// work if you're using some other, less-obvious, framework.
{
	OSStatus 	err;
	FSRef 		frameworksFolderRef;
	CFURLRef	baseURL;
	CFURLRef	bundleURL;
	
	MoreAssertQ(bundlePtr != nil);
	
	*bundlePtr = nil;
	
	baseURL = nil;
	bundleURL = nil;
	
	// Find the frameworks folder and create a URL for it.
	
	err = FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, true, &frameworksFolderRef);
	if (err == noErr) {
		baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
		if (baseURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	
	// Append the name of the framework to the URL.
	
	if (err == noErr) {
		bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, framework, false);
		if (bundleURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	
	// Create a bundle based on that URL and load the bundle into memory.
	// We never unload the bundle, which is reasonable in this case because 
	// the sample assumes that you'll be calling functions from this 
	// framework throughout the life of your application.
	
	if (err == noErr) {
		*bundlePtr = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
		if (*bundlePtr == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
	    if ( ! CFBundleLoadExecutable( *bundlePtr ) ) {
			err = coreFoundationUnknownErr;
	    }
	}

	// Clean up.
	
	if (err != noErr && *bundlePtr != nil) {
		CFRelease(*bundlePtr);
		*bundlePtr = nil;
	}
	if (bundleURL != nil) {
		CFRelease(bundleURL);
	}	
	if (baseURL != nil) {
		CFRelease(baseURL);
	}	
	
	return err;
}

// our supervisor message port invalidation callback implementation
void AmpedeMessagePortInvalidationCallback ( CFMessagePortRef ms,  void *info )
{
	#pragma unused ( ms, info )
//	CFShow( CFSTR("AmpedeMessagePortInvalidationCallback called") );
}

// our Mach-O to CFM glue code
static CFMessagePortInvalidationCallBack ampedeMessagePortInvalidationCallbackPtr;


static CFBundleRef lsBundle;
static LSFindApplicationForInfoPtr LSFindApplicationForInfo;
static LSOpenFromURLSpecPtr LSOpenFromURLSpec;

static CFBundleRef cfBundle;
static CFMessagePortCreateRemotePtr CFMessagePortCreateRemote;
static CFMessagePortSetInvalidationCallBackPtr CFMessagePortSetInvalidationCallBack;
static CFMessagePortSendRequestPtr CFMessagePortSendRequest;
static CFMessagePortIsValidPtr CFMessagePortIsValid;
static CFStringRef *kCFRunLoopDefaultMode;
static CFStringRef *kCFRunLoopCommonModes;

static CFBundleRef sysBundle;
static usleepPtr usleep;

static CFMessagePortRef ampedeMsgPort;

OSStatus LaunchAmpede()
{
	const LSLaunchFlags launchFlags = 0 | kLSLaunchDontAddToRecents | kLSLaunchDontSwitch;
	LSLaunchURLSpec launchSpec = { NULL, NULL, NULL, launchFlags, NULL };
	OSStatus err = noErr;
	
	err = LoadCoreGraphicsPointers();
	
	if ( err != noErr) return err;
	
	err = LoadFrameworkBundle(CFSTR("ApplicationServices.framework"), &lsBundle);

	if ( err == noErr )
	{
		LSFindApplicationForInfo = (LSFindApplicationForInfoPtr) CFBundleGetFunctionPointerForName( lsBundle, CFSTR("LSFindApplicationForInfo") );
		if (LSFindApplicationForInfo == nil)  err = cfragNoSymbolErr;
	
		LSOpenFromURLSpec = (LSOpenFromURLSpecPtr) CFBundleGetFunctionPointerForName( lsBundle, CFSTR("LSOpenFromURLSpec") );
		if (LSOpenFromURLSpec == nil)  err = cfragNoSymbolErr;
	}
	
	if (err != noErr)
	{
		CFShow( CFSTR("Ampede error: failed to acquire Launch Services function pointers.") );
		return err;
	}
	
	err = LoadFrameworkBundle(CFSTR("System.framework"), &sysBundle);

	if ( err == noErr )
	{
		usleep = (usleepPtr) CFBundleGetFunctionPointerForName( sysBundle, CFSTR("usleep") );
		if (usleep == nil)  err = cfragNoSymbolErr;
	}
	
	if (err != noErr)
	{
		CFShow( CFSTR("Ampede error: failed to acquire System.framework function pointers.") );
		return err;
	}
	
#ifdef LICENSING_CONTROL_ON
	launchSpec.appURL = CFURLCreateWithFileSystemPath( kCFAllocatorDefault, CFSTR("/Library/Application Support/Final Cut Pro System Support/Plugins/Ampede PDF.bundle/Contents/Ampede LCed.app"), kCFURLPOSIXPathStyle, true );
#else
	launchSpec.appURL = CFURLCreateWithFileSystemPath( kCFAllocatorDefault, CFSTR("/Users/bizman/Build/Ampede.app"), kCFURLPOSIXPathStyle, true );
#endif

	if ( err ) // we'll give this a try as a last resort
	{
		err = LSFindApplicationForInfo(		kLSUnknownCreator,
											NULL, // CFSTR("com.ampede.EF.app"),
											CFSTR("Ampede LCed.app"), // NULL,
											NULL,
											&launchSpec.appURL
										);
	}
	
	if ( err == noErr )
	{
		err = LSOpenFromURLSpec( &launchSpec, NULL );
		
		if ( err != noErr )
			CFShow( CFSTR("Ampede error: Launch Services failed to launch the Ampede application.") );
	}
	else
	{
#ifdef DEBUG_VALUES
		CFShow( CFSTR("The error returned from LSFindApplicationForInfo is:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &err );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
		CFShow( CFSTR("Ampede fatal error: Launch Services couldn't find the Ampede PDF application.") );
	}
	return err;
}

//
// based on code from CallMachOFramework.c
//

OSStatus LoadCFMessagePortPointers()
{
	OSStatus err = noErr;
	
	err = LoadFrameworkBundle(CFSTR("CoreFoundation.framework"), &cfBundle);

	if ( err == noErr )
	{
		CFMessagePortCreateRemote = (CFMessagePortCreateRemotePtr) CFBundleGetFunctionPointerForName( cfBundle, CFSTR("CFMessagePortCreateRemote") );
		if (CFMessagePortCreateRemote == nil)  err = cfragNoSymbolErr;

		CFMessagePortSetInvalidationCallBack = (CFMessagePortSetInvalidationCallBackPtr) CFBundleGetFunctionPointerForName( cfBundle, CFSTR("CFMessagePortSetInvalidationCallBack") );
		if (CFMessagePortSetInvalidationCallBack == nil)  err = cfragNoSymbolErr;

		CFMessagePortSendRequest = (CFMessagePortSendRequestPtr) CFBundleGetFunctionPointerForName( cfBundle, CFSTR("CFMessagePortSendRequest") );
		if (CFMessagePortSendRequest == nil)  err = cfragNoSymbolErr;

		CFMessagePortIsValid = (CFMessagePortIsValidPtr) CFBundleGetFunctionPointerForName( cfBundle, CFSTR("CFMessagePortIsValid") );
		if (CFMessagePortIsValid == nil)  err = cfragNoSymbolErr;
		
		kCFRunLoopDefaultMode = (CFStringRef *) CFBundleGetDataPointerForName( cfBundle, CFSTR("kCFRunLoopDefaultMode") );
		if (kCFRunLoopDefaultMode == nil)  err = cfragNoSymbolErr;

		kCFRunLoopCommonModes = (CFStringRef *) CFBundleGetDataPointerForName( cfBundle, CFSTR("kCFRunLoopCommonModes") );
		if (kCFRunLoopCommonModes == nil)  err = cfragNoSymbolErr;
	}
	
	// make mach-O to CFM glue
	ampedeMessagePortInvalidationCallbackPtr = NULL;
	ampedeMessagePortInvalidationCallbackPtr = MachOFunctionPointerForCFMFunctionPointer( AmpedeMessagePortInvalidationCallback );

	if (err != noErr) CFShow( CFSTR("Ampede error: LoadCFMessagePortPointers() failed.") );
	
	return err;
}


//
// Begin After Effects SDK functions
//

PF_Err main (	PF_Cmd			cmd,
				PF_InData		*in_data,
				PF_OutData		*out_data,
				PF_ParamDef		*params[],
				PF_LayerDef		*output )
{
	PF_Err		err = PF_Err_NONE;
	
	switch (cmd) {
	case PF_Cmd_RENDER:
		err = Render(in_data,out_data,params,output);
#ifdef DEBUG_ERRORS
		if ( err != PF_Err_NONE ) CFShow( CFSTR( "Ampeded error: PF_Cmd_RENDER encountered an error." ) );
#endif
		break;
	case PF_Cmd_ABOUT:
		err = About(in_data,out_data,params,output);
#ifdef DEBUG_ERRORS
		if ( err != PF_Err_NONE ) CFShow( CFSTR( "Ampeded error: PF_Cmd_ABOUT encountered an error." ) );
#endif
		break;
	case PF_Cmd_GLOBAL_SETUP:
		err = GlobalSetup(in_data,out_data,params,output);
#ifdef DEBUG_ERRORS
		if ( err != PF_Err_NONE ) CFShow( CFSTR( "Ampeded error: PF_Cmd_GLOBAL_SETUP encountered an error." ) );
#endif
		break;
	case PF_Cmd_PARAMS_SETUP:
		err = ParamsSetup(in_data,out_data,params,output);
#ifdef DEBUG_ERRORS
		if ( err != PF_Err_NONE ) CFShow( CFSTR( "Ampeded error: PF_Cmd_PARAMS_SETUP encountered an error." ) );
#endif
		break;
	case PF_Cmd_SEQUENCE_SETUP:
		err = SequenceSetup(in_data,out_data,params,output);
#ifdef DEBUG_ERRORS
		if ( err != PF_Err_NONE ) CFShow( CFSTR( "Ampeded error: PF_Cmd_SEQUENCE_SETUP encountered an error." ) );
#endif
		break;
	case PF_Cmd_SEQUENCE_SETDOWN:
		err = SequenceSetdown(in_data,out_data,params,output);
#ifdef DEBUG_ERRORS
		if ( err != PF_Err_NONE ) CFShow( CFSTR( "Ampeded error: PF_Cmd_SEQUENCE_SETDOWN encountered an error." ) );
#endif
		break;
	case PF_Cmd_SEQUENCE_RESETUP:
		err = SequenceResetup(in_data,out_data,params,output);
#ifdef DEBUG_ERRORS
		if ( err != PF_Err_NONE ) CFShow( CFSTR( "Ampeded error: PF_Cmd_SEQUENCE_RESETUP encountered an error." ) );
#endif
		break;
	case PF_Cmd_SEQUENCE_FLATTEN:
		err = SequenceFlatten(in_data,out_data,params,output);
#ifdef DEBUG_ERRORS
		if ( err != PF_Err_NONE ) CFShow( CFSTR( "Ampeded error: PF_Cmd_SEQUENCE_FLATTEN encountered an error." ) );
#endif
		break;
	case PF_Cmd_FRAME_SETUP:
		err = FrameSetup(in_data,out_data,params,output);
#ifdef DEBUG_ERRORS
		if ( err != PF_Err_NONE ) CFShow( CFSTR( "Ampeded error: PF_Cmd_FRAME_SETUP encountered an error." ) );
#endif
		break;
	case PF_Cmd_GLOBAL_SETDOWN:
		err = GlobalSetdown(in_data,out_data,params,output);
#ifdef DEBUG_ERRORS
		if ( err != PF_Err_NONE ) CFShow( CFSTR( "Ampeded error: PF_Cmd_GLOBAL_SETDOWN encountered an error." ) );
#endif
		break;
	case PF_Cmd_DO_DIALOG:
		err = DoDialog(in_data,out_data,params,output);
#ifdef DEBUG_ERRORS
		if ( err != PF_Err_NONE ) CFShow( CFSTR( "Ampeded error: PF_Cmd_DO_DIALOG encountered an error." ) );
#endif
		break;
	}
	return err;
}

#define OUT_SEQUENCE_DATA ((AmpedeSequenceData)**out_data->sequence_data)
#define IN_SEQUENCE_DATA ((AmpedeSequenceData)**in_data->sequence_data)


static PF_Err 
DoDialog (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
#pragma unused ( in_data, out_data, params, output )
	int mpErr = 0;
	CFMessagePortRef messagePort = IN_SEQUENCE_DATA.messagePort;
//	CFDataRef returnData = NULL;
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR( "PF_Cmd_DO_DIALOG called" ) );
#endif

	if ( ! messagePort ) return PF_Err_INTERNAL_STRUCT_DAMAGED;
	
	mpErr = 0;
	mpErr = CFMessagePortSendRequest(  messagePort,
										'INFO',
										NULL,
										1.0, // send timeout in seconds
										10, // receive timeout in seconds
										*kCFRunLoopDefaultMode, // run loop reply mode
										NULL );
										
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("returned from 'INFO' message send") );
#endif
	
	if ( mpErr )
	{
		switch ( mpErr ) {
		
		case kCFMessagePortSendTimeout: break;
		case kCFMessagePortReceiveTimeout: break;
		case kCFMessagePortIsInvalid:
			CFShow( CFSTR("Ampede error: kCFMessagePortIsInvalid occured on dialog command") );
			goto MessagePortFailure;

		case kCFMessagePortTransportError:
			CFShow( CFSTR("Ampede error: kCFMessagePortTransportError occured on dialog command") );
			goto MessagePortFailure;
		}
	}
	
//	out_data->out_flags |=	PF_OutFlag_FORCE_RERENDER;
	return PF_Err_NONE;
	
MessagePortFailure:
	return PF_Err_INTERNAL_STRUCT_DAMAGED;
}

static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
#pragma unused ( in_data, out_data, params, output )
	int mpErr = 0;
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR( "PF_Cmd_ABOUT called" ) );
#endif
	if ( ! ampedeMsgPort ) return PF_Err_INTERNAL_STRUCT_DAMAGED;
	
	mpErr = CFMessagePortSendRequest(  ampedeMsgPort,
										'INFO',
										NULL,
										1.0, // send timeout in seconds
										1.0, // receive timeout in seconds
										*kCFRunLoopDefaultMode, // run loop reply mode
										NULL );
	
	if ( mpErr ) return PF_Err_INTERNAL_STRUCT_DAMAGED;

	return PF_Err_NONE;
}

static PF_Err 
GlobalSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
#pragma unused ( in_data, params, output )
	PF_Err err = PF_Err_NONE;
	OSStatus osErr = noErr;
	static int didRunGlobalSetup = 0;
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR( "PF_Cmd_GLOBAL_SETUP called" ) );
#endif
	out_data->my_version = PF_VERSION(	MAJOR_VERSION, 
										MINOR_VERSION,
										BUG_VERSION, 
										STAGE_VERSION, 
										BUILD_VERSION);

	 
	out_data->out_flags |=	PF_OutFlag_I_DO_DIALOG |
							PF_OutFlag_NON_PARAM_VARY |
							PF_OutFlag_SEQUENCE_DATA_NEEDS_FLATTENING |
							PF_OutFlag_I_HAVE_EXTERNAL_DEPENDENCIES;

	// this is only executed once
	if ( didRunGlobalSetup ) return err;
	else
	{
		didRunGlobalSetup = 1;
		
		osErr = LaunchAmpede();
		
		if ( osErr ) return PF_Err_INTERNAL_STRUCT_DAMAGED;
		
		osErr = LoadCFMessagePortPointers();
		
		if ( osErr ) return PF_Err_INTERNAL_STRUCT_DAMAGED;
		
		ampedeMsgPort = NULL;
		ampedeMsgPort = CFMessagePortCreateRemote( kCFAllocatorDefault, CFSTR("AmpedeMessagePort") );
		
		if ( ampedeMsgPort )
		{
			CFMessagePortSetInvalidationCallBack( ampedeMsgPort, ampedeMessagePortInvalidationCallbackPtr );
		}
		else
		{
			CFShow( CFSTR("Ampede error: could not communicate with Ampede PDF UI\n") );
			return PF_Err_INTERNAL_STRUCT_DAMAGED; // FCP doesn't seem to care much
		}
		return err;
	}
}

static PF_Err 
ParamsSetup (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
#pragma unused ( output, params )

	PF_Err			err = PF_Err_NONE;
	PF_ParamDef		def;
	AEFX_CLR_STRUCT(def);

#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR( "PF_Cmd_PARAMS_SETUP called" ) );
#endif	
	PF_ADD_TOPIC(   "Basic Vector Motion",
					BASIC_VECTOR_MOTION_TOPIC_DISK_ID   );

	PF_ADD_FIXED(	"Vector Scale", 
					SCALE_MIN, 
					SCALE_BIG_MAX, 
					SCALE_MIN, 
					SCALE_MAX, 
					SCALE_DFLT,
					4, 
					0,
					0,
					SCALE_DISK_ID   );
					
	// Create the SCALE_MULITPLIER parameter...
	def.param_type = PF_Param_FIX_SLIDER;
	PF_STRCPY(def.name, "Scale Multiplier");
	def.flags = 0;
	def.u.fd.value_str[0] = def.u.fd.value_desc[0] = '\0';
	def.u.fd.value = def.u.fd.dephault = SCALE_MULTIPLIER_DFLT;
	def.u.fd.valid_min = def.u.fd.slider_min = SCALE_MULTIPLIER_MIN;
	def.u.fd.valid_max = SCALE_MULTIPLIER_BIG_MAX;
	def.u.fd.slider_max = SCALE_MULTIPLIER_MAX;
	def.u.fd.precision = 4;
	def.u.fd.display_flags = 0;
	err = PF_ADD_PARAM(in_data, -1, &def);
	if (err) return err;
					
	PF_ADD_ANGLE(   "Vector Rotation",
					ROTATION_DFLT,
					ROTATION_DISK_ID	);
					
	PF_ADD_POINT(   "Vector Center",
					CENTER_X_DFLT,
					CENTER_Y_DFLT,
					CENTER_RESTRICT_BOUNDS,
					CENTER_DISK_ID  );
	
	PF_ADD_POINT(   "Vector Anchor Point",
					ANCHOR_POINT_X_DFLT,
					ANCHOR_POINT_Y_DFLT,
					ANCHOR_POINT_RESTRICT_BOUNDS,
					ANCHOR_POINT_DISK_ID  );
	
	PF_ADD_CHECKBOX(	"Force Update",
						NULL,
						FORCE_UPDATE_DFLT,
						PF_ParamFlag_CANNOT_INTERP, // required
						FORCE_UPDATE_DISK_ID );
	
	PF_END_TOPIC( BASIC_VECTOR_MOTION_TOPIC_DISK_ID );

	out_data->num_params = AMPEDE_NUM_PARAMS;

	return err;
}


static PF_Err 
SequenceSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
#pragma unused ( output, params )
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR( "PF_Cmd_SEQUENCE_SETUP called" ) );
#endif
	out_data->sequence_data = NULL;
	out_data->sequence_data = PF_NEW_HANDLE(sizeof(AmpedeSequenceData));
	if (!out_data->sequence_data)  return PF_Err_OUT_OF_MEMORY;

	AEFX_CLR_STRUCT( OUT_SEQUENCE_DATA.frameInfo );
	OUT_SEQUENCE_DATA.messagePort = NULL;
	
	if ( ! ampedeMsgPort ) return PF_Err_INTERNAL_STRUCT_DAMAGED;
	
	if ( CFMessagePortIsValid( ampedeMsgPort ) )
	{
		CFDataRef uuidBytes = NULL;
		int mpErr = 0;

		do {
			mpErr = 0;
			mpErr = CFMessagePortSendRequest(   ampedeMsgPort,
												'SSET',
												NULL,
												0.5, // send timeout in seconds
												10, // receive timeout in seconds
												*kCFRunLoopDefaultMode, // run loop reply mode
												&uuidBytes );
			
			if ( mpErr )
			{
				switch ( mpErr ) {
				
				case kCFMessagePortSendTimeout: break;
				case kCFMessagePortReceiveTimeout: break;
				case kCFMessagePortIsInvalid:
					CFShow( CFSTR("Ampede error: kCFMessagePortIsInvalid occured on sequence setup command") );
					goto MessagePortFailure;

				case kCFMessagePortTransportError:
					CFShow( CFSTR("Ampede error: kCFMessagePortTransportError occured on sequence setup command") );
					goto MessagePortFailure;
				}
			}
			else
			{
				if ( CFDataGetLength( uuidBytes ) ) // true if we didn't get an empty CFData back
				{
					CFUUIDBytes uuidByteStruct;
					CFUUIDRef uuid;
					
					CFDataGetBytes( uuidBytes, CFRangeMake(0,16), (UInt8 *)&uuidByteStruct );
					uuid = CFUUIDCreateFromUUIDBytes( kCFAllocatorDefault, uuidByteStruct );
					CFRelease( uuidBytes );
					
					do {
						// busy wait for message port; shouldn't be long
						OUT_SEQUENCE_DATA.messagePort = CFMessagePortCreateRemote( kCFAllocatorDefault, CFUUIDCreateString( kCFAllocatorDefault, uuid ) );
					} while ( !OUT_SEQUENCE_DATA.messagePort );

					break; // out of the do-while loop
				}
				else CFRelease( uuidBytes );
			}
		} while ( ! PF_ABORT(in_data) ); // loop until we get our return data, or the user aborts
		
		if ( PF_ABORT(in_data) )
		{
			CFShow( CFSTR("User Aborted") );
			return PF_Interrupt_CANCEL;
		}
	}
	return PF_Err_NONE;

MessagePortFailure:
	return PF_Err_INTERNAL_STRUCT_DAMAGED;
}


static PF_Err 
SequenceFlatten (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
#pragma unused ( in_data, out_data, params, output )
	CFMessagePortRef messagePort = IN_SEQUENCE_DATA.messagePort;
#ifdef 	DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR( "PF_Cmd_SEQUENCE_FLATTEN called" ) );
#endif
	if ( ! messagePort ) return PF_Err_INTERNAL_STRUCT_DAMAGED;
	
	if ( CFMessagePortIsValid( messagePort ) )
	{
		CFDataRef flatData = NULL;
		int mpErr = 0;

		do {
			mpErr = 0;
			mpErr = CFMessagePortSendRequest(   messagePort,
												'FLAT',
												NULL,
												0.5, // send timeout in seconds
												10, // receive timeout in seconds
												*kCFRunLoopDefaultMode, // run loop reply mode
												&flatData );
			
			if ( mpErr )
			{
				switch ( mpErr ) {
				
				case kCFMessagePortSendTimeout: break;
				case kCFMessagePortReceiveTimeout: break;
				case kCFMessagePortIsInvalid:
					CFShow( CFSTR("Ampede error: kCFMessagePortIsInvalid occured on sequence flatten command") );
					goto MessagePortFailure;

				case kCFMessagePortTransportError:
					CFShow( CFSTR("Ampede error: kCFMessagePortTransportError occured on sequence flatten command") );
					goto MessagePortFailure;
				}
			}
			else
			{
				if ( CFDataGetLength( flatData ) ) // true if we didn't get an empty CFData back
				{
					int magic = 0xBABECAFE;
					int *magicSlot;
					UInt8 *handleMemPointer;

					out_data->sequence_data = NULL;
					out_data->sequence_data = PF_NEW_HANDLE( CFDataGetLength( flatData ) + 4 );
					if (!out_data->sequence_data)  return PF_Err_OUT_OF_MEMORY;
					
					magicSlot = ((int *)*out_data->sequence_data);
					*magicSlot = magic; // so that we know this is flattened data
					
					handleMemPointer = ((UInt8 *)*out_data->sequence_data);
					CFDataGetBytes( flatData, CFRangeMake( 0, CFDataGetLength( flatData ) ), handleMemPointer + 4 );
					
//					PF_DISPOSE_HANDLE( in_data->sequence_data );
					
					break; // out of the do-while loop
				}
				else CFRelease( flatData );
			}
		} while ( ! PF_ABORT(in_data) ); // loop until we get our return data, or the user aborts
	}
	return PF_Err_NONE;

MessagePortFailure:
	return PF_Err_INTERNAL_STRUCT_DAMAGED;
}

//#define DEBUG_SEQUENCE_RESETUP

static PF_Err 
SequenceResetup (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err err = PF_Err_NONE;
	int *magic = ((int *)*in_data->sequence_data);
	char **handle = in_data->sequence_data;
	const UInt8 *handleMemPointer = ((UInt8 *)*in_data->sequence_data);
	CFDataRef flatData = NULL;
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR( "PF_Cmd_SEQUENCE_RESETUP called" ) );
#endif

	if ( handleMemPointer == NULL )
	{
		CFShow( CFSTR( "Ampede warning: PF_Cmd_SEQUENCE_RESETUP called without any sequenced data." ) );
		return SequenceSetup( in_data, out_data, params, output );
	}
	
#ifdef DEBUG_SEQUENCE_RESETUP
	CFShow( CFSTR( "1" ) );
#endif
	if ( *magic == 0xBABECAFE )
	{
		// we need to unflatten the data

		CFMessagePortRef messagePort;
		CFDataRef flatData;
		int flatDataLength = PF_GET_HANDLE_SIZE( handle );
		
#ifdef DEBUG_SEQUENCE_RESETUP
	CFShow( CFSTR( "2" ) );
#endif
		// this loads a fresh document in Ampede with the default initialization
		// it also sets up our message port
		err = SequenceSetup( in_data, out_data, params, output );
		if ( err ) 
		{
			PF_DISPOSE_HANDLE( handle ); // free our flattened data, otherwise we'll leak
			return err;
		}
		
#ifdef DEBUG_SEQUENCE_RESETUP
	CFShow( CFSTR( "3" ) );
#endif
//		in_data->sequence_data = out_data->sequence_data;
//		out_data->sequence_data = NULL;
		
		messagePort = OUT_SEQUENCE_DATA.messagePort;
		
#ifdef DEBUG_SEQUENCE_RESETUP
	CFShow( CFSTR( "4" ) );
#endif
		flatData = CFDataCreateWithBytesNoCopy( NULL, handleMemPointer + 4, flatDataLength - 4, kCFAllocatorNull );
		
		if ( ! messagePort )
		{
			CFRelease( flatData );
			PF_DISPOSE_HANDLE( handle );
			return PF_Err_INTERNAL_STRUCT_DAMAGED;
		}
		
#ifdef DEBUG_SEQUENCE_RESETUP
	CFShow( CFSTR( "5" ) );
#endif
		if ( CFMessagePortIsValid( messagePort ) )
		{
			int mpErr = 0;

			do {
				mpErr = 0;
				mpErr = CFMessagePortSendRequest(   messagePort,
													'LOAD',
													flatData,
													0.5, // send timeout in seconds
													10, // receive timeout in seconds
													*kCFRunLoopDefaultMode, // run loop reply mode
													NULL );
				
#ifdef DEBUG_SEQUENCE_RESETUP
	CFShow( CFSTR( "6" ) );
#endif
				if ( mpErr )
				{
					switch ( mpErr ) {
					
					case kCFMessagePortSendTimeout: break;
					case kCFMessagePortReceiveTimeout: break;
					case kCFMessagePortIsInvalid:
						CFShow( CFSTR("Ampede error: kCFMessagePortIsInvalid occured on sequence load command") );
						goto MessagePortFailure;

					case kCFMessagePortTransportError:
						CFShow( CFSTR("Ampede error: kCFMessagePortTransportError occured on sequence load command") );
						goto MessagePortFailure;
					}
				}
				else break; // we successfully sent the 'LOAD' command
			} while ( ! PF_ABORT(in_data) ); // loop until we get our return data, or the user aborts
		}
		
#ifdef DEBUG_SEQUENCE_RESETUP
	CFShow( CFSTR( "7" ) );
#endif
		CFRelease( flatData );
		PF_DISPOSE_HANDLE( handle ); // free our flattened data
	}
	// else in_data->sequence_data is already a handle to unflattened data
	// I don't think we need to create a new document for this one; it should be valid for the document it was already
	// connected to
		
#ifdef DEBUG_SEQUENCE_RESETUP
	CFShow( CFSTR( "8" ) );
#endif
	return PF_Err_NONE;
	
MessagePortFailure:
#ifdef DEBUG_SEQUENCE_RESETUP
	CFShow( CFSTR( "9" ) );
#endif
	CFRelease( flatData );
	PF_DISPOSE_HANDLE( handle ); // free our flattened data
	return PF_Err_INTERNAL_STRUCT_DAMAGED;
}


static PF_Err 
SequenceSetdown (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
#pragma unused ( output, params )
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR( "PF_Cmd_SEQUENCE_SETDOWN called" ) );
#endif
	if (in_data->sequence_data)
	{
		PF_DISPOSE_HANDLE(in_data->sequence_data);
		out_data->sequence_data = NULL;
	}
	return PF_Err_NONE;
}


static PF_Err
AmpedeGetData (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	CFDataRef		*data )
{
#pragma unused ( out_data, params )
	PF_Err err = PF_Err_NONE;
	CFMessagePortRef messagePort = IN_SEQUENCE_DATA.messagePort;
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR( "AmpedeGetData() called" ) );
#endif	
	// send the 'DATA' command to Ampede
	if ( ! messagePort ) return PF_Err_INTERNAL_STRUCT_DAMAGED;

	if ( CFMessagePortIsValid( messagePort ) )
	{
		MessagePortError mpErr = 0;
		
		CFMutableDataRef sendData = CFDataCreateMutable( kCFAllocatorDefault, 12 );
		CFDataRef returnData = NULL;

		// setup dimensions
		int ints[2] = { 0, 0 };
		float floats[1] = { 0 };
		ints[0] = output->width;
		ints[1] = output->height;
		floats[0] = (float) ( (float)in_data->pixel_aspect_ratio.num / (float)in_data->pixel_aspect_ratio.den );

		if ( sendData )
		{
			CFDataAppendBytes( sendData, (unsigned char const *)ints, 8 );
			CFDataAppendBytes( sendData, (unsigned char const *)floats, 4 );
		}
		else return PF_Err_OUT_OF_MEMORY;
		
		do {
			mpErr = 0;
			mpErr = CFMessagePortSendRequest(  messagePort,
												'DATA',
												sendData,
												.5, // send timeout in seconds
												.5, // receive timeout in seconds
												*kCFRunLoopDefaultMode, // run loop reply mode
												&returnData );
			
			if ( mpErr )
			{
				switch ( mpErr ) {
				
				case kCFMessagePortSendTimeout: break;
				case kCFMessagePortReceiveTimeout: break;
				case kCFMessagePortIsInvalid:
					CFShow( CFSTR("Ampede error: kCFMessagePortIsInvalid occured on data command") );
					CFRelease( sendData );
					goto MessagePortFailure;

				case kCFMessagePortTransportError:
					CFShow( CFSTR("Ampede error: kCFMessagePortTransportError occured on data command") );
					CFRelease( sendData );
					goto MessagePortFailure;
				}
			}
			else
			{
				if ( CFDataGetLength( returnData ) ) // true if we didn't get an empty CFData back
				{
//					CFShow( CFCopyDescription( returnData ) );
					*data = returnData;
					break; // out of the do-while loop
				}
				else CFRelease( returnData );
			}
			err = PF_ABORT(in_data);
		} while ( ! err ); // loop until we get our return data, or the user aborts
		
		CFRelease( sendData );
		
		if ( err )
		{
#ifdef DEBUG_ABORTS
			CFShow( CFSTR("User Aborted in AmpedeGetData()") );
#endif
//			if (*data) CFRelease( returnData );
//			*data = NULL;
			return err;
		}
	}
	return PF_Err_NONE;
	
MessagePortFailure:
	*data = NULL;
	return PF_Err_INTERNAL_STRUCT_DAMAGED;
}


static PF_Err 
BitmapContextSetup (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	CFDataRef		ampedeData,
	int				*didSetup,
	int				forceSetup)
{
#pragma unused ( data, params, out_data )
	PF_Err err = PF_Err_NONE;
	int setupBitmapContext = 0;
	AmpedeDATA *data;
#ifdef DEBUG_CALLING_SEQUENCE	
	CFShow( CFSTR( "BitmapContextSetup() called" ) );
#endif	
	// see AmpedeDocument.h for information on the structure of the returned data
	data = (AmpedeDATA *)CFDataGetBytePtr( ampedeData );
	*didSetup = 0;
	
	// first check width, height, and colorspace; changes here invalidate everything
	if (IN_SEQUENCE_DATA.frameInfo.width != output->width)
	{
		IN_SEQUENCE_DATA.frameInfo.width = output->width;
		setupBitmapContext = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.width != output->width. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &IN_SEQUENCE_DATA.frameInfo.width );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	if (IN_SEQUENCE_DATA.frameInfo.height != output->height)
	{
		IN_SEQUENCE_DATA.frameInfo.height = output->height;
		setupBitmapContext = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.height != output->height. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &IN_SEQUENCE_DATA.frameInfo.height );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	if (IN_SEQUENCE_DATA.frameInfo.shouldColorManage != data->shouldColorManage)
	{
		IN_SEQUENCE_DATA.frameInfo.shouldColorManage = data->shouldColorManage;
		setupBitmapContext = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.shouldColorManage != data->shouldColorManage. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &IN_SEQUENCE_DATA.frameInfo.shouldColorManage );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	if (IN_SEQUENCE_DATA.frameInfo.doSubpixelRendering != data->doSubpixelRendering)
	{
		IN_SEQUENCE_DATA.frameInfo.doSubpixelRendering = data->doSubpixelRendering;
		setupBitmapContext = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.doSubpixelRendering != data->doSubpixelRendering. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &IN_SEQUENCE_DATA.frameInfo.doSubpixelRendering );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	if (IN_SEQUENCE_DATA.frameInfo.superSampling != data->superSampling)
	{
		IN_SEQUENCE_DATA.frameInfo.superSampling = data->superSampling;
		setupBitmapContext = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.superSampling != data->superSampling. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &IN_SEQUENCE_DATA.frameInfo.superSampling );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	
	// now setup the bitmap context if we need to
	if ( setupBitmapContext || forceSetup )
	{
		CGColorSpaceRef colorSpace;
		vImage_Buffer buffer = { NULL, 0, 0, 0 };
		vImage_Error vErr = kvImageNoError;
		
#ifdef DEBUG_CALLING_SEQUENCE
		CFShow( CFSTR("Entered BitmapContextSetup() (setupBitmapContext || forceSetup)") );
#endif
		// get color space for context
		if ( IN_SEQUENCE_DATA.frameInfo.shouldColorManage )
		{
			CMProfileRef aProfile;

			CMGetDefaultProfileBySpace(cmRGBData, &aProfile);
			if ( aProfile == NULL ) return PF_Err_OUT_OF_MEMORY;
			
			colorSpace = CGColorSpaceCreateWithPlatformColorSpace( aProfile );

			CMCloseProfile( aProfile );
		}
		else colorSpace = CGColorSpaceCreateDeviceRGB_p();
		if ( colorSpace == NULL ) return PF_Err_OUT_OF_MEMORY;
		
		// allocate context
		if ( IN_SEQUENCE_DATA.frameInfo.bitmapContext ) CGContextRelease_p( IN_SEQUENCE_DATA.frameInfo.bitmapContext );
		
		if ( IN_SEQUENCE_DATA.frameInfo.doSubpixelRendering )
		{
			CFShow( CFSTR("Building subpixel rendering bitmap context.") );
			vErr = InitvImage_Buffer(	&buffer,
										IN_SEQUENCE_DATA.frameInfo.height * IN_SEQUENCE_DATA.frameInfo.superSampling,
										IN_SEQUENCE_DATA.frameInfo.width * IN_SEQUENCE_DATA.frameInfo.superSampling,
										4 );
		}
		else
		{
			vErr = InitvImage_Buffer( &buffer, IN_SEQUENCE_DATA.frameInfo.height, IN_SEQUENCE_DATA.frameInfo.width, 4 );
		}
		
		if ( vErr == kvImageNoError )
		{
			IN_SEQUENCE_DATA.frameInfo.bitmapContext = CGBitmapContextCreate(   buffer.data, 
																				buffer.width,
																				buffer.height,
																				8,      // bits per component
																				buffer.rowBytes,
																				colorSpace,
																				kCGImageAlphaPremultipliedFirst );
			if ( IN_SEQUENCE_DATA.frameInfo.bitmapContext == NULL ) return PF_Err_OUT_OF_MEMORY;
		}
		else return PF_Err_OUT_OF_MEMORY;
		
		CFRelease( colorSpace );
		
		// report success
		*didSetup = 1;
	}
#ifdef DEBUG_VALUES	
	CFShow( CFCopyDescription(IN_SEQUENCE_DATA.frameInfo.bitmapContext) );
#endif	
	return PF_Err_NONE;
}


static PF_Err 
BufferSetup (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	CFDataRef		ampedeData,
	int				*didSetup,
	int				forceSetup )
{
#pragma unused ( data, params, out_data )
	PF_Err err = PF_Err_NONE;
	int setupBufferContext = 0;

	// see AmpedeDocument.h for information on the structure of the returned data
	AmpedeDATA *data = (AmpedeDATA *)CFDataGetBytePtr( ampedeData );
#ifdef DEBUG_CALLING_SEQUENCE	
	CFShow( CFSTR( "BufferSetup() called" ) );
#endif	
	*didSetup = 0;
	
	if (IN_SEQUENCE_DATA.frameInfo.drawStyle != data->drawStyle)
	{
		IN_SEQUENCE_DATA.frameInfo.drawStyle = data->drawStyle;
		setupBufferContext = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.drawStyle != data->drawStyle. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &IN_SEQUENCE_DATA.frameInfo.drawStyle );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	// now set up the buffers if we need to
	if ( setupBufferContext || forceSetup )
	{
#ifdef DEBUG_CALLING_SEQUENCE
		CFShow( CFSTR("Entered BufferSetup() (setupBufferContext || forceSetup)") );
#endif
		// free our tmpBuf, if there is one
		if ( IN_SEQUENCE_DATA.frameInfo.tmpBuf )
		{
			free( IN_SEQUENCE_DATA.frameInfo.tmpBuf );
			IN_SEQUENCE_DATA.frameInfo.tmpBuf = NULL;
		}
		
		if ( IN_SEQUENCE_DATA.frameInfo.doSubpixelRendering )
		{
			size_t tmpBufSize;
			vImage_Buffer sourceBuf;
			vImage_Buffer destBuf;
			CGContextRef bitmapContext = IN_SEQUENCE_DATA.frameInfo.bitmapContext;
			
			sourceBuf.width = CGBitmapContextGetWidth( bitmapContext );
			sourceBuf.height = CGBitmapContextGetHeight( bitmapContext );
			sourceBuf.rowBytes = CGBitmapContextGetBytesPerRow_p( bitmapContext );
			sourceBuf.data = CGBitmapContextGetData( bitmapContext );
			
			destBuf.width = output->width;
			destBuf.height = output->height;
			destBuf.rowBytes = output->rowbytes;
			destBuf.data = output->data;
			
			switch ( IN_SEQUENCE_DATA.frameInfo.drawStyle ) {
			
			case kDrawWithLanczos3Resize:
				tmpBufSize = vImageGetMinimumGeometryTempBufferSize_p( &sourceBuf, &destBuf, 4, ( kvImageEdgeExtend ) );
				break;
			
			case kDrawWithLanczos5Resize:
				tmpBufSize = vImageGetMinimumGeometryTempBufferSize_p( &sourceBuf, &destBuf, 4, ( kvImageEdgeExtend | kvImageHighQualityResampling ) );
				break;
			}

			IN_SEQUENCE_DATA.frameInfo.tmpBuf = malloc( tmpBufSize );
			if ( !IN_SEQUENCE_DATA.frameInfo.tmpBuf ) return PF_Err_OUT_OF_MEMORY;
		}

		// report success
		*didSetup = 1;
	}

	return PF_Err_NONE;
}


static PF_Err 
FileSetup (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	CFDataRef		data,
	int				*didSetup,
	int				forceSetup )
{
#pragma unused ( data, params, in_data, out_data, output )
	PF_Err err = PF_Err_NONE;
	int setupFile = 0;

	// see SequenceDocument.h for information on the structure of the returned data
	AmpedeDRAW *drawData = (AmpedeDRAW *)CFDataGetBytePtr( data );
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR( "FileSetup() called" ) );
#endif	
	*didSetup = 0;
	
	// first check if the file has changed
	if (IN_SEQUENCE_DATA.frameInfo.fileGeneration < drawData->fileGeneration)
	{
		IN_SEQUENCE_DATA.frameInfo.fileGeneration = drawData->fileGeneration;
		setupFile = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.fileGeneration < drawData->fileGeneration. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &IN_SEQUENCE_DATA.frameInfo.fileGeneration );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	
	// now read the new file if we need to
	if ( setupFile || forceSetup )
	{
		CFStringRef filePath = NULL;
		CFURLRef url = NULL;
		
		CFDataRef returnData = NULL;
		CFMessagePortRef messagePort = IN_SEQUENCE_DATA.messagePort;
#ifdef DEBUG_CALLING_SEQUENCE		
		CFShow( CFSTR("Entered FileSetup() (setupFile || forceSetup)") );
#endif
		// if we need a new file, we don't want to leave the old one around if we fail midway through
		if ( IN_SEQUENCE_DATA.frameInfo.pdf )
		{
			CGPDFDocumentRelease_p( IN_SEQUENCE_DATA.frameInfo.pdf );
			IN_SEQUENCE_DATA.frameInfo.pdf = NULL;
		}
		
		// send the 'FILE' command to Ampede
		if ( ! messagePort ) return PF_Err_INTERNAL_STRUCT_DAMAGED;

		if ( CFMessagePortIsValid( messagePort ) )
		{
			MessagePortError mpErr = 0;
			
			do {
				mpErr = 0;
				mpErr = CFMessagePortSendRequest(  messagePort,
													'FILE',
													NULL,
													.5, // send timeout in seconds
													.5, // receive timeout in seconds
													*kCFRunLoopDefaultMode, // run loop reply mode
													&returnData );
				
				if ( mpErr )
				{
					switch ( mpErr ) {
					
					case kCFMessagePortSendTimeout: break;
					case kCFMessagePortReceiveTimeout: break;
					case kCFMessagePortIsInvalid:
						CFShow( CFSTR("Ampede error: kCFMessagePortIsInvalid occured on data command") );
						goto MessagePortFailure;

					case kCFMessagePortTransportError:
						CFShow( CFSTR("Ampede error: kCFMessagePortTransportError occured on data command") );
						goto MessagePortFailure;
					}
				}
				else
				{
					if ( CFDataGetLength( returnData ) ) // true if we didn't get an empty CFData back
					{
						break; // out of the do-while loop
					}
					else CFRelease( returnData );
				}
				err = PF_ABORT(in_data);
			} while ( ! err ); // loop until we get our return data, or the user aborts
		
			
			if ( err )
			{
#ifdef DEBUG_ABORTS
				CFShow( CFSTR("User Aborted in FileSetup()") );
#endif
				return PF_Interrupt_CANCEL;
			}
		}

		//// we got the return data successfully
		
		filePath = CFStringCreateFromExternalRepresentation( NULL, returnData, kCFStringEncodingUTF8 );
		CFRelease( returnData ); if ( !filePath ) return PF_Err_OUT_OF_MEMORY;
#ifdef DEBUG_VALUES		
		CFShow(  filePath );
#endif		
		url = CFURLCreateWithFileSystemPath( NULL, filePath, kCFURLPOSIXPathStyle, 0 );
		CFRelease( filePath ); if ( !url ) return PF_Err_OUT_OF_MEMORY;
#ifdef DEBUG_VALUES
		CFShow(  url );
#endif		
		IN_SEQUENCE_DATA.frameInfo.pdf = CGPDFDocumentCreateWithURL_p( url );
		CFRelease( url ); if ( !IN_SEQUENCE_DATA.frameInfo.pdf ) return PF_Err_OUT_OF_MEMORY;
		
		// report success
		*didSetup = 1;
	}
#ifdef DEBUG_VALUES	
	CFShow( IN_SEQUENCE_DATA.frameInfo.pdf );
#endif
	return PF_Err_NONE;
	
MessagePortFailure:
	return PF_Err_INTERNAL_STRUCT_DAMAGED;
}


static PF_Err 
PageSetup (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	CFDataRef		data,
	int				*didSetup,
	int				forceSetup)
{
#pragma unused ( data, params, in_data, out_data, output )
	PF_Err err = PF_Err_NONE;
	int setupPage = 0;
	
	// see SequenceDocument.h for information on the structure of the returned data
	AmpedeDRAW *drawData = (AmpedeDRAW *)CFDataGetBytePtr( data );
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR( "PageSetup() called" ) );
#endif	
	*didSetup = 0;
	
	// test page-only parameters here
	if (IN_SEQUENCE_DATA.frameInfo.currentPage != drawData->currentPage) {
		IN_SEQUENCE_DATA.frameInfo.currentPage = drawData->currentPage;
		setupPage = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.currentPage != data->currentPage. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &IN_SEQUENCE_DATA.frameInfo.currentPage );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	if (IN_SEQUENCE_DATA.frameInfo.boxType != drawData->boxType) {
		IN_SEQUENCE_DATA.frameInfo.boxType = drawData->boxType;
		setupPage = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.boxType != drawData->boxType. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.boxType );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	

	// now setup the page if we need to (don't test IN_SEQUENCE_DATA.frameInfo.pdfPage; it could point to garbage)
	if ( setupPage || forceSetup )
	{
		// this function should only be called when there is a valid pdf document available
		if ( !IN_SEQUENCE_DATA.frameInfo.pdf ) return PF_Err_INTERNAL_STRUCT_DAMAGED;
		
		IN_SEQUENCE_DATA.frameInfo.pdfPage = NULL; // don't need to memory manage pdfPage
		IN_SEQUENCE_DATA.frameInfo.pdfPage = CGPDFDocumentGetPage( IN_SEQUENCE_DATA.frameInfo.pdf, IN_SEQUENCE_DATA.frameInfo.currentPage );
		if ( !IN_SEQUENCE_DATA.frameInfo.pdfPage )
		{
			// fallback in case currentPage is out-of-range, rather than simply fail
			CFShow( CFSTR( "Ampede error: selected page did not exist; using page 1 instead." ) );
			IN_SEQUENCE_DATA.frameInfo.pdfPage = CGPDFDocumentGetPage( IN_SEQUENCE_DATA.frameInfo.pdf, 1 );
			if ( !IN_SEQUENCE_DATA.frameInfo.pdfPage )
			{
				CGPDFDocumentRelease_p( IN_SEQUENCE_DATA.frameInfo.pdf );
				IN_SEQUENCE_DATA.frameInfo.pdf = NULL;
				IN_SEQUENCE_DATA.frameInfo.fileGeneration = 0; // this forces us to re-fetch the PDF file, and thus the page, next time around
				return PF_Err_OUT_OF_MEMORY;
			}
		}
		
		// report success
		*didSetup = 1;
	}
#ifdef DEBUG_VALUES
	CFShow( IN_SEQUENCE_DATA.frameInfo.pdfPage );
#endif
	return PF_Err_NONE;
}


static PF_Err 
PDFSetup (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	AmpedeDATA		*data,
	int				*didSetup,
	int				forceSetup)
{
#pragma unused ( data, params, out_data )
	PF_Err err = PF_Err_NONE;
	CFDataRef returnData = NULL;
	int setupPDF = 0;
	int setupFile = 0;
	int setupPage = 0;
	int invalidateFrameCache = 0;

	// see SequenceDocument.h for information on the structure of the returned data
	AmpedeDRAW *drawData;
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("PDFSetup() called") );
#endif	
	*didSetup = 0;
	
	if ( !data->pdfDataFresh ) 
	{
#ifdef DEBUG_VALUES
		CFShow( CFSTR("!data->pdfDataFresh") );
#endif
		setupPDF = 1;
	}

	// now set up the pdf if we need to
	if ( setupPDF || forceSetup )
	{
		CFMessagePortRef messagePort = IN_SEQUENCE_DATA.messagePort;

		// send the 'DRAW' command to Ampede
		if ( ! messagePort ) return PF_Err_INTERNAL_STRUCT_DAMAGED;

		if ( CFMessagePortIsValid( messagePort ) )
		{
			MessagePortError mpErr = 0;

			do {
				mpErr = 0;
				mpErr = CFMessagePortSendRequest(   messagePort,
													'DRAW',
													NULL,
													.5, // send timeout in seconds
													.5, // receive timeout in seconds
													*kCFRunLoopDefaultMode, // run loop reply mode
													&returnData );
				
				if ( mpErr )
				{
					switch ( mpErr ) {
					
					case kCFMessagePortSendTimeout: break;
					case kCFMessagePortReceiveTimeout: break;
					case kCFMessagePortIsInvalid:
						CFShow( CFSTR("Ampede error: kCFMessagePortIsInvalid occured on draw command") );
						goto MessagePortFailure;

					case kCFMessagePortTransportError:
						CFShow( CFSTR("Ampede error: kCFMessagePortTransportError occured on draw command") );
						goto MessagePortFailure;
					}
				}
				else
				{
					if ( CFDataGetLength( returnData ) ) // true if we didn't get an empty CFData back
					{
						break; // out of the do-while loop
					}
					else CFRelease( returnData );
				}
				err = PF_ABORT(in_data);
			} while ( ! err ); // loop until we get our return data, or the user aborts
			
			if ( err ) // return if there was a user abort, cleaning up data
			{
#ifdef DEBUG_ABORTS
				CFShow( CFSTR("User Aborted in PDFSetup()") );
#endif
				return PF_Interrupt_CANCEL;
			}
		}
		
		// the drawing data was returned by Ampede, without an intervening abort
		
		err = FileSetup( in_data, out_data, params, output, returnData, &setupFile, setupPDF );
		if ( err != PF_Err_NONE )
		{
			CFRelease( returnData );
			return err;
		}
		
		err = PageSetup( in_data, out_data, params, output, returnData, &setupPage, setupFile );
		if ( err != PF_Err_NONE )
		{
			CFRelease( returnData );
			return err;
		}
		
		drawData = (AmpedeDRAW *)CFDataGetBytePtr( returnData );
		
		// update drawing variables that could potentially invalidate the frame cache
		if (IN_SEQUENCE_DATA.frameInfo.pageRotation != drawData->pageRotation)
		{
			IN_SEQUENCE_DATA.frameInfo.pageRotation = drawData->pageRotation;
			invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
			CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.pageRotation != drawData->pageRotation. It's now set to:") );
			if (1) {
			CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &IN_SEQUENCE_DATA.frameInfo.pageRotation );
			CFShow( CFCopyDescription(n) );
			CFRelease(n);
			}
#endif
		}
		if (IN_SEQUENCE_DATA.frameInfo.boxType != drawData->boxType)
		{
			IN_SEQUENCE_DATA.frameInfo.boxType = drawData->boxType;
			invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
			CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.boxType != drawData->boxType. It's now set to:") );
			if (1) {
			CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &IN_SEQUENCE_DATA.frameInfo.boxType );
			CFShow( CFCopyDescription(n) );
			CFRelease(n);
			}
#endif
		}
		if (IN_SEQUENCE_DATA.frameInfo.shouldDrawPageBackground != drawData->shouldDrawPageBackground)
		{
			IN_SEQUENCE_DATA.frameInfo.shouldDrawPageBackground = drawData->shouldDrawPageBackground;
			invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
			CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.shouldDrawPageBackground != drawData->shouldDrawPageBackground. It's now set to:") );
			if (1) {
			CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &IN_SEQUENCE_DATA.frameInfo.shouldDrawPageBackground );
			CFShow( CFCopyDescription(n) );
			CFRelease(n);
			}
#endif
		}
		if (IN_SEQUENCE_DATA.frameInfo.shouldPreserveAspectRatio != drawData->shouldPreserveAspectRatio)
		{
			IN_SEQUENCE_DATA.frameInfo.shouldPreserveAspectRatio = drawData->shouldPreserveAspectRatio;
			invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
			CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.shouldPreserveAspectRatio != drawData->shouldPreserveAspectRatio. It's now set to:") );
			if (1) {
			CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &IN_SEQUENCE_DATA.frameInfo.shouldPreserveAspectRatio );
			CFShow( CFCopyDescription(n) );
			CFRelease(n);
			}
#endif
		}
		if (IN_SEQUENCE_DATA.frameInfo.red != drawData->red)
		{
			IN_SEQUENCE_DATA.frameInfo.red = drawData->red;
			invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
			CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.red != drawData->red. It's now set to:") );
			if (1) {
			CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.red );
			CFShow( CFCopyDescription(n) );
			CFRelease(n);
			}
#endif
		}
		if (IN_SEQUENCE_DATA.frameInfo.green != drawData->green)
		{
			IN_SEQUENCE_DATA.frameInfo.green = drawData->green;
			invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
			CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.green != drawData->green. It's now set to:") );
			if (1) {
			CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.green );
			CFShow( CFCopyDescription(n) );
			CFRelease(n);
			}
#endif
		}
		if (IN_SEQUENCE_DATA.frameInfo.blue != drawData->blue)
		{
			IN_SEQUENCE_DATA.frameInfo.blue = drawData->blue;
			invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
			CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.blue != drawData->blue. It's now set to:") );
			if (1) {
			CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.blue );
			CFShow( CFCopyDescription(n) );
			CFRelease(n);
			}
#endif
		}
		if (IN_SEQUENCE_DATA.frameInfo.alpha != drawData->alpha)
		{
			IN_SEQUENCE_DATA.frameInfo.alpha = drawData->alpha;
			invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
			CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.alpha != drawData->alpha. It's now set to:") );
			if (1) {
			CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.alpha );
			CFShow( CFCopyDescription(n) );
			CFRelease(n);
			}
#endif
		}

		if ( invalidateFrameCache ) // don't need to check for "force"; if it's on, FrameSetup will invalidate for us
		{
#ifdef DEBUG_CALLING_SEQUENCE
			CFShow( CFSTR("Entered PDFSetup() (invalidateFrameCache)") );
#endif
			if ( IN_SEQUENCE_DATA.frameInfo.frameCache ) CGContextRelease_p( IN_SEQUENCE_DATA.frameInfo.frameCache );
			IN_SEQUENCE_DATA.frameInfo.frameCache = NULL;
		}

		CFRelease( returnData );
		
		// report success
		*didSetup = 1;
	}
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR("PDFSetup() finished successfully") );
#endif
	return PF_Err_NONE;
	
MessagePortFailure:
	return PF_Err_INTERNAL_STRUCT_DAMAGED;
}


static PF_Err 
FrameSetup (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
#pragma unused ( out_data )
	PF_Err err = PF_Err_NONE;
	
	CFDataRef returnData = NULL;
	AmpedeDATA *data;
	float tmpAspectRatio = 0.0;
	int forceRerender;
	
	int setupBitmapContext = 0;
	int setupBuffers = 0;
	int setupPDF = 0;
	int invalidateFrameCache = 0;
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR( "PF_Cmd_FRAME_SETUP called" ) );
#endif	
	// send the 'DATA' command to Ampede
	err = AmpedeGetData( in_data, out_data, params, output, &returnData );
	if ( err != PF_Err_NONE ) return err;
		// err could be a user-abort, in which case we don't want to continue.
		
	// see AmpedeDocument.h for information on the structure of the returned data
	data = (AmpedeDATA *)CFDataGetBytePtr( returnData );

	forceRerender = data->forceRerender;
#ifdef DEBUG_FORCE_RENDER
	if ( forceRerender ) CFShow( CFSTR("forced rerender") );
#endif

	// setup bitmap context (Note: this is lazy; it only re-sets up if it has to.)
	// if bitmapContext is NULL, we force a context to be created
	err = BitmapContextSetup( in_data, out_data, params, output, returnData, &setupBitmapContext, !(IN_SEQUENCE_DATA.frameInfo.bitmapContext) );
	if ( err != PF_Err_NONE ) return err;
	
	err = BufferSetup( in_data, out_data, params, output, returnData, &setupBuffers, setupBitmapContext );
	if ( err != PF_Err_NONE ) return err;
	
	err = PDFSetup( in_data, out_data, params, output, (AmpedeDATA *)CFDataGetBytePtr(returnData), &setupPDF, setupBuffers );
	if ( err != PF_Err_NONE ) return err;
	
	//// now check our Ampede and Final Cut Pro data for changes; changes here only invalidate our frame cache

	// Frame parameters from Ampede
	if (IN_SEQUENCE_DATA.frameInfo.antialias != data->antialias) {
		IN_SEQUENCE_DATA.frameInfo.antialias = data->antialias;
		invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.antialias != data->antialias. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.antialias );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	if (IN_SEQUENCE_DATA.frameInfo.shouldSmoothFonts != data->shouldSmoothFonts) {
		IN_SEQUENCE_DATA.frameInfo.shouldSmoothFonts = data->shouldSmoothFonts;
		invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.shouldSmoothFonts != data->shouldSmoothFonts. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.shouldSmoothFonts );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	if (IN_SEQUENCE_DATA.frameInfo.intent != data->intent) {
		IN_SEQUENCE_DATA.frameInfo.intent = data->intent;
		invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.intent != data->intent. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.intent );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	if (IN_SEQUENCE_DATA.frameInfo.flatness != data->flatness) {
		IN_SEQUENCE_DATA.frameInfo.flatness = data->flatness;
		invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.flatness != data->flatness. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.flatness );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	if (IN_SEQUENCE_DATA.frameInfo.quality != data->quality) {
		IN_SEQUENCE_DATA.frameInfo.quality = data->quality;
		invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.quality != data->quality. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.quality );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	
	// Frame parameters from Final Cut Pro
	if (IN_SEQUENCE_DATA.frameInfo.scale != ((float)(params[AMPEDE_SCALE]->u.fd.value / (float)(1L << 16) ) / 100)) {
		IN_SEQUENCE_DATA.frameInfo.scale = (float)((params[AMPEDE_SCALE]->u.fd.value / (float)(1L << 16)) / 100);
		invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.scale != params[AMPEDE_SCALE]. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.scale );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	if (IN_SEQUENCE_DATA.frameInfo.scaleMultiplier != ((float)(params[AMPEDE_SCALE_MULTIPLIER]->u.fd.value / (float)(1L << 16)))) {
		IN_SEQUENCE_DATA.frameInfo.scaleMultiplier = (float)(params[AMPEDE_SCALE_MULTIPLIER]->u.fd.value / (float)(1L << 16));
		invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.scaleMultiplier != params[AMPEDE_SCALE_MULTIPLIER]. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.scaleMultiplier );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	if (IN_SEQUENCE_DATA.frameInfo.rotation != ((float)(params[AMPEDE_ROTATION]->u.fd.value / (float)(1L << 16)))) {
		IN_SEQUENCE_DATA.frameInfo.rotation = (float)(params[AMPEDE_ROTATION]->u.fd.value / (float)(1L << 16));
		invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.rotation != params[AMPEDE_ROTATION]. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.rotation );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	if (IN_SEQUENCE_DATA.frameInfo.centerX != ((float)(params[AMPEDE_CENTER]->u.td.x_value / (float)(1L << 16)))) {
		IN_SEQUENCE_DATA.frameInfo.centerX = (float)(params[AMPEDE_CENTER]->u.td.x_value / (float)(1L << 16));
		invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.centerX != params[AMPEDE_CENTER]. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.centerX );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	if (IN_SEQUENCE_DATA.frameInfo.centerY != ((float)(params[AMPEDE_CENTER]->u.td.y_value / (float)(1L << 16)))) {
		IN_SEQUENCE_DATA.frameInfo.centerY = (float)(params[AMPEDE_CENTER]->u.td.y_value / (float)(1L << 16));
		invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.centerY != params[AMPEDE_CENTER]. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.centerY );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	if (IN_SEQUENCE_DATA.frameInfo.anchorX != ((float)(params[AMPEDE_ANCHOR_POINT]->u.td.x_value / (float)(1L << 16)))) {
		IN_SEQUENCE_DATA.frameInfo.anchorX = (float)(params[AMPEDE_ANCHOR_POINT]->u.td.x_value / (float)(1L << 16));
		invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.anchorX != params[AMPEDE_ANCHOR_POINT]. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.anchorX );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	if (IN_SEQUENCE_DATA.frameInfo.anchorY != ((float)(params[AMPEDE_ANCHOR_POINT]->u.td.y_value / (float)(1L << 16)))) {
		IN_SEQUENCE_DATA.frameInfo.anchorY = (float)(params[AMPEDE_ANCHOR_POINT]->u.td.y_value / (float)(1L << 16));
		invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.anchorY != params[AMPEDE_ANCHOR_POINT]. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.anchorY );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	
	tmpAspectRatio = (float)((float)in_data->pixel_aspect_ratio.num / (float)in_data->pixel_aspect_ratio.den);
	if (IN_SEQUENCE_DATA.frameInfo.pixelAspectRatio != tmpAspectRatio) {
		IN_SEQUENCE_DATA.frameInfo.pixelAspectRatio = tmpAspectRatio;
		invalidateFrameCache = 1;
#ifdef DEBUG_VALUES
		CFShow( CFSTR("IN_SEQUENCE_DATA.frameInfo.pixelAspectRatio != params[pixel aspect ratio]. It's now set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &IN_SEQUENCE_DATA.frameInfo.pixelAspectRatio );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	}
	

	if ( invalidateFrameCache || setupPDF )
	{
#ifdef DEBUG_CALLING_SEQUENCE
		CFShow( CFSTR("Entered FrameSetup() (invalidateFrameCache || setupPDF)") );
#endif
		if ( IN_SEQUENCE_DATA.frameInfo.frameCache ) CGContextRelease_p( IN_SEQUENCE_DATA.frameInfo.frameCache );
		IN_SEQUENCE_DATA.frameInfo.frameCache = NULL;
	}
	
#ifdef DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR( "PF_Cmd_FRAME_SETUP ended" ) );
#endif	

	if ( forceRerender ) out_data->out_flags |=	PF_OutFlag_FORCE_RERENDER;
	return err;
}


static PF_Err 
Render (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
#pragma unused ( params, out_data )
	CGContextRef bitmapContext = IN_SEQUENCE_DATA.frameInfo.bitmapContext;
	CGRect bitmapContextRect = CGRectMake( 0, 0, CGBitmapContextGetWidth( bitmapContext ), CGBitmapContextGetHeight( bitmapContext ) );
	float scale		= IN_SEQUENCE_DATA.frameInfo.scale * IN_SEQUENCE_DATA.frameInfo.scaleMultiplier;
	float rotate	= -(IN_SEQUENCE_DATA.frameInfo.rotation * PF_RAD_PER_DEGREE); // rotation UI is opposite of this value
	float centerX   = IN_SEQUENCE_DATA.frameInfo.centerX;
	float centerY   = IN_SEQUENCE_DATA.frameInfo.centerY;
	float pivotX	= IN_SEQUENCE_DATA.frameInfo.anchorX;
	float pivotY	= IN_SEQUENCE_DATA.frameInfo.anchorY;
	int quality		= IN_SEQUENCE_DATA.frameInfo.quality;
	int antialias   = IN_SEQUENCE_DATA.frameInfo.antialias;
	int intent		= IN_SEQUENCE_DATA.frameInfo.intent;
	int width		= IN_SEQUENCE_DATA.frameInfo.width;
	int height		= IN_SEQUENCE_DATA.frameInfo.height;
	float flatness  = IN_SEQUENCE_DATA.frameInfo.flatness;
	int shouldSmoothFonts = IN_SEQUENCE_DATA.frameInfo.shouldSmoothFonts;
	int shouldDrawPageBackground = IN_SEQUENCE_DATA.frameInfo.shouldDrawPageBackground;
	int shouldPreserveAspectRatio = IN_SEQUENCE_DATA.frameInfo.shouldPreserveAspectRatio;
	float superSampling = IN_SEQUENCE_DATA.frameInfo.superSampling;
	float pixelAspectRatio = IN_SEQUENCE_DATA.frameInfo.pixelAspectRatio;
	int pageRotation = IN_SEQUENCE_DATA.frameInfo.pageRotation;
	float aspectRatio = (((float)width/(float)height) * pixelAspectRatio);
	CGAffineTransform pdfTransform;
	CGRect pdfRect;
	CGRect mediaBoxRect;
	CGPDFPageRef page = IN_SEQUENCE_DATA.frameInfo.pdfPage;
	CGPoint centerPoint;
	CGPoint pivotPoint;
	CGSize newSize;
	CGRect mb = CGPDFPageGetBoxRect_p( page, kCGPDFMediaBox );
	int boxType = IN_SEQUENCE_DATA.frameInfo.boxType;
	DrawStyle drawStyle = IN_SEQUENCE_DATA.frameInfo.drawStyle;
	float red = IN_SEQUENCE_DATA.frameInfo.red;
	float green = IN_SEQUENCE_DATA.frameInfo.green;
	float blue = IN_SEQUENCE_DATA.frameInfo.blue;
	float alpha = IN_SEQUENCE_DATA.frameInfo.alpha;
	CGColorSpaceRef colorSpace;
	CGColorRef backgroundColor;
	float components[4];
	int doSubpixelRendering = IN_SEQUENCE_DATA.frameInfo.doSubpixelRendering;

	int i;
	int imageBytesRowBytes =  CGBitmapContextGetBytesPerRow_p( bitmapContext );
	char *imageBytes;
	vImage_Error imgErr = kvImageNoError;
#ifdef 	DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR( "PF_Cmd_RENDER called" ) );
#endif
	// get color space for context
	if ( IN_SEQUENCE_DATA.frameInfo.shouldColorManage )
	{
		CMProfileRef aProfile;

		CMGetDefaultProfileBySpace(cmRGBData, &aProfile);
		if ( aProfile == NULL ) return PF_Err_OUT_OF_MEMORY;
		
		colorSpace = CGColorSpaceCreateWithPlatformColorSpace( aProfile );

		CMCloseProfile( aProfile );
	}
	else colorSpace = CGColorSpaceCreateDeviceRGB_p();
	if ( colorSpace == NULL ) return PF_Err_OUT_OF_MEMORY;
#ifdef DEBUG_VALUES	
	CFShow( colorSpace );
#endif
	// set background color
	components[0] = red; components[1] = green; components[2] = blue; components[3] = alpha;
	backgroundColor = CGColorCreate_p( colorSpace, components );
	if ( !backgroundColor )
	{
		CFRelease(colorSpace);
		return PF_Err_OUT_OF_MEMORY;
	}
#ifdef DEBUG_VALUES					
	CFShow( CFCopyDescription(backgroundColor) );
#endif	
	// set up our transformation parameters
	
	if ( doSubpixelRendering )
	{
		centerPoint = CGPointMake( centerX*superSampling, centerY*superSampling );
		pivotPoint = CGPointMake( pivotX*superSampling, pivotY*superSampling );
		newSize = CGSizeMake( width*superSampling, height*superSampling );
	}
	else
	{
		centerPoint = CGPointMake( centerX, centerY );
		pivotPoint = CGPointMake( pivotX, pivotY );
		newSize = CGSizeMake( width, height );
	}

	if ( doSubpixelRendering )
	{
		// this gives us a square pixels box
		if ( pixelAspectRatio < 1.0 ) // grow the height
			pdfRect = CGRectMake( 0, 0, width*superSampling, (width/aspectRatio)*superSampling );
		else // grow the width
			pdfRect = CGRectMake( 0, 0, (height*aspectRatio)*superSampling, height*superSampling );
	}
	else
	{
		// this gives us a square pixels box
		if ( pixelAspectRatio < 1.0 ) // grow the height
			pdfRect = CGRectMake( 0, 0, width, (width/aspectRatio) );
		else // grow the width
			pdfRect = CGRectMake( 0, 0, (height*aspectRatio), height );
	}

	
#ifdef DEBUG_VALUES
		CFShow( CFSTR("pdfRect.size.width is set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &pdfRect.size.width );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
#ifdef DEBUG_VALUES
		CFShow( CFSTR("pdfRect.size.height is set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &pdfRect.size.height );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif

	pdfTransform = GetDrawingTransformToCenterAndScaleSourceRectInDestinationRect(	mb,
																					pdfRect, // bitmapContextRect,
																					shouldPreserveAspectRatio	);

	// this accounts for the pixel aspect ratio
	if ( pixelAspectRatio < 1.0 ) // shrink the height
		pdfTransform = CGAffineTransformScale_p( pdfTransform, 1.0, pixelAspectRatio );
	else // shrink the width
		pdfTransform = CGAffineTransformScale_p( pdfTransform, 1.0/pixelAspectRatio, 1.0 );
#ifdef DEBUG_VALUES		
	CFShow( bitmapContext );
#endif
	// push the graphics context (this is pre-setup in the semi-variable setup stage)
	CGContextSaveGState_p( bitmapContext );
	
	// setup our rendering options
	CGContextSetInterpolationQuality_p( bitmapContext, quality );
	CGContextSetShouldAntialias_p( bitmapContext, antialias );
	CGContextSetRenderingIntent_p( bitmapContext, intent );
	CGContextSetFlatness_p( bitmapContext, flatness );
	CGContextSetShouldSmoothFonts_p( bitmapContext, shouldSmoothFonts );
	
	CGContextClearRect_p( bitmapContext, bitmapContextRect ); // this gives us a transparent background

	if ( shouldDrawPageBackground )
	{
		CGContextSetFillColorWithColor_p( bitmapContext, backgroundColor );
#ifdef DEBUG_VALUES
		CFShow( CFSTR("boxType is set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &boxType );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
		// we do the full background fill here (early) because it's convenient
		if ( boxType == -1 ) CGContextFillRect_p( bitmapContext, bitmapContextRect );
	}
	// adjust point measurement because pdfTransform screws up the scaling
	centerPoint = CGPointApplyAffineTransform_p ( centerPoint, CGAffineTransformInvert_p( pdfTransform ) );
	pivotPoint = CGPointApplyAffineTransform_p ( pivotPoint, CGAffineTransformInvert_p( pdfTransform ) );
	
	centerX = centerPoint.x;
	centerY = centerPoint.y;
	pivotX = pivotPoint.x;
	pivotY = pivotPoint.y;
	
	newSize = CGSizeApplyAffineTransform_p ( newSize, CGAffineTransformInvert_p( pdfTransform ) );

#ifdef DEBUG_VALUES
		CFShow( CFSTR("newSize.width is set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &newSize.width );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
#ifdef DEBUG_VALUES
		CFShow( CFSTR("newSize.height is set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &newSize.height );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
	CGContextConcatCTM_p( bitmapContext, pdfTransform );
	
	// this correctly tracks the center point
	CGContextTranslateCTM_p( bitmapContext, -((mb.size.width/2) - centerX), ((mb.size.height/2) - centerY) );
	
	// this correctly scales and rotates, taking into account the pivot point
	CGContextTranslateCTM_p( bitmapContext, pivotX, mb.size.height - pivotY );
	CGContextScaleCTM_p( bitmapContext, scale, scale );
	CGContextRotateCTM_p( bitmapContext, rotate );
	CGContextTranslateCTM_p( bitmapContext, -pivotX, pivotY - mb.size.height );

	if ( shouldDrawPageBackground )
	{
		CGContextSetFillColorWithColor_p( bitmapContext, backgroundColor );
#ifdef DEBUG_VALUES
		CFShow( CFSTR("boxType is set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &boxType );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
		if ( boxType != -1 && boxType >= 0 && boxType < 5 ) // it's a valid box type
		{
			mediaBoxRect = CGPDFPageGetBoxRect_p( page, boxType );
			CGContextFillRect_p( bitmapContext, mediaBoxRect );
#ifdef DEBUG_VALUES
		CFShow( CFSTR("mediaBoxRect.size.width is set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &mediaBoxRect.size.width );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
#ifdef DEBUG_VALUES
		CFShow( CFSTR("mediaBoxRect.size.height is set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberFloat32Type, &mediaBoxRect.size.height );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
		}
	}
	
	CFRelease( backgroundColor );
#ifdef DEBUG_VALUES
	CFShow( page );
#endif
	CGContextDrawPDFPage( bitmapContext, page );
	
	//	restore the graphics context
	CGContextRestoreGState_p( bitmapContext );
	
	if ( doSubpixelRendering )
	{
		// downsample from source to destination
		vImage_Buffer sourceBuf;
		vImage_Buffer destBuf;
		vImage_Error vErr = 0;
		sourceBuf.width = width*superSampling;
		sourceBuf.height = height*superSampling;
		sourceBuf.rowBytes = CGBitmapContextGetBytesPerRow_p( bitmapContext );
		sourceBuf.data = CGBitmapContextGetData( bitmapContext );
		
#ifdef DEBUG_VALUES
		CFShow( CFSTR("sourceBuf.width is set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &sourceBuf.width );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
#ifdef DEBUG_VALUES
		CFShow( CFSTR("sourceBuf.height is set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &sourceBuf.height );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
		destBuf.width = output->width;
		destBuf.height = output->height;
		destBuf.rowBytes = output->rowbytes;
		destBuf.data = output->data;

#ifdef DEBUG_VALUES
		CFShow( CFSTR("destBuf.width is set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &destBuf.width );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif
#ifdef DEBUG_VALUES
		CFShow( CFSTR("destBuf.height is set to:") );
		if (1) {
		CFNumberRef n = CFNumberCreate( NULL, kCFNumberIntType, &destBuf.height );
		CFShow( CFCopyDescription(n) );
		CFRelease(n);
		}
#endif

#ifdef DEBUG_VALUES		
		CFShow( CFSTR("wants to do subpixel rendering") );
#endif
		switch ( drawStyle ) {
		
		case kDrawWithLanczos3Resize:
			vErr = vImageScale_ARGB8888_p( &sourceBuf, &destBuf, IN_SEQUENCE_DATA.frameInfo.tmpBuf, ( kvImageEdgeExtend ) );
			if ( vErr ) return PF_Err_INTERNAL_STRUCT_DAMAGED;
			break;
			
		case kDrawWithLanczos5Resize:
			vErr = vImageScale_ARGB8888_p( &sourceBuf, &destBuf, IN_SEQUENCE_DATA.frameInfo.tmpBuf, ( kvImageEdgeExtend | kvImageHighQualityResampling ) );
			if ( vErr ) return PF_Err_INTERNAL_STRUCT_DAMAGED;
			break;
		}
	}
	else
	{
#ifdef DEBUG_VALUES
		CFShow( CFSTR("copy from source to destination") );
#endif
		// simply copy from source to destination
		imageBytes = (void *)CGBitmapContextGetData( bitmapContext );
		for ( i = 0; i < output->height; i++ )
		{
			char *imageBytesRowPtr = imageBytes + ( i * imageBytesRowBytes ) ; // the AE SDK has an extra +4, not sure why
			char *outputBytesRowPtr = (char *)output->data + ( i * output->rowbytes );
			
			memcpy( outputBytesRowPtr, imageBytesRowPtr, (output->width*4) );
		}
	}
	
	CFRelease(colorSpace);
	
	// I'm a little bit concerned about the memory management here
	IN_SEQUENCE_DATA.frameInfo.frameCache = bitmapContext;
	CFRetain( IN_SEQUENCE_DATA.frameInfo.frameCache );
	
	return PF_Err_NONE;
}


static PF_Err 
GlobalSetdown (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
#pragma unused ( in_data, out_data, params, output )
#ifdef 	DEBUG_CALLING_SEQUENCE
	CFShow( CFSTR( "PF_Cmd_GLOBAL_SETDOWN called" ) );
#endif
	if ( ! ampedeMsgPort ) return PF_Err_NONE;
	
	CFMessagePortSendRequest(  ampedeMsgPort,
								'QUIT',
								NULL,
								1.0, // send timeout in seconds
								10.0, // receive timeout in seconds
								*kCFRunLoopDefaultMode, // run loop reply mode
								NULL );
	CFRelease( ampedeMsgPort );
	return PF_Err_NONE;
}

vImage_Error InitvImage_Buffer( vImage_Buffer *result, int height, int width, size_t bytesPerPixel )
{
	size_t rowBytes = width * bytesPerPixel;

	// Widen rowBytes out to a integer multiple of 16 bytes
	rowBytes = (rowBytes + 15) & ~15;

	// Make sure we are not an even power of 2 wide. Will loop a few times for rowBytes <= 16.
	while ( 0 == (rowBytes & (rowBytes - 1) ) ) rowBytes += 16; //grow rowBytes

	//Set up the buffer
	result->height = height;
	result->width = width;
	result->rowBytes = rowBytes;
	result->data = malloc( rowBytes * height );

	if (result->data == NULL) return kvImageMemoryAllocationError;

	return kvImageNoError;
}

void FreevImage_Buffer( vImage_Buffer *buffer )
{
	if( buffer && buffer->data ) free( buffer->data );
	buffer->data = NULL;
	buffer->width = 0;
	buffer->height = 0;
	buffer->rowBytes = 0;
}

