// LatinPrescriptionMacrosDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LatinPrescriptionMacrosDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CLatinPrescriptionMacrosDlg dialog


// (j.jones 2010-05-07 10:43) - PLID 36062 - added column enums
enum LatinMacroColumns {

	lmcID = 0,
	lmcLatin,
	lmcEnglish,
	lmcOldLatin,
	lmcOldeEnglish,
};

CLatinPrescriptionMacrosDlg::CLatinPrescriptionMacrosDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLatinPrescriptionMacrosDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLatinPrescriptionMacrosDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLatinPrescriptionMacrosDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLatinPrescriptionMacrosDlg)
	DDX_Control(pDX, IDC_CHECK_SHOW_CONVERSION, m_btnShowConversion);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_DELETE_NOTATION, m_btnDelete);
	DDX_Control(pDX, IDC_BTN_ADD_NOTATION, m_btnAdd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLatinPrescriptionMacrosDlg, CNxDialog)
	//{{AFX_MSG_MAP(CLatinPrescriptionMacrosDlg)
	ON_BN_CLICKED(IDC_BTN_ADD_NOTATION, OnBtnAddNotation)
	ON_BN_CLICKED(IDC_BTN_DELETE_NOTATION, OnBtnDeleteNotation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLatinPrescriptionMacrosDlg message handlers

BOOL CLatinPrescriptionMacrosDlg::OnInitDialog() 
{
	try {
		// (c.haag 2008-05-20 13:36) - PLID 29790 - NxIconified buttons
		CNxDialog::OnInitDialog();
		
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);

		m_Latin_List = BindNxDataListCtrl(this,IDC_LATIN_LIST,GetRemoteData(),true);

		CheckDlgButton(IDC_CHECK_SHOW_CONVERSION, GetRemotePropertyInt("ShowEnglishPrescriptionColumn",0,0,GetCurrentUserName(),true) == 1);
	}
	NxCatchAll("Error in CLatinPrescriptionMacrosDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLatinPrescriptionMacrosDlg::OnOK() 
{
	try {

		CWaitCursor pWait;

		// (j.jones 2010-05-07 10:46) - PLID 36062 - turned this into a param. batch

		CString strSqlBatch;
		CNxParamSqlArray aryParams;

		//first delete notations
		for(int i=0; i<m_dwaryDeleted.GetSize();i++) {
			long nID = (long)(m_dwaryDeleted.GetAt(i));
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM LatinPrescNotationT WHERE ID = {INT}",nID);
		}

		//now update existing / save new

		BOOL bDeclarationCreated = FALSE;

		for(i=0; i<m_Latin_List->GetRowCount();i++) {
			long nID = VarLong(m_Latin_List->GetValue(i,lmcID));
			CString strLatin = VarString(m_Latin_List->GetValue(i,lmcLatin));
			CString strEnglish = VarString(m_Latin_List->GetValue(i,lmcEnglish));
			// (j.jones 2010-05-07 10:46) - PLID 36062 - we now track the old values, to know when to update or not
			CString strOldLatin = VarString(m_Latin_List->GetValue(i,lmcOldLatin));
			CString strOldeEnglish = VarString(m_Latin_List->GetValue(i,lmcOldeEnglish));

			if(nID == -1) {

				if(!bDeclarationCreated) {
					AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewPrescNotationID INT");
					bDeclarationCreated = TRUE;
				}

				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET @nNewPrescNotationID = (SELECT Coalesce(Max(ID),0) + 1 AS NewID FROM LatinPrescNotationT)");

				//add new
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO LatinPrescNotationT (ID, Latin, English) "
					"VALUES (@nNewPrescNotationID, {STRING}, {STRING})", strLatin, strEnglish);
			}
			// (j.jones 2010-05-07 10:46) - PLID 36062 - only update if something changed
			else if(strOldLatin != strLatin || strEnglish != strOldeEnglish) {
				//update existing
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE LatinPrescNotationT SET Latin = {STRING}, English = {STRING} WHERE ID = {INT}",
					strLatin, strEnglish, nID);
			}
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
		}

		SetRemotePropertyInt("ShowEnglishPrescriptionColumn",IsDlgButtonChecked(IDC_CHECK_SHOW_CONVERSION) ? 1 : 0, 0, GetCurrentUserName());

		// (j.jones 2010-05-07 10:35) - PLID 36062 - we removed this ability, we should never be changing existing medications
		//UpdateMedicationsWithLatinToEnglishConversion();

		CNxDialog::OnOK();

	}NxCatchAll("Error saving notations.");
}

void CLatinPrescriptionMacrosDlg::OnBtnAddNotation() 
{
	try {

		int Count = m_Latin_List->GetRowCount();
		Count++;

		CString strNewName;
		strNewName.Format("[New Notation %li]",Count);
		while(m_Latin_List->FindByColumn(lmcLatin,_bstr_t(strNewName),0,FALSE) != -1) {
			Count++;
			strNewName.Format("[New Notation %li]",Count);
		}

		IRowSettingsPtr pRow = m_Latin_List->GetRow(-1);
		pRow->PutValue(lmcID,(long)-1);
		pRow->PutValue(lmcLatin,_bstr_t(strNewName));
		pRow->PutValue(lmcEnglish,_bstr_t("[Enter English Translation]"));
		pRow->PutValue(lmcOldLatin,_bstr_t(""));
		pRow->PutValue(lmcOldeEnglish,_bstr_t(""));
		int row = m_Latin_List->AddRow(pRow);
		m_Latin_List->CurSel = row;
		m_Latin_List->StartEditing(row,lmcLatin);

	}NxCatchAll("Error adding new notation.");
}

void CLatinPrescriptionMacrosDlg::OnBtnDeleteNotation() 
{
	try {

		if(m_Latin_List->CurSel == -1) {
			AfxMessageBox("Please select a notation to delete.");
			return;
		}

		long nID = VarLong(m_Latin_List->GetValue(m_Latin_List->CurSel,lmcID));
		if(nID != -1)
			m_dwaryDeleted.Add((DWORD)nID);

		m_Latin_List->RemoveRow(m_Latin_List->CurSel);

	}NxCatchAll("Error deleting notation.");	
}

BEGIN_EVENTSINK_MAP(CLatinPrescriptionMacrosDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CLatinPrescriptionMacrosDlg)
	ON_EVENT(CLatinPrescriptionMacrosDlg, IDC_LATIN_LIST, 9 /* EditingFinishing */, OnEditingFinishingLatinList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CLatinPrescriptionMacrosDlg, IDC_LATIN_LIST, 10 /* EditingFinished */, OnEditingFinishedLatinList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CLatinPrescriptionMacrosDlg, IDC_LATIN_LIST, 6 /* RButtonDown */, OnRButtonDownLatinList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CLatinPrescriptionMacrosDlg::OnEditingFinishingLatinList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	if(*pbCommit) {

		// (j.fouts 2013-04-04 09:08) - PLID 53396 - As a fail safe remove any bksp chars, 
		// though I have no idea how they could exist here in the first place. We are going
		// to use these to tokenize the latin phrases before converting, so we just
		// need to make sure they don't exist.
		CString strNewValue(strUserEntered);
		strNewValue.Remove((char)8);
		strUserEntered = strNewValue;

		if(nCol == lmcLatin && CString(varOldValue.bstrVal) != strUserEntered) {

			CString strUser = strUserEntered;
			strUser.TrimRight();
			strUser.TrimLeft();

			if(m_Latin_List->FindByColumn(lmcLatin,_bstr_t(strUser),0,FALSE)!=-1) {
				AfxMessageBox("This notation already exists in the list. Please enter a unique notation.");
				*pbCommit = FALSE;
				return;
			}

			if(strUser == "") {
				AfxMessageBox("You must enter a value.");
				*pbCommit = FALSE;
				return;
			}
		}
		else if(nCol == lmcEnglish) {

			CString strUser = strUserEntered;
			strUser.TrimRight();
			strUser.TrimLeft();

			if(strUser == "") {
				AfxMessageBox("You must enter a value.");
				*pbCommit = FALSE;
				return;
			}
		}
	}
}

void CLatinPrescriptionMacrosDlg::OnEditingFinishedLatinList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	
}

void CLatinPrescriptionMacrosDlg::OnRButtonDownLatinList(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_Latin_List->CurSel = nRow;

	if(nRow == -1)
		return;

	CMenu menu;
	menu.CreatePopupMenu();
	menu.InsertMenu(0, MF_BYPOSITION, IDC_BTN_DELETE_NOTATION, "Delete Notation");
	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
}

void CLatinPrescriptionMacrosDlg::OnCancel() 
{
	//if there are blank explanations, update anyways
	//TES 2/10/2009 - PLID 33002 - Renamed Description to PatientExplanation
	// (j.jones 2010-05-07 10:35) - PLID 36062 - we removed this ability, we should never be changing existing medications
	/*
	if(!IsRecordsetEmpty("SELECT ID FROM PatientMedications WHERE convert(nvarchar,EnglishDescription) = '' AND convert(nvarchar,PatientExplanation) <> ''")) {
		CWaitCursor pWait;
		UpdateMedicationsWithLatinToEnglishConversion();
	}
	*/
		
	CDialog::OnCancel();
}