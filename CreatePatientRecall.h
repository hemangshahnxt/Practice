#pragma once

#include "RecallUtils.h"

// (b.savon 2012-02-28 17:09) - PLID 48301 - created

// CCreatePatientRecall dialog
enum RecallTemplateColumns{
	rtcCreate = 0,
	rtcID = 1,
	rtcName = 2,
};

enum DiagCodeColumns{
	dccID = 0,
	dccCode = 1, 
	dccDescription = 2,
};

class CCreatePatientRecall : public CNxDialog
{
	DECLARE_DYNAMIC(CCreatePatientRecall)
public:
	struct PatientRecall{
		PatientRecall()
		{
			nRecallTemplateID = -1;
			strRecallTemplateName = "";

			nDiagCodeID = -1;
			strDiagCode = "";
			strDiagDesc = "";

			nEmrGroupID = -1;
			nLabID = -1;
			nApptID = -1;

			nPatientID = -1;
			nPatientDisplayID = -1;
			strPatientName = "";

			// (j.jones 2016-02-17 16:30) - PLID 68348 - added provider ID and location ID, optional
			nProviderID = -1;
			nLocationID = -1;
		}

		long nRecallTemplateID;
		CString strRecallTemplateName;

		long nDiagCodeID;
		CString strDiagCode;
		CString strDiagDesc;

		// (z.manning 2013-10-29 11:19) - PLID 59212 - Renamed this to make it clear that it refers to EmrGroupsT.ID
		long nEmrGroupID;
		long nLabID;
		long nApptID;

		long nPatientID;
		long nPatientDisplayID;
		CString strPatientName;

		// (j.jones 2016-02-17 16:30) - PLID 68348 - added provider ID and location ID, optional
		long nProviderID;
		long nLocationID;
	};

private:
	CNxColor		m_nxcBackground;
	CNxIconButton	m_btnCreate;
	CNxIconButton	m_btnCancel;
	CNxStatic		m_nxsDescription;

	NXDATALIST2Lib::_DNxDataListPtr m_pdlRecallTemplates;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlDiagnosisCodes;
	// (j.jones 2016-02-17 16:20) - PLID 68348 - added provider & location combos
	NXDATALIST2Lib::_DNxDataListPtr m_ProviderCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_LocationCombo;

	PatientRecall m_prciPatientRecall;

	//DiagCode - Template
	CMap<int, int, int, int> m_mapRecalls;

public:
	CCreatePatientRecall(CCreatePatientRecall::PatientRecall &prRecall, CWnd* pParent = NULL);   // standard constructor
	virtual BOOL OnInitDialog();
	virtual ~CCreatePatientRecall();

	void CreateRecalls();

// Dialog Data
	enum { IDD = IDD_CREATE_RECALL_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// (r.gonet 04/13/2014) - PLID 60870 - Combined GetEMNDiagnosisCodeIDs into GetRecallTemplateDiagnosisIDs.
	void GetRecallTemplateDiagnosisIDs(CArray<CSimpleArray<int>, CSimpleArray<int>>& aryRecallTemplateDiagIDs, CArray<int, int>& aryDiagCodeIDs);

	BOOL IsValid();

	void IntellisenseRecallTemplateList(CArray<CSimpleArray<int>, CSimpleArray<int>>& aryRecallTemplateDiagIDs, CArray<int, int>& aryDiagCodeIDs);
	
	void FillMap(CArray<CSimpleArray<int>, CSimpleArray<int>>& aryRecallTemplateDiagIDs, CArray<int, int>& aryDiagCodeIDs);

	void FillListStruct(CArray<RecallUtils::RecallListMap, RecallUtils::RecallListMap>& aryRecallListMap);

	void SetMapChecks();
	void UpdateButtons();
	long GetCheckedRecallCount();

	void GetCurrentRecallDiagCodes(CArray<long, long>& aryCurrDiagCodes);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedNxdlRecallTemplate(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};
