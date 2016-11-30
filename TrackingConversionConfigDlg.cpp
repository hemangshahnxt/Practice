// TrackingConversionConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "reports.h"
#include "TrackingConversionConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum LadderListColumns {
	llcID = 0,
	llcName,
};

enum BeginStepColumns {
	bscID = 0,
	bscStepOrder,
	bscName,
};

enum EndStepColumns {
	escID = 0,
	escOrder, 
	escName,
};

enum ConversionListColumns {
	clcLadderTemplateID = 0,
	clcBeginStepID,
	clcEndStepID,
	clcOrder,
	clcConversionName,
	clcBeginStepName,
	clcEndStepName,
};
	
#define ID_REMOVE_CONVERSION_PHASE 49149

// (j.gruber 2007-08-17 15:18) - PLID 27091 - Created For
/////////////////////////////////////////////////////////////////////////////
// CTrackingConversionConfigDlg dialog


CTrackingConversionConfigDlg::CTrackingConversionConfigDlg(CWnd* pParent)
	: CNxDialog(CTrackingConversionConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTrackingConversionConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTrackingConversionConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTrackingConversionConfigDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_TRACK_CONFIG_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_TRACK_CONFIG_DOWN, m_btnDown);
	DDX_Control(pDX, IDC_TRACK_CONFIG_UP, m_btnUp);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CTrackingConversionConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CTrackingConversionConfigDlg)
	ON_BN_CLICKED(IDC_TRACK_CONFIG_ADD, OnTrackConfigAdd)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_TRACK_CONFIG_DOWN, OnTrackConfigDown)
	ON_BN_CLICKED(IDC_TRACK_CONFIG_UP, OnTrackConfigUp)
	ON_COMMAND(ID_REMOVE_CONVERSION_PHASE, OnRemoveTrackingConversion)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTrackingConversionConfigDlg message handlers

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CTrackingConversionConfigDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CTrackingConversionConfigDlg)
	ON_EVENT(CTrackingConversionConfigDlg, IDC_TRACK_CONFIG_CONVERSIONS, 7 /* RButtonUp */, OnRButtonUpTrackConfigConversions, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CTrackingConversionConfigDlg, IDC_TRACK_CONFIG_LADDER_LIST, 16 /* SelChosen */, OnSelChosenTrackConfigLadderList, VTS_DISPATCH)
	ON_EVENT(CTrackingConversionConfigDlg, IDC_TRACK_CONFIG_LADDER_LIST, 18 /* RequeryFinished */, OnRequeryFinishedTrackConfigLadderList, VTS_I2)
	ON_EVENT(CTrackingConversionConfigDlg, IDC_TRACK_CONFIG_CONVERSIONS, 10 /* EditingFinished */, OnEditingFinishedTrackConfigConversions, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CTrackingConversionConfigDlg, IDC_TRACK_CONFIG_CONVERSIONS, 9 /* EditingFinishing */, OnEditingFinishingTrackConfigConversions, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CTrackingConversionConfigDlg, IDC_TRACK_CONFIG_START_STEP, 2 /* SelChanged */, OnSelChangedTrackConfigStartStep, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CTrackingConversionConfigDlg, IDC_TRACK_CONFIG_CONVERSIONS, 2 /* SelChanged */, OnSelChangedTrackConfigConversions, VTS_DISPATCH VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CTrackingConversionConfigDlg::OnTrackConfigAdd() 
{

	try { 
		NXDATALIST2Lib::IRowSettingsPtr pRowStart = m_pBeginStepList->CurSel;
		NXDATALIST2Lib::IRowSettingsPtr pRowEnd = m_pEndStepList->CurSel;
		NXDATALIST2Lib::IRowSettingsPtr pRowTop = m_pLadderList->CurSel;

		long nLadderTemplateID = -1;

		if (pRowTop) {
			nLadderTemplateID = VarLong(pRowTop->GetValue(llcID));
		}
		else {
			MsgBox("Please choose a ladder template.");
			return;
		}


		long nStartID, nEndID;
		CString strStart, strEnd;

		if (pRowStart) {
			nStartID = VarLong(pRowStart->GetValue(bscID));
			strStart = VarString(pRowStart->GetValue(bscName));
		}
		else {
			MsgBox("Please choose a starting step.");
			return;
		}

		if (pRowEnd) {
			nEndID = VarLong(pRowEnd->GetValue(escID));
			strEnd = VarString(pRowEnd->GetValue(escName));
		}
		else {
			MsgBox("Please select an ending step.");
			return;
		}

		//now add the row 
		NXDATALIST2Lib::IRowSettingsPtr pRowConv;
		pRowConv = m_pConversionList->GetNewRow();

		if (pRowConv) {
			long nOrderID = m_pConversionList->GetRowCount() + 1;

			pRowConv->PutValue(clcLadderTemplateID, nLadderTemplateID);
			pRowConv->PutValue(clcBeginStepID, (long)nStartID);
			pRowConv->PutValue(clcEndStepID, (long) nEndID);
			pRowConv->PutValue(clcOrder, (long) nOrderID);
			pRowConv->PutValue(clcConversionName, _variant_t("<Enter Conversion Name>"));
			pRowConv->PutValue(clcBeginStepName, _variant_t(strStart));
			pRowConv->PutValue(clcEndStepName, _variant_t(strEnd));

			//now add it to the data
			ExecuteSql(" INSERT INTO TrackingConversionT (LadderTemplateID, BeginStepTemplateID, EndStepTemplateID, "
				" StepOrder, Name) "
				" VALUES "
				" (%li, %li, %li, %li, '%s')",
				nLadderTemplateID, nStartID, nEndID, nOrderID, _Q("<Enter Conversion Name>"));

			//now add the row
			m_pConversionList->AddRowAtEnd(pRowConv, NULL);

			//and set it to editing
			m_pConversionList->StartEditing(pRowConv, clcConversionName);
		}

	}NxCatchAll("Error in CTrackingConversionConfigDlg::OnTrackConfigAdd() ");		

	
}

void CTrackingConversionConfigDlg::OnRButtonUpTrackConfigConversions(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			m_pConversionList->CurSel = pRow;

			CMenu menPopup;
			menPopup.m_hMenu = CreatePopupMenu();
			menPopup.InsertMenu(1, MF_BYPOSITION, ID_REMOVE_CONVERSION_PHASE, "Remove");

			CPoint pt(x,y);
			CWnd* pWnd = GetDlgItem(IDC_TRACK_CONFIG_CONVERSIONS);

			if (pWnd != NULL) {
				pWnd->ClientToScreen(&pt);
				menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
			}
			else {
				ThrowNxException("Error in CTrackingConversionConfigDlg::OnRButtonUpTrackConfigConversions: An error ocurred while creating menu");
			}
		}
	}NxCatchAll("Error in OnRButtonUpTrackConfigConversions");
	
}

void CTrackingConversionConfigDlg::OnCancel() {

	CDialog::OnCancel();

}
void CTrackingConversionConfigDlg::OnOK() 
{
		
	CDialog::OnOK();
}

void CTrackingConversionConfigDlg::OnSelChosenTrackConfigLadderList(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		m_pEndStepList->Clear();

		if (pRow) {
			long nID = VarLong(pRow->GetValue(llcID));

			CString strWhere;
			strWhere.Format(" Inactive = 0 AND LadderTemplateID = %li", nID);

			m_pBeginStepList->WhereClause = _bstr_t(strWhere);
			m_pBeginStepList->Requery();

			strWhere.Format(" LadderTemplateID = %li", nID);

			m_pConversionList->WhereClause = _bstr_t(strWhere);
			m_pConversionList->Requery();
		}	
	}NxCatchAll("Error in OnSelChosenTrackConfigLadderList");
	
}

HBRUSH CTrackingConversionConfigDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if (nCtlColor == CTLCOLOR_STATIC) {
		extern CPracticeApp theApp;
		pDC->SelectPalette(&theApp.m_palette, FALSE);
		pDC->RealizePalette();
		pDC->SetBkColor(PaletteColor(0x00DEB05C));
		return m_brush;
	}
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
	*/

	// (a.walling 2008-04-02 09:14) - PLID 29497 - Deprecated, use base class
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CTrackingConversionConfigDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog(); // (a.walling 2011-01-14 16:44) - no PLID - Fix bad base class OnInitDialog call

		m_pLadderList = BindNxDataList2Ctrl(IDC_TRACK_CONFIG_LADDER_LIST, GetRemoteData(), TRUE);
		m_pBeginStepList = BindNxDataList2Ctrl(IDC_TRACK_CONFIG_START_STEP, GetRemoteData(), FALSE);
		m_pEndStepList = BindNxDataList2Ctrl(IDC_TRACK_CONFIG_END_STEP, GetRemoteData(), FALSE);
		m_pConversionList = BindNxDataList2Ctrl(IDC_TRACK_CONFIG_CONVERSIONS, GetRemoteData(), FALSE);

		CString strFrom;
		strFrom = " (SELECT TrackingConversionT.LadderTemplateID, TrackingConversionT.Name, "
			" TrackingConversionT.BeginStepTemplateID as BeginStepID, TrackingConversionT.EndStepTemplateID as EndStepID, "
			" TrackingConversionT.StepOrder, BeginStepTempsT.StepName as BeginStepName, EndStepTempsT.StepName as EndStepName "
			" FROM TrackingConversionT LEFT JOIN StepTemplatesT BeginStepTempsT ON "
			" TrackingConversionT.BeginStepTemplateID = BeginStepTempsT.ID "
			" LEFT JOIN StepTemplatesT EndStepTempsT ON "
			" TrackingConversionT.EndStepTemplateID = EndStepTempsT.ID) Q ";

		m_pConversionList->FromClause = _bstr_t(strFrom);
		
		m_brush.CreateSolidBrush(PaletteColor(0x00DEB05C));
		m_btnUp.AutoSet(NXB_UP);
		m_btnDown.AutoSet(NXB_DOWN);
		// (z.manning, 04/28/2008) - PLID 29807 - More button styles
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnOK.AutoSet(NXB_CLOSE);

		CheckButtonStatus();

	}NxCatchAll("Error in OnInitDialog");

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTrackingConversionConfigDlg::OnRequeryFinishedTrackConfigLadderList(short nFlags) 
{
	try {
		//set the selection to the first ladder in the list
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pLadderList->GetFirstRow();
		if (pRow) {
			m_pLadderList->CurSel = pRow;
			OnSelChosenTrackConfigLadderList(m_pLadderList->GetFirstRow());
		}
	}NxCatchAll("Error in CTrackingConversionConfigDlg::OnRequeryFinishedTrackConfigLadderList");
	
}

void CTrackingConversionConfigDlg::OnEditingFinishedTrackConfigConversions(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		if (bCommit) {
			switch (nCol) {
				case clcConversionName:
					{
						NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

						if (pRow) {
							NXDATALIST2Lib::IRowSettingsPtr pRowLadder = m_pLadderList->CurSel;
							if (pRowLadder) {
								long nLadderID = VarLong(pRowLadder->GetValue(llcID));
								long nOrder = VarLong(pRow->GetValue(clcOrder));

								ExecuteParamSql("UPDATE TrackingConversionT SET Name = {STRING} WHERE LadderTemplateID = {INT} AND StepOrder = {INT}",
									_T(VarString(varNewValue, "")), nLadderID, nOrder);
							}
						}
					}
				break;					
			}
		}		
	}NxCatchAll("Error in CTrackingConversionConfigDlg::OnEditingFinishedTrackConfigConversions");
}

void CTrackingConversionConfigDlg::OnEditingFinishingTrackConfigConversions(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		switch (nCol) {
			case clcConversionName:
				{
					if (*pbCommit) {
						CString strValue = VarString(pvarNewValue);

						strValue.TrimLeft();
						strValue.TrimRight();

						if (strValue.IsEmpty()) {
							MsgBox("The conversion name cannot be blank.");
							*pbCommit = FALSE;
							*pbContinue = FALSE;
						}
					}
				}
			break;
		}
	}NxCatchAll("Error in CTrackingConversionConfigDlg::OnEditingFinishingTrackConfigConversions");
	
}

void CTrackingConversionConfigDlg::OnSelChangedTrackConfigStartStep(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		//get the order ID that we are on
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);

		if (pRow) {
			long nOrderID = VarLong(pRow->GetValue(bscStepOrder));
	
			pRow  = m_pLadderList->CurSel;

			if (pRow) {
				long nID = VarLong(pRow->GetValue(llcID));

				CString strWhere;
				strWhere.Format(" StepOrder > %li AND Inactive = 0 AND LadderTemplateID = %li", nOrderID, nID);

				m_pEndStepList->WhereClause = _bstr_t(strWhere);
				m_pEndStepList->Requery();
			}
		}
	}NxCatchAll("Error in OnSelChangedTrackConfigStartStep");
	
	
}

void CTrackingConversionConfigDlg::OnTrackConfigDown() 
{
	try {
		//we just need to switch the two rows
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConversionList->CurSel;

		if (pRow) {
			if (pRow != m_pConversionList->GetLastRow()) {
				long nOrigOrderID = VarLong(pRow->GetValue(clcOrder));
				long nNewOrderID = nOrigOrderID + 1;

				NXDATALIST2Lib::IRowSettingsPtr pRowLadder = m_pLadderList->CurSel;
			
				if (pRowLadder) {
					long nLadderTemplateID = VarLong(pRowLadder->GetValue(llcID));

					ExecuteParamSql(" UPDATE TrackingConversionT SET StepOrder = {INT} WHERE StepOrder = {INT} AND LadderTemplateID = {INT}; "
						" UPDATE TrackingConversionT SET StepOrder = {INT} WHERE StepOrder = {INT} AND LadderTemplateID = {INT}; "
						" UPDATE TrackingConversionT SET StepOrder = {INT} WHERE StepOrder = {INT} AND LadderTemplateID = {INT}; "
						,-1, nNewOrderID, nLadderTemplateID,
						nNewOrderID, nOrigOrderID, nLadderTemplateID,
						nOrigOrderID, -1, nLadderTemplateID);
										
					pRow->PutValue(clcOrder, nNewOrderID);

					pRow = pRow->GetNextRow();
				
					if (pRow) {
						pRow->PutValue(clcOrder, nOrigOrderID);
					}

					m_pConversionList->Sort();

					CheckButtonStatus();
				}
			}
		}
	}NxCatchAll("Error moving step down");
			
	
}

void CTrackingConversionConfigDlg::OnTrackConfigUp() 
{

	try {
		//we just need to switch the two rows
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConversionList->CurSel;

		if (pRow) {
			if (pRow != m_pConversionList->GetFirstRow()) {
				long nOrigOrderID = VarLong(pRow->GetValue(clcOrder));
				long nNewOrderID = nOrigOrderID - 1;

				NXDATALIST2Lib::IRowSettingsPtr pRowLadder = m_pLadderList->CurSel;
				
				if (pRowLadder) {
					long nLadderTemplateID = VarLong(pRowLadder->GetValue(llcID));

					ExecuteParamSql("UPDATE TrackingConversionT SET StepOrder = {INT} WHERE StepOrder = {INT} AND LadderTemplateID = {INT}; "
						" UPDATE TrackingConversionT SET StepOrder = {INT} WHERE StepOrder = {INT} AND LadderTemplateID = {INT}; "
						"UPDATE TrackingConversionT SET StepOrder = {INT} WHERE StepOrder = {INT} AND LadderTemplateID = {INT}; ",
						-1, nNewOrderID, nLadderTemplateID,
						nNewOrderID, nOrigOrderID, nLadderTemplateID,
						nOrigOrderID, -1, nLadderTemplateID);
					
					pRow->PutValue(clcOrder, nNewOrderID);

					pRow = pRow->GetPreviousRow();

					if (pRow) {
						pRow->PutValue(clcOrder, nOrigOrderID);
					}
					m_pConversionList->Sort();

					CheckButtonStatus();
				}
			}
		}
	}NxCatchAll("Error moving step up");	
}

void CTrackingConversionConfigDlg::CheckButtonStatus() {

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConversionList->CurSel;

		if (pRow) {
			if (pRow == m_pConversionList->GetFirstRow()) {
				m_btnUp.EnableWindow(FALSE);
			}
			else {
				m_btnUp.EnableWindow(TRUE);
			}

			if (pRow == m_pConversionList->GetLastRow()) {
				m_btnDown.EnableWindow(FALSE);
			}
			else {
				m_btnDown.EnableWindow(TRUE);
			}
		
		}
		else {
			//disable the buttons
			m_btnUp.EnableWindow(FALSE);
			m_btnDown.EnableWindow(FALSE);
		}
	}NxCatchAll("Error in CheckButtonStatus");
}


void CTrackingConversionConfigDlg::OnSelChangedTrackConfigConversions(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	CheckButtonStatus();
}


void CTrackingConversionConfigDlg::OnRemoveTrackingConversion() {

	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;

		pRow = m_pConversionList->CurSel;

		if (pRow) {
			long nLadderID = VarLong(pRow->GetValue(clcLadderTemplateID));
			long nOrder = VarLong(pRow->GetValue(clcOrder));

			CString strSql = BeginSqlBatch();
			
			AddStatementToSqlBatch(strSql, "DELETE FROM TrackingConversionT WHERE LadderTemplateID = %li AND StepOrder = %li", nLadderID, nOrder);

			//loop through the rows under this one and 
			NXDATALIST2Lib::IRowSettingsPtr pRowNext;
			pRowNext = pRow->GetNextRow();
			
			while (pRowNext) {
				AddStatementToSqlBatch(strSql, "UPDATE TrackingConversionT SET StepOrder = %li WHERE LadderTemplateID = %li AND StepOrder = %li",
					nOrder, nLadderID, VarLong(pRowNext->GetValue(clcOrder)));

				pRowNext->PutValue(clcOrder, nOrder);

				nOrder++;

				pRowNext = pRowNext->GetNextRow();
			}

			ExecuteSqlBatch(strSql);
			
			m_pConversionList->RemoveRow(pRow);

			CheckButtonStatus();
		}
	}NxCatchAllCall("Error in CTrackingConversionConfigDlg::OnRemoveTrackingConversion()", m_pLadderList->Requery());
}