// ConfigEMRPreviewDlg.cpp : implementation file
//

// (a.walling 2007-07-13 11:14) - PLID 26640 - Dialog to configure info to show on the more info part of the preview

#include "stdafx.h"
#include "ConfigEMRPreviewDlg.h"

#include "emrutils.h"
#include "EmrRc.h"
#include "EmrColors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-03-09 14:05) - PLID 36740 - Moved to cpp
CPreviewFields::CPreviewFields()
{
	arFields.Add(new CField(fFirstMLast, "First M. Last"));
	arFields.Add(new CField(fFirstMiddleLast, "First Middle Last"));
	arFields.Add(new CField(fID, "ID"));
	arFields.Add(new CField(fFirst, "First"));
	arFields.Add(new CField(fMiddle, "Middle"));
	arFields.Add(new CField(fMiddleInitial, "Middle Initial"));
	arFields.Add(new CField(fLast, "Last"));
	arFields.Add(new CField(fBirthDate, "Birth Date"));
	arFields.Add(new CField(fAge, "Age"));
	arFields.Add(new CField(fGenderMaleFemale, "Gender Male/Female"));
	arFields.Add(new CField(fGenderMF, "Gender M/F"));
	arFields.Add(new CField(fInsuranceCompany, "Insurance Company"));
	arFields.Add(new CField(fEMNDate, "EMN Date"));
	arFields.Add(new CField(fEMNLocation, "EMN Location"));
	arFields.Add(new CField(fEMNDescription, "EMN Description"));
	arFields.Add(new CField(fEMRDescription, "EMR Description"));
	arFields.Add(new CField(fPrintDate, "Print Date"));
	// (r.gonet 09-17-2010) - PLID 38968 - Adding Print_Time field support to headers and footers
	arFields.Add(new CField(fPrintTime, "Print Time"));
	// (r.gonet 09-17-2010) - PLID 38968 - Adding Print_Date_Time field support to headers and footers
	arFields.Add(new CField(fPrintDateTime, "Print Date and Time"));
	arFields.Add(new CField(fPage, "Page"));
	arFields.Add(new CField(fPageTotal, "Page Total"));
};

CPreviewFields::~CPreviewFields()
{
	for (int i = 0; i < arFields.GetSize(); i++) {
		delete arFields[i];
	}

	arFields.RemoveAll();
};

CString CPreviewFields::GetFieldCode(const CString& strFieldDescription)
{
	CString str;
	str.Format("[%s]", strFieldDescription);
	str.Replace(" ", "_");

	return str;
};

/////////////////////////////////////////////////////////////////////////////
// CConfigEMRPreviewDlg dialog

// (a.walling 2008-10-15 10:25) - PLID 31404 - CNxEditWithFields controls
// this allows us to override the context menu
BEGIN_MESSAGE_MAP(CNxEditWithFields, CNxEdit)
	//{{AFX_MSG_MAP(CNxEditWithFields)
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CNxEditWithFields::OnContextMenu(CWnd* pWnd, CPoint point)
{
	try {
		// (a.walling 2008-10-15 10:25) - PLID 31404 - Show context menu with field and formatting codes
		CMenu mnu;
		if (mnu.CreatePopupMenu()) {
			CPreviewFields pf;

			const long cOffset = 50;
			enum EOtherMenuOptions {
				eHelp = 1,
				eBold,
				ecBold,
				eItalics,
				ecItalics,
				eUnderline,
				ecUnderline,
				eBig,
				ecBig,
				eSmall,
				ecSmall,
				eLight,
				ecLight,
				eBlank,
				EOtherMenuOptionsCount,
			};

			long nMenuID = 0;

			mnu.InsertMenu(nMenuID++, MF_BYPOSITION, eHelp, "Help!");
			mnu.InsertMenu(nMenuID++, MF_BYPOSITION|MF_SEPARATOR);
			for (int i = 0; i < pf.arFields.GetSize(); i++) {
				CPreviewFields::CField* pField = pf.arFields[i];
				mnu.InsertMenu(nMenuID++, MF_BYPOSITION, pField->nID + cOffset, pField->strFieldName);
			}

			mnu.InsertMenu(nMenuID++, MF_BYPOSITION|MF_MENUBARBREAK|MF_GRAYED, 1000, "Formatting Tags");
			mnu.InsertMenu(nMenuID++, MF_BYPOSITION|MF_SEPARATOR);

			mnu.InsertMenu(nMenuID++, MF_BYPOSITION, eBold, "[b] - Bold");
			mnu.InsertMenu(nMenuID++, MF_BYPOSITION, ecBold, "[/b] - End Bold");
			mnu.InsertMenu(nMenuID++, MF_BYPOSITION, eItalics, "[i] - Italics");
			mnu.InsertMenu(nMenuID++, MF_BYPOSITION, ecItalics, "[/i] - End Italics");
			mnu.InsertMenu(nMenuID++, MF_BYPOSITION, eUnderline, "[u] - Underline");
			mnu.InsertMenu(nMenuID++, MF_BYPOSITION, ecUnderline, "[/u] - End Underline");
			mnu.InsertMenu(nMenuID++, MF_BYPOSITION, eBig, "[big] - Bigger");
			mnu.InsertMenu(nMenuID++, MF_BYPOSITION, ecBig, "[/big] - End Bigger");
			mnu.InsertMenu(nMenuID++, MF_BYPOSITION, eSmall, "[small] - Smaller");
			mnu.InsertMenu(nMenuID++, MF_BYPOSITION, ecSmall, "[/small] - End Smaller");
			mnu.InsertMenu(nMenuID++, MF_BYPOSITION, eLight, "[light] - Light (Gray) Text");
			mnu.InsertMenu(nMenuID++, MF_BYPOSITION, ecLight, "[/light] - End Light (Gray) Text");
			mnu.InsertMenu(nMenuID++, MF_BYPOSITION|MF_SEPARATOR);
			mnu.InsertMenu(nMenuID++, MF_BYPOSITION, eBlank, "[blank] - Explicit blank");

			long n = mnu.TrackPopupMenu(TPM_RETURNCMD, point.x, point.y, this);

			if (n <= 0) {
				return;
			} else {
				if (n < EOtherMenuOptionsCount) {
					CString strCode;

					switch(n) {

						case eHelp: {
										CConfigEMRPreviewDlg* pParent = (CConfigEMRPreviewDlg*)GetParent();
										if (pParent) {
											pParent->ShowHeaderFooterFormattingHelp();
										}
									} break;
						case eBold: strCode = "[b]"; break;
						case ecBold: strCode = "[/b]"; break;
						case eItalics: strCode = "[i]"; break;
						case ecItalics: strCode = "[/i]"; break;
						case eUnderline: strCode = "[u]"; break;
						case ecUnderline: strCode = "[/u]"; break;
						case eBig: strCode = "[big]"; break;
						case ecBig: strCode = "[/big]"; break;
						case eSmall: strCode = "[small]"; break;
						case ecSmall: strCode = "[/small]"; break;
						case eLight: strCode = "[light]"; break;
						case ecLight: strCode = "[/light]"; break;
						case eBlank: strCode = "[blank]"; break;

						default:
							ASSERT(FALSE);
							return;
					}

					if (!strCode.IsEmpty()) {
						ReplaceSel(strCode, TRUE);
					}
				} else {
					long nIndex = n - cOffset;
					if (nIndex < 0 || nIndex >= pf.arFields.GetSize()) {
						ASSERT(FALSE);
					} else {
						ReplaceSel(pf.GetFieldCode(pf.arFields[nIndex]->strFieldName), TRUE);
					}
				}
			}
		}

		SetFocus();
	} NxCatchAll("Failed to handle context menu");
}

void CNxEditWithFields::OnSetFocus(CWnd *pOldWnd)
{
	CNxEdit::OnSetFocus(pOldWnd);
}

// (a.walling 2008-10-14 10:50) - PLID 31678 - CWnd parent rather than CView
CConfigEMRPreviewDlg::CConfigEMRPreviewDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigEMRPreviewDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigEMRPreviewDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nOptions = 0;
	m_nDetailHideOptions = 0;
	m_nMedAllergyOptions = 0; // (b.savon 2012-05-23 16:37) - PLID 48092
	m_bMoreInfoChanged = FALSE;
	m_bEMNGlobalDataChanged = FALSE;
}


void CConfigEMRPreviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	// (a.walling 2009-08-03 10:46) - PLID 34542 - Toggle for detail dash prefix
	//{{AFX_DATA_MAP(CConfigEMRPreviewDlg)
	DDX_Control(pDX, IDC_EMRP_ENABLENARRATIVELINKS, m_btnEnableNarrativeLinks);
	DDX_Control(pDX, IDC_EMRP_OVERRIDEFONT, m_btnOverrideFont);
	DDX_Control(pDX, IDC_EMRP_DASH, m_btnPrefixDash);
	DDX_Control(pDX, IDC_EMRP_LOCATIONLOGO, m_btnDisplayLocationLogo);
	DDX_Control(pDX, IDC_MIP_ASTOPICS, m_btnDisplayAsTopics);	
	DDX_Control(pDX, IDC_MIP_SHOWEMPTY, m_btnShowEmpty);		
	DDX_Control(pDX, IDC_EMRP_HIDE_EMPTY_TOPICS_PRINT, m_btnHideEmptyTopicsPrint);
	DDX_Control(pDX, IDC_EMRP_HIDE_EMPTY_TOPICS, m_btnHideEmptyTopics);
	DDX_Control(pDX, IDC_EMRP_HIDE_NARRATIVE_PRINT, m_btnHideOnNarrativePrint);
	DDX_Control(pDX, IDC_EMRP_HIDE_NARRATIVE, m_btnHideOnNarrative);
	DDX_Control(pDX, IDC_EMRP_HIDE_EMPTY_PRINT, m_btnHideEmptyOnPrint);
	DDX_Control(pDX, IDC_EMRP_HIDE_EMPTY, m_btnHideEmpty);
	DDX_Control(pDX, IDC_MIP_MEDICATIONS, m_btnMedicatiosn);
	DDX_Control(pDX, IDC_MIP_DIAGS, m_btnDiagCodes);
	DDX_Control(pDX, IDC_MIP_SERVICES, m_btnServiceCodes);
	DDX_Control(pDX, IDC_MIP_SECONDARY, m_btnSecProviders);
	DDX_Control(pDX, IDC_MIP_TECHNICIANS, m_btnTechnicians);
	DDX_Control(pDX, IDC_MIP_PROVIDERS, m_btnProviders);
	DDX_Control(pDX, IDC_MIP_PROCEDURES, m_btnProcedures);
	DDX_Control(pDX, IDC_MIP_NOTES, m_btnNotes);
	DDX_Control(pDX, IDC_MIP_ASC_TIMES, m_btnASCTimes);
	DDX_Control(pDX, IDC_MIP_ASC_STATUS, m_btnASCStatus);
	DDX_Control(pDX, IDC_NXCOLORCTRL_CONFIG_PREVIEW, m_color);
	DDX_Control(pDX, IDC_NXCOLORCTRLHEADER, m_nxcHeader);
	DDX_Control(pDX, IDC_NXCOLORCTRL_MOREINFO, m_nxcMoreInfo);
	DDX_Control(pDX, IDC_NXCOLORCTRL_VISIBILITY, m_nxcVisibility);
	DDX_Control(pDX, IDC_NXCOLORCTRL_MEDSALL, m_nxcMedicationsAllergies);
	DDX_Control(pDX, IDC_NXCOLORCTRL_MISC, m_nxcMisc);
	DDX_Control(pDX, IDC_CHECK_DISPLAYMEDS, m_btnMedications);
	DDX_Control(pDX, IDC_CHECK_DISMEDS, m_btnMedDiscontinued);
	DDX_Control(pDX, IDC_CHECK_DISPALLERGIES, m_btnAllergies);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EDIT_HEADERLEFT, m_editHeaderLeft);
	DDX_Control(pDX, IDC_EDIT_HEADERRIGHT, m_editHeaderRight);
	DDX_Control(pDX, IDC_EDIT_HEADER2LEFT, m_editHeader2Left);
	DDX_Control(pDX, IDC_EDIT_HEADER2RIGHT, m_editHeader2Right);
	DDX_Control(pDX, IDC_EDIT_FOOTERLEFT, m_editFooterLeft);
	DDX_Control(pDX, IDC_EDIT_FOOTERRIGHT, m_editFooterRight);
	DDX_Control(pDX, IDC_EDIT_FOOTER2LEFT, m_editFooter2Left);
	DDX_Control(pDX, IDC_EDIT_FOOTER2RIGHT, m_editFooter2Right);
	DDX_Control(pDX, IDC_BTN_HEADFOOTHELP, m_btnHelp);	
	DDX_Control(pDX, IDC_IMAGES_USE_FILE_SIZE, m_btnImagesUseFileSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigEMRPreviewDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConfigEMRPreviewDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_HEADFOOTHELP, &CConfigEMRPreviewDlg::OnBnClickedBtnHeadfoothelp)
	ON_BN_CLICKED(IDC_CHECK_DISPLAYMEDS, &CConfigEMRPreviewDlg::OnBnClickedCheckDisplaymeds)
	ON_BN_CLICKED(IDC_CHECK_DISMEDS, &CConfigEMRPreviewDlg::OnBnClickedCheckDismeds)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigEMRPreviewDlg message handlers

BOOL CConfigEMRPreviewDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();	

		// (c.haag 2008-05-01 10:22) - PLID 29863 - NxIconified buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
		m_bkgColor = EmrColors::Topic::PatientBackground();

		m_color.SetColor(m_bkgColor);
		
		// (b.savon 2012-05-23 16:01) - PLID 48092 - Redesigned Dialog
		m_nxcHeader.SetColor(m_bkgColor);
		m_nxcMoreInfo.SetColor(m_bkgColor);
		m_nxcVisibility.SetColor(m_bkgColor);
		m_nxcMedicationsAllergies.SetColor(m_bkgColor);
		m_nxcMisc.SetColor(m_bkgColor);
	
		// (d.lange 2011-04-27 17:18) - PLID 43380 - Changed 127 to 255
		m_nOptions = GetRemotePropertyInt("EMRPreviewMoreInfoFields", 255, 0, "<None>", true);
		m_nDetailHideOptions = GetRemotePropertyInt("EMRPreview_HideDetails", g_dhPreviewDisplayHideDefaults, 0, "<None>", true);
		// (b.savon 2012-05-23 16:38) - PLID 48092
		m_nMedAllergyOptions = GetRemotePropertyInt("EMRPreview_MedAllergy", 5, 0, "<None>", true);

		// (a.walling 2008-07-01 16:57) - PLID 30586
		m_nLogoOption = GetRemotePropertyInt("EMRPreviewIncludeLocationLogo", TRUE, 0, "<None>", true);

		// (a.walling 2009-08-03 10:46) - PLID 34542 - Toggle for detail dash prefix
		m_bPrefixDash = GetRemotePropertyInt("EMRPreviewPrefixDash", TRUE, 0, "<None>", true);

		// (a.walling 2010-03-24 20:28) - PLID 29293 - Option to override font
		m_bOverrideFont = GetRemotePropertyInt("EMRPreviewOverrideDefaultNarrativeFont", TRUE, 0, "<None>", true);

		// (a.walling 2010-03-26 12:44) - PLID 29099 - Option to enable/disable interactive narratives
		m_bEnableNarrativeLinks = GetRemotePropertyInt("EMRPreviewEnableInteractiveNarrative", TRUE, 0, "<None>", true);

		//TES 1/25/2012 - PLID 47505 - Setting to use the original file size when sizing image details
		m_bImagesUseFileSize = GetRemotePropertyInt("EMRPreview_UseOriginalFileSize", FALSE, 0, "<None>", true);

		// (a.walling 2008-10-15 10:26) - PLID 31404 - Limit header/footer text to 255 chars each
		m_editHeaderLeft.SetLimitText(255);
		m_editHeaderRight.SetLimitText(255);
		m_editHeader2Left.SetLimitText(255);
		m_editHeader2Right.SetLimitText(255);
		m_editFooterLeft.SetLimitText(255);
		m_editFooterRight.SetLimitText(255);
		m_editFooter2Left.SetLimitText(255);
		m_editFooter2Right.SetLimitText(255);

		// (a.walling 2008-10-14 11:24) - PLID 31404 - Load the header/footer fields into controls
		m_HeaderFooterInfo.Load();		
		m_editHeaderLeft.SetWindowText(m_HeaderFooterInfo.strHeaderLeft);
		m_editHeaderRight.SetWindowText(m_HeaderFooterInfo.strHeaderRight);
		m_editHeader2Left.SetWindowText(m_HeaderFooterInfo.strHeader2Left);
		m_editHeader2Right.SetWindowText(m_HeaderFooterInfo.strHeader2Right);
		m_editFooterLeft.SetWindowText(m_HeaderFooterInfo.strFooterLeft);
		m_editFooterRight.SetWindowText(m_HeaderFooterInfo.strFooterRight);
		m_editFooter2Left.SetWindowText(m_HeaderFooterInfo.strFooter2Left);
		m_editFooter2Right.SetWindowText(m_HeaderFooterInfo.strFooter2Right);

		// (b.savon 2012-05-23 16:41) - PLID 48092 - Check defaults
		if (m_nMedAllergyOptions & maoDisplayMedications)
			m_btnMedications.SetCheck(BST_CHECKED);
		if (m_nMedAllergyOptions & maoDiscontinuedMeds)
			m_btnMedDiscontinued.SetCheck(BST_CHECKED);
		if (m_nMedAllergyOptions & maoDisplayAllergies)
			m_btnAllergies.SetCheck(BST_CHECKED);

		if (m_nOptions & mipNotes)
			CheckDlgButton(IDC_MIP_NOTES, TRUE);
		if (m_nOptions & mipProcedures)
			CheckDlgButton(IDC_MIP_PROCEDURES, TRUE);
		if (m_nOptions & mipProviders)
			CheckDlgButton(IDC_MIP_PROVIDERS, TRUE);
		if (m_nOptions & mipSecondaryProviders)
			CheckDlgButton(IDC_MIP_SECONDARY, TRUE);
		// (d.lange 2011-04-25 16:03) - PLID 43380
		if (m_nOptions & mipTechnicians)
			CheckDlgButton(IDC_MIP_TECHNICIANS, TRUE);
		if (m_nOptions & mipDiagCodes)
			CheckDlgButton(IDC_MIP_DIAGS, TRUE);
		if (m_nOptions & mipCharges)
			CheckDlgButton(IDC_MIP_SERVICES, TRUE);
		if (m_nOptions & mipMedications)
			CheckDlgButton(IDC_MIP_MEDICATIONS, TRUE);
		// (a.walling 2010-11-11 11:51) - PLID 40848
		if (m_nOptions & mipDisplayAsTopics)
			CheckDlgButton(IDC_MIP_ASTOPICS, TRUE);
		if (m_nOptions & mipShowEmpty)
			CheckDlgButton(IDC_MIP_SHOWEMPTY, TRUE);
		// (b.eyers 2016-02-23) - PLID 68322
		if (m_nOptions & mipASCTimes)
			CheckDlgButton(IDC_MIP_ASC_TIMES, TRUE);
		if (m_nOptions & mipASCStatus)
			CheckDlgButton(IDC_MIP_ASC_STATUS, TRUE);

		// (a.walling 2007-12-17 13:46) - PLID 28354
		if (m_nDetailHideOptions & dhEmpty)
			CheckDlgButton(IDC_EMRP_HIDE_EMPTY, TRUE);
		if (m_nDetailHideOptions & dhEmptyPrint)
			CheckDlgButton(IDC_EMRP_HIDE_EMPTY_PRINT, TRUE);
		if (m_nDetailHideOptions & dhNarrative)
			CheckDlgButton(IDC_EMRP_HIDE_NARRATIVE, TRUE);
		if (m_nDetailHideOptions & dhNarrativePrint)
			CheckDlgButton(IDC_EMRP_HIDE_NARRATIVE_PRINT, TRUE);
		// (a.walling 2012-07-16 08:42) - PLID 48896
		if (m_nDetailHideOptions & dhAlwaysHideIfIndirect)
			CheckDlgButton(IDC_EMRP_ALWAYS_HIDE_IF_INDIRECT, TRUE);
		// (a.walling 2007-12-18 13:11) - PLID 28391
		if (m_nDetailHideOptions & dhEmptyTopics)
			CheckDlgButton(IDC_EMRP_HIDE_EMPTY_TOPICS, TRUE);
		if (m_nDetailHideOptions & dhEmptyTopicsPrint)
			CheckDlgButton(IDC_EMRP_HIDE_EMPTY_TOPICS_PRINT, TRUE);

		// (a.walling 2008-07-01 16:57) - PLID 30586
		if (m_nLogoOption != 0) {
			CheckDlgButton(IDC_EMRP_LOCATIONLOGO, TRUE);
		}
		if (m_bPrefixDash) {
			CheckDlgButton(IDC_EMRP_DASH, TRUE);
		}
		// (a.walling 2010-03-24 20:28) - PLID 29293 - Option to override font
		if (m_bOverrideFont) {
			CheckDlgButton(IDC_EMRP_OVERRIDEFONT, TRUE);
		}
		// (a.walling 2010-03-26 12:44) - PLID 29099 - Option to enable/disable interactive narratives
		if (m_bEnableNarrativeLinks) {
			CheckDlgButton(IDC_EMRP_ENABLENARRATIVELINKS, TRUE);
		}

		//TES 1/25/2012 - PLID 47505 - Setting to use the original file size when sizing image details
		if(m_bImagesUseFileSize) {
			CheckDlgButton(IDC_IMAGES_USE_FILE_SIZE, TRUE);
		}

		SetFocus();

	} NxCatchAll("Error initializing CConfigEMRPreviewDlg");
	return FALSE;
}

void CConfigEMRPreviewDlg::OnOK() 
{
	try{
		long nOptions = GetNewOptions();

		if (m_nOptions == nOptions) {
			// nothing has changed
		} else {
			m_nOptions = nOptions;

			SetRemotePropertyInt("EMRPreviewMoreInfoFields", m_nOptions, 0, "<None>");
		
			m_bMoreInfoChanged = TRUE;
		}

		// (a.walling 2007-12-17 13:52) - PLID 28354
		nOptions = GetNewDetailHideOptions();
		if (m_nDetailHideOptions == nOptions) {
			// nothing has changed
		} else {
			m_nDetailHideOptions = nOptions;

			SetRemotePropertyInt("EMRPreview_HideDetails", m_nDetailHideOptions, 0, "<None>");

			m_bEMNGlobalDataChanged = TRUE;
		}

		// (b.savon 2012-05-23 16:44) - PLID 48092 - Set Med/Allergy Options
		nOptions = GetNewMedAllergyOptions();
		if (m_nMedAllergyOptions == nOptions) {
			// nothing has changed
		} else {
			m_nMedAllergyOptions = nOptions;

			SetRemotePropertyInt("EMRPreview_MedAllergy", m_nMedAllergyOptions, 0, "<None>");

			//	I am bundling this information in with the MoreInfo HTML Section, so, let's share this
			//  flag so that it updates if the options change.
			m_bEMNGlobalDataChanged = TRUE;
		}

		// (a.walling 2008-07-01 16:59) - PLID 30586
		long nLogoOption = IsDlgButtonChecked(IDC_EMRP_LOCATIONLOGO) ? TRUE : FALSE;
		if (nLogoOption != m_nLogoOption) {
			m_nLogoOption = nLogoOption;
			SetRemotePropertyInt("EMRPreviewIncludeLocationLogo", nLogoOption, 0, "<None>");
			m_bEMNGlobalDataChanged = TRUE;
		}

		// (a.walling 2009-08-03 10:46) - PLID 34542 - Toggle for detail dash prefix
		BOOL bPrefixDash = IsDlgButtonChecked(IDC_EMRP_DASH) ? TRUE : FALSE;
		if (bPrefixDash != m_bPrefixDash) {
			m_bPrefixDash = bPrefixDash;
			SetRemotePropertyInt("EMRPreviewPrefixDash", bPrefixDash, 0, "<None>");
			m_bEMNGlobalDataChanged = TRUE;
		}
		// (a.walling 2010-03-24 20:28) - PLID 29293 - Option to override font
		BOOL bOverrideFont = IsDlgButtonChecked(IDC_EMRP_OVERRIDEFONT) ? TRUE : FALSE;
		if (bOverrideFont != m_bOverrideFont) {
			m_bOverrideFont = bOverrideFont;
			SetRemotePropertyInt("EMRPreviewOverrideDefaultNarrativeFont", bOverrideFont, 0, "<None>");
			m_bEMNGlobalDataChanged = TRUE;
		}
		// (a.walling 2010-03-26 12:44) - PLID 29099 - Option to enable/disable interactive narratives
		BOOL bEnableNarrativeLinks = IsDlgButtonChecked(IDC_EMRP_ENABLENARRATIVELINKS) ? TRUE : FALSE;
		if (bEnableNarrativeLinks != m_bEnableNarrativeLinks) {
			m_bEnableNarrativeLinks = bEnableNarrativeLinks;
			SetRemotePropertyInt("EMRPreviewEnableInteractiveNarrative", bEnableNarrativeLinks, 0, "<None>");
			m_bEMNGlobalDataChanged = TRUE;
		}

		//TES 1/25/2012 - PLID 47505 - Setting to use the original file size when sizing image details
		BOOL bImagesUseFileSize = IsDlgButtonChecked(IDC_IMAGES_USE_FILE_SIZE) ? TRUE : FALSE;
		if(bImagesUseFileSize != m_bImagesUseFileSize) {
			m_bImagesUseFileSize = bImagesUseFileSize;
			SetRemotePropertyInt("EMRPreview_UseOriginalFileSize", bImagesUseFileSize, 0, "<None>");
			m_bEMNGlobalDataChanged = TRUE;
		}

		{
			// (a.walling 2008-10-15 10:26) - PLID 31404 - Load header/footer info from controls
			CHeaderFooterInfo hfiNew;
			m_editHeaderLeft.GetWindowText(hfiNew.strHeaderLeft);
			m_editHeaderRight.GetWindowText(hfiNew.strHeaderRight);
			m_editHeader2Left.GetWindowText(hfiNew.strHeader2Left);
			m_editHeader2Right.GetWindowText(hfiNew.strHeader2Right);
			m_editFooterLeft.GetWindowText(hfiNew.strFooterLeft);
			m_editFooterRight.GetWindowText(hfiNew.strFooterRight);
			m_editFooter2Left.GetWindowText(hfiNew.strFooter2Left);
			m_editFooter2Right.GetWindowText(hfiNew.strFooter2Right);

			if (hfiNew == m_HeaderFooterInfo) {
				// equal, do nothing
			} else {
				hfiNew.Save();
			}
		}

	}NxCatchAll(__FUNCTION__);

	CDialog::OnOK();
}

void CConfigEMRPreviewDlg::OnCancel() 
{
	CDialog::OnCancel();
}

// (b.savon 2012-05-23 16:25) - PLID 48092 - Get the Med/Allergy Options
long CConfigEMRPreviewDlg::GetNewMedAllergyOptions()
{
	long nOptions = 0;
	
	if (m_btnMedications.GetCheck() == BST_CHECKED)
		nOptions |= maoDisplayMedications;
	if (m_btnMedDiscontinued.GetCheck() == BST_CHECKED)
		nOptions |= maoDiscontinuedMeds;
	if (m_btnAllergies.GetCheck() == BST_CHECKED)
		nOptions |= maoDisplayAllergies;

	return nOptions;
}

long CConfigEMRPreviewDlg::GetNewOptions()
{
	long nOptions = 0;
	if (IsDlgButtonChecked(IDC_MIP_NOTES))
		nOptions |= mipNotes;
	if (IsDlgButtonChecked(IDC_MIP_PROCEDURES))
		nOptions |= mipProcedures;
	if (IsDlgButtonChecked(IDC_MIP_PROVIDERS))
		nOptions |= mipProviders;
	if (IsDlgButtonChecked(IDC_MIP_SECONDARY))
		nOptions |= mipSecondaryProviders;
	// (d.lange 2011-04-25 16:20) - PLID 43380
	if (IsDlgButtonChecked(IDC_MIP_TECHNICIANS))
		nOptions |= mipTechnicians;
	if (IsDlgButtonChecked(IDC_MIP_DIAGS))
		nOptions |= mipDiagCodes;
	if (IsDlgButtonChecked(IDC_MIP_SERVICES))
		nOptions |= mipCharges;
	if (IsDlgButtonChecked(IDC_MIP_MEDICATIONS))
		nOptions |= mipMedications;
	// (a.walling 2010-11-11 11:51) - PLID 40848
	if (IsDlgButtonChecked(IDC_MIP_ASTOPICS))
		nOptions |= mipDisplayAsTopics;
	if (IsDlgButtonChecked(IDC_MIP_SHOWEMPTY))
		nOptions |= mipShowEmpty;
	// (b.eyers 2016-02-23) - PLID 68322
	if (IsDlgButtonChecked(IDC_MIP_ASC_TIMES))
		nOptions |= mipASCTimes;
	if (IsDlgButtonChecked(IDC_MIP_ASC_STATUS))
		nOptions |= mipASCStatus;

	return nOptions;
}

long CConfigEMRPreviewDlg::GetNewDetailHideOptions()
{
	// (a.walling 2007-12-17 13:43) - PLID 28354 - return new detail hide options
	long nOptions = 0;

	if (IsDlgButtonChecked(IDC_EMRP_HIDE_EMPTY))
		nOptions |= dhEmpty;
	if (IsDlgButtonChecked(IDC_EMRP_HIDE_EMPTY_PRINT))
		nOptions |= dhEmptyPrint;
	if (IsDlgButtonChecked(IDC_EMRP_HIDE_NARRATIVE))
		nOptions |= dhNarrative;
	if (IsDlgButtonChecked(IDC_EMRP_HIDE_NARRATIVE_PRINT))
		nOptions |= dhNarrativePrint;
	// (a.walling 2012-07-16 08:43) - PLID 48896
	if (IsDlgButtonChecked(IDC_EMRP_ALWAYS_HIDE_IF_INDIRECT))
		nOptions |= dhAlwaysHideIfIndirect;
	// (a.walling 2007-12-18 13:11) - PLID 28391
	if (IsDlgButtonChecked(IDC_EMRP_HIDE_EMPTY_TOPICS))
		nOptions |= dhEmptyTopics;
	if (IsDlgButtonChecked(IDC_EMRP_HIDE_EMPTY_TOPICS_PRINT))
		nOptions |= dhEmptyTopicsPrint;

	return nOptions;
}

// (a.walling 2008-10-15 10:24) - PLID 31404 - Show help for customization and formatting
void CConfigEMRPreviewDlg::ShowHeaderFooterFormattingHelp()
{
	CString strMessage = 
		"The header and footer are customizable by using tags. Several tags are available which will be replaced by the "
		"appropriate information. Other tags can modify the display of the text; for example, [b]text[/b] will make "
		"everything between the tags bold.\r\n\r\n"
		"There are two lines available for the header, and two for the footer. These lines are split into left and right "
		"sections. If one section is blank but the other has information, then the two sections will be merged and the "
		"content will be centered. This can be overridden by putting the [blank] tag in one of the fields.\r\n\r\n"
		"Simply right click in a text box to see the list of supported tags. If you need further assistance, please "
		"contact NexTech Technical Support.";

	MessageBox(strMessage, NULL, MB_ICONINFORMATION);
}

// (a.walling 2008-10-14 11:28) - PLID 31404
void CHeaderFooterInfo::Load()
{
	strHeaderLeft	= GetRemotePropertyText("EMRPreview_Header", "[b][First_M._Last][/b] ([ID])", 0, "<None>", true);	
	strHeaderRight	= GetRemotePropertyText("EMRPreview_Header", "Page [Page] of [Page_Total]", 1, "<None>", true);	
	strHeader2Left	= GetRemotePropertyText("EMRPreview_Header", "[Age] yr. [Gender_Male/Female], [Birth_Date]", 2, "<None>", true);	
	strHeader2Right	= GetRemotePropertyText("EMRPreview_Header", "[blank]", 3, "<None>", true);	
	
	strFooterLeft	= GetRemotePropertyText("EMRPreview_Footer", "[EMN_Date] - [EMN_Location]", 0, "<None>", true);	
	strFooterRight	= GetRemotePropertyText("EMRPreview_Footer", "Printed on [Print_Date]", 1, "<None>", true);	
	strFooter2Left	= GetRemotePropertyText("EMRPreview_Footer", "", 2, "<None>", true);	
	strFooter2Right	= GetRemotePropertyText("EMRPreview_Footer", "", 3, "<None>", true);
}


// (a.walling 2008-10-14 11:28) - PLID 31404
void CHeaderFooterInfo::Save()
{
	SetRemotePropertyText("EMRPreview_Header", strHeaderLeft, 0, "<None>");
	SetRemotePropertyText("EMRPreview_Header", strHeaderRight, 1, "<None>");
	SetRemotePropertyText("EMRPreview_Header", strHeader2Left, 2, "<None>");
	SetRemotePropertyText("EMRPreview_Header", strHeader2Right, 3, "<None>");
	
	SetRemotePropertyText("EMRPreview_Footer", strFooterLeft, 0, "<None>");
	SetRemotePropertyText("EMRPreview_Footer", strFooterRight, 1, "<None>");
	SetRemotePropertyText("EMRPreview_Footer", strFooter2Left, 2, "<None>");
	SetRemotePropertyText("EMRPreview_Footer", strFooter2Right, 3, "<None>");
}

// (a.walling 2008-10-14 11:32) - PLID 31404
bool CHeaderFooterInfo::operator==(const CHeaderFooterInfo& hfi) const
{
	return (
		strHeaderLeft == hfi.strHeaderLeft &&
		strHeaderRight == hfi.strHeaderRight &&
		strHeader2Left == hfi.strHeader2Left &&
		strHeader2Right == hfi.strHeader2Right &&

		strFooterLeft == hfi.strFooterLeft &&
		strFooterRight == hfi.strFooterRight &&
		strFooter2Left == hfi.strFooter2Left &&
		strFooter2Right == hfi.strFooter2Right
		);
}

void CConfigEMRPreviewDlg::OnBnClickedBtnHeadfoothelp()
{
	ShowHeaderFooterFormattingHelp();
}

// (b.savon 2012-05-24 12:21) - PLID 48092 - If we uncheck the Display Meds, we should uncheck the 
// include discontinued for the user as well
void CConfigEMRPreviewDlg::OnBnClickedCheckDisplaymeds()
{
	try{
		if( m_btnMedications.GetCheck() == BST_UNCHECKED ){
			if( m_btnMedDiscontinued.GetCheck() == BST_CHECKED ){
				m_btnMedDiscontinued.SetCheck(BST_UNCHECKED);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-05-24 12:21) - PLID 48092 - If we check the include discontinued, we should check the 
// display meds for the user as well
void CConfigEMRPreviewDlg::OnBnClickedCheckDismeds()
{
	try{
		if( m_btnMedDiscontinued.GetCheck() == BST_CHECKED ){
			if( m_btnMedications.GetCheck() == BST_UNCHECKED ){
				m_btnMedications.SetCheck(BST_CHECKED);
			}
		}
	}NxCatchAll(__FUNCTION__);
}
