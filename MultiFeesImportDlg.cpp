// MultiFeesImportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MultiFeesImportDlg.h"
#include "fileutils.h"
#include "internationalutils.h"
#include "audittrail.h"
#include "SoapUtils.h"
#include "MultiFeeImportFieldSelectionDlg.h"
#include "GlobalFinancialUtils.h"

#include "AdministratorRc.h"

#import "msxml.tlb"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (a.walling 2007-02-22 13:07) - PLID 2470 - Dialog allows importing and updating multi fees and standard fees
// from a simple .csv file. m_nFeeGroup should be set to the multifee group, and m_strFeeGroup to that group's name.
// If you set m_nFeeGroup to -1, then we will be modifying the ServiceT prices.

enum FeeListColumns {
	flcServiceID = 0,
	flcMultiFeeID,
	flcCode,
	flcDescription,
	flcCurFee,
	flcCurMultiFee,
	flcCurAllow,
	flcFee,
	flcAllow,
	flcFeeChange,
	flcAllowChange,
	flcNotes
};

/////////////////////////////////////////////////////////////////////////////
// CMultiFeesImportDlg dialog


CMultiFeesImportDlg::CMultiFeesImportDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMultiFeesImportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMultiFeesImportDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nFeeGroup = -1;
	m_bProcessCSV = TRUE;
	m_bProcessXML = FALSE;
	m_bFormatSelected = FALSE;
}


void CMultiFeesImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiFeesImportDlg)
	DDX_Control(pDX, IDC_FEE_INCLUDE_ALLOW, m_btnShowAllowable);
	DDX_Control(pDX, IDC_FEE_SHOW_DESCRIPTION, m_btnShowDesc);
	DDX_Control(pDX, IDC_FEE_SHOW_CHANGES, m_btnShowChanges);
	DDX_Control(pDX, IDC_FEE_FILE, m_nxeditFeeFile);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_FEE_GROUP, m_nxstaticFeeGroup);
	DDX_Control(pDX, IDC_RADIO_XML_FORMAT, m_radioXMLFormat);
	DDX_Control(pDX, IDC_RADIO_CSV_FORMAT, m_radioCSVFormat);
	DDX_Control(pDX, IDC_FORMAT_COLUMNS, m_btnFormatColumns);	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiFeesImportDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMultiFeesImportDlg)
	ON_BN_CLICKED(IDC_BROWSE_FEE_FILE, OnBrowseFeeFile)
	ON_BN_CLICKED(IDC_PROCESS_FEE_FILE, OnProcessFeeFile)
	ON_BN_CLICKED(IDC_FEE_SHOW_CHANGES, OnFeeShowChanges)
	ON_BN_CLICKED(IDC_FEE_SHOW_DESCRIPTION, OnFeeShowDescription)
	ON_EN_CHANGE(IDC_FEE_FILE, OnChangeFeeFile)
	ON_BN_CLICKED(IDOK, OnSaveChanges)
	ON_BN_CLICKED(IDC_FEE_INCLUDE_ALLOW, OnFeeIncludeAllow)
	ON_BN_CLICKED(IDC_FEE_IMPORT_HELP, OnFeeImportHelp)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_FORMAT_COLUMNS, &CMultiFeesImportDlg::OnBnClickedFormatColumns)
	ON_BN_CLICKED(IDC_RADIO_XML_FORMAT, &CMultiFeesImportDlg::OnBnClickedRadioXmlFormat)
	ON_BN_CLICKED(IDC_RADIO_CSV_FORMAT, &CMultiFeesImportDlg::OnBnClickedRadioCsvFormat)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiFeesImportDlg message handlers

void CMultiFeesImportDlg::OnBrowseFeeFile() 
{
	try {
		CString strDefExt;
		CString strFilter;
		if (m_bProcessCSV) {
			strDefExt = "csv";
			strFilter = "Comma Separated Values Files (*.csv;*.txt)|*.csv; *.txt|All Files (*.*)|*.*||";
		}
		else {
			strDefExt = "xml";
			strFilter = "XML Files (*.xml;)|*.xml; All Files (*.*)|*.*||";
		}

		// (j.armen 2011-10-25 14:07) - PLID 46137 - We are prompting the user where to open from, so the practice path is safe
		CFileDialog OpenFeeFile(TRUE, strDefExt, NULL, OFN_FILEMUSTEXIST, strFilter, this);
		// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
		CString dir = GetEnvironmentDirectory();
		OpenFeeFile.m_ofn.lpstrInitialDir = (LPCSTR)dir;
		if (OpenFeeFile.DoModal() != IDOK) {
			EndDialog(IDCANCEL);
			return;
		}

		m_strFile = OpenFeeFile.GetPathName();
		
		if (!FileUtils::DoesFileOrDirExist(m_strFile)) {
			m_strFile = "";
		} else {
			GetDlgItem(IDC_FEE_FILE)->SetWindowText(m_strFile);
			GetDlgItem(IDC_PROCESS_FEE_FILE)->EnableWindow(TRUE);
			if (m_bProcessXML) {
				GetDlgItem(IDC_FORMAT_COLUMNS)->EnableWindow(TRUE);
			}
		}

	} NxCatchAll("Error browsing for fee file in CMultiFeesImportDlg::OnBrowseFeeFile()");
}

BOOL CMultiFeesImportDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_dlList = BindNxDataList2Ctrl(this, IDC_FEE_REVIEW, GetRemoteData(), false);

	// (j.gruber 2009-10-22 16:00) - PLID 35632 - support XML import
	m_btnFormatColumns.AutoSet(NXB_MODIFY);

	//default to csv
	m_radioXMLFormat.SetCheck(0);
	m_radioCSVFormat.SetCheck(1);
	m_btnFormatColumns.EnableWindow(FALSE);
	
	// (z.manning, 04/30/2008) - PLID 29864 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	GetDlgItem(IDC_PROCESS_FEE_FILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_FORMAT_COLUMNS)->EnableWindow(FALSE);
#ifdef _DEBUG
	NXDATALIST2Lib::IColumnSettingsPtr pCol = m_dlList->GetColumn(flcServiceID);

	pCol->PutStoredWidth(50);
#endif

	if (m_nFeeGroup <= -2) {
		// this is an error state; we must be passed in a valid fee group ID, or -1 for standard fees.
		EndDialog(IDCANCEL);
		return TRUE;
	} else if (m_nFeeGroup == -1) {
		m_bStandard = true;

		CheckDlgButton(IDC_FEE_INCLUDE_ALLOW, FALSE);
		GetDlgItem(IDC_FEE_INCLUDE_ALLOW)->ShowWindow(SW_HIDE);

		GetDlgItem(IDC_FEE_SHOW_CHANGES)->SetWindowText("Show Changes in Fee");

		SetWindowText("Standard Fee Import and Update");

	} else {
		m_bStandard = false;
	}

	if (m_nFeeGroup >= 0) {
		// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
		GetDlgItem(IDC_FEE_GROUP)->SetWindowText(FormatString("Using fee schedule: '%s'", m_strFeeGroup));
	} else {
		GetDlgItem(IDC_FEE_GROUP)->SetWindowText("Updating standard fees");
	}

	m_bDataReady = false;


	long nShowAllowed = GetRemotePropertyInt("MultiFeeImportIncludeAllow", 1, 0, GetCurrentUserName(), true);
	if ( (nShowAllowed == 1) && !m_bStandard) {
		CheckDlgButton(IDC_FEE_INCLUDE_ALLOW, TRUE);
		ShowAllowed(true);
	} else {
		CheckDlgButton(IDC_FEE_INCLUDE_ALLOW, FALSE);
		ShowAllowed(false);
	}

	long nShowChanges = GetRemotePropertyInt("MultiFeeImportShowChanges", 0, 0, GetCurrentUserName(), true);
	if (nShowChanges == 1) {
		CheckDlgButton(IDC_FEE_SHOW_CHANGES, TRUE);
		ShowChanges(true);
	} else {
		CheckDlgButton(IDC_FEE_SHOW_CHANGES, FALSE);
		ShowChanges(false);
	}

	long nShowDescription = GetRemotePropertyInt("MultiFeeImportShowDescription", 1, 0, GetCurrentUserName(), true);
	if (nShowDescription == 1) {
		CheckDlgButton(IDC_FEE_SHOW_DESCRIPTION, TRUE);
		ShowDescription(true);
	} else {
		CheckDlgButton(IDC_FEE_SHOW_DESCRIPTION, FALSE);
		ShowDescription(false);
	}
	
	GetDlgItem(IDOK)->EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMultiFeesImportDlg::OnProcessFeeFile() 
{
	CWaitCursor wc;

	GetDlgItem(IDC_PROCESS_FEE_FILE)->EnableWindow(FALSE);
	GetDlgItem(IDC_FORMAT_COLUMNS)->EnableWindow(FALSE);

	// (j.gruber 2009-10-22 16:41) - PLID 35632 - check to see if they are using xml and have picked their columns
	if (m_bProcessXML && !m_bFormatSelected) {
		MessageBox("You must choose which fields in the XML file you would like to use by clicking the 'Select Fields' button before processing.");
		GetDlgItem(IDC_PROCESS_FEE_FILE)->EnableWindow(TRUE);
		GetDlgItem(IDC_FORMAT_COLUMNS)->EnableWindow(TRUE);
		return;
	}
	
	CStdioFile file;
	try {
		if (m_strFile.IsEmpty()) {
			return;	
		}

		if (!file.Open(m_strFile, CFile::modeRead | CFile::shareDenyWrite)) {
			MessageBox(FormatString("Could not open the file '%s'", m_strFile), "Practice", MB_OK | MB_ICONEXCLAMATION);
			return;
		}
		
	} NxCatchAll("Error opening file in CMultiFeesImportDlg::OnProcessFeeFile()");

	try {
		CString strLine;
		// (d.thompson 2010-09-15) - PLID 40548 - Must initialize variable!
		long nLine = 0;

		m_dlList->Clear();
		m_bDataReady = false;
		
		GetDlgItem(IDOK)->EnableWindow(FALSE);

		if (m_bProcessCSV) {

			while (file.ReadString(strLine)) {
				nLine++;

				if (!strLine.IsEmpty()) {
					// we have a line from the file!
					
					CString strCode, strFee, strAllow;
					
					long nDelimIndex = strLine.Find(",", 0);
					if (nDelimIndex >= 0) {
						strCode = strLine.Left(nDelimIndex);
						strCode.TrimLeft("\""); strCode.TrimLeft("'");
						strCode.TrimRight("\""); strCode.TrimRight("'");

						if (!strCode.IsEmpty()) {
							long nNextDelimIndex = strLine.Find(",", nDelimIndex + 1);
							if (nNextDelimIndex >= nDelimIndex + 1) {
								strFee = strLine.Mid(nDelimIndex + 1, (nNextDelimIndex - nDelimIndex - 1));
								strFee.TrimLeft("\""); strFee.TrimLeft("'");
								strFee.TrimRight("\""); strFee.TrimRight("'");

								long nFinalDelimIndex = strLine.Find(",", nNextDelimIndex + 1);

								if (nFinalDelimIndex == -1)
									nFinalDelimIndex = strLine.GetLength();

								if (nFinalDelimIndex >= nNextDelimIndex + 1) {
									strAllow = strLine.Mid(nNextDelimIndex + 1, (nFinalDelimIndex - nNextDelimIndex - 1));
									strAllow.TrimLeft("\""); strAllow.TrimLeft("'");
									strAllow.TrimRight("\""); strAllow.TrimRight("'");

									SetOneLine(strCode, strFee, strAllow);

								} else {
									// bad data
								}

							} else {
								// bad data
							}
						}
					} else {
						// bad data
					}

					if (rand() % 5 == 0) // using a random value is more realistic feeling than a static value. try and see!
						GetDlgItem(IDC_FEE_REVIEW)->RedrawWindow();
				}
			}

		}
		else {
			//processing XML
			ProcessXMLFile();
		}

		m_bDataReady = true;
		GetDlgItem(IDOK)->EnableWindow(TRUE);
	} NxCatchAll("Error loading fees in CMultiFeesImportDlg::OnProcessFeeFile()");

	file.Close();
	GetDlgItem(IDC_PROCESS_FEE_FILE)->EnableWindow(TRUE);
	if (m_bProcessXML) {
		GetDlgItem(IDC_FORMAT_COLUMNS)->EnableWindow(TRUE);
	}
}


// (j.gruber 2009-10-26 12:00) - PLID 35632 - moved some processing so both parsers could use it
void CMultiFeesImportDlg::SetOneLine(CString strCode, CString strFee, CString strAllow) {

	COleCurrency cyNull;
	cyNull.SetStatus(COleCurrency::null);

	COleCurrency cyFee, cyAllow;
	cyAllow.ParseCurrency(strAllow);
	cyFee.ParseCurrency(strFee);
	// (b.eyers 2015-11-06) - PLID 34061 - round these both before saving
	RoundCurrency(cyAllow);
	RoundCurrency(cyFee);
									
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlList->GetNewRow();

	pRow->PutValue(flcCode, AsVariant(strCode));

	bool bValidFee, bValidAllow;

	if ( strFee.IsEmpty() || (cyFee.GetStatus() != COleCurrency::valid)) {
		cyFee.SetStatus(COleCurrency::null);
		pRow->PutValue(flcFee, g_cvarNull);
		bValidFee = false;
	} else {
		pRow->PutValue(flcFee, _variant_t(cyFee));
		bValidFee = true;
	}

	if ( strAllow.IsEmpty() || (cyAllow.GetStatus() != COleCurrency::valid)) {
		cyAllow.SetStatus(COleCurrency::null);
		pRow->PutValue(flcAllow, g_cvarNull);
		bValidAllow = false;
	} else {
		pRow->PutValue(flcAllow, _variant_t(cyAllow));
		bValidAllow = true;
	}

	CString strSql;
	
	if (m_bStandard) {
		strSql = FormatString(
			"SELECT ServiceT.Name, NULL AS MultiFeeID, ServiceT.ID AS ServiceCodeID, ServiceT.Active, "
			"CASE WHEN ServiceT.Anesthesia = 1 AND ServiceT.UseAnesthesiaBilling = 1 THEN 1 ELSE 0 END AS IsAnesCode, "
			"CASE WHEN ServiceT.FacilityFee = 1 AND ServiceT.UseFacilityBilling = 1 THEN 1 ELSE 0 END AS IsFacCode, "
			"NULL AS MultiAllowable, ServiceT.Price AS Price, NULL AS MultiPrice "
			"FROM CPTCodeT "
			"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
			"WHERE ServiceT.ID NOT IN (SELECT ID FROM ProductT) "
			"AND CPTCodeT.Code = '%s'", _Q(strCode));
	} else {
		strSql = FormatString(
			"SELECT ServiceT.Name, MultiFeeItemsT.ServiceID AS MultiFeeID, ServiceT.ID AS ServiceCodeID, ServiceT.Active, "
			"CASE WHEN ServiceT.Anesthesia = 1 AND ServiceT.UseAnesthesiaBilling = 1 THEN 1 ELSE 0 END AS IsAnesCode, "
			"CASE WHEN ServiceT.FacilityFee = 1 AND ServiceT.UseFacilityBilling = 1 THEN 1 ELSE 0 END AS IsFacCode, "
			"MultiFeeItemsT.Allowable AS MultiAllowable, COALESCE(MultiFeeItemsT.Price, ServiceT.Price) AS Price, MultiFeeItemsT.Price as MultiPrice "
			"FROM CPTCodeT "
			"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
			"LEFT JOIN MultiFeeItemsT ON ServiceID = ServiceT.ID AND FeeGroupID = %li "
			"WHERE ServiceT.ID NOT IN (SELECT ID FROM ProductT) "
			"AND CPTCodeT.Code = '%s'", m_nFeeGroup, _Q(strCode));
	}

	ADODB::_RecordsetPtr prs = CreateRecordsetStd(strSql);

	bool bExists = prs->eof ? false : true;

	bool bIsAnes = false;
	bool bIsFac = false;
	bool bIsActive = true;

	long nServiceCodeID = bExists ? AdoFldLong(prs, "ServiceCodeID") : -1;

	// basically we are filling in everything possible and making it NULL if not. This way, we can easily tell a row
	// to be skipped by NULLifying its ServiceID. Also we can tell to update an item instead of insert by checking for
	// the existence of MulitFeeItems' ServiceID, which is the same serviceID, but will be null if it does not already
	// exist in the group.

	if (bExists) {
		bIsAnes = AdoFldLong(prs, "IsAnesCode", 0) == 1;
		bIsFac = AdoFldLong(prs, "IsFacCode", 0) == 1;
		bIsActive = AdoFldBool(prs, "Active", FALSE) == TRUE;

		COleCurrency cyCurFee, cyCurAllow, cyMultiPrice;

		cyCurFee = AdoFldCurrency(prs, "Price", cyNull);
		cyCurAllow = AdoFldCurrency(prs, "MultiAllowable", cyNull);
		cyMultiPrice = AdoFldCurrency(prs, "MultiPrice", cyNull);

		pRow->PutValue(flcServiceID, AdoFldLong(prs, "ServiceCodeID"));

		pRow->PutValue(flcDescription, AsVariant(AdoFldString(prs, "Name", "")));

		long nMultiFeeID = AdoFldLong(prs, "MultiFeeID", -1);
		if (nMultiFeeID >= 0) {
			pRow->PutValue(flcMultiFeeID, _variant_t(nMultiFeeID, VT_I4));
		} else {
			pRow->PutValue(flcMultiFeeID, g_cvarNull);
		}

		if (cyCurFee.GetStatus() != COleCurrency::null)
			pRow->PutValue(flcCurFee, _variant_t(cyCurFee));
		else 
			pRow->PutValue(flcCurFee, g_cvarNull);

		if (cyCurAllow.GetStatus() != COleCurrency::null)
			pRow->PutValue(flcCurAllow, _variant_t(cyCurAllow));
		else
			pRow->PutValue(flcCurAllow, g_cvarNull);

		if (cyMultiPrice.GetStatus() != COleCurrency::null)
			pRow->PutValue(flcCurMultiFee, _variant_t(cyMultiPrice));
		else
			pRow->PutValue(flcCurMultiFee, g_cvarNull);

		// Fees should never initially be null. So we have three real scenarios:
		// 100 -> NULL = (if standard, ignore. if multifee, set to NULL, so change is back to the <standard fee>)
		// 100 -> 200  = (change is 100)
		// 100 -> 100  = (change is NULL)
		COleCurrency cyChange;
		if (cyFee.GetStatus() == COleCurrency::valid) {
			// new fee is not null
			if (cyFee == cyCurFee) {
				// same values, no change
				pRow->PutValue(flcFeeChange, g_cvarNull);
			} else {
				cyChange = cyFee - cyCurFee;
				pRow->PutValue(flcFeeChange, _variant_t(cyChange));
			}
		} else {
			// new fee is null
			// if standard, ignore. if multifee, set to NULL, so change is back to the <standard fee>
			if (m_bStandard) {
				pRow->PutValue(flcFeeChange, g_cvarNull);
			} else {
				_variant_t varMultiPrice = prs->Fields->Item["MultiPrice"]->Value;

				if (varMultiPrice.vt == VT_CY) {
					// this is an existing entry in the multifee, so it will be removed and become standard fee
					pRow->PutValue(flcFeeChange, AsVariant("<standard fee>"));
				} else {
					// well the fee that is displayed IS the standard fee, so ignore.
					pRow->PutValue(flcFeeChange, g_cvarNull);
				}
			}
		}

		// However, allows can be NULL or n; so we have more scenarios:
		// NULL -> 100 = (Change is 100)
		// 100 -> 200 = (Change is 100)
		// 100 -> 100 = (Change is NULL)
		// 100 -> NULL = (Change is apparently infinite. Let's write <removed>)
		// NULL -> NULL = (Change is NULL)

		if (cyAllow.GetStatus() == COleCurrency::valid) {
			// new allow is not null
			if (cyCurAllow.GetStatus() == COleCurrency::valid) {
				// existing allow is not null
				if (cyAllow == cyCurAllow) {
					// no change, so null change.
					pRow->PutValue(flcAllowChange, g_cvarNull);
				} else {
					pRow->PutValue(flcAllowChange, _variant_t(cyAllow - cyCurAllow));
				}
			} else {
				// existing allow is null, so the change is simply the new allow amount
				pRow->PutValue(flcAllowChange, _variant_t(cyAllow));
			}
		} else {
			// new allow is null
			if (cyCurAllow.GetStatus() == COleCurrency::valid) {
				// change is apparently infinite since the limit is being lifted.
				pRow->PutValue(flcAllowChange, AsVariant("<removed>"));
			} else {
				// both null, no change.
				pRow->PutValue(flcAllowChange, g_cvarNull);
			}
		}
	} else {
		pRow->PutValue(flcServiceID, g_cvarNull);
		pRow->PutValue(flcMultiFeeID, g_cvarNull);
		pRow->PutValue(flcCurFee, g_cvarNull);
		pRow->PutValue(flcCurMultiFee, g_cvarNull);
		pRow->PutValue(flcCurAllow, g_cvarNull);
		pRow->PutValue(flcFeeChange, g_cvarNull);
		pRow->PutValue(flcAllowChange, g_cvarNull);
		pRow->PutValue(flcServiceID, g_cvarNull);
	}

	CString strMsg;

	NXDATALIST2Lib::IRowSettingsPtr pOtherRow;
	if (nServiceCodeID >= 0)
	pOtherRow = m_dlList->FindByColumn(flcServiceID, nServiceCodeID, NULL, VARIANT_FALSE);

	// we set the service id to null to prevent these from being processed when saving
	if (pOtherRow) {
		strMsg = "Duplicate code!\r\n(will be skipped)";
		pRow->PutForeColor(RGB(128,0,0));
		pRow->PutValue(flcServiceID, g_cvarNull);
	/*} else if (!bValidFee && !bValidAllow) {
		strMsg = "No new values in file!\r\n(will be skipped)";
		pRow->PutForeColor(RGB(128,0,0));
		pRow->PutValue(flcServiceID, g_cvarNull);*/
	// we used to skip if the fee and allow were both null,
	// but that is legal, assuming that a multi fee already exists.
	} else if (!bExists) {
		strMsg = "Not found!\r\n(will be skipped)";
		pRow->PutForeColor(RGB(128,0,0));
	} else {
		if (!bIsActive) {
			strMsg = "Inactive\r\n(will be updated).";
			pRow->PutForeColor(RGB(64,128,128));
		} else if (bIsAnes) {
			strMsg = "Anesthesia code\r\n(will be skipped).";
			pRow->PutForeColor(RGB(128,128,128));
			pRow->PutValue(flcServiceID, g_cvarNull);
		} else if (bIsFac) {
			strMsg = "Facility code\r\n(will be skipped).";
			pRow->PutForeColor(RGB(128,128,128));
			pRow->PutValue(flcServiceID, g_cvarNull);
		}
	}	

	pRow->PutValue(flcNotes, AsVariant(strMsg));
	
	m_dlList->AddRowSorted(pRow, NULL);


}

void CMultiFeesImportDlg::ProcessXMLFile() {

	//if we got here, we already have our fields

	try {

		CWaitCursor pWait;

		CString strParentTemp = m_strParentNode;
		CString strCodeTemp = m_strCodeField;
		CString strFeeTemp = m_strFeeField;
		CString strAllowableTemp = m_strAllowableField;
		CString strCode, strFee, strAllowable;
		BOOL bShowAllowable = m_nFeeGroup != -1;

		//we can trim the three fields by the parent since we know they all include it
		strCodeTemp.TrimLeft(strParentTemp + "->");
		strFeeTemp.TrimLeft(strParentTemp + "->");
		strAllowableTemp.TrimLeft(strParentTemp + "->");
		//open the document
		MSXML::IXMLDOMDocumentPtr pXMLDoc(__uuidof(MSXML2::DOMDocument60));
		
		if (pXMLDoc->load(_variant_t(m_strFile)) == VARIANT_TRUE) {
			
			MSXML::IXMLDOMNodeListPtr pRootNodes = pXMLDoc->GetchildNodes();
			for (int i = 0; i < pRootNodes->Getlength(); i++) {

				//we know we have a parent that is the same for all fields, because otherwise how could we correlate them together, so find the parent
				MSXML::IXMLDOMNodePtr pRootNode = pRootNodes->Getitem(i);
				if (pRootNode) {

					if (pRootNode->childNodes->length != 0) {

						strParentTemp = m_strParentNode;
						strCodeTemp = m_strCodeField;
						strFeeTemp = m_strFeeField;
						strAllowableTemp = m_strAllowableField;
						strCode, strFee, strAllowable;
						
						//we can trim the three fields by the parent since we know they all include it
						strCodeTemp.TrimLeft(strParentTemp + "->");
						strFeeTemp.TrimLeft(strParentTemp + "->");
						strAllowableTemp.TrimLeft(strParentTemp + "->");
				
						CString strParentTop = GetNodeName(strParentTemp, strParentTemp);
						MSXML2::IXMLDOMNodePtr pParent = pRootNode;
						//check if we were on the first node already
						if (strParentTemp.IsEmpty()) {
							pParent = FindChildNode((MSXML2::IXMLDOMNode *)pParent, strParentTop);
						}
						else {
							while (!strParentTemp.IsEmpty()) {
								pParent = FindChildNode((MSXML2::IXMLDOMNode *)pParent, strParentTop);
								strParentTop = GetNodeName(strParentTemp, strParentTemp);
							}
						}

						//now that we have our parent, we can start filling our values
						while (pParent != NULL) {
							strCode = strFee = strAllowable = "";

							strCode = GetValue(pParent, strCodeTemp);
							strFee = GetValue(pParent, strFeeTemp);
							if (bShowAllowable) {
								strAllowable = GetValue(pParent, strAllowableTemp);
							}

							SetOneLine(strCode, strFee, strAllowable);						

							pParent = pParent->GetnextSibling();

							//we'll use the same technique the cpt codes do for updating
							if (rand() % 5 == 0) // using a random value is more realistic feeling than a static value. try and see!
								GetDlgItem(IDC_FEE_REVIEW)->RedrawWindow();
						}
					}
				}

			}
		}

	}NxCatchAll(__FUNCTION__);
}

CString CMultiFeesImportDlg::GetNodeName(CString strPath, CString &strRemainingPath)
{
	long nResult = strPath.Find("->");
	if (nResult == -1) {
		strRemainingPath = "";
		return strPath;
	}
	else {
		strRemainingPath = strPath.Right(strPath.GetLength() - (nResult + 2));
		return strPath.Left(nResult);
	}
}

CString CMultiFeesImportDlg::GetValue(MSXML2::IXMLDOMNodePtr pParent, CString strPath) 
{
	
	while (!strPath.IsEmpty()) {

		CString strCurrentNode = GetNodeName(strPath, strPath);
		pParent = FindChildNode((MSXML2::IXMLDOMNode *)pParent, strCurrentNode);

		//we can't find the parent, must not exist in this data
		if (pParent == NULL) {
			return "";
		}
	}
	//TRACE((LPCTSTR)pParent->GetnodeName());
	//TRACE(pParent->nodeValue.bstrVal);
	CString strReturn = (LPCTSTR)pParent->text;

	return strReturn;
}




void CMultiFeesImportDlg::OnFeeShowChanges() 
{
	SetRemotePropertyInt("MultiFeeImportShowChanges", (IsDlgButtonChecked(IDC_FEE_SHOW_CHANGES) == TRUE) ? 1 : 0, 0, GetCurrentUserName());
	ShowChanges(IsDlgButtonChecked(IDC_FEE_SHOW_CHANGES) == TRUE);
}

void CMultiFeesImportDlg::ShowChanges(bool bShow)
{
	NXDATALIST2Lib::IColumnSettingsPtr pFeeChangeCol = m_dlList->GetColumn(flcFeeChange);
	NXDATALIST2Lib::IColumnSettingsPtr pAllowChangeCol = m_dlList->GetColumn(flcAllowChange);

	if (bShow) {
		pFeeChangeCol->PutColumnStyle(NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csWidthAuto);
		if (IsDlgButtonChecked(IDC_FEE_INCLUDE_ALLOW))
			pAllowChangeCol->PutColumnStyle(NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csWidthAuto);
	} else {
		pFeeChangeCol->PutColumnStyle(NXDATALIST2Lib::csFixedWidth);
		pFeeChangeCol->PutStoredWidth(0);
		pAllowChangeCol->PutColumnStyle(NXDATALIST2Lib::csFixedWidth);
		pAllowChangeCol->PutStoredWidth(0);
	}
}

void CMultiFeesImportDlg::OnFeeShowDescription() 
{
	SetRemotePropertyInt("MultiFeeImportShowDescription", (IsDlgButtonChecked(IDC_FEE_SHOW_DESCRIPTION) == TRUE) ? 1 : 0, 0, GetCurrentUserName());
	ShowDescription(IsDlgButtonChecked(IDC_FEE_SHOW_DESCRIPTION) == TRUE);
}

void CMultiFeesImportDlg::ShowDescription(bool bShow)
{
	NXDATALIST2Lib::IColumnSettingsPtr pDescriptionCol = m_dlList->GetColumn(flcDescription);

	if (bShow) {
		pDescriptionCol->PutColumnStyle(NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csWidthPercent);
		pDescriptionCol->PutStoredWidth(20);
	} else {
		pDescriptionCol->PutColumnStyle(NXDATALIST2Lib::csFixedWidth);
		pDescriptionCol->PutStoredWidth(0);
	}
}

CString CMultiFeesImportDlg::GenerateSaveSql(OUT long &nAuditID)
{
	try {
		CString strSql = BeginSqlBatch();
		nAuditID = BeginAuditTransaction();

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlList->GetFirstRow();
		_variant_t var;

		while (pRow) {
			var = pRow->GetValue(flcServiceID);
			bool bCommitString = false;
			CString strStatement;

			if (var.vt == VT_I4) {
				// the service id is not empty or null
				long nServiceID = VarLong(var);
				long nMultiFeeID = VarLong(pRow->GetValue(flcMultiFeeID), -1);

				_variant_t varFee, varAllow;
				_variant_t varCurFee, varCurAllow, varCurMultiFee;

				varCurFee = pRow->GetValue(flcCurFee);
				varCurAllow = pRow->GetValue(flcCurAllow);
			
				varCurMultiFee = pRow->GetValue(flcCurMultiFee);

				varFee = pRow->GetValue(flcFee);
				varAllow = pRow->GetValue(flcAllow);

				if (m_bStandard) {
					// update the ServiceT table directly. possible scenarios:
					// 100 -> NULL = ignore
					// 100 -> 100 = no change
					// 100 -> 200 = set to 200
					// NULL -> NULL = no change
					// NULL -> 100 = no change (we should not have NULL fees; so don't mess with it)

					if (varCurFee.vt == VT_CY) {
						if (varFee.vt == VT_CY) {
							// both fees are not null
							if (VarCurrency(varFee) != VarCurrency(varCurFee)) {
								// Fee has changed, so write and audit.
								bCommitString = true;
								strStatement = FormatString("UPDATE ServiceT SET Price = %s WHERE ID = %li", 
									FormatCurrencyForSql(VarCurrency(varFee)), nServiceID);
								
								CString strOld, strNew;

								strOld = FormatCurrencyForInterface(VarCurrency(varCurFee), TRUE, TRUE);
								strNew = FormatCurrencyForInterface(VarCurrency(varFee), TRUE, TRUE);

								ASSERT(strNew != strOld);

								// audit the change
								AuditEvent(-1, "", nAuditID, aeiCPTFee, nServiceID, strOld, strNew, aepHigh, aetChanged);
							} else {
								// values are equal, so don't audit or write.
								bCommitString = false;
							}
						} else {
							// the new fee is null. we will ignore.
							bCommitString = false;
						}
					} else {
						// the current fee is null; we don't touch it.
						bCommitString = false;
					}

				} else if (nMultiFeeID >= 0) {
					bCommitString = true;

					// a valid multifee ID means we should update rather than insert.
					// not that CurFee is updated to be either the ServiceT price or the
					// multifeeitemst price. So use varCurMultiFee if you want to know
					// specifically which one.


					// fee scenarios:
					// NULL -> 100 = set to 100
					// 100 -> 200 = set to 200
					// 100 -> 100 = ignore
					// 100 -> NULL = set to NULL / remove special fee
					// NULL -> NULL = ignore
					CString strSet, strFee, strAllow;
					
					bool bFeeChanged = false;
					bool bNullFee = false;

					if (varFee.vt == VT_CY) {
						// new fee is not null
						if (varCurMultiFee.vt == VT_CY) {
							// existing fee is not null
							if (VarCurrency(varFee) == VarCurrency(varCurMultiFee)) {
								// same values, so ignore.
								strFee = "Price";
							} else {
								// values have changed!
								strFee = FormatCurrencyForSql(VarCurrency(varFee));
								bFeeChanged = true;
							}
						} else {
							// existing fee is null
							strFee = FormatCurrencyForSql(VarCurrency(varFee));
							bFeeChanged = true;
						}
					} else {
						// new fee is null..
						if (varCurMultiFee.vt == VT_CY) {
							// existing fee is not null
							strFee = "NULL"; // remove the special fee
							bNullFee = true;
							bFeeChanged = true;
						} else {
							// existing fee is null. so we ignore.
							strFee = "Price"; // this is effectively ignoring, setting Price = Price
						}
					}


					// allow scenarios:
					// NULL -> 100 = set to 100
					// NULL -> NULL = ignore
					// 100 -> 100 = ignore
					// 100 -> 200 = set to 200
					// 100 -> NULL = set to NULL / remove allow

					bool bNullAllow = false;

					bool bAllowChanged = false;
					if (IsDlgButtonChecked(IDC_FEE_INCLUDE_ALLOW)) { // only modify the allowed amount if we should
						if (varAllow.vt == VT_CY) {
							// new allowable is not null
							if (varCurAllow.vt == VT_CY) {
								// existing allowable is not null
								if (VarCurrency(varAllow) == VarCurrency(varCurAllow)) {
									// same values, so ignore.
									strAllow = "Allowable";
								} else {
									// values have changed!
									strAllow = FormatCurrencyForSql(VarCurrency(varAllow));
									bAllowChanged = true;
								}
							} else {
								// existing allowable is null
								strAllow = FormatCurrencyForSql(VarCurrency(varAllow));
								bAllowChanged = true;
							}
						} else {
							// new allowable is null..
							if (varCurAllow.vt == VT_CY) {
								// existing allowable is not null
								strAllow = "NULL"; // remove the allowed amount
								bNullAllow = true;
								bAllowChanged = true;
							} else {
								// existing allowable is null. so we ignore.
								strAllow = "Allowable"; // this is effectively ignoring, setting Allowable = Allowable
							}
						}
					} else {
						// don't mess with the allowed amount.
						strAllow = "Allowable";
					}

					if (bNullFee && bNullAllow) {
						// if these are both set to null, there is no point in having a multifee entry for them.
						strStatement = FormatString("DELETE FROM MultiFeeItemsT WHERE FeeGroupID = %li AND ServiceID = %li", m_nFeeGroup, nMultiFeeID);
					} else {
						strStatement = FormatString("UPDATE MultiFeeItemsT SET Price = %s, Allowable = %s WHERE FeeGroupID = %li AND ServiceID = %li", strFee, strAllow, m_nFeeGroup, nMultiFeeID);
					}

					bCommitString = bAllowChanged || bFeeChanged;
					
				} else {
					bCommitString = true;
					// insert a new row.
					// much simpler than updating. 1 to 1 correspondence here.
					// if the new fee is null, insert null. otherwise insert the new fee.
					// if the new allow is null, insert null. otherwise insert the new allowed amount.
					CString strValues, strFee, strAllow;

					bool bNullFee = false, bNullAllow = false;

					if (varFee.vt == VT_CY) {
						strFee = FormatCurrencyForSql(VarCurrency(varFee));
					} else {
						strFee = "NULL";
						bNullFee = true;
					}
					

					if (IsDlgButtonChecked(IDC_FEE_INCLUDE_ALLOW)) { // only insert the allowed amount if we should
						if ((varAllow.vt == VT_CY) && IsDlgButtonChecked(IDC_FEE_INCLUDE_ALLOW)) {
							strAllow = FormatCurrencyForSql(VarCurrency(varAllow));
						} else {
							strAllow = "NULL";
							bNullAllow = true;
						}
					} else {
						strAllow = "NULL"; // we are creating a new record, so setting this to null means there is no special allowable.
						bNullAllow = true;
					}

					if (bNullFee && bNullAllow) {
						// both fields are null; this doesn't make much sense inserting, since if we did this on the
						// multifees dialog it would remove the row. Therefore, don't add this row.
						bCommitString = false;
					} else {
						strStatement = FormatString("INSERT INTO MultiFeeItemsT(FeeGroupID, ServiceID, Price, Allowable) VALUES(%li, %li, %s, %s)", m_nFeeGroup, nServiceID, strFee, strAllow);	
					}
				}

				if (bCommitString) {
					AddStatementToSqlBatch(strSql, strStatement);
				}
			}

			pRow = pRow->GetNextRow();
		}

		if (!m_bStandard) 
			AuditEvent(-1, "", nAuditID, aeiUpdateMultiFees, m_nFeeGroup, FormatString("Updating %s", m_strFeeGroup), FormatString("From file %s", m_strFile), aepHigh, aetChanged);

		return strSql;
	} NxCatchAll ("Error in CMultiFeesImportDlg::GenerateSaveSql()");

	return "";
}

void CMultiFeesImportDlg::OnChangeFeeFile() 
{
	GetDlgItem(IDC_PROCESS_FEE_FILE)->EnableWindow(TRUE);
	GetDlgItem(IDC_FEE_FILE)->GetWindowText(m_strFile);
}

void CMultiFeesImportDlg::OnSaveChanges() 
{
	ASSERT(m_bDataReady);
	if (!m_bDataReady) {
		return;
	}

	long nAuditID;
	CString strSql = GenerateSaveSql(nAuditID);
	if (strSql.IsEmpty()) {
		MessageBox("There are no records to update!", "Practice", MB_OK | MB_ICONHAND);
		RollbackAuditTransaction(nAuditID);
		CDialog::OnOK();
		return;
	}

	try {
#ifdef _DEBUG
		if (IDCANCEL == MessageBox(strSql, "Practice", MB_OKCANCEL))
			return;
#endif
		ExecuteSqlStd(strSql);
		CommitAuditTransaction(nAuditID);

	}NxCatchAllCall("There was an error updating the data. It is possible that the data changed while you were reviewing it, "
			"which may have caused an invalid state. Please try again when the network is less busy, or call Nextech Support.",
		RollbackAuditTransaction(nAuditID)); // remove this transaction from memory

	
	CDialog::OnOK();
}

void CMultiFeesImportDlg::OnFeeIncludeAllow() 
{
	SetRemotePropertyInt("MultiFeeImportIncludeAllow", (IsDlgButtonChecked(IDC_FEE_INCLUDE_ALLOW) == TRUE) ? 1 : 0, 0, GetCurrentUserName());
	ShowAllowed(IsDlgButtonChecked(IDC_FEE_INCLUDE_ALLOW) == TRUE);
}

void CMultiFeesImportDlg::ShowAllowed(bool bShow)
{
	NXDATALIST2Lib::IColumnSettingsPtr pAllowCol = m_dlList->GetColumn(flcAllow);
	NXDATALIST2Lib::IColumnSettingsPtr pCurAllowCol = m_dlList->GetColumn(flcCurAllow);
	NXDATALIST2Lib::IColumnSettingsPtr pAllowChangeCol = m_dlList->GetColumn(flcAllowChange);

	if (bShow) {
		pAllowCol->PutColumnStyle(NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csWidthAuto);
		pCurAllowCol->PutColumnStyle(NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csWidthAuto);
		pAllowChangeCol->PutColumnStyle(NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csWidthAuto);
	} else {
		pAllowCol->PutColumnStyle(NXDATALIST2Lib::csFixedWidth);
		pAllowCol->PutStoredWidth(0);
		pCurAllowCol->PutColumnStyle(NXDATALIST2Lib::csFixedWidth);
		pCurAllowCol->PutStoredWidth(0);
		pAllowChangeCol->PutColumnStyle(NXDATALIST2Lib::csFixedWidth);
		pAllowChangeCol->PutStoredWidth(0);
	}
}

void CMultiFeesImportDlg::OnFeeImportHelp() 
{
	CString strMessage;
	
	// (j.gruber 2009-10-27 14:11) - PLID 35632 - change help if on xml
	// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
	if (m_bProcessCSV) {
		strMessage = "The file to import is a simple list of codes, prices, and allowables, in that order. "
		"Only comma-separated-values (CSV) files are supported. These can be created from Excel and most other "
		"data programs. Ensure that the first three columns of data are, IN ORDER, 'code', 'price', 'allowable'. "
		"Then save as a CSV file. Allowable values can be blank or not exist at all, and there can be extra "
		"columns following (they will be ignored). Blank values in the price will be ignored; blank values in "
		"the allowable will remove that allowable for the given fee schedule. If you do not use fee schedules, the "
		"allowable column will be ignored."
		"\r\n\r\n"
		"Please call Nextech Support if you have further questions.";
	}
	else {
		strMessage = "The file to import is an XML file that includes service codes, prices, and allowables. "
		"Select the XML file that you would like to import and then click the 'Select Fields' button in order "
		"to choose which elements in the XML document correspond to the service code, fee, and allowable fields. "
		"Allowable values can be blank or not exist at all. Blank values in the price will be ignored; blank values in "
		"the allowable will remove that allowable for the given fee schedule. If you do not use fee schedules, the "
		"allowable column will be ignored."
		"\r\n\r\n"
		"Please call Nextech Support if you have further questions.";
	}

	MessageBox(strMessage, "Practice", MB_OK | MB_ICONINFORMATION);
}

// (j.gruber 2009-10-27 14:12) - PLID 35632 - added xml support
void CMultiFeesImportDlg::OnBnClickedFormatColumns()
{
	try {
		BOOL bUpdateAllowable = m_nFeeGroup != -1;
		if (m_strFile.IsEmpty()) {
			MsgBox("Please enter a filename before trying to select the fields");
			return;
		}
		CMultiFeeImportFieldSelectionDlg dlg(m_strFile, bUpdateAllowable, m_strCodeField, m_strFeeField, m_strAllowableField, this);

		if (dlg.DoModal() == IDOK) {
			
			m_strCodeField = dlg.m_strCodeField;
			m_strFeeField = dlg.m_strFeeField;
			m_strAllowableField = dlg.m_strAllowableField;
			m_strParentNode = dlg.m_strParentNode;
			m_bFormatSelected = TRUE;
		}		
	}NxCatchAll(__FUNCTION__);
	
}

void CMultiFeesImportDlg::OnBnClickedRadioCsvFormat()
{
	try {
		if (IsDlgButtonChecked(IDC_RADIO_CSV_FORMAT)) {
			
			m_bProcessXML = FALSE;
			m_bProcessCSV = TRUE;			

			m_btnFormatColumns.EnableWindow(FALSE);
		}
		else {

			m_bProcessXML = TRUE;
			m_bProcessCSV = FALSE;

			if (!m_strFile.IsEmpty()) {
				m_btnFormatColumns.EnableWindow(TRUE);
			}
			else {
				m_btnFormatColumns.EnableWindow(FALSE);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CMultiFeesImportDlg::OnBnClickedRadioXmlFormat()
{
	try {
		if (IsDlgButtonChecked(IDC_RADIO_XML_FORMAT)) {
			
			m_bProcessXML = TRUE;
			m_bProcessCSV = FALSE;

			if (!m_strFile.IsEmpty()) {
				m_btnFormatColumns.EnableWindow(TRUE);
			}
			else {
				m_btnFormatColumns.EnableWindow(FALSE);
			}
		}
		else {

			m_bProcessXML = FALSE;
			m_bProcessCSV = TRUE;

			m_btnFormatColumns.EnableWindow(FALSE);
		}
	}NxCatchAll(__FUNCTION__);
}
