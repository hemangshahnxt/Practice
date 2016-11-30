// PatientEmrByDateDlg.cpp : implementation file
//
// (s.dhole 2010-02-08 13:45) - PLID 37112 Workflow change from room manager -> EMR for doctors.
#include "stdafx.h"
#include "EmrRc.h"
#include "Practice.h"
#include "PatientEmrByDateDlg.h"
#include "GlobalUtils.h"
#include "PatientNexEMRDlg.h"

//Enumeration for the Existing EMR Columns
enum EMRColumns {
	eecPicID = 0,
	eecEMNID,
	eecEMRID,
	eecDate,
	eecInputDate,
	eecProvider,	
	eecEMRDescription,
	eecDescription,
	eecStatus,
};

// CPatientEmrByDateDlg dialog

IMPLEMENT_DYNAMIC(CPatientEmrByDateDlg, CDialog)

CPatientEmrByDateDlg::CPatientEmrByDateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPatientEmrByDateDlg::IDD, pParent)
{

}
BOOL CPatientEmrByDateDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	try {
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		//Initialize datalists
		GetDlgItem(IDOPEN)->EnableWindow(FALSE);
		m_pExistingList = BindNxDataList2Ctrl(this,IDC_EXISTING_LIST_BY_DATE,  GetRemoteData(), false);
		SetWindowText("Edit EMNs: " + m_strTitle);
		m_pExistingList->Clear();
		ReloadEMRList();
		}
	NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



//Call to reload the entire EMR / EMN list.  This function will clear the list and reload it from data.
//	No Parameters
//	No Return Value
//This function catches its own exceptions
void CPatientEmrByDateDlg::ReloadEMRList()
{
	try {
		// (z.manning 2011-05-20 14:59) - PLID 33114 - This now filters on EMR chart permissions
		CString strWhere, strFrom;
		// (j.jones 2011-07-05 17:49) - PLID 44432 - supported custom statuses
		strFrom = FormatString(" (SELECT     EMRGroupsT.ID AS EMRID, EMRMasterT.ID AS EMNID, PicT.ID AS PicID, EMRMasterT.PatientID AS PatID,  "
                      " dbo.GetEmnProviderList(EMRMasterT.ID) AS ProviderName, EMRMasterT.Description, "
                      " EMRStatusListT.Name AS Status, LocationsT.Name AS LocName, EMRMasterT.Date, EMRMasterT.InputDate, EMRMasterT.ModifiedDate,  "
                      " EMRGroupsT.Description AS EmrDescription "
					" FROM EMRGroupsT LEFT OUTER JOIN "
                      " EMRMasterT ON EMRGroupsT.ID = EMRMasterT.EMRGroupID LEFT OUTER JOIN "
					  " PicT ON PicT.EmrGroupID = EMRGroupsT.ID LEFT OUTER JOIN "
                      " LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
					  " LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
					  " LEFT JOIN EMRStatusListT ON EMRMasterT.Status = EMRStatusListT.ID "
					 " WHERE (EMRMasterT.Deleted = 0) AND (EMRGroupsT.Deleted = 0) AND (PicT.IsCommitted = 1 OR "
                      " PicT.IsCommitted IS NULL) AND (PicT.ID IS NOT NULL) %s) as EmrQ "
					  , GetEmrChartPermissionFilter().Flatten());
		strWhere.Format(" date=CAST(FLOOR(CAST( getdate() AS float)) AS datetime) AND  PatID = %li ", m_nPatientID );
		m_pExistingList->FromClause = _bstr_t(strFrom);
		m_pExistingList->WhereClause = _bstr_t(strWhere);
		m_pExistingList->Requery();
		}
	NxCatchAll("Error in ReloadEMRList");
}


CPatientEmrByDateDlg::~CPatientEmrByDateDlg()
{
}

void CPatientEmrByDateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOPEN, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CPatientEmrByDateDlg, CDialog)
	ON_BN_CLICKED(IDOPEN, &CPatientEmrByDateDlg::OnBnClickedOpen)
	ON_BN_CLICKED(IDCANCEL, &CPatientEmrByDateDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CPatientEmrByDateDlg message handlers
BEGIN_EVENTSINK_MAP(CPatientEmrByDateDlg, CDialog)
	ON_EVENT(CPatientEmrByDateDlg, IDC_EXISTING_LIST_BY_DATE, 19, CPatientEmrByDateDlg::LeftClickExistingListByDate, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPatientEmrByDateDlg, IDC_EXISTING_LIST_BY_DATE, 29, CPatientEmrByDateDlg::SelSetExistingListByDate, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CPatientEmrByDateDlg::LeftClickExistingListByDate(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if(!lpRow) 
				return;
		if (nCol==eecDescription)
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				long nEMNID = VarLong(pRow->GetValue(eecEMNID));
				_variant_t varPicID = pRow->GetValue(eecPicID);
				long nPicID = VarLong(varPicID);
				GetMainFrame()->EditEmrRecord(nPicID, nEMNID);
				EndDialog(IDOK) ;
			}
		} 
	NxCatchAll("Error in CPatientEmrByDateDlg::LeftClickExistingListByDate");
}

void CPatientEmrByDateDlg::OnBnClickedOpen()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->GetCurSel();
		if(pRow != NULL) 
			{
				long nEMNID = VarLong(pRow->GetValue(eecEMNID));
				_variant_t varPicID = pRow->GetValue(eecPicID);
				long nPicID = VarLong(varPicID);
				GetMainFrame()->EditEmrRecord(nPicID, nEMNID);
				EndDialog(IDOK) ;
			}
		}
	NxCatchAll("Error in CPatientEmrByDateDlg::OnBnClickedOpen");

	// TODO: Add your control notification handler code here
}

void CPatientEmrByDateDlg::SelSetExistingListByDate(LPDISPATCH lpSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->GetCurSel();
		if(pRow == NULL) 
			{
				GetDlgItem(IDOPEN)->EnableWindow(FALSE);
				}
			else
				{
					GetDlgItem(IDOPEN)->EnableWindow(TRUE);
				}
		} 
	NxCatchAll("Error in CPatientEmrByDateDlg::SelSetExistingListByDate");
	// TODO: Add your message handler code here
}

void CPatientEmrByDateDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	EndDialog(IDCANCEL) ;
	//OnCancel();
}
