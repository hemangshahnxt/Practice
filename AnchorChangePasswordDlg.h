#pragma once
#include "NxChangePasswordDlg.h"

// (f.gelderloos 2013-08-14 14:49) - PLID 57914
class CAnchorChangePasswordDlg : public CNxChangePasswordDlg
{
public:
	CAnchorChangePasswordDlg(long nPersonID, CString strUserName, CWnd* pParent);
	~CAnchorChangePasswordDlg(void);

	// (j.armen 2013-10-03 07:08) - PLID 57914 - UpdateData returns true on success
	virtual bool UpdateData(const CString &strPass) override;
	virtual BOOL OnInitDialog() override;

	struct NexWebPasswordComplexity* GenerateComplexity();
};
