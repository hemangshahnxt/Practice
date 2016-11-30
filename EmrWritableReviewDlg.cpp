// EmrWritableReviewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "EmrWritableReviewDlg.h"
#include "InternationalUtils.h"

// (a.walling 2008-06-26 13:23) - PLID 30531 - Dialog to review EMNs/Templates that are currently locked for editing

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


enum EWritableListColumns {
	wlcObjectID = 0,
	wlcPatientID,
	wlcIsEMN,
	wlcDescription,
	wlcPatientName,
	wlcDate,
	wlcHeldByUserName,
	wlcAcquiredDate,
	wlcPath,
};

/////////////////////////////////////////////////////////////////////////////
// CEmrWritableReviewDlg dialog


CEmrWritableReviewDlg::CEmrWritableReviewDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrWritableReviewDlg::IDD, pParent)
{
	EnableDragHandle(false);	// (j.armen 2012-05-30 11:46) - PLID 49854 - Drag handle doesn't look good here.  Disable it.
	m_bNeedRefresh = FALSE;
}

CEmrWritableReviewDlg::~CEmrWritableReviewDlg()
{

}


void CEmrWritableReviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmrWritableReviewDlg)
	DDX_Control(pDX, IDC_EMR_WRITABLE_REFRESH_BTN, m_nxibRefresh);
	DDX_Control(pDX, IDC_EMR_WRITABLE_LABEL, m_lblInfo);
	DDX_Control(pDX, IDC_BTN_CLOSE_EMRWRITABLE, m_nxibOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmrWritableReviewDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEmrWritableReviewDlg)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_EMR_WRITABLE_REFRESH_BTN, OnEmrWritableRefreshBtn)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BTN_CLOSE_EMRWRITABLE, OnBtnClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrWritableReviewDlg message handlers


BOOL CEmrWritableReviewDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		((CNxColor*)GetDlgItem(IDC_NXCOLORCTRL1))->SetColor(GetNxColor(GNC_ADMIN, 0));
		GetMainFrame()->RequestTableCheckerMessages(GetSafeHwnd());
		m_pList = BindNxDataList2Ctrl(IDC_EMR_WRITABLE_LIST, false);

		m_nxibOK.AutoSet(NXB_CLOSE);
		
		OnEmrWritableRefreshBtn();
	} NxCatchAll("Error in CEmrWritableReviewDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CEmrWritableReviewDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		switch(wParam) {
			case NetUtils::EMNAccessT:
			case NetUtils::EMNTemplateAccessT:
				if (IsWindowVisible()) {
					OnEmrWritableRefreshBtn();
				} else {
					m_bNeedRefresh = TRUE;
				}
				break;
			default:
				break;
		}
	} NxCatchAll("Error in CEmrWritableReviewDlg::OnTableChanged");

	return 0;
}

void CEmrWritableReviewDlg::OnEmrWritableRefreshBtn() 
{
	try {
		m_lblInfo.SetWindowText(FormatString("Last refreshed at %s", FormatDateTimeForInterface(COleDateTime::GetCurrentTime(), NULL, dtoTime)));

		// (z.manning 2011-05-20 12:36) - PLID 33114 - Filter out EMNs the user can't access because of charting permissions
		// (j.armen 2013-05-14 12:26) - PLID 56680 - Refactor EMN Access
		// (j.armen 2013-05-14 12:27) - PLID 56683 - Refactor EMN Template Access
		CString strWhere;
		CSqlFragment sqlChartFilter = GetEmrChartPermissionFilter(FALSE);
		if(!sqlChartFilter.IsEmpty()) {
			strWhere = "WHERE " + sqlChartFilter.Flatten() + " \r\n";
		}
		m_pList->PutFromClause(_bstr_t(FormatString(
			"( \r\n"
			"	SELECT M.ID AS ObjectID, M.PatientID AS PatientID, CONVERT(bit, 0) AS IsTemplate, \r\n"
			"		P.Last + ', ' + P.First + ' ' + P.Middle AS PersonName, \r\n"
			"		M.Description AS Description, M.Date AS Date, \r\n"
			"		A.Date AS DateAcquired, T.DeviceInfo AS Path, U.UserName as HeldByUserName \r\n"
			"	FROM EMRMasterT M\r\n"
			"	INNER JOIN PersonT P ON M.PatientID = P.ID \r\n"
			"	INNER JOIN EMNAccessT A ON M.ID = A.EmnID \r\n"
			"	INNER JOIN UserLoginTokensT T ON A.UserLoginTokenID = T.ID \r\n"
			"	INNER JOIN UsersT U ON T.UserID = U.PersonID \r\n"
			"	LEFT JOIN EmnTabChartsLinkT ON M.ID = EmnTabChartsLinkT.EmnID \r\n"
			"	%s"
			"	UNION ALL \r\n"
			"	SELECT M.ID AS ObjectID, NULL AS PatientID, CONVERT(bit, 1) AS IsTemplate, \r\n"
			"		'(Template)' AS PersonName, \r\n"
			"		M.Name AS Description, NULL AS Date, \r\n"
			"		A.Date AS DateAcquired, T.DeviceInfo AS Path, U.UserName as HeldByUserName \r\n"
			"	FROM EMRTemplateT M \r\n"
			"	INNER JOIN EMNTemplateAccessT A ON M.ID = A.EmnID \r\n"
			"	INNER JOIN UserLoginTokensT T ON A.UserLoginTokenID = T.ID \r\n"
			"	INNER JOIN UsersT U ON T.UserID = U.PersonID \r\n"
			") TotalQ \r\n"
			, strWhere)));
		m_pList->Requery();
		m_bNeedRefresh = FALSE;

	} NxCatchAll("Error refreshing writable list");	
}

void CEmrWritableReviewDlg::OnDestroy() 
{
	try {
		GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());
	} NxCatchAll("Error in CEmrWritableReviewDlg::OnDestroy");

	CNxDialog::OnDestroy();
}

BEGIN_EVENTSINK_MAP(CEmrWritableReviewDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEmrWritableReviewDlg)
	ON_EVENT(CEmrWritableReviewDlg, IDC_EMR_WRITABLE_LIST, 19 /* LeftClick */, OnLeftClickEmrWritableList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEmrWritableReviewDlg::OnLeftClickEmrWritableList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		if (nCol == wlcDescription) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			if (pRow) {
				BOOL bIsEMN = VarBool(pRow->GetValue(wlcIsEMN));
				long nObjectID = VarLong(pRow->GetValue(wlcObjectID));

				GetMainFrame()->PostMessage(NXM_EDIT_EMR_OR_TEMPLATE, nObjectID, bIsEMN ? 1 : 0);

				CEmrWritableReviewDlg::OnOK();
			}
		} else if (nCol == wlcPatientName) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			// (a.walling 2008-06-26 14:12) - PLID 30531 - Use the shared function in the mainframe
			long nPatID = VarLong(pRow->GetValue(wlcPatientID), -1);

			if (nPatID != -1) {
				GetMainFrame()->GotoPatient(nPatID);
				// we can stay up
				// CEmrWritableReviewDlg::OnOK();
			} else {
				// must be a template
				BOOL bIsEMN = VarBool(pRow->GetValue(wlcIsEMN));
				long nObjectID = VarLong(pRow->GetValue(wlcObjectID));

				GetMainFrame()->PostMessage(NXM_EDIT_EMR_OR_TEMPLATE, nObjectID, bIsEMN ? 1 : 0);

				CEmrWritableReviewDlg::OnOK();
			}
		}
	} NxCatchAll("Error in CEmrWritableReviewDlg::OnLeftClickEmrWritableList");
}

int CEmrWritableReviewDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
		
	return 0;
}

void CEmrWritableReviewDlg::OnOK() 
{
	ShowWindow(SW_HIDE);
}

void CEmrWritableReviewDlg::OnCancel() 
{
	ShowWindow(SW_HIDE);
}

void CEmrWritableReviewDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CNxDialog::OnShowWindow(bShow, nStatus);
	
	if (bShow) {
		if (m_bNeedRefresh) {
			OnEmrWritableRefreshBtn();
		}
	} else {
		m_bNeedRefresh = TRUE;
	}
}

void CEmrWritableReviewDlg::OnBtnClose() 
{
	OnOK();
}
