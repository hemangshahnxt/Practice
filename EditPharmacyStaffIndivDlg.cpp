// EditPharmacyStaffIndivDlg.cpp : implementation file
//

//DRT 11/19/2008 - PLID 32093 - Created

#include "stdafx.h"
#include "Practice.h"
#include "EditPharmacyStaffIndivDlg.h"

using namespace ADODB;

// CEditPharmacyStaffIndivDlg dialog

IMPLEMENT_DYNAMIC(CEditPharmacyStaffIndivDlg, CNxDialog)

CEditPharmacyStaffIndivDlg::CEditPharmacyStaffIndivDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditPharmacyStaffIndivDlg::IDD, pParent)
{
	m_nPharmacyID = -1;
	m_nPersonID = -1;
	m_nType = -1;
}

CEditPharmacyStaffIndivDlg::~CEditPharmacyStaffIndivDlg()
{
}

void CEditPharmacyStaffIndivDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PHARM_INDIV_FIRST, m_editFirst);
	DDX_Control(pDX, IDC_PHARM_INDIV_MIDDLE, m_editMiddle);
	DDX_Control(pDX, IDC_PHARM_INDIV_LAST, m_editLast);
	DDX_Control(pDX, IDC_PHARM_INDIV_SUFFIX, m_editSuffix);
}


BEGIN_MESSAGE_MAP(CEditPharmacyStaffIndivDlg, CNxDialog)
END_MESSAGE_MAP()


// CEditPharmacyStaffIndivDlg message handlers
BOOL CEditPharmacyStaffIndivDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//bind datalist
		m_pPrefixList = BindNxDataList2Ctrl(IDC_PHARM_INDIV_PREFIX_LIST);

		//control setup
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//Set text limits
		m_editFirst.SetLimitText(50);
		m_editMiddle.SetLimitText(50);
		m_editLast.SetLimitText(50);
		m_editSuffix.SetLimitText(5);

		//Load everything from data
		LoadFromData();

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

void CEditPharmacyStaffIndivDlg::OnOK()
{
	try {
		if(!SaveToData()) {
			return;
		}

		//Success
		CDialog::OnOK();
	} NxCatchAll("Error in OnOK");
}

void CEditPharmacyStaffIndivDlg::OnCancel()
{
	//Just quit
	CDialog::OnCancel();
}

UINT CEditPharmacyStaffIndivDlg::EditStaff(long nPharmacyID, long nPersonID, long nType)
{
	m_nPharmacyID = nPharmacyID;
	m_nPersonID = nPersonID;
	m_nType = nType;

	return DoModal();
}

void CEditPharmacyStaffIndivDlg::LoadFromData()
{
	if(m_nPersonID == -1) {
		//New person, no data to load -- it's all new
	}
	else {
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Suffix, PersonT.PrefixID "
			"FROM PharmacyStaffT INNER JOIN PersonT ON PharmacyStaffT.PersonID = PersonT.ID "
			"WHERE PersonT.ID = {INT}", m_nPersonID);
		if(prs->eof) {
			//should not be possible, we cannot handle this
			AfxThrowNxException("Could not load pharmacy demographic information for staff %li", m_nPersonID);
		}

		//Load the string values
		SetDlgItemText(IDC_PHARM_INDIV_FIRST, AdoFldString(prs, "First"));
		SetDlgItemText(IDC_PHARM_INDIV_MIDDLE, AdoFldString(prs, "Middle"));
		SetDlgItemText(IDC_PHARM_INDIV_LAST, AdoFldString(prs, "Last"));
		SetDlgItemText(IDC_PHARM_INDIV_SUFFIX, AdoFldString(prs, "Suffix"));

		//Load the prefix datalist value
		long nPrefixID = AdoFldLong(prs, "PrefixID", -1);
		if(nPrefixID != -1) {
			m_pPrefixList->SetSelByColumn(0, (long)nPrefixID);
		}
	}
}

bool CEditPharmacyStaffIndivDlg::SaveToData()
{
	//Get the values we'll be using
	CString strFirst, strMiddle, strLast, strSuffix;
	_variant_t varPrefix;
	varPrefix.vt = VT_NULL;

	m_editFirst.GetWindowText(strFirst);
	m_editMiddle.GetWindowText(strMiddle);
	m_editLast.GetWindowText(strLast);
	m_editSuffix.GetWindowText(strSuffix);
	NXDATALIST2Lib::IRowSettingsPtr pPrefixRow = m_pPrefixList->CurSel;
	if(pPrefixRow != NULL) {
		//Get the VT_I4 variant value of the selected prefix
		varPrefix = pPrefixRow->GetValue(0);
	}

	//Trim and ensure that one of either first or last has data
	strFirst.Trim();
	strLast.Trim();
	strMiddle.Trim();
	strSuffix.Trim();

	if(strFirst.IsEmpty() && strLast.IsEmpty()) {
		AfxMessageBox("You must fill in either a First or Last name.");
		return false;
	}

	//Generate our save statements based on the new-ness of the record
	if(m_nPersonID == -1) {
		//We must generate a new record
		CString strSqlBatch = BeginSqlBatch();
		CNxParamSqlArray args;
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewPersonID int;\r\n");
		AddStatementToSqlBatch(strSqlBatch, "SET @NewPersonID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM PersonT);\r\n");

		AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO PersonT (ID, First, Middle, Last, Suffix, PrefixID) values "
			"(@NewPersonID, {STRING}, {STRING}, {STRING}, {STRING}, {VT_I4});\r\n", strFirst, strMiddle, strLast, strSuffix, varPrefix);
		AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO PharmacyStaffT (PersonID, Type, PharmacyID) values "
			"(@NewPersonID, {INT}, {INT});\r\n", m_nType, m_nPharmacyID);

		//Execute it to data
		// (e.lally 2009-06-21) PLID 34679 - Leave as execute batch
		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);
	}
	else {
		//We must update the existing record
		//	No changes are made to the pharmacy staff table, this dialog only affects PersonT records
		ExecuteParamSql("UPDATE PersonT SET First = {STRING}, Middle = {STRING}, Last = {STRING}, Suffix = {STRING}, PrefixID = {VT_I4} "
			"WHERE PersonT.ID = {INT};", strFirst, strMiddle, strLast, strSuffix, varPrefix, m_nPersonID);
	}

	//Success!
	return true;
}
