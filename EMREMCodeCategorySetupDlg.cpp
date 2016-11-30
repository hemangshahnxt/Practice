// EMREMCodeCategorySetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMREMCodeCategorySetupDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2007-08-15 10:09) - PLID 27052 - created

// (j.jones 2013-01-04 14:48) - PLID 28135 - changed all references to say E/M, and not use an ampersand

// (c.haag 2007-09-11 15:55) - PLID 27353 - Changed all message boxes
// and modal dialog invocations to use this dialog as their parent rather
// than the main window

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



enum EMCodeCategoryListColumn {
	emcclcID = 0,
	emcclcDesc = 1,
};

/////////////////////////////////////////////////////////////////////////////
// CEMREMCodeCategorySetupDlg dialog


CEMREMCodeCategorySetupDlg::CEMREMCodeCategorySetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMREMCodeCategorySetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMREMCodeCategorySetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEMREMCodeCategorySetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMREMCodeCategorySetupDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BTN_REMOVE_EM_CATEGORY, m_btnDelete);
	DDX_Control(pDX, IDC_BTN_ADD_EM_CATEGORY, m_btnAdd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMREMCodeCategorySetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMREMCodeCategorySetupDlg)
	ON_BN_CLICKED(IDC_BTN_ADD_EM_CATEGORY, OnBtnAddEmCategory)
	ON_BN_CLICKED(IDC_BTN_REMOVE_EM_CATEGORY, OnBtnRemoveEmCategory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMREMCodeCategorySetupDlg message handlers

BOOL CEMREMCodeCategorySetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		// (c.haag 2008-04-29 17:08) - PLID 29837 - NxIconify the close button
		m_btnClose.AutoSet(NXB_CLOSE);
		
		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_EMCategoryList = BindNxDataList2Ctrl(this, IDC_EM_CODE_CATEGORY_LIST, GetRemoteData(), true);

	}NxCatchAll("Error in CEMREMCodeCategorySetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMREMCodeCategorySetupDlg::OnBtnAddEmCategory() 
{
	try {

		CString strItem;
		BOOL bValid = FALSE;
		while(!bValid) {
			// (j.jones 2013-01-04 14:48) - PLID 28135 - changed to say E/M, and not use an ampersand
			if (InputBoxLimitedWithParent(this, "Enter a new E/M category:", strItem, "",255,false,false,NULL) == IDOK) {
				strItem.TrimLeft(); strItem.TrimRight();

				if(strItem.IsEmpty()) {
					MessageBox("You may not have a blank category name.", "Practice", MB_OK | MB_ICONERROR);
					continue;
				}

				if(ReturnsRecords("SELECT ID FROM EMCodeCategoryT WHERE Name = '%s'", _Q(strItem))) {
					MessageBox("There is already an E/M category by that name. Please enter a new name.", "Practice", MB_OK | MB_ICONERROR);
					continue;
				}
				
				bValid = TRUE;
			}
			else {
				return;
			}
		}

		long nID = NewNumber("EMCodeCategoryT", "ID");		
		ExecuteSql("INSERT INTO EMCodeCategoryT (ID, Name) VALUES (%li, '%s')", nID, _Q(strItem));

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiEMCodeCategoryCreated, nID, "", strItem, aepMedium, aetCreated);

		IRowSettingsPtr pRow = m_EMCategoryList->GetNewRow();
		pRow->PutValue(emcclcID, nID);
		pRow->PutValue(emcclcDesc, _bstr_t(strItem));
		m_EMCategoryList->AddRowSorted(pRow, NULL);

	}NxCatchAll("Error creating E/M Code Category");
}

void CEMREMCodeCategorySetupDlg::OnBtnRemoveEmCategory() 
{
	try {

		IRowSettingsPtr pRow = m_EMCategoryList->CurSel;

		if(pRow == NULL) {
			MessageBox("There is no category selected.", "Practice", MB_OK | MB_ICONERROR);
			return;
		}

		long nID = VarLong(pRow->GetValue(emcclcID));
		CString strName = VarString(pRow->GetValue(emcclcDesc),"");

		//disallow deleting categories in use
		// (j.jones 2011-03-08 11:50) - PLID 42282 - also check for EMRDataGroupsT records
		_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ID) AS CountRecords "
			"FROM EMRInfoMasterT WHERE EMCodeCategoryID = {INT} "
			"OR ID IN (SELECT EMRInfoMasterID FROM EMRInfoT "
			"	INNER JOIN EMRDataT ON EMRInfoT.ID = EMRDataT.EMRInfoID "
			"	INNER JOIN EMRDataGroupsT ON EMRDataT.EMRDataGroupID = EMRDataGroupsT.ID "
			"	WHERE EMRDataGroupsT.EMCodeCategoryID = {INT}) \r\n"
			"SELECT Count(ID) AS CountRecords FROM EMChecklistRuleDetailsT WHERE CategoryID = {INT}", nID, nID, nID);
		if(!rs->eof) {
			long nCountRecords = AdoFldLong(rs, "CountRecords",0);
			if(nCountRecords > 0) {
				MessageBox(FormatString("You may not delete this E/M Category because there are %li EMR items linked to it.", nCountRecords), "Practice", MB_OK | MB_ICONERROR);
				return;
			}
		}

		rs = rs->NextRecordset(NULL);

		if(!rs->eof) {
			long nCountRecords = AdoFldLong(rs, "CountRecords",0);
			if(nCountRecords > 0) {
				MessageBox(FormatString("You may not delete this E/M Category because there are %li E/M Checklist Rule Details linked to it.", nCountRecords), "Practice", MB_OK | MB_ICONERROR);
				return;
			}
		}
		rs->Close();

		//if we get here, it's ok to delete
		ExecuteSql("DELETE FROM EMCodeCategoryT WHERE ID = %li", nID);

		//store an ID for each record deleted (even if we just created it),
		//incase we are called from the Checklist Element Setup dialog,
		//which will need to remove any references to that ID
		m_dwaryDeletedIDs.Add((DWORD)nID);

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiEMCodeCategoryDeleted, nID, strName, "<Deleted>", aepMedium, aetDeleted);

		m_EMCategoryList->RemoveRow(pRow);

	}NxCatchAll("Error deleting E/M Code Category");
}

BEGIN_EVENTSINK_MAP(CEMREMCodeCategorySetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMREMCodeCategorySetupDlg)
	ON_EVENT(CEMREMCodeCategorySetupDlg, IDC_EM_CODE_CATEGORY_LIST, 9 /* EditingFinishing */, OnEditingFinishingEmCodeCategoryList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEMREMCodeCategorySetupDlg, IDC_EM_CODE_CATEGORY_LIST, 10 /* EditingFinished */, OnEditingFinishedEmCodeCategoryList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMREMCodeCategorySetupDlg::OnEditingFinishingEmCodeCategoryList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL || nCol != emcclcDesc)
			return;

		long nID = VarLong(pRow->GetValue(emcclcID));

		CString strEntered = strUserEntered;
		strEntered.TrimLeft();
		strEntered.TrimRight();

		if(strEntered.IsEmpty()) {
			MessageBox("You may not have a blank category name.", "Practice", MB_OK | MB_ICONERROR);
			*pbCommit = FALSE;
			return;
		}

		if(VarString(varOldValue, "") != strUserEntered) {
			//(e.lally 2008-10-13) PLID 31665 - Put format string inside ReturnsRecords params.
			if(ReturnsRecords("SELECT ID FROM EMCodeCategoryT WHERE Name LIKE '%s' AND ID <> %li", _Q(strUserEntered), nID)) {
				MessageBox("There is already an E/M Category with this name.", "Practice", MB_OK | MB_ICONERROR);
				*pbCommit = FALSE;
				return;
			}

			//warn before renaming categories in use
			// (j.jones 2011-03-08 11:50) - PLID 42282 - also check for EMRDataGroupsT records
			_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ID) AS CountRecords "
				"FROM EMRInfoMasterT WHERE EMCodeCategoryID = {INT} "
				"OR ID IN (SELECT EMRInfoMasterID FROM EMRInfoT "
				"	INNER JOIN EMRDataT ON EMRInfoT.ID = EMRDataT.EMRInfoID "
				"	INNER JOIN EMRDataGroupsT ON EMRDataT.EMRDataGroupID = EMRDataGroupsT.ID "
				"	WHERE EMRDataGroupsT.EMCodeCategoryID = {INT})", nID, nID);
			if(!rs->eof) {
				long nCountRecords = AdoFldLong(rs, "CountRecords",0);
				if(nCountRecords > 0) {
					CString str;
					str.Format("There are %li EMR items linked to this E/M Category, are you sure you still wish to rename it?\n"
						"If you renamed this category, the existing items will continue to link to it, referencing the new name.", nCountRecords);
					if(IDNO == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
						*pbCommit = FALSE;
						return;
					}
				}
			}
			rs->Close();
		}

	}NxCatchAll("Error in CEMREMCodeCategorySetupDlg::OnEditingFinishingEmCodeCategoryList");
}

void CEMREMCodeCategorySetupDlg::OnEditingFinishedEmCodeCategoryList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		if(!bCommit)
			return;

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL || nCol != emcclcDesc)
			return;

		long nID = VarLong(pRow->GetValue(emcclcID));

		CString strValue = VarString(varNewValue, "");

		if(strValue != VarString(varOldValue, "")) {

			if(strValue.GetLength() > 255) {
				MessageBox("You entered a value greater then the maximum length (255).  The data will be truncated.", "Practice", MB_OK | MB_ICONERROR);
				strValue = strValue.Left(255);
				pRow->PutValue(emcclcDesc, _variant_t(strValue));
			}

			ExecuteSql("UPDATE EMCodeCategoryT SET Name = '%s' WHERE ID = %li", _Q(strValue), nID);

			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiEMCodeCategoryName, nID, VarString(varOldValue, ""), strValue, aepMedium, aetChanged);

			m_EMCategoryList->Sort();
		}

	}NxCatchAll("Error editing E/M Code Category");
}
