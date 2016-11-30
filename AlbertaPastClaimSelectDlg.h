#pragma once

// (j.jones 2011-07-21 15:55) - PLID 44662 - created

// CAlbertaPastClaimSelectDlg dialog

#include "FinancialRc.h"

struct ClaimHistoryInfo {

	long nID;
	COleDateTime dtDate;
	CString strClaimNumber;	// (j.dinatale 2013-01-18 16:57) - PLID 54419 - get the transaction number
};

class CAlbertaPastClaimSelectDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAlbertaPastClaimSelectDlg)

public:
	CAlbertaPastClaimSelectDlg(CWnd* pParent);   // standard constructor
	virtual ~CAlbertaPastClaimSelectDlg();

	virtual int DoModal(long nCurrentClaimHistoryDetailID, CString strActionType, long nUserDefinedID, CString strPatientName,
						long nBillID, COleDateTime dtBillDate, CString strBillDescription,
						CString strSubmitterPrefix, CString strSourceCode);

	CArray<ClaimHistoryInfo, ClaimHistoryInfo> m_aryClaimHistoryInfo;
	long m_nSelectedClaimHistoryDetailID;

	CString m_strSelectedClaimNumber;	// (j.dinatale 2013-01-18 16:57) - PLID 54419 - get the transaction number

// Dialog Data
	enum { IDD = IDD_ALBERTA_PAST_CLAIM_SELECT_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxStatic		m_nxstaticLabel1;
	CNxStatic		m_nxstaticLabel2;
	CNxStatic		m_nxstaticLabel3;
	CNxStatic		m_nxstaticLabel4;

protected:

	long m_nCurrentClaimHistoryDetailID;
	CString m_strActionType;
	long m_nUserDefinedID;
	CString m_strPatientName;
	long m_nBillID;
	COleDateTime m_dtBillDate;
	CString m_strBillDescription;
	CString m_strSubmitterPrefix;
	CString m_strSourceCode;

	NXDATALIST2Lib::_DNxDataListPtr m_List;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnOk();
	afx_msg void OnCancel();
	DECLARE_EVENTSINK_MAP()
	void OnDblClickCellClaimHistoryList(LPDISPATCH lpRow, short nColIndex);
};
