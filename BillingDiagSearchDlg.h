#pragma once

// (j.armen 2014-08-06 10:06) - PLID 63161 - Created

#include "DiagCodeInfoFwd.h"

typedef boost::function<void(long, long, CString, CString, CString, CString)> AddDiagCode;
typedef boost::function<void()> DiagOrderModified;
typedef boost::function<void()> RemoveDiagCode;
typedef boost::function<void(std::pair<long, long>)> SetICD9;
typedef boost::function<void(std::pair<long, long>)> SetICD10;
typedef boost::function<long(long, long)> DetectDuplicateDiagnosisCode;
typedef boost::function<void()> EndOfTabSequence;
typedef boost::function<void()> BeginningOfTabSequence;
typedef boost::function<bool(std::pair<long, long>, long)> VerifyChargeCodeOrder;
// (j.jones 2014-12-22 10:48) - PLID 64490 - added ability to replace an existing code
typedef boost::function<void(DiagCodeInfoPtr, DiagCodeInfoPtr)> ReplaceDiagCode;

class CBillingDiagSearchDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CBillingDiagSearchDlg)

public:
	// (j.jones 2014-12-22 10:48) - PLID 64490 - added ability to replace an existing code
	CBillingDiagSearchDlg(long nEntryType, AddDiagCode fnAddDiagCode, DiagOrderModified fnDiagOrderModified,
		RemoveDiagCode fnRemoveDiagCode, ReplaceDiagCode fnReplaceDiagCode,
		SetICD9 fnSetICD9, SetICD10 fnSetICD10, DetectDuplicateDiagnosisCode fnDetectDuplicateDiagnosisCode,
		EndOfTabSequence fnEndOfTabSequence, BeginningOfTabSequence fnBeginningOfTabSequence,
		VerifyChargeCodeOrder fnVerifyChargeCodeOrder);
	~CBillingDiagSearchDlg();

	BOOL Create(CWnd* pParent);

	void SetDiagCodes(const std::vector<shared_ptr<struct DiagCodeInfo>>& aryDiagCodes) const;
	const std::vector<shared_ptr<struct DiagCodeInfo>> GetDiagCodes() const;

	void SetShiftTabFocus();	// (j.armen 2014-08-14 14:20) - PLID 63334

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pDiagSearch;
	NXDATALIST2Lib::_DNxDataListPtr m_pDiagList;
	AddDiagCode m_fnAddDiagCode;
	DiagOrderModified m_fnDiagOrderModified;
	RemoveDiagCode m_fnRemoveDiagCode;
	// (j.jones 2014-12-22 10:48) - PLID 64490 - added ability to replace an existing code
	ReplaceDiagCode m_fnReplaceDiagCode;
	SetICD9 m_fnSetICD9;
	SetICD10 m_fnSetICD10;
	DetectDuplicateDiagnosisCode m_fnDetectDuplicateDiagnosisCode;
	EndOfTabSequence m_fnEndOfTabSequence;
	BeginningOfTabSequence m_fnBeginningOfTabSequence;
	VerifyChargeCodeOrder m_fnVerifyChargeCodeOrder;
	long m_nEntryType;

	bool m_bCanAddToQuickList = false;

	void CheckArrowEnabledState() const;
	void SwapRow(NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::IRowSettingsPtr pSwapRow, bool bSwapForRemoval = false);

	virtual void DoDataExchange(CDataExchange* pDX) override;
	virtual BOOL OnInitDialog() override;
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;
	virtual int SetControlPositions() override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnBnClickedUp();
	afx_msg void OnBnClickedDown();
	afx_msg void OnBnClickedDiagQuickList();
	afx_msg void OnBnClickedNexCode();
	afx_msg void OnRemoveDiagCode();
	afx_msg void OnAddToDiagnosisQuicklistUI(CCmdUI* pCmdUI);
	afx_msg void OnAddToDiagnosisQuicklist();
	// (j.jones 2014-12-22 10:48) - PLID 64490 - added ability to replace an existing code
	afx_msg void OnReplaceDiagCode();

	DECLARE_EVENTSINK_MAP()
	void SelChosenDiagSearch(LPDISPATCH lpRow);
	void SelChangingDiagList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangedDiagList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void LeftClickDiagList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void ShowContextMenuDiagList(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, VARIANT_BOOL* pbContinue);
};