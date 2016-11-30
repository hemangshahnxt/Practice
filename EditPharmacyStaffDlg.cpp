// EditPharmacyStaffDlg.cpp : implementation file
//

//DRT 11/19/2008 - PLID 32093 - Created

#include "stdafx.h"
#include "Practice.h"
#include "EditPharmacyStaffDlg.h"
#include "EditPharmacyStaffIndivDlg.h"


// CEditPharmacyStaffDlg dialog

IMPLEMENT_DYNAMIC(CEditPharmacyStaffDlg, CNxDialog)

CEditPharmacyStaffDlg::CEditPharmacyStaffDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditPharmacyStaffDlg::IDD, pParent)
{
	m_nLocationID = -1;
}

CEditPharmacyStaffDlg::~CEditPharmacyStaffDlg()
{
}

void CEditPharmacyStaffDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PHARMACY_NAME, m_nxstaticPharmacyName);
	DDX_Control(pDX, IDC_ADD_PHARMACIST, m_btnAddPharmacist);
	DDX_Control(pDX, IDC_EDIT_PHARMACIST, m_btnEditPharmacist);
	DDX_Control(pDX, IDC_REMOVE_PHARMACIST, m_btnRemovePharmacist);
	DDX_Control(pDX, IDC_ADD_STAFF, m_btnAddStaff);
	DDX_Control(pDX, IDC_EDIT_STAFF, m_btnEditStaff);
	DDX_Control(pDX, IDC_REMOVE_STAFF, m_btnRemoveStaff);
	DDX_Control(pDX, IDC_PHARM_STAFF_CLOSE, m_btnClose);
}


BEGIN_MESSAGE_MAP(CEditPharmacyStaffDlg, CNxDialog)
	ON_BN_CLICKED(IDC_PHARM_STAFF_CLOSE, &CEditPharmacyStaffDlg::OnBnClickedPharmStaffClose)
	ON_BN_CLICKED(IDC_ADD_PHARMACIST, &CEditPharmacyStaffDlg::OnBnClickedAddPharmacist)
	ON_BN_CLICKED(IDC_EDIT_PHARMACIST, &CEditPharmacyStaffDlg::OnBnClickedEditPharmacist)
	ON_BN_CLICKED(IDC_REMOVE_PHARMACIST, &CEditPharmacyStaffDlg::OnBnClickedRemovePharmacist)
	ON_BN_CLICKED(IDC_ADD_STAFF, &CEditPharmacyStaffDlg::OnBnClickedAddStaff)
	ON_BN_CLICKED(IDC_EDIT_STAFF, &CEditPharmacyStaffDlg::OnBnClickedEditStaff)
	ON_BN_CLICKED(IDC_REMOVE_STAFF, &CEditPharmacyStaffDlg::OnBnClickedRemoveStaff)
END_MESSAGE_MAP()


// CEditPharmacyStaffDlg message handlers
BOOL CEditPharmacyStaffDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//Bind the datalists
		m_pPharmacistList = BindNxDataList2Ctrl(IDC_PHARMACIST_LIST, false);
		m_pStaffList = BindNxDataList2Ctrl(IDC_PHARMACY_STAFF_LIST, false);

		//Flag them to the current pharmacy
		m_pPharmacistList->WhereClause = _bstr_t( FormatString("PharmacyStaffT.Type = 0 AND PharmacyStaffT.PharmacyID = %li", m_nLocationID) );
		m_pPharmacistList->Requery();
		m_pStaffList->WhereClause = _bstr_t( FormatString("PharmacyStaffT.Type = 1 AND PharmacyStaffT.PharmacyID = %li", m_nLocationID) );
		m_pStaffList->Requery();


		//Load the given pharmacy name to the screen
		m_nxstaticPharmacyName.SetWindowText(m_strPharmacyName);

		//Set NxIconButton styles
		m_btnAddPharmacist.AutoSet(NXB_NEW);
		m_btnEditPharmacist.AutoSet(NXB_MODIFY);
		m_btnRemovePharmacist.AutoSet(NXB_DELETE);
		m_btnAddStaff.AutoSet(NXB_NEW);
		m_btnEditStaff.AutoSet(NXB_MODIFY);
		m_btnRemoveStaff.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}
void CEditPharmacyStaffDlg::OnBnClickedPharmStaffClose()
{
	try {
		//No validation needs done, we save as we go
		CDialog::OnOK();

	} NxCatchAll("Error in OnBnClickedPharmStaffClose");
}

void CEditPharmacyStaffDlg::OnBnClickedAddPharmacist()
{
	try {
		CEditPharmacyStaffIndivDlg dlg(this);
		if(dlg.EditStaff(m_nLocationID, -1, 0) == IDOK) {
			m_pPharmacistList->Requery();
		}

	} NxCatchAll("Error in OnBnClickedAddPharmacist");
}

void CEditPharmacyStaffDlg::OnBnClickedEditPharmacist()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPharmacistList->CurSel;
		if(pRow == NULL) {
			return;
		}

		CEditPharmacyStaffIndivDlg dlg(this);
		if(dlg.EditStaff(m_nLocationID, VarLong(pRow->GetValue(0)), 0) == IDOK) {
			m_pPharmacistList->Requery();
		}
	} NxCatchAll("Error in OnBnClickedEditPharmacist");
}

void CEditPharmacyStaffDlg::OnBnClickedRemovePharmacist()
{
	try {
		//ensure row selected
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPharmacistList->CurSel;
		if(pRow == NULL) {
			return;
		}

		//confirm
		if(AfxMessageBox("Are you sure you wish to permanently delete this pharmacist?", MB_YESNO) != IDYES) {
			return;
		}

		//Delete it
		DeletePharmacyStaffRecords( VarLong(pRow->GetValue(0)) );

		//Remove the row
		m_pPharmacistList->RemoveRow(pRow);

	} NxCatchAll("Error in OnBnClickedRemovePharmacist");
}

void CEditPharmacyStaffDlg::OnBnClickedAddStaff()
{
	try {
		CEditPharmacyStaffIndivDlg dlg(this);
		if(dlg.EditStaff(m_nLocationID, -1, 1) == IDOK) {
			m_pStaffList->Requery();
		}

	} NxCatchAll("Error in OnBnClickedAddStaff");
}

void CEditPharmacyStaffDlg::OnBnClickedEditStaff()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pStaffList->CurSel;
		if(pRow == NULL) {
			return;
		}

		CEditPharmacyStaffIndivDlg dlg(this);
		if(dlg.EditStaff(m_nLocationID, VarLong(pRow->GetValue(0)), 1) == IDOK) {
			m_pStaffList->Requery();
		}
	} NxCatchAll("Error in OnBnClickedEditStaff");
}

void CEditPharmacyStaffDlg::OnBnClickedRemoveStaff()
{
	try {
		//ensure row selected
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pStaffList->CurSel;
		if(pRow == NULL) {
			return;
		}

		//confirm
		if(AfxMessageBox("Are you sure you wish to permanently delete this pharmacy staff member?", MB_YESNO) != IDYES) {
			return;
		}

		//Delete it
		DeletePharmacyStaffRecords( VarLong(pRow->GetValue(0)) );

		//Remove from interface
		m_pStaffList->RemoveRow(pRow);
	} NxCatchAll("Error in OnBnClickedRemoveStaff");
}
BEGIN_EVENTSINK_MAP(CEditPharmacyStaffDlg, CNxDialog)
	ON_EVENT(CEditPharmacyStaffDlg, IDC_PHARMACIST_LIST, 3, CEditPharmacyStaffDlg::OnDblClickPharmacistList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEditPharmacyStaffDlg, IDC_PHARMACY_STAFF_LIST, 3, CEditPharmacyStaffDlg::OnDblClickCellPharmacyStaffList, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CEditPharmacyStaffDlg::OnDblClickPharmacistList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		//Just fire the 'edit' button handler
		OnBnClickedEditPharmacist();

	} NxCatchAll("Error in OnDblClickPharmacistList");
}

void CEditPharmacyStaffDlg::OnDblClickCellPharmacyStaffList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		//Just fire the 'edit' button handler
		OnBnClickedEditStaff();
	} NxCatchAll("Error in OnDblClickCellPharmacyStaffList");
}

void CEditPharmacyStaffDlg::DeletePharmacyStaffRecords(long nPersonID)
{
	ExecuteParamSql(
		"DELETE FROM PharmacyStaffT WHERE PersonID = {INT};\r\n"
		"DELETE FROM PersonT WHERE ID = {INT};\r\n", nPersonID, nPersonID);
}
