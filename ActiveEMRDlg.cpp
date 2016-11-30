// ActiveEMRDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ActiveEMRDlg.h"
#include "EMRPreviewPopupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CActiveEMRDlg dialog


CActiveEMRDlg::CActiveEMRDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CActiveEMRDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CActiveEMRDlg)
	//}}AFX_DATA_INIT

	// (a.walling 2010-01-11 12:11) - PLID 31482
	m_hIconPreview = NULL;
	m_pEMRPreviewPopupDlg = NULL;
}


void CActiveEMRDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CActiveEMRDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_ACTIVE_RECORD_COUNT, m_nxstaticActiveRecordCount);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CActiveEMRDlg, CNxDialog)
	//{{AFX_MSG_MAP(CActiveEMRDlg)
	// (a.walling 2010-01-11 13:21) - PLID 31482
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CActiveEMRDlg message handlers

BOOL CActiveEMRDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (a.walling 2010-01-11 12:11) - PLID 31482
		m_hIconPreview = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_INSPECT), IMAGE_ICON, 16,16, 0);

		// (c.haag 2008-04-30 12:45) - PLID 29790 - NxIconify the close button
		m_btnOK.AutoSet(NXB_CLOSE);

		/*if (!CheckCurrentUserPermissions(bioPatientEMR, sptRead)) {
			return TRUE;
		}*/
		// (a.walling 2007-01-10 17:59) - PLID 22682 - We are removing the permission check and have
		//	the caller be responsible. This way we can avoid multiple prompts for passwords with w/pass

		// (a.walling 2006-10-05 15:44) - PLID 22875 - Create an icon for the dialog in the taskbar if necessary
		//								  PLID 22877 - and respect the preference to not do so
		if (GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true) == 1) {
			HWND hwnd = GetSafeHwnd();
			long nStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			nStyle |= WS_EX_APPWINDOW;
			SetWindowLong(hwnd, GWL_EXSTYLE, nStyle);
		}
		
		m_ProviderCombo = BindNxDataListCtrl(this,IDC_ACTIVE_EMR_PROVIDER_COMBO,GetRemoteData(),true);
		m_List = BindNxDataListCtrl(this,IDC_ACTIVE_EMR_LIST,GetRemoteData(),false);

		IRowSettingsPtr pRow = m_ProviderCombo->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,_bstr_t(" { All Providers }"));
		m_ProviderCombo->InsertRow(pRow,0);

		long nProvID = GetRemotePropertyInt("ActiveEMRListLastProvID",-1,0,GetCurrentUserName(),true);
		m_ProviderCombo->SetSelByColumn(0,(long)nProvID);

		if(m_ProviderCombo->GetCurSel() == -1)
			m_ProviderCombo->SetSelByColumn(0,(long)-1);

		// (a.walling 2010-01-11 14:23) - PLID 31482 - Set up this column to pull in the HICON handle
		IColumnSettingsPtr pCol = m_List->GetColumn(4);
		if (pCol != NULL && m_hIconPreview != NULL) {
			CString strHICON;
			strHICON.Format("%li", m_hIconPreview);
			pCol->FieldName = (LPCTSTR)strHICON;
		}

		SetWindowText("Active EMN Records");

		OnSelChosenEmrProviderCombo(m_ProviderCombo->GetCurSel());
	}
	NxCatchAll("Error in CActiveEMRDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CActiveEMRDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CActiveEMRDlg)
	ON_EVENT(CActiveEMRDlg, IDC_ACTIVE_EMR_PROVIDER_COMBO, 16 /* SelChosen */, OnSelChosenEmrProviderCombo, VTS_I4)
	ON_EVENT(CActiveEMRDlg, IDC_ACTIVE_EMR_LIST, 18 /* RequeryFinished */, OnRequeryFinishedActiveEmrList, VTS_I2)
	ON_EVENT(CActiveEMRDlg, IDC_ACTIVE_EMR_LIST, 3 /* DblClickCell */, OnDblClickCellActiveEmrList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CActiveEMRDlg, IDC_ACTIVE_EMR_LIST, 19, OnLeftClickActiveEmrList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CActiveEMRDlg::OnSelChosenEmrProviderCombo(long nRow) 
{
	try {

		SetDlgItemText(IDC_ACTIVE_RECORD_COUNT,"");
	
		CRect rect;
		GetDlgItem(IDC_ACTIVE_RECORD_COUNT)->GetWindowRect(rect);
		ScreenToClient(rect);
		InvalidateRect(rect);

		if(nRow == -1)
			m_ProviderCombo->SetSelByColumn(0,(long)-1);

		long ProvID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(),0),-1);

		CString str;

		// (j.jones 2011-07-06 09:30) - PLID 44432 - changed this filter to be all non-Finished, non-Locked EMNs
		// (it was previously only Open EMNs, but now we have custom statuses which will also show here)
		if(ProvID == -1)
			str = "EMRMasterT.Status NOT IN (1,2) AND EMRMasterT.Deleted = 0";
		else
			str.Format("EMRMasterT.Status NOT IN (1,2) AND EMRMasterT.Deleted = 0 AND EmrMasterT.ID IN (SELECT EmrID FROM EmrProvidersT WHERE ProviderID = %li AND Deleted = 0)",ProvID);

		// (z.manning 2011-05-20 12:21) - PLID 33114 - Factor in EMR chart permissions
		str += GetEmrChartPermissionFilter().Flatten();

		m_List->PutWhereClause(_bstr_t(str));
		m_List->Requery();

		SetRemotePropertyInt("ActiveEMRListLastProvID",ProvID,0,GetCurrentUserName());

	}NxCatchAll("Error changing provider filter.");
}

void CActiveEMRDlg::OnRequeryFinishedActiveEmrList(short nFlags) 
{
	try {
		long nCount = m_List->GetRowCount();
		SetDlgItemInt(IDC_ACTIVE_RECORD_COUNT,nCount);
		
		CRect rect;
		GetDlgItem(IDC_ACTIVE_RECORD_COUNT)->GetWindowRect(rect);
		ScreenToClient(rect);
		InvalidateRect(rect);
	} NxCatchAll(__FUNCTION__);
}

void CActiveEMRDlg::OnDblClickCellActiveEmrList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex==-1)
		return;

	try {

		_variant_t varPicID = m_List->GetValue(m_List->GetCurSel(),0);

		if(varPicID.vt == VT_NULL) {
			//DRT TODO - This is possible if some records were created in the custom record, then upgraded to NexEMR. - PLID 15353
			AfxMessageBox("This record cannot be opened.  Please try loading it from the patients module.");
			return;
		}

		//DRT 3/30/2006 - PLID 19947 - We need to open the PicID and the EmnID both, so that the correct EMN is opened.
		long nEMNID = VarLong(m_List->GetValue(m_List->GetCurSel(), 2), -1);		

		// (a.walling 2010-01-11 14:59) - PLID 31482
		if (m_pEMRPreviewPopupDlg) {
			m_pEMRPreviewPopupDlg->ShowWindow(SW_HIDE);
		}

		GetMainFrame()->EditEmrRecord(VarLong(varPicID), nEMNID);

	} NxCatchAll("Error editing EMR.");	
}

void CActiveEMRDlg::Refresh()
{
	if(!GetSafeHwnd())
		return;

	if(m_List)
		m_List->Requery();
}

void CActiveEMRDlg::OnOK() 
{	
	CDialog::OnOK();

	DestroyWindow();
}

void CActiveEMRDlg::OnCancel() 
{	
	CDialog::OnCancel();

	DestroyWindow();
}

void CActiveEMRDlg::OnLeftClickActiveEmrList(long nRow, short nCol, long x, long y, long nFlags)
{
	try {
		if (nCol == 4) {
			// (a.walling 2010-01-11 12:53) - PLID 31482 - Open up the EMN preview

			NXDATALISTLib::IRowSettingsPtr pRow = m_List->GetRow(nRow);

			if (pRow == NULL)
				return;

			long nPatID = VarLong(m_List->GetValue(m_List->GetCurSel(), 5), -1);
			long nEMNID = VarLong(m_List->GetValue(m_List->GetCurSel(), 2), -1);
			COleDateTime dtEmnModifiedDate = VarDateTime(m_List->GetValue(m_List->GetCurSel(), 14));

			ShowPreview(nPatID, nEMNID, dtEmnModifiedDate);
		}
	} NxCatchAll(__FUNCTION__);
}

void CActiveEMRDlg::OnDestroy()
{
	// (a.walling 2010-01-11 12:25) - PLID 31482	
	if (m_pEMRPreviewPopupDlg) {
		m_pEMRPreviewPopupDlg->DestroyWindow();
		delete m_pEMRPreviewPopupDlg;
		m_pEMRPreviewPopupDlg = NULL;
	}

	if (m_hIconPreview) {
		DestroyIcon(m_hIconPreview);
	}

	CNxDialog::OnDestroy();
}

// (a.walling 2010-01-11 12:52) - PLID 31482 - Show the emn preview popup
// (z.manning 2012-09-10 12:31) - PLID 52543 - Added modified date
void CActiveEMRDlg::ShowPreview(long nPatID, long nEMNID, COleDateTime dtEmnModifiedDate)
{
	if (nPatID == -1 || nEMNID == -1) {
		return;
	}

	if (m_pEMRPreviewPopupDlg == NULL) {
		// create the dialog!

		// (a.walling 2007-04-13 09:49) - PLID 25648 - Load and initialize our preview popup
		m_pEMRPreviewPopupDlg = new CEMRPreviewPopupDlg(this);
		m_pEMRPreviewPopupDlg->Create(IDD_EMR_PREVIEW_POPUP, this);

		// (a.walling 2010-01-11 12:37) - PLID 31482
		m_pEMRPreviewPopupDlg->RestoreSize("ActiveEMR");
	}
	
	// (j.jones 2009-09-22 11:55) - PLID 31620 - PreviewEMN now takes in an array
	// of all available EMN IDs, but since we haven't opened the dialog yet,
	// we can pass in an empty array.
	// (z.manning 2012-09-10 13:52) - PLID 52543 - Use the new EmnPreviewPopup object
	EmnPreviewPopup emn(nEMNID, dtEmnModifiedDate);
	m_pEMRPreviewPopupDlg->SetPatientID(nPatID, emn);
	m_pEMRPreviewPopupDlg->PreviewEMN(emn, 0);
	
	// (a.walling 2010-01-11 16:20) - PLID 27733 - Only show if it is not already
	if (!m_pEMRPreviewPopupDlg->IsWindowVisible()) {
		m_pEMRPreviewPopupDlg->ShowWindow(SW_SHOWNA);
	}
}