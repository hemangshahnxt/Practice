/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 1999 Canfield Clinical Systems
//	All Rights Reserved.
//
//	File name:			T2KLIB.H
//	Description:		t2k/bitmap conversions
//	Original Author:	Jamie
//	Current Owner:		Jamie
//
//	Date	Name		Description
//	26Aug97	Jamie		creation
//
///////////////////////////////////////////////////////////////////////////////

#if	!defined(_T2KLIB_H_)
#define _T2KLIB_H_

#pragma once



//Each .T2K file has a thumbnail image (called a "stamp"), 
//an original image, and may have a modified image.
//
//Currently, the thumbnail image is always 64x64 pixels, but this may change
//in the future.
//
//The original and the modified images can be any size, but the original and
//modified will always be the same size.
//
//The "Bitmap" functions are designed for use with C/C++
//
//The "Picture" functions are designed for use with Visual Basic
//
//The following will define the dll exports for use with Visual Basic:
//	Declare Function T2KGetStampPicture Lib "t2klib" (ByVal filename As String) As Picture
//	Declare Function T2KGetOriginalPicture Lib "t2klib" (ByVal filename As String) As Picture
//	Declare Function T2KGetModifiedPicture Lib "t2klib" (ByVal filename As String) As Picture
//
//Note that the filename parameter to these functions includes the path

extern "C"
{
	// Load stamp from T2K, return handle of bitmap
	HBITMAP
		WINAPI T2KGetStampBitmap(LPCTSTR filename);

	// Load original image from T2K, return handle of bitmap
	HBITMAP
		WINAPI T2KGetOriginalBitmap(LPCTSTR filename);

	// Load modified image from T2K, return handle of bitmap
	// (returns original image if no modified image exists)
	HBITMAP
		WINAPI T2KGetModifiedBitmap(LPCTSTR filename);

	// Load stamp from T2K, return IDispatch of Picture for VB
	IDispatch *
		WINAPI T2KGetStampPicture(LPCTSTR filename);

	// Load original image from T2K,  return IDispatch of Picture for VB
	IDispatch *
		WINAPI T2KGetOriginalPicture(LPCTSTR filename);

	// Load original image from T2K,  return IDispatch of Picture for VB
	IDispatch *
		WINAPI T2KGetModifiedPicture(LPCTSTR filename);
}

/////////////////////////////////////////////////////////////////////////////
#endif

