#pragma once
#include "afxwin.h"
#include "AdministratorRc.h"

// CInactiveProcedureWarningDlg dialog
// (c.haag 2009-01-06 09:54) - PLID 10776 - Initial implementation. This dialog
// lists reasons why a user cannot inactivate a procedure, and warnings that make
// them think hard before doing so.

class CInactiveProcedureWarnings
{
// The following members correspond to items that will prevent this
// procedure from being inactivated
public:
	CString m_strProcedureName;
public:
	// Child procedures
	CStringArray m_astrChildProcedures;
	// Ladders with "Quote a procedure / bill a procedure" steps for this procedure
	CStringArray m_astrLadderSteps;
	// EMR list items using this procedure for an action
	CStringArray m_astrEmrItemSpawners;
	// EMR image items using this procedure for a hotspot action
	CStringArray m_astrEmrHotSpotSpawners;
	// EMR templates using this procedure
	CStringArray m_astrEMRTemplates;
	// Custom record templates using this procedure
	CStringArray m_astrCustomRecordTemplates;

// The following members correspond to warnings that the user should
// consider when inactivating the procedure
public:
	CStringArray m_astrLinkedServiceCodes;
	CStringArray m_astrLinkedInvItems;
	BOOL m_bHasParentProcedure;
	CString m_strParentProcedure;
	CStringArray m_astrLadders;

public:
	CInactiveProcedureWarnings(long nProcedureID, const CString& strProcedureName);
public:
	// Returns TRUE if any condition would prevent the procedure from being inactivated
	BOOL HasErrors();
	// Returns TRUE if the procedure can be inactivated, but it will have side effects
	// (which would be no different from a user manually removing such data relationships
	// one by one)
	BOOL HasWarnings();
};

class CInactiveProcedureWarningDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInactiveProcedureWarningDlg)

public:
	CInactiveProcedureWarningDlg(CInactiveProcedureWarnings& ipw, CWnd* pParent);   // standard constructor
	virtual ~CInactiveProcedureWarningDlg();

// Dialog Data
	enum { IDD = IDD_INACTIVE_PROCEDURE_WARNING };

private:
	CInactiveProcedureWarnings& m_ipw;

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_dlList;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	void AddRow(const CString& str, COLORREF clr = RGB(0,0,0));
protected:
	CStatic m_stHeader;
	CStatic m_stFooter;
	CNxIconButton m_btnClose;
	CNxIconButton m_btnYes;
	CNxIconButton m_btnNo;
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnBnClickedYes();
	afx_msg void OnBnClickedNo();
	afx_msg void OnBnClickedSendToNotepad();
};
