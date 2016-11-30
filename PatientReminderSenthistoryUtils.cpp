#include "stdafx.h"
#include "PatientReminderSenthistoryUtils.h"

// (b.savon 2014-09-02 10:27) - PLID 62791 - Add Option to letterwriting and Recall diloag to capture Sent Remider option
void DontShowMe(CWnd *pParent, const CString& strName)
{
	DontShowMeAgain(
		pParent,
		"Reminding patients for care they are already scheduled to receive is against Meaningful Use rules.\r\n" 
		"Continuing will mark each patient as having a reminder sent and will be counted in the Meaningful\r\n"
		"Use Reporting.  You should confirm this is correct for each patient before proceeding.",
		strName,
		"Meaningful Use - Patient Reminders"
	);
}