// RuleEntryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RuleEntryDlg.h"
#include "MultiSelectDlg.h"
#include "TemplateRuleInfo.h"
#include "GlobalDataUtils.h"
#include "GlobalDrawingUtils.h"
#include "SchedulerRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TEXT_IS		"is"
#define TEXT_ISNOT	"is not"

/////////////////////////////////////////////////////////////////////////////
// CRuleEntryDlg dialog


CRuleEntryDlg::CRuleEntryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CRuleEntryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRuleEntryDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_rcTypeList.left = m_rcTypeList.top = m_rcTypeList.right = m_rcTypeList.bottom = 0;
	m_rcTypeListIs.left = m_rcTypeListIs.top = m_rcTypeListIs.right = m_rcTypeListIs.bottom = 0;
	m_rcTypeListList.left = m_rcTypeListList.top = m_rcTypeListList.right = m_rcTypeListList.bottom = 0;
	m_strTypeListIs = TEXT_IS;
	m_aryTypeListIDs.RemoveAll();
	m_strTypeListNames = "";

	
	m_rcPurposeList.left = m_rcPurposeList.top = m_rcPurposeList.right = m_rcPurposeList.bottom = 0;
	m_rcPurposeListIs.left = m_rcPurposeListIs.top = m_rcPurposeListIs.right = m_rcPurposeListIs.bottom = 0;
	m_rcPurposeListList.left = m_rcPurposeListList.top = m_rcPurposeListList.right = m_rcPurposeListList.bottom = 0;
	m_strPurposeListIs = TEXT_IS;
	m_aryPurposeListIDs.RemoveAll();
	m_strPurposeListNames = "";

	m_rcBookingLabel.left = m_rcBookingLabel.top = m_rcBookingLabel.right = m_rcBookingLabel.bottom = 0;
	m_rcBookingCount.left = m_rcBookingCount.top = m_rcBookingCount.right = m_rcBookingCount.bottom = 0;
	m_nBookingCount = 1;

	m_rcAndDetailsLabel.left = m_rcAndDetailsLabel.top = m_rcAndDetailsLabel.right = m_rcAndDetailsLabel.bottom = 0;
	m_rcAllOrAny.left = m_rcAllOrAny.top = m_rcAllOrAny.right = m_rcAllOrAny.bottom = 0;
	m_bAndDetails = TRUE;

	m_pRuleInfo = NULL;
}

void CRuleEntryDlg::OnDestroy() 
{
	CNxDialog::OnDestroy();
}

void CRuleEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRuleEntryDlg)
	DDX_Control(pDX, IDC_DESCRIPTION_EDIT, m_nxeditDescriptionEdit);
	DDX_Control(pDX, IDC_WARNING_EDIT, m_nxeditWarningEdit);
	DDX_Control(pDX, IDC_LISTOF_TYPES, m_nxstaticListofTypes);
	DDX_Control(pDX, IDC_LISTOF_PURPOSES, m_nxstaticListofPurposes);
	DDX_Control(pDX, IDC_LISTOF_BOOKINGS, m_nxstaticListofBookings);
	DDX_Control(pDX, IDC_LISTOF_ANDDETAILS, m_nxstaticListofAnddetails);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_DESC_GROUPBOX, m_btnDescGroupbox);
	DDX_Control(pDX, IDC_RULE_OPTIONS_GROUPBOX, m_btnRuleOptionsGroupbox);
	DDX_Control(pDX, IDC_WARNING_GROUPBOX, m_btnWarningGroupbox);
	DDX_Control(pDX, IDC_ALL_CHECK, m_checkAll);
	DDX_Control(pDX, IDC_TYPE_CHECK, m_checkType);
	DDX_Control(pDX, IDC_PURPOSE_CHECK, m_checkPurpose);
	DDX_Control(pDX, IDC_BOOKING_CHECK, m_checkBooking);
	DDX_Control(pDX, IDC_WARN_CHECK, m_checkWarn);
	DDX_Control(pDX, IDC_PREVENT_CHECK, m_checkPrevent);
	DDX_Control(pDX, IDC_OVERRIDE_LOCATION_TEMPLATING, m_checkOverrideLocationTemplating);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRuleEntryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CRuleEntryDlg)
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_TYPE_CHECK, OnTypeCheck)
	ON_BN_CLICKED(IDC_PURPOSE_CHECK, OnPurposeCheck)
	ON_BN_CLICKED(IDC_WARN_CHECK, OnWarnCheck)
	ON_BN_CLICKED(IDC_BOOKING_CHECK, OnBookingCheck)
	ON_BN_CLICKED(IDC_ALL_CHECK, OnAllCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRuleEntryDlg message handlers

void CRuleEntryDlg::DrawTypeList(CDC *pdc)
{
	BOOL bSelectionEnabled = IsDlgButtonChecked(IDC_TYPE_CHECK);

	// Draw the "is" or "is not" text
	{
		m_rcTypeListIs = m_rcTypeList;

		// (j.jones 2008-05-01 16:42) - PLID 29874 - Set background color to transparent
		DrawTextOnDialog(this, pdc, m_rcTypeListIs, m_strTypeListIs, bSelectionEnabled?dtsHyperlink:dtsDisabledHyperlink, true, DT_LEFT, true, false, 0);
	}

	BOOL bTextSelectionEnabled = GetDlgItem(IDC_TYPE_CHECK)->IsWindowEnabled();

	// Draw the text: " one of "
	CRect rc(m_rcTypeList);
	rc.left = m_rcTypeListIs.right;

	// (j.jones 2008-05-01 16:42) - PLID 29874 - Set background color to transparent
	DrawTextOnDialog(this, pdc, rc, " one of ", bTextSelectionEnabled?dtsText:dtsDisabledText, true, DT_LEFT, true, false, 0);

	// Draw the type list
	{
		m_rcTypeListList = m_rcTypeList;
		m_rcTypeListList.left = rc.right;
		CString strTypeListNames = m_strTypeListNames;
		strTypeListNames.TrimLeft(); strTypeListNames.TrimRight();
		if (strTypeListNames.IsEmpty()) {
			strTypeListNames = "Select Types";		
		}

		// (j.jones 2008-05-01 16:42) - PLID 29874 - Set background color to transparent
		DrawTextOnDialog(this, pdc, m_rcTypeListList, strTypeListNames, bSelectionEnabled?dtsHyperlink:dtsDisabledHyperlink, true, DT_LEFT, true, false, 0);
	}
}

void CRuleEntryDlg::DrawPurposeList(CDC *pdc)
{
	BOOL bSelectionEnabled = IsDlgButtonChecked(IDC_PURPOSE_CHECK);

	// Draw the "is" or "is not" text
	{
		m_rcPurposeListIs = m_rcPurposeList;

		// (j.jones 2008-05-01 16:42) - PLID 29874 - Set background color to transparent
		DrawTextOnDialog(this, pdc, m_rcPurposeListIs, m_strPurposeListIs, bSelectionEnabled?dtsHyperlink:dtsDisabledHyperlink, true, DT_LEFT, true, false, 0);
	}

	BOOL bTextSelectionEnabled = GetDlgItem(IDC_PURPOSE_CHECK)->IsWindowEnabled();

	// Draw the text: " one of "
	CRect rc(m_rcPurposeList);
	rc.left = m_rcPurposeListIs.right;

	// (j.jones 2008-05-01 16:42) - PLID 29874 - Set background color to transparent
	DrawTextOnDialog(this, pdc, rc, " one of ", bTextSelectionEnabled?dtsText:dtsDisabledText, true, DT_LEFT, true, false, 0);

	// Draw the purpose list
	{
		m_rcPurposeListList = m_rcPurposeList;
		m_rcPurposeListList.left = rc.right;
		CString strPurposeListNames = m_strPurposeListNames;
		strPurposeListNames.TrimLeft(); strPurposeListNames.TrimRight();
		if (strPurposeListNames.IsEmpty()) {
			strPurposeListNames = "Select Purposes";
		}

		// (j.jones 2008-05-01 16:42) - PLID 29874 - Set background color to transparent
		DrawTextOnDialog(this, pdc, m_rcPurposeListList, strPurposeListNames, bSelectionEnabled?dtsHyperlink:dtsDisabledHyperlink, true, DT_LEFT, true, false, 0);
	}
}

void CRuleEntryDlg::DrawBookingLabel(CDC *pdc)
{
	BOOL bSelectionEnabled = IsDlgButtonChecked(IDC_BOOKING_CHECK);

	// Draw the "1" or "2" or whatever number the booking max is
	{
		m_rcBookingCount = m_rcBookingLabel;
		CString strCount;
		strCount.Format("%li", m_nBookingCount);

		// (j.jones 2008-05-01 16:42) - PLID 29874 - Set background color to transparent
		DrawTextOnDialog(this, pdc, m_rcBookingCount, strCount, bSelectionEnabled?dtsHyperlink:dtsDisabledHyperlink, true, DT_LEFT, true, false, 0);
	}

	BOOL bTextSelectionEnabled = GetDlgItem(IDC_BOOKING_CHECK)->IsWindowEnabled();

	// Draw the text: " or more appointments"
	CRect rc(m_rcBookingLabel);
	rc.left = m_rcBookingCount.right;

	// (j.jones 2008-05-01 16:42) - PLID 29874 - Set background color to transparent
	DrawTextOnDialog(this, pdc, rc, " or more appointments", bTextSelectionEnabled?dtsText:dtsDisabledText, true, DT_LEFT, true, false, 0);
}

void CRuleEntryDlg::DrawAndDetails(CDC *pdc)
{
	BOOL bSelectionEnabled = !IsDlgButtonChecked(IDC_ALL_CHECK);

	// Draw the "all" or "any" text
	{
		m_rcAllOrAny = m_rcAndDetailsLabel;
		CString strAllOrAny;
		if (m_bAndDetails) {
			strAllOrAny = "all";
		} else {
			strAllOrAny = "any";		
		}

		// (j.jones 2008-05-01 16:42) - PLID 29874 - Set background color to transparent
		DrawTextOnDialog(this, pdc, m_rcAllOrAny, strAllOrAny, bSelectionEnabled?dtsHyperlink:dtsDisabledHyperlink, true, DT_LEFT, true, false, 0);
	}

	// Draw the text: " of the selected requirements"
	CRect rc(m_rcAndDetailsLabel);
	rc.left = m_rcAllOrAny.right;

	// (j.jones 2008-05-01 16:42) - PLID 29874 - Set background color to transparent
	// (j.jones 2015-01-15 09:35) - PLID 64429 - added a period at the end
	DrawTextOnDialog(this, pdc, rc, " of the selected requirements.", dtsText, true, DT_LEFT, true, false, 0);
}

void CRuleEntryDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	DrawTypeList(&dc);
	DrawPurposeList(&dc);
	DrawBookingLabel(&dc);
	DrawAndDetails(&dc);
}

BOOL CRuleEntryDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	if (IsDlgButtonChecked(IDC_TYPE_CHECK)) {
		if (m_rcTypeListIs.PtInRect(pt) || m_rcTypeListList.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}
	if (IsDlgButtonChecked(IDC_PURPOSE_CHECK)) {
		if (m_rcPurposeListIs.PtInRect(pt) || m_rcPurposeListList.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}
	if (IsDlgButtonChecked(IDC_BOOKING_CHECK)) {
		if (m_rcBookingCount.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}
	if (m_rcAllOrAny.PtInRect(pt) && !IsDlgButtonChecked(IDC_ALL_CHECK)) {
		SetCursor(GetLinkCursor());
		return TRUE;
	}

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

int CRuleEntryDlg::ZoomRule(CTemplateRuleInfo *pRuleInfo)
{
	m_pRuleInfo = pRuleInfo;
	return CNxDialog::DoModal();
}

BOOL CRuleEntryDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/29/2008) - PLID 29814 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	// Calculate hyperlink rectangles
	{
		CWnd *pWnd;

		pWnd = GetDlgItem(IDC_LISTOF_TYPES);
		if (pWnd->GetSafeHwnd()) {
			// Get the position of the is hotlinks
			pWnd->GetWindowRect(m_rcTypeList);
			ScreenToClient(&m_rcTypeList);

			// Hide the static text that was there
			pWnd->ShowWindow(SW_HIDE);
		}
		
		pWnd = GetDlgItem(IDC_LISTOF_PURPOSES);
		if (pWnd->GetSafeHwnd()) {
			// Get the position of the is hotlinks
			pWnd->GetWindowRect(m_rcPurposeList);
			ScreenToClient(&m_rcPurposeList);

			// Hide the static text that was there
			pWnd->ShowWindow(SW_HIDE);
		}
		
		pWnd = GetDlgItem(IDC_LISTOF_BOOKINGS);
		if (pWnd->GetSafeHwnd()) {
			// Get the position of the booking max hotlink
			pWnd->GetWindowRect(m_rcBookingLabel);
			ScreenToClient(&m_rcBookingLabel);

			// Hide the static text that was there
			pWnd->ShowWindow(SW_HIDE);
		}
	
		pWnd = GetDlgItem(IDC_LISTOF_ANDDETAILS);
		if (pWnd->GetSafeHwnd()) {
			// Get the position of the all/any hotlink
			pWnd->GetWindowRect(m_rcAndDetailsLabel);
			ScreenToClient(&m_rcAndDetailsLabel);

			// Hide the static text that was there
			pWnd->ShowWindow(SW_HIDE);
		}
	}

	LoadRule();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRuleEntryDlg::DoClickHyperlink(UINT nFlags, CPoint point)
{
	if (IsDlgButtonChecked(IDC_TYPE_CHECK)) {
		if (m_rcTypeListIs.PtInRect(point)) {
			if (m_strTypeListIs == TEXT_IS) {
				m_strTypeListIs = TEXT_ISNOT;
			} else {
				m_strTypeListIs = TEXT_IS;
			}
			InvalidateRect(m_rcTypeList);
		} else if (m_rcTypeListList.PtInRect(point)) {
			// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
			CMultiSelectDlg dlg(this, "AptTypeT");
			dlg.PreSelect(m_aryTypeListIDs);

			dlg.m_strNameColTitle = "Appointment Types";

			CString strWhereClause = "Inactive = 0";
			//We want to include any types that are already part of this rule.
			if(m_aryTypeListIDs.GetSize() > 0) {
				strWhereClause += " OR ID IN (";
				CString strPart;
				for(int i = 0; i < m_aryTypeListIDs.GetSize(); i++) {
					strPart.Format("%li, ", m_aryTypeListIDs.GetAt(i));
					strWhereClause += strPart;
				}
				strWhereClause = strWhereClause.Left(strWhereClause.GetLength()-2);
				strWhereClause += ")";
			}

			if (IDOK == dlg.Open("AptTypeT", strWhereClause, "ID", "Name", "Please select one or more appointment types"))
			{
				// Fill m_aryTypeListIDs with all the selected ID's
				dlg.FillArrayWithIDs(m_aryTypeListIDs);

				// Build the array of selected names for display
				m_strTypeListNames = dlg.GetMultiSelectString();

				InvalidateRect(m_rcTypeList);
			}
		}	
	}
	if (IsDlgButtonChecked(IDC_PURPOSE_CHECK)) {
		if (m_rcPurposeListIs.PtInRect(point)) {
			if (m_strPurposeListIs == TEXT_IS) {
				m_strPurposeListIs = TEXT_ISNOT;
			} else {
				m_strPurposeListIs = TEXT_IS;
			}
			InvalidateRect(m_rcPurposeList);
		} else if (m_rcPurposeListList.PtInRect(point)) {

			// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
			CMultiSelectDlg dlg(this, "AptPurposeT");
			dlg.PreSelect(m_aryPurposeListIDs);

			dlg.m_strNameColTitle = "Appointment Purposes";

			// (c.haag 2008-12-17 17:05) - PLID 32376 - Filter out inactive procedures unless they were
			// tied with the rule before they were inactivated. Also, it should read "purposes", not "types".
			if (IDOK == dlg.Open("AptPurposeT", 
				FormatString("ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) OR ID IN (%s)", ArrayAsString(m_aryPurposeListIDs)),
				"ID", "Name", "Please select one or more appointment purposes"))
			{
				// Fill m_aryTypeListIDs with all the selected ID's
				dlg.FillArrayWithIDs(m_aryPurposeListIDs);

				// Build the array of selected names for display
				m_strPurposeListNames = dlg.GetMultiSelectString();

				InvalidateRect(m_rcPurposeList);
			}
		}	
	}
	if (IsDlgButtonChecked(IDC_BOOKING_CHECK)) {
		if (m_rcBookingCount.PtInRect(point)) {
			CString strCount;
			strCount.Format("%li", m_nBookingCount);
			if (InputBox(this, "Maximum number of bookings allowed", strCount, "", false, false, NULL, TRUE) == IDOK) {
				m_nBookingCount = atol(strCount);
				InvalidateRect(m_rcBookingLabel);
			}
		}	
	}
	if (m_rcAndDetailsLabel.PtInRect(point) && !IsDlgButtonChecked(IDC_ALL_CHECK)) {
		m_bAndDetails = !m_bAndDetails;
		InvalidateRect(m_rcAndDetailsLabel);
	}	
}

void CRuleEntryDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	DoClickHyperlink(nFlags, point);
	CNxDialog::OnLButtonDown(nFlags, point);
}

void CRuleEntryDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	DoClickHyperlink(nFlags, point);
	CNxDialog::OnLButtonDblClk(nFlags, point);
}

void CRuleEntryDlg::OnTypeCheck() 
{
	InvalidateRect(m_rcTypeList);
}

void CRuleEntryDlg::OnPurposeCheck() 
{
	InvalidateRect(m_rcPurposeList);
}

void CRuleEntryDlg::OnBookingCheck() 
{
	InvalidateRect(m_rcBookingLabel);
}

void CRuleEntryDlg::OnWarnCheck() 
{
	CWnd *pWnd = GetDlgItem(IDC_WARNING_EDIT);
	if (pWnd) {
		if (IsDlgButtonChecked(IDC_WARN_CHECK)) {
			pWnd->EnableWindow(TRUE);
		} else {
			pWnd->EnableWindow(FALSE);
		}
	}
}

void CRuleEntryDlg::OnOK() 
{
	if (ValidateRule()) {
		SaveRule();
		CNxDialog::OnOK();
	}
}

void CRuleEntryDlg::SaveTypeList()
{
	if (m_pRuleInfo) {
		m_pRuleInfo->m_aryTypeList.RemoveAll();
		if (IsDlgButtonChecked(IDC_TYPE_CHECK)) {
			if (m_strTypeListIs == TEXT_IS) {
				m_pRuleInfo->m_nTypeListObjectType = 1;
			} else {
				m_pRuleInfo->m_nTypeListObjectType = 101;
			}
			long nSize = m_aryTypeListIDs.GetSize();
			for (long i=0; i<nSize; i++) {
				m_pRuleInfo->m_aryTypeList.Add(m_aryTypeListIDs[i]);
			}
		} else {
			m_pRuleInfo->m_nTypeListObjectType = 0;
		}
	}
}

void CRuleEntryDlg::SavePurposeList()
{
	if (m_pRuleInfo) {
		m_pRuleInfo->m_aryPurposeList.RemoveAll();
		if (IsDlgButtonChecked(IDC_PURPOSE_CHECK)) {
			if (m_strPurposeListIs == TEXT_IS) {
				m_pRuleInfo->m_nPurposeListObjectType = 2;
			} else {
				m_pRuleInfo->m_nPurposeListObjectType = 102;
			}
			long nSize = m_aryPurposeListIDs.GetSize();
			for (long i=0; i<nSize; i++) {
				m_pRuleInfo->m_aryPurposeList.Add(m_aryPurposeListIDs[i]);
			}
		} else {
			m_pRuleInfo->m_nPurposeListObjectType = 0;
		}
	}
}

BOOL CRuleEntryDlg::ValidateRule()
{
	if (GetSafeHwnd() && m_pRuleInfo) {
		// General Info
		{
			CString strDesc;
			GetDlgItemText(IDC_DESCRIPTION_EDIT, strDesc);
			strDesc.TrimLeft(); strDesc.TrimRight();
			if (strDesc.IsEmpty()) {
				MsgBox(MB_ICONINFORMATION|MB_OK, "Please enter a short description for this rule.");
				return FALSE;
			}
			else if(strDesc.GetLength() > 255) {
				MsgBox(MB_ICONINFORMATION|MB_OK, "Your description is too long. Please enter a description with less than 256 characters.");
				return FALSE;
			}
		}

		// Detail info
		{
			BOOL bIsSomethingChecked = FALSE;

			if (IsDlgButtonChecked(IDC_ALL_CHECK)) {
				bIsSomethingChecked = TRUE;
			}

			if (IsDlgButtonChecked(IDC_TYPE_CHECK)) {
				bIsSomethingChecked = TRUE;
				if (m_aryTypeListIDs.GetSize() == 0) {
					MsgBox(MB_ICONINFORMATION|MB_OK, "Please select at least one type.");
					return FALSE;
				}
			}
			
			if (IsDlgButtonChecked(IDC_PURPOSE_CHECK)) {
				bIsSomethingChecked = TRUE;
				if (m_aryPurposeListIDs.GetSize() == 0) {
					MsgBox(MB_ICONINFORMATION|MB_OK, "Please select at least one purpose.");
					return FALSE;
				}
			}
			
			if (IsDlgButtonChecked(IDC_BOOKING_CHECK)) {
				bIsSomethingChecked = TRUE;
				if (m_nBookingCount <= 0) {
					MsgBox(MB_ICONINFORMATION|MB_OK, "Please select a booking count of at least 1.");
					return FALSE;
				}
			}

			if (!bIsSomethingChecked) {
				int nResult = MsgBox(MB_ICONQUESTION|MB_YESNO, 
					"You have not entered any criteria to specify the kinds of appointments that "
					"this rule applies to.  If you proceed, the rule will be rendered useless.  "
					"Would you like to save it anyway?");
				if (nResult != IDYES) {
					return FALSE;
				}
			}
		}

		// Action info
		{
			if (IsDlgButtonChecked(IDC_PREVENT_CHECK) == 0 && IsDlgButtonChecked(IDC_WARN_CHECK) == 0) {
				int nResult = MsgBox(MB_ICONQUESTION|MB_YESNO, 
					"You have chosen not to give a warning and not to prevent the user from "
					"making appointments.  Doing this will render the rule useless.  "
					"Would you like to save it anyway?");
				if (nResult != IDYES) {
					return FALSE;
				}
			}
		}
		if(IsDlgButtonChecked(IDC_WARN_CHECK) != 0) {
			CString strWarning;
			GetDlgItemText(IDC_WARNING_EDIT, strWarning);
			strWarning.TrimLeft(); strWarning.TrimRight(); 
			if(strWarning.IsEmpty()) {
				MsgBox(MB_ICONINFORMATION|MB_OK, "Your warning message is blank.  Please enter a descriptive warning message.");
				return FALSE;
			}
			else if(strWarning.GetLength() > 255) {
				MsgBox(MB_ICONINFORMATION|MB_OK, "Your warning message is too long.  Please enter a warning message with less than 256 characters.");
				return FALSE;
			}
		}
	} else {
		ASSERT(FALSE); // How could this be?
		MsgBox(MB_ICONEXCLAMATION|MB_OK, 
			"The rule cannot be saved right now because of an internal processing "
			"error.  Please cancel and try to create the rule again from scratch.");
		return FALSE;
	}

	// Validated successfully
	return TRUE;
}

void CRuleEntryDlg::SaveRule() 
{
	if (GetSafeHwnd() && m_pRuleInfo) {
		// General Info
		{
			GetDlgItemText(IDC_DESCRIPTION_EDIT, m_pRuleInfo->m_strDescription);
			
			m_pRuleInfo->m_bAndDetails = m_bAndDetails;
		}

		// Detail info
		{
			SaveTypeList();
			
			SavePurposeList();

			if (IsDlgButtonChecked(IDC_BOOKING_CHECK)) {
				m_pRuleInfo->m_nBookingCount = m_nBookingCount;
			} else {
				m_pRuleInfo->m_nBookingCount = -1;
			}

			m_pRuleInfo->m_bAllAppts = IsDlgButtonChecked(IDC_ALL_CHECK);
		}

		// Action info
		{
			m_pRuleInfo->m_bPreventOnFail = IsDlgButtonChecked(IDC_PREVENT_CHECK) ? TRUE : FALSE;
			m_pRuleInfo->m_bWarningOnFail = IsDlgButtonChecked(IDC_WARN_CHECK) ? TRUE : FALSE;
			GetDlgItemText(IDC_WARNING_EDIT, m_pRuleInfo->m_strWarningOnFail);
			//TES 8/31/2010 - PLID 39630 - Added Override Location Templating option
			m_pRuleInfo->m_bOverrideLocationTemplating = IsDlgButtonChecked(IDC_OVERRIDE_LOCATION_TEMPLATING);
		}
	}
}

// This function is IDENTICAL to the LoadPurposeList function, except that
// EVERYWHERE you see "purpose" this function has "type" (with appropriate case)
// and the object types are 1 and 101 instead of 2 and 102
void CRuleEntryDlg::LoadTypeList()
{
	if (m_pRuleInfo) {
		// Defaults
		m_strTypeListIs = TEXT_IS;
		m_aryTypeListIDs.RemoveAll();
		m_strTypeListNames = "";

		// Now fill them more accurately
		if (m_pRuleInfo->m_nTypeListObjectType == 0) {
			// Uncheck the TYPE entry
			CheckDlgButton(IDC_TYPE_CHECK, 0);
		} else {
			// Check the TYPE entry
			CheckDlgButton(IDC_TYPE_CHECK, 1);
			// Set the is or is not
			if (m_pRuleInfo->m_nTypeListObjectType == 1) {
				m_strTypeListIs = TEXT_IS;
			} else if (m_pRuleInfo->m_nTypeListObjectType == 101) {
				m_strTypeListIs = TEXT_ISNOT;
			} else {
				ASSERT(FALSE); // We only support 0, 1, and 101 for TYPE object types
			}
			// Loop through the array and generate our own lists
			long nSize = m_pRuleInfo->m_aryTypeList.GetSize();
			for (long i=0; i<nSize; i++) {
				// Add each TYPE detail
				try {
					m_strTypeListNames += 
						AdoFldString(CreateRecordset("SELECT Name FROM AptTypeT WHERE ID = %li", m_pRuleInfo->m_aryTypeList[i])->Fields, "Name") +
						", ";
					m_aryTypeListIDs.Add(m_pRuleInfo->m_aryTypeList[i]);
				} catch (_com_error e) {
					// ignore com errors because the ID might not exist
				}
			}
			if (m_strTypeListNames.Right(2) == ", ") {
				m_strTypeListNames.Delete(m_strTypeListNames.GetLength()-2, 2);
			}
		}
	}
}

// This function is IDENTICAL to the LoadTypeList function, except that
// EVERYWHERE you see "type" this function has "purpose" (with appropriate case)
// and the object types are 2 and 102 instead of 1 and 101
void CRuleEntryDlg::LoadPurposeList()
{
	if (m_pRuleInfo) {
		// Defaults
		m_strPurposeListIs = TEXT_IS;
		m_aryPurposeListIDs.RemoveAll();
		m_strPurposeListNames = "";

		// Now fill them more accurately
		if (m_pRuleInfo->m_nPurposeListObjectType == 0) {
			// Uncheck the PURPOSE entry
			CheckDlgButton(IDC_PURPOSE_CHECK, 0);
		} else {
			// Check the PURPOSE entry
			CheckDlgButton(IDC_PURPOSE_CHECK, 1);
			// Set the is or is not
			if (m_pRuleInfo->m_nPurposeListObjectType == 2) {
				m_strPurposeListIs = TEXT_IS;
			} else if (m_pRuleInfo->m_nPurposeListObjectType == 102) {
				m_strPurposeListIs = TEXT_ISNOT;
			} else {
				ASSERT(FALSE); // We only support 0, 1, and 101 for PURPOSE object Purposes
				m_strPurposeListIs = TEXT_IS;
			}
			// Loop through the array and generate our own lists
			long nSize = m_pRuleInfo->m_aryPurposeList.GetSize();
			for (long i=0; i<nSize; i++) {
				// Add each PURPOSE detail
				try {
					m_strPurposeListNames += 
						AdoFldString(CreateRecordset("SELECT Name FROM AptPurposeT WHERE ID = %li", m_pRuleInfo->m_aryPurposeList[i])->Fields, "Name") +
						", ";
					m_aryPurposeListIDs.Add(m_pRuleInfo->m_aryPurposeList[i]);
				} catch (_com_error e) {
					// ignore com errors because the ID might not exist
				}
			}
			if (m_strPurposeListNames.Right(2) == ", ") {
				m_strPurposeListNames.Delete(m_strPurposeListNames.GetLength()-2, 2);
			}
		}
	}
}

void CRuleEntryDlg::LoadRule() 
{
	if (GetSafeHwnd() && m_pRuleInfo) {
		// General rule info
		{
			SetDlgItemText(IDC_DESCRIPTION_EDIT, m_pRuleInfo->m_strDescription);

			m_bAndDetails = m_pRuleInfo->m_bAndDetails;
		}

		// Detail info
		{
			LoadTypeList();
			
			LoadPurposeList();
			
			if (m_pRuleInfo->m_nBookingCount >= 0) {
				CheckDlgButton(IDC_BOOKING_CHECK, 1);
				m_nBookingCount = m_pRuleInfo->m_nBookingCount;
			} else {
				CheckDlgButton(IDC_BOOKING_CHECK, 0);
				m_nBookingCount = 1;
			}

			CheckDlgButton(IDC_ALL_CHECK,m_pRuleInfo->m_bAllAppts);
			OnAllCheck();
		}

		// Action Info
		{
			CheckDlgButton(IDC_WARN_CHECK, m_pRuleInfo->m_bWarningOnFail?1:0);
			CheckDlgButton(IDC_PREVENT_CHECK, m_pRuleInfo->m_bPreventOnFail?1:0);
			SetDlgItemText(IDC_WARNING_EDIT, m_pRuleInfo->m_strWarningOnFail);
			//TES 8/31/2010 - PLID 39630 - Added Override Location Templating option
			CheckDlgButton(IDC_OVERRIDE_LOCATION_TEMPLATING, m_pRuleInfo->m_bOverrideLocationTemplating?1:0);
			CWnd *pWnd = GetDlgItem(IDC_WARNING_EDIT);
			if (pWnd) {
				if (m_pRuleInfo->m_bWarningOnFail) {
					pWnd->EnableWindow(TRUE);
				} else {
					pWnd->EnableWindow(FALSE);
				}
			}
		}

		Invalidate(TRUE);
	}
}

void CRuleEntryDlg::OnAllCheck() 
{
	if(IsDlgButtonChecked(IDC_ALL_CHECK)) {
		CheckDlgButton(IDC_TYPE_CHECK,false);
		CheckDlgButton(IDC_PURPOSE_CHECK,false);
		CheckDlgButton(IDC_BOOKING_CHECK,false);
		GetDlgItem(IDC_TYPE_CHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_LISTOF_TYPES)->EnableWindow(FALSE);
		GetDlgItem(IDC_PURPOSE_CHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_LISTOF_PURPOSES)->EnableWindow(FALSE);
		GetDlgItem(IDC_BOOKING_CHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_LISTOF_BOOKINGS)->EnableWindow(FALSE);
		//TES 8/31/2010 - PLID 39630 - The Override Location Templating option is only used for "of any kind" rules
		GetDlgItem(IDC_OVERRIDE_LOCATION_TEMPLATING)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_TYPE_CHECK)->EnableWindow(TRUE);
		GetDlgItem(IDC_LISTOF_TYPES)->EnableWindow(TRUE);
		GetDlgItem(IDC_PURPOSE_CHECK)->EnableWindow(TRUE);
		GetDlgItem(IDC_LISTOF_PURPOSES)->EnableWindow(TRUE);
		GetDlgItem(IDC_BOOKING_CHECK)->EnableWindow(TRUE);
		GetDlgItem(IDC_LISTOF_BOOKINGS)->EnableWindow(TRUE);
		//TES 8/31/2010 - PLID 39630 - The Override Location Templating option is only used for "of any kind" rules
		GetDlgItem(IDC_OVERRIDE_LOCATION_TEMPLATING)->EnableWindow(FALSE);
	}

	OnTypeCheck();
	OnPurposeCheck();
	OnBookingCheck();
	InvalidateRect(m_rcAndDetailsLabel);
}
