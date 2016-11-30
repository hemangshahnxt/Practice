#if !defined(AFX_INVMULTISERIALPICKERDLG_H__69827B47_8EF7_4E3A_AA11_09CF21482AD6__INCLUDED_)
#define AFX_INVMULTISERIALPICKERDLG_H__69827B47_8EF7_4E3A_AA11_09CF21482AD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvMultiSerialPickerDlg.h : header file
//
// (c.haag 2008-06-18 10:16) - PLID 28339 - Initial implementation
//
#include "inventoryrc.h"

/////////////////////////////////////////////////////////////////////////////
// CInvMultiSerialPickerDlg dialog

class CInvMultiSerialPickerDlg : public CNxDialog
{
// Construction
public:
	CInvMultiSerialPickerDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInvMultiSerialPickerDlg)
	enum { IDD = IDD_INVMULTISERIALPICKER };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxStatic	m_nxstaticSerial;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvMultiSerialPickerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	// The recordset from Barcode_CheckExistenceOfSerialNumber
	ADODB::_RecordsetPtr m_prs;
	// The list of qualifying ID's in m_prs
	CArray<long,long> m_anIDs;
	// The serial number
	CString m_strSerialNum;

public:
	// The ID of the serialized item the user selected
	long m_nResultID;

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_dlList;

	// Generated message map functions
	//{{AFX_MSG(CInvMultiSerialPickerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClickCellListMultiserialpicker(LPDISPATCH lpRow, short nColIndex);
	virtual void OnOK();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVMULTISERIALPICKERDLG_H__69827B47_8EF7_4E3A_AA11_09CF21482AD6__INCLUDED_)
