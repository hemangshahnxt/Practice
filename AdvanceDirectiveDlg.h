#pragma once

// (a.walling 2009-06-01 09:30) - PLID 34410 - Advance Directives support

// CAdvanceDirectiveDlg dialog

class CAdvanceDirectiveDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAdvanceDirectiveDlg)

public:
	CAdvanceDirectiveDlg(CWnd* pParent);   // standard constructor
	virtual ~CAdvanceDirectiveDlg();

	long m_nPatientID;

// Dialog Data
	enum { IDD = IDD_ADVANCE_DIRECTIVE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void RefreshList() throw(...);

	BOOL SaveChanges(UINT* pnResult = NULL);
	BOOL NeedToSave() throw(...);

	CString GetAuditString(long nID) throw(...);

	CNxEdit m_nxeditDescription;
	CNxIconButton m_nxibCancel;
	CNxIconButton m_nxibAddOtherContact;
	CNxIconButton m_nxibAddNew;
	NxButton m_nxbReviewed;
	CNxColor m_nxcolor;
	CNxColor m_nxcolorList;
	CNxStatic m_nxsLastReviewed;

	CDateTimePicker m_dtpFrom;
	CDateTimePicker m_dtpTo;

	NXDATALIST2Lib::_DNxDataListPtr m_listDirectives;
	enum ListColumns {
		lcID = 0,
		lcTypeID,
		lcDescription,
		lcDateFrom,
		lcDateTo,
		lcLastReview,
		lcLastReviewBy,
		lcCustodianID
	};

	NXDATALIST2Lib::_DNxDataListPtr m_listType;
	enum TypeListColumns {
		tlcID = 0,
		tlcName,
		tlcCode,
	};

	NXDATALIST2Lib::_DNxDataListPtr m_listContacts;
	enum ContactListColumns {
		clcID = 0,
		clcLast,
		clcMiddle,
		clcFirst,
		// nothing else seems necessary atm.
	};

	void Load(NXDATALIST2Lib::IRowSettingsPtr pRow);

	CString GetLastReviewString(const CString& strLastReviewBy, _variant_t varLastReview);


	DECLARE_MESSAGE_MAP()
protected:
	virtual BOOL OnInitDialog();
	afx_msg virtual void OnCancel();
	afx_msg virtual void OnOK();
	afx_msg void OnBnClickedBtnAddOtherContact();
	afx_msg void OnBnClickedCheckAdvanceDirective();
public:
	afx_msg void OnEnChangeEditAdvanceDirective();
	DECLARE_EVENTSINK_MAP()
	void SelChosenListAdvanceDirectiveType(LPDISPATCH lpRow);
	void SelChangedListAdvanceDirectiveType(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void SelChangingListAdvanceDirectiveType(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangedCustodianContactList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void SelChangingCustodianContactList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnDtnDatetimechangeAdDateFrom(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeAdDateTo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnAddNewDirective();
	void SelChangingListAdvanceDirectives(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangedListAdvanceDirectives(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
};