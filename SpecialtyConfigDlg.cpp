// SpecialtyConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SpecialtyConfigDlg.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

enum {
	enColumnID = 0,
	enColumnName = 1,
	enColumnCheck = 2
};


// CSpecialtyConfigDlg dialog

IMPLEMENT_DYNAMIC(CSpecialtyConfigDlg, CDialog)

CSpecialtyConfigDlg::CSpecialtyConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpecialtyConfigDlg::IDD, pParent)
{

}

CSpecialtyConfigDlg::~CSpecialtyConfigDlg()
{
}

void CSpecialtyConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpecialtyConfigDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpecialtyConfigDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CSpecialtyConfigDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSpecialtyConfigDlg message handlers


BOOL CSpecialtyConfigDlg::OnInitDialog() {
	try {
		CDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_SpecialtyList = BindNxDataList2Ctrl(this, IDC_SPECIALTIES_CONFIG_LIST, GetRemoteDataSnapshot(), true);
		m_strSql = "";
	} NxCatchAll(__FUNCTION__);
	return TRUE;

}


BEGIN_EVENTSINK_MAP(CSpecialtyConfigDlg, CDialog)
	ON_EVENT(CSpecialtyConfigDlg, IDC_SPECIALTIES_CONFIG_LIST, 10, CSpecialtyConfigDlg::EditingFinishedSpecialtiesConfigList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()





// Make sure that specialty names don't match because that is a waste of a database open.
void CSpecialtyConfigDlg::EditingFinishedSpecialtiesConfigList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		if(bCommit) {
			IRowSettingsPtr pRow(lpRow);
			if(pRow == NULL) {
				return;			
			}


			long nID = VarLong(pRow->GetValue(enColumnID));
			BOOL bTravel = VarBool(pRow->GetValue(enColumnCheck));

			CString strSql;
			if(!bTravel) {
				strSql.Format("INSERT INTO SpecialtyNoTravelT(SpecialtyID) VALUES (%li) \r\n", nID);
			} else {
				strSql.Format("DELETE FROM SpecialtyNoTravelT WHERE SpecialtyID = %li \r\n", nID);
			}
			m_strSql += strSql;
		}
	} NxCatchAll(__FUNCTION__);
}

// On OK, execute all of our sql into the database
void CSpecialtyConfigDlg::OnBnClickedOk()
{
	if(m_strSql != "") {
		ExecuteParamSql(m_strSql);
	}
	OnOK();
}

