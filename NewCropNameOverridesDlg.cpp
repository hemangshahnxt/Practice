// NewCropNameOverridesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NewCropNameOverridesDlg.h"
#include "NewCropUtils.h"

// (j.jones 2011-06-17 08:39) - PLID 44157 - created

// CNewCropNameOverridesDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

IMPLEMENT_DYNAMIC(CNewCropNameOverridesDlg, CNxDialog)

// (j.jones 2010-07-29 10:53) - PLID 39880 - added ability to override provider & user names
enum OverrideNameListColumns {
	onlcProviderID = 0,
	onlcUserID,
	onlcPersonType,
	onlcPracticeName,
	onlcOldLastName,
	onlcNewLastName,
	onlcOldSuffix,
	onlcNewSuffix,
	onlcOldFirstName,
	onlcNewFirstName,
	onlcOldMiddleName,
	onlcNewMiddleName,
	onlcOldPrefix,
	onlcNewPrefix,
};

CNewCropNameOverridesDlg::CNewCropNameOverridesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNewCropNameOverridesDlg::IDD, pParent)
{

}

CNewCropNameOverridesDlg::~CNewCropNameOverridesDlg()
{
}

void CNewCropNameOverridesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CHECK_SUPPRESS_SUFFIX, m_checkSuppressSuffix);
}


BEGIN_MESSAGE_MAP(CNewCropNameOverridesDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CNewCropNameOverridesDlg, CNxDialog)
	ON_EVENT(CNewCropNameOverridesDlg, IDC_NEWCROP_SETUP_NAME_OVERRIDE_LIST, 9, OnEditingFinishingNewcropSetupNameOverrideList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

// CNewCropNameOverridesDlg message handlers

BOOL CNewCropNameOverridesDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		g_propManager.CachePropertiesInBulk("CNewCropNameOverridesDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'NewCropHideProviderSuffix' "
			")",
			_Q(GetCurrentUserName()));

		// (j.jones 2010-07-29 10:36) - PLID 39880 - added ability to override provider & user names
		m_OverrideNameList = BindNxDataList2Ctrl(IDC_NEWCROP_SETUP_NAME_OVERRIDE_LIST, false);

		CString strOverrideNameListFromClause = "("
			"SELECT ProvidersT.PersonID AS ProviderID, NULL AS UserID, "
			"'Provider' AS PersonType, "
			"CASE WHEN Prefix Is Not Null AND Prefix <> '' THEN Prefix + ' ' ELSE '' END "
			"+ First + ' ' + "
			"CASE WHEN Middle <> '' THEN Middle + ' ' ELSE '' END "
			"+ Last + "
			"CASE WHEN Title <> '' THEN ', ' + Title ELSE '' END AS PersonName, "
			"NewCropPrefixOver, NewCropSuffixOver, NewCropFirstOver, NewCropMiddleOver, NewCropLastOver "
			"FROM PersonT "
			"INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
			"LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID "
			"WHERE PersonT.ID > 0 AND PersonT.Archived = 0 "
			""
			"UNION ALL "
			""
			"SELECT NULL AS ProviderID, UsersT.PersonID AS UserID, "
			"'User' AS PersonType, "
			"CASE WHEN Prefix Is Not Null AND Prefix <> '' THEN Prefix + ' ' ELSE '' END "
			"+ First + ' ' + "
			"CASE WHEN Middle <> '' THEN Middle + ' ' ELSE '' END "
			"+ Last + "
			"CASE WHEN Title <> '' THEN ', ' + Title ELSE '' END AS PersonName, "
			"NewCropPrefixOver, NewCropSuffixOver, NewCropFirstOver, NewCropMiddleOver, NewCropLastOver "
			"FROM PersonT "
			"INNER JOIN UsersT ON PersonT.ID = UsersT.PersonID "
			"LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID "
			"WHERE PersonT.ID > 0 AND PersonT.Archived = 0 "
			") AS PersonsQ";

		m_OverrideNameList->PutFromClause(_bstr_t(strOverrideNameListFromClause));
		m_OverrideNameList->Requery();

		// (j.jones 2010-04-13 16:09) - PLID 38183 - added ability to suppress the suffix
		m_checkSuppressSuffix.SetCheck(GetRemotePropertyInt("NewCropHideProviderSuffix", 0, 0, "<None>", true) == 1);
		
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CNewCropNameOverridesDlg::OnOk()
{
	try {

		// (j.jones 2010-04-13 16:09) - PLID 38183 - added ability to suppress the suffix
		SetRemotePropertyInt("NewCropHideProviderSuffix", m_checkSuppressSuffix.GetCheck() ? 1 : 0, 0, "<None>");

		CString strSqlBatch;
		CNxParamSqlArray args;
		
		strSqlBatch = BeginSqlBatch();

		// (j.jones 2010-07-29 10:53) - PLID 39880 - added ability to override provider & user names
		NXDATALIST2Lib::IRowSettingsPtr pOverrideRow = m_OverrideNameList->GetFirstRow();		
		while(pOverrideRow) {

			long nProviderID = VarLong(pOverrideRow->GetValue(onlcProviderID), -1);
			long nUserID = VarLong(pOverrideRow->GetValue(onlcUserID), -1);

			CString strOldLastName = VarString(pOverrideRow->GetValue(onlcOldLastName), "");
			CString strNewLastName = VarString(pOverrideRow->GetValue(onlcNewLastName), "");
			CString strOldSuffix = VarString(pOverrideRow->GetValue(onlcOldSuffix), "");
			CString strNewSuffix = VarString(pOverrideRow->GetValue(onlcNewSuffix), "");
			CString strOldFirstName = VarString(pOverrideRow->GetValue(onlcOldFirstName), "");
			CString strNewFirstName = VarString(pOverrideRow->GetValue(onlcNewFirstName), "");
			CString strOldMiddleName = VarString(pOverrideRow->GetValue(onlcOldMiddleName), "");
			CString strNewMiddleName = VarString(pOverrideRow->GetValue(onlcNewMiddleName), "");
			CString strOldPrefix = VarString(pOverrideRow->GetValue(onlcOldPrefix), "");
			CString strNewPrefix = VarString(pOverrideRow->GetValue(onlcNewPrefix), "");

			//check each value, if anything changed, just update all
			if(strOldLastName != strNewLastName
				|| strOldSuffix !=  strNewSuffix
				|| strOldFirstName != strNewFirstName
				|| strOldMiddleName != strNewMiddleName
				|| strOldPrefix != strNewPrefix) {

				if(nUserID != -1) {
					//this is a user
					AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE UsersT "
						"SET NewCropPrefixOver = {STRING}, NewCropSuffixOver = {STRING}, "
						"NewCropFirstOver = {STRING}, NewCropMiddleOver = {STRING}, "
						"NewCropLastOver = {STRING} "
						"WHERE PersonID = {INT}",
						strNewPrefix, strNewSuffix, strNewFirstName, strNewMiddleName, strNewLastName,
						nUserID);
				}
				else if(nProviderID != -1) {
					//this is a provider
					AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE ProvidersT "
						"SET NewCropPrefixOver = {STRING}, NewCropSuffixOver = {STRING}, "
						"NewCropFirstOver = {STRING}, NewCropMiddleOver = {STRING}, "
						"NewCropLastOver = {STRING} "
						"WHERE PersonID = {INT}",
						strNewPrefix, strNewSuffix, strNewFirstName, strNewMiddleName, strNewLastName,
						nProviderID);
				}
			}

			pOverrideRow = pOverrideRow->GetNextRow();
		}

		if (!strSqlBatch.IsEmpty()) {
			// (e.lally 2009-06-21) PLID 34680 - Leave as execute batch
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-07-29 10:53) - PLID 39880 - added ability to override provider & user names
void CNewCropNameOverridesDlg::OnEditingFinishingNewcropSetupNameOverrideList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//warn about invalid data, mainly suffix & prefix, it should
		//actually be impossible to get the length limit validations
		//if the datalist length enforcement works

		CString strNewValue = strUserEntered;

		if(nCol == onlcNewLastName) {
			if(strNewValue.GetLength() > 35) {
				AfxMessageBox("A last name cannot exceed 35 characters in length.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}
		}
		else if(nCol == onlcNewFirstName) {
			if(strNewValue.GetLength() > 35) {
				AfxMessageBox("A first name cannot exceed 35 characters in length.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}
		}
		else if(nCol == onlcNewMiddleName) {
			if(strNewValue.GetLength() > 35) {
				AfxMessageBox("A middle name cannot exceed 35 characters in length.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}
		}
		else if(nCol == onlcNewSuffix) {

			if(strNewValue.GetLength() > 10) {
				AfxMessageBox("A title/suffix cannot exceed 10 characters in length.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}

			if(!strNewValue.IsEmpty() && !VerifyIsNewCropValidNameSuffix(strNewValue)) {
				AfxMessageBox("The title/suffix you entered is not a valid suffix for E-Prescribing.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}
		}
		else if(nCol == onlcNewPrefix) {

			if(strNewValue.GetLength() > 10) {
				AfxMessageBox("A prefix cannot exceed 10 characters in length.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}

			if(!strNewValue.IsEmpty() && !VerifyIsNewCropValidNamePrefix(strNewValue)) {
				AfxMessageBox("The prefix you entered is not a valid prefix for E-Prescribing.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}
		}

	}NxCatchAll(__FUNCTION__);
}