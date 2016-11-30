#pragma once


// (z.manning 2009-05-12 12:41) - PLID 34219
void PrintReferralOrder(const long nReferralOrderID, BOOL bPreview);

// CReferralOrderEntryDlg dialog
// (z.manning 2009-05-05 09:48) - PLID 28529 - Created

struct ReferralOrder
{
	long nID;
	long nPatientID;
	COleDateTime dtDate;
	long nReferToID;
	_variant_t varReferredByID;
	CString strReason;

	// (z.manning 2009-05-06 12:45) - PLID 28530 - Needed for auditing
	CString strReferToName;
	CString strReferredByName;

	ReferralOrder()
	{
		nID = -1;
		nPatientID = -1;
		dtDate.SetStatus(COleDateTime::invalid);
		nReferToID = -1;
		varReferredByID.vt = VT_NULL;
	}
};

class CReferralOrderEntryDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CReferralOrderEntryDlg)

public:
	CReferralOrderEntryDlg(const long nPatientID, CWnd* pParent);   // standard constructor
	virtual ~CReferralOrderEntryDlg();

	int EditExistingReferralOrder(const long nReferralOrderID);
	int EditExistingReferralOrder(const ReferralOrder &referralorder);

	// (z.manning 2009-05-08 09:39) - PLID 28554 - Sets the order set ID if this order is part of an order set.
	void SetOrderSetID(const long nOrderSetID);

	// (z.manning 2009-05-13 12:29) - PLID 28554 - Use this to auto-select a ref phys when the dialog loads.
	void SetDefaultRefPhysID(const long nRefPhysID);

	// (z.manning 2009-05-12 10:54) - PLID 34219
	BOOL m_bCloseParent;

	// (z.manning 2009-05-13 15:21) - PLID 28554 - Set this to true to hide the preview button
	BOOL m_bPreventPreview;

// Dialog Data
	enum { IDD = IDD_REFERRAL_ORDER_ENTRY };

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pdlReferTo;
	enum EReferToColumns {
		rtcID = 0,
		rtcName,
		rtcCompany,
		rtcInactive,
		rtcSpecialty,
		rtcAddress1,
		rtcCity,
		rtcState,
		rtcZip,
		rtcWorkPhone,
	};
	NXDATALIST2Lib::_DNxDataListPtr m_pdlReferredBy;
	enum EReferredByColumns {
		rbcID = 0,
		rbcName,
		rbcInactive,
	};

	long m_nPatientID;
	ReferralOrder m_OriginalOrder;

	// (z.manning 2009-05-08 09:39) - PLID 28554 - Used if this referral order is part of an order set.
	_variant_t m_varOrderSetID;

	long m_nDefaultRefPhysID;

	void UpdateRefPhysText();

	BOOL IsNew();

	BOOL Save();
	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	CNxColor m_nxcolor;
	CNxStatic m_nxstaticHeader;
	CNxStatic m_nxstaticRefPhysText;
	CNxEdit m_nxeditReason;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
	CDateTimePicker	m_dtpDate;
	CNxIconButton m_btnPrint;
	CNxIconButton m_btnPreview;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void RequeryFinishedReferToList(short nFlags);
	void RequeryFinishedReferredByList(short nFlags);
	void SelChosenReferToList(LPDISPATCH lpRow);
	// (z.manning 2009-05-12 10:54) - PLID 34219 - Print referral orders
	afx_msg void OnBnClickedPreviewReferralOrder();
	afx_msg void OnBnClickedPrintReferralOrder();
};
