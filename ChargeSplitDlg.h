#pragma once
#include "BillingRc.h"


// (j.dinatale 2012-01-05 17:19) - PLID 47341 - Created

// (j.jones 2012-01-26 11:09) - PLID 47700 - added a modular function for getting the responsibility dropdown
// combo box source, so it can be shared across multiple dialogs that assign resps to charges
CString GetEMRChargeRespAssignComboBoxSource(long nPatientID, OPTIONAL long nEMNID = -1);

// (j.dinatale 2012-01-12 09:38) - PLID 47483 - created an EMNChargeSummary object to pass along to this dialog
class EMNChargeSummary{
public:
	long nID;
	CString strDescription;	
	double dblQuantity;
	COleCurrency cyUnitCost;
	CString strSubCode;
	CString strCode;
	long nInsuredPartyID;
	BOOL bBillable;

	EMNChargeSummary() {
		nID = -1;
		dblQuantity = 0.0;
		cyUnitCost = COleCurrency(0,0);
		bBillable = TRUE;	
		nInsuredPartyID = -2;	
	}
};

// (j.dinatale 2012-01-12 10:49) - PLID 47483 - enum to determine what type we are passing to the charge split dlg
namespace ChrgSptDlgEnums{
	enum SupportedTypes{
		EMNCharge = 0,
		EMNChargeSummary = 1,
	};
};

// CChargeSplitDlg dialog
enum ChgSplitDlgCol{
	csdcID = 0,
	csdcCode = 1,
	csdcSubCode = 2,
	csdcDescription = 3,
	csdcQuantity = 4,
	csdcUnitCost = 5,
	csdcOldInsPartyID = 6,
	csdcNewInsPartyID = 7,
};

class CChargeSplitDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CChargeSplitDlg)

public:
	// (j.dinatale 2012-01-12 11:00) - PLID 47483
	CChargeSplitDlg(ChrgSptDlgEnums::SupportedTypes stPtrType, CArray<long,long> &arypChargeObjects, long nPatientID, long nEMNID, BOOL bReadOnly, CWnd* pParent = NULL);   // standard constructor
	virtual ~CChargeSplitDlg();

	void SetColor(long clrBackcolor);

	// (j.dinatale 2012-01-12 14:53) - PLID 47483 - need to record if dialog has changes
	bool ChargesChanged();

// Dialog Data
	enum { IDD = IDD_CHARGE_SPLIT_DLG };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Buttons on the form
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxColor m_NxColor;

	// (j.dinatale 2012-01-12 14:53) - PLID 47483 - need to record if dialog has changes
	bool m_bHasChanges;

	BOOL m_bReadOnly;
	ChrgSptDlgEnums::SupportedTypes m_stPtrType;

	CArray<long,long> m_arypChargeObjects;
	long m_nPatientID;
	long m_nEMNID;

	NXDATALIST2Lib::_DNxDataListPtr m_pChargeList;
	void SetupList();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnDestroy();
};
