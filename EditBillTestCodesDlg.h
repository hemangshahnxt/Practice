#pragma once

// (r.gonet 08/05/2014) - PLID 63099 - Initial header file

#include "BillingRc.h"

// CEditBillTestCodesDlg dialog

class CEditBillTestCodesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEditBillTestCodesDlg)
private:
	// (r.gonet 08/05/2014) - PLID 63099 - Enumeration for the test code list columns.
	enum class ETestCodeListColumns {
		Selected = 0, // (r.gonet 08/05/2014) - PLID 63102 - Whether the row's selection checkbox is checked or not.
		State, // (r.gonet 08/05/2014) - PLID 63099 - If the row is unchanged, new, or modified. Deleted rows get placed elsewhere.
		ID,
		Code,
		Description,
		Inactive, // (r.gonet 08/05/2014) - PLID 63100 - Added Inactive.
	};

	// (r.gonet 08/05/2014) - PLID 63099 - Enumeration for the test code list row states.
	enum class ETestCodeListRowState {
		Unchanged = 0, // (r.gonet 08/05/2014) - PLID 63099 - Existing row. No changes to it so far.
		New, // (r.gonet 08/05/2014) - PLID 63099 - New row. Need to INSERT it during saving.
		Modified // (r.gonet 08/05/2014) - PLID 63099 - Changed existing row. Need to UPDATE it during saving.
	};

	// (r.gonet 08/05/2014) - PLID 63099 - Background color for the dialog.
	CNxColor m_nxcolorBackground;
	// (r.gonet 08/05/2014) - PLID 63099 - The list of test codes.
	NXDATALIST2Lib::_DNxDataListPtr m_pTestCodeList;
	// (r.gonet 08/05/2014) - PLID 63099 - Add button to add a new code to the test code list.
	CNxIconButton m_btnAdd;
	// (r.gonet 08/05/2014) - PLID 63099 - Delete button to delete a code from the test code list.
	CNxIconButton m_btnDelete;
	// (r.gonet 08/05/2014) - PLID 63100 - Checkbox to hide inactive test code rows. Remembered per user.
	NxButton m_checkHideInactive;
	// (r.gonet 08/05/2014) - PLID 63099 - OK button that saves any changes.
	CNxIconButton m_btnOK;
	// (r.gonet 08/05/2014) - PLID 63099 - Cancel button that abandons changes.
	CNxIconButton m_btnCancel;

	// (r.gonet 08/05/2014) - PLID 63102 - List of test code IDs to pre-select when the dialog loads.
	std::vector<long> m_vecPreSelectedTestCodeIDs;
	// (r.gonet 08/05/2014) - PLID 63102 - List of test code IDs that the user selected before the dialog closed.
	std::vector<long> m_vecSelectedTestCodeIDs;
	// (r.gonet 08/05/2014) - PLID 63099 - Set of unique test code IDs that the user deleted and that need included in the DELETE statement during saving.
	std::set<long> m_setDeletedTestCodeIDs;

public:
	// (r.gonet 08/05/2014) - PLID 63099 - Constructs a new CEditBillTestCodesDlg.
	// (r.gonet 08/06/2014) - PLID 63102 - Added vecPreSelectedTestCodeIDs
	// - pParent: Parent window.
	// - vecPreSelectedTestCodeIDs: IDs of Test codes to select automatically when the window is shown.
	CEditBillTestCodesDlg(CWnd* pParent, std::vector<long> &vecPreSelectedTestCodeIDs);   // standard constructor
	virtual ~CEditBillTestCodesDlg();

	// (r.gonet 08/05/2014) - PLID 63102 - Fills a vector with the selected test code IDs
	// - vecSelectedTestCodeIDs: vector to hold the selected test code IDs.
	void GetSelectedTestCodeIDs(OUT std::vector<long> &vecSelectedTestCodeIDs);

// Dialog Data
	enum { IDD = IDD_EDIT_BILL_LAB_TEST_CODES_DLG };

private:
	// (r.gonet 08/05/2014) - PLID 63099 - Ensures the controls are in valid states.
	void EnsureControls();
	// (r.gonet 08/05/2014) - PLID 63100 - Shows or hides the inactive test code rows. Does not hide selected inactive rows.
	// - bShow: true to show the inactive test codes. false to hide the inactive test codes.
	void ShowInactiveTestCodes(bool bShow);
	// (r.gonet 08/05/2014) - PLID 63099 - Returns true if the test code is in use by some non-deleted charge. Returns false if it is not.
	bool IsInUse(long nTestCodeID);
	// (r.gonet 08/05/2014) - PLID 63100 - Sets a database row to inactive, grays it, and hides it if necessary.
	// Undoes this if setting to active.
	// - pRow: Test code list row to inactivate/activate.
	// - bInactive: true to inactivate the row. false to activate it.
	void SetInactive(NXDATALIST2Lib::IRowSettingsPtr pRow, bool bInactive);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	// (r.gonet 08/05/2014) - PLID 63100 - Handles the event when the user checks the hide inactive checkbox.
	afx_msg void OnBnClickedEditTestCodesHideInactiveCheck();
	// (r.gonet 08/05/2014) - PLID 63099 - Handles the event when the user clicks the OK button. Saves the test code
	// changes to the database.
	afx_msg void OnBnClickedOk();
	// (r.gonet 08/05/2014) - PLID 63099 - Handle the event when the user clicks the delete button. Delete a test code.
	afx_msg void OnBnClickedEditTestCodesDeleteBtn();
	// (r.gonet 08/05/2014) - PLID 63099 - Handles the event when the user clicks the Add button. Adds a new test code row.
	afx_msg void OnBnClickedEditTestCodesAddBtn();
	DECLARE_EVENTSINK_MAP()
	// (r.gonet 08/05/2014) - PLID 63099 - Handles the event when the user selects a different row from the test code list.
	void SelChangedEditTestCodesList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	// (r.gonet 08/05/2014) - PLID 63099 - Handles the event where the user has just finished editing a test code list cell.
	void EditingFinishedEditTestCodesList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	// (r.gonet 08/05/2014) - PLID 63099 - Handles the event when the user is finishing editing a cell in the test code list.
	void EditingFinishingEditTestCodesList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};
