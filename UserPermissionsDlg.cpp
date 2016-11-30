// UserPermissionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UserPermissionsDlg.h"
#include "GlobalDataUtils.h"

// RAC - 2/16
// TODO: Are you fricking kidding me with this definition?
#define magic 187
//

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CUserPermissionsDlg dialog

CUserPermissionsDlg::~CUserPermissionsDlg()
{
	for (int i = 0; i < m_box.GetSize(); i++)
		delete ((CButton*)m_box.GetAt(i));	
}

CUserPermissionsDlg::CUserPermissionsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUserPermissionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUserPermissionsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CUserPermissionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUserPermissionsDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUserPermissionsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CUserPermissionsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

extern CString g_strUserPermissions;

void CUserPermissionsDlg::CreateBoxes(RECT &area, RECT &box, double font)
{
	CString sql;
	_RecordsetPtr rs;
	FieldsPtr rsFields;
	FieldPtr rsField;
	CButton *pButton;
	CRect rect;
	long count;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&m_font, (int)(font * 10.0), "Arial");
	_variant_t vIndex;
	
	rs = CreateRecordset(g_strUserPermissions + " WHERE ID = %i", m_id);
	rsFields = rs->Fields;
	count = rsFields->GetCount();

	m_box.SetSize(count);
	for (short i = 0; i < count; i++)
	{
		vIndex = i;
		rsField = rsFields->GetItem(&vIndex);
		rect.top = area.top + ((box.bottom * i) % (area.bottom - area.top - box.bottom));
		rect.left = area.left + (box.right - box.left) * ((box.bottom * i) / (area.bottom - area.top - box.bottom));
		rect.right = rect.left + box.right;
		rect.bottom = rect.top + box.bottom;
		pButton = new CButton();
		pButton->Create(rsField->GetName(), WS_CHILD|WS_VISIBLE|BS_AUTO3STATE, rect, this, i + magic);
		pButton->SetFont(&m_font);
		pButton->SetCheck(VarByte(rsField->GetValue()));
		m_box.SetAt(i, pButton);
	}
	rs->Close();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
}

/////////////////////////////////////////////////////////////////////////////
// CUserPermissionsDlg message handlers

BOOL CUserPermissionsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	try {
		CreateBoxes(CRect(12, 8, 600, 408), CRect(0, 0, 170, 20), 10.0);
	} NxCatchAllCall("CUserPermissionsDlg::OnInitDialog", {
		OnCancel();
		return FALSE;
	});
	return TRUE;
}

void CUserPermissionsDlg::OnOK() 
{
	CString name, sql = "UPDATE UserPermissionsT SET ";
	CButton *pButton;
	for (int i = 0; i < m_box.GetSize(); i++)
	{	pButton = (CButton *)m_box[i];
		pButton->GetWindowText(name);
		sql += name;
		switch (pButton->GetCheck())
		{	case 0:
				sql += " = 0, ";
				break;
			case 1:
				sql += " = 1, ";
				break;
			default:
				sql += " = 2, ";
		}
	}
	sql.TrimRight();
	sql.TrimRight(',');
	name.Format (" WHERE ID = %i;", m_id);
	sql += name;

	EnsureRemoteData();
	try
	{
		ExecuteSql("%s", sql);
		
		
		//check to see if there are no users with EmployeeConfig as a permission because if there aren't any then we
		//can't let them out of this dialog
		_RecordsetPtr rsEmpConfig;
		rsEmpConfig = CreateRecordset("SELECT Count(*) AS nCount From UserPermissionsT WHERE EmployeeConfig <> 0");
		long nPerCount = AdoFldLong(rsEmpConfig, "nCount"); 
		if (nPerCount == 0 ) {
			//don't let them out of the dialog
			MessageBox("No users have EmployeeConfig as a permission. \nThis means no one has access to change employee permissions, once you exit this screen.  \nPlease check the EmployeeConfig Permission Box");
		}
		else {
			CDialog::OnOK();
		}			
		
	}NxCatchAll("Error in OnOK()");

	
}

void CUserPermissionsDlg::OnCancel() 
{
	CDialog::OnCancel();
}
