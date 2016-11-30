#pragma once

#include <NxUILib/NxStaticIcon.h>
#include "PatientsRc.h"
#include "NexERxQuickListDlg.h"

// CNexERxAddQuickListDlg dialog
// (b.savon 2013-01-23 10:02) - PLID 54831 - Created

class CNexERxAddQuickListDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNexERxAddQuickListDlg)
private:
	CString m_strDrugListID;
	// (r.gonet 2016-02-10 13:34) - PLID 58689 - Start tracking the name of the drug in order
	// to get it while creating a QuickListRx object after the dialog has closed and the
	// medication search datalist has been destroyed.
	CString m_strDrugListName;
	CString m_strRefills;
	CString m_strQuantity;
	CString m_strSig;
	CString m_strDosageFreq;
	CString m_strDosageQuantity;
	CString m_strNotes;
	// (j.fouts 2013-02-01 15:07) - PLID 54986 - Added route and unit
	CString m_strDosageUnit;
	long m_nDosageUnitID;
	CString m_strDosageRoute;
	long m_nDosageRouteID;

	QuickListRx m_qlRx;
	BOOL m_bEditMode;

	CNxColor m_nxcBack;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med search color
	CNxStaticIcon m_icoAboutQLMedicationColors;

	NXDATALIST2Lib::_DNxDataListPtr m_nxdlRefill;
	// (j.fouts 2013-02-01 15:07) - PLID 54986 - Added route and unit lists
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlDosageRoute;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlDosageUnit;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlDosageQuantity;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlDosageFrequency;
		// (s.tullis 2016-02-08 10:45) - PLID 67972 
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlWritePrescriptionResults;

	CNxEdit m_editQuantity;
	CNxEdit m_editSig;
	CNxEdit m_editNotes;
		// (s.tullis 2016-02-08 10:45) - PLID 67972 
	CNxEdit m_DruglistItem;
	NxButton m_checkIncludeFreeTextRx;

	boost::scoped_ptr<class AutoPopulatingSig> m_pAutoPopulatingSig;

public:
	CNexERxAddQuickListDlg(CWnd* pParent = NULL, QuickListRx qlRx = QuickListRx());   // standard constructor
	virtual ~CNexERxAddQuickListDlg();
	virtual BOOL OnInitDialog();

	// (r.gonet 2016-02-10 13:34) - PLID 58689 - Gets a QuickListRx representation of the edited quicklist medication.
	QuickListRx GetQuickListRx();
	CString GetDrugListID();
	CString GetRefills();
	CString GetQuantity();
	CString GetSig();
	// (j.fouts 2013-02-01 15:07) - PLID 54986 - Added route and unit
	CString GetDosageUnit();
	long GetDosageUnitID();
	CString GetDosageRoute();
	long GetDosageRouteID();
	CString GetDosageFreq();
	CString GetDosageQuantity();
	CString GetNotes();
	// (s.tullis 2016-02-08 10:45) - PLID 67972 
	void ResetPrescriptionSearchProvider();
// Dialog Data
	enum { IDD = IDD_ADD_QUICK_LIST_DLG };

protected:
	void FillRefills();
	void UpdateSig();
	// (s.tullis 2016-02-08 10:45) - PLID 67972 
	void FillMedicationName(long nMedicationID, CString strMedicationName = "");
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void SelChosenNxdlAddQlRefill(LPDISPATCH lpRow);
	void SelChosenNxdlAddQlDosMethod(LPDISPATCH lpRow);
	void SelChosenNxdlAddQlDosFreq(LPDISPATCH lpRow);
	void SelChosenNxdlAddQlDosQuantity(LPDISPATCH lpRow);
	afx_msg void OnEnKillfocusEditAddQlQuantity();
	afx_msg void OnEnKillfocusEditAddQlSig();
	afx_msg void OnEnKillfocusEditAddQlNotes();
	void SelChosenNxdlAddQlDosUnit(LPDISPATCH lpRow);
	void RequeryFinishedNxdlAddQlDosQuantity(short nFlags);
	void RequeryFinishedNxdlAddQlDosUnit(short nFlags);
	void RequeryFinishedNxdlAddQlDosMethod(short nFlags);
	void RequeryFinishedNxdlAddQlDosFreq(short nFlags);
	afx_msg void OnBnClickedOk();
	//(r.wilson 4/8/2013) pl 56117
	void FormatAndSetQlQuantity();
	void FormatAndSetQlNotes();
	void FormatAndSetQlSig();
public:
		// (s.tullis 2016-02-08 10:45) - PLID 67972 
	afx_msg void OnBnClickedQlCheckIncludeFreeTextRx();
	void SelChosenNxdlWriteRx(LPDISPATCH lpRow);
};
