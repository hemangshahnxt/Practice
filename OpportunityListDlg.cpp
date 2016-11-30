// OpportunityListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "OpportunityListDlg.h"
#include "OpportunityEditDlg.h"
#include "GlobalDrawingUtils.h"
#include "OpportunityProposalDlg.h"
#include "InternationalUtils.h"
#include "TaskEditDlg.h"			// (d.lange 2010-11-30 11:38) - PLID 41336 - Added Next Follow Up column

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (z.manning, 11/05/2007) - PLID 27971 - Use this color for inactive opportunity rows.
#define INACTIVE_ROW_COLOR RGB(128,128,128)

// (z.manning, 11/08/2007) - PLID 27971 - Added columns for city, state, type ID, and inactive.
enum eListColumns {
	elcID = 0,
	elcAccountManager,	// (d.lange 2010-12-22 17:35) - PLID 41356 - Added Account Manager
	elcClientName,
	elcSpeciality,		// (d.lange 2010-11-08 16:53) - PLID 41357 - Added speciality
	elcCity,
	elcState,
	elcOppName,
	elcAlliance,		// (d.lange 2010-11-09 16:43) - PLID 41359 - Added Alliance
	elcCompetition,		// (d.lange 2010-12-01 09:58) - PLID 41358 - Added Competition column
	elcTypeID,
	elcType,
	elcStage,
	elcClientID,
	elcNextFollowUpID,
	elcNextFollowUp,	// (d.lange 2010-11-30 11:39) - PLID 41336 - Added Next Follow Up date
	elcEstCloseDate,
	elcCloseDateCount,	// (d.lange 2010-11-10 17:02) - PLID 41437 - Added Close Date Count
	elcAmount,
	elcActiveProposalID,
	elcSecondaryCoord,
	elcInactive,
	elcPayMethod,			// (d.thompson 2009-08-27) - PLID 35365
};


/*	TODO:  Do something with this, these are the queries to create the current structure.  


  --This was removed
  --CREATE TABLE OpportunityProposalDiscountsT (ID int NOT NULL PRIMARY KEY identity, ProposalID int NOT NULL CONSTRAINT FK_OppDiscount_ProposalID FOREIGN KEY REFERENCES OpportunityProposalsT(ID),
  --  UserID int NOT NULL CONSTRAINT FK_OppDiscount_UserID FOREIGN KEY REFERENCES UsersT(PersonID), Date datetime NOT NULL, DiscountPercent int NOT NULL)

drop table OpportunityProposalsT
drop table OpportunityPriceStructureT
drop table OpportunityStageHistoryT
drop table OpportunityAllianceT
drop table AlliancePartnersT
drop table OpportunityCompetitionT
drop table CompetitorsT
drop table OpportunitiesT
drop table OpportunityStagesT
drop table OpportunityAssociatesT
drop table OpportunityCategoriesT
drop table OpportunityTypesT
drop table OpportunityPayMethodsT


CREATE TABLE OpportunityTypesT (ID int NOT NULL PRIMARY KEY, Name nvarchar(255) NOT NULL DEFAULT(''))
CREATE TABLE OpportunityCategoriesT (ID int NOT NULL PRIMARY KEY, Name nvarchar(255) NOT NULL DEFAULT(''))
CREATE TABLE OpportunityStagesT (ID int NOT NULL PRIMARY KEY, Name nvarchar(255) NOT NULL DEFAULT(''), StageNumber int NOT NULL, Description nvarchar(500) NOT NULL DEFAULT(''), 
  DefaultPercent int NOT NULL DEFAULT(0))

-- (z.manning, 11/05/2007) - Added a table for opportunity payment methods.
CREATE TABLE OpportunityPayMethodsT
(
	ID int NOT NULL CONSTRAINT PK_OpportunityPayMethodsT PRIMARY KEY,
	Name nvarchar(255) NOT NULL DEFAULT('')
)

CREATE TABLE OpportunitiesT (ID int NOT NULL PRIMARY KEY, PersonID int NOT NULL CONSTRAINT FK_Opportunity_PersonID FOREIGN KEY REFERENCES PersonT(ID),
  TypeID int NOT NULL CONSTRAINT FK_Opp_OppType FOREIGN KEY REFERENCES OpportunityTypesT(ID), 
  Name nvarchar(255) NOT NULL DEFAULT(''), PrimaryCoordID int NOT NULL CONSTRAINT FK_Opp_PtCoordPri FOREIGN KEY REFERENCES UsersT(PersonID), 
  SecondaryCoordID int NULL CONSTRAINT FK_Opp_PtCoordSec FOREIGN KEY REFERENCES UsersT(PersonID), 
  CategoryID int NOT NULL CONSTRAINT FK_Opp_OppCategory FOREIGN KEY REFERENCES OpportunityCategoriesT(ID), LostReason ntext NOT NULL DEFAULT(''),
  CurrentStageID int NOT NULL CONSTRAINT FK_Opp_OppStage FOREIGN KEY REFERENCES OpportunityStagesT(ID), Notes ntext NOT NULL DEFAULT(''),
  EstCloseDate datetime NOT NULL, EstPrice money NOT NULL DEFAULT(0), QBEstimate nvarchar(255) NOT NULL DEFAULT(''))


CREATE TABLE OpportunityAssociatesT (ID int NOT NULL PRIMARY KEY, OpportunityID int NOT NULL CONSTRAINT FK_OppAssoc_OppID FOREIGN KEY REFERENCES OpportunitiesT(ID), 
  AssocID int NOT NULL CONSTRAINT FK_OppAssoc_AssocID FOREIGN KEY REFERENCES UsersT(PersonID))
CREATE TABLE CompetitorsT (ID int NOT NULL PRIMARY KEY, Name nvarchar(255) NOT NULL DEFAULT(''))
CREATE TABLE OpportunityCompetitionT (ID int NOT NULL PRIMARY KEY, OpportunityID int NOT NULL CONSTRAINT FK_OppCompetition_ID_OppID FOREIGN KEY REFERENCES OpportunitiesT(ID),
  CompetitorID int NOT NULL CONSTRAINT FK_OppCompetition_CompID_Competitors FOREIGN KEY REFERENCES CompetitorsT(ID))
CREATE TABLE AlliancePartnersT (ID int NOT NULL PRIMARY KEY, Name nvarchar(255) NOT NULL DEFAULT(''))
CREATE TABLE OpportunityAllianceT (ID int NOT NULL PRIMARY KEY, OpportunityID int NOT NULL CONSTRAINT FK_OppAlliance_ID_OppID FOREIGN KEY REFERENCES OpportunitiesT(ID),
  PartnerID int NOT NULL CONSTRAINT FK_OppAlliance_PartnerID FOREIGN KEY REFERENCES AlliancePartnersT(ID))
CREATE TABLE OpportunityStageHistoryT (ID int NOT NULL PRIMARY KEY, OpportunityID int NOT NULL CONSTRAINT FK_OppStageHistory_OppID FOREIGN KEY REFERENCES OpportunitiesT(ID), 
  StageID int NOT NULL CONSTRAINT FK_OppStageHistory_StageID FOREIGN KEY REFERENCES OpportunityStagesT(ID), DateActive datetime NOT NULL DEFAULT(getdate()), 
  ChangedByUserID int NOT NULL CONSTRAINT FK_OppStageHistory_UserID FOREIGN KEY REFERENCES UsersT(PersonID))

CREATE TABLE OpportunityPriceStructureT (ID int NOT NULL PRIMARY KEY identity, Scheduler money NULL, LicenseSched money NULL, Billing money NULL, 
  HCFA money NULL, EBilling money NULL, Letters money NULL, Quotes money NULL, Tracking money NULL, NexForms money NULL, Inventory money NULL, 
  NexSpa money NULL, NexASC money NULL, Mirror money NULL, United money NULL, Workstations money NULL, Doctors money NULL, PDA money NULL, 
  EMRFirst money NULL, EMRAddtl money NULL, Training money NULL, Conversion money NULL, Travel money NULL, PackageScheduler money NULL,
  PackageFinancial money NULL, PackageCosmetic money NULL, Active bit NOT NULL DEFAULT(0))

CREATE TABLE OpportunityProposalsT (ID int NOT NULL PRIMARY KEY identity, OpportunityID int NOT NULL CONSTRAINT FK_OppProposal_OppID FOREIGN KEY REFERENCES OpportunitiesT(ID), 
  MailSentID int NULL CONSTRAINT FK_OppProposal_MailSent FOREIGN KEY REFERENCES MailSent(MailID), ProposalDate datetime NOT NULL, ExpiresOn datetime NOT NULL, QuoteNum int NOT NULL CONSTRAINT UQ_OppProposal_QuoteNum UNIQUE,
  CreatedByUserID int NOT NULL CONSTRAINT FK_OppProposal_CreatedUser FOREIGN KEY REFERENCES UsersT(PersonID), CreatedDate datetime NOT NULL DEFAULT(getdate()), 
  Scheduler bit NOT NULL DEFAULT(0), LicenseSched bit NOT NULL DEFAULT(0), Billing bit NOT NULL DEFAULT(0), HCFA bit NOT NULL DEFAULT(0), EBilling bit NOT NULL DEFAULT(0), 
  Letters bit NOT NULL DEFAULT(0), Quotes bit NOT NULL DEFAULT(0), Tracking bit NOT NULL DEFAULT(0), NexForms bit NOT NULL DEFAULT(0), Inventory bit NOT NULL DEFAULT(0), 
  NexSpa bit NOT NULL DEFAULT(0), NexASC bit NOT NULL DEFAULT(0), Mirror bit NOT NULL DEFAULT(0), United bit NOT NULL DEFAULT(0), 
  Workstations int NOT NULL DEFAULT(0), Doctors int NOT NULL DEFAULT(0), PDA int NOT NULL DEFAULT(0), EMR int NOT NULL DEFAULT(0), Training int NOT NULL DEFAULT(0), 
  Support int NOT NULL DEFAULT(0), Conversion int NOT NULL DEFAULT(0), Travel int NOT NULL DEFAULT(0), 
  PriceStructureID int NOT NULL CONSTRAINT FK_OppProposal_StructureID FOREIGN KEY REFERENCES OpportunityPriceStructureT(ID), SavedTotal money NOT NULL DEFAULT(0), 
  DiscountAmount money NOT NULL DEFAULT(0), DiscountedBy int NULL CONSTRAINT FK_OppProposal_DiscountUserID FOREIGN KEY REFERENCES UsersT(PersonID))

CREATE TABLE OpportunityMaxDiscountsT (ID int NOT NULL PRIMARY KEY identity, UserID int NOT NULL CONSTRAINT FK_OppMaxDiscount_UserID FOREIGN KEY REFERENCES UsersT(PersonID),
  MaximumPercentage float NOT NULL DEFAULT(0.0))

ALTER TABLE OpportunitiesT ADD ActiveProposalID int NULL CONSTRAINT FK_Opp_ActiveProposal FOREIGN KEY REFERENCES OpportunityProposalsT(ID)

CREATE TABLE UserQuotasT (ID int NOT NULL PRIMARY KEY identity, UserID int NOT NULL CONSTRAINT FK_Quota_UserID FOREIGN KEY REFERENCES UsersT(PersonID), Quota money NOT NULL DEFAULT(0), 
  BeginRange datetime NOT NULL, EndRange datetime NOT NULL)

INSERT INTO UserQuotasT (UserID, Quota, BeginRange, EndRange) (select PersonID, convert(money, 360000), '4/1/2007', '7/1/2007' from userst where patientcoordinator = 1)

ALTER TABLE OpportunityProposalsT ADD HL7 bit NOT NULL DEFAULT(0), Inform bit NOT NULL DEFAULT(0), Quickbooks bit NOT NULL DEFAULT(0)
ALTER TABLE OpportunityPriceStructureT ADD HL7 money NULL, Inform money NULL, Quickbooks money NULL

-- (z.manning, 11/05/2007) - Added inactive column.
ALTER TABLE OpportunitiesT  ADD Inactive bit NOT NULL DEFAULT(0)

-- (z.manning, 11/05/2007) - Added pay method column to opportunities
ALTER TABLE OpportunitiesT ADD PayMethod int NULL CONSTRAINT FK_OpportunitiesT_OpportunityPayMethodsT REFERENCES OpportunityPayMethodsT(ID)

-- (z.manning, 11/06/2007) - Added a way to link categories and stages.
ALTER TABLE OpportunityStagesT ADD OpportunityCategoryID INT NULL CONSTRAINT FK_OpportunityStagesT_OpportunityCategoriesT REFERENCES OpportunityCategoriesT(ID)
UPDATE OpportunityStagesT SET OpportunityCategoryID = 1 WHERE ID = 1
UPDATE OpportunityStagesT SET OpportunityCategoryID = 2 WHERE ID = 2
UPDATE OpportunityStagesT SET OpportunityCategoryID = 3 WHERE ID IN (3,4,5,6)
UPDATE OpportunityStagesT SET OpportunityCategoryID = 4 WHERE ID IN (8,9)
UPDATE OpportunityStagesT SET OpportunityCategoryID = 5 WHERE ID IN (10)
UPDATE OpportunityStagesT SET OpportunityCategoryID = 6 WHERE ID = 11
ALTER TABLE OpportunityStagesT ALTER COLUMN OpportunityCategoryID INT NOT NULL

-- (z.manning, 11/06/2007) - Added columns for HW/IT details
ALTER TABLE OpportunitiesT ADD DiscussedHardware bit NOT NULL DEFAULT(0)
ALTER TABLE OpportunitiesT ADD ITPersonKnown bit NOT NULL DEFAULT(0)
ALTER TABLE OpportunitiesT ADD ITPersonName nvarchar(255) NOT NULL DEFAULT('')
ALTER TABLE OpportunitiesT ADD ITNotes ntext NOT NULL DEFAULT('')

-- (z.manning, 11/26/2007) - Track EMR training days separately
ALTER TABLE OpportunityProposalsT ADD EmrTraining int NOT NULL DEFAULT(0)

--(z.manning, 12/11/2007) - PriCoordID is no longer requied on add-on sales
ALTER TABLE OpportunitiesT ALTER COLUMN PrimaryCoordID INT NULL

*/

/*
--Functions
CREATE FUNCTION "GetOpportunityAssociatesString" ( @OpportunityID int ) RETURNS nvarchar(1000) AS  
BEGIN  
	declare @total nvarchar(1000);
	SELECT @total = COALESCE(@total + ', ', '') + UsersT.Username FROM UsersT WHERE PersonID IN (SELECT AssocID FROM OpportunityAssociatesT WHERE OpportunityID = @OpportunityID) 
	ORDER BY Username;
	return @total  
END

CREATE FUNCTION "GetOpportunityAllianceString" ( @OpportunityID int ) RETURNS nvarchar(1000) AS  
BEGIN  
	declare @total nvarchar(1000);
	SELECT @total = COALESCE(@total + ', ', '') + AlliancePartnersT.Name FROM AlliancePartnersT WHERE ID IN (SELECT PartnerID FROM OpportunityAllianceT WHERE OpportunityID = @OpportunityID) 
	ORDER BY Name;
	return @total  
END

CREATE FUNCTION "GetOpportunityCompetitionString" ( @OpportunityID int ) RETURNS nvarchar(1000) AS  
BEGIN  
	declare @total nvarchar(1000);
	SELECT @total = COALESCE(@total + ', ', '') + CompetitorsT.Name FROM CompetitorsT WHERE ID IN (SELECT CompetitorID FROM OpportunityCompetitionT WHERE OpportunityID = @OpportunityID) 
	ORDER BY Name;
	return @total  
END

-- (z.manning, 11/06/2007) - Created a function to calculate the subtotal of a proposal.
ALTER FUNCTION GetOppProposalSubtotal(@nProposalID int)
RETURNS money
BEGIN
return (
SELECT
 -- Handle schedule package costs
 (CASE WHEN OpportunityProposalsT.Scheduler = 1 AND OpportunityProposalsT.LicenseSched = 1 THEN PackageScheduler ELSE (OpportunityProposalsT.Scheduler * OpportunityPriceStructureT.Scheduler) + (OpportunityProposalsT.LicenseSched * OpportunityPriceStructureT.LicenseSched) END) +
 -- Handle billing package
 (CASE WHEN OpportunityProposalsT.Billing = 1 AND OpportunityProposalsT.HCFA = 1 AND OpportunityProposalsT.EBilling = 1 THEN PackageFinancial ELSE (OpportunityProposalsT.Billing * OpportunityPriceStructureT.Billing) + (OpportunityProposalsT.HCFA * OpportunityPriceStructureT.HCFA) + (OpportunityProposalsT.EBilling * OpportunityPriceStructureT.EBilling) END) +
 -- Handle cosmetic package
 (CASE WHEN OpportunityProposalsT.Letters = 1 AND OpportunityProposalsT.Quotes = 1 AND OpportunityProposalsT.Tracking = 1 AND OpportunityProposalsT.NexForms = 1 THEN PackageCosmetic ELSE (OpportunityProposalsT.Letters * OpportunityPriceStructureT.Letters) + (OpportunityProposalsT.Quotes * OpportunityPriceStructureT.Quotes) +  (OpportunityProposalsT.Tracking * OpportunityPriceStructureT.Tracking) + (OpportunityProposalsT.NexForms * OpportunityPriceStructureT.NexForms) END) +
 (OpportunityProposalsT.Inventory * OpportunityPriceStructureT.Inventory) +
 (OpportunityProposalsT.NexSpa * OpportunityPriceStructureT.NexSpa) + 
 (OpportunityProposalsT.NexASC * OpportunityPriceStructureT.NexASC) +
 (OpportunityProposalsT.Mirror * OpportunityPriceStructureT.Mirror) +
 (OpportunityProposalsT.United * OpportunityPriceStructureT.United) +
 (OpportunityProposalsT.Workstations * OpportunityPriceStructureT.Workstations) + 
 (OpportunityProposalsT.Doctors * OpportunityPriceStructureT.Doctors) +
 (OpportunityProposalsT.PDA * OpportunityPriceStructureT.PDA) +
 (CASE WHEN OpportunityProposalsT.EMR = 0 THEN convert(money, 0) ELSE OpportunityPriceStructureT.EMRFirst + ((OpportunityProposalsT.EMR - 1) * OpportunityPriceStructureT.EMRAddtl) END) +
 (OpportunityProposalsT.HL7 * OpportunityPriceStructureT.HL7) +
 (OpportunityProposalsT.Inform * OpportunityPriceStructureT.Inform) + 
 (OpportunityProposalsT.Quickbooks * OpportunityPriceStructureT.Quickbooks)
FROM OpportunityProposalsT
LEFT JOIN OpportunityPriceStructureT ON OpportunityProposalsT.PriceStructureID = OpportunityPriceStructureT.ID
WHERE OpportunityProposalsT.ID = @nProposalID
)
END

*/

/*
--Default Data
INSERT INTO OpportunityTypesT values (1, 'New Sale')
INSERT INTO OpportunityTypesT values (2, 'AddOn Sale')
INSERT INTO OpportunityTypesT values (3, 'Renew Support')
INSERT INTO OpportunityTypesT values (4, '3 Month AddOn') --ZM: Added 11/05/2007

INSERT INTO OpportunityCategoriesT values (1, 'Suspect')
INSERT INTO OpportunityCategoriesT values (2, 'Lead')
INSERT INTO OpportunityCategoriesT values (3, 'Prospect')
INSERT INTO OpportunityCategoriesT values (4, 'Client')
INSERT INTO OpportunityCategoriesT values (5, 'Live Client')
INSERT INTO OpportunityCategoriesT values (6, 'Lost Opportunity')

INSERT INTO OpportunityStagesT values (1, 'Suspect', 6, 'An identified practice in our markets.',  0)
INSERT INTO OpportunityStagesT values (2, 'Lead', 5, 'Has expressed interest in NexTech.',  5)
INSERT INTO OpportunityStagesT values (3, 'Qualification', 4, 'Completed qualification call.',  10)
INSERT INTO OpportunityStagesT values (4, 'Needs Analysis', 3, 'Analyzed practice needs.',  25)
INSERT INTO OpportunityStagesT values (5, 'Selling', 2, 'Demo completed.',  50)
INSERT INTO OpportunityStagesT values (6, 'Proposal & ROI', 1, 'Proposal given.',  90)
-- (z.manning, 11/06/2007) - Per Sales, the 'Signed Proposal' step was pointless.
--INSERT INTO OpportunityStagesT values (7, 'Sold', 0, 'Signed proposal.',  100)
INSERT INTO OpportunityStagesT values (8, 'Order Processing', 0, 'Signed proposal submitted to invoices + down payment.',  100)
INSERT INTO OpportunityStagesT values (9, 'Collected', 0, 'Paid in full.',  100)
INSERT INTO OpportunityStagesT values (10, 'Implemented', 0, 'Implemented & using.',  100)
INSERT INTO OpportunityStagesT values (11, 'Lost Opportunity', 7, 'Opportunity has been lost.', 0)

-- (z.manning, 11/05/2007) - Added a table for opportunity payment methods.
INSERT INTO OpportunityPayMethodsT VALUES (1, 'Other')
INSERT INTO OpportunityPayMethodsT VALUES (2, 'Financing - Deal type 1')
INSERT INTO OpportunityPayMethodsT VALUES (3, 'Financing - Deal type 2')
INSERT INTO OpportunityPayMethodsT VALUES (4, 'Financing - Deal type 3')
INSERT INTO OpportunityPayMethodsT VALUES (5, 'Financing - Deal type 4')
INSERT INTO OpportunityPayMethodsT VALUES (6, 'Financing - Deal type 5')
INSERT INTO OpportunityPayMethodsT VALUES (7, 'Direct Payment')
INSERT INTO OpportunityPayMethodsT VALUES (8, 'Payment Installments')

INSERT INTO OpportunityPriceStructureT (Scheduler, LicenseSched, Billing, HCFA, EBilling, Letters, Quotes, Tracking, NexForms, Inventory, 
  NexSpa, NexASC, Mirror, United, Workstations, Doctors, PDA, EMRFirst, EMRAddtl, Training, Conversion, Travel, PackageScheduler,
  PackageFinancial, PackageCosmetic, HL7, Inform, Quickbooks, Active)
  values (3950, 1250, 2500, 2000, 3000, 1000, 1000, 1000, 3000, 2000, 3000, 3950, 750, 750, 1250, 1950, 750, 14500, 12500, 1000, 2950, 950, 3950, 6500, 5000, 1000, 1000, 750, 1)

INSERT INTO OpportunityMaxDiscountsT values (8001, 100)	--k.majeed
INSERT INTO OpportunityMaxDiscountsT values (14842, 20)	--m.rosenberg
--All sales
INSERT INTO OpportunityMaxDiscountsT values (12717, 15)
INSERT INTO OpportunityMaxDiscountsT values (14077, 15)
INSERT INTO OpportunityMaxDiscountsT values (11643, 15)
INSERT INTO OpportunityMaxDiscountsT values (13009, 15)
INSERT INTO OpportunityMaxDiscountsT values (7344, 15)
INSERT INTO OpportunityMaxDiscountsT values (13697, 15)
INSERT INTO OpportunityMaxDiscountsT values (12296, 15)
INSERT INTO OpportunityMaxDiscountsT values (13017, 15)
INSERT INTO OpportunityMaxDiscountsT values (7348, 15)
INSERT INTO OpportunityMaxDiscountsT values (8228, 15)
INSERT INTO OpportunityMaxDiscountsT values (14170, 15)

*/


/////////////////////////////////////////////////////////////////////////////
// COpportunityListDlg dialog


COpportunityListDlg::COpportunityListDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(COpportunityListDlg::IDD, pParent)
{
	// (j.armen 2012-06-06 12:39) - PLID 50830 - Set min size
	SetMinSize(900, 250);
}


void COpportunityListDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpportunityListDlg)
	DDX_Control(pDX, IDC_OPP_LIST_COUNT, m_labelOppListCount);
	DDX_Control(pDX, IDC_COUNT_LABEL, m_labelOppCountLabel);
	DDX_Control(pDX, IDC_SHOW_OPP_LABEL, m_labelShowOppLabel);
	DDX_Control(pDX, IDC_NEW_SALES_LABEL, m_labelNewSales);
	DDX_Control(pDX, IDC_ADDONS_LABEL, m_labelAddOns);
	DDX_Control(pDX, IDC_30_DAY_LABEL, m_30DayLabel);
	DDX_Control(pDX, IDC_30_DAY_TEXT, m_30DayText);
	DDX_Control(pDX, IDC_60_DAY_LABEL, m_60DayLabel);
	DDX_Control(pDX, IDC_60_DAY_TEXT, m_60DayText);
	DDX_Control(pDX, IDC_90_DAY_LABEL, m_90DayLabel);
	DDX_Control(pDX, IDC_90_DAY_TEXT, m_90DayText);
	DDX_Control(pDX, IDC_30_DAY_LABEL_ADDON, m_30DayLabelAddOn);
	DDX_Control(pDX, IDC_30_DAY_TEXT_ADDON, m_30DayTextAddOn);
	DDX_Control(pDX, IDC_60_DAY_LABEL_ADDON, m_60DayLabelAddOn);
	DDX_Control(pDX, IDC_60_DAY_TEXT_ADDON, m_60DayTextAddOn);
	DDX_Control(pDX, IDC_90_DAY_LABEL_ADDON, m_90DayLabelAddOn);
	DDX_Control(pDX, IDC_90_DAY_TEXT_ADDON, m_90DayTextAddOn);
	DDX_Control(pDX, IDC_RAD_LOST, m_btnLost);
	DDX_Control(pDX, IDC_RAD_CLOSED, m_btnClosed);
	DDX_Control(pDX, IDC_RAD_FORECAST, m_btnForecast);
	DDX_Control(pDX, IDC_RAD_COLLECTIONS, m_btnCollections);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_ADD_OPPORTUNITY, m_btnAddOpp);
	DDX_Control(pDX, IDC_OPP_SHOW_INACTIVE, m_btnShowInactive);
	DDX_Control(pDX, IDC_RAD_DAY_ALL, m_btnDayAll);
	DDX_Control(pDX, IDC_RAD_DAY_30, m_btnDay30);
	DDX_Control(pDX, IDC_RAD_DAY_60, m_btnDay60);
	DDX_Control(pDX, IDC_RAD_DAY_90, m_btnDay90);
	DDX_Control(pDX, IDC_DAYS_LABEL, m_labelDays);
	DDX_Control(pDX, IDC_SHOW_NEWSALE, m_btnNewSale);
	DDX_Control(pDX, IDC_SHOW_ADDON, m_btnAddOn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COpportunityListDlg, CNxDialog)
	ON_BN_CLICKED(IDC_RAD_COLLECTIONS, OnRadCollections)
	ON_BN_CLICKED(IDC_RAD_FORECAST, OnRadForecast)
	ON_BN_CLICKED(IDC_RAD_CLOSED, OnRadClosed)
	ON_BN_CLICKED(IDC_RAD_LOST, OnRadLost)
	ON_BN_CLICKED(IDC_ADD_OPPORTUNITY, OnAddOpportunity)
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_OPP_SHOW_INACTIVE, OnOppShowInactive)
	ON_BN_CLICKED(IDC_RAD_DAY_ALL, OnRadDayAll)
	ON_BN_CLICKED(IDC_RAD_DAY_30, OnRadDay30)
	ON_BN_CLICKED(IDC_RAD_DAY_60, OnRadDay60)
	ON_BN_CLICKED(IDC_RAD_DAY_90, OnRadDay90)
	ON_BN_CLICKED(IDC_SHOW_NEWSALE, &COpportunityListDlg::OnBnClickedShowNewsale)
	ON_BN_CLICKED(IDC_SHOW_ADDON, &COpportunityListDlg::OnBnClickedShowAddon)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpportunityListDlg message handlers

void COpportunityListDlg::OnOK() 
{
	ShowWindow(SW_HIDE);
}

void COpportunityListDlg::OnCancel() 
{
	ShowWindow(SW_HIDE);
}

BOOL COpportunityListDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//Size the window to the last size it was.  I copied this from the Todo alarm, which is why it's pulling
		//	the setting from the registry and not ConfigRT
		{
			// Get the work area to make sure that wherever we put it, it's accessible
			CRect rcWork;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
			// Get the last size and position of the window
			CRect rcDialog;
			CString strBuffer = AfxGetApp()->GetProfileString("Settings", "OppListSize");
			if (strBuffer.IsEmpty() || _stscanf(strBuffer, "%d,%d,%d,%d", &rcDialog.left, &rcDialog.top, &rcDialog.right, &rcDialog.bottom) != 4) {
				// We couldn't get the registry setting for some reason - Just leave it at whatever size it was default generated at
				GetWindowRect(rcDialog);
				ScreenToClient(rcDialog);
			}
			// Make sure if we put the dialog at rcDialog it's accessible (we consider 'accessible' 
			// to mean that the dialog title bar is visible vertically, and 1/3 visible horizontally)
			if (rcDialog.top+rcDialog.Height()/8<rcWork.bottom && rcDialog.top>rcWork.top &&
				rcDialog.left<rcWork.right-rcDialog.Width()/3 && rcDialog.right>rcWork.left+rcDialog.Width()/3) {
				// It's accessible so leave it
			} else {
				// It's not accessible so center it
				CSize ptDlgHalf(rcDialog.Width()/2, rcDialog.Height()/2);
				CPoint ptScreenCenter(rcWork.CenterPoint());
				rcDialog.SetRect(ptScreenCenter - ptDlgHalf, ptScreenCenter + ptDlgHalf);
			}
			// Move the window to its new position
			MoveWindow(rcDialog);
		}


		//Bind datalist controls
		m_pCoordList = BindNxDataList2Ctrl(this, IDC_COORD_LIST, GetRemoteData(), false);
		m_pList = BindNxDataList2Ctrl(this, IDC_OPPORTUNITY_LIST, GetRemoteData(), false);

		// (d.lange 2010-11-08 16:57) - PLID 41357 - After adding speciality, this from clause is getting too big for 
		// the datalist settings
		// (d.lange 2010-11-09 16:43) - PLID 41359 - Added Alliance
		// (j.armen 2011-11-02 13:36) - PLID 11490 - CustomListData was moved to CustomListDataT.  Handle display from the datalist.
		CString strFromClause = "OpportunitiesT "
								"LEFT JOIN PersonT ON OpportunitiesT.PersonID = PersonT.ID "
								"LEFT JOIN OpportunityTypesT ON OpportunitiesT.TypeID = OpportunityTypesT.ID "
								"LEFT JOIN OpportunityStagesT ON OpportunitiesT.CurrentStageID = OpportunityStagesT.ID "
								//"LEFT JOIN (SELECT PersonID, Text FROM CustomListDataT INNER JOIN CustomListItemsT ON CustomListDataT.CustomListItemsID = CustomListItemsT.ID WHERE FieldID = 24) SpecialityQ "
								//	"ON SpecialityQ.PersonID = PersonT.ID "
								/*"LEFT JOIN (SELECT OpportunityID, Name FROM AlliancePartnersT INNER JOIN OpportunityAllianceT "
										"ON AlliancePartnersT.ID = OpportunityAllianceT.PartnerID) AllianceQ "
									"ON AllianceQ.OpportunityID = OpportunitiesT.ID"*/;
		m_pList->FromClause = _bstr_t(strFromClause);

		//Setup the coordinator list, depending on permissions
		CString strWhere = "PatientCoordinator = 1 AND Archived = 0 ";
		if(GetCurrentUserPermissions(bioInternalOppSeeOthers) & sptRead) {
			//This user has access to see other user's opportunities.  Thus, we just make no additional filter
		}
		else {
			//This user does not have permission for all.  Everyone is allowed to access their own.
			CString str;
			str.Format("AND UsersT.PersonID = %li", GetCurrentUserID());
			strWhere += str;
		}
		m_pCoordList->WhereClause = _bstr_t(strWhere);
		m_pCoordList->Requery();

		//Coloring, etc to "prettify" the dialog
		//TODO - While sales is still nitpicking over colors, this is loaded by a non-cached configrt
		//	proprty so I can change it on the fly.
		g_propManager.EnableCaching(FALSE);
		DWORD dwColor = GetRemotePropertyInt("InternalPropBGColor", 10542240, 0, "<None>", false);
		g_propManager.EnableCaching(TRUE);
		((CNxColor*)GetDlgItem(IDC_OPP_LIST_COLOR1))->SetColor(dwColor);
		((CNxColor*)GetDlgItem(IDC_OPP_LIST_COLOR2))->SetColor(dwColor);

		//Set colors on all the other labels/buttons

		m_labelShowOppLabel.SetColor(dwColor);
		m_labelShowOppLabel.SetText("Show Opportunities For:");
		m_labelOppCountLabel.SetColor(dwColor);
		m_labelOppCountLabel.SetText("Opportunities:");
		m_labelOppListCount.SetColor(dwColor);
		m_labelOppListCount.SetText("0");
		m_labelNewSales.SetColor(dwColor);
		m_labelNewSales.SetText("New Sales");
		m_labelAddOns.SetColor(dwColor);
		m_labelAddOns.SetText("AddOns");
		m_30DayLabel.SetColor(dwColor);
		m_30DayLabel.SetText("30 Day:");
		m_30DayText.SetColor(dwColor);
		m_30DayText.SetText("$0.00");
		m_60DayLabel.SetColor(dwColor);
		m_60DayLabel.SetText("60 Day:");
		m_60DayText.SetColor(dwColor);
		m_60DayText.SetText("$0.00");
		m_90DayLabel.SetColor(dwColor);
		m_90DayLabel.SetText("90 Day:");
		m_90DayText.SetColor(dwColor);
		m_90DayText.SetText("$0.00");
		m_30DayLabelAddOn.SetColor(dwColor);
		m_30DayLabelAddOn.SetText("30 Day:");
		m_30DayTextAddOn.SetColor(dwColor);
		m_30DayTextAddOn.SetText("$0.00");
		m_60DayLabelAddOn.SetColor(dwColor);
		m_60DayLabelAddOn.SetText("60 Day:");
		m_60DayTextAddOn.SetColor(dwColor);
		m_60DayTextAddOn.SetText("$0.00");
		m_90DayLabelAddOn.SetColor(dwColor);
		m_90DayLabelAddOn.SetText("90 Day:");
		m_90DayTextAddOn.SetColor(dwColor);
		m_90DayTextAddOn.SetText("$0.00");
		m_labelDays.SetColor(dwColor);
		m_labelDays.SetText("Days");

		//NxIconButton setup
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAddOpp.AutoSet(NXB_NEW);

		//Set some default checkbox options
		CheckDlgButton(IDC_RAD_COLLECTIONS, TRUE);
		CheckDlgButton(IDC_RAD_FORECAST, TRUE);
		CheckDlgButton(IDC_RAD_DAY_ALL, BST_CHECKED);
		CheckDlgButton(IDC_RAD_DAY_30, BST_CHECKED);
		CheckDlgButton(IDC_RAD_DAY_60, BST_CHECKED);
		CheckDlgButton(IDC_RAD_DAY_90, BST_CHECKED);

		//Automatically select the current user as a coordinator
		BOOL bHasAllPermission = (GetCurrentUserPermissions(bioInternalOppSeeOthers) & sptRead);
		NXDATALIST2Lib::IRowSettingsPtr pCoordRow = m_pCoordList->SetSelByColumn(0, (long)GetCurrentUserID());
		//If they're a manager w/ all permissions, or a coordinator, let them in.  Otherwise, DENY!
		if(!bHasAllPermission && pCoordRow == NULL) {
			//The current user is not a coordinator, and not a manager
			MessageBox("Only patient coordinators may use the Opportunity list.");
			CDialog::OnCancel();
		}
		else {
			ReloadFromFilters();
		}

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COpportunityListDlg::OnRadCollections() 
{
	try {
		ReloadFromFilters();
	} NxCatchAll("Error in OnRadCollections");
}

void COpportunityListDlg::OnRadForecast() 
{
	try {
		ReloadFromFilters();
	} NxCatchAll("Error in OnRadForecast");
}

void COpportunityListDlg::OnRadClosed() 
{
	try {
		ReloadFromFilters();
	} NxCatchAll("Error in OnRadClosed");
}

void COpportunityListDlg::OnRadLost() 
{
	try {
		ReloadFromFilters();
	} NxCatchAll("Error in OnRadLost");
}

void COpportunityListDlg::OnAddOpportunity() 
{
	CWaitCursor wc;
	COpportunityEditDlg dlg(this);
	if(dlg.DoModal() == IDOK) {
		//Refresh
		ReloadFromFilters();
	}
}

void COpportunityListDlg::OnRadDayAll()
{
	try
	{
		ReloadFromFilters();

	}NxCatchAll("COpportunityListDlg::OnRadDayAll");
}

void COpportunityListDlg::OnRadDay30()
{
	try
	{
		ReloadFromFilters();

	}NxCatchAll("COpportunityListDlg::OnRadDay30");
}

void COpportunityListDlg::OnRadDay60()
{
	try
	{
		ReloadFromFilters();

	}NxCatchAll("COpportunityListDlg::OnRadDay60");
}

void COpportunityListDlg::OnRadDay90()
{
	try
	{
		ReloadFromFilters();

	}NxCatchAll("COpportunityListDlg::OnRadDay90");
}

BEGIN_EVENTSINK_MAP(COpportunityListDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(COpportunityListDlg)
	ON_EVENT(COpportunityListDlg, IDC_COORD_LIST, 16 /* SelChosen */, OnSelChosenCoordList, VTS_DISPATCH)
	ON_EVENT(COpportunityListDlg, IDC_COORD_LIST, 18 /* RequeryFinished */, OnRequeryFinishedCoordList, VTS_I2)
	ON_EVENT(COpportunityListDlg, IDC_OPPORTUNITY_LIST, 18 /* RequeryFinished */, OnRequeryFinishedOpportunityList, VTS_I2)
	ON_EVENT(COpportunityListDlg, IDC_OPPORTUNITY_LIST, 3 /* DblClickCell */, OnDblClickCellOpportunityList, VTS_DISPATCH VTS_I2)
	ON_EVENT(COpportunityListDlg, IDC_OPPORTUNITY_LIST, 4 /* LButtonDown */, OnLButtonDownOpportunityList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(COpportunityListDlg, IDC_OPPORTUNITY_LIST, 6 /* RButtonDown */, OnRButtonDownOpportunityList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void COpportunityListDlg::OnSelChosenCoordList(LPDISPATCH lpRow) 
{
	try {
		ReloadFromFilters();

	} NxCatchAll("Error in OnSelChosenCoordList");
}

void COpportunityListDlg::OnRequeryFinishedCoordList(short nFlags)
{
	try
	{
		// (z.manning, 11/26/2007) - PLID 27971 - Add an option for all coordinatiors if they have permission to see others.
		if(GetCurrentUserPermissions(bioInternalOppSeeOthers) & sptRead) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCoordList->GetNewRow();
			pRow->PutValue(0, (long)-1);
			pRow->PutValue(1, "{ All }");
			m_pCoordList->AddRowBefore(pRow, m_pCoordList->GetFirstRow());
		}

	}NxCatchAll("COpportunityListDlg::OnRequeryFinishedCoordList");
}

void COpportunityListDlg::ReloadFromFilters()
{
	//Clear the list
	m_pList->Clear();

	//1)  Coordinator.  Users are allowed to see all opportunities that they are a part of.   So we must filter on 
	//	everyone that is a primary, secondary, or an associate.
	NXDATALIST2Lib::IRowSettingsPtr pCoordRow = m_pCoordList->CurSel;
	if(pCoordRow == NULL) {
		//If no coordinator, we just quit
		return;
	}

	CString strWhere;
	long nCoordID = VarLong(pCoordRow->GetValue(0));
	if(nCoordID > 0) {
		strWhere.Format("(PrimaryCoordID = %li OR SecondaryCoordID = %li OR OpportunitiesT.ID IN (SELECT OpportunityID FROM OpportunityAssociatesT WHERE AssocID = %li)) ", 
			nCoordID, nCoordID, nCoordID);
	}
	else {
		// (z.manning, 11/26/2007) - PLID 27971 - They must have selected all coordinators so don't filter on a user ID.
		strWhere = "1=1 ";
	}

	//2)  Filter on all the possible checkboxes.  Note that these are a mix of category & stage, and are pretty much hardcoded.
	//	TODO:  We need to come up with a method for assigning these so the stages/cats can be editable.  The '0' stage items (there
	//	are several) below have to be filtered on the ID of the stage, which must remain hardcoded for this to work.
	CString strFilter, strTmp;

	// Pipeline
	//	Suspects, Leads, and Prospects in the Qualification or Needs Analysis stages
	// (z.manning, 11/21/2007) - PLID 27971 - The pipeline filter checkbox is gone and these should now be part
	// of the forecast filter.
	// Forecast
	// (d.lange 2010-12-17 16:23) - PLID 41355 - the stage number need to be changed to sequential letters
	if(IsDlgButtonChecked(IDC_RAD_FORECAST)) {
		strTmp.Format(" OR (OpportunityStagesT.StageNumber = 'B' OR OpportunityStagesT.StageNumber = 'A' OR OpportunitiesT.CurrentStageID = 7 OR OpportunityStagesT.ID = 1 OR OpportunityStagesT.ID = 2 OR OpportunityStagesT.StageNumber = 'D' OR OpportunityStagesT.StageNumber = 'C') ");
		strFilter += strTmp;
	}

	// Collections
	if(IsDlgButtonChecked(IDC_RAD_COLLECTIONS)) {
		strTmp.Format(" OR (OpportunitiesT.CurrentStageID = 8) ");
		strFilter += strTmp;
	}

	// Won/Closed
	if(IsDlgButtonChecked(IDC_RAD_CLOSED)) {
		strTmp.Format(" OR (OpportunitiesT.CurrentStageID = 9 OR OpportunitiesT.CurrentStageID = 10) ");
		strFilter += strTmp;
	}

	// Lost
	// (d.lange 2010-12-17 16:23) - PLID 41355 - the stage number need to be changed to sequential letters
	if(IsDlgButtonChecked(IDC_RAD_LOST)) {
		strTmp.Format(" OR (OpportunityStagesT.StageNumber = 'F') ");
		strFilter += strTmp;
	}

	// (z.manning, 11/21/2007) - PLID 27971 - Handle the 30/60/90 day filters.
	CString strDaysFilter;
	if(IsDlgButtonChecked(IDC_RAD_DAY_30) == BST_CHECKED && IsDlgButtonChecked(IDC_RAD_DAY_60) == BST_CHECKED 
		&& IsDlgButtonChecked(IDC_RAD_DAY_90) == BST_CHECKED  && IsDlgButtonChecked(IDC_RAD_DAY_ALL) == BST_CHECKED)
	{
		// (z.manning, 12/11/2007) - All are selected, no need to filter anything.
	}
	else
	{
		// (z.manning, 11/21/2007) - Do this based on Don's comment in COpportunityListDlg::OnRequeryFinishedOpportunityList
		//DRT 8/9/2007 - Add up the 30/60/90 day totals.  NOTE:  These aren't actually 30/60/90 days.  They do it 'per month', so 
		//	if today is 8/9, then the 30 day is through the end of August.  60 is through the end of September, etc.  Even if
		//	today is August 31, the 30 day is through August 31.
		int nMonth = COleDateTime::GetCurrentTime().GetMonth();
		int nYear = COleDateTime::GetCurrentTime().GetYear();
		COleDateTime dt30, dt60, dt90;

		nMonth++;
		if(nMonth > 12) {
			nYear++;
			nMonth = 1;
		}
		dt30.SetDate(nYear, nMonth, 1);
		if(IsDlgButtonChecked(IDC_RAD_DAY_30) == BST_CHECKED) {
			strDaysFilter += FormatString(" EstCloseDate < '%s' OR ", FormatDateTimeForSql(dt30, dtoDate));
		}

		nMonth++;
		if(nMonth > 12) {
			nYear++;
			nMonth = 1;
		}
		dt60.SetDate(nYear, nMonth, 1);
		if(IsDlgButtonChecked(IDC_RAD_DAY_60) == BST_CHECKED) {
			strDaysFilter += FormatString(" (EstCloseDate >= '%s' AND EstCloseDate < '%s') OR ", FormatDateTimeForSql(dt30, dtoDate), FormatDateTimeForSql(dt60, dtoDate));
		}

		nMonth++;
		if(nMonth > 12) {
			nYear++;
			nMonth = 1;
		}
		dt90.SetDate(nYear, nMonth, 1);
		if(IsDlgButtonChecked(IDC_RAD_DAY_90) == BST_CHECKED) {
			strDaysFilter += FormatString(" (EstCloseDate >= '%s' AND EstCloseDate < '%s') OR ", FormatDateTimeForSql(dt60, dtoDate), FormatDateTimeForSql(dt90, dtoDate));
		}

		if(IsDlgButtonChecked(IDC_RAD_DAY_ALL) == BST_CHECKED) {
			strDaysFilter += FormatString(" EstCloseDate >= '%s' OR ", FormatDateTimeForSql(dt90, dtoDate));
		}

		if(strDaysFilter.IsEmpty()) {
			// (z.manning, 12/11/2007) - PLID 27971 - No date filters are selected so we need to filter everything out.
			strDaysFilter = " 1=0 ";
		}
		else {
			// (z.manning, 12/11/2007) - Remove the final "OR" from the date part of the where clause.
			strDaysFilter.TrimRight();
			strDaysFilter.TrimRight("OR");
		}
	}


	//Now piece it all together
	if(strFilter.IsEmpty()) {
		//No filters chosen, we cannot show any data.  Use the requery still so the count, etc are updated
		strWhere += "AND (1=0) ";
	}
	else {
		//Strip the leading OR
		strFilter.TrimLeft(" OR ");
		strWhere += "AND (" + strFilter + ")";
	}

	if(!strDaysFilter.IsEmpty()) {
		strWhere += " AND (" + strDaysFilter + ")";
	}

	// (d.lange 2010-11-11 12:15) - PLID 41441 - Added New Sale filter
	CString strTemp = "";
	if(IsDlgButtonChecked(IDC_SHOW_NEWSALE)) {
		strTemp += " OpportunityTypesT.ID = 1 OR";
	}

	// (d.lange 2010-11-11 13:53) - PLID 41441 - Added AddOn filter
	if(IsDlgButtonChecked(IDC_SHOW_ADDON)) {
		strTemp += " OpportunityTypesT.ID = 2 OR OpportunityTypesT.ID = 4 OR";
	}

	if(!strTemp.IsEmpty()) {
		strTemp.TrimRight("OR");
		strWhere += " AND (" + strTemp + ")";
	}

	m_pList->WhereClause = _bstr_t(strWhere);
	m_pList->Requery();
	
}

void COpportunityListDlg::OnRequeryFinishedOpportunityList(short nFlags) 
{
	try {
	
		//DRT 8/9/2007 - Add up the 30/60/90 day totals.  NOTE:  These aren't actually 30/60/90 days.  They do it 'per month', so 
		//	if today is 8/9, then the 30 day is through the end of August.  60 is through the end of September, etc.  Even if
		//	today is August 31, the 30 day is through August 31.
		// (z.manning, 11/07/2007) - PLID 27971 - Added separate totals for AddOn sales.
		COleCurrency cy30, cy60, cy90, cy30AddOn, cy60AddOn, cy90AddOn;

		//Figure out what counts as 30/60/90 markers
		COleDateTime dtCurrent = COleDateTime::GetCurrentTime();
		long nMonth = dtCurrent.GetMonth();
		long nYear = dtCurrent.GetYear();

		//For 30 day
		nMonth++;
		if(nMonth > 12) {
			nMonth = 1;
			nYear++;
		}

		//Set the ending period to the beginning of the next month
		COleDateTime dtEndOf30( nYear, nMonth, 1, 0, 0, 0);

		//For 60 day
		nMonth++;
		if(nMonth > 12) {
			nMonth = 1;
			nYear++;
		}

		//Set the ending period to the beginning of the next month
		COleDateTime dtEndOf60( nYear, nMonth, 1, 0, 0, 0);

		//For 60 day
		nMonth++;
		if(nMonth > 12) {
			nMonth = 1;
			nYear++;
		}

		//Set the ending period to the beginning of the next month
		COleDateTime dtEndOf90( nYear, nMonth, 1, 0, 0, 0);


		//Traverse the datalist
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->FindAbsoluteFirstRow(VARIANT_TRUE);

		long nVisibleRowCount = 0;
		while (pRow)
		{
			if(VarBool(pRow->GetValue(elcInactive))) {
				// (z.manning, 11/05/2007) - PLID 27971 - Gray out inactive opportunities.
				pRow->PutForeColor(INACTIVE_ROW_COLOR);
				if(!IsShowingInactive()) {
					// (z.manning, 11/05/2007) - Also hide them unless we are showing inactive.
					pRow->PutVisible(VARIANT_FALSE);
				}
			}
			else
			{
				// (z.manning, 11/07/2007) - Ignore inactive rows when calculating the totals.
				nVisibleRowCount++;

				COleCurrency cyAmt = VarCurrency(pRow->GetValue(elcAmount), COleCurrency(0, 0));
				long nSecondaryCoord = VarLong(pRow->GetValue(elcSecondaryCoord), -1);

				//If there are 2 coordinators, the amount gets cut in half
				if(nSecondaryCoord != -1)
					cyAmt /= 2;

				BOOL bIsAddOn = IsAddOn(VarLong(pRow->GetValue(elcTypeID)));

				//This will include anything "past due" in the 30 day calculation.  So if you forget to update, and
				//	the est close date is 7/31, and today is 8/4, that proposal will count as 30 day.
				COleDateTime dtEst = VarDateTime(pRow->GetValue(elcEstCloseDate));
				if(dtEst < dtEndOf30) {
					if(bIsAddOn) {
						cy30AddOn += cyAmt;
					}
					else {
						cy30 += cyAmt;
					}
				}
				else if(dtEst < dtEndOf60) {
					if(bIsAddOn) {
						cy60AddOn += cyAmt;
					}
					else {
						cy60 += cyAmt;
					}
				}
				else if(dtEst < dtEndOf90) {
					if(bIsAddOn) {
						cy90AddOn += cyAmt;
					}
					else {
						cy90 += cyAmt;
					}
				}
				else {
					//We don't care past 90
				}
			}

			//move to the next row
			pRow = m_pList->FindAbsoluteNextRow(pRow, VARIANT_TRUE);
		}

		m_labelOppListCount.SetText(FormatString("%li", nVisibleRowCount));
		InvalidateDlgItem(IDC_OPP_LIST_COUNT);

		//Now update our text markers
		m_30DayText.SetText(FormatCurrencyForInterface(cy30));
		InvalidateDlgItem(IDC_30_DAY_TEXT);
		m_30DayText.RedrawWindow();		// (d.lange 2010-12-20 16:38) - PLID 41333 - Redraw the label
		m_60DayText.SetText(FormatCurrencyForInterface(cy60));
		InvalidateDlgItem(IDC_60_DAY_TEXT);
		m_60DayText.RedrawWindow();		// (d.lange 2010-12-20 16:38) - PLID 41333 - Redraw the label
		m_90DayText.SetText(FormatCurrencyForInterface(cy90));
		InvalidateDlgItem(IDC_90_DAY_TEXT);
		m_90DayText.RedrawWindow();		// (d.lange 2010-12-20 16:38) - PLID 41333 - Redraw the label
		m_30DayTextAddOn.SetText(FormatCurrencyForInterface(cy30AddOn));
		InvalidateDlgItem(IDC_30_DAY_TEXT_ADDON);
		m_30DayTextAddOn.RedrawWindow();	// (d.lange 2010-12-20 16:38) - PLID 41333 - Redraw the label
		m_60DayTextAddOn.SetText(FormatCurrencyForInterface(cy60AddOn));
		InvalidateDlgItem(IDC_60_DAY_TEXT_ADDON);
		m_60DayTextAddOn.RedrawWindow();	// (d.lange 2010-12-20 16:38) - PLID 41333 - Redraw the label
		m_90DayTextAddOn.SetText(FormatCurrencyForInterface(cy90AddOn));
		InvalidateDlgItem(IDC_90_DAY_TEXT_ADDON);
		m_90DayTextAddOn.RedrawWindow();	// (d.lange 2010-12-20 16:38) - PLID 41333 - Redraw the label

		// (d.lange 2010-12-01 09:53) - PLID  41336 - Color the text on specific rows
		SetRowColors();

	} NxCatchAll("Error in OnRequeryFinishedOpportunityList");
}

void COpportunityListDlg::OnDblClickCellOpportunityList(LPDISPATCH lpRow, short nColIndex) 
{
	CWaitCursor wc;
	try {
		//no data
		if(lpRow == NULL)
			return;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		//Load the ID into the editor dialog
		COpportunityEditDlg dlg(this);
		dlg.SetID( VarLong(pRow->GetValue(elcID)) );

		//launch!
		dlg.DoModal();

		//Refresh
		ReloadFromFilters();

	} NxCatchAll("Error in OnDblClickCellOpportunityList");
}

void COpportunityListDlg::OnLButtonDownOpportunityList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	CWaitCursor wc;
	try {
		if(lpRow == NULL)
			return;

		switch(nCol) {
		case elcClientName:
			{	//Client name - We want to open this patient account
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				long nClientID = VarLong(pRow->GetValue(elcClientID));
				if(nClientID == -1) {
					//Should not be possible, but if so, just quit
					return;
				}

				CMainFrame *p = GetMainFrame();
				CNxTabView *pView;

				//See if this client is in our active lookup, if not, we need to reset the filters
				if (nClientID != GetActivePatientID()) {
					if(!p->m_patToolBar.DoesPatientExistInList(nClientID)) {
						if(IDNO == MessageBox("This patient is not in the current lookup. \n"
							"Do you wish to reset the lookup to include all patients?","Practice", MB_ICONQUESTION|MB_YESNO)) {
							return;
						}
					}
					//TES 1/7/2010 - PLID 36761 - This function may fail now
					if(!p->m_patToolBar.TrySetActivePatientID(nClientID)) {
						return;
					}
				}	

				//Move to the patients module
				if(p->FlipToModule(PATIENT_MODULE_NAME)) {

					pView = (CNxTabView *)p->GetOpenView(PATIENT_MODULE_NAME);
					if (pView) 
					{
						//Load the preferred tab.  We'll use the same property as the todo alarm, no sense making a new one
						//	for an internal-only feature.
						long nPreferredTab = g_Modules[Modules::Patients]->ResolveDefaultTab((short)GetRemotePropertyInt("MyDefaultToDoTab_Patients", 0, 0, GetCurrentUserName(), true));
						
						// If the active tab != preferred tab, set the tab to the preferred tab
						if(nPreferredTab != pView->GetActiveTab())
							pView->SetActiveTab((short)nPreferredTab);

						//Hide or minize this opportunity list
						ShowWindow(SW_MINIMIZE);

						// Display the tab
						pView->UpdateView();
					}
				}
			}
			break;
		case elcStage:	// (d.lange 2010-11-05 11:50) - PLID 41337 - Added a hyperlink to the stage column to go to that specific opportunity
		case elcOppName:
			//Opportunity name - This is the same as double clicking
			OnDblClickCellOpportunityList(lpRow, nCol);
			break;

		case elcAmount:
			{
				//Clicked on amount, we want to open the active proposal
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				long nProposalID = VarLong(pRow->GetValue(elcActiveProposalID), -1);
				if(nProposalID == -1) {
					//There is no active proposal, which means they have no proposals at all
					MessageBox("There are no proposals yet created for this patient.");
					return;
				}
				long nClientID = VarLong(pRow->GetValue(elcClientID));
				long nOpportunityID = VarLong(pRow->GetValue(elcID));
				CString strClientName = VarString(pRow->GetValue(elcClientName));

				//DRT 4/1/2008 - PLID 29493 - We need the type now
				long nTypeID = VarLong(pRow->GetValue(elcTypeID), -1);

				// (d.thompson 2009-08-27) - PLID 35365 - Need the pay method to determine financing
				bool bIsFinancing = false;
				{
					_variant_t varMethod = pRow->GetValue(elcPayMethod);
					if(varMethod.vt == VT_I4) {
						//Again, these are hardcoded.  Match with OpportunityEdit screen.
						if(VarLong(varMethod) >= 2 && VarLong(varMethod) <= 6) {
							bIsFinancing = true;
						}
					}
				}

				//Now launch the proposal dialog
				COpportunityProposalDlg dlg(this);
				dlg.OpenProposal(nProposalID, nOpportunityID, nClientID, strClientName, nTypeID, bIsFinancing);

				//They may have changed the proposal, we have to reload
				ReloadFromFilters();
			}
			break;
		// (d.lange 2010-11-30 11:41) - PLID 41336 - Added Next Follow Up date and when clicked will bring up the ToDo alarm
		case elcNextFollowUp:
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				long nTaskID = VarLong(pRow->GetValue(elcNextFollowUpID), -1);
				
				CTaskEditDlg dlg(this);

				dlg.m_nPersonID = VarLong(pRow->GetValue(elcClientID), -1);
				//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields 
				if(nTaskID != -1) {
					dlg.m_iTaskID = nTaskID;
					dlg.m_bIsNew = FALSE;
				}
				else {
					dlg.m_bIsNew = TRUE;
				}

				if(dlg.DoModal() != IDCANCEL) {
					ReloadFromFilters();
				}
			}
			break;
		default:
			//Nothing
			break;
		};


	} NxCatchAll("Error in OnLButtonDownOpportunityList");
}

void COpportunityListDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	try {
		switch(nType) {
		// (d.lange 2010-12-20 15:35) - PLID 41347 - Added the ability to maximize the Opportunities list dialog
		case SIZE_MAXIMIZED:	//maximized
		case SIZE_RESTORED:
			{	//This window was resized.
				CWnd* pWnd = NULL;

				const long MARGIN_BUFFER = 20;

				//Stretch the topmost nxcolor in the x direction only
				pWnd = GetDlgItem(IDC_OPP_LIST_COLOR1);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					//Do not move, just resize so that it is the width of the screen minus a buffer.  height stays the same always
					pWnd->SetWindowPos(NULL, 0, 0, cx - MARGIN_BUFFER, rcOld.bottom - rcOld.top, SWP_NOMOVE|SWP_NOZORDER);
				}

				//The add opportunity button should be tied to the bottom left, aligned with the datalist on the left
				CRect rcAddOpp;
				pWnd = GetDlgItem(IDC_ADD_OPPORTUNITY);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);
					rcAddOpp = rcOld;

					//The coordinator list is static left-most point
					CWnd* pWndList = GetDlgItem(IDC_COORD_LIST);
					if(pWndList) {
						CRect rcCoord;
						pWndList->GetWindowRect(rcCoord);
						ScreenToClient(rcCoord);

						//Do not resize.  Just move so that the left & bottom are along the bottom of where we moved to
						pWnd->SetWindowPos(NULL, rcCoord.left, cy - ((int)MARGIN_BUFFER / 2) - (rcOld.Height()), 0, 0, SWP_NOSIZE|SWP_NOZORDER);
					}
				}

				//Stretch the main list
				CRect rcList;
				pWnd = GetDlgItem(IDC_OPPORTUNITY_LIST);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					// (d.lange 2010-12-20 15:36) - PLID 41347 - Need to grab the new position of the 'Add Opportunity' button
					CWnd* pWndAdd = GetDlgItem(IDC_ADD_OPPORTUNITY);
					if(pWndAdd) {
						CRect rcAddNew;
						pWndAdd->GetWindowRect(rcAddNew);
						ScreenToClient(rcAddNew);

						//Set width to double the buffer
						pWnd->SetWindowPos(NULL, 0, 0, cx - ((int)(MARGIN_BUFFER*2.5)), rcAddNew.top - rcOld.top - ((int)(MARGIN_BUFFER*2.5)), SWP_NOMOVE|SWP_NOZORDER);
						pWnd->GetWindowRect(rcList);
						ScreenToClient(rcList);
					}
				}

				//The close button should be tied to the bottom right, aligned with the datalist on the right
				CRect rcClose;
				pWnd = GetDlgItem(IDOK);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					//Do not resize, just move to bottom right
					pWnd->SetWindowPos(NULL, rcList.right - (rcOld.Width()), cy - ((int)MARGIN_BUFFER / 2) - (rcOld.Height()), 0, 0, SWP_NOSIZE|SWP_NOZORDER);
					CRect rcTmp;
					pWnd->GetWindowRect(rcClose);
					ScreenToClient(rcClose);

					pWnd->RedrawWindow();
				}

				//Stretch the big nxcolor in the x and y both
				pWnd = GetDlgItem(IDC_OPP_LIST_COLOR2);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					//Do not move, just resize so that it is the width of the screen minus a buffer.  Height is based off the 
					//	close button below it
					pWnd->SetWindowPos(NULL, 0, 0, cx - MARGIN_BUFFER, rcClose.top - 5 - rcOld.top, SWP_NOMOVE|SWP_NOZORDER);
				}

				//Move the # of opportunities label to the bottom right
				CRect rcCount;
				pWnd = GetDlgItem(IDC_OPP_LIST_COUNT);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					//No change in size, just movement
					pWnd->SetWindowPos(NULL, rcList.right - rcOld.Width(), rcList.bottom + 5, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//get the new position
					pWnd->GetWindowRect(rcCount);
					ScreenToClient(rcCount);
					InvalidateDlgItem(IDC_OPP_LIST_COUNT);

					pWnd->RedrawWindow();
				}

				//Label for the opportunity count
				pWnd = GetDlgItem(IDC_COUNT_LABEL);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					//no change in size, just move it
					pWnd->SetWindowPos(NULL, rcCount.left - rcOld.Width() - 5, rcCount.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//Invalidate the area
					InvalidateDlgItem(IDC_COUNT_LABEL);

					pWnd->RedrawWindow();
				}
				
				// (z.manning, 11/08/2007) - PLID 27971 - New sales label
				pWnd = GetDlgItem(IDC_NEW_SALES_LABEL);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					// (d.lange 2010-12-20 16:38) - PLID 41333 - Now the size and position never changes
					pWnd->SetWindowPos(NULL, rcOld.left, rcOld.top/*rcList.bottom + 5*/, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//Invalidate the area
					InvalidateDlgItem(IDC_NEW_SALES_LABEL);
					pWnd->RedrawWindow();
				}

				//DRT 8/9/2007 - Added 6 labels for 30/60/90 day amounts.  These will not move in the x position, only y
				//Label for 30 day
				pWnd = GetDlgItem(IDC_30_DAY_LABEL);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					// (d.lange 2010-12-20 16:38) - PLID 41333 - Now the size and position never changes
					pWnd->SetWindowPos(NULL, rcOld.left, rcOld.top/*rcList.bottom + 5*/, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//Invalidate the area
					InvalidateDlgItem(IDC_30_DAY_LABEL);
					pWnd->RedrawWindow();
				}

				//Text for 30 day
				pWnd = GetDlgItem(IDC_30_DAY_TEXT);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					// (d.lange 2010-12-20 16:38) - PLID 41333 - Now the size and position never changes
					pWnd->SetWindowPos(NULL, rcOld.left, rcOld.top/*rcList.bottom + 5*/, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//Invalidate the area
					InvalidateDlgItem(IDC_30_DAY_TEXT);
					pWnd->RedrawWindow();
				}

				//Label for 60 day
				pWnd = GetDlgItem(IDC_60_DAY_LABEL);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					// (d.lange 2010-12-20 16:38) - PLID 41333 - Now the size and position never changes
					pWnd->SetWindowPos(NULL, rcOld.left, rcOld.top/*rcList.bottom + 5*/, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//Invalidate the area
					InvalidateDlgItem(IDC_60_DAY_LABEL);
					pWnd->RedrawWindow();
				}

				//Text for 60 day
				pWnd = GetDlgItem(IDC_60_DAY_TEXT);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					// (d.lange 2010-12-20 16:38) - PLID 41333 - Now the size and position never changes
					pWnd->SetWindowPos(NULL, rcOld.left, rcOld.top/*rcList.bottom + 5*/, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//Invalidate the area
					InvalidateDlgItem(IDC_60_DAY_TEXT);
					pWnd->RedrawWindow();
				}

				//Label for 90 day
				pWnd = GetDlgItem(IDC_90_DAY_LABEL);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					// (d.lange 2010-12-20 16:38) - PLID 41333 - Now the size and position never changes
					pWnd->SetWindowPos(NULL, rcOld.left, rcOld.top/*rcList.bottom + 5*/, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//Invalidate the area
					InvalidateDlgItem(IDC_90_DAY_LABEL);
					pWnd->RedrawWindow();
				}

				//Text for 90 day
				pWnd = GetDlgItem(IDC_90_DAY_TEXT);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					// (d.lange 2010-12-20 16:38) - PLID 41333 - Now the size and position never changes
					pWnd->SetWindowPos(NULL, rcOld.left, rcOld.top/*rcList.bottom + 5*/, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//Invalidate the area
					InvalidateDlgItem(IDC_90_DAY_TEXT);
					pWnd->RedrawWindow();
				}

				int nSecondRowOffset = 23;
				// (z.manning, 11/08/2007) - PLID 27971 - AddOns label
				pWnd = GetDlgItem(IDC_ADDONS_LABEL);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					// (d.lange 2010-12-20 16:38) - PLID 41333 - Now the size and position never changes
					pWnd->SetWindowPos(NULL, rcOld.left, rcOld.top/*rcList.bottom + nSecondRowOffset*/, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//Invalidate the area
					InvalidateDlgItem(IDC_ADDONS_LABEL);
					pWnd->RedrawWindow();
				}

				// (z.manning, 11/08/2007) - PLID 27971 - Added a 2nd row of 30/60/90 totals to separate add on sales
				// from new sales.
				pWnd = GetDlgItem(IDC_30_DAY_LABEL_ADDON);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					// (d.lange 2010-12-20 16:38) - PLID 41333 - Now the size and position never changes
					pWnd->SetWindowPos(NULL, rcOld.left, rcOld.top/*rcList.bottom + nSecondRowOffset*/, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//Invalidate the area
					InvalidateDlgItem(IDC_30_DAY_LABEL_ADDON);
					pWnd->RedrawWindow();
				}

				//Text for 30 day AddOns
				pWnd = GetDlgItem(IDC_30_DAY_TEXT_ADDON);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					// (d.lange 2010-12-20 16:38) - PLID 41333 - Now the size and position never changes
					pWnd->SetWindowPos(NULL, rcOld.left, rcOld.top/*rcList.bottom + nSecondRowOffset*/, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//Invalidate the area
					InvalidateDlgItem(IDC_30_DAY_TEXT_ADDON);
					pWnd->RedrawWindow();
				}

				//Label for 60 day AddOns
				pWnd = GetDlgItem(IDC_60_DAY_LABEL_ADDON);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					// (d.lange 2010-12-20 16:38) - PLID 41333 - Now the size and position never changes
					pWnd->SetWindowPos(NULL, rcOld.left, rcOld.top/*rcList.bottom + nSecondRowOffset*/, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//Invalidate the area
					InvalidateDlgItem(IDC_60_DAY_LABEL_ADDON);
					pWnd->RedrawWindow();
				}

				//Text for 60 day AddOns
				pWnd = GetDlgItem(IDC_60_DAY_TEXT_ADDON);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					// (d.lange 2010-12-20 16:38) - PLID 41333 - Now the size and position never changes
					pWnd->SetWindowPos(NULL, rcOld.left, rcOld.top/*rcList.bottom + nSecondRowOffset*/, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//Invalidate the area
					InvalidateDlgItem(IDC_60_DAY_TEXT_ADDON);
					pWnd->RedrawWindow();
				}

				//Label for 90 day AddOns
				pWnd = GetDlgItem(IDC_90_DAY_LABEL_ADDON);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					// (d.lange 2010-12-20 16:38) - PLID 41333 - Now the size and position never changes
					pWnd->SetWindowPos(NULL, rcOld.left, rcOld.top/*rcList.bottom + nSecondRowOffset*/, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//Invalidate the area
					InvalidateDlgItem(IDC_90_DAY_LABEL_ADDON);
					pWnd->RedrawWindow();
				}

				//Text for 90 day AddOns
				pWnd = GetDlgItem(IDC_90_DAY_TEXT_ADDON);
				if(pWnd) {
					CRect rcOld;
					pWnd->GetWindowRect(rcOld);
					ScreenToClient(rcOld);

					// (d.lange 2010-12-20 16:38) - PLID 41333 - Now the size and position never changes
					pWnd->SetWindowPos(NULL, rcOld.left, rcOld.top/*rcList.bottom + nSecondRowOffset*/, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

					//Invalidate the area
					InvalidateDlgItem(IDC_90_DAY_TEXT_ADDON);
					pWnd->RedrawWindow();
				}

			}
			break;

		case SIZE_MINIMIZED:	//minized
		case SIZE_MAXSHOW:		//another window was maximized
		case SIZE_MAXHIDE:		//another window was minimized
			//Do not need to do anything with these
			break;

		}


	} NxCatchAll("Error in OnSize");
}

void COpportunityListDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	try {
		if(bShow) {
			//Copied this from the todo alarm.  If the user has chosen that icons should be displayed in the
			//	taskbar, then put the Opportunity list dialog there as well.
			int nTaskbar = GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true);
			HWND hwnd = GetSafeHwnd();
			long nStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			if (nTaskbar == 1)
				nStyle |= WS_EX_APPWINDOW;
			else
				nStyle &= !WS_EX_APPWINDOW;
			SetWindowLong(hwnd, GWL_EXSTYLE, nStyle);
		}

	} NxCatchAll("Error in OnShowWindow");
}

void COpportunityListDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	try {
		//Now save the box size and position - Copied this from the todo alarm
		//TES 5/27/2004: Note that we are using GetWindowPlacement; this gives us the rect for the window when it isn't 
		//minimized, even if at the moment it happens to be minimized.
		WINDOWPLACEMENT wp;
		GetWindowPlacement(&wp);
		CString strBuffer;
		strBuffer.Format("%d,%d,%d,%d", wp.rcNormalPosition.left, wp.rcNormalPosition.top, wp.rcNormalPosition.right, wp.rcNormalPosition.bottom);
		AfxGetApp()->WriteProfileString("Settings", "OppListSize", strBuffer);

	} NxCatchAll("Error in OnDestroy");
}

// (z.manning, 11/05/2007) - PLID 27971 - Returns true if we are showing inactive opportunites and false if we're not.
BOOL COpportunityListDlg::IsShowingInactive()
{
	if(IsDlgButtonChecked(IDC_OPP_SHOW_INACTIVE) == BST_CHECKED) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// (z.manning, 11/05/2007) - PLID 27971 - Goes through every row (including currently invisible ones) and either
// shows or hides all inactive rows.
void COpportunityListDlg::OnOppShowInactive()
{
	try
	{
		VARIANT_BOOL bShowInactive = IsShowingInactive();

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		for(pRow = m_pList->FindAbsoluteFirstRow(VARIANT_FALSE); pRow != NULL; pRow = m_pList->FindAbsoluteNextRow(pRow, VARIANT_FALSE))
		{
			if(bShowInactive) {
				// (z.manning, 11/05/2007) - We're showing inactive, so make every row visible.
				pRow->PutVisible(VARIANT_TRUE);
			}
			else if(VarBool(pRow->GetValue(elcInactive))) {
				// (z.manning, 11/05/2007) - We're not showing inactive, and this row in inactive, so make it invisible.
				pRow->PutVisible(VARIANT_FALSE);
			}
		}

	}NxCatchAll("COpportunityListDlg::OnOppShowInactive");
}

// (z.manning, 11/05/2007) - PLID 27971 - If the user right clicks on an opportunity then give them a popup
// menu with the option to either activate or inactivate that opportunity.
void COpportunityListDlg::OnRButtonDownOpportunityList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		if(lpRow == NULL) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		//Set the cursel to this row
		m_pList->PutCurSel(pRow);

		BOOL bIsCurrentlyInactive = VarBool(pRow->GetValue(elcInactive));

		//Create a 1 option menu depending on the current active status of the row
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		if(bIsCurrentlyInactive) {
			mnu.InsertMenu(0, MF_BYPOSITION, 1, "Mark &Active");
		}
		else {
			mnu.InsertMenu(0, MF_BYPOSITION, 1, "Mark &Inactive");
		}

		//Popup the menu at the x/y
		CPoint pt;
		GetCursorPos(&pt);
		if(mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL) == 1)
		{
			//They selected to toggle the inactive status
			short nNewInactiveValue;
			if(bIsCurrentlyInactive)
			{
				// (z.manning, 11/05/2007) - It's being changed from inactive to active.
				nNewInactiveValue = 0;
				// (z.manning, 11/05/2007) - They right clicked on it, so it must already be visible.
				ASSERT(pRow->GetVisible());
				pRow->PutForeColor(RGB(0,0,0));
			}
			else
			{
				// (z.manning, 11/05/2007) - It's being changed from active to inactive.
				nNewInactiveValue = 1;
				if(!IsShowingInactive()) {
					pRow->PutVisible(VARIANT_FALSE);
				}
				pRow->PutForeColor(INACTIVE_ROW_COLOR);
			}

			// (z.manning, 11/05/2007) - Update the active status in data.
			ExecuteParamSql("UPDATE OpportunitiesT SET Inactive = {INT} WHERE ID = {INT}"
				, nNewInactiveValue, VarLong(pRow->GetValue(elcID)));
			// (z.manning, 11/05/2007) - And then update the value we keep track of in the datalist.
			pRow->PutValue(elcInactive, _variant_t(nNewInactiveValue,VT_BOOL));

			// (z.manning, 11/07/2007) - Need to recalculate the totals.
			OnRequeryFinishedOpportunityList(0);
		}

		//Cleanup
		mnu.DestroyMenu();

	}NxCatchAll("COpportunityEditDlg::OnRButtonDownOpportunityList");
}

// (d.lange 2010-11-11 12:14) - PLID 41441 - Added New Sale filter
void COpportunityListDlg::OnBnClickedShowNewsale()
{
	try {
		ReloadFromFilters();
	} NxCatchAll("COpportunityListDlg::OnBnClickedShowNewsale");
}

// (d.lange 2010-11-11 13:52) - PLID 41441 - Added AddOn filter
void COpportunityListDlg::OnBnClickedShowAddon()
{
	try {
		ReloadFromFilters();
	} NxCatchAll("COpportunityListDlg::OnBnClickedShowAddon");
}

// (d.lange 2010-12-01 09:53) - PLID  41336 - Color the text on specific rows
void COpportunityListDlg::SetRowColors()
{
	try {
		for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->FindAbsoluteFirstRow(VARIANT_TRUE); pRow != NULL; pRow = m_pList->FindAbsoluteNextRow(pRow, VARIANT_TRUE)) {
			long nTaskID = VarLong(pRow->GetValue(elcNextFollowUpID), -1);
			if(nTaskID != -1) {
				//Compare the deadline date with current date, if past due then color text red
				COleDateTime dtCurrentDate = COleDateTime::GetCurrentTime();
				
				COleDateTime dtFollowUpDate;
				dtFollowUpDate.ParseDateTime(VarString(pRow->GetValue(elcNextFollowUp)), VAR_DATEVALUEONLY);

				if(dtCurrentDate >= dtFollowUpDate) {
					pRow->PutCellForeColor(elcNextFollowUp, RGB(255, 0, 0));
				}
			}else {
				//There is not a Follow up date, color the text blue
				pRow->PutCellForeColor(elcNextFollowUp, RGB(0, 0, 255));
			}

			// (d.lange 2010-12-01 10:17) - PLID 41358 - Color some specific competition
			CString strCompetition = VarString(pRow->GetValue(elcCompetition), "");
			if(strCompetition.CompareNoCase("Patient Now") == 0) {
				pRow->PutCellForeColor(elcCompetition, RGB(106, 90, 205));
			}else if(strCompetition.CompareNoCase("Encite") == 0) {
				pRow->PutCellForeColor(elcCompetition, RGB(139, 69, 19));
			}else if(strCompetition.CompareNoCase("RSI") == 0) {
				pRow->PutCellForeColor(elcCompetition, RGB(0, 0, 255));
			}
		}
	} NxCatchAll("COpportunityListdlg::EnsureRowColors");
}