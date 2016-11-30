#if !defined(AFX_UNITEDLINK_H__58220533_C190_40E3_BFD3_CAFF39942830__INCLUDED_)
#define AFX_UNITEDLINK_H__58220533_C190_40E3_BFD3_CAFF39942830__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnitedLink.h : header file
//

#include "PracticeRc.h"
#include "GenericLink.h"

CString GetUnitedExecutePath();

/////////////////////////////////////////////////////////////////////////////
// CUnitedLink

class CUnitedLink : public CGenericLink
{
// Construction
public:
	CUnitedLink(CWnd* pParent);   // standard constructor

	virtual CString GetRemotePath();
	virtual void SetRemotePath(const CString &path);
	long GetImageCount(long nUnitedId);
	HBITMAP LoadImage(long nUnitedId, const long &nIndex);
	void OnMassLink();
	virtual unsigned long RefreshData();
	virtual BOOL TestConnection(CString strRemotePath);
	virtual void Unlink(long lNexTechID);

// Dialog Data
	//{{AFX_DATA(CUnitedLink)
	enum { IDD = IDD_UNITED_LINK };
	NxButton	m_checkAdvanced;
	CNxStatic	m_nxstaticNextechCount;
	CNxStatic	m_nxstaticNextechExportCount;
	CNxStatic	m_nxstaticRemoteCount;
	CNxStatic	m_nxstaticRemoteExportCount;
	CNxStatic	m_nxstaticUni32Location;
	CNxIconButton	m_btnExport;
	CNxIconButton	m_btnClose;
	NxButton	m_btnShowImages;
	NxButton	m_btnLinkUserID;
	NxButton	m_btnNewPatExport;
	NxButton	m_btnEnableLink;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUnitedLink)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bExecutePathChanged;

protected:
	void RefreshColors();

	// Generated message map functions
	//{{AFX_MSG(CUnitedLink)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBtnRemotePath();
	afx_msg void OnPracAdd();
	afx_msg void OnPracRemove();
	afx_msg void OnPracRemoveAll();
	afx_msg void OnRemoteAdd();
	afx_msg void OnRemoteRemove();
	afx_msg void OnRemoteRemoveAll();
	afx_msg void OnDblClickCellNextech(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellRemote(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellExport(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellImport(long nRowIndex, short nColIndex);
	afx_msg void OnExportToRemote();
	afx_msg void OnRequeryFinishedNextech(short nFlags);
	afx_msg void OnRequeryFinishedRemote(short nFlags);
	afx_msg void OnImportFromRemote();
	afx_msg void OnShowAdvancedOptions();
	afx_msg void OnCopyFromUnitedList();
	afx_msg void OnLink();
	afx_msg void OnUnlink();
	afx_msg void OnRButtonUpNexTech(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnNewPatCheck();
	afx_msg void OnCheckLinkUserdefinedid();
	afx_msg void OnCheckUnitedshowimages();
	afx_msg void OnCheckEnableunitedlink();
	afx_msg void OnBtnUni32Location();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITEDLINK_H__58220533_C190_40E3_BFD3_CAFF39942830__INCLUDED_)
