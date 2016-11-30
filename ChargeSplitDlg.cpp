// ChargeSplitDlg.cpp : implementation file
//

// (j.dinatale 2012-01-05 17:19) - PLID 47341 - Created

#include "stdafx.h"
#include "Practice.h"
#include "ChargeSplitDlg.h"
#include "EmrColors.h"
#include "EMNCharge.h"


// (j.jones 2012-01-26 11:09) - PLID 47700 - added a modular function for getting the responsibility dropdown
// combo box source, so it can be shared across multiple dialogs that assign resps to charges
CString GetEMRChargeRespAssignComboBoxSource(long nPatientID, OPTIONAL long nEMNID /*= -1*/)
{
	//nPatientID would be -1 if called on a template, though we should never really
	//be getting to this code on a template. If we do, the only option is unassigned.
	if(nPatientID == -1) {
		return "-2;< Unassigned >;";
	}

	CSqlFragment sqlInsInfo;
	if(nEMNID == -1){
		sqlInsInfo.Create(
			"SELECT InsuredPartyT.PersonID, InsuranceCoT.Name, RespTypeT.TypeName, RespTypeT.ID AS RespTypeID FROM "
			"InsuredPartyT "
			"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE PatientID = {INT} AND RespTypeT.Priority <> -1 "
			"ORDER BY Coalesce(RespTypeT.CategoryPlacement,1000), RespTypeT.CategoryType, RespTypeT.Priority ", nPatientID);
	}else{
		// (j.dinatale 2012-01-24 16:36) - PLID 47341 - need to not look at deleted charges here
		sqlInsInfo.Create(
			"SELECT InsuredPartyT.PersonID, InsuranceCoT.Name, RespTypeT.TypeName, RespTypeT.ID AS RespTypeID FROM "
			"InsuredPartyT "
			"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"LEFT JOIN "
			"( "
			"	SELECT DISTINCT InsuredPartyID "
			"	FROM EMRChargesT "
			"	LEFT JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
			"	WHERE EMRID = {INT} AND InsuredPartyID IS NOT NULL AND Deleted = 0 "
			") EMRChargedInsQ ON InsuredPartyT.PersonID = EMRChargedInsQ.InsuredPartyID "
			"WHERE PatientID = {INT} AND (RespTypeT.Priority <> -1 OR EMRChargedInsQ.InsuredPartyID IS NOT NULL) "
			"ORDER BY Coalesce(RespTypeT.CategoryPlacement,1000), RespTypeT.CategoryType, RespTypeT.Priority ", nEMNID, nPatientID);
	}

	// (j.dinatale 2012-01-19 14:16) - PLID 47341 - need to include inactive insured parties if they are assigned to a charge on this EMR
	ADODB::_RecordsetPtr rs = CreateParamRecordset(sqlInsInfo);
		

	CString strComboSource = "-2;< Unassigned >;-1;Patient Responsibility;";
	while(!rs->eof){
		long nInsPartyID = AdoFldLong(rs, "PersonID", -1);
		CString strInsCoName = AdoFldString(rs, "Name","");
		CString strRespTypeName = AdoFldString(rs, "TypeName","");

		CString strAppend;
		strAppend.Format("%li;%s (%s);", nInsPartyID, strInsCoName, strRespTypeName);

		strComboSource += strAppend;

		rs->MoveNext();
	}
	rs->Close();

	return strComboSource;
}

// CChargeSplitDlg dialog

IMPLEMENT_DYNAMIC(CChargeSplitDlg, CNxDialog)

CChargeSplitDlg::CChargeSplitDlg(ChrgSptDlgEnums::SupportedTypes stPtrType, CArray<long,long> &arypChargeObjects, long nPatientID, long nEMNID, BOOL bReadOnly, CWnd* pParent /*=NULL*/)
	: CNxDialog(CChargeSplitDlg::IDD, pParent)
{ 
	m_nPatientID = nPatientID; // need this later on for our query to construct a datalist's embedded combo box

	m_bReadOnly = bReadOnly;
	m_stPtrType = stPtrType;

	// (j.dinatale 2012-01-19 14:20) - PLID 47341
	m_nEMNID = nEMNID;

	// (j.dinatale 2012-01-12 14:55) - PLID 47483
	m_bHasChanges = false;

	// load our array with 
	int nSize = arypChargeObjects.GetSize();
	for(int i = 0; i < nSize; i++){
		m_arypChargeObjects.Add(arypChargeObjects.GetAt(i));
	}
}

CChargeSplitDlg::~CChargeSplitDlg()
{
}

void CChargeSplitDlg::OnDestroy()
{
	try{
		m_arypChargeObjects.RemoveAll();
		m_nPatientID = -1;
	}NxCatchAll(__FUNCTION__);

	// (a.walling 2012-01-26 13:24) - PLID 47814 - Need to call base class when handling OnDestroy!
	__super::OnDestroy();
}

void CChargeSplitDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CHARGESPLITNXCOLOR, m_NxColor);
}


BEGIN_MESSAGE_MAP(CChargeSplitDlg, CNxDialog)
	ON_BN_CLICKED(IDCANCEL, &CChargeSplitDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CChargeSplitDlg::OnBnClickedOk)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CChargeSplitDlg message handlers
BOOL CChargeSplitDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
		m_NxColor.SetColor(EmrColors::Topic::PatientBackground());

		// (j.dinatale 2012-01-16 10:12) - PLID 47341 - moved the label text to code because it was too long for resources
		GetDlgItem(IDC_CHG_SPLIT_INS_LABEL)->SetWindowText(
			"This window will allow you to assign responsibilities to individual EMN charges. When this EMN is billed, "
			"this will generate a bill for each responsible party. If any charges have not been assigned a responsibility "
			"when this EMN is billed, you will be asked to assign responsibilities before any bill is created. If the EMN "
			"is locked, assigning charge responsibilities will have to occur through Patients to be Billed.");

		m_pChargeList = BindNxDataList2Ctrl(IDC_CHARGE_RESP_LIST, false);
		m_pChargeList->ReadOnly = m_bReadOnly;

		SetupList();
	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChargeSplitDlg::SetupList()
{
	if(m_nPatientID <= 0){
		return;
	}

	int nSize = m_arypChargeObjects.GetSize();

	NXDATALIST2Lib::IColumnSettingsPtr pColumn = m_pChargeList->GetColumn(csdcNewInsPartyID);

	// (j.jones 2012-01-26 11:09) - PLID 47700 - moved this code into a global function,
	// defined in this file, but exposed for use elsewhere
	CString strComboSource = GetEMRChargeRespAssignComboBoxSource(m_nPatientID, m_nEMNID);
	pColumn->PutComboSource(_bstr_t(strComboSource));
	pColumn->Editable = g_cvarTrue;

	for(int i = 0; i < nSize; i++){
		// add rows here
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pChargeList->GetNewRow();
		pRow->PutValue(csdcID, m_arypChargeObjects.GetAt(i));

		// (j.dinatale 2012-01-12 14:49) - PLID 47483 - need to handle either an EMNCharge or an EMNChargeSummary object
		if(m_stPtrType == ChrgSptDlgEnums::EMNCharge){
			EMNCharge *pCharge = (EMNCharge *)m_arypChargeObjects.GetAt(i);
			pRow->PutValue(csdcCode,  _variant_t(pCharge->strCode));
			pRow->PutValue(csdcSubCode,  _variant_t(pCharge->strSubCode));
			pRow->PutValue(csdcDescription, _variant_t(pCharge->strDescription));
			pRow->PutValue(csdcQuantity, pCharge->dblQuantity);
			pRow->PutValue(csdcUnitCost, _variant_t(pCharge->cyUnitCost));
			pRow->PutValue(csdcOldInsPartyID, pCharge->nInsuredPartyID);
			pRow->PutValue(csdcNewInsPartyID, pCharge->nInsuredPartyID);
		}else{
			if(m_stPtrType == ChrgSptDlgEnums::EMNChargeSummary){
				EMNChargeSummary *pCharge = (EMNChargeSummary *)m_arypChargeObjects.GetAt(i);
				pRow->PutValue(csdcCode,  _variant_t(pCharge->strCode));
				pRow->PutValue(csdcSubCode,  _variant_t(pCharge->strSubCode));
				pRow->PutValue(csdcDescription, _variant_t(pCharge->strDescription));
				pRow->PutValue(csdcQuantity, pCharge->dblQuantity);
				pRow->PutValue(csdcUnitCost, _variant_t(pCharge->cyUnitCost));
				pRow->PutValue(csdcOldInsPartyID, pCharge->nInsuredPartyID);
				pRow->PutValue(csdcNewInsPartyID, pCharge->nInsuredPartyID);
			}else{
				// this means a new type was added, and never supported...
				ASSERT(FALSE);
			}
		}

		m_pChargeList->AddRowAtEnd(pRow, NULL);
	}
}

void CChargeSplitDlg::OnBnClickedCancel()
{
	return CNxDialog::OnCancel();
}

void CChargeSplitDlg::OnBnClickedOk()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pChargeList->GetFirstRow();
		bool bChargeChanged = false;

		// loop through each row and see if the user decided to change any of the Insured Party IDs
		while(pRow){
			long nChargeObjPtr = VarLong(pRow->GetValue(csdcID), 0);
			if(nChargeObjPtr){
				long nOldInsPartyID = VarLong(pRow->GetValue(csdcOldInsPartyID), -2);
				long nNewInsPartyID = VarLong(pRow->GetValue(csdcNewInsPartyID), -2);

				// if the assignment changed, then we must signify as such and assign the new ID to the charge
				if(nOldInsPartyID != nNewInsPartyID){
					bChargeChanged = true;

					// (j.dinatale 2012-01-12 14:49) - PLID 47483 - need to handle either an EMNCharge or an EMNChargeSummary object.
					if(m_stPtrType == ChrgSptDlgEnums::EMNCharge){
						EMNCharge *pCharge = (EMNCharge *)nChargeObjPtr;
						pCharge->nInsuredPartyID = nNewInsPartyID;
					}else{
						if(m_stPtrType == ChrgSptDlgEnums::EMNChargeSummary){
							EMNChargeSummary *pCharge = (EMNChargeSummary *)nChargeObjPtr;
							pCharge->nInsuredPartyID = nNewInsPartyID;
						}else{
							// this means a new type was added, and never supported...
							ASSERT(FALSE);
						}
					}
				}
			}

			pRow = pRow->GetNextRow();
		}

		// (j.dinatale 2012-01-12 14:52) - PLID 47483 - always return OK
		// if something changed, we need to set our member variable as such
		m_bHasChanges = bChargeChanged;
	}NxCatchAll(__FUNCTION__);

	// if we end up here, we arent sure what happened, so assume something changed
	return CNxDialog::OnOK();
}

// (j.dinatale 2012-01-12 14:54) - PLID 47483 - return if dialog has changes
bool CChargeSplitDlg::ChargesChanged()
{
	return m_bHasChanges;
}