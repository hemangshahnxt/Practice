#pragma once

//(e.lally 2009-05-07) PLID 34187 - Created
// COrderSetTemplateDlg dialog

class COrderSetTemplateDlg : public CNxDialog
{
	DECLARE_DYNAMIC(COrderSetTemplateDlg)

public:
	COrderSetTemplateDlg(CWnd* pParent);   // standard constructor
	

// Dialog Data
	enum { IDD = IDD_ORDER_SET_TEMPLATE_DLG };

protected:
	enum EOrderSetComponent
	{
		eLabProcedure=0,
		eMedication,
		eReferral,
	};

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	void EnsureControls();
	void ReLoad();
	void LoadTemplateDetails(long nOrderSetTemplateID);
	void HandleRButtonDown(LPDISPATCH lpRow, EOrderSetComponent eSelectList);
	void Save();
	void ClearDetails();
	void ClearArrays();
	BOOL HasChanges();
	int SaveAndContinue();
	BOOL DeleteOrderSetTemplate(long nTemplateID);

	NXDATALIST2Lib::_DNxDataListPtr m_pdlOrderSetList,
		m_pdlLabProceduresList,
		m_pdlLabProceduresSelected,
		m_pdlMedicationsList,
		m_pdlMedicationsSelected,
		m_pdlReferringPhysiciansList,
		m_pdlReferringPhysiciansSelected;

	CNxColor m_nxcolorTop, m_nxcolorBottom;
	CNxStatic m_nxstaticOrderSet;
	CNxStatic m_nxstaticLabs;
	CNxStatic m_nxstaticMeds;
	CNxStatic m_nxstaticRefPhy;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnDelete;

	struct OrderSetLabInfo
	{
		long nID;
		CString strProcedureName;
		CString strToBeOrdered;
		CString strOrigToBeOrdered;

		OrderSetLabInfo()
		{
			nID = -1;
		}
	};

	BOOL m_bIsNewTemplate;
	long m_nOrderSetTemplateID;
	// (e.lally 2009-07-20) PLID 34187
	CArray<long> m_aryDeletedTemplates;
	CArray<long> m_aryDeletedLabProcs;
	CArray<OrderSetLabInfo,OrderSetLabInfo&> m_aryChangedLabProcs;
	CArray<long> m_aryAddedMedications, m_aryDeletedMedications;
	CArray<long> m_aryAddedReferringPhys, m_aryDeletedReferringPhys;
	CStringArray m_aryAddedDetailNames, m_aryDeletedDetailNames;
	// (e.lally 2009-07-20) PLID 34187
	CPtrArray m_paryAddedLabRowPtr;

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void OnRequeryFinishedOrderSetTemplateList(short nFlags);
	void OnSelChosenOrderSetTemplateList(LPDISPATCH lpRow);
	void OnSelChosenOrdersetTmplLabs(LPDISPATCH lpRow);
	void OnSelChosenOrdersetTmplMeds(LPDISPATCH lpRow);
	void OnSelChosenOrdersetTmplRefPhy(LPDISPATCH lpRow);
	afx_msg void OnBtnClickedAddOrderSetTemplate();
	afx_msg void OnBtnClickedDeleteOrderSetTemplate();
	void OnRButtonDownOrdersetTmplLabsSelected(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnRButtonDownOrdersetTmplMedsSelected(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnRButtonDownOrdersetTmplRefPhySelected(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnLeftClickOrdersetTmplLabsSelected(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RequeryFinishedOrdersetTmplMeds(short nFlags);
	void RequeryFinishedOrdersetTmplMedsSelected(short nFlags);
};
