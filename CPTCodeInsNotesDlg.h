#if !defined(AFX_CPTCODEINSNOTESDLG_H__F97153EB_B3D9_4B1A_91C9_C0990112FC5B__INCLUDED_)
#define AFX_CPTCODEINSNOTESDLG_H__F97153EB_B3D9_4B1A_91C9_C0990112FC5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CPTCodeInsNotesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCPTCodeInsNotesDlg dialog

class CCPTCodeInsNotesDlg : public CNxDialog
{
// Construction
public:
	CCPTCodeInsNotesDlg(CWnd* pParent);   // standard constructor

	long m_nInsCoID;
	long m_nCPTCodeID;
	long m_nDiagCodeID;

	NXDATALISTLib::_DNxDataListPtr m_InsCoList, m_CPTList, m_pDiagList;

	void Load();
	void Save();

	// (z.manning, 04/30/2008) - PLID 29852 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CCPTCodeInsNotesDlg)
	enum { IDD = IDD_CPT_CODE_INS_NOTES_DLG };
	NxButton	m_btnSvcCode;
	NxButton	m_btnDiagCode;
	CNxEdit	m_nxeditEditCptNotes;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCPTCodeInsNotesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCPTCodeInsNotesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusEditCptNotes();
	afx_msg void OnSelChosenCptList(long nRow);
	afx_msg void OnSelChosenInscosList(long nRow);
	afx_msg void OnServiceCodeNote();
	afx_msg void OnDiagCodeNote();
	afx_msg void OnSelChosenDiagList(long nRow);
	afx_msg void OnAdvancedCodeNotes();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CPTCODEINSNOTESDLG_H__F97153EB_B3D9_4B1A_91C9_C0990112FC5B__INCLUDED_)
