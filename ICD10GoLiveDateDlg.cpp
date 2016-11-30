// ICD10GoLiveDateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "ICD10GoLiveDateDlg.h"
#include "AuditTrail.h"
#include <NxPracticeSharedLib/SharedDiagnosisUtils.h>

using namespace ADODB; 
using namespace NXDATALIST2Lib; 

// CICD10GoLiveDateDlg dialog
// (b.spivey - March 6th, 2014) - PLID 61196 - Created. 

IMPLEMENT_DYNAMIC(CICD10GoLiveDateDlg, CNxDialog)

CICD10GoLiveDateDlg::CICD10GoLiveDateDlg(long nInsCoID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CICD10GoLiveDateDlg::IDD, pParent)
{
	m_nLoadingInsCoID = nInsCoID;
}

CICD10GoLiveDateDlg::~CICD10GoLiveDateDlg()
{
}

// (b.spivey, March 12th, 2014) - PLID 61196 - 
enum ICD10GoLiveInsList
{
	glilPersonID = 0,
	glilSelected, 
	glilDate,
	glilName,
	glilAddress1,
	glilAddress2,
	glilCity,
	glilState,
	glilZip,
};

void CICD10GoLiveDateDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_nxbOK); 
	DDX_Control(pDX, IDC_ICD10_GO_LIVE_CHECK, m_checkGoLiveDate);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel); 
	DDX_Control(pDX, IDC_ICD10_GO_LIVE_DATEPICKER, m_dtICD10GoLiveDate);
}


BEGIN_MESSAGE_MAP(CICD10GoLiveDateDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_ICD10_GO_LIVE_CHECK, OnGoLiveDateChecked)
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CICD10GoLiveDateDlg, CNxDialog)
	ON_EVENT(CICD10GoLiveDateDlg, IDC_UNSELECTED_INS_LIST, 18 /* RequeryFinished */, OnRequeryFinishedInsList, VTS_I2)
END_EVENTSINK_MAP()

BOOL CICD10GoLiveDateDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		//Set this in the interface. 
		// (b.spivey -- April 2nd, 2014) - PLID 61632 - Updated to 2015 because of 11th hour ICD-10 delay. 
		// (r.gonet 04/03/2014) - PLID 61640 - Needed this default elsewhere. Moved into a library function.
		COleDateTime dtDefault = GetDefaultICD10GoLiveDate(); 
		COleDateTime dtLoadingDate = dtDefault; 


		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CLOSE);

		m_dlInsList = BindNxDataList2Ctrl(IDC_UNSELECTED_INS_LIST, true);

		//If we have a loading insurance co, lets set that as the default go-live and 
		if (m_nLoadingInsCoID > 0) {

			_RecordsetPtr prs = CreateParamRecordset("SELECT ICD10GoLiveDate AS GoLive FROM InsuranceCoT WHERE PersonID = {INT}", m_nLoadingInsCoID);
			if (!prs->eof) {
				//Load the default values. 
				dtLoadingDate = AdoFldDateTime(prs->Fields, "GoLive", dtDefault); 
				IRowSettingsPtr pRow = m_dlInsList->SetSelByColumn(glilPersonID, m_nLoadingInsCoID); 
				_variant_t varBool(VARIANT_TRUE, VT_BOOL);
				pRow->PutValue(glilSelected, varBool); 
				m_checkGoLiveDate.SetCheck(TRUE); 
			}
			else {
				m_checkGoLiveDate.SetCheck(FALSE); 
			}
		}

		//Set this in the interface. 
		m_dtICD10GoLiveDate.SetValue(dtLoadingDate);

	} NxCatchAll(__FUNCTION__);

	return TRUE; 
}

//OnOK is really apply. 
void CICD10GoLiveDateDlg::OnOK() 
{
	//Begin auditing. 
	long nAuditTransactionID = -1; 

	try {

		//Default to null, if we have a value then we'll put it in this variant. 
		_variant_t vtNewDate = g_cvarNull;
		if(m_checkGoLiveDate.GetCheck()) {
			vtNewDate = _variant_t(VarDateTime(m_dtICD10GoLiveDate.GetValue(), g_cvarNull), VT_DATE);
		}

		//Use every row in this list. 
		IRowSettingsPtr pRow = m_dlInsList->GetFirstRow();
		if (!pRow) {
			RollbackAuditTransaction(nAuditTransactionID); 
			return;
		}

		CArray<long, long> aryIDs; 
		while (pRow) {
			_variant_t varBool(pRow->GetValue(glilSelected)); 

			if (VarBool(varBool, FALSE)) {
				aryIDs.Add(VarLong(pRow->GetValue(glilPersonID), -1));
				varBool = FALSE;
				pRow->PutValue(glilSelected, varBool);
				pRow->PutValue(glilDate, vtNewDate); 
			}
			pRow = pRow->GetNextRow(); 
		}

		if (!(aryIDs.GetCount() > 0)) {
			return;
		}

		nAuditTransactionID = BeginAuditTransaction();

		//Select only that which will change. 
		_RecordsetPtr prs = CreateParamRecordset("SELECT * FROM InsuranceCoT "
			"WHERE PersonID IN ({INTARRAY}) "
			"AND PersonID IS NOT NULL ", aryIDs); 

		 
		CSqlFragment sql(""); 

		while (!prs->eof) {

			long nID = AdoFldLong(prs->Fields, "PersonID", -1);
			CString strName = AdoFldString(prs->Fields, "Name", ""); 
			COleDateTime dtOldDate = AdoFldDateTime(prs->Fields, "ICD10GoLiveDate", g_cdtInvalid);
			_variant_t vtOldDate = g_cvarNull;

			//If there is a valid value, lets set it. 
			if(dtOldDate.GetStatus() != COleDateTime::invalid) {
				vtOldDate = _variant_t(dtOldDate, VT_DATE);
			}

			//Update the insurances in a batch. 
			if (nID > 0 && (vtNewDate != vtOldDate)) {
				sql += CSqlFragment("UPDATE InsuranceCoT "
					"SET ICD10GoLiveDate = {VT_DATE} "
					"WHERE PersonID = {INT} ", vtNewDate, nID); 

				//if null, say none. 
				CString strOldval = (vtOldDate == g_cvarNull ? "< None >" : FormatDateTimeForInterface(VarDateTime(vtOldDate)));
				CString strNewVal = (vtNewDate == g_cvarNull ? "< None >" : FormatDateTimeForInterface(VarDateTime(vtNewDate)));

				//Finally, audit. 
				AuditEvent(-1, strName, nAuditTransactionID, aeiInsCoICD10GoLiveDate, 
					nID, strOldval, 
					strNewVal, 
					aepMedium, aetChanged);

			}

			prs->MoveNext(); 
		}

		ExecuteParamSql("{SQL} ", sql); 
		CommitAuditTransaction(nAuditTransactionID);

		m_dlInsList->Requery(); 

		//CDialog::OnOK(); 
		
	} NxCatchAllCall(__FUNCTION__, 
		if(nAuditTransactionID != -1) { 
			RollbackAuditTransaction(nAuditTransactionID); 
		}
	);
}


//cancel is really close. 
void CICD10GoLiveDateDlg::OnCancel()
{
	try {
		CNxDialog::OnCancel(); 
	} NxCatchAll(__FUNCTION__); 
}

void CICD10GoLiveDateDlg::OnGoLiveDateChecked() 
{
	try {
		if (!m_checkGoLiveDate.GetCheck()) {

			//we have to warn them of the repercussions of their actions!
			DontShowMeAgain(this, "By disabling the date you acknowledge that ICD-10 codes WILL NOT be sent on claims "
				"for these insurance companies for any reason. ", "ICD10GoLiveDateDlg", "ICD-10 Go Live Date", 
				FALSE, FALSE, FALSE); 

			m_dtICD10GoLiveDate.EnableWindow(FALSE);
		}
		else {
			m_dtICD10GoLiveDate.EnableWindow(TRUE); 
		}
	} NxCatchAll(__FUNCTION__); 
}

// (b.spivey, March 12th, 2014) - PLID 61196 - set the column size so the dialog looks decent. 
// (b.spivey, March 14th, 2014) - PLID 61196 - Just sort the list. 
// (b.spivey, March 18th, 2014) - PLID 61196 - just make sure the current selection is in view. 
void CICD10GoLiveDateDlg::OnRequeryFinishedInsList(short nFlags)
{
	try {
		IRowSettingsPtr pRow = m_dlInsList->CurSel;
		if (pRow) {
			m_dlInsList->EnsureRowInView(pRow);
		}
	} NxCatchAll(__FUNCTION__); 
}