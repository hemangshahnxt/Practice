//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_PRINTALIGNDLG_H__7B4C52C4_4423_11D3_AD42_00104B318376__INCLUDED_)
#define AFX_PRINTALIGNDLG_H__7B4C52C4_4423_11D3_AD42_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrintAlignDlg.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// CPrintAlignDlg dialog

class CPrintAlignDlg : public CNxDialog
{
// Construction
public:
	NXDATALISTLib::_DNxDataListPtr m_ComboPrinters;
	CPrintAlignDlg(CWnd* pParent);   // standard constructor

	void UpdateControls();
	void OnOK();

	BOOL Save();
	
	long m_nCurrentFormAlignID;
	int m_FormID;

// Dialog Data
	//{{AFX_DATA(CPrintAlignDlg)
	enum { IDD = IDD_PRINT_ALIGN_DLG };
	// (j.jones 2007-04-25 14:16) - PLID 4758 - added ability to set a default per workstation
	NxButton	m_CheckDefaultForMe;
	CNxEdit	m_EditYScale;
	CNxEdit	m_EditXScale;
	CNxIconButton	m_down;
	CNxIconButton	m_up;
	CNxIconButton	m_right;
	CNxIconButton	m_left;
	CNxEdit	m_EditMini;
	NxButton	m_CheckDefault;
	CNxEdit	m_EditYShift;
	CNxEdit	m_EditXShift;
	CNxEdit	m_EditFont;
	CNxStatic	m_nxstaticMiniFontLabel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDelete;
	NxButton	m_btnAdvancedSettingsGroupbox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrintAlignDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrintAlignDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddRecordComboPrinters();
	afx_msg void OnDeleteRecordComboPrinters(const VARIANT FAR& varBoundItem);
	afx_msg void OnSelChosenComboPrinters(long nNewSel);
	afx_msg void OnRequeryFinishedComboPrinters(short nFlags);
	afx_msg void OnDelete();
	afx_msg void OnUp();
	afx_msg void OnRight();
	afx_msg void OnDown();
	afx_msg void OnLeft();
	afx_msg void OnAdd();
	virtual void OnCancel();
	afx_msg void OnCheckDefault();
	// (j.jones 2007-04-25 14:16) - PLID 4758 - added ability to set a default per workstation
	afx_msg void OnCheckDefaultForMe();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRINTALIGNDLG_H__7B4C52C4_4423_11D3_AD42_00104B318376__INCLUDED_)
