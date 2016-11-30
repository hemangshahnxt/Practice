// QuickBookRepID.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "QuickBookRepID.h"


// CQuickBookRepID dialog
// (a.vengrofski 2010-04-22 09:23) - PLID <38205> - File created
IMPLEMENT_DYNAMIC(CQuickBookRepID, CNxDialog)

CQuickBookRepID::CQuickBookRepID(CWnd* pParent /*=NULL*/)
: CNxDialog(CQuickBookRepID::IDD, pParent)
{

}

CQuickBookRepID::~CQuickBookRepID()
{
}

BOOL CQuickBookRepID::OnInitDialog() 
{
	try{
		CNxDialog::OnInitDialog();
		m_pQBReps = BindNxDataList2Ctrl(IDC_QBREPS_LIST, true);
		m_btnOK.AutoSet(NXB_OK);
	}NxCatchAll("Error in CQuickBookRepID::OnInitDialog");
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CQuickBookRepID::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
}


BEGIN_MESSAGE_MAP(CQuickBookRepID, CNxDialog)
END_MESSAGE_MAP()


// CQuickBookRepID message handlers
BEGIN_EVENTSINK_MAP(CQuickBookRepID, CNxDialog)
	ON_EVENT(CQuickBookRepID, IDC_QBREPS_LIST, 10, CQuickBookRepID::EditingFinishedQbrepsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CQuickBookRepID, IDC_QBREPS_LIST, 9, CQuickBookRepID::EditingFinishingQbrepsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

void CQuickBookRepID::EditingFinishedQbrepsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{
		if (bCommit)
		{//We are cleared for database updating
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			CString strEntered(varNewValue);
			strEntered.MakeUpper();
			_variant_t varEntered(strEntered);
			long lUserID = VarLong(pRow->GetValue(qbrPersonID), (long)-1);
			if (lUserID != -1)
			{//check for invalid selections

				// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
				NxAdo::PushMaxRecordsWarningLimit pmr(1);

				BOOL bIsInMap = VarLong(pRow->GetValue(qbrIsInMap),(long)0) == 0 ? FALSE : TRUE;
				if (bIsInMap == TRUE)
				{
					if (strEntered.GetLength() == 2 //needs to have BOTH UserID and a two letter ID
						|| strEntered.GetLength() == 0)//Or a 0 letter ID
					{
						ExecuteParamSql("UPDATE MapQBRepsT SET QBID = {STRING} WHERE UserID = {INT}",strEntered, lUserID);
						pRow->PutValue(qbrQBID,varEntered);

					}
				}
				else if (strEntered.GetLength() == 2)
				{
					ExecuteParamSql("INSERT INTO MapQBRepsT (UserID, QBID) VALUES ({INT}, {STRING})",lUserID ,strEntered);
					pRow->PutValue(qbrQBID,varEntered);
					pRow->PutValue(qbrIsInMap,!bIsInMap);
				}
			}
		}
	}NxCatchAll("Error in CQuickBookRepID::EditingFinishedQbrepsList");
}

void CQuickBookRepID::EditingFinishingQbrepsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		if (*pbCommit)
		{//They are commiting this selection
			CString strEntered = strUserEntered;
			strEntered.MakeUpper();
			_variant_t varNew(strEntered);
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			long lUserID = VarLong(pRow->GetValue(qbrPersonID), (long)-1);
			if (lUserID != -1)
			{//check for invalid selections
				if (strEntered.GetLength() == 2)
				{//Only allow two characters 
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pQBReps->FindByColumn(qbrQBID, varNew, m_pQBReps->FindAbsoluteFirstRow(VARIANT_FALSE), VARIANT_FALSE);
					if (pRow && (pRow != lpRow))
					{//Someone already has that QBID
						CString strDupe;
						strDupe.Format("Duplicate Quickbooks ID \"%s\" already exists for %s, %s",strEntered, VarString(pRow->GetValue(qbrLast)), VarString(pRow->GetValue(qbrFirst)));
						MessageBox(strDupe);
						// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
						VariantClear(pvarNewValue);
						*pvarNewValue = _variant_t(varOldValue).Detach();
						*pbCommit = false;
						*pbContinue = false;
					}
				}
				else if (strEntered.GetLength() == 0)
				{//They have erased the contents let them pass
					*pbCommit = true;
					*pbContinue = true;
				}
				else
				{//They have neither entered two or erased it all
					//They Shall not pass.
					MessageBox("You must enter a two or zero character ID","Map QB IDs");
					*pbCommit = false;
					*pbContinue = false;
				}
			}
			else
			{//Invalid selection do not pass go.
				*pbCommit = false;
				*pbContinue = false;
			}
		}
	}NxCatchAll("Error in CQuickBookRepID::EditingFinishingQbrepsList");
}

void CQuickBookRepID::OnCancel()
{
	try {
		CNxDialog::OnCancel(); 
	}NxCatchAll("Error in CQuickBookRepID::OnCancel");
}