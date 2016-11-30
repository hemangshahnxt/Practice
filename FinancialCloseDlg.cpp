// FinancialCloseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FinancialCloseDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "AuditTrail.h"
#include "EnterActiveDateDlg.h"

// (j.jones 2010-12-22 10:07) - PLID 41911 - created

// CFinancialCloseDlg dialog

using namespace ADODB;
using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CFinancialCloseDlg, CNxDialog)

CFinancialCloseDlg::CFinancialCloseDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFinancialCloseDlg::IDD, pParent)
{

}

CFinancialCloseDlg::~CFinancialCloseDlg()
{
}

void CFinancialCloseDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_CLOSE_DIALOG, m_btnExitDialog);
	DDX_Control(pDX, IDC_BTN_PERFORM_CLOSE, m_btnPerformClose);
	DDX_Control(pDX, IDC_LABEL_CURRENT_CLOSE_DATE, m_nxstaticCurrentCloseDate);
}


BEGIN_MESSAGE_MAP(CFinancialCloseDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_CLOSE_DIALOG, OnBtnExitDialog)
	ON_BN_CLICKED(IDC_BTN_PERFORM_CLOSE, OnBtnPerformClose)
END_MESSAGE_MAP()


// CFinancialCloseDlg message handlers

BOOL CFinancialCloseDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnExitDialog.AutoSet(NXB_CLOSE);
		m_btnPerformClose.AutoSet(NXB_MODIFY);

		m_CloseHistoryList = BindNxDataList2Ctrl(IDC_CLOSE_HISTORY_LIST);

		//get our last date/time of a hard close
		COleDateTime dtHardCloseAsOfDate = g_cdtInvalid;
		COleDateTime dtHardCloseInputDate = g_cdtInvalid;
		// (j.jones 2011-01-20 13:59) - PLID 41999 - get both the close date and input date of the most recent close
		_RecordsetPtr rsHardClose = CreateParamRecordset("SELECT Max(CloseDate) AS HardCloseAsOfDate, "
			"Max(InputDate) AS HardCloseInputDate FROM FinancialCloseHistoryT");
		if(!rsHardClose->eof) {
			//this is nullable since this is a Max()
			dtHardCloseAsOfDate = AdoFldDateTime(rsHardClose, "HardCloseAsOfDate", g_cdtInvalid);
			dtHardCloseInputDate = AdoFldDateTime(rsHardClose, "HardCloseInputDate", g_cdtInvalid);
		}
		rsHardClose->Close();

		CString strLabel = "Financial history is not currently closed as of any date.";
		if(dtHardCloseAsOfDate.GetStatus() != COleDateTime::invalid) {
			strLabel.Format("Financial history is currently closed as of %s.\n"
				"(Performed on %s.)",
				FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
				FormatDateTimeForInterface(dtHardCloseInputDate, DTF_STRIP_SECONDS, dtoDateTime));
		}

		m_nxstaticCurrentCloseDate.SetWindowText(strLabel);

		//disable the ability if they don't have permission
		//(which is passworded-only), although right now you
		//can't open this dialog without having this permission
		if(!(GetCurrentUserPermissions(bioPerformFinancialClose) & sptWriteWithPass)) {
			m_btnPerformClose.EnableWindow(FALSE);
		}

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CFinancialCloseDlg::OnBtnExitDialog()
{
	try {

		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}

void CFinancialCloseDlg::OnBtnPerformClose()
{
	try {

		//check the permission first, without a password
		if(!(GetCurrentUserPermissions(bioPerformFinancialClose) & sptWriteWithPass)) {
			
			PermissionsFailedMessageBox();
			
			//this should have been disabled to begin with
			m_btnPerformClose.EnableWindow(FALSE);
			return;
		}

		// (j.jones 2011-01-05 16:29) - PLID 41998 - prompt for a date
		CEnterActiveDateDlg dlg(this);
		dlg.m_strPrompt = "Enter the date that financial history should be closed through:";
		dlg.m_strWindowTitle = "Enter the Financial Close Date";
		dlg.m_dtDate = COleDateTime::GetCurrentTime();
		dlg.m_bAllowCancel = true;
		dlg.m_bAllowPastDate = true;
		
		if(IDOK != dlg.DoModal()) {
			return;
		}

		if(dlg.m_dtDate.GetStatus() == COleDateTime::invalid) {
			MessageBox("The date you entered is not valid.", "Practice", MB_ICONEXCLAMATION|MB_OK);
			return;
		}
		
		//disallow dates in the future
		if(dlg.m_dtDate > COleDateTime::GetCurrentTime()) {
			MessageBox("You may not enter a date in the future.", "Practice", MB_ICONEXCLAMATION|MB_OK);
			return;
		}

		//set the time to 11:59:59pm
		COleDateTime dtDateToClose;
		dtDateToClose.SetDateTime(dlg.m_dtDate.GetYear(), dlg.m_dtDate.GetMonth(), dlg.m_dtDate.GetDay(), 23, 59, 59);

		COleDateTime dtServerTime = GetRemoteServerTime();
		if(dtServerTime.GetDay() == dtDateToClose.GetDay()
			&& dtServerTime.GetMonth() == dtDateToClose.GetMonth()
			&& dtServerTime.GetYear() == dtDateToClose.GetYear()) {

			//closing today, which will use the current time *as of the insert*, but
			//we can show the current server time in the message box

			dtDateToClose = dtServerTime;
		}

		_variant_t varDateToClose(dtDateToClose, VT_DATE);

		//lastly confirm the date is not prior to our most recent close,
		//query data again to get the most accurate date/time

		{
			COleDateTime dtHardCloseAsOfDate = g_cdtInvalid;
			// (j.jones 2011-01-20 13:59) - PLID 41999 - get both the close date and input date of the most recent close
			_RecordsetPtr rsHardClose = CreateParamRecordset("SELECT Max(CloseDate) AS HardCloseAsOfDate FROM FinancialCloseHistoryT");
			if(!rsHardClose->eof) {
				//this is nullable since this is a Max()
				dtHardCloseAsOfDate = AdoFldDateTime(rsHardClose, "HardCloseAsOfDate", g_cdtInvalid);
			}
			rsHardClose->Close();

			//is our date earlier than the last close date?
			if(dtHardCloseAsOfDate.GetStatus() == COleDateTime::valid && dtDateToClose <= dtHardCloseAsOfDate) {
				CString strMessage;
				strMessage.Format("You may not enter a date earlier than the most recent close date of %s.", FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime));
				MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_OK);
				return;
			}
		}

		//give scary warning #1
		CString strWarning1;
		// (j.jones 2011-01-19 11:41) - PLID 42149 - updated wording to mention applies
		// (j.jones 2011-01-20 15:28) - PLID 41999 - mentioned how service dates of existing
		// line items are also closed, showing date only, since service dates have no times
		strWarning1.Format("This financial close will lock all bills, charges, payments, adjustments, refunds, and applies "
			"created at or before %s from being edited.\n\n"
			"Any existing line item that has a service date on or before %s will also be closed.\n\n"
			"Only Administrators and users with the permission to specifically edit closed line items will be able to make "
			"further changes to this existing data.\n\n"
			"Are you sure you want to perform a financial close as of %s? "
			"You will be required to enter your password to authorize this close.",
			FormatDateTimeForInterface(dtDateToClose, DTF_STRIP_SECONDS, dtoDateTime),
			FormatDateTimeForInterface(dtDateToClose, NULL, dtoDate),
			FormatDateTimeForInterface(dtDateToClose, DTF_STRIP_SECONDS, dtoDateTime));

		if(IDNO == MessageBox(strWarning1, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		//demand a password, always
		if(!CheckCurrentUserPassword()) {
			return;
		}

		//give the final scary warning
		if(IDNO == MessageBox("Are you SURE you wish to close the financial data? This process is irreversible!", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		//fine, so be it, close it up
		COleDateTime dtHardCloseAsOfDate = g_cdtInvalid;
		COleDateTime dtHardCloseInputDate = g_cdtInvalid;

		// (j.jones 2011-01-05 16:23) - PLID 41998 - The CloseDate is now configurable, but if
		// the date we pass in is the server's date, we will force it to use the server's time as well
		_RecordsetPtr rs = CreateParamRecordset("SET NOCOUNT ON "
				"DECLARE @dtCloseDate DATETIME "
				"SET @dtCloseDate = {VT_DATE} "
				""
				"IF dbo.AsDateNoTime(@dtCloseDate) = dbo.AsDateNoTime(GetDate()) "
				"BEGIN "
				"SET @dtCloseDate = GetDate() "
				"END "
				""
				"INSERT INTO FinancialCloseHistoryT (UserID, CloseDate) VALUES ({INT}, @dtCloseDate) "
				"SET NOCOUNT OFF "
				""
				//the most recent date is the closed date, period,
				//regardless of whatever date we just inserted
				"SELECT Max(CloseDate) AS HardCloseAsOfDate, Max(InputDate) AS HardCloseInputDate "
				"FROM FinancialCloseHistoryT", varDateToClose, GetCurrentUserID());
		
		//should be impossible to be eof, and impossible to be NULL,
		//we want to get an exception if either occur
		dtHardCloseAsOfDate = AdoFldDateTime(rs, "HardCloseAsOfDate");
		dtHardCloseInputDate = AdoFldDateTime(rs, "HardCloseInputDate");
		rs->Close();

		//audit this
		long nAuditID = BeginNewAuditEvent();
		// (j.jones 2011-01-05 17:58) - PLID 41998 - audit the time that it is now closed at
		CString strNew;
		strNew.Format("Closed as of %s", FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime));
		AuditEvent(-1, "", nAuditID, aeiFinancialClose, -1, "", strNew, aepHigh, aetCreated);

		//update our label & requery the list, even though this dialog
		//is about to close, we should make sure its content is correct
		//while the 'success' messagebox is up
		CString strLabel, strSuccess;

		// (j.jones 2011-01-20 15:04) - PLID 41999 - the label now lists both the as of date and create date
		strLabel.Format("Financial history is currently closed as of %s.\n"
			"(Performed on %s.)",
			FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime),
			FormatDateTimeForInterface(dtHardCloseInputDate, DTF_STRIP_SECONDS, dtoDateTime));

		strSuccess.Format("Financial history has successfully been closed as of %s.", FormatDateTimeForInterface(dtHardCloseAsOfDate, DTF_STRIP_SECONDS, dtoDateTime));

		m_nxstaticCurrentCloseDate.SetWindowText(strLabel);
		m_CloseHistoryList->Requery();

		//confirm success, showing the closed date/time
		MessageBox(strSuccess, "Practice", MB_ICONINFORMATION|MB_OK);

		//close the dialog
		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}