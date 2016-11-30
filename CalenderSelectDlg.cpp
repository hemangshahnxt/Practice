// CalenderSelectDlg.cpp : implementation file
//

// (d.singleton 2012-05-17 16:37) - PLID 50478 added new dialog

#include "stdafx.h"
#include "Practice.h"
#include "CalenderSelectDlg.h"

using namespace NXDATALIST2Lib;

// CCalenderSelectDlg dialog

enum eCalendarColumns
{
	ccID = 0,
	ccName,
	ccDate,
};


IMPLEMENT_DYNAMIC(CCalenderSelectDlg, CNxDialog)

CCalenderSelectDlg::CCalenderSelectDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCalenderSelectDlg::IDD, pParent)
{
	m_strWhere = "";
	m_nCalendarID = -1;
}

CCalenderSelectDlg::~CCalenderSelectDlg()
{
}

void CCalenderSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BEGIN_MESSAGE_MAP(CCalenderSelectDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CCalenderSelectDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// CCalenderSelectDlg message handlers
BOOL CCalenderSelectDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	
	try {
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_dlCalendarList = BindNxDataList2Ctrl(this, IDC_PRE_OP_CALENDAR_LIST, GetRemoteData(), false);
		m_dlCalendarList->PutWhereClause(_bstr_t(m_strWhere));

		m_dlCalendarList->Requery();

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}
BEGIN_EVENTSINK_MAP(CCalenderSelectDlg, CNxDialog)
	ON_EVENT(CCalenderSelectDlg, IDC_PRE_OP_CALENDAR_LIST, 3, CCalenderSelectDlg::DblClickCellPreOpCalendarList, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CCalenderSelectDlg::DblClickCellPreOpCalendarList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			m_nCalendarID = VarLong(pRow->GetValue(ccID));
			OnOK();
		}	
	}NxCatchAll(__FUNCTION__);
}

void CCalenderSelectDlg::OnBnClickedOk()
{
	try {
		IRowSettingsPtr pRow = m_dlCalendarList->GetCurSel();
		if(pRow) {
			m_nCalendarID = VarLong(pRow->GetValue(ccID));
			OnOK();
		}
		else {
			AfxMessageBox("Please select a calendar to edit before clicking OK.");
		}
	}NxCatchAll(__FUNCTION__);
}
