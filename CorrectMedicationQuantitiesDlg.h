#pragma once
// (d.thompson 2009-03-12) - PLID 33482

// CCorrectMedicationQuantitiesDlg dialog

class CCorrectMedicationQuantitiesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CCorrectMedicationQuantitiesDlg)

public:
	CCorrectMedicationQuantitiesDlg(CWnd* pParent);   // standard constructor
	virtual ~CCorrectMedicationQuantitiesDlg();

	//Incoming
	long m_nDrugListID;		//The DrugList.ID that we're updating
	//Output
	CString m_strOutputDefQty;		//DrugList.DefaultQuantity after updating
	CString m_strOutputQtyUnit;		//DrugList.QuantityUnitID matched to its name in DrugStrengthNamesT
	// (j.fouts 2013-03-19 12:06) - PLID 55740 - Added Quantity Unit ID
	long m_nOutputQtyUnitID;		//DrugList.QuantityUnitID

// Dialog Data
	enum { IDD = IDD_CORRECT_MEDICATION_QUANTITIES_DLG };

protected:
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxEdit			m_nxeditDefQty;

	NXDATALIST2Lib::_DNxDataListPtr m_pList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual	BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
};
