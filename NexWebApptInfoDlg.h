#if !defined(AFX_NEXWEBAPPTINFODLG_H__790D26C1_CEF5_4903_9CC3_1CCA5160E1D6__INCLUDED_)
#define AFX_NEXWEBAPPTINFODLG_H__790D26C1_CEF5_4903_9CC3_1CCA5160E1D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexWebApptInfoDlg.h : header file
//
#include "NexWebApptListDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CNexWebApptInfoDlg dialog

class CNexWebApptInfoDlg : public CNxDialog
{
// Construction
public:
	CNexWebApptInfoDlg(long nApptID, BOOL bIsNew, long nPersonID, ApptImport *pAppt, CWnd* pParent);   // standard constructor
	long m_nPersonID;
	long m_nApptID;
	BOOL m_bIsNew;
	NXDATALISTLib::_DNxDataListPtr  m_pLocationList;
	NXDATALISTLib::_DNxDataListPtr  m_pStatusList;
	NXDATALISTLib::_DNxDataListPtr  m_pResourceList;
	NXDATALISTLib::_DNxDataListPtr  m_pTypeList;
	NXDATALISTLib::_DNxDataListPtr  m_pPurposeList;
	// (j.gruber 2007-02-23 10:58) - PLID 24767 - changed moveup from combo box to datalist2
	NXDATALIST2Lib::_DNxDataListPtr  m_pMoveUpList;
	void FillBoxes();
	// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - Need to specify namespace
	NXTIMELib::_DNxTimePtr		m_pStartTime;
	NXTIMELib::_DNxTimePtr		m_pEndTime;
	void InitializeControls(BOOL bEnabled = FALSE);
	void ProcessField(long nFieldID, CString strEntry);
	CArray<long, long>  m_IDsToSave;
	CDWordArray  m_PurposeList;
	CDWordArray m_ResourceList;
	void ProcessMultiSelectList(CDWordArray *pAry, CString *strList, NXDATALISTLib::_DNxDataListPtr pDatalist, int nID, CNxLabel *pLbl, CString strField, CString strTable);
	void ParseDateTimeOnly(CString strDateTime, COleDateTime &dt);
	BOOL OnMultiSelectResourceList();
	BOOL OnMultiSelectPurposeList();
	CString m_strMultiPurpose, m_strMultiResource;
	BOOL ValidateData();
	ApptImport *m_pAppt;
	void FillDWordArray(CDWordArray &aryToFill, CDWordArray &aryFillFrom);
	void RequeryApptPurposes();

// Dialog Data
	//{{AFX_DATA(CNexWebApptInfoDlg)
	enum { IDD = IDD_NEXWEB_APPT };
	CNxLabel	m_nxtPurposeLabel;
	CNxLabel	m_nxtResourceLabel;
	CComboBox	m_Confirmed;
	CDateTimePicker	m_ApptDate;
	CNxEdit	m_nxeditNexwebNotesBox;
	CNxStatic	m_nxstaticNexwebCancelledStatus;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexWebApptInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNexWebApptInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedNexwebAptlocationCombo(short nFlags);
	afx_msg void OnRequeryFinishedNexwebAptpurposeCombo(short nFlags);
	afx_msg void OnSelChosenNexwebAptpurposeCombo(long nRow);
	afx_msg void OnSelChosenNexwebAptresourceCombo(long nRow);
	afx_msg void OnRequeryFinishedNexwebAptresourceCombo(short nFlags);
	afx_msg void OnRequeryFinishedNexwebApttypeCombo(short nFlags);
	afx_msg void OnImport();
	afx_msg void OnDelete();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSelChosenNexwebApttypeCombo(long nRow);
	afx_msg void OnNexwebCancelledStatus();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnSelChosenNexwebMoveUpList(LPDISPATCH lpRow);
	afx_msg void OnSelChangingNexwebMoveUpList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXWEBAPPTINFODLG_H__790D26C1_CEF5_4903_9CC3_1CCA5160E1D6__INCLUDED_)
