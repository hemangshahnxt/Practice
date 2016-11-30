// IntegrationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "IntegrationDlg.h"
#include "EditComboBox.h"
#include "TaskEditDlg.h"
#include "boost/date_time/posix_time/posix_time.hpp"

// (f.dinatale 2010-07-07) - PLID 39527 - Added the Integration Info Dialog
// CIntegrationDlg dialog

IMPLEMENT_DYNAMIC(CIntegrationDlg, CNxDialog)

CIntegrationDlg::CIntegrationDlg(CWnd* pParent /*=NULL*/, long nClientID /*=-1*/)
: CNxDialog(CIntegrationDlg::IDD, pParent), m_nClientID(nClientID)
{
}

CIntegrationDlg::~CIntegrationDlg()
{
}

void CIntegrationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INTEGRATIONADD, m_nxbAdd);
	DDX_Control(pDX, IDC_INTEGRATIONCLOSE, m_nxbClose);
	DDX_Control(pDX, IDC_INTEGRATIONREMOVE, m_nxbDelete);
	DDX_Control(pDX, IDC_INTEGRATIONEDITBILLTO, m_nxbEditBillTo);
	DDX_Control(pDX, IDC_INTEGRATIONEDITLABTYPES, m_nxbEditLabTypes);
	DDX_Control(pDX, IDC_INTEGRATION_ADDTODO, m_nxbAddToDo);
}

BEGIN_MESSAGE_MAP(CIntegrationDlg, CNxDialog)
	ON_BN_CLICKED(IDC_INTEGRATIONADD, &CIntegrationDlg::OnAdd)
	ON_BN_CLICKED(IDC_INTEGRATIONREMOVE, &CIntegrationDlg::OnRemove)
	ON_BN_CLICKED(IDC_INTEGRATIONCLOSE, &CIntegrationDlg::OnClose)
	ON_BN_CLICKED(IDC_INTEGRATIONEDITBILLTO, &CIntegrationDlg::OnEditIntegrationBillTo)
	ON_BN_CLICKED(IDC_INTEGRATIONEDITLABTYPES, &CIntegrationDlg::OnEditLabTypes)
	ON_BN_CLICKED(IDC_INTEGRATION_ADDTODO, &CIntegrationDlg::OnIntegrationAddtodo)
	ON_BN_CLICKED(IDC_INTEGRATIONGOTOCLIENT, &CIntegrationDlg::OnGoToClient)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CIntegrationDlg, CNxDialog)
	ON_EVENT(CIntegrationDlg, IDC_INTEGRATIONINFODL, 28, CIntegrationDlg::OnCurSelWasSet, VTS_NONE)
	ON_EVENT(CIntegrationDlg, IDC_INTEGRATIONINFODL, 9, CIntegrationDlg::OnEditingFinishing, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CIntegrationDlg, IDC_INTEGRATIONINFODL, 10, CIntegrationDlg::EditingFinishedIntegration, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CIntegrationDlg, IDC_INTEGRATIONINFODL, 4, CIntegrationDlg::LButtonDownIntegration, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// CIntegrationDlg message handlers
BOOL CIntegrationDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{
		//(j.deskurakis 2013-01-24) - PLID 53151 - add close and billTo
		// (f.dinatale 2010-07-07) - PLID 39527
		// Set up the Integration window to look like the rest of the Patients Module.
		m_nxbAdd.AutoSet(NXB_NEW);
		m_nxbDelete.AutoSet(NXB_DELETE);
		m_nxbClose.AutoSet(NXB_CLOSE);
		m_nxbEditBillTo.AutoSet(NXB_MODIFY);
		m_nxbEditLabTypes.AutoSet(NXB_MODIFY);
		m_nxbAddToDo.AutoSet(NXB_NEW);
		((CNxColor*) GetDlgItem(IDC_NXCOLOR_INTEGRATION))->SetColor((GetNxColor(GNC_PATIENT_STATUS, 1)));

		// Get access to the Integration datalist.
		m_dlcIntegration = BindNxDataList2Ctrl(IDC_INTEGRATIONINFODL, false);

		//(j.deskurakis 2013-01-24) - PLID 53151 - created new view for use here with respect to new functionality
		CString strFrom;
		strFrom.Format("( "
			"Select * from LabsInternalV "

			"where ClientId = %li) AS Integrations", m_nClientID);
		m_dlcIntegration->FromClause = _bstr_t(strFrom);

		// Requery.
		m_dlcIntegration->Requery();
	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

// (f.dinatale 2010-07-07) - PLID 39527
//(j.deskurakis 2013-01-24) - PLID 53151 - added validation, set additional columns
void CIntegrationDlg::OnAdd()
{
	try{
		CParamSqlBatch SQLBatch;
		CString strNewIncident;
		BOOL bRetry = TRUE;

		while (bRetry) {

			int nResult = InputBoxLimited(NULL, "Enter a new incident number", strNewIncident, "",10,false,false,NULL);
			if (nResult == IDOK) {
				// Validate it

				strNewIncident.TrimLeft();
				strNewIncident.TrimRight();
				long nNewIncident = atol(strNewIncident);
				//ensure input is int 
				if (nNewIncident > 0) {
					//check for no input 
					if (!strNewIncident.IsEmpty()) {
						//ensure no double entry of incident numbers & incident exists
						ADODB::_RecordsetPtr rsChecks = CreateParamRecordset(
							"SELECT TOP 1 1 FROM IssueT WHERE ID = {INT} and ClientID = {INT}; \r\n"
							"SELECT TOP 1 1 FROM LabsInternalV WHERE IncidentID = {INT}; ", 
							nNewIncident, m_nClientID, nNewIncident);
						//ensure no double entry of incident numbers
						if(!rsChecks->eof){
							rsChecks = rsChecks->NextRecordset(NULL);
							//ensure incident exists
							if(rsChecks->eof){
								rsChecks->Close();

								SQLBatch.Add("INSERT INTO IntegrationsT (IncidentID, ClientId) VALUES ({INT}, {INT}) ", nNewIncident, m_nClientID);
								SQLBatch.Execute(GetRemoteData());
								m_dlcIntegration->Requery();
								bRetry = FALSE;

								return;
							} else {
								// then it already exists in LabsInternalV
								AfxMessageBox("The Incident ID has already been input.  Please try again.", NULL, MB_ICONINFORMATION|MB_OK);
							}
						} else {
							// then incident doesnt exist
							AfxMessageBox("This is not a valid incident number or does not match this client. Please enter a new incident number.", NULL, MB_ICONINFORMATION|MB_OK);
						}
					} else {
						// Nothing entered
						AfxMessageBox("You must enter a valid incident number. Entry is blank.", NULL, MB_ICONINFORMATION|MB_OK);
					}
				} else {
					// Input can't be converted to int 
					AfxMessageBox("You must enter a valid incident number. Entry cannot be converted to an integer.", NULL, MB_ICONINFORMATION|MB_OK);
				}
			} else {
				// The user canceled
				bRetry = FALSE;
				return;
			}
		}

	}NxCatchAll(__FUNCTION__);	
}

// (f.dinatale 2010-07-07) - PLID 39527
void CIntegrationDlg::OnRemove()
{
	try{
		//(j.deskurakis 2013-01-24) - PLID 53151 - warning for row deletion
		if (IDNO == MessageBox("You are about to delete the selected row. Do you want to continue?", NULL, MB_ICONQUESTION | MB_YESNO)) {
			return;
		}
		//(j.deskurakis 2013-01-24) - PLID 53151 - moved deletion from table to this function from OnSave
		// Get the currently selected row because that is the one we want to remove.
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		if(m_dlcIntegration->GetCurSel() != NULL){
			pRow = m_dlcIntegration->GetCurSel();
			long nRowID = VarLong(pRow->GetValue(dlcIntegrationID), -1);
			CParamSqlBatch SQLBatch;
			SQLBatch.Add("DELETE FROM IntegrationsT WHERE ID = {INT}", nRowID);
			// Execute the batch.
			SQLBatch.Execute(GetRemoteData());
			m_dlcIntegration->RemoveRow(pRow);
		}

	}NxCatchAll(__FUNCTION__);
}

// (f.dinatale 2010-07-07) - PLID 39527
void CIntegrationDlg::OnClose()
{
	try{
		//(j.deskurakis 2013-01-24) - PLID 53151 - eliminated save on close - this saves on the fly
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

// (f.dinatale 2010-08-13) - PLID 39527 - Refactored the code to fix a logical error when editing companies, links, etc.
void CIntegrationDlg::SaveData(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try{
		// Set up a parameterized batch to handle the saving.
		CParamSqlBatch SQLBatch;
		CString strNotes;

		//(j.deskurakis 2013-01-24) - PLID 53151 - altered to handle one row at a time
		//(j.deskurakis 2013-01-24) - PLID 53151 - updated to handle new columns 
		long nID = VarLong(pRow->GetValue(dlcIntegrationID), -1);
		// Store the notes in a string (exception otherwise)

		long nIncidentID = pRow->GetValue(dlcIncidentID);
		_variant_t vtAssigned = pRow->GetValue(dlcIntegrationAssigned);
		_variant_t vtLab = pRow->GetValue(dlcIntegrationInterface);
		_variant_t vtLabType = pRow->GetValue(dlcIntegrationLabType);
		_variant_t vtIntegrationStatus = pRow->GetValue(dlcIntegrationStatus);
		_variant_t vtIntegrationPO = pRow->GetValue(dlcIntegrationPO);
		_variant_t vtIntegrationPaid = pRow->GetValue(dlcIntegrationPaid);
		_variant_t vtIntegrationGoLive = pRow->GetValue(dlcIntegrationGoLive);
		_variant_t vtIntegrationTrained = pRow->GetValue(dlcIntegrationTrained);
		_variant_t vtNotes = pRow->GetValue(dlcIntegrationNotes);
		_variant_t vtIntegrationStart = pRow->GetValue(dlcIntegrationStart);
		_variant_t vtExpDate = pRow->GetValue(dlcIntegrationExpiration);


		//(j.deskurakis 2013-01-24) - PLID 53151 - altered to handle new columns
		// Check if it's an already existing row or a new row.
		switch(nID){
		case(-1):
			SQLBatch.Add("INSERT INTO IntegrationsT (IncidentID, ExpirationDate, IntegrationLabType, ClientId, IntegrationsBillTo, Notes, StartDate) "
				"VALUES ({INT}, dbo.AsDateNoTime({VT_DATE}), {VT_I4}, {INT}, {VT_I4}, {VT_BSTR}, dbo.AsDateNoTime({VT_DATE})) ", 
				nIncidentID, vtExpDate, vtLabType, m_nClientID, vtLab, vtNotes, vtIntegrationStart);
			break;
		default:
			SQLBatch.Add("UPDATE IntegrationsT SET IncidentID = {INT}, ExpirationDate = dbo.AsDateNoTime({VT_DATE}), IntegrationLabType = {VT_I4}, ClientId = {INT}, "
				"IntegrationsBillTo = {VT_I4}, Notes = {VT_BSTR}, StartDate = dbo.AsDateNoTime({VT_DATE}) WHERE ID = {INT} ", 
				nIncidentID, vtExpDate, vtLabType, m_nClientID, vtLab, vtNotes, vtIntegrationStart, nID);
			break;
		}

		// Execute the batch and close the window.
		SQLBatch.Execute(GetRemoteData());
	}NxCatchAll(__FUNCTION__);
}

//(j.deskurakis 2013-01-24) - PLID 53151 - removed warning; no longer necessary
// (f.dinatale 2010-07-07) - PLID 39527
void CIntegrationDlg::OnEditIntegrationBillTo()
{
	try{
		EditTables(eIntegrationBillToT);
	}NxCatchAll(__FUNCTION__);
}

//(j.deskurakis 2013-01-24) - PLID 53151 - removed warning; no longer necessary
// (f.dinatale 2010-07-07) - PLID 39527
void CIntegrationDlg::OnEditLabTypes()
{
	try{
		EditTables(eIntegrationLabTypesT);
	}NxCatchAll(__FUNCTION__);
}

//(j.deskurakis 2013-01-24) - PLID 53151 - modified
// (f.dinatale 2010-07-07) - PLID 39527 - Function that modifies the table passed in.
void CIntegrationDlg::EditTables(long tableEnum /*=-1*/)
{
	try{
		// Check which table we are modifying and set the caption and list ID to their appropriate values.
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		switch(tableEnum)
		{
		case(eIntegrationBillToT):
			CEditComboBox(this, 72, "Edit Bill To").DoModal();
			break;
		case(eIntegrationLabTypesT):
			CEditComboBox(this, 73, "Edit Integration Types").DoModal();
			break;
		default:
			// Throw an exception if a list is passed in that isn't in the set.
			ThrowNxException("CIntegrationDlg::EditTables : Invalid Integration table specified");
		}

		// Requery the appropriate embedded combobox once the modifications are done.
		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		switch(tableEnum)
		{
		case(eIntegrationBillToT):
			pCol = m_dlcIntegration->GetColumn(dlcIntegrationInterface);
			pCol->ComboSource = pCol->ComboSource;
			break;
		case(eIntegrationLabTypesT):
			pCol = m_dlcIntegration->GetColumn(dlcIntegrationLabType);
			pCol->ComboSource = pCol->ComboSource;
			break;
		}
	}NxCatchAllThrow(__FUNCTION__);
}

// (f.dinatale 2010-07-07) - PLID 39527
void CIntegrationDlg::OnCurSelWasSet()
{
	try{
		// If there is a valid selection, enable the Remove button.
		if(m_dlcIntegration->GetCurSel() != NULL){
			GetDlgItem(IDC_INTEGRATIONREMOVE)->EnableWindow(TRUE);
		} else{
			GetDlgItem(IDC_INTEGRATIONREMOVE)->EnableWindow(FALSE);
		}
	}NxCatchAll(__FUNCTION__);
}

// (f.dinatale 2010-07-07) - PLID 39527
void CIntegrationDlg::OnEditingFinishing(LPDISPATCH lpRow, short nCol, VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try{
		// (f.dinatale 2010-08-17) - PLID 39527 - Check if the key pressed is the ESC key.
		// If so, cancel changes.
		if(*pbCommit == FALSE){
			*pvarNewValue = varOldValue;
			return;
		}

		//(j.deskurakis 2013-01-24) - PLID 53151 - added validation of start date, changed bContinue flag to TRUE for better ease of use
		// If the column edited is  date, make sure the date is valid.
		if((nCol == dlcIntegrationStart) || (nCol == dlcIntegrationExpiration)){

			//warning for invalid date
			if(pvarNewValue->vt != VT_DATE || VarDateTime(*pvarNewValue).GetStatus() != COleDateTime::valid || VarDateTime(*pvarNewValue).GetYear() <= 1899) {
				//null is okay as date.. no warning for that
				if(pvarNewValue->date = VT_NULL){
					pvarNewValue->vt = VT_NULL;
					*pbCommit = TRUE;
					*pbContinue = TRUE; 
					return;

				}
				MsgBox("The text you entered does not correspond to a valid date. \n Your changes will not be saved.");
				*pbCommit = FALSE;
				*pbContinue = TRUE;
			}

		} else {
			// If the column edited is the notes column check the length.
			if(nCol == dlcIntegrationNotes){
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				CString strNotes(strUserEntered);

				if(strNotes.GetLength() > 3000){
					MessageBox("Please reduce your notes down to 3000 characters.", "Note Character Limit Exceeded", MB_ICONEXCLAMATION|MB_OK);
					*pbCommit = FALSE;
					*pbContinue = FALSE;
				}
			}
		}
	}NxCatchAll(__FUNCTION__);

}

// (f.dinatale 2010-07-15) - PLID 39565
void CIntegrationDlg::OnIntegrationAddtodo()
{
	try{
		CTaskEditDlg dlg(this);
		dlg.m_nPersonID = GetActivePatientID();
		//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields
		dlg.m_bIsNew = TRUE;
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}

//(j.deskurakis 2013-01-24) - PLID 53151 - added Control Event to set default expiration date 
void CIntegrationDlg::EditingFinishedIntegration(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{

	try{
		if(!bCommit){
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(!pRow){
			bCommit = FALSE;
			return;
		}

		if(nCol == dlcIntegrationStart){
			// set the support expiration date
			COleDateTime dtStart(pRow->GetValue(dlcIntegrationStart));
			_variant_t vtExp = pRow->GetValue(dlcIntegrationExpiration);

			//if exp is Null
			if (vtExp.vt == 1) {
				// if date is valid
				if(!dtStart.GetStatus()){ 

					//This namespace is for the boost lib --> used to account for leap years
					using namespace boost::gregorian; 
					date d(dtStart.GetYear(),dtStart.GetMonth(), dtStart.GetDay());

					//Increase the current date by 1 year
					d += years(1);
					// (d.thompson 2013-05-15) - PLID 56402 - Per lab mgmt, they now want this to be 1 day less than a year
					d -= days(1);

					//Create a ColeDateTime from the pieces of the boost lib's date
					COleDateTime dtExpirationDateTmp(d.year(),d.month(), d.day() , dtStart.GetHour() ,dtStart.GetMinute(),dtStart.GetSecond());

					//check to move up one year, not 365 days
					pRow->PutValue(dlcIntegrationExpiration, _variant_t(dtExpirationDateTmp, VT_DATE));

				}
			}
		}

		// save to data
		SaveData(pRow);		
	}NxCatchAll(__FUNCTION__);

}

//(j.deskurakis 2013-01-24) - PLID 53151 - added Control Event to handle hyperlink to incident 
void CIntegrationDlg::LButtonDownIntegration(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try{
		if(nCol == dlcIncidentID){
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			if(!pRow) return;

			// Set the columns to their appropriate values.
			long nIncidentID = VarLong(pRow->GetValue(dlcIncidentID), -1);
			if(nIncidentID <= 0){
				return;
			}

			CString strText;
			strText.Format("nxsup://incident/%li", nIncidentID);
			ShellExecute(NULL, NULL, strText, NULL, NULL, SW_SHOW);
		}
	}NxCatchAll(__FUNCTION__);
}

//(j.deskurakis 2013-01-24) - PLID 53151 - added Control Event to handlle new Go To Client button
void CIntegrationDlg::OnGoToClient()
{
	try{
		ADODB::_RecordsetPtr rs = CreateParamRecordset("select userdefinedid from patientst where personid = {INT}", m_nClientID);
		if(rs->eof){
			return;
		}

		long nUserDefinedID = AdoFldLong(rs, "UserDefinedID", -1);

		if(nUserDefinedID <= 0)
			return;

		CString strClient;
		strClient.Format("nxsup://client/%li", nUserDefinedID);
		ShellExecute(NULL, NULL, strClient, NULL, NULL, SW_SHOW);
	}NxCatchAll(__FUNCTION__);

}

