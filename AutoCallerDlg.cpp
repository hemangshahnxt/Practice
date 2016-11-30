// AutoCallerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "AutoCallerDlg.h"
#include "CallerSetupDlg.h"
#include "AutoCaller.h"
#include "GlobalDataUtils.h"
#include "DateTimeUtils.h"
#include "TodoUtils.h"
#include "GlobalTodoUtils.h"
extern CString m_strResponseValue;
extern long m_nPreviousPatient;
#define IDT_CALL_TIMER 200

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CAutoCallerDlg dialog


CAutoCallerDlg::CAutoCallerDlg(CWnd* pParent)
	: CNxDialog(CAutoCallerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAutoCallerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAutoCallerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAutoCallerDlg)
	DDX_Control(pDX, IDC_DOUBLE_LEFT, m_btnRemAll);
	DDX_Control(pDX, IDC_DOUBLE_RIGHT, m_btnSelAll);
	DDX_Control(pDX, IDC_SINGLE_LEFT, m_btnRemOne);
	DDX_Control(pDX, IDC_SINGLE_RIGHT, m_btnSelOne);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAutoCallerDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAutoCallerDlg)
	ON_BN_CLICKED(IDC_DOUBLE_LEFT, OnUnselectAll)
	ON_BN_CLICKED(IDC_DOUBLE_RIGHT, OnSelectAll)
	ON_BN_CLICKED(IDC_SINGLE_LEFT, OnUnselect)
	ON_BN_CLICKED(IDC_SINGLE_RIGHT, OnSelect)
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_SETUP, OnSetup)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAutoCallerDlg message handlers

BOOL CAutoCallerDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnSelOne.AutoSet(NXB_RIGHT);
	m_btnRemOne.AutoSet(NXB_LEFT);
	m_btnSelAll.AutoSet(NXB_RRIGHT);
	m_btnRemAll.AutoSet(NXB_LLEFT);
	
	m_listUnsel = BindNxDataListCtrl(IDC_UNSEL);
	m_listSel = BindNxDataListCtrl(IDC_SEL, false);
	m_listGroup = BindNxDataListCtrl(IDC_GROUP_LIST);
	m_listGroup->CurSel = 0;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAutoCallerDlg::OnUnselectAll() 
{

	IRowSettingsPtr pRow = NULL;

	try
	{	
		for(int i = 0; i < m_listSel->GetRowCount(); i++)
		{
			pRow = m_listSel->GetRow(i);
			m_listUnsel->AddRow(pRow);
		}

		m_listSel->Clear();
		
	}NxCatchAll("Could not move patients");

}

void CAutoCallerDlg::OnSelectAll() 
{
	IRowSettingsPtr pRow = NULL;

	try
	{	
		for(int i = 0; i < m_listUnsel->GetRowCount(); i++)
		{
			pRow = m_listUnsel->GetRow(i);
			m_listSel->AddRow(pRow);
		}

		m_listUnsel->Clear();
		
	}NxCatchAll("Could not move patients");

}

void CAutoCallerDlg::OnUnselect() 
{

	IRowSettingsPtr pRow = NULL;

	if(m_listSel->CurSel == -1)
		return;

	try
	{	
		pRow = m_listSel->GetRow(m_listSel->CurSel);
		m_listUnsel->AddRow(pRow);
		m_listSel->RemoveRow(m_listSel->CurSel);
		
	}NxCatchAll("Could not move patient");
	
	
}

void CAutoCallerDlg::OnSelect() 
{
	IRowSettingsPtr pRow = NULL;

	if(m_listUnsel->CurSel == -1)
		return;

	try
	{	
		pRow = m_listUnsel->GetRow(m_listUnsel->CurSel);
		m_listSel->AddRow(pRow);
		m_listUnsel->RemoveRow(m_listUnsel->CurSel);
		
	}NxCatchAll("Could not move patient");

}

void CAutoCallerDlg::OnStart() 
{
	
	if(m_listGroup->GetCurSel() == -1)
	{
		AfxMessageBox("Please select a group to call first");
		return;
	}

	SetTimer(IDT_CALL_TIMER, 0, NULL);

}

void CAutoCallerDlg::OnSetup() 
{
	CCallerSetupDlg dlg(this);
	if(dlg.DoModal() == IDOK)
		m_listGroup->Requery();

	m_listGroup->CurSel = 0;
	
}

void CAutoCallerDlg::OnCancel()
{

	CDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CAutoCallerDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAutoCallerDlg)
	ON_EVENT(CAutoCallerDlg, IDC_SEL, 3 /* DblClickCell */, OnDblClickCellSelected, VTS_I4 VTS_I2)
	ON_EVENT(CAutoCallerDlg, IDC_UNSEL, 3 /* DblClickCell */, OnDblClickCellUnselected, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAutoCallerDlg::OnDblClickCellSelected(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;

	m_listSel->CurSel = nRowIndex;
	OnUnselect();

}

void CAutoCallerDlg::OnDblClickCellUnselected(long nRowIndex, short nColIndex) 
{
	if(nRowIndex == -1)
		return;

	m_listUnsel->CurSel = nRowIndex;
	OnSelect();

}

void CAutoCallerDlg::OnTimer(UINT nIDEvent) 
{

	switch(nIDEvent)
	{

	case IDT_CALL_TIMER:
		{

			KillTimer(nIDEvent);
			CString strPhone;
			long nRowCount = m_listSel->GetRowCount();
			long nGroup = VarLong(m_listGroup->GetValue(m_listGroup->GetCurSel(), 0));

			_RecordsetPtr rs;

			try{
				rs = CreateRecordset("SELECT StartMessage, Message, Rings, User0, User1, User2, User3, User4, User5, User6, User7, User8, User9, LocalCode, OutsideLine, UserID FROM CallSetupT WHERE ID = %li", nGroup);

				//while we're calling the next person, let's add the last one to the To do list
				if(m_nPreviousPatient > 0){
					//add a todo list item
					CString str;
					long nUserID;
					nUserID = AdoFldLong(rs, "UserID");
					COleDateTimeSpan dtSpan(1, 0, 0, 0);
					COleDateTime dt = COleDateTime::GetCurrentTime();
					dt += dtSpan;

					if(m_strResponseValue == "")
						str.Format("Auto-Caller:  Patient did not respond");
					else
						str.Format("Auto-Caller:  Patient responded by pressing a %s", m_strResponseValue);
					
					// (c.haag 2008-06-09 09:33) - PLID 30321 - Use a utility function to create the todo
					TodoCreate(dt, dt, nUserID, str, "", m_nPreviousPatient,
						ttPatientContact, m_nPreviousPatient, GetCurrentLocationID(), ttpMedium);

					//empty it out just in case
					m_strResponseValue = "";
				}

			} NxCatchAll("Error in saving todo item in CAutoCallerDlg::OnTimer()");

			if(nRowCount == 0){	//we're out of patients
				KillTimer(nIDEvent);
				m_nPreviousPatient = -1;	//in case we run it again without restarting practice
				break;
			}
			
			//continue on with the rest
			try{

				CString strLocal;
				CString strOutside;

				//setup the message files
				m_strStartMsg = AdoFldString(rs, "StartMessage");
				m_strMsg = AdoFldString(rs, "Message");
				m_strRings = AdoFldString(rs, "Rings");
				
				strLocal = AdoFldString(rs, "LocalCode");
				strOutside = AdoFldString(rs, "OutsideLine");

				//get the phone number
				strPhone = VarString(m_listSel->GetValue(0, 3));

				//if it's a local number, don't bother dialing the area code
				if(strPhone.GetLength() < 8 || strPhone.Left(3) == strLocal){
					strPhone = strPhone.Right(7);
				}

				//otherwise, add a 1 so we can dial out
				else
					strPhone = "1" + strPhone;

				//add whatever is needed to dial an outside line
				if(strOutside != "")
					strPhone = strOutside + strPhone;

				StartAutoCaller(strPhone);

				//remove the row from the datalist so we don't call them again
				m_nPreviousPatient = VarLong(m_listSel->GetValue(0, 0));
				m_listSel->RemoveRow(0);

				SetTimer(IDT_CALL_TIMER, 45000, NULL);

			} NxCatchAll("Error in OnStart()");
		}
		break;

	}

	CNxDialog::OnTimer(nIDEvent);
}
