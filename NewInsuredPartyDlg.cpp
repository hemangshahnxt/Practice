// NewInsuredPartyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NewInsuredPartyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewInsuredPartyDlg dialog

using namespace ADODB;
using namespace NXDATALIST2Lib;

enum InsCoComboColumn {

	icccID = 0,
	icccName,
	icccAddress1,
	icccAddress2,
	icccCity,
	icccState,
	icccZip,
};

enum RespTypeListColumn {

	rtlcID = 0,
	rtlcName,
	rtlcPriority,
	rtlcIsInactive,
	rtlcCategoryTypeName,
	rtlcHasRespAlready,
	rtlcColor,
};

CNewInsuredPartyDlg::CNewInsuredPartyDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNewInsuredPartyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewInsuredPartyDlg)
	m_nPatientID = -1;
	m_nDefaultInsCoID = -1;
	m_nDefaultRespTypeID = -2;	//-1 is a default for inactive, -2 means no default
	m_nSelectedInsuranceCoID = -1;
	m_nSelectedRespTypeID = -1;
	//}}AFX_DATA_INIT
}


void CNewInsuredPartyDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewInsuredPartyDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewInsuredPartyDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNewInsuredPartyDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewInsuredPartyDlg message handlers

BOOL CNewInsuredPartyDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-25 13:57) - PLID 29790 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		
		m_InsCoCombo = BindNxDataList2Ctrl(IDC_NEW_INSCO_COMBO, true);
		m_RespTypeList = BindNxDataList2Ctrl(IDC_RESP_TYPE_LIST, false);

		COLORREF clrGray = RGB(128,128,128);

		CString strFromClause;
		// (j.gruber 2012-07-25 14:16) - PLID 51777 - add more categories
		strFromClause.Format("(SELECT ID, TypeName, Priority, "
			"CASE WHEN Priority IN (1,2) THEN 'Medical' WHEN Priority = -1 THEN '' "
			"	ELSE "
			"		CASE WHEN CategoryType = 2 THEN 'Vision' "
			"		WHEN CategoryType = 3 THEN 'Auto' "
			"		WHEN CategoryType = 4 THEN 'Workers'' Comp.' "
			"		WHEN CategoryType = 5 THEN 'Dental' "			
			"		WHEN CategoryType = 6 THEN 'Study' "			
			"		WHEN CategoryType = 7 THEN 'Letter of Protection' "			
			"		WHEN CategoryType = 8 THEN 'Letter of Agreement' "			
			"       ELSE 'Medical' END "
			"	END AS CategoryType, "
			"Convert(bit, CASE WHEN InsuredPartyQ.RespTypeID Is Null OR RespTypeT.Priority = -1 THEN 0 ELSE 1 END) AS HasRespAlready, "
			"CASE WHEN InsuredPartyQ.RespTypeID Is Null OR RespTypeT.Priority = -1 THEN 0 ELSE %li END AS Color "
			"FROM RespTypeT "
			"LEFT JOIN ("
			"	SELECT RespTypeID FROM InsuredPartyT "
			"	WHERE PatientID = %li "
			"	GROUP BY RespTypeID, PatientID "
			") AS InsuredPartyQ ON RespTypeT.ID = InsuredPartyQ.RespTypeID) "
			"AS RespTypeQ", (long)clrGray, m_nPatientID);

		m_RespTypeList->PutFromClause(_bstr_t(strFromClause));
		m_RespTypeList->Requery();

		//set our defaults
		if(m_nDefaultInsCoID != -1) {
			m_InsCoCombo->SetSelByColumn(icccID, (long)m_nDefaultInsCoID);
		}

		TrySelectDefaultResp();
		
	}NxCatchAll("Error in CNewInsuredPartyDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNewInsuredPartyDlg::OnOK() 
{
	try {

		IRowSettingsPtr pInsCoRow = m_InsCoCombo->GetCurSel();

		if(pInsCoRow == NULL) {
			AfxMessageBox("Please select an insurance company.");
			return;
		}

		m_nSelectedInsuranceCoID = VarLong(pInsCoRow->GetValue(icccID));

		IRowSettingsPtr pRespTypeRow = m_RespTypeList->GetCurSel();

		if(pRespTypeRow == NULL) {
			AfxMessageBox("Please select a responsibility placement for this company.");
			return;
		}

		m_nSelectedRespTypeID = VarLong(pRespTypeRow->GetValue(rtlcID));
		long nPriority = VarLong(pRespTypeRow->GetValue(rtlcPriority));

		//rtlcHasRespAlready should be set to false if inactive, since we can
		//have unlimited inactive resps
		BOOL bHasRespAlready = VarBool(pRespTypeRow->GetValue(rtlcHasRespAlready), FALSE);

		//remember that unlimited -1 types are allowed
		if(m_nSelectedRespTypeID != -1 && !bHasRespAlready) {

			//verify the patient hasn't had this resp type entered since we started
			_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID FROM InsuredPartyT "
				"WHERE PatientID = {INT} AND RespTypeID = {INT}", m_nPatientID, m_nSelectedRespTypeID);
			if(!rs->eof) {
				//set the flag, we will warn momentarily
				bHasRespAlready = TRUE;

				//requery for proper coloring
				m_RespTypeList->Requery();
			}
			rs->Close();
		}

		if(bHasRespAlready) {
			AfxMessageBox("The responsibility you selected has already been used by this patient.\n"
				"Please select a responsibility that has not already been used.");

			//try to select a good default
			TrySelectDefaultResp();
			return;
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CNewInsuredPartyDlg::OnCancel() 
{
	if(IDNO == MessageBox("If you do not select an insurance company, an insured party will not be created.\n"
		"Are you sure you wish to cancel?","Practice",MB_ICONQUESTION|MB_YESNO)) {
		return;
	}
	
	CNxDialog::OnCancel();
}


// (j.jones 2010-08-17 15:50) - PLID 40128 - this function will try to
//select the first available resp type, though it will also take into
//account the preferred default resp ID
void CNewInsuredPartyDlg::TrySelectDefaultResp()
{
	try {

		//for a default resp, find the first one in the list that is not in use,
		//try to use our default if we are given one and it is not in use
		//(-1 is a default for inactive, -2 means no default)

		m_RespTypeList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		IRowSettingsPtr pRespRow = m_RespTypeList->GetFirstRow();
		
		long nFirstValidRespFound = -2;
		long nRespIDToUse = -2;

		while(pRespRow && nRespIDToUse == -2) {

			long nRespID = VarLong(pRespRow->GetValue(rtlcID));
			//rtlcHasRespAlready should be set to false if inactive, since we can
			//have unlimited inactive resps
			BOOL bHasRespAlready = VarBool(pRespRow->GetValue(rtlcHasRespAlready), FALSE);

			if(!bHasRespAlready) {

				//set this selection
				nRespIDToUse = nRespID;

				if(nFirstValidRespFound == -2) {
					//track the first resp
					nFirstValidRespFound = nRespID;
				}

				//do we have a default?
				if(m_nDefaultRespTypeID == -2 || m_nDefaultRespTypeID == nRespID) {
					//either we have no default, or this is it, so use it
					nRespIDToUse = nRespID;
				}
			}

			pRespRow = pRespRow->GetNextRow();
		}

		if(nFirstValidRespFound != -2 && nRespIDToUse == -2) {
			//we found a resp, but didn't use it (possibly because
			//we were given a default that we could not use),
			//so use it
			nRespIDToUse = nFirstValidRespFound;
		}

		if(nRespIDToUse == -2) {
			//shouldn't be possible because we should have selected Inactive,
			//but if not, select Inactive now
			nRespIDToUse = -1;
		}

		m_RespTypeList->SetSelByColumn(rtlcID, (long)nRespIDToUse);

	}NxCatchAll(__FUNCTION__);
}