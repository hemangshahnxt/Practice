#if !defined(AFX_CUSTOM1DLG_H__712CC0B5_05E3_11D2_8087_00104B2FE914__INCLUDED_)
#define AFX_CUSTOM1DLG_H__712CC0B5_05E3_11D2_8087_00104B2FE914__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
#include "PatientDialog.h"
#include "client.h"

// Custom1Dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCustom1Dlg dialog

class CCustom1Dlg : public CPatientDialog
{
// Construction
public:

	void ClearBoxes();
	void RestoreAllBoxes();
	void SetAllLabels();
	void RenameCustomField(CString strOld, const int nID);
	NXDATALISTLib::_DNxDataListPtr m_CustomList1;
	NXDATALISTLib::_DNxDataListPtr m_CustomList2;
	NXDATALISTLib::_DNxDataListPtr m_CustomList3;
	NXDATALISTLib::_DNxDataListPtr m_CustomList4;
	NXDATALISTLib::_DNxDataListPtr m_CustomList5;
	NXDATALISTLib::_DNxDataListPtr m_CustomList6;
	NXDATALISTLib::_DNxDataListPtr m_CustomContact1;


	CTableChecker m_custom1Checker, m_custom2Checker, m_custom3Checker,
		m_custom4Checker, m_custom5Checker, m_custom6Checker,
		m_customContactChecker;

	void AddRecordCustomCombo(NXDATALISTLib::_DNxDataListPtr* ctrl, long listid, long resid);
	void PopulateCustomDataLabelsTable();
	void SetLabel(const long &FieldID);
	virtual int	Hotkey(int key);
	CCustom1Dlg(CWnd* pParent);   // standard constructor
	long GetLabelName(int nID);
	// (a.walling 2010-12-02 13:20) - PLID 41315
	bool IsLabel(int nID);
	CString GetLabelNameDefault(int nID);
	virtual void SetColor(OLE_COLOR nNewColor);
	BOOL OnInitDialog();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void CallModalDialog(int nID);
	void StoreBox(int nID);
	// (a.walling 2010-11-01 11:07) - PLID 41315 - Parameterized all these queries and got rid of lots of duplicated code
	void StoreCustomText(int nID);
	void StoreCustomDate(int nID, bool& bSavingDate, COleDateTime& dtDate, NXTIMELib::_DNxTimePtr nxtDate);
	void StoreCustomCheckBox(int nID);
	// (j.armen 2011-06-22 11:04) - PLID 11490 - Repurposed function specifically for custom lists
	void StoreCustomList(NXDATALISTLib::_DNxDataListPtr customList, int nFieldID, UINT customListIDC );
	// (j.armen 2011-06-22 11:04) - PLID 11490 - Kept functionality of StoreCustomList for storing custom contacts
	void StoreCustomContact(int nID, NXDATALISTLib::_DNxDataListPtr customList);
	void StoreCustomField(int nFieldID, const CString& strFieldName, int nFieldType, const _variant_t& varInt, const _variant_t& varText, const _variant_t& varDate);
	void RestoreBox(int nID);
	void OnTimer(UINT nIDEvent);
	void AutoRefresh();
	// (j.armen 2011-06-22 11:05) - PLID 11490 - function for setting the display of custom list information
	void SetCustomListDisplay(ADODB::_RecordsetPtr rs, NXDATALISTLib::_DNxDataListPtr customList, UINT customListIDC, CNxLabel *customListLabel);
	virtual void StoreDetails();
	// (a.walling 2010-10-13 10:44) - PLID 40908 - Dead code
	//CMap<int, int, CString, CString> m_map;
	//CMap<int, int, int, int> m_labelToBox;
// Dialog Data
	//{{AFX_DATA(CCustom1Dlg)
	enum { IDD = IDD_CUSTOM_DLG };
	NxButton	m_CustomCheck6;
	NxButton	m_CustomCheck5;
	NxButton	m_CustomCheck4;
	NxButton	m_CustomCheck3;
	NxButton	m_CustomCheck2;
	NxButton	m_CustomCheck1;
	CNxColor	m_checkBkg;
	CNxColor	m_dateBkg;
	CNxColor	m_listBkg;
	CNxColor	m_noteBkg;
	CNxColor	m_textBkg;
	CNxEdit	m_nxeditCustomText1;
	CNxEdit	m_nxeditCustomText2;
	CNxEdit	m_nxeditCustomText3;
	CNxEdit	m_nxeditCustomText4;
	CNxEdit	m_nxeditCustomText5;
	CNxEdit	m_nxeditCustomText6;
	CNxEdit	m_nxeditCustomText7;
	CNxEdit	m_nxeditCustomText8;
	CNxEdit	m_nxeditCustomText9;
	CNxEdit	m_nxeditCustomText10;
	CNxEdit	m_nxeditCustomText11;
	CNxEdit	m_nxeditCustomText12;
	CNxEdit	m_nxeditCustomNote;
	CNxStatic	m_nxstaticCustomText1Label;
	CNxStatic	m_nxstaticCustomText2Label;
	CNxStatic	m_nxstaticCustomText3Label;
	CNxStatic	m_nxstaticCustomText4Label;
	CNxStatic	m_nxstaticCustomText5Label;
	CNxStatic	m_nxstaticCustomText6Label;
	CNxStatic	m_nxstaticCustomText7Label;
	CNxStatic	m_nxstaticCustomText8Label;
	CNxStatic	m_nxstaticCustomText9Label;
	CNxStatic	m_nxstaticCustomText10Label;
	CNxStatic	m_nxstaticCustomText11Label;
	CNxStatic	m_nxstaticCustomText12Label;
	CNxStatic	m_nxstaticCustomList1Label;
	CNxStatic	m_nxstaticCustomList2Label;
	CNxStatic	m_nxstaticCustomList3Label;
	CNxStatic	m_nxstaticCustomList4Label;
	CNxStatic	m_nxstaticCustomList5Label;
	CNxStatic	m_nxstaticCustomList6Label;
	CNxStatic	m_nxstaticCustomContact1Label;
	CNxStatic	m_nxstaticCustomMemo1Label;
	CNxStatic	m_nxstaticCustomCheck1Label;
	CNxStatic	m_nxstaticCustomCheck2Label;
	CNxStatic	m_nxstaticCustomCheck3Label;
	CNxStatic	m_nxstaticCustomCheck4Label;
	CNxStatic	m_nxstaticCustomCheck5Label;
	CNxStatic	m_nxstaticCustomCheck6Label;
	CNxStatic	m_nxstaticCustomDate1Label;
	CNxStatic	m_nxstaticCustomDate2Label;
	CNxStatic	m_nxstaticCustomDate3Label;
	CNxStatic	m_nxstaticCustomDate4Label;
	// (j.armen 2011-06-22 11:05) - PLID 11490 - labels used when multiple custom items are selected
	CNxLabel	m_nxlCustomMultiList1Label;
	CNxLabel	m_nxlCustomMultiList2Label;
	CNxLabel	m_nxlCustomMultiList3Label;
	CNxLabel	m_nxlCustomMultiList4Label;
	CNxLabel	m_nxlCustomMultiList5Label;
	CNxLabel	m_nxlCustomMultiList6Label;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustom1Dlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void EditCustomList(NXDATALISTLib::_DNxDataListPtr &list, long listID) ;
	virtual void SecureControls();
	virtual BOOL IsEditBox(CWnd* pWnd);
	void UpdateLabel(UINT nID, const CString &strNewLabelText);

	// (a.walling 2010-10-13 10:44) - PLID 40908 - Dead code
	//CString LabelToField(int nID);
	//COleDateTime m_dtDateTmp;
	//CString m_strNewDataTmp;
	//bool m_bAutoRefreshing;	
	//bool m_bSettingBox;	
	//CString m_strCurDataTmp;
	long m_id;

	NXTIMELib::_DNxTimePtr m_nxtDate1, m_nxtDate2, m_nxtDate3, m_nxtDate4;
	COleDateTime m_dtDate1, m_dtDate2, m_dtDate3, m_dtDate4;
	bool m_bSavingDate1, m_bSavingDate2, m_bSavingDate3, m_bSavingDate4;

	// Generated message map functions
	//{{AFX_MSG(CCustom1Dlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnCustomCheck1();
	afx_msg void OnCustomCheck2();
	afx_msg void OnCustomCheck3();
	afx_msg void OnCustomCheck4();
	afx_msg void OnCustomCheck5();
	afx_msg void OnCustomCheck6();
	afx_msg void OnFocusChangeCustomDate1(BSTR FAR* FocusDate, BSTR FAR* OldFocusDate, short FAR* MonthNum, short FAR* YearNum, short FAR* DayNum);
	afx_msg void OnFocusChangeCustomDate2(BSTR FAR* FocusDate, BSTR FAR* OldFocusDate, short FAR* MonthNum, short FAR* YearNum, short FAR* DayNum);
	afx_msg void OnFocusChangeCustomDate3(BSTR FAR* FocusDate, BSTR FAR* OldFocusDate, short FAR* MonthNum, short FAR* YearNum, short FAR* DayNum);
	afx_msg void OnFocusChangeCustomDate4(BSTR FAR* FocusDate, BSTR FAR* OldFocusDate, short FAR* MonthNum, short FAR* YearNum, short FAR* DayNum);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam); // (j.armen 2011-06-22 11:06) - PLID 11490
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message); // (j.armen 2011-06-22 11:06) - PLID 11490
	afx_msg void OnEditCustomList1();
	afx_msg void OnEditCustomList2();
	afx_msg void OnEditCustomList3();
	afx_msg void OnEditCustomList4();
	afx_msg void OnEditCustomList5();
	afx_msg void OnEditCustomList6();
	afx_msg void OnSelChosenCustomList1(long nRow);
	afx_msg void OnSelChosenCustomList2(long nRow);
	afx_msg void OnSelChosenCustomList3(long nRow);
	afx_msg void OnSelChosenCustomList4(long nRow);
	afx_msg void OnSelChosenCustomList5(long nRow);
	afx_msg void OnSelChosenCustomList6(long nRow);
	afx_msg void OnSelChosenCustomContact1(long nRow);
	afx_msg void OnKillFocusCustomDate1();
	afx_msg void OnKillFocusCustomDate2();
	afx_msg void OnKillFocusCustomDate3();
	afx_msg void OnKillFocusCustomDate4();
	afx_msg void OnGotoCustomContact();
	afx_msg void OnRequeryFinishedCustomContact1(short nFlags);
	afx_msg void OnTrySetSelFinishedCustomContact1(long nRowEnum, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void AddSelectRecordCustomCombo(NXDATALISTLib::_DNxDataListPtr* ctrl, long listid, long resid);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOM1DLG_H__712CC0B5_05E3_11D2_8087_00104B2FE914__INCLUDED_)
