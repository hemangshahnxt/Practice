// BlankAssignmentBenefitsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BlankAssignmentBenefitsDlg.h"

// CBlankAssignmentBenefitsDlg dialog

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (j.jones 2010-07-26 11:45) - PLID 34105 - created

// (j.jones 2010-07-23 14:09) - PLID 34105 - this function returns true/false if at least one
// HCFA group exists that can send claims with the "Assignments Of Benefits" not filled
// (j.jones 2010-07-27 09:19) - PLID 39854 - added an enum parameter, and moved from
// GlobalFinancialUtils to this class, it's global still, you just have to include this
// class to use it, both for using enums, and to remind developers that you shouldn't call
// this function without providing the ability to open this dialog
BOOL CanAssignmentOfBenefitsBeBlank(BlankAssignmentBenefitsType babtType /*= babtBothForms*/)
{
	//"Assignment Of Benefits" controls whether the insurance company sends a check to the
	//office or to the patient. On a HCFA, this is the signature in Box 13. If Box 13 is blank,
	//then the check is sent to the patient. This is also ANSI Loop 2300 CLM08.

	//Box 13 is filled on HCFA claims based on a "Fill Box 13" setting which can Always fill,
	//Never fill, fill if Accept Assignment, or fill if they don't Accept Assignment.
	//There is also an override per bill, but we will NOT check for that here.

	//Also it is safe to check by HCFA Group as we do not allow viewing, printing, or exporting
	//any claims for insurance companies that do not have a HCFA Group.

	//InsuranceAcceptedT should exist for all Providers and all Insurance Companies, if it doesn't
	//then everywhere in code should assume it is accepted. PLID 39783 ensured that.

	//this warning should display even if something inactive qualifies

	// (c.haag 2010-10-01 12:05) - PLID 40763 - Optimized the query
	CString strHCFA =
		//are any groups configured to never fill Box 13?
		"SELECT TOP 1 HCFASetupT.ID FROM HCFASetupT "
		"INNER JOIN InsuranceCoT ON HCFASetupT.ID = InsuranceCoT.HCFASetupGroupID "		
		"WHERE Box13Accepted = 0 "
		//are they configured to fill when accepted, but at least one provider/insurance combination for the group is not accepted?
		"UNION SELECT TOP 1 HCFASetupT.ID FROM HCFASetupT  "
		"INNER JOIN InsuranceCoT ON HCFASetupT.ID = InsuranceCoT.HCFASetupGroupID "
		"INNER JOIN InsuranceAcceptedT ON InsuranceAcceptedT.InsuranceCoID = InsuranceCoT.PersonID "
		"WHERE Box13Accepted = 1 AND InsuranceAcceptedT.Accepted = 0 "
		//are they configured to fill when not accepted, but at least one provider/insurance combination for the group is accepted?
		//InsuranceAcceptedT can potentially be missing for a provider, and if so,
		//we assume it IS accepted
		"UNION SELECT TOP 1 HCFASetupT.ID FROM HCFASetupT  "
		"INNER JOIN InsuranceCoT ON HCFASetupT.ID = InsuranceCoT.HCFASetupGroupID "
		"CROSS JOIN ProvidersT "
		"LEFT JOIN InsuranceAcceptedT ON InsuranceCoT.PersonID = InsuranceAcceptedT.InsuranceCoID AND ProvidersT.PersonID = InsuranceAcceptedT.ProviderID "
		"WHERE Box13Accepted = 2 AND (InsuranceAcceptedT.Accepted Is Null OR InsuranceAcceptedT.Accepted = 1) ";

	// (j.jones 2010-07-27 09:30) - PLID 39854 - added UB support
	// (j.jones 2010-11-23 11:06) - PLID 41592 - optimized the query
	CString strUB =
		//are any groups configured to always fill Box 53 with N?
		"SELECT TOP 1 UB92SetupT.ID FROM UB92SetupT "
		"INNER JOIN InsuranceCoT ON UB92SetupT.ID = InsuranceCoT.UB92SetupGroupID "		
		"WHERE Box53Accepted = 2 "
		//are they configured to fill Y when accepted, but at least one provider/insurance combination for the group is not accepted?
		"UNION SELECT TOP 1 UB92SetupT.ID FROM UB92SetupT  "
		"INNER JOIN InsuranceCoT ON UB92SetupT.ID = InsuranceCoT.UB92SetupGroupID "
		"INNER JOIN InsuranceAcceptedT ON InsuranceAcceptedT.InsuranceCoID = InsuranceCoT.PersonID "
		"WHERE Box53Accepted = 0 AND InsuranceAcceptedT.Accepted = 0 ";

	// (j.jones 2010-07-27 09:30) - PLID 39854 - support the enum to search for HCFA, UB, or both

	CString strSql;
	if(babtType == babtHCFA) {
		strSql = strHCFA;
	}
	else if(babtType == babtUB) {
		strSql = strUB;
	}
	else {
		strSql.Format("%s UNION ALL %s", strHCFA, strUB);
	}
	
	_RecordsetPtr rs = CreateRecordsetStd(strSql);

	return !rs->eof;
}

enum ListColumns {

	lcInsCoName = 0,
	lcClaimType,
	lcGroupName,
	lcProblem,
	lcProvider,
};

IMPLEMENT_DYNAMIC(CBlankAssignmentBenefitsDlg, CNxDialog)

CBlankAssignmentBenefitsDlg::CBlankAssignmentBenefitsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CBlankAssignmentBenefitsDlg::IDD, pParent)
{

}

CBlankAssignmentBenefitsDlg::~CBlankAssignmentBenefitsDlg()
{
}

void CBlankAssignmentBenefitsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BLANK_ASG_BEN_HCFA_LABEL, m_nxstaticHCFALabel);
	DDX_Control(pDX, IDC_BLANK_ASG_BEN_UB_LABEL, m_nxstaticUBLabel);
}


BEGIN_MESSAGE_MAP(CBlankAssignmentBenefitsDlg, CNxDialog)
END_MESSAGE_MAP()


// CBlankAssignmentBenefitsDlg message handlers
BOOL CBlankAssignmentBenefitsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		m_btnClose.AutoSet(NXB_CLOSE);

		m_List = BindNxDataList2Ctrl(IDC_BLANK_ASG_BEN_LIST, false);

		// (j.jones 2010-07-27 09:58) - PLID 39854 - if filtering
		// on one claim type, hide the claim type column, then hide
		// the opposite label
		if(m_babtType == babtHCFA) {
			IColumnSettingsPtr pCol = m_List->GetColumn(lcClaimType);
			if(pCol) {
				pCol->PutColumnStyle(csVisible | csFixedWidth);
				pCol->PutStoredWidth(0);
			}

			m_nxstaticUBLabel.ShowWindow(SW_HIDE);
		}
		else if(m_babtType == babtUB) {
			IColumnSettingsPtr pCol = m_List->GetColumn(lcClaimType);
			if(pCol) {
				pCol->PutColumnStyle(csVisible | csFixedWidth);
				pCol->PutStoredWidth(0);
			}

			m_nxstaticHCFALabel.ShowWindow(SW_HIDE);
		}

		//this list should include inactive entries

		//the from clause is in code just for readability, ease of changes		
		CString strFromClause, strHCFA, strUB;
		
		strHCFA =
			//first list all companies in all HCFA Groups that have Box 13 set to Never fill in
			"SELECT InsuranceCoT.Name AS InsCoName, 'HCFA' AS ClaimType, HCFASetupT.Name AS GroupName, "
			"'Box 13 is set to Never fill' AS Problem, '' AS ProvName "
			"FROM InsuranceCoT "
			"INNER JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
			"WHERE Box13Accepted = 0 "
			//now show all companies & providers that do not accept assignment, when the company
			//is in a HCFA Group that only fills Box 13 when accept assignment is true
			"UNION ALL "
			"SELECT InsuranceCoT.Name AS InsCoName, 'HCFA' AS ClaimType, HCFASetupT.Name AS GroupName, "
			"'Box 13 is set to fill If Accepted, and the Insurance Co. is not accepted by this Provider' AS Problem, "
			"Last + ', ' + First + ' ' + Middle AS ProvName "
			"FROM InsuranceCoT "
			"INNER JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
			"INNER JOIN InsuranceAcceptedT ON InsuranceCoT.PersonID = InsuranceAcceptedT.InsuranceCoID "
			"INNER JOIN PersonT ON InsuranceAcceptedT.ProviderID = PersonT.ID "
			"WHERE Box13Accepted = 1 AND InsuranceAcceptedT.Accepted = 0 "
			//and lastly show all companies & providers that accept assignment, when the company
			//is in a HCFA Group that only fills Box 13 when accept assignment is false
			//(which is ridiculous and should never happen, but whatever)
			"UNION ALL "
			"SELECT InsuranceCoT.Name AS InsCoName, 'HCFA' AS ClaimType, HCFASetupT.Name AS GroupName, "
			"'Box 13 is set to fill If Not Accepted, and the Insurance Co. is accepted by this Provider' AS Problem, "
			"Last + ', ' + First + ' ' + Middle AS ProvName "
			"FROM InsuranceCoT "
			"INNER JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
			//InsuranceAcceptedT can potentially be missing for a provider, and if so,
			//we assume it IS accepted
			"CROSS JOIN ProvidersT "
			"LEFT JOIN InsuranceAcceptedT ON InsuranceCoT.PersonID = InsuranceAcceptedT.InsuranceCoID AND ProvidersT.PersonID = InsuranceAcceptedT.ProviderID "
			"INNER JOIN PersonT ON InsuranceAcceptedT.ProviderID = PersonT.ID "
			"WHERE Box13Accepted = 2 AND (InsuranceAcceptedT.Accepted Is Null OR InsuranceAcceptedT.Accepted = 1) ";

		// (j.jones 2010-07-27 09:37) - PLID 39854 - added UB support
		strUB =
			//first list all companies in all UB Groups that have Box 53 set to always fill with N
			"SELECT InsuranceCoT.Name AS InsCoName, 'UB' AS ClaimType, UB92SetupT.Name AS GroupName, "
			"'Box 53 is set to Always No' AS Problem, '' AS ProvName "
			"FROM InsuranceCoT "
			"INNER JOIN UB92SetupT ON InsuranceCoT.UB92SetupGroupID = UB92SetupT.ID "
			"WHERE Box53Accepted = 2 "
			//now show all companies & providers that do not accept assignment, when the company
			//is in a UB Group that only fills Box 53 with Y when accept assignment is true
			"UNION ALL "
			"SELECT InsuranceCoT.Name AS InsCoName, 'UB' AS ClaimType, UB92SetupT.Name AS GroupName, "
			"'Box 53 is set to Use Accepted Status, and the Insurance Co. is not accepted by this Provider' AS Problem, "
			"Last + ', ' + First + ' ' + Middle AS ProvName "
			"FROM InsuranceCoT "
			"INNER JOIN UB92SetupT ON InsuranceCoT.UB92SetupGroupID = UB92SetupT.ID "
			"INNER JOIN InsuranceAcceptedT ON InsuranceCoT.PersonID = InsuranceAcceptedT.InsuranceCoID "
			"INNER JOIN PersonT ON InsuranceAcceptedT.ProviderID = PersonT.ID "
			"WHERE Box53Accepted = 0 AND InsuranceAcceptedT.Accepted = 0 ";

		// (j.jones 2010-07-27 09:44) - PLID 39854 - support the enum to search for HCFA, UB, or both
		CString strSql;
		if(m_babtType == babtHCFA) {
			strSql = strHCFA;
		}
		else if(m_babtType == babtUB) {
			strSql = strUB;
		}
		else {
			strSql.Format("%s UNION ALL %s", strHCFA, strUB);
		}

		strFromClause.Format("(%s) AS Q", strSql);

		m_List->PutFromClause(_bstr_t(strFromClause));
		m_List->Requery();

	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}