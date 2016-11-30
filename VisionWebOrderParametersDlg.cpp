// VisionWebOrderParametersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InventoryRc.h"
#include "VisionWebOrderParametersDlg.h"
#include "VisionWebOrderDlg.h"

//TES 12/7/2010 - PLID 41715 - Created
// CVisionWebOrderParametersDlg dialog

IMPLEMENT_DYNAMIC(CVisionWebOrderParametersDlg, CNxDialog)

CVisionWebOrderParametersDlg::CVisionWebOrderParametersDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CVisionWebOrderParametersDlg::IDD, pParent)
{
	m_pOrderDlg = NULL;
}

CVisionWebOrderParametersDlg::~CVisionWebOrderParametersDlg()
{
}

void CVisionWebOrderParametersDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVisionWebOrderParametersDlg)
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
}


BEGIN_MESSAGE_MAP(CVisionWebOrderParametersDlg, CNxDialog)
END_MESSAGE_MAP()

enum ParamListColumns {
	plcID = 0,
	plcName = 1,
	plcValue = 2,
	plcDisplayOrder = 3,
	plcArrayPosition = 4,
};

// CVisionWebOrderParametersDlg message handlers
using namespace NXDATALIST2Lib;
BOOL CVisionWebOrderParametersDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		//TES 12/7/2010 - PLID 41715 - Set up our controls.
		//NxIconify
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		m_pParamList = BindNxDataList2Ctrl(IDC_VISIONWEB_PARAMETERS, false);

		//TES 12/8/2010 - PLID 41715 - Our job is to update our parent's member variables, so we must have one.
		if(!m_pOrderDlg) {
			ASSERT(FALSE);
			CNxDialog::OnCancel();
			return TRUE;
		}

		//TES 12/8/2010 - PLID 41715 - Go through the parameter list and fill in our rows.
		for(int i = 0; i < m_pOrderDlg->m_arCustomParams.GetSize(); i++) {
			VisionWebCustomParam vwcp = m_pOrderDlg->m_arCustomParams[i];
			IRowSettingsPtr pRow = m_pParamList->GetNewRow();
			pRow->PutValue(plcID, _bstr_t(vwcp.strID));
			CString strName = CString(vwcp.bIsRequired?"*":"") + vwcp.strName;
			pRow->PutValue(plcName, _bstr_t(strName));
			//TES 12/8/2010 - PLID 41715 - Does our parent have a value for this parameter?
			CString strValue;
			m_pOrderDlg->m_mapParamValues.Lookup(vwcp.strID, strValue);
			pRow->PutValue(plcValue, _bstr_t(strValue));
			pRow->PutValue(plcDisplayOrder, vwcp.nDisplayOrder);
			pRow->PutValue(plcArrayPosition, (long)i);
			m_pParamList->AddRowSorted(pRow, NULL);
		}

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVisionWebOrderParametersDlg::OnOK()
{
	try {
		//TES 12/8/2010 - PLID 41715 - First, check whether any required fields are blank.
		IRowSettingsPtr pRow = m_pParamList->GetFirstRow();
		bool bMissingRequiredField = false;
		while(pRow && !bMissingRequiredField) {
			long nArrayPos = VarLong(pRow->GetValue(plcArrayPosition));
			VisionWebCustomParam vwcp = m_pOrderDlg->m_arCustomParams[nArrayPos];
			if(vwcp.bIsRequired) {
				if(VarString(pRow->GetValue(plcValue),"").IsEmpty()) {
					bMissingRequiredField = true;
				}
			}
			pRow = pRow->GetNextRow();
		}
		if(bMissingRequiredField) {
			if(IDYES != MsgBox(MB_YESNO, "At least one required value has not been entered.  Are you sure you wish to continue?")) {
				return;
			}
		}

		//TES 12/8/2010 - PLID 41715 - Go through and pull all the values that were filled in.
		pRow = m_pParamList->GetFirstRow();
		while(pRow) {
			CString strID = VarString(pRow->GetValue(plcID));
			m_pOrderDlg->m_mapParamValues.SetAt(strID, VarString(pRow->GetValue(plcValue),""));
			pRow = pRow->GetNextRow();
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}BEGIN_EVENTSINK_MAP(CVisionWebOrderParametersDlg, CNxDialog)
ON_EVENT(CVisionWebOrderParametersDlg, IDC_VISIONWEB_PARAMETERS, 9, CVisionWebOrderParametersDlg::OnEditingFinishingVisionwebParameters, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

void CVisionWebOrderParametersDlg::OnEditingFinishingVisionwebParameters(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		if(nCol != plcValue) {
			return;
		}
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//TES 2/16/2011 - PLID 41715 - Don't validate if they're not committing.
		if(!(*pbCommit)) {
			return;
		}

		CString strValue = strUserEntered;
		bool bIsValidValue = false;
		long nArrayPos = VarLong(pRow->GetValue(plcArrayPosition));
		VisionWebCustomParam vwcp = m_pOrderDlg->m_arCustomParams[nArrayPos];
		//TES 12/8/2010 - PLID 41715 - Determine the type
		if(vwcp.dIncrementValue == INVALID_FLOAT_VALUE) {
			//TES 12/8/2010 - PLID 41715 - This is a string, just truncate it to the specified max length.
			if(vwcp.nMaxLength != -1) {
				strValue = strValue.Left(vwcp.nMaxLength);
				VariantClear(pvarNewValue);
				*pvarNewValue = _variant_t(strValue).Detach();
			}
		}
		else if(vwcp.dIncrementValue == 1) {
			//TES 12/8/2010 - PLID 41715 - This needs to be an int.
			if(strValue.SpanIncluding("0123456789-").GetLength() == strValue.GetLength()) {
				long nValue = atol(strValue);
				if(nValue >= vwcp.dMinValue && nValue <= vwcp.dMaxValue) {
					bIsValidValue = true;
				}
			}
			if(!bIsValidValue) {
				MsgBox("This value must be an integer between %.0f and %.0f.", vwcp.dMinValue, vwcp.dMaxValue);
				*pbCommit = FALSE;
				*pbContinue = FALSE;
			}
		}
		else {
			//TES 12/8/2010 - PLID 41715 - The only increments we know of are 1 (int) and 0.1 (float)
			ASSERT(vwcp.dIncrementValue == 0.1);
			if(strValue.SpanIncluding("0123456789-.").GetLength() == strValue.GetLength()) {
				double dValue = atof(strValue);
				if(dValue >= vwcp.dMinValue && dValue <= vwcp.dMaxValue) {
					bIsValidValue = true;
					//TES 12/8/2010 - PLID 41715 - Truncate to no more than one digit after the decimal.
					int nDecimal = strValue.Find(".");
					if(nDecimal != -1) {
						strValue = strValue.Left(nDecimal+2);
						*pvarNewValue = _variant_t(strValue).Detach();
					}
				}
			}
			if(!bIsValidValue) {
				MsgBox("This value must be a decimal number between %.1f and %.1f.", vwcp.dMinValue, vwcp.dMaxValue);
				*pbCommit = FALSE;
				*pbContinue = FALSE;
			}
		}
	}NxCatchAll(__FUNCTION__);
}
