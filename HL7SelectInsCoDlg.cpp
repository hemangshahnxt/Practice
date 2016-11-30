// HL7SelectInsCoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FinancialRc.h"
#include "HL7SelectInsCoDlg.h"

//TES 7/14/2010 - PLID 39635 - Created
// CHL7SelectInsCoDlg dialog

IMPLEMENT_DYNAMIC(CHL7SelectInsCoDlg, CNxDialog)

CHL7SelectInsCoDlg::CHL7SelectInsCoDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CHL7SelectInsCoDlg::IDD, pParent)
{
	m_nInsurancePlacement = -1;
	m_nPatientID = -1;
	m_nInsuranceCoID = -1;
}

CHL7SelectInsCoDlg::~CHL7SelectInsCoDlg()
{
}

void CHL7SelectInsCoDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHL7SelectInsCoDlg)
	DDX_Control(pDX, IDC_CREATE_NEW_INS_CO, m_nxbCreateNew);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
	DDX_Control(pDX, IDC_SELECT_INS_CO_CAPTION, m_nxsCaption);
	DDX_Control(pDX, IDC_INS_CO_NAME, m_nxsName);
	DDX_Control(pDX, IDC_INS_CO_PLACEMENT, m_nxsPlacement);
	DDX_Control(pDX, IDC_INS_CO_ADDRESS_1, m_nxsAddress1);
	DDX_Control(pDX, IDC_INS_CO_ADDRESS_2, m_nxsAddress2);
	DDX_Control(pDX, IDC_INS_CO_CITY, m_nxsCity);
	DDX_Control(pDX, IDC_INS_CO_STATE, m_nxsState);
	DDX_Control(pDX, IDC_INS_CO_ZIP, m_nxsZip);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHL7SelectInsCoDlg, CNxDialog)
	ON_BN_CLICKED(IDC_CREATE_NEW_INS_CO, &CHL7SelectInsCoDlg::OnCreateNewInsCo)
END_MESSAGE_MAP()

enum InsCoListColumns {
	iclcPersonID = 0,
	iclcColor = 1,
	iclcPlacement = 2,
	iclcName = 3,
	iclcAddress1 = 4,
	iclcAddress2 = 5,
	iclcCity = 6,
	iclcState = 7,
	iclcZip = 8,
};

using namespace ADODB;
using namespace NXDATALIST2Lib;
// CHL7SelectInsCoDlg message handlers
BOOL CHL7SelectInsCoDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {

		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);
		m_nxbCreateNew.AutoSet(NXB_NEW);

		//TES 7/14/2010 - PLID 39635 - Fill in the caption text.
		CString strCaption;
		strCaption.Format("The incoming %s patient '%s' has an insurance company that could not be linked to "
			"an existing insurance company in Practice.\r\n"
			"\r\n"
			"Please select an existing Practice insurance company to link this %s insurance company to, "
			"or select 'Create New' to create a new insurance company in Practice using the information from %s.%s",
			m_strGroupName, m_PID.strLast + ", " + m_PID.strFirst + " " + m_PID.strMiddle, 
			m_strGroupName, m_strGroupName,
			m_Insurance.strInsCoID.IsEmpty()?"\r\n\r\nNOTE: This message did not contain an insurance company identifier.  "
			"Therefore, you may receive this prompt again for any future messages related to this insurance company.":"");
		m_nxsCaption.SetWindowText(strCaption);

		//TES 7/14/2010 - PLID 39635 - Fill in the information about the incoming company.
		m_nxsName.SetWindowText(m_Insurance.strInsCoName);
		if(m_nInsurancePlacement == 1) {
			m_nxsPlacement.SetWindowText("Primary");
		}
		else if(m_nInsurancePlacement == 2) {
			m_nxsPlacement.SetWindowText("Secondary");
		}
		else {
			//TES 7/14/2010 - PLID 39635 - See if there's a name for this placement, otherwise just use the number.
			//TES 8/11/2010 - PLID 39635 - We've got the priority, not the ID
			_RecordsetPtr rsRespType = CreateParamRecordset("SELECT TypeName FROM RespTypeT WHERE Priority = {INT}",
				m_nInsurancePlacement);
			if(rsRespType->eof) {
				m_nxsPlacement.SetWindowText(AsString(m_nInsurancePlacement));
			}
			else {
				m_nxsPlacement.SetWindowText(AdoFldString(rsRespType, "TypeName"));
			}
		}
		m_nxsAddress1.SetWindowText(m_Insurance.strInsCoAddress1);
		m_nxsAddress2.SetWindowText(m_Insurance.strInsCoAddress2);
		m_nxsCity.SetWindowText(m_Insurance.strInsCoCity);
		m_nxsState.SetWindowText(m_Insurance.strInsCoState);
		m_nxsZip.SetWindowText(m_Insurance.strInsCoZip);

		m_pInsCoList = BindNxDataList2Ctrl(IDC_NEXTECH_INS_CO_LIST, false);

		//TES 7/14/2010 - PLID 39635 - Fill the list; we use 98 as the RespTypeID for inactive types because we sort by the placement,
		// and the companies that don't have a placement for this patient will use 99.
		CString strFromClause;
		strFromClause.Format("PersonT INNER JOIN InsuranceCoT ON PersonT.ID = InsuranceCoT.PersonID "
			"LEFT JOIN (SELECT InsuranceCoID, PatientID, "
			"Min(CASE WHEN RespTypeID = -1 THEN 98 ELSE RespTypeID END) AS RespTypeID "
			"FROM InsuredPartyT GROUP BY InsuranceCoID, PatientID) InsuredPartyQ ON InsuranceCoT.PersonID = InsuredPartyQ.InsuranceCoID "
			"AND InsuredPartyQ.PatientID = %li", m_nPatientID);
		m_pInsCoList->FromClause = _bstr_t(strFromClause);

		IColumnSettingsPtr pCol = m_pInsCoList->GetColumn(iclcColor);
		//TES 7/14/2010 - PLID 39635 - Color the primary and secondary insurance companies for this patient, same colors as the PaymentDlg.
		pCol->PutFieldName(_bstr_t(FormatString("CASE WHEN InsuredPartyQ.RespTypeID = 1 THEN %li "
			"WHEN InsuredPartyQ.RespTypeID = 2 THEN %li ELSE 0 END", RGB(192,0,0), RGB(0,0,128))));
		if(m_nPatientID == -1) {
			//TES 7/14/2010 - PLID 39635 - The placements are all the same, so don't have it sort by them,
			// that way they can type in an insurance name.
			pCol = m_pInsCoList->GetColumn(iclcPlacement);
			pCol->PutSortPriority((long)-1);
		}


		m_pInsCoList->Requery();

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHL7SelectInsCoDlg::OnOK()
{
	try {
		IRowSettingsPtr pRow = m_pInsCoList->CurSel;
		if(pRow) {
			//TES 7/14/2010 - PLID 39635 - Output the selected company.
			m_nInsuranceCoID = VarLong(pRow->GetValue(iclcPersonID));
			m_strInsuranceCoName = VarString(pRow->GetValue(iclcName));
			CNxDialog::OnOK();
		}
		else {
			MsgBox("Please select an insurance company from the list");
			return;
		}
	}NxCatchAll(__FUNCTION__);
}

void CHL7SelectInsCoDlg::OnCancel()
{
	try {
		m_nInsuranceCoID = -1;
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}
void CHL7SelectInsCoDlg::OnCreateNewInsCo()
{
	try {
		//TES 7/14/2010 - PLID 39635 - Our caller will handle creating the company.
		m_nInsuranceCoID = -2;
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}
