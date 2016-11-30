// EmrProviderConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "EmrProviderConfigDlg.h"
#include "AuditTrail.h"
#include <SharedEmrUtils.h>

using namespace NXDATALIST2Lib;

// CEmrProviderConfigDlg dialog
// (z.manning 2011-01-31 09:36) - PLID 42334 - Created

IMPLEMENT_DYNAMIC(CEmrProviderConfigDlg, CNxDialog)

CEmrProviderConfigDlg::CEmrProviderConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrProviderConfigDlg::IDD, pParent)
{

}

CEmrProviderConfigDlg::~CEmrProviderConfigDlg()
{
}

// (a.walling 2011-06-30 12:38) - PLID 44388 - Clear this out in OnDestroy rather than the destructor
void CEmrProviderConfigDlg::OnDestroy()
{
	try {
		if (m_pdlProviderCombo) {
			for(IRowSettingsPtr pRow = m_pdlProviderCombo->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
			{
				EmrProviderData *pProv = (EmrProviderData*)VarLong(pRow->GetValue(pccNewPointer));
				if(pProv != NULL) {
					delete pProv;
				}
			}
		}
	} NxCatchAll(__FUNCTION__);

	CNxDialog::OnDestroy();
}


void CEmrProviderConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EMR_PROVIDER_CONFIG_BACKGROUND, m_nxcolor);
	DDX_Control(pDX, IDC_EMR_PROVIDER_CONFIG_BACKGROUND2, m_nxcolor2);
	DDX_Control(pDX, IDC_EMR_PROVIDER_MANUAL_UPDATE, m_btnUpdateProviderFloatingSelections);
}


BEGIN_MESSAGE_MAP(CEmrProviderConfigDlg, CNxDialog)
	ON_BN_CLICKED(IDC_EMR_PROVIDER_FLOAT_CHECK, &CEmrProviderConfigDlg::OnBnClickedEmrProviderFloatCheck)
	ON_EN_CHANGE(IDC_EMR_PROVIDER_FLOAT_COUNT, &CEmrProviderConfigDlg::OnEnChangeEmrProviderFloatCount)
	ON_EN_CHANGE(IDC_EMR_PROVIDER_FLOAT_DAYS, &CEmrProviderConfigDlg::OnEnChangeEmrProviderFloatDays)
	ON_BN_CLICKED(IDC_EMR_PROVIDER_MANUAL_UPDATE, &CEmrProviderConfigDlg::OnBnClickedEmrProviderManualUpdate)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CEmrProviderConfigDlg, CNxDialog)
ON_EVENT(CEmrProviderConfigDlg, IDC_EMR_PROVIDER_CONFIG_COMBO, 18, CEmrProviderConfigDlg::RequeryFinishedEmrProviderConfigCombo, VTS_I2)
ON_EVENT(CEmrProviderConfigDlg, IDC_EMR_PROVIDER_CONFIG_COMBO, 16, CEmrProviderConfigDlg::SelChosenEmrProviderConfigCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()


// CEmrProviderConfigDlg message handlers

BOOL CEmrProviderConfigDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		// (a.walling 2011-06-30 12:38) - PLID 44388 - Bind the combo first thing (auto requery is false anyway)
		// Otherwise if we returned out of here, an exception would be thrown from the destructor when it tries to access the 
		// datalist. That should be done in the WM_DESTROY handler anyway.
		m_pdlProviderCombo = BindNxDataList2Ctrl(IDC_EMR_PROVIDER_CONFIG_COMBO, false);

		CDWordArray arydwLicensedProviders;
		g_pLicense->GetUsedEMRProviders(arydwLicensedProviders);
		if(arydwLicensedProviders.GetSize() == 0) {
			MessageBox("You do not have any licensed EMR providers.", NULL, MB_OK|MB_ICONINFORMATION);
			EndDialog(IDCANCEL);
			return TRUE;
		}

		m_nxcolor.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_nxcolor2.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnUpdateProviderFloatingSelections.AutoSet(NXB_MODIFY); // (z.manning 2011-04-19 17:05) - PLID 42337

		((CEdit*)GetDlgItem(IDC_EMR_PROVIDER_FLOAT_COUNT))->SetLimitText(6);
		((CEdit*)GetDlgItem(IDC_EMR_PROVIDER_FLOAT_DAYS))->SetLimitText(6);

		CString strProviderWhere = FormatString("PersonT.Archived = 0 AND PersonT.ID IN (%s)", ArrayAsString(arydwLicensedProviders, false));
		m_pdlProviderCombo->PutWhereClause(_bstr_t(strProviderWhere));
		m_pdlProviderCombo->Requery();

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CEmrProviderConfigDlg::EnableControls(BOOL bEnable)
{
	GetDlgItem(IDC_EMR_PROVIDER_FLOAT_CHECK)->EnableWindow(bEnable);

	IRowSettingsPtr pRow = m_pdlProviderCombo->GetCurSel();
	BOOL bEnableFloatFields = (bEnable && (IsDlgButtonChecked(IDC_EMR_PROVIDER_FLOAT_CHECK) == BST_CHECKED));
	GetDlgItem(IDC_EMR_PROVIDER_FLOAT_COUNT)->EnableWindow(bEnableFloatFields);
	GetDlgItem(IDC_EMR_PROVIDER_FLOAT_DAYS)->EnableWindow(bEnableFloatFields);
}

void CEmrProviderConfigDlg::RequeryFinishedEmrProviderConfigCombo(short nFlags)
{
	try
	{
		if(m_pdlProviderCombo->GetRowCount() > 0)
		{
			m_pdlProviderCombo->PutCurSel(m_pdlProviderCombo->GetTopRow());
			Load();
			EnableControls(TRUE);
		}

	}NxCatchAll(__FUNCTION__);
}

void CEmrProviderConfigDlg::SelChosenEmrProviderConfigCombo(LPDISPATCH lpRow)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			EnableControls(FALSE);
			CheckDlgButton(IDC_EMR_PROVIDER_FLOAT_CHECK, BST_UNCHECKED);
			SetDlgItemText(IDC_EMR_PROVIDER_FLOAT_COUNT, "");
			SetDlgItemText(IDC_EMR_PROVIDER_FLOAT_DAYS, "");
		}
		else {
			Load();
			EnableControls(TRUE);
		}

	}NxCatchAll(__FUNCTION__);
}

void CEmrProviderConfigDlg::Load()
{
	IRowSettingsPtr pRow = m_pdlProviderCombo->GetCurSel();
	if(pRow == NULL) {
		return;
	}

	EmrProviderData prov;
	EmrProviderData *pProvNew = (EmrProviderData*)VarLong(pRow->GetValue(pccNewPointer));
	// (z.manning 2011-01-31 11:42) - If they already changed something for the newly selected provider
	// then load from the new data. Otherwise just load the existing values from data (via the datalist).
	if(pProvNew == NULL) {
		GetDataFromRow(pRow, &prov);
	}
	else {
		prov = *pProvNew;
	}

	CheckDlgButton(IDC_EMR_PROVIDER_FLOAT_CHECK, prov.bFloat ? BST_CHECKED : BST_UNCHECKED);
	SetDlgItemInt(IDC_EMR_PROVIDER_FLOAT_COUNT, prov.nFloatCount);
	SetDlgItemInt(IDC_EMR_PROVIDER_FLOAT_DAYS, prov.nFloatDays);
}

void CEmrProviderConfigDlg::GetDataFromRow(LPDISPATCH lpRow, OUT EmrProviderData *pProvData)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow != NULL)
	{
		pProvData->bFloat = VarBool(pRow->GetValue(pccFloat));
		pProvData->nFloatCount = VarLong(pRow->GetValue(pccFloatCount));
		pProvData->nFloatDays = VarLong(pRow->GetValue(pccFloatDays));
	}
}

void CEmrProviderConfigDlg::OnOK()
{
	try
	{
		if(ValidateAndSave()) {
			CNxDialog::OnOK();
		}

	}NxCatchAll(__FUNCTION__);
}

BOOL CEmrProviderConfigDlg::ValidateAndSave()
{
	CParamSqlBatch sqlBatch;
	CAuditTransaction auditTran;

	for(IRowSettingsPtr pRow = m_pdlProviderCombo->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		const long nProvID = VarLong(pRow->GetValue(pccID));
		const CString strProvName = VarString(pRow->GetValue(pccName), "");
		EmrProviderData *pProvNew = (EmrProviderData*)VarLong(pRow->GetValue(pccNewPointer));
		if(pProvNew != NULL)
		{
			EmrProviderData provOld;
			GetDataFromRow(pRow, &provOld);
			if(provOld != *pProvNew)
			{
				if(pProvNew->bFloat)
				{
					if(pProvNew->nFloatCount <= 0) {
						MessageBox("Please enter a value greater than zero for the count.", NULL, MB_OK|MB_ICONERROR);
						m_pdlProviderCombo->SetSelByColumn(pccID, nProvID);
						Load();
						return FALSE;
					}
					if(pProvNew->nFloatDays <= 0) {
						MessageBox("Please enter a value greater than zero for the number of days.", NULL, MB_OK|MB_ICONERROR);
						m_pdlProviderCombo->SetSelByColumn(pccID, nProvID);
						Load();
						return FALSE;
					}
				}

				sqlBatch.Add(
					"UPDATE ProvidersT SET \r\n"
					"	FloatEmrData = {BIT} \r\n"
					"	, EmrFloatCount = {INT} \r\n"
					"	, EmrFloatDays = {INT} \r\n"
					"WHERE ProvidersT.PersonID = {INT} "
					, pProvNew->bFloat, pProvNew->nFloatCount, pProvNew->nFloatDays, nProvID);

				if(provOld.bFloat != pProvNew->bFloat) {
					CString strOld = provOld.bFloat ? "Yes" : "No";
					CString strNew = pProvNew->bFloat ? "Yes" : "No";
					AuditEvent(-1, strProvName, auditTran, aeiEmrProviderFloat, nProvID, strOld, strNew, aepMedium, aetChanged);
				}
				if(provOld.nFloatCount != pProvNew->nFloatCount) {
					CString strOld = AsString(provOld.nFloatCount);
					CString strNew = AsString(pProvNew->nFloatCount);
					AuditEvent(-1, strProvName, auditTran, aeiEmrProviderFloatCount, nProvID, strOld, strNew, aepMedium, aetChanged);
				}
				if(provOld.nFloatDays != pProvNew->nFloatDays) {
					CString strOld = AsString(provOld.nFloatDays);
					CString strNew = AsString(pProvNew->nFloatDays);
					AuditEvent(-1, strProvName, auditTran, aeiEmrProviderFloatDays, nProvID, strOld, strNew, aepMedium, aetChanged);
				}
			}
		}
	}

	if(!sqlBatch.IsEmpty()) {
		sqlBatch.Execute(GetRemoteData());
	}
	auditTran.Commit();

	return TRUE;
}

void CEmrProviderConfigDlg::HandleProviderChange()
{
	IRowSettingsPtr pRow = m_pdlProviderCombo->GetCurSel();
	if(pRow == NULL) {
		return;
	}

	EmrProviderData *pProv = (EmrProviderData*)VarLong(pRow->GetValue(pccNewPointer));
	if(pProv == NULL) {
		pProv = new EmrProviderData();
		pRow->PutValue(pccNewPointer, (long)pProv);
	}

	pProv->bFloat = IsDlgButtonChecked(IDC_EMR_PROVIDER_FLOAT_CHECK) == BST_CHECKED ? TRUE : FALSE;
	pProv->nFloatCount = GetDlgItemInt(IDC_EMR_PROVIDER_FLOAT_COUNT);
	pProv->nFloatDays = GetDlgItemInt(IDC_EMR_PROVIDER_FLOAT_DAYS);
}

void CEmrProviderConfigDlg::OnBnClickedEmrProviderFloatCheck()
{
	try
	{
		HandleProviderChange();
		EnableControls(TRUE);

	}NxCatchAll(__FUNCTION__);
}

void CEmrProviderConfigDlg::OnEnChangeEmrProviderFloatCount()
{
	try
	{
		HandleProviderChange();

	}NxCatchAll(__FUNCTION__);
}

void CEmrProviderConfigDlg::OnEnChangeEmrProviderFloatDays()
{
	try
	{
		HandleProviderChange();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-04-19 16:47) - PLID 42337
void CEmrProviderConfigDlg::OnBnClickedEmrProviderManualUpdate()
{
	try
	{
		int nMsgResult = MessageBox("Are you sure you want to manually update the most common selections for all EMR providers?", NULL, MB_YESNO|MB_ICONQUESTION);
		if(nMsgResult == IDYES) {
			CWaitCursor wc;
			::UpdatePerProviderFloatingEmrSelections(GetRemoteData());
			MessageBox("Update Successful!");
		}

	}NxCatchAll(__FUNCTION__);
}