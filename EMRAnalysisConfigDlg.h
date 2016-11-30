#pragma once

// (j.jones 2008-10-14 16:31) - PLID 31692 - created

// CEMRAnalysisConfigDlg dialog

#include "EmrUtils.h"

class CEMRAnalysisConfigDlg : public CNxDialog
{

public:
	CEMRAnalysisConfigDlg(CWnd* pParent);   // standard constructor

	long m_nID;

// Dialog Data
	enum { IDD = IDD_EMR_ANALYSIS_CONFIG_DLG };
	CNxColor	m_bkg;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	NxButton	m_radioAllDates;
	NxButton	m_radioEMNDate;
	NxButton	m_radioFilterOnPatient;
	NxButton	m_radioFilterOnEMN;
	NxButton	m_radioFilterOnEMR;
	NxButton	m_radioAllItems;
	NxButton	m_radioAnyItems;
	NxButton	m_radioGroupByPatient;
	NxButton	m_radioGroupByDate;
	NxButton	m_radioGroupByEMN;
	NxButton	m_radioGroupByEMR;
	CDateTimePicker	m_dtTo;
	CDateTimePicker	m_dtFrom;
	CNxEdit		m_editDescription;
	CNxStatic	m_nxstaticAnyAll;
	NxButton	m_radioColumnGroupByItem;
	NxButton	m_radioColumnGroupByCondensed;
	// (j.jones 2009-03-27 10:00) - PLID 33703 - added m_checkFilterOnSelectedTemplate
	NxButton	m_checkFilterOnSelectedTemplate;
	// (j.jones 2009-04-09 12:31) - PLID 33916 - added m_checkShowSpawningInfo
	NxButton	m_checkShowSpawningInfo;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr	m_TemplateCombo;
	NXDATALIST2Lib::_DNxDataListPtr	m_ItemCombo;
	NXDATALIST2Lib::_DNxDataListPtr	m_ItemList;

	void Load();
	BOOL Save();

	CArray<long, long> m_aryDeletedItems;

	//returns the format settings for the iloDataOperator column, given an EMRInfoT.DataType
	NXDATALIST2Lib::IFormatSettingsPtr GetOperatorFormatSettings(EmrInfoType eDataType);
	//returns the format settings for the iloDataFilter column, given an EMRInfoT.DataType and EMRInfoT.EMRInfoMasterID
	// (j.jones 2009-04-09 15:18) - PLID 33947 - renamed to ApplyDataFilterFormatSettings and added several parameters
	void TryApplyDataFilterFormatSettings(NXDATALIST2Lib::IRowSettingsPtr pRow, EMRAnalysisDataOperatorType eOperator,
											EmrInfoType eDataType, long nEMRInfoMasterID);

	// (j.jones 2009-04-09 15:07) - PLID 33947 - track the operator format settings pointers we use
	// (a.wilson 2014-09-05 11:39) - PLID 63535 - key value should never be LPCTSTR as that would save the address of the string as the key
	CMap<CString, LPCTSTR, NXDATALIST2Lib::IFormatSettingsPtr, NXDATALIST2Lib::IFormatSettingsPtr> m_mapComboSourceToOperatorFormatSettings;

	// (j.jones 2009-03-30 11:26) - PLID 33703 - used only to re-select the previous template, if needed
	long m_nSelTemplateID;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	
	virtual BOOL OnInitDialog();
	afx_msg void OnOk();	
	afx_msg void OnSelChosenEmrInfoItemCombo(LPDISPATCH lpRow);
	afx_msg void OnRButtonDownEmrAnalysisConfigList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRadioAllEmnDates();
	afx_msg void OnRadioEmnDate();
	afx_msg void OnEditingStartingEmrAnalysisConfigList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedEmrAnalysisConfigList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnSelChosenEmrTemplateCombo(LPDISPATCH lpRow);
	afx_msg LRESULT OnStartEditingFilterList(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRadioFilterOnPatient();
	afx_msg void OnRadioFilterOnEmn();
	afx_msg void OnRadioFilterOnEmr();
	afx_msg void OnRadioAllEmrItems();
	afx_msg void OnRadioAnyEmrItems();
};
