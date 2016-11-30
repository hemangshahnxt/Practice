// HL7DuplicateRefPhysDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "HL7DuplicateRefPhysDlg.h"
#include <NxHL7Lib\HL7CommonTypes.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHL7DuplicateRefPhysDlg dialog


CHL7DuplicateRefPhysDlg::CHL7DuplicateRefPhysDlg(CWnd* pParent)
	: CNxDialog(CHL7DuplicateRefPhysDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHL7DuplicateRefPhysDlg)
		// NOTE: the ClassWizard will add member initialization here
	m_nID = -1;
	m_nHL7GroupID = -1;
	m_bIsRefPhys = true;
	m_strRecord = "Patient";
	//}}AFX_DATA_INIT
}


void CHL7DuplicateRefPhysDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHL7DuplicateRefPhysDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnNoRefPhy);
	DDX_Control(pDX, IDC_PHYS_TEXT, m_nxstaticPhysText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHL7DuplicateRefPhysDlg, CNxDialog)
	//{{AFX_MSG_MAP(CHL7DuplicateRefPhysDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHL7DuplicateRefPhysDlg message handlers

BOOL CHL7DuplicateRefPhysDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (j.jones 2008-05-08 09:42) - PLID 29953 - added nxiconbuttons for modernization
	m_btnOK.AutoSet(NXB_OK);
	m_btnNoRefPhy.AutoSet(NXB_MODIFY); //modify, because this action will change the imported content

	m_pList = BindNxDataListCtrl(IDC_PHYS_LIST, false);

	//query the list
	CString strDemographicsCriteria;
	strDemographicsCriteria.Format("PersonT.Last = '%s' AND PersonT.First = '%s'", _Q(m_strLast), _Q(m_strFirst));
	if (m_bUseTitle)
	{
		CString str;
		str.Format(" AND PersonT.Title = '%s'", _Q(m_strTitle));
		strDemographicsCriteria += str;
	}

	CString strDerivedPersonTableName;
	HL7IDLink_RecordType eRecordType;

	if (m_bIsRefPhys)
	{
		strDerivedPersonTableName = "ReferringPhysT";
		eRecordType = hilrtReferringPhysician;
	}
	else
	{
		strDerivedPersonTableName = "ProvidersT";
		eRecordType = hilrtProvider;
	}

	// Display all physician records that match by demographics and also,
	// since the duplicates error can be triggered by mulitple of the same HL7IDLinkT.ThirdPartyID
	// pointing to different Practice PersonIDs, include any physicians linked to HL7IDLinkT records
	// matching the third party ID.
	CString strFrom = FormatString(R"(
(
	SELECT 
		PersonT.ID, 
		PersonT.Last,
		PersonT.First,
		PersonT.Middle, 
		PersonT.Address1, 
		PersonT.City, 
		PersonT.State, 
		PersonT.Zip, 
		%s.UPIN 
	FROM %s 
	INNER JOIN PersonT ON %s.PersonID = PersonT.ID
	LEFT JOIN
	(
		SELECT DISTINCT
			HL7IDLinkT.PersonID
		FROM HL7IDLinkT
		WHERE HL7IDLinkT.GroupID = %li
			AND HL7IDLinkT.ThirdPartyID = '%s'
			AND HL7IDLinkT.RecordType = %li
	) LinkedPersonsQ ON PersonT.ID = LinkedPersonsQ.PersonID
	WHERE LinkedPersonsQ.PersonID IS NOT NULL
		OR (%s)
) SubQ
)", strDerivedPersonTableName, strDerivedPersonTableName, strDerivedPersonTableName,
m_nHL7GroupID, _Q(m_strThirdPartyID), eRecordType,
strDemographicsCriteria);
	m_pList->PutFromClause(_bstr_t(strFrom));

	m_pList->Requery();

	//update the description
	CString str;
	//TES 10/8/2008 - PLID 31414 - Change the caption based on whether we are searching providers or referring physicians,
	// and include the record descriptor we were given.
	str.Format("The %s for an imported %s is named '%s %s'.  Please choose the NexTech Practice physician this corresponds to.", m_bIsRefPhys ? "Referring Physician" : "Provider", m_strRecord, m_strFirst, m_strLast);

	SetDlgItemText(IDC_PHYS_TEXT, str);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//Returns the ID number chosen by the user.  -1 if they chose to continue with no referral
//TES 10/8/2008 - PLID 31414 - Added strRecord, for the user-readable type of the
// record the physician will be assigned to, and bIsRefPhys, which can be set to
// false to have this dialog search for Providers, rather than Referring Physicians.
long CHL7DuplicateRefPhysDlg::Open(long nHL7GroupID, CString strThirdPartyID, CString strFirst, CString strLast, CString strTitle, bool bUseTitle, CString strRecord /*= "Patient"*/, bool bIsRefPhys /*= true*/)
{
	m_nHL7GroupID = nHL7GroupID;
	m_strThirdPartyID = strThirdPartyID;
	m_strFirst = strFirst;
	m_strLast = strLast;
	m_strTitle = strTitle;
	m_bUseTitle = bUseTitle;
	m_strRecord = strRecord;
	m_bIsRefPhys = bIsRefPhys;

	DoModal();

	return m_nID;
}

void CHL7DuplicateRefPhysDlg::OnOK() 
{
	long nCurSel = m_pList->GetCurSel();
	if(nCurSel == -1) {
		MsgBox("You must choose an item before closing.");
		return;
	}

	m_nID = VarLong(m_pList->GetValue(nCurSel, 0));

	CDialog::OnOK();
}

void CHL7DuplicateRefPhysDlg::OnCancel() 
{
	m_nID = -1;

	CDialog::OnCancel();
}
