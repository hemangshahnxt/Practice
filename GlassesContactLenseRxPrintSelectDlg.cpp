// GlassesContactLenseRxPrintSelectDlg.cpp : implementation file
  //r.wilson (6/18/2012) plid 50916 

#include "stdafx.h"
#include "Practice.h"
#include "InventoryRc.h"
#include "GlassesContactLenseRxPrintSelectDlg.h"

using namespace NXDATALIST2Lib;

enum ProviderDropDownColumns
{
	eProvID = 0,
	eProviderName,	
};

// GlassesContactLenseRxPrintSelectDlg dialog

IMPLEMENT_DYNAMIC(GlassesContactLenseRxPrintSelectDlg, CNxDialog)

GlassesContactLenseRxPrintSelectDlg::GlassesContactLenseRxPrintSelectDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(GlassesContactLenseRxPrintSelectDlg::IDD, pParent)
{	
	m_nCommonProviderID = -1;
	m_nProviderID = -1;
}

BOOL GlassesContactLenseRxPrintSelectDlg::OnInitDialog()
{
	 //r.wilson (6/18/2012) plid 50916 
	CNxDialog::OnInitDialog();
	try
	{
		m_BtnOk.AutoSet(NXB_PRINT);
		m_BtnCancel.AutoSet(NXB_CANCEL);
		m_BtnPrintPreview.AutoSet(NXB_PRINT_PREV);

		m_bPrintPreview = FALSE;

		
		m_pProviderList = BindNxDataList2Ctrl(this, IDC_NXDATALIST_PROVIDERS, GetRemoteData(), false);	
		m_pProviderList->PutFromClause("ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID");
		//r.wilson - We show all providers even if they are archived
		//m_pProviderList->PutWhereClause("Archived = 0");
				
		m_pProviderList->Requery();
		m_pProviderList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		ParseProviderIDs();

				
		IRowSettingsPtr pNewRow = NULL;
		pNewRow = m_pProviderList->GetNewRow();
		pNewRow->PutValue(eProvID,(long) -1);
		pNewRow->PutValue(eProviderName," < NONE >");

		CWnd* tmpStaticText = GetDlgItem(IDC_STATIC_GLASSES_RX_PROVIDER);
		CString strTmpText = "";
		m_pGlassesProviderLabel.GetWindowTextA(strTmpText);
		if(m_strProviders.GetLength() > 32){
			m_strProviders = m_strProviders.Left(29) + "...";
		}
		m_pGlassesProviderLabel.SetText(strTmpText + m_strProviders);

		tmpStaticText = GetDlgItem(IDC_STATIC_CON_LENS_RX_PROVIDER2);
		m_pContactLensProviderLabel.GetWindowTextA(strTmpText);
		if(m_strCLProviders.GetLength() > 32){
			m_strCLProviders = m_strCLProviders.Left(29) + "...";
		}
		m_pContactLensProviderLabel.SetText(strTmpText + m_strCLProviders);		

		m_pProviderList->AddRowSorted(pNewRow,NULL);		
		m_pProviderList->FindByColumn(eProvID, (long) -1,m_pProviderList->GetTopRow(),TRUE);	

		if(m_nCommonProviderID > 0)
		{
			// //r.wilson (6/18/2012) plid 50916 - Go to the selected provider
			m_pProviderList->SetSelByColumn(eProvID,(long) m_nCommonProviderID);
			m_nProviderID = m_nCommonProviderID;
		}
		else
		{
			m_pProviderList->SetSelByColumn(eProvID,(long) -1);
			m_nProviderID = -1;
		}
		
		
		//r.wilson plid 50916  - If a glasses Rx is not getting printed then disable its checkbox
		if(m_bPrintGlassesRx == FALSE)
		{			
			m_BtnPrintGlassesRx.EnableWindow(FALSE);
			tmpStaticText = (CStatic*) GetDlgItem(IDC_STATIC_GLASSES_RX_PROVIDER);
			tmpStaticText->EnableWindow(FALSE);
		}
		else
		{			
			m_BtnPrintGlassesRx.SetCheck(TRUE);
			CString tmpText = "";
			m_BtnPrintGlassesRx.GetWindowTextA(tmpText);
			tmpText += m_strGlassesExamDate;
			m_BtnPrintGlassesRx.SetWindowTextA(tmpText);
		}	

		//r.wilson plid 50916  - If a contact lens Rx is not getting printed then disable its checkbox
		if(m_bPrintContactLensRx == FALSE)
		{			
			m_BtnPrintConLensRx.EnableWindow(FALSE);
			tmpStaticText = (CStatic*) GetDlgItem(IDC_STATIC_CON_LENS_RX_PROVIDER2);
			tmpStaticText->EnableWindow(FALSE);
		}
		else
		{			
			m_BtnPrintConLensRx.SetCheck(TRUE);
			CString tmpText = "";
			m_BtnPrintConLensRx.GetWindowTextA(tmpText);
			tmpText += m_strConLensExamDate;
			m_BtnPrintConLensRx.SetWindowTextA(tmpText);
		}		

		//r.wilson - Add patient name to the window text
		CString strTempDialogTitle = "";
		GetWindowText(strTempDialogTitle);
		strTempDialogTitle = m_strPatientName + " - " + strTempDialogTitle;
		SetWindowText(strTempDialogTitle);
		m_nSelectedProviderID = VarLong(m_pProviderList->CurSel->GetValue(eProvID),-1);

		//r.wilson
		

	}NxCatchAll(__FUNCTION__);
	return TRUE;  
}

GlassesContactLenseRxPrintSelectDlg::~GlassesContactLenseRxPrintSelectDlg()
{
}

void GlassesContactLenseRxPrintSelectDlg::ParseProviderIDs()
{
	m_nProviderID = -1;
	m_nCLProviderID = -1;

	BOOL bBreak = FALSE;
	bool bFoundMatchingIDs = false;

	int nTokenPos1 = 0;
	int nTokenPos2 = 0;

	CString strProvIDs = "";
	CString strCLProvIDs = "";	

	//r.wilson - if both strings holding the list of provider ID's are empty then lets set the common
	//           providerID to -1 and return.
	if(m_strProviderIDs.IsEmpty() == true && 
		(m_strCLProviderIDs.IsEmpty() == true || m_strCLProviderIDs.Compare(_T("-1")) == 0))
	{
		m_nCommonProviderID = -1;
		return;
	}	
	
	strProvIDs = m_strProviderIDs.Tokenize(_T(";"),nTokenPos1);
	strCLProvIDs = m_strCLProviderIDs.Tokenize(_T(";"),nTokenPos2);		

	int nCounter = 0;
	while(!strProvIDs.IsEmpty())
	{		

		m_nProviderID = atol(strProvIDs);
		if(nCounter == 0){
			m_nCommonProviderID = m_nProviderID;
			if((m_strCLProviderIDs.IsEmpty() == true || m_strCLProviderIDs.Compare(_T("-1")) == 0))
			{
				bFoundMatchingIDs = true;
				break;
			}
		}

		while(!strCLProvIDs.IsEmpty())
		{
			m_nCLProviderID = atol(strCLProvIDs);
			if(m_nProviderID == m_nCLProviderID && m_nProviderID > 0)
			{
				m_nCommonProviderID = m_nProviderID;
				bBreak = TRUE;
				bFoundMatchingIDs = true;
				break;
			}			
			strCLProvIDs = m_strCLProviderIDs.Tokenize(_T(";"),nTokenPos2);
		}

		if(bBreak != FALSE)
		{
			break;
		}
		nCounter++;
		strProvIDs = m_strProviderIDs.Tokenize(_T(";"),nTokenPos1);
		nTokenPos2 = 0;
		strCLProvIDs = m_strCLProviderIDs.Tokenize(_T(";"),nTokenPos2);	
	}

	nTokenPos1 = 0;
	nTokenPos2 = 0;
	strProvIDs = m_strProviderIDs.Tokenize(_T(";"),nTokenPos1);
	strCLProvIDs = m_strCLProviderIDs.Tokenize(_T(";"),nTokenPos2);		
	
	if(!strCLProvIDs.IsEmpty() && 
		strCLProvIDs.Compare(_T("-1")) != 0 && 
		strProvIDs.IsEmpty() &&
		bFoundMatchingIDs == false		
		)
	{
		m_nCLProviderID = atol(strCLProvIDs);
		m_nCommonProviderID = m_nCLProviderID;
	}
	else
	{
		if(m_bPrintContactLensRx != FALSE && bFoundMatchingIDs == false)
		{
			m_nCommonProviderID = -1;
		}
	}
	
}


void GlassesContactLenseRxPrintSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_GLASSES_RX, m_BtnPrintGlassesRx);
	DDX_Control(pDX, IDC_CHECK_CONTACT_LENSE_RX, m_BtnPrintConLensRx);
	DDX_Control(pDX, IDC_PRINT_RX , m_BtnOk);
	DDX_Control(pDX, IDCANCEL , m_BtnCancel);
	DDX_Control(pDX, IDC_BTN_RX_PRINT_PREVIEW , m_BtnPrintPreview);
	DDX_Control(pDX, IDC_STATIC_GLASSES_RX_PROVIDER, m_pGlassesProviderLabel);
	DDX_Control(pDX, IDC_STATIC_CON_LENS_RX_PROVIDER2, m_pContactLensProviderLabel);
}


BEGIN_MESSAGE_MAP(GlassesContactLenseRxPrintSelectDlg, CNxDialog)
	//ON_BN_CLICKED(IDOK, &GlassesContactLenseRxPrintSelectDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_PRINT_RX, &GlassesContactLenseRxPrintSelectDlg::OnBnClickedPrint)	
	ON_BN_CLICKED(IDC_BTN_RX_PRINT_PREVIEW, &GlassesContactLenseRxPrintSelectDlg::OnBnClickedBtnRxPrintPreview)
	ON_BN_CLICKED(IDC_CHECK_GLASSES_RX, &GlassesContactLenseRxPrintSelectDlg::OnBnClickedCheckGlassesRx)
	ON_BN_CLICKED(IDC_CHECK_CONTACT_LENSE_RX, &GlassesContactLenseRxPrintSelectDlg::OnBnClickedCheckContactLenseRx)
END_MESSAGE_MAP()
// GlassesContactLenseRxPrintSelectDlg message handlers

 
//r.wilson (6/18/2012) plid 50916 
void GlassesContactLenseRxPrintSelectDlg::OnBnClickedPrint()
{
	try
	{		

		if(m_bPrintGlassesRx == FALSE && m_bPrintContactLensRx == FALSE)
		{
			MessageBox("Please select a prescription to print.");
			return;
		}	

		if(m_nSelectedProviderID < 0 )
		{		
			if(IDNO == MessageBox("There is no provider selected for this prescription.\n Continue?", "Practice", MB_ICONEXCLAMATION|MB_YESNO))
			{		
				m_bPrintPreview = FALSE;
				return;	
			}		
		}

		OnOK();
	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(GlassesContactLenseRxPrintSelectDlg, CNxDialog)
	ON_EVENT(GlassesContactLenseRxPrintSelectDlg, IDC_NXDATALIST_PROVIDERS, 1, GlassesContactLenseRxPrintSelectDlg::SelChangingNxdatalistProviders, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(GlassesContactLenseRxPrintSelectDlg, IDC_NXDATALIST_PROVIDERS, 16, GlassesContactLenseRxPrintSelectDlg::SelChosenNxdatalistProviders, VTS_DISPATCH)
END_EVENTSINK_MAP()

void GlassesContactLenseRxPrintSelectDlg::SelChangingNxdatalistProviders(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{	
	try {
		//TES 12/10/2010 - Code borrowed from VisionWebOrderDlg... Makes sure that NULL can't be selected on the dropdown
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);

}

void GlassesContactLenseRxPrintSelectDlg::OnBnClickedBtnRxPrintPreview()
{		
	try
	{		

		if(m_bPrintGlassesRx == FALSE && m_bPrintContactLensRx == FALSE)
		{
			MessageBox("Please select a prescription to print.");
			return;
		}			

		if(m_nSelectedProviderID < 0 )
		{		
			if(IDNO == MessageBox("There is no provider selected for this prescription.\n Continue?", "Practice", MB_ICONEXCLAMATION|MB_YESNO))
			{					
				return;	
			}		
		}

		m_bPrintPreview = TRUE;
		OnOK();
	}NxCatchAll(__FUNCTION__);
}

void GlassesContactLenseRxPrintSelectDlg::SelChosenNxdatalistProviders(LPDISPATCH lpRow)
{
	try{
		IRowSettingsPtr pRow(lpRow);

		if(pRow)
		{
			m_nSelectedProviderID = VarLong(pRow->GetValue(eProvID), -1);
		}
	}NxCatchAll(__FUNCTION__);
}

void GlassesContactLenseRxPrintSelectDlg::OnBnClickedCheckGlassesRx()
{
	try
	{
		m_bPrintGlassesRx = m_BtnPrintGlassesRx.GetCheck();
	}NxCatchAll(__FUNCTION__);
}

void GlassesContactLenseRxPrintSelectDlg::OnBnClickedCheckContactLenseRx()
{
	try
	{
		m_bPrintContactLensRx = m_BtnPrintConLensRx.GetCheck();
	}NxCatchAll(__FUNCTION__);
}
