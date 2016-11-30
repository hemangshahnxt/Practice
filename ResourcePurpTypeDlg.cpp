// ResourcePurpTypeDlg.cpp : implementation file
//
#include "stdafx.h"
#include "ResourcePurpTypeDlg.h"
#include "globalutils.h"
#include "phasetracking.h"
using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CResourcePurpTypeDlg dialog


CResourcePurpTypeDlg::CResourcePurpTypeDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CResourcePurpTypeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResourcePurpTypeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CResourcePurpTypeDlg::~CResourcePurpTypeDlg()
{
	for (long i=0; i < m_apCombinations.GetSize(); i++)
	{
		delete m_apCombinations.GetAt(i);
	}
	for (i=0; i < m_apChanged.GetSize(); i++)
	{
		delete m_apChanged.GetAt(i);
	}
	m_apCombinations.RemoveAll();
	m_apChanged.RemoveAll();
}

void CResourcePurpTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResourcePurpTypeDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResourcePurpTypeDlg, CNxDialog)
	//{{AFX_MSG_MAP(CResourcePurpTypeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourcePurpTypeDlg message handlers

BOOL CResourcePurpTypeDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		if (!m_apCombinations.GetSize())
		{
			_RecordsetPtr prs = CreateRecordset("SELECT ResourceID, AptTypeID, AptPurposeID FROM ResourcePurposeTypeT");
			FieldsPtr f = prs->Fields;
			while (!prs->eof)
			{
				m_apCombinations.Add(new CCombination(AdoFldLong(f, "ResourceID"),
					AdoFldLong(f, "AptTypeID"), AdoFldLong(f, "AptPurposeID")));
				prs->MoveNext();
			}
		}

		// Fill all the controls with data
		RequeryResources();
		RequeryAptTypes();
		RequeryAptPurposes();

	} NxCatchAll("Error in CResourcePurpTypeDlg::OnInitDialog()");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CResourcePurpTypeDlg::SetResults(CArray<CResourcePurpTypeDlg::CCombination*, CResourcePurpTypeDlg::CCombination*>& aResults,
									  CArray<CResourcePurpTypeDlg::CCombination*, CResourcePurpTypeDlg::CCombination*>& aChanged)
{
	// Clear our saved combinations array
	for (long i=0; i < m_apCombinations.GetSize(); i++)
	{
		delete m_apCombinations.GetAt(i);
	}
	for (i=0; i < m_apChanged.GetSize(); i++)
	{
		delete m_apChanged.GetAt(i);
	}
	m_apCombinations.RemoveAll();
	m_apChanged.RemoveAll();

	// Now copy the combination array to the current combination
	// array
	for (i=0; i < aResults.GetSize(); i++)
	{
		m_apCombinations.Add(new CCombination( aResults.GetAt(i) ) );
	}
	for (i=0; i < aChanged.GetSize(); i++)
	{
		m_apChanged.Add(new CCombination( aChanged.GetAt(i) ) );
	}
}

void CResourcePurpTypeDlg::GetResults(CArray<CResourcePurpTypeDlg::CCombination*, CResourcePurpTypeDlg::CCombination*>& aResults,
									  CArray<CResourcePurpTypeDlg::CCombination*, CResourcePurpTypeDlg::CCombination*>& aChanged)
{
	// Clear our saved combinations array
	for (long i=0; i < aResults.GetSize(); i++)
	{
		delete aResults.GetAt(i);
	}
	for (i=0; i < aChanged.GetSize(); i++)
	{
		delete aChanged.GetAt(i);
	}
	aResults.RemoveAll();
	aChanged.RemoveAll();

	// Now copy the current combination array to the saved combination
	// array
	for (i=0; i < m_apCombinations.GetSize(); i++)
	{
		aResults.Add(new CCombination( m_apCombinations.GetAt(i) ) );
	}
	for (i=0; i < m_apChanged.GetSize(); i++)
	{
		aChanged.Add(new CCombination( m_apChanged.GetAt(i) ) );
	}
}

void CResourcePurpTypeDlg::RequeryResources()
{
	long nID = -1;
	if (m_dlAptResource == NULL) {
		m_dlAptResource = BindNxDataListCtrl(this, IDC_APTRESOURCE_COMBO, GetRemoteData(), false);
	} else if (m_dlAptResource->CurSel > -1) {
		nID = VarLong(m_dlAptResource->GetValue(m_dlAptResource->CurSel, 0));
	}
	m_dlAptResource->Requery();
	if (nID != -1)
		m_dlAptResource->TrySetSelByColumn(0, nID);
	else
		m_dlAptResource->CurSel = 0;
}

void CResourcePurpTypeDlg::RequeryAptTypes()
{
	long nID = -1;
	if (m_dlAptType == NULL) {
		m_dlAptType = BindNxDataListCtrl(this, IDC_APTTYPE_COMBO, GetRemoteData(), false);
	} else if (m_dlAptType->CurSel > -1) {
		nID = VarLong(m_dlAptType->GetValue(m_dlAptType->CurSel, 0));
	}
	m_dlAptType->WhereClause = "Inactive = 0";
	m_dlAptType->Requery();

	if (nID != -1)
		m_dlAptType->SetSelByColumn(0, nID);
	else
		m_dlAptType->CurSel = 0;
}

void CResourcePurpTypeDlg::RequeryAptPurposes()
{
	if (m_dlAptPurpose == NULL) {
		m_dlAptPurpose = BindNxDataListCtrl(this, IDC_PROCEDURE_LIST, GetRemoteData(), false);
	}
	long nAptTypeID = (m_dlAptType->CurSel > -1) ? VarLong(m_dlAptType->GetValue(m_dlAptType->CurSel, 0)) : -1;
	short nCategory = (m_dlAptType->CurSel > -1) ? VarShort(m_dlAptType->GetValue(m_dlAptType->CurSel, 2)) : -1;
	if (nAptTypeID == -1)
	{
		m_dlAptPurpose->Clear();
		return;
	}
	
	// Generate our filter
	CString strFrom, strWhere;
	strFrom.Format("AptPurposeT "
		"INNER JOIN AptPurposeTypeT ON AptPurposeT.ID = AptPurposeTypeT.AptPurposeID "
			"AND AptPurposeTypeT.AptTypeID = %i "
		"LEFT JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID", nAptTypeID);
	if (nCategory == PhaseTracking::AC_NON_PROCEDURAL || nCategory == PhaseTracking::AC_BLOCK_TIME)
		strWhere = "ProcedureT.ID IS NULL";
	else strWhere = "ProcedureT.ID IS NOT NULL";

	// (c.haag 2008-12-09 17:18) - PLID 32264 - Filter out inactive procedures
	strWhere += " AND AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) ";

	m_dlAptPurpose->FromClause = _bstr_t(strFrom);
	m_dlAptPurpose->WhereClause = _bstr_t(strWhere);
	m_dlAptPurpose->Requery();
}

BEGIN_EVENTSINK_MAP(CResourcePurpTypeDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CResourcePurpTypeDlg)
	ON_EVENT(CResourcePurpTypeDlg, IDC_PROCEDURE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedProcedureList, VTS_I2)
	ON_EVENT(CResourcePurpTypeDlg, IDC_APTTYPE_COMBO, 16 /* SelChosen */, OnSelChosenApttypeCombo, VTS_I4)
	ON_EVENT(CResourcePurpTypeDlg, IDC_PROCEDURE_LIST, 10 /* EditingFinished */, OnEditingFinishedProcedureList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CResourcePurpTypeDlg, IDC_APTRESOURCE_COMBO, 16 /* SelChosen */, OnSelChosenAptresourceCombo, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CResourcePurpTypeDlg::OnRequeryFinishedProcedureList(short nFlags) 
{
	COleVariant vTrue;
	long nResourceID = (m_dlAptResource->CurSel > -1) ? VarLong(m_dlAptResource->GetValue(m_dlAptResource->CurSel, 0)) : -1;
	long nAptTypeID = (m_dlAptType->CurSel > -1) ? VarLong(m_dlAptType->GetValue(m_dlAptType->CurSel, 0)) : -1;
	vTrue.vt = VT_BOOL;
	vTrue.boolVal = TRUE;
	for (long i=0; i < m_apCombinations.GetSize(); i++)
	{
		CCombination* p = m_apCombinations.GetAt(i);
		if (p->GetResourceID() != nResourceID || p->GetAptTypeID() != nAptTypeID)
			continue;

		long nRow = m_dlAptPurpose->FindByColumn(0, p->GetAptPurposeID(), 0, FALSE);
		if (nRow != -1) m_dlAptPurpose->PutValue(nRow, 1, vTrue);
	}
}

void CResourcePurpTypeDlg::OnSelChosenApttypeCombo(long nRow) 
{
	RequeryAptPurposes();
}

void CResourcePurpTypeDlg::OnEditingFinishedProcedureList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if (!bCommit || nCol != 1) return;
	long nResourceID = (m_dlAptResource->CurSel > -1) ? VarLong(m_dlAptResource->GetValue(m_dlAptResource->CurSel, 0)) : -1;
	long nAptTypeID = (m_dlAptType->CurSel > -1) ? VarLong(m_dlAptType->GetValue(m_dlAptType->CurSel, 0)) : -1;
	long nPurposeID = m_dlAptPurpose->GetValue(nRow, 0);

	if (varNewValue.boolVal)
	{
		m_apCombinations.Add(new CCombination(nResourceID, nAptTypeID, nPurposeID));
	}
	else
	{
		for (long i=0; i < m_apCombinations.GetSize(); i++)
		{
			CCombination* p = m_apCombinations.GetAt(i);
			if (p->GetResourceID() == nResourceID && p->GetAptTypeID() == nAptTypeID &&
				p->GetAptPurposeID() == nPurposeID)
			{
				delete p;
				m_apCombinations.RemoveAt(i);
				break;
			}
		}
	}

	CCombination* pChanged = NULL;
	for (long i=0; i < m_apChanged.GetSize(); i++)
	{
		pChanged = m_apChanged.GetAt(i);
		if (pChanged->GetResourceID() == nResourceID && pChanged->GetAptTypeID() == nAptTypeID &&
			pChanged->GetAptPurposeID() == nPurposeID)
		{
			break;
		}
	}
	if (i == m_apChanged.GetSize())
	{
		pChanged = new CCombination(nResourceID, nAptTypeID, nPurposeID);
		m_apChanged.Add(pChanged);
	}
	if (pChanged)
	{
		pChanged->SetSelected(varNewValue.boolVal);
	}
}

void CResourcePurpTypeDlg::OnSelChosenAptresourceCombo(long nRow) 
{
	RequeryAptTypes();
	RequeryAptPurposes();	
}
