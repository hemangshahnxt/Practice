#pragma once

//TES 12/7/2010 - PLID 41715 - Created
// CVisionWebOrderParametersDlg dialog

//TES 12/8/2010 - PLID 41715 - Struct representing a record from VisionWebCustomParameterT
struct VisionWebCustomParam {
	CString strID;
	long nDisplayOrder;
	BOOL bIsRequired;
	long nMaxLength;
	double dMinValue;
	double dMaxValue;
	CString strDefaultValue;
	double dIncrementValue;
	long nPrecisionValue;
	long nScaleValue;
	CString strEyeSite;
	CString strName;
};

#define INVALID_FLOAT_VALUE	-12345.67
class CVisionWebOrderParametersDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CVisionWebOrderParametersDlg)

public:
	CVisionWebOrderParametersDlg(CWnd* pParent);   // standard constructor
	virtual ~CVisionWebOrderParametersDlg();

	//TES 12/8/2010 - PLID 41715 - Our parent, our job is to update its member variables.
	CVisionWebOrderDlg *m_pOrderDlg;

// Dialog Data
	enum { IDD = IDD_VISIONWEB_ORDER_PARAMETERS_DLG };

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pParamList;

	CNxIconButton m_nxbOK, m_nxbCancel;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void OnOK();
public:
	DECLARE_EVENTSINK_MAP()
	void OnEditingFinishingVisionwebParameters(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};
