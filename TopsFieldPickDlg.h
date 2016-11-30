#pragma once
#include "TopsSearchDlg.h"
#include "Financialrc.h"

// CTopsFieldPickDlg dialog
// (j.gruber 2009-11-20 17:09) - PLID 36139 - created for

class CTopsFieldPickDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CTopsFieldPickDlg)

public:
	CTopsFieldPickDlg(CWnd* pParent);   // standard constructor
	virtual ~CTopsFieldPickDlg();
	CArray<TOPLevel*, TOPLevel*> *m_paryTopsList;
	CMap<CString, LPCTSTR, CString, LPCTSTR> *m_pmapReturnValues;
	CString m_strPatientName;
	COleDateTime m_dtProcDate;
	void LoadList();

// Dialog Data
	enum { IDD = IDD_TOPS_FIELD_PICK_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	void LoadValuesList(LPDISPATCH lpRow, long nRowItemID);
	CNxStatic m_stDescription;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedTopsFieldList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
