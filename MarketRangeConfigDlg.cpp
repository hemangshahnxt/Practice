// MarketRangeConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "marketingRc.h"
#include "MarketRangeConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CMarketRangeConfigDlg dialog


CMarketRangeConfigDlg::CMarketRangeConfigDlg(CWnd* pParent)
	: CNxDialog(CMarketRangeConfigDlg::IDD, pParent)
{
	m_aryAddList.RemoveAll();
	m_aryRemoveList.RemoveAll();
	m_aryChangeList.RemoveAll();
	m_bNeedToSave = FALSE;
	//{{AFX_DATA_INIT(CMarketRangeConfigDlg)
	//}}AFX_DATA_INIT
}


void CMarketRangeConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketRangeConfigDlg)
	DDX_Control(pDX, IDC_MOVE_UP, m_MoveUp);
	DDX_Control(pDX, IDC_MOVE_DOWN, m_MoveDown);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_RESTORE_DEFAULTS, m_btnRestoreDefaults);
	DDX_Control(pDX, IDC_ADD_RANGE, m_btnAddRange);
	DDX_Control(pDX, IDC_REMOVE_RANGE, m_btnRemoveRange);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CMarketRangeConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMarketRangeConfigDlg)
	ON_BN_CLICKED(IDC_ADD_RANGE, OnAddRange)
	ON_BN_CLICKED(IDC_MOVE_DOWN, OnMoveDown)
	ON_BN_CLICKED(IDC_MOVE_UP, OnMoveUp)
	ON_BN_CLICKED(IDC_REMOVE_RANGE, OnRemoveRange)
	ON_BN_CLICKED(IDC_RESTORE_DEFAULTS, OnRestoreDefaults)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketRangeConfigDlg message handlers

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CMarketRangeConfigDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMarketRangeConfigDlg)
	ON_EVENT(CMarketRangeConfigDlg, IDC_RANGE_LIST_EDIT, 9 /* EditingFinishing */, OnEditingFinishingRangeListEdit, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CMarketRangeConfigDlg, IDC_RANGE_LIST_EDIT, 10 /* EditingFinished */, OnEditingFinishedRangeListEdit, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

long CMarketRangeConfigDlg::GetNewIDFromList(long nCol) {

	//loop through the list and return 1 + the highest ID found
	long p = m_pRangeList->GetFirstRowEnum();
	LPDISPATCH lpDisp = NULL;
	long nHighID = 0;
	while (p) {
		m_pRangeList->GetNextRowEnum(&p, &lpDisp);
		IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
		long nIDVal = VarLong(pRow->GetValue((short)nCol));
		if (nIDVal > nHighID) {
			nHighID = nIDVal;
		}
		
	}

	return nHighID + 1;
}

void CMarketRangeConfigDlg::OnAddRange() 
{
	IRowSettingsPtr pRow = m_pRangeList->GetRow(-1);

	long nNewID = GetNewIDFromList(0);

	//figure out what the new order ID will be
	//it will just be the number of rows in the list
	long nOrderID = GetNewIDFromList(1);

	pRow->PutValue(0, (long)nNewID);
	pRow->PutValue(1, (long)nOrderID);
	pRow->PutValue(2, _variant_t("[Enter Range Description]"));
	pRow->PutValue(3, (long)0);
	pRow->PutValue(4, (long)0);
	pRow->PutValue(5, (long)(0));
	
	//add the row
	m_pRangeList->AddRow(pRow);

	//add it to our array list
	m_aryAddList.Add(nNewID);

	//remember we need to save now
	m_bNeedToSave = TRUE;

	
	
	
}

BOOL CMarketRangeConfigDlg::IDInList(CDWordArray *pAry, long nIDToLookFor) {

	//check to see that it isn't in our added list first
	BOOL  bFound = FALSE;
	for (int i = 0; i < pAry->GetSize(); i++) {
		if (pAry->GetAt(i) == (DWORD)nIDToLookFor) {
			bFound = TRUE;
		}
	}

	return bFound;

}

void CMarketRangeConfigDlg::OnMoveDown() 
{
	//change the order
	long nCurSel = m_pRangeList->GetCurSel();
	if (nCurSel == -1) {
		return;
	}


	if (nCurSel != m_pRangeList->GetRowCount() - 1) {
		long nID = VarLong(m_pRangeList->GetValue(nCurSel, 0));
		long nOtherID = VarLong(m_pRangeList->GetValue(nCurSel + 1, 0));
		if (nCurSel != -1)  {
			long nOrder = VarLong(m_pRangeList->GetValue(nCurSel, 1));

			m_pRangeList->PutValue(nCurSel, 1, (long)(nOrder + 1));
			m_pRangeList->PutValue(nCurSel + 1, 1, nOrder);

			m_pRangeList->Sort();
		}

				
		if ((!IDInList(&m_aryAddList, nID)) && (! IDInList(&m_aryChangeList, nID))) {
			//add it to the change list
			m_aryChangeList.Add(nID);
		}

		if ((!IDInList(&m_aryAddList, nOtherID)) && (! IDInList(&m_aryChangeList, nOtherID))) {
			//add it to the change list
			m_aryChangeList.Add(nOtherID);
		}

		m_bNeedToSave = TRUE;
	}

	Resort();
	
}

void CMarketRangeConfigDlg::Resort() {

	long p = m_pRangeList->GetFirstRowEnum();
	LPDISPATCH lpDisp = NULL;
	long nOrderID = 1;
	while (p) {
		m_pRangeList->GetNextRowEnum(&p, &lpDisp);
		IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
		long nIDVal = VarLong(pRow->GetValue(1));
		long nID = VarLong(pRow->GetValue(0));
		if (nIDVal != nOrderID) {
			
			//put the correct order on it
			pRow->PutValue(1, nOrderID);

			//add it to the list of changes
			if ((!IDInList(&m_aryAddList, nID)) && (! IDInList(&m_aryChangeList, nID))) {
				m_aryChangeList.Add(nID);
			}

		}
		nOrderID++;
		
	}

}

void CMarketRangeConfigDlg::OnMoveUp() 
{
	//change the order
	long nCurSel = m_pRangeList->GetCurSel();
	if (nCurSel > 0) {
		long nID = VarLong(m_pRangeList->GetValue(nCurSel, 0));
		long nOtherID = VarLong(m_pRangeList->GetValue(nCurSel - 1, 0));
		if (nCurSel != -1) {
			long nOrder = VarLong(m_pRangeList->GetValue(nCurSel, 1));

			m_pRangeList->PutValue(nCurSel, 1, (long)(nOrder - 1));
			m_pRangeList->PutValue(nCurSel - 1, 1, nOrder);

			m_pRangeList->Sort();
		}

		if ((!IDInList(&m_aryAddList, nID)) && (! IDInList(&m_aryChangeList, nID))) {
			//add it to the change list
			m_aryChangeList.Add(nID);
		}

		if ((!IDInList(&m_aryAddList, nOtherID)) && (! IDInList(&m_aryChangeList, nOtherID))) {
			//add it to the change list
			m_aryChangeList.Add(nOtherID);
		}
		m_bNeedToSave = TRUE;

		//Set the SortOrders starting at 1
		Resort();
	}
	
}

void CMarketRangeConfigDlg::OnRemoveRange() 
{
	long nCurSel = m_pRangeList->CurSel;
	
	if (nCurSel != -1) {
		long nID = m_pRangeList->GetValue(nCurSel, 0);
		long nOrder = m_pRangeList->GetValue(nCurSel, 1);
		m_pRangeList->RemoveRow(nCurSel);


		//add it to our remove list
		m_aryRemoveList.Add(nID);
		m_bNeedToSave = TRUE;

		//check to see if the ID is in our other lists and if so, remove it
		for (int i = 0; i < m_aryAddList.GetSize(); i++) {
			if (m_aryAddList.GetAt(i) == (DWORD)nID) {
				m_aryAddList.RemoveAt(i);
			}
		}

		for (int j = 0; j < m_aryChangeList.GetSize(); j++) {
			if (m_aryChangeList.GetAt(j) == (DWORD)nID) {
				m_aryChangeList.RemoveAt(j);
			}
		}
	}

	Resort();


	
	
}

void CMarketRangeConfigDlg::OnEditingFinishingRangeListEdit(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{

	//we are just checking here for correct values
	switch (nCol) {

		case 2:
			if (pvarNewValue->vt != VT_BSTR) {
				MessageBox("Please Enter Valid Data");
				pbCommit = FALSE;
				pbContinue = FALSE;
			}
		break;
		case 3:
		case 4:
		case 5:
			if (pvarNewValue->vt != VT_I4) {
				MessageBox("Please Enter Valid Data");
				pbCommit = FALSE;
				pbContinue = FALSE;
			}
		break;
	}
		

		
}

void CMarketRangeConfigDlg::OnEditingFinishedRangeListEdit(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{

	if (bCommit) {
		//add the ID of the column to our changed list so we know what to update
		long nID = VarLong(m_pRangeList->GetValue(nRow, 0));

	if ((!IDInList(&m_aryAddList, nID)) && (! IDInList(&m_aryChangeList, nID))) {
		//add it to the change list
		m_aryChangeList.Add(nID);
	}
		m_bNeedToSave = TRUE;

	}
	
	
}

BOOL CMarketRangeConfigDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog(); // (a.walling 2011-01-14 16:44) - no PLID - Fix bad base class OnInitDialog call
		
		m_pRangeList = BindNxDataListCtrl(IDC_RANGE_LIST_EDIT, true);

		m_MoveUp.AutoSet(NXB_UP);
		m_MoveDown.AutoSet(NXB_DOWN);
		// (c.haag 2008-04-29 12:55) - PLID 29824 - NxIconify more buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnRestoreDefaults.AutoSet(NXB_MODIFY);
		m_btnAddRange.AutoSet(NXB_NEW);
		m_btnRemoveRange.AutoSet(NXB_DELETE);
	}
	NxCatchAll("Error in CMarketRangeConfigDlg::OnInitDialog");
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CMarketRangeConfigDlg::IsValidData() {

	/*make sure none of the rows have invalid data which is
	1. No blank descriptions
	2. No descriptions called [Enter Range Description]
	3. No starts > ends
	4. no unit selected*/

	long p = m_pRangeList->GetFirstRowEnum();
	LPDISPATCH lpDisp = NULL;
	CString strTemp, strMessage = "";
	BOOL bIsValid = TRUE;
	CString strDescription;
	long nStart, nEnd, nUnit;
	long nCount  = 1;
	while (p) {
		m_pRangeList->GetNextRowEnum(&p, &lpDisp);
		IRowSettingsPtr pRow(lpDisp); lpDisp->Release();

		strDescription = VarString(pRow->GetValue(2));
		strDescription.TrimRight();
		strDescription.TrimLeft();
		//Bad descriptions
		if (strDescription.IsEmpty()) {
			strTemp.Format("%sRow %li: Please enter a description. \n", strMessage, nCount);
			strMessage = strTemp;
			bIsValid = FALSE;
		}
		if (strDescription == "[Enter Range Description]") {
			strTemp.Format("%sRow %li: Please enter a description other than the default. \n", strMessage, nCount);
			strMessage = strTemp;
			bIsValid = FALSE;
		}

		//start before ends
		nStart = VarLong(pRow->GetValue(3));
		nEnd = VarLong(pRow->GetValue(4));
		if (nStart >= nEnd) {
			strTemp.Format("%sRow %li: Please enter a start value that is less than the end value. \n", strMessage, nCount);
			strMessage = strTemp;
			bIsValid = FALSE;
		}

		//no unit selected
		nUnit = VarLong(pRow->GetValue(5));
		if (nUnit != 1 && nUnit != 2) {
			strTemp.Format("%sRow %li: Please select with Month(s) or Year(s). \n", strMessage, nCount);
			strMessage = strTemp;
			bIsValid = FALSE;
		}

		nCount++;		
	}

	if (! bIsValid) {
		MsgBox(strMessage);
	}

	return bIsValid;
}

void CMarketRangeConfigDlg::OnOK() 
{

	if (m_bNeedToSave) {
		//save everything
		if (IsValidData()) {
			if (Save()) {
				EndDialog(1);
			}
		}
	}
	else {
		EndDialog(0);
	}
		
}

void CMarketRangeConfigDlg::OnCancel() 
{
	
	
	EndDialog(0);
}


BOOL CMarketRangeConfigDlg::Save() {


	try {
		// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
		CSqlTransaction trans("RetentionRangeConfigT");
		trans.Begin();
		long nRemoveCount = m_aryRemoveList.GetSize();
		if (nRemoveCount > 0)  {
			CString strRemoveIDs = "";
			//lets remove everything first 
			for (int i = 0; i < nRemoveCount; i++) {
				strRemoveIDs += AsString((long)m_aryRemoveList.GetAt(i)) + ", ";
			}

			//remove the last comma
			strRemoveIDs = strRemoveIDs.Left(strRemoveIDs.GetLength() - 2);

			//now generate our SQL statement
			ExecuteSql("DELETE FROM RetentionRangeConfigT WHERE ID IN (%s) ", strRemoveIDs);
		}


		long nAddCount = m_aryAddList.GetSize(); 
		if (nAddCount > 0) {
			//now do our additions
			CString strAddIDs;
			CString strDescription;
			long nStart, nEnd, nOrder, nUnit;
			IRowSettingsPtr pRow;

			for (int j = 0; j < nAddCount; j++) {

				long nRow = m_pRangeList->FindByColumn(0, (long)m_aryAddList.GetAt(j), 0, FALSE);

				if (nRow != -1) {
					pRow = m_pRangeList->GetRow(nRow);

					nOrder = VarLong(pRow->GetValue(1));
					strDescription = VarString(pRow->GetValue(2));
					nStart = VarLong(pRow->GetValue(3));
					nEnd = VarLong(pRow->GetValue(4));
					nUnit = VarLong(pRow->GetValue(5));

					//now insert it
					if (nUnit == 2) {
						//years
						nStart = nStart * 12;
						nEnd = nEnd * 12;
					}

					//just in case someone else has entered things into the table since we have
					long nNewID = NewNumber("RetentionRangeConfigT", "ID");
					ExecuteSql("INSERT INTO RetentionRangeConfigT (ID, Description, RangeStart, RangeEnd, SortOrder) "
						" VALUES (%li, '%s', %li, %li, %li) ", nNewID, _Q(strDescription), nStart, nEnd, nOrder);
					
				}
				else {
					ASSERT(FALSE);
				}
			}

		}


		//and finally our changes
		long nChangeCount = m_aryChangeList.GetSize(); 
		if (nChangeCount > 0) {

			//now do our additions
			CString strAddIDs;
			CString strDescription;
			long nStart, nEnd, nOrder, nUnit;
			IRowSettingsPtr pRow;


			for (int k = 0; k < nChangeCount; k++) {

				long nIDToFind = m_aryChangeList.GetAt(k);

				long nRow = m_pRangeList->FindByColumn(0, nIDToFind, 0, FALSE);

				if (nRow != -1) {
					pRow = m_pRangeList->GetRow(nRow);

					nOrder = VarLong(pRow->GetValue(1));
					strDescription = VarString(pRow->GetValue(2));
					nStart = VarLong(pRow->GetValue(3));
					nEnd = VarLong(pRow->GetValue(4));
					nUnit = VarLong(pRow->GetValue(5));

					//now insert it
					if (nUnit == 2) {
						//years
						nStart = nStart * 12;
						nEnd = nEnd * 12;
					}

					ExecuteSql("UPDATE RetentionRangeConfigT SET SortOrder = %li, Description = '%s', "
						" RangeStart = %li, RangeEnd = %li WHERE ID = %li", nOrder, _Q(strDescription),
						nStart, nEnd, m_aryChangeList.GetAt(k));			
				}
				else {
					ASSERT(FALSE);
				}
			}
		}

		trans.Commit();

		//remove everything from our lists
		m_aryAddList.RemoveAll();
		m_aryChangeList.RemoveAll();
		m_aryRemoveList.RemoveAll();

		return TRUE;
	}NxCatchAll("Error Saving Retention Ranges");

	return FALSE;

		
}

void CMarketRangeConfigDlg::OnRestoreDefaults() 
{
	if (IDYES == MsgBox(MB_YESNO, "This will restore the default ranges. All changes that have ever been made will be overwritten.  Are you sure you wish to continue?")) {

		try {

			//need need to loop through tht list taking everything out and then add everything to the list.
			//we have to do it this way so that they have the ability to cancel
			long p = m_pRangeList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while (p) {
				m_pRangeList->GetNextRowEnum(&p, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
				m_aryRemoveList.Add((DWORD)VarLong(pRow->GetValue(0)));
			}
			//now clear the list
			m_pRangeList->Clear();

			//TES 4/12/2012 - PLID 49645 - We need to clear out our lists of added and changed rows, they no longer apply
			m_aryAddList.RemoveAll();
			m_aryChangeList.RemoveAll();


			//add the defaults back in
			IRowSettingsPtr pRow;
			const _variant_t varFalse(VARIANT_FALSE, VT_BOOL);
			const _variant_t varTrue(VARIANT_TRUE, VT_BOOL);

			//set up the ranges
			pRow = m_pRangeList->GetRow(-1);
			pRow->PutValue(0, (long) 1);
			pRow->PutValue(1, (long) 1);
			pRow->PutValue(2, _variant_t("< 3 months"));
			pRow->PutValue(3, (long) 0);
			pRow->PutValue(4, (long) 3);
			pRow->PutValue(5, (long) 1);
			m_pRangeList->InsertRow(pRow, 0);
			m_aryAddList.Add(1);

			pRow = m_pRangeList->GetRow(-1);
			pRow->PutValue(0, (long) 2);
			pRow->PutValue(1, (long)2);
			pRow->PutValue(2, _variant_t("3 to 6 months"));
			pRow->PutValue(3, (long) 3);
			pRow->PutValue(4, (long) 6);
			pRow->PutValue(5, (long) 1);
			m_pRangeList->InsertRow(pRow, 1);
			m_aryAddList.Add(2);

			pRow = m_pRangeList->GetRow(-1);
			pRow->PutValue(0, (long) 3);
			pRow->PutValue(1, (long) 3);
			pRow->PutValue(2, _variant_t("6 to 9 months"));
			pRow->PutValue(3, (long) 6);
			pRow->PutValue(4, (long) 9);
			pRow->PutValue(5, (long) 1);
			m_pRangeList->InsertRow(pRow, 2);
			m_aryAddList.Add(3);

			pRow = m_pRangeList->GetRow(-1);
			pRow->PutValue(0, (long) 4);
			pRow->PutValue(1, (long) 4);
			pRow->PutValue(2, _variant_t("9 to 12 months"));
			pRow->PutValue(3, (long) 9);
			pRow->PutValue(4, (long) 12);
			pRow->PutValue(5, (long) 1);
			m_pRangeList->InsertRow(pRow, 3);
			m_aryAddList.Add(4);

			pRow = m_pRangeList->GetRow(-1);
			pRow->PutValue(0, (long) 5);
			pRow->PutValue(1, (long) 5);
			pRow->PutValue(2, _variant_t("12 to 15 months"));
			pRow->PutValue(3, (long) 12);
			pRow->PutValue(4, (long) 15);
			pRow->PutValue(5, (long) 1);
			m_pRangeList->InsertRow(pRow, 4);
			m_aryAddList.Add(5);

			pRow = m_pRangeList->GetRow(-1);
			pRow->PutValue(0, (long) 6);
			pRow->PutValue(1, (long) 6);
			pRow->PutValue(2, _variant_t("15 to 18 months"));
			pRow->PutValue(3, (long) 15);
			pRow->PutValue(4, (long) 18);
			pRow->PutValue(5, (long) 1);
			m_pRangeList->InsertRow(pRow, 5);
			m_aryAddList.Add(6);

			pRow = m_pRangeList->GetRow(-1);
			pRow->PutValue(0, (long) 7);
			pRow->PutValue(1, (long) 7);
			pRow->PutValue(2, _variant_t("18 to 24 months"));
			pRow->PutValue(3, (long) 18);
			pRow->PutValue(4, (long) 24);
			pRow->PutValue(5, (long) 1);
			m_pRangeList->InsertRow(pRow, 6);
			m_aryAddList.Add(7);

			pRow = m_pRangeList->GetRow(-1);
			pRow->PutValue(0, (long) 8);
			pRow->PutValue(1, (long) 8);
			pRow->PutValue(2, _variant_t("2 to 3 years"));
			pRow->PutValue(3, (long) 2);
			pRow->PutValue(4, (long) 3);
			pRow->PutValue(5, (long) 2);
			m_pRangeList->InsertRow(pRow, 7);
			m_aryAddList.Add(8);

			
			pRow = m_pRangeList->GetRow(-1);
			pRow->PutValue(0, (long) 9);
			pRow->PutValue(1, (long) 9);
			pRow->PutValue(2, _variant_t("3 to 4 years"));
			pRow->PutValue(3, (long) 3);
			pRow->PutValue(4, (long) 4);
			pRow->PutValue(5, (long) 2);
			m_pRangeList->InsertRow(pRow, 8);
			m_aryAddList.Add(9);

			
			pRow = m_pRangeList->GetRow(-1);
			pRow->PutValue(0, (long) 10);
			pRow->PutValue(1, (long) 10);
			pRow->PutValue(2, _variant_t("4 to 5 years"));
			pRow->PutValue(3, (long) 4);
			pRow->PutValue(4, (long) 5);
			pRow->PutValue(5, (long) 2);
			m_pRangeList->InsertRow(pRow, 9);
			m_aryAddList.Add(10);

			m_bNeedToSave = TRUE;
			
			// (a.walling 2010-09-08 13:26) - PLID 40377 - Yet another transaction rollback despite there being no active transaction
		}NxCatchAll("Error restoring defaults"); //, RollbackTrans("RestoreRangeDefaults"););
	}
	
}
