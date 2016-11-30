// MarketFilterPickerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "marketingrc.h"
#include "MarketFilterPickerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CMarketFilterPickerDlg dialog


CMarketFilterPickerDlg::CMarketFilterPickerDlg(int mktType, MarketFilterType mfType, CWnd* pParent /*=NULL*/)
	: CNxDialog(CMarketFilterPickerDlg::IDD, pParent)
{
	m_mktType = mktType;
	m_mfType = mfType;
	//{{AFX_DATA_INIT(CMarketFilterPickerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


CMarketFilterPickerDlg::CMarketFilterPickerDlg(CDWordArray &dwAllowedFilters, MarketFilterType mfType, CWnd* pParent /*=NULL*/)
	: CNxDialog(CMarketFilterPickerDlg::IDD, pParent)
{
	m_mktType = -1;
	m_mfType = mfType;
	for(int i = 0; i < dwAllowedFilters.GetSize(); i++) {
		m_dwAllowedFilters.Add(dwAllowedFilters.GetAt(i));
	}
	//{{AFX_DATA_INIT(CMarketFilterPickerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}



void CMarketFilterPickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketFilterPickerDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMarketFilterPickerDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMarketFilterPickerDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketFilterPickerDlg message handlers

void CMarketFilterPickerDlg::OnOK() 
{
	m_Filter = (MarketFilter)VarLong(m_pMarketFilter->GetValue(m_pMarketFilter->CurSel, 0));	
	
	CNxDialog::OnOK();
}



#define ADD_FILTER(mfFilter)  if(DoesGraphSupportFilter((MarketGraphType)m_mktType, mfFilter)) {\
			pRow = m_pMarketFilter->GetRow(-1);\
			pRow->PutValue(0, (long)mfFilter);\
			pRow->PutValue(1, _bstr_t(GetFieldFromEnum(mfFilter)));\
			pRow->PutValue(2, _bstr_t(GetDisplayNameFromEnum(mfFilter)));\
			m_pMarketFilter->AddRow(pRow);\
		}


BOOL CMarketFilterPickerDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 12:41) - PLID 29824 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//Bind the data list
		m_pMarketFilter = BindNxDataListCtrl(this, IDC_MARKET_FILTER_LIST, GetRemoteData(), FALSE);

		//set the title for the dialog
		switch (m_mfType) {
			case mftDate:
				SetWindowText("Pick Date Type");
			break;

			case mftLocation:
				SetWindowText("Pick Location Type");
			break;

			case mftProvider:
				SetWindowText("Pick Provider Type");
			break;
		}

		if(m_mktType == -1) {
			IRowSettingsPtr pRow;
			for(int i = 0; i < m_dwAllowedFilters.GetSize(); i++) {
				pRow = m_pMarketFilter->GetRow(-1);
				pRow->PutValue(0, (long)m_dwAllowedFilters.GetAt(i));
				pRow->PutValue(1, _bstr_t(GetFieldFromEnum((MarketFilter)m_dwAllowedFilters.GetAt(i))));
				pRow->PutValue(2, _bstr_t(GetDisplayNameFromEnum((MarketFilter)m_dwAllowedFilters.GetAt(i))));
				m_pMarketFilter->AddRow(pRow);
			}
		}
		else {			
			//lets add them all and then take out the ones we don't need
			IRowSettingsPtr pRow;
			switch(m_mfType) {
				case mftDate:
					ADD_FILTER(mfFirstContactDate);
					ADD_FILTER(mfChargeDate);
					ADD_FILTER(mfPaymentDate);
					ADD_FILTER(mfApptInputDate);
					ADD_FILTER(mfApptDate);
					ADD_FILTER(mfEffectivenessDate);
					ADD_FILTER(mfCostDatePaid);
					ADD_FILTER(mfConsultDate);
					ADD_FILTER(mfConsultInputDate);
					//DRT 5/8/2008 - PLID 29966 - Added referral Date
					ADD_FILTER(mfReferralDate);
				break;

				case mftLocation:
					ADD_FILTER(mfPatientLocation);
					ADD_FILTER(mfTransLocation);
					ADD_FILTER(mfApptLocation);
					ADD_FILTER(mfCostLocation);
					ADD_FILTER(mfPatCostLocation);
					ADD_FILTER(mfTransCostLocation);
					ADD_FILTER(mfPatNoCostLocation);
					ADD_FILTER(mfTransNoCostLocation);
					ADD_FILTER(mfPatApptLocation);
					ADD_FILTER(mfNoPatApptLocation);
					ADD_FILTER(mfPatNoApptLocation);
					ADD_FILTER(mfPayLocation);
					ADD_FILTER(mfChargeLocation);
					ADD_FILTER(mfGraphDependant);
				break;

				case mftProvider:
					ADD_FILTER(mfPatientProvider);
					ADD_FILTER(mfTransProvider);
					ADD_FILTER(mfApptProvider);
					ADD_FILTER(mfNoPatApptProvider);
					ADD_FILTER(mfPatNoApptProvider);
					ADD_FILTER(mfChargeProvider);
					ADD_FILTER(mfPayProvider);
					ADD_FILTER(mfDependantProvider);
				break;

				default:
					ASSERT(FALSE);
				break;
			}

			
		}

		//set the value to be what ever the tab had before
		if (m_pMarketFilter->SetSelByColumn(0, (long)m_Filter) == -1) {
			m_pMarketFilter->CurSel = 0;
		}
	}
	NxCatchAll("Error in CMarketFilterPickerDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CMarketFilterPickerDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMarketFilterPickerDlg)
	ON_EVENT(CMarketFilterPickerDlg, IDC_MARKET_FILTER_LIST, 16 /* SelChosen */, OnSelChosenMarketFilterList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMarketFilterPickerDlg::OnSelChosenMarketFilterList(long nRow) 
{
	if(nRow == -1) {
		m_pMarketFilter->CurSel = 0;
	}
	
}
