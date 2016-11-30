#pragma once


// (z.manning 2010-09-15 12:11) - PLID 40319 - This is the operand that's used in place of the cost
// in the markup formula (i.e. if the formula is "Cost * 2" then "Cost" will be replaced with the
// wholesale cost of the product.
#define MARKUP_COST_OPERAND "Cost"


// CMarkupFormulaEditDlg dialog
// (z.manning 2010-09-15 12:10) - PLID 40319 - Created

class CMarkupFormulaEditDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CMarkupFormulaEditDlg)

public:
	CMarkupFormulaEditDlg(CWnd* pParent);   // standard constructor
	virtual ~CMarkupFormulaEditDlg();

// Dialog Data
	enum { IDD = IDD_MARKUP_FORMULA_EDITOR };
	virtual BOOL OnInitDialog();
	virtual void OnOK();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// (b.eyers 2016-03-14) - PLID 68590 - added a dropdown for round up options for prices
	NXDATALIST2Lib::_DNxDataListPtr m_pRoundUpRule; 
	enum RoundUpRuleColumns {
		rrcID = 0,
		rrcName,
	};
	//since these should never change, but hey more might be added one day for some reason
	//this is also in invutils.h
	enum RoundUpRules {
		NoRoundUpRule = 0,
		RoundUpNearestDollar = 1,
		RoundUpToNine = 2, 
	};

	NXDATALIST2Lib::_DNxDataListPtr m_pdlMarkupCombo;
	enum MarkupComboColumns {
		mccID = 0,
		mccName,
		mccFormula,
		mccRoundUpRule,
	};

	LPDISPATCH m_lpLastChosenRow;

	void Load();
	BOOL ValidateAndSave(LPDISPATCH lpRowToSave);

	BOOL IsNew(LPDISPATCH lpRow);
	BOOL HasRowChanged(LPDISPATCH lpRow);

	CString GetFormula();

	CNxColor m_nxcolor;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnNew;
	CNxIconButton m_btnDelete;

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void RequeryFinishedMarkupList(short nFlags);
	void SelChosenMarkupList(LPDISPATCH lpRow);
	afx_msg void OnBnClickedNewMarkup();
	afx_msg void OnBnClickedDeleteMarkup();
	afx_msg void OnBnClickedMarkupHelp();
	afx_msg void OnBnClickedRoundupHelp(); // (b.eyers 2016-03-15) - PLID 68590
};
