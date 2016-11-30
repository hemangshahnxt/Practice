// SuperBillDlg.cpp : implementation file
//
#include "stdafx.h"
#include "SuperBillDlg.h"
#include "LetterWriting.h"
#include "MergeEngine.h"
#include "GlobalDataUtils.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "Superbill.h"
#include "DateTimeUtils.h"
#include "LetterWritingRc.h"
#include "DontShowDlg.h"
#include "ConfigureSuperbillTemplatesDlg.h"
#include "ConfigureSuperbillAddFileDlg.h"
#include "FileUtils.h"
#include "NxTaskDialog.h"
#include "MiscSystemUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// CSuperBillDlg dialog


// (z.manning 2009-10-21 16:01) - PLID 35990 - These are stored in data and cannot be changed!
enum EDefaultSortRadio {
	dsrDate = 1,
	dsrResource = 2,
	dsrResourceThenDate = 3,
};


CSuperBillDlg::CSuperBillDlg()
	: CNxDialog(CSuperBillDlg::IDD, NULL)
{
	//{{AFX_DATA_INIT(CSuperBillDlg)
	//}}AFX_DATA_INIT
	m_nDefTemplateRowIndex = -1;
}


void CSuperBillDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSuperBillDlg)
	DDX_Control(pDX, IDC_REMEMBER_SELECTED, m_btnRememberSelected);
	DDX_Control(pDX, IDC_CHECK_SB_SHOWINACTIVERESOURCES, m_btnShowInactiveResources);
	DDX_Control(pDX, IDC_MERGE_PRINTER, m_btnMergeToPrinter);
	DDX_Control(pDX, IDC_CHECK_LOCATION_FILTER, m_btnLocFilter);
	DDX_Control(pDX, IDC_CANCEL, m_cancelBtn);
	DDX_Control(pDX, IDC_RES_ADD, m_resAddBtn);
	DDX_Control(pDX, IDC_RES_REMOVE_ALL, m_resRemoveAllBtn);
	DDX_Control(pDX, IDC_RES_REMOVE, m_resRemoveBtn);
	DDX_Control(pDX, IDC_RES_ADD_ALL, m_resAddAllBtn);
	DDX_Control(pDX, IDC_APT_ADD, m_aptAddBtn);
	DDX_Control(pDX, IDC_APT_REMOVE, m_aptRemoveBtn);
	DDX_Control(pDX, IDC_ATP_REMOVE_ALL, m_aptRemoveAllBtn);
	DDX_Control(pDX, IDC_APT_ADD_ALL, m_aptAddAllBtn);
	DDX_Control(pDX, IDC_PRINT_SUPERBILLS, m_printBtn);
	DDX_Control(pDX, IDC_FROM, m_from);
	DDX_Control(pDX, IDC_TO, m_to);
	DDX_Control(pDX, IDC_USE_DATE_RANGE, m_dateFilter);
	DDX_Control(pDX, IDC_SORT_BY_NAME, m_nameSort);
	DDX_Control(pDX, IDC_SORT_BY_DATE, m_dateSort);
	DDX_Control(pDX, IDC_DEFTEMPLATE_BTN, m_btnDefaultTemplate);
	DDX_Control(pDX, IDC_EDIT_TEMPLATE_BTN, m_btnEditTemplate);
	DDX_Control(pDX, IDC_SORT_BY_RESOURCE_DATE, m_btnResourceDateSort);
	DDX_Control(pDX, IDC_USE_ADVANCED_CONFIG, m_btnAdvancedTemplateConfig);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CSuperBillDlg, IDC_FROM, 2 /* Change */, OnChangeFrom, VTS_NONE)
//	ON_EVENT(CSuperBillDlg, IDC_TO, 2 /* Change */, OnChangeTo, VTS_NONE)
// (a.walling 2008-05-13 14:57) - PLID 27591 - Use Notify handlers for DateTimePicker
BEGIN_MESSAGE_MAP(CSuperBillDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSuperBillDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_FROM, OnChangeFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TO, OnChangeTo)
	ON_BN_CLICKED(IDC_APT_ADD, OnAptAdd)
	ON_BN_CLICKED(IDC_APT_ADD_ALL, OnAptAddAll)
	ON_BN_CLICKED(IDC_APT_REMOVE, OnAptRemove)
	ON_BN_CLICKED(IDC_ATP_REMOVE_ALL, OnAptRemoveAll)
	ON_BN_CLICKED(IDC_RES_ADD, OnResAdd)
	ON_BN_CLICKED(IDC_RES_ADD_ALL, OnResAddAll)
	ON_BN_CLICKED(IDC_RES_REMOVE, OnResRemove)
	ON_BN_CLICKED(IDC_RES_REMOVE_ALL, OnResRemoveAll)
	ON_BN_CLICKED(IDC_PRINT_SUPERBILLS, OnPrintSuperbills)
	ON_BN_CLICKED(IDC_DEFTEMPLATE_BTN, OnDeftemplateBtn)
	ON_BN_CLICKED(IDC_EDIT_TEMPLATE_BTN, OnEditTemplateBtn)
	ON_BN_CLICKED(IDC_REMEMBER_SELECTED, OnRememberSelected)
	ON_BN_CLICKED(IDC_CHECK_SB_SHOWINACTIVERESOURCES, OnShowInactiveResources)
	ON_BN_CLICKED(IDC_MERGE_PRINTER, OnMergePrinter)
	ON_BN_CLICKED(IDC_CHECK_LOCATION_FILTER, OnCheckLocationFilter)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	ON_BN_CLICKED(IDC_USE_DATE_RANGE, OnUseDateRange)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_USE_ADVANCED_CONFIG, &CSuperBillDlg::OnBnClickedUseAdvancedConfig)
	ON_BN_CLICKED(IDC_TEMPLATE_CONFIG, &CSuperBillDlg::OnBnClickedTemplateConfig)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CSuperBillDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSuperBillDlg)
	ON_EVENT(CSuperBillDlg, IDC_RES_SELECTED, 3 /* DblClickCell */, OnDblClickCellResSelected, VTS_I4 VTS_I2)
	ON_EVENT(CSuperBillDlg, IDC_RES_UNSELECTED, 3 /* DblClickCell */, OnDblClickCellResUnselected, VTS_I4 VTS_I2)
	ON_EVENT(CSuperBillDlg, IDC_TYPE_SELECTED, 3 /* DblClickCell */, OnDblClickCellTypeSelected, VTS_I4 VTS_I2)
	ON_EVENT(CSuperBillDlg, IDC_TYPE_UNSELECTED, 3 /* DblClickCell */, OnDblClickCellTypeUnselected, VTS_I4 VTS_I2)
	ON_EVENT(CSuperBillDlg, IDC_TEMPLATE, 16 /* SelChosen */, OnSelChosenTemplate, VTS_I4)
	ON_EVENT(CSuperBillDlg, IDC_LOCATION_FILTER, 1 /* SelChanging */, OnSelChangingLocationFilter, VTS_PI4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()
/////////////////////////////////////////////////////////////////////////////
// CSuperBillDlg message handlers

void CSuperBillDlg::FillSuperBillList(CString strPath) {

	IRowSettingsPtr pRow;

	// (a.walling 2007-06-14 16:16) - PLID 26342 - Support Word2007 templates

	//build file list (automatically selecting the default template)
	CFileFind finder;
	CString strFind = strPath;
	if (finder.FindFile(strFind ^ "*.*"))
	{
		long nIndex;
		while (finder.FindNextFile())
		{
			if(finder.IsDots())
				continue;

			if(finder.IsDirectory()) {
				if(!finder.IsDots()) {
					FillSuperBillList(finder.GetFilePath());
				}
			}
			else {

				CString strFileName = finder.GetFileName();
				CString strFileNameLower = strFileName;
				strFileNameLower.MakeLower(); // (a.walling 2008-04-28 13:17) - PLID 28108 - Filenames are case insensitive

				// (a.walling 2007-06-14 16:17) - PLID 26342
				// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
				if(strFileNameLower.Right(4) == ".dot" || (strFileNameLower.Right(5) == ".dotx") || (strFileNameLower.Right(5) == ".dotm")) {
					pRow = m_template->GetRow(-1);
					pRow->Value[0] = _bstr_t(strFileName);
					pRow->Value[1] = _bstr_t(finder.GetFilePath());
					nIndex = m_template->AddRow(pRow);
				}
			}
		}
		//do once more
		if(finder.IsDirectory()) {
			if(!finder.IsDots()) {
				FillSuperBillList(finder.GetFilePath());
			}
		}
		else {

			CString strFileName = finder.GetFileName();
			CString strFileNameLower = strFileName;
			strFileNameLower.MakeLower(); // (a.walling 2008-04-28 13:17) - PLID 28108 - Filenames are case insensitive
			// (a.walling 2007-06-14 16:17) - PLID 26342
			// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
			if(strFileNameLower.Right(4) == ".dot" || (strFileNameLower.Right(5) == ".dotx") || (strFileNameLower.Right(5) == ".dotm")) {
				pRow = m_template->GetRow(-1);
				pRow->Value[0] = _bstr_t(strFileName);
				pRow->Value[1] = _bstr_t(finder.GetFilePath());
				nIndex = m_template->AddRow(pRow);
			}
		}
	}
}

BOOL CSuperBillDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		IRowSettingsPtr pRow;
		m_resUnselected = BindNxDataListCtrl(IDC_RES_UNSELECTED);
		m_resSelected = BindNxDataListCtrl(IDC_RES_SELECTED, false);
		m_aptUnselected = BindNxDataListCtrl(IDC_TYPE_UNSELECTED);
		m_aptSelected = BindNxDataListCtrl(IDC_TYPE_SELECTED, false);
		m_template = BindNxDataListCtrl(IDC_TEMPLATE, false);
		m_pLocation = BindNxDataListCtrl(IDC_LOCATION_FILTER);

		// (d.thompson 2009-09-24) - PLID 9975 - This function lacked batch loading of preferences, so I included it.
		g_propManager.CachePropertiesInBulk("SuperbillForSchedPatsDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'SuperbillRemember' "							//Pre-existing, not bulk cached
				"OR Name = 'SuperbillAdvancedTemplateConfigInUse' "		// (d.thompson 2009-09-24) - PLID 9975
				"OR Name = 'SuperbillConfig_Types' "					// (d.thompson 2009-09-24) - PLID 9975
				"OR Name = 'SuperbillConfig_Resources' "				// (d.thompson 2009-09-24) - PLID 9975
				"OR Name = 'SuperbillConfig_PickList' "					// (d.thompson 2009-09-24) - PLID 9975
				"OR Name = 'SuperbillConfig_UseGlobal' "				// (d.thompson 2009-09-24) - PLID 9975
				"OR Name = 'SuperbillDialogDefaultSort' "				// (z.manning 2009-10-28 17:15) - PLID 35990
				"OR Name = 'MergeSuperbillsToPrinter' "					// (j.jones 2011-07-08 15:28) - PLID 13660
				")",
				_Q(GetCurrentUserName()));

		// (d.thompson 2009-10-20) - PLID 9975 - bulk cache text props too
		g_propManager.CachePropertiesInBulk("SuperbillForSchedPatsDlg_Text", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'DefaultSuperbillFilename' "					//Pre-existing, not bulk cached
				"OR Name = 'SuperbillRememberType' "					//Pre-existing, not bulk cached
				"OR Name = 'SuperbillRememberRes' "						//Pre-existing, not bulk cached
				")",
				_Q(GetCurrentUserName()));

		// (j.jones 2011-07-08 15:28) - PLID 13660 - added pref. to merge to printer
		m_btnMergeToPrinter.SetCheck(GetRemotePropertyInt("MergeSuperbillsToPrinter", 0, 0, GetCurrentUserName(), true) == 1);
		OnMergePrinter();

		m_dateFilter.SetCheck(BST_CHECKED);
		// (z.manning 2009-08-03 12:54) - PLID 19645 - I added a new option for sorting by resource then date.
		// I'm making this the default now as as I think this is the best sorting option (default used to be
		// sorting just by date).
		// (z.manning 2009-10-21 15:58) - PLID 35990 - Don't change a long standing default, moron. Instead, 
		// let's add an option to remember this setting, keeping the default as date as that's what it used to be.
		EDefaultSortRadio eSortOption = (EDefaultSortRadio)GetRemotePropertyInt("SuperbillDialogDefaultSort", dsrDate, 0, GetCurrentUserName());
		switch(eSortOption)
		{
			case dsrDate:
				m_dateSort.SetCheck(BST_CHECKED);
				break;

			case dsrResource:
				m_nameSort.SetCheck(BST_CHECKED);
				break;

			case dsrResourceThenDate:
				m_btnResourceDateSort.SetCheck(BST_CHECKED);
				break;

			default:
				ASSERT(FALSE);
				m_dateSort.SetCheck(BST_CHECKED);
				break;
		}
		
		m_printBtn.AutoSet(NXB_MERGE); // (z.manning, 04/25/2008) - PLID 29795 - Changed to use the new merge style
		m_cancelBtn.AutoSet(NXB_CANCEL); // (z.manning, 04/25/2008) - PLID 29795 - Changed to use the cancel style
		m_aptAddBtn.AutoSet(NXB_RIGHT);
		m_aptRemoveBtn.AutoSet(NXB_LEFT);
		m_aptAddAllBtn.AutoSet(NXB_RRIGHT);
		m_aptRemoveAllBtn.AutoSet(NXB_LLEFT);
		m_resAddBtn.AutoSet(NXB_RIGHT);
		m_resRemoveBtn.AutoSet(NXB_LEFT);
		m_resAddAllBtn.AutoSet(NXB_RRIGHT);
		m_resRemoveAllBtn.AutoSet(NXB_LLEFT);
		m_btnDefaultTemplate.AutoSet(NXB_MODIFY);
		m_btnEditTemplate.AutoSet(NXB_MODIFY);

		GetMainFrame()->DisableHotKeys();

		CString strFind = GetSharedPath() ^ "Templates\\Forms";
			
		FillSuperBillList(strFind);

		// Load the name of the default template
		CString strDefTemplate = GetPropertyText("DefaultSuperbillFilename", "{ No Default Superbill Filename }", 0, false);
		if (strDefTemplate != "{ No Default Superbill Filename }") {
			LPDISPATCH pDisp = NULL;
			long p = m_template->GetFirstRowEnum();

			// (a.walling 2008-08-12 16:13) - PLID 28108 - Perform our search ourselves for case insensitivity
			while (p)
			{
				m_template->GetNextRowEnum(&p, &pDisp);

				IRowSettingsPtr pRow(pDisp);

				CString strTemplateName = VarString(pRow->GetValue(0), "");
				if (strTemplateName.CompareNoCase(strDefTemplate) == 0) {
					pRow->ForeColor = RGB(255,0,0);
					GetDlgItem(IDC_DEFTEMPLATE_BTN)->EnableWindow(FALSE);

					m_nDefTemplateRowIndex = pRow->GetIndex();
					m_template->PutCurSel(m_nDefTemplateRowIndex);
				}

				pDisp->Release();
			}
		}
		/*
		long nIndex = m_template->FindByColumn(0,_bstr_t(strDefTemplate),0,TRUE);
		if (nIndex != -1) {
			m_template->CurSel = nIndex;
			m_nDefTemplateRowIndex = nIndex;
			IRowSettingsPtr(m_template->GetRow(nIndex))->ForeColor = RGB(255,0,0);
			GetDlgItem(IDC_DEFTEMPLATE_BTN)->EnableWindow(FALSE);
		}*/

		// If nothing else was selected, default to the first element
		if (m_template->CurSel == -1) {
			m_template->CurSel = 0;
		}

		if(m_template->GetRowCount() == 0) {
			// Nothing was placed into the list, which means we won't be able 
			// to run the superbill because there's no template to run it against
			MsgBox(MB_OK|MB_ICONINFORMATION, 
				"There are no templates in the shared templates Forms sub-folder:\n\n%s\n\n"
				"Without any superbill templates, the superbill cannot be run.", strFind);
			CDialog::OnCancel();
			return TRUE;
		}

		COleDateTime now = COleDateTime::GetCurrentTime();
		COleDateTime dt(now.GetYear(), 
						now.GetMonth(),
						now.GetDay(),
						0,0,0);
		dt += COleDateTimeSpan(1);
		m_from.SetValue(COleVariant(dt));
		m_to.SetValue(COleVariant(dt));

		//set the remember checkbox appropriately
		if(GetRemotePropertyInt("SuperbillRemember", 0, 0, GetCurrentUserName(), true) == 1)
			CheckDlgButton(IDC_REMEMBER_SELECTED, TRUE);
		else
			CheckDlgButton(IDC_REMEMBER_SELECTED, FALSE);

		//setup the location filter
		m_pLocation->PutCurSel(0);
		OnCheckLocationFilter();


		//select the items if it was chosen
		if(IsDlgButtonChecked(IDC_REMEMBER_SELECTED)) {

			CString strType, strRes;
			strType = GetRemotePropertyText("SuperbillRememberType", "", 0, GetCurrentUserName(), true);
			strRes = GetRemotePropertyText("SuperbillRememberRes", "", 0, GetCurrentUserName(), true);

			//now loop through all the items in strType and select them (if they still exist)
			//we're not going to bother with items that are no longer in existence
			long nComma = strType.Find(",");
			while(nComma > -1) {
				long nID = atol(strType.Left(nComma));

				//Move this item over - the easy way to do this is just to select the item on the left, and call OnAdd...()
				if(m_aptUnselected->FindByColumn(0, (long)nID, 0, VARIANT_TRUE) > -1)
					OnAptAdd();

				strType = strType.Right(strType.GetLength() - (nComma+1));

				nComma = strType.Find(",");
			}

			//now do the same thing for the resources
			nComma = strRes.Find(",");
			while(nComma > -1) {
				long nID = atol(strRes.Left(nComma));

				//Move this item over - the easy way to do this is just to select the item on the left, and call OnAdd...()
				if(m_resUnselected->FindByColumn(0, (long)nID, 0, VARIANT_TRUE) > -1)
					OnResAdd();

				strRes = strRes.Right(strRes.GetLength() - (nComma+1));

				nComma = strRes.Find(",");
			}
		}


		// (d.thompson 2009-09-24) - PLID 9975 - Recall the status of the "advanced template config" option
		if(GetRemotePropertyInt("SuperbillAdvancedTemplateConfigInUse", 0, 0, GetCurrentUserName(), true)) {
			CheckDlgButton(IDC_USE_ADVANCED_CONFIG, TRUE);
		}
		EnsureControls();
	} NxCatchAll(__FUNCTION__);

	return FALSE;
}

//#define MoveSelected(from, to)		{ long nIndex; while ((nIndex = GetFirstSelIndex(from)) >= 0) MoveItem(from, to, nIndex); }
//#define MoveAll(from, to)				{ for (long i = from.GetListCount()-1; i >= 0; i--) MoveItem(from, to, i); }

CString GetInList(_DNxDataListPtr &list, const CString &strFieldName)
{
	long nCount = list->GetRowCount();
	if (nCount > 0) 
	{	CString strAns = strFieldName + " IN (";
		for (long i = 0; i < nCount; i++) 
			strAns += AsString(list->Value[i][0]) + ", ";
		strAns.TrimRight(", ");
		strAns += ")";
		return strAns;
	} 
	else return "1=1";
}

CString CSuperBillDlg::GetPurposeFilter()
{
	// Only filter if there is both something selected, and something unselected (if 
	// everything is selected, OR everything is unselected, we assume no filtering)
	if (m_aptSelected->GetRowCount() > 0 && m_aptUnselected->GetRowCount() > 0) 
		return GetInList(m_aptSelected, "AptTypeID");
	else return "1=1";
}

CString CSuperBillDlg::GetResourceFilter()
{
	// Only filter if there is both something selected, and something unselected (if 
	// everything is selected, OR everything is unselected, we assume no filtering)

	if (m_resSelected->GetRowCount() > 0 && m_resUnselected->GetRowCount() > 0)
		return GetInList(m_resSelected, "AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID") + ")";
	else return "1=1";
}

bool CSuperBillDlg::GenerateBaseWhereClauseFromInterface(CString &strBaseWhere)
{
	COleDateTime dtFrom(m_from.GetValue());
	COleDateTime dtTo(m_to.GetValue());

	if (m_dateFilter.GetCheck()) {
		if ((COleDateTime)m_to.GetValue() < (COleDateTime)m_from.GetValue()){
			int nReturn = MsgBox(MB_ICONHAND + MB_OK, "The date range is invalid: The \"From\" date is after the \"To\" date.");
			return false;
		}
		strBaseWhere.Format("AppointmentsT.PatientID > 0 AND "
			"AppointmentsT.Date >= '%s' AND AppointmentsT.Date <= '%s' "
			"AND AppointmentsT.Status <> 4 AND (%s) AND (%s)",
			FormatDateTimeForSql(dtFrom, dtoDate), 
			FormatDateTimeForSql(dtTo, dtoDate), 
			GetPurposeFilter(), 
			GetResourceFilter());
	}
	else {
		strBaseWhere.Format("AppointmentsT.PatientID > 0 AND "
			"AppointmentsT.Status <> 4 AND (%s) AND (%s)",
			GetPurposeFilter(), 
			GetResourceFilter());
	}

	//DRT 2/10/2004 - PLID 10164 - Add location filter to the superbill
	if(IsDlgButtonChecked(IDC_CHECK_LOCATION_FILTER)) {
		if(m_pLocation->GetCurSel() == -1) {
			MsgBox("You must choose a location to filter on.");
			return false;
		}

		CString str;
		str.Format(" AND (AppointmentsT.LocationID = %li) ", VarLong(m_pLocation->GetValue(m_pLocation->GetCurSel(), 0)));
		strBaseWhere += str;
	}

	return true;
}

bool CSuperBillDlg::SetupMergeEngine(CMergeEngine *pmi)
{
	// (z.manning, 03/06/2008) - PLID 29131 - Need to load the sender merge fields
	if(!pmi->LoadSenderInfo(TRUE)) {
		return false;
	}

	if(m_dateSort.GetCheck() == BST_CHECKED) {
		pmi->m_strOrderBy = "DateStartTime";
	}
	else if(m_btnResourceDateSort.GetCheck() == BST_CHECKED) {
		// (z.manning 2009-08-03 16:30) - PLID 19645 - New option to sort by resource then by date.
		pmi->m_strOrderBy = "dbo.GetResourceString(SubQuery.AppointmentID), DateStartTime";
	}
	else {
		pmi->m_strOrderBy = "PersonT.Last, PersonT.First";
	}

	pmi->m_nFlags |= BMS_SUPERBILL | BMS_APPOINTMENT_BASED;

	// Decide whether to save the file and write to the history or just write to the history
	if (AllowSaveSuperbill()) {
		pmi->m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;
	} else {
		pmi->m_nFlags |= BMS_SAVE_HISTORY_NO_FILE;
	}

	if (g_bMergeAllFields) {
		pmi->m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;
	}

	//DRT 7/24/03 - merge straight to the printer
	if(IsDlgButtonChecked(IDC_MERGE_PRINTER)) {
		pmi->m_nFlags = (pmi->m_nFlags | BMS_MERGETO_PRINTER) & ~BMS_MERGETO_SCREEN;
	}

	return true;
}

//This function, given an array of IDs and some superbill group information, generates a where clause and 
//	determines the template that these IDs should be merged to.
//
//pary - An array of appointmentsT.ID values to be in the WHERE clause
//strWhereClause - Output of a comma separated list / WHERE clause
//strTemplatePath - Output of the chosen template
//strPathTable	- This can be either 'SuperbillTemplateTypePathsT' or 'SuperbillTemplateResourcePathsT'.  It relies
//	on the fact that these tables have identical structure.
//nGroupID - The groupID in the given strPathTable that we're referencing
//strPromptText - Optional text to provide only if multiple templates fit the criteria, the user will be required to pick
//		just one of the templates.
bool CSuperBillDlg::GenerateWhereClauseAndPathFromIDArray(const IN CDWordArray *pary, OUT CString &strWhereClause, OUT CString &strTemplatePath,
														  const IN CString strPathTable, const IN long nGroupID, const IN CString strPromptText)
{
	//Simply generate the where clause as a comma delimited list of IDs
	strWhereClause = "(AppointmentsT.ID IN (" + GenerateDelimitedListFromLongArray(*pary, ",") + "))";

	//Figuring out the template is trickier.  Use the path table provided by the caller.
	_RecordsetPtr prsTemplates = CreateParamRecordset("SELECT Path "
		"FROM " + strPathTable + " \r\n"
		"INNER JOIN MergeTemplatesT ON " + strPathTable + ".TemplateID = MergeTemplatesT.ID \r\n"
		"WHERE " + strPathTable + ".GroupID = {INT}", nGroupID);
	if(!prsTemplates->eof) {
		if(prsTemplates->GetRecordCount() == 1) {
			//Easy, there's just 1, so use it.
			strTemplatePath = AdoFldString(prsTemplates, "Path");
		}
		else {
			//There are more than 1 templates, so we need to prompt the user with the list.
			CStringArray aryPaths;
			while(!prsTemplates->eof) {
				CString strPath = AdoFldString(prsTemplates, "Path");
				//We just want the last part of the path, none of the leading-up folders.
				aryPaths.Add(FileUtils::GetFileName(strPath));
				prsTemplates->MoveNext();
			}

			//Now do the prompting with the given list.
			CConfigureSuperbillAddFileDlg dlg(this);
			dlg.m_strPromptText = strPromptText;
			dlg.OverrideTemplateList(&aryPaths);
			if(dlg.DoModal() == IDOK) {
				//the chosen one!
				strTemplatePath = "Templates\\Forms\\" + dlg.m_strResults_SuperbillPath;
			}
			else {
				//Cancelling any prompt aborts the entire process
				return false;
			}
		}
	}
	else {
		//This is considered a nonsense / undefined error.  It means that we have called this function with a given group, but there
		//	are no type templates for that group.  Either we have failed to properly detect before coming to this code, or there's
		//	bad data of groups without templates.
		AfxMessageBox("Your configuration of templates for appointments types is invalid.  Please review the groups and ensure they are "
			"setup correctly.");
		return false;
	}

	return true;
}

void CSuperBillDlg::OnPrintSuperbills() 
{
	try {
		CWaitCursor wait;

		// (z.manning 2009-10-21 16:06) - PLID 35990 - Remember the sort option
		EDefaultSortRadio eSortOption;
		if(m_nameSort.GetCheck() == BST_CHECKED) { eSortOption = dsrResource; }
		else if(m_btnResourceDateSort.GetCheck() == BST_CHECKED) { eSortOption = dsrResourceThenDate; }
		else { eSortOption = dsrDate; }
		SetRemotePropertyInt("SuperbillDialogDefaultSort", eSortOption, 0, GetCurrentUserName());

		// Maks sure word is installed
		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}

		if (m_template->CurSel == -1)
			return;
		// Assemble the list of all qualifying appointments

		// (d.thompson 2009-09-24) - PLID 9975 - Re-grouped things so that we can use the Advanced Template Config, allowing a single
		//	merge action to handle multiple templates.  Most of the existing code remained intact, I just had to move things around
		//	to a few loops and arrays.

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//This will be the designed behavior (whether the advanced config is on or not)
		//	1)  Generate the where clause for all appointments determining the types, resources, dates, and location.
		//	2)  Configure the merge engine, all merges will share the same settings.
		//BEGIN LOOP
		//	3)  Break all the appointments into their appropriate grouping.  All appointments which will be merged to a type-group
		//		go into a map (per group), all appts for a resource-group go into a map (per group), and all remaining appts
		//		remain in a "leftover" map.  Any groups with no elements are removed at this step.
		//	4)  Build the merge clause for each grouping and determine what template should be merged to at the same time.  If there
		//		are multiple allowed templates, prompt the user to make a selection.  Queue the where clause of IDs and the template
		//		path into our main queue for execution.
		//	5)  Iterate through all the execution items in the queue, generating the temp table with the available Appt IDs 
		//		from the step 3 where clause.
		//	6)  Get the template name for the current execution item.
		//	7)  PERFORM THE MERGE
		//END LOOP
		//	8)  Remember UI elements in data and close.
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//
		//1)  Generate the base WHERE clause
		CString strBaseWhere;
		if(!GenerateBaseWhereClauseFromInterface(strBaseWhere)) {
			return;
		}
		//

		//1.5)  Check for existing superbills! // (a.walling 2010-09-15 09:20) - PLID 7018
		ExistingSuperBillInfo::Action action = ExistingSuperBillInfo::PrintAll;

		ExistingSuperBillInfo existingInfo(this);

		try {
			existingInfo.Load(strBaseWhere);

			action = existingInfo.Prompt();
			
			if (action == ExistingSuperBillInfo::Cancel) {
				return;
			}
		} NxCatchAllThrow("Checking for existing superbills");
		//
		//End Step 1
		//

		//
		//Step 2 - Setup the merge engine
		CMergeEngine mi;
		if(!SetupMergeEngine(&mi)) {
			return;
		}
		//End Step 2
		//

		// (a.walling 2010-09-15 09:20) - PLID 7018 - Set up our temp table
		// GetNewUniqueID now in NxSystemUtilitiesLib
		CString strTempPrintedSuperBillsT;
		strTempPrintedSuperBillsT.Format("#Temp%s", NewUUID());
		while (strTempPrintedSuperBillsT.Replace("-", "")) {};
		ExecuteSql(
			"SELECT DISTINCT PrintedSuperBillsT.ReservationID "
			"INTO %s "
			"FROM AppointmentsT "
			"LEFT JOIN PrintedSuperBillsT "
				"ON AppointmentsT.ID = PrintedSuperBillsT.ReservationID AND PrintedSuperBillsT.Void = 0 "
			"LEFT JOIN AppointmentResourceT "
				"ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
			"WHERE "
				"%s "
				"AND PrintedSuperBillsT.Void = 0"
			, strTempPrintedSuperBillsT
			, strBaseWhere);

		CString strBaseWhereOnlyFresh;
		strBaseWhereOnlyFresh.Format("%s AND AppointmentsT.ID NOT IN (SELECT ReservationID FROM %s)", strBaseWhere, strTempPrintedSuperBillsT);
		CString strBaseWhereOnlyExisting;
		strBaseWhereOnlyExisting.Format("%s AND AppointmentsT.ID IN (SELECT ReservationID FROM %s)", strBaseWhere, strTempPrintedSuperBillsT);

		// (a.walling 2010-09-15 09:20) - PLID 7018 - Everything else has moved into PrintSuperbills function so we can easily call it with two partitions if necessary
		switch (action) {
			case ExistingSuperBillInfo::PrintNew:
				PrintSuperbills(mi, strBaseWhereOnlyFresh, true);
				break;
			case ExistingSuperBillInfo::PrintNewWithCopies:
				if (existingInfo.m_nFreshSuperBills > 0) {
					PrintSuperbills(mi, strBaseWhereOnlyFresh, true);
				}
				PrintSuperbills(mi, strBaseWhereOnlyExisting, false);
				break;
			case ExistingSuperBillInfo::PrintAll:
				PrintSuperbills(mi, strBaseWhere, true);
				break;
			default:
				ASSERT(FALSE);
				break;
		}
		
		//
		//Step 8 - Remember any UI elements and close
		if(IsDlgButtonChecked(IDC_REMEMBER_SELECTED)) {
			SaveSelectedItems();
		}
		else {
			//clear out the 2 settings
			SetRemotePropertyText("SuperbillRememberType", "", 0, GetCurrentUserName());
			SetRemotePropertyText("SuperbillRememberRes", "", 0, GetCurrentUserName());
		}

		//remember if this box is checked
		long nRemember = 0;
		if(IsDlgButtonChecked(IDC_REMEMBER_SELECTED))
			nRemember = 1;
		SetRemotePropertyInt("SuperbillRemember", nRemember, 0, GetCurrentUserName());

		// And close the dialog
		CDialog::OnOK();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2010-09-15 09:20) - PLID 7018 - bIsFresh determines whether this is a new superbill or a re-printed copy
void CSuperBillDlg::PrintSuperbills(CMergeEngine& mi, const CString& strBaseWhere, bool bIsFresh)
{
	try {			
		// (a.walling 2010-09-15 09:20) - PLID 7018 - ensure we have the right flags if this is a fresh merge or a reprint
		if (bIsFresh) {
			mi.m_nFlags &= ~BMS_REPRINT;
		} else {
			mi.m_nFlags |= BMS_REPRINT;
		}

		//
		//Template loop generation.  We need to define what loops we are going to process based on how the template
		//	configuration is setup.
		CStringArray aryWhereClauses;		//Accessed by the loop counter
		CMap<long, long, CString, LPCTSTR> mapClauseTemplates;		//aryWhereClauses.GetAt() index -> CString of the template path

		//These are a few maps we need to keep track of
		CMap<long, long, CDWordArray*, CDWordArray*> mapTypeGroups;		//GroupID -> Array of Appointment IDs
		CMap<long, long, CDWordArray*, CDWordArray*> mapResourceGroups;	//GroupID -> Array of Appointment IDs
		if(IsDlgButtonChecked(IDC_USE_ADVANCED_CONFIG)) {
			//
			//This is difficult.  We have a base where clause of appointments, all of which could have entirely different
			//	setups for their merging.  We're going to have to query the list of appointments, and for each one, determine
			//	the correct template that should be merged to.  In many cases it may require prompting the user.
			//

			//A)  Query all the appointments and generate maps per type and resource and total
			CMap<long, long, bool, bool> mapAllAppts;			//Just a list of IDs and a flag if they exist
			CMap<long, long, long, long> mapApptsToTypes;		//Appt ID to Type ID
			CMap<long, long, CDWordArray*, CDWordArray*> mapApptsToResources;		//Appt ID to list of Resource IDs
			FillAppointmentMapsFromBaseQuery(strBaseWhere, &mapAllAppts, &mapApptsToTypes, &mapApptsToResources);

			//Check for failure -- no appointments
			if(mapAllAppts.GetSize() == 0) {
				AfxMessageBox("No appointments meet your selected criteria.");
				return;
			}

			//This code pulls heavily from the design in globalschedutils.cpp, GetDefaultSuperbillTemplates()
			bool bUseTypes = GetRemotePropertyInt("SuperbillConfig_Types", 0, 0, "<None>", true) == 0 ? false : true;
			bool bUseResources = GetRemotePropertyInt("SuperbillConfig_Resources", 0, 0, "<None>", true) == 0 ? false : true;
			bool bUsePickList = GetRemotePropertyInt("SuperbillConfig_PickList", 0, 0, "<None>", true) == 0 ? false : true;
			bool bPromptIfEmpty = GetRemotePropertyInt("SuperbillConfig_UseGlobal", 1, 0, "<None>", true) == 1 ? false : true;

			//These must be filled in order
			if(bUseTypes) {
				//B)  Iterate through all the type groups.  Assign appointments into each type group appropriately, and remove them from the
				//	"all appointments" map.  Our goal here is to fill the 'mapTypeGroups' map.
				FillTypeGroupMap(&mapTypeGroups, &mapAllAppts, &mapApptsToTypes, &mapApptsToResources);

				//Iterate through the type group map.  For each type group, determine which template we'll be using, and the where
				//	clause for that template.
				POSITION pos = mapTypeGroups.GetStartPosition();
				while(pos != NULL) {
					long nGroupID;
					CDWordArray *pary;
					mapTypeGroups.GetNextAssoc(pos, nGroupID, pary);

					//Ensure there were some appointments for this group
					if(pary->GetSize() > 0) {
						//The where clause is simple -- turn the dword array into a string
						CString strWhereClause;
						CString strTemplatePath;

						//All the work and possible prompting is done in here.
						if(!GenerateWhereClauseAndPathFromIDArray(pary, strWhereClause, strTemplatePath, "SuperbillTemplateTypePathsT", nGroupID,
							"Some appointments (grouped by type) meet the criteria for more than one template.  Please choose the template "
							"you wish to merge these to below.")) {
							return;
						}

						//Set the path to the template for this clause.
						mapClauseTemplates.SetAt(aryWhereClauses.GetSize(), strTemplatePath);
						//now add it into our array of clauses
						aryWhereClauses.Add(strWhereClause);
					}
					else {
						//There are no appointments that match this GroupID, so just skip it entirely.
					}
				}
			}

			//Now that we have finished the type research, we no longer need the map of appts to types.
			mapApptsToTypes.RemoveAll();

			if(bUseResources) {
				//C)  Almost exactly the same thing as above, but done with resource IDs.  This is a little trickier because appointments
				//	can have more than 1 resource.
				//NOTE:  I have intentionally decided that we're going to pick a "random*" resource for each appointment.  the prompt
				//	possibilities otherwise could get out of hand quickly and just become too difficult to manage.
				//*Really it's matching the resource in the earliest ID'd resource group, so most likely the one that was created first.
				FillResourceGroupMap(&mapResourceGroups, &mapAllAppts, &mapApptsToResources);

				//Iterate through the resource group map.  For each resource group, determine which template we'll be using, and the where
				//	clause for that template.
				POSITION pos = mapResourceGroups.GetStartPosition();
				while(pos != NULL) {
					long nGroupID;
					CDWordArray *pary;
					mapResourceGroups.GetNextAssoc(pos, nGroupID, pary);

					//Ensure there were some appointments for this group
					if(pary->GetSize() > 0) {
						//The where clause is simple -- turn the dword array into a string
						CString strWhereClause;
						CString strTemplatePath;

						if(!GenerateWhereClauseAndPathFromIDArray(pary, strWhereClause, strTemplatePath, "SuperbillTemplateResourcePathsT", 
							nGroupID, "Some appointments (grouped by resource) meet the criteria for more than one template.  Please choose the template "
							"you wish to merge these to below.")) {
							return;
						}

						//Set the path to the template for this clause.
						mapClauseTemplates.SetAt(aryWhereClauses.GetSize(), strTemplatePath);
						//now add it into our array of clauses
						aryWhereClauses.Add(strWhereClause);
					}
					else {
						//there are no appointments that match this group, so skip it entirely
					}
				}
			}

			//Now that we have finished the resource research, we no longer need the map of appts to resources.
			FreeMemoryInPointerArrayMap(&mapApptsToResources);


			//All of the remaining 3 options use the same methodology.
			if(mapAllAppts.GetCount() > 0) {
				CString strTemplatePath;

				//There where clause is going to be the same no matter what -- it's all remaining appts
				CString strWhereClause = "AppointmentsT.ID IN (";
				{
					POSITION pos = mapAllAppts.GetStartPosition();
					while(pos != NULL) {
						long nID;
						bool bIgnored = false;
						mapAllAppts.GetNextAssoc(pos, nID, bIgnored);
						strWhereClause += AsString(nID) + ",";
					}
					strWhereClause.TrimRight(",");
					strWhereClause += ")";
				}

				//The template will vary based on the method the user has setup
				if(bUsePickList) {
					//D)  Now we've processed types and resources, and removed all "found" appts from the "total" map.  All the leftover
					//	appointments will be processed using the "prompt" list (a set of pre-defined templates).
					CStringArray aryPaths;
					_RecordsetPtr prsPick = CreateParamRecordset("SELECT Path FROM SuperbillTemplatePickListT "
						"INNER JOIN MergeTemplatesT ON SuperbillTemplatePickListT.TemplateID = MergeTemplatesT.ID");
					while(!prsPick->eof) {
						CString strPath = AdoFldString(prsPick, "Path");
						aryPaths.Add(FileUtils::GetFileName(strPath));
						prsPick->MoveNext();
					}

					//i)  Pick the template to use
					CConfigureSuperbillAddFileDlg dlg(this);
					dlg.m_strPromptText = "Your configuration requires that you choose which template all remaining appointments "
						"should be merged to.  Please choose the template from the pre-set list below.";
					dlg.OverrideTemplateList(&aryPaths);
					if(dlg.DoModal() == IDOK) {
						strTemplatePath = "Templates\\Forms\\" + dlg.m_strResults_SuperbillPath;
					}
					else {
						//Cancelling any prompt aborts the entire process
						return;
					}
				}
				else {
					if(bPromptIfEmpty) {
						//E)  All the appointments remaining in the 'total' map should be merged to whatever the user chooses on this prompt.
						CConfigureSuperbillAddFileDlg dlg(this);
						dlg.m_strPromptText = "Your configuration requires that you choose which template all remaining appointments "
							"should be merged to.  Please choose the template below.";
						if(dlg.DoModal() == IDOK) {
							strTemplatePath = "Templates\\Forms\\" + dlg.m_strResults_SuperbillPath;
						}
						else {
							//Cancelling any prompt aborts the entire process
							return;
						}
					}
					else {
						//F)  All the appointments remaining in the 'total' map should be merged to the global template.  The global is 
						//	saved as just the filename after templates\forms.
						strTemplatePath = "Templates\\Forms\\" + GetPropertyText("DefaultSuperbillFilename", "", 0, false);
					}
				}

				//Set the path to the template for this clause.
				mapClauseTemplates.SetAt(aryWhereClauses.GetSize(), strTemplatePath);
				//now add it into our array of clauses
				aryWhereClauses.Add(strWhereClause);
			}
		}
		else {
			//Simple case, go back to what we used to do in a single loop
			// Get the template name
			CString strTemplateName, strTemplatePath;
			//The files are populated from templates\forms automatically, so prepend that
			strTemplateName = "Templates\\Forms\\" + VarString(m_template->GetValue(m_template->CurSel, 0));

			mapClauseTemplates.SetAt(0, strTemplateName);
			aryWhereClauses.Add(strBaseWhere);
		}

		//BEGIN LOOPING
		for(int nCurrentLoop = 0; nCurrentLoop < aryWhereClauses.GetSize(); nCurrentLoop++) {
			//
			//Step 3 - Build a merge clause

			//Build a list of appointments (not patients!) from which to merge
			// (a.walling 2007-11-07 11:42) - PLID 27998 - VS2008 - Cannot add two pointers
			CString strCurrentLoopWhere = aryWhereClauses.GetAt(nCurrentLoop);
			CString strSql;
			strSql = CString("SELECT AppointmentsT.ID AS ID, AppointmentsT.PatientID AS PatientID FROM AppointmentsT ")
				+ (strCurrentLoopWhere.IsEmpty() ? CString("") : (CString(" WHERE ") + strCurrentLoopWhere));

			//End Step 3
			//

			//
			//Step 4 - Test to see if there is anything in our current merge clause
			// (z.manning 2008-12-10 15:18) - PLID 32397 - Fixed IsRecordsetEmpty call to prevent text formatting errors
			if (IsRecordsetEmpty("%s", strSql)) {
				// (d.thompson 2009-10-20) - PLID 9975 - This should now be impossible in advanced config.  It may still happen
				//	for the "old" mode.
				MsgBox("No patients match this criteria!");// No records so don't do the merge
			}
			else {
				//
				//Step 5 - Create the temp table filled with appointment IDs we'll be merging to

				// Create the temp table with just the patient IDs
				CString strMergeT;
				{
					CStringArray aryFieldNames, aryFieldTypes;
					aryFieldNames.Add("ID"); // Appointment id
					aryFieldTypes.Add("INT");
					aryFieldNames.Add("PatientID"); // Patient id (obviously)
					aryFieldTypes.Add("INT");
					strMergeT = CreateTempIDTable(strSql, aryFieldNames, aryFieldTypes, TRUE, TRUE, NULL);
				}

				//End Step 5
				//

				// (d.thompson 2009-10-20) - PLID 9975
				//According to my note in superbill.cpp from long ago, the m_strResFilter further filters the list of things being
				//	merged on top of the ID list you already gave it.  Since we're doing all the filtering out here, and the
				//	ID list is already filtered, there's no reason we'd want to reduce it anymore, so just pass through our where
				mi.m_strResFilter = strCurrentLoopWhere;

				//
				//Step 6 - Determine the template name we are merging to

				// Get the template name from our map for this where clause
				CString strTemplatePath;
				if(!mapClauseTemplates.Lookup(nCurrentLoop, strTemplatePath)) {
					//This shouldn't be possible without a bug in code putting the path in the wrong map
					AfxThrowNxException("Unable to determine the template to use for merging superbills.");
				}

				//Set to the full path.  By definition, superbills are required to live in Templates\\forms, and nowhere
				//	else.  The configuration constrains to those requirements as well, so we know our path will be
				//	shared path + template path.
				strTemplatePath = GetSharedPath() ^ strTemplatePath;

				//End Step 6
				//

				//
				//Step 7 - Perform the merge
				// (z.manning 2016-06-03 8:41) - NX-100806 - Check if the merge was successful
				if (!mi.MergeToWord(strTemplatePath, std::vector<CString>(), strMergeT, "PatientID")) {
					break;
				}
				//End Step 7
				//
			}//end else
		}//END MAIN MERGE LOOP


		//
		//	Cleanup!  We need to destroy all those fancy maps we created above, in reverse order.
		//
		//Map of resource groups to appts that fit in the groups
		FreeMemoryInPointerArrayMap(&mapResourceGroups);

		//Map of type groups to appointments that fit in the groups
		FreeMemoryInPointerArrayMap(&mapTypeGroups);

		//end Step 8
		//
	} NxCatchAll(__FUNCTION__);
}

//Given a map that points longs to CDWordArray* objects, iterate through the map and free all the pointer objects.
//The key/value pairs in the map are not touched, so the map should no longer be used, as it contains invalid pointers.
void CSuperBillDlg::FreeMemoryInPointerArrayMap(CMap<long, long, CDWordArray*, CDWordArray*> *pmap)
{
	POSITION pos = pmap->GetStartPosition();
	while(pos != NULL) {
		long nKey;
		CDWordArray *pary;
		pmap->GetNextAssoc(pos, nKey, pary);

		//Must destroy the array of integers
		if(pary) {
			delete pary;
		}
	}
}

//Given a base WHERE clause against an AppointmentsT table, fills the 3 maps with the appointment information pulled from the query results.
//	pmapAllAppts - Just the IDs (boolean flag set true) of all appts returned
//	pmapApptsToTypes - All appointments and their AptTypeID values
//	pmapApptsToResources - All appointments and an array of their resources
void CSuperBillDlg::FillAppointmentMapsFromBaseQuery(CString strBaseWhere, CMap<long, long, bool, bool> *pmapAllAppts, 
													 CMap<long, long, long, long> *pmapApptsToTypes, 
													 CMap<long, long, CDWordArray*, CDWordArray*> *pmapApptsToResources)
{
	_RecordsetPtr prs = CreateRecordset("SELECT AppointmentsT.ID, AppointmentsT.AptTypeID, AppointmentResourceT.ResourceID "
			"FROM AppointmentsT "
			"LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
			"WHERE " + strBaseWhere);
	while(!prs->eof) {
		long nApptID = AdoFldLong(prs, "ID");
		long nTypeID = AdoFldLong(prs, "AptTypeID", -1);
		long nResourceID = AdoFldLong(prs, "ResourceID", -1);

		//1)  Add to 'all' map
		bool bNotUsed = false;
		if(!pmapAllAppts->Lookup(nApptID, bNotUsed)) {
			//Not already there, add it
			pmapAllAppts->SetAt(nApptID, true);
		}

		//2)  Add to 'type' map
		long nType = -1;
		if(!pmapApptsToTypes->Lookup(nApptID, nType)) {
			//Not already there, add it
			pmapApptsToTypes->SetAt(nApptID, nTypeID);
		}

		//3)  Add to 'resource' map list
		CDWordArray *pary = NULL;
		if(!pmapApptsToResources->Lookup(nApptID, pary)) {
			//Doesn't exist yet, we need to create it
			pary = new CDWordArray();
			pmapApptsToResources->SetAt(nApptID, pary);
		}
		//Either way, we now have a pointer to the resource array, so add this resource ID.  It should be impossible for
		//	the database to contain to records for the same resource on the same appt, so we will not check for duplicates.
		pary->Add(nResourceID);

		prs->MoveNext();
	}
}

//Given the 3 maps of appointments, fill the map of type groups by querying which appointments
//	match into each group by type.  Any appointments that match will be removed from all three source maps.
void CSuperBillDlg::FillTypeGroupMap(CMap<long, long, CDWordArray*, CDWordArray*> *pmapTypeGroups,
										CMap<long, long, bool, bool> *pmapAllAppts, 
										CMap<long, long, long, long> *pmapApptsToTypes, 
										CMap<long, long, CDWordArray*, CDWordArray*> *pmapApptsToResources)
{
	_RecordsetPtr prsTypeGroups = CreateParamRecordset("SELECT GroupID, TypeID FROM SuperbillTemplateTypeT ORDER BY GroupID");
	while(!prsTypeGroups->eof) {
		long nGroupID = AdoFldLong(prsTypeGroups, "GroupID");
		long nGroupTypeID = AdoFldLong(prsTypeGroups, "TypeID");

		CDWordArray *paryAppts = NULL;
		if(!pmapTypeGroups->Lookup(nGroupID, paryAppts)) {
			//No appts for this group yet, so create an array
			paryAppts = new CDWordArray();
			//add to map
			pmapTypeGroups->SetAt(nGroupID, paryAppts);
		}

		//Now we have an array for the current group (all types are combined and cannot be in multiple groups), 
		//	so iterate through our map of appts -> types, and if they match the current type, save them for this merge.
		POSITION pos = pmapApptsToTypes->GetStartPosition();
		while(pos != NULL) {
			long nThisApptID, nThisTypeID;
			pmapApptsToTypes->GetNextAssoc(pos, nThisApptID, nThisTypeID);

			if(nThisTypeID == nGroupTypeID) {
				//The particular appt in the map fits into this group, so add its ID
				paryAppts->Add(nThisApptID);
				//And now wipe it from the "all appts" map, because we don't want it to merge in
				//	more than 1 set.
				pmapAllAppts->RemoveKey(nThisApptID);
				//Remove from the resource map too so we don't hit it later.  We need to cleanup memory too
				CDWordArray *paryToDelete;
				if(pmapApptsToResources->Lookup(nThisApptID, paryToDelete)) {
					pmapApptsToResources->RemoveKey(nThisApptID);
					delete paryToDelete;
				}
				//Remove from our type map as well
				pmapApptsToTypes->RemoveKey(nThisApptID);
			}
		}

		prsTypeGroups->MoveNext();
	}
}

//Given the two appointment maps, fills the resource group map with appointment IDs per group.  Any appointments that match
//	a group will be removed from the two source maps.
void CSuperBillDlg::FillResourceGroupMap(CMap<long, long, CDWordArray*, CDWordArray*> *pmapResourceGroups, 
										CMap<long, long, bool, bool> *pmapAllAppts, 
										CMap<long, long, CDWordArray*, CDWordArray*> *pmapApptsToResources)
{
	_RecordsetPtr prsResourceGroups = CreateParamRecordset("SELECT GroupID, ResourceID FROM SuperbillTemplateResourceT ORDER BY GroupID");
	while(!prsResourceGroups->eof) {
		long nGroupID = AdoFldLong(prsResourceGroups, "GroupID");
		long nGroupResourceID = AdoFldLong(prsResourceGroups, "ResourceID");

		CDWordArray *paryAppts = NULL;
		if(!pmapResourceGroups->Lookup(nGroupID, paryAppts)) {
			//No appts for this group yet, so create an array
			paryAppts = new CDWordArray();
			//add to map
			pmapResourceGroups->SetAt(nGroupID, paryAppts);
		}

		//Now we have an array for the current group, so start iterating through our
		//	map of appts to resources.  To simplify things, we are intentionally 
		//	just taking the first resource/appt combo that we match.
		POSITION pos = pmapApptsToResources->GetStartPosition();
		while(pos != NULL) {
			long nThisApptID;
			CDWordArray *paryResources;
			pmapApptsToResources->GetNextAssoc(pos, nThisApptID, paryResources);

			if(IsIDInArray(nGroupResourceID, paryResources)) {
				//The particular appt in the map fits into this group, so add its ID
				paryAppts->Add(nThisApptID);
				//And now wipe it from the "all appts" map, because we don't want it to merge in
				//	more than 1 set.
				pmapAllAppts->RemoveKey(nThisApptID);
				//Remove from resource map so we don't hit it again.  We need to cleanup memory too
				CDWordArray *paryToDelete;
				if(pmapApptsToResources->Lookup(nThisApptID, paryToDelete)) {
					pmapApptsToResources->RemoveKey(nThisApptID);
					delete paryToDelete;
				}
			}
		}

		prsResourceGroups->MoveNext();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CSuperBillDlg::OnChangeFrom(NMHDR* pNMHDR, LRESULT* pResult) 
{
// PLID 11717  This is an annoying feature when trying to set the date
//	if ((COleDateTime)m_to.GetValue().date < (COleDateTime)m_from.GetValue().date)
//		m_to.SetValue(m_from.GetValue());	
	
	*pResult = 0;
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CSuperBillDlg::OnChangeTo(NMHDR* pNMHDR, LRESULT* pResult)
{
// PLID 11717  This is an annoying feature when trying to set the date
//	if ((COleDateTime)m_to.GetValue().date < (COleDateTime)m_from.GetValue().date)
//		m_from.SetValue(m_to.GetValue());

	*pResult = 0;
}

void CSuperBillDlg::OnAptAdd()
{
	try {
		m_aptSelected->TakeCurrentRow(m_aptUnselected);
	} NxCatchAll(__FUNCTION__);
}

void CSuperBillDlg::OnAptRemove()			
{
	try {
		m_aptUnselected->TakeCurrentRow(m_aptSelected); 
	} NxCatchAll(__FUNCTION__);
}

void CSuperBillDlg::OnResAdd()				
{ 
	try {
		m_resSelected->TakeCurrentRow(m_resUnselected);
	} NxCatchAll(__FUNCTION__);
}

void CSuperBillDlg::OnResRemove()			
{
	try {
		m_resUnselected->TakeCurrentRow(m_resSelected);
	} NxCatchAll(__FUNCTION__);
}

void CSuperBillDlg::OnResAddAll()			
{
	try {
		m_resSelected->TakeAllRows(m_resUnselected); 
	} NxCatchAll(__FUNCTION__);
}

void CSuperBillDlg::OnResRemoveAll()		
{ 
	try {
		m_resUnselected->TakeAllRows(m_resSelected); 
	} NxCatchAll(__FUNCTION__);
}

void CSuperBillDlg::OnAptAddAll()
{ 
	try {
		m_aptSelected->TakeAllRows(m_aptUnselected);
	} NxCatchAll(__FUNCTION__);
}

void CSuperBillDlg::OnAptRemoveAll()
{
	try {
		m_aptUnselected->TakeAllRows(m_aptSelected); 
	} NxCatchAll(__FUNCTION__);
}

void CSuperBillDlg::OnDblClickCellResUnselected(long nRowIndex, short nColIndex) 
{
	OnResAdd();
}

void CSuperBillDlg::OnDblClickCellResSelected(long nRowIndex, short nColIndex) 
{
	OnResRemove();
}

void CSuperBillDlg::OnDblClickCellTypeUnselected(long nRowIndex, short nColIndex) 
{
	OnAptAdd();
}

void CSuperBillDlg::OnDblClickCellTypeSelected(long nRowIndex, short nColIndex) 
{
	OnAptRemove();
}

void CSuperBillDlg::OnCancel() 
{
	CDialog::OnCancel();	
}

void CSuperBillDlg::OnSelChosenTemplate(long nRow) 
{
	if (nRow != -1) {
		try {
			CString strTemplate = VarString(m_template->Value[nRow][0]);
			CString strDefaultTemplate = GetPropertyText("DefaultSuperbillFilename", "{ No Default Superbill Filename }", 0, false);
			if (strTemplate.CompareNoCase(strDefaultTemplate) != 0) {
				// This is different from the default so allow the checkbox
				GetDlgItem(IDC_DEFTEMPLATE_BTN)->EnableWindow(TRUE);
			} else {
				// This is the same as the default so disallow the checkbox
				GetDlgItem(IDC_DEFTEMPLATE_BTN)->EnableWindow(FALSE);
			}
		} NxCatchAll("CSuperBillDlg::OnSelChosenTemplate");
	}
}

void CSuperBillDlg::OnDeftemplateBtn() 
{
	try {
		// Get the current selection
		long nIndex = m_template->CurSel;

		// If we made it here we had success so save any settings
		CString strTemplateName;
		strTemplateName = (LPCSTR)_bstr_t(m_template->Value[nIndex][0]);
		SetPropertyText("DefaultSuperbillFilename", strTemplateName, 0);

		// Change the datalist text colors and remember the new default
		if (m_nDefTemplateRowIndex != -1) {
			IRowSettingsPtr(m_template->Row[m_nDefTemplateRowIndex])->ForeColor = dlColorNotSet;
		}
		m_nDefTemplateRowIndex = nIndex;
		IRowSettingsPtr(m_template->Row[m_nDefTemplateRowIndex])->ForeColor = RGB(255,0,0);
		
		GetDlgItem(IDC_DEFTEMPLATE_BTN)->EnableWindow(FALSE);
	} NxCatchAll("CSuperBillDlg::OnDeftemplateBtn");
}

void CSuperBillDlg::OnEditTemplateBtn() 
{
	try {
		if (!CheckCurrentUserPermissions(bioLWEditTemplate, sptView))
			return;

		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}

		CString path, strInitPath;

		// (a.walling 2007-06-14 16:02) - PLID 26342 - Support Word 2007 templates
		// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
		CString strFilter;
		// Always support Word 2007 templates
		strFilter = "Microsoft Word Templates|*.dot;*.dotx;*.dotm||";

		CFileDialog dlg(TRUE, "dot", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
		strInitPath = GetTemplatePath() + "Forms\\"; // We need to store this because the next line is a pointer to it
		dlg.m_ofn.lpstrInitialDir = strInitPath;
		dlg.m_ofn.lpstrTitle = "Select a template to edit";
		
		if (dlg.DoModal() == IDOK) {
			path = dlg.GetPathName();
		} else {
			return;
		}

		CString strMergeInfoFilePath;

		try {
			// (z.manning 2016-02-12 13:57) - PLID 68230 - Use the base word processor app class
			std::shared_ptr<CGenericWordProcessorApp> pApp = GetWPManager()->GetAppInstance();
			if (nullptr == pApp) return; // (c.haag 2016-06-01 11:48) - NX-100320 - If it's null then it's not supported and an exception was not thrown
			pApp->EnsureValid();

			// Create an empty MergeInfo.nxt
			long nFlags = BMS_HIDE_ALL_DATA	| BMS_DEFAULT | /*BMS_HIDE_PRACTICE_INFO |*/
						/*BMS_HIDE_PERSON_INFO | BMS_HIDE_DATE_INFO |*/ BMS_HIDE_PRESCRIPTION_INFO |
						/*BMS_HIDE_CUSTOM_INFO | BMS_HIDE_INSURANCE_INFO |*/ BMS_HIDE_BILL_INFO |
						BMS_HIDE_PROCEDURE_INFO /*| BMS_HIDE_DOCTOR_INFO*/;

			strMergeInfoFilePath = CMergeEngine::CreateBlankMergeInfo(nFlags, NULL, NULL);
			if (!strMergeInfoFilePath.IsEmpty()) {

				// Open the template
				// (z.manning 2016-02-18 09:18) - PLID 68366 - This now returns a word processor document object
				// (c.haag 2016-04-22 10:40) - NX-100275 - OpenTemplate no longer returns a document. We never did anything with it except throw an exception if it were null
				// anyway, and now OpenTemplate does that for us
				pApp->OpenTemplate(path, strMergeInfoFilePath);

				// We can't delete the merge info text file right now because it is in use, but 
				// it's a temp file so mark it to be deleted after the next reboot
				DeleteFileWhenPossible(strMergeInfoFilePath);
				strMergeInfoFilePath.Empty();
			} else {
				AfxThrowNxException("Could not create blank merge info");
			}
			// (c.haag 2016-02-23) - PLID 68416 - We no longer catch Word-specific exceptions here. Those are now managed deep within the WordProcessor application object
		}NxCatchAll("SuperbillDlg::OnEditTemplateBtn:Outside");

		if (!strMergeInfoFilePath.IsEmpty()) {
			// This means the file wasn't used and/or it wasn't 
			// marked for deletion at startup, so delete it now
			DeleteFile(strMergeInfoFilePath);
		}
	} NxCatchAll(__FUNCTION__);
}

BOOL CSuperBillDlg::DestroyWindow() 
{
	try {
		GetMainFrame()->EnableHotKeys();
	} NxCatchAll(__FUNCTION__);

	return CNxDialog::DestroyWindow();
}

void CSuperBillDlg::OnRememberSelected() 
{
}

void CSuperBillDlg::OnShowInactiveResources()
{
	try {
		if(IsDlgButtonChecked(IDC_CHECK_SB_SHOWINACTIVERESOURCES)) {
			m_resUnselected->WhereClause = "";
		}
		else {
			m_resUnselected->WhereClause = "Inactive = 0";
		}
		m_resSelected->Clear();
		m_resUnselected->Requery();
	} NxCatchAll(__FUNCTION__);
}

//saves the items that are selected currently
void CSuperBillDlg::SaveSelectedItems()
{
	CString strType, strRes;
	CString str;	//temp

	//loop through and get all the selected types
	for(int i = 0; i < m_aptSelected->GetRowCount(); i++) {
		str.Format("%li,", VarLong(m_aptSelected->GetValue(i, 0)));
		strType += str;
	}

	//loop through and get all the selected resources
	for(i = 0; i < m_resSelected->GetRowCount(); i++) {
		str.Format("%li,", VarLong(m_resSelected->GetValue(i, 0)));
		strRes += str;
	}

	//now save 'em
	SetRemotePropertyText("SuperbillRememberType", strType, 0, GetCurrentUserName());
	SetRemotePropertyText("SuperbillRememberRes", strRes, 0, GetCurrentUserName());
	

}

void CSuperBillDlg::OnMergePrinter() 
{
	try {
		SetDlgItemText(IDC_PRINT_SUPERBILLS, IsDlgButtonChecked(IDC_MERGE_PRINTER) ? "&Print" : "&Preview");
	} NxCatchAll(__FUNCTION__);
}

void CSuperBillDlg::OnSelChangingLocationFilter(long FAR* nNewSel) 
{
	try {
		//if trying to select nothing, set it back to the first selection
		if(*nNewSel == -1) {
			*nNewSel = 0;
		}
	} NxCatchAll(__FUNCTION__);
}

void CSuperBillDlg::OnCheckLocationFilter() 
{
	try {
		if(IsDlgButtonChecked(IDC_CHECK_LOCATION_FILTER))
			GetDlgItem(IDC_LOCATION_FILTER)->EnableWindow(TRUE);
		else
			GetDlgItem(IDC_LOCATION_FILTER)->EnableWindow(FALSE);
	} NxCatchAll(__FUNCTION__);
}

void CSuperBillDlg::OnUseDateRange() 
{
	try {
		if(IsDlgButtonChecked(IDC_USE_DATE_RANGE)) {
			GetDlgItem(IDC_FROM)->EnableWindow(TRUE);
			GetDlgItem(IDC_TO)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_FROM)->EnableWindow(FALSE);
			GetDlgItem(IDC_TO)->EnableWindow(FALSE);
		}
	} NxCatchAll(__FUNCTION__);
}

//DRT 6/2/2008 - PLID 30230 - Added OnOK handler to keep behavior the same as pre-NxDialog changes
void CSuperBillDlg::OnOK()
{
	//Eat the message
}

// (d.thompson 2009-09-24) - PLID 9975 - Ensure that any controls are enabled/disabled properly
void CSuperBillDlg::EnsureControls()
{
	BOOL bUsingAdvanced = FALSE;
	if(IsDlgButtonChecked(IDC_USE_ADVANCED_CONFIG)) {
		bUsingAdvanced = TRUE;
	}

	GetDlgItem(IDC_EDIT_TEMPLATE_BTN)->EnableWindow(!bUsingAdvanced);
	GetDlgItem(IDC_TEMPLATE)->EnableWindow(!bUsingAdvanced);

	//This button can be disabled if the 'default' is currently selected.
	if(bUsingAdvanced) {
		GetDlgItem(IDC_DEFTEMPLATE_BTN)->EnableWindow(FALSE);
	}
	else {
		//Just call the function to make that determination
		OnSelChosenTemplate(m_template->CurSel);
	}

	//I do intentionally leave the 'Setup' button enabled even if you aren't using advanced.  That config
	//	applies to the right click "Print Superbill" functionality as well for individual appointments.
}

// (d.thompson 2009-09-24) - PLID 9975 - Enable the option to use the advanced templating config instead of merging
//	all the documents to a single template.
void CSuperBillDlg::OnBnClickedUseAdvancedConfig()
{
	try {
		if(IsDlgButtonChecked(IDC_USE_ADVANCED_CONFIG)) {
			DontShowMeAgain(this, "Please note that when using the advanced configuration, the 'Sort By...' options will apply to "
				"each individual template merge, not to the entire set of merged documents.", "SuperbillAdvancedConfig");
		}

		EnsureControls();

		//Save this status for next go-round
		SetRemotePropertyInt("SuperbillAdvancedTemplateConfigInUse", IsDlgButtonChecked(IDC_USE_ADVANCED_CONFIG), 0, GetCurrentUserName());

	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2009-09-24) - PLID 9975 - Let users setup the advanced options
void CSuperBillDlg::OnBnClickedTemplateConfig()
{
	try {
		CConfigureSuperbillTemplatesDlg dlg(this);
		dlg.DoModal();

	} NxCatchAll(__FUNCTION__);
}
