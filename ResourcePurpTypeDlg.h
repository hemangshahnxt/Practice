#if !defined(AFX_RESOURCEPURPTYPEDLG_H__C8B79A47_DF43_4823_A5C5_C023A19996E3__INCLUDED_)
#define AFX_RESOURCEPURPTYPEDLG_H__C8B79A47_DF43_4823_A5C5_C023A19996E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResourcePurpTypeDlg.h : header file
//
#include "administratorrc.h"

/////////////////////////////////////////////////////////////////////////////
// CResourcePurpTypeDlg dialog

class CResourcePurpTypeDlg : public CNxDialog
{
public:
	class CCombination
	{
	protected:
		long m_nResourceID;
		long m_nAptTypeID;
		long m_nAptPurposeID;
		BOOL m_bSelected;

	public:
		CCombination(CCombination* p)
		{
			m_nResourceID = p->m_nResourceID;
			m_nAptTypeID = p->m_nAptTypeID;
			m_nAptPurposeID = p->m_nAptPurposeID;
			m_bSelected = p->m_bSelected;
		}
		CCombination(long nResourceID, long nAptTypeID, long nAptPurposeID)
		{
			m_nResourceID = nResourceID;
			m_nAptTypeID = nAptTypeID;
			m_nAptPurposeID = nAptPurposeID;
			m_bSelected = FALSE;
		}
		inline long GetResourceID() const { return m_nResourceID; }
		inline long GetAptTypeID() const { return m_nAptTypeID; }
		inline long GetAptPurposeID() const { return m_nAptPurposeID; }
		inline BOOL GetSelected() { return m_bSelected; }
		inline void SetSelected(BOOL bSelected) { m_bSelected = bSelected; }
		virtual ~CCombination() {};
	};
protected:
	CArray<CCombination*, CCombination*> m_apCombinations;
	CArray<CCombination*, CCombination*> m_apChanged;
	NXDATALISTLib::_DNxDataListPtr m_dlAptPurpose;
	NXDATALISTLib::_DNxDataListPtr m_dlAptType;
	NXDATALISTLib::_DNxDataListPtr m_dlAptResource;

	void RequeryResources();
	void RequeryAptTypes();
	void RequeryAptPurposes();

// Construction
public:
	CResourcePurpTypeDlg(CWnd* pParent);   // standard constructor
	virtual ~CResourcePurpTypeDlg();

	void GetResults(CArray<CCombination*, CCombination*>& aResults,
		CArray<CCombination*, CCombination*>& aChanged);
	void SetResults(CArray<CCombination*, CCombination*>& aResults,
		CArray<CCombination*, CCombination*>& aChanged);

// Dialog Data
	//{{AFX_DATA(CResourcePurpTypeDlg)
	enum { IDD = IDD_RESOURCE_PURPTYPE };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResourcePurpTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CResourcePurpTypeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedProcedureList(short nFlags);
	afx_msg void OnSelChosenApttypeCombo(long nRow);
	afx_msg void OnEditingFinishedProcedureList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnSelChosenAptresourceCombo(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESOURCEPURPTYPEDLG_H__C8B79A47_DF43_4823_A5C5_C023A19996E3__INCLUDED_)
