#pragma once
#include "patientsrc.h"

// CEMRProblemChooserDlg dialog
//
// (c.haag 2009-05-13 17:38) - PLID 34249 - Initial implementation
//

class CEMRProblemChooserDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEMRProblemChooserDlg)

protected:
	// The array of all problems in the visible list that came from memory. The caller
	// is responsible for maintaining these problems.
	CArray<CEmrProblem*, CEmrProblem*> m_apListProblemsInMemory;
	// The array of all problems in the visible list that this dialog read from data. This
	// dialog is responsible for maintaining these problems.
	CArray<CEmrProblem*, CEmrProblem*> m_apListProblemsInData;

protected:
	// The memory problems which the user selected during the dialog session
	CArray<CEmrProblem*, CEmrProblem*> m_apMemoryResults;
	// The data problems which the user selected during the dialog session
	CArray<long,long> m_apDataResults;

protected:
	CNxColor m_nxcTop;

public:
	CEMRProblemChooserDlg(const CArray<CEmrProblem*, CEmrProblem*>& apProblemsInMemory,
		const CArray<CEmrProblem*, CEmrProblem*>& apProblemsToExclude,
		long nPatientID,
		CWnd* pParent);   // standard constructor
	virtual ~CEMRProblemChooserDlg();

public:
	// (c.haag 2009-05-27 12:35) - PLID 34249 - This is the main point of "entry" as far as
	// invoking the dialog. Both parameters are outputs; one for selected problems that exist
	// in memory (managed by the caller), and one for selected problems that were loaded from
	// data by this dialog (for which we return EMR Problem ID's)
	int Open(OUT CArray<CEmrProblem*, CEmrProblem*>& apSelectionsInMemory,
		OUT CArray<long,long>& anSelectionsInData);

public:
	// (c.haag 2009-05-27 12:35) - PLID 34249 - Returns true if there are any problems available to select from.
	// This should be called after construction and before invoking the dialog.
	BOOL HasProblems();

// Dialog Data
	enum { IDD = IDD_EMR_PROBLEM_CHOOSER };

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_dlProblems;

protected:
	long m_nPatientID;

protected:
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;

protected:
	// (c.haag 2009-05-27 12:39) - PLID 34249 - Populates the list with problems
	void RefilterList(CArray<CEmrProblem*, CEmrProblem*>& aryProblems, BOOL bOwnedByCaller);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

public:
	void OnOK();
};
