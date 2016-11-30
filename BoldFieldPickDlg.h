#pragma once

#include "BoldLinkDlg.h"
// CBoldFieldPickDlg dialog

// (j.gruber 2010-06-02 16:33) - PLID 38538 - created

class CBoldFieldPickDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CBoldFieldPickDlg)

public:
	CBoldFieldPickDlg(CWnd* pParent);   // standard constructor
	virtual ~CBoldFieldPickDlg();
	CArray<BOLDCodeInfo*, BOLDCodeInfo*> *m_paryBoldList;
	CMap<CString, LPCTSTR, CString, LPCTSTR> *m_pmapReturnValues;
	CString m_strPatientName;
	COleDateTime m_dtProcDate;
	CString m_strVisitType;
	

// Dialog Data
	enum { IDD = IDD_BOLD_FIELD_PICK_DLG };

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	CNxStatic m_stDescription;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void LoadList();
	void LoadValuesList(LPDISPATCH lpRow, long nRowItemID);
	
	
	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedBoldFieldList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};
