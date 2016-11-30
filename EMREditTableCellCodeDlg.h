#pragma once

#include "EmrItemEntryDlg.h"
// CEMREditTableCellCodeDlg dialog
// (j.gruber 2013-09-30 10:58) - PLID 58816 - created for



class CEMREditTableCellCodeDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEMREditTableCellCodeDlg)

public:
	CEMREditTableCellCodeDlg(CEmrInfoDataElementArray &aryRows, CEmrInfoDataElementArray &aryColumns,
		CEmrItemEntryDlg* pParent, EmrInfoSubType eDataSubType, CEMRTableCellCodes *pTableCodes);   // standard constructor
	virtual ~CEMREditTableCellCodeDlg();

	BOOL m_bTableRowsAsFields;

// Dialog Data
	enum { IDD = IDD_EDIT_TABLE_CELL_CODE_DLG };

protected:

	CEMRTableCellCodes *m_pmapCodes;

	CEmrInfoDataElementArray m_aryRows;
	CEmrInfoDataElementArray m_aryColumns;

	long m_nEditingColumnSortOrder;
	long m_nEditingRowSortOrder;
	
	CEmrItemEntryDlg *m_pEmrItemEntryDlg;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlTable;
	NXDATALIST2Lib::_DNxDataListPtr m_pCodeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSelectedList;

	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;

	// (j.jones 2011-07-08 13:18) - PLID 43032 - tracks if this table is a Current Medications or Allergies table
	EmrInfoSubType m_DataSubType;

	CString m_strCurrentRowID;
	CString m_strCurrentColID;

	long m_nCurrentRowDataIndex;
	long m_nCurrentColDataIndex;

	NXDATALIST2Lib::IRowSettingsPtr m_pCurrentRow;
	long m_nCurrentColListIndex;	

	CEmrInfoDataElement* GetCurrentData();
	int GetCellColor(CEmrInfoDataElement *pColumnInfo, CEmrInfoDataElement *pRowInfo);
	//CArray<CodeStruct, CodeStruct>* GetCellCodes(long nRowID, long nColID, BOOL bRemove = FALSE);
	//long GetCellCodeCount(long nRowID, long nColID);
	void LoadCodeList(long nRowIndex, long nColIndex);
	void SaveCodeList(CEMRTableCell cell);
	void NavigateCells();	
	void ColorCell(NXDATALIST2Lib::IRowSettingsPtr pRow, long nColIndex, int color);
	BOOL CanClick(long nRowIndex, long nColIndex);
	void EnableControls(BOOL bEnable);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
public:
	/*afx_msg void OnBnClickedAddCode();
	afx_msg void OnBnClickedEditCode();
	afx_msg void OnBnClickedRemoveCode();*/
	afx_msg void OnBnClickedOpenUmls();
	DECLARE_EVENTSINK_MAP()
	void LeftClickTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void SelChangedCodeList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void LButtonDownTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void SelChosenCodeSelectList(LPDISPATCH lpRow);
	void RButtonUpCodeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);	
	afx_msg void OnBnClickedOk();
};
