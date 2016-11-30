// EditVisitTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "EditVisitTypeDlg.h"
#include "GlobalDataUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEditVisitTypeDlg dialog


CEditVisitTypeDlg::CEditVisitTypeDlg(CWnd* pParent)
	: CNxDialog(CEditVisitTypeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditVisitTypeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditVisitTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditVisitTypeDlg)
	DDX_Control(pDX, IDC_EYE_TYPE_DOWN, m_btnDown);
	DDX_Control(pDX, IDC_EYE_TYPE_UP, m_btnUp);
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditVisitTypeDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditVisitTypeDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_EYE_TYPE_UP, OnUp)
	ON_BN_CLICKED(IDC_EYE_TYPE_DOWN, OnDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditVisitTypeDlg message handlers

BOOL CEditVisitTypeDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 10:22) - PLID 29817 - NxIconify the buttons
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_CLOSE);
		
		m_pTypeList = BindNxDataListCtrl(IDC_VISIT_TYPES);

		m_btnUp.AutoSet(NXB_UP);
		m_btnDown.AutoSet(NXB_DOWN);
	}
	NxCatchAll("Error in CEditVisitTypeDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEditVisitTypeDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditVisitTypeDlg)
	ON_EVENT(CEditVisitTypeDlg, IDC_VISIT_TYPES, 10 /* EditingFinished */, OnEditingFinishedVisitTypes, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEditVisitTypeDlg, IDC_VISIT_TYPES, 18 /* RequeryFinished */, OnRequeryFinishedVisitTypes, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditVisitTypeDlg::OnEditingFinishedVisitTypes(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		switch(nCol) {
		case 1:
			if(varNewValue.vt == VT_BSTR) {
				ExecuteSql("UPDATE EyeVisitTypesT SET Type = '%s' WHERE ID = %li", _Q(VarString(varNewValue)), VarLong(m_pTypeList->GetValue(nRow, 0)));
			}
			break;
		default:
			//This should never happen.
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error in CEditVisitTypeDlg::OnEditingFinishedVisitTypes()");
}

void CEditVisitTypeDlg::OnAdd() 
{
	try {
		long nNewID = NewNumber("EyeVisitTypesT", "ID");
		long nNewOrder = NewNumber("EyeVisitTypesT", "TypeOrder");
		ExecuteSql("INSERT INTO EyeVisitTypesT (ID, Type, TypeOrder) VALUES (%li, '', %li)", nNewID, nNewOrder);
		IRowSettingsPtr pRow = m_pTypeList->GetRow(-1);
		pRow->PutValue(0, nNewID);
		pRow->PutValue(1, _bstr_t(""));
		pRow->PutValue(2, nNewOrder);
		m_pTypeList->AddRow(pRow);
		m_pTypeList->StartEditing(m_pTypeList->GetRowCount()-1, 1);
	}NxCatchAll("Error in CEditVisitTypeDlg::OnAdd()");
}

void CEditVisitTypeDlg::OnDelete() 
{
	try {
	
		// (j.gruber 2007-02-20 15:17) - PLID 23655 - make sure something is selected
		if (m_pTypeList->CurSel == -1) {
			MsgBox("Please select a Visit Type to delete.");
			return;
		}


		//First, we can't delete the "Pre-op" type.
		_variant_t varID = m_pTypeList->GetValue(m_pTypeList->CurSel, 0);
		if(varID.vt == VT_I4) {
			long nID = VarLong(varID);
			if(nID == 1) {
				MsgBox("The pre-operative visit type is required by the system and cannot be moved or deleted, although it can be renamed if desired.");
			}
			else {
				//Is this in use?
				if(ReturnsRecords("SELECT ID FROM EyeVisitsT WHERE VisitType = %li", nID)) {
					MsgBox("The visit type you are trying to delete is in use, and so cannot be deleted.");
				}
				else {
					if(AfxMessageBox("Are you sure you wish to delete this visit type?", MB_YESNO) == IDNO)
						return;

					//OK, let's delete.
					ExecuteSql("DELETE FROM EyeVisitTypesT WHERE ID = %li", nID);
					m_nSelectedID = -1;
					m_pTypeList->Requery();
				}
			}
		}
	}NxCatchAll("Error in CEditVisitTypeDlg::OnDelete()");
}

void CEditVisitTypeDlg::OnUp() 
{
	try {
		//If we're on the top one, we can't move it (duh).
		if(m_pTypeList->CurSel == 0 || m_pTypeList->CurSel == -1) {
			return;
		}
		//Also, if it's the second one, we can't move it because the pre-op has to stay first.
		if(m_pTypeList->CurSel == 1) {
			MsgBox("The pre-operative visit type is required by the system and cannot be moved or deleted, although it can be renamed if desired.");
		}
		else {
			//OK, let's move it up.
			//Let's get the value of the higher row.
			long nHigherID = VarLong(m_pTypeList->GetValue(m_pTypeList->CurSel-1, 0));
			long nHigherOrder = VarLong(m_pTypeList->GetValue(m_pTypeList->CurSel-1,2));
			//Let's get the value of our row.
			long nID = VarLong(m_pTypeList->GetValue(m_pTypeList->CurSel, 0));
			long nOrder = VarLong(m_pTypeList->GetValue(m_pTypeList->CurSel, 2));
			//Now, switch 'em.
			ExecuteSql("UPDATE EyeVisitTypesT SET TypeOrder = %li WHERE ID = %li", nHigherOrder, nID);
			ExecuteSql("UPDATE EyeVisitTypesT SET TypeOrder = %li WHERE ID = %li", nOrder, nHigherID);
			m_nSelectedID = nID;
			m_pTypeList->Requery();
		}
	}NxCatchAll("Error in CEditVisitTypeDlg::OnUp()");


}

void CEditVisitTypeDlg::OnDown() 
{
	try {
		//If we're on the bottom one, we can't move it (duh).
		if(m_pTypeList->CurSel == -1 || m_pTypeList->CurSel == m_pTypeList->GetRowCount()-1) {
			return;
		}
		//Also, if we're on the top one, it can't be moved.
		if(m_pTypeList->CurSel == 0) {
			MsgBox("The pre-operative visit type is required by the system and cannot be moved or deleted, although it can be renamed if desired.");
		}
		else {
			//OK, let's move it down.
			//Let's get the value of the lower row.
			long nLowerID = VarLong(m_pTypeList->GetValue(m_pTypeList->CurSel+1, 0));
			long nLowerOrder = VarLong(m_pTypeList->GetValue(m_pTypeList->CurSel+1,2));
			//Let's get the value of our row.
			long nID = VarLong(m_pTypeList->GetValue(m_pTypeList->CurSel, 0));
			long nOrder = VarLong(m_pTypeList->GetValue(m_pTypeList->CurSel, 2));
			//Now, switch 'em.
			ExecuteSql("UPDATE EyeVisitTypesT SET TypeOrder = %li WHERE ID = %li", nLowerOrder, nID);
			ExecuteSql("UPDATE EyeVisitTypesT SET TypeOrder = %li WHERE ID = %li", nOrder, nLowerID);
			m_nSelectedID = nID;
			m_pTypeList->Requery();
		}
	}NxCatchAll("Error in CEditVisitTypeDlg::OnDown()");


}

void CEditVisitTypeDlg::OnOK() 
{	
	CDialog::OnOK();
}

void CEditVisitTypeDlg::OnRequeryFinishedVisitTypes(short nFlags) 
{
	m_pTypeList->SetSelByColumn(0, m_nSelectedID);
}
