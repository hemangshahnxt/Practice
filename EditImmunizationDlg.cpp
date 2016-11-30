// EditImmunizationDlg.cpp : implementation file
// (d.thompson 2009-05-12) - PLID 34232 - Created
//

#include "stdafx.h"
#include "Practice.h"
#include "EditImmunizationDlg.h"
#include "EditComboBox.h"
#include "EditImmunizationTypesDlg.h"
#include "UTSSearchDlg.h"

using namespace NXDATALIST2Lib;
using namespace ADODB;

// CEditImmunizationDlg dialog
enum eTypeColumns {
	etcID = 0,
	etcType,
	etcDefaultDosage,
	// (a.walling 2010-09-13 14:31) - PLID 40505 - Support dosage units for vaccinations
	etcDefaultDosageUnits,
	// (a.walling 2010-02-18 09:26) - PLID 37434
	etcDescription,
};

enum eRouteColumns {
	ercID = 0,
	ercName,
};

enum eSiteColumns {
	escID = 0,
	escName,
};

// (a.walling 2010-09-13 08:19) - PLID 40497
enum eManufacturerColumns {
	emcID = 0,
	emcName,
};

// (d.singleton 2013-08-23 15:37) - PLID 58274 - add ordering provider to the immunization dlg
enum eProviderColumns {
	epcID = 0,
	epcName,
};

enum eAdministrativeNotesColumns {
	eanID = 0,
	eanName,
};

enum eEvidenceImmunity {
	eeiName = 0,
	eeiDescription,
	eeiCode,
};

enum eRefusalReason {
	errCode = 0,
	errName,
};

enum eFinancialClass {
	efcName = 0,
	efcCode,
};

enum eVISColumns {
	evcPresented = 0,
	evcPublished,
	evcCVX,
};

IMPLEMENT_DYNAMIC(CEditImmunizationDlg, CNxDialog)

CEditImmunizationDlg::CEditImmunizationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditImmunizationDlg::IDD, pParent)
{
	m_nID = -1;
	m_nPersonID = -1;
}

CEditImmunizationDlg::~CEditImmunizationDlg()
{
}

void CEditImmunizationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_IMM_DOSAGE, m_nxeditDosage);
	DDX_Control(pDX, IDC_IMM_LOT_NUMBER, m_nxeditLotNumber);
	DDX_Control(pDX, IDC_IMM_MANUFACTURER, m_nxeditManufacturer);
	DDX_Control(pDX, IDC_IMM_REACTION, m_nxeditReaction);
	// (a.walling 2010-09-13 14:31) - PLID 40505 - Support dosage units for vaccinations
	DDX_Control(pDX, IDC_IMM_UCUM_LABEL, m_nxlabelUnits);
	DDX_Control(pDX, IDC_IMM_DOSAGE_UNITS_EDIT, m_nxeditDosageUnits);
	// (d.singleton 2013-07-11 12:30) - PLID 57515 - Immunizations need to track an "Administrative Notes"
	DDX_Control(pDX, IDC_EDIT_ADMINISTRATIVE_NOTES, m_nxeditAdminNotes);
}


BEGIN_MESSAGE_MAP(CEditImmunizationDlg, CNxDialog)
	ON_BN_CLICKED(IDC_EDIT_IMM_TYPE, &CEditImmunizationDlg::OnBnClickedEditImmType)
	ON_BN_CLICKED(IDC_EDIT_IMM_ROUTE, &CEditImmunizationDlg::OnBnClickedEditImmRoute)
	ON_BN_CLICKED(IDC_EDIT_IMM_SITE, &CEditImmunizationDlg::OnBnClickedEditImmSite)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, &CEditImmunizationDlg::OnLabelClick)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_IMMUNIZATIONS_ADD_VIS, &CEditImmunizationDlg::OnBnClickedAddVIS) // (a.walling 2013-11-11 14:43) - PLID 59414 - Immunizations - Vaccine Information Statements
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEditImmunizationDlg, CNxDialog)
	ON_EVENT(CEditImmunizationDlg, IDC_IMM_TYPE_LIST, 16 /* SelChosen */, OnSelChosenTypeList, VTS_DISPATCH)
	ON_EVENT(CEditImmunizationDlg, IDC_IMM_MANUFACTURER_LIST, 16, CEditImmunizationDlg::SelChosenManufacturerList, VTS_DISPATCH)
	ON_EVENT(CEditImmunizationDlg, IDC_ORDERING_PROVIDER, 1, CEditImmunizationDlg::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEditImmunizationDlg, IDC_ADMINISTERING_PROVIDER, 1, CEditImmunizationDlg::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEditImmunizationDlg, IDC_ADMINISTRATIVE_NOTES_LIST, 1, CEditImmunizationDlg::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEditImmunizationDlg, IDC_ADMINISTRATIVE_NOTES_LIST, 16, CEditImmunizationDlg::SelChosenAdministrativeNotesList, VTS_DISPATCH)
	ON_EVENT(CEditImmunizationDlg, IDC_EVIDENCE_IMMUNITY, 1, CEditImmunizationDlg::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()


// CEditImmunizationDlg message handlers
BOOL CEditImmunizationDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//interface niceties
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_pTypeList = BindNxDataList2Ctrl(IDC_IMM_TYPE_LIST, GetRemoteData(), true);
		m_pRouteList = BindNxDataList2Ctrl(IDC_ROUTES_LIST, GetRemoteData(), true);
		m_pSiteList = BindNxDataList2Ctrl(IDC_SITES_LIST, GetRemoteData(), true);
		m_pManufacturerList = BindNxDataList2Ctrl(IDC_IMM_MANUFACTURER_LIST, GetRemoteData(), true); // (a.walling 2010-09-13 08:19) - PLID 40497
		// (d.singleton 2013-08-23 15:37) - PLID 58274 - add ordering provider and location to the immunization dlg
		m_pOrderingProviderList = BindNxDataList2Ctrl(IDC_ORDERING_PROVIDER, GetRemoteData(), true);
		// (a.walling 2013-11-13 13:34) - PLID 59464 - Administering provider
		m_pAdministeringProviderList = BindNxDataList2Ctrl(IDC_ADMINISTERING_PROVIDER, GetRemoteData(), true);
		m_pAdminNotes = BindNxDataList2Ctrl(IDC_ADMINISTRATIVE_NOTES_LIST, GetRemoteData(), true);
		// (d.singleton 2013-10-01 16:45) - PLID 58843 - Need to add snomed code selection to immunization dialog for evidence of immunity.
		m_pEvidenceImmunity = BindNxDataList2Ctrl(IDC_EVIDENCE_IMMUNITY, GetRemoteData(), true);		
		// (a.walling 2013-11-11 11:33) - PLID 58781 - immunization financial class / VFC eligibility
		m_pFinancialClass = BindNxDataList2Ctrl(IDC_IMMUNIZATION_FINANCIAL_CLASS, GetRemoteData(), true);
		// (a.walling 2013-11-11 14:43) - PLID 59414 - Immunizations - Vaccine Information Statements
		m_pVISList = BindNxDataList2Ctrl(IDC_IMMUNIZATIONS_VIS_LIST, GetRemoteData(), false);

		LoadImmTypes();

		m_pDateAdministered = BindNxTimeCtrl(this, IDC_IMM_DATE);
		m_pExpDate = BindNxTimeCtrl(this, IDC_IMM_EXP_DATE);
		m_nxeditDosage.SetLimitText(50);
		m_nxeditDosageUnits.SetLimitText(20);
		m_nxeditLotNumber.SetLimitText(50);
		m_nxeditManufacturer.SetLimitText(255);
		//(e.lally 2010-01-04) PLID 35768 - Added Reaction field
		m_nxeditReaction.SetLimitText(255);

		// (d.singleton 2013-07-11 12:30) - PLID 57515 - Immunizations need to track an "Administrative Notes"
		m_nxeditAdminNotes.SetLimitText(250);

		m_pRefusalReason = BindNxDataList2Ctrl(IDC_REFUSAL_REASON, GetRemoteData(), true);

		// (a.walling 2010-09-13 14:31) - PLID 40505 - Support dosage units for vaccinations
		CString strOriginalUnitLabelText;
		m_nxlabelUnits.GetWindowText(strOriginalUnitLabelText);
		m_nxlabelUnits.SetText(strOriginalUnitLabelText);
		m_nxlabelUnits.SetType(dtsHyperlink);

		//data integrity
		if(m_nPersonID == -1) {
			//not set
			AfxMessageBox("You must provide a patient when using this dialog.");
			CDialog::OnCancel();
			return TRUE;
		}

		//Load everything from data
		// (d.thompson 2013-07-16) - PLID 57579 - Refactored to its own class
		m_dataLastSaved.m_nPersonID = m_nPersonID;
		// (d.thompson 2013-07-16) - PLID 57513 - For auditing
		m_dataLastSaved.m_strPersonName = m_strPersonName;
		m_dataLastSaved.LoadFromData(m_nID);

		// (d.thompson 2013-07-16) - PLID 57579 - Update the UI with the data we loaded
		ReflectDataToUI();

		if(m_dataLastSaved.IsNew()) {
			//If new, set default values for all new records and ignore the last saved values for these specific fields.
			m_pDateAdministered->SetDateTime(COleDateTime::GetCurrentTime());
			// (a.walling 2010-09-13 08:20) - PLID 40497
			m_pManufacturerList->SetSelByColumn(emcID, (LPCTSTR)"UNK");
			SetDlgItemText(IDC_IMM_MANUFACTURER, "Unknown manufacturer");
		}

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

void CEditImmunizationDlg::OnOK()
{
	try {
		// (a.walling 2013-11-11 14:43) - PLID 59414 - Immunizations - Vaccine Information Statements
		if (!CheckVISData()) {
			return;
		}

		//Get the interface into variables again
		// (d.thompson 2013-07-16) - PLID 57579 - Refactored data to its own type
		CPatientImmunizationData data = GetImmunizationDataFromUI();

		// (d.thompson 2013-07-16) - PLID 57579 - Now let's save.  This now does it's own error checking
		//	to ensure the data set contains everything that is required before save will succeed.
		CString strErrorMessage;
		// (d.thompson 2013-07-16) - PLID 57513 - Pass in the last saved data so we can audit against it
		if(data.SaveToData(m_dataLastSaved, strErrorMessage)) {
			//Success!
		}
		else {
			//Failure!  Tell them why.
			AfxMessageBox("Failed to save immunization:\r\n" + strErrorMessage);
			return;
		}

		//Success!
		CDialog::OnOK();

	} NxCatchAll("Error in OnOK()");
}

void CEditImmunizationDlg::OnCancel()
{
	try {
		CDialog::OnCancel();
	} NxCatchAll("Error in OnCancel()");
}

void CEditImmunizationDlg::OnBnClickedEditImmType()
{
	try {
		CEditImmunizationTypesDlg dlg(this);
		dlg.DoModal();

		//refresh the type list
		_variant_t varSel = g_cvarNull;
		IRowSettingsPtr pRow = m_pTypeList->CurSel;
		if(pRow != NULL) {
			varSel = pRow->GetValue(etcID);
		}
		m_pTypeList->Requery();
		m_pTypeList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		m_pTypeList->SetSelByColumn(etcID, varSel);

		// (a.walling 2013-11-11 14:43) - PLID 59414 - Immunizations - Vaccine Information Statements
		LoadImmTypes();

	} NxCatchAll("Error in OnBnClickedEditImmType()");
}

void CEditImmunizationDlg::OnBnClickedEditImmRoute()
{
	try {
		//Save the current selection and requery
		_variant_t varRoute = g_cvarNull;
		if(m_pRouteList->CurSel != NULL) {
			varRoute = m_pRouteList->CurSel->GetValue(ercID);
		}

		//Edit the list.  This will force a requery
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 67, m_pRouteList, "Edit Combo Box").DoModal();

		//Now reload what was selected
		m_pRouteList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		m_pRouteList->SetSelByColumn(ercID, varRoute);

	} NxCatchAll("Error in OnBnClickedEditImmRoute()");
}

void CEditImmunizationDlg::OnBnClickedEditImmSite()
{
	try {
		//Save the current selection and requery
		_variant_t varSite = g_cvarNull;
		if(m_pSiteList->CurSel != NULL) {
			varSite = m_pSiteList->CurSel->GetValue(escID);
		}

		//Edit the list.  This will force a requery
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 68, m_pSiteList, "Edit Combo Box").DoModal();

		//Now reload what was selected
		m_pSiteList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		m_pSiteList->SetSelByColumn(escID, varSite);

	} NxCatchAll("Error in OnBnClickedEditImmSite()");
}


void CEditImmunizationDlg::OnSelChosenTypeList(LPDISPATCH lpRow)
{
	try {
		if(lpRow == NULL) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);

		//Fill the default dosage if it's not already filled
		CString strCurrentDosage;
		GetDlgItemText(IDC_IMM_DOSAGE, strCurrentDosage);

		if(!strCurrentDosage.IsEmpty()) {
			//We only fill if nothing is there
			return;
		}

		//Otherwise, set the text box with whatever is in our default
		SetDlgItemText(IDC_IMM_DOSAGE, VarString(pRow->GetValue(etcDefaultDosage), ""));
		// (a.walling 2010-09-13 14:31) - PLID 40505 - Support dosage units for vaccinations
		SetDlgItemText(IDC_IMM_DOSAGE_UNITS_EDIT, VarString(pRow->GetValue(etcDefaultDosageUnits), ""));


	} NxCatchAll("Error in OnSelChosenTypeList");
}

// (a.walling 2010-09-13 08:19) - PLID 40497
void CEditImmunizationDlg::SelChosenManufacturerList(LPDISPATCH lpRow)
{
	try {
		if(lpRow == NULL) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);

		SetDlgItemText(IDC_IMM_MANUFACTURER, VarString(pRow->GetValue(emcName), ""));

	} NxCatchAll("Error in OnSelChosenManufacturerList");
}

// (a.walling 2010-09-13 14:31) - PLID 40505 - Support dosage units for vaccinations
BOOL CEditImmunizationDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		if (pWnd == this || pWnd == &m_nxlabelUnits) {
			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);
			
			CRect rc;
			m_nxlabelUnits.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	} NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (a.walling 2010-09-13 14:31) - PLID 40505 - Support dosage units for vaccinations
LRESULT CEditImmunizationDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		// open it up!
		// (d.singleton 2013-06-12 12:50) - PLID 57144 - units of measurement for hl7 immunizations now uses UCUM instead of ansi+ so need the help text to reflect that.
		ShellExecute(NULL, NULL, "http://www.unitsofmeasure.org/", NULL, NULL, SW_SHOW);
		// (a.walling 2010-09-14 10:22) - PLID 40505 - Need to use ANSI+ instead of UCUM for HL7 v2.5.1 compatibility
		//CEditImmunizationTypesDlg::PopupANSIPlusUnitsDescription(this);
	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (d.thompson 2013-07-16) - PLID 57579 - Refactored data to its own class.  This function will take whatever is in the
//	m_dataLastSaved object and update the UI with those values.
void CEditImmunizationDlg::ReflectDataToUI()
{
	//Now put the values into the interface
	m_pTypeList->SetSelByColumn(etcID, (long)m_dataLastSaved.GetTypeID());
	m_pRouteList->SetSelByColumn(ercID, m_dataLastSaved.GetRoute());
	m_pSiteList->SetSelByColumn(escID, m_dataLastSaved.GetSite());
	m_pManufacturerList->SetSelByColumn(emcID, (LPCTSTR)m_dataLastSaved.m_strManufacturerCode); // (a.walling 2010-09-13 08:25) - PLID 40497
	SetDlgItemText(IDC_IMM_DOSAGE, m_dataLastSaved.m_strDosage);
	// (a.walling 2010-09-13 14:31) - PLID 40505 - Support dosage units for vaccinations
	SetDlgItemText(IDC_IMM_DOSAGE_UNITS_EDIT, m_dataLastSaved.m_strDosageUnits);
	SetDlgItemText(IDC_IMM_LOT_NUMBER, m_dataLastSaved.m_strLotNumber);
	SetDlgItemText(IDC_IMM_MANUFACTURER, m_dataLastSaved.m_strManufacturer);
	//(e.lally 2010-01-04) PLID 35768 - Load Reaction field
	SetDlgItemText(IDC_IMM_REACTION, m_dataLastSaved.m_strReaction);
	if(m_dataLastSaved.m_varDateAdministered.vt == VT_DATE) {
		m_pDateAdministered->SetDateTime(VarDateTime(m_dataLastSaved.m_varDateAdministered));
	}
	if(m_dataLastSaved.m_varExpDate.vt == VT_DATE) {
		m_pExpDate->SetDateTime(VarDateTime(m_dataLastSaved.m_varExpDate));
	}
	// (d.singleton 2013-07-11 12:33) - PLID 57515 - Immunizations need to track an "Administrative Notes"
	SetDlgItemText(IDC_EDIT_ADMINISTRATIVE_NOTES, m_dataLastSaved.m_strAdminNotes);

	// (d.singleton 2013-08-23 15:37) - PLID 58274 - add ordering provider to the immunization dlg
	m_pOrderingProviderList->SetSelByColumn(epcID, m_dataLastSaved.GetOrderingProvider());
	// (a.walling 2013-11-13 13:34) - PLID 59464 - Administering provider
	m_pAdministeringProviderList->SetSelByColumn(epcID, m_dataLastSaved.GetAdministeringProvider());

	m_pAdminNotes->SetSelByColumn(eanID, m_dataLastSaved.m_varAdminNotesCode);
	// (d.singleton 2013-10-02 12:06) - PLID 58843 - Need to add snomed code selection to immunization dialog for evidence of immunity.
	// (a.walling 2013-11-11 09:59) - PLID 58843 - Using pre-defined hl7 subset for snomed immunity codes; also using variant for the code directly rather than an ID
	m_pEvidenceImmunity->SetSelByColumn(eeiCode, m_dataLastSaved.GetSnomedImmunityCode());
	m_pRefusalReason->SetSelByColumn(errCode, m_dataLastSaved.GetRefusalReasonCode());

	// (a.walling 2013-11-11 11:33) - PLID 58781 - immunization financial class / VFC eligibility
	m_pFinancialClass->SetSelByColumn(efcCode, m_dataLastSaved.GetFinancialClass());

	// (a.walling 2013-11-11 14:43) - PLID 59414 - Immunizations - Vaccine Information Statements
	for each (Nx::HL7::VIS vis in m_dataLastSaved.GetVIS()) {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pVISList->GetNewRow();

		pRow->Value[evcCVX] = (const char*)vis.cvx;
		pRow->Value[evcPresented] = vis.presented.GetStatus() != COleDateTime::valid ? "" : FormatDateTimeForInterface(vis.presented, 0, dtoDate);
		pRow->Value[evcPublished] = vis.published.GetStatus() != COleDateTime::valid ? "" : FormatDateTimeForInterface(vis.published, 0, dtoDate);

		m_pVISList->AddRowAtEnd(pRow, NULL);
	}
}

// (d.thompson 2013-07-16) - PLID 57579 - Retrieves all data from the UI and returns a PatientImmunizationData
//	structure that contains all that data.  Does not do any error checking.
CPatientImmunizationData CEditImmunizationDlg::GetImmunizationDataFromUI()
{
	CPatientImmunizationData data;

	//Always set person ID
	data.m_nPersonID = m_nPersonID;
	// (d.thompson 2013-07-16) - PLID 57513 - For auditing
	data.m_strPersonName = m_strPersonName;
	//And immunization ID
	data.m_nImmunizationID = m_nID;

	if(m_pTypeList->CurSel != NULL) {
		data.SetType(VarLong(m_pTypeList->CurSel->GetValue(etcID)), VarString(m_pTypeList->CurSel->GetValue(etcType)));
	}
	if(m_pRouteList->CurSel != NULL) {
		data.SetRoute(m_pRouteList->CurSel->GetValue(ercID), VarString(m_pRouteList->CurSel->GetValue(ercName)));
	}
	if(m_pSiteList->CurSel != NULL) {
		data.SetSite(m_pSiteList->CurSel->GetValue(escID), VarString(m_pSiteList->CurSel->GetValue(escName)));
	}		
	// (a.walling 2010-09-13 08:19) - PLID 40497
	if (m_pManufacturerList->CurSel != NULL) {
		data.m_strManufacturerCode = VarString(m_pManufacturerList->CurSel->GetValue(emcID));
	}
	else {
		data.m_strManufacturerCode = "";
	}
	GetDlgItemText(IDC_IMM_DOSAGE, data.m_strDosage);
	// (a.walling 2010-09-13 14:31) - PLID 40505 - Support dosage units for vaccinations
	GetDlgItemText(IDC_IMM_DOSAGE_UNITS_EDIT, data.m_strDosageUnits);

	GetDlgItemText(IDC_IMM_LOT_NUMBER, data.m_strLotNumber);
	GetDlgItemText(IDC_IMM_MANUFACTURER, data.m_strManufacturer);
	// (a.walling 2010-09-13 08:19) - PLID 40497
	if (data.m_strManufacturer.IsEmpty() && m_pManufacturerList->CurSel != NULL) {
		data.m_strManufacturer = VarString(m_pManufacturerList->CurSel->GetValue(emcName));
	}

	//(e.lally 2010-01-04) PLID 35768 - Added Reaction field
	GetDlgItemText(IDC_IMM_REACTION, data.m_strReaction); 
	
	if(m_pDateAdministered->GetStatus() == 1) {
		data.m_varDateAdministered = COleVariant(COleDateTime(m_pDateAdministered->GetDateTime()));
	}
	else {
		data.m_varDateAdministered = g_cvarNull;
	}
	if(m_pExpDate->GetStatus() == 1) {
		data.m_varExpDate = COleVariant(COleDateTime(m_pExpDate->GetDateTime()));
	}
	else {
		data.m_varExpDate = g_cvarNull;
	}

	// (d.singleton 2013-08-23 15:37) - PLID 58274 - add ordering provider to the immunization dlg
	data.SetOrderingProvider(m_pOrderingProviderList->CurSel->GetValue(epcID), VarString(m_pOrderingProviderList->CurSel->GetValue(epcName)));

	// (a.walling 2013-11-13 13:34) - PLID 59464 - Administering provider
	data.SetAdministeringProvider(m_pAdministeringProviderList->CurSel->GetValue(epcID), VarString(m_pAdministeringProviderList->CurSel->GetValue(epcName)));

	data.m_varAdminNotesCode = m_pAdminNotes->CurSel ? m_pAdminNotes->CurSel->Value[eanID] : g_cvarNull;

	// (d.singleton 2013-07-11 12:33) - PLID 57515 - Immunizations need to track an "Administrative Notes"
	GetDlgItemText(IDC_EDIT_ADMINISTRATIVE_NOTES, data.m_strAdminNotes);
	if(data.m_strAdminNotes.IsEmpty() && m_pAdminNotes->CurSel != NULL) {
		data.m_strAdminNotes == VarString(m_pAdminNotes->CurSel->GetValue(eanName));
	}

	// (d.singleton 2013-10-02 12:06) - PLID 58843 - Need to add snomed code selection to immunization dialog for evidence of immunity.
	// (a.walling 2013-11-11 09:59) - PLID 58843 - Using pre-defined hl7 subset for snomed immunity codes; also using variant for the code directly rather than an ID
	data.SetSnomedImmunity(m_pEvidenceImmunity->CurSel->GetValue(eeiCode), VarString(m_pEvidenceImmunity->CurSel->GetValue(eeiName)), VarString(m_pEvidenceImmunity->CurSel->GetValue(eeiDescription)));

	// (d.singleton 2013-10-21 17:30) - PLID 59133 - need check boxes for Immunization Refused
	data.SetRefusalReason(m_pRefusalReason->CurSel->Value[errCode], VarString(m_pRefusalReason->CurSel->Value[errName]));

	// (a.walling 2013-11-11 11:33) - PLID 58781 - immunization financial class / VFC eligibility
	data.SetFinancialClass(m_pFinancialClass->CurSel->Value[efcCode], VarString(m_pFinancialClass->CurSel->Value[efcName]));
	

	// (a.walling 2013-11-11 14:43) - PLID 59414 - Immunizations - Vaccine Information Statements
	{
		std::vector<Nx::HL7::VIS> vis;

		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pVISList->GetFirstRow(); pRow; pRow = pRow->GetNextRow()) {
			vis.push_back(Nx::HL7::VIS(
				VarString(pRow->Value[evcCVX]),
				AsDateTime(pRow->Value[evcPresented]),
				AsDateTime(pRow->Value[evcPublished])
			));
		}

		data.SetVIS(vis);
	}

	return data;
}

void CEditImmunizationDlg::SelChosenAdministrativeNotesList(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			CString strAdminText = VarString(pRow->GetValue(eanName));
			//set the text box
			SetDlgItemText(IDC_EDIT_ADMINISTRATIVE_NOTES, strAdminText);
		}
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2013-11-11 14:43) - PLID 59414 - Immunizations - Vaccine Information Statements
void CEditImmunizationDlg::OnBnClickedAddVIS()
{
	try {		
		IRowSettingsPtr pRow = m_pVISList->GetNewRow();

		pRow->Value[evcPresented] = _variant_t(AsDateNoTime(COleDateTime::GetCurrentTime()), VT_DATE);
		pRow->Value[evcPublished] = g_cvarNull;
		pRow->Value[evcCVX] = g_cvarNull;

		m_pVISList->StartEditing(
			m_pVISList->AddRowAtEnd(pRow, NULL)
			, evcCVX
		);
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2013-11-11 14:43) - PLID 59414 - Immunizations - Vaccine Information Statements
void CEditImmunizationDlg::LoadImmTypes()
{
	CString strTypes = ";;-1;;";

	ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT CVX, Type FROM ImmunizationsT WHERE CVX IS NOT NULL ORDER BY Type ASC");

	while(!prs->eof) {
		strTypes.AppendFormat("%s\x02%s\x02%s\x02\x03"
			, AdoFldString(prs, "CVX") // (a.walling 2013-11-15 12:44) - PLID 59511 - CVX should be a string column to allow for leading zeros
			, AdoFldString(prs, "Type", "")
			, "1"
		);

		prs->MoveNext();
	}

	m_pVISList->GetColumn(evcCVX)->ComboSource = (LPCTSTR)strTypes;
}

// (a.walling 2013-11-11 14:43) - PLID 59414 - Immunizations - Vaccine Information Statements
bool CEditImmunizationDlg::CheckVISData()
{
#pragma TODO("Verify this stuff with Editing events when we have time")
	{
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pVISList->GetFirstRow(); pRow;) {
			CString presented = AsString(pRow->Value[evcPresented]);
			if (presented.IsEmpty()) {
				m_pVISList->RemoveRow(pRow);
				pRow = m_pVISList->GetFirstRow();
			} else {
				pRow = pRow->GetNextRow();
			}
		}
	}

	for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pVISList->GetFirstRow(); pRow; pRow = pRow->GetNextRow()) {
		if (pRow->Value[evcCVX].vt != VT_BSTR) {
			MessageBox("The selected information statement type is invalid");
			return false;
		}

		CString str;

		str = AsString(pRow->Value[evcPresented]);

		{
			COleDateTime dt;
			if (!dt.ParseDateTime(str, VAR_DATEVALUEONLY)) {
				MessageBox(FormatString("The date value `%s` is invalid", str));
				return false;
			}
		}

		if (pRow->Value[evcPublished].vt == VT_BSTR) {
			str = VarString(pRow->Value[evcPublished], "");
			if (!str.IsEmpty()) {
				COleDateTime dt;
				if (!dt.ParseDateTime(str, VAR_DATEVALUEONLY)) {
					MessageBox(FormatString("The date value `%s` is invalid", str));
					return false;
				}
			}
		}
	}

	return true;
}

