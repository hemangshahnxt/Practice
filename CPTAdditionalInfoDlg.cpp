// CPTAdditionalInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "CPTAdditionalInfoDlg.h"

// (a.wilson 2014-01-13 15:53) - PLID 59956 - Created.
// CCPTAdditionalInfoDlg dialog

enum CCDAProcedureTypeColumn
{
	ptcID = 0,
	ptcName = 1,
};

IMPLEMENT_DYNAMIC(CCPTAdditionalInfoDlg, CNxDialog)

CCPTAdditionalInfoDlg::CCPTAdditionalInfoDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCPTAdditionalInfoDlg::IDD, pParent)
{
	m_eCCDAType = cptInvalidType;
}

CCPTAdditionalInfoDlg::~CCPTAdditionalInfoDlg()
{}

void CCPTAdditionalInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CPT_TOS, m_editTOS);
	DDX_Control(pDX, IDCANCEL, m_nxClose);
	DDX_Control(pDX, IDC_CPT_ADDITIONAL_INFO_COLOR, m_nxBackground);
}

BEGIN_MESSAGE_MAP(CCPTAdditionalInfoDlg, CNxDialog)
	ON_BN_CLICKED(IDCANCEL, &CCPTAdditionalInfoDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL CCPTAdditionalInfoDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//Close
		m_nxClose.AutoSet(NXB_CLOSE);

		//Color Background
		m_nxBackground.SetColor(GetNxColor(GNC_ADMIN, 0));

		//TOS
		m_editTOS.SetLimitText(2);
		SetDlgItemText(IDC_CPT_TOS, m_strTOS);

		//CCDA Type
		m_pCCDAType = BindNxDataList2Ctrl(IDC_CPT_CCDATYPE, false);
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCCDAType->GetNewRow();
			pRow->PutValue(ptcID, (long)cptInvalidType);
			pRow->PutValue(ptcName, _bstr_t("< No Selection >"));
			m_pCCDAType->AddRowAtEnd(pRow, NULL);
			pRow = m_pCCDAType->GetNewRow();
			pRow->PutValue(ptcID, (long)cptProcedure);
			pRow->PutValue(ptcName, _bstr_t("Procedure"));
			m_pCCDAType->AddRowAtEnd(pRow, NULL);
			pRow = m_pCCDAType->GetNewRow();
			pRow->PutValue(ptcID, (long)cptObservation);
			pRow->PutValue(ptcName, _bstr_t("Observation"));
			m_pCCDAType->AddRowAtEnd(pRow, NULL);
			pRow = m_pCCDAType->GetNewRow();
			pRow->PutValue(ptcID, (long)cptAct);

			pRow->PutValue(ptcName, _bstr_t("Act"));
			m_pCCDAType->AddRowAtEnd(pRow, NULL);
			pRow = m_pCCDAType->GetNewRow();
			pRow->PutValue(ptcID, (long)cptEncounter);
			pRow->PutValue(ptcName, _bstr_t("Encounter"));
			m_pCCDAType->AddRowAtEnd(pRow, NULL);
		}
		if (!m_pCCDAType->SetSelByColumn(ptcID, (long)m_eCCDAType))
			m_pCCDAType->SetSelByColumn(ptcID, (long)cptInvalidType);

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (a.wilson 2014-01-14 12:28) - PLID 59956 - update member variables on close of dialog.
void CCPTAdditionalInfoDlg::OnBnClickedCancel()
{
	try {
		//TOS
		GetDlgItemText(IDC_CPT_TOS, m_strTOS);

		//CCDA Type
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCCDAType->GetCurSel();
		if (pRow) {
			m_eCCDAType = (CCDAProcedureType)VarLong(pRow->GetValue(ptcID), -1);
		}
	} NxCatchAll(__FUNCTION__);

	CNxDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CCPTAdditionalInfoDlg, CNxDialog)
	ON_EVENT(CCPTAdditionalInfoDlg, IDC_CPT_CCDATYPE, 1, CCPTAdditionalInfoDlg::SelChangingCptCcdatype, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

// (a.wilson 2014-01-14 12:28) - PLID 59956 - prevent null row selection.
void CCPTAdditionalInfoDlg::SelChangingCptCcdatype(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__);
}
