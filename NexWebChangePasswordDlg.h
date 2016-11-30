#pragma once
#include "NxChangePasswordDlg.h"

// (f.gelderloos 2013-08-14 14:49) - PLID 57914
class CNexWebChangePasswordDlg : public CNxChangePasswordDlg
{
public:
	CNexWebChangePasswordDlg(long nPersonID, CString strUserName, CWnd* pParent);
	~CNexWebChangePasswordDlg(void);

protected:
	// (j.armen 2013-10-03 07:08) - PLID 57914
	virtual bool UpdateData(const CString &strPass) override;
	virtual BOOL OnInitDialog() override;

private:
	struct NexWebPasswordComplexity* GenerateComplexity();
};
