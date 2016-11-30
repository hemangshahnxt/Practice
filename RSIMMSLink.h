//
// (c.haag 2009-07-08 11:32) - PLID 34379 - Initial implementation
//
// This namespace provides Practice with basic functionality to show images from MSI
// Medical Media System in NexTech Practice
//
#pragma once

namespace RSIMMSLink
{
	BOOL IsLinkEnabled();
	void EnsureNotLink();
	long GetInternalPatientID(const CString& strPatFirst, const CString& strPatLast);
	long GetImageCount(long nInternalPatientID);
	HBITMAP LoadImage(long nInternalPatientID, long nIndex, OUT CString* pstrFullPathName);
};