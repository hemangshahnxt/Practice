// InvSelectMultipleCPTDlg.cpp : implementation file
//
// (s.dhole 2012-03-06 09:12) - PLID 48638 new Dialog 
#include "stdafx.h"
#include "Practice.h"
#include "InvSelectMultipleCPTDlg.h"
#include "InventoryRc.h"

// CInvSelectMultipleCPTDlg dialog
enum MultiCPTList
{
	lstCPTID =0,
	lstStatus =1,
	lstCode =2,
	lstDescription =3,
};
using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CInvSelectMultipleCPTDlg, CNxDialog)

CInvSelectMultipleCPTDlg::CInvSelectMultipleCPTDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvSelectMultipleCPTDlg::IDD, pParent)
{

}

CInvSelectMultipleCPTDlg::~CInvSelectMultipleCPTDlg()
{
}

void CInvSelectMultipleCPTDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CInvSelectMultipleCPTDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CInvSelectMultipleCPTDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CInvSelectMultipleCPTDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

// set dialog caption
void CInvSelectMultipleCPTDlg::SetDlgCaption()
{
	try {
		CString strMsg ="Link Service Code To " + m_strType ;
		SetWindowText( strMsg );
		strMsg = m_strType + ": " ;  
		strMsg += m_strItem;
		SetDlgItemText(IDC_CPT_MSG_LABEL, strMsg );
	} NxCatchAll(__FUNCTION__);

}

BOOL CInvSelectMultipleCPTDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	SetDlgCaption();
	try {
		m_CptList = BindNxDataList2Ctrl(IDC_CATALOG_CPT_LIST);
		for(int j = 0; j < m_arPreSelectedID.GetSize(); j++) {
			IRowSettingsPtr pRow = m_CptList->FindByColumn(lstCPTID, (long)m_arPreSelectedID[j], 0, FALSE);
			if(pRow) {
				pRow->PutValue(lstStatus, g_cvarTrue);
			}
		}
	
	} NxCatchAll(__FUNCTION__);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CInvSelectMultipleCPTDlg::OnBnClickedOk()
{
try {
		if (ValidateAndStore()) {
			CNxDialog::OnOK();
		}
	} NxCatchAll(__FUNCTION__);
}

void CInvSelectMultipleCPTDlg::OnBnClickedCancel()
{
try {
	CNxDialog::OnCancel();
	} NxCatchAll(__FUNCTION__);
	
}
BOOL CInvSelectMultipleCPTDlg::ValidateAndStore()
{
	DWORD nSelections = 0;
	CString  strSelectedIDs ="";
	CString  strSelectedCodes="";
	IRowSettingsPtr pRow = m_CptList->GetFirstRow();
	while(pRow)
	{
		if(pRow->GetValue(lstStatus).boolVal != FALSE) {
			if (strSelectedIDs.IsEmpty())
			{ 
				strSelectedIDs = AsString(pRow->GetValue(lstCPTID));
				strSelectedCodes= VarString(pRow->GetValue(lstCode ),"" );
			}
			else
			{
				strSelectedIDs += ", " + AsString(pRow->GetValue(lstCPTID));
				strSelectedCodes +=  "; " +  VarString(pRow->GetValue(lstCode ),"" );
			}
			nSelections++;
		}
		else {
			
			// nothing
		}
		pRow = pRow->GetNextRow();
	}

	// Allow only 10 code selection		
	//PLID 48637 Should select max 10 items
	if (nSelections > 10){
		MsgBox("You cannot select more than %d item(s) from the list", 10);
		return FALSE;
	}
	else{
		m_strSelectedIDs=strSelectedIDs;
		m_strSelectedCodes=strSelectedCodes;
	}
	return TRUE;
}
