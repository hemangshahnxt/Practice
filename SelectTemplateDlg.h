#if !defined(AFX_SELECTTEMPLATEDLG_H__0B7BF554_0C6B_4D2D_8A77_8AB1CFB0BA69__INCLUDED_)
#define AFX_SELECTTEMPLATEDLG_H__0B7BF554_0C6B_4D2D_8A77_8AB1CFB0BA69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectTemplateDlg.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CSelectTemplateDlg dialog

class CSelectTemplateDlg : public CNxDialog
{
// Construction
public:
	CSelectTemplateDlg(CWnd* pParent);   // standard constructor

	ADODB::_RecordsetPtr m_rsTemplates;

	//Can the user select "Track Separately"? (defaults to true).
	bool m_bAllowMultiple;

	// (d.moore 2007-07-09 13:02) - PLID 14670 - Since it is now possible to select multiple
	//  ladder templates in some cases, the selected IDs are now accessed by passing in a
	//  CMap that will have the values copied into it.
	void GetSelectedLadderIds(CMap<long, long, long, long> &arIDs);
	// (d.moore 2007-08-20) - PLID 14670 - It is sometimes much easier to just get
	//  the ID selected when you know that there should only be one row selected.
	long GetSelectedLadderID();

	// Get the number of ladders that were selected. This should only be called after 
	//  the dialog has been opened and OK or Cancel were clicked.
	long GetSelectionCount();

// Dialog Data
	//{{AFX_DATA(CSelectTemplateDlg)
	enum { IDD = IDD_SELECT_TEMPLATE };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOk;
	CNxStatic	m_nxstaticSelectTemplateCaption;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectTemplateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pTemplateNameList;

	// (d.moore 2007-07-09 10:40) - PLID 14670 - Map used to store the number of ladders
	//  associated with each procedure.
	CMap<long, long, long, long> m_mProcedureCounts;
	// (d.moore 2007-07-09 12:53) - PLID 14670 - Map a ladder to each procedure based on
	//  the user selections. These values will eventually be accessable from outside.
	CMap<long, long, long, long> m_mProcedureToLadder;

	// (d.moore 2007-08-20) - PLID 14670 - Keep track of how many ladders are currently selected.
	long m_nLadderSelCount;
	
	// (d.moore 2007-07-06 12:13) - PLID 14670 - Column names for the ladder template data list.
	enum ELadderTemplateCols {
		ltcProcedureIdCol,
		ltcLadderTemplateIdCol,
		ltcCheckBoxCol,
		ltcProcedureNameCol,
		ltcLadderTemplateNameCol
	};

	// (d.moore 2007-07-06 09:08) - PLID 14670 - I've seperated out most of the functionality
	//  for building the ladder template list. There are four different types of behaviour
	//  for this dialog depending on the type of data that is passed in.
	//  1) Only one procedure. Pick one ladder, no seperate tracking. 
	//  2) Pick one ladder. Same for multiple procedures and multiple ladders.
	//  3) Seperate tracking, there is only one ladder per procedure.
	//  4) Seperate tracking, multiple ladders per procedure. Pick one ladder per procedure.
	enum EBehaviourTypes {
		btSingleProcedure, 
		btMultProceduresNoSeperateTracking, 
		btMultProceduresAllowSeperateTracking, 
		btPickOneLadderPerProcedure
	};
	// Store and keep track of the behavior type for the dialog.
	long m_nBehaviorType;
	
	// (d.moore 2007-07-06 11:48) - PLID 14670 - Determine the correct behavior type. Returns
	//  values that correspond to the EBehaviourTypes enum.
	long DetermineBehaviorType(ADODB::_RecordsetPtr rsLadderData);

	// (d.moore 2007-07-06 09:22) - PLID 14670 - Fill the ladder template data list based
	//  on the type of behavior for the dialog.
	void BuildLadderDataList(ADODB::_RecordsetPtr rsLadderData, long nBehaviorType);

	// (d.moore 2007-07-06 10:12) - PLID 14670 - There should only be one procedure in the list.
	//  Set the dialog up to allow only a single selection.
	void BuildSingleProcedureControls();

	// (d.moore 2007-07-06 10:15) - PLID 14670 - There should only be one ladder per procedure.
	//  Select one ladder or select 'Track Seperately' option.
	void BuildMultipleProcedureControls(bool bAllowSeperateTracking);

	// (d.moore 2007-07-06 10:16) - PLID 14670 - When there are multiple ladders for at least one
	//  procedure, and m_bAllowMultiple is true, force the user to select one ladder per procedure.
	void BuildLadderCheckboxControls();
	
	// (d.moore 2007-07-09 13:11) - PLID 14670 - Validate and store the selection for
	//  behaviors allowing either single selection or the 'Track Separately' options.
	bool GetSingleLadderSelection();

	// (d.moore 2007-07-09 13:13) - PLID 14670 - Validate and store the selection for
	//  behaviors that allow selection of ladders templates for each procedure.
	bool GetMultipleLadderSelection();

	// Generated message map functions
	//{{AFX_MSG(CSelectTemplateDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDblClickCellTemplateNameList(LPDISPATCH lpRow, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTTEMPLATEDLG_H__0B7BF554_0C6B_4D2D_8A77_8AB1CFB0BA69__INCLUDED_)
