// InsuranceDeductOOPDlg.cpp : implementation file
//

// (j.gruber 2010-07-30 11:08) - PLID 39727 - created
#include "stdafx.h"
#include "Practice.h"
#include "InsuranceDeductOOPDlg.h"
#include "Internationalutils.h"
#include "AuditTrail.h"
#include "GlobalAuditUtils.h"
#include "GlobalFinancialUtils.h"

// CInsuranceDeductOOPDlg dialog

using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CInsuranceDeductOOPDlg, CNxDialog)

// (j.jones 2011-12-23 09:44) - PLID 47013 - added datalist of pay groups
enum PayGroupListColumns {

	pglcID = 0,
	pglcPayGroupID,
	pglcPayGroupName,
	pglcCoinsurance,
	pglcOldCoinsurance,
	pglcDeductibleRemain,
	pglcOldDeductibleRemain,
	pglcDeductibleTotal,
	pglcOldDeductibleTotal,
	pglcOOPRemain,
	pglcOldOOPRemain,
	pglcOOPTotal,
	pglcOldOOPTotal,
	pglcLastModified,
};

CInsuranceDeductOOPDlg::CInsuranceDeductOOPDlg(long nInsuredPartyID, long nPatientID, CString strName, long nColor, CWnd* pParent /*=NULL*/)
	: CNxDialog(CInsuranceDeductOOPDlg::IDD, pParent)
{
	m_nInsuredPartyID = nInsuredPartyID;
	m_nPatientID = nPatientID;
	m_strName = strName;
	m_nColor = nColor;

	m_bDeductiblePerPayGroup = FALSE;
	m_bCoinsuranceChanged = FALSE;
}

CInsuranceDeductOOPDlg::~CInsuranceDeductOOPDlg()
{
}

void CInsuranceDeductOOPDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_RADIO_DEDUCTIBLE_ALL, m_radioAllPayGroups);
	DDX_Control(pDX, IDC_RADIO_DEDUCTIBLE_PER_PAY_GROUP, m_radioPerPayGroup);
	DDX_Control(pDX, IDC_DED_REMAIN_LABEL, m_nxstaticDeductibleRemainLabel);
	DDX_Control(pDX, IDC_TOTAL_DED_LABEL, m_nxstaticTotalDeductibleLabel);
	DDX_Control(pDX, IDC_OOP_REMAIN_LABEL, m_nxstaticOOPRemainLabel);
	DDX_Control(pDX, IDC_TOTAL_OOP_LABEL, m_nxstaticTotalOOPLabel);
	DDX_Control(pDX, IDC_LAST_MODIFIED_LABEL, m_nxstaticLastModifiedLabel);
	DDX_Control(pDX, IDC_DED_OOP_BKG, m_bkg1);
	DDX_Control(pDX, IDC_DED_OOP_BKG2, m_bkg2);
}


BEGIN_MESSAGE_MAP(CInsuranceDeductOOPDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_EN_KILLFOCUS(IDC_DEDUCT_REMAINING, OnEnKillfocusDeductRemaining)
	ON_EN_KILLFOCUS(IDC_DEDUCT_TOTAL, OnEnKillfocusDeductTotal)
	ON_EN_KILLFOCUS(IDC_OOP_REMAINING, OnEnKillfocusOopRemaining)
	ON_EN_KILLFOCUS(IDC_OOP_TOTAL, OnEnKillfocusOopTotal)
	ON_BN_CLICKED(IDC_RADIO_DEDUCTIBLE_ALL, OnDeductibleAll)
	ON_BN_CLICKED(IDC_RADIO_DEDUCTIBLE_PER_PAY_GROUP, OnDeductiblePerPayGroup)
END_MESSAGE_MAP()


// CInsuranceDeductOOPDlg message handlers
BOOL CInsuranceDeductOOPDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_bkg1.SetColor(m_nColor);
		m_bkg2.SetColor(m_nColor);

		// (j.jones 2011-12-22 15:27) - PLID 47013 - supported configuring deductibles/OOP per pay group
		m_PayGroupList = BindNxDataList2Ctrl(IDC_DED_PAY_GROUP_LIST, false);

		m_radioAllPayGroups.SetCheck(TRUE);
		ToggleDisplay();

		Load();

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CInsuranceDeductOOPDlg::Load() 
{
	try {

		m_bDeductiblePerPayGroup = FALSE;
		m_bCoinsuranceChanged = FALSE;

		// (j.jones 2011-12-22 15:27) - PLID 47013 - supported configuring deductibles/OOP per pay group
		CString strFrom;
		//need to show all pay groups, they might not have an entry for this insured party yet
		strFrom.Format("ServicePayGroupsT LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID AND InsuredPartyPayGroupsT.InsuredPartyID = %li ", m_nInsuredPartyID);
		m_PayGroupList->FromClause = _bstr_t(strFrom);
		//they should not be editing coinsurance data on the copay group, so do not show the copay group unless
		//it already has a coinsurance % in it
		m_PayGroupList->WhereClause = _bstr_t("ServicePayGroupsT.Name <> 'Copay' OR InsuredPartyPayGroupsT.Coinsurance Is Not Null");
		m_PayGroupList->Requery();

		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT DeductibleRemaining, TotalDeductible, "
			"OOPRemaining, TotalOOP, LastModifiedDate, DeductiblePerPayGroup "
			"FROM InsuredPartyT WHERE PersonID = {INT}", m_nInsuredPartyID);

		if (!rs->eof) {

			_variant_t var;

			// (j.jones 2011-12-22 15:27) - PLID 47013 - supported configuring deductibles/OOP per pay group
			m_bDeductiblePerPayGroup = VarBool(rs->Fields->Item["DeductiblePerPayGroup"]->Value, FALSE);
			m_radioAllPayGroups.SetCheck(!m_bDeductiblePerPayGroup);
			m_radioPerPayGroup.SetCheck(m_bDeductiblePerPayGroup);

			ToggleDisplay();

			//load everything even if m_bDeductiblePerPayGroup is true, incase they turn it off
			var = rs->Fields->Item["DeductibleRemaining"]->Value;
			if (var.vt == VT_CY) {
				SetDlgItemText(IDC_DEDUCT_REMAINING, FormatCurrencyForInterface(VarCurrency(var)));
				m_strDeductRemain = FormatCurrencyForInterface(VarCurrency(var));
			}
			else {
				m_strDeductRemain = "";
			}

			var = rs->Fields->Item["TotalDeductible"]->Value;
			if (var.vt == VT_CY) {
				SetDlgItemText(IDC_DEDUCT_TOTAL, FormatCurrencyForInterface(VarCurrency(var)));
				m_strDeductTotal = FormatCurrencyForInterface(VarCurrency(var));
			}
			else {
				m_strDeductTotal = "";
			}

			var = rs->Fields->Item["OOPRemaining"]->Value;
			if (var.vt == VT_CY) {
				SetDlgItemText(IDC_OOP_REMAINING, FormatCurrencyForInterface(VarCurrency(var)));
				m_strOOPRemain = FormatCurrencyForInterface(VarCurrency(var));
			}
			else {
				m_strOOPRemain = "";
			}

			var = rs->Fields->Item["TotalOOP"]->Value;
			if (var.vt == VT_CY) {
				SetDlgItemText(IDC_OOP_TOTAL, FormatCurrencyForInterface(VarCurrency(var)));
				m_strOOPTotal = FormatCurrencyForInterface(VarCurrency(var));
			}
			else {
				m_strOOPTotal = "";
			}

			var = rs->Fields->Item["LastModifiedDate"]->Value;
			if (var.vt == VT_DATE) {
				SetDlgItemText(IDC_LST_MOD_DATE, FormatDateTimeForInterface(VarDateTime(var)));
			}		
		}

	}NxCatchAll(__FUNCTION__);
}

_variant_t CInsuranceDeductOOPDlg::GetCurrencyBox(long nID, CString &strBox) 
{

	GetDlgItemText(nID, strBox);
	
	if (strBox.IsEmpty()) {		
		return g_cvarNull;
	}
	else {
		COleCurrency cyBox = ParseCurrencyFromInterface(strBox);
		strBox = FormatCurrencyForInterface(cyBox);
		return _variant_t(cyBox);	
	}
	
}

void CInsuranceDeductOOPDlg::OnBnClickedOk()
{

	long nAuditTransID = -1;
	try {

		BOOL bDeductiblePerPayGroup = m_radioPerPayGroup.GetCheck();

		CString strSqlBatch;
		CNxParamSqlArray aryParams;
		BOOL bAddedDeclaration = FALSE;
		
		// (j.jones 2011-12-23 09:28) - PLID 47013 - regardless of the toggle's value,
		// we always need to update and audit it if it changed
		if(bDeductiblePerPayGroup != m_bDeductiblePerPayGroup) {

			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE InsuredPartyT SET DeductiblePerPayGroup = {INT} WHERE PersonID = {INT}", bDeductiblePerPayGroup ? 1 : 0, m_nInsuredPartyID);

			if(nAuditTransID == -1) {
				nAuditTransID = BeginAuditTransaction();
			}

			CString strOldValue, strNewValue;
			strOldValue = m_bDeductiblePerPayGroup ? "Individual Pay Groups" : "All Pay Groups";
			strNewValue = bDeductiblePerPayGroup ? "Individual Pay Groups" : "All Pay Groups";

			AuditEvent(m_nPatientID, m_strName, nAuditTransID, aeiInsPartyDeductiblePerPayGroup, m_nInsuredPartyID, strOldValue, strNewValue, aepMedium, aetChanged);
		}

		// (j.jones 2011-12-22 17:06) - PLID 47013 - only save the values for the current configuration,
		// do not save values for hidden fields
		if(!bDeductiblePerPayGroup) {

			//save the options for all pay groups

			_variant_t varDeductRemain, varDeductTotal, varOOPRemain, varOOPTotal;
			CString strDeductRemain, strDeductTotal, strOOPRemain, strOOPTotal;

			varDeductRemain = GetCurrencyBox(IDC_DEDUCT_REMAINING, strDeductRemain);
			varDeductTotal = GetCurrencyBox(IDC_DEDUCT_TOTAL, strDeductTotal);
			varOOPRemain = GetCurrencyBox(IDC_OOP_REMAINING, strOOPRemain);
			varOOPTotal = GetCurrencyBox(IDC_OOP_TOTAL, strOOPTotal);

			if (strDeductRemain != m_strDeductRemain ||
				strDeductTotal != m_strDeductTotal ||
				strOOPRemain != m_strOOPRemain ||
				strOOPTotal != m_strOOPTotal) {

					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE InsuredPartyT SET DeductibleRemaining = {VT_CY}, "
						" TotalDeductible = {VT_CY}, "
						" OOPRemaining = {VT_CY}, "
						" TotalOOP = {VT_CY}, "
						" LastModifiedDate = GetDate() "
						" WHERE PersonID = {INT} ",
						varDeductRemain, varDeductTotal, varOOPRemain, varOOPTotal, m_nInsuredPartyID);

				//audit		
				if (strDeductRemain != m_strDeductRemain) {
					if (nAuditTransID == -1) {
						nAuditTransID = BeginAuditTransaction();
					}
					AuditEvent(m_nPatientID, m_strName, nAuditTransID, aeiInsPartyDeductibleRemain, m_nInsuredPartyID, m_strDeductRemain, strDeductRemain, aepMedium, aetChanged);				
				}

				if (strDeductTotal != m_strDeductTotal) {
					if (nAuditTransID == -1) {
						nAuditTransID = BeginAuditTransaction();
					}
					AuditEvent(m_nPatientID, m_strName, nAuditTransID, aeiInsPartyDeductibleTotal, m_nInsuredPartyID, m_strDeductTotal, strDeductTotal, aepMedium, aetChanged);								
				}

				if (strOOPRemain != m_strOOPRemain) {
					if (nAuditTransID == -1) {
						nAuditTransID = BeginAuditTransaction();
					}
					AuditEvent(m_nPatientID, m_strName, nAuditTransID, aeiInsPartyOOPRemain, m_nInsuredPartyID, m_strOOPRemain, strOOPRemain, aepMedium, aetChanged);									
				}

				if (strOOPTotal != m_strOOPTotal) {
					if (nAuditTransID == -1) {
						nAuditTransID = BeginAuditTransaction();
					}
					AuditEvent(m_nPatientID, m_strName, nAuditTransID, aeiInsPartyOOPTotal, m_nInsuredPartyID, m_strOOPTotal, strOOPTotal, aepMedium, aetChanged);
				}
			}
		}
		else {

			//save per pay group

			// (j.jones 2011-12-22 17:07) - PLID 47013 - supported configuring deductibles/OOP per pay group

			//Normally we would not have a param batch in an array that is looped, but in
			//this case we're not going to have a lot of total parameters, as even the most complex
			//setup would never have more than 3 to 5 pay groups for one insured party.
			//99% of the time they are going to be changing just one pay group and the batch size
			//will be identical, thus making this a decent speed improvement.

			IRowSettingsPtr pRow = m_PayGroupList->GetFirstRow();
			while(pRow) {

				long nCurPayGroupID = VarLong(pRow->GetValue(pglcID), -1);
				long nServicePayGroupID = VarLong(pRow->GetValue(pglcPayGroupID));
				CString strPayGroupName = VarString(pRow->GetValue(pglcPayGroupName));

				long nCoinsurance = VarLong(pRow->GetValue(pglcCoinsurance), -1);
				long nOldCoinsurance = VarLong(pRow->GetValue(pglcOldCoinsurance), -1);

				COleCurrency cyInvalid;
				cyInvalid.SetStatus(COleCurrency::invalid);

				COleCurrency cyDeductibleRemain = VarCurrency(pRow->GetValue(pglcDeductibleRemain), cyInvalid);
				COleCurrency cyOldDeductibleRemain = VarCurrency(pRow->GetValue(pglcOldDeductibleRemain), cyInvalid);

				COleCurrency cyDeductibleTotal = VarCurrency(pRow->GetValue(pglcDeductibleTotal), cyInvalid);
				COleCurrency cyOldDeductibleTotal = VarCurrency(pRow->GetValue(pglcOldDeductibleTotal), cyInvalid);

				COleCurrency cyOOPRemain = VarCurrency(pRow->GetValue(pglcOOPRemain), cyInvalid);
				COleCurrency cyOldOOPRemain = VarCurrency(pRow->GetValue(pglcOldOOPRemain), cyInvalid);

				COleCurrency cyOOPTotal = VarCurrency(pRow->GetValue(pglcOOPTotal), cyInvalid);
				COleCurrency cyOldOOPTotal = VarCurrency(pRow->GetValue(pglcOldOOPTotal), cyInvalid);

				//if anything changed, first set up the @CurPayGroupID, since we might
				//be making a new pay group entry and should only do it once for this row
				if(nCoinsurance != nOldCoinsurance ||
					cyDeductibleRemain != cyOldDeductibleRemain ||
					cyDeductibleTotal != cyOldDeductibleTotal ||
					cyOOPRemain != cyOldOOPRemain ||
					cyOOPTotal != cyOldOOPTotal) {

					if(!bAddedDeclaration) {
						AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @CurPayGroupID INT");
						bAddedDeclaration = TRUE;
					}

					if(nCurPayGroupID == -1) {
						//if the pay group doesn't exist, create it
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO InsuredPartyPayGroupsT (InsuredPartyID, PayGroupID) VALUES ({INT}, {INT})", m_nInsuredPartyID, nServicePayGroupID);
						//set the ID
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET @CurPayGroupID = (SELECT Convert(int, SCOPE_IDENTITY()))");
					}
					else {
						//set the ID
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET @CurPayGroupID = {INT}", nCurPayGroupID);
					}
				}

				//if the coinsurance changed, audit using a separate update, so we don't change
				//the DedLastModifiedDate - that's for only when the deductible/OOP info. changes

				if(nCoinsurance != nOldCoinsurance) {

					//track that at least one coinsurance changed
					m_bCoinsuranceChanged = TRUE;

					CString strOldValue = "", strNewValue = "";
					strOldValue.Format("Pay Group: %s Value: ", strPayGroupName);
					strNewValue = strOldValue;

					_variant_t varCoinsurance = g_cvarNull;

					if(nOldCoinsurance != -1) {
						strOldValue += AsString(nOldCoinsurance);
						strOldValue += "%";
					}
					if(nCoinsurance != -1) {

						varCoinsurance = _variant_t(nCoinsurance, VT_I4);

						strNewValue += AsString(nCoinsurance);
						strNewValue += "%";
					}

					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE InsuredPartyPayGroupsT SET Coinsurance = {VT_I4} WHERE ID = @CurPayGroupID", varCoinsurance);

					if(nAuditTransID == -1) {
						nAuditTransID = BeginAuditTransaction();
					}

					AuditEvent(m_nPatientID, m_strName, nAuditTransID, aeiInsPartyPayGroupCoInsurance, m_nInsuredPartyID, strOldValue, strNewValue, aepMedium, aetChanged);
				}

				//if the deductible or OOP changed, that will be a separate update
				//so we also update the last modified date for these fields only
				if (cyDeductibleRemain != cyOldDeductibleRemain ||
					cyDeductibleTotal != cyOldDeductibleTotal ||
					cyOOPRemain != cyOldOOPRemain ||
					cyOOPTotal != cyOldOOPTotal) {

					_variant_t varDeductRemain = g_cvarNull;
					if(cyDeductibleRemain.GetStatus() != COleCurrency::invalid) {
						// (j.jones 2015-11-05 11:39) - PLID 63866 - ensured this is rounded
						RoundCurrency(cyDeductibleRemain);
						varDeductRemain = _variant_t(cyDeductibleRemain);
					}

					_variant_t varDeductTotal = g_cvarNull;
					if(cyDeductibleTotal.GetStatus() != COleCurrency::invalid) {
						// (j.jones 2015-11-05 11:39) - PLID 63866 - ensured this is rounded
						RoundCurrency(cyDeductibleTotal);
						varDeductTotal = _variant_t(cyDeductibleTotal);
					}

					_variant_t varOOPRemain = g_cvarNull;
					if(cyOOPRemain.GetStatus() != COleCurrency::invalid) {
						// (j.jones 2015-11-05 11:39) - PLID 63866 - ensured this is rounded
						RoundCurrency(cyOOPRemain);
						varOOPRemain = _variant_t(cyOOPRemain);
					}

					_variant_t varOOPTotal = g_cvarNull;
					if(cyOOPTotal.GetStatus() != COleCurrency::invalid) {
						// (j.jones 2015-11-05 11:39) - PLID 63866 - ensured this is rounded
						RoundCurrency(cyOOPTotal);
						varOOPTotal = _variant_t(cyOOPTotal);
					}

					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE InsuredPartyPayGroupsT SET "
						" DeductibleRemaining = {VT_CY}, "
						" TotalDeductible = {VT_CY}, "
						" OOPRemaining = {VT_CY}, "
						" TotalOOP = {VT_CY}, "
						" DedLastModifiedDate = GetDate() "
						" WHERE ID = @CurPayGroupID",
						varDeductRemain, varDeductTotal, varOOPRemain, varOOPTotal);

					//audit	using InsuredPartyID as the record ID, which is what regular pay group changes do
					if (cyDeductibleRemain != cyOldDeductibleRemain) {
						if (nAuditTransID == -1) {
							nAuditTransID = BeginAuditTransaction();
						}

						CString strOldValue = "", strNewValue = "";
						strOldValue.Format("Pay Group: %s Value: ", strPayGroupName);
						strNewValue = strOldValue;

						if(cyOldDeductibleRemain.GetStatus() != COleCurrency::invalid) {
							strOldValue += FormatCurrencyForInterface(cyOldDeductibleRemain);
						}
						if(cyDeductibleRemain.GetStatus() != COleCurrency::invalid) {
							strNewValue += FormatCurrencyForInterface(cyDeductibleRemain);
						}

						AuditEvent(m_nPatientID, m_strName, nAuditTransID, aeiInsPartyPayGroupDeductibleRemain, m_nInsuredPartyID, strOldValue, strNewValue, aepMedium, aetChanged);				
					}

					if (cyDeductibleTotal != cyOldDeductibleTotal) {
						if (nAuditTransID == -1) {
							nAuditTransID = BeginAuditTransaction();
						}

						CString strOldValue = "", strNewValue = "";
						strOldValue.Format("Pay Group: %s Value: ", strPayGroupName);
						strNewValue = strOldValue;

						if(cyOldDeductibleTotal.GetStatus() != COleCurrency::invalid) {
							strOldValue += FormatCurrencyForInterface(cyOldDeductibleTotal);
						}
						if(cyDeductibleTotal.GetStatus() != COleCurrency::invalid) {
							strNewValue += FormatCurrencyForInterface(cyDeductibleTotal);
						}

						AuditEvent(m_nPatientID, m_strName, nAuditTransID, aeiInsPartyPayGroupTotalDeductible, m_nInsuredPartyID, strOldValue, strNewValue, aepMedium, aetChanged);								
					}

					if (cyOOPRemain != cyOldOOPRemain) {
						if (nAuditTransID == -1) {
							nAuditTransID = BeginAuditTransaction();
						}

						CString strOldValue = "", strNewValue = "";
						strOldValue.Format("Pay Group: %s Value: ", strPayGroupName);
						strNewValue = strOldValue;

						if(cyOldOOPRemain.GetStatus() != COleCurrency::invalid) {
							strOldValue += FormatCurrencyForInterface(cyOldOOPRemain);
						}
						if(cyOOPRemain.GetStatus() != COleCurrency::invalid) {
							strNewValue += FormatCurrencyForInterface(cyOOPRemain);
						}

						AuditEvent(m_nPatientID, m_strName, nAuditTransID, aeiInsPartyPayGroupOOPRemaining, m_nInsuredPartyID, strOldValue, strNewValue, aepMedium, aetChanged);									
					}

					if (cyOOPTotal != cyOldOOPTotal) {
						if (nAuditTransID == -1) {
							nAuditTransID = BeginAuditTransaction();
						}

						CString strOldValue = "", strNewValue = "";
						strOldValue.Format("Pay Group: %s Value: ", strPayGroupName);
						strNewValue = strOldValue;

						if(cyOldOOPTotal.GetStatus() != COleCurrency::invalid) {
							strOldValue += FormatCurrencyForInterface(cyOldOOPTotal);
						}
						if(cyOOPTotal.GetStatus() != COleCurrency::invalid) {
							strNewValue += FormatCurrencyForInterface(cyOOPTotal);
						}

						AuditEvent(m_nPatientID, m_strName, nAuditTransID, aeiInsPartyPayGroupTotalOOP, m_nInsuredPartyID, strOldValue, strNewValue, aepMedium, aetChanged);
					}
				}

				pRow = pRow->GetNextRow();
			}
		}

		if(!strSqlBatch.IsEmpty()) {

			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

			if(nAuditTransID != -1) {
				CommitAuditTransaction(nAuditTransID);
			}
		}

		CNxDialog::OnOK();

	}NxCatchAllCall(__FUNCTION__,
		if (nAuditTransID != -1) {
			RollbackAuditTransaction(nAuditTransID);
		}
	);
}

void CInsuranceDeductOOPDlg::OnBnClickedCancel()
{
	try {
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}

void CInsuranceDeductOOPDlg::SetCurrencyBox(long nID) {

	CString str;
	GetDlgItemText(nID, str);

	COleCurrency cy = ParseCurrencyFromInterface(str);
	if (cy.GetStatus() == COleCurrency::invalid) {
		SetDlgItemText(nID, "");
	}
	else {
		SetDlgItemText(nID, FormatCurrencyForInterface(cy));
	}
}

void CInsuranceDeductOOPDlg::OnEnKillfocusDeductRemaining()
{
	try {
		SetCurrencyBox(IDC_DEDUCT_REMAINING);
	}NxCatchAll(__FUNCTION__);
}

void CInsuranceDeductOOPDlg::OnEnKillfocusDeductTotal()
{
	try {
		SetCurrencyBox(IDC_DEDUCT_TOTAL);
	}NxCatchAll(__FUNCTION__);
}

void CInsuranceDeductOOPDlg::OnEnKillfocusOopRemaining()
{
	try {
		SetCurrencyBox(IDC_OOP_REMAINING);
	}NxCatchAll(__FUNCTION__);
}

void CInsuranceDeductOOPDlg::OnEnKillfocusOopTotal()
{
	try {
		SetCurrencyBox(IDC_OOP_TOTAL);
	}NxCatchAll(__FUNCTION__);	
}

// (j.jones 2011-12-22 15:27) - PLID 47013 - supported configuring deductibles/OOP per pay group
void CInsuranceDeductOOPDlg::OnDeductibleAll()
{
	try {

		ToggleDisplay();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-12-22 15:27) - PLID 47013 - supported configuring deductibles/OOP per pay group
void CInsuranceDeductOOPDlg::OnDeductiblePerPayGroup()
{
	try {

		ToggleDisplay();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-12-22 15:27) - PLID 47013 - based on the radio button settings,
// this will show/hide either the pay group datalist or the text fields
void CInsuranceDeductOOPDlg::ToggleDisplay()
{
	try {

		BOOL bConfigureForAll = m_radioAllPayGroups.GetCheck();

		//if bConfigureForAll is true, show the text fields, hide the datalist
		//if bConfigureForAll is false, hide the text fields, show the datalist

		m_nxstaticDeductibleRemainLabel.ShowWindow(bConfigureForAll ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_DEDUCT_REMAINING)->ShowWindow(bConfigureForAll ? SW_SHOW : SW_HIDE);

		m_nxstaticTotalDeductibleLabel.ShowWindow(bConfigureForAll ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_DEDUCT_TOTAL)->ShowWindow(bConfigureForAll ? SW_SHOW : SW_HIDE);

		m_nxstaticOOPRemainLabel.ShowWindow(bConfigureForAll ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_OOP_REMAINING)->ShowWindow(bConfigureForAll ? SW_SHOW : SW_HIDE);

		m_nxstaticTotalOOPLabel.ShowWindow(bConfigureForAll ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_OOP_TOTAL)->ShowWindow(bConfigureForAll ? SW_SHOW : SW_HIDE);

		m_nxstaticLastModifiedLabel.ShowWindow(bConfigureForAll ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_LST_MOD_DATE)->ShowWindow(bConfigureForAll ? SW_SHOW : SW_HIDE);
		
		//datalists have to use SW_SHOWNA
		ShowDlgItem(IDC_DED_PAY_GROUP_LIST, bConfigureForAll ? SW_HIDE : SW_SHOWNA);

	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CInsuranceDeductOOPDlg, CNxDialog)
ON_EVENT(CInsuranceDeductOOPDlg, IDC_DED_PAY_GROUP_LIST, 9, OnEditingFinishingDedPayGroupList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

// (j.jones 2011-12-22 15:27) - PLID 47013 - supported configuring deductibles/OOP per pay group
void CInsuranceDeductOOPDlg::OnEditingFinishingDedPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if(pRow && *pbContinue && *pbCommit) {

			CString strPayGroupName = VarString(pRow->GetValue(pglcPayGroupName));
			if(strPayGroupName.CompareNoCase("Copay") == 0 && CString(strUserEntered) != "") {
				//if this is the copay group, warn that they should not be doing this
				if(IDNO == MessageBox("The Copay group should not be used for Coinsurance or Deductible purposes.\n\n"
					"Are you sure you wish to put non-Copay information in the Copay group?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
					*pvarNewValue = g_cvarNull;
					return;
				}
			}

			CString strEntered = strUserEntered;

			switch (nCol) {

				case pglcCoinsurance:
					{
						if (strEntered.IsEmpty()) {
							//put a null value in
							*pvarNewValue = g_cvarNull;
							return;
						}

						//check to make sure its a number
						if (!IsNumeric(strEntered)) {
							*pvarNewValue = g_cvarNull;
							MsgBox("Please enter a valid percentage value.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}

						long nPercent = atoi(strEntered);
						if (nPercent < 0 || nPercent > 100) {
							*pvarNewValue = g_cvarNull;
							MsgBox("Please enter a valid percentage value.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}					
					}

				break;

				case pglcDeductibleRemain:
				case pglcDeductibleTotal:
				case pglcOOPRemain:
				case pglcOOPTotal:
					{
						if (strEntered.IsEmpty()) {
							//put a null value in
							*pvarNewValue = g_cvarNull;
							return;
						}

						//check to make sure its a valid currency
						COleCurrency cyAmt = ParseCurrencyFromInterface(strEntered);
						if (cyAmt.GetStatus() != COleCurrency::valid) {
							*pvarNewValue = g_cvarNull;
							MsgBox("Please enter a valid currency.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}

						if (cyAmt < COleCurrency(0,0)) {
							*pvarNewValue = g_cvarNull;
							MsgBox("Please enter a currency greater than 0.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}
					}

				break;
			}
		}

	}NxCatchAll(__FUNCTION__);
}
