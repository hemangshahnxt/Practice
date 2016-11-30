// PracticeDoc.h : interface of the CPracticeDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PRACTICEDOC_H__F2B94DB5_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_)
#define AFX_PRACTICEDOC_H__F2B94DB5_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CPracticeDoc : public COleDocument
{
protected: // create from serialization only
	CPracticeDoc();
	DECLARE_DYNCREATE(CPracticeDoc)

// Attributes
public:
//	CPatientSet m_rsActivePatient;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPracticeDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void OnCloseDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPracticeDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	CString m_strDatabase;
	//{{AFX_MSG(CPracticeDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CPracticeDoc)
	afx_msg long AddPatient(LPDISPATCH pNewPatient);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRACTICEDOC_H__F2B94DB5_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_)
