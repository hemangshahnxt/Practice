/* (c.haag 2009-09-01 13:00) - PLID 35433 - Initial implementation

This is the header for the Managed C++ NxManagedWrapper library. This library enables Practice
to utilize NexTech C# code without having to directly compile with Common Language Runtime support.

*/

#pragma once

namespace NxManagedWrapper
{
	// Class used to access the C# NexPhoto tab implementation
	class AFX_EXT_CLASS CNexPhoto
	{
	public:
		static void SetConnectionString(const CString& strConn);
		static void SetAppPath(const CString& strAppPath);
		static void SetPatientID(long nPatientID);
		static void SetLocationID(long nLocationID);
		static void SetUserID(long nUserID);
		static void SetLogFileName(const CString& strLogFileName);
	public:
		static CWnd* CreateNexPhotoControl(CWnd* pWndParent);
	public:
		static void UpdateView(CWnd* pWnd);
		static void SetBackgroundColor(CWnd* pWnd, OLE_COLOR clrDlgBack, OLE_COLOR clrNxColor);
	};

	// Class used to access the C# global NexPhoto search implementation
	class AFX_EXT_CLASS CNexPhotoSearch
	{
	public:
		static void RunGlobalSearch();
	};
};
