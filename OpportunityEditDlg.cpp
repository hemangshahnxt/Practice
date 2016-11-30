// OpportunityEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "OpportunityEditDlg.h"
#include "OpportunityProposalDlg.h"
#include "MultiSelectDlg.h"
#include "InternationalUtils.h"
#include "GlobalUtils.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum eProposalColumns {
	epcID = 0,
	epcQuoteNum,
	epcDate,
	epcExpires,
	epcTotal,
	epcDiscount,
	epcIsActive,
	epcMergePath, // (z.manning, 11/05/2007) - PLID 27972 - Added a column for merged document path.
	epcSubtotal, // (z.manning, 11/06/2007) - PLID 27972 - Added column for subtotal
};

#define ACTIVE_PROP_HIGHLIGHT_COLOR	RGB(10, 140, 10)


long GetSelectedIntValue(NXDATALIST2Lib::_DNxDataListPtr pList, short nColumn, long nDefaultValue)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = pList->CurSel;
	if(pRow == NULL)
		return nDefaultValue;

	return VarLong(pRow->GetValue(nColumn), nDefaultValue);
}

CString GetSelectedStringValue(NXDATALIST2Lib::_DNxDataListPtr pList, short nColumn, CString strDefaultValue)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = pList->CurSel;
	if(pRow == NULL)
		return strDefaultValue;

	return VarString(pRow->GetValue(nColumn), strDefaultValue);
}


/////////////////////////////////////////////////////////////////////////////
// COpportunityEditDlg dialog

void CopyArray(IN CDWordArray* arySource, OUT CDWordArray* aryDest)
{
	for(int i = 0; i < arySource->GetSize(); i++) {
		aryDest->Add(arySource->GetAt(i));
	}
}

BOOL IsAddOn(long nTypeID)
{
	//TODO: Need a better way to determine if a sale is an addon rather than just hardcoded value here.
	return (nTypeID == 2);
}

//Given a start and ending array, returns 2 arrays 
//	- aryNew is everything added in the second that was not in the first
//	- aryRemoved is everything removed from the second that was in the first
//This function only adds to aryNew and aryRemoved, they may already have some elements when this function is called
void FindChangedElements(IN CDWordArray* aryFirst, IN CDWordArray* arySecond, OUT CDWordArray* aryNew, OUT CDWordArray* aryRemoved)
{
	long nFirstSize = aryFirst->GetSize();
	long nSecondSize = arySecond->GetSize();

	//Shortcuts - save some looping time
	//	1)  If the first is empty, then everything in the second is new
	if(nFirstSize == 0) {
		//Copy
		CopyArray(arySecond, aryNew);
		return;
	}

	//2)  If the second is empty, then everything was removed
	if(nSecondSize == 0) {
		//Copy
		CopyArray(aryFirst, aryRemoved);
		return;
	}

	//We now want to loop through and see what is different
	//	TODO:  There should be a way to make this faster!  Perhaps if we sort the arrays?  For now, the arrays in this dialog should never have more
	//	than half a dozen entries, so this is manageable.
	for(int i = 0; i < nFirstSize; i++) {
		long nID = aryFirst->GetAt(i);
		if(IsIDInArray(nID, arySecond)) {
			//It is, skip it
		}
		else {
			//It is not in the second array, so it must have been removed
			aryRemoved->Add(nID);
		}
	}

	//Do the same for new items
	for(i = 0; i < nSecondSize; i++) {
		long nID = arySecond->GetAt(i);
		if(IsIDInArray(nID, aryFirst)) {
			//It is, skip it
		}
		else {
			//It is not in the first array, so must be new
			aryNew->Add(nID);
		}
	}
}

//This is only kept in this dialog, no need to take it outside
#define	NXM_LOAD_OPPORTUNITY		WM_USER + 10001

COpportunityEditDlg::COpportunityEditDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(COpportunityEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COpportunityEditDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nID = -1;
	m_nSavedClientID = GetActivePatientID();
	m_nSavedTypeID = -1;
	m_strSavedName = "";
	m_nSavedPriCoordID = -1;
	m_nSavedSecCoordID = -1;
	m_arySavedAssociates.RemoveAll();
	m_dtSavedCloseDate = COleDateTime::GetCurrentTime();
	m_arySavedCompetition.RemoveAll();
	m_arySavedAlliance.RemoveAll();
	m_strSavedNotes = "";
	m_aryCurrentAssociates.RemoveAll();
	m_aryCurrentCompetition.RemoveAll();
	m_aryCurrentAlliance.RemoveAll();
	m_cySavedEstPrice = COleCurrency(0, 0);
	m_strSavedQBEstimate = "";
	m_nSavedPayMethodID = -1;
	m_bUserModifiedOppName = FALSE;
	m_bSavedDiscussedHardware = FALSE;
	//m_bSavedITPersonKnown = FALSE;		// (d.lange 2010-11-09 12:25) - PLID 41335 - Removed this control

	//TODO:  These are currently hardcoded to the ID in the tables.  What better method can we come up with?
	m_nSavedCategoryID = 1;		//Suspect
	m_nSavedStageID = 1;		//Suspect
	m_nSavedTypeID = 1;			//New Sale
}

void COpportunityEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpportunityEditDlg)
	DDX_Control(pDX, IDC_OPP_PREVIOUS, m_btnPrevious);
	DDX_Control(pDX, IDC_OPP_NEXT, m_btnNext);
	DDX_Control(pDX, IDC_OPP_EMAIL_SUMMARY, m_btnEmail);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_ADD_PROPOSAL, m_btnAddProposal);
	DDX_Control(pDX, IDC_COMPETITION_LABEL, m_nxlCompetitionLabel);
	DDX_Control(pDX, IDC_ASSOCIATE_LABEL, m_nxlAssociateLabel);
	DDX_Control(pDX, IDC_ALLIANCE_LABEL, m_nxlAllianceLabel);
	DDX_Control(pDX, IDC_CLOSE_DATE, m_pickerCloseDate);
	DDX_Control(pDX, IDC_DISCUSSED_HW, m_btnDiscussedHardware);
	//DDX_Control(pDX, IDC_IT_PERSON_IDENTIFIED, m_btnITPersonKnown);
	DDX_Control(pDX, IDC_OPP_NAME, m_nxeditOppName);
	DDX_Control(pDX, IDC_PRICE_ESTIMATE, m_nxeditPriceEstimate);
	DDX_Control(pDX, IDC_QB_ESTIMATE, m_nxeditQbEstimate);
	DDX_Control(pDX, IDC_OPP_NOTES, m_nxeditOppNotes);
	DDX_Control(pDX, IDC_LOST_REASON, m_nxeditLostReason);
	//DDX_Control(pDX, IDC_IT_PERSON_NAME, m_nxeditItPersonName);
	DDX_Control(pDX, IDC_IT_NOTES, m_nxeditItNotes);
	DDX_Control(pDX, IDC_LOST_REASON_LABEL, m_nxstaticLostReasonLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COpportunityEditDlg, CNxDialog)
	//{{AFX_MSG_MAP(COpportunityEditDlg)
	ON_BN_CLICKED(IDC_ADD_PROPOSAL, OnAddProposal)
	ON_BN_CLICKED(IDC_OPP_EMAIL_SUMMARY, OnOppEmailSummary)
	ON_BN_CLICKED(IDC_OPP_PREVIOUS, OnOppPrevious)
	ON_BN_CLICKED(IDC_OPP_NEXT, OnOppNext)
	ON_MESSAGE(NXM_LOAD_OPPORTUNITY, OnLoadOpportunity)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
	ON_EN_KILLFOCUS(IDC_PRICE_ESTIMATE, OnKillfocusPriceEstimate)
	ON_EN_CHANGE(IDC_OPP_NAME, OnChangeOppName)
	//ON_BN_CLICKED(IDC_IT_PERSON_IDENTIFIED, OnITPersonKnown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpportunityEditDlg message handlers

BOOL COpportunityEditDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//Bind all datalists
		m_pClient = BindNxDataList2Ctrl(this, IDC_OPP_CLIENT, GetRemoteData(), false);	//Don't requery until the loading starts
		m_pType = BindNxDataList2Ctrl(this, IDC_OPP_TYPE, GetRemoteData(), true);
		m_pPriCoord = BindNxDataList2Ctrl(this, IDC_OPP_PRI_COORD, GetRemoteData(), true);
		m_pSecCoord = BindNxDataList2Ctrl(this, IDC_OPP_SEC_COORD, GetRemoteData(), true);
		m_pAssociate = BindNxDataList2Ctrl(this, IDC_OPP_ASSOCIATE, GetRemoteData(), true);
		//m_pCategory = BindNxDataList2Ctrl(this, IDC_OPP_CATEGORY, GetRemoteData(), true);		// (d.lange 2010-11-11 10:01) - PLID 41442 - Removed the Category dropdown
		m_pStage = BindNxDataList2Ctrl(this, IDC_OPP_STAGE, GetRemoteData(), true);
		m_pCompetition = BindNxDataList2Ctrl(this, IDC_OPP_COMPETITION, GetRemoteData(), true);
		m_pAlliance = BindNxDataList2Ctrl(this, IDC_OPP_ALLIANCE, GetRemoteData(), true);
		m_pProposalList = BindNxDataList2Ctrl(this, IDC_OPP_PROPOSAL_LIST, GetRemoteData(), false);
		m_pPayMethod = BindNxDataList2Ctrl(this, IDC_OPP_PAY_METHOD, GetRemoteData(), true);
		// (d.lange 2010-11-09 09:54) - PLID 41335 - Bind the IT Contact list
		m_pITContact = BindNxDataList2Ctrl(this, IDC_ITCONTACT_LIST, GetRemoteData(), true);

		//Setup labels as hyperlinks for multi selects
		m_nxlAssociateLabel.SetType(dtsHyperlink);
		m_nxlAllianceLabel.SetType(dtsHyperlink);
		m_nxlCompetitionLabel.SetType(dtsHyperlink);

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		// (z.manning, 12/11/2007) - PLID 27972 - Add a <No Coordinator> option for primary coordinator.
		pRow = m_pPriCoord->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("<No Coordinator>"));
		m_pPriCoord->AddRowBefore(pRow, m_pPriCoord->GetFirstRow());

		//Add a <No Coordinator> option for secondary
		pRow = m_pSecCoord->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("<No Coordinator>"));
		m_pSecCoord->AddRowBefore(pRow, m_pSecCoord->GetFirstRow());

		//Add a <Multiple> and <No Associate> option for associates
		pRow = m_pAssociate->GetNewRow();
		pRow->PutValue(0, (long)-2);
		pRow->PutValue(1, _bstr_t("<Multiple Managers>"));
		m_pAssociate->AddRowBefore(pRow, m_pAssociate->GetFirstRow());

		pRow = m_pAssociate->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("<No Manager>"));
		m_pAssociate->AddRowBefore(pRow, m_pAssociate->GetFirstRow());

		//Add a <Multiple> and <No Competition> option for competition
		pRow = m_pCompetition->GetNewRow();
		pRow->PutValue(0, (long)-2);
		pRow->PutValue(1, _bstr_t("<Multiple Competitors>"));
		m_pCompetition->AddRowBefore(pRow, m_pCompetition->GetFirstRow());

		pRow = m_pCompetition->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("<No Competition>"));
		m_pCompetition->AddRowBefore(pRow, m_pCompetition->GetFirstRow());

		//Add a <Multiple> and <No Alliance> option for alliance
		pRow = m_pAlliance->GetNewRow();
		pRow->PutValue(0, (long)-2);
		pRow->PutValue(1, _bstr_t("<Multiple Alliances>"));
		m_pAlliance->AddRowBefore(pRow, m_pAlliance->GetFirstRow());

		pRow = m_pAlliance->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("<No Alliance>"));
		m_pAlliance->AddRowBefore(pRow, m_pAlliance->GetFirstRow());

		// (z.manning, 11/05/2007) - PLID 27972 - Add no selection option for payment method.
		pRow = m_pPayMethod->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, "<None Selected>");
		m_pPayMethod->AddRowBefore(pRow, m_pPayMethod->GetFirstRow());

		//Do some coloring to liven things up
		//TODO - While sales is still nitpicking over colors, this is loaded by a non-cached configrt
		//	proprty so I can change it on the fly.
		g_propManager.EnableCaching(FALSE);
		DWORD dwColor = GetRemotePropertyInt("InternalPropBGColor", 10542240, 0, "<None>", false);
		g_propManager.EnableCaching(TRUE);
		//Note:  I made a bunch of these and ended up not using #1, so it's gone
		((CNxColor*)GetDlgItem(IDC_OPP_EDIT_COLOR2))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_OPP_EDIT_COLOR3))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_OPP_EDIT_COLOR4))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_OPP_EDIT_COLOR5))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_OPP_EDIT_COLOR6))->SetColor(dwColor);

		m_nxlAssociateLabel.SetColor(dwColor);
		m_nxlAllianceLabel.SetColor(dwColor);
		m_nxlCompetitionLabel.SetColor(dwColor);
		
		//NxIconButton setup
		m_btnPrevious.AutoSet(NXB_LEFT);
		m_btnNext.AutoSet(NXB_RIGHT);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnOK.AutoSet(NXB_OK);
		m_btnAddProposal.AutoSet(NXB_NEW);

		((CNxEdit*)GetDlgItem(IDC_OPP_NAME))->SetLimitText(255);
		//((CNxEdit*)GetDlgItem(IDC_IT_PERSON_NAME))->SetLimitText(255);

		//Post a message to start the load, this gives the appearance of speed by letting the dialog draw
		//	before the loading halts message pumping again.
		PostMessage(NXM_LOAD_OPPORTUNITY, 0, 0);

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//Set the ID to be loaded.  This should only be called before the dialog is shown
void COpportunityEditDlg::SetID(long nIDToLoad)
{
	m_nID = nIDToLoad;
}

//This is called from OnInitDialog, via a posted message
LRESULT COpportunityEditDlg::OnLoadOpportunity(WPARAM wParam, LPARAM lParam)
{
	try {
		//We put this here because it's very slow (at least in debug mode), and the whole dialog just "feels"
		//	better if we let it pop up immediately then start the requery.
		m_pClient->Requery();

		if(m_nID != -1) {
			//Existing opportunity.  Load all of our pre-saved values
			_CommandPtr pCmd = OpenParamQuery(
				"SELECT PersonID, TypeID, Name, PrimaryCoordID, SecondaryCoordID, LostReason " //, CategoryID "	// (d.lange 2010-11-11 10:07) - PLID 41442 - Removed Category dropdown
				"	, CurrentStageID, Notes, EstCloseDate, EstPrice, QBEstimate, PayMethod, DiscussedHardware "
				"	, ITPersonKnown, ITPersonName, ITNotes "
				"FROM OpportunitiesT WHERE ID = ?;\r\n"
				"SELECT AssocID, Username FROM OpportunityAssociatesT INNER JOIN UsersT ON OpportunityAssociatesT.AssocID = UsersT.PersonID WHERE OpportunityID = ? ORDER BY UserName;\r\n"
				"SELECT PartnerID, Name FROM OpportunityAllianceT INNER JOIN AlliancePartnersT ON OpportunityAllianceT.PartnerID = AlliancePartnersT.ID WHERE OpportunityID = ? ORDER BY Name;\r\n"
				"SELECT CompetitorID, Name FROM OpportunityCompetitionT INNER JOIN CompetitorsT ON OpportunityCompetitionT.CompetitorID = CompetitorsT.ID WHERE OpportunityID = ? ORDER BY Name;\r\n");
			AddParameterLong(pCmd, "OpportunityID", m_nID);
			AddParameterLong(pCmd, "OpportunityID", m_nID);
			AddParameterLong(pCmd, "OpportunityID", m_nID);
			AddParameterLong(pCmd, "OpportunityID", m_nID);
			_RecordsetPtr prsLoad = CreateRecordset(pCmd);
			if(!prsLoad->eof) {
				FieldsPtr pFields = prsLoad->Fields;

				//
				//Query 1 - All opportunity Data
				//PersonID - Not allowed to be NULL
				m_nSavedClientID = AdoFldLong(pFields, "PersonID");

				//TypeID - Not allowed to be NULL
				m_nSavedTypeID = AdoFldLong(pFields, "TypeID");

				//Name - Not allowed to be NULL
				m_strSavedName = AdoFldString(pFields, "Name");

				//Primary Coordinator - Not allowed to be NULL
				// (z.manning, 12/11/2007) - PLID 27972 - Now it can be NULL
				m_nSavedPriCoordID = AdoFldLong(pFields, "PrimaryCoordID", -1);

				//Secondary Coordinator - Can be NULL
				m_nSavedSecCoordID = AdoFldLong(pFields, "SecondaryCoordID", -1);

				//Category - Not allowed to be NULL
				//m_nSavedCategoryID = AdoFldLong(pFields, "CategoryID");		// (d.lange 2010-11-11 10:07) - PLID 41442 - Removed Category dropdown

				//Reason for lost opportunity
				m_strSavedLostReason = AdoFldString(pFields, "LostReason");

				//Current stage - Not allowed to be NULL
				m_nSavedStageID = AdoFldLong(pFields, "CurrentStageID");

				//Notes
				m_strSavedNotes = AdoFldString(pFields, "Notes");

				//Estimated Closing Date - Not allowed to be NULL
				m_dtSavedCloseDate = AdoFldDateTime(pFields, "EstCloseDate");

				//Estimated price - not NULL
				m_cySavedEstPrice = AdoFldCurrency(pFields, "EstPrice");

				//Quickbooks estimate number - not null
				m_strSavedQBEstimate = AdoFldString(pFields, "QBEstimate");

				// (z.manning, 11/05/2007) - PLID 27972 - PayMethod ID - Can be NULL
				m_nSavedPayMethodID = AdoFldLong(pFields, "PayMethod", -1);

				// (z.manning, 11/06/2007) - PLID 27972 - Hardware/IT information, none are NULLable
				m_bSavedDiscussedHardware = AdoFldBool(pFields, "DiscussedHardware");
				//m_bSavedITPersonKnown = AdoFldBool(pFields, "ITPersonKnown");		// (d.lange 2010-11-09 12:25) - PLID 41335 - Removed this control
				//m_strSavedITPersonName = AdoFldString(pFields, "ITPersonName");	// (d.lange 2010-11-09 12:25) - PLID 41335 - Removed this control
				m_strSavedITNotes = AdoFldString(pFields, "ITNotes");

				//
				//Query 2 - All associates
				prsLoad = prsLoad->NextRecordset(NULL);

				CString strAssoc;
				while(!prsLoad->eof) {
					long nID = AdoFldLong(prsLoad, "AssocID");
					m_arySavedAssociates.Add(nID);
					m_aryCurrentAssociates.Add(nID);
					strAssoc += AdoFldString(prsLoad, "Username") + ", ";
					prsLoad->MoveNext();
				}
				//Trim the trailing ", "
				strAssoc = strAssoc.Left(strAssoc.GetLength() - 2);

				//Load initial text
				m_nxlAssociateLabel.SetText(strAssoc);

				//
				//Query 3 - All partners
				prsLoad = prsLoad->NextRecordset(NULL);

				CString strAlliance;
				while(!prsLoad->eof) {
					long nID = AdoFldLong(prsLoad, "PartnerID");
					m_arySavedAlliance.Add(nID);
					m_aryCurrentAlliance.Add(nID);
					strAlliance += AdoFldString(prsLoad, "Name") + ", ";
					prsLoad->MoveNext();
				}
				//Trim the trailing ", "
				strAlliance = strAlliance.Left(strAlliance.GetLength() - 2);

				//Load initial text
				m_nxlAllianceLabel.SetText(strAlliance);

				//
				//Query 4 - All competition
				prsLoad = prsLoad->NextRecordset(NULL);

				CString strComp;
				while(!prsLoad->eof) {
					long nID = AdoFldLong(prsLoad, "CompetitorID");
					m_arySavedCompetition.Add(nID);
					m_aryCurrentCompetition.Add(nID);
					strComp += AdoFldString(prsLoad, "Name") + ", ";
					prsLoad->MoveNext();
				}
				//Trim the trailing ", "
				strComp = strComp.Left(strComp.GetLength() - 2);

				//Load initial text
				m_nxlCompetitionLabel.SetText(strComp);

				//Setup the WHERE clause of the proposal list for this ID only
				CString strWhere;
				strWhere.Format("OpportunityID = %li", m_nID);
				m_pProposalList->WhereClause = _bstr_t(strWhere);
				m_pProposalList->Requery();
			}
			else {
				//EOF, but we're loading existing.  Failure
				AfxThrowNxException("Invalid Opportunity ID %li when attempting to load opportunity.", m_nID);
			}
		}


		//At this point we are ready to load either new or existing, doesn't matter.  We just use our saved
		//	member variable status, which are defaulted in the constructor to the appropriate "new opportunity"
		//	values.
		//
		//	TODO:  Bob has not implemented the TrySetSelFinished handlers in the Datalist2.  Thus, it is not really safe
		//	to be using this function, because we cannot properly handle the return value.  That said, I'm using it anyways, 
		//	because using a SetSelByColumn here is just way too slow when loading this dialog.  If for some reason the client
		//	fails to load, the user will be unable to save this dialog anyways, because the client must be selected for
		//	the save to go through.
		//	When this functionality is available, we should use it here.
		//
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		m_pClient->TrySetSelByColumn_Deprecated(0, m_nSavedClientID);
		if(m_pType->SetSelByColumn(0, m_nSavedTypeID) == NULL) {
			MessageBox("Failed to load the saved opportunity type.  Please contact NexTech Technical Support.");
		}
		if(m_pPriCoord->SetSelByColumn(0, m_nSavedPriCoordID) == NULL) {
			MessageBox("Failed to load the saved opportunity primary coordinator.  Please contact NexTech Technical Support.");
		}
		if(m_pSecCoord->SetSelByColumn(0, m_nSavedSecCoordID) == NULL) {
			MessageBox("Failed to load the saved opportunity secondary coordinator.  Please contact NexTech Technical Support.");
		}
		// (d.lange 2010-11-11 10:07) - PLID 41442 - Removed Category dropdown
		/*if(m_pCategory->SetSelByColumn(0, m_nSavedCategoryID) == NULL) {
			MessageBox("Failed to load the saved opportunity category.  Please contact NexTech Technical Support.");
		}*/
		if(m_pStage->SetSelByColumn(0, m_nSavedStageID) == NULL) {
			MessageBox("Failed to load the saved opportunity stage.  Please contact NexTech Technical Support.");
		}
		if(m_pPayMethod->SetSelByColumn(0, m_nSavedPayMethodID) == NULL) {
			MessageBox("Failed to load the saved payment method.  Please contact NexTech Technical Support.");
		}
		// (z.manning, 11/06/2007) - PLID 27972 - Don't even try to set the name on a new opportunity
		// (which shouldn't matter cause it's blank anyway) because we default it to something when the
		// client list finishes requerying.
		if(m_nID != -1) {
			SetDlgItemText(IDC_OPP_NAME, m_strSavedName);
		}
		else {
			ASSERT(m_strSavedName.IsEmpty());
		}
		SetDlgItemText(IDC_LOST_REASON, m_strSavedLostReason);
		SetDlgItemText(IDC_OPP_NOTES, m_strSavedNotes);
		m_pickerCloseDate.SetValue(_variant_t(m_dtSavedCloseDate));
		SetDlgItemText(IDC_PRICE_ESTIMATE, FormatCurrencyForInterface(m_cySavedEstPrice));
		SetDlgItemText(IDC_QB_ESTIMATE, m_strSavedQBEstimate);

		// (z.manning, 11/06/2007) - PLID 27972 - Load hardware/IT information.
		if(m_bSavedDiscussedHardware) {
			CheckDlgButton(IDC_DISCUSSED_HW, BST_CHECKED);
		}
		// (d.lange 2010-11-09 12:25) - PLID 41335 - Removed this control
		/*if(m_bSavedITPersonKnown) {
			CheckDlgButton(IDC_IT_PERSON_IDENTIFIED, BST_CHECKED);
		}*/
		//SetDlgItemText(IDC_IT_PERSON_NAME, m_strSavedITPersonName);
		SetDlgItemText(IDC_IT_NOTES, m_strSavedITNotes);
		//OnITPersonKnown();

		//Display the associate label if more than 1
		EnsureAssociates();

		//Display the alliance label if more than 1
		EnsurePartners();

		//Display the competitor label if more than 1
		EnsureCompetitors();
		if(m_nID == -1) {
			// (z.manning, 11/05/2007) - PLID 27972 - This is a new opportunity, so by default we do not want
			// to select the "No Competition" row, but rather select nothing at all and require they choose
			// something themselves.
			ASSERT(m_aryCurrentCompetition.GetSize() == 0);
			m_pCompetition->PutCurSel(NULL);
		}

		//Lost reason display
		EnsureLostReason();

		//Set focus to the client list
		GetDlgItem(IDC_OPP_CLIENT)->SetFocus();

		// (d.lange 2010-11-09 12:13) - PLID 41335 - Grab the Contact's PersonID and try to set selection
		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT IntParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = 31", m_nSavedClientID);
		if(!rs->eof) {
			m_pITContact->SetSelByColumn((long)0, AdoFldLong(rs, "IntParam", -1));
		}
		rs->Close();

	} NxCatchAll("Error in OnLoadOpportunity");

	return 0;
}


//Apply all changes to data.  Returns true on success, false on failure.  Do not call this function
//	if you do not need to save.
bool COpportunityEditDlg::ApplyChanges()
{
	try {
		//
		//First, test all possible failure cases

		//Get all data into variables
		if(m_pClient->IsRequerying()) {
			//Can't save yet -- I only put this on the client datalist because it is larger
			//	and tends to get "stuck" while requerying, especially in debug mode.
			m_pClient->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		}
		long nClientID = GetSelectedIntValue(m_pClient, 0, -1);
		//This is required
		if(nClientID == -1) {
			MessageBox("You must choose a client before saving this opportunity.");
			return false;
		}

		long nTypeID = GetSelectedIntValue(m_pType, 0, -1);
		//This is required
		if(nTypeID == -1) {
			MessageBox("You must choose a type before saving this opportunity.");
			return false;
		}

		//Not required
		// (z.manning, 11/06/2007) - PLID 27972 - I beg to differ.
		CString strName;
		GetDlgItemText(IDC_OPP_NAME, strName);
		CString strTemp = strName;
		strTemp.TrimRight();
		if(strTemp.IsEmpty()) {
			MessageBox("You must enter a name for this opportunity.");
			return false;
		}

		long nPriCoordID = GetSelectedIntValue(m_pPriCoord, 0, -1);
		//This is required
		// (z.manning, 12/11/2007) - PLID 27972 - Not for addon sales
		if(!IsAddOn(nTypeID)) {
			if(nPriCoordID == -1) {
				MessageBox("You must choose a primary coordinator before saving this opportunity.");
				return false;
			}
		}
		else {
			// (z.manning, 12/11/2007) - PLID 27972 - However, add on sales must have at least one account manager.
			if(m_aryCurrentAssociates.GetSize() <= 0) {
				MessageBox("You must select at least one account manager for add on opportunities.");
				return false;
			}
		}

		//This is not required
		long nSecCoordID = GetSelectedIntValue(m_pSecCoord, 0, -1);

		//Required
		COleDateTime dtCloseDate = VarDateTime(m_pickerCloseDate.GetValue());

		// (d.lange 2010-11-11 10:07) - PLID 41442 - Removed Category dropdown
		/*long nCategoryID = GetSelectedIntValue(m_pCategory, 0, -1);
		//This is required
		if(nCategoryID == -1) {
			MessageBox("You must choose a category before saving this opportunity.");
			return false;
		}*/

		long nStageID = GetSelectedIntValue(m_pStage, 0, -1);
		//This is required
		if(nStageID == -1) {
			MessageBox("You must choose a stage before saving this opportunity.");
			return false;
		}

		// (z.manning, 11/05/2007) - PLID 27972 - We now require a competion selection, which may include
		// selecting <no competion>, which IS NOT the same as not having anything selected at all.
		if(m_aryCurrentCompetition.GetSize() == 0) {
			if(m_pCompetition->GetCurSel() == NULL) {
				MessageBox("You must select at least one competitor or <No Competition>.");
				return false;
			}
			else {
				// (z.manning, 11/05/2007) - There's no competitors selected but we do have a row. It better
				// be the no competition row.
				ASSERT(GetSelectedIntValue(m_pCompetition, 0, -2) == -1);
			}
		}

		CString strNotes;
		GetDlgItemText(IDC_OPP_NOTES, strNotes);

		//Lost reason, not required
		CString strLostReason;
		GetDlgItemText(IDC_LOST_REASON, strLostReason);

		//Estimated price.  KillFocus should have formatted this appropriately, so if it
		//	happens to be invalid, just reset to 0.
		COleCurrency cyEstPrice;
		{
			CString strEstPrice;
			GetDlgItemText(IDC_PRICE_ESTIMATE, strEstPrice);

			cyEstPrice.ParseCurrency(strEstPrice);
			if(cyEstPrice.GetStatus() != COleCurrency::valid)
				cyEstPrice = COleCurrency(0, 0);
		}

		CString strQBEstimate;
		GetDlgItemText(IDC_QB_ESTIMATE, strQBEstimate);

		//Check for a too-long name field.
		if(strName.GetLength() > 255) {
			MessageBox("You may not use a name greater than 255 characters.  Please shorten your name and try saving again.");
			return false;
		}

		// (z.manning, 11/05/2007) - PLID 27972 - Handle payment method.
		long nPayMethodID = GetSelectedIntValue(m_pPayMethod, 0, -1);
		CString strPayMethod = "NULL";
		if(nPayMethodID > 0) {
			strPayMethod = AsString(nPayMethodID);
		}

		// (z.manning, 11/06/2007) - PLID 27972 - Handle IT info.
		CString strITPersonName, strITNotes;
		GetDlgItemText(IDC_IT_PERSON_NAME, strITPersonName);
		GetDlgItemText(IDC_IT_NOTES, strITNotes);
		BOOL bDiscussedHardware = IsDlgButtonChecked(IDC_DISCUSSED_HW) == BST_CHECKED ? TRUE : FALSE;
		BOOL bITPersonKnown = IsDlgButtonChecked(IDC_IT_PERSON_IDENTIFIED) == BST_CHECKED ? TRUE : FALSE;

		//Notes and lost reason are NTEXT fields

		CString strPriCoord = "NULL";
		if(nPriCoordID > 0) {
			strPriCoord = AsString(nPriCoordID);
		}

		//
		//At this point, saving is approved.  Only 1 'return' statement should ever
		//	be implemented below this point.

		//
		//Generate save statement
		CString strSql;
		if(m_nID == -1) {
			//New opportunity, need to insert

			CString strFmtSecCoord = "NULL";
			if(nSecCoordID != -1)
				strFmtSecCoord.Format("%li", nSecCoordID);

			strSql.Format("SET NOCOUNT ON;\r\n"
				"DECLARE @ID int;\r\n"
				"SET @ID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM OpportunitiesT);\r\n"
				"INSERT INTO OpportunitiesT (ID, PersonID, TypeID, Name, PrimaryCoordID, SecondaryCoordID, LostReason, CategoryID, "
				"	CurrentStageID, Notes, EstCloseDate, EstPrice, QBEstimate, PayMethod, DiscussedHardware, ITPersonKnown, "
				"	ITPersonName, ITNotes) "
				"VALUES (@ID, %li, %li, '%s', %s, %s, '%s', %li, %li, '%s', '%s', '%s', '%s', %s, %i, %i, '%s', '%s');\r\n"
				"SET NOCOUNT OFF;\r\n"
				"SELECT @ID AS NewID;",
				nClientID, nTypeID, _Q(strName), strPriCoord, strFmtSecCoord, _Q(strLostReason), -1, nStageID, _Q(strNotes), 
				FormatDateTimeForSql(dtCloseDate), FormatCurrencyForSql(cyEstPrice), _Q(strQBEstimate), strPayMethod, bDiscussedHardware ? 1 : 0,
				bITPersonKnown ? 1 : 0, _Q(strITPersonName), _Q(strITNotes));

			//First time, insert a record into the stage history so we know this one was active
			CString str;
			str.Format("INSERT INTO OpportunityStageHistoryT (ID, OpportunityID, StageID, DateActive, ChangedByUserID) values (%li, @ID, %li, getdate(), %li);\r\n", 
				NewNumber("OpportunityStageHistoryT", "ID"), nStageID, GetCurrentUserID());
			strSql += str;

			//Associate(s) - First time save, we just insert everything
			if(m_aryCurrentAssociates.GetSize() > 0) {
				CString strAssoc;
				strAssoc.Format("DECLARE @OppAssocID int;\r\n"
					"SET @OppAssocID = (SELECT COALESCE(MAX(ID), 0) FROM OpportunityAssociatesT) + 1;\r\n");
				for(int i = 0; i < m_aryCurrentAssociates.GetSize(); i++) {
					str.Format("INSERT INTO OpportunityAssociatesT (ID, OpportunityID, AssocID) values (@OppAssocID, @ID, %li);\r\n"
						"SET @OppAssocID = @OppAssocID + 1;\r\n", m_aryCurrentAssociates.GetAt(i));
					strAssoc += str;
				}

				//Done, add it to our main insertion
				strSql += strAssoc;
			}

			//Competition - First time save, write all
			if(m_aryCurrentCompetition.GetSize() > 0) {
				CString strComp;
				strComp.Format("DECLARE @OppCompID int;\r\n"
					"SET @OppCompID = (SELECT COALESCE(MAX(ID), 0) FROM OpportunityCompetitionT) + 1;\r\n");
				for(int i = 0; i < m_aryCurrentCompetition.GetSize(); i++) {
					str.Format("INSERT INTO OpportunityCompetitionT (ID, OpportunityID, CompetitorID) values (@OppCompID, @ID, %li);\r\n"
						"SET @OppCompID = @OppCompID + 1;\r\n", m_aryCurrentCompetition.GetAt(i));
					strComp += str;
				}

				//Done, add it to our main insertion
				strSql += strComp;
			}

			//Alliance - First time write
			if(m_aryCurrentAlliance.GetSize() > 0) {
				CString strAlliance;
				strAlliance.Format("DECLARE @OppAllianceID int;\r\n"
					"SET @OppAllianceID = (SELECT COALESCE(MAX(ID), 0) FROM OpportunityAllianceT) + 1;\r\n");
				for(int i = 0; i < m_aryCurrentAlliance.GetSize(); i++) {
					str.Format("INSERT INTO OpportunityAllianceT (ID, OpportunityID, PartnerID) values (@OppAllianceID, @ID, %li);\r\n"
						"SET @OppAllianceID = @OppAllianceID + 1;\r\n", m_aryCurrentAlliance.GetAt(i));
					strAlliance += str;
				}

				//Done, add it to our main insertion
				strSql += strAlliance;
			}

			// (d.lange 2010-11-09 11:50) - PLID 41335 - Save the IT Contact selected
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pITContact->GetCurSel();
				CString strITContact = "";
				if(pRow) {
					//Get the selected row's ID
					long nCurSel = pRow->GetValue(0);
					if(nCurSel != -1) {
						strITContact.Format("IF NOT EXISTS (SELECT * FROM CustomFieldDataT WHERE PersonID = %li AND FieldID = 31)\r\n "
										"BEGIN "
										"INSERT INTO CustomFieldDataT (PersonID, FieldID, IntParam) VALUES (%li, 31, %li)\r\n "
										"END "
										"ELSE BEGIN "
										"UPDATE CustomFieldDataT SET IntParam = %li WHERE PersonID = %li AND FieldID =  31\r\n "
										"END ", m_nSavedClientID, m_nSavedClientID, nCurSel, nCurSel, m_nSavedClientID);
					}
				}

				//Done, add to the main insertion
				strSql += strITContact;
			}

		}
		else {
			//Existing opportunity, we just need to update
			CString strFmtSecCoord = "NULL";
			if(nSecCoordID != -1)
				strFmtSecCoord.Format("%li", nSecCoordID);
			strSql.Format("UPDATE OpportunitiesT SET PersonID = %li, TypeID = %li, Name = '%s', PrimaryCoordID = %s, SecondaryCoordID = %s, "//CategoryID = %li, "
				"LostReason = '%s', CurrentStageID = %li, Notes = '%s', EstCloseDate = '%s', EstPrice = '%s', QBEstimate = '%s', PayMethod = %s, "
				"DiscussedHardware = %i, ITPersonKnown = %i, ITPersonName = '%s', ITNotes = '%s' "
				"WHERE OpportunitiesT.ID = %li;\r\n", 
				nClientID, nTypeID, _Q(strName), strPriCoord, strFmtSecCoord, /*nCategoryID,*/ _Q(strLostReason), nStageID, _Q(strNotes), 
				FormatDateTimeForSql(dtCloseDate), FormatCurrencyForSql(cyEstPrice), _Q(strQBEstimate), strPayMethod, bDiscussedHardware ? 1 : 0,
				bITPersonKnown ? 1 : 0, _Q(strITPersonName), _Q(strITNotes), m_nID);

			//If the Stage changes, we want to track it historically, with the date
			if(m_nSavedStageID != nStageID) {
				CString str;
				str.Format("INSERT INTO OpportunityStageHistoryT (ID, OpportunityID, StageID, DateActive, ChangedByUserID) values (%li, %li, %li, getdate(), %li);\r\n", 
					NewNumber("OpportunityStageHistoryT", "ID"), m_nID, nStageID, GetCurrentUserID());
				strSql += str;
			}

			//Save changes to associates
			{
				//We have the last saved and the current, just find the differences
				CDWordArray aryNew, aryRemoved;
				FindChangedElements(&m_arySavedAssociates, &m_aryCurrentAssociates, &aryNew, &aryRemoved);

				//Create insert statements for all the new elements
				CString strAssoc;
				strAssoc.Format("DECLARE @OppAssocID int;\r\n"
					"SET @OppAssocID = (SELECT COALESCE(MAX(ID), 0) FROM OpportunityAssociatesT) + 1;\r\n");
				for(int i = 0; i < aryNew.GetSize(); i++) {
					CString str;
					str.Format("INSERT INTO OpportunityAssociatesT (ID, OpportunityID, AssocID) values (@OppAssocID, %li, %li);\r\n"
						"SET @OppAssocID = @OppAssocID + 1;\r\n", m_nID, aryNew.GetAt(i));
					strAssoc += str;
				}

				//Now create deletion statements for everything removed
				for(i = 0; i < aryRemoved.GetSize(); i++) {
					CString str;
					str.Format("DELETE FROM OpportunityAssociatesT WHERE OpportunityID = %li AND AssocID = %li", m_nID, aryRemoved.GetAt(i));
					strAssoc += str;
				}

				//Done, add it to our main insertion
				strSql += strAssoc;
			}

			//Save changes to competition
			{
				//We have the last saved and the current, just find the differences
				CDWordArray aryNew, aryRemoved;
				FindChangedElements(&m_arySavedCompetition, &m_aryCurrentCompetition, &aryNew, &aryRemoved);

				//insert statements for all new
				CString strComp;
				strComp.Format("DECLARE @OppCompID int;\r\n"
					"SET @OppCompID = (SELECT COALESCE(MAX(ID), 0) FROM OpportunityCompetitionT) + 1;\r\n");
				for(int i = 0; i < aryNew.GetSize(); i++) {
					CString str;
					str.Format("INSERT INTO OpportunityCompetitionT (ID, OpportunityID, CompetitorID) values (@OppCompID, %li, %li);\r\n"
						"SET @OppCompID = @OppCompID + 1;\r\n", m_nID, aryNew.GetAt(i));
					strComp += str;
				}

				//deletion statements for removals
				for(i = 0; i < aryRemoved.GetSize(); i++) {
					CString str;
					str.Format("DELETE FROM OpportunityCompetitionT WHERE OpportunityID = %li AND CompetitorID = %li", m_nID, aryRemoved.GetAt(i));
					strComp += str;
				}

				//Done, add it to our main insertion
				strSql += strComp;
			}

			//Save changes to alliance
			{
				//We have the last saved and the current, just find the differences
				CDWordArray aryNew, aryRemoved;
				FindChangedElements(&m_arySavedAlliance, &m_aryCurrentAlliance, &aryNew, &aryRemoved);

				//Insert all new records
				CString strAlliance;
				strAlliance.Format("DECLARE @OppAllianceID int;\r\n"
					"SET @OppAllianceID = (SELECT COALESCE(MAX(ID), 0) FROM OpportunityAllianceT) + 1;\r\n");
				for(int i = 0; i < aryNew.GetSize(); i++) {
					CString str;
					str.Format("INSERT INTO OpportunityAllianceT (ID, OpportunityID, PartnerID) values (@OppAllianceID, %li, %li);\r\n"
						"SET @OppAllianceID = @OppAllianceID + 1;\r\n", m_nID, aryNew.GetAt(i));
					strAlliance += str;
				}

				//Handle all the removals
				for(i = 0; i < aryRemoved.GetSize(); i++) {
					CString str;
					str.Format("DELETE FROM OpportunityAllianceT WHERE OpportunityID = %li AND PartnerID = %li", m_nID, aryRemoved.GetAt(i));
					strAlliance += str;
				}

				//Done, add it to our main insertion
				strSql += strAlliance;
			}

			// (d.lange 2010-11-09 11:50) - PLID 41335 - Save the IT Contact selected
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pITContact->GetCurSel();
				CString strITContact = "";
				if(pRow) {
					//Get the selected row's ID
					long nCurSel = pRow->GetValue(0);
					if(nCurSel != -1) {
						strITContact.Format("IF NOT EXISTS (SELECT * FROM CustomFieldDataT WHERE PersonID = %li AND FieldID = 31)\r\n "
										"BEGIN "
										"INSERT INTO CustomFieldDataT (PersonID, FieldID, IntParam) VALUES (%li, 31, %li)\r\n "
										"END "
										"ELSE BEGIN "
										"UPDATE CustomFieldDataT SET IntParam = %li WHERE PersonID = %li AND FieldID =  31\r\n "
										"END ", m_nSavedClientID, m_nSavedClientID, nCurSel, nCurSel, m_nSavedClientID);
					}
				}

				//Done, add to the main insertion
				strSql += strITContact;
			}

		}

		//Run save statement
		// (a.walling 2010-09-08 13:16) - PLID 40377 - Use CSqlTransaction
		CSqlTransaction trans;
		trans.Begin();
		{
			_RecordsetPtr prsExec = CreateRecordsetStd(strSql);
			if(m_nID == -1) {
				//We need to pull the ID back out
				m_nID = AdoFldLong(prsExec, "NewID");

				//Setup the WHERE clause of the proposal list for this ID only for future requeries
				CString strWhere;
				strWhere.Format("OpportunityID = %li", m_nID);
				m_pProposalList->WhereClause = _bstr_t(strWhere);
			}
			else {
				//Existing opportunity, no need to get the ID
			}
		}
		// (a.walling 2010-09-08 13:16) - PLID 40377 - Use CSqlTransaction
		trans.Commit();

		//Update our "saved" member variables for comparisons if we save again.
		m_nSavedClientID = nClientID;
		m_nSavedTypeID = nTypeID;
		m_strSavedName = strName;
		m_nSavedPriCoordID = nPriCoordID;
		m_nSavedSecCoordID = nSecCoordID;
		CopyArray(&m_aryCurrentAssociates, &m_arySavedAssociates);
		m_dtSavedCloseDate = dtCloseDate;
		//m_nSavedCategoryID = nCategoryID;		// (d.lange 2010-11-11 10:07) - PLID 41442 - Removed Category dropdown
		m_nSavedStageID = nStageID;
		CopyArray(&m_aryCurrentCompetition, &m_arySavedCompetition);
		CopyArray(&m_aryCurrentAlliance, &m_arySavedAlliance);
		m_strSavedNotes = strNotes;
		m_strSavedLostReason = strLostReason;
		m_cySavedEstPrice = cyEstPrice;
		m_strSavedQBEstimate = strQBEstimate;
		m_nSavedPayMethodID = nPayMethodID;
		m_bSavedDiscussedHardware = bDiscussedHardware;
		//m_bSavedITPersonKnown = bITPersonKnown;	// (d.lange 2010-11-09 12:25) - PLID 41335 - Removed this control
		//m_strSavedITPersonName = strITPersonName;	// (d.lange 2010-11-09 12:25) - PLID 41335 - Removed this control
		m_strSavedITNotes = strITNotes;

		//Clear our changed flag
		m_bChanged = false;

		//Successfully committed our save
		return true;

	} NxCatchAll("Error in ApplyChanges");


	return false;
}

void COpportunityEditDlg::OnOK() 
{
	try {
		//Attempt to save
		if(!ApplyChanges()) {
			//Save failed, we must leave the dialog open
			return;
		}

		//Save succeeded, do any other cleanup here


		//Close dialog
		CDialog::OnOK();

	} NxCatchAll("Error in OnOK");
}

void COpportunityEditDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void COpportunityEditDlg::OnAddProposal() 
{
	try {
		COpportunityProposalDlg dlg(this);
		long nPatientID = GetSelectedIntValue(m_pClient, 0, -1);
		if(nPatientID <= 0) {
			MessageBox("You must select a client before creating a proposal.");
			return;
		}

		// (j.luckoski 2012-04-10 09:10) - PLID 49492 - Ensure a specialty is selected before allowing you to edit a proposal.
		_RecordsetPtr  prsSpecialty = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT * FROM CustomListDataT WHERE FieldID = 24 AND PersonID = {INT}", nPatientID);

		if(prsSpecialty->eof) {
			MessageBox("You must select a specialty for this client before creating a proposal.");
			return;
		}

		CString strPatientName = GetSelectedStringValue(m_pClient, 1, "");

		//This opportunity must be saved before we can continue.
		if(!ApplyChanges()) {
			//Save failed, we must leave the dialog open
			return;
		}

		//We want to allow the user 2 options -- either start a new proposal, or start a proposal based on the currently active one
		bool bNew = false;

		//If there are no existing proposals, just start a new one
		if(m_pProposalList->GetRowCount() == 0)
			bNew = true;
		else {
			//Popup a menu
			enum
			{
				oNew = 1,
				oExist = 2,
			};

			CMenu mnu;
			mnu.m_hMenu = CreatePopupMenu();
			mnu.InsertMenu(0, MF_BYPOSITION, oNew, "New &Blank Proposal");
			mnu.InsertMenu(1, MF_BYPOSITION, oExist, "New Based on Current &Active");

			long nMenuChoice = 0;

			//Popup the menu on the button
			CRect rc;
			CWnd *pWnd = GetDlgItem(IDC_ADD_PROPOSAL);
			if (pWnd) {
				pWnd->GetWindowRect(&rc);
				nMenuChoice = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, rc.right, rc.top, this, NULL);
				mnu.DestroyMenu();
			} 
			else {
				//Couldn't get the window, just popup wherever the mouse is
				CPoint pt;
				GetCursorPos(&pt);
				nMenuChoice = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
				mnu.DestroyMenu();
			}

			//Nothing selected
			if(nMenuChoice == 0) {
				return;
			}

			switch(nMenuChoice) 
			{
				case oNew:
				{
					bNew = true;
				}
				break;

				case oExist:
				{
					bNew = false;
				}
				break;
			}
		}

		//TODO - We can probably do this more efficiently.  But it's possible that we merged & cancelled the proposal, and
		//	would need to refresh.
		bool bRefresh = true;
		//DRT 3/31/2008 - PLID 29493 - We now must apply the type here.
		long nTypeID = GetSelectedIntValue(m_pType, 0, -1);
		if(nTypeID == -1) {
			AfxMessageBox("You must select a proposal type before creating the proposal itself.");
			return;
		}
		if(bNew) {
			dlg.OpenNewProposal(m_nID, nPatientID, strPatientName, nTypeID, IsFinancing());
		}
		else {
			//Open a new one from an existing one.  Need to get the active ID
			long nExistingID = GetActiveProposalID();
			dlg.OpenFromExisting(m_nID, nPatientID, nExistingID, strPatientName, nTypeID, IsFinancing());
		}

		if(bRefresh) {
			//Refresh our proposal list, something changed
			//TODO:  Do this more efficiently - don't forget the active proposal coloring
			m_pProposalList->Requery();
		}

	} NxCatchAll("Error in OnAddProposal");
}

void COpportunityEditDlg::OnOppEmailSummary() 
{
	try {
		//Get the client
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pClient->CurSel;

		CString strSubject;
		strSubject.Format("Sales Opportunity for '%s'", pRow == NULL ? "<No Client Selected>" : VarString(pRow->GetValue(1)));
		//
		//These are all the data bits we need to put in the email

		CString strType = GetSelectedStringValue(m_pType, 1, "<No Type>");
		CString strName;
		GetDlgItemText(IDC_OPP_NAME, strName);
		COleDateTime dtCloseDate = VarDateTime(m_pickerCloseDate.GetValue());
		//CString strCategory = GetSelectedStringValue(m_pCategory, 1, "<No Category>");	// (d.lange 2010-11-11 10:07) - PLID 41442 - Removed Category dropdown
		CString strStage = GetSelectedStringValue(m_pStage, 1, "<No Stage>");
		CString strNotes;
		GetDlgItemText(IDC_OPP_NOTES, strNotes);
		CString strLostReason;
		GetDlgItemText(IDC_LOST_REASON, strLostReason);
		if(!strLostReason.IsEmpty())
			strLostReason = "Lost Reason: " + strLostReason;
		CString strQBEstimate;
		GetDlgItemText(IDC_QB_ESTIMATE, strQBEstimate);
		CString strPayMethod = GetSelectedStringValue(m_pPayMethod, 1, "<None>");

		CString strCompetition = "<None>", strAlliance = "<None>";
		if(m_aryCurrentCompetition.GetSize() == 1) {
			strCompetition = GetSelectedStringValue(m_pCompetition, 1, "<None>");
		}
		else if(m_aryCurrentCompetition.GetSize() > 1) {
			strCompetition = m_nxlCompetitionLabel.GetText();
		}

		if(m_aryCurrentAlliance.GetSize() == 1) {
			strAlliance = GetSelectedStringValue(m_pAlliance, 1, "<None>");
		}
		else if(m_aryCurrentAlliance.GetSize() > 1) {
			strAlliance = m_nxlAllianceLabel.GetText();
		}

		CString strCurrentPropTotal, strCurrentPropSubtotal;
		CString strCurrentDiscount = FormatCurrencyForInterface(COleCurrency(0, 0));
		double dDiscountPercent = 0.0;
		{	//Get the total & discount amount of the "active" proposal.  This is the one highlighted in green.
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProposalList->FindAbsoluteFirstRow(VARIANT_TRUE);
			NXDATALIST2Lib::IRowSettingsPtr pActiveRow = NULL;
			while(pRow != NULL && pActiveRow == NULL) {
				if(pRow->GetForeColor() == ACTIVE_PROP_HIGHLIGHT_COLOR)
					pActiveRow = pRow;

				pRow = m_pProposalList->FindAbsoluteNextRow(pRow, VARIANT_TRUE);
			}

			if(pActiveRow == NULL) {
				//No active proposals found, use the estimate
				GetDlgItemText(IDC_PRICE_ESTIMATE, strCurrentPropTotal);
			}
			else {
				COleCurrency cySubtotal = VarCurrency(pActiveRow->GetValue(epcSubtotal), COleCurrency(0, 0));
				COleCurrency cyDiscount = VarCurrency(pActiveRow->GetValue(epcDiscount), COleCurrency(0, 0));
				if(cySubtotal != COleCurrency(0,0)) {
					dDiscountPercent = (double)cyDiscount.m_cur.int64 / (double)cySubtotal.m_cur.int64;
				}

				strCurrentPropTotal = FormatCurrencyForInterface( VarCurrency(pActiveRow->GetValue(epcTotal), COleCurrency(0, 0)) );
				strCurrentDiscount = FormatCurrencyForInterface(cyDiscount);
				strCurrentPropSubtotal = FormatCurrencyForInterface(cySubtotal);
			}
		}

		// (z.manning, 11/06/2007) - PLID 27972 - IT/hardware info
		CString strDiscussedHardware = IsDlgButtonChecked(IDC_DISCUSSED_HW) == BST_CHECKED ? "Yes" : "No";
		CString strITPerson, strITNotes;
		if(IsDlgButtonChecked(IDC_IT_PERSON_IDENTIFIED) == BST_CHECKED) {
			GetDlgItemText(IDC_IT_PERSON_NAME, strITPerson);
		}
		else {
			strITPerson = "<Not Known>";
		}
		GetDlgItemText(IDC_IT_NOTES, strITNotes);

		//End data pulling
		//

		CString strBody;
		//%0A is a line break (don't forget to do %% since we're in a format statement)
		// (z.manning, 11/08/2007) - PLID 27972 - We no longer use a ShellExecute with mailto as that
		// does not support attachments, so the body is now just normal text.
		strBody.Format("\r\n\r\n"		//2 empty lines to start so the user can type some text above
			"Current Proposal = %s \r\n"
			"Current Discount = %s (%.1f%%) \r\n"
			"Subtotal = %s \r\n"
			"Type = %s \r\n"
			"Name = %s \r\n"
			"Est Close Date = %s \r\n"
			//"Category = %s \r\n"		// (d.lange 2010-11-11 10:07) - PLID 41442 - Removed Category dropdown
			"Current Stage = %s \r\n"
			"Competition = %s \r\n"
			"Alliance = %s \r\n"
			"QB Estimate = %s \r\n"
			"Pay Method = %s \r\n"
			"Notes = %s \r\n\r\n"
			"Discussed Hardware = %s \r\n"
			"IT Person = %s \r\n"
			"IT Notes = %s \r\n\r\n"
			"%s",
			strCurrentPropTotal, strCurrentDiscount, dDiscountPercent * 100, strCurrentPropSubtotal, strType, strName, FormatDateTimeForInterface(dtCloseDate), //strCategory,
			strStage, strCompetition, strAlliance, strQBEstimate, strPayMethod, strNotes, strDiscussedHardware,
			strITPerson, strITNotes, strLostReason);

		// (z.manning, 11/08/2007) - PLID 27972 - We now attach the merged proposal for the current active proposal if we have one.
		CString strFileToAttach;
		NXDATALIST2Lib::IRowSettingsPtr pActiveProposalRow = m_pProposalList->FindByColumn(epcIsActive, (long)1, NULL, VARIANT_FALSE);
		if(pActiveProposalRow != NULL) {
			if(!VarString(pActiveProposalRow->GetValue(epcMergePath),"").IsEmpty()) {
				strFileToAttach = GetPatientDocumentPath(GetSelectedIntValue(m_pClient, 0, -1)) ^ VarString(pActiveProposalRow->GetValue(epcMergePath));
			}
		}

		//We have no particular recipient in mind
		// (z.manning, 11/05/2007) - PLID 27972 - A called to ShellExecute using "mailto" was replaced with
		// our own SendEmail function as mailto does not support attachments and we now attach the merged
		// proposal file.
		// Note: This failed with a blank recepient, but not when I added the space.
		SendEmail(this, " ", strSubject, strBody, strFileToAttach);

	} NxCatchAll("Error in OnOppEmailSummary");
}

void COpportunityEditDlg::OnOppPrevious() 
{
	// TODO: At one point Todd mentioned having arrow buttons in the opportunity dialog to move between opportunities.  Noone
	//	really has any idea how that would work, so I just hid the buttons.  If we decide to implement, just show & position them.
}

void COpportunityEditDlg::OnOppNext() 
{
	// TODO: At one point Todd mentioned having arrow buttons in the opportunity dialog to move between opportunities.  Noone
	//	really has any idea how that would work, so I just hid the buttons.  If we decide to implement, just show & position them.
}

BEGIN_EVENTSINK_MAP(COpportunityEditDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(COpportunityEditDlg)
	ON_EVENT(COpportunityEditDlg, IDC_OPP_PROPOSAL_LIST, 3 /* DblClickCell */, OnDblClickCellOppProposalList, VTS_DISPATCH VTS_I2)
	ON_EVENT(COpportunityEditDlg, IDC_OPP_PROPOSAL_LIST, 18 /* RequeryFinished */, OnRequeryFinishedOppProposalList, VTS_I2)
	ON_EVENT(COpportunityEditDlg, IDC_OPP_ASSOCIATE, 16 /* SelChosen */, OnSelChosenOppAssociate, VTS_DISPATCH)
	ON_EVENT(COpportunityEditDlg, IDC_OPP_COMPETITION, 16 /* SelChosen */, OnSelChosenOppCompetition, VTS_DISPATCH)
	ON_EVENT(COpportunityEditDlg, IDC_OPP_ALLIANCE, 16 /* SelChosen */, OnSelChosenOppAlliance, VTS_DISPATCH)
	ON_EVENT(COpportunityEditDlg, IDC_OPP_STAGE, 16 /* SelChosen */, OnSelChosenOppStage, VTS_DISPATCH)
	ON_EVENT(COpportunityEditDlg, IDC_OPP_PROPOSAL_LIST, 6 /* RButtonDown */, OnRButtonDownOppProposalList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(COpportunityEditDlg, IDC_OPP_CLIENT, 18 /* RequeryFinished */, OnRequeryFinishedOppClient, VTS_I2)
	ON_EVENT(COpportunityEditDlg, IDC_OPP_CLIENT, 16 /* SelChosen */, OnSelChosenOppClient, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(COpportunityEditDlg, IDC_ITCONTACT_LIST, 18, COpportunityEditDlg::OnRequeryFinishedITContactList, VTS_I2)
END_EVENTSINK_MAP()

void COpportunityEditDlg::OnDblClickCellOppProposalList(LPDISPATCH lpRow, short nColIndex) 
{
	CWaitCursor wc;
	try {
		if(lpRow == NULL)
			return;

		//Edit this proposal
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long nID = VarLong(pRow->GetValue(epcID));

		long nPatientID = GetSelectedIntValue(m_pClient, 0, -1);
		if(nPatientID <= 0) {
			MessageBox("You must select a client before creating a proposal.");
			return;
		}

		CString strPatientName = GetSelectedStringValue(m_pClient, 1, "");

		COpportunityProposalDlg dlg(this);
		//DRT 3/31/2008 - PLID 29493 - We now must apply the type here.
		long nTypeID = GetSelectedIntValue(m_pType, 0, -1);
		if(nTypeID == -1) {
			AfxMessageBox("You must select a proposal type before creating the proposal itself.");
			return;
		}
		dlg.OpenProposal(nID, m_nID, nPatientID, strPatientName, nTypeID, IsFinancing());
		//Refresh our proposal list, something changed
		//TODO:  Do this more efficiently.  They might have hit merge & cancel instead of OK so we must always refresh
		m_pProposalList->Requery();

	} NxCatchAll("Error in OnDblClickCellOppProposalList");
}

void COpportunityEditDlg::OnRequeryFinishedOppProposalList(short nFlags) 
{
	try {
		//Ensure that the active proposal is properly colored, etc
		EnsureActiveProposal();

		//DRT 4/3/2008 - PLID 29493 - Ensure the type cannot change if we have proposals.
		EnsureType();

	} NxCatchAll("Error in OnRequeryFinishedOppProposalList");
}

void COpportunityEditDlg::EnsureActiveProposal()
{
	//We want to do some things with the active proposal, if it exists
	long nActiveID = GetActiveProposalID();
	if(nActiveID != -1) {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProposalList->FindByColumn(epcID, (long)nActiveID, NULL, VARIANT_FALSE);
		if(pRow != NULL) {
			//This is the active proposal
			pRow->PutForeColor(ACTIVE_PROP_HIGHLIGHT_COLOR);

			//Change the "estimated" price to be the proposal price since we have a proposal
			COleCurrency cyTotal = VarCurrency(pRow->GetValue(epcTotal));
			SetDlgItemText(IDC_PRICE_ESTIMATE, FormatCurrencyForInterface(cyTotal));

			//Disable the price estimate so it cannot change
			GetDlgItem(IDC_PRICE_ESTIMATE)->EnableWindow(FALSE);
		}
	}
}

void COpportunityEditDlg::EnsureAssociates()
{
	if(m_aryCurrentAssociates.GetSize() > 1) {
		m_nxlAssociateLabel.ShowWindow(SW_SHOW);
		ShowDlgItem(IDC_OPP_ASSOCIATE, SW_HIDE);
		GetDlgItem(IDC_ASSOCIATE_LABEL)->Invalidate();
	}
	else {
		//1 or less, we use the datalist
		m_nxlAssociateLabel.ShowWindow(SW_HIDE);
		ShowDlgItem(IDC_OPP_ASSOCIATE, SW_SHOW);

		if(m_aryCurrentAssociates.GetSize() == 1) {
			//Single selection
			m_pAssociate->SetSelByColumn(0, (long)m_aryCurrentAssociates.GetAt(0));
		}
		else {
			//No selection
			m_pAssociate->SetSelByColumn(0, (long)-1);
		}
	}
}

void COpportunityEditDlg::EnsurePartners()
{
	if(m_aryCurrentAlliance.GetSize() > 1) {
		ShowDlgItem(IDC_OPP_ALLIANCE, SW_HIDE);
		m_nxlAllianceLabel.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ALLIANCE_LABEL)->Invalidate();
	}
	else {
		//1 or less, we use the datalist
		m_nxlAllianceLabel.ShowWindow(SW_HIDE);
		ShowDlgItem(IDC_OPP_ALLIANCE, SW_SHOW);

		if(m_aryCurrentAlliance.GetSize() == 1) {
			//Single selection
			m_pAlliance->SetSelByColumn(0, (long)m_aryCurrentAlliance.GetAt(0));
		}
		else {
			//No selection
			m_pAlliance->SetSelByColumn(0, (long)-1);
		}
	}
}

void COpportunityEditDlg::EnsureCompetitors()
{
	if(m_aryCurrentCompetition.GetSize() > 1) {
		ShowDlgItem(IDC_OPP_COMPETITION, SW_HIDE);
		m_nxlCompetitionLabel.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_COMPETITION_LABEL)->Invalidate();
	}
	else {
		//1 or less, we use the datalist
		m_nxlCompetitionLabel.ShowWindow(SW_HIDE);
		ShowDlgItem(IDC_OPP_COMPETITION, SW_SHOW);

		//ensure the selection is set
		if(m_aryCurrentCompetition.GetSize() == 1) {
			//Single selection
			m_pCompetition->SetSelByColumn(0, (long)m_aryCurrentCompetition.GetAt(0));
		}
		else {
			//No selection
			m_pCompetition->SetSelByColumn(0, (long)-1);
		}
	}
}

void COpportunityEditDlg::OnSelChosenOppAssociate(LPDISPATCH lpRow) 
{
	try {
		if(lpRow == NULL)
			return;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long nID = VarLong(pRow->GetValue(0));

		if(nID == -2) {
			//Multiple -- this is the same behavior as if they'd clicked the hyperlink, so 
			//	we'll put all the code there.
			OnAssociateLabelClick();
		}
		else {
			//1 selection
			m_aryCurrentAssociates.RemoveAll();
			if(nID != -1)
				m_aryCurrentAssociates.Add(nID);

			//Make sure we're drawing the right stuff
			EnsureAssociates();
		}

	} NxCatchAll("Error in OnSelChosenOppAssociate");
}

void COpportunityEditDlg::OnSelChosenOppCompetition(LPDISPATCH lpRow) 
{
	try {
		if(lpRow == NULL)
			return;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long nID = VarLong(pRow->GetValue(0));

		if(nID == -2) {
			//Multiple -- this is the same behavior as if they'd clicked the hyperlink, so 
			//	we'll put all the code there.
			OnCompetitionLabelClick();
		}
		else {
			//1 selection
			m_aryCurrentCompetition.RemoveAll();
			if(nID != -1)
				m_aryCurrentCompetition.Add(nID);

			//Make sure we're drawing the right stuff
			EnsureCompetitors();
		}
	} NxCatchAll("Error in OnSelChosenOppCompetition");
}

void COpportunityEditDlg::OnSelChosenOppAlliance(LPDISPATCH lpRow) 
{
	try {
		if(lpRow == NULL)
			return;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long nID = VarLong(pRow->GetValue(0));

		if(nID == -2) {
			//Multiple -- this is the same behavior as if they'd clicked the hyperlink, so 
			//	we'll put all the code there.
			OnAllianceLabelClick();
		}
		else {
			//1 selection
			m_aryCurrentAlliance.RemoveAll();
			if(nID != -1)
				m_aryCurrentAlliance.Add(nID);

			//Make sure we're drawing the right stuff
			EnsurePartners();
		}
	} NxCatchAll("Error in OnSelChosenOppAlliance");
}

LRESULT COpportunityEditDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_ASSOCIATE_LABEL:
			OnAssociateLabelClick();
			break;
		case IDC_ALLIANCE_LABEL:
			OnAllianceLabelClick();
			break;
		case IDC_COMPETITION_LABEL:
			OnCompetitionLabelClick();
			break;
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	} NxCatchAll("Error in OnLabelClick");
	return 0;
}

void COpportunityEditDlg::OnAssociateLabelClick()
{
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "UsersT");
	dlg.PreSelect(m_aryCurrentAssociates);
	if(dlg.Open("UsersT", "PatientCoordinator = 1", "PersonID", "Username", "Please choose sales associates:") == IDOK) {
		m_aryCurrentAssociates.RemoveAll();
		dlg.FillArrayWithIDs(m_aryCurrentAssociates);

		if(m_aryCurrentAssociates.GetSize() > 1) {
			//Many selections
			m_nxlAssociateLabel.SetText(dlg.GetMultiSelectString());
		}
	}

	EnsureAssociates();
}

void COpportunityEditDlg::OnAllianceLabelClick()
{
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "AlliancePartnersT");
	dlg.PreSelect(m_aryCurrentAlliance);
	if(dlg.Open("AlliancePartnersT", "", "ID", "Name", "Please choose partners:") == IDOK) {
		m_aryCurrentAlliance.RemoveAll();
		dlg.FillArrayWithIDs(m_aryCurrentAlliance);

		if(m_aryCurrentAlliance.GetSize() > 1) {
			//Many selections
			m_nxlAllianceLabel.SetText(dlg.GetMultiSelectString());
		}
	}

	EnsurePartners();
}

void COpportunityEditDlg::OnCompetitionLabelClick()
{
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "CompetitorsT");
	dlg.PreSelect(m_aryCurrentCompetition);
	if(dlg.Open("CompetitorsT", "", "ID", "Name", "Please choose competitors:") == IDOK) {
		m_aryCurrentCompetition.RemoveAll();
		dlg.FillArrayWithIDs(m_aryCurrentCompetition);

		if(m_aryCurrentCompetition.GetSize() > 1) {
			//Many selections
			m_nxlCompetitionLabel.SetText(dlg.GetMultiSelectString());
		}
	}

	EnsureCompetitors();
}

BOOL COpportunityEditDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);


		//Associates label
		if(m_aryCurrentAssociates.GetSize() > 1) {
			CRect rc;
			GetDlgItem(IDC_ASSOCIATE_LABEL)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

		//Alliance label
		if(m_aryCurrentAlliance.GetSize() > 1) {
			CRect rc;
			GetDlgItem(IDC_ALLIANCE_LABEL)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

		//Competition label
		if(m_aryCurrentCompetition.GetSize() > 1) {
			CRect rc;
			GetDlgItem(IDC_COMPETITION_LABEL)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	} NxCatchAll("Error in OnSetCursor");

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void COpportunityEditDlg::EnsureLostReason()
{
	UINT nShow = SW_HIDE;

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pStage->CurSel;
	if(pRow != NULL) {
		// (d.lange 2010-12-20 09:35) - PLID 41355 - Stage Number changed to sequential letters
		// Instead of getting the StageNumber, get the StageID
		long nStageID = VarLong(pRow->GetValue(0));
		//TODO:  This is currently hardcoded, find a better way to handle "lost"
		// 7 - Lost Opportunity, this is now 'F' - StageID = 11
		if(nStageID = 11/*nStageID == 7*/) {
			nShow = SW_SHOW;
		}
	}

	//Do the display or hide
	ShowDlgItem(IDC_LOST_REASON, nShow);
	ShowDlgItem(IDC_LOST_REASON_LABEL, nShow);

	//Now we need to appropriately size the "notes" box
	CRect rcReasonLabel, rcReasonBox, rcNotes;
	GetDlgItem(IDC_OPP_NOTES)->GetWindowRect(rcNotes);
	ScreenToClient(rcNotes);
	GetDlgItem(IDC_LOST_REASON_LABEL)->GetWindowRect(rcReasonLabel);
	ScreenToClient(rcReasonLabel);
	GetDlgItem(IDC_LOST_REASON)->GetWindowRect(rcReasonBox);
	ScreenToClient(rcReasonBox);

	//If we are showing the lost reason, then we need to shrink the notes box
	if(nShow == SW_SHOW) {
		//Set the bottom of the notes box to be the top of the label, with
		//	a slight buffer
		rcNotes.bottom = rcReasonLabel.top - 10;
	}
	else {
		//Otherwise, we want the notes expanded
		rcNotes.bottom = rcReasonBox.bottom;
	}

	//And finally, move the notes box to its new position
	GetDlgItem(IDC_OPP_NOTES)->MoveWindow(rcNotes);
}

void COpportunityEditDlg::OnSelChosenOppStage(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		// (d.lange 2010-11-11 10:07) - PLID 41442 - Removed Category dropdown
		/*if(pRow != NULL)
		{
			// (z.manning, 11/06/2007) - PLID 27972 - Make sure the category dropdown matches the stage dropdown.
			long nStageCategoryID = VarLong(pRow->GetValue(5));
			if(GetSelectedIntValue(m_pCategory, 0, -1) != nStageCategoryID) {
				m_pCategory->SetSelByColumn(0, nStageCategoryID);
			}
		}*/

		//Ensure that the lost reason fields are hidden/shown appropriately
		EnsureLostReason();

	} NxCatchAll("Error in OnSelChosenOppStage");
}

void COpportunityEditDlg::OnKillfocusPriceEstimate() 
{
	try {
		//Ensure that the price is formatted correctly as a currency.
		CString strEstPrice;
		GetDlgItemText(IDC_PRICE_ESTIMATE, strEstPrice);

		COleCurrency cy;
		cy.ParseCurrency(strEstPrice);

		//If it's not a valid currency, just reset to $0.00
		if(cy.GetStatus() != COleCurrency::valid)
			cy = COleCurrency(0, 0);

		SetDlgItemText(IDC_PRICE_ESTIMATE, FormatCurrencyForInterface(cy));
	} NxCatchAll("Error in OnKillfocusPriceEstimate");
}

//Returns -1 if no proposals
long COpportunityEditDlg::GetActiveProposalID()
{
	//Search for it
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProposalList->FindByColumn(epcIsActive, (long)1, NULL, VARIANT_FALSE);
	if(pRow != NULL) {
		//Found our active row
		long nCurrentID = VarLong(pRow->GetValue(epcID));
		return nCurrentID;
	}

	//Not found
	return -1;
}

void COpportunityEditDlg::OnRButtonDownOppProposalList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		if(lpRow == NULL)
			return;

		//If the opportunity isn't yet saved, then we should be unable to have any proposals.  But just in case...
		if(m_nID == -1)
			return;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		//Set the cursel to this row
		m_pProposalList->CurSel = pRow;

		enum MenuOptions
		{
			moMarkActive = 1,
			moOpenMergedDoc,
		};

		CString strMergeFile = VarString(pRow->GetValue(epcMergePath), "");
		UINT nMergeMenuOptionFlags;
		if(strMergeFile.IsEmpty()) {
			nMergeMenuOptionFlags = MF_GRAYED;
		}
		else {
			nMergeMenuOptionFlags = MF_ENABLED;
		}

		//Create a 1 option menu
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		mnu.InsertMenu(0, MF_BYPOSITION, moMarkActive, "Mark &Active");
		// (z.manning, 11/05/2007) - PLID 27972 - Add an option to open the merged proposal.
		mnu.InsertMenu(1, MF_BYPOSITION|nMergeMenuOptionFlags, moOpenMergedDoc, "View &Merged Proposal");

		//Popup the menu at the x/y
		CPoint pt;
		GetCursorPos(&pt);
		switch(mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL))
		{
			case moMarkActive:
			{
				//They selected our mark active option

				//Get the previous active, if there was one
				NXDATALIST2Lib::IRowSettingsPtr pRowPrev = m_pProposalList->FindByColumn(epcIsActive, (long)1, NULL, VARIANT_FALSE);
				if(pRowPrev != NULL) {
					//This row was previously active, get rid of it
					pRowPrev->PutValue(epcIsActive, (long)0);
					pRowPrev->PutForeColor(RGB(0, 0, 0));
				}

				//Update the database
				//	Keep the EstPrice field in sync as well
				long nID = VarLong(pRow->GetValue(epcID));
				COleCurrency cyAmount = VarCurrency(pRow->GetValue(epcTotal));
				ExecuteSql("UPDATE OpportunitiesT SET ActiveProposalID = %li, EstPrice = '%s' WHERE ID = %li", nID, _Q(FormatCurrencyForInterface(cyAmount)), m_nID);

				//Now that we've saved the estimated price, update our saved variables
				m_cySavedEstPrice = cyAmount;

				//Set the new row to active
				pRow->PutValue(epcIsActive, (long)1);
				EnsureActiveProposal();
			}
			break;

			case moOpenMergedDoc:
			{
				// (z.manning, 11/05/2007) - PLID 27972 - Ok, they want to open the previously merged proposal.
				// We have the file name in the datalist, so go ahead an try to open it.
				CWaitCursor wc;
				if(!OpenDocument(strMergeFile, GetSelectedIntValue(m_pClient, 0, -1))) {
					CString strError;
					FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
					strError.ReleaseBuffer();
					MessageBox(FormatString("Failed to open file '%s' due to the following error:\r\n\r\n%s", strMergeFile, strError));
				}
			}
			break;
		}

		//Cleanup
		mnu.DestroyMenu();

	} NxCatchAll("Error in OnRButtonDownOppProposalList");
}

void COpportunityEditDlg::OnRequeryFinishedOppClient(short nFlags)
{
	try
	{
		// (z.manning, 11/06/2007) - PLID 27972 - Default the name to the client's name on new opportunities.
		if(m_nID == -1) {
			ASSERT(m_strSavedName.IsEmpty());
			SetDlgItemText(IDC_OPP_NAME, GetSelectedStringValue(m_pClient, 1, ""));
			// (z.manning, 11/06/2007) - Since it's us that changed the opportunity name make sure we reset
			// the fact that the user has not yet modified it.
			m_bUserModifiedOppName = FALSE;
		}

	}NxCatchAll("COpportunityEditDlg::OnRequeryFinishedOppClientList");
}

void COpportunityEditDlg::OnChangeOppName()
{
	try
	{
		// (z.manning, 11/06/2007) - PLID 27972 - Remember that the user has changed the opportunity name.
		if(!m_bUserModifiedOppName) {
			m_bUserModifiedOppName = TRUE;
		}

	}NxCatchAll("COpportunityEditDlg::OnChangeOppName");
}

void COpportunityEditDlg::OnSelChosenOppClient(LPDISPATCH lpRow)
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		// (z.manning, 11/06/2007) - PLID 27972 - If this is a new opportunity and the user has not yet
		// modified its name, then we should change the name to the new client since it was defaulted
		// to whatever client was selected when creating the opportunity.
		if(pRow != NULL && m_nID == -1 && !m_bUserModifiedOppName) {
			CString strPreviousName;
			GetDlgItemText(IDC_OPP_NAME, strPreviousName);
			ASSERT(m_pClient->FindByColumn(1, _bstr_t(strPreviousName), NULL, VARIANT_FALSE) != NULL);
			SetDlgItemText(IDC_OPP_NAME, VarString(pRow->GetValue(1)));
			m_bUserModifiedOppName = FALSE;
		}

		if(pRow) {
			long nClientID = VarLong(pRow->GetValue(0), -1);
			// (d.lange 2010-11-09 12:13) - PLID 41335 - Grab the Contact's PersonID and try to set selection
			ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT IntParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = 31", nClientID);
			if(!rs->eof) {
				m_pITContact->SetSelByColumn((long)0, AdoFldLong(rs, "IntParam", -1));
			}else {
				m_pITContact->SetSelByColumn((long)0, -1);
			}
			rs->Close();
		}

	}NxCatchAll("COpportunityEditDlg::OnSelChosenOppClient");
}
// (d.lange 2010-11-09 12:25) - PLID 41335 - Removed this control
/*void COpportunityEditDlg::OnITPersonKnown()
{
	try
	{
		// (z.manning, 11/08/2007) - PLID 27972 - Enable/disable the IT person name field depending on the current
		// status of the IT person known checkbox.
		if(IsDlgButtonChecked(IDC_IT_PERSON_IDENTIFIED) == BST_CHECKED) {
			GetDlgItem(IDC_IT_PERSON_NAME)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_IT_PERSON_NAME)->EnableWindow(FALSE);
		}
		
	}NxCatchAll("COpportunityEditDlg::OnITPersonKnown");
}*/

void COpportunityEditDlg::EnsureType()
{
	//DRT 4/3/2008 - PLID 29493 - If there are any proposals in this opportunity, the type may not change.
	if(m_pProposalList->GetRowCount() > 0) {
		GetDlgItem(IDC_OPP_TYPE)->EnableWindow(FALSE);
	}
}

// (d.thompson 2009-08-27) - PLID 35365 - For right now, the payment method contains 5 kinds of financing.  These
//	aren't really defined, but we may rename them.  We may decide to come back later and do this function a little
//	bit differently... maybe adding a flag or something to them.  For now sales has requested we go this route.
bool COpportunityEditDlg::IsFinancing()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPayMethod->CurSel;
	if(pRow) {
		long nMethodID = VarLong(pRow->GetValue(0));

		//The following are the data IDs we are looking for, again hardcoded.
		if(nMethodID >= 2 && nMethodID <= 6) {
			return true;
		}
	}

	return false;
}

// (d.lange 2010-11-09 11:22) - PLID 41335 - Insert a generic row
void COpportunityEditDlg::OnRequeryFinishedITContactList(short nFlags)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pITContact->GetNewRow();
	pRow->PutValue(0, (long)0);
	pRow->PutValue(1, "<No Contact Selected>");
	pRow->PutValue(2, "");
	pRow->PutValue(3, "");
	pRow->PutValue(4, "");
	pRow->PutValue(5, "");
	m_pITContact->AddRowBefore(pRow, m_pITContact->GetFirstRow());
}
