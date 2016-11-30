// SchedulerStatusDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SchedulerStatusDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (z.manning, 08/06/2007) - PLID 26962 - Added a column for count used.
// (c.haag 2010-10-26 10:07) - PLID 39199 - Added a column for Show in Waiting Area
typedef enum { C_ID = 0, C_NAME, C_SYMBOL, C_SHOWINWAITINGAREA, C_COLOR, C_ACTUALCOLOR, C_COUNT_USED } EColumns;

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSchedulerStatusDlg dialog


CSchedulerStatusDlg::CSchedulerStatusDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSchedulerStatusDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSchedulerStatusDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSchedulerStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSchedulerStatusDlg)
	DDX_Control(pDX, IDC_BTN_ADDSTATUS, m_btnAddStatus);
	DDX_Control(pDX, IDC_BTN_DELETESTATUS, m_btnDeleteStatus);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSchedulerStatusDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSchedulerStatusDlg)
	ON_BN_CLICKED(IDC_BTN_ADDSTATUS, OnBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DELETESTATUS, OnBtnDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSchedulerStatusDlg message handlers

BOOL CSchedulerStatusDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnAddStatus.AutoSet(NXB_NEW);
	m_btnDeleteStatus.AutoSet(NXB_DELETE);
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_dlStatus = BindNxDataListCtrl(this, IDC_MULTIPURPOSE_DURATION_LIST, GetRemoteData(), true);
	m_adwDeletedStatusIDs.RemoveAll();
	m_adwEditedStatusIDs.RemoveAll();
	m_astrDeletedStatusNames.RemoveAll();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CSchedulerStatusDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSchedulerStatusDlg)
	ON_EVENT(CSchedulerStatusDlg, IDC_MULTIPURPOSE_DURATION_LIST, 18 /* RequeryFinished */, OnRequeryFinishedMultipurposeDurationList, VTS_I2)
	ON_EVENT(CSchedulerStatusDlg, IDC_MULTIPURPOSE_DURATION_LIST, 9 /* EditingFinishing */, OnEditingFinishingMultipurposeDurationList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CSchedulerStatusDlg, IDC_MULTIPURPOSE_DURATION_LIST, 10 /* EditingFinished */, OnEditingFinishedMultipurposeDurationList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CSchedulerStatusDlg, IDC_MULTIPURPOSE_DURATION_LIST, 8 /* EditingStarting */, OnEditingStartingMultipurposeDurationList, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CSchedulerStatusDlg, IDC_MULTIPURPOSE_DURATION_LIST, 19 /* LeftClick */, OnLeftClickMultipurposeDurationList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CSchedulerStatusDlg::IsSystemStatus(long nID)
{
	return (nID >= 0 && nID < 5);
}

void CSchedulerStatusDlg::OnEditingStartingMultipurposeDurationList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	COleVariant vID = m_dlStatus->GetValue(nRow,C_ID);
	CString str;

	if (vID.vt == VT_I4)
	{
		if (IsSystemStatus(VarLong(vID)))
		{
			MsgBox("You may not modify system-defined statuses");
			*pbContinue = FALSE;
		}
	}
}

void CSchedulerStatusDlg::OnRequeryFinishedMultipurposeDurationList(short nFlags) 
{
	// (c.haag 2004-09-13 09:42) - PLID 10907 - Color the statuses
	for (long i=0; i < m_dlStatus->GetRowCount(); i++)
	{
		IRowSettingsPtr pRow = m_dlStatus->GetRow(i);
		pRow->CellBackColor[C_COLOR] = VarLong(m_dlStatus->GetValue(i, C_ACTUALCOLOR));
	}

	// Color the first five statuses in gray to signal that the user can't modify them
	for (i=0; i < m_dlStatus->GetRowCount(); i++)
	{
		long nID = VarLong(m_dlStatus->GetValue(i, C_ID));
		if (IsSystemStatus(nID))
		{		
			IRowSettingsPtr pRow = m_dlStatus->GetRow(i);
			pRow->ForeColor = RGB(127,127,127);
		}
	}
}

void CSchedulerStatusDlg::OnBtnAdd() 
{
	try {
		CString strName, strSymbol;
		BOOL bValid = FALSE;
		while (!bValid)
		{
			if (IDCANCEL == InputBoxLimited(this, "Enter a name for the new status", strName, "",255,false,false,NULL))
				return;

			strName.TrimRight();
			if (strName.IsEmpty()) {
				MsgBox("The name you entered is invalid.");
				return;
			}

			//DRT 10/7/03 - PLID 9720 - we can't use a FindByColumn because SQL is not case sensitve, this function is.
/*			else if (-1 != m_dlStatus->FindByColumn(1, (LPCTSTR)strName, 0, FALSE))
			{
				MsgBox("This status name already exists. Please choose another.");
			}
*/

			//TES 12/15/2003 - PLID 10418 - Um, DRT seems to have been confused here.  It should indeed be the name that 
			//it's looking at.  Note the CompareNoCase(strName).
			//DRT 11/20/2003 - PLID 9930 - This loop was looking at column 1 (not the enum) ... the Name field!  Changed
			//	to look at the status field, which it should have been all along
			for(int i = 0;  i < m_dlStatus->GetRowCount(); i++) {
				CString strFind = VarString(m_dlStatus->GetValue(i, C_NAME));

				if(!strFind.CompareNoCase(strName)) {
					MsgBox("This status name already exists.  Please choose another.");
					return;
				}
			}
			if (!strName.CompareNoCase("Confirmed") || !strName.CompareNoCase("Unconfirmed") ||
				!strName.CompareNoCase("Move-Up") || !strName.CompareNoCase("Move Up") ||
				!strName.CompareNoCase("Un-confirmed") || !strName.CompareNoCase("MoveUp") ||
				!strName.CompareNoCase("Cut") || !strName.CompareNoCase("Copy") ||
				!strName.CompareNoCase("Paste") || !strName.CompareNoCase("Edit"))
			{
				MsgBox("The name '%s' is reserved for system use. Please enter a different name.", strName);
				return;
			}

			//if we get here, we've passed all checks
			bool bValidSymbolFound = false;
			for(int nChar = 0; nChar < strName.GetLength() && !bValidSymbolFound; nChar++) {
				strSymbol = strName.Mid(nChar, 1);
				//Now, let's check for a duplicate symbol.
				bool bDuplicateFound = false;
				for(i = 0; i < m_dlStatus->GetRowCount() && !bDuplicateFound; i++) {
					CString strFind = VarString(m_dlStatus->GetValue(i, C_SYMBOL));
					if(!strFind.CompareNoCase(strSymbol)) {
						bDuplicateFound = true;
					}
				}
				if(!bDuplicateFound) {
					//Great!
					bValidSymbolFound = true;
				}
			}
			if(!bValidSymbolFound) {
				//Well, we need to find SOMETHING to use for the symbol.
				CString strAllChars = "abcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()[]\\{}|;':\",./<>?`~";
				for(int nChar = 0; nChar < strAllChars.GetLength() && !bValidSymbolFound; nChar++) {
					strSymbol = strAllChars.Mid(nChar, 1);
					//Now, let's check for a duplicate symbol.
					bool bDuplicateFound = false;
					for(i = 0; i < m_dlStatus->GetRowCount() && !bDuplicateFound; i++) {
						CString strFind = VarString(m_dlStatus->GetValue(i, C_SYMBOL));
						if(!strFind.CompareNoCase(strSymbol)) {
							bDuplicateFound = true;
						}
					}
					if(!bDuplicateFound) {
						//Great!
						bValidSymbolFound = true;
					}
				}
			}
			if(!bValidSymbolFound) {
				//Good Lord!  We give up.
				MsgBox("Practice could not find a valid character to use as the symbol for this status.  Please delete some statuses before continuing.");
				return;
			}	
					
			bValid = TRUE;
		}

		// Add the status to the list
		IRowSettingsPtr pRow = m_dlStatus->GetRow(-1);
		pRow->PutValue(C_SYMBOL, (LPCTSTR)strSymbol);
		pRow->PutValue(C_NAME, (LPCTSTR)strName);
		pRow->PutValue(C_ACTUALCOLOR, (long)RGB(255,255,255));
		pRow->PutValue(C_SHOWINWAITINGAREA, g_cvarFalse); // (c.haag 2010-10-26 10:13) - PLID 39199
		pRow->PutValue(C_COUNT_USED, (long)0); // (z.manning, 08/06/2007) - PLID 26962
		m_dlStatus->AddRow(pRow);
		m_dlStatus->Sort();

		// Now that we added the item, conjure the edit box over the symbol
		long nNewRow = m_dlStatus->FindByColumn(1, (LPCTSTR)strName, 0, FALSE);
		ASSERT(nNewRow > -1);
		m_dlStatus->StartEditing(nNewRow, C_SYMBOL);

	} NxCatchAll("Error adding new status.");
}

void CSchedulerStatusDlg::OnBtnDelete() 
{
	if (m_dlStatus->CurSel < 0)
	{
		MsgBox("Please select a status to delete.");
		return;
	}

	if (m_dlStatus->GetValue(m_dlStatus->CurSel, C_ID).vt == VT_I4)
	{
		long nID = VarLong(m_dlStatus->GetValue(m_dlStatus->CurSel, C_ID));	
		if (IsSystemStatus(nID))
		{
			MsgBox("You may not delete system-defined statuses.");
			return;
		}
		m_adwDeletedStatusIDs.Add(nID);
		m_astrDeletedStatusNames.Add(VarString(m_dlStatus->GetValue(m_dlStatus->CurSel, C_NAME)));
	}

	if(MsgBox(MB_YESNO, "Are you sure you wish to delete this status?") == IDNO)
		return;

	m_dlStatus->RemoveRow(m_dlStatus->CurSel);

/*
	try
	{
		ExecuteSql("DELETE FROM AptShowStateT WHERE ID = %d", nID);
		m_dlStatus->RemoveRow(m_dlStatus->CurSel);
		m_dlStatus->CurSel = -1;
	}
	NxCatchAll("Error deleting appointment status");*/
}

void CSchedulerStatusDlg::OnEditingFinishingMultipurposeDurationList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	COleVariant v = *pvarNewValue;
	COleVariant vID = m_dlStatus->GetValue(nRow,C_ID);
	CString str;
	int i;

	if (vID.vt == VT_I4)
	{
		if (IsSystemStatus(VarLong(vID)))
		{
			MsgBox("You may not modify system-defined statuses");
			*pbCommit = FALSE;
			return;
		}
	}
	
	// (z.manning, 08/06/2007) - PLID 26962 - If any of these are used in either history or for appointments
	// then warn them they're changing this for everything.
	if(varOldValue.vt == VT_BSTR && pvarNewValue->vt == VT_BSTR)
	{
		if(*pbCommit && VarLong(m_dlStatus->GetValue(nRow, C_COUNT_USED),0) > 0 && VarString(varOldValue) != VarString(*pvarNewValue)) 
		{
			int nResult = MessageBox("This status is being used by appointments and changing it will change it "
				"on all existing appointments and reports.\r\n\r\nAre you sure you want to "
				"change this status?", NULL, MB_YESNO);
			if(nResult != IDYES) {
				*pbCommit = FALSE;
				return;
			}
		}
	}
	// (c.haag 2010-10-26 10:13) - PLID 39199 - We now have an editable checkbox. 
	else if (C_SHOWINWAITINGAREA == nCol) {
		if(*pbCommit && VarLong(m_dlStatus->GetValue(nRow, C_COUNT_USED),0) > 0 && VarBool(varOldValue) != VarBool(*pvarNewValue)) 
		{
			int nResult = MessageBox("This status is being used by appointments. Changing it may change their appearance "
				"in the Room Manager Waiting Area list.\r\n\r\nAre you sure you want to "
				"change this status?", NULL, MB_YESNO | MB_ICONQUESTION);
			if(nResult != IDYES) {
				*pbCommit = FALSE;
				return;
			}
		}
	}
	else {
		// (z.manning, 08/06/2007) - PLID 26962 - The only 2 columns that are editable are string fields.
		ASSERT(FALSE);
	}

	switch (nCol)
	{
	case C_NAME:
		str = VarString(v, "");
		str.TrimRight();
		if (str.GetLength() == 0)
		{
			*pbCommit = FALSE;
		}
		else if (str.GetLength() > 255)
		{
			MsgBox("Please choose a name shorter than 255 characters.");
			*pbCommit = FALSE;
		}
		for (i=0; i < m_dlStatus->GetRowCount(); i++)
		{
			if (i == nRow)
				continue;

			//DRT 10/7/03 - PLID 9720 - MUST compare no case!
			if (!str.CompareNoCase(VarString(m_dlStatus->GetValue(i,C_NAME),"")))
			{
				MsgBox("The name '%s' already exists in another status.", str);
				*pbCommit = FALSE;
			}
		}
		if (!str.CompareNoCase("Confirmed") || !str.CompareNoCase("Unconfirmed") ||
			!str.CompareNoCase("Move-Up") || !str.CompareNoCase("Move Up") ||
			!str.CompareNoCase("Un-confirmed") || !str.CompareNoCase("MoveUp") ||
			!str.CompareNoCase("Cut") || !str.CompareNoCase("Copy") ||
			!str.CompareNoCase("Paste") || !str.CompareNoCase("Edit"))
		{
			MsgBox("The name '%s' is reserved for system use.", str);
			*pbCommit = FALSE;
		}
		break;
	case C_SYMBOL:
		str = VarString(v, "");
		str.TrimRight();
		if (str.GetLength() == 0)
		{
			*pbCommit = FALSE;
		}
		else if (str.GetLength() > 1)
		{
			MsgBox("A symbol can only be one letter or number long.");
			*pbCommit = FALSE;
		}
		for (i=0; i < m_dlStatus->GetRowCount(); i++)
		{
			if (i == nRow)
				continue;

			if (!str.CompareNoCase(VarString(m_dlStatus->GetValue(i,C_SYMBOL),"")))
			{
				if ((str[0] >= 'A' && str[0] <= 'Z') || (str[0] >= 'a' && str[0] <= 'z'))
				{
					if (str == VarString(m_dlStatus->GetValue(i,C_SYMBOL),""))
						MsgBox("The letter '%s' already exists for the '%s' status.", str, VarString(m_dlStatus->GetValue(i,C_NAME),""));
					else
						MsgBox("The letter '%s' already exists for the '%s' status, and you may not assign a letter more than once even if the case is different.", VarString(m_dlStatus->GetValue(i,C_SYMBOL),""), VarString(m_dlStatus->GetValue(i,C_NAME),""));
				}
				else
					MsgBox("The symbol '%s' already exists for the '%s' status.", str, VarString(m_dlStatus->GetValue(i,C_NAME),""));
				*pbCommit = FALSE;
			}
		}
		break;
	default:
		break;
	}
}

void CSchedulerStatusDlg::OnEditingFinishedMultipurposeDurationList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	COleVariant v = m_dlStatus->GetValue(nRow, C_ID);
	if (v.vt == VT_I4)
		m_adwEditedStatusIDs.Add(v.lVal);
}

void CSchedulerStatusDlg::OnOK() 
{
	// Check for duplicate symbols
	BOOL bDuplicateSymbols = FALSE;
	for (long i=0; i < m_dlStatus->GetRowCount() && !bDuplicateSymbols; i++)
	{
		CString strSymbol = VarString(m_dlStatus->GetValue(i, C_SYMBOL));
		for (long j=0; j < m_dlStatus->GetRowCount() && !bDuplicateSymbols; j++)
		{
			if (j == i)
				continue;
			if (strSymbol == VarString(m_dlStatus->GetValue(j, C_SYMBOL)))
			{
				bDuplicateSymbols = TRUE;
			}
		}
	}
	if (bDuplicateSymbols)
	{
		//DRT 11/20/2003 - PLID 9930 - If there are duplicates, we WILL NOT save, under any circumstances.  The code previous to this
		//		should prevent you from ever reaching this case.
		MsgBox("There are duplicate symbols in the list.  You must remove all duplicates before saving the information.");
		return;
	}

	try {
		// Traverse the list editing existing statuses
		for (long i=0; i < m_adwEditedStatusIDs.GetSize(); i++)
		{
			long nID = m_adwEditedStatusIDs[i];
			long nRow = m_dlStatus->FindByColumn(C_ID, nID, 0, FALSE);
			if (nRow > -1)
			{
				// (c.haag 2010-10-26 10:13) - PLID 39199 - Parameterized, added WaitingArea
				ExecuteParamSql("UPDATE AptShowStateT SET Name = {STRING}, Symbol = {STRING}, "
					"WaitingArea = {INT}, Color = {INT} WHERE ID = {INT}",	
					VarString(m_dlStatus->GetValue(nRow, C_NAME)),
					VarString(m_dlStatus->GetValue(nRow, C_SYMBOL)),
					VarBool(m_dlStatus->GetValue(nRow, C_SHOWINWAITINGAREA)),
					VarLong(m_dlStatus->GetValue(nRow, C_ACTUALCOLOR)), nID);
			}
		}
		// Traverse the list adding new statuses
		for (i=0; i < m_dlStatus->GetRowCount(); i++)
		{
			if (m_dlStatus->GetValue(i, C_ID).vt != VT_I4)
			{
				// (c.haag 2010-10-26 10:13) - PLID 39199 - Parameterized, added WaitingArea
				ExecuteParamSql("INSERT INTO AptShowStateT (ID, [Name], Symbol, WaitingArea, Color) "
					"VALUES ({INT}, {STRING}, {STRING}, {INT}, {INT})",
					NewNumber("AptShowStateT", "ID"),
					VarString(m_dlStatus->GetValue(i, C_NAME)),
					VarString(m_dlStatus->GetValue(i, C_SYMBOL)),
					VarBool(m_dlStatus->GetValue(i, C_SHOWINWAITINGAREA)),
					VarLong(m_dlStatus->GetValue(i, C_ACTUALCOLOR)));
			}
		}
		// Traverse the list deleting existing statuses
		for (i=0; i < m_adwDeletedStatusIDs.GetSize(); i++)
		{
			CString str;
			long nID = m_adwDeletedStatusIDs[i];
			str.Format("SELECT ID FROM AppointmentsT WHERE ShowState = %d", nID);
			long nRecordCount = GetRecordCount(str);
			if (nRecordCount)
			{
				str.Format("SELECT ID FROM AppointmentsT WHERE ShowState = %d AND Status = 4", nID);
				MsgBox("The status '%s' has %d appointment(s) associated with it; and of those, %d are cancelled. This status will not be deleted.",
					m_astrDeletedStatusNames[i], nRecordCount, GetRecordCount(str));
			}
			else
			{
				//check to see if it was used in AptShowStateHistoryT
				CString strSql;
				strSql.Format("SELECT ShowStateID FROM AptShowStateHistoryT WHERE ShowStateID = %li", nID);
				long nStatusCount = GetRecordCount(strSql);
				if (nStatusCount) {
					
					//give them a warning
					CString strMsg;
					strMsg.Format("The status '%s' has %li records associated with it that is used for reporting purposes.  Deleting this status will render this information useless. \nAre you sure you want delete this status?", m_astrDeletedStatusNames[i], nStatusCount);
					if (IDYES == MsgBox(MB_YESNO, strMsg)) {
						
						ExecuteSql("DELETE FROM AptShowStateHistoryT WHERE ShowStateID = %li", nID);
						ExecuteSql("DELETE FROM AptShowStateT WHERE ID = %d", nID);
					}
					else {
						//we don't return, but we don't do anything else either
					}
				}
				else {
					
					//there are no showstate hostories with this item, so just delete the appt
					ExecuteSql("DELETE FROM AptShowStateT WHERE ID = %d", nID);
				}

			}
		}
		CClient::RefreshTable(NetUtils::AptShowStateT);
		// (a.wilson 2014-08-12 11:04) - PLID 63199 - this should not update all appointments.
		//CClient::RefreshTable(NetUtils::AppointmentsT);
		CDialog::OnOK();
	}
	NxCatchAll("Error saving appointment status list");
}

void CSchedulerStatusDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CSchedulerStatusDlg::OnLeftClickMultipurposeDurationList(long nRow, short nCol, long x, long y, long nFlags) 
{
	// (c.haag 2004-09-30 11:21) - PLID 10907 - Let the user edit the color in the list.
	// Unfortunately, we cannot hide the highlight from that column.
	if (nRow > -1 && nCol == C_COLOR) {
		COLORREF clr = VarLong(m_dlStatus->GetValue(nRow, C_ACTUALCOLOR));
		CColorDialog dlg(clr, CC_ANYCOLOR|CC_RGBINIT);
		if (dlg.DoModal() == IDOK) {
			IRowSettingsPtr pRow = m_dlStatus->GetRow(nRow);
			m_dlStatus->PutValue(nRow, C_ACTUALCOLOR, (long)dlg.m_cc.rgbResult);
			pRow->CellBackColor[C_COLOR] = (long)dlg.m_cc.rgbResult;			

			// (c.haag 2004-09-30 11:42) - Flag the row as having been modified
			COleVariant v = m_dlStatus->GetValue(nRow, C_ID);
			if (v.vt == VT_I4) {
				m_adwEditedStatusIDs.Add(v.lVal);
			}
		}
	}
}
