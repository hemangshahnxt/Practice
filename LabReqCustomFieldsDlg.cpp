// (r.gonet 10/16/2011) - PLID 45968 - Dialog that allows the configuration of which lab custom fields will go on a given custom lab request report.

// LabReqCustomFieldsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "LabReqCustomFieldsDlg.h"
#include "Reports.h"
#include "ReportInfo.h"

using namespace ADODB;

// CLabReqCustomFieldsDlg dialog

IMPLEMENT_DYNAMIC(CLabReqCustomFieldsDlg, CNxDialog)

// (r.gonet 10/16/2011) - PLID 45968 - Creates a new configuration dialog. nCustomReportNumber is the lab request custom report number being configured.
CLabReqCustomFieldsDlg::CLabReqCustomFieldsDlg(long nCustomReportNumber, CWnd* pParent /*=NULL*/)
	: CNxDialog(CLabReqCustomFieldsDlg::IDD, pParent)
{
	m_nCustomReportNumber = nCustomReportNumber;
	m_bChainingAdds = false;
}

CLabReqCustomFieldsDlg::~CLabReqCustomFieldsDlg()
{
}

void CLabReqCustomFieldsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REQ_CUSTOM_FIELDS_ADD_BTN, m_nxbAdd);
	DDX_Control(pDX, IDC_REQ_CUSTOM_FIELDS_ADD_MULTIPLE_BTN, m_nxbAddMultiple);
	DDX_Control(pDX, IDC_REQ_CUSTOM_FIELDS_REMOVE_BTN, m_nxbRemove);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
}


BEGIN_MESSAGE_MAP(CLabReqCustomFieldsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_REQ_CUSTOM_FIELDS_ADD_BTN, &CLabReqCustomFieldsDlg::OnBnClickedReqCustomFieldsAddBtn)
	ON_BN_CLICKED(IDC_REQ_CUSTOM_FIELDS_ADD_MULTIPLE_BTN, &CLabReqCustomFieldsDlg::OnBnClickedReqCustomFieldsAddMultipleBtn)
	ON_BN_CLICKED(IDC_REQ_CUSTOM_FIELDS_REMOVE_BTN, &CLabReqCustomFieldsDlg::OnBnClickedReqCustomFieldsRemoveBtn)
	ON_BN_CLICKED(IDOK, &CLabReqCustomFieldsDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CLabReqCustomFieldsDlg message handlers

// (r.gonet 10/16/2011) - PLID 45968 - Add a single new report field.
void CLabReqCustomFieldsDlg::OnBnClickedReqCustomFieldsAddBtn()
{
	try {
		StopChaining();
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = AddField();
		// (r.gonet 10/16/2011) - PLID 45968 - Open up the drop down to select a lab custom field so it is apparent what the user must do next.
		m_pFieldsList->StartEditing(pNewRow, lrcfcLabCustomFieldID);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/16/2011) - PLID 45968 - Chain adds multiple rows, one after another, until the user kills the focus on the datalist or an error happens.
void CLabReqCustomFieldsDlg::OnBnClickedReqCustomFieldsAddMultipleBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = AddField();
		// (r.gonet 10/16/2011) - PLID 45968 - Start the mode to add rows one after another.
		StartChaining();
		// (r.gonet 10/16/2011) - PLID 45968 - Open up the drop down to select a lab custom field so it is apparent what the user must do next.
		m_pFieldsList->StartEditing(pNewRow, lrcfcLabCustomFieldID);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/16/2011) - PLID 45968 - Remove the report field associated with the currently selected row.
void CLabReqCustomFieldsDlg::OnBnClickedReqCustomFieldsRemoveBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldsList->CurSel;
		if(!pRow) {
			MessageBox("You must select a row before attempting to remove it.", "Error", MB_ICONERROR|MB_OK);
			return;
		}
		
		long nID = pRow->GetValue(lrcfcID);
		if(nID != -1) {
			// (r.gonet 10/16/2011) - PLID 45968 - We only need to delete this from the database if it has been saved.
			m_aryDeletedFields.Add(nID);
		}
		m_pFieldsList->RemoveRow(pRow);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/16/2011) - PLID 45968 - Save the custom report fields to the database and close the dialog.
void CLabReqCustomFieldsDlg::OnBnClickedOk()
{
	try {
		CParamSqlBatch sqlBatch;
		// (r.gonet 10/16/2011) - PLID 45968 - Go through all of the report fields desired to be on the report.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldsList->GetFirstRow();
		while(pRow) {
			long nID = VarLong(pRow->GetValue(lrcfcID), -1);
			long nLabCustomFieldID = VarLong(pRow->GetValue(lrcfcLabCustomFieldID), -1);
			CString strReportFieldName = VarString(pRow->GetValue(lrcfcReportFieldName), "");
			BOOL bModified = VarBool(pRow->GetValue(lrcfcModified), FALSE);

			if(nLabCustomFieldID == -1) {
				MessageBox("All report fields must have a lab custom field associated with them before saving.", "Error", MB_ICONERROR|MB_OK);
				return;
			}
			if(strReportFieldName == "") {
				MessageBox("All report fields must have a name.", "Error", MB_ICONERROR|MB_OK);
				return;
			}

			if(nID == -1) {
				// (r.gonet 10/16/2011) - PLID 45968 - This report field is new, so always insert it.
				sqlBatch.Add(
					"INSERT INTO LabReqCustomFieldsT (CustomReportNumber, LabCustomFieldID, ReportFieldName) "
					"VALUES "
					"({INT}, {INT}, {STRING}); ",
					m_nCustomReportNumber, nLabCustomFieldID, strReportFieldName);
			} else if(bModified) {
				// (r.gonet 10/16/2011) - PLID 45968 - Only update a report field if it is not new and has been modified.
				sqlBatch.Add(
					"UPDATE LabReqCustomFieldsT SET "
					"	LabCustomFieldID = {INT}, "
					"	ReportFieldName = {STRING} "
					"WHERE ID = {INT}; ",
					nLabCustomFieldID, strReportFieldName, nID);
			}

			pRow = pRow->GetNextRow();
		}

		// (r.gonet 10/16/2011) - PLID 45968 - The user may have deleted some report fields, so commit those changes here.
		for(int i = 0; i < m_aryDeletedFields.GetSize(); i++) {
			sqlBatch.Add(
				"DELETE FROM LabReqCustomFieldsT "
				"WHERE ID = {INT}; ",
				m_aryDeletedFields[i]);
		}

		if(!sqlBatch.IsEmpty()) {
			sqlBatch.Execute(GetRemoteData());

			// (r.gonet 10/16/2011) - PLID 45968 - Now we need to sync up the actual custom report's TTX file to get these fields available 
			//  on the report, or to remove them from the report if they were deleted.
			MessageBox("The report must now be verified. Practice will now attempt to verify the report.");
			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(658)]);
			infReport.nDefaultCustomReport = m_nCustomReportNumber;
			BOOL bVerified = false;
			try {
				bVerified = infReport.VerifyCustomReport();
			} catch(...) { }
			if(!bVerified) {
				MessageBox("Practice was not able to complete verification of the report. You will need to do this manually by editing the report and clicking Verify.", "Error", MB_OK|MB_ICONERROR);
			}
		}
	} NxCatchAll(__FUNCTION__);
	OnOK();
}

BOOL CLabReqCustomFieldsDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		m_pFieldsList = BindNxDataList2Ctrl(this, IDC_REQ_CUSTOM_FIELDS_LIST, GetRemoteData(), false);
		m_nxbAdd.AutoSet(NXB_NEW);
		m_nxbAddMultiple.AutoSet(NXB_NEW);
		m_nxbRemove.AutoSet(NXB_DELETE);
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		// (r.gonet 10/16/2011) - PLID 45968 - Only display the custom report fields for the current custom report.
		m_pFieldsList->WhereClause = _bstr_t(FormatString("CustomReportNumber = %li ", m_nCustomReportNumber));
		m_pFieldsList->Requery();
		m_pFieldsList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
BEGIN_EVENTSINK_MAP(CLabReqCustomFieldsDlg, CNxDialog)
	ON_EVENT(CLabReqCustomFieldsDlg, IDC_REQ_CUSTOM_FIELDS_LIST, 10, CLabReqCustomFieldsDlg::EditingFinishedReqCustomFieldsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CLabReqCustomFieldsDlg, IDC_REQ_CUSTOM_FIELDS_LIST, 9, CLabReqCustomFieldsDlg::EditingFinishingReqCustomFieldsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

void CLabReqCustomFieldsDlg::EditingFinishedReqCustomFieldsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		if(!bCommit) {
			// (r.gonet 10/16/2011) - PLID 45968 - We have prevented the user from saving their new value, so there must have been an error. In that case, stop chaining new rows.
			StopChaining();
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(!pRow) {
			// (r.gonet 10/16/2011) - PLID 45968 - I don't know what happened here. I don't think this is possible, but if it is, we handle it.
			StopChaining();
			return;
		}

		if(nCol == lrcfcLabCustomFieldID && VarLong(varOldValue) != VarLong(varNewValue)) {
			// (r.gonet 10/16/2011) - PLID 45968 - Only if the value actually changed does this report field count as modified
			pRow->PutValue(lrcfcModified, g_cvarTrue);
			if(VarString(pRow->GetValue(lrcfcReportFieldName), "") == "") {
				// (r.gonet 10/16/2011) - PLID 45968 - We need a field name now, so prod the user to do that.
				m_pFieldsList->StartEditing(pRow, lrcfcReportFieldName);
			}
		} else if(nCol == lrcfcReportFieldName && VarString(varOldValue) != VarString(varNewValue)) {
			pRow->PutValue(lrcfcModified, g_cvarTrue);
			if(m_bChainingAdds) {
				// (r.gonet 10/16/2011) - PLID 45968 - If we are continuously adding new rows, then add a new row since this one is completed.
				NXDATALIST2Lib::IRowSettingsPtr pNewRow = AddField();
				m_pFieldsList->StartEditing(pNewRow, lrcfcLabCustomFieldID);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CLabReqCustomFieldsDlg::EditingFinishingReqCustomFieldsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		if(*pbCommit == FALSE) {
			// (r.gonet 10/16/2011) - PLID 45968 - Somehow the edit was uncommitted before it was even finished...
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(!pRow) {
			*pbCommit = FALSE;
			return;
		}

		if(nCol == lrcfcLabCustomFieldID && VarLong(pvarNewValue, -1) == -1) {
			// (r.gonet 10/16/2011) - PLID 45968 - We do not let the user attempt to save a null lab custom field.
			*pbCommit = FALSE;
		}
		else if(nCol == lrcfcReportFieldName) {
			CString strRowReportFieldName = VarString(*pvarNewValue, "");
			// (r.gonet 10/16/2011) - PLID 45968 - Validate the input. It must be alphanumeric + _
			for(int i = 0; i < strRowReportFieldName.GetLength(); i++) {
				if(!iswalnum(strRowReportFieldName[i]) && strRowReportFieldName[i] != '_') {
					MessageBox("Report field names must have only letters and numbers in them. The underscore is allowed as well.", "Error", MB_ICONERROR|MB_OK);
					*pbCommit = FALSE;
					return;
				}
			}

			// (r.gonet 10/16/2011) - PLID 45968 - Report field names must be unique to the report.
			NXDATALIST2Lib::IRowSettingsPtr pRowIter = m_pFieldsList->GetFirstRow();
			while(pRowIter) {
				if(pRow != pRowIter) {
					CString strRowIterReportFieldName = VarString(pRowIter->GetValue(lrcfcReportFieldName), "");
					if(strRowReportFieldName == strRowIterReportFieldName) {
						MessageBox("You cannot have multiple report fields with the same name.", "Error", MB_ICONERROR|MB_OK);
						*pbCommit = FALSE;
						return;
					}
				}

				pRowIter = pRowIter->GetNextRow();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/16/2011) - PLID 45968 - Adds a new row to the datalist and returns it.
NXDATALIST2Lib::IRowSettingsPtr CLabReqCustomFieldsDlg::AddField()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFieldsList->GetNewRow();
	pRow->PutValue(lrcfcID, _variant_t((long)-1, VT_I4));
	pRow->PutValue(lrcfcLabCustomFieldID, _variant_t((long)-1, VT_I4));
	pRow->PutValue(lrcfcReportFieldName, _variant_t(""));
	pRow->PutValue(lrcfcModified, g_cvarTrue);
	m_pFieldsList->AddRowAtEnd(pRow, NULL);
	m_pFieldsList->EnsureRowInView(pRow);
	m_pLastAddedRow = pRow;
	return pRow;
}

// (r.gonet 10/16/2011) - PLID 45968 - Starts the mode of adding new rows after each one is completed.
void CLabReqCustomFieldsDlg::StartChaining()
{
	m_bChainingAdds = true;
}

// (r.gonet 10/16/2011) - PLID 45968 - Stops the mode of adding new rows after each one is completed. If the last entered row
//  does not have a lab custom field ID assigned to it, it is deleted.
void CLabReqCustomFieldsDlg::StopChaining()
{
	if(m_bChainingAdds) {
		m_bChainingAdds = false;
		if(m_pLastAddedRow && VarLong(m_pLastAddedRow->GetValue(lrcfcLabCustomFieldID), -1) == -1) {
			m_pFieldsList->RemoveRow(m_pLastAddedRow);
		}
	}
	m_pLastAddedRow = NULL;
}