// PhaseRecordPickerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "PhaseRecordPickerDlg.h"
#include "InternationalUtils.h"
#include "SharedScheduleUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CPhaseRecordPickerDlg dialog


CPhaseRecordPickerDlg::CPhaseRecordPickerDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPhaseRecordPickerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPhaseRecordPickerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPhaseRecordPickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPhaseRecordPickerDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STATIC_PICKER_TEXT, m_nxstaticPickerText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPhaseRecordPickerDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPhaseRecordPickerDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPhaseRecordPickerDlg message handlers

BOOL CPhaseRecordPickerDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		m_List = BindNxDataListCtrl(this, IDC_LIST_PHASERECORDPICKER, GetRemoteData(), false);
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		if (IsAppointmentAction())
		{
			SetWindowText(m_strProcedureName + " Tracking Auto-Completion for '" + PhaseTracking::GetPhaseActionDescription((PhaseTracking::PhaseAction)m_nAction) + "' Step");

			// Set up for the interface for appointments
			SetDlgItemText(IDC_STATIC_PICKER_TEXT,
				"This patient (" + m_strPersonName + ") has multiple appointments that qualify to complete their active tracking step. "
				"Please choose an appointment to associate with the completion of this step. If you press 'Cancel', "
				"the step will remain incomplete.");

			// (c.haag 2004-05-19 11:59) - Normally we would add datalist columns here, but, since we don't
			// support anything outside appointments, I just did it in the resources.
			
			
			// Now filter on the IDs
			CString strIDs;
			for (long i=0; i < m_arEvents.GetSize(); i++)
			{
				CString str;
				str.Format("%d,", m_arEvents[i].nID);
				strIDs += str;
			}
			strIDs = strIDs.Left( strIDs.GetLength() - 1 );
			m_List->WhereClause = _bstr_t(CString("ID IN (") + strIDs + ")");
		}
		else
		{
			PostMessage(WM_COMMAND, IDCANCEL);
			return TRUE;
		}
		m_List->Requery();
	}
	NxCatchAll("Error loading CPhaseRecordPickerDlg");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CPhaseRecordPickerDlg::Open(const CArray<PhaseTracking::TrackingEvent,PhaseTracking::TrackingEvent&>& arEvents,
								const CString& strProcedureName,
								long nAction, const CString& strPersonName)
{
	// (a.walling 2007-11-07 09:26) - PLID 27998 - VS2008 - Use Append
	m_arEvents.Append(arEvents);

	m_strProcedureName = strProcedureName;
	m_nAction = nAction;
	m_SelectedEvent = m_arEvents[0];
	m_strPersonName = strPersonName;
	return DoModal();
}

PhaseTracking::TrackingEvent CPhaseRecordPickerDlg::GetSelectedEvent()
{
	return m_SelectedEvent;
}

void CPhaseRecordPickerDlg::OnOK()
{
	if (m_List->CurSel < 0)
	{
		MsgBox("Please select an item before clicking OK");
		return;
	}

	m_SelectedEvent.nID = VarLong(m_List->GetValue(m_List->CurSel, 0));
	m_SelectedEvent.dtDate = VarDateTime(m_List->GetValue(m_List->CurSel,2));

	CDialog::OnOK();
}

BEGIN_EVENTSINK_MAP(CPhaseRecordPickerDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPhaseRecordPickerDlg)
	ON_EVENT(CPhaseRecordPickerDlg, IDC_LIST_PHASERECORDPICKER, 18 /* RequeryFinished */, OnRequeryFinishedListPhaserecordpicker, VTS_I2)
	ON_EVENT(CPhaseRecordPickerDlg, IDC_LIST_PHASERECORDPICKER, 3 /* DblClickCell */, OnDblClickCellListPhaserecordpicker, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CPhaseRecordPickerDlg::OnRequeryFinishedListPhaserecordpicker(short nFlags) 
{	
	if (IsAppointmentAction())
	{
		// Color appointments based on type, and assign Days
		CString day;
		COleDateTime dt;
		long Start, End;
		Start = 0;
		End = m_List->GetRowCount();

		for(long i = Start; i < End; i++){
			COleVariant var = m_List->GetValue(i,0);
			OLE_COLOR color = m_List->GetValue(i, 1).lVal;
			// (z.manning 2015-04-23 17:19) - NX-100448 - Made a function for this
			color = GetDarkerColorForApptText(color);
			IRowSettingsPtr pRow = m_List->GetRow(i);
			dt = COleDateTime(pRow->GetValue(4));
			day = FormatDateTimeForInterface(dt, "%a");
			pRow->PutForeColor(color);
			pRow->PutValue(3, (_bstr_t)day);
		}

	}
	m_List->CurSel = 0;
}

BOOL CPhaseRecordPickerDlg::IsAppointmentAction()
{
	switch (m_nAction)
	{
		case PhaseTracking::PA_ScheduleAptCategory:
		case PhaseTracking::PA_ScheduleAptType:
		case PhaseTracking::PA_ActualAptCategory:
		case PhaseTracking::PA_ActualAptType:
			return TRUE;
		default: break;
	}
	return FALSE;
}

void CPhaseRecordPickerDlg::OnDblClickCellListPhaserecordpicker(long nRowIndex, short nColIndex) 
{
	m_List->CurSel = nRowIndex;
	OnOK();
}
