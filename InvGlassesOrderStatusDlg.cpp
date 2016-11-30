// InvGlassesOrderStatusDlg.cpp : implementation file
//
// (s.dhole 2011-03-31 14:12) - PLID 43077 Create new Dialog to change Glasses order status
#include "stdafx.h"
#include "Practice.h"
#include "InvGlassesOrderStatusDlg.h"
#include "InventoryRc.h"
#include "InvVisionWebDlg.h"
#include "EditComboBox.h"
#include "InvVisionWebUtils.h"


// CInvGlassesOrderStatusDlg dialog
using namespace ADODB;
using namespace NXDATALIST2Lib;
IMPLEMENT_DYNAMIC(CInvGlassesOrderStatusDlg, CNxDialog)

enum VisionWebOrderStatusColumns {
		vwosOrderStatusID = 0,
		vwosOrderStatusName,
	};

CInvGlassesOrderStatusDlg::CInvGlassesOrderStatusDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvGlassesOrderStatusDlg::IDD, pParent)
{

}

CInvGlassesOrderStatusDlg::~CInvGlassesOrderStatusDlg()
{
}

void CInvGlassesOrderStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ORDER_MESSAGES , m_nxMsg);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EDIT_STANDARD_NOTES, m_btnEditStandardNotes);
}


BEGIN_MESSAGE_MAP(CInvGlassesOrderStatusDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CInvGlassesOrderStatusDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CInvGlassesOrderStatusDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_EDIT_STANDARD_NOTES, &CInvGlassesOrderStatusDlg::OnEditStandardNotes)
END_MESSAGE_MAP()


// CInvGlassesOrderStatusDlg message handlers
BEGIN_EVENTSINK_MAP(CInvGlassesOrderStatusDlg, CNxDialog)
	ON_EVENT(CInvGlassesOrderStatusDlg, IDC_ORDER_STATUS_LIST, 1, CInvGlassesOrderStatusDlg::SelChangingOrderStatusList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvGlassesOrderStatusDlg, IDC_STANDARD_HISTORY_NOTES, 16, CInvGlassesOrderStatusDlg::OnSelChosenStandardHistoryNotes, VTS_DISPATCH)
END_EVENTSINK_MAP()

BOOL CInvGlassesOrderStatusDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	try {
		m_btnOk.AutoSet(NXB_OK)  ;
		m_btnCancel.AutoSet(NXB_CANCEL );
		SetDlgItemText( IDC_ORDER_MESSAGES, "");
		SetWindowText("Manage Order Status - " + m_strCaption);
		m_nxMsg.SetLimitText(255);
		m_VisionWebOrderStatusCombo =  BindNxDataList2Ctrl(IDC_ORDER_STATUS_LIST ,GetRemoteData(), true); //(r.wilson 4/11/2012) PLID 43741 - Auto requery set to TRUE

		//TES 5/25/2011 - PLID 43842 - Load our list of standard notes
		m_pStandardNotes = BindNxDataList2Ctrl(IDC_STANDARD_HISTORY_NOTES);

		//(r.wilson 4/11/2012) PLID 43741 - If this is a Vision Web order then remove the following rows
		// (j.dinatale 2013-02-26 12:10) - PLID 52849 - no longer need to remove order status options

	}NxCatchAll("Error in CInvGlassesOrderStatusDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CInvGlassesOrderStatusDlg::SelChangingOrderStatusList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("Error in CInvGlassesOrderStatusDlg::SelChangingOrderStatusList()")
}

void CInvGlassesOrderStatusDlg::OnBnClickedOk()
{
	try {
		IRowSettingsPtr pRow= m_VisionWebOrderStatusCombo->GetCurSel();
		if (pRow == NULL){ 
			return;
		} else {
			long nID = VarLong(pRow->GetValue(vwosOrderStatusID), -1);
			CString strStatus;
			GetDlgItemText(IDC_ORDER_MESSAGES, strStatus);
			if (nID <= 0){
				MsgBox("Please select an order status.");
				return;
			} else {
				m_nOrderStatus = nID;
				m_strOrderStatusMsg = strStatus;
				CNxDialog::OnOK();
			}
		}
	}NxCatchAll("CInvGlassesOrderStatusDlg::OnBnClickedOk");
}

void CInvGlassesOrderStatusDlg::OnBnClickedCancel()
{
	try{ 
		CNxDialog::OnCancel();	
	}NxCatchAll("Error in CInvGlassesOrderStatusDlg::OnBnClickedCancel");
}

void CInvGlassesOrderStatusDlg::OnSelChosenStandardHistoryNotes(LPDISPATCH lpRow)
{
	try {
		//TES 5/25/2011 - PLID 43842 - If they selected a note, replace the current note with what they selected
		IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			SetDlgItemText(IDC_ORDER_MESSAGES, VarString(pRow->GetValue(0)));
		}
	}NxCatchAll(__FUNCTION__);
}

void CInvGlassesOrderStatusDlg::OnEditStandardNotes()
{
	try {
		//TES 5/25/2011 - PLID 43842 - Call CEditComboBox
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		/*GLASSES_ORDER_HISTORY_NOTES*/
		CEditComboBox(this, 76, m_pStandardNotes, "Edit Combo Box").DoModal();
	}NxCatchAll(__FUNCTION__);
}
