#if !defined(AFX_RESPONSIBLEPARTYDLG_H__D17D7681_A478_4F31_AD2F_197CD7C7E2F7__INCLUDED_)
#define AFX_RESPONSIBLEPARTYDLG_H__D17D7681_A478_4F31_AD2F_197CD7C7E2F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResponsiblePartyDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CResponsiblePartyDlg dialog

class CResponsiblePartyDlg : public CNxDialog // (d.moore 2007-04-25 10:20) - PLID 25754 - Changed inheritence from CDialog to CNxDialog.
{
// Construction
public:
	BOOL Save();
	void Load();
	void Delete();
	void EnableItems(BOOL bEnabled);
	CResponsiblePartyDlg(CWnd* pParent);   // standard constructor
	long m_ID;

	NXDATALISTLib::_DNxDataListPtr m_RelateCombo, m_GenderCombo;

	int m_color;
	CBrush m_brush;

	void Capitalize(int ID);
	void FormatItem(int ID, CString format);
	void FillAreaCode(long nPhoneID);
	bool SaveAreaCode(long nID);
	CString m_strAreaCode;

// Dialog Data
	//{{AFX_DATA(CResponsiblePartyDlg)
	enum { IDD = IDD_RESPONSIBLE_PARTY_DLG };
	NxButton	m_btnPrimaryParty;
	CNxColor	m_bkg;
	CNxEdit	m_nxeditFirstNameBox;
	CNxEdit	m_nxeditMiddleNameBox;
	CNxEdit	m_nxeditLastNameBox;
	CNxEdit	m_nxeditAddress1Box;
	CNxEdit	m_nxeditAddress2Box;
	CNxEdit	m_nxeditZipBox;
	CNxEdit	m_nxeditCityBox;
	CNxEdit	m_nxeditStateBox;
	CNxEdit	m_nxeditEmployerSchoolBox;
	CNxEdit	m_nxeditPhoneBox;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnCopyPatInfo;
	CNxIconButton	m_btnMakeNewRespParty;
	CNxIconButton	m_btnDeleteRespParty;	
	CNxEdit m_nxeditMaidenName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides	
	//{{AFX_VIRTUAL(CResponsiblePartyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);	
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.gruber 2009-10-08 09:56) - PLID 35826 - Override the tab order if they have city lookup	
	BOOL m_bLookupByCity;

	BOOL m_bFormatPhoneNums;

	bool m_bReady;
	CString m_strPhoneFormat;

	void ClearInfo(); /// (a.walling 2006-10-13 09:15) - PLID 16059 - just clear everything in the fields

	// (d.singleton 2013-11-12 18:05) - PLID 59442 - need to have a maiden name text box for the resp party dialog.  it will only show on the dialog if the relationship is set to mother.
	void EnableMaidenNameBox(long nRow);


	NXDATALIST2Lib::_DNxDataListPtr m_dlList;

	NXTIMELib::_DNxTimePtr m_nxtBirthDate;
	// Generated message map functions
	//{{AFX_MSG(CResponsiblePartyDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMakeNewRespParty();
	afx_msg void OnCopyPatInfo();
	afx_msg void OnDeleteRespParty();
	afx_msg void OnSelChosenListResp(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedListResp(short nFlags);
	afx_msg void OnSelChangingListResp(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingRelateCombo(long FAR* nNewSel);
	afx_msg void OnSelChangingGenderList(long FAR* nNewSel);
	virtual void OnCancel();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void SelChosenRelateCombo(long nRow);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESPONSIBLEPARTYDLG_H__D17D7681_A478_4F31_AD2F_197CD7C7E2F7__INCLUDED_)
