#if !defined(AFX_VISITDLG_H__9EA187CE_0583_4797_B925_2C6DBACB14A0__INCLUDED_)
#define AFX_VISITDLG_H__9EA187CE_0583_4797_B925_2C6DBACB14A0__INCLUDED_

//m.hancock - 6-30-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
#define OUTCOMES_CUSTOM_LIST__NO_SELECTION			" { No Selection } "
#define OUTCOMES_CUSTOM_LIST__MULTIPLE_SELECTIONS	" { Multiple Selections } "

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VisitDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVisitDlg dialog

class CVisitDlg : public CNxDialog
{
// Construction
public:
	long m_nCurrentID;						//this is filled in by the calling class, and must be the correct ID (in EyeProceduresT) of this dialogs information
	CVisitDlg(BOOL bIsNewRecord, CWnd* pParent);   // standard constructor

	//m.hancock - 7-05-2005 - PLID 16755 - Add custom data list combos to the outcomes dialogs
	CDWordArray m_adwCustomList1;
	CDWordArray m_adwCustomList2;
	CDWordArray m_adwCustomList3;
	CDWordArray m_adwCustomList4;
	CRect m_rcCustom1;
	CRect m_rcCustom2;
	CRect m_rcCustom3;
	CRect m_rcCustom4;
	BOOL m_bNewRecord;
	long m_nProviderID;
	long m_nLocationID;

// Dialog Data
	//{{AFX_DATA(CVisitDlg)
	enum { IDD = IDD_VISIT_DLG };
	NxButton	m_btnLeftEye;
	NxButton	m_btnRightEye;
	NxButton	m_btnWearsContacts;
	NxButton	m_btnMonovision;
	CDateTimePicker	m_dtPicker;
	CNxEdit	m_nxeditProcedureType;
	CNxEdit	m_nxeditOperationType;
	CNxEdit	m_nxeditLaserUsed;
	CNxEdit	m_nxeditEquipmentUsed;
	CNxEdit	m_nxeditKeratomeType;
	CNxEdit	m_nxeditPower;
	CNxEdit	m_nxeditBladeType;
	CNxEdit	m_nxeditNumPulses;
	CNxEdit	m_nxeditPatAge;
	CNxEdit	m_nxeditPupilLight;
	CNxEdit	m_nxeditPupilDark;
	CNxEdit	m_nxeditCustom1;
	CNxEdit	m_nxeditCustom2;
	CNxEdit	m_nxeditCustom3;
	CNxEdit	m_nxeditCustom4;
	CNxEdit	m_nxeditContactLensType;
	CNxEdit	m_nxeditContactLensUse;
	CNxEdit	m_nxeditComplaintsBox;
	CNxStatic	m_nxstaticPatientName;
	CNxStatic	m_nxstaticCustlist1;
	CNxStatic	m_nxstaticCustlist2;
	CNxStatic	m_nxstaticCustlist3;
	CNxStatic	m_nxstaticCustlist4;
	CNxStatic	m_nxstaticVisitTestsNote;
	CNxIconButton	m_btnNewVisit;
	CNxIconButton	m_btnDeleteVisit;
	CNxIconButton	m_btnOk;
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
	//{{AFX_VIRTUAL(CVisitDlg)
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

	void Load();

	NXDATALISTLib::_DNxDataListPtr m_visitsCombo;
	NXDATALISTLib::_DNxDataListPtr m_procedureCombo;
	NXDATALISTLib::_DNxDataListPtr m_providerCombo;
	NXDATALISTLib::_DNxDataListPtr m_locationCombo;
	NXDATALISTLib::_DNxDataListPtr m_custom1Combo;
	NXDATALISTLib::_DNxDataListPtr m_custom2Combo;
	NXDATALISTLib::_DNxDataListPtr m_custom3Combo;
	NXDATALISTLib::_DNxDataListPtr m_custom4Combo;

	CString m_strVisitsWhere;

	BOOL ChangeCustomLabel(int nID);
	int GetLabelFieldID(int nID);
	bool Save();

	// Generated message map functions
	//{{AFX_MSG(CVisitDlg)
	afx_msg void OnNewVisitBtn();
	virtual BOOL OnInitDialog();
	afx_msg void OnLeftEye();
	afx_msg void OnRightEye();
	afx_msg void OnSelChangedProvCombo(long nNewSel);
	afx_msg void OnSelChangedLocationCmb(long nNewSel);
	afx_msg void OnSelChangedProcedureCombo(long nNewSel);
	afx_msg void OnContactsCheck();
	afx_msg void OnMonovision();
	afx_msg void OnChangeSurgeryDate(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnCancel();
	afx_msg void OnDeleteVisitBtn();
	afx_msg void OnLButtonDownVisits(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDblClickCellVisits(long nRowIndex, short nColIndex);
	virtual void OnOK();
	afx_msg void OnEditCustomList1();
	afx_msg void OnEditCustomList2();
	afx_msg void OnEditCustomList3();
	afx_msg void OnEditCustomList4();
	afx_msg void OnRequeryFinishedCustlist1(short nFlags);
	afx_msg void OnRequeryFinishedCustlist2(short nFlags);
	afx_msg void OnRequeryFinishedCustlist3(short nFlags);
	afx_msg void OnRequeryFinishedCustlist4(short nFlags);
	afx_msg void OnSelChosenCustlist1(long nCurSel);
	afx_msg void OnSelChosenCustlist2(long nCurSel);
	afx_msg void OnSelChosenCustlist3(long nCurSel);
	afx_msg void OnSelChosenCustlist4(long nCurSel);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSelChosenProvCombo(long nRow);
	afx_msg void OnSelChosenLocationCombo(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	int m_nSelectedEye;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VISITDLG_H__9EA187CE_0583_4797_B925_2C6DBACB14A0__INCLUDED_)
