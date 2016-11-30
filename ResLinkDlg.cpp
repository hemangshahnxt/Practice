// ResLinkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "schedulerrc.h"
#include "ResLinkDlg.h"
#include "GlobalSchedUtils.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "SharedScheduleUtils.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//columns
enum CRLCol {
	crlApptID = 0, 
	crlPatID, 
	crlColor, 
	crlDate, 
	crlDay, 
	crlStart, 
	crlPurpose,
	crlResource,
	crlConf,
	crlNotes,
	crlStatus,
	crlGroupID,
} CRLCol;

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CResLinkDlg dialog


CResLinkDlg::CResLinkDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CResLinkDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResLinkDlg)
	m_nPatientID = -25;
	m_nResID = -1;
	//}}AFX_DATA_INIT
}


void CResLinkDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResLinkDlg)
	DDX_Control(pDX, IDC_UNSELECT_ALL, m_btnUnselectAll);
	DDX_Control(pDX, IDC_UNSELECT_ONE, m_btnUnselectOne);
	DDX_Control(pDX, IDC_SELECT_ALL_ITEMS, m_btnSelectAll);
	DDX_Control(pDX, IDC_SELECT_ONE_ITEM, m_btnSelectOne);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_BTN_CANCEL_LINK, m_btnCancel);
	DDX_Control(pDX, IDC_GROUPED_GROUPBOX, m_btnGroupedGroupbox);
	DDX_Control(pDX, IDC_NOT_GROUPED_GROUPBOX, m_btnNotGroupedGroupbox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResLinkDlg, CNxDialog)
	//{{AFX_MSG_MAP(CResLinkDlg)
	ON_BN_CLICKED(IDC_BTN_CANCEL_LINK, OnBtnCancel)
	ON_BN_CLICKED(IDC_SELECT_ONE_ITEM, OnSelectOne)
	ON_BN_CLICKED(IDC_SELECT_ALL_ITEMS, OnSelectAll)
	ON_BN_CLICKED(IDC_UNSELECT_ONE, OnUnselectOne)
	ON_BN_CLICKED(IDC_UNSELECT_ALL, OnUnselectAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResLinkDlg message handlers

BOOL CResLinkDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (c.haag 2008-04-24 13:58) - PLID 29776 - NxIconify the buttons
	m_btnOK.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	//setup our datalists
	m_listGrouped = BindNxDataListCtrl(this, IDC_APPOINTMENTS_GROUPED, GetRemoteData(), false);
	m_listUngrouped = BindNxDataListCtrl(this, IDC_APPOINTMENTS_UNGROUPED, GetRemoteData(), false);

	//set the icons on the NxIconButtons
	m_btnSelectOne.SetIcon(IDI_UARROW);
	m_btnSelectAll.SetIcon(IDI_UUARROW);
	m_btnUnselectOne.SetIcon(IDI_DARROW);
	m_btnUnselectAll.SetIcon(IDI_DDARROW);

	//load the appointment lists
	Load();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CResLinkDlg::Load()
{
	CString strWhere, strFrom;

	// (d.moore 2007-07-25) - PLID 4013 - Altered the query to switch MoveUp field over to the WaitingListT table.
	strFrom  = 
			"(SELECT     ResBasicQ.*, dbo.PersonT.Last + ', ' + dbo.PersonT.First + ' ' + dbo.PersonT.Middle AS PatientName, dbo.GetResourceString(ResBasicQ.ID) AS Item,  "
			"CASE WHEN dbo.GetPurposeString(ResBasicQ.ID) <> '' THEN AptTypeT.Name + ' - ' + dbo.GetPurposeString(ResBasicQ.ID) ELSE AptTypeT.Name END AS Purpose, AptTypeT.Color, 0 AS SetID, '' AS Day  "
			"FROM         AptTypeT RIGHT OUTER JOIN  "
			"                      PersonT RIGHT OUTER JOIN  "
			"                        (SELECT     AppointmentsT.ID, AppointmentsT.PatientID, AppointmentsT.AptTypeID, CONVERT(datetime, CONVERT(varchar, StartTime, 1)) AS Date,  "
			"                                 StartTime, AppointmentsT.Confirmed, AppointmentsT.Notes, "
			"                                 CONVERT(bit, CASE WHEN (WaitingListT.ID > 0) THEN 1 ELSE 0 END) AS Moveup, "
			"                                 AppointmentsT.RecordID, AppointmentsT.Status, PersonT.Archived, AppointmentsT.ShowState, AppointmentsT.CreatedDate,  "
			"                                 AppointmentsT.CreatedLogin, AptLinkT.GroupID "
			"                           FROM  AppointmentsT LEFT JOIN  "
			"                                 PersonT ON AppointmentsT.PatientID = PersonT.ID "
			"								  LEFT JOIN AptLinkT ON AppointmentsT.ID = AptLinkT.AppointmentID "
			"                                 LEFT JOIN WaitingListT ON AppointmentsT.ID = WaitingListT.AppointmentID "
			"							WHERE AppointmentsT.Status <> 4 "
			"						 ) ResBasicQ ON dbo.PersonT.ID = ResBasicQ.PatientID ON  "
			"                      AptTypeT.ID = ResBasicQ.AptTypeID) AS ResComplexQ  ";

	//filter on only appts for this patient that are not grouped with us
	strWhere.Format("PatientID = %li AND ID <> %li AND (GroupID IS NULL OR GroupID NOT IN (SELECT GroupID FROM AptLinkT WHERE AppointmentID = %li))", m_nPatientID, m_nResID, m_nResID);
	m_listUngrouped->FromClause = (LPCTSTR)strFrom;
	m_listUngrouped->WhereClause = (LPCTSTR)strWhere;
	m_listUngrouped->Requery();

	//now requery the datalist for the grouped appts
	strWhere.Format("PatientID = %li AND ID <> %li AND GroupID = (SELECT GroupID FROM AptLinkT WHERE AppointmentID = %li)", m_nPatientID, m_nResID, m_nResID);
	m_listGrouped->FromClause = (LPCTSTR)strFrom;
	m_listGrouped->WhereClause = (LPCTSTR)strWhere;
	m_listGrouped->Requery();
}

void CResLinkDlg::SetAppointmentColors(bool bGroup)
{
	long Start, End;

	_DNxDataListPtr pTemp;

	//figure out which one we are refreshing and assign the ptr to it
	if(bGroup)
		pTemp = m_listGrouped;
	else
		pTemp = m_listUngrouped;

	//set the colors on the ungrouped
	Start = 0;
	End = pTemp->GetRowCount();

	for(long i = Start; i < End; i++){
		OLE_COLOR color = VarLong(pTemp->GetValue(i, crlColor), RGB(0, 0, 0));
		// (z.manning 2015-04-23 17:19) - NX-100448 - Made a function for this
		color = GetDarkerColorForApptText(color);
		IRowSettingsPtr pRow = pTemp->GetRow(i);
		pRow->PutForeColor(color);
	}
}

void CResLinkDlg::SetDays(bool bGroup)
{
	COleDateTime dt;
	CString day;

	_DNxDataListPtr pTemp;

	//figure out which one we are refreshing and assign the ptr to it
	if(bGroup)
		pTemp = m_listGrouped;
	else
		pTemp = m_listUngrouped;

	//loop through the ungrouped items
	for(int i = 0; i < pTemp->GetRowCount(); i++)
	{
		IRowSettingsPtr pRow = pTemp->GetRow(i);
		dt = COleDateTime(pRow->GetValue(crlDate));
		day = FormatDateTimeForInterface(dt, "%a");
		pRow->PutValue(crlDay, (_bstr_t)day.Left(3));
	}
}

BEGIN_EVENTSINK_MAP(CResLinkDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CResLinkDlg)
	ON_EVENT(CResLinkDlg, IDC_APPOINTMENTS_GROUPED, 18 /* RequeryFinished */, OnRequeryFinishedGroupedList, VTS_I2)
	ON_EVENT(CResLinkDlg, IDC_APPOINTMENTS_UNGROUPED, 18 /* RequeryFinished */, OnRequeryFinishedUngroupedList, VTS_I2)
	ON_EVENT(CResLinkDlg, IDC_APPOINTMENTS_GROUPED, 3 /* DblClickCell */, OnDblClickGrouped, VTS_I4 VTS_I2)
	ON_EVENT(CResLinkDlg, IDC_APPOINTMENTS_UNGROUPED, 3 /* DblClickCell */, OnDblClickUngrouped, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

int CResLinkDlg::Open(long nResID, long nPatientID)
{
	m_nResID = nResID;
	m_nPatientID = nPatientID;
	return DoModal();
}

void CResLinkDlg::OnBtnCancel() {
	CNxDialog::OnCancel();
}

void CResLinkDlg::OnOK() {

	//we need to loop through all the items that have changed and update them accordingly
	POSITION pos = m_mapChanged.GetStartPosition();
	long nResID = -1;
	bool bGrouped = false;

	while(pos) {
		m_mapChanged.GetNextAssoc(pos, nResID, bGrouped);

		if(bGrouped) {
			//this was added to the map
			AppointmentLink(m_nResID, nResID, true);
		}
		else {
			//set to removal
			try {
				ExecuteSql("DELETE FROM AptLinkT WHERE AppointmentID = %li", nResID);
			} NxCatchAll("Error removing appointment link.");
		}
	}

	// CAH 6/27/03 - Delete all the records in AptLinkT where the instance of the group occurs only
	// once.
	try {
		ExecuteSql("DELETE FROM AptLinkT WHERE GroupID IN (SELECT GroupID FROM AptLinkT GROUP BY GroupID HAVING (COUNT(GroupID) = 1))");
	} NxCatchAll("Error removing appointment link.");

	CNxDialog::OnOK();
}

void CResLinkDlg::OnSelectOne() {

	try {
		//move 1 item from the ungrouped to the grouped list
		long nUngroupSel = -1, nGroupToCheck = -1, nResID;

		nUngroupSel = m_listUngrouped->GetCurSel();

		if(nUngroupSel == -1)
			return;

		nGroupToCheck = VarLong(m_listUngrouped->GetValue(nUngroupSel, crlGroupID), -1);
		nResID = VarLong(m_listUngrouped->GetValue(nUngroupSel, crlApptID), -1);

		if(nGroupToCheck > -1) {
			//this item is in a group, and a group cannot be of size 1, so let them know
			//something else will be moving with it - and give them the chance to cancel
			if(AfxMessageBox("The selected item is part of an existing group.  If you choose to move this appointment, all members of its group "
				"will be moved to the new group.  Are you sure you wish to do this?", MB_YESNO) == IDNO)
				return;
		}

		m_listUngrouped->PutValue(nUngroupSel, crlGroupID, long(-1));
		m_listGrouped->TakeCurrentRow(m_listUngrouped);

		if(nGroupToCheck == -1) {
			//this item is not in a group, we're all set
			m_mapChanged.SetAt(nResID, true);
		}
		else {
			//we are moving this item to the grouped list, but first see if any other items
			//are tied to it
			for(int i = 0; i < m_listUngrouped->GetRowCount(); i++) {
				long nGroup = VarLong(m_listUngrouped->GetValue(i, crlGroupID), -1);

				if(nGroup == nGroupToCheck) {
					//we need to move this item as well
					nResID = VarLong(m_listUngrouped->GetValue(i, crlApptID), -1);
					m_mapChanged.SetAt(nResID, true);
					m_listUngrouped->PutValue(i, crlGroupID, long(-1));	//remove the ID to avoid future issues
					IRowSettingsPtr pRow = m_listUngrouped->GetRow(i);
					m_listGrouped->TakeRow(pRow);
				}
			}
		}
	} NxCatchAll("Error linking single appointment.");
}

void CResLinkDlg::OnSelectAll() {
	//move all items from the ungrouped to the grouped list
	try {
		//we don't need to check the groups, because appts can only be grouped w/ this patient, 
		//and we're already moving all of them
		for(int i = 0; i < m_listUngrouped->GetRowCount(); i++) {
			long nResID;

			nResID = VarLong(m_listUngrouped->GetValue(i, crlApptID), -1);

			m_mapChanged.SetAt(nResID, true);
			m_listUngrouped->PutValue(i, crlGroupID, long(-1));
		}

		m_listGrouped->TakeAllRows(m_listUngrouped);

	} NxCatchAll("Error linking all appointments.");
}

void CResLinkDlg::OnUnselectOne() {
	try {
		//move 1 item from grouped to ungrouped
		long nGroupSel = -1, nResID;

		nGroupSel = m_listGrouped->GetCurSel();

		if(nGroupSel == -1)
			return;

		nResID = VarLong(m_listGrouped->GetValue(nGroupSel, crlApptID), -1);

		//we don't have to check for an existing group here
		m_mapChanged.SetAt(nResID, false);
		m_listGrouped->PutValue(nGroupSel, crlGroupID, long(-1));
		m_listUngrouped->TakeCurrentRow(m_listGrouped);
	} NxCatchAll("Error unlinking single appointment.");
}

void CResLinkDlg::OnUnselectAll() {
	//move all items from grouped to ungrouped
	try {

		for(int i = 0; i < m_listGrouped->GetRowCount(); i++) {
			long nResID;

			nResID = VarLong(m_listGrouped->GetValue(i, crlApptID), -1);

			m_mapChanged.SetAt(nResID, false);
			m_listGrouped->PutValue(i, crlGroupID, long(-1));
		}

		m_listUngrouped->TakeAllRows(m_listGrouped);

	} NxCatchAll("Error unlinking all appointments.");
}

void CResLinkDlg::OnRequeryFinishedGroupedList(short nFlags) {

	// Now draw the colors
	SetAppointmentColors(true);
	SetDays(true);

}

void CResLinkDlg::OnRequeryFinishedUngroupedList(short nFlags) {

	// Now draw the colors
	SetAppointmentColors(false);
	SetDays(false);

}

void CResLinkDlg::OnDblClickGrouped(long nRowIndex, short nColIndex) {

	if (nRowIndex != -1)
	{
		m_listGrouped->CurSel = nRowIndex;
		OnUnselectOne();
	}
}

void CResLinkDlg::OnDblClickUngrouped(long nRowIndex, short nColIndex) {

	if (nRowIndex != -1)
	{
		m_listUngrouped->CurSel = nRowIndex;
		OnSelectOne();
	}
}
