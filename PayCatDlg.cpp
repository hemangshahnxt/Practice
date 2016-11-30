// PayCatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "PayCatDlg.h"
#include "GetNewIDName.h"
#include "EditComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CPayCatDlg dialog

// (j.gruber 2012-11-15 14:55) - PLID 53766
#define ID_SET_DEFAULT		53654
#define ID_REMOVE_DEFAULT	53655

enum PayCatColumns {
	pccID = 0,
	pccName = 1,
	pccDescription = 2,
};


CPayCatDlg::CPayCatDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPayCatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPayCatDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPayCatDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPayCatDlg)
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPayCatDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPayCatDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)	
	ON_COMMAND(ID_SET_DEFAULT, OnSetDefault)
	ON_COMMAND(ID_REMOVE_DEFAULT, OnRemoveDefault)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPayCatDlg message handlers

BEGIN_EVENTSINK_MAP(CPayCatDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPayCatDlg)
	ON_EVENT(CPayCatDlg, IDC_CATEGORIES, 17 /* RepeatedLeftClick */, OnRepeatedLeftClickCategories, VTS_VARIANT VTS_I4 VTS_I4)
	ON_EVENT(CPayCatDlg, IDC_CATEGORIES, 10 /* EditingFinished */, OnEditingFinishedCategories, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPayCatDlg, IDC_CATEGORIES, 2 /* SelChanged */, OnSelChangedCategories, VTS_I4)
	ON_EVENT(CEditComboBox, IDC_CATEGORIES, 6 /* RButtonDown */, OnRButtonDownEditList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEditComboBox, IDC_CATEGORIES, 8 /* EditingStarting */, OnEditingStartingList, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CPayCatDlg::OnRepeatedLeftClickCategories(const VARIANT FAR& varBoundValue, long iColumn, long nClicks) 
{/*
	if(nClicks > 1 && varBoundValue.vt == VT_I4
		&& (iColumn == 1 || iColumn == 2)) 
		m_categories.Edit(varBoundValue, iColumn);*/
}

BOOL CPayCatDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/30/2008) - PLID 29850 - Set button styles
	m_btnAdd.AutoSet(NXB_NEW);
	m_btnDelete.AutoSet(NXB_DELETE);
	m_btnClose.AutoSet(NXB_CLOSE);

	m_pCategories = BindNxDataListCtrl(this,IDC_CATEGORIES,GetRemoteData(),true);

	GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);

	// (j.gruber 2012-11-15 15:11) - PLID 53766 - - bulk cache
	g_propManager.CachePropertiesInBulk("PatientGeneral1", propNumber,
				"(Username = '<None>' OR Username = '%s' OR Username = '%s') AND ("
				"Name = 'DefaultPayCat' "			
				")",
				_Q(GetCurrentUserName()), _Q(GetCurrentUserComputerName()) );

	// (j.gruber 2012-11-15 15:11) - PLID 53766 
	m_nDefault = GetRemotePropertyInt("DefaultPayCat",-1,0,"<None>",TRUE);
	int nRow = m_pCategories->FindByColumn(pccID,(long)m_nDefault,0,FALSE);
	if(nRow >= 0) {
		NXDATALISTLib::IRowSettingsPtr(m_pCategories->GetRow(nRow))->PutForeColor(RGB(255,0,0));
	}

	return TRUE; 
}

void CPayCatDlg::OnAdd() 
{
	// (j.gruber 2012-11-16 09:53) - PLID 39557 - Permissions
	if (!CheckCurrentUserPermissions(bioPaymentCategories, sptCreate))
		return;

	CGetNewIDName NewCategory(this);
	CString NewName,
			sql;

	NewCategory.m_pNewName = &NewName;
	NewCategory.m_nMaxLength = 150;
	if (NewCategory.DoModal() == IDOK) {
		try {
			ExecuteSql("INSERT INTO PaymentGroupsT (ID,GroupName,Explanation) VALUES (%li,'%s','')",NewNumber("PaymentGroupsT","ID"),_Q(NewName));
			CClient::RefreshTable(NetUtils::PaymentGroupsT);
		}NxCatchAll("Error in adding Payment Category.");
	}
	m_pCategories->Requery();
}

void CPayCatDlg::OnDelete() 
{
	try {

		// (j.gruber 2012-11-16 09:54) - PLID 39557
		if (!CheckCurrentUserPermissions(bioPaymentCategories, sptDelete))
			return;

		if(m_pCategories->CurSel == -1) {
			return;
		}

		if (m_pCategories->GetValue(m_pCategories->CurSel,0).vt == VT_I4)
		{	
			try{
				// (c.haag 2007-02-20 14:26) - PLID 24255 - Warn the user if this category is in use by payments
				const long nGroupID = VarLong(m_pCategories->GetValue(m_pCategories->CurSel,0));
				if (ReturnsRecords("SELECT 1 FROM PaymentsT INNER JOIN LineItemT ON LineItemT.ID = PaymentsT.ID WHERE PaymentGroupID = %d AND LineItemT.Deleted = 0", nGroupID)) {
					if (IDNO == MessageBox("This payment category is in use by payments. Do you wish to continue?", "Delete?", MB_YESNO | MB_ICONQUESTION)) {
						return;
					}
				}
				// (c.haag 2007-02-20 14:29) - PLID 24255 - Warn the user if this category is in use by e-remittance preferences
				if (ReturnsRecords("SELECT 1 FROM ConfigRT WHERE IntParam = %d AND Name IN ('DefaultERemitPayCategory', 'DefaultERemitAdjCategory')", nGroupID)) {
					if (IDNO == MessageBox("This payment category is used as a default category for E-Remittance processing. Do you wish to continue?", "Delete?", MB_YESNO | MB_ICONQUESTION)) {
						return;
					}
				}

				// (j.jones 2008-07-10 11:59) - PLID 28756 - Warn the user if this category is in use by any insurance company
				if (ReturnsRecords("SELECT TOP 1 PersonID FROM InsuranceCoT WHERE DefaultPayCategoryID = %li OR DefaultAdjCategoryID = %li", nGroupID, nGroupID)) {
					if (IDNO == MessageBox("This payment category is used as a default category for at least one insurance company. Do you wish to continue?", "Delete?", MB_YESNO | MB_ICONQUESTION)) {
						return;
					}
				}

				// (j.jones 2008-07-10 11:59) - PLID 28756 - clear out its usage on insurance companies
				ExecuteParamSql("UPDATE InsuranceCoT SET DefaultPayCategoryID = NULL WHERE DefaultPayCategoryID = {INT}", nGroupID);
				ExecuteParamSql("UPDATE InsuranceCoT SET DefaultAdjCategoryID = NULL WHERE DefaultAdjCategoryID = {INT}", nGroupID);
				ExecuteParamSql("DELETE FROM PaymentGroupsT WHERE ID = {INT}", nGroupID);
				CClient::RefreshTable(NetUtils::PaymentGroupsT);

				// (j.gruber 2012-11-16 10:32) - PLID 53766
				if(IsDefaultSelected()) {
					SetRemotePropertyInt("DefaultPayCat",-1,0,"<None>");
					m_nDefault = -1;
				}
			}NxCatchAll("Error in adding Payment Category.");

			m_pCategories->Requery();
			GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);

		}
	}NxCatchAll("Error in CPayCatDlg::OnDelete()");

}

void CPayCatDlg::OnEditingFinishedCategories(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if(nRow==-1)
		return;
	if(varNewValue.vt == VT_NULL || varNewValue.vt == VT_EMPTY)
		return;
	switch(nCol) {
	case 1:
		try {

			CString str = VarString(varNewValue, "");
			str = str.Trim(" "); 
			// (b.spivey, March 06, 2013) - PLID 51186 - Parametize this, prevent "   " entries. 
			ExecuteParamSql("UPDATE PaymentGroupsT "
				"SET GroupName = {STRING} "
				"WHERE ID = {INT} ", str, VarLong(m_pCategories->GetValue(nRow,0)));

			CClient::RefreshTable(NetUtils::PaymentGroupsT);
		} NxCatchAll("Error in editing payment category.");
		break;
	case 2:
		try {

			CString str = VarString(varNewValue, "");
			str = str.Trim(" "); 
			// (b.spivey, March 06, 2013) - PLID 51186 - Parametize this, prevent "   " entries. 
			ExecuteParamSql("UPDATE PaymentGroupsT "
				"SET Explanation = {STRING} "
				"WHERE ID = {INT} ", str, VarLong(m_pCategories->GetValue(nRow,0))); 

			CClient::RefreshTable(NetUtils::PaymentGroupsT);
		} NxCatchAll("Error in editing payment category.");
		break;
	}	
}

void CPayCatDlg::OnSelChangedCategories(long nNewSel) 
{
	GetDlgItem(IDC_DELETE)->EnableWindow(nNewSel == -1 ? FALSE : TRUE);
}


// (j.gruber 2012-11-15 14:33) - PLID 53766 - support the default category
void CPayCatDlg::OnRButtonDownEditList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		if (nRow == -1) {
			return;
		}
		NXDATALISTLib::IRowSettingsPtr pRow = m_pCategories->GetRow(nRow);
		if (pRow) {

			//set this as the cur sel
			m_pCategories->CurSel = nRow;

			CMenu menPopup;
			menPopup.m_hMenu = CreatePopupMenu();

			if (IsDefaultSelected() ) {
				//its already the default, pop up a menu to undefault it
				menPopup.InsertMenu(0, MF_BYPOSITION, ID_REMOVE_DEFAULT, "Remove Default");
			}
			else {
				//pop up a menu to default it
				menPopup.InsertMenu(0, MF_BYPOSITION, ID_SET_DEFAULT, "Set As Default");				
			}	
			

			CPoint pt(x,y);
			CWnd* pWnd = GetDlgItem(IDC_CATEGORIES);
			if (pWnd != NULL)
			{	pWnd->ClientToScreen(&pt);
				menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
			}
			else{
				HandleException(NULL, "An error ocurred while creating menu");
			}
		}
	}NxCatchAll(__FUNCTION__)
}

// (j.gruber 2012-11-16 09:54) - PLID 39557 - added to check permissions
void CPayCatDlg::OnEditingStartingList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {
		if (nRow == -1) {
			return;
		}
		if (!CheckCurrentUserPermissions(bioPaymentCategories, sptWrite)) {
			*pbContinue = false;
			return;
		}

	}NxCatchAll(__FUNCTION__)
}

// (j.gruber 2012-11-15 15:12) - PLID 53766
void CPayCatDlg::OnSetDefault()
{
	try {

		// (j.gruber 2012-11-16 10:20) - PLID 59557 - check permissions
		if (!CheckCurrentUserPermissions(bioPaymentCategories, sptWrite)) {		
			return;
		}

		//first remove colors		
		int nRow = m_pCategories->FindByColumn(pccID,(long)m_nDefault,0,FALSE);
		if(nRow >= 0)
			NXDATALISTLib::IRowSettingsPtr(m_pCategories->GetRow(nRow))->PutForeColor(RGB(0,0,0));

		//save the default
		long nNewDefault = VarLong(m_pCategories->GetValue(m_pCategories->CurSel,pccID));
		SetRemotePropertyInt("DefaultPayCat",nNewDefault,0,"<None>");
		m_nDefault = nNewDefault;

		//now set colors
		NXDATALISTLib::IRowSettingsPtr(m_pCategories->GetRow(m_pCategories->CurSel))->PutForeColor(RGB(255,0,0));	
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-11-15 15:12) - PLID 53766
void CPayCatDlg::OnRemoveDefault()
{
	try {

		// (j.gruber 2012-11-16 10:20) - PLID 39557 - check permissions
		if (!CheckCurrentUserPermissions(bioPaymentCategories, sptWrite)) {			
			return;
		}

		//first remove colors		
		int nRow = m_pCategories->FindByColumn(pccID,m_nDefault,0,FALSE);
		if(nRow >= 0) {
			NXDATALISTLib::IRowSettingsPtr(m_pCategories->GetRow(nRow))->PutForeColor(RGB(0,0,0));
		}

		//now remove the default
		SetRemotePropertyInt("DefaultPayCat",-1,0,"<None>");
		m_nDefault = -1;
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-11-16 10:34) - PLID 53766
BOOL CPayCatDlg::IsDefaultSelected()
{
	long nCurSel = m_pCategories->CurSel;
	
	if (nCurSel != -1) {

		//is this ID already the default ID		
		long nID = VarLong(m_pCategories->GetValue(m_pCategories->CurSel,pccID));

		if (m_nDefault == nID) {
			return TRUE;
		}
	}
	return FALSE;
}