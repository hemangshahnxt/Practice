// InvInternalReturnChecklistDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "InvInternalReturnChecklistDlg.h"


// CInvInternalReturnChecklistDlg dialog

IMPLEMENT_DYNAMIC(CInvInternalReturnChecklistDlg, CNxDialog)

CInvInternalReturnChecklistDlg::CInvInternalReturnChecklistDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvInternalReturnChecklistDlg::IDD, pParent)
{
	// (j.fouts 2012-05-15 15:33) - PLID 50297 - Intialize this to a invalid ID
	m_nReqID = -1;
}

CInvInternalReturnChecklistDlg::~CInvInternalReturnChecklistDlg()
{
}

void CInvInternalReturnChecklistDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	// (j.fouts 2012-06-06 16:43) - PLID 50297 - Bind the Ok and Cancel buttons
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CInvInternalReturnChecklistDlg, CNxDialog)
	ON_BN_CLICKED(IDCANCEL, &CInvInternalReturnChecklistDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CInvInternalReturnChecklistDlg::OnBnClickedOk)
END_MESSAGE_MAP()

int CInvInternalReturnChecklistDlg::OnInitDialog()
{
	// (j.fouts 2012-05-15 15:33) - PLID 50297 - Load the data about the selcted item
	CNxDialog::OnInitDialog();

	try
	{	
		//Add Icons to buttons
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);


		CString strName;
		CString strUnitDesc;
		CString strBarcode;
		CString strNotes;

		ADODB::_RecordsetPtr prsItem = CreateParamRecordset(
			"SELECT Name, UnitDesc, Barcode, Notes "
			"FROM ProductRequestsT "
			"INNER JOIN ProductT ON ProductRequestsT.ProductID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"WHERE ProductRequestsT.ID = {INT}",
			m_nReqID);

		if(!prsItem->eof)
		{
			strName = AdoFldString(prsItem, "Name", "");
			strUnitDesc = AdoFldString(prsItem, "UnitDesc", "");
			strBarcode = AdoFldString(prsItem, "Barcode", "");
			strNotes = AdoFldString(prsItem, "Notes", "");

			SetDlgItemText(IDC_NAME, strName);
			SetDlgItemText(IDC_DESCRIPTION, strUnitDesc);
			SetDlgItemText(IDC_BARCODE, strBarcode);
			SetDlgItemText(IDC_NOTES, strNotes);
		}


	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

// CInvInternalReturnChecklist message handlers

void CInvInternalReturnChecklistDlg::OnBnClickedCancel()
{
	OnCancel();
}

void CInvInternalReturnChecklistDlg::OnBnClickedOk()
{
	OnOK();
}
