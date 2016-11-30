// ReconcileMergeMedicationsDlg.cpp : implementation file
//
// (s.dhole 2013-06-18 14:54) - PLID 57219 new Dialog
#include "stdafx.h"
#include "Practice.h"
#include "ReconcileMergeMedicationsDlg.h"


using namespace NXDATALIST2Lib;
#define COLOR_PRESCRIPTION_MED RGB(239,228,176)
#define COLOR_CURRENT_MED_BLUE RGB(153,217,234)

#define COLOR_ACTION_DELETE RGB(222,69,19)
#define COLOR_ACTION_MERGE RGB(0,100,0)
#define COLOR_ACTION_ADD RGB(139,69,19)
#define COLOR_ACTION_DO_NOTHING RGB(0,0,0)
#define COLOR_CURRENT_MED_DISCONTINUED RGB(222, 225, 231)

enum ReconciliationTBColumns
{
	erctcAction= 0,
	erctcName ,
	erctcSig ,
	erctcIsStatus ,
	erctcDate ,
	erctcCurrentMedName ,
	erctcCurrentMedSig ,
	erctcCurrentMedStatus ,
	erctcCurrentMedDate ,
};
// CReconcileMergeMedicationsDlg dialog

IMPLEMENT_DYNAMIC(CReconcileMergeMedicationsDlg, CNxDialog)

CReconcileMergeMedicationsDlg::CReconcileMergeMedicationsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CReconcileMergeMedicationsDlg::IDD, pParent)
{
	m_clrReconcilationBack =0;
	m_ReconciliationType=erMergNone;
}

CReconcileMergeMedicationsDlg::~CReconcileMergeMedicationsDlg()
{
}

void CReconcileMergeMedicationsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, ID_RECONCILIATION_OK, m_btnReconcilationOk);
	DDX_Control(pDX, ID_RECONCILIATION_CANCEL, m_btnReconcilationCancel);
	DDX_Control(pDX, IDC_MEDICATIONS_RECONCILATION_BKG, m_nxcReconcilationBack);	
}
BOOL CReconcileMergeMedicationsDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();
		m_btnReconcilationOk.AutoSet(NXB_OK);
		m_btnReconcilationCancel.AutoSet(NXB_CANCEL);
		m_nxcReconcilationBack.SetColor(m_clrReconcilationBack);
		extern CPracticeApp theApp;
		GetDlgItem(IDC_STATIC_DESCRIPTION)->SetFont(&theApp.m_boldFont);
		m_dlMergeMedRx = BindNxDataList2Ctrl(IDC_LIST_MED_MERGE_RECONCILIATION, false);		
		//IDC_STATIC_DESCRIPTION
		SetColumnHeaders();
		LoadData();
		// (b.spivey, October 30, 2015) PLID 67514 - was an override provided? if so lets set it now. 
		if (!m_strOkButtonOverrideText.IsEmpty()) {
			m_btnReconcilationOk.SetWindowTextA(m_strOkButtonOverrideText); 
		}
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
// (s.dhole 2013-09-13 10:29) - PLID 
void CReconcileMergeMedicationsDlg::UpdateColumnCaption(NXDATALIST2Lib::_DNxDataListPtr ListPtr, int  nColumn, CString strCaption )	
{
	IColumnSettingsPtr pCol = ListPtr->GetColumn(nColumn);
	pCol->PutColumnTitle(_bstr_t( strCaption) );
}

// (s.dhole 2013-09-13 10:29) - PLID 59232
void CReconcileMergeMedicationsDlg::SetColumnHeaders()
{
	
	switch (m_ReconciliationType)
	{
	case erMergMedication:
		{
			SetDlgItemText( IDC_STATIC_DESCRIPTION,	"Please review changes and press OK when done.\nItems in beige are new prescriptions. Items in blue are existing patient current medications. ");
			SetWindowText("Reconcile Medications: Preview and Validation");
			UpdateColumnCaption(m_dlMergeMedRx,erctcAction,"Action");
			UpdateColumnCaption(m_dlMergeMedRx,erctcName,"Name");
			UpdateColumnCaption(m_dlMergeMedRx,erctcSig,"Sig");
			UpdateColumnCaption(m_dlMergeMedRx,erctcIsStatus,"Status");
			UpdateColumnCaption(m_dlMergeMedRx,erctcDate,"Date");
			UpdateColumnCaption(m_dlMergeMedRx,erctcCurrentMedName,"Name");
			UpdateColumnCaption(m_dlMergeMedRx,erctcCurrentMedSig,"Sig");
			UpdateColumnCaption(m_dlMergeMedRx,erctcCurrentMedStatus,"Status");
			UpdateColumnCaption(m_dlMergeMedRx,erctcCurrentMedDate,"Last Update");
			break;
		}
	case erMergAllergy:
		{
			// (s.dhole 2013-10-30 15:12) - PLID 59235
			SetDlgItemText( IDC_STATIC_DESCRIPTION,	"Please review changes and press OK when done.\nItems in beige are new allergies. Items in blue are existing patient current allergies. ");
			SetWindowText("Reconcile Allergies: Preview and Validation");
			UpdateColumnCaption(m_dlMergeMedRx,erctcAction,"Action");
			UpdateColumnCaption(m_dlMergeMedRx,erctcName,"Allergy");
			UpdateColumnCaption(m_dlMergeMedRx,erctcSig,"Note/Reaction");
			UpdateColumnCaption(m_dlMergeMedRx,erctcIsStatus,"Status");
			UpdateColumnCaption(m_dlMergeMedRx,erctcDate,"Date");
			UpdateColumnCaption(m_dlMergeMedRx,erctcCurrentMedName,"Allergy");
			UpdateColumnCaption(m_dlMergeMedRx,erctcCurrentMedSig,"Note/Reaction");
			UpdateColumnCaption(m_dlMergeMedRx,erctcCurrentMedStatus,"Status");
			UpdateColumnCaption(m_dlMergeMedRx,erctcCurrentMedDate,"Last Update");
			break;
		}
	case erMergProblem:
		{
			// (s.dhole 2013-10-30 11:58) - PLID 56933
			SetDlgItemText( IDC_STATIC_DESCRIPTION,	"Please review changes and press OK when done.\nItems in beige are new problems. Items in blue are existing patient current problems. ");
			SetWindowText("Reconcile Problems: Preview and Validation");
			UpdateColumnCaption(m_dlMergeMedRx,erctcAction,"Action");
			UpdateColumnCaption(m_dlMergeMedRx,erctcName,"Problem");
			UpdateColumnCaption(m_dlMergeMedRx,erctcSig,"Description");
			UpdateColumnCaption(m_dlMergeMedRx,erctcIsStatus,"Status");
			UpdateColumnCaption(m_dlMergeMedRx,erctcDate,"Date");
			UpdateColumnCaption(m_dlMergeMedRx,erctcCurrentMedName,"Problem");
			UpdateColumnCaption(m_dlMergeMedRx,erctcCurrentMedSig,"Description");
			UpdateColumnCaption(m_dlMergeMedRx,erctcCurrentMedStatus,"Status");
			UpdateColumnCaption(m_dlMergeMedRx,erctcCurrentMedDate,"Last Update");
			break;
		}
	default:
		break;
	}
}


void CReconcileMergeMedicationsDlg::LoadData()
{
	for (int i=0; i < m_aReconcileValidationItem.GetSize(); i++) {
		IRowSettingsPtr pRow = m_dlMergeMedRx->GetNewRow();
		ReconcileValidationItem  aMergeItem =m_aReconcileValidationItem[i] ;
		switch(aMergeItem.Action){
		case aMergeItem.eAddItem: 
			pRow->PutValue(erctcAction, _bstr_t("Add"));
			pRow->PutForeColor(COLOR_ACTION_ADD); 
			break; 
		case aMergeItem.eDeleteItem:
			pRow->PutValue(erctcAction, _bstr_t("Remove"));
			pRow->PutForeColor(COLOR_ACTION_DELETE); 
			break; 
		case aMergeItem.eMergeCurItem: 
			pRow->PutValue(erctcAction, _bstr_t("Merge"));
			pRow->PutForeColor(COLOR_ACTION_MERGE); 
			break;
		case aMergeItem.eExcludeCurItem: 
			// this should be  "Do nothing " but as per document should be "Remove"
			pRow->PutValue(erctcAction, _bstr_t("Remove"));
			pRow->PutForeColor(COLOR_ACTION_DO_NOTHING); 
			break; 
		case aMergeItem.eKeepCurItem : 
			// this is unchaged record , but as per document will say "Merge"
			pRow->PutValue(erctcAction, _bstr_t("Merge"));
			pRow->PutForeColor(COLOR_ACTION_DO_NOTHING); 
			break; 
		}
		pRow->PutValue(erctcName , _bstr_t(aMergeItem.strName));
		pRow->PutValue(erctcSig , _bstr_t(aMergeItem.strDescription  ));
		if((aMergeItem.dtLastDate.m_status == COleDateTime::invalid) 
			|| (aMergeItem.dtLastDate.m_dt < 1.0)){
			pRow->PutValue(erctcDate ,g_cvarNull);
		}
		else{
			pRow->PutValue(erctcIsStatus ,_variant_t(aMergeItem.bIsActive? "Active": "Discontinued"));
			pRow->PutValue(erctcDate , _variant_t(aMergeItem.dtLastDate,VT_DATE));
		}
		pRow->PutCellBackColor(erctcIsStatus, COLOR_PRESCRIPTION_MED );
		pRow->PutCellBackColor(erctcName, COLOR_PRESCRIPTION_MED );
		pRow->PutCellBackColor(erctcSig, COLOR_PRESCRIPTION_MED);
		pRow->PutCellBackColor(erctcDate,COLOR_PRESCRIPTION_MED );
		if (aMergeItem.strCurrentName==""){
			pRow->PutCellBackColor(erctcCurrentMedStatus,COLOR_CURRENT_MED_BLUE );
		}
		else{
			pRow->PutValue(erctcCurrentMedStatus,_variant_t(aMergeItem.bCurrentIsActive? "Active": "Discontinued"));
			//show back cor if medication is active
			if (aMergeItem.bCurrentIsActive){
				pRow->PutCellBackColor(erctcCurrentMedStatus,COLOR_CURRENT_MED_BLUE );
			}
			else{
				pRow->PutCellBackColor(erctcCurrentMedStatus,COLOR_CURRENT_MED_DISCONTINUED);
			}
		}
		pRow->PutValue(erctcCurrentMedName, _bstr_t(aMergeItem.strCurrentName));
		pRow->PutValue(erctcCurrentMedSig, _bstr_t(aMergeItem.strCurrentDescription ));
		if((aMergeItem.dtCurrentLastDate.m_status == COleDateTime::invalid) 
			|| (aMergeItem.dtCurrentLastDate.m_dt < 1.0)){
			pRow->PutValue(erctcCurrentMedDate, g_cvarNull);
		}
		else{
			pRow->PutValue(erctcCurrentMedDate, _variant_t(aMergeItem.dtCurrentLastDate,VT_DATE));
		}
		pRow->PutCellBackColor(erctcCurrentMedName,COLOR_CURRENT_MED_BLUE );
		pRow->PutCellBackColor(erctcCurrentMedSig, COLOR_CURRENT_MED_BLUE );
		pRow->PutCellBackColor(erctcCurrentMedDate, COLOR_CURRENT_MED_BLUE ) ;
		m_dlMergeMedRx->AddRowSorted(pRow, NULL);
	}
}
BEGIN_MESSAGE_MAP(CReconcileMergeMedicationsDlg, CNxDialog)
	ON_BN_CLICKED(ID_RECONCILIATION_OK, &CReconcileMergeMedicationsDlg::OnBnClickedReconciliationOk)
	ON_BN_CLICKED(ID_RECONCILIATION_CANCEL, &CReconcileMergeMedicationsDlg::OnBnClickedReconciliationCancel)
END_MESSAGE_MAP()


// CReconcileMergeMedicationsDlg message handlers

void CReconcileMergeMedicationsDlg::OnBnClickedReconciliationOk()
{
	try {
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
	}
	NxCatchAll(__FUNCTION__);
}

void CReconcileMergeMedicationsDlg::OnBnClickedReconciliationCancel()
{
	try {
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
	}
	NxCatchAll(__FUNCTION__);
}
