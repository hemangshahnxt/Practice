// ApptTypeCodeLinkDlg.cpp : implementation file
//

// (j.gruber 2010-07-21 10:46) - PLID 30481 - created
#include "stdafx.h"
#include "Practice.h"
#include "ApptTypeCodeLinkDlg.h"


enum InvListColumn {
	ilcID = 0,
	ilcDesc,
	ilcType,
};

enum SelectListColumn {
	slcID = 0,
	slcDesc,
	slcType,
};

enum ServiceCodeListColumn {
	sclcID = 0,
	sclcCode,
	sclcName,
	sclcDesc,
	sclcType,
};



// CApptTypeCodeLinkDlg dialog

IMPLEMENT_DYNAMIC(CApptTypeCodeLinkDlg, CNxDialog)

CApptTypeCodeLinkDlg::CApptTypeCodeLinkDlg(long nApptTypeID, CString strApptType, CWnd* pParent /*=NULL*/)
	: CNxDialog(CApptTypeCodeLinkDlg::IDD, pParent)
{

	m_nApptTypeID = nApptTypeID;
	m_strApptType = strApptType;

}

CApptTypeCodeLinkDlg::~CApptTypeCodeLinkDlg()
{
}

void CApptTypeCodeLinkDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RD_SERVICE_CODES, m_rdService);
	DDX_Control(pDX, IDC_RD_INV_ITEMS, m_rdInventory);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CATC_DESC, m_stDesc);	
}


BEGIN_MESSAGE_MAP(CApptTypeCodeLinkDlg, CNxDialog)
	ON_BN_CLICKED(IDC_RD_SERVICE_CODES, &CApptTypeCodeLinkDlg::OnBnClickedRdServiceCodes)
	ON_BN_CLICKED(IDC_RD_INV_ITEMS, &CApptTypeCodeLinkDlg::OnBnClickedRdInvItems)
	ON_BN_CLICKED(IDOK, &CApptTypeCodeLinkDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CApptTypeCodeLinkDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CApptTypeCodeLinkDlg message handlers
BOOL CApptTypeCodeLinkDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
	
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pServiceList = BindNxDataList2Ctrl(IDC_CATC_SERVICE_CODE_LIST, false);
		m_pInvList = BindNxDataList2Ctrl(IDC_CATC_INV_LIST, false);
		m_pSelectList = BindNxDataList2Ctrl(IDC_CATC_SELECTED_LIST, false);

		//default to service codes
		m_rdService.SetCheck(1);
		m_rdInventory.SetCheck(0);

		OnBnClickedRdServiceCodes();

		m_pSelectList->FromClause = " (SELECT ServiceID, AptTypeID, "
			" CASE WHEN CPTCodeT.ID IS NOT NULL THEN Code + ' - ' + ServiceT.Name else ServiceT.Name END as Description, SErviceT.Name,  "
			" CASE WHEN CPTCodeT.ID IS NULL THEN 'Inventory Item'else 'Service Code' END as Type "
			" FROM ApptTypeServiceLinkT  "
			" INNER JOIN ServiceT ON ApptTypeServiceLinkT.SErviceID = SErviceT.ID "
			" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			" WHERE ServiceT.ID IN (SELECT ID FROM CPTCodeT UNION SELECT ID FROM ProductT)) Q ";
		
		CString strWhere;
		strWhere.Format(" Q.AptTypeID = %li ", m_nApptTypeID);
		m_pSelectList->WhereClause  = _bstr_t(strWhere);


		m_pServiceList->FromClause = " (SELECT ServiceT.ID, Active, Code, Name, 'Service Code' as Type FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID) Q ";
		m_pServiceList->WhereClause = " Active = 1 ";

		m_pInvList->FromClause = " (SELECT ServiceT.ID, '' as Code, Active, ServiceT.Name, 'Inventory Item' as Type FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID) Q ";
		m_pInvList->WhereClause = " Active = 1 ";		

		m_pSelectList->Requery();
		m_pServiceList->Requery();
		m_pInvList->Requery();

		CString strDesc;
		strDesc.Format("Select the Service Codes and Inventory Items in the list below to be associated with a %s appointment type", m_strApptType);
		m_stDesc.SetWindowText(strDesc);

		

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CApptTypeCodeLinkDlg::SetAvailList()
{
	CString strFrom, strWhere;
	if (m_rdService.GetCheck()) {
		
		GetDlgItem(IDC_CATC_SERVICE_CODE_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CATC_INV_LIST)->ShowWindow(SW_HIDE);

	}
	else {
		GetDlgItem(IDC_CATC_SERVICE_CODE_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CATC_INV_LIST)->ShowWindow(SW_SHOW);
	}

}

void CApptTypeCodeLinkDlg::OnBnClickedRdServiceCodes()
{
	try {
		SetAvailList();	
	}NxCatchAll(__FUNCTION__);
}

void CApptTypeCodeLinkDlg::OnBnClickedRdInvItems()
{
	try {
		SetAvailList();
	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CApptTypeCodeLinkDlg, CNxDialog)
	ON_EVENT(CApptTypeCodeLinkDlg, IDC_CATC_SERVICE_CODE_LIST, 16, CApptTypeCodeLinkDlg::SelChosenCatcCodeList, VTS_DISPATCH)
	ON_EVENT(CApptTypeCodeLinkDlg, IDC_CATC_INV_LIST, 16, CApptTypeCodeLinkDlg::SelChosenCatcInvList, VTS_DISPATCH)
	ON_EVENT(CApptTypeCodeLinkDlg, IDC_CATC_SELECTED_LIST, 7, CApptTypeCodeLinkDlg::RButtonUpCatcSelectedList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CApptTypeCodeLinkDlg::SelChosenCatcCodeList(LPDISPATCH lpRow)
{
	try {

		//first check to see if it is already in the selected list
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			long nID = VarLong(pRow->GetValue(sclcID));

			//it is already in the select list?
			NXDATALIST2Lib::IRowSettingsPtr pRowFind = m_pSelectList->FindByColumn(sclcID, nID, NULL, false);
			if (pRowFind) {
				//don't add it
				return;
			}

			//take the row if its not there
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pSelectList->GetNewRow();
			pNewRow->PutValue(slcID, pRow->GetValue(sclcID));
			pNewRow->PutValue(slcDesc, pRow->GetValue(sclcDesc));
			pNewRow->PutValue(slcType, pRow->GetValue(sclcType));
			m_pSelectList->AddRowAtEnd(pNewRow, NULL);
		}

	}NxCatchAll(__FUNCTION__);
}


void CApptTypeCodeLinkDlg::SelChosenCatcInvList(LPDISPATCH lpRow)
{
	try {

		//first check to see if it is already in the selected list
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			long nID = VarLong(pRow->GetValue(ilcID));

			//it is already in the select list?
			NXDATALIST2Lib::IRowSettingsPtr pRowFind = m_pSelectList->FindByColumn(ilcID, nID, NULL, false);
			if (pRowFind) {
				//don't add it
				return;
			}

			//take the row if its not there
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pSelectList->GetNewRow();
			pNewRow->PutValue(slcID, pRow->GetValue(ilcID));
			pNewRow->PutValue(slcDesc, pRow->GetValue(ilcDesc));
			pNewRow->PutValue(slcType, pRow->GetValue(ilcType));
			m_pSelectList->AddRowAtEnd(pNewRow, NULL);
		}

	}NxCatchAll(__FUNCTION__);
}

void CApptTypeCodeLinkDlg::RButtonUpCatcSelectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);	
				
		if(pRow) {

			m_pSelectList->CurSel = pRow;
			
			enum EMenuOptions
			{
				moRemove = 1,				
			};
			CMenu mnu;
			mnu.CreatePopupMenu();

			mnu.AppendMenu(MF_ENABLED, moRemove, "Remove");		
		
			CPoint ptClicked(x, y);
			GetDlgItem(IDC_CATC_SELECTED_LIST)->ClientToScreen(&ptClicked);
			int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);

			switch(nResult) {
			case moRemove:
					//just remove the row
					m_pSelectList->RemoveRow(pRow);				
				break;

				default:
					ASSERT(nResult == 0);
				break;
			}
		}

	}NxCatchAll(__FUNCTION__);
}


void CApptTypeCodeLinkDlg::Save()
{

	//first, clear the codes for this type

	CString strSqlBatch = BeginSqlBatch();
	CNxParamSqlArray ary;

	AddParamStatementToSqlBatch(strSqlBatch, ary, "DELETE FROM ApptTypeServiceLinkT WHERE AptTypeID = {INT}", m_nApptTypeID);

	//now loop through the list and add the codes
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelectList->GetFirstRow();

	while (pRow) {

		AddParamStatementToSqlBatch(strSqlBatch, ary, "INSERT INTO ApptTypeServiceLinkT (AptTypeID, ServiceID) VALUES ({INT}, {INT})", 
			m_nApptTypeID, VarLong(pRow->GetValue(slcID)));

		pRow = pRow->GetNextRow();
	}

	ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, ary);
	
}


void CApptTypeCodeLinkDlg::OnBnClickedOk()
{
	try {

		Save();

		OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CApptTypeCodeLinkDlg::OnBnClickedCancel()
{
	try {
		OnCancel();
	}NxCatchAll(__FUNCTION__);
}
