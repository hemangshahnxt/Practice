#if !defined(AFX_CONFIGURESUPERBILLMODIFYGROUPSDLG_H__4BC3F564_B55A_4A4E_B393_3779B4F2C082__INCLUDED_)
#define AFX_CONFIGURESUPERBILLMODIFYGROUPSDLG_H__4BC3F564_B55A_4A4E_B393_3779B4F2C082__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigureSuperbillModifyGroupsDlg.h : header file
//

//DRT 6/6/2008 - PLID 30306 - Created.
/////////////////////////////////////////////////////////////////////////////
// CConfigureSuperbillModifyGroupsDlg dialog

//This dialog is designed to be somewhat generic, so it can load either types or resources.  This
//	struct is designed to let the dialog work modularly with any type of data that has an ID / Name pair.
struct ConfigItem {
	long nID;
	CString strName;
};

//These are all the types of things currently supported by this dialog.  If you wish to add a new item type, 
//	just add an enum here then add the corresponding code everywhere else in the .cpp that uses one of these.
enum ConfigSuperbillItemType {
	csitUnknown = -1,			//Not valid
	csitApptType = 0,			//AptTypeT
	csitApptResource = 1,		//ResourceT
};

class CConfigureSuperbillModifyGroupsDlg : public CNxDialog
{
// Construction
public:
	CConfigureSuperbillModifyGroupsDlg(CWnd* pParent);   // standard constructor

	////////////////////////////////////
	//Input only.  These should be set before the dialog is launched to pre-fill the lists.
	////////////////////////////////////

	//If we are modifying, this is the saved GroupID.  If new, it's -1.
	long m_nGroupID;

	//This can be resources, types, whatever.  Anything with an ID & a Name.  The things added
	//	here should be supported in the ConfigSuperbillItemType enum.  This is an optional
	//	parameter, if you have nothing to pre-load, leave it empty.  This must be loaded
	//	before invoking DoModal().
	CArray<ConfigItem, ConfigItem&> m_aryInitialItems;

	//We don't track IDs, just the path itself.  This is the "superbill path", which always
	//	starts at GetSharedPath() ^ Templates ^ Forms.  This is an optional parameter, if you
	//	have nothing to pre-load, leave it empty.  This must be loaded before invoking DoModal().
	CStringArray m_aryInitialTemplates;

	//This is the "Item Type" setup for this particular dialog instance.  This must be one of the enums
	//	above, not csitUnknown.  This is a required parameter, and must be set by the caller before
	//	invoking DoModal().
	ConfigSuperbillItemType m_csitItemType;

	////////////////////////////////////
	//Output only.  These will be filled when the user presses OK.
	////////////////////////////////////
	//This array is a list of all items which have been added newly since the dialog was opened.
	CArray<ConfigItem, ConfigItem&> m_aryItemsAdded;
	//This array is a list of all items which were in the initial array and have since been removed by the user.
	CArray<ConfigItem, ConfigItem&> m_aryItemsRemoved;

	//This array is a list of all templates which have been added newly since the dialog was opened.
	CStringArray m_aryTemplatesAdded;
	//This array is a list of all templates which were in the initial array and have since been removed by the user.
	CStringArray m_aryTemplatesRemoved;


// Dialog Data
	//{{AFX_DATA(CConfigureSuperbillModifyGroupsDlg)
	enum { IDD = IDD_CONFIGURE_SUPERBILL_MODIFY_GROUPS };
	CNxStatic	m_nxstaticItemLabel;
	CNxStatic	m_nxstaticTemplateLabel;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnRemoveTemplate;
	CNxIconButton	m_btnRemoveItem;
	CNxIconButton	m_btnAddTemplate;
	CNxIconButton	m_btnAddItem;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigureSuperbillModifyGroupsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pItemList;
	NXDATALIST2Lib::_DNxDataListPtr m_pTemplateList;

	void LoadInitialData();
	void AddRowToTemplateList(CString strPath);
	void AddRowToItemList(ConfigItem *pci);
	void DetermineItemListChanges();
	void DetermineTemplateListChanges();

	// Generated message map functions
	//{{AFX_MSG(CConfigureSuperbillModifyGroupsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSuperbillGroupAddItem();
	afx_msg void OnSuperbillGroupAddTemplate();
	afx_msg void OnSuperbillGroupRemoveItem();
	afx_msg void OnSuperbillGroupRemoveTemplate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGURESUPERBILLMODIFYGROUPSDLG_H__4BC3F564_B55A_4A4E_B393_3779B4F2C082__INCLUDED_)
