// CorrectMedicationQuantitiesDlg.cpp : implementation file
//
// (d.thompson 2009-03-12) - PLID 33482
#include "stdafx.h"
#include "Practice.h"
#include "CorrectMedicationQuantitiesDlg.h"


using namespace ADODB;
using namespace NXDATALIST2Lib;

// CCorrectMedicationQuantitiesDlg dialog

// (j.fouts 2013-03-19 12:06) - PLID 55740 - Added enum for columns
enum QuantityUnitListColumns
{
	qulcID = 0,
	qulcName,
};

IMPLEMENT_DYNAMIC(CCorrectMedicationQuantitiesDlg, CNxDialog)

CCorrectMedicationQuantitiesDlg::CCorrectMedicationQuantitiesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCorrectMedicationQuantitiesDlg::IDD, pParent)
{
	m_nDrugListID = -1;
}

CCorrectMedicationQuantitiesDlg::~CCorrectMedicationQuantitiesDlg()
{
}

void CCorrectMedicationQuantitiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEF_QTY, m_nxeditDefQty);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CCorrectMedicationQuantitiesDlg, CNxDialog)
END_MESSAGE_MAP()


// CCorrectMedicationQuantitiesDlg message handlers
BOOL CCorrectMedicationQuantitiesDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//Setup controls
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_pList = BindNxDataList2Ctrl(IDC_UNIT_LIST, true);
		m_nxeditDefQty.LimitText(50);
		SetDlgItemText(IDC_MED_HELPTEXT, "Practice requires that the Default Quantity and Units fields be filled out before writing a prescription for a patient.  The system has detected that at least one of these fields is currently blank.  It is highly recommended that you update this data for the current drug so that it will apply to all future prescriptions.");

		//1 time load of data
		_RecordsetPtr prs = CreateParamRecordset("SELECT EMRDataT.Data AS DrugDescription, DefaultQuantity, QuantityUnitID "
			"FROM DrugList INNER JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID WHERE DrugList.ID = {INT};", m_nDrugListID);

		if(prs->eof) {
			EndDialog(IDCANCEL);
			AfxThrowNxException("Failed to find drug ID %li to correct quantity data.", m_nDrugListID);
			return TRUE;
		}

		//Get the data
		CString strName = AdoFldString(prs, "DrugDescription");
		CString strQty = AdoFldString(prs, "DefaultQuantity");
		_variant_t varUnit = prs->Fields->Item["QuantityUnitID"]->Value;

		//Put the drug in the title bar
		SetWindowText("Update Drug Information for:  " + strName);

		//The rest in the fields
		SetDlgItemText(IDC_DEF_QTY, strQty);
		m_pList->SetSelByColumn(0, varUnit);

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

void CCorrectMedicationQuantitiesDlg::OnOK()
{
	try {
		//Get the data
		CString strQty;
		GetDlgItemText(IDC_DEF_QTY, strQty);
		strQty.TrimRight();
		if(strQty.GetLength() > 50) {
			//error
			AfxMessageBox("The default quantity may not exceed 50 characters in length.  Please shorten your data and try again.");
			return;
		}
		if(strQty.IsEmpty()) {
			//error
			AfxMessageBox("You may not enter a blank default quantity, please correct this and try to save again.");
			return;
		}

		IRowSettingsPtr pRow = m_pList->CurSel;
		if(pRow == NULL) {
			//This dialog requires data.  Cancel if you don't want to fill it out.
			AfxMessageBox("You must select a valid quantity unit to continue.");
			return;
		}
		_variant_t varUnit = pRow->GetValue(qulcID);
		if(varUnit.vt != VT_I4) {
			//This shouldn't be possible either, but warn them if so
			AfxMessageBox("You must select a valid quantity unit to continue.");
			return;
		}

		//all good, so let's save
		ExecuteParamSql("UPDATE DrugList SET DefaultQuantity = {STRING}, QuantityUnitID = {VT_I4} WHERE ID = {INT};", 
			strQty, varUnit, m_nDrugListID);

		//Setup data for output
		m_strOutputDefQty = strQty;
		m_strOutputQtyUnit = VarString(pRow->GetValue(qulcName));
		// (j.fouts 2013-03-19 12:06) - PLID 55740 - We also need Quantity Unit ID
		m_nOutputQtyUnitID = VarLong(varUnit, -1);

		CDialog::OnOK();

	} NxCatchAll("Error in OnOK");
}

void CCorrectMedicationQuantitiesDlg::OnCancel()
{
	try {
		//Warn the user this might be a bad idea.
		// (j.jones 2012-11-21 11:48) - PLID 53818 - Tweaked the warning to not state that this is ever ok.
		// When called from an EMR, cancelling this dialog will cancel adding the drug, so the message needs to be clearer.
		// Some places in code do permit cancelling, but we shouldn't let them think it's actually a good idea.
		if(AfxMessageBox("It is required to have both quantity and unit information filled out for your drugs.  Are you SURE you wish to "
			"cancel this update?", MB_YESNO) != IDYES)
		{
			return;
		}

		CDialog::OnCancel();

	} NxCatchAll("Error in OnCancel");
}
