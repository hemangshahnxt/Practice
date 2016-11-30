// PatientNexEMRConfigureColumns.cpp : implementation file
//

#include "stdafx.h"
#include "PatientNexEMRConfigureColumnsDlg.h"
#include "PatientNexEMRDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;

enum EmrColumnListColumns
{
	colsCheckbox = 0,
	colsName,
};

/////////////////////////////////////////////////////////////////////////////
// CPatientNexEMRConfigureColumnsDlg dialog

CPatientNexEMRConfigureColumnsDlg::CPatientNexEMRConfigureColumnsDlg(CPatientNexEMRDlg* pParent /*=NULL*/)
	: CNxDialog(CPatientNexEMRConfigureColumnsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPatientNexEMRConfigureColumnsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pParentNexEMRDlg = pParent;
}


void CPatientNexEMRConfigureColumnsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPatientNexEMRConfigureColumnsDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPatientNexEMRConfigureColumnsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPatientNexEMRConfigureColumnsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPatientNexEMRConfigureColumnsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPatientNexEMRDlg)
	ON_EVENT(CPatientNexEMRConfigureColumnsDlg, IDC_NEXEMR_COLUMNS, 10 /* EditingFinished */, OnEditingFinishedColumnList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPatientNexEMRConfigureColumnsDlg message handlers

BOOL CPatientNexEMRConfigureColumnsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-05-01 12:25) - PLID 29866 - NxIconify the Close button
		m_btnOK.AutoSet(NXB_CLOSE);

		m_pColumns = BindNxDataList2Ctrl(this, IDC_NEXEMR_COLUMNS, GetRemoteData(), false);

		// (z.manning, 04/05/2007) - PLID 25518 - Populate the list of all possible viewable columns for
		// EMR tab view.
		// (z.manning 2008-07-02 14:00) - Note: You can not ever change the names of these columns
		// as they are stored in ConfigRT (unless you wrote a mod to handle that).
		if(m_pParentNexEMRDlg != NULL && m_pParentNexEMRDlg->IsEMNTabView()) {
			// (z.manning 2010-05-11 17:42) - PLID 38354 - These only are available in tab view
			m_arystrColumnNames.Add("Category");
			m_arystrColumnNames.Add("Chart");
			m_arystrColumnNames.Add("EMR Description");
		}
		m_arystrColumnNames.Add("Date");
		m_arystrColumnNames.Add("EMN Description");
		m_arystrColumnNames.Add("History Documents");
		m_arystrColumnNames.Add("Preview Icon"); // (z.manning 2008-07-02 14:01) - PLID 30596
		m_arystrColumnNames.Add("Clinical Summary Icon"); // (r.gonet 04/22/2014) - PLID 61807
		m_arystrColumnNames.Add("Problem");
		m_arystrColumnNames.Add("Provider");		// (d.thompson 2010-01-08) - PLID 35925
		m_arystrColumnNames.Add("Secondary Provider"); // (z.manning 2010-05-11 16:41) - PLID 38354
		m_arystrColumnNames.Add("Assistant/Technician");	// (d.lange 2011-04-29 15:59) - PLID 43382 - Added Assistant/Technician
		m_arystrColumnNames.Add("Status");

		Load();

	}NxCatchAll("CPatientNexEMRConfigureColumnsDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPatientNexEMRConfigureColumnsDlg::Load()
{
	m_pColumns->Clear();

	IRowSettingsPtr pRow;
	for(int i = 0; i < m_arystrColumnNames.GetSize(); i++)
	{
		// (z.manning, 04/06/2007) - PLID 25518 - Load the list of columns and set the checkbox depending on
		// the option for each column.
		pRow = m_pColumns->GetNewRow();
		VARIANT_BOOL bCheck = VARIANT_FALSE;
		if(1 == GetRemotePropertyInt(FormatString("ShowNexEMRColumn%s",m_arystrColumnNames.GetAt(i)), 1, 0, "<None>", true)) {
			bCheck = VARIANT_TRUE;
		}
		pRow->PutValue(colsCheckbox, bCheck);
		pRow->PutValue(colsName, _bstr_t(m_arystrColumnNames.GetAt(i)));
		m_pColumns->AddRowSorted(pRow, NULL);
	}

}

void CPatientNexEMRConfigureColumnsDlg::OnEditingFinishedColumnList(LPDISPATCH lpRow, short nCol, const _variant_t &varOldValue, const _variant_t &varNewValue, VARIANT_BOOL bCommit)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		switch(nCol)
		{
		case colsCheckbox:
			// (z.manning, 04/06/2007) - PLID 25518 - Update the property value.
			long nIntParam = VarBool(varNewValue) ? 1 : 0;
			SetRemotePropertyInt(FormatString("ShowNexEMRColumn%s",VarString(pRow->GetValue(colsName))), nIntParam, 0, "<None>");

			// (z.manning, 04/06/2007) - PLID 25518 - If we have a NexEMR tab parent, update its tabs.
			if(m_pParentNexEMRDlg) {
				m_pParentNexEMRDlg->EnsureTabsPosition();
			}
			break;
		}

	}NxCatchAll("CPatientNexEMRConfigureColumnsDlg::OnEditingFinishedColumnList");
}