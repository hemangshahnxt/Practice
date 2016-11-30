#ifndef NX_PRACTICE_SUPERBILL_H
#define NX_PRACTICE_SUPERBILL_H

#pragma once



#include "NxTaskDialog.h"

void SuperBill(int PatientID);
//DRT 6/12/2008 - PLID 9679 - Added override path
void SuperbillByAppt(long nApptID, CString strOverridePath);
//DRT 6/12/2008 - PLID 9679 - Added override path
void PrintSuperbill(CString strSql, bool bApptBased, CString strOverridePath);
BOOL AllowSaveSuperbill();

// (a.walling 2010-09-15 09:20) - PLID 7018 - Gather info on existing superbills
class ExistingSuperBillInfo
	 : public NxTaskDialog
{
public:
	ExistingSuperBillInfo(CWnd* pParent);

	void Load(const CString& strBaseWhere);
	
	enum Action {
		Cancel = IDCANCEL,
		PrintNew = 0x100,
		PrintNewWithCopies,
		PrintAll
	};

	Action Prompt();
	
	int m_nFreshSuperBills;
	int m_nExistingSuperBills;
	int m_nExistingSuperBillsWithFinancialInfo;

	Action m_Action;

protected:
	virtual BOOL OnButtonClicked(int nButton);
};


#endif // #ifndef NX_PRACTICE_SUPERBILL_H