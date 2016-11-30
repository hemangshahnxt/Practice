#if !defined(AFX_IMPORTAMACODESDLG_H__1A1160E7_74E0_4959_87E1_B99C9CBF4918__INCLUDED_)
#define AFX_IMPORTAMACODESDLG_H__1A1160E7_74E0_4959_87E1_B99C9CBF4918__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImportAMACodesDlg.h : header file
//


//DRT 4/23/2007 - PLID 25598 - Global function to get the version from a file.
long GetAMAVersionOfFile(long nFile);

/////////////////////////////////////////////////////////////////////////////
// CImportAMACodesDlg dialog

class CImportAMACodesDlg : public CNxDialog
{
// Construction
public:
	CImportAMACodesDlg(CWnd* pParent);   // standard constructor
	void SetType(int nType);

	// (z.manning, 05/01/2008) - PLID 29860 - Added NxIconButton for OK and Cancel
// Dialog Data
	//{{AFX_DATA(CImportAMACodesDlg)
	enum { IDD = IDD_IMPORT_AMA_CODES_DLG };
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnUnselectAll;
	CNxIconButton	m_btnSelectOne;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnFilter;
	CNxStatic	m_nxstaticCodeVersion;
	CNxStatic	m_nxstaticProgressLabel;
	CNxStatic	m_nxstaticProgressAmt;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnImport;
	CNxStatic	m_nxstaticCategoryLabel; //(e.lally 2010-02-16) PLID 36849
	CNxStatic	m_nxstaticTopsNoticeLabel; //(e.lally 2010-02-16) PLID 36849
	CNxStatic	m_nxstaticTopsCptInstructions; //(e.lally 2010-02-16) PLID 36849
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImportAMACodesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pUnselected;
	NXDATALISTLib::_DNxDataListPtr m_pSelected;
	NXDATALISTLib::_DNxDataListPtr m_pCategories;
	//(e.lally 2010-02-16) PLID 36849
	NXDATALIST2Lib::_DNxDataListPtr m_pTopsCptList;
	int m_nPct;		//used to keep track of the last % update on the progress window
	int m_nType;
	BOOL m_bImporting;
	long m_nLastCategoryRow;
	BOOL m_bAlwaysChoose;
	BOOL m_bFiltered;//stores the status of an applied filter (on or off)
	

	void LoadCurrentAMACodes();
	void LoadAMACodes();		//DRT 4/23/2007 - PLID 25598 - No longer allow multiple versions per file.
	CFile* OpenAMACodeFile();
	void TryAddToCategory(CString strCat);
	void ReadCodesFromArchive(CArchive* ar, long nCntToRead);	//DRT 4/23/2007 - PLID 25598 - No longer in need of ignoring values.
	void CleanupFileVars(CFile* f, CArchive* ar);
	void ImportSingleCode(CString strCode, CString strDesc, long nAuditID);
	void UpdatePercentage(double pct);
	void ImportSingleCPT(CString strCode, CString strDesc, long nAuditID);
	void ImportSingleDiag(CString strCode, CString strDesc, long nAuditID);
	void ImportSingleMod(CString strCode, CString strDesc, long nAuditID);
	void PrepareImport();
	void UnprepareImport();

	//DRT 4/10/2007 - PLID 25556 - Verifies the member file version vs the member allowed version, if they
	//	differ, attempts to download the newest files.
	long VerifyDownloadedFiles();

	//DRT 4/10/2007 - PLID 25556 - Track the currently allowed version.  A value of 0 means
	//	nothing is allowed.  This is the value pulled from AMAVersionsT.
	DWORD m_dwCurrentAMAVersionAllowed;
	//	This value is the AMA version of the actual file that is currently downloaded.  If we are
	//	in a transition period to a new update, this may differ from the above.  A value of 0
	//	means that we have not yet found out what version we have, or no version exists.
	DWORD m_dwCurrentAMAVersionDownloaded;

	//DRT 4/19/2007 - PLID 25598 - Do the authorization
	void UpdateCodes(BOOL bIgnoreNoChange = FALSE);
	//(e.lally 2010-02-16) PLID 36849 - New functions for the TOPS mode
	void AddTopsCptCodeLink(long nCptCodeID, CStringArray* paryAmaCodes, CStringArray* paryAmaDescriptions);
	void RemoveTopsCptCodeLink(long nCptCodeID, CStringArray* paryAmaCodes);
	void RemoveAllTopsCptCodeLinks(long nCptCodeID);
	void LoadTopsLinkedAmaCodes(long nCptCodeID);

	// Generated message map functions
	//{{AFX_MSG(CImportAMACodesDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChangingAmaCategoryList(long FAR* nNewSel);
	afx_msg void OnSelChosenAmaCategoryList(long nRow);
	afx_msg void OnAmaSelectOne();
	afx_msg void OnAmaSelectAll();
	afx_msg void OnAmaUnselectOne();
	afx_msg void OnAmaUnselectAll();
	afx_msg void OnDblClickCellAmaUnselected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellAmaSelected(long nRowIndex, short nColIndex);
	afx_msg void OnAmaFilter();
	afx_msg void OnUpdateCodes();
	DECLARE_EVENTSINK_MAP()
	void OnSelChosenTopsCptList(LPDISPATCH lpRow);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void RemoveFilter();
	void ApplyFilter();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPORTAMACODESDLG_H__1A1160E7_74E0_4959_87E1_B99C9CBF4918__INCLUDED_)
