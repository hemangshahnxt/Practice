// InvMultiSerialPickerDlg.cpp : implementation file
//
// (c.haag 2008-06-18 10:16) - PLID 28339 - Initial implementation
//

#include "stdafx.h"
#include "InvMultiSerialPickerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef enum {
	eclID = 0,
	eclProduct,
	eclLocation,
	eclExpDate,
	eclPatientName
} EColumns;

using namespace ADODB;
using namespace NXDATALIST2Lib;

/////////////////////////////////////////////////////////////////////////////
// CInvMultiSerialPickerDlg dialog


CInvMultiSerialPickerDlg::CInvMultiSerialPickerDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvMultiSerialPickerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInvMultiSerialPickerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nResultID = -1;
}


void CInvMultiSerialPickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvMultiSerialPickerDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STATIC_SERIAL, m_nxstaticSerial);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInvMultiSerialPickerDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInvMultiSerialPickerDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvMultiSerialPickerDlg message handlers

BOOL CInvMultiSerialPickerDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// Set up icons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// Set up the title text
		m_nxstaticSerial.SetWindowText( FormatString("The serial code '%s' applies to multiple items.", m_strSerialNum) );

		// Set up the datalist
		m_dlList = BindNxDataList2Ctrl(IDC_LIST_MULTISERIALPICKER, false);

		// Populate the datalist
		FieldsPtr f = m_prs->Fields;
		m_prs->MoveFirst();
		while (!m_prs->eof) {

			// See if we should add the record
			BOOL bAdd = FALSE;
			const long nID = AdoFldLong(f, "ID");
			for (int i=0; i < m_anIDs.GetSize() && !bAdd; i++) {
				if (m_anIDs[i] == nID) {
					bAdd = TRUE;
				}
			}

			// Add the record if necessary
			if (bAdd) {
				IRowSettingsPtr pRow = m_dlList->GetNewRow();
				pRow->Value[eclID] = nID;
				pRow->Value[eclProduct] = f->Item["ProductName"]->Value;
				pRow->Value[eclLocation] = f->Item["LocName"]->Value;
				if (AdoFldBool(f, "HasExpDate", VARIANT_FALSE)) {
					pRow->Value[eclExpDate] = f->Item["ExpDate"]->Value;
				}
				pRow->Value[eclPatientName] = f->Item["PatientName"]->Value;
				m_dlList->AddRowSorted(pRow, NULL);				
			}

			m_prs->MoveNext();
		}
	}
	NxCatchAll("Error in CInvMultiSerialPickerDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CInvMultiSerialPickerDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvMultiSerialPickerDlg)
	ON_EVENT(CInvMultiSerialPickerDlg, IDC_LIST_MULTISERIALPICKER, 3 /* DblClickCell */, OnDblClickCellListMultiserialpicker, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInvMultiSerialPickerDlg::OnDblClickCellListMultiserialpicker(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		// If the user double-clicked on a row, that amounts to an auto-selection
		IRowSettingsPtr pRow(lpRow);
		if (NULL != pRow) {
			m_dlList->CurSel = pRow;
			OnOK();
		}
	}
	NxCatchAll("Error in CInvMultiSerialPickerDlg::OnDblClickCellListMultiserialpicker");	
}

void CInvMultiSerialPickerDlg::OnOK() 
{
	try {
		// Take the selection and dismiss the dialog
		IRowSettingsPtr pRow = m_dlList->CurSel;
		if (NULL != pRow) {
			m_nResultID = VarLong(pRow->Value[eclID]);
			CNxDialog::OnOK();
		} else {
			AfxMessageBox("Please select an item from the list before continuing.", MB_ICONERROR);
		}	
	}
	NxCatchAll("Error in CInvMultiSerialPickerDlg::OnOK");
}
