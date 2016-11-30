#pragma once


// CWarrantyDlg dialog
//(c.copits 2010-10-28) PLID 38598 - Warranty tracking system

class CWarrantyDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CWarrantyDlg)

public:
	CWarrantyDlg(CWnd* pParent);   // standard constructor
	virtual ~CWarrantyDlg();
	
	void SetFromClause(CString strFrom);
	void SetWhereClause(CString strWhere);

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_WARRANTY };
	CNxColor	m_WarrantyDlgBkg;
	OLE_COLOR	m_nWarrantyBkgColor;

	// Set the dialog's background color
	void SetWarrantyBkgColor(OLE_COLOR nNewColor);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CString m_strFrom;
	CString m_strWhere;
	
	NXDATALIST2Lib::_DNxDataListPtr m_pWarrantyList;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBtnClose();
};
