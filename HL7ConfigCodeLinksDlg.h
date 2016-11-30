#if !defined(AFX_HL7CONFIGCODELINKSDLG_H__D60C9874_D651_4981_A429_18725E73C344__INCLUDED_)
#define AFX_HL7CONFIGCODELINKSDLG_H__D60C9874_D651_4981_A429_18725E73C344__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HL7ConfigCodeLinksDlg.h : header file
//

//TES 5/24/2010 - PLID 38865 - Changed CHL7ConfigLocationsDlg to CHL7ConfigCodeLinksDlg.  You now must pass in an HL7CodeLink_RecordType
// enum value, to specify which set of codes you're modifying.  I didn't bother putting PLID comments everywhere.
/////////////////////////////////////////////////////////////////////////////
// CHL7ConfigCodeLinksDlg dialog

enum HL7CodeLink_RecordType;
class CHL7ConfigCodeLinksDlg : public CNxDialog
{
// Construction
public:
	CHL7ConfigCodeLinksDlg(CWnd* pParent);   // standard constructor

	long m_nHL7GroupID;
	HL7CodeLink_RecordType m_hclrtType;

	// (j.jones 2008-05-08 09:38) - PLID 29953 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CHL7ConfigCodeLinksDlg)
	enum { IDD = IDD_HL7_CONFIG_CODE_LINKS };
	// (r.gonet 09/27/2011) - PLID 45719
	CNxIconButton	m_btnImport;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnRemove;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHL7ConfigCodeLinksDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pCodeMapList;

	//TES 9/16/2010 - PLID 40518 - Instead of tracking this in variables, we now track the old maps in the datalist, deleted maps in 
	// the new m_arDeletedCodeMaps variable, and we read the group name out of the global CHL7Settings cache, rather than having our
	// own cached value.
	// (j.jones 2010-05-12 09:25) - PLID 36527 - for auditing,
	// cache the group name and the initial values
	/*CString m_strHL7GroupName;
	CStringArray m_arystrOldCodes;
	CMap<CString, LPCTSTR, long, long> m_mapOldCodesToPracticeIDs;*/
	struct CodeMap {
		CString strCode;
		long nPracticeID;
	};
	CArray<CodeMap,CodeMap&> m_arDeletedCodeMaps;

	//TES 9/16/2010 - PLID 40518 - I added a long-overdue utility function to get the record name for a given ID.
	CString GetPracticeName(long nPracticeID);

	// Generated message map functions
	//{{AFX_MSG(CHL7ConfigCodeLinksDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddHl7CodeMap();
	afx_msg void OnRemoveHl7CodeMap();
	virtual void OnOK();
	afx_msg void OnSelChangedHL7CodeMap(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnRButtonUpHL7CodeMap(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnEditingFinishingHL7CodeMap(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (r.gonet 09/27/2011) - PLID 45719
	afx_msg void OnBnClickedImportHl7CodeMap();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HL7CONFIGCODELINKSDLG_H__D60C9874_D651_4981_A429_18725E73C344__INCLUDED_)
