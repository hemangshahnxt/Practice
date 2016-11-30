// EditVisionRatingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditVisionRatingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEditVisionRatingsDlg dialog


CEditVisionRatingsDlg::CEditVisionRatingsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditVisionRatingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditVisionRatingsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditVisionRatingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditVisionRatingsDlg)
	DDX_Control(pDX, IDC_EYE_RATING_DOWN, m_btnDown);
	DDX_Control(pDX, IDC_EYE_RATING_UP, m_btnUp);
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditVisionRatingsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditVisionRatingsDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_EYE_RATING_DOWN, OnEyeRatingDown)
	ON_BN_CLICKED(IDC_EYE_RATING_UP, OnEyeRatingUp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditVisionRatingsDlg message handlers

BOOL CEditVisionRatingsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_pRatingList = BindNxDataListCtrl(this,IDC_VISION_RATINGS,GetRemoteData(),true);

	m_btnUp.AutoSet(NXB_UP);
	m_btnDown.AutoSet(NXB_DOWN);
	m_btnAdd.AutoSet(NXB_NEW);
	m_btnDelete.AutoSet(NXB_DELETE);
	m_btnClose.AutoSet(NXB_CLOSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditVisionRatingsDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CEditVisionRatingsDlg::OnAdd() 
{
	try {
		long nNewID = NewNumber("EyeVisitRatingsT", "ID");
		long nNewOrder = NewNumber("EyeVisitRatingsT", "RatingOrder");
		ExecuteSql("INSERT INTO EyeVisitRatingsT (ID, Rating, RatingOrder) VALUES (%li, '', %li)", nNewID, nNewOrder);
		IRowSettingsPtr pRow = m_pRatingList->GetRow(-1);
		pRow->PutValue(0, nNewID);
		pRow->PutValue(1, _bstr_t(""));
		pRow->PutValue(2, nNewOrder);
		m_pRatingList->AddRow(pRow);
		m_pRatingList->StartEditing(m_pRatingList->GetRowCount()-1, 1);
	}NxCatchAll("Error adding new rating.");
}

void CEditVisionRatingsDlg::OnDelete() 
{
	try {
		_variant_t varID = m_pRatingList->GetValue(m_pRatingList->CurSel, 0);
		if(varID.vt == VT_I4) {
			long nID = VarLong(varID);
			//Is this in use?
			/*
			if(ReturnsRecords("SELECT ID FROM EyeVisitsT WHERE VisitType = %li", nID)) {
				MsgBox("The visit type you are trying to delete is in use, and so cannot be deleted.");
			}
			else {*/
				if(AfxMessageBox("Are you sure you wish to delete this visit type?", MB_YESNO) == IDNO)
					return;

				//OK, let's delete.
				ExecuteSql("DELETE FROM EyeVisitRatingsT WHERE ID = %li", nID);
				m_nSelectedID = -1;
				m_pRatingList->Requery();
			//}
		}
	}NxCatchAll("Error deleting rating");
}

void CEditVisionRatingsDlg::OnEyeRatingDown() 
{
	try {
		//If we're on the bottom one, we can't move it (duh).
		if(m_pRatingList->CurSel == -1 || m_pRatingList->CurSel == m_pRatingList->GetRowCount()-1) {
			return;
		}

		//Let's get the value of the lower row.
		long nLowerID = VarLong(m_pRatingList->GetValue(m_pRatingList->CurSel+1, 0));
		long nLowerOrder = VarLong(m_pRatingList->GetValue(m_pRatingList->CurSel+1,2));
		//Let's get the value of our row.
		long nID = VarLong(m_pRatingList->GetValue(m_pRatingList->CurSel, 0));
		long nOrder = VarLong(m_pRatingList->GetValue(m_pRatingList->CurSel, 2));
		//Now, switch 'em.
		ExecuteSql("UPDATE EyeVisitRatingsT SET RatingOrder = %li WHERE ID = %li", nLowerOrder, nID);
		ExecuteSql("UPDATE EyeVisitRatingsT SET RatingOrder = %li WHERE ID = %li", nOrder, nLowerID);
		m_nSelectedID = nID;
		m_pRatingList->Requery();
	}NxCatchAll("Error moving item down.");
}

void CEditVisionRatingsDlg::OnEyeRatingUp() 
{
	try {
		//If we're on the top one, we can't move it (duh).
		if(m_pRatingList->CurSel == 0 || m_pRatingList->CurSel == -1) {
			return;
		}

		//Let's get the value of the higher row.
		long nHigherID = VarLong(m_pRatingList->GetValue(m_pRatingList->CurSel-1, 0));
		long nHigherOrder = VarLong(m_pRatingList->GetValue(m_pRatingList->CurSel-1,2));
		//Let's get the value of our row.
		long nID = VarLong(m_pRatingList->GetValue(m_pRatingList->CurSel, 0));
		long nOrder = VarLong(m_pRatingList->GetValue(m_pRatingList->CurSel, 2));
		//Now, switch 'em.
		ExecuteSql("UPDATE EyeVisitRatingsT SET RatingOrder = %li WHERE ID = %li", nHigherOrder, nID);
		ExecuteSql("UPDATE EyeVisitRatingsT SET RatingOrder = %li WHERE ID = %li", nOrder, nHigherID);
		m_nSelectedID = nID;
		m_pRatingList->Requery();
	}NxCatchAll("Error moving item up.");
}

BEGIN_EVENTSINK_MAP(CEditVisionRatingsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditVisionRatingsDlg)
	ON_EVENT(CEditVisionRatingsDlg, IDC_VISION_RATINGS, 10 /* EditingFinished */, OnEditingFinishedVisionRatings, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEditVisionRatingsDlg, IDC_VISION_RATINGS, 18 /* RequeryFinished */, OnRequeryFinishedVisionRatings, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditVisionRatingsDlg::OnEditingFinishedVisionRatings(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		switch(nCol) {
		case 1:
			if(varNewValue.vt == VT_BSTR) {
				ExecuteSql("UPDATE EyeVisitRatingsT SET Rating = '%s' WHERE ID = %li", _Q(VarString(varNewValue)), VarLong(m_pRatingList->GetValue(nRow, 0)));
			}
			break;
		default:
			//This should never happen.
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error saving new rating name.");
}

void CEditVisionRatingsDlg::OnRequeryFinishedVisionRatings(short nFlags) 
{
	m_pRatingList->SetSelByColumn(0, m_nSelectedID);	
}
