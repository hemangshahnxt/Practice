// EditPrefixesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "EditPrefixesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEditPrefixesDlg dialog


CEditPrefixesDlg::CEditPrefixesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditPrefixesDlg::IDD, pParent)
{
	m_bChangeInformIds = false;
	//{{AFX_DATA_INIT(CEditPrefixesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditPrefixesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditPrefixesDlg)
	DDX_Control(pDX, IDC_ADD_PREFIX, m_btnAddPrefix);
	DDX_Control(pDX, IDC_REMOVE_PREFIX, m_btnRemovePrefix);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditPrefixesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditPrefixesDlg)
	ON_BN_CLICKED(IDC_ADD_PREFIX, OnAddPrefix)
	ON_BN_CLICKED(IDC_REMOVE_PREFIX, OnRemovePrefix)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditPrefixesDlg message handlers

BOOL CEditPrefixesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnAddPrefix.AutoSet(NXB_NEW);
	m_btnRemovePrefix.AutoSet(NXB_DELETE);
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_pPrefixes = BindNxDataListCtrl(this, IDC_EDIT_PREFIX, GetRemoteData(), true);
	if(m_bChangeInformIds) {
		//Make this column visible.
		DWORD nStyle = m_pPrefixes->GetColumn(3)->GetColumnStyle();
		nStyle &= csWidthAuto;
		nStyle |= ~csFixedWidth;
		nStyle |= ~csWidthPercent;
		nStyle |= ~csWidthData;
		m_pPrefixes->GetColumn(3)->PutColumnStyle(nStyle);
	}
	
	RefreshButtons();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditPrefixesDlg::OnAddPrefix() 
{
	IRowSettingsPtr pRow = m_pPrefixes->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, _bstr_t(""));
	pRow->PutValue(2, (long)0);
	pRow->PutValue(3, (long)0);
	m_pPrefixes->PutCurSel(m_pPrefixes->AddRow(pRow));
	m_pPrefixes->StartEditing(m_pPrefixes->CurSel, 1);
	RefreshButtons();
}

void CEditPrefixesDlg::OnRemovePrefix() 
{
	if(m_pPrefixes->CurSel != -1) {
		long nDoomedPrefix = VarLong(m_pPrefixes->GetValue(m_pPrefixes->CurSel, 0));
		if(ReturnsRecords("SELECT ID FROM PersonT WHERE PrefixID = %li", nDoomedPrefix)) {
			if(IDYES != MsgBox(MB_YESNO, "Warning!  This prefix is being used by patients or contacts.\n"
				"If you delete it, ALL patients or contacts with this prefix will be irreversibly set to "
				"no prefix.\nAre you sure you wish to do this?")) {
				return;
			}
		}
		if(GetRemotePropertyInt("DefaultPatientPrefix",-1,0,"<None>",true) == nDoomedPrefix) {
			if(IDYES != MsgBox(MB_YESNO, "Warning!  This prefix is the default for new patients.\n"
				"If you delete it, there will no longer be a default prefix for new patients.\n"
				"Are you sure you wish to do this?")) {
				return;
			}
		}
		if(GetRemotePropertyInt("DefaultDrPrefix",-1,0,"<None>",true) == nDoomedPrefix) {
			if(IDYES != MsgBox(MB_YESNO, "Warning!  This prefix is the default for doctors.\n"
				"If you delete it, there will no longer be a default prefix for new providers and referring physicians.\n"
				"Are you sure you wish to do this?")) {
				return;
			}
		}
		if(GetRemotePropertyInt("GenderPrefixLink",1,0,"<None>",true)) {
			if(GetRemotePropertyInt("DefaultFemalePrefix",-1,0,"<None>",true) == nDoomedPrefix) {
				if(IDYES != MsgBox(MB_YESNO, "Warning!  This prefix is the default for female patients.\n"
					"If you delete it, there will no longer be a default prefix for female patients.\n"
					"Are you sure you wish to do this?")) {
					return;
				}
			}
			if(GetRemotePropertyInt("DefaultMalePrefix",-1,0,"<None>",true) == nDoomedPrefix) {
				if(IDYES != MsgBox(MB_YESNO, "Warning!  This prefix is the default for male patients.\n"
					"If you delete it, there will no longer be a default prefix for male patients.\n"
					"Are you sure you wish to do this?")) {
					return;
				}
			}
		}
		m_pPrefixes->RemoveRow(m_pPrefixes->CurSel);
		RefreshButtons();
	}
}

void CEditPrefixesDlg::OnOK() 
{
	if(IDYES != MsgBox(MB_YESNO, "Any prefixes you have changed will affect all patients with the changed prefix.\n"
		"These changes can NOT be undone.  Are you sure you wish to continue?")) {
		return;
	}

	try {
		//Loop through each row; save it.
		CString strSavedIds, strId;
		for(int i=0; i < m_pPrefixes->GetRowCount(); i++) {
			long nID = VarLong(m_pPrefixes->GetValue(i, 0));
			if(nID == -1) {
				//This is a new row; save it.
				long nNewId = NewNumber("PrefixT", "ID");
				strId.Format("%li, ", nNewId);
				strSavedIds += strId;
				ExecuteSql("INSERT INTO PrefixT (ID, Prefix, Gender, InformID) "
					"VALUES (%li, '%s', %li, %li)", nNewId, _Q(VarString(m_pPrefixes->GetValue(i, 1))), 
					VarLong(m_pPrefixes->GetValue(i, 2)), VarLong(m_pPrefixes->GetValue(i, 3)));
			}
			else {
				//This is an existing row; update it.
				strId.Format("%li, ", nID);
				strSavedIds += strId;
				ExecuteSql("UPDATE PrefixT SET Prefix = '%s', Gender = %li, InformID = %li WHERE ID = %li",
					_Q(VarString(m_pPrefixes->GetValue(i, 1))), VarLong(m_pPrefixes->GetValue(i, 2)), 
					VarLong(m_pPrefixes->GetValue(i, 3)), nID);
			}
		}
		//Now delete all the rows that we removed.
		strSavedIds = strSavedIds.Left(strSavedIds.GetLength()-2);

		//if there are no items left we need something
		if(strSavedIds.IsEmpty()) {
			ExecuteSql("UPDATE PersonT SET PrefixID = NULL");	//wipe 'em all out
			ExecuteSql("DELETE FROM ConfigRT WHERE Name = 'DefaultPatientPrefix'");
			ExecuteSql("DELETE FROM ConfigRT WHERE Name = 'DefaultDrPrefix'");
			ExecuteSql("DELETE FROM ConfigRT WHERE Name = 'DefaultFemalePrefix'");
			ExecuteSql("DELETE FROM ConfigRT WHERE Name = 'DefaultMalePrefix'");
			//(e.lally 2011-05-05) PLID 43481 - remove NexWeb display setup
			ExecuteSql("DELETE FROM NexWebDisplayT WHERE PrefixID IS NOT NULL ");
			ExecuteSql("DELETE FROM PrefixT");	//wipe 'em all out
		}	
		else {
			ExecuteSql("UPDATE PersonT SET PrefixID = NULL WHERE PrefixID NOT IN (%s)", strSavedIds);
			ExecuteSql("DELETE FROM ConfigRT WHERE Name = 'DefaultPatientPrefix' AND IntParam NOT IN (%s)", strSavedIds);
			ExecuteSql("DELETE FROM ConfigRT WHERE Name = 'DefaultDrPrefix' AND IntParam NOT IN (%s)", strSavedIds);
			ExecuteSql("DELETE FROM ConfigRT WHERE Name = 'DefaultFemalePrefix' AND IntParam NOT IN (%s)", strSavedIds);
			ExecuteSql("DELETE FROM ConfigRT WHERE Name = 'DefaultMalePrefix' AND IntParam NOT IN (%s)", strSavedIds);
			//(e.lally 2011-05-05) PLID 43481 - remove NexWeb display setup
			ExecuteSql("DELETE FROM NexWebDisplayT WHERE PrefixID IS NOT NULL AND PrefixID NOT IN (%s) ", strSavedIds);
			ExecuteSql("DELETE FROM PrefixT WHERE ID NOT IN (%s)", strSavedIds);
		}
	}NxCatchAll("Error in CEditPrefixesDlg::OnOK()");

	CDialog::OnOK();
}

BEGIN_EVENTSINK_MAP(CEditPrefixesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditPrefixesDlg)
	ON_EVENT(CEditPrefixesDlg, IDC_EDIT_PREFIX, 9 /* EditingFinishing */, OnEditingFinishingEditPrefix, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEditPrefixesDlg, IDC_EDIT_PREFIX, 10 /* EditingFinished */, OnEditingFinishedEditPrefix, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEditPrefixesDlg, IDC_EDIT_PREFIX, 16 /* SelChosen */, OnSelChosenEditPrefix, VTS_I4)
	ON_EVENT(CEditPrefixesDlg, IDC_EDIT_PREFIX, 19 /* LeftClick */, OnLeftClickEditPrefix, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditPrefixesDlg::OnEditingFinishingEditPrefix(long nRow, short nCol,  const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	switch(nCol) {
	case 1:
		{
			CString strTemp = strUserEntered;
			strTemp.TrimLeft();
			strTemp.TrimRight();
			if(strTemp.IsEmpty()) {
				MsgBox("You cannot have an empty prefix.");
				*pbCommit = FALSE;
				return;
			}
			else if(strTemp.GetLength() > 50) {
				MsgBox("A prefix cannot exceed 50 characters.");
				*pbCommit = FALSE;
				return;
			}
			else {
				//Is this prefix already entered in another row?
				bool bDuplicateFound = false;
				for(int i = 0; i < m_pPrefixes->GetRowCount(); i++) {
					if(i != nRow && VarString(m_pPrefixes->GetValue(i, 1)) == strTemp) bDuplicateFound = true;
				}
				if(bDuplicateFound) {
					MsgBox("This prefix already exists.");
					*pbCommit = FALSE;
					return;
				}
				strUserEntered = strTemp;
			}
		}
		break;
	case 2:
	case 3:
		//No validation necessary.,
		break;
	default:
		//They shouldn't be able to edit anything else!
		ASSERT(FALSE);
		break;
	}
}

void CEditPrefixesDlg::OnEditingFinishedEditPrefix(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if(!bCommit) {
		if(varOldValue.vt == VT_BSTR && varNewValue.vt == VT_BSTR){
			CString strOldValue = VarString(varOldValue);
			CString strNewValue = VarString(varNewValue);
			strOldValue.TrimLeft();
			strOldValue.TrimRight();
			strNewValue.TrimLeft();
			strNewValue.TrimRight();
			// (s.dhole 2009-12-22 12:02) - PLID 13136 Each time you name a prefix the same as another prefix, it tells you and clears what you entered, but leaves a blank row there, that gets saved if you hit "OK"
			// added (strOldValue == "" && strNewValue != "") condition to fix this issue. 
			if((strOldValue == "" && strNewValue == "") || (strOldValue == "" && strNewValue != "")) {
				//This must be one they added and then didn't enter anything.  Let's remove this rown.
				m_pPrefixes->RemoveRow(nRow);
			}
		}
	}
}

void CEditPrefixesDlg::RefreshButtons()
{
	if(m_pPrefixes->GetCurSel() == sriNoRow){
		GetDlgItem(IDC_REMOVE_PREFIX)->EnableWindow(FALSE);
	}
	else{
		GetDlgItem(IDC_REMOVE_PREFIX)->EnableWindow(TRUE);
	}
}

void CEditPrefixesDlg::OnSelChosenEditPrefix(long nRow) 
{
	RefreshButtons();	
}

void CEditPrefixesDlg::OnLeftClickEditPrefix(long nRow, short nCol, long x, long y, long nFlags) 
{
	RefreshButtons();	
}
