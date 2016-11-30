// ProcInfoCenterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "ProcInfoCenterDlg.h"
#include "globalfinancialutils.h"
#include "GlobalSchedUtils.h"
#include "phasetracking.h"
#include "paymentdlg.h"
#include "SelectPrepayDlg.h"
#include "patientview.h"
#include "resentrydlg.h"
#include "SelectBillDlg.h"
#include "SelectApptDlg.h"
#include "AddProcedureDlg.h"
#include "LetterWriting.h"
#include "mergeengine.h"
#include "EditComboBox.h"
#include "InternationalUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "globalreportutils.h"
#include "PhaseTracking.h"
#include "FinancialApply.h"
#include "PicContainerDlg.h"
#include "NxMessageDef.h"
#include "AuditTrail.h"
#include "CaseHistoryDlg.h"
#include "MedSchedule.h"
#include "MedSchedDetail.h"
#include "CalenderSelectDlg.h"
#include "BillingModuleDlg.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
 
// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace NXTIMELib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
// m_pContainerDlg replaced with Set / GetPicContainer so as to avoid GetParent stuff

#define DRAG_COLOR RGB(0,0,255)
#define SKIP_COLOR RGB(235,235,235)
#define SURGERY_TITLE_ROW	0
#define SURGERY_ROW			1
#define SEPARATOR_ROW		2
#define OTHER_TITLE_ROW		3

// (d.singleton 2012-04-20 14:54) - PLID 40019 - for opening existing or creating new pre op calander sub menu
#define PREOP_CALENDAR_NEW	0x008
#define PREOP_CALENDAR_EXISTING	0x0016

// (j.jones 2008-11-17 11:24) - PLID 31686 - added enum for the appts. list
enum AppointmentListColumns {

	alcID = 0,
	alcType,
	alcStartTime,
	alcLocation,
	alcColor,
	alcCategory,
};

// (j.dinatale 2012-07-10 14:53) - PLID 3073 - bill combo columns
namespace PicBillComboList{
	enum PicBillComboCols {
		BillID = 0,
		Description,
		Amount,
		Date,
		Balance,
	};
};

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CProcInfoCenterDlg dialog


CProcInfoCenterDlg::CProcInfoCenterDlg(CWnd* pParent)
	: CPatientDialog(CProcInfoCenterDlg::IDD, pParent)
{
	m_pFont = NULL;
	m_bLButtonDownHandled = false;
	m_pPicContainer = NULL;
	// (a.walling 2008-05-05 11:11) - PLID 29894 - Surgery appointment ID; -2 = uninit, -1 = none
	m_nSurgApptID = -2;
	// (j.jones 2009-08-06 13:50) - PLID 7397 - tracked active quote amount
	m_cyActiveQuoteAmount = COleCurrency(0,0);
	//{{AFX_DATA_INIT(CProcInfoCenterDlg)
		m_nLastSelQuote = -1;
		m_nLastSelCase = -1;
		m_nColor = 0xFFB9A8;
	//}}AFX_DATA_INIT

	m_varCoSurgeonID.vt = VT_NULL; // (z.manning 2008-11-19 11:22) - PLID 31687
}


void CProcInfoCenterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcInfoCenterDlg)
	DDX_Control(pDX, IDC_PIC_COLOR1, m_nxc1);
	DDX_Control(pDX, IDC_PIC_COLOR2, m_nxc2);
	DDX_Control(pDX, IDC_PIC_COLOR3, m_nxc3);
	DDX_Control(pDX, IDC_PIC_COLOR4, m_nxc4);
	DDX_Control(pDX, IDC_ANESTHESIA, m_nxeditAnesthesia);
	DDX_Control(pDX, IDC_PAT_COORD, m_nxeditPatCoord);
	// (j.dinatale 2012-07-10 16:07) - PLID 3073 - no longer need these
	/*DDX_Control(pDX, IDC_BILL_AMT, m_nxeditBillAmt);
	DDX_Control(pDX, IDC_BILL_DATE_PIC, m_nxeditBillDatePic);
	DDX_Control(pDX, IDC_BILL_BALANCE, m_nxeditBillBalance);*/
	DDX_Control(pDX, IDC_PREPAYS_ENTERED, m_nxeditPrepaysEntered);
	DDX_Control(pDX, IDC_PREPAYS_APPLIED, m_nxeditPrepaysApplied);
	DDX_Control(pDX, IDC_QUOTE_AMT, m_nxstaticQuoteAmt);
	DDX_Control(pDX, IDC_REMAINING_AMT, m_nxstaticRemainingAmt);
	DDX_Control(pDX, IDC_ADD_PROC, m_btnAddProc);
	DDX_Control(pDX, IDC_REMOVE_PROC, m_btnRemoveProc);
	DDX_Control(pDX, IDC_ADD_PROC_DETAIL, m_btnAddProcDetail);
	DDX_Control(pDX, IDC_REMOVE_PROC_DETAIL, m_btnRemoveProcDetail);
	DDX_Control(pDX, IDC_ADD_PREPAY, m_btnAddPrePay);
	DDX_Control(pDX, IDC_ADD_BILL, m_btnAddBill);
	DDX_Control(pDX, IDC_NEW_QUOTE_PIC, m_btnNewQuote);
	DDX_Control(pDX, IDC_VIEW_QUOTE, m_btnViewQuote);
	DDX_Control(pDX, IDC_ANESTHESIA_CONFIG, m_btnAnesthesiaConfig);
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_DEFAULT_QUOTE, m_btnDefaultQuote);
	DDX_Control(pDX, IDC_PROCEDURE_INFORMATION, m_nxstaticProcInfo);
	DDX_Control(pDX, IDC_SURGERY_INFORMATION, m_nxstaticSurgInfo);
	DDX_Control(pDX, IDC_COSURGEON_PROVIDER, m_nxbtnCoSurgeonProvider);
	DDX_Control(pDX, IDC_COSURGEON_REFPHYS, m_nxbtnCoSurgeonRefPhys);
	DDX_Control(pDX, IDC_VIEW_CASE, m_btnViewCase);
	DDX_Control(pDX, IDC_NEW_CASE, m_btnNewCase);
	DDX_Control(pDX, IDC_DEFAULT_CASE_HISTORY, m_btnDefaultCase);
	DDX_Control(pDX, IDC_CASE_HISTORY_LABEL, m_nxstaticCaseHistoryLabel);
	DDX_Control(pDX, IDC_UNAPPLY_QUOTE, m_btnUnapplyQuote);// (a.vengrofski 2010-03-16 13:46) - PLID <34617> - New button
	DDX_Control(pDX, IDC_UNLINK_BILL, m_btnUnapplyBill); // (j.dinatale 2011-10-03 16:13) - PLID 43528 - be able to unapply a bill
	DDX_Control(pDX, IDC_PREOP_SCHED, m_btnPreOpCalendar);// (d.singleton 2012-04-30 14:27) - PLID 
	DDX_Control(pDX, IDC_PIC_MARK_BILL_ACTIVE, m_btnMarkBillActive);	// (j.dinatale 2012-07-10 16:55) - PLID 3073
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcInfoCenterDlg, CPatientDialog)
	ON_BN_CLICKED(IDC_DEFAULT_QUOTE, OnDefaultQuote)
	ON_BN_CLICKED(IDC_VIEW_QUOTE, OnViewQuote)
	ON_BN_CLICKED(IDC_ADD_PREPAY, OnAddPrepay)
	ON_COMMAND(NXM_PREPAY_NEW, OnPrepayNew)
	ON_COMMAND(NXM_PREPAY_EXISTING, OnPrepayExisting)
	ON_BN_CLICKED(IDC_ADD_BILL, OnAddBill)
	ON_COMMAND(NXM_BILL_NEW, OnBillNew)
	ON_COMMAND(NXM_BILL_EXISTING, OnBillExisting)
	ON_COMMAND(NXM_BILL_ACTIVE_QUOTE, OnBillActiveQuote)
	ON_MESSAGE(NXM_POST_EDIT_BILL, OnPostEditBill)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_COMMAND(NXM_APPT_NEW, OnApptNew)
	ON_COMMAND(NXM_APPT_EXISTING, OnApptExisting)
	ON_BN_CLICKED(IDC_NEW_QUOTE_PIC, OnNewQuotePic)
	ON_BN_CLICKED(IDC_ADD_PROC, OnAddProc)
	ON_BN_CLICKED(IDC_REMOVE_PROC, OnRemoveProc)
	ON_BN_CLICKED(IDC_ANESTHESIA_CONFIG, OnAnesthesiaConfig)
	ON_BN_CLICKED(IDC_REMOVE_PROC_DETAIL, OnRemoveProcDetail)
	ON_BN_CLICKED(IDC_ADD_PROC_DETAIL, OnAddProcDetail)
	ON_BN_CLICKED(IDC_PREVIEW_PIC, OnPreviewPic)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_COSURGEON_REFPHYS, OnBnClickedCosurgeonRefphys)
	ON_BN_CLICKED(IDC_COSURGEON_PROVIDER, OnBnClickedCosurgeonProvider)
	ON_BN_CLICKED(IDC_VIEW_CASE, OnViewCase)
	ON_BN_CLICKED(IDC_NEW_CASE, OnNewCase)
	ON_BN_CLICKED(IDC_DEFAULT_CASE_HISTORY, OnDefaultCaseHistory)
	// (a.vengrofski 2010-02-08 09:19) - PLID <34617>
	ON_BN_CLICKED(IDC_UNAPPLY_QUOTE, &CProcInfoCenterDlg::OnBnClickedUnapplyQuote)
	ON_BN_CLICKED(IDC_UNLINK_BILL, OnBnClickedUnapplyBill)
	// (d.singleton 2012-04-04 17:52) - PLID 
	ON_BN_CLICKED(IDC_PREOP_SCHED, OnBnClickedPreOpSched)
	ON_COMMAND(PREOP_CALENDAR_NEW, OnAddNewCalendar)
	ON_COMMAND(PREOP_CALENDAR_EXISTING, OnOpenExistingCalendar)
	ON_BN_CLICKED(IDC_PIC_MARK_BILL_ACTIVE, &CProcInfoCenterDlg::OnBnClickedPicMarkBillActive)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcInfoCenterDlg message handlers


BOOL CProcInfoCenterDlg::OnInitDialog() 
{
	try
	{
		CPatientDialog::OnInitDialog();

		// (z.manning 2008-11-19 10:12) - PLID 31687 - Cache the co-surgeon checkbox properties
		//TES 2/26/2009 - PLID 33168 - Added PIC_CreateLadderOnProcedureAdded
		g_propManager.CachePropertiesInBulk("ProcInfoCenterDlg", propNumber,
			"Name IN ("
			"'PICCoSurgeonIncludeProvider', "
			"'PICCoSurgeonIncludeRefPhys', "
			"'PIC_CreateLadderOnProcedureAdded', "
			"'PICHideOldAppts' "	// (j.jones 2010-07-28 09:29) - PLID 28316
			") "
			"	AND Username IN ('%s', '<None>') "
			, _Q(GetCurrentUserName()));

		// (a.walling 2008-06-11 09:32) - PLID 30351 - Create and set fonts
		// (a.walling 2008-11-18 09:25) - PLID 31956 - We are now cleartype-safe, so use DEFAULT_QUALITY
		m_pFont = new CFont;
		// (a.walling 2016-06-01 11:12) - NX-100195 - use Segoe UI
		m_pFont->CreateFont(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));

		m_nxstaticProcInfo.SetFont(m_pFont, FALSE);
		m_nxstaticSurgInfo.SetFont(m_pFont, FALSE);

		// (z.manning 2008-11-19 10:15) - PLID 31687 - Load the remembered settings for the co-surgeon combo
		m_nxbtnCoSurgeonProvider.SetCheck(GetRemotePropertyInt("PICCoSurgeonIncludeProvider", BST_CHECKED, 0, "<None>", true));
		m_nxbtnCoSurgeonRefPhys.SetCheck(GetRemotePropertyInt("PICCoSurgeonIncludeRefPhys", BST_CHECKED, 0, "<None>", true));

		m_bProcLoaded = false;
		m_pProcNames = BindNxDataListCtrl(IDC_PROC_NAME_LIST, false);
		CString strProcWhere;
		strProcWhere.Format("ProcedureT.MasterProcedureID Is Null AND ProcInfoDetailsT.ProcInfoID = %li", m_nProcInfoID);
		m_pProcNames->WhereClause = _bstr_t(strProcWhere);
		m_pProcNames->Requery();

		m_pProcDetailNames = BindNxDataListCtrl(IDC_PROC_NAME_LIST_DETAIL, false);
		strProcWhere.Format("ProcedureT.MasterProcedureID Is Not Null AND ProcInfoDetailsT.ProcInfoID = %li", m_nProcInfoID);
		m_pProcDetailNames->WhereClause = _bstr_t(strProcWhere);
		m_pProcDetailNames->Requery();

		m_nxtArrivalTime = BindNxTimeCtrl(this, IDC_ARRIVAL_HR);

		m_pSurgeon = BindNxDataListCtrl(IDC_SURGEON);
		m_pCoSurgeon = BindNxDataListCtrl(IDC_COSURGEON, false);
		// (z.manning 2008-11-19 10:13) - PLID 31687 - Now have a separate function to requery the co-surgeon list
		RequeryCoSurgeonCombo();
		m_pNurse = BindNxDataListCtrl(IDC_NURSE);
		m_pAnesthesiologist = BindNxDataListCtrl(IDC_ANESTHESIOLOGIST_LIST);
		m_pAnesthesiaList = BindNxDataListCtrl(IDC_ANESTHESIA_LIST);
		m_pApptList = BindNxDataListCtrl(IDC_OTHER_APPTS, false);
		m_pQuoteList = BindNxDataListCtrl(IDC_QUOTES, false);
		//DRT 2/16/2006 - PLID 18960 - This was never implemented, it's just hidden, so I've removed it
		//m_pMedList = BindNxDataListCtrl(IDC_MED_LIST, false);
		// (j.jones 2009-08-06 14:21) - PLID 7397 - added case history combo
		m_pCaseHistoryCombo = BindNxDataListCtrl(IDC_CASE_HISTORY_PIC, false);
		// (j.dinatale 2012-07-09 13:11) - PLID 3073 - added bill combo
		m_pBillCombo = BindNxDataList2Ctrl(IDC_PIC_BILLS_LIST, false);

		// (j.jones 2009-08-06 14:09) - PLID 7397 - show/hide controls based on the ASC license
		if(IsSurgeryCenter(false)) {
			//hide the quote amount label
			m_nxstaticQuoteAmt.ShowWindow(SW_HIDE);

			//change the label
			m_nxstaticCaseHistoryLabel.SetWindowText("Case History");

			//show the case history controls
			m_btnViewCase.ShowWindow(SW_SHOW);
			m_btnNewCase.ShowWindow(SW_SHOW);
			m_btnDefaultCase.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_CASE_HISTORY_PIC)->ShowWindow(SW_SHOW);
		}
		else {
			//show the quote amount label
			m_nxstaticQuoteAmt.ShowWindow(SW_SHOW);

			//change the label
			m_nxstaticCaseHistoryLabel.SetWindowText("Quote Amount:");			

			//hide the case history controls
			m_btnViewCase.ShowWindow(SW_HIDE);
			m_btnNewCase.ShowWindow(SW_HIDE);
			m_btnDefaultCase.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CASE_HISTORY_PIC)->ShowWindow(SW_HIDE);
		}

		m_nxc1.SetColor(m_nColor);
		m_nxc2.SetColor(m_nColor);
		m_nxc3.SetColor(m_nColor);
		m_nxc4.SetColor(m_nColor);

		// (c.haag 2008-04-22 16:04) - PLID 29751 - NxIconify
		m_btnAddProc.AutoSet(NXB_NEW);
		m_btnRemoveProc.AutoSet(NXB_DELETE);
		m_btnAddProcDetail.AutoSet(NXB_NEW);
		m_btnRemoveProcDetail.AutoSet(NXB_DELETE);
		m_btnAddPrePay.AutoSet(NXB_NEW);
		m_btnAddBill.AutoSet(NXB_NEW);
		m_btnNewQuote.AutoSet(NXB_NEW);
		m_btnViewQuote.AutoSet(NXB_MODIFY);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDefaultQuote.AutoSet(NXB_MODIFY);
		m_btnUnapplyQuote.AutoSet(NXB_MODIFY);// (a.vengrofski 2010-03-16 13:44) - PLID <34617> - New button
		m_btnUnapplyBill.AutoSet(NXB_MODIFY); // (j.dinatale 2011-10-03 16:50) - PLID 43528 - need button to unapply
		m_btnMarkBillActive.AutoSet(NXB_MODIFY); // (j.dinatale 2012-07-10 16:56) - PLID 3073

		// (j.jones 2009-08-06 14:01) - PLID 7397 - added case history to the PIC
		m_btnViewCase.AutoSet(NXB_MODIFY);
		m_btnDefaultCase.AutoSet(NXB_MODIFY);
		m_btnNewCase.AutoSet(NXB_NEW);

		// (d.singleton 2012-04-30 14:28) - PLID 
		m_btnPreOpCalendar.AutoSet(NXB_NEW);
		
		// (c.haag 2003-07-16 16:54) - Add <No nurse> and <No Anesthesiologist>
		COleVariant vNull;
		vNull.vt = VT_NULL;
		IRowSettingsPtr pRow;
		pRow = m_pNurse->Row[-1];
		pRow->Value[0] = vNull;
		pRow->Value[1] = _bstr_t(" { No Nurse } ");
		m_pNurse->InsertRow(pRow, 0);

		pRow = m_pAnesthesiologist->Row[-1];
		pRow->Value[0] = vNull;
		pRow->Value[1] = _bstr_t(" { No Anesthesiologist } ");
		m_pAnesthesiologist->InsertRow(pRow, 0);

	}NxCatchAll("CProcInfoCenterDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CProcInfoCenterDlg, CPatientDialog)
	ON_EVENT(CProcInfoCenterDlg, IDC_PROC_NAME_LIST, 18 /* RequeryFinished */, OnRequeryFinishedProcNameList, VTS_I2)
	ON_EVENT(CProcInfoCenterDlg, IDC_QUOTES, 18 /* RequeryFinished */, OnRequeryFinishedQuotes, VTS_I2)
	ON_EVENT(CProcInfoCenterDlg, IDC_QUOTES, 16 /* SelChosen */, OnSelChosenQuotes, VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_SURGEON, 16 /* SelChosen */, OnSelChosenSurgeon, VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_NURSE, 16 /* SelChosen */, OnSelChosenNurse, VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_ANESTHESIOLOGIST_LIST, 16 /* SelChosen */, OnSelChosenAnesthesiologistList, VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_OTHER_APPTS, 18 /* RequeryFinished */, OnRequeryFinishedOtherAppts, VTS_I2)
	ON_EVENT(CProcInfoCenterDlg, IDC_ARRIVAL_HR, 1 /* KillFocus */, OnKillFocusArrivalHr, VTS_NONE)
	ON_EVENT(CProcInfoCenterDlg, IDC_OTHER_APPTS, 12 /* DragBegin */, OnDragBeginOtherAppts, VTS_PBOOL VTS_I4 VTS_I2 VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_OTHER_APPTS, 13 /* DragOverCell */, OnDragOverCellOtherAppts, VTS_PBOOL VTS_I4 VTS_I2 VTS_I4 VTS_I2 VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_OTHER_APPTS, 14 /* DragEnd */, OnDragEndOtherAppts, VTS_I4 VTS_I2 VTS_I4 VTS_I2 VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_ANESTHESIA_LIST, 16 /* SelChosen */, OnSelChosenAnesthesiaList, VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_PROC_NAME_LIST_DETAIL, 10 /* EditingFinished */, OnEditingFinishedProcNameListDetail, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CProcInfoCenterDlg, IDC_PROC_NAME_LIST, 2 /* SelChanged */, OnSelChangedProcNameList, VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_OTHER_APPTS, 4 /* LButtonDown */, OnLButtonDownOtherAppts, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_OTHER_APPTS, 5 /* LButtonUp */, OnLButtonUpOtherAppts, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_NURSE, 20 /* TrySetSelFinished */, OnTrySetSelFinishedNurse, VTS_I4 VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_ANESTHESIOLOGIST_LIST, 20 /* TrySetSelFinished */, OnTrySetSelFinishedAnesthesiologistList, VTS_I4 VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_SURGEON, 20 /* TrySetSelFinished */, OnTrySetSelFinishedSurgeon, VTS_I4 VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_COSURGEON, 16 /* SelChosen */, OnSelChosenCoSurgeon, VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_COSURGEON, 18, RequeryFinishedCosurgeon, VTS_I2)
	ON_EVENT(CProcInfoCenterDlg, IDC_CASE_HISTORY_PIC, 16, OnSelChosenCaseHistoryPic, VTS_I4)
	ON_EVENT(CProcInfoCenterDlg, IDC_CASE_HISTORY_PIC, 18, OnRequeryFinishedCaseHistoryPic, VTS_I2)
	ON_EVENT(CProcInfoCenterDlg, IDC_PIC_BILLS_LIST, 18, OnRequeryFinishedBillComboPic, VTS_I2)
END_EVENTSINK_MAP()

void CProcInfoCenterDlg::OnRequeryFinishedProcNameList(short nFlags) 
{
	try {
		m_bProcLoaded = true;
		Load(PIC_AREA_ALL);

		m_pProcNames->CurSel = 0;
		LoadProcSpecific();
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::Load(int nArea)
{
	/*****************************************************************************************************
	/*NOTE:  There is some duplicated code here between the PIC_AREA_ALL and the other areas.  However, 
	/*		 I am intentionally keeping these separate, because if we're loading all data, we can improve
	/*		 our efficiency by loading just one recordset.
	/****************************************************************************************************/
	try {
		CWaitCursor cuWait;

		switch (nArea) {
		case PIC_AREA_QUOTE:
		{
			long lPatID = GetPatientID();
			CString strListSQL;
			// (d.thompson 2009-08-18) - PLID 16758 - Added 'Expired' column
			strListSQL.Format("(SELECT PatientBillsQ.ID AS QuoteID, PatientBillsQ.Date AS QuoteDate, PatientBillsQ.Description AS FirstOfNotes, "
				"dbo.GetQuoteTotal(PatientBillsQ.ID, 0) AS Total, "
				"convert(bit, CASE WHEN UseExp = 1 THEN CASE WHEN DATEADD(d, ExpDays, Date) < getdate() then 1 else 0 end ELSE 0 END) AS Expired "
				"FROM (SELECT BillsT.* FROM BillsT WHERE BillsT.PatientID=%li AND BillsT.Deleted=0) AS PatientBillsQ "
				"WHERE PatientBillsQ.EntryType=2) AS Q",lPatID);

			m_pQuoteList->FromClause = _bstr_t(strListSQL);

			CString strQuoteWhere;
			// (j.jones 2010-06-23 13:48) - PLID 39276 - fixed the where clause so the OR code properly fired for always
			// showing the ActiveQuoteID even if the quote no longer has details that match the procedures
			strQuoteWhere.Format("((EXISTS (SELECT ID FROM ChargesT WHERE BillID = Q.QuoteID AND ServiceID IN (SELECT ID FROM ServiceT WHERE ServiceT.ProcedureID IN (SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoID = %li))) OR Q.QuoteID IN (SELECT QuoteID FROM ProcInfoQuotesT WHERE ProcInfoID = %li)) "
				"AND (Q.QuoteID NOT IN (SELECT ID FROM BillsT WHERE Active = 0)) OR Q.QuoteID = (SELECT ActiveQuoteID FROM ProcInfoT WHERE ID = %li))", m_nProcInfoID, m_nProcInfoID, m_nProcInfoID);
			m_pQuoteList->WhereClause = _bstr_t(strQuoteWhere);
			if(m_pQuoteList->CurSel != -1) m_nLastSelQuote = VarLong(m_pQuoteList->GetValue(m_pQuoteList->CurSel, 0), -1);
			else m_nLastSelQuote = -1;
			//The onrequeryfinished will set the quote total.
			m_pQuoteList->Requery();
		}
		break;

		// (j.jones 2009-08-06 14:24) - PLID 7397 - added case history
		case PIC_AREA_CASE:
		{
			if(IsSurgeryCenter(false)) {

				long nPatID = GetPatientID();
				CString strListSQL;
				//show all cases not currently linked to other PIC records,
				//whether they are completed or not, and include the total cost from all line items
				
				//Do NOT show the total cost if they cannot see contact costs, and those fees exist.
				BOOL bCanViewPersonCosts = (GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead);

				strListSQL.Format("(SELECT CaseHistoryT.ID, PersonID, Name, SurgeryDate, CompletedDate, "
					//if the case has a person in it, hide the cost if the user has no permission
					"(CASE WHEN SUM(CASE WHEN ItemType = -3 THEN 1 ELSE 0 END) > 0 AND %li=0 THEN NULL "
					"ELSE "
					"Sum(Round(Convert(money,Cost*Quantity),2)) "
					"END) AS TotalCost "
					"FROM CaseHistoryT "
					"INNER JOIN CaseHistoryDetailsT ON CaseHistoryT.ID = CaseHistoryDetailsT.CaseHistoryID "
					"WHERE PersonID = %li "
					"AND CaseHistoryT.ID NOT IN (SELECT CaseHistoryID FROM ProcInfoT WHERE ID  <> %li AND CaseHistoryID Is Not Null) "
					"GROUP BY CaseHistoryT.ID, PersonID, Name, SurgeryDate, CompletedDate) AS CaseHistoryQ",
					bCanViewPersonCosts ? 1 : 0, nPatID, m_nProcInfoID);

				m_pCaseHistoryCombo->FromClause = _bstr_t(strListSQL);

				if(m_pCaseHistoryCombo->CurSel != -1) {
					m_nLastSelCase = VarLong(m_pCaseHistoryCombo->GetValue(m_pCaseHistoryCombo->CurSel, 0), -1);
				}
				else {
					m_nLastSelCase = -1;
				}
				m_pCaseHistoryCombo->Requery();

				//OnRequeryFinished will set the selection
			}
		}
		break;

		case PIC_AREA_BILL:
		{
			_RecordsetPtr rsProcInfo = CreateRecordset("SELECT BillSubQ.Amount, BillSubQ.Date AS BillDate, CASE WHEN BillSubQ.Amount Is Null THEN 0 ELSE BillSubQ.Amount END - CASE WHEN AppliesSubQ.Amount Is Null THEN 0 ELSE AppliesSubQ.Amount END AS Balance "
				"FROM ProcInfoT "
				"LEFT JOIN "
				"(SELECT BillsT.ID, dbo.GetBillTotal(BillsT.ID) AS Amount, "
				"BillsT.Date, BillsT.Deleted AS Deleted "
				"FROM ((ChargesT INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID) LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE LineItemT.Deleted = 0 GROUP BY BillsT.ID, BillsT.Date, BillsT.Deleted HAVING BillsT.Deleted = 0) BillSubQ "
				"ON ProcInfoT.BillID = BillSubQ.ID "
				"LEFT JOIN (SELECT BillID, Sum(Amount) AS Amount FROM AppliesT INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID GROUP BY ChargesT.BillID) AppliesSubQ ON ProcInfoT.BillID = AppliesSubQ.BillID "
				"WHERE ProcInfoT.ID = %li", m_nProcInfoID);

			if(rsProcInfo->eof) {
				MessageBox("Procedure Information record not found!");
				CDialog::OnOK();
				return;
			}
			FieldsPtr fProcInfo = rsProcInfo->Fields;

			// (j.dinatale 2012-07-10 14:22) - PLID 3073 - bill combo list!
			CString strBillComboSql;
			strBillComboSql.Format(
				"(SELECT BillSubQ.ID AS ID, BillSubQ.Amount AS Amount, BillSubQ.Date AS Date, BillsT.Description AS Description, "
				"CASE "
					"WHEN BillSubQ.Amount Is Null "
						"THEN 0 "
					"ELSE BillSubQ.Amount "
				"END - "
				"CASE "
					"WHEN AppliesSubQ.Amount Is Null "
						"THEN 0 "
					"ELSE AppliesSubQ.Amount "
				"END AS Balance "
				"FROM ProcInfoBillsT "
				"LEFT JOIN ( "
					"SELECT BillsT.ID, dbo.GetBillTotal(BillsT.ID) AS Amount, "
					"BillsT.Date, BillsT.Deleted AS Deleted "
					"FROM ( "
						"( "
							"ChargesT "
							"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
						") "
						"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
						"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
					") "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"WHERE LineItemT.Deleted = 0 GROUP BY BillsT.ID, BillsT.Date, BillsT.Deleted HAVING BillsT.Deleted = 0 "
				") BillSubQ ON ProcInfoBillsT.BillID = BillSubQ.ID "
				"LEFT JOIN ( "
					"SELECT BillID, Sum(Amount) AS Amount "
					"FROM AppliesT "
					"INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
					"GROUP BY ChargesT.BillID "
				") AppliesSubQ ON ProcInfoBillsT.BillID = AppliesSubQ.BillID "
				"LEFT JOIN BillsT ON ProcInfoBillsT.BillID = BillsT.ID "
				"WHERE ProcInfoBillsT.ProcInfoID = %li) SubQ", m_nProcInfoID);
			m_pBillCombo->FromClause = _bstr_t(strBillComboSql);
			m_pBillCombo->Requery();

			// (j.dinatale 2012-07-10 16:05) - PLID 3073 - no longer need to update those textboxes

			//The applied amount may now be different, since the Active Bill has changed.
			_RecordsetPtr rsPrepayApplied = CreateRecordset("SELECT Sum(CASE WHEN AppliesT.Amount Is Null THEN 0 ELSE AppliesT.Amount END) AS AppliedAmount "
				"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
				"WHERE LineItemT.Deleted = 0 AND PaymentsT.Prepayment = 1 AND (AppliesT.DestID IN (SELECT ID FROM ChargesT WHERE BillID = (SELECT BillID FROM ProcInfoT WHERE ID = %li)))",
				m_nProcInfoID);

			CString strPrepaysApplied;
			_variant_t varPrepaysApplied;
			varPrepaysApplied = rsPrepayApplied->Fields->GetItem("AppliedAmount")->Value;
			if(varPrepaysApplied.vt == VT_NULL) strPrepaysApplied = "";
			else strPrepaysApplied.Format("%s", FormatCurrencyForInterface(VarCurrency(varPrepaysApplied)));

			SetDlgItemText(IDC_PREPAYS_APPLIED, strPrepaysApplied);
		}
		break;


		case PIC_AREA_PAY:
		{
			//TES 2/6/2007 - PLID 24377 - Filtered out refunds applied to the prepayments.
			_RecordsetPtr rsPrepayAmount = CreateRecordset("SELECT Sum(LineItemT.Amount) - COALESCE(Sum(AppliedToQ.Amount),0) AS Amount "
				"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"LEFT JOIN (SELECT DestID, Sum(-1*Amount) AS Amount FROM AppliesT GROUP BY DestID) AppliedToQ ON LineItemT.ID = AppliedToQ.DestID "
				"WHERE LineItemT.Deleted = 0 AND PaymentsT.Prepayment = 1 AND (PaymentsT.ID IN (SELECT SourceID FROM AppliesT WHERE DestID IN (SELECT ID FROM ChargesT WHERE BillID = (SELECT BillID FROM ProcInfoT WHERE ID = %li))) "
				"OR PaymentsT.ID IN (SELECT PayID FROM ProcInfoPaymentsT WHERE ProcInfoID = %li))", m_nProcInfoID, m_nProcInfoID);
			
			_RecordsetPtr rsPrepayApplied = CreateRecordset("SELECT Sum(CASE WHEN AppliesT.Amount Is Null THEN 0 ELSE AppliesT.Amount END) AS AppliedAmount "
				"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
				"WHERE LineItemT.Deleted = 0 AND PaymentsT.Prepayment = 1 AND (AppliesT.DestID IN (SELECT ID FROM ChargesT WHERE BillID = (SELECT BillID FROM ProcInfoT WHERE ID = %li)))",
				m_nProcInfoID);

			CString strPrepaysEntered, strPrepaysApplied;
			_variant_t varPrepaysEntered, varPrepaysApplied;
			varPrepaysEntered = rsPrepayAmount->Fields->GetItem("Amount")->Value;
			if(varPrepaysEntered.vt == VT_NULL) strPrepaysEntered = "";
			else strPrepaysEntered.Format("%s", FormatCurrencyForInterface(VarCurrency(varPrepaysEntered)));
			varPrepaysApplied = rsPrepayApplied->Fields->GetItem("AppliedAmount")->Value;
			if(varPrepaysApplied.vt == VT_NULL) strPrepaysApplied = "";
			else strPrepaysApplied.Format("%s", FormatCurrencyForInterface(VarCurrency(varPrepaysApplied)));

			SetDlgItemText(IDC_PREPAYS_ENTERED, strPrepaysEntered);
			SetDlgItemText(IDC_PREPAYS_APPLIED, strPrepaysApplied);

			//Set the remaining amount, if possible.
			COleCurrency cuPrepays = VarCurrency(varPrepaysEntered, COleCurrency(0,0));

			//TES 5/26/2004: Show the remaining amount even if there is no active quote.
			// (j.jones 2009-08-06 13:50) - PLID 7397 - tracked active quote amount in memory
			if(m_cyActiveQuoteAmount.GetStatus() == COleCurrency::valid) {
				SetDlgItemText(IDC_REMAINING_AMT, FormatCurrencyForInterface(m_cyActiveQuoteAmount-cuPrepays, TRUE, TRUE));
				InvalidateDlgItem(IDC_REMAINING_AMT);
			}
		}
		break;


		case PIC_AREA_APPT:
		{
			m_pApptList->Clear();
			//First, put in the Surgery title row.
			IRowSettingsPtr pRow = m_pApptList->GetRow(-1);
			pRow->PutValue(alcType, "----Surgery----");
			m_pApptList->InsertRow(pRow, 0);
			
			//Now, add the actual surgery row
			// (j.jones 2008-11-17 11:25) - PLID 31686 - added LocationName and parameterized
			// (j.jones 2010-07-28 09:47) - PLID 28316 - added PICDate
			// (d.singleton 2012-05-02 11:23) - PLID 40019 added the appt type category for preop calendar purposes
			_RecordsetPtr rsProcInfo = CreateParamRecordset("SELECT AppointmentsT.ID, AptTypeT.Name AS Type, "
				"AptTypeT.Color, AppointmentsT.StartTime, AppointmentsT.ArrivalTime, "
				"LocationsT.Name AS LocationName, AptTypeT.Category, "
				"CASE WHEN LaddersT.FirstInterestDate Is Not Null THEN LaddersT.FirstInterestDate ELSE EMRGroupDateQ.InputDate END AS PICDate "
				"FROM ProcInfoT "
				"LEFT JOIN AppointmentsT ON ProcInfoT.SurgeryApptID = AppointmentsT.ID "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
				"LEFT JOIN LaddersT ON ProcInfoT.ID = LaddersT.ProcInfoID "
				"LEFT JOIN (SELECT PicT.ProcInfoID, EMRGroupsT.InputDate "
				"	FROM PicT INNER JOIN EMRGroupsT ON PicT.EMRGroupID = EMRGroupsT.ID "
				"	WHERE PicT.IsCommitted = 1) AS EMRGroupDateQ ON ProcInfoT.ID = EMRGroupDateQ.ProcInfoID "
				"WHERE ProcInfoT.ID = {INT}", m_nProcInfoID);

			if(rsProcInfo->eof) {
				MessageBox("Procedure Information record not found!");
				CDialog::OnOK();
				return;
			}
			_variant_t varSurgeryTime;
			varSurgeryTime.vt = VT_NULL; //for the moment.
			FieldsPtr fProcInfo = rsProcInfo->Fields;
			pRow = m_pApptList->GetRow(-1);
			_variant_t varID = fProcInfo->GetItem("ID")->Value;
			
			// (a.walling 2008-05-05 11:11) - PLID 29894 - Set the surgery appt id
			m_nSurgApptID = VarLong(varID, -1);

			if(varID.vt == VT_NULL) {
				pRow->PutValue(alcType, _bstr_t("<None>"));
				m_pApptList->InsertRow(pRow, 1);
				m_nxtArrivalTime->Enabled = VARIANT_FALSE; // No surgery then no arrival time
			}
			else {
				pRow->PutValue(alcID, varID);
				CString strType = AdoFldString(fProcInfo, "Type", "<No type>");
				pRow->PutValue(alcType, _bstr_t(strType));
				varSurgeryTime = fProcInfo->GetItem("StartTime")->Value;
				pRow->PutValue(alcStartTime, varSurgeryTime);
				// (j.jones 2008-11-17 11:25) - PLID 31686 - added Location column
				pRow->PutValue(alcLocation, fProcInfo->GetItem("LocationName")->Value);
				pRow->PutValue(alcColor, fProcInfo->GetItem("Color")->Value);
				pRow->PutValue(alcCategory, fProcInfo->GetItem("Category")->Value);
				m_pApptList->InsertRow(pRow, 1);
				m_nxtArrivalTime->SetDateTime(VarDateTime(fProcInfo->GetItem("ArrivalTime")->Value));
				m_nxtArrivalTime->Enabled = VARIANT_TRUE;
			}
				
			//Now put in a separator row.
			pRow = m_pApptList->GetRow(-1);
			m_pApptList->InsertRow(pRow, 2);

			//Now, put in the "Other Appts" title row
			pRow = m_pApptList->GetRow(-1);
			pRow->PutValue(alcType, "----Other Appts----");
			m_pApptList->InsertRow(pRow, 3);


			//Now, loop through the other appointments and put them in.

			// (j.jones 2010-07-28 09:43) - PLID 28316 - check the preference to hide old appointments
			// (j.jones 2010-08-20 16:04) - PLID 40205 - default this preference to on
			_variant_t varPICDate = g_cvarNull;
			if(GetRemotePropertyInt("PICHideOldAppts",1,0,"<None>",true)) {
				//this really shouldn't be NULL, but if it is, we will just not filter by date
				varPICDate = fProcInfo->GetItem("PICDate")->Value;
			}

			// (j.jones 2008-11-17 11:25) - PLID 31686 - added LocationName and parameterized
			// (d.singleton 2012-05-02 11:28) - PLID 40019 added apt type category
			_RecordsetPtr rsOtherAppts = CreateParamRecordset("SELECT AppointmentsT.ID, AptTypeT.Name AS Type, StartTime, ArrivalTime, Color, "
				"LocationsT.Name AS LocationName, AptTypeT.Category "
				"FROM AppointmentsT "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
				"WHERE PatientID = {INT} AND AppointmentsT.ID <> (SELECT  CASE WHEN SurgeryApptID Is Null THEN -1 ELSE SurgeryApptID END FROM ProcInfoT WHERE ID = {INT}) "
				"AND (AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoID = {INT})) OR AppointmentsT.ID IN (SELECT AppointmentID FROM ProcInfoAppointmentsT WHERE ProcInfoID = {INT})) "
				"AND AppointmentsT.Status <> 4 "
				"AND ({VT_DATE} Is Null OR {VT_DATE} <= AppointmentsT.Date)", GetPatientID(), m_nProcInfoID, m_nProcInfoID, m_nProcInfoID, varPICDate, varPICDate);
			FieldsPtr fOtherAppts = rsOtherAppts->Fields;
			while(!rsOtherAppts->eof) {
				pRow = m_pApptList->GetRow(-1);
				pRow->PutValue(alcID, fOtherAppts->GetItem("ID")->Value);
				pRow->PutValue(alcType, fOtherAppts->GetItem("Type")->Value);
				pRow->PutValue(alcStartTime, fOtherAppts->GetItem("StartTime")->Value);
				// (j.jones 2008-11-17 11:25) - PLID 31686 - added Location column
				pRow->PutValue(alcLocation, fOtherAppts->GetItem("LocationName")->Value);
				pRow->PutValue(alcColor, fOtherAppts->GetItem("Color")->Value);
				pRow->PutValue(alcCategory, fOtherAppts->GetItem("Category")->Value);
				m_pApptList->AddRow(pRow);
				rsOtherAppts->MoveNext();
			}
			//Finally, put in the double-secret blank other row.  This is always available as a destination
			//for a dragged surgery appt.
			pRow = m_pApptList->GetRow(-1);
			pRow->PutValue(alcID, (long)-1);
			m_pApptList->AddRow(pRow);

			SetApptColors();	
		}
		break;

		case PIC_AREA_MED:
		{
		}
		break;

		case PIC_AREA_ALL:
		{
			long lPatID = GetPatientID();
			CString strListSQL;
			// (d.thompson 2009-08-18) - PLID 16758 - Added 'Expired' field
			strListSQL.Format("(SELECT PatientBillsQ.ID AS QuoteID, PatientBillsQ.Date AS QuoteDate, PatientBillsQ.Description AS FirstOfNotes, "
				"dbo.GetQuoteTotal(PatientBillsQ.ID, 0) AS Total, "
				"convert(bit, CASE WHEN UseExp = 1 THEN CASE WHEN DATEADD(d, ExpDays, Date) < getdate() then 1 else 0 end ELSE 0 END) AS Expired "
				"FROM (SELECT BillsT.* FROM BillsT WHERE BillsT.PatientID=%li AND BillsT.Deleted=0) AS PatientBillsQ "
				"WHERE PatientBillsQ.EntryType=2) AS Q",lPatID);

			m_pQuoteList->FromClause = _bstr_t(strListSQL);
			CString strQuoteWhere;
			// (j.jones 2010-06-23 13:48) - PLID 39276 - fixed the where clause so the OR code properly fired for always
			// showing the ActiveQuoteID even if the quote no longer has details that match the procedures
			strQuoteWhere.Format("((EXISTS (SELECT ID FROM ChargesT WHERE BillID = Q.QuoteID AND ServiceID IN (SELECT ID FROM ServiceT WHERE ServiceT.ProcedureID IN (SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoID = %li))) OR Q.QuoteID IN (SELECT QuoteID FROM ProcInfoQuotesT WHERE ProcInfoID = %li)) "
				"AND (Q.QuoteID NOT IN (SELECT ID FROM BillsT WHERE Active = 0)) OR Q.QuoteID = (SELECT ActiveQuoteID FROM ProcInfoT WHERE ID = %li))", m_nProcInfoID, m_nProcInfoID, m_nProcInfoID);
			m_pQuoteList->WhereClause = _bstr_t(strQuoteWhere);
			if(m_pQuoteList->CurSel != -1) m_nLastSelQuote = VarLong(m_pQuoteList->GetValue(m_pQuoteList->CurSel, 0), -1);
			else m_nLastSelQuote = -1;
			m_pQuoteList->Requery();

			if(IsSurgeryCenter(false)) {

				// (j.jones 2009-08-06 14:24) - PLID 7397 - added case history
				//show all cases not currently linked to other PIC records,
				//whether they are completed or not, and include the total from all line items

				//Do NOT show the total cost if they cannot see contact costs, and those fees exist.
				BOOL bCanViewPersonCosts = (GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead);

				strListSQL.Format("(SELECT CaseHistoryT.ID, PersonID, Name, SurgeryDate, CompletedDate, "
					//if the case has a person in it, hide the cost if the user has no permission
					"(CASE WHEN SUM(CASE WHEN ItemType = -3 THEN 1 ELSE 0 END) > 0 AND %li=0 THEN NULL "
					"ELSE "
					"Sum(Round(Convert(money,Cost*Quantity),2)) "
					"END) AS TotalCost "
					"FROM CaseHistoryT "
					"INNER JOIN CaseHistoryDetailsT ON CaseHistoryT.ID = CaseHistoryDetailsT.CaseHistoryID "
					"WHERE PersonID = %li "
					"AND CaseHistoryT.ID NOT IN (SELECT CaseHistoryID FROM ProcInfoT WHERE ID  <> %li AND CaseHistoryID Is Not Null) "
					"GROUP BY CaseHistoryT.ID, PersonID, Name, SurgeryDate, CompletedDate) AS CaseHistoryQ",
					bCanViewPersonCosts ? 1 : 0, lPatID, m_nProcInfoID);

				m_pCaseHistoryCombo->FromClause = _bstr_t(strListSQL);

				if(m_pCaseHistoryCombo->CurSel != -1) {
					m_nLastSelCase = VarLong(m_pCaseHistoryCombo->GetValue(m_pCaseHistoryCombo->CurSel, 0), -1);
				}
				else {
					m_nLastSelCase = -1;
				}
				m_pCaseHistoryCombo->Requery();

				//OnRequeryFinished will set the selection
			}

/*
	//DRT 2/16/2006 - PLID 18960 - This was never implemented, it's just hidden, so I've removed it
			CString strMedWhere;
			strMedWhere.Format("PatientMedications.PatientID = %li AND PatientMedications.Deleted = 0 AND PatientMedications.ID IN (SELECT MedicationID FROM ProcInfoMedicationsT WHERE ProcInfoID = %li)", GetPatientID(), m_nProcInfoID);
			m_pMedList->WhereClause = _bstr_t(strMedWhere);
			m_pMedList->Requery();
*/
			//OK, here goes:
			// (j.jones 2008-11-17 11:25) - PLID 31686 - added LocationName and parameterized
			// (j.jones 2010-07-28 09:58) - PLID 28316 - added PICDate
			// (d.singleton 2012-04-05 15:39) - PLID 40019 addeed AptTypeT.Category
			_RecordsetPtr rsProcInfo = CreateParamRecordset("SELECT SurgeonID, CoSurgeonID, NurseID, AnesthesiologistID, Anesthesia, AppointmentsT.ArrivalTime, "
				"PersonTCoord.Last + ', ' + PersonTCoord.First + ' ' + PersonTCoord.Middle AS Coord, AppointmentsT.ID AS ApptID, AptTypeT.Name AS Type, AptTypeT.Color, AptTypeT.Category, AppointmentsT.StartTime, LocationsT.Name AS LocationName, "
				"ActiveQuoteID, BillSubQ.Amount, BillSubQ.Date AS BillDate, CASE WHEN BillSubQ.Amount Is Null THEN 0 ELSE BillSubQ.Amount END - CASE WHEN AppliesSubQ.Amount Is Null THEN 0 ELSE AppliesSubQ.Amount END AS Balance, "
				"CASE WHEN LaddersT.FirstInterestDate Is Not Null THEN LaddersT.FirstInterestDate ELSE EMRGroupDateQ.InputDate END AS PICDate "
				"FROM ProcInfoT LEFT JOIN AppointmentsT LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID ON ProcInfoT.SurgeryApptID = AppointmentsT.ID "
				"LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
				"INNER JOIN (PatientsT LEFT JOIN PersonT PersonTCoord ON PatientsT.EmployeeID = PersonTCoord.ID) ON ProcInfoT.PatientID = PatientsT.PersonID "
				"LEFT JOIN "
				"(SELECT BillsT.ID, dbo.GetBillTotal(BillsT.ID) AS Amount, "
				"BillsT.Date, BillsT.Deleted AS BillDeleted "
				"FROM (ChargesT INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID) INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE LineItemT.Deleted = 0 GROUP BY BillsT.ID, BillsT.Date, BillsT.Deleted HAVING BillsT.Deleted = 0) BillSubQ "
				"ON ProcInfoT.BillID = BillSubQ.ID "
				"LEFT JOIN (SELECT BillID, Sum(Amount) AS Amount FROM AppliesT INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID GROUP BY ChargesT.BillID) AppliesSubQ ON ProcInfoT.BillID = AppliesSubQ.BillID "
				"LEFT JOIN LaddersT ON ProcInfoT.ID = LaddersT.ProcInfoID "
				"LEFT JOIN (SELECT PicT.ProcInfoID, EMRGroupsT.InputDate "
				"	FROM PicT INNER JOIN EMRGroupsT ON PicT.EMRGroupID = EMRGroupsT.ID "
				"	WHERE PicT.IsCommitted = 1) AS EMRGroupDateQ ON ProcInfoT.ID = EMRGroupDateQ.ProcInfoID "
				"WHERE ProcInfoT.ID = {INT}", m_nProcInfoID);

			if(rsProcInfo->eof) {
				MessageBox("Procedure Information record not found!");
				CDialog::OnOK();
				return;
			}
			FieldsPtr fProcInfo = rsProcInfo->Fields;

			COleVariant vNull;
			vNull.vt = VT_NULL;
			if(m_pSurgeon->TrySetSelByColumn(0, AdoFldLong(fProcInfo, "SurgeonID", -1)) == -1) {
				//they may have an inactive provider
				_RecordsetPtr rsProv = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = (SELECT SurgeonID FROM ProcInfoT WHERE ID = %li)", m_nProcInfoID);
				if(!rsProv->eof) {
					m_pSurgeon->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
				}
				else 
					m_pSurgeon->PutCurSel(-1);
			}

			// (a.walling 2006-11-28 12:46) - PLID 21003 - Load the co-surgeon
			m_varCoSurgeonID = fProcInfo->Item["CoSurgeonID"]->Value;
			if(!m_pCoSurgeon->IsRequerying()) {
				if(m_pCoSurgeon->SetSelByColumn(0, m_varCoSurgeonID) == sriNoRow) {
					LoadHiddenCoSurgeonRow();
				}
			}
			else {
				// (z.manning 2008-11-19 11:24) - PLID 31687 - Will be handled by the requery finished handler
			}

			//(e.lally 2005-11-28) PLID 18153 - add the ability to make Other contacts inactive
			if (fProcInfo->Item["NurseID"]->Value.vt == VT_NULL)
				m_pNurse->SetSelByColumn(0, vNull);
			else{
				long nNurseID = AdoFldLong(fProcInfo, "NurseID", -1);
				if(m_pNurse->TrySetSelByColumn(0, nNurseID) == -1){
					m_pNurse->PutCurSel(-1);
					//they may have an inactive nurse
					_RecordsetPtr rsNurse = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li", nNurseID);
					if(!rsNurse->eof) {
						m_pNurse->PutComboBoxText(_bstr_t(AdoFldString(rsNurse, "Name", "")));
					}
				}
			}

			if (fProcInfo->Item["AnesthesiologistID"]->Value.vt == VT_NULL)
				m_pAnesthesiologist->SetSelByColumn(0, vNull);
			else{
				long nAnesthesiologistID = AdoFldLong(fProcInfo, "AnesthesiologistID", -1);
				if(m_pAnesthesiologist->TrySetSelByColumn(0, nAnesthesiologistID == -1)){
					m_pAnesthesiologist->PutCurSel(-1);
					//they may have an inactive Anesthesiologist
					_RecordsetPtr rsAnesth = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li", nAnesthesiologistID);
					if(!rsAnesth->eof) {
						m_pAnesthesiologist->PutComboBoxText(_bstr_t(AdoFldString(rsAnesth, "Name", "")));
					}
				}
			}

			SetDlgItemText(IDC_ANESTHESIA, AdoFldString(fProcInfo, "Anesthesia", ""));
			
			SetDlgItemText(IDC_PAT_COORD, AdoFldString(fProcInfo, "Coord", ""));

			m_pApptList->Clear();
			//First put in the surgery title row
			IRowSettingsPtr pRow = m_pApptList->GetRow(-1);
			pRow->PutValue(alcType, "----Surgery----");
			m_pApptList->InsertRow(pRow, SURGERY_TITLE_ROW);

			//Now, put in the actual surgery
			pRow = m_pApptList->GetRow(-1);
			_variant_t varSurgeryTime;
			varSurgeryTime.vt = VT_NULL; //for the moment.
			_variant_t varID = fProcInfo->GetItem("ApptID")->Value;
			
			// (a.walling 2008-05-05 11:11) - PLID 29894 - Set the surgery appt id
			m_nSurgApptID = VarLong(varID, -1);

			if(varID.vt == VT_NULL) {
				pRow->PutValue(alcType, _bstr_t("<None>"));
				m_pApptList->InsertRow(pRow, SURGERY_ROW);
				m_nxtArrivalTime->Enabled = VARIANT_FALSE;
			}
			else {
				pRow->PutValue(alcID, varID);
				CString strType = AdoFldString(fProcInfo, "Type", "<No type>");
				pRow->PutValue(alcType, _bstr_t(strType));
				varSurgeryTime = fProcInfo->GetItem("StartTime")->Value;
				pRow->PutValue(alcStartTime, varSurgeryTime);
				// (j.jones 2008-11-17 11:25) - PLID 31686 - added Location column
				pRow->PutValue(alcLocation, fProcInfo->GetItem("LocationName")->Value);
				_variant_t varColor = fProcInfo->GetItem("Color")->Value;
				pRow->PutValue(alcColor, varColor);
				// (d.singleton 2012-04-05 15:41) - PLID 40019 added apttype category
				_variant_t varCategory = fProcInfo->GetItem("Category")->Value;
				pRow->PutValue(alcCategory, varCategory);
				m_pApptList->InsertRow(pRow, SURGERY_ROW);
				m_nxtArrivalTime->SetDateTime(VarDateTime(fProcInfo->GetItem("ArrivalTime")->Value));
				m_nxtArrivalTime->Enabled = VARIANT_TRUE;
			}
				
			//Now put in a separator row.
			pRow = m_pApptList->GetRow(-1);
			m_pApptList->InsertRow(pRow, SEPARATOR_ROW);

			//Now, put in the "Other Appts" title row
			pRow = m_pApptList->GetRow(-1);
			pRow->PutValue(alcType, "----Other Appts----");
			m_pApptList->InsertRow(pRow, OTHER_TITLE_ROW);

			//Now, loop through the other appointments and put them in.

			// (j.jones 2010-07-28 09:43) - PLID 28316 - check the preference to hide old appointments
			// (j.jones 2010-08-20 16:04) - PLID 40205 - default this preference to on
			_variant_t varPICDate = g_cvarNull;
			if(GetRemotePropertyInt("PICHideOldAppts",1,0,"<None>",true)) {
				//this really shouldn't be NULL, but if it is, we will just not filter by date
				varPICDate = fProcInfo->GetItem("PICDate")->Value;
			}

			// (j.jones 2008-11-17 11:25) - PLID 31686 - added LocationName and parameterized
			// (d.singleton 2012-04-05 15:42) - PLID 40019 added AptTypeT.Category
			_RecordsetPtr rsOtherAppts = CreateParamRecordset("SELECT AppointmentsT.ID, AptTypeT.Name AS Type, StartTime, Color, AptTypeT.Category, LocationsT.Name AS LocationName "
				"FROM AppointmentsT "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
				"WHERE PatientID = {INT} AND AppointmentsT.ID <> (SELECT  CASE WHEN SurgeryApptID Is Null THEN -1 ELSE SurgeryApptID END FROM ProcInfoT WHERE ID = {INT}) "
				"AND (AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoID = {INT})) OR AppointmentsT.ID IN (SELECT AppointmentID FROM ProcInfoAppointmentsT WHERE ProcInfoID = {INT})) "
				"AND AppointmentsT.Status <> 4 "
				"AND ({VT_DATE} Is Null OR {VT_DATE} <= AppointmentsT.Date)", GetPatientID(), m_nProcInfoID, m_nProcInfoID, m_nProcInfoID, varPICDate, varPICDate);
			FieldsPtr fOtherAppts = rsOtherAppts->Fields;
			while(!rsOtherAppts->eof) {
				pRow = m_pApptList->GetRow(-1);
				pRow->PutValue(alcID, fOtherAppts->GetItem("ID")->Value);
				pRow->PutValue(alcType, fOtherAppts->GetItem("Type")->Value);
				pRow->PutValue(alcStartTime, fOtherAppts->GetItem("StartTime")->Value);
				// (j.jones 2008-11-17 11:25) - PLID 31686 - added Location column
				pRow->PutValue(alcLocation, fOtherAppts->GetItem("LocationName")->Value);
				pRow->PutValue(alcColor, fOtherAppts->GetItem("Color")->Value);
				// (d.singleton 2012-04-05 15:42) - PLID 40019
				pRow->PutValue(alcCategory, fOtherAppts->GetItem("Category")->Value);
				m_pApptList->AddRow(pRow);
				rsOtherAppts->MoveNext();
			}
			//Finally, put in the double-secret blank other row.  This is always available as a destination
			//for a dragged surgery appt.
			pRow = m_pApptList->GetRow(-1);
			pRow->PutValue(alcID, (long)-1); 
			m_pApptList->AddRow(pRow);

			SetApptColors();

			// (j.dinatale 2012-07-10 14:22) - PLID 3073 - bill combo list!
			CString strBillComboSql;
			strBillComboSql.Format(
				"(SELECT BillSubQ.ID AS ID, BillSubQ.Amount AS Amount, BillSubQ.Date AS Date, BillsT.Description AS Description, "
				"CASE "
					"WHEN BillSubQ.Amount Is Null "
						"THEN 0 "
					"ELSE BillSubQ.Amount "
				"END - "
				"CASE "
					"WHEN AppliesSubQ.Amount Is Null "
						"THEN 0 "
					"ELSE AppliesSubQ.Amount "
				"END AS Balance "
				"FROM ProcInfoBillsT "
				"LEFT JOIN ( "
					"SELECT BillsT.ID, dbo.GetBillTotal(BillsT.ID) AS Amount, "
					"BillsT.Date, BillsT.Deleted AS Deleted "
					"FROM ( "
						"( "
							"ChargesT "
							"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
						") "
						"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
						"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
					") "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"WHERE LineItemT.Deleted = 0 GROUP BY BillsT.ID, BillsT.Date, BillsT.Deleted HAVING BillsT.Deleted = 0 "
				") BillSubQ ON ProcInfoBillsT.BillID = BillSubQ.ID "
				"LEFT JOIN ( "
					"SELECT BillID, Sum(Amount) AS Amount "
					"FROM AppliesT "
					"INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
					"GROUP BY ChargesT.BillID "
				") AppliesSubQ ON ProcInfoBillsT.BillID = AppliesSubQ.BillID "
				"LEFT JOIN BillsT ON ProcInfoBillsT.BillID = BillsT.ID "
				"WHERE ProcInfoBillsT.ProcInfoID = %li) SubQ", m_nProcInfoID);
			m_pBillCombo->FromClause = _bstr_t(strBillComboSql);
			m_pBillCombo->Requery();

			// (j.dinatale 2012-07-10 16:05) - PLID 3073 - no longer need to update those textboxes

			rsProcInfo->Close();
			
			//Let's calculate the prepays especially
			//TES 2/6/2007 - PLID 24377 - Filtered out refunds applied to the prepayments.
			_RecordsetPtr rsPrepayAmount = CreateRecordset("SELECT Sum(LineItemT.Amount) - COALESCE(Sum(AppliedToQ.Amount),0) AS Amount "
				"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"LEFT JOIN (SELECT DestID, Sum(-1*Amount) AS Amount FROM AppliesT GROUP BY DestID) AppliedToQ ON LineItemT.ID = AppliedToQ.DestID "
				"WHERE LineItemT.Deleted = 0 AND PaymentsT.Prepayment = 1 AND (PaymentsT.ID IN (SELECT SourceID FROM AppliesT WHERE DestID IN (SELECT ID FROM ChargesT WHERE BillID = (SELECT BillID FROM ProcInfoT WHERE ID = %li))) "
				"OR PaymentsT.ID IN (SELECT PayID FROM ProcInfoPaymentsT WHERE ProcInfoID = %li))", m_nProcInfoID, m_nProcInfoID);
			
			_RecordsetPtr rsPrepayApplied = CreateRecordset("SELECT Sum(CASE WHEN AppliesT.Amount Is Null THEN 0 ELSE AppliesT.Amount END) AS AppliedAmount "
				"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
				"WHERE LineItemT.Deleted = 0 AND PaymentsT.Prepayment = 1 AND (AppliesT.DestID IN (SELECT ID FROM ChargesT WHERE BillID = (SELECT BillID FROM ProcInfoT WHERE ID = %li)))",
				m_nProcInfoID);

			CString strPrepaysEntered, strPrepaysApplied;
			_variant_t varPrepaysEntered, varPrepaysApplied;
			varPrepaysEntered = rsPrepayAmount->Fields->GetItem("Amount")->Value;
			if(varPrepaysEntered.vt == VT_NULL) strPrepaysEntered = "";
			else strPrepaysEntered.Format("%s", FormatCurrencyForInterface(VarCurrency(varPrepaysEntered)));
			varPrepaysApplied = rsPrepayApplied->Fields->GetItem("AppliedAmount")->Value;
			if(varPrepaysApplied.vt == VT_NULL) strPrepaysApplied = "";
			else strPrepaysApplied.Format("%s", FormatCurrencyForInterface(VarCurrency(varPrepaysApplied)));

			SetDlgItemText(IDC_PREPAYS_ENTERED, strPrepaysEntered);
			SetDlgItemText(IDC_PREPAYS_APPLIED, strPrepaysApplied);
			//Try to fill in the remaining amount.
			COleCurrency cuPrepays = VarCurrency(varPrepaysEntered, COleCurrency(0,0));

			//TES 5/26/2004: Show the remaining amount even if there is no active quote.
			// (j.jones 2009-08-06 13:50) - PLID 7397 - tracked active quote amount in memory
			if(m_cyActiveQuoteAmount.GetStatus() == COleCurrency::valid) {
				SetDlgItemText(IDC_REMAINING_AMT, FormatCurrencyForInterface(m_cyActiveQuoteAmount-cuPrepays, TRUE, TRUE));
				InvalidateDlgItem(IDC_REMAINING_AMT);
			}
		}
		break;

		default:
			Load(PIC_AREA_ALL);
			break;
		}



	}NxCatchAll("Error in CProcInfoCenterDlg::Load");
}

void CProcInfoCenterDlg::LoadProcSpecific()
{
	long nMasterProc;
	if(m_pProcNames->CurSel == -1) nMasterProc = -1;
	else nMasterProc = VarLong(m_pProcNames->GetValue(m_pProcNames->CurSel, 2), -1);
	m_pProcDetailNames = BindNxDataListCtrl(IDC_PROC_NAME_LIST_DETAIL, false);
	CString strProcWhere;
	strProcWhere.Format("ProcedureT.MasterProcedureID = %li AND ProcInfoDetailsT.ProcInfoID = %li", nMasterProc, m_nProcInfoID);
	m_pProcDetailNames->WhereClause = _bstr_t(strProcWhere);
	m_pProcDetailNames->Requery();
}

void CProcInfoCenterDlg::OnOK() 
{
	CNxDialog::OnOK();
}

void CProcInfoCenterDlg::OnDefaultQuote() 
{
	try {
		if(m_pQuoteList->CurSel != -1) {

			long nQuoteID = VarLong(m_pQuoteList->GetValue(m_pQuoteList->CurSel, 0));

			CString strSql = BeginSqlBatch();
			AddStatementToSqlBatch(strSql, "UPDATE ProcInfoT SET ActiveQuoteID = %li WHERE ID = %li", nQuoteID, m_nProcInfoID);
			
			//TES 8/6/2007 - PLID 20720 - Also attach any linked prepayments (also made these all into a single batch).
			// (j.dinatale 2012-07-11 11:21) - PLID 3073 - need to filter out with whats in ProcInfoBillsT
			AddStatementToSqlBatch(strSql, "INSERT INTO ProcInfoPaymentsT (ProcInfoID, PayID) "
				"SELECT %li, ID FROM PaymentsT WHERE QuoteID = %li AND ID NOT IN "
				"(SELECT PayID FROM ProcInfoPaymentsT WHERE ProcInfoID = %li) "
				"AND ID NOT IN (SELECT SourceID FROM AppliesT INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
				"INNER JOIN ProcInfoBillsT ON ChargesT.BillID = ProcInfoBillsT.BillID WHERE ProcInfoBillsT.ProcInfoID = %li) ", 
				m_nProcInfoID, nQuoteID, m_nProcInfoID, m_nProcInfoID);

			// (j.jones 2005-05-13 09:53) - PLID 16506 - if any PrePayments are attached to the old Quote, attach them to the new Active quote
			AddStatementToSqlBatch(strSql, "UPDATE PaymentsT SET QuoteID = %li WHERE ID IN (SELECT PayID FROM ProcInfoPaymentsT WHERE ProcInfoID = %li)",nQuoteID, m_nProcInfoID);
			ExecuteSqlBatch(strSql);

			m_nLastSelQuote = nQuoteID;
			m_pQuoteList->Requery();

			//TES 8/6/2007 - PLID 20720 - We also need to refresh the payment area, since we may have added prepayments.
			Load(PIC_AREA_PAY);
		}
	}NxCatchAll("Error in CProcInfoCenterDlg::OnDefaultQuote()");
}

void CProcInfoCenterDlg::OnRequeryFinishedQuotes(short nFlags) 
{
	try {

		SetDlgItemText(IDC_QUOTE_AMT, "");
		InvalidateDlgItem(IDC_QUOTE_AMT);

		// (d.thompson 2009-08-18) - PLID 16758 - Gray out any expired quotes.  We do this first, so that if the active
		//	quote is expired, it will still be red.
		long p = m_pQuoteList->GetFirstRowEnum();
		LPDISPATCH pDisp = NULL;
		while(p) {
			m_pQuoteList->GetNextRowEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);

			if(VarBool(pRow->GetValue(4))) {
				//Expired quote, set the color
				pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
			}
			pDisp->Release();
		}

		_RecordsetPtr rsActiveQuote = CreateParamRecordset("SELECT ActiveQuoteID FROM ProcInfoT WHERE ID = {INT}", m_nProcInfoID);
		long nActiveRow = m_pQuoteList->FindByColumn(0, AdoFldLong(rsActiveQuote, "ActiveQuoteID", -1), 0, FALSE);
		if(nActiveRow != -1) {
			IRowSettingsPtr pRow = m_pQuoteList->GetRow(nActiveRow);
			pRow->ForeColor = RGB(255,0,0);
			pRow->ForeColorSel = RGB(255,0,0);
			// (j.jones 2009-08-06 13:50) - PLID 7397 - tracked active quote amount in memory
			m_cyActiveQuoteAmount = VarCurrency(pRow->GetValue(2), COleCurrency(0,0));
			SetDlgItemText(IDC_QUOTE_AMT, FormatCurrencyForInterface(m_cyActiveQuoteAmount, TRUE, TRUE));
			InvalidateDlgItem(IDC_QUOTE_AMT);
			CString strPrepays;
			GetDlgItemText(IDC_PREPAYS_ENTERED, strPrepays);
			COleCurrency cuPrepays;
			if(strPrepays == "") {
				cuPrepays = COleCurrency(0,0);
			}
			else {
				cuPrepays = ParseCurrencyFromInterface(strPrepays);
			}
			if(cuPrepays.GetStatus() == COleCurrency::valid) {
				SetDlgItemText(IDC_REMAINING_AMT, FormatCurrencyForInterface(m_cyActiveQuoteAmount - cuPrepays, TRUE, TRUE));
				InvalidateDlgItem(IDC_REMAINING_AMT);
			}
		}

		if(m_nLastSelQuote == -1) m_pQuoteList->CurSel = nActiveRow;
		else m_pQuoteList->SetSelByColumn(0, m_nLastSelQuote);

		GetDlgItem(IDC_DEFAULT_QUOTE)->EnableWindow(!(nActiveRow == m_pQuoteList->CurSel || m_pQuoteList->CurSel == -1));
		GetDlgItem(IDC_UNAPPLY_QUOTE)->EnableWindow((nActiveRow == m_pQuoteList->CurSel || m_pQuoteList->CurSel == -1) && nActiveRow != -1);
		GetDlgItem(IDC_VIEW_QUOTE)->EnableWindow(m_pQuoteList->CurSel != -1);

	}NxCatchAll("Error in CProcInfoCenterDlg::OnRequeryFinishedQuotes()");
}

void CProcInfoCenterDlg::OnSelChosenQuotes(long nRow) 
{
	try {
		if(nRow == -1) {
			GetDlgItem(IDC_DEFAULT_QUOTE)->EnableWindow(FALSE);
			GetDlgItem(IDC_VIEW_QUOTE)->EnableWindow(FALSE);
			GetDlgItem(IDC_UNAPPLY_QUOTE)->EnableWindow(FALSE);
		}
		else {
			_RecordsetPtr rsActiveQuote = CreateRecordset("SELECT ActiveQuoteID FROM ProcInfoT WHERE ID = %li", m_nProcInfoID);
			if(rsActiveQuote->eof) {
				GetDlgItem(IDC_DEFAULT_QUOTE)->EnableWindow(TRUE);
				GetDlgItem(IDC_UNAPPLY_QUOTE)->EnableWindow(FALSE);
			}
			else {
				//DRT 10/15/2003 - PLID 9691 - What if there is no active quote?
				if(rsActiveQuote->Fields->Item["ActiveQuoteID"]->Value.vt == VT_I4){
					BOOL bIsSelActiveQuote = VarLong(m_pQuoteList->GetValue(nRow,0)) == AdoFldLong(rsActiveQuote, "ActiveQuoteID");
					GetDlgItem(IDC_DEFAULT_QUOTE)->EnableWindow(!bIsSelActiveQuote);
					GetDlgItem(IDC_UNAPPLY_QUOTE)->EnableWindow(bIsSelActiveQuote);
				}
				else{
					GetDlgItem(IDC_DEFAULT_QUOTE)->EnableWindow(TRUE);
					GetDlgItem(IDC_UNAPPLY_QUOTE)->EnableWindow(FALSE);
				}
			}

			GetDlgItem(IDC_VIEW_QUOTE)->EnableWindow(TRUE);
		}
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::OnViewQuote() 
{
	try {
		//DRT 6/4/2008 - PLID 27065 - Need to check permissions
		if(!CheckCurrentUserPermissions(bioPatientQuotes, sptRead)) {
			return;
		}

		if(m_pQuoteList->CurSel != -1) {
			long nQuoteID = VarLong(m_pQuoteList->GetValue(m_pQuoteList->CurSel, 0));
			
			CBillingModuleDlg dlg(this);
			dlg.m_nPatientID = GetPatientID(); // (c.haag 2007-03-14 10:22) - PLID 25110 - Use the current patient
			dlg.m_pPostCloseMsgWnd = this;	//PLID 12071 - this gives the quote a pointer to send us a quit message back if they hit preview
			//Because this is a quote, this is done modally
			dlg.OpenWithBillID(nQuoteID, BillEntryType::Quote, 0);

			//We're refreshing regardless, because the description may have changed.
			Load(PIC_AREA_QUOTE);
		}
	}NxCatchAll("Error in CProcInfoCenterDlg::OnViewQuote()");
}

BOOL CProcInfoCenterDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int nID = LOWORD(wParam);
	
	CString strSql, strNewName;

	switch (HIWORD(wParam))
	{	
		case EN_KILLFOCUS:
			switch (nID)
			{	
			case IDC_CUSTOM1_PIC_BOX:
			case IDC_CUSTOM2_PIC_BOX:
			case IDC_CUSTOM3_PIC_BOX:
			case IDC_CUSTOM4_PIC_BOX:
			case IDC_CUSTOM5_PIC_BOX:
			case IDC_CUSTOM6_PIC_BOX:
			case IDC_ANESTHESIA:
				Save(nID);
				break;
			default:
				break;
			}

		default:
			break;
	}
	return CPatientDialog::OnCommand(wParam, lParam);
}


void CProcInfoCenterDlg::Save(int nID)
{
	try {
		CString strField, strValue;
		bool bProcSpecific = false;
		switch(nID) {
		case IDC_CUSTOM1_PIC_BOX:
			strField = "Custom1";
			GetDlgItemText(nID, strValue);
			strValue = "'" + _Q(strValue) + "'";
			bProcSpecific = true;
			break;
		case IDC_CUSTOM2_PIC_BOX:
			strField = "Custom2";
			GetDlgItemText(nID, strValue);
			strValue = "'" + _Q(strValue) + "'";
			bProcSpecific = true;
			break;
		case IDC_CUSTOM3_PIC_BOX:
			strField = "Custom3";
			GetDlgItemText(nID, strValue);
			strValue = "'" + _Q(strValue) + "'";
			bProcSpecific = true;
			break;
		case IDC_CUSTOM4_PIC_BOX:
			strField = "Custom4";
			GetDlgItemText(nID, strValue);
			strValue = "'" + _Q(strValue) + "'";
			bProcSpecific = true;
			break;
		case IDC_CUSTOM5_PIC_BOX:
			strField = "Custom5";
			GetDlgItemText(nID, strValue);
			strValue = "'" + _Q(strValue) + "'";
			bProcSpecific = true;
			break;
		case IDC_CUSTOM6_PIC_BOX:
			strField = "Custom6";
			GetDlgItemText(nID, strValue);
			strValue = "'" + _Q(strValue) + "'";
			bProcSpecific = true;
			break;

		case IDC_ANESTHESIA:
			strField = "Anesthesia";
			GetDlgItemText(nID, strValue);
			strValue = "'" + _Q(strValue) + "'";
			break;

		case IDC_SURGEON:
			if(m_pSurgeon->CurSel == -1) return;
			strField = "SurgeonID";
			strValue.Format("%li", VarLong(m_pSurgeon->GetValue(m_pSurgeon->CurSel, 0)));
			break;

		case IDC_COSURGEON: // (a.walling 2006-11-28 12:42) - PLID 21003 - Save the cosurgeon
			if(m_pCoSurgeon->CurSel == -1) return;
			strField = "CoSurgeonID";
			if (m_pCoSurgeon->GetValue(m_pCoSurgeon->CurSel, 0).vt == VT_NULL) {
				strValue = "NULL";
				m_varCoSurgeonID.vt = VT_NULL;
			}
			else {
				m_varCoSurgeonID = m_pCoSurgeon->GetValue(m_pCoSurgeon->CurSel, 0);
				strValue.Format("%li", VarLong(m_varCoSurgeonID));
			}
			break;

		case IDC_NURSE:
			if(m_pNurse->CurSel == -1) return;
			strField = "NurseID";
			if (m_pNurse->GetValue(m_pNurse->CurSel, 0).vt == VT_NULL)
				strValue = "NULL";
			else
				strValue.Format("%li", VarLong(m_pNurse->GetValue(m_pNurse->CurSel, 0)));
			break;

		case IDC_ANESTHESIOLOGIST_LIST:
			if(m_pAnesthesiologist->CurSel == -1) return;
			strField = "AnesthesiologistID";
			if (m_pAnesthesiologist->GetValue(m_pAnesthesiologist->CurSel, 0).vt == VT_NULL)
				strValue = "NULL";
			else
				strValue.Format("%li", VarLong(m_pAnesthesiologist->GetValue(m_pAnesthesiologist->CurSel, 0)));
			break;

		case IDC_ARRIVAL_HR:
			{
				// (z.manning, 4/25/2006, PLID 19093) - This is now a property of appts and not the PIC.

				if(m_pApptList->GetValue(SURGERY_ROW, alcID).vt != VT_I4) {
					// Should not have been able to enter an arrival time if there's no surgery.
					ASSERT(FALSE);
					return;
				}

				COleDateTime dtStart = VarDateTime(m_pApptList->GetValue(SURGERY_ROW, alcStartTime));
				if(m_nxtArrivalTime->GetStatus() != 1) {
					m_nxtArrivalTime->SetDateTime(dtStart);
				}

				COleDateTime dtArrival = m_nxtArrivalTime->GetDateTime();
				if(dtArrival.GetStatus() != COleDateTime::valid) {
					dtArrival = dtStart;
					ASSERT(dtArrival.GetStatus() == COleDateTime::valid);
				}

				// Make sure arrival time is >= start time
				if(dtArrival > dtStart) {
					MsgBox("The arrival time cannot be after the surgery's start time.");
					dtArrival = dtStart;
				}
				// The arrival time should be on the same date as the surgery.
				else if( dtArrival.GetYear() != dtStart.GetYear() 
					|| dtArrival.GetMonth() != dtStart.GetMonth()
					|| dtArrival.GetDay() != dtStart.GetDay() )
				{
					MsgBox("The arrival time must be on the same date as the surgery.");
					dtArrival.SetDateTime(dtStart.GetYear(), dtStart.GetMonth(), dtStart.GetDay(), dtArrival.GetHour(), dtArrival.GetMinute(), dtArrival.GetSecond());
				}

				long nAptID = VarLong(m_pApptList->GetValue(SURGERY_ROW, alcID));
				COleDateTime dtOldArrival = VarDateTime(GetTableField("AppointmentsT", "ArrivalTime", "ID", nAptID));
				// (z.manning, 4/27/2006, PLID 20287) - If the arrival time changed, update it and audit.
				if(dtArrival != dtOldArrival) {
					ExecuteSql("UPDATE AppointmentsT SET ArrivalTime = '%s' WHERE ID = %li ",
						FormatDateTimeForSql(dtArrival), nAptID);

					long nAuditID = BeginNewAuditEvent();
					if(nAuditID != -1) {
						AuditEvent(GetPatientID(), GetPatientName(), nAuditID, aeiApptArrivalTime, GetPatientID(), FormatDateTimeForInterface(dtOldArrival), FormatDateTimeForInterface(dtArrival), aepMedium);
					}
				}

				m_nxtArrivalTime->SetDateTime(dtArrival);

				return;
			}
			break;

		default:
			return;
			break;
		}

		CString strSql;
		if(bProcSpecific) {
			strSql.Format("UPDATE ProcInfoDetailsT SET %s = %s WHERE ID = %li", strField, strValue, VarLong(m_pProcNames->GetValue(m_pProcNames->CurSel, 0)));
		}
		else {
			strSql.Format("UPDATE ProcInfoT SET %s = %s WHERE ID = %li", strField, strValue, m_nProcInfoID);
		}

		// (j.jones 2010-04-20 09:07) - PLID 38273 - use ExecuteSqlStd (can't parameterize the way this saving code currently works)
		ExecuteSqlStd(strSql);

	}NxCatchAll("Error in CProcInfoCenterDlg::Save()");
}

void CProcInfoCenterDlg::OnSelChosenSurgeon(long nRow) 
{
	if(nRow != -1) {
		Save(IDC_SURGEON);
	}
}

void CProcInfoCenterDlg::OnSelChosenNurse(long nRow) 
{
	try {
		if(nRow != -1) {

			if(m_pNurse->CurSel != -1) {
				long nPersonID = VarLong(m_pNurse->GetValue(m_pNurse->CurSel, 0),-1);

				if(!CheckWarnPersonLicenses(nPersonID,"Nurse")) {
					//clear out and re-select a new Nurse
					m_pNurse->CurSel = -1;
					m_pNurse->DropDownState = TRUE;
					return;
				}
			}
			Save(IDC_NURSE);
		}
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::OnSelChosenAnesthesiologistList(long nRow) 
{
	try {
		if(nRow != -1) {
			if(m_pAnesthesiologist->CurSel != -1) {
				long nPersonID = VarLong(m_pAnesthesiologist->GetValue(m_pAnesthesiologist->CurSel, 0),-1);

				if(!CheckWarnPersonLicenses(nPersonID,"Anesthesiologist")) {
					//clear out and re-select a new Anesthesiologist
					m_pAnesthesiologist->CurSel = -1;
					m_pAnesthesiologist->DropDownState = TRUE;
					return;
				}
			}
			Save(IDC_ANESTHESIOLOGIST_LIST);
		}
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::OnAddPrepay() 
{
	try {
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		// (j.jones 2008-11-13 16:18) - PLID 28187 - clarified these menu labels
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, NXM_PREPAY_NEW, "&Create New PrePayment");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, NXM_PREPAY_EXISTING, "&Link Existing PrePayment");

		CRect rBtn;
		GetDlgItem(IDC_ADD_PREPAY)->GetWindowRect(rBtn);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, rBtn.right, rBtn.top, this, NULL);
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::OnPrepayNew()
{
	try {
		if(!CheckCurrentUserPermissions(bioPayment,sptCreate)) return;

		CPaymentDlg dlg(this);
		dlg.m_bIsPrePayment = true;
		dlg.m_boIsNewPayment = TRUE;
		dlg.m_PatientID = GetPatientID();
		dlg.m_iDefaultPaymentType = 0;
		dlg.m_iDefaultInsuranceCo = -1;
		
		// (j.jones 2009-08-25 11:55) - PLID 31549 - send the ProcInfoID to the Payment, which removes
		// the PIC's responsibility to update ProcInfoPaymentsT
		dlg.m_nProcInfoID = m_nProcInfoID;
		//bool bNeedInsert = true;

		// (j.jones 2005-05-13 09:41) - PLID 16487 - auto-link to the active quote, if there is one
		_RecordsetPtr rsActiveQuote = CreateParamRecordset("SELECT ActiveQuoteID FROM ProcInfoT WHERE ID = {INT}", m_nProcInfoID);
		if(!rsActiveQuote->eof) {
			long nQuoteID = AdoFldLong(rsActiveQuote, "ActiveQuoteID",-1);
			dlg.m_varBillID = (long)nQuoteID;
			dlg.m_QuoteID = nQuoteID;
			//TES 8/16/2007 - PLID 20720 - The payment dialog now fills ProcInfoPaymentsT by itself, if it is a new
			// prepayment and it is given a quote ID.  So, we need to remember not to re-add it.
			// (j.jones 2009-08-25 11:55) - PLID 31549 - this is now obsolete
			//bNeedInsert = false;
		}
		rsActiveQuote->Close();

		int nReturn = dlg.DoModal(__FUNCTION__, __LINE__);
		// (j.jones 2009-08-25 11:54) - PLID 31549 - the payment dialog is now fully responsible
		// for updating ProcInfoPaymentsT, and sending a message to tell the open PIC to reload
		// the payment section
		/*
		if(nReturn != IDCANCEL) {
			//TES 8/16/2007 - PLID 20720 - Don't bother with this if the payment dialog already took care of it.
			if(bNeedInsert) {
				//Make sure it's still a prepayment
				if(ReturnsRecords("SELECT LineItemT.ID FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID WHERE Type = 1 AND PrePayment = 1 AND LineItemT.ID = %li", VarLong(dlg.m_varPaymentID))) {
					ExecuteSql("INSERT INTO ProcInfoPaymentsT (ProcInfoID, PayID) VALUES (%li, %li)", m_nProcInfoID, VarLong(dlg.m_varPaymentID));
					PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_PaymentApplied, GetPatientID(), COleDateTime::GetCurrentTime(),VarLong(dlg.m_varPaymentID),true,-1);
					Load(PIC_AREA_PAY);
				}
			}
			else {
				//The payment dialog took care of it, so we need to refresh the screen.
				Load(PIC_AREA_PAY);
			}
		}
		*/
		if(nReturn == RETURN_PREVIEW) {
			//They're previewing.  Let's get out of here!
			if(GetPicContainer()) {
				GetPicContainer()->SendMessage(NXM_CLOSE_PIC, cprNone);
			}
			else {
				OnOK();
			}
		}
	}NxCatchAll("Error in CProcInfoCenterDlg::OnPrepayNew()");
}


void CProcInfoCenterDlg::OnPrepayExisting()
{
	try {
		// (j.dinatale 2011-09-13 12:25) - PLID 45277 - Need to account for financial corrections, exclude original and voiding line items from the list
		//TES 2/6/2007 - PLID 24377 - Filtered out refunds applied to the prepayments.
		_RecordsetPtr rsPrepays = CreateRecordset("SELECT LineItemT.ID, LineItemT.Description, LineItemT.Date, "
			"LineItemT.Amount - COALESCE(AppliedToQ.Amount,0) AS Amount "
			"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID LEFT JOIN (SELECT DestID, Sum(-1*Amount) AS Amount "
			"	 FROM AppliesT GROUP BY DestID) AS AppliedToQ ON LineItemT.ID = AppliedToQ.DestID "
			"LEFT JOIN LineItemCorrectionsT OrigLineItems ON LineItemT.ID = OrigLineItems.OriginalLineItemID "
			"LEFT JOIN LineItemCorrectionsT VoidingLineItems ON LineItemT.ID = VoidingLineItems.VoidingLineItemID "
			"WHERE PaymentsT.Prepayment = 1 AND LineItemT.PatientID = %li AND LineItemT.Deleted = 0 AND "
			"VoidingLineItems.VoidingLineItemID IS NULL AND OrigLineItems.OriginalLineItemID IS NULL AND "
			"PaymentsT.ID NOT IN (SELECT PayID FROM ProcInfoPaymentsT WHERE ProcInfoID = %li)", GetPatientID(), m_nProcInfoID);

		if(rsPrepays->eof) {
			int nReturn = MsgBox(MB_YESNO, "There are no unattached prepayments available.  Would you like to create a new prepayment?");
			if(nReturn == IDYES) {
				OnPrepayNew();
			}
		}
		else {
			CSelectPrepayDlg dlg(this);
			dlg.m_rsPrepays = rsPrepays;
			int nReturn = dlg.DoModal();
			if(nReturn == IDOK) {

				// (j.jones 2005-05-13 09:41) - PLID 16487 - we will auto-link to the active quote, if there is one
				long nQuoteID = -1;
				_RecordsetPtr rsActiveQuote = CreateRecordset("SELECT ActiveQuoteID FROM ProcInfoT WHERE ID = %li", m_nProcInfoID);
				if(!rsActiveQuote->eof) {
					nQuoteID = AdoFldLong(rsActiveQuote, "ActiveQuoteID",-1);
				}
				rsActiveQuote->Close();

				for(int i = 0; i < dlg.m_arSelectedIds.GetSize(); i++) {
					ExecuteSql("INSERT INTO ProcInfoPaymentsT (ProcInfoID, PayID) VALUES (%li, %li)", m_nProcInfoID, dlg.m_arSelectedIds.GetAt(i));
					PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_PaymentApplied, GetPatientID(), COleDateTime::GetCurrentTime(),dlg.m_arSelectedIds.GetAt(i),true,-1);

					// (j.jones 2005-05-13 09:41) - PLID 16487 - auto-link to the active quote, if there is one
					if(nQuoteID != -1) {
						ExecuteSql("UPDATE PaymentsT SET QuoteID = %li WHERE ID = %li",nQuoteID,dlg.m_arSelectedIds.GetAt(i));
					}
				}
				Load(PIC_AREA_PAY);
			}
		}
	}NxCatchAll("Error in CProcInfoCenterDlg::OnPrepayExisting()");
}

void CProcInfoCenterDlg::OnAddBill() 
{
	try {
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		// (j.jones 2008-11-13 16:18) - PLID 28187 - clarified these menu labels
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, NXM_BILL_NEW, "&Create New Bill");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, NXM_BILL_EXISTING, "&Link Existing Bill");
		//DRT 7/5/2007 - PLID 23496 - Bill the active quote directly.  If there is no active quote, disable the option
		UINT nFlags = MF_BYPOSITION;
		if(GetActiveQuoteID() == -1)
			nFlags |= MF_GRAYED;
		mnu.InsertMenu(nIndex++, nFlags, NXM_BILL_ACTIVE_QUOTE, "Create New Bill from Active &Quote");

		CRect rBtn;
		GetDlgItem(IDC_ADD_BILL)->GetWindowRect(rBtn);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, rBtn.right, rBtn.top, this, NULL);
	} NxCatchAll("Error in OnAddBill");
}


void CProcInfoCenterDlg::OnBillNew()
{
	try {
		if(!CheckCurrentUserPermissions(bioBill,sptCreate)) return;

		// (j.dinatale 2012-07-10 15:34) - PLID 3073 - no longer need this prompt... we now support multi bills
		/*if(ReturnsRecords("SELECT BillID FROM ProcInfoT WHERE ID = %li AND BillID IS Not Null", m_nProcInfoID)) {
			int nReturn = MsgBox(MB_YESNO, "There is already a bill associated with this procedure.  Are you sure you wish to continue?\n  If you say \"Yes,\" the new bill will become the active bill for this procedure.");
			if(nReturn == IDNO) return;
		}*/
		
		// (a.walling 2009-12-22 17:27) - PLID 7002 - Maintain only one instance of a bill
		if (GetMainFrame()->IsBillingModuleOpen(true)) {
			return;
		}

		// (j.jones 2009-08-10 09:21) - PLID 35143 - ensure the patients module is open
		CMainFrame *pMainFrame = GetMainFrame();
		if(pMainFrame) {
			if(pMainFrame->FlipToModule(PATIENT_MODULE_NAME)) {

				CPatientView* pPatView = (CPatientView*)pMainFrame->GetOpenView(PATIENT_MODULE_NAME);
				if(pPatView) {

					m_pBillingDlg = pPatView->GetBillingDlg();

					if(m_pBillingDlg == NULL) {
						ASSERT(FALSE);
						ThrowNxException("Could not open bill!");
					}

					m_pBillingDlg->m_nPatientID = GetPatientID(); // (c.haag 2007-03-14 10:22) - PLID 25110 - Use the current patient

					m_pOldFinancialDlg = m_pBillingDlg->m_pFinancialDlg;
					m_pBillingDlg->m_pFinancialDlg = this;

					//try to get the surgery date
					_RecordsetPtr rs = CreateRecordset("SELECT Date FROM AppointmentsT WHERE ID IN (SELECT SurgeryApptID FROM ProcInfoT WHERE ID = %li)",m_nProcInfoID);
					if(!rs->eof) {
						m_pBillingDlg->m_bUseDefaultDate = TRUE;
						m_pBillingDlg->m_dtDefaultDate = AdoFldDateTime(rs, "Date",COleDateTime::GetCurrentTime());
					}
					
					m_pBillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 1);
				}
				else {
					ASSERT(FALSE);
				}
			}
			else {
				ASSERT(FALSE);
			}
		}
		else {
			ASSERT(FALSE);
		}

	}NxCatchAll("Error in CProcInfoCenterDlg::OnBillNew()");
}


void CProcInfoCenterDlg::OnBillExisting()
{
	try {
		// (j.gruber 2011-08-08 15:49) - PLID 44930 - take out original bills
		// (j.dinatale 2012-07-10 15:37) - PLID 3073 - multi bills so look at procinfobillst
		if(!ReturnsRecords("SELECT BillsT.ID FROM BillsT LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID WHERE BillCorrectionsT.ID IS NULL AND BillsT.ID IN (SELECT BillsT.ID FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE LineItemT.PatientID = (SELECT PatientID FROM ProcInfoT WHERE ID = %li) AND LineItemT.Deleted = 0 AND BillsT.Deleted = 0) AND BillsT.ID NOT IN (SELECT BillID FROM ProcInfoBillsT WHERE ProcInfoID = %li) AND BillsT.EntryType = 1", m_nProcInfoID, m_nProcInfoID)) {
			int nReturn = MsgBox(MB_YESNO, "There are no unattached bills to use.  Would you like to create a new bill?");
			if(nReturn == IDYES) {
				OnBillNew();
			}
			return;
		}
		else {
			// (j.dinatale 2012-07-10 15:13) - PLID 3073 - I dont think this warning is necessary anymore
			/*if(ReturnsRecords("SELECT BillID FROM ProcInfoT WHERE ID = %li AND BillID Is Not Null", m_nProcInfoID)) {
				int nReturn = MsgBox(MB_YESNO, "There is already a bill associated with this procedure.  Are you sure you wish to continue?\n  If you say \"Yes,\" the bill you select will become the active bill for this procedure.");
				if(nReturn == IDNO) return;
			}*/

			CString strWhere;
			// (j.gruber 2011-08-08 15:51) - PLID 44930 - take out original bills
			// (j.dinatale 2012-07-10 15:14) - PLID 3073 - take out already linked bills
			strWhere.Format("BillsT.ID IN (SELECT BillsT.ID FROM BillsT "
				"LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE BillCorrectionsT.ID IS NULL "
				"AND LineItemT.PatientID = (SELECT PatientID FROM ProcInfoT WHERE ID = %li) "
				"AND LineItemT.Deleted = 0 AND BillsT.Deleted = 0) "
				"AND BillsT.ID NOT IN (SELECT BillID FROM ProcInfoBillsT WHERE ProcInfoID = %li) AND BillsT.EntryType = 1", 
				m_nProcInfoID, m_nProcInfoID);
			CSelectBillDlg dlg(this);
			dlg.m_strWhere = strWhere;
			int nReturn = dlg.DoModal();
			if(nReturn == IDOK && dlg.m_nBillID != -1) {
				// (j.dinatale 2012-07-10 15:08) - PLID 3073 - update procinfobillst
				ExecuteParamSql(
					"UPDATE ProcInfoT SET BillID = {INT} WHERE ID = {INT} AND BillID IS NULL; "
					"INSERT INTO ProcInfoBillsT(ProcInfoID, BillID) VALUES ({INT}, {INT}); ", 
					dlg.m_nBillID, m_nProcInfoID, m_nProcInfoID, dlg.m_nBillID);
				ApplyPrePaysToBill(dlg.m_nBillID);
				Load(PIC_AREA_BILL);
			}
		}
	}NxCatchAll("Error in CProcInfoCenterDlg::OnBillExisting()");
}

//DRT 7/5/2007 - PLID 23496 - Added optimization to attempt to get the active quote ID without looking to data
long CProcInfoCenterDlg::GetActiveQuoteID()
{
	//Search the quote list for one that's highlighted red.  If we don't find one, do a lookup in data
	long nPos = m_pQuoteList->GetFirstRowEnum();
	LPDISPATCH lpDisp = NULL;
	while(nPos) {
		m_pQuoteList->GetNextRowEnum(&nPos, &lpDisp);

		IRowSettingsPtr pRow(lpDisp);

		if(pRow->GetForeColor() == RGB(255, 0, 0)) {
			//This is the one
			long nQuoteID = VarLong(pRow->GetValue(0), -1);
			lpDisp->Release();

			return nQuoteID;
		}

		lpDisp->Release();
	}

	//If we got here, it was never found.  We need to do a manual lookup in data
	_RecordsetPtr rsActiveQuote = CreateRecordset("SELECT ActiveQuoteID FROM ProcInfoT WHERE ID = %li", m_nProcInfoID);
	if(!rsActiveQuote->eof) {
		return AdoFldLong(rsActiveQuote, "ActiveQuoteID", -1);
	}
	rsActiveQuote->Close();

	//not found
	return -1;
}

//DRT 7/5/2007 - PLID 23496 - Added this function to bill the active quote directly
//	Note:  As of this date, I had a big debate internally about the speed.  This function is basically a copy of OnBillNew, which
//	also opens numerous recordsets.  The best way to fix all this would be to change some fundamental behavior of this entire dialog
//	to better support caching information.  While that is an excellent idea, it does not fit within the current scope, and thus
//	cannot be implemented.  I went a middle ground and added the above function, GetActiveQuoteID(), which attempts to get the ID 
//	from the datalist, then checks data if that doesn't exist for safety.  Anyways, I added PLID 26558 to improve a lot of
//	the speed issues in this dialog.
//The main problem with implementing a better solution is that with EMR, this is a modeless dialog that can be kept open for hours if
//	the user wishes, so I can't take the current behavior (always query data) and change to (always query interface) without making
//	sure that we have table checkers, etc working and in place.
void CProcInfoCenterDlg::OnBillActiveQuote()
{
	try {
		if(!CheckCurrentUserPermissions(bioBill,sptCreate)) return;

		//TODO:  There has to be a way to get this info w/o a data access
		// (j.dinatale 2012-07-10 15:39) - PLID 3073 - dont need a warning anymore, multibills are here
		/*if(ReturnsRecords("SELECT BillID FROM ProcInfoT WHERE ID = %li AND BillID IS Not Null", m_nProcInfoID)) {
			if(MsgBox(MB_YESNO, "There is already a bill associated with this procedure.  Are you sure you wish to continue?\n  If you say \"Yes,\" the new bill will become the active bill for this procedure.") != IDYES) {
				return;
			}
		}*/

		//Find the active quote
		long nActiveQuoteID = GetActiveQuoteID();
		if(nActiveQuoteID == -1) {
			//This should not be possible
			ASSERT(FALSE);
			return;
		}
		
		// (a.walling 2009-12-22 17:27) - PLID 7002 - Maintain only one instance of a bill
		if (GetMainFrame()->IsBillingModuleOpen(true)) {
			return;
		}

		// (j.jones 2009-08-10 09:21) - PLID 35143 - ensure the patients module is open
		CMainFrame *pMainFrame = GetMainFrame();
		if(pMainFrame) {
			if(pMainFrame->FlipToModule(PATIENT_MODULE_NAME)) {

				CPatientView* pPatView = (CPatientView*)pMainFrame->GetOpenView(PATIENT_MODULE_NAME);
				if(pPatView) {

					m_pBillingDlg = pPatView->GetBillingDlg();

					if(m_pBillingDlg == NULL) {
						ASSERT(FALSE);
						ThrowNxException("Could not open bill!");
					}

					m_pBillingDlg->m_nPatientID = GetPatientID();

					//This will be reset in OnPostEditBill
					m_pOldFinancialDlg = m_pBillingDlg->m_pFinancialDlg;
					m_pBillingDlg->m_pFinancialDlg = this;

					//try to get the surgery date
					_RecordsetPtr rs = CreateRecordset("SELECT Date FROM AppointmentsT WHERE ID IN (SELECT SurgeryApptID FROM ProcInfoT WHERE ID = %li)",m_nProcInfoID);
					if(!rs->eof) {
						m_pBillingDlg->m_bUseDefaultDate = TRUE;
						m_pBillingDlg->m_dtDefaultDate = AdoFldDateTime(rs, "Date",COleDateTime::GetCurrentTime());
					}

					m_pBillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 1);

					//This is oddly labeled as a package, but really it's just a quote (the PendingQuotesDlg uses this too).  I got this
					//	functionality from CFinancialDlg::BillPackage()
					m_pBillingDlg->PostMessage(NXM_ADD_PACKAGE, (WPARAM)nActiveQuoteID, (LPARAM)0);

				}
				else {
					ASSERT(FALSE);
				}
			}
			else {
				ASSERT(FALSE);
			}
		}
		else {
			ASSERT(FALSE);
		}

	} NxCatchAll("Error in OnBillActiveQuote");
}

// (a.walling 2007-11-07 10:18) - PLID 27998 - VS2008 - OnPostEditBill should use WPARAM/LPARAM
LRESULT CProcInfoCenterDlg::OnPostEditBill(WPARAM wParam, LPARAM lParam)
{
	try {
		long iBillID = (long)wParam;
		long iSaveType = (long)lParam;
		//First, save changes
		if(iSaveType == 1) {
			if(iBillID != -1) {
				// (j.dinatale 2012-07-10 14:37) - PLID 3073 - we need to only update ProcInfoT if the billID is null
				ExecuteParamSql(
					"UPDATE ProcInfoT SET BillID = {INT} WHERE ID = {INT} AND BillID IS NULL "
					"INSERT INTO ProcInfoBillsT(ProcInfoID, BillID) VALUES ({INT}, {INT}) ", 
					iBillID, m_nProcInfoID, m_nProcInfoID, iBillID);
				ApplyPrePaysToBill(iBillID);
				Load(PIC_AREA_BILL);
			}
			//Now, clean up our tracks, so that billing will never know we "borrowed" its dialog.
			m_pBillingDlg->m_pFinancialDlg = m_pOldFinancialDlg;

			m_pOldFinancialDlg = NULL;
						
		}

		m_pBillingDlg = NULL; //Don't worry about cleanup; that's up to patient view.
	}NxCatchAll("Error in CProcInfoCenterDlg::OnPostEditBill()");
	return 0;
}

void CProcInfoCenterDlg::OnRequeryFinishedOtherAppts(short nFlags) 
{
	try {
		//Color all the rows.
		for(int i = 0; i < m_pApptList->GetRowCount(); i++) {
			IRowSettingsPtr pRow = m_pApptList->GetRow(i);
			pRow->PutForeColor(VarLong(pRow->GetValue(alcColor), RGB(0,0,0)));
		}
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::OnAdd() 
{
	try {
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		
		// (j.jones 2008-11-13 16:18) - PLID 28187 - clarified these menu labels
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, NXM_APPT_NEW, "&Create New Appointment");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, NXM_APPT_EXISTING, "&Link Existing Appointment");

		CRect rBtn;
		GetDlgItem(IDC_ADD)->GetWindowRect(rBtn);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, rBtn.right, rBtn.top, this, NULL);
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::OnApptNew()
{
	try {
		//(c.copits 2010-10-05) PLID 27066 - Add message when user lacks permission
		if (!CheckCurrentUserPermissions(bioAppointment, sptCreate)) {
			return;
		}

		// (c.haag 2007-03-14 10:23) - PLID 25110 - Since we don't get a handle to the new appointment object,
		// all we can really do is ensure the active patient ID is our patient ID
		//TES 1/7/2010 - PLID 36761 - No need to check for failure, we wouldn't have been able to open the PIC if we couldn't 
		// access this patient.
		GetMainFrame()->m_patToolBar.TrySetActivePatientID(GetPatientID());

		//Sadly, there is no way to just pop-up a resentry, or anything.  They'll just have to create it, then come back and link it manually.
		if(GetPicContainer()) {
			GetPicContainer()->SendMessage(NXM_CLOSE_PIC, cprScheduler);
		}
		else {
			CDialog::OnOK();
			// (z.manning 2011-09-01 15:25) - PLID 43347 - We now call ActivateTab to do this.
			g_Modules[Modules::Scheduler]->ActivateTab(SchedulerModule::MonthTab);
		}
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::OnApptExisting()
{
	try {
		//We want all appointments that aren't already the surgery appointment or on the list.
		CString strWhere;
		strWhere.Format("PatientID = %li AND AppointmentsT.ID <> (SELECT CASE WHEN SurgeryApptID Is Null THEN -1 ELSE SurgeryApptID END FROM ProcInfoT WHERE ID = %li) AND (AppointmentsT.ID NOT IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoID = %li)) AND AppointmentsT.ID NOT IN (SELECT AppointmentID FROM ProcInfoAppointmentsT WHERE ProcInfoID = %li)) AND AppointmentsT.Status <> 4", GetPatientID(), m_nProcInfoID, m_nProcInfoID, m_nProcInfoID);
		if(ReturnsRecords("SELECT ID FROM AppointmentsT WHERE %s", strWhere)) {
			CSelectApptDlg dlg(this);
			dlg.m_strWhere = strWhere;
			int nReturn = dlg.DoModal();
			if(nReturn == IDOK) {
				for(int i = 0; i < dlg.m_arSelectedIds.GetSize(); i++) {
					ExecuteSql("INSERT INTO ProcInfoAppointmentsT (ProcInfoID, AppointmentID) VALUES (%li, %li)", m_nProcInfoID, dlg.m_arSelectedIds.GetAt(i));
				}
				Load(PIC_AREA_APPT);
			}
		}
		else {
			int nReturn = MsgBox(MB_YESNO, "There are no unattached appointments to associate with this procedure. Would you like to create a new appointment?");
			if(nReturn == IDYES) {
				OnApptNew();
			}
		}
	}NxCatchAll("Error in CProcInfoCenterDlg::OnApptExisting()");
}

void CProcInfoCenterDlg::OnNewQuotePic() 
{
	try {
		//DRT 6/4/2008 - PLID 27065 - Need to check permissions
		if(!CheckCurrentUserPermissions(bioPatientQuotes, sptCreate)) {
			return;
		}

		CBillingModuleDlg dlg(this);
		dlg.m_pPostCloseMsgWnd = this;	//PLID 12071 - this gives the quote a pointer to send us a quit message back if they hit preview
		dlg.m_nPatientID = GetPatientID(); // (c.haag 2007-03-14 10:24) - PLID 25110 - Use the current patient ID
		dlg.OpenWithBillID(-1, BillEntryType::Quote, 2);

		if(dlg.m_iBillID != -1) {
			bool bLoadPayArea = false;
			//Is the quote already in the list?
			if(!ReturnsRecords("SELECT BillsT.ID FROM (BillsT INNER JOIN (ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID) ON BillsT.ID = ChargesT.BillID) WHERE BillsT.ID = %li AND BillsT.EntryType = 2 AND LineItemT.PatientID = %li AND (ChargesT.ServiceID IN (SELECT ID FROM ServiceT WHERE ProcedureID IN (SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoID = %li)) OR BillsT.ID IN (SELECT QuoteID FROM ProcInfoQuotesT WHERE ProcInfoID = %li))", dlg.m_iBillID, GetPatientID(), m_nProcInfoID, m_nProcInfoID)) {
				//It isn't, let's add it.
				ExecuteSql("INSERT INTO ProcInfoQuotesT (ProcInfoID, QuoteID) VALUES (%li, %li)", m_nProcInfoID, dlg.m_iBillID);
			}
			//If we didn't have a active quote before, well, we do now!
			if(!ReturnsRecords("SELECT ID FROM ProcInfoT WHERE ID = %li AND ActiveQuoteID Is Not Null", m_nProcInfoID)) {
				ExecuteSql("UPDATE ProcInfoT SET ActiveQuoteID = %li WHERE ID = %li", dlg.m_iBillID, m_nProcInfoID);

				// (j.jones 2005-05-13 09:53) - PLID 16506 - if any PrePayments are attached to the PIC, attach them to the new Active quote
				ExecuteSql("UPDATE PaymentsT SET QuoteID = %li WHERE ID IN (SELECT PayID FROM ProcInfoPaymentsT WHERE ProcInfoID = %li)", dlg.m_iBillID, m_nProcInfoID);

				//TES 10/25/2006 - PLID 22851 - Any payments now applied to this quote (either from the query just above, or because
				// they entered a payment when the CBillingModuleDlg prompted them to), could complete steps on this ladder.
				//TES 8/6/2007 - PLID 20720 - While we're at it, check whether any of these are not currently attached
				// to this PIC, add them if they aren't.
				_RecordsetPtr rsPayments = CreateRecordset("SELECT ID, "
					"CASE WHEN ID IN (SELECT PayID FROM ProcInfoPaymentsT WHERE ProcInfoID = %li) OR ID IN "
					" (SELECT SourceID FROM AppliesT INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID INNER JOIN "
					"   ProcInfoT ON ChargesT.BillID = ProcInfoT. BillID WHERE ProcInfoT.ID = %li) "
					"THEN 1 ELSE 0 END AS IsAttached FROM PaymentsT WHERE QuoteID = %li", 
					m_nProcInfoID, m_nProcInfoID, dlg.m_iBillID);
				CArray<long,long> arUnattachedPays;
				while(!rsPayments->eof) {
					long nID = AdoFldLong(rsPayments, "ID");
					long nIsAttached = AdoFldLong(rsPayments, "IsAttached");
					if(!nIsAttached) {
						arUnattachedPays.Add(nID);
					}
					PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_PaymentApplied, GetPatientID(), COleDateTime::GetCurrentTime(), nID, true, -1);
					rsPayments->MoveNext();
				}
				if(arUnattachedPays.GetSize()) {
					//TES 8/6/2007 - PLID 20720 - We'll need to load the payment area as well as the quote area, since
					// we're adding payments.
					bLoadPayArea = true;
					for(int i = 0; i < arUnattachedPays.GetSize(); i++) {
						ExecuteSql("INSERT INTO ProcInfoPaymentsT (ProcInfoID, PayID) VALUES (%li, %li)",
							m_nProcInfoID, arUnattachedPays[i]);
					}
				}
			}

			//We're refreshing regardless, because the description may have changed.
			Load(PIC_AREA_QUOTE);

			//TES 8/6/2007 - PLID 20720 - If we added prepayments, refresh that area.
			if(bLoadPayArea) {
				Load(PIC_AREA_PAY);
			}
		}

	}NxCatchAll("Error in CProcInfoCenterDlg::OnNewQuotePic()");
}

void CProcInfoCenterDlg::OnAddProc() 
{
	try {
		CAddProcedureDlg dlg(this);
		dlg.m_bAllowProcGroups = false;
		int nReturn = dlg.DoModal();
		if(nReturn == IDOK) {
			if (dlg.m_arProcIDs.GetSize())
			{
				CArray<long,long> arProcIDs;

				for(int i = 0; i < dlg.m_arProcIDs.GetSize(); i++) {
					if(!ReturnsRecordsParam("SELECT ID FROM ProcInfoDetailsT WHERE ProcInfoID = {INT} AND ProcedureID = {INT}", m_nProcInfoID, dlg.m_arProcIDs[i])) {
						arProcIDs.Add(dlg.m_arProcIDs.GetAt(i));
						// (j.armen 2013-06-27 19:14) - PLID 57362 - Idenitate ProcInfoDetailsT
						ExecuteParamSql("INSERT INTO ProcInfoDetailsT (ProcInfoID, ProcedureID) VALUES ({INT}, {INT})", m_nProcInfoID, dlg.m_arProcIDs[i]);
						// (z.manning 2010-02-09 11:53) - PLID 37118 - Don't add inactive detail procedures
						_RecordsetPtr rsDetails = CreateParamRecordset("SELECT ID FROM ProcedureT WHERE Inactive = 0 AND MasterProcedureID = {INT}", dlg.m_arProcIDs[i]);
						while(!rsDetails->eof) {
							// (j.armen 2013-06-27 19:14) - PLID 57362 - Idenitate ProcInfoDetailsT
							ExecuteParamSql("INSERT INTO ProcInfoDetailsT (ProcInfoID, ProcedureID) "
								"VALUES ({INT}, {INT})", m_nProcInfoID, AdoFldLong(rsDetails, "ID"));
							rsDetails->MoveNext();
						}
					}
				}
				// (c.haag 2009-01-14 17:11) - PLID 25578 - Update the procedures of related appointments
				UpdateLinkedPICPurposes(m_nProcInfoID);

				m_pProcNames->Requery();

				GetPicContainer()->HandleNewProcInfoProcedures(arProcIDs);

				//TES 9/21/2005 - Don't forget to start tracking these procedures
				//This will have no effect if the procinfo already has a ladder, which is what we want.
				//TES 2/26/2009 - PLID 33168 - This is now controlled by a preference
				// (j.jones 2011-07-22 14:25) - PLID 38119 - do not do this unless they have the tracking license
				// (b.eyers 2016-05-12) - PLID 67574 - this should only really happen if there isn't already a ladder tied to the pic for the emr
				if (!ReturnsRecordsParam("SELECT ID FROM LaddersT WHERE ProcInfoID = {INT}", m_nProcInfoID)) {
					if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent)) {
						if (GetRemotePropertyInt("PIC_CreateLadderOnProcedureAdded", 1, 0, "<None>")) {
							PhaseTracking::AddLadderToProcInfo(m_nProcInfoID, GetPatientID());
						}
						else {
							//TES 2/26/2009 - PLID 33168 - If we're adding from the EMR, and the preference is off, then we just
							// don't add the ladder.  Here, we give them a choice (but default to No).
							if (IDYES == MsgBox(MB_YESNO | MB_DEFBUTTON2, "Would you like to create a tracking ladder for the procedure(s) you have added?")) {
								PhaseTracking::AddLadderToProcInfo(m_nProcInfoID, GetPatientID());
							}
						}
					}
				}
				
				// (a.walling 2008-05-05 11:11) - PLID 29894 - If we do not have a surgery appt, and the procedures have now changed,
				// update and load to ensure we have a valid one.
				if (m_nSurgApptID == -2 || m_nSurgApptID == -1) {
					if (-1 != GetPicContainer()->UpdateSurgeryAppt()) {
						Load(PIC_AREA_APPT);
					}
				}

				// (c.haag 2007-03-07 09:03) - PLID 21207 - At this point, the procedures have been
				// completely added to the non-clinical tab. We want to be able to add them to the
				// clinical tab as well, so we post a message to our parent to get that ball rolling.
				CArray<int,int>* paProcIDs = new CArray<int,int>;
				CArray<CString,CString>* paProcNames = new CArray<CString,CString>;
				paProcIDs->Copy(dlg.m_arProcIDs);
				paProcNames->Copy(dlg.m_arProcString);
				GetPicContainer()->PostMessage(NXM_NON_CLINICAL_PROCEDURES_ADDED, (WPARAM)paProcIDs, (LPARAM)paProcNames);
			}
		}
	}NxCatchAll("Error in CProcInfoCenterDlg::OnAddProc()");
}

void CProcInfoCenterDlg::OnRemoveProc() 
{
	try {
		if(m_pProcNames->CurSel != -1) {
			//TES 9/29/2004 - It is now legal to have a PIC with no Procedure.
			/*if(m_pProcNames->GetRowCount() == 1) {
				MsgBox("You must track at least one procedure.");
				return;
			}*/
			CString strMessage;
			strMessage.Format("Are you sure you wish to stop tracking this patient's interest in %s?", VarString(m_pProcNames->GetValue(m_pProcNames->CurSel, 1)));
			// (j.jones 2016-02-25 10:54) - PLID 22663 - reverted back to a normal use of MsgBox, which now works fine with strings without parameters
			if (IDYES == MsgBox(MB_YESNO, strMessage)) {

				CArray<long, long> arProcIDs;
				_RecordsetPtr rs = CreateRecordset("SELECT ProcedureID FROM ProcInfoDetailsT WHERE ID = %li OR (ProcedureID IN (SELECT ID FROM ProcedureT WHERE MasterProcedureID = (SELECT ProcedureID FROM ProcInfoDetailsT WHERE ID = %li)) AND ProcInfoID = %li)", VarLong(m_pProcNames->GetValue(m_pProcNames->CurSel, 0)), VarLong(m_pProcNames->GetValue(m_pProcNames->CurSel, 0)), m_nProcInfoID);
				while(!rs->eof) {
					arProcIDs.Add(AdoFldLong(rs, "ProcedureID"));
					rs->MoveNext();
				}
				rs->Close();

				GetPicContainer()->HandleDeletingProcInfoProcedures(arProcIDs);

				ExecuteSql("DELETE FROM ProcInfoDetailsT WHERE ID = %li OR (ProcedureID IN (SELECT ID FROM ProcedureT WHERE MasterProcedureID = (SELECT ProcedureID FROM ProcInfoDetailsT WHERE ID = %li)) AND ProcInfoID = %li)", VarLong(m_pProcNames->GetValue(m_pProcNames->CurSel, 0)), VarLong(m_pProcNames->GetValue(m_pProcNames->CurSel, 0)), m_nProcInfoID);
				long AuditID = BeginNewAuditEvent();
				if(AuditID != -1) {
					AuditEvent(GetPatientID(), GetPatientName(), AuditID, aeiPatientProcedureRemoved, GetPatientID(), VarString(m_pProcNames->GetValue(m_pProcNames->CurSel, 1)), "", aepMedium, aetDeleted);
				}
				// (c.haag 2009-01-14 17:11) - PLID 25578 - Update the procedures of related appointments
				UpdateLinkedPICPurposes(m_nProcInfoID);

				m_pProcNames->Requery();
			}
		}
	}NxCatchAll("Error in CProcInfoCenterDlg::OnRemoveProc()");
}

void CProcInfoCenterDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CNxDialog::OnCancel();
}

void CProcInfoCenterDlg::OnKillFocusArrivalHr() 
{
	Save(IDC_ARRIVAL_HR);
}

void CProcInfoCenterDlg::OnDragBeginOtherAppts(BOOL FAR* pbShowDrag, long nRow, short nCol, long nFlags) 
{
	try {
		//This is a valid place to begin dragging if it is a valid appointment row.
		//This will be true iff there is a valid ID in column 0.
		if(nRow != -1) {
			if(m_pApptList->GetValue(nRow, alcID).vt != VT_I4 || VarLong(m_pApptList->GetValue(nRow, alcID),-1) == -1) {
				//This is like a separator or something.
				*pbShowDrag = false;
			}
			else {
				//Set this row to the drag-begin color
				IRowSettingsPtr pRow = m_pApptList->GetRow(nRow);
				pRow->BackColor = DRAG_COLOR;
				pRow->BackColorDragFrom = DRAG_COLOR;
				pRow->BackColorDragTo = DRAG_COLOR;
				pRow->ForeColorDragFrom = pRow->ForeColorDragFrom == dlColorNotSet ? RGB(255,255,255) : pRow->ForeColorDragFrom;
				pRow->ForeColor = pRow->ForeColorDragFrom;
				pRow->ForeColorDragTo = pRow->ForeColorDragFrom;
				
			}
		}
		else {
			*pbShowDrag = false;
		}
	}NxCatchAll("Error in CProcInfoCenterDlg::OnDragBeginOtherAppts()");
}

void CProcInfoCenterDlg::OnDragOverCellOtherAppts(BOOL FAR* pbShowDrop, long nRow, short nCol, long nFromRow, short nFromCol, long nFlags) 
{
	try {
		//First, if we didn't start from a valid start row, then show nothing.
		if(nFromRow == -1 || m_pApptList->GetValue(nFromRow, alcID).vt != VT_I4 || VarLong(m_pApptList->GetValue(nFromRow, alcID), -1) == -1) {
			*pbShowDrop = false;
		}
		else {
			//OK, the start row was valid.  How about this row?
			if(nFromRow == nRow) {
				//We're on the same row, don't change a thing.
			}
			else {
				if(nRow == -1) {
					*pbShowDrop = false;
				}
				else if(nRow == SURGERY_ROW) {
					//We're on the surgery row, and we started from a different row that was valid.
					//Therefore, it must have been another appointment, so show the appropriate color.
					IRowSettingsPtr pRow = m_pApptList->GetRow(nRow);
					pRow->BackColor = GetLighterColor(DRAG_COLOR);
					pRow->BackColorDragTo = GetLighterColor(DRAG_COLOR);
					pRow->ForeColor = RGB(0,0,0);
				}
				else if(m_pApptList->GetValue(nRow, alcID).vt != VT_I4) {
					//This is not a valid row, show the appropriate color.
					IRowSettingsPtr pRow = m_pApptList->GetRow(nRow);
					pRow->BackColor = SKIP_COLOR;
					pRow->BackColorDragTo = SKIP_COLOR;
					pRow->ForeColor = RGB(0,0,0);
				}
				else if(nFromRow == SURGERY_ROW) {
					//We started from the surgery row, and now we're on a different (but valid row) (including possibly the double-secret row).
					//Therefore, this must be a valid dest for the surgery row, so show the appropriate color.
					IRowSettingsPtr pRow = m_pApptList->GetRow(nRow);
					pRow->BackColor = GetLighterColor(DRAG_COLOR);
					pRow->BackColorDragTo = GetLighterColor(DRAG_COLOR);
					pRow->ForeColor = RGB(0,0,0);
				}
				else {
					//Well, this is an invalid drag/drop combination, show the appropriate color.
					IRowSettingsPtr pRow = m_pApptList->GetRow(nRow);
					pRow->BackColor = SKIP_COLOR;
					pRow->BackColorDragTo = SKIP_COLOR;
					pRow->ForeColor = RGB(0,0,0);
				}
			}
		}

		//OK, we've colored this row appropriately, let's set the background colors of all other rows 
		//to be white.
		for(int i = 0; i < m_pApptList->GetRowCount(); i++) {
			if(i != nRow && i != nFromRow) {
				IRowSettingsPtr pRow = m_pApptList->GetRow(i);
				pRow->BackColor = RGB(255,255,255);
				if(pRow->GetValue(alcColor).vt == VT_I4) {
					pRow->ForeColor = VarLong(pRow->GetValue(alcColor));
				}
				else pRow->ForeColor = RGB(0,0,0);
			}
		}
	}NxCatchAll("Error in CProcInfoCenterDlg::OnDragOverOtherCellAppts()");

}

void CProcInfoCenterDlg::OnDragEndOtherAppts(long nRow, short nCol, long nFromRow, short nFromCol, long nFlags) 
{
	try {
		//This is a valid drag/drop combination if exactly one of nRow or nFromRow is the surgery title row.
		if((nRow == SURGERY_ROW || nFromRow == SURGERY_ROW) && nRow != nFromRow) {
			//OK, let's get dragging.  First, are we dragging from or to the surgery.
			if(nFromRow == SURGERY_ROW) {
				//First, let's double-check that the "other" appt we're dragging to is a valid appt (or the double-secret row).
				if(nRow != -1 && m_pApptList->GetValue(nRow, alcID).vt == VT_I4) {
					//OK, we're demoting the surgery.  First let's check if it's even valid.
					if(m_pApptList->GetValue(SURGERY_ROW, alcID).vt == VT_I4) {
						//OK, we're good so far.  Let's make sure they want to do this.
						if(IDYES == MsgBox(MB_YESNO, "Are you sure you want to remove this record as the surgery for this procedure?")) {
							//OK, they're sure.
							ExecuteParamSql("UPDATE ProcInfoT SET SurgeryApptID = NULL WHERE ID = {INT}", m_nProcInfoID);
							m_nxtArrivalTime->Clear();
							m_nxtArrivalTime->Enabled = VARIANT_FALSE;
							Load(PIC_AREA_APPT);
						}
					}
				}
			}
			else {
				//First, let's double-check that the "other" appt we're dragging is a valid appt.
				if(m_pApptList->GetValue(nFromRow, alcID).vt == VT_I4 && VarLong(m_pApptList->GetValue(nFromRow, alcID), -1) != -1) {
					//OK, we're promoting a non-surgery appointment. First, is there a surgery already?
					if(m_pApptList->GetValue(SURGERY_ROW, alcID).vt == VT_I4) {
						//Double-check
						if(IDYES != MsgBox(MB_YESNO, "Are you sure you want to replace the existing surgery appointment?")) {
							SetApptColors();
							return;
						}
					}
					//We're good so far.  Let's see if this is a surgery or minor procedure
					_RecordsetPtr rsCategory = CreateParamRecordset("SELECT Category FROM AptTypeT WHERE ID = (SELECT AptTypeID FROM AppointmentsT WHERE ID = {INT})", VarLong(m_pApptList->GetValue(nFromRow, alcID)));
					CString strCategoryName;
					if(rsCategory->eof) {
						strCategoryName = "<None>";
					}
					else {
						switch(AdoFldByte(rsCategory, "Category")) {
						case 0:
							strCategoryName = "Non Procedural";
							break;
						case 1:
							strCategoryName = "Consult";
							break;
						case 2:
							strCategoryName = "PreOp";
							break;
						case 3:
							strCategoryName = "Minor Procedure";
							break;
						case 4:
							strCategoryName = "Surgery";
							break;
						case 5:
							strCategoryName = "Follow-up";
							break;
						case 6:
							strCategoryName = "Other Procedure";
							break;
						case 7:
							strCategoryName = "Block Time";
							break;
						default:
							//Should never happen.,
							strCategoryName = "<None>";
						}
					}
					if(strCategoryName != "Surgery" && strCategoryName != "Minor Procedure" && strCategoryName != "Other Procedure") {
						//Let's double-check.
						if(IDYES != MsgBox(MB_YESNO, "The appointment you are setting as the surgery for this procedure has a category of '%s'.\nIt is not recommended that appointments with this category be set as the surgery for a procedure.  Are you sure you wish to proceed?", strCategoryName)) {
							SetApptColors();
							return;
						}
					}
					//Well, we've made it this far.  Let's do it.
					long nSurgeryApptID = VarLong(m_pApptList->GetValue(nFromRow, alcID), -1);
					ExecuteParamSql("UPDATE ProcInfoT SET SurgeryApptID = {INT} WHERE ID = {INT}", nSurgeryApptID, m_nProcInfoID);
					//Clear out the arrival time; the load function will fill it back in
					m_nxtArrivalTime->Clear();
					m_nxtArrivalTime->Enabled = VARIANT_FALSE;
					//TES 11/25/2008 - PLID 32066 - Check whether the ProcInfo's surgeon should be updated based on this
					// new surgery.
					PhaseTracking::UpdateProcInfoSurgeon(m_nProcInfoID, nSurgeryApptID);
					Load(PIC_AREA_ALL);
				}
			}
		}


		//Reset all the colors
		SetApptColors();
	}NxCatchAll("Error in CProcInfoCenterDlg::OnDragEndOtherAppts()");
}

void CProcInfoCenterDlg::SetApptColors()
{
	IRowSettingsPtr pRow;
	//First row: surgery title row.  BackColor is white, ForeColor is black
	pRow = m_pApptList->GetRow(SURGERY_TITLE_ROW);
	if(pRow == NULL) {
		return;//The list must not be set up properly yet.
	}
	pRow->BackColor = RGB(255,255,255);
	pRow->ForeColor = RGB(0,0,0);

	//Second row: actual surgery row. BackColor is white, ForeColor is black for <None>, otherwise based on type
	pRow = m_pApptList->GetRow(SURGERY_ROW);
	if(pRow == NULL) return;//The list must not be set up properly yet.
	pRow->BackColor = RGB(255,255,255);
	if(pRow->GetValue(alcID).vt == VT_I4) {
		pRow->ForeColor = VarLong(pRow->GetValue(alcColor), RGB(0,0,0));
	}
	else pRow->ForeColor = RGB(0,0,0);

	//Third row: separator row.  BackColor is white, ForeColor is black (but it doesn't matter).
	pRow = m_pApptList->GetRow(SEPARATOR_ROW);
	if(pRow == NULL) {
		return;//The list must not be set up properly yet.
	}
	pRow->BackColor = RGB(255,255,255);
	pRow->ForeColor = RGB(0,0,0);

	//Fourth row: consult title row.  BackColor is white, ForeColor is black
	pRow = m_pApptList->GetRow(OTHER_TITLE_ROW);
	if(pRow == NULL) {
		return;//The list must not be set up properly yet.
	}
	pRow->BackColor = RGB(255,255,255);
	pRow->ForeColor = RGB(0,0,0);

	//All "other" rows: appointment rows.  BackColor is white, ForeColor is based on type.
	for(int i = 4; i < m_pApptList->GetRowCount()-1; i++) {
		pRow = m_pApptList->GetRow(i);
		pRow->BackColor = RGB(255,255,255);
		pRow->ForeColor = VarLong(pRow->GetValue(alcColor), RGB(0,0,0));
	}

	//Double-secret other row.  BackColor is white, ForeColor is black (but it doesn't matter).
	pRow = m_pApptList->GetRow(m_pApptList->GetRowCount()-1);
	if(pRow == NULL) {
		return;//The list must not be set up properly yet.
	}
	pRow->BackColor = RGB(255,255,255);
	pRow->ForeColor = RGB(0,0,0);
}

void CProcInfoCenterDlg::OnAnesthesiaConfig() 
{
	try {
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 9, m_pAnesthesiaList, "Edit Combo Box").DoModal();
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::OnSelChosenAnesthesiaList(long nRow) 
{
	try {
		if(nRow != -1) {
			SetDlgItemText(IDC_ANESTHESIA, VarString(m_pAnesthesiaList->GetValue(nRow, 0)));
			GetDlgItem(IDC_ANESTHESIA)->SetFocus();
		}
	}NxCatchAll("Error in CProcInfoCenterDlg::OnSelChoseAnesthesiaList()");
}

void CProcInfoCenterDlg::OnEditingFinishedProcNameListDetail(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if(bCommit) {
		if(nRow != -1) {
			if(nCol == 1) {
				try {
					CString strNewVal = VarBool(varNewValue) ? "1" : "0";
					ExecuteSql("UPDATE ProcInfoDetailsT SET Chosen = %s WHERE ID = %li", strNewVal, VarLong(m_pProcDetailNames->GetValue(nRow, 0)));
					// (c.haag 2009-01-14 17:11) - PLID 25578 - Update the procedures of related appointments
					UpdateLinkedPICPurposes(m_nProcInfoID);
				}NxCatchAll("Error in CProcInfoCenterDlg::OnEditingFinishedProcNameListDetail()");
			}
		}
	}

}

void CProcInfoCenterDlg::OnSelChangedProcNameList(long nNewSel) 
{
	try {
		LoadProcSpecific();
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::OnRemoveProcDetail() 
{
	try {
		if(m_pProcDetailNames->CurSel != -1) {
			if(IDYES == MsgBox(MB_YESNO, "Are you sure you wish to stop tracking this detail procedure for this patient?")) {

				ExecuteSql("DELETE FROM ProcInfoDetailsT WHERE ID = %li", VarLong(m_pProcDetailNames->GetValue(m_pProcDetailNames->CurSel, 0)));
				long AuditID = BeginNewAuditEvent();
				if(AuditID != -1) {
					AuditEvent(GetPatientID(), GetPatientName(), AuditID, aeiPatientProcedureRemoved, GetPatientID(), VarString(m_pProcDetailNames->GetValue(m_pProcDetailNames->CurSel, 2)), "", aepMedium, aetDeleted);
				}
				// (c.haag 2009-01-14 17:11) - PLID 25578 - Update the procedures of related appointments
				UpdateLinkedPICPurposes(m_nProcInfoID);

				LoadProcSpecific();
			}
		}
	}NxCatchAll("Error in CProcInfoCenterDlg::OnRemoveProcDetail()");
}

void CProcInfoCenterDlg::OnAddProcDetail() 
{
	try {
		if(m_pProcNames->CurSel != -1) {
			CString strWhereClause;
			// (c.haag 2008-12-18 10:30) - PLID 32264 - Filter out inactive procedures
			strWhereClause.Format("ProcedureT.Inactive = 0 AND MasterProcedureID = %li AND ProcedureT.ID NOT IN (SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoID = %li)", VarLong(m_pProcNames->GetValue(m_pProcNames->CurSel, 2)), m_nProcInfoID);
			if(!ReturnsRecords("SELECT ID FROM ProcedureT WHERE %s", strWhereClause)) {
				MsgBox("You are already tracking all available details for this procedure.");
				return;
			}

			CAddProcedureDlg dlg(this);
			dlg.m_strProcWhereClause = strWhereClause;
			dlg.m_bAllowProcGroups = false;
			dlg.DoModal();

			for(int i = 0; i < dlg.m_arProcIDs.GetSize(); i++) {
				// (j.armen 2013-06-27 19:14) - PLID 57362 - Idenitate ProcInfoDetailsT
				ExecuteParamSql("INSERT INTO ProcInfoDetailsT (ProcInfoID, ProcedureID) "
					"VALUES ({INT}, {INT})", m_nProcInfoID, dlg.m_arProcIDs[i]);
			}
			// (c.haag 2009-01-14 17:11) - PLID 25578 - Update the procedures of related appointments
			UpdateLinkedPICPurposes(m_nProcInfoID);

			// (a.walling 2008-05-05 11:11) - PLID 29894 - If we do not have a surgery appt, and the procedures have now changed,
			// update and load to ensure we have a valid one.
			if (m_nSurgApptID == -2 || m_nSurgApptID == -1) {
				if (-1 != GetPicContainer()->UpdateSurgeryAppt()) {
					Load(PIC_AREA_APPT);
				}
			}

			LoadProcSpecific();
		}
	}NxCatchAll("Error in CProcInfoCenterDlg::OnAddProcDetail()");

}

void CProcInfoCenterDlg::OnPreviewPic() 
{
	try {
		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(396)]);
		infReport.nExtraID = m_nProcInfoID;

		MsgBox("The PIC will be saved and closed before previewing.");
		
		CRParameterInfo *paramInfo;
		CPtrArray paParams;

		paramInfo = new CRParameterInfo;
		COleDateTime dt;
		dt.ParseDateTime("01/01/1000");
		paramInfo->m_Data = FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS);
		paramInfo->m_Name = "DateFrom";
		paParams.Add((void *)paramInfo);

		paramInfo = new CRParameterInfo;
		dt.ParseDateTime("12/31/5000");
		paramInfo->m_Data = FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS);
		paramInfo->m_Name = "DateTo";
		paParams.Add((void *)paramInfo);

		infReport.strReportFile += "dtld";

		//Made new function for running reports - JMM 5-28-04
		RunReport(&infReport, &paParams, true, this, "PIC Report");

		ClearRPIParameterList(&paParams);	//DRT - PLID 18085 - Cleanup after ourselves

		if(GetPicContainer()) {
			GetPicContainer()->SendMessage(NXM_CLOSE_PIC, cprNone);
		}
		else {
			OnOK();
		}
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::OnLButtonDownOtherAppts(long nRow, long nCol, long x, long y, long nFlags) 
{
	try {
		if(!m_bLButtonDownHandled) {
			//We only want to handle this once.
			m_bLButtonDownHandled = true;
			if(nRow != -1 && m_pApptList->GetValue(nRow, alcID).vt == VT_I4 && VarLong(m_pApptList->GetValue(nRow, alcID),-1) != -1) {
				//Set this row to the drag-begin color
				IRowSettingsPtr pRow = m_pApptList->GetRow(nRow);
				pRow->BackColor = DRAG_COLOR;
				pRow->BackColorDragFrom = DRAG_COLOR;
				pRow->BackColorDragTo = DRAG_COLOR;
				pRow->ForeColorDragFrom = pRow->ForeColorDragFrom == dlColorNotSet ? RGB(255,255,255) : pRow->ForeColorDragFrom;
				pRow->ForeColor = pRow->ForeColorDragFrom;
				pRow->ForeColorDragTo = pRow->ForeColorDragFrom;
			}
		}
	}NxCatchAll("Error in CProcInfoCenterDlg::OnLButtonDownOtherAppts()");
}

void CProcInfoCenterDlg::OnLButtonUpOtherAppts(long nRow, long nCol, long x, long y, long nFlags)
{
	try {
		if(m_bLButtonDownHandled) {
			//Reset the colors.
			SetApptColors();
			//We're ready for another lbutton down.
			m_bLButtonDownHandled = false;
		}
	}NxCatchAll("Error in CProcInfoCenterDlg::OnLButtonUpOtherAppts()");
}

void CProcInfoCenterDlg::ApplyPrePaysToBill(long iBillID)
{
	//TES 2/25/2004: Now, if they have any prepayments, they should probably be applied.
	//TES 6/28/2004: But only if there is a balance for the responsibility of the prepayment.
	_RecordsetPtr rsPrePayIds = CreateRecordset("SELECT LineItemT.ID, LineItemT.Type "
		"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"WHERE LineItemT.Deleted = 0 AND PaymentsT.Prepayment = 1 AND PaymentsT.ID IN "
		"(SELECT PayID FROM ProcInfoPaymentsT WHERE ProcInfoID = %li) AND PaymentsT.ID NOT IN "
		"(SELECT SourceID FROM AppliesT)", m_nProcInfoID);
	bool bPrompted = false;
	if(!rsPrePayIds->eof) {
		CWaitCursor cuWait;
		while(!rsPrePayIds->eof) {
			bool bCanApply = true;
			/*******************************************************************************************
			/* Begin code copied off of OnDragEndFinancialList
			/*******************************************************************************************/
			CFinancialApply dlg(this);
			long nPayID = AdoFldLong(rsPrePayIds, "ID");
			long nPayType = AdoFldLong(rsPrePayIds, "Type");
			//What's the resp type on the payment?
			_RecordsetPtr rsRespType = CreateRecordset("SELECT RespTypeID, PaymentsT.InsuredPartyID "
				"FROM PaymentsT LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				"WHERE PaymentsT.ID = %li", nPayID);
			long nInsPartyID = AdoFldLong(rsRespType, "InsuredPartyID");
			dlg.m_nResponsibility = AdoFldLong(rsRespType, "RespTypeID", 0);
			rsRespType->Close();

			//Get some totals
			COleCurrency cyCharges, cyPayments, cyAdjustments, cyRefunds, cyInsurance, cyAmount, cyOutgoing, cyIncoming;
			if(dlg.m_nResponsibility == 0) {
				//Patient resp
				if (!GetBillTotals(iBillID, GetPatientID(), &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
					bCanApply = false;
					rsPrePayIds->MoveNext();
					continue;
				}
				dlg.m_cyNetCharges = cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance;				
			}
			else if(dlg.m_nResponsibility == -1) {
				//inactive resp
				GetInactiveInsTotals(nInsPartyID, iBillID, -1, GetPatientID(), cyCharges, cyPayments);
				dlg.m_cyNetCharges = cyCharges - cyPayments;
			}
			else {
				//all other resp
				// (j.jones 2011-07-22 12:19) - PLID 42231 - GetBillInsuranceTotals now takes in an insured party ID, not a RespTypeID
				if (!GetBillInsuranceTotals(iBillID, GetPatientID(), nInsPartyID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds)) {
					bCanApply = false;
					rsPrePayIds->MoveNext();
					continue;
				}
				// cyCharges is the total insurance responsbility, less applies
				dlg.m_cyNetCharges = cyCharges - cyPayments - cyAdjustments - cyRefunds;
			}

			dlg.m_boShowIncreaseCheck = FALSE;
			dlg.m_boShowAdjCheck = TRUE;

			//Gather information about our payment.
			if (!GetPayAdjRefTotals(nPayID, GetPatientID(), cyAmount, cyOutgoing, cyIncoming)) {
				bCanApply = false;
				rsPrePayIds->MoveNext();
				continue;
			}
			dlg.m_cyNetPayment = cyAmount - cyOutgoing + cyIncoming;

			if (nPayType == 1 && dlg.m_cyNetPayment < COleCurrency(0,0)) 
			{	
				bCanApply = false;
				rsPrePayIds->MoveNext();
				continue;
			}

			if(IsZeroDollarPayment(nPayID))
				dlg.m_boZeroAmountAllowed = TRUE;

			//stop people from applying to zero balances, or applying payments of wrong responsibility
			if (nPayType == 1 && (dlg.m_cyNetCharges == COleCurrency(0,0) && !dlg.m_boZeroAmountAllowed)) 
			{	
				bCanApply = false;
				rsPrePayIds->MoveNext();
				continue;
			}

			//adjustments have different properties, so the FinancialApplyDlg must know if it is an adjustment
			if(nPayType == 2)
				dlg.m_boIsAdjustment = TRUE;

			if(bCanApply) {
				//OK, now, have we prompted them yet?
				if(!bPrompted) {
					if(IDYES != MsgBox(MB_YESNO, "You have unapplied prepayments associated with this procedure.  Would you like to apply them to this bill?")) {
						return;
					}
					bPrompted = true;
				}
				//Now, go ahead and apply.

				// (j.jones 2011-08-25 09:30) - PLID 45176 - CanApplyLineItem will check the normal apply create permission,
				// but only after it checks to see if the source payment is closed. This will be somewhat annoying
				// if we apply several payments, but the odds are that they will not have more than one prepayment,
				// and most people will have permission anyways.
				// (j.jones 2013-07-01 10:45) - PLID 55517 - this function can now potentially correct the payment, if so
				// the payment ID will be changed to reflect the new, corrected line item
				if(!CanApplyLineItem(nPayID)) {
					rsPrePayIds->MoveNext();
					continue;
				}
			
				if (!(IDCANCEL == dlg.DoModal() || (!dlg.m_boZeroAmountAllowed && dlg.m_cyApplyAmount == COleCurrency(0,0)))) {
					if (dlg.m_nResponsibility == 0)
						ApplyPayToBill(nPayID, GetPatientID(), dlg.m_cyApplyAmount, "Bill", iBillID);
					else {
						//(e.lally 2007-03-30) PLID 25263 - Switched this to the new general apply function
						ApplyPayToBill(nPayID, GetPatientID(), dlg.m_cyApplyAmount, "Bill", iBillID, nInsPartyID, FALSE, dlg.m_boShiftBalance, dlg.m_boAdjustBalance, FALSE, TRUE, FALSE, TRUE, FALSE);
					}

					if (dlg.m_boAdjustBalance) {
						CPaymentDlg paydlg(this);
						paydlg.m_PatientID = GetPatientID(); // (c.haag 2007-03-14 10:24) - PLID 25110 - Use the PIC patient ID
						paydlg.m_iDefaultPaymentType = 1;
						paydlg.m_cyFinalAmount = AdjustBalance(iBillID,iBillID,GetPatientID(), 1,dlg.m_nResponsibility,nInsPartyID);
						if(paydlg.m_cyFinalAmount.GetStatus()==COleCurrency::invalid) {
							MsgBox("This bill has been deleted (possibly by another user). Practice will now requery this patient's financial information.");
							rsPrePayIds->MoveNext();
							continue;
						}
						paydlg.m_varBillID = (long)iBillID;
						paydlg.m_ApplyOnOK = TRUE;
						paydlg.m_PromptToShift = FALSE;
						// (j.jones 2008-04-29 10:29) - PLID 29744 - this didn't support inactive insurance
						if(dlg.m_nResponsibility != 0 && nInsPartyID > 0) {
							paydlg.m_iDefaultInsuranceCo = nInsPartyID;
						}
						paydlg.DoModal(__FUNCTION__, __LINE__);
					}

					if (dlg.m_nResponsibility != 0) {
						// See if we need to shift the remaining balance to the 
						// patient in the case of an insurance apply.
						//JMJ - 7/24/2003 - only do this if there is still a balance
						if (dlg.m_boShiftBalance && -AdjustBalance(iBillID,iBillID,GetPatientID(),1,dlg.m_nResponsibility,nInsPartyID) > COleCurrency(0,0)) {
							try {
								// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string
								ShiftInsBalance(iBillID, GetPatientID(), GetInsuranceIDFromType(GetPatientID(), dlg.m_nResponsibility), GetInsuranceIDFromType(GetPatientID(), dlg.m_nShiftToResp), "Bill",
									"after applying a prepayment to a bill from the PIC");
							}NxCatchAll("Error in ShiftInsBalance");
						}
					}

					// (j.jones 2011-03-23 17:28) - PLID 42936 - now we have to check the allowable for what we applied
					_RecordsetPtr rsAppliedTo = CreateParamRecordset("SELECT ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
						"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location AS POSID, "
						"InsuredPartyT.PersonID, ChargesT.ID, Sum(AppliesT.Amount) AS Amount "
						"FROM LineItemT "
						"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
						"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
						"INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
						"INNER JOIN ChargeRespT ON AppliesT.RespID = ChargeRespT.ID "
						"INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
						"WHERE LineItemT.Deleted = 0 AND AppliesT.SourceID = {INT} AND BillsT.ID = {INT} "
						"GROUP BY ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
						"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location, "
						"InsuredPartyT.PersonID, ChargesT.ID", nPayID, iBillID);
					while(!rsAppliedTo->eof) {
						//WarnAllowedAmount takes in the ServiceID, InsCoID, ProviderID, ChargeID, and the dollar amount we applied
						WarnAllowedAmount(AdoFldLong(rsAppliedTo, "ServiceID", -1), AdoFldLong(rsAppliedTo, "InsuranceCoID", -1),
							AdoFldLong(rsAppliedTo, "DoctorsProviders", -1), AdoFldLong(rsAppliedTo, "LocationID", -1),
							AdoFldLong(rsAppliedTo, "POSID", -1), AdoFldLong(rsAppliedTo, "PersonID", -1),
							AdoFldLong(rsAppliedTo, "ID"), AdoFldCurrency(rsAppliedTo, "Amount"));

						//if we underpaid multiple charges, this will cause multiple prompts (this is not typical)
						rsAppliedTo->MoveNext();
					}
					rsAppliedTo->Close();

					//we may need to unbatch the claim
					CheckUnbatchClaim(iBillID);
				}							
				
				/*******************************************************************************************
				/* End code copied off of OnDragEndFinancialList
				/*******************************************************************************************/
			}
			rsPrePayIds->MoveNext();

		}
	}
}

LRESULT CProcInfoCenterDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if(message == NXM_QUOTE_PREVIEWING) {
		if(GetPicContainer()) {
			GetPicContainer()->PostMessage(NXM_CLOSE_PIC, cprNone);
		}
		else {
			OnOK();
		}
	} else if (message == WM_DESTROY) {
		// (a.walling 2008-06-11 09:35) - PLID 30351 - Free resource
		delete m_pFont;
		m_pFont = NULL;
	}

	return CPatientDialog::WindowProc(message, wParam, lParam);
}

BOOL CProcInfoCenterDlg::CheckWarnPersonLicenses(long nPersonID, CString strPersonType)
{
	try {

		//if they don't want to warn, then leave
		if(GetRemotePropertyInt("ExpLicenseWarnPIC",0,0,"<None>",true) == 0)
			return TRUE;

		ECredentialWarning eCredWarning = CheckPersonCertifications(nPersonID);

		if(eCredWarning != ePassedAll) {

			CString str;

			CString strName = "";
			_RecordsetPtr rs = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = %li",nPersonID);
			if(!rs->eof) {
				strName = AdoFldString(rs, "Name","");
				strName.TrimRight();
			}
			rs->Close();

			if(eCredWarning == eFailedLicenseExpired) {

				CString strLicenses;
				_RecordsetPtr rs2 = CreateRecordset("SELECT '''' + Name + ''' - Expired: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
					"WHERE PersonID = %li AND ExpDate < Convert(datetime,(Convert(nvarchar,GetDate(),1)))",nPersonID);
				while(!rs2->eof) {
					strLicenses += AdoFldString(rs2, "ExpiredLicense","");
					strLicenses += "\n";
					rs2->MoveNext();
				}
				rs2->Close();

				str.Format("The selected %s (%s) has the following expired licenses:\n\n%s\n"
					"Do you still wish to use this %s?",strPersonType,strName,strLicenses,strPersonType);
			}
			else if(eCredWarning == eFailedLicenseExpiringSoon) {

				//check if a license will expire within the given day range
				long nLicenseWarnDayRange = GetRemotePropertyInt("DefaultASCLicenseWarnDayRange",30,0,"<None>",true);

				CString strLicenses;
				_RecordsetPtr rs2 = CreateRecordset("SELECT '''' + Name + ''' - Expires on: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
					"WHERE PersonID = %li AND ExpDate < DateAdd(day,%li,Convert(datetime,(Convert(nvarchar,GetDate(),1))))",nPersonID,nLicenseWarnDayRange);
				while(!rs2->eof) {
					strLicenses += AdoFldString(rs2, "ExpiredLicense","");
					strLicenses += "\n";
					rs2->MoveNext();
				}
				rs2->Close();

				str.Format("The following licenses are about to expire for the selected %s (%s):\n\n%s\n"
					"Do you still wish to use this %s?",strPersonType,strName,strLicenses,strPersonType);
			}

			if(IDNO == MsgBox(MB_YESNO,str)) {
				return FALSE;
			}
		}

	}NxCatchAll("Error validating licensing.");

	return TRUE;
}

bool CProcInfoCenterDlg::IsEmpty()
{
	return (m_pProcNames->GetRowCount() == 0)?true:false;
}

void CProcInfoCenterDlg::OnSize(UINT nType, int cx, int cy) 
{
	CPatientDialog::OnSize(nType, cx, cy);
	try {
		SetControlPositions();
		// (a.walling 2008-06-11 09:21) - PLID 30099 - Send ALLCHILDREN
		#ifndef NXDIALOG_NOCLIPCHILDEN
				RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
		#else
				RedrawWindow();
		#endif
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::OnTrySetSelFinishedNurse(long nRowEnum, long nFlags) 
{
	try {
		if(nFlags == dlTrySetSelFinishedFailure) {
			//they may have an inactive nurse
			_RecordsetPtr rsNurse = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT "
				"WHERE ID = (SELECT ProcInfoT.NurseID FROM ProcInfoT WHERE ProcInfoT.ID = %li)", m_nProcInfoID);
			if(!rsNurse->eof) {
				m_pNurse->PutComboBoxText(_bstr_t(AdoFldString(rsNurse, "Name", "")));
			}
			else 
				m_pNurse->PutCurSel(-1);
		}
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::OnTrySetSelFinishedAnesthesiologistList(long nRowEnum, long nFlags) 
{
	try {
		if(nFlags == dlTrySetSelFinishedFailure) {
			//they may have an inactive anesthesiologist
			_RecordsetPtr rsAnesth = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT "
				"WHERE ID = (SELECT ProcInfoT.AnesthesiologistID FROM ProcInfoT WHERE ProcInfoT.ID = %li)", m_nProcInfoID);
			if(!rsAnesth->eof) {
				m_pAnesthesiologist->PutComboBoxText(_bstr_t(AdoFldString(rsAnesth, "Name", "")));
			}
			else 
				m_pAnesthesiologist->PutCurSel(-1);
		}
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::OnTrySetSelFinishedSurgeon(long nRowEnum, long nFlags) 
{
	try {
		if(nFlags == dlTrySetSelFinishedFailure) {
			//they may have an inactive surgeon
			_RecordsetPtr rsSurgeon = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT "
				"WHERE ID = (SELECT ProcInfoT.SurgeonID FROM ProcInfoT WHERE ProcInfoT.ID = %li)", m_nProcInfoID);
			if(!rsSurgeon->eof) {
				m_pSurgeon->PutComboBoxText(_bstr_t(AdoFldString(rsSurgeon, "Name", "")));
			}
			else 
				m_pSurgeon->PutCurSel(-1);
		}
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::OnSelChosenCoSurgeon(long nRow) 
{
	// (a.walling 2006-11-28 12:41) - PLID 21003 - Add a co-surgeon field
	if(nRow != -1) {
		Save(IDC_COSURGEON);
	}	
}

long CProcInfoCenterDlg::GetPatientID() const
{
	// (c.haag 2007-03-07 16:16) - PLID 25110 - Returns the patient ID
	// (j.jones 2016-02-12 13:02) - PLID 68073 - improved the error message
	if (NULL == GetPicContainer()) ThrowNxException("CProcInfoCenterDlg::GetPatientID tried to get the patient ID from invalid PIC dialog!");
	return GetPicContainer()->GetPatientID();
}

CString CProcInfoCenterDlg::GetPatientName() const
{
	// (c.haag 2007-03-07 16:16) - PLID 25110 - Returns the name of the active patient
	// (j.jones 2016-02-12 13:02) - PLID 68073 - improved the error message
	if (NULL == GetPicContainer()) ThrowNxException("CProcInfoCenterDlg::GetPatientName tried to get the patient name from invalid PIC dialog!");
	return GetPicContainer()->GetPatientName();
}

void CProcInfoCenterDlg::OnBnClickedCosurgeonRefphys()
{
	try
	{
		// (z.manning 2008-11-19 11:54) - PLID 31687 - Save the checkbox's state and requery the co-surgeon list
		SetRemotePropertyInt("PICCoSurgeonIncludeRefPhys", m_nxbtnCoSurgeonRefPhys.GetCheck(), 0, "<None>");
		RequeryCoSurgeonCombo();

	}NxCatchAll("CProcInfoCenterDlg::OnBnClickedCosurgeonRefphys");
}

void CProcInfoCenterDlg::OnBnClickedCosurgeonProvider()
{
	try
	{
		// (z.manning 2008-11-19 11:54) - PLID 31687 - Save the checkbox's state and requery the co-surgeon list
		SetRemotePropertyInt("PICCoSurgeonIncludeProvider", m_nxbtnCoSurgeonProvider.GetCheck(), 0, "<None>");
		RequeryCoSurgeonCombo();

	}NxCatchAll("CProcInfoCenterDlg::OnBnClickedCosurgeonProvider");
}

// (z.manning 2008-11-19 10:05) - PLID 31687 - Function to requey the co-surgeon combo now that
// it can also include referrying physicians.
void CProcInfoCenterDlg::RequeryCoSurgeonCombo()
{
	_bstr_t bstrWhere;
	const _bstr_t bstrEmpty;
	short nPersonTypeCount = 0;

	if(m_nxbtnCoSurgeonProvider.GetCheck() == BST_CHECKED) {
		bstrWhere += " ProvidersT.PersonID IS NOT NULL ";
		nPersonTypeCount++;
	}

	if(m_nxbtnCoSurgeonRefPhys.GetCheck() == BST_CHECKED) {
		if(bstrWhere != bstrEmpty) {
			bstrWhere += " OR ";
		}
		bstrWhere += " ReferringPhysT.PersonID IS NOT NULL ";
		nPersonTypeCount++;
	}

	if(bstrWhere == bstrEmpty) {
		// (z.manning 2008-11-19 10:23) - PLID 31687 - We have no where clause meaning we aren't
		// filtering on any type of person.
		m_pCoSurgeon->Clear();
		HandleCoSurgeonRequeryFinished();
	}
	else {
		m_pCoSurgeon->PutWhereClause(bstrWhere);
		m_pCoSurgeon->Requery();
	}

	IColumnSettingsPtr pTypeCol = m_pCoSurgeon->GetColumn(2);
	if(nPersonTypeCount > 1) {
		// (z.manning 2008-11-19 10:33) - PLID 31687 - We have more than one person type so
		// make sure the type field is visible.
		pTypeCol->PutStoredWidth(75);
	}
	else {
		pTypeCol->PutStoredWidth(0);
	}
}

void CProcInfoCenterDlg::RequeryFinishedCosurgeon(short nFlags)
{
	try
	{
		HandleCoSurgeonRequeryFinished();

	}NxCatchAll("CProcInfoCenterDlg::RequeryFinishedCosurgeon");
}

// (z.manning 2008-11-19 11:12) - PLID 31687
void CProcInfoCenterDlg::HandleCoSurgeonRequeryFinished()
{
	// (z.manning 2008-11-19 11:03) - PLID 31687 - Moved this code here from OnInitDialog
	// (a.walling 2006-11-28 12:44) - PLID 21003 - And add <No co-surgeon>
	IRowSettingsPtr pRow = m_pCoSurgeon->Row[-1];
	pRow->Value[0] = g_cvarNull;
	pRow->Value[1] = _bstr_t(" { No Co-Surgeon } ");
	pRow->Value[2] = "";
	m_pCoSurgeon->InsertRow(pRow, 0);

	if(m_pCoSurgeon->SetSelByColumn(0, m_varCoSurgeonID) == sriNoRow) {
		LoadHiddenCoSurgeonRow();
	}
}

// (z.manning 2008-11-19 11:34) - PLID 31687 - Move this functionality to its own function
void CProcInfoCenterDlg::LoadHiddenCoSurgeonRow()
{
	// (a.walling 2006-11-28 12:49) - PLID 21003 - Handle invalid/inactive cosurgeon IDs.
	//co-surgeon inactive, deleted, etc.
	_RecordsetPtr rsCoSurgeon = CreateParamRecordset(
		"SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT \r\n"
		"WHERE ID = (SELECT ProcInfoT.CoSurgeonID FROM ProcInfoT WHERE ProcInfoT.ID = {INT})\r\n"
		, m_nProcInfoID);
	if(!rsCoSurgeon->eof) {
		m_pCoSurgeon->PutComboBoxText(_bstr_t(AdoFldString(rsCoSurgeon, "Name", "")));
	}
	else {
		m_pCoSurgeon->SetSelByColumn(0, g_cvarNull);
	}
}

// (j.jones 2009-08-06 14:01) - PLID 7397 - added case history to the PIC
void CProcInfoCenterDlg::OnViewCase()
{
	try {

		if(m_pCaseHistoryCombo->CurSel != -1) {

			//there really aren't any case history permissions, so make sure they can view the regular History
			if(!CheckCurrentUserPermissions(bioPatientHistory, sptRead)) {
				return;
			}

			long nCaseHistoryID = VarLong(m_pCaseHistoryCombo->GetValue(m_pCaseHistoryCombo->CurSel, 0));

			CCaseHistoryDlg dlg(this);
			int nResult = dlg.OpenExistingCase(nCaseHistoryID);
			if (nResult != IDCANCEL) {
				Load(PIC_AREA_CASE);

				//did they print preview?
				if(nResult == RETURN_PREVIEW_CASE_HISTORY) {
					if(GetPicContainer()) {
						GetPicContainer()->SendMessage(NXM_CLOSE_PIC, cprNone);
					}
					else {
						OnOK();
					}
				}
			}
		}

	}NxCatchAll("Error in CProcInfoCenterDlg::OnViewCase");
}

// (j.jones 2009-08-06 14:01) - PLID 7397 - added case history to the PIC
void CProcInfoCenterDlg::OnNewCase()
{
	try {

		//there really aren't any case history permissions, so make sure they can write to the regular History,
		//which is what we check in that tab when creating case histories
		if(!CheckCurrentUserPermissions(bioPatientHistory, sptWrite)) {
			return;
		}

		long nPatientID = GetPatientID();

		// Have the user choose the preference card upon which this case history is to be based
		CString strCaseHistoryName;
		long nProviderID;		
		// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
		// (j.jones 2009-08-31 15:07) - PLID 35378 - we now allow multiple preference cards to be chosen
		CArray<long, long> arynPreferenceCardIDs;
		// (j.jones 2009-08-31 17:54) - PLID 17734 - this now takes in the appointment ID
		if(CCaseHistoryDlg::ChoosePreferenceCards(arynPreferenceCardIDs, strCaseHistoryName, nProviderID, nPatientID, m_nSurgApptID)) {
						
			long nAppointmentID = -1;
			COleDateTime dtSurgeryDate = COleDateTime::GetCurrentTime();
			

			//do we have an active surgery appointment?
			if(m_nSurgApptID > -1) {
				nAppointmentID = m_nSurgApptID;

				if(m_pApptList->GetValue(SURGERY_ROW, alcID).vt != VT_I4) {
					//if this isn't an integer, why do we have a m_nSurgApptID?
					ASSERT(FALSE);
				}
				else {
					dtSurgeryDate = VarDateTime(m_pApptList->GetValue(SURGERY_ROW, alcStartTime));
				}
			}

			dtSurgeryDate.SetDate(dtSurgeryDate.GetYear(), dtSurgeryDate.GetMonth(), dtSurgeryDate.GetDay());

			CCaseHistoryDlg dlg(this);
			// (j.jones 2009-08-26 09:52) - PLID 34943 - we no longer pass in location ID, a preference inside this function will calculate it
			int nResult = dlg.OpenNewCase(nPatientID, arynPreferenceCardIDs, strCaseHistoryName, nProviderID, dtSurgeryDate, nAppointmentID);
			if(nResult != IDCANCEL) {
				
				//if we have a default already, prompt to make the default case, otherwise just do it automatically

				BOOL bMakeDefault = TRUE;

				_RecordsetPtr rs = CreateParamRecordset("SELECT CaseHistoryID FROM ProcInfoT WHERE ID = {INT} AND CaseHistoryID Is Not Null", m_nProcInfoID);
				if(!rs->eof) {
					if(IDNO == MessageBox("Would you like to make the new case history the active case for this PIC?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
						bMakeDefault = FALSE;
					}
				}
				rs->Close();

				if(bMakeDefault) {				
					long nCaseHistoryID = dlg.m_nCaseHistoryID;
					ExecuteParamSql("UPDATE ProcInfoT SET CaseHistoryID = {INT} WHERE ID = {INT}", nCaseHistoryID, m_nProcInfoID);
				}

				Load(PIC_AREA_CASE);

				//did they print preview?
				if(nResult == RETURN_PREVIEW_CASE_HISTORY) {
					if(GetPicContainer()) {
						GetPicContainer()->SendMessage(NXM_CLOSE_PIC, cprNone);
					}
					else {
						OnOK();
					}
				}
			}
		}

	}NxCatchAll("Error in CProcInfoCenterDlg::OnNewCase");
}

// (j.jones 2009-08-06 14:01) - PLID 7397 - added case history to the PIC
void CProcInfoCenterDlg::OnSelChosenCaseHistoryPic(long nRow)
{
	try {

		if(nRow == -1) {
			GetDlgItem(IDC_DEFAULT_CASE_HISTORY)->EnableWindow(FALSE);
			GetDlgItem(IDC_VIEW_CASE)->EnableWindow(FALSE);
		}
		else {
			_RecordsetPtr rsActiveCase = CreateParamRecordset("SELECT CaseHistoryID FROM ProcInfoT WHERE ID = {INT} AND CaseHistoryID Is Not Null", m_nProcInfoID);
			if(rsActiveCase->eof) {
				GetDlgItem(IDC_DEFAULT_CASE_HISTORY)->EnableWindow(TRUE);
			}
			else {
				if(rsActiveCase->Fields->Item["CaseHistoryID"]->Value.vt == VT_I4)
					GetDlgItem(IDC_DEFAULT_CASE_HISTORY)->EnableWindow(VarLong(m_pCaseHistoryCombo->GetValue(nRow,0)) != AdoFldLong(rsActiveCase, "CaseHistoryID"));
				else
					GetDlgItem(IDC_DEFAULT_CASE_HISTORY)->EnableWindow(TRUE);
			}
			GetDlgItem(IDC_VIEW_CASE)->EnableWindow(TRUE);
		}

	}NxCatchAll("Error in CProcInfoCenterDlg::OnSelChosenCaseHistoryPic");
}

// (j.jones 2009-08-06 14:01) - PLID 7397 - added case history to the PIC
void CProcInfoCenterDlg::OnRequeryFinishedCaseHistoryPic(short nFlags)
{
	try {

		//set the current selection
		long nActiveRow = -1;
		_RecordsetPtr rs = CreateParamRecordset("SELECT CaseHistoryID FROM ProcInfoT WHERE ID = {INT} AND CaseHistoryID Is Not Null", m_nProcInfoID);
		if(!rs->eof) {
			nActiveRow = m_pCaseHistoryCombo->FindByColumn(0, rs->Fields->Item["CaseHistoryID"]->Value, 0, FALSE);
			if(nActiveRow != -1) {
				IRowSettingsPtr pRow = m_pCaseHistoryCombo->GetRow(nActiveRow);
				pRow->ForeColor = RGB(255,0,0);
				pRow->ForeColorSel = RGB(255,0,0);
			}
		}
		rs->Close();

		//set the last selection
		if(m_nLastSelCase == -1) {
			m_pCaseHistoryCombo->CurSel = nActiveRow;
		}
		else {
			m_pCaseHistoryCombo->SetSelByColumn(0, m_nLastSelCase);
		}

		GetDlgItem(IDC_DEFAULT_CASE_HISTORY)->EnableWindow(!(nActiveRow == m_pCaseHistoryCombo->CurSel || m_pCaseHistoryCombo->CurSel == -1));
		GetDlgItem(IDC_VIEW_CASE)->EnableWindow(m_pCaseHistoryCombo->CurSel != -1);
	
	}NxCatchAll("Error in CProcInfoCenterDlg::OnRequeryFinishedCaseHistoryPic");
}

// (j.jones 2009-08-06 14:01) - PLID 7397 - added case history to the PIC
void CProcInfoCenterDlg::OnDefaultCaseHistory()
{
	try {

		if(m_pCaseHistoryCombo->CurSel != -1) {

			long nCaseHistoryID = VarLong(m_pCaseHistoryCombo->GetValue(m_pCaseHistoryCombo->CurSel, 0));

			ExecuteParamSql("UPDATE ProcInfoT SET CaseHistoryID = {INT} WHERE ID = {INT}", nCaseHistoryID, m_nProcInfoID);

			m_nLastSelCase = nCaseHistoryID;
			m_pCaseHistoryCombo->Requery();
		}

	}NxCatchAll("Error in CProcInfoCenterDlg::OnDefaultCaseHistory");
}

// (a.vengrofski 2010-02-08 09:19) - PLID <34617> - Added a way to remove quotes from the PIC.
void CProcInfoCenterDlg::OnBnClickedUnapplyQuote()
{
	try {
		//To use any of this functionality the user must be able to change quotes and the payments that apply to them.
		if(!CheckCurrentUserPermissions(bioPatientQuotes,sptWrite)) return;//Must be able to make changes to Patient Quotes
		if(!CheckCurrentUserPermissions(bioPayment,sptWrite)) return;//Must be able to make changes to payments.

		long nRow = m_pQuoteList->CurSel;
		long nQuoteID;
		if (nRow != -1){
			nQuoteID = VarLong(m_pQuoteList->GetValue(nRow, 0));
		}else{
			return;
		}
		long nUserID = GetPatientID();
		CString strPaymentIDs = "";
		if (nQuoteID != -1) {
			CParamSqlBatch sqlBatch;
			sqlBatch.Add("UPDATE ProcInfoT SET ActiveQuoteID = NULL WHERE ActiveQuoteID = {INT}", nQuoteID);

			//Create a list of Payment IDs
			_RecordsetPtr rs = CreateParamRecordset("SELECT PaymentsT.ID FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"WHERE (LineItemT.Deleted = 0) AND (LineItemT.PatientID = {INT}) AND (PaymentsT.QuoteID = {INT})",
											   nUserID , nQuoteID);
			while(!rs->eof) {
				strPaymentIDs += AsString(AdoFldLong(rs, "ID")) + ",";
				rs->MoveNext();
			}
			rs->Close();
			strPaymentIDs.TrimRight(",");

			//get rid of the attached prepayment(s).
			if (strPaymentIDs != ""){
				CString strParamSql = "";
				strParamSql.Format("SELECT ID FROM ProcInfoT WHERE ID IN (SELECT ProcInfoID FROM ProcInfoPaymentsT WHERE "
								   "PayID IN (%s)) AND (BillID Is Null OR BillID NOT IN (SELECT BillID FROM ChargesT "
								   "INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID WHERE AppliesT.SourceID IN (%s)))",
								   strPaymentIDs, strPaymentIDs);
				rs = CreateParamRecordset(strParamSql);
				if(!rs->eof) {
					if(IDYES == MsgBox(MB_YESNO, "The active quote has at least one attached PrePayment.  Would you like to detach the PrePayments from this procedure?\n"
												 "(The PrePayment will not be deleted; it will be made available for general use.)")) {
						CString strProcInfoIDs;
						while(!rs->eof) {
							strProcInfoIDs += AsString(AdoFldLong(rs, "ID")) + ",";
							rs->MoveNext();
						}
						rs->Close();
						strProcInfoIDs.TrimRight(",");
						if (!strProcInfoIDs.IsEmpty()) {
							sqlBatch.Add("UPDATE PaymentsT SET QuoteID = NULL WHERE ID IN (SELECT PayID FROM ProcInfoPaymentsT WHERE ProcInfoID = {INT})", m_nProcInfoID);
							sqlBatch.Add("DELETE FROM ProcInfoPaymentsT WHERE ProcInfoID IN ({INTSTRING}) AND PayID IN ({INTSTRING})",strProcInfoIDs, strPaymentIDs);
						}
					}
					else {
						// (j.jones 2013-05-07 17:20) - PLID 45797 - if they said no to unlinking payments from the PIC, we still
						// need to unlink them from this quote (whereas unlinking from the PIC also unlinks from *all* quotes)
						sqlBatch.Add("UPDATE PaymentsT SET QuoteID = NULL WHERE ID IN (SELECT PayID FROM ProcInfoPaymentsT WHERE ProcInfoID = {INT}) AND QuoteID = {INT}", m_nProcInfoID, nQuoteID);
					}
				}
			}

			// (j.jones 2013-05-07 16:58) - PLID 45797 - Always execute, because our batch is always
			// going to unapply the quote. It may or may not also unlink payments.
			sqlBatch.Execute(GetRemoteData());
			m_pQuoteList->Requery();
			GetDlgItem(IDC_UNAPPLY_QUOTE)->EnableWindow(FALSE);
			Load(PIC_AREA_ALL);
		}

	}NxCatchAll("Error in CProcInfoCenterDlg::OnBnClickedUnapplyQuote");
}


// (j.dinatale 2011-10-04 09:10) - PLID 43528 - Be able to unapply a bill from a PIC
void CProcInfoCenterDlg::OnBnClickedUnapplyBill()
{
	try{
		// (j.dinatale 2012-07-10 15:47) - PLID 3073 - handle procinfobillst
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pBillCombo->CurSel;
		if(pRow){
			long nBillID = VarLong(pRow->GetValue(PicBillComboList::BillID), -1);
			if(nBillID > 0){
				ExecuteParamSql(
					"UPDATE ProcInfoT SET BillID = NULL WHERE ID = {INT} AND BillID = {INT}; "
					"DELETE FROM ProcInfoBillsT WHERE ProcInfoID = {INT} AND BillID = {INT}; ", 
					m_nProcInfoID, nBillID, m_nProcInfoID, nBillID);
				Load(PIC_AREA_BILL);
			}
		}
	}NxCatchAll(__FUNCTION__);
}
// (d.singleton 2012-04-05 10:00) - PLID 40019
void CProcInfoCenterDlg::OnBnClickedPreOpSched()
{
	try {
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;

		mnu.InsertMenu(nIndex++, MF_BYPOSITION, PREOP_CALENDAR_NEW, "&Create New Calendar");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, PREOP_CALENDAR_EXISTING, "&Open Existing Calendar");

		CRect rBtn;
		GetDlgItem(IDC_PREOP_SCHED)->GetWindowRect(rBtn);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, rBtn.right, rBtn.top, this, NULL);		
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2012-05-16 16:41) - PLID 50436 open pre op calender setup dlg and load apt data
void CProcInfoCenterDlg::OnAddNewCalendar()
{
	try {
		//get the procedure info
		NXDATALISTLib::IRowSettingsPtr pRow = m_pProcNames->GetRow(0);
		CMedSchedule dlg(this, TRUE);
		if(pRow) {
			dlg.m_nProcedureID = VarLong(pRow->GetValue(2));
		}
		//get the appointment info
		for(int i = 0; i < m_pApptList->GetRowCount(); i++) {
			pRow = m_pApptList->GetRow(i);
			//make sure we skip header and blank rows
			_variant_t varTemp = pRow->GetValue(alcType);
			if(varTemp.vt != VT_EMPTY) {
				CString strTemp = VarString(varTemp, "");
				if(strTemp.Left(4) == "----" || strTemp.CompareNoCase("<None>") == 0) {
					continue;
				}
			}
			else {
				continue;
			}

			//fill the array of appointments start with apt type category
			AppointmentInfo aiAptInfo;
			long nCategory = AsLong(VarByte(pRow->GetValue(alcCategory), 0));
			switch(nCategory) {
				case 1:
					aiAptInfo.strCategory = "Consult Appt: ";
					aiAptInfo.nCategoryID = 1;
					break;
				case 2:
					aiAptInfo.strCategory = "PreOp Appt: ";
					aiAptInfo.nCategoryID = 2;
					break;
				case 4:
					aiAptInfo.strCategory = "Surgery Appt: ";
					aiAptInfo.nCategoryID = 4;
					break;
				case 5:
					aiAptInfo.strCategory = "Follow Up Appt: ";
					aiAptInfo.nCategoryID = 5;
					break;
				default:
					aiAptInfo.strCategory = "Appt: ";
					aiAptInfo.nCategoryID = -1;
					break;
			}

			//grab the type just in case and also the start date
			aiAptInfo.strAptType = VarString(pRow->GetValue(alcType), "");				
			aiAptInfo.dtStartDate =  VarDateTime(pRow->GetValue(alcStartTime));

			//get the string version of the time
			CString strAptTime = aiAptInfo.strCategory + FormatDateTimeForInterface(VarDateTime(pRow->GetValue(alcStartTime)), "%X");
			aiAptInfo.strAptTime = strAptTime;

			dlg.m_arAppointmentInfo.Add(aiAptInfo);
		}
		dlg.DoModal();
		if(dlg.m_bPrintOnClose) {
			//auto select the procedure tab if its not already open and then preview the calendar
			CMainFrame *pMainFrame = GetMainFrame();
			pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
			CPatientView *pView = (CPatientView *)pMainFrame->GetOpenView(PATIENT_MODULE_NAME);
			pView->SetActiveTab(PatientsModule::ProcedureTab);

			pView->m_bIsPicPreOpCalendar = TRUE;
			pView->bMedSchedPrintInfoLoaded = FALSE;
			pView->m_tempMedSchedID = dlg.m_MedSchedID;
			pView->m_arAppointmentInfo = &dlg.m_arAppointmentInfo;
			pView->OnFilePrintPreview();
		}
	}NxCatchAll(__FUNCTION__);
}

void CProcInfoCenterDlg::OnOpenExistingCalendar()
{
	try {
		CCalenderSelectDlg csDlg(this);

		//get the procedures
		for(int i = 0; i < m_pProcNames->GetRowCount(); i++) {
			IRowSettingsPtr pRow = m_pProcNames->GetRow(i);
			if(pRow) {
				m_arProcedureIDs.Add(VarLong(pRow->GetValue(2)));
			}
		}
		//set where clause
		CString strProcIDs = "";
		for(int i = 0; i < m_arProcedureIDs.GetCount(); i++) {
			strProcIDs += AsString(m_arProcedureIDs.GetAt(i));
			if(i != m_arProcedureIDs.GetCount() - 1) {
				strProcIDs += ",";
			}
		}
		CString strWhere;
		strWhere.Format("PatientID = %li ", GetPatientID());
		csDlg.m_strWhere = strWhere;

		if(csDlg.DoModal() == IDOK) {

			CMedSchedule dlg(this, TRUE);
			dlg.m_MedSchedID = csDlg.m_nCalendarID;

			//get the appointment info
			IRowSettingsPtr pRow;
			for(int i = 0; i < m_pApptList->GetRowCount(); i++) {
				pRow = m_pApptList->GetRow(i);
				//make sure we skip header and blank rows
				_variant_t varTemp = pRow->GetValue(alcType);
				if(varTemp.vt != VT_EMPTY) {
					CString strTemp = VarString(varTemp, "");
					if(strTemp.Left(4) == "----" || strTemp.CompareNoCase("<None>") == 0) {
						continue;
					}
				}
				else {
					continue;
				}

				//fill the array of appointments start with apt type category
				AppointmentInfo aiAptInfo;
				long nCategory = AsLong(VarByte(pRow->GetValue(alcCategory), 0));
				switch(nCategory) {
				case 1:
					aiAptInfo.strCategory = "Consult Appt: ";
					aiAptInfo.nCategoryID = 1;
					break;
				case 2:
					aiAptInfo.strCategory = "PreOp Appt: ";
					aiAptInfo.nCategoryID = 2;
					break;
				case 3:
				case 4:
					aiAptInfo.strCategory = "Surgery Appt: ";
					aiAptInfo.nCategoryID = 4;
					break;
				case 5:
					aiAptInfo.strCategory = "Follow Up Appt: ";
					aiAptInfo.nCategoryID = 5;
					break;
				default:
					aiAptInfo.strCategory = "Appt: ";
					aiAptInfo.nCategoryID = -1;
					break;
				}

				//grab the type just in case and also the start date
				aiAptInfo.strAptType = VarString(pRow->GetValue(alcType), "");				
				aiAptInfo.dtStartDate =  VarDateTime(pRow->GetValue(alcStartTime));

				//get the string version of the time
				CString strAptTime = aiAptInfo.strCategory + FormatDateTimeForInterface(VarDateTime(pRow->GetValue(alcStartTime)), "%X");
				aiAptInfo.strAptTime = strAptTime;

				dlg.m_arAppointmentInfo.Add(aiAptInfo);
			}

			dlg.DoModal();
			if(dlg.m_bPrintOnClose) {
				CMainFrame *pMainFrame = GetMainFrame();
				pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
				CPatientView *pView = (CPatientView *)pMainFrame->GetOpenView(PATIENT_MODULE_NAME);
				pView->SetActiveTab(PatientsModule::ProcedureTab);

				pView->m_bIsPicPreOpCalendar = TRUE;
				pView->bMedSchedPrintInfoLoaded = FALSE;
				pView->m_tempMedSchedID = dlg.m_MedSchedID;
				pView->m_arAppointmentInfo = &dlg.m_arAppointmentInfo;
				pView->OnFilePrintPreview();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-07-10 16:31) - PLID 3073 - be able to mark a bill active
void CProcInfoCenterDlg::OnBnClickedPicMarkBillActive()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pBillCombo->CurSel;
		if(pRow){
			long nBillID = VarLong(pRow->GetValue(PicBillComboList::BillID), -1);
			if(nBillID > 0){
				ExecuteParamSql(
					"UPDATE ProcInfoT SET BillID = {INT} WHERE ID = {INT}; ",
					nBillID, m_nProcInfoID);
				Load(PIC_AREA_BILL);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-07-10 16:39) - PLID 3073 - on requery finished, we want to set the active bill if any
void CProcInfoCenterDlg::OnRequeryFinishedBillComboPic(short nFlags)
{
	try{
		_RecordsetPtr rs = CreateParamRecordset("SELECT BillID FROM ProcInfoT WHERE ID = {INT} AND BillID IS NOT NULL", m_nProcInfoID);
		if(!rs->eof){
			long nBillID = AdoFldLong(rs, "BillID", -1);
			if(nBillID > 0){
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pBillCombo->FindByColumn(PicBillComboList::BillID, nBillID, NULL, FALSE);
				if(pRow) {
					pRow->ForeColor = RGB(255,0,0);
					pRow->ForeColorSel = RGB(255,0,0);
					m_pBillCombo->CurSel = pRow;
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}