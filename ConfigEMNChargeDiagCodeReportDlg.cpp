// ConfigEMNChargeDiagCodeReportDlg.cpp : implementation file
//

// (j.gruber 2010-03-10 10:34) - PLID 37660 - created for
#include "stdafx.h"
#include "Practice.h"
#include "ConfigEMNChargeDiagCodeReportDlg.h"
#include "ReportsRc.h"

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_REMOVE_CAT_EMN_REPORT 44561

enum CatListColumns {
	catID = 0,
	catName ,
};

enum SelCatList {
	clcID = 0,
	clcOrder,
	clcName,
};
// CConfigEMNChargeDiagCodeReportDlg dialog

IMPLEMENT_DYNAMIC(CConfigEMNChargeDiagCodeReportDlg, CNxDialog)

CConfigEMNChargeDiagCodeReportDlg::CConfigEMNChargeDiagCodeReportDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigEMNChargeDiagCodeReportDlg::IDD, pParent)
{

}

CConfigEMNChargeDiagCodeReportDlg::~CConfigEMNChargeDiagCodeReportDlg()
{
}

void CConfigEMNChargeDiagCodeReportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_CONFIG_EMN_REPORT_DOWN, m_btnDown);
	DDX_Control(pDX, IDC_CONFIG_EMN_REPORT_UP, m_btnUp);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	
}


BEGIN_MESSAGE_MAP(CConfigEMNChargeDiagCodeReportDlg, CNxDialog)
	ON_BN_CLICKED(IDC_CONFIG_EMN_REPORT_UP, &CConfigEMNChargeDiagCodeReportDlg::OnBnClickedConfigEmnReportUp)
	ON_BN_CLICKED(IDC_CONFIG_EMN_REPORT_DOWN, &CConfigEMNChargeDiagCodeReportDlg::OnBnClickedConfigEmnReportDown)
	ON_BN_CLICKED(IDOK, &CConfigEMNChargeDiagCodeReportDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CConfigEMNChargeDiagCodeReportDlg::OnBnClickedCancel)
	ON_COMMAND(ID_REMOVE_CAT_EMN_REPORT, OnRemoveCategory)
END_MESSAGE_MAP()


// CConfigEMNChargeDiagCodeReportDlg message handlers

void CConfigEMNChargeDiagCodeReportDlg::OnBnClickedConfigEmnReportUp()
{
	try {
		//we just need to switch the two rows
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelCatList->CurSel;

		if (pRow) {
			if (pRow != m_pSelCatList->GetFirstRow()) {
				long nOrigOrderID = VarLong(pRow->GetValue(clcOrder));
				long nNewOrderID = nOrigOrderID - 1;
					
				pRow->PutValue(clcOrder, nNewOrderID);

				pRow = pRow->GetPreviousRow();

				if (pRow) {
					pRow->PutValue(clcOrder, nOrigOrderID);
				}
				m_pSelCatList->Sort();

				CheckButtonStatus();
				
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CConfigEMNChargeDiagCodeReportDlg::CheckButtonStatus() {

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelCatList->CurSel;

		if (pRow) {
			if (pRow == m_pSelCatList->GetFirstRow()) {
				m_btnUp.EnableWindow(FALSE);
			}
			else {
				m_btnUp.EnableWindow(TRUE);
			}

			if (pRow == m_pSelCatList->GetLastRow()) {
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
	}NxCatchAll(__FUNCTION__);
}


void CConfigEMNChargeDiagCodeReportDlg::OnBnClickedConfigEmnReportDown()
{
	try {
		//we just need to switch the two rows
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelCatList->CurSel;

		if (pRow) {
			if (pRow != m_pSelCatList->GetLastRow()) {
				long nOrigOrderID = VarLong(pRow->GetValue(clcOrder));
				long nNewOrderID = nOrigOrderID + 1;

				pRow->PutValue(clcOrder, nNewOrderID);

				pRow = pRow->GetNextRow();
				
				if (pRow) {
					pRow->PutValue(clcOrder, nOrigOrderID);
				}

				m_pSelCatList->Sort();

				CheckButtonStatus();				
			}
		}

	}NxCatchAll(__FUNCTION__);	
}

void CConfigEMNChargeDiagCodeReportDlg::OnBnClickedOk()
{
	try {

		//loop through and update the data
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelCatList->GetFirstRow();
		CString strIDs;
		
		while (pRow) {
			strIDs += AsString(VarLong(pRow->GetValue(clcID))) + ", ";

			pRow = pRow->GetNextRow();
		}

		//trim the last , 
		strIDs.TrimRight(", ");

		SetRemotePropertyMemo("EMNReportImageCategories", strIDs, 0, "<None>");

		CNxDialog::OnOK();
	
	}NxCatchAll(__FUNCTION__);
}

void CConfigEMNChargeDiagCodeReportDlg::OnBnClickedCancel()
{
	try {
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CConfigEMNChargeDiagCodeReportDlg, CNxDialog)
	ON_EVENT(CConfigEMNChargeDiagCodeReportDlg, IDC_CONFIG_EMN_REPORT_CATEGORY_LIST, 16, CConfigEMNChargeDiagCodeReportDlg::SelChosenConfigEmnReportCategoryList, VTS_DISPATCH)
	ON_EVENT(CConfigEMNChargeDiagCodeReportDlg, IDC_CONFIG_EMN_REPORT_SELECT_LIST, 7, CConfigEMNChargeDiagCodeReportDlg::RButtonUpConfigEmnReportSelectList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CConfigEMNChargeDiagCodeReportDlg, IDC_CONFIG_EMN_REPORT_SELECT_LIST, 2, CConfigEMNChargeDiagCodeReportDlg::SelChangedConfigEmnReportSelectList, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

void CConfigEMNChargeDiagCodeReportDlg::SelChosenConfigEmnReportCategoryList(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pNewRow(lpRow);

		if (pNewRow) {
			
			long nNewID = VarLong(pNewRow->GetValue(catID));

			//loop through and make sure its not in the list already
			BOOL bFound = FALSE;
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelCatList->GetFirstRow();	
			
			while (pRow && (!bFound)) {
				long nID = VarLong(pRow->GetValue(clcID));

				if (nID == nNewID) {
					bFound = TRUE;
				}

				pRow = pRow->GetNextRow();
			}

			if (!bFound) {

				//add it
				pRow = m_pSelCatList->GetNewRow();
				if (pRow) {

					pRow->PutValue(clcID, pNewRow->GetValue(catID));
					pRow->PutValue(clcName, pNewRow->GetValue(catName));
					pRow->PutValue(clcOrder, (long)(m_pSelCatList->GetRowCount() + 1));

					m_pSelCatList->AddRowAtEnd(pRow, NULL);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}


BOOL CConfigEMNChargeDiagCodeReportDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_pCatList = BindNxDataList2Ctrl(IDC_CONFIG_EMN_REPORT_CATEGORY_LIST, true);
		m_pSelCatList = BindNxDataList2Ctrl(IDC_CONFIG_EMN_REPORT_SELECT_LIST, false);

		m_btnUp.AutoSet(NXB_UP);
		m_btnDown.AutoSet(NXB_DOWN);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnOK.AutoSet(NXB_CLOSE);


		CString strIDs = GetRemotePropertyMemo("EMNReportImageCategories", "", 0, "<None>");
		long nCount = 1;

		if (!strIDs.IsEmpty()) {

			//see if we can find a comma
			long nResult = strIDs.Find(",");
			CString strTemp = strIDs;
			CString strSql;

			while (nResult != -1) {
				CString strID = strTemp.Left(nResult);
				strSql += FormatString("SELECT ID, Description, %li as Ordered FROM NoteCatsF WHERE ID = %s UNION ALL ", nCount, strID);

				strTemp = strTemp.Right(strTemp.GetLength() - (nResult + 1));
				strTemp.TrimLeft();
				strTemp.TrimRight();
				nResult = strTemp.Find(",");

				nCount++;
			}

			//now do the last one
			strSql += FormatString("SELECT ID, Description, %li as Ordered FROM NoteCatsF WHERE ID = %s ", nCount, strTemp);

			m_pSelCatList->FromClause = _bstr_t("(" + strSql + ")Q");
			m_pSelCatList->Requery();	

		}
			
		

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}
void CConfigEMNChargeDiagCodeReportDlg::RButtonUpConfigEmnReportSelectList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			m_pSelCatList->CurSel = pRow;

			CMenu menPopup;
			menPopup.m_hMenu = CreatePopupMenu();
			menPopup.InsertMenu(1, MF_BYPOSITION, ID_REMOVE_CAT_EMN_REPORT, "Remove");

			CPoint pt(x,y);
			CWnd* pWnd = GetDlgItem(IDC_CONFIG_EMN_REPORT_SELECT_LIST);

			if (pWnd != NULL) {
				pWnd->ClientToScreen(&pt);
				menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
			}
			else {
				ThrowNxException("Error in CConfigEMNChargeDiagCodeReportDlg::RButtonUpConfigEmnReportSelectList: An error ocurred while creating menu");
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CConfigEMNChargeDiagCodeReportDlg::OnRemoveCategory() {

	try {
	
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelCatList->CurSel;

		if (pRow) {

			long nOrder = VarLong(pRow->GetValue(clcOrder));			

			//loop through the rows under this one and 
			NXDATALIST2Lib::IRowSettingsPtr pRowNext;
			pRowNext = pRow->GetNextRow();
			
			while (pRowNext) {
				
				pRowNext->PutValue(clcOrder, nOrder);

				nOrder++;

				pRowNext = pRowNext->GetNextRow();
			}

			m_pSelCatList->RemoveRow(pRow);

			CheckButtonStatus();
		}

	}NxCatchAll(__FUNCTION__);

}
void CConfigEMNChargeDiagCodeReportDlg::SelChangedConfigEmnReportSelectList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		CheckButtonStatus();
	}NxCatchAll(__FUNCTION__);
}
