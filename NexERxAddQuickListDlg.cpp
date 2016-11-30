// NexERxAddQuickListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NexERxAddQuickListDlg.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here
#include "AutoPopulatingSig.h"
#include "PatientsRc.h"
#include "PrescriptionUtilsAPI.h"

// CNexERxAddQuickListDlg dialog
// (b.savon 2013-01-23 10:02) - PLID 54831 - Created

enum EDrugList{
	dlID = 0,
	dlName,
	dlFDBID,
	dlFDBOutOfDate, //TES 5/9/2013 - PLID 56614
};

enum ERefill{
	erID = 0,
};

// (j.fouts 2013-02-01 15:07) - PLID 54986 - Added an enum for the Route coulmns
enum DosageRouteColumns
{
	drcID = 0,
	drcRoute,
	drcRouteSig,
};

// (j.fouts 2013-02-01 15:07) - PLID 54986 - Added a Units list
enum DosageUnitsColumns
{
	ducID = 0,
	ducUnit,
	ducUnitPl,
};

enum DosageFrequencyColumn{
	dfcSig = 0,
};

enum DosageQuantityColumn{
	dqcSig = 0,
};

IMPLEMENT_DYNAMIC(CNexERxAddQuickListDlg, CNxDialog)

CNexERxAddQuickListDlg::CNexERxAddQuickListDlg(CWnd* pParent /*=NULL*/, QuickListRx qlRx /* = QuickListRx()*/)
	: CNxDialog(CNexERxAddQuickListDlg::IDD, pParent)
{
	m_nDosageUnitID = -1;
	m_nDosageRouteID = -1;
	m_qlRx = qlRx;
	if( qlRx.nDrugListID > 0 ){
		m_bEditMode = TRUE;
	}else{
		m_bEditMode = FALSE;
	}
}

CNexERxAddQuickListDlg::~CNexERxAddQuickListDlg()
{
}

BOOL CNexERxAddQuickListDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{

		SetTitleBarIcon(IDI_ADD_QUICK_LIST);
		m_nxcBack.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// Don't requery the drug list if were editing, just put the name in the drop down and make it readonly.
		// Only requery if we are adding or editing but something unexpected happens.
	
		m_nxdlRefill = BindNxDataList2Ctrl(IDC_NXDL_ADD_QL_REFILL, false);
		FillRefills();
		// (j.fouts 2013-02-01 15:07) - PLID 54986 - Renamed method to route and added a new list
		m_nxdlDosageRoute = BindNxDataList2Ctrl(IDC_NXDL_ADD_QL_DOS_METHOD, true);
		m_nxdlDosageUnit = BindNxDataList2Ctrl(IDC_NXDL_ADD_QL_DOS_UNIT, true);
		m_nxdlDosageFrequency = BindNxDataList2Ctrl(IDC_NXDL_ADD_QL_DOS_FREQ, true);
		m_nxdlDosageQuantity = BindNxDataList2Ctrl(IDC_NXDL_ADD_QL_DOS_QUANTITY, true);
		
		m_editQuantity.SetLimitText(50);

		m_pAutoPopulatingSig.reset(new AutoPopulatingSig);
		// (s.tullis 2016-02-08 10:45) - PLID 67972 -Check for FDB license
		m_DruglistItem.SetReadOnly();
		if (!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			//no FDB? no checkbox, and always include free-text meds, because that's all you've got
			m_checkIncludeFreeTextRx.SetCheck(TRUE);
			m_checkIncludeFreeTextRx.ShowWindow(SW_HIDE);
			// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med search color, hide if no fdb
			m_icoAboutQLMedicationColors.ShowWindow(SW_HIDE);
		}
		else {// (s.tullis 2016-02-08 10:45) - PLID 67972 - Set our preference value for the include free text checkbox
			long nIncludeFreeTextFDBSearchResults = GetRemotePropertyInt("IncludeFreeTextFDBSearchResults", 0, 0, GetCurrentUserName(), true);
			m_checkIncludeFreeTextRx.SetCheck(nIncludeFreeTextFDBSearchResults & INCLUDEFREETEXT_PRESCRIPTIONS);
			// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med search color
			CString strMedsToolTipText = "All medications with a salmon background are imported and are checked for interactions. \r\n"
				"All medications with a red background have changed since being imported, and must be updated before being used on new prescriptions. \r\n"
				"Using free text medications (white background) rather than those from the imported lists (salmon background) will result in a lack of interaction warnings.";
			m_icoAboutQLMedicationColors.LoadToolTipIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_QUESTION_MARK), strMedsToolTipText, false, false, false);
			m_icoAboutQLMedicationColors.EnableClickOverride();

		}
		
		// (s.tullis 2016-02-08 10:45) - PLID 67972 - Set the Medication Search
		ResetPrescriptionSearchProvider();
		//Set the text boxes
		if( m_bEditMode ){
			m_editQuantity.SetWindowText(m_qlRx.strQuantity);
			OnEnKillfocusEditAddQlQuantity();
			m_editSig.SetWindowText(m_qlRx.strSig);
			OnEnKillfocusEditAddQlSig();
			m_editNotes.SetWindowText(m_qlRx.strNotes);
			OnEnKillfocusEditAddQlNotes();
			SetWindowText("Edit Quick List");
			// (s.tullis 2016-02-08 10:45) - PLID 67972 - If this is edit mode set our medication name is the edit window
			FillMedicationName(m_qlRx.nDrugListID);
			m_nxdlWritePrescriptionResults->PutEnabled(VARIANT_FALSE);
			m_checkIncludeFreeTextRx.EnableWindow(FALSE);
			m_pAutoPopulatingSig->SetSig(m_qlRx.strSig);
		
		}
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CNexERxAddQuickListDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	
	DDX_Control(pDX, IDC_NXC_BACK, m_nxcBack);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EDIT_ADD_QL_QUANTITY, m_editQuantity);
	DDX_Control(pDX, IDC_EDIT_ADD_QL_ADD_SIG, m_editSig);
	DDX_Control(pDX, IDC_EDIT_ADD_QL_NOTES, m_editNotes);
	// (s.tullis 2016-02-08 10:45) - PLID 67972
	DDX_Control(pDX, IDC_QL_CHECK_INCLUDE_FREE_TEXT_RX, m_checkIncludeFreeTextRx);
	DDX_Control(pDX, IDC_DRUG_ITEM_EDIT, m_DruglistItem);
	DDX_Control(pDX, IDC_QL_ABOUT_MEDICATION_COLORS, m_icoAboutQLMedicationColors);
}

void CNexERxAddQuickListDlg::FillRefills()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		for( int idx = 0; idx < 13; idx++ ){
			pRow = m_nxdlRefill->GetNewRow();
			if( pRow ){
				pRow->PutValue(erID, (long)idx);
			}
			m_nxdlRefill->AddRowSorted(pRow, NULL);
		}
		// (b.savon 2013-02-05 14:14) - PLID 55020
		if( m_bEditMode ){
			pRow = m_nxdlRefill->SetSelByColumn(erID, m_qlRx.nRefill);
			if( pRow ){
				CString strRefill;
				strRefill.Format("%li", VarLong(pRow->GetValue(erID)));
				m_nxdlRefill->PutComboBoxText(AsBstr(strRefill));
				SelChosenNxdlAddQlRefill(pRow);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

BEGIN_MESSAGE_MAP(CNexERxAddQuickListDlg, CNxDialog)
	ON_EN_KILLFOCUS(IDC_EDIT_ADD_QL_QUANTITY, &CNexERxAddQuickListDlg::OnEnKillfocusEditAddQlQuantity)
	ON_EN_KILLFOCUS(IDC_EDIT_ADD_QL_ADD_SIG, &CNexERxAddQuickListDlg::OnEnKillfocusEditAddQlSig)
	ON_EN_KILLFOCUS(IDC_EDIT_ADD_QL_NOTES, &CNexERxAddQuickListDlg::OnEnKillfocusEditAddQlNotes)
	ON_BN_CLICKED(IDOK, &CNexERxAddQuickListDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_QL_CHECK_INCLUDE_FREE_TEXT_RX, &CNexERxAddQuickListDlg::OnBnClickedQlCheckIncludeFreeTextRx)
END_MESSAGE_MAP()


// CNexERxAddQuickListDlg message handlers
BEGIN_EVENTSINK_MAP(CNexERxAddQuickListDlg, CNxDialog)

	ON_EVENT(CNexERxAddQuickListDlg, IDC_NXDL_ADD_QL_REFILL, 16, CNexERxAddQuickListDlg::SelChosenNxdlAddQlRefill, VTS_DISPATCH)
	ON_EVENT(CNexERxAddQuickListDlg, IDC_NXDL_ADD_QL_DOS_METHOD, 16, CNexERxAddQuickListDlg::SelChosenNxdlAddQlDosMethod, VTS_DISPATCH)
	ON_EVENT(CNexERxAddQuickListDlg, IDC_NXDL_ADD_QL_DOS_FREQ, 16, CNexERxAddQuickListDlg::SelChosenNxdlAddQlDosFreq, VTS_DISPATCH)
	ON_EVENT(CNexERxAddQuickListDlg, IDC_NXDL_ADD_QL_DOS_QUANTITY, 16, CNexERxAddQuickListDlg::SelChosenNxdlAddQlDosQuantity, VTS_DISPATCH)
	ON_EVENT(CNexERxAddQuickListDlg, IDC_NXDL_ADD_QL_DOS_UNIT, 16, CNexERxAddQuickListDlg::SelChosenNxdlAddQlDosUnit, VTS_DISPATCH)
	ON_EVENT(CNexERxAddQuickListDlg, IDC_NXDL_ADD_QL_DOS_QUANTITY, 18, CNexERxAddQuickListDlg::RequeryFinishedNxdlAddQlDosQuantity, VTS_I2)
	ON_EVENT(CNexERxAddQuickListDlg, IDC_NXDL_ADD_QL_DOS_UNIT, 18, CNexERxAddQuickListDlg::RequeryFinishedNxdlAddQlDosUnit, VTS_I2)
	ON_EVENT(CNexERxAddQuickListDlg, IDC_NXDL_ADD_QL_DOS_METHOD, 18, CNexERxAddQuickListDlg::RequeryFinishedNxdlAddQlDosMethod, VTS_I2)
	ON_EVENT(CNexERxAddQuickListDlg, IDC_NXDL_ADD_QL_DOS_FREQ, 18, CNexERxAddQuickListDlg::RequeryFinishedNxdlAddQlDosFreq, VTS_I2)

	
	ON_EVENT(CNexERxAddQuickListDlg, IDC_NXDL_WRITE_RX, 16, CNexERxAddQuickListDlg::SelChosenNxdlWriteRx, VTS_DISPATCH)
END_EVENTSINK_MAP()


void CNexERxAddQuickListDlg::SelChosenNxdlAddQlRefill(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow ){
			m_nxdlRefill->PutCurSel(pRow);
			long nRefills = VarLong(pRow->GetValue(erID));
			m_strRefills.Format("%li", nRefills);
			m_nxdlRefill->PutComboBoxText(_bstr_t(m_strRefills));
		}else{
			m_strRefills = "";
		}
	}NxCatchAll(__FUNCTION__);
}

void CNexERxAddQuickListDlg::SelChosenNxdlAddQlDosMethod(LPDISPATCH lpRow)
{
	try{
		CString strSigRoute;
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		// (j.fouts 2013-02-01 15:07) - PLID 54986 - Renamed Method to route and added an ID
		if( pRow ){
			m_strDosageRoute = VarString(pRow->GetValue(drcRoute), "");
			m_nDosageRouteID = VarLong(pRow->GetValue(drcID), -1);
			strSigRoute = VarString(pRow->GetValue(drcRouteSig), "");
		}else{
			m_strDosageRoute = "";
			m_nDosageRouteID = -1;
			strSigRoute = "";
		}
		
		m_pAutoPopulatingSig->SetRoute(strSigRoute);
		UpdateSig();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxAddQuickListDlg::SelChosenNxdlAddQlDosFreq(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow ){
			m_nxdlDosageFrequency->PutCurSel(pRow);
			m_nxdlDosageFrequency->PutComboBoxText(_bstr_t(pRow->GetValue(dfcSig)));
			m_strDosageFreq = VarString(pRow->GetValue(dfcSig), "");
		}else{
			m_strDosageFreq = "";
		}
		m_pAutoPopulatingSig->SetFrequency(m_strDosageFreq, -1.0/*We don't care about value here*/);
		UpdateSig();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxAddQuickListDlg::SelChosenNxdlAddQlDosQuantity(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow ){
			m_nxdlDosageQuantity->PutCurSel(pRow);
			m_nxdlDosageQuantity->PutComboBoxText(_bstr_t(pRow->GetValue(dqcSig)));
			m_strDosageQuantity = VarString(pRow->GetValue(dqcSig), "");	
		}else{
			m_strDosageQuantity = "";
		}
		m_pAutoPopulatingSig->SetDosageQuantity(m_strDosageQuantity, -1.0/*We don't care about value here*/);
		UpdateSig();
	}NxCatchAll(__FUNCTION__);
}


void CNexERxAddQuickListDlg::OnEnKillfocusEditAddQlQuantity()
{
	try{
	
		//(r.wilson 4/8/2013) pl 56117 - Formatting and setting of Quantity done in function now
		FormatAndSetQlQuantity();

	}NxCatchAll(__FUNCTION__);
}

void CNexERxAddQuickListDlg::FormatAndSetQlQuantity()
{
		//(r.wilson 4/8/2013) pl 56117 - First trim the quantity field of white spaces on the ends
		CString strQuantityRaw;
		m_editQuantity.GetWindowText(strQuantityRaw);
		
		//(r.wilson 4/8/2013) pl 56117 - We will trim regardless of whether the number is formatted correctly or not
		strQuantityRaw.Trim();		

		//(r.wilson 4/8/2013) pl 56117 - Format the value
		strQuantityRaw = ::FormatDecimalText(CString(strQuantityRaw), 11);
		//(r.wilson 4/8/2013) pl 56117 - Remove unnecessary zeros from both ends
		strQuantityRaw = ::RemoveInsignificantZeros(CString(strQuantityRaw));
		
		//(r.wilson 4/8/2013) pl 56117 - Update the textbox with new quantity value
		m_editQuantity.SetWindowTextA(strQuantityRaw);
		//(r.wilson 4/8/2013) pl 56117 - Update the memebr variable 
		m_strQuantity = strQuantityRaw;
}

void CNexERxAddQuickListDlg::OnEnKillfocusEditAddQlSig()
{
	try{

		//(r.wilson 4/8/2013) pl 56117 - Formatting and setting of Sig now done in a function
		FormatAndSetQlSig();

	}NxCatchAll(__FUNCTION__);
}

//(r.wilson 4/8/2013) pl 56117 - Code moved from OnEnKillfocusEditAddQlSig to allow us to use this code in other places..
void CNexERxAddQuickListDlg::FormatAndSetQlSig()
{
	//(r.wilson 4/8/2013) pl 56117  - Going to trim off white spaces before saving to the member variable
		CString strSigRaw;
		m_editSig.GetWindowText(strSigRaw);
		strSigRaw.Trim();
				
		//(r.wilson 4/8/2013) pl 56117 - Put the trimmed version of the variable back in the textbox
		m_editSig.SetWindowTextA(strSigRaw);		

		//(r.wilson 4/8/2013) pl 56117 - Assign member varible the value of the trimmed variable
		m_strSig = strSigRaw;
		m_pAutoPopulatingSig->SetSig(m_strSig);
}


void CNexERxAddQuickListDlg::OnEnKillfocusEditAddQlNotes()
{
	try{
	
		//(r.wilson 4/8/2013) pl 56117 - Formatting and Setting of Notes done in a functon now..
		FormatAndSetQlNotes();		

	}NxCatchAll(__FUNCTION__);
}

void CNexERxAddQuickListDlg::FormatAndSetQlNotes()
{
		//(r.wilson 4/8/2013) pl 56117 - Trimming the notes field of white spaces
		CString strNotesRaw;
		m_editNotes.GetWindowText(strNotesRaw);
		strNotesRaw.Trim();
		
		//(r.wilson 4/8/2013) pl 56117 - Put trimmed notes back into the textbox
		m_editNotes.SetWindowTextA(strNotesRaw);

		//(r.wilson 4/8/2013) pl 56117 - Set Notes member variable with the trimmed version
		m_strNotes = strNotesRaw;
}

// (r.gonet 2016-02-10 13:34) - PLID 58689 - Gets the edited quicklist medication as a QuickListRx object.
QuickListRx CNexERxAddQuickListDlg::GetQuickListRx()
{
	// (r.gonet 2016-02-10 13:34) - PLID 58689 - Some values we don't edit in this dialog, like OrderIndex, so
	// first just copy the object, field by field, then assign the fields that we do edit.
	QuickListRx quickListRx = m_qlRx;
	quickListRx.nDrugListID = m_strDrugListID.IsEmpty() ? -1 : atol(m_strDrugListID);
	quickListRx.strName = m_strDrugListName;
	quickListRx.nRefill = m_strRefills.IsEmpty() ? -1 : atol(m_strRefills);
	quickListRx.strQuantity = m_strQuantity;
	quickListRx.strSig = m_strSig;
	quickListRx.nDosUnitID = m_nDosageUnitID;
	quickListRx.nDosRouteID = m_nDosageRouteID;
	quickListRx.strDosFreq = m_strDosageFreq;
	quickListRx.strDosQuantity = m_strDosageQuantity;
	quickListRx.strNotes = m_strNotes;
	return quickListRx;
}

CString CNexERxAddQuickListDlg::GetDrugListID()
{
	return m_strDrugListID;
}

CString CNexERxAddQuickListDlg::GetRefills()
{
	return m_strRefills;
}

CString CNexERxAddQuickListDlg::GetQuantity()
{
	return m_strQuantity;
}

CString CNexERxAddQuickListDlg::GetSig()
{
	return m_strSig;
}

// (j.fouts 2013-02-01 15:07) - PLID 54986 - public accessor for Dosage unit
CString CNexERxAddQuickListDlg::GetDosageUnit()
{
	return m_strDosageUnit;
}

// (j.fouts 2013-02-01 15:07) - PLID 54986 - public accessor for Dosage unit ID
long CNexERxAddQuickListDlg::GetDosageUnitID()
{
	return m_nDosageUnitID;
}

// (j.fouts 2013-02-01 15:07) - PLID 54986 - public accessor for Dosage Route
CString CNexERxAddQuickListDlg::GetDosageRoute()
{
	return m_strDosageRoute;
}

// (j.fouts 2013-02-01 15:07) - PLID 54986 - public accessor for Dosage Route ID
long CNexERxAddQuickListDlg::GetDosageRouteID()
{
	return m_nDosageRouteID;
}

CString CNexERxAddQuickListDlg::GetDosageFreq()
{
	return m_strDosageFreq;
}

CString CNexERxAddQuickListDlg::GetDosageQuantity()
{
	return m_strDosageQuantity;
}

CString CNexERxAddQuickListDlg::GetNotes()
{
	return m_strNotes;
}

// (j.fouts 2013-02-01 15:07) - PLID 54986 - Save the changes for the new Unit list
void CNexERxAddQuickListDlg::SelChosenNxdlAddQlDosUnit(LPDISPATCH lpRow)
{
	try
	{
		CString strDosageUnitPl;
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow ){
			m_strDosageUnit = VarString(pRow->GetValue(ducUnit), "");
			strDosageUnitPl = VarString(pRow->GetValue(ducUnitPl), "");
			m_nDosageUnitID = VarLong(pRow->GetValue(ducID), -1);
		}else{
			m_strDosageUnit = "";
			strDosageUnitPl = "";
			m_nDosageUnitID = -1;
		}
		m_pAutoPopulatingSig->SetUnit(m_strDosageUnit, strDosageUnitPl);
		UpdateSig();
	}
	NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-02-05 14:30) - PLID 55020
void CNexERxAddQuickListDlg::RequeryFinishedNxdlAddQlDosQuantity(short nFlags)
{
	try{
		if( m_bEditMode ){
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlDosageQuantity->SetSelByColumn(dqcSig, AsBstr(m_qlRx.strDosQuantity));
			if( pRow ){
				m_nxdlDosageQuantity->PutComboBoxText(AsBstr(m_qlRx.strDosQuantity));
				SelChosenNxdlAddQlDosQuantity(pRow);
				m_pAutoPopulatingSig->SetDosageQuantity(m_qlRx.strDosQuantity, -1.0/*We don't care about value, we are not calculating Qty*/);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-02-05 14:31) - PLID 55020
void CNexERxAddQuickListDlg::RequeryFinishedNxdlAddQlDosUnit(short nFlags)
{
	try{
		if( m_bEditMode ){
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlDosageUnit->SetSelByColumn(ducID, m_qlRx.nDosUnitID);
			if( pRow ){
				CString strUnit = VarString(pRow->GetValue(ducUnit));
				CString strUnitPl = VarString(pRow->GetValue(ducUnitPl));
				m_nxdlDosageUnit->PutComboBoxText(AsBstr(strUnit));
				SelChosenNxdlAddQlDosUnit(pRow);
				m_pAutoPopulatingSig->SetUnit(strUnit, strUnitPl);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-02-05 14:31) - PLID 55020
void CNexERxAddQuickListDlg::RequeryFinishedNxdlAddQlDosMethod(short nFlags)
{
	try{
		if( m_bEditMode ){
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlDosageRoute->SetSelByColumn(drcID, m_qlRx.nDosRouteID);
			if( pRow ){
				CString strSigRoute = VarString(pRow->GetValue(drcRouteSig));
				m_nxdlDosageRoute->PutComboBoxText(AsBstr(VarString(pRow->GetValue(drcRoute))));
				SelChosenNxdlAddQlDosMethod(pRow);
				m_pAutoPopulatingSig->SetRoute(strSigRoute);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-02-05 14:36) - PLID 55020
void CNexERxAddQuickListDlg::RequeryFinishedNxdlAddQlDosFreq(short nFlags)
{
	try{
		if( m_bEditMode ){
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlDosageFrequency->SetSelByColumn(dfcSig, AsBstr(m_qlRx.strDosFreq));
			if( pRow ){
				m_nxdlDosageFrequency->PutComboBoxText(AsBstr(m_qlRx.strDosFreq));
				SelChosenNxdlAddQlDosFreq(pRow);
				m_pAutoPopulatingSig->SetFrequency(m_qlRx.strDosFreq, -1.0/*We don't care about value, we are not calculating Qty*/);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CNexERxAddQuickListDlg::OnBnClickedOk()
{
	try{
		// (s.tullis 2016-02-08 10:45) - PLID 67972 - Check and see if we have chosen a medication
		CString strDrug = "";
		m_DruglistItem.GetWindowText(strDrug);
		if( strDrug.IsEmpty() ){
			MsgBox("You must select an item from the drug list or click Cancel to continue.", "Make a selection", MB_ICONINFORMATION);
			return;
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxAddQuickListDlg::UpdateSig()
{
	CString strSig;
	if(m_pAutoPopulatingSig->GetSig(strSig))
	{
		m_editSig.SetWindowText(strSig);
		m_strSig = strSig;
	}
}

// (s.tullis 2016-02-08 10:45) - PLID 67972 - Set the Medication Search according to our include FDB free text checkbox
void CNexERxAddQuickListDlg::ResetPrescriptionSearchProvider()
{
	try {

		bool bIncludeFDBMedsOnly = m_checkIncludeFreeTextRx.GetCheck() ? false : true;
		//check for FDB license
		if (!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			bIncludeFDBMedsOnly = false;
		}
		
		//rebind the search this is the quicklist so no formularies
		m_nxdlWritePrescriptionResults = BindMedicationSearchListCtrl(this, IDC_NXDL_WRITE_RX, GetRemoteData(), false, bIncludeFDBMedsOnly);

	}NxCatchAll(__FUNCTION__);
}

	// (s.tullis 2016-02-08 10:45) - PLID 67972 - Warn the User if they click the include FDB freetext
void CNexERxAddQuickListDlg::OnBnClickedQlCheckIncludeFreeTextRx()
{
	try {

		if (m_checkIncludeFreeTextRx.GetCheck()) {

			if (!ConfirmFreeTextSearchWarning(this, false)) {
				//they changed their minds
				m_checkIncludeFreeTextRx.SetCheck(FALSE);
				return;
			}

			//there are some weird focus issues here where the messagebox takes away focus
			//and the datalist placeholder text can't figure out where focus is,
			//so set the focus back to the checkbox
			m_checkIncludeFreeTextRx.SetFocus();
		}

		//reflect their choice in the search results
		ResetPrescriptionSearchProvider();
	
	}NxCatchAll(__FUNCTION__)
}

	// (s.tullis 2016-02-08 10:45) - PLID 67972 - Handle med search selection
void CNexERxAddQuickListDlg::SelChosenNxdlWriteRx(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}

		if (VarLong(pRow->GetValue(mrcMedicationID)) == NO_RESULTS_ROW) {
			return;
		}

		long nMedicationID = VarLong(pRow->GetValue(mrcMedicationID), -1);
		CString strMedicationName = VarString(pRow->GetValue(mrcMedicationName));
		long nFDBID = VarLong(pRow->GetValue(mrcFirstDataBankID), -1);
		BOOL bFDBOutOfDate = VarBool(pRow->GetValue(mrcFDBOutOfDate), FALSE) && nFDBID > 0;

		//If the medicationid is -1 (i.e. DrugList.ID), we need to import it from FDB
		long nNewDrugListID = nMedicationID;
		if (nMedicationID == -1) {
			// (r.gonet 2016-02-09) - PLID 68215 - Remember to actually use the returned drug list ID. We were just ignoring it
			// before and passing -1 along as the nMedicationID, even though the drug was imported. This caused a FK violation later.
			if (ImportMedication(nFDBID, strMedicationName, nNewDrugListID) != VARIANT_FALSE) {
				nMedicationID = nNewDrugListID;
			} else {
				ThrowNxException("%s : Medication could not be imported.", __FUNCTION__);
			}
		}
		
		if (nFDBID != -1) {
			if (nFDBID > 0 && bFDBOutOfDate) {
				m_DruglistItem.SetBackgroundColorStandard((COLORREF)ERX_IMPORTED_OUTOFDATE_COLOR, true, true);
				m_DruglistItem.SetBackgroundColorHovered((COLORREF)ERX_IMPORTED_OUTOFDATE_COLOR, true, true);
				m_DruglistItem.SetBackgroundColorFocus((COLORREF)ERX_IMPORTED_OUTOFDATE_COLOR, true, true);
				m_DruglistItem.SetBackgroundColorHoveredFocus((COLORREF)ERX_IMPORTED_OUTOFDATE_COLOR, true, true);
			}
			else {
				m_DruglistItem.SetBackgroundColorStandard((COLORREF)ERX_IMPORTED_COLOR, true, true);
				m_DruglistItem.SetBackgroundColorHovered((COLORREF)ERX_IMPORTED_COLOR, true, true);
				m_DruglistItem.SetBackgroundColorFocus((COLORREF)ERX_IMPORTED_COLOR, true, true);
				m_DruglistItem.SetBackgroundColorHoveredFocus((COLORREF)ERX_IMPORTED_COLOR, true, true);
			}
		}
		else {
			m_DruglistItem.ResetBackgroundColorStandard();
			m_DruglistItem.ResetBackgroundColorHovered();
			m_DruglistItem.ResetBackgroundColorFocus();
			m_DruglistItem.ResetBackgroundColorHoveredFocus();
		}
		
		FillMedicationName(nMedicationID, strMedicationName);

	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2016-02-08 10:45) - PLID 67972 - Fill the Medication Name
void CNexERxAddQuickListDlg::FillMedicationName(long nMedicationID, CString strMedicationName )
{
	try {
		if (strMedicationName.IsEmpty())
		{
			ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT DrugName FROM DrugList WHERE ID = {INT} ",nMedicationID);
			strMedicationName = AdoFldString(prs, "DrugName");
		}
		m_strDrugListID = FormatString("%li",nMedicationID);
		// (r.gonet 2016-02-10 13:34) - PLID 58689 - Start tracking the drug list name in order
		// to be able to create a QuickListRx object if necessary.
		m_strDrugListName = strMedicationName;
		m_DruglistItem.SetWindowText(strMedicationName);
	}NxCatchAll(__FUNCTION__)
}



