#include "stdafx.h"
#include "BillingRc.h"
#include "BillingDiagSearchDlg.h"
#include "DiagQuickListUtils.h"
#include "DiagQuickListDlg.h"
#include "DiagSearchUtils.h"
#include "DiagCodeInfo.h"
#include "NexCodeDlg.h"
#include "BillingDlg.h"
#include "ReplaceDiagCodeDlg.h"

// (j.armen 2014-08-06 10:06) - PLID 63161 - Created

using namespace NXDATALIST2Lib;
using namespace NexTech_Accessor;

enum DiagnosisCodeListColumns {
	dclcDiagCodeOrder = 0,
	dclcDiagICD9CodeID,
	dclcDiagICD9Code,
	dclcDiagICD9Desc,
	dclcDiagICD10CodeID,
	dclcDiagICD10Code,
	dclcDiagICD10Desc,
	dclcID,
};

// (j.jones 2014-12-22 10:48) - PLID 64490 - added ability to replace an existing code
enum class Cmd {
	ReplaceDiagCode = WM_USER,
	RemoveDiagCode,
	AddToDiagnosisQuicklist,
};

IMPLEMENT_DYNAMIC(CBillingDiagSearchDlg, CNxDialog)

// (j.jones 2014-12-22 10:48) - PLID 64490 - added ability to replace an existing code
CBillingDiagSearchDlg::CBillingDiagSearchDlg(long nEntryType, AddDiagCode fnAddDiagCode, DiagOrderModified fnDiagOrderModified,
	RemoveDiagCode fnRemoveDiagCode, ReplaceDiagCode fnReplaceDiagCode,
	SetICD9 fnSetICD9, SetICD10 fnSetICD10, DetectDuplicateDiagnosisCode fnDetectDuplicateDiagnosisCode,
	EndOfTabSequence fnEndOfTabSequence, BeginningOfTabSequence fnBeginningOfTabSequence,
	VerifyChargeCodeOrder fnVerifyChargeCodeOrder)
	: CNxDialog(IDD_BILL_DIAG_SEARCH_DLG, NULL),
	m_fnAddDiagCode(fnAddDiagCode), m_fnDiagOrderModified(fnDiagOrderModified),
	m_fnRemoveDiagCode(fnRemoveDiagCode), m_fnReplaceDiagCode(fnReplaceDiagCode),
	m_fnSetICD9(fnSetICD9), m_fnSetICD10(fnSetICD10), m_fnDetectDuplicateDiagnosisCode(fnDetectDuplicateDiagnosisCode),
	m_fnEndOfTabSequence(fnEndOfTabSequence), m_fnBeginningOfTabSequence(fnBeginningOfTabSequence),
	m_fnVerifyChargeCodeOrder(fnVerifyChargeCodeOrder)
{
	m_nEntryType = nEntryType;
}

CBillingDiagSearchDlg::~CBillingDiagSearchDlg()
{
}

BOOL CBillingDiagSearchDlg::Create(CWnd* pParent)
{
	return __super::Create(IDD_BILL_DIAG_SEARCH_DLG, pParent);
}

void CBillingDiagSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
}

BOOL CBillingDiagSearchDlg::OnInitDialog()
{
	__super::OnInitDialog();

	try
	{
		static_cast<CNxIconButton*>(GetDlgItem(IDC_BILLING_DIAG_UP))->AutoSet(NXB_UP);
		static_cast<CNxIconButton*>(GetDlgItem(IDC_BILLING_DIAG_DOWN))->AutoSet(NXB_DOWN);

		m_pDiagSearch = DiagSearchUtils::BindDiagPreferenceSearchListCtrl(this, IDC_BILLING_DIAG_SEARCH, GetRemoteDataSnapshot());
		m_pDiagList = BindNxDataList2Ctrl(IDC_BILLING_DIAG_LIST, false);
		m_pDiagList->GetColumn(dclcDiagCodeOrder)->BackColor = RGB(224, 224, 224);
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

BOOL CBillingDiagSearchDlg::PreTranslateMessage(MSG* pMsg)
{
	try
	{
		bool bShiftKeyIsDown = !!(GetAsyncKeyState(VK_SHIFT) & 0x80000000);

		if (pMsg->message == WM_KEYDOWN)
		{
			switch (pMsg->wParam)
			{
				case VK_TAB:	// (j.armen 2014-08-12 09:39) - PLID 63334 - When we tab off this element,
								// tell the owner that it should take focus (we have reached the end of our tab sequence
					if (!bShiftKeyIsDown)
					{
						if ((GetDlgItem(IDC_BILLING_DIAG_NEXCODE)->IsWindowVisible()
							? GetDlgItem(IDC_BILLING_DIAG_NEXCODE)
							: GetDlgItem(IDC_BILLING_DIAG_QUICKLIST))
							== __super::GetFocus())
						{
							m_fnEndOfTabSequence();
							return TRUE;
						}
					}
					else
					{
						// (j.armen 2014-08-14 14:20) - PLID 63334 - If using shift-tab
						//	Tell the host that it needs to take control in the reverse direction
						if (__super::GetFocus()
							&& __super::GetFocus()->GetParent() == GetDlgItem(IDC_BILLING_DIAG_SEARCH))
						{
							__super::PreTranslateMessage(pMsg); // PreTranslate the message so that the datalist can finish anything it is doing
							m_fnBeginningOfTabSequence();
							return TRUE;
						}
					}
					break;
			}
		}
	}NxCatchAll(__FUNCTION__);

	return __super::PreTranslateMessage(pMsg);
}

// (j.armen 2014-08-06 13:52) - PLID 63161 - Position Controls
int CBillingDiagSearchDlg::SetControlPositions()
{
	int nMoved = __super::SetControlPositions();

	CRect rcClient;
	GetClientRect(&rcClient);
	rcClient.DeflateRect(CSize(5, 5));

	CRect rc(rcClient);

	rc.bottom = rc.top + 20;
	rc.right -= 35;
	GetDlgItem(IDC_BILLING_DIAG_SEARCH)->MoveWindow(rc);

	rc.MoveToY(rc.bottom);
	rc.bottom = rcClient.Height() - 20;
	GetDlgItem(IDC_BILLING_DIAG_LIST)->MoveWindow(rc);

	GetDlgItem(IDC_BILLING_DIAG_UP)->MoveWindow(rcClient.Width() - 25, rc.top + (rc.Height() / 2) - 25 - 15, 30, 30);
	GetDlgItem(IDC_BILLING_DIAG_DOWN)->MoveWindow(rcClient.Width() - 25, rc.top + (rc.Height() / 2) + 25 - 15, 30, 30);

	rc.MoveToY(rc.bottom);
	rc.bottom = rc.top + 25;

	// (j.armen 2014-08-07 15:34) - PLID 63233 - When in ICD9 Only mode, hide NexCode
	if (DiagSearchUtils::GetPreferenceSearchStyle() == eManagedICD9_Search)
	{
		GetDlgItem(IDC_BILLING_DIAG_QUICKLIST)->MoveWindow(rc);

		GetDlgItem(IDC_BILLING_DIAG_NEXCODE)->ShowWindow(SW_HIDE);
	}
	else
	{
		rc.right = rc.left + (rc.Width() / 2);
		GetDlgItem(IDC_BILLING_DIAG_QUICKLIST)->MoveWindow(rc);

		rc.MoveToX(rc.right);
		GetDlgItem(IDC_BILLING_DIAG_NEXCODE)->MoveWindow(rc);
	}

	return nMoved;
}

// (j.armen 2014-08-08 10:43) - PLID 63242 - Helper for setting row style
// (j.armen 2014-08-08 11:08) - PLID 63245 - Set the tool tip override
void SetRowStyle(IRowSettingsPtr pRow, short nColID, short nColCode, short nColDesc)
{
	if (VarLong(pRow->Value[nColID], -1) == -1) {
		pRow->CellForeColor[nColCode] = RGB(255, 0, 0);

		// (j.armen 2014-08-14 09:24) - PLID 63231 - When moving rows up / down, if the row is empty, don't set the link style
		pRow->CellLinkStyle[nColCode] = (VarLong(pRow->Value[dclcDiagICD9CodeID], -1) != -1 || VarLong(pRow->Value[dclcDiagICD10CodeID], -1) != -1)
				? dlLinkStyleTrue
				: dlLinkStyleNotSet;
		pRow->CellToolTipOverride[nColCode] = g_cbstrNull;
	}
	else {
		pRow->CellForeColor[nColCode] = pRow->Value[dclcDiagCodeOrder] == g_cvarNull ? RGB(128, 128, 128) : RGB(0, 0, 0);
		pRow->CellLinkStyle[nColCode] = dlLinkStyleNotSet;
		pRow->CellToolTipOverride[nColCode] =
			(DiagSearchUtils::GetPreferenceSearchStyle() == eICD9_10_Crosswalk) // Always show in crosswalk
			|| (DiagSearchUtils::GetPreferenceSearchStyle() == eManagedICD9_Search && VarLong(pRow->Value[dclcDiagICD10CodeID], -1) != -1) // If searching 9's and 10 is present
			|| (DiagSearchUtils::GetPreferenceSearchStyle() == eManagedICD10_Search && VarLong(pRow->Value[dclcDiagICD9CodeID], -1) != -1) // If searching 10's and 9 is present
			? _bstr_t(VarString(pRow->Value[nColCode]) + " - " + VarString(pRow->Value[nColDesc]))
			: g_cbstrNull;
	}
}

// (j.armen 2014-08-08 10:43) - PLID 63242 - Helper for setting row style
void SetRowStyle(IRowSettingsPtr pRow)
{
	SetRowStyle(pRow, dclcDiagICD9CodeID, dclcDiagICD9Code, dclcDiagICD9Desc);
	SetRowStyle(pRow, dclcDiagICD10CodeID, dclcDiagICD10Code, dclcDiagICD10Desc);
}

// (j.armen 2014-08-07 10:57) - PLID 63227 - Set diagnosis codes
void CBillingDiagSearchDlg::SetDiagCodes(const std::vector<shared_ptr<struct DiagCodeInfo>>& aryDiagCodes) const
{
	m_pDiagList->Clear();
	for each(const auto& pDiag in aryDiagCodes)
	{
		auto pRow = m_pDiagList->GetNewRow();
		long nRowCount = m_pDiagList->GetRowCount();

		// (j.armen 2014-08-07 10:57) - PLID 63227 - For the first 12 codes, use A-L
		pRow->Value[dclcDiagCodeOrder] = (nRowCount < 12) ? _variant_t(static_cast<char>('A' + nRowCount)) : g_cvarNull;
		pRow->Value[dclcDiagICD9CodeID] = pDiag->nDiagCode9ID;
		pRow->Value[dclcDiagICD9Code] = _bstr_t(pDiag->strDiagCode9Code);
		pRow->Value[dclcDiagICD9Desc] = _bstr_t(pDiag->strDiagCode9Desc);
		pRow->Value[dclcDiagICD10CodeID] = pDiag->nDiagCode10ID;
		pRow->Value[dclcDiagICD10Code] = _bstr_t(pDiag->strDiagCode10Code);
		pRow->Value[dclcDiagICD10Desc] = _bstr_t(pDiag->strDiagCode10Desc);
		pRow->Value[dclcID] = pDiag->nID;

		SetRowStyle(pRow);	// (j.armen 2014-08-08 10:43) - PLID 63242 - Set Row Style
		m_pDiagList->AddRowAtEnd(pRow, NULL);
	}

	// (j.armen 2014-08-07 10:58) - PLID 63227 - If we don't have 12 rows, fillin the rest of the A-L place holders
	for (long nRowCount = m_pDiagList->GetRowCount(); nRowCount < 12; nRowCount++) {
		auto pRow = m_pDiagList->GetNewRow();
		pRow->Value[dclcDiagCodeOrder] = static_cast<char>('A' + nRowCount);
		pRow->Value[dclcDiagICD9CodeID] = g_cvarNull;
		pRow->Value[dclcDiagICD9Code] = g_cvarNull;
		pRow->Value[dclcDiagICD9Desc] = g_cvarNull;
		pRow->Value[dclcDiagICD10CodeID] = g_cvarNull;
		pRow->Value[dclcDiagICD10Code] = g_cvarNull;
		pRow->Value[dclcDiagICD10Desc] = g_cvarNull;
		pRow->Value[dclcID] = g_cvarNull;
		m_pDiagList->AddRowAtEnd(pRow, NULL);
	}

	// (j.armen 2014-08-07 10:59) - PLID 63227 - Follow our preferences as to which columns should be displayed
	DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference(m_pDiagList, dclcDiagICD9Code, dclcDiagICD10Code,
		50, 50, "", "", dclcDiagICD9Desc, dclcDiagICD10Desc, false, true, true);

	// (j.armen 2014-08-11 11:15) - PLID 63243 - After the columns have been sized, Add place holders where ICD-9 or ICD-10 are missing
	for (auto pRow = m_pDiagList->GetFirstRow(); pRow; pRow = pRow->GetNextRow())
	{
		long nICD9 = VarLong(pRow->Value[dclcDiagICD9CodeID], -1);
		long nICD10 = VarLong(pRow->Value[dclcDiagICD10CodeID], -1);

		if ((nICD9 != -1 || nICD10 != -1) && (nICD9 == -1 || nICD10 == -1))
		{
			if (nICD9 == -1)
				pRow->Value[dclcDiagICD9Code] = "< ICD-9 >";
			if (nICD10 == -1)
				pRow->Value[dclcDiagICD10Code] = "< ICD-10 >";
		}
	}

	CheckArrowEnabledState();	// (j.armen 2014-10-10 12:14) - PLID 63231 - Check arrow state after setting diag codes
}

// (j.armen 2014-08-07 15:12) - PLID 63231 - Provide the list of current diagnosis codes
const std::vector<shared_ptr<DiagCodeInfo>> CBillingDiagSearchDlg::GetDiagCodes() const
{
	std::vector<shared_ptr<DiagCodeInfo>> aryDiagCodes;
	long nOrder = 1;
	for (auto pRow = m_pDiagList->GetFirstRow(); pRow; pRow = pRow->GetNextRow()) {
		if (pRow->Value[dclcDiagICD9CodeID] != g_cvarNull || pRow->Value[dclcDiagICD10CodeID] != g_cvarNull) {
			shared_ptr<DiagCodeInfo> pDiag = make_shared<DiagCodeInfo>();
			pDiag->nID = VarLong(pRow->Value[dclcID], -1);
			pDiag->nOrderIndex = nOrder++;
			pDiag->nDiagCode9ID = VarLong(pRow->Value[dclcDiagICD9CodeID]);
			pDiag->strDiagCode9Code = VarLong(pRow->Value[dclcDiagICD9CodeID]) != -1 ? VarString(pRow->Value[dclcDiagICD9Code]) : "";
			pDiag->strDiagCode9Desc = VarLong(pRow->Value[dclcDiagICD9CodeID]) != -1 ? VarString(pRow->Value[dclcDiagICD9Desc]) : "";
			pDiag->nDiagCode10ID = VarLong(pRow->Value[dclcDiagICD10CodeID]);
			pDiag->strDiagCode10Code = VarLong(pRow->Value[dclcDiagICD10CodeID]) != -1 ? VarString(pRow->Value[dclcDiagICD10Code]) : "";
			pDiag->strDiagCode10Desc = VarLong(pRow->Value[dclcDiagICD10CodeID]) != -1 ? VarString(pRow->Value[dclcDiagICD10Desc]) : "";
			aryDiagCodes.push_back(pDiag);
		}
	}

	return aryDiagCodes;
}

// (j.armen 2014-08-14 14:20) - PLID 63334 - Handle the host telling us to take focus, from a shift-tab
void CBillingDiagSearchDlg::SetShiftTabFocus()
{
	if (GetDlgItem(IDC_BILLING_DIAG_NEXCODE)->IsWindowVisible())
		PostMessage(WM_NEXTDLGCTL, (WPARAM)GetDlgItem(IDC_BILLING_DIAG_NEXCODE)->GetSafeHwnd(), TRUE);
	else
		PostMessage(WM_NEXTDLGCTL, (WPARAM)GetDlgItem(IDC_BILLING_DIAG_QUICKLIST)->GetSafeHwnd(), TRUE);
}

// (j.armen 2014-08-07 13:45) - PLID 63231 - Verify the correct button state given our selection
void CBillingDiagSearchDlg::CheckArrowEnabledState() const
{
	IRowSettingsPtr pCurSel = m_pDiagList->CurSel;
	IRowSettingsPtr pFirstRow;
	IRowSettingsPtr pLastRow;

	for (auto pRow = m_pDiagList->GetFirstRow(); pRow; pRow = pRow->GetNextRow()) {
		if (pRow->Value[dclcDiagICD9CodeID] != g_cvarNull || pRow->Value[dclcDiagICD10CodeID] != g_cvarNull) {
			pFirstRow = pRow;
			break;
		}
	}

	for (auto pRow = m_pDiagList->GetLastRow(); pRow; pRow = pRow->GetPreviousRow()) {
		if (pRow->Value[dclcDiagICD9CodeID] != g_cvarNull || pRow->Value[dclcDiagICD10CodeID] != g_cvarNull) {
			pLastRow = pRow;
			break;
		}
	}

	GetDlgItem(IDC_BILLING_DIAG_UP)->EnableWindow(pCurSel && pCurSel != pFirstRow
		&& (pCurSel->Value[dclcDiagICD9CodeID] != g_cvarNull || pCurSel->Value[dclcDiagICD10CodeID] != g_cvarNull));
	GetDlgItem(IDC_BILLING_DIAG_DOWN)->EnableWindow(pCurSel && pCurSel != pLastRow
		&& (pCurSel->Value[dclcDiagICD9CodeID] != g_cvarNull || pCurSel->Value[dclcDiagICD10CodeID] != g_cvarNull));
}

// (j.armen 2014-08-07 15:12) - PLID 63231 - Swap two rows (Must be adjacent rows)	
void CBillingDiagSearchDlg::SwapRow(IRowSettingsPtr pRow, IRowSettingsPtr pSwapRow, bool bSwapForRemoval /*= false */)
{
	// (j.armen 2014-08-13 11:14) - PLID 63352 - Check if either of the rows, in their new positions would become the 13'th row.
	// If so, and this code is linked to charges, our parent will give the option to continue.
	// (j.armen 2014-10-14 11:55) - PLID 63236 - When swapping for row removal, we don't check to verify code order
	if (!bSwapForRemoval && (
			!m_fnVerifyChargeCodeOrder(std::pair<long, long>(VarLong(pRow->Value[dclcDiagICD9CodeID], -1), VarLong(pRow->Value[dclcDiagICD10CodeID], -1)), pRow->GetNextRow() == pSwapRow ? 1 : -1)
		||	!m_fnVerifyChargeCodeOrder(std::pair<long, long>(VarLong(pSwapRow->Value[dclcDiagICD9CodeID], -1), VarLong(pSwapRow->Value[dclcDiagICD10CodeID], -1)), pRow->GetNextRow() == pSwapRow ? -1 : 1)))
		return;

	// Save a list of all column values from the row are are swaping to
	std::vector<_variant_t> arySwapValues;
	for (int i = 0; i < m_pDiagList->GetColumnCount(); i++)
		arySwapValues.push_back(pSwapRow->Value[i]);

	// Skipping the first column, copy all values from the current row to the swap row
	for (int i = 1; i < m_pDiagList->GetColumnCount(); i++)
		pSwapRow->Value[i] = pRow->Value[i];

	// Skipping the first column, copy all the values from the saved array to the current row
	for (int i = 1; i < m_pDiagList->GetColumnCount(); i++)
		pRow->Value[i] = arySwapValues[i];

	// (j.armen 2014-08-08 10:43) - PLID 63242 - Set row styles for both rows
	SetRowStyle(pRow);
	SetRowStyle(pSwapRow);

	// Update the current row to be the row we just swapped into
	m_pDiagList->CurSel = pSwapRow;

	// Check Button State (we may have moved to the top, or to the bottom of our list)
	CheckArrowEnabledState();

	// Call our function to notify the parent that the order has been modified
	m_fnDiagOrderModified();
}

BEGIN_MESSAGE_MAP(CBillingDiagSearchDlg, CNxDialog)
	ON_WM_ENABLE()
	ON_BN_CLICKED(IDC_BILLING_DIAG_UP, CBillingDiagSearchDlg::OnBnClickedUp)
	ON_BN_CLICKED(IDC_BILLING_DIAG_DOWN, CBillingDiagSearchDlg::OnBnClickedDown)
	ON_BN_CLICKED(IDC_BILLING_DIAG_QUICKLIST, CBillingDiagSearchDlg::OnBnClickedDiagQuickList)
	ON_BN_CLICKED(IDC_BILLING_DIAG_NEXCODE, CBillingDiagSearchDlg::OnBnClickedNexCode)
	ON_COMMAND(Cmd::RemoveDiagCode, CBillingDiagSearchDlg::OnRemoveDiagCode)
	ON_UPDATE_COMMAND_UI(Cmd::AddToDiagnosisQuicklist, CBillingDiagSearchDlg::OnAddToDiagnosisQuicklistUI)
	ON_COMMAND(Cmd::AddToDiagnosisQuicklist, CBillingDiagSearchDlg::OnAddToDiagnosisQuicklist)
	ON_COMMAND(Cmd::ReplaceDiagCode, CBillingDiagSearchDlg::OnReplaceDiagCode)
END_MESSAGE_MAP()

// (j.armen 2014-08-06 14:49) - PLID 63161 - Handle enabling / disabling controls
void CBillingDiagSearchDlg::OnEnable(BOOL bEnable)
{
	try
	{
		for each(const CControlInfo& info in m_aryControls)
		{
			CWnd* pWnd = CWnd::FromHandle(info.hwnd);
			if (dynamic_cast<CNxStatic*>(pWnd))
				pWnd->Invalidate(); // We don't enable / disable labels, but they must still be invalidated
			else
				pWnd->EnableWindow(bEnable);
		}

		if (bEnable)	// (j.armen 2014-08-07 13:45) - PLID 63231 - When enabling, check our button states
			CheckArrowEnabledState();

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-08-07 15:12) - PLID 63231 - Handle Up Arrow
void CBillingDiagSearchDlg::OnBnClickedUp()
{
	try
	{
		SwapRow(m_pDiagList->CurSel, m_pDiagList->CurSel->GetPreviousRow());
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-08-07 15:12) - PLID 63231 - Handle Down Arrow
void CBillingDiagSearchDlg::OnBnClickedDown()
{
	try
	{
		SwapRow(m_pDiagList->CurSel, m_pDiagList->CurSel->GetNextRow());
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-08-11 11:56) - PLID 63238 - Handle adding to diag codes from the quick list
void CBillingDiagSearchDlg::OnBnClickedDiagQuickList()
{
	try
	{
		CDiagQuickListDlg dlg;
		if (IDOK == dlg.DoModal())
		{
			CString strWarning = "";
			for each(_DiagQuickListItemPtr pItem in dlg.m_aSelectedQuickListItems)
			{
				if (m_fnDetectDuplicateDiagnosisCode(
					(pItem->ICD9) ? AsLong(pItem->ICD9->ID) : -1,
					(pItem->ICD10) ? AsLong(pItem->ICD10->ID) : -1))
				{
					CString strCodeWarn = "";
					if ((pItem->ICD10) ? AsLong(pItem->ICD10->ID) : -1 > 0)
						strCodeWarn += " - " + ((pItem->ICD10) ? VarString(pItem->ICD10->Code) : "");

					if ((pItem->ICD9) ? AsLong(pItem->ICD9->ID) : -1 > 0)
					{
						if (!strCodeWarn.IsEmpty())
							strCodeWarn += " (" + ((pItem->ICD9) ? VarString(pItem->ICD9->Code) : "") + ") \r\n";
						else
							strCodeWarn += " - " + ((pItem->ICD9) ? VarString(pItem->ICD9->Code) : "") + "\r\n";
					}
					strWarning += strCodeWarn;
				}
				else
				{
					m_fnAddDiagCode(
						(pItem->ICD9) ? AsLong(pItem->ICD9->ID) : -1,
						(pItem->ICD10) ? AsLong(pItem->ICD10->ID) : -1,
						(pItem->ICD9) ? VarString(pItem->ICD9->Code) : "",
						(pItem->ICD10) ? VarString(pItem->ICD10->Code) : "",
						(pItem->ICD9) ? VarString(pItem->ICD9->description) : "",
						(pItem->ICD10) ? VarString(pItem->ICD10->description) : "");
				}
			}
			if (!strWarning.IsEmpty()) {
				//(b.spivey - March 18th, 2014) PLID 61245 - formatting change. 
				strWarning = "The following code(s) have been detected as duplicates. "
					"They will not be added to the bill. \r\n\r\n" + strWarning;
				AfxMessageBox(strWarning, MB_OK | MB_ICONWARNING, NULL);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-08-08 09:38) - PLID 63239 - Handle NexCode button
void CBillingDiagSearchDlg::OnBnClickedNexCode()
{
	try{
		CNexCodeDlg dlg(this);
		if (IDOK == dlg.DoModal())
			m_fnAddDiagCode(-1, dlg.GetDiagCodeID(), "", dlg.GetDiagCode(), "", dlg.GetDiagCodeDescription());
	}NxCatchAll(__FUNCTION__);
}


// (j.armen 2014-08-07 17:12) - PLID 63236 - Handle removing diag codes
void CBillingDiagSearchDlg::OnRemoveDiagCode()
{
	try
	{
		// Turn off redraw, we don't want any flickering
		m_pDiagList->SetRedraw(VARIANT_FALSE);

		try {
			// We'll need to start at the current selection and swap rows until the row to be 
			// removed is at the end of the list. We then remove the last row.
			// As such, tell the SwapRow function not to check for code order.
			for (auto pRow = m_pDiagList->CurSel; pRow; pRow = pRow->GetNextRow())
			{
				if (pRow->GetNextRow())
					SwapRow(pRow, pRow->GetNextRow(), true);
				else
				{
					// On the last row, if the code order is null, then just remove it
					if (pRow->Value[dclcDiagCodeOrder] == g_cvarNull)
						m_pDiagList->RemoveRow(pRow);
					else
					{
						// Otherwise, it is A-L, just set the contents to null
						for (int i = 1; i < m_pDiagList->GetColumnCount(); i++) {
							pRow->Value[i] = g_cvarNull;
							pRow->CellLinkStyle[i] = dlLinkStyleNotSet;
							pRow->CellToolTipOverride[i] = g_cbstrNull;
						}
					}
				}
			}

			m_pDiagList->CurSel = NULL;
		}NxCatchAllSilentCallThrow(
		{
			m_pDiagList->SetRedraw(VARIANT_TRUE);
		});

		// Make sure we turn redraw back on
		m_pDiagList->SetRedraw(VARIANT_TRUE);

		// Call our parent to tell them we removed a diag code
		m_fnRemoveDiagCode();

		// Since we removed a row, we may no longer have a selection.  Check the arrow state
		CheckArrowEnabledState();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-08-08 09:28) - PLID 63238 - Enable / Disable the quicklist menu option
void CBillingDiagSearchDlg::OnAddToDiagnosisQuicklistUI(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!!m_bCanAddToQuickList);
}

// (j.armen 2014-08-08 09:28) - PLID 63238 - Handle the action to add an item to the diagnosis quick list
void CBillingDiagSearchDlg::OnAddToDiagnosisQuicklist()
{
	try
	{
		NexTech_Accessor::_QuickListDiagCommitPtr pCommit(__uuidof(NexTech_Accessor::QuickListDiagCommit));
		if (VarLong(m_pDiagList->CurSel->Value[dclcDiagICD9CodeID]) > 0)
		{
			pCommit->ICD9DiagID = _bstr_t(AsString(m_pDiagList->CurSel->Value[dclcDiagICD9CodeID]));
		}
		if (VarLong(m_pDiagList->CurSel->Value[dclcDiagICD10CodeID]) > 0)
		{
			pCommit->ICD10DiagID = _bstr_t(AsString(m_pDiagList->CurSel->Value[dclcDiagICD10CodeID]));
		}
		Nx::SafeArray<IUnknown *> saCommits = Nx::SafeArray<IUnknown *>::FromValue(pCommit);
		GetAPI()->AddDiagCodesToQuickList(GetAPISubkey(), GetAPILoginToken(), saCommits);
	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CBillingDiagSearchDlg, CNxDialog)
	ON_EVENT(CBillingDiagSearchDlg, IDC_BILLING_DIAG_SEARCH, 16 /* SelChosen */, CBillingDiagSearchDlg::SelChosenDiagSearch, VTS_DISPATCH)
	ON_EVENT(CBillingDiagSearchDlg, IDC_BILLING_DIAG_LIST, 1 /* SelChanging */, CBillingDiagSearchDlg::SelChangingDiagList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CBillingDiagSearchDlg, IDC_BILLING_DIAG_LIST, 2 /* SelChanged */, CBillingDiagSearchDlg::SelChangedDiagList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CBillingDiagSearchDlg, IDC_BILLING_DIAG_LIST, 19 /* LeftClick */, CBillingDiagSearchDlg::LeftClickDiagList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CBillingDiagSearchDlg, IDC_BILLING_DIAG_LIST, 32 /* ShowContextMenu */, CBillingDiagSearchDlg::ShowContextMenuDiagList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4 VTS_PBOOL)
END_EVENTSINK_MAP()

// (j.armen 2014-08-06 16:23) - PLID 63161 - Handle when a diag code is selected
void CBillingDiagSearchDlg::SelChosenDiagSearch(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			CDiagSearchResults dsrResult = DiagSearchUtils::ConvertPreferenceSearchResults(pRow);
			//do not save a <no result> value
			if (dsrResult.m_ICD9.m_nDiagCodesID == -1 && dsrResult.m_ICD10.m_nDiagCodesID == -1) {
				return;
			}
			//load data from our result into our code info object
			m_fnAddDiagCode(
				dsrResult.m_ICD9.m_nDiagCodesID, dsrResult.m_ICD10.m_nDiagCodesID,
				dsrResult.m_ICD9.m_strCode, dsrResult.m_ICD10.m_strCode,
				dsrResult.m_ICD9.m_strDescription, dsrResult.m_ICD10.m_strDescription);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-08-07 13:45) - PLID 63231 - Only allow actual diagnosis codes to be selected.  Otherwise, set the selection to NULL
void CBillingDiagSearchDlg::SelChangingDiagList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try
	{
		IRowSettingsPtr pNewRow(*lppNewSel);
		if (pNewRow && pNewRow->Value[dclcDiagICD9CodeID] == g_cvarNull && pNewRow->Value[dclcDiagICD10CodeID] == g_cvarNull)
			SafeSetCOMPointer(lppNewSel, static_cast<LPDISPATCH>(NULL));

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-08-07 13:45) - PLID 63231 - When selection changes, update the button state
void CBillingDiagSearchDlg::SelChangedDiagList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{
		CheckArrowEnabledState();
	}NxCatchAll(__FUNCTION__);
}

void CBillingDiagSearchDlg::LeftClickDiagList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if (pRow)
		{
			long nDiag9ID = VarLong(pRow->GetValue(dclcDiagICD9CodeID), -1);
			long nDiag10ID = VarLong(pRow->GetValue(dclcDiagICD10CodeID), -1);

			if (nDiag9ID == -1 && nDiag10ID == -1)
				return;

			switch (nCol)
			{
				case dclcDiagICD9Code:	// (j.armen 2014-08-08 10:43) - PLID 63243 - Select ICD-9
					if (nDiag9ID == -1)
						m_fnSetICD9(std::pair<long, long>(nDiag9ID, nDiag10ID));
					return;
				case dclcDiagICD10Code:	// (j.armen 2014-08-08 10:46) - PLID 63242 - Open NexCode
					if (nDiag10ID == -1)
						m_fnSetICD10(std::pair<long, long>(nDiag9ID, nDiag10ID));
					return;
			}

		}
	}NxCatchAll(__FUNCTION__);

}

// (j.armen 2014-08-07 17:12) - PLID 63236 - Right click context menu
void CBillingDiagSearchDlg::ShowContextMenuDiagList(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, VARIANT_BOOL* pbContinue)
{
	try {
		IRowSettingsPtr pRow(lpRow);

		// If user right clicks off a row, or on a row that does not have a code, clear the selection and return
		if (!pRow || (pRow->Value[dclcDiagICD9CodeID] == g_cvarNull && pRow->Value[dclcDiagICD10CodeID] == g_cvarNull))
		{
			m_pDiagList->CurSel = NULL;
			CheckArrowEnabledState();
			return;
		}
		else
		{
			// User clicked a valid row, set the selection
			m_pDiagList->CurSel = pRow;
			CheckArrowEnabledState();
		}

		*pbContinue = VARIANT_FALSE;

		// (j.armen 2014-08-08 09:28) - PLID 63238 - Before we can show the context menu, we need to determine if the current row can be added to the diag quicklist
		switch (DiagQuickListUtils::GetAPIDiagDisplayType())
		{
			case NexTech_Accessor::DiagDisplayType_Crosswalk:
				m_bCanAddToQuickList = (VarLong(m_pDiagList->CurSel->Value[dclcDiagICD9CodeID], -1) != -1 && VarLong(m_pDiagList->CurSel->Value[dclcDiagICD10CodeID], -1) != -1);
				break;
			case NexTech_Accessor::DiagDisplayType_ICD10:
				m_bCanAddToQuickList = (VarLong(m_pDiagList->CurSel->Value[dclcDiagICD10CodeID], -1) != -1);
				break;
			default:
				m_bCanAddToQuickList = (VarLong(m_pDiagList->CurSel->Value[dclcDiagICD9CodeID], -1) != -1);
				break;
		}

		if (m_bCanAddToQuickList)
		{
			NexTech_Accessor::_DiagnosisCrosswalkCommitPtr pCommit(__uuidof(NexTech_Accessor::DiagnosisCrosswalkCommit));
			NexTech_Accessor::_DiagnosisCodeCommitPtr pICD9(__uuidof(NexTech_Accessor::DiagnosisCodeCommit));
			NexTech_Accessor::_DiagnosisCodeCommitPtr pICD10(__uuidof(NexTech_Accessor::DiagnosisCodeCommit));
			if (VarLong(m_pDiagList->CurSel->Value[dclcDiagICD9CodeID]) > 0)
				pICD9->ID = _bstr_t(AsString(m_pDiagList->CurSel->Value[dclcDiagICD9CodeID]));
			if (VarLong(m_pDiagList->CurSel->Value[dclcDiagICD10CodeID]) > 0)
				pICD10->ID = _bstr_t(AsString(m_pDiagList->CurSel->Value[dclcDiagICD10CodeID]));
			pCommit->ICD9 = pICD9;
			pCommit->ICD10 = pICD10;
			Nx::SafeArray<IUnknown *> saCommits = Nx::SafeArray<IUnknown *>::FromValue(pCommit);
			NexTech_Accessor::_DiagnosisCrosswalksPtr pFoundCrosswalks = GetAPI()->GetCrosswalksInQuickListByDisplayPreference(GetAPISubkey(), GetAPILoginToken(), saCommits);
			Nx::SafeArray<IUnknown *> saResults(pFoundCrosswalks->Items);

			m_bCanAddToQuickList = (saResults.GetCount() == 0);
		}

		// Create and display popup
		CNxMenu menu;
		menu.CreatePopupMenu();

		// (j.jones 2014-12-22 10:48) - PLID 64490 - added ability to replace an existing code
		menu.AppendMenu(MF_ENABLED, static_cast<UINT_PTR>(Cmd::ReplaceDiagCode), "&Replace Diagnosis Code");
		menu.AppendMenu(MF_ENABLED, static_cast<UINT_PTR>(Cmd::RemoveDiagCode), "Remo&ve Diagnosis Code");
		menu.AppendMenu(MF_ENABLED, static_cast<UINT_PTR>(Cmd::AddToDiagnosisQuicklist), "&Add to Diagnosis Quicklist");

		menu.ShowPopupMenu(CPoint(x, y), this, TRUE);

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-12-22 10:48) - PLID 64490 - added ability to replace an existing code
void CBillingDiagSearchDlg::OnReplaceDiagCode()
{
	try
	{
		IRowSettingsPtr pRow = m_pDiagList->CurSel;
		if (pRow == NULL) {
			return;
		}

		DiagCodeInfoPtr oldDiagCode = make_shared<DiagCodeInfo>();

		oldDiagCode->nDiagCode9ID = VarLong(pRow->GetValue(dclcDiagICD9CodeID), -1);
		if (oldDiagCode->nDiagCode9ID != -1) {
			oldDiagCode->strDiagCode9Code = VarString(pRow->GetValue(dclcDiagICD9Code), "");
			oldDiagCode->strDiagCode9Desc = VarString(pRow->GetValue(dclcDiagICD9Desc), "");
		}		
		oldDiagCode->nDiagCode10ID = VarLong(pRow->GetValue(dclcDiagICD10CodeID), -1);
		if (oldDiagCode->nDiagCode10ID != -1) {
			oldDiagCode->strDiagCode10Code = VarString(pRow->GetValue(dclcDiagICD10Code), "");
			oldDiagCode->strDiagCode10Desc = VarString(pRow->GetValue(dclcDiagICD10Desc), "");
		}

		long nNewDiag9CodeID = -1;
		long nNewDiag10CodeID = -1;

		CReplaceDiagCodeDlg dlg(this);
		if (IDOK == dlg.DoModal(m_nEntryType == 1 ? 0xFFC0C0 : RGB(255, 179, 128), oldDiagCode)) {

			if (dlg.m_newDiagCode->nDiagCode9ID == -1 && dlg.m_newDiagCode->nDiagCode10ID == -1) {
				//the dialog should not have allowed this
				ASSERT(FALSE);
				return;
			}

			//make sure the code does not already exist

			// Call our parent to tell them we replaced a diag code
			m_fnReplaceDiagCode(oldDiagCode, dlg.m_newDiagCode);
		}

	}NxCatchAll(__FUNCTION__);
}