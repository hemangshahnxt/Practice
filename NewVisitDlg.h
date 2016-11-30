#if !defined(AFX_NEWVISITDLG_H__349B7BBD_551F_402F_A4D5_19876B589258__INCLUDED_)
#define AFX_NEWVISITDLG_H__349B7BBD_551F_402F_A4D5_19876B589258__INCLUDED_

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
#define OUTCOMES_CUSTOM_LIST__NO_SELECTION			" { No Selection } "
#define OUTCOMES_CUSTOM_LIST__MULTIPLE_SELECTIONS	" { Multiple Selections } "

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewVisitDlg.h : header file
//

#include "EyeDrawDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CNewVisitDlg dialog

class CNewVisitDlg : public CNxDialog
{
// Construction
public:

	CEyeDrawDlg &m_LeftEyeDrawDlg, &m_RightEyeDrawDlg;
	void Load();
	CNewVisitDlg(CWnd* pParent);   // standard constructor
	~CNewVisitDlg();
	long m_nProcID;
	long m_nLeftVisitID, m_nRightVisitID;

	BOOL m_bIsNew;

	//m.hancock - 7-05-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
	CDWordArray m_adwCustomList1;
	CDWordArray m_adwCustomList2;
	CDWordArray m_adwCustomList3;
	CDWordArray m_adwCustomList4;
	CRect m_rcCustom1;
	CRect m_rcCustom2;
	CRect m_rcCustom3;
	CRect m_rcCustom4;

// Dialog Data
	//{{AFX_DATA(CNewVisitDlg)
	enum { IDD = IDD_NEW_VISIT };
	CDateTimePicker	m_dtPicker;
	CNxEdit	m_nxeditCustomNv1;
	CNxEdit	m_nxeditCustomNv2;
	CNxEdit	m_nxeditCustomNv3;
	CNxEdit	m_nxeditCustomNv4;
	CNxEdit	m_nxeditVisitNotes;
	CNxStatic	m_nxstaticLeftEyeDraw;
	CNxStatic	m_nxstaticRightEyeDraw;
	CNxStatic	m_nxstaticCustlist1;
	CNxStatic	m_nxstaticCustlist2;
	CNxStatic	m_nxstaticCustlist3;
	CNxStatic	m_nxstaticCustlist4;
	CNxIconButton	m_btnAddTest;
	CNxIconButton	m_btnDeleteTest;
	CNxIconButton	m_btnSave;
	CNxIconButton	m_btnCancel;
	CNxStatic	m_nxstaticCustomLabel1;
	CNxStatic	m_nxstaticCustomLabel2;
	CNxStatic	m_nxstaticCustomLabel3;
	CNxStatic	m_nxstaticCustomLabel4;
	CNxStatic	m_nxstaticCustomlistLabel1;
	CNxStatic	m_nxstaticCustomlistLabel2;
	CNxStatic	m_nxstaticCustomlistLabel3;
	CNxStatic	m_nxstaticCustomlistLabel4;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewVisitDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
	void EditCustomList(NXDATALISTLib::_DNxDataListPtr &list, long listID, CDWordArray *adwCustomSelection, CRect &rc);
	void RequeryFinishedCustomList(NXDATALISTLib::_DNxDataListPtr &customCombo);
	void RefreshCustomCombo(NXDATALISTLib::_DNxDataListPtr &customCombo, CDWordArray *adwCustomSelection, CRect &rc);
	void DrawCustomHyperlinkList(CDC *pdc, CRect &rc, NXDATALISTLib::_DNxDataListPtr &customCombo, CDWordArray *adwCustomSelection);
	CString GetMultiSelectString(NXDATALISTLib::_DNxDataListPtr &customCombo, CDWordArray *adwCustomSelection);
	void DoClickHyperlink(UINT nFlags, CPoint point);
	void SelectionChosenCustomList(long nCurSel, NXDATALISTLib::_DNxDataListPtr &customCombo, CDWordArray *adwCustomSelection, CRect &rc);
	CString GetCustomListInsertStmt(CDWordArray *adwCustomSelection);
	void LoadCustomListData(NXDATALISTLib::_DNxDataListPtr &customCombo, CDWordArray *adwCustomSelection, CRect &rc, long nFieldID);

	NXDATALISTLib::_DNxDataListPtr m_visitTypeCombo, m_ratingCombo;
	NXDATALISTLib::_DNxDataListPtr m_custom1Combo;
	NXDATALISTLib::_DNxDataListPtr m_custom2Combo;
	NXDATALISTLib::_DNxDataListPtr m_custom3Combo;
	NXDATALISTLib::_DNxDataListPtr m_custom4Combo;

	//m.hancock - PLID 16756 - Add advanced test types for eye visits
	NXDATALISTLib::_DNxDataListPtr m_TestsList;
	bool ValidTestSelection(long nTestID);
	bool IsTestDataEmpty(long nRow);
	
	BOOL ChangeCustomLabel(int nID);
	int GetLabelFieldID(int nID);

	// Generated message map functions
	//{{AFX_MSG(CNewVisitDlg)
	afx_msg void OnSaveButton();
	virtual BOOL OnInitDialog();
	afx_msg void OnAddVisitType();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnBtnEditRatings();
	afx_msg void OnEditCustomList1();
	afx_msg void OnEditCustomList2();
	afx_msg void OnEditCustomList3();
	afx_msg void OnEditCustomList4();
	afx_msg void OnRequeryFinishedCustlist1(short nFlags);
	afx_msg void OnRequeryFinishedCustlist2(short nFlags);
	afx_msg void OnRequeryFinishedCustlist3(short nFlags);
	afx_msg void OnRequeryFinishedCustlist4(short nFlags);
	afx_msg void OnSelChosenCustlist1(long nRow);
	afx_msg void OnSelChosenCustlist2(long nRow);
	afx_msg void OnSelChosenCustlist3(long nRow);
	afx_msg void OnSelChosenCustlist4(long nRow);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnAddTestButton();
	afx_msg void OnEditingFinishingTestsList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnDeleteTestButton();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWVISITDLG_H__349B7BBD_551F_402F_A4D5_19876B589258__INCLUDED_)
