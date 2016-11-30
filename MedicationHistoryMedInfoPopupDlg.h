//#include "PatientsRc.h"
//#include "GenericBrowserDlg.h"
#include "MedicationHistoryDlg.h"
#pragma once
//(r.wilson 12/9/2013) PLID 57947 
namespace MedicationHistory 
{
	void PopupInfo(CWnd* pParent,  std::vector<shared_ptr<CMedicationHistoryDlg::MedHistoryRowInfo>>& values);

	void DocumentComplete(LPDISPATCH pDisp, VARIANT* URL);
}