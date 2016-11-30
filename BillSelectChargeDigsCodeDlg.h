#pragma once
#include "BillingRc.h"
#include "DiagCodeInfoFwd.h"

// (s.dhole 2011-05-16 14:44) - PLID 33666 New Dlg to assign Digs to Line Item
// CBillSelectChargeDigsCodeDlg dialog


class CBillSelectChargeDigsCodeDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CBillSelectChargeDigsCodeDlg)

public:	
	CBillSelectChargeDigsCodeDlg(CWnd* pParent);   
	virtual ~CBillSelectChargeDigsCodeDlg();
	MFCArray<DiagCodeInfoPtr> m_arypAllDiagCodes;
	
	// (j.gruber 2014-02-25 12:56) - PLID 61028 - new structure
	CChargeWhichCodesMapPtr m_mapSelectedCodes;

	// (j.gruber 2014-02-25 13:36) - PLID 61028
	DiagCodeInfoPtr CBillSelectChargeDigsCodeDlg::FindDiagCodeByPair(CChargeWhichCodePair pair);

	CString m_strSelectedDiagCodesExt;
	CString m_strCaption;
	BOOL m_bReadOnly;
// Dialog Data
	enum { IDD = IDD_BILL_SELECT_DIGs_CODE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog() ;
	//int SplitString(const CString& strInput, const CString& strDelimiter , OUT CStringArray& arrArrayStr) ;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	

	void ClearDiagCodeArray();
	void EnsureButtons();
	NXDATALIST2Lib::_DNxDataListPtr m_DiagCodeListSelected;
	
	DECLARE_MESSAGE_MAP()


	afx_msg void OnOk();
	afx_msg void OnCancel();


public:
	
	DECLARE_EVENTSINK_MAP()
	void EditingFinishingDiagCodeSelectedList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};
