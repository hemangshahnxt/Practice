#if !defined(AFX_CONFIGURESUPERBILLTEMPLATESDLG_H__9C76197E_8CD9_47A4_A57F_8E1E8545D6A1__INCLUDED_)
#define AFX_CONFIGURESUPERBILLTEMPLATESDLG_H__9C76197E_8CD9_47A4_A57F_8E1E8545D6A1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigureSuperbillTemplatesDlg.h : header file
//

//DRT 6/6/2008 - PLID 30306 - Created.
/////////////////////////////////////////////////////////////////////////////
// CConfigureSuperbillTemplatesDlg dialog

//Parameter for the load function to allow overriding and just loading a certain type of data.
enum eLoadTypes {
	eltGeneral = 0x01,
	eltTypes = 0x02,
	eltResources = 0x04,

	//If you add more types, add them here as well.
	eltAll = eltGeneral|eltTypes|eltResources,
};

class CConfigureSuperbillTemplatesDlg : public CNxDialog
{
// Construction
public:
	CConfigureSuperbillTemplatesDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigureSuperbillTemplatesDlg)
	enum { IDD = IDD_CONFIGURE_SUPERBILL_TEMPLATES_DLG };
	CNxStatic	m_nxstaticHeaderText;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnRemovePickList;
	CNxIconButton	m_btnRemoveResource;
	CNxIconButton	m_btnRemoveType;
	CNxIconButton	m_btnModifyResource;
	CNxIconButton	m_btnModifyType;
	CNxIconButton	m_btnAddType;
	CNxIconButton	m_btnAddPickList;
	CNxIconButton	m_btnAddResource;
	NxButton	m_btnBrowse;
	NxButton	m_btnGlobal;
	NxButton	m_btnByPickList;
	NxButton	m_btnByResource;
	NxButton	m_btnByType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigureSuperbillTemplatesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pResourceList;
	NXDATALIST2Lib::_DNxDataListPtr m_pPickList;

	void LoadFromData(eLoadTypes eltLoadInfo = eltAll);
	void EnsureInterface();

	long GetMergeTemplateID(CString strPath);
	void AddTextToCell(NXDATALIST2Lib::IRowSettingsPtr pRow, short nColumn, CString strTextToAdd);

	// Generated message map functions
	//{{AFX_MSG(CConfigureSuperbillTemplatesDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSuperbillAddType();
	afx_msg void OnSuperbillAddResource();
	afx_msg void OnSuperbillAddPicklist();
	afx_msg void OnSuperbillRemoveType();
	afx_msg void OnSuperbillRemoveResource();
	afx_msg void OnSuperbillRemovePicklist();
	afx_msg void OnSuperbillModifyType();
	afx_msg void OnSuperbillModifyResource();
	afx_msg void OnGlobalTemplateBrowse();
	afx_msg void OnSuperbillBrowse();
	afx_msg void OnSuperbillGlobal();
	afx_msg void OnSuperbillByType();
	afx_msg void OnSuperbillByResource();
	afx_msg void OnSuperbillByPicklist();
	afx_msg void OnDblClickCellSuperbillTypeList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellSuperbillResourceList(LPDISPATCH lpRow, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGURESUPERBILLTEMPLATESDLG_H__9C76197E_8CD9_47A4_A57F_8E1E8545D6A1__INCLUDED_)
