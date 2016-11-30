#if !defined(AFX_CHOOSEDRAGRESPDLG_H__F226C250_C394_46C9_AF7A_5ADB55C686E3__INCLUDED_)
#define AFX_CHOOSEDRAGRESPDLG_H__F226C250_C394_46C9_AF7A_5ADB55C686E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseDragRespDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseDragRespDlg dialog

class CChooseDragRespDlg : public CNxDialog
{
// Construction
public:
	CChooseDragRespDlg(CWnd* pParent);   // standard constructor

	//variables to be returned filled
	long m_nRespTypeID;		//RespTypeT.ID of the chosen responsibility
	long m_nInsPartyID;		//InsuredPartyT.PersonID of the chosen resp.

	//setup variables (filled by the calling function)
	long m_nTargetType;		//1 = We are asking for the source, 2 = we are asking for the destination, 3 = we are asking to apply a payment
	CString m_strLineType;		//"Bill" or "Charge"
	long m_nLineID;			//id of the line (either BillID or ChargeID per above)

	// (j.jones 2010-06-18 14:32) - PLID 39150 - take in an optional comma delimited list of RespTypeIDs to ignore,
	// to be appended to the existing ignored list of IDs 1 and 2
	CString m_strRespTypeIDsToIgnore;

// Dialog Data
	//{{AFX_DATA(CChooseDragRespDlg)
	enum { IDD = IDD_CHOOSE_DRAG_RESP_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticTargetText;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseDragRespDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pResps;

	// Generated message map functions
	//{{AFX_MSG(CChooseDragRespDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRequeryFinishedResponsibilityList(short nFlags);
	afx_msg void OnDblClickCellResponsibilityList(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSEDRAGRESPDLG_H__F226C250_C394_46C9_AF7A_5ADB55C686E3__INCLUDED_)
