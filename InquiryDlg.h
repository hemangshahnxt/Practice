#if !defined(AFX_INQUIRYDLG_H__A234CB01_FBC3_48BC_BAF1_56EF5E42AB7D__INCLUDED_)
#define AFX_INQUIRYDLG_H__A234CB01_FBC3_48BC_BAF1_56EF5E42AB7D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InquiryDlg.h : header file
//
#include "ReferralSubDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CInquiryDlg dialog


class CInquiryDlg : public CNxDialog
{
// Construction
public:
	CInquiryDlg(CWnd* pParent); // standard constructor
	// (j.gruber 2008-09-08 13:52) - PLID 30899 - allow fields to be added
	CInquiryDlg(CString strFirst, CString strMiddle, CString strLast, CString strEmail, long nReferralID, CArray<int,int> *paryProcs, CArray<int,int> *paryProcGroups, BOOL bLoadProcedures, CString strNote, CWnd* pParent); // standard constructor
	void UpdateArray(long nPersonID);
	CArray<int, int> m_arProcIDs;
	CArray<int, int> m_arProcGroupIDs;
	CArray<CString, CString> m_arProcString;
	CArray<CString, CString> m_arProcGroupString;	
	bool m_bProcedures; //Did they select procedures? (as opposed to procedure groups)
	bool m_bAllowProcGroups; //Will we allow them to select procedure groups? 
	CString m_strProcWhereClause; //If you pass in a where clause, it will override any other where clause.
	CString m_strGroupWhereClause; //If you pass in a where clause, it will override any other where clause.
	void LoadFromArray();
	BOOL IsProcedureChosen();
	//r.wilson PLID 39357 10/28/2011
	BOOL IsIdInvalid(int id, CString& strErrors );


// Dialog Data
	//{{AFX_DATA(CInquiryDlg)
	enum { IDD = IDD_INQUIRY_DLG };
	NxButton	m_btShowProcedures;
	NxButton	m_btnShowProcGroups;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnSave;
	CNxEdit	m_nxeditIdBox;
	CNxEdit	m_nxeditNotes;
	CNxEdit	m_nxeditFirstNameBox;
	CNxEdit	m_nxeditMiddleNameBox;
	CNxEdit	m_nxeditLastNameBox;
	CNxEdit	m_nxeditEmail;
	CNxStatic	m_nxstaticReferralDlg;
	//}}AFX_DATA
	//r.wilson PLID 39357 10/28/2011
	//BOOL bUserCanWrite;
	//BOOL bUserCanWriteOnlyWithPass;
	long m_nOriginalUserDefinedID;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInquiryDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_procedureList;
	NXDATALISTLib::_DNxDataListPtr m_pProcGroupList;
	NXDATALISTLib::_DNxDataListPtr m_pLocation;
	
	CReferralSubDlg* m_pReferralSubDlg;
	
	// (j.gruber 2008-09-08 15:29) - PLID 30899 - load from newpatientdlg
	CString m_strLoadFirst;
	CString m_strLoadMiddle;
	CString m_strLoadLast;
	CString m_strLoadEmail;
	CString m_strLoadNote;
	long m_nLoadReferralID;
	BOOL m_bLoadProcedures;

	// Generated message map functions
	//{{AFX_MSG(CInquiryDlg)
	afx_msg void OnSaveInquiry();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnShowProcedures();
	afx_msg void OnShowProcedureGroups();
	afx_msg void OnLButtonDownProcedureList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnKillFocusID();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INQUIRYDLG_H__A234CB01_FBC3_48BC_BAF1_56EF5E42AB7D__INCLUDED_)
