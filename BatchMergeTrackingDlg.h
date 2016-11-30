#if !defined(AFX_BATCHMERGETRACKINGDLG_H__2B6BBC5B_6270_4A01_8C91_4CBC69714F28__INCLUDED_)
#define AFX_BATCHMERGETRACKINGDLG_H__2B6BBC5B_6270_4A01_8C91_4CBC69714F28__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BatchMergeTrackingDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBatchMergeTrackingDlg dialog

bool CompareArrays(const CArray<long,long> &ar1, const CArray<long,long> &ar2);

class CBatchMergeTrackingDlg : public CNxDialog
{
// Construction
public:
	CBatchMergeTrackingDlg(CWnd* pParent);   // standard constructor

	// (a.walling 2007-12-04 13:37) - PLID 28272 - Added PicID column
	enum MergeListColumns {
		mlcSelected = 0,
		mlcPatientID,
		mlcPatientName,
		mlcLadderName,
		mlcProcInfoID,
		mlcPicID,
		mlcProcedureList,
		mlcStepID,
		mlcStepTemplateID,
		mlcActiveDate,
		mlcStepName,
		mlcActionName,
		mlcActionID,
		mlcAction,
		mlcScope
	};

	// (a.walling 2007-12-04 13:42) - PLID 28272 - Added support for nPicID
	struct MergeTemplate {
		long nMergeTemplateID;
		long nScope; //0;PIC;2;Procedure;1;Master Procedure;4;Detail Procedure;3;Prescription
		long nPicID;
		CArray<long,long> arProcIDs;  //Which procedures are we merging to this template?
		CArray<long,long> arMedicationIDs; //Which prescriptions are we merging to this template?  Should be filled only if nScope == 3.

		MergeTemplate() {}

		MergeTemplate(MergeTemplate &mtSource) {
			nMergeTemplateID = mtSource.nMergeTemplateID;
			nScope = mtSource.nScope;
			nPicID = mtSource.nPicID;
			arProcIDs.RemoveAll();
			// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - for() loops
			int i = 0;
			for(i = 0; i < mtSource.arProcIDs.GetSize(); i++) {
				arProcIDs.Add(mtSource.arProcIDs[i]);
			}
			for(i = 0; i < mtSource.arMedicationIDs.GetSize(); i++) {
				arMedicationIDs.Add(mtSource.arMedicationIDs[i]);
			}
		}

		void operator =(MergeTemplate &mtSource) {
			nMergeTemplateID = mtSource.nMergeTemplateID;
			nScope = mtSource.nScope;
			nPicID = mtSource.nPicID;
			arProcIDs.RemoveAll();
			// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - for() loops
			int i = 0;
			for(i = 0; i < mtSource.arProcIDs.GetSize(); i++) {
				arProcIDs.Add(mtSource.arProcIDs[i]);
			}
			for(i = 0; i < mtSource.arMedicationIDs.GetSize(); i++) {
				arMedicationIDs.Add(mtSource.arMedicationIDs[i]);
			}
		}

		BOOL operator ==(MergeTemplate &mtCompare) {
			if(nMergeTemplateID != mtCompare.nMergeTemplateID) return FALSE;
			if(nScope != mtCompare.nScope) return FALSE;
			//TES 7/16/2010 - PLID 39400 - If the scope is per-patient, then they match.
			if(nScope == PhaseTracking::mtsPatient) return TRUE;
			if(nPicID != mtCompare.nPicID) return FALSE;
			if(!CompareArrays(arProcIDs, mtCompare.arProcIDs)) return FALSE;
			//TES 12/21/2006 - If either one has any prescriptions, they are not identical, because prescriptions are
			// ladder-specific.
			if(arMedicationIDs.GetSize() || mtCompare.arMedicationIDs.GetSize()) return FALSE;
			return TRUE;
		}

	};
	//A MergeDocumentInfo will have all the steps grouped together possible for a given packet/template and procedure list.
	struct MergeDocumentInfo {
		bool bIsPacket;
		long nActionID;
		CArray<MergeTemplate,MergeTemplate&> arTemplates;//Will only be one entry if this is not a packet.
		//arRows will have all the selected rows which have the same ActionID and Procedure IDs.
		CArray<LPDISPATCH,LPDISPATCH> arRows;

		MergeDocumentInfo()
		{
			bIsPacket = false;
			nActionID = -1;
		}
	};


	// (z.manning, 04/25/2008) - PLID 29795 - Added NxIconButton for close
// Dialog Data
	//{{AFX_DATA(CBatchMergeTrackingDlg)
	enum { IDD = IDD_BATCH_MERGE_TRACKING };
	NxButton	m_btnMergeToPrinter;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnMergeAll;
	NxButton m_btnShowOnHoldLadders; // (z.manning 2010-06-25 13:56) - PLID 39369
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBatchMergeTrackingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void RefreshList();

	NXDATALIST2Lib::_DNxDataListPtr m_pList, m_pUserCombo;

	// Generated message map functions
	//{{AFX_MSG(CBatchMergeTrackingDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnRequeryFinishedBatchMergeList(short nFlags);
	afx_msg void OnLeftClickBatchMergeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnMergeAll();
	afx_msg void OnEditingStartingBatchMergeList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedBatchMergeList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnSelChangingTrackingUsers(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenTrackingUsers(LPDISPATCH lpRow);
	afx_msg void OnSelectAllSteps();
	afx_msg void OnUnselectAllSteps();
	afx_msg void OnShowOnHoldLadders(); // (z.manning 2010-06-25 14:00) - PLID 39369
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATCHMERGETRACKINGDLG_H__2B6BBC5B_6270_4A01_8C91_4CBC69714F28__INCLUDED_)
