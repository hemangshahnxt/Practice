#include "stdafx.h"
#include "NexWebChangePasswordDlg.h"
#include "GlobalNexWebUtils.h"
// (f.gelderloos 2013-08-07 12:20) - PLID 57914 Creating new child-class for the password window
CNexWebChangePasswordDlg::CNexWebChangePasswordDlg(long nPersonID, CString strUserName, CWnd* pParent)
	: CNxChangePasswordDlg(nPersonID, strUserName, pParent, boost::bind(&CNexWebChangePasswordDlg::GenerateComplexity, this))
{
}

CNexWebChangePasswordDlg::~CNexWebChangePasswordDlg(void)
{
}

// (j.armen 2013-10-03 07:08) - PLID 57914 - Do our validation here instead of in the super class
bool CNexWebChangePasswordDlg::UpdateData(const CString &strPass)
{

	// (b.savon 2012-09-05 16:44) - PLID 52464 - Validate NexWeb password from user/office defined complexity defintion
	if (!ValidateNexWebPasswordComplexity(GetRemoteData(), strPass)){
		CString strBanner = GetPasswordRequirementBanner();
		MessageBox("The password does not conform to the NexWeb complexity rules, please correct this." +
			(!strBanner.IsEmpty() ? "  The password complexity rules are as follows:\r\n\r\n" + strBanner : ""), "Password Complexity Violation",
			MB_ICONINFORMATION);
		return false;
	}

	//now check that they are an appropriate length
	//(e.lally 2009-01-28) PLID 32814 - Expanded the Password to 50 characters.
	if (strPass.GetLength() > 50) {
		MessageBox("Passwords have a maximum of 50 characters, please shorten the password.");
		return false;
	}

	//we should be good to go now
	//(e.lally 2009-01-22) PLID 32812 - When we set the password, we should default to requiring the patient
	// (j.jones 2009-04-30 14:19) - PLID 33853 - used AES encryption on the password
	ExecuteParamSql("Update NexWebLoginInfoT SET Password = {VARBINARY}, PWExpireNextLogin = 1 "
		"WHERE PersonID = {INT} AND UserName = {STRING}",
		EncryptStringToVariant(strPass), m_nPersonID, m_strUserName);
	return true;
}

BOOL CNexWebChangePasswordDlg::OnInitDialog()
{
	try
	{
		__super::OnInitDialog();

		SetWindowText("Change Nexweb User Password");
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (j.armen 2013-10-03 07:08) - PLID 57914 - Password complexity function
NexWebPasswordComplexity* CNexWebChangePasswordDlg::GenerateComplexity()
{
	return GetNexWebPasswordRules(GetRemoteData());
}