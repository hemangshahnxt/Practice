
// EventSelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsRc.h"
#include "EventSelectDlg.h"
#include "GlobalDataUtils.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEventSelectDlg dialog


CEventSelectDlg::CEventSelectDlg(CWnd* pParent)
	: CNxDialog(CEventSelectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEventSelectDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEventSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEventSelectDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEventSelectDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEventSelectDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// CEventSelectDlg message handlers

BOOL CEventSelectDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 11:01) - PLID 29863 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		
		m_pEventList = BindNxDataListCtrl(IDC_EVENTS, false);
		//Fill in the datalist based on the Phase Action for this step
		CArray<PhaseTracking::TrackingEvent, PhaseTracking::TrackingEvent&> arEvents;
		// (a.walling 2008-03-18 17:26) - PLID 28276 - If manually selecting an event, we should show historical merged template documents
		//TES 4/11/2008 - PLID 29510 - Consolidated the parameters for this function, now we just pass in TRUE for 
		// bManuallyApplied, so that it will use looser criteria to find matching events.
		PhaseTracking::GetMatchingEvents(m_nStepTemplateID, m_nLadderID, m_nPatientID, m_nAction, arEvents, TRUE, TRUE);
		
		if(!arEvents.GetSize()) {
			//No valid events, we can do nothing.
			MessageBox("There are no eligible events recorded for this patient.");
			CDialog::OnCancel();
			return TRUE;
		}

		//If we got here, there are some matching events.
		for(int i = 0; i < arEvents.GetSize(); i++) {
			IRowSettingsPtr pRow = m_pEventList->GetRow(-1);
			pRow->PutValue(0, arEvents[i].nID);
			pRow->PutValue(1, _bstr_t(arEvents[i].strDescription));
			_variant_t varDate;
			varDate.vt = VT_DATE;
			varDate.date = arEvents[i].dtDate;
			pRow->PutValue(2, varDate);
			m_pEventList->AddRow(pRow);
		}
	}
	NxCatchAll("Error in CEventSelectDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEventSelectDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEventSelectDlg)
	ON_EVENT(CEventSelectDlg, IDC_EVENTS, 3 /* DblClickCell */, OnDblClickCellEvents, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


void CEventSelectDlg::OnOK() 
{
	if(m_pEventList->CurSel == -1) {
		MsgBox("Please select an event to apply.");
		return;
	}

	m_nItemID = VarLong(m_pEventList->GetValue(m_pEventList->CurSel, 0));
	m_dtDate = VarDateTime(m_pEventList->GetValue(m_pEventList->CurSel, 2));

	CDialog::OnOK();
}

void CEventSelectDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CEventSelectDlg::OnDblClickCellEvents(long nRowIndex, short nColIndex) 
{
	OnOK();
}
