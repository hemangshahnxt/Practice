// EMRVisitTypesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRVisitTypesDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2007-08-16 08:39) - PLID 27054 - created

// (c.haag 2007-09-11 15:55) - PLID 27353 - Changed all message boxes
// and modal dialog invocations to use this dialog as their parent rather
// than the main window

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



enum EMRVisitTypesColumn {
	evtcID = 0,
	evtcName = 1,
	evtcInactive = 2,
};

/////////////////////////////////////////////////////////////////////////////
// CEMRVisitTypesDlg dialog


CEMRVisitTypesDlg::CEMRVisitTypesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRVisitTypesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRVisitTypesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEMRVisitTypesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRVisitTypesDlg)
	DDX_Control(pDX, IDC_CHECK_SHOW_INACTIVE_VISIT_TYPES, m_checkShowInactive);
	DDX_Control(pDX, IDC_BTN_REMOVE_VISIT_TYPE, m_btnDelete);
	DDX_Control(pDX, IDC_BTN_ADD_VISIT_TYPE, m_btnAdd);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRVisitTypesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRVisitTypesDlg)
	ON_BN_CLICKED(IDC_CHECK_SHOW_INACTIVE_VISIT_TYPES, OnCheckShowInactiveVisitTypes)
	ON_BN_CLICKED(IDC_BTN_ADD_VISIT_TYPE, OnBtnAddVisitType)
	ON_BN_CLICKED(IDC_BTN_REMOVE_VISIT_TYPE, OnBtnRemoveVisitType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRVisitTypesDlg message handlers

BOOL CEMRVisitTypesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		// (c.haag 2008-04-30 09:16) - PLID 29840 - NxIconify the Close button
		m_btnOK.AutoSet(NXB_CLOSE);

		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_VisitTypeList = BindNxDataList2Ctrl(this, IDC_EMR_VISIT_TYPES_LIST, GetRemoteData(), true);

	}NxCatchAll("Error in CEMRVisitTypesDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEMRVisitTypesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRVisitTypesDlg)
	ON_EVENT(CEMRVisitTypesDlg, IDC_EMR_VISIT_TYPES_LIST, 8 /* EditingStarting */, OnEditingStartingEmrVisitTypesList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CEMRVisitTypesDlg, IDC_EMR_VISIT_TYPES_LIST, 9 /* EditingFinishing */, OnEditingFinishingEmrVisitTypesList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEMRVisitTypesDlg, IDC_EMR_VISIT_TYPES_LIST, 10 /* EditingFinished */, OnEditingFinishedEmrVisitTypesList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)	
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMRVisitTypesDlg::OnEditingStartingEmrVisitTypesList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL)
			return;

		if(nCol == evtcName) {

			long nID = VarLong(pRow->GetValue(evtcID));

			//if on any saved EMN, disallow editing the name
			_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ID) AS CountRecords FROM EMRMasterT WHERE VisitTypeID = {INT}", nID);
			if(!rs->eof) {
				long nCountRecords = AdoFldLong(rs, "CountRecords",0);
				if(nCountRecords > 0) {
					MessageBox(FormatString("There are %li patient EMN records using this visit type. You may not rename it.\n"
						"You may, however, inactivate this visit.", nCountRecords), "Practice", MB_OK | MB_ICONERROR);					
					*pbContinue = FALSE;
					return;
				}
			}
			rs->Close();
		}

	}NxCatchAll("Error in CEMRVisitTypesDlg::OnEditingFinishingEmrVisitTypesList");
}
void CEMRVisitTypesDlg::OnEditingFinishingEmrVisitTypesList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL)
			return;

		if(nCol == evtcName) {

			long nID = VarLong(pRow->GetValue(evtcID));

			CString strEntered = strUserEntered;
			strEntered.TrimLeft();
			strEntered.TrimRight();

			if(strEntered.IsEmpty()) {
				MessageBox("You may not have a blank visit type.", "Practice", MB_OK | MB_ICONERROR);
				*pbCommit = FALSE;
				return;
			}

			if(VarString(varOldValue, "") != strUserEntered) {
				//(e.lally 2008-10-13) PLID 31665 - Put format string inside ReturnsRecords params.
				// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
				if(ReturnsRecordsParam("SELECT ID FROM EMRVisitTypesT WHERE Name LIKE {STRING} AND ID <> {INT}", strUserEntered, nID)) {
					MessageBox("There is already a visit type with this name (it may be inactive).", "Practice", MB_OK | MB_ICONERROR);
					*pbCommit = FALSE;
					return;
				}

				//if on any saved template, warn about changes to the name
				_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ID) AS CountRecords FROM EMRTemplateT WHERE VisitTypeID = {INT}", nID);
				if(!rs->eof) {
					long nCountRecords = AdoFldLong(rs, "CountRecords",0);
					if(nCountRecords > 0) {
						CString str;
						str.Format("There are %li EMR template records using this visit type. Renaming the visit will cause those templates to use the new visit name.\n"
							"Are you sure you still wish to rename it?", nCountRecords);
						if(IDNO == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
							*pbCommit = FALSE;
							return;
						}
					}
				}
				rs->Close();
			}
		}

	}NxCatchAll("Error in CEMRVisitTypesDlg::OnEditingFinishingEmrVisitTypesList");
}

void CEMRVisitTypesDlg::OnEditingFinishedEmrVisitTypesList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		if(!bCommit)
			return;

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL)
			return;

		long nID = VarLong(pRow->GetValue(evtcID));

		if(nCol == evtcName && VarString(varNewValue, "") != VarString(varOldValue, "")) {

			CString strValue = VarString(varNewValue, "");

			if(strValue.GetLength() > 255) {
				MessageBox("You entered a value greater then the maximum length (255).  The data will be truncated.", "Practice", MB_OK | MB_ICONINFORMATION);
				strValue = strValue.Left(255);
				pRow->PutValue(evtcName, _variant_t(strValue));
			}

			ExecuteSql("UPDATE EMRVisitTypesT SET Name = '%s' WHERE ID = %li", _Q(strValue), nID);

			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiEMRVisitTypeName, nID, VarString(varOldValue, ""), strValue, aepMedium, aetChanged);

			m_VisitTypeList->Sort();
		}
		else if(nCol == evtcInactive) {

			BOOL bInactive = FALSE;
			if(varNewValue.vt == VT_BOOL)
				bInactive = VarBool(varNewValue);
			else if(varNewValue.vt == VT_I4)
				bInactive = VarLong(varNewValue) == 1;

			ExecuteSql("UPDATE EMRVisitTypesT SET Inactive = %li WHERE ID = %li", bInactive ? 1 : 0, nID);

			CString strName = VarString(pRow->GetValue(evtcName));
			CString strOldValue, strNewValue;
			strOldValue.Format("%s - %s", strName, !bInactive ? "Inactive" : "Active");
			strNewValue.Format("%s - %s", strName, bInactive ? "Inactive" : "Active");

			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiEMRVisitTypeInactive, nID, strOldValue, strNewValue, aepMedium, aetChanged);

			//remove from the list if inactivated and we are not showing active items
			if(bInactive && !m_checkShowInactive.GetCheck()) {
				m_VisitTypeList->RemoveRow(pRow);
			}

			m_VisitTypeList->Sort();			
		}

	}NxCatchAll("Error editing visit type.");
}

void CEMRVisitTypesDlg::OnCheckShowInactiveVisitTypes() 
{
	try {

		if(!m_checkShowInactive.GetCheck()) {
			//hide inactive items
			m_VisitTypeList->WhereClause = _bstr_t("Inactive = 0");
		}
		else {
			//show all items
			m_VisitTypeList->WhereClause = _bstr_t("");
		}

		m_VisitTypeList->Requery();

	}NxCatchAll("Error displaying inactive visit types.");
}

void CEMRVisitTypesDlg::OnBtnAddVisitType() 
{
	try {

		CString strItem;
		BOOL bValid = FALSE;
		while(!bValid) {
			if (InputBoxLimitedWithParent(this, "Enter a new EMR visit type:", strItem, "",255,false,false,NULL) == IDOK) {
				strItem.TrimLeft(); strItem.TrimRight();

				if(strItem.IsEmpty()) {
					MessageBox("You may not have a blank visit type.", "Practice", MB_OK | MB_ICONERROR);
					continue;
				}

				// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
				if(ReturnsRecordsParam("SELECT ID FROM EMRVisitTypesT WHERE Name = {STRING}", strItem)) {
					MessageBox("There is already a visit type by that name (it may be inactive). Please enter a new name.", "Practice", MB_OK | MB_ICONERROR);
					continue;
				}
				
				bValid = TRUE;
			}
			else {
				return;
			}
		}

		long nID = NewNumber("EMRVisitTypesT", "ID");		
		ExecuteSql("INSERT INTO EMRVisitTypesT (ID, Name, Inactive) VALUES (%li, '%s', 0)", nID, _Q(strItem));

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiEMRVisitTypeCreated, nID, "", strItem, aepMedium, aetCreated);

		_variant_t varFalse(VARIANT_FALSE, VT_BOOL);

		IRowSettingsPtr pRow = m_VisitTypeList->GetNewRow();
		pRow->PutValue(evtcID, nID);
		pRow->PutValue(evtcName, _bstr_t(strItem));
		pRow->PutValue(evtcInactive, varFalse);
		m_VisitTypeList->AddRowSorted(pRow, NULL);

	}NxCatchAll("Error adding visit type.");
}

void CEMRVisitTypesDlg::OnBtnRemoveVisitType() 
{
	try {

		IRowSettingsPtr pRow = m_VisitTypeList->CurSel;

		if(pRow == NULL) {
			MessageBox("There is no visit type selected.", "Practice", MB_OK | MB_ICONERROR);
			return;
		}

		long nID = VarLong(pRow->GetValue(evtcID));
		CString strName = VarString(pRow->GetValue(evtcName),"");

		//if on any saved EMN or template, disallow deletion
		_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ID) AS CountRecords FROM EMRMasterT WHERE VisitTypeID = {INT} \r\n"
				"SELECT Count(ID) AS CountRecords FROM EMRTemplateT WHERE VisitTypeID = {INT} \r\n"
				"SELECT Count(ID) AS CountRecords FROM EMChecklistsT WHERE VisitTypeID = {INT} ", nID, nID, nID);
		long nEMRMasterRecords = 0;
		long nEMRTemplateRecords = 0;
		long nEMChecklistRecords = 0;
		if(!rs->eof) {
			nEMRMasterRecords = AdoFldLong(rs, "CountRecords",0);
		}
		rs = rs->NextRecordset(NULL);
		if(!rs->eof) {
			nEMRTemplateRecords = AdoFldLong(rs, "CountRecords",0);
		}
		rs = rs->NextRecordset(NULL);
		if(!rs->eof) {
			nEMChecklistRecords = AdoFldLong(rs, "CountRecords",0);
		}
		rs->Close();

		if(nEMRMasterRecords > 0 || nEMRTemplateRecords > 0) {
			MessageBox(FormatString("This visit type is in use on %li patient EMN records and %li EMR template records, and cannot be deleted.\n"
				"You may, however, inactivate this visit.", nEMRMasterRecords, nEMRTemplateRecords), "Practice", MB_OK | MB_ICONERROR);
			return;
		}

		//disallow deleting checklists from here, force them to go to the checklist editor and review it first
		if(nEMChecklistRecords > 0) {
			MessageBox(FormatString("This visit type has an E/M Checklist set up for it, and cannot be deleted without first deleting the checklist.\n"
				"If you wish to delete the checklist, you will need to do so from the checklist editor."), "Practice", MB_OK | MB_ICONERROR);
			return;
		}

		//if we get here, it's ok to delete
		ExecuteSql("DELETE FROM EMRVisitTypesT WHERE ID = %li", nID);

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiEMRVisitTypeDeleted, nID, strName, "<Deleted>", aepMedium, aetDeleted);

		m_VisitTypeList->RemoveRow(pRow);

	}NxCatchAll("Error deleting visit type.");
}
