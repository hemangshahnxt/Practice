#pragma once

#include "AdministratorRc.h"
#include "soaputils.h"

// (j.gruber 2009-10-27 13:56) - PLID 35632 - created for
// CMultiFeeImportFieldSelectionDlg dialog

class CMultiFeeImportFieldSelectionDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CMultiFeeImportFieldSelectionDlg)

public:
	CMultiFeeImportFieldSelectionDlg(CString strFileName, BOOL bUseAllowable, CString strCodeField, CString strFeeField, CString strAllowableField, CWnd* pParent);   // standard constructor
	virtual ~CMultiFeeImportFieldSelectionDlg();

// Dialog Data
	enum { IDD = IDD_MULTIFEE_CHOOSE_COLUMNS };

	CString m_strCodeField;
	CString m_strFeeField;
	CString m_strAllowableField;
	BOOL m_bUseAllowable;
	CString m_strFileName;	
	CString m_strParentNode;

protected:

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxStatic m_nxstInstructions;
	
	NXDATALIST2Lib::_DNxDataListPtr m_pCodeField;
	NXDATALIST2Lib::_DNxDataListPtr m_pFeeField;
	NXDATALIST2Lib::_DNxDataListPtr m_pAllowableField;	

	void ProcessFieldList();	
	void ParseChildren(MSXML2::IXMLDOMNodePtr pRootNode, NXDATALIST2Lib::IRowSettingsPtr pRow);
	NXDATALIST2Lib::IRowSettingsPtr EnsureDataListRow(CString strRowName, NXDATALIST2Lib::IRowSettingsPtr pParentRow);
	void CopyList(NXDATALIST2Lib::_DNxDataListPtr pList1, NXDATALIST2Lib::_DNxDataListPtr pList2);
	BOOL SaveFieldPath(NXDATALIST2Lib::_DNxDataListPtr pList, CString &aryList, CString strDescriptor);
	CString GetPathParent(CString strCodePath, CString strFeePath, CString strAllowablePath);
	void FinalizeLoading(NXDATALIST2Lib::_DNxDataListPtr pList);
	void InitLoad(NXDATALIST2Lib::_DNxDataListPtr pList, long nID);
	void LoadDefaults(NXDATALIST2Lib::_DNxDataListPtr pList, CString strFieldPath);
	void FinalizeLoading(NXDATALIST2Lib::_DNxDataListPtr pList, CString strFieldPath);
	

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void SelChangingMultifeeCodeField(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingMultifeeFeeField(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingMultifeeAllowableField(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnBnClickedOk();
	afx_msg void OnTimer(UINT nIDEvent);
};
