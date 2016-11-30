// EmrGraphSetupDlg.cpp : implementation file
//(a.wilson 2012-4-5) PLID 49479 - implementation of emr graphing setup dialog.

#include "stdafx.h"
#include "Practice.h"
#include "EmrGraphSetupDlg.h"

//enums for the datalist controls.
enum GraphListColumns {
	glcID,
	glcName,
	glcInactive,
};

enum LineListColumns {
	llcID,
	llcGraphID,
	llcName,
	llcColor,
};

enum AddItemListColumns {
	ailcID,
	ailcName,
	ailcFlipped,
	ailcType,
	ailcInputDate,
};

enum ItemListColumns {
	ilcID,
	ilcLineID,
	ilcEMRID,
	ilcName,
	ilcType,
	ilcColumn,
	ilcRow,
	ilcDate,
	ilcFlipped,
};

//defines for comparing datalist values.
#define COLUMN_TYPE 3
#define ROW_TYPE 2
#define SELECT_ALL 0
#define GRAPH_INACTIVE_COLOR RGB(143, 148, 152)
#define GRAPH_ACTIVE_COLOR RGB(0, 0, 0)

using namespace ADODB;
using namespace NXDATALIST2Lib;

// CEmrGraphSetupDlg dialog
IMPLEMENT_DYNAMIC(CEmrGraphSetupDlg, CNxDialog)

CEmrGraphSetupDlg::CEmrGraphSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrGraphSetupDlg::IDD, pParent) {
	m_bGraphChanged = false; 
	m_bLineChanged = false;
}

CEmrGraphSetupDlg::~CEmrGraphSetupDlg() { }

void CEmrGraphSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_EMRGS_NEWGRAPH, m_btnNewGraph);
	DDX_Control(pDX, IDC_EMRGS_NEWLINE, m_btnNewLine);
	DDX_Control(pDX, IDC_EMRGS_REMOVEGRAPH, m_btnRemoveGraph);
	DDX_Control(pDX, IDC_EMRGS_REMOVELINE, m_btnRemoveLine);
	DDX_Control(pDX, IDC_EMRGS_REMOVEITEM, m_btnRemoveItem);
}
//(a.wilson 2012-4-5) PLID 49479
BOOL CEmrGraphSetupDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnNewGraph.AutoSet(NXB_NEW);
		m_btnNewLine.AutoSet(NXB_NEW);
		m_btnRemoveGraph.AutoSet(NXB_DELETE);
		m_btnRemoveLine.AutoSet(NXB_DELETE);
		m_btnRemoveItem.AutoSet(NXB_DELETE);

		SetDlgItemText(IDC_EMRGS_HELPTEXT, 
			"Each graph contains a list of lines that will be drawn within that graph.  "
			"Each line contains a list of EMR items which have the patient's data to be graphed on that particular line."
			"\r\nAdd EMR items to each line of the graph that you want to display in the graphing pane.");

		m_dlpGraphList = BindNxDataList2Ctrl(IDC_EMRGS_GRAPH_LIST, true);
		m_dlpAddItemList = BindNxDataList2Ctrl(IDC_EMRGS_ADD_ITEM_LIST, false);
		m_dlpLineList = BindNxDataList2Ctrl(IDC_EMRGS_LINE_LIST, false);
		m_dlpItemList = BindNxDataList2Ctrl(IDC_EMRGS_ITEM_LIST, false);

		// (a.wilson 2012-4-30) PLID 50085 - generate the add item list query.
		m_dlpAddItemList->PutFromClause(_bstr_t(
			"(SELECT EMRInfoMasterT.ID, EMRInfoT.Name, "
			"EMRInfoT.TableRowsAsFields as IsFlipped, "
			"CASE WHEN EMRInfoT.DataType = 1 THEN 'Text' ELSE "
			"CASE WHEN EMRInfoT.DataType = 2 THEN 'Single Select' "
			"ELSE CASE WHEN EMRInfoT.DataType = 5 THEN 'Slider' "
			"ELSE CASE WHEN EMRInfoT.DataType = 7 THEN 'Table' "
			"END END END END as DataType, EMRInfoT.InputDate FROM EmrInfoMasterT "
			"INNER JOIN EMRInfoT ON EMRInfoT.ID = EMRInfoMasterT.ActiveEmrInfoID "
			"WHERE EMRInfoT.DataType IN (1, 2, 5, 7) AND EMRInfoT.DataSubType <> 3 "
			"AND EmrInfoMasterT.Inactive = 0) Q"));
		m_dlpAddItemList->Requery();
		
		UpdateActiveControls();
	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CEmrGraphSetupDlg, CNxDialog)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EMRGS_REMOVEITEM, OnRemoveItem)
	ON_COMMAND(ID_EMRGS_REMOVELINE, OnRemoveLine)
	ON_COMMAND(ID_EMRGS_INACTIVATEGRAPH, OnChangeGraphActivity)
	ON_COMMAND(ID_EMRGS_NEWGRAPH, OnNewGraph)
	ON_COMMAND(ID_EMRGS_NEWLINE, OnNewLine)
	ON_COMMAND(ID_EMRGS_REACTIVATEGRAPH, OnChangeGraphActivity)
	ON_BN_CLICKED(IDCANCEL, &CEmrGraphSetupDlg::OnClose)
	ON_COMMAND(ID_EMRGS_REMOVEGRAPH, OnRemoveGraph)
	ON_BN_CLICKED(IDC_EMRGS_NEWGRAPH, OnNewGraph)
	ON_BN_CLICKED(IDC_EMRGS_NEWLINE, OnNewLine)
	ON_BN_CLICKED(IDC_EMRGS_REMOVEGRAPH, OnRemoveGraph)
	ON_BN_CLICKED(IDC_EMRGS_REMOVELINE, OnRemoveLine)
	ON_BN_CLICKED(IDC_EMRGS_REMOVEITEM, OnRemoveItem)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEmrGraphSetupDlg, CNxDialog)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_GRAPH_LIST, 29, CEmrGraphSetupDlg::SelSetEmrgsGraphList, VTS_DISPATCH)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_LINE_LIST, 29, CEmrGraphSetupDlg::SelSetEmrgsLineList, VTS_DISPATCH)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_LINE_LIST, 18, CEmrGraphSetupDlg::RequeryFinishedEmrgsLineList, VTS_I2)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_LINE_LIST, 19, CEmrGraphSetupDlg::LeftClickEmrgsLineList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_ADD_ITEM_LIST, 16, CEmrGraphSetupDlg::SelChosenEmrgsAddItemList, VTS_DISPATCH)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_GRAPH_LIST, 7, CEmrGraphSetupDlg::RightClickEmrgsGraphList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_LINE_LIST, 7, CEmrGraphSetupDlg::RightClickEmrgsLineList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_ITEM_LIST, 7, CEmrGraphSetupDlg::RightClickEmrgsItemList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_GRAPH_LIST, 18, CEmrGraphSetupDlg::RequeryFinishedEmrgsGraphList, VTS_I2)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_ITEM_LIST, 18, CEmrGraphSetupDlg::RequeryFinishedEmrgsItemList, VTS_I2)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_ITEM_LIST, 10, CEmrGraphSetupDlg::EditingFinishedEmrgsItemList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_ITEM_LIST, 8, CEmrGraphSetupDlg::EditingStartingEmrgsItemList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_LINE_LIST, 10, CEmrGraphSetupDlg::EditingFinishedEmrgsLineList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_GRAPH_LIST, 10, CEmrGraphSetupDlg::EditingFinishedEmrgsGraphList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_ITEM_LIST, 29, CEmrGraphSetupDlg::SelSetEmrgsItemList, VTS_DISPATCH)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_GRAPH_LIST, 1, CEmrGraphSetupDlg::SelChangingEmrgsGraphList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_LINE_LIST, 1, CEmrGraphSetupDlg::SelChangingEmrgsLineList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEmrGraphSetupDlg, IDC_EMRGS_GRAPH_LIST, 8, CEmrGraphSetupDlg::EditingStartingEmrgsGraphList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
END_EVENTSINK_MAP()

//(a.wilson 2012-4-30) PLID 50083 - generate lines based on graph list
void CEmrGraphSetupDlg::SelSetEmrgsGraphList(LPDISPATCH lpSel)
{
	try {
		if (!m_bGraphChanged)
			return;
		//clear all the lists first.
		m_dlpLineList->Clear();
		m_dlpItemList->Clear();
		//next, check if a null row was selected.
		IRowSettingsPtr pRow(lpSel);
		if (!pRow) {
			UpdateActiveControls();
			//return if so because we don't need to update the line and item lists.
			return;
		//otherwise, we need to update the line list based on the graph selected, if the graph is not inactive.
		} else if (AsBool(pRow->GetValue(glcInactive)) == FALSE) {
			long nGraphID = pRow->GetValue(glcID);
			//update the line list for the particular graph chosen.
			m_dlpLineList->PutWhereClause(FormatBstr("GraphID = %li", nGraphID));
			m_dlpLineList->SetRedraw(VARIANT_FALSE);
			m_dlpLineList->Requery();
			m_dlpLineList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		}
		UpdateActiveControls();
		m_bGraphChanged = false;
	} NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-4-30) PLID 50084 - generate item list based on line list
void CEmrGraphSetupDlg::SelSetEmrgsLineList(LPDISPATCH lpSel)
{
	try {
		if (!m_bLineChanged)
			return;

		IRowSettingsPtr pRow(lpSel);

		//check for a null row selection
		if (!pRow) {
			m_dlpItemList->Clear();
			UpdateActiveControls();
			return;
		} else {
			long nLineID = pRow->GetValue(llcID);
			
			m_dlpItemList->PutFromClause(FormatBstr(
				"(SELECT EMRGraphLineDetailsT.ID, EMRGraphLineDetailsT.LineID, \r\n"
				"CASE WHEN EMRInfoT.TableRowsAsFields = 1 THEN EMRGraphLineDetailsT.ColumnID \r\n"
				"ELSE EMRGraphLineDetailsT.RowID END AS RowID, \r\n"
				"CASE WHEN EMRInfoT.TableRowsAsFields = 1 THEN EMRGraphLineDetailsT.RowID \r\n"
				"ELSE EMRGraphLineDetailsT.ColumnID END AS ColumnID, \r\n"
				"EMRGraphLineDetailsT.DateID, EMRInfoMasterT.ID AS EMRInfoMasterID, \r\n"
				"EMRInfoT.Name, EMRInfoT.TableRowsAsFields AS IsFlipped, \r\n"
				"CASE WHEN EMRInfoT.DataType = 1 THEN 'Text' ELSE \r\n"
				"CASE WHEN EMRInfoT.DataType = 2 THEN 'Single Select' ELSE \r\n"
				"CASE WHEN EMRInfoT.DataType = 5 THEN 'Slider' ELSE \r\n"
				"CASE WHEN EMRInfoT.DataType = 7 THEN 'Table' \r\n"
				"END END END END AS DataType FROM EMRGraphLineDetailsT \r\n"
				"INNER JOIN EMRInfoMasterT ON EMRGraphLineDetailsT.EMRInfoMasterID = EMRInfoMasterT.ID \r\n"
				"INNER JOIN EMRInfoT ON EMRInfoMasterT.ActiveEmrInfoID = EMRInfoT.ID \r\n"
				"WHERE DataType IN (1, 2, 5, 7) AND EMRGraphLineDetailsT.LineID = %li) Q \r\n", 
				nLineID));

			m_dlpItemList->SetRedraw(VARIANT_FALSE);
			m_dlpItemList->Clear();
			m_dlpItemList->Requery();		
			m_dlpItemList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		}
		m_bLineChanged = false;
		UpdateActiveControls();
	} NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-4-30) PLID 50083 - assign colors to each line
void CEmrGraphSetupDlg::RequeryFinishedEmrgsLineList(short nFlags)
{
	try {
		//we need to set the color of the cell based on its value.
		for(IRowSettingsPtr pRow = m_dlpLineList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
			_variant_t var = pRow->GetValue(llcColor);
			if (var.vt != VT_NULL || var.vt != VT_EMPTY) {
				pRow->PutCellBackColor(llcColor, (long)var);
				pRow->PutCellBackColorSel(llcColor, (long)var);
				pRow->PutCellForeColor(llcColor, (long)var);
				pRow->PutCellForeColorSel(llcColor, (long)var);
			}
		}
		m_dlpLineList->SetRedraw(VARIANT_TRUE);
	} NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-4-30) PLID 50080 - change line color in list
void CEmrGraphSetupDlg::LeftClickEmrgsLineList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			switch (nCol) {
				//if they click on the color cell. launch the color picker.
				case llcColor:
				{
					//launch color picker dialog.
					CColorDialog dlg(pRow->GetValue(llcColor), CC_ANYCOLOR|CC_RGBINIT, this);
					if(dlg.DoModal() == IDOK) {
						long nColor = (long)dlg.m_cc.rgbResult;
						pRow->PutValue(llcColor, nColor);
						pRow->PutCellBackColor(llcColor, nColor);
						pRow->PutCellBackColorSel(llcColor, nColor);
						pRow->PutCellForeColor(llcColor, nColor);
						pRow->PutCellForeColorSel(llcColor, nColor);
						//save to database.
						ExecuteParamSql("UPDATE EMRGraphLinesT SET Color = {INT} WHERE ID = {INT}", 
							nColor, (long)pRow->GetValue(llcID));
					}
					break;
				}
				default:
					break;
			}
		}
		UpdateActiveControls();
	} NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-4-30) PLID 50084 - insert selection into item list
void CEmrGraphSetupDlg::SelChosenEmrgsAddItemList(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pDetailRow(lpRow);
		//check for valid new item
		if (pDetailRow) {
			long nDetailID = AsLong(pDetailRow->GetValue(ailcID));
			IRowSettingsPtr pLineRow(m_dlpLineList->GetCurSel());
			//check we have a line selected
			if (pLineRow) {
				long nLineID = AsLong(pLineRow->GetValue(ailcID));
				//insert the data into the table
				ExecuteParamSql("INSERT INTO EMRGraphLineDetailsT (LineID, EMRInfoMasterID) VALUES ({INT}, {INT})", 
					nLineID, nDetailID);
				//update the table
				m_dlpItemList->SetRedraw(VARIANT_FALSE);
				m_dlpItemList->Requery();
				m_dlpItemList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			}
		}
		UpdateActiveControls();
	} NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-4-30) PLID 50077 - setfocus to current selection
void CEmrGraphSetupDlg::RightClickEmrgsGraphList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if (m_dlpGraphList->GetReadOnly() == VARIANT_FALSE) {
			GetDlgItem(IDC_EMRGS_GRAPH_LIST)->SetFocus();

			if (IRowSettingsPtr(lpRow) != m_dlpGraphList->GetCurSel()) {
				m_dlpGraphList->PutCurSel(IRowSettingsPtr(lpRow));
				m_bGraphChanged = true;
				SelSetEmrgsGraphList(lpRow);
			}
		}
		UpdateActiveControls();
	} NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-4-30) PLID 50083 - setfocus to current selection
void CEmrGraphSetupDlg::RightClickEmrgsLineList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if (m_dlpLineList->GetReadOnly() == VARIANT_FALSE) {
			GetDlgItem(IDC_EMRGS_LINE_LIST)->SetFocus();

			if (IRowSettingsPtr(lpRow) != m_dlpLineList->GetCurSel()) {
				m_dlpLineList->PutCurSel(IRowSettingsPtr(lpRow));
				m_bLineChanged = true;
				SelSetEmrgsLineList(lpRow);
			}
		}
		UpdateActiveControls();
	} NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-4-30) PLID 50084 - setfocus to current selection
void CEmrGraphSetupDlg::RightClickEmrgsItemList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if (m_dlpItemList->GetReadOnly() == VARIANT_FALSE) {
			GetDlgItem(IDC_EMRGS_ITEM_LIST)->SetFocus();
			m_dlpItemList->PutCurSel(IRowSettingsPtr(lpRow));
		}
		UpdateActiveControls();
	} NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-4-30) PLID 49479 - generate context menu based on datalist your in.
void CEmrGraphSetupDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	try {
		CWnd *pGraphWnd = GetDlgItem(IDC_EMRGS_GRAPH_LIST);
		CWnd *pLineWnd = GetDlgItem(IDC_EMRGS_LINE_LIST);
		CWnd *pItemWnd = GetDlgItem(IDC_EMRGS_ITEM_LIST);

		if (pWnd->GetSafeHwnd() == pGraphWnd->GetSafeHwnd() ||
			pWnd->GetSafeHwnd() == pLineWnd->GetSafeHwnd() || 
			pWnd->GetSafeHwnd() == pItemWnd->GetSafeHwnd()) {
			
			CMenu mnu;
			mnu.LoadMenu(IDR_EMR_GRAPH_SETUP_POPUP);
			CMenu *pmnuSub = mnu.GetSubMenu(0);

			if (pmnuSub) {
				if (point.x == -1) {
					CRect rc;
					pWnd->GetWindowRect(&rc);
					GetCursorPos(&point);

					if (!rc.PtInRect(point)) {
						point.x = rc.left + 5;
						point.y = rc.top + 5;
					}
				}

				//if right click was in graph datalist
				if (pWnd->GetSafeHwnd() == pGraphWnd->GetSafeHwnd()) {

					pmnuSub->RemoveMenu(ID_EMRGS_NEWLINE, MF_BYCOMMAND);
					pmnuSub->RemoveMenu(ID_EMRGS_REMOVELINE, MF_BYCOMMAND);
					pmnuSub->RemoveMenu(ID_EMRGS_REMOVEITEM, MF_BYCOMMAND);
					//check to see if we right clicked a row
					if (!m_dlpGraphList->GetCurSel()) {
						pmnuSub->RemoveMenu(ID_EMRGS_INACTIVATEGRAPH, MF_BYCOMMAND);
						pmnuSub->RemoveMenu(ID_EMRGS_REACTIVATEGRAPH, MF_BYCOMMAND);
						pmnuSub->RemoveMenu(ID_EMRGS_REMOVEGRAPH, MF_BYCOMMAND);
					} else {
						//show specific activation option based on current selections active state.
						if (AsBool(m_dlpGraphList->GetCurSel()->GetValue(glcInactive)) == TRUE) {
							pmnuSub->RemoveMenu(ID_EMRGS_INACTIVATEGRAPH, MF_BYCOMMAND);
						} else {
							pmnuSub->RemoveMenu(ID_EMRGS_REACTIVATEGRAPH, MF_BYCOMMAND);
						}
					}
				//if right click was in line datalist
				} else if (pWnd->GetSafeHwnd() == pLineWnd->GetSafeHwnd()) {

					pmnuSub->RemoveMenu(ID_EMRGS_NEWGRAPH, MF_BYCOMMAND);
					pmnuSub->RemoveMenu(ID_EMRGS_INACTIVATEGRAPH, MF_BYCOMMAND);
					pmnuSub->RemoveMenu(ID_EMRGS_REACTIVATEGRAPH, MF_BYCOMMAND);
					pmnuSub->RemoveMenu(ID_EMRGS_REMOVEITEM, MF_BYCOMMAND);
					pmnuSub->RemoveMenu(ID_EMRGS_REMOVEGRAPH, MF_BYCOMMAND);
					//check to see if we right clicked a row
					if (!m_dlpLineList->GetCurSel()) {
						pmnuSub->RemoveMenu(ID_EMRGS_REMOVELINE, MF_BYCOMMAND);
					}
					//do not show if current graph is inactive.
					if (!m_dlpGraphList->GetCurSel() || AsBool(m_dlpGraphList->GetCurSel()->GetValue(glcInactive)) == TRUE) {
						pmnuSub->EnableMenuItem(ID_EMRGS_NEWLINE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					}
				//if right click was in item datalist
				} else if (pWnd->GetSafeHwnd() == pItemWnd->GetSafeHwnd()) {

					pmnuSub->RemoveMenu(ID_EMRGS_NEWGRAPH, MF_BYCOMMAND);
					pmnuSub->RemoveMenu(ID_EMRGS_INACTIVATEGRAPH, MF_BYCOMMAND);
					pmnuSub->RemoveMenu(ID_EMRGS_REACTIVATEGRAPH, MF_BYCOMMAND);
					pmnuSub->RemoveMenu(ID_EMRGS_NEWLINE, MF_BYCOMMAND);
					pmnuSub->RemoveMenu(ID_EMRGS_REMOVELINE, MF_BYCOMMAND);
					pmnuSub->RemoveMenu(ID_EMRGS_REMOVEGRAPH, MF_BYCOMMAND);
					//check to see if we right clicked a row
					if (!m_dlpItemList->GetCurSel()) {
						pmnuSub->RemoveMenu(ID_EMRGS_REMOVEITEM, MF_BYCOMMAND);
					}
				}
				pmnuSub->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
			}
		}
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-4-30) PLID 50084 - removing an item from the item datalist.
void CEmrGraphSetupDlg::OnRemoveItem() 
{
	try {
		IRowSettingsPtr pRow(m_dlpItemList->GetCurSel());
			
		if (pRow) {	//check for valid row
			long nID = AsLong(pRow->GetValue(ilcID));
			
			if (IDNO == MsgBox(MB_YESNO, "Are you sure you want to remove this item?")) 
				return;	//cancel removal if they aren't sure.

			ExecuteParamSql("DELETE FROM EMRGraphLineDetailsT WHERE ID = {INT}", nID);
			m_dlpItemList->RemoveRow(pRow);
			m_dlpItemList->PutCurSel(NULL);
		}
		UpdateActiveControls();
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-4-30) PLID 50083 - remove a line from the list
void CEmrGraphSetupDlg::OnRemoveLine() 
{
	try {
		IRowSettingsPtr pRow(m_dlpLineList->GetCurSel());
			
		if (pRow) {
			long nID = AsLong(pRow->GetValue(llcID));
			
			if (IDNO == MsgBox(MB_YESNO, "Are you sure you want to remove this line?")) 
				return;

			ExecuteParamSql("DELETE FROM EMRGraphLinesT WHERE ID = {INT} ", nID);
			m_dlpLineList->RemoveRow(pRow);
			m_dlpLineList->PutCurSel(NULL);
			m_dlpItemList->Clear();
		}
		UpdateActiveControls();
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-4-30) PLID 50077 - change the current inactive status of the graph.
void CEmrGraphSetupDlg::OnChangeGraphActivity() 
{
	try {
		IRowSettingsPtr pRow(m_dlpGraphList->GetCurSel());

		if (pRow) {
			long nID = AsLong(pRow->GetValue(glcID));
			BOOL bArchived = AsBool(pRow->GetValue(glcInactive));
			
			//flip value to assign new value.
			if (bArchived) {
				bArchived = FALSE;
			} else {
				bArchived = TRUE;
			}

			ExecuteParamSql("UPDATE EMRGraphsT SET Archived = {BIT} WHERE ID = {INT}", (long)bArchived, nID);

			if (bArchived) {
				pRow->PutValue(glcInactive, g_cvarTrue);
				pRow->PutForeColor(GRAPH_INACTIVE_COLOR);
				pRow->PutForeColorSel(GRAPH_INACTIVE_COLOR);
				m_bGraphChanged = true;
				SelSetEmrgsGraphList(pRow);
			} else {
				pRow->PutValue(glcInactive, g_cvarFalse);
				pRow->PutForeColor(GRAPH_ACTIVE_COLOR);
				pRow->PutForeColorSel(GRAPH_ACTIVE_COLOR);
				m_bGraphChanged = true;
				SelSetEmrgsGraphList(pRow);
			}
		}
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-4-30) PLID 50077 - create a new graph and make the name field editable.
void CEmrGraphSetupDlg::OnNewGraph() 
{
	try {
		IRowSettingsPtr pRow = m_dlpGraphList->GetNewRow();

		if (pRow) {
			pRow->PutValue(glcID, -1);
			pRow->PutValue(glcName, _bstr_t(""));
			pRow->PutValue(glcInactive, g_cvarFalse);

			m_dlpGraphList->AddRowBefore(pRow, m_dlpGraphList->GetFirstRow());
			m_dlpLineList->Clear();
			m_dlpItemList->Clear();
			m_dlpGraphList->PutCurSel(pRow);
			m_dlpGraphList->StartEditing(pRow, glcName);
		}
		UpdateActiveControls();
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-4-30) PLID 50083 - add a line to the list and make the name field editable.
void CEmrGraphSetupDlg::OnNewLine() 
{
	try {
		IRowSettingsPtr pRow(m_dlpGraphList->GetCurSel());

		if (pRow) {
			long nGraphID = AsLong(pRow->GetValue(glcID));
			IRowSettingsPtr pLineRow = m_dlpLineList->GetNewRow();

			if (pLineRow) {
				pLineRow->PutValue(llcID, -1);
				pLineRow->PutValue(llcGraphID, nGraphID);
				pLineRow->PutValue(llcName, _bstr_t(""));
				pLineRow->PutValue(llcColor, 0);
				pLineRow->PutCellBackColor(llcColor, 0);
				pLineRow->PutCellBackColorSel(llcColor, 0);
				pLineRow->PutCellForeColor(llcColor, 0);
				pLineRow->PutCellForeColorSel(llcColor, 0);

				m_dlpLineList->AddRowBefore(pLineRow, m_dlpLineList->GetFirstRow());
				m_dlpLineList->StartEditing(pLineRow, llcName);
			}
		}
		UpdateActiveControls();
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-4-30) PLID 50077 - once requery runs, change the text color of inactive graphs to a gray.
void CEmrGraphSetupDlg::RequeryFinishedEmrgsGraphList(short nFlags)
{
	try {
		for(IRowSettingsPtr pRow = m_dlpGraphList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {

			if (AsBool(pRow->GetValue(glcInactive)) == TRUE) {
				pRow->PutForeColor(GRAPH_INACTIVE_COLOR);
				pRow->PutForeColorSel(GRAPH_INACTIVE_COLOR);
			}
		}
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-4-30) PLID 50084 - we need to get the specific column, row, and date selections for tables.
void CEmrGraphSetupDlg::RequeryFinishedEmrgsItemList(short nFlags)
{
	try {
		//loop through each row and generate their dropdowns.
		for (IRowSettingsPtr pRow = m_dlpItemList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
			
			if (VarString(pRow->GetValue(ilcType)) == "Table") {
				LoadCellValues(pRow, (long)ROW_TYPE, ilcRow, true);
				LoadCellValues(pRow, (long)COLUMN_TYPE, ilcColumn, true);
				LoadDateValues(pRow);
			}
		}
		m_dlpItemList->SetRedraw(VARIANT_TRUE);
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-4-30) PLID 50084 - generate the dropdown list for columns, rows, and dates.
void CEmrGraphSetupDlg::LoadCellValues(LPDISPATCH lpRow, long nType, long nItemListColumn, bool bAddAll)
{
	IRowSettingsPtr pRow(lpRow);
	long nEmrID = VarLong(pRow->GetValue(ilcEMRID));
	CString strIncludeDropdown;
	
	//check for flip and reverse dropdowns if true.
	if (VarBool(pRow->GetValue(ilcFlipped)) != FALSE) {
		if (nType == ROW_TYPE) nType = COLUMN_TYPE;
		else if (nType == COLUMN_TYPE) nType = ROW_TYPE;
	}
	
	//include dropdowns if its not for the date column
	if (nType == COLUMN_TYPE && bAddAll) {
		strIncludeDropdown = (", 4");
	}

	_RecordsetPtr rs = CreateParamRecordset("SELECT EMRDataGroupID, Data, Inactive FROM EMRDataT WHERE EMRInfoID = "
		"(SELECT ActiveEmrInfoID FROM EMRInfoMasterT WHERE ID = {INT}) AND ListType IN ({INT}{CONST_STRING}) ", 
		nEmrID, nType, strIncludeDropdown);

	IFormatSettingsPtr pfs(__uuidof(NXDATALIST2Lib::FormatSettings));
	pfs->PutDataType(VT_BSTR);
	pfs->PutFieldType(cftComboSimple);
	pfs->PutConnection(_variant_t((LPDISPATCH)NULL));

	CString strComboSource = ";;1;;";

	//add the all selection for rows and columns
	if (bAddAll) strComboSource += "0;{All};1;";

	//loop through until all choices are collected.
	while (!rs->eof) {

		CString strTemp;
		BOOL bInactive = AdoFldBool(rs, "Inactive");
		strTemp.Format("%li;%s;%li;", AdoFldLong(rs, "EmrDataGroupID"), AdoFldString(rs, "Data", ""), bInactive ? 0 : 1);
		strComboSource += strTemp;

		rs->MoveNext();
	}

	pfs->PutEditable(VARIANT_TRUE);
	pfs->PutComboSource(_bstr_t(strComboSource));
	pRow->PutRefCellFormatOverride((short)nItemListColumn, pfs);
}
//(a.wilson 2012-4-30) PLID 50084 - do some special handling for loading the date dropdown.
void CEmrGraphSetupDlg::LoadDateValues(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		long nListType;
		long nEmrID = VarLong(pRow->GetValue(ilcEMRID));
		//if flipped, we need to get other type.
		if (AsLong(pRow->GetValue(ilcColumn)) == SELECT_ALL) {
			nListType = (long)ROW_TYPE;
		} else if (AsLong(pRow->GetValue(ilcRow)) == SELECT_ALL) {
			nListType = (long)COLUMN_TYPE;
		} else {
			return;
		}

		LoadCellValues(lpRow, nListType, ilcDate, false);

	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-4-30) PLID 50084 - save any changes to the dropdown selections.
void CEmrGraphSetupDlg::EditingFinishedEmrgsItemList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		IRowSettingsPtr pRow(lpRow);

		if (pRow && bCommit) {
			
			switch (nCol) {
				case ilcRow:
				case ilcColumn:
					{
						long nID = VarLong(pRow->GetValue(ilcID));
						long nNewValue = AsLong(varNewValue);
						long nOldValue = AsLong(varOldValue);
						BOOL bFlipped = VarBool(pRow->GetValue(ilcFlipped));
						CString strColumn;

						if (bFlipped) {	//if its flipped we need to mask the row as column and column as row.
							if (nCol == ilcRow) nCol = ilcColumn;
							else if (nCol == ilcColumn) nCol = ilcRow;
						}
						//check to make sure that they are both not select all
						if (nCol == ilcRow) {
							if (((AsLong(pRow->GetValue(ilcColumn)) == SELECT_ALL && !bFlipped) 
								|| (AsLong(pRow->GetValue(ilcRow)) == SELECT_ALL && bFlipped)) 
								&& nNewValue == SELECT_ALL) {
								MsgBox("You cannot set both row and column values to {All}.");
								pRow->PutValue(nCol, varOldValue);
								return;
							}
							strColumn = "RowID";
						//check to make sure that they are both not select all
						} else if (nCol == ilcColumn) {
							if (((AsLong(pRow->GetValue(ilcRow)) == SELECT_ALL && !bFlipped) 
								|| (AsLong(pRow->GetValue(ilcColumn)) == SELECT_ALL && bFlipped)) 
								&& nNewValue == SELECT_ALL) {
								MsgBox("You cannot set both row and column values to {All}.");
								pRow->PutValue(nCol, varOldValue);
								return;
							}
							strColumn = "ColumnID";
						}
						//save the change
						if (nNewValue == SELECT_ALL) {
							ExecuteParamSql("UPDATE EMRGraphLineDetailsT SET {CONST_STRING} = NULL WHERE ID = {INT}", 
								strColumn, nID);
							LoadDateValues(lpRow);
						} else {
							ExecuteParamSql("UPDATE EMRGraphLineDetailsT SET {CONST_STRING} = {INT} WHERE ID = {INT}", 
								strColumn, nNewValue, nID);
						}
						if (nOldValue == SELECT_ALL && nNewValue != SELECT_ALL) {
							ExecuteParamSql("UPDATE EMRGraphLineDetailsT SET DateID = NULL WHERE ID = {INT}", nID);
							pRow->PutValue(ilcDate, g_cvarNull);	
						}
						break;
					}
				case ilcDate:
					{
						long nID = VarLong(pRow->GetValue(ilcID));
						long nNewValue = AsLong(varNewValue);
						long nOldValue = AsLong(varOldValue);
						
						if (nNewValue != nOldValue) {
							if (nNewValue == 0) {
								ExecuteParamSql("UPDATE EMRGraphLineDetailsT SET DateID = NULL WHERE ID = {INT}", 
									nID);
							} else {
								ExecuteParamSql("UPDATE EMRGraphLineDetailsT SET DateID = {INT} WHERE ID = {INT}", 
									nNewValue, nID);
							}
						}
						break;
					}
			}
		}
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-4-30) PLID 50084 - only allow for tables to have the drop down capability
void CEmrGraphSetupDlg::EditingStartingEmrgsItemList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			if (pRow) {
				CString strDataType = VarString(pRow->GetValue(ilcType), "");
		
				switch (nCol) {				
					case ilcRow:
					case ilcColumn:
					case ilcDate:
						{
							if (strDataType	!= "Table") { //only allow if its a table
								*pbContinue = FALSE;
								return;
							}
							if (nCol == ilcDate) { //ensure that one of the fields is set to select all.
								bool bValCol = (AsLong(pRow->GetValue(ilcColumn)) == SELECT_ALL);
								bool bValRow = (AsLong(pRow->GetValue(ilcRow)) == SELECT_ALL);
								if ((!bValCol) && (!bValRow)) {
									*pbContinue = FALSE;								
								}							
							}
						}
					break;
				}
			}

	}NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-4-30) PLID 50082 - on close make sure that no active graph setups are invalid.
void CEmrGraphSetupDlg::OnClose()
{
	try {
		//check to ensure that everything is filled in correctly before allowing the user to close the dialog.
		_RecordsetPtr prs = CreateParamRecordset(
			"--Check for Incomplete items.\r\n"
			"SELECT EMRGraphsT.Name AS GraphName, EMRGraphLinesT.Name AS LineName FROM EMRGraphsT INNER JOIN \r\n"
			"EMRGraphLinesT ON EMRGraphsT.ID = EMRGraphLinesT.GraphID INNER JOIN \r\n"
			"EMRGraphLineDetailsT ON EMRGraphLinesT.ID = EMRGraphLineDetailsT.LineID \r\n"
			"INNER JOIN (SELECT EMRInfoMasterT.ID, EMRInfoT.DataType FROM EMRInfoMasterT \r\n"
			"INNER JOIN EMRInfoT ON EMRInfoMasterT.ActiveEMRInfoID = EMRInfoT.ID) DataTypeQ \r\n"
			"ON EMRGraphLineDetailsT.EMRInfoMasterID = DataTypeQ.ID \r\n"
			"WHERE ((RowID IS NULL AND ColumnID IS NULL) \r\n"
			"OR (DateID IS NULL AND (RowID IS NULL OR ColumnID IS NULL))) AND Archived = 0 AND DataType = 7; \r\n"
			"\r\n"
			"--Check for duplicate graph names.\r\n"
			"SELECT Distinct A.Name FROM EMRGraphsT A INNER JOIN EMRGraphsT B ON A.Name = B.Name AND A.ID <> B.ID \r\n"
			"WHERE A.Archived = 0 AND B.Archived = 0; \r\n"
			"\r\n"
			"--Check for duplicate line names within graphs.\r\n"
			"SELECT EMRGraphsT.Name AS GraphName, DuplicateNameQ.LineName FROM "
			"(SELECT Distinct A.GraphID, A.Name AS LineName FROM EMRGraphLinesT A "
			"INNER JOIN EMRGraphLinesT B ON A.GraphID = B.GraphID AND A.ID <> B.ID AND A.Name = B.Name) DuplicateNameQ "
			"INNER JOIN EMRGraphsT ON EMRGraphsT.ID = DuplicateNameQ.GraphID "
			"WHERE EMRGraphsT.Archived = 0; \r\n");

		//check for incomplete details.
		CString strDetailConflicts;
		for (; !prs->eof; prs->MoveNext()) {
			strDetailConflicts += FormatString("Graph: %s - Line: %s\r\n", AdoFldString(prs, "GraphName"), 
				AdoFldString(prs, "LineName"));
		}
		if (!strDetailConflicts.IsEmpty()) {
			MsgBox("The following graphs and lines have incomplete items assigned to "
				"them with an unassigned row, column, or date and need to be fixed:\r\n\r\n"
				"%s", strDetailConflicts);
			return;
		}
		prs = prs->NextRecordset(NULL);
		
		//check for duplicate graph names.
		CString strConflictMessage, strGraphNameConflicts;
		for (; !prs->eof; prs->MoveNext()) {
			strGraphNameConflicts += FormatString("%s\r\n", AdoFldString(prs, "Name"));
		}
		if (!strGraphNameConflicts.IsEmpty()) {
			strConflictMessage.Format("The following active graph names are duplicates:\r\n\r\n"
				"%s\r\n", strGraphNameConflicts);
		}
		prs = prs->NextRecordset(NULL);

		//check for duplicate line names under the same graph.
		CString strLineNameConflicts;
		for (; !prs->eof; prs->MoveNext()) {
			strLineNameConflicts += FormatString("Graph: %s - Duplicate: %s\r\n", AdoFldString(prs, "GraphName"), 
				AdoFldString(prs, "LineName"));
		}
		if (!strLineNameConflicts.IsEmpty()) {
			strConflictMessage += FormatString("The following active graphs have duplicate line names:\r\n\r\n"
				"%s", strLineNameConflicts);
		}
		if (!strConflictMessage.IsEmpty()) {
			if (IDNO == MsgBox(MB_YESNO, "%s\r\n\r\nAre you sure you want to continue?", strConflictMessage))
				return;
		}

	} NxCatchAll(__FUNCTION__);

	CNxDialog::OnCancel();
}
//(a.wilson 2012-4-30) PLID 50077 - remove a graph from the list.
void CEmrGraphSetupDlg::OnRemoveGraph()
{
	try {
		IRowSettingsPtr pRow(m_dlpGraphList->GetCurSel());
			
		if (pRow) {
			long nID = AsLong(pRow->GetValue(glcID));
			
			if (IDNO == MsgBox(MB_YESNO, "Are you sure you want to remove this graph?\r\n"
				"You will lose all setup for this graph.")) 
				return;

			//lines and items associated with this graph will be deleted by cascade deletion key.
			ExecuteParamSql("DELETE FROM EMRGraphsT WHERE ID = {INT} ", nID);
			m_dlpGraphList->RemoveRow(pRow);
			m_dlpGraphList->PutCurSel(NULL);
			m_dlpLineList->Clear();
			m_dlpItemList->Clear();
		}
		UpdateActiveControls();
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-4-30) PLID 50083 - when the name is change we need to save the changes to the database.
void CEmrGraphSetupDlg::EditingFinishedEmrgsLineList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			switch (nCol) {
				case llcName:
					{	
						CString strNewValue = VarString(varNewValue);
						strNewValue.Trim();
						if (VarLong(pRow->GetValue(llcID)) == -1) {
							if (!bCommit) {
								m_dlpLineList->RemoveRow(pRow);
							} else if (strNewValue.IsEmpty()) {
								MsgBox("The name you entered for the line is not a valid entry.");
								m_dlpLineList->RemoveRow(pRow);
							} else {
								ExecuteParamSql("INSERT INTO EMRGraphLinesT (GraphID, Name, Color) "
									"VALUES ({INT}, {STRING}, {INT})", 
									VarLong(pRow->GetValue(llcGraphID)), VarString(varNewValue), 
									VarLong(pRow->GetValue(llcColor)));
								m_dlpLineList->Clear();
								m_dlpLineList->Requery();
								UpdateActiveControls();
							}
						} else {
							if (!bCommit) {
								pRow->PutValue(llcName, varOldValue);
							} else if (strNewValue.IsEmpty()) {
								MsgBox("The name you entered for the line is not a valid entry.");
								pRow->PutValue(llcName, varOldValue);
							} else {
								ExecuteParamSql("UPDATE EMRGraphLinesT SET Name = {STRING} WHERE ID = {INT}", 
									VarString(varNewValue), AsLong(pRow->GetValue(llcID)));
								m_dlpLineList->Sort();
							}
						}
					}
					break;
				default:
					break;
			}
		}
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-4-30) PLID 50077 - after completing a name change we need to save it to the database.
void CEmrGraphSetupDlg::EditingFinishedEmrgsGraphList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			switch (nCol) {
				case glcName:
					{
						CString strNewValue = VarString(varNewValue);
						strNewValue.Trim();
						if (VarLong(pRow->GetValue(glcID)) == -1) {
							if (!bCommit) {
								m_dlpGraphList->RemoveRow(pRow);
							} else if (strNewValue.IsEmpty()) {
								MsgBox("The name you entered for the graph is not a valid entry.");
								m_dlpGraphList->RemoveRow(pRow);
							} else {
								ExecuteParamSql("INSERT INTO EMRGraphsT (Name, Archived) "
									"VALUES ({STRING}, {INT})", VarString(varNewValue), VarBool(pRow->GetValue(glcInactive)));
								m_dlpGraphList->Clear();
								m_dlpGraphList->Requery();
								UpdateActiveControls();
							}
						} else {
							if (!bCommit) {
								pRow->PutValue(glcName, varOldValue);
							} else if (strNewValue.IsEmpty()) {
								MsgBox("The name you entered for the graph is not a valid entry.");
								pRow->PutValue(glcName, varOldValue);
							} else {
								ExecuteParamSql("UPDATE EMRGraphsT SET Name = {STRING} WHERE ID = {INT}", 
									VarString(varNewValue), AsLong(pRow->GetValue(glcID)));
								m_dlpGraphList->Sort();
							}
						}
					}
					break;
				default:
					break;
			}
		}
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-5-7) PLID 49479 - need to update the buttons accordingly.
void CEmrGraphSetupDlg::SelSetEmrgsItemList(LPDISPATCH lpSel)
{
	try {
		UpdateActiveControls();
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-5-7) PLID 49479 - update the buttons in the dialog based on the current datalist selections.
void CEmrGraphSetupDlg::UpdateActiveControls()
{
	try {
		//check graph selection
		if (m_dlpGraphList->GetCurSel()) {
			m_btnRemoveGraph.EnableWindow(TRUE);
			if (AsBool(m_dlpGraphList->GetCurSel()->GetValue(glcInactive)) == TRUE) {
				m_btnNewLine.EnableWindow(FALSE);
				m_dlpLineList->Enabled = VARIANT_FALSE;
			} else {
				m_btnNewLine.EnableWindow(TRUE);
				m_dlpLineList->Enabled = VARIANT_TRUE;
			}
		} else {
			m_btnRemoveGraph.EnableWindow(FALSE);
			m_btnNewLine.EnableWindow(FALSE);
			m_dlpLineList->Enabled = VARIANT_FALSE;
		}

		//check line selection
		if (m_dlpLineList->GetCurSel()) {
			m_btnRemoveLine.EnableWindow(TRUE);
			GetDlgItem(IDC_EMRGS_ADD_ITEM_LIST)->EnableWindow(TRUE);
			m_dlpItemList->Enabled = VARIANT_TRUE;
		} else {
			m_btnRemoveLine.EnableWindow(FALSE);
			GetDlgItem(IDC_EMRGS_ADD_ITEM_LIST)->EnableWindow(FALSE);
			m_dlpItemList->Enabled = VARIANT_FALSE;
		}

		//check item selection
		if (m_dlpItemList->GetCurSel()) {
			m_btnRemoveItem.EnableWindow(TRUE);
		} else {
			m_btnRemoveItem.EnableWindow(FALSE);
		}

	} NxCatchAll(__FUNCTION__);
}
// (a.wilson 6/25/2012) PLID 49479 - ensure that the lists dont clear when the same selection is made
void CEmrGraphSetupDlg::SelChangingEmrgsGraphList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		m_bGraphChanged = true;
	} NxCatchAll(__FUNCTION__);
}
// (a.wilson 6/25/2012) PLID 49479 - ensure that the lists dont clear when the same selection is made
void CEmrGraphSetupDlg::SelChangingEmrgsLineList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		m_bLineChanged = true;
	} NxCatchAll(__FUNCTION__);
}
// (a.wilson 6/25/2012) PLID 49479 - ensure you cannot modify the name of an inactive graph.
void CEmrGraphSetupDlg::EditingStartingEmrgsGraphList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {
		IRowSettingsPtr pRow(lpRow);

		if (pRow && VarBool(pRow->GetValue(glcInactive), g_cvarFalse) == TRUE)
			*pbContinue = FALSE;
	} NxCatchAll(__FUNCTION__);
}
