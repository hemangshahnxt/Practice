#pragma once


// CNexSyncEditSubjectLineDlg dialog
// (z.manning 2009-10-19 16:44) - PLID 35997 - Created

// (c.haag 2010-03-31 11:36) - PLID 37891 - The callback for populating the subject line field list
typedef void (*FillNexSyncEditSubjectLineListCallbackFunc)(LPDISPATCH);
typedef void (*HandleSubjectLineEditChangeCallbackFunc)(CEdit *peditSubject, CEdit *peditSubjectPreview, LPDISPATCH lpFieldsList);
typedef void (*HandleSubjectLineFieldsListEditingFinishedCallbackFunc)(CDialog *pdlg, CEdit *peditSubject, CEdit *peditSubjectPreview, LPDISPATCH lpFieldsList, long nRow, _variant_t varNewValue, IN OUT CString &strSubject);

class CNexSyncEditSubjectLineDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNexSyncEditSubjectLineDlg)

private:
	// (c.haag 2010-03-29 13:17) - PLID 37891 - We can now configure the default text for this dialog
	CString m_strDefaultWindowText;
	CString m_strDefaultDescription;
	CString m_strDefaultSampleText;
	CString m_strDefaultSubjectText;

	// (c.haag 2010-03-29 13:22) - PLID 37891 - Now that we have a named static control, we need a 
	// CNxStatic member for it
	CNxStatic m_nxsDescription;
	CNxStatic m_nxsSample;
	CNxStatic m_nxsSubject;

	// (c.haag 2010-03-31 11:36) - PLID 37891 - The callback for populating the field list
	FillNexSyncEditSubjectLineListCallbackFunc m_cbFillSubjectLineList;
	// (c.haag 2010-04-01 17:13) - PLID 37891 - The callback for populating the sample box
	HandleSubjectLineEditChangeCallbackFunc m_cbHandleSubjectLineEditChange;
	HandleSubjectLineFieldsListEditingFinishedCallbackFunc m_cbHandleSubjectLineFieldsListEditingFinished;

public:
	CNexSyncEditSubjectLineDlg(CWnd* pParent);   // standard constructor
	virtual ~CNexSyncEditSubjectLineDlg();

	CString GetSubject();
	void SetSubject(const CString &strSubject);

// Dialog Data
	enum { IDD = IDD_NEXSYNC_EDIT_SUBJECT };

public:
	// (c.haag 2010-03-29 13:15) - PLID 37891 - We can now configure the default window text
	void SetDefaultWindowText(const CString& str);
	// (c.haag 2010-03-29 13:15) - PLID 37891 - We can now configure the default description
	void SetDefaultDescription(const CString& str);
	// (c.haag 2010-04-01 12:13) - PLID 37891 - Assigns a callback for populating the text above the preview
	void SetDefaultSampleText(const CString& str);
	// (c.haag 2010-04-01 12:13) - PLID 37891 - Assigns a callback for populating the text above the subject format
	void SetDefaultSubjectText(const CString& str);

	// (c.haag 2010-03-31 11:36) - PLID 37891 - Assigns a callback for populating the subject field list
	void SetSubjectLineListCallbackFunc(FillNexSyncEditSubjectLineListCallbackFunc func);
	// (c.haag 2010-04-01 17:14) - PLID 37891 - Assigns a callback for populating the sample box
	void SetHandleSubjectLineEditChangeCallbackFunc(HandleSubjectLineEditChangeCallbackFunc func);
	// (c.haag 2010-04-01 17:37) - PLID 37891 - Assigns a callback for populating the sample box
	void SetHandleSubjectLineFieldsListEditingFinishedCallbackFunc(HandleSubjectLineFieldsListEditingFinishedCallbackFunc func);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CString m_strSubject;

	// (z.manning 2009-10-19 15:54) - PLID 35997 - Made this a datalist1 because it uses code
	// shared with an equivelant dialog in NxOutlookAddin that already had a datalist1.
	NXDATALISTLib::_DNxDataListPtr m_pdlFields;

	virtual BOOL OnInitDialog();

	CNxColor m_nxcolorBackground;
	CNxEdit m_nxeditSubject;
	CNxEdit m_nxeditSubjectPreview;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnChangeNexsyncSubjectEdit();
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedSubjectLineFieldList(long nRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};
