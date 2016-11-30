#pragma once

#include "AdministratorRc.h"

// CBillStatusSetupDlg dialog

// (r.gonet 07/03/2014) - PLID 62533 - Added.
/// <summary>
/// Dialog that lets the user add, edit, and delete custom bill statuses.
/// </summary>
class CBillStatusSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CBillStatusSetupDlg)

private:
	/// <summary>
	/// Column enumeration for the m_pBillStatusList datalist.
	/// </summary>
	enum class EBillStatusListColumns {
		ID = 0,
		Name,
		OnHold,
		Inactive,
		Custom,
	};

	/// <summary>
	/// The NxColor control for the background of the dialog.
	/// </summary>
	CNxColor m_nxcolorBackground;
	/// <summary>
	/// The small static label above the statuses datalist.
	/// </summary>
	CNxStatic m_nxstaticHeader;
	/// <summary>
	/// The bill status datalist.
	/// </summary>
	NXDATALIST2Lib::_DNxDataListPtr m_pBillStatusList;
	/// <summary>
	/// The Add button.
	/// </summary>
	CNxIconButton m_btnAdd;
	/// <summary>
	/// The Delete button.
	/// </summary>
	CNxIconButton m_btnDelete;
	/// <summary>
	/// The Close button.
	/// </summary>
	CNxIconButton m_btnClose;

	/// <summary>
	/// The RGB background color to apply to the m_nxcolorBackground control.
	/// </summary>
	DWORD m_dwBackgroundColor;

public:
	// (r.gonet 07/03/2014) - PLID 62533 - Added.
	/// <summary>
	/// Initializes a new instance of the CBillStatusSetupDlg class.
	/// </summary>
	/// <param name="pParent">The parent window.</param>
	/// <param name="dwBackgroundColor">Color of the background nxcolor.</param>
	CBillStatusSetupDlg(CWnd* pParent, DWORD dwBackgroundColor);   // standard constructor
	// (r.gonet 07/03/2014) - PLID 62533 - Added.
	/// <summary>
	/// Finalizes an instance of the CBillStatusSetupDlg class.
	/// </summary>
	virtual ~CBillStatusSetupDlg();

// Dialog Data
	enum { IDD = IDD_BILL_STATUS_SETUP_DLG };


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// (r.gonet 07/03/2014) - PLID 62533 - Added.
	/// <summary>
	/// Dialog initialization.
	/// </summary>
	virtual BOOL OnInitDialog();
	// (r.gonet 07/03/2014) - PLID 62533 - Added.
	/// <summary>
	/// Called when the Add button is clicked. Adds a new bill status row and starts the user typing the name of the status out.
	/// </summary>
	afx_msg void OnBnClickedAddBtn();
	// (r.gonet 07/03/2014) - PLID 62533 - Added.
	/// <summary>
	/// Called when the Delete button is clicked. Tries to remove the current row and delete it from the database, if it is not in use.
	/// </summary>
	afx_msg void OnBnClickedDeleteBtn();
	DECLARE_EVENTSINK_MAP()
	// (r.gonet 07/03/2014) - PLID 62533 - Added.
	/// <summary>
	/// Called when the status list has finished requerying. Makes sure that the non-custom statuses are not editable.
	/// </summary>
	/// <param name="nFlags">The reason why the requerying finished.</param>
	void RequeryFinishedBscStatusList(short nFlags);
	// (r.gonet 07/03/2014) - PLID 62533 - Added.
	/// <summary>
	/// Called when the user is just finishing editing a cell in the Status List but before their edits are committed. Ensures they are properly warned.
	/// </summary>
	/// <param name="lpRow">The row of the cell being edited.</param>
	/// <param name="nCol">The column index of the cell being edited.</param>
	/// <param name="varOldValue">The old value of the cell before editing.</param>
	/// <param name="strUserEntered">The string the user typed.</param>
	/// <param name="pvarNewValue">A pointer to the new value of the cell.</param>
	/// <param name="pbCommit">A pointer to a boolean flag to commit this new value.</param>
	/// <param name="pbContinue">A pointer to a boolean flag to continue committing or force the user to keep editing.</param>
	void EditingFinishingBssStatusList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	// (r.gonet 07/03/2014) - PLID 62533 - Added.
	/// <summary>
	/// Called after the client has finished editing a datalist cell. Ensures the edit is committed to the database.
	/// </summary>
	/// <param name="lpRow">The row of the cell being edited.</param>
	/// <param name="nCol">The column index of the cell being edited.</param>
	/// <param name="varOldValue">The old value before editing.</param>
	/// <param name="varNewValue">The new value after editing.</param>
	/// <param name="bCommit">Boolean flag telling if the new value has been committed to the datalist or not.</param>
	void EditingFinishedBssStatusList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	// (r.gonet 07/03/2014) - PLID 62533 - Added.
	/// <summary>
	/// Called when the user has selected a row from the datalist.
	/// </summary>
	/// <param name="lpOldSel">The old selected row.</param>
	/// <param name="lpNewSel">The new selected row.</param>
	void SelChangedBssStatusList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
private:
	// (r.gonet 07/03/2014) - PLID 62533 - Added.
	/// <summary>
	/// Determines whether the status datalist row is a custom status.
	/// </summary>
	/// <param name="pRow">The row to check.</param>
	/// <returns>true if the row is a custom status. false if it is a non-custom status.</returns>
	bool IsCustomStatus(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (r.gonet 07/03/2014) - PLID 62533 - Added.
	/// <summary>
	/// Determines whether a status is in use by some non-deleted bill. Deleted bills with the status don't count.
	/// </summary>
	/// <param name="nID">The ID of the status to check.</param>
	/// <returns>true if the status is in use by a non-deleted bill. false if it isn't.</returns>
	bool IsInUse(long nID);
	// (r.gonet 07/03/2014) - PLID 62533 - Added.
	/// <summary>
	/// Ensures the controls are in valid states given the current context.
	/// </summary>
	void EnsureControls();
};
