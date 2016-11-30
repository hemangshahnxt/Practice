// WarrantyDlg.cpp : implementation file
//
//(c.copits 2010-10-28) PLID 38598 - Warranty tracking system

#include "stdafx.h"
#include "PatientsRc.h"
#include "WarrantyDlg.h"


// CWarrantyDlg dialog

IMPLEMENT_DYNAMIC(CWarrantyDlg, CNxDialog)

CWarrantyDlg::CWarrantyDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CWarrantyDlg::IDD, pParent)
{

}

CWarrantyDlg::~CWarrantyDlg()
{
}

void CWarrantyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CWarrantyDlg, CNxDialog)
	ON_BN_CLICKED(IDCLOSE, &CWarrantyDlg::OnBtnClose)
END_MESSAGE_MAP()


// CWarrantyDlg message handlers

// Set the datalist's from clause
void CWarrantyDlg::SetFromClause(CString strFrom)
{
	m_strFrom = strFrom;
}

// Set the datalist's where clause
void CWarrantyDlg::SetWhereClause(CString strWhere)
{
	m_strWhere = strWhere;
}

// Set the NxColor background color
void CWarrantyDlg::SetWarrantyBkgColor(OLE_COLOR nNewColor)
{
	m_nWarrantyBkgColor = nNewColor;
}

BOOL CWarrantyDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		// Set dialog control types / colors
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDCLOSE))->AutoSet(NXB_CLOSE);
		((CNxColor*)GetDlgItem(IDC_NXCOLOR_WARRANTYDLG))->SetColor(m_nWarrantyBkgColor);

		// Bind and load the datalist's clauses
		m_pWarrantyList = BindNxDataList2Ctrl(IDC_WARRANTY_LIST, false);
		m_pWarrantyList->PutWhereClause(_bstr_t(m_strWhere));
		m_pWarrantyList->PutFromClause(_bstr_t(m_strFrom));
		m_pWarrantyList->Requery();

		GetDlgItem(IDCLOSE)->SetFocus();

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CWarrantyDlg::OnBtnClose()
{
	try {
	
		CNxDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}
