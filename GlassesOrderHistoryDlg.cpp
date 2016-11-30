// GlassesOrderHistoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "GlassesOrderHistoryDlg.h"

//TES 6/20/2011 - PLID 43700 - Created
// CGlassesOrderHistoryDlg dialog

IMPLEMENT_DYNAMIC(CGlassesOrderHistoryDlg, CNxDialog)

CGlassesOrderHistoryDlg::CGlassesOrderHistoryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CGlassesOrderHistoryDlg::IDD, pParent)
{
	m_nPatientID = -1;
}

CGlassesOrderHistoryDlg::~CGlassesOrderHistoryDlg()
{
}

void CGlassesOrderHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlassesOrderHistoryDlg)
	DDX_Control(pDX, IDOK, m_nxbClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlassesOrderHistoryDlg, CNxDialog)
END_MESSAGE_MAP()


// CGlassesOrderHistoryDlg message handlers
BOOL CGlassesOrderHistoryDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_nxbClose.AutoSet(NXB_CLOSE);

		//TES 6/20/2011 - PLID 43700 - Update the title bar
		// (j.dinatale 2012-04-17 17:26) - PLID 49078 - changed glasses order to optical order
		CString strCaption;
		strCaption.Format("Optical Order History for %s", GetExistingPatientName(m_nPatientID));
		SetWindowText(strCaption);

		m_pBkg = GetDlgItem(IDC_GO_HISTORY_CLR)->GetControlUnknown();
		// (b.spivey, May 21, 2012) - PLID 50558 - We use the default patient blue always.
		m_pBkg->PutColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		//TES 6/20/2011 - PLID 43700 - Load the list of glasses orders for this patient
		m_pList = BindNxDataList2Ctrl(IDC_GLASSES_ORDER_HISTORY_LIST, false);
		m_pList->WhereClause = _bstr_t(FormatString("IsDelete = 0 AND GlassesOrderT.PersonID = %li", m_nPatientID));
		m_pList->Requery();
	

	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}