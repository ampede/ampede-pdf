/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 2003 Adobe Systems Incorporated                       */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Systems Incorporated and its suppliers, if    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Systems Incorporated and its    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Systems         */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/

/** A.h

	Adobe-standard (hopefully one day) types to promote plug-in sharing.

**/
#ifndef _H_A
#define _H_A

#ifdef A_INTERNAL

	#include <A_Private.h>

#else

	typedef long			A_long;
	typedef unsigned long	A_u_long;
	typedef short			A_short;
	typedef unsigned short	A_u_short;
	typedef char			A_char;
	typedef unsigned char	A_u_char;
	typedef double			A_FpLong;
	typedef float			A_FpShort;
	typedef A_long			A_Err;
	typedef unsigned char	A_Boolean;
	typedef void *			A_Handle;

	typedef A_long			A_Fixed;
	typedef A_u_long		A_UFixed;

#ifdef _MSC_VER	// visual c++ 
	typedef unsigned __int64 	A_u_longlong;
#endif

#ifdef __MWERKS__ // metrowerks codewarrior
	typedef unsigned long long	A_u_longlong;
#endif

	typedef struct {
		A_long		value;
		A_u_long	scale;
	} A_Time;

	typedef struct {
		A_long		num;	/* numerator */
		A_u_long	den;	/* denominator */
	} A_Ratio;
	
	typedef struct {
		A_FpLong			x, y;
	} A_FloatPoint;
	
	typedef struct {
		A_FpLong			x, y, z;
	} A_FloatPoint3;

	typedef struct {
		A_FpLong			left, top, right, bottom;
	} A_FloatRect;

	typedef struct {
		A_FpLong			mat[3][3];
	} A_Matrix3;

	typedef struct {
		A_FpLong			mat[4][4];
	} A_Matrix4;

	typedef struct {
		A_short				top, left, bottom, right;
	} A_Rect;

	typedef struct {
		A_long				left, top, right, bottom;
	} A_LRect;

	typedef struct {
		A_long				x, y;
	} A_LPoint;

	typedef struct {
		A_FpLong			radius, angle; 
	} A_FloatPolar; 

#endif

typedef struct {
	A_FpLong		alpha;
	A_FpLong		red;
	A_FpLong		green;
	A_FpLong		blue;

} A_Color;

enum {
	A_Err_NONE = 0,
	A_Err_GENERIC,
	A_Err_STRUCT,
	A_Err_PARAMETER,
	A_Err_ALLOC,
    A_Err_MISSING_SUITE = 13	// acquire suite failed on a required suite 
};

typedef struct {
	A_short		majorS;
	A_short		minorS;
} A_Version;

typedef struct _Up_OpaqueMem **AEGP_MemHandle;

#endif
