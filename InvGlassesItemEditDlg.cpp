// InvGlassesITemEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "InvGlassesItemEditDlg.h"
#include "InventoryRc.h"
// (s.dhole 2011-03-15 14:59) - PLID 42845 Add/Edit Glasses custom catlog item

// CInvGlassesItemEditDlg dialog

IMPLEMENT_DYNAMIC(CInvGlassesItemEditDlg, CNxDialog)
using namespace ADODB;
CInvGlassesItemEditDlg::CInvGlassesItemEditDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvGlassesItemEditDlg::IDD, pParent)
{

}

CInvGlassesItemEditDlg::~CInvGlassesItemEditDlg()
{
}

void CInvGlassesItemEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ITEM_NAME , m_nxeName);
	DDX_Control(pDX, IDC_EDIT_ITEM_CODE, m_nxeCode);
	DDX_Control(pDX, IDOK , m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);

}


BEGIN_MESSAGE_MAP(CInvGlassesItemEditDlg, CNxDialog)

	ON_BN_CLICKED(IDOK, &CInvGlassesItemEditDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CInvGlassesItemEditDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL CInvGlassesItemEditDlg::OnInitDialog() 
{
try {
	CNxDialog::OnInitDialog();
	m_btnOk.AutoSet(NXB_OK)  ;
	m_btnCancel.AutoSet(NXB_CANCEL );
	
	SetDlgItemText(IDC_EDIT_ITEM_NAME,m_strNameValue );  
	SetDlgItemText(IDC_EDIT_ITEM_CODE ,m_strCodeValue );
	m_nxeName.SetLimitText(100);
	m_nxeCode.SetLimitText(50);
	if (m_nID>0) 
		SetWindowText(FormatString("Edit %s", m_strDescription));
	else
		SetWindowText(FormatString("Add %s", m_strDescription));
	}NxCatchAll("CInvGlassesItemEditDlg::OnInitDialog");
	return TRUE;
}


// CInvGlassesItemEditDlg message handlers
HRESULT CInvGlassesItemEditDlg::Open(CString strTable,CString strName, CString strCode,CString strNameValue, CString strCodeValue,long nGlassOrderProcessType,long nID, CString strDescription)
{
try {
	//setTit(IDD_GLASSES_ITEM_DLG, FormatString("Select one or more %s:", strDescription));
	m_strTable= strTable;
	m_strName = strName;
	m_strCode = strCode;
	m_strDescription=strDescription;
	m_strNameValue =strNameValue;
	m_strCodeValue=strCodeValue;
	m_nGlassOrderProcessType=nGlassOrderProcessType;
	m_nID=nID;
	}NxCatchAll("CInvGlassesItemEditDlg::Open");
	return DoModal();
}

void CInvGlassesItemEditDlg::OnBnClickedOk()
{
try {
	if (CheckEmptyField()) 
		{
		CNxDialog::OnOK();
		}
	}NxCatchAll("CInvGlassesItemDlg::OnBnClickedOk");
}

BOOL CInvGlassesItemEditDlg::CheckEmptyField()
{
  BOOL bResult=FALSE;
try {
	CString strName,strCode;
	GetDlgItemText(IDC_EDIT_ITEM_NAME,strName);  
	GetDlgItemText(IDC_EDIT_ITEM_CODE ,strCode);
	strName.Remove(' ') ;
	strCode.Remove(' ') ;
	if (strName.IsEmpty() && strCode.IsEmpty()) 	
	{
		
		AfxMessageBox(FormatString( "You must enter a %s name and code.",m_strDescription)) ;
		return bResult;
	}
	if (strName.IsEmpty()) 	
	{
		AfxMessageBox(FormatString( "You must enter a %s name.",m_strDescription)) ;
		return bResult;
	}
	if (strCode.IsEmpty()) 	
	{
		AfxMessageBox(FormatString( "You must enter a %s code",m_strDescription)) ;
		return bResult;
	}
	bResult=SaveData();  
	}NxCatchAll("CInvGlassesItemDlg::CheckEmptyField");
	return bResult;
}

BOOL CInvGlassesItemEditDlg::SaveData()
{
try {
	CString strName,strCode;
	GetDlgItemText(IDC_EDIT_ITEM_NAME,strName);  
	GetDlgItemText(IDC_EDIT_ITEM_CODE ,strCode);
	_RecordsetPtr rs;
	  if (m_nID>0)
	  {
		
		  // Update
		  rs = CreateParamRecordset( FormatString( " SET NOCOUNT ON \r\n "
			  "if not exists (SELECT * FROM %s WHERE %s={STRING}  AND ID<>{INT})  \r\n"
				" BEGIN \r\n"	
				"UPDATE %s SET %s ={STRING} , %s ={STRING}  Where ID= {INT} AND GlassesOrderProcessTypeID=1  \r\n"
				"  SELECT * FROM %s WHERE ID ={INT}; \r\n"
				"END \r\n"
				"ELSE\r\n"
				"BEGIN \r\n"
				" SELECT -1  AS ID   \r\n"
				"END \r\n",
				m_strTable,m_strCode, m_strTable, m_strName ,m_strCode,m_strTable)
				,strCode.Left(50),m_nID,strName.Left(100) ,strCode.Left(50) ,m_nID,m_nID,strCode.Left(50));
		if(!rs->eof) {
			//just check return id. if it is -1 than we di not update record only reaso is, the record is exist in table
			if ( VarLong(AdoFldLong(rs, "ID"),-1) ==-1 ) {
				AfxMessageBox(FormatString("You are not allowed to use same code more than once in the %s list.",m_strDescription)) ;
				return FALSE;
			}
			else{
			m_strNewName =strName.Left(100);
			m_strNewCode=strCode.Left(50);
			}
		}
	  
	  }
	  else 
	  {
		 rs = CreateParamRecordset( FormatString("SET NOCOUNT ON \r\n "
			 	"if not exists (SELECT * FROM %s WHERE %s={STRING})  \r\n"
				" BEGIN \r\n"	
				"INSERT INTO  %s \r\n"
				"( %s,%s,GlassesOrderProcessTypeID)   \r\n"
				" Values ({STRING},{STRING},{INT}); \r\n"
				"SELECT  Convert(int, SCOPE_IDENTITY()) as NewID, ProcessName FROM  GlassesOrderProcessTypeT WHERE id ={INT}  "
				"END \r\n"
				"ELSE\r\n"
				"BEGIN \r\n"
				"SELECT  -1 as NewID \r\n"
				"END \r\n",
				m_strTable,m_strCode,m_strTable, m_strName ,m_strCode ),
				strCode.Left(50),strName.Left(100) ,strCode.Left(50) ,m_nGlassOrderProcessType,m_nGlassOrderProcessType );
		 
		 if(!rs->eof) {
			long nNewID= AdoFldLong(rs, "NewID");
			if (nNewID<1)
			{
				AfxMessageBox(FormatString("You are not allowed to use same code more than once in the %s list.",m_strDescription)) ;
				return FALSE;
			}

			m_strNewName =strName.Left(100);
			m_strNewCode=strCode.Left(50);
			m_nNewGlassOrderProcessTypeID= m_nGlassOrderProcessType;
			m_nNewID= AdoFldLong(rs, "NewID");
			m_strNewGlassOrderProcessType = AdoFldString(rs, "ProcessName");;
		 }
		 else
		 {
		   AfxMessageBox(FormatString("You are not allowed to use same code more than once in the %s list.",m_strDescription)) ;
		   return FALSE;
		 }
	  }
		

	}NxCatchAll("CInvGlassesItemEditDlg::SaveData");
	return TRUE;
}

void CInvGlassesItemEditDlg::OnBnClickedCancel()
{
try{ 
	CNxDialog::OnCancel();	
	}NxCatchAll("Error in CInvGlassesItemEditDlg::OnBnClickedCancel");
}
