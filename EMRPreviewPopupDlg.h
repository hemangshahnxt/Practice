#if !defined(AFX_EMRPREVIEWPOPUPDLG_H__3A903BFF_9C31_4780_85C7_3FE1556AB8B0__INCLUDED_)
#define AFX_EMRPREVIEWPOPUPDLG_H__3A903BFF_9C31_4780_85C7_3FE1556AB8B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRPreviewPopupDlg.h : header file
//

#include "EMRPreviewCtrlDlg.h"
#include "EmrRc.h"
#include "PatientNexEMRDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CEMRPreviewPopupDlg dialog

// (z.manning 2012-09-10 12:10) - PLID 52543 - Simple struct containing all the info we need for displaying
// popped up EMN previews.
struct EmnPreviewPopup
{
	long nID;
	COleDateTime dtModifiedDate;

	EmnPreviewPopup()
	{
		nID = -1;
		dtModifiedDate = g_cdtInvalid;
	}

	EmnPreviewPopup(const long nEmnID, const COleDateTime dtEmnModifiedDate)
	{
		nID = nEmnID;
		dtModifiedDate = dtEmnModifiedDate;
	}
};

class CEMRPreviewPopupDlg : public CNxDialog
{
// Construction
public:
	CEMRPreviewPopupDlg(CWnd* pParent);   // standard constructor
	~CEMRPreviewPopupDlg(); // destructor

	// (a.walling 2010-01-11 12:36) - PLID 36837 - Support different 'subsections' for positioning preferences
	void RestoreSize(const CString& strSubsection);

	CEMRPreviewCtrlDlg* m_pEMRPreviewCtrlDlg;

	// (j.jones 2009-09-22 11:44) - PLID 31620 - Since the preview popup has
	// previous & next buttons, we need to know what EMNs are available to
	// iterate through, and what order. These EMN IDs should be passed in
	// through aryEMNIDs. The EMN we wish to display should be referenced
	// through nCurIndex, indicating which index of the array is displayed.
	// (z.manning 2012-09-10 12:14) - PLID 52453 - Use the new EmnPreviewPopup struct
	BOOL PreviewEMN(EmnPreviewPopup emn, const long nCurIndex);
	BOOL PreviewEMN(CArray<EmnPreviewPopup, EmnPreviewPopup&> &aryEMNs, long nCurIndex);

	void RefreshPreview();

	// (j.jones 2009-09-22 12:37) - PLID 31620 - takes in a list of EMNIDs
	// available for this patient
	// (z.manning 2012-09-10 12:14) - PLID 52453 - Use the new EmnPreviewPopup struct
	void SetPatientID(long nNewPatientID, EmnPreviewPopup emn);
	void SetPatientID(long nNewPatientID, CArray<EmnPreviewPopup, EmnPreviewPopup&> &aryEMNs);

	void NavigateToMessage(CString strMessage);

	class CPatientNexEMRDlg* m_pNexEMRDlg;

	//(j.camacho 2015-07-10 4:31pm) - PLID 66500 - Gain access to the protected member 
	const long getCurIndex(){ return m_nCurEMNIndex; };

// Dialog Data
	//{{AFX_DATA(CEMRPreviewPopupDlg)
	enum { IDD = IDD_EMR_PREVIEW_POPUP };
	// (j.jones 2009-09-22 11:19) - PLID 31620 - added previous/next buttons
	CNxIconButton	m_btnPrevious;
	CNxIconButton	m_btnNext;
	CNxIconButton	m_btnClose; // (a.walling 2010-01-11 16:24) - PLID 36837 - close button
	CNxStatic		m_nxstaticPreviewArea;
	// (a.walling 2010-03-24 12:57) - PLID 37677
	CNxLabel		m_nxlOpenThisEMN;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRPreviewPopupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_nPatientID;

	CString m_strTempFile;

	void CleanupTempFiles();
	CStringArray m_arTempFiles;

	// (a.walling 2010-01-11 12:36) - PLID 36837 - Support different 'subsections' for positioning preferences
	CString m_strSubsection;

	// (j.jones 2009-09-22 11:44) - PLID 31620 - Since the preview popup has
	// previous & next buttons, we need to know what EMNs are available to
	// iterate through, and what order. m_naryEMNIDs will track all the
	// available EMNs, in order. m_nCurEMNIndex is the index of the array
	// that should be currently displayed.
	long m_nCurEMNIndex;
	// (a.walling 2011-06-17 15:43) - PLID 42367 - Keep track of the source's file time
	CString m_strCurEMNFileName;
	FILETIME m_ftCurEMNLastWriteTime;
	// (z.manning 2012-09-10 15:59) - PLID 52543 - Use the new EmnPreviewPopup struct
	CArray<EmnPreviewPopup, EmnPreviewPopup&> m_aryEMNs;

	// (a.walling 2011-06-17 15:43) - PLID 42367 - Will refresh the preview if the source file has been modified
	void RefreshPreviewIfModified();

	// (j.jones 2009-09-22 11:47) - PLID 31620 - will display the EMN
	// preview for the EMN at m_nCurEMNIndex
	BOOL DisplayCurrentEMNPreview();

	// (a.walling 2010-03-24 13:07) - PLID 37677 - Open up the current EMN!
	void OpenCurrentEMN();

	// (j.jones 2009-09-22 11:56) - PLID 31620 - enable/disable next/previous buttons
	void UpdateButtons();

	// Generated message map functions
	//{{AFX_MSG(CEMRPreviewPopupDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	// (j.jones 2009-09-22 11:19) - PLID 31620 - added previous/next buttons
	afx_msg void OnBtnEmrPreviewPrev();
	afx_msg void OnBtnEmrPreviewNext();
	// (c.haag 2013-03-07) - PLID 55365 - Repurposed for printing one EMN with multiple layouts, or multiple EMN's
	afx_msg LRESULT OnPrintMultiple(WPARAM wParam, LPARAM lParam); // (a.walling 2009-11-24 13:39) - PLID 36418
	// (a.walling 2010-01-11 18:02) - PLID 36840 - Handle a custom preview command
	afx_msg LRESULT OnCustomEmrPreviewCommand(WPARAM wParam, LPARAM lParam);
	// (a.walling 2010-03-24 12:59) - PLID 37677
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	// (a.walling 2010-01-11 16:13) - PLID 27733 - If we are being 'shown', then make sure we are restored
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	// (a.walling 2011-06-17 15:49) - PLID 42367 - Activate handler
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRPREVIEWPOPUPDLG_H__3A903BFF_9C31_4780_85C7_3FE1556AB8B0__INCLUDED_)
