// TopazSigPadSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "TopazSigPadSettingsDlg.h"
#include "practiceRc.h"


// CTopazSigPadSettingsDlg dialog
// (d.singleton 2013-05-03 09:11) - PLID 56520 - need to be able to set the COM port the Topaz sig pad is using.  Then read that value when initializing the pad.

IMPLEMENT_DYNAMIC(CTopazSigPadSettingsDlg, CNxDialog)

using namespace NXDATALIST2Lib;

CTopazSigPadSettingsDlg::CTopazSigPadSettingsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CTopazSigPadSettingsDlg::IDD, pParent)
{
	long m_nOriginalPort = -1;
	long m_nOriginalStatus = -1;
	long m_nCurrentPort = -1;
	long m_nCurrentStatus = -1;
}

CTopazSigPadSettingsDlg::~CTopazSigPadSettingsDlg()
{
}

void CTopazSigPadSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RADIO_TOPAZ_ON, m_nxbOn);
	DDX_Control(pDX, IDC_RADIO_TOPAZ_OFF, m_nxbOff);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
}


BEGIN_MESSAGE_MAP(CTopazSigPadSettingsDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CTopazSigPadSettingsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO_TOPAZ_ON, &CTopazSigPadSettingsDlg::OnBnClickedRadioTopazOn)
	ON_BN_CLICKED(IDC_RADIO_TOPAZ_OFF, &CTopazSigPadSettingsDlg::OnBnClickedRadioTopazOff)
END_MESSAGE_MAP()

enum ComPorts
{
	cpID = 0,
	cpComPort,
};

// CTopazSigPadSettingsDlg message handlers
BOOL CTopazSigPadSettingsDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	try {
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		//Datalists
		m_dlComPorts = BindNxDataList2Ctrl(IDC_COM_PORTS, false);
		//Add rows
		CString strPort;
		IRowSettingsPtr pRow;
		for(int i = 1; i <= 9; i++) {
			pRow = m_dlComPorts->GetNewRow();
			pRow->PutValue(cpID, (long)i);
			strPort.Format("COM%li:", i);
			pRow->PutValue(cpComPort, _bstr_t(strPort));
			m_dlComPorts->AddRowSorted(pRow, NULL);
		}
		//load saved values
		m_nOriginalPort = GetPropertyInt("TopazSigPadPortNumber", -1, 0);
		//set our current value
		m_nCurrentPort = m_nOriginalPort;
		if(m_nCurrentPort > -1) {
			m_dlComPorts->SetSelByColumn(cpID, m_nOriginalPort);
		}
		m_nOriginalStatus = GetPropertyInt("IsTopazSigPadOn", 0, 0);
		//set current value
		m_nCurrentStatus = m_nOriginalStatus;
		if(m_nCurrentStatus == 1) {
			m_nxbOn.SetCheck(1);
			m_nxbOff.SetCheck(0);
		}
		else {
			m_nxbOn.SetCheck(0);
			m_nxbOff.SetCheck(1);
		}
		RefreshControls(m_nCurrentStatus == 0 ? FALSE : TRUE);
	}
	NxCatchAll(__FUNCTION__);
	return TRUE;
}

// (d.singleton 2013-05-03 09:11) - PLID 56520 - save if anything changed
void CTopazSigPadSettingsDlg::OnBnClickedOk()
{
	try {
		//save status first
		m_nCurrentStatus = m_nxbOn.GetCheck();
		
		//only worry about com port if status is on
		if(m_nCurrentStatus != 0) {
			IRowSettingsPtr pRow = m_dlComPorts->GetCurSel();
			if(!pRow) {
				//make sure they select a row if trying to save
				AfxMessageBox("Please select a COM port before saving.");
				return;
			}
			//save only if we have changes
			m_nCurrentPort = VarLong(pRow->GetValue(cpID));		
			if(m_nCurrentPort != m_nOriginalPort) {
				SetPropertyInt("TopazSigPadPortNumber", m_nCurrentPort);
			}
		}
		//save status if different
		if(m_nCurrentStatus != m_nOriginalStatus) {
			SetPropertyInt("IsTopazSigPadOn", m_nCurrentStatus);
		}
	}NxCatchAll(__FUNCTION__);	
	CNxDialog::OnOK();
}

void CTopazSigPadSettingsDlg::OnBnClickedRadioTopazOn()
{
	try {
		m_nCurrentStatus = m_nxbOn.GetCheck();
		RefreshControls(m_nCurrentStatus == 0 ? FALSE : TRUE);
	}NxCatchAll(__FUNCTION__);
}

void CTopazSigPadSettingsDlg::OnBnClickedRadioTopazOff()
{
	try {
		m_nCurrentStatus = m_nxbOn.GetCheck();
		RefreshControls(m_nCurrentStatus == 0 ? FALSE : TRUE);
	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CTopazSigPadSettingsDlg, CNxDialog)
	ON_EVENT(CTopazSigPadSettingsDlg, IDC_COM_PORTS, 16, CTopazSigPadSettingsDlg::SelChosenComPorts, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CTopazSigPadSettingsDlg::SelChosenComPorts(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(!pRow) {
			return;
		}
		m_nCurrentPort = VarLong(pRow->GetValue(cpID));
	}NxCatchAll(__FUNCTION__);
}

void CTopazSigPadSettingsDlg::RefreshControls(BOOL bShow)
{
	m_dlComPorts->PutEnabled(bShow);
}