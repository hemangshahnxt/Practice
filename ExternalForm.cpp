// ExternalForm.cpp : implementation file
//

#include "stdafx.h"
#include "ExternalForm.h"
#include "GlobalDataUtils.h"
#include "ReportsRc.h"
#include "ReportInfo.h"
#include "ReportAdo.h"

using namespace ADODB;
using namespace NXDATALISTLib;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExternalForm dialog
//
// For access reports, 

CExternalForm::CExternalForm(CWnd* pParent /*=NULL*/)
	: CNxDialog(CExternalForm::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExternalForm)
	// (a.walling 2010-02-24 10:15) - PLID 37483
	m_bRequireSelection = false;
	m_bDefaultRemember = false;
	//}}AFX_DATA_INIT
}


void CExternalForm::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExternalForm)
	DDX_Control(pDX, IDC_REMONE, m_remOneButton);
	DDX_Control(pDX, IDC_REMALL, m_remAllButton);
	DDX_Control(pDX, IDC_ADDONE, m_addOneButton);
	DDX_Control(pDX, IDC_ADDALL, m_addAllButton);
	DDX_Control(pDX, IDC_EXTERNALFORM_NOTE, m_nxsNote); // (a.walling 2010-02-24 10:14) - PLID 37483
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_REMEMBER_EXTERNAL_LIST, m_checkRememberExternalList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExternalForm, CNxDialog)
	//{{AFX_MSG_MAP(CExternalForm)
	ON_BN_CLICKED(IDC_ADDONE, OnAddOne)
	ON_BN_CLICKED(IDC_REMALL, OnRemoveAll)
	ON_BN_CLICKED(IDC_ADDALL, OnAddAll)
	ON_BN_CLICKED(IDC_REMONE, OnRemoveOne)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_EXF_FILTER, &CExternalForm::OnEnChangeFilter)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CExternalForm, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CExternalForm)
	ON_EVENT(CExternalForm, IDC_EXF_AVAILABLE_LIST, 3, CExternalForm::DblClickCellAvailableList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CExternalForm, IDC_EXF_SELECTED_LIST, 3, CExternalForm::DblClickCellSelectedList, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExternalForm message handlers

BOOL CExternalForm::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_remOneButton.AutoSet(NXB_LEFT);
	m_remAllButton.AutoSet(NXB_LLEFT);
	m_addOneButton.AutoSet(NXB_RIGHT);
	m_addAllButton.AutoSet(NXB_RRIGHT);
	// (z.manning, 04/28/2008) - PLID 29807 - Set button styles for OK & Cancel
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	// (a.walling 2010-02-24 10:17) - PLID 37483
	if (m_bRequireSelection) {
		m_nxsNote.SetWindowText("Please select one or more items by moving them to the right.");
	}

	// Attach to the datalist controls
	try {
		// (j.gruber 2014-03-05 15:59) - PLID 61203 - convert to datalist2
		m_pAvailableList = BindNxDataList2Ctrl(IDC_EXF_AVAILABLE_LIST, false);
		m_pSelectedList = BindNxDataList2Ctrl(IDC_EXF_SELECTED_LIST, false);
		/*m_AvailList = BindNxDataListCtrl(this, IDC_AVAILLIST, GetRemoteData(), FALSE);
		m_SelectList = BindNxDataListCtrl(this, IDC_SELECTLIST, GetRemoteData(), FALSE);*/


		// (d.thompson 2012-06-19) - PLID 50834 - Bulk cache properties
		g_propManager.BulkCache("ExternalForm", propbitNumber | propbitMemo, 
			"(Username = '<None>' OR Username = '%s') AND Name IN ("
			"'RecurseReportCategories' "
			", 'ReportsExternalList' "
		")", _Q(GetCurrentUserName()));

	} NxCatchAll("CExternalForm::OnInitDialog 1");
	
	// Don't know why this is here, but I don't want to change it because I didn't put it here
	GetMainFrame()->SetForegroundWindow();

	// Set the caption label
	SetWindowText(m_Caption);

	// (j.gruber 2014-03-06 12:01) - PLID 61201
	m_bIsFiltered = FALSE;

	// NxDataList
	// TODO: Eventually make the datalist accept a sql statement
	// Right now we just load the datalist manually because we're 
	// sure this sql will return exactly two fields
	try {
		// Empty the lists
		// (j.gruber 2014-03-05 16:00) - PLID 61203 - datalist2
		m_pAvailableList->Clear();
		m_pSelectedList->Clear();		
		
		// Create the given query to find out how many fields there are.
		_RecordsetPtr prs = CreateRecordsetStd(m_SQL, adOpenForwardOnly, adLockReadOnly, adCmdText);
		FieldsPtr flds = prs->Fields;
	
		//Add the columns
		CString strWidths = m_ColWidths;
		CString strNames = m_ColFormat;
		long nWidthPipe, nFormatPipe, nFlags;
		int nPriority = 0;
		for(int i = 0; i < flds->GetCount(); i++){
			
			//Parse the width and format strings
			
			if(strWidths.Find("|") == -1)
				nWidthPipe = strWidths.GetLength();
			else
				nWidthPipe = strWidths.Find("|");
			
			if(strNames.Find("|") == -1)
				nFormatPipe = strNames.GetLength();
			else
				nFormatPipe = strNames.Find("|");
			
			if(atol(strWidths.Left(nWidthPipe)) == -1)
				nFlags = csVisible|csWidthAuto;
			else
				nFlags = csVisible|csWidthPercent;

			//Actually insert the columns
			// (j.gruber 2014-03-05 16:01) - PLID 61203 - DL2
			m_pAvailableList->InsertColumn(i, flds->GetItem((long)i)->GetName(), (LPCTSTR)strNames.Left(nFormatPipe), atol(strWidths.Left(nWidthPipe)), nFlags);
			m_pSelectedList->InsertColumn(i, flds->GetItem((long)i)->GetName(), (LPCTSTR)strNames.Left(nFormatPipe), atol(strWidths.Left(nWidthPipe)), nFlags);			

			//JJ - 1/9/2003 - for switching items between the lists, we achieve much better results if the datalists are sorted by a given column
			//right now, lets choose the first shown column (a nonzero width) and sort on that. -1 is a valid number.
			// (j.gruber 2014-03-05 16:03) - PLID 61203 - DL2
			if(atol(strWidths.Left(nWidthPipe)) != 0) {
				m_pAvailableList->GetColumn(i)->PutSortPriority(nPriority);
				m_pAvailableList->GetColumn(i)->PutSortAscending(TRUE);
				m_pSelectedList->GetColumn(i)->PutSortPriority(nPriority);
				m_pSelectedList->GetColumn(i)->PutSortAscending(TRUE);
				nPriority++;
			}

			//Set the column types
			// (j.gruber 2014-03-05 16:04) - PLID 61203 - DL2
			m_pAvailableList->GetColumn(i)->FieldType = NXDATALIST2Lib::cftTextSingleLine;
			m_pSelectedList->GetColumn(i)->FieldType = NXDATALIST2Lib::cftTextSingleLine;
			//Remove the parts of the strings which we have already parsed
			strWidths = strWidths.Right(strWidths.GetLength() - nWidthPipe - 1);
			strNames = strNames.Right(strNames.GetLength() - nFormatPipe - 1);
		}		

		// Loop through the recordset adding a row for each one
		CStringArray arSavedList;
		GetRemotePropertyCStringArrayMemo("ReportsExternalList", arSavedList, atol(m_RepID), GetCurrentUserName());
		// (j.gruber 2014-03-05 16:05) - PLID 61203 - DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		long nTickStart = GetTickCount();
		while (!prs->eof) {
			// (j.gruber 2014-03-05 16:04) - PLID 61203 - DL2
			pRow = m_pAvailableList->GetNewRow();
			// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - for() loops
			for (int k = 0; k < flds->GetCount(); k++){
				pRow->Value[k] = flds->GetItem((long)k)->Value;
			}
			//Put it in the selected list if appropriate.
			bool bSelected = false;
			for(int j=0; j < arSavedList.GetSize() && !bSelected; j++) {
				if(AsString(pRow->GetValue(0)) == arSavedList.GetAt(j)) {
					// (j.gruber 2014-03-05 16:05) - PLID 61203 - DL2
					m_pSelectedList->AddRowSorted(pRow, NULL);
					bSelected = true;
					//If we're loading a remembered selection, let's reflect that.
					CheckDlgButton(IDC_REMEMBER_EXTERNAL_LIST, BST_CHECKED);
				}
			}
			if(!bSelected) {
				// (j.gruber 2014-03-05 16:06) - PLID 61203 - DL2
				NXDATALIST2Lib::IRowSettingsPtr pRowAdded = m_pAvailableList->AddRowSorted(pRow, NULL);

				// (j.gruber 2014-03-06 09:54) - PLID -- 61201 - load our list			
				m_mapAvailList.insert(std::make_pair(AsString(pRowAdded->GetValue(0)), pRowAdded));
			}			

		//	m_SelectList->AddRow(pRow);
			HR(prs->MoveNext());
		}
		
		long nTickStop = GetTickCount();

		long nTotal = nTickStop - nTickStart;
		TRACE("TIME TAKEN: %li\r\n", nTotal);

		// (a.walling 2010-02-24 10:29) - PLID 37483 - Always check the remember box if we are set to
		if (m_bDefaultRemember) {
			CheckDlgButton(IDC_REMEMBER_EXTERNAL_LIST, BST_CHECKED);
		}
	} NxCatchAll("CExternalForm::OnInitDialog 2");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExternalForm::OnOK() 
{  
	// First return a filter string for Crystal to use
	ASSERT(m_Filter);
	if (m_Filter) {
		try {
			// Just a cleaner reference to the output filter string
			CString &m_strFilter = *m_Filter;

			//TS: First off, let's empty the Filter of anything left over from the last time this report was run.
			m_strFilter.Empty();

			CString strID;
			// Loop through each record in the select list
			// (j.gruber 2014-03-05 16:07) - PLID 61203 - DL2
			long nRowCount = m_pSelectedList->GetRowCount();

			// (a.walling 2010-02-24 10:15) - PLID 37483
			if (m_bRequireSelection && nRowCount <= 0) {
				MessageBox("There are no items selected; you must select at least one item to continue.", NULL, MB_ICONINFORMATION);
				return;
			}


			//DRT 7/31/03 - We no longer have a restriction on items selected.  Previously, we made a filter by looping
			//		through all rows and generating a query like (ID = 1 OR ID = 2 OR ID = 3 OR ID = 4).  Now we are 
			//		generating a temp table (see LetterWriting) that has all of the ID's written to it.  This removes the
			//		arbitrary decision of 500 items allowed to be selected.

			//Determine if this is a 'special case' report or not.  These instances filter on multiple things, so we'll
			//leave them to do their specialness as previously coded.  These will, unfortunately, still be bound to the 
			//arbitrary 500 row limit.

			bool bIsSpecial = false;

			// (a.walling 2010-02-23 17:28) - PLID 37483
			if (atol(m_RepID) <= -100) {
				bIsSpecial = true;
			} else {

				//If you are adding a special case, you MUST put it's ID in this list, as well as handle the case below in the 
				//for loop
				//TES 5/15/2008 - PLID 30014 - Took 341 (Case History) and 387 (Individual Case Histories) out of this, even though
				// they're still handled specially, because they're now in the branch that creates a temp table.
				// (d.thompson 2012-06-19) - PLID 50832 - Removed 125 (Charges by Category)				
				if(/*m_RepID == "341" || m_RepID == "387" || */m_RepID == "295" || m_RepID == "293" || m_RepID == "292" || m_RepID == "283" || m_RepID == "300"
				|| m_RepID == "396" || m_RepID == "141" || m_RepID == "126" || m_RepID == "396" || m_RepID == "202" || m_RepID == "395" 
				|| m_RepID == "87" || m_RepID == "357" || m_RepID == "358"				
				) {
					bIsSpecial = true;
				}
			}


			//Save this list (if they asked us to).
			CStringArray arSelected;
			BOOL bStoreArray = IsDlgButtonChecked(IDC_REMEMBER_EXTERNAL_LIST);
			if(bIsSpecial) {

				// (a.walling 2010-03-02 13:24) - PLID 22432 - No longer necessary due to ConfigRT.MemoParam now NTEXT (PLID 37129)
				// But we'll keep a sane limit so as not to introduce stack overflows or massive processing delays on the server.
				// 5000 is an excessive but acceptable limit, although even SQL 2000 RTM can handle 20000 without choking
				if(nRowCount > 5000){
					AfxMessageBox("No more than 5000 items may be selected in the list. Please select a smaller set of items to filter on, or remove all items from the list to get unfiltered results.");
					return;
				}

				long m_nRepID = atol(m_RepID);
					

				if(nRowCount > 0) m_strFilter = "(";
				// (j.gruber 2014-03-05 16:08) - PLID 61203 - DL2
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelectedList->GetFirstRow();
				int i = 0;
				while (pRow) {
					// Or this with the one before it (if there WAS one before it)
					// (a.walling 2010-02-23 17:37) - PLID 37483
					if (i > 0) m_strFilter += m_nRepID > -100 ? " OR " : ",";

					// Get this item's ID
					// (j.gruber 2014-03-05 16:07) - PLID 61203 - DL2
					if(pRow->GetValue(0).vt == VT_BSTR) strID = "'" + _Q(AsString(pRow->GetValue(0))) + "'";
					else strID = AsString(pRow->GetValue(0));
					
					if(bStoreArray) arSelected.Add(AsString(pRow->GetValue(0)));

					//TS: This was all here because, in the past, the daily batch subreports were filtered
					//by actually opening the queries in code and manipulating their where clauses 
					//directly.  This should no longer be necessary, because selection formulas are now
					//passed to subreports automatically.

					/*// If this is Daily batch we want to also
					if (m_RepID == "187") {
						
						
						ASSERT(FALSE);
						if (i == 0) m_strFilter += " AND ";
						///////////////////////////////////////////////////////////
						m_strFilter += " ((LineItemT_2.InputName) = '";
						m_strFilter += strID;
						m_strFilter += "') ";
					} else {*/
						
					// Simple case, just append the ID value

					//TES 5/8/03 - I agree with Don
					if((m_RepID == "295" || m_RepID == "293")
						// (j.gruber 2014-03-05 16:13) - PLID 61203 - DL2
						&& pRow->GetValue(2).vt == VT_BSTR && VarString(pRow->GetValue(2)) == "Group") { //Short-circuited &&
						m_strFilter += "{" + m_FilterField + "} In (SELECT ID FROM ProcedureT WHERE ProcedureGroupID = " + strID + ")";
					}
					else if(m_RepID == "292" || m_RepID == "283" || m_RepID == "300") {
						m_strFilter += "({" + m_FilterField + "} = " + strID + " OR MasterProcsT.ID = " + strID + ")";
					}
					//DRT 6/5/03 - Wonderful, we have to do this AGAIN!  Really need to fix this to not suck.
					//PIC Report
					else if(m_RepID == "396") {
						m_strFilter += " ProcedureID = ";
						m_strFilter += strID;
					}
					//DRT 8/4/03 - and more!!  Default icd-9's by patient
					//TES 3/26/2014 - PLID 61455 - Check ICD-10 IDs as well
					else if(m_RepID == "202") {
						CString strTemp;
						strTemp.Format(" (PatientsT.DefaultDiagID1 = %s OR PatientsT.DefaultDiagID2 = %s OR PatientsT.DefaultDiagID3 = %s "
							"OR PatientsT.DefaultDiagID4 = %s OR PatientsT.DefaultICD10DiagID1 = %s OR PatientsT.DefaultICD10DiagID2 = %s "
							"OR PatientsT.DefaultICD10DiagID3 = %s OR PatientsT.DefaultICD10DiagID4 = %s) ", strID, strID, strID, strID, strID, strID, strID, strID);
						m_strFilter += strTemp;
					}
					//DRT 1/28/2004 - Unapplied Superbills
					else if(m_RepID == "395") {
						CString strTemp;
						strTemp.Format(" AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID = %s) ", strID);
						m_strFilter += strTemp;
					}
					//DRT 2/2/2004 - PLID 9733 - Provider Multiple Fee Schedules
					else if(m_RepID == "87") {
						CString strTemp;
						strTemp.Format(" MultiFeeGroupsT.ID IN (SELECT FeeGroupID FROM MultiFeeInsuranceT WHERE InsuranceCoID = %s) ", strID);
						m_strFilter += strTemp;
					}
					//TES 3/3/2004 - PLID 11247 - Projected Surgery Income
					//TES 12/22/2004 - PLID 15048 - Actual Surgery Income
					else if(m_RepID == "357" || m_RepID == "358") {
						CString strTemp;
						strTemp.Format(" ProcInfoT.ID IN (SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcInfoDetailsT.ProcedureID = %s) ", strID);
						m_strFilter += strTemp;
					}					
					// (a.walling 2010-02-23 17:30) - PLID 37483
					else if (m_nRepID <= -100) {
						m_strFilter += strID;
					} else {
						m_strFilter += "{" + m_FilterField + "} = " + strID;
					}

					//}

					///////////////////////////////////
					//TS:  If the report is PaymentsByCPTCategory, include the possibility that CatID meets the criteria (this enables subcategory filtering).
					if (m_RepID == "141") {
						m_strFilter += " OR ";
						m_strFilter += "{CategoriesQ.CategoryID} = ";
						m_strFilter += strID;
					}
					if (m_RepID == "126" ) {
						m_strFilter += " OR ";
						m_strFilter += "{CategoriesQ.CategoryID} = ";
						m_strFilter += strID;
					}

					// (j.gruber 2014-03-05 16:15) - PLID 61203 - DL2
					pRow = pRow->GetNextRow();					
					i++;
				}

				//DRT - More special cases!
				if(m_RepID == "396") {
					if(m_strFilter.Right(4) == " OR ") {
						m_strFilter = m_strFilter.Left(m_strFilter.GetLength() - 4);	//remove the last OR
					}
					if(nRowCount > 0) {
						m_strFilter = " ProcInfoT.ID IN (SELECT ProcInfoID FROM ProcInfoDetailsT WHERE " + m_strFilter + ")";
					}
				}			

				
				if(nRowCount > 0) m_strFilter += ")";

			}	//end special case if
			else {
				if(nRowCount > 0) {
					//this is the standard case
					CString strTempTable = CreateTempReportIDTable(m_pSelectedList, 0, NULL, arSelected);

					// (d.thompson 2012-06-19) - PLID 50834
					HandleSpecialChargeCategoryRecursion(strTempTable);
					
					//TES 5/15/2008 - PLID 30014 - These two are special cases, but they can still benefit from using a temp
					// table.  Otherwise, it adds a new subquery for every procedure you filter on, which is inefficient and
					// will even throw an exception if it gets above 255.
					//DRT 3/17/03 - I hate special cases (especially ones hidden in files noone will ever find!), but no other way around this that I can see
					if(m_RepID == "341" || m_RepID == "387") {
						CString str;
						str.Format("(CaseHistoryQ.ID IN (SELECT CaseHistoryID FROM CaseHistoryProceduresT WHERE ProcedureID IN (SELECT ID FROM [%s])))", strTempTable);
						m_strFilter += str;
					}
					else {						
						m_strFilter.Format(" ({%s} IN (SELECT ID FROM [%s])) ", m_FilterField, strTempTable);
					}

					// (j.gruber 2014-03-25 15:43) - PLID 61345 - Updating Charges by DiagCode by Service Code and Charge Totals by Diag code for ICD-10
					if (m_RepID == "276" || m_RepID == "389")
					{						
						m_strFilter.Format(" _DiagCodesByProvQ.ICD9Code IN (SELECT ID FROM [%s]) OR _DiagCodesByProvQ.ICD10Code IN (SELECT ID FROM [%s])", strTempTable, strTempTable);
					}
				}
			}
			if(bStoreArray) {
				//ACW 6/8/04 - If there are too many codes selected the string which is stored in ConfigRT is
				//too large and causes an error.  So, the number of codes chosen had to be restricted.
				// (a.walling 2010-03-02 13:24) - PLID 22432 - No longer necessary due to ConfigRT.MemoParam now NTEXT (PLID 37129)
				// But we'll keep a sane limit so as not to introduce stack overflows or massive processing delays on the server.
				// 5000 is an excessive but acceptable limit, although even SQL 2000 RTM can handle 20000 without choking
				if(nRowCount > 5000){
					AfxMessageBox("No more than 5000 items may be selected in the list. Please select a smaller set of items to filter on, or remove all items from the list to get unfiltered results.");
					return;
				}
				SetRemotePropertyCStringArrayMemo("ReportsExternalList", arSelected, atol(m_RepID), GetCurrentUserName());
			}
			//TES 1/30/2004: If they don't want to remember, clear out whatever we were remembering.
			else {
				SetRemotePropertyCStringArrayMemo("ReportsExternalList", CStringArray(), atol(m_RepID), GetCurrentUserName());
			}

		} NxCatchAll("CExternalForm::OnOK");
	}

	CNxDialog::OnOK();
}

// (j.gruber 2014-03-05 16:25) - PLID 61203 - removed

void CExternalForm::OnAddOne() 
{ 
	try {
		BeginWaitCursor();
		
		// (j.gruber 2014-03-06 11:22) - PLID 61201 - remove this row from our filter list
		//this function name is misleading, it can be more than one row
		NXDATALIST2Lib::IRowSettingsPtr pRowCurrent = m_pAvailableList->GetFirstSelRow();
		while (pRowCurrent) {
			CString strID = AsString(pRowCurrent->GetValue(0));
			std::map<CString, NXDATALIST2Lib::IRowSettingsPtr>::iterator itFind = m_mapAvailList.find(strID);
			if (itFind != m_mapAvailList.end())
			{
				//we found it, now remove it
				m_mapAvailList.erase(itFind);
			}
			pRowCurrent = pRowCurrent->GetNextSelRow();
		}

		// (j.gruber 2014-03-05 16:15) - PLID 61203 - DL2
		m_pSelectedList->TakeCurrentRowAddSorted(m_pAvailableList, NULL);
		
		EndWaitCursor(); 
	} NxCatchAll("CExternalForm::OnAddOne");
}

void CExternalForm::OnRemoveOne() 
{
	try {

		BeginWaitCursor();
		

		// (j.gruber 2014-03-06 11:25) - PLID 61201 - add this row back to our filterable list
		// this function name is misleading, it can actually be multiple rows
		NXDATALIST2Lib::IRowSettingsPtr pRowCurrent = m_pSelectedList->GetFirstSelRow();
		while (pRowCurrent)
		{
			m_mapAvailList.insert(std::make_pair(AsString(pRowCurrent->GetValue(0)), pRowCurrent));
			pRowCurrent = pRowCurrent->GetNextSelRow();
		}

		// (j.gruber 2014-03-05 16:16) - PLID 61203 - DL2
		m_pAvailableList->TakeCurrentRowAddSorted(m_pSelectedList, NULL);

		// (j.gruber 2014-03-06 13:15) - PLID 61201 - refilter
		if (m_bIsFiltered)
		{
			CString strFilter;
			GetDlgItemText(IDC_EXF_FILTER, strFilter);
			FilterAvailableList(strFilter);
		}
		
		EndWaitCursor(); 

	} NxCatchAll("CExternalForm::OnRemoveOne");
}
void CExternalForm::OnAddAll() 
{
	try {
		BeginWaitCursor();	

		// (j.gruber 2014-03-06 11:27) - PLID 61201 - we can't clear everything if we are filtered
		if (m_bIsFiltered)
		{
			NXDATALIST2Lib::IRowSettingsPtr pRowCurrent = m_pAvailableList->GetFirstRow();
			while (pRowCurrent) {
				CString strID = AsString(pRowCurrent->GetValue(0));
				std::map<CString, NXDATALIST2Lib::IRowSettingsPtr>::iterator itFind = m_mapAvailList.find(strID);
				if (itFind != m_mapAvailList.end())
				{
					//we found it, now remove it
					m_mapAvailList.erase(itFind);
				}
				pRowCurrent = pRowCurrent->GetNextRow();
			}
		}
		else {
			//we can clear everything
			m_mapAvailList.clear();
		}

		// (j.gruber 2014-03-05 16:16) - PLID 61203
		m_pSelectedList->TakeAllRows(m_pAvailableList);

		EndWaitCursor();
	} NxCatchAll("CExternalForm::OnAddAll");
}

void CExternalForm::OnRemoveAll() 
{
	try {
		BeginWaitCursor();

		// (j.gruber 2014-03-06 11:41) - PLID 61201 - add everything from selected into our available map
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelectedList->GetFirstRow();
		while (pRow)
		{
			m_mapAvailList.insert(std::make_pair(AsString(pRow->GetValue(0)), pRow));
			pRow = pRow->GetNextRow();
		}

		// (j.gruber 2014-03-05 16:16) - PLID 61203 - DL2
		m_pAvailableList->TakeAllRows(m_pSelectedList);

		// (j.gruber 2014-03-06 13:15) - PLID 61201 - refiltered
		if (m_bIsFiltered)
		{
			CString strFilter;
			GetDlgItemText(IDC_EXF_FILTER, strFilter);
			FilterAvailableList(strFilter);
		}

		EndWaitCursor();
	} NxCatchAll("CExternalForm::OnRemoveAll");
}

// (c.haag 2015-01-22) - PLID 64646 - Gets the connection to use for the creation and insertion of temp table records.
_ConnectionPtr CExternalForm::GetTempTableConnection()
{
	// We need to see if server side cursors are involved; and if so, use the MARS snapshot.
	return (m_RepID.GetLength() > 0 && atol(m_RepID) > 0 && CReportInfo::IsServerSide(atol(m_RepID))) ? GetRemoteDataReportSnapshot() : GetRemoteDataSnapshot();
}

//returns the name of the temp table created
//Copied and modified from LetterWriting.cpp, CreateTempIDTable_70
//I went with the 7.0 version, because it should work in all cases, where the 
//8.0 version will not work with SQL 7
// (j.gruber 2014-03-05 16:17) - PLID 61203 - DL2
CString CExternalForm::CreateTempReportIDTable(NXDATALIST2Lib::_DNxDataListPtr pDataList, short nIDColIndex, OUT long *pnRecordCount, OUT CStringArray &arSelected)
{
	// Create a local temporary table (local means it only exists 
	// in the context of this connection, and temporary means it 
	// will be deleted as soon as this connection closes) and fill it

	// Unique temp table name within this connection (other connections have their own names)
	CString strTempT;
	strTempT.Format("#TempMerge%lu", GetTickCount());

	//The major reason this is not using a LW version, is that some reports filter on an ID, and some filter on a string!
	//So we need to create the id field appropriately.  NOTE:  This is all setup in the OnInitDialog, so we have no idea
	//what type the field is.  So get the value of the first one out, and try our best to guess from there.
	CString strType;
	// (j.gruber 2014-03-05 16:17) - PLID 61203 - DL2
	NXDATALIST2Lib::IRowSettingsPtr pRow = pDataList->GetFirstRow();
	if (pRow) {
		_variant_t var = pRow->GetValue(nIDColIndex);
		VARTYPE vt = var.vt;

		if(vt == VT_BSTR) {
			//we are working with a number
			strType.Format("nvarchar(500)");	//should be ample size
		}
		else if(vt == VT_I4) {
			strType.Format("int");
		}
		else {
			//TODO:  if you get this assertion, your type is not setup.  define it
			//		in a style similar to the above.  You also need to set it up
			//		in the while loop below.
			ASSERT(FALSE);
		}

		// (a.walling 2009-08-26 12:08) - PLID 35178 - Use the snapshot connection if available		
		// (c.haag 2015-01-22) - PLID 64646 - Use GetTempTableConnection
		_ConnectionPtr pCon = GetTempTableConnection();
		ExecuteSql(pCon, "CREATE TABLE %s (ID %s)", strTempT, strType);

		// Loop through the datalist
		// (j.gruber 2014-03-05 16:20) - PLID 61203 - DL2
		int i = 0;
		NXDATALIST2Lib::IRowSettingsPtr pLoopRow = pDataList->GetFirstRow();
		while (pLoopRow) {						
			_variant_t var = pLoopRow->GetValue(nIDColIndex);

			if(var.vt == VT_BSTR) {
				// Add the ID
				// (a.walling 2009-08-26 12:08) - PLID 35178 - Parameterized, also use the snapshot connection if available
				// (c.haag 2015-01-22) - PLID 64646 - Use pCon instead of GetRemoteDataSnapshot
				ExecuteParamSql(pCon, FormatString("INSERT INTO %s (ID) VALUES ({STRING})", strTempT), VarString(var, ""));
			}
			else if(var.vt == VT_I4) {
				// (a.walling 2009-08-26 12:08) - PLID 35178 - Parameterized, also use the snapshot connection if available
				// (c.haag 2015-01-22) - PLID 64646 - Use pCon instead of GetRemoteDataSnapshot
				ExecuteParamSql(pCon, FormatString("INSERT INTO %s (ID) VALUES ({INT})", strTempT), VarLong(var));
			}
			else {
				//TODO:  If you get this assertion, you probably got the one above.  You need to 
				//setup the insert for the type
				ASSERT(FALSE);
			}

			//Save this id in our list.
			arSelected.Add(AsString(var));

			// (j.gruber 2014-03-05 16:22) - PLID 61203
			pLoopRow = pLoopRow->GetNextRow();
		}

		// If the caller wants it, give the record count
		if (pnRecordCount) {
			*pnRecordCount = i;
		}

		// Return the name of the temp table
		return strTempT;

	}
	return "";
}

// (d.thompson 2012-06-19) - PLID 50834 - Special case for certain reports using charge category.  We
//	are actually going to add to the temp table data.  These reports specially use the charge
//	categories, and we are going to support (if preference is on) recursively choosing all
//	subcategories.  We'll do this by modifying the temp table to include all children.
void CExternalForm::HandleSpecialChargeCategoryRecursion(CString strTempTable)
{
	// (d.thompson 2012-06-19) - PLID 50834 - Option to disable this behavior
	if(GetRemotePropertyInt("RecurseReportCategories", 1, 0, "<None>", true) == 0)
	{
		//option is off, ignore this possibility
		return;
	}

	if(
		m_RepID == "580"		// (d.thompson 2012-06-19) - PLID 50834 - Apply this to Daily Sales (by Charge Category)
		|| m_RepID == "325"		// (d.thompson 2012-06-19) - PLID 50833 - Financial Activity Daily (by Charge Category)
		|| m_RepID == "125"		// (d.thompson 2012-06-19) - PLID 50832 - Charges by Category
		|| m_RepID == "421"		// (d.thompson 2012-06-19) - PLID 50832 - Charges by Category by Pt Coord
		|| m_RepID == "275"		// (d.thompson 2012-06-19) - PLID 50832 - Charges by Category by Provider
		|| m_RepID == "542"		// (d.thompson 2012-06-19) - PLID 50832 - Charges by Category Split by ...
	  )
	{
		CSqlFragment sqlAddSubs(
			FormatString("INSERT INTO %s "
				"SELECT ID FROM CategoriesT "
				"WHERE Parent IN (SELECT ID FROM %s) "
				"AND ID NOT IN (SELECT ID FROM %s);\r\n",
			strTempTable, strTempTable, strTempTable));

		//Because we have a hard cap of 4 levels deep on the category tree, we just need to run
		//	the above query 3 times (we already have the highest levels) to ensure we nabbed
		//	all children.
		//Future TODO:  Technically this is CategorySelectDlg.cpp's MAX_LEVELS - 1, but that is protected
		//	and I'm unable to presently revise that work to make it exposed, so we'll have to hardcode.
		CSqlFragment sqlToRun;
		for(int nSub = 0; nSub < 4 - 1; nSub++) {
			sqlToRun += sqlAddSubs;
		}

		// (c.haag 2015-01-22) - PLID 64646 - Use GetTempTableConnection
		ExecuteParamSql(GetTempTableConnection(), sqlToRun);
	}
	else {
		//This report does not support special category handling, so we shall do nothing.
	}
}

// (j.gruber 2014-03-05 15:33) - PLID 61201
void CExternalForm::OnEnChangeFilter()
{
	try{

		CString strEntry;
		GetDlgItemText(IDC_EXF_FILTER, strEntry);

		if (strEntry.GetLength() >= 3)
		{
			FilterAvailableList(strEntry);
			m_bIsFiltered = TRUE;
		}
		else {
			if (m_bIsFiltered) {
				ResetAvailableList();
				m_bIsFiltered = FALSE;
			}
		}
		
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2014-03-06 11:16) - PLID 61201
void CExternalForm::ResetAvailableList()
{
	BeginWaitCursor();

	m_pAvailableList->Clear();

	for each(std::map<CString, NXDATALIST2Lib::IRowSettingsPtr>::value_type pItem in m_mapAvailList)
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow = pItem.second;
		if (pRow) 
		{
			AddRowFromMap(pRow);
		}
	}

	EndWaitCursor();

}

// (j.gruber 2014-03-06 12:45) - PLID 61201 - we need this function because of an apparently datalist bug where it doesn't like the row from the map
void CExternalForm::AddRowFromMap(NXDATALIST2Lib::IRowSettingsPtr pRowFromMap)
{
	NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pAvailableList->GetNewRow();
	for(int i = 0; i < m_pAvailableList->GetColumnCount(); i++) 
	{
		pNewRow->PutValue(i, pRowFromMap->GetValue(i));
	}
	m_pAvailableList->AddRowSorted(pNewRow, NULL);
}


// (j.gruber 2014-03-06 11:16) - PLID 61201
void CExternalForm::FilterAvailableList(CiString strFilter)
{
	BeginWaitCursor();

	// (j.gruber 2014-03-10 14:41) - PLID 61201 - take out leading spaces
	strFilter = strFilter.TrimLeft();

	//first clear the list
	m_pAvailableList->Clear();

	//loop through our array of rows to filter for our string
	for each(std::map<CString, NXDATALIST2Lib::IRowSettingsPtr>::value_type pItem in m_mapAvailList)
	{	
		NXDATALIST2Lib::IRowSettingsPtr pRow = pItem.second;

		if (pRow) 
		{
			//loop through each column in the list
			BOOL bInclude = FALSE;
			for(int i = 0; i < m_pAvailableList->GetColumnCount() && !bInclude; i++) 
			{
				_variant_t varValue = pRow->GetValue(i);
				//we are only looking at string values
				if (varValue.vt == VT_BSTR)
				{
					CiString cisValue = VarString(varValue);
					if (cisValue.Find(strFilter) != -1)
					{
						bInclude = TRUE;
					}
				}
			}

			if (bInclude)
			{				
				//making a new row here because of an apparently datalist bug
				AddRowFromMap(pRow);
				
			}
		}
	}

	EndWaitCursor();

}

void CExternalForm::DblClickCellAvailableList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		OnAddOne();
	}NxCatchAll(__FUNCTION__);
}

void CExternalForm::DblClickCellSelectedList(LPDISPATCH lpRow, short nColIndex)
{
	try
	{
		OnRemoveOne();

	}NxCatchAll(__FUNCTION__);
}
