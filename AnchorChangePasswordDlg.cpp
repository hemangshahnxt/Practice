#include "stdafx.h"
#include "AnchorChangePasswordDlg.h"
#include "GlobalNexWebUtils.h"
#include "AuditSupportDlg.h"
#include <NxPracticeSharedLib\SupportUtils.h>

CAnchorChangePasswordDlg::CAnchorChangePasswordDlg(long nPersonID, CString strUserName, CWnd* pParent)
	: CNxChangePasswordDlg(nPersonID, strUserName, pParent, boost::bind(&CAnchorChangePasswordDlg::GenerateComplexity, this))
{
}

CAnchorChangePasswordDlg::~CAnchorChangePasswordDlg(void)
{
}

// (j.armen 2013-10-03 07:07) - PLID 57914 - Return that the password change completed
bool CAnchorChangePasswordDlg::UpdateData(const CString &strPass)
{
	CAuditSupportDlg dlg(this);
	if (dlg.DoModal() != IDCANCEL)
	{
		SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "Anchor Password", "<Encrypted>", "<Encrypted>", _Q(dlg.m_strText));

		_variant_t vtAnchorPassword = EncryptStringToVariant(strPass);

		// (j.armen 2013-10-03 07:32) - PLID 57914 - Execute Sql
		ExecuteParamSql(R"(
UPDATE NxClientsT
	SET AnchorPassword = {VARBINARY}
WHERE PersonID = {INT})",
			vtAnchorPassword,
			m_nPersonID);

		return true;
	}
	else
	{
		return false;
	}
}

BOOL CAnchorChangePasswordDlg::OnInitDialog()
{
	try
	{
		__super::OnInitDialog();

		SetWindowText("Change Anchor Password");
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

NexWebPasswordComplexity* CAnchorChangePasswordDlg::GenerateComplexity()
{
	NexWebPasswordComplexity* pRules = new NexWebPasswordComplexity();
	pRules->nMinLength = 25;
	pRules->nMinLetters = 5;
	pRules->nMinLower = 0;
	pRules->nMinNumbers = 10;
	pRules->nMinUpper = 3;
	return pRules;
}