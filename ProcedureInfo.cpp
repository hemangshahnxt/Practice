// ProcedureInfo.cpp : implementation file
//

#include "stdafx.h"
#include "ProcedureInfo.h"
#include "GlobalDataUtils.h"
#include "Reports.h"
#include "ReportInfo.h"
#include "GlobalReportUtils.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// CProcedureInfo dialog


CProcedureInfo::CProcedureInfo(CWnd* pParent)
	: CNxDialog(CProcedureInfo::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProcedureInfo)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bAllowPreview = false;
}


void CProcedureInfo::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcedureInfo)
	DDX_Control(pDX, IDC_INFO, m_text);
	DDX_Control(pDX, IDC_PROC_CAPTION, m_nxstaticProcCaption);
	DDX_Control(pDX, IDC_PREVIEW_CHEAT_SHEET_RPT, m_nxbuttonPreview);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CProcedureInfo, CNxDialog)
	ON_WM_NCACTIVATE()
	ON_WM_CTLCOLOR()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_PREVIEW_CHEAT_SHEET_RPT, OnPreviewCheatSheetRpt)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CProcedureInfo message handlers

static CHARRANGE AppendInfo(CString &text, LPCSTR field, LPCSTR fieldName, FieldsPtr &f)
{

	CHARRANGE cr;

	CString value = AdoFldString (f, field);
	if (value != "")
	{	if (text != "")
			text += "\r\n\r\n";
		cr.cpMin = text.GetLength();
		text += fieldName;
		text += ":\r\n\t";
		cr.cpMax = text.GetLength();
		text += value;
	}
	else cr.cpMax = cr.cpMin = 0;

	return cr;
}

BOOL CProcedureInfo::OnInitDialog() 
{
	try {
		m_closing = false;
		CNxDialog::OnInitDialog();

		// (d.thompson 2009-02-17) - PLID 3843 - Setup the preview button.  If
		//	they don't want it shown, do so here.
		m_nxbuttonPreview.AutoSet(NXB_PRINT_PREV);
		if(m_bAllowPreview == false) {
			m_nxbuttonPreview.EnableWindow(FALSE);
			m_nxbuttonPreview.ShowWindow(SW_HIDE);
		}

		//Get the ids into a comma-delimited list.
		CString strProcIDs, strTemp;
		strProcIDs.Format("%li", m_arProcIDs.GetAt(0));
		for(int i = 1; i < m_arProcIDs.GetSize(); i++) {
			strTemp.Format(", %li", m_arProcIDs.GetAt(i));
			strProcIDs += strTemp;
		}
		m_pProcSelect = BindNxDataListCtrl(IDC_PROC_SELECT, false);
		m_pProcSelect->WhereClause = _bstr_t("ID IN (" + strProcIDs + ")");
		m_pProcSelect->Requery();

	} NxCatchAll("Error in OnInitDialog");

	return FALSE;
}

void CProcedureInfo::OnCancel() 
{
	m_closing = true;
	CDialog::OnCancel();
}

HBRUSH CProcedureInfo::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	return (HBRUSH)GetStockObject(WHITE_BRUSH);
}

BOOL CProcedureInfo::OnNcActivate(BOOL bActive)
{
	if (!bActive && !m_closing)//CDialog::OnOK calls OnNcActivate on Win98 machines
	{	m_closing = true;
		CDialog::OnOK();
	}
	return CNxDialog::OnNcActivate(bActive);
}

void CProcedureInfo::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CProcedureInfo, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CProcedureInfo)
	ON_EVENT(CProcedureInfo, IDC_PROC_SELECT, 16 /* SelChosen */, OnSelChosenProcSelect, VTS_I4)
	ON_EVENT(CProcedureInfo, IDC_PROC_SELECT, 18 /* RequeryFinished */, OnRequeryFinishedProcSelect, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CProcedureInfo::OnSelChosenProcSelect(long nRow) 
{
	try {
		CString str;
		CHARRANGE cr[6];
		CHARFORMAT cfBold, cfDefault;

		cfBold.dwMask = CFM_STRIKEOUT|CFM_BOLD;
		cfBold.dwEffects = CFE_BOLD;

		cfDefault.dwMask = CFM_UNDERLINE|CFM_BOLD;
		cfDefault.dwEffects = 0;

		long nProcID = VarLong(m_pProcSelect->GetValue(nRow, 0));
		_RecordsetPtr rs = CreateRecordset("SELECT "
			"Name, Custom1, Custom2, Custom3, Custom4, Custom5, Custom6 "
			"FROM ProcedureT "
			"WHERE ID = %i", 
			nProcID);
		FieldsPtr f = rs->Fields;

		str = AdoFldString(f, "Name");
		SetWindowText(str);
		str = "";

		_RecordsetPtr rsCustomFields = CreateRecordset("SELECT Name FROM CustomFieldsT WHERE ID > 62 AND ID < 69");
		cr[0] = AppendInfo(str, "Custom1", AdoFldString(rsCustomFields, "Name"), f);
		rsCustomFields->MoveNext();
		cr[1] = AppendInfo(str, "Custom2", AdoFldString(rsCustomFields, "Name"), f);
		rsCustomFields->MoveNext();
		cr[2] = AppendInfo(str, "Custom3", AdoFldString(rsCustomFields, "Name"), f);
		rsCustomFields->MoveNext();
		cr[3] = AppendInfo(str, "Custom4", AdoFldString(rsCustomFields, "Name"), f);
		rsCustomFields->MoveNext();
		cr[4] = AppendInfo(str, "Custom5", AdoFldString(rsCustomFields, "Name"), f);
		rsCustomFields->MoveNext();
		cr[5] = AppendInfo(str, "Custom6", AdoFldString(rsCustomFields, "Name"), f);

		if (str.IsEmpty())
			str = "[No script entered - you may enter this on the procedure tab of the Administrator]";

		m_text.SetWindowText(str);

		int nLastBold = 0;
		for (int i = 0; i < 6; i++)
		{
			m_text.SetSel(cr[i]);
			m_text.SetSelectionCharFormat(cfBold);
			if(cr[i].cpMax != 0) nLastBold = cr[i].cpMax;
			if(i < 5) {
				if(cr[i+1].cpMin > cr[i].cpMax) {//Sometimes one of these will be zeroes because there is no script for that part.
					m_text.SetSel(nLastBold, cr[i+1].cpMin);
					m_text.SetSelectionCharFormat(cfDefault);
				}
			}

		}
		//Finish it off
		if(nLastBold > 0) {
			m_text.SetSel(nLastBold, m_text.GetTextLength());
			m_text.SetSelectionCharFormat(cfDefault);
		}
		

		m_text.SetFocus();//returning TRUE to set the selection selects all the text
		m_text.SetSel(0,0);
	}
	NxCatchAll("Could not load procedure info");
}

void CProcedureInfo::OnRequeryFinishedProcSelect(short nFlags) 
{
	m_pProcSelect->CurSel = 0;
	if(m_arProcIDs.GetSize() == 1) {
		GetDlgItem(IDC_PROC_SELECT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PROC_CAPTION)->ShowWindow(SW_HIDE);
	}
	else {
		GetDlgItem(IDC_PROC_SELECT)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_PROC_CAPTION)->ShowWindow(SW_SHOW);
	}

	OnSelChosenProcSelect(0);
		
}

// (d.thompson 2009-02-17) - PLID 3843 - Added preview button
void CProcedureInfo::OnPreviewCheatSheetRpt()
{
	try {
		//Get the current procedure ID
		long nRow = m_pProcSelect->CurSel;
		if(nRow == NXDATALISTLib::sriNoRow) {
			return;
		}

		long nProcID = VarLong(m_pProcSelect->GetValue(nRow, 0));

		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(569)]);
		infReport.strExternalFilter = FormatString("ProcedureT.ID = %li", nProcID);

		RunReport(&infReport, TRUE, this, "Procedure Cheat Sheet Information");

	} NxCatchAll("Error in OnPreviewCheatSheetRpt");
}
