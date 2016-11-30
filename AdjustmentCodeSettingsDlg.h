#pragma once

// CAdjustmentCodeSettingsDlg dialog

// (j.jones 2008-11-24 10:09) - PLID 32075 - created

#include "FinancialRc.h"

// (r.gonet 2016-04-18) - NX-100162 - Forward declaration to avoid an include.
struct CAuditTransaction;

class CAdjustmentCodeSettingsDlg : public CNxDialog
{

public:
	CAdjustmentCodeSettingsDlg(CWnd* pParent);   // standard constructor

	BOOL m_bInfoChanged;	//tracks if the setup changed

// Dialog Data
	// (r.gonet 2016-04-18) - NX-100162 - Renamed the identifier.
	enum { IDD = IDD_ADJUSTMENT_CODE_SETTINGS_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	// (r.gonet 2016-04-18) - NX-100162 - Renamed to clarify that these refer to the ignored codes list.
	CNxIconButton m_btnAddIgnoredAdjCode;
	CNxIconButton m_btnRemoveIgnoredAdjCode;
	CNxStatic	m_nxstaticZeroAdjLabel;
	// (j.jones 2011-04-04 15:34) - PLID 42571 - added ability to ignore all secondary adjustments
	NxButton	m_checkIgnoreSecondaryAdjs;

	// (r.gonet 2016-04-18) - NX-100162 - Added buttons for the allow negative adjustment codes list.
	NxButton m_checkAllowAllNegativeAdjustmentsToBePosted;
	CNxIconButton m_btnAddAllowNegativeAdjCode;
	CNxIconButton m_btnRemoveAllowNegativeAdjCode;

protected:
	// (r.gonet 2016-04-18) - NX-100162 - Encapsulates the changes for the ignored codes list.
	struct IgnoredAdjCodeModifications {
		std::set<long> setDeleted;
		std::set<long> setChanged;

		// (j.jones 2011-04-04 15:34) - PLID 42571 - cache the secondary adjustment ignore setting
		BOOL bOldIgnoreSecondaryAdjs = FALSE;
	} m_ignoredAdjCodeModifications;

	// (r.gonet 2016-04-18) - NX-100162 - Encapsulates the changes for the allow negative adjustment codes list.
	struct AllowNegativePostingAdjCodeModifications {
		std::set<long> setDeleted;
		std::set<long> setChanged;

		BOOL bOldAllowAllNegativeAdjustmentsToBePosted = FALSE;
	} m_allowNegativePostingAdjCodeModifications;

	// (r.gonet 2016-04-18) - NX-100162 - Renamed the m_List to m_pIgnoredAdjCodesList to reflect the fact that its not
	// the only list in this dialog now.
	NXDATALIST2Lib::_DNxDataListPtr m_pIgnoredAdjCodesList;
	NXDATALIST2Lib::_DNxDataListPtr m_pAllowNegativeAdjCodesList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()

	// (j.jones 2010-09-23 14:31) - PLID 40653 - cache the ID for the PR group code
	long m_nPRGroupCodeID;

	// (r.gonet 2016-04-18) - NX-100162
	bool ValidateIgnoredAdjCodes();
	// (r.gonet 2016-04-18) - NX-100162
	void DeleteRemovedIgnoredAdjCodes(CParamSqlBatch& sqlSaveBatch, CAuditTransaction& auditTransaction);
	// (r.gonet 2016-04-18) - NX-100162
	void UpdateChangedIgnoredAdjCodes(CParamSqlBatch& sqlSaveBatch, CAuditTransaction& auditTransaction);
	// (r.gonet 2016-04-18) - NX-100162
	void CreateNewIgnoredAdjCodes(CParamSqlBatch& sqlSaveBatch, CAuditTransaction& auditTransaction);
	
	// (r.gonet 2016-04-18) - NX-100162
	bool ValidateAllowNegativePostingAdjCodes();
	// (r.gonet 2016-04-18) - NX-100162
	void DeleteRemovedAllowNegativePostingAdjCodes(CParamSqlBatch& sqlSaveBatch, CAuditTransaction& auditTransaction);
	// (r.gonet 2016-04-18) - NX-100162
	void UpdateChangedAllowNegativePostingAdjCodes(CParamSqlBatch& sqlSaveBatch, CAuditTransaction& auditTransaction);
	// (r.gonet 2016-04-18) - NX-100162
	void CreateNewAllowNegativePostingAdjCodes(CParamSqlBatch& sqlSaveBatch, CAuditTransaction& auditTransaction);

	// (r.gonet 2016-04-18) - NX-100162
	void EnsureControls();

	virtual BOOL OnInitDialog();
	afx_msg void OnOk();
	
	// (r.gonet 2016-04-18) - NX-100162 - Renamed.
	afx_msg void OnAddIgnoredAdjCode();
	// (r.gonet 2016-04-18) - NX-100162 - Renamed.
	afx_msg void OnRemoveIgnoredAdjCode();	
	// (r.gonet 2016-04-18) - NX-100162 - Renamed.
	afx_msg	void OnEditingFinishedIgnoredAdjCodesList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	// (r.gonet 2016-04-18) - NX-100162
	afx_msg void OnBnClickedAllowAllNegativeAdjustmentsCheck();
	// (r.gonet 2016-04-18) - NX-100162
	afx_msg void OnBnClickedAddAllowNegativeAdjCode();
	// (r.gonet 2016-04-18) - NX-100162
	afx_msg void OnBnClickedRemoveAllowNegativeAdjCode();
	// (r.gonet 2016-04-18) - NX-100162
	afx_msg void EditingFinishedAllowNegativeAdjCodesList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	// (r.gonet 2016-04-18) - NX-100162
	afx_msg void CurSelWasSetAllowNegativeAdjCodesList();
	// (r.gonet 2016-04-18) - NX-100162
	afx_msg void CurSelWasSetEremitCodesToSkipList();
};
