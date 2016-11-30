// PrescriptionDosingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "PrescriptionDosingDlg.h"
#include "NxAPI.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include <foreach.h>

// (s.dhole 2012-10-23 ) - PLID 51718   New Dialog Dosing information
// CPrescriptionDosingDlg dialog
using namespace NXDATALIST2Lib;
IMPLEMENT_DYNAMIC(CPrescriptionDosingDlg, CNxDialog)
enum DosageListColumns {
	dlcMedId = 0,
	dlcGCN_SEQNO,
	dlcDosingLegalSingleMaxDosageUnit,
	dlcDosingDiagnosis,
	dlcDosingDoseType,
	dlcDosingLowDosePerDay,
	dlcDosingHighDosePerDay,
	dlcResultUnit,
};
extern CPracticeApp theApp;
void CPrescriptionDosingDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrescriptionEditDlg)
	DDX_Control(pDX, IDC_TXT_DOSE_VALUE, m_nxtxtDoseValue);
	DDX_Control(pDX, IDC_LBL_DOSE_UNIT, m_nxlabelDoseUnit);
	DDX_Control(pDX, IDPRESCRIPTIONDOSECANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_TXT_DOSE_PATIENT_WEIGHT, m_nxtxtWeight);
	DDX_Control(pDX, IDC_RADIO_IS_LB, m_nxbIsLb);
	DDX_Control(pDX, IDC_RADIO_IS_KG, m_nxbIsKg);
	DDX_Control(pDX, IDC_PRESCRIPTION_EDITOR_BKG, m_bkg);
	
	//}}AFX_DATA_MAP
}
BEGIN_EVENTSINK_MAP(CPrescriptionDosingDlg, CNxDialog)
	ON_EVENT(CPrescriptionDosingDlg, IDD_PRESCRIPTION_DOSING_LIST, 16, CPrescriptionDosingDlg::SelChosenPrescriptionDoasingList, VTS_I4)
	ON_EVENT(CPrescriptionDosingDlg, IDD_PRESCRIPTION_DOSING_LIST, 2, CPrescriptionDosingDlg::SelChangedPrescriptionDosingList, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

BEGIN_MESSAGE_MAP(CPrescriptionDosingDlg, CNxDialog)
	ON_BN_CLICKED(IDPRESCRIPTIONDOSECANCEL, &CPrescriptionDosingDlg::OnBnClickedPrescriptiondosecancel)
	ON_EN_CHANGE(IDC_TXT_DOSE_PATIENT_WEIGHT, &CPrescriptionDosingDlg::OnEnChangeTxtDosePatientWeight)
	ON_EN_CHANGE(IDC_TXT_DOSE_VALUE, &CPrescriptionDosingDlg::OnEnChangeTxtDoseValue)
	ON_BN_CLICKED(IDC_RADIO_IS_LB, &CPrescriptionDosingDlg::OnBnClickedRadioIsLb)
	ON_BN_CLICKED(IDC_RADIO_IS_KG, &CPrescriptionDosingDlg::OnBnClickedRadioIsKg)
END_MESSAGE_MAP()


BOOL CPrescriptionDosingDlg::OnInitDialog() 
{	
	try {
		CNxDialog::OnInitDialog();
    	m_btnCancel.AutoSet(NXB_CLOSE);
		m_nxtxtWeight.SetLimitText(4);
		m_nxtxtDoseValue.SetLimitText(4);
		CheckDlgButton(IDC_RADIO_IS_KG, BST_CHECKED);
		m_bkg.SetColor(m_bkgColor);
		m_DosingList = BindNxDataList2Ctrl(IDD_PRESCRIPTION_DOSING_LIST, false);
		g_propManager.CachePropertiesInBulk("PrescriptionDosingDlg", propNumber, 
			"("
			"("
			"Username = '<None>' OR Username = '%s') AND ("
			"Name = 'eRxDosingCalculationWeightUnit' "
			")"
			")", 
			_Q(GetCurrentUserName()));
		SetDlgItemText(IDC_TXT_DOSE_VALUE, "0");
		SetDlgItemText(IDC_TXT_DOSE_PATIENT_WEIGHT, "0");
		CFont *fBold = &theApp.m_boldFont;
		GetDlgItem(IDC_LBL_DOSAGE_MSG)->SetFont(fBold);
		SetWindowText("Medication: " + m_strMedicationName);
		SetDlgItemText(IDC_LBL_DOSAGE_MSG, "No indications are listed for this drug for this patient's age.");
		long IsUseKg = GetRemotePropertyInt("eRxDosingCalculationWeightUnit", 0, 0, GetCurrentUserName());
		if (IsUseKg==0)
		{
			CheckDlgButton(IDC_RADIO_IS_LB, BST_CHECKED);
			CheckDlgButton(IDC_RADIO_IS_KG, BST_UNCHECKED);
		}
		else
		{
			CheckDlgButton(IDC_RADIO_IS_KG , BST_CHECKED);
			CheckDlgButton(IDC_RADIO_IS_LB , BST_UNCHECKED );
		}
		LoadData();
	}NxCatchAll("Error in CPrescriptionDosingDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPrescriptionDosingDlg::LoadData()
{
	try{
		if (m_nMedID>0) {
			NexTech_Accessor::_FDBDosageRangeArrayPtr  searchResults = GetAPI()->GetDosageRange(GetAPISubkey(), GetAPILoginToken(),m_nMedID , m_nAgeInDays);
			//	If our search produced no results, tell the user and bail
			if (searchResults->DosageRanges  == NULL){
				ShowDataFiled(FALSE);
				return;
			}	
			else{
				ShowDataFiled(TRUE);
			}
			Nx::SafeArray<IUnknown *> saryDosageRange(searchResults->DosageRanges );
			if (saryDosageRange.GetCount()>0) {
				ShowDataFiled(TRUE);
			}
			else{
				ShowDataFiled(FALSE);
			}
			// Run through our array and add the meds found to the list
			foreach(NexTech_Accessor::_FDBDosageRangePtr DosageRange, saryDosageRange){
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_DosingList->GetNewRow();
				pRow->PutValue(dlcMedId , DosageRange->MedID );
				pRow->PutValue(dlcGCN_SEQNO , DosageRange->GCN_SEQNO );
				pRow->PutValue(dlcDosingLegalSingleMaxDosageUnit , DosageRange->DosingLegalSingleMaxDosageUnit);
				pRow->PutValue(dlcDosingDoseType , DosageRange->DosingDoseType );
				pRow->PutValue(dlcDosingDiagnosis , DosageRange->DiseaseIdentifier );
				pRow->PutValue(dlcDosingLowDosePerDay , DosageRange->DosingLowFrequencyOfAdministrator  );
				pRow->PutValue(dlcDosingHighDosePerDay , DosageRange->DosingHighFrequencyOfAdministrator );
				pRow->PutValue(dlcResultUnit, DosageRange->ResultDosageUnit );
				
				m_DosingList->AddRowAtEnd(pRow, NULL);
			}
			m_DosingList->CurSel = m_DosingList->GetFirstRow(); 
			CalculateDosage();
		}
		else{
			ShowDataFiled(FALSE);
		}
	}NxCatchAll(__FUNCTION__);
}

void CPrescriptionDosingDlg::ShowDataFiled( BOOL bShowData)
{
	try{
		if (bShowData){
			GetDlgItem(IDD_PRESCRIPTION_DOSING_LIST)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_TXT_DOSE_PATIENT_WEIGHT)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_TXT_DOSE_VALUE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LBL_DOSE_UNIT)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_RADIO_IS_LB)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_RADIO_IS_KG)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LBL_DOSE_PER_WEIGHT)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LBL_PER_WEIGHT)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LBL_DOSAGE_MSG)->ShowWindow(SW_HIDE);
		}
		else{
			GetDlgItem(IDD_PRESCRIPTION_DOSING_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_TXT_DOSE_PATIENT_WEIGHT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_TXT_DOSE_VALUE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LBL_DOSE_UNIT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_IS_LB)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_IS_KG)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LBL_DOSE_PER_WEIGHT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LBL_PER_WEIGHT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LBL_DOSAGE_MSG)->ShowWindow(SW_SHOW);
		}
	}NxCatchAll(__FUNCTION__);

}

void CPrescriptionDosingDlg::CalculateDosage()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow  =m_DosingList->CurSel ;
		if(!pRow  )
		{
			return;
		}
		CString strUnit = VarString(  pRow->GetValue (dlcDosingLegalSingleMaxDosageUnit) ,"");
		CString strResultUnit = VarString(  pRow->GetValue (dlcResultUnit) ,"");
		
		CString strWeight="";
		GetDlgItemText(IDC_TXT_DOSE_PATIENT_WEIGHT, strWeight);
		
		double dblweigh =atof(strWeight) ;
		// convert lb  to  KG
		if  (IsDlgButtonChecked(IDC_RADIO_IS_LB) )
		{
			dblweigh = dblweigh * 0.4535924 ;
		}
		CString strDose="";
 		GetDlgItemText(IDC_TXT_DOSE_VALUE, strDose);
		CString strDoseWeight  = FormatString("%s X %.2f Kg = %.2f %s" , strUnit.MakeLower() ,dblweigh, VarDouble(dblweigh  * atof(strDose)) ,strResultUnit.MakeLower()  );
		m_nxlabelDoseUnit.SetText(strDoseWeight  );
		SetRemotePropertyInt("eRxDosingCalculationWeightUnit", IsDlgButtonChecked(IDC_RADIO_IS_KG), 0, GetCurrentUserName());
	}NxCatchAll(__FUNCTION__);
}

CPrescriptionDosingDlg::CPrescriptionDosingDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPrescriptionDosingDlg::IDD, pParent)
{
	
}

CPrescriptionDosingDlg::~CPrescriptionDosingDlg()
{
}


void CPrescriptionDosingDlg::OnBnClickedPrescriptiondosecancel()
{
	CNxDialog::OnCancel();
}

void CPrescriptionDosingDlg::SelChosenPrescriptionDoasingList(long nRow)
{
	CalculateDosage();
}

void CPrescriptionDosingDlg::OnEnChangeTxtDosePatientWeight()
{
	CalculateDosage();
}

void CPrescriptionDosingDlg::OnEnChangeTxtDoseValue()
{
	CalculateDosage();
}

void CPrescriptionDosingDlg::OnBnClickedRadioIsLb()
{
	CalculateDosage();
}

void CPrescriptionDosingDlg::OnBnClickedRadioIsKg()
{
	CalculateDosage();
}

void CPrescriptionDosingDlg::SelChangedPrescriptionDosingList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);
		if (!pRow)
		{
			NXDATALIST2Lib::IRowSettingsPtr pOldRow(lpOldSel);
			if (pOldRow)
			{
				m_DosingList->CurSel =pOldRow;
			}
		}
		CalculateDosage();
	}NxCatchAll(__FUNCTION__)
}
