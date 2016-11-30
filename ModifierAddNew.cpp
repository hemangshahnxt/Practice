// ModifierAddNew.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "CPTCodes.h"
#include "ModifierAddNew.h"
#include "GlobalDataUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CModifierAddNew dialog


CModifierAddNew::CModifierAddNew(CWnd* pParent /*=NULL*/)
	: CNxDialog(CModifierAddNew::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModifierAddNew)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CModifierAddNew::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifierAddNew)
	DDX_Control(pDX, IDC_MODIFIER, m_nxeditModifier);
	DDX_Control(pDX, IDC_DESC, m_nxeditDesc);
	DDX_Control(pDX, IDC_MULTIPLIER, m_nxeditMultiplier);
	DDX_Control(pDX, ID_OK_BTN, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModifierAddNew, CNxDialog)
	//{{AFX_MSG_MAP(CModifierAddNew)
	ON_BN_CLICKED(ID_OK_BTN, OnOkBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifierAddNew message handlers

void CModifierAddNew::OnOK() 
{

}

void CModifierAddNew::OnOkBtn() 
{
	CString modifier,
			multiplier,
			desc;
	double Multiplier;

	GetDlgItemText (IDC_MODIFIER, modifier);
	GetDlgItemText (IDC_DESC, desc);
	GetDlgItemText (IDC_MULTIPLIER, multiplier);

	// (j.dinatale 2010-08-30) - PLID 38121 - Need to trim modifier to remove all white space, then we can use it
	modifier.TrimRight();
	modifier.TrimLeft();

	if (modifier.IsEmpty() || multiplier.IsEmpty()) {
		AfxMessageBox (IDS_MODIFIER_BLANK);
		return;
	}

	if (desc.IsEmpty()) desc = " ";

	//DRT 5/27/2004 - PLID 12481 - Upped limit to 255 chars from 50.
	if(desc.GetLength() > 255) {
		AfxMessageBox("The description you entered is too long, please shorten it to less than 255 characters.");
		return;
	}

	CString sql;

	try 
	{
		_RecordsetPtr rs = CreateRecordset("SELECT Active FROM CPTModifierT WHERE Number = '%s'", _Q(modifier));
		if(!rs->eof){
			if(AdoFldBool(rs, "Active")) {
				AfxMessageBox(IDS_MODIFIER_DUPLICATE);
			}
			else {
				// (z.manning, 05/01/2007) - PLID 16623 - If it's inactive, let them know so they can find it.
				MessageBox("This modifier already exists, but is inactive. Please click the 'Inactive Codes' button if you'd like to reactivate it.");
			}
			rs->Close();
			return;
		}
		rs->Close();

		Multiplier = (double)atof(multiplier);

		if(Multiplier < -100.0) {
			AfxMessageBox("You cannot have a multiplier less than -100.");
			return;
		}

		ExecuteSql("INSERT INTO CPTModifierT (Number, Note, Multiplier) VALUES ('%s', '%s', %lg)", _Q(modifier), _Q(desc), (Multiplier + 100.0) / 100.0);

		strMod = modifier;
		strDesc = desc;
		nMultiplier = Multiplier;


		CDialog::OnOK();

	}NxCatchAll("Error in OnOkBtn()");
}

BOOL CModifierAddNew::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// (z.manning, 04/30/2008) - PLID 29864 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	GetDlgItem(IDC_MODIFIER)->SetFocus();
	SetDlgItemText(IDC_MULTIPLIER, "0");

	((CNxEdit*)GetDlgItem(IDC_MODIFIER))->SetLimitText(10);
	((CNxEdit*)GetDlgItem(IDC_DESC))->SetLimitText(255);
	
	return TRUE;
}
