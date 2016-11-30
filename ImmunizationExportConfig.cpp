// ImmunizationExportConfig.cpp : implementation file
//

// (d.singleton 2013-06-05 14:01) - PLID 57057 - added new dialog to choose hl7 group id, facility name, facility id for immunization hl7 export

#include "stdafx.h"
#include "Practice.h"
#include "ImmunizationExportConfig.h"
#include "UTSSearchDlg.h"

using namespace NXDATALIST2Lib;

// CImmunizationExportConfig dialog

enum HL7GroupID
{
	ID = 0,
	Name,
};

IMPLEMENT_DYNAMIC(CImmunizationExportConfig, CNxDialog)

CImmunizationExportConfig::CImmunizationExportConfig(CWnd* pParent /*=NULL*/)
	: CNxDialog(CImmunizationExportConfig::IDD, pParent)
{
	m_strFacilityName = "";
	m_strFacilityID = "";
	m_nHL7GroupID = -1;
}

CImmunizationExportConfig::~CImmunizationExportConfig()
{
}

void CImmunizationExportConfig::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCANCEL);
}


BEGIN_MESSAGE_MAP(CImmunizationExportConfig, CNxDialog)
	ON_BN_CLICKED(IDOK, &CImmunizationExportConfig::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CImmunizationExportConfig::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	try{
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCANCEL.AutoSet(NXB_CANCEL);

		m_dlHL7GroupID = BindNxDataList2Ctrl(IDC_IMMUNIZATION_EXPORT_HL7_GROUP, GetRemoteData(), true);		
		m_dlHL7GroupID->SetSelByColumn(ID, (long)m_nHL7GroupID);

		SetDlgItemText(IDC_IMMUNIZATION_EXPORT_FACILITY_ID, m_strFacilityID);
		SetDlgItemText(IDC_IMMUNIZATION_EXPORT_FACILITY_NAME, m_strFacilityName);
	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

// CImmunizationExportConfig message handlers
BEGIN_EVENTSINK_MAP(CImmunizationExportConfig, CNxDialog)
END_EVENTSINK_MAP()

void CImmunizationExportConfig::OnBnClickedOk()
{
	try {
		//make sure we have a valid row selected
		IRowSettingsPtr pRow = m_dlHL7GroupID->GetCurSel();
		if(!pRow) {
			AfxMessageBox("Please select a valid HL7 Group before saving.");
			return;
		}
		//get values
		long nHl7GroupID = VarLong(pRow->GetValue(ID), -1);
		CString strFacilityID, strFacilityName;
		GetDlgItemText(IDC_IMMUNIZATION_EXPORT_FACILITY_ID, strFacilityID);
		GetDlgItemText(IDC_IMMUNIZATION_EXPORT_FACILITY_NAME, strFacilityName);
		//only save if we have changes
		if(nHl7GroupID != m_nHL7GroupID) {
			SetRemotePropertyInt("HL7ImmunizationHL7GroupID", nHl7GroupID, 0, "<None>");
			m_nHL7GroupID = nHl7GroupID;
		}
		if(strFacilityID.Compare(m_strFacilityID) != 0) {
			SetRemotePropertyText("HL7ImmunizationFacilityID", strFacilityID, 0, "<None>");
		}
		if(strFacilityName.Compare(m_strFacilityName) != 0) {
			SetRemotePropertyText("HL7ImmunizationFacilityName", strFacilityName, 0, "<None>");
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);	
}