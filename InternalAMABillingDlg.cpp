// InternalAMABillingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InternalAMABillingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CInternalAMABillingDlg dialog

enum eListColumns {
	elcPersonID = 0,
	elcLast,
	elcFirst,
	elcCurrentAMAVersion,
	elcPrevAMAVersion,
};


CInternalAMABillingDlg::CInternalAMABillingDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInternalAMABillingDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInternalAMABillingDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CInternalAMABillingDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInternalAMABillingDlg)
	DDX_Control(pDX, IDC_AMA_FROM, m_pickerFrom);
	DDX_Control(pDX, IDC_AMA_TO, m_pickerTo);
	DDX_Control(pDX, IDC_AMA_COUNT, m_nxstaticAmaCount);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInternalAMABillingDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInternalAMABillingDlg)
	ON_BN_CLICKED(IDC_AMA_COMMIT, OnAmaCommit)
	ON_BN_CLICKED(IDC_AMA_COMPARE, OnAmaCompare)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInternalAMABillingDlg message handlers

BOOL CInternalAMABillingDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_pClientList = BindNxDataListCtrl(this, IDC_AMA_LIST, GetRemoteData(), false);

	} NxCatchAll("Error in OnInitDialog");

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInternalAMABillingDlg::OnAmaCompare() 
{
	try {

/*	DRT - This is the old method, looking at the currently installed release vs what the client last had marked in AMABillingHistoryT.

		//Generate the query to find and compare the old and new AMA versions
		CString strFrom;
		strFrom.Format("NxClientsT "
			"LEFT JOIN ReleasedVersionsT ON NxClientsT.VersionCurrent = ReleasedVersionsT.ID "
			"LEFT JOIN PersonT ON NxClientsT.PersonID = PersonT.ID "
			"LEFT JOIN "
			"	(SELECT PersonID, MAX(AMAVersion) AS LastAMAVersion "
			"	FROM AMABillingHistoryT "
			"	WHERE Date < DATEADD(dd, 1, '%s') "
			"	GROUP BY PersonID "
			"	) LastVersionQ "
			"ON PersonT.ID = LastVersionQ.PersonID ", strDate);

		CString strWhere;
		strWhere.Format("NxClientsT.VersionCurrent IS NOT NULL "
			"AND (ReleasedVersionsT.AMAVersion <> LastVersionQ.LastAMAVersion "
			"OR (LastVersionQ.PersonID IS NULL AND ReleasedVersionsT.AMAVersion <> ''))");
*/

		//DRT 7/2/2007 - PLID 26522 - New method.  AMA_AuthorizedVersionsT is what the NxLicenseActivationServer has authorized via the download
		//	feature now in Practice.  We will use that as the "currently installed" and compare to what has been previously billed.  Note
		//	that we don't really need to keep the billing history anymore (AMA_AuthorizedVersionsT is a complete history), but it seems
		//	safest to keep track of what we submitted to the AMA, just in case any questions arise.
		CString strStartDate, strToDate;
		strStartDate = FormatDateTimeForSql(VarDateTime(m_pickerFrom.GetValue()));
		strToDate = FormatDateTimeForSql(VarDateTime(m_pickerTo.GetValue()));

		CString strFrom;
		strFrom.Format("AMA_AuthorizedVersionsT  "
			"LEFT JOIN  "
			"	(SELECT PersonID, MAX(AMAVersion) AS LastAMAVersion  "
			"	FROM AMABillingHistoryT  "
			"	GROUP BY PersonID  "
			"	) LastVersionQ  "
			"ON AMA_AuthorizedVersionsT.PersonID = LastVersionQ.PersonID "
			"LEFT JOIN NxClientsT ON AMA_AuthorizedVersionsT.PersonID = NxClientsT.PersonID "
			"LEFT JOIN PersonT ON NxClientsT.PersonID = PersonT.ID ");

		CString strWhere;
		strWhere.Format("(LastVersionQ.LastAMAVersion IS NULL OR AMA_AuthorizedVersionsT.AMAVersion <> LastVersionQ.LastAMAVersion) "
			"AND AMA_AuthorizedVersionsT.Date >= '%s' AND AMA_AuthorizedVersionsT.Date < '%s'", strStartDate, strToDate);



		m_pClientList->PutFromClause(_bstr_t(strFrom));
		m_pClientList->PutWhereClause(_bstr_t(strWhere));

		m_pClientList->Requery();


	} NxCatchAll("Error in OnAmaCompare()");
}

void CInternalAMABillingDlg::OnAmaCommit() 
{
	try {
		//make sure something is in our client list
		if(m_pClientList->GetRowCount() <= 0) {
			AfxMessageBox("You must have rows in the client list before committing.");
			return;
		}

		//hardcoded user requirement.  For safety purposes, I'm hardcoding this to just a few people
		CString strCurUser = GetCurrentUserName();
		if(strCurUser.CompareNoCase("k.majeed") == 0 || 
			strCurUser.CompareNoCase("d.thompson") == 0 || 
			strCurUser.CompareNoCase("m.clark") == 0) {
				//these 3 people are valid
		}
		else {
			AfxMessageBox("You are not verified to commit this data, sorry.  Please contact an administrator to save this data.");
			return;
		}

		//confirm they want to do it.
		if(AfxMessageBox("Are you absolutely sure you wish to do this?  The commit cannot be undone.\r\n\r\n"
			"Make sure you have copied the information out of the above box, this dialog will be dismissed after you commit the data.", MB_YESNO) == IDNO)
			return;


		//now commit the data
		CString strInsert;

		long p = m_pClientList->GetFirstRowEnum();
		LPDISPATCH pDisp = NULL;
		while (p) {
			m_pClientList->GetNextRowEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);

			//info we'll need...
			long nPersonID = VarLong(pRow->GetValue(elcPersonID));
			long nCurAMA = VarLong(pRow->GetValue(elcCurrentAMAVersion));

			//Generate a string
			CString str;
			str.Format("INSERT INTO AMABillingHistoryT (AMAVersion, PersonID, Date) values ('%li', %li, GetDate());\r\n", nCurAMA, nPersonID);

			//add to our total insert
			strInsert += str;

			pDisp->Release();
		}

		//commit data
		// (a.walling 2010-09-08 13:16) - PLID 40377 - Use CSqlTransaction
		try {
			CSqlTransaction trans;

			trans.Begin();
			ExecuteSqlStd(strInsert);
			trans.Commit();

			AfxMessageBox("Successfully committed!");
			OnOK();
		} NxCatchAll("Error committing data!");

	} NxCatchAll("Error in OnAMACommit()");
}

BEGIN_EVENTSINK_MAP(CInternalAMABillingDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInternalAMABillingDlg)
	ON_EVENT(CInternalAMABillingDlg, IDC_AMA_LIST, 18 /* RequeryFinished */, OnRequeryFinishedAmaList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInternalAMABillingDlg::OnRequeryFinishedAmaList(short nFlags) 
{
	try {

		long nCount = m_pClientList->GetRowCount();

		CString str;
		str.Format("%li clients.", nCount);
		SetDlgItemText(IDC_AMA_COUNT, str);

	} NxCatchAll("Error in OnRequeryFinishedAMAList()");
}

void CInternalAMABillingDlg::OnOK() 
{
	CDialog::OnOK();
}

void CInternalAMABillingDlg::OnCancel() 
{
	CDialog::OnCancel();
}
