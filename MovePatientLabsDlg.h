#pragma once

// (r.gonet 05/19/2014) - PLID 40426 - Dialog that lets you move labs orders and results to a different patient or a different lab on the same patient.

#include "PatientsRc.h"

// CMovePatientLabsDlg dialog

class CMovePatientLabsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CMovePatientLabsDlg)

private:
	class CMailSentEntry {
	public:
		long m_nMailID;
		long m_nPersonID;
		CString m_strSelection;
		CString m_strPathName;
		CString m_strSubject;
		CString m_strSender;
		COleDateTime m_dtDate;
		long m_nLocation;
		long m_nMailBatchID;
		long m_nInternalRefID;
		CString m_strInternalTblName;
		long m_nMergedPacketID;
		COleDateTime m_dtServiceDate;
		long m_nCategoryID;
		long m_nChecksum;
		long m_nPicID;
		long m_nLabStepID;
		long m_nEMNID;
		BOOL m_bIsPhoto;
		long m_nTemplateID;
		long m_nPictureOrigID;
		long m_nCCDAtypeField;
		CString m_strNote;

		CMailSentEntry();
		CMailSentEntry(long nMailID, long nPersonID, CString strSelection, CString strPathName, CString strSubject, CString strSender,
			COleDateTime dtDate, long nLocation, long nMailBatchID, long nInternalRefID, CString strInternalTblName, long nMergedPacketID, COleDateTime dtServiceDate,
			long nCategoryID, long nChecksum, long nPicID, long nLabStepID, long nEMNID, BOOL bIsPhoto, long nTemplateID, long nPictureOrigID, long nCCDAtypeField, CString strNote);
		CMailSentEntry(CMailSentEntry &other);
	};

	enum class EResultComboColumns {
		ID = 0,
		Name,
		ReceivedDate,
		Value,
		FinalDiagnosis,
		MicroscopicDescription,
		Comments,
	};

	enum class EToPatientComboColumns {
		PersonID = 0,
		Name,
		UserDefinedID,
		BirthDate,
		SocialSecurity,
	};

	enum class EToOrderComboColumns {
		ID = 0,
		FormNumberTextID,
		Specimen,
		Description,
		InputDate,
	};

	CNxStatic m_nxstaticHeader;
	CNxColor m_nxcolorBackground;
	CNxStatic m_nxstaticResults;
	NXDATALIST2Lib::_DNxDataListPtr m_pResultCombo;
	CNxLabel m_nxlMultiResults;
	CNxStatic m_nxstaticToPatient;
	NXDATALIST2Lib::_DNxDataListPtr m_pToPatientCombo;
	CNxStatic m_nxstaticToOrder;
	NXDATALIST2Lib::_DNxDataListPtr m_pToOrderCombo;
	
	CNxIconButton m_btnMove;
	CNxIconButton m_btnCancel;

	long m_nFromPatientID;
	long m_nFromLabID;
	CArray<long, long> m_arySelectedLabResultIDs;
	bool m_bNotifyOnce = true;

public:
	CMovePatientLabsDlg(CWnd* pParent, long nPatientID, long nLabID, long nLabResultID);   // standard constructor
	virtual ~CMovePatientLabsDlg();

// Dialog Data
	enum { IDD = IDD_MOVE_PATIENT_LABS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	DECLARE_EVENTSINK_MAP()
	void RequeryFinishedMplResultCombo(short nFlags);
	void AddSpecialRowsToPatientCombo();
	void ReloadToOrderCombo(long nToPatientID);
	CString GetLabDescription(ADODB::FieldsPtr pFields);
	void SelChosenMplResultCombo(LPDISPATCH lpRow);
	void SelectMultipleResults();
	void SelChosenMplToPatientCombo(LPDISPATCH lpRow);
	afx_msg void OnBnClickedMplMoveButton();
	void CopyMailSent(long nSourceMailID, long nToPatientID);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	bool MoveMailSent(CMailSentEntry &mailSentEntry, long nToPatientID, bool bCopy);
	bool IsDocumentInUse(CMailSentEntry &mailSentEntry);
	void GetPathComponents(CMailSentEntry &mailSentEntry, CString &strFileNameFromList, CString &strPathFromList);
	void FillMailSentEntry(ADODB::FieldsPtr pMailSentFields, CMailSentEntry &mailSentEntry);
	CString GetPatientDocumentPath(long nPatientID);
};