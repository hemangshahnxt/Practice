// EditPatientTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EditPatientTypeDlg.h"


// (b.spivey, May 08, 2012) - PLID 50224 - Created. 

// (b.spivey, May 09, 2012) - PLID 50224 - command defines
#define ID_SET_DEFAULT 0x8888
#define ID_REMOVE_DEFAULT 0x8889
// (b.spivey, May 15, 2012) - PLID 20752 - Command to reset the color. 
#define ID_RESET_COLOR 0x8890
//These are the default color defines. 
#define DEFAULT_COLOR	RGB(255, 255, 255)
#define DEFAULT_SEL_COLOR	1


using namespace ADODB;
using namespace NXDATALIST2Lib;

// CEditPatientTypeDlg dialog

IMPLEMENT_DYNAMIC(CEditPatientTypeDlg, CNxDialog)

CEditPatientTypeDlg::CEditPatientTypeDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditPatientTypeDlg::IDD, pParent)
{

}

CEditPatientTypeDlg::~CEditPatientTypeDlg()
{
}

void CEditPatientTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_BTN_EDIT, m_btnEdit);
	DDX_Control(pDX, IDC_BTN_DELETE, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnClose);
}


BEGIN_MESSAGE_MAP(CEditPatientTypeDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_ADD, OnAdd)
	ON_BN_CLICKED(IDC_BTN_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_BTN_DELETE, OnDelete)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEditPatientTypeDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditPatientTypeDlg)
	ON_EVENT(CEditPatientTypeDlg, IDC_PATIENT_TYPE_LIST, 6 /* RButtonDown */, OnRButtonDownPatientTypeList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEditPatientTypeDlg, IDC_PATIENT_TYPE_LIST, 10 /* EditingFinished */, OnEditingFinishedPatientTypeList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CEditPatientTypeDlg, IDC_PATIENT_TYPE_LIST, 2, CEditPatientTypeDlg::SelChangedPatientTypeList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CEditPatientTypeDlg, IDC_PATIENT_TYPE_LIST, 19, CEditPatientTypeDlg::LeftClickPatientTypeList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// CEditPatientTypeDlg message handlers

BOOL CEditPatientTypeDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	try {
		m_dlPatientType = BindNxDataList2Ctrl(IDC_PATIENT_TYPE_LIST); 
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnEdit.AutoSet(NXB_MODIFY);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);

		//Set the forecolor of the default. 
		long nDefaultPatType = GetRemotePropertyInt("DefaultPatType", -1, 0, "<None>", true); 
		IRowSettingsPtr pRow = m_dlPatientType->FindByColumn(ptlID, _variant_t(nDefaultPatType), 0, FALSE);
		if (pRow != NULL) {
			pRow->PutForeColor(RGB(255, 0, 0)); 
		}

		// (b.spivey, May 15, 2012) - PLID 20752 - Load colors. 
		{
			IRowSettingsPtr pRow = m_dlPatientType->GetFirstRow();

			while (pRow) {
				long nColor = VarLong(pRow->GetValue(ptlColorValue), -1);
				long nSelColor = nColor; 
				CString strColor = ""; 

				if (nColor == -1) {
					// (b.spivey, May 22, 2012) - PLID 20752 - Use the defines. 
					nColor = DEFAULT_COLOR; 
					nSelColor = DEFAULT_SEL_COLOR;
					strColor = "< Default >";
				}
				pRow->PutCellBackColor(ptlColor, nColor); 
				pRow->PutCellBackColorSel(ptlColor, nSelColor); 
				pRow->PutValue(ptlColor, _variant_t(strColor)); 
				pRow = pRow->GetNextRow();
			}
		}

		EnableAppropriateButtons(); 
	}NxCatchAll(__FUNCTION__); 
	return TRUE; 
}

void CEditPatientTypeDlg::OnAdd() 
{
	try {

		IRowSettingsPtr pRow;
		pRow = m_dlPatientType->GetNewRow();
		CString newVal;
		long ID;

		ID = NewNumber("GroupTypes", "TypeIndex");
		newVal.Format("[New Type %li]",ID);

		while(m_dlPatientType->FindByColumn(ptlName, _bstr_t(newVal), NULL, FALSE) != NULL) { 
			//FindByColumn(1,_bstr_t(newVal),0,FALSE)!=-1) {
			ID++;
			newVal.Format("[New Type %li]",ID);
		}

		pRow->PutValue(ptlID, ID);
		pRow->PutValue(ptlName, _bstr_t(newVal));
		// (b.spivey, May 15, 2012) - PLID 20752 - Default color when you add one. 
		pRow->PutValue(ptlColorValue, -1); 
		pRow->PutCellBackColor(ptlColor, DEFAULT_COLOR); 
		pRow->PutCellBackColorSel(ptlColor, DEFAULT_SEL_COLOR); 
		pRow->PutValue(ptlColor, "< Default >"); 

		ExecuteParamSql("INSERT INTO GroupTypes (TypeIndex,GroupName) VALUES ({INT}, {STRING})", ID, newVal);

		m_dlPatientType->AddRowAtEnd(pRow, NULL); 
		m_dlPatientType->SetSelByColumn(ptlName, _bstr_t(newVal));
		m_dlPatientType->StartEditing(m_dlPatientType->CurSel, 1); 
		EnableAppropriateButtons(); 
		// (j.fouts 2012-06-13 16:30) - PLID 50863 - We made changes, refresh the table
		RefreshTable();
	}NxCatchAll(__FUNCTION__); 
}

void CEditPatientTypeDlg::OnEdit()
{
	try {
		m_dlPatientType->StartEditing(m_dlPatientType->GetCurSel(), ptlName);
	}NxCatchAll(__FUNCTION__); 
}

void CEditPatientTypeDlg::OnDelete() 
{
	try {

		//Get a count of patients using this type, if any. 
		_RecordsetPtr rsPatientCount = CreateParamRecordset(
			"SELECT Count(PersonID) as NumOfPats FROM PatientsT WHERE TypeOfPatient = {INT}", 
			VarLong(m_dlPatientType->CurSel->GetValue(ptlID))); 

		bool bDelete = false;

		if (rsPatientCount->eof){

			//there are no patients of this type, so go ahead and let them delete it
			bDelete = true;
		}
		else {

			//give them a messagebox
			long nNumber = VarLong(AdoFldLong(rsPatientCount, "NumofPats"));
			if (nNumber > 0) {

				CString strMsg;
				strMsg.Format("There are %li patient(s) with this type.  Are you sure you want to delete it?", nNumber);
				if (IDYES == MsgBox(MB_YESNO, strMsg)) {
					bDelete = true;
				}
			}
			else {
				bDelete = true;
			}

		}

		//If we're good to delete, lets delete. 
		if (bDelete) {
			// (b.spivey, May 22, 2012) - PLID 50224 - If a default is selected, we need to remove the default in the database. 
			if (IsDefaultSelected()) {
				RemoveDefaultPatientType(); 
			}
			//First clear the type of patient for patients with this group type, and then we delete the group type.
			ExecuteParamSql("UPDATE PatientsT SET TypeOfPatient = NULL WHERE TypeOfPatient = {INT} " 
				"DELETE FROM GroupTypes WHERE TypeIndex = {INT}", 
				VarLong(m_dlPatientType->CurSel->GetValue(ptlID)),
				VarLong(m_dlPatientType->CurSel->GetValue(ptlID)));

			m_dlPatientType->RemoveRow(m_dlPatientType->GetCurSel());

		}
		else {
			return;
		}
		EnableAppropriateButtons(); 
		// (j.fouts 2012-06-13 16:30) - PLID 50863 - We made changes, refresh the table
		RefreshTable();
	}NxCatchAll(__FUNCTION__); 
}

void CEditPatientTypeDlg::OnEditingFinishedPatientTypeList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	try {

		if (!bCommit) {
			return;
		}

		CString strOldVal, strNewVal;
		strOldVal = VarString(varOldValue, ""); 
		strNewVal = VarString(varNewValue, ""); 
		IRowSettingsPtr pRow(lpRow);

		//They're the same, why bother doing anything else? 
		if (strOldVal.Compare(strNewVal) != 0) {

			// (b.spivey, June 07, 2012) - PLID 50224 - You should not be allowed to enter a blank name. 
			if (strNewVal.IsEmpty()) {
				AfxMessageBox("Please enter valid data.");
				pRow->PutValue(nCol, _variant_t(strOldVal)); 
				return; 
			}

			// (b.spivey, June 05, 2012) - PLID 50224 - Entered this switch for expandability. 
			switch(nCol) {
				case (long)ptlName:
				{
					//check for duplicates. 
					if (!ReturnsRecordsParam("SELECT TypeIndex FROM GroupTypes WHERE GroupName = {STRING}", strNewVal)) {
						if(strNewVal.GetLength()>100) {
							AfxMessageBox("You entered a value greater than the maximum length (100). The data will be truncated.");
							strNewVal = strNewVal.Left(100);
							pRow->PutValue(nCol, _variant_t(strNewVal));  
						}
						// (b.spivey, June 07, 2012) - PLID 50224 - Was escaping quotes in a param, effectively doubling their number. 
						ExecuteParamSql("UPDATE GroupTypes SET GroupName = {STRING} WHERE TypeIndex = {INT}", 
							strNewVal, VarLong(pRow->GetValue(ptlID))); 
					}
					else {
						//Warn them about already entered data then reset the row's value. 
						AfxMessageBox("The data you entered already exists in the list.");
						pRow->PutValue(nCol, _variant_t(strOldVal)); 
					}
					break;
				}
				default: 
				{
					//Nothing happens here right now. 
					break;
				}
			}
		}

		EnableAppropriateButtons(); 
		// (j.fouts 2012-06-13 16:30) - PLID 50863 - We made changes, refresh the table
		RefreshTable();
	}NxCatchAll(__FUNCTION__); 
}

void CEditPatientTypeDlg::OnRButtonDownPatientTypeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try{
		IRowSettingsPtr pRow(lpRow);

		m_dlPatientType->CurSel = pRow; 

		if (pRow == NULL) {
			EnableAppropriateButtons();
			return;
		}
		
		CMenu menPopup;
		menPopup.m_hMenu = CreatePopupMenu();

		if(!IsDefaultSelected())
			menPopup.InsertMenu(0, MF_BYPOSITION, ID_SET_DEFAULT, "Set As Default");
		else
			menPopup.InsertMenu(0, MF_BYPOSITION, ID_REMOVE_DEFAULT, "Remove Default");

		// (b.spivey, May 15, 2012) - PLID 20752 - Reset color added to dropdown menu. 
		//Since we make a database call on this command and -1 is already default, disable it. 
		if(VarLong(pRow->GetValue(ptlColorValue), -1) == -1) {
			menPopup.InsertMenu(1, MF_DISABLED|MF_BYPOSITION, ID_RESET_COLOR, "Reset Color"); 
		}
		else {
			menPopup.InsertMenu(1, MF_ENABLED|MF_BYPOSITION, ID_RESET_COLOR, "Reset Color"); 
		}

		CPoint pt(x,y);
		CWnd* pWnd = GetDlgItem(IDC_PATIENT_TYPE_LIST);
		if (pWnd != NULL)
		{	pWnd->ClientToScreen(&pt);
			menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
		else{
			HandleException(NULL, "An error ocurred while creating menu");
		}
		
		EnableAppropriateButtons();	
	}NxCatchAll(__FUNCTION__);
}

BOOL CEditPatientTypeDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try{
		switch(wParam) {
			case ID_SET_DEFAULT:
				{
					//1 - Reset the old default.
					long nOldDefault = GetRemotePropertyInt("DefaultPatType", -1, 0, "<None>", true); 
					IRowSettingsPtr pRow = m_dlPatientType->FindByColumn(ptlID, _variant_t(nOldDefault), 0, FALSE); 
					if(pRow != NULL) {
						pRow->PutForeColor(RGB(0,0,0));
					}

					//2 - Save the new default to the database. 
					//	If, for whatever reason, this is null then we use the "remove" case value as a default.
					long nNewDefault = VarLong(m_dlPatientType->CurSel->GetValue(ptlID), -1);
					SetRemotePropertyInt("DefaultPatType", nNewDefault, 0, "<None>");

					//3 - Update the interface to indicate the new default.
					m_dlPatientType->CurSel->PutForeColor(RGB(255, 0, 0)); 
					break;
				}

			case ID_REMOVE_DEFAULT:
				{

					RemoveDefaultPatientType(); 
					break;
				}
			case ID_RESET_COLOR:
				{
					// (b.spivey, May 15, 2012) - PLID 20752 - If they want to reset the color, revert to defaults. 
					IRowSettingsPtr pRow = m_dlPatientType->CurSel; 
					pRow->PutCellBackColor(ptlColor, DEFAULT_COLOR); 
					pRow->PutCellBackColorSel(ptlColor, DEFAULT_SEL_COLOR); 
					pRow->PutValue(ptlColor, "< Default >");
					pRow->PutValue(ptlColorValue, -1); 
					ExecuteParamSql("UPDATE GroupTypes SET GroupTypes.Color = -1 WHERE GroupTypes.TypeIndex = {INT}", 
						VarLong(pRow->GetValue(ptlID), -1)); 
					// (j.fouts 2012-06-13 16:30) - PLID 50863 - We made changes, refresh the table
					RefreshTable();
				}
		}

	}NxCatchAll(__FUNCTION__);
	
	return CDialog::OnCommand(wParam, lParam);
}

bool CEditPatientTypeDlg::IsDefaultSelected() 
{
	long nDefault = GetRemotePropertyInt("DefaultPatType", -1, 0, "<None>", true); 
	IRowSettingsPtr pRow = m_dlPatientType->FindByColumn(ptlID, nDefault, 0, FALSE); 

	// (b.spivey, June 07, 2012) - PLID 50224 - Check for nulls. 
	if (pRow != NULL && pRow == m_dlPatientType->CurSel) {
		return true;
	}
	
	return false; 
}

void CEditPatientTypeDlg::EnableAppropriateButtons() 
{
	if (m_dlPatientType->GetRowCount() == 0 || m_dlPatientType->GetCurSel() == NULL) {
		m_btnEdit.EnableWindow(FALSE); 
		m_btnDelete.EnableWindow(FALSE); 
	}
	else if (m_dlPatientType->GetRowCount() > 0 && m_dlPatientType->GetCurSel() != NULL) {
		m_btnEdit.EnableWindow(TRUE);
		m_btnDelete.EnableWindow(TRUE);
	}
	
}

// (b.spivey, May 14, 2012) - PLID 50224 - When changing selections, we'll need to make sure that our buttons are 
//	 enabled appropriately. 
void CEditPatientTypeDlg::SelChangedPatientTypeList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		EnableAppropriateButtons(); 
	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, May 15, 2012) - PLID 20752 - Need to check for the color column. 
void CEditPatientTypeDlg::LeftClickPatientTypeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow); 
		if (pRow == NULL) {
			return; 
		}
		
		//Color column
		if (nCol == ptlColor) {

			CColorDialog dlg(pRow->GetValue(ptlColorValue), CC_ANYCOLOR|CC_RGBINIT, this); 
			//If they selected a color, we need to update that cell. 
			if (dlg.DoModal() == IDOK) {
				long nColor = (long)dlg.m_cc.rgbResult; 
				// (b.spivey, May 24, 2012) - PLID 20752 - If it's zero or below assume black. 
				if (nColor < 0) {
					nColor = 0; 
				}
				long nTypeID = VarLong(pRow->GetValue(ptlID), -1);

				pRow->PutCellBackColor(ptlColor, nColor); 
				pRow->PutCellBackColorSel(ptlColor, nColor); 
				pRow->PutValue(ptlColor, "");
				pRow->PutValue(ptlColorValue, _variant_t(nColor));
				

				//Whatever value we got from the color picker, put it in the database. 
				ExecuteParamSql("UPDATE GroupTypes SET GroupTypes.Color = {INT} WHERE GroupTypes.TypeIndex = {INT}",
					nColor, nTypeID); 
				// (j.fouts 2012-06-13 16:30) - PLID 50863 - We made changes, refresh the table
				RefreshTable();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.spivey, May 22, 2012) - PLID 50224 - Function to remove the default patient type, since we need this logic in more than one place. 
void CEditPatientTypeDlg::RemoveDefaultPatientType()
{
	//1 - Reset the old default
	long nOldDefault = GetRemotePropertyInt("DefaultPatType", -1, 0, "<None>", true); 
	IRowSettingsPtr pRow = m_dlPatientType->FindByColumn(ptlID, _variant_t(nOldDefault), 0, FALSE); 
	if(pRow != NULL) {
		pRow->PutForeColor(RGB(0,0,0));
	}

	//2 - Reset the database property. 
	SetRemotePropertyInt("DefaultPatType", -1, 0, "<None>");
}

// (j.fouts 2012-06-13 16:30) - PLID 50863 - Use the table checker when this changes
void CEditPatientTypeDlg::RefreshTable()
{
	CClient::RefreshTable(NetUtils::GroupTypes);
}