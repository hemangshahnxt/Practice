#pragma once


// CEmrCodingGroupEditDlg dialog
// (z.manning 2011-07-05 14:50) - PLID 44421 - Created


class CEmrCodingGroup;
class CEmrCodingGroupDetail;

class CEmrCodingGroupEditDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrCodingGroupEditDlg)

public:
	CEmrCodingGroupEditDlg(CWnd* pParent);   // standard constructor
	virtual ~CEmrCodingGroupEditDlg();

// Dialog Data
	enum { IDD = IDD_EMR_CODING_EDITOR };

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pdlGroups;
	enum EGroupListColumns {
		glcPointer = 0,
		glcName,
	};

	NXDATALIST2Lib::_DNxDataListPtr m_pdlGroupDetails;
	enum EGroupListDetailColumns {
		gldcRangePointer = 0,
		gldcDetailPointer,
		gldcQuantity,
		gldcCptQuantity,
		gldcCptCode,
		gldcCptName,
	};

	enum ECptQuantitySentinelValues
	{
		cqsvInvalid = -1,
		cqsvOne = 1,
		cqsvTotal,
		cqsvTotalMinus1,
	};

	ECptQuantitySentinelValues GetQuantitySentinelFromCodingDetail(CEmrCodingGroupDetail *pCodingDetail);

	CNxColor m_nxcolor;
	CNxColor m_nxcolor2;
	CNxIconButton m_btnClose;
	CNxIconButton m_btnAddGroup;
	CNxIconButton m_btnRenameGroup;
	CNxIconButton m_btnDeleteGroup;
	CNxIconButton m_btnAddRange;
	CNxIconButton m_btnAddCpt;
	CNxIconButton m_btnDeleteSelected;

	void LoadCurrentGroup();
	void AddQuantityRange();
	void AddCptCode();
	void DeleteSelected();
	void UpdateCurrentRowQuantity();

	void HandleCodingGroupChange();

	CEmrCodingGroup* GetCurrentCodingGroupPointer();
	NXDATALIST2Lib::IRowSettingsPtr GetNewDetailRow();
	NXDATALIST2Lib::IRowSettingsPtr GetCurrentRangeRow();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void UpdateView(bool bForceRefresh = true);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBnClickedAddEmrCodingGroup();
	afx_msg void OnBnClickedRenameEmrCodingGroup();
	afx_msg void OnBnClickedDeleteEmrCodingGroup();
	afx_msg void OnBnClickedNewEmrCodingQuantityRange();
	afx_msg void OnBnClickedNewEmrCodingCptCode();
	void SelChosenEmrCodingGroupList(LPDISPATCH lpRow);
	afx_msg void OnBnClickedEmrCodingDeleteSelected();
	void SelChangedEmrCodingDetailList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void EditingStartingEmrCodingDetailList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void EditingFinishedEmrCodingDetailList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};
