#if !defined(AFX_EMRCOLLECTIONSETUPDLG_H__0D577549_3F2C_43F7_91AB_B1F4192F6878__INCLUDED_)
#define AFX_EMRCOLLECTIONSETUPDLG_H__0D577549_3F2C_43F7_91AB_B1F4192F6878__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrCollectionSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEmrCollectionSetupDlg dialog

class CEmrCollectionSetupDlg : public CNxDialog
{
// Construction
public:
	CEmrCollectionSetupDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEmrCollectionSetupDlg)
	enum { IDD = IDD_EMR_COLLECTION_SETUP_DLG };
	CNxIconButton	m_btnDataUp;
	CNxIconButton	m_btnDataDown;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnRename;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrCollectionSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pdlEmrCollectionList;

protected:
	void Load();
	BOOL ValidateAndSave();
	
	CDWordArray m_arydwDeletedCollectionIDs;

protected:
	void RenumberCollectionMenuOrders();
	void ReflectEnabledState();
	bool IsCollectionNameValid(CString strCollectionNameToValidate, CString strOldCollectionName = "");

protected:
	// Generated message map functions
	//{{AFX_MSG(CEmrCollectionSetupDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedEmrcollectionList(short nFlags);
	afx_msg void OnDataUpBtn();
	afx_msg void OnDataDownBtn();
	afx_msg void OnEditingFinishedEmrcollectionList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRButtonDownEmrcollectionList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelChangedEmrcollectionList(long nNewSel);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnAddBtn();
	afx_msg void OnRenameBtn();
	afx_msg void OnDeleteBtn();
	afx_msg void OnEditingFinishingEmrcollectionList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRCOLLECTIONSETUPDLG_H__0D577549_3F2C_43F7_91AB_B1F4192F6878__INCLUDED_)
