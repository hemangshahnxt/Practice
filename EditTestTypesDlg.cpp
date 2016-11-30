// EditTestTypesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditTestTypesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEditTestTypesDlg dialog


CEditTestTypesDlg::CEditTestTypesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditTestTypesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditTestTypesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditTestTypesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditTestTypesDlg)
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditTestTypesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditTestTypesDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditTestTypesDlg message handlers

BOOL CEditTestTypesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnAdd.AutoSet(NXB_NEW);
	m_btnDelete.AutoSet(NXB_DELETE);
	m_btnClose.AutoSet(NXB_CLOSE);
	
	m_pTestsList = BindNxDataListCtrl(this, IDC_TEST_TYPES_LIST, GetRemoteData(), true);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditTestTypesDlg::OnOK() 
{	
	CDialog::OnOK();
}

void CEditTestTypesDlg::OnAdd() 
{
	try {
		long nNewID = NewNumber("EyeTestTypesT", "TestID");
		ExecuteSql("INSERT INTO EyeTestTypesT (TestID, TestName) VALUES (%li, '')", nNewID);
		IRowSettingsPtr pRow = m_pTestsList->GetRow(-1);
		pRow->PutValue(0, nNewID);
		pRow->PutValue(1, _bstr_t(""));
		m_pTestsList->AddRow(pRow);
		m_pTestsList->StartEditing(m_pTestsList->GetRowCount()-1, 1);
	}NxCatchAll("Error adding new test type.");
}

void CEditTestTypesDlg::OnDelete() 
{
	try {
		_variant_t varID = m_pTestsList->GetValue(m_pTestsList->CurSel, 0);
		if(varID.vt == VT_I4) {
			long nID = VarLong(varID);
			//Is this in use?
			if(ReturnsRecords("SELECT ID FROM EyeTestsT WHERE TestID = %li", nID)) {
				MsgBox("The test type you are trying to delete is in use and cannot be deleted.");
			}
			else {
				if(AfxMessageBox("Are you sure you wish to delete this test type?", MB_YESNO) == IDNO)
					return;

				//OK, let's delete.
				ExecuteSql("DELETE FROM EyeTestTypesT WHERE TestID = %li", nID);
				m_pTestsList->Requery();
			}
		}
	}NxCatchAll("Error deleting test type.");
}

BEGIN_EVENTSINK_MAP(CEditTestTypesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditTestTypesDlg)
	ON_EVENT(CEditTestTypesDlg, IDC_TEST_TYPES_LIST, 10 /* EditingFinished */, OnEditingFinishedTestTypesList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditTestTypesDlg::OnEditingFinishedTestTypesList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		switch(nCol) {
		case 1:
			if(varNewValue.vt == VT_BSTR) {
				ExecuteSql("UPDATE EyeTestTypesT SET TestName = '%s' WHERE TestID = %li", _Q(VarString(varNewValue)), VarLong(m_pTestsList->GetValue(nRow, 0)));
			}
			break;
		default:
			//This should never happen.
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error saving new test.");
}


