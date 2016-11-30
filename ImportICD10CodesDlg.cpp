#include "stdafx.h"
#include "ImportICD10CodesDlg.h"
#include "NxAPI.h"
#include "NxThread.h"
#include "AsyncDiagSearchQuery.h"	// (j.armen 2014-03-20 09:47) - PLID 60943 - Async search is a little more generic

using namespace NXDATALIST2Lib;
using namespace NexTech_Accessor;

enum CodesList {
	dlcID,
	dlcCode,
	dlcDescription,
	dlcPCS,
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

CImportICD10CodesDlg::CImportICD10CodesDlg(CWnd* pParent)
	: CNxDialog(IDD_IMPORT_ICD10, pParent, "ImportICD10DiagCodes")
{
	// (j.armen 2014-03-10 08:26) - PLID 61210 - Set our min dlg size
	__super::SetMinSize(450, 400);
}

CImportICD10CodesDlg::~CImportICD10CodesDlg()
{
}

// (j.armen 2014-03-10 08:26) - PLID 61210 - Handle setting Icon, binding controls
BOOL CImportICD10CodesDlg::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();

	try
	{
		HICON hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
		SetIcon(hIcon, TRUE);
		SetIcon(hIcon, FALSE);

		m_nxcolor.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_btnMoveRight.AutoSet(NXB_RIGHT);
		m_btnMoveLeft.AutoSet(NXB_LEFT);
		m_btnMoveLLeft.AutoSet(NXB_LLEFT);
		m_btnImport.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_pdlUnselected = BindNxDataList2Ctrl(IDC_IMPORT_ICD10_NXDATALISTCTRL_LEFT, false);
		m_pdlSelected = BindNxDataList2Ctrl(IDC_IMPORT_ICD10_NXDATALISTCTRL_RIGHT, false);

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

void CImportICD10CodesDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IMPORT_ICD10_NXCOLOR2, m_nxcolor);
	DDX_Control(pDX, IDC_IMPORT_ICD10_SEARCH, m_editSearch);
	DDX_Control(pDX, IDC_IMPORT_ICD10_BUTTON_MOVE_RIGHT, m_btnMoveRight);
	DDX_Control(pDX, IDC_IMPORT_ICD10_BUTTON_MOVE_LEFT, m_btnMoveLeft);
	DDX_Control(pDX, IDC_IMPORT_ICD10_BUTTON_MOVE_LLEFT, m_btnMoveLLeft);
	DDX_Control(pDX, IDOK, m_btnImport);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BEGIN_MESSAGE_MAP(CImportICD10CodesDlg, CNxDialog)
	ON_EN_CHANGE(IDC_IMPORT_ICD10_SEARCH, &CImportICD10CodesDlg::OnEnSearchModified)
	ON_REGISTERED_MESSAGE(AsyncDiagSearchQuery::NotifyMessage, &CImportICD10CodesDlg::OnMessageSearchResult)
	ON_BN_CLICKED(IDC_IMPORT_ICD10_BUTTON_MOVE_RIGHT, &CImportICD10CodesDlg::OnBnClickedMoveRight)
	ON_BN_CLICKED(IDC_IMPORT_ICD10_BUTTON_MOVE_LEFT, &CImportICD10CodesDlg::OnBnClickedMoveLeft)
	ON_BN_CLICKED(IDC_IMPORT_ICD10_BUTTON_MOVE_LLEFT, &CImportICD10CodesDlg::OnBnClickedMoveLLeft)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

// (j.armen 2014-03-10 08:32) - PLID 61284 - Fired when search text is modified
void CImportICD10CodesDlg::OnEnSearchModified()
{
	// Reset timer
	m_TimerInstance.reset();

	// Disable selection controls.  These will be re-enabled when the new data is loaded
	m_pdlUnselected->Enabled = VARIANT_FALSE;
	m_pdlSelected->Enabled = VARIANT_FALSE;
	m_btnMoveRight.EnableWindow(FALSE);
	m_btnMoveLeft.EnableWindow(FALSE);
	m_btnMoveLLeft.EnableWindow(FALSE);

	// Set timer to trigger in X ms.  This will start the async search
	m_TimerInstance = WindowlessTimer::OnElapsed(500, boost::bind(&CImportICD10CodesDlg::TriggerAsyncSearch, this), false);
}

// (j.armen 2014-03-10 08:32) - PLID 61284 - Called by timer to trigger the async search
void CImportICD10CodesDlg::TriggerAsyncSearch()
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
		NxThread(boost::bind(&AsyncDiagSearchQuery::Run, m_pPendingData, GetSafeHwnd(), strSearchText, FullICD10Only))->RunDetached();

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-03-10 08:32) - PLID 61284 - Receives notification that an async query has completed.
LRESULT CImportICD10CodesDlg::OnMessageSearchResult(WPARAM wParam, LPARAM lParam)
{
	// Make sure the result that posted is the current executing search
	if (lParam != (LPARAM)m_pPendingData.get())
        return 0; // Must be an old search, ignore

	// Move the pending data to live data
	swap(m_pData, m_pPendingData);
	m_pPendingData.reset();
	
	// Load our records
	m_pdlUnselected->Clear();
	// (j.armen 2014-03-20 09:47) - PLID 60943 - Diag Codes from the search are now stored in a pair of 9's and 10's
	for each(std::pair<shared_ptr<DiagCode9>, shared_ptr<DiagCode10>> code in m_pData->aryCodes)
	{
		// Only add codes that are not on our selected list
		if(code.second &&
			!m_pdlSelected->FindByColumn(dlcCode, code.second->strCode, NULL, VARIANT_FALSE))
		{
			IRowSettingsPtr pRow = m_pdlUnselected->GetNewRow();
			pRow->Value[dlcID] = code.second->nDiagCodesID;
			pRow->Value[dlcCode] = code.second->strCode;
			pRow->Value[dlcDescription] = code.second->strDescription;
			pRow->Value[dlcPCS] = code.second->vbPCS ? g_cvarTrue : g_cvarFalse;
			pRow->BackColor = code.second->nBackColor;
			m_pdlUnselected->AddRowAtEnd(pRow, NULL);
		}
	}
	m_pdlUnselected->Sort();

	// Re-Enable interface
	m_pdlUnselected->Enabled = VARIANT_TRUE;
	m_pdlSelected->Enabled = VARIANT_TRUE;
	SetControlState();

	return 0;
}

// (j.armen 2014-03-10 08:26) - PLID 61210 - Handle moving codes from unselected to selected
void CImportICD10CodesDlg::OnBnClickedMoveRight()
{
	try
	{
		for each(IRowSettingsPtr pRow in GetSelectedRows(m_pdlUnselected))
		{
			m_pdlUnselected->RemoveRow(pRow);
			m_pdlSelected->AddRowSorted(pRow, NULL);
		}

		SetControlState();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-03-10 08:26) - PLID 61210 -  Handle moving codes from selected to unselected
void CImportICD10CodesDlg::OnBnClickedMoveLeft()
{
	try
	{
		for each(IRowSettingsPtr pRow in GetSelectedRows(m_pdlSelected))
		{
			m_pdlSelected->RemoveRow(pRow);
			m_pdlUnselected->AddRowSorted(pRow, NULL);
		}

		SetControlState();
	}NxCatchAll(__FUNCTION__);
}

void CImportICD10CodesDlg::OnBnClickedMoveLLeft()
{
	try
	{
		for each(IRowSettingsPtr pRow in GetAllRows(m_pdlSelected))
		{
			m_pdlSelected->RemoveRow(pRow);
			m_pdlUnselected->AddRowSorted(pRow, NULL);
		}

		SetControlState();
	}NxCatchAll(__FUNCTION__);
}

LRESULT CImportICD10CodesDlg::OnNcHitTest(CPoint point) 
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

// (j.armen 2014-03-10 08:26) - PLID 61210 - Run the modal dialog
int CImportICD10CodesDlg::DoModal()
{
	int nRet = __super::DoModal();
	if(nRet == IDOK)
	{
		try
		{
			Nx::SafeArray<IUnknown *> aryCommits;
			for each(IRowSettingsPtr pRow in GetAllRows(m_pdlSelected))
			{
				_DiagnosisCodeCommitPtr pCode(__uuidof(DiagnosisCodeCommit));
				pCode->Code = _bstr_t(pRow->Value[dlcCode]);
				pCode->description = _bstr_t(pRow->Value[dlcDescription]);
				pCode->ICD10 = true;
				_NullableBoolPtr pIsAMA(__uuidof(NullableBool));
				pIsAMA->SetBool(VARIANT_FALSE);
				pCode->IsAMA = pIsAMA;
				pCode->PCS = VarBool(pRow->Value[dlcPCS]) ? VARIANT_TRUE : VARIANT_FALSE;
				aryCommits.Add(pCode);
			}

			//Now create the code via the API
			GetAPI()->CreateDiagnosisCodes(GetAPISubkey(), GetAPILoginToken(), aryCommits);

			AfxMessageBox("Diagnosis code import successful!", MB_OK | MB_ICONINFORMATION);

		}NxCatchAll(__FUNCTION__);
	}
	return nRet;
}

BEGIN_EVENTSINK_MAP(CImportICD10CodesDlg, CNxDialog)
	ON_EVENT(CImportICD10CodesDlg, IDC_IMPORT_ICD10_NXDATALISTCTRL_LEFT, 3, CImportICD10CodesDlg::DblClickCellUnselectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CImportICD10CodesDlg, IDC_IMPORT_ICD10_NXDATALISTCTRL_RIGHT, 3, CImportICD10CodesDlg::DblClickCellSelectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CImportICD10CodesDlg, IDC_IMPORT_ICD10_NXDATALISTCTRL_LEFT, 29, CImportICD10CodesDlg::SelSetUnselectedList, VTS_DISPATCH)
	ON_EVENT(CImportICD10CodesDlg, IDC_IMPORT_ICD10_NXDATALISTCTRL_RIGHT, 29, CImportICD10CodesDlg::SelSetSelectedList, VTS_DISPATCH)
END_EVENTSINK_MAP()

// (j.armen 2014-03-10 08:26) - PLID 61210 - Handle double click to select
void CImportICD10CodesDlg::DblClickCellUnselectedList(LPDISPATCH lpRow, short nColIndex)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow && VarLong(pRow->Value[dlcID], -1) != -1)
		{
			m_pdlUnselected->RemoveRow(pRow);
			m_pdlSelected->AddRowSorted(pRow, NULL);
		}

		SetControlState();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-03-10 08:26) - PLID 61210 - Hande double click to unselect
void CImportICD10CodesDlg::DblClickCellSelectedList(LPDISPATCH lpRow, short nColIndex)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow)
		{
			m_pdlSelected->RemoveRow(pRow);
			m_pdlUnselected->AddRowSorted(pRow, NULL);
		}

		SetControlState();
	}NxCatchAll(__FUNCTION__);
}

void CImportICD10CodesDlg::SelSetUnselectedList(LPDISPATCH lpRow)
{
	try
	{
		SetControlState();
	}NxCatchAll(__FUNCTION__);
}

void CImportICD10CodesDlg::SelSetSelectedList(LPDISPATCH lpRow)
{
	try
	{
		SetControlState();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-03-10 08:26) - PLID 61210 - Handle resizing controls
int CImportICD10CodesDlg::SetControlPositions()
{
	try
	{
		CRect rc;
		GetClientRect(rc);

		CWnd *pwndNxColor = GetDlgItem(IDC_IMPORT_ICD10_NXCOLOR2);
		CWnd *pwndUnselected = GetDlgItem(IDC_IMPORT_ICD10_NXDATALISTCTRL_LEFT);
		CWnd *pwndSelected = GetDlgItem(IDC_IMPORT_ICD10_NXDATALISTCTRL_RIGHT);
		CWnd *pwndUnselectedLabel = GetDlgItem(IDC_IMPORT_ICD10_UNSEL_LABEL);
		CWnd *pwndSelectedLabel = GetDlgItem(IDC_IMPORT_ICD10_SEL_LABEL);

		rc.DeflateRect(10, 10);

		pwndNxColor->MoveWindow(rc);

		rc.DeflateRect(10, 10);

		m_editSearch.MoveWindow(rc.left, rc.top, (rc.Width() / 2) - 25, 20);
		pwndUnselected->MoveWindow(rc.left, rc.top + 25, (rc.Width() / 2) - 25, rc.Height() - 80);
		m_btnImport.MoveWindow((rc.left) + ((rc.Width() / 2) - 25) - 100, rc.bottom - 35, 100, 35);
		pwndUnselectedLabel->MoveWindow(rc.left, rc.bottom - 50, (rc.Width() / 2) - 25, 15);

		m_btnMoveLeft.MoveWindow(rc.Width() / 2, (rc.Height() / 2) + 0 - 25, 40, 40);
		m_btnMoveLLeft.MoveWindow(rc.Width() / 2, (rc.Height() / 2) + 50 - 25, 40, 40);
		m_btnMoveRight.MoveWindow(rc.Width() / 2, (rc.Height() / 2) - 50 - 25, 40, 40);

		pwndSelected->MoveWindow((rc.Width() / 2) + 45, rc.top, (rc.Width() / 2) - 25, rc.Height() - 55);
		m_btnCancel.MoveWindow((rc.Width() / 2) + 45, rc.bottom - 35, 100, 35);
		pwndSelectedLabel->MoveWindow((rc.Width() / 2) + 45, rc.bottom - 50, (rc.Width() / 2) - 25, 20);

		return 1;
	}NxCatchAll(__FUNCTION__);
	return 0;
}

void CImportICD10CodesDlg::SetControlState()
{
	m_btnImport.EnableWindow(!!m_pdlSelected->GetRowCount());
	m_btnMoveRight.EnableWindow(!!m_pdlUnselected->CurSel && VarLong(m_pdlUnselected->CurSel->Value[dlcID], -1) != -1);
	m_btnMoveLeft.EnableWindow(!!m_pdlSelected->CurSel);
	m_btnMoveLLeft.EnableWindow(!!m_pdlSelected->GetRowCount());
}