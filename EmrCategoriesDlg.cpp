// EmrCategoriesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorRc.h"
#include "EmrCategoriesDlg.h"
#include "EmrUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;

/////////////////////////////////////////////////////////////////////////////
// CEmrCategoriesDlg dialog


CEmrCategoriesDlg::CEmrCategoriesDlg(CWnd* pParent)
	: CNxDialog(CEmrCategoriesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmrCategoriesDlg)
		m_bIsLevel1EMR = TRUE;
	//}}AFX_DATA_INIT
}


void CEmrCategoriesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmrCategoriesDlg)
	DDX_Control(pDX, IDC_EMR_CATEGORY_DOWN, m_btnDown);
	DDX_Control(pDX, IDC_EMR_CATEGORY_UP, m_btnUp);
	DDX_Control(pDX, IDC_ADD_EMR_CATEGORY, m_btnAddEMRCategory);
	DDX_Control(pDX, IDC_DELETE_EMR_CATEGORY, m_btnDeleteEMRCategory);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmrCategoriesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEmrCategoriesDlg)
	ON_BN_CLICKED(IDC_ADD_EMR_CATEGORY, OnAddEmrCategory)
	ON_BN_CLICKED(IDC_DELETE_EMR_CATEGORY, OnDeleteEmrCategory)
	ON_BN_CLICKED(IDC_EMR_CATEGORY_UP, OnEmrCategoryUp)
	ON_BN_CLICKED(IDC_EMR_CATEGORY_DOWN, OnEmrCategoryDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrCategoriesDlg message handlers
using namespace ADODB;

BOOL CEmrCategoriesDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		m_btnUp.AutoSet(NXB_UP);
		m_btnDown.AutoSet(NXB_DOWN);
		// (c.haag 2008-04-29 16:54) - PLID 29837 - NxIconify more buttons
		m_btnAddEMRCategory.AutoSet(NXB_NEW);
		m_btnDeleteEMRCategory.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_CLOSE);
		
		// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
		if(g_pLicense->HasEMR(CLicense::cflrSilent) == 2) {
			SetWindowText("EMR Categories");
			m_bIsLevel1EMR = FALSE;
		}
		else {
			SetWindowText("Categories");
			m_bIsLevel1EMR = TRUE;
		}

		GetDlgItem(IDC_DELETE_EMR_CATEGORY)->EnableWindow(FALSE);
		GetDlgItem(IDC_EMR_CATEGORY_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_EMR_CATEGORY_DOWN)->EnableWindow(FALSE);

		if(!m_bIsLevel1EMR) {
			GetDlgItem(IDC_EMR_CATEGORY_UP)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EMR_CATEGORY_DOWN)->ShowWindow(SW_HIDE);
		}

		m_pCategories = BindNxDataListCtrl(IDC_EMR_CATEGORY_LIST, false);

		if(m_bIsLevel1EMR) {
			//sort by priority
			m_pCategories->GetColumn(1)->SortPriority = 0;
			m_pCategories->AllowSort = FALSE;
		}
		else {
			//sort by name
			m_pCategories->GetColumn(2)->SortPriority = 0;
			m_pCategories->AllowSort = TRUE;
		}

		m_pCategories->Requery();
	}
	NxCatchAll("Error in CEmrCategoriesDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEmrCategoriesDlg::OnAddEmrCategory() 
{
	try {
		CString strNewName;
		if(IDOK == InputBoxLimited(this, "Enter a name for the new category", strNewName, "",255,false,false,NULL)) {
			strNewName.TrimLeft();
			strNewName.TrimRight();
			if(strNewName.IsEmpty()) {
				MsgBox("You cannot have a category with a blank name.");
				return;
			}
			else if(ReturnsRecords("SELECT ID FROM EmrCategoriesT WHERE NAme = '%s'", _Q(strNewName))) {
				MsgBox("There is already an EMR Category with the name '%s'", strNewName);
				return;
			}
			// (j.jones 2006-01-18 12:00) - priority is not used in EMRL2, but is used in EMRL1
			ExecuteSql("INSERT INTO EmrCategoriesT (ID, Name, Priority, Format) "
				"VALUES (%li, '%s', %li, %li)", NewNumber("EmrCategoriesT", "ID"), _Q(strNewName), 
				NewNumber("EmrCategoriesT", "Priority"), ecfParagraph);
			m_pCategories->Requery();
		}
	}NxCatchAll("Error in CEmrCategoriesDlg::OnAddEmrCategory()");
}

void CEmrCategoriesDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CNxDialog::OnOK();
}

void CEmrCategoriesDlg::OnDeleteEmrCategory() 
{
	if(m_pCategories->CurSel == -1) return;

	try {
		if(ReturnsRecords("SELECT * FROM EmrInfoCategoryT WHERE EMRCategoryID = %li", VarLong(m_pCategories->GetValue(m_pCategories->CurSel, 0)))) {
			MsgBox("There are Emr Items assigned to this category.  Please re-assign these items before deleting this category.");
			return;
		}
		if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to permanently delete this category?")) {
			return;
		}
		ExecuteSql("DELETE FROM EmrCategoriesT WHERE ID = %li", VarLong(m_pCategories->GetValue(m_pCategories->CurSel, 0)));
		m_pCategories->Requery();
	}NxCatchAll("Error in OnDeleteEmrCategory()");
}

BEGIN_EVENTSINK_MAP(CEmrCategoriesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEmrCategoriesDlg)
	ON_EVENT(CEmrCategoriesDlg, IDC_EMR_CATEGORY_LIST, 9 /* EditingFinishing */, OnEditingFinishingEmrCategoryList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEmrCategoriesDlg, IDC_EMR_CATEGORY_LIST, 10 /* EditingFinished */, OnEditingFinishedEmrCategoryList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEmrCategoriesDlg, IDC_EMR_CATEGORY_LIST, 2 /* SelChanged */, OnSelChangedEmrCategoryList, VTS_I4)
	ON_EVENT(CEmrCategoriesDlg, IDC_EMR_CATEGORY_LIST, 18 /* RequeryFinished */, OnRequeryFinishedEmrCategoryList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEmrCategoriesDlg::OnEditingFinishingEmrCategoryList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	if(nRow == -1) return;
	if(!*pbCommit) return;
	
	try {
		switch(nCol) {
		case 2: //Name.
			CString strNew(strUserEntered);
			strNew.TrimLeft();
			strNew.TrimRight();
			if(strNew.IsEmpty()) {
				MsgBox("You cannot have an EMR Category with a blank name.");
				*pbContinue = false;
				*pbCommit = false;
			}
			else if(ReturnsRecords("SELECT ID FROM EmrCategoriesT WHERE Name = '%s' AND ID <> %li", _Q(strNew), VarLong(m_pCategories->GetValue(nRow, 0)))) {
				MsgBox("There is already an EMR Category with the name '%s.'", strUserEntered);
				*pbContinue = false;
				*pbCommit = false;
			}
			break;
		}
	}NxCatchAll("Error in CEmrCategoriesDlg::OnEditingFinishingEmrCategoryList()");

}

void CEmrCategoriesDlg::OnEditingFinishedEmrCategoryList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if(nRow == -1 || !bCommit) return;

	try {
		switch(nCol) {
		case 2: //Name
			{
				CString strNew = VarString(varNewValue);
				strNew.TrimLeft();
				strNew.TrimRight();
				ExecuteSql("UPDATE EmrCategoriesT SET Name = '%s' WHERE ID = %li", _Q(strNew), VarLong(m_pCategories->GetValue(nRow, 0)));
				if(!m_bIsLevel1EMR)
					m_pCategories->Sort();
				break;
			}
			
		case 3: //Format
			ExecuteSql("UPDATE EmrCategoriesT SET Format = %li WHERE ID = %li", VarLong(varNewValue), VarLong(m_pCategories->GetValue(nRow, 0)));
			if(!m_bIsLevel1EMR)
				m_pCategories->Sort();
			break;
		}
	}NxCatchAll("Error in CEmrCategoriesDlg::OnEditingFinishedEmrCategoryList()");
}

void CEmrCategoriesDlg::OnEmrCategoryUp() 
{
	if(m_pCategories->CurSel == -1 || m_pCategories->CurSel == 0) return;

	try {
		long nSourceID = VarLong(m_pCategories->GetValue(m_pCategories->CurSel,0));
		long nSourcePriority = VarLong(m_pCategories->GetValue(m_pCategories->CurSel,1));
		long nDestID = VarLong(m_pCategories->GetValue(m_pCategories->CurSel-1,0));
		long nDestPriority = VarLong(m_pCategories->GetValue(m_pCategories->CurSel-1,1));

		//Switch.
		ExecuteSql("UPDATE EmrCategoriesT SET Priority = (CASE WHEN ID = %li THEN %li WHEN ID = %li THEN %li END )WHERE ID = %li OR ID = %li",
			nSourceID, nDestPriority, nDestID, nSourcePriority, nSourceID, nDestID);

		m_pCategories->Requery();
		m_pCategories->SetSelByColumn(0, nSourceID);
	}NxCatchAll("Error in CEmrCategoriesDlg::OnEmrCategoryDown()");
}

void CEmrCategoriesDlg::OnEmrCategoryDown() 
{
	if(m_pCategories->CurSel == -1 || m_pCategories->CurSel == m_pCategories->GetRowCount()-1) return;

	try {
		long nSourceID = VarLong(m_pCategories->GetValue(m_pCategories->CurSel,0));
		long nSourcePriority = VarLong(m_pCategories->GetValue(m_pCategories->CurSel,1));
		long nDestID = VarLong(m_pCategories->GetValue(m_pCategories->CurSel+1,0));
		long nDestPriority = VarLong(m_pCategories->GetValue(m_pCategories->CurSel+1,1));

		//Switch.
		ExecuteSql("UPDATE EmrCategoriesT SET Priority = CASE WHEN ID = %li THEN %li WHEN ID = %li THEN %li END WHERE ID = %li OR ID = %li",
			nSourceID, nDestPriority, nDestID, nSourcePriority, nSourceID, nDestID);

		m_pCategories->Requery();
		m_pCategories->SetSelByColumn(0, nSourceID);
	}NxCatchAll("Error in CEmrCategoriesDlg::OnEmrCategoryDown()");
}

void CEmrCategoriesDlg::OnSelChangedEmrCategoryList(long nNewSel) 
{
	if(nNewSel == sriNoRow){
		GetDlgItem(IDC_DELETE_EMR_CATEGORY)->EnableWindow(FALSE);
		GetDlgItem(IDC_EMR_CATEGORY_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_EMR_CATEGORY_DOWN)->EnableWindow(FALSE);
	} else if(nNewSel == 0){
		GetDlgItem(IDC_DELETE_EMR_CATEGORY)->EnableWindow(TRUE);
		GetDlgItem(IDC_EMR_CATEGORY_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_EMR_CATEGORY_DOWN)->EnableWindow(TRUE);
	} else if(nNewSel == m_pCategories->GetRowCount() - 1){
		GetDlgItem(IDC_DELETE_EMR_CATEGORY)->EnableWindow(TRUE);
		GetDlgItem(IDC_EMR_CATEGORY_UP)->EnableWindow(TRUE);
		GetDlgItem(IDC_EMR_CATEGORY_DOWN)->EnableWindow(FALSE);
	}else{
		GetDlgItem(IDC_DELETE_EMR_CATEGORY)->EnableWindow(TRUE);
		GetDlgItem(IDC_EMR_CATEGORY_UP)->EnableWindow(TRUE);
		GetDlgItem(IDC_EMR_CATEGORY_DOWN)->EnableWindow(TRUE);
	}
}

void CEmrCategoriesDlg::OnRequeryFinishedEmrCategoryList(short nFlags) 
{
	int row = m_pCategories->GetCurSel();
	if(row != -1) {
		m_pCategories->EnsureRowVisible(row);
		OnSelChangedEmrCategoryList(row);
	}
}

//DRT 6/2/2008 - PLID 30230 - Added OnCancel handlers to keep behavior the same as pre-NxDialog changes
void CEmrCategoriesDlg::OnCancel()
{
	//Eat the message
}
