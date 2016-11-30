// EmrItemSelectStampsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "EmrItemSelectStampsDlg.h"
#include "EmnDetailStructures.h"
#include "SharedEmrUtils.h"

using namespace NXDATALIST2Lib;

// CEmrItemSelectStampsDlg dialog
// (z.manning 2011-10-24 10:21) - PLID 46082 - Created

IMPLEMENT_DYNAMIC(CEmrItemSelectStampsDlg, CNxDialog)

CEmrItemSelectStampsDlg::CEmrItemSelectStampsDlg(CEmrItemStampExclusions *pStampExclusions, CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrItemSelectStampsDlg::IDD, pParent)
	, m_pStampExclusions(pStampExclusions)
{

}

CEmrItemSelectStampsDlg::~CEmrItemSelectStampsDlg()
{
}

void CEmrItemSelectStampsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EMR_ITEM_SELECT_STAMP_BACKGROUND, m_nxcolor);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CEmrItemSelectStampsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_EMR_ITEM_STAMP_SELECT_ALL, &CEmrItemSelectStampsDlg::OnBnClickedEmrItemStampSelectAll)
	ON_BN_CLICKED(IDC_EMR_ITEM_STAMP_SELECT_NONE, &CEmrItemSelectStampsDlg::OnBnClickedEmrItemStampSelectNone)
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CEmrItemSelectStampsDlg, CNxDialog)
END_EVENTSINK_MAP()


// CEmrItemSelectStampsDlg message handlers

BOOL CEmrItemSelectStampsDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_pdlStamps = BindNxDataList2Ctrl(IDC_EMR_ITEM_SELECT_STAMP_LIST, false);
		m_nxcolor.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		Load();
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CEmrItemSelectStampsDlg::Load()
{
	try
	{
		// (z.manning 2011-10-24 12:11) - PLID 46082 - Go through and uncheck all the stamps that are excluded.
		GetMainFrame()->LoadEMRImageStamps();
		for(int nStampIndex = 0; nStampIndex < GetMainFrame()->m_aryEMRImageStamps.GetCount(); nStampIndex++)
		{
			EMRImageStamp *pStamp = GetMainFrame()->m_aryEMRImageStamps.GetAt(nStampIndex);
			if(!pStamp->bInactive)
			{
				IRowSettingsPtr pRow = m_pdlStamps->GetNewRow();
				pRow->PutValue(slcCheck, m_pStampExclusions->IsExcluded(pStamp->nID) ? g_cvarFalse : g_cvarTrue);
				pRow->PutValue(slcID, pStamp->nID);
				pRow->PutValue(slcName, _bstr_t(pStamp->strStampText));
				m_pdlStamps->AddRowSorted(pRow, NULL);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrItemSelectStampsDlg::OnOK()
{
	try
	{
		// (j.jones 2012-12-28 15:20) - PLID 54377 - There is a limit on how many images can be
		// displayed on an image. Warn if they are going to exceed that limit.
		long nCountIncludedStamps = 0;
		for(IRowSettingsPtr pRow = m_pdlStamps->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			long nStampID = VarLong(pRow->GetValue(slcID));
			if(VarBool(pRow->GetValue(slcCheck))) {
				nCountIncludedStamps++;
			}
		}

		//warn if they have too many stamps included
		if(nCountIncludedStamps > MAX_NUM_STAMP_BUTTONS) {
			CString strWarn;
			strWarn.Format("The current image has %li stamps associated with it. "
				"When using this image on an EMN, only a maximum of %li stamps will be available to use.\n\n"
				"Are you sure you wish to continue with this selection of stamps?", nCountIncludedStamps, (long)MAX_NUM_STAMP_BUTTONS);
			if(IDNO == MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
		}

		//now save their changes
		m_pStampExclusions->RemoveAll();
		for(IRowSettingsPtr pRow = m_pdlStamps->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			long nStampID = VarLong(pRow->GetValue(slcID));
			if(!VarBool(pRow->GetValue(slcCheck))) {
				m_pStampExclusions->AddExclusion(nStampID);
			}
		}

		CNxDialog::OnOK();
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrItemSelectStampsDlg::SelectAll(BOOL bSelect)
{
	for(IRowSettingsPtr pRow = m_pdlStamps->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		pRow->PutValue(slcCheck, bSelect ? g_cvarTrue : g_cvarFalse);
	}
}

void CEmrItemSelectStampsDlg::OnBnClickedEmrItemStampSelectAll()
{
	try
	{
		SelectAll(TRUE);
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrItemSelectStampsDlg::OnBnClickedEmrItemStampSelectNone()
{
	try
	{
		SelectAll(FALSE);
	}
	NxCatchAll(__FUNCTION__);
}
