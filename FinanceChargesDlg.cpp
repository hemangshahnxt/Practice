// FinanceChargesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FinanceChargesDlg.h"
#include "GlobalFinancialUtils.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "GlobalReportUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "PatientInterestCalculationDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#define NXM_FINANCE_CHARGES	33485

struct FCCharge {
	long nOriginalChargeID;
	long nProviderID;
	COleDateTime dtAssignmentDate;
	COleCurrency cyFCAmount;
	// (j.jones 2007-09-06 09:50) - PLID 27307 - added flag for whether this is an initial finance charge, or a compounded one
	BOOL bIsInitialFC;
};

struct FCBill {
	long nBillID;
	long nPatientID;
	long nLocationID;
	CPtrArray pChargeAry;
};

/////////////////////////////////////////////////////////////////////////////
// CFinanceChargesDlg dialog


CFinanceChargesDlg::CFinanceChargesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFinanceChargesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFinanceChargesDlg)

	//}}AFX_DATA_INIT
}


void CFinanceChargesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFinanceChargesDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_PROGRESS_BAR_STATUS, m_nxeditProgressBarStatus);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFinanceChargesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CFinanceChargesDlg)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFinanceChargesDlg message handlers

BOOL CFinanceChargesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	SetTimer(NXM_FINANCE_CHARGES, 500, NULL);

	PeekAndPump();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFinanceChargesDlg::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent == NXM_FINANCE_CHARGES) {

		//process the finance charges

		KillTimer(NXM_FINANCE_CHARGES);

		try {

			SetDlgItemText(IDC_PROGRESS_BAR_STATUS,"Checking for overdue charges...");

			PeekAndPump();

			// (j.jones 2009-06-10 11:19) - PLID 33834 - added caching
			g_propManager.CachePropertiesInBulk("CFinanceChargesDlg-1", propNumber,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'FCChargeDaysUntilOverdue' OR "
				"Name = 'FCChargeInterestInterval' OR "
				"Name = 'FCChargeInterestFeeType' OR "
				"Name = 'FCChargesToCalculate' OR "
				"Name = 'FCIncrementExistingFinanceCharges' OR "
				"Name = 'FCCompoundExistingFinanceCharges' OR "
				"Name = 'FCChargeInterestFlatFeeApplyType' OR "
				// (j.jones 2009-06-11 16:04) - PLID 34577 - this is an existing preference
				// that is now used within this dialog
				"Name = 'FCChooseInvididualPatients' "
				")",
				_Q(GetCurrentUserName()));

			g_propManager.CachePropertiesInBulk("CFinanceChargesDlg-2", propText,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'FCChargeInterestRate' OR "
				"Name = 'FCChargeFlatFee' "
				")",
				_Q(GetCurrentUserName()));

			//TODO: in the future when there are more records in AdministrativeFeesT,
			//and there are more than one entry with a Type of 1, we should prompt for the charge to use,
			//and also have the ability to set a default charge to use
			long nFinanceChargeServiceID = -1;
			_RecordsetPtr rs = CreateRecordset("SELECT TOP 1 ID FROM AdministrativeFeesT WHERE Type = 1");
			if(!rs->eof) {
				nFinanceChargeServiceID = AdoFldLong(rs, "ID");
			}
			else {
				// (j.jones 2007-07-02 10:19) - PLID 26513 - converted into a SQL batch
				CString strSqlBatch = BeginSqlBatch();
				AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nServiceID INT");
				AddStatementToSqlBatch(strSqlBatch, "SET @nServiceID = (SELECT Coalesce(Max(ID),0) + 1 AS NewID FROM ServiceT)");
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ServiceT (ID, Name, Price, Taxable1, Taxable2, Active) VALUES "
					"(@nServiceID, 'Finance Charge', 0, 0, 0, 1)");
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO AdministrativeFeesT (ID, Type) VALUES (@nServiceID, 1)");
				// (j.gruber 2012-12-05 11:42) - PLID 48566 - ServiceLocationInfoT
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ServiceLocationInfoT (ServiceID, LocationID) \r\n"
							"SELECT @nServiceID, ID FROM LocationsT WHERE Managed = 1 ");	

				
				CString strFinalRecordset;
				strFinalRecordset.Format(
						"SET NOCOUNT ON \r\n"
						"BEGIN TRAN \r\n"
						"%s "
						"COMMIT TRAN \r\n"
						"SET NOCOUNT OFF \r\n"
						"SELECT @nServiceID AS ServiceID ",
						strSqlBatch);

				// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
				NxAdo::PushPerformanceWarningLimit ppw(-1);
				_RecordsetPtr prsResults = CreateRecordsetStd(strFinalRecordset,
					adOpenForwardOnly, adLockReadOnly, adCmdText, adUseClient);
				if(!prsResults->eof) {
					nFinanceChargeServiceID = AdoFldLong(prsResults, "ServiceID",-1);
				}
				prsResults->Close();
			}
			rs->Close();

			CWaitCursor pWait;

			//load the finance charge settings

			long nDaysUntilOverdue = GetRemotePropertyInt("FCChargeDaysUntilOverdue",30,0,"<None>",true);

			long nChargeInterestDayInterval = GetRemotePropertyInt("FCChargeInterestInterval",30,0,"<None>",true);

			long nFeeType = GetRemotePropertyInt("FCChargeInterestFeeType",0,0,"<None>",true);
			//0 - Use Percentage, 1 - Use Flat Fee

			// (j.jones 2009-06-10 11:21) - PLID 33834 - added flat fee apply types
			long nFlatFeeApplyType = GetRemotePropertyInt("FCChargeInterestFlatFeeApplyType",0,0,"<None>",true);

			CString strInterestRate = GetRemotePropertyText("FCChargeInterestRate","1.0",0,"<None>",true);
			double dblInterestRate = atof(strInterestRate);
			dblInterestRate -= 1.0;

			CString strFlatFee = GetRemotePropertyText("FCChargeFlatFee","$0.00",0,"<None>",true);
			COleCurrency cyFlatFee;
			cyFlatFee.ParseCurrency(strFlatFee);

			long nChargeType = GetRemotePropertyInt("FCChargesToCalculate",0,0,"<None>",true);
			//0 - the patient responsibility of any charge, 1 - only charges that are 100% patient resp

			BOOL bIncrementExistingFinanceCharges = GetRemotePropertyInt("FCIncrementExistingFinanceCharges",0,0,"<None>",true) == 1;

			BOOL bCompoundExistingFinanceCharges = GetRemotePropertyInt("FCCompoundExistingFinanceCharges",0,0,"<None>",true) == 1;

			PeekAndPump();

			//get the recordset of charges that need finance charges added, per our settings
			rs = GetOverdueChargeRecordset(nChargeType, nDaysUntilOverdue, nChargeInterestDayInterval, bCompoundExistingFinanceCharges);

			m_progress.SetRange(0,(short)rs->GetRecordCount());
			m_progress.SetStep(1);

			SetDlgItemText(IDC_PROGRESS_BAR_STATUS,"Calculating finance charges...");

			PeekAndPump();

			CPtrArray aryFinanceCharges;

			//calculate all finance charges

			while(!rs->eof) {

				//calculate what the finance charge will be, and store all appropriate data

				long nChargeID = AdoFldLong(rs, "ChargeID");
				long nBillID = AdoFldLong(rs, "BillID");
				long nPatientID = AdoFldLong(rs, "PatientID");
				long nLocationID = AdoFldLong(rs, "LocationID");
				long nProviderID = AdoFldLong(rs, "ProviderID", -1); // (c.haag 2016-05-02 10:00) - NX-100403 - This column is nullable now
				COleCurrency cyBalance = AdoFldCurrency(rs, "Balance");
				COleDateTime dtAssignmentDate = AdoFldDateTime(rs, "AssignmentDate");
				// (j.jones 2007-09-06 09:50) - PLID 27307 - added flag for whether this is an initial finance charge, or a compounded one
				BOOL bIsInitialFC = AdoFldLong(rs, "IsInitialFC", 0) == 1;

				//calculate the finance charge
				COleCurrency cyFinanceCharge;
				if(nFeeType == 0) {
					//the finance charge is the interest rate times the patient balance
					//use the CalculateAmtQuantity function to correctly calculate a currency and a double
					cyFinanceCharge = CalculateAmtQuantity(cyBalance,dblInterestRate);
					RoundCurrency(cyFinanceCharge);
				}
				else {
					//the finance charge is a flat fee
					cyFinanceCharge = cyFlatFee;
				}

				//store this information in the array
				BOOL bAdded = FALSE;
				BOOL bPatientExists = FALSE;
				for(int i=0;i<aryFinanceCharges.GetSize() && !bAdded;i++) {
					FCBill *pBill = (FCBill*)aryFinanceCharges.GetAt(i);

					// (j.jones 2009-06-10 11:44) - PLID 33834 - We normally add each charge on a bill
					// to the bill's pChargeAry, because in the processing portion of this code
					// we will add the finance charge to this bill ID, and have it reference each
					// charge in pChargeAry.
					// However, if we are applying a flat fee per patient, then we only want to add
					// the flat fee to one bill on the patient's account. If this is the case,
					// simply add all charges (they may be from different bills) to the same bill here,
					// because in the end only one finance charge will be created in LineItemT, but
					// a FinanceChargesT entry will be created for all charges that the flat fee represents.					
					if(pBill->nBillID == nBillID						
						|| (pBill->nPatientID == nPatientID
							&& nFeeType == 1 && nFlatFeeApplyType == 1)
						) {
						//the bill exists in the array, so add a new charge
						FCCharge *pCharge = new FCCharge;
						pCharge->nOriginalChargeID = nChargeID;
						pCharge->nProviderID = nProviderID;
						pCharge->dtAssignmentDate = dtAssignmentDate;						
						pCharge->cyFCAmount = cyFinanceCharge;
						// (j.jones 2007-09-06 09:50) - PLID 27307 - added flag for whether this is an initial finance charge, or a compounded one
						pCharge->bIsInitialFC = bIsInitialFC;
						
						pBill->pChargeAry.Add(pCharge);
						//set bAdded to TRUE to indicate we're done with this record
						bAdded = TRUE;
					}
				}

				if(!bAdded) {
					//we couldn't find the bill so add both that record and the charge record
					FCBill *pBill = new FCBill;
					pBill->nBillID = nBillID;
					pBill->nPatientID = nPatientID;
					pBill->nLocationID = nLocationID;

					FCCharge *pCharge = new FCCharge;
					pCharge->nOriginalChargeID = nChargeID;
					pCharge->nProviderID = nProviderID;
					pCharge->dtAssignmentDate = dtAssignmentDate;
					pCharge->cyFCAmount = cyFinanceCharge;
					// (j.jones 2007-09-06 09:50) - PLID 27307 - added flag for whether this is an initial finance charge, or a compounded one
					pCharge->bIsInitialFC = bIsInitialFC;

					pBill->pChargeAry.Add(pCharge);

					aryFinanceCharges.Add(pBill);

					bAdded = TRUE;
				}

				m_progress.StepIt();

				PeekAndPump();

				rs->MoveNext();
			}
			rs->Close();

			// (j.jones 2009-06-11 16:18) - PLID 34577 - the patient selection ability now occurs
			// after we have already calculated which patients need finance charges
			if(aryFinanceCharges.GetSize() > 0
				&& GetRemotePropertyInt("FCChooseInvididualPatients",0,0,"<None>",true) == 1) {

				CPatientInterestCalculationDlg dlg(this);

				//populate the dialog with the patients who require finance charges
				for(int i=0; i<aryFinanceCharges.GetSize(); i++) {

					FCBill *pBill = (FCBill*)aryFinanceCharges.GetAt(i);
					//add to the dialog's IN clause, no need to check for duplicates
					if(!dlg.m_strPatientIDs.IsEmpty()) {
						dlg.m_strPatientIDs += ",";
					}
					dlg.m_strPatientIDs += AsString((long)pBill->nPatientID);
				}

				if(IDOK == dlg.DoModal()) {

					//check m_aryPatientIDsToSkip, and remove all the patients in this array
					//from our finance charge list
					for(int i=0; i<dlg.m_aryPatientIDsToSkip.GetSize(); i++) {
						long nPatientIDToSkip = dlg.m_aryPatientIDsToSkip.GetAt(i);

						for(int j=aryFinanceCharges.GetSize()-1; j>=0; j--) {
							FCBill *pBill = (FCBill*)aryFinanceCharges.GetAt(j);
							if(pBill->nPatientID == nPatientIDToSkip) {								
								//this finance charge is for a skipped patient, so drop it
								for(int k=pBill->pChargeAry.GetSize()-1;k>=0;k--) {
									delete pBill->pChargeAry.GetAt(k);
								}
								pBill->pChargeAry.RemoveAll();
								delete pBill;
								aryFinanceCharges.RemoveAt(j);
							}
						}
					}
				}
				else {
					//they cancelled, and were already warned about it, so leave silently

					//delete our arrays
					for(int i=aryFinanceCharges.GetSize()-1;i>=0;i--) {
						FCBill *pBill = (FCBill*)aryFinanceCharges.GetAt(i);
						for(int j=pBill->pChargeAry.GetSize()-1;j>=0;j--) {
							delete pBill->pChargeAry.GetAt(j);
						}
						pBill->pChargeAry.RemoveAll();
						delete pBill;
					}
					aryFinanceCharges.RemoveAll();

					EndDialog(IDOK);
					return;
				}
			}

			m_progress.SetRange(0,(short)aryFinanceCharges.GetSize());
			m_progress.SetStep(1);

			SetDlgItemText(IDC_PROGRESS_BAR_STATUS,"Creating finance charges...");

			PeekAndPump();

			//now create the finance charges

			CString strIDs;

			// (j.jones 2009-06-10 13:46) - PLID 34583 - we now track two metrics now for our success message,
			// how many charges were created or updated, and how many entries in FinanceChargesT we created
			long nChargesT_RecordsAffected = 0;
			long nFinanceChargesT_RecordsAffected = 0;

			// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - for() loops
			int i = 0;

			long nFinanceChargeHistoryID = -1;

			for(i=0;i<aryFinanceCharges.GetSize();i++) {

				//now create the finance charge

				//we will add up all the finance charges to be added per bill, and add one charge per bill
				//however we will create a FinanceChargesT record for each charge in a bill that is having an FC calculated

				FCBill *pBill = (FCBill*)aryFinanceCharges.GetAt(i);

				COleCurrency cyFCTotal = COleCurrency(0,0);

				//add up the total finance charges to be added to this bill

				if(nFeeType == 0) { //percentage of charges
					//TODO: consider an option to split by provider, so if there are charges with different providers,
					//the FC could optionally not be added together (for now we use the provider on the first charge)

					for(int j=0;j<pBill->pChargeAry.GetSize();j++) {
						FCCharge *pCharge = (FCCharge*)pBill->pChargeAry.GetAt(j);
						cyFCTotal += pCharge->cyFCAmount;
					}
				}
				else { //flat fee
					cyFCTotal = cyFlatFee;
				}

				//now create (or update) the actual charge

				long nFinanceChargeID = AddFinanceCharge(nFinanceChargeServiceID, bIncrementExistingFinanceCharges, pBill->nPatientID, pBill->nBillID, ((FCCharge*)pBill->pChargeAry.GetAt(0))->nProviderID, pBill->nLocationID, cyFCTotal);

				// (j.jones 2009-06-10 13:46) - PLID 34583 - increment nChargesT_RecordsAffected
				// for each entry we made into ChargesT
				nChargesT_RecordsAffected++;

				//now log the creation in our FinanceChargesT table

				//JMJ - for future usage, we store the date in which the finance charge *should* have been created
				//in the 'Date' field, and of course the date it was really created in the 'InputDate' field
				//we also store the amount of the finance charge

				// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - for() loops
				int j = 0;

				for(j=0;j<pBill->pChargeAry.GetSize();j++) {
					FCCharge *pCharge = (FCCharge*)pBill->pChargeAry.GetAt(j);

					COleDateTime dtOfficial = pCharge->dtAssignmentDate;
					COleDateTimeSpan dtSpan;

					// (j.jones 2007-09-06 10:24) - PLID 27307 - If an initial finance charge, the overdue date
					// is based off of nDaysUntilOverdue. If the charge has already had an FC made or is itself
					// an FC, then the overdue date is based off of nChargeInterestDayInterval
					if(pCharge->bIsInitialFC)
						dtSpan.SetDateTimeSpan(nDaysUntilOverdue,0,0,0);
					else
						dtSpan.SetDateTimeSpan(nChargeInterestDayInterval,0,0,0);

					dtOfficial += dtSpan;

					// (j.jones 2007-09-06 10:26) - PLID 27307 - the date we calculate the charge as having been due on
					// should never be in the future
					// (Note: this will not work if your workstation date <> the server date, but it is
					// not worth comparing to GetDate() here. The actual dtOfficial should never be
					// in the future for the server date though.
					ASSERT(dtOfficial <= COleDateTime::GetCurrentTime());

					// (j.jones 2009-06-10 14:27) - PLID 33834 - if using a flat fee,
					// only track the flat fee in the first finance charge on this bill
					COleCurrency cyFinanceChargeAmt = COleCurrency(0,0);
					if(nFeeType == 0 || j == 0) {
						//if adding as a percentage, or are on the first charge
						//of a flat fee bill, track the finance charge amount
						cyFinanceChargeAmt = pCharge->cyFCAmount;
					}

					// (j.jones 2013-07-31 15:25) - PLID 53317 - Added FinanceChargeHistoryT.
					// If we haven't created an entry yet for this batch of finance charges,
					// do so now.
					if(nFinanceChargeHistoryID == -1) {
						_RecordsetPtr rsNewHistory = CreateParamRecordset("SET NOCOUNT ON; \r\n"
							"INSERT INTO FinanceChargeHistoryT (InputDate) SELECT GetDate(); \r\n"
							"SET NOCOUNT OFF; \r\n"
							"SELECT Convert(int, SCOPE_IDENTITY()) AS NewID;");
						if(!rsNewHistory->eof) {
							nFinanceChargeHistoryID = VarLong(rsNewHistory->Fields->Item["NewID"]->Value);
						}
						else {
							//this should not be possible
							ThrowNxException("Failed to create a new FinanceChargeHistoryT record.");
						}
						rsNewHistory->Close();
					}

					// (j.jones 2013-07-31 15:31) - PLID 53317 - parameterized and added FinanceChargeHistoryID

					// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
					NxAdo::PushPerformanceWarningLimit ppw(-1);
					_RecordsetPtr prsResults = CreateParamRecordset("SET NOCOUNT ON \r\n"
						"BEGIN TRAN \r\n"
						""
						"DECLARE @nFCID INT \r\n"
						"SET @nFCID = (SELECT Coalesce(Max(ID),0) + 1 AS NewID FROM FinanceChargesT WITH(UPDLOCK, HOLDLOCK)) \r\n"
						"INSERT INTO FinanceChargesT (ID, FinanceChargeHistoryID, OriginalChargeID, FinanceChargeID, Amount, Date) "
						"VALUES (@nFCID, {INT}, {INT}, {INT}, {OLECURRENCY}, {OLEDATETIME}) \r\n"
						""
						"COMMIT TRAN \r\n"
						"SET NOCOUNT OFF \r\n"
						""
						"SELECT @nFCID AS FCID",
						nFinanceChargeHistoryID, pCharge->nOriginalChargeID, nFinanceChargeID, cyFinanceChargeAmt, dtOfficial);

					long nFCID = -1;
					if(!prsResults->eof) {
						nFCID = AdoFldLong(prsResults, "FCID",-1);
					}
					prsResults->Close();

					CString str;
					str.Format("%li,",nFCID);
					strIDs += str;

					CString strAuditDesc;
					strAuditDesc.Format("For %s",FormatCurrencyForInterface(pCharge->cyFCAmount,TRUE,TRUE));

					long nAuditID = BeginNewAuditEvent();
					AuditEvent(pBill->nPatientID, GetExistingPatientName(pBill->nPatientID),nAuditID,aeiFCCreate,nFCID,"",strAuditDesc,aepHigh,aetCreated);

					// (j.jones 2009-06-10 13:46) - PLID 34583 - increment nFinanceChargesT_RecordsAffected
					// for each entry we made into FinanceChargesT
					nFinanceChargesT_RecordsAffected++;
				}

				m_progress.StepIt();

				PeekAndPump();
			}

			strIDs.TrimRight(",");

			//delete our arrays
			for(i=aryFinanceCharges.GetSize()-1;i>=0;i--) {
				FCBill *pBill = (FCBill*)aryFinanceCharges.GetAt(i);
				for(int j=pBill->pChargeAry.GetSize()-1;j>=0;j--) {
					delete pBill->pChargeAry.GetAt(j);
				}
				pBill->pChargeAry.RemoveAll();
				delete pBill;
			}
			aryFinanceCharges.RemoveAll();

			if(GetMainFrame())
				GetMainFrame()->UpdateAllViews();

			//store the current date/time as the last time we ran this process
			// (j.jones 2013-07-31 15:14) - PLID 53317 - obsolete, this is now in FinanceChargeHistoryT
			//COleDateTime dtNow = COleDateTime::GetCurrentTime();
			//SetRemotePropertyDateTime("LastFinanceChargeProcess", dtNow, 0, "<None>");

			//if none are created, don't ask to run a report
			if(nFinanceChargesT_RecordsAffected == 0) {

				AfxMessageBox("Finance Charge calculations complete. No finance charges were created.");

				EndDialog(IDOK);

				return;
			}

			// (j.jones 2009-06-10 13:46) - PLID 34583 - should be impossible for
			// nFinanceChargesT_RecordsAffected to be positive and nChargesT_RecordsAffected to be 0
			ASSERT(nChargesT_RecordsAffected > 0);

			CString str;
			// (j.jones 2009-06-10 13:46) - PLID 34583 - rephrase this message to be more accurate
			str.Format("Finance Charge calculations complete.\n\n"
				"%li finance charges were created for %li overdue charge responsibilities.\n\n"
				"Would you like to view a report of these charges now?",
				nChargesT_RecordsAffected, nFinanceChargesT_RecordsAffected);

			if(IDYES == MessageBox(str,"Practice",MB_YESNO|MB_ICONINFORMATION)) {

				//preview the report
				CWaitCursor pWait;
				
				CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(559)]);

				CPrintDialog* dlg;
				dlg = new CPrintDialog(FALSE);
				CPrintInfo prInfo;
				prInfo.m_bPreview = true;
				prInfo.m_bDirect = false;
				prInfo.m_bDocObject = false;
				if (prInfo.m_pPD) delete prInfo.m_pPD;
				prInfo.m_pPD = dlg;

				COleDateTime dtToday, dtNow;
				dtNow = COleDateTime::GetCurrentTime();
				dtToday.SetDate(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay());

				//Set up the parameters.
				CPtrArray paParams;
				CRParameterInfo *paramInfo;
				paramInfo = new CRParameterInfo;
				paramInfo->m_Data = dtToday.Format("%m/%d/%Y");
				paramInfo->m_Name = "DateFrom";
				paParams.Add((void *)paramInfo);

				paramInfo = new CRParameterInfo;
				paramInfo->m_Data = dtToday.Format("%m/%d/%Y");
				paramInfo->m_Name = "DateTo";
				paParams.Add((void *)paramInfo);

				paramInfo = new CRParameterInfo;
				paramInfo->m_Data = "Service Date";
				paramInfo->m_Name = "DateFilter";
				paParams.Add((void *)paramInfo);	

				infReport.nDateFilter = 2;
				infReport.nDateRange = 2;
				infReport.DateFrom = dtToday;
				infReport.DateTo = dtToday;

				infReport.SetExtraValue(strIDs);

				// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
				RunReport(&infReport, &paParams, true, this, "Finance Charges", &prInfo, TRUE);
				ClearRPIParameterList(&paParams);	//DRT - PLID 18085 - Cleanup after ourselves
			}

			EndDialog(IDOK);

		}NxCatchAll("Error processing finance charges.");

		EndDialog(IDCANCEL);
	}
	
	CDialog::OnTimer(nIDEvent);
}

long CFinanceChargesDlg::AddFinanceCharge(long nServiceID, BOOL bIncrementExistingFinanceCharges, long nPatientID, long nBillID, long nProviderID, long nLocationID, COleCurrency cyTotalFCAmount)
{
	long nFinanceChargeID = -1;

	//calculate the existing charge ID, if we need one
	if(bIncrementExistingFinanceCharges) {

		// (j.jones 2011-08-17 10:58) - PLID 44890 - ignore original and void charges
		_RecordsetPtr rsFinChgID = CreateParamRecordset("SELECT Max(ChargesT.ID) AS MaxChargeID "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN FinanceChargesT ON ChargesT.ID = FinanceChargesT.FinanceChargeID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
			"WHERE LineItemT.Deleted = 0 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null " 
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND ChargesT.BillID = {INT}", nBillID);

		if(!rsFinChgID->eof) {
			nFinanceChargeID = AdoFldLong(rsFinChgID, "MaxChargeID",-1);
		}
		rsFinChgID->Close();
	}

	if(nFinanceChargeID == -1) {

		//if there is no existing finance charge, add a new charge

		// (j.jones 2007-07-02 08:56) - PLID 26513 - converted to use a SQL batch
		CString strSqlBatch = BeginSqlBatch();

		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nLineItemID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nLineID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nChargeRespID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nChargeRespDetailID INT");

		// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
		AddStatementToSqlBatch(strSqlBatch, 
			"INSERT INTO LineItemT (\r\n"
			"	PatientID, Type, Amount, Description, Date, InputName, LocationID\r\n"
			") VALUES (\r\n"
			"	%li, 10, Convert(money, '%s'), 'Finance Charge', GetDate(), '%s', %li)",
			nPatientID, FormatCurrencyForSql(cyTotalFCAmount), _Q(GetCurrentUserName()), nLocationID);

		AddStatementToSqlBatch(strSqlBatch, "SET @nLineItemID = SCOPE_IDENTITY() ");

		AddStatementToSqlBatch(strSqlBatch, "SET @nLineID = (SELECT Coalesce(Max(LineID),0) + 1 AS MaxLineID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"WHERE Deleted = 0 AND BillID = %li)",nBillID);
		//DRT 4/10/2006 - PLID 11734 - Removed ProcCode
		// (j.jones 2015-03-18 14:24) - PLID 64974 - Category and SubCategory are now nullable, not 0, and thus do not need to be filled
		// (c.haag 2016-05-02 10:00) - NX-100403 - DoctorsProviders is nullable now
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ChargesT (ID, BillID, ServiceID, ItemCode, TaxRate, Quantity, DoctorsProviders, LineID, TaxRate2, ServiceDateFrom, ServiceDateTo) VALUES "
			"(@nLineItemID, %li, %li, '%s', 1, 1, %s, @nLineID, 1, GetDate(), GetDate())", nBillID, nServiceID, "FC", (nProviderID > 0 ? AsString(nProviderID) : "NULL"));

		// (j.armen 2013-06-28 15:03) - PLID 57383 - Idenitate ChargeRespT
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ChargeRespT (ChargeID, Amount) VALUES (@nLineItemID, Convert(money, '%s'))", FormatCurrencyForSql(cyTotalFCAmount));
		AddStatementToSqlBatch(strSqlBatch, "SET @nChargeRespID = SCOPE_IDENTITY()");

		//create a new ChargeRespDetailT record to store the current assignment date
		// (j.armen 2013-06-28 16:49) - PLID 57384 - Idenitate ChargeRespDetailT
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ChargeRespDetailT (ChargeRespID, Amount, Date) "
			"VALUES (@nChargeRespID, Convert(money, '%s'), GetDate())", 
			FormatCurrencyForSql(cyTotalFCAmount));

		CString strFinalRecordset;
		strFinalRecordset.Format(
				"SET NOCOUNT ON \r\n"
				"BEGIN TRAN \r\n"
				"%s "
				"COMMIT TRAN \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT @nLineItemID AS FinanceChargeID ",
				strSqlBatch);

		// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
		NxAdo::PushPerformanceWarningLimit ppw(-1);
		_RecordsetPtr prsResults = CreateRecordsetStd(strFinalRecordset,
			adOpenForwardOnly, adLockReadOnly, adCmdText, adUseClient);
		if(!prsResults->eof) {
			nFinanceChargeID = AdoFldLong(prsResults, "FinanceChargeID",-1);
		}
		prsResults->Close();
		
		long AuditID = BeginNewAuditEvent();
		CString strAuditDesc;
		strAuditDesc.Format("Charge Added: %s Finance Charge",FormatCurrencyForInterface(cyTotalFCAmount,TRUE,TRUE));
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID),AuditID,aeiBillChargeAdded,nFinanceChargeID,"",_Q(strAuditDesc),aepHigh,aetCreated);
	}
	else {

		//otherwise, update the existing finance charge and add in the new finance charge amount

		// (j.jones 2007-07-02 08:56) - PLID 26513 - converted to use a SQL batch
		CString strSqlBatch = BeginSqlBatch();

		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nChargeRespID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nChargeRespDetailID INT");

		AddStatementToSqlBatch(strSqlBatch, "UPDATE LineItemT SET Amount = Amount + Convert(money,'%s') WHERE ID = %li", FormatCurrencyForSql(cyTotalFCAmount), nFinanceChargeID);

		AddStatementToSqlBatch(strSqlBatch, "SET @nChargeRespID = (SELECT ID FROM ChargeRespT WHERE ChargeID = %li AND InsuredPartyID Is Null)", nFinanceChargeID);
		AddDeclarationToSqlBatch(strSqlBatch, "IF @nChargeRespID <> -1");
		AddDeclarationToSqlBatch(strSqlBatch, "BEGIN");
		// (j.jones 2008-02-06 12:58) - PLID 28833 - fixed a bug where charge resps were not updating properly with @nChargeRespID
		AddStatementToSqlBatch(strSqlBatch, "UPDATE ChargeRespT SET Amount = Amount + Convert(money,'%s') WHERE ID = @nChargeRespID", FormatCurrencyForSql(cyTotalFCAmount));
		AddDeclarationToSqlBatch(strSqlBatch, "END"); 
		
		//if no patient charge resp. was found, then no patient resp. exists for the existing finance charge
		AddDeclarationToSqlBatch(strSqlBatch, "ELSE");
		AddDeclarationToSqlBatch(strSqlBatch, "BEGIN");
		// (j.armen 2013-06-28 15:03) - PLID 57383 - Idenitate ChargeRespT
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ChargeRespT (ChargeID, Amount) VALUES (%li, Convert(money, '%s'))", nFinanceChargeID, FormatCurrencyForSql(cyTotalFCAmount));
		AddStatementToSqlBatch(strSqlBatch, "SET @nChargeRespID = SCOPE_IDENTITY()");
		AddDeclarationToSqlBatch(strSqlBatch, "END"); 

		//create a new ChargeRespDetailT record to store the current assignment date
		// (j.armen 2013-06-28 16:49) - PLID 57384 - Idenitate ChargeRespDetailT
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ChargeRespDetailT (ChargeRespID, Amount, Date) "
			"VALUES (@nChargeRespID, Convert(money, '%s'), GetDate())", 
			FormatCurrencyForSql(cyTotalFCAmount));

		//throw an exception if any ChargeRespT record for this charge mismatches the ChargeRespDetailT total underneath it
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ChargeRespTotalToCheck money");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ChargeRespIDToCheck INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ChargeRespInsuredPartyIDToCheck INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ChargeRespAmountToCheck money");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @ChargeRespDetailTotalToCheck money");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @errorText nvarchar(max)");
		AddStatementToSqlBatch(strSqlBatch, "SELECT "
			"@ChargeRespIDToCheck = ChargeRespT.ID, "
			"@ChargeRespInsuredPartyIDToCheck = ChargeRespT.InsuredPartyID, "
			"@ChargeRespAmountToCheck = ChargeRespT.Amount, "
			"@ChargeRespDetailTotalToCheck = Sum(IsNull(ChargeRespDetailT.Amount, Convert(money,0))) "
			"FROM ChargeRespT "
			"LEFT JOIN ChargeRespDetailT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID "
			"WHERE ChargeRespT.ChargeID = %li "
			"GROUP BY ChargeRespT.ID, ChargeRespT.InsuredPartyID, ChargeRespT.Amount "
			"HAVING ChargeRespT.Amount <> Sum(IsNull(ChargeRespDetailT.Amount, Convert(money, 0)))", nFinanceChargeID);
		AddStatementToSqlBatch(strSqlBatch, "IF @ChargeRespIDToCheck Is Not Null \r\n"
			"BEGIN \r\n"
			"SET @errorText = 'The finance charge ID ' + Convert(nvarchar, %li) + ' cannot be saved because its ChargeRespDetailT responsibilites do not match the ChargeRespT responsibility total.\n\n"
			"ChargeRespT.ID = ' + Convert(nvarchar, @ChargeRespIDToCheck) + ', ChargeRespT.InsuredPartyID = ' + Convert(nvarchar, IsNull(@ChargeRespInsuredPartyIDToCheck, -1)) + ', "
			"ChargeRespT.Amount = ' + Convert(nvarchar, @ChargeRespAmountToCheck) + ', ChargeRespDetailT Total = ' + Convert(nvarchar, @ChargeRespDetailTotalToCheck); \r\n"
			"RAISERROR(@errorText, 16, 1) ROLLBACK TRAN RETURN \r\n"
			"END", nFinanceChargeID);

		ExecuteSqlBatch(strSqlBatch);

		long AuditID = BeginNewAuditEvent();
		CString strAuditDesc;
		strAuditDesc.Format("%s Finance Charge Added",FormatCurrencyForInterface(cyTotalFCAmount,TRUE,TRUE));
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID),AuditID,aeiChargeLineAmount,nFinanceChargeID,"",strAuditDesc,aepHigh);
	}

	return nFinanceChargeID;
}

//DRT 4/10/2007 - PLID 25564 - Removed PeekAndPump in favor of a global version.

_RecordsetPtr CFinanceChargesDlg::GetOverdueChargeRecordset(long nChargeType, long nDaysUntilOverdue, long nChargeInterestDayInterval, BOOL bCompoundExistingFinanceCharges)
{
	CWaitCursor pWait;

	CString strSql;

	CString strWhichDates = "", strFC = "";
	

	// (j.jones 2011-01-05 09:20) - PLID 41988 - removed IN clauses

	//if nChargeType = 0 then we calculate based on the patient responsibility of any charge
	//if nChargeType = 1 then we only calculate charges that are 100% patient resp
	CString strWhere = "";
	if(nChargeType == 1) {
		strWhere += "AND InsuranceRespQ.ChargeID Is Null ";
	}

	if(!bCompoundExistingFinanceCharges) {
		//disallow finance charges to be calculated on existing finance charges
		strWhere += "AND FinanceChargesQ.FinanceChargeID Is Null ";
	}

	if(nChargeInterestDayInterval == 0) {
		//this will filter on original, non-finance-charges that have not had finance charges yet
		strWhere += "AND OriginalChargesQ.OriginalChargeID Is Null ";
	}
	else {
		//this will filter on charges that have not had finance charges yet, or original charges or
		//finance charges that have not themselves had finance charges in the past nChargeInterestDayInterval days

		// (j.gruber 2007-08-13 10:19) - PLID 26969 - ANDed these together instead of ORed if there is compounded interest so we aren't constantly calculating finance charges on original charges
		// (j.jones 2008-02-06 11:49) - PLID 28828 - unfortunately with that change, original charges never had interest calculated more than once, ever
		CString str;
		if(bCompoundExistingFinanceCharges) {

			//in addition to running finance charges on those charges that have no finance charges after nDaysUntilOverdue,
			//we must also run them on finance charges that are themselves past the nChargeInterestDayInterval
			strFC.Format(" OR (FinanceChargeID Is Not Null AND FinanceChargeMaxInputDate <= DateAdd(day,-%li,GetDate())) "
				//we must also run them on charges that have finance charges that are past the nChargeInterestDayInterval
				" OR (OriginalChargeID Is Not Null AND OriginalChargeMaxInputDate <= DateAdd(day,-%li,GetDate())) ", nChargeInterestDayInterval, nChargeInterestDayInterval);

			str.Format("AND ("
				//find charges that have never had finance charges created
				"OriginalChargesQ.OriginalChargeID Is Null "
				"OR "
				//find charges that have had finance charges created,
				//but the latest one created for it is over nChargeInterestDayInterval days old
				"(OriginalChargesQ.OriginalChargeID Is Not Null AND OriginalChargesQ.MaxInputDate <= DateAdd(day,-%li,GetDate())) "
				"OR "
				//find charges that are themselves finance charges, that have never been compounded,
				//and are over nChargeInterestDayInterval days old
				"(FinanceChargesQ.FinanceChargeID Is Not Null AND OriginalChargesQ.OriginalChargeID Is Null "
				"AND FinanceChargesQ.MaxInputDate <= DateAdd(day,-%li,GetDate())) "
				"OR "
				//find charges that are themselves finance charges, that have been compounded before,
				//but the latest one created for it is over nChargeInterestDayInterval days old
				"(FinanceChargesQ.FinanceChargeID Is Not Null AND OriginalChargesQ.OriginalChargeID Is Not Null "
				"AND OriginalChargesQ.MaxInputDate <= DateAdd(day,-%li,GetDate()) AND FinanceChargesQ.MaxInputDate <= DateAdd(day,-%li,GetDate())) "
				") ", nChargeInterestDayInterval, nChargeInterestDayInterval, nChargeInterestDayInterval, nChargeInterestDayInterval);
		}
		else {
			str.Format("AND ("
				//find charges that have never had finance charges created
				"OriginalChargesQ.OriginalChargeID Is Null "
				"OR "
				//find charges that have had finance charges created,
				//but the latest one created for it is over nChargeInterestDayInterval days old
				"(OriginalChargesQ.OriginalChargeID Is Not Null AND OriginalChargesQ.MaxInputDate <= DateAdd(day,-%li,GetDate()))) ", nChargeInterestDayInterval);

			//in addition to running finance charges on those charges that have no finance charges after nDaysUntilOverdue,
			//we must also run them on charges that have finance charges that are past the nChargeInterestDayInterval
			strFC.Format(" OR (OriginalChargeID Is Not Null AND OriginalChargeMaxInputDate <= DateAdd(day,-%li,GetDate())) ", nChargeInterestDayInterval);
		}
		strWhere += str;
	}
	
	//this will filter on charges that are overdue by the given overdue days
	strWhichDates.Format("(AssignmentDate < DateAdd(day,-%li,GetDate()) %s )", nDaysUntilOverdue, strFC);

	// (j.jones 2009-06-10 11:43) - PLID 33834 - sorted by Bill date
	// (j.jones 2011-01-05 09:20) - PLID 41988 - removed IN clauses in favor of LEFT JOINed queries
	// (j.jones 2011-08-17 10:58) - PLID 44890 - ignore original and void line items
	strSql.Format("SELECT Total - ApplyAmount AS Balance, BillID, ChargeID, NeedFinanceChargeQ.PatientID, NeedFinanceChargeQ.LocationID, ProviderID, AssignmentDate, "
		// (j.jones 2007-09-06 10:13) - PLID 27307 - added ability to track whether this is the first time the charge is overdue
		// or is it being compounded, by checking whether the charge ID is in FinanceChargesT as an "original charge" or is itself
		// a finance charge
		"CASE WHEN OriginalChargeID Is Not Null OR FinanceChargeID Is Not Null THEN 0 ELSE 1 END AS IsInitialFC "
		"FROM ("
		"	SELECT Sum(ChargeRespDetailT.Amount) AS Total, "
		"	Coalesce(TotalAppliesQ.TotalApplyAmount,0) AS ApplyAmount, "
		"	ChargesT.BillID, ChargesT.ID AS ChargeID, LineItemT.PatientID, LineItemT.LocationID, "
		"	ChargesT.DoctorsProviders AS ProviderID, "

		// (j.jones 2008-02-06 12:02) - PLID 28828 - if this will be the first finance charge, use
		// the main charge's charge resp date as the assignment date, otherwise we want the most recent
		// charge resp date for a finance charge created on this charge
		"	CASE WHEN OriginalChargesQ.MaxInputDate Is Not Null THEN OriginalChargesQ.MaxInputDate ELSE ChargeRespDetailT.Date END AS AssignmentDate, "

		"	OriginalChargesQ.OriginalChargeID, OriginalChargesQ.MaxInputDate AS OriginalChargeMaxInputDate, "
		"	FinanceChargesQ.FinanceChargeID, FinanceChargesQ.MaxInputDate AS FinanceChargeMaxInputDate "

		"	FROM ChargesT "		
		"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
		"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
		"	INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
		"	INNER JOIN ChargeRespDetailT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID "
		"	LEFT JOIN ("
		"		SELECT Sum(ApplyDetailsT.Amount) AS TotalApplyAmount, ApplyDetailsT.DetailID "
		"		FROM AppliesT "
		"		INNER JOIN ApplyDetailsT ON AppliesT.ID = ApplyDetailsT.ApplyID "
		"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalAppliesQ ON AppliesT.SourceID = LineItemCorrections_OriginalAppliesQ.OriginalLineItemID " 
		"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingAppliesQ ON AppliesT.SourceID = LineItemCorrections_VoidingAppliesQ.VoidingLineItemID " 
		"		WHERE LineItemCorrections_OriginalAppliesQ.OriginalLineItemID Is Null " 
		"		AND LineItemCorrections_VoidingAppliesQ.VoidingLineItemID Is Null " 
		"		GROUP BY ApplyDetailsT.DetailID "
		"	) AS TotalAppliesQ ON ChargeRespDetailT.ID = TotalAppliesQ.DetailID "
		""
		"	LEFT JOIN ("
		"		SELECT OriginalChargeID, Max(FinanceChargesT.InputDate) AS MaxInputDate "
		"		FROM FinanceChargesT "
		"		INNER JOIN LineItemT ON FinanceChargesT.FinanceChargeID = LineItemT.ID "
		"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
		"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
		"		WHERE LineItemT.Deleted = 0 "
		"		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null " 
		"		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null " 
		"		GROUP BY OriginalChargeID "
		"	) AS OriginalChargesQ ON ChargesT.ID = OriginalChargesQ.OriginalChargeID "
		""
		"	LEFT JOIN ("
		"		SELECT FinanceChargeID, Max(FinanceChargesT.InputDate) AS MaxInputDate "
		"		FROM FinanceChargesT "
		"		INNER JOIN LineItemT ON FinanceChargesT.FinanceChargeID = LineItemT.ID "
		"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
		"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
		"		WHERE LineItemT.Deleted = 0 "
		"		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null " 
		"		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null " 
		"		GROUP BY FinanceChargeID "
		"	) AS FinanceChargesQ ON ChargesT.ID = FinanceChargesQ.FinanceChargeID "

		// (j.jones 2006-09-19 13:36) - PLID 22076 - filter on charges with a balance
		"	LEFT JOIN ("
		"		SELECT Sum(AppliesT.Amount) AS Amt, DestID "
		"		FROM AppliesT "
		"		INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
		"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalAppliesQ ON LineItemT.ID = LineItemCorrections_OriginalAppliesQ.OriginalLineItemID " 
		"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingAppliesQ ON LineItemT.ID = LineItemCorrections_VoidingAppliesQ.VoidingLineItemID " 
		"		WHERE LineItemT.Deleted = 0 "
		"		AND LineItemCorrections_OriginalAppliesQ.OriginalLineItemID Is Null " 
		"		AND LineItemCorrections_VoidingAppliesQ.VoidingLineItemID Is Null " 
		"		GROUP BY DestID "
		"	) AS AppliesQ ON ChargesT.ID = AppliesQ.DestID "
		""
		// (j.jones 2011-01-05 11:27) - PLID 41988 - find if there is any insurance resp
		"	LEFT JOIN ("
		"		SELECT ChargeID "
		"		FROM ChargeRespT "
		"		INNER JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID "
		"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
		"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
		"		WHERE LineItemT.Deleted = 0 "
		"		AND ChargeRespT.InsuredPartyID Is Not Null AND ChargeRespT.Amount > Convert(money,0) "
		"		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null " 
		"		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null " 
		"		GROUP BY ChargeRespT.ChargeID "
		"	) AS InsuranceRespQ ON ChargesT.ID = InsuranceRespQ.ChargeID "
		""
		"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
		"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null " 
		"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null " 
		"	AND ChargeRespT.InsuredPartyID Is Null "
		"	AND (AppliesQ.Amt Is Null OR AppliesQ.Amt < dbo.GetChargeTotal(ChargesT.ID)) "
		""
		//and now use our customized where filter
		"	%s "
		""
		"	GROUP BY TotalAppliesQ.TotalApplyAmount, ChargesT.BillID, ChargesT.ID, LineItemT.PatientID, ChargeRespDetailT.ID, "
		"	LineItemT.LocationID, ChargesT.DoctorsProviders, ChargeRespDetailT.Date, LineItemT.Deleted, LineItemT.Type, "
		"	ChargeRespT.ChargeID, OriginalChargesQ.OriginalChargeID, OriginalChargesQ.MaxInputDate, "
		"	FinanceChargesQ.FinanceChargeID, FinanceChargesQ.MaxInputDate "
		""
		") AS NeedFinanceChargeQ "
		""
		"INNER JOIN BillsT ON NeedFinanceChargeQ.BillID = BillsT.ID "
		""
		"WHERE Total - ApplyAmount > Convert(money,0) "
		"AND %s "
		"ORDER BY NeedFinanceChargeQ.PatientID, BillsT.Date, BillID, ProviderID, ChargeID", strWhere, strWhichDates);

	CIncreaseCommandTimeout ict(600);

#ifdef _DEBUG
	MsgBox(strSql);
#endif

	_RecordsetPtr rs = CreateRecordset(strSql);

	return rs;
}
