#pragma once
#include "EMRRc.h"
#include "NxApi.h"

// (a.walling 2014-05-09 10:20) - PLID 61788 - CSelectCCDAInfoDlg formerly CSelectClinicalSummaryInfoDlg

// CSelectCCDAInfoDlg dialog

// (j.gruber 2013-12-09 10:00) - PLID 59420 - Created For

class CSelectCCDAInfoDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSelectCCDAInfoDlg)

public:
	// (r.gonet 04/22/2014) - PLID 61805 - Added the PICID since Clinical Summaries now have to be associated with a PIC.
	CSelectCCDAInfoDlg(CCDAType ccdaType, long nPICID, long nEMNID, long nPersonID, CWnd* pParent);   // standard constructor
	virtual ~CSelectCCDAInfoDlg();

// Dialog Data
	enum { IDD = IDD_SELECT_CCDA_INFO_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// (r.gonet 04/22/2014) - PLID 61805 - The ID of the PIC that the clinical summary will be associated with once it is 
	// attached to MailSent.
	long m_nPICID = -1;
	// (a.walling 2014-05-09 10:20) - PLID 61788 - Keep track of EMN, Person, UserDefinedID, and CCDA type
	long m_nEMNID = -1;
	long m_nPersonID = -1;
	long m_nUserDefinedID = -1;
	CCDAType m_ccdaType = ctNone;

	bool IsSummaryOfCare() const;
	bool IsClinicalSummary() const;

	IUnknownPtr MakePatientFilter();

	NXDATALIST2Lib::_DNxDataListPtr m_pSelectList;
	void LoadList();
	void GetSectionExclusions(CArray<NexTech_Accessor::_CCDAExclusionPtr, NexTech_Accessor::_CCDAExclusionPtr> &aryExclusions);
	void SetSection(CString strLoinc, bool bExcluded);
	void ChangeAllChildren(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bChecked);
	BOOL AllChildrenUnchecked(NXDATALIST2Lib::IRowSettingsPtr pParentRow);

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

	NexTech_Accessor::_CCDAOptionsPtr m_pOptions;

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();

	void GenerateDocument();

public:
	afx_msg void OnBnClickedOk();
	
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedCcdaSelectionList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};
