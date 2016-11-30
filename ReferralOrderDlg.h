#pragma once


// CReferralOrderDlg dialog
// (z.manning 2009-05-05 09:44) - PLID 34172 - Created

class CReferralOrderDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CReferralOrderDlg)

public:
	CReferralOrderDlg(const long nPatientID, CWnd* pParent);   // standard constructor
	virtual ~CReferralOrderDlg();

// Dialog Data
	enum { IDD = IDD_REFERRAL_ORDERS };

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pdlOrders;
	enum EReferralOrderColumns {
		rocID = 0,
		rocDate,
		rocReferredByID,
		rocReferredByName,
		rocReferToID,
		rocReferToName,
		rocSpecialty,
		rocAddress,
		rocCity,
		rocState,
		rocZip,
		rocWorkPhone,
		rocReason,
		rocInputDate,
		rocInputUsername,
	};

	long m_nPatientID;

	CNxIconButton m_btnNewReferralOrder;
	CNxIconButton m_btnEditReferralOrder;
	CNxIconButton m_btnDeleteReferralOrder;
	CNxIconButton m_btnClose;
	CNxColor m_nxcolor;
	CNxStatic m_nxstaticHeader;
	CNxIconButton m_btnPrint;
	CNxIconButton m_btnPreview;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedNewReferralOrder();
	afx_msg void OnBnClickedEditReferralOrder();
	afx_msg void OnBnClickedDeleteReferralOrder();
	DECLARE_EVENTSINK_MAP()
	void DblClickCellReferralOrderList(LPDISPATCH lpRow, short nColIndex);
	void SelChangedReferralOrderList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void RequeryFinishedReferralOrderList(short nFlags);
	// (z.manning 2009-05-13 09:51) - PLID 34219 - Added print buttons
	afx_msg void OnBnClickedPreviewRefOrder();
	afx_msg void OnBnClickedPrintRefOrder();
};
