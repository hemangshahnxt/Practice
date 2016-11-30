// ApptsWithoutAllocationsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "inventoryrc.h"
#include "ApptsWithoutAllocationsDlg.h"
#include "ApptsRequiringAllocationsDlg.h"
#include "InvPatientAllocationDlg.h"
#include "InvEditOrderDlg.h"
#include "GlobalReportUtils.h"
#include "Reports.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//TES 6/16/2008 - PLID 30394 - Created
/////////////////////////////////////////////////////////////////////////////
// CApptsWithoutAllocationsDlg dialog


CApptsWithoutAllocationsDlg::CApptsWithoutAllocationsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CApptsWithoutAllocationsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CApptsWithoutAllocationsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CApptsWithoutAllocationsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CApptsWithoutAllocationsDlg)
	DDX_Control(pDX, IDC_PREVIEW_APPTS, m_nxbPreview);
	DDX_Control(pDX, IDC_CREATE_ORDER_WO, m_nxbCreateOrder);
	DDX_Control(pDX, IDC_CONFIGURE_REQUIREMENTS, m_nxbConfigureRequirements);
	DDX_Control(pDX, IDC_CREATE_ALLOCATION_WO, m_nxbCreateAllocation);
	DDX_Control(pDX, IDC_CLOSE_APPTS_WO_ALLOCS, m_nxbClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CApptsWithoutAllocationsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_CLOSE_APPTS_WO_ALLOCS, OnCloseApptsWoAllocs)
	ON_BN_CLICKED(IDC_CONFIGURE_REQUIREMENTS, OnConfigureRequirements)
	ON_BN_CLICKED(IDC_CREATE_ALLOCATION_WO, OnCreateAllocationWo)
	ON_BN_CLICKED(IDC_CREATE_ORDER_WO, OnCreateOrderWo)
	ON_BN_CLICKED(IDC_PREVIEW_APPTS, OnPreviewAppts)
	ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApptsWithoutAllocationsDlg message handlers

BOOL CApptsWithoutAllocationsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		
		//TES 6/16/2008 - PLID 30394 - Set up our NxIconButtons.
		m_nxbClose.AutoSet(NXB_CLOSE);
		m_nxbCreateAllocation.AutoSet(NXB_NEW);	
		m_nxbCreateOrder.AutoSet(NXB_NEW);
		m_nxbConfigureRequirements.AutoSet(NXB_MODIFY);
		m_nxbPreview.AutoSet(NXB_PRINT_PREV);

		//TES 6/16/2008 - PLID 30394 - Load our list.
		m_pApptsList = BindNxDataList2Ctrl(IDC_APPOINTMENTS_LIST, false);
		m_pApptsList->WhereClause = _bstr_t(GetWhereClause());
		m_pApptsList->Requery();

		EnableButtons();
	}NxCatchAll("Error in CApptsWithoutAllocationsDlg::OnInitDialog()");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CApptsWithoutAllocationsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CApptsWithoutAllocationsDlg)
	ON_EVENT(CApptsWithoutAllocationsDlg, IDC_APPOINTMENTS_LIST, 18 /* RequeryFinished */, OnRequeryFinishedAppointmentsList, VTS_I2)
	ON_EVENT(CApptsWithoutAllocationsDlg, IDC_APPOINTMENTS_LIST, 2 /* SelChanged */, OnSelChangedAppointmentsList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CApptsWithoutAllocationsDlg, IDC_APPOINTMENTS_LIST, 7 /* RButtonUp */, OnRButtonUpAppointmentsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

enum AppointmentListColumns {
	alcID = 0,
	alcColor = 1,
	alcPatientName = 2,
	alcStartTime = 3,
	alcTypeName = 4,
	alcPurposeString = 5,
	alcResourceString = 6,
	alcPatientID = 7,
	alcLocationID = 8,
};

using namespace NXDATALIST2Lib;
void CApptsWithoutAllocationsDlg::OnRequeryFinishedAppointmentsList(short nFlags) 
{
	try {
		//TES 6/16/2008 - PLID 30394 - We want to color the rows based on their type, just like the Appointments tab.
		IRowSettingsPtr pRow = m_pApptsList->GetFirstRow();
		while(pRow) {
			pRow->PutForeColor(VarLong(pRow->GetValue(alcColor)));
			pRow = pRow->GetNextRow();
		}
	}NxCatchAll("Error in CApptsWithoutAllocationsDlg::OnRequeryFinishedAppointmentsList()");
}

void CApptsWithoutAllocationsDlg::OnCloseApptsWoAllocs() 
{
	CNxDialog::OnCancel();
}

void CApptsWithoutAllocationsDlg::OnConfigureRequirements() 
{
	try {
		//TES 6/16/2008 - PLID 30394 - Popup the dialog.
		CApptsRequiringAllocationsDlg dlg(this);
		dlg.DoModal();

		//TES 6/16/2008 - PLID 30394 - This may change which appointments we should be showing.
		m_pApptsList->WhereClause = _bstr_t(GetWhereClause());
		m_pApptsList->Requery();
		EnableButtons();
	}NxCatchAll("Error in CApptsWithoutAllocationsDlg::OnConfigureRequirements()");
}

void CApptsWithoutAllocationsDlg::OnCreateAllocationWo() 
{
	try {
		//TES 6/16/2008 - PLID 30394 - Make sure a row is selected, and that we have permission.
		IRowSettingsPtr pRow = m_pApptsList->CurSel;
		if(pRow == NULL) {
			return;
		}
		if(CheckCurrentUserPermissions(bioInventoryAllocation, sptCreate)) {
			//TES 6/16/2008 - PLID 30394 - Popup a new allocation dialog, with appropriate defaults.
			CInvPatientAllocationDlg dlg(this);
			dlg.m_nID = -1;
			dlg.m_nDefaultPatientID = VarLong(pRow->GetValue(alcPatientID));
			dlg.m_nDefaultAppointmentID = VarLong(pRow->GetValue(alcID));
			dlg.m_nDefaultLocationID = VarLong(pRow->GetValue(alcLocationID));
			dlg.DoModal();

			//TES 6/16/2008 - PLID 30394 - This may have changed which appointments we should be showing.
			m_pApptsList->Requery();
			EnableButtons();
		}
	}NxCatchAll("Error in CApptsWithoutAllocationsDlg::OnCreateAllocationWo()");
}

void CApptsWithoutAllocationsDlg::OnCreateOrderWo() 
{
	try {
		//TES 6/16/2008 - PLID 30394 - Make sure a row is selected, and that we have permission.
		IRowSettingsPtr pRow = m_pApptsList->CurSel;
		if(pRow == NULL) {
			return;
		}
		if(CheckCurrentUserPermissions(bioInvOrder, sptCreate)) {
			//TES 6/16/2008 - PLID 30394 - Popup a new order dialog, with appropriate defaults.
			CInvEditOrderDlg dlg(NULL);
			dlg.DoModal(-1, FALSE, VarLong(pRow->GetValue(alcID)), VarLong(pRow->GetValue(alcLocationID)));

			//TES 6/16/2008 - PLID 30394 - This may have changed which appointments we should be showing.
			m_pApptsList->Requery();
			EnableButtons();
		}
	}NxCatchAll("Error in CApptsWithoutAllocationsDlg::OnCreateOrderWo()");
}

void CApptsWithoutAllocationsDlg::OnSelChangedAppointmentsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		//TES 6/16/2008 - PLID 30394 - Make sure the buttons are disabled if nothing's selected.
		EnableButtons();
	}NxCatchAll("Error in CApptsWithoutAllocationsDlg::OnSelChangedAppointmentsList()");
}

void CApptsWithoutAllocationsDlg::EnableButtons()
{
	IRowSettingsPtr pRow = m_pApptsList->CurSel;
	if(pRow) {
		//TES 6/16/2008 - PLID 30394 - Enable the buttons, if we have permission.
		m_nxbCreateAllocation.EnableWindow(GetCurrentUserPermissions(bioInventoryAllocation) & SPT____C_______ANDPASS);
		m_nxbCreateOrder.EnableWindow(GetCurrentUserPermissions(bioInvOrder) & SPT____C_______ANDPASS);
	}
	else {
		//TES 6/16/2008 - PLID 30394 - No row is selected, so disable the buttons.
		m_nxbCreateAllocation.EnableWindow(FALSE);
		m_nxbCreateOrder.EnableWindow(FALSE);
	}
}

void CApptsWithoutAllocationsDlg::OnRButtonUpAppointmentsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		//TES 6/16/2008 - PLID 30394 - Highlight the row they right-clicked on.
		IRowSettingsPtr pRow(lpRow);
		m_pApptsList->CurSel = pRow;
		EnableButtons();
		
		if(pRow) {
			//TES 6/16/2008 - PLID 30394 - If they right-clicked on a row, build a context menu with their options.
			enum EMenuOptions
			{
				moCreateAllocation = 1,
				moCreateOrder,
			};
			CMenu mnu;
			mnu.CreatePopupMenu();

			DWORD dwEnabled = 0;
			if(GetCurrentUserPermissions(bioInventoryAllocation) & SPT____C_______ANDPASS) {
				dwEnabled = MF_ENABLED;
			}
			else {
				dwEnabled = MF_GRAYED;
			}
			mnu.AppendMenu(dwEnabled, moCreateAllocation, "Create Allocation...");
			
			if(GetCurrentUserPermissions(bioInvOrder) & SPT____C_______ANDPASS) {
				dwEnabled = MF_ENABLED;
			}
			else {
				dwEnabled = MF_GRAYED;
			}
			mnu.AppendMenu(dwEnabled, moCreateOrder, "Create Order...");
		
			CPoint ptClicked(x, y);
			GetDlgItem(IDC_APPOINTMENTS_LIST)->ClientToScreen(&ptClicked);
			int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);

			//TES 6/16/2008 - PLID 30394 - Now, take the appropriate action.
			switch(nResult) {
			case moCreateAllocation:
				OnCreateAllocationWo();
				break;

			case moCreateOrder:
				OnCreateOrderWo();
				break;

			default:
				ASSERT(nResult == 0);
				break;
			}
		}

	}NxCatchAll("Error in CApptsWithoutAllocationsDlg::OnRButtonUpAppointmentsList()");

}

CString CApptsWithoutAllocationsDlg::GetWhereClause()
{
	//TES 6/16/2008 - PLID 30394 - Apply our preference for how far ahead to look.  Keep this in sync with 
	// InvUtils::DoAppointmentsWithoutAllocationsExist()
	// (j.jones 2009-12-21 17:02) - PLID 35169 - we also want to show appointments that have allocations 
	// with any products marked "To Be Ordered", and those with orders that have any products still
	// unreceived
	return FormatString("AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND "
		"AppointmentsT.PatientID > 0 AND "
		"dbo.AsDateNoTime(AppointmentsT.StartTime) >= dbo.AsDateNoTime(getdate()) AND "
		"dbo.AsDateNoTime(AppointmentsT.StartTime) <= dbo.AsDateNoTime(DATEADD(day,%li,getdate())) "
		"AND "
		"("
			//show appointments that have neither an allocation or an order
			"("				
				"AppointmentsT.ID NOT IN (SELECT AppointmentID FROM PatientInvAllocationsT "
					"WHERE AppointmentID Is Not Null AND PatientInvAllocationsT.Status <> %li) "
				"AND AppointmentsT.ID NOT IN (SELECT AppointmentID FROM OrderAppointmentsT "
					"INNER JOIN OrderT ON OrderAppointmentsT.OrderID = OrderT.ID "
					"WHERE OrderT.Deleted = 0) "
			") "
			"OR "
				//show appointments that have an allocation that has any "to be ordered" product
				"AppointmentsT.ID IN (SELECT AppointmentID FROM PatientInvAllocationsT "
					"WHERE AppointmentID Is Not Null AND PatientInvAllocationsT.Status <> %li "
					"AND ID IN (SELECT AllocationID FROM PatientInvAllocationDetailsT WHERE Status = %li)) "
			"OR "
				//show appointments that have an order that has any unreceived product
				"AppointmentsT.ID IN (SELECT AppointmentID FROM OrderAppointmentsT "
					"INNER JOIN OrderT ON OrderAppointmentsT.OrderID = OrderT.ID "
					"INNER JOIN OrderDetailsT ON OrderT.ID = OrderDetailsT.OrderID "
					"WHERE OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 "
					"AND OrderDetailsT.DateReceived Is Null) "
		")", 
		GetRemotePropertyInt("ApptsRequiringAllocationsDays", 14, 0, "<None>", true),
		InvUtils::iasDeleted, InvUtils::iasDeleted, InvUtils::iadsOrder);
}

void CApptsWithoutAllocationsDlg::OnPreviewAppts() 
{
	try {
		//TES 6/18/2008 - PLID 30395 - Close this dialog.
		OnOK();

		//TES 6/18/2008 - PLID 30395 - Run the Appointments Without Allocations report; by design, if it's unfiltered
		// it should always be showing exactly the same thing as this screen.
		CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(628)]);
		
		//TES 6/18/2008 - PLID 30395 - Filter on all dates
		CPtrArray params;
		CRParameterInfo *tmpParam;
		tmpParam = new CRParameterInfo;
		tmpParam->m_Name = "DateTo";
		tmpParam->m_Data = "12/31/5000";
		params.Add((void *)tmpParam);

		tmpParam = new CRParameterInfo;
		tmpParam->m_Name = "DateFrom";
		tmpParam->m_Data = "01/01/1000";
		params.Add((void *)tmpParam);

		RunReport(&infReport, &params, true, this, "Appointments Without Allocations");
		ClearRPIParameterList(&params);

	}NxCatchAll("Error in CApptsWithoutAllocationsDlg::OnPreviewAppts()");
}

int CApptsWithoutAllocationsDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	try {
		//TES 8/4/2008 - PLID 30394 - On Don's computer, he occasionally gets an assertion that this dialog is calling 
		// SetControlPositions() without calling GetControlPositions().  I've never gotten that assertion myself, but this
		// seems like it ought to fix it.
		GetControlPositions();
	}NxCatchAll("Error in CApptsWithoutAllocationsDlg::OnCreate()");
	return 0;
}
