// Ebilling.cpp : implementation file
//

#include "stdafx.h"
#include "Ebilling.h"
#include "practice.h"
#include "PracProps.h"
#include "NxStandard.h"
#include "MsgBox.h"
#include "GlobalUtils.h"
#include "GlobalDataUtils.h"
#include "financialrc.h"
#include "nxmessagedef.h"
#include "InternationalUtils.h"
#include "InsuranceDlg.h"
#include "OHIPDialerUtils.h"
#include "NxCompressUtils.h"
#include "AlbertaPastClaimSelectDlg.h"
#include "Base64.h"
#include "NxAPI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RETURN_ON_FAIL(function)	{ int nResult = function; if(nResult!=0) return nResult; }

using namespace ADODB;

#define EBILLING_TIMER_EVENT		22101

/////////////////////////////////////////////////////////////////////////////
// CEbilling dialog
//
//
// Page down to the AA0 function to read all about this class and its structure.
//
//

// (b.spivey, August 27th, 2014) - PLID 63492 - We may be doing this for preview only, and if so it's going to be for a specific bill, not a batch.
CEbilling::CEbilling(CWnd* pParent /*=NULL*/, bool bFileExportOnly, long nBillID)
	: CNxDialog(CEbilling::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEbilling)
	m_bCapitalizeEbilling = FALSE;
	m_bStripANSIPunctuation = FALSE;
	m_bFourDigitRevenueCode = TRUE;
	m_bSendSubscriberPerClaim = TRUE;
	m_bHidePatAmtPaid = TRUE;
	m_bIsGroup = FALSE;
	m_actClaimType = actProf;
	m_strLast2010AAQual = "";
	m_strLast2010AA_NM102 = "";
	m_strLast2010AA_NM103 = "";
	m_nFilterByLocationID = -1;
	m_nFilterByProviderID = -1;
	m_BatchCount = 0;
	m_bDisableREFXX = TRUE;
	m_nAlbertaTotalTransactionCounts = 0;
	m_nAlbertaCurrentSegmentCount = 0;
	m_nAlbertaTotalSegmentCounts = 0;
	//}}AFX_DATA_INIT

	m_strEbillingFormatName = m_strEbillingFormatContact = m_strEbillingFormatFilename =
		m_strISA01Qual = m_strISA02 = m_strISA03Qual = m_strISA04 = m_strReceiverISA08ID =
		m_strReceiverGS03ID = m_strReceiver1000BID = m_strSubmitterISA06ID = m_strSubmitterGS02ID =
		m_strSubmitter1000AID = m_strReceiverISA07Qual = m_strReceiver1000BQual =
		m_strSubmitterISA05Qual = m_strSubmitter1000AQual = m_strAddnl_2010AA_Qual = 
		m_strAddnl_2010AA = m_strPER05Qual_1000A = m_strPER06ID_1000A = m_strPrependPatientIDCode = "";

	m_bEbillingFormatZipped = m_bUse2420A = m_bHide2310C_WhenType11Or12 = m_bDontSendSecondaryOnDiagnosisMismatch = 
		m_bUseSV106 = m_bUse2010AB = m_bDontSubmitSecondary = m_bExport2330BPER =
		m_bExportAll2010AAIDs = m_bTruncateCurrency = m_bUseSSN = m_bPrependPayerNSF =
		m_bUse_Addnl_2010AA = m_bSeparateBatchesByInsCo = m_bUse1000APER = m_bPrependPatientID = FALSE;
	
	m_bHide2310C_WhenBillLocation = FALSE;

	// (j.jones 2012-02-01 11:02) - PLID 40880 - the default is now 5010
	m_avANSIVersion = av5010;

	m_bAlbertaSubmitterPrefixCached = false;

	// (b.spivey, August 27th, 2014) - PLID 63492 - This variable decides if we're previewing or not. If in debug we always want this file.
	m_bHumanReadableFormat = bFileExportOnly;
	m_bFileExportOnly = m_bHumanReadableFormat; 
#ifdef _DEBUG
	m_bHumanReadableFormat = true;
#endif


	if (bFileExportOnly == true && !(nBillID > 0))
	{
		//I have my doubts this will do what you expect it to. 
		ASSERT(FALSE);
	}

	m_nBillID = nBillID;

}
void CEbilling::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEbilling)
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CURR_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_CURR_EVENT, m_nxstaticCurrEvent);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEbilling, CNxDialog)
	//{{AFX_MSG_MAP(CEbilling)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEbilling message handlers

BOOL CEbilling::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{	
	return CDialog::Create(IDD, pParentWnd);
}

BOOL CEbilling::DestroyWindow() 
{
	try {
		//clear out the data
		int i=0;
		for(i=m_aryEBillingInfo.GetSize()-1;i>=0;i--) {		
			EBillingInfo *pInfo = (EBillingInfo*)(m_aryEBillingInfo.GetAt(i));
			if(pInfo) {
				delete pInfo;
			}
		}
		m_aryEBillingInfo.RemoveAll();

	}NxCatchAll("Error closing window.");
	
	return CDialog::DestroyWindow();
}

BOOL CEbilling::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (j.jones 2008-05-07 15:07) - PLID 29854 - added nxiconbutton for modernization
	m_btnCancel.AutoSet(NXB_CANCEL);

	try {
		
		// (j.jones 2008-05-09 09:47) - PLID 29986 - completely removed the Envoy/NSF code from the program

		if(m_FormatStyle != ANSI) {
			if(GetRemotePropertyInt("CapitalizeEbilling",0,m_FormatID,"<None>",FALSE)) 
				m_bCapitalizeEbilling = TRUE;
			else
				m_bCapitalizeEbilling = FALSE;

			m_VendorID = GetRemotePropertyText(_T("EbillingSubmitter"),_T("311494916"),m_FormatID,_T("<None>"));
			m_bIsClearinghouseIntegrationEnabled = false;
		}
		else {
			_RecordsetPtr rs = CreateParamRecordset("SELECT * FROM EbillingFormatsT WHERE ID = {INT}",m_FormatID);
			if(!rs->eof) {
				m_bCapitalizeEbilling = AdoFldBool(rs, "Capitalize",FALSE);
				m_bStripANSIPunctuation = AdoFldBool(rs, "UnPunctuate",FALSE);
				// (j.jones 2007-05-02 10:32) - PLID 25855 - cache the FourDigitRevCode value
				m_bFourDigitRevenueCode = AdoFldBool(rs, "FourDigitRevCode",TRUE);
				// (j.jones 2008-02-15 12:31) - PLID 28943 - cache the SendSubscriberPerClaim setting
				m_bSendSubscriberPerClaim = AdoFldBool(rs, "SendSubscriberPerClaim",TRUE);
				// (j.jones 2008-02-19 11:40) - PLID 29004 - cache the Send2330BAddress setting
				m_bSend2330BAddress = AdoFldBool(rs, "Send2330BAddress", FALSE);

				// (j.jones 2008-05-06 10:52) - PLID 29937 - cached all remaining settings
				m_strEbillingFormatName = AdoFldString(rs, "Name", "");
				m_strEbillingFormatContact = AdoFldString(rs, "Contact", "");
				m_strEbillingFormatFilename = AdoFldString(rs, "Filename", "");
				m_bEbillingFormatZipped = AdoFldBool(rs, "Zipped", FALSE);
				m_bIsGroup = AdoFldBool(rs, "IsGroup", FALSE);
				m_bUse2420A = AdoFldBool(rs, "Use2420A", FALSE);
				// (j.jones 2012-01-06 15:16) - PLID 47351 - this is Hide2310D in data, but I renamed it to be
				// 5010 compliant and make more sense with respect to our other new 2310C-hiding ability
				m_bHide2310C_WhenType11Or12 = AdoFldBool(rs, "Hide2310D", FALSE);
				// (j.jones 2012-01-06 15:13) - PLID 47351 - added option to not send place of service if the same
				// location as 2010AA, HCFA only, 5010 only
				m_bHide2310C_WhenBillLocation = AdoFldBool(rs, "Hide2310CWhenBillLocation", FALSE);
				m_bUseSV106 = AdoFldBool(rs, "UseSV106", FALSE);
				m_bUse2010AB = AdoFldBool(rs, "Use2010AB", FALSE);
				m_bDontSubmitSecondary = AdoFldBool(rs, "DontSubmitSecondary", FALSE);
				// (a.wilson 2014-06-27 14:40) - PLID 62517 - load setting.
				m_bDontSendSecondaryOnDiagnosisMismatch = AdoFldBool(rs, "DontSendSecondaryOnDiagMismatch", TRUE);
				m_strISA01Qual = AdoFldString(rs, "ISA01Qual", "");
				m_strISA02 = AdoFldString(rs, "ISA02", "");
				m_strISA03Qual = AdoFldString(rs, "ISA03Qual", "");
				m_strISA04 = AdoFldString(rs, "ISA04", "");
				m_strReceiverISA08ID = AdoFldString(rs, "ReceiverISA08ID", "");
				m_strReceiverGS03ID = AdoFldString(rs, "ReceiverGS03ID", "");
				m_strReceiver1000BID = AdoFldString(rs, "Receiver1000BID", "");
				m_strSubmitterISA06ID = AdoFldString(rs, "SubmitterISA06ID", "");
				m_strSubmitterGS02ID = AdoFldString(rs, "SubmitterGS02ID", "");
				m_strSubmitter1000AID = AdoFldString(rs, "Submitter1000AID", "");				
				m_bExport2330BPER = AdoFldBool(rs, "Export2330BPER", FALSE);
				m_bExportAll2010AAIDs = AdoFldBool(rs, "ExportAll2010AAIDs", FALSE);
				m_strReceiverISA07Qual = AdoFldString(rs, "ReceiverISA07Qual", "");
				m_strReceiver1000BQual = AdoFldString(rs, "Receiver1000BQual", "");
				m_strSubmitterISA05Qual = AdoFldString(rs, "SubmitterISA05Qual", "");
				m_strSubmitter1000AQual = AdoFldString(rs, "Submitter1000AQual", "");
				m_bTruncateCurrency = AdoFldBool(rs, "TruncateCurrency", FALSE);
				m_bUseSSN = AdoFldBool(rs, "UseSSN", FALSE);
				// (j.jones 2009-08-04 11:34) - PLID 14573 - renamed to PrependTHINPayerNSF to PrependPayerNSF,
				// and removed m_bUseTHINPayerIDs
				//m_bUseTHINPayerIDs = AdoFldBool(rs, "UseTHINPayerIDs", FALSE);
				m_bPrependPayerNSF = AdoFldBool(rs, "PrependPayerNSF", FALSE);
				m_bUse_Addnl_2010AA = AdoFldBool(rs, "Use_Addnl_2010AA", FALSE);
				m_strAddnl_2010AA_Qual = AdoFldString(rs, "Addnl_2010AA_Qual", "");
				m_strAddnl_2010AA = AdoFldString(rs, "Addnl_2010AA", "");
				m_bSeparateBatchesByInsCo = AdoFldBool(rs, "SeparateBatchesByInsCo", FALSE);
				m_bUse1000APER = AdoFldBool(rs, "Use1000APER", FALSE);
				m_strPER05Qual_1000A = AdoFldString(rs, "PER05Qual_1000A", "");
				m_strPER06ID_1000A = AdoFldString(rs, "PER06ID_1000A", "");

				// (j.jones 2008-09-09 16:45) - PLID 26482 - cache the HidePatAmtPaid setting
				m_bHidePatAmtPaid = AdoFldBool(rs, "HidePatAmtPaid", TRUE);

				// (j.jones 2009-10-01 14:02) - PLID 35711 - cache the PrepentPatientID data
				m_bPrependPatientID = AdoFldBool(rs, "PrependPatientID", TRUE);
				m_strPrependPatientIDCode = AdoFldString(rs, "PrependPatientIDCode", "");
				m_strPrependPatientIDCode.TrimLeft();
				m_strPrependPatientIDCode.TrimRight();

				// (j.jones 2010-10-13 13:29) - PLID 40913 - cache the toggle for ANSI 4010/5010
				// (j.jones 2012-02-01 11:02) - PLID 40880 - the default is now 5010 (although this field is not nullable)
				m_avANSIVersion = (ANSIVersion)AdoFldLong(rs, "ANSIVersion", (long)av5010);
			}
			rs->Close();

			if (!m_bFileExportOnly && m_avANSIVersion == av5010)
			{
				m_bIsClearinghouseIntegrationEnabled = GetRemotePropertyInt("EbillingClearinghouseIntegration_Enabled", 0, FALSE, "<None>", true) != FALSE;
			}
		}

		SetTimer(EBILLING_TIMER_EVENT, 20, NULL);

	} NxCatchAll("Error in Ebilling::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CString CEbilling::GetExportFilePath()
{
	CString strDefaultFileName;
	CString strDefaultDirectory = GetEnvironmentDirectory() ^ "EBilling\\";
	CString strFilter;
	bool bPromptUser = true;

	if (m_FormatStyle != ANSI) 
	{
		if (m_FormatStyle == OHIP) {
			// (j.jones 2006-11-09 10:27) - PLID 21570 - if OHIP, export a name as:
			// H, plus an alpha representation of the month (A = January, B = February),
			// plus the group or provider number, plus a 3-digit sequence number
			// example: HA123456.001

			CString strDefaultFileName = "H";

			//month indicator
			COleDateTime dtNow = COleDateTime::GetCurrentTime();
			char chMonth = 'A';
			chMonth = chMonth + dtNow.GetMonth() - 1;

			strDefaultFileName += chMonth;

			//group or provider number
			CString strGroup = GetRemotePropertyText("OHIP_GroupNumber", "0000", 0, "<None>", true);
			strGroup.Replace(" ", "");
			// (j.jones 2009-02-09 12:03) - PLID 32997 - the group number should not be more than 4 digits
			if (strGroup.GetLength() > 4) {
				strGroup = strGroup.Left(4);
			}
			if (!strGroup.IsEmpty() && strGroup != "0000") {
				//use the group number
				strDefaultFileName += strGroup;
			}
			else {
				//use the provider number, from the first provider in the batch
				if (m_aryEBillingInfo.GetSize() > 0) {
					long nProviderID = ((EBillingInfo*)m_aryEBillingInfo.GetAt(0))->ProviderID;
					// (j.jones 2008-05-06 11:55) - PLID 27453 - parameterized
					_RecordsetPtr rs = CreateParamRecordset("SELECT NPI FROM ProvidersT WHERE PersonID = {INT}", nProviderID);
					if (!rs->eof) {
						CString strNPI = AdoFldString(rs, "NPI", "");
						strNPI.Replace(" ", "");
						//the OHIP number should not be more than 6 digits
						if (strNPI.GetLength() > 6)
							strNPI = strNPI.Left(6);
						strDefaultFileName += strNPI;
					}
					rs->Close();
				}
			}

			// (j.jones 2009-02-09 12:07) - PLID 32997 - shouldn't be possible now
			// to have a strDefaultFileName greater than 8 characters (before the extension)
			// but let's doublecheck and confirm it is not going to exceed the 8.3
			// format
			if (strDefaultFileName.GetLength() > 8) {
				strDefaultFileName = strDefaultFileName.Left(8);
			}

			strDefaultFileName += ".";

			//Transmission Number
			//just grab the last three digits of the batch number
			if (m_strBatchNumber.GetLength() > 3)
				strDefaultFileName += m_strBatchNumber.Right(3);
			else
				strDefaultFileName += m_strBatchNumber;

			strFilter = "All Files (*.*)|*.*||";
		}
		else {
			//save as an ebilling.txt file for all formats
			strDefaultFileName = "ebilling.txt";
			strFilter = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*||";
		}
	}
	else if(m_bFileExportOnly) 
	{
		// (b.spivey, August 28, 2014) - PLID 63492 - Save this to sessions if we're just doing a preview file.			
		strDefaultDirectory = GetPracPath(PracPath::SessionPath);
		strDefaultFileName = FormatString("ebillingpreview_%li", (long)GetTickCount());
		bPromptUser = false;
	}
	else 
	{
		//ANSI special file name

		CString ext;

		// (j.jones 2008-05-06 11:16) - PLID 29937 - cached the Filename instead of
		// opening a recordset
		m_strEbillingFormatFilename.Replace(" ", "");
		if (m_strEbillingFormatFilename.IsEmpty()) {
			strDefaultFileName = "ebilling.txt";
		}
		else {
			strDefaultFileName = m_strEbillingFormatFilename;
		}

		//see if they want to include the batch number
		if (strDefaultFileName.Find("%b") != -1) {
			strDefaultFileName.Replace("%b", m_strBatchNumber);
		}
		else if (strDefaultFileName.Find("%") != -1) {
			//get the digits they need
			int pos = strDefaultFileName.Find("%");

			// (j.jones 2008-05-01 09:13) - PLID 27832 - properly supported
			// a %#b setting where # is > 4

			CString strToReplace;
			if (strDefaultFileName.GetLength() < pos + 2) {
				//bad formatting, just slap on the full batch number
				strDefaultFileName = strDefaultFileName.Left(pos) + m_strBatchNumber;
			}
			else {

				CString strToReplace = strDefaultFileName.Mid(pos, 3);

				CString strTest = strToReplace;
				strTest.TrimLeft("%");
				strTest.TrimRight("b");
				if (!strTest.IsEmpty() && atoi(strTest) == 0) {
					//bad formatting, just slap on the full batch number
					strDefaultFileName = strDefaultFileName.Left(pos) + m_strBatchNumber;
				}
				else {

					CString strChar = strDefaultFileName.GetAt(pos + 1);
					int num = atoi(strChar);
					if (num == 0) {
						num = 2;
					}

					CString strNewNumber;
					if (num <= m_strBatchNumber.GetLength()) {
						strNewNumber = m_strBatchNumber.Right(num);
					}
					else {
						//pad the number with zeroes if needed
						strNewNumber = m_strBatchNumber;
						while (strNewNumber.GetLength() < num) {
							strNewNumber = "0" + strNewNumber;
						}
					}

					strDefaultFileName.Replace(strToReplace, strNewNumber);
				}
			}
		}

		if (strDefaultFileName.Find(".") != -1) {
			ext = strDefaultFileName.Right(strDefaultFileName.GetLength() - strDefaultFileName.Find(".") - 1);
		}
		else {
			strDefaultFileName += ".txt";
			ext = "txt";
		}

		strFilter.Format("Claim Files (*.%s)|*.%s|All Files (*.*)|*.*||", ext, ext);

		if (m_bIsClearinghouseIntegrationEnabled)
		{
			CString strGuid = NewUUID(false);
			strDefaultDirectory = FileUtils::GetTempPath() ^ FormatString("ebilling_%s", strGuid);
			m_bDeleteExportDirectory = true;
			bPromptUser = false;
		}
	}

	//create the ebilling directory - does not give error if directory exists
	// (j.armen 2011-10-25 13:09) - PLID 46134 - Since this section of ebilling prompts where the files will go, 
	// we can safely send to the practice path
	// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure. Also, changed to FileUtils::CreatePath() so that 
	// non-existent intermediate paths are created.
	FileUtils::CreatePath(strDefaultDirectory);

	if (bPromptUser)
	{
		CFileDialog SaveAs(FALSE, NULL, strDefaultFileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
		SaveAs.m_ofn.lpstrInitialDir = strDefaultDirectory;
		if (SaveAs.DoModal() == IDCANCEL) 
		{
			return "";
		}
		else
		{
			return SaveAs.GetPathName();
		}
	}
	else
	{
		// strDefaultFileName is not limited to a simple file name. It can be a path because the default file name
		// can be set in ANSI Properties and does not restrict the user from entering a full path.
		return strDefaultDirectory ^ FileUtils::GetFileName(strDefaultFileName);
	}
}

void CEbilling::ExportData()
{
	CString str;
	_RecordsetPtr rs;

	int tempint;
	
	// (j.jones 2010-10-12 16:39) - PLID 29971 - supported Alberta HLINK
	if (m_FormatStyle != IMAGE && m_FormatStyle != OHIP && m_FormatStyle != ALBERTA && m_FormatID <= 0) {
		MessageBox("Please select an Export Style","Error!",MB_ICONEXCLAMATION);
		EndDialog(IDCANCEL);
		return;
	}

	try {
		// (j.dinatale 2012-12-28 16:30) - PLID 54365 - need to clear out our cached values for submitter prefix
		m_bAlbertaSubmitterPrefixCached = false;
		m_strAlbertaSubmitterPrefix = "";
		
		// (j.jones 2008-05-09 09:37) - PLID 29986 - completely removed NSF from the program		

		// (b.spivey, August 27th, 2014) - PLID 63492 - If we're specifically doing a preview claim don't update. 
		if(m_FormatStyle != IMAGE && !m_bFileExportOnly) {

			// update LastBatchID
			tempint = 0;
			tempint = GetRemotePropertyInt(_T("LastBatchID"),0,0,_T("<None>"));
			if (tempint==0) {
				SetRemotePropertyInt(_T("LastBatchID"),1,0,_T("<None>"));
				m_BatchID = 1;
			}
			else { // record does exist
				m_BatchID = tempint+1;
				SetRemotePropertyInt(_T("LastBatchID"),m_BatchID,0,_T("<None>"));
			}  // end of LastBatchID update
		}

	} NxCatchAll("Error In Ebilling::ExportData - Initialize");

	try {

		//set up the progress bar
		m_Progress.SetMin(0.0);  // progress bar setup
		m_Progress.SetMax(300.0);	//0 through 100 is for loading, 101 to 200 is for exporting, 201 to 300 is for connecting
		m_Progress.SetValue(0.0);
		m_Progress.SetRedraw(TRUE);

		//the progress increment is decided in LoadClaimInfo (based on the count of claims)

		//load all the default information we will need
		// (j.jones 2007-02-22 12:56) - PLID 24089 - changed behavior to support sending errors back
		int nError = LoadClaimInfo();
		ResolveErrors(nError);
		if(nError != Success) {
			//ResolveErrors will give an appropriate warning, all we need to do is return
			return;
		}

		m_strBatchNumber.Format("%li",m_BatchID);
		while (m_strBatchNumber.GetLength() < 4)
			m_strBatchNumber.Insert(0,'0');

		// (j.jones 2008-05-09 09:37) - PLID 29986 - completely removed NSF from the program

		if (m_FormatStyle != ANSI) {

			//if we aren't connecting to submit the claims, adjust the size of the progress bar
			//this way, the first half is for loading, the second half is for exporting
			m_Progress.SetMax(200.0);
		}

		CString strExportFilePath = GetExportFilePath();
		if (strExportFilePath.IsEmpty())
		{
			EndDialog(IDCANCEL);
			return;
		}
		m_ExportName = strExportFilePath;
		m_ZipName = m_ExportName.Left(m_ExportName.Find('.')) + ".zip";

	} NxCatchAll("Error In Ebilling::ExportData - Load");

	//begin the export!

	try {

		// (j.jones 2015-11-19 14:10) - PLID 67600 - Now we always export claim histories prior
		// to exporting the file. This is required for Alberta claims, otherwise it's done to ensure
		// you cannot create an export file without first logging the claim as being sent.

		// (b.spivey, August 27th, 2014) - PLID 63492 - If we're doing a single file export don't claim to be doing this. 
		// (j.jones 2015-11-19 14:10) - this 'file export only' annotates the file to be human readable, it is not a valid
		// ANSI file, so they cannot possibly submit it to a clearinghouse
		if (!m_bFileExportOnly) {
			UpdateClaimHistory();
		}

		switch(m_FormatStyle) {
		case IMAGE:	//2
			ResolveErrors(ExportToHCFAImage());
			break;
		case ANSI:	//3
			ResolveErrors(ExportToANSI());
			break;
		case OHIP:	//4
			// (j.jones 2006-11-09 08:29) - PLID 21570 - added support for OHIP
			ResolveErrors(ExportToOHIP());
			break;
		case ALBERTA:	//5
			// (j.jones 2010-10-12 16:39) - PLID 29971 - supported Alberta HLINK
			ResolveErrors(ExportToAlbertaHLINK());
			break;
		default:
			ThrowNxException("%s : Invalid format style (%li)", __FUNCTION__, m_FormatStyle);
		}
	} NxCatchAll("Error In Ebilling::ExportData - Export");
}

// (j.jones 2007-02-22 12:56) - PLID 24089 - changed return value to support sending errors back
int CEbilling::LoadClaimInfo()
{
	//this recordset gets all the information we need to start the export. The idea is to get out all the key IDs that
	//will be used in further queries, and hopefully this will reduce the number of joins used in later queries.

	try {
		//this query needs to group insurance companies and doctors, so we can separate batches when they change
		_RecordsetPtr rs;

		CString strLocationFilter = "";
		if(m_nFilterByLocationID != -1) {
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			strLocationFilter.Format(" AND BillsT.ID IN (SELECT BillID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND LineItemT.LocationID = %li)", m_nFilterByLocationID);
		}

		CString strProviderFilter;
		if (m_nFilterByProviderID != -1) {
			//filter on any bill that contains the selected provider, even if they may
			//also contain other providers
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			strProviderFilter.Format("AND BillsT.ID IN (SELECT BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND ChargesT.DoctorsProviders = %li)", m_nFilterByProviderID);
		}

		CString strClaimType = "";
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		strClaimType.Format(" AND BillsT.FormType = %li",m_actClaimType == actInst ? 2 : 1);

		// (j.jones 2010-04-16 11:14) - PLID 38225 - added ability to disable sending REF*XX in provider loops
		m_bDisableREFXX = GetRemotePropertyInt("ANSIDisableREFXX", 1, 0, "<None>", true) == 1;
		
		// (j.jones 2006-12-01 15:50) - PLID 22110 - supported the ClaimProviderID
		// (j.jones 2008-04-03 09:23) - PLID 28995 - supported the HCFAClaimProvidersT setup
		// (j.jones 2008-08-01 10:16) - PLID 30917 - HCFAClaimProvidersT is now per insurance company

		if(m_FormatStyle == ANSI) {
			//if ANSI

			// (j.jones 2006-12-06 15:56) - PLID 23631 - order by insurance co
			// if separating by insco, otherwise by provider
			CString strOrderBy = "ProviderID, InsuredPartyID, HCFAID";
			// (j.jones 2008-05-06 11:17) - PLID 29937 - cached m_bSeparateBatchesByInsCo instead of using a recordset
			if(m_bSeparateBatchesByInsCo) {
				strOrderBy = "InsuranceCoID, ProviderID, InsuredPartyID, HCFAID";
			}

			// (b.spivey, August 27th, 2014) - PLID 63492 - filtering for specific claims. 
			CString strBatchFilter = "HCFATrackT.Batch = 2 AND HCFATrackT.CurrentSelect = 1 ";
			if (m_bHumanReadableFormat && m_nBillID > -1) {
				strBatchFilter.Format("BillsT.ID = %li ", m_nBillID); 
			}

			// (j.jones 2007-05-10 11:31) - PLID 25948 - supported UB referring physician as provider
			// (j.jones 2008-12-11 13:25) - PLID 32401 - added the referring physician ID and supervising provider ID
			// from the bill, to reduce recordsets later
			// (j.jones 2009-06-23 12:09) - PLID 34689 - supported SubmitAsPrimary in the IsPrimary calculation
			// can't parameterize this query
			// (j.jones 2009-08-05 10:08) - PLID 34467 - supported payer IDs per location / insurance
			// (j.jones 2009-12-16 15:54) - PLID 36237 - split payer IDs out into HCFA and UB payer IDs
			// (j.jones 2010-05-12 14:40) - PLID 38615 - IsPrimary is now always TRUE if there is no "Other" insurance
			// (j.jones 2010-08-30 17:12) - PLID 15025 - added SendTPLCode and OtherTPLCode
			// (j.jones 2010-11-09 12:57) - PLID 41389 - supported ChargesT.ClaimProviderID
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			// (j.jones 2011-08-19 10:00) - PLID 44984 - added UB04 Box77 and 78
			// (j.jones 2011-08-23 08:53) - PLID 44984 - added UB 2310B and 2310C provider ID
			// (j.jones 2012-01-18 15:06) - PLID 47627 - added OtherHCFA_Hide2330AREF and OtherUB92_Hide2330AREF,
			// which fills from the HCFA or UB group for the "Other" insurance on the claim, instead of relying
			// on the ANSI_Hide2330AREF value in m_HCFAInfo/m_UB92Info
			// (j.jones 2012-08-06 14:54) - PLID 51916 - supported the new payer ID data structure, always pulling from EbillingInsCoIDs
			// (j.jones 2013-04-24 17:27) - PLID 55564 - we now get the POS designation code (11, 22, etc.)
			// (j.jones 2013-06-20 12:27) - PLID 57245 - moved medicaid resubmission and original ref no logic
			// into the query (if both exist, medicaid resubmission is used)
			// (j.jones 2014-07-09 10:08) - PLID 62568 - ignore on hold claims
			// (j.jones 2014-08-22 13:58) - PLID 63446 - we now load the earliest date of service on the claim
			// (b.spivey, August 27th, 2014) - PLID 63492 - Changed the selection source for the purpose previewing one claim. 
			rs = CreateRecordset("SELECT * FROM "
				"(SELECT HCFATrackT.ID AS HCFAID, BillsT.ID AS BillID, BillsT.PatientID, PatientsT.UserDefinedID, HCFASetupT.Box33Setup, "
				"UB92SetupT.Box82Setup, UB92SetupT.UB04Box77Setup AS Box77Setup, UB92SetupT.Box83Setup AS Box78Setup, "
				"BillsT.RefPhyID, BillsT.SupervisingProviderID, "
				
				//find who the master (billing) provider is
				"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
				"(CASE WHEN BillsT.FormType = 2 "
				"THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"	WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
				"	ELSE ChargeProvidersT.ClaimProviderID END) "
				"WHEN BillsT.FormType = 1 "
				"	THEN (CASE WHEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID Is Not Null "
				"		THEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID "
				"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"			ELSE ChargeProvidersT.ClaimProviderID END) "
				"		END) "
				"ELSE ChargeProvidersT.ClaimProviderID END) END AS ProviderID, "

				//now find out who the ANSI rendering provider is (2310B)
				"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
				"(CASE WHEN BillsT.FormType = 2 "
				"THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"	WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
				"	ELSE ChargeProvidersT.ClaimProviderID END) "
				"WHEN BillsT.FormType = 1 "
				"	THEN (CASE WHEN HCFAClaimProvidersT.ANSI_2310B_ProviderID Is Not Null "
				"		THEN HCFAClaimProvidersT.ANSI_2310B_ProviderID "
				"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"			ELSE ChargeProvidersT.ClaimProviderID END) "
				"		END) "
				"ELSE ChargeProvidersT.ClaimProviderID END) END AS ANSI_RenderingProviderID, "

				//now find out who the HCFA Box24J provider is
				"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
				"(CASE WHEN BillsT.FormType = 2 "
				"THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"	WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
				"	ELSE ChargeProvidersT.ClaimProviderID END) "
				"WHEN BillsT.FormType = 1 "
				"	THEN (CASE WHEN HCFAClaimProvidersT.Box24J_ProviderID Is Not Null "
				"		THEN HCFAClaimProvidersT.Box24J_ProviderID "
				"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"			ELSE ChargeProvidersT.ClaimProviderID END) "
				"		END) "
				"ELSE ChargeProvidersT.ClaimProviderID END) END AS Box24J_ProviderID, "

				//now find out who the ANSI UB 2310B provider is (Box 77)				
				"CASE WHEN BillsT.FormType = 2 THEN "
				"	CASE WHEN UB92SetupT.UB04Box77Setup = 1 THEN "
				"		CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ChargeProvidersT.ClaimProviderID END "
				"	WHEN UB92SetupT.UB04Box77Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"	WHEN UB92SetupT.UB04Box77Setup = 3 THEN ReferringPhysT.PersonID "
				"	ELSE -1 END "
				"ELSE -1 END AS UB_2310B_ProviderID, "

				//now find out who the ANSI UB 2310C provider is (Box 78)				
				"CASE WHEN BillsT.FormType = 2 THEN "
				"	CASE WHEN UB92SetupT.Box83Setup = 1 THEN "
				"		CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ChargeProvidersT.ClaimProviderID END "
				"	WHEN UB92SetupT.Box83Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"	WHEN UB92SetupT.Box83Setup = 3 THEN ReferringPhysT.PersonID "
				"	ELSE -1 END "
				"ELSE -1 END AS UB_2310C_ProviderID, "

				//get the payer IDs
				"CASE WHEN HCFA_Primary_ClaimPayerIDsT.ID Is Not Null THEN HCFA_Primary_ClaimPayerIDsT.EbillingID ELSE HCFAPayerIDsT_PrimaryT.EBillingID END AS HCFAPrimaryPayerID, "
				"CASE WHEN HCFA_Othr_ClaimPayerIDsT.ID Is Not Null THEN HCFA_Othr_ClaimPayerIDsT.EbillingID ELSE HCFAPayerIDsT_OthrT.EBillingID END AS HCFAOthrPayerID, "
				"CASE WHEN UB_Primary_ClaimPayerIDsT.ID Is Not Null THEN UB_Primary_ClaimPayerIDsT.EbillingID ELSE UBPayerIDsT_PrimaryT.EbillingID END AS UBPrimaryPayerID, "
				"CASE WHEN UB_Othr_ClaimPayerIDsT.ID Is Not Null THEN UB_Othr_ClaimPayerIDsT.EbillingID ELSE UBPayerIDsT_OthrT.EBillingID END AS UBOthrPayerID, "

				"HCFASetupT.ANSI_SendTPLNumber AS HCFA_SendTPLCode, UB92SetupT.ANSI_SendTPLNumber AS UB92_SendTPLCode, OthrInsuranceCoT.TPLCode AS OtherTPLCode, "
				"OtherHCFASetupT.ANSI_Hide2330AREF AS OtherHCFA_Hide2330AREF, OtherUB92SetupT.ANSI_Hide2330AREF AS OtherUB92_Hide2330AREF, "

				"BillsT.InsuredPartyID, BillsT.OthrInsuredPartyID, BillsT.Location, Min(PlaceOfServiceCodesT.PlaceCodes) AS POSCode, "
				"InsuranceCoT.PersonID AS InsuranceCoID, InsuranceCoT.HCFASetupGroupID, InsuranceCoT.UB92SetupGroupID, "
				"LineItemT.LocationID, Min(ChargesT.ServiceDateFrom) AS FirstDateOfService, "
				"HCFASetupT.ShowAnesthesiaTimes, HCFASetupT.ShowAnesthMinutesOnly, "
				"CASE WHEN InsuredPartyT.RespTypeID = 1 OR InsuredPartyT.SubmitAsPrimary = 1 OR OthrInsuredPartyT.PersonID Is Null THEN 1 ELSE 0 END AS IsPrimary, "
				"RespTypeT.Priority AS InsuredPartyPriority, OthrRespTypeT.Priority AS OthrInsuredPartyPriority, "
				"LocationsT.NPI AS BillLocationNPI, "
				"CASE WHEN LTRIM(RTRIM(BillsT.MedicaidResubmission)) <> '' THEN BillsT.MedicaidResubmission ELSE BillsT.OriginalRefNo END AS OriginalReferenceNumber "
				"FROM BillsT "
				"LEFT JOIN HCFATrackT ON BillsT.ID = HCFATrackT.BillID "
				"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
				"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
				"LEFT JOIN ProvidersT ChargeProvidersT ON ChargesT.DoctorsProviders = ChargeProvidersT.PersonID "
				"LEFT JOIN ProvidersT PatientProvidersT ON PatientsT.MainPhysician = PatientProvidersT.PersonID "
				"LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
				"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN InsuranceLocationPayerIDsT ON InsuranceCoT.PersonID = InsuranceLocationPayerIDsT.InsuranceCoID AND LineItemT.LocationID = InsuranceLocationPayerIDsT.LocationID "
				"LEFT JOIN EbillingInsCoIDs AS HCFA_Primary_ClaimPayerIDsT ON InsuranceLocationPayerIDsT.ClaimPayerID = HCFA_Primary_ClaimPayerIDsT.ID "
				"LEFT JOIN EbillingInsCoIDs AS UB_Primary_ClaimPayerIDsT ON InsuranceLocationPayerIDsT.UBPayerID = UB_Primary_ClaimPayerIDsT.ID "
				"LEFT JOIN EbillingInsCoIDs AS UBPayerIDsT_PrimaryT ON InsuranceCoT.UBPayerID = UBPayerIDsT_PrimaryT.ID "
				"LEFT JOIN EbillingInsCoIDs AS HCFAPayerIDsT_PrimaryT ON InsuranceCoT.HCFAPayerID = HCFAPayerIDsT_PrimaryT.ID "
				"LEFT JOIN InsuredPartyT OthrInsuredPartyT ON BillsT.OthrInsuredPartyID = OthrInsuredPartyT.PersonID "
				"LEFT JOIN RespTypeT OthrRespTypeT ON OthrInsuredPartyT.RespTypeID = OthrRespTypeT.ID "
				"LEFT JOIN InsuranceCoT OthrInsuranceCoT ON OthrInsuredPartyT.InsuranceCoID = OthrInsuranceCoT.PersonID "
				"LEFT JOIN InsuranceLocationPayerIDsT OthrInsuranceLocationPayerIDsT ON OthrInsuranceCoT.PersonID = OthrInsuranceLocationPayerIDsT.InsuranceCoID AND LineItemT.LocationID = OthrInsuranceLocationPayerIDsT.LocationID "
				"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				"LEFT JOIN EbillingInsCoIDs AS HCFA_Othr_ClaimPayerIDsT ON OthrInsuranceLocationPayerIDsT.ClaimPayerID = HCFA_Othr_ClaimPayerIDsT.ID "
				"LEFT JOIN EbillingInsCoIDs AS UB_Othr_ClaimPayerIDsT ON OthrInsuranceLocationPayerIDsT.UBPayerID = UB_Othr_ClaimPayerIDsT.ID "
				"LEFT JOIN EbillingInsCoIDs AS UBPayerIDsT_OthrT ON OthrInsuranceCoT.UBPayerID = UBPayerIDsT_OthrT.ID "
				"LEFT JOIN EbillingInsCoIDs AS HCFAPayerIDsT_OthrT ON OthrInsuranceCoT.HCFAPayerID = HCFAPayerIDsT_OthrT.ID "
				"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
				"LEFT JOIN UB92SetupT ON InsuranceCoT.UB92SetupGroupID = UB92SetupT.ID "
				"LEFT JOIN HCFASetupT AS OtherHCFASetupT ON OthrInsuranceCoT.HCFASetupGroupID = OtherHCFASetupT.ID "
				"LEFT JOIN UB92SetupT AS OtherUB92SetupT ON OthrInsuranceCoT.UB92SetupGroupID = OtherUB92SetupT.ID "
				"LEFT JOIN HCFAClaimProvidersT ON InsuranceCoT.PersonID = HCFAClaimProvidersT.InsuranceCoID AND (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.PersonID ELSE ChargeProvidersT.PersonID END) = HCFAClaimProvidersT.ProviderID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "

				"WHERE Coalesce(BillStatusT.Type, -1) != %li "

				"GROUP BY HCFATrackT.ID, BillsT.ID, BillsT.FormType, BillsT.PatientID, PatientsT.UserDefinedID, HCFASetupT.Box33Setup, "
				"UB92SetupT.Box82Setup, UB92SetupT.UB04Box77Setup, UB92SetupT.Box83Setup, BillsT.RefPhyID, BillsT.SupervisingProviderID, "

				//we have to duplicate the provider ID logic here in the GROUP BY clause
				"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
				"(CASE WHEN BillsT.FormType = 2 "
				"THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"	WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
				"	ELSE ChargeProvidersT.ClaimProviderID END) "
				"WHEN BillsT.FormType = 1 "
				"	THEN (CASE WHEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID Is Not Null "
				"		THEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID "
				"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"			ELSE ChargeProvidersT.ClaimProviderID END) "
				"		END) "
				"ELSE ChargeProvidersT.ClaimProviderID END) END, "

				"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
				"(CASE WHEN BillsT.FormType = 2 "
				"THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"	WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
				"	ELSE ChargeProvidersT.ClaimProviderID END) "
				"WHEN BillsT.FormType = 1 "
				"	THEN (CASE WHEN HCFAClaimProvidersT.ANSI_2310B_ProviderID Is Not Null "
				"		THEN HCFAClaimProvidersT.ANSI_2310B_ProviderID "
				"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"			ELSE ChargeProvidersT.ClaimProviderID END) "
				"		END) "
				"ELSE ChargeProvidersT.ClaimProviderID END) END, "

				"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
				"(CASE WHEN BillsT.FormType = 2 "
				"THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"	WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
				"	ELSE ChargeProvidersT.ClaimProviderID END) "
				"WHEN BillsT.FormType = 1 "
				"	THEN (CASE WHEN HCFAClaimProvidersT.Box24J_ProviderID Is Not Null "
				"		THEN HCFAClaimProvidersT.Box24J_ProviderID "
				"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"			ELSE ChargeProvidersT.ClaimProviderID END) "
				"		END) "
				"ELSE ChargeProvidersT.ClaimProviderID END) END, "
		
				"CASE WHEN BillsT.FormType = 2 THEN "
				"	CASE WHEN UB92SetupT.UB04Box77Setup = 1 THEN "
				"		CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ChargeProvidersT.ClaimProviderID END "
				"	WHEN UB92SetupT.UB04Box77Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"	WHEN UB92SetupT.UB04Box77Setup = 3 THEN ReferringPhysT.PersonID "
				"	ELSE -1 END "
				"ELSE -1 END, "

				//now find out who the ANSI UB 2310C provider is (Box 78)				
				"CASE WHEN BillsT.FormType = 2 THEN "
				"	CASE WHEN UB92SetupT.Box83Setup = 1 THEN "
				"		CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ChargeProvidersT.ClaimProviderID END "
				"	WHEN UB92SetupT.Box83Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"	WHEN UB92SetupT.Box83Setup = 3 THEN ReferringPhysT.PersonID "
				"	ELSE -1 END "
				"ELSE -1 END, "

				"CASE WHEN HCFA_Primary_ClaimPayerIDsT.ID Is Not Null THEN HCFA_Primary_ClaimPayerIDsT.EbillingID ELSE HCFAPayerIDsT_PrimaryT.EBillingID END, "
				"CASE WHEN HCFA_Othr_ClaimPayerIDsT.ID Is Not Null THEN HCFA_Othr_ClaimPayerIDsT.EbillingID ELSE HCFAPayerIDsT_OthrT.EBillingID END, "
				"CASE WHEN UB_Primary_ClaimPayerIDsT.ID Is Not Null THEN UB_Primary_ClaimPayerIDsT.EbillingID ELSE UBPayerIDsT_PrimaryT.EbillingID END, "
				"CASE WHEN UB_Othr_ClaimPayerIDsT.ID Is Not Null THEN UB_Othr_ClaimPayerIDsT.EbillingID ELSE UBPayerIDsT_OthrT.EBillingID END, "
				
				"HCFASetupT.ANSI_SendTPLNumber, UB92SetupT.ANSI_SendTPLNumber, OthrInsuranceCoT.TPLCode, "
				"OtherHCFASetupT.ANSI_Hide2330AREF, OtherUB92SetupT.ANSI_Hide2330AREF, "

				"HCFATrackT.Batch, HCFATrackT.CurrentSelect, BillsT.InsuredPartyID, BillsT.OthrInsuredPartyID, "
				"BillsT.Location, InsuranceCoT.PersonID, InsuranceCoT.HCFASetupGroupID, InsuranceCoT.UB92SetupGroupID, LineItemT.LocationID, "
				"RespTypeT.Priority, OthrRespTypeT.Priority, "
				"Convert(int,BillsT.Deleted), Convert(int,LineItemT.Deleted), Convert(int,ChargesT.Batched), HCFASetupT.ShowAnesthesiaTimes, HCFASetupT.ShowAnesthMinutesOnly, InsuredPartyT.RespTypeID, InsuredPartyT.SubmitAsPrimary, OthrInsuredPartyT.PersonID, LocationsT.NPI, "
				"LineItemCorrections_OriginalChargesQ.OriginalLineItemID, LineItemCorrections_VoidingChargesQ.VoidingLineItemID, "
				"CASE WHEN LTRIM(RTRIM(BillsT.MedicaidResubmission)) <> '' THEN BillsT.MedicaidResubmission ELSE BillsT.OriginalRefNo END "
				"HAVING %s" 
				"AND Convert(int,BillsT.Deleted) = 0 "
				"AND Convert(int,LineItemT.Deleted) = 0 AND Convert(int,ChargesT.Batched) = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"%s %s %s) AS HCFAEbillingQ "
				"ORDER BY %s", EBillStatusType::OnHold, strBatchFilter, strLocationFilter, strProviderFilter, strClaimType, strOrderBy);
		}
		// (j.jones 2011-06-27 16:05) - PLID 44335 - changed the else to be HCFA Image only
		else if(m_FormatStyle == IMAGE) {
			//if HCFA Image

			// (j.jones 2009-06-23 12:09) - PLID 34689 - supported SubmitAsPrimary in the IsPrimary calculation			
			CString strOrderBy = "InsType, InsuranceCoID, ProviderID, HCFAID";
			
			// (j.jones 2009-08-14 14:06) - PLID 35235 - if OHIP is separating claims by provider,
			// (j.dinatale 2012-12-27 12:38) - PLID 54365 - removed code here that was trying to create a provider filter. It was checking if 
			//		we were exporting for OHIP but we arent here, so removed it.

			//can't parameterize this query
			// (j.jones 2009-12-16 15:54) - PLID 36237 - split payer IDs out into HCFA and UB payer IDs
			// (j.jones 2010-05-12 14:40) - PLID 38615 - IsPrimary is now always TRUE if there is no "Other" insurance
			// (j.jones 2010-08-30 17:12) - PLID 15025 - added SendTPLCode and OtherTPLCode
			// (j.jones 2010-11-09 12:57) - PLID 41389 - supported ChargesT.ClaimProviderID
			// (j.jones 2011-06-17 13:21) - PLID 39798 - removed a bad %s that had no parameter to go with it!
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			// (j.jones 2011-08-19 10:00) - PLID 44984 - added UB04 Box77 and 78
			// (j.jones 2011-08-23 08:53) - PLID 44984 - added UB 2310B and 2310C provider ID
			// (j.jones 2012-01-18 15:06) - PLID 47627 - added OtherHCFA_Hide2330AREF and OtherUB92_Hide2330AREF,
			// both are unused in image so they default to 0
			// (j.jones 2012-08-06 14:54) - PLID 51916 - supported the new payer ID data structure, always pulling from EbillingInsCoIDs
			// (j.jones 2013-04-24 17:27) - PLID 55564 - we now get the POS designation code (11, 22, etc.)
			// (j.jones 2013-06-20 12:27) - PLID 57245 - moved medicaid resubmission and original ref no logic
			// into the query (if both exist, medicaid resubmission is used)
			// (j.jones 2014-07-09 10:08) - PLID 62568 - ignore on hold claims
			// (j.jones 2014-08-22 13:58) - PLID 63446 - we now load the earliest date of service on the claim
			rs = CreateRecordset("SELECT * FROM "
				"(SELECT HCFATrackT.ID AS HCFAID, BillsT.ID AS BillID, BillsT.PatientID, PatientsT.UserDefinedID, HCFASetupT.Box33Setup, "
				"-1 AS Box82Setup, -1 AS Box77Setup, -1 AS Box78Setup, BillsT.RefPhyID, BillsT.SupervisingProviderID, "

				//find who the master (billing) provider is
				"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
				"(CASE WHEN BillsT.FormType = 1 "
				"	THEN (CASE WHEN HCFAClaimProvidersT.Box33_ProviderID Is Not Null "
				"		THEN HCFAClaimProvidersT.Box33_ProviderID "
				"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"			ELSE ChargeProvidersT.ClaimProviderID END) "
				"		END) "
				"ELSE ChargeProvidersT.ClaimProviderID END) END AS ProviderID, "

				//we don't care what the ANSI Rendering Provider is in this query,
				//but the code is going to look for this field, so assign -1
				"-1 AS ANSI_RenderingProviderID, "

				//now find out who the HCFA Box24J provider is
				"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
				"(CASE WHEN BillsT.FormType = 1 "
				"	THEN (CASE WHEN HCFAClaimProvidersT.Box24J_ProviderID Is Not Null "
				"		THEN HCFAClaimProvidersT.Box24J_ProviderID "
				"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"			ELSE ChargeProvidersT.ClaimProviderID END) "
				"		END) "
				"ELSE ChargeProvidersT.ClaimProviderID END) END AS Box24J_ProviderID, "

				//this query does not need to load UB providers
				"-1 AS UB_2310B_ProviderID, "
				"-1 AS UB_2310C_ProviderID, "

				//get the payer IDs
				"CASE WHEN HCFA_Primary_ClaimPayerIDsT.ID Is Not Null THEN HCFA_Primary_ClaimPayerIDsT.EbillingID ELSE HCFAPayerIDsT_PrimaryT.EBillingID END AS HCFAPrimaryPayerID, "
				"CASE WHEN HCFA_Othr_ClaimPayerIDsT.ID Is Not Null THEN HCFA_Othr_ClaimPayerIDsT.EbillingID ELSE HCFAPayerIDsT_OthrT.EBillingID END AS HCFAOthrPayerID, "
				"CASE WHEN UB_Primary_ClaimPayerIDsT.ID Is Not Null THEN UB_Primary_ClaimPayerIDsT.EbillingID ELSE UBPayerIDsT_PrimaryT.EbillingID END AS UBPrimaryPayerID, "
				"CASE WHEN UB_Othr_ClaimPayerIDsT.ID Is Not Null THEN UB_Othr_ClaimPayerIDsT.EbillingID ELSE UBPayerIDsT_OthrT.EBillingID END AS UBOthrPayerID, "

				"HCFASetupT.ANSI_SendTPLNumber AS HCFA_SendTPLCode, UB92SetupT.ANSI_SendTPLNumber AS UB92_SendTPLCode, OthrInsuranceCoT.TPLCode AS OtherTPLCode, "
				"0 AS OtherHCFA_Hide2330AREF, 0 AS OtherUB92_Hide2330AREF, "
				
				"BillsT.InsuredPartyID, BillsT.OthrInsuredPartyID, BillsT.Location, Min(PlaceOfServiceCodesT.PlaceCodes) AS POSCode, "
				"InsuranceCoT.PersonID AS InsuranceCoID, InsuranceCoT.HCFASetupGroupID, InsuranceCoT.UB92SetupGroupID, "
				"LineItemT.LocationID, Min(ChargesT.ServiceDateFrom) AS FirstDateOfService, "
				"HCFASetupT.ShowAnesthesiaTimes, HCFASetupT.ShowAnesthMinutesOnly, InsuranceCoT.InsType, "
				"CASE WHEN InsuredPartyT.RespTypeID = 1 OR InsuredPartyT.SubmitAsPrimary = 1 OR OthrInsuredPartyT.PersonID Is Null THEN 1 ELSE 0 END AS IsPrimary, "
				"RespTypeT.Priority AS InsuredPartyPriority, OthrRespTypeT.Priority AS OthrInsuredPartyPriority, "
				"LocationsT.NPI AS BillLocationNPI, "
				"CASE WHEN LTRIM(RTRIM(BillsT.MedicaidResubmission)) <> '' THEN BillsT.MedicaidResubmission ELSE BillsT.OriginalRefNo END AS OriginalReferenceNumber "
				"FROM HCFATrackT "
				"INNER JOIN BillsT ON HCFATrackT.BillID = BillsT.ID "
				"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
				"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
				"LEFT JOIN ProvidersT ChargeProvidersT ON ChargesT.DoctorsProviders = ChargeProvidersT.PersonID "
				"LEFT JOIN ProvidersT PatientProvidersT ON PatientsT.MainPhysician = PatientProvidersT.PersonID "
				"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN InsuranceLocationPayerIDsT ON InsuranceCoT.PersonID = InsuranceLocationPayerIDsT.InsuranceCoID AND LineItemT.LocationID = InsuranceLocationPayerIDsT.LocationID "
				"LEFT JOIN EbillingInsCoIDs AS HCFA_Primary_ClaimPayerIDsT ON InsuranceLocationPayerIDsT.ClaimPayerID = HCFA_Primary_ClaimPayerIDsT.ID "
				"LEFT JOIN EbillingInsCoIDs AS UB_Primary_ClaimPayerIDsT ON InsuranceLocationPayerIDsT.UBPayerID = UB_Primary_ClaimPayerIDsT.ID "
				"LEFT JOIN EbillingInsCoIDs AS UBPayerIDsT_PrimaryT ON InsuranceCoT.UBPayerID = UBPayerIDsT_PrimaryT.ID "
				"LEFT JOIN EbillingInsCoIDs AS HCFAPayerIDsT_PrimaryT ON InsuranceCoT.HCFAPayerID = HCFAPayerIDsT_PrimaryT.ID "
				"LEFT JOIN InsuredPartyT OthrInsuredPartyT ON BillsT.OthrInsuredPartyID = OthrInsuredPartyT.PersonID "
				"LEFT JOIN RespTypeT OthrRespTypeT ON OthrInsuredPartyT.RespTypeID = OthrRespTypeT.ID "
				"LEFT JOIN InsuranceCoT OthrInsuranceCoT ON OthrInsuredPartyT.InsuranceCoID = OthrInsuranceCoT.PersonID "
				"LEFT JOIN InsuranceLocationPayerIDsT OthrInsuranceLocationPayerIDsT ON OthrInsuranceCoT.PersonID = OthrInsuranceLocationPayerIDsT.InsuranceCoID AND LineItemT.LocationID = OthrInsuranceLocationPayerIDsT.LocationID "
				"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				"LEFT JOIN EbillingInsCoIDs AS HCFA_Othr_ClaimPayerIDsT ON OthrInsuranceLocationPayerIDsT.ClaimPayerID = HCFA_Othr_ClaimPayerIDsT.ID "
				"LEFT JOIN EbillingInsCoIDs AS UB_Othr_ClaimPayerIDsT ON OthrInsuranceLocationPayerIDsT.UBPayerID = UB_Othr_ClaimPayerIDsT.ID "
				"LEFT JOIN EbillingInsCoIDs AS UBPayerIDsT_OthrT ON OthrInsuranceCoT.UBPayerID = UBPayerIDsT_OthrT.ID "
				"LEFT JOIN EbillingInsCoIDs AS HCFAPayerIDsT_OthrT ON OthrInsuranceCoT.HCFAPayerID = HCFAPayerIDsT_OthrT.ID "
				"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
				"LEFT JOIN UB92SetupT ON InsuranceCoT.UB92SetupGroupID = UB92SetupT.ID "
				"LEFT JOIN HCFAClaimProvidersT ON InsuranceCoT.PersonID = HCFAClaimProvidersT.InsuranceCoID AND (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.PersonID ELSE ChargeProvidersT.PersonID END) = HCFAClaimProvidersT.ProviderID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "

				"WHERE Coalesce(BillStatusT.Type, -1) != %li "

				"GROUP BY HCFATrackT.ID, BillsT.ID, BillsT.FormType, BillsT.PatientID, PatientsT.UserDefinedID, HCFASetupT.Box33Setup, BillsT.RefPhyID, BillsT.SupervisingProviderID, "
				
				//we have to duplicate the provider ID logic here in the GROUP BY clause
				"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
				"(CASE WHEN BillsT.FormType = 1 "
				"	THEN (CASE WHEN HCFAClaimProvidersT.Box33_ProviderID Is Not Null "
				"		THEN HCFAClaimProvidersT.Box33_ProviderID "
				"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"			ELSE ChargeProvidersT.ClaimProviderID END) "
				"		END) "
				"ELSE ChargeProvidersT.ClaimProviderID END) END, "

				"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
				"(CASE WHEN BillsT.FormType = 1 "
				"	THEN (CASE WHEN HCFAClaimProvidersT.Box24J_ProviderID Is Not Null "
				"		THEN HCFAClaimProvidersT.Box24J_ProviderID "
				"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"			ELSE ChargeProvidersT.ClaimProviderID END) "
				"		END) "
				"ELSE ChargeProvidersT.ClaimProviderID END) END, "

				"CASE WHEN HCFA_Primary_ClaimPayerIDsT.ID Is Not Null THEN HCFA_Primary_ClaimPayerIDsT.EbillingID ELSE HCFAPayerIDsT_PrimaryT.EBillingID END, "
				"CASE WHEN HCFA_Othr_ClaimPayerIDsT.ID Is Not Null THEN HCFA_Othr_ClaimPayerIDsT.EbillingID ELSE HCFAPayerIDsT_OthrT.EBillingID END, "
				"CASE WHEN UB_Primary_ClaimPayerIDsT.ID Is Not Null THEN UB_Primary_ClaimPayerIDsT.EbillingID ELSE UBPayerIDsT_PrimaryT.EbillingID END, "
				"CASE WHEN UB_Othr_ClaimPayerIDsT.ID Is Not Null THEN UB_Othr_ClaimPayerIDsT.EbillingID ELSE UBPayerIDsT_OthrT.EBillingID END, "

				"HCFASetupT.ANSI_SendTPLNumber, UB92SetupT.ANSI_SendTPLNumber, OthrInsuranceCoT.TPLCode, "
				"HCFATrackT.Batch, HCFATrackT.CurrentSelect, BillsT.InsuredPartyID, BillsT.OthrInsuredPartyID, "
				"BillsT.Location, InsuranceCoT.PersonID, InsuranceCoT.HCFASetupGroupID, InsuranceCoT.UB92SetupGroupID, LineItemT.LocationID, "
				"RespTypeT.Priority, OthrRespTypeT.Priority, "
				"InsuranceCoT.InsType, "
				"Convert(int,BillsT.Deleted), Convert(int,LineItemT.Deleted), Convert(int,ChargesT.Batched), HCFASetupT.ShowAnesthesiaTimes, HCFASetupT.ShowAnesthMinutesOnly, InsuredPartyT.RespTypeID, InsuredPartyT.SubmitAsPrimary, OthrInsuredPartyT.PersonID, LocationsT.NPI, "
				"LineItemCorrections_OriginalChargesQ.OriginalLineItemID, LineItemCorrections_VoidingChargesQ.VoidingLineItemID, "
				"CASE WHEN LTRIM(RTRIM(BillsT.MedicaidResubmission)) <> '' THEN BillsT.MedicaidResubmission ELSE BillsT.OriginalRefNo END "
				"HAVING HCFATrackT.Batch = 2 AND HCFATrackT.CurrentSelect = 1 AND Convert(int,BillsT.Deleted) = 0 "
				"AND Convert(int,LineItemT.Deleted) = 0 AND Convert(int,ChargesT.Batched) = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"%s %s %s) AS HCFAEbillingQ "
				"ORDER BY %s", EBillStatusType::OnHold, strLocationFilter, strProviderFilter, strClaimType, strOrderBy);
		}
		else {
			//OHIP or Alberta

			// (j.jones 2011-06-27 16:05) - PLID 44335 - made a unique query for Canada

			CString strOrderBy = "ProviderID, InsuranceCoID, HCFAID";

			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges

			//can't parameterize this query
			// (j.jones 2011-08-19 10:00) - PLID 44984 - added UB04 Box77 and 78
			// (j.jones 2011-08-23 08:53) - PLID 44984 - added UB 2310B and 2310C provider ID
			// (j.jones 2012-01-18 15:06) - PLID 47627 - added OtherHCFA_Hide2330AREF and OtherUB92_Hide2330AREF,
			// both are unused in Canadia so they default to 0
			// (j.jones 2012-08-06 14:54) - PLID 51916 - supported the new payer ID data structure, always pulling from EbillingInsCoIDs
			// (j.jones 2013-04-24 17:27) - PLID 55564 - we now get the POS designation code (11, 22, etc.)
			// (j.jones 2013-06-20 12:27) - PLID 57245 - moved medicaid resubmission and original ref no logic
			// into the query (if both exist, medicaid resubmission is used)
			// (j.jones 2014-07-09 10:08) - PLID 62568 - ignore on hold claims
			// (j.jones 2014-08-22 13:58) - PLID 63446 - we now load the earliest date of service on the claim
			rs = CreateRecordset("SELECT * FROM "
				"(SELECT HCFATrackT.ID AS HCFAID, BillsT.ID AS BillID, BillsT.PatientID, PatientsT.UserDefinedID, HCFASetupT.Box33Setup, "
				"-1 AS Box82Setup, -1 AS Box77Setup, -1 AS Box78Setup, BillsT.RefPhyID, BillsT.SupervisingProviderID, "

				"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ChargesT.DoctorsProviders END AS ProviderID, "

				//we don't care what the ANSI Rendering Provider is in this query,
				//but the code is going to look for this field, so assign -1
				"-1 AS ANSI_RenderingProviderID, "

				"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ChargesT.DoctorsProviders END AS Box24J_ProviderID, "

				//this query does not need to load UB providers
				"-1 AS UB_2310B_ProviderID, "
				"-1 AS UB_2310C_ProviderID, "

				//get the payer IDs
				"CASE WHEN HCFA_Primary_ClaimPayerIDsT.ID Is Not Null THEN HCFA_Primary_ClaimPayerIDsT.EbillingID ELSE HCFAPayerIDsT_PrimaryT.EBillingID END AS HCFAPrimaryPayerID, "
				"CASE WHEN HCFA_Othr_ClaimPayerIDsT.ID Is Not Null THEN HCFA_Othr_ClaimPayerIDsT.EbillingID ELSE HCFAPayerIDsT_OthrT.EBillingID END AS HCFAOthrPayerID, "
				"'' AS UBPrimaryPayerID, "
				"'' AS UBOthrPayerID, "

				"HCFASetupT.ANSI_SendTPLNumber AS HCFA_SendTPLCode, UB92SetupT.ANSI_SendTPLNumber AS UB92_SendTPLCode, OthrInsuranceCoT.TPLCode AS OtherTPLCode, "
				"0 AS OtherHCFA_Hide2330AREF, 0 AS OtherUB92_Hide2330AREF, "
				
				"BillsT.InsuredPartyID, BillsT.OthrInsuredPartyID, BillsT.Location, Min(PlaceOfServiceCodesT.PlaceCodes) AS POSCode, "
				"InsuranceCoT.PersonID AS InsuranceCoID, InsuranceCoT.HCFASetupGroupID, InsuranceCoT.UB92SetupGroupID, "
				"LineItemT.LocationID, Min(ChargesT.ServiceDateFrom) AS FirstDateOfService, "
				"HCFASetupT.ShowAnesthesiaTimes, HCFASetupT.ShowAnesthMinutesOnly, InsuranceCoT.InsType, "
				"CASE WHEN InsuredPartyT.RespTypeID = 1 OR InsuredPartyT.SubmitAsPrimary = 1 OR OthrInsuredPartyT.PersonID Is Null THEN 1 ELSE 0 END AS IsPrimary, "
				"RespTypeT.Priority AS InsuredPartyPriority, OthrRespTypeT.Priority AS OthrInsuredPartyPriority, "
				"LocationsT.NPI AS BillLocationNPI, "
				"CASE WHEN LTRIM(RTRIM(BillsT.MedicaidResubmission)) <> '' THEN BillsT.MedicaidResubmission ELSE BillsT.OriginalRefNo END AS OriginalReferenceNumber "
				"FROM HCFATrackT "
				"INNER JOIN BillsT ON HCFATrackT.BillID = BillsT.ID "
				"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
				"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
				"LEFT JOIN ProvidersT ChargeProvidersT ON ChargesT.DoctorsProviders = ChargeProvidersT.PersonID "
				"LEFT JOIN ProvidersT PatientProvidersT ON PatientsT.MainPhysician = PatientProvidersT.PersonID "
				"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				"LEFT JOIN InsuranceLocationPayerIDsT ON InsuranceCoT.PersonID = InsuranceLocationPayerIDsT.InsuranceCoID AND LineItemT.LocationID = InsuranceLocationPayerIDsT.LocationID "
				"LEFT JOIN EbillingInsCoIDs AS HCFA_Primary_ClaimPayerIDsT ON InsuranceLocationPayerIDsT.ClaimPayerID = HCFA_Primary_ClaimPayerIDsT.ID "
				"LEFT JOIN EbillingInsCoIDs AS HCFAPayerIDsT_PrimaryT ON InsuranceCoT.HCFAPayerID = HCFAPayerIDsT_PrimaryT.ID "
				"LEFT JOIN InsuredPartyT OthrInsuredPartyT ON BillsT.OthrInsuredPartyID = OthrInsuredPartyT.PersonID "
				"LEFT JOIN RespTypeT OthrRespTypeT ON OthrInsuredPartyT.RespTypeID = OthrRespTypeT.ID "
				"LEFT JOIN InsuranceCoT OthrInsuranceCoT ON OthrInsuredPartyT.InsuranceCoID = OthrInsuranceCoT.PersonID "
				"LEFT JOIN InsuranceLocationPayerIDsT OthrInsuranceLocationPayerIDsT ON OthrInsuranceCoT.PersonID = OthrInsuranceLocationPayerIDsT.InsuranceCoID AND LineItemT.LocationID = OthrInsuranceLocationPayerIDsT.LocationID "
				"LEFT JOIN EbillingInsCoIDs AS HCFA_Othr_ClaimPayerIDsT ON OthrInsuranceLocationPayerIDsT.ClaimPayerID = HCFA_Othr_ClaimPayerIDsT.ID "
				"LEFT JOIN EbillingInsCoIDs AS HCFAPayerIDsT_OthrT ON OthrInsuranceCoT.HCFAPayerID = HCFAPayerIDsT_OthrT.ID "
				"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
				"LEFT JOIN UB92SetupT ON InsuranceCoT.UB92SetupGroupID = UB92SetupT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "

				"WHERE Coalesce(BillStatusT.Type, -1) != %li "
				
				"GROUP BY HCFATrackT.ID, BillsT.ID, BillsT.FormType, BillsT.PatientID, PatientsT.UserDefinedID, HCFASetupT.Box33Setup, BillsT.RefPhyID, BillsT.SupervisingProviderID, "
				
				//we have to duplicate the provider ID logic here in the GROUP BY clause
				"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ChargesT.DoctorsProviders END, "
				"CASE WHEN HCFA_Primary_ClaimPayerIDsT.ID Is Not Null THEN HCFA_Primary_ClaimPayerIDsT.EbillingID ELSE HCFAPayerIDsT_PrimaryT.EBillingID END, "
				"CASE WHEN HCFA_Othr_ClaimPayerIDsT.ID Is Not Null THEN HCFA_Othr_ClaimPayerIDsT.EbillingID ELSE HCFAPayerIDsT_OthrT.EBillingID END, "

				"HCFASetupT.ANSI_SendTPLNumber, UB92SetupT.ANSI_SendTPLNumber, OthrInsuranceCoT.TPLCode, "
				"HCFATrackT.Batch, HCFATrackT.CurrentSelect, BillsT.InsuredPartyID, BillsT.OthrInsuredPartyID, "
				"BillsT.Location, InsuranceCoT.PersonID, InsuranceCoT.HCFASetupGroupID, InsuranceCoT.UB92SetupGroupID, LineItemT.LocationID, "
				"RespTypeT.Priority, OthrRespTypeT.Priority, "
				"InsuranceCoT.InsType, "
				"Convert(int,BillsT.Deleted), Convert(int,LineItemT.Deleted), Convert(int,ChargesT.Batched), HCFASetupT.ShowAnesthesiaTimes, HCFASetupT.ShowAnesthMinutesOnly, InsuredPartyT.RespTypeID, InsuredPartyT.SubmitAsPrimary, OthrInsuredPartyT.PersonID, LocationsT.NPI, "
				"LineItemCorrections_OriginalChargesQ.OriginalLineItemID, LineItemCorrections_VoidingChargesQ.VoidingLineItemID, "
				"CASE WHEN LTRIM(RTRIM(BillsT.MedicaidResubmission)) <> '' THEN BillsT.MedicaidResubmission ELSE BillsT.OriginalRefNo END "
				"HAVING HCFATrackT.Batch = 2 AND HCFATrackT.CurrentSelect = 1 AND Convert(int,BillsT.Deleted) = 0 "
				"AND Convert(int,LineItemT.Deleted) = 0 AND Convert(int,ChargesT.Batched) = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"%s %s %s) AS HCFAEbillingQ "
				"ORDER BY %s", EBillStatusType::OnHold, strLocationFilter, strProviderFilter, strClaimType, strOrderBy);
		}

		if(rs->eof) {
			//no HCFAs!
			rs->Close();
			AfxMessageBox("There are no HCFAs to be exported.");
			EndDialog(IDCANCEL);
			// (j.jones 2007-10-05 14:40) - PLID 25867 - changed from returning Success to returning Cancel_Silent
			//return Cancel_Silent so that we don't fire any other warnings after this one
			return Cancel_Silent;
		}

		m_BatchCount = rs->GetRecordCount();

		CString str;
		str.Format("%li",m_BatchCount);
		SetDlgItemText(IDC_TOTALNUM,str);

		if (m_bIsClearinghouseIntegrationEnabled)
		{
			// One more operation after the claims finish exporting to the disk, the actual transfer.
			m_ProgIncrement = (float)100.0 / (float)(m_BatchCount + 1);
		}
		else
		{
			m_ProgIncrement = (float)100.0 / (float)m_BatchCount;
		}

		//store all this information into a struct, to keep less recordsets open and speed up the process.
		//again, this info will make subsequent queries more efficient
		while (!rs->eof) {
			FieldsPtr fields = rs->Fields;

			EBillingInfo *pNew = new EBillingInfo;
			// (j.jones 2012-03-21 15:12) - PLID 48870 - moved all data for the "Other" insured party into a pointer
			pNew->pOtherInsuranceInfo = OtherInsuranceInfo(); // (a.walling 2014-03-19 10:05) - PLID 61346 - Now just boost::optional, so initialize with a default OtherInsuranceInfo object

			pNew->HCFAID = AdoFldLong(fields->Item["HCFAID"], -1);
			pNew->BillID = AdoFldLong(fields->Item["BillID"], -1);
			pNew->PatientID = AdoFldLong(fields->Item["PatientID"], -1);
			pNew->UserDefinedID = AdoFldLong(fields->Item["UserDefinedID"], -1);
			pNew->HCFASetupID = AdoFldLong(fields->Item["HCFASetupGroupID"], -1);
			pNew->UB92SetupID = AdoFldLong(fields->Item["UB92SetupGroupID"], -1);

			//1 - Use Bill Provider, 2 - Use Provider From General 1, 3 - Use Override
			pNew->Box33Setup = AdoFldLong(fields->Item["Box33Setup"], 1);

			// (j.jones 2007-05-10 12:01) - PLID 25948 - needed the Box 82/76 setup for referring physician
			// (j.jones 2011-08-19 10:00) - PLID 44984 - added Box77 and 78
			//1 - Use Bill Provider, 2 - Use Provider From General 1, 3 - Use Referring Physician, 0 - Do Not Fill (not an option for Box76)
			pNew->Box82Setup = AdoFldLong(fields->Item["Box82Setup"], 1);
			pNew->Box77Setup = AdoFldLong(fields->Item["Box77Setup"], 0);
			pNew->Box78Setup = AdoFldLong(fields->Item["Box78Setup"], 0);

			// (j.jones 2008-12-11 13:28) - PLID 32401 - added RefPhyID and SupervisingProviderID, to reduce recordsets later
			pNew->nRefPhyID = VarLong(fields->Item["RefPhyID"]->Value, -1);
			pNew->nSupervisingProviderID = VarLong(fields->Item["SupervisingProviderID"]->Value, -1);

			pNew->ProviderID = AdoFldLong(fields->Item["ProviderID"], -1);

			// (j.jones 2008-04-02 16:52) - PLID 28995 - added ANSI RenderingProviderID and Box24J ProviderID,
			//which are usually the same as the ProviderID unless the advanced per-HCFA-Group Claim Provider
			//setup is in use			
			pNew->ANSI_RenderingProviderID = AdoFldLong(fields->Item["ANSI_RenderingProviderID"], -1);

			if (pNew->ANSI_RenderingProviderID == -1) {
				//this should never be -1 unless it is a non-ANSI export,
				//and in turn shouldn't be used in a non-ANSI export,
				//but assign the provider ID anyways for safety
				pNew->ANSI_RenderingProviderID = pNew->ProviderID;
			}

			pNew->Box24J_ProviderID = AdoFldLong(fields->Item["Box24J_ProviderID"], -1);

			// (j.jones 2011-08-23 08:53) - PLID 44984 - added UB 2310B and 2310C provider ID
			pNew->UB_2310B_ProviderID = AdoFldLong(fields->Item["UB_2310B_ProviderID"], -1);
			pNew->UB_2310C_ProviderID = AdoFldLong(fields->Item["UB_2310C_ProviderID"], -1);

			pNew->InsuredPartyID = AdoFldLong(fields->Item["InsuredPartyID"], -1);
			pNew->OthrInsuredPartyID = AdoFldLong(fields->Item["OthrInsuredPartyID"], -1);
			pNew->InsuranceCoID = AdoFldLong(fields->Item["InsuranceCoID"], -1);

			pNew->nPOSID = AdoFldLong(fields->Item["Location"], -1);
			// (j.jones 2013-04-24 17:27) - PLID 55564 - we now get the POS designation code (11, 22, etc.)
			pNew->strPOSCode = AdoFldString(fields->Item["POSCode"], "");
			// (j.jones 2013-04-25 10:02) - PLID 55564 - added boolean to track whether we should send the patient address as the POS
			//TRUE if strPOSCode is 12 and the Claim_SendPatAddressWhenPOS12 preference is on
			pNew->bSendPatientAddressAsPOS = FALSE;
			if (pNew->strPOSCode == "12" && GetRemotePropertyInt("Claim_SendPatAddressWhenPOS12", 1, 0, "<None>", true) == 1) {
				pNew->bSendPatientAddressAsPOS = TRUE;
			}

			pNew->BillLocation = AdoFldLong(fields->Item["LocationID"], -1);
			// (j.jones 2008-05-06 14:58) - PLID 29937 - added the Bill Location's NPI
			pNew->strBillLocationNPI = AdoFldString(fields->Item["BillLocationNPI"], "");

			// (j.jones 2009-08-05 10:18) - PLID 34467 - load the payer IDs
			// (j.jones 2009-12-16 15:54) - PLID 36237 - split out into HCFA and UB payer IDs
			pNew->strHCFAPayerID = AdoFldString(fields->Item["HCFAPrimaryPayerID"], "");
			pNew->strUBPayerID = AdoFldString(fields->Item["UBPrimaryPayerID"], "");

			// (j.jones 2010-08-30 17:12) - PLID 15025 - added SendTPLCode and OtherTPLCode
			// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
			if (m_actClaimType == actInst) {
				pNew->nSendTPLCode = AdoFldLong(fields->Item["UB92_SendTPLCode"], 1);
			}
			else {
				pNew->nSendTPLCode = AdoFldLong(fields->Item["HCFA_SendTPLCode"], 1);
			}

			// (j.jones 2012-03-21 15:12) - PLID 48870 - moved all data for the "Other" insured party into a pointer
			{
				//this pointer's nInsuredPartyID is redundant because it's the bill's OthrInsuredPartyID,
				//but the struct uses it later on
				pNew->pOtherInsuranceInfo->nInsuredPartyID = pNew->OthrInsuredPartyID;
				pNew->pOtherInsuranceInfo->strHCFAPayerID = AdoFldString(fields->Item["HCFAOthrPayerID"], "");
				pNew->pOtherInsuranceInfo->strUBPayerID = AdoFldString(fields->Item["UBOthrPayerID"], "");
				pNew->pOtherInsuranceInfo->strTPLCode = AdoFldString(fields->Item["OtherTPLCode"], "");

				// (j.jones 2012-01-18 15:06) - PLID 47627 - added OtherANSI_Hide2330AREF, which fills from the HCFA or UB group
				// for the "Other" insurance on the claim, instead of relying on the ANSI_Hide2330AREF value in m_HCFAInfo/m_UB92Info
				if (m_actClaimType == actInst) {
					pNew->pOtherInsuranceInfo->nANSI_Hide2330AREF = AdoFldLong(fields->Item["OtherUB92_Hide2330AREF"], 0);
				}
				else {
					pNew->pOtherInsuranceInfo->nANSI_Hide2330AREF = AdoFldLong(fields->Item["OtherHCFA_Hide2330AREF"], 0);
				}

				// (j.jones 2014-08-25 08:40) - PLID 54213 - Added bIsOnBill, HasPaid, and SBR01Qual.
				// HasPaid will not be checked if bIsOnBill is true, but set it to true to make sure
				// the payer that is on the bill is never skipped.
				pNew->pOtherInsuranceInfo->bIsOnBill = true;
				pNew->pOtherInsuranceInfo->bHasPaid = true;

				// (j.jones 2014-08-25 09:07) - PLID 54213 - track the qualifier to use in Loop 2320 SBR01
				pNew->pOtherInsuranceInfo->sbr01Qual = SBR01Qualifier::sbrU;
				if (pNew->OthrInsuredPartyID != -1) {
					pNew->pOtherInsuranceInfo->sbr01Qual = CalculateSBR01Qualifier(false, pNew->IsPrimary == 1 ? true : false,
						m_actClaimType != actInst ? m_HCFAInfo.ANSI_EnablePaymentInfo : m_UB92Info.ANSI_EnablePaymentInfo,
						AdoFldLong(fields->Item["OthrInsuredPartyPriority"]));
				}
			}

			//if the option is enabled and they have all the anesthesia billing set up, show the anesth. times in the charge list
			if (AdoFldLong(fields->Item["ShowAnesthesiaTimes"], -1) == 1
				// (j.jones 2007-10-18 08:36) - PLID 27757 - rewrote this check to be one query, not two,
				// that uses the new Anesthesia structure - returns true if any charge on the bill is
				// an anesthesia charge that uses the anesthesia billing, and has an anesthesia setup that is not a flat fee
				// (meaning if no anesthesia setup exists, the return value is still correct)
				// (j.jones 2008-05-02 10:50) - PLID 28012 - removed the requirement for UseAnesthesiaBilling
				// to be checked for the service, and for a flat-fee
				// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
				&& ReturnsRecordsParam("SELECT ChargesT.ID FROM BillsT "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND BillsT.Deleted = 0 "
				"AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND ServiceT.Anesthesia = 1 "
				"AND ChargesT.BillID = {INT}", pNew->BillID)) {

				pNew->AnesthesiaTimes = TRUE;

				if (AdoFldLong(fields->Item["ShowAnesthMinutesOnly"], -1) == 1) {
					pNew->OnlyAnesthesiaMinutes = TRUE;
				}
				else {
					pNew->OnlyAnesthesiaMinutes = FALSE;
				}
			}
			else {
				pNew->AnesthesiaTimes = FALSE;
				pNew->OnlyAnesthesiaMinutes = FALSE;
			}

			// (j.jones 2008-09-09 10:01) - PLID 18695 - converted NSF Code to InsType
			if (m_FormatStyle != ANSI) {
				pNew->eInsType = (InsuranceTypeCode)AdoFldLong(fields->Item["InsType"], (long)itcInvalid);
			}

			pNew->IsPrimary = AdoFldLong(fields->Item["IsPrimary"], 0);

			// (j.jones 2013-06-20 12:27) - PLID 57245 - moved medicaid resubmission and original ref no logic
			// into the query (if both exist, medicaid resubmission is used) and cached in the pointer
			pNew->strOriginalReferenceNumber = AdoFldString(fields->Item["OriginalReferenceNumber"], "");
			pNew->strOriginalReferenceNumber.TrimLeft(); pNew->strOriginalReferenceNumber.TrimRight();

			// (j.jones 2014-08-22 13:58) - PLID 63446 - tracks the earliest date of service on the claim
			pNew->dtFirstDateOfService = AdoFldDateTime(fields->Item["FirstDateOfService"]);

			// (j.jones 2014-08-25 09:07) - PLID 54213 - track the qualifier to use in loop 2000B SBR01, for the InsuredPartyID of the bill
			pNew->sbr2000B_SBR01 = CalculateSBR01Qualifier(true, pNew->IsPrimary == 1 ? true : false,
				m_actClaimType != actInst ? m_HCFAInfo.ANSI_EnablePaymentInfo : m_UB92Info.ANSI_EnablePaymentInfo,
				AdoFldLong(fields->Item["InsuredPartyPriority"]));

			// (j.jones 2007-02-22 12:45) - PLID 24089 - return an error if a claim doesn't have a provider
			if(pNew->ProviderID == -1) {
				//there is no doctor, so cease loading claims.
				CString str;
				// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
				CString strPatientName = GetExistingPatientName(pNew->PatientID);
				if(!strPatientName.IsEmpty()) {
					// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
					str.Format("Could not open provider information for patient '%s', Bill ID %li. (LoadClaimInfo)",
						strPatientName, pNew->BillID);
				}
				else {
					//serious problems if you get here
					str = "Could not open provider information. (LoadClaimInfo)";
				}

				// (j.jones 2007-05-10 12:07) - PLID 25948 - tweaked the error to be more accurate
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum				
				if((m_actClaimType != actInst && pNew->Box33Setup == 2) || (m_actClaimType == actInst && pNew->Box82Setup == 2))
					str += "\nIt is possible that you have no provider selected in General 1 for this patient.";
				else if(m_actClaimType == actInst && pNew->Box82Setup == 3)
					str += "\nIt is possible that you have no referring physician selected on this patient's bill.";
				else
					str += "\nIt is possible that you have no provider selected on this patient's bill.";
				AfxMessageBox(str);

				//delete the memory we just allocated
				delete pNew;
				return Error_Missing_Info;
			}

			m_aryEBillingInfo.Add(pNew);
			
			rs->MoveNext();

			m_Progress.SetValue(m_Progress.GetValue() + m_ProgIncrement);
		}

		rs->Close();

		m_Progress.SetValue(100.0);

		return Success;

	} NxCatchAll("Error in EBilling::LoadClaimInfo");

	return Error_Other;
}

bool CEbilling::ZipFile()
{
	// (j.jones 2009-06-16 17:34) - PLID 34643 - we now use NxCompress to zip the file
	try {

		if(NxCompressUtils::NxCompressFileToFile(m_ExportName, m_ZipName) != S_OK) {

			LogDetail("The claim file has been exported, but failed to zip up properly.");
			AfxMessageBox("The claim file has been exported, but failed to zip up properly.");

			// Return failure
			return false;
		}

		return true;

	} NxCatchAll("Error in CEbilling::ZipFile");

	return false;
}

// (j.jones 2009-03-16 16:52) - PLID 32321 - added boolean to determine when OHIP
// (j.jones 2011-07-13 10:44) - PLID 44542 - changed the boolean to reflect what it is really used for
// (j.jones 2015-11-19 14:11) - PLID 67600 - removed the parameters, they are now calculated values
void CEbilling::UpdateClaimHistory()
{
	CString str,desc,insco,strNote;
	long Loc, PatID;
	_variant_t var;

	// (j.jones 2015-11-19 14:08) - PLID 67600 - throw all exceptions to the caller
	//try
	{
		// (j.jones 2015-11-19 14:13) - PLID 67600 - the destination text is calculated
		// based on the export style
		CString strDest = "Ebilling Clearinghouse";
		bool bUseGenericClaimNaming = false;
		if (m_FormatStyle == OHIP) {
			strDest = "OHIP";
			bUseGenericClaimNaming = true;
		}
		else if (m_FormatStyle == ALBERTA) {
			strDest = "Alberta HLINK";
			bUseGenericClaimNaming = true;
		}

		SetDlgItemText(IDC_CURR_EVENT,"Updating Claim Histories");

		// (j.jones 2006-04-28 14:30) - PLID 20346 - our m_aryEBillingInfo array
		// is an array of all claims sent, which may separate by provider.
		// but we only need to update the ClaimHistory/MailSent one time per bill.
		CArray<long,long> aryBillIDs;

		for(int i=0;i<m_aryEBillingInfo.GetSize();i++) {
			m_pEBillingInfo = ((EBillingInfo*)m_aryEBillingInfo.GetAt(i));

			//check to see we haven't already updated this bill
			BOOL bFound = FALSE;
			for(int j=0;j<aryBillIDs.GetSize() && !bFound;j++) {
				if(aryBillIDs.GetAt(j) == m_pEBillingInfo->BillID) {
					bFound = TRUE;
				}
			}

			if(bFound) {
				//we already updated the history for this bill
				continue;
			}
			else {
				//we need to remember we've updated the history for this bill
				aryBillIDs.Add(m_pEBillingInfo->BillID);
			}

			//Send Type 0 - Electronic Billing

			// (j.jones 2013-01-23 09:19) - PLID 54734 - moved the claim history addition to its own function,
			// this also updates HCFATrackT.LastSendDate
			AddToClaimHistory(m_pEBillingInfo->BillID, m_pEBillingInfo->InsuredPartyID, ClaimSendType::Electronic, strDest);

			//now add to patient history			
			Loc = GetCurrentLocation();
			PatID = m_pEBillingInfo->PatientID;
			// (j.jones 2008-05-06 11:55) - PLID 27453 - parameterized and converted to use NextRecordset
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			_RecordsetPtr rs = CreateParamRecordset("SELECT Description FROM BillsT WHERE ID = {INT}\r\n"
				"SELECT Name FROM InsuranceCoT WHERE PersonID = {INT}\r\n"
				"SELECT TOP 1 LineItemT.Date FROM LineItemT "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE ChargesT.BillID = {INT} AND Batched = 1 AND Deleted = 0 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"ORDER BY LineItemT.Date ASC",
				m_pEBillingInfo->BillID, m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->BillID);
			
			if(!rs->eof) {
				desc = AdoFldString(rs, "Description","");
			}
			else {
				desc = "";
			}

			rs = rs->NextRecordset(NULL);

			if(!rs->eof) {
				insco = AdoFldString(rs, "Name","");
			}
			else {
				insco = "";
			}

			rs = rs->NextRecordset(NULL);

			CString strDate = "";
			if(!rs->eof) {
				COleDateTime dt;
				dt = AdoFldDateTime(rs, "Date");
				strDate.Format(", Service Date: %s", FormatDateTimeForInterface(dt));
			}
			rs->Close();

			//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB

			// (j.jones 2009-03-16 16:53) - PLID 32321 - added OHIP support
			// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
			CString strClaimType = "HCFA";
			if(m_actClaimType == actInst) {
				strClaimType = "UB";
			}
			// (j.jones 2011-07-13 10:44) - PLID 44542 - changed the boolean to reflect what it is really used for
			if(bUseGenericClaimNaming) {
				strClaimType = "Claim";
			}

			strNote.Format("%s sent to %s - Bill Description: '%s', Insurance Company: %s", strClaimType, strDest,desc,insco);
			strNote += strDate;
			
			// (j.jones 2007-02-20 09:13) - PLID 24790 - converted the insert to include the NewNumber in the batch,
			// then return the new ID so we can send a tablechecker
			// (j.jones 2008-09-04 13:31) - PLID 30288 - converted to use CreateNewMailSentEntry,
			// which creates the data in one batch and sends a tablechecker
			// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
			CreateNewMailSentEntry(PatID, strNote, "", "", GetCurrentUserName(), "", Loc);
		}

		aryBillIDs.RemoveAll();

	}
	// (j.jones 2015-11-19 14:08) - PLID 67600 - throw all exceptions to the caller
	//NxCatchAll("Error updating claim histories.");
}

// (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
CString CEbilling::NormalizeName(CString str)
{
	auto shouldIgnoreChar = [](char c) {
		// all alphanumerics, though i dont know why someone might have a number in their name,
		// turns out that actually does happen -- Jennifer 8. Lee
		if (::isalnum((unsigned char)c)) {
			return false;
		}
		switch (c) {
		case '\'': // apostrophes are fine
		case '-':  // so are hyphens
		case ' ': // spaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaace
			return false;
		}
		return true;
	};

	// quick exit if nothing to escape
	if (0 == std::count_if((const char*)str, (const char*)str + str.GetLength(), shouldIgnoreChar)) {
		return str;
	}
	
	{
		CString::CStrBuf buf(str);

		char* szBegin = buf;
		char* szEnd = szBegin + str.GetLength();

		// std::remove will move anything that matches shouldIgnoreChar to the end of the string
		// and return a pointer to the 'new' end of the string, which we can use to SetLength.
		const char* szJunk = std::remove_if(szBegin, szEnd, shouldIgnoreChar);

		buf.SetLength(szJunk - szBegin);
	}

	return str;
}

CString CEbilling::StripNonNum(CString str)
{
	try {
		CString strbuf = "";
		for(int i=0;i<str.GetLength();i++) {
			if(str.GetAt(i)>='0' && str.GetAt(i)<='9')
				strbuf += str.GetAt(i);
		}
		return strbuf;
	} NxCatchAll("Error in StripNonNum: Data = " + str);
	return str;
}

// (j.jones 2013-05-06 17:48) - PLID 55100 - added function to strip spaces and hyphens only
CString CEbilling::StripSpacesAndHyphens(CString str)
{
	//throw exceptions to the caller (like that would happen)
	CString strNew = str;
	strNew.Replace("-", "");
	strNew.Replace(" ", "");
	return strNew;
}

CString CEbilling::StripPunctuation(CString str, BOOL bReplaceWithSpaces /*= TRUE*/)
{
	try {
		CString strbuf = "";
		for(int i=0;i<str.GetLength();i++) {
			// (j.jones 2007-10-05 14:26) - PLID 25867 - fixed so we won't try to search A to z
			if((str.GetAt(i)>='0' && str.GetAt(i)<='9') || (str.GetAt(i)>='A' && str.GetAt(i)<='Z') || (str.GetAt(i)>='a' && str.GetAt(i)<='z') || (bReplaceWithSpaces && str.GetAt(i)==' '))
				strbuf += str.GetAt(i);
			else {
				if(bReplaceWithSpaces)
					strbuf += " ";
			}
		}
		return strbuf;
	} NxCatchAll("Error in StripPunctuation: Data = " + str);
	return str;
}

CString CEbilling::StripANSIPunctuation(CString str)
{
	try {

		CString strbuf = "";
		for(int i=0;i<str.GetLength();i++) {
			//if we are taking out all punctuation, then take out all non numeric, non alpha chars
			//but we ALWAYS take out asterisks and tildes. ALWAYS.
			// (j.jones 2007-10-05 14:26) - PLID 25867 - fixed so we won't try to search A to z
			if((m_bStripANSIPunctuation && !((str.GetAt(i)>='0' && str.GetAt(i)<='9') || (str.GetAt(i)>='A' && str.GetAt(i)<='Z') || (str.GetAt(i)>='a' && str.GetAt(i)<='z') || str.GetAt(i)==':' || str.GetAt(i)=='.' || str.GetAt(i)==' '))
				|| str.GetAt(i) == '*' || str.GetAt(i) == '~'
				// (j.jones 2010-10-15 11:30) - PLID 32848 - 5010 differences are that ^ and ` are in the always-remove list
				|| (m_avANSIVersion == av5010 && (str.GetAt(i) == '^' || str.GetAt(i) == '`')))

				strbuf += " ";
			else
				strbuf += str.GetAt(i);
		}
		return strbuf;

	} NxCatchAll("Error in StripANSIPunctuation: Data = " + str);
	return str;
}

// (j.jones 2012-05-25 16:22) - PLID 50657 - FormatANSICurrency will first remove the dollar sign
// and commas from the string. Then it will check the m_bTruncateCurrency setting, and if enabled,
// will trim leading and trailing zeros.
CString CEbilling::FormatANSICurrency(const CString strCurrency)
{
	CString strFormattedCurrency = strCurrency;

	//remove $ and , incase they exist (though nobody outside the US should use this, use the international symbols here)
	strFormattedCurrency.Replace(GetCurrencySymbol(),"");
	strFormattedCurrency.Replace(GetThousandsSeparator(),"");

	if(!m_bTruncateCurrency) {
		//truncation is not enabled, return the string we have thus far
		return strFormattedCurrency;
	}

	if(strFormattedCurrency.IsEmpty()) {
		//the field is already blank, so leave now, do not
		//try to change to 0, perhaps the caller wants it blank
		return strFormattedCurrency;
	}

	//trim trailing zeros, including the decimal
	//Example: 12.30 becomes 12.3
	//Example: 12.00 becomes 12

	strFormattedCurrency.TrimRight("0");	//first the zeros
	strFormattedCurrency.TrimRight(".");	//then the decimal, if necessary

	// (j.jones 2012-05-25 16:25) - PLID 50657 - Everything up to this point
	// was pre-existing logic that I just moved to a modular function.
	// Now we're adding the new ability to truncate leading zeroes for
	// values less than 1.00.
	//Example: 0.12 becomes .12
	strFormattedCurrency.TrimLeft("0");

	//finally, we have to return a valid number, and if it is 0, then that is permitted
	//Example: 0.00 becomes 0
	if(strFormattedCurrency.IsEmpty() || strFormattedCurrency == ".") {
		strFormattedCurrency = "0";
	}

	return strFormattedCurrency;
}

CString CEbilling::ParseField(CString data, long Size, char Justify, char FillChar)
{
	CString strData = "";
	long strLen;

	try {

		strData = data;

		strLen = strData.GetLength();
		if(strLen>Size)
			//if the data is bigger than the field, truncate
			strData = strData.Left(Size);
		else if(strLen<Size) {
			//otherwise (and this is more likely), fill with the FillChar
			for(int i=strLen;i<Size;i++) {
				if(Justify=='R') {
					//right justify
					strData = FillChar + strData;
				}
				else {
					//by default, left justify
					strData += FillChar;					
				}
			}
		}

		if(m_bCapitalizeEbilling)
			strData.MakeUpper();

		return strData;

	} NxCatchAll("Error in ParseField: Data = " + data);

	//if it fails, at least return the right size of data
	strData = "";
	for(int z=0;z<Size;z++) {
		strData += FillChar;
	}

	return strData;
}

// (j.jones 2012-10-30 16:49) - PLID 53364 - added override to disable punctuation stripping
CString CEbilling::ParseANSIField(CString data, long Min, long Max,
								  BOOL bForceFill /* = FALSE */, char Justify /* = 'L'*/, char FillChar /* = ' '*/,
								  BOOL bKeepAllPunctuation /*= FALSE*/)
{
	CString strData = "";
	long strLen;

	try {

		strData = data;

		// (j.jones 2012-10-30 16:49) - PLID 53364 - added override to disable punctuation stripping
		if(!bKeepAllPunctuation) {
			strData = StripANSIPunctuation(strData);
		}

		strData.TrimRight(" ");

		strLen = strData.GetLength();
		if(strLen>Max)
			//if the data is bigger than the field, truncate
			strData = strData.Left(Max);
		else if(strLen<Min && (strData != "" || bForceFill)) {
			//otherwise (and this is more likely), fill with the FillChar
			
			//JJ - for ANSI, only fill if the strData is not deliberately blank

			for(int i=strLen;i<Min;i++) {
				if(Justify=='R') {
					//right justify
					strData = FillChar + strData;
				}
				else {
					//by default, left justify
					strData += FillChar;					
				}
			}
		}

		strData = "*" + strData;

		if(m_bCapitalizeEbilling)
			strData.MakeUpper();

		return strData;

	} NxCatchAll("Error in ParseANSIField: Data = " + data);

	//if it fails, just return a *
	strData = "*";

	return strData;
}

/****************************************************************************************************
*
*	HCFA Image
*
*	This function in its entirety will export a HCFA Image.
*	Of the different companies we support that use HCFA Images, they are all nearly identical,
*	but for each minor difference, we will handle it in this function with an if statement.
*	If we support a company whose HCFA Image is more drastically different, it should be in its own function.
*
*	Due to the nature of the HCFA Image, it is much more difficult to break up the HCFA Image function
*	into smaller subsets. Therefore all the code for it is broken up into only three functions.
*	However, each line of the file is separated by a commented line (i.e. Line 1, Line 2, etc.) to
*	keep some semblance of order and to allow for easy readability.
*
*	Each line will be written separately to a file, in order to ensure better organization.
*	The only reason that two or more lines should ever be grouped together is if two differing
*	formats have specific requirements that affect multiple lines, and cannot be broken up.
*
*
****************************************************************************************************/

int CEbilling::ExportToHCFAImage()
{
	try {

		CString OutputString, str;

		SetDlgItemText(IDC_CURR_EVENT,"Exporting to Disk");

		_RecordsetPtr rsBills;

		//open the file for writing
		if(!m_OutputFile.Open(m_ExportName,CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
			// (j.armen 2011-10-25 13:39) - PLID 46134 - EBilling is located in the practice path
			// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure. Also, changed to FileUtils::CreatePath() so that 
			// non-existent intermediate paths are created.
			FileUtils::CreatePath(FileUtils::GetFilePath(m_ExportName));
			if(!m_OutputFile.Open(m_ExportName,CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
				AfxMessageBox("The ebilling export file could not be created. Contact Nextech for assistance.");
				return Error_Other;
			}
		};

		for(int i=0;i<m_aryEBillingInfo.GetSize();i++) {

			OutputString = "";

			str.Format("Exporting to Disk - Claim %li of %li",i+1,m_aryEBillingInfo.GetSize());
			SetDlgItemText(IDC_CURR_EVENT,str);
			
			m_pEBillingInfo = ((EBillingInfo*)m_aryEBillingInfo.GetAt(i));

			m_HCFAInfo.LoadData(m_pEBillingInfo->HCFASetupID);

			TRACE("HCFA ID=%d\r\n", m_pEBillingInfo->HCFAID);
			TRACE("==========================================================\r\n");

			//begin exporting the HCFA image
			
			//since this is not separated by a function for each record,
			//the structure isn't the same as an NSF export. However,
			//we should still write each line to the file separately
			//in order to keep a logical structure

			//JJ - we need to generate multiple claims if there are more than "ChargesPerPage" charges (default is 6)
			//We accomplish this by determining how many pages we will need, then looping that many times.
			//m_nCurrPage will be the member variable that the export code will look at to know whether to print
			//charges 1-6, 7-12, etc.

			long nPages = 1;
			
			_RecordsetPtr rs;

			if(m_pEBillingInfo->Box33Setup == 2)
				//since 2 is the general 1 provider, we do not separate charges at all
				// (j.jones 2008-05-06 12:02) - PLID 27543 - parameterized
				// (j.jones 2011-04-20 11:18) - PLID 43329 - now this returns every charge ID, instead of a count
				// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
				rs = CreateParamRecordset("SELECT ChargesT.ID AS ChargeID "
					"FROM BillsT "
					"LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"WHERE BillsT.ID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					"ORDER BY ChargesT.LineID", m_pEBillingInfo->BillID);
			else {
				//separate the charges per doctor - the bill should already be separated in the loaded claims

				// (j.jones 2006-12-01 15:50) - PLID 22110 - supported the ClaimProviderID
				// (j.jones 2008-04-03 09:23) - PLID 28995 - supported the HCFAClaimProvidersT setup
				// (j.jones 2008-05-06 12:02) - PLID 27543 - parameterized
				// (j.jones 2008-08-01 10:17) - PLID 30917 - HCFAClaimProvidersT is now per insurance company
				// (j.jones 2010-11-09 12:57) - PLID 41389 - supported ChargesT.ClaimProviderID
				// (j.jones 2011-04-20 11:18) - PLID 43329 - now this returns every charge ID, instead of a count
				// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
				rs = CreateParamRecordset("SELECT ChargesT.ID AS ChargeID "
					"FROM BillsT "
					"LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"WHERE BillsT.ID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					"AND (ChargesT.ClaimProviderID = {INT} "
					"	OR (ChargesT.ClaimProviderID Is Null AND ChargesT.DoctorsProviders IN (SELECT PersonID FROM "
					"		(SELECT ProvidersT.PersonID, "
					"		(CASE WHEN Box33_ProviderID Is Null THEN ProvidersT.ClaimProviderID ELSE Box33_ProviderID END) AS ProviderIDToUse "
					"		FROM ProvidersT "
					"		LEFT JOIN (SELECT * FROM HCFAClaimProvidersT WHERE InsuranceCoID = {INT}) AS HCFAClaimProvidersT ON ProvidersT.PersonID = HCFAClaimProvidersT.ProviderID) SubQ "
					"		WHERE ProviderIDToUse = {INT}))) "
					"ORDER BY ChargesT.LineID",
					m_pEBillingInfo->BillID, m_pEBillingInfo->ProviderID, m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);
			}

			if(!rs->eof) {
				int ChargesPerPage = GetRemotePropertyInt("HCFAImageChargesPerPage", 6, 0, "<None>", true);
				if(ChargesPerPage == 0)
					ChargesPerPage = 1;

				// (j.jones 2011-04-20 11:18) - PLID 43329 - this is now the record count
				long count = rs->GetRecordCount();
				//the count of pages is the amount of
				nPages = count / ChargesPerPage;
				if((count % ChargesPerPage) > 0)
					nPages++;
			}
			
			// (j.jones 2011-04-20 11:18) - PLID 43329 - now we track the charges to export in an array
			// so we do not have to calculate the IDs twice

			CArray<long, long> aryChargesToExport;
			
			while(!rs->eof) {

				aryChargesToExport.Add(VarLong(rs->Fields->Item["ChargeID"]->Value));

				rs->MoveNext();
			}
			rs->Close();

			// (a.walling 2014-03-05 16:29) - PLID 61202 - GetDiagnosisCodesForHCFA - Pass bUseICD10 as appropriate for this InsuredParty and BillID
			bool bUseICD10 = ShouldUseICD10();

			// (a.walling 2014-03-19 10:05) - PLID 61346 - Load all diag codes for the bill
			m_pEBillingInfo->billDiags = GetBillDiagCodes(m_pEBillingInfo->BillID, bUseICD10, ShouldSkipUnlinkedDiags(), &aryChargesToExport);

			for(m_nCurrPage = 1;m_nCurrPage <= nPages;m_nCurrPage++) {

				//JJ - I had to break it up into three functions, because this function was too long to compile!

				// (j.jones 2006-11-09 16:32) - PLID 22668 - we support the legacy, pre-NPI
				// HCFA Image by calling Old_ versions of these functions
				// (j.jones 2007-05-15 09:04) - PLID 25953 - supported yet a newer version of the HCFA
				int nEnableLegacyHCFAImage = GetRemotePropertyInt("EnableLegacyHCFAImage",2,0,"<None>",true);
				if(nEnableLegacyHCFAImage == 0) {
					//use the "intermediate" HCFA - the old one with appended NPI numbers
					RETURN_ON_FAIL(IntermediateNPI_HCFABoxes1to11(rsBills));
					RETURN_ON_FAIL(IntermediateNPI_HCFABoxes12to23(rsBills));
					RETURN_ON_FAIL(IntermediateNPI_HCFABoxes24to33(rsBills, m_nCurrPage, nPages));
				}
				else if(nEnableLegacyHCFAImage == 1) {
					//use the old HCFA - pre-NPI
					RETURN_ON_FAIL(Legacy_HCFABoxes1to11(rsBills));
					RETURN_ON_FAIL(Legacy_HCFABoxes12to23(rsBills));
					RETURN_ON_FAIL(Legacy_HCFABoxes24to33(rsBills, m_nCurrPage, nPages));
				}
				else {
					//use the modern HCFA with NPI numbers in correct places
					
					// (j.jones 2011-04-20 11:18) - PLID 43329 - pass in our list of charge IDs
					RETURN_ON_FAIL(HCFABoxes1to11(rsBills, aryChargesToExport));

					// (j.jones 2011-04-20 11:18) - PLID 43329 - pass in our info. on skipping diag. codes
					RETURN_ON_FAIL(HCFABoxes12to23(rsBills));

					// (j.jones 2011-04-20 11:18) - PLID 43329 - pass in our info. on skipping diag. indexes
					RETURN_ON_FAIL(HCFABoxes24to33(rsBills, m_nCurrPage, nPages));
				}
			}

			//JJ - because of fun rounding and division and lovely math all around, it is possible for
			//us to increment too high on the last claim (ex. 9 claims would increase it to 200.0001).
			//If you aren't sending to Envoy, this will be greater than the max and therefore cause
			//an error. This code is a safeguard against going past 200. And since we will always go up to
			//200 even if we are sending claims, we will still adjust the code here regardless.
			if(m_Progress.GetValue()+m_ProgIncrement > 200.0)
				m_Progress.SetValue(200.0);
			else
				m_Progress.SetValue(m_Progress.GetValue() + m_ProgIncrement);

		} //end of loop for m_aryEBillingInfo

		//Last Line //////////////////////////////////////////////////
		OutputString = "";

		str = "--------------LAST CLAIM-----------------------";
		OutputString += ParseField(str,60,'L',' ');	

		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Last Line ///////////////////////////////////////////

		m_OutputFile.Close();

		m_Progress.SetValue(m_Progress.GetMax());
		
		// (j.jones 2015-11-19 14:10) - PLID 67600 - updating the claim history is now called before the export
		
		str = "All claims were successfully exported and are ready to send to your Ebilling Clearinghouse.\nDo you want to transfer these claims to the unselected list?";
		if (MessageBox(str,"Electronic Billing",MB_YESNO|MB_ICONINFORMATION) == IDYES) {
			// (j.jones 2008-05-06 16:53) - PLID 27453 - parameterized
			// (j.jones 2014-07-09 10:26) - PLID 62568 - do not move on hold claims
			ExecuteParamSql("UPDATE HCFATrackT SET CurrentSelect = 0 WHERE Batch = 2 AND BillID IN ("
				"SELECT BillsT.ID FROM BillsT "
				"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
				"WHERE Coalesce(BillStatusT.Type, -1) != {CONST_INT} "
				")", EBillStatusType::OnHold);
		}

		EndDialog(IDOK);

		return Success;

	} NxCatchAll("Error in Ebilling::ExportToHCFAImage");
	
	return Error_Other;
}

// (j.jones 2011-04-20 11:18) - PLID 43329 - this function now takes in a list of Charge IDs to export,
// so we don't have to calculate which charges to export twice
int CEbilling::HCFABoxes1to11(_RecordsetPtr &rsBills, IN CArray<long, long> &aryChargesToExport)

{
	//this will export the insurance address, and all fields from box 1 to box 11, HCFA Image format

	try {
		//Line 1   ///////////////////////////////////////////
		CString str, OutputString = "";		

		// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name, Address1, Address2, City, State, Zip, IDForInsurance, "
						"CASE WHEN InsurancePlansT.PlanType='Champus' THEN 3 WHEN InsurancePlansT.PlanType='Champva' THEN 4 WHEN InsurancePlansT.PlanType='FECA Black Lung' THEN 5 WHEN InsurancePlansT.PlanType='Group Health Plan' THEN 6 WHEN InsurancePlansT.PlanType='Medicaid' THEN 2 WHEN InsurancePlansT.PlanType='Medicare' THEN 1 ELSE 7 END AS InsType "
						"FROM InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
						"INNER JOIN PersonT ON InsuranceCoT.PersonID = PersonT.ID LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
						"WHERE InsuredPartyT.PersonID = {INT}",m_pEBillingInfo->InsuredPartyID);
		if(rs->eof) {
			//if the recordset is empty, there is no insurance company. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open primary insurance information from this patient's bill for '%s', Bill ID %li. (HCFABoxes1to11 1)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open primary insurance information from this patient's bill. (HCFABoxes1to11 1)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		_variant_t var;


		//check the secondary ins address as well
		// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
		// (j.jones 2010-08-31 17:02) - PLID 40303 - added TPLCode
		_RecordsetPtr rsOtherIns = CreateParamRecordset("SELECT Name, Address1, Address2, City, State, Zip, IDForInsurance, HCFASetupGroupID, InsuranceCoT.TPLCode, "
						"CASE WHEN InsurancePlansT.PlanType='Champus' THEN 3 WHEN InsurancePlansT.PlanType='Champva' THEN 4 WHEN InsurancePlansT.PlanType='FECA Black Lung' THEN 5 WHEN InsurancePlansT.PlanType='Group Health Plan' THEN 6 WHEN InsurancePlansT.PlanType='Medicaid' THEN 2 WHEN InsurancePlansT.PlanType='Medicare' THEN 1 ELSE 7 END AS InsType "
						"FROM InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
						"INNER JOIN PersonT ON InsuranceCoT.PersonID = PersonT.ID LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
						"WHERE InsuredPartyT.PersonID = {INT}",m_pEBillingInfo->OthrInsuredPartyID);

		OutputString += ParseField("",2,'L',' ');
		
		str = "";

		//use the secondary ins. name, if requested and available
		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "Name","");
		}

		OutputString += ParseField(str,40,'L',' ');
		OutputString += ParseField("",10,'L',' ');


		//InsuranceCoT.Name
		var = rs->Fields->Item["Name"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,50,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 1   ///////////////////////////////////

		//Line 2   //////////////////////////////////////////
		OutputString = "";
		
		OutputString += ParseField("",2,'L',' ');
		
		str = "";

		//use the secondary ins. address, if requested and available
		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "Address1","");
		}

		OutputString += ParseField(str,40,'L',' ');
		OutputString += ParseField("",10,'L',' ');
		
		//PersonT.Address1
		var = rs->Fields->Item["Address1"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 2   ///////////////////////////////////

		//Line 3   //////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",2,'L',' ');
		
		str = "";

		//use the secondary ins. address, if requested and available
		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "Address2","");
		}

		OutputString += ParseField(str,40,'L',' ');
		OutputString += ParseField("",10,'L',' ');
		
		//PersonT.Address2
		var = rs->Fields->Item["Address2"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 3   ///////////////////////////////////

		//Line 4   //////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",2,'L',' ');
		
		str = "";

		//use the secondary ins. address, if requested and available
		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "City","");
		}
		OutputString += ParseField(str,18,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "State","");
		}
		OutputString += ParseField(str,2,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "Zip","");
		}
		OutputString += ParseField(str,10,'L',' ');

		OutputString += ParseField("",18,'L',' ');
		
		//PersonT.City
		var = rs->Fields->Item["City"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,18,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.State
		var = rs->Fields->Item["State"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,2,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.Zip
		var = rs->Fields->Item["Zip"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 4   ///////////////////////////////////

		//Line 5 (double)  //////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",2,'L',' ');

		str = "";

		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1 && m_HCFAInfo.ShowSecPINNumber == 1) {
			str = Box33Pin(TRUE);
		}
		OutputString += ParseField(str,20,'L',' ');

		str = "";

		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1 && m_HCFAInfo.ShowSec24JNumber == 1) {
			//InsuranceBox24J.Box24JNumber
			// (j.jones 2008-04-03 10:12) - PLID 28995 - use the Box24J_ProviderID here for the current HCFA group,
			// even though the actual ID will pull from the secondary insurance
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Box24JNumber FROM InsuranceBox24J INNER JOIN InsuredPartyT ON InsuranceBox24J.InsCoID = InsuredPartyT.InsuranceCoID "
				"WHERE InsuredPartyT.PersonID = {INT} AND ProviderID = {INT}",
				m_pEBillingInfo->OthrInsuredPartyID,m_pEBillingInfo->Box24J_ProviderID);

			if(!rs->eof && rs->Fields->Item["Box24JNumber"]->Value.vt==VT_BSTR) {
				str = CString(rs->Fields->Item["Box24JNumber"]->Value.bstrVal);
			}
			else
				str = "";

			//check for the override		
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			_RecordsetPtr rsTemp = CreateParamRecordset("SELECT Box24J FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->Box24J_ProviderID,m_pEBillingInfo->BillLocation,AdoFldLong(rsOtherIns, "HCFASetupGroupID",-1));
			if(!rsTemp->eof) {
				var = rsTemp->Fields->Item["Box24J"]->Value;
				if(var.vt == VT_BSTR) {
					CString strTemp = CString(var.bstrVal);
					strTemp.TrimLeft();
					strTemp.TrimRight();
					if(!strTemp.IsEmpty())
						str = strTemp;
				}
			}
			rsTemp->Close();
		}
		OutputString += ParseField(str,15,'L',' ');

		// (j.jones 2009-08-04 12:32) - PLID 14573 - used payer ID, though
		// we kept the same preference name internally
		if(GetRemotePropertyInt("HCFAImageShowTHINNumber",0,0,"<None>",true) == 1) {
			//show EbillingID number
			OutputString += ParseField("",15,'L',' ');
			OutputString += ParseField("ID#   ",6,'L',' ');
		
			//InsuranceCoT.HCFAPayerID
			// (j.jones 2009-08-05 10:20) - PLID 34467 - this is now in the loaded claim info
			// (j.jones 2009-12-16 15:54) - PLID 36237 - split out into HCFA and UB payer IDs
			OutputString += ParseField(m_pEBillingInfo->strHCFAPayerID,32,'L',' ');
		}
		
		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 5  //////////////////////////////

		//Line 6 (double)  //////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",2,'L',' ');	

		//based on InsurancePlansT.PlanType
		var = rs->Fields->Item["InsType"]->Value;
		if(var.vt==VT_I4) {
			int InsType = var.lVal;
			switch(InsType) {
			case 1:
				str = "X";
				break;
			case 2:
				str = "         X";
				break;
			case 3:
				str = "                  X";
				break;
			case 4:
				str = "                           X";
				break;
			case 5:
				str = "                                    X";
				break;
			case 6:
				str = "                                             X";
				break;
			case 7:
				str = "                                                      X";
				break;
			}
		}
		else
			str = "";
		OutputString += ParseField(str,57,'L',' ');

		//InsuredPartyT.IDForInsurance
		var = rs->Fields->Item["IDForInsurance"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');	

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 6   ///////////////////////////////////

		// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
		_RecordsetPtr rsPatient = CreateParamRecordset("SELECT * FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE PatientsT.PersonID = {INT}",m_pEBillingInfo->PatientID);

		if(rsPatient->eof) {
			//if the recordset is empty, there is no patient. So halt everything!!!
			rsPatient->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open patient information for '%s', Bill ID %li. (HCFABoxes1to11)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open patient information. (HCFABoxes1to11)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		//Line 7 (double)   //////////////////////////////////////////
		OutputString = "";

		//Patient Information

		//PersonT.Last
		var = rsPatient->Fields->Item["Last"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,16,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.First
		var = rsPatient->Fields->Item["First"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,13,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.Middle
		var = rsPatient->Fields->Item["Middle"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,1,'L',' ');
		OutputString += ParseField("",4,'L',' ');

		//PersonT.BirthDate
		//Box3
		var = rsPatient->Fields->Item["BirthDate"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			
			if(m_HCFAInfo.Box3E == 1)
				str = dt.Format("%Y%m%d");
			else
				str = dt.Format("%m%d%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",3,'L',' ');

		//PersonT.Gender
		var = rsPatient->Fields->Item["Gender"]->Value;
		int Gender = VarByte(var,0);
		if(Gender==1)
			str = "X";
		else if(Gender==2)
			str = "      X";
		else
			str = "";

		OutputString += ParseField(str,10,'L',' ');

		// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
		_RecordsetPtr rsIns = CreateParamRecordset("SELECT * FROM PersonT INNER JOIN InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID WHERE ID = {INT}",m_pEBillingInfo->InsuredPartyID);
		if(rsIns->eof) {
			//if the recordset is empty, there is no insured party. So halt everything!!!
			rsIns->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open primary insurance information from this patient's bill for '%s', Bill ID %li. (HCFABoxes1to11 2)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open primary insurance information from this patient's bill. (HCFABoxes1to11 2)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		//Insured Party Information

		//PersonT.Last
		if(m_HCFAInfo.HideBox4 == 0) {
			var = rsIns->Fields->Item["Last"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,16,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.First
		if(m_HCFAInfo.HideBox4 == 0) {
			var = rsIns->Fields->Item["First"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else 
			str = "";
		OutputString += ParseField(str,13,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.Middle
		if(m_HCFAInfo.HideBox4 == 0) {
			var = rsIns->Fields->Item["Middle"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else 
			str = "";
		OutputString += ParseField(str,1,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 7   ///////////////////////////////////

		//Line 8 (double)   //////////////////////////////////////////
		OutputString = "";

		//Patient Information

		//PersonT.Address1
		var = rsPatient->Fields->Item["Address1"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');
		OutputString += ParseField("",4,'L',' ');

		//InsuredPartyT.RelationToPatient
		var = rsIns->Fields->Item["RelationToPatient"]->Value;
		if(var.vt==VT_BSTR) {
			str = CString(var.bstrVal);
			if(str=="Self")
				str="X";
			else if(str=="Spouse")
				str="       X";
			else if(str=="Child")
				str="             X";
			else
				str="                   X";
		}
		else
			str = "                   X";
		OutputString += ParseField(str,23,'L',' ');

		//Insurance Information

		//PersonT.Address1
		if(m_HCFAInfo.HideBox7 == 0) {
			var = rsIns->Fields->Item["Address1"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 8   ///////////////////////////////////


		//Line 9 (double)  /////////////////////////////////////////
		OutputString = "";
		
		//Patient Information

		//PersonT.City
		var = rsPatient->Fields->Item["City"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,28,'L',' ');
		OutputString += ParseField("",3,'L',' ');

		//PersonT.State
		var = rsPatient->Fields->Item["State"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,2,'L',' ');
		OutputString += ParseField("",7,'L',' ');

		//PatientsT.MaritalStatus
		var = rsPatient->Fields->Item["MaritalStatus"]->Value;
		if(var.vt==VT_BSTR) {
			str = CString(var.bstrVal);
			if(str=="1")
				str="X";
			else if(str=="2")
				str="        X";
			else
				str="                X";
		}
		else
			str = "                X";
		OutputString += ParseField(str,19,'L',' ');

		//Insurance Information

		//PersonT.City
		if(m_HCFAInfo.HideBox7 == 0) {
			var = rsIns->Fields->Item["City"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,18,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		//PersonT.State
		if(m_HCFAInfo.HideBox7 == 0) {
			var = rsIns->Fields->Item["State"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else 
			str = "";
		OutputString += ParseField(str,2,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 9   //////////////////////////////////

		//Line 10 (double)   /////////////////////////////////////////
		OutputString = "";
		
		//Patient Information

		//PersonT.Zip
		var = rsPatient->Fields->Item["Zip"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,15,'L',' ');
		OutputString += ParseField("",3,'L',' ');

		//PersonT.HomePhone
		var = rsPatient->Fields->Item["HomePhone"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str = StripNonNum(str);
		OutputString += ParseField(str,15,'L',' ');
		OutputString += ParseField("",7,'L',' ');

		//PatientsT.Employment
		var = rsPatient->Fields->Item["Employment"]->Value;
		if(var.vt==VT_I4) {
			long emp = var.lVal;
			if(emp==1)
				str="X";
			else if(emp==2)
				str="        X";
			else if(emp==3)
				str="               X";
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,19,'L',' ');

		//Insurance Information

		//PersonT.Zip
		if(m_HCFAInfo.HideBox7 == 0) {
			var = rsIns->Fields->Item["Zip"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,16,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		//PersonT.HomePhone
		if(m_HCFAInfo.HideBox7 == 0) {
			var = rsIns->Fields->Item["HomePhone"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		str = StripNonNum(str);
		OutputString += ParseField(str,15,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 10   //////////////////////////////////


		//Line 11 (double)   /////////////////////////////////////////
		OutputString = "";
		
		_RecordsetPtr rsOthrIns;

		if(m_pEBillingInfo->OthrInsuredPartyID!=-1) {
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			rsOthrIns = CreateParamRecordset("SELECT * FROM PersonT INNER JOIN InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID WHERE ID = {INT}",m_pEBillingInfo->OthrInsuredPartyID);
			if(rsOthrIns->eof) {
				//if the recordset is empty, there is no insured party. So halt everything!!!
				rsOthrIns->Close();
				
				// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
				CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
				if(!strPatientName.IsEmpty()) {
					// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
					str.Format("Could not open secondary insurance information from this patient's bill for '%s', Bill ID %li. (HCFABoxes1to11)", strPatientName, m_pEBillingInfo->BillID);
				}
				else
					//serious problems if you get here
					str = "Could not open secondary insurance information from this patient's bill. (HCFABoxes1to11)";

				AfxMessageBox(str);
				return Error_Missing_Info;
			}
		}

		//Other Insurance Information

		//PersonT.Last
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1 && rsOthrIns->Fields->Item["Last"]->Value.vt==VT_BSTR)
			str = CString(rsOthrIns->Fields->Item["Last"]->Value.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,16,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.First
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1 && rsOthrIns->Fields->Item["First"]->Value.vt==VT_BSTR)
			str = CString(rsOthrIns->Fields->Item["First"]->Value.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,13,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.Middle
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1 && rsOthrIns->Fields->Item["Middle"]->Value.vt==VT_BSTR)
			str = CString(rsOthrIns->Fields->Item["Middle"]->Value.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,1,'L',' ');
		OutputString += ParseField("",27,'L',' ');

		//InsuredPartyT.PolicyGroupNum (Primary Ins.)

		str = "";

		// (j.jones 2006-10-31 15:49) - PLID 21845 - check the secondary insurance's HCFA group,
		// and see if we need to use the secondary insurance's information in Box 11. If so,
		// this will override the Hide Box 11 option
		BOOL bShowSecondaryInBox11 = FALSE;
		if(m_pEBillingInfo->OthrInsuredPartyID != -1) {
			if(ReturnsRecords("SELECT ID FROM HCFASetupT WHERE SecondaryFillBox11 = 1 AND "
				"ID IN (SELECT HCFASetupGroupID FROM InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
				"WHERE InsuredPartyT.PersonID = %li)", m_pEBillingInfo->OthrInsuredPartyID)) {
				//there is a secondary insurance and its HCFA group says to overwrite Box 11
				bShowSecondaryInBox11 = TRUE;
			}
		}

		if(bShowSecondaryInBox11) {

			//if we are showing the secondary information in Box 11, that takes
			//precedence over all other Box 11 possibilities
			str = AdoFldString(rsOthrIns, "IDForInsurance", "");
		}
		else {

			//check for the HCFA group's override
  			BOOL bOverride = FALSE;

			// (j.jones 2006-03-13 14:44) - PLID 19555 - the Box 11 default overrides
			// the "Hide Box 11" options. If they want Box 11 to be truly blank, then they
			// should have it hidden and have no override.
			if(m_HCFAInfo.Box11Rule == 1) {
				bOverride = TRUE;
			}
			else {			
				//use only if blank (default)
				var = rsIns->Fields->Item["PolicyGroupNum"]->Value;
				if(var.vt == VT_BSTR) {
					str = CString(var.bstrVal);
					str.TrimLeft(" ");
					str.TrimRight(" ");
					if(str.GetLength()==0)
						bOverride = TRUE;
				}
				else
					bOverride = TRUE;
			}

			if(bOverride) {
				str = m_HCFAInfo.Box11;
			}
			else if(m_HCFAInfo.HideBox11 == 1) {
				str = "";
			}
		}

		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 11   //////////////////////////////////

		//Line 12 (double)   /////////////////////////////////////////
		OutputString = "";

		//Other Insurance Information

		//InsuredPartyT.IDForInsurance
		if(m_HCFAInfo.Use1aIn9a && (m_pEBillingInfo->OthrInsuredPartyID != -1 || m_HCFAInfo.Use1aIn9aAlways)) {
			//InsuredPartyT.IDForInsurance
			var = rs->Fields->Item["IDForInsurance"]->Value;
			str = VarString(var, "");
		}
		// (j.jones 2010-08-31 16:54) - PLID 40303 - supported TPL in 9a, which is mutually exclusive of Use1aIn9a
		else if (!m_HCFAInfo.Use1aIn9a && m_HCFAInfo.TPLIn9a == 1 && m_pEBillingInfo->OthrInsuredPartyID != -1) {
			str = AdoFldString(rsOtherIns, "TPLCode", "");
		}
		else {
			if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID != -1 && rsOthrIns->Fields->Item["IDForInsurance"]->Value.vt==VT_BSTR)
				str = CString(rsOthrIns->Fields->Item["IDForInsurance"]->Value.bstrVal);
			else
				str = "";
		}
		OutputString += ParseField(str,32,'L',' ');
		OutputString += ParseField("",8,'L',' ');

		rs->Close();

		if(m_pEBillingInfo->Box33Setup == 2) {
			//since 2 is the general 1 provider, we do not separate charges at all
			// (j.jones 2007-10-05 10:50) - PLID 27659 - added ChargeID as a field
			// (j.jones 2007-10-15 15:03) - PLID 27757 - added ServiceID as a field
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			// (j.jones 2008-05-28 15:09) - PLID 30177 - added NDCCode
			// (j.jones 2009-03-09 09:10) - PLID 33354 - added insured party accident information
			// (j.jones 2011-04-20 11:18) - PLID 43329 - just filter on our passed-in list of charge IDs
			// (j.gruber 2014-03-17 11:09) - PLID 61395 - support new billing structure
			// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - DiagCodes and WhichCodes must be unique and filtered for NULLs. Use ChargeWhichICD9CodesIF/10 and the BillDiags
			// (j.jones 2016-04-11 17:22) - NX-100149 - Box15Date is now calculated from ConditionDateType
			rsBills = CreateParamRecordset("SELECT GetDate() AS TodaysDate, ChargesT.ID AS ChargeID, ChargesT.ServiceID, BillsT.*, ChargesT.*, LineItemT.*, dbo.GetChargeTotal(ChargesT.ID) AS ChargeTot, "
									"PlaceOfServiceCodesT.PlaceCodes, "
									"ServiceT.Anesthesia, ServiceT.UseAnesthesiaBilling, ChargesT.NDCCode, InsuredPartyT.AccidentType, InsuredPartyT.AccidentState, InsuredPartyT.DateOfCurAcc, ChargeWhichCodesQ.WhichCodes as ChargeWhichCodes, "
									"CASE BillsT.ConditionDateType "
									"WHEN {CONST_INT} THEN BillsT.FirstVisitOrConsultationDate "
									"WHEN {CONST_INT} THEN BillsT.InitialTreatmentDate "
									"WHEN {CONST_INT} THEN BillsT.LastSeenDate "
									"WHEN {CONST_INT} THEN BillsT.AcuteManifestationDate "
									"WHEN {CONST_INT} THEN BillsT.LastXRayDate "
									"WHEN {CONST_INT} THEN BillsT.HearingAndPrescriptionDate "
									"WHEN {CONST_INT} THEN BillsT.AssumedCareDate "
									"WHEN {CONST_INT} THEN BillsT.RelinquishedCareDate "
									"WHEN {CONST_INT} THEN BillsT.AccidentDate "
									"ELSE NULL END AS Box15Date "
									"FROM BillsT LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
									"INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
									"LEFT JOIN CPTModifierT ON CPTModifier = CPTModifierT.Number "
									"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
									"OUTER APPLY dbo.{CONST_STRING}({STRING}, ChargesT.ID) ChargeWhichCodesQ "
									"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
									"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
									"WHERE ChargesT.ID IN ({INTARRAY}) "
									"ORDER BY LineID",
									ConditionDateType::cdtFirstVisitOrConsultation444,
									ConditionDateType::cdtInitialTreatmentDate454,
									ConditionDateType::cdtLastSeenDate304,
									ConditionDateType::cdtAcuteManifestation453,
									ConditionDateType::cdtLastXray455,
									ConditionDateType::cdtHearingAndPrescription471,
									ConditionDateType::cdtAssumedCare090,
									ConditionDateType::cdtRelinquishedCare91,
									ConditionDateType::cdtAccident439
									, ShouldUseICD10() ? "ChargeWhichICD10CodesIF" : "ChargeWhichICD9CodesIF"
									, MakeBillDiagsXml(m_pEBillingInfo->billDiags)
									, aryChargesToExport);
		}
		else {
			//separate the charges per doctor - the bill should already be separated in the loaded claims

			// (j.jones 2006-12-01 15:50) - PLID 22110 - supported the ClaimProviderID
			// (j.jones 2007-10-05 10:50) - PLID 27659 - added ChargeID as a field
			// (j.jones 2007-10-15 15:03) - PLID 27757 - added ServiceID as a field
			// (j.jones 2008-04-03 09:23) - PLID 28995 - supported the HCFAClaimProvidersT setup
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			// (j.jones 2008-05-28 15:09) - PLID 30177 - added NDCCode
			// (j.jones 2008-08-01 10:17) - PLID 30917 - HCFAClaimProvidersT is now per insurance company
			// (j.jones 2009-03-09 09:10) - PLID 33354 - added insured party accident information
			// (j.jones 2010-11-09 12:57) - PLID 41389 - supported ChargesT.ClaimProviderID
			// (j.jones 2011-04-20 11:18) - PLID 43329 - just filter on our passed-in list of charge IDs
			// (j.gruber 2014-03-17 11:11) - PLID 61395 - support new billing structure
			// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - DiagCodes and WhichCodes must be unique and filtered for NULLs. Use ChargeWhichICD9CodesIF/10 and the BillDiags
			// (j.jones 2016-04-11 17:22) - NX-100149 - Box15Date is now calculated from ConditionDateType
			rsBills = CreateParamRecordset("SELECT GetDate() AS TodaysDate, ChargesT.ID AS ChargeID, ChargesT.ServiceID, BillsT.*, ChargesT.*, LineItemT.*, dbo.GetChargeTotal(ChargesT.ID) AS ChargeTot, "
									"PlaceOfServiceCodesT.PlaceCodes, "
									"ServiceT.Anesthesia, ServiceT.UseAnesthesiaBilling, ChargesT.NDCCode, InsuredPartyT.AccidentType, InsuredPartyT.AccidentState, InsuredPartyT.DateOfCurAcc, "
									"ChargeWhichCodesQ.WhichCodes as ChargeWhichCodes, "
									"CASE BillsT.ConditionDateType "
									"WHEN {CONST_INT} THEN BillsT.FirstVisitOrConsultationDate "
									"WHEN {CONST_INT} THEN BillsT.InitialTreatmentDate "
									"WHEN {CONST_INT} THEN BillsT.LastSeenDate "
									"WHEN {CONST_INT} THEN BillsT.AcuteManifestationDate "
									"WHEN {CONST_INT} THEN BillsT.LastXRayDate "
									"WHEN {CONST_INT} THEN BillsT.HearingAndPrescriptionDate "
									"WHEN {CONST_INT} THEN BillsT.AssumedCareDate "
									"WHEN {CONST_INT} THEN BillsT.RelinquishedCareDate "
									"WHEN {CONST_INT} THEN BillsT.AccidentDate "
									"ELSE NULL END AS Box15Date "
									"FROM BillsT LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
									"INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
									"LEFT JOIN CPTModifierT ON CPTModifier = CPTModifierT.Number "
									"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
									"OUTER APPLY dbo.{CONST_STRING}({STRING}, ChargesT.ID) ChargeWhichCodesQ "
									"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
									"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
									"WHERE ChargesT.ID IN ({INTARRAY}) "
									"ORDER BY LineID ",
									ConditionDateType::cdtFirstVisitOrConsultation444,
									ConditionDateType::cdtInitialTreatmentDate454,
									ConditionDateType::cdtLastSeenDate304,
									ConditionDateType::cdtAcuteManifestation453,
									ConditionDateType::cdtLastXray455,
									ConditionDateType::cdtHearingAndPrescription471,
									ConditionDateType::cdtAssumedCare090,
									ConditionDateType::cdtRelinquishedCare91,
									ConditionDateType::cdtAccident439
									, ShouldUseICD10() ? "ChargeWhichICD10CodesIF" : "ChargeWhichICD9CodesIF"
									, MakeBillDiagsXml(m_pEBillingInfo->billDiags)
									, aryChargesToExport);
		}

		if(rsBills->eof) {
			//if the recordset is empty, there is no bill information. So halt everything!!!
			rsBills->Close();
			
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open bill information for '%s', Bill ID %li. (HCFABoxes1to11)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open bill information. (HCFABoxes1to11)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		// (j.jones 2009-03-09 09:11) - PLID 33354 - we now may use the insured party accident type instead
		InsuredPartyAccidentType ipatAccType = (InsuredPartyAccidentType)AdoFldLong(rsBills, "AccidentType", (long)ipatNone);		

		BOOL bRelatedToEmp = FALSE;
		
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//pull from the insured party, not the bill
			bRelatedToEmp = (ipatAccType == ipatEmployment);
		}
		else {
			//pull from BillsT.RelatedToEmp
			bRelatedToEmp = AdoFldBool(rsBills, "RelatedToEmp", FALSE);
		}

		if(bRelatedToEmp) {
			str = "X";
		}
		else {
			str = "        X";
		}
		OutputString += ParseField(str,21,'L',' ');

		//Insurance Information

		str = "";
		
		//PersonT.BirthDate
		if(bShowSecondaryInBox11) {

			// (j.jones 2006-10-31 15:49) - PLID 21845 - if we are showing the secondary
			//information in Box 11, that takes precedence over all other Box 11 possibilities
			var = rsOthrIns->Fields->Item["BirthDate"]->Value;
			if(var.vt==VT_DATE) {
				COleDateTime dt;
				dt = var.date;

				if(m_HCFAInfo.Box11aE == 1)
					str = dt.Format("%Y%m%d");
				else
					str = dt.Format("%m%d%Y");
			}
			else
				str = "";
		}
		else {

			//Box11a

			// (j.jones 2007-04-09 17:29) - PLID 25537 - supported hiding Box11a Birthdate individually
			if(m_HCFAInfo.HideBox11a_Birthdate == 0) {
				var = rsIns->Fields->Item["BirthDate"]->Value;
				if(var.vt==VT_DATE) {
					COleDateTime dt;
					dt = var.date;

					if(m_HCFAInfo.Box11aE == 1)
						str = dt.Format("%Y%m%d");
					else
						str = dt.Format("%m%d%Y");
				}
				else
					str = "";
			}
			else
				str = "";
		}
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",9,'L',' ');

		//PersonT.Gender
		if(bShowSecondaryInBox11) {

			// (j.jones 2006-10-31 15:49) - PLID 21845 - if we are showing the secondary
			//information in Box 11, that takes precedence over all other Box 11 possibilities
			var = rsOthrIns->Fields->Item["Gender"]->Value;				
			Gender = VarByte(var,0);
			if(Gender==1)
				str = "X";
			else if(Gender==2)
				str = "        X";
			else
				str = "";
		}
		else {
			// (j.jones 2007-04-09 17:29) - PLID 25537 - supported hiding Box11a Gender individually
			if(m_HCFAInfo.HideBox11a_Gender == 0) {
				var = rsIns->Fields->Item["Gender"]->Value;
				
				Gender = VarByte(var,0);
				if(Gender==1)
					str = "X";
				else if(Gender==2)
					str = "        X";
				else
					str = "";
			}
			else
				str = "";
		}
		OutputString += ParseField(str,9,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 12   //////////////////////////////////

		//Line 13 (double)   /////////////////////////////////////////
		OutputString = "";

		//Other Insurance Information

		//PersonT.BirthDate
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1 && rsOthrIns->Fields->Item["BirthDate"]->Value.vt==VT_DATE) {
			COleDateTime dt;
			dt = rsOthrIns->Fields->Item["BirthDate"]->Value.date;
			
			if(m_HCFAInfo.Box9bE == 1)
				str = dt.Format("%Y%m%d");
			else
				str = dt.Format("%m%d%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",9,'L',' ');

		//PersonT.Gender
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1) {
			int Gender = AdoFldByte(rsOthrIns, "Gender",0);
			if(Gender==1)
				str = "X";
			else if(Gender==2)
				str = "        X";
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,21,'L',' ');

		BOOL bRelatedToAutoAcc = FALSE;
		
		// (j.jones 2009-03-09 09:30) - PLID 33354 - support the insured party accident option
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//pull from the insured party, not the bill
			bRelatedToAutoAcc = (ipatAccType == ipatAutoAcc);
		}
		else {
			//pull from BillsT.RelatedToAutoAcc
			bRelatedToAutoAcc = AdoFldBool(rsBills, "RelatedToAutoAcc", FALSE);
		}

		if(bRelatedToAutoAcc) {
			str = "X";
		}
		else {
			str = "        X";
		}
		OutputString += ParseField(str,12,'L',' ');

		//BillsT.State
		CString strState = "";

		// (j.jones 2009-03-09 09:30) - PLID 33354 - support the insured party accident option
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//pull from the insured party, not the bill
			strState = AdoFldString(rsBills, "AccidentState", "");
		}
		else {
			//pull from BillsT.State
			strState = AdoFldString(rsBills, "State", "");
		}

		OutputString += ParseField(strState,2,'L',' ');
		OutputString += ParseField("",5,'L',' ');

		//Insurance Information

		//InsuredPartyT.Employer
		if(bShowSecondaryInBox11) {

			// (j.jones 2006-10-31 15:49) - PLID 21845 - if we are showing the secondary
			//information in Box 11, that takes precedence over all other Box 11 possibilities
			str = AdoFldString(rsOthrIns, "Employer","");
		}
		else {
			// (j.jones 2007-04-09 17:29) - PLID 25537 - supported hiding Box11b individually
			if(m_HCFAInfo.HideBox11b == 0) {
				var = rsIns->Fields->Item["Employer"]->Value;
				if(var.vt==VT_BSTR) {
					str = CString(var.bstrVal);
				}
				else
					str = "";
			}
			else
				str = "";
		}
		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 13   //////////////////////////////////


		//Line 14 (double)   /////////////////////////////////////////
		OutputString = "";

		//Other Insurance Information

		//InsuredPartyT.Employer
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1 && rsOthrIns->Fields->Item["Employer"]->Value.vt==VT_BSTR) {
			str = CString(rsOthrIns->Fields->Item["Employer"]->Value.bstrVal);
		}
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');
		OutputString += ParseField("",8,'L',' ');

		BOOL bRelatedToOther = FALSE;
		
		// (j.jones 2009-03-09 09:30) - PLID 33354 - support the insured party accident option
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//pull from the insured party, not the bill
			bRelatedToOther = (ipatAccType == ipatOtherAcc);
		}
		else {
			//pull from BillsT.RelatedToOther
			bRelatedToOther = AdoFldBool(rsBills, "RelatedToOther", FALSE);
		}

		if(bRelatedToOther) {
			str = "X";
		}
		else {
			str = "        X";
		}
		OutputString += ParseField(str,19,'L',' ');

		//InsurancePlansT.PlanName
		if(bShowSecondaryInBox11) {

			// (j.jones 2006-10-31 15:49) - PLID 21845 - if we are showing the secondary
			//information in Box 11, that takes precedence over all other Box 11 possibilities
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			_RecordsetPtr rsInsPlan = CreateParamRecordset("SELECT PlanName FROM InsurancePlansT WHERE InsurancePlansT.ID = {INT}", AdoFldLong(rsOthrIns, "InsPlan", -1));
			if(!rsInsPlan->eof){
				str = AdoFldString(rsInsPlan, "PlanName", "");
			}
			else{
				str = "";
			}
			rsInsPlan->Close();
		}
		else {
			// (j.jones 2007-04-09 17:29) - PLID 25537 - supported hiding Box11c individually
			if(m_HCFAInfo.HideBox11c == 0) {
				// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
				_RecordsetPtr rsInsPlan = CreateParamRecordset("SELECT PlanName FROM InsurancePlansT WHERE InsurancePlansT.ID = {INT}", AdoFldLong(rsIns, "InsPlan", -1));
				if(!rsInsPlan->eof){
					str = AdoFldString(rsInsPlan, "PlanName", "");
				}
				else{
					str = "";
				}
				rsInsPlan->Close();
			}
			else
				str = "";
		}
		OutputString += ParseField(str,32,'L',' ');


		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 14   //////////////////////////////////


		//Line 15 (triple)  /////////////////////////////////////////
		OutputString = "";

		//Other Ins Plan
		//InsurancePlansT.PlanName
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1){
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			_RecordsetPtr rsOtherInsPlan = CreateParamRecordset("SELECT PlanName FROM InsurancePlansT WHERE InsurancePlansT.ID = {INT}", AdoFldLong(rsOthrIns, "InsPlan", -1));
			if(!rsOtherInsPlan->eof){
				str = AdoFldString(rsOtherInsPlan, "PlanName", "");
			}
			else{
				str = "";
			}
			rsOtherInsPlan->Close();
			rsOthrIns->Close();
		}
		else{
			str = "";
		}
		OutputString += ParseField(str,36,'L',' ');

		//BillsT.HCFABox10D
		str = AdoFldString(rsBills, "HCFABox10D","");
		OutputString += ParseField(str,25,'L',' ');

		rsIns->Close();

		//Other Ins

		//Box11D: 0 = fill normally, 1 = fill if no secondary, 2 = never fill
		if(m_HCFAInfo.Box11D == 2) {
			str = "";
		}
		else {
			if(m_pEBillingInfo->OthrInsuredPartyID != -1){
				str = "X";
			}
			else{
				if(m_HCFAInfo.Box11D == 0) {
					str = "       X";
				}
				else {
					str = "";
				}
			}
		}
		OutputString += ParseField(str,8,'L',' ');

		OutputString+= "\r\n\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 15 (triple)  //////////////////////////////////

		return Success;

	}NxCatchAll("Error in Ebilling::HCFABoxes1to11");

	return Error_Other;
}

// (j.jones 2011-04-20 11:18) - PLID 43329 - this function now takes in info. on skipping diag. codes
int CEbilling::HCFABoxes12to23(_RecordsetPtr &rsBills)
{
	//this exports fields 12 to 23, HCFA Image format

	try {

		_variant_t var;

		//Line 16 (double)  /////////////////////////////////////////
		CString str, OutputString = "";
		OutputString += ParseField("",9,'L',' ');

		// (j.jones 2010-07-23 15:02) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsCo now
		BOOL bAccepted = GetAcceptAssignment_ByInsCo(m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);

		// (j.jones 2010-07-23 10:52) - PLID 39795 - fixed to follow the HCFA group setting for filling Box 12
		BOOL bFillBox12 = TRUE;
		if(m_HCFAInfo.Box12Accepted == 0) {
			//never fill
			bFillBox12 = FALSE;
		}
		else if(m_HCFAInfo.Box12Accepted == 1) {
			//fill if accepted
			bFillBox12 = bAccepted;
		}
		else if(m_HCFAInfo.Box12Accepted == 2) {
			//fill if not accepted
			bFillBox12 = !bAccepted;
		}
		else if(m_HCFAInfo.Box12Accepted == 3) {
			//always fill
			bFillBox12 = TRUE;
		}

		//Box 12
		if(bFillBox12) {
			str = "SIGNATURE ON FILE";
		}
		else {
			str = "";
		}
		OutputString += ParseField(str,25,'L',' ');
		OutputString += ParseField("",14,'L',' ');

		//BillsT.Date
		//0 - the Print Date (today), 1 - the Bill Date
		if(!bFillBox12) {
			str = "";
		}
		else {
			if(m_HCFAInfo.Box12UseDate == 0)
				var = rsBills->Fields->Item["TodaysDate"]->Value;			
			else
				var = rsBills->Fields->Item["Date"]->Value;
			if(var.vt==VT_DATE) {
				COleDateTime dt;
				dt = var.date;
				str = dt.Format("%m/%d/%Y");
			}
			else
				str = "";
		}
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",10,'L',' ');

		// (j.jones 2010-07-22 12:01) - PLID 39780 - fixed to follow the HCFA group setting for filling Box 13
		BOOL bFillBox13 = TRUE;
		if(m_HCFAInfo.Box13Accepted == 0) {
			//never fill
			bFillBox13 = FALSE;
		}
		else if(m_HCFAInfo.Box13Accepted == 1) {
			//fill if accepted
			bFillBox13 = bAccepted;
		}
		else if(m_HCFAInfo.Box13Accepted == 2) {
			//fill if not accepted
			bFillBox13 = !bAccepted;
		}
		else if(m_HCFAInfo.Box13Accepted == 3) {
			//always fill
			bFillBox13 = TRUE;
		}

		// (j.jones 2010-06-10 12:53) - PLID 39095 - This is Box 13. We will output "SIGNATURE ON FILE" if the provider accepts assignment,
		// and have a per-bill setting to optionally override this to be blank or not.
		HCFABox13Over hb13Value = (HCFABox13Over)AdoFldLong(rsBills, "HCFABox13Over", (long)hb13_UseDefault);		
		if(hb13Value == hb13_No || (hb13Value == hb13_UseDefault && !bFillBox13)) {
			str = "";
		}
		else {
			str = "SIGNATURE ON FILE";
		}
		OutputString += ParseField(str,21,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 16 (double)  /////////////////////////////////

		//Line 17 /////////////////////////////////////////
		OutputString = "";

		//BillsT.ConditionDate
		
		// (j.jones 2009-03-09 09:39) - PLID 33354 - supported the option to load the accident from the insured party		
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//load from the insured party
			var = rsBills->Fields->Item["DateOfCurAcc"]->Value;
		}
		else {
			//load from the bill
			var = rsBills->Fields->Item["ConditionDate"]->Value;
		}

		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",36,'L',' ');

		// (j.jones 2016-04-11 17:22) - NX-100149 - this is now the calculated Box 15 date,
		// based on the value of BillsT.ConditionDateType
		var = rsBills->Fields->Item["Box15Date"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",7,'L',' ');

		//BillsT.NoWorkFrom
		var = rsBills->Fields->Item["NoWorkFrom"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",6,'L',' ');

		//BillsT.NoWorkTo
		var = rsBills->Fields->Item["NoWorkTo"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 17 //////////////////////////////////

		// (j.jones 2007-05-15 09:31) - PLID 25953 - Line 18 is now the Referring Physician Legacy ID, with a qualifier
		
		//Line 18 /////////////////////////////////////////
		
		OutputString = "";
		OutputString += ParseField("",32,'L',' ');

		//ReferringPhyID
		//this is the ID specified by the setup of Box17

		CString strQual, strID;

		// (j.jones 2009-02-06 16:29) - PLID 32951 - this function now requires the bill location
		// (j.jones 2009-02-23 10:54) - PLID 33167 - now we require the billing provider ID and rendering provider ID,
		// in this case the Rendering Provider ID is the Box 24J provider ID
		// (j.jones 2011-04-05 13:39) - PLID 42372 - this now returns a struct of information
		ECLIANumber eCLIANumber = UseCLIANumber(m_pEBillingInfo->BillID,m_pEBillingInfo->InsuredPartyID,m_pEBillingInfo->ProviderID, m_pEBillingInfo->Box24J_ProviderID, m_pEBillingInfo->BillLocation);

		_RecordsetPtr rsRefPhy;

		BOOL bUseProvAsRefPhy = (eCLIANumber.bUseCLIANumber && eCLIANumber.bCLIAUseBillProvInHCFABox17);

		if(bUseProvAsRefPhy) {
			//use the provider info instead of the referring physician
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			rsRefPhy = CreateParamRecordset("SELECT [Last] + ', ' + [First] + ' ' + Middle AS NameLFM, [First] + ' ' + [Middle] + ' ' + Last AS NameFML, ProvidersT.* FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
				"WHERE PersonID = {INT}",m_pEBillingInfo->ProviderID);
		}
		else {
			//use the referring physician normally
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			rsRefPhy = CreateParamRecordset("SELECT [Last] + ', ' + [First] + ' ' + Middle AS NameLFM, [First] + ' ' + [Middle] + ' ' + Last AS NameFML, ReferringPhysT.* FROM PersonT INNER JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
				"INNER JOIN BillsT ON PersonT.ID = BillsT.RefPhyID WHERE BillsT.ID = {INT}",m_pEBillingInfo->BillID);
		}

		if(bUseProvAsRefPhy) {

			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			m_rsDoc = CreateParamRecordset("SELECT * FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE ProvidersT.PersonID = {INT}",m_pEBillingInfo->ProviderID);

			if(m_rsDoc->eof) {
				//if the recordset is empty, there is no doctor. So halt everything!!!
				m_rsDoc->Close();
				
				// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
				CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
				if(!strPatientName.IsEmpty()) {
					// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
					str.Format("Could not open provider information for patient '%s', Bill ID %li. (HCFABoxes12to23)",
						strPatientName, m_pEBillingInfo->BillID);
				}
				else
					//serious problems if you get here
					str = "Could not open provider information. (HCFABoxes12to23)";

				if(m_pEBillingInfo->Box33Setup==2)
					str += "\nIt is possible that you have no provider selected in General 1 for this patient.";
				else
					str += "\nIt is possible that you have no provider selected on this patient's bill.";
				
				AfxMessageBox(str);
				return Error_Missing_Info;
			}

			//provider			
			EBilling_Box33PinANSI(strQual, strID, m_pEBillingInfo->ProviderID, m_pEBillingInfo->HCFASetupID,
				m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->InsuredPartyID, m_pEBillingInfo->BillLocation);

			m_rsDoc->Close();
		}
		else {

			if(m_HCFAInfo.UseBox23InBox17a == 0) {
				//ref phy ID (normal case)

				long nRefPhyID = -1;
				// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
				_RecordsetPtr rsTemp = CreateParamRecordset("SELECT ReferringPhysT.PersonID FROM ReferringPhysT LEFT JOIN BillsT ON ReferringPhysT.PersonID = BillsT.RefPhyID WHERE BillsT.ID = {INT}",m_pEBillingInfo->BillID);
				if(!rsTemp->eof) {
					nRefPhyID = AdoFldLong(rsTemp, "PersonID",-1);
				}
				rsTemp->Close();

				// (j.jones 2010-10-20 15:42) - PLID 40936 - moved Box17aANSI to GlobalFinancialUtils and renamed it
				EBilling_Box17aANSI(m_HCFAInfo.Box17a, m_HCFAInfo.Box17aQual, strQual, strID, nRefPhyID);
			}
			else {
				//if UseBox23InBox17a = 1 or 2 then we use the Prior Auth Num from Box 23
				//BillsT.PriorAuthNum
				var = rsBills->Fields->Item["PriorAuthNum"]->Value;
				if(var.vt == VT_BSTR){
					strID = CString(var.bstrVal);
				}
				else{
					strID = "";
				}

				//use the override from the HCFA setup
				if(str == "")
					strID = m_HCFAInfo.DefaultPriorAuthNum;

				//use the qualifier from the HCFA Setup
				strQual = m_HCFAInfo.Box17aQual;
			}
		}

		//don't send a qualifier if the ID is blank
		if(strID.IsEmpty())
			strQual = "";

		OutputString += ParseField(strQual,2,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		OutputString += ParseField(strID,22,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 18 //////////////////////////////////

		//Line 19 (double) /////////////////////////////////////////
		OutputString = "";		

		//Referring Physician Info

		//PersonT.Last, First Middle
		str = "";
		if(!rsRefPhy->eof) {
			if(m_HCFAInfo.Box17Order == 1)
				str = AdoFldString(rsRefPhy, "NameLFM","");
			else
				str = AdoFldString(rsRefPhy, "NameFML","");
		}
		OutputString += ParseField(str,30,'L',' ');
		OutputString += ParseField("",6,'L',' ');		

		// (j.jones 2006-11-10 09:42) - PLID 22668 - now we send the ref phy NPI

		// (j.jones 2007-05-15 09:46) - PLID 25953 - moved to center more since it is now on its own line
		
		//send the NPI
		str = "";
		// (j.jones 2009-01-06 10:00) - PLID 32614 - supported hiding Box 17b NPI
		if(m_HCFAInfo.HideBox17b == 0) {
			if(!rsRefPhy->eof) {
				str = AdoFldString(rsRefPhy, "NPI","");
			}

			// (j.jones 2007-08-08 10:43) - PLID 25395 - if using the provider as referring physician, we
			// need to check AdvHCFAPinT for their 24J NPI
			if(bUseProvAsRefPhy) {
				// (j.jones 2008-04-03 10:12) - PLID 28995 - continue to use the main ProviderID here
				// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
				_RecordsetPtr rsTemp = CreateParamRecordset("SELECT Box24JNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
				if(!rsTemp->eof) {
					var = rsTemp->Fields->Item["Box24JNPI"]->Value;
					if(var.vt == VT_BSTR) {
						CString strTemp = CString(var.bstrVal);
						strTemp.TrimLeft();
						strTemp.TrimRight();
						if(!strTemp.IsEmpty())
							str = strTemp;
					}
				}
				rsTemp->Close();
			}
		}

		OutputString += ParseField(str,15,'L',' ');
		OutputString += ParseField("",12,'L',' ');
		
		rsRefPhy->Close();		

		//BillsT.HospFrom
		var = rsBills->Fields->Item["HospFrom"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",6,'L',' ');

		//BillsT.HospTo
		var = rsBills->Fields->Item["HospTo"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 19 (double) //////////////////////////////////

		//Line 20 (double) /////////////////////////////////////////
		OutputString = "";

		//BillsT.HCFABlock19
		var = rsBills->Fields->Item["HCFABlock19"]->Value;
		if(var.vt == VT_BSTR){
			str = CString(var.bstrVal);
		}
		else{
			str = "";
		}

		if(str == "" && m_HCFAInfo.Box19RefPhyID == 1)
			str = Box17a();	//use the Ref Phy ID
		else if(str == "" && m_HCFAInfo.Box19Override == 1)
			str = m_HCFAInfo.Box19;

		//check for the override
		// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
		_RecordsetPtr rsTemp = CreateParamRecordset("SELECT Box19 FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
		if(!rsTemp->eof) {
			var = rsTemp->Fields->Item["Box19"]->Value;
			if(var.vt == VT_BSTR) {
				CString strTemp = CString(var.bstrVal);
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty())
					str = strTemp;
			}
		}
		rsTemp->Close();

		OutputString += ParseField(str,60,'L',' ');
		OutputString += ParseField("",4,'L',' ');

		//BillsT.OutsideLab
		var = rsBills->Fields->Item["OutsideLab"]->Value;
		if(var.vt == VT_BOOL){
			if(var.boolVal){
				str = "X";
			}
			else{
				str = "    X";
			}
		}
		else{
			str = "    X";
		}

		OutputString += ParseField(str,9,'L',' ');

		BOOL bUseDecimals = (GetRemotePropertyInt("HCFAImageUseDecimals",0,0,"<None>",true) == 1);

		//BillsT.OutLabCharges
		var = rsBills->Fields->Item["OutsideLabCharges"]->Value;
		if(var.vt == VT_CY){
			str = FormatCurrencyForInterface(var.cyVal,FALSE,FALSE);
			if(!bUseDecimals)
				str = StripNonNum(str);
		}
		else{
			str = "0";
		}
		
		if(!bUseDecimals)
			OutputString += ParseField(str,6,'R','0');
		else
			OutputString += ParseField(str,6,'R',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 20 (double) //////////////////////////////////


		//For the next two lines we're using charge information common to all charges, the 
		//looping will be later.

		//Line 21 (double) /////////////////////////////////////////
		OutputString = "";
		
		//Spaces
		OutputString += ParseField("", 2, 'L', ' ');

		//BillsT.Diag1ID
		
		// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(0).number; 

		OutputString += ParseField(str, 6, 'L', ' ');

		// (j.jones 2006-11-09 16:42) - PLID 22668 - in the new HCFA image,
		// diag codes are always 1, 3 on the first line and 2, 4 on the second line

		if(m_HCFAInfo.ShowICD9Desc) {

			str = m_pEBillingInfo->GetSafeBillDiag(0).description;

			OutputString += ParseField("", 3, 'L', ' ');
			OutputString += ParseField(str, 20, 'L', ' ');
			OutputString += ParseField("", 4, 'L', ' ');
		}
		else {
			OutputString += ParseField("", 27, 'L', ' ');
		}

		
		//BillsT.Diag3ID

		// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(2).number;

		OutputString += ParseField(str, 6, 'L', ' ');

		if(m_HCFAInfo.ShowICD9Desc) {
			
			str = m_pEBillingInfo->GetSafeBillDiag(2).description;

			OutputString += ParseField("", 3, 'L', ' ');
			OutputString += ParseField(str, 12, 'L', ' ');
			OutputString += ParseField("", 4, 'L', ' ');
		}
		else {
			OutputString += ParseField("", 19, 'L', ' ');
		}
		
		//BillsT.MedicaidResubmission
		var = rsBills->Fields->Item["MedicaidResubmission"]->Value;
		if(var.vt == VT_BSTR){
			str = CString(var.bstrVal);
		}
		else{
			str = "";
		}
		OutputString += ParseField(str, 12, 'L', ' ');
		OutputString += ParseField("", 2, 'L', ' ');

		//BillsT.OriginalRefNo
		var = rsBills->Fields->Item["OriginalRefNo"]->Value;
		if(var.vt == VT_BSTR){
			str = CString(var.bstrVal);
		}
		else{
			str = "";
		}
		OutputString += ParseField(str, 13, 'L', ' ');


		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 21 (double) //////////////////////////////////

		//Line 22 (double) /////////////////////////////////////////
		OutputString = "";

		//Spaces
		OutputString += ParseField("", 2, 'L', ' ');

		// (j.jones 2006-11-09 16:42) - PLID 22668 - in the new HCFA image,
		// diag codes are always 1, 3 on the first line and 2, 4 on the second line

		//BillsT.Diag2ID
		
		// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(1).number;

		OutputString += ParseField(str, 6, 'L', ' ');

		if(m_HCFAInfo.ShowICD9Desc) {
			
			str = m_pEBillingInfo->GetSafeBillDiag(1).description;

			OutputString += ParseField("", 3, 'L', ' ');
			OutputString += ParseField(str, 20, 'L', ' ');
			OutputString += ParseField("", 4, 'L', ' ');
		}
		else {
			OutputString += ParseField("", 27, 'L', ' ');
		}
		
		//BillsT.Diag4ID
		
		// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(3).number;

		OutputString += ParseField(str, 6, 'L', ' ');

		if(m_HCFAInfo.ShowICD9Desc) {
			
			str = m_pEBillingInfo->GetSafeBillDiag(3).description;

			OutputString += ParseField("", 3, 'L', ' ');
			OutputString += ParseField(str, 12, 'L', ' ');
			OutputString += ParseField("", 4, 'L', ' ');
		}
		else {
			OutputString += ParseField("", 19, 'L', ' ');
		}

		//if UseBox23InBox17a = 2, then we don't fill Box 23 (though CLIA overrides this)
		if(m_HCFAInfo.UseBox23InBox17a != 2) {
			//BillsT.PriorAuthNum
			var = rsBills->Fields->Item["PriorAuthNum"]->Value;
			if(var.vt == VT_BSTR){
				str = CString(var.bstrVal);
			}
			else{
				str = "";
			}

			//use the override from the HCFA setup
			if(str == "")
				str = m_HCFAInfo.DefaultPriorAuthNum;
		}
		else {
			str = "";
		}

		if(eCLIANumber.bUseCLIANumber && eCLIANumber.bUseCLIAInHCFABox23) {
			//override with the CLIA
			str = eCLIANumber.strCLIANumber;
		}
		
		OutputString += ParseField(str, 32, 'L', ' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 22 (double) //////////////////////////////////

		return Success;

	}NxCatchAll("Error in Ebilling::HCFABoxes12to23");

	return Error_Other;
}

// (j.jones 2011-04-20 11:18) - PLID 43329 - this function now takes in info. on skipping diag. codes
int CEbilling::HCFABoxes24to33(ADODB::_RecordsetPtr &rsBills, long nCurPage, long nPages)
{
	//this exports fields 24 to 33, HCFA Image format

	try {

		CString str = "";

		//open the doctors' recordset first
		// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
		m_rsDoc = CreateParamRecordset("SELECT * FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE ProvidersT.PersonID = {INT}",m_pEBillingInfo->ProviderID);

		if(m_rsDoc->eof) {
			//if the recordset is empty, there is no doctor. So halt everything!!!
			m_rsDoc->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open provider information for patient '%s', Bill ID %li. (HCFABoxes24To33)",
						strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open provider information. (HCFABoxes24To33)";

			if(m_pEBillingInfo->Box33Setup==2)
				str += "\nIt is possible that you have no provider selected in General 1 for this patient.";
			else
				str += "\nIt is possible that you have no provider selected on this patient's bill.";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		int ChargesPerPage = GetRemotePropertyInt("HCFAImageChargesPerPage", 6, 0, "<None>", true);

		BOOL bUseDecimals = (GetRemotePropertyInt("HCFAImageUseDecimals",0,0,"<None>",true) == 1);

		_variant_t var;

		CString OutputString;
		int chargecount = ChargesPerPage;

		//JJ - we must only output 6 charges per claim, so we output the amount of charges on the current page

		//also calculate the total as we go
		COleCurrency cyCharges = COleCurrency(0,0);
		
		long nLastCharge = m_nCurrPage * ChargesPerPage;
		long nFirstCharge = nLastCharge - (ChargesPerPage - 1);
		
		//we will loop from nFirstCharge through nLastCharge, or EOF, whichever comes first;
		
		//first loop up until the charge we need to start on
		//TES 11/5/2007 - PLID 27978 - VS2008 - for() loops
		int charge = 1;
		for(charge = 1; charge < nFirstCharge; charge++) {

			if(rsBills->eof) {
				continue;
			}

			rsBills->MoveNext();
		}

		CString strChargeIDs = "";

		for(charge = nFirstCharge; charge <= nLastCharge; charge++) {

			if(rsBills->eof) {
				continue;
			}

			// (j.jones 2007-10-05 10:52) - PLID 27659 - track the IDs of the charges we are outputting
			long nChargeID = AdoFldLong(rsBills, "ChargeID");
			if(!strChargeIDs.IsEmpty())
				strChargeIDs += ",";
			strChargeIDs += AsString(nChargeID);

			// (j.jones 2007-10-15 15:05) - PLID 27757 - track the ServiceID
			long nServiceID = AdoFldLong(rsBills, "ServiceID");

			// (j.jones 2007-05-15 11:26) - PLID 25953 - now send the Box24J info on the first line,
			// charge info and NPI on the second line

			//Line 23 /////////////////////////////////////////
			OutputString = "";

			str = "";

			// (j.jones 2007-05-15 11:59) - PLID 25953 - send the new HCFA note format for anesthesia times,
			// meaning the "only show minutes" option no longer applies
			// (j.jones 2008-05-02 10:50) - PLID 28012 - removed the requirement for UseAnesthesiaBilling
			// to be checked for the service
			if(m_pEBillingInfo->AnesthesiaTimes && AdoFldBool(rsBills, "Anesthesia",FALSE)
				/*&& AdoFldBool(rsBills, "UseAnesthesiaBilling",FALSE)*/) {
				
				str = CalculateAnesthesiaNoteForHCFA(m_pEBillingInfo->BillID);
			}

			// (j.jones 2008-05-28 15:08) - PLID 30177 - supported NDC codes, only if there is no anesthesia note
			if(str.IsEmpty()) {
				str = AdoFldString(rsBills, "NDCCode", "");
				str.TrimLeft();
				str.TrimRight();
			}

			OutputString += ParseField(str,78,'L',' ');
			OutputString += ParseField("",5,'L',' ');

			CString strQual, strID;

			//InsuranceBox24J.Box24JNumber
			// (j.jones 2008-04-03 10:12) - PLID 28995 - use the Box24J_ProviderID here
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Box24JNumber, Box24IQualifier FROM InsuranceBox24J WHERE InsCoID = {INT} AND ProviderID = {INT}",
					m_pEBillingInfo->InsuranceCoID,m_pEBillingInfo->Box24J_ProviderID);

			if(!rs->eof) {
				strID = AdoFldString(rs, "Box24JNumber","");
				strQual = AdoFldString(rs, "Box24IQualifier","");
			}
			rs->Close();

			//check for the override
			// (j.jones 2008-04-03 10:12) - PLID 28995 - use the Box24J_ProviderID here
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			_RecordsetPtr rsTemp = CreateParamRecordset("SELECT Box24J, Box24JQual FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->Box24J_ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
			if(!rsTemp->eof) {
				CString strTemp = AdoFldString(rsTemp, "Box24J","");
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty())
					strID = strTemp;

				strTemp = AdoFldString(rsTemp, "Box24JQual","");
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty())
					strQual = strTemp;
			}
			rsTemp->Close();

			if(strID.IsEmpty())
				strQual = "";

			OutputString += ParseField(strQual,2,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			OutputString += ParseField(strID,15,'L',' ');

			OutputString+= "\r\n";
			TRACE(OutputString);
			m_OutputFile.Write(OutputString,OutputString.GetLength());
			//End of Line 23 /////////////////////////////////////////

			//Line 24 /////////////////////////////////////////
			OutputString = "";

			//BillsT.ServiceDateFrom
			var = rsBills->Fields->Item["ServiceDateFrom"]->Value;
			if(var.vt==VT_DATE) {
				COleDateTime dt;
				dt = var.date;
				str = dt.Format("%m/%d/%Y");
			}
			else
				str = "";
			OutputString += ParseField(str,10,'L',' ');
			OutputString += ParseField("",2,'L',' ');
			
			//BillsT.ServiceDateTo
			var = rsBills->Fields->Item["ServiceDateTo"]->Value;
			// (j.jones 2007-04-10 08:44) - PLID 25539 - added ability to hide the ServiceDateTo field
			if(var.vt==VT_DATE && m_HCFAInfo.Hide24ATo == 0) {
				COleDateTime dt;
				dt = var.date;
				str = dt.Format("%m/%d/%Y");
			}
			else
				str = "";
			OutputString += ParseField(str,10,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.ServiceLocation
			var = rsBills->Fields->Item["PlaceCodes"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",1,'L',' ');

			// (j.jones 2007-05-15 12:02) - PLID 25953 - ServiceType is no longer present on the new HCFA
			/*
			//ChargesT.ServiceType
			var = rsBills->Fields->Item["ServiceType"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			*/

			// (j.jones 2008-02-04 14:46) - PLID 28788 - send the Box24C EMG setup here
			// (j.jones 2010-04-08 13:46) - PLID 38095 - ChargesT.IsEmergency determines whether
			// we send the setting on this charge, or use the HCFA Setup value
			ChargeIsEmergencyType eIsEmergency = (ChargeIsEmergencyType)AdoFldLong(rsBills, "IsEmergency");
			if(eIsEmergency == cietNo || (eIsEmergency == cietUseDefault && m_HCFAInfo.Box24C == 2)) {
				str = "N";
			}
			else if(eIsEmergency == cietYes || (eIsEmergency == cietUseDefault && m_HCFAInfo.Box24C == 1)) {
				str = "Y";
			}
			//whether eIsEmergency is cietBlank, or m_HCFAInfo.Box24C is not 1 or 2, send nothing
			else {
				str = "";
			}
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",1,'L',' ');

			//ChargesT.ItemCode
			var = rsBills->Fields->Item["ItemCode"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,6,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.CPTModifier
			var = rsBills->Fields->Item["CPTModifier"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.CPTModifier2
			var = rsBills->Fields->Item["CPTModifier2"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.CPTModifier3
			var = rsBills->Fields->Item["CPTModifier3"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.CPTModifier4
			var = rsBills->Fields->Item["CPTModifier4"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",2,'L',' ');


			//ChargesT.WhichCodes
			// (j.gruber 2014-03-17 11:26) - PLID 61395 - update billing structure
			var = rsBills->Fields->Item["ChargeWhichCodes"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";

			OutputString += ParseField(str,7,'L',' ');

			// (j.jones 2011-06-17 14:34) - PLID 34795 - I reduced this space from 6 to 5,
			// and increased the charge field from 7 to 8, so if you submit a 10000.00 
			// charge it will take up one more space to the left.
			OutputString += ParseField("",5,'L',' ');

			//we need the Amount * Quantity here, which I calculate in the recordset
			var = rsBills->Fields->Item["ChargeTot"]->Value;
			if(var.vt==VT_CY) {
				cyCharges += var.cyVal;
				str = FormatCurrencyForInterface(var.cyVal,FALSE,FALSE);
				if(!bUseDecimals)
					str = StripNonNum(str);
			}
			else
				str = "";
			if(!bUseDecimals)
				OutputString += ParseField(str,8,'R','0');
			else
				OutputString += ParseField(str,8,'R',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.Quantity
			var = rsBills->Fields->Item["Quantity"]->Value;
			if(var.vt==VT_R8) {
				str.Format("%g",var.dblVal);
			}
			else
				str = "";

			// (j.jones 2007-01-03 13:26) - PLID 24084 - show minutes as quantity if anesthesia
			// (j.jones 2008-05-02 10:50) - PLID 28012 - removed the requirement for UseAnesthesiaBilling
			// to be checked for the service, and for a flat-fee
			if(m_HCFAInfo.ANSI_UseAnesthMinutesAsQty == 1 && AdoFldBool(rsBills, "Anesthesia", FALSE)
				/*&& AdoFldBool(rsBills, "UseAnesthesiaBilling", FALSE)
				// (j.jones 2007-10-15 14:57) - PLID 27757 - converted to use the new structure
				&& ReturnsRecords("SELECT ID FROM AnesthesiaSetupT WHERE ServiceID = %li AND LocationID = %li AND AnesthesiaFeeBillType <> 1", nServiceID, m_pEBillingInfo->nPOSID)*/) {

				//they want to show minutes, and this is indeed an anesthesia charge that is based on time
				str.Format("%li",AdoFldLong(rsBills, "AnesthesiaMinutes", 0));
			}

			OutputString += ParseField(str,3,'L',' ');
			OutputString += ParseField("",8,'L',' ');

			// (j.jones 2007-05-15 11:27) - PLID 25953 - send the provider's individual NPI here
			// (j.jones 2007-07-12 16:22) - PLID 26636 - only if it is not hidden
			if(m_HCFAInfo.HideBox24JNPI == 0) {
				str = AdoFldString(m_rsDoc, "NPI","");

				// (j.jones 2007-08-08 10:43) - PLID 25395 - we need to check AdvHCFAPinT for the 24J NPI
				// (j.jones 2008-04-03 10:12) - PLID 28995 - use the Box24J_ProviderID here
				// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
				_RecordsetPtr rsTemp = CreateParamRecordset("SELECT Box24JNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->Box24J_ProviderID, m_pEBillingInfo->BillLocation, m_pEBillingInfo->HCFASetupID);
				if(!rsTemp->eof) {
					var = rsTemp->Fields->Item["Box24JNPI"]->Value;
					if(var.vt == VT_BSTR) {
						CString strTemp = CString(var.bstrVal);
						strTemp.TrimLeft();
						strTemp.TrimRight();
						if(!strTemp.IsEmpty())
							str = strTemp;
					}
				}
				rsTemp->Close();
			}
			else
				str = "";
			OutputString += ParseField(str,15,'L',' ');

			OutputString += "\r\n";
			TRACE(OutputString);
			m_OutputFile.Write(OutputString,OutputString.GetLength());
			//End Of Line 24 //////////////////////////////////

			chargecount--;
			rsBills->MoveNext();
		}

		rsBills->Close();

		for(int i=0;i<=chargecount;i++) {
			OutputString = "\r\n\r\n";
			TRACE(OutputString);
			m_OutputFile.Write(OutputString,OutputString.GetLength());
		}

		//Line 25 (double) /////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",2,'L',' ');

		//Federal Tax ID Number - SSN or EIN

		//check for the override
		BOOL bOverride = FALSE;
		// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
		// (j.jones 2008-09-10 11:35) - PLID 30788 - added Box25Check
		_RecordsetPtr rsTemp = CreateParamRecordset("SELECT EIN, Box25Check FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
		long nBox25Check = 0;
		if(!rsTemp->eof) {
			var = rsTemp->Fields->Item["EIN"]->Value;
			if(var.vt == VT_BSTR) {
				CString strTemp = CString(var.bstrVal);
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty()) {
					str = strTemp;
					bOverride = TRUE;
				}
			}
			nBox25Check = AdoFldLong(rsTemp, "Box25Check", 0);
		}
		rsTemp->Close();

		if(m_HCFAInfo.Box25 == 1) { //SSN

			//PersonT.SocialSecurity
			if(!bOverride) {
				var = m_rsDoc->Fields->Item["SocialSecurity"]->Value;
				if(var.vt==VT_BSTR) {
					str = CString(var.bstrVal);
				}
				else
					str = "";
			}
			// (j.jones 2007-06-21 17:13) - PLID 26424 - needs to be 11 characters
			OutputString += ParseField(str,11,'L',' ');
			OutputString += ParseField("",1,'L',' ');

			// (j.jones 2008-09-10 11:36) - PLID 30788 - if the nBox25Check is 2,
			// override with EIN, otherwise it is SSN
			if(nBox25Check == 2) {
				str = "   X";
			}
			else {
				str = "X";
			}
			OutputString += ParseField(str,15,'L',' ');
		}
		else { //Fed Emp. ID

			//ProvidersT.[Fed Employer ID] OR LocationsT.EIN
			if(!bOverride) {

				if(m_HCFAInfo.Box33Setup == 4) {
					//use bill location's EIN
					//DRT 8/25/2008 - PLID 31163 - Undid j.jones change, which I believe was mistaken.  This is the EIN, not the NPI.
					_RecordsetPtr rsLoc = CreateRecordset("SELECT EIN FROM LocationsT WHERE ID = %li",m_pEBillingInfo->BillLocation);
					if(!rsLoc->eof) {
						var = rsLoc->Fields->Item["EIN"]->Value;
						if(var.vt==VT_BSTR) {
							str = CString(var.bstrVal);
						}
						else
							str = "";
					}
					rsLoc->Close();
				}
				else {
					//use provider's fed. employer ID

					var = m_rsDoc->Fields->Item["Fed Employer ID"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
				}
			}
			OutputString += ParseField(str,10,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			// (j.jones 2008-09-10 11:36) - PLID 30788 - if the nBox25Check is 1,
			// override with SSN, otherwise it is EIN
			if(nBox25Check == 1) {
				str = "X";
			}
			else {
				str = "   X";
			}
			OutputString += ParseField(str,15,'L',' ');
		}

		//PatientsT.UserDefinedID
		str.Format("%li",m_pEBillingInfo->UserDefinedID);

		//determine if we need to enlarge this field
		BOOL bExtendPatIDField = FALSE;

		if(GetRemotePropertyInt("SearchByBillID", 0, 0, "<None>", TRUE) == 1) {
			//use patient ID / bill ID as the patient ID
			str.Format("%li/%li",m_pEBillingInfo->UserDefinedID, m_pEBillingInfo->BillID);

			bExtendPatIDField = TRUE;

			if(GetRemotePropertyInt("HCFAIDAppendRespType", 0, 0, "<None>", TRUE) == 1) {
				//use patient ID / bill ID-RespTypeID as the patient ID (but only if it is secondary or higher)
				int RespTypeID = GetInsuranceTypeFromID(m_pEBillingInfo->InsuredPartyID);
				if(RespTypeID > 1)
					str.Format("%li/%li-%li",m_pEBillingInfo->UserDefinedID, m_pEBillingInfo->BillID, RespTypeID);
			}
		}
		OutputString += ParseField(str,bExtendPatIDField ? 17 : 7,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		//InsuranceAcceptedT.Accepted

		str = "    X";
		// (j.jones 2010-07-23 15:02) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsCo now
		BOOL bAccepted = GetAcceptAssignment_ByInsCo(m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);
		if(bAccepted) {
			str = "X";
		}
		OutputString += ParseField(str,bExtendPatIDField ? 12 : 22,'L',' ');

		//JJ 10/27/2003 - We can't always calculate cyCharges here because if the HCFA form is two pages or more,
		//then the total won't be accurate with the charges displayed on the page. So we must calculate that total instead.

		// (j.jones 2007-10-05 10:48) - PLID 27659 - Somehow, past-Josh didn't seem to think this same logic would count towards
		// the amount paid as well. We need to show the amount paid for the charges on this page, not the whole bill.

		COleCurrency cyApplies = COleCurrency(0,0),
					cyTotal = COleCurrency(0,0);

		// (j.jones 2008-06-18 09:34) - PLID 30403 - CalculateBoxes29and30 will calculate the values
		// for Box 29 and Box 30 based on the settings to ShowPays, IgnoreAdjustments, ExcludeAdjFromBal,
		// and DoNotFillBox29. I made it a modular function because it is identical for all three image types.
		CalculateBoxes29and30(cyCharges, cyApplies, cyTotal, strChargeIDs);	

		// (j.jones 2007-02-28 16:03) - PLID 25009 - for the three totals fields,
		// they now allow 8 characters total, supporting up to 5 characters to the
		// right of the decimal, if decimals are used. Spaces in between were
		// updated appropriately so the dollar amounts didn't move.

		//total charges
		str = FormatCurrencyForInterface(cyCharges,FALSE,FALSE);
		if(!bUseDecimals)
			str = StripNonNum(str);
		if(!bUseDecimals)
			OutputString += ParseField(str,8,'R','0');
		else
			OutputString += ParseField(str,8,'R',' ');
		OutputString += ParseField("",7,'L',' ');

		//total applies, which are only filled in if HCFASetupT.ShowPays = TRUE
		str = FormatCurrencyForInterface(cyApplies,FALSE,FALSE);
		if(!bUseDecimals)
			str = StripNonNum(str);
		if(!bUseDecimals)
			OutputString += ParseField(str,8,'R','0');
		else
			OutputString += ParseField(str,8,'R',' ');
		OutputString += ParseField("",4,'L',' ');

		//based on our setting, hide the total balance
		if(m_HCFAInfo.HideBox30 == 1) {
			str = "";
		}
		else {
			//if HCFASetupT.ShowPays = TRUE, this is the balance,
			//if not, this is the total charges
			str = FormatCurrencyForInterface(cyTotal,FALSE,FALSE);
			if(!bUseDecimals)
				str = StripNonNum(str);
		}

		if(!bUseDecimals && m_HCFAInfo.HideBox30 == 0)
			OutputString += ParseField(str,8,'R','0');
		else
			OutputString += ParseField(str,8,'R',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 25 (double) //////////////////////////////////		

		//Line 26 //////////////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",29,'L',' ');

		bool bHideBox32NameAdd = false;
		//check to see if we should hide box 32

		// (j.jones 2007-05-11 12:02) - PLID 25932 - reworked this to have options to 
		// hide just the name and address, and the IDs
		// (j.gruber 2007-06-28 16:49) - PLID 26154 - fixed the query for if POS is null
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		if(m_HCFAInfo.HideBox32NameAdd != 0) {

			//if HideBox32NameAdd = 1, they want to hide the name/add in box 32 if the POS is 11,
			//so check to see if any of the charges associated with this bill have a POS of 11
			//if HideBox32NameAdd = 2, they want to hide the name/add in box 32 if the POS is NOT 11,
			//so check to see if any of the charges associated with this bill do not have a POS of 11
			//if HideBox32NameAdd = 3, they want to always hide the name/add in box 32
			// (j.jones 2013-04-25 11:05) - PLID 55564 - the POS code is now cached, we can save a recordset

			if(m_HCFAInfo.HideBox32NameAdd == 3
				|| (m_HCFAInfo.HideBox32NameAdd == 1 && m_pEBillingInfo->strPOSCode == "11")
				|| (m_HCFAInfo.HideBox32NameAdd == 2 && m_pEBillingInfo->strPOSCode != "11")) {

				bHideBox32NameAdd = true;
			}
			else{
				bHideBox32NameAdd = false;
			}
		}
		else{
			bHideBox32NameAdd = false;
		}

		//LocationsT.Name (POS)
		_RecordsetPtr rsPOS;

		// (j.jones 2013-04-25 11:07) - PLID 55564 - supported the preference to use the patient's address
		// if the POS code is 12, this is assigned in LoadClaimInfo
		if(m_pEBillingInfo->bSendPatientAddressAsPOS) {
			rsPOS = CreateParamRecordset("SELECT 'Patient Home' AS Name, '' AS NPI, Address1, Address2, City, State, Zip "
				"FROM PersonT "
				"WHERE ID = {INT}", m_pEBillingInfo->PatientID);
		}
		else {
			//get the location where the service was performed
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized)
			rsPOS = CreateParamRecordset("SELECT Name, NPI, Address1, Address2, City, State, Zip "
				"FROM LocationsT "
				"WHERE ID = {INT}",m_pEBillingInfo->nPOSID);
		}

		if(!bHideBox32NameAdd && !rsPOS->eof && rsPOS->Fields->Item["Name"]->Value.vt==VT_BSTR)
			str = CString(rsPOS->Fields->Item["Name"]->Value.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,26,'L',' ');
		OutputString += ParseField("",6,'L',' ');

		// (j.jones 2007-05-07 14:38) - PLID 25922 - added Box33Name override
		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		CString strBox33NameOver = "", strBox33Address1_Over = "", strBox33Address2_Over = "",
			strBox33City_Over = "", strBox33State_Over = "", strBox33Zip_Over = "";
		// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
		_RecordsetPtr rsBox33Over = CreateParamRecordset("SELECT Box33Name, "
			"Box33_Address1, Box33_Address2, Box33_City, Box33_State, Box33_Zip "
			"FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
		if(!rsBox33Over->eof) {
			strBox33NameOver = AdoFldString(rsBox33Over, "Box33Name","");
			strBox33NameOver.TrimLeft();
			strBox33NameOver.TrimRight();

			// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
			strBox33Address1_Over = AdoFldString(rsBox33Over, "Box33_Address1","");
			strBox33Address1_Over.TrimLeft();
			strBox33Address1_Over.TrimRight();
			strBox33Address2_Over = AdoFldString(rsBox33Over, "Box33_Address2","");
			strBox33Address2_Over.TrimLeft();
			strBox33Address2_Over.TrimRight();
			strBox33City_Over = AdoFldString(rsBox33Over, "Box33_City","");
			strBox33City_Over.TrimLeft();
			strBox33City_Over.TrimRight();
			strBox33State_Over = AdoFldString(rsBox33Over, "Box33_State","");
			strBox33State_Over.TrimLeft();
			strBox33State_Over.TrimRight();
			strBox33Zip_Over = AdoFldString(rsBox33Over, "Box33_Zip","");
			strBox33Zip_Over.TrimLeft();
			strBox33Zip_Over.TrimRight();
		}
		rsBox33Over->Close();
		
		if(!strBox33NameOver.IsEmpty()) {
			//if not empty, output the override
			OutputString += ParseField(strBox33NameOver,45,'L',' ');
		}
		else {
			//otherwise do the normal load

			if(m_HCFAInfo.Box33Setup == 4) {
				//bill location
				// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
				_RecordsetPtr rsLoc = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}",m_pEBillingInfo->BillLocation);
				if(!rsLoc->eof) {
					CString strName = AdoFldString(rsLoc, "Name","");
					OutputString += ParseField(strName,45,'L',' ');
				}
				rsLoc->Close();
			}
			else if(m_HCFAInfo.Box33Setup == 3) {
				//override
				// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
				_RecordsetPtr rsOver = CreateParamRecordset("SELECT Note FROM PersonT WHERE ID = {INT}",m_HCFAInfo.Box33Num);
				if(!rsOver->eof) {
					CString strName = AdoFldString(rsOver, "Note","");
					OutputString += ParseField(strName,45,'L',' ');
				}
				rsOver->Close();
			}
			else {
				//a provider

				if(m_HCFAInfo.Box33Order == 1) { //LFM

					//PersonT.Last
					var = m_rsDoc->Fields->Item["Last"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,16,'L',' ');
					OutputString += ParseField("",1,'L',' ');

					//PersonT.First
					var = m_rsDoc->Fields->Item["First"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,13,'L',' ');
					OutputString += ParseField("",1,'L',' ');

					//PersonT.Middle
					var = m_rsDoc->Fields->Item["Middle"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,1,'L',' ');
					OutputString += ParseField("",1,'L',' ');				

				}
				else {	//FML

					//PersonT.First
					var = m_rsDoc->Fields->Item["First"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,13,'L',' ');
					OutputString += ParseField("",1,'L',' ');

					//PersonT.Middle
					var = m_rsDoc->Fields->Item["Middle"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,1,'L',' ');
					OutputString += ParseField("",1,'L',' ');

					//PersonT.Last
					var = m_rsDoc->Fields->Item["Last"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,16,'L',' ');
					OutputString += ParseField("",1,'L',' ');

				}

				//PersonT.Title
				var = m_rsDoc->Fields->Item["Title"]->Value;
				if(var.vt==VT_BSTR) {
					str = CString(var.bstrVal);
				}
				else
					str = "";
				OutputString += ParseField(str,5,'L',' ');
			}
		}

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 26 ///////////////////////////////////////////

		//Line 27 //////////////////////////////////////////////////
		OutputString = "";

		//InsuranceBox31.Box31Info
		// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT Box31Info FROM InsuranceBox31 WHERE InsCoID = {INT} AND ProviderID = {INT}",
				m_pEBillingInfo->InsuranceCoID,m_pEBillingInfo->ProviderID);

		if(!rs->eof && rs->Fields->Item["Box31Info"]->Value.vt==VT_BSTR) {
			str = CString(rs->Fields->Item["Box31Info"]->Value.bstrVal);
		}
		else
			str = "";
		OutputString += ParseField(str,15,'L',' ');
		OutputString += ParseField("",14,'L',' ');

		// (j.jones 2007-05-03 09:37) - PLID 25885 - mimic the HCFA such that
		// Address 1 and Address 2 are concatenated on the same line,
		// though are still limited to 29 characters total

		//LocationsT.Address1 + Address 2 (POS)
		if(!bHideBox32NameAdd) {
			str.Format("%s %s", AdoFldString(rsPOS, "Address1",""), AdoFldString(rsPOS, "Address2",""));
		}
		else
			str = "";
		OutputString += ParseField(str,29,'L',' ');		
		OutputString += ParseField("",3,'L',' ');

		//LocationsT.Address1 + Address 2 (Location)
		_RecordsetPtr rsLoc;
		if(m_HCFAInfo.Box33Setup == 3) {
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			rsLoc = CreateParamRecordset("SELECT * FROM PersonT WHERE ID = {INT}",m_HCFAInfo.Box33Num);
		}
		else if(m_HCFAInfo.DocAddress == 1 || m_HCFAInfo.Box33Setup == 4) {
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			rsLoc = CreateParamRecordset("SELECT * FROM LocationsT WHERE ID = {INT}",m_pEBillingInfo->BillLocation);
		}
		else {
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			rsLoc = CreateParamRecordset("SELECT * FROM PersonT WHERE ID = {INT}",m_pEBillingInfo->ProviderID);
		}

		// (j.jones 2007-05-03 09:37) - PLID 25885 - mimic the HCFA such that
		// Address 1 and Address 2 are concatenated on the same line,
		// though are still limited to 32 characters total

		CString strAddress1 = AdoFldString(rsLoc, "Address1","");
		CString strAddress2 = AdoFldString(rsLoc, "Address2","");

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33Address1_Over.IsEmpty()) {
			strAddress1 = strBox33Address1_Over;
		}
		if(!strBox33Address2_Over.IsEmpty()) {
			strAddress2 = strBox33Address2_Over;
		}

		str.Format("%s %s", strAddress1, strAddress2);
		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 27 ////////////////////////////////////////////

		//Line 28 ///////////////////////////////////////////////////
		OutputString = "";

		str = "";

		//Box31 may use the Override name
		if(m_HCFAInfo.UseOverrideInBox31 == 1 && m_HCFAInfo.Box33Setup == 3) {
			//override
			str = AdoFldString(rsLoc, "Note","");
		}
		else {

			//PersonT.First
			var = m_rsDoc->Fields->Item["First"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";

			str += " ";

			//PersonT.Middle
			var = m_rsDoc->Fields->Item["Middle"]->Value;
			if(var.vt==VT_BSTR) {
				str += CString(var.bstrVal);
			}
			else
				str += "";

			str += " ";

			//PersonT.Last
			var = m_rsDoc->Fields->Item["Last"]->Value;
			if(var.vt==VT_BSTR) {
				str += CString(var.bstrVal);
			}
			else
				str += "";
			
			str += " ";

			//PersonT.Title
			var = m_rsDoc->Fields->Item["Title"]->Value;
			if(var.vt==VT_BSTR) {
				str += CString(var.bstrVal);
			}
			else
				str += "";
		}

		OutputString += ParseField(str,27,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.City (POS)
		if(!bHideBox32NameAdd) {
			var = rsPOS->Fields->Item["City"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,14,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.State (POS)
		if(!bHideBox32NameAdd) {
			var = rsPOS->Fields->Item["State"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,2,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.Zip (POS)
		if(!bHideBox32NameAdd) {
			var = rsPOS->Fields->Item["Zip"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.City (Location)
		var = rsLoc->Fields->Item["City"]->Value;
		if(var.vt==VT_BSTR) {
			str = CString(var.bstrVal);
		}
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33City_Over.IsEmpty()) {
			str = strBox33City_Over;
		}

		OutputString += ParseField(str,15,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.State (Location)
		var = rsLoc->Fields->Item["State"]->Value;
		if(var.vt==VT_BSTR) {
			str = CString(var.bstrVal);
		}
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33State_Over.IsEmpty()) {
			str = strBox33State_Over;
		}

		OutputString += ParseField(str,2,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.Zip (Location)
		var = rsLoc->Fields->Item["Zip"]->Value;
		if(var.vt==VT_BSTR) {
			str = CString(var.bstrVal);
		}
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33Zip_Over.IsEmpty()) {
			str = strBox33Zip_Over;
		}

		// (j.jones 2007-08-28 08:40) - PLID 27197 - expanded to 10 characters
		OutputString += ParseField(str,10,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		// (j.jones 2006-12-11 09:22) - PLID 23823 - we used to export the phone
		// number here but now we do it on line 30

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 28 ////////////////////////////////////////////

		//Line 29 //////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",16,'L',' ');

		//0 - the Print Date (today), 1 - the Bill Date
		if(m_HCFAInfo.Box31UseDate == 0) {
			COleDateTime dt;
			dt = COleDateTime::GetCurrentTime();
			str = dt.Format("%m/%d/%Y");
		}
		else {

			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			rsBills = CreateParamRecordset("SELECT Date FROM BillsT WHERE ID = {INT}",m_pEBillingInfo->BillID);
			//BillsT.Date
			if(!rsBills->eof && rsBills->Fields->Item["Date"]->Value.vt == VT_DATE) {
				COleDateTime dt = rsBills->Fields->Item["Date"]->Value.date;
				str = dt.Format("%m/%d/%Y");
			}
			else
				str = "";
		}
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",3,'L',' ');

		bool bHideBox32NPIID = false;
		//check to see if we should hide box 32

		// (j.jones 2007-05-11 12:02) - PLID 25932 - reworked this to have options to 
		// hide just the name and address, and the IDs
		// (j.gruber 2007-06-28 16:49) - PLID 26154 - fixed the query for if POS is null
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		if(m_HCFAInfo.HideBox32NPIID != 0) {
			//if HideBox32NPIID = 1, they want to hide the IDs in box 32 if the POS is 11,
			//so check to see if any of the charges associated with this bill have a POS of 11
			//if HideBox32NPIID = 2, they want to hide the IDs in box 32 if the POS is NOT 11,
			//so check to see if any of the charges associated with this bill do not have a POS of 11
			//if HideBox32NPIID = 3, they want to always hide the IDs in box 32
			if(m_HCFAInfo.HideBox32NPIID == 3 || (m_HCFAInfo.HideBox32NPIID == 1 && !IsRecordsetEmpty("SELECT PlaceOfServiceCodesT.PlaceCodes FROM ChargesT INNER JOIN LINEITEMT ON ChargesT.ID = LineItemT.ID "
				" LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				" LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				" LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				" WHERE BillID = %li AND Deleted = 0 AND Batched = 1 "
				" AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				" AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				" AND PlaceCodes = '11'", m_pEBillingInfo->BillID)) ||
				
				(m_HCFAInfo.HideBox32NPIID == 2 && !IsRecordsetEmpty("SELECT PlaceOfServiceCodesT.PlaceCodes FROM ChargesT INNER JOIN LINEITEMT ON ChargesT.ID = LineItemT.ID "
				" LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				" LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				" LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				" WHERE BillID = %li AND Deleted = 0 AND Batched = 1 "
				" AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				" AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				" AND (PlaceCodes <> '11' OR PlaceCodes IS NULL) ", m_pEBillingInfo->BillID))) {

				bHideBox32NPIID = true;
			}
			else{
				bHideBox32NPIID = false;
			}
		}
		else{
			bHideBox32NPIID = false;
		}

		// (j.jones 2007-05-04 11:55) - PLID 25908 - supported option to send Box 32 NPI
		// and Facility ID, instead of just the facility ID;

		CString strPOSNPI = "";
		CString strFacilityID = "";
		CString strFacilityQual = "";

		// (j.jones 2013-04-25 11:09) - PLID 55564 - do not send an ID if we are sending the patient address
		if(m_pEBillingInfo->bSendPatientAddressAsPOS) {
			bHideBox32NPIID = TRUE;
		}

		if(!bHideBox32NPIID) {

			strPOSNPI = AdoFldString(rsPOS, "NPI","");

			//Facility ID (POS)
			// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
			_RecordsetPtr rsFID = CreateParamRecordset("SELECT FacilityID, Qualifier FROM InsuranceFacilityID WHERE LocationID = {INT} AND InsCoID = {INT}",m_pEBillingInfo->nPOSID, m_pEBillingInfo->InsuranceCoID);
			if(!rsFID->eof) {
				strFacilityQual = AdoFldString(rsFID, "Qualifier", "");
				strFacilityID = AdoFldString(rsFID, "FacilityID", "");
			}
			rsFID->Close();

			// (j.jones 2009-02-06 16:29) - PLID 32951 - this function now requires the bill location
			// (j.jones 2009-02-23 10:54) - PLID 33167 - now we require the billing provider ID and rendering provider ID,
			// in this case the Rendering Provider ID is the Box 24J provider ID
			// (j.jones 2011-04-05 13:39) - PLID 42372 - this now returns a struct of information
			ECLIANumber eCLIANumber = UseCLIANumber(m_pEBillingInfo->BillID,m_pEBillingInfo->InsuredPartyID,m_pEBillingInfo->ProviderID,m_pEBillingInfo->Box24J_ProviderID,m_pEBillingInfo->BillLocation);
			if(eCLIANumber.bUseCLIANumber && eCLIANumber.bUseCLIAInHCFABox32) {
				strFacilityID = eCLIANumber.strCLIANumber;
				//no qualifier
				strFacilityQual = "";
			}

			if(strFacilityID.IsEmpty())
				strFacilityQual = "";
		}

		// (j.jones 2007-05-15 12:24) - PLID 25953 - always send NPI and Facility ID,
		// but for facility ID, use the setting for QualifierSpace
		
		//NPI
		OutputString += ParseField(strPOSNPI,12,'L',' ');
		OutputString += ParseField("",3,'L',' ');

		//Facility ID
		//determine whether to use a space
		str.Format("%s%s%s", strFacilityQual, m_HCFAInfo.QualifierSpace == 1 ? " " : "", strFacilityID);
		OutputString += ParseField(str,15,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		// (j.jones 2007-05-15 13:47) - PLID 25953 - the bottom of Box 33 is now NPI and Box 33b, formerly the PIN

		//NPI

		// (j.jones 2006-11-16 09:07) - PLID 23563 - added LocationNPIUsage
		// option to determine whether to use the Location NPI or the
		// Provider's NPI

		// (j.jones 2007-01-18 11:46) - PLID 24264 - the LocationNPIUsage
		// used to be only available when using Bill Location, now it is
		// an option at all times

		if(m_HCFAInfo.LocationNPIUsage == 1) {
			// (j.jones 2008-05-06 15:02) - PLID 29937 - use the NPI from the loaded claim pointer
			str = m_pEBillingInfo->strBillLocationNPI;
		}
		else {
			str = AdoFldString(m_rsDoc, "NPI","");
		}

		// (j.jones 2007-08-08 10:43) - PLID 25395 - we need to check AdvHCFAPinT for the 33a NPI
		// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
		rsTemp = CreateParamRecordset("SELECT Box33aNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
		if(!rsTemp->eof) {
			var = rsTemp->Fields->Item["Box33aNPI"]->Value;
			if(var.vt == VT_BSTR) {
				CString strTemp = CString(var.bstrVal);
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty())
					str = strTemp;
			}
		}
		rsTemp->Close();

		OutputString += ParseField(str,20,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		//Box 33b
		CString strQual, strID;
		EBilling_Box33PinANSI(strQual, strID, m_pEBillingInfo->ProviderID, m_pEBillingInfo->HCFASetupID,
			m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->InsuredPartyID, m_pEBillingInfo->BillLocation);

		// (a.walling 2007-07-18 11:56) - PLID 26718 - Check for an override for Box33B
		// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
		rsTemp = CreateParamRecordset("SELECT Box33bQual, PIN FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",
			m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillLocation, m_pEBillingInfo->HCFASetupID);
		if(!rsTemp->eof) {
			CString strTemp = AdoFldString(rsTemp, "PIN","");
			strTemp.TrimLeft();
			strTemp.TrimRight();
			if(!strTemp.IsEmpty())
				strID = strTemp;

			strTemp = AdoFldString(rsTemp, "Box33bQual","");
			strTemp.TrimLeft();
			strTemp.TrimRight();
			if(!strTemp.IsEmpty())
				strQual = strTemp;
		}
		rsTemp->Close();

		//determine whether to use a space
		str.Format("%s%s%s", strQual, m_HCFAInfo.QualifierSpace == 1 ? " " : "", strID);

		OutputString += ParseField(str,20,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 29 ///////////////////////////////////

		rsPOS->Close();

		//Line 30 ///////////////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",61,'L',' ');

		//LocationsT.Phone (Location)
		
		// (j.jones 2007-02-20 15:17) - PLID 23935 - check to see if we should hide the phone number in Box 33
		if(m_HCFAInfo.HidePhoneBox33 == 1) {
			str = "";
		}
		else {
			if(m_HCFAInfo.Box33Setup == 3)
				var = rsLoc->Fields->Item["WorkPhone"]->Value;
			else if(m_HCFAInfo.Box33Setup == 4)
				var = rsLoc->Fields->Item["Phone"]->Value;
			else if(m_HCFAInfo.DocAddress == 1)
				var = rsLoc->Fields->Item["Phone"]->Value;
			else
				var = rsLoc->Fields->Item["WorkPhone"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
		}
		OutputString += ParseField(str,14,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 30 ////////////////////////////////////////////

		//Line 31 ///////////////////////////////////////////////////
		OutputString = "";

		str = "\r\n\r\n--------------NEXT CLAIM-----------------------";
		OutputString += ParseField(str,60,'L',' ');	

		OutputString+= "\r\n";
		//if(m_FormatID==INTERLINK)
		//	OutputString += "\f";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 31 ////////////////////////////////////////////

		rsLoc->Close();
		m_rsDoc->Close();

		return Success;

	}NxCatchAll("Error in Ebilling::HCFABoxes24to33");

	return Error_Other;
}

// (j.jones 2008-06-18 09:34) - PLID 30403 - CalculateBoxes29and30 will calculate the values
// for Box 29 and Box 30 based on the settings to ShowPays, IgnoreAdjustments, ExcludeAdjFromBal,
// and DoNotFillBox29. I made it a modular function because it is identical for all three image types.
void CEbilling::CalculateBoxes29and30(const COleCurrency cyCharges, COleCurrency &cyApplies, COleCurrency &cyTotal, CString strChargeIDs)
{
	//this should be impossible
	if(strChargeIDs.IsEmpty()) {
		ASSERT(FALSE);
		return;
	}

	//throw all exceptions to the caller

	if(m_pEBillingInfo->Box33Setup == 2) {

		// (j.jones 2008-06-18 09:34) - PLID 30403 - Added ability to not fill Box 29,
		// also ExcludeAdjFromBal can be checked independently of IgnoreAdjustments.
		// To do this I combined the two apply recordsets into one, and just
		// did the calculations all at once per the settings.

		if(m_HCFAInfo.ShowPays) {

			//calculate all the applies

			COleCurrency cyTotalApplies = COleCurrency(0,0);
			COleCurrency cyTotalPaysOnly = COleCurrency(0,0);

			// (j.jones 2007-10-05 10:56) - PLID 27659 - calculate only for the charges on this page
			//this cannot be parameterized
			// (j.jones 2011-08-16 17:22) - PLID 44805 - We will filter out "original" and "void" charges & applies.
			// The applies part of this does not need to check LineItemCorrectionsBalancingAdjT,
			// because they are only applied to original and void charges.
			_RecordsetPtr rsApplies = CreateRecordset("SELECT "
				"Sum(Round(Convert(money,CASE WHEN AppliesT.Amount Is NULL THEN 0 ELSE AppliesT.Amount END),2)) AS Total, "
				"Sum(Round(Convert(money,CASE WHEN AppliesT.Amount Is NULL OR LineItemT2.Type <> 1 THEN 0 ELSE AppliesT.Amount END),2)) AS TotalPays "
				"FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
				"LEFT JOIN LineItemT LineItemT2 ON AppliesT.SourceID = LineItemT2.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalCharges2Q ON LineItemT2.ID = LineItemCorrections_OriginalCharges2Q.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingCharges2Q ON LineItemT2.ID = LineItemCorrections_VoidingCharges2Q.VoidingLineItemID "
				"WHERE ChargesT.ID IN (%s) AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND LineItemCorrections_OriginalCharges2Q.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingCharges2Q.VoidingLineItemID Is Null",_Q(strChargeIDs));

			if(!rsApplies->eof) {
				cyTotalApplies = AdoFldCurrency(rsApplies, "Total",COleCurrency(0,0));
				cyTotalPaysOnly = AdoFldCurrency(rsApplies, "TotalPays",COleCurrency(0,0));
			}
			rsApplies->Close();

			if(m_HCFAInfo.DoNotFillBox29) {
				cyApplies = COleCurrency(0,0);
			}
			else if(m_HCFAInfo.IgnoreAdjustments) {
				cyApplies = cyTotalPaysOnly;
			}
			else {
				cyApplies = cyTotalApplies;
			}

			//set the total to equal charges - total applies (per our balance setting)			
			if(m_HCFAInfo.ExcludeAdjFromBal) {
				cyTotal = cyCharges - cyTotalPaysOnly;
			}
			else {
				cyTotal = cyCharges - cyTotalApplies;
			}
		}
		else {
			cyTotal = cyCharges;
			cyApplies = COleCurrency(0,0);
		}
	}
	else {

		// (j.jones 2008-06-18 09:34) - PLID 30403 - Added ability to not fill Box 29,
		// also ExcludeAdjFromBal can be checked independently of IgnoreAdjustments.
		// To do this I combined the two apply recordsets into one, and just
		// did the calculations all at once per the settings.

		if(m_HCFAInfo.ShowPays) {

			//calculate all the applies

			COleCurrency cyTotalApplies = COleCurrency(0,0);
			COleCurrency cyTotalPaysOnly = COleCurrency(0,0);

			// (j.jones 2006-12-01 15:50) - PLID 22110 - supported the ClaimProviderID
			// (j.jones 2007-10-05 10:56) - PLID 27659 - calculate only for the charges on this page
			// (j.jones 2008-04-03 09:23) - PLID 28995 - supported the HCFAClaimProvidersT setup
			// (j.jones 2008-08-01 10:17) - PLID 30917 - HCFAClaimProvidersT is now per insurance company
			//this cannot be parameterized
			// (j.jones 2010-11-09 12:57) - PLID 41389 - supported ChargesT.ClaimProviderID
			// (j.jones 2011-08-16 17:22) - PLID 44805 - We will filter out "original" and "void" charges & applies.
			// The applies part of this does not need to check LineItemCorrectionsBalancingAdjT,
			// because they are only applied to original and void charges.
			_RecordsetPtr rsApplies = CreateRecordset("SELECT "
				"Sum(Round(Convert(money,CASE WHEN AppliesT.Amount Is NULL THEN 0 ELSE AppliesT.Amount END),2)) AS Total, "
				"Sum(Round(Convert(money,CASE WHEN AppliesT.Amount Is NULL OR LineItemT2.Type <> 1 THEN 0 ELSE AppliesT.Amount END),2)) AS TotalPays "
				"FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
				"LEFT JOIN LineItemT LineItemT2 ON AppliesT.SourceID = LineItemT2.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalCharges2Q ON LineItemT2.ID = LineItemCorrections_OriginalCharges2Q.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingCharges2Q ON LineItemT2.ID = LineItemCorrections_VoidingCharges2Q.VoidingLineItemID "
				"WHERE ChargesT.ID IN (%s) AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND LineItemCorrections_OriginalCharges2Q.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingCharges2Q.VoidingLineItemID Is Null "

				"AND (ChargesT.ClaimProviderID = %li OR (ChargesT.ClaimProviderID Is Null AND ChargesT.DoctorsProviders IN (SELECT PersonID FROM "
				"	(SELECT ProvidersT.PersonID, "
				"	(CASE WHEN Box33_ProviderID Is Null THEN ProvidersT.ClaimProviderID ELSE Box33_ProviderID END) AS ProviderIDToUse "
				"	FROM ProvidersT "
				"	LEFT JOIN (SELECT * FROM HCFAClaimProvidersT WHERE InsuranceCoID = %li) AS HCFAClaimProvidersT ON ProvidersT.PersonID = HCFAClaimProvidersT.ProviderID) SubQ "
				"	WHERE ProviderIDToUse = %li))) "
				, _Q(strChargeIDs), m_pEBillingInfo->ProviderID, m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);

			if(!rsApplies->eof) {
				cyTotalApplies = AdoFldCurrency(rsApplies, "Total",COleCurrency(0,0));
				cyTotalPaysOnly = AdoFldCurrency(rsApplies, "TotalPays",COleCurrency(0,0));
			}
			rsApplies->Close();

			if(m_HCFAInfo.DoNotFillBox29) {
				cyApplies = COleCurrency(0,0);
			}
			else if(m_HCFAInfo.IgnoreAdjustments) {
				cyApplies = cyTotalPaysOnly;
			}
			else {
				cyApplies = cyTotalApplies;
			}

			//set the total to equal charges - total applies (per our balance setting)			
			if(m_HCFAInfo.ExcludeAdjFromBal) {
				cyTotal = cyCharges - cyTotalPaysOnly;
			}
			else {
				cyTotal = cyCharges - cyTotalApplies;
			}
		}
		else {
			cyTotal = cyCharges;
			cyApplies = COleCurrency(0,0);
		}
	}
}

// (j.jones 2007-05-15 09:11) - PLID 25953 - these "IntermediateNPI_" functions are for the interim
// image that is the legacy layout but with NPIs appended in the lower right

int CEbilling::IntermediateNPI_HCFABoxes1to11(_RecordsetPtr &rsBills)
{
	//this will export the insurance address, and all fields from box 1 to box 11, HCFA Image format

	try {
		//Line 1   ///////////////////////////////////////////
		CString str, OutputString = "";		

		// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name, Address1, Address2, City, State, Zip, IDForInsurance, "
						"CASE WHEN InsurancePlansT.PlanType='Champus' THEN 3 WHEN InsurancePlansT.PlanType='Champva' THEN 4 WHEN InsurancePlansT.PlanType='FECA Black Lung' THEN 5 WHEN InsurancePlansT.PlanType='Group Health Plan' THEN 6 WHEN InsurancePlansT.PlanType='Medicaid' THEN 2 WHEN InsurancePlansT.PlanType='Medicare' THEN 1 ELSE 7 END AS InsType "
						"FROM InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
						"INNER JOIN PersonT ON InsuranceCoT.PersonID = PersonT.ID LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
						"WHERE InsuredPartyT.PersonID = {INT}",m_pEBillingInfo->InsuredPartyID);
		if(rs->eof) {
			//if the recordset is empty, there is no insurance company. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open primary insurance information from this patient's bill for '%s', Bill ID %li. (IntermediateNPI_HCFABoxes1to11 1)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open primary insurance information from this patient's bill. (IntermediateNPI_HCFABoxes1to11 1)";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		_variant_t var;


		//check the secondary ins address as well
		// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
		// (j.jones 2010-08-31 17:02) - PLID 40303 - added TPLCode
		_RecordsetPtr rsOtherIns = CreateParamRecordset("SELECT Name, Address1, Address2, City, State, Zip, IDForInsurance, HCFASetupGroupID, InsuranceCoT.TPLCode, "
						"CASE WHEN InsurancePlansT.PlanType='Champus' THEN 3 WHEN InsurancePlansT.PlanType='Champva' THEN 4 WHEN InsurancePlansT.PlanType='FECA Black Lung' THEN 5 WHEN InsurancePlansT.PlanType='Group Health Plan' THEN 6 WHEN InsurancePlansT.PlanType='Medicaid' THEN 2 WHEN InsurancePlansT.PlanType='Medicare' THEN 1 ELSE 7 END AS InsType "
						"FROM InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
						"INNER JOIN PersonT ON InsuranceCoT.PersonID = PersonT.ID LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
						"WHERE InsuredPartyT.PersonID = {INT}",m_pEBillingInfo->OthrInsuredPartyID);

		OutputString += ParseField("",2,'L',' ');
		
		str = "";

		//use the secondary ins. name, if requested and available
		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "Name","");
		}

		OutputString += ParseField(str,40,'L',' ');
		OutputString += ParseField("",10,'L',' ');


		//InsuranceCoT.Name
		var = rs->Fields->Item["Name"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,50,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 1   ///////////////////////////////////

		//Line 2   //////////////////////////////////////////
		OutputString = "";
		
		OutputString += ParseField("",2,'L',' ');
		
		str = "";

		//use the secondary ins. address, if requested and available
		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "Address1","");
		}

		OutputString += ParseField(str,40,'L',' ');
		OutputString += ParseField("",10,'L',' ');
		
		//PersonT.Address1
		var = rs->Fields->Item["Address1"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 2   ///////////////////////////////////

		//Line 3   //////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",2,'L',' ');
		
		str = "";

		//use the secondary ins. address, if requested and available
		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "Address2","");
		}

		OutputString += ParseField(str,40,'L',' ');
		OutputString += ParseField("",10,'L',' ');
		
		//PersonT.Address2
		var = rs->Fields->Item["Address2"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 3   ///////////////////////////////////

		//Line 4   //////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",2,'L',' ');
		
		str = "";

		//use the secondary ins. address, if requested and available
		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "City","");
		}
		OutputString += ParseField(str,18,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "State","");
		}
		OutputString += ParseField(str,2,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "Zip","");
		}
		OutputString += ParseField(str,10,'L',' ');

		OutputString += ParseField("",18,'L',' ');
		
		//PersonT.City
		var = rs->Fields->Item["City"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,18,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.State
		var = rs->Fields->Item["State"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,2,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.Zip
		var = rs->Fields->Item["Zip"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 4   ///////////////////////////////////

		//Line 5 (double)  //////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",2,'L',' ');

		str = "";

		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1 && m_HCFAInfo.ShowSecPINNumber == 1) {
			str = Box33Pin(TRUE);
		}
		OutputString += ParseField(str,20,'L',' ');

		str = "";

		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1 && m_HCFAInfo.ShowSec24JNumber == 1) {
			//InsuranceBox24J.Box24JNumber
			// (j.jones 2008-04-03 10:12) - PLID 28995 - use the Box24J_ProviderID here for the current HCFA group,
			// even though the actual ID will pull from the secondary insurance
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Box24JNumber FROM InsuranceBox24J INNER JOIN InsuredPartyT ON InsuranceBox24J.InsCoID = InsuredPartyT.InsuranceCoID "
				"WHERE InsuredPartyT.PersonID = {INT} AND ProviderID = {INT}",
				m_pEBillingInfo->OthrInsuredPartyID,m_pEBillingInfo->Box24J_ProviderID);

			if(!rs->eof && rs->Fields->Item["Box24JNumber"]->Value.vt==VT_BSTR) {
				str = CString(rs->Fields->Item["Box24JNumber"]->Value.bstrVal);
			}
			else
				str = "";

			//check for the override
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			_RecordsetPtr rsTemp = CreateParamRecordset("SELECT Box24J FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->Box24J_ProviderID,m_pEBillingInfo->BillLocation,AdoFldLong(rsOtherIns, "HCFASetupGroupID",-1));
			if(!rsTemp->eof) {
				var = rsTemp->Fields->Item["Box24J"]->Value;
				if(var.vt == VT_BSTR) {
					CString strTemp = CString(var.bstrVal);
					strTemp.TrimLeft();
					strTemp.TrimRight();
					if(!strTemp.IsEmpty())
						str = strTemp;
				}
			}
			rsTemp->Close();
		}
		OutputString += ParseField(str,15,'L',' ');

		// (j.jones 2009-08-04 12:32) - PLID 14573 - used payer ID, though
		// we kept the same preference name internally
		if(GetRemotePropertyInt("HCFAImageShowTHINNumber",0,0,"<None>",true) == 1) {
			//show EbillingID number
			OutputString += ParseField("",15,'L',' ');
			OutputString += ParseField("ID#   ",6,'L',' ');
		
			//InsuranceCoT.HCFAPayerID
			// (j.jones 2009-08-05 10:20) - PLID 34467 - this is now in the loaded claim info
			// (j.jones 2009-12-16 15:54) - PLID 36237 - split out into HCFA and UB payer IDs
			OutputString += ParseField(m_pEBillingInfo->strHCFAPayerID,32,'L',' ');
		}
		
		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 5  //////////////////////////////

		//Line 6 (double)  //////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",2,'L',' ');	

		//based on InsurancePlansT.PlanType
		var = rs->Fields->Item["InsType"]->Value;
		if(var.vt==VT_I4) {
			int InsType = var.lVal;
			switch(InsType) {
			case 1:
				str = "X";
				break;
			case 2:
				str = "         X";
				break;
			case 3:
				str = "                  X";
				break;
			case 4:
				str = "                           X";
				break;
			case 5:
				str = "                                    X";
				break;
			case 6:
				str = "                                             X";
				break;
			case 7:
				str = "                                                      X";
				break;
			}
		}
		else
			str = "";
		OutputString += ParseField(str,57,'L',' ');

		//InsuredPartyT.IDForInsurance
		var = rs->Fields->Item["IDForInsurance"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');	

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 6   ///////////////////////////////////

		// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
		_RecordsetPtr rsPatient = CreateParamRecordset("SELECT * FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE PatientsT.PersonID = {INT}",m_pEBillingInfo->PatientID);

		if(rsPatient->eof) {
			//if the recordset is empty, there is no patient. So halt everything!!!
			rsPatient->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open patient information for '%s', Bill ID %li. (IntermediateNPI_HCFABoxes1to11)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open patient information. (IntermediateNPI_HCFABoxes1to11)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		//Line 7 (double)   //////////////////////////////////////////
		OutputString = "";

		//Patient Information

		//PersonT.Last
		var = rsPatient->Fields->Item["Last"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,16,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.First
		var = rsPatient->Fields->Item["First"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,13,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.Middle
		var = rsPatient->Fields->Item["Middle"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,1,'L',' ');
		OutputString += ParseField("",4,'L',' ');

		//PersonT.BirthDate
		//Box3
		var = rsPatient->Fields->Item["BirthDate"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			
			if(m_HCFAInfo.Box3E == 1)
				str = dt.Format("%Y%m%d");
			else
				str = dt.Format("%m%d%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",3,'L',' ');

		//PersonT.Gender
		var = rsPatient->Fields->Item["Gender"]->Value;
		int Gender = VarByte(var,0);
		if(Gender==1)
			str = "X";
		else if(Gender==2)
			str = "      X";
		else
			str = "";

		OutputString += ParseField(str,10,'L',' ');

		// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
		_RecordsetPtr rsIns = CreateParamRecordset("SELECT * FROM PersonT INNER JOIN InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID WHERE ID = {INT}",m_pEBillingInfo->InsuredPartyID);
		if(rsIns->eof) {
			//if the recordset is empty, there is no insured party. So halt everything!!!
			rsIns->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open primary insurance information from this patient's bill for '%s', Bill ID %li. (IntermediateNPI_HCFABoxes1to11 2)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open primary insurance information from this patient's bill. (IntermediateNPI_HCFABoxes1to11 2)";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		//Insured Party Information

		//PersonT.Last
		if(m_HCFAInfo.HideBox4 == 0) {
			var = rsIns->Fields->Item["Last"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,16,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.First
		if(m_HCFAInfo.HideBox4 == 0) {
			var = rsIns->Fields->Item["First"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else 
			str = "";
		OutputString += ParseField(str,13,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.Middle
		if(m_HCFAInfo.HideBox4 == 0) {
			var = rsIns->Fields->Item["Middle"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else 
			str = "";
		OutputString += ParseField(str,1,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 7   ///////////////////////////////////

		//Line 8 (double)   //////////////////////////////////////////
		OutputString = "";

		//Patient Information

		//PersonT.Address1
		var = rsPatient->Fields->Item["Address1"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');
		OutputString += ParseField("",4,'L',' ');

		//InsuredPartyT.RelationToPatient
		var = rsIns->Fields->Item["RelationToPatient"]->Value;
		if(var.vt==VT_BSTR) {
			str = CString(var.bstrVal);
			if(str=="Self")
				str="X";
			else if(str=="Spouse")
				str="       X";
			else if(str=="Child")
				str="             X";
			else
				str="                   X";
		}
		else
			str = "                   X";
		OutputString += ParseField(str,23,'L',' ');

		//Insurance Information

		//PersonT.Address1
		if(m_HCFAInfo.HideBox7 == 0) {
			var = rsIns->Fields->Item["Address1"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 8   ///////////////////////////////////


		//Line 9 (double)  /////////////////////////////////////////
		OutputString = "";
		
		//Patient Information

		//PersonT.City
		var = rsPatient->Fields->Item["City"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,28,'L',' ');
		OutputString += ParseField("",3,'L',' ');

		//PersonT.State
		var = rsPatient->Fields->Item["State"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,2,'L',' ');
		OutputString += ParseField("",7,'L',' ');

		//PatientsT.MaritalStatus
		var = rsPatient->Fields->Item["MaritalStatus"]->Value;
		if(var.vt==VT_BSTR) {
			str = CString(var.bstrVal);
			if(str=="1")
				str="X";
			else if(str=="2")
				str="        X";
			else
				str="                X";
		}
		else
			str = "                X";
		OutputString += ParseField(str,19,'L',' ');

		//Insurance Information

		//PersonT.City
		if(m_HCFAInfo.HideBox7 == 0) {
			var = rsIns->Fields->Item["City"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,18,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		//PersonT.State
		if(m_HCFAInfo.HideBox7 == 0) {
			var = rsIns->Fields->Item["State"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else 
			str = "";
		OutputString += ParseField(str,2,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 9   //////////////////////////////////

		//Line 10 (double)   /////////////////////////////////////////
		OutputString = "";
		
		//Patient Information

		//PersonT.Zip
		var = rsPatient->Fields->Item["Zip"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,15,'L',' ');
		OutputString += ParseField("",3,'L',' ');

		//PersonT.HomePhone
		var = rsPatient->Fields->Item["HomePhone"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str = StripNonNum(str);
		OutputString += ParseField(str,15,'L',' ');
		OutputString += ParseField("",7,'L',' ');

		//PatientsT.Employment
		var = rsPatient->Fields->Item["Employment"]->Value;
		if(var.vt==VT_I4) {
			long emp = var.lVal;
			if(emp==1)
				str="X";
			else if(emp==2)
				str="        X";
			else if(emp==3)
				str="               X";
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,19,'L',' ');

		//Insurance Information

		//PersonT.Zip
		if(m_HCFAInfo.HideBox7 == 0) {
			var = rsIns->Fields->Item["Zip"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,16,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		//PersonT.HomePhone
		if(m_HCFAInfo.HideBox7 == 0) {
			var = rsIns->Fields->Item["HomePhone"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		str = StripNonNum(str);
		OutputString += ParseField(str,15,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 10   //////////////////////////////////


		//Line 11 (double)   /////////////////////////////////////////
		OutputString = "";
		
		_RecordsetPtr rsOthrIns;

		if(m_pEBillingInfo->OthrInsuredPartyID!=-1) {
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			rsOthrIns = CreateParamRecordset("SELECT * FROM PersonT INNER JOIN InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID WHERE ID = {INT}",m_pEBillingInfo->OthrInsuredPartyID);
			if(rsOthrIns->eof) {
				//if the recordset is empty, there is no insured party. So halt everything!!!
				rsOthrIns->Close();
				// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
				CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
				if(!strPatientName.IsEmpty()) {
					// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
					str.Format("Could not open secondary insurance information from this patient's bill for '%s', Bill ID %li. (IntermediateNPI_HCFABoxes1to11)", strPatientName, m_pEBillingInfo->BillID);
				}
				else
					//serious problems if you get here
					str = "Could not open secondary insurance information from this patient's bill. (IntermediateNPI_HCFABoxes1to11)";
				
				AfxMessageBox(str);
				return Error_Missing_Info;
			}
		}

		//Other Insurance Information

		//PersonT.Last
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1 && rsOthrIns->Fields->Item["Last"]->Value.vt==VT_BSTR)
			str = CString(rsOthrIns->Fields->Item["Last"]->Value.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,16,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.First
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1 && rsOthrIns->Fields->Item["First"]->Value.vt==VT_BSTR)
			str = CString(rsOthrIns->Fields->Item["First"]->Value.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,13,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.Middle
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1 && rsOthrIns->Fields->Item["Middle"]->Value.vt==VT_BSTR)
			str = CString(rsOthrIns->Fields->Item["Middle"]->Value.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,1,'L',' ');
		OutputString += ParseField("",27,'L',' ');

		//InsuredPartyT.PolicyGroupNum (Primary Ins.)

		str = "";

		// (j.jones 2006-10-31 15:49) - PLID 21845 - check the secondary insurance's HCFA group,
		// and see if we need to use the secondary insurance's information in Box 11. If so,
		// this will override the Hide Box 11 option
		BOOL bShowSecondaryInBox11 = FALSE;
		if(m_pEBillingInfo->OthrInsuredPartyID != -1) {
			if(ReturnsRecords("SELECT ID FROM HCFASetupT WHERE SecondaryFillBox11 = 1 AND "
				"ID IN (SELECT HCFASetupGroupID FROM InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
				"WHERE InsuredPartyT.PersonID = %li)", m_pEBillingInfo->OthrInsuredPartyID)) {
				//there is a secondary insurance and its HCFA group says to overwrite Box 11
				bShowSecondaryInBox11 = TRUE;
			}
		}

		if(bShowSecondaryInBox11) {

			//if we are showing the secondary information in Box 11, that takes
			//precedence over all other Box 11 possibilities
			str = AdoFldString(rsOthrIns, "IDForInsurance", "");
		}
		else {

			//check for the HCFA group's override
  			BOOL bOverride = FALSE;

			// (j.jones 2006-03-13 14:44) - PLID 19555 - the Box 11 default overrides
			// the "Hide Box 11" options. If they want Box 11 to be truly blank, then they
			// should have it hidden and have no override.
			if(m_HCFAInfo.Box11Rule == 1) {
				bOverride = TRUE;
			}
			else {			
				//use only if blank (default)
				var = rsIns->Fields->Item["PolicyGroupNum"]->Value;
				if(var.vt == VT_BSTR) {
					str = CString(var.bstrVal);
					str.TrimLeft(" ");
					str.TrimRight(" ");
					if(str.GetLength()==0)
						bOverride = TRUE;
				}
				else
					bOverride = TRUE;
			}

			if(bOverride) {
				str = m_HCFAInfo.Box11;
			}
			else if(m_HCFAInfo.HideBox11 == 1) {
				str = "";
			}
		}

		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 11   //////////////////////////////////

		//Line 12 (double)   /////////////////////////////////////////
		OutputString = "";

		//Other Insurance Information

		//InsuredPartyT.IDForInsurance
		if(m_HCFAInfo.Use1aIn9a && (m_pEBillingInfo->OthrInsuredPartyID != -1 || m_HCFAInfo.Use1aIn9aAlways)) {
			//InsuredPartyT.IDForInsurance
			var = rs->Fields->Item["IDForInsurance"]->Value;
			str = VarString(var, "");
		}
		// (j.jones 2010-08-31 16:54) - PLID 40303 - supported TPL in 9a, which is mutually exclusive of Use1aIn9a
		else if (!m_HCFAInfo.Use1aIn9a && m_HCFAInfo.TPLIn9a == 1 && m_pEBillingInfo->OthrInsuredPartyID != -1) {
			str = AdoFldString(rsOtherIns, "TPLCode", "");
		}
		else {
			if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID != -1 && rsOthrIns->Fields->Item["IDForInsurance"]->Value.vt==VT_BSTR)
				str = CString(rsOthrIns->Fields->Item["IDForInsurance"]->Value.bstrVal);
			else
				str = "";
		}
		OutputString += ParseField(str,32,'L',' ');
		OutputString += ParseField("",8,'L',' ');

		rs->Close();

		if(m_pEBillingInfo->Box33Setup == 2) {
			//since 2 is the general 1 provider, we do not separate charges at all
			// (j.jones 2007-10-05 10:50) - PLID 27659 - added ChargeID as a field
			// (j.jones 2007-10-15 15:03) - PLID 27757 - added ServiceID as a field
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			// (j.jones 2009-03-09 09:10) - PLID 33354 - added insured party accident information
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			// (j.gruber 2014-03-17 11:28) - PLID 61395 - support new billing structure for HCFA image
			// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - DiagCodes and WhichCodes must be unique and filtered for NULLs. Use ChargeWhichICD9CodesIF/10 and the BillDiags
			// (j.jones 2016-04-11 17:22) - NX-100149 - Box15Date is now calculated from ConditionDateType
			rsBills = CreateParamRecordset("SELECT GetDate() AS TodaysDate, ChargesT.ID AS ChargeID, ChargesT.ServiceID, BillsT.*, ChargesT.*, LineItemT.*, dbo.GetChargeTotal(ChargesT.ID) AS ChargeTot, "
									"PlaceOfServiceCodesT.PlaceCodes, "
									"ServiceT.Anesthesia, ServiceT.UseAnesthesiaBilling, InsuredPartyT.AccidentType, InsuredPartyT.AccidentState, InsuredPartyT.DateOfCurAcc, ChargeWhichCodesQ.WhichCodes as ChargeWhichCodes, "
									"CASE BillsT.ConditionDateType "
									"WHEN {CONST_INT} THEN BillsT.FirstVisitOrConsultationDate "
									"WHEN {CONST_INT} THEN BillsT.InitialTreatmentDate "
									"WHEN {CONST_INT} THEN BillsT.LastSeenDate "
									"WHEN {CONST_INT} THEN BillsT.AcuteManifestationDate "
									"WHEN {CONST_INT} THEN BillsT.LastXRayDate "
									"WHEN {CONST_INT} THEN BillsT.HearingAndPrescriptionDate "
									"WHEN {CONST_INT} THEN BillsT.AssumedCareDate "
									"WHEN {CONST_INT} THEN BillsT.RelinquishedCareDate "
									"WHEN {CONST_INT} THEN BillsT.AccidentDate "
									"ELSE NULL END AS Box15Date "
									"FROM BillsT LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
									"INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
									"LEFT JOIN CPTModifierT ON CPTModifier = CPTModifierT.Number "
									"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
									"OUTER APPLY dbo.{CONST_STRING}({STRING}, ChargesT.ID) ChargeWhichCodesQ "
									"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
									"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
									"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
									"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
									"WHERE BillsT.ID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
									"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
									"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
									"ORDER BY LineID",
									ConditionDateType::cdtFirstVisitOrConsultation444,
									ConditionDateType::cdtInitialTreatmentDate454,
									ConditionDateType::cdtLastSeenDate304,
									ConditionDateType::cdtAcuteManifestation453,
									ConditionDateType::cdtLastXray455,
									ConditionDateType::cdtHearingAndPrescription471,
									ConditionDateType::cdtAssumedCare090,
									ConditionDateType::cdtRelinquishedCare91,
									ConditionDateType::cdtAccident439
									, ShouldUseICD10() ? "ChargeWhichICD10CodesIF" : "ChargeWhichICD9CodesIF"
									, MakeBillDiagsXml(m_pEBillingInfo->billDiags)
									,m_pEBillingInfo->BillID);
		}
		else {
			//separate the charges per doctor - the bill should already be separated in the loaded claims

			// (j.jones 2006-12-01 15:50) - PLID 22110 - supported the ClaimProviderID
			// (j.jones 2007-10-05 10:50) - PLID 27659 - added ChargeID as a field
			// (j.jones 2007-10-15 15:03) - PLID 27757 - added ServiceID as a field
			// (j.jones 2008-04-03 09:23) - PLID 28995 - supported the HCFAClaimProvidersT setup
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			// (j.jones 2008-08-01 10:17) - PLID 30917 - HCFAClaimProvidersT is now per insurance company
			// (j.jones 2009-03-09 09:10) - PLID 33354 - added insured party accident information
			// (j.jones 2010-11-09 12:57) - PLID 41389 - supported ChargesT.ClaimProviderID
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			// (j.gruber 2014-03-17 11:31) - PLID 61395  update Image for new billing structure
			// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - DiagCodes and WhichCodes must be unique and filtered for NULLs. Use ChargeWhichICD9CodesIF/10 and the BillDiags
			// (j.jones 2016-04-11 17:22) - NX-100149 - Box15Date is now calculated from ConditionDateType
			rsBills = CreateParamRecordset("SELECT GetDate() AS TodaysDate, ChargesT.ID AS ChargeID, ChargesT.ServiceID, BillsT.*, ChargesT.*, LineItemT.*, dbo.GetChargeTotal(ChargesT.ID) AS ChargeTot, "
									"PlaceOfServiceCodesT.PlaceCodes, "
									"ServiceT.Anesthesia, ServiceT.UseAnesthesiaBilling, InsuredPartyT.AccidentType, InsuredPartyT.AccidentState, InsuredPartyT.DateOfCurAcc, ChargeWhichCodesQ.WhichCodes as ChargeWhichCodes, "
									"CASE BillsT.ConditionDateType "
									"WHEN {CONST_INT} THEN BillsT.FirstVisitOrConsultationDate "
									"WHEN {CONST_INT} THEN BillsT.InitialTreatmentDate "
									"WHEN {CONST_INT} THEN BillsT.LastSeenDate "
									"WHEN {CONST_INT} THEN BillsT.AcuteManifestationDate "
									"WHEN {CONST_INT} THEN BillsT.LastXRayDate "
									"WHEN {CONST_INT} THEN BillsT.HearingAndPrescriptionDate "
									"WHEN {CONST_INT} THEN BillsT.AssumedCareDate "
									"WHEN {CONST_INT} THEN BillsT.RelinquishedCareDate "
									"WHEN {CONST_INT} THEN BillsT.AccidentDate "
									"ELSE NULL END AS Box15Date "
									"FROM BillsT LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
									"INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
									"LEFT JOIN CPTModifierT ON CPTModifier = CPTModifierT.Number "
									"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
									"OUTER APPLY dbo.{CONST_STRING}({STRING}, ChargesT.ID) ChargeWhichCodesQ "
									"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
									"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
									"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
									"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
									"WHERE BillsT.ID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
									"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
									"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
									"AND (ChargesT.ClaimProviderID = {INT} OR (ChargesT.ClaimProviderID Is Null AND ChargesT.DoctorsProviders IN (SELECT PersonID FROM "
									"	(SELECT ProvidersT.PersonID, "
									"	(CASE WHEN Box33_ProviderID Is Null THEN ProvidersT.ClaimProviderID ELSE Box33_ProviderID END) AS ProviderIDToUse "
									"	FROM ProvidersT "
									"	LEFT JOIN (SELECT * FROM HCFAClaimProvidersT WHERE InsuranceCoID = {INT}) AS HCFAClaimProvidersT ON ProvidersT.PersonID = HCFAClaimProvidersT.ProviderID) SubQ "
									"	WHERE ProviderIDToUse = {INT}))) "
									"ORDER BY LineID",
									ConditionDateType::cdtFirstVisitOrConsultation444,
									ConditionDateType::cdtInitialTreatmentDate454,
									ConditionDateType::cdtLastSeenDate304,
									ConditionDateType::cdtAcuteManifestation453,
									ConditionDateType::cdtLastXray455,
									ConditionDateType::cdtHearingAndPrescription471,
									ConditionDateType::cdtAssumedCare090,
									ConditionDateType::cdtRelinquishedCare91,
									ConditionDateType::cdtAccident439
									, ShouldUseICD10() ? "ChargeWhichICD10CodesIF" : "ChargeWhichICD9CodesIF"
									, MakeBillDiagsXml(m_pEBillingInfo->billDiags)
									, m_pEBillingInfo->BillID, m_pEBillingInfo->ProviderID, m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);
		}

		if(rsBills->eof) {
			//if the recordset is empty, there is no bill information. So halt everything!!!
			rsBills->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open bill information for '%s', Bill ID %li. (IntermediateNPI_HCFABoxes1to11)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open bill information. (IntermediateNPI_HCFABoxes1to11)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		// (j.jones 2009-03-09 09:11) - PLID 33354 - we now may use the insured party accident type instead
		InsuredPartyAccidentType ipatAccType = (InsuredPartyAccidentType)AdoFldLong(rsBills, "AccidentType", (long)ipatNone);		

		BOOL bRelatedToEmp = FALSE;
		
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//pull from the insured party, not the bill
			bRelatedToEmp = (ipatAccType == ipatEmployment);
		}
		else {
			//pull from BillsT.RelatedToEmp
			bRelatedToEmp = AdoFldBool(rsBills, "RelatedToEmp", FALSE);
		}

		if(bRelatedToEmp) {
			str = "X";
		}
		else {
			str = "        X";
		}
		OutputString += ParseField(str,21,'L',' ');

		//Insurance Information

		str = "";
		
		//PersonT.BirthDate
		if(bShowSecondaryInBox11) {

			// (j.jones 2006-10-31 15:49) - PLID 21845 - if we are showing the secondary
			//information in Box 11, that takes precedence over all other Box 11 possibilities
			var = rsOthrIns->Fields->Item["BirthDate"]->Value;
			if(var.vt==VT_DATE) {
				COleDateTime dt;
				dt = var.date;

				if(m_HCFAInfo.Box11aE == 1)
					str = dt.Format("%Y%m%d");
				else
					str = dt.Format("%m%d%Y");
			}
			else
				str = "";
		}
		else {

			//Box11a

			// (j.jones 2007-04-09 17:29) - PLID 25537 - supported hiding Box11a Birthdate individually
			if(m_HCFAInfo.HideBox11a_Birthdate == 0) {
				var = rsIns->Fields->Item["BirthDate"]->Value;
				if(var.vt==VT_DATE) {
					COleDateTime dt;
					dt = var.date;

					if(m_HCFAInfo.Box11aE == 1)
						str = dt.Format("%Y%m%d");
					else
						str = dt.Format("%m%d%Y");
				}
				else
					str = "";
			}
			else
				str = "";
		}
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",9,'L',' ');

		//PersonT.Gender
		if(bShowSecondaryInBox11) {

			// (j.jones 2006-10-31 15:49) - PLID 21845 - if we are showing the secondary
			//information in Box 11, that takes precedence over all other Box 11 possibilities
			var = rsOthrIns->Fields->Item["Gender"]->Value;				
			Gender = VarByte(var,0);
			if(Gender==1)
				str = "X";
			else if(Gender==2)
				str = "        X";
			else
				str = "";
		}
		else {
			// (j.jones 2007-04-09 17:29) - PLID 25537 - supported hiding Box11a Gender individually
			if(m_HCFAInfo.HideBox11a_Gender == 0) {
				var = rsIns->Fields->Item["Gender"]->Value;
				
				Gender = VarByte(var,0);
				if(Gender==1)
					str = "X";
				else if(Gender==2)
					str = "        X";
				else
					str = "";
			}
			else
				str = "";
		}
		OutputString += ParseField(str,9,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 12   //////////////////////////////////

		//Line 13 (double)   /////////////////////////////////////////
		OutputString = "";

		//Other Insurance Information

		//PersonT.BirthDate
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1 && rsOthrIns->Fields->Item["BirthDate"]->Value.vt==VT_DATE) {
			COleDateTime dt;
			dt = rsOthrIns->Fields->Item["BirthDate"]->Value.date;
			
			if(m_HCFAInfo.Box9bE == 1)
				str = dt.Format("%Y%m%d");
			else
				str = dt.Format("%m%d%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",9,'L',' ');

		//PersonT.Gender
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1) {
			int Gender = AdoFldByte(rsOthrIns, "Gender",0);
			if(Gender==1)
				str = "X";
			else if(Gender==2)
				str = "        X";
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,21,'L',' ');

		BOOL bRelatedToAutoAcc = FALSE;
		
		// (j.jones 2009-03-09 09:30) - PLID 33354 - support the insured party accident option
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//pull from the insured party, not the bill
			bRelatedToAutoAcc = (ipatAccType == ipatAutoAcc);
		}
		else {
			//pull from BillsT.RelatedToAutoAcc
			bRelatedToAutoAcc = AdoFldBool(rsBills, "RelatedToAutoAcc", FALSE);
		}

		if(bRelatedToAutoAcc) {
			str = "X";
		}
		else {
			str = "        X";
		}
		OutputString += ParseField(str,12,'L',' ');

		//BillsT.State
		CString strState = "";

		// (j.jones 2009-03-09 09:30) - PLID 33354 - support the insured party accident option
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//pull from the insured party, not the bill
			strState = AdoFldString(rsBills, "AccidentState", "");
		}
		else {
			//pull from BillsT.State
			strState = AdoFldString(rsBills, "State", "");
		}

		OutputString += ParseField(strState,2,'L',' ');
		OutputString += ParseField("",5,'L',' ');

		//Insurance Information

		//InsuredPartyT.Employer
		if(bShowSecondaryInBox11) {

			// (j.jones 2006-10-31 15:49) - PLID 21845 - if we are showing the secondary
			//information in Box 11, that takes precedence over all other Box 11 possibilities
			str = AdoFldString(rsOthrIns, "Employer","");
		}
		else {
			// (j.jones 2007-04-09 17:29) - PLID 25537 - supported hiding Box11b individually
			if(m_HCFAInfo.HideBox11b == 0) {
				var = rsIns->Fields->Item["Employer"]->Value;
				if(var.vt==VT_BSTR) {
					str = CString(var.bstrVal);
				}
				else
					str = "";
			}
			else
				str = "";
		}
		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 13   //////////////////////////////////


		//Line 14 (double)   /////////////////////////////////////////
		OutputString = "";

		//Other Insurance Information

		//InsuredPartyT.Employer
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1 && rsOthrIns->Fields->Item["Employer"]->Value.vt==VT_BSTR) {
			str = CString(rsOthrIns->Fields->Item["Employer"]->Value.bstrVal);
		}
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');
		OutputString += ParseField("",8,'L',' ');

		BOOL bRelatedToOther = FALSE;
		
		// (j.jones 2009-03-09 09:30) - PLID 33354 - support the insured party accident option
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//pull from the insured party, not the bill
			bRelatedToOther = (ipatAccType == ipatOtherAcc);
		}
		else {
			//pull from BillsT.RelatedToOther
			bRelatedToOther = AdoFldBool(rsBills, "RelatedToOther", FALSE);
		}

		if(bRelatedToOther) {
			str = "X";
		}
		else {
			str = "        X";
		}
		OutputString += ParseField(str,19,'L',' ');

		//InsurancePlansT.PlanName
		if(bShowSecondaryInBox11) {

			// (j.jones 2006-10-31 15:49) - PLID 21845 - if we are showing the secondary
			//information in Box 11, that takes precedence over all other Box 11 possibilities
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			_RecordsetPtr rsInsPlan = CreateParamRecordset("SELECT PlanName FROM InsurancePlansT WHERE InsurancePlansT.ID = {INT}", AdoFldLong(rsOthrIns, "InsPlan", -1));
			if(!rsInsPlan->eof){
				str = AdoFldString(rsInsPlan, "PlanName", "");
			}
			else{
				str = "";
			}
			rsInsPlan->Close();
		}
		else {
			// (j.jones 2007-04-09 17:29) - PLID 25537 - supported hiding Box11c individually
			if(m_HCFAInfo.HideBox11c == 0) {
				// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
				_RecordsetPtr rsInsPlan = CreateParamRecordset("SELECT PlanName FROM InsurancePlansT WHERE InsurancePlansT.ID = {INT}", AdoFldLong(rsIns, "InsPlan", -1));
				if(!rsInsPlan->eof){
					str = AdoFldString(rsInsPlan, "PlanName", "");
				}
				else{
					str = "";
				}
				rsInsPlan->Close();
			}
			else
				str = "";
		}
		OutputString += ParseField(str,32,'L',' ');


		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 14   //////////////////////////////////


		//Line 15 (triple)  /////////////////////////////////////////
		OutputString = "";

		//Other Ins Plan
		//InsurancePlansT.PlanName
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1){
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			_RecordsetPtr rsOtherInsPlan = CreateParamRecordset("SELECT PlanName FROM InsurancePlansT WHERE InsurancePlansT.ID = {INT}", AdoFldLong(rsOthrIns, "InsPlan", -1));
			if(!rsOtherInsPlan->eof){
				str = AdoFldString(rsOtherInsPlan, "PlanName", "");
			}
			else{
				str = "";
			}
			rsOtherInsPlan->Close();
			rsOthrIns->Close();
		}
		else{
			str = "";
		}
		OutputString += ParseField(str,36,'L',' ');

		//BillsT.HCFABox10D
		str = AdoFldString(rsBills, "HCFABox10D","");
		OutputString += ParseField(str,25,'L',' ');

		rsIns->Close();

		//Other Ins

		//Box11D: 0 = fill normally, 1 = fill if no secondary, 2 = never fill
		if(m_HCFAInfo.Box11D == 2) {
			str = "";
		}
		else {
			if(m_pEBillingInfo->OthrInsuredPartyID != -1){
				str = "X";
			}
			else{
				if(m_HCFAInfo.Box11D == 0) {
					str = "       X";
				}
				else {
					str = "";
				}
			}
		}
		OutputString += ParseField(str,8,'L',' ');

		OutputString+= "\r\n\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 15 (triple)  //////////////////////////////////

		return Success;

	}NxCatchAll("Error in Ebilling::IntermediateNPI_HCFABoxes1to11");

	return Error_Other;
}

int CEbilling::IntermediateNPI_HCFABoxes12to23(_RecordsetPtr &rsBills)
{
	//this exports fields 12 to 23, HCFA Image format

	try {

		_variant_t var;

		//Line 16 (double)  /////////////////////////////////////////
		CString str, OutputString = "";
		OutputString += ParseField("",9,'L',' ');

		// (j.jones 2010-07-23 15:02) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsCo now
		BOOL bAccepted = GetAcceptAssignment_ByInsCo(m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);

		// (j.jones 2010-07-23 10:52) - PLID 39795 - fixed to follow the HCFA group setting for filling Box 12
		BOOL bFillBox12 = TRUE;
		if(m_HCFAInfo.Box12Accepted == 0) {
			//never fill
			bFillBox12 = FALSE;
		}
		else if(m_HCFAInfo.Box12Accepted == 1) {
			//fill if accepted
			bFillBox12 = bAccepted;
		}
		else if(m_HCFAInfo.Box12Accepted == 2) {
			//fill if not accepted
			bFillBox12 = !bAccepted;
		}
		else if(m_HCFAInfo.Box12Accepted == 3) {
			//always fill
			bFillBox12 = TRUE;
		}

		//Box 12
		if(bFillBox12) {
			str = "SIGNATURE ON FILE";
		}
		else {
			str = "";
		}
		OutputString += ParseField(str,25,'L',' ');
		OutputString += ParseField("",14,'L',' ');

		//BillsT.Date
		//0 - the Print Date (today), 1 - the Bill Date
		if(!bFillBox12) {
			str = "";
		}
		else {
			if(m_HCFAInfo.Box12UseDate == 0)
				var = rsBills->Fields->Item["TodaysDate"]->Value;			
			else
				var = rsBills->Fields->Item["Date"]->Value;
			if(var.vt==VT_DATE) {
				COleDateTime dt;
				dt = var.date;
				str = dt.Format("%m/%d/%Y");
			}
			else
				str = "";
		}
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",10,'L',' ');

		// (j.jones 2010-07-22 12:01) - PLID 39780 - fixed to follow the HCFA group setting for filling Box 13
		BOOL bFillBox13 = TRUE;
		if(m_HCFAInfo.Box13Accepted == 0) {
			//never fill
			bFillBox13 = FALSE;
		}
		else if(m_HCFAInfo.Box13Accepted == 1) {
			//fill if accepted
			bFillBox13 = bAccepted;
		}
		else if(m_HCFAInfo.Box13Accepted == 2) {
			//fill if not accepted
			bFillBox13 = !bAccepted;
		}
		else if(m_HCFAInfo.Box13Accepted == 3) {
			//always fill
			bFillBox13 = TRUE;
		}

		// (j.jones 2010-06-10 12:53) - PLID 39095 - This is Box 13. We will output "SIGNATURE ON FILE" if the provider accepts assignment,
		// and have a per-bill setting to optionally override this to be blank or not.
		HCFABox13Over hb13Value = (HCFABox13Over)AdoFldLong(rsBills, "HCFABox13Over", (long)hb13_UseDefault);		
		if(hb13Value == hb13_No || (hb13Value == hb13_UseDefault && !bFillBox13)) {
			str = "";
		}
		else {
			str = "SIGNATURE ON FILE";
		}
		//Box 13
		OutputString += ParseField(str,21,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 16 (double)  /////////////////////////////////

		//Line 17 (double) /////////////////////////////////////////
		OutputString = "";

		//BillsT.ConditionDate
		
		// (j.jones 2009-03-09 09:39) - PLID 33354 - supported the option to load the accident from the insured party		
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//load from the insured party
			var = rsBills->Fields->Item["DateOfCurAcc"]->Value;
		}
		else {
			//load from the bill
			var = rsBills->Fields->Item["ConditionDate"]->Value;
		}

		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",36,'L',' ');

		// (j.jones 2016-04-11 17:22) - NX-100149 - this is now the calculated Box 15 date,
		// based on the value of BillsT.ConditionDateType
		var = rsBills->Fields->Item["Box15Date"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",7,'L',' ');

		//BillsT.NoWorkFrom
		var = rsBills->Fields->Item["NoWorkFrom"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",6,'L',' ');

		//BillsT.NoWorkTo
		var = rsBills->Fields->Item["NoWorkTo"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 17 (double) //////////////////////////////////

		//Line 18 (double) /////////////////////////////////////////
		OutputString = "";

		// (j.jones 2009-02-06 16:29) - PLID 32951 - this function now requires the bill location
		// (j.jones 2009-02-23 10:54) - PLID 33167 - now we require the billing provider ID and rendering provider ID,
		// in this case the Rendering Provider ID is the Box 24J provider ID
		// (j.jones 2011-04-05 13:39) - PLID 42372 - this now returns a struct of information
		ECLIANumber eCLIANumber = UseCLIANumber(m_pEBillingInfo->BillID,m_pEBillingInfo->InsuredPartyID,m_pEBillingInfo->ProviderID,m_pEBillingInfo->Box24J_ProviderID, m_pEBillingInfo->BillLocation);

		_RecordsetPtr rsRefPhy;

		BOOL bUseProvAsRefPhy = (eCLIANumber.bUseCLIANumber && eCLIANumber.bCLIAUseBillProvInHCFABox17);

		if(bUseProvAsRefPhy) {			
			//use the provider info instead of the referring physician
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			rsRefPhy = CreateParamRecordset("SELECT [Last] + ', ' + [First] + ' ' + Middle AS NameLFM, [First] + ' ' + [Middle] + ' ' + Last AS NameFML, ProvidersT.* FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
				"WHERE PersonID = {INT}",m_pEBillingInfo->ProviderID);
		}
		else {
			//use the referring physician normally
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			rsRefPhy = CreateParamRecordset("SELECT [Last] + ', ' + [First] + ' ' + Middle AS NameLFM, [First] + ' ' + [Middle] + ' ' + Last AS NameFML, ReferringPhysT.* FROM PersonT INNER JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
				"INNER JOIN BillsT ON PersonT.ID = BillsT.RefPhyID WHERE BillsT.ID = {INT}",m_pEBillingInfo->BillID);
		}

		//Referring Physician Info

		//PersonT.Last, First Middle
		str = "";
		if(!rsRefPhy->eof) {
			if(m_HCFAInfo.Box17Order == 1)
				str = AdoFldString(rsRefPhy, "NameLFM","");
			else
				str = AdoFldString(rsRefPhy, "NameFML","");
		}
		OutputString += ParseField(str,22,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		//ReferringPhyID
		//this is the ID specified by the setup of Box17

		if(bUseProvAsRefPhy) {
			
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			m_rsDoc = CreateParamRecordset("SELECT * FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE ProvidersT.PersonID = {INT}",m_pEBillingInfo->ProviderID);

			if(m_rsDoc->eof) {
				//if the recordset is empty, there is no doctor. So halt everything!!!
				m_rsDoc->Close();
				// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
				CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
				if(!strPatientName.IsEmpty()) {
					// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
					str.Format("Could not open provider information for patient '%s', Bill ID %li. (IntermediateNPI_HCFABoxes12to23)", strPatientName, m_pEBillingInfo->BillID);
				}
				else
					//serious problems if you get here
					str = "Could not open provider information. (IntermediateNPI_HCFABoxes12to23)";

				if(m_pEBillingInfo->Box33Setup==2)
					str += "\nIt is possible that you have no provider selected in General 1 for this patient.";
				else
					str += "\nIt is possible that you have no provider selected on this patient's bill.";
				
				AfxMessageBox(str);
				return Error_Missing_Info;
			}

			//provider
			str = Box33Pin();

			m_rsDoc->Close();
		}
		else {

			if(m_HCFAInfo.UseBox23InBox17a == 0) {
				//ref phy ID (normal case)
				str = Box17a();
			}
			else {
				//if UseBox23InBox17a = 1 or 2 then we use the Prior Auth Num from Box 23
				//BillsT.PriorAuthNum
				var = rsBills->Fields->Item["PriorAuthNum"]->Value;
				if(var.vt == VT_BSTR){
					str = CString(var.bstrVal);
				}
				else{
					str = "";
				}

				//use the override from the HCFA setup
				if(str == "")
					str = m_HCFAInfo.DefaultPriorAuthNum;
			}
		}

		OutputString += ParseField(str,14,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		// (j.jones 2006-11-10 09:42) - PLID 22668 - now we send the ref phy NPI
		
		//send the NPI

		str = "";
		// (j.jones 2009-01-06 10:00) - PLID 32614 - supported hiding Box 17b NPI
		if(m_HCFAInfo.HideBox17b == 0) {
			if(!rsRefPhy->eof) {
				str = AdoFldString(rsRefPhy, "NPI","");
			}

			// (j.jones 2007-08-08 10:43) - PLID 25395 - if using the provider as referring physician, we
			// need to check AdvHCFAPinT for their 24J NPI
			if(bUseProvAsRefPhy) {
				// (j.jones 2008-04-03 10:12) - PLID 28995 - continue to use the main ProviderID here
				// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
				_RecordsetPtr rsTemp = CreateParamRecordset("SELECT Box24JNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
				if(!rsTemp->eof) {
					var = rsTemp->Fields->Item["Box24JNPI"]->Value;
					if(var.vt == VT_BSTR) {
						CString strTemp = CString(var.bstrVal);
						strTemp.TrimLeft();
						strTemp.TrimRight();
						if(!strTemp.IsEmpty())
							str = strTemp;
					}
				}
				rsTemp->Close();
			}
		}

		OutputString += ParseField(str,20,'L',' ');
		OutputString += ParseField("",3,'L',' ');
		
		rsRefPhy->Close();		

		//BillsT.HospFrom
		var = rsBills->Fields->Item["HospFrom"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",6,'L',' ');

		//BillsT.HospTo
		var = rsBills->Fields->Item["HospTo"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 18 (double) //////////////////////////////////

		//Line 19 (double) /////////////////////////////////////////
		OutputString = "";

		//BillsT.HCFABlock19
		var = rsBills->Fields->Item["HCFABlock19"]->Value;
		if(var.vt == VT_BSTR){
			str = CString(var.bstrVal);
		}
		else{
			str = "";
		}

		if(str == "" && m_HCFAInfo.Box19RefPhyID == 1)
			str = Box17a();	//use the Ref Phy ID
		else if(str == "" && m_HCFAInfo.Box19Override == 1)
			str = m_HCFAInfo.Box19;

		//check for the override
		// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
		_RecordsetPtr rsTemp = CreateParamRecordset("SELECT Box19 FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
		if(!rsTemp->eof) {
			var = rsTemp->Fields->Item["Box19"]->Value;
			if(var.vt == VT_BSTR) {
				CString strTemp = CString(var.bstrVal);
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty())
					str = strTemp;
			}
		}
		rsTemp->Close();

		OutputString += ParseField(str,60,'L',' ');
		OutputString += ParseField("",4,'L',' ');

		//BillsT.OutsideLab
		var = rsBills->Fields->Item["OutsideLab"]->Value;
		if(var.vt == VT_BOOL){
			if(var.boolVal){
				str = "X";
			}
			else{
				str = "    X";
			}
		}
		else{
			str = "    X";
		}

		OutputString += ParseField(str,9,'L',' ');

		BOOL bUseDecimals = (GetRemotePropertyInt("HCFAImageUseDecimals",0,0,"<None>",true) == 1);

		//BillsT.OutLabCharges
		var = rsBills->Fields->Item["OutsideLabCharges"]->Value;
		if(var.vt == VT_CY){
			str = FormatCurrencyForInterface(var.cyVal,FALSE,FALSE);
			if(!bUseDecimals)
				str = StripNonNum(str);
		}
		else{
			str = "0";
		}
		
		if(!bUseDecimals)
			OutputString += ParseField(str,6,'R','0');
		else
			OutputString += ParseField(str,6,'R',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 19 (double) //////////////////////////////////


		//For the next two lines we're using charge information common to all charges, the 
		//looping will be later.

		//Line 20 (double) /////////////////////////////////////////
		OutputString = "";
		
		//Spaces
		OutputString += ParseField("", 2, 'L', ' ');

		//BillsT.Diag1ID

		// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(0).number;

		OutputString += ParseField(str, 6, 'L', ' ');

		// (j.jones 2006-11-09 16:42) - PLID 22668 - in the new HCFA image,
		// diag codes are always 1, 3 on the first line and 2, 4 on the second line

		if(m_HCFAInfo.ShowICD9Desc) {
			str = m_pEBillingInfo->GetSafeBillDiag(0).description;
			OutputString += ParseField("", 3, 'L', ' ');
			OutputString += ParseField(str, 20, 'L', ' ');
			OutputString += ParseField("", 4, 'L', ' ');
		}
		else {
			OutputString += ParseField("", 27, 'L', ' ');
		}

		
		//BillsT.Diag3ID

		// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(2).number;

		OutputString += ParseField(str, 6, 'L', ' ');

		if(m_HCFAInfo.ShowICD9Desc) {
			str = m_pEBillingInfo->GetSafeBillDiag(2).description;
			OutputString += ParseField("", 3, 'L', ' ');
			OutputString += ParseField(str, 12, 'L', ' ');
			OutputString += ParseField("", 4, 'L', ' ');
		}
		else {
			OutputString += ParseField("", 19, 'L', ' ');
		}
		
		//BillsT.MedicaidResubmission
		var = rsBills->Fields->Item["MedicaidResubmission"]->Value;
		if(var.vt == VT_BSTR){
			str = CString(var.bstrVal);
		}
		else{
			str = "";
		}
		OutputString += ParseField(str, 12, 'L', ' ');
		OutputString += ParseField("", 2, 'L', ' ');

		//BillsT.OriginalRefNo
		var = rsBills->Fields->Item["OriginalRefNo"]->Value;
		if(var.vt == VT_BSTR){
			str = CString(var.bstrVal);
		}
		else{
			str = "";
		}
		OutputString += ParseField(str, 13, 'L', ' ');


		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 20 (double) //////////////////////////////////

		//Line 21 (triple) /////////////////////////////////////////
		OutputString = "";

		//Spaces
		OutputString += ParseField("", 2, 'L', ' ');

		// (j.jones 2006-11-09 16:42) - PLID 22668 - in the new HCFA image,
		// diag codes are always 1, 3 on the first line and 2, 4 on the second line

		//BillsT.Diag2ID

		// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(1).number;

		OutputString += ParseField(str, 6, 'L', ' ');

		if(m_HCFAInfo.ShowICD9Desc) {
			str = m_pEBillingInfo->GetSafeBillDiag(1).description;
			OutputString += ParseField("", 3, 'L', ' ');
			OutputString += ParseField(str, 20, 'L', ' ');
			OutputString += ParseField("", 4, 'L', ' ');
		}
		else {
			OutputString += ParseField("", 27, 'L', ' ');
		}
		
		//BillsT.Diag4ID

		// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(3).number;

		OutputString += ParseField(str, 6, 'L', ' ');

		if(m_HCFAInfo.ShowICD9Desc) {
			str = m_pEBillingInfo->GetSafeBillDiag(3).description;
			OutputString += ParseField("", 3, 'L', ' ');
			OutputString += ParseField(str, 12, 'L', ' ');
			OutputString += ParseField("", 4, 'L', ' ');
		}
		else {
			OutputString += ParseField("", 19, 'L', ' ');
		}

		//if UseBox23InBox17a = 2, then we don't fill Box 23 (though CLIA overrides this)
		if(m_HCFAInfo.UseBox23InBox17a != 2) {
			//BillsT.PriorAuthNum
			var = rsBills->Fields->Item["PriorAuthNum"]->Value;
			if(var.vt == VT_BSTR){
				str = CString(var.bstrVal);
			}
			else{
				str = "";
			}

			//use the override from the HCFA setup
			if(str == "")
				str = m_HCFAInfo.DefaultPriorAuthNum;
		}
		else {
			str = "";
		}

		if(eCLIANumber.bUseCLIANumber && eCLIANumber.bUseCLIAInHCFABox23) {
			//override with the CLIA
			str = eCLIANumber.strCLIANumber;
		}
		
		OutputString += ParseField(str, 32, 'L', ' ');

		OutputString+= "\r\n\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 21 (triple) //////////////////////////////////

		return Success;

	}NxCatchAll("Error in Ebilling::IntermediateNPI_HCFABoxes12to23");

	return Error_Other;
}

int CEbilling::IntermediateNPI_HCFABoxes24to33(_RecordsetPtr &rsBills, long nCurPage, long nPages)
{
	//this exports fields 24 to 33, HCFA Image format

	try {

		int ChargesPerPage = GetRemotePropertyInt("HCFAImageChargesPerPage", 6, 0, "<None>", true);

		BOOL bNeedPrintAnesthesiaTimes = FALSE;

		BOOL bUseDecimals = (GetRemotePropertyInt("HCFAImageUseDecimals",0,0,"<None>",true) == 1);

		_variant_t var;

		CString str, OutputString;
		int chargecount = ChargesPerPage;

		//JJ - we must only output 6 charges per claim, so we output the amount of charges on the current page

		//also calculate the total as we go
		COleCurrency cyCharges = COleCurrency(0,0);
		
		long nLastCharge = m_nCurrPage * ChargesPerPage;
		long nFirstCharge = nLastCharge - (ChargesPerPage - 1);
		
		//we will loop from nFirstCharge through nLastCharge, or EOF, whichever comes first;
		
		//first loop up until the charge we need to start on
		//TES 11/5/2007 - PLID 27978 - VS2008 - for() loops
		int charge = 1;
		for(charge = 1; charge < nFirstCharge; charge++) {

			if(rsBills->eof) {
				continue;
			}

			rsBills->MoveNext();
		}

		CString strChargeIDs = "";

		for(charge = nFirstCharge; charge <= nLastCharge; charge++) {

			if(rsBills->eof) {
				continue;
			}

			// (j.jones 2007-10-05 10:52) - PLID 27659 - track the IDs of the charges we are outputting
			long nChargeID = AdoFldLong(rsBills, "ChargeID");
			if(!strChargeIDs.IsEmpty())
				strChargeIDs += ",";
			strChargeIDs += AsString(nChargeID);

			// (j.jones 2007-10-15 15:05) - PLID 27757 - track the ServiceID
			long nServiceID = AdoFldLong(rsBills, "ServiceID");

			//Line 22 (double) /////////////////////////////////////////
			OutputString = "";

			// (j.jones 2008-05-02 10:50) - PLID 28012 - removed the requirement for UseAnesthesiaBilling
			// to be checked for the service
			if(m_pEBillingInfo->AnesthesiaTimes && AdoFldBool(rsBills, "Anesthesia",FALSE)
				/*&& AdoFldBool(rsBills, "UseAnesthesiaBilling",FALSE)*/) {
				bNeedPrintAnesthesiaTimes = TRUE;
			}

			//BillsT.ServiceDateFrom
			var = rsBills->Fields->Item["ServiceDateFrom"]->Value;
			if(var.vt==VT_DATE) {
				COleDateTime dt;
				dt = var.date;
				str = dt.Format("%m/%d/%Y");
			}
			else
				str = "";
			OutputString += ParseField(str,10,'L',' ');
			OutputString += ParseField("",2,'L',' ');
			
			//BillsT.ServiceDateTo
			var = rsBills->Fields->Item["ServiceDateTo"]->Value;
			// (j.jones 2007-04-10 08:44) - PLID 25539 - added ability to hide the ServiceDateTo field
			if(var.vt==VT_DATE && m_HCFAInfo.Hide24ATo == 0) {
				COleDateTime dt;
				dt = var.date;
				str = dt.Format("%m/%d/%Y");
			}
			else
				str = "";
			OutputString += ParseField(str,10,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.ServiceLocation
			var = rsBills->Fields->Item["PlaceCodes"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",1,'L',' ');

			//ChargesT.ServiceType
			var = rsBills->Fields->Item["ServiceType"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",1,'L',' ');

			//ChargesT.ItemCode
			var = rsBills->Fields->Item["ItemCode"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,6,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.CPTModifier
			var = rsBills->Fields->Item["CPTModifier"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.CPTModifier2
			var = rsBills->Fields->Item["CPTModifier2"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.CPTModifier3
			var = rsBills->Fields->Item["CPTModifier3"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.CPTModifier4
			var = rsBills->Fields->Item["CPTModifier4"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",2,'L',' ');


			//ChargesT.WhichCodes
			// (j.gruber 2014-03-17 11:35) - PLID 61395 - new billing structure
			var = rsBills->Fields->Item["ChargeWhichCodes"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,7,'L',' ');

			// (j.jones 2011-06-17 14:34) - PLID 34795 - I reduced this space from 6 to 5,
			// and increased the charge field from 7 to 8, so if you submit a 10000.00 
			// charge it will take up one more space to the left.
			OutputString += ParseField("",5,'L',' ');

			//we need the Amount * Quantity here, which I calculate in the recordset
			var = rsBills->Fields->Item["ChargeTot"]->Value;
			if(var.vt==VT_CY) {
				cyCharges += var.cyVal;
				str = FormatCurrencyForInterface(var.cyVal,FALSE,FALSE);
				if(!bUseDecimals)
					str = StripNonNum(str);
			}
			else
				str = "";
			if(!bUseDecimals)
				OutputString += ParseField(str,8,'R','0');
			else
				OutputString += ParseField(str,8,'R',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.Quantity
			var = rsBills->Fields->Item["Quantity"]->Value;
			if(var.vt==VT_R8) {
				str.Format("%g",var.dblVal);
			}
			else
				str = "";

			// (j.jones 2007-01-03 13:26) - PLID 24084 - show minutes as quantity if anesthesia
			// (j.jones 2008-05-02 10:50) - PLID 28012 - removed the requirement for UseAnesthesiaBilling
			// to be checked for the service, and for a flat-fee
			if(m_HCFAInfo.ANSI_UseAnesthMinutesAsQty == 1 && AdoFldBool(rsBills, "Anesthesia", FALSE)
				/*&& AdoFldBool(rsBills, "UseAnesthesiaBilling", FALSE)
				// (j.jones 2007-10-15 14:57) - PLID 27757 - converted to use the new structure
				&& ReturnsRecords("SELECT ID FROM AnesthesiaSetupT WHERE ServiceID = %li AND LocationID = %li AND AnesthesiaFeeBillType <> 1", nServiceID, m_pEBillingInfo->nPOSID)*/) {

				//they want to show minutes, and this is indeed an anesthesia charge that is based on time
				str.Format("%li",AdoFldLong(rsBills, "AnesthesiaMinutes", 0));
			}

			OutputString += ParseField(str,3,'L',' ');
			OutputString += ParseField("",8,'L',' ');

			//InsuranceBox24J.Box24JNumber
			// (j.jones 2008-04-03 10:12) - PLID 28995 - use the Box24J_ProviderID here
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Box24JNumber FROM InsuranceBox24J WHERE InsCoID = {INT} AND ProviderID = {INT}",
					m_pEBillingInfo->InsuranceCoID,m_pEBillingInfo->Box24J_ProviderID);

			if(!rs->eof && rs->Fields->Item["Box24JNumber"]->Value.vt==VT_BSTR) {
				str = CString(rs->Fields->Item["Box24JNumber"]->Value.bstrVal);
			}
			else
				str = "";

			//check for the override
			// (j.jones 2008-04-03 10:12) - PLID 28995 - use the Box24J_ProviderID here
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			_RecordsetPtr rsTemp = CreateParamRecordset("SELECT Box24J FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->Box24J_ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
			if(!rsTemp->eof) {
				var = rsTemp->Fields->Item["Box24J"]->Value;
				if(var.vt == VT_BSTR) {
					CString strTemp = CString(var.bstrVal);
					strTemp.TrimLeft();
					strTemp.TrimRight();
					if(!strTemp.IsEmpty())
						str = strTemp;
				}
			}
			rsTemp->Close();

			OutputString += ParseField(str,15,'L',' ');

			rs->Close();

			if(bNeedPrintAnesthesiaTimes) {
				OutputString += "\r\n";
			}
			else {
				OutputString += "\r\n\r\n";
			}

			TRACE(OutputString);
			m_OutputFile.Write(OutputString,OutputString.GetLength());
			//End Of Line 22 (double) //////////////////////////////////

			chargecount--;
			rsBills->MoveNext();

			if(bNeedPrintAnesthesiaTimes) {
				OutputHCFAImageAnesthesiaTimes(m_pEBillingInfo->BillID, m_pEBillingInfo->OnlyAnesthesiaMinutes);
				bNeedPrintAnesthesiaTimes = FALSE;
			}
		}

		rsBills->Close();

		for(int i=0;i<=chargecount;i++) {
			OutputString = "\r\n\r\n";
			TRACE(OutputString);
			m_OutputFile.Write(OutputString,OutputString.GetLength());
		}

		//Line 23 (double) /////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",2,'L',' ');

		//Federal Tax ID Number - SSN or EIN

		// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
		m_rsDoc = CreateParamRecordset("SELECT * FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE ProvidersT.PersonID = {INT}",m_pEBillingInfo->ProviderID);

		if(m_rsDoc->eof) {
			//if the recordset is empty, there is no doctor. So halt everything!!!
			m_rsDoc->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open provider information for patient '%s', Bill ID %li. (IntermediateNPI_HCFABoxes24to33)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open provider information. (IntermediateNPI_HCFABoxes24to33)";

			if(m_pEBillingInfo->Box33Setup==2)
				str += "\nIt is possible that you have no provider selected in General 1 for this patient.";
			else
				str += "\nIt is possible that you have no provider selected on this patient's bill.";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		//check for the override
		BOOL bOverride = FALSE;
		// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
		// (j.jones 2008-09-10 11:35) - PLID 30788 - added Box25Check
		_RecordsetPtr rsTemp = CreateParamRecordset("SELECT EIN, Box25Check FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
		long nBox25Check = 0;
		if(!rsTemp->eof) {
			var = rsTemp->Fields->Item["EIN"]->Value;
			if(var.vt == VT_BSTR) {
				CString strTemp = CString(var.bstrVal);
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty()) {
					str = strTemp;
					bOverride = TRUE;
				}
			}
			nBox25Check = AdoFldLong(rsTemp, "Box25Check", 0);
		}
		rsTemp->Close();

		if(m_HCFAInfo.Box25 == 1) { //SSN

			//PersonT.SocialSecurity
			if(!bOverride) {
				var = m_rsDoc->Fields->Item["SocialSecurity"]->Value;
				if(var.vt==VT_BSTR) {
					str = CString(var.bstrVal);
				}
				else
					str = "";
			}
			// (j.jones 2007-06-21 17:13) - PLID 26424 - needs to be 11 characters
			OutputString += ParseField(str,11,'L',' ');
			OutputString += ParseField("",1,'L',' ');

			// (j.jones 2008-09-10 11:36) - PLID 30788 - if the nBox25Check is 2,
			// override with EIN, otherwise it is SSN
			if(nBox25Check == 2) {
				str = "   X";
			}
			else {
				str = "X";
			}
			OutputString += ParseField(str,15,'L',' ');
		}
		else { //Fed Emp. ID

			//ProvidersT.[Fed Employer ID] OR LocationsT.EIN
			if(!bOverride) {

				if(m_HCFAInfo.Box33Setup == 4) {
					//use bill location's EIN
					//DRT 8/25/2008 - PLID 31163 - Undid j.jones change, which I believe was mistaken.  This is the EIN, not the NPI.
					_RecordsetPtr rsLoc = CreateRecordset("SELECT EIN FROM LocationsT WHERE ID = %li",m_pEBillingInfo->BillLocation);
					if(!rsLoc->eof) {
						var = rsLoc->Fields->Item["EIN"]->Value;
						if(var.vt==VT_BSTR) {
							str = CString(var.bstrVal);
						}
						else
							str = "";
					}
					rsLoc->Close();
				}
				else {
					//use provider's fed. employer ID

					var = m_rsDoc->Fields->Item["Fed Employer ID"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
				}
			}
			OutputString += ParseField(str,10,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			// (j.jones 2008-09-10 11:36) - PLID 30788 - if the nBox25Check is 1,
			// override with SSN, otherwise it is EIN
			if(nBox25Check == 1) {
				str = "X";
			}
			else {
				str = "   X";
			}
			OutputString += ParseField(str,15,'L',' ');
		}

		//PatientsT.UserDefinedID
		str.Format("%li",m_pEBillingInfo->UserDefinedID);

		//determine if we need to enlarge this field
		BOOL bExtendPatIDField = FALSE;

		if(GetRemotePropertyInt("SearchByBillID", 0, 0, "<None>", TRUE) == 1) {
			//use patient ID / bill ID as the patient ID
			str.Format("%li/%li",m_pEBillingInfo->UserDefinedID, m_pEBillingInfo->BillID);

			bExtendPatIDField = TRUE;

			if(GetRemotePropertyInt("HCFAIDAppendRespType", 0, 0, "<None>", TRUE) == 1) {
				//use patient ID / bill ID-RespTypeID as the patient ID (but only if it is secondary or higher)
				int RespTypeID = GetInsuranceTypeFromID(m_pEBillingInfo->InsuredPartyID);
				if(RespTypeID > 1)
					str.Format("%li/%li-%li",m_pEBillingInfo->UserDefinedID, m_pEBillingInfo->BillID, RespTypeID);
			}
		}
		OutputString += ParseField(str,bExtendPatIDField ? 17 : 7,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		//InsuranceAcceptedT.Accepted

		str = "    X";
		// (j.jones 2010-07-23 15:02) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsCo now
		BOOL bAccepted = GetAcceptAssignment_ByInsCo(m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);
		if(bAccepted) {
			str = "X";
		}
		OutputString += ParseField(str,bExtendPatIDField ? 12 : 22,'L',' ');

		//JJ 10/27/2003 - We can't always calculate cyCharges here because if the HCFA form is two pages or more,
		//then the total won't be accurate with the charges displayed on the page. So we must calculate that total instead.

		// (j.jones 2007-10-05 10:48) - PLID 27659 - Somehow, past-Josh didn't seem to think this same logic would count towards
		// the amount paid as well. We need to show the amount paid for the charges on this page, not the whole bill.

		COleCurrency cyApplies = COleCurrency(0,0),
					cyTotal = COleCurrency(0,0);

		// (j.jones 2008-06-18 09:34) - PLID 30403 - CalculateBoxes29and30 will calculate the values
		// for Box 29 and Box 30 based on the settings to ShowPays, IgnoreAdjustments, ExcludeAdjFromBal,
		// and DoNotFillBox29. I made it a modular function because it is identical for all three image types.
		CalculateBoxes29and30(cyCharges, cyApplies, cyTotal, strChargeIDs);	

		// (j.jones 2007-02-28 16:03) - PLID 25009 - for the three totals fields,
		// they now allow 8 characters total, supporting up to 5 characters to the
		// right of the decimal, if decimals are used. Spaces in between were
		// updated appropriately so the dollar amounts didn't move.

		//total charges
		str = FormatCurrencyForInterface(cyCharges,FALSE,FALSE);
		if(!bUseDecimals)
			str = StripNonNum(str);
		if(!bUseDecimals)
			OutputString += ParseField(str,8,'R','0');
		else
			OutputString += ParseField(str,8,'R',' ');
		OutputString += ParseField("",7,'L',' ');

		//total applies, which are only filled in if HCFASetupT.ShowPays = TRUE
		str = FormatCurrencyForInterface(cyApplies,FALSE,FALSE);
		if(!bUseDecimals)
			str = StripNonNum(str);
		if(!bUseDecimals)
			OutputString += ParseField(str,8,'R','0');
		else
			OutputString += ParseField(str,8,'R',' ');
		OutputString += ParseField("",4,'L',' ');

		//based on our setting, hide the total balance
		if(m_HCFAInfo.HideBox30 == 1) {
			str = "";
		}
		else {
			//if HCFASetupT.ShowPays = TRUE, this is the balance,
			//if not, this is the total charges
			str = FormatCurrencyForInterface(cyTotal,FALSE,FALSE);
			if(!bUseDecimals)
				str = StripNonNum(str);
		}

		if(!bUseDecimals && m_HCFAInfo.HideBox30 == 0)
			OutputString += ParseField(str,8,'R','0');
		else
			OutputString += ParseField(str,8,'R',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 23 (double) //////////////////////////////////		

		//Line 24 //////////////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",29,'L',' ');

		bool bHideBox32NameAdd = false;
		//check to see if we should hide box 32

		// (j.jones 2007-05-11 12:02) - PLID 25932 - reworked this to have options to 
		// hide just the name and address, and the IDs
		// (j.gruber 2007-06-28 16:49) - PLID 26154 - fixed the query for if POS is null
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		if(m_HCFAInfo.HideBox32NameAdd != 0) {
			//if HideBox32NameAdd = 1, they want to hide the name/add in box 32 if the POS is 11,
			//so check to see if any of the charges associated with this bill have a POS of 11
			//if HideBox32NameAdd = 2, they want to hide the name/add in box 32 if the POS is NOT 11,
			//so check to see if any of the charges associated with this bill do not have a POS of 11
			//if HideBox32NameAdd = 3, they want to always hide the name/add in box 32
			if(m_HCFAInfo.HideBox32NameAdd == 3 || (m_HCFAInfo.HideBox32NameAdd == 1 && !IsRecordsetEmpty("SELECT PlaceOfServiceCodesT.PlaceCodes FROM ChargesT INNER JOIN LINEITEMT ON ChargesT.ID = LineItemT.ID "
				" LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				" LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				" LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				" WHERE BillID = %li AND Deleted = 0 AND Batched = 1 "
				" AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				" AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				" AND PlaceCodes = '11'", m_pEBillingInfo->BillID)) ||
				
				(m_HCFAInfo.HideBox32NameAdd == 2 && !IsRecordsetEmpty("SELECT PlaceOfServiceCodesT.PlaceCodes FROM ChargesT INNER JOIN LINEITEMT ON ChargesT.ID = LineItemT.ID "
				" LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				" LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				" LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				" WHERE BillID = %li AND Deleted = 0 AND Batched = 1 "
				" AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				" AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				" AND (PlaceCodes <> '11' OR PlaceCodes IS NULL) ", m_pEBillingInfo->BillID))) {

				bHideBox32NameAdd = true;
			}
			else{
				bHideBox32NameAdd = false;
			}
		}
		else{
			bHideBox32NameAdd = false;
		}

		//LocationsT.Name (POS)
		// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
		_RecordsetPtr rsPOS = CreateParamRecordset("SELECT * FROM LocationsT WHERE ID = {INT}",m_pEBillingInfo->nPOSID);

		if(!bHideBox32NameAdd && !rsPOS->eof && rsPOS->Fields->Item["Name"]->Value.vt==VT_BSTR)
			str = CString(rsPOS->Fields->Item["Name"]->Value.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,26,'L',' ');
		OutputString += ParseField("",6,'L',' ');

		// (j.jones 2007-05-07 14:38) - PLID 25922 - added Box33Name override
		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		CString strBox33NameOver = "", strBox33Address1_Over = "", strBox33Address2_Over = "",
			strBox33City_Over = "", strBox33State_Over = "", strBox33Zip_Over = "";
		// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
		_RecordsetPtr rsBox33Over = CreateParamRecordset("SELECT Box33Name, "
			"Box33_Address1, Box33_Address2, Box33_City, Box33_State, Box33_Zip "
			"FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
		if(!rsBox33Over->eof) {
			strBox33NameOver = AdoFldString(rsBox33Over, "Box33Name","");
			strBox33NameOver.TrimLeft();
			strBox33NameOver.TrimRight();

			// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
			strBox33Address1_Over = AdoFldString(rsBox33Over, "Box33_Address1","");
			strBox33Address1_Over.TrimLeft();
			strBox33Address1_Over.TrimRight();
			strBox33Address2_Over = AdoFldString(rsBox33Over, "Box33_Address2","");
			strBox33Address2_Over.TrimLeft();
			strBox33Address2_Over.TrimRight();
			strBox33City_Over = AdoFldString(rsBox33Over, "Box33_City","");
			strBox33City_Over.TrimLeft();
			strBox33City_Over.TrimRight();
			strBox33State_Over = AdoFldString(rsBox33Over, "Box33_State","");
			strBox33State_Over.TrimLeft();
			strBox33State_Over.TrimRight();
			strBox33Zip_Over = AdoFldString(rsBox33Over, "Box33_Zip","");
			strBox33Zip_Over.TrimLeft();
			strBox33Zip_Over.TrimRight();
		}
		rsBox33Over->Close();
		
		if(!strBox33NameOver.IsEmpty()) {
			//if not empty, output the override
			OutputString += ParseField(strBox33NameOver,45,'L',' ');
		}
		else {
			//otherwise do the normal load

			if(m_HCFAInfo.Box33Setup == 4) {
				//bill location
				// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
				_RecordsetPtr rsLoc = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}",m_pEBillingInfo->BillLocation);
				if(!rsLoc->eof) {
					CString strName = AdoFldString(rsLoc, "Name","");
					OutputString += ParseField(strName,45,'L',' ');
				}
				rsLoc->Close();
			}
			else if(m_HCFAInfo.Box33Setup == 3) {
				//override
				// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
				_RecordsetPtr rsOver = CreateParamRecordset("SELECT Note FROM PersonT WHERE ID = {INT}",m_HCFAInfo.Box33Num);
				if(!rsOver->eof) {
					CString strName = AdoFldString(rsOver, "Note","");
					OutputString += ParseField(strName,45,'L',' ');
				}
				rsOver->Close();
			}
			else {
				//a provider

				if(m_HCFAInfo.Box33Order == 1) { //LFM

					//PersonT.Last
					var = m_rsDoc->Fields->Item["Last"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,16,'L',' ');
					OutputString += ParseField("",1,'L',' ');

					//PersonT.First
					var = m_rsDoc->Fields->Item["First"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,13,'L',' ');
					OutputString += ParseField("",1,'L',' ');

					//PersonT.Middle
					var = m_rsDoc->Fields->Item["Middle"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,1,'L',' ');
					OutputString += ParseField("",1,'L',' ');				

				}
				else {	//FML

					//PersonT.First
					var = m_rsDoc->Fields->Item["First"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,13,'L',' ');
					OutputString += ParseField("",1,'L',' ');

					//PersonT.Middle
					var = m_rsDoc->Fields->Item["Middle"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,1,'L',' ');
					OutputString += ParseField("",1,'L',' ');

					//PersonT.Last
					var = m_rsDoc->Fields->Item["Last"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,16,'L',' ');
					OutputString += ParseField("",1,'L',' ');

				}

				//PersonT.Title
				var = m_rsDoc->Fields->Item["Title"]->Value;
				if(var.vt==VT_BSTR) {
					str = CString(var.bstrVal);
				}
				else
					str = "";
				OutputString += ParseField(str,5,'L',' ');
			}
		}

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 24 ///////////////////////////////////////////

		//Line 25 //////////////////////////////////////////////////
		OutputString = "";

		//InsuranceBox31.Box31Info
		// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT Box31Info FROM InsuranceBox31 WHERE InsCoID = {INT} AND ProviderID = {INT}",
				m_pEBillingInfo->InsuranceCoID,m_pEBillingInfo->ProviderID);

		if(!rs->eof && rs->Fields->Item["Box31Info"]->Value.vt==VT_BSTR) {
			str = CString(rs->Fields->Item["Box31Info"]->Value.bstrVal);
		}
		else
			str = "";
		OutputString += ParseField(str,15,'L',' ');
		OutputString += ParseField("",14,'L',' ');

		// (j.jones 2007-05-03 09:37) - PLID 25885 - mimic the HCFA such that
		// Address 1 and Address 2 are concatenated on the same line,
		// though are still limited to 29 characters total

		//LocationsT.Address1 + Address 2 (POS)
		if(!bHideBox32NameAdd) {
			str.Format("%s %s", AdoFldString(rsPOS, "Address1",""), AdoFldString(rsPOS, "Address2",""));
		}
		else
			str = "";
		OutputString += ParseField(str,29,'L',' ');		
		OutputString += ParseField("",3,'L',' ');

		//LocationsT.Address1 + Address 2 (Location)
		_RecordsetPtr rsLoc;
		if(m_HCFAInfo.Box33Setup == 3) {
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			rsLoc = CreateParamRecordset("SELECT * FROM PersonT WHERE ID = {INT}",m_HCFAInfo.Box33Num);
		}
		else if(m_HCFAInfo.DocAddress == 1 || m_HCFAInfo.Box33Setup == 4) {
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			rsLoc = CreateParamRecordset("SELECT * FROM LocationsT WHERE ID = {INT}",m_pEBillingInfo->BillLocation);
		}
		else {
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			rsLoc = CreateParamRecordset("SELECT * FROM PersonT WHERE ID = {INT}",m_pEBillingInfo->ProviderID);
		}

		// (j.jones 2007-05-03 09:37) - PLID 25885 - mimic the HCFA such that
		// Address 1 and Address 2 are concatenated on the same line,
		// though are still limited to 32 characters total

		CString strAddress1 = AdoFldString(rsLoc, "Address1","");
		CString strAddress2 = AdoFldString(rsLoc, "Address2","");

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33Address1_Over.IsEmpty()) {
			strAddress1 = strBox33Address1_Over;
		}
		if(!strBox33Address2_Over.IsEmpty()) {
			strAddress2 = strBox33Address2_Over;
		}

		str.Format("%s %s", strAddress1, strAddress2);
		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 25 ////////////////////////////////////////////

		//Line 26 ///////////////////////////////////////////////////
		OutputString = "";

		str = "";

		//Box31 may use the Override name
		if(m_HCFAInfo.UseOverrideInBox31 == 1 && m_HCFAInfo.Box33Setup == 3) {
			//override
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			_RecordsetPtr rsOver = CreateParamRecordset("SELECT Note FROM PersonT WHERE ID = {INT}",m_HCFAInfo.Box33Num);
			if(!rsOver->eof) {
				str = AdoFldString(rsOver, "Note","");
			}
			rsOver->Close();
		}
		else {

			//PersonT.First
			var = m_rsDoc->Fields->Item["First"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";

			str += " ";

			//PersonT.Middle
			var = m_rsDoc->Fields->Item["Middle"]->Value;
			if(var.vt==VT_BSTR) {
				str += CString(var.bstrVal);
			}
			else
				str += "";

			str += " ";

			//PersonT.Last
			var = m_rsDoc->Fields->Item["Last"]->Value;
			if(var.vt==VT_BSTR) {
				str += CString(var.bstrVal);
			}
			else
				str += "";
			
			str += " ";

			//PersonT.Title
			var = m_rsDoc->Fields->Item["Title"]->Value;
			if(var.vt==VT_BSTR) {
				str += CString(var.bstrVal);
			}
			else
				str += "";
		}

		OutputString += ParseField(str,27,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.City (POS)
		if(!bHideBox32NameAdd) {
			var = rsPOS->Fields->Item["City"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,14,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.State (POS)
		if(!bHideBox32NameAdd) {
			var = rsPOS->Fields->Item["State"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,2,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.Zip (POS)
		if(!bHideBox32NameAdd) {
			var = rsPOS->Fields->Item["Zip"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.City (Location)
		var = rsLoc->Fields->Item["City"]->Value;
		if(var.vt==VT_BSTR) {
			str = CString(var.bstrVal);
		}
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33City_Over.IsEmpty()) {
			str = strBox33City_Over;
		}

		OutputString += ParseField(str,15,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.State (Location)
		var = rsLoc->Fields->Item["State"]->Value;
		if(var.vt==VT_BSTR) {
			str = CString(var.bstrVal);
		}
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33State_Over.IsEmpty()) {
			str = strBox33State_Over;
		}

		OutputString += ParseField(str,2,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.Zip (Location)
		var = rsLoc->Fields->Item["Zip"]->Value;
		if(var.vt==VT_BSTR) {
			str = CString(var.bstrVal);
		}
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33Zip_Over.IsEmpty()) {
			str = strBox33Zip_Over;
		}

		// (j.jones 2007-08-28 08:40) - PLID 27197 - expanded to 10 characters
		OutputString += ParseField(str,10,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		// (j.jones 2006-12-11 09:22) - PLID 23823 - we used to export the phone
		// number here but now we do it on line 28

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 26 ////////////////////////////////////////////

		//Line 27 //////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",16,'L',' ');

		//0 - the Print Date (today), 1 - the Bill Date
		if(m_HCFAInfo.Box31UseDate == 0) {
			COleDateTime dt;
			dt = COleDateTime::GetCurrentTime();
			str = dt.Format("%m/%d/%Y");
		}
		else {
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			rsBills = CreateParamRecordset("SELECT Date FROM BillsT WHERE ID = {INT}",m_pEBillingInfo->BillID);
			//BillsT.Date
			if(!rsBills->eof && rsBills->Fields->Item["Date"]->Value.vt == VT_DATE) {
				COleDateTime dt = rsBills->Fields->Item["Date"]->Value.date;
				str = dt.Format("%m/%d/%Y");
			}
			else
				str = "";
		}
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",3,'L',' ');

		bool bHideBox32NPIID = false;
		//check to see if we should hide box 32

		// (j.jones 2007-05-11 12:02) - PLID 25932 - reworked this to have options to 
		// hide just the name and address, and the IDs
		// (j.gruber 2007-06-28 16:49) - PLID 26154 - fixed the query for if POS is null
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		if(m_HCFAInfo.HideBox32NPIID != 0) {
			//if HideBox32NPIID = 1, they want to hide the IDs in box 32 if the POS is 11,
			//so check to see if any of the charges associated with this bill have a POS of 11
			//if HideBox32NPIID = 2, they want to hide the IDs in box 32 if the POS is NOT 11,
			//so check to see if any of the charges associated with this bill do not have a POS of 11
			//if HideBox32NPIID = 3, they want to always hide the IDs in box 32
			if(m_HCFAInfo.HideBox32NPIID == 3 || (m_HCFAInfo.HideBox32NPIID == 1 && !IsRecordsetEmpty("SELECT PlaceOfServiceCodesT.PlaceCodes FROM ChargesT INNER JOIN LINEITEMT ON ChargesT.ID = LineItemT.ID "
				" LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				" LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				" LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				" WHERE BillID = %li AND Deleted = 0 AND Batched = 1 "
				" AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				" AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				" AND PlaceCodes = '11'", m_pEBillingInfo->BillID)) ||
				
				(m_HCFAInfo.HideBox32NPIID == 2 && !IsRecordsetEmpty("SELECT PlaceOfServiceCodesT.PlaceCodes FROM ChargesT INNER JOIN LINEITEMT ON ChargesT.ID = LineItemT.ID "
				" LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				" LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				" LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				" WHERE BillID = %li AND Deleted = 0 AND Batched = 1 "
				" AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				" AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				" AND (PlaceCodes <> '11' OR PlaceCodes IS NULL) ", m_pEBillingInfo->BillID))) {

				bHideBox32NPIID = true;
			}
			else{
				bHideBox32NPIID = false;
			}
		}
		else{
			bHideBox32NPIID = false;
		}

		// (j.jones 2007-05-04 11:55) - PLID 25908 - supported option to send Box 32 NPI
		// and Facility ID, instead of just the facility ID;

		CString strPOSNPI = "";
		CString strFacilityID = "";

		if(!bHideBox32NPIID) {

			strPOSNPI = AdoFldString(rsPOS, "NPI","");

			//Facility ID (POS)
			// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
			_RecordsetPtr rsFID = CreateParamRecordset("SELECT FacilityID FROM InsuranceFacilityID WHERE LocationID = {INT} AND InsCoID = {INT}",m_pEBillingInfo->nPOSID, m_pEBillingInfo->InsuranceCoID);
			if(!rsFID->eof && rsFID->Fields->Item["FacilityID"]->Value.vt == VT_BSTR) {
				strFacilityID = CString(rsFID->Fields->Item["FacilityID"]->Value.bstrVal);
			}
			else
				strFacilityID = "";
			rsFID->Close();

			// (j.jones 2009-02-06 16:29) - PLID 32951 - this function now requires the bill location
			// (j.jones 2009-02-23 10:54) - PLID 33167 - now we require the billing provider ID and rendering provider ID,
			// in this case the Rendering Provider ID is the Box 24J provider ID
			// (j.jones 2011-04-05 13:39) - PLID 42372 - this now returns a struct of information
			ECLIANumber eCLIANumber = UseCLIANumber(m_pEBillingInfo->BillID,m_pEBillingInfo->InsuredPartyID,m_pEBillingInfo->ProviderID,m_pEBillingInfo->Box24J_ProviderID, m_pEBillingInfo->BillLocation);
			if(eCLIANumber.bUseCLIANumber && eCLIANumber.bUseCLIAInHCFABox32) {
				strFacilityID = eCLIANumber.strCLIANumber;
			}
		}

		// (j.jones 2007-05-04 11:55) - PLID 25908 - now, decide whether to output the NPI and Facility ID,
		// or just the facility ID
		if(GetRemotePropertyInt("HCFAImageSendBox32NPI",1,0,"<None>",true) == 1) {

			//both NPI and facility ID
			OutputString += ParseField(strPOSNPI,12,'L',' ');
			OutputString += ParseField("",3,'L',' ');
			OutputString += ParseField(strFacilityID,15,'L',' ');	
			OutputString += ParseField("",2,'L',' ');
		}
		else {
			//just the facility ID
			OutputString += ParseField(strFacilityID,15,'L',' ');		
			OutputString += ParseField("",17,'L',' ');
		}

		//PIN
		str = Box33Pin();
		OutputString += ParseField(str,20,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		//GRP

		//first check the Insurance Co's group 
		// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
		rsTemp = CreateParamRecordset("SELECT GRP FROM InsuranceGroups INNER JOIN InsuredPartyT ON InsuranceGroups.InsCoID = InsuredPartyT.InsuranceCoID "
			"WHERE InsuredPartyT.PersonID = {INT} AND ProviderID = {INT}",m_pEBillingInfo->InsuredPartyID,m_rsDoc->Fields->Item["ID"]->Value.lVal);
		if(!rsTemp->eof && rsTemp->Fields->Item["GRP"]->Value.vt == VT_BSTR)
			str = CString(rsTemp->Fields->Item["GRP"]->Value.bstrVal);
		else
			str = "";
		rsTemp->Close();

		//then check the HCFA group's default
		if(m_HCFAInfo.Box33GRP.GetLength() != 0)
			str = m_HCFAInfo.Box33GRP;

		//last check for the override
		// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
		rsTemp = CreateParamRecordset("SELECT GRP FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
		if(!rsTemp->eof) {
			var = rsTemp->Fields->Item["GRP"]->Value;
			if(var.vt == VT_BSTR) {
				CString strTemp = CString(var.bstrVal);
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty())
					str = strTemp;
			}
		}
		rsTemp->Close();

		OutputString += ParseField(str,20,'L',' ');		

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 27 ///////////////////////////////////

		rsPOS->Close();

		//Line 28 ///////////////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",61,'L',' ');

		//LocationsT.Phone (Location)
		
		// (j.jones 2007-02-20 15:17) - PLID 23935 - check to see if we should hide the phone number in Box 33
		if(m_HCFAInfo.HidePhoneBox33 == 1) {
			str = "";
		}
		else {
			if(m_HCFAInfo.Box33Setup == 3)
				var = rsLoc->Fields->Item["WorkPhone"]->Value;
			else if(m_HCFAInfo.Box33Setup == 4)
				var = rsLoc->Fields->Item["Phone"]->Value;
			else if(m_HCFAInfo.DocAddress == 1)
				var = rsLoc->Fields->Item["Phone"]->Value;
			else
				var = rsLoc->Fields->Item["WorkPhone"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
		}
		OutputString += ParseField(str,14,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 28 ////////////////////////////////////////////

		// (j.jones 2006-11-10 10:34) - PLID 22668 - Lines 29/30 are all new
		// for the NPI HCFA Image

		//Line 29 ///////////////////////////////////////////////////
		OutputString = "";
		
		//output taxonomy code
		str = AdoFldString(m_rsDoc, "TaxonomyCode","");
		OutputString += ParseField(str, 20, 'L', ' ');
		OutputString += ParseField("", 41, 'L', ' ');

		// (j.jones 2006-11-10 10:48) - PLID 22668 - technically, line 29
		// is the "Billing NPI" and line 30 is the "Rendering NPI",
		// so here, use the LocationNPIUsage choice

		//output NPI

		// (j.jones 2006-11-16 09:07) - PLID 23563 - added LocationNPIUsage
		// option to determine whether to use the Location NPI or the
		// Provider's NPI

		// (j.jones 2007-01-18 11:46) - PLID 24264 - the LocationNPIUsage
		// used to be only available when using Bill Location, now it is
		// an option at all times

		if(m_HCFAInfo.LocationNPIUsage == 1) {
			// (j.jones 2008-05-06 15:06) - PLID 29937 - use the NPI from the loaded claim pointer
			str = m_pEBillingInfo->strBillLocationNPI;
		}
		else {
			str = AdoFldString(m_rsDoc, "NPI","");
		}

		// (j.jones 2007-08-08 10:43) - PLID 25395 - we need to check AdvHCFAPinT for the 33a NPI
		// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
		rsTemp = CreateParamRecordset("SELECT Box33aNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
		if(!rsTemp->eof) {
			var = rsTemp->Fields->Item["Box33aNPI"]->Value;
			if(var.vt == VT_BSTR) {
				CString strTemp = CString(var.bstrVal);
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty())
					str = strTemp;
			}
		}
		rsTemp->Close();

		OutputString += ParseField(str, 20, 'L', ' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 29 ////////////////////////////////////////////

		//Line 30 ///////////////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("", 61, 'L', ' ');

		// (j.jones 2006-11-10 10:48) - PLID 22668 - technically, line 29
		// is the "Billing NPI" and line 30 is the "Rendering NPI",
		// so here, use the provider's NPI at all times

		//output NPI
		str = AdoFldString(m_rsDoc, "NPI","");

		// (j.jones 2007-08-08 10:43) - PLID 25395 - we need to check AdvHCFAPinT for the 24J NPI
		// (j.jones 2008-04-03 10:12) - PLID 28995 - use the Box24J_ProviderID here
		// (j.jones 2008-05-06 12:31) - PLID 27453 - parameterized
		rsTemp = CreateParamRecordset("SELECT Box24JNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->Box24J_ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
		if(!rsTemp->eof) {
			var = rsTemp->Fields->Item["Box24JNPI"]->Value;
			if(var.vt == VT_BSTR) {
				CString strTemp = CString(var.bstrVal);
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty())
					str = strTemp;
			}
		}
		rsTemp->Close();

		OutputString += ParseField(str, 20, 'L', ' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 30 ////////////////////////////////////////////

		//Line 31 ///////////////////////////////////////////////////
		OutputString = "";

		str = "\r\n\r\n--------------NEXT CLAIM-----------------------";
		OutputString += ParseField(str,60,'L',' ');	

		OutputString+= "\r\n";
		//if(m_FormatID==INTERLINK)
		//	OutputString += "\f";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 28 ////////////////////////////////////////////

		rsLoc->Close();
		m_rsDoc->Close();

		return Success;

	}NxCatchAll("Error in Ebilling::IntermediateNPI_HCFABoxes24to33");

	return Error_Other;
}

// (j.jones 2006-11-09 15:43) - PLID 22668 - these "Old_" functions are for legacy support of the
// pre-2007 HCFA Image, without NPI numbers and qualifiers
// (j.jones 2007-05-15 09:11) - PLID 25953 - renamed from "Old_" to "Legacy_"

int CEbilling::Legacy_HCFABoxes1to11(_RecordsetPtr &rsBills)
{
	//this will export the insurance address, and all fields from box 1 to box 11, HCFA Image format

	try {
		//Line 1   ///////////////////////////////////////////
		CString str, OutputString = "";		

		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name, Address1, Address2, City, State, Zip, IDForInsurance, "
						"CASE WHEN InsurancePlansT.PlanType='Champus' THEN 3 WHEN InsurancePlansT.PlanType='Champva' THEN 4 WHEN InsurancePlansT.PlanType='FECA Black Lung' THEN 5 WHEN InsurancePlansT.PlanType='Group Health Plan' THEN 6 WHEN InsurancePlansT.PlanType='Medicaid' THEN 2 WHEN InsurancePlansT.PlanType='Medicare' THEN 1 ELSE 7 END AS InsType "
						"FROM InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
						"INNER JOIN PersonT ON InsuranceCoT.PersonID = PersonT.ID LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
						"WHERE InsuredPartyT.PersonID = {INT}",m_pEBillingInfo->InsuredPartyID);
		if(rs->eof) {
			//if the recordset is empty, there is no insurance company. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open primary insurance information from this patient's bill for '%s', Bill ID %li. (Legacy_HCFABoxes1to11 1)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open primary insurance information from this patient's bill. (Legacy_HCFABoxes1to11 1)";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		_variant_t var;


		//check the secondary ins address as well
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		// (j.jones 2010-08-31 17:02) - PLID 40303 - added TPLCode
		_RecordsetPtr rsOtherIns = CreateParamRecordset("SELECT Name, Address1, Address2, City, State, Zip, IDForInsurance, HCFASetupGroupID, InsuranceCoT.TPLCode, "
						"CASE WHEN InsurancePlansT.PlanType='Champus' THEN 3 WHEN InsurancePlansT.PlanType='Champva' THEN 4 WHEN InsurancePlansT.PlanType='FECA Black Lung' THEN 5 WHEN InsurancePlansT.PlanType='Group Health Plan' THEN 6 WHEN InsurancePlansT.PlanType='Medicaid' THEN 2 WHEN InsurancePlansT.PlanType='Medicare' THEN 1 ELSE 7 END AS InsType "
						"FROM InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
						"INNER JOIN PersonT ON InsuranceCoT.PersonID = PersonT.ID LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
						"WHERE InsuredPartyT.PersonID = {INT}",m_pEBillingInfo->OthrInsuredPartyID);

		OutputString += ParseField("",2,'L',' ');
		
		str = "";

		//use the secondary ins. name, if requested and available
		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "Name","");
		}

		OutputString += ParseField(str,40,'L',' ');
		OutputString += ParseField("",10,'L',' ');


		//InsuranceCoT.Name
		var = rs->Fields->Item["Name"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,50,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 1   ///////////////////////////////////

		//Line 2   //////////////////////////////////////////
		OutputString = "";
		
		OutputString += ParseField("",2,'L',' ');
		
		str = "";

		//use the secondary ins. address, if requested and available
		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "Address1","");
		}

		OutputString += ParseField(str,40,'L',' ');
		OutputString += ParseField("",10,'L',' ');
		
		//PersonT.Address1
		var = rs->Fields->Item["Address1"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 2   ///////////////////////////////////

		//Line 3   //////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",2,'L',' ');
		
		str = "";

		//use the secondary ins. address, if requested and available
		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "Address2","");
		}

		OutputString += ParseField(str,40,'L',' ');
		OutputString += ParseField("",10,'L',' ');
		
		//PersonT.Address2
		var = rs->Fields->Item["Address2"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 3   ///////////////////////////////////

		//Line 4   //////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",2,'L',' ');
		
		str = "";

		//use the secondary ins. address, if requested and available
		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "City","");
		}
		OutputString += ParseField(str,18,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "State","");
		}
		OutputString += ParseField(str,2,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1) {
			str = AdoFldString(rsOtherIns, "Zip","");
		}
		OutputString += ParseField(str,10,'L',' ');

		OutputString += ParseField("",18,'L',' ');
		
		//PersonT.City
		var = rs->Fields->Item["City"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,18,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.State
		var = rs->Fields->Item["State"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,2,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.Zip
		var = rs->Fields->Item["Zip"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 4   ///////////////////////////////////

		//Line 5 (double)  //////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",2,'L',' ');

		str = "";

		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1 && m_HCFAInfo.ShowSecPINNumber == 1) {
			str = Box33Pin(TRUE);
		}
		OutputString += ParseField(str,20,'L',' ');

		str = "";

		if(!rsOtherIns->eof && m_HCFAInfo.ShowSecInsAdd == 1 && m_HCFAInfo.ShowSec24JNumber == 1) {
			//InsuranceBox24J.Box24JNumber
			// (j.jones 2008-04-03 10:12) - PLID 28995 - use the Box24J_ProviderID here for the current HCFA group,
			// even though the actual ID will pull from the secondary insurance
			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Box24JNumber FROM InsuranceBox24J INNER JOIN InsuredPartyT ON InsuranceBox24J.InsCoID = InsuredPartyT.InsuranceCoID "
				"WHERE InsuredPartyT.PersonID = {INT} AND ProviderID = {INT}",
				m_pEBillingInfo->OthrInsuredPartyID,m_pEBillingInfo->Box24J_ProviderID);

			if(!rs->eof && rs->Fields->Item["Box24JNumber"]->Value.vt==VT_BSTR) {
				str = CString(rs->Fields->Item["Box24JNumber"]->Value.bstrVal);
			}
			else
				str = "";

			//check for the override
			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			_RecordsetPtr rsTemp = CreateParamRecordset("SELECT Box24J FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->Box24J_ProviderID,m_pEBillingInfo->BillLocation,AdoFldLong(rsOtherIns, "HCFASetupGroupID",-1));
			if(!rsTemp->eof) {
				var = rsTemp->Fields->Item["Box24J"]->Value;
				if(var.vt == VT_BSTR) {
					CString strTemp = CString(var.bstrVal);
					strTemp.TrimLeft();
					strTemp.TrimRight();
					if(!strTemp.IsEmpty())
						str = strTemp;
				}
			}
			rsTemp->Close();
		}
		OutputString += ParseField(str,15,'L',' ');

		// (j.jones 2009-08-04 12:32) - PLID 14573 - used payer ID, though
		// we kept the same preference name internally
		if(GetRemotePropertyInt("HCFAImageShowTHINNumber",0,0,"<None>",true) == 1) {
			//show EbillingID number
			OutputString += ParseField("",15,'L',' ');
			OutputString += ParseField("ID#   ",6,'L',' ');
		
			//InsuranceCoT.HCFAPayerID
			// (j.jones 2009-08-05 10:20) - PLID 34467 - this is now in the loaded claim info
			// (j.jones 2009-12-16 15:54) - PLID 36237 - split out into HCFA and UB payer IDs
			OutputString += ParseField(m_pEBillingInfo->strHCFAPayerID,32,'L',' ');
		}
		
		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 5  //////////////////////////////

		//Line 6 (double)  //////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",2,'L',' ');	

		//based on InsurancePlansT.PlanType
		var = rs->Fields->Item["InsType"]->Value;
		if(var.vt==VT_I4) {
			int InsType = var.lVal;
			switch(InsType) {
			case 1:
				str = "X";
				break;
			case 2:
				str = "         X";
				break;
			case 3:
				str = "                  X";
				break;
			case 4:
				str = "                           X";
				break;
			case 5:
				str = "                                    X";
				break;
			case 6:
				str = "                                             X";
				break;
			case 7:
				str = "                                                      X";
				break;
			}
		}
		else
			str = "";
		OutputString += ParseField(str,57,'L',' ');

		//InsuredPartyT.IDForInsurance
		var = rs->Fields->Item["IDForInsurance"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');	

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 6   ///////////////////////////////////

		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		_RecordsetPtr rsPatient = CreateParamRecordset("SELECT * FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE PatientsT.PersonID = {INT}",m_pEBillingInfo->PatientID);

		if(rsPatient->eof) {
			//if the recordset is empty, there is no patient. So halt everything!!!
			rsPatient->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open patient information for '%s', Bill ID %li. (Legacy_HCFABoxes1to11)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open patient information. (Legacy_HCFABoxes1to11)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		//Line 7 (double)   //////////////////////////////////////////
		OutputString = "";

		//Patient Information

		//PersonT.Last
		var = rsPatient->Fields->Item["Last"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,16,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.First
		var = rsPatient->Fields->Item["First"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,13,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.Middle
		var = rsPatient->Fields->Item["Middle"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,1,'L',' ');
		OutputString += ParseField("",4,'L',' ');

		//PersonT.BirthDate
		//Box3
		var = rsPatient->Fields->Item["BirthDate"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			
			if(m_HCFAInfo.Box3E == 1)
				str = dt.Format("%Y%m%d");
			else
				str = dt.Format("%m%d%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",3,'L',' ');

		//PersonT.Gender
		var = rsPatient->Fields->Item["Gender"]->Value;
		int Gender = VarByte(var,0);
		if(Gender==1)
			str = "X";
		else if(Gender==2)
			str = "      X";
		else
			str = "";

		OutputString += ParseField(str,10,'L',' ');

		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		_RecordsetPtr rsIns = CreateParamRecordset("SELECT * FROM PersonT INNER JOIN InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID WHERE ID = {INT}",m_pEBillingInfo->InsuredPartyID);
		if(rsIns->eof) {
			//if the recordset is empty, there is no insured party. So halt everything!!!
			rsIns->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open primary insurance information from this patient's bill for '%s', Bill ID %li. (Legacy_HCFABoxes1to11 2)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open primary insurance information from this patient's bill. (Legacy_HCFABoxes1to11 2)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		//Insured Party Information

		//PersonT.Last
		if(m_HCFAInfo.HideBox4 == 0) {
			var = rsIns->Fields->Item["Last"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,16,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.First
		if(m_HCFAInfo.HideBox4 == 0) {
			var = rsIns->Fields->Item["First"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else 
			str = "";
		OutputString += ParseField(str,13,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.Middle
		if(m_HCFAInfo.HideBox4 == 0) {
			var = rsIns->Fields->Item["Middle"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else 
			str = "";
		OutputString += ParseField(str,1,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 7   ///////////////////////////////////

		//Line 8 (double)   //////////////////////////////////////////
		OutputString = "";

		//Patient Information

		//PersonT.Address1
		var = rsPatient->Fields->Item["Address1"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');
		OutputString += ParseField("",4,'L',' ');

		//InsuredPartyT.RelationToPatient
		var = rsIns->Fields->Item["RelationToPatient"]->Value;
		if(var.vt==VT_BSTR) {
			str = CString(var.bstrVal);
			if(str=="Self")
				str="X";
			else if(str=="Spouse")
				str="       X";
			else if(str=="Child")
				str="             X";
			else
				str="                   X";
		}
		else
			str = "                   X";
		OutputString += ParseField(str,23,'L',' ');

		//Insurance Information

		//PersonT.Address1
		if(m_HCFAInfo.HideBox7 == 0) {
			var = rsIns->Fields->Item["Address1"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 8   ///////////////////////////////////


		//Line 9 (double)  /////////////////////////////////////////
		OutputString = "";
		
		//Patient Information

		//PersonT.City
		var = rsPatient->Fields->Item["City"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,28,'L',' ');
		OutputString += ParseField("",3,'L',' ');

		//PersonT.State
		var = rsPatient->Fields->Item["State"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,2,'L',' ');
		OutputString += ParseField("",7,'L',' ');

		//PatientsT.MaritalStatus
		var = rsPatient->Fields->Item["MaritalStatus"]->Value;
		if(var.vt==VT_BSTR) {
			str = CString(var.bstrVal);
			if(str=="1")
				str="X";
			else if(str=="2")
				str="        X";
			else
				str="                X";
		}
		else
			str = "                X";
		OutputString += ParseField(str,19,'L',' ');

		//Insurance Information

		//PersonT.City
		if(m_HCFAInfo.HideBox7 == 0) {
			var = rsIns->Fields->Item["City"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,18,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		//PersonT.State
		if(m_HCFAInfo.HideBox7 == 0) {
			var = rsIns->Fields->Item["State"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else 
			str = "";
		OutputString += ParseField(str,2,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 9   //////////////////////////////////

		//Line 10 (double)   /////////////////////////////////////////
		OutputString = "";
		
		//Patient Information

		//PersonT.Zip
		var = rsPatient->Fields->Item["Zip"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,15,'L',' ');
		OutputString += ParseField("",3,'L',' ');

		//PersonT.HomePhone
		var = rsPatient->Fields->Item["HomePhone"]->Value;
		if(var.vt==VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str = StripNonNum(str);
		OutputString += ParseField(str,15,'L',' ');
		OutputString += ParseField("",7,'L',' ');

		//PatientsT.Employment
		var = rsPatient->Fields->Item["Employment"]->Value;
		if(var.vt==VT_I4) {
			long emp = var.lVal;
			if(emp==1)
				str="X";
			else if(emp==2)
				str="        X";
			else if(emp==3)
				str="               X";
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,19,'L',' ');

		//Insurance Information

		//PersonT.Zip
		if(m_HCFAInfo.HideBox7 == 0) {
			var = rsIns->Fields->Item["Zip"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,16,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		//PersonT.HomePhone
		if(m_HCFAInfo.HideBox7 == 0) {
			var = rsIns->Fields->Item["HomePhone"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		str = StripNonNum(str);
		OutputString += ParseField(str,15,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 10   //////////////////////////////////


		//Line 11 (double)   /////////////////////////////////////////
		OutputString = "";
		
		_RecordsetPtr rsOthrIns;

		if(m_pEBillingInfo->OthrInsuredPartyID!=-1) {
			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			rsOthrIns = CreateParamRecordset("SELECT * FROM PersonT INNER JOIN InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID WHERE ID = {INT}",m_pEBillingInfo->OthrInsuredPartyID);
			if(rsOthrIns->eof) {
				//if the recordset is empty, there is no insured party. So halt everything!!!
				rsOthrIns->Close();
				// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
				CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
				if(!strPatientName.IsEmpty()) {
					// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
					str.Format("Could not open secondary insurance information from this patient's bill for '%s', Bill ID %li. (Legacy_HCFABoxes1to11)", strPatientName, m_pEBillingInfo->BillID);
				}
				else
					//serious problems if you get here
					str = "Could not open secondary insurance information from this patient's bill. (Legacy_HCFABoxes1to11)";

				AfxMessageBox(str);
				return Error_Missing_Info;
			}
		}

		//Other Insurance Information

		//PersonT.Last
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1 && rsOthrIns->Fields->Item["Last"]->Value.vt==VT_BSTR)
			str = CString(rsOthrIns->Fields->Item["Last"]->Value.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,16,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.First
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1 && rsOthrIns->Fields->Item["First"]->Value.vt==VT_BSTR)
			str = CString(rsOthrIns->Fields->Item["First"]->Value.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,13,'L',' ');
		OutputString += ParseField("",1,'L',' ');

		//PersonT.Middle
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1 && rsOthrIns->Fields->Item["Middle"]->Value.vt==VT_BSTR)
			str = CString(rsOthrIns->Fields->Item["Middle"]->Value.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,1,'L',' ');
		OutputString += ParseField("",27,'L',' ');

		//InsuredPartyT.PolicyGroupNum (Primary Ins.)

		str = "";

		// (j.jones 2006-10-31 15:49) - PLID 21845 - check the secondary insurance's HCFA group,
		// and see if we need to use the secondary insurance's information in Box 11. If so,
		// this will override the Hide Box 11 option
		BOOL bShowSecondaryInBox11 = FALSE;
		if(m_pEBillingInfo->OthrInsuredPartyID != -1) {
			if(ReturnsRecords("SELECT ID FROM HCFASetupT WHERE SecondaryFillBox11 = 1 AND "
				"ID IN (SELECT HCFASetupGroupID FROM InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
				"WHERE InsuredPartyT.PersonID = %li)", m_pEBillingInfo->OthrInsuredPartyID)) {
				//there is a secondary insurance and its HCFA group says to overwrite Box 11
				bShowSecondaryInBox11 = TRUE;
			}
		}

		if(bShowSecondaryInBox11) {

			//if we are showing the secondary information in Box 11, that takes
			//precedence over all other Box 11 possibilities
			str = AdoFldString(rsOthrIns, "IDForInsurance", "");
		}
		else {

			//check for the HCFA group's override
  			BOOL bOverride = FALSE;

			// (j.jones 2006-03-13 14:44) - PLID 19555 - the Box 11 default overrides
			// the "Hide Box 11" options. If they want Box 11 to be truly blank, then they
			// should have it hidden and have no override.
			if(m_HCFAInfo.Box11Rule == 1) {
				bOverride = TRUE;
			}
			else {			
				//use only if blank (default)
				var = rsIns->Fields->Item["PolicyGroupNum"]->Value;
				if(var.vt == VT_BSTR) {
					str = CString(var.bstrVal);
					str.TrimLeft(" ");
					str.TrimRight(" ");
					if(str.GetLength()==0)
						bOverride = TRUE;
				}
				else
					bOverride = TRUE;
			}

			if(bOverride) {
				str = m_HCFAInfo.Box11;
			}
			else if(m_HCFAInfo.HideBox11 == 1) {
				str = "";
			}
		}

		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 11   //////////////////////////////////

		//Line 12 (double)   /////////////////////////////////////////
		OutputString = "";

		//Other Insurance Information

		//InsuredPartyT.IDForInsurance
		if(m_HCFAInfo.Use1aIn9a && (m_pEBillingInfo->OthrInsuredPartyID != -1 || m_HCFAInfo.Use1aIn9aAlways)) {
			//InsuredPartyT.IDForInsurance
			var = rs->Fields->Item["IDForInsurance"]->Value;
			str = VarString(var, "");
		}
		// (j.jones 2010-08-31 16:54) - PLID 40303 - supported TPL in 9a, which is mutually exclusive of Use1aIn9a
		else if (!m_HCFAInfo.Use1aIn9a && m_HCFAInfo.TPLIn9a == 1 && m_pEBillingInfo->OthrInsuredPartyID != -1) {
			str = AdoFldString(rsOtherIns, "TPLCode", "");
		}
		else {
			if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID != -1 && rsOthrIns->Fields->Item["IDForInsurance"]->Value.vt==VT_BSTR)
				str = CString(rsOthrIns->Fields->Item["IDForInsurance"]->Value.bstrVal);
			else
				str = "";
		}
		OutputString += ParseField(str,32,'L',' ');
		OutputString += ParseField("",8,'L',' ');

		rs->Close();

		if(m_pEBillingInfo->Box33Setup == 2) {
			//since 2 is the general 1 provider, we do not separate charges at all
			// (j.jones 2007-10-05 10:50) - PLID 27659 - added ChargeID as a field
			// (j.jones 2007-10-15 15:03) - PLID 27757 - added ServiceID as a field
			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			// (j.jones 2009-03-09 09:10) - PLID 33354 - added insured party accident information
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			// (j.gruber 2014-03-17 11:32) - PLID 61395 - update image for billing structure
			// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - DiagCodes and WhichCodes must be unique and filtered for NULLs. Use ChargeWhichICD9CodesIF/10 and the BillDiags
			// (j.jones 2016-04-11 17:22) - NX-100149 - Box15Date is now calculated from ConditionDateType
			rsBills = CreateParamRecordset("SELECT GetDate() AS TodaysDate, ChargesT.ID AS ChargeID, ChargesT.ServiceID, BillsT.*, ChargesT.*, LineItemT.*, dbo.GetChargeTotal(ChargesT.ID) AS ChargeTot, "
									"PlaceOfServiceCodesT.PlaceCodes, "
									"ServiceT.Anesthesia, ServiceT.UseAnesthesiaBilling, InsuredPartyT.AccidentType, InsuredPartyT.AccidentState, InsuredPartyT.DateOfCurAcc, ChargeWhichCodesQ.WhichCodes as ChargeWhichCodes, "
									"CASE BillsT.ConditionDateType "
									"WHEN {CONST_INT} THEN BillsT.FirstVisitOrConsultationDate "
									"WHEN {CONST_INT} THEN BillsT.InitialTreatmentDate "
									"WHEN {CONST_INT} THEN BillsT.LastSeenDate "
									"WHEN {CONST_INT} THEN BillsT.AcuteManifestationDate "
									"WHEN {CONST_INT} THEN BillsT.LastXRayDate "
									"WHEN {CONST_INT} THEN BillsT.HearingAndPrescriptionDate "
									"WHEN {CONST_INT} THEN BillsT.AssumedCareDate "
									"WHEN {CONST_INT} THEN BillsT.RelinquishedCareDate "
									"WHEN {CONST_INT} THEN BillsT.AccidentDate "
									"ELSE NULL END AS Box15Date "
									"FROM BillsT LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
									"INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
									"LEFT JOIN CPTModifierT ON CPTModifier = CPTModifierT.Number "
									"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
									"OUTER APPLY dbo.{CONST_STRING}({STRING}, ChargesT.ID) ChargeWhichCodesQ "
									"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
									"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
									"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
									"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
									"WHERE BillsT.ID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
									"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
									"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
									"ORDER BY LineID",
									ConditionDateType::cdtFirstVisitOrConsultation444,
									ConditionDateType::cdtInitialTreatmentDate454,
									ConditionDateType::cdtLastSeenDate304,
									ConditionDateType::cdtAcuteManifestation453,
									ConditionDateType::cdtLastXray455,
									ConditionDateType::cdtHearingAndPrescription471,
									ConditionDateType::cdtAssumedCare090,
									ConditionDateType::cdtRelinquishedCare91,
									ConditionDateType::cdtAccident439
									, ShouldUseICD10() ? "ChargeWhichICD10CodesIF" : "ChargeWhichICD9CodesIF"
									, MakeBillDiagsXml(m_pEBillingInfo->billDiags)
									,m_pEBillingInfo->BillID);
		}
		else {
			//separate the charges per doctor - the bill should already be separated in the loaded claims

			// (j.jones 2006-12-01 15:50) - PLID 22110 - supported the ClaimProviderID
			// (j.jones 2007-10-05 10:50) - PLID 27659 - added ChargeID as a field
			// (j.jones 2007-10-15 15:03) - PLID 27757 - added ServiceID as a field
			// (j.jones 2008-04-03 09:23) - PLID 28995 - supported the HCFAClaimProvidersT setup
			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			// (j.jones 2008-08-01 10:17) - PLID 30917 - HCFAClaimProvidersT is now per insurance company
			// (j.jones 2009-03-09 09:10) - PLID 33354 - added insured party accident information
			// (j.jones 2010-11-09 12:57) - PLID 41389 - supported ChargesT.ClaimProviderID
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			// (j.gruber 2014-03-17 11:34) - PLID 61395 - update image for billing structure change
			// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - DiagCodes and WhichCodes must be unique and filtered for NULLs. Use ChargeWhichICD9CodesIF/10 and the BillDiags
			// (j.jones 2016-04-11 17:22) - NX-100149 - Box15Date is now calculated from ConditionDateType
			rsBills = CreateParamRecordset("SELECT GetDate() AS TodaysDate, ChargesT.ID AS ChargeID, ChargesT.ServiceID, BillsT.*, ChargesT.*, LineItemT.*, dbo.GetChargeTotal(ChargesT.ID) AS ChargeTot, "
									"PlaceOfServiceCodesT.PlaceCodes, "
									"ServiceT.Anesthesia, ServiceT.UseAnesthesiaBilling, InsuredPartyT.AccidentType, InsuredPartyT.AccidentState, InsuredPartyT.DateOfCurAcc, ChargeWhichCodesQ.WhichCodes as ChargeWhichCodes, "
									"CASE BillsT.ConditionDateType "
									"WHEN {CONST_INT} THEN BillsT.FirstVisitOrConsultationDate "
									"WHEN {CONST_INT} THEN BillsT.InitialTreatmentDate "
									"WHEN {CONST_INT} THEN BillsT.LastSeenDate "
									"WHEN {CONST_INT} THEN BillsT.AcuteManifestationDate "
									"WHEN {CONST_INT} THEN BillsT.LastXRayDate "
									"WHEN {CONST_INT} THEN BillsT.HearingAndPrescriptionDate "
									"WHEN {CONST_INT} THEN BillsT.AssumedCareDate "
									"WHEN {CONST_INT} THEN BillsT.RelinquishedCareDate "
									"WHEN {CONST_INT} THEN BillsT.AccidentDate "
									"ELSE NULL END AS Box15Date "
									"FROM BillsT LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
									"INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
									"LEFT JOIN CPTModifierT ON CPTModifier = CPTModifierT.Number "
									"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
									"OUTER APPLY dbo.{CONST_STRING}({STRING}, ChargesT.ID) ChargeWhichCodesQ "
									"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
									"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
									"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
									"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
									"WHERE BillsT.ID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
									"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
									"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
									"AND (ChargesT.ClaimProviderID = {INT} OR (ChargesT.ClaimProviderID Is Null AND ChargesT.DoctorsProviders IN (SELECT PersonID FROM "
									"	(SELECT ProvidersT.PersonID, "
									"	(CASE WHEN Box33_ProviderID Is Null THEN ProvidersT.ClaimProviderID ELSE Box33_ProviderID END) AS ProviderIDToUse "
									"	FROM ProvidersT "
									"	LEFT JOIN (SELECT * FROM HCFAClaimProvidersT WHERE InsuranceCoID = {INT}) AS HCFAClaimProvidersT ON ProvidersT.PersonID = HCFAClaimProvidersT.ProviderID) SubQ "
									"	WHERE ProviderIDToUse = {INT}))) "
									"ORDER BY LineID",
									ConditionDateType::cdtFirstVisitOrConsultation444,
									ConditionDateType::cdtInitialTreatmentDate454,
									ConditionDateType::cdtLastSeenDate304,
									ConditionDateType::cdtAcuteManifestation453,
									ConditionDateType::cdtLastXray455,
									ConditionDateType::cdtHearingAndPrescription471,
									ConditionDateType::cdtAssumedCare090,
									ConditionDateType::cdtRelinquishedCare91,
									ConditionDateType::cdtAccident439
									, ShouldUseICD10() ? "ChargeWhichICD10CodesIF" : "ChargeWhichICD9CodesIF"
									, MakeBillDiagsXml(m_pEBillingInfo->billDiags)
									, m_pEBillingInfo->BillID, m_pEBillingInfo->ProviderID, m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);
		}

		if(rsBills->eof) {
			//if the recordset is empty, there is no bill information. So halt everything!!!
			rsBills->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open bill information for '%s', Bill ID %li. (Legacy_HCFABoxes1to11)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open bill information. (Legacy_HCFABoxes1to11)";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		// (j.jones 2009-03-09 09:11) - PLID 33354 - we now may use the insured party accident type instead
		InsuredPartyAccidentType ipatAccType = (InsuredPartyAccidentType)AdoFldLong(rsBills, "AccidentType", (long)ipatNone);		

		BOOL bRelatedToEmp = FALSE;
		
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//pull from the insured party, not the bill
			bRelatedToEmp = (ipatAccType == ipatEmployment);
		}
		else {
			//pull from BillsT.RelatedToEmp
			bRelatedToEmp = AdoFldBool(rsBills, "RelatedToEmp", FALSE);
		}

		if(bRelatedToEmp) {
			str = "X";
		}
		else {
			str = "        X";
		}
		OutputString += ParseField(str,21,'L',' ');

		//Insurance Information

		str = "";
		
		//PersonT.BirthDate
		if(bShowSecondaryInBox11) {

			// (j.jones 2006-10-31 15:49) - PLID 21845 - if we are showing the secondary
			//information in Box 11, that takes precedence over all other Box 11 possibilities
			var = rsOthrIns->Fields->Item["BirthDate"]->Value;
			if(var.vt==VT_DATE) {
				COleDateTime dt;
				dt = var.date;

				if(m_HCFAInfo.Box11aE == 1)
					str = dt.Format("%Y%m%d");
				else
					str = dt.Format("%m%d%Y");
			}
			else
				str = "";
		}
		else {

			//Box11a

			// (j.jones 2007-04-09 17:29) - PLID 25537 - supported hiding Box11a Birthdate individually
			if(m_HCFAInfo.HideBox11a_Birthdate == 0) {
				var = rsIns->Fields->Item["BirthDate"]->Value;
				if(var.vt==VT_DATE) {
					COleDateTime dt;
					dt = var.date;

					if(m_HCFAInfo.Box11aE == 1)
						str = dt.Format("%Y%m%d");
					else
						str = dt.Format("%m%d%Y");
				}
				else
					str = "";
			}
			else
				str = "";
		}
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",9,'L',' ');

		//PersonT.Gender
		if(bShowSecondaryInBox11) {

			// (j.jones 2006-10-31 15:49) - PLID 21845 - if we are showing the secondary
			//information in Box 11, that takes precedence over all other Box 11 possibilities
			var = rsOthrIns->Fields->Item["Gender"]->Value;				
			Gender = VarByte(var,0);
			if(Gender==1)
				str = "X";
			else if(Gender==2)
				str = "        X";
			else
				str = "";
		}
		else {
			// (j.jones 2007-04-09 17:29) - PLID 25537 - supported hiding Box11a Gender individually
			if(m_HCFAInfo.HideBox11a_Gender == 0) {
				var = rsIns->Fields->Item["Gender"]->Value;
				
				Gender = VarByte(var,0);
				if(Gender==1)
					str = "X";
				else if(Gender==2)
					str = "        X";
				else
					str = "";
			}
			else
				str = "";
		}
		OutputString += ParseField(str,9,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 12   //////////////////////////////////

		//Line 13 (double)   /////////////////////////////////////////
		OutputString = "";

		//Other Insurance Information

		//PersonT.BirthDate
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1 && rsOthrIns->Fields->Item["BirthDate"]->Value.vt==VT_DATE) {
			COleDateTime dt;
			dt = rsOthrIns->Fields->Item["BirthDate"]->Value.date;
			
			if(m_HCFAInfo.Box9bE == 1)
				str = dt.Format("%Y%m%d");
			else
				str = dt.Format("%m%d%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",9,'L',' ');

		//PersonT.Gender
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1) {
			int Gender = AdoFldByte(rsOthrIns, "Gender",0);
			if(Gender==1)
				str = "X";
			else if(Gender==2)
				str = "        X";
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,21,'L',' ');

		BOOL bRelatedToAutoAcc = FALSE;
		
		// (j.jones 2009-03-09 09:30) - PLID 33354 - support the insured party accident option
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//pull from the insured party, not the bill
			bRelatedToAutoAcc = (ipatAccType == ipatAutoAcc);
		}
		else {
			//pull from BillsT.RelatedToAutoAcc
			bRelatedToAutoAcc = AdoFldBool(rsBills, "RelatedToAutoAcc", FALSE);
		}

		if(bRelatedToAutoAcc) {
			str = "X";
		}
		else {
			str = "        X";
		}
		OutputString += ParseField(str,12,'L',' ');

		//BillsT.State
		CString strState = "";

		// (j.jones 2009-03-09 09:30) - PLID 33354 - support the insured party accident option
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//pull from the insured party, not the bill
			strState = AdoFldString(rsBills, "AccidentState", "");
		}
		else {
			//pull from BillsT.State
			strState = AdoFldString(rsBills, "State", "");
		}

		OutputString += ParseField(strState,2,'L',' ');
		OutputString += ParseField("",5,'L',' ');

		//Insurance Information

		//InsuredPartyT.Employer
		if(bShowSecondaryInBox11) {

			// (j.jones 2006-10-31 15:49) - PLID 21845 - if we are showing the secondary
			//information in Box 11, that takes precedence over all other Box 11 possibilities
			str = AdoFldString(rsOthrIns, "Employer","");
		}
		else {
			// (j.jones 2007-04-09 17:29) - PLID 25537 - supported hiding Box11b individually
			if(m_HCFAInfo.HideBox11b == 0) {
				var = rsIns->Fields->Item["Employer"]->Value;
				if(var.vt==VT_BSTR) {
					str = CString(var.bstrVal);
				}
				else
					str = "";
			}
			else
				str = "";
		}
		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 13   //////////////////////////////////


		//Line 14 (double)   /////////////////////////////////////////
		OutputString = "";

		//Other Insurance Information

		//InsuredPartyT.Employer
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1 && rsOthrIns->Fields->Item["Employer"]->Value.vt==VT_BSTR) {
			str = CString(rsOthrIns->Fields->Item["Employer"]->Value.bstrVal);
		}
		else
			str = "";
		OutputString += ParseField(str,32,'L',' ');
		OutputString += ParseField("",8,'L',' ');

		BOOL bRelatedToOther = FALSE;
		
		// (j.jones 2009-03-09 09:30) - PLID 33354 - support the insured party accident option
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//pull from the insured party, not the bill
			bRelatedToOther = (ipatAccType == ipatOtherAcc);
		}
		else {
			//pull from BillsT.RelatedToOther
			bRelatedToOther = AdoFldBool(rsBills, "RelatedToOther", FALSE);
		}

		if(bRelatedToOther) {
			str = "X";
		}
		else {
			str = "        X";
		}
		OutputString += ParseField(str,19,'L',' ');

		//InsurancePlansT.PlanName
		if(bShowSecondaryInBox11) {

			// (j.jones 2006-10-31 15:49) - PLID 21845 - if we are showing the secondary
			//information in Box 11, that takes precedence over all other Box 11 possibilities
			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			_RecordsetPtr rsInsPlan = CreateParamRecordset("SELECT PlanName FROM InsurancePlansT WHERE InsurancePlansT.ID = {INT}", AdoFldLong(rsOthrIns, "InsPlan", -1));
			if(!rsInsPlan->eof){
				str = AdoFldString(rsInsPlan, "PlanName", "");
			}
			else{
				str = "";
			}
			rsInsPlan->Close();
		}
		else {
			// (j.jones 2007-04-09 17:29) - PLID 25537 - supported hiding Box11c individually
			if(m_HCFAInfo.HideBox11c == 0) {
				// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
				_RecordsetPtr rsInsPlan = CreateParamRecordset("SELECT PlanName FROM InsurancePlansT WHERE InsurancePlansT.ID = {INT}", AdoFldLong(rsIns, "InsPlan", -1));
				if(!rsInsPlan->eof){
					str = AdoFldString(rsInsPlan, "PlanName", "");
				}
				else{
					str = "";
				}
				rsInsPlan->Close();
			}
			else
				str = "";
		}
		OutputString += ParseField(str,32,'L',' ');


		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 14   //////////////////////////////////


		//Line 15 (triple)  /////////////////////////////////////////
		OutputString = "";

		//Other Ins Plan
		//InsurancePlansT.PlanName
		if(m_HCFAInfo.HideBox9 == 0 && m_pEBillingInfo->OthrInsuredPartyID!=-1){
			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			_RecordsetPtr rsOtherInsPlan = CreateParamRecordset("SELECT PlanName FROM InsurancePlansT WHERE InsurancePlansT.ID = {INT}", AdoFldLong(rsOthrIns, "InsPlan", -1));
			if(!rsOtherInsPlan->eof){
				str = AdoFldString(rsOtherInsPlan, "PlanName", "");
			}
			else{
				str = "";
			}
			rsOtherInsPlan->Close();
			rsOthrIns->Close();
		}
		else{
			str = "";
		}
		OutputString += ParseField(str,36,'L',' ');

		//BillsT.HCFABox10D
		str = AdoFldString(rsBills, "HCFABox10D","");
		OutputString += ParseField(str,25,'L',' ');

		rsIns->Close();

		//Other Ins

		//Box11D: 0 = fill normally, 1 = fill if no secondary, 2 = never fill
		if(m_HCFAInfo.Box11D == 2) {
			str = "";
		}
		else {
			if(m_pEBillingInfo->OthrInsuredPartyID != -1){
				str = "X";
			}
			else{
				if(m_HCFAInfo.Box11D == 0) {
					str = "       X";
				}
				else {
					str = "";
				}
			}
		}
		OutputString += ParseField(str,8,'L',' ');

		OutputString+= "\r\n\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 15 (triple)  //////////////////////////////////

		return Success;

	}NxCatchAll("Error in Ebilling::Legacy_HCFABoxes1to11");

	return Error_Other;
}

int CEbilling::Legacy_HCFABoxes12to23(_RecordsetPtr &rsBills)
{
	//this exports fields 12 to 23, HCFA Image format

	try {

		_variant_t var;

		//Line 16 (double)  /////////////////////////////////////////
		CString str, OutputString = "";
		OutputString += ParseField("",9,'L',' ');

		// (j.jones 2010-07-23 15:02) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsCo now
		BOOL bAccepted = GetAcceptAssignment_ByInsCo(m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);

		// (j.jones 2010-07-23 10:52) - PLID 39795 - fixed to follow the HCFA group setting for filling Box 12
		BOOL bFillBox12 = TRUE;
		if(m_HCFAInfo.Box12Accepted == 0) {
			//never fill
			bFillBox12 = FALSE;
		}
		else if(m_HCFAInfo.Box12Accepted == 1) {
			//fill if accepted
			bFillBox12 = bAccepted;
		}
		else if(m_HCFAInfo.Box12Accepted == 2) {
			//fill if not accepted
			bFillBox12 = !bAccepted;
		}
		else if(m_HCFAInfo.Box12Accepted == 3) {
			//always fill
			bFillBox12 = TRUE;
		}

		//Box 12
		if(bFillBox12) {
			str = "SIGNATURE ON FILE";
		}
		else {
			str = "";
		}
		OutputString += ParseField(str,25,'L',' ');
		OutputString += ParseField("",14,'L',' ');

		//BillsT.Date
		//0 - the Print Date (today), 1 - the Bill Date
		if(!bFillBox12) {
			str = "";
		}
		else {
			if(m_HCFAInfo.Box12UseDate == 0)
				var = rsBills->Fields->Item["TodaysDate"]->Value;			
			else
				var = rsBills->Fields->Item["Date"]->Value;
			if(var.vt==VT_DATE) {
				COleDateTime dt;
				dt = var.date;
				str = dt.Format("%m/%d/%Y");
			}
			else
				str = "";
		}
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",10,'L',' ');

		// (j.jones 2010-07-22 12:01) - PLID 39780 - fixed to follow the HCFA group setting for filling Box 13
		BOOL bFillBox13 = TRUE;
		if(m_HCFAInfo.Box13Accepted == 0) {
			//never fill
			bFillBox13 = FALSE;
		}
		else if(m_HCFAInfo.Box13Accepted == 1) {
			//fill if accepted
			bFillBox13 = bAccepted;
		}
		else if(m_HCFAInfo.Box13Accepted == 2) {
			//fill if not accepted
			bFillBox13 = !bAccepted;
		}
		else if(m_HCFAInfo.Box13Accepted == 3) {
			//always fill
			bFillBox13 = TRUE;
		}

		// (j.jones 2010-06-10 12:53) - PLID 39095 - This is Box 13. We will output "SIGNATURE ON FILE" if the provider accepts assignment,
		// and have a per-bill setting to optionally override this to be blank or not.
		HCFABox13Over hb13Value = (HCFABox13Over)AdoFldLong(rsBills, "HCFABox13Over", (long)hb13_UseDefault);		
		if(hb13Value == hb13_No || (hb13Value == hb13_UseDefault && !bFillBox13)) {
			str = "";
		}
		else {
			str = "SIGNATURE ON FILE";
		}
		//Box 13
		OutputString += ParseField(str,21,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 16 (double)  /////////////////////////////////

		//Line 17 (double) /////////////////////////////////////////
		OutputString = "";

		//BillsT.ConditionDate
		
		// (j.jones 2009-03-09 09:39) - PLID 33354 - supported the option to load the accident from the insured party		
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//load from the insured party
			var = rsBills->Fields->Item["DateOfCurAcc"]->Value;
		}
		else {
			//load from the bill
			var = rsBills->Fields->Item["ConditionDate"]->Value;
		}

		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",36,'L',' ');

		// (j.jones 2016-04-11 17:22) - NX-100149 - this is now the calculated Box 15 date,
		// based on the value of BillsT.ConditionDateType
		var = rsBills->Fields->Item["Box15Date"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",7,'L',' ');

		//BillsT.NoWorkFrom
		var = rsBills->Fields->Item["NoWorkFrom"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",6,'L',' ');

		//BillsT.NoWorkTo
		var = rsBills->Fields->Item["NoWorkTo"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 17 (double) //////////////////////////////////

		//Line 18 (double) /////////////////////////////////////////
		OutputString = "";

		// (j.jones 2009-02-06 16:29) - PLID 32951 - this function now requires the bill location
		// (j.jones 2009-02-23 10:54) - PLID 33167 - now we require the billing provider ID and rendering provider ID,
		// in this case the Rendering Provider ID is the Box 24J provider ID
		// (j.jones 2011-04-05 13:39) - PLID 42372 - this now returns a struct of information
		ECLIANumber eCLIANumber = UseCLIANumber(m_pEBillingInfo->BillID,m_pEBillingInfo->InsuredPartyID,m_pEBillingInfo->ProviderID,m_pEBillingInfo->Box24J_ProviderID,m_pEBillingInfo->BillLocation);

		_RecordsetPtr rsRefPhy;

		BOOL bUseProvAsRefPhy = (eCLIANumber.bUseCLIANumber && eCLIANumber.bCLIAUseBillProvInHCFABox17);

		if(bUseProvAsRefPhy) {
			//use the provider info instead of the referring physician
			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			rsRefPhy = CreateParamRecordset("SELECT [Last] + ', ' + [First] + ' ' + Middle AS NameLFM, [First] + ' ' + [Middle] + ' ' + Last AS NameFML, ProvidersT.* FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
				"WHERE PersonID = {INT}",m_pEBillingInfo->ProviderID);
		}
		else {
			//use the referring physician normally
			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			rsRefPhy = CreateParamRecordset("SELECT [Last] + ', ' + [First] + ' ' + Middle AS NameLFM, [First] + ' ' + [Middle] + ' ' + Last AS NameFML, ReferringPhysT.* FROM PersonT INNER JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
				"INNER JOIN BillsT ON PersonT.ID = BillsT.RefPhyID WHERE BillsT.ID = {INT}",m_pEBillingInfo->BillID);
		}

		//Referring Physician Info

		//PersonT.Last, First Middle
		str = "";
		if(!rsRefPhy->eof) {
			if(m_HCFAInfo.Box17Order == 1)
				str = AdoFldString(rsRefPhy, "NameLFM","");
			else
				str = AdoFldString(rsRefPhy, "NameFML","");
		}
		OutputString += ParseField(str,32,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		//ReferringPhyID
		//this is the ID specified by the setup of Box17

		if(bUseProvAsRefPhy) {

			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			m_rsDoc = CreateParamRecordset("SELECT * FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE ProvidersT.PersonID = {INT}",m_pEBillingInfo->ProviderID);

			if(m_rsDoc->eof) {
				//if the recordset is empty, there is no doctor. So halt everything!!!
				m_rsDoc->Close();
				// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
				CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
				if(!strPatientName.IsEmpty()) {
					// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
					str.Format("Could not open provider information for patient '%s', Bill ID %li. (Legacy_HCFABoxes12to23)", strPatientName, m_pEBillingInfo->BillID);
				}
				else
					//serious problems if you get here
					str = "Could not open provider information. (Legacy_HCFABoxes12to23)";

				if(m_pEBillingInfo->Box33Setup==2)
					str += "\nIt is possible that you have no provider selected in General 1 for this patient.";
				else
					str += "\nIt is possible that you have no provider selected on this patient's bill.";
				
				AfxMessageBox(str);
				return Error_Missing_Info;
			}

			//provider
			str = Box33Pin();

			m_rsDoc->Close();
		}
		else {

			if(m_HCFAInfo.UseBox23InBox17a == 0) {
				//ref phy ID (normal case)
				str = Box17a();
			}
			else {
				//if UseBox23InBox17a = 1 or 2 then we use the Prior Auth Num from Box 23
				//BillsT.PriorAuthNum
				var = rsBills->Fields->Item["PriorAuthNum"]->Value;
				if(var.vt == VT_BSTR){
					str = CString(var.bstrVal);
				}
				else{
					str = "";
				}

				//use the override from the HCFA setup
				if(str == "")
					str = m_HCFAInfo.DefaultPriorAuthNum;
			}
		}

		OutputString += ParseField(str,26,'L',' ');
		OutputString += ParseField("",3,'L',' ');
		
		rsRefPhy->Close();		

		//BillsT.HospFrom
		var = rsBills->Fields->Item["HospFrom"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",6,'L',' ');

		//BillsT.HospTo
		var = rsBills->Fields->Item["HospTo"]->Value;
		if(var.vt==VT_DATE) {
			COleDateTime dt;
			dt = var.date;
			str = dt.Format("%m/%d/%Y");
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');
		

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 18 (double) //////////////////////////////////

		//Line 19 (double) /////////////////////////////////////////
		OutputString = "";

		//BillsT.HCFABlock19
		var = rsBills->Fields->Item["HCFABlock19"]->Value;
		if(var.vt == VT_BSTR){
			str = CString(var.bstrVal);
		}
		else{
			str = "";
		}

		if(str == "" && m_HCFAInfo.Box19RefPhyID == 1)
			str = Box17a();	//use the Ref Phy ID
		else if(str == "" && m_HCFAInfo.Box19Override == 1)
			str = m_HCFAInfo.Box19;

		//check for the override
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		_RecordsetPtr rsTemp = CreateParamRecordset("SELECT Box19 FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
		if(!rsTemp->eof) {
			var = rsTemp->Fields->Item["Box19"]->Value;
			if(var.vt == VT_BSTR) {
				CString strTemp = CString(var.bstrVal);
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty())
					str = strTemp;
			}
		}
		rsTemp->Close();

		OutputString += ParseField(str,60,'L',' ');
		OutputString += ParseField("",4,'L',' ');

		//BillsT.OutsideLab
		var = rsBills->Fields->Item["OutsideLab"]->Value;
		if(var.vt == VT_BOOL){
			if(var.boolVal){
				str = "X";
			}
			else{
				str = "    X";
			}
		}
		else{
			str = "    X";
		}

		OutputString += ParseField(str,9,'L',' ');

		BOOL bUseDecimals = (GetRemotePropertyInt("HCFAImageUseDecimals",0,0,"<None>",true) == 1);

		//BillsT.OutLabCharges
		var = rsBills->Fields->Item["OutsideLabCharges"]->Value;
		if(var.vt == VT_CY){
			str = FormatCurrencyForInterface(var.cyVal,FALSE,FALSE);
			if(!bUseDecimals)
				str = StripNonNum(str);
		}
		else{
			str = "0";
		}
		
		if(!bUseDecimals)
			OutputString += ParseField(str,6,'R','0');
		else
			OutputString += ParseField(str,6,'R',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 19 (double) //////////////////////////////////


		//For the next two lines we're using charge information common to all charges, the 
		//looping will be later.

		//Line 20 (double) /////////////////////////////////////////
		OutputString = "";
		
		//Spaces
		OutputString += ParseField("", 2, 'L', ' ');

		//BillsT.Diag1ID

		// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(0).number;

		OutputString += ParseField(str, 6, 'L', ' ');

		if(m_HCFAInfo.ShowICD9Desc) {
			str = m_pEBillingInfo->GetSafeBillDiag(0).description;
			OutputString += ParseField("", 3, 'L', ' ');
			OutputString += ParseField(str, 20, 'L', ' ');
			OutputString += ParseField("", 4, 'L', ' ');
		}
		else {
			OutputString += ParseField("", 27, 'L', ' ');
		}

		//1 = diag codes should be 1, 3, 2, 4
		//0 = diag codes should be 1, 2, 3, 4
		if(GetRemotePropertyInt("HCFAImageDiagCodeSetup",0, 0, "<None>", true) == 1) {
			//BillsT.Diag3ID

			// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - Get diag info from bill
			str = m_pEBillingInfo->GetSafeBillDiag(2).number;

			OutputString += ParseField(str, 6, 'L', ' ');

			if(m_HCFAInfo.ShowICD9Desc) {
				str = m_pEBillingInfo->GetSafeBillDiag(2).description;
				OutputString += ParseField("", 3, 'L', ' ');
				OutputString += ParseField(str, 12, 'L', ' ');
				OutputString += ParseField("", 4, 'L', ' ');
			}
			else {
				OutputString += ParseField("", 19, 'L', ' ');
			}
		}
		else {
			//BillsT.Diag2ID

			// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - Get diag info from bill
			str = m_pEBillingInfo->GetSafeBillDiag(1).number;

			OutputString += ParseField(str, 6, 'L', ' ');

			if(m_HCFAInfo.ShowICD9Desc) {
				str = m_pEBillingInfo->GetSafeBillDiag(1).description;
				OutputString += ParseField("", 3, 'L', ' ');
				OutputString += ParseField(str, 12, 'L', ' ');
				OutputString += ParseField("", 4, 'L', ' ');
			}
			else {
				OutputString += ParseField("", 19, 'L', ' ');
			}			
		}

		//BillsT.MedicaidResubmission
		var = rsBills->Fields->Item["MedicaidResubmission"]->Value;
		if(var.vt == VT_BSTR){
			str = CString(var.bstrVal);
		}
		else{
			str = "";
		}
		OutputString += ParseField(str, 12, 'L', ' ');
		OutputString += ParseField("", 2, 'L', ' ');

		//BillsT.OriginalRefNo
		var = rsBills->Fields->Item["OriginalRefNo"]->Value;
		if(var.vt == VT_BSTR){
			str = CString(var.bstrVal);
		}
		else{
			str = "";
		}
		OutputString += ParseField(str, 13, 'L', ' ');


		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 20 (double) //////////////////////////////////

		//Line 21 (triple) /////////////////////////////////////////
		OutputString = "";

		//Spaces
		OutputString += ParseField("", 2, 'L', ' ');

		//1 = diag codes should be 1, 3, 2, 4
		//0 = diag codes should be 1, 2, 3, 4
		if(GetRemotePropertyInt("HCFAImageDiagCodeSetup",0, 0, "<None>", true) == 1) {
			//BillsT.Diag2ID

			// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - Get diag info from bill
			str = m_pEBillingInfo->GetSafeBillDiag(1).number;

			OutputString += ParseField(str, 6, 'L', ' ');

			if(m_HCFAInfo.ShowICD9Desc) {
				str = m_pEBillingInfo->GetSafeBillDiag(1).description;
				OutputString += ParseField("", 3, 'L', ' ');
				OutputString += ParseField(str, 20, 'L', ' ');
				OutputString += ParseField("", 4, 'L', ' ');
			}
			else {
				OutputString += ParseField("", 27, 'L', ' ');
			}
		}
		else {
			//BillsT.Diag3ID

			// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - Get diag info from bill
			str = m_pEBillingInfo->GetSafeBillDiag(2).number;

			OutputString += ParseField(str, 6, 'L', ' ');

			if(m_HCFAInfo.ShowICD9Desc) {
				str = m_pEBillingInfo->GetSafeBillDiag(2).description;
				OutputString += ParseField("", 3, 'L', ' ');
				OutputString += ParseField(str, 20, 'L', ' ');
				OutputString += ParseField("", 4, 'L', ' ');
			}
			else {
				OutputString += ParseField("", 27, 'L', ' ');
			}			
		}

		//BillsT.Diag4ID

		// (a.walling 2014-03-19 10:10) - PLID 61417 - EBilling - HCFA Image - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(3).number;

		OutputString += ParseField(str, 6, 'L', ' ');

		if(m_HCFAInfo.ShowICD9Desc) {
			str = m_pEBillingInfo->GetSafeBillDiag(3).description;
			OutputString += ParseField("", 3, 'L', ' ');
			OutputString += ParseField(str, 12, 'L', ' ');
			OutputString += ParseField("", 4, 'L', ' ');
		}
		else {
			OutputString += ParseField("", 19, 'L', ' ');
		}

		//if UseBox23InBox17a = 2, then we don't fill Box 23 (though CLIA overrides this)
		if(m_HCFAInfo.UseBox23InBox17a != 2) {
			//BillsT.PriorAuthNum
			var = rsBills->Fields->Item["PriorAuthNum"]->Value;
			if(var.vt == VT_BSTR){
				str = CString(var.bstrVal);
			}
			else{
				str = "";
			}

			//use the override from the HCFA setup
			if(str == "")
				str = m_HCFAInfo.DefaultPriorAuthNum;
		}
		else {
			str = "";
		}

		if(eCLIANumber.bUseCLIANumber && eCLIANumber.bUseCLIAInHCFABox23) {
			//override with the CLIA
			str = eCLIANumber.strCLIANumber;
		}
		
		OutputString += ParseField(str, 32, 'L', ' ');

		OutputString+= "\r\n\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 21 (triple) //////////////////////////////////

		return Success;

	}NxCatchAll("Error in Ebilling::Legacy_HCFABoxes12to23");

	return Error_Other;
}

int CEbilling::Legacy_HCFABoxes24to33(_RecordsetPtr &rsBills, long nCurPage, long nPages)
{
	//this exports fields 24 to 33, HCFA Image format

	try {

		int ChargesPerPage = GetRemotePropertyInt("HCFAImageChargesPerPage", 6, 0, "<None>", true);

		BOOL bNeedPrintAnesthesiaTimes = FALSE;

		BOOL bUseDecimals = (GetRemotePropertyInt("HCFAImageUseDecimals",0,0,"<None>",true) == 1);

		_variant_t var;

		CString str, OutputString;
		int chargecount = ChargesPerPage;

		//JJ - we must only output 6 charges per claim, so we output the amount of charges on the current page

		//also calculate the total as we go
		COleCurrency cyCharges = COleCurrency(0,0);
		
		long nLastCharge = m_nCurrPage * ChargesPerPage;
		long nFirstCharge = nLastCharge - (ChargesPerPage - 1);
		
		//we will loop from nFirstCharge through nLastCharge, or EOF, whichever comes first;
		
		//first loop up until the charge we need to start on
		//TES 11/5/2007 - PLID 27978 - VS2008 - for() loops
		int charge = 1;
		for(charge = 1; charge < nFirstCharge; charge++) {

			if(rsBills->eof) {
				continue;
			}

			rsBills->MoveNext();
		}

		CString strChargeIDs = "";

		for(charge = nFirstCharge; charge <= nLastCharge; charge++) {

			if(rsBills->eof) {
				continue;
			}

			// (j.jones 2007-10-05 10:52) - PLID 27659 - track the IDs of the charges we are outputting
			long nChargeID = AdoFldLong(rsBills, "ChargeID");
			if(!strChargeIDs.IsEmpty())
				strChargeIDs += ",";
			strChargeIDs += AsString(nChargeID);

			// (j.jones 2007-10-15 15:05) - PLID 27757 - track the ServiceID
			long nServiceID = AdoFldLong(rsBills, "ServiceID");

			//Line 22 (double) /////////////////////////////////////////
			OutputString = "";

			// (j.jones 2008-05-02 10:50) - PLID 28012 - removed the requirement for UseAnesthesiaBilling
			// to be checked for the service
			if(m_pEBillingInfo->AnesthesiaTimes && AdoFldBool(rsBills, "Anesthesia",FALSE)
				/*&& AdoFldBool(rsBills, "UseAnesthesiaBilling",FALSE)*/) {
				bNeedPrintAnesthesiaTimes = TRUE;
			}

			//BillsT.ServiceDateFrom
			var = rsBills->Fields->Item["ServiceDateFrom"]->Value;
			if(var.vt==VT_DATE) {
				COleDateTime dt;
				dt = var.date;
				str = dt.Format("%m/%d/%Y");
			}
			else
				str = "";
			OutputString += ParseField(str,10,'L',' ');
			OutputString += ParseField("",2,'L',' ');
			
			//BillsT.ServiceDateTo
			var = rsBills->Fields->Item["ServiceDateTo"]->Value;
			// (j.jones 2007-04-10 08:44) - PLID 25539 - added ability to hide the ServiceDateTo field
			if(var.vt==VT_DATE && m_HCFAInfo.Hide24ATo == 0) {
				COleDateTime dt;
				dt = var.date;
				str = dt.Format("%m/%d/%Y");
			}
			else
				str = "";
			OutputString += ParseField(str,10,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.ServiceLocation
			var = rsBills->Fields->Item["PlaceCodes"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",1,'L',' ');

			//ChargesT.ServiceType
			var = rsBills->Fields->Item["ServiceType"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",1,'L',' ');

			//ChargesT.ItemCode
			var = rsBills->Fields->Item["ItemCode"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,6,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.CPTModifier
			var = rsBills->Fields->Item["CPTModifier"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.CPTModifier2
			var = rsBills->Fields->Item["CPTModifier2"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.CPTModifier3
			var = rsBills->Fields->Item["CPTModifier3"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.CPTModifier4
			var = rsBills->Fields->Item["CPTModifier4"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,2,'L',' ');
			OutputString += ParseField("",2,'L',' ');


			//ChargesT.WhichCodes
			// (j.gruber 2014-03-17 11:35) - PLID 61395 - update billing structure
			var = rsBills->Fields->Item["ChargeWhichCodes"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
			OutputString += ParseField(str,7,'L',' ');

			// (j.jones 2011-06-17 14:34) - PLID 34795 - I reduced this space from 6 to 5,
			// and increased the charge field from 7 to 8, so if you submit a 10000.00 
			// charge it will take up one more space to the left.
			OutputString += ParseField("",5,'L',' ');

			//we need the Amount * Quantity here, which I calculate in the recordset
			var = rsBills->Fields->Item["ChargeTot"]->Value;
			if(var.vt==VT_CY) {
				cyCharges += var.cyVal;
				str = FormatCurrencyForInterface(var.cyVal,FALSE,FALSE);
				if(!bUseDecimals)
					str = StripNonNum(str);
			}
			else
				str = "";
			if(!bUseDecimals)
				OutputString += ParseField(str,8,'R','0');
			else
				OutputString += ParseField(str,8,'R',' ');
			OutputString += ParseField("",2,'L',' ');

			//ChargesT.Quantity
			var = rsBills->Fields->Item["Quantity"]->Value;
			if(var.vt==VT_R8) {
				str.Format("%g",var.dblVal);
			}
			else
				str = "";

			// (j.jones 2007-01-03 13:26) - PLID 24084 - show minutes as quantity if anesthesia
			// (j.jones 2008-05-02 10:50) - PLID 28012 - removed the requirement for UseAnesthesiaBilling
			// to be checked for the service, and for a flat-fee
			if(m_HCFAInfo.ANSI_UseAnesthMinutesAsQty == 1 && AdoFldBool(rsBills, "Anesthesia", FALSE)
				/*&& AdoFldBool(rsBills, "UseAnesthesiaBilling", FALSE)
				// (j.jones 2007-10-15 14:57) - PLID 27757 - converted to use the new structure
				&& ReturnsRecords("SELECT ID FROM AnesthesiaSetupT WHERE ServiceID = %li AND LocationID = %li AND AnesthesiaFeeBillType <> 1", nServiceID, m_pEBillingInfo->nPOSID)*/) {

				//they want to show minutes, and this is indeed an anesthesia charge that is based on time
				str.Format("%li",AdoFldLong(rsBills, "AnesthesiaMinutes", 0));
			}

			OutputString += ParseField(str,3,'L',' ');
			OutputString += ParseField("",8,'L',' ');

			//InsuranceBox24J.Box24JNumber
			// (j.jones 2008-04-03 10:12) - PLID 28995 - use the Box24J_ProviderID here
			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Box24JNumber FROM InsuranceBox24J WHERE InsCoID = {INT} AND ProviderID = {INT}",
					m_pEBillingInfo->InsuranceCoID,m_pEBillingInfo->Box24J_ProviderID);

			if(!rs->eof && rs->Fields->Item["Box24JNumber"]->Value.vt==VT_BSTR) {
				str = CString(rs->Fields->Item["Box24JNumber"]->Value.bstrVal);
			}
			else
				str = "";

			//check for the override
			// (j.jones 2008-04-03 10:12) - PLID 28995 - use the Box24J_ProviderID here
			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			_RecordsetPtr rsTemp = CreateParamRecordset("SELECT Box24J FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->Box24J_ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
			if(!rsTemp->eof) {
				var = rsTemp->Fields->Item["Box24J"]->Value;
				if(var.vt == VT_BSTR) {
					CString strTemp = CString(var.bstrVal);
					strTemp.TrimLeft();
					strTemp.TrimRight();
					if(!strTemp.IsEmpty())
						str = strTemp;
				}
			}
			rsTemp->Close();

			OutputString += ParseField(str,15,'L',' ');

			rs->Close();

			if(bNeedPrintAnesthesiaTimes) {
				OutputString += "\r\n";
			}
			else {
				OutputString += "\r\n\r\n";
			}

			TRACE(OutputString);
			m_OutputFile.Write(OutputString,OutputString.GetLength());
			//End Of Line 22 (double) //////////////////////////////////

			chargecount--;
			rsBills->MoveNext();

			if(bNeedPrintAnesthesiaTimes) {
				OutputHCFAImageAnesthesiaTimes(m_pEBillingInfo->BillID, m_pEBillingInfo->OnlyAnesthesiaMinutes);
				bNeedPrintAnesthesiaTimes = FALSE;
			}
		}

		rsBills->Close();

		for(int i=0;i<=chargecount;i++) {
			OutputString = "\r\n\r\n";
			TRACE(OutputString);
			m_OutputFile.Write(OutputString,OutputString.GetLength());
		}

		//Line 23 (double) /////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",2,'L',' ');

		//Federal Tax ID Number - SSN or EIN

		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		m_rsDoc = CreateParamRecordset("SELECT * FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE ProvidersT.PersonID = {INT}",m_pEBillingInfo->ProviderID);

		if(m_rsDoc->eof) {
			//if the recordset is empty, there is no doctor. So halt everything!!!
			m_rsDoc->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open provider information for patient '%s', Bill ID %li. (Legacy_HCFABoxes24to33)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open provider information. (Legacy_HCFABoxes24to33)";

			if(m_pEBillingInfo->Box33Setup==2)
				str += "\nIt is possible that you have no provider selected in General 1 for this patient.";
			else
				str += "\nIt is possible that you have no provider selected on this patient's bill.";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		//check for the override
		BOOL bOverride = FALSE;
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		// (j.jones 2008-09-10 11:35) - PLID 30788 - added Box25Check
		_RecordsetPtr rsTemp = CreateParamRecordset("SELECT EIN, Box25Check FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
		long nBox25Check = 0;
		if(!rsTemp->eof) {
			var = rsTemp->Fields->Item["EIN"]->Value;
			if(var.vt == VT_BSTR) {
				CString strTemp = CString(var.bstrVal);
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty()) {
					str = strTemp;
					bOverride = TRUE;
				}
			}
			nBox25Check = AdoFldLong(rsTemp, "Box25Check", 0);
		}
		rsTemp->Close();

		if(m_HCFAInfo.Box25 == 1) { //SSN

			//PersonT.SocialSecurity
			if(!bOverride) {
				var = m_rsDoc->Fields->Item["SocialSecurity"]->Value;
				if(var.vt==VT_BSTR) {
					str = CString(var.bstrVal);
				}
				else
					str = "";
			}
			// (j.jones 2007-06-21 17:13) - PLID 26424 - needs to be 11 characters
			OutputString += ParseField(str,11,'L',' ');
			OutputString += ParseField("",1,'L',' ');

			// (j.jones 2008-09-10 11:36) - PLID 30788 - if the nBox25Check is 2,
			// override with EIN, otherwise it is SSN
			if(nBox25Check == 2) {
				str = "   X";
			}
			else {
				str = "X";
			}
			OutputString += ParseField(str,15,'L',' ');
		}
		else { //Fed Emp. ID

			//ProvidersT.[Fed Employer ID] OR LocationsT.EIN
			if(!bOverride) {

				if(m_HCFAInfo.Box33Setup == 4) {
					//use bill location's EIN
					//DRT 8/25/2008 - PLID 31163 - Undid j.jones change, which I believe was mistaken.  This is the EIN, not the NPI.
					_RecordsetPtr rsLoc = CreateRecordset("SELECT EIN FROM LocationsT WHERE ID = %li",m_pEBillingInfo->BillLocation);
					if(!rsLoc->eof) {
						var = rsLoc->Fields->Item["EIN"]->Value;
						if(var.vt==VT_BSTR) {
							str = CString(var.bstrVal);
						}
						else
							str = "";
					}
					rsLoc->Close();
				}
				else {
					//use provider's fed. employer ID

					var = m_rsDoc->Fields->Item["Fed Employer ID"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
				}
			}
			OutputString += ParseField(str,10,'L',' ');
			OutputString += ParseField("",2,'L',' ');

			// (j.jones 2008-09-10 11:36) - PLID 30788 - if the nBox25Check is 1,
			// override with SSN, otherwise it is EIN
			if(nBox25Check == 1) {
				str = "X";
			}
			else {
				str = "   X";
			}
			OutputString += ParseField(str,15,'L',' ');
		}

		//PatientsT.UserDefinedID
		str.Format("%li",m_pEBillingInfo->UserDefinedID);

		//determine if we need to enlarge this field
		BOOL bExtendPatIDField = FALSE;

		if(GetRemotePropertyInt("SearchByBillID", 0, 0, "<None>", TRUE) == 1) {
			//use patient ID / bill ID as the patient ID
			str.Format("%li/%li",m_pEBillingInfo->UserDefinedID, m_pEBillingInfo->BillID);

			bExtendPatIDField = TRUE;

			if(GetRemotePropertyInt("HCFAIDAppendRespType", 0, 0, "<None>", TRUE) == 1) {
				//use patient ID / bill ID-RespTypeID as the patient ID (but only if it is secondary or higher)
				int RespTypeID = GetInsuranceTypeFromID(m_pEBillingInfo->InsuredPartyID);
				if(RespTypeID > 1)
					str.Format("%li/%li-%li",m_pEBillingInfo->UserDefinedID, m_pEBillingInfo->BillID, RespTypeID);
			}
		}
		OutputString += ParseField(str,bExtendPatIDField ? 17 : 7,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		//InsuranceAcceptedT.Accepted

		str = "    X";
		// (j.jones 2010-07-23 15:02) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsCo now
		BOOL bAccepted = GetAcceptAssignment_ByInsCo(m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);
		if(bAccepted) {
			str = "X";
		}	
		OutputString += ParseField(str,bExtendPatIDField ? 12 : 22,'L',' ');

		//JJ 10/27/2003 - We can't always calculate cyCharges here because if the HCFA form is two pages or more,
		//then the total won't be accurate with the charges displayed on the page. So we must calculate that total instead.

		// (j.jones 2007-10-05 10:48) - PLID 27659 - Somehow, past-Josh didn't seem to think this same logic would count towards
		// the amount paid as well. We need to show the amount paid for the charges on this page, not the whole bill.

		COleCurrency cyApplies = COleCurrency(0,0),
					cyTotal = COleCurrency(0,0);

		// (j.jones 2008-06-18 09:34) - PLID 30403 - CalculateBoxes29and30 will calculate the values
		// for Box 29 and Box 30 based on the settings to ShowPays, IgnoreAdjustments, ExcludeAdjFromBal,
		// and DoNotFillBox29. I made it a modular function because it is identical for all three image types.
		CalculateBoxes29and30(cyCharges, cyApplies, cyTotal, strChargeIDs);	

		// (j.jones 2007-02-28 16:03) - PLID 25009 - for the three totals fields,
		// they now allow 8 characters total, supporting up to 5 characters to the
		// right of the decimal, if decimals are used. Spaces in between were
		// updated appropriately so the dollar amounts didn't move.

		//total charges
		str = FormatCurrencyForInterface(cyCharges,FALSE,FALSE);
		if(!bUseDecimals)
			str = StripNonNum(str);
		if(!bUseDecimals)
			OutputString += ParseField(str,8,'R','0');
		else
			OutputString += ParseField(str,8,'R',' ');
		OutputString += ParseField("",7,'L',' ');

		//total applies, which are only filled in if HCFASetupT.ShowPays = TRUE
		str = FormatCurrencyForInterface(cyApplies,FALSE,FALSE);
		if(!bUseDecimals)
			str = StripNonNum(str);
		if(!bUseDecimals)
			OutputString += ParseField(str,8,'R','0');
		else
			OutputString += ParseField(str,8,'R',' ');
		OutputString += ParseField("",4,'L',' ');

		//based on our setting, hide the total balance
		if(m_HCFAInfo.HideBox30 == 1) {
			str = "";
		}
		else {
			//if HCFASetupT.ShowPays = TRUE, this is the balance,
			//if not, this is the total charges
			str = FormatCurrencyForInterface(cyTotal,FALSE,FALSE);
			if(!bUseDecimals)
				str = StripNonNum(str);
		}

		if(!bUseDecimals && m_HCFAInfo.HideBox30 == 0)
			OutputString += ParseField(str,8,'R','0');
		else
			OutputString += ParseField(str,8,'R',' ');

		OutputString+= "\r\n\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 23 (double) //////////////////////////////////		

		//Line 24 //////////////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",61,'L',' ');

		// (j.jones 2007-05-07 14:38) - PLID 25922 - added Box33Name override
		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		CString strBox33NameOver = "", strBox33Address1_Over = "", strBox33Address2_Over = "",
			strBox33City_Over = "", strBox33State_Over = "", strBox33Zip_Over = "";
		// (j.jones 2008-05-06 12:05) - PLID 27453 - parameterized
		_RecordsetPtr rsBox33Over = CreateParamRecordset("SELECT Box33Name, "
			"Box33_Address1, Box33_Address2, Box33_City, Box33_State, Box33_Zip "
			"FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
		if(!rsBox33Over->eof) {
			strBox33NameOver = AdoFldString(rsBox33Over, "Box33Name","");
			strBox33NameOver.TrimLeft();
			strBox33NameOver.TrimRight();

			// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
			strBox33Address1_Over = AdoFldString(rsBox33Over, "Box33_Address1","");
			strBox33Address1_Over.TrimLeft();
			strBox33Address1_Over.TrimRight();
			strBox33Address2_Over = AdoFldString(rsBox33Over, "Box33_Address2","");
			strBox33Address2_Over.TrimLeft();
			strBox33Address2_Over.TrimRight();
			strBox33City_Over = AdoFldString(rsBox33Over, "Box33_City","");
			strBox33City_Over.TrimLeft();
			strBox33City_Over.TrimRight();
			strBox33State_Over = AdoFldString(rsBox33Over, "Box33_State","");
			strBox33State_Over.TrimLeft();
			strBox33State_Over.TrimRight();
			strBox33Zip_Over = AdoFldString(rsBox33Over, "Box33_Zip","");
			strBox33Zip_Over.TrimLeft();
			strBox33Zip_Over.TrimRight();
		}
		rsBox33Over->Close();
		
		if(!strBox33NameOver.IsEmpty()) {
			//if not empty, output the override
			OutputString += ParseField(strBox33NameOver,45,'L',' ');
		}
		else {
			//otherwise do the normal load

			if(m_HCFAInfo.Box33Setup == 4) {
				//bill location
				// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
				_RecordsetPtr rsLoc = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}",m_pEBillingInfo->BillLocation);
				if(!rsLoc->eof) {
					CString strName = AdoFldString(rsLoc, "Name","");
					OutputString += ParseField(strName,45,'L',' ');
				}
				rsLoc->Close();
			}
			else if(m_HCFAInfo.Box33Setup == 3) {
				//override
				// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
				_RecordsetPtr rsOver = CreateParamRecordset("SELECT Note FROM PersonT WHERE ID = {INT}",m_HCFAInfo.Box33Num);
				if(!rsOver->eof) {
					CString strName = AdoFldString(rsOver, "Note","");
					OutputString += ParseField(strName,45,'L',' ');
				}
				rsOver->Close();
			}
			else {
				//a provider

				if(m_HCFAInfo.Box33Order == 1) { //LFM

					//PersonT.Last
					var = m_rsDoc->Fields->Item["Last"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,16,'L',' ');
					OutputString += ParseField("",1,'L',' ');

					//PersonT.First
					var = m_rsDoc->Fields->Item["First"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,13,'L',' ');
					OutputString += ParseField("",1,'L',' ');

					//PersonT.Middle
					var = m_rsDoc->Fields->Item["Middle"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,1,'L',' ');
					OutputString += ParseField("",1,'L',' ');				

				}
				else {	//FML

					//PersonT.First
					var = m_rsDoc->Fields->Item["First"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,13,'L',' ');
					OutputString += ParseField("",1,'L',' ');

					//PersonT.Middle
					var = m_rsDoc->Fields->Item["Middle"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,1,'L',' ');
					OutputString += ParseField("",1,'L',' ');

					//PersonT.Last
					var = m_rsDoc->Fields->Item["Last"]->Value;
					if(var.vt==VT_BSTR) {
						str = CString(var.bstrVal);
					}
					else
						str = "";
					OutputString += ParseField(str,16,'L',' ');
					OutputString += ParseField("",1,'L',' ');

				}

				//PersonT.Title
				var = m_rsDoc->Fields->Item["Title"]->Value;
				if(var.vt==VT_BSTR) {
					str = CString(var.bstrVal);
				}
				else
					str = "";
				OutputString += ParseField(str,5,'L',' ');
			}
		}

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 24 ///////////////////////////////////////////

		//Line 25 //////////////////////////////////////////////////
		OutputString = "";

		//InsuranceBox31.Box31Info
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT Box31Info FROM InsuranceBox31 WHERE InsCoID = {INT} AND ProviderID = {INT}",
				m_pEBillingInfo->InsuranceCoID,m_pEBillingInfo->ProviderID);

		if(!rs->eof && rs->Fields->Item["Box31Info"]->Value.vt==VT_BSTR) {
			str = CString(rs->Fields->Item["Box31Info"]->Value.bstrVal);
		}
		else
			str = "";
		OutputString += ParseField(str,15,'L',' ');
		OutputString += ParseField("",14,'L',' ');

		bool bHideBox32NameAdd = false;
		//check to see if we should hide box 32

		// (j.jones 2007-05-11 12:15) - PLID 25932 - reworked this to have options to 
		// hide just the name and address, and the IDs
		// (j.gruber 2007-06-28 16:49) - PLID 26154 - fixed the query for if POS is null
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		if(m_HCFAInfo.HideBox32NameAdd != 0) {
			//if HideBox32NameAdd = 1, they want to hide the name/add in box 32 if the POS is 11,
			//so check to see if any of the charges associated with this bill have a POS of 11
			//if HideBox32NameAdd = 2, they want to hide the name/add in box 32 if the POS is NOT 11,
			//so check to see if any of the charges associated with this bill do not have a POS of 11
			//if HideBox32NameAdd = 3, they want to always hide the name/add in box 32
			if(m_HCFAInfo.HideBox32NameAdd == 3 || (m_HCFAInfo.HideBox32NameAdd == 1 && !IsRecordsetEmpty("SELECT PlaceOfServiceCodesT.PlaceCodes FROM ChargesT INNER JOIN LINEITEMT ON ChargesT.ID = LineItemT.ID "
				" LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				" LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				" LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				" WHERE BillID = %li AND Deleted = 0 AND Batched = 1 "
				" AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				" AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				" AND PlaceCodes = '11'", m_pEBillingInfo->BillID)) ||
				
				(m_HCFAInfo.HideBox32NameAdd == 2 && !IsRecordsetEmpty("SELECT PlaceOfServiceCodesT.PlaceCodes FROM ChargesT INNER JOIN LINEITEMT ON ChargesT.ID = LineItemT.ID "
				" LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				" LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				" LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				" WHERE BillID = %li AND Deleted = 0 AND Batched = 1 "
				" AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				" AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				" AND (PlaceCodes <> '11' OR PlaceCodes IS NULL) ", m_pEBillingInfo->BillID))) {

				bHideBox32NameAdd = true;
			}
			else{
				bHideBox32NameAdd = false;
			}
		}
		else{
			bHideBox32NameAdd = false;
		}

		//LocationsT.Name (POS)
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		_RecordsetPtr rsPOS = CreateParamRecordset("SELECT * FROM LocationsT WHERE ID = {INT}",m_pEBillingInfo->nPOSID);

		if(!bHideBox32NameAdd && !rsPOS->eof && rsPOS->Fields->Item["Name"]->Value.vt==VT_BSTR)
			str = CString(rsPOS->Fields->Item["Name"]->Value.bstrVal);
		else
			str = "";
		OutputString += ParseField(str,26,'L',' ');
		OutputString += ParseField("",6,'L',' ');

		//LocationsT.Address1 + Address 2 (Location)
		_RecordsetPtr rsLoc;
		if(m_HCFAInfo.Box33Setup == 3) {
			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			rsLoc = CreateParamRecordset("SELECT * FROM PersonT WHERE ID = {INT}",m_HCFAInfo.Box33Num);
		}
		else if(m_HCFAInfo.DocAddress == 1 || m_HCFAInfo.Box33Setup == 4) {
			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			rsLoc = CreateParamRecordset("SELECT * FROM LocationsT WHERE ID = {INT}",m_pEBillingInfo->BillLocation);
		}
		else {
			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			rsLoc = CreateParamRecordset("SELECT * FROM PersonT WHERE ID = {INT}",m_pEBillingInfo->ProviderID);
		}

		// (j.jones 2007-05-03 09:37) - PLID 25885 - mimic the HCFA such that
		// Address 1 and Address 2 are concatenated on the same line,
		// though are still limited to 32 characters total

		CString strAddress1 = AdoFldString(rsLoc, "Address1","");
		CString strAddress2 = AdoFldString(rsLoc, "Address2","");

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33Address1_Over.IsEmpty()) {
			strAddress1 = strBox33Address1_Over;
		}
		if(!strBox33Address2_Over.IsEmpty()) {
			strAddress2 = strBox33Address2_Over;
		}

		str.Format("%s %s", strAddress1, strAddress2);
		OutputString += ParseField(str,32,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 25 ////////////////////////////////////////////

		//Line 26 ///////////////////////////////////////////////////
		OutputString = "";

		str = "";

		//Box31 may use the Override name
		if(m_HCFAInfo.UseOverrideInBox31 == 1 && m_HCFAInfo.Box33Setup == 3) {
			//override
			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			_RecordsetPtr rsOver = CreateParamRecordset("SELECT Note FROM PersonT WHERE ID = {INT}",m_HCFAInfo.Box33Num);
			if(!rsOver->eof) {
				str = AdoFldString(rsOver, "Note","");
			}
			rsOver->Close();
		}
		else {

			//PersonT.First
			var = m_rsDoc->Fields->Item["First"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";

			str += " ";

			//PersonT.Middle
			var = m_rsDoc->Fields->Item["Middle"]->Value;
			if(var.vt==VT_BSTR) {
				str += CString(var.bstrVal);
			}
			else
				str += "";

			str += " ";

			//PersonT.Last
			var = m_rsDoc->Fields->Item["Last"]->Value;
			if(var.vt==VT_BSTR) {
				str += CString(var.bstrVal);
			}
			else
				str += "";
			
			str += " ";

			//PersonT.Title
			var = m_rsDoc->Fields->Item["Title"]->Value;
			if(var.vt==VT_BSTR) {
				str += CString(var.bstrVal);
			}
			else
				str += "";
		}

		OutputString += ParseField(str,27,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		// (j.jones 2007-05-03 09:37) - PLID 25885 - mimic the HCFA such that
		// Address 1 and Address 2 are concatenated on the same line,
		// though are still limited to 29 characters total

		//LocationsT.Address1 + Address 2 (POS)
		if(!bHideBox32NameAdd) {
			str.Format("%s %s", AdoFldString(rsPOS, "Address1",""), AdoFldString(rsPOS, "Address2",""));
		}
		else
			str = "";
		OutputString += ParseField(str,29,'L',' ');		
		OutputString += ParseField("",3,'L',' ');

		//LocationsT.City (Location)
		var = rsLoc->Fields->Item["City"]->Value;
		if(var.vt==VT_BSTR) {
			str = CString(var.bstrVal);
		}
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33City_Over.IsEmpty()) {
			str = strBox33City_Over;
		}

		OutputString += ParseField(str,15,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.State (Location)
		var = rsLoc->Fields->Item["State"]->Value;
		if(var.vt==VT_BSTR) {
			str = CString(var.bstrVal);
		}
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33State_Over.IsEmpty()) {
			str = strBox33State_Over;
		}

		OutputString += ParseField(str,2,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.Zip (Location)
		var = rsLoc->Fields->Item["Zip"]->Value;
		if(var.vt==VT_BSTR) {
			str = CString(var.bstrVal);
		}
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33Zip_Over.IsEmpty()) {
			str = strBox33Zip_Over;
		}

		// (j.jones 2007-08-28 08:40) - PLID 27197 - expanded to 10 characters
		OutputString += ParseField(str,10,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.Phone (Location)

		// (j.jones 2007-02-20 15:17) - PLID 23935 - check to see if we should hide the phone number in Box 33
		if(m_HCFAInfo.HidePhoneBox33 == 1) {
			str = "";
		}
		else {
			if(m_HCFAInfo.Box33Setup == 3)
				var = rsLoc->Fields->Item["WorkPhone"]->Value;
			else if(m_HCFAInfo.Box33Setup == 4)
				var = rsLoc->Fields->Item["Phone"]->Value;
			else if(m_HCFAInfo.DocAddress == 1)
				var = rsLoc->Fields->Item["Phone"]->Value;
			else
				var = rsLoc->Fields->Item["WorkPhone"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
		}
		OutputString += ParseField(str,14,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 26 ////////////////////////////////////////////

		//Line 27 (triple) //////////////////////////////////////////
		OutputString = "";
		OutputString += ParseField("",16,'L',' ');

		//0 - the Print Date (today), 1 - the Bill Date
		if(m_HCFAInfo.Box31UseDate == 0) {
			COleDateTime dt;
			dt = COleDateTime::GetCurrentTime();
			str = dt.Format("%m/%d/%Y");
		}
		else {

			// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
			rsBills = CreateParamRecordset("SELECT Date FROM BillsT WHERE ID = {INT}",m_pEBillingInfo->BillID);
			//BillsT.Date
			if(!rsBills->eof && rsBills->Fields->Item["Date"]->Value.vt == VT_DATE) {
				COleDateTime dt = rsBills->Fields->Item["Date"]->Value.date;
				str = dt.Format("%m/%d/%Y");
			}
			else
				str = "";
		}
		OutputString += ParseField(str,10,'L',' ');
		OutputString += ParseField("",3,'L',' ');

		//rsBills->Close();

		//LocationsT.City (POS)
		if(!bHideBox32NameAdd) {
			var = rsPOS->Fields->Item["City"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,14,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.State (POS)
		if(!bHideBox32NameAdd) {
			var = rsPOS->Fields->Item["State"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,2,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//LocationsT.Zip (POS)
		if(!bHideBox32NameAdd) {
			var = rsPOS->Fields->Item["Zip"]->Value;
			if(var.vt==VT_BSTR) {
				str = CString(var.bstrVal);
			}
			else
				str = "";
		}
		else
			str = "";
		OutputString += ParseField(str,10,'L',' ');		
		OutputString += ParseField("",2,'L',' ');

		//PIN
		str = Box33Pin();
		OutputString += ParseField(str,20,'L',' ');
		OutputString += ParseField("",2,'L',' ');

		//GRP

		//first check the Insurance Co's group 
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		rsTemp = CreateParamRecordset("SELECT GRP FROM InsuranceGroups INNER JOIN InsuredPartyT ON InsuranceGroups.InsCoID = InsuredPartyT.InsuranceCoID "
			"WHERE InsuredPartyT.PersonID = {INT} AND ProviderID = {INT}",m_pEBillingInfo->InsuredPartyID,m_rsDoc->Fields->Item["ID"]->Value.lVal);
		if(!rsTemp->eof && rsTemp->Fields->Item["GRP"]->Value.vt == VT_BSTR)
			str = CString(rsTemp->Fields->Item["GRP"]->Value.bstrVal);
		else
			str = "";
		rsTemp->Close();

		//then check the HCFA group's default
		if(m_HCFAInfo.Box33GRP.GetLength() != 0)
			str = m_HCFAInfo.Box33GRP;

		//last check for the override
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		rsTemp = CreateParamRecordset("SELECT GRP FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
		if(!rsTemp->eof) {
			var = rsTemp->Fields->Item["GRP"]->Value;
			if(var.vt == VT_BSTR) {
				CString strTemp = CString(var.bstrVal);
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty())
					str = strTemp;
			}
		}
		rsTemp->Close();

		OutputString += ParseField(str,20,'L',' ');		

		OutputString+= "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 27 (triple) ///////////////////////////////////

		rsPOS->Close();

		if(GetRemotePropertyInt("HCFAImageShowFacilityCode",0,0,"<None>",true) == 1) {
			//Super Double Secret Line////////////////////////////////////
			OutputString = "";

			OutputString += ParseField("",29,'L',' ');

			bool bHideBox32NPIID = false;
			//check to see if we should hide box 32

			// (j.jones 2007-05-11 12:16) - PLID 25932 - reworked this to have options to 
			// hide just the name and address, and the IDs
			// (j.gruber 2007-06-28 16:49) - PLID 26154 - fixed the query for if POS is null
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			if(m_HCFAInfo.HideBox32NPIID != 0) {
				//if HideBox32NPIID = 1, they want to hide the IDs in box 32 if the POS is 11,
				//so check to see if any of the charges associated with this bill have a POS of 11
				//if HideBox32NPIID = 2, they want to hide the IDs in box 32 if the POS is NOT 11,
				//so check to see if any of the charges associated with this bill do not have a POS of 11
				//if HideBox32NPIID = 3, they want to always hide the IDs in box 32
				if(m_HCFAInfo.HideBox32NPIID == 3 || (m_HCFAInfo.HideBox32NPIID == 1 && !IsRecordsetEmpty("SELECT PlaceOfServiceCodesT.PlaceCodes FROM ChargesT INNER JOIN LINEITEMT ON ChargesT.ID = LineItemT.ID "
					" LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
					" LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					" LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					" WHERE BillID = %li AND Deleted = 0 AND Batched = 1 "
					" AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					" AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					" AND PlaceCodes = '11'", m_pEBillingInfo->BillID)) ||
					
					(m_HCFAInfo.HideBox32NPIID == 2 && !IsRecordsetEmpty("SELECT PlaceOfServiceCodesT.PlaceCodes FROM ChargesT INNER JOIN LINEITEMT ON ChargesT.ID = LineItemT.ID "
					" LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
					" LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					" LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					" WHERE BillID = %li AND Deleted = 0 AND Batched = 1 "
					" AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					" AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					" AND (PlaceCodes <> '11' OR PlaceCodes IS NULL) ", m_pEBillingInfo->BillID))) {

					bHideBox32NPIID = true;
				}
				else{
					bHideBox32NPIID = false;
				}
			}
			else{
				bHideBox32NPIID = false;
			}

			str = "";

			if(!bHideBox32NPIID) {

				// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
				_RecordsetPtr rsFID = CreateParamRecordset("SELECT FacilityID FROM InsuranceFacilityID WHERE LocationID = {INT} AND InsCoID = {INT}",m_pEBillingInfo->nPOSID, m_pEBillingInfo->InsuranceCoID);
				if(!rsFID->eof && rsFID->Fields->Item["FacilityID"]->Value.vt == VT_BSTR) {
					str = CString(rsFID->Fields->Item["FacilityID"]->Value.bstrVal);
				}
				else
					str = "";
				rsFID->Close();

				// (j.jones 2009-02-06 16:29) - PLID 32951 - this function now requires the bill location
				// (j.jones 2009-02-23 10:54) - PLID 33167 - now we require the billing provider ID and rendering provider ID,
				// in this case the Rendering Provider ID is the Box 24J provider ID
				// (j.jones 2011-04-05 13:39) - PLID 42372 - this now returns a struct of information
				ECLIANumber eCLIANumber = UseCLIANumber(m_pEBillingInfo->BillID,m_pEBillingInfo->InsuredPartyID,m_pEBillingInfo->ProviderID,m_pEBillingInfo->Box24J_ProviderID,m_pEBillingInfo->BillLocation);
				if(eCLIANumber.bUseCLIANumber && eCLIANumber.bUseCLIAInHCFABox32) {
					str = eCLIANumber.strCLIANumber;
				}
			}

			OutputString += ParseField(str,15,'L',' ');

			OutputString+= "\r\n";
			//if(m_FormatID==INTERLINK)
			//	OutputString += "\f";
			TRACE(OutputString);
			m_OutputFile.Write(OutputString,OutputString.GetLength());
			//////////////////////////////////////////////////////////////
		}

		//Line 28 ///////////////////////////////////////////////////
		OutputString = "";

		str = "\r\n\r\n--------------NEXT CLAIM-----------------------";
		OutputString += ParseField(str,60,'L',' ');	

		OutputString+= "\r\n";
		//if(m_FormatID==INTERLINK)
		//	OutputString += "\f";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());
		//End Of Line 28 ////////////////////////////////////////////

		m_rsDoc->Close();

		return Success;

	}NxCatchAll("Error in Ebilling::Legacy_HCFABoxes24to33");

	return Error_Other;
}

void CEbilling::ResolveErrors(int Error)
{
	if(Error==Success) {
		//do nothing
	}
	else if(Error==Error_Missing_Info) {
		EndDialog(ID_CANCEL);
		MessageBox("Errors were found in your claim that will cause a rejection,\n"
			"as a result, the export has been aborted. Please correct the errors before exporting again.","Electronic Billing",MB_OK|MB_ICONEXCLAMATION);
	}
	else if(Error==Error_Other) {
		EndDialog(ID_CANCEL);
		MessageBox("An unexpected error has occurred and the export has been aborted. \n"
			"Please contact NexTech for assistance.","Electronic Billing",MB_OK|MB_ICONEXCLAMATION);
	}
	else if (Error == Cancel_Silent)
	{
		//do nothing, it should already be handled appropriately
	}
}

BEGIN_EVENTSINK_MAP(CEbilling, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEbilling)
	ON_EVENT(CEbilling, IDC_FILE_ZIP, 6001 /* ListingFile */, OnListingFileFileZip, VTS_BSTR VTS_BSTR VTS_I4 VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_DATE VTS_DATE VTS_DATE VTS_I4 VTS_BOOL VTS_I4 VTS_BOOL VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEbilling::OnListingFileFileZip(LPCTSTR sFilename, LPCTSTR sComment, long lSize, long lCompressedSize, short nCompressionRatio, long xAttributes, long lCRC, DATE dtLastModified, DATE dtLastAccessed, DATE dtCreated, long xMethod, BOOL bEncrypted, long lDiskNumber, BOOL bExcluded, long xReason) 
{
	// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
	// (j.armen 2011-10-25 13:41) - PLID 46134 - EBilling is located in the shared practice path
	// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
	int nResult = (int)ShellExecute ((HWND)this, NULL, "notepad.exe", ("'" + GetEnvironmentDirectory() + "\\EBilling\\Reports\\" + sFilename + "'"), NULL, SW_SHOW);
}

void CEbilling::OnCancel() 
{
	//set the text, incase the cancel procedure takes a moment
	//SetDlgItemText(IDC_CURR_EVENT,"Cancelling -- Please wait...");

	CDialog::OnCancel();
}

//Box33Pin can be calculated for the primary or secondary
CString CEbilling::Box33Pin(BOOL bCalcForSecondary /*= FALSE*/)
{
	//PIN

	_variant_t var;

	CString str;

	long nHCFAGroupID = m_pEBillingInfo->HCFASetupID;
	long nBox33 = m_HCFAInfo.Box33;
	long nInsuredPartyID = m_pEBillingInfo->InsuredPartyID;
	long nInsuranceCoID = m_pEBillingInfo->InsuranceCoID;
	CString strBox33GRP = m_HCFAInfo.Box33GRP;

	if(bCalcForSecondary && m_pEBillingInfo->OthrInsuredPartyID != -1) {
		//change the group ID and Box33 status
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT Box33, Box33GRP, HCFASetupGroupID, InsuranceCoT.PersonID AS InsCoID "
			"FROM HCFASetupT "
			"INNER JOIN InsuranceCoT ON HCFASetupT.ID = InsuranceCoT.HCFASetupGroupID "
			"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
			"WHERE InsuredPartyT.PersonID = {INT}", m_pEBillingInfo->OthrInsuredPartyID);
		if(!rs->eof) {
			nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID");
			nBox33 = AdoFldLong(rs, "Box33");
			// (j.jones 2007-03-12 14:52) - PLID 25175 - also track the insured party ID, insurance Co ID, and GRP number
			nInsuredPartyID = m_pEBillingInfo->OthrInsuredPartyID;
			nInsuranceCoID = AdoFldLong(rs, "InsCoID");
			strBox33GRP = AdoFldString(rs, "Box33GRP", "");
		}
		else {
			// (j.jones 2007-03-12 15:03) - PLID 25175 - return an empty string if no group is selected
			return "";
		}
		rs->Close();
	}

	// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
	_RecordsetPtr rsTemp = CreateParamRecordset("SELECT PIN FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,nHCFAGroupID);
	if(!rsTemp->eof) {
		var = rsTemp->Fields->Item["PIN"]->Value;
		if(var.vt == VT_BSTR) {
			CString strTemp = CString(var.bstrVal);
			strTemp.TrimLeft();
			strTemp.TrimRight();
			if(!strTemp.IsEmpty()) {
				return strTemp;
			}
		}
	}
	rsTemp->Close();

	BOOL bOpenedDocRecordset = FALSE;

	if(m_rsDoc == NULL || m_rsDoc->GetState() == adStateClosed) {
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		m_rsDoc = CreateParamRecordset("SELECT * FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE ProvidersT.PersonID = {INT}",m_pEBillingInfo->ProviderID);
		bOpenedDocRecordset = TRUE;

		//an error will be posted later
		if(m_rsDoc->eof) {
			return "";
		}
	}

	switch(nBox33) {
	case 1: { //NPI
		//ProvidersT.NPI
		var = m_rsDoc->Fields->Item["NPI"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	}
	case 2: { //Social Security Number
		//PersonT.SocialSecurity
		var = m_rsDoc->Fields->Item["SocialSecurity"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str = StripNonNum(str);
		break;
	}
	case 3: { //Federal ID Number
		//ProvidersT.[Fed Employer ID]
		var = m_rsDoc->Fields->Item["Fed Employer ID"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str = StripNonNum(str);
		break;
	}
	case 4: { //DEA Number
		//ProvidersT.[DEA Number]
		var = m_rsDoc->Fields->Item["DEA Number"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	}
	case 5: { //BCBS Number
		//ProvidersT.[BCBS Number]
		var = m_rsDoc->Fields->Item["BCBS Number"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	}
	case 6: { //Medicare Number
		//ProvidersT.[Medicare Number]
		var = m_rsDoc->Fields->Item["Medicare Number"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	}
	case 7: { //Medicaid Number
		//ProvidersT.[Medicaid Number]
		var = m_rsDoc->Fields->Item["Medicaid Number"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	}
	case 8: { //Workers Comp Number
		//ProvidersT.[Workers Comp Number]
		var = m_rsDoc->Fields->Item["Workers Comp Number"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	}
	case 9: { //Other ID
		//ProvidersT.[Other ID Number]
		var = m_rsDoc->Fields->Item["Other ID Number"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	}
	case 11: { //UPIN
		//ProvidersT.UPIN
		var = m_rsDoc->Fields->Item["UPIN"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	}
	case 12: { //Box24J

		// (j.jones 2007-03-12 14:52) - PLID 25175 - ensured the Box24J number loaded properly if loading for secondary insurance

		//InsuranceBox24J.Box24JNumber
		// (j.jones 2008-04-03 10:12) - PLID 28995 - continue to use the main ProviderID here
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		_RecordsetPtr rsBox24J = CreateParamRecordset("SELECT Box24JNumber FROM InsuranceBox24J WHERE ProviderID = {INT} AND InsCoID = {INT}",m_pEBillingInfo->ProviderID, nInsuranceCoID);
		if(!rsBox24J->eof && rsBox24J->Fields->Item["Box24JNumber"]->Value.vt == VT_BSTR) {
			str = CString(rsBox24J->Fields->Item["Box24JNumber"]->Value.bstrVal);
		}
		else
			str = "";

		rsBox24J->Close();

		//check for the override
		// (j.jones 2008-04-03 10:12) - PLID 28995 - continue to use the main ProviderID here
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		rsTemp = CreateParamRecordset("SELECT Box24J FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillLocation, nHCFAGroupID);
		if(!rsTemp->eof) {
			var = rsTemp->Fields->Item["Box24J"]->Value;
			if(var.vt == VT_BSTR) {
				CString strTemp = CString(var.bstrVal);
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty())
					str = strTemp;
			}
		}
		rsTemp->Close();
		break;
	}
	case 13: {	//GRP
		// (j.jones 2007-01-17 14:30) - PLID 24263 - nobody's really going to select this when submitting electronically,
		// but since it is an option we have no choice but to support it

		// (j.jones 2007-03-12 14:52) - PLID 25175 - ensured the GRP number loaded properly if loading for secondary insurance

		//first check the Insurance Co's group 
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		rsTemp = CreateParamRecordset("SELECT GRP FROM InsuranceGroups INNER JOIN InsuredPartyT ON InsuranceGroups.InsCoID = InsuredPartyT.InsuranceCoID "
			"WHERE InsuredPartyT.PersonID = {INT} AND ProviderID = {INT}", nInsuredPartyID, m_rsDoc->Fields->Item["ID"]->Value.lVal);
		if(!rsTemp->eof && rsTemp->Fields->Item["GRP"]->Value.vt == VT_BSTR)
			str = CString(rsTemp->Fields->Item["GRP"]->Value.bstrVal);
		else
			str = "";
		rsTemp->Close();

		//then check the HCFA group's default
		// (j.jones 2007-03-12 15:04) - PLID 25175 - changed to handle the secondary loading
		if(strBox33GRP.GetLength() != 0)
			str = strBox33GRP;

		//last check for the override
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		rsTemp = CreateParamRecordset("SELECT GRP FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillLocation, nHCFAGroupID);
		if(!rsTemp->eof) {
			var = rsTemp->Fields->Item["GRP"]->Value;
			if(var.vt == VT_BSTR) {
				CString strTemp = CString(var.bstrVal);
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty())
					str = strTemp;
			}
		}
		rsTemp->Close();
		break;
	}
	// (j.jones 2007-04-24 12:54) - PLID 25764 - supported Taxonomy Code
	case 14: { //Taxonomy Code
		//ProvidersT.TaxonomyCode
		var = m_rsDoc->Fields->Item["TaxonomyCode"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	}
	case 16: { //Custom ID 1
		//Custom1.TextParam
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 6 AND PersonID = {INT}",m_pEBillingInfo->ProviderID);
		str = "";
		if(!rsCustom->eof) {
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
		}
		rsCustom->Close();
		break;
	}
	case 17: { //Custom ID 2
		//Custom2.TextParam
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 7 AND PersonID = {INT}",m_pEBillingInfo->ProviderID);
		str = "";
		if(!rsCustom->eof) {
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
		}
		rsCustom->Close();
		break;
	}
	case 18: { //Custom ID 3
		//Custom3.TextParam
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 8 AND PersonID = {INT}",m_pEBillingInfo->ProviderID);
		str = "";
		if(!rsCustom->eof) {
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
		}
		rsCustom->Close();
		break;
	}
	case 19: { //Custom ID 4
		//Custom4.TextParam
		// (j.jones 2008-05-06 12:49) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 9 AND PersonID = {INT}",m_pEBillingInfo->ProviderID);
		str = "";
		if(!rsCustom->eof) {
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
		}
		rsCustom->Close();
	}
	default:
		str = "";
		break;
	}

	if(bOpenedDocRecordset)
		m_rsDoc->Close();

	return str;
}

CString CEbilling::Box17a()
{
	//this is the ID specified by the setup of Box17

	CString str = "";
	_variant_t var;

	// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
	_RecordsetPtr rsTemp = CreateParamRecordset("SELECT ReferringPhysT.* FROM ReferringPhysT LEFT JOIN BillsT ON ReferringPhysT.PersonID = BillsT.RefPhyID WHERE BillsT.ID = {INT}",m_pEBillingInfo->BillID);
	
	if(rsTemp->eof) {
		//end of file does not necessarily mean a bad claim
		return "";
	}

	switch(m_HCFAInfo.Box17a) {
	case 1: { //NPI
		//ReferringPhysT.NPI
		var = rsTemp->Fields->Item["NPI"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	}
	case 2: { //Referring ID
		//ReferringPhysT.ReferringPhyID
		var = rsTemp->Fields->Item["ReferringPhyID"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	}
	case 3: { //Referring UPIN
		//ReferringPhysT.UPIN
		var = rsTemp->Fields->Item["UPIN"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	}
	case 4: { //Referring Blue Shield
		//ReferringPhysT.BlueShieldID
		var = rsTemp->Fields->Item["BlueShieldID"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	}
	case 5:	//Referring FedEmployerID
		//ReferringPhysT.FedEmployerID
		var = rsTemp->Fields->Item["FedEmployerID"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	case 6:	//Referring DEANumber
		//ReferringPhysT.DEANumber
		var = rsTemp->Fields->Item["DEANumber"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	case 7:	//Referring MedicareNumber
		//ReferringPhysT.MedicareNumber
		var = rsTemp->Fields->Item["MedicareNumber"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	case 8:	//Referring MedicaidNumber
		//ReferringPhysT.MedicaidNumber
		var = rsTemp->Fields->Item["MedicaidNumber"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	case 9:	//Referring WorkersCompNumber
		//ReferringPhysT.WorkersCompNumber
		var = rsTemp->Fields->Item["WorkersCompNumber"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	case 10:	//Referring OtherIDNumber
		//ReferringPhysT.OtherIDNumber
		var = rsTemp->Fields->Item["OtherIDNumber"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	case 11:	//Referring License Number
		//ReferringPhysT.License
		var = rsTemp->Fields->Item["License"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	// (j.jones 2007-04-24 12:54) - PLID 25764 - supported Taxonomy Code
	// (j.jones 2012-03-07 15:58) - PLID  48676 - removed Taxonomy Code
	/*
	case 12: { //Referring Taxonomy Code
		//ReferringPhysT.TaxonomyCode
		var = rsTemp->Fields->Item["TaxonomyCode"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		break;
	}
	*/
	case 16: { //Custom ID 1
		//Custom1.TextParam
		// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT LEFT JOIN BillsT ON CustomFieldDataT.PersonID = BillsT.RefPhyID WHERE BillsT.ID = {INT} AND FieldID = 6",m_pEBillingInfo->BillID);
		str = "";
		if(!rsCustom->eof) {
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
		}
		rsCustom->Close();
		break;
	}
	case 17: { //Custom ID 2
		//Custom2.TextParam
		// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT LEFT JOIN BillsT ON CustomFieldDataT.PersonID = BillsT.RefPhyID WHERE BillsT.ID = {INT} AND FieldID = 7",m_pEBillingInfo->BillID);
		str = "";
		if(!rsCustom->eof) {
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
		}
		rsCustom->Close();
		break;
	}
	case 18: { //Custom ID 3
		//Custom3.TextParam
		// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT LEFT JOIN BillsT ON CustomFieldDataT.PersonID = BillsT.RefPhyID WHERE BillsT.ID = {INT} AND FieldID = 8",m_pEBillingInfo->BillID);
		str = "";
		if(!rsCustom->eof) {
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
		}
		rsCustom->Close();
		break;
	}
	case 19: { //Custom ID 4
		//Custom4.TextParam
		// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT LEFT JOIN BillsT ON CustomFieldDataT.PersonID = BillsT.RefPhyID WHERE BillsT.ID = {INT} AND FieldID = 9",m_pEBillingInfo->BillID);
		str = "";
		if(!rsCustom->eof) {
			var = rsCustom->Fields->Item["TextParam"]->Value;
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
		}
		rsCustom->Close();
		break;
	}
	default:
		str = "";
		break;
	}

	rsTemp->Close();

	return str;
}

void CEbilling::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == EBILLING_TIMER_EVENT)
	{
		KillTimer(EBILLING_TIMER_EVENT);

		// (j.jones 2008-05-09 09:54) - PLID 29986 - removed modem usage from the program
		
		ExportData();
	}
	
	CDialog::OnTimer(nIDEvent);
}

/***********************************************************************
*
*	ANSI X12 4010 Implementation
*
*	(Once again, we're heavily commenting the export style.)
*	
*	ANSI is a lot different from NSF, as it is not as easily separated
*	by 320 character records. It is arranged logically with "loops", which
*	roughly correspond related information together. The outputted file
*	has "data segments" and "data elements". A segment begins with an identifier
*	(REF, SV1, etc.) which unfortunately isn't unique. The segment ends with ~.
*	Each "element" begins with a *. Even if the element is not used, the * is used.
*	A finished data segment looks like: ST*837*3456~
*	
*
*	Segment Definitions:
*
*	Name:	The three letter abbreviation for the segment name.
*
*	Position:	I have no idea what this number is for.
*
*	Loop:	The loop this segment belongs to.
*
*	Repeat:	Loop repeat info.
*
*	Requirement:	Mandatory or Optional
*
*	Purpose:	Explains what data this segment holds.
*	
*	Notes:		Special instructions.
*
*	Syntax:		When there are elements with the X property, Syntax explains how the X is used.
*
*		Relational condition codes:
*
*		P - Paired or Multiple - If any element in the relational condition is present, then all must be present.
*
*		R - Required - At least one element in the relational condition must be present.
*
*		E - Exclusion - No more than one of the elements in the relational condition may be present.
*
*		C - Conditional - If the first element in the condition is present, then all other elements in the condition must be present.
*		However, any or all elements in the condition may appear without the requiring the first element be present.
*		(ie. Using element A requires using both B and C, but you may have elements B and/or C without having A.
*
*		ie. Syntax: R0203 - Means that either element 02 or element 03 is required.
*			Syntax: P0506 - Means that if element 05 is present, than element 06 must be present, and vice-versa.
*
*
*	Element Definitions:
*
*	Usage:	REQ or SIT, Required or Situational
*
*	Ref. Des.:	The element ID - generally the three letter segment name and an incremental number.
*
*	Data Element:	I have no idea what this number is for.
*
*	Name:	The element name.
*
*	Attributes:	ie. M ID 2/3					
*				The first attribute is M, O, or X. Mandatory, optional, or relational. Relational should follow
*				the associated syntax (P, R, E, C, see above).
*				The second attribute is the data type.
*					Nn		Numeric
*					R		Decimal
*					ID		Identifier
*					AN		String
*					DT		Date
*					TM		Time
*					B		Binary
*				The third attribute is the min/max size of the element.
*
*	When outputting a record (data segment), manually output the 3-character segment name,
*	then output all the elements, then manually output the ~ before continuing onto the next segment.
*
*	When outputting a field (data element), use ParseANSIField. It will take in your data, a minimum size,
*	maximum size, justification type (L or R for left or right), and a fill character (' ' or '0').
*	ParseANSIField will automatically prepend a *. Please call ParseANSIField even if the data is blank
*	so the * will be prepended.
*
*	When adding information for new fields (data elements), please do all your calculations and outputting
*	directly under the comment for that field. It will help separate the code in a logical
*	fashion. Also, always show the TableName.FieldName just after the commented line. This way
*	we can quickly see which fields in the database correspond to fields in the exported form.
*
*	When a set of fields are blank, please don't output one field with the total number of * characters.
*	Output each field as you normally would, just leave it blank. This will simplify matters
*	when/if we use this field in the future.
*
*	You may also see that there are many functions corresponding to line types that
*	we don't use. ANSI is an all-purpose medical format, and there are many lines that
*	our clients will never need. However, it is good practice to at least have the function
*	there, with a description, so we are aware of these lines existing and are prepared
*	to use them in the future. Again, it's a good quick-reference.
*
*	When exporting to multiple types of clearinghouses, we should be able to call the lines
*	we need, in order. That way we don't need to redo code for each destination. If, for
*	some reason, a line is used in two clearinghouses yet behaves differently for each place,
*	use your best judgement on how to handle it. If it's a minor change, an if or case statement
*	is acceptable.
*
*	And whatever you do, PLEASE do not try and combine all the recordsets used into one giant query.
*	This is what we used to do, and it was terrible. So let's not reproduce old mistakes.
*
*	- Josh Jones, 6/13/2002
*
*
************************************************************************/

int CEbilling::ExportToANSI()
{
	//JJ - The nice part about the "loops" in ANSI is that there is no record that
	//ends the loop. So instead of nested loops, we can run one loop through each claim
	//and occasionally export extra lines.

	//The loops go like this:

	//Header

	//Providers

		//Subscribers

			//Claims

				//Charges

	//Trailer

	//But since there are no records to signify the end of a loop, we just output the
	//provider, subscriber, or claim lines to start a new loop. Easy!

	//For the file as a whole, it is surrounded with an interchange header and a functional group header,
	//and associated trailers.

	try {

		CString str;

		SetDlgItemText(IDC_CURR_EVENT, GetExportEventMessage());

		//open the file for writing
		if(!m_OutputFile.Open(m_ExportName,CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
			// (j.armen 2011-10-25 13:41) - PLID 46134 - Ebilling is located in the practice path
			// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure. Also, changed to FileUtils::CreatePath() so that 
			// non-existent intermediate paths are created.
			FileUtils::CreatePath(FileUtils::GetFilePath(m_ExportName));
			if(!m_OutputFile.Open(m_ExportName,CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
				AfxMessageBox("The ebilling export file could not be created. Contact Nextech for assistance.");
				return Error_Other;
			}
		};

		//-2, because if they have an invalid ProviderID/InsCoID it would be -1, and we compare those IDs
		//prior to stopping the export for an invalid Provider/InsCoID
		m_PrevProvider = -2;
		m_PrevInsCo = -2;
		m_PrevInsParty = -2;
		m_PrevLocation = -2;
		
		m_pEBillingInfo = ((EBillingInfo*)m_aryEBillingInfo.GetAt(0));

		m_HCFAInfo.LoadData(m_pEBillingInfo->HCFASetupID);
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType == actInst) {
			m_UB92Info.LoadData(m_pEBillingInfo->UB92SetupID);
		}

		//Run all the loops and output all the records.

		// (j.jones 2010-10-13 13:31) - PLID 40913 - check the toggle for 4010/5010
		if(m_avANSIVersion == av5010) {
			//5010

			// (j.jones 2010-10-13 13:31) - PLID 32848 - supported ANSI 5010 claims,
			// the body for these functions is in EbillingANSI5010.cpp

			RETURN_ON_FAIL(ANSI_5010_InterchangeHeader());	//Interchange Control Header

			RETURN_ON_FAIL(ANSI_5010_FunctionalGroupHeader());	//Functional Group Header

			//set this segment count to zero AFTER the functional headers are called.
			//We only want to count the segments between the transaction header and trailer.
			m_ANSISegmentCount = 0;
			
			RETURN_ON_FAIL(ANSI_5010_Header());	//Header
			
			RETURN_ON_FAIL(ANSI_5010_1000A());	//Submitter Info

			RETURN_ON_FAIL(ANSI_5010_1000B());	//Receiver Info

			//initialize all counters
			m_ANSIHLCount = m_ANSICurrProviderHL = m_ANSICurrSubscriberHL = m_ANSICurrPatientParent = m_ANSIClaimCount = m_ANSIServiceCount = 0;

			//loop through all claims
			for(int i=0;i<m_aryEBillingInfo.GetSize();i++) {

				str.Format("%s - Claim %li of %li", GetExportEventMessage(), i+1,m_aryEBillingInfo.GetSize());
				SetDlgItemText(IDC_CURR_EVENT,str);
				
				m_pEBillingInfo = ((EBillingInfo*)m_aryEBillingInfo.GetAt(i));

				m_HCFAInfo.LoadData(m_pEBillingInfo->HCFASetupID);
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType == actInst) {
					m_UB92Info.LoadData(m_pEBillingInfo->UB92SetupID);
				}

				TRACE("HCFA ID=%d\r\n", m_pEBillingInfo->HCFAID);
				TRACE("==========================================================\r\n");

				
				//loop through each provider

				// (j.jones 2006-12-06 15:49) - PLID 23631 - if enabled, separate batches by insurance company as well
				// (j.jones 2007-01-30 09:05) - PLID 24411 - also separate by location
				// (j.jones 2008-05-06 11:18) - PLID 29937 - cached m_bSeparateBatchesByInsCo
				if(m_PrevProvider != m_pEBillingInfo->ProviderID || m_PrevLocation != m_pEBillingInfo->BillLocation || (m_bSeparateBatchesByInsCo && m_PrevInsCo != m_pEBillingInfo->InsuranceCoID)) {

					RETURN_ON_FAIL(ANSI_5010_2000A());	//Billing/Pay-To Provider HL (Hierarchical Level)
					RETURN_ON_FAIL(ANSI_5010_2010AA());	//Billing Provider Name

					//JJ - Yet again, another clearinghouse broke ANSI. The 2010AB should never be exported if it is the same
					//provider as 2010AA, which it always is, yet NDEX in Washington wants it anyways.
					// (j.jones 2008-05-06 11:30) - PLID 29937 - cached m_bUse2010AB
					
					// (j.jones 2010-11-01 15:32) - PLID 40919 - we now have a specific 2010AB override,
					// if it is in use we should always send 2010AB

					_RecordsetPtr rsAddressOverride;

					if(m_actClaimType == actInst) {
						//UB			
						rsAddressOverride = CreateParamRecordset("SELECT PayTo2010AB_Address1, PayTo2010AB_Address2, PayTo2010AB_City, PayTo2010AB_State, PayTo2010AB_Zip "
							"FROM UB92EbillingSetupT "
							"WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT} "
							"AND "
							"("
							"(PayTo2010AB_Address1 Is Not Null AND PayTo2010AB_Address1 <> '') "
							"OR (PayTo2010AB_Address2 Is Not Null AND PayTo2010AB_Address2 <> '') "
							"OR (PayTo2010AB_City Is Not Null AND PayTo2010AB_City <> '') "
							"OR (PayTo2010AB_State Is Not Null AND PayTo2010AB_State <> '') "
							"OR (PayTo2010AB_Zip Is Not Null AND PayTo2010AB_Zip <> '')"
							")",
							m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillLocation, m_pEBillingInfo->UB92SetupID);
					}
					else {
						//HCFA
						rsAddressOverride = CreateParamRecordset("SELECT PayTo2010AB_Address1, PayTo2010AB_Address2, PayTo2010AB_City, PayTo2010AB_State, PayTo2010AB_Zip "
							"FROM EbillingSetupT "
							"WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT} "
							"AND "
							"("
							"(PayTo2010AB_Address1 Is Not Null AND PayTo2010AB_Address1 <> '') "
							"OR (PayTo2010AB_Address2 Is Not Null AND PayTo2010AB_Address2 <> '') "
							"OR (PayTo2010AB_City Is Not Null AND PayTo2010AB_City <> '') "
							"OR (PayTo2010AB_State Is Not Null AND PayTo2010AB_State <> '') "
							"OR (PayTo2010AB_Zip Is Not Null AND PayTo2010AB_Zip <> '')"
							")",
							m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillLocation, m_pEBillingInfo->HCFASetupID);
					}

					if(m_bUse2010AB || !rsAddressOverride->eof) {
						RETURN_ON_FAIL(ANSI_5010_2010AB(rsAddressOverride));	//Pay-To-Provider Name
					}

					//fix the counts so that we restart under this provider
					m_ANSIClaimCount = m_ANSIServiceCount = 0;
				}

				//loop through all subscribers (insured parties)

				//JJ - if the provider changed, then we have to output these again

				// (j.jones 2006-12-06 15:49) - PLID 23631 - if enabled, separate batches by insurance company as well
				// (j.jones 2007-01-30 09:09) - PLID 24411 - same for locations
				// (j.jones 2008-02-15 12:32) - PLID 28943 - we now have a setting to always re-send the subscriber level
				// for every claim, apparently some clearinghouses and insurance companies can't handle the ANSI standard
				// of sending multiple 2300 loops under one 2000B header, so if checked (which is the default), always
				// restart at the subscriber level for every claim
				// (j.jones 2008-05-06 11:18) - PLID 29937 - cached m_bSeparateBatchesByInsCo
				if(m_bSendSubscriberPerClaim || m_PrevInsParty != m_pEBillingInfo->InsuredPartyID || m_PrevProvider != m_pEBillingInfo->ProviderID || m_PrevLocation != m_pEBillingInfo->BillLocation || (m_bSeparateBatchesByInsCo && m_PrevInsCo != m_pEBillingInfo->InsuranceCoID)) {
			
					RETURN_ON_FAIL(ANSI_5010_2000B());	//Subscriber HL
					RETURN_ON_FAIL(ANSI_5010_2010BA());	//Subscriber Name
					RETURN_ON_FAIL(ANSI_5010_2010BB());	//Payer Name

					m_ANSIClaimCount = m_ANSIServiceCount = 0;

					//the current patient number (used for lower levels) will be the subscriber number
					m_ANSICurrPatientParent = m_ANSICurrSubscriberHL;

					//if the patient is different from the subscriber, then export these two lines

					if(IsRecordsetEmpty("SELECT RelationToPatient FROM InsuredPartyT WHERE PersonID = %li AND RelationToPatient = 'Self'",m_pEBillingInfo->InsuredPartyID)) {

						RETURN_ON_FAIL(ANSI_5010_2000C());	//Patient HL
						RETURN_ON_FAIL(ANSI_5010_2010CA());	//Patient Name
					}
				}

				// (j.jones 2011-04-20 10:45) - PLID 41490 - the charge filtering has been moved here so
				// we have a unified list of all charges we are about to export

				CArray<long, long> aryChargesToExport;
				bool bHasChargeReferringProvider = false;
				bool bHasChargeOrderingProvider = false;

				{
					CSqlFragment sqlDocProv("");

					// (j.jones 2006-12-01 15:50) - PLID 22110 - supported the ClaimProviderID
					// (j.jones 2008-04-03 09:23) - PLID 28995 - supported the HCFAClaimProvidersT setup
					// (j.jones 2008-08-01 10:17) - PLID 30917 - HCFAClaimProvidersT is now per insurance company
					// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
					// (j.jones 2010-11-09 12:57) - PLID 41389 - supported ChargesT.ClaimProviderID
					// (j.jones 2015-03-04 10:37) - PLID 65003 - this needs to find the 2310B rendering provider, not the 2010AA provider
					if(m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 2) {
						CSqlFragment sql("AND (ChargesT.ClaimProviderID = {INT} OR (ChargesT.ClaimProviderID Is Null AND ChargesT.DoctorsProviders IN "
							"(SELECT PersonID FROM "
							"	(SELECT ProvidersT.PersonID, "
							"	(CASE WHEN ANSI_2310B_ProviderID Is Null THEN ProvidersT.ClaimProviderID ELSE ANSI_2310B_ProviderID END) AS ProviderIDToUse "
							"	FROM ProvidersT "
							"	LEFT JOIN (SELECT * FROM HCFAClaimProvidersT WHERE InsuranceCoID = {INT}) AS HCFAClaimProvidersT ON ProvidersT.PersonID = HCFAClaimProvidersT.ProviderID) SubQ "
							"WHERE ProviderIDToUse = {INT}))) ", m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ANSI_RenderingProviderID);

						sqlDocProv = sql;
					}
					else if(m_actClaimType == actInst && m_UB92Info.Box82Setup == 1) {
						CSqlFragment sql("AND (ChargesT.ClaimProviderID = {INT} OR (ChargesT.ClaimProviderID Is Null AND ChargesT.DoctorsProviders IN (SELECT PersonID FROM ProvidersT WHERE ClaimProviderID = {INT}))) ", m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->ANSI_RenderingProviderID);

						sqlDocProv = sql;
					}

					// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
					// (j.jones 2014-04-29 08:28) - PLID 61840 - we need to know if any of these charges have
					// referring or ordering provider
					_RecordsetPtr rsCharges = CreateParamRecordset("SELECT ChargesT.ID AS ChargeID, "
						"ChargesT.ReferringProviderID, ChargesT.OrderingProviderID "
						"FROM ChargesT "
						"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
						"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
						"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
						"WHERE ChargesT.BillID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
						"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
						"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
						"{SQL} "
						"ORDER BY ChargesT.LineID", m_pEBillingInfo->BillID, sqlDocProv);
					while(!rsCharges->eof) {
						aryChargesToExport.Add(VarLong(rsCharges->Fields->Item["ChargeID"]->Value));

						// (j.jones 2014-04-29 08:30) - PLID 61840 - we need to know if any charges have
						// a referring provider or an ordering provider
						if(!bHasChargeReferringProvider && VarLong(rsCharges->Fields->Item["ReferringProviderID"]->Value, -1) != -1) {
							bHasChargeReferringProvider = true;
						}
						if(!bHasChargeOrderingProvider && VarLong(rsCharges->Fields->Item["OrderingProviderID"]->Value, -1) != -1) {
							bHasChargeOrderingProvider = true;
						}

						rsCharges->MoveNext();
					}
					rsCharges->Close();
				}

				// (a.walling 2014-03-05 16:29) - PLID 61202 - GetDiagnosisCodesForHCFA - Pass bUseICD10 as appropriate for this InsuredParty and BillID
				bool bUseICD10 = ShouldUseICD10();

				// (a.walling 2014-03-19 10:05) - PLID 61346 - Load all diag codes for the bill
				m_pEBillingInfo->billDiags = GetBillDiagCodes(m_pEBillingInfo->BillID, bUseICD10, ShouldSkipUnlinkedDiags(), &aryChargesToExport);

				//loop through bills

				//most of the loops are so similar between HCFAs and UB92s, that we just call those loops
				//and let individual 'if' statements modify them. However the 2300 loop is a different format,
				//and the 23x0 sub-loops are all re-arranged, so we have two separate functions for each loop.
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType != actInst) {
					//Professional (HCFA) Claim

					// (j.jones 2011-03-07 14:00) - PLID 42660 - load the CLIA information
					// (j.jones 2011-04-05 13:39) - PLID 42372 - this now returns a struct of information
					ECLIANumber eCLIANumber = UseCLIANumber(m_pEBillingInfo->BillID,m_pEBillingInfo->InsuredPartyID,m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->BillLocation);
					BOOL bUseProvAsRefPhy = (eCLIANumber.bUseCLIANumber && eCLIANumber.bCLIAUseBillProvInHCFABox17);

					// (j.jones 2011-03-07 14:12) - PLID 42660 - pass in the CLIA data
					// (j.jones 2011-04-20 10:45) - PLID 41490 - we now pass in info. on skipping diagnosis codes
					RETURN_ON_FAIL(ANSI_5010_2300_Prof(eCLIANumber));	//Claim Info

					// (j.jones 2014-04-29 08:30) - PLID 61840 - based on the Hide2310A setting (HCFA only),
					// we might not send loop 2310A if the claim is going to have a 2420E (ordering provider)
					// or 2420F (referring provider) record
					bool bHide2310A = false;
					if(m_HCFAInfo.Hide2310A != hide2310A_Never) {
						if(m_HCFAInfo.Hide2310A == hide2310A_When2420E && bHasChargeOrderingProvider) {
							bHide2310A = true;
						}
						else if(m_HCFAInfo.Hide2310A == hide2310A_When2420F && bHasChargeReferringProvider) {
							bHide2310A = true;
						}
						else if(m_HCFAInfo.Hide2310A == hide2310A_When2420EorF && (bHasChargeOrderingProvider || bHasChargeReferringProvider)) {
							bHide2310A = true;
						}
					}

					// (j.jones 2008-12-11 13:25) - PLID 32401 - moved the ref phy. ID into the struct
					// (j.jones 2011-03-07 14:11) - PLID 42660 - check to see if the CLIA setup is configured to send the
					// billing provider as the referring provider, and if so, send the billing provider
					if(!bHide2310A && (m_pEBillingInfo->nRefPhyID != -1 || bUseProvAsRefPhy)) {
						RETURN_ON_FAIL(ANSI_5010_2310A_Prof(bUseProvAsRefPhy));	//Referring Provider
					}

					//Only use 2310B when different than 2000A, which would be when 2000A is a group
					// (j.jones 2010-01-11 13:43) - PLID 36831 - default to TRUE, we always want to send
					// it unless told not to, and remember that there might not be an EbillingSetupT record!
					BOOL bSend2310B = TRUE;

					//send 2310B if a group
					/*already defaults to TRUE, but do not forget this if we ever change the default to FALSE
					if(m_bIsGroup) {
						bSend2310B = TRUE;
					}
					*/

					// (j.jones 2009-08-03 15:14) - PLID 33827 - send 2310B if the override says so				
					//It is really not clear whether this setting should load for the 2010AA Billing Provider (as it does now),
					//or from the 2310B Rendering Provider, which you would think as it has 2310B in the name.
					//We pondered, and ultimately we concurred that this setting is indeed a property of the 2010AA provider,
					//it's for that provider that the decision is made to send the 2310B record.
					// (j.jones 2010-01-11 13:45) - PLID 36831 - changed to send unless the override says not to,
					// but if it is a group, it will always send, ignoring the override
					if(!m_bIsGroup) {
						_RecordsetPtr rsOver = CreateParamRecordset("SELECT Export2310BRecord FROM EbillingSetupT "
							"WHERE Export2310BRecord = 0 AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",m_pEBillingInfo->HCFASetupID,m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation);
						if(!rsOver->eof) {
							bSend2310B = FALSE;
						}
						rsOver->Close();
					}

					if(bSend2310B) {
						RETURN_ON_FAIL(ANSI_5010_2310B_Prof());	//Rendering Provider
					}

					// (j.jones 2012-01-06 15:15) - PLID 47351 - we now have options to not send the place of service
					// if:
					// 1. m_bHide2310CWhenBillLocation is enabled, and the POS location name is identical to what we sent
					// as a location name in 2010AA
					// or
					// 2. m_bHide2310C_WhenType11Or12 is enabled, and the POS Place Code is 11 or 12

					BOOL bHide2310C = FALSE;
					if(m_bHide2310C_WhenBillLocation && m_strLast2010AA_NM102 == "2"
						&& ReturnsRecordsParam("SELECT LocationsT.Name FROM LocationsT "
							"INNER JOIN BillsT ON LocationsT.ID = BillsT.Location "
							"WHERE BillsT.ID = {INT} AND LocationsT.Name = {STRING}", m_pEBillingInfo->BillID, m_strLast2010AA_NM103)) {

						// (j.jones 2012-01-06 15:21) - PLID 47351 - They want to hide 2310C when the billing location in 2010AA
						// is the same location as the place of service, we did send a location (NM102 = 2) and not a person in 2010AA,
						// and that location we sent in 2010AA has the same name (NM103) as our place of service name.
						// So do not send 2310C.
						bHide2310C = TRUE;
					}
					
					//if the POS code is 11 or 12 and they want to hide 2310C when it's 11 or 12, hide it!
					// (j.jones 2013-04-25 09:06) - PLID 55564 - we now load the place code in our ebilling object, so we no longer need a query
					if(!bHide2310C && m_bHide2310C_WhenType11Or12
						&& (m_pEBillingInfo->strPOSCode == "11" || m_pEBillingInfo->strPOSCode == "12")) {

						// They want to hide 2310C when the place of service code is 11 or 12, and it is.
						// So do not send 2310C.
						bHide2310C = TRUE;
					}

					if(!bHide2310C) {
						RETURN_ON_FAIL(ANSI_5010_2310C_Prof());	//Service Facility Location
					}

					// (j.jones 2008-12-11 13:24) - PLID 32401 - supported loop 2310E
					// (j.jones 2010-10-14 15:55) - PLID 32848 - in ANSI 5010, 2310E is now 2310D
					if(m_pEBillingInfo->nSupervisingProviderID != -1) {
						RETURN_ON_FAIL(ANSI_5010_2310D_Prof());	//Supervising Provider
					}
				}
				else {
					//Institutional (UB92) Claim

					RETURN_ON_FAIL(ANSI_5010_2300_Inst());	//Claim Info

					
					RETURN_ON_FAIL(ANSI_5010_2310A_Inst());	//Attending Physician

					// (j.jones 2011-08-23 09:15) - PLID 44984 - if we have a 2310B provider, send it
					if(m_pEBillingInfo->UB_2310B_ProviderID != -1) {
						RETURN_ON_FAIL(ANSI_5010_2310B_Inst());	//Operating Physician
					}

					// (j.jones 2011-08-23 09:15) - PLID 44984 - if we have a 2310C provider, send it
					if(m_pEBillingInfo->UB_2310C_ProviderID != -1) {
						RETURN_ON_FAIL(ANSI_5010_2310C_Inst());	//Other Operating Physician
					}
				}				

				//reset the count of charges
				m_ANSIServiceCount = 0;

				//if there is another insurance company, export these three lines

				// (j.jones 2012-03-21 13:49) - PLID 48870 - if this is a secondary claim,
				// then we have to send information for all prior payers, instead of just
				// the "other" insured party on the claim, as we may be sending to tertiary

				CArray<OtherInsuranceInfo, OtherInsuranceInfo> aryOtherInsuredPartyInfo;
				
				//(j.jones 2003-07-24 15:30) - there is now an ANSI setting to NOT export secondaries
				// (j.jones 2008-05-06 11:22) - PLID 29937 - cached m_bDontSubmitSecondary
				// (a.wilson 2014-06-27 14:25) - PLID 62518 - if code sets dont match for primary and secondary then we dont submit either (based on setting).
				if (!m_bDontSubmitSecondary && !DiagnosisCodeSetMismatch(bUseICD10)) {

					// (j.jones 2014-08-22 14:12) - PLID 63446 - always send the "other" insured party
					// that is selected on the bill, at all times
					if (m_pEBillingInfo->OthrInsuredPartyID != -1) {
						//we're not sending a secondary claim, so just send the "other" insured party normally
						OtherInsuranceInfo oInfo;
						oInfo.nInsuredPartyID = m_pEBillingInfo->pOtherInsuranceInfo->nInsuredPartyID;
						oInfo.strHCFAPayerID = m_pEBillingInfo->pOtherInsuranceInfo->strHCFAPayerID;
						oInfo.strUBPayerID = m_pEBillingInfo->pOtherInsuranceInfo->strUBPayerID;
						oInfo.strTPLCode = m_pEBillingInfo->pOtherInsuranceInfo->strTPLCode;
						oInfo.nANSI_Hide2330AREF = m_pEBillingInfo->pOtherInsuranceInfo->nANSI_Hide2330AREF;
						// (j.jones 2014-08-25 08:40) - PLID 54213 - Added bIsOnBill, HasPaid, and SBR01Qual.
						// HasPaid will not be checked if bIsOnBill is true, but set it to true to make sure
						// the payer that is on the bill is never skipped.
						oInfo.bIsOnBill = true;
						oInfo.bHasPaid = true;
						oInfo.sbr01Qual = m_pEBillingInfo->pOtherInsuranceInfo->sbr01Qual;
						aryOtherInsuredPartyInfo.Add(oInfo);
					}

					// (j.jones 2012-03-21 13:49) - PLID 48870 - if this is a secondary claim,
					// then we have to send information for all prior payers, instead of just
					// the "other" insured party on the claim, as we may be sending to tertiary
					if(!m_pEBillingInfo->IsPrimary && 
						((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1)
						|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1))) {

						//Get all insured party information for this patient that are of the same category (medical/vision),
						//and of a lesser priority. Inactive parties are ignored unless they paid or adjusted this claim.
						// (j.jones 2012-08-06 14:54) - PLID 51916 - supported the new payer ID data structure, always pulling from EbillingInsCoIDs
						//TES 10/1/2012 - PLID 52968 - Always include m_pEBillingInfo->OthrInsuredPartyID, even if it's Inactive
						// (j.jones 2014-08-22 14:13) - PLID 63446 - Now we always send the "Other" insured party ID, earlier in this code.
						// This recordset only calculates additional insurances not on the bill.
						// (j.jones 2014-08-22 14:14) - PLID 63446 - Changed to exclude any insured party that was created after the earliest
						// date of service on the claim, or has an effective date after that date of service.
						// (j.jones 2014-08-22 16:20) - PLID 54213 - We now always include insured parties that paid on the claim,
						// we will never include an inactive insured party unless it has paid. This also now sorts by priority
						// with inactive companies last, not first, and inactive companies sorted by effective/assignment date.
						_RecordsetPtr rs = CreateParamRecordset("SELECT InsuredPartyT.PersonID, "
							"CASE WHEN HCFA_ClaimPayerIDsT.ID Is Not Null THEN HCFA_ClaimPayerIDsT.EbillingID ELSE HCFAPayerIDsT.EBillingID END AS HCFAPayerID, "
							"CASE WHEN UB_ClaimPayerIDsT.ID Is Not Null THEN UB_ClaimPayerIDsT.EbillingID ELSE UBPayerIDsT.EBillingID END AS UBPayerID, "
							"InsuranceCoT.TPLCode AS TPLCode, "
							"HCFASetupT.ANSI_Hide2330AREF AS HCFA_Hide2330AREF, UB92SetupT.ANSI_Hide2330AREF AS UB92_Hide2330AREF, "
							"Convert(bit, CASE WHEN AppliedPaymentsQ.InsuredPartyID Is Not Null THEN 1 ELSE 0 END) AS HasPaid, "
							"RespTypeT.Priority "

							"FROM InsuredPartyT "
							"INNER JOIN (SELECT InsuredPartyT.PersonID, InsuredPartyT.PatientID, RespTypeT.Priority, RespTypeT.CategoryType "
							"	FROM InsuredPartyT "
							"	INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
							"	WHERE PersonID = {INT}) AS CurInsuredPartyT ON InsuredPartyT.PatientID = CurInsuredPartyT.PatientID "
							"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
							"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "							
							"LEFT JOIN (SELECT * FROM InsuranceLocationPayerIDsT WHERE LocationID = {INT}) AS InsuranceLocationPayerIDsQ ON InsuranceCoT.PersonID = InsuranceLocationPayerIDsQ.InsuranceCoID "
							"LEFT JOIN EbillingInsCoIDs AS HCFA_ClaimPayerIDsT ON InsuranceLocationPayerIDsQ.ClaimPayerID = HCFA_ClaimPayerIDsT.ID "
							"LEFT JOIN EbillingInsCoIDs AS UB_ClaimPayerIDsT ON InsuranceLocationPayerIDsQ.UBPayerID = UB_ClaimPayerIDsT.ID "
							"LEFT JOIN EbillingInsCoIDs AS UBPayerIDsT ON InsuranceCoT.UBPayerID = UBPayerIDsT.ID "
							"LEFT JOIN EbillingInsCoIDs AS HCFAPayerIDsT ON InsuranceCoT.HCFAPayerID = HCFAPayerIDsT.ID "
							"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
							"LEFT JOIN UB92SetupT ON InsuranceCoT.UB92SetupGroupID = UB92SetupT.ID "

							//find out if this insured party has any applies on this bill
							"LEFT JOIN ("
							"	SELECT PaymentsT.InsuredPartyID "
							"	FROM PaymentsT "
							"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
							"	INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
							"	INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
							"	INNER JOIN LineItemT LineItemT_Charge ON ChargesT.ID = LineItemT_Charge.ID "
							"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaymentQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentQ.OriginalLineItemID "
							"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentQ.VoidingLineItemID "
							"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT_Charge.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
							"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT_Charge.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
							"	WHERE LineItemT.Deleted = 0 AND LineItemT_Charge.Deleted = 0 "
							"	AND ChargesT.Batched = 1 "
							"	AND ChargesT.BillID = {INT} "
							"	AND PaymentsT.InsuredPartyID Is Not Null "
							"	AND LineItemCorrections_OriginalPaymentQ.OriginalLineItemID Is Null "
							"	AND LineItemCorrections_VoidingPaymentQ.VoidingLineItemID Is Null "
							"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
							"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
							"	GROUP BY PaymentsT.InsuredPartyID "
							") AS AppliedPaymentsQ ON InsuredPartyT.PersonID = AppliedPaymentsQ.InsuredPartyID "

							//this returns all the patient's insured parties that are:
							//	- neither of the insured parties on the bill
							//	- any insurance that paid or adjusted this claim
							//	- any active insurance that is a lower priority than the company we're sending to, and:
							//		--of the same Medical/Vision category
							//		--doesn't have a creation/effective date later than the date of service
							"WHERE InsuredPartyT.PersonID <> CurInsuredPartyT.PersonID "
							"AND InsuredPartyT.PersonID <> {INT} "
							"AND ("
							"	AppliedPaymentsQ.InsuredPartyID Is Not Null "
							"	OR "
							"	("
							"		RespTypeT.Priority <> -1 "
							"		AND RespTypeT.Priority < CurInsuredPartyT.Priority "
							"		AND RespTypeT.CategoryType = CurInsuredPartyT.CategoryType "
							"		AND Coalesce(InsuredPartyT.EffectiveDate, InsuredPartyT.AssignDate) < {OLEDATETIME} "
							"	) "
							")"

							"ORDER BY (CASE WHEN RespTypeT.Priority = -1 THEN 1 ELSE 0 END) ASC, "
							"	RespTypeT.Priority ASC, "
							"	Coalesce(InsuredPartyT.EffectiveDate, InsuredPartyT.AssignDate) ASC",
							m_pEBillingInfo->InsuredPartyID, m_pEBillingInfo->BillLocation, m_pEBillingInfo->BillID,
							m_pEBillingInfo->OthrInsuredPartyID, m_pEBillingInfo->dtFirstDateOfService);
						while(!rs->eof) {
							OtherInsuranceInfo oInfo;
							oInfo.nInsuredPartyID = VarLong(rs->Fields->Item["PersonID"]->Value);
							oInfo.strHCFAPayerID = VarString(rs->Fields->Item["HCFAPayerID"]->Value, "");
							oInfo.strUBPayerID = VarString(rs->Fields->Item["UBPayerID"]->Value, "");
							oInfo.strTPLCode = VarString(rs->Fields->Item["TPLCode"]->Value, "");
							if(m_actClaimType == actInst) {
								oInfo.nANSI_Hide2330AREF = VarLong(rs->Fields->Item["UB92_Hide2330AREF"]->Value, 0);
							}
							else {
								oInfo.nANSI_Hide2330AREF = VarLong(rs->Fields->Item["HCFA_Hide2330AREF"]->Value, 0);
							}
							// (j.jones 2014-08-25 08:40) - PLID 54213 - added bIsOnBill, HasPaid, and SBR01Qual
							oInfo.bIsOnBill = false;
							oInfo.bHasPaid = VarBool(rs->Fields->Item["HasPaid"]->Value, FALSE) ? true : false;
							oInfo.sbr01Qual = CalculateSBR01Qualifier(false, m_pEBillingInfo->IsPrimary == 1 ? true : false,
								m_actClaimType != actInst ? m_HCFAInfo.ANSI_EnablePaymentInfo : m_UB92Info.ANSI_EnablePaymentInfo,
								VarLong(rs->Fields->Item["Priority"]->Value));
							aryOtherInsuredPartyInfo.Add(oInfo);
							rs->MoveNext();
						}
						rs->Close();
					}
				}

				// (j.jones 2014-08-25 08:45) - PLID 54213 - try to remove unnecessary duplicate payers,
				// and update the SBR01 qualifier for those that remain
				ScrubOtherInsuredPartyList(aryOtherInsuredPartyInfo);

				// (j.jones 2012-03-21 14:20) - PLID 48870 - now export these three "other" insurance
				// loops for each entry in our array, which may be empty, one, or multiple
				for(int ip = 0; ip < aryOtherInsuredPartyInfo.GetSize(); ip++) {
					OtherInsuranceInfo oInfo = (OtherInsuranceInfo)aryOtherInsuredPartyInfo.GetAt(ip);
							
					RETURN_ON_FAIL(ANSI_5010_2320(oInfo));	//Other Subscriber Info
					RETURN_ON_FAIL(ANSI_5010_2330A(oInfo));	//Other Subscriber Name
					RETURN_ON_FAIL(ANSI_5010_2330B(oInfo));	//Other Payer Name
				}

				//loop through charges

				//if using the bill provider, filter only on that doctor's charges, otherwise get all charges
				//in ANSI, we're using the bill provider if we are using anything but the G1 provider

				// (j.jones 2011-04-20 10:45) - PLID 41490 - the charge filtering has been moved to earlier in this code

				// (j.jones 2008-04-30 12:15) - PLID 28283 - ordered the charges by LineID
				// (j.jones 2008-05-14 15:36) - PLID 30044 - added test result fields
				// (j.jones 2008-05-28 14:31) - PLID 30176 - added NDCCode
				// (j.jones 2009-08-12 17:29) - PLID 35096 - added other drug information
				// (j.jones 2011-09-19 15:32) - PLID 45526 - added LineNote - which is the Notes record flagged to send on claim
				// (j.jones 2012-01-04 08:39) - PLID 47277 - added NOCCode, identifying a "not otherwise classified" code
				// (j.jones 2013-06-03 14:23) - PLID 54091 - added setting to send the claim provider
				// as ordering physician when this service is billed
				// (j.jones 2013-07-15 16:17) - PLID 57566 - NOCType is now in ServiceT, it's only auto-calculated
				// for non-CPT codes if the ItemCode is filled on the charge
				// (d.singleton 2014-03-05 15:49) - PLID 61195 - update ANSI5010 claim export to support ICD10 and new diagcode billing structure
				// (a.walling 2014-03-19 9:13) - PLID 61419 - EBilling - ANSI5010 - DiagCodes and WhichCodes must be unique and filtered for NULLs. Use ChargeWhichICD9CodesIF/10 and the BillDiags
				// (j.jones 2014-04-28 13:31) - PLID 61834 - added referring, ordering, supervising providers
				// (j.jones 2014-08-01 10:51) - PLID 63105 - added charge lab test codes
				_RecordsetPtr rsCharges = CreateParamRecordset("SELECT ChargesT.ID AS ChargeID, ChargesT.ServiceID, ChargesT.*, LineItemT.*, "
					"ServiceT.Anesthesia, ServiceT.UseAnesthesiaBilling, dbo.GetChargeTotal(ChargesT.ID) AS LineTotal, "
					"BillsT.AnesthesiaMinutes, OthrInsuranceCoT.InsType, "
					"SendTestResults, TestResultID, TestResultType, TestResult, NDCCode, "
					"DrugUnitPrice, DrugUnitTypeQual, DrugUnitQuantity, PrescriptionNumber, "
					"LineNotesQ.LineNote, ChargeWhichCodesQ.WhichCodes AS ChargeWhichCodes, "
					"ChargeLabTestCodesQ.LabTestCodes, "
					"ChargesT.ReferringProviderID, ChargesT.OrderingProviderID, ChargesT.SupervisingProviderID, "
					""
					//a service/product is an NOC code if it has a keyword in this list, or NOCType forces it on (or off)
					"Convert(bit, CASE WHEN ServiceT.NOCType = 1 THEN 1 WHEN ServiceT.NOCType = 0 THEN 0 "
					"	WHEN (CPTCodeT.ID Is Not Null OR ChargesT.ItemCode <> '') "
					"		AND ("
					"			ServiceT.Name LIKE '%Not Otherwise Classified%' "
					"			OR ServiceT.Name LIKE '%Not Otherwise%' "
					"			OR ServiceT.Name LIKE '%Unlisted%' "
					"			OR ServiceT.Name LIKE '%Not listed%' "
					"			OR ServiceT.Name LIKE '%Unspecified%' "
					"			OR ServiceT.Name LIKE '%Unclassified%' "
					"			OR ServiceT.Name LIKE '%Not otherwise specified%' "
					"			OR ServiceT.Name LIKE '%Non-specified%' "
					"			OR ServiceT.Name LIKE '%Not elsewhere specified%' "
					"			OR ServiceT.Name LIKE '%Not elsewhere%' "
					"			OR ServiceT.Name LIKE '%nos' "
					"			OR ServiceT.Name LIKE '%nos %' "
					"			OR ServiceT.Name LIKE '%nos;%' "
					"			OR ServiceT.Name LIKE '%nos,%' "
					"			OR ServiceT.Name LIKE '%noc' "
					"			OR ServiceT.Name LIKE '%noc %' "
					"			OR ServiceT.Name LIKE '%noc;%' "
					"			OR ServiceT.Name LIKE '%noc,%' "
					"		)"
					"	THEN 1 ELSE 0 END) AS NOCCode "
					"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
					"OUTER APPLY dbo.{CONST_STRING}({STRING}, ChargesT.ID) ChargeWhichCodesQ "
					"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
					"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
					"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
					"LEFT JOIN InsuredPartyT OthrInsuredPartyT ON BillsT.OthrInsuredPartyID = OthrInsuredPartyT.PersonID "
					"LEFT JOIN InsuranceCoT OthrInsuranceCoT ON OthrInsuredPartyT.InsuranceCoID = OthrInsuranceCoT.PersonID "
					"LEFT JOIN (SELECT Min(Note) AS LineNote, LineItemID FROM Notes "
					"	WHERE LineItemID Is Not Null AND SendOnClaim = 1 "
					"	GROUP BY LineItemID) AS LineNotesQ ON ChargesT.ID = LineNotesQ.LineItemID "
					"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "

					"LEFT JOIN (SELECT ChargesT.ID, "
					"	STUFF((SELECT ',' + BillLabTestCodesT.Code "
					"	FROM BillLabTestCodesT "
					"	INNER JOIN ChargeLabTestCodesT ON BillLabTestCodesT.ID = ChargeLabTestCodesT.BillLabTestCodeID "
					"	WHERE ChargeLabTestCodesT.ChargeID = ChargesT.ID "
					"	FOR XML PATH('')), 1, 1, '') AS LabTestCodes "
					"	FROM ChargesT "
					") AS ChargeLabTestCodesQ ON ChargesT.ID = ChargeLabTestCodesQ.ID "

					"WHERE ChargesT.ID IN ({INTARRAY}) "
					"ORDER BY ChargesT.LineID"
					, ShouldUseICD10() ? "ChargeWhichICD10CodesIF" : "ChargeWhichICD9CodesIF"
					, MakeBillDiagsXml(m_pEBillingInfo->billDiags)
					, aryChargesToExport);

				if(rsCharges->eof) {
					//if the recordset is empty, there is no patient. So halt everything!!!
					// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
					CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
					if(!strPatientName.IsEmpty()) {
						// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
						str.Format("No charges were found for '%s', Bill ID %li. (ExportToANSI)\n\n"
							"Contact NexTech for assistance.", strPatientName, m_pEBillingInfo->BillID);
					}
					else
						//serious problems if you get here
						str = "Could not open charge information. (ExportToANSI)";

					AfxMessageBox(str);
					return Error_Missing_Info;
				}

				while(!rsCharges->eof) {

					//most of the loops are so similar between HCFAs and UB92s, that we just call those loops
					//and let individual 'if' statements modify them. However the 2400 loop is a different format,
					//so we have two separate functions altogether

					// (j.jones 2008-05-23 10:49) - PLID 29084 - loop 2430 needs to know if an allowable
					// was sent in 2400, and the amount
					BOOL bSentAllowedAmount = FALSE;
					COleCurrency cyAllowableSent = COleCurrency(0,0);

					// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
					if(m_actClaimType != actInst) {
						// (j.jones 2011-04-20 10:45) - PLID 41490 - we now pass in info. on skipping diagnosis codes
						RETURN_ON_FAIL(ANSI_5010_2400_Prof(rsCharges, bSentAllowedAmount, cyAllowableSent));	//Service Line (HCFA/Professional claim)					
					}
					else {
						RETURN_ON_FAIL(ANSI_5010_2400_Inst(rsCharges));	//Service Line (UB92/Institutional claim)
					}

					// (j.jones 2008-05-28 14:14) - PLID 30176 - added support for Loop 2410 and NDC Codes,
					// this is supported in both HCFA and UB claims
					CString strNDCCode = AdoFldString(rsCharges, "NDCCode", "");
					strNDCCode.TrimLeft();
					strNDCCode.TrimRight();
					if(!strNDCCode.IsEmpty()) {

						// (j.jones 2009-08-12 17:30) - PLID 35096 - Also send the other drug information
						// into loop 2410, but we require the NDC code to send the loop as a whole.
						CString strUnitType = AdoFldString(rsCharges, "DrugUnitTypeQual", "");
						double dblUnitQty = AdoFldDouble(rsCharges, "DrugUnitQuantity", 0.0);
						CString strPrescriptionNumber = AdoFldString(rsCharges, "PrescriptionNumber", "");

						// (j.jones 2010-10-18 09:11) - PLID 32848 - Price is obsolete in 5010, so I removed the parameter
						RETURN_ON_FAIL(ANSI_5010_2410(strNDCCode, strUnitType, dblUnitQty, strPrescriptionNumber));
					}

					//JJ - 3/4/2003 - Okay the presence of the 2420A record is up for debate.
					//The "correct" ANSI implementation is to follow the commented out code below,
					//(labeled "CORRECT ANSI BEHAVIOR") with the instructions I laid out.
					//Right now, we orchestrate the export in such a way that the 2420A record
					//will never be used. But we could change it eventually, at which point it will be needed.
					//However we have encountered at least one clearinghouse (Noridian Blue Cross)
					//that felt the need to always want the 2420A record, regardless if there is only 
					//one provider. This, of course, is an error state for anyone following the ANSI guidelines.
					//So once again, we have had to accomodate this crazy clearing house by adding a 
					//checkbox to the ANSI properties.

					/*==== BEGIN RIDICULOUSLY INCORRECT IMPLEMENTATION HERE ====*/

					//sigh, here we go:

					// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
					if(m_actClaimType != actInst) {

						// (j.jones 2008-05-06 11:22) - PLID 29937 - cached m_bUse2420A
						if(m_bUse2420A) {
							// (j.jones 2008-04-03 09:55) - PLID 28995 - this function now takes in the ANSI_RenderingProviderID value
							RETURN_ON_FAIL(ANSI_5010_2420A(m_pEBillingInfo->ANSI_RenderingProviderID));
						}
					}


					/*==== END RIDICULOUSLY INCORRECT IMPLEMENTATION HERE ====*/

					//CORRECT ANSI BEHAVIOR: Right now we export a new claim for each rendering provider, 
					//and only the charges for that claim. If we choose to keep the bills together, 
					//comment this code in (it already works) and make the appropriate changes elsewhere.
					//Remove the filter for doctor from the rsCharges recordset, and change the query in LoadClaimInfo.
					//Also, don't forget to still accomodate the code in the above "ridiculously incorrect
					//implementation" block.

					/*

					TODO: If we ever comment this code in, use the charge provider's ClaimProviderID,
					not the charge provider ID

					//Check if the rendering provider for this charge is different from the provider
					//we determined to be the "default provider" in 2310B. But, this will only be if we are
					//using the bill provider to begin with.
					if(m_actClaimType != actInst) {

						long RenProvID = -1;
						_variant_t var = rsCharges->Fields->Item["DoctorsProviders"]->Value;
						if(var.vt == VT_I4 && var.lVal > 0) {
							RenProvID = var.lVal;
						}
						if(m_pEBillingInfo->Box33Setup==1 && RenProvID != m_pEBillingInfo->ProviderID) 
							RETURN_ON_FAIL(ANSI_5010_2420A(RenProvID));	//Rendering Provider Name
					}
					*/

					// (j.jones 2013-06-03 11:56) - PLID 54091 - Supported 2420E for HCFAs, will send only
					// if the service code requires it.
					// (j.jones 2014-04-23 15:05) - PLID 61834 - now charge-level selections can cause
					// 2420D, 2420E, or 2420F to be filled
					if(m_actClaimType != actInst) {

						long n2420D_PersonID = VarLong(rsCharges->Fields->Item["SupervisingProviderID"]->Value, -1);
						long n2420E_PersonID = VarLong(rsCharges->Fields->Item["OrderingProviderID"]->Value, -1);
						long n2420F_PersonID = VarLong(rsCharges->Fields->Item["ReferringProviderID"]->Value, -1);

						// (j.jones 2014-04-23 15:16) - PLID 61834 - if the HCFA setting says to respect
						// the charge setting (value of 0), don't override the charge selection
						if(m_HCFAInfo.OrderingProvider != 0) {

							// (j.jones 2014-01-22 09:49) - PLID 60034 - The HCFA group now has settings to
							// define whether this service code setting is respected.
							// 0 - respect the charge setting (default value)
							// 1 - always send 2420E
							// 2 - never send 2420E
							if(m_HCFAInfo.OrderingProvider == 1) {
								//always send the claim provider as 2420E
								n2420E_PersonID = m_pEBillingInfo->ANSI_RenderingProviderID;
							}
							else if(m_HCFAInfo.OrderingProvider == 2) {
								//never send 2420E
								n2420E_PersonID = -1;
							}
						}
						
						if(n2420D_PersonID != -1) {
							RETURN_ON_FAIL(ANSI_5010_2420D_Prof(n2420D_PersonID));
						}
						if(n2420E_PersonID != -1) {
							RETURN_ON_FAIL(ANSI_5010_2420E_Prof(n2420E_PersonID));
						}
						if(n2420F_PersonID != -1) {
							RETURN_ON_FAIL(ANSI_5010_2420F_Prof(n2420F_PersonID));
						}
					}

					// (j.jones 2006-11-21 10:12) - PLID 23415 - supported the 2430 loop
					// (j.jones 2010-03-31 15:19) - PLID 37918 - send the count of charges
					// (j.jones 2012-03-21 14:20) - PLID 48870 - We now export 2430 for each "other" insurance
					// previously sent in 2320, 2330A, and 2330B. This array may be empty, have one record, or multiple.
					for(int ip = 0; ip < aryOtherInsuredPartyInfo.GetSize(); ip++) {
						OtherInsuranceInfo oInfo = (OtherInsuranceInfo)aryOtherInsuredPartyInfo.GetAt(ip);

						//Line Adjudication Information
						RETURN_ON_FAIL(ANSI_5010_2430(oInfo, rsCharges, bSentAllowedAmount, cyAllowableSent, rsCharges->GetRecordCount()));
					}

					rsCharges->MoveNext();

				}	//end charges loop

				rsCharges->Close();

				//this claim is done, now do all the calculations needed prior to the next claim

				//set the previous provider/ins.party, as this is the end of the claim
				m_PrevProvider = m_pEBillingInfo->ProviderID;
				m_PrevInsCo = m_pEBillingInfo->InsuranceCoID;
				m_PrevInsParty = m_pEBillingInfo->InsuredPartyID;
				m_PrevLocation = m_pEBillingInfo->BillLocation;

				//JJ - because of fun rounding and division and lovely math all around, it is possible for
				//us to increment too high on the last claim (ex. 9 claims would increase it to 200.0001).
				//If you aren't sending to Envoy, this will be greater than the max and therefore cause
				//an error. This code is a safeguard against going past 200. And since we will always go up to
				//200 even if we are sending claims, we will still adjust the code here regardless.
				if(m_Progress.GetValue()+m_ProgIncrement > 200.0)
					m_Progress.SetValue(200.0);
				else
					m_Progress.SetValue(m_Progress.GetValue() + m_ProgIncrement);

			}	//end claims loop

			RETURN_ON_FAIL(ANSI_5010_Trailer());	//Trailer

			RETURN_ON_FAIL(ANSI_5010_FunctionalGroupTrailer()); //Functional Group Trailer

			RETURN_ON_FAIL(ANSI_5010_InterchangeTrailer()); //Interchange Control Trailer
		}
		else {
			//4010

			// (j.jones 2010-10-13 10:48) - PLID 40913 - renamed all ANSI 4010 functions to state 4010 in the function header,
			// the body for each of these functions is now in EbillingANSI4010.cpp

			RETURN_ON_FAIL(ANSI_4010_InterchangeHeader());	//Interchange Control Header

			RETURN_ON_FAIL(ANSI_4010_FunctionalGroupHeader());	//Functional Group Header

			//set this segment count to zero AFTER the functional headers are called.
			//We only want to count the segments between the transaction header and trailer.
			m_ANSISegmentCount = 0;
			
			RETURN_ON_FAIL(ANSI_4010_Header());	//Header
			
			RETURN_ON_FAIL(ANSI_4010_1000A());	//Submitter Info

			RETURN_ON_FAIL(ANSI_4010_1000B());	//Receiver Info

			//initialize all counters
			m_ANSIHLCount = m_ANSICurrProviderHL = m_ANSICurrSubscriberHL = m_ANSICurrPatientParent = m_ANSIClaimCount = m_ANSIServiceCount = 0;

			//loop through all claims
			for(int i=0;i<m_aryEBillingInfo.GetSize();i++) {

				str.Format("Exporting to Disk - Claim %li of %li",i+1,m_aryEBillingInfo.GetSize());
				SetDlgItemText(IDC_CURR_EVENT,str);
				
				m_pEBillingInfo = ((EBillingInfo*)m_aryEBillingInfo.GetAt(i));

				m_HCFAInfo.LoadData(m_pEBillingInfo->HCFASetupID);
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType == actInst) {
					m_UB92Info.LoadData(m_pEBillingInfo->UB92SetupID);
				}

				TRACE("HCFA ID=%d\r\n", m_pEBillingInfo->HCFAID);
				TRACE("==========================================================\r\n");

				
				//loop through each provider

				// (j.jones 2006-12-06 15:49) - PLID 23631 - if enabled, separate batches by insurance company as well
				// (j.jones 2007-01-30 09:05) - PLID 24411 - also separate by location
				// (j.jones 2008-05-06 11:18) - PLID 29937 - cached m_bSeparateBatchesByInsCo
				if(m_PrevProvider != m_pEBillingInfo->ProviderID || m_PrevLocation != m_pEBillingInfo->BillLocation || (m_bSeparateBatchesByInsCo && m_PrevInsCo != m_pEBillingInfo->InsuranceCoID)) {

					RETURN_ON_FAIL(ANSI_4010_2000A());	//Billing/Pay-To Provider HL (Hierarchical Level)
					RETURN_ON_FAIL(ANSI_4010_2010AA());	//Billing Provider Name

					//JJ - Yet again, another clearinghouse broke ANSI. The 2010AB should never be exported if it is the same
					//provider as 2010AA, which it always is, yet NDEX in Washington wants it anyways.
					// (j.jones 2008-05-06 11:30) - PLID 29937 - cached m_bUse2010AB
					if(m_bUse2010AB) {
						RETURN_ON_FAIL(ANSI_4010_2010AB());	//Pay-To-Provider Name
					}

					//fix the counts so that we restart under this provider
					m_ANSIClaimCount = m_ANSIServiceCount = 0;
				}

				//loop through all subscribers (insured parties)

				//JJ - if the provider changed, then we have to output these again

				// (j.jones 2006-12-06 15:49) - PLID 23631 - if enabled, separate batches by insurance company as well
				// (j.jones 2007-01-30 09:09) - PLID 24411 - same for locations
				// (j.jones 2008-02-15 12:32) - PLID 28943 - we now have a setting to always re-send the subscriber level
				// for every claim, apparently some clearinghouses and insurance companies can't handle the ANSI standard
				// of sending multiple 2300 loops under one 2000B header, so if checked (which is the default), always
				// restart at the subscriber level for every claim
				// (j.jones 2008-05-06 11:18) - PLID 29937 - cached m_bSeparateBatchesByInsCo
				if(m_bSendSubscriberPerClaim || m_PrevInsParty != m_pEBillingInfo->InsuredPartyID || m_PrevProvider != m_pEBillingInfo->ProviderID || m_PrevLocation != m_pEBillingInfo->BillLocation || (m_bSeparateBatchesByInsCo && m_PrevInsCo != m_pEBillingInfo->InsuranceCoID)) {
			
					RETURN_ON_FAIL(ANSI_4010_2000B());	//Subscriber HL
					RETURN_ON_FAIL(ANSI_4010_2010BA());	//Subscriber Name
					RETURN_ON_FAIL(ANSI_4010_2010BB());	//Payer Name

					m_ANSIClaimCount = m_ANSIServiceCount = 0;

					//the current patient number (used for lower levels) will be the subscriber number
					m_ANSICurrPatientParent = m_ANSICurrSubscriberHL;

					//if the patient is different from the subscriber, then export these two lines

					if(IsRecordsetEmpty("SELECT RelationToPatient FROM InsuredPartyT WHERE PersonID = %li AND RelationToPatient = 'Self'",m_pEBillingInfo->InsuredPartyID)) {

						RETURN_ON_FAIL(ANSI_4010_2000C());	//Patient HL
						RETURN_ON_FAIL(ANSI_4010_2010CA());	//Patient Name
					}
				}

				// (j.jones 2011-04-20 10:45) - PLID 41490 - the charge filtering has been moved here so
				// we have a unified list of all charges we are about to export

				CArray<long, long> aryChargesToExport;

				{
					CSqlFragment sqlDocProv("");

					// (j.jones 2006-12-01 15:50) - PLID 22110 - supported the ClaimProviderID
					// (j.jones 2008-04-03 09:23) - PLID 28995 - supported the HCFAClaimProvidersT setup
					// (j.jones 2008-08-01 10:17) - PLID 30917 - HCFAClaimProvidersT is now per insurance company
					// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
					// (j.jones 2010-11-09 12:57) - PLID 41389 - supported ChargesT.ClaimProviderID
					if(m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 2) {
						CSqlFragment sql("AND (ChargesT.ClaimProviderID = {INT} OR (ChargesT.ClaimProviderID Is Null AND ChargesT.DoctorsProviders IN "
							"(SELECT PersonID FROM "
							"	(SELECT ProvidersT.PersonID, "
							"	(CASE WHEN ANSI_2010AA_ProviderID Is Null THEN ProvidersT.ClaimProviderID ELSE ANSI_2010AA_ProviderID END) AS ProviderIDToUse "
							"	FROM ProvidersT "
							"	LEFT JOIN (SELECT * FROM HCFAClaimProvidersT WHERE InsuranceCoID = {INT}) AS HCFAClaimProvidersT ON ProvidersT.PersonID = HCFAClaimProvidersT.ProviderID) SubQ "
							"WHERE ProviderIDToUse = {INT}))) ", m_pEBillingInfo->ProviderID, m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);

						sqlDocProv = sql;
					}
					else if(m_actClaimType == actInst && m_UB92Info.Box82Setup == 1) {
						CSqlFragment sql("AND (ChargesT.ClaimProviderID = {INT} OR (ChargesT.ClaimProviderID Is Null AND ChargesT.DoctorsProviders IN (SELECT PersonID FROM ProvidersT WHERE ClaimProviderID = {INT}))) ", m_pEBillingInfo->ProviderID, m_pEBillingInfo->ProviderID);

						sqlDocProv = sql;
					}


					// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
					_RecordsetPtr rsCharges = CreateParamRecordset("SELECT ChargesT.ID AS ChargeID "
						"FROM ChargesT "
						"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
						"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
						"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
						"WHERE ChargesT.BillID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
						"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
						"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
						"{SQL} "
						"ORDER BY ChargesT.LineID", m_pEBillingInfo->BillID, sqlDocProv);
					while(!rsCharges->eof) {
						aryChargesToExport.Add(VarLong(rsCharges->Fields->Item["ChargeID"]->Value));

						rsCharges->MoveNext();
					}
					rsCharges->Close();
				}

				// (a.walling 2014-03-05 16:29) - PLID 61202 - GetDiagnosisCodesForHCFA - Pass bUseICD10 as appropriate for this InsuredParty and BillID
				bool bUseICD10 = ShouldUseICD10();

				// (a.walling 2014-03-19 10:05) - PLID 61346 - Load all diag codes for the bill
				m_pEBillingInfo->billDiags = GetBillDiagCodes(m_pEBillingInfo->BillID, bUseICD10, ShouldSkipUnlinkedDiags(), &aryChargesToExport);

				//loop through bills

				//most of the loops are so similar between HCFAs and UB92s, that we just call those loops
				//and let individual 'if' statements modify them. However the 2300 loop is a different format,
				//and the 23x0 sub-loops are all re-arranged, so we have two separate functions for each loop.
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType != actInst) {
					//Professional (HCFA) Claim

					// (j.jones 2011-03-07 14:00) - PLID 42660 - load the CLIA information
					// (j.jones 2011-04-05 13:39) - PLID 42372 - this now returns a struct of information
					ECLIANumber eCLIANumber = UseCLIANumber(m_pEBillingInfo->BillID,m_pEBillingInfo->InsuredPartyID,m_pEBillingInfo->ProviderID, m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->BillLocation);
					BOOL bUseProvAsRefPhy = (eCLIANumber.bUseCLIANumber && eCLIANumber.bCLIAUseBillProvInHCFABox17);

					// (j.jones 2011-03-07 14:12) - PLID 42660 - pass in the CLIA data
					// (j.jones 2011-04-20 10:45) - PLID 41490 - we now pass in info. on skipping diagnosis codes
					RETURN_ON_FAIL(ANSI_4010_2300_Prof(eCLIANumber));	//Claim Info

					// (j.jones 2008-12-11 13:25) - PLID 32401 - moved the ref phy. ID into the struct
					// (j.jones 2011-03-07 14:11) - PLID 42660 - check to see if the CLIA setup is configured to send the
					// billing provider as the referring provider, and if so, send the billing provider
					if(m_pEBillingInfo->nRefPhyID != -1 || bUseProvAsRefPhy) {
						RETURN_ON_FAIL(ANSI_4010_2310A_Prof(bUseProvAsRefPhy));	//Referring Provider
					}

					//Only use 2310B when different than 2000A, which would be when 2000A is a group
					// (j.jones 2010-01-11 13:43) - PLID 36831 - default to TRUE, we always want to send
					// it unless told not to, and remember that there might not be an EbillingSetupT record!
					BOOL bSend2310B = TRUE;

					//send 2310B if a group
					/*already defaults to TRUE, but do not forget this if we ever change the default to FALSE
					if(m_bIsGroup) {
						bSend2310B = TRUE;
					}
					*/

					// (j.jones 2009-08-03 15:14) - PLID 33827 - send 2310B if the override says so				
					//It is really not clear whether this setting should load for the 2010AA Billing Provider (as it does now),
					//or from the 2310B Rendering Provider, which you would think as it has 2310B in the name.
					//We pondered, and ultimately we concurred that this setting is indeed a property of the 2010AA provider,
					//it's for that provider that the decision is made to send the 2310B record.
					// (j.jones 2010-01-11 13:45) - PLID 36831 - changed to send unless the override says not to,
					// but if it is a group, it will always send, ignoring the override
					if(!m_bIsGroup) {
						_RecordsetPtr rsOver = CreateParamRecordset("SELECT Export2310BRecord FROM EbillingSetupT "
							"WHERE Export2310BRecord = 0 AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",m_pEBillingInfo->HCFASetupID,m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation);
						if(!rsOver->eof) {
							bSend2310B = FALSE;
						}
						rsOver->Close();
					}

					if(bSend2310B) {
						RETURN_ON_FAIL(ANSI_4010_2310B_Prof());	//Rendering Provider
					}

					//if the POS code is 11 or 12 and they want to hide 2310D when it's 11 or 12, hide it!
					// (j.jones 2012-01-06 15:16) - PLID 47351 - this is Hide2310D in data, and this is still loop 2310D
					// in 4010, but I renamed the variable to be 5010 compliant and make more sense with respect to our
					// other new 2310C-hiding ability (which is only used in 5010)
					// (j.jones 2013-04-25 09:06) - PLID 55564 - we now load the place code in our ebilling object, so we no longer need a query						
					BOOL bHide2310D = FALSE;
					if(m_bHide2310C_WhenType11Or12
						&& (m_pEBillingInfo->strPOSCode == "11" || m_pEBillingInfo->strPOSCode == "12")) {

						// They want to hide 2310D when the place of service code is 11 or 12, and it is.
						// So do not send 2310D.
						bHide2310D = TRUE;
					}
					if(!bHide2310D) {
						RETURN_ON_FAIL(ANSI_4010_2310D_Prof());	//Service Facility Location
					}

					// (j.jones 2008-12-11 13:24) - PLID 32401 - supported loop 2310E
					if(m_pEBillingInfo->nSupervisingProviderID != -1) {
						RETURN_ON_FAIL(ANSI_4010_2310E_Prof());	//Supervising Provider
					}
				}
				else {
					//Institutional (UB92) Claim

					RETURN_ON_FAIL(ANSI_4010_2300_Inst());	//Claim Info

					
					RETURN_ON_FAIL(ANSI_4010_2310A_Inst());	//Attending Physician

					// (j.jones 2011-08-23 09:15) - PLID 44984 - if we have a 2310B provider, send it
					if(m_pEBillingInfo->UB_2310B_ProviderID != -1) {
						RETURN_ON_FAIL(ANSI_4010_2310B_Inst());	//Operating Physician
					}

					// (j.jones 2011-08-23 09:15) - PLID 44984 - if we have a 2310C provider, send it
					if(m_pEBillingInfo->UB_2310C_ProviderID != -1) {
						RETURN_ON_FAIL(ANSI_4010_2310C_Inst());	//Other Operating Physician
					}

					//if the POS code is 11 or 12 and they want to hide 2310D when it's 11 or 12, hide it!
					/*
					if(!m_bHide2310D ||
						IsRecordsetEmpty("SELECT ServiceLocation FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
									"WHERE Deleted = 0 AND BillID = %li AND Batched = 1 AND (ServiceLocation = '11' OR ServiceLocation = '12') ",m_pEBillingInfo->BillID)) {

						if(m_actClaimType != actInst) {  //not used on the UB92 when 2000A is used, which is always

							RETURN_ON_FAIL(ANSI_4010_2310D_Inst());	//Service Facility Location
						}
					}
					*/
				}

				

				//reset the count of charges
				m_ANSIServiceCount = 0;

				//if there is another insurance company, export these three lines

				//(j.jones 2003-07-24 15:30) - there is now an ANSI setting to NOT export secondaries
				// (j.jones 2008-05-06 11:22) - PLID 29937 - cached m_bDontSubmitSecondary
				if(m_pEBillingInfo->OthrInsuredPartyID != -1 && !m_bDontSubmitSecondary) {

					RETURN_ON_FAIL(ANSI_4010_2320());	//Other Subscriber Info
					RETURN_ON_FAIL(ANSI_4010_2330A());	//Other Subscriber Name
					RETURN_ON_FAIL(ANSI_4010_2330B());	//Other Payer Name
				}

				//loop through charges

				//if using the bill provider, filter only on that doctor's charges, otherwise get all charges
				//in ANSI, we're using the bill provider if we are using anything but the G1 provider

				// (j.jones 2011-04-20 10:45) - PLID 41490 - the charge filtering has been moved to earlier in this code

				// (j.jones 2008-04-30 12:15) - PLID 28283 - ordered the charges by LineID
				// (j.jones 2008-05-14 15:36) - PLID 30044 - added test result fields
				// (j.jones 2008-05-28 14:31) - PLID 30176 - added NDCCode
				// (j.jones 2009-08-12 17:29) - PLID 35096 - added other drug information
				// (j.jones 2011-09-19 15:32) - PLID 45526 - added LineNote - which is the Notes record flagged to send on claim
				// (j.gruber 2014-03-17 10:55) - PLID 61394 - updated for WhichCodes new structure
				// (a.walling 2014-03-19 10:11) - PLID 61418 - EBilling - ANSI4010 - DiagCodes and WhichCodes must be unique and filtered for NULLs. Use ChargeWhichICD9CodesIF/10 and the BillDiags
				_RecordsetPtr rsCharges = CreateParamRecordset("SELECT ChargesT.ID AS ChargeID, ChargesT.ServiceID, ChargesT.*, LineItemT.*, "
					"ServiceT.Anesthesia, ServiceT.UseAnesthesiaBilling, dbo.GetChargeTotal(ChargesT.ID) AS LineTotal, "
					"BillsT.AnesthesiaMinutes, OthrInsuranceCoT.InsType, "
					"SendTestResults, TestResultID, TestResultType, TestResult, NDCCode, "
					"DrugUnitPrice, DrugUnitTypeQual, DrugUnitQuantity, PrescriptionNumber, "
					"LineNotesQ.LineNote, ChargeWhichCodesQ.WhichCodes AS ChargeWhichCodes "
					"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
					"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
					"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
					"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
					"LEFT JOIN InsuredPartyT OthrInsuredPartyT ON BillsT.OthrInsuredPartyID = OthrInsuredPartyT.PersonID "
					"LEFT JOIN InsuranceCoT OthrInsuranceCoT ON OthrInsuredPartyT.InsuranceCoID = OthrInsuranceCoT.PersonID "
					"OUTER APPLY dbo.{CONST_STRING}({STRING}, ChargesT.ID) ChargeWhichCodesQ "
					"LEFT JOIN (SELECT Min(Note) AS LineNote, LineItemID FROM Notes "
					"	WHERE LineItemID Is Not Null AND SendOnClaim = 1 "
					"	GROUP BY LineItemID) AS LineNotesQ ON ChargesT.ID = LineNotesQ.LineItemID "
					"WHERE ChargesT.ID IN ({INTARRAY}) "
					"ORDER BY ChargesT.LineID"
					, ShouldUseICD10() ? "ChargeWhichICD10CodesIF" : "ChargeWhichICD9CodesIF"
					, MakeBillDiagsXml(m_pEBillingInfo->billDiags)
					, aryChargesToExport);

				if(rsCharges->eof) {
					//if the recordset is empty, there is no patient. So halt everything!!!
					// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
					CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
					if(!strPatientName.IsEmpty()) {
						// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
						str.Format("No charges were found for '%s', Bill ID %li. (ExportToANSI)\n\n"
							"Contact NexTech for assistance.", strPatientName, m_pEBillingInfo->BillID);
					}
					else
						//serious problems if you get here
						str = "Could not open charge information. (ExportToANSI)";

					AfxMessageBox(str);
					return Error_Missing_Info;
				}

				while(!rsCharges->eof) {

					//most of the loops are so similar between HCFAs and UB92s, that we just call those loops
					//and let individual 'if' statements modify them. However the 2400 loop is a different format,
					//so we have two separate functions altogether

					// (j.jones 2008-05-23 10:49) - PLID 29084 - loop 2430 needs to know if an allowable
					// was sent in 2400, and the amount
					BOOL bSentAllowedAmount = FALSE;
					COleCurrency cyAllowableSent = COleCurrency(0,0);

					// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
					if(m_actClaimType != actInst) {
						// (j.jones 2011-04-20 10:45) - PLID 41490 - we now pass in info. on skipping diagnosis codes
						RETURN_ON_FAIL(ANSI_4010_2400_Prof(rsCharges, bSentAllowedAmount, cyAllowableSent));	//Service Line (HCFA/Professional claim)
					}
					else {
						RETURN_ON_FAIL(ANSI_4010_2400_Inst(rsCharges));	//Service Line (UB92/Institutional claim)
					}

					// (j.jones 2008-05-28 14:14) - PLID 30176 - added support for Loop 2410 and NDC Codes,
					// this is supported in both HCFA and UB claims
					CString strNDCCode = AdoFldString(rsCharges, "NDCCode", "");
					strNDCCode.TrimLeft();
					strNDCCode.TrimRight();
					if(!strNDCCode.IsEmpty()) {

						// (j.jones 2009-08-12 17:30) - PLID 35096 - Also send the other drug information
						// into loop 2410, but we require the NDC code to send the loop as a whole.
						COleCurrency cyDrugUnitPrice = AdoFldCurrency(rsCharges, "DrugUnitPrice", COleCurrency(0,0));
						CString strUnitType = AdoFldString(rsCharges, "DrugUnitTypeQual", "");
						double dblUnitQty = AdoFldDouble(rsCharges, "DrugUnitQuantity", 0.0);
						CString strPrescriptionNumber = AdoFldString(rsCharges, "PrescriptionNumber", "");

						RETURN_ON_FAIL(ANSI_4010_2410(strNDCCode, cyDrugUnitPrice, strUnitType, dblUnitQty, strPrescriptionNumber));
					}

					//JJ - 3/4/2003 - Okay the presence of the 2420A record is up for debate.
					//The "correct" ANSI implementation is to follow the commented out code below,
					//(labeled "CORRECT ANSI BEHAVIOR") with the instructions I laid out.
					//Right now, we orchestrate the export in such a way that the 2420A record
					//will never be used. But we could change it eventually, at which point it will be needed.
					//However we have encountered at least one clearinghouse (Noridian Blue Cross)
					//that felt the need to always want the 2420A record, regardless if there is only 
					//one provider. This, of course, is an error state for anyone following the ANSI guidelines.
					//So once again, we have had to accomodate this crazy clearing house by adding a 
					//checkbox to the ANSI properties.

					/*==== BEGIN RIDICULOUSLY INCORRECT IMPLEMENTATION HERE ====*/

					//sigh, here we go:

					// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
					if(m_actClaimType != actInst) {

						// (j.jones 2008-05-06 11:22) - PLID 29937 - cached m_bUse2420A
						if(m_bUse2420A) {
							// (j.jones 2008-04-03 09:55) - PLID 28995 - this function now takes in the ANSI_RenderingProviderID value
							RETURN_ON_FAIL(ANSI_4010_2420A(m_pEBillingInfo->ANSI_RenderingProviderID));
						}
					}


					/*==== END RIDICULOUSLY INCORRECT IMPLEMENTATION HERE ====*/

					//CORRECT ANSI BEHAVIOR: Right now we export a new claim for each rendering provider, 
					//and only the charges for that claim. If we choose to keep the bills together, 
					//comment this code in (it already works) and make the appropriate changes elsewhere.
					//Remove the filter for doctor from the rsCharges recordset, and change the query in LoadClaimInfo.
					//Also, don't forget to still accomodate the code in the above "ridiculously incorrect
					//implementation" block.

					/*

					TODO: If we ever comment this code in, use the charge provider's ClaimProviderID,
					not the charge provider ID

					//Check if the rendering provider for this charge is different from the provider
					//we determined to be the "default provider" in 2310B. But, this will only be if we are
					//using the bill provider to begin with.
					if(m_actClaimType != actInst) {

						long RenProvID = -1;
						_variant_t var = rsCharges->Fields->Item["DoctorsProviders"]->Value;
						if(var.vt == VT_I4 && var.lVal > 0) {
							RenProvID = var.lVal;
						}
						if(m_pEBillingInfo->Box33Setup==1 && RenProvID != m_pEBillingInfo->ProviderID) 
							RETURN_ON_FAIL(ANSI_4010_2420A(RenProvID));	//Rendering Provider Name
					}
					*/

					// (j.jones 2006-11-21 10:12) - PLID 23415 - supported the 2430 loop
					// (j.jones 2010-03-31 15:19) - PLID 37918 - send the count of charges
					RETURN_ON_FAIL(ANSI_4010_2430(rsCharges, bSentAllowedAmount, cyAllowableSent, rsCharges->GetRecordCount()));	//Line Adjudication Information

					rsCharges->MoveNext();

				}	//end charges loop

				rsCharges->Close();

				//this claim is done, now do all the calculations needed prior to the next claim

				//set the previous provider/ins.party, as this is the end of the claim
				m_PrevProvider = m_pEBillingInfo->ProviderID;
				m_PrevInsCo = m_pEBillingInfo->InsuranceCoID;
				m_PrevInsParty = m_pEBillingInfo->InsuredPartyID;
				m_PrevLocation = m_pEBillingInfo->BillLocation;

				//JJ - because of fun rounding and division and lovely math all around, it is possible for
				//us to increment too high on the last claim (ex. 9 claims would increase it to 200.0001).
				//If you aren't sending to Envoy, this will be greater than the max and therefore cause
				//an error. This code is a safeguard against going past 200. And since we will always go up to
				//200 even if we are sending claims, we will still adjust the code here regardless.
				if(m_Progress.GetValue()+m_ProgIncrement > 200.0)
					m_Progress.SetValue(200.0);
				else
					m_Progress.SetValue(m_Progress.GetValue() + m_ProgIncrement);

			}	//end claims loop

			RETURN_ON_FAIL(ANSI_4010_Trailer());	//Trailer

			RETURN_ON_FAIL(ANSI_4010_FunctionalGroupTrailer()); //Functional Group Trailer

			RETURN_ON_FAIL(ANSI_4010_InterchangeTrailer()); //Interchange Control Trailer
		}

		m_OutputFile.Close();

		bool bUnselectClaims = false;
		bool bUnbatchClaims = false;
		if (m_bIsClearinghouseIntegrationEnabled)
		{
			SetDlgItemText(IDC_CURR_EVENT, "Uploading to TriZetto. Please wait...");
			if (!UploadClaimFile())
			{
				EndDialog(IDCANCEL);
				return Cancel_Silent;
			}
			m_Progress.SetValue(m_Progress.GetMax());
			MessageBox("All claims were successfully sent to TriZetto. These claims will now be unbatched.", "Success", MB_ICONINFORMATION | MB_OK);
			bUnselectClaims = true;
			bUnbatchClaims = true;
		}
		else
		{
			m_Progress.SetValue(m_Progress.GetMax());
		}

		if (!m_bIsClearinghouseIntegrationEnabled && !m_bFileExportOnly)
		{
			// (j.jones 2015-11-19 14:10) - PLID 67600 - updating the claim history is now called before the export

			str = "All claims were successfully exported and are ready to send to your Ebilling Clearinghouse.\n\nDo you want to transfer these claims to the unselected list?";
			// (j.jones 2008-05-06 11:24) - PLID 29937 - cached m_bEbillingFormatZipped
			// (b.spivey, August 27th, 2014) - PLID 63492 - if it's a single file export this shouldn't matter. 
			if (m_bEbillingFormatZipped)
			{
				// (j.jones 2009-06-16 17:34) - PLID 34643 - give a clean message if this fails
				if (!ZipFile())
				{
					str = "All claims were exported, but the export file failed to zip correctly. "
						"You will need to manually zip the claim file before submitting to your Ebilling Clearinghouse."
						"Please contact NexTech Technical Support for assistance.\n\n"
						"Do you want to transfer these claims to the unselected list?";
				}
			}

			if (MessageBox(str, "Electronic Billing", MB_YESNO | MB_ICONINFORMATION) == IDYES)
			{
				bUnselectClaims = true;
			}
		}

		if (bUnselectClaims)
		{
			CSqlFragment sqlWhereClause("WHERE Coalesce(BillStatusT.Type, -1) <> {CONST_INT}", EBillStatusType::OnHold);
			if (m_nFilterByProviderID != -1)
			{
				sqlWhereClause += CSqlFragment(R"(
AND BillsT.ID IN 
(
	SELECT BillID 
	FROM ChargesT 
	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID 
	LEFT JOIN 
	(
		SELECT OriginalLineItemID 
		FROM LineItemCorrectionsT
	) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID 
	LEFT JOIN 
	(
		SELECT VoidingLineItemID 
		FROM LineItemCorrectionsT
	) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID 
	WHERE 
		LineItemT.Deleted = 0 
		AND ChargesT.Batched = 1
		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID IS NULL
		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID IS NULL
		AND ChargesT.DoctorsProviders = {INT}
))", m_nFilterByProviderID);
			}
			if (m_nFilterByLocationID != -1)
			{
				sqlWhereClause += CSqlFragment(R"(
AND BillsT.ID IN 
(
	SELECT BillID 
	FROM ChargesT 
	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID
	LEFT JOIN 
	(
		SELECT OriginalLineItemID 
		FROM LineItemCorrectionsT
	) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID
	LEFT JOIN 
	(
		SELECT VoidingLineItemID 
		FROM LineItemCorrectionsT
	) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID
	WHERE 
		LineItemT.Deleted = 0 
		AND ChargesT.Batched = 1 
		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID IS NULL
		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID IS NULL
		AND LineItemT.LocationID = {INT}
))", m_nFilterByLocationID);
			}

			// (j.jones 2008-05-06 16:53) - PLID 27453 - parameterized
			// (j.jones 2014-07-09 10:26) - PLID 62568 - do not move on hold claims
			ExecuteParamSql("UPDATE HCFATrackT SET CurrentSelect = 0 WHERE Batch = 2 AND BillID IN ("
				"SELECT BillsT.ID FROM BillsT "
				"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
				"{SQL} "
				")", sqlWhereClause);
		}
		if (bUnbatchClaims)
		{
			bool bSomeSucceededToUnbatch = false;
			bool bSomeFailedToUnbatch = false;
			for (int i = 0; i < m_aryEBillingInfo.GetSize(); i++)
			{
				EBillingInfo *pEBillingInfo = (EBillingInfo*)m_aryEBillingInfo.GetAt(i);
				BatchChange::Status eBatchChangeStatus = BatchBill(pEBillingInfo->BillID, 0, TRUE);
				if(eBatchChangeStatus != BatchChange::Status::Unbatched)
				{
					bSomeFailedToUnbatch = true;
				}
				if (eBatchChangeStatus == BatchChange::Status::Unbatched)
				{
					bSomeSucceededToUnbatch = true;
				}
			}
			if (!bSomeSucceededToUnbatch)
			{
				MessageBox("Nextech failed to automatically unbatch the selected claims. You will need to unbatch these manually.", "Error", MB_ICONERROR | MB_OK);
			}
			else if (bSomeFailedToUnbatch)
			{
				MessageBox("Nextech failed to automatically unbatch some of the selected claims. You will need to unbatch these manually.", "Error", MB_ICONERROR|MB_OK);
			}
		}
		if (bUnselectClaims && ReturnsRecordsParam(R"(
SELECT TOP 1 NULL 
FROM HCFATrackT 
WHERE 
	CurrentSelect = 1 
	AND Batch = 2 
	AND BillID IN
	(
		SELECT BillsT.ID 
		FROM BillsT
		LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID
		WHERE Coalesce(BillStatusT.Type, -1) <> {CONST_INT}
	))", EBillStatusType::OnHold))
		{
			MessageBox("There are still some batched claims in the selected list that are hidden due to current filters.", NULL, MB_ICONINFORMATION|MB_OK);
		}

		EndDialog(IDOK);

		// (b.spivey, August 28, 2014) - PLID 63492 - Show the file then delete it on exit. 
		if (m_bFileExportOnly) {
			ShellExecute((HWND)GetDesktopWindow(), NULL, "notepad.exe", m_ExportName, NULL, SW_SHOW);
			FileUtils::DeleteFileOnTerm(m_ExportName);
		}

	return Success;

	} NxCatchAll("Error in Ebilling::ExportToANSI");

	return Error_Other;

}

// (j.jones 2010-10-13 10:48) - PLID 40913 - all ANSI 4010 functions are now in EbillingANSI4010.cpp,
// and ANSI 5010 functions are in EbillingANSI5010.cpp

// (j.jones 2006-11-09 08:29) - PLID 21570 - added support for OHIP

/****************************************************************************************************
*
*	OHIP
*
*	This function in its entirety will export an OHIP file..
*
*
*
****************************************************************************************************/

int CEbilling::ExportToOHIP()
{
	try {

		//-2, because if they have an invalid ProviderID/InsCoID it would be -1, and we compare those IDs
		//prior to stopping the export for an invalid Provider/InsCoID
		m_PrevProvider = -2;

		m_OHIP_ClaimHeader1_Count = 0;
		m_OHIP_ClaimHeader2_Count = 0;
		m_OHIP_Item_Record_Count = 0;

		CString OutputString, str;

		SetDlgItemText(IDC_CURR_EVENT, "Exporting to Disk");

		_RecordsetPtr rsBills;

		//open the file for writing
		if (!m_OutputFile.Open(m_ExportName, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive)) {
			// (j.armen 2011-10-25 13:42) - PLID 46134 - Ebilling is located in the practice path
			// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure. Also, changed to FileUtils::CreatePath() so that 
			// non-existent intermediate paths are created.
			FileUtils::CreatePath(FileUtils::GetFilePath(m_ExportName));
			if (!m_OutputFile.Open(m_ExportName, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive)) {
				AfxMessageBox("The ebilling export file could not be created. Contact Nextech for assistance.");
				return Error_Other;
			}
		};

		//update the batch number

		// update LastBatchID
		int tempint = 0;
		tempint = GetRemotePropertyInt(_T("LastBatchID"), 0, 0, _T("<None>"));
		if (tempint == 0) {
			SetRemotePropertyInt(_T("LastBatchID"), 1, 0, _T("<None>"));
			m_BatchID = 1;
		}
		else { // record does exist
			m_BatchID = tempint + 1;
			SetRemotePropertyInt(_T("LastBatchID"), m_BatchID, 0, _T("<None>"));
		}  // end of LastBatchID update

		m_strBatchNumber.Format("%li", m_BatchID);
		while (m_strBatchNumber.GetLength() < 4)
			m_strBatchNumber.Insert(0, '0');

		for (int i = 0; i < m_aryEBillingInfo.GetSize(); i++) {

			OutputString = "";

			str.Format("Exporting to Disk - Claim %li of %li", i + 1, m_aryEBillingInfo.GetSize());
			SetDlgItemText(IDC_CURR_EVENT, str);

			m_pEBillingInfo = ((EBillingInfo*)m_aryEBillingInfo.GetAt(i));

			// (a.walling 2014-03-19 10:05) - PLID 61346 - Load all diag codes for the bill
			m_pEBillingInfo->billDiags = GetBillDiagCodes(m_pEBillingInfo->BillID, ShouldUseICD10(), false, NULL);

			TRACE("Claim ID=%d\r\n", m_pEBillingInfo->HCFAID);
			TRACE("==========================================================\r\n");

			//send a new batch whenever the provider changes
			if (m_PrevProvider != m_pEBillingInfo->ProviderID) {
				//we may need a new trailer
				if (i > 0) {
					RETURN_ON_FAIL(OHIP_Batch_Trailer());
				}
				RETURN_ON_FAIL(OHIP_Batch_Header());
			}

			//find out the plan name, it must be HCP, WCB, or RMB
			//if empty or invalid, assume HCP
			CString strPlanName = "HCP";
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT PlanName FROM InsurancePlansT "
				"WHERE ID IN (SELECT InsPlan FROM InsuredPartyT WHERE PersonID = {INT})", m_pEBillingInfo->InsuredPartyID);
			if (!rs->eof) {
				CString strPlan = AdoFldString(rs, "PlanName", "");
				strPlan.TrimLeft();
				strPlan.TrimRight();
				strPlan.MakeUpper();
				if (strPlan == "WCB" || strPlan == "RMB")
					strPlanName = strPlan;
			}
			rs->Close();

			//begin exporting the OHIP file
			RETURN_ON_FAIL(OHIP_Claim_Header1(strPlanName));

			//only used if RMB
			if (strPlanName == "RMB") {
				RETURN_ON_FAIL(OHIP_Claim_Header2());
			}

			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			// (j.jones 2009-10-29 12:11) - PLID 36060 - supported batched charges
			// (j.jones 2011-06-27 16:21) - PLID 44335 - now we always filter by our provider ID
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			// (j.gruber 2014-03-17 10:26) - PLID 61393 - update billing structure - no changes for ICD-10
			// (a.walling 2014-03-19 10:49) - PLID 61420 - EBilling - OHIP - DiagCodes and WhichCodes must be unique and filtered for NULLs. Use the BillDiags
			_RecordsetPtr rsCharges = CreateParamRecordset("SELECT "
				"ChargesT.ItemCode, dbo.GetChargeTotal(ChargesT.ID) AS LineTotal, "
				"ChargesT.Quantity, ChargesT.ServiceDateFrom "
				"FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = {INT} AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ChargesT.DoctorsProviders END) = {INT} "
				"ORDER BY ChargesT.LineID ", m_pEBillingInfo->BillID, m_pEBillingInfo->ProviderID);
			while (!rsCharges->eof) {

				RETURN_ON_FAIL(OHIP_Item_Record(rsCharges));

				//the OHIP_Item_Record function can advance the recordset,
				//so be careful to check for eof
				if (!rsCharges->eof) {
					rsCharges->MoveNext();
				}
			}
			rsCharges->Close();

			//set the previous provider/InsCo, as this is the end of the claim
			m_PrevProvider = m_pEBillingInfo->ProviderID;
		}

		//always send our last trailer
		RETURN_ON_FAIL(OHIP_Batch_Trailer());

		m_OutputFile.Close();

		m_Progress.SetValue(m_Progress.GetMax());

		//TES 11/5/2007 - PLID 27978 - VS 2008 - "export" is now a keyword, so this was causing warnings.
		// And since this was an unreferenced variable anyway, I just commented it out.
		//CString export;

		// (j.jones 2015-11-19 14:10) - PLID 67600 - updating the claim history is now called before the export

		// (j.jones 2009-08-14 11:45) - PLID 34914 - if we're sending for a specific OHIP provider ID,
		// it should have been given to us by the caller
		// (b.spivey, June 26th, 2014) - PLID 62603 - Check if auto dialing is enabled. If it is, 
		//		behave as normal. If not, we don't do any crazy dial out to OHIP stuff, just exit gracefully. 
		BOOL bSentSuccessfully;
		bool bAutoDialEnabled = !!(BOOL)GetRemotePropertyInt("OHIP_AutoDialerEnabled", 0);
		if (bAutoDialEnabled) {

			// (j.jones 2009-03-10 14:33) - PLID 33418 - submit the claim to OHIP
			SetDlgItemText(IDC_CURR_EVENT, "Sending Claim to OHIP");
			bSentSuccessfully = SendClaimFileToOHIP(m_ExportName, m_nFilterByProviderID);
		}
		else {
			bSentSuccessfully = false; 
		}


		if (!bAutoDialEnabled) {
			str = "All claims were successfully exported. \n\n"
				"Do you want to transfer these claims to the unselected list?"; 
		}
		else if(bSentSuccessfully) {
			str = "All claims were successfully exported and have been submitted to OHIP.\n\n"
				"Do you want to transfer these claims to the unselected list?";
		}
		else {
			str = "All claims were successfully exported, but failed to send to OHIP.\n"
				"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.\n\n"
				"Do you want to transfer these claims to the unselected list?";
		}
		if (MessageBox(str,"Electronic Billing",MB_YESNO|MB_ICONINFORMATION) == IDYES) {
			// (j.jones 2008-05-06 16:54) - PLID 27453 - parameterized
			// (j.jones 2014-07-09 10:26) - PLID 62568 - do not move on hold claims
			ExecuteParamSql("UPDATE HCFATrackT SET CurrentSelect = 0 WHERE Batch = 2 AND BillID IN ("
				"SELECT BillsT.ID FROM BillsT "
				"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
				"WHERE Coalesce(BillStatusT.Type, -1) != {CONST_INT} "
				")", EBillStatusType::OnHold);
		}

		EndDialog(IDOK);

		return Success;

	} NxCatchAll("Error in Ebilling::ExportToOHIP");
	
	return Error_Other;
}

int CEbilling::OHIP_Batch_Header()
{
	try {

		m_OHIP_ClaimHeader1_Count = 0;
		m_OHIP_ClaimHeader2_Count = 0;
		m_OHIP_Item_Record_Count = 0;

		//Record Type: B

		//From OHIP: "The first record of each batch must be a Batch Header Record
		//"In multiple batch submissions, the first record of each subsequent batch
		//must always be a Batch Header Record."

		CString OutputString,str;

		//Field							Start	Length	Format (A=Alpha,N=Numeric,X=Alphanumeric,D=Date(YYYYMMDD),S=Spaces)

		//Transaction Identifier		1		2		A

		//Must be 'HE'

		OutputString = "HE";
		
		//Record Identification			3		1		A

		//Must be 'B'

		OutputString += "B";
		
		//Tech.Spec. Release
		//Identifier					4		3		X

		//Must be 'V03'

		OutputString += "V03";
		
		//MOH Office Code				7		1		A

		//listed in Appendix M

		//this is a code for the office they report to, apparently,
		//N is the main office in Toronto, Ontario, for example
		str = GetRemotePropertyText("OHIP_MOHOfficeCode", "N", 0, "<None>", true);

		OutputString += ParseField(str,1,'R',' ');
		
		//Batch Identification			8		12		N

		//must be in format YYYYMMDD####
		//first 8 digits are Creation Date (date the export file is created),
		//last 4 digits are a sequential number assigned by the health care provider

		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		str.Format("%s%s", dtNow.Format("%Y%m%d"), m_strBatchNumber);

		OutputString += ParseField(str,12,'R',' ');
		
		//Operator Number				20		6		N or S

		//only used on tape and cartridge submissions
		//zero-fill or space fill for disk submissions

		//we will never use this

		OutputString += ParseField("",6,'R',' ');
		
		//Group Number or
		//Laboratory Licence Number or
		//Independent Health Facility
		//Number						26		4		X

		//required field
		//group number assigned by Ministry of Health
		//must be 0000 for a solo Health Care Provider

		//for testing, we are permitted to use 0000

		str = GetRemotePropertyText("OHIP_GroupNumber", "0000", 0, "<None>", true);

		OutputString += ParseField(str,4,'R','0');

		//Health Care Provider /
		//Private Physio Facility /
		//Laboratory Director /
		//Independent Health Facility
		//Practitioner Number			30		6		N

		//must be present
		//provider number assigned by Ministry of Health

		//for testing, we are permitted to use 144436

		//use the provider's NPI number
		CString strNPI = "";
		CString strSpecialty = "08";
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		// (j.jones 2009-06-26 09:36) - PLID 34292 - Specialty is now in ProvidersT
		_RecordsetPtr rs = CreateParamRecordset("SELECT NPI, OHIPSpecialty FROM ProvidersT WHERE PersonID = {INT}", m_pEBillingInfo->ProviderID);
		if(!rs->eof) {
			strNPI = AdoFldString(rs, "NPI","");
			strSpecialty = AdoFldString(rs, "OHIPSpecialty","");
		}
		rs->Close();
		OutputString += ParseField(strNPI,6,'L',' ');

		//Specialty						36		2		N

		//must be a valid specialty code assigned by Ministry of Health
		//Refer to Appendix A - Health Care Provider Specialty Codes

		//02 - Dermatology
		//08 - Plastic Surgery

		OutputString += ParseField(strSpecialty,2,'R','0');

		//Reserved For MOH Use			38		42		S

		//must be spaces

		OutputString += ParseField("",42,'R',' ');

		OutputString += "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());

		return Success;

	}NxCatchAll("Error in Ebilling::OHIP_Batch_Header");

	return Error_Other;
}

int CEbilling::OHIP_Claim_Header1(CString strPlanName)
{
	try {

		//Record Type: H

		//From OHIP: "A Claim Header-1 Record must always follow each Batch Header Record.
		//A Claim Header-1 Record must always be present for each claim.

		CString OutputString,str;

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		// (j.jones 2009-02-11 10:35) - PLID 33035 - added ManualReview
		// (j.jones 2010-02-02 09:03) - PLID 33060 - pull the General 2 ref. phy. NPI in a separate field
		// provided that there is no ref. phy. selected on the bill
		_RecordsetPtr rs = CreateParamRecordset("SELECT "
			"PersonT.BirthDate, BillsT.HospFrom, LocationsT.NPI AS POSNPI, "
			"ReferringPhysT.NPI AS BillRefPhyNPI, "
			"CASE WHEN ReferringPhysT.PersonID Is Not Null THEN ReferringPhysT.NPI ELSE PatReferringPhysT.NPI END AS RefPhyNPI, "
			"BillsT.ManualReview "
			"FROM BillsT "
			"INNER JOIN PersonT ON BillsT.PatientID = PersonT.ID "			
			"INNER JOIN LocationsT ON BillsT.Location = LocationsT.ID "
			"LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN ReferringPhysT PatReferringPhysT ON PatientsT.DefaultReferringPhyID = PatReferringPhysT.PersonID "
			"WHERE BillsT.ID = {INT}", m_pEBillingInfo->BillID);

		if(rs->eof) {
			//if the recordset is empty, there is no bill. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open bill information for '%s', Bill ID %li. (OHIP_Claim_Header1)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open bill information. (OHIP_Claim_Header1)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		_variant_t var;

		//Field						Required?	Start	Length	Format (A=Alpha,N=Numeric,X=Alphanumeric,D=Date(YYYYMMDD),S=Spaces)
		//							(Mandatory,
		//							Optional,
		//							Conditional)

		//Transaction Identifier	M			1		2		A

		//Must be 'HE'

		OutputString = "HE";

		//Record Identification		M			3		1		A

		//Must be 'H'

		OutputString += "H";

		//Health Number				M			4		10		N or S

		//if absent, Claim Header-2 must exist
		//not required for RMB claims

		str = "";
		long nHealthNumberCustomField = GetRemotePropertyInt("OHIP_HealthNumberCustomField", 1, 0, "<None>", true);
		//this field will correspond to a patient's custom text field
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT "
			"WHERE FieldID = {INT} AND PersonID = {INT}", nHealthNumberCustomField, m_pEBillingInfo->PatientID);
		if(!rsCustom->eof) {
			str = AdoFldString(rsCustom, "TextParam","");
			// (j.jones 2008-12-02 16:28) - PLID 32299 - strip hyphens and spaces only
			str.Replace("-","");
			str.Replace(" ","");
		}
		rsCustom->Close();
		
		OutputString += ParseField(str,10,'R','0');

		//Version Code				M			14		2		A

		//can be 1 or 2 characters
		//a 1-character code may be left or right justified

		str = "";
		long nVersionCodeCustomField = GetRemotePropertyInt("OHIP_VersionCodeCustomField", 2, 0, "<None>", true);
		//this field will correspond to a patient's custom text field
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT "
			"WHERE FieldID = {INT} AND PersonID = {INT}", nVersionCodeCustomField, m_pEBillingInfo->PatientID);
		if(!rsCustom->eof) {
			str = AdoFldString(rsCustom, "TextParam","");
		}
		rsCustom->Close();

		OutputString += ParseField(str,2,'L',' ');

		//Patient Birthdate			M			16		8		D

		str = "";
		var = rs->Fields->Item["BirthDate"]->Value;
		if(var.vt == VT_DATE) {
			 COleDateTime dt = var.date;
			 str = dt.Format("%Y%m%d");
		}

		OutputString += ParseField(str,8,'L',' ');

		//Accounting Number			O			24		8		X

		//this is the Bill ID, just a reference for the office for
		//when they get correspondence back
		str.Format("%li", m_pEBillingInfo->BillID);
		OutputString += ParseField(str,8,'L',' ');
		
		//available for use by the provider for claim identification

		//Payment Program			M			32		3		A
		
		//must be one of:
		//HCP (Health Claims Payment)
		//WCB (Workers' Compensation Board)
		//RMB (Reciprocal Medical Billings)

		//this pulls from the insured party's plan name
		//if invalid, or none selected, then assume HCP

		//it is passed in from the calling function
		str = strPlanName;
		OutputString += ParseField(str,3,'L',' ');

		//Payee						M			35		1		A

		//must be one of:
		//P (Provider)
		//S (Patient)

		//will always be provider

		OutputString += ParseField("P",1,'L',' ');

		//Referring /
		//Requisitioning Health
		//Care Provider Number		C			36		6		N

		//Referring Physician NPI
		// (j.jones 2010-02-02 09:16) - PLID 33060 - added option to send G2 ref. phy. in a claim
		// if the bill has no ref. phy. selected
		// (d.thompson 2010-03-23) - PLID 37850 - m.clark set this to enabled by default
		if(GetRemotePropertyInt("OHIPUseG2RefPhy", 1, 0, "<None>", true) == 1) {
			//this field shows the Bill Ref. Phy. NPI if a ref. phy. is selected (the NPI may be blank),
			//and the G2 Ref. Phy. NPI if no ref. phy. is selected on the bill
			str = AdoFldString(rs, "RefPhyNPI","");
		}
		else {
			//this is always the bill's ref. phy. NPI even if no ref. phy. is selected
			str = AdoFldString(rs, "BillRefPhyNPI","");
		}

		OutputString += ParseField(str,6,'L',' ');

		//Facility Number			C			42		4		X

		//for testing, we are permitted to use 9999

		//Place Of Service NPI
		str = AdoFldString(rs, "POSNPI","");

		OutputString += ParseField(str,4,'L',' ');

		//Inpatient Admission Date	C			46		8		D
		
		//if present, must be same or prior to service date

		str = "";
		var = rs->Fields->Item["HospFrom"]->Value;
		if(var.vt == VT_DATE) {
			 COleDateTime dt = var.date;
			 str = dt.Format("%Y%m%d");
		}

		OutputString += ParseField(str,8,'L',' ');

		//Referring Laboratory
		//Licence Number			C			54		4		N

		//we do not fill this
		
		OutputString += ParseField("",4,'L',' ');

		//Manual Review Indicator	C			58		1		A

		//must be blank or Y

		// (j.jones 2009-02-11 10:35) - PLID 33035 - we added a Manual Review checkbox on the bill for this
		str = "";
		if(AdoFldBool(rs, "ManualReview", FALSE)) {
			str = "Y";
		}
		OutputString += ParseField(str,1,'L',' ');

		//Location Code				C			59		4		N or S

		//Bill Location NPI

		str = "";
		// (j.jones 2008-05-06 15:19) - PLID 29937 - now the NPI is in the loaded claim pointer
		str = m_pEBillingInfo->strBillLocationNPI;
		OutputString += ParseField(str,4,'L',' ');

		//Reserved For OOC			M			63		11		S

		//must be spaces

		OutputString += ParseField("",11,'L',' ');

		//Reserved For MOH Use		M			74		6		S

		//must be spaces

		OutputString += ParseField("",6,'L',' ');

		rs->Close();

		m_OHIP_ClaimHeader1_Count++;

		OutputString += "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());

		return Success;

	}NxCatchAll("Error in Ebilling::OHIP_Claim_Header1");

	return Error_Other;
}

int CEbilling::OHIP_Claim_Header2()
{
	try {

		// (j.jones 2006-12-08 11:09) - PLID 21570 - used only if the plan name is RMB

		//Record Type: R

		//From OHIP: "A Claim Header-2 Record is required only for Reciprocal claims.
		//If required, a Claim Header-2 Record must follow the Claim Header-1 Record.

		CString OutputString,str;

		//Field						Required?	Start	Length	Format (A=Alpha,N=Numeric,X=Alphanumeric,D=Date(YYYYMMDD),S=Spaces)
		//							(Mandatory,
		//							Optional,
		//							Conditional)

		//Transaction Identifier	M			1		2		A

		//Must be 'HE'

		OutputString = "HE";

		//Record Identification		M			3		1		A

		//Must be 'R'

		OutputString += "R";

		//Registration Number		M			4		12		X
		
		//any number less than 12 digits must be left justified and space filled

		str = "";
		long nRegistrationNumberCustomField = GetRemotePropertyInt("OHIP_RegistrationNumberCustomField", 3, 0, "<None>", true);
		//this field will correspond to a patient's custom text field
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT "
			"WHERE FieldID = {INT} AND PersonID = {INT}", nRegistrationNumberCustomField, m_pEBillingInfo->PatientID);
		if(!rsCustom->eof) {
			str = AdoFldString(rsCustom, "TextParam","");
		}
		rsCustom->Close();

		OutputString += ParseField(str,12,'L',' ');

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsPatients = CreateParamRecordset("SELECT Last, First, Gender, State FROM PersonT WHERE ID = {INT}", m_pEBillingInfo->PatientID);

		//Patient Last Name			M			16		9		A

		//must be left justified
		//any non alpha, non-numeric characters must be removed,
		//this means spaces too

		str = "";
		if(!rsPatients->eof) {
			str = AdoFldString(rsPatients, "Last","");
			str = StripPunctuation(str, FALSE);
			str.Replace(" ","");
		}
		OutputString += ParseField(str,9,'L',' ');

		//Patient First Name		M			25		5		A

		//must be left justified
		//any non alpha, non-numberic characters must be removed,
		//this means spaces too

		str = "";
		if(!rsPatients->eof) {
			str = AdoFldString(rsPatients, "First","");
			str = StripPunctuation(str, FALSE);
			str.Replace(" ","");
		}
		OutputString += ParseField(str,5,'L',' ');

		//Patient Sex				M			30		1		N

		//Male = 1
		//Female = 2

		str = "";
		if(!rsPatients->eof) {
			if(AdoFldByte(rsPatients, "Gender", 0) == 1) {
				str = "1";
			}
			else if(AdoFldByte(rsPatients, "Gender", 0) == 2) {
				str = "2";
			}
		}
		OutputString += ParseField(str,1,'L',' ');

		//Province Code				M			31		2		A

		str = "";
		if(!rsPatients->eof) {
			str = AdoFldString(rsPatients, "State","");
			str.MakeUpper();
		}
		OutputString += ParseField(str,2,'L',' ');

		rsPatients->Close();

		//Reserved For MOH Use		M			33		47		S

		//must be spaces

		OutputString += ParseField("",47,'L',' ');

		m_OHIP_ClaimHeader2_Count++;

		OutputString += "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());

		return Success;

	}NxCatchAll("Error in Ebilling::OHIP_Claim_Header2");

	return Error_Other;
}

int CEbilling::OHIP_Item_Record(_RecordsetPtr &rsCharges)
{
	try {

		//Record Type: T

		//From OHIP: "An option of having two items per Item Record has been provided and may be utilized.

		CString OutputString,str;

		//Field						Required?	Start	Length	Format (A=Alpha,N=Numeric,X=Alphanumeric,D=Date(YYYYMMDD),S=Spaces)
		//							(Mandatory,
		//							Optional,
		//							Conditional)

		//Transaction Identifier	M			1		2		A

		//Must be 'HE'

		OutputString = "HE";

		//Record Identification		M			3		1		A

		//Must be 'T'

		OutputString += "T";

		// (j.jones 2006-11-07 17:01) - strangely, the allow two items per record,
		// and if a second item doesn't exist, it must be space filled to keep the
		// same total width

		//================================
		//Item 1
		//================================

		//Service Code				M			4		5		X

		//must be present in the format 'ANNNA'
		//Prefix is alpha, except I, O, and U
		//'NNN' must be numeric
		//Suffix must be A, B, or C

		str = AdoFldString(rsCharges, "ItemCode","");
		// (j.jones 2008-12-05 15:34) - PLID 32356 - ensure we send this as uppercase
		str.MakeUpper();

		OutputString += ParseField(str,5,'L',' ');

		//Reserved For MOH Use		M			9		2		S

		//must be spaces

		OutputString += ParseField("",2,'L',' ');

		//Fee Submitted				M			11		6		N

		//is the multiple of charge * quantity
		//no decimals, so $100.00 = 10000, or $123.45 = 12345

		COleCurrency cy = AdoFldCurrency(rsCharges, "LineTotal",COleCurrency(0,0));
		str = FormatCurrencyForInterface(cy,FALSE,FALSE);
		str = StripNonNum(str);
		
		OutputString += ParseField(str,6,'R','0');

		//Number of Services		M			17		2		N

		//the quantity, right justified, 0 filled
		//ie. 01, 02, 12, 15, etc.

		double dblQty = AdoFldDouble(rsCharges, "Quantity",0.0);
		//has to be rounded
		long nQty = (long)dblQty;
		str = AsString(nQty);
		
		OutputString += ParseField(str,2,'R','0');

		//Service Date				M			19		8		D

		//formatted as YYYYMMDD
		//cannot be greater than file creation date
		//cannot be more than 6 months old

		_variant_t var = rsCharges->Fields->Item["ServiceDateFrom"]->Value;
		if(var.vt == VT_DATE) {
			 COleDateTime dt = var.date;
			 str = dt.Format("%Y%m%d");
		}
		else
			str = "";		

		OutputString += ParseField(str,8,'L',' ');

		//Diagnostic Code			C			27		4		X

		//if only 3 characters, left justify, space fill

		// (a.walling 2014-03-19 10:49) - PLID 61420 - EBilling - OHIP - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(0).number;
		str.Replace(".","");
		if(str.GetLength() > 4)
			str = str.Left(4);

		OutputString += ParseField(str,4,'L',' ');

		//Reserved For OOC			M			31		10		S
		
		//must be spaces

		OutputString += ParseField("",10,'L',' ');

		//Reserved For MOH Use		M			41		1		S

		//must be spaces

		OutputString += ParseField("",1,'L',' ');

		//================================
		//Item 2   (Optional)
		//================================

		if(!rsCharges->eof)
			rsCharges->MoveNext();

		// (j.jones 2006-11-07 17:02) - remember, if item 2 is blank,
		// then fill with spaces

		//Service Code				M			42		5		X

		//must be present in the format 'ANNNA'
		//Prefix is alpha, except I, O, and U
		//'NNN' must be numeric
		//Suffix must be A, B, or C

		if(!rsCharges->eof)
			str = AdoFldString(rsCharges, "ItemCode","");
		else
			str = "";

		// (j.jones 2008-12-05 15:34) - PLID 32356 - ensure we send this as uppercase
		str.MakeUpper();

		OutputString += ParseField(str,5,'L',' ');

		//Reserved For MOH Use		M			47		2		S

		//must be spaces

		OutputString += ParseField("",2,'L',' ');

		//Fee Submitted				M			49		6		N

		//is the multiple of charge * quantity
		//no decimals, so $100.00 = 10000, or $123.45 = 12345
		
		if(!rsCharges->eof) {
			COleCurrency cy = AdoFldCurrency(rsCharges, "LineTotal",COleCurrency(0,0));
			str = FormatCurrencyForInterface(cy,FALSE,FALSE);
			str = StripNonNum(str);
			OutputString += ParseField(str,6,'R','0');
		}
		else {
			//if empty, do not send zeros
			str = "";		
			OutputString += ParseField(str,6,'R',' ');
		}

		//Number of Services		M			55		2		N

		//the quantity, right justified, 0 filled
		//ie. 01, 02, 12, 15, etc.

		if(!rsCharges->eof) {
			double dblQty = AdoFldDouble(rsCharges, "Quantity",0.0);
			//has to be rounded
			long nQty = (long)dblQty;
			str = AsString(nQty);
			OutputString += ParseField(str,2,'R','0');
		}
		else {
			//if empty, do not send zeros
			str = "";
			OutputString += ParseField(str,2,'R',' ');
		}

		//Service Date				M			57		8		D

		//formatted as YYYYMMDD
		//cannot be greater than file creation date
		//cannot be more than 6 months old

		str = "";
		if(!rsCharges->eof) {
			_variant_t var = rsCharges->Fields->Item["ServiceDateFrom"]->Value;
			if(var.vt == VT_DATE) {
				 COleDateTime dt = var.date;
				 str = dt.Format("%Y%m%d");
			}
		}

		OutputString += ParseField(str,8,'L',' ');

		//Diagnostic Code			C			65		4		X

		//if only 3 characters, left justify, space fill

		if(!rsCharges->eof) {
			// (a.walling 2014-03-19 10:49) - PLID 61420 - EBilling - OHIP - Get diag info from bill
			str = m_pEBillingInfo->GetSafeBillDiag(0).number;
			str.Replace(".","");
			if(str.GetLength() > 4)
				str = str.Left(4);
		}
		else
			str = "";

		OutputString += ParseField(str,4,'L',' ');

		//Reserved For OOC			M			69		10		S
		
		//must be spaces

		OutputString += ParseField("",10,'L',' ');

		//Reserved For MOH Use		M			79		1		S

		//must be spaces

		OutputString += ParseField("",1,'L',' ');
		
		m_OHIP_Item_Record_Count++;

		OutputString += "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());

		return Success;

	}NxCatchAll("Error in Ebilling::OHIP_Item_Record");

	return Error_Other;
}

int CEbilling::OHIP_Batch_Trailer()
{
	try {

		//Record Type: E

		//From OHIP: "A Batch Trailer Record must be present at the end of every batch and contain the
		//appropriate counts of the number of Claim Header-1 Record (H), Claim Header-2 Records (R),
		//and Item Records (T).

		CString OutputString,str;

		//Field						Start	Length	Format (A=Alpha,N=Numeric,X=Alphanumeric,D=Date(YYYYMMDD),S=Spaces)

		//Transaction Identifier	1		2		A

		//Must be 'HE'

		OutputString = "HE";

		//Record Identification		3		1		A

		//Must be 'E'

		OutputString += "E";

		//H Count					4		4		N

		//total H records in the batch
		//right justified with leading zeros

		str.Format("%li", m_OHIP_ClaimHeader1_Count);
		OutputString += ParseField(str,4,'R','0');

		//R Count					8		4		N

		//total R records in the batch
		//right justified with leading zeros

		str.Format("%li", m_OHIP_ClaimHeader2_Count);
		OutputString += ParseField(str,4,'R','0');

		//T Count					12		5		N

		//total T records in the batch
		//right justified with leading zeros

		str.Format("%li", m_OHIP_Item_Record_Count);
		OutputString += ParseField(str,5,'R','0');

		//Reserved For MOH Use		17		63		S

		//must be spaces

		OutputString += ParseField("",63,'L',' ');

		OutputString += "\r\n";
		TRACE(OutputString);
		m_OutputFile.Write(OutputString,OutputString.GetLength());

		return Success;

	}NxCatchAll("Error in Ebilling::OHIP_Batch_Trailer");

	return Error_Other;
}

// (j.jones 2008-05-02 09:57) - PLID 27478 - simple CStringArray search utility
BOOL CEbilling::IsQualifierInArray(CStringArray &arystrQualifiers, CString strQualifier)
{
	//let the caller catch any exceptions

	for(int i=0; i<arystrQualifiers.GetSize(); i++) {
		if(arystrQualifiers.GetAt(i).CompareNoCase(strQualifier) == 0) {
			return TRUE;
		}
	}

	return FALSE;
}

void CEbilling::EndANSISegment(CString OutputString) {

	OutputString.TrimRight("*");

	//this is so we don't output segments with no data, such as "N3~"
	if(OutputString.Find("*") == -1)
		return;

	OutputString += "~";
	// (j.gruber 2009-04-24 12:44) - PLID 34073 - don't format anything here
	TRACE0(OutputString);
	m_OutputFile.Write(OutputString,OutputString.GetLength());

	//increment the count of segments
	m_ANSISegmentCount++;

	// (b.spivey, August 27th, 2014) - PLID 63492 - use the variable for the debug check
	if (m_bHumanReadableFormat) { 

		CString str;

		str = "\r\n";
		m_OutputFile.Write(str, str.GetLength());
	}

}

//returns a string value from the field in the given recordset
//Parameters:
//		_RecordsetPtr rs	-	An opened recordset containing the field in argument 2
//		CString strField	-	The name of a field that exists in the recordset
//Return value:
//		A CString that is the value contained in the field
//		A blank string, if the field does not exist or is empty/null
CString CEbilling::GetFieldFromRecordset(_RecordsetPtr rs, CString strField) {

	_variant_t var;
	CString str;

	var = rs->Fields->Item[(_variant_t)strField]->Value;

	if(var.vt == VT_BSTR)
		str = CString(var.bstrVal);
	else
		str = "";

	str.TrimRight(" ");

	return str;
}

// (j.jones 2010-04-14 09:19) - PLID 38194 - moved Box33PinANSI to GlobalFinancialUtils

// (j.jones 2010-10-20 15:42) - PLID 40936 - moved Box17aANSI to GlobalFinancialUtils

// (j.jones 2010-04-14 09:19) - PLID 38194 - moved Box8283NumANSI to GlobalFinancialUtils

void CEbilling::OutputHCFAImageAnesthesiaTimes(long nBillID, BOOL bOnlyAnesthesiaMinutes)
{
	CString str = "\r\n", strStart, strEnd;
	strStart = GetAnesthStartTimeFromBill(nBillID);
	strEnd = GetAnesthEndTimeFromBill(nBillID);
	long nMinutes = GetAnesthMinutesFromBill(nBillID);

	if(bOnlyAnesthesiaMinutes && nMinutes != -1) {
		//just the minutes, not the times
		str.Format("                              %li\r\n", nMinutes);
	}
	else if(!strStart.IsEmpty() && !strEnd.IsEmpty()) {
		str.Format("                              %s - %s\r\n", strStart, strEnd);
	}

	TRACE(str);
	m_OutputFile.Write(str,str.GetLength());
}

// (j.jones 2008-11-12 12:33) - PLID 31740 - Take in info for a CAS adjusment segment,
// and store it in memory. If we get a duplicate group & reason code, add the dollar amount
// to the matching memory object.
// (j.jones 2009-08-28 16:55) - PLID 35006 - added a boolean to allow zero
void CEbilling::AddCASSegmentToMemory(CString strGroupCode, CString strReasonCode, COleCurrency cyAmount, CArray<ANSI_CAS_Info*, ANSI_CAS_Info*> &aryCASSegments, BOOL bAllowZeroAmt /*= FALSE*/)
{	
	//throw any exceptions to the caller

	// (j.jones 2009-08-28 16:55) - PLID 35006 - a zero amount is allowed only if
	// we received the override specifically stating it is allowed
	if(strGroupCode.IsEmpty() || strReasonCode.IsEmpty()
		|| (cyAmount == COleCurrency(0,0) && !bAllowZeroAmt)) {
		ASSERT(FALSE);
		return;
	}

	//The CAS segment has a Group Code, a Reason Code, an Amount, and a Quantity field (unused).
	//Group Codes cannot be duplicated. Reason Codes cannot be duplicated per Group.
	//A CAS segment can have one Group Code, then up to 6 instances of Reason Codes, Amount, & Quantity.
	
	//This function needs to build up our CAS segments based on the passed-in data, updating
	//amounts when duplicates occur. In the event that we have more than 6 reasons in a group,
	//we have no choice but to duplicate the group. This is highly unlikely to ever occur.

	int i=0, j=0;

	//first see if our group code is in our array
	for(i=0; i<aryCASSegments.GetSize(); i++) {
		ANSI_CAS_Info* pInfo = (ANSI_CAS_Info*)aryCASSegments.GetAt(i);
		if(pInfo->strGroupCode == strGroupCode) {
			//it exists, so lets see if our reason code exists
			for(j=0; j<pInfo->aryDetails.GetSize(); j++) {
				ANSI_CAS_Detail* pDetail = (ANSI_CAS_Detail*)pInfo->aryDetails.GetAt(j);
				if(pDetail->strReasonCode == strReasonCode) {
					//it exists, so add our amount into it
					pDetail->cyAmount += cyAmount;
					return;
				}
			}

			//if we get here, we could not find the reason code,
			//so add a new one UNLESS we already have 6 of them
			if(j < 6) {
				ANSI_CAS_Detail* pNewDetail = new ANSI_CAS_Detail;
				pNewDetail->strReasonCode = strReasonCode;
				pNewDetail->cyAmount = cyAmount;
				pInfo->aryDetails.Add(pNewDetail);
				return;
			}

			//if we already had 6 reasons, we'll go to the next group code in the list
		}
	}

	//if we get here, we could not find a group code to add to,
	//so we must add an all-new record
	ANSI_CAS_Detail* pNewDetail = new ANSI_CAS_Detail;
	pNewDetail->strReasonCode = strReasonCode;
	pNewDetail->cyAmount = cyAmount;
	ANSI_CAS_Info* pNewInfo = new ANSI_CAS_Info;
	pNewInfo->strGroupCode = strGroupCode;
	pNewInfo->aryDetails.Add(pNewDetail);
	aryCASSegments.Add(pNewInfo);
	return;
}

// (j.jones 2010-07-12 11:14) - PLID 29971 - supported Alberta HLINK
int CEbilling::ExportToAlbertaHLINK()
{
	try {

		m_nAlbertaTotalTransactionCounts = 0;
		m_nAlbertaCurrentSegmentCount = 0;
		m_nAlbertaTotalSegmentCounts = 0;

		SetDlgItemText(IDC_CURR_EVENT,"Exporting to Disk");

		_RecordsetPtr rsBills;

		//open the file for writing
		if(!m_OutputFile.Open(m_ExportName,CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
			// (j.armen 2011-10-25 13:42) - PLID 46134 - Ebilling is located in the practice path
			// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure. Also, changed to FileUtils::CreatePath() so that 
			// non-existent intermediate paths are created.
			FileUtils::CreatePath(FileUtils::GetFilePath(m_ExportName));
			if(!m_OutputFile.Open(m_ExportName,CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
				AfxMessageBox("The ebilling export file could not be created. Contact Nextech for assistance.");
				return Error_Other;
			}
		};

		//update the batch number

		// update LastBatchID
		int tempint = 0;
		tempint = GetRemotePropertyInt(_T("LastBatchID"),0,0,_T("<None>"));
		if (tempint==0) {
			SetRemotePropertyInt(_T("LastBatchID"),1,0,_T("<None>"));
			m_BatchID = 1;
		}
		else { // record does exist
			m_BatchID = tempint+1;

			//reset after 999,999
			if(m_BatchID > 999999) {
				m_BatchID = 1;
			}

			SetRemotePropertyInt(_T("LastBatchID"),m_BatchID,0,_T("<None>"));
		}  // end of LastBatchID update

		m_strBatchNumber.Format("%li",m_BatchID);

		// (j.dinatale 2013-01-21 11:50) - PLID 54733 - get the last transaction ID
		long nLastTransactionID = GetRemotePropertyInt("Alberta_LastTransactionID", -1, 0, "<None>", true);

		//there is only one header and one trailer
		RETURN_ON_FAIL(Alberta_Header());


		// (j.jones 2011-07-13 10:44) - PLID 44542 - the Alberta export updates claim history
		// first, instead of last, because we need to export claim history IDs for the purposes
		// of having unique identifiers per export
		// (j.jones 2015-11-19 14:10) - PLID 67600 - updating the claim history is now called before the export function

		for(int i=0;i<m_aryEBillingInfo.GetSize();i++) {

			CString str;
			str.Format("Exporting to Disk - Claim %li of %li",i+1,m_aryEBillingInfo.GetSize());
			SetDlgItemText(IDC_CURR_EVENT,str);
			
			m_pEBillingInfo = ((EBillingInfo*)m_aryEBillingInfo.GetAt(i));
			
			// (a.walling 2014-03-19 10:05) - PLID 61346 - Load all diag codes for the bill
			m_pEBillingInfo->billDiags = GetBillDiagCodes(m_pEBillingInfo->BillID, ShouldUseICD10(), false, NULL);

			TRACE("Claim ID=%d\r\n", m_pEBillingInfo->HCFAID);
			TRACE("==========================================================\r\n");

			//find out the plan name, it must be HCP, WCB, or RMB
			//if empty or invalid, assume HCP
			CString strPlanName = "HCP";
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT PlanName FROM InsurancePlansT "
			"WHERE ID IN (SELECT InsPlan FROM InsuredPartyT WHERE PersonID = {INT})", m_pEBillingInfo->InsuredPartyID);
			if(!rs->eof) {
				CString strPlan = AdoFldString(rs, "PlanName","");
				strPlan.TrimLeft();
				strPlan.TrimRight();
				strPlan.MakeUpper();
				if(strPlan == "WCB" || strPlan == "RMB")
					strPlanName = strPlan;
			}
			rs->Close();

			// (j.jones 2011-07-25 09:34) - PLID 44662 - If the user exports a claim with an action
			// code of Change / Reassess / Delete, they have to pick a previous ClaimHistoryDetailID.
			// However, a "claim" is per charge, as is that prior selection. If they pick a prior export,
			// store that selection so they don't have to pick it again for other charges on the same bill.
			long nNewClaimHistoryDetailID = -1;
			BOOL bHasChosenNewSequenceNumber = FALSE;

			CString strSegmentToUse;

			//Alberta segment types:

			//CIB1 - In-Province Service Provider Base Claim segment (1 only).
			//		This is the first segment type. A "CIB1" segment must always be present
			//		on a Claim with an 'A' (Add) Action since it provides the base data
			//		for claims by in-province Service Providers and thus forms the basis of
			//		a transaction. Only one "CIB1" segment is allowed per transaction.
			//		Other segment types are conditional for Add transactions.

			//CPD1 - Claim Person Data segments (multiple as needed).
			//		The second segment type "CPD1", in addition to the mandatory first segment
			//		"CIB1" for an Add, is required for each person referenced on a claim where:
			//		-	the Service Recipient (person receiving a service from a Service Provider)
			//			does not have a Unique Lifetime Identifier (ULI) and;
			//		-	a Payee not already identified by the Pay to ULI, where the Pay to Code = OTHR.
			//		-	an out of province Referring Service Provider
			//		This segment is necessary to document the name, address, and other pertinent
			//		information so processing may be completed.

			//CST1 - Claim Supporting Text segments (up to 500).
			//		The third segment "CST1" is used when supporting text is required for a claim.
			//		Up to 500 "CST1" segments can be used.

			//CTX1 - Supporting Text cross-reference segment (as required).
			//		The fourth segment "CTX1" is used where the supporting text in segments "CST1"
			//		above is also used for other claims. This segment can include up to fourteen (14)
			//		Claim Numbers that use the same supporting text. Only one (1) "CTX1" segment can
			//		be used per transaction.

			//begin exporting the transactions
			// (j.jones 2011-06-27 16:21) - PLID 44335 - now we always filter by our provider ID
			// (j.jones 2011-07-13 13:43) - PLID 44542 - we now load the ClaimHistoryDetailsT.ID as a Sequence Number
			// (j.jones 2011-07-20 09:23) - PLID 44637 - use the ISO3Code for the CountryCode
			// (j.jones 2011-07-21 12:13) - PLID 44662 - load up the ClaimTypeCode as the ActionCode
			// (j.jones 2011-07-22 10:26) - PLID 44662 - added HCFABlock19
			// (j.jones 2011-08-05 11:57) - PLID 44898 - added the supervising provider
			// (j.jones 2011-07-13 17:51) - PLID 44558 - now we always filter by our provider ID
			// (j.jones 2011-07-13 18:04) - PLID 44553 - we now load the ClaimHistoryDetailsT.ID as a Sequence Number
			// (j.jones 2011-07-21 13:27) - PLID 44638 - use the ISO3Code for the CountryCode
			// (j.jones 2011-07-21 13:25) - PLID 44663 - load up the ClaimTypeCode as the ActionCode
			// (j.jones 2011-07-25 10:50) - PLID 44663 - added HCFABlock19
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			// (j.jones 2011-09-20 09:04) - PLID 44934 - switched HCFABlock19 usage to use billing notes
			// (j.jones 2011-10-25 17:29) - PLID 46088 - added Calls
			// (j.jones 2012-03-21 12:54) - PLID 49084 - added the POS Code from the bill, which we use as the Functional Centre
			// (d.singleton 2012-05-22 09:53) - PLID 50700 added skill code alberta only,  use taxonomycode if no skill code 
			// (j.gruber 2014-03-17 10:11) - PLID 61392 - update for new billing structure
			// (a.walling 2014-03-19 10:49) - PLID 61420 - EBilling - Alberta - DiagCodes and WhichCodes must be unique and filtered for NULLs. Use the BillDiags
			_RecordsetPtr rsCharges = CreateParamRecordset("SELECT ChargesT.ID, "
				//we need to report how many bills were performed by the provider for a patient on a single day,
				//so we're counting all bills that have charges with the same service date, for the same
				//patient & provider, that are have a lesser BillID than the one we are exporting right now
				"(SELECT Count(BillsQ.ID) + 1 FROM BillsT AS BillsQ "
				"	WHERE BillsQ.ID < BillsT.ID AND BillsQ.Deleted = 0 "
				"	AND BillsQ.PatientID = BillsT.PatientID "
				"	AND BillsQ.ID IN ("
				"		SELECT BillID FROM ChargesT AS ChargesQ "
				"		INNER JOIN LineItemT AS LineItemQ ON ChargesQ.ID = LineItemQ.ID "
				"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"		WHERE LineItemQ.Deleted = 0 "
				"		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"		AND LineItemQ.Date = LineItemT.Date "
				"		AND ChargesQ.DoctorsProviders = ChargesT.DoctorsProviders "
				"	)"
				") AS DailyBillIndex, "
				"ChargesT.ItemCode, dbo.GetChargeTotal(ChargesT.ID) AS LineTotal, "
				"ChargesT.Quantity, ChargesT.ServiceDateFrom, ChargesT.ServiceDateTo, "
				"BillsT.SupervisingProviderID, "
				"BillsT.HospFrom, "
				"ChargesT.CPTModifier, ChargesT.CPTModifier2, ChargesT.CPTModifier3, "
				"PersonT.Last, PersonT.First, PersonT.Middle, "
				"PersonT.BirthDate, PersonT.Gender, "
				"PersonT.Address1, PersonT.Address2, "
				"PersonT.City, PersonT.State, PersonT.Zip, CountriesT.ISO3Code AS CountryCode, "
				"ProvidersT.NPI AS ProviderNPI, ProvidersT.TaxonomyCode AS ProviderTaxonomyCode, "
				"ProviderPersonT.Last AS ProviderLast, ProviderPersonT.First AS ProviderFirst, "
				"BillsT.SendCorrespondence, "
				"PlaceOfServiceT.NPI AS PlaceOfServiceNPI, "
				"ReferringPhysT.NPI AS RefPhyNPI, "
				"InsurancePlansT.PlanName, "
				"ClaimHistoryDetailsQ.CurrentClaimHistoryDetailID, "
				"BillsT.ANSI_ClaimTypeCode AS ActionCode, LineNotesQ.LineNote, "
				"ChargesT.Calls, CASE WHEN COALESCE(ChargesT.SkillCode, '') = '' THEN ProvidersT.TaxonomyCode ELSE ChargesT.SkillCode END AS SkillCode "				
				"FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
				"INNER JOIN PersonT ON BillsT.PatientID = PersonT.ID "
				//the ebilling info object calculated a provider ID that uses the claim provider calculations, so just join that provider
				"CROSS JOIN (SELECT * FROM ProvidersT WHERE PersonID = {INT}) AS ProvidersT "
				"LEFT JOIN PersonT ProviderPersonT ON ProvidersT.PersonID = ProviderPersonT.ID "
				"LEFT JOIN CountriesT ON PersonT.Country = CountriesT.CountryName "
				"LEFT JOIN LocationsT AS PlaceOfServiceT ON BillsT.Location = PlaceOfServiceT.ID "
				"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				"LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
				"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
				"LEFT JOIN (SELECT Max(ClaimHistoryDetailsT.ID) AS CurrentClaimHistoryDetailID, ClaimHistoryDetailsT.ChargeID "
				"	FROM ClaimHistoryDetailsT "
				"	GROUP BY ClaimHistoryDetailsT.ChargeID "
				"	) AS ClaimHistoryDetailsQ ON ChargesT.ID = ClaimHistoryDetailsQ.ChargeID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"LEFT JOIN (SELECT Min(Note) AS LineNote, LineItemID FROM Notes "
					"	WHERE LineItemID Is Not Null AND SendOnClaim = 1 "
					"	GROUP BY LineItemID) AS LineNotesQ ON ChargesT.ID = LineNotesQ.LineItemID "
				"WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = {INT} AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ChargesT.DoctorsProviders END) = {INT} "
				"ORDER BY ChargesT.LineID ", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID, m_pEBillingInfo->ProviderID);
			while(!rsCharges->eof) {

				long nChargeID = AdoFldLong(rsCharges, "ID");

				// (j.jones 2011-07-21 12:28) - PLID 44662 - we use the ANSI_ClaimTypeCode as the ActionCode,
				// which might end up changing our Sequence Number to use a prior ClaimHistoryDetailID
				ANSI_ClaimTypeCode eActionCode = (ANSI_ClaimTypeCode)AdoFldLong(rsCharges, "ActionCode", (long)ctcOriginal);

				//this should never be NULL, if it was, our ClaimHistory update failed
				long nCurrentClaimHistoryDetailID = AdoFldLong(rsCharges, "CurrentClaimHistoryDetailID");
				CString strAlbertaTransactionNumber = "";	// (j.dinatale 2013-01-18 16:57) - PLID 54419 - need to keep track of this now

				//if bHasChosenNewSequenceNumber is true, then the user already picked a previous export
				//for another charge on this bill, and should not have to pick one again, if this charge
				//has been exported before
				if(bHasChosenNewSequenceNumber && nNewClaimHistoryDetailID != -1) {

					//get the ClaimHistoryDetailID for this charge, from the same
					//export as nNewClaimHistoryDetailID

					_RecordsetPtr rsClaimHistory = CreateParamRecordset("SELECT ClaimHistoryDetailsT.ID, AlbertaTransNum "
						"FROM ClaimHistoryT "
						"INNER JOIN ClaimHistoryDetailsT ON ClaimHistoryT.ID = ClaimHistoryDetailsT.ClaimHistoryID "
						"WHERE ClaimHistoryT.BillID = {INT} AND ClaimHistoryDetailsT.ChargeID = {INT} "
						"AND ClaimHistoryT.ID IN (SELECT ClaimHistoryID FROM ClaimHistoryDetailsT WHERE ID = {INT})",
						m_pEBillingInfo->BillID, nChargeID, nNewClaimHistoryDetailID);
					if(!rsClaimHistory->eof) {
						nCurrentClaimHistoryDetailID = AdoFldLong(rsClaimHistory, "ID");
						//track this ID as the new ID as well, for posterity
						nNewClaimHistoryDetailID = nCurrentClaimHistoryDetailID;

						// (j.dinatale 2013-01-18 16:57) - PLID 54419 - collect the previous transaction number
						strAlbertaTransactionNumber = AdoFldString(rsClaimHistory, "AlbertaTransNum", "");
					}
					else {
						//this would only happen if this charge was not exported at the same time
						//the previously selected charge was, so we have to reset these values
						//such that they get prompted again for selecting the prior submission
						bHasChosenNewSequenceNumber = FALSE;
						nNewClaimHistoryDetailID = -1;
					}
					rsClaimHistory->Close();
				}

				CString strSequenceNumber;

				//The sequence number needs to be unique per transaction per calendar year
				//and less than 7 digits. So far, it appears to mean that it's unique per
				//transaction, changes use the name number, and the calendar year part
				//just means it is allowed to roll over.

				// (j.jones 2011-07-13 13:46) - PLID 44590 - SequenceNumber is now
				// the most recent ClaimHistoryDetailsT.ID for this charge, and Claim History
				// now updates prior to export for Alberta claims. This ID is unique, but
				// use only the right-most 7 numbers if they have over 10 million charges.

				// (j.jones 2011-07-21 12:29) - PLID 44662 - If the Action Code is not "Add",
				// the Sequence Number will actually be a previous ClaimHistoryDetailID, instead
				// of the current one. The code inside Alberta_ClaimTransaction() will later
				// change the sequence number if this is required.

				// (j.dinatale 2013-01-21 11:50) - PLID 54733 - if for some reason we dont have the configRT setting yet, go ahead and
				//	start with the current Claim History Detail ID
				if(nLastTransactionID == -1){
					nLastTransactionID = nCurrentClaimHistoryDetailID;
				}else{
					nLastTransactionID++;
				}

				strSequenceNumber.Format("%li", nLastTransactionID);
				if(strSequenceNumber.GetLength() > 7) {
					strSequenceNumber = strSequenceNumber.Right(7);
				}
				while(strSequenceNumber.GetLength() < 7) {
					//force it to be 7 digits long
					strSequenceNumber = "0" + strSequenceNumber;
				}

				// (j.dinatale 2013-01-21 11:50) - PLID 54733 - store the last transaction ID
				SetRemotePropertyInt("Alberta_LastTransactionID", nLastTransactionID, 0, "<None>");

				m_nAlbertaTotalTransactionCounts++;
				m_nAlbertaCurrentSegmentCount = 1;
				m_nAlbertaTotalSegmentCounts++;

				//load the patient ULI (also known as the personal health number)
				CString strServiceRecipientULI = "";
				{
					long nPatientULICustomField = GetRemotePropertyInt("Alberta_PatientULICustomField", 1, 0, "<None>", true);
					//this field will correspond to a patient's custom text field
					_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT "
						"WHERE FieldID = {INT} AND PersonID = {INT}", nPatientULICustomField, m_pEBillingInfo->PatientID);
					if(!rsCustom->eof) {
						strServiceRecipientULI = AdoFldString(rsCustom, "TextParam","");
						//this is numeric, so strip hyphens and spaces only incase they entered them,
						//anything else is their own data entry problem
						strServiceRecipientULI.Replace("-","");
						strServiceRecipientULI.Replace(" ","");
					}
					rsCustom->Close();
				}

				// (j.jones 2011-09-20 09:14) - PLID 44934 - need to pass in a note variable, but
				// it is not used in the CIB1 segment
				CString strNote = "";
				
				//each charge is one "claim transaction"
				// (j.dinatale 2013-01-18 16:57) - PLID 54419 - pass in the transaction number
				RETURN_ON_FAIL(Alberta_ClaimTransaction(nChargeID, nCurrentClaimHistoryDetailID, strServiceRecipientULI, strSequenceNumber,
					eActionCode, bHasChosenNewSequenceNumber,
					asCIB1, rsCharges, strNote, strAlbertaTransactionNumber));

				// (j.dinatale 2013-01-18 16:57) - PLID 54419 - update our table if we have a claim number
				if(!strAlbertaTransactionNumber.IsEmpty()){
					// only start doing this on new claims
					ExecuteParamSql("UPDATE ClaimHistoryDetailsT SET AlbertaTransNum = {STRING} WHERE ID = {INT}", strAlbertaTransactionNumber, AdoFldLong(rsCharges, "CurrentClaimHistoryDetailID"));
				}

				// (j.jones 2011-07-25 09:36) - PLID 44662 - if they chose a new sequence number,
				// track it, if we aren't already tracking one for this bill
				if(nNewClaimHistoryDetailID == -1 && bHasChosenNewSequenceNumber) {
					nNewClaimHistoryDetailID = nCurrentClaimHistoryDetailID;
				}

				//send a patient record if the patient does not have a ULI,
				//if the payee is not identified by the Pay to ULI (with a Pay to Code = OTHR),
				//if the Referring Service Provider is out of province (OOP)

				BOOL bSendPatientRecord = FALSE;

				//does the patient have a ULI?
				if(strServiceRecipientULI.IsEmpty()) {
					//nope, send the record
					bSendPatientRecord = TRUE;
				}
				else if(GetRemotePropertyText("Alberta_PayToCode", "BAPY", 0, "<None>", true) == "OTHR") {
					//pay to code is OTHR
					bSendPatientRecord = TRUE;
				}
				//OOP referring providers are not currently supported

				// (j.jones 2011-07-22 10:16) - PLID 44663 - if the action is to Reassess (Replace) or Delete (Void), do not send this line
				if(bSendPatientRecord && eActionCode != ctcReplacement && eActionCode != ctcVoid) {

					m_nAlbertaCurrentSegmentCount++;
					m_nAlbertaTotalSegmentCounts++;

					// (j.jones 2011-09-20 09:14) - PLID 44934 - need to pass in a note variable, but
					// it is not used in the CPD1 segment
					CString strNote = "";

					// (j.dinatale 2013-01-18 16:57) - PLID 54419 - alberta transaction ID tracking
					RETURN_ON_FAIL(Alberta_ClaimTransaction(nChargeID, nCurrentClaimHistoryDetailID, strServiceRecipientULI, strSequenceNumber,
						eActionCode, bHasChosenNewSequenceNumber,
						asCPD1, rsCharges, strNote, strAlbertaTransactionNumber));
				}

				// (j.jones 2011-07-22 10:16) - PLID 44663 - if the action is to Reassess (Replace), send a CST1 line
				// (j.jones 2015-10-06 14:24) - PLID 66889 - we now want this at all times, if a claim note exists, send it
				//if(eActionCode == ctcReplacement)
				{
					// (j.jones 2011-09-20 09:08) - PLID 44934 - CST1 only sends a note, so don't send if we don't have a note
					CString strNote = AdoFldString(rsCharges, "LineNote", "");
					strNote.TrimLeft();
					strNote.TrimRight();

					//there is a limit of 500 CST1 segments per charge, ensure we do not exceed that
					long nCountCST = 0;

					while(!strNote.IsEmpty() && nCountCST < 500) {

						//cache the current note for an error check later
						CString strOldNote = strNote;

						nCountCST++;
						m_nAlbertaCurrentSegmentCount++;
						m_nAlbertaTotalSegmentCounts++;

						//the CST1 segment will output as much of the note as it can, then trim off what it used,
						//such that it can be called again with the remainder of the note
						// (j.dinatale 2013-01-18 16:57) - PLID 54419 - alberta transaction ID tracking
						RETURN_ON_FAIL(Alberta_ClaimTransaction(nChargeID, nCurrentClaimHistoryDetailID, strServiceRecipientULI, strSequenceNumber,
							eActionCode, bHasChosenNewSequenceNumber,
							asCST1, rsCharges, strNote, strAlbertaTransactionNumber));

						//infinite loop check - if for some reason the above function does
						//not change strNote, don't loop forever! Something went wrong.
						if(strOldNote == strNote) {
							ASSERT(FALSE);
							//forcibly end the loop
							strNote = "";
						}
					}
				}

				//the OHIP_Item_Record function can advance the recordset,
				//so be careful to check for eof
				if(!rsCharges->eof) {
					rsCharges->MoveNext();
				}
			}
			rsCharges->Close();

			//set the previous provider/InsCo, as this is the end of the claim
			m_PrevProvider = m_pEBillingInfo->ProviderID;
		}

		//send the trailer
		RETURN_ON_FAIL(Alberta_Trailer());

		m_OutputFile.Close();

		m_Progress.SetValue(m_Progress.GetMax());

		if (MessageBox("All claims were successfully exported and are ready to send to Alberta HLINK.\n"
			"Do you want to transfer these claims to the unselected list?",
			"Electronic Billing",MB_YESNO|MB_ICONINFORMATION) == IDYES) {

			// (j.jones 2014-07-09 10:08) - PLID 62568 - ignore on hold claims
			ExecuteParamSql("UPDATE HCFATrackT SET CurrentSelect = 0 WHERE Batch = 2 AND BillID IN ("
				"SELECT BillsT.ID FROM BillsT "
				"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
				"WHERE Coalesce(BillStatusT.Type, -1) != {CONST_INT} "
				")", EBillStatusType::OnHold);
		}

		EndDialog(IDOK);

		return Success;

	} NxCatchAll(__FUNCTION__);
	
	return Error_Other;
}

// (j.jones 2010-07-12 08:43) - PLID 29971 - supported Alberta HLINK
int CEbilling::Alberta_Header()
{
	try {

		CString OutputString;

		//Sample line: 2ABC000001

		//BATCH HEADER		FIELD		FIELD		COMMENTS
		//DATA FIELD		POSITION	SIZE

		//Record Type		01-01		N(01)		Constant value = "2"

		OutputString += ParseField("2",1,'R','0');

		//Submitter Prefix	02-04		A(03)		As given by Alberta Health

		// (j.dinatale 2012-12-28 11:46) - PLID 54365 - use our new handy dandy function to get the submitter prefix
		CString strSubmitterPrefix = GetProviderSubmitterPrefix(m_nFilterByProviderID);
		OutputString += ParseField(strSubmitterPrefix,3,'L',' ');

		//Batch Number		05-10		N(06)		This must be unique across all batches in all files
		//											submitted by the submitter within the last 720 days.
		//											It should be assigned sequentially starting at 1 and
		//											can restart at 1 once 999,999 is reached.

		OutputString += ParseField(m_strBatchNumber,6,'R','0');

		//(not used)		11-255		A(244)		Leave blank

		OutputString += ParseField("", 244,'L',' ');

		ASSERT(OutputString.GetLength() == 254);

		// (j.jones 2011-06-15 11:50) - PLID 44124 - newlines are our friends
		OutputString += "\r\n";
		m_OutputFile.Write(OutputString,OutputString.GetLength());

		return Success;

	}NxCatchAll(__FUNCTION__);

	return Error_Other;
}

// (j.jones 2011-07-21 12:25) - PLID 44662 - added more parameters for the purposes of potentially changing
// the sequence number based on the action code
// (j.jones 2011-09-20 09:11) - PLID 44934 - added strNote, only used when the segment is CST1
// (j.dinatale 2013-01-18 16:57) - PLID 54419 - pass in transaction number
int CEbilling::Alberta_ClaimTransaction(long nChargeID, IN OUT long &nClaimHistoryDetailID,
										CString strServiceRecipientULI, IN OUT CString &strSequenceNumber,
										IN OUT ANSI_ClaimTypeCode &eActionCode, IN OUT BOOL &bHasChosenNewSequenceNumber,
										EAlbertaSegment asSegmentType, ADODB::_RecordsetPtr &rsCharges, CString &strNote, CString &strAlbertaTransactionNumber)
{
	try {
		// (j.dinatale 2013-01-18 16:57) - PLID 54419 - reworked the order of this function, since we had to show the dialog sooner in order to collect certain info from the user

		CString OutputString;

		//Sample line: 3XXXYYSS1234567DCIP1CIB10001 A

		//TRANSACTION		FIELD		FIELD		NOTES
		//FIRST PART		POSITION	SIZE
		//DATA FIELD

		//Record Type		01-01		N(01)		This is always the number "3". Transaction Records
		//											are placed between a Batch Header (record type 2)
		//											and Batch Trailer Record (record type 4).

		OutputString += ParseField("3",1,'R','0');

		// (j.dinatale 2012-12-28 11:46) - PLID 54365 - use our new handy dandy function to get the submitter prefix
		CString strSubmitterPrefix = GetProviderSubmitterPrefix(m_nFilterByProviderID);
		if(strSubmitterPrefix.GetLength() > 3) {
			strSubmitterPrefix = strSubmitterPrefix.Left(3);
		}
		else if(strSubmitterPrefix.GetLength() < 3) {
			while(strSubmitterPrefix.GetLength() < 3) {
				strSubmitterPrefix += " ";
			}
		}

		CString strSourceCode;
		//use the provider's initials
		CString strProviderLast = AdoFldString(rsCharges, "ProviderLast", "");
		CString strProviderFirst = AdoFldString(rsCharges, "ProviderFirst", "");
		CString strProviderLastInitial, strProviderFirstInitial;
		strProviderLast.TrimLeft();
		if(!strProviderLast.IsEmpty()) {
			strProviderLastInitial = strProviderLast.Left(1);
		}
		strProviderFirst.TrimLeft();
		if(!strProviderFirst.IsEmpty()) {
			strProviderFirstInitial = strProviderFirst.Left(1);
		}
		strSourceCode.Format("%s%s", strProviderFirstInitial, strProviderLastInitial);

		// (j.jones 2011-07-22 09:39) - PLID 44662 - if we're not adding a claim, then we need to send a 
		// different sequence number, chosen from the list of those we have already sent in the past
		// (but only call this function once per charge)
		if(eActionCode != ctcOriginal && !bHasChosenNewSequenceNumber) {
			//find the count of times we have exported this claim *besides* the current export,
			//remember that Alberta updates claim history prior to exporting so we can use
			//the current ClaimHistoryDetailID as the SequenceNumber
			//(r.wilson 10/2/2012) plid 53082 - Replace hardcoded SendType with enumerated value
			_RecordsetPtr rsClaimHistory = CreateParamRecordset("SELECT ClaimHistoryDetailsT.ID, "
				"ClaimHistoryT.Date, PatientsT.UserDefinedID, Last + ', ' + First + ' ' + Middle AS PatName, "
				"BillsT.ID AS BillID, BillsT.Date AS BillDate, BillsT.Description, AlbertaTransNum "
				"FROM ClaimHistoryT "
				"INNER JOIN ClaimHistoryDetailsT ON ClaimHistoryT.ID = ClaimHistoryDetailsT.ClaimHistoryID "
				"INNER JOIN BillsT ON ClaimHistoryT.BillID = BillsT.ID "
				"INNER JOIN PersonT ON BillsT.PatientID = PersonT.ID "
				"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE ClaimHistoryT.SendType = {INT} AND ClaimHistoryDetailsT.ChargeID = {INT} "
				"AND ClaimHistoryDetailsT.ID <> {INT} "
				"ORDER BY ClaimHistoryT.Date DESC ", ClaimSendType::Electronic, nChargeID, nClaimHistoryDetailID);
			if(rsClaimHistory->eof) {
				//this claim has never been exported before now, so it has to be "Add"
				eActionCode = ctcOriginal;
			}
			else if(rsClaimHistory->GetRecordCount() == 1) {

				// (j.dinatale 2013-01-18 16:57) - PLID 54419 - get the transaction number
				strAlbertaTransactionNumber = AdoFldString(rsClaimHistory, "AlbertaTransNum", "");

				//this claim has only been exported once before, so use its ID
				nClaimHistoryDetailID = AdoFldLong(rsClaimHistory, "ID");

				//this is the new sequence number
				bHasChosenNewSequenceNumber = TRUE;
				strSequenceNumber = AsString(nClaimHistoryDetailID);

				if(strSequenceNumber.GetLength() > 7) {
					strSequenceNumber = strSequenceNumber.Right(7);
				}
				while(strSequenceNumber.GetLength() < 7) {
					//force it to be 7 digits long
					strSequenceNumber = "0" + strSequenceNumber;
				}
			}
			else if(rsClaimHistory->GetRecordCount() > 1) {
				//this claim has only been exported more than once,
				//so they need to select which claim to update

				CString strAction;
				switch(eActionCode) {
					case ctcReplacement:
						strAction = "reassess";
						break;
					case ctcVoid:
						strAction = "delete";
						break;
					case ctcCorrected:
					default:
						strAction = "change";
						break;
				}

				CAlbertaPastClaimSelectDlg dlg(this);

				long nUserDefinedID = AdoFldLong(rsClaimHistory, "UserDefinedID");
				CString strPatientName = AdoFldString(rsClaimHistory, "PatName");
				long nBillID = AdoFldLong(rsClaimHistory, "BillID");
				COleDateTime dtBillDate = AdoFldDateTime(rsClaimHistory, "BillDate");
				CString strBillDescription = AdoFldString(rsClaimHistory, "Description");

				while(!rsClaimHistory->eof) {
					ClaimHistoryInfo chInfo;
					chInfo.nID = AdoFldLong(rsClaimHistory, "ID");
					chInfo.dtDate = AdoFldDateTime(rsClaimHistory, "Date");
					chInfo.strClaimNumber = AdoFldString(rsClaimHistory, "AlbertaTransNum", "");
					dlg.m_aryClaimHistoryInfo.Add(chInfo);
					
					rsClaimHistory->MoveNext();
				}

				if(dlg.DoModal(nClaimHistoryDetailID, strAction, nUserDefinedID, strPatientName,
					nBillID, dtBillDate, strBillDescription, strSubmitterPrefix, strSourceCode) == IDOK) {
						// (j.dinatale 2013-01-18 16:57) - PLID 54419 - get the transaction number
						strAlbertaTransactionNumber = dlg.m_strSelectedClaimNumber;
						nClaimHistoryDetailID = dlg.m_nSelectedClaimHistoryDetailID;

						//this is the new sequence number
						bHasChosenNewSequenceNumber = TRUE;
						strSequenceNumber = AsString(nClaimHistoryDetailID);

						if(strSequenceNumber.GetLength() > 7) {
							strSequenceNumber = strSequenceNumber.Right(7);
						}
						while(strSequenceNumber.GetLength() < 7) {
							//force it to be 7 digits long
							strSequenceNumber = "0" + strSequenceNumber;
						}
				}
				else {
					//they did not pick a previous claim,
					//so change the action to add
					eActionCode = ctcOriginal;
				}
			}

			rsClaimHistory->Close();
		}

		// (j.dinatale 2013-01-18 16:57) - PLID 54419 - if we have a transaction number, then we need to output that instead
		if(!strAlbertaTransactionNumber.IsEmpty()){
			OutputString += ParseField(strAlbertaTransactionNumber, 15, 'L', ' ');
		} else {
			//Submitter Prefix	02-04		A(03)		A three-character code as assigned by Alberta Health
			//As assigned by							that uniquely identifies a submitter. Only the 'A' Add
			//Alberta Health							transaction must have a Prefix = to the current Submitter.
			//(part of claim #)							For C, D and R transactions, the original claim number
			//											must be used (it may have a different submitter prefix).
			//											Forms part of the Claim Number.

			// (j.dinatale 2013-01-18 16:57) - PLID 54419 - if we didnt get a prior transaction #, 
			//		then we need to concat a new string to give it to the caller
			strAlbertaTransactionNumber = "";

			OutputString += ParseField(strSubmitterPrefix,3,'L',' ');
			strAlbertaTransactionNumber += ParseField(strSubmitterPrefix,3,'L',' ');

			//Current Year		05-06		N(02)		The year the transaction was created (captured) by the Submitter.
			//(part of claim #)

			COleDateTime dtNow = COleDateTime::GetCurrentTime();
			CString strYear;
			strYear.Format("%li", dtNow.GetYear());
			strYear = strYear.Right(2);
			OutputString += ParseField(strYear,2,'R','0');

			strAlbertaTransactionNumber += ParseField(strYear,2,'R','0');

			//Source Code		07-08		A(02)		For Submitter use. Can be used to differentiate transactions
			//(part of claim #)							from different sources (e.g. Service Provider). This field
			//											cannot have a space in it.
			//											Examples of how the Source code can be used are:
			//											-a lone practitioner who works from a single facility
			//											may choose to always use the same source code
			//											-clinics with many practitioners may choose to use a
			//											different source code to identify each practitioner
			//											-organizations with several facilities or departments
			//											may choose to use different sources codes to identify
			//											each location

			OutputString += ParseField(strSourceCode,2,'L',' ');
			strAlbertaTransactionNumber += ParseField(strSourceCode,2,'L',' ');

			//Sequence Number	09-15		N(07)		A unique number within each transaction. No two transactions
			//(part of claim #)							can have the same SUBMITTER PREFIX, SOURCE CODE, CURRENT YEAR and
			//											SEQUENCE NUMBER except when a claim that was previously sent is to
			//											be changed, deleted, or reassessed. These fields must be the same
			//											as on the original claim addition transaction. In the case of
			//											claim transactions, the SUBMITTER PREFIX, CURRENT YEAR, SOURCE CODE,
			//											SEQUENCE NUMBER and CHECK DIGIT will form the Claim Number.

			while(strSequenceNumber.GetLength() < 7) {
				//force it to be 7 digits long
				strSequenceNumber = "0" + strSequenceNumber;
			}

			OutputString += ParseField(strSequenceNumber,7,'R','0');
			strAlbertaTransactionNumber += ParseField(strSequenceNumber,7,'R','0');

			//Check Digit		16-16		N(01)		Calculated from Sequence Number using modulus 10 formula
			//(part of claim #)							(see Appendix A for calculation method).

			// (j.jones 2011-07-22 09:24) - PLID 44662 - moved to be a global function
			// (j.jones 2013-04-10 16:30) - PLID 56191 - this is actually just calling CalculateLuhn,
			// but for posterity I kept the function name the same to keep some exception handling
			long nCheckDigit = CalculateAlbertaClaimNumberCheckDigit(strSequenceNumber);

			CString strCheckDigit;
			strCheckDigit.Format("%li", nCheckDigit);
			OutputString += ParseField(strCheckDigit,1,'R','0');
			strAlbertaTransactionNumber += ParseField(strCheckDigit,1,'R','0');
		}

		//Transaction Type	17-20		A(04)		Within record type "3' there is a transaction code that
		//											tells the system the type of transaction being
		//											submitted (e.g. In-Province claims). At this time
		//											only one transaction type code (CIP1) is being used
		//											for In-Province (IPC) and Out-of-Province (OOP)
		//											claims. Out-of-Province claims are for services to
		//											Out-of-Province recipients who are registered with
		//											another Provincial (except Quebec) Health Plan.
		//											This transaction excludes Hospital reciprocal claims.

		OutputString += ParseField("CIP1",4,'L',' ');

		//Segment Type		21-24		A(04)		A transaction can contain one or more segments and
		//											each segment, must be uniquely identified by a
		//											segment type.
		//											The header portion of the transaction identifies the
		//											type of transaction and type of segment. The
		//											following segment types exist (depending on the
		//											Action code).
		//
		//											CIB1 - In-Province Provider Base Claim
		//											segment (one only).
		//											CPD1 - Claim Person Data segments (maximum of
		//											three per transaction; one each for
		//											Service Recipient, Payee and OOP
		//											Referring Service Provider.
		//											CST1 - Claim Supporting Text segments (up to
		//											500 segments).
		//											CTX1 - Supporting Text cross-reference segment -
		//											can include a maximum of 14 crossreferenced
		//											claims that use the supporting
		//											text in segments "CPD1" or "CST1".
		//											The transaction header and segment portion data
		//											together cannot exceed 254 characters.

		CString strSegmentType = "";
		if(asSegmentType == asCIB1) {
			strSegmentType = "CIB1";
		}
		else if(asSegmentType == asCPD1) {
			strSegmentType = "CPD1";
		}
		else if(asSegmentType == asCST1) {
			strSegmentType = "CST1";
		}
		else if(asSegmentType == asCTX1) {
			strSegmentType = "CTX1";
		}
		else {
			//we would just fail again later if it's invalid, so throw an exception now
			ThrowNxException("Invalid claim segment used!");
		}

		OutputString += ParseField(strSegmentType,4,'L',' ');

		//Segment Sequence	25-28		N(04)		The unique sequence of a segment within a
		//											transaction. This is required for text segments to
		//											ensure text lines are kept in the proper sequence.
		//											Each segment within a transaction must have a
		//											unique sequence number - usually starting at one (1).

		CString strSegmentSequence;
		strSegmentSequence.Format("%li", m_nAlbertaCurrentSegmentCount);
		OutputString += ParseField(strSegmentSequence,4,'R','0');

		//Action Code		29-29		A(01)		The Action Code tells the system whether a claim is
		//											being added, changed, deleted, or re-assessed. The
		//											valid action codes that can be coded for this
		//											transaction are A, C, R, and D. All segments within a
		//											transaction must have the same action code. See
		//											Action Definitions described earlier in this Section.

		//A - Add Claim (default)	(tracked as ctcOriginal)
		//C - Change Claim			(tracked as ctcCorrected)
		//R - Reassess Claim		(tracked as ctcReplacement)
		//D - Delete Claim			(tracked as ctcVoid)
		
		// (j.jones 2011-07-21 12:27) - PLID 44663 - eActionCode is now passed in to this function
		// (j.jones 2016-05-24 14:21) - NX-100704 - this logic is now in a global function
		CString strActionCode = GetClaimTypeCode_Alberta(eActionCode);
		
		OutputString += ParseField(strActionCode, 1, 'L', ' ');
		
		//Unused			30-35		A(06)		Unused - leave blank.

		OutputString += ParseField("",6,'L',' ');

		ASSERT(OutputString.GetLength() == 35);
		
		//SEGMENT			36-254		A(219)		Segment part of transaction and contents depends on
		//(data portion)							the type of segment being completed. If a specific		
		//											transaction includes multiple segments, the segments
		//											must be in the following sequence (alphabetic):
		//											- CIB1 segment
		//											- CPD1 segments
		//											- CST1 segments
		//											- CTX1 segments

		//the functions will add to our OutputString
		if(asSegmentType == asCIB1) {
			RETURN_ON_FAIL(Alberta_ClaimSegment_CIB1(strServiceRecipientULI, OutputString, rsCharges));
		}
		else if(asSegmentType == asCPD1) {
			RETURN_ON_FAIL(Alberta_ClaimSegment_CPD1(OutputString, rsCharges));
		}
		else if(asSegmentType == asCST1) {
			RETURN_ON_FAIL(Alberta_ClaimSegment_CST1(OutputString, strNote));
		}
		else if(asSegmentType == asCTX1) {
			RETURN_ON_FAIL(Alberta_ClaimSegment_CTX1(OutputString));
		}
		else {
			//we will never get here as we check the segment type earlier in this function
			ThrowNxException("Invalid claim segment used!");
		}

		ASSERT(OutputString.GetLength() == 254);

		// (j.jones 2011-06-15 11:50) - PLID 44124 - newlines are our friends
		OutputString += "\r\n";
		m_OutputFile.Write(OutputString,OutputString.GetLength());

		return Success;

	}NxCatchAll(__FUNCTION__);

	return Error_Other;
}


int CEbilling::Alberta_ClaimSegment_CIB1(CString strServiceRecipientULI, CString &OutputString, ADODB::_RecordsetPtr &rsCharges)
{
	//used within the transaction header

	//CIB1 - In-Province Service Provider Base Claim segment (1 only).
	//		This is the first segment type. A "CIB1" segment must always be present
	//		on a Claim with an 'A' (Add) Action since it provides the base data
	//		for claims by in-province Service Providers and thus forms the basis of
	//		a transaction. Only one "CIB1" segment is allowed per transaction.
	//		Other segment types are conditional for Add transactions.

	try {

		//DATA FIELD		FIELD		FIELD		COMMENTS
		//					POSITION	SIZE

		//Transaction data	1-35					Transaction data is entered in column's 1-35		
		//entered here								(see Alberta_ClaimTransaction()).
		
		//Claim Type		36-39		A(04)		Use type "RGLR" for all In-Province Service Provider claims.

		//"RGLR" seems to be the only option allowed
		OutputString += ParseField("RGLR",4,'L',' ');
		
		//Service Provider	40-48		N(09)		A nine digit Unique Lifetime Identifier. All Service Providers
		//ULI										will have a unique ULI, which must be coded on each claim.		
		//											All ULI's have a check digit in the 5th position, which
		//											can be calculated using the Modulus 10
		//											formula located in Appendix D.

		CString strServiceProviderULI = AdoFldString(rsCharges, "ProviderNPI", "");
		// (j.jones 2011-06-16 16:52) - PLID 44124 - remove dashes & spaces
		strServiceProviderULI.Replace("-","");
		strServiceProviderULI.Replace(" ","");
		// (j.jones 2011-07-12 09:39) - PLID 44519 - despite being a number, we need to fill with spaces, not zeroes
		OutputString += ParseField(strServiceProviderULI,9,'R',' ');
		
		//Skill Code		49-52		A(04)		Designates the Service Provider's discipline
		//											and specialty/accreditation that the service was
		//											performed under.
		//											The valid SKILLs for each Service Provider
		//											will be registered in the Alberta Health
		//											Stakeholder Registry.
		//											This field need only be populated when the
		//											Service Provider has more than one SKILL
		//											and the HEALTH SERVICE can be
		//											performed by more than one of those SKILLS
		//											and the default SKILL for the BUSINESS
		//											ARRANGEMENT is not to be used.

		//use the provider's taxonomy code field
		// (d.singleton 2012-05-24 15:56) - PLID 50700 if skillcode is null in chargesT then we use the providers taxonomy code.  that is all done in the recordset query
		CString strSkillCode = AdoFldString(rsCharges, "SkillCode", "");
		OutputString += ParseField(strSkillCode,4,'L',' ');

		//Service Recipient	53-61		N(09)		All persons in Alberta will have a ULI/PHN
		//ULI										(Personal Health Number).		
		//											If the claim is being submitted as a Good Faith
		//											claim, this field and the Service Recipient
		//											Registration Number field should be
		//											spaces/blanks. A person data segment (CPD1)
		//											must accompany the transaction.
		//											If this is a Newborn claim this field and the
		//											Service Recipient Registration Number field
		//											could be spaces.
		//											All ULIs have a check digit in the 5th position,
		//											which can be calculated using the Modulus 10
		//											formula located in Appendix D.

		//this was already loaded by the caller
		// (j.jones 2011-06-16 16:52) - PLID 44124 - remove dashes & spaces
		strServiceRecipientULI.Replace("-","");
		strServiceRecipientULI.Replace(" ","");
		// (j.jones 2011-07-12 09:39) - PLID 44519 - despite being a number, we need to fill with spaces, not zeroes
		OutputString += ParseField(strServiceRecipientULI,9,'R',' ');

		//Service Recipient		62-73	A(12)		If this is a Medical Reciprocal claim this field
		//Registration Number						contains the other province Health Plan Registration Number.
		//											Note: If the other province’s health plan
		//											number and the PHN are known, a person
		//											data segment is not required.
		//											The Registration Number must pass check
		//											digit validation of the respective province (see
		//											Recovery Code field that follows). The
		//											validation routine depends on the province.

		CString strRegistrationNumber = "";
		{
			long nPatientRegNumberCustomField = GetRemotePropertyInt("Alberta_PatientRegNumberCustomField", 2, 0, "<None>", true);
			//this field will correspond to a patient's custom text field
			_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT "
				"WHERE FieldID = {INT} AND PersonID = {INT}", nPatientRegNumberCustomField, m_pEBillingInfo->PatientID);
			if(!rsCustom->eof) {
				strRegistrationNumber = AdoFldString(rsCustom, "TextParam","");
				//can be alphanumeric, so do not try to modify this value
			}
			rsCustom->Close();
		}
		OutputString += ParseField(strRegistrationNumber,12,'L',' ');

		//Health Service Code	74-80	A(07)		Indicates the health service performed. The
		//											Health Service Code must be a valid code
		//											within the Health Services Procedures List
		//											applicable to the discipline of the Service
		//											Provider. Procedures claimed under the
		//											Miscellaneous category (e.g. Unlisted
		//											procedures 99.09 and all other by-assessment
		//											items) must have supporting documentation
		//											(with the exception of 99.09T). The claimed
		//											amount must also be indicated. The Health
		//											Service must be claimed in accordance with
		//											the Health Service Governing Rules applicable
		//											to the discipline of the Service Provider.
		//											The Health Service Code must be allowable
		//											for the Service Provider based on the Service
		//											Provider's Skill and any service restrictions.
		//											The Health Service Code must be allowable
		//											for the Service Recipient's gender, age and
		//											other biological characteristics.
		//											The Health Service code must be allowable for
		//											the Business Arrangement based on any
		//											service restrictions defined for the Business
		//											Arrangement.
		//											The Health Service code must be allowable for
		//											the Facility/Functional Centre/Location based
		//											on any restrictions defined for the
		//											Facility/Functional Centre.
		//											The Health Service Code must be allowable
		//											for the Diagnostic code.
		//											The Health Service Code may be submitted in
		//											compressed format. (No embedded spaces).
		//											Alberta Health will decompress. (E.g. Input:
		//											E1 or E _ _ 1 will be allowed).
		//											Alberta Health's external format of the Health
		//											Service codes will be in decompressed format
		//											only. (E.g. Output:E _ _ 1).

		CString strServiceCode = AdoFldString(rsCharges, "ItemCode", "");
		OutputString += ParseField(strServiceCode,7,'L',' ');

		//Service Start Date	81-88	N(08)		YYYYMMDD - Indicates the day the health
		//											service was performed. In the case of hospital
		//											visits (03.03D), the date of the first day of
		//											consecutive hospital visit days is coded and
		//											the CALLS field must indicate the number of
		//											consecutive days.

		CString strServiceDate = "";
		_variant_t varDateFrom = rsCharges->Fields->Item["ServiceDateFrom"]->Value;
		if(varDateFrom.vt == VT_DATE) {
			COleDateTime dtDate = VarDateTime(varDateFrom);
			strServiceDate = dtDate.Format("%Y%m%d");
		}
		OutputString += ParseField(strServiceDate,8,'R','0');

		//Encounter Number		89-89	N(01)		Indicates if the service was performed during
		//											the first, second, third, etc. time the Service
		//											Provider has seen the Service Recipient on the
		//											same day.

		//Further notes from the specs:
		//Pertains to the number of separate times the practitioner sees the same patient on the same day.
		//Each separate encounter with a Service Recipient on the same day by the same Service Provider
		//must be given a unique Encounter Number. All services performed during an encounter with the
		//Service Recipient must be given the same Encounter Number.

		//DailyBillIndex was calculated as a unique index of bills for the same patient,
		//same provider, same service date. Will probably reject if they tried to send more
		//than 10 bills in one day for the same patient. Which is just crazy anyways.
		long nDailyBillIndex = AdoFldLong(rsCharges, "DailyBillIndex", 1);
		CString strEncounterNumber;
		strEncounterNumber.Format("%li", nDailyBillIndex);
		//this can only be one digit, so use the right-most digit
		strEncounterNumber = strEncounterNumber.Right(1);
		OutputString += ParseField(strEncounterNumber,1,'R','0');

		//Diagnosis Code 1		90-95	A(06)		Primary diagnosis (ICD-9 format) The
		//											Diagnostic Code must be valid for the Health
		//											Service Code entered. Diagnostic Code 1 is
		//											used for the primary diagnosis. Diagnostic
		//											Codes 2 and 3 are used for any secondary
		//											diagnosis, if applicable.

		// (a.walling 2014-03-19 10:49) - PLID 61420 - EBilling - Alberta - Get diag info from bill
		CString strDiagCode1 = m_pEBillingInfo->GetSafeBillDiag(0).number;
		OutputString += ParseField(strDiagCode1,6,'L',' ');

		//Diagnosis Code 2		96-101	A(06)		Secondary diagnosis code - if necessary (ICD-9 format).

		// (a.walling 2014-03-19 10:49) - PLID 61420 - EBilling - Alberta - Get diag info from bill
		CString strDiagCode2 = m_pEBillingInfo->GetSafeBillDiag(1).number;
		OutputString += ParseField(strDiagCode2,6,'L',' ');

		//Diagnosis Code 3		102-107	A(06)		Tertiary diagnosis code - if necessary (ICD-9 format).

		// (a.walling 2014-03-19 10:49) - PLID 61420 - EBilling - Alberta - Get diag info from bill
		CString strDiagCode3 = m_pEBillingInfo->GetSafeBillDiag(2).number;
		OutputString += ParseField(strDiagCode3,6,'L',' ');

		//Calls				108-110		N(03)		This field is used to indicate either the number
		//											of consecutive hospital visit days (03.03D),
		//											the number of services performed or the
		//											number of units (e.g. 15 minute time blocks)
		//											required. The Health Service Price List defines
		//											the meaning of this field for each applicable
		//											Health Service Code and defines the
		//											maximum calls allowed for each applicable
		//											Health Service Code.

		/// (j.jones 2011-10-25 17:19) - PLID 46088 - we now have a Calls field per charge
		CString strCalls = "";
		double dblCalls = VarDouble(rsCharges->Fields->Item["Calls"]->Value, 0.0);
		//don't send if zero
		if(dblCalls > 0) {			
			strCalls.Format("%g", dblCalls);
			//if this is a fraction (which I believe is not allowed anyways),
			//and > 3 characters, try to round to 1 decimal place
			if(strCalls.GetLength() > 3 && (long)dblCalls != dblCalls) {				
				dblCalls = Round(dblCalls, 1);
				strCalls.Format("%g", dblCalls);
			}
			//if this is still a fraction and > 3 characters, try to round to a whole number
			if(strCalls.GetLength() > 3 && (long)dblCalls != dblCalls) {
				//This data is limited to 3 characters, so if they enter a fraction that
				//is too big, just convert to a long, and let ParseField truncate as needed.
				//This is not valid data anyways, so it will likely not be accepted in any case.
				dblCalls = Round(dblCalls, 0);
				strCalls.Format("%g", dblCalls);
			}
			//if the size is still too big (>999), ParseField will only be able to send the first 3 digits,
			//so we might as well control what we send - 999 is not going to be valid anyways
			if(dblCalls > 999) {
				strCalls = "999";
			}
		}
		OutputString += ParseField(strCalls,3,'R','0');


		//Explicit Fee		111-116		A(06)		The Explicit Fee Modifier fields are used to
		//Modifier 1								enter explicit modifiers required to further
		//											identify the nature of the service for payment
		//											purposes.
		//											Explicit Fee Modifiers are those that cannot be
		//											derived from other data on the claim. An
		//											example of an Explicit Fee Modifier is the
		//											Role modifier, which indicates the role (e.g.
		//											surgical assist or anaesthetist) that the Service
		//											Provider was performing for the service.
		//											Fee Modifiers affect how much Alberta Health
		//											will pay for a Health Service Code. The
		//											applicable derived and the allowable explicit
		//											Fee Modifiers are described in the Health
		//											Service Price List.

		CString strModifier1 = AdoFldString(rsCharges, "CPTModifier", "");
		OutputString += ParseField(strModifier1,6,'L',' ');

		//Explicit Fee		117-122		A(06)		Used if more than one Explicit Fee Modifier is
		//Modifier 2								required. An example of two modifiers could
		//											be "role" and "services unscheduled" for any
		//											service where payment is affected by the role
		//											of the Service Provider (e.g. surgical assistant)
		//											and by the time block (e.g. at night) the service
		//											was performed.

		CString strModifier2 = AdoFldString(rsCharges, "CPTModifier2", "");
		OutputString += ParseField(strModifier2,6,'L',' ');

		//Explicit Fee		123-128		A(06)		Used if more than two Explicit Fee Modifiers
		//Modifier 3								are required.

		CString strModifier3 = AdoFldString(rsCharges, "CPTModifier3", "");
		OutputString += ParseField(strModifier3,6,'L',' ');

		//Facility Number	129-134		N(06)		The specific Facility where the service was
		//											performed as per the Alberta Health Facility
		//											Registry.
		//											If the service was not performed at a registered
		//											Facility, the LOCATION CODE field must be
		//											coded instead.

		// (j.jones 2012-03-21 12:54) - PLID 49084 - We used to make clients type the functional centre
		// code at the end of the POS NPI and split the numbers across these two fields. But sometimes
		// the functional centre will change per bill, so we had to make it pull from the POS Code instead.

		CString strFacilityNumber = AdoFldString(rsCharges, "PlaceOfServiceNPI", "");
		// (j.jones 2011-06-16 16:52) - PLID 44124 - remove dashes & spaces
		strFacilityNumber.Replace("-","");
		strFacilityNumber.Replace(" ","");
		if(strFacilityNumber.GetLength() > 6) {
			strFacilityNumber = strFacilityNumber.Left(6);
		}
		// (j.jones 2011-07-12 09:39) - PLID 44519 - despite being a number, we need to fill with spaces, not zeroes
		OutputString += ParseField(strFacilityNumber,6,'R',' ');

		//Functional Centre		135-138	A(04)		The specific Functional Centre within the
		//											Facility where the service was performed. An
		//											example is the Neonatal Intensive Care
		//											Functional Centre within the University
		//											Hospital.
		//											The Alberta Health Facility Registry details
		//											the valid Functional Centers for each
		//											registered Facility.
		//											This field is only required if the service was
		//											performed at a registered Facility and that
		//											Facility requires a Functional Centre to be
		//											coded.
		//											Some Facility/Functional Centers will have
		//											restrictions as to the Health Services that can
		//											be performed at them.

		// (j.jones 2012-03-21 12:54) - PLID 49084 - this is now the POS Code on the bill
		// (j.jones 2013-04-25 09:06) - PLID 55564 - we now load the place code in our ebilling object,
		// so we no longer need to pull from the recordset
		CString strFunctionalCentre = m_pEBillingInfo->strPOSCode;
		//remove dashes & spaces
		strFunctionalCentre.Replace("-","");
		strFunctionalCentre.Replace(" ","");
		if(strFunctionalCentre.GetLength() > 4) {			
			strFunctionalCentre = strFunctionalCentre.Left(4);
		}
		OutputString += ParseField(strFunctionalCentre,4,'L',' ');

		//Location Code		139-142		A(04)		If the service was not performed at a
		//											Registered Facility, the Location Code is
		//											required. Values are "HOME" (e.g. the
		//											Service Recipient's home) and "OTHR".

		//not currently supported
		OutputString += ParseField("",4,'L',' ');

		//Originating Facility	143-148 N(06)		Used to indicate the Facility where the
		//											encounter with the Service Recipient occurred,
		//											for those types of services, where the
		//											encounter can be at a different Facility from
		//											where the service was performed (e.g. when a
		//											specimen/procedure (blood sample/ECG/xray)
		//											is taken in one facility and tested/interpreted in
		//											another).

		//not used, it would be the same facility we sent above in the Facility Number field
		OutputString += ParseField("",6,'R','0');

		//Originating		149-152		A(04)		Valid values are; 'OTHR' for Other, and 'HOME' for Home.
		//Location									This field should only be used if the encounter
		//											with the patient occurred at somewhere other
		//											than where the service occurred, and that
		//											location is NOT a recognized facility. The
		//											Originating Facility and Originating Location
		//											cannot both be used on a single claim.
		//											If the code "OTHR" is used, then the exact
		//											location must be provided in the text.

		//we do not support this
		OutputString += ParseField("",4,'L',' ');

		//Business			153-159		N(07)		The Service Provider's Business Arrangement
		//Arrangement								that the Service Provider is claiming under.
		//											Business Arrangements are agreements
		//											between one or more Stakeholders and Alberta
		//											Health for provision and payment of Health
		//											services.
		//											The Business Arrangement describes the
		//											Service Providers that can provide services,
		//											the Contract Holder that the Service Providers
		//											are contracted to, and the payee that is to be
		//											paid for any claims. In some cases the above
		//											will all be the same Stakeholder.
		//											All Service Providers in Alberta must have or
		//											be part of a Business Arrangement registered
		//											with Alberta Health in order to claim for
		//											services. Some Service Providers may have
		//											and/or may be part of more than one Business
		//											Arrangement.
		//											Some Business Arrangements will have
		//											restrictions as to the Health Services that can
		//											be claimed.
		//											The Business Arrangement or the Contract
		//											Holder of the Business Arrangement must be
		//											registered to submit via the Submitter that has
		//											submitted the transaction. An exception to this
		//											would be if the practitioner is acting as a
		//											locum. If this practitioner has a Business
		//											Arrangement for this locum practice, then the
		//											locum Business Arrangement must be
		//											registered with the Submitter and the Locum
		//											Business Arrangement field is to be coded on
		//											the claim.

		// (j.jones 2011-08-05 11:54) - PLID 44898 - this is the provider's BAID (or claim provider),
		// if a supervising provider is in use then we would send theirs as the locum BAID later

		CString strBusinessArrangement = "";
		
		long nProviderBAIDCustomField = GetRemotePropertyInt("Alberta_ProviderBAIDCustomField", 6, 0, "<None>", true);

		if(m_pEBillingInfo->ProviderID != -1) {
			//this field will correspond to a provider's custom text field
			_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT "
				"WHERE FieldID = {INT} AND PersonID = {INT}", nProviderBAIDCustomField, m_pEBillingInfo->ProviderID);
			if(!rsCustom->eof) {
				strBusinessArrangement = AdoFldString(rsCustom, "TextParam","");
				//this is numeric, so strip hyphens and spaces only incase they entered them,
				//anything else is their own data entry problem
				strBusinessArrangement.Replace("-","");
				strBusinessArrangement.Replace(" ","");
			}
			rsCustom->Close();
		}

		OutputString += ParseField(strBusinessArrangement,7,'R','0');

		//Pay To Code	160-163		A(04)			Indicates to what person or organization the
		//											payment is to be made.
		//											Normally, the Pay To Code will indicate
		//											"Business Arrangement" which results in the
		//											payee defined for the Business Arrangement to
		//											be paid. If Pay To Code indicates Service
		//											Recipient, or Contract Holder (for example,
		//											pay to the parent Contract Holder if the service
		//											recipient is a child), or Other, then the claim is
		//											considered a Subscriber claim.
		//											If Pay To Code indicates "OTHR", the Pay To
		//											ULI or a Person Segment for Payee must be
		//											coded. The Pay to Code "OTHR" should only
		//											be used when the Pay to ULI does not act in
		//											any of the roles identified by the other Pay to
		//											Codes.
		//											The valid codes are:
		//											- "CONT" (Contract Holder)
		//											- "RECP" (Service Recipient)
		//											- "BAPY" (Business Arrangement), or
		//											- "OTHR" (Other)
		//											- "PRVD" (Service Provider)

		CString strPayToCode = GetRemotePropertyText("Alberta_PayToCode", "BAPY", 0, "<None>", true);
		//we store the code directly in ConfigRT, output whatever it is that's saved there
		OutputString += ParseField(strPayToCode,4,'L',' ');

		//Pay to ULI	164-172		N(09)			If Pay To Code indicates "other" and the ULI
		//											of the other person is known, code the ULI
		//											here. If ULI of the other person is not known,
		//											a Person Segment for Payee is required.
		//											All ULIs have a check digit in the 5th position,
		//											which can be calculated using the Modulus 10
		//											formula located in Appendix D.

		//not supported
		OutputString += ParseField("",9,'R','0');

		//Locum			173-179		N(07)			If the Service Provider is performing the
		//Arrangement								Service as a result of a Locum arrangement
		//Business									with a second Service Provider and the claims
		//Arrangement								are to be submitted via the first Service
		//											Provider's Submitter, then the first Service
		//											Provider's Business Arrangement must be
		//											coded.

		// (j.jones 2011-08-05 11:54) - PLID 44898 - we now support this, it pulls the BAID
		// from the supervising provider on the bill

		CString strLocumBusinessArrangement = "";

		//get the supervising provider's ID
		long nSupervisingProviderID = VarLong(rsCharges->Fields->Item["SupervisingProviderID"]->Value, -1);

		//if this ID is different, load that provider's BAID as the locum ID
		if(m_pEBillingInfo->ProviderID != -1 && m_pEBillingInfo->ProviderID != nSupervisingProviderID) {
			//this field will correspond to a provider's custom text field
			_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT "
				"WHERE FieldID = {INT} AND PersonID = {INT}", nProviderBAIDCustomField, nSupervisingProviderID);
			if(!rsCustom->eof) {
				strLocumBusinessArrangement = AdoFldString(rsCustom, "TextParam","");
				//this is numeric, so strip hyphens and spaces only incase they entered them,
				//anything else is their own data entry problem
				strLocumBusinessArrangement.Replace("-","");
				strLocumBusinessArrangement.Replace(" ","");
			}
			rsCustom->Close();
		}

		//can't use locum if it matches the main BAID
		if(strLocumBusinessArrangement == strBusinessArrangement) {
			strLocumBusinessArrangement = "";
		}

		OutputString += ParseField(strLocumBusinessArrangement,7,'R','0');

		//Referral ULI	180-188		A(09)			If the Service was a referred service, the
		//											referring Service Provider's ULI must be
		//											coded. If the service was referred from an
		//											OOP Service Provider, this field can be blank
		//											but the OOP REFERRAL IND must be set to
		//											Y, and the person data segment must be
		//											completed.
		//											All ULIs have a check digit in the 5th position,
		//											which can be calculated using the Modulus 10
		//											formula located in Appendix D.

		CString strReferralULI = AdoFldString(rsCharges, "RefPhyNPI", "");
		OutputString += ParseField(strReferralULI,9,'L',' ');

		//OOP Referral	189-189		A(01)			"Y" indicates that the service was referred
		//Indicator									from an OOP Service Provider. A person data
		//											segment for the OOP Referring Service
		//											Provider must be included.
		//											Note: Medical Reciprocal claims referred
		//											from an OOP Service Provider does
		//											not need a person data segment for
		//											the OOP Referring Service
		//											Provider.

		//not currently supported
		OutputString += ParseField("",1,'L',' ');

		//Recovery Code		190-193		A(04)		If the responsibility for the entire payment for
		//											the Service is not Alberta Health, then the
		//											responsible organization must be coded (e.g.
		//											WCB).
		//											If the claim is for services provided to a
		//											Service Recipient registered in another
		//											provincial health plan as per the Medical
		//											Reciprocal Agreement, the other province
		//											code (e.g. SK) is coded in this field.
		//											A Medical Reciprocal Claim requires the OOP
		//											Registration Number to be coded in the
		//											SERVICE RECIPIENT REGN NO. FIELD
		//											and also requires a Person Data segment for
		//											Service Recipient.
		//											The valid codes are:
		//											- "WCB" (WCB claims), or
		//											- any valid province abbreviation (except
		//											Quebec) for Medical Reciprocal claims.

		//send the plan name only if it is WCB or a non-Quebec province
		CString strPlanName = AdoFldString(rsCharges, "PlanName", "");
		strPlanName.MakeUpper();
		CString strRecoveryCode = "";
		if(strPlanName == "WCB"	//Worker's Compensation
			|| strPlanName == "AB"	//Alberta
			|| strPlanName == "BC"	//British Columbia
			|| strPlanName == "MB"	//Manitoba
			|| strPlanName == "NB"	//New Brunswick
			|| strPlanName == "NL"	//Newfoundland and Labrador
			|| strPlanName == "NF"	//Newfoundland (pre-2002)	// (j.jones 2011-07-21 08:58) - PLID 44660 - NF is an accepted abbreviation, despite being obsolete
			|| strPlanName == "NT"	//Northwest Territories
			|| strPlanName == "NS"	//Nova Scotia
			|| strPlanName == "NU"	//Nunavut
			|| strPlanName == "ON"	//Ontario
			|| strPlanName == "PE"	//Prince Edward Island
			|| strPlanName == "SK"	//Saskatchewan
			|| strPlanName == "YT"	//Yukon
			) {
			strRecoveryCode = strPlanName;
		}
		OutputString += ParseField(strRecoveryCode,4,'L',' ');

		//Chart Number		194-207		A(14)		Free format field.
		//											This is a clinic use field, which can be used for
		//											any source reference number.

		CString strChartNumber;
		strChartNumber.Format("%li", m_pEBillingInfo->UserDefinedID);
		OutputString += ParseField(strChartNumber,14,'L',' ');

		//Claimed			208-216		N(09)		This field is used in conjunction with the
		//Amount									Claimed Amount Indicator.				
		//											If the CLAIMED AMOUNT INDICATOR is
		//											not "Y", the Claimed Amount is ignored but
		//											will be returned on the Assessment Result
		//											Details File List and Statement of Assessments
		//											for reconciliation purposes.
		//											If the Service Provider wishes to claim an
		//											amount less than the normal amount to be paid
		//											for the service then the Claimed Amount
		//											Indicator can be set to "Y". If the Claimed
		//											Amount is less than the amount assessed by
		//											Alberta Health, the Claimed Amount will be
		//											paid. If the Claimed Amount Indicator is "Y"
		//											and the Claimed Amount is more than the
		//											Alberta Health Assessed Amount, the Claimed
		//											Amount will be ignored.

		//it is not clear what the purpose of this field is, seems as though it is used to state
		//"I expect to get this much" but HLINK says "too bad, you get what we pay you and you'll like it!"
		COleCurrency cyLineTotal = AdoFldCurrency(rsCharges, "LineTotal", COleCurrency(0,0));
		CString strClaimedAmount = FormatCurrencyForInterface(cyLineTotal,FALSE,FALSE);
		strClaimedAmount = StripNonNum(strClaimedAmount);
		OutputString += ParseField(strClaimedAmount,9,'R','0');
		
		//Claimed Amount	217-217		A(01)		"Y" indicates that the Service Provider is
		//Indicator									claiming an amount less than the normal
		//											amount to be paid for the service.

		//not currently supported
		OutputString += ParseField("",1,'L',' ');

		//Intercept Reason	218-221		A(04)		If the payment for the claim is to be
		//											intercepted by Alberta Health (e.g. not to be
		//											mailed directly to the payee), the reason for
		//											the intercept must be coded.
		//											The valid codes are:
		//											- "PKUP"(Hold for pickup).
		//											NOTE: This code cannot be used if the pay to
		//											code is “BAPY”.

		//not currently supported
		OutputString += ParseField("",4,'L',' ');
		
		//Confidential		222-222		A(01)		"Y" indicates that the Service Recipient
		//Indicator									indicated that the service is to be confidential.

		//not currently supported
		OutputString += ParseField("",1,'L',' ');
		
		//Good Faith		223-223		A(01)		"Y" indicates that the Service Provider is
		//Indicator									submitting the claim as a Good Faith claim as
		//											per the Alberta Health Good Faith policy. If
		//											the claim is submitted as a Good Faith claim, a
		//											Person Data Segment for the Service Recipient
		//											is required.

		//not currently supported
		OutputString += ParseField("",1,'L',' ');
		
		//Newborn Code		224-227		A(04)		Indicates Service Recipient is new-born
		//											without an Alberta Health ULI.
		//											If Newborn, a Person Segment for the Service
		//											Recipient is required.
		//											Valid codes are:
		//											- "ADOP" (Adoption)
		//											- "LVBR" (Live birth)
		//											- "STBN" (Still Born)
		//											- "MULT" (Multiple Birth)

		//not currently supported
		OutputString += ParseField("",4,'L',' ');
		
		//EMSAF Indicator	228-228		A(01)		"Y" indicates that the Service Provider is
		//(Extraordinary							submitting the claim as an EMSAF claim as
		//Medical Services							per the Alberta Health EMSAF policy.
		//Assessment Fund)							If the claim is submitted as an EMSAF claim,		
		//											Supporting Text is required. The extra amount
		//											claimed must be indicated in the Supporting
		//											Text.

		//not currently supported
		OutputString += ParseField("",1,'L',' ');

		//Paper Supporting	229-229		A(01)		Indicates that supporting documentation is
		//Documentation								being sent on paper (e.g. not as electronic text).
		//Indicator									Supporting documentation should only be sent
		//											on paper if graphics are required. The paper
		//											supporting documentation must reference the
		//											Claim Number.

		//use the SendCorrespondence feature from the bill
		BOOL bSendCorrespondence = AdoFldBool(rsCharges, "SendCorrespondence", FALSE);
		CString strSupportingDocInd = "N";
		if(bSendCorrespondence) {
			strSupportingDocInd = "Y";
		}
		OutputString += ParseField(strSupportingDocInd,1,'L',' ');

		//Hospital Admission /	230-237	N(08)		YYYYMMDD format. If the service performed is
		//Originating								a hospital visit (03.03D), the date the
		//Encounter Date							Service Recipient was admitted to hospital
		//											must be indicated on each hospital visit claim.
		//											When the Originating Facility field is entered
		//											on a claim, the "Originating Encounter Date"
		//											may also be used.

		CString strHospAdmissionDate = "";
		_variant_t varHospFrom = rsCharges->Fields->Item["HospFrom"]->Value;
		if(varHospFrom.vt == VT_DATE) {
			COleDateTime dtHospFrom = VarDateTime(varHospFrom);
			strHospAdmissionDate = dtHospFrom.Format("%Y%m%d");
		}
		OutputString += ParseField(strHospAdmissionDate,8,'R','0');
		
		//Tooth Code		238-239		A(02)		As per Dental Procedures List.

		//we don't support this
		OutputString += ParseField("",2,'L',' ');

		//Tooth Surface 1	240-241		A(02)		As per Dental Procedures List.

		//we don't support this
		OutputString += ParseField("",2,'L',' ');

		//Tooth Surface 2	242-243		A(02)		As per Dental Procedures List.

		//we don't support this
		OutputString += ParseField("",2,'L',' ');

		//Tooth Surface 3	244-245		A(02)		 As per Dental Procedures List.

		//we don't support this
		OutputString += ParseField("",2,'L',' ');

		//Tooth Surface 4	246-247		A(02)		 As per Dental Procedures List.

		//we don't support this
		OutputString += ParseField("",2,'L',' ');

		//Tooth Surface 5	248-249		A(02)		 As per Dental Procedures List.

		//we don't support this
		OutputString += ParseField("",2,'L',' ');

		//Unused			250-254		A(05)

		OutputString += ParseField("",5,'L',' ');

		ASSERT(OutputString.GetLength() == 254);

		return Success;

	}NxCatchAll(__FUNCTION__);

	return Error_Other;
}

int CEbilling::Alberta_ClaimSegment_CPD1(CString &OutputString, _RecordsetPtr &rsCharges)
{
	//used within the transaction header

	//CPD1 - Claim Person Data segments (multiple as needed).
	//		The second segment type "CPD1", in addition to the mandatory first segment
	//		"CIB1" for an Add, is required for each person referenced on a claim where:
	//		-	the Service Recipient (person receiving a service from a Service Provider)
	//			does not have a Unique Lifetime Identifier (ULI) and;
	//		-	a Payee not already identified by the Pay to ULI, where the Pay to Code = OTHR.
	//		-	an out of province Referring Service Provider
	//		This segment is necessary to document the name, address, and other pertinent
	//		information so processing may be completed.

	try {

		//DATA FIELD		FIELD		FIELD		COMMENTS
		//					POSITION	SIZE

		//Transaction data	1-35					Transaction data is entered in column's 1-35		
		//entered here								(see Alberta_ClaimTransaction()).

		//Person Type		36-39		A(04)		Indicates if person segment is for Service
		//											Recipient, or Payee, or OOP Referring Service
		//											Provider.
		//											Valid codes are:
		//											- "RECP" (Service Recipient)
		//											- "PYST" (Payee) and
		//											- "RFRC" (OOP Referring Service Provider)

		//we only support "RECP"
		OutputString += ParseField("RECP",4,'L',' ');

		//Surname			40-69		A(30)

		CString strLastName = AdoFldString(rsCharges, "Last", "");
		OutputString += ParseField(strLastName,30,'L',' ');

		//Middle Name		70-81		A(12)

		CString strMiddleName = AdoFldString(rsCharges, "Middle", "");
		OutputString += ParseField(strMiddleName,12,'L',' ');

		//First Name		82-93		A(12)		Do not use wording such as Baby Boy, Infant Girl
		//											in this field for new-borns. If a baby's name is not
		//											known, this field must be left blank.

		CString strFirstName = AdoFldString(rsCharges, "First", "");
		OutputString += ParseField(strFirstName,12,'L',' ');

		//Birth Date		94-101		N(08)		YYYYMMDD format.

		CString strBirthDate = "";
		_variant_t varBD = rsCharges->Fields->Item["BirthDate"]->Value;
		if(varBD.vt == VT_DATE) {
			COleDateTime dtBirthDate = VarDateTime(varBD);
			strBirthDate = dtBirthDate.Format("%Y%m%d");
		}
		OutputString += ParseField(strBirthDate,8,'R','0');

		//Gender Code		102-102		A(01)		Valid values are M and F.

		short nGender = AdoFldByte(rsCharges, "Gender",0);
		CString strGender = "";
		if(nGender == 1) {
			strGender = "M";
		}
		else if(nGender == 2) {
			strGender = "F";
		}
		OutputString += ParseField(strGender,1,'L',' ');

		//Address Line 1	103-127		A(25)		Should contain non-address data (e.g. company
		//											name) if applicable, otherwise the street or mailing
		//											address should be here. The apartment or unit
		//											number is to be placed at the end of the street
		//											address. No symbols (#,-) are to be placed before
		//											the number.

		CString strAddress1 = AdoFldString(rsCharges, "Address1", "");
		OutputString += ParseField(strAddress1,25,'L',' ');

		//Address line 2	128-152		A(25)

		CString strAddress2 = AdoFldString(rsCharges, "Address2", "");
		OutputString += ParseField(strAddress2,25,'L',' ');

		//Address Line 3	153-177		A(25)

		//not supported
		OutputString += ParseField("",25,'L',' ');

		//City Name			178-207		A(30)		Full name or abbreviations for Alberta cities
		//											allowed by Alberta Health (provided as one of the
		//											Validation files available for Submitter retrieval).

		CString strCity = AdoFldString(rsCharges, "City", "");
		OutputString += ParseField(strCity,30,'L',' ');

		//Postal Code		208-213		A(06)

		CString strZip = AdoFldString(rsCharges, "Zip", "");
		OutputString += ParseField(strZip,6,'L',' ');
		
		//Province/			214-215		A(02)
		//State Code

		CString strState = AdoFldString(rsCharges, "State", "");
		OutputString += ParseField(strState,2,'L',' ');
		
		//Country Code		216-219		A(04)
		//Guardian/

		// (j.jones 2011-07-20 09:23) - PLID 44637 - this uses the ISO3Code for the CountryCode,
		// as this needs to be three characters, not two
		CString strCountry = AdoFldString(rsCharges, "CountryCode", "");
		if(strCountry.IsEmpty()) {
			strCountry = "CAN";
		}
		OutputString += ParseField(strCountry,4,'L',' ');
		
		//Parent Code (ULI)	220-228		N(09)		Used when Service Recipient is a Newborn.
		//											All ULIs have a check digit in the 5th position,
		//											which can be calculated using the Modulus 10
		//											formula located in Appendix D.

		//not currently supported
		OutputString += ParseField("",9,'R','0');
		
		//Guardian/Parent	229-240		A(12)		Used if Guardian/Parent ULI is not known and the
		//Registration								Guardian/Parent Registration Number is known.
		//No.

		//not currently supported
		OutputString += ParseField("",12,'L',' ');

		//Unused			241-254		A(14)		Leave blank.

		OutputString += ParseField("",14,'L',' ');

		ASSERT(OutputString.GetLength() == 254);

		return Success;

	}NxCatchAll(__FUNCTION__);

	return Error_Other;
}

// (j.jones 2011-07-22 10:25) - PLID 44662 - added the charge recordset
// (j.jones 2011-09-20 09:07) - PLID 44934 - this now takes in a note that may be spanned across multiple CST1 segments
// which can be repeated 500 times
int CEbilling::Alberta_ClaimSegment_CST1(CString &OutputString, CString &strNote)
{
	//used within the transaction header

	//CST1 - Claim Supporting Text segments (up to 500).
	//		The third segment "CST1" is used when supporting text is required for a claim.
	//		Up to 500 "CST1" segments can be used.

	try {

		CString strLine1 = "";
		CString strLine2 = "";
		CString strLine3 = "";

		// (j.jones 2011-09-20 09:19) - PLID 44934 - try to split the note by newlines,
		// otherwise split by text length
		while(strLine1.IsEmpty() && !strNote.IsEmpty()) {	//while loop will strip unnecessary extra newlines
			long nNewLine = strNote.Find("\r\n");
			if(nNewLine != -1 && nNewLine < 73) {
				strLine1 = strNote.Left(nNewLine);
				strNote = strNote.Right(strNote.GetLength() - nNewLine - 2);
			}
			else if(strNote.GetLength() > 73) {
				strLine1 = strNote.Left(73);
				strNote = strNote.Right(strNote.GetLength() - 73);
			}
			else {
				strLine1 = strNote;
				strNote = "";
			}
		}

		//Line 2
		while(strLine2.IsEmpty() && !strNote.IsEmpty()) {	//while loop will strip unnecessary extra newlines
			long nNewLine = strNote.Find("\r\n");
			if(nNewLine != -1 && nNewLine < 73) {
				strLine2 = strNote.Left(nNewLine);
				strNote = strNote.Right(strNote.GetLength() - nNewLine - 2);
			}
			else if(strNote.GetLength() > 73) {
				strLine2 = strNote.Left(73);
				strNote = strNote.Right(strNote.GetLength() - 73);
			}
			else {
				strLine2 = strNote;
				strNote = "";
			}
		}

		//Line 3
		while(strLine3.IsEmpty() && !strNote.IsEmpty()) {	//while loop will strip unnecessary extra newlines
			long nNewLine = strNote.Find("\r\n");
			if(nNewLine != -1 && nNewLine < 73) {
				strLine3 = strNote.Left(nNewLine);
				strNote = strNote.Right(strNote.GetLength() - nNewLine - 2);
			}
			else if(strNote.GetLength() > 73) {
				strLine3 = strNote.Left(73);
				strNote = strNote.Right(strNote.GetLength() - 73);
			}
			else {
				strLine3 = strNote;
				strNote = "";
			}
		}

		//do not trim the text, there may be intended spaces between split segments

		//DATA FIELD		FIELD		FIELD		COMMENTS
		//					POSITION	SIZE

		//Transaction data	1-35					Transaction data is entered in column's 1-35		
		//entered here								(see Alberta_ClaimTransaction()).

		//Text Line 1		36-108		A(73)
		OutputString += ParseField(strLine1,73,'L',' ');

		//Text Line 2		109-181		A(73)
		OutputString += ParseField(strLine2,73,'L',' ');

		//Text Line 3		182-254		A(73)
		OutputString += ParseField(strLine3,73,'L',' ');

		ASSERT(OutputString.GetLength() == 254);

		return Success;

	}NxCatchAll(__FUNCTION__);

	return Error_Other;
}

int CEbilling::Alberta_ClaimSegment_CTX1(CString &OutputString)
{
	//used within the transaction header

	//CTX1 - Supporting Text cross-reference segment (as required).
	//		The fourth segment "CTX1" is used where the supporting text in segments "CST1"
	//		above is also used for other claims. This segment can include up to fourteen (14)
	//		Claim Numbers that use the same supporting text. Only one (1) "CTX1" segment can
	//		be used per transaction.

	try {

		//DATA FIELD		FIELD		FIELD		COMMENTS
		//					POSITION	SIZE

		//Transaction data	1-35					Transaction data is entered in column's 1-35		
		//entered here								(see Alberta_ClaimTransaction()).

		//Claim Number		36-245		A(15) X14	Occurs up to 14 times.

		//1
		OutputString += ParseField("",15,'L',' ');
		//2
		OutputString += ParseField("",15,'L',' ');
		//3
		OutputString += ParseField("",15,'L',' ');
		//4
		OutputString += ParseField("",15,'L',' ');
		//5
		OutputString += ParseField("",15,'L',' ');
		//6
		OutputString += ParseField("",15,'L',' ');
		//7
		OutputString += ParseField("",15,'L',' ');
		//8
		OutputString += ParseField("",15,'L',' ');
		//9
		OutputString += ParseField("",15,'L',' ');
		//10
		OutputString += ParseField("",15,'L',' ');
		//11
		OutputString += ParseField("",15,'L',' ');
		//12
		OutputString += ParseField("",15,'L',' ');
		//13
		OutputString += ParseField("",15,'L',' ');
		//14
		OutputString += ParseField("",15,'L',' ');

		//Unused			246-254		A(09)		Leave blank.

		OutputString += ParseField("",9,'L',' ');

		ASSERT(OutputString.GetLength() == 254);

		return Success;

	}NxCatchAll(__FUNCTION__);

	return Error_Other;
}

int CEbilling::Alberta_Trailer()
{
	try {

		CString OutputString;

		//BATCH TRAILER		FIELD		FIELD		COMMENTS
		//DATA FIELD		POSITION	SIZE

		//Record Type		01-01		N(01)		Constant value = "4"

		OutputString += ParseField("4",1,'R','0');

		//Submitter Prefix	02-04		A(03)		As given by Alberta Health

		// (j.dinatale 2012-12-28 11:46) - PLID 54365 - use our new handy dandy function to get the submitter prefix
		CString strSubmitterPrefix = GetProviderSubmitterPrefix(m_nFilterByProviderID);
		OutputString += ParseField(strSubmitterPrefix,3,'L',' ');

		//Batch Number		05-10		N(06)		Same as the Batch Header Number. Must be
		//											unique within the Submitter Prefix starting at 1
		//											and re-started after 999999 for all batches in all
		//											files submitted within the last 720 days.

		OutputString += ParseField(m_strBatchNumber,6,'R','0');
		
		//Total TXN's		11-15		N(05)		Total number of transactions in the batch
		//											(excludes Header and Trailer records). A batch
		//											can have a maximum of 99999 transactions,
		//											although Alberta Health recommends that
		//											batches have between 100-500 transactions.
		//											Total number of transactions should include
		//											leading zeros. Do not leave blanks.

		CString strTotalTXNs;
		strTotalTXNs.Format("%li", m_nAlbertaTotalTransactionCounts);
		OutputString += ParseField(strTotalTXNs,5,'R','0');
		
		//Total Segments	16-23		N(08)		The total of all segments in all transactions.
		//											Total number of transactions should include
		//											leading zeros. Do not leave blanks.

		CString strTotalSegments;
		strTotalSegments.Format("%li", m_nAlbertaTotalSegmentCounts);
		OutputString += ParseField(strTotalSegments,8,'R','0');

		//Unused			24-254		A(231)		Leave blank.

		OutputString += ParseField("",231,'L',' ');

		ASSERT(OutputString.GetLength() == 254);

		// (j.jones 2011-06-15 11:50) - PLID 44124 - there is intentionally no new line
		// here, because this is the final record in the file
		m_OutputFile.Write(OutputString,OutputString.GetLength());

		return Success;

	}NxCatchAll(__FUNCTION__);

	return Error_Other;
}

// (j.dinatale 2012-12-28 11:26) - PLID 54365 - need to be able to get the provider's submitter prefix
CString CEbilling::GetProviderSubmitterPrefix(long nProviderID)
{
	// see if we already cached for this export
	if(m_bAlbertaSubmitterPrefixCached){
		return m_strAlbertaSubmitterPrefix;
	}

	// (j.dinatale 2012-12-31 13:04) - PLID 54382 - took out the logic here, since its needed else where
	CString strSubmitterPrefix = GetAlbertaProviderSubmitterPrefix(nProviderID);

	m_strAlbertaSubmitterPrefix = strSubmitterPrefix;
	m_bAlbertaSubmitterPrefixCached = true;
	return strSubmitterPrefix;
}


// (a.walling 2014-03-17 14:31) - PLID 61202 - Returns whether ICD10 should be used based on the m_pEBillingInfo's InsuredParty and BillID and m_FormatStyle+m_avANSIVersion
bool CEbilling::ShouldUseICD10() const
{
	// (a.walling 2014-03-19 14:32) - PLID 61346 - only ANSI will use ICD10; IMAGE, OHIP, ALBERTA styles will all continue to use ICD9.
	if (m_FormatStyle != ANSI) {
		return false;
	}

	// always false if ANSI 4010
	if (m_avANSIVersion == av4010) {
		return false;
	}

	if (!m_pEBillingInfo) {
		ASSERT(FALSE);
		return false;
	}

	return ::ShouldUseICD10(m_pEBillingInfo->InsuredPartyID, m_pEBillingInfo->BillID);
}

// (a.walling 2014-03-19 10:05) - PLID 61346 - Only skip unlinked diags when using a HCFA export with the SkipUnlinkedDiagsOnClaims preference enabled
bool CEbilling::ShouldSkipUnlinkedDiags() const
{
	if (m_actClaimType != actProf) {
		return false;
	}

	if (m_FormatStyle != ANSI && m_FormatStyle != IMAGE) {
		return false;
	}

	if (!GetRemotePropertyInt("SkipUnlinkedDiagsOnClaims", 0, 0, "<None>", true)) {
		return false;
	}

	return true;
}

// (a.wilson 2014-06-30 09:48) - PLID 62518 - determine whether the primary and secondary have the same diagnosis codes sets
bool CEbilling::DiagnosisCodeSetMismatch(bool bInsuredPartyShouldUseICD10) const
{
	//if setting is off then mismatch doesn't matter.
	if (!m_bDontSendSecondaryOnDiagnosisMismatch) {
		return false;
	}

	bool bOtherInsuredPartyShouldUseICD10 = (::ShouldUseICD10(m_pEBillingInfo->OthrInsuredPartyID, m_pEBillingInfo->BillID));

	if (bInsuredPartyShouldUseICD10 != bOtherInsuredPartyShouldUseICD10) {
		return true;
	} else {
		return false;
	}
}

// (j.jones 2014-08-25 08:45) - PLID 54213 - given an array of OtherInsuranceInfo objects,
// try to remove unnecessary duplicate payers, and update the SBR01 qualifier for those
// that remain
void CEbilling::ScrubOtherInsuredPartyList(CArray<OtherInsuranceInfo, OtherInsuranceInfo> &aryOtherInsuredPartyInfo)
{
	//first loop through and see if any payer exists twice with the same payer ID
	for (int i = aryOtherInsuredPartyInfo.GetSize() - 1; i >= 0; i--) {

		OtherInsuranceInfo oiiCurPayer = aryOtherInsuredPartyInfo.GetAt(i);

		if (oiiCurPayer.bIsOnBill) {
			//we will never remove a payer that is on the bill,
			//so don't bother comparing payer IDs
			continue;
		}

		bool bRemoved = 0;
		for (int j = aryOtherInsuredPartyInfo.GetSize() - 2; j >= 0 && !bRemoved; j--) {			
			OtherInsuranceInfo oiiPayerToCheck = aryOtherInsuredPartyInfo.GetAt(j);
			CString strFirstPayerID, strSecondPayerID;
			if (m_actClaimType == actInst) {
				strFirstPayerID = oiiCurPayer.strUBPayerID;
				strSecondPayerID = oiiPayerToCheck.strUBPayerID;
			}
			else {
				strFirstPayerID = oiiCurPayer.strHCFAPayerID;
				strSecondPayerID = oiiPayerToCheck.strHCFAPayerID;
			}
			if (strFirstPayerID.CompareNoCase(strSecondPayerID) == 0) {
				//These two insured parties have the same payer ID.
				//If one did not pay, and is also not on the bill, remove it.
				if (!oiiCurPayer.bHasPaid && oiiPayerToCheck.bHasPaid
					&& !oiiCurPayer.bIsOnBill) {
					//Remove oiiCurPayer. If oiiPayerToCheck is the one to be removed,
					//a future iteration of the i loop will catch it.
					aryOtherInsuredPartyInfo.RemoveAt(i);
					bRemoved = true;
				}
				else {
					//Do nothing.
					//The payer IDs match, but the CurPayer paid, so we cannot
					//remove it. Later we will iterate to PayerToCheck, and
					//remove it if it did not pay.

					//If both paid, we cannot remove either payer.
					//The client may get a rejection but we cannot control
					//this, it may just be a bad payer ID setup.
				}
			}
		}
	}

	CMap<SBR01Qualifier, SBR01Qualifier, bool, bool> mapQualifiers;

	//first cache the 2000B SBR01 qualifier
	mapQualifiers.SetAt(m_pEBillingInfo->sbr2000B_SBR01, true);

	//now recalculate the SBR01 qualifiers, as needed
	for (int i = 0; i < aryOtherInsuredPartyInfo.GetSize(); i++) {

		OtherInsuranceInfo oiiPayer = aryOtherInsuredPartyInfo.GetAt(i);
		bool bFound = false;
		bool bChanged = false;
		//if the qualifier exists, and is not U, increase it by one
		while (oiiPayer.sbr01Qual != SBR01Qualifier::sbrU
			&& mapQualifiers.Lookup(oiiPayer.sbr01Qual, bFound) && bFound) {

			oiiPayer.sbr01Qual = (SBR01Qualifier)((long)oiiPayer.sbr01Qual + 1);
			bFound = false;
			bChanged = true;
		}

		if (bChanged) {
			//update the array
			aryOtherInsuredPartyInfo.SetAt(i, oiiPayer);
		}

		//now cache the qualifier
		mapQualifiers.SetAt(oiiPayer.sbr01Qual, true);
	}
	
}

// (j.jones 2014-08-25 09:09) - PLID 54213 - returns "P" for Primary, "S" for secondary,
// or additional SBR01 qualifiers based on the RespTypeT.Priority
SBR01Qualifier CEbilling::CalculateSBR01Qualifier(bool bIs2000B, bool bSendingToPrimary, long nANSI_EnablePaymentInfo, long nPriority)
{
	//if this is for loop 2000B, default to P, else S
	SBR01Qualifier sbr01 = SBR01Qualifier::sbrP;
	if (!bIs2000B) {
		sbr01 = SBR01Qualifier::sbrS;
	}

	// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
	if (!bSendingToPrimary && nANSI_EnablePaymentInfo == 1) {

		//if this is for loop 2000B, default to S, else P
		sbr01 = SBR01Qualifier::sbrS;
		if (!bIs2000B) {
			sbr01 = SBR01Qualifier::sbrP;
		}

		//now try to map our priority to the right qualifier

		//technically the enumerations are these same values but for
		//clarity we will keep the case statement
		switch (nPriority) {
		case 1:
			//P - Primary
			sbr01 = SBR01Qualifier::sbrP;
			break;
		case 2:
			//S - Secondary
			sbr01 = SBR01Qualifier::sbrS;
			break;
		case 3:
			//T - Tertiary
			sbr01 = SBR01Qualifier::sbrT;
			break;
		case 4:
			//A - Payer Responsibility Four
			sbr01 = SBR01Qualifier::sbrA;
			break;
		case 5:
			//B - Payer Responsibility Five
			sbr01 = SBR01Qualifier::sbrB;
			break;
		case 6:
			//C - Payer Responsibility Six
			sbr01 = SBR01Qualifier::sbrC;
			break;
		case 7:
			//D - Payer Responsibility Seven
			sbr01 = SBR01Qualifier::sbrD;
			break;
		case 8:
			//E - Payer Responsibility Eight
			sbr01 = SBR01Qualifier::sbrE;
			break;
		case 9:
			//F - Payer Responsibility Nine
			sbr01 = SBR01Qualifier::sbrF;
			break;
		case 10:
			//G - Payer Responsibility Ten
			sbr01 = SBR01Qualifier::sbrG;
			break;
		case 11:
			//H - Payer Responsibility Eleven
			sbr01 = SBR01Qualifier::sbrH;
			break;
		default:
			//there is a U for unknown, but we never want to default to it,
			//so do not replace our default value
			break;
		}

		//sanity check - if the priority is not -1 and <= 11, the enum
		//should be the same value as the priority
		if (nPriority != -1 && nPriority <= 11) {
			ASSERT((long)sbr01 == nPriority);
		}
	}

	return sbr01;
}

// (j.jones 2014-08-25 10:16) - PLID 54213 - converts the SBR01Qualifier enum to a string
CString CEbilling::OutputSBR01Qualifier(SBR01Qualifier sbr)
{
	switch (sbr) {
	case sbrP:
		//P - Primary
		return "P";
	case sbrS:
		//S - Secondary
		return "S";
	case sbrT:
		//T - Tertiary
		return "T";
	case sbrA:
		//A - Payer Responsibility Four
		return "A";
	case sbrB:
		//B - Payer Responsibility Five
		return "B";
	case sbrC:
		//C - Payer Responsibility Six
		return "C";
	case sbrD:
		//D - Payer Responsibility Seven
		return "D";
	case sbrE:
		//E - Payer Responsibility Eight
		return "E";
	case sbrF:
		//F - Payer Responsibility Nine
		return "F";
	case sbrG:
		//G - Payer Responsibility Ten
		return "G";
	case sbrH:
		//H - Payer Responsibility Eleven
		return "H";
	default:
		//U - Unknown
		return "U";
	}
}

CString CEbilling::GetExportEventMessage()
{
	if (!m_bIsClearinghouseIntegrationEnabled)
	{
		return "Exporting to Disk";
	}
	else
	{
		return "Generating Export Files";
	}
}
 
/// <remarks>
/// Uploads the export file to the clearinghouse and deletes the export file and containing folder. This is a long operation. 
/// </remarks>
bool CEbilling::UploadClaimFile()
{
	try
	{
		CString strClaimFileContent = FileUtils::ReadAllText(m_ExportName);
		// Delete the export file. Its intended to be temporary.
		DeleteFile(m_ExportName);
		if (m_bDeleteExportDirectory)
		{
			FileUtils::Deltree(FileUtils::GetFileName(m_ExportName));
		}

		if (m_pClearinghouseLogin == nullptr)
		{
			ThrowNxException("%s: m_pClearinghouseLogin is null.", __FUNCTION__);
		}

		NexTech_Accessor::_ClaimFileInfoPtr pClaimFileInfo(__uuidof(NexTech_Accessor::ClaimFileInfo));
		pClaimFileInfo->fileName = _bstr_t(FileUtils::GetFileName(m_ExportName));
		pClaimFileInfo->FileContent = _bstr_t(strClaimFileContent);
		NexTech_Accessor::_UploadClaimFileResultPtr pResult = nullptr;
		{
			// This is going to be a long operation because its an HTTP request and involves an FTP connection.
			CWaitCursor waitCursor;
			pResult = GetAPI()->UploadClaimFile(GetAPISubkey(), GetAPILoginToken(), m_pClearinghouseLogin, pClaimFileInfo);
		}
		if (!pResult->Success)
		{
			MessageBox(FormatString("Failed to upload claim to clearinghouse. %s", (LPCTSTR)pResult->ErrorMessage), "Error", MB_ICONERROR | MB_OK);
			return false;
		}
		return true;
	} NxCatchAll(__FUNCTION__);

	return false;
}