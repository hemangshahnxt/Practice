// SelectApptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "SelectApptDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSelectApptDlg dialog

enum ApptListColumns {
	alcID = 0,
	alcColor,
	alcStartTime,
	alcType,
	alcPurpose,
	alcPatient,
};

CSelectApptDlg::CSelectApptDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSelectApptDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectApptDlg)
		m_bAllowMultiSelect = TRUE;
		m_strLabel = "Please select one or more appointments to associate with this procedure.";
		// (j.jones 2008-03-19 10:32) - PLID 29324 - added ability to show a patient column,
		// and to select no appt. at all
		m_bShowPatientName = FALSE;
		m_bShowNoneSelectedRow = FALSE;
	//}}AFX_DATA_INIT
}


void CSelectApptDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectApptDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_SELECT_APPT_LABEL, m_nxstaticSelectApptLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectApptDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectApptDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectApptDlg message handlers

BOOL CSelectApptDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		SetDlgItemText(IDC_SELECT_APPT_LABEL,m_strLabel);
		
		m_pApptList = BindNxDataListCtrl(this, IDC_APPT_LIST, GetRemoteData(), false);
		m_pApptList->AllowMultiSelect = m_bAllowMultiSelect;

		// (j.jones 2008-03-19 10:32) - PLID 29324 - added ability to show a patient column
		if(m_bShowPatientName) {
			IColumnSettingsPtr pCol = m_pApptList->GetColumn(alcPatient);
			pCol->PutColumnStyle(csVisible | csWidthAuto);
		}
		
		m_pApptList->WhereClause = _bstr_t(m_strWhere);
		m_pApptList->Requery();

		// (j.jones 2008-03-19 10:34) - PLID 29324 - added ability to select no appt. at all
		if(m_bShowNoneSelectedRow) {

			IRowSettingsPtr pRow = m_pApptList->GetRow(-1);
			pRow->PutValue(alcID, (long)-1);
			pRow->PutValue(alcColor, (long)COLORREF(RGB(0,0,0)));
			pRow->PutValue(alcStartTime, g_cvarNull);
			pRow->PutValue(alcType, _bstr_t("<No Appointment>"));
			pRow->PutValue(alcPurpose, g_cvarNull);
			pRow->PutValue(alcPatient, g_cvarNull);
			m_pApptList->AddRow(pRow);
		}

		//assert if both options are checked, doesn't seem like you would ever want both
		//(if someone does in the future, then feel free to remove this assertion)
		ASSERT(!(m_bShowNoneSelectedRow && m_bAllowMultiSelect));

	}NxCatchAll("Error in CSelectApptDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectApptDlg::OnOK() 
{
	LPDISPATCH lpRow = NULL;
	long p = m_pApptList->GetFirstSelEnum();
	while (p) {
		//this will output the next row's p, and the current row's lpRow.
		m_pApptList->GetNextSelEnum(&p, &lpRow);
		
		
		IRowSettingsPtr pRow(lpRow);
		//Add appt id to array
		long nApptID = VarLong(pRow->Value[0]);
		m_arSelectedIds.Add(nApptID);

		lpRow->Release();
	}

	CDialog::OnOK();
}

BEGIN_EVENTSINK_MAP(CSelectApptDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSelectApptDlg)
	ON_EVENT(CSelectApptDlg, IDC_APPT_LIST, 18 /* RequeryFinished */, OnRequeryFinishedApptList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSelectApptDlg::OnRequeryFinishedApptList(short nFlags) 
{
	//Color all the rows.
	for(int i = 0; i < m_pApptList->GetRowCount(); i++) {
		IRowSettingsPtr pRow = m_pApptList->GetRow(i);
		pRow->PutForeColor(VarLong(pRow->GetValue(1), RGB(0,0,0)));
	}
}
