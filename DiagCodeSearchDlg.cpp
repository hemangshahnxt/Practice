#include "stdafx.h"
#include "AdministratorRc.h"
#include "DiagCodeSearchDlg.h"
#include "NxAPI.h"
#include "NxThread.h"
#include "AsyncDiagSearchQuery.h"

using namespace NXDATALIST2Lib;
using namespace NexTech_Accessor;

enum CodesList {
	dlcID,
	dlcCode,
	dlcDescription,
};

inline const std::vector<NXDATALIST2Lib::IRowSettingsPtr> GetAllRows(const NXDATALIST2Lib::_DNxDataListPtr pdl)
{
	std::vector<NXDATALIST2Lib::IRowSettingsPtr> ary;
	for(NXDATALIST2Lib::IRowSettingsPtr pRow = pdl->GetFirstRow(); pRow; pRow = pRow->GetNextRow())
		ary.push_back(pRow);
	return ary;
}

inline const std::vector<NXDATALIST2Lib::IRowSettingsPtr> GetSelectedRows(const NXDATALIST2Lib::_DNxDataListPtr pdl)
{
	std::vector<NXDATALIST2Lib::IRowSettingsPtr> ary;
	for(NXDATALIST2Lib::IRowSettingsPtr pRow = pdl->GetFirstSelRow(); pRow; pRow = pRow->GetNextSelRow())
		ary.push_back(pRow);
	return ary;
}

boost::optional<DiagCode9> CDiagCodeSearchDlg::DoManagedICD9Search(CWnd* pParent)
{
	CDiagCodeSearchDlg dlg(pParent);
	boost::optional<DiagCode9> pCode;
	if(IDOK == dlg.DoModal())
	{
		pCode = DiagCode9();
		pCode->strCode = _bstr_t(dlg.m_pdlUnselected->CurSel->Value[dlcCode]);
		pCode->strDescription = _bstr_t(dlg.m_pdlUnselected->CurSel->Value[dlcDescription]);
		pCode->nDiagCodesID = VarLong(dlg.m_pdlUnselected->CurSel->Value[dlcID]);
		pCode->nBackColor = VarULong(dlg.m_pdlUnselected->CurSel->BackColor);
	}
	return pCode;
}

CDiagCodeSearchDlg::CDiagCodeSearchDlg(CWnd* pParent)
	: CNxDialog(IDD_INSERT_SINGLE_ICD9, pParent, "DiagCodeSearchDlg")
{
	// (j.armen 2014-03-20 09:58) - PLID 60943 - Set our min dlg size
	__super::SetMinSize(300, 400);
}

CDiagCodeSearchDlg::~CDiagCodeSearchDlg()
{
}

// (j.armen 2014-03-20 09:58) - PLID 60943 - Handle setting Icon, binding controls
BOOL CDiagCodeSearchDlg::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();

	try
	{
		HICON hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
		SetIcon(hIcon, TRUE);
		SetIcon(hIcon, FALSE);

		m_nxcolor.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_pdlUnselected = BindNxDataList2Ctrl(IDC_INSERT_SINGLE_ICD9_DL, false);

		IRowSettingsPtr pRow = m_pdlUnselected->GetNewRow();
		pRow->Value[dlcID] = -1;
		pRow->Value[dlcDescription] = "< Enter Search Above >";
		pRow->BackColor = RGB(255,255,255);	//white
		m_pdlUnselected->AddRowAtEnd(pRow, NULL);

		SetControlPositions();
		SetControlState();

	}NxCatchAll(__FUNCTION__);

	return ret;
}

void CDiagCodeSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INSERT_SINGLE_ICD9_NXC, m_nxcolor);
	DDX_Control(pDX, IDC_INSERT_SINGLE_ICD9_EDIT, m_editSearch);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BEGIN_MESSAGE_MAP(CDiagCodeSearchDlg, CNxDialog)
	ON_EN_CHANGE(IDC_INSERT_SINGLE_ICD9_EDIT, &CDiagCodeSearchDlg::OnEnSearchModified)
	ON_REGISTERED_MESSAGE(AsyncDiagSearchQuery::NotifyMessage, &CDiagCodeSearchDlg::OnMessageSearchResult)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

// (j.armen 2014-03-20 09:58) - PLID 60943 - Fired when search text is modified
void CDiagCodeSearchDlg::OnEnSearchModified()
{
	// Reset timer
	m_TimerInstance.reset();

	// Disable selection controls.  These will be re-enabled when the new data is loaded
	m_pdlUnselected->Enabled = VARIANT_FALSE;

	// Set timer to trigger in X ms.  This will start the async search
	m_TimerInstance = WindowlessTimer::OnElapsed(500, boost::bind(&CDiagCodeSearchDlg::TriggerAsyncSearch, this), false);
}

// (j.armen 2014-03-20 09:58) - PLID 60943 - Called by timer to trigger the async search
void CDiagCodeSearchDlg::TriggerAsyncSearch()
{
	try
	{
		// Reset timer
		m_TimerInstance.reset();

		// Grab search text
		CString strSearchText = m_editSearch.GetText();

		// Reset the pending data with our new search thread
		m_pData.reset();
		m_pPendingData.reset(new AsyncDiagSearchQuery);

		// Run the pending data
		NxThread(boost::bind(&AsyncDiagSearchQuery::Run, m_pPendingData, GetSafeHwnd(), strSearchText, ManagedICD9Only))->RunDetached();

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-03-20 09:58) - PLID 60943 - Receives notification that an async query has completed.
LRESULT CDiagCodeSearchDlg::OnMessageSearchResult(WPARAM wParam, LPARAM lParam)
{
	// Make sure the result that posted is the current executing search
	if (lParam != (LPARAM)m_pPendingData.get())
		return 0; // Must be an old search, ignore

	// Move the pending data to live data
	swap(m_pData, m_pPendingData);
	m_pPendingData.reset();
	
	// Load our records
	m_pdlUnselected->Clear();
	for each(std::pair<shared_ptr<DiagCode9>, shared_ptr<DiagCode10>> code in m_pData->aryCodes)
	{
		if(code.first)
		{
			IRowSettingsPtr pRow = m_pdlUnselected->GetNewRow();
			pRow->Value[dlcID] = code.first->nDiagCodesID;
			pRow->Value[dlcCode] = code.first->strCode;
			pRow->Value[dlcDescription] = code.first->strDescription;
			pRow->BackColor = code.first->nBackColor;
			m_pdlUnselected->AddRowAtEnd(pRow, NULL);
		}
	}
	m_pdlUnselected->Sort();

	// Re-Enable interface
	m_pdlUnselected->Enabled = VARIANT_TRUE;
	SetControlState();

	return 0;
}

LRESULT CDiagCodeSearchDlg::OnNcHitTest(CPoint point) 
{
	/* Calculate the new position of the size grip */
	CRect rc;
	GetWindowRect(&rc);
	rc.top = rc.bottom - GetSystemMetrics( SM_CYHSCROLL );
	rc.left = rc.right - GetSystemMetrics( SM_CXVSCROLL ); 

	if (rc.PtInRect(point)) {
		return HTBOTTOMRIGHT;
	}
	
	return __super::OnNcHitTest(point);
}

BEGIN_EVENTSINK_MAP(CDiagCodeSearchDlg, CNxDialog)
	ON_EVENT(CDiagCodeSearchDlg, IDC_INSERT_SINGLE_ICD9_DL, 3, CDiagCodeSearchDlg::DblClickCellUnselectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CDiagCodeSearchDlg, IDC_INSERT_SINGLE_ICD9_DL, 29, CDiagCodeSearchDlg::SelSetUnselectedList, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CDiagCodeSearchDlg::DblClickCellUnselectedList(LPDISPATCH lpRow, short nColIndex)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow && VarLong(pRow->Value[dlcID], -1) != -1)
		{
			return __super::OnOK();
		}

		SetControlState();
	}NxCatchAll(__FUNCTION__);
}

void CDiagCodeSearchDlg::SelSetUnselectedList(LPDISPATCH lpRow)
{
	try
	{
		SetControlState();
	}NxCatchAll(__FUNCTION__);
}

int CDiagCodeSearchDlg::SetControlPositions()
{
	try
	{
		CRect rc;
		GetClientRect(rc);

		CWnd *pwndNxColor = GetDlgItem(IDC_INSERT_SINGLE_ICD9_NXC);
		CWnd *pwndUnselected = GetDlgItem(IDC_INSERT_SINGLE_ICD9_DL);

		rc.DeflateRect(10, 10);

		pwndNxColor->MoveWindow(rc);

		rc.DeflateRect(10, 10);

		m_editSearch.MoveWindow(rc.left, rc.top, rc.Width(), 20);
		pwndUnselected->MoveWindow(rc.left, rc.top + 25, rc.Width(), rc.Height() - 70);
		m_btnOK.MoveWindow((rc.left) + ((rc.Width() / 2) - 25) - 100, rc.bottom - 35, 100, 35);
		m_btnCancel.MoveWindow((rc.Width() / 2) + 45, rc.bottom - 35, 100, 35);

		return 1;
	}NxCatchAll(__FUNCTION__);
	return 0;
}

void CDiagCodeSearchDlg::SetControlState()
{
	m_btnOK.EnableWindow(m_pdlUnselected->CurSel && VarLong(m_pdlUnselected->CurSel->Value[dlcID], -1) != -1);
}