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

/**	AE_Macros.h
	
	Part of the Adobe After Effects SDK.
	
	REVISION HISTORY	
		06/12/96	bsa		Updated for After Effects 3.1
		04/06/97	bsa		Updated for After Effects 3.1 Windows version
		03/01/99	bbb		Added DH.
		2/17/03		bbb		Added a bunch of rect macros, ERR() and ERR2()

**/

#ifndef _H_AE_MACROS
#define _H_AE_MACROS

#ifndef ERR
	#define ERR(FUNC)		do { if (!err) { err = (FUNC); } } while (0)
#endif
#ifndef ERR2
	#define ERR2(FUNC)		do { if (((err2 = (FUNC)) != A_Err_NONE) && !err) err = err2; } while (0)
#endif

#define	INSET_RECT(R,DH,DV)			\
	(R).left+=(DH), (R).right-=(DH), (R).top+=(DV), (R).bottom-=(DV)

#define	OFFSET_RECT(R,DH,DV)			\
	(R).left+=(DH), (R).right+=(DH), (R).top+=(DV), (R).bottom+=(DV)


#define			AEFX_CLR_STRUCT(STRUCT)		\
	do {									\
		long _t = sizeof(STRUCT);			\
		char *_p = (char*)&(STRUCT);		\
		while (_t--) {						\
			*_p++ = 0;						\
		}									\
	} while (0);										


#ifndef AEFX_COPY_STRUCT

#define			AEFX_COPY_STRUCT(FROM, TO)	\
	do {									\
		long _t = sizeof(FROM);				\
		char *_p = (char*)&(FROM);			\
		char *_q = (char*)&(TO);			\
		while (_t--) {						\
			*_q++ = *_p++;					\
		}									\
	} while (0);	

#endif

#define DH(h)				(*(h))

#define FIX2INT(X)				((long)(X) >> 16)
#define INT2FIX(X)				((long)(X) << 16)
#define FIX2INT_ROUND(X)		(FIX2INT((X) + 32768))
#define	FIX_2_FLOAT(X)			((double)(X) / 65536.0)
#define	FLOAT_2_FIX(F)			((PF_Fixed)((F) * 65536 + (((F) < 0) ? -0.5 : 0.5)))
#define ABS(N)					((N) < 0 ? -(N) : (N))
#define MIN(A,B)			((A) < (B) ? (A) : (B))
#define ABS(N)				((N) < 0 ? -(N) : (N))
#define ROUND_DBL2LONG(DBL) ((DBL) > 0 ? (long)((DBL) + 0.5) : ((long)(DBL + 0.5) == (DBL + 0.5) ? (DBL) : (long)((DBL) - 0.5)))


#define A_Fixed_1			((A_Fixed)0x00010000L)
#define A_Fixed_ONE			((A_Fixed)0x00010000L)
#define A_Fixed_HALF		((A_Fixed)0x00008000L)


#define	RECT_WIDTH(R)					((R).right - (R).left)
#define	RECT_HEIGHT(R)					((R).bottom - (R).top)
#define	RECT_X_CENTER(R)				(((R).right + (R).left) >> 1)
#define	RECT_Y_CENTER(R)				(((R).top + (R).bottom) >> 1)


#define	PF_RECT_2_FIXEDRECT(R,FR)	do {	\
	(FR).left = INT2FIX((R).left);			\
	(FR).top = INT2FIX((R).top);			\
	(FR).right = INT2FIX((R).right);		\
	(FR).bottom = INT2FIX((R).bottom);		\
	} while (0)

#define	PF_FIXEDRECT_2_RECT(FR,R)	do {			\
	(R).left = (short)FIX2INT_ROUND((FR).left);		\
	(R).top  = (short)FIX2INT_ROUND((FR).top);		\
	(R).right = (short)FIX2INT_ROUND((FR).right);	\
	(R).bottom = (short)FIX2INT_ROUND((FR).bottom);	\
	} while (0)

#define	RECT_EMPTY(A)					\
	(M_Rect_WIDTH(A) <= 0 || M_Rect_HEIGHT(A) <= 0)
#define	EMPTY_RECT(A)					M_RECT_EMPTY(A)

#define	RECT_ENCLOSES(OUT,IN)								\
	((OUT).left < (IN).left && (OUT).right  > (IN).right &&	\
	 (OUT).top  < (IN).top  && (OUT).bottom > (IN).bottom)

#define	RECT_ENCLOSES_OR_EQUALS(O,I)					\
	((O).left <= (I).left && (O).right  >= (I).right &&	\
	 (O).top  <= (I).top  && (O).bottom >= (I).bottom)

#define	PT_IN_RECT(P,R)						\
	(M_IN_RANGE((R).left,(P).h,(R).right) &&	\
	 M_IN_RANGE((R).top,(P).v,(R).bottom))

//#define	FIX2INT_ROUND(F)		FIX2INT((F) + A_Fixed_HALF)

#define SWAP_LONG(a)		((a >> 24) | ((a >> 8) & 0xff00) | ((a << 8) & 0xff0000) | (a << 24))


#define CONVERT8TO16(A)		( (((long)(A) * PF_MAX_CHAN16) + PF_HALF_CHAN8) / PF_MAX_CHAN8 )
#define CONVERT16TO8(A)		( (((long)(A) * PF_MAX_CHAN8) + PF_HALF_CHAN16) / PF_MAX_CHAN16)

#define RATIO2FLOAT(R)		(A_FpLong)((A_FpLong)(R).num / ((A_FpLong)(R).den))
#define RATIO2FIX(R)		(A_Fixed)(A_Fixed_1 * ((A_FpLong)(R).num / ((A_FpLong)(R).den)))
#endif		// _H_AX_MACROS
