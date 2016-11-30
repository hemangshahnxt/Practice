#if !defined(AFX_NEXWEBTODOTASKDLG_H__E0E4616B_8CF6_411C_BDDB_2DF7CF3099DE__INCLUDED_)
#define AFX_NEXWEBTODOTASKDLG_H__E0E4616B_8CF6_411C_BDDB_2DF7CF3099DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexWebToDoTaskDlg.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// CNexWebToDoTaskDlg dialog

//(e.lally 2007-05-17) PLID 26014 - Add ToDo tasks to the NexWeb import
class CNexWebToDoTaskDlg : public CNxDialog
{
// Construction
public:
	CNexWebToDoTaskDlg(CWnd* pParent);   // standard constructor
	~CNexWebToDoTaskDlg();

	void SetPersonID(long nPersonID, BOOL bIsNewPatient);
	long m_nPersonID;
	void LoadToDoList();
	BOOL SaveInfo(long nPersonID = -1);
	//BOOL CheckExistingToDoData();
	BOOL ValidateData();
	BOOL m_bIsNewPatient;
	CString m_strError;
	//void GenerateAuditItem();


// Dialog Data
	//{{AFX_DATA(CNexWebToDoTaskDlg)
	enum { IDD = IDD_NEXWEB_TODO_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexWebToDoTaskDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CPtrArray m_ToDoList;
	NXDATALIST2Lib::_DNxDataListPtr  m_pToDoList;

	void CreateToDo(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// Generated message map functions
	//{{AFX_MSG(CNexWebToDoTaskDlg)
		virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXWEBTODOTASKDLG_H__E0E4616B_8CF6_411C_BDDB_2DF7CF3099DE__INCLUDED_)
