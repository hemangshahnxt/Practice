// (c.haag 2010-06-22 16:57) - PLID 39295 - Moved class into its own source/header pair
#pragma once

// (c.haag 2010-03-30 17:13) - PLID 36327 - We now support handling events fired from the NxManagedWrapper.
// At present, we only have OnGoToPatient.
// (c.haag 2010-04-16 12:05) - PLID 36457 - Added OnConvertFilterStringToClause
// (c.haag 2010-06-08 11:29) - PLID 38898 - Added OnShowImportForm
class CNxManagedWrapperEventSink : public CCmdTarget
{
	DECLARE_DYNCREATE(CNxManagedWrapperEventSink)
	CNxManagedWrapperEventSink();    // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxManagedWrapperEventSink)
public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNxManagedWrapperEventSink();

	// Attempts to establish a connection with the photo tab object
	void EnsureSink(NxManagedWrapperLib::INxPhotoTabPtr pNxPhotoTab);
	// (c.haag 2010-06-22 13:37) - PLID 39295 - We now support sinks for import objects
	void EnsureSink(NxManagedWrapperLib::INxPhotoImportFormPtr pNxPhotoTab);
	// Disconnects from all sources
	void CleanUp();

protected:
	LPDISPATCH m_pNxPhotoTabSink; // The object we have a sink for
	DWORD m_dwNxPhotoTabCookie; // The cookie we have a sync for

	// (c.haag 2010-06-22 13:37) - PLID 39295 - We now support sinks for import objects
	LPDISPATCH m_pNxPhotoImportFormSink; // The object we have a sink for
	DWORD m_dwNxPhotoImportFormCookie; // The cookie we have a sync for

protected:
	// Generated message map functions
	//{{AFX_MSG(CNxManagedWrapperEventSink)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CNxManagedWrapperEventSink)
	virtual afx_msg void PhotoTab_OnGoToPatient(long nPersonID, BOOL* pbSuccess);
	virtual afx_msg void PhotoTab_OnConvertFilterStringToClause(long nFilterID, LPCTSTR strFilter, BOOL* pbSuccess, BSTR* bstrFrom, BSTR* bstrWhere);
	virtual afx_msg void PhotoTab_OnShowImportForm();
	virtual afx_msg void PhotoImportForm_OnVisibilityChanged(BOOL bVisible);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};