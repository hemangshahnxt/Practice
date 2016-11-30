//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_EXPORTDUPLICATES_H__9E36492F_F594_4668_A336_022BE1FC5437__INCLUDED_)
#define AFX_EXPORTDUPLICATES_H__9E36492F_F594_4668_A336_022BE1FC5437__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportDuplicates.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportDuplicates dialog

class CExportDuplicates : public CNxDialog
{
// Construction
public:
	CExportDuplicates(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_Duplicate_List;
	
	//can be Mirror, Inform, or Practice
	CString m_Source, m_Dest;

	CString m_strPassword; // Database password

	CString m_name, m_sql, m_path;

	BOOL m_bAllowUpdate;

	_variant_t m_varIDToUpdate;

	BOOL m_bOneMatchingNameLinks;

	ADODB::_ConnectionPtr m_pConn;

	bool FindDuplicates(CString first, CString last, CString middle, CString Source, CString Dest, ADODB::_ConnectionPtr pConn, CString path, BOOL bAllowUpdate = TRUE, CString strPassword = "", BOOL bAssumeOneMatchingNameLinks = FALSE, CString strSSN = "");

// Dialog Data
	//{{AFX_DATA(CExportDuplicates)
	enum { IDD = IDD_EXPORT_DUPLICATES };
	CNxIconButton	m_Stop;
	CNxIconButton	m_Skip;
	CNxIconButton	m_Update;
	CNxIconButton	m_AddNew;
	CNxStatic	m_nxstaticNameLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportDuplicates)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExportDuplicates)
	afx_msg void OnAddNew();
	afx_msg void OnUpdate();
	virtual BOOL OnInitDialog();
	afx_msg void OnSkip();
	afx_msg void OnStop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTDUPLICATES_H__9E36492F_F594_4668_A336_022BE1FC5437__INCLUDED_)
