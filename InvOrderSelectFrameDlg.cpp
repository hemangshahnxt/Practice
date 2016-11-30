// InvOrderSelectFrameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "InvOrderSelectFrameDlg.h"


// CInvOrderSelectFrameDlg dialog

IMPLEMENT_DYNAMIC(CInvOrderSelectFrameDlg, CNxDialog)

CInvOrderSelectFrameDlg::CInvOrderSelectFrameDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvOrderSelectFrameDlg::IDD, pParent)
{
	//m_strFrameSelectWhereClause and m_strAllSelection should never change. 
	m_strFrameSelectWhereClause = "(FramesDataT.IsCatalog = 1) AND (ProductFrames.ID IS NULL) ";
	m_strAllSelection = " { All } ";
	m_strManufacture = "";
	m_strCollection = "";
	m_strBrand = "";
	m_nFramesID = -1; 
}

CInvOrderSelectFrameDlg::~CInvOrderSelectFrameDlg()
{
}

void CInvOrderSelectFrameDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK); 
	DDX_Control(pDX, IDCANCEL, m_btnCancel); 
	DDX_Control(pDX, IDC_REFRESH, m_btnRefresh); 
}


BEGIN_MESSAGE_MAP(CInvOrderSelectFrameDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CInvOrderSelectFrameDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CInvOrderSelectFrameDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_REFRESH, &CInvOrderSelectFrameDlg::OnBnClickedRefresh)
END_MESSAGE_MAP()


// CInvOrderSelectFrameDlg message handlers
BEGIN_EVENTSINK_MAP(CInvOrderSelectFrameDlg, CNxDialog)
	ON_EVENT(CInvOrderSelectFrameDlg, IDC_MANUFACTURE_FILTER, 1, CInvOrderSelectFrameDlg::SelChangingManufactureFilter, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvOrderSelectFrameDlg, IDC_COLLECTION_FILTER, 1, CInvOrderSelectFrameDlg::SelChangingCollectionFilter, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvOrderSelectFrameDlg, IDC_BRAND_FILTER, 1, CInvOrderSelectFrameDlg::SelChangingBrandFilter, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvOrderSelectFrameDlg, IDC_FRAMES_SELECT_LIST, 1, CInvOrderSelectFrameDlg::SelChangingFramesSelectList, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

BOOL CInvOrderSelectFrameDlg::OnInitDialog() 
{
	try{
		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK); 
		m_btnCancel.AutoSet(NXB_CANCEL); 
		m_btnRefresh.AutoSet(NXB_REFRESH); 

		// (b.spivey, September 20, 2011) - PLID 45266 - The filter datalists should requery because they're fairly small. 
		//		The Frames Select datalist should not requery because it's huge. 
		m_pManufacturerFilter = BindNxDataList2Ctrl(IDC_MANUFACTURE_FILTER, true);
		m_pCollectionFilter = BindNxDataList2Ctrl(IDC_COLLECTION_FILTER, true);
		m_pBrandFilter = BindNxDataList2Ctrl(IDC_BRAND_FILTER, true);
		m_pFramesSelect = BindNxDataList2Ctrl(IDC_FRAMES_SELECT_LIST, false); 

		// (b.spivey, September 20, 2011) - PLID 45266 - Auto-select " {All} "
		m_pManufacturerFilter->SetSelByColumn(0, _bstr_t(m_strAllSelection)); 
		m_pCollectionFilter->SetSelByColumn(0, _bstr_t(m_strAllSelection));
		m_pBrandFilter->SetSelByColumn(0, _bstr_t(m_strAllSelection));

		return TRUE;

	}NxCatchAll(__FUNCTION__);

	return FALSE; 
}

void CInvOrderSelectFrameDlg::SelChangingManufactureFilter(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Should never be able to get a null. 
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

		NXDATALIST2Lib::IRowSettingsPtr pNewSel(*lppNewSel);
		CString str = VarString(pNewSel->GetValue(0), m_strAllSelection);

		//Becomes part of the where clause if there is an actual selection. 
		if(str != m_strAllSelection){
			m_strManufacture.Format(" AND (FramesDataT.ManufacturerName = '%s')", _Q(str));
		}
		else{
			m_strManufacture = ""; 
		}

	} NxCatchAll(__FUNCTION__);

}

void CInvOrderSelectFrameDlg::SelChangingCollectionFilter(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Should never be able to get a null. 
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

		NXDATALIST2Lib::IRowSettingsPtr pNewSel(*lppNewSel);
		CString str = VarString(pNewSel->GetValue(0), m_strAllSelection);

		//Becomes part of the where clause if there is an actual selection. 
		if(str != m_strAllSelection){
			m_strCollection.Format(" AND (FramesDataT.CollectionName = '%s')", _Q(str));
		}
		else{
			m_strCollection = ""; 
		}
	} NxCatchAll(__FUNCTION__);

}

void CInvOrderSelectFrameDlg::SelChangingBrandFilter(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Should never be able to get a null. 
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

		NXDATALIST2Lib::IRowSettingsPtr pNewSel(*lppNewSel);
		CString str = VarString(pNewSel->GetValue(0), m_strAllSelection);

		//Becomes part of the where clause if there is an actual selection. 
		if(str != m_strAllSelection){
			m_strBrand.Format(" AND (FramesDataT.BrandName = '%s')", _Q(str));
		}
		else{
			m_strBrand = ""; 
		}
	} NxCatchAll(__FUNCTION__);
}

void CInvOrderSelectFrameDlg::OnBnClickedOk()
{
	// (b.spivey, November 22, 2011) - PLID 45266 - Added try/catch.
	try{
		// (b.spivey, September 20, 2011) - PLID 45266 - Safety case: The datalist starts out with a NULL selection, so we 
		//   have to always consider that it could be null when you click OK. 
		// (b.spivey, November 22, 2011) - PLID 45266 - Whoops, forgot my VarLong call. 
		if(m_pFramesSelect->CurSel){
			m_nFramesID = VarLong(m_pFramesSelect->CurSel->GetValue(efsID), -1);
			CNxDialog::OnOK(); 
		}
		else{
			m_nFramesID = -1; 
			MessageBox("Please select a frame.", "Warning", MB_ICONWARNING); 
		}
	}NxCatchAll(__FUNCTION__);
}

void CInvOrderSelectFrameDlg::OnBnClickedCancel()
{
	try{
		CNxDialog::OnCancel(); 
	}NxCatchAll(__FUNCTION__);
}

void CInvOrderSelectFrameDlg::OnBnClickedRefresh()
{
	// (b.spivey, November 22, 2011) - PLID 45266 - Added try/catch.
	try{
		CString strWhere;
		// (b.spivey, September 20, 2011) - PLID 45266 - Construct this clause every time. 
		strWhere = m_strFrameSelectWhereClause + m_strManufacture + m_strCollection + m_strBrand;

		// (b.spivey, September 16, 2011) - PLID 45266 - If the WhereClause is different from the current filters, we need 
		//		to requery. 
		if(strWhere.CompareNoCase(m_pFramesSelect->GetWhereClause())){
			// (b.spivey, November 22, 2011) - PLID 45266 - use _bstr_t()
			m_pFramesSelect->PutWhereClause(_bstr_t(strWhere)); 
			m_pFramesSelect->Requery(); 
			m_pFramesSelect->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely); 
			m_pFramesSelect->PutCurSel(m_pFramesSelect->GetTopRow()); 
		}
	}NxCatchAll(__FUNCTION__)
}

// (b.spivey, November 21, 2011) - PLID 45266 - Handle nulls.
void CInvOrderSelectFrameDlg::SelChangingFramesSelectList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try{
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__)
}
