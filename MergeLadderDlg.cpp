// MergeLadderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "MergeLadderDlg.h"
#include "PhaseTracking.h"
#include "MsgBox.h"
#include "GlobalDrawingUtils.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMergeLadderDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;
using namespace PhaseTracking;

CMergeLadderDlg::CMergeLadderDlg(long nLadderID, long nLadderTemplateID, long nPersonID, CWnd* pParent)
	: CNxDialog(CMergeLadderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMergeLadderDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nLadderID = nLadderID;
	m_nLadderTemplateID = nLadderTemplateID;
	m_nPersonID = nPersonID;
}




void CMergeLadderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMergeLadderDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_LADDER_NAME, m_nxeditLadderName);
	DDX_Control(pDX, IDC_LADDER_START_DATE, m_nxeditLadderStartDate);
	DDX_Control(pDX, IDC_LADDER_STATUS, m_nxeditLadderStatus);
	DDX_Control(pDX, IDC_LADDER_ACTION, m_nxeditLadderAction);
	DDX_Control(pDX, IDC_STATIC_INFO, m_nxstaticInfo);
	DDX_Control(pDX, IDC_STATIC_PROCS, m_nxstaticProcs);
	DDX_Control(pDX, IDC_STATIC_MERGE_START, m_nxstaticMergeStart);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_nxstaticStatus);
	DDX_Control(pDX, IDC_STATIC_ACTION, m_nxstaticAction);
	DDX_Control(pDX, IDC_STATIC_SELECT, m_nxstaticSelect);
	DDX_Control(pDX, IDC_MERGE, m_btnMerge);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CMergeLadderDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMergeLadderDlg)
	ON_BN_CLICKED(IDC_MERGE, OnMerge)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMergeLadderDlg message handlers

void CMergeLadderDlg::OnMerge() 
{
	
	//get the ladder ID they want to merge into
	IRowSettingsPtr pCurSel = m_pLadders->CurSel;
	
	if (pCurSel) {

		long nLadderIDToMergeInto = pCurSel->GetValue(0);	
	
		//make sure they know that this in unrecoverable

		CString strMessage;
		strMessage.Format("This action will merge the top ladder into the ladder selected in the drop down.  "
			" If the top ladder has any fields filled out (ie: Active Quote) that the bottom ladder doesn't, "
			" the information in the top ladder will be used.  If data exists in the bottom ladder that also exists "
			" in the top ladder, then the information in the top ladder will be lost. Any appointments or procedures associated with "
			" either ladder will be combined into the bottom ladder.\n\n"
			" This action is UNRECOVERABLE, are you sure you want to continue?");

		if (MsgBox(MB_YESNO | MB_ICONEXCLAMATION, strMessage) == IDNO) { 
			
			//close the box, they don't want to continue
			OnCancel();
			return;

		}

		//they want to continue, lets go
		if (PhaseTracking::MergeLadders(m_nLadderID, nLadderIDToMergeInto, m_nPersonID)) {
			MsgBox("Ladders merged successfully");
			OnOK();
			return;
		}

	}
	else {
		MsgBox("Please select a ladder to merge this ladder into");
		return;
	}

}
void CMergeLadderDlg::OnOK()  {

	CDialog::OnOK();

}



void CMergeLadderDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}


BOOL CMergeLadderDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog(); // (a.walling 2011-01-14 16:44) - no PLID - Fix bad base class OnInitDialog call

	m_brush.CreateSolidBrush(PaletteColor(0x00FFCC99));
	try {
		// (c.haag 2008-05-01 12:18) - PLID 29866 - NxIconify the buttons
		m_btnMerge.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//initialize the datalist
		m_pLadders = BindNxDataList2Ctrl(IDC_MERGEABLE_LADDER_LIST, GetRemoteData(), FALSE);

		//set the info
		_RecordsetPtr rs = CreateRecordset("SELECT LaddersT.ID, dbo.CalcProcInfoName(ProcInfoT.ID) as ProcName, FirstInterestDate, "
			" LadderStatusT.Name as Status "
			" FROM LaddersT INNER JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID "
			" LEFT JOIN LadderStatusT ON LaddersT.Status = LadderStatusT.ID "
			" WHERE LaddersT.ID = %li "
			" Group By LaddersT.ID,  FirstInterestDate, LadderstatusT.Name, ProcInfoT.ID ", m_nLadderID);

		if (! rs->eof) {

			SetDlgItemText(IDC_LADDER_NAME, AdoFldString(rs, "ProcName", ""));
			SetDlgItemText(IDC_LADDER_START_DATE, FormatDateTimeForInterface(AdoFldDateTime(rs, "FirstInterestDate"), NULL, dtoDateTime, true));
			CString strStatus = GetLadderDescription(m_nLadderID);
			SetDlgItemText(IDC_LADDER_STATUS, AdoFldString(rs, "Status", ""));
			if (strStatus.CompareNoCase("<LadderStatusName>") == 0) {
				SetDlgItemText(IDC_LADDER_ACTION, AdoFldString(rs, "Status", ""));
			}
			else {
				SetDlgItemText(IDC_LADDER_ACTION, strStatus);
			}
			
		}
		rs->Close();
		
		
		//get all the ladders that can possibly be merged into
		rs =  CreateRecordset("SELECT LaddersT.ID, dbo.CalcProcInfoName(ProcInfoT.ID) as ProcName, FirstInterestDate, "
			" LadderStatusT.Name as Status "
			" FROM LaddersT INNER JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID "
			" INNER JOIN StepsT ON LaddersT.ID = StepsT.LadderID "
			" INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID  "
			" LEFT JOIN LadderStatusT ON LaddersT.Status = LadderStatusT.ID "
			" WHERE StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE PersonID = %li AND LadderID <> %li) "
			" AND StepTemplatesT.LadderTemplateID = %li "
			" Group By LaddersT.ID,  FirstInterestDate, LadderstatusT.Name, ProcInfoT.ID ", m_nPersonID, m_nLadderID, m_nLadderTemplateID);

		//now add these records to our datalist
		while (!rs->eof) {
			IRowSettingsPtr pRow = m_pLadders->GetNewRow();
			FieldsPtr flds = rs->Fields;
			long nLadderID = AdoFldLong(flds, "ID");
			pRow->PutValue(0, nLadderID);
			pRow->PutValue(1, _variant_t(FormatDateTimeForInterface(AdoFldDateTime(flds, "FirstInterestDate"), NULL, dtoDateTime, true)));
			pRow->PutValue(2, _variant_t(AdoFldString(flds, "ProcName", "")));
			pRow->PutValue(3, _variant_t(AdoFldString(flds, "Status", "")));
			CString strStatus = GetLadderDescription(nLadderID);
			if (strStatus.CompareNoCase("<LadderStatusName>") == 0) {
				pRow->PutValue(4, _variant_t(AdoFldString(flds, "Status", "")));
			}
			else {
				pRow->PutValue(4, _variant_t(strStatus));
			}
			m_pLadders->AddRowAtEnd(pRow, NULL);

			rs->MoveNext();
		}
		rs->Close();
	}NxCatchAllCall("CMergeLadderDlg::OnInitDialog Error loading dialog", return FALSE;);
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CMergeLadderDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	switch (pWnd->GetDlgCtrlID())  {
		case IDC_STATIC:
		case IDC_STATIC_ACTION:
		case IDC_STATIC_STATUS:
		case IDC_STATIC_MERGE_START:
		case IDC_STATIC_PROCS:
		case IDC_STATIC_INFO:
		case IDC_STATIC_SELECT:
	
			extern CPracticeApp theApp;
  			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x00FFCC99));
			return m_brush;
		break;
		default:
		break;
	}
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
	*/

	// (a.walling 2008-04-02 09:13) - PLID 29497 - Handle new NxColor's non-solid backgrounds
	HANDLE_GENERIC_TRANSPARENT_CTL_COLOR();
}


CString CMergeLadderDlg::GetLadderDescription(long nLadderID) {


	_RecordsetPtr rsStatus = CreateRecordset("SELECT Status, LadderStatusT.Name, LadderStatusT.IsActive FROM LaddersT LEFT JOIN LadderStatusT ON LadderStatusT.ID = LaddersT.Status WHERE LaddersT.ID = %li", nLadderID);
	if(!AdoFldBool(rsStatus, "IsActive")) {
		return AdoFldString(rsStatus, "Name", "");
	}
	else {
		if(GetNextStepID(nLadderID) != -1) {
			_RecordsetPtr rsActiveDate = CreateRecordset("SELECT ActiveDate FROM StepsT WHERE ID = %li", GetNextStepID(nLadderID));
			COleDateTime dtActive = AdoFldDateTime(rsActiveDate, "ActiveDate", COleDateTime::GetCurrentTime());
			if(dtActive > COleDateTime::GetCurrentTime()) {
				//We're on hold.
				CString strStatus;
				if(dtActive.GetYear() == 9999) {
					strStatus = "On Hold indefinitely";
				}
				else {
					strStatus.Format("On Hold until %s", FormatDateTimeForInterface(AdoFldDateTime(rsActiveDate, "ActiveDate")));
				}
				return strStatus;
			}
			else {
				if(AdoFldLong(rsStatus, "Status") == 1) {//Active
					//Get the name of the active step.
					_RecordsetPtr rsStepName = CreateRecordset("SELECT StepName FROM StepTemplatesT WHERE ID = (SELECT StepTemplateID FROM StepsT WHERE ID = %li)", GetNextStepID(nLadderID));
					CString strStatus = AdoFldString(rsStepName, "StepName");
					return strStatus;
				}
				else {
					return "<LadderStatusName>";
				}
			}
		}
		else {							  
			return "<LadderStatusName>";
		}
	}

	
}
