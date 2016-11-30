// GlassesOrderServiceSelectDlg.cpp : implementation file
//

// (j.dinatale 2013-04-03 15:24) - PLID 56075 - Created

#include "stdafx.h"
#include "Practice.h"
#include "GlassesOrderServiceSelectDlg.h"
#include "InventoryRc.h"


// CGlassesOrderServiceSelectDlg dialog

IMPLEMENT_DYNAMIC(CGlassesOrderServiceSelectDlg, CNxDialog)

CGlassesOrderServiceSelectDlg::CGlassesOrderServiceSelectDlg(OrderServiceListType::ServiceListType slt, long nID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CGlassesOrderServiceSelectDlg::IDD, pParent), m_sltListType(slt), m_nID(nID)
{

}

CGlassesOrderServiceSelectDlg::~CGlassesOrderServiceSelectDlg()
{
}

void CGlassesOrderServiceSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_btnOK);
}


BEGIN_MESSAGE_MAP(CGlassesOrderServiceSelectDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CGlassesOrderServiceSelectDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CGlassesOrderServiceSelectDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CGlassesOrderServiceSelectDlg message handlers
BOOL CGlassesOrderServiceSelectDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{
		m_btnOK.AutoSet(NXB_OK);

		CMenu *pMenu = GetSystemMenu(FALSE); 
		if(pMenu){
			pMenu->DeleteMenu(SC_CLOSE, MF_BYCOMMAND);
		}

		CString strTitle;
		strTitle.Format("Select %s Code(s)", OrderServiceListType::GetDisplayName(m_sltListType));
		this->SetWindowText(strTitle);

		CString strLabelText;
		strLabelText.Format("Select services for the selected %s:", OrderServiceListType::GetDisplayName(m_sltListType));
		GetDlgItem(IDC_GLASSES_SERVICE_SELECT_STATIC)->SetWindowText(strLabelText);

		m_pServiceList = BindNxDataList2Ctrl(IDC_GLASSES_ORDER_SERVICE_SELECT_LIST, false);
		m_pServiceList->FromClause = _bstr_t(GenerateListSql());
		m_pServiceList->Requery();
	}NxCatchAll(__FUNCTION__);
	
	return TRUE;	// return TRUE unless you set the focus to a control
					// EXCEPTION: OCX Property Pages should return FALSE
}

CString CGlassesOrderServiceSelectDlg::GetTableName(OrderServiceListType::ServiceListType slt)
{
	return OrderServiceListType::GetCptTableName(slt);
}

CString CGlassesOrderServiceSelectDlg::GetColumnName(OrderServiceListType::ServiceListType slt)
{
	return OrderServiceListType::GetCptColumnName(slt);
}

CString CGlassesOrderServiceSelectDlg::GenerateListSql()
{
	CSqlFragment Sql;
	Sql.Create(
		"("
		"	SELECT ServiceT.Name AS Name, CptCodeT.Code AS Code, CptID AS ID "
		"	FROM {CONST_STR} "
		"	LEFT JOIN ServiceT ON CptID = ServiceT.ID "
		"	LEFT JOIN CptCodeT ON ServiceT.ID = CptCodeT.ID "
		"	WHERE {CONST_STR} = {CONST_INT} "
		") SubQ", 
		GetTableName(m_sltListType), GetColumnName(m_sltListType), m_nID
	);

	return Sql.Flatten();
}

GlassesOrderServices CGlassesOrderServiceSelectDlg::GetSelectedServices()
{
	return m_setSelectedServiceIDs;
}

void CGlassesOrderServiceSelectDlg::OnBnClickedOk()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pServiceList->GetFirstRow();
		while(pRow){
			BOOL bSelected = VarBool(pRow->GetValue(GOServiceList::Selected));
			if(bSelected){
				m_setSelectedServiceIDs.insert(VarLong(pRow->GetValue(GOServiceList::ID)));
			}

			pRow = pRow->GetNextRow();
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CGlassesOrderServiceSelectDlg::OnBnClickedCancel()
{
	// eat the message. OM NOM NOM NOM!
}
