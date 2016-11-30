#pragma once

#include "PatientsRc.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include "NxAPI.h"

// CNexERxQuickListDlg dialog
// (b.savon 2013-01-24 14:43) - PLID 54831 - Created

struct QuickListRx{
	// (r.gonet 2016-02-10 13:34) - PLID 58689 - ERxQuickListT.ID. -1 if unsaved.
	long nID;
	// (r.gonet 2016-02-10 13:34) - PLID 58689 - ERxQuickListT.OrderIndex. The display order of the
	// medication in the user's quicklist. 1 based.
	long nOrderIndex;
	long nDrugListID;
	long nRefill;
	long nDosUnitID;
	long nDosRouteID;
	CString strQuantity;
	CString strSig;
	CString strDosFreq;
	CString strDosQuantity;
	CString strNotes;
	CString strName;

	//Need to initialize this
	QuickListRx()
	{
		// (r.gonet 2016-02-10 13:34) - PLID 58689 - Init to unsaved.
		nID = -1;
		// (r.gonet 2016-02-10 13:34) - PLID 58689 - Init to a bad value.
		nOrderIndex = -1;
		nDrugListID = 0;
		nRefill = 0;
		nDosUnitID = 0;
		nDosRouteID = 0;
		strQuantity = "";
		strSig = "";
		strDosFreq = "";
		strDosQuantity = "";
		strNotes = "";
		strName = "";
	}
};

class CNexERxQuickListDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNexERxQuickListDlg)
private:
	CNxIconButton m_btnClose;
	CNxIconButton m_btnImport;
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnEdit;
	CNxIconButton m_btnDelete;
	// (r.gonet 2016-02-10 13:34) - PLID 58689 - Buttons for moving quicklist medications up and down in position.
	CNxIconButton m_btnMoveUp;
	CNxIconButton m_btnMoveDown;
	CNxColor m_nxcBack;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlUserList;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlUserMedQuickList;

public:
	CNexERxQuickListDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNexERxQuickListDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_NEXERX_QUICK_LIST_DLG };

protected:
	void LoadUserList();
	void LoadUserQuickList(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void PopulateUserArrays(CArray<long, long> &aryIDs, CStringArray &aryNames);
	CString GetCurrentSelectedUserID();

	// (r.gonet 2016-02-10 13:34) - PLID 58689
	NXDATALIST2Lib::IRowSettingsPtr InsertItemIntoQuickListDatalist(NexTech_Accessor::_ERxQuickListMedicationPtr pMedication);
	void InsertItemIntoQuickListDatalist(Nx::SafeArray<IUnknown *> saryMedications);

	// (r.gonet 2016-02-10 13:34) - PLID 58689 - Struct allowing a multi return value
	// from AddQuickListRxs and UpdateQuickListRxs.
	struct SaveQuickListRxResult {
		// (r.gonet 2016-02-10 13:34) - PLID 58689 - Various errors encountered during saving.
		enum class EErrorCode {
			None,
			InvalidUser,
			SaveFailed,
		};

		// (r.gonet 2016-02-10 13:34) - PLID 58689 - Whether the add/update save was successful.
		bool bSuccess = false;
		// (r.gonet 2016-02-10 13:34) - PLID 58689 - The error code if the save was unsuccessful.
		EErrorCode eErrorCode = EErrorCode::None;
		// (r.gonet 2016-02-10 13:34) - PLID 58689 - The saved quicklist medications 
		std::vector<NexTech_Accessor::_ERxQuickListMedicationPtr> vecQuickListMedication;
	};

	// (r.gonet 2016-02-10 13:34) - PLID 58689
	void SwapRowPositions(NXDATALIST2Lib::IRowSettingsPtr pRow1, NXDATALIST2Lib::IRowSettingsPtr pRow2);
	// (r.gonet 2016-02-10 13:34) - PLID 58689
	void SaveRows(std::vector<NXDATALIST2Lib::IRowSettingsPtr> vecRows);
	// (r.gonet 2016-02-10 13:34) - PLID 58689
	QuickListRx GetQuickListRxFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (r.gonet 2016-02-10 13:34) - PLID 58689
	NexTech_Accessor::_ERxQuickListMedicationPtr GetERxQuickListMedicationFromQuickListRx(QuickListRx quickListRx);
	// (r.gonet 2016-02-10 13:34) - PLID 58689
	SaveQuickListRxResult AddQuickListRxs(std::vector<QuickListRx> vecQuickListRxs);
	// (r.gonet 2016-02-10 13:34) - PLID 58689
	SaveQuickListRxResult UpdateQuickListRxs(std::vector<QuickListRx> vecQuickListRxs);
	// (r.gonet 2016-02-10 13:34) - PLID 58689
	void EnsureControls();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
	void SelChosenNxdlUserList(LPDISPATCH lpRow);
	void SelChangingNxdlUserList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnImportQuickList();
	afx_msg void OnBnClickedBtnDelete();
	afx_msg void OnBnClickedBtnQuickListUpdate();
	void RequeryFinishedNxdlUserList(short nFlags);
	// (r.gonet 2016-02-10 13:34) - PLID 58689
	afx_msg void OnBnClickedBtnMoveUp();
	// (r.gonet 2016-02-10 13:34) - PLID 58689
	afx_msg void OnBnClickedBtnMoveDown();
	// (r.gonet 2016-02-10 13:34) - PLID 58689
	void CurSelWasSetNxdlUserMedQuickList();
};
