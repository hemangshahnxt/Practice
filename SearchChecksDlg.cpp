// SearchChecksDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SearchChecksDlg.h"
#include "PaymentDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSearchChecksDlg dialog

//Payment List Columns
enum PaymentListColumns {
	plcID = 0, 
	plcPatID, 
	plcPatName, 
	plcNumber, 
	plcDate,
	plcAmount,
};

//Patient Columns
enum PatientColumns {
	pcID = 0,
	pcPatID,
	pcPatName,
};


CSearchChecksDlg::CSearchChecksDlg(CWnd* pParent)
	: CNxDialog(CSearchChecksDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSearchChecksDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSearchChecksDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSearchChecksDlg)
	DDX_Control(pDX, IDC_SEARCH_CC, m_btnSearchCC);
	DDX_Control(pDX, IDC_SEARCH_CHECKS, m_btnSearchChecks);
	DDX_Control(pDX, IDC_TEXT_FILTER, m_btnCheckFilter);
	DDX_Control(pDX, IDC_PATIENT_FILTER, m_btnFilterPatient);
	DDX_Control(pDX, IDC_TEXT_FILTER_SEARCH, m_nxeditTextFilterSearch);
	DDX_Control(pDX, IDC_CLOSE_BUTTON, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSearchChecksDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSearchChecksDlg)
	ON_BN_CLICKED(IDC_TEXT_FILTER_GO, OnTextFilterGo)
	ON_BN_CLICKED(IDC_SEARCH_CC, OnSearchCc)
	ON_BN_CLICKED(IDC_SEARCH_CHECKS, OnSearchChecks)
	ON_BN_CLICKED(IDC_PATIENT_FILTER, OnPatientFilter)
	ON_BN_CLICKED(IDC_TEXT_FILTER, OnTextFilter)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_CLOSE_BUTTON, OnCloseButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSearchChecksDlg message handlers

BEGIN_EVENTSINK_MAP(CSearchChecksDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSearchChecksDlg)
	ON_EVENT(CSearchChecksDlg, IDC_PATIENT_FILTER_LIST, 16 /* SelChosen */, OnSelChosenPatientFilterList, VTS_I4)
	ON_EVENT(CSearchChecksDlg, IDC_PAY_LIST, 6 /* RButtonDown */, OnRButtonDownPayList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSearchChecksDlg::OnTextFilterGo() 
{
	RequeryPaymentsWithFilters();
}

void CSearchChecksDlg::OnSelChosenPatientFilterList(long nRow) 
{
	//if theyr'e choosing a patient, this is going to be fast, so
	//just requery
	RequeryPaymentsWithFilters();
}

void CSearchChecksDlg::OnSearchCc() 
{
	//enable / disable controls appropriately and requery
	EnsureControls();

	RequeryPaymentsWithFilters();
}

void CSearchChecksDlg::OnSearchChecks() 
{
	//enable / disable controls appropriately and requery
	EnsureControls();

	RequeryPaymentsWithFilters();
}

BOOL CSearchChecksDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-02 09:47) - PLID 29879 - NxIconify the buttons
		m_btnClose.AutoSet(NXB_CLOSE);

		//setup the controls
		m_pPatientList = BindNxDataListCtrl(IDC_PATIENT_FILTER_LIST, true);
		m_pPaymentList = BindNxDataListCtrl(IDC_PAY_LIST, false);
		
		//searching for checks by default
		CheckDlgButton(IDC_SEARCH_CHECKS, TRUE);
		CheckDlgButton(IDC_SEARCH_CC, FALSE);

		//requery the payment list with the given filters
		RequeryPaymentsWithFilters();

		//make sure everything is displayed correctly
		EnsureControls();
	}
	NxCatchAll("Error in CSearchChecksDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSearchChecksDlg::RequeryPaymentsWithFilters()
{
	//ensure some data first
	long nPatSel = m_pPatientList->GetCurSel();
	if(nPatSel == -1 && IsDlgButtonChecked(IDC_PATIENT_FILTER)) {
		MsgBox("Must have a patient selected to continue.");
		GetDlgItem(IDC_PATIENT_FILTER_LIST)->SetFocus();
		return;
	}

	//possible filters:
	//1)  Checks Only
	//	  CC Only
	//2)  Patient filter
	//3)  Specific Check # or CC #

	CString strWhere;

	//one of these must be checked
	if(IsDlgButtonChecked(IDC_SEARCH_CC)) {
		strWhere = "PaymentsT.PayMethod = 3 ";
	}
	else {
		//check
		strWhere = "PaymentsT.PayMethod = 2 ";
	}

	if(IsDlgButtonChecked(IDC_PATIENT_FILTER)) {
		CString str;
		str.Format(" AND LineItemT.PatientID = %li ", VarLong(m_pPatientList->GetValue(nPatSel, pcID)));
		strWhere += str;
	}

	if(IsDlgButtonChecked(IDC_TEXT_FILTER)) {
		CString str, strNumber;
		GetDlgItemText(IDC_TEXT_FILTER_SEARCH, strNumber);
		//allow this to be blank, maybe they want to search for all cc #'s
		//that aren't filled in

		if(IsDlgButtonChecked(IDC_SEARCH_CC)){
			//(e.lally 2007-10-31) PLID 27911 - The CCNumber field is only 4 characters long now, but
			//for future safety let's enforce just the last 4. We will leave the full entered number alone though.
			// (a.walling 2008-04-28 13:32) - PLID 27938 - Escape user input
			str.Format(" AND Right(PaymentPlansT.CCNumber,4) = '%s' ", _Q(strNumber));
		}
		else{
			// (a.walling 2008-04-28 13:32) - PLID 27938 - Escape user input
			str.Format(" AND PaymentPlansT.CheckNo = '%s' ", _Q(strNumber));
		}

		strWhere += str;
	}

	strWhere += " AND LineItemT.Deleted = 0 AND LineItemT.Type = 1 ";

	//we've successfully made it!
	m_pPaymentList->PutWhereClause(_bstr_t(strWhere));
	m_pPaymentList->Requery();
}

void CSearchChecksDlg::EnsureControls()
{
	//this function makes sure the controls have the proper names associated with them, 
	//and are disabled when necessary
	BOOL bEnable;

	/////////////
	// Disabling
	//	- Patient filter
	//	- Specific check/cc #
	//	- Go button

	if(IsDlgButtonChecked(IDC_PATIENT_FILTER))
		bEnable = TRUE;
	else
		bEnable = FALSE;

	m_pPatientList->Enabled = bEnable;

	if(IsDlgButtonChecked(IDC_TEXT_FILTER)) 
		bEnable = TRUE;
	else
		bEnable = FALSE;

	GetDlgItem(IDC_TEXT_FILTER_SEARCH)->EnableWindow(bEnable);
	GetDlgItem(IDC_TEXT_FILTER_GO)->EnableWindow(bEnable);

	//////////
	// Naming
	//	- Dialog title
	//	- Check / cc # column of the payment list
	//	- Button for text search
	//	- Field for the check / cc # column

	if(IsDlgButtonChecked(IDC_SEARCH_CC)) {
		//Credit card is checked, update things appropriately
		SetDlgItemText(IDC_TEXT_FILTER, "Only Credit Ca&rd:");
		CRect rect;
		GetDlgItem(IDC_TEXT_FILTER)->GetWindowRect(&rect);
		ScreenToClient(&rect);
		InvalidateRect(&rect);
		m_pPaymentList->GetColumn(plcNumber)->PutColumnTitle(_bstr_t("Last 4 of CC #"));
		//(e.lally 2007-10-31) PLID 27911 - The CCNumber field is only 4 characters long now, but
		//for future safety let's enforce just the last 4.
		m_pPaymentList->GetColumn(plcNumber)->PutFieldName(_bstr_t("Right(PaymentPlansT.CCNumber,4)"));
		SetWindowText("Search Credit Cards");
	}
	else {
		//check is checked, update things appropriately
		SetDlgItemText(IDC_TEXT_FILTER, "Only C&heck:");
		CRect rect;
		GetDlgItem(IDC_TEXT_FILTER)->GetWindowRect(&rect);
		ScreenToClient(&rect);
		InvalidateRect(&rect);
		m_pPaymentList->GetColumn(plcNumber)->PutColumnTitle(_bstr_t("Check #"));
		m_pPaymentList->GetColumn(plcNumber)->PutFieldName(_bstr_t("PaymentPlansT.CheckNo"));
		SetWindowText("Search Checks");
	}
}

void CSearchChecksDlg::OnPatientFilter() 
{
	EnsureControls();

	if(IsDlgButtonChecked(IDC_PATIENT_FILTER)) {
	}
	else {
		//if they unchecked it, we're no longer filtering on a pt and should requery
		RequeryPaymentsWithFilters();
	}

}

void CSearchChecksDlg::OnTextFilter() 
{
	EnsureControls();

	//put focus in the edit box
	GetDlgItem(IDC_TEXT_FILTER_SEARCH)->SetFocus();

	if(!IsDlgButtonChecked(IDC_TEXT_FILTER)) {
		RequeryPaymentsWithFilters();
	}
}

void CSearchChecksDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	//Is the focus on the payment list?
	CWnd* pPayWnd = GetDlgItem(IDC_PAY_LIST);
	if(GetFocus()->GetSafeHwnd() == pPayWnd->GetSafeHwnd()) {
		CMenu mnu;
		mnu.LoadMenu(IDR_BILLING_POPUP);
		CMenu *pmnuSub = mnu.GetSubMenu(0);	//Search Checks menu

		if(pmnuSub) {
			//see if they right clicked a row
			long nCurSel = m_pPaymentList->GetCurSel();
			if(nCurSel != -1) {
				// Clicked on a row
				try {
					IRowSettingsPtr pRow = m_pPaymentList->GetRow(nCurSel);

					//set default item to go to payment
					pmnuSub->SetDefaultItem(ID_SEARCHCHECKS_GOTOPAYMENT);
				} NxCatchAllCall("CSearchChecksDlg::OnContextMenu", return);
			}
			else {
				//there is no current selection, so don't pop up
				return;
			}

			//now pop up the menu wherever it shall roam
			// Make sure we have an appropriate place to pop up the menu
			if (point.x == -1) {
				CRect rc;
				pWnd->GetWindowRect(&rc);
				GetCursorPos(&point);
				if (!rc.PtInRect(point)) {
					point.x = rc.left+5;
					point.y = rc.top+5;
				}
			}

			// Show the popup
			pmnuSub->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
		}
	}
}

void CSearchChecksDlg::OnCloseButton() 
{
	CDialog::OnOK();
}

void CSearchChecksDlg::OnRButtonDownPayList(long nRow, short nCol, long x, long y, long nFlags) 
{
	((CWnd*)GetDlgItem(IDC_PAY_LIST))->SetFocus();
	//set the selection to whatever they clicked on
	m_pPaymentList->PutCurSel(nRow);
}

BOOL CSearchChecksDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{

	switch(wParam) {
	case ID_SEARCHCHECKS_GOTOPATIENT:
		{
			long nCurSel = m_pPaymentList->GetCurSel();
			if(nCurSel == -1)
				return 0;

			CMainFrame *p = GetMainFrame();
			CNxTabView *pView;

			long nPatID = VarLong(m_pPaymentList->GetValue(nCurSel, plcPatID));

			if (nPatID != GetActivePatientID()) {
				if(!p->m_patToolBar.DoesPatientExistInList(nPatID)) {
					if(IDNO == MessageBox("This patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return 0;
					}
				}
				//TES 1/7/2010 - PLID 36761 - This function may fail now
				if(!p->m_patToolBar.TrySetActivePatientID(nPatID)) {
					return 0;
				}
			}	

			if(p->FlipToModule(PATIENT_MODULE_NAME)) {

				pView = (CNxTabView *)p->GetOpenView(PATIENT_MODULE_NAME);
				if (pView) 
				{	if(pView->GetActiveTab()!=0)
						pView->SetActiveTab(0);
					pView->UpdateView();
				}

				//and close our dialog
				OnCloseButton();
			}
		}
		break;

	case ID_SEARCHCHECKS_GOTOPAYMENT:
		{
			long nCurSel = m_pPaymentList->GetCurSel();
			if(nCurSel == -1)
				return 0;

			//popup the payment dialog
			CPaymentDlg dlg(this);
			dlg.m_PatientID = m_pPaymentList->GetValue(nCurSel, plcPatID);
			dlg.m_boIsNewPayment = FALSE;
			dlg.m_varPaymentID = m_pPaymentList->GetValue(nCurSel, plcID);
			dlg.DoModal(__FUNCTION__, __LINE__);
		}
		break;

	default:
		return CNxDialog::OnCommand(wParam, lParam);
	}

	return CNxDialog::OnCommand(wParam, lParam);
}
