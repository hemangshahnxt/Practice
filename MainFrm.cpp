// MainFrm.cpp : implementation of the CMainFrame class


/****************************************************************


					NexTech Practice 
					mainfrm.cpp

			Copyright NexTech Systems LLC
			    All hail the hypnotoad


/***************************************************************/


#include "stdafx.h"
#include "Practice.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "ClosingDlg.h"
#include "DocBar.h"
#include "FilterEditDlg.h"
#include "GlobalUtils.h"
#include "InformLink.h"
#include "UnitedLink.h"
#include "MarketUtils.h"
#include "MirrorLink.h"
#include "NewContact.h"
#include "NewPatient.h"
#include "NxStandard.h"
#include "NxView.h"
#include "PracProps.h"
#include "PracticeDoc.h"
#include "ReportView.h"
#include "NxSchedulerDlg.h"
#include "MonthDlg.h"
#include "PatientsRc.h"
#include "ReportsRc.h"
#include "StatementSetup.h"
#include "superbilldlg.h"
#include "ChangeTypeDlg.h"
#include "AdminView.h"
#include "resentrydlg.h"
#include "windowsx.h"
#include "GlobalFinancialUtils.h"
#include "GlobalDataUtils.h"
#include "Client.h"
#include "Barcode.h"
#include "CallerID.h"
#include "UserPropsDlg.h"
#include "MessagerDlg.h"
#include "AutoCallerDlg.h"
#include "AuditTrail.h"
#include "LetterView.h"
#include "MoveUpListDlg.h"
#include "SchedulerRc.h"
#include "EyeGraphDlg.h"
#include "FilterFieldInfo.h"
#include "PatientView.h"
#include "UpdatePastPendingApptsDlg.h"
#include "WhatsNewHTMLDlg.h" 
#include "UpdatePendingDlg.h"
#include "UpdatePendingConfigDlg.h"
#include "UpdatingPendingDlg.h"
#include "RegUtils.h"
#include "nxtwain.h"
#include "EditNoCatReportsDlg.h"
#include "nxmessagedef.h"
#include "CopyPermissionsDlg.h"
#include "MediNotesLink.h"
#include "PreferenceUtils.h"
#include "SearchNotesDlg.h"
#include "LogManagementDlg.h"
#include "AppliedSuperbillsDlg.h"
#include "VoidSuperbillDlg.h"
#include "Mirror.h"
#include "HistoryFilterDlg.h"
#include "ReportInfo.h"
#include "GlobalReportUtils.h"
#include "Groups.h"
#include "SelectUserDlg.h"
#include "UpdatePrefixesDlg.h"
#include "QuickbooksLink.h"
#include "InvSerialTrackerDlg.h"
#include "UpdatePriceDlg.h"
#include "AptBookAlarmListDlg.h"
#include "ChangePasswordDlg.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "LetterWriting.h"
#include "MergeEngine.h"
#include "InternationalUtils.h"
#include "PPCLink.h"
#include "Filter.h"
#include "IDPASetupDlg.h"
#include "DateTimeUtils.h"
#include "TransferTodosDlg.h"
#include "SearchChecksDlg.h"
#include "SearchReportDescDlg.h"
#include "ChangeLocationDlg.h"
#include "MedicationSelectDlg.h"
#include "HL7SettingsDlg.h"
#include "NxSocketUtils.h"
#include "NxBackupUtils.h"
#include "DontShowDlg.h"
#include "LicenseDlg.h"
#include "SendMessageDlg.h"
#include "ScanMultiDocDlg.h"
#include "MergePatientsDlg.h"
#include "GenProductBarcodesDlg.h"
#include "FinancialLineItemPostingDlg.h"
#include "GCEntryDlg.h"
#include "GCSearchDlg.h"
#include "TelevoxExportDlg.h"
#include "PaymentDlg.h"
#include "SchedulingProductivityDlg.h"
#include "ViewInquiries.h"
#include "EditDrawersDlg.h"
#include "CodeLinkDlg.h"
#include "NYWCSetupDlg.h"
#include "sti.h"
#include "SelectDlg.h"
#include "MarketingRc.h"
#include "NexWebImportDlg.h"
#include "PicContainerDlg.h"
#include "DeactivateWorkstationsDlg.h"
#include "InactivateMultiplePatientsDlg.h"
#include "NxOutlookUtils.h"
#include "ReportGroupsDlg.h"
#include "EmrTemplateEditorDlg.h"
#include "EMNToBeBilledDlg.h"
#include "TemplateItemEntryGraphicalDlg.h"
#include "EditFamilyDlg.h" // (a.walling 2006-11-21 13:36) - PLID 23621 - FamilyUtils:: namespace
#include "ConfigureAAFPRSSurveyDlg.h"
#include "AllowedAmountSearchDlg.h"
#include "BatchMergeTrackingDlg.h"
#include "LicenseEMRProvidersDlg.h" 
#include "CareCreditUtils.h"
#include "OPOSMSRDevice.h"
#include "SwiperSetupDlg.h"
#include "OPOSPrinterDevice.h"
#include "logindlg.h"
#include "NxRPEMDIChildWnd.h"
#include "POSPrinterSettingsDlg.h"
#include "POSReceiptConfigDlg.h"
#include "MICRSetupDlg.h"
#include "OPOSCashDrawerDevice.h"
#include "POSCashDrawerConfigDlg.h"
#include "ImportWizardDlg.h"
#include "CreditCardProcessingSetupDlg.h"
#include "CreditCardProcessingSummaryDlg.h"
#include "CareCreditSetupDLG.h"
#include "PinpadUtils.h"
#include "NexFormsImportWizardMasterDlg.h"
#include "TrackingConversionConfigDlg.h"
#include "InvConsignmentPurchaseDlg.h"
#include "InvConsignmentAssignDlg.h"
#include "LabFollowUpDlg.h"
#include "MultiUserLogDlg.h"
#include "NxDockBar.h"
#include "HL7Utils.h"
#include "TodoUtils.h"
#include "ConfigureSuperbillTemplatesDlg.h"
#include "ApptsWithoutAllocationsDlg.h"
#include "FaxSetupDlg.h"
#include "FaxSendDlg.h"
#include "FaxServiceSetupDlg.h"				//(e.lally 2011-03-18) PLID 42767
#include "PatientSummaryDlg.h"
#include "ConfigureReportSegmentsDlg.h"
#include "ParseTranscriptionsDlg.h"
#include "PastSuperbillsDlg.h"
#include "FileUtils.h"
#include "NxCoCWizardMasterDlg.h"
#include "InvReconcileHistoryDlg.h"
#include "NewCropSetupDlg.h"
#include "SureScriptsSettingsDlg.h"
#include "NewCropBrowserDlg.h"
#include "SureScriptsCommDlg.h"
#include "EditComboBox.h"
#include "OrderSetTemplateDlg.h"
#include "PrescriptionRenewalRequestDlg.h"
#include "CCDUtils.h"
#include "CCDInterface.h"					//(e.lally 2010-02-18) PLID 37438
#include "HistoryUtils.h"
#include "QBMSProcessingSetupDlg.h"			// (d.thompson 2009-06-23) - PLID 34690 - Added QBMS processing.
#include "NxModalParentDlg.h"
#include "RSIMMSLink.h"
#include "NexSyncSettingsDlg.h"
#include "FirstDataBankUtils.h"
#include "ForceNexWebEMNDlg.h"
#include "HL7BatchDlg.h"
#include "SignatureDlg.h"
#include "UpdateProductsDlg.h"
#include "ConfigureReferralSourcePhoneNumbersDlg.h"
#include "EMRImageStampSetupDlg.h"
#include "InterventionTemplatesDlg.h"
#include "ConfigEMNChargeDiagCodeReportDlg.h"
#include "PurgeCardholderDataDlg.h"
#include "AppointmentReminderSetupDlg.h"
#include "SingleSelectDlg.h"
#include "ContactView.h" // (k.messina 2010-04-12 11:15) - PLID 37957
#include "ConfigurePermissionGroupsDlg.h"
#include "QuickBookRepID.h"// (a.vengrofski 2010-04-20 15:18) - PLID <38205>
#include "BoldSettingsDlg.h" // (j.gruber 2010-05-21 11:08) - PLID 38817
#include "RemoteDataCache.h"
#include "NYMedicaidSetupDlg.h" // (k.messina 2010-07-15 17:22) - PLID 39685
#include "LabEntryDlg.h"
#include "OHIPUtils.h"
#include "SecurityGroupsDlg.h"
#include "ChaseProcessingSetupDlg.h"	// (d.lange 2010-09-08 14:03) - PLID 40309 - Chase CC Processing setup dialog
#include "DecisionRuleUtils.h"
#include "EducationalMaterialSetupDlg.h"
#include "EducationalMaterialsDlg.h"
#include "AlbertaHLINKUtils.h"
#include "BillTabSettingsDlg.h"	// (j.dinatale 2010-11-05) - PLID 39226
#include "VisionWebOrderDlg.h"
#include "FinancialCloseDlg.h"
#include "BillableCPTCodesDlg.h"
#include "InvVisionWebUtils.h"
#include <HL7ParseUtils.h>
#include "EmrCodingGroupManager.h"
#include "UnlinkHL7ThirdPartyID.h" // (b.savon 2011-10-04 12:01) - PLID 39890
#include "BarcodeSetupDlg.h"
#include "OPOSBarcodeScanner.h"	// (a.wilson 2012-1-10) PLID 47517
#include "OposScan.h"
#include "EmrTemplateFrameWnd.h"
#include "RecallsNeedingAttentionDlg.h"	// (j.armen 2012-02-27 12:22) - PLID 48303
#include "CodeCorrectSetupDlg.h"	// (d.singleton 2012-04-02 17:56) - PLID 49336 added new dialog
#include "SpecialtyConfigDlg.h" // (j.luckoski 2012-04-10 17:17) - PLID 49491
#include "ContactLensOrderForm.h" // (j.dinatale 2012-03-21 13:33) - PLID 49079
#include "EOBDlg.h"
#include <NxAdoLib/NxAdoConnectionEvents.h>
#include "EmrPatientFrameWnd.h"
#include "WoundCareSetupDlg.h" // (r.gonet 08/03/2012) - PLID 51947
#include "EEligibility.h"
#include "OMRScanDlg.h"	// (j.dinatale 2012-08-13 15:50) - PLID 51941
#include "EMROMRMapperDlg.h" // (b.spivey, August 30, 2012) - PLID 51928 - 
#include "NexERxRegistrationDlg.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here
#include "DrugInteractionDlg.h"
#include "NexERxSetupDlg.h"
#include "HL7Utils.h"
#include "EMRCustomPreviewLayoutsMDIFrame.h"
#include "PrescriptionQueueDlg.h"	// (a.wilson 2013-01-10 09:06) - PLID 54535
#include "NxModelessParentDlg.h"	// (a.wilson 2013-01-10 09:06) - PLID 54535
#include "TopazSigPad.h"	//(d.singleton 2013-01-05 15:58) - PLID 56520
#include "TopazSigPadSettingsDlg.h"
#include "DeviceImportMonitor.h"
#include "CCHITReportsDlg.h" // (r.gonet 06/12/2013) - PLID 55151
#include "HL7Client_Practice.h"
#include "MUDetailedReportDlg.h"
#include "EStatementPatientSelect.h"
#include "CCDAConfigDlg.h"
#include "UMLSLoginDlg.h" // (j.camacho 2013-10-07 14:53) - PLID 58678
#include "PatientListsDlg.h" // (r.gonet 09/30/2013) - PLID 56236
#include "FirstAvailList.h" // (z.manning 2013-11-15 15:03) - PLID 58756
#include "EMN.h"
#include "DiagSearchConfig.h"				// (d.thompson 2014-02-04) - PLID 60638
#include "DiagSearchConfig_ManagedICD9Search.h"	// (d.thompson 2014-02-04) - PLID 60638
#include "DiagSearchConfig_ManagedICD10Search.h"	// (d.thompson 2014-02-04) - PLID 60638
#include "DiagSearchConfig_Crosswalk.h"		// (d.thompson 2014-02-04) - PLID 60638
#include "DiagSearchConfig_DiagCodesDropdown.h"	// (d.thompson 2014-02-04) - PLID 60638
#include "DiagSearchConfig_FullICD9or10Search.h"	// (d.thompson 2014-02-04) - PLID 60638
#include "EMRProblemListDlg.h"
#include "HL7ExportDlg.h" // (r.gonet 03/18/2014) - PLID 60782 - Moved to cpp file to avoid rebuilds
#include "AnchorSettingsDlg.h" // (j.armen 2014-07-09 17:31) - PLID 58034
#include "LockBoxPaymentImportDlg.h" // (d.singleton 2014-07-11 10:02) - PLID 62862
#include "EligibilityReviewDlg.h"	// (r.goldschmidt 2014-10-08 16:18) - PLID 62644
#include "EEligibilityTabDlg.h"		// (r.goldschmidt 2014-10-10 16:11) - PLID 62644
#include "EligibilityRequestDetailDlg.h"	// (r.goldschmidt 2014-10-10 16:11) - PLID 62644
#include "FirstAvailableAppt.h" // (r.gonet 2014-11-19) - PLID 64173 - Moved the include to the cpp file.
#include "NxPracticeSharedLib\ICCPDeviceManager.h"
#include "NxPracticeSharedLib\ICCPUtils.h"
#include "ComplexReportQuery.h"
#include "NextechIconButton.h"

// (a.walling 2013-04-11 17:05) - PLID 56225 - Move DeviceImport stuff into its own class

#include <NxAlgorithm.h>
#include <algorithm>
#include <map>
#include <vector>

#include "LoggingUtils.h"
#include <WindowUtils.h>
#include "ReconcileMedicationsUtils.h"
#include "financialdlg.h"
#include "EMRSearch.h"
#include "ConfigurePrimaryEMNProviderDlg.h"
#include "CDSInterventionListDlg.h"
#include "EligibilityRequestDlg.h"	// (j.jones 2014-02-28 10:27) - PLID 60767 - moved header to the cpp
#include "NxWIA.h" // (a.walling 2014-03-12 12:31) - PLID 61334 - reduce stdafx dependencies and #imports
#include "invutils.h"
#include "BillingModuleDlg.h"
#include "RoomManagerDlg.h"
//(s.dhole 12/4/2014 5:30 PM ) - PLID  64337
#include "MassAssignSecurityCodesDlg.h"
#include "GlobalSchedUtils.h"

#include "ReschedulingQueue.h"
#include "ICCPSetupDlg.h"
#include <NxSystemUtilitiesLib\RemoteDesktopServices.h>
#include "NxAPIManager.h"
#include "SSRSSetupDlg.h"

using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2012-07-12 08:56) - PLID 51501 - Now derived from CNxMDIFrameWnd

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (a.wilson 2014-04-02) PLID 61629 - removed all code and code related to the group picture dialog.

//DRT 6/15/2007 - PLID 25531 - Packets are no longer part of NetUtils namespace, removed numerous NetUtils::

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2012-12-04 15:32) - PLID 54037 - Timer resilience, logging for device import
extern CPracticeApp theApp;


static CWinThread* g_pPreloadNexPhotoThread; // (a.walling 2011-12-15 10:25) - PLID 40593 - NexPhoto preload thread and bool
static bool g_bPreloadedNexPhotoLibs;

// (a.walling 2011-12-15 10:25) - PLID 40593 - Wait for NexPhoto preload thread
void WaitForPreloadNexPhotoLibs()
{
	if (!g_bPreloadedNexPhotoLibs || !g_pPreloadNexPhotoThread) {
		return;
	}

	DWORD dwResult = ::WaitForSingleObject(g_pPreloadNexPhotoThread->m_hThread, 0);

	if (WAIT_TIMEOUT == dwResult) {
		g_pPreloadNexPhotoThread->SetThreadPriority(THREAD_PRIORITY_NORMAL);

#ifdef _DEBUG
		static const DWORD dwWait = 90000;
#else
		static const DWORD dwWait = 30000;
#endif

		dwResult = ::WaitForSingleObject(g_pPreloadNexPhotoThread->m_hThread, dwWait);
	}

	CWinThread* pThread = NULL;
	std::swap(pThread, g_pPreloadNexPhotoThread);

	if (WAIT_OBJECT_0 == dwResult) {
		delete pThread;
	} else {
		// well, it's not responding for 30 seconds, the best thing to do probably is just to let it go
		// the crt will terminate it anyway, the same as if we do it here, but with the benefit of doing
		// so closer to the natural termination point of the process.
		pThread->m_bAutoDelete = TRUE;
	}
}

//TES 1/6/2010 - PLID 36761 - Moved ResetFilter to PatientToolbar.cpp

// (a.walling 2008-04-18 17:17) - PLID 29731 - Toolbar text height
long g_cnToolbarTextHeight = 16;

//extern CDaoDatabase g_dbPracData;
extern CPracticeApp theApp;
/////////////////////////////////////////////////////////////////////////////
// CMainFrame

#pragma TODO("!!Temporary table checker statistics for rusonis incident!!")

namespace TableCheckerStats
{
	// (c.haag 2015-06-24) - PLID 65911 - The TableCheckerStats namespace now has its own logger
	static CThreadsafeLogFile LogFile;
	static CLogger Logger(LogFile, CLogger::defaultOptions | CLogger::eLogTicks);
	static BOOL bLogFileInitialized = FALSE;

	struct Stats
	{
		Stats()
			: hits(0)
			, ms(0)
			, maxms(0)
		{
		}

		DWORD hits;
		__int64 ms;
		__int64 maxms;

		void Update(__int64 newms)
		{
			++hits;
			ms += newms;

			if (newms > maxms) {
				maxms = newms;
			}
		}
	};

	typedef std::map<int, Stats> StatsMap;		

	struct StatsByTime
	{
		bool operator()(const StatsMap::value_type& l, const StatsMap::value_type& r) const
		{
			if (l.second.ms > r.second.ms) {
				return true;
			} else if (l.second.ms < r.second.ms) {
				return false;
			}

			if (l.second.maxms > r.second.maxms) {
				return true;
			} else if (l.second.maxms < r.second.maxms) {
				return false;
			}

			if (l.second.hits > r.second.hits) {
				return true;
			} else if (l.second.hits < r.second.hits) {
				return false;
			}

			if (l.first > r.first) {
				return true;
			} else if (l.first < r.first) {
				return false;
			}

			return false;
		}
	};

	struct StatsByHits
	{
		bool operator()(const StatsMap::value_type& l, const StatsMap::value_type& r) const
		{
			if (l.second.hits > r.second.hits) {
				return true;
			} else if (l.second.hits < r.second.hits) {
				return false;
			}

			if (l.second.ms > r.second.ms) {
				return true;
			} else if (l.second.ms < r.second.ms) {
				return false;
			}

			if (l.second.maxms > r.second.maxms) {
				return true;
			} else if (l.second.maxms < r.second.maxms) {
				return false;
			}

			if (l.first > r.first) {
				return true;
			} else if (l.first < r.first) {
				return false;
			}

			return false;
		}
	};

	struct Accumulator
	{
		Accumulator()
			: lastLog_(GetTickCount())
		{
		}

		Stats total_;
		StatsMap statsMap_;
		DWORD lastLog_;

		void Add(NetUtils::ERefreshTable table, __int64 ms)
		{
			if (ms <= 1) {
				return;
			}

			total_.Update(ms);
			statsMap_[table].Update(ms);

			if (ShouldLog()) {
				LogStats();
			}
		}

		bool ShouldLog()
		{
			if (0 == (total_.hits % 25)) {
				return true;
			} else if (10000 < (GetTickCount() - lastLog_)) {
				return true;
			} else if (1 == total_.hits) {
				return true;
			}

			return false;
		}

		void LogStats()
		{
			// (c.haag 2015-06-24) - PLID 65911 - Initialize the log file
			if (!bLogFileInitialized)
			{
				LogFile.SetLog(GetPracPath(PracPath::SessionPath) ^ "TableCheckerStats.log");
				bLogFileInitialized = TRUE;
			}

			lastLog_ = GetTickCount();

			// (c.haag 2015-06-24) - PLID 65911 - Use the Logger to log the stats
			Logger.Log("TableCheckerStats: %lu hits \t - %I64i ms \t[max %I64i]\r\n", total_.hits, total_.ms, total_.maxms);

			typedef std::pair<int, Stats> TableStats;
			typedef std::vector<TableStats> StatsVector;
			StatsVector all(statsMap_.begin(), statsMap_.end());

			{
				CString str;
				str.Preallocate(statsMap_.size() * 64);

				str.Append("By time:\r\n");

				std::sort(all.begin(), all.end(), StatsByTime());

				for (StatsVector::iterator it = all.begin(); it != all.end(); ++it) {
					str.AppendFormat("[0x%08x] \t : %lu hits \t - %I64i ms \t[max %I64i]\r\n", it->first, it->second.hits, it->second.ms, it->second.maxms);
				}

				// (c.haag 2015-06-24) - PLID 65911 - Use the Logger to log the stats
				Logger.Log("%s", str);
			}

			{
				CString str;
				str.Preallocate(statsMap_.size() * 64);

				str.Append("By hits:\r\n");

				std::sort(all.begin(), all.end(), StatsByHits());

				for (StatsVector::iterator it = all.begin(); it != all.end(); ++it) {
					str.AppendFormat("[0x%08x] \t : %lu hits \t - %I64i ms \t[max %I64i]\r\n", it->first, it->second.hits, it->second.ms, it->second.maxms);
				}

				// (c.haag 2015-06-24) - PLID 65911 - Use the Logger to log the stats
				Logger.Log("%s", str);
			}
		}
	};

	Accumulator g_tableCheckerStatistics;

	struct Entry
	{
		Entry(NetUtils::ERefreshTable table)
			: table(table)
		{
			::QueryPerformanceCounter(&ticksInitial);
		}

		~Entry()
		{
			LARGE_INTEGER ticksFinal;
			::QueryPerformanceCounter(&ticksFinal);

			__int64 total = ticksFinal.QuadPart - ticksInitial.QuadPart;

			static LARGE_INTEGER freq = {0};
			if (0 == freq.QuadPart) {
				::QueryPerformanceFrequency(&freq);
				freq.QuadPart /= 1000;
			}

			__int64 ms = total / freq.QuadPart;

			g_tableCheckerStatistics.Add(table, ms);
		}

		LARGE_INTEGER ticksInitial;
		NetUtils::ERefreshTable table;
	};

} // namespace TableCheckerStats


#define IDT_TEST_CALLERID				2001


// (a.walling 2007-11-20 12:52) - PLID 28062 - VS2008 - Aggghh, IDs should never exceed one WORD! These were huge! Causing VS2008 to crash
// when floating a toolbar. The problem is that MFC internals sometimes use pointers and IDs interchangably, assuming that all IDs will be
// < 0xFFFF and all pointers >= 0x1,0000. According to TN020 http://msdn2.microsoft.com/en-us/library/t2zechd4(VS.80).aspx the highest range
// should be 0xDFFF, which I have now changed these to be close to.
#define IDW_PATIENTS_TOOLBAR			57300
#define IDW_CONTACTS_TOOLBAR			57301
#define IDW_DOC_TOOLBAR					57302
//DRT 3/14/2008 - After reviewing PLID 28062, I noticed this was the same ID and not used.  To reduce confusion I just removed it.
//#define IDW_DATE_TOOLBAR				57302

#define ID_VIEW_TOOLBARS_CONTACTS_BAR	57303

#define CHECK_SUPER(dlg_text) ((GetAsyncKeyState(VK_SHIFT) & 0x80000000) && (AfxMessageBox(dlg_text, MB_YESNO | MB_ICONQUESTION) == IDYES))

//tells whether Global tool bar buttons are enabled
#define TB_PATIENT		0x0001
#define TB_SCHEDULE		0x0002
#define TB_DOCUMENT		0x0004
#define TB_CONTACT		0x0008
#define TB_MARKET		0x0010
#define TB_INVENTORY	0x0020
#define TB_FINANCIAL	0x0040
#define TB_REPORTS		0x0100
#define TB_ADMIN		0x0200
#define TB_NEWPATIENT	0x0400
#define TB_ASC			0x0800
#define TB_EMR			0x1000
#define	TB_FFA			0x2000
#define TB_CARECREDIT	0x4000
// (d.thompson 2009-11-16) - PLID 36134
#define TB_LINKS		0x8000

// (z.manning 2009-08-25 10:52) - PLID 31944 - Define a debug only system menu option 
// to size Practice to 1024x768.
#define DEBUGMENU_ID_SIZE_TO_1024_768		WM_USER + 0x444
// (a.walling 2009-10-23 09:46) - PLID 36046 - Ability to dump all EMNDetail objects in memory with reference count history
#ifdef WATCH_EMNDETAIL_REFCOUNTS
#define DEBUGMENU_ID_DUMP_ALL_EMNDETAILS	WM_USER + 0x443
#endif


IMPLEMENT_DYNAMIC (CClip, CObject)

IMPLEMENT_DYNAMIC(CMainFrame, CNxMDIFrameWnd)

// (a.walling 2008-09-10 13:21) - PLID 31334 - Handle OnActivate
// (a.walling 2008-10-28 13:05) - PLID 31334 - Added OnWIASourceLaunch, when launched from WIA auto-start
BEGIN_MESSAGE_MAP(CMainFrame, CNxMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_INITMENU()
	ON_WM_CLOSE()
	ON_WM_WINDOWPOSCHANGING()
	// (a.walling 2014-06-09 09:37) - PLID 57388 - Placeholders - useful for debugging without rebuilding everything
	ON_WM_ACTIVATE()
	ON_WM_ACTIVATEAPP()
	ON_MESSAGE(WM_ACTIVATETOPLEVEL, OnActivateTopLevel)
	ON_BN_CLICKED(IDC_ACTIVE_TOOLBAR, OnActiveClicked)
	ON_BN_CLICKED(IDC_MAIN_SEARCH, OnMainPhysSearchClicked)
	ON_BN_CLICKED(IDC_REFERING_SEARCH, OnReferringPhysSearchClicked)
	ON_BN_CLICKED(IDC_EMPLOYEE_SEARCH, OnEmployeesSearchClicked)
	ON_BN_CLICKED(IDC_OTHER_SEARCH, OnOtherContactsSearchClicked)
	ON_BN_CLICKED(IDC_CONTACTS_SEARCH, OnAllContactsSearchClicked)
	ON_BN_CLICKED(IDC_ALL_TOOLBAR, OnAllClicked)
	ON_BN_CLICKED(IDC_PATIENTS_SEARCH, OnPatientSearchClicked)
	ON_BN_CLICKED(IDC_PROSPECT_SEARCH, OnProspectSearchClicked)
	ON_BN_CLICKED(IDC_ALL_SEARCH, OnAllSearchClicked)
	ON_BN_CLICKED(IDC_SUPPLIER_SEARCH, OnSupplierSearchClicked)
	ON_BN_CLICKED(IDC_FILTER, OnFilterSearchClicked)
	ON_BN_CLICKED(ID_BATCH_PAYS, OnBatchPayments)
	ON_BN_CLICKED(IDC_ACTIVE_CONTACTS, OnActiveContacts)
	ON_WM_TIMER()
	ON_COMMAND(ID_VIEW_RESETTOOLBARS, OnViewResettoolbars)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RESETTOOLBARS, OnUpdateViewResettoolbars)
//for tooltips
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnNeedText)
ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnNeedText)

//* Brad, let's discuss if this is really needed - RAC
	ON_WM_QUERYNEWPALETTE()
//*/
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_PALETTECHANGED()
	ON_MESSAGE(NXM_NOTIFICATION_CLICKED, OnNotificationClicked)
	ON_MESSAGE(NXM_ALLOW_POPUP, OnMessageAllowPopup)
	ON_MESSAGE(NXM_OPEN_MOVEUP_DIALOG, OnMessageOpenMoveupDialog)
	ON_MESSAGE(NXM_EXPLICIT_DESTROY_VIEW, OnExplicitDestroyView)
	ON_MESSAGE(NXM_OUTLOOKSYNC_FINISHED, OnOutlookSyncFinished)
	ON_MESSAGE(NXM_PROMPT_MOVEUP, OnPromptMoveup)
	ON_MESSAGE(NXM_NEW_PAYMENT, OnNewPayment)
	ON_MESSAGE(NXM_TWAIN_SOURCE_LAUNCH, OnTWAINSourceLaunch)
	ON_REGISTERED_MESSAGE(NxWIA::NXM_ACQUIRE_FROM_WIA, OnWIASourceLaunch)
	ON_MESSAGE(NXM_EMR_CLOSING, OnEMRGroupClosing)
	ON_MESSAGE(NXM_PRINT_EMN_REPORT, OnPrintEMNReport)
	ON_MESSAGE(NXM_FORCIBLY_CLOSE, OnForciblyClose)
	ON_MESSAGE(NXM_RECHECK_LICENSE, OnRecheckLicense)
	ON_MESSAGE(NXM_PREVIEW_PRINTED, OnPreviewPrinted)
	ON_MESSAGE(NXM_PREVIEW_CLOSED, OnPreviewClosed)
	ON_MESSAGE(NXM_EMRITEMADVDLG_ITEM_SAVED, OnEmrItemEntryDlgItemSaved)
	ON_MESSAGE(NXM_DISPLAY_ERROR_MESSAGE, OnDisplayErrorMessage)
	// (b.savon 2013-06-27 10:02) - PLID 57344
	ON_MESSAGE(NXM_POP_UP_MESSAGEBOX, OnPopUpMessageBox)
	ON_MESSAGE(NXM_INITIATE_OPOSMSRDEVICE, OnInitiateOPOSMSRDevice)
	ON_MESSAGE(NXM_INITIATE_OPOSBARCODESCANNER, OnInitiateOPOSBarcodeScanner)
	ON_MESSAGE(NXM_INITIATE_OPOSPRINTERDEVICE, OnInitiateOPOSPrinterDevice)
	ON_MESSAGE(NXM_INITIATE_OPOSCASHDRAWERDEVICE, OnInitiateOPOSCashDrawerDevice)
	ON_MESSAGE(NXM_POPUP_APPTS_WO_ALLOCATIONS, OnPopupApptsWoAllocations)
	// (a.walling 2008-06-26 13:12) - PLID 30531
	ON_MESSAGE(NXM_EDIT_EMR_OR_TEMPLATE, OnMsgEditEMROrTemplate)
	// (a.walling 2008-09-10 13:31) - PLID 31334
	// (a.walling 2009-06-22 10:48) - PLID 34635 - Dead/useless code
	//ON_MESSAGE(NXM_WIA_EVENT, OnWIAEvent)
	//}}AFX_MSG_MAP
	// Global help commands
	ON_COMMAND(ID_HELP_FINDER, OnHelpFinder)
	ON_COMMAND(ID_CONTEXT_HELP, OnManualBtn)
	ON_COMMAND(ID_DEFAULT_HELP, OnManualBtn)
	// Other commands
	ON_COMMAND(ID_SUPERBILL, OnSuperbill)
	ON_COMMAND(ID_ENVELOPE, OnEnvelope)
	ON_COMMAND(ID_PRESCRIPTION, OnPrescription)
	ON_COMMAND(ID_BATCH_MERGE, OnBatchMerge)
	ON_COMMAND(ID_EDUCATIONAL_MATERIALS, OnEducationalMaterials)
	ON_COMMAND(ID_PRACYAKKER_SEND, OnPracYakkerSend)
	ON_COMMAND(ID_HELP_LICENSEAGREEMENT, OnHelpLicenseAgreement)
	// (a.pietrafesa 2015-06-22) - PLID 65910 - View Log File in Help menu
	ON_COMMAND(ID_HELP_VIEWLOGFILE, OnViewLogFile)
	ON_COMMAND(ID_ACTIVITIES_UPDATEINVENTORYPRICES, OnActivitiesUpdateInventoryPrices)
	ON_COMMAND(ID_ACTIVITIES_UPDATEINFORMATIONFORMULTIPLEPRODUCTS, OnUpdateInformationForMultipleProducts)
	ON_COMMAND(ID_ACTIVITIES_CONSIGNMENT_PURCHASEITEMSFROMCONSIGNMENT, OnActivitiesPurchaseItemsFromConsignment)
ON_COMMAND(ID_ACTIVITIES_CONSIGNMENT_TRANSFERITEMSTOCONSIGNMENT, OnActivitiesTransferItemsToConsignment)
ON_COMMAND(ID_TOOLS_LOGTIME_PROMPT, OnToolsLogtimePrompt)
ON_COMMAND(ID_TOOLS_LOGTIME_LOGMANAGEMENT, OnToolsLogtimeLogManagement)
ON_COMMAND(ID_ACTIVITIES_APPLYSUPERBILLS, OnActivitiesApplySuperbills)
ON_COMMAND(ID_ACTIVITIES_VOIDSUPERBILLS, OnActivitiesVoidSuperbills)
ON_COMMAND(ID_ACTIVITIES_EYEGRAPH, OnActivitiesEyeGraph)
ON_COMMAND(ID_EXPORT_TO_MEDINOTES, OnExportToMedinotes)
ON_COMMAND(ID_SHOW_PREFERENCES, OnShowPreferences)
ON_COMMAND(ID_SEARCH_NOTES, OnSearchNotes)
ON_COMMAND(ID_CHANGE_PASSWORD, OnChangePassword)
ON_COMMAND(ID_VIEW_CURRENTALERTS, OnViewCurrentAlerts)
ON_COMMAND(ID_ACTIVITIES_SEARCHCHECKCCNUMBERS, OnActivitiesSearchCheckCCNumbers)
// (a.walling 2007-05-04 12:05) - PLID 4850 - We really can switch users now.
ON_COMMAND(ID_FILE_SWITCH_USER, OnFileSwitchUser)
ON_COMMAND(ID_ACTIVITIES_SEARCHREPORTDESCRIPTIONS, OnActivitiesSearchReportDescriptions)
ON_COMMAND(ID_TOOLS_HL7SETTINGS, OnToolsHL7Settings)
ON_COMMAND(ID_TOOLS_PROCESS_HL7_MESSAGES, OnToolsProcessHL7Messages)
// (j.jones 2008-04-14 09:29) - PLID 29596 - this dialog was removed in lieu
// of the new HL7 tab in the Financial module
//ON_COMMAND(ID_MODULES_IMPORTFROMHL7, OnModulesImportFromHL7)
ON_COMMAND(ID_MODULES_IMPORTFROMNEXWEB, OnImportFromNexWeb)
//ON_COMMAND(ID_MODULES_IMPORTFROMLYB, OnImportFromLYB)
ON_COMMAND(IDM_SCANMULTIPLEDOCS, OnScanMultipleDocs)
ON_COMMAND(ID_TOOLS_SCANMULTIPLEDOCUMENTS, OnScanMultipleDocs)
ON_COMMAND(ID_ACTIVITIES_MERGEPATIENT, OnActivitiesMergePatient)
ON_COMMAND(IDM_INACTIVATE_MULTI_PATS, OnActivitiesInactivateMultiplePatients)
ON_COMMAND(IDM_CONFIGURE_REPORT_SEGMENTS, OnActivitiesConfigureReportSegments)
ON_COMMAND(ID_ACTIVITIES_ADDGIFTCERTIFICATE, OnActivitiesAddGiftCertificate)
ON_COMMAND(ID_ACTIVITIES_FINDGIFTCERTIFICATE, OnActivitiesFindGiftCertificate)
ON_COMMAND(ID_ACTIVITIES_NEXSPA_EDITCASHDRAWERSESSIONS, OnActivitiesNexSpaEditCashDrawerSessions)
ON_COMMAND(ID_MODULES_EXPORTTOTELEVOX, OnModulesExportToTelevox)
ON_COMMAND(ID_ACTIVITIES_SCHEDULING_PRODUCTIVITY, OnActivitiesSchedulingProductivity)
ON_COMMAND(ID_VIEW_INQUIRIES, OnViewInquiries)
ON_COMMAND(ID_IMPORT_EMR_CONTENT, OnImportEmrContent)
ON_COMMAND(ID_EXPORT_EMR_CONTENT, OnExportEmrContent)
ON_COMMAND(ID_TOOLS_IMPORT_NEXFORMS_CONTENT, OnToolsImportNexFormsContent) // (z.manning, 07/17/2007) - PLID 18359
ON_COMMAND(ID_TOOLS_IMPORTNXCYCLEOFCARECONTENT, OnToolsImportNxCycleOfCareContent)
ON_COMMAND(ID_TOOLS_IMPORTEMRSTANDARDCONTENT, OnToolsImportEmrStandardContent)
ON_COMMAND(ID_TB_NEW_EMR, OnNewEmr)
ON_NOTIFY(TBN_DROPDOWN, AFX_IDW_TOOLBAR, OnDropDown)
ON_COMMAND(ID_UPDATE_LICENSE, OnUpdateLicense)
ON_COMMAND(ID_DEACTIVATE_WORKSTATIONS, OnDeactivateWorkstations)
ON_COMMAND(ID_EMR_PATIENTS_TO_BE_BILLED, ShowPatientEMNsToBeBilled)
ON_COMMAND(ID_SEARCH_ALLOWABLES, OnSearchAllowables)
ON_COMMAND(ID_TOOLS_LOGTIME_LOG_OTHER_USER, OnToolsLogTimeLogOtherUser)
ON_COMMAND(ID_APPTS_WO_ALLOCATIONS, OnApptsWithoutAllocations)
// (j.jones 2008-07-08 12:06) - PLID 24624 - added ability to run the patient summary
ON_COMMAND(ID_PATIENT_SUMMARY, OnPatientSummary)
// (a.walling 2009-05-13 15:44) - PLID 34243 - Import CCD documents
ON_COMMAND(ID_ACTIVITIES_IMPORTCCD, OnImportCCD)
// (a.walling 2009-05-21 08:41) - PLID 34318 - Create a CCD summary document
ON_COMMAND(ID_ACTIVITIES_CREATECCD, OnCreateCCD)
// (a.walling 2008-07-16 17:18) - PLID 30751 - Access transcription parsing
ON_COMMAND(IDM_PARSE_TRANSCRIPTIONS, OnParseTranscriptions)
ON_COMMAND(ID_ACTIVITIES_REPRINTSUPERBILLS, OnReprintSuperbills)
// (j.jones 2009-02-27 12:20) - PLID 33265 - added e-prescribing settings
//ON_COMMAND(ID_TOOLS_ELECTRONICPRESCRIBINGSETTINGS, OnEPrescribingSettings)
//TES 4/10/2009 - PLID 33889 - Replaced with menu options specific to each type
ON_COMMAND(ID_NEWCROP_SETTINGS, OnNewCropSettings)
ON_COMMAND(ID_SURESCRIPTS_SETTINGS, OnSureScriptsSettings)
ON_COMMAND(ID_ELECTRONICPRESCRIBING_REGISTERNEXERXCLIENTANDPRESCRIBERS, OnRegisterNexERxClientAndPrescribers)
ON_COMMAND(ID_TOOLS_UMLSSETTINGS, OnUMLSLoginDlg)// (j.camacho 2013-10-07 16:00) - PLID 58678
ON_COMMAND(ID_ACTIVITIES_HL7_TO_BE_BILLED, ShowHL7ToBeBilledDlg)
ON_COMMAND(ID_ACTIVITIES_EDITPROVIDERTYPES, OnEditProviderTypes)
// (j.gruber 2009-05-15 10:20) - PLID 28541 - View renewal requests
ON_COMMAND(ID_ELECTRONICPRESCRIBING_VIEW_NEWCROP_RENEWAL_REQUESTS, OnNewCropRenewalRequests)
ON_WM_SYSCOMMAND()
// (j.gruber 2013-10-08 13:08) - PLID 59062
ON_COMMAND(ID_TOOLS_CCDA_SETTINGS, OnCCDASettings)
// (j.jones 2009-12-28 08:59) - PLID 32150 - added ability to delete unused service codes
ON_COMMAND(ID_DELETE_UNUSED_SERVICE_CODES, OnDeleteUnusedServiceCodes)
// (j.jones 2012-03-27 08:59) - PLID 45752 - added ability to delete unused diagnosis codes
ON_COMMAND(ID_DELETE_UNUSED_DIAG_CODES, OnDeleteUnusedDiagCodes)
// (j.jones 2012-03-27 09:20) - PLID 47448 - added ability to delete unused modifiers
ON_COMMAND(ID_DELETE_UNUSED_MODIFIERS, OnDeleteUnusedModifiers)
// (j.gruber 2010-01-11 15:46) - PLID 36647 - added configuration of referral source phone numbers
ON_COMMAND(ID_ACTIVITIES_CONFIGUREREFERRALSOURCEPHONENUMBERS, OnConfigureReferralSourcePhoneNumbers)
// (j.gruber 2010-02-23 16:10) - PLID 37509
ON_COMMAND(ID_ACTIVITIES_CONFIGURE_CLINICAL_SUPPORT_RULES, OnConfigureClinicalSupportDecisionRules)
// (j.gruber 2010-03-09 09:49) - PLID 37660
ON_COMMAND(ID_ACTIVITIES_CONFIGUREEMNCHARGESANDDIAGNOSISCODESBYEMNREPORT, OnConfigureEmnChargesDiagCodesReport)
// (j.gruber 2010-04-14 09:32) - PLID 37948
ON_COMMAND(IDM_CONFIGURE_PERM_GROUPS, OnConfigurePermissionGroups)
// (j.gruber 2010-05-21 10:49) - PLID 38817 - bold setup dialog
ON_COMMAND(ID_ACTIVITIES_BOLDSETTINGS, OnConfigureBold)
ON_COMMAND(ID_ACTIVITIES_PATIENTEDUCATIONSETUP, OnPatientEducationSetup)
// (r.gonet 08/03/2012) - PLID 51947 - Wound Care Configuration
ON_COMMAND(ID_ACTIVITIES_CONFIGUREWOUNDCARECODING, OnConfigureWoundCareCoding)
// (b.savon 2011-10-04 12:01) - PLID 39890
ON_COMMAND(ID_TOOLS_HL7_UNLINKHL73RDPARTYIDS, OnMenuClickUnlinkHL7ThirdPartyID)
// (d.singleton 2012-04-02 17:02) - PLID 49336
ON_COMMAND(ID_FINANCIAL_CODECORRECTSETUP, OnMenuClickCodeCorrectSetup)
ON_COMMAND(ID_ACTIVITIES_EDITSPECIALTYTRAINING, OnMenuClickSpecialtyDlg) // (j.luckoski 2012-04-10 17:17) - PLID 49491
ON_COMMAND(ID_ACTIVITIES_OMRPROCESSING, OnOMRProcessing)	// (j.dinatale 2012-08-13 15:35) - PLID 51941
ON_COMMAND(ID_ACTIVITIES_OMRFORMEDITING, OnRemarkOMRFormEditting) // (b.spivey, August 30, 2012) - PLID 51928 - 
ON_COMMAND(ID_ACTIVITIES_PRESCRIPTIONS_NEEDING_ATTENTION, OnPrescriptionsNeedingAttention)
ON_COMMAND(ID_DEVICES_TOPAZ_SIG_PAD, OnTopazSigPadSettings)
// (b.spivey, May 06, 2013) - PLID 56542
ON_MESSAGE(NXM_INITIATE_TOPAZSIGPADDEVICE, OnInitiateTopazSigPadDevice)
ON_COMMAND(ID_VIEW_CDS_INTERVENTIONS, OnViewCdsInterventions)
ON_WM_SIZE()
ON_MESSAGE(NXM_POST_NEXT_QUEUED_TABLECHECKER, OnPostNextQueuedTablechecker)
//(s.dhole 12/4/2014 10:56 AM ) - PLID 64337
ON_COMMAND(ID_ACTIVITIES_MASSASSIGNSECURITYCODES, OnMenuClickedMassAssignSecurityCodes)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

IMPLEMENT_MEMBER_TIMER(CMainFrame, IDT_DELETE_QUEUED_FILES_TIMER, 1020);

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

// (c.haag 2003-12-10 15:54) - This is a socket used
// for picking up backups in progress. Ultimately all of the
// netutils and CClient code will be replaced with NxSocketUtils,
// at which time this will become a member variable. Right now
// nxsocketutils.h should only exist in the scope of mainframe
// and pracprops.
NxSocketUtils::HCLIENT g_hNxServerSocket = NULL;

CMainFrame::CMainFrame()
	: m_dlgToDoAlarm(this)
	, m_dlgAlert(CWnd::GetDesktopWindow()) // (a.walling 2012-07-10 13:43) - PLID 46648 - Transient alert window should not have a parent to steal focus
	, m_pActiveEMRDlg(this)
	, m_pEMRLockManagerDlg(this)
	, m_pEMRSearchDlg((*(new CEMRSearch(this))))	// (j.jones 2013-07-15 15:16) - PLID 57477 - changed to a reference
	, m_EMNToBeBilledDlg(this)
	, m_HL7ToBeBilledDlg(this)
	, m_EMRWritableReviewDlg(this)
	, m_dlgHL7Export(*(new CHL7ExportDlg(this))) // (r.gonet 03/18/2014) - PLID 60782 - Changed to reference 
	, m_dlgAutoLogoff(this)
	, m_dlgAptBookAlarmList(this)
	, m_PalmPilotDlg(this)
	, m_NexFormEditorDlg(this)
	, m_dlgProductItems(this)
	, m_OMRProcessingDlg(this)
	, m_FirstAvailAppt(*(new CFirstAvailableAppt(this))) // (r.gonet 2014-11-19) - PLID 64173 - Changed to a reference
	
{
	// (a.walling 2009-10-09 16:42) - PLID 35911 - Set the main frame explicitly
	SetMainFrame(this);

	DeviceImport::GetMonitor().Init();

	CONSTRUCT_MEMBER_TIMER(IDT_DELETE_QUEUED_FILES_TIMER);

	m_pOpenDoc = NULL;
	m_enableViews = 0;	
	m_licensedViews = 0;
	m_nOpenDocCnt = 0;
	m_initialEnabledViews = 0;
	m_pView.SetSize(0, 1);
	m_pLockPatToolBar = 0;
	//m_bAllowTimers = true;
	m_pMirrorDlg = NULL;
	m_pUnitedDlg = NULL;
	m_pDocToolBar = NULL;
	m_pWhatsNewDlg = NULL;
	m_pNotificationDlg = NULL;
	StartHotKeys();
	InitializeNexTechIconButtonIcons();

	m_nWaitingType = WT_DONT_WAIT;
	m_nCountPopupDisallows = 0;

	m_nHighlightSortedColumn = 2; // initialize to unknown

	m_pLastActivatedView = NULL;
	m_bIsWaitingForProcess = FALSE;

	m_bAlreadyCheckedModulePermissions = FALSE;

	m_bIsReportRunning = false;

	m_nPatientIDToGoToOnClickNotification = -1;

	// (j.gruber 2010-01-07 15:05) - PLID 36648
	m_nReferralIDForNewPatient = -1;

	// (j.gruber 2010-01-11 13:43) - PLID 36140
	m_bNewPatientOpen = FALSE;

	m_pClipData = NULL;
	m_bMovingDocBar = false;

	m_pReportEngine = NULL;

	m_bIsPaletteChangedHandlerRunning = FALSE;

	m_bCloseNthModule = FALSE;
	m_nMaxOpenModules = 5;

	m_bForciblyClosing = false;

	m_pRoomManagerDlg = NULL;

	// (j.jones 2008-07-17 08:51) - PLID 30730 - added a modeless EMR Problem List
	m_pEMRProblemListDlg = NULL;

	// (j.jones 2008-10-21 17:41) - PLID 14411 - added EMR Analysis as a modeless dialog
	m_pEMRAnalysisDlg = NULL;

	// (j.jones 2012-05-25 10:55) - PLID 44367 - added E-Remittance as modeless
	m_pEOBDlg = NULL;

	m_pPOSPrinterDevice = NULL;
	// (a.walling 2011-03-21 17:32) - PLID 42931 - Use POSPrinterAccess
	//m_nPOSPrinterDeviceClaims = 0;

	m_pPOSCashDrawerDevice = NULL;

	// (a.walling 2007-05-04 10:20) - PLID 4850 - These are pointers now, so initialize them first thing.
	// the windows themselves are created in InitPracYakker.
	// (a.walling 2012-04-02 08:32) - PLID 46648 - Dialogs must set a parent!
	// (a.walling 2012-10-05 09:36) - PLID 53027 - Fix mainframe activation (pass NULL as parent)
	m_pdlgSendYakMessage = new CSendMessageDlg(NULL);
	m_pdlgMessager = new CMessagerDlg(this);

	//DRT 5/24/2007 - PLID 25892 - Default to non-existent
	m_pdlgOpportunityList = NULL;

	// (a.wetta 2007-07-03 13:04) - PLID 26547 - Initialize the MSR thread variable
	m_pOPOSMSRThread = NULL;
	// (a.wetta 2007-07-05 09:56) - PLID 26547 - Keep track of when the MSR device is initiated
	m_bOPOSMSRDeviceInitComplete = FALSE;

	// (j.jones 2009-08-13 14:14) - PLID 35213 - added a mutex for NewCrop patient account access
	m_bIsBrowsingNewCropPatientAccount = FALSE;

	// (a.walling 2008-04-15 10:21) - PLID 25755
	m_pLabFollowupDlg = NULL;

	m_bDisplayGradients = TRUE;

	m_pParseTranscriptionsDlg = NULL;

	// (j.jones 2009-03-02 14:34) - PLID 33053 - added NewCrop browser
	m_pNewCropBrowserDlg = NULL;

	// (a.walling 2008-09-10 13:21) - PLID 31334
	// (a.walling 2009-06-22 10:48) - PLID 34635 - Dead/useless code
	//m_pLastActivePIC = NULL;
	//m_pWIAEventSink = NULL;

	// (j.jones 2008-10-30 16:52) - PLID 31869 - track the thread ID for the Problem List's PopulateDetailValuesThread
	// and the Problem deletion dialog's PopulateDeletedProblemValuesThread
	m_nProblemListDlg_PopulateDetailValues_CurThreadID = -1;
	m_nProblemDeleteDlg_PopulateDeletedProblemValues_CurThreadID = -1;

	m_bNexERxNeedingAttentionLoaded = false;
	//TES 4/21/2009 - PLID 33376
	m_pSureScriptsCommDlg = NULL;

	// (a.walling 2009-07-10 09:31) - PLID 33644
	m_bNxServerBroadcastStagger = FALSE;
	m_nTableCheckerStaggerMin = 0;
	m_nTableCheckerStaggerMax = 0;

	m_nQueuedMessageTimerID = 0;
	
	// (j.jones 2014-08-19 10:21) - PLID 63412 - the batch limit also has a timer now,
	// the count is now a minmum
	m_nQueuedMessageBatchMinLimit = 25;
	m_nQueuedMessageBatchTimeLimitMS = 125;

	// (a.walling 2009-10-14 13:20) - PLID 35941 - The report engine destructor can result in the mainframe's ClearReportEngine being called again!
	m_bReportEngineClosing = false;

	//TES 1/12/2010 - PLID 36761 - Need to load our list of blocked security groups
	m_bBlockedGroupsValid = false;

	// (j.jones 2010-02-10 11:33) - PLID 37224 - we have not loaded EMR Image Stamps
	m_bEMRImageStampsLoaded = FALSE;

	// (j.dinatale 2010-10-05) - PLID 40604 - Need to setup NewCrop info
	GetNewCropSettings().SetIsProduction(GetNewCropIsProduction() == VARIANT_FALSE ? false : true);
	GetNewCropSettings().SetLicenseKey(g_pLicense->GetLicenseKey());
	GetNewCropSettings().SetSubRegistryKey(GetSubRegistryKey());

	// (j.dinatale 2011-03-29 15:57) - PLID 42982 - Initialize E-Statement Patient Selection dialog to null since we dont have one
	m_pEStatementPatientSelectDlg = NULL;

	// (j.gruber 2011-11-01 10:01) - PLID 46219 - modeless CCHITReportsDlg
	m_pCCHITReportsDlg = NULL;

	m_pdlgFirstAvailList = NULL; // (z.manning 2011-06-15 09:45) - PLID 44120

	m_parypEmrCodingGroupManager = NULL; // (z.manning 2011-07-05 12:42) - PLID 44421

	//(a.wilson 2012-1-10) PLID 47517 - initialize
	m_pOPOSBarcodeScannerThread = NULL;
	m_bOPOSBarcodeScannerInitComplete = false;
	m_pBarcodeSetupDlg = NULL;


	m_pRecallsNeedingAttentionDlg = NULL;	// (j.armen 2012-02-27 12:23) - PLID 48303
	m_pDrugInteractionDlg = NULL;	// (j.fouts 2012-9-5 3:28) - PLID 52482 - Need this dialog to be modeless

	// (s.dhole 07/23/2012) PLID 48693 modeless CEligibilityRequestDlg
	m_pEligibilityRequestDlg = NULL;
	//(j.camacho 2016-01-27) PLID 68001
	//m_pHL7ToBeBilledDlg = NULL;
	// (j.dinatale 2012-01-17 12:42) - PLID 47539
	m_EMNBillController.Initialize();
	m_HL7ToBeBilledDlg.Initialize();

	// (r.gonet 12/11/2012) - PLID 54117 - Create a new asynchronous CHL7Client
	// (z.manning 2013-05-20 11:21) - PLID 56777 - Renamed
	m_pHL7Client = new CHL7Client_Practice(true, boost::bind(&CMainFrame::HandleHL7Response, this, _1, _2));

	// (b.spivey, May 06, 2013) - PLID 56542 - Default this to OFF in case there is a problem with the message we post later. 
	m_bIsTopazConnected = FALSE;

	// (d.thompson 2014-02-04) - PLID 60638
	m_pDiagPreferenceSearchConfig = NULL;
	m_pDiagDualSearchConfig = NULL;

	// (d.singleton 2014-07-11 13:37) - PLID 62862 - create new modeless dialog that will import a lockbox payment file
	m_pLockboxPaymentImportDlg = NULL;

	// (r.goldschmidt 2014-10-08 16:18) - PLID 62644 - make eligibility review dlg and eligibility request detail dlg modeless
	m_pEligibilityReviewDlg = NULL;
	m_pEligibilityRequestDetailDlg = NULL;

	// (z.manning 2015-08-04 15:13) - PLID 67221
	m_pICCPDeviceManager = NULL;
	// (s.tullis 2016-01-28 17:46) - PLID 68090
	m_bTimerGoingErxNeedingAttentionMessages = FALSE;
	// (s.tullis 2016-01-28 17:46) - PLID 67965
	m_bTimerGoingRecieveRenewalMessages = FALSE;
	// (v.maida 2016-02-19 15:43) - PLID 68385 - Added m_bPracticeMinimized.
	m_bPracticeMinimized = FALSE;
}

CMainFrame::~CMainFrame()
{		
	// (a.walling 2009-07-10 09:31) - PLID 33644
	CleanupAllQueuedMessages();

	if (m_pMirrorDlg) {
		delete m_pMirrorDlg;
	}
	if (m_pDocToolBar) {
		delete m_pDocToolBar;
	}
	if (m_pUnitedDlg) {
		delete m_pUnitedDlg;
	}
	if (m_pWhatsNewDlg) {
		delete m_pWhatsNewDlg;
	}
	if (m_pNotificationDlg) {
		delete m_pNotificationDlg;
		m_pNotificationDlg = NULL;
	}
	if(m_pClipData) {
		delete m_pClipData;
		m_pClipData = NULL;
	}
	// (a.walling 2009-10-14 13:20) - PLID 35941 - The report engine destructor can result in the mainframe's ClearReportEngine being called again!
	ClearReportEngine();
	if(m_pdlgMessager) {
		delete m_pdlgMessager;
		m_pdlgMessager = NULL;
	}
	if(m_pdlgSendYakMessage) {
		delete m_pdlgSendYakMessage;
		m_pdlgSendYakMessage = NULL;
	}
	//DRT 6/7/2007 - PLID 25892 - Cleanup the opportunity list
	if(m_pdlgOpportunityList) {
		m_pdlgOpportunityList->DestroyWindow();
		delete m_pdlgOpportunityList;
		m_pdlgOpportunityList = NULL;
	}
	// (a.walling 2008-04-15 12:32) - PLID 25755
	if(m_pLabFollowupDlg) {
		m_pLabFollowupDlg->DestroyWindow();
		delete m_pLabFollowupDlg;
		m_pLabFollowupDlg = NULL;
	}

	// (j.armen 2012-02-27 12:23) - PLID 48303
	if(m_pRecallsNeedingAttentionDlg) {
		m_pRecallsNeedingAttentionDlg->DestroyWindow();
		delete m_pRecallsNeedingAttentionDlg;
		m_pRecallsNeedingAttentionDlg = NULL;
	}

	// (a.wilson 2013-01-08 11:28) - PLID 54535 - destroy the parent which will destroy the child as well.
	if (m_pPrescriptionQueueParentDlg) {
		m_pPrescriptionQueueParentDlg->DestroyWindow();
		m_pPrescriptionQueueParentDlg.reset();
	}

	// (j.fouts 2012-9-5 3:28) - PLID 52482 - Need this dialog to be modeless
	if(m_pDrugInteractionDlg) {
		m_pDrugInteractionDlg->DestroyWindow();
		delete m_pDrugInteractionDlg;
		m_pDrugInteractionDlg = NULL;
	}

	//TES 4/21/2009 - PLID 33376
	if(m_pSureScriptsCommDlg) {
		m_pSureScriptsCommDlg->DestroyWindow();
		delete m_pSureScriptsCommDlg;
		m_pSureScriptsCommDlg = NULL;
	}

	// (d.thompson 2014-02-04) - PLID 60638
	if(m_pDiagPreferenceSearchConfig) {
		delete m_pDiagPreferenceSearchConfig;
		m_pDiagPreferenceSearchConfig = NULL;
	}

	// (d.thompson 2014-02-07) - PLID 60638
	if(m_pDiagDualSearchConfig) {
		delete m_pDiagDualSearchConfig;
		m_pDiagDualSearchConfig = NULL;
	}

	// (z.manning 2015-08-04 15:05) - PLID 67221
	if (m_pICCPDeviceManager != NULL)
	{
		delete m_pICCPDeviceManager;
		m_pICCPDeviceManager = NULL;
	}

	// (j.camacho 2016-02-02) PLID 68001
	//if (m_pHL7ToBeBilledDlg)
	//{ 
	//	delete m_pHL7ToBeBilledDlg;
	//	m_pHL7ToBeBilledDlg = NULL;
	//}

	// (d.lange 2010-06-08 09:12) - PLID 38850 - Clean up the DevicePluginImportDlg when finished
	DeviceImport::GetMonitor().CleanUpDevicePluginImportDlg();

	//TES 6/23/2011 - PLID 44261 - Clean up any memory held by our cache of HL7 settings
	DestroyHL7SettingsCache();

	// (z.manning 2011-07-05 12:45) - PLID 44421
	ClearCachedEmrCodingGroups();

	// (a.walling 2009-10-09 16:42) - PLID 35911 - Set the main frame explicitly
	SetMainFrame(NULL);

	// (a.walling 2011-12-15 10:25) - PLID 40593 - Wait for NexPhoto preload thread
	WaitForPreloadNexPhotoLibs();

	// (r.gonet 12/11/2012) - PLID 54117 - Delete the client when done.
	if(m_pHL7Client) {
		delete m_pHL7Client;
		m_pHL7Client = NULL;
	}

	// (j.jones 2013-07-15 15:16) - PLID 57477 - this is a reference now
	delete &m_pEMRSearchDlg;
	// (r.gonet 03/182/2014) - PLID 60782 - This is a reference now
	delete &m_dlgHL7Export;
	// (r.gonet 2014-11-19) - PLID 64173 - FFA is now by reference
	delete &m_FirstAvailAppt;
}

// (a.walling 2010-01-27 13:17) - PLID 22496 - Reset the title bar text timer which checks for date changes
bool CMainFrame::ResetTitleBarTextTimer()
{
	// first get the time we reset
	static COleDateTime dtInitial = COleDateTime::GetCurrentTime();

	// need to check for a new date at the next hour.
	COleDateTime dtCurrent = COleDateTime::GetCurrentTime();
	const DWORD dwElapse = 
		(
			(59 - dtCurrent.GetMinute()) * 60	// minutes * 60
			+ (60 - dtCurrent.GetSecond())		// + seconds
			+ 2									// 2 second buffer just in case
		) * 1000; // in milliseconds

	SetTimer(IDT_CHECKDATE_TIMER, dwElapse, NULL);

	if (dtInitial.GetYear() == dtCurrent.GetYear() && dtInitial.GetMonth() == dtCurrent.GetMonth() && dtInitial.GetDay() == dtCurrent.GetDay()) {
		dtInitial = dtCurrent;
		return false;
	} else {
		dtInitial = dtCurrent;
		return true;
	}
}

// (a.walling 2010-01-27 13:17) - PLID 22496 - Return the formatted title bar text
CString CMainFrame::GenerateTitleBarText(CString strOverride /*= ""*/)
{
	ResetTitleBarTextTimer();

	// (c.haag 2004-06-08 10:16) - PLID 12884 - We need to have MedSpa Boutique appear here too
	CString strRegBase = GetSubRegistryKey();
	//CString strBoutique;
// (c.haag 2005-01-31 17:32) - PLID 15480 - No more MedSpa.
//	if(GetRemotePropertyInt("MedSpaBoutique", 0, 0, "<None>", true) == 1)
//		strBoutique = "& MedSpa Boutique ";

	//DRT 6/20/2007 - PLID 26406 - Changed to 2008.
	// (d.thompson 2009-01-08) - PLID 32656 - Changed to 2009.
	// (d.thompson 2009-04-30) - PLID 33707 - Changed to 2010
	// (z.manning 2010-01-11 14:12) - PLID 36427 - 2011
	// (b.savon 2011-7-14) - PLID 44566 - 2012
	// (j.dinatale 2013-02-12 11:55) - PLID 55075 - 2013
	// (a.walling 2010-01-27 13:00) - PLID 22496 - Modified the way this is generated to get rid of some ugly
	// (j.jones 2014-01-27 16:32) - PLID 60475 - 2014
	// (r.gonet 04/02/2014) - PLID 61628 - Change instances of "NexTech Practice 20NN" to "Nextech".
	CString strNewTitle = "Nextech";

	if (!strRegBase.IsEmpty()) {
		strNewTitle += FormatString(" (%s)", strRegBase);
	}

	// no one is logged in!
	if (GetCurrentUserHandle() == NULL) {
		strNewTitle += " - Please login";
	} else {
		if (!strOverride.IsEmpty()) {
			strNewTitle += FormatString(" - %s", strOverride);
		} else {
			strNewTitle += FormatString(" - %s at %s", GetCurrentUserName(), GetCurrentLocationName());
		}
	}

	// (a.walling 2010-01-27 13:00) - PLID 22496 - Now, include the date
	const bool bIncludeDate = true;
	if (bIncludeDate) {
		strNewTitle += FormatString(" - %s", FormatDateTimeForInterface(COleDateTime::GetCurrentTime(), 0, dtoDate, false));
	}

	//TES 3/2/2015 - PLID 64739 - We want to append " - DEBUG MODE" if NxDebug is attached. GetDebutModeActive() will return TRUE
	// if ANY debugger (like, say, Visual Studio) is attached, so only append it if Debug Mode has been enabled at some point in this session.
	if (theApp.GetDebugModeActive() && (theApp.GetDebugModeEnabled() || theApp.GetDebugModeChangedThisSession())) {
		strNewTitle += " - DEBUG MODE";
	}
	return strNewTitle;
}

//resets the text in the title bar
void CMainFrame::LoadTitleBarText(CString strOverride /* = ""*/)
{
	//DRT 8/1/03 - I copied this from the OnCreate.  For some reason, you have to do a SetTitle in the creation for it
	//		to load correctly.  But anytime after that, you have to call SetWindowText.
	// (b.cardillo 2004-01-05 13:51) - It turns out calling SetWindowText is really not enough because as soon as you 
	// switch modules the text that you had set gets set back to the original text.  So we DO have to call SetTitle, 
	// it's just that the title won't be visibly updated unless we set the flag telling MFC to update the title bar on 
	// the next idle loop, which we do by calling DelayUpdateFrameTitle().

	CString strNewTitle = GenerateTitleBarText();

	SetTitle(strNewTitle);
	AfxGetMainWnd()->SetWindowText(strNewTitle);
	DelayUpdateFrameTitle();
}

// (j.armen 2012-06-11 17:13) - PLID 48808 - Events to handle when a database connection was dropped 
//	and has been re-established
static void HandleDatabaseReconnect(NxAdo::Events::ConnectComplete& e)
{
	try {
		if (!e) return;

		CEmrPatientFrameWnd::HandleDatabaseReconnect();
		CEmrTemplateFrameWnd::HandleDatabaseReconnect();
	} NxCatchAll(__FUNCTION__);
}

//Used in the UpdatePendingAppointments stuff to store the results of a recordset.
struct PendingAppointment {
	long nApptID;
	long nPatientID;
	COleDateTime dtDate;
};

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	try {
		extern CPracticeApp theApp;
		if (CNxMDIFrameWnd::OnCreate(lpCreateStruct) == -1) {
			// (a.walling 2010-10-11 16:46) - PLID 36504
			ThrowNxException("Failed to create CNxMDIFrameWnd");
			return -1;
		}

		// (a.walling 2008-04-03 15:19) - PLID 29497
		// (a.walling 2010-02-09 09:49) - PLID 37277 - Use a default param
		m_bUseEnhancedNxColor = NxRegUtils::ReadLong("HKEY_CURRENT_USER\\Software\\NexTech\\NxColor\\Advanced\\EnhancedDrawing", TRUE);

		// (a.walling 2008-04-04 10:05) - PLID 29544 - Subclass our MDIClient so we can intercept paint and erase background messages
	#ifdef SubclassWindow
	#undef SubclassWindow
	#endif
		m_MDIClientWnd.SubclassWindow(m_hWndMDIClient);

	#ifndef NETWORK_ON
		InitPracYakker();
		InitNxServer();
	#endif

		// (z.manning 2009-08-25 11:08) - PLID 31944 - In debug mode, add an option to the system menu
		// to size Practice to 1024x768 for aid during interface development.
	#ifdef _DEBUG
		CMenu* pSysMenu = GetSystemMenu(FALSE);
		if (pSysMenu != NULL)
		{		
			pSysMenu->InsertMenu(0, MF_BYCOMMAND, DEBUGMENU_ID_SIZE_TO_1024_768, "Size Window to 1024x768");

			// (a.walling 2009-10-23 09:46) - PLID 36046 - Ability to dump all EMNDetail objects in memory with reference count history
	#ifdef WATCH_EMNDETAIL_REFCOUNTS
			pSysMenu->InsertMenu(0, MF_BYCOMMAND, DEBUGMENU_ID_DUMP_ALL_EMNDETAILS, "Dump All EMNDetails");
	#endif

			pSysMenu->InsertMenu(DEBUGMENU_ID_SIZE_TO_1024_768, MF_SEPARATOR|MF_BYCOMMAND);
		}
	#endif

		// (a.walling 2007-07-24 15:12) - PLID 26787 - Tell the property manager that the initial login is complete,
		// and to behave normally
		g_propManager.SetLoginComplete();

		// (c.haag 2005-11-14 15:02) - PLID 18091 - Assign the property manager table checker message window
		g_propManager.SetTableCheckerMsgWnd(GetSafeHwnd());

		// (a.walling 2007-07-24 12:49) - PLID 26787 - Cache all login properties
		CacheLoginProperties();

		// (a.walling 2008-05-08 13:43) - PLID 29963 - Get the gradients option now
		m_bDisplayGradients = GetRemotePropertyInt("DisplayGradients", TRUE, 0, GetCurrentUserName(), true);
		
		// (a.walling 2009-08-12 16:04) - PLID 35214
		HDC dc = ::GetDC(NULL);
		if (GetDeviceCaps(dc, BITSPIXEL) <= 8) {
			m_bDisplayGradients = FALSE;
		}
		::ReleaseDC(NULL, dc);
		
		// (a.walling 2008-04-21 13:26) - PLID 29731 - Get toolbar text preferences
		g_cnToolbarTextHeight = GetRemotePropertyInt("ToolbarTextHeight", 16, 0, GetCurrentUserName(), true);
		m_bToolbarText = g_cnToolbarTextHeight == 0 ? FALSE : TRUE;

		// (a.walling 2008-10-07 12:17) - PLID 31575 - Get toolbar border preference
		// (a.walling 2012-03-02 10:43) - PLID 48589 - No more toolbar borders
		/*g_bClassicToolbarBorders = GetRemotePropertyInt("ToolbarBorders", TRUE, 0, GetCurrentUserName(), true);
		m_bToolbarBorders = g_bClassicToolbarBorders;*/

		// Create all the toolbars
		CreateToolBars();
		UpdateToolBarButtons();

		// Create the auto-logoff dialog
		m_dlgAutoLogoff.Create(IDD_AUTOLOGOFF, GetDesktopWindow());
		if (IsWindow(m_dlgAutoLogoff.GetSafeHwnd()))
			m_dlgAutoLogoff.ResetInactivityTimer();

		//set the Practice title bar
		// (a.walling 2010-01-27 13:02) - PLID 22496 - Use the GenerateTitleBarText function rather than calculate it all out here
		SetTitle(GenerateTitleBarText());

		// (a.walling 2007-05-04 10:21) - PLID 4850 - Moved all user-specific code to HandleUserLogin
		HandleUserLogin();

		// Initialize communications with the barcode scanner
		// (j.jones 2013-05-15 13:42) - PLID 25479 - converted these settings to per user, per workstation,
		// pulling the defaults from their old per-workstation value
		bool bUseBarcodeScanner = (GetRemotePropertyInt("UseBarcodeScanner_UserWS", GetPropertyInt("UseBarcodeScanner", 0), 0, GetCurrentUserComputerName(), true) == 1);
		if (bUseBarcodeScanner)
		{
			CString strPort = GetRemotePropertyText("BarcodeScannerPort_UserWS", GetPropertyText("BarcodeScannerPort", "COM1:"), 0, GetCurrentUserComputerName(), true);
			Barcode_Open(GetSafeHwnd(), strPort);
		} else {
			Barcode_Close();
		}

		// (j.gruber 2007-07-17 11:30) - PLID 26710 - Initialize the pin pad if necessary
		//I changed this to just post a message
		// (a.walling 2007-08-03 09:17) - PLID 26899 - Check for license
		// (d.thompson 2010-09-02) - PLID 40371 - Any cc licensing satisfies
		if (g_pLicense && g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent)) {
			PostMessage(NXM_INITIALIZE_PINPAD);
		}	

		// (a.wetta 2007-05-24 10:26) - PLID 25960 - Make sure they also have the NexSpa license, if not, turn off the cash drawer
		if (!(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent))) {
			// Turn off the cash drawer
			SetPropertyInt("POSCashDrawer_UseDevice", FALSE);
		}

		// (a.wetta 2007-06-14 17:11) - PLID 25163 - Make sure they also have the Billing license, if not, turn off the receipt printer
		if (!(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent))) {
			// Turn off the receipt printer
			SetPropertyInt("POSPrinter_UseDevice", FALSE);
		}

		// (a.wetta 2007-06-05 11:25) - PLID 25947 - The OPOS MSR, Printer, and Cash Drawer devices are now initiated using a message so that if something
		// goes wrong during the initialization it will not cause the log in process to freeze.
		// (a.walling 2007-09-28 10:33) - PLID 27556 - Send OPOS_SUCCESS as the WPARAM so there will be no error message
		PostMessage(NXM_INITIATE_OPOSMSRDEVICE, OPOS_SUCCESS, 0);
		PostMessage(NXM_INITIATE_OPOSPRINTERDEVICE);
		PostMessage(NXM_INITIATE_OPOSCASHDRAWERDEVICE);
		//(a.wilson 2012-1-10) PLID 47517 - to begin the thread if necessary.
		PostMessage(NXM_INITIATE_OPOSBARCODESCANNER);

		// (b.spivey, May 03, 2013) - PLID 56542 - post the message to check connectivity.
		PostMessage(NXM_INITIATE_TOPAZSIGPADDEVICE);

		// (z.manning 2015-08-04 15:10) - PLID 67221 - Get credit card processing going (this function will check the license, etc.)
		EnsureICCPDeviceManager();

		// (r.gonet 2016-05-19 18:17) - NX-100695 - Listen for when the user connects or disconnects
		// locally or remotely.
		RdsApi::Instance().AddSessionChangedListener(GetSafeHwnd());
		RdsApi::Instance().EnableCaching(true);

		// Initialize communications with the caller ID
		if (GetPropertyInt("UseCallerID", 0))
		{
		//	if (!SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
			// (a.walling 2010-10-11 16:43) - PLID 36504 - AfxOleInit already does this
			/*
			if(!SUCCEEDED(CoInitialize(0)))
			{
				return 0;
			}
			*/

			// do all tapi initialization
			if (S_OK != InitializeTapi())
			{
				// (a.walling 2010-10-11 16:43) - PLID 36504 - Throw an exception rather than inexplicably return
				ThrowNxException("Failed to initialize TAPI");
			}
		}

		// (c.haag 2006-04-13 15:24) - PLID 20128 - Moving this to the top
	/*#ifndef NETWORK_ON
		try {
			InitPracYakker();
			g_hNxServerSocket = NxSocketUtils::Connect(GetSafeHwnd(), NxRegUtils::ReadString(GetRegistryBase() + "NxServerIP"));

			// (c.haag 2004-01-06 09:35) - Make sure NxServer associates our connection
			// with our database so table checker messages get filtered to the right places.
			CString strDefaultDatabase = (LPCTSTR)GetRemoteData()->GetDefaultDatabase();
			char* szDefaultDatabase = new char[strDefaultDatabase.GetLength()+1];
			strcpy(szDefaultDatabase, strDefaultDatabase);
			NxSocketUtils::Send(g_hNxServerSocket, PACKET_TYPE_DATABASE_NAME, (void*)szDefaultDatabase, strDefaultDatabase.GetLength()+1);
			delete szDefaultDatabase;
		}
		catch (...)
		{
			// (c.haag 2003-12-11 17:07) - We are already warned earlier that we can't connect
			// to nxserver back we connected within the scope of a single function call to check
			// backup information.
			Log("Could not connect to NxServer. Practice backups will not be detected.");
		}
	#endif*/

		// (a.walling 2012-12-04 16:12) - PLID 54037 - Device import notification tray icon
		DeviceImport::GetMonitor().GetTrayIcon().Init(DeviceImport::ID_TRAYICON, GetSafeHwnd(), theApp.LoadIcon(IDI_DEVICE_IMPORT), "NexTech Device Import");

		Mirror::ShowApplication(GetPropertyInt("MirrorShowSoftware", 0));

		// (a.walling 2008-09-09 17:23) - PLID 31334 - Initialize WIA event sink
		
		// (a.walling 2008-10-28 13:51) - PLID 31334 - Disabled for now
		/*
		try {
			
			HRESULT hr = m_pWIADeviceManager.CreateInstance("WIA.DeviceManager");

			if (SUCCEEDED(hr)) {
				m_pWIAEventSink = new NxWIA::CWIAEventSink;
				if (m_pWIAEventSink) {
					m_pWIAEventSink->EnsureSink(this, m_pWIADeviceManager);
				}

				HRESULT hr;
				hr = m_pWIADeviceManager->RegisterEvent(WIA::wiaEventDeviceConnected, WIA::wiaAnyDeviceID);
				if (SUCCEEDED(hr) && hr != S_FALSE) {
					m_arRegisteredWIAEvents.Add(WIA::wiaEventDeviceConnected);
				}

				//hr = m_pWIADeviceManager->RegisterEvent(WIA::wiaEventItemCreated, WIA::wiaAnyDeviceID);
				//if (SUCCEEDED(hr) && hr != S_FALSE) {
				//	m_arRegisteredWIAEvents.Add(WIA::wiaEventItemCreated);
				//}
				
				// (a.walling 2008-09-10 15:56) - PLID 31334 - Have not been able to test this yet
				//hr = m_pWIADeviceManager->RegisterEvent(WIA::wiaEventScanImage, WIA::wiaAnyDeviceID);
				//if (SUCCEEDED(hr) && hr != S_FALSE) {
				//	m_arRegisteredWIAEvents.Add(WIA::wiaEventScanImage);
				//}
			}
		} NxCatchAllCallIgnore({LogDetail("Failed to register for WIA Events");});
		*/

		// (j.armen 2012-06-11 17:13) - PLID 48808 - Register handler for detecting database reconnects
		GetRemoteConnection().Events()->OnConnectComplete += boost::bind(&::HandleDatabaseReconnect, _1);

	} NxCatchAll(__FUNCTION__);

	// (a.walling 2011-12-14 10:41) - PLID 40593 - only does something once, safe to call repeatedly
	CMainFrame::PreloadNexPhotoLibs();

	return 0;
}

// (a.wetta 2007-06-05 13:11) - PLID 25947 - Initiates the OPOS MSR Device, called by the NXM_INITIATE_OPOSMSRDEVICE message
LRESULT CMainFrame::OnInitiateOPOSMSRDevice(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2007-09-28 10:34) - PLID 27556 - MessageBox if we get an error
		if (wParam != OPOS_SUCCESS) {
			MessageBox(FormatString("Card Reader device open failed - %s", OPOS::GetMessage(wParam)), NULL, MB_OK|MB_ICONASTERISK);
			SetTimer(IDT_OPOSMSRDEVICE_INIT_TIMER, 0, NULL); // immediately fire the timer
			return 0;
		}
		// (a.walling 2007-05-04 10:23) - PLID 4850 - Only need to init if NULL.
		// (a.wetta 2007-03-15 17:37) - PLID 25234 - Initialize the MSR device if it is enabled
		// (a.wetta 2007-07-03 13:04) - PLID 26547 - Start the MSR device in its own thread
		if (m_pOPOSMSRThread == NULL && GetPropertyInt("MSR_UseDevice", 0, 0, TRUE) == emmOn) {
			m_pOPOSMSRThread = (COPOSMSRThread*)AfxBeginThread(RUNTIME_CLASS (COPOSMSRThread), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
			m_pOPOSMSRThread->m_bAutoDelete = TRUE;
			m_pOPOSMSRThread->m_hwndNotify = GetSafeHwnd();
			m_pOPOSMSRThread->m_bReturnInitCompleteMsg = TRUE;
			// (a.walling 2007-09-28 13:32) - PLID 26547 - send in the device name
			m_pOPOSMSRThread->m_strDeviceName = GetPropertyText("MSR_DeviceName", "", 0, TRUE);
			m_pOPOSMSRThread->ResumeThread();

			// Set a timer and if the device hasn't been initiated before it's fired, then there was probably a problem with the initialization
			SetTimer(IDT_OPOSMSRDEVICE_INIT_TIMER, 10000, NULL);
		}

	}NxCatchAll("Error in CMainFrame::OnInitiateOPOSMSRDevice");

	return 0;
}

// (a.wetta 2007-06-05 13:11) - PLID 25947 - Initiates the OPOS Printer Device, called by the NXM_INITIATE_OPOSPRINTERDEVICE message
LRESULT CMainFrame::OnInitiateOPOSPrinterDevice(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2007-05-04 10:23) - PLID 4850 - Only need to init if NULL.
		// (j.gruber 2007-04-24 14:48) - PLID 9802 - Initialize the POS Printer device if it is enabled
		// (a.wetta 2007-06-14 17:12) - PLID 25163 - Make sure they also have the Billing license
		if ((m_pPOSPrinterDevice == NULL) && GetPropertyInt("POSPrinter_UseDevice", 0, 0, TRUE) &&
			g_pLicense && g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) {
			m_pPOSPrinterDevice = new COPOSPrinterDevice(this);
		}

		if (m_pPOSPrinterDevice && !m_pPOSPrinterDevice->InitiatePOSPrinterDevice(GetPropertyText("POSPrinter_DeviceName", "", 0, TRUE))) {
			// It seems that this device name is not correct, so lets tell the user
			// (a.walling 2007-09-28 11:39) - Removed line breaks which was making this message look awful in Vista
			MessageBox("The receipt printer could not be initialized.  Make sure that the POS Printer settings are correct "
					   "by going to Tools->POS Tools->Receipt Printer Settings.  Also ensure that the device has been "
					   "correctly installed.", 
					   "Receipt Printer Error", MB_ICONWARNING|MB_OK);
			// Delete the pointer to the device because it has not been initiated
			// (a.walling 2011-03-23 13:13) - PLID 42931 - Don't delete it here...
			/*
			m_pPOSPrinterDevice->DestroyWindow();
			delete m_pPOSPrinterDevice;
			m_pPOSPrinterDevice = NULL;
			*/
		}
	}NxCatchAll("Error in CMainFrame::OnInitiateOPOSPrinterDevice");

	return 0;
}

// (a.walling 2011-03-21 17:32) - PLID 42931 - Use POSPrinterAccess
/*
COPOSPrinterDevice* CMainFrame::ClaimPOSPrinter() 
{
	//TES 12/6/2007 - PLID 28192 - If we don't currently have the POS Printer claimed, then claim it.  That allows us
	// to use it, and disallows all other applications from using it.
	if(m_pPOSPrinterDevice == NULL) {
		//We don't have a POS Printer.
		return NULL;
	}
	else {
		if(m_nPOSPrinterDeviceClaims > 0) {
			//We've already got it claimed, awesome.
			m_nPOSPrinterDeviceClaims++;
			return m_pPOSPrinterDevice;
		}
		else {
			if(!m_pPOSPrinterDevice->CheckStatus()) {
				return NULL;
			}
			if(m_pPOSPrinterDevice->ClaimPOSPrinter()) {
				m_nPOSPrinterDeviceClaims++;
				return m_pPOSPrinterDevice;
			}
			else {
				return NULL;
			}
		}
	}
}

// (a.walling 2011-03-21 17:32) - PLID 42931 - Use POSPrinterAccess
void CMainFrame::ReleasePOSPrinter()
{
	ASSERT(m_nPOSPrinterDeviceClaims > 0);
	m_nPOSPrinterDeviceClaims--;
	if(m_nPOSPrinterDeviceClaims == 0) {
		//TES 12/6/2007 - PLID 28192 - We don't have any part of our program needing the POS Printer any more, so
		// release it, that way third-party applications can use it.
		m_pPOSPrinterDevice->ReleasePOSPrinter();
	}
}
*/

bool CMainFrame::CheckPOSPrinter()
{
	//TES 12/6/2007 - PLID 28192 - Just check whether we successfully initialized the POS Printer
	if(m_pPOSPrinterDevice == NULL) {
		return false;
	}
	else {
		return true;
	}
}

// (a.wetta 2007-06-05 13:11) - PLID 25947 - Initiates the OPOS Cash Drawer Device, called by the NXM_INITIATE_OPOSCASHDRAWERDEVICE message
LRESULT CMainFrame::OnInitiateOPOSCashDrawerDevice(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2007-05-15 10:39) - PLID 9801 - Initialize the Cash Drawer service object and class
		// (a.wetta 2007-05-24 10:26) - PLID 25960 - Make sure they also have the NexSpa license
		if ((m_pPOSCashDrawerDevice == NULL) && GetPropertyInt("POSCashDrawer_UseDevice", 0, 0, TRUE) &&
			g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent)) {
			m_pPOSCashDrawerDevice = new COPOSCashDrawerDevice(this);

			// (a.walling 2007-10-04 14:54) - PLID 26058 - Throw an exception if we fail to create it.
			if (m_pPOSCashDrawerDevice == NULL) {
				ThrowNxException("Failed to create cash drawer device object!");
			}

			if (!m_pPOSCashDrawerDevice->InitiatePOSCashDrawerDevice(GetPropertyText("POSCashDrawer_DeviceName", "", 0, TRUE))) {
				// It seems that this device name is not correct, so lets tell the user
				// (a.walling 2007-09-28 11:38) - Removed line breaks which was making this look awful in Vista
				MessageBox("The cash drawer could not be initialized.  Make sure that the POS Cash Drawer settings are correct "
						   "by going to Tools->POS Tools->Cash Drawer Settings.  Also ensure that the device has been "
						   "correctly installed.", 
						   "Cash Drawer Printer Error", MB_ICONWARNING|MB_OK);
				// Delete the pointer to the device because it has not been initiated
				m_pPOSCashDrawerDevice->DestroyWindow();
				delete m_pPOSCashDrawerDevice;
				m_pPOSCashDrawerDevice = NULL;
			}		
		}
	}NxCatchAll("Error in CMainFrame::OnInitiateOPOSPrinterDevice");

	return 0;
}

namespace RecallUtils
{
	void UpdateRecalls(long nPatientID = -1);
}

// (a.walling 2007-05-04 10:21) - PLID 4850 - Handle all tasks associated with a user logging in
//
// (d.thompson 2014-03-07) - FYI!  Unfortunately this function is not called consistently.  Be aware of
//	what you put in here!  On the initial load of the program, this is called before any views/tabs are
//	created.  But if you switch users, that users default view and tab will be initialized BEFORE
//	the code in this function is called.  This means you cannot cache anything here that might be needed
//	by any tab to exist during initialization.
void CMainFrame::HandleUserLogin(BOOL bInitialLogin /*= TRUE*/)
{
	try {
		// (a.walling 2010-11-26 13:08) - PLID 40444 - Reset all module info
		g_Modules.ResetAll();

		if (!bInitialLogin) { // we are switching users
			#ifndef NETWORK_ON
				InitPracYakker();
			#endif

			// (a.walling 2007-07-24 12:50) - PLID 26787 - Cache common login properties
			CacheLoginProperties();

			if (IsWindow(m_dlgAutoLogoff.GetSafeHwnd()))
				m_dlgAutoLogoff.ResetInactivityTimer();

			// (a.walling 2007-05-29 12:31) - PLID 4850
			// Re-initializing toolbars moved to PostLogin so they can be updated before the modules are loaded
			
			UpdateToolBarButtons(false); // this is called with true (to requery) in PostLogin
			LoadTitleBarText(); // (a.walling 2007-05-04 10:22) - PLID 4850 - Updates location, user, etc in the title
		}

		// (c.haag 2006-11-27 12:15) - PLID 20772 - Create the first available appointment
		// list dialog
		// (e.lally 2007-01-02) PLID 23812 - We want the FFA to be a child of practice so that it will
		// always appear in front of practice when visible.
		// (z.manning 2011-06-15 09:42) - PLID 44120 - Don't create this until we need it.
		//if (!m_dlgFirstAvailList.Create(IDD_FIRST_AVAIL_LIST, this)) {
			// (a.walling 2010-10-11 16:41) - PLID 36504
		//	ThrowNxException("Unable to create FFA dialog!");
		//}
		
		if (!bInitialLogin && m_pdlgFirstAvailList != NULL && IsWindow(m_pdlgFirstAvailList->GetSafeHwnd())) {
			m_pdlgFirstAvailList->OnUserChanged(); // refresh for a checkbox if switching users
		}

		//set these variables once
		// (a.walling 2009-07-10 09:31) - PLID 33644
		m_bNxServerBroadcastStagger = GetRemotePropertyInt("NxServerBroadcastStagger", FALSE, 0, "<None>", true);	
		m_nTableCheckerStaggerMin = GetRemotePropertyInt("TableCheckerStaggerMin", 200, 0, "<None>", true);
		m_nTableCheckerStaggerMax = GetRemotePropertyInt("TableCheckerStaggerMax", 200, 0, "<None>", true); // * # of workstations

		// (j.jones 2014-08-19 10:21) - PLID 63412 - the batch limit also has a timer now,
		// the count is now a minmum
		m_nQueuedMessageBatchMinLimit = GetRemotePropertyInt("TableCheckerQueueBatchLimit", 25, 0, "<None>", true);
		m_nQueuedMessageBatchTimeLimitMS = GetRemotePropertyInt("TableCheckerQueueBatchTimeLimitMS", 125, 0, "<None>", true);

		// (d.thompson 2012-08-01) - PLID 51898 - Changed default to on and 3
		m_bCloseNthModule = GetRemotePropertyInt("EnableNthModuleClosing",1,0,GetCurrentUserName(),true) == 1;
		m_nMaxOpenModules = GetRemotePropertyInt("MaxModulesAllowedOpen",3,0,GetCurrentUserName(),true);

		//TODO:  This stuff (updating pending appointments,
		// and popping up the ToDo list) doesn't seem like it should be here.

		// (a.walling 2013-11-25 11:47) - PLID 60007 - At least update recalls when logging in
		RecallUtils::UpdateRecalls();

		//Past Pending Appointments
		if(GetRemotePropertyInt("EnablePendingUpdate", 0, 0, "<None>", true)) {
			//Also make sure that, if they have assigned a specific user, this is it.
			int nUser = GetRemotePropertyInt("UpdatePendingUser", -1, 0, "<None>", true);
			if(nUser == -1 || nUser == GetCurrentUserID()) {
				//Find out how many pending appointments there are.
				// (a.walling 2011-06-24 13:50) - PLID 29330 - Use the server time
				COleDateTime dtNow = GetRemoteServerTime();
				COleDateTime dtLastUpdate = GetRemotePropertyDateTime("LastPendingApptsUpdate", &dtNow, 0, "<None>", true);
				if(dtNow.Format("%Y-%m-%d") != dtLastUpdate.Format("%Y-%m-%d")) {
					//OK, it's been at least a day, are there any pending appointments?
					// (a.walling 2013-06-18 16:08) - PLID 57220 - This was the same query as the one that follows, basically, just ReturnsRecords for this one.
					// Instead we can simply re-use the same query for both purposes.
					// filter on StartTime which is the clustered index; filtering on EndTime was causing a full table scan. For safety's sake the StartTime is
					// one day previous, so the EndTime logic is unchanged. We don't support appointments that span midnight anyway.
					COleDateTime dtLastUpdatePreviousDay = dtLastUpdate - COleDateTimeSpan(1, 0, 0, 0);
					_RecordsetPtr rsAptStatuses = CreateParamRecordset(
						"SELECT ShowState, Count(ShowState) AS Total "
						"FROM AppointmentsT "
						"WHERE Status <> 4 "
						"AND StartTime >= {OLEDATETIME} AND StartTime <= {OLEDATETIME} "
						"AND EndTime > {OLEDATETIME} AND EndTime < {OLEDATETIME} "
						"GROUP BY ShowState"
						, AsDateNoTime(dtLastUpdatePreviousDay), AsDateNoTime(dtNow)
						, AsDateNoTime(dtLastUpdate), AsDateNoTime(dtNow)
					);
					if (!rsAptStatuses->eof) {
						//Yes, there are pending appointments.  Let's get all the numbers.
						int nPending = 0, nIn = 0, nOut = 0, nNoShow = 0, nTotal = 0;
						while(!rsAptStatuses->eof) {
							long nShowState = AdoFldLong(rsAptStatuses, "ShowState");
							long nCount = AdoFldLong(rsAptStatuses, "Total");
							nTotal += nCount;
							switch(nShowState) {
							case 0:
								nPending = nCount;
								break;
							case 1:
								nIn = nCount;
								break;
							case 2:
								nOut = nCount;
								break;
							case 3:
								nNoShow = nCount;
								break;
							}
							rsAptStatuses->MoveNext();
						}
						rsAptStatuses->Close();
						bool bUpdate = false;
						//DRT 1/29/03 - This was checking for !true, when it should be checking for true
						if(GetRemotePropertyInt("AlwaysUpdatePending", 0, 0, "<None>", true) && nPending > 0) {
							CString strMessage = "Since this update was run, the following appointments have been entered: \n";
							CString strNextLine;
							strNextLine.Format("%li Pending Appointments\n", nPending);
							strMessage += strNextLine;
							if(nIn > 0) {
								strNextLine.Format("%li In Appointments (these may be incorrect)\n", nIn);
								strMessage += strNextLine;
							}
							if(nOut > 0) {
								strNextLine.Format("%li Out Appointments\n", nOut);
								strMessage += strNextLine;
							}
							if(nNoShow > 0) {
								strNextLine.Format("%li NoShow Appointments\n", nNoShow);
								strMessage += strNextLine;
							}
							strMessage += "Would you like to update the Pending Appointments?";

							CUpdatePendingDlg dlg(this);
							dlg.m_strMessage = strMessage;
							int nReturn = dlg.DoModal();
							if(nReturn == IDOK) {
								bUpdate = true;
							}
						}
						else {
							if(nPending > 0) {
								bUpdate = true;
							}
						}
		 				if(bUpdate) {
							//Actually update them.
							
							//But how?
							int nUpdateType = GetRemotePropertyInt("UpdatePendingType", 0, 0, "<None>", true); //0=Ask, 1=Out, 2=NoShow
							if(nUpdateType == 0) {
								CUpdatePendingConfigDlg dlg(this);
								dlg.DoModal();
								nUpdateType = dlg.m_nUpdateType;
							}
							if(nUpdateType == 1) {//If it were still 0, that would mean they canceled out of the config dlg.
								CUpdatingPendingDlg dlg(this);
								dlg.Create(IDD_UPDATING_PENDING, GetDesktopWindow());
								dlg.ShowWindow(SW_SHOW);
								dlg.UpdateWindow();
								//Out
								//TES 2/9/2004: New plan: Get a recordset of all the ones we'll need to track, do a single 
								//batch update, then loop through and do the tracking.
								// (a.walling 2013-06-18 16:08) - PLID 57220 - Filter on StartTime here as well as parameterize
								_RecordsetPtr rsPendings = CreateParamRecordset(
									"SELECT ID, PatientID, Date FROM AppointmentsT "
									"WHERE ShowState = 0 AND Status <> 4 "
									"AND StartTime >= {OLEDATETIME} AND StartTime <= {OLEDATETIME} "
									"AND EndTime > {OLEDATETIME} AND EndTime < {OLEDATETIME} "
									"AND PatientID IN (SELECT PersonID FROM LaddersT WHERE LaddersT.ID IN "
									"(SELECT LadderID FROM StepsT WHERE StepsT.ID NOT IN (SELECT StepID FROM EventAppliesT) AND "
									"StepTemplateID IN (SELECT ID FROM StepTemplatesT WHERE Action IN ({CONST_INT},{CONST_INT}))))"
									, AsDateNoTime(dtLastUpdatePreviousDay), AsDateNoTime(dtNow)
									, AsDateNoTime(dtLastUpdate), AsDateNoTime(dtNow)									
									, PhaseTracking::PA_ActualAptCategory
									, PhaseTracking::PA_ActualAptType
								);
								FieldsPtr Fields = rsPendings->Fields;
								
								// (a.walling 2013-06-18 16:08) - PLID 57220 - Filter on StartTime here as well as parameterize
								ExecuteParamSql(
									"UPDATE AppointmentsT "
									"SET ShowState = 2 "
									"WHERE ShowState = 0 AND Status <> 4 "
									"AND StartTime >= {OLEDATETIME} AND StartTime <= {OLEDATETIME} "
									"AND EndTime > {OLEDATETIME} AND EndTime < {OLEDATETIME}"
									, AsDateNoTime(dtLastUpdatePreviousDay), AsDateNoTime(dtNow)
									, AsDateNoTime(dtLastUpdate), AsDateNoTime(dtNow)
								);
								// (c.haag 2010-01-12 15:36) - PLID 31157 - Auditing
								CUpdatePastPendingApptsDlg::AuditMassUpdate("Pending","Out", dtLastUpdate, dtNow);
								
								CArray<PendingAppointment,PendingAppointment> arPendings;
								while(!rsPendings->eof) {
									PendingAppointment pa;
									pa.nApptID = AdoFldLong(Fields, "ID");
									pa.nPatientID = AdoFldLong(Fields, "PatientID");
									pa.dtDate = AdoFldDateTime(Fields, "Date");
									arPendings.Add(pa);
									rsPendings->MoveNext();
								}
								rsPendings->Close();

								for(int i = 0; i < arPendings.GetSize(); i++) {
									dlg.SetProgressMessage(i, arPendings.GetSize());
									PendingAppointment pa = arPendings.GetAt(i);
									PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_ActualAppointment, pa.nPatientID, pa.dtDate, pa.nApptID, false);
								}
								dlg.DestroyWindow();
							}
							else if(nUpdateType == 2) {
								//NoShow

								// (c.haag 2009-08-06 11:37) - PLID 25943 - Ask the user about what to do with superbills for these appointments
								BOOL bVoidSuperbills = FALSE;
								// (z.manning 2011-04-01 15:26) - PLID 42973 - Converted where clause to a SQL fragment
								// (a.walling 2013-06-27 10:22) - PLID 57220 - also added StartTime filter here - EndTime predicate parameters were not using AsDateNoTime, modified to do so
								switch (NeedsToVoidNoShowSuperbills(CSqlFragment("ShowState = 0 AND Status <> 4 AND StartTime >= {OLEDATETIME} AND StartTime <= {OLEDATETIME} AND EndTime > {OLEDATETIME} AND EndTime < {OLEDATETIME}", AsDateNoTime(dtLastUpdatePreviousDay), AsDateNoTime(dtNow), AsDateNoTime(dtLastUpdate), AsDateNoTime(dtNow))))
								{
									case IDYES: // Yes, there are superbills to void
										bVoidSuperbills = TRUE;
										break;
									case IDNO: // Do not void superbills
										bVoidSuperbills = FALSE;
										break;
									case IDCANCEL: // User changed their mind
										return;
								}

								CUpdatingPendingDlg dlg(this);
								dlg.Create(IDD_UPDATING_PENDING, GetDesktopWindow());
								dlg.ShowWindow(SW_SHOW);
								dlg.UpdateWindow();
								if (bVoidSuperbills) {									
									// (a.walling 2013-06-18 16:08) - PLID 57220 - Filter on StartTime here as well as parameterize
									ExecuteParamSql(
										"UPDATE PrintedSuperbillsT "
										"SET Void = 1, VoidDate = GetDate(), VoidUser = {STRING} "
										"WHERE PrintedSuperbillsT.ReservationID IN ( "
											"SELECT ID FROM AppointmentsT "
											"WHERE ShowState = 0 AND Status <> 4 "
											"AND StartTime >= {OLEDATETIME} AND StartTime <= {OLEDATETIME} "
											"AND EndTime > {OLEDATETIME} AND EndTime < {OLEDATETIME}"
										") AND Void = 0"
										, GetCurrentUserName() 
										, AsDateNoTime(dtLastUpdatePreviousDay), AsDateNoTime(dtNow)
										, AsDateNoTime(dtLastUpdate), AsDateNoTime(dtNow)
									);
								}
								
								// (a.walling 2013-06-18 16:08) - PLID 57220 - Filter on StartTime here as well as parameterize
								ExecuteParamSql(
									"UPDATE AppointmentsT "
									"SET ShowState = 3, NoShowDate = GetDate() "
									"WHERE ShowState = 0 AND Status <> 4 "
									"AND StartTime >= {OLEDATETIME} AND StartTime <= {OLEDATETIME} "
									"AND EndTime > {OLEDATETIME} AND EndTime < {OLEDATETIME}"
									, AsDateNoTime(dtLastUpdatePreviousDay), AsDateNoTime(dtNow)
									, AsDateNoTime(dtLastUpdate), AsDateNoTime(dtNow)
								);
								// (c.haag 2010-01-12 15:36) - PLID 31157 - Auditing
								CUpdatePastPendingApptsDlg::AuditMassUpdate("Pending","NoShow", dtLastUpdate, dtNow);
								dlg.DestroyWindow();
							}
							//OK, the update's been run.
							// (a.walling 2011-06-24 13:50) - PLID 29330 - Use the server time
							SetRemotePropertyDateTime("LastPendingApptsUpdate", dtNow, 0, "<None>");
						}
					}
				} 
			}
		}

		// (j.jones 2005-03-04 15:48) - try to create To-Do alarms for Person License expirations
		if(IsSurgeryCenter(FALSE)) {
			UpdateASCLicenseToDos();
		}

		// (j.jones 2010-04-09 08:40) - PLID 14360 - moved the marketing productivity popup to PostLogin()

		//JMM 8/4/2005: PLID 16527 - How about reminding about them to Inactivate patients
		//first see if this is the correct user
		if (GetRemotePropertyInt("InactivatePatientsUser", -1, 0, "<None>", true) == GetCurrentUserID()) {

			//it is the right user!! now do they want it to popup?
			if (GetRemotePropertyInt("InactivatePatientsRemind", 0, 0, "<None>", true) != 0) {
				//they do!! they do!!  but how often?
				long nInterval = GetRemotePropertyInt("InactivatePatientsInterval", 3, 0, "<None>", true);
				/*1 = every day
				2 = every week
				3 = every month
				4 = every year*/
				//lets see when the last pop up was
				COleDateTime dtLastRemind = GetRemotePropertyDateTime("InactivatePatientsLastPopup", &COleDateTime(1899,12,30,0,0,0),0, "<None>", true);
				COleDateTime dtNow = COleDateTime::GetCurrentTime();
				switch (nInterval) {
					case 1:  //remind them everyday
						if(dtLastRemind.GetYear() != dtNow.GetYear() || dtLastRemind.GetMonth() != dtNow.GetMonth() || dtLastRemind.GetDay() != dtNow.GetDay()) {
							SetRemotePropertyDateTime("InactivatePatientsLastPopup", dtNow, 0, "<None>");
							PostMessage(WM_COMMAND, MAKEWPARAM(IDM_INACTIVATE_MULTI_PATS,0), NULL);
						}
					break;

					case 2: //weekly 
						{
							//lets try to get it to pop up on mondays
							COleDateTimeSpan ts = dtNow - dtLastRemind;

							//if its been 7 days, pop it up now
							if (ts.GetDays() >= 7) {

								//its been more than a week, pop it up!
								SetRemotePropertyDateTime("InactivatePatientsLastPopup", dtNow, 0, "<None>");
								PostMessage(WM_COMMAND, MAKEWPARAM(IDM_INACTIVATE_MULTI_PATS,0), NULL);
							}
							else {
								//if we have passed a weekend, pop it up
								long nLastDay, nDayNow;
								nLastDay = dtLastRemind.GetDayOfWeek();
								nDayNow = dtNow.GetDayOfWeek();

								if (nLastDay > nDayNow && nDayNow >= 1) {
									// that means that a new week has begun, so pop it up
									SetRemotePropertyDateTime("InactivatePatientsLastPopup", dtNow, 0, "<None>");
									PostMessage(WM_COMMAND, MAKEWPARAM(IDM_INACTIVATE_MULTI_PATS,0), NULL);
								}
							}

						}
					break;

					case 3: //month

						if(dtLastRemind.GetYear() != dtNow.GetYear() || dtLastRemind.GetMonth() != dtNow.GetMonth()) {
							SetRemotePropertyDateTime("InactivatePatientsLastPopup", dtNow, 0, "<None>");
							PostMessage(WM_COMMAND, MAKEWPARAM(IDM_INACTIVATE_MULTI_PATS,0), NULL);
						}
					break;

					case 4: //Year
						if(dtLastRemind.GetYear() != dtNow.GetYear()) {
							SetRemotePropertyDateTime("InactivatePatientsLastPopup", dtNow, 0, "<None>");
							PostMessage(WM_COMMAND, MAKEWPARAM(IDM_INACTIVATE_MULTI_PATS,0), NULL);
						}
					break;
				}//end switch
			}//end Remind if
		}//end User if

		// (a.walling 2007-06-19 18:04) - PLID 4850 - Moved report initialization to before the opening of modules	

		EnsureNotificationDlg();
		
		// (j.jones 2010-04-23 08:59) - PLID 11596 - added ability to disable the to-do alarm when entering Practice
		if(GetRemotePropertyInt("TodoDoNotOpenOnLogin", 0, 0, GetCurrentUserComputerName(), true) == 0) {
			//set the todo timer to popup now (only pops up if you have todos of course)
			SetToDoTimer(1);
		}
		else {
			// (j.dinatale 2012-10-22 18:00) - PLID  52393 - Move to per-user option
			long nDontRemind = GetRemotePropertyInt("DontRemind_User", -1, 0, GetCurrentUserName());
			if(nDontRemind == -1) {
				//fall back to legacy per-system/path behavior
				nDontRemind = GetPropertyInt("DontRemind", 0);
			}
			//they don't want the todo list to open immediately, so set the timer based on their current
			//remind settings - which might possibly not set a timer at all!
			if(nDontRemind == 0) {
				//they do want to be reminded eventually
				int nMinutes = GetRemotePropertyInt("LastTimeOption_User", -1, 0, GetCurrentUserName());
				if(nMinutes == -1)
				{
					//Legacy per machine/path option
					nMinutes = GetPropertyInt("LastTimeOption", 10);
				}
				SetToDoTimer(nMinutes * 60 * 1000);
			}
		}
		
		// (z.manning 2008-07-16 15:29) - PLID 30490 - We keep track of the IDs of any HL7 settings
		// group that exports to the schedule. This way we only need to load these once ever (plus
		// any time we get a table checker message that the settings have changed) rather than
		// loading them every single time an appointment is created, modified, or cancelled (which
		// is what patient exports do).
		LoadScheduleHL7GroupIDs();
		
		//TES 6/16/2008 - PLID 30394 - Check whether they have appointments needing allocations to be warned about.  This code
		// is copied from the SchedulingProductivityDlg code above.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(g_pLicense->HasCandAModule(CLicense::cflrSilent)
			&& GetRemotePropertyInt("RemindForApptsWithoutAllocations", 0, 0, GetCurrentUserName(), true)) {
			//Have we popped it up already today?
			COleDateTime dtLastPopup = GetRemotePropertyDateTime("LastApptsWithoutAllocationsPopup", &COleDateTime(1899,12,30,0,0,0),0, GetCurrentUserName(), true);
			COleDateTime dtNow = COleDateTime::GetCurrentTime();
			//We're assuming dtLastPopup isn't in the future.
			if(dtLastPopup.GetYear() != dtNow.GetYear() || dtLastPopup.GetMonth() != dtNow.GetMonth() || dtLastPopup.GetDay() != dtNow.GetDay()) {
				//Let's pop it up now.
				SetRemotePropertyDateTime("LastApptsWithoutAllocationsPopup", dtNow, 0, GetCurrentUserName());
				//TES 6/16/2008 - PLID 30394 - Does it even need to be popped up?
				if(InvUtils::DoAppointmentsWithoutAllocationsExist()) {
					//TES 6/16/2008 - PLID 30394 - Send TRUE for the bSilent parameter, it shouldn't ever be an issue, but
					// I just wanted to make sure they never got a message saying there were no appointments on startup.
					PostMessage(NXM_POPUP_APPTS_WO_ALLOCATIONS, (LPARAM)TRUE);
				}
			}
		}

		// (j.fouts 2013-01-28 09:02) - PLID 54472 - Refresh NexERx needing attn
		// (s.tullis 2016-01-28 17:46) - PLID 68090
        // (s.tullis 2016-01-28 17:46) - PLID 67965
		// added login flag
		RefreshNexERxNeedingAttention(TRUE);

	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2011-12-14 10:41) - PLID 40593 - uses a special minimal nexphototab to avoid the issues it has 
// with cleaning up itself and its activex controls
static UINT PreloadNexPhotoLibs_Thread(LPVOID pParam)
{
	OleInitialize(NULL);

	try {
		CWnd parentWnd;

		if (!parentWnd.CreateEx(WS_EX_CONTROLPARENT | WS_EX_TOOLWINDOW, AfxRegisterWndClass(0), "Preload NexPhoto", WS_POPUP | WS_DISABLED, CRect(0,0,0,0), NULL, 0)) {
			OleUninitialize();
			return GetLastError();
		}

		try {

			NxManagedWrapperLib::INxPhotoTabPtr pNxPhotoTab;

			HR(pNxPhotoTab.CreateInstance(__uuidof(NxManagedWrapperLib::NxPhotoTab)));

			pNxPhotoTab->CreatePreloadNexPhotoControl((OLE_HANDLE)parentWnd.GetSafeHwnd());

			::OutputDebugString("NexTech - NexPhoto libs successfully preloaded.\n");

			pNxPhotoTab->DestroyPreloadNexPhotoControl();

		} NxCatchAllIgnore();

		parentWnd.DestroyWindow();

	} NxCatchAllIgnore();

	OleUninitialize();

	return 0;
}

// (a.walling 2011-12-14 10:41) - PLID 40593 - Preloads NexPhoto and dependencies in a separate thread now
void CMainFrame::PreloadNexPhotoLibs() 
{
	// for simplicity I am ignoring the permission status of the current user

	if (g_bPreloadedNexPhotoLibs) {
		return;
	}

	g_bPreloadedNexPhotoLibs = true;

	// (a.walling 2014-04-25 16:00) - VS2013 - We definitely don't support Win2k any longer

	if (NxRegUtils::ReadLong(CString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\") + CString("DisableNexPhotoPreLoad"), FALSE)) {
		return;
	}

	if (!g_pLicense->CheckForLicense(CLicense::lcNexPhoto , CLicense::cflrSilent)) {
		return;
	}

	g_pPreloadNexPhotoThread = AfxBeginThread(PreloadNexPhotoLibs_Thread, NULL, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);
	g_pPreloadNexPhotoThread->m_bAutoDelete = FALSE;
	g_pPreloadNexPhotoThread->ResumeThread();


	return;
}

#define MAIN_TOOLBAR_BTN_COUNT 16

// (a.walling 2008-04-22 10:20) - PLID 29673 - Moved the EMR icon, added a seperator
static TBBUTTON tbMain[MAIN_TOOLBAR_BTN_COUNT] = 
{
	{0, ID_PATIENTS_MODULE,			TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL}, 
	{1, ID_SCHEDULER_MODULE,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{2, ID_LETTER_WRITING_MODULE,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{3, ID_CONTACTS_MODULE,			TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{4, ID_MARKETING_MODULE,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{5, ID_INVENTORY_MODULE,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{6, ID_FINANCIAL_MODULE,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{7,	ID_SURGERYCENTER_MODULE,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{12,ID_TB_NEW_EMR,				TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{8, ID_REPORTS_MODULE,			TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{9, ID_ADMINISTRATOR_MODULE,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	// (d.thompson 2009-11-16) - PLID 36134
	{14,ID_LINKS_MODULE,			TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{-1, 0,							0,				 TBSTYLE_SEP,	 0, 0},
	{10,ID_NEW_PATIENT_BTN,			TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{11,ID_FIRST_AVAILABLE_APPT,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	// (z.manning, 03/02/2007) - PLID 25044 - The CareCredit button is not always loaded,
	// so if you ever add another button, make sure you handle this in CreateToolBars().
	{13,ID_CARECREDIT_BTN,			TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
}; 

static TBBUTTON tbGui[4] = 
{
	{0, IDM_UPDATE_VIEW,			TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL}, 
	// (z.manning 2009-03-04 12:16) - PLID 33104 - Renamed to ID_MANUAL_TOOLBAR_BTN
	{1, ID_MANUAL_TOOLBAR_BTN,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{2, ID_FILE_PRINT_PREVIEW,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{3, ID_FILE_PRINT,				TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
}; 

static TBBUTTON tbContacts[5] = 
{
	{0, ID_BUTTON32868,			TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL}, 
	{1, ID_PREV_CONTACT_BTN,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{2, 11111,					TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{3, ID_NEXT_CONTACT_BTN,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
	{4, ID_NEW_CONTACT,			TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, NULL},
}; 

void CMainFrame::EnableDockingEx(DWORD dwDockStyle)
{
	// (a.walling 2008-04-14 11:12) - PLID 29642 - This function allows us to specify a derived DockBar class


    // must be CBRS_ALIGN_XXX or CBRS_FLOAT_MULTI only
    ASSERT((dwDockStyle & ~(CBRS_ALIGN_ANY|CBRS_FLOAT_MULTI)) == 0);

    // define class that will be used to hold floating 
    //  toolbars or dialogbars
    m_pFloatingFrameClass = RUNTIME_CLASS(CMiniDockFrameWnd);

    // define style that every docking bar will have
    CNxDockBar *pDock;
    DWORD dwStyle=WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | 
                        WS_CLIPCHILDREN;

    // create docking bars in the correct order
    pDock = new CNxDockBar();
    pDock->Create(this,dwStyle|CBRS_TOP, AFX_IDW_DOCKBAR_TOP);
    pDock = new CNxDockBar();
    pDock->Create(this,dwStyle|CBRS_BOTTOM, AFX_IDW_DOCKBAR_BOTTOM);
	/*
    pDock = new CNxDockBar();
    pDock->Create(this,dwStyle|CBRS_LEFT, AFX_IDW_DOCKBAR_LEFT);
    pDock = new CNxDockBar();
    pDock->Create(this,dwStyle|CBRS_RIGHT, AFX_IDW_DOCKBAR_RIGHT);
	*/

	/*
	// must be CBRS_ALIGN_XXX or CBRS_FLOAT_MULTI only
	ASSERT((dwDockStyle & ~(CBRS_ALIGN_ANY|CBRS_FLOAT_MULTI)) == 0);

	m_pFloatingFrameClass = RUNTIME_CLASS(CMiniDockFrameWnd);
	for (int i = 0; i < 4; i++)
	{
		if (dwDockBarMap[i][1] & dwDockStyle & CBRS_ALIGN_ANY)
		{
			CDockBar* pDock = (CDockBar*)GetControlBar(dwDockBarMap[i][0]);
			if (pDock == NULL)
			{
				pDock = new CDockBar;
				if (!pDock->Create(this,
					WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_CHILD|WS_VISIBLE |
						dwDockBarMap[i][1], dwDockBarMap[i][0]))
				{
					AfxThrowResourceException();
				}
			}
		}
	}
	*/
}

long CMainFrame::CreateToolBars()
{
	// (a.walling 2008-10-03 09:50) - PLID 31575 - geez, you can actually set the borders here. How did I miss this??
	RECT rect;
	rect.top = rect.left = 0;
	rect.bottom = rect.right = 0;

//////////////////////////////////////////
/*	if (!m_wndToolBar.CreateEx(this))
	{	TRACE0("Failed to create toolbar\n");
		return -1;
	}
	if (-1 == m_wndToolBar.GetToolBarCtrl().AddBitmap(5, IDR_MAINFRAME))
	{	TRACE0("Failed to create toolbar\n");
		return -1;
	}

	m_wndToolBar.GetToolBarCtrl().AddButtons(5, tbButtons); 
	if (!m_wndToolBar.GetToolBarCtrl().SetBitmapSize(CSize(48, 48)))
	{	TRACE0("Failed to create toolbar\n");
		return -1;
	}
	m_wndToolBar.GetToolBarCtrl().SetButtonSize(CSize(48, 54));

	if (!m_wndDlgBar.Create(this, IDR_MAINFRAME, 
		CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;		// fail to create
	}
*/
///////////////////////////////////////////

	// (a.walling 2008-08-01 17:58) - PLID 29673? - Removed CBRS_TOOLTIPS which seemed to be the cause of the duplicate tips
	// (a.walling 2008-08-20 15:29) - PLID 31056 - Turns out the best thing to do is remove TBSTYLE_TOOLTIPS and use CBRS_TOOLTIPS instead!
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
			WS_CHILD | WS_VISIBLE | CBRS_SIZE_DYNAMIC | CBRS_TOP | CBRS_FLYBY | CBRS_TOOLTIPS, 
			rect, AFX_IDW_TOOLBAR))
		return -1;

	
	// (a.walling 2008-04-21 13:21) - PLID 29731 - Respect the toolbar text height
	if (!m_wndToolBar.GetToolBarCtrl().SetButtonSize(CSize(48, 48 + g_cnToolbarTextHeight)))
		return -1;
	//must add +7, +6 for a margin
	if (!m_wndToolBar.GetToolBarCtrl().SetBitmapSize(CSize(41, 42 + g_cnToolbarTextHeight)))
		return -1;

	int nMainToolbarButtonCount = MAIN_TOOLBAR_BTN_COUNT;
	// (z.manning, 03/02/2007) - PLID 25044 - Don't load the CareCredit button if its license status is expired.
	// This was done to give people a way to take it away as some clients flat-out could not stand it.
	try {
		if(tbMain[nMainToolbarButtonCount - 1].idCommand == ID_CARECREDIT_BTN) {
			if(NxCareCredit::GetCareCreditLicenseStatus() == cclsExpired) {
				nMainToolbarButtonCount--;
			}
		}
		else {
			ASSERT(FALSE);
		}
	}NxCatchAllIgnore();

	if (-1 == m_wndToolBar.GetToolBarCtrl().AddBitmap(nMainToolbarButtonCount, IDR_MAINFRAME))
		return -1;

	if (!m_wndToolBar.GetToolBarCtrl().AddButtons(nMainToolbarButtonCount, tbMain))
		return -1;

	m_wndToolBar.ModifyStyleEx(0, WS_EX_TRANSPARENT);

/////////////////////////////////////////////////
	// (a.walling 2008-08-01 17:58) - PLID 29673? - Removed CBRS_TOOLTIPS which seemed to be the cause of the duplicate tips
	// (a.walling 2008-08-20 15:29) - PLID 31056 - Turns out the best thing to do is remove TBSTYLE_TOOLTIPS and use CBRS_TOOLTIPS instead!
	if (!m_GUIToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
			WS_CHILD | WS_VISIBLE | CBRS_SIZE_DYNAMIC | CBRS_TOP | CBRS_FLYBY | CBRS_TOOLTIPS, 
			rect, AFX_IDW_TOOLBAR))
	return -1;

	// (a.walling 2008-04-21 13:21) - PLID 29731 - Respect the toolbar text height
	if (!m_GUIToolBar.GetToolBarCtrl().SetButtonSize(CSize(48, 48 + g_cnToolbarTextHeight)))
		return -1;

	if (!m_GUIToolBar.GetToolBarCtrl().SetBitmapSize(CSize(41, 42 + g_cnToolbarTextHeight)))
		return -1;

	if (-1 == m_GUIToolBar.GetToolBarCtrl().AddBitmap(4, IDR_GUI_TOOLBAR))
		return -1;

	if (!m_GUIToolBar.GetToolBarCtrl().AddButtons(4, tbGui))
		return - 1;

	m_GUIToolBar.ModifyStyleEx(0, WS_EX_TRANSPARENT);

	if (IsScreen640x480())
	{
		m_wndToolBar.ShowWindow(SW_HIDE);
		m_GUIToolBar.ShowWindow(SW_HIDE);
	}

/////////////////////////////////////////////////
	
	// (a.walling 2008-08-20 15:29) - PLID 31056 - Turns out the best thing to do is remove TBSTYLE_TOOLTIPS and use CBRS_TOOLTIPS instead!
	// (a.walling 2008-08-20 15:30) - PLID 29642 - Need to remove FLAT style to prevent drawing separators
	if (!m_patToolBar.CreateEx(this, TBSTYLE_TRANSPARENT,
			WS_CHILD | WS_VISIBLE | CBRS_SIZE_FIXED | CBRS_TOP | CBRS_FLYBY | CBRS_TOOLTIPS, 
			rect, IDW_PATIENTS_TOOLBAR) ||
		!m_patToolBar.LoadToolBar(IDR_PATIENT_TOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_patToolBar.ModifyStyleEx(0, WS_EX_TRANSPARENT);

/////////////////////////////////////////////////
	// (a.walling 2008-08-20 15:29) - PLID 31056 - Turns out the best thing to do is remove TBSTYLE_TOOLTIPS and use CBRS_TOOLTIPS instead!
	// (a.walling 2008-08-20 15:30) - PLID 29642 - Need to remove FLAT style to prevent drawing separators
	if (!m_contactToolBar.CreateEx(this, TBSTYLE_TRANSPARENT,
			WS_CHILD | WS_VISIBLE | /*CBRS_SIZE_FIXED*/CBRS_SIZE_DYNAMIC | CBRS_TOP | CBRS_FLYBY | CBRS_TOOLTIPS, 
			rect, AFX_IDW_TOOLBAR/*IDW_CONTACTS_TOOLBAR*/))
	{
		TRACE0("Failed to create contact toolbar\n");
		return -1;      // fail to create
	}

	if (!m_contactToolBar.GetToolBarCtrl().SetButtonSize(CSize(44, 44)))
		return -1;

	if (!m_contactToolBar.GetToolBarCtrl().SetBitmapSize(CSize(30, 30)))
		return -1;

	if (-1 == m_contactToolBar.GetToolBarCtrl().AddBitmap(5, IDR_CONTACT_TOOLBAR))
		return -1;

	if (!m_contactToolBar.GetToolBarCtrl().AddButtons(5, tbContacts))
		return - 1;

	m_contactToolBar.ModifyStyleEx(0, WS_EX_TRANSPARENT);


/////////////////////////////////////////////////

	m_pDocToolBar = new CDocBar;
	if (!m_pDocToolBar->Create(this, IDD_MARKET_BAR, 
			WS_CHILD | WS_VISIBLE | CBRS_SIZE_FIXED | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY, 
			IDW_DOC_TOOLBAR))
	{
		TRACE0("Failed to create doc toolbar\n");
		return -1;      // fail to create
	}
	m_pDocToolBar->ModifyStyleEx(0, WS_EX_TRANSPARENT);
/*	if (!m_dateToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | TBSTYLE_TRANSPARENT,
			WS_CHILD | WS_VISIBLE | CBRS_SIZE_FIXED | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY, 
			rect, IDW_DATE_TOOLBAR) ||
		!m_dateToolBar.LoadToolBar(IDR_DATE_TOOLBAR))
	{
		TRACE0("Failed to create date toolbar\n");
		return -1;      // fail to create
	}*/


//	m_NewPatient = false;

	/*if (!m_wndMenuBar.CreateEx(this, WS_CHILD | WS_VISIBLE | CBRS_TOP
        | CBRS_SIZE_DYNAMIC) ||
        !m_wndMenuBar.LoadMenuBar(IDR_MAINFRAME))
    {
        TRACE0("Failed to create menubar\n");
        return -1;      // failed to create
    }*/

	if (!m_patToolBar.CreateSearchButtons())
	{
		TRACE0("Failed to create search buttons in toolbar\n");
		return -1;      // fail to create
	}

	if (!m_patToolBar.CreateComboBox())
	{
		TRACE0("Failed to create combobox in toolbar\n");
		return -1;      // fail to create
	}

	if (!m_patToolBar.CreateRightOfComboButtons())
	{
		TRACE0("Failed to create Right Of combobox in toolbar\n");
		return -1;      // fail to create
	}

	
   if (!m_contactToolBar.CreateComboBox())
	{
		TRACE0("Failed to create combobox in toolbar\n");
		return -1;      // fail to create
	}
	if (!m_pDocToolBar->CreateComboBox())
	{
		TRACE0("Failed to create combobox in toolbar\n");
		return -1;      // fail to create
	}
	
/*	if (!m_dateToolBar.CreateDateCombos())
	{
		TRACE0("Failed to create date combo's in toolbar\n");
		return -1;		//fail to create
	}*/

	m_Search = 3;
	m_Include = 1;

	//m_wndMenuBar.EnableDockingEx(CBRS_ALIGN_ANY);
    EnableDockingEx(CBRS_ALIGN_ANY);
    //DockControlBar(&m_wndMenuBar);

	m_wndToolBar.EnableDocking(CBRS_ALIGN_BOTTOM | CBRS_ALIGN_TOP);
	m_GUIToolBar.EnableDocking(CBRS_ALIGN_BOTTOM | CBRS_ALIGN_TOP);
	m_patToolBar.EnableDocking(CBRS_ALIGN_BOTTOM | CBRS_ALIGN_TOP);
	m_contactToolBar.EnableDocking(CBRS_ALIGN_BOTTOM | CBRS_ALIGN_TOP);
	m_pDocToolBar->EnableDocking(CBRS_ALIGN_BOTTOM | CBRS_ALIGN_TOP);
//	m_dateToolBar.EnableDocking(CBRS_ALIGN_BOTTOM | CBRS_ALIGN_TOP);

	//EnableDocking(CBRS_ALIGN_ANY);

	return (0);
}

void CMainFrame::DockControlBarLeftOf(CToolBar* Bar,CToolBar* LeftOf)
{
	if (Bar->GetSafeHwnd() && LeftOf->GetSafeHwnd()) {
		CRect rect;
		DWORD dw;
		UINT n;

		// get MFC to adjust the dimensions of all docked ToolBars
		// so that GetWindowRect will be accurate
		RecalcLayout();
		LeftOf->GetWindowRect(&rect);
		rect.OffsetRect(1,0);
		dw=LeftOf->GetBarStyle();
		n = 0;
		n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
		n = (dw&CBRS_ALIGN_BOTTOM && n==0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
		n = (dw&CBRS_ALIGN_LEFT && n==0) ? AFX_IDW_DOCKBAR_LEFT : n;
		n = (dw&CBRS_ALIGN_RIGHT && n==0) ? AFX_IDW_DOCKBAR_RIGHT : n;

		// When we take the default parameters on rect, DockControlBar will dock
		// each Toolbar on a seperate line.  By calculating a rectangle, we in effect
		// are simulating a Toolbar being dragged to that location and docked.
		DockControlBar(Bar,n,&rect);
		Invalidate(FALSE);
	}
}

void CMainFrame::DockControlBarBelow(CControlBar* Bar, CControlBar* BelowMe)
{
	if (Bar->GetSafeHwnd() && BelowMe->GetSafeHwnd()) {
		CRect rect;
		DWORD dw;
		UINT n;

		// get MFC to adjust the dimensions of all docked ToolBars
		// so that GetWindowRect will be accurate
		try
		{
			RecalcLayout();
		}
		catch (CException *e)
		{
			e->ReportError();
		}
		catch (...)
		{
			return;
		}

		BelowMe->GetWindowRect(&rect);

		// (a.walling 2008-04-21 13:21) - PLID 29731 - Respect the toolbar text height
		rect.OffsetRect(0,32 + g_cnToolbarTextHeight);
		dw=BelowMe->GetBarStyle();
		n = 0;
		n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
		n = (dw&CBRS_ALIGN_BOTTOM && n==0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
		n = (dw&CBRS_ALIGN_LEFT && n==0) ? AFX_IDW_DOCKBAR_LEFT : n;
		n = (dw&CBRS_ALIGN_RIGHT && n==0) ? AFX_IDW_DOCKBAR_RIGHT : n;

		// When we take the default parameters on rect, DockControlBar will dock
		// each Toolbar on a seperate line.  By calculating a rectangle, we in effect
		// are simulating a Toolbar being dragged to that location and docked.
		DockControlBar(Bar, n ,&rect);
		Invalidate(FALSE);
	}
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		| WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE;

	return CNxMDIFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CNxMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CNxMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
void CMainFrame::OnPatientsModule()
{
	if (CheckCurrentUserPermissions(bioPatientsModule, sptView))
	{
		// (a.walling 2008-05-07 15:20) - PLID 29951 - If the user is already in the NexEMR
		// tab of the patients module, then our module is already active. However, we should do
		// _something_ in response to clicking the module icon. So we'll load the default tab,
		// and if it is the NexEMR tab, we'll send them to General 1.
		CChildFrame *pActiveFrame = GetActiveViewFrame();
		if (pActiveFrame)
		{
			if (pActiveFrame->IsOfType(PATIENT_MODULE_NAME))
			{
				// Patient view should only be the active view when we are in the NexEMR tab. Otherwise the icon
				// would be 'disabled'.

				CNxTabView* pActiveView = GetActiveView();

				// (z.manning 2015-03-13 12:31) - NX-100432 - The NexEMR icon may open the dashboard tab so account for that here
				PatientsModule::Tab eEmrTab = GetPrimaryEmrTab();
				if (pActiveView && pActiveView->GetActiveTab() == eEmrTab)
				{
					// the default tab has already been resolved when the view was first activated
					if (pActiveView->m_defaultTab == eEmrTab) {
						// dang, this is their default tab already! Send them to general 1.
						pActiveView->SetActiveTab(PatientsModule::General1Tab);
					}
					else {
						pActiveView->SetActiveTab(pActiveView->m_defaultTab);
					}
				}

				// however, if somehow the active tab is not the NexEMRTab, just handle the default
			}
		}

		OpenModule(PATIENT_MODULE_NAME);
		CNxTabView* pView = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
		// this will cause the patient list to be dropped once we are finished opening the module
		if (pView){
			((CPatientView*)pView)->NeedToDropPatientList(true);
		}
	}
}

BOOL CMainFrame::IsPatientBarVisible()
{
	return m_patToolBar.IsWindowVisible();
}

//#define ShowControlBar(pbar, bShow, bDelay)		(pbar)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
void CMainFrame::ShowControlBar( CControlBar* pBar, BOOL bShow, BOOL bDelay)
{
	if (pBar->GetSafeHwnd()) {
		pBar->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	}
}


void CMainFrame::ShowPatientBar(BOOL bShow)
{
	ShowControlBar(&m_patToolBar, bShow, FALSE);
}
/*
void CMainFrame::ShowDateBar(BOOL bShow)
{
	ShowControlBar( &m_dateToolBar, bShow, FALSE);
}*/

void CMainFrame::ShowContactBar(BOOL bShow)
{
	ShowControlBar(&m_contactToolBar, bShow, FALSE);
}

void CMainFrame::ShowDoctorBar(BOOL bShow)
{
	ShowControlBar(m_pDocToolBar, bShow, FALSE);
}

BOOL CMainFrame::IsStandardBarVisible()
{
	return m_wndToolBar.IsWindowVisible();
}

BOOL CMainFrame::IsContactsBarVisible()
{
	return m_contactToolBar.IsWindowVisible();
}

void CMainFrame::ShowStandardBar(BOOL bShow)
{
	ShowControlBar(&m_wndToolBar, bShow, FALSE);
}

void CMainFrame::PreExit()
{
	int nCount = m_pView.GetSize();
	CView *pView;
	for (int i=0; i<nCount; i++) {
		// Loop through all open views, tell them we're about to close.
		pView = (CView *)m_pView[i];
		pView->SendMessage(NXM_PRE_CLOSE);
	}
}

// (a.walling 2007-05-04 10:27) - PLID 4850 - Changed some prompts when bSwitchUser is true.
bool CMainFrame::CheckAllowExit(BOOL bSwitchUser /*= FALSE*/)
{
	if(m_bIsReportRunning) {
		MessageBox(bSwitchUser ? "You cannot switch users while a report is running." : "You cannot close Practice while a report is running.");
		return false;
	}

	// (b.savon 2011-09-23 12:10) - PLID 44954 - Inform the user that they cannot close/switch user in Practice
	//											 while there are files still processing in threads.
	if( !DeviceImport::GetMonitor().IsPluginProcessComplete() ){
		MessageBox(bSwitchUser ? "You cannot switch users while processing device plugin files." : "You cannot close Practice while processing device plugin files.",
								 "Device Import Files Processing...", MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	// (a.walling 2009-12-22 17:39) - PLID 7002 - Maintain only one instance of a bill
	if (IsBillingModuleOpen(true)) {
		return false;
	}

	// (c.haag 2010-06-28 15:11) - PLID 39392 - Check if the NexPhoto import window is open
	if (IsNexPhotoImportFormOpen()) {
		MessageBox(bSwitchUser ? "You cannot switch users while the NexPhoto import window is open." : "You cannot close Practice while the NexPhoto import window is open.", NULL, MB_OK|MB_ICONEXCLAMATION);
		// Bring the import window to the front
		ShowNexPhotoImportForm(TRUE);
		return false;
	}

	// (c.haag 2006-03-09 17:40) - PLID 19208 - This is not necessary
//#pragma TODO("(t.schneider 1/13/2006) PLID 19208 - Implement this (if needed) in the new EMR.")
	/*for(int i = 0; i < m_arOpenEMRGroups.GetSize(); i++) {
		if(m_arOpenEMRGroups.GetAt(i)->GetSafeHwnd()) {
			if(AC_CANNOT_CLOSE == m_arOpenEMRGroups.GetAt(i)->SendMessage(NXM_ALLOW_CLOSE)) {
				return false;
			}
		}
	}*/

	// (b.cardillo 2006-06-13 18:41) - PLID 14292 - Check if any floating PIC containers are open.
	if (!m_lstpPicContainerDlgs.IsEmpty()) {
		// The user is not allowed to close Practice when there are PICs open
		POSITION pos = m_lstpPicContainerDlgs.GetHeadPosition();
		while (pos) {
			CPicContainerDlg *pPicContainer = m_lstpPicContainerDlgs.GetNext(pos);
			if (IsWindow(pPicContainer->GetSafeHwnd()) && pPicContainer->IsWindowVisible()) {
				// Notify the user
				CString strNotify;
				strNotify.Format("You still have at least one Procedure Information Center window open.  Please close all PIC windows before attempting to %s",
					bSwitchUser ? "switch users." : "close Practice.");
				MessageBox(strNotify, NULL, MB_OK|MB_ICONEXCLAMATION);
				// Bring the window up if it's minimized
				WINDOWPLACEMENT wp;
				wp.length = sizeof(WINDOWPLACEMENT);
				if (pPicContainer->GetWindowPlacement(&wp)) {
					if (pPicContainer->IsIconic()) {
						if (wp.flags & WPF_RESTORETOMAXIMIZED) {
							wp.showCmd = SW_MAXIMIZE;
						} else {
							wp.showCmd = SW_RESTORE;
						}
						pPicContainer->SetWindowPlacement(&wp);
					}
				}
				// Bring the window to the foreground so the user is sure to see it
				pPicContainer->SetForegroundWindow();
				// Break out of the loop and this function by telling our caller that it cannot 
				// close Practice at this time
				return false;
			}
		}
	}

	// (c.haag 2013-02-01) - PLID 54612 - Check if any existing custom preview layout editor windows are open
	extern std::vector<CEMRCustomPreviewLayoutsMDIFrame*> g_allCustomPreviewLayoutFrameWnds;
	foreach (CEMRCustomPreviewLayoutsMDIFrame* pFrameWnd, g_allCustomPreviewLayoutFrameWnds) 
	{
		if (IsWindow(pFrameWnd->GetSafeHwnd()) && pFrameWnd->IsWindowVisible()) 
		{
			CString strNotify;
			strNotify.Format("You still have at least one EMR Custom Preview Layout editor window open.  Please close all editor windows before attempting to %s",
				bSwitchUser ? "switch users." : "close Practice.");
			MessageBox(strNotify, NULL, MB_OK|MB_ICONEXCLAMATION);
			pFrameWnd->SetForegroundWindow();
			return false;
		}
	}

	// (c.haag 2010-07-15 17:23) - PLID 34338 - Check if any existing labs are open
	if (!m_lstpLabEntryDlgs.IsEmpty()) {
		POSITION pos = m_lstpLabEntryDlgs.GetHeadPosition();
		while (pos) {
			CLabEntryDlg* pDlg = m_lstpLabEntryDlgs.GetNext(pos);
			if (IsWindow(pDlg->GetSafeHwnd()) && pDlg->IsWindowVisible()) {
				// Notify the user
				CString strNotify;
				strNotify.Format("You still have at least one Lab Entry window open. Please close all Lab Entry windows before attempting to %s",
					bSwitchUser ? "switch users." : "close Practice.");
				MessageBox(strNotify, NULL, MB_OK|MB_ICONEXCLAMATION);
				// Bring the window up if it's minimized
				BringDialogToFront(pDlg);
				// Break out of the loop and this function by telling our caller that it cannot 
				// close Practice at this time
				return false;
			}
		}
	}

	// (c.haag 2010-11-15 13:12) - PLID 41124 - Check if any VisionWeb orders are open
	if (!m_lstpVisionWebOrderDlgs.IsEmpty()) {
		POSITION pos = m_lstpVisionWebOrderDlgs.GetHeadPosition();
		while (pos) {
			CVisionWebOrderDlg* pDlg = m_lstpVisionWebOrderDlgs.GetNext(pos);
			if (IsWindow(pDlg->GetSafeHwnd()) && pDlg->IsWindowVisible()) {
				// Notify the user
				CString strNotify;
				strNotify.Format("You still have at least one VisionWeb order window open. Please close all order windows before attempting to %s",
					bSwitchUser ? "switch users." : "close Practice.");
				MessageBox(strNotify, NULL, MB_OK|MB_ICONEXCLAMATION);
				// Bring the window up if it's minimized
				BringDialogToFront(pDlg);
				// Break out of the loop and this function by telling our caller that it cannot 
				// close Practice at this time
				return false;
			}
		}
	}

	// (j.dinatale 2012-03-21 13:40) - PLID 49079 - check before the quit!
	if (!m_lstpContactLensOrderForms.IsEmpty()) {
		POSITION pos = m_lstpContactLensOrderForms.GetHeadPosition();
		while (pos) {
			CContactLensOrderForm* pDlg = m_lstpContactLensOrderForms.GetNext(pos);
			if (IsWindow(pDlg->GetSafeHwnd()) && pDlg->IsWindowVisible()) {
				// Notify the user
				CString strNotify;
				strNotify.Format("You still have at least one Contact Lens order window open. Please close all order windows before attempting to %s",
					bSwitchUser ? "switch users." : "close Practice.");
				MessageBox(strNotify, NULL, MB_OK|MB_ICONEXCLAMATION);
				// Bring the window up if it's minimized
				BringDialogToFront(pDlg);
				// Break out of the loop and this function by telling our caller that it cannot 
				// close Practice at this time
				return false;
			}
		}
	}

	// (j.jones 2006-10-02 10:05) - PLID 22604 - check the Room Manager dlg
	// (a.walling 2007-05-04 09:11) - PLID 25907 - Included in the check for all modeless dialogs
	// (a.walling 2007-05-04 12:38) - I was pondering the situation of the ToDo alarm dialog, and decided
	// NOT to include it in this check. See the PL item for a bit more discussion. Basically, the ToDo alarms
	// pop up automatically, and users are already in the habit of minimizing it.
	// (d.moore 2007-09-27) - PLID 23861 - Added the NexForm editor to the list of managed dialogs.
	// (z.manning 2011-06-24 12:09) - PLID 37932 - We no longer pointlessly require the room manager to be manually closed.

	// To add a new dialog to this check, simply put a reference
	// to the window in arModelessWnds and its description in arModelessNames.

	CWnd* arModelessWnds[] = {
		(CWnd*)&m_pActiveEMRDlg,
		(CWnd*)&m_pEMRLockManagerDlg,
		(CWnd*)&m_pEMRSearchDlg,
		(CWnd*)&m_EMNToBeBilledDlg, // (a.walling 2010-04-26 13:18) - PLID 38364
		(CWnd*)&m_HL7ToBeBilledDlg,
		(CWnd*)&m_NexFormEditorDlg,
		// (j.jones 2008-07-17 08:51) - PLID 30730 - added a modeless EMR Problem List
		(CWnd*)m_pEMRProblemListDlg,
		// (j.jones 2008-10-21 17:41) - PLID 14411 - added EMR Analysis as a modeless dialog
		(CWnd*)m_pEMRAnalysisDlg,
		// (d.lange 2010-06-07 23:47) - PLID 38850 - added DevicePluginImportDlg as a modeless dialog
		(CWnd*)DeviceImport::GetMonitor().GetImportDlg(),
		(CWnd*)CAudioRecordDlg::GetCurrentInstance(), // (a.walling 2010-04-14 15:26) - PLID 36821
		(CWnd*)m_pEOBDlg // (j.jones 2012-05-25 10:55) - PLID 44367 - added E-Remittance as modeless
	};

	LPCTSTR arModelessNames[] = {
		"Active EMR",
		"EMR Lock Manager",
		"EMR Search",
		"EMNs To Be Billed",
		"Intellechart Encounters To Be Billed",
		"Procedure Form Contents Editor",
		"EMR Problem List",
		"EMR Analysis",
		"Device Import",
		"Audio Recording",
		"Electronic Remittance / EOB Processing" // (j.jones 2012-05-25 10:55) - PLID 44367 - added E-Remittance as modeless
	};
	
	const long nModelessWnds = sizeof(arModelessWnds) / sizeof(CWnd*);

	CString strWindows;
	CArray<CWnd*, CWnd*> arWindowsToPop;

	for (int i = 0; i < nModelessWnds; i++) {
		CWnd* pWnd = arModelessWnds[i];
		if (pWnd && pWnd->GetSafeHwnd()) {
			if (IsWindow(pWnd->GetSafeHwnd()) && pWnd->IsWindowVisible()) {
				strWindows += " - " + CString(arModelessNames[i]) + "\r\n";
				arWindowsToPop.Add(pWnd);
			}
		}
	}

	if (arWindowsToPop.GetSize() > 0) {
		// (a.walling 2007-05-04 09:26) - PLID 25907 - Prompt the user
		{
			CString strPlural = arWindowsToPop.GetSize() > 1 ? "s" : "";
			CString strPluralThis = arWindowsToPop.GetSize() > 1 ? "these" : "this";
			CString strAction = bSwitchUser ? "switch users" : "close Practice";

			CString strNotify = FormatString("You still have the following window%s open:\r\n\r\n%s\r\n"
				"Please review and close %s window%s before attempting to %s.",
				strPlural, strWindows, strPluralThis, strPlural, strAction);
			MessageBox(strNotify, NULL, MB_OK|MB_ICONEXCLAMATION);
		}

		// (a.walling 2007-05-04 09:26) - PLID 25907 - Now bring them to the front, reverse order.
		CWnd* pWnd = NULL;
		for (int j = arWindowsToPop.GetSize() - 1; j >= 0; j--) {
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);

			pWnd = arWindowsToPop.GetAt(j);

			// Bring the window up if it's minimized
			pWnd->GetWindowPlacement(&wp);
			if (pWnd->IsIconic()) {
				if (wp.flags & WPF_RESTORETOMAXIMIZED) {
					wp.showCmd = SW_MAXIMIZE;
				} else {
					wp.showCmd = SW_RESTORE;
				}
				pWnd->SetWindowPlacement(&wp);
			}

			pWnd->BringWindowToTop(); // bring all the windows to the top of the zorder, but only set foreground on our last one.
		}

		if (pWnd)
			pWnd->SetForegroundWindow(); // bring the last window to the foreground (also recieves input focus)

		// Break out of the loop and this function by telling our caller that it cannot 
		// close Practice at this time
		return false;
	}

	int nCount = m_pView.GetSize();
	CView *pView;
	for (i=0; i<nCount; i++) {
		// Loop through all open views, asking whether we can close or not
		pView = (CView *)m_pView[i];
		switch (pView->SendMessage(NXM_ALLOW_CLOSE)) {
		// If anyone cannot close, the program can't close
		case AC_CANNOT_CLOSE:
			return false;
			break;
		case AC_CAN_CLOSE:
		default:
			break;
		}
	}
	// (c.haag 2003-07-14 12:25) - If we made it to here, all open views
	// allowed us to close, so we are allowed to close. Lets ask the user
	// if it's ok.
	// (d.thompson 2012-08-01) - PLID 51898 - Changed default to Yes
	if (bSwitchUser || GetRemotePropertyInt("PracticePromptOnClose", 1, 0, GetCurrentUserName(), true))
	{
		if (IDNO == MsgBox(MB_YESNO | MB_ICONEXCLAMATION, bSwitchUser ? "Are you sure you wish to close all open modules and log in as another user?" : "Are you sure you wish to close NexTech Practice?"))
			return false;
	}
	return true;
}

// CH 4/5/01 - Central handler for barcode messages
void CMainFrame::OnBarcodeEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	int nCount = m_pView.GetSize();
	//CView *pView;
	BSTR bstr;

	switch (message)
	{
		case WM_BARCODE_OPEN_FAILED:
			MsgBox("Practice could not connect to the barcode scanner. Please have your hardware administrator verify that the Port, Parity, and Stop Bit settings are correct.");
			break;

		case WM_BARCODE_SCAN:
/*			for (int i=0; i<nCount; i++) {
				// Loop through all open views
				pView = (CView *)m_pView[i];
				switch (pView->SendMessage(message, wParam, lParam)) {
				case 1:
					bstr = (BSTR)lParam;
					SysFreeString(bstr);
					return;
				case 0:
				default:
					break;
				}
			}*/
			//(a.wilson 2012-1-13) PLID 47517 - changed because dialog was made into a pointer.
			if (m_pBarcodeSetupDlg && m_pBarcodeSetupDlg->GetSafeHwnd() && m_pBarcodeSetupDlg->IsWindowVisible())
			{
				m_pBarcodeSetupDlg->SendMessage(message, wParam, lParam);
			}
			//DRT 11/7/2007 - PLID 28020 - This dialog now uses the RegisterForBarcode functionality like everything else.  No
			//	sense having 2 cases for the same dialog.
/*			else if (m_dlgProductItems.GetSafeHwnd() && m_dlgProductItems.IsWindowVisible())
			{
				m_dlgProductItems.SendMessage(message, wParam, lParam);
			}
*/			else {
				//DRT 4/6/2004 - PLID 11801 - The current architecture is to keep member variables
				//	for everything that wants a barcode message.  This isn't totally bad, because
				//	some things (like the the scanner setup) should prevent other windows from
				//	receiving messages.  However, as more things require the barcode scanner, 
				//	we do not want to have dozens of member variables here for it.  This code will
				//	loop through all windows that told us they want the message and send it to them.
				//We also send a message to the active view if nothing was sent to one of the registered windows.
				bool bSent = false;

				//DRT 11/7/2007 - PLID 28020 - 1 more important change.  Previously it looped over .GetSize(), which meant that
				//	if you added a dialog while in the process of handling a barcode scan, that same barcode would be sent to
				//	the new dialog immediately.  I checked around and I don't see that anyone was relying on that functionality
				//	(seems a bug to me), so I'm just having it loop over all the known windows, and anything new will not get
				//	this specific code until the next scan.

				//DRT 11/30/2007 - PLID 28252 - Revision #3 (? - I lost count).  The new plan is that only one window should
				//	ever get a barcode scan message when the user scans a physical code.  Windows will still register themselves
				//	for barcode messages, and if none are interested, we will fall back to the active view.  I did consider 
				//	removing the Register requirement altogether, but this would fall apart if you had a floating modeless
				//	window and the view wanted the scan.
				//
				//Usage:  If you are on a tab, just make sure that the view is properly sending barcode messages along, and 
				//	implement a handler for WM_BARCODE_SCAN.  All views should really just blindly send it to the active sheet, 
				//	there's no reason to require we hardcode every possibility.  If the sheet doesn't want it, it can be ignored.
				//	You really should not be calling RegisterForBarcodeScan on a tab.
				//
				//	If you are on a dialog that pops up, you need to call RegisterForBarcodeScan(this), and the corresponding
				//	UnregisterForBarcodeScan(this) when done.  When you do that, your dialog will only get the barcode scan
				//	messages when it is the active window (GetForegroundWindow())
				//
				CWnd *pWndActive = GetForegroundWindow();

				long nWindows = m_aryBarcodeWindows.GetSize();
				for(int i = 0; i < nWindows; i++) {
					CWnd* pWnd = m_aryBarcodeWindows.GetAt(i);
					// (a.walling 2008-02-21 09:10) - PLID 29043 - Check if it is a valid window before calling IsWindowVisible.
					// Also compare window equivalency based on HWNDs rather than CWnd pointers, although in most cases they should
					// be the same.
#ifdef _DEBUG
					CString strWindowText = m_saBarcodeWindowText[i];
#endif
					_ASSERTE(::IsWindow(pWnd->GetSafeHwnd()));

					if(pWnd && ::IsWindow(pWnd->GetSafeHwnd()) && pWnd->IsWindowVisible() && ( (pWnd->GetSafeHwnd() == pWndActive->GetSafeHwnd()) || (pWnd == pWndActive) ) ) {
#ifdef _DEBUG
						TRACE("* Sending WM_BARCODE_SCAN to pWnd(0x%08x) HWND(0x%08x) \"%s\"\n", pWnd, pWnd->GetSafeHwnd(), strWindowText);
#endif
						pWnd->SendMessage(message, wParam, lParam);
						bSent = true;
					}
				}

				//If we didn't send anything above, we send a message to the active view.  We can't
				//	just always send it, because the view might have already registered, or be passing
				//	code to someone else.
				//DRT 11/30/2007 - PLID 28252 - We will only pass the message to the view if this is the 
				//	active window.  For a long time we've been able to cause weird behavior by getting a modal
				//	dialog up (anything from activities, for example), then scanning, where the active tab
				//	forces another dialog to pop up.
				//e.lally 2007-12-20 - PLID 28252 - We have to at least temporarily comment out this last
				//  expression because it does not account for the BillingDlg so scans do not work there.
				//  Best we can tell, this change does not negatively affect the rest of the barcode scanning,
				//  but it has not been proven that this item still works as intended.
				// (a.walling 2007-12-20 17:37) - PLID 28252 - The whole purpose of this was to prevent issues with
				//  other dialogs popping up in front of other dialogs, effectively ruining modality and causing 
				//  undefined behaviour. Even for a temporary fix, we should fix the actual problem, not this part.
				//  And so, I've reverted this back, and fixed the Billing Module itself, which is where the real
				//  problem was. (it was only registering as waiting for barcode events when functioning as a quote)
				if(!bSent && GetActiveView() && pWndActive == this) {
					TRACE("* Fallback: sending WM_BARCODE_SCAN to active view...\n");
					GetActiveView()->SendMessage(message, wParam, lParam);
				}
			}

			bstr = (BSTR)lParam;
			SysFreeString(bstr);
			break;

		case WM_BARCODE_OVERFLOW:
			// TODO: Error msg
			bstr = (BSTR)lParam;
			SysFreeString(bstr);
			break;

		case WM_ENQUEUE_BARCODE:
			// (j.jones 2008-12-23 13:30) - PLID 32545 - Barcode_Thread now posts WM_ENQUEUE_BARCODE
			// instead of directly posting WM_BARCODE_SCAN, we will queue this code and OnIdle()
			// will handle posting the queue.

			TRACE("MainFrm received WM_ENQUEUE_BARCODE, adding to queue.\n");

			bstr = (BSTR)lParam;
			_variant_t var(bstr);
			CString strBarcode = VarString(var);
			m_arystrQueuedBarcodes.Add(strBarcode);

			SysFreeString(bstr);
			break;
	}
}

// (j.gruber 2007-07-17 13:06) - PLID 26710 - Handle Pin Pad Messages
void CMainFrame::OnPinPadDeviceEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {

		case WM_PINPAD_MESSAGE_READ_TRACK_DONE:
		case WM_PINPAD_MESSAGE_INTERAC_DEBIT_DONE:

			if (GetActiveWindow() && GetActiveWindow() != GetMainFrame()) {
				GetActiveWindow()->SendMessage(message, wParam, lParam);
			}
			else if (GetActiveView()) {
				GetActiveView()->SendMessage(message, wParam, lParam);
			}
		break;

		case WM_PINPAD_OPEN_FAILED:
			MsgBox("Practice could not connect to the pin pad.  Please make sure your settings are correct.");
		break;

		case NXM_INITIALIZE_PINPAD:
			if (GetPropertyInt("CCProcessingUsingPinPad", 0, 0, TRUE)) {
				PinPadUtils::InitializePinPad(GetSafeHwnd(), GetPropertyText("CCProcessingPinPadComPort", "COM1", 0, TRUE));
				if (!PinPadUtils::SendInitMessage()) {
					MsgBox("Practice could not connect to the pin pad.  Please make sure your settings are correct.");
					PinPadUtils::PinPadClose();
					SetPropertyInt("CCProcessingUsingPinPad", 0, 0);
				}
			}
			else {
				PinPadUtils::PinPadClose();
			}
		break;
	}
			
}

// (a.wetta 2007-03-15 17:29) - PLID 25234 - Central handler for all messages coming from the MSR device
void CMainFrame::OnOPOSMSRDeviceEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_MSR_DATA_EVENT:
		case WM_MSR_STATUS_INFO:
			// (a.wetta 2007-03-19 17:01) - PLID 24983 - Make sure that the message isn't sent back to the main frame
			if (GetActiveWindow() && GetActiveWindow() != GetMainFrame()) {
				GetActiveWindow()->SendMessage(message, wParam, lParam);
			}
			else if (GetActiveView()) {
				// If no window took the message, send it to the active view
				GetActiveView()->SendMessage(message, wParam, lParam);
			}
			break;
		case WM_MSR_INIT_COMPLETE:
			// (a.wetta 2007-07-05 09:55) - PLID 26547 - The MSR device is done initiating
			m_bOPOSMSRDeviceInitComplete = TRUE;

			long nStatus = *((long*)wParam);
			if (nStatus == MSR_DEVICE_ERROR) {
				// (a.walling 2007-09-28 10:46) - PLID 27556 - Include more error info
				CString strExtended;
				if (lParam != OPOS_SUCCESS) {
					strExtended.Format("\r\n\r\nError: %s", OPOS::GetMessage(lParam));
				}

				// It seems that the device could not initiate, so lets tell the user
				MessageBox(FormatString("The magnetic strip reader device could not be initialized.  Make sure that the MSR settings are correct "
					   "by going to Tools->POS Tools->Magnetic Strip Reader Settings.  Ensure that the device has been "
					   "correctly installed and that nothing else is currently using the MSR device.%s", strExtended), 
					   "Magnetic Strip Reader Device Error", MB_ICONWARNING|MB_OK);
			}
			break;
	}
}

#define LOG_IF_TOO_SLOW_BEFORE(obj, alt)				\
	DWORD __dwBefore_##obj##__##alt##__ = GetTickCount();

#define LOG_IF_TOO_SLOW_AFTER(obj, alt, toolong, msg)	\
	DWORD __dwDuration_##obj##__##alt##__ = GetTickCount() - __dwBefore_##obj##__##alt##__; if (__dwDuration_##obj##__##alt##__ > 2000) { \
	Log("PERFORMANCE WARNING: " #obj "(" #alt ") took too long (%lu ms):  %s", __dwDuration_##obj##__##alt##__, msg); }

// (a.walling 2009-07-13 08:56) - PLID 33644 - Performance warning class, similar to LOG_IF_TOO_SLOW_BEFORE/AFTER
class CPerformanceWarning
{
public:
	CPerformanceWarning(LPCTSTR szName, long nTooLong)
		: m_strName(szName), m_nTooLong(nTooLong)
	{
		m_nTicks = GetTickCount();
	};
	
	~CPerformanceWarning()
	{
		long nPostTicks = GetTickCount();
		
		if ((nPostTicks - m_nTicks) >= m_nTooLong) {
			CString strLog;
			strLog.Format("PERFORMANCE WARNING: '%s' took too long (%lu ms)", m_strName, (nPostTicks - m_nTicks));
			TRACE("%s\n", strLog);
			Log("%s", strLog);
		}
	};

protected:
	CString m_strName;
	long m_nTooLong;

private:
	long m_nTicks;
};

void CMainFrame::OnClose() 
{
	// this will close all of our documents, and call PreExit which sends NXM_PRE_EXIT to all open views.
	// this also will not prompt if m_bForciblyClosing.
	if (!TryCloseAll())
		return;

	BeginWaitCursor();
	try//just in case
	{
		// (a.walling 2007-05-04 10:28) - PLID 4850 - Moved all this user-specific code to HandleUserLogout
		CClosingDlg dlg(this);
		HandleUserLogout(FALSE, &dlg); // this will not close the user handle if we are not switching users
		// (a.walling 2007-05-04 10:28) - PLID 4850 - Now proceed destroying system/global members

		LOG_IF_TOO_SLOW_BEFORE(CMainFrame__OnClose, 1);		
		
		// (a.walling 2008-10-28 13:52) - PLID 31334 - Disabled for now
		/*
		// (a.walling 2008-09-10 14:51) - PLID 31334 - Close event sink
		try {
			if (m_pWIAEventSink) {
				m_pWIAEventSink->CleanUp();
				delete m_pWIAEventSink;
			}
		} catch(...) {

		}

		// (a.walling 2008-09-10 14:52) - PLID 31334 - Unregister events
		if (m_pWIADeviceManager) {
			for (int i = 0; i < m_arRegisteredWIAEvents.GetSize(); i++) {
				try {
					m_pWIADeviceManager->UnregisterEvent(_bstr_t(m_arRegisteredWIAEvents[i]), WIA::wiaAnyDeviceID);
				} catch(...) {

				}
			}

			m_arRegisteredWIAEvents.RemoveAll();
			try {
				m_pWIADeviceManager = NULL;
			} catch (...) {}
		}
		*/

		// (b.savon 2011-09-23 12:10) - PLID 44954 - We verified the threads are not running previously
		//											 Now, let's cleanup the threads.
		DeviceImport::GetMonitor().CleanupPluginProcessThreads();

#ifdef _DEBUG
		// (a.walling 2008-02-21 13:05) - PLID 29043 - Warn if any windows are still registered for barcodes
		// (a.walling 2008-05-22 13:13) - Just fixed a typo
		if (m_aryBarcodeWindows.GetSize() > 0) {
			CString strWarning = "WARNING: The following windows are still registered to receive barcode messages! Please contact a developer.\n\n";

			for (int i = 0; i < m_aryBarcodeWindows.GetSize(); i++) {
				CString str;
				str.Format("pWnd(0x%08x) \"%s\"\n", m_aryBarcodeWindows[i], m_saBarcodeWindowText[i]);
				strWarning += str;
			}

			LogDetail(strWarning);

			AfxMessageBox(strWarning);
		}
#endif
		
		DeviceImport::GetMonitor().Close();
		
		// (a.walling 2009-10-23 09:46) - PLID 36046 - Dump all leaked EMNDetail objects in memory with reference count history
#ifdef WATCH_EMNDETAIL_REFCOUNTS
		_ASSERTE(g_EMNDetailManager.GetCount() == 0);
		g_EMNDetailManager.DumpAll();
#endif

		// Try one more time to delete any other stragglers
		// (a.walling 2007-07-25 11:55) - PLID 26261
		CleanupEMRTempFiles(TRUE);

		// Now that as much as possible is closed, try one last time to delete queued files
		DeleteQueuedFiles(TRUE);

		LOG_IF_TOO_SLOW_AFTER(CMainFrame__OnClose, 1, 2000, "DeleteQueuedFilesAndCleanupEMRTemp");
		LOG_IF_TOO_SLOW_BEFORE(CMainFrame__OnClose, 2);

		// (r.gonet 2016-05-19 18:17) - NX-100695 - Stop listening for when the user connects or disconnects
		// locally or remotely.
		RdsApi::Instance().RemoveSessionChangedListener(GetSafeHwnd());

		// (a.walling 2007-05-15 10:41) - PLID 9801 - Close the Cash Drawer Device
		try {
			if (m_pPOSCashDrawerDevice) {
				m_pPOSCashDrawerDevice->ClosePOSCashDrawer();
				m_pPOSCashDrawerDevice->DestroyWindow();
				delete m_pPOSCashDrawerDevice;
				m_pPOSCashDrawerDevice = NULL;
			}
		// The Citizen service object can cause an access violation when being destroyed here.
		} catch(...) {}	// Ignore errors since we are closing anyway.

		try {
			// (a.wetta 2007-03-15 17:45) - PLID 25234 - Close the MSR device
			if (m_pOPOSMSRThread) {
				// (a.wetta 2007-07-03 15:58) - PLID 26547 - Close the MSR device thread
				PostThreadMessage(m_pOPOSMSRThread->m_nThreadID, WM_QUIT, 0, 0);

			}
		} catch(...) {} // We should not prevent shutdown due to a bad driver

		//(a.wilson 2012-1-11) PLID 47517 - close the opos barcode thread device.
		try {
			if (m_pOPOSBarcodeScannerThread) {
				m_pOPOSBarcodeScannerThread->Shutdown();
				m_pOPOSBarcodeScannerThread = NULL;
			}
		} catch(...) {} // close anyway

		try {
			// (j.gruber 2007-04-24 14:59) - PLID 9802 - Close the POS Printer
			if (m_pPOSPrinterDevice) {
				m_pPOSPrinterDevice->ClosePOSPrinter();
				m_pPOSPrinterDevice->DestroyWindow();
				delete m_pPOSPrinterDevice;
				m_pPOSPrinterDevice = NULL;
			}
		} catch(...) {} // We should not prevent shutdown due to a bad driver

		LOG_IF_TOO_SLOW_AFTER(CMainFrame__OnClose, 2, 2000, "OPOSDevices_Close");
		LOG_IF_TOO_SLOW_BEFORE(CMainFrame__OnClose, 3);

		// (c.haag 2006-11-27 12:17) - PLID 20772 - Close the first available list
		// dialog
		// (z.manning 2011-06-15 09:47) - PLID 44120 - The first available list dialog is now a pointer.
		if (m_pdlgFirstAvailList != NULL) {
			m_pdlgFirstAvailList->DestroyWindow();
			delete m_pdlgFirstAvailList;
			m_pdlgFirstAvailList = NULL;
		}

		// (c.haag 2010-06-08 17:55) - PLID 38898 - Hide the NexPhoto modeless import form
		if (NULL != m_pNxPhotoImportForm) {
			ShowNexPhotoImportForm(FALSE);
			m_NxPhotoEventSink.CleanUp();
			m_pNxPhotoImportForm.Release();
		}

		// (a.walling 2011-06-22 11:59) - PLID 44260 - Release the acrobat reference
		if (m_pHeldAdobeAcrobatReference)
		{
			try {
				m_pHeldAdobeAcrobatReference.Release();
			} NxCatchAllIgnore();
		}

		// Do the actual close for the main frame
		CNxMDIFrameWnd::OnClose();

		LOG_IF_TOO_SLOW_AFTER(CMainFrame__OnClose, 3, 2000, "CNxMDIFrameWnd::OnClose");
		LOG_IF_TOO_SLOW_BEFORE(CMainFrame__OnClose, 4);

		// (c.haag 2009-07-07 13:15) - PLID 34379 - Close the RSI Medical Media System link
		RSIMMSLink::EnsureNotLink();

		//TES 9/16/2009 - PLID 35529 - Clean up the global dialog used by "windowless" narratives.
		EnsureNotGlobalEmrItemAdvNarrativeDlg();

		// Close the mirror link
		Mirror::CloseMirrorLink();

		// Disconnect from NxServer
		try {
			if (g_hNxServerSocket != NULL)
			{
				NxSocketUtils::Disconnect(g_hNxServerSocket);
				g_hNxServerSocket = NULL;
			}
		}
		catch (...)
		{
			// We don't care if we fail because we're closing
			// down anyway.
		}

		//TES 5/26/2005 - We have to clean up the license (which uses socket stuff) before calling NxSocketUtils::Destroy().
		if(g_pLicense) g_pLicense->Destroy();

		//TES 6/19/2006 - Since NxSocketUtils::Initialize() is called from CPracticeApp::InitInstance(), the Destroy() needs
		//to be called from CPracticeApp::ExitInstance(); otherwise, if they hit Cancel on the login dialog, Destroy() is never
		//called and it's a memory leak.
		// Close the network utilities
		//NxSocketUtils::Destroy();

		//Close the remote database.
		//TODO: this should probably have a block of related code afer it, to clean up.
		EnsureNotRemoteData();

		// (a.walling 2009-08-11 09:25) - PLID 34814 - clean this up too, in case it is around
		EnsureNotRemoteDataSnapshot();

		LOG_IF_TOO_SLOW_AFTER(CMainFrame__OnClose, 4, 2000, "EnsureNotRemoteData");
		LOG_IF_TOO_SLOW_BEFORE(CMainFrame__OnClose, 5);

		// Kill the barcode scanner connection
		Barcode_Close();
		
		LOG_IF_TOO_SLOW_AFTER(CMainFrame__OnClose, 5, 2000, "Barcode_Close");
		LOG_IF_TOO_SLOW_BEFORE(CMainFrame__OnClose, 6);

		// (a.walling 2007-06-01 16:41) - PLID 4850 - Finally, close the user handle.
		// (a.walling 2010-03-25 20:26) - PLID 32089 - But only if not null
		if (GetCurrentUserHandle() != NULL) {
			CloseUserHandle(GetCurrentUserHandle());
		}

		EndWaitCursor();

		LOG_IF_TOO_SLOW_AFTER(CMainFrame__OnClose, 6, 2000, "EndWaitCursor");
		LOG_IF_TOO_SLOW_BEFORE(CMainFrame__OnClose, 7);

		// That's it, the fun is over!
		dlg.DestroyWindow();

		LOG_IF_TOO_SLOW_AFTER(CMainFrame__OnClose, 7, 2000, "dlg.DestroyWindow");

	} NxCatchAll("Nextech Practice had a problem shutting down");
}

// (a.walling 2007-05-04 10:29) - PLID 4850 - Log out a user, saving settings and prompting when necessary
void CMainFrame::HandleUserLogout(BOOL bSwitchUser, CClosingDlg* pClosingDlg)
{
	CWnd* pMessageWnd = pClosingDlg == NULL ? (CWnd*)this : (CWnd*)pClosingDlg;

	// (b.savon 2016-01-15 08:46) - PLID 67718 - Logout the user from the API
	theApp.GetAPIObject()->LogOut();

	// (a.walling 2007-05-07 09:50) - PLID 4850
	if(GetCurrentUserHandle() == NULL) {
		// there is no currently logged on user! Just return.
		return;
	}

	// (a.walling 2012-06-04 16:30) - PLID 49829 - Reset and clear the settings data upon user logout
	AfxGetPracticeApp()->SaveSettingsData();
	AfxGetPracticeApp()->ResetSettingsData();

	// (a.walling 2010-06-23 11:54) - PLID 39321
	FreeRemoteDataCache();

	//DRT 2/7/03 - Added Cancel button to the exit dialog.  Because it halts program exit, this code needs to be first, so we don't shut domething down
	//			and then not close the program - could cause some wierd errors.
	//DRT 1/29/03 - Moved here from ExitInstance - we cannot guarantee messages are pumping there (which makes the message box not always work)
	//DRT 10/25/02 - Show Release
	//give users the option to stop logging their time
	//we must prompt even if they have it turned off, because they have an open record
	try {
		//ensure we have a connection first, in case something goes wrong and we don't have one, this code still executes
		if(GetRemoteData()) {
			//DRT 2/7/03 - Changed this to no longer check if they are prompting because of an open record, but prompt if they want
			//			prompted.  If they don't want prompted, and one is open ... they are on their own! (Bob's idea =P)
			//TES 12/18/2008 - PLID 32520 - This feature is blocked for Scheduler Standard users.
			if(g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM UserTimesT WHERE UserID = {INT} AND Checkout IS NULL", GetCurrentUserID());
				if(!rs->eof) {
					long oneLast = GetRemotePropertyInt("LogTimePromptOneLastTime", 0, 0, GetCurrentUserName(), true);
					if(GetRemotePropertyInt("LogTimePrompt", 0, 0, GetCurrentUserName(), true) || oneLast) {
						if(oneLast) {
							SetRemotePropertyInt("LogTimePromptOneLastTime", 0, 0, GetCurrentUserName());	//no more!
						}
						long nID = AdoFldLong(rs, "ID");
						//prompt for time
						int res;
						// (z.manning 2010-09-07 12:05) - PLID 38148 - Do not prompt if we are forcibly closing Practice.
						// Instead silently continue logging time.
						if(m_bForciblyClosing) {
							res = IDNO;
						}
						else {
							res = pMessageWnd->MessageBox("Would you like to end your logging for the current session?", NULL, MB_YESNOCANCEL);
						}

						if(res == IDCANCEL) {
							//don't exit the program
							return;
						}
						else if(res == IDYES) {
							//log the user out
							// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
							// (b.eyers 2016-01-18) - PLID 67542 - checkout time is now gotten from the server
							ExecuteParamSql("UPDATE UserTimesT SET Checkout = GETDATE() WHERE ID = {INT}", nID);
						}
					}
				}
			}
		}
	} NxCatchAll("Error closing log time.");

	// (j.anspach 05-24-2005 11:12 PLID 16561) - Audit the logout
	try {
		int AuditID;
		AuditID = BeginNewAuditEvent();
		if (AuditID != -1)
			AuditEvent(-1, GetCurrentUserName(), AuditID, aeiSessionLogout, -1, GetCurrentUserName(), "", aepMedium);
	} NxCatchAll("Error auditing logout.");

	// (a.walling 2007-05-04 10:29) - PLID 4850 - If we don't have our own CClosingDlg, use the one passed into us
	// (and let the caller handle freeing resources).
	BOOL m_bOwnClosingDlg = FALSE;
	if (pClosingDlg == NULL) {
		pClosingDlg = new CClosingDlg(this);
		m_bOwnClosingDlg = TRUE;
	}

	if (pClosingDlg != NULL && pClosingDlg->GetSafeHwnd() == NULL)
		pClosingDlg->Create(IDD_CLOSING_DLG, GetDesktopWindow());

	if (!bSwitchUser) {
		SaveLastOpenModule();
	}

	// (a.walling 2009-12-24 14:45) - PLID 36377 - Clear out any saved signatures
	CSignatureDlg::ClearSignatureCache();

	// Remember which patient we're on (note we don't care if we get an exception)
	// (a.walling 2010-05-21 10:12) - PLID 17768 - Let the toolbar save the last patient (along with the other patient history info)
	m_patToolBar.SaveLastPatient();
	// and contact
	try { SetRemotePropertyInt("lastContact",GetActiveContactID(), 0, GetCurrentUserName()); } catch (...) { ASSERT(FALSE); };
	
	// Remember toolbar states
	SaveBarState("ToolBar");

	//Dispatch notification dialog as appropriate.
	if(m_pNotificationDlg && m_pNotificationDlg->GetSafeHwnd())
		m_pNotificationDlg->ClearNotifications();

	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	if (GetWindowPlacement(&wp))
	{
		if (IsIconic()) {
			wp.flags |= WPF_RESTORETOMAXIMIZED;
		} else if (IsZoomed()) {
			wp.flags |= WPF_RESTORETOMAXIMIZED;
		} else { // is Restored 
			wp.flags = wp.flags;
		}
		// and write it to the .INI file
		WriteWindowPlacement(&wp);
	}

	if (m_bOwnClosingDlg) {
		delete pClosingDlg; // free the closing dialog memory if necessary
	}

	// (a.walling 2007-05-07 09:53) - PLID 4850 - No user is logged on now!
	if (bSwitchUser) // only close the handle here if we are switching users.
		CloseUserHandle(GetCurrentUserHandle());
}

// (a.walling 2007-05-04 10:31) - PLID 4850 - Try closing all windows and hiding all toolbars. return FALSE if failure (allow exit is false)
BOOL CMainFrame::TryCloseAll(CClosingDlg* pClosingDlg /*=NULL*/, BOOL bSwitchUser /*= FALSE*/)
{
	try {
		if (!m_bForciblyClosing) {
			if (!CheckAllowExit(bSwitchUser)) return FALSE;
		}
		else {
			PreExit();
		}

		//Remember which module was open just before closing
		SaveLastOpenModule();

		CWnd* pMdiClient = CWnd::FromHandle(m_hWndMDIClient);

		BeginWaitCursor();

		// Create our closing dialog since this is the longest part of switching users
		if (pClosingDlg != NULL && pClosingDlg->GetSafeHwnd() == NULL)
			pClosingDlg->Create(IDD_CLOSING_DLG, GetDesktopWindow());

		// Close all windows
		// This was a rough one to figure out. Eventually used the same method I used to subclass
		// the datalist (GetWindow iteration). The problem was that CloseAllDocuments does not
		// close the reports (since they do not derive fom a DocTemplate).
		
		// (a.walling 2009-06-12 13:23) - PLID 34176 - Destroy all modeless windows
		{
			POSITION pos = m_mapModelessWindows.GetStartPosition();
			while (pos) {
				HWND hwnd = NULL;
				long nCount = 0;
				m_mapModelessWindows.GetNextAssoc(pos, hwnd, nCount);

				if (::IsWindow(hwnd)) {
					::DestroyWindow(hwnd);
				}
			}

			m_mapModelessWindows.RemoveAll();
		}
		
		CArray<HWND, HWND> arDoomedWnds;
		CWnd* pChild = pMdiClient->GetWindow(GW_CHILD); // get first child
		// iterate through all our children and hide them for responsiveness sake.
		while(pChild)
		{
			if(pChild->IsWindowVisible())
			{
				pChild->ShowWindow(SW_HIDE); // hide the window
				arDoomedWnds.Add(pChild->GetSafeHwnd());
			}
			pChild = pChild->GetWindow(GW_HWNDNEXT); // get sibling
		}
	
		// hide all the toolbars
		ShowPatientBar(false);
		ShowContactBar(false);
		ShowDoctorBar(false);

		AfxGetApp()->CloseAllDocuments(FALSE); // close all of our open documents

		// now close all those windows
		if (arDoomedWnds.GetSize()) {
			for (int i = 0; i < arDoomedWnds.GetSize(); i++) {
				if (IsWindow(arDoomedWnds[i])) // is the window still valid?
					::SendMessage(arDoomedWnds[i], WM_CLOSE, NULL, NULL);
			}
		}
		
		RecalcLayout();

		UnNotifyUser(-1); // remove all notifications
		m_dlgAlert.Clear(); // clear all alerts

		// (z.manning 2011-06-15 09:49) - PLID 44120 - This fisrt available list dialog is now a pointer.
		if(m_pdlgFirstAvailList != NULL) {
			m_pdlgFirstAvailList->Clear();
			m_pdlgFirstAvailList->DestroyWindow(); // Recreated in HandleUserLogin.
			delete m_pdlgFirstAvailList;
			m_pdlgFirstAvailList = NULL;
		}

		KillTimer(IDT_TODO_TIMER);
		m_dlgToDoAlarm.DestroyWindow(); // ShowToDoList will recreate the window

		// (j.jones 2010-01-14 15:00) - PLID 35775 - clear the list of patients that
		// had been emergency accessed by the previous user
		m_arEmergencyAccessedPatientIDs.RemoveAll();
		m_bBlockedGroupsValid = false;

		// (j.jones 2010-02-10 11:33) - PLID 37224 - clear EMR Image Stamps
		ClearCachedEMRImageStamps();
		m_bEMRImageStampsLoaded = FALSE;

		// (j.jones 2010-06-01 10:23) - PLID 37976 - close & clear the folder monitors
		DeviceImport::GetMonitor().CloseAllDeviceFolderMonitors();

		//DRT 5/24/2007 - PLID 25892 - Cleanup the opportunity list
		if(m_pdlgOpportunityList) {
			m_pdlgOpportunityList->DestroyWindow();
			delete m_pdlgOpportunityList;
			m_pdlgOpportunityList = NULL;
		}

		// both the yakker and the sendmessage window will be recreated in InitPracYakker via HandleUserLogin
		if (m_pdlgSendYakMessage) {
			m_pdlgSendYakMessage->DestroyWindow();
			delete m_pdlgSendYakMessage;
			m_pdlgSendYakMessage = NULL;
		}

		if (m_pdlgMessager) {
			m_pdlgMessager->m_bSystemClose = TRUE;
			m_pdlgMessager->DestroyWindow();
			delete m_pdlgMessager;
			m_pdlgMessager = NULL;
		}

		// (z.manning 2009-06-11 17:29) - PLID 34589
		if(m_pLabFollowupDlg) {
			m_pLabFollowupDlg->DestroyWindow();
			delete m_pLabFollowupDlg;
			m_pLabFollowupDlg = NULL;
		}

		// (j.armen 2012-02-27 12:23) - PLID 48303
		if(m_pRecallsNeedingAttentionDlg) {
			m_pRecallsNeedingAttentionDlg->DestroyWindow();
			delete m_pRecallsNeedingAttentionDlg;
			m_pRecallsNeedingAttentionDlg = NULL;
		}

		// (a.wilson 2013-01-08 11:28) - PLID 54535 - destroy the parent which will destroy the child as well.
		if (m_pPrescriptionQueueParentDlg) {
			m_pPrescriptionQueueParentDlg->DestroyWindow();
			m_pPrescriptionQueueParentDlg.reset();
		}

		// (j.fouts 2012-9-5 3:28) - PLID 52482 - Need this dialog to be modeless
		if(m_pDrugInteractionDlg) {
			m_pDrugInteractionDlg->DestroyWindow();
			delete m_pDrugInteractionDlg;
			m_pDrugInteractionDlg = NULL;
		}

		// (z.manning 2009-06-11 17:52) - PLID 34589
		if(IsWindow(m_dlgAlert.GetSafeHwnd())) {
			m_dlgAlert.DestroyWindow();
		}

		// (z.manning 2009-06-12 08:39) - PLID 34589
		if(IsWindow(m_EMRWritableReviewDlg.GetSafeHwnd())) {
			m_EMRWritableReviewDlg.DestroyWindow();
		}

		// (z.manning 2009-06-12 08:39) - PLID 34589
		if(m_pParseTranscriptionsDlg != NULL && IsWindow(m_pParseTranscriptionsDlg->GetSafeHwnd())) {
			m_pParseTranscriptionsDlg->DestroyWindow();
			m_pParseTranscriptionsDlg = NULL;
		}

		// (z.manning 2011-06-24 12:14) - PLID 37932 - Clean up the room manager dialog if needed
		if(m_pRoomManagerDlg != NULL)
		{
			if(IsWindow(m_pRoomManagerDlg->GetSafeHwnd())) {
				m_pRoomManagerDlg->DestroyWindow();
			}
			delete m_pRoomManagerDlg;
			m_pRoomManagerDlg = NULL;
		}

		//TES 4/27/2009 - PLID 33889 - We also need to clear out all our SureScripts notification variables, so that
		// if we're switching users, the new user will be notified appropriately.
		if(m_pSureScriptsCommDlg) {
			m_pSureScriptsCommDlg->DestroyWindow();
			delete m_pSureScriptsCommDlg;
			m_pSureScriptsCommDlg = NULL;
		}

		m_bNexERxNeedingAttentionLoaded = false;
		m_mapProvIDToRenewalTime.clear();
		m_mapProvIDToPrescriptionTime.clear();

		//delete all tracked Case Histories
		extern CPracticeApp theApp;
		theApp.DeleteAllCaseHistories();

		// (j.jones 2007-05-04 11:04) - PLID 23280 - clear out the list of last payment dates
		EmptyLastPaymentDates();

		// (j.jones 2007-05-07 09:49) - PLID 25906 - clear out the lists of last bill/quote dates
		EmptyLastBillDates();
		EmptyLastQuoteDates();

		// (j.dinatale 2011-03-29 15:55) - PLID 42982 - remove the E-Statements Patient Selection dialog
		if(m_pEStatementPatientSelectDlg){
			m_pEStatementPatientSelectDlg->DestroyWindow();
			delete m_pEStatementPatientSelectDlg;
			m_pEStatementPatientSelectDlg = NULL;
		}

		// (j.gruber 2011-11-01 10:02) - PLID 46219 - CCHITReportsDlg
		if (m_pCCHITReportsDlg) {			
			m_pCCHITReportsDlg->DestroyWindow();
			delete m_pCCHITReportsDlg;
			m_pCCHITReportsDlg = NULL;
		}

		// (s.dhole 07/23/2012) PLID 48693 EligibilityRequestDlg
		if(m_pEligibilityRequestDlg != NULL)
		{
			if(m_pEligibilityRequestDlg) {
				m_pEligibilityRequestDlg->DestroyWindow();
			}
			delete m_pEligibilityRequestDlg;
			m_pEligibilityRequestDlg = NULL;
		}

		// (J.camacho 2016-01-27) PLID 68000
		//if (m_pHL7ToBeBilledDlg != NULL)
		//{
		//	m_pHL7ToBeBilledDlg->DestroyWindow();
		//	delete m_pHL7ToBeBilledDlg;
		//	m_pHL7ToBeBilledDlg = NULL;
		//}

		// (r.goldschmidt 2014-10-08 16:18) - PLID 62644 - CEligibilityReviewDlg
		if (m_pEligibilityReviewDlg)
		{
			m_pEligibilityReviewDlg->DestroyWindow();
			delete m_pEligibilityReviewDlg;
			m_pEligibilityReviewDlg = NULL;
		}
		
		// (r.goldschmidt 2014-10-10 16:18) - PLID 62644 - CEligibilityRequestDetailDlg
		if (m_pEligibilityRequestDetailDlg)
		{
			m_pEligibilityRequestDetailDlg->DestroyWindow();
			delete m_pEligibilityRequestDetailDlg;
			m_pEligibilityRequestDetailDlg = NULL;
		}
		
		// (j.dinatale 2012-08-13 16:07) - PLID 51941
		if(IsWindow(m_OMRProcessingDlg.GetSafeHwnd())) {
			m_OMRProcessingDlg.DestroyWindow();
		}

		// (a.walling 2015-01-29 13:59) - PLID 64558 - Close rescheduling queue
		if (m_pReschedulingQueueParentDlg) {
			if (::IsWindow(m_pReschedulingQueueParentDlg->GetSafeHwnd())) {
				::DestroyWindow(*m_pReschedulingQueueParentDlg);
			}

			m_pReschedulingQueueParentDlg.reset();
		}

		EndWaitCursor();

		return TRUE;
	} NxCatchAll("Error preparing to switch users!");

	// if we have an error, we should return true, so as not to prevent the user from closing the application.
	return TRUE;
}

void CMainFrame::OnSchedulerModule() 
{
	//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
	if(!g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrUse)) {
		return;
	}

	if (UserPermission(SchedulerModuleItem)) {

		// (j.jones 2006-12-19 10:32) - PLID 22793 - check the preference to open the room manager,
		// but only do so if the scheduler module is not already in memory
		if(GetRemotePropertyInt("OpenRoomMgrWhenOpenScheduler", 0, 0, GetCurrentUserName(), true) == 1) {

			CChildFrame *pFrame = GetOpenViewFrame(SCHEDULER_MODULE_NAME);
			if(pFrame == NULL) {
				//it is indeed not open already, so open the room manager
				GetMainFrame()->PostMessage(NXM_OPEN_ROOM_MANAGER, (long)-1,
					(BOOL)GetRemotePropertyInt("OpenRoomMgrWhenOpenSchedulerMinimized", 0, 0, GetCurrentUserName(), true) == 1);
			}
		}

		//and now open the scheduler module
		OpenModule(SCHEDULER_MODULE_NAME);
	}
}

CFrameWnd * CMainFrame::OpenModule(LPCTSTR moduleName, bool newModule/*=false*/)
{
	try
	{
		extern CPracticeApp theApp;

		//DRT 9/29/03 - PLID 9548 - If we're in the patients module, send a message to hide the 
		//		package window in case it's up.
		CChildFrame* p = GetActiveViewFrame();
		if(p && p->IsOfType(PATIENT_MODULE_NAME)) {
			if(p->GetActiveView())
				p->GetActiveView()->PostMessage(NXM_HIDE_BILLING_TOOL_WINDOWS);
		}
		//TES 2/6/04 - PLID 6350 - Similarly with the res entry, now that it's sticky-able.
		if(p && p->IsOfType(SCHEDULER_MODULE_NAME)) {
			if(p->GetActiveView()) {
				// (c.haag 2005-11-01 8:35) - PLID 17979 - When this message is sent,
				// the user gets prompted as to whether to hide the resentry dialog.
				// If they say 'No', then we should fail here.
				//
				if (TRUE == p->GetActiveView()->SendMessage(NXM_HIDE_RESENTRY)) {
					return GetActiveViewFrame();
				}
			}
		}

		//JMJ 3/5/04 - PLID 11288 - Don't let them leave the surgery center module if there are
		//open case histories. There should not be any open case histories unless they are in surgery center,
		//but we need to find a way to tell if they are in the surgery center or not.
		//(Note: however, we wouldn't want them to leave the module they are in anyways...)
		if(theApp.m_arypCaseHistories.GetSize() > 0) {
			long nSize = theApp.m_arypCaseHistories.GetSize();
			for (long i=0; i<nSize; i++) {
				CWnd *pWnd = (CWnd *)theApp.m_arypCaseHistories[i];
				if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible()) {
					MessageBox("Please close all open Case Histories before continuing.");
					return GetActiveViewFrame();
				}
			}
		}

		// This will be NULL if m_pOpenDoc is NULL
		CChildFrame *pFrame = GetOpenViewFrame(moduleName);
		CDocTemplate* pTemplate = NULL;

		// (c.haag 2003-10-03 09:28) - When we enter this function, we already checked user permissions.
		// Setting this flag will prevent the view from checking them.
		m_bAlreadyCheckedModulePermissions = TRUE;

		// Check to see if the document is already open
		if (m_pOpenDoc) {
			// Branch to the appropriate open operation
			if (!newModule && pFrame && !(GetActiveViewFrame() && GetActiveViewFrame()->IsOfType(moduleName))) {
				// The module is open but not active so activate it
				pFrame->MDIActivate();
				UpdateToolBarButtons();
				//TES 2/11/2004: This is now handled in the OnMDIActivate() handler.
				/*if (GetActiveView())//refresh the current view - BVB
					GetActiveView()->UpdateView();*/
			} else if (!newModule && pFrame && GetActiveViewFrame()->IsOfType(moduleName)){
				// The module is open and already active so do nothing
			} else {	
				// The module is not open yet so get it's correct document template
	//BVB - We can open all we want now
	//			if (GetOpenModuleCount() < MAXMODULES) {  
					// (a.walling 2014-06-09 15:19) - PLID 53127 - Explicitly using theApp rather than casting AfxGetApp
					pTemplate = theApp.GetTemplate(moduleName);
					if (pTemplate) {
						// Now that we have the template, get the frame
						ASSERT_VALID(pTemplate);
						try {
							pFrame = (CChildFrame *)pTemplate->CreateNewFrame(m_pOpenDoc, NULL);
						} NxCatchAllCall(__FUNCTION__" -- CreateNewFrame", { return nullptr; });
						if (pFrame) {
							try {
								pTemplate->InitialUpdateFrame(pFrame, m_pOpenDoc);
							} NxCatchAllCall(__FUNCTION__" -- InitialUpdateFrame", { return nullptr; });
						}
					}
	//			} else {
	//				pFrame = NULL;
	//				AfxMessageBox("To maintain system integrity, please close one or more modules before continuing.",MB_OK);
	//			}
			}
		} else {
			pTemplate = theApp.GetTemplate(moduleName);
			// (a.walling 2014-06-09 15:19) - PLID 53127 - Checking for NULL here
			if (pTemplate) {
				try {
					m_pOpenDoc = (CPracticeDoc *)(pTemplate->OpenDocumentFile(NULL));
				} NxCatchAllCall(__FUNCTION__" -- OpenDocumentFile", { return nullptr; });

				if (m_pOpenDoc) {
					pFrame = GetActiveViewFrame();
				}
			}
		}	

		// Now do all necessary initialization
		if (pFrame) {
			pFrame->SetType(moduleName);
			pFrame->ModifyStyle(FWS_ADDTOTITLE, WS_CAPTION);//BVB
			pFrame->SetWindowText(moduleName);
			
			UpdateToolBarButtons();
	#ifdef _DEBUG
		} else {
			// If we made it to here, we've failed
			TRACE0("Warning: failed to open module.\n");
	#endif
		}

		// (c.haag 2003-10-03 09:28) - When we enter this function, we already checked user permissions.
		// Setting this flag will prevent the view from checking them. We reset the flag before we leave.
		m_bAlreadyCheckedModulePermissions = FALSE;
		
		// Return our success status
		return pFrame;
	}
	NxCatchAll("Could not open module")
	{	return NULL;
	}
}

void CMainFrame::OnUpdateView() 
{
	extern int g_hotkeys_enabled;
	UpdateToolBarButtons(true);
	if (m_pOpenDoc) {
		CNxTabView *pActiveView = GetActiveView();
		if (pActiveView) {
			// Only update the active view if there IS an active view!
			// (a.walling 2010-10-12 15:36) - PLID 40906 - UpdateView now has a bForceRefresh option -- which we definitely want when hitting the refresh button.
			pActiveView->UpdateView(true);
		}
	}
}

void CMainFrame::OnViewToolbarsPatientsBar() 
{
	ShowPatientBar (FALSE == IsPatientBarVisible());
}

void CMainFrame::OnUpdateViewToolbarsPatientsBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(IsPatientBarVisible());
}

void CMainFrame::OnViewToolbarsStandardBar() 
{
	// TODO: RAC - I just came across this and I don't understand 
	// why this calls AfxGetMainWnd (well it used to do that 
	// but I'm changing all such references to GetMainFrame) 
	// when 'this' is the MainFrame class!) Can anyone
	// enlighten me?
	CMainFrame *tmpWnd;
	tmpWnd = GetMainFrame();
	if (tmpWnd) {
		if (tmpWnd->IsStandardBarVisible()) {
			tmpWnd->ShowStandardBar(0);
		} else {
			tmpWnd->ShowStandardBar(1);
		}
	}
}

void CMainFrame::OnUpdateViewToolbarsStandardBar(CCmdUI* pCmdUI) 
{
	CMainFrame *tmpWnd;
	tmpWnd = GetMainFrame();
	if (tmpWnd) {
		if (tmpWnd->IsStandardBarVisible()) {
			pCmdUI->SetCheck(1);
		} else {
			pCmdUI->SetCheck(0);
		}
	}
}

void CMainFrame::OnUpdateViewToolbarsContactsBar(CCmdUI* pCmdUI) 
{
	CMainFrame *tmpWnd;
	tmpWnd = GetMainFrame();
	if (tmpWnd) {
		if (tmpWnd->IsContactsBarVisible()) {
			pCmdUI->SetCheck(1);
		} else {
			pCmdUI->SetCheck(0);
		}
	}
}

void CMainFrame::OnChoosePatient() 
{
	ShowPatientBar(TRUE);
	//TES 1/6/2010 - PLID 36761 - New accessor for dropdown state.
	m_patToolBar.SetDroppedDown(TRUE);
}

void CMainFrame::OnUpdateChoosePatient(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CMainFrame::OnGotoFirstPatient() 
{
	CNxTabView *ptab = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
	if (ptab) {
		ptab->GetActiveSheet()->StoreDetails();
	}

	// Move
	//TES 1/6/2010 - PLID 36761 - New function for changing patients.
	// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
	m_patToolBar.SelectFirstRow();
}

void CMainFrame::OnUpdateGotoFirstPatient(CCmdUI* pCmdUI) 
{
	//TES 1/6/2010 - PLID 36761 - New accessor for row count
	if (m_patToolBar.GetRowCount() > 0) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

void CMainFrame::OnGotoLastPatient() 
{
	CNxTabView *ptab = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
	if (ptab) {
		ptab->GetActiveSheet()->StoreDetails();
	}

	// Move
	//TES 1/6/2010 - PLID 36761 - New function for changing patients.
	// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
	m_patToolBar.SelectLastRow();
}

void CMainFrame::OnUpdateGotoLastPatient(CCmdUI* pCmdUI) 
{
	//TES 1/6/2010 - PLID 36761 - New accessor for row count
	if (m_patToolBar.GetRowCount() > 0) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

void CMainFrame::OnGotoPreviousPatient() 
{
	CNxTabView *ptab = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
	if (ptab) {
		ptab->GetActiveSheet()->StoreDetails();
	}

	// Make sure we're allowed to move previous
	//TES 1/6/2010 - PLID 36761 - New accessor for position.

	// (a.walling 2010-05-06 13:47) - PLID 37081 - Skip over patients that cannot be accessed.
	bool bContinue = true;
	int nOffset = 0;
	while (bContinue) {
		nOffset++;

		// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_patToolBar.GetCurrentlySelectedRow();
		if (pRow) {
			for (int i = 0; i < nOffset && pRow; i++) {
				pRow = pRow->GetPreviousRow();
			}
		}
		if (pRow) {
			//TES 1/6/2010 - PLID 36761 - New function for changing patients.
			if (m_patToolBar.SelectByRow(pRow)) {
				bContinue = false;
			}
		} else {
			bContinue = false;
		}

		// sanity check -- if nOffset > 12, then exit out of this loop, since the user probably does not want to click on any more dialogs.
		// They can navigate manually if this is the case. Also it could be incredibly slow to iterate through a huge patient list if the
		// user does not have permission for a large amount of them.
		if (nOffset >= 12) {
			bContinue = false;
		}
	}

	// (d.lange 2011-03-11 10:56) - PLID 41010 - We need to update the device import dialog if the current patient filter is enabled
	DeviceImport::GetMonitor().NotifyDeviceImportPatientChanged();
}

void CMainFrame::OnUpdateGotoPreviousPatient(CCmdUI* pCmdUI) 
{
	//TES 1/6/2010 - PLID 36761 - New accessor for row count
	// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_patToolBar.GetCurrentlySelectedRow();
	bool enable = pRow != NULL && pRow != m_patToolBar.GetFirstRow();
	pCmdUI->Enable(enable);
}

void CMainFrame::OnGotoNextPatient() 
{
//	OnShowWindow(TRUE, 0);
	CNxTabView *ptab = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
	if (ptab) {
		ptab->GetActiveSheet()->StoreDetails();
	}

	// (a.walling 2010-05-06 13:47) - PLID 37081 - Skip over patients that cannot be accessed.
	bool bContinue = true;
	int nOffset = 0;
	while (bContinue) {
		nOffset++;

		// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_patToolBar.GetCurrentlySelectedRow();		
		if (pRow) {
			for (int i = 0; i < nOffset && pRow; i++) {
				pRow = pRow->GetNextRow();
			}
		}
		if (pRow) {
			//TES 1/6/2010 - PLID 36761 - New function for changing patients.
			if (m_patToolBar.SelectByRow(pRow)) {
				bContinue = false;
			}
		} else {
			bContinue = false;
		}

		// sanity check -- if nOffset > 12, then exit out of this loop, since the user probably does not want to click on any more dialogs.
		// They can navigate manually if this is the case. Also it could be incredibly slow to iterate through a huge patient list if the
		// user does not have permission for a large amount of them.
		if (nOffset >= 12) {
			bContinue = false;
		}
	}
	
	// (d.lange 2011-03-11 10:56) - PLID 41010 - We need to update the device import dialog if the current patient filter is enabled
	DeviceImport::GetMonitor().NotifyDeviceImportPatientChanged();
}

void CMainFrame::OnUpdateGotoNextPatient(CCmdUI* pCmdUI) 
{
	//TES 1/6/2010 - PLID 36761 - New accessor for row count
	// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_patToolBar.GetCurrentlySelectedRow();
	bool enable = pRow != NULL && pRow != m_patToolBar.GetLastRow();
	pCmdUI->Enable(enable);
}

// (a.walling 2010-05-21 10:12) - PLID 17768 - Popup the patient breadcrumbs menu
void CMainFrame::OnPatientBreadcrumbs()
{
	m_patToolBar.PopupBreadcrumbMenu();
}

// (j.gruber 2010-01-11 13:10) - PLID 36140 - added referral ID
void CMainFrame::OnNewPatientBtn(long nReferralID /*=-1*/) 
{
	if (!GetMainFrame()->CheckAllowCreatePatient()) {
		return;
	}

	//if they are on the case history tab, don't let them leave if they have open case histories
	if(theApp.m_arypCaseHistories.GetSize() > 0) {
		long nSize = theApp.m_arypCaseHistories.GetSize();
		for (long i=0; i<nSize; i++) {
			CWnd *pWnd = (CWnd *)theApp.m_arypCaseHistories[i];
			if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible()) {
				MessageBox("Please close all open Case Histories before continuing.");
				return;
			}
		}
	}

	long id = -1;
	int choice;
	//TES 8/13/2014 - PLID 63194 - Track the status to pass to our EX tablechecker
	CClient::PatCombo_StatusType pcstStatusForTC = CClient::pcstUnchanged;
	{

	// (z.manning, 04/16/2008) - PLID 29680 - Removed all references to obsolete 640x480 dialogs
	CNewPatient dlg(NULL, IDD_NEW_PATIENT, nReferralID);

//	if (!CheckAccess("Patients", "Add New Patient"))
	if (!UserPermission(NewPatient))
		return;
	// (j.gruber 2010-01-11 13:43) - PLID 36140 - track when the new patient dialog is open
	m_bNewPatientOpen = TRUE;
	choice = dlg.DoModal(&id);
	m_bNewPatientOpen = FALSE;

	pcstStatusForTC = dlg.GetIsProspect() ? CClient::pcstProspect : CClient::pcstPatient;

	}

	// (j.gruber 2010-09-08 11:34) - PLID 40413 - check to see whether we are opening the security groups dialog
	long nOpen = GetRemotePropertyInt("NewPatientOpenSecurityGroups", 0, 0, GetCurrentUserName(), true);

	//they want to open it and they didn't cancel and they have permission
	// (j.gruber 2010-10-26 14:06) - PLID 40416 - change to assign permission, instead of write
	BOOL bHasPerms = CheckCurrentUserPermissions(bioSecurityGroup, sptDynamic0, 0, 0, TRUE, TRUE);
	if (id > -1 && nOpen == 1 && choice != 0 && bHasPerms) {		
		
		CSecurityGroupsDlg dlg(this);
		dlg.m_nPatientID = id;
		dlg.DoModal();
	}

	CNxTabView *pView;
	CChildFrame *pFrame;	
	long nTempID;
	short tab = 0;

	switch (choice) {
		case 0:		//they canceled
			break;
		case 1:		//they saved, but nothing else
			nTempID = m_patToolBar.GetActivePatientID();
			// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
			m_patToolBar.UpdatePatient(id);
			//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here
			CClient::RefreshPatCombo(id, false, CClient::pcatActive, pcstStatusForTC);
//			m_patToolBar.Requery();
			//TES 1/7/2010 - PLID 36761 - No need to check for failure, we were already on this patient.
			m_patToolBar.TrySetActivePatientID(nTempID);
			if(GetActiveView())
				GetActiveView()->UpdateView();
			break;
		case 2:		//patients
			m_patToolBar.m_bChangedInScheduler = false;
			
			// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
			m_patToolBar.UpdatePatient(id);
			//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here
			CClient::RefreshPatCombo(id, false, CClient::pcatActive, pcstStatusForTC);
			//this will set the lastpatientID to be the currently active id
			//TES 1/7/2010 - PLID 36761 - This function can't fail, since you can't assign security groups on the New Patient screen.
			m_patToolBar.TrySetActivePatientID(id);//don't need to requery if we set to active
			
			if(FlipToModule(PATIENT_MODULE_NAME)) {
				pView = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
				if (pView) 
				{	
					if(pView->GetActiveTab() != 0) {
						pView->SetActiveTab(0);
					}

					// (j.jones 2009-06-03 10:25) - PLID 34461 - need to always update the patient view
					pView->UpdateView();

					//set Focus to the notes edit box
					CNxDialog*   pSheet;
					pSheet = pView->GetActiveSheet();

					try {
						if (pSheet) {
							pSheet->GetDlgItem(IDC_NOTES)->SetFocus();
						}
						else {
							AfxThrowNxException("Error Setting Focus In General 1");
						}

					}NxCatchAll("GetActiveSheet Returned NULL");
				}
			}
			
			break;
		case 3:		//schedualh
			// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
			m_patToolBar.UpdatePatient(id);
			//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here
			CClient::RefreshPatCombo(id, false, CClient::pcatActive, pcstStatusForTC);
			//this will set the lastpatientID to be the currently active id
			//TES 1/7/2010 - PLID 36761 - This function can't fail, since you can't assign security groups on the New Patient screen.
			m_patToolBar.TrySetActivePatientID(id);//don't need to requery if we set to active

			//JJ - this is done so if you are on the patients module, it will update
			//the view. Otherwise when you come back from the Scheduler, the data
			//wil show the last patient and not the new one.
			pFrame = GetActiveViewFrame();
			if (pFrame && pFrame->IsOfType(PATIENT_MODULE_NAME)) {
				CNxTabView *pActiveView = (CNxTabView *)pFrame->GetActiveView();
				if (pActiveView) {
					// Only update the active view if there IS an active view!
					pActiveView->UpdateView();
				}
			}

			FlipToModule(SCHEDULER_MODULE_NAME);

			////////////////////////////////////////////////////////////////////////////////////////////////////////
			break;

		case 4:		//add another

			//first save as if they did nothing, case 1
			nTempID = m_patToolBar.GetActivePatientID();
			// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
			m_patToolBar.UpdatePatient(id);
			//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here
			CClient::RefreshPatCombo(id, false, CClient::pcatActive, pcstStatusForTC);
//			m_patToolBar.Requery();
			//TES 1/7/2010 - PLID 36761 - No need to check for failure, we were already on this patient.
			m_patToolBar.TrySetActivePatientID(nTempID);

			//and now act like they clicked the button again
			//TES 2/17/2004: Let's post a message, because otherwise all our variables (like the CNewPatientDlg) will stay
			//on the stack, and if they add a bunch of patients in a row they could get a stack overflow.
			//OnNewPatientBtn();
			PostMessage(WM_COMMAND, MAKEWPARAM(ID_NEW_PATIENT_BTN,0));
			////////////////////////////////////////////////////////////////////////////////////////////////////////
			break;

		case 5:		//FFA
			// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
			m_patToolBar.UpdatePatient(id);
			//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here
			CClient::RefreshPatCombo(id, false, CClient::pcatActive, pcstStatusForTC);
			//this will set the lastpatientID to be the currently active id
			//TES 1/7/2010 - PLID 36761 - This function can't fail, since you can't assign security groups on the New Patient screen.
			m_patToolBar.TrySetActivePatientID(id);//don't need to requery if we set to active

			//JJ - this is done so if you are on the patients module, it will update
			//the view. Otherwise when you come back from the Scheduler, the data
			//wil show the last patient and not the new one.
			pFrame = GetActiveViewFrame();
			if (pFrame && pFrame->IsOfType(PATIENT_MODULE_NAME)) {
				CNxTabView *pActiveView = (CNxTabView *)pFrame->GetActiveView();
				if (pActiveView) {
					// Only update the active view if there IS an active view!
					pActiveView->UpdateView();
				}
			}

			OnFirstAvailableAppt();
			break;
	}
}

void CMainFrame::UpdateAllViews()
{
	TRACE("CMainFrame::UpdateAllViews()\n");
	BeginWaitCursor();
	for (long i=0; i<m_nOpenDocCnt; i++) {
		((CNxView *)m_pView[i])->UpdateView();
	}
	// (a.walling 2006-08-14 10:12) - PLID 21755 - Include the ToDoAlarmDlg, since it is not associated with a view
	if (m_dlgToDoAlarm) {
		m_dlgToDoAlarm.RecolorList();
	}

	// (j.jones 2009-06-25 17:08) - PLID 34729 - reset the room manager
	if(m_pRoomManagerDlg) {
		m_pRoomManagerDlg->ResetHourTimer();
		m_pRoomManagerDlg->RequeryAll(TRUE);
	}

	EndWaitCursor();
}

void CMainFrame::AddView(CNxView *newView)
{
	m_nOpenDocCnt++;
	m_pView.Add(newView);

	// (j.jones 2005-08-26 16:09) - PLID 17221 - add to our active module list as well, which orders differently
	UpdateOpenModuleList(newView);

	//anytime a module is added (not activated, just added), try to close an old one
	if(m_bCloseNthModule) {
		TryCloseNthModule();
	}
}

void CMainFrame::RemoveView(CNxView * oldView)
{
	for (long i=0; i<m_pView.GetSize(); i++) {
		if (m_pView[i] == oldView) {
			m_pView.RemoveAt(i);
			break;
		}
	}
	if ((--m_nOpenDocCnt) == 0) {
		m_pOpenDoc = NULL;
	}

	// (j.jones 2005-08-26 16:09) - PLID 17221 - remove from our active module list as well, which orders differently
	RemoveFromOpenModuleList(oldView);
}

void CMainFrame::SetClip(const CString & strClipType, CClip *pClipData, int nClipMethod /* = CLIP_TYPE_COPY */)
{
	if(m_pClipData) {
		delete m_pClipData;
	}
	m_strClipType = strClipType;
	m_pClipData = pClipData;
	m_nClipMethod = nClipMethod;
}

void CMainFrame::GetClip(const CString & strClipType, CClip *&pClipData, int &nClipMethod)
{
	if (strClipType == m_strClipType) {
		pClipData = m_pClipData;
		nClipMethod = m_nClipMethod;
	} else {
		pClipData = NULL;
		nClipMethod = 0;
	}
}

void CMainFrame::ClearClip()
{
	m_strClipType = "";
	m_pClipData->Clear();
	m_nClipMethod = 0;
}

void CMainFrame::RequeryPatientsBar()
{
 	m_patToolBar.Requery();
}

CChildFrame * CMainFrame::GetActiveViewFrame()
{
	CRuntimeClass *pRuntime;
	CFrameWnd *view = GetActiveFrame();
//	if (view != this)
	pRuntime = view->GetRuntimeClass();
	if (!strcmp (pRuntime->m_lpszClassName, "CChildFrame"))
		return (CChildFrame *)view;//(CChildFrame *)GetActiveFrame();
	else return NULL;
}

CChildFrame * CMainFrame::GetOpenViewFrame(LPCTSTR strModuleName)
{
	CNxView *p = GetOpenView(strModuleName);
	if (p) {
		return (CChildFrame *)p->GetParent();
	} else {
		return NULL;
	}
}

CNxView * CMainFrame::GetOpenView(LPCTSTR strModuleName)
{
	CNxView *pAns = 0;
	if (m_pOpenDoc) {
		POSITION pos = m_pOpenDoc->GetFirstViewPosition();

		CView* pView;
		CChildFrame *pFrame;
		while (pos != NULL) {
			pView = m_pOpenDoc->GetNextView(pos);
			if (pView && pView->GetParent() && pView->GetParent()->IsKindOf(RUNTIME_CLASS(CChildFrame)))
			{
				pFrame = (CChildFrame *)pView->GetParent();
				if (pFrame->IsOfType(strModuleName)) {
					pAns = (CNxView *)pView;
					return pAns;//BVB - without this, it just keeps cycling
				}
			}
		}
	}
	return pAns;
}

// (j.jones 2014-08-07 16:09) - PLID 63232 - returns true if the requested module name is the currently viewed module
bool CMainFrame::IsActiveView(LPCTSTR strModuleName)
{
	CChildFrame *pFrame = GetActiveViewFrame();
	if (pFrame != NULL) {
		return pFrame->IsOfType(strModuleName);
	}
	return false;
}

bool CMainFrame::EnablePatientsToolBar(bool bEnable, void *pLockedBy /* = 0 */)
{
	// If it's locked and not by the calling function then do nothing (return)
	if (m_pLockPatToolBar) {
		if (pLockedBy != m_pLockPatToolBar) return false;
	}
	
	if (bEnable) {
		// Since we are enabling the toolbar, we unlock this function
		m_pLockPatToolBar = 0;
	} else {
		// Since we are disabling the toolbar, we want to lock this function
		m_pLockPatToolBar = pLockedBy;
	}
	m_patToolBar.EnableWindow(bEnable);
	return true;
}

bool CMainFrame::FlipToModule(const CString & strModuleName)
{
	//First off, let's determine whether the user is even allowed to open this module.
	if(strModuleName == SCHEDULER_MODULE_NAME) {
		//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
		if(!g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrUse)) {
			MsgBox("You are not licensed to access this module.");
			return false;
		}
		if(!UserPermission(SchedulerModuleItem)) {
			return false;
		}
	}
	else if(strModuleName == PATIENT_MODULE_NAME) {
		//DRT 11/24/2004 - PLID 14779 - See notes in PatientView::ShowTabs.  They are now able to get into
		//	the patients module if they have 'Patients' or 'Billing' selected in the license.
		if(!g_pLicense->CheckForLicense(CLicense::lcPatient, CLicense::cflrUse) && !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrJustCheckingLicenseNoUI) 
			&& !g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrJustCheckingLicenseNoUI)) {
			//Here's a line of code I don't expect to get hit very often.
			MsgBox("You are not licensed to access this module.");
			return false;
		}
		if(!UserPermission(PatientModuleItem)) {
			return false;
		}
	}
	else if(strModuleName == ADMIN_MODULE_NAME) {
		if(!UserPermission(AdministratorModuleItem)) {
			return false;
		}
	}
	else if(strModuleName == INVENTORY_MODULE_NAME) {
		if(!g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrUse)) {
			MsgBox("You are not licensed to access this module.");
			return false;
		}
		if(!UserPermission(InventoryModuleItem)) {
			return false;
		}
	}
	else if(strModuleName == FINANCIAL_MODULE_NAME) {

		// (j.jones 2010-06-23 10:33) - PLID 39297 - check all possible licenses that show tabs in the financial module, but do not 'use' them
		// (d.thompson 2010-09-02) - PLID 40371 - Any cc licensing satisfies
		if(!g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent) && !g_pLicense->CheckForLicense(CLicense::lcHCFA, CLicense::cflrSilent)
			&& !g_pLicense->CheckForLicense(CLicense::lcEbill, CLicense::cflrSilent) && !g_pLicense->CheckForLicense(CLicense::lcEEligibility, CLicense::cflrSilent)
			&& !g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent)) {			
			MsgBox("You are not licensed to access this module.");
			return false;
		}

		if(!UserPermission(FinancialModuleItem)) {
			return false;
		}
	}
	else if(strModuleName == MARKET_MODULE_NAME) {
		if(!g_pLicense->CheckForLicense(CLicense::lcMarket, CLicense::cflrUse) && !g_pLicense->CheckForLicense(CLicense::lcRetention, CLicense::cflrJustCheckingLicenseNoUI) ) {
			MsgBox("You are not licensed to access this module.");
			return false;
		}
		if(!UserPermission(MarketingModuleItem)) {
			return false;
		}
	}
	else if(strModuleName == REPORT_MODULE_NAME) {
		if(!UserPermission(ReportModuleItem)) {
			return false;
		}
	}
	else if(strModuleName == CONTACT_MODULE_NAME) {
		if(!UserPermission(ContactModuleItem)) {
			return false;
		}
	}
	else if(strModuleName == LETTER_MODULE_NAME) {
		if(!g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrUse)) {
			MsgBox("You are not licensed to access this module.");
			return false;
		}
		if(!UserPermission(LetterWritingModuleItem)) {
			return false;
		}
	}
	else if(strModuleName == SURGERY_CENTER_MODULE_NAME) {
		if(!g_pLicense->CheckForLicense(CLicense::lcSurgeryCenter, CLicense::cflrUse)) {
			MsgBox("You are not licensed to access this module.");
			return false;
		}
	}
	// (d.thompson 2009-11-16) - PLID 36134
	else if(strModuleName == LINKS_MODULE_NAME) {
		if(!CheckCurrentUserPermissions(bio3rdPartyLinks, sptView)) {
			return false;
		}
	}

	//DRT 9/29/03 - PLID 9548 - If we're in the patients module, send a message to hide the 
	//		package window in case it's up.
	CChildFrame* p = GetActiveViewFrame();
	if(p && p->IsOfType(PATIENT_MODULE_NAME)) {
		if(p->GetActiveView())
			p->GetActiveView()->PostMessage(NXM_HIDE_BILLING_TOOL_WINDOWS);
	}
	//TES 2/6/04 - PLID 6350 - Similarly with the res entry, now that it's sticky-able.
	if(p && p->IsOfType(SCHEDULER_MODULE_NAME)) {
		if(p->GetActiveView()) {

			// (c.haag 2005-11-01 8:35) - PLID 17979 - When this message is sent,
			// the user gets prompted as to whether to hide the resentry dialog.
			// If they say 'No', then we should fail here.
			//
			if (TRUE == p->GetActiveView()->SendMessage(NXM_HIDE_RESENTRY)) {
				return false;
			}
		}
	}

	//If we got here, we are allowed to open it, so let's do so.
	if (m_pOpenDoc) {
		CChildFrame *pFrame;

		// Check to see if this type of module is already open
		pFrame = GetOpenViewFrame(strModuleName);
		if (pFrame) {
			// If so, make sure it is the active view
			if (!GetActiveViewFrame()->IsOfType(strModuleName)) {
				pFrame->MDIActivate();
				if(pFrame->GetActiveView()) {
					((CNxView *)pFrame->GetActiveView())->ShowToolBars();//bvb
				}
				UpdateToolBarButtons();
			}
		} else {
			// If the module isn't open yet, open it
			OpenModule(strModuleName);

		}
	}
	else {
		// If the module isn't open yet, open it
		OpenModule(strModuleName);
	}
	return true;
}

void CMainFrame::OnPalmPilot() 
{
	//This is now the call receive button
	try{
		_RecordsetPtr rs;

		CString m_strWithoutParenthesis;
		m_strWithoutParenthesis = m_strIncomingCallNum;
		//remove (, ), -
		m_strWithoutParenthesis.Remove('(');
		m_strWithoutParenthesis.Remove(')');
		m_strWithoutParenthesis.Remove('-');

		m_strWithoutParenthesis = m_strWithoutParenthesis.Left(10);

		rs = CreateRecordset("SELECT PersonT.ID, PersonT.First + ', ' + PersonT.Last + ' ' + PersonT.Middle AS PatName FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"WHERE PersonT.HomePhone = '%s' OR PersonT.WorkPhone = '%s' OR PersonT.HomePhone = '%s' OR PersonT.WorkPhone = '%s'", m_strIncomingCallNum, m_strIncomingCallNum, m_strWithoutParenthesis, m_strWithoutParenthesis);

		if(!rs->eof){
			long nID;
			nID = AdoFldLong(rs, "ID");
			//TES 1/7/2010 - PLID 36761 - This function may fail now
			if(m_patToolBar.TrySetActivePatientID(nID)) {

				FlipToModule(PATIENT_MODULE_NAME);

				if(GetMainFrame() && GetMainFrame()->GetActiveView())
					GetMainFrame()->GetActiveView()->UpdateView();
			}
		}
		else
			AfxMessageBox("No patients are recorded with this phone number");

		rs->Close();
	} NxCatchAll("Error in CMainFrame::OnPalmPilot()");
}

void CMainFrame::OnNexPDALinkSettings()
{
	try {
		NxOutlookUtils::EnsureOutlookMonitorIsRunning();
		if(NxOutlookUtils::IsOutlookLinkActive()) {
			NxOutlookUtils::ShowAddIn();
		}
		else {
			NxOutlookUtils::ShowMonitor();
		}
	}NxCatchAll("CMainFrame::OnNexPDALinkSettings");
}

// (z.manning 2009-08-26 14:40) - PLID 35345
void CMainFrame::OnNexSyncSettings()
{
	try
	{
		CNexSyncSettingsDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-04-01 16:47) - PLID 38018
void CMainFrame::OnAppointmentReminderSettings()
{
	try {
		if (CheckCurrentUserPermissions(bioConfigureNxReminderSetup, sptWrite)) 
		{
			CAppointmentReminderSetupDlg dlg(this);
			dlg.DoModal();
		}
	}NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnFirstAvailableAppt() 
{
	if (!UserPermission(SchedulerModuleItem))
		return;

	// If the FFA Results window exists, show that window instead of opening up a new instance of FFA.
	if (m_pdlgFirstAvailList->GetSafeHwnd() != NULL && m_pdlgFirstAvailList->IsWindowVisible()) {
		m_pdlgFirstAvailList->ShowWindow(SW_RESTORE);
		m_pdlgFirstAvailList->CenterWindow();
		return;
	}

	//(s.dhole 6/1/2015 4:18 PM ) - PLID 65645  Call without  scheduler module
	if (IDCANCEL == GetMainFrame()->m_FirstAvailAppt.DoModal()) return;

	//(s.dhole 6/1/2015 4:18 PM ) - PLID 65645  Comented out this code
	//**I changed it back since we were nearing the end of the scope and I didn't want to introduce instability** - JMM
	//I changed this from FlipToModule to OpenModule because if there was no modules open, then FlipToModule fails
	//as so nothing is open, so the GetOpenView which initializes pView fails and ncauses nothing to happen -JMM
	//OpenModule(SCHEDULER_MODULE_NAME);
	// Kids: Please don't try this at home.
	// (z.manning 2011-09-01 15:25) - PLID 43347 - We now call ActivateTab to do this.
	//g_Modules[Modules::Scheduler]->ActivateTab(SchedulerModule::MonthTab);
	//CNxTabView* pView = (CNxTabView *)GetOpenView(SCHEDULER_MODULE_NAME);
	//if (pView) {
	//	// Not good coding practice right here....
	//	CMonthDlg* pDlg = (CMonthDlg*)pView->GetActiveSheet();
	//	pDlg->FindFirstAvailableAppt();
	//}
}

void CMainFrame::SetTabStates(bool bState) 
{
//	m_NewPatient = !bState;
/*
	// Enable or disable everything
	CNxTabView *pView = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
	if (pView) {
		m_GUIToolBar.EnableWindow(bState);
		m_patToolBar.EnableWindow(bState);
		for(short i = 1; i < 7; i++){
			pView->m_Tabs.SetActivePage(i);
			pView->m_Tabs.SetEnabled(bState);
		}
		
		// Put us on the general 1 tab
		pView->m_Tabs.SetActiveTab(0);
		short tabid = 0;
		short pageid = 0;
		pView->OnTabPageActivateTabs(&tabid,&pageid);
	}*/
}

void CMainFrame::SetToolBars(LPCTSTR moduleName)
{
/*	CRect rect;
	if(!strcmp(moduleName,PATIENT_MODULE_NAME)){
		m_wndToolBar.GetWindowRect(&rect);
		rect.OffsetRect(0,rect.Height() +1);
		DockControlBar(&m_patToolBar, AFX_IDW_DOCKBAR_TOP,&rect);
	}else if(!strcmp(moduleName,SCHEDULER_MODULE_NAME)){
		DockControlBarLeftOf(&m_patToolBar, &m_GUIToolBar);
	}*/
}

long CMainFrame::SaveDeleteRecord()
{
	try{
		long ID = NewNumber("PatientsDeleted","ID");
		long nDeletingPatientID = GetActivePatientID();

		// (a.walling 2010-01-22 14:54) - PLID 37038 - Also store the UserDefinedID
		ExecuteSql("INSERT INTO PatientsDeleted (ID, FirstName, MiddleName, LastName, [SS#], "
			"OldID, DeleteDate, LoginName, CurrentStatus, Address1, Address2, City, State, "
			"Zip, OldUserDefinedID ) SELECT %li, [First], [Middle], [Last], [SocialSecurity], ID, GetDate(), '%s', "
			"CurrentStatus, Address1, Address2, City, State, Zip, UserDefinedID FROM PatientsT Inner Join "
			"PersonT ON PatientsT.PersonID = PersonT.ID WHERE PatientsT.PersonID = %li",
			ID,
			_Q(GetCurrentUserName()),
			nDeletingPatientID);

		// (a.walling 2010-01-22 14:57) - PLID 37038 - And store the UserDefinedID into the PersonT.ArchivedUserDefinedID column
		ExecuteSql("UPDATE PersonT SET ArchivedUserDefinedID = (SELECT UserDefinedID FROM PatientsT WHERE PersonID = %li) WHERE ID = %li", nDeletingPatientID, nDeletingPatientID);

		return ID;
	// (a.walling 2010-01-22 15:00) - PLID 37038 - This is in a transaction!! We need to at least throw this back to the caller.
	// Also using the Thread variation so as not to block execution leaving the transaction open. There are some other functions
	// called from OnPatientDelete that also use NxCatchAll, but they are unlikely to cause an exception (no db access, etc)
	//}NxCatchAll("Error in Deleting Patient");
	} NxCatchAllThrowThread("Error in Deleting Patient");

	return -1;
}

//DRT 2/6/2004 - PLID 3598 - Had to add ability to delete the current patient from outside this class
//	so the patient can be dropped once we have merged.  Contains a paramater to make it run in silent
//	mode, so that the user is not prompted for any messages.  Also does NOT check permissions if you 
//	go silent.
void CMainFrame::DeleteCurrentPatient(bool bSilent /* = false*/)
{
	OnPatientDelete(bSilent);	//deletes the current patient
}

//DRT 2/6/2004 - Added ability to go in "silent" mode, where all message boxes are blocked (it assumes you want to continue at all points)
void CMainFrame::OnPatientDelete(bool bSilent /* = false*/)
{
	CString tmpStr;
	_RecordsetPtr rs(__uuidof(Recordset));
	CArray<long, long> aDeletedAppts;

	try {

	//if they are on the case history tab, don't let them leave if they have open case histories
	if(theApp.m_arypCaseHistories.GetSize() > 0) {
		long nSize = theApp.m_arypCaseHistories.GetSize();
		for (long i=0; i<nSize; i++) {
			CWnd *pWnd = (CWnd *)theApp.m_arypCaseHistories[i];
			if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible()) {
				MessageBox("Please close all open Case Histories before continuing.");
				return;
			}
		}
	}

	if(!bSilent) {
		if (!UserPermission(DeletePatient))
			return;
		if (MessageBox("This will permanently delete this patient and all related records, \n"
			"including financial activity, from the database. Are you sure?", 
			"Confirm Delete", MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2) != IDYES) 
			return;
	}

	long ID = GetActivePatientID();
	// get the name for auditing
	CString strDeletedPatientName = GetExistingPatientName(ID);

	// (c.haag 2015-08-24) - PLID 67198 - Don't let users delete patients with any credit card transactions
	if (DoesPatientHaveCreditCardTransactions(GetExistingPatientUserDefinedID(ID)))
	{
		MsgBox("This patient has Credit Card transactions applied to it.  This patient cannot be deleted.");
		return;
	}

	// (j.armen 2012-04-03 10:01) - PLID 48299 - Cleaned up the cases so that we don't run a ton of records.
	//	These are the cases when we cannot delete the patient
	// (j.armen 2012-04-10 14:11) - PLID 48299 - Added Recalls
	// (a.wilson 2013-02-07 17:29) - PLID 51711 - Added Renewals
	_RecordsetPtr prs = CreateParamRecordset(
		"DECLARE @PatID INT\r\n"
		"SET @PatID = {INT}\r\n"
		"SELECT\r\n"
		"	CASE WHEN EXISTS(\r\n"
		"		SELECT GiftCertificatesT.ID FROM GiftCertificatesT\r\n"
		"		LEFT JOIN LineItemT ON GiftCertificatesT.ID = LineItemT.GiftID\r\n"
		"		WHERE PurchasedBy = @PatID OR ReceivedBy = @PatID AND LineItemT.Deleted = 0)\r\n"
		"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasGiftCert,\r\n"
		"	CASE WHEN EXISTS(\r\n"
		"		SELECT ID FROM EmrMasterT WHERE PatientID = @PatID)\r\n"
		"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasLockedEMR,\r\n"
		"	CASE WHEN EXISTS(\r\n"
		"		SELECT ID FROM LabsT WHERE PatientID = @PatID)\r\n"
		"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasLabs,\r\n"
		// (j.jones 2012-10-29 14:45) - PLID 53259 - also cannot delete if the prescription status is E-Prescribed
		"	CASE WHEN EXISTS(\r\n"
		"		SELECT ID FROM SureScriptsMessagesT\r\n"
		"		WHERE (PatientID = @PatID OR PatientMedicationID IN (SELECT ID FROM PatientMedications WHERE PatientID = @PatID)))\r\n"
		"		OR EXISTS(\r\n"
		"		SELECT ID FROM PatientMedications WHERE PatientID = @PatID AND PatientMedications.QueueStatus IN ({SQL}))\r\n"
		"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasSureScripts,\r\n"
		"	CASE WHEN EXISTS(\r\n"
		"		SELECT ID FROM GlassesOrderT WHERE PersonID = @PatID)\r\n"
		"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasGlassesOrder,\r\n"
		"	CASE WHEN EXISTS(\r\n"
		"		SELECT ID FROM LensRxT WHERE PersonID = @PatID)\r\n"
		"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasLensRx,\r\n"
		"	CASE WHEN EXISTS(\r\n"
		"		SELECT ID FROM RecallT WHERE PatientID = @PatID)\r\n"
		"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasRecall,\r\n"
		"	CASE WHEN EXISTS(\r\n"
		"		SELECT ID FROM RenewalRequestsT WHERE PatientID = @PatID)\r\n"
		"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasRenewalRequest,\r\n"
		// (j.fouts 2013-09-05 14:23) - PLID 58043 - Added HasSSEligibility
		"	CASE WHEN EXISTS(\r\n"
		"		SELECT ID FROM SureScriptsEligibilityRequestT WHERE PatientID = @PatID)\r\n"
		"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasSSEligibility,\r\n"
		// (j.jones 2015-11-04 09:27) - PLID 67459 - if the patient has any charges that returned a product, they cannot be deleted
		"	CASE WHEN EXISTS(\r\n"
		"		SELECT ReturnedProductsT.ID FROM ReturnedProductsT "
		"		INNER JOIN LineItemT ON ReturnedProductsT.ChargeID = LineItemT.ID "
		"		WHERE LineItemT.PatientID = @PatID)\r\n"
		"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasReturnedProduct\r\n"
		, ID, GetERxStatusFilter());

	//DRT 4/8/2004 - If this patient is marked on some gift certificate, they cannot be deleted!
	//MSC 5/21/2004 - Changed the query because the old query didn't take into account if the the patient 
	//			had an unused gift certificate in that case, there wouldn't be a record in LineItemT for 
	//			that patient which means there would still be a violation
	if(AdoFldBool(prs, "HasGiftCert")) {
		MsgBox("This patient is recorded in a gift certificate transaction.  Because these work between multiple patients, "
			"this patient cannot be deleted.");
		return;
	}

	//TES 9/24/2004 - If this patient has a locked EMR, they cannot be deleted!
	// (j.jones 2006-05-01 12:19) - instead, we just won't delete EMR information, ever
	if(AdoFldBool(prs, "HasLockedEMR")) {
		MsgBox("This patient has EMR data associated with it (possibly marked deleted), and therefore the patient cannot be deleted.");
		return;
	}

	//JMM- PLID 21480 - Don't allow if the patient has a lab, even if its deleted
	if (AdoFldBool(prs, "HasLabs")) {
		MsgBox("This patient has Lab data associated with it (possibly marked deleted), and therefore the patient cannot be deleted.");
		return;
	}

	// (d.thompson 2009-04-14 16:17) - PLID 33957 - Do not allow deleting any patients that have credit card
	//	transactions.  This is information that must remain permanent.
	// (d.thompson 2010-12-20) - PLID 41897 - Chase too
	// (c.haag 2015-08-24) - PLID 67198 - Moved this check outside of the query and into a utility function

	// (a.walling 2009-04-23 10:31) - PLID 34046 - Don't allow deleting if they have had SureScripts messages
	// (a.walling 2009-04-28 12:19) - PLID 34046 - Well, we should be able to delete pending prescriptions
	// (c.haag 2009-07-02 11:57) - PLID 34102 - Don't discriminate on the message type being mtPendingRx. Just don't allow the deletion
	// if the prescription exists in the SureScripts table.
	// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
	if (AdoFldBool(prs, "HasSureScripts")) {
		MsgBox("This patient has had electronic prescriptions, and therefore the patient cannot be deleted.");
		return;
	}

	// (s.dhole 2011-02-24 12:06) - PLID 40535 if there is Glasse order associate with patient than skip delete
	// (s.dhole 2010-03-10 17:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
	// (j.dinatale 2012-04-17 17:26) - PLID 49078 - changed glasses order to optical order
	if (AdoFldBool(prs, "HasGlassesOrder")) {
		MsgBox("This patient has Optical orders associated with it (possibly marked deleted), and therefore the patient cannot be deleted.");
		return;
	}

	// (s.dhole 2011-02-24 12:06) - PLID 40535 if there is lense prescription data associate with patient than skip delete.
	// Ideally this should not be a case to have data without order, but this is fail safe mechanism to make sure we do not have this data and we can delete patient.
	if (AdoFldBool(prs, "HasLensRx")) {
		MsgBox("This patient has Glasses prescription data associated with it (possibly marked deleted), and therefore the patient cannot be deleted.");
		return;
	}

	// (j.armen 2012-04-03 10:24) - PLID 48299 - If the patient has had a recall, do not allow deletion
	if (AdoFldBool(prs, "HasRecall")) {
		MsgBox("This patient has Recall data associated with it. This patient cannot be deleted.");
		return;
	}

	// (a.wilson 2013-02-07 17:33) - PLID 51711 - if the patient has a renewalrequest, do not allow deletion
	if (AdoFldBool(prs, "HasRenewalRequest")) {
		MsgBox("This patient has a Renewal Request associated with them. This patient cannot be deleted.");
		return;
	}

	// (j.fouts 2013-09-05 14:22) - PLID 58043 - If they have an eligibility request , do not allow deletion
	if (AdoFldBool(prs, "HasSSEligibility")) {
		MsgBox("This patient has an Eligibility Request associated with them. This patient cannot be deleted.");
		return;
	}

	// (j.jones 2015-11-04 09:27) - PLID 67459 - if the patient has any charges that returned a product, they cannot be deleted
	if (AdoFldBool(prs, "HasReturnedProduct")) {
		MsgBox("This patient has charges for inventory items that have since been returned. This patient cannot be deleted.");
		return;
	}

	//Build a list of things that will also be deleted.
	bool bThingsFound = false;
	CString strMessage = "This patient has the following data associated with it:\n";
	CString strTemp;
	_RecordsetPtr rsTemp = CreateRecordset("SELECT Sum(Amount) AS Charges FROM ChargeRespT WHERE ChargeID IN (SELECT ChargesT.ID FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID WHERE BillsT.PatientID = %li AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND BillsT.EntryType = 1)", ID);
	COleCurrency cuTemp = AdoFldCurrency(rsTemp, "Charges", COleCurrency(0,0));
	if(cuTemp != COleCurrency(0,0)) {
		bThingsFound = true;
		strTemp.Format("%s in charges\n", FormatCurrencyForInterface(cuTemp));
		strMessage += strTemp;
	}
	rsTemp = CreateRecordset("SELECT Sum(Amount) AS Pays FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  WHERE LineItemT.PatientID = %li AND LineItemT.Deleted = 0", ID);
	cuTemp = AdoFldCurrency(rsTemp, "Pays", COleCurrency(0,0));
	if(cuTemp != COleCurrency(0,0)) {
		bThingsFound = true;
		strTemp.Format("%s in payments, refunds, and adjustments\n", FormatCurrencyForInterface(cuTemp));
		strMessage += strTemp;
	}
	// (j.jones 2008-06-10 17:11) - PLID 28392 - warn about any (non-deleted) returned products
	rsTemp = CreateParamRecordset("SELECT Sum(QtyReturned) AS AmtReturned FROM ReturnedProductsT "
		"INNER JOIN ChargesT ON ReturnedProductsT.ChargeID = ChargesT.ID "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0", ID);
	if(!rsTemp->eof) {
		double dblQtyReturned = AdoFldDouble(rsTemp, "AmtReturned", 0.0);
		if(dblQtyReturned != 0.0) {
			bThingsFound = true;
			strTemp.Format("%g returned products\n", dblQtyReturned);
			strMessage += strTemp;
		}
	}
	rsTemp->Close();
	rsTemp = CreateRecordset("SELECT Count(ID) AS Appts FROM AppointmentsT WHERE PatientID = %li", ID);
	long nTemp = AdoFldLong(rsTemp, "Appts", 0);
	if(nTemp != 0) {
		bThingsFound = true;
		strTemp.Format("%li Appointment(s)\n", nTemp);
		strMessage += strTemp;
	}
	rsTemp = CreateRecordset("SELECT Count(TaskID) AS ToDos FROM ToDoList Where PersonID = %li", ID);
	long nCountToDos = AdoFldLong(rsTemp, "ToDos", 0);
	if (nCountToDos != 0 ) {
		bThingsFound = true;
		strTemp.Format("%li To do Item(s)\n", nCountToDos);
		strMessage += strTemp;
	}
	//DRT 8/6/2004 - PLID 13795 - Warn them if there are referring patients.
	rsTemp = CreateRecordset("SELECT COUNT(PersonID) AS Cnt FROM PatientsT WHERE ReferringPatientID = %li", ID);
	long nCntRefPat = AdoFldLong(rsTemp, "Cnt", 0);
	if(nCntRefPat != 0) {
		bThingsFound = true;
		strTemp.Format("%li referred patients/prospects.  These will be changed to <Unspecified>\n", nCntRefPat);
		strMessage += strTemp;
	}
	// (a.walling 2006-11-27 09:55) - PLID 23621 - At least warn them if they are marked as a family relative
	rsTemp = CreateRecordset("SELECT Count(PersonID) AS Relatives FROM PersonFamilyT WHERE RelativeID = %li", ID);
	long nCntRelatives = AdoFldLong(rsTemp, "Relatives", 0);
	if (nCntRelatives != 0) {
		bThingsFound = true;
		strTemp.Format("%li patients have this patient marked as their immediate relative. "
				"This will be updated to another member of the family, and the family "
				"will be deleted if it contains only 1 member.\n", nCntRelatives);
		strMessage += strTemp;
	}
	// (v.maida 2016-03-09 18:41) - PLID 68564 - Warn them if there are online visit records associated with this patient.
	rsTemp = CreateRecordset("SELECT Count(ID) AS OnlineVisits FROM IagnosisOnlineVisitT WHERE PatientID = %li", ID);
	long nCntOnlineVisits = AdoFldLong(rsTemp, "OnlineVisits", 0);
	if (nCntOnlineVisits != 0) {
		bThingsFound = true;
		strTemp.Format("%li online visit records are linked to this patient. These will be unlinked from this patient.\n", nCntOnlineVisits);
		strMessage += strTemp;
	}


	if(!bSilent) {
		if(bThingsFound) {
			strMessage += "These and all other records associated with this patient will be permanently deleted.\nAre you absolutely sure you wish to delete this patient?";
			if(MessageBox(strMessage, "Confirm Delete", MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2) != IDYES) {
				return;
			}
		}
		else if( MessageBox("This is an unrecoverable operation! "
			"Are you absolutely sure you wish to delete this patient?", 
			"Confirm Delete", MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2) != IDYES ) 
				return;
	}

	if(!bSilent) {
		//check to see if they are a partner
		rs = CreateRecordset("SELECT PersonT.ID, Last, First FROM PersonT INNER JOIN PartnerLinkT ON PersonT.ID = PartnerLinkT.PatientID WHERE PartnerID = %li",ID);
		if(!rs->eof) {
			tmpStr.Format("This patient is linked as the partner of patient '%s, %s'.\nDo you wish to delete this relation? (Clicking 'No' will cancel deleting this patient.)",CString(rs->Fields->Item["Last"]->Value.bstrVal),CString(rs->Fields->Item["First"]->Value.bstrVal));
			if(IDNO == MessageBox(tmpStr,"Practice",MB_YESNO|MB_ICONQUESTION))
				return;
		}
		rs->Close();
	}

	if(!bSilent) {
		//next check to see if they are a patient
		rs = CreateRecordset("SELECT PersonT.ID, Last, First FROM PersonT INNER JOIN PartnerLinkT ON PersonT.ID = PartnerLinkT.PartnerID WHERE PatientID = %li",ID);
		if(!rs->eof) {
			tmpStr.Format("This patient is linked to partner '%s, %s'.\nDo you wish to delete this relation? (Clicking 'No' will cancel deleting this patient.)",CString(rs->Fields->Item["Last"]->Value.bstrVal),CString(rs->Fields->Item["First"]->Value.bstrVal));
			if(IDNO == MessageBox(tmpStr,"Practice",MB_YESNO|MB_ICONQUESTION))
				return;
		}
		rs->Close();
	}

	if(!bSilent) {
		//last check to see if they are a donor
		rs = CreateRecordset("SELECT PersonT.ID, Last, First FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE DonorID = %li",ID);
		if(!rs->eof) {
			tmpStr.Format("This patient is a donor to '%s, %s'.\nDo you wish to delete this relation? (Clicking 'No' will cancel deleting this patient.)",CString(rs->Fields->Item["Last"]->Value.bstrVal),CString(rs->Fields->Item["First"]->Value.bstrVal));
			if(IDNO == MessageBox(tmpStr,"Practice",MB_YESNO|MB_ICONQUESTION))
				return;
		}
		rs->Close();
	}

	CWaitCursor pWait;

	// (a.wilson 2014-08-13 09:13) - PLID 63199 - combine sending tablecheckers for ex setup. get a list of to be deleted appointments to send tablecheckers for.
	_RecordsetPtr rsDeletedAppts = CreateParamRecordset("SELECT ID, StartTime, EndTime, Status, ShowState, LocationID, dbo.GetResourceIDString(ID) AS ResourceIDs FROM AppointmentsT WHERE PatientID = {INT}", ID);

	// (c.haag 2008-02-29 15:07) - PLID 29115 - Support for inventory todo alarm transactions. Because
	// the patient may have allocations, deleting that person will affect inventory on-hand totals
	int nInvTodoTransactionID = -1;
	try {		
		long DeletedPatientID = SaveDeleteRecord();	//Saves a record of deletion
		long tempID = -1;

		// (c.haag 2008-02-29 15:08) - PLID 29115 - Gather all the product ID's and location ID's that we can now,
		// because the allocations are actually being deleted rather than being marked as deleted.
		nInvTodoTransactionID = InvUtils::BeginInventoryTodoAlarmsTransaction();
		_RecordsetPtr prsInvTodos = CreateRecordset(
			"SELECT ProductID, LocationID FROM PatientInvAllocationDetailsT "
			"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
			"WHERE PatientInvAllocationsT.PatientID = %d",
			ID);
		while (!prsInvTodos->eof) {
			const long nProductID = AdoFldLong(prsInvTodos, "ProductID", -1);
			const long nLocationID = AdoFldLong(prsInvTodos, "LocationID", -1);
			InvUtils::AddProductLocToInventoryTodoAlarmsTransaction(nInvTodoTransactionID, nProductID, nLocationID);
			prsInvTodos->MoveNext();
		}
		prsInvTodos->Close();

		// (r.farnworth 2015-11-02 11:05) - PLID 66922 - Change Practice's patient deletion functionality to call our new API function for deleting patients
		Nx::SafeArray<BSTR> nPatientAry;
		nPatientAry.Add(AsString(ID));
		GetAPI()->DeletePatients(GetAPISubkey(), GetAPILoginToken(), nPatientAry);

		long AuditID = -1;
		AuditID = BeginNewAuditEvent();
		if(AuditID!=-1) {
			// (a.walling 2010-01-22 14:25) - PLID 37024 - This should be the ID not DeletedPatientID, which is the ID of the PatientsDeleted table
			AuditEvent(ID, strDeletedPatientName,AuditID,aeiPatientDeleted,DeletedPatientID,"0","1",aepHigh,aetDeleted);
		}

		//GetOpenView(PATIENT_MODULE_NAME)->Delete();
		SetTabStates(true);
		//m_pOpenDoc->m_rsActivePatient.Delete();
		UnselectDeletedPatientFromToolbar();
//		IncrementTimeStamp(PATIENT_TIME_STAMP_NAME);
//		IncrementTimeStamp(PATIENT_LIST_TIME_STAMP_NAME);

		// (c.haag 2008-02-29 14:54) - PLID 29115 - Update inventory todo alarms.	
		//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
		InvUtils::CommitInventoryTodoAlarmsTransaction(nInvTodoTransactionID, true);
		// also moved this to after the committed transaction
		CClient::RefreshTable(NetUtils::TodoList, -1);

		// (a.wilson 2014-08-13 09:20) - PLID 63199 - change to use ex table checkers.
		while (!rsDeletedAppts->eof) {
			long nApptID = AdoFldLong(rsDeletedAppts, "ID");
			CClient::RefreshAppointmentTable(nApptID, ID, AdoFldDateTime(rsDeletedAppts, "StartTime"), AdoFldDateTime(rsDeletedAppts, "EndTime"), (long)AdoFldByte(rsDeletedAppts, "Status"), 
				AdoFldLong(rsDeletedAppts, "ShowState"), AdoFldLong(rsDeletedAppts, "LocationID"), AdoFldString(rsDeletedAppts, "ResourceIDs", ""));
			// (c.haag 2005-10-28 15:31) - PLID 18121 - Notify the NexPDA link of the deletions.
			PPCDeleteAppt(nApptID);
			CClient::RefreshTable(NetUtils::DeletedPtAppt, nApptID);
			rsDeletedAppts->MoveNext();
		}

	} NxCatchAllCallThrow("Error Deleting Patient", 
		if (-1 != nInvTodoTransactionID) { 
			InvUtils::RollbackInventoryTodoAlarmsTransaction(nInvTodoTransactionID);
		}
	);

	//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here
	CClient::RefreshPatCombo(ID, true, CClient::pcatUnchanged, CClient::pcstUnchanged);

	//JJ - this outside try/catch is to catch transaction errors
	}NxCatchAll(__FUNCTION__);

	UpdateAllViews();
}

void CMainFrame::UnselectDeletedPatientFromToolbar()
{
	//TES 1/6/2010 - PLID 36761 - Encapsulated within CPatientToolbar
	m_patToolBar.RemoveCurrentRow();
}

void CMainFrame::SetStatusButtons()
{
	//TES 1/6/2010 - PLID 36761 - Encapsulated within CPatientToolbar
	m_patToolBar.UpdateStatusText();
}


void CMainFrame::OnActiveClicked()
{
//	m_patToolBar.m_butAll.SetCheck(0);
	if(m_Search == 1){
		m_patToolBar.m_butPatientSearchCheck.SetCheck(1);
	}else if(m_Search == 2){
		m_patToolBar.m_butProspectSearchCheck.SetCheck(1);
	}else{
		m_patToolBar.m_butAllSearchCheck.SetCheck(1);}
	m_Include = 1;
	GenerateFilters();
}

void CMainFrame::OnAllClicked()
{
//	m_patToolBar.m_butActive.SetCheck(0);
	if(m_Search == 1){
		m_patToolBar.m_butPatientSearchCheck.SetCheck(1);
	}else if(m_Search == 2){
		m_patToolBar.m_butProspectSearchCheck.SetCheck(1);
	}else{
		m_patToolBar.m_butAllSearchCheck.SetCheck(1);}
	m_Include = 2;
	GenerateFilters();
}

void CMainFrame::OnPatientSearchClicked()
{
	if(m_Include == 1){
		m_patToolBar.m_butActive.SetCheck(1);
	}else{
		m_patToolBar.m_butAll.SetCheck(1);}
	m_Search = 1;
	GenerateFilters();
}

void CMainFrame::OnProspectSearchClicked()
{
	if(m_Include == 1){
		m_patToolBar.m_butActive.SetCheck(1);
	}else{
		m_patToolBar.m_butAll.SetCheck(1);}
	m_Search = 2;
	GenerateFilters();
}

void CMainFrame::OnAllSearchClicked()
{
	m_patToolBar.m_butProspectSearchCheck.SetCheck(0);
	m_patToolBar.m_butPatientSearchCheck.SetCheck(0);
	if(m_Include == 1){
		m_patToolBar.m_butActive.SetCheck(1);
	}else{
		m_patToolBar.m_butAll.SetCheck(1);}
	m_Search = 3;
	GenerateFilters();
}

void CMainFrame::OnFilterSearchClicked()
{
	//TES 1/6/2010 - PLID 36761 - Moved into handler that can be called from CPatientToolbar
	HandleFilterSearchClicked();
}

void CMainFrame::HandleFilterSearchClicked()
{
	//TES 1/6/2010 - PLID 36761 - Moved into handler that can be called from CPatientToolbar
	BOOL check = (0 != m_patToolBar.m_butFilter.GetCheck());

	//this way you can't change the checkbox while the list is requerying
	CString str;
	if(m_patToolBar.IsLoading()) {
		m_patToolBar.m_butFilter.SetCheck(!check);
		return;
	}

	//TES 4/1/2008 - PLID 29509 - We were never re-enabling these buttons, but, due to a quirk in MFC, they were getting
	// re-enabled anyway.  That quirk was resolved in VS 2008, meaning that these buttons were staying disabled.  Since
	// it's not clear why we were disabling them in the first place, given that they're never disabled for any of the
	// other times the list requeries, we'll just take this out.
	/*// Enable or Disable the other buttons appropriately
	m_patToolBar.m_butActive.EnableWindow(!check);
	m_patToolBar.m_butAll.EnableWindow(!check);
	m_patToolBar.m_butPatientSearchCheck.EnableWindow(!check);
	m_patToolBar.m_butAllSearchCheck.EnableWindow(!check);
	m_patToolBar.m_butProspectSearchCheck.EnableWindow(!check);
	m_patToolBar.m_butFilter.EnableWindow(true);
	m_patToolBar.m_text.EnableWindow(!check);
	m_patToolBar.m_text2.EnableWindow(!check);*/

	// Change the filter that the dropdown is based upon
	GenerateFilters();
}

static void SafeSetCombo(_DNxDataListPtr combo, _variant_t &var)
{
	long row = combo->SetSelByColumn(0,var);
	if (row < 0)
		combo->PutCurSel(0);
}

// New way which uses MODULARITY!!!
void CMainFrame::GenerateFilters(int id/*=-25*/)
{
	bool ret = false;

	// (z.manning 2009-06-15 12:19) - PLID 34614 - We used to check for this here, but for no good
	// reason as far as I can tell as this is null when switching users yet this function still works
	// perfectly fine.
	//if (m_pOpenDoc) {

	TRACE("CMainFrame::GenerateFilters\n");
	long CurrentPatient = 0, NewPatient = 0;
	// Change the drop down box to hold the new sql statement
	try {
		//TES 1/6/2010 - PLID 36761 - New functions for accessing datalist.
		// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
		if(m_patToolBar.GetCurrentlySelectedRow())
			CurrentPatient = VarLong(m_patToolBar.GetCurrentlySelectedValue(CPatientToolBar::ptbcPersonID));
		else
			CurrentPatient = 0;
		ret = m_patToolBar.SetComboSQL(GetPatientSQL());
	}NxCatchAllCall("CMainFrame::GenerateFilters 1", {
		//TES 1/6/2010 - PLID 36761 - ResetFilter() moved to CPatientToolbar
		m_patToolBar.ResetFilter();
		HandleFilterSearchClicked();
	});
	try
	{
		if (!ret) {
			if (m_patToolBar.m_butFilter.GetCheck())
				m_patToolBar.Requery();

			NewPatient = m_patToolBar.SafeGetPatientID(CurrentPatient);

			//TS:  Let's check if it's invalid _before_ we set the ActivePatientID, am I crazy here?
			if(NewPatient<=0) {
				//TES 1/6/2010 - PLID 36761 - ResetFilter() moved to CPatientToolbar
				m_patToolBar.ResetFilter();
				HandleFilterSearchClicked();
				//TES 1/7/2010 - PLID 36761 - This function may fail now
				if(m_patToolBar.TrySetActivePatientID(CurrentPatient)) {
					NewPatient = CurrentPatient;
				}
			}
			else {
				//TES 1/6/2010 - PLID 36761 - Encapsulated in CPatientToolbar
				//TES 1/7/2010 - PLID 36761 - This function may fail now
				m_patToolBar.TrySetActivePatientID(NewPatient);
			}
		}
		OnUpdateView();
		SetStatusButtons();
	} NxCatchAllCall("CMainFrame::GenerateFilters 2", {
		//TES 1/6/2010 - PLID 36761 - ResetFilter() moved to CPatientToolbar
		m_patToolBar.ResetFilter();
		OnFilterSearchClicked();
	});
}

void CMainFrame::OnUpdatePatientDelete(CCmdUI* pCmdUI)
{
	if (IsPatientBarVisible())
		pCmdUI->Enable(true);
	else pCmdUI->Enable(false);
}

void CMainFrame::OnUpdateSuperbill(CCmdUI* pCmdUI) 
{
	try{
		CString str = "&Print Superbill for Current Patient";
		BOOL bEnable = FALSE;
		// (e.lally 2009-06-08) PLID 34498 - Get permissions - View scheduler module
		if(GetCurrentUserPermissions(bioSchedulerModule) & (sptView|sptViewWithPass)) {
			str.Format("&Print Superbill for '%s'", GetActivePatientName());
			pCmdUI->SetText(str);
			bEnable = TRUE;
		}
		pCmdUI->Enable(bEnable);
	}NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnUpdateEnvelope(CCmdUI* pCmdUI) 
{
	try{
		CString str = "&Envelope for Current Patient";
		BOOL bEnable = FALSE;
		// (e.lally 2009-06-08) PLID 34498 - Get permissions - Write patient history tab
		if(GetCurrentUserPermissions(bioPatientHistory) & (sptWrite|sptWriteWithPass)) {
			str.Format("&Envelope for '%s'", GetActivePatientName());
			pCmdUI->SetText(str);
			bEnable = TRUE;
		}
		pCmdUI->Enable(bEnable);
	}NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-09-22 16:17) - PLID 40640 - Educational materials
void CMainFrame::OnUpdateEducationalMaterials(CCmdUI* pCmdUI) 
{
	try{
		CString str = "Ed&ucational Materials for this Patient";
		BOOL bEnable = FALSE;
		if(GetCurrentUserPermissions(bioPatientHistory) & (sptWrite|sptWriteWithPass)) 
		{
			str.Format("Ed&ucational Materials for '%s'", GetActivePatientName());
			pCmdUI->SetText(str);
			bEnable = TRUE;
		}
		pCmdUI->Enable(bEnable);
	}NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnUpdatePrescription(CCmdUI* pCmdUI) 
{
	// (e.lally 2009-06-10) PLID 34529 - Get Permissions - Create patient medication
	BOOL bEnable = FALSE;
	CString str = "Ne&w Prescription for Current Patient";
	if(GetCurrentUserPermissions(bioPatientMedication) & (sptCreate|sptCreateWithPass)) {
		str.Format("Ne&w Prescription for '%s'", GetActivePatientName());
		bEnable = TRUE;
	}
	pCmdUI->SetText(str);
	pCmdUI->Enable(bEnable);	
}

void CMainFrame::OnUpdateAppointmentReminderSettings(CCmdUI* pCmdUI)
{
	try {
		// (c.haag 2010-06-14 11:30) - PLID 38018 - Update the UI based on licensing and permissions
		BOOL bEnable = FALSE;
		// (z.manning 2010-11-17 12:13) - PLID 40643 - Renamed to NxReminder
		if(g_pLicense->CheckForLicense(CLicense::lcNxReminder, CLicense::cflrSilent)){
			if(GetCurrentUserPermissions(bioConfigureNxReminderSetup) & (sptWrite|sptWriteWithPass)) {
				bEnable = TRUE;
			}
		}
		else {
			// (z.manning 2010-07-19 10:13) - PLID 24607 - Remove this from the menu if the client isn't licensed
			RemoveEntryFromSubMenu(GetMenu(), "Tools", ID_TOOLS_APPOINTMENTREMINDERSETTINGS, FALSE);
		}
		pCmdUI->Enable(bEnable);
	} NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnUpdateTrackSerializedItems(CCmdUI* pCmdUI)
{
	try {
		BOOL bEnable = FALSE;
		if(g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent)){
			// (e.lally 2009-06-08) PLID 34497 - Get permission - View patients module
			if(GetCurrentUserPermissions(bioPatientsModule) & (sptView|sptViewWithPass)) {
				bEnable = TRUE;
			}
		}
		pCmdUI->Enable(bEnable);
	} NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnUpdateGenerateBarcodes(CCmdUI* pCmdUI)
{
	if(g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent))
		pCmdUI->Enable(true);
	else
		pCmdUI->Enable(false);
}								 

void CMainFrame::OnUpdatePatientSerializedItems(CCmdUI* pCmdUI)
{
	try {
		// (e.lally 2009-06-08) PLID 34497 - Permission - View Patients module
		CString str = "&Track Serial Numbered / Expirable Items for Current Patient";
		BOOL bEnable = FALSE;
		if(GetCurrentUserPermissions(bioPatientsModule) & (sptView|sptViewWithPass)) {
			str.Format("&Track Serial Numbered / Expirable Items for '%s'...", GetActivePatientName());
			//Check the license after we have set the text
			if(g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent)){
				bEnable = TRUE;
			}
		}

		pCmdUI->SetText(str);
		pCmdUI->Enable(bEnable);
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2008-07-08 12:06) - PLID 24624 - added ability to run the patient summary
void CMainFrame::OnUpdatePatientSummary(CCmdUI* pCmdUI) 
{
	try {
	
		CString str = "&Patient Summary for Current Patient";
		// (e.lally 2009-06-08) PLID 34497 - Permission - View Patients module (dlg checks individual component permissions)
		BOOL bEnable = FALSE;
		if(GetCurrentUserPermissions(bioPatientsModule) & (sptView|sptViewWithPass)) {
			str.Format("&Patient Summary for '%s'...", GetActivePatientName());
			bEnable = TRUE;
		}

		pCmdUI->SetText(str);
		pCmdUI->Enable(bEnable);

	}NxCatchAll("Error in CMainFrame::OnUpdatePatientSummary");
}

CString CMainFrame::GetPatientSQL()
{
	CString Ans;
	
	// Result = Default SQL WHERE Filter;
	Ans.Format("%s", GetPatientFilter(true));

	return Ans;
}

//TODO - I notice that this parameter is not used.  Why do we have it?
CString CMainFrame::GetPatientFilter(bool bIncludeWhere /* = false */)
{
	CString Ans;
	CString strWhereAnd;

	// Are we using the patient filter?
	if (m_patToolBar.m_butFilter.GetCheck()) {
		CString strFilterClause;
		if(m_patToolBar.m_strFilterFrom != "" && m_patToolBar.m_strFilterWhere!="") {
			strFilterClause.Format("PersonID IN (SELECT PersonT.ID FROM %s WHERE (%s))", 
				m_patToolBar.m_strFilterFrom, m_patToolBar.m_strFilterWhere);
			Ans += strFilterClause;
			strWhereAnd = " AND ";
		}
	}

	// See if we want only active patients
	if (m_patToolBar.m_butActive.GetCheck() == 1) {
		// Add " WHERE (Archived = false)"
		Ans += strWhereAnd + CString("Archived = 0");
		strWhereAnd = " AND ";
	}

	// See if we want Patients, Prospects, or All
	// (d.moore 2007-05-02 14:37) - PLID 23602 - 'CurrentStatus = 3' was previously used for both
	//  patients and prospects. It should only be used for patients.
	if(m_patToolBar.m_butPatientSearchCheck.GetCheck() == 1) {
		// Add " AND (CurrentStatus = 1 OR CurrentStatus = 3)"
		Ans += strWhereAnd + CString("(CurrentStatus = 1 OR CurrentStatus = 3)");
		strWhereAnd = " AND ";
	} else if (m_patToolBar.m_butProspectSearchCheck.GetCheck() == 1) {
		// Add " AND (CurrentStatus = 2 OR CurrentStatus = 3)"
		Ans += strWhereAnd + CString("(CurrentStatus = 2)");
		strWhereAnd = " AND ";
	} else {
		// Add nothing
	}

	//DRT 6/18/2004 - PLID 13069 - We need to filter out those crazy inquiries!  There is
	//	code in the PatientToolbar::GetDefaultFilter which appears to be very similar
	//	to this function... I think whoever fixed that one should have done it here
	//	as well.
	Ans += strWhereAnd + "CurrentStatus <> 4";

	// Result = Default SQL WHERE tmpSearch AND tmpInclude
	return Ans;
}

void CMainFrame::OnAdministratorModule() 
{
	if (UserPermission(AdministratorModuleItem)) {
		OpenModule(ADMIN_MODULE_NAME);
	}
}

void CMainFrame::OnContactsModule() 
{
	if (UserPermission(ContactModuleItem)) {
		OpenModule(CONTACT_MODULE_NAME);
	}
}

void CMainFrame::OnFinancialModule() 
{
	// (j.jones 2010-06-23 10:33) - PLID 39297 - check all possible licenses that show tabs in the financial module, but do not 'use' them
	// (d.thompson 2010-09-02) - PLID 40371 - Any cc licensing satisfies
	if(!g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent) && !g_pLicense->CheckForLicense(CLicense::lcHCFA, CLicense::cflrSilent)
		&& !g_pLicense->CheckForLicense(CLicense::lcEbill, CLicense::cflrSilent) && !g_pLicense->CheckForLicense(CLicense::lcEEligibility, CLicense::cflrSilent)
		&& !g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent)) {			
		return;
	}

	if (UserPermission(FinancialModuleItem)) {
		OpenModule(FINANCIAL_MODULE_NAME);
	}
}

void CMainFrame::OnInventoryModule() 
{
	if(!g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrUse))
		return;

	if (UserPermission(InventoryModuleItem)) {
		OpenModule(INVENTORY_MODULE_NAME);
	}
}

void CMainFrame::OnLetterWritingModule() 
{
	if(!g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrUse))
		return;

	if (UserPermission(LetterWritingModuleItem)) {
		OpenModule(LETTER_MODULE_NAME);
	}
}

void CMainFrame::OnMarketingModule() 
{
	//check to see if they have marketing
	m_pDocToolBar->m_nUseMarketing = g_pLicense->CheckForLicense(CLicense::lcMarket, CLicense::cflrUse);

	if (m_pDocToolBar->m_nUseMarketing == 0) {
		m_pDocToolBar->m_nUseRetention = g_pLicense->CheckForLicense(CLicense::lcRetention, CLicense::cflrUse);

		//if they are both false, kick them out of the module
		if (m_pDocToolBar->m_nUseRetention == 0) {
			return;
		}
	}
	else {
		//check retention silently 
		m_pDocToolBar->m_nUseRetention = g_pLicense->CheckForLicense(CLicense::lcRetention, CLicense::cflrSilent);
	}

	if (UserPermission(MarketingModuleItem)) {
		OpenModule(MARKET_MODULE_NAME);
	}
}

void CMainFrame::OnReportsModule() 
{
	if (UserPermission(ReportModuleItem)) {
		OpenModule(REPORT_MODULE_NAME);
	}
}

void CMainFrame::OnSurgeryCenterModule()
{
	if(!g_pLicense->CheckForLicense(CLicense::lcSurgeryCenter, CLicense::cflrUse))
		return;

	if (CheckCurrentUserPermissions(bioASCModule,sptView))
	{
		OpenModule(SURGERY_CENTER_MODULE_NAME);
	}
}

// (d.thompson 2009-11-16) - PLID 36134
void CMainFrame::OnLinksModule() 
{
	if (CheckCurrentUserPermissions(bio3rdPartyLinks, sptView)) {
		OpenModule(LINKS_MODULE_NAME);
	}
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	//if (!m_bAllowTimers) return;

	// (a.walling 2012-12-04 16:12) - PLID 54037 - Check device import timers
	if (DeviceImport::GetMonitor().HandleTimer(nIDEvent)) {
		return;
	}

	// Make sure this is the correct timer
	switch (nIDEvent) {
	case IDT_TODO_TIMER:
		try {			
			//LogDetail("IDT_TODO_TIMER"); // (c.haag 2015-07-08) - PLID 65912 - We no longer log this since it isn't really helpful
			//check to see if the user has permissions to read their own to-do's before popping up the window
			if((GetCurrentUserPermissions(bioSelfFollowUps) & (SPT__R_________ANDPASS)) && DoesUserHaveTasks(GetCurrentUserID())) {
				if(!m_dlgToDoAlarm.GetSafeHwnd() || (m_dlgToDoAlarm.GetSafeHwnd() && !m_dlgToDoAlarm.IsWindowVisible())) {
					NotifyUser(NT_TODO, "");
				}
			}
		}
		NxCatchAll("Error in CMainFrame::OnTimer : IDT_TODO_TIMER");
		KillTimer(IDT_TODO_TIMER);
		break;
	case IDT_WAIT_FOR_SHEET_TIMER:
		{
			CNxTabView *pView = GetActiveView();
			CNxDialog* pSheet = NULL;
			if(pView && pView->IsKindOf(RUNTIME_CLASS(CNxTabView))) pSheet = pView->GetActiveSheet();
			if(pSheet && pSheet->IsKindOf(RUNTIME_CLASS(CNxDialog))) {
				m_nWaitingType = pSheet->AllowPopup();
				if(m_nWaitingType == WT_DONT_WAIT) {
					if(IsPopupAllowed()) {
						//Hooray!
						//TES 1/4/2007 - PLID 24119 - I don't know how we could get here without m_pNotificationDlg being 
						// valid, but it couldn't hurt to check.
						EnsureNotificationDlg();
						m_pNotificationDlg->ReleaseNotifications();
					}
					else {
						m_nWaitingType = WT_WAIT_FOR_SELF;
						KillTimer(IDT_WAIT_FOR_SHEET_TIMER);
					}
				}
				else if(m_nWaitingType != WT_WAIT_FOR_SHEET_ACTIVE) {
					KillTimer(IDT_WAIT_FOR_SHEET_TIMER);
				}
			}
			else {
				//If we couldn't get an active sheet, just check ourselves.
				if(IsPopupAllowed()) {
					//Hooray!
					//TES 1/4/2007 - PLID 24119 - I don't know how we could get here without m_pNotificationDlg being 
					// valid, but it couldn't hurt to check.
					EnsureNotificationDlg();
					m_pNotificationDlg->ReleaseNotifications();
				}
				else {
					m_nWaitingType = WT_WAIT_FOR_SELF;
					KillTimer(IDT_WAIT_FOR_SHEET_TIMER);
				}
			}
		}
		break;

	case MemberTimer(IDT_DELETE_QUEUED_FILES_TIMER):
		DeleteQueuedFiles();
		break;
	case IDT_TEST_CALLERID:
		{
			//OnReceivedCallerIdNumber("9876543210");
			KillTimer(nIDEvent);
			m_bIncomingCall = false;

		}
		break;

	case IDT_CONFIGRT_CACHE_FLUSH_TIMER:
		// (c.haag 2005-11-07 16:28) - We still flush in regular intervals
		FlushRemotePropertyCache();
		break;

	case IDT_NXSERVER_RECONNECT:
		KillTimer(IDT_NXSERVER_RECONNECT);
		InitNxServer();
		break;

	case IDT_OPOSMSRDEVICE_INIT_TIMER:
		KillTimer(IDT_OPOSMSRDEVICE_INIT_TIMER);
		if (!m_bOPOSMSRDeviceInitComplete) {
			// (a.wetta 2007-07-05 11:46) - PLID 26547 - The OPOS MSR device didn't start in a timely fashion, so let the user know that there may be a problem with the device
			MessageBox("Timed out while initializing the magnetic strip reader device.  Make sure that the MSR settings are correct "
				   "by going to Tools->POS Tools->Magnetic Strip Reader Settings.  Ensure that the device has been "
				   "correctly installed and that nothing else is currently using the MSR device.  Also, Practice may just need to be "
				   "restarted to resolve the issue.", 
				   "Magnetic Strip Reader Device Error", MB_ICONWARNING|MB_OK);
			// Close the MSR device thread
			PostThreadMessage(m_pOPOSMSRThread->m_nThreadID, WM_QUIT, 0, 0);
			m_pOPOSMSRThread = NULL;
		}
		break;
	//(a.wilson 2012-1-12) PLID 47517 - timer goes off for barcode scanner timeout.
	case IDT_OPOS_BARSCAN_INIT_TIMER:
		{
			KillTimer(IDT_OPOS_BARSCAN_INIT_TIMER);
			if (!m_bOPOSBarcodeScannerInitComplete) {
				MessageBox("Timed out while initializing the OPOS Barcode Scanner Device. "
					"Make sure that the settings in Tools->Barcode Scanner Settings "
					"are correct and ensure that the device has been correctly installed.", 
					"OPOS Barcode Scanner Device Error", 
					MB_ICONWARNING | MB_OK);
				//close the thread
				if (m_pOPOSBarcodeScannerThread) {
					m_pOPOSBarcodeScannerThread->Shutdown();
					m_pOPOSBarcodeScannerThread = NULL;
				}
			}
			break;
		}
	// (a.walling 2009-07-10 10:18) - PLID 33644
	case IDT_QUEUEDMESSAGE_TIMER:
		// (j.jones 2014-08-19 10:52) - PLID 63412 - this now starts the queue processing
		BeginProcessingQueuedMessages();
		break;

	case IDT_CHECKDATE_TIMER:
		// (a.walling 2010-01-27 13:17) - PLID 22496 - Reset the title bar text timer
		if (ResetTitleBarTextTimer()) {
			LoadTitleBarText();
		}
		break;

	case IDT_CHECK_DEBUG_MODE_TIMER:
		KillTimer(IDT_CHECK_DEBUG_MODE_TIMER);
		//TES 3/2/2015 - PLID 64736 - This is fired a couple seconds after we tried to attach NxDebug. If it's succeeded, update the title bar, otherwise notify the user
		if (!theApp.GetDebugModeActive()) {
			AfxMessageBox("This change will take effect after restarting Practice");
		}
		else {
			LoadTitleBarText();
		}
		break;
		// (s.tullis 2016-01-28 17:46) - PLID 68090
		// timer invoked we need to remind the user if they have any prescriptions needing their attention
	case IDT_REMINDER_PRESCRIPTION_NEEDING_ATTNETION:
		KillTimer(IDT_REMINDER_PRESCRIPTION_NEEDING_ATTNETION);
		m_bTimerGoingErxNeedingAttentionMessages = FALSE;
		RefreshNexERxNeedingAttention();
		break;
		// (s.tullis 2016-01-28 17:45) - PLID 67965
		// timer invoked we need to remind the user if they have any renewals
	case IDT_REMINDER_RENEWALS:
		KillTimer(IDT_REMINDER_RENEWALS);
		m_bTimerGoingRecieveRenewalMessages = FALSE;
		RefreshNexERxNeedingAttention();
		break;
	default:
		CNxMDIFrameWnd::OnTimer(nIDEvent);
		break;
	} 
}

// (j.jones 2014-08-26 11:48) - PLID 63222 - called by tablecheckers, this resets the todo timer for the given user
void CMainFrame::TryResetTodoTimer(long nTaskID)
{
	try {

		// (j.jones 2014-08-26 11:48) - PLID 63222 - All this code was moved from WindowProc:WM_TABLE_CHANGED.
		// - If a normal NetUtils::TodoList tablechecker arrives, this function is always called. nTaskID may be -1.
		// - If an Ex NetUtils::TodoList tablechecker arrives, this function is only called if the assigned user ID
		//	 is the current user, or is -1 which means it applies to several users. nTaskID won't be -1.

		//in case someone added an item for this user, and if our timer is killed, we want to start it again
		//TS: Only do this if they have not checked "Don't Remind Me"

		//DRT 4/22/03 - We need to update the todo timer if the added item has a time on it, even if DontRemind is
		//		set.
		bool bOverride = false;
		//DRT 11/12/2003 - PLID 10062 - We need to ensure the username is the currently logged in person.  We don't want
		//		to force a popup for someone who doesn't even have the item assigned to them.
		if (nTaskID != -1) {
			//DRT 3/28/2008 - PLID 29466 - Parameterize, this is hit VERY often (thousands of times a day in Internal).
			// (c.haag 2008-06-10 09:26) - PLID 11599 - Use TodoAssignToT
			_RecordsetPtr prs = CreateParamRecordset("SELECT Remind FROM ToDoList "
				"INNER JOIN TodoAssignToT ON TodoAssignToT.TaskID = ToDoList.TaskID "
				"WHERE ToDoList.TaskID = {INT} AND Remind >= GetDate() AND convert(nvarchar, Remind, 8) > '00:00:00' AND TodoAssignToT.AssignTo = {INT}", nTaskID, GetCurrentUserID());
			if (!prs->eof) {
				bOverride = true;
			}
		}

		//0 = we want to be reminded, 1 = we do not want to be reminded
		// (j.dinatale 2012-10-22 18:00) - PLID  52393 - Move to per-user option
		long nDontRemind = GetRemotePropertyInt("DontRemind_User", -1, 0, GetCurrentUserName());
		if (nDontRemind == -1) {
			//fall back to legacy per-system/path behavior
			nDontRemind = GetPropertyInt("DontRemind", 0);
		}

		if (bOverride || !nDontRemind) {

			//////////////////////
			// TODO: Find a way to see if the given task id demands refreshing here instead of just always refreshing 
			// if (nTaskID == -1 || nTaskID == DoesTaskAffectTheCurrentUser_NeedToBeEfficientSoDoNotMakeACallToSql(nTaskID))
			///////////////////////
			{

				// b.cardillo (2003-04-17 12:11:00) - Notice this code is similar to the code in 
				// CToDoAlarmDlg::Save().  Ultimately I think they need to both call a shared function to produce 
				// the same results, but for now we just need to keep the two functions in sync.

				// Gets the last ToDo time
				COleDateTime dt, now = COleDateTime::GetCurrentTime();
				dt = GetPropertyDateTime("TodoTimer", &dt);	//DRT 4/22/03 - this needs to fill in the dt value, not the now value!

				//DRT 11/12/2003 - Changed the order of things here.  If the 'dt' value is < the 'now' value, we want to pop up.  
				//	However, if that is true, and we don't want to be reminded, then we can't trust the 'dt' value, it is 
				//	probably not up to date.  In the case of untrustworthy data, we need to run the 'else' clause, which calcs
				//	when we should remind (if at all).
				if (dt < now && !nDontRemind) {
					SetToDoTimer(1);	//popup now
				}
				else {
					//DRT 11/12/2003 - Either dt > now, or we don't want to be reminded.  In either case, we need to calculate
					//	what time should really be popped up (if any)
					//it is possible that a new appointment has slipped in between the last time we saved the time and now

					// b.cardillo (2003-04-17 12:06:00) - I'm changing this to 1. Work for all editions of Practice (not 
					// just NexTech Internal), 2. Base the popup time on the Remind field instead of the Deadline field, 
					// and 3. Ignore the Remind values where the time is midnight.  This stuff must have been missed by 
					// d.thompson on 2003-01-30, and 2003-03-11 because here's his comment in his check-in of 
					// TodoAlarmDlg.h on 1-30: "DRT 1/30/03 - This is now enabled for ALL users (not just internal usage).  
					// Also, I made it use the remind time, not the deadline (since that isn't what determines when the 
					// popup comes up)" and this is his comment from that file on 2003-03-11: "DRT 3/11/03 - ignore 
					// midnight times!"
					//DRT 3/28/2008 - PLID 29466 - Parameterized
					// (c.haag 2008-06-10 09:27) - PLID 11599 - Use TodoAssignToT
					// (a.walling 2013-07-12 08:42) - PLID 57537 - The ToDoList table checker checks for the min upcoming 
					// remind time, but ignores > 24hrs, so we can limit this range to prevent scans when the ToDoList becomes 
					// large and there are many tasks with remind times in the future.
					_RecordsetPtr rs = CreateParamRecordset(
						"SELECT Min(Remind) AS MinTime FROM ToDoList "
						"INNER JOIN TodoAssignToT ON TodoAssignToT.TaskID = ToDoList.TaskID "
						"WHERE TodoAssignToT.AssignTo = {INT} AND Remind >= GetDate() AND Remind < DATEADD(d, 1, GetDate()) AND convert(nvarchar, Remind, 8) > '00:00:00' GROUP BY TodoAssignToT.AssignTo", GetCurrentUserID());

					COleDateTime dtNext;
					int nSeconds = -1;	//number of seconds until the next todo is due
					if (!rs->eof) {
						dtNext = rs->Fields->Item["MinTime"]->Value.date;
						nSeconds = (int)(dtNext - COleDateTime::GetCurrentTime()).GetTotalSeconds();
					}

					// b.cardillo (2003-04-17 12:07:00) - Another change that was made to the TodoAlarmDlg.cpp without being 
					// made here is the need to check for nSeconds values that are out of range: "DRT 3/21/03 - There was an 
					// issue with the "next time" being too far away, creating values that went over the highest long.  This 
					// wrapped around, causing a negative time to get entered for the timer (which pops it up immediately).  
					// Now, if the time is > 24 hours (86400 seconds), just set it to 0.  They aren't going to be running 
					// Practice for days at a time anyways.
					if (nSeconds > 86400) {
						nSeconds = -1;
					}

					//now check to see if the date we just read is before the date stored
					if (nSeconds == -1) {
						//we didn't find anything, so no reason to pop up the dialog
					}
					else {
						if ((dt - now).GetTotalSeconds() < 0 && nDontRemind) {
							//the 'dt' value is in the past, and we had "dont remind" set.  We can't trust this thing at all!
							//Just set the timer to nSeconds, which is what we calculated should be the next time
							SetToDoTimer(nSeconds * 1000);
						}
						else {
							//we have some finding, pop it up
							if (nSeconds < (dt - now).GetTotalSeconds()) {
								//our findings say we need to popup before the next time, so set it to ours
								SetToDoTimer(nSeconds * 1000);
							}
							else {
								//our findings say that the time already set to popup is earlier than our next
								//todo, so set the timer to the next set time
								SetToDoTimer((long)(1000 * (dt - now).GetTotalSeconds()));
							}
						}
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CMainFrame::SetToDoTimer(long nTimerLength)
{
	//DRT - 12/02/02 - Made a big change to the way the timer is setup.  If you enter a time (anything other than midnight) for a
	//			task, then you will be prompted at that task time no matter what.  This means that if the time is wanted to be set
	//			to 1 hour from now, but there is a task with a time of 30 minutes from now, we need to have the timer set 30 mins, 
	//			not 1 hour.  Also, if they turn it off (dont remind), but there is a time, we must set the timer to that time, 
	//			and not turned off.
	//			Nothing in this file changed for this, it all relies on the fact that things were saved correctly by the alarm.


	COleDateTime dt;
	// (s.tullis 2015-10-21 17:38) - PLID 62087 - Option where To-Do does not pop up ever
	// If the timer length is 0, or if they don't want to be reminded, then don't set the timer at all 
	if ((IDT_TODO_TIMER == 0) || (nTimerLength == 0) || GetRemotePropertyInt("TodoDoNotOpenToRemind", 0, 0, GetCurrentUserComputerName(), true) == 1)
	{	//This is _not_ "setting the time to never show."
		/*dt.SetDate(2978, 2, 10);
		SetPropertyDateTime("TodoTimer", dt);//set the timer to never show*/
		KillTimer(IDT_TODO_TIMER);
		return;
	} 
	else 
	{	// Set up the timer for the to do alarm
		try//save date/time in the database in case they exit
		{	dt = COleDateTime::GetCurrentTime();
			int days = nTimerLength		/ (24 * 60 * 60 * 1000),
				hours = nTimerLength	/ (60 * 60 * 1000),
				minutes = nTimerLength	/ (60 * 1000),
				seconds = nTimerLength	/ (1000);
			
			///////////////////////////////////
			// Changed by CH 1/17
			//dt += COleDateTimeSpan(days, hours, minutes, seconds);
			dt += COleDateTimeSpan(0, 0, 0, seconds);

			SetPropertyDateTime("TodoTimer", dt);
		}NxCatchAll ("Internal timer failure.");

		SetTimer(IDT_TODO_TIMER, nTimerLength, NULL);
	}
}

void CMainFrame::OpenTemplateEditor(ESchedulerTemplateEditorType eType, COleDateTime dtDefaultDate /* = g_cdtInvalid */)
{
	//TES 2/20/2015 - PLID 64336 - This was just checking bioSchedTemplating, regardless of eType
	if (!CheckCurrentUserPermissions(eType == stetLocation ? bioResourceAvailTemplating : bioSchedTemplating, sptRead)) {
		return;
	}

	CChildFrame *frmSched = GetOpenViewFrame(SCHEDULER_MODULE_NAME);
	CSchedulerView *viewSched;
	CTemplateItemEntryGraphicalDlg dlgTemplateEditor(eType, this);

	// (j.jones 2011-07-15 14:36) - PLID 39838 - give it the date we clicked on
	if (dtDefaultDate.GetStatus() == COleDateTime::valid) {
		dlgTemplateEditor.SetDefaultDate(dtDefaultDate);
	}

	if (frmSched) {
		viewSched = (CSchedulerView*)frmSched->GetActiveView();
		if (viewSched) {
			// (z.manning, 11/10/2006) - PLID 7555 - Pass in the active resource ID.
			dlgTemplateEditor.SetDefaultResourceID(viewSched->GetActiveResourceID());
		}
	}

	dlgTemplateEditor.DoModal();

	if (frmSched) {
		CNxSchedulerDlg *dlgScheduler = (CNxSchedulerDlg*)viewSched->GetActiveSheet();
		BeginWaitCursor();
		dlgScheduler->UpdateBlocks(true, true);
		CWnd::FromHandle((HWND)(dlgScheduler->m_pSingleDayCtrl.GethWnd()))->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
		dlgScheduler->m_pSingleDayCtrl.Refresh();
		if (dlgScheduler->m_pEventCtrl) {
			CWnd::FromHandle((HWND)(dlgScheduler->m_pEventCtrl.GethWnd()))->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			dlgScheduler->m_pEventCtrl.Refresh();
		}
		EndWaitCursor();
	}
}

void CMainFrame::OnTemplatingBtn() 
{
	OpenTemplateEditor(stetNormal);
}

// (z.manning 2014-12-01 14:03) - PLID 64205
void CMainFrame::OnTemplateCollectionsBtn()
{
	try
	{
		OpenSchedulerTemplateCollectionEditor();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2014-12-01 14:03) - PLID 64205
void CMainFrame::OpenSchedulerTemplateCollectionEditor()
{
	OpenTemplateEditor(stetCollection);
}

//TES 6/19/2010 - PLID 5888 - Same as OnTemplatingBtn(), except it tells the editor to use the Resource Availability templates.
void CMainFrame::OnResourceAvailTemplating() 
{
	//try/catch is in the public method
	EditLocationTemplating();
}

//(e.lally 2010-07-15) PLID 39626 - public method for opening the Location Template editor (scheduler module)
// (j.jones 2011-07-15 14:45) - PLID 39838 - added default date
void CMainFrame::EditLocationTemplating(COleDateTime dtDefaultDate /*= g_cdtInvalid*/)
{
	try
	{
		//TES 6/19/2010 - PLID 5888 - Scheduler Standard users aren't allowed to use this.
		if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Location Templating")) {
			return;
		}

		OpenTemplateEditor(stetLocation, dtDefaultDate);
	}
	NxCatchAll(__FUNCTION__);
}


void CMainFrame::OnAptBookingAlarmsBtn() 
{
	//TES 12/18/2008 - PLID 32513 - This isn't allowed for Scheduler Standard users.
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Configure booking alarms for possible duplicate appointments")) {
		return;
	}

	if (CheckCurrentUserPermissions(bioAdminScheduler, sptView|sptRead|sptWrite)) {

		m_dlgAptBookAlarmList.DoModal();
	}
}

// (j.jones 2008-07-17 08:51) - PLID 30730 - added a modeless EMR Problem List
void CMainFrame::ShowEMRProblemList(long nPatientID, EMRProblemRegardingTypes eprtDefaultRegardingType /*= eprtInvalid*/, long nDefaultRegardingID /*= -1*/, CString strRegardingDesc /*= ""*/)
{
	try {

		if(nPatientID == -1) {
			ASSERT(FALSE);
			return;
		}

		if(g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
			return;
		}

		if(CheckCurrentUserPermissions(bioEMRProblems, sptRead)) {

			if(m_pEMRProblemListDlg) {

				// Bring the window up if it's minimized
				WINDOWPLACEMENT wp;
				wp.length = sizeof(WINDOWPLACEMENT);
				if (m_pEMRProblemListDlg->GetWindowPlacement(&wp)) {
					if (m_pEMRProblemListDlg->IsIconic()) {
						if (wp.flags & WPF_RESTORETOMAXIMIZED) {
							wp.showCmd = SW_MAXIMIZE;
						} else {
							wp.showCmd = SW_RESTORE;
						}
						m_pEMRProblemListDlg->SetWindowPlacement(&wp);
					}
				}
				// Bring the window to the foreground so the user is sure to see it
				m_pEMRProblemListDlg->SetForegroundWindow();

				//prompt if the patient is different
				if(nPatientID != m_pEMRProblemListDlg->GetPatientID()) {
					CString str;
					str.Format("The EMR problem list is already open for patient '%s'.\n"
						"Would you like to close this list and reopen it for patient '%s'?",
						GetExistingPatientName(m_pEMRProblemListDlg->GetPatientID()),
						GetExistingPatientName(nPatientID));
					if(IDYES == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {

						//destroy the window, the code below will recreate it
						DestroyEMRProblemList();
					}
					else {
						return;
					}
				}
				else if(eprtDefaultRegardingType != eprtInvalid || m_pEMRProblemListDlg->GetRegardingType() != eprtInvalid) {
					//If it's the same patient but the list is filtered by regarding type
					//OR we're about to filter by regarding type, destroy the window, and
					//the code below will recreate it. We do not prompt if it's the same patient,
					//but we do reload at all times if there is any filter involved.
					DestroyEMRProblemList();
				}
			}
			
			if(m_pEMRProblemListDlg == NULL) {

				m_pEMRProblemListDlg = new CEMRProblemListDlg(this);
				m_pEMRProblemListDlg->m_bIsOwnedByMainframe = TRUE;
				m_pEMRProblemListDlg->SetDefaultFilter(nPatientID, eprtDefaultRegardingType, nDefaultRegardingID, strRegardingDesc);
				m_pEMRProblemListDlg->Create(IDD_EMR_PROBLEM_LIST_DLG);			
				m_pEMRProblemListDlg->ShowWindow(SW_SHOW);
			}
		}

	}NxCatchAll("Error in CMainFrame::ShowEMRProblemList");
}

// (j.jones 2008-07-17 16:39) - PLID 30730 - cleanly destroys modeless EMR Problem List
void CMainFrame::DestroyEMRProblemList()
{
	try {

		if(m_pEMRProblemListDlg) {
			if(m_pEMRProblemListDlg->m_hWnd) {
				m_pEMRProblemListDlg->DestroyWindow();
			}
			delete m_pEMRProblemListDlg;
			m_pEMRProblemListDlg = NULL;
		}

	}NxCatchAll("Error in CMainFrame::DestroyEMRProblemList");
}

// (j.jones 2008-10-22 08:40) - PLID 14411 - added EMR Analysis as a modeless dialog
void CMainFrame::ShowEMRAnalysisDlg()
{
	try {

		if(g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
			return;
		}

		if(!CheckCurrentUserPermissions(bioPatientEMR, sptRead)) {
			return;
		}

		if(m_pEMRAnalysisDlg) {

			// Bring the window up if it's minimized
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);
			if (m_pEMRAnalysisDlg->GetWindowPlacement(&wp)) {
				if (m_pEMRAnalysisDlg->IsIconic()) {
					if (wp.flags & WPF_RESTORETOMAXIMIZED) {
						wp.showCmd = SW_MAXIMIZE;
					} else {
						wp.showCmd = SW_RESTORE;
					}
					m_pEMRAnalysisDlg->SetWindowPlacement(&wp);
				}
			}
			// Bring the window to the foreground so the user is sure to see it
			m_pEMRAnalysisDlg->SetForegroundWindow();
		}
		
		if(m_pEMRAnalysisDlg == NULL) {

			m_pEMRAnalysisDlg = new CEMRAnalysisDlg(this);
			m_pEMRAnalysisDlg->Create(IDD_EMR_ANALYSIS_DLG);			
			m_pEMRAnalysisDlg->ShowWindow(SW_SHOW);
		}

	}NxCatchAll("Error in CMainFrame::ShowEMRAnalysisDlg");
}

// (j.jones 2008-10-21 17:58) - PLID 14411 - cleanly destroys the EMR Analysis screen
void CMainFrame::DestroyEMRAnalysisDlg()
{
	try {

		if(m_pEMRAnalysisDlg) {
			if(m_pEMRAnalysisDlg->m_hWnd) {
				m_pEMRAnalysisDlg->DestroyWindow();
			}
			delete m_pEMRAnalysisDlg;
			m_pEMRAnalysisDlg = NULL;
		}

	}NxCatchAll("Error in CMainFrame::DestroyEMRAnalysisDlg");
}

// (j.jones 2012-05-25 10:55) - PLID 44367 - added E-Remittance as modeless
// (j.jones 2012-10-12 10:06) - PLID 53149 - supported a file to auto-open (for OHIP)
void CMainFrame::ShowEOBDlg(CString strAutoOpenFilePath /*= ""*/)
{
	try {

		// (j.jones 2007-06-29 08:53) - PLID 23951 - disallow access if they do not
		// have the E-Remittance license (takes up a usage)
		// *Note: prior to the E-Remittance license existing, this used to use the Ebilling license
		if(!g_pLicense->CheckForLicense(CLicense::lcERemittance, CLicense::cflrUse)) {
			AfxMessageBox("You must have the Electronic Remittance license in order to use this feature.");
			return;
		}

		if(m_pEOBDlg) {

			// Bring the window up if it's minimized
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);
			if (m_pEOBDlg->GetWindowPlacement(&wp)) {
				if (m_pEOBDlg->IsIconic()) {
					if (wp.flags & WPF_RESTORETOMAXIMIZED) {
						wp.showCmd = SW_MAXIMIZE;
					} else {
						wp.showCmd = SW_RESTORE;
					}
					m_pEOBDlg->SetWindowPlacement(&wp);
				}
			}
			// Bring the window to the foreground so the user is sure to see it
			m_pEOBDlg->SetForegroundWindow();
		}
		
		if(m_pEOBDlg == NULL) {

			m_pEOBDlg = new CEOBDlg(this);
			// (j.jones 2012-10-12 10:08) - PLID 53149 - supported a file to auto-open, only used
			// when creating a new dialog
			m_pEOBDlg->m_strAutoOpenFilePath = strAutoOpenFilePath;
			//by setting the parent as the desktop window, it will always have its own taskbar entry
			//and messageboxes will be modal to the EOBDlg and not to Practice
			m_pEOBDlg->Create(IDD_EOB_DLG, GetDesktopWindow());
			m_pEOBDlg->BringWindowToTop();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-05-25 10:55) - PLID 44367 - added E-Remittance as modeless
void CMainFrame::DestroyEOBDlg()
{
	try {

		if(m_pEOBDlg) {
			if(m_pEOBDlg->m_hWnd) {
				m_pEOBDlg->DestroyWindow();
			}
			delete m_pEOBDlg;
			m_pEOBDlg = NULL;
		}

	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-07-11 10:10) - PLID 62862 - create new dialog that will import a lockbox payment file
void CMainFrame::ShowLockboxPaymentImportDlg(long nBatchID /*= -1*/)
{
	try {
		if (m_pLockboxPaymentImportDlg) {
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);
			if (m_pLockboxPaymentImportDlg->GetWindowPlacement(&wp)) {
				if (m_pLockboxPaymentImportDlg->IsIconic()) {
					if (wp.flags & WPF_RESTORETOMAXIMIZED) {
						wp.showCmd = SW_MAXIMIZE;
					}
					else {
						wp.showCmd = SW_RESTORE;
					}
					m_pLockboxPaymentImportDlg->SetWindowPlacement(&wp);
				}
			}
			m_pLockboxPaymentImportDlg->SetForegroundWindow();
		}
		if (m_pLockboxPaymentImportDlg == NULL) {
			m_pLockboxPaymentImportDlg = new CLockBoxPaymentImportDlg(this, nBatchID);
			m_pLockboxPaymentImportDlg->Create(IDD_LOCKBOX_IMPORT_DLG, GetDesktopWindow());
			m_pLockboxPaymentImportDlg->ShowWindow(SW_SHOW);
			m_pLockboxPaymentImportDlg->BringWindowToTop();
		}
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-07-11 10:10) - PLID 62862 - create new dialog that will import a lockbox payment file
void CMainFrame::DestroyLockboxPaymentImportDlg()
{
	try {
		if (m_pLockboxPaymentImportDlg) {
			if (m_pLockboxPaymentImportDlg->m_hWnd) {
				m_pLockboxPaymentImportDlg->DestroyWindow();
			}
			delete m_pLockboxPaymentImportDlg;
			m_pLockboxPaymentImportDlg = NULL;
		}
	}NxCatchAll(__FUNCTION__);
}

void CMainFrame::ShowRoomManager(long nShowAppointmentID /*= -1*/, BOOL bShowMinimized /*= FALSE*/)
{
	// (j.jones 2007-03-05 12:35) - PLID 25059 - converted to perform the same checks as
	// entering the scheduler module - permissions & licensing - though here I decided to
	// make the license be silent as opposed to Use because the Room Manager would be used
	// in tandem with the scheduler module, and it's not really fair to decrement the usage
	// count again.

	//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
	if(!g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent)) {
		return;
	}

	if (UserPermission(SchedulerModuleItem)) {

		if(m_pRoomManagerDlg) {

			// Bring the window up if it's minimized
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);
			if (m_pRoomManagerDlg->GetWindowPlacement(&wp)) {
				if (m_pRoomManagerDlg->IsIconic() && !bShowMinimized) {
					if (wp.flags & WPF_RESTORETOMAXIMIZED) {
						wp.showCmd = SW_MAXIMIZE;
					} else {
						wp.showCmd = SW_RESTORE;
					}
					// (a.walling 2014-05-06 08:44) - PLID 57388 - Just use ShowWindow; SetWindowPlacement was causing the mainfrm to popup as well.
					m_pRoomManagerDlg->ShowWindow(wp.showCmd);
				}
			}
			m_pRoomManagerDlg->SetForegroundWindow();
			// Bring the window to the foreground so the user is sure to see it
			m_pRoomManagerDlg->ShowAppointment(nShowAppointmentID);
		}
		else {

			m_pRoomManagerDlg = new CRoomManagerDlg(this);
			m_pRoomManagerDlg->m_nPendingShowAppointment = nShowAppointmentID;
			m_pRoomManagerDlg->Create(IDD_ROOM_MANAGER_DLG);
			// (a.walling 2014-05-06 08:44) - PLID 57388 - Use SW_SHOWMINNOACTIVE instead of SW_SHOWMINIMIZED -- we don't want to activate a minimized window.
			m_pRoomManagerDlg->ShowWindow(bShowMinimized ? SW_SHOWMINNOACTIVE : SW_SHOW);
		}
	}
}

void CMainFrame::OnRoomManagerBtn() 
{
	// (e.lally 2009-06-10) PLID 34529 - Added try/catch
	try {
		ShowRoomManager();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2014-12-22 12:09) - PLID 64369 - Show rescheduling queue
void CMainFrame::OnReschedulingQueue()
{
	try {
		ShowReschedulingQueue();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2014-12-22 12:09) - PLID 64369 - Show rescheduling queue
void CMainFrame::ShowReschedulingQueue(long nApptID)
{
	if (!CheckCurrentUserPermissions(bioSchedulerModule, sptView)) {
		return;
	}

	if (CReschedulingQueue::ShouldDock()) {		
		if (m_pReschedulingQueueParentDlg) {
			if (m_pReschedulingQueueParentDlg->GetSafeHwnd()) {
				m_pReschedulingQueueParentDlg->DestroyWindow();
			}
			m_pReschedulingQueueParentDlg.reset();
		}

		PracticeModulePtr module(g_Modules[Modules::Scheduler]);
		if (module) {
			module->ActivateTab(SchedulerModule::DayTab);
		}
		if (FlipToModule(SCHEDULER_MODULE_NAME)) {
			if (auto* pSchedulerView = dynamic_cast<CSchedulerView*>(GetActiveView())) {
				// (a.walling 2015-01-22 16:08) - PLID 64662 - Rescheduling Queue - selecting specific appointments
				pSchedulerView->ShowReschedulingQueueEmbedded(true, nApptID);
			}
		}
	}
	else {
		if (auto* pSchedulerView = dynamic_cast<CSchedulerView*>(GetOpenView(SCHEDULER_MODULE_NAME))) {
			pSchedulerView->ShowReschedulingQueueEmbedded(false);
		}
		if (!m_pReschedulingQueueParentDlg) {
			m_pReschedulingQueueParentDlg.reset(new CNxModelessParentDlg(this, "CReschedulingQueue"));
			m_pReschedulingQueueParentDlg->CreateWithChildDialog(std::make_unique<CReschedulingQueue>(this), CReschedulingQueue::IDD, IDI_RESCHEDULINGQ, "Rescheduling Queue", this);
		}

		// (a.walling 2015-01-22 16:08) - PLID 64662 - Rescheduling Queue - selecting specific appointments
		boost::polymorphic_downcast<CReschedulingQueue*>(m_pReschedulingQueueParentDlg->m_pChild.get())->Refresh(nApptID);
		m_pReschedulingQueueParentDlg->ShowWindow(SW_SHOW);
	}
}

// (d.moore 2007-09-26) - PLID 23861 - The NexForms Editor is now modeless. So it makes more
//  sense to manage it from here rather than ProcedureDlg.
void CMainFrame::ShowNexFormEditor(long nProcedureID /*= -1*/ )
{
	if (m_NexFormEditorDlg.GetSafeHwnd() == NULL) {
		m_NexFormEditorDlg.Create(IDD_PROCEDURE_SECTION_DLG,GetActiveWindow());
	}
	
	m_NexFormEditorDlg.CenterWindow();
	
	m_NexFormEditorDlg.ShowWindow(SW_SHOWNORMAL);
	m_NexFormEditorDlg.SetProcedure(nProcedureID);
}


BOOL CMainFrame::DoesUserHaveTasks(long nUserID)
{
	//JMJ 2/25/2004 - removed the code that made the todo alarm pop up
	//if the deadline was <= now. We fully enforce that the deadline is after
	//the remind time, so we only need to check on the remind time
	//(the Todo list only shows based on remind time)
	//this fixed a problem where if a deadline was today but the remind time
	//was at a time later than the current time, the todo list popped up
	//but with no contents!
	// (c.haag 2008-06-10 09:19) - PLID 11599 - Use TodoAssignToT
	// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
	// Parameter sniffing can cause this to perform very poorly; use @UserID to avoid SQL trying to be too clever
	// since users may have drastically varying amounts of assigned todos
	auto prs = CreateParamRecordset(GetRemoteDataSnapshot(), "DECLARE @UserID INT; SET @UserID = {INT}; SELECT CASE WHEN EXISTS ("
	"SELECT ToDoList.TaskID FROM ToDoList "
	"INNER JOIN TodoAssignToT ON TodoAssignToT.TaskID = TodoList.TaskID "
	"LEFT JOIN PatientsT ON PatientsT.PersonID = ToDoList.PersonID "
	"WHERE (((ToDoList.Done Is Null) AND (ToDoList.Remind <= GetDate()) "
	"AND (PatientsT.CurrentStatus IS NULL OR PatientsT.CurrentStatus <> 4) AND ((TodoAssignToT.AssignTo)=@UserID)))"
	") THEN 1 ELSE 0 END", nUserID);
	
	BOOL bTasksExist = AdoFldLong(prs->Fields->Item[(long)0]);
	prs->Close();
	
	return bTasksExist;
}

void CMainFrame::ShowTodoList()
{
	try {
		// If we've been asked to open
		if (!m_dlgToDoAlarm.m_hWnd) {
			// (a.walling 2007-05-04 10:32) - PLID 4850 - Our dialog may have been created and destroyed, meaning
			// there is no hwnd but the members are still in an uninitialized state. So call Init() to reset them!
			m_dlgToDoAlarm.Init();
			// (a.walling 2012-04-02 08:32) - PLID 46648 - Dialogs must set a parent!
			m_dlgToDoAlarm.Create(CToDoAlarmDlg::IDD, GetDesktopWindow());
		}
		
		// If the window doesn't have focus or is minimized we've been asked to bring it to the top
		// (j.fouts 2012-05-10 09:29) - PLID 49858 - Now that we can maximize let the window open maximized
		m_dlgToDoAlarm.ShowWindow(SW_SHOW);

		// (j.fouts 2012-05-10 09:29) - PLID 49858 - Bring the window up if it's minimized
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		if (m_dlgToDoAlarm.GetWindowPlacement(&wp)) {
			if (m_dlgToDoAlarm.IsIconic()) {
				if (wp.flags & WPF_RESTORETOMAXIMIZED) {
					wp.showCmd = SW_MAXIMIZE;
				} else {
					wp.showCmd = SW_RESTORE;
				}
				m_dlgToDoAlarm.SetWindowPlacement(&wp);
			}
		}

		m_dlgToDoAlarm.SetForegroundWindow();

	} NxCatchAll("CMainFrame::ShowTodoList");
}

BOOL CMainFrame::SelectTodoListUser(long nUserID)
{
	if (!m_dlgToDoAlarm.m_hWnd)
		return FALSE;
	return m_dlgToDoAlarm.SelectTodoListUser(nUserID);
}

//DRT 5/24/2007 - PLID 25892 - Internal only dialog, simlar to todo alarm for sales staff
void CMainFrame::ShowOpportunityList()
{
	//Only to be used internally
	if(IsNexTechInternal()) {
		//This is a pointer instead of a member because I don't want the dialog instantiated in non-internal situations
		//	where it could accidentally show up to clients, or take up resources unnecessarily.
		if(m_pdlgOpportunityList == NULL) {
			//Create it!
			m_pdlgOpportunityList = new COpportunityListDlg(this);
			m_pdlgOpportunityList->Create(COpportunityListDlg::IDD);
		}

		m_pdlgOpportunityList->ShowWindow(SW_SHOWNORMAL);
		m_pdlgOpportunityList->SetForegroundWindow();
	}
}

void CMainFrame::ShowAlertDialog()
{
	// (e.lally 2009-06-08) PLID 34497 - Get Permissions - View Scheduler Module
	if(GetCurrentUserPermissions(bioSchedulerModule) & (sptView|sptViewWithPass)) {
		// TODO: Handle multiple alerts like ZoneAlarm does
		if (!m_dlgAlert.m_hWnd) {
			m_dlgAlert.Create(IDD_ALERT_DLG, CWnd::GetDesktopWindow()); // (a.walling 2012-07-10 13:43) - PLID 46648 - Transient alert window should not have a parent to steal focus
		}
		m_dlgAlert.ShowWindow(SW_SHOWNORMAL);
		m_dlgAlert.BringWindowToTop(); // (a.walling 2012-07-10 13:38) - PLID 46648 - Bring to top of z-order
		m_dlgAlert.SetForegroundWindow();
	}
}

void CMainFrame::EnableHotKeys (void)
{
	extern int	g_hotkeys_enabled;

	if (!HotKeysEnabled())
		g_hotkeys_enabled--;
}

void CMainFrame::DisableHotKeys (void)
{
	extern int g_hotkeys_enabled;

	g_hotkeys_enabled++;
}

//DRT 5/27/2004 - PLID 12610 - We should be able to determine if the hotkey was handled or not.
//	Return 0 if it was handled, non-zero if it was not handled.
//	Right now, we don't do anything with the final return value, and we don't have any specified
//	return values aside from 0, but we can easily add them in the future.
int CMainFrame::HotKey (int key)
{
	CNxView		*view = NULL;

	if (HotKeysEnabled() && IsWindowEnabled()) {	
		if (GetAsyncKeyState(VK_CONTROL) & 0xE000) {//Ctrl is down
			//TES 2004-2-2: In international layouts, the right Alt key gets called VK_CONTROL, as well as VK_MENU.
			if(!(GetAsyncKeyState(VK_RMENU) & 0xE000) || !IgnoreRightAlt()) {
					switch (key)			//check what key was pushed
					{						//key pushed
						case 'Q':
							OnPatientsModule();
							return 0;
						case 'H':	
							OnSchedulerModule();
							return 0;
						case 'W':
							OnLetterWritingModule();
							return 0;
						case 'N':
							OnContactsModule();
							return 0;
						case 'M':
							OnMarketingModule();
							return 0;
						case 'I':	
							OnInventoryModule();
							return 0;
						case 'F':	
							OnFinancialModule();
							return 0;
						case 'R':
							OnReportsModule();
							return 0;
						case 'A':	
							OnAdministratorModule();
							return 0;
						case 'G':
							OnSurgeryCenterModule();
							return 0;
						case 'L':	
							OnGoBack();
							return 0;
						case 'T':
							PostMessage(WM_COMMAND, ID_VIEW_SHOWPATIENTINFORMATION);
							return 0;
						default:
							break;
					}
			}
		}
		else {//ctrl not down
			switch (key)
			{	
				case VK_F1:
					OnManualBtn();
					return 0;
				case VK_F5:
					UpdateAllViews();
					return 0;
			}
		}

		//If we get here, the hotkey has not already been handled.  pass it down
		view = GetActiveView();//we should send this for all keys
		if (view)
			return view->Hotkey(key);
	}

	//unhandled
	return 1;
}

CNxTabView * CMainFrame::GetActiveView()
{
	CChildFrame *pFrame = GetActiveViewFrame();
	if (!pFrame)
		return NULL;
	
	CView* pActiveView = pFrame->GetActiveView();

	// (j.jones 2012-08-08 09:45) - PLID 51063 - should return NULL if the active view is not a CNxTabView
	// (r.wilson 4/12/2013) pl 54257 - Correction should return NULL if the active view is not a CNxTabView or a CReportView or LetterView (preemptive prevention :)  )
	if(pActiveView != NULL && (pActiveView->IsKindOf(RUNTIME_CLASS(CNxTabView)) || pActiveView->IsKindOf(RUNTIME_CLASS(CReportView)) || pActiveView->IsKindOf(RUNTIME_CLASS(CLetterView))) ) {
		CNxTabView *pAns;
		pAns = (CNxTabView *)(pFrame->GetActiveView());
		return pAns;
	}
	
	//couldn't find an active view
	return NULL;
}

// j.anspach 04/11/2005 PLID 13663 - Removing the ability to do your own download from Practice.
/*void CMainFrame::OnDownload() 
{
	//Start NxUpdate.exe
	STARTUPINFO si;
	::ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION pi;
	CString strPath;
	//Let's try to find NxUpdate
	//TES 6/20/03: Actually, first let's check the install path.
	strPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath");
	strPath = strPath ^ "NxUpdate.exe";
	if(!DoesExist(strPath)) {
		//Next, we'll check the place where Practice.exe is.
		GetModuleFileName(NULL, strPath.GetBuffer(MAX_PATH), MAX_PATH);
		strPath.ReleaseBuffer();
		//Some operating systems(?, I don't know whether it's the operating system or what) capitalize GetModuleFileName, some don't.
		strPath.MakeUpper();
		strPath.Replace("PRACTICE.EXE", "NXUPDATE.EXE");
		if(!DoesExist(strPath)){
			//Well, it wasn't there, let's try the working directory.
			GetCurrentDirectory(MAX_PATH, strPath.GetBuffer(MAX_PATH));
			strPath.ReleaseBuffer();
			strPath = strPath ^ "NxUpdate.exe";
			if(!DoesExist(strPath)){
				//That's it, we give up.  Let's get out of here.
				MessageBox("Could not start ActiveUpdate.\nError: Invalid file name 'NxUpdate.exe'.");
				return;
			}
		}
	}

	if(::CreateProcess(strPath, strPath.GetBuffer(MAX_PATH), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)){
		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);
	}
	else {
		//Let's tell them why.
		CString strError;	
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
		strError.ReleaseBuffer();
		MessageBox("Could not start ActiveUpdate.\nError: " + strError);
	}

	strPath.ReleaseBuffer();
}*/

void CMainFrame::OnFilter() 
{
	CFilterEditDlg dlg(NULL, fboPerson, CGroups::IsActionSupported, CGroups::CommitSubfilterAction);
	dlg.DoModal();
}

void CMainFrame::OnViewToolbarsGui() 
{
	CMainFrame *tmpWnd;
	tmpWnd = GetMainFrame();
	if (tmpWnd) {
		if (m_GUIToolBar.GetSafeHwnd()) {
			if (m_GUIToolBar.IsWindowVisible()) {
				ShowControlBar(&m_GUIToolBar, FALSE, FALSE);
			} else {
				ShowControlBar(&m_GUIToolBar, TRUE, FALSE);
			}
		}
	}
}

void CMainFrame::OnUpdateViewToolbarsGui(CCmdUI* pCmdUI) 
{
	CMainFrame *tmpWnd;
	tmpWnd = GetMainFrame();
	if (tmpWnd) {
		if (m_GUIToolBar.GetSafeHwnd()) {
			if (m_GUIToolBar.IsWindowVisible()) {
				pCmdUI->SetCheck(1);
			} else {
				pCmdUI->SetCheck(0);
			}
		}
	}
}

void CMainFrame::OnManualBtn() 
{
	// I'm worried about null pointers; modified by Bob 2-3-2000)
	CNxTabView *pActiveView = GetActiveView();

	if (pActiveView)
	{	if (pActiveView->IsKindOf(RUNTIME_CLASS(CNxTabView))) {
			CNxDialog *pDlg = pActiveView->GetActiveSheet();
			if (pDlg) {
				OpenManual(pDlg->GetManualLocation(), pDlg->GetManualBookmark());
			}
		} 
		else if (pActiveView->IsKindOf(RUNTIME_CLASS(CNxView)))//not everything is a CNxTabView
		{	CNxView *p = pActiveView;
			OpenManual (p->m_strManualLocation, p->m_strManualBookmark);
		}
	}
	//(j.anspach 06-13-2005 13:39 PLID 16662) - Instead of just throwing a message we should load up the default help page
	//else MsgBox(MB_ICONINFORMATION|MB_OK, "Help documentation for this module cannot be found.");
	else OpenManual("NexTech_Practice_Manual.chm", "System_Setup/Nextech_Installation/Things_to_Know_Before_you_Install.htm");
}

void CMainFrame::OnManualToolbarBtn()
{
	try
	{
		// (z.manning 2009-03-04 12:48) - PLID 33104 - Now when clicking on the Help icon in the
		// toolbar the user will get a menu instead of opening the help manual directly.
		CPoint pt;
		GetCursorPos(&pt);
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		UINT nFlags = MF_BYPOSITION;
		mnu.InsertMenu(nIndex++, nFlags, ID_MANUAL_BTN, "Practice &Manual");
		mnu.InsertMenu(nIndex++, nFlags, ID_HELP_CONNECTTOTECHNICALSUPPORT, "&Connect to Technical Support");
		mnu.InsertMenu(nIndex++, nFlags, IDM_VIEW_WEBINARS, "View Previously Recorded &Webinars");

		mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);

	}NxCatchAll("CMainFrame::OnManualToolbarBtn");
}

void CMainFrame::OnPDFManual()
{
	CString strDirectory = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath") ^ "Manual\\PDF";
	if(!DoesExist(strDirectory)) {
		AfxMessageBox("Unable to open path '" + strDirectory + "'.  Please contact NexTech for assistance.");
		return;
	}
	// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
	ShellExecute(GetSafeHwnd(), NULL, strDirectory, NULL, strDirectory, SW_SHOWNORMAL);
}

void CMainFrame::RestoreDocking()
{
	//DockControlBar(&m_wndMenuBar, AFX_IDW_DOCKBAR_TOP);
	DockControlBar(&m_wndToolBar, AFX_IDW_DOCKBAR_TOP);
	DockControlBarLeftOf(&m_GUIToolBar, &m_wndToolBar);
	DockControlBarBelow(&m_patToolBar, &m_wndToolBar);
	DockControlBarBelow(&m_contactToolBar, &m_wndToolBar);
	DockControlBarBelow(m_pDocToolBar, &m_wndToolBar);
}

void CMainFrame::OnSuperbillall() 
{
	try{
		// (e.lally 2009-06-08) PLID 34498 - Check permissions - View scheduler module
		if(CheckCurrentUserPermissions(bioSchedulerModule, sptView)) {
			CSuperBillDlg dlg;
			dlg.DoModal();
			if (GetActiveView()){
				GetActiveView()->UpdateView();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void SuperBill(int PatientID);

void CMainFrame::OnSuperbill()
{
	try {
		// (e.lally 2009-06-08) PLID 34498 - Check permissions - View scheduler module
		if(CheckCurrentUserPermissions(bioSchedulerModule, sptView)) {
			SuperBill(GetActivePatientID());
			if (GetActiveView()){
				GetActiveView()->UpdateView();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnEnvelope()
{
	try {
		// (e.lally 2009-06-08) PLID 34498 - Check permissions - Write patient history tab
		if(!CheckCurrentUserPermissions(bioPatientHistory, sptWrite)) {
			return;
		}

		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}

		// Get template to merge to
		// (a.walling 2007-06-14 16:02) - PLID 26342 - Support Word 2007 templates
		// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
		CString strFilter;
		// Always support Word 2007 templates
		strFilter = "Microsoft Word Templates|*.dot;*.dotx;*.dotm||";

		CFileDialog dlg(TRUE, "dot", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
		CString initialDir = GetTemplatePath("Envelopes");
		dlg.m_ofn.lpstrInitialDir = initialDir;
		if (dlg.DoModal() == IDOK) {
			
			// If the user clicked OK do the merge
			CWaitCursor wc;
			CString strTemplateName = dlg.GetPathName();
			
			/// Generate the temp table
			CString strSql;
			strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", GetActivePatientID());
			CString strMergeT = CreateTempIDTable(strSql, "ID");
			
			// Merge
			CMergeEngine mi;
			if (g_bMergeAllFields) mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;
			mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;
			
			// (z.manning, 03/05/2008) - PLID 29131 - Now have a function to load sender information.
			mi.LoadSenderInfo(FALSE);
			
			// Do the merge
			// (z.manning 2016-06-03 8:41) - NX-100806 - Check if the merge was successful
			if (mi.MergeToWord(strTemplateName, std::vector<CString>(), strMergeT))
			{
				if (GetActiveView())
					GetActiveView()->UpdateView();
			}
		}

	} NxCatchAll("Error merging envelope.");
}

void CMainFrame::OnPrescription()
{

	WritePrescription();
}

void CMainFrame::WritePrescription() {
	
	CString sql, description;

	
	try {

		//check to see that they have permissions - this will prompt a password
		if(!CheckCurrentUserPermissions(bioPatientMedication,sptCreate))
			return;

		long nMedicationID;
		
		
		//Open the Medication Selection Dialog
		CMedicationSelectDlg dlg(this);
		long nResult;
		nResult = dlg.DoModal();

		if (nResult == IDOK) {

			nMedicationID = dlg.m_nMedicationID;
		
			//Check that the patient isn't allergic to it.
			if(!CheckAllergies(GetActivePatientID(), nMedicationID)) return;

			//insert the medication into PatientMedications			
			CString strTimeToDisplay = FormatDateTimeForInterface(COleDateTime::GetCurrentTime(), NULL, dtoDate);
			CString strTimeToInsert = _Q(FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate));
			
			_RecordsetPtr rs;
			// (c.haag 2007-02-02 17:57) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
			// (d.thompson 2008-12-02) - PLID 32174 - DefaultPills is now DefaultQuantity, Description is now PatientInstructions (and parameterized)
			// (d.thompson 2009-01-15) - PLID 32176 - DrugList.Unit is now DrugStrengthUnitsT.Name, joined from DrugList.StrengthUnitID
			// (d.thompson 2009-03-11) - PLID 33481 - Actually this should use QuantityUnitID, not StrengthUnitID
			rs = CreateParamRecordset("SELECT EMRDataT.Data AS Name, PatientInstructions, DefaultRefills, DefaultQuantity, "
				"COALESCE(DrugStrengthUnitsT.Name, '') AS Unit FROM DrugList "
				"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
				"LEFT JOIN DrugStrengthUnitsT ON DrugList.QuantityUnitID = DrugStrengthUnitsT.ID "
				"WHERE DrugList.ID = {INT}", nMedicationID);

			
			FieldsPtr fields;
			fields = rs->Fields;

			//Put the variables from the recordset into local variables
			CString strName, strPatientExplanation, strUnit, strRefills, strPills;
			long nLocID = -1; //nProvID = -1;
			_variant_t varProvID = g_cvarNull;// (a.vengrofski 2009-12-18 18:46) - PLID <34890> - Added to be able to have a null provider name.

			strName = VarString(fields->Item["Name"]->Value);
			// (d.thompson 2008-12-02) - PLID 32174 - Description is now PatientInstructions
			strPatientExplanation = VarString(fields->Item["PatientInstructions"]->Value);
			strRefills = VarString(fields->Item["DefaultRefills"]->Value);
			// (d.thompson 2008-12-02) - PLID 32174 - DefaultPills is now DefaultQuantity
			strPills = VarString(fields->Item["DefaultQuantity"]->Value);
			strUnit = AdoFldString(fields, "Unit");
			

			rs->Close();

			rs = CreateParamRecordset("SELECT MainPhysician, Location FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE PatientsT.PersonID = {INT}",GetActivePatientID());
			_variant_t var;
			if(!rs->eof) {
				varProvID = rs->Fields->Item["MainPhysician"]->Value; // (a.vengrofski 2009-12-18 18:49) - PLID <34890> - varProvID is a var now.

				var = rs->Fields->Item["Location"]->Value;
				if(var.vt == VT_I4)
					nLocID = var.lVal;
			}
			rs->Close();

			if(nLocID == -1)
				nLocID = GetCurrentLocationID();

			if(varProvID.vt == VT_NULL) {
				rs = CreateParamRecordset("SELECT DefaultProviderID FROM LocationsT WHERE ID = {INT}",GetCurrentLocationID());
				if(!rs->eof) {
					varProvID = rs->Fields->Item["DefaultProviderID"]->Value;// (a.vengrofski 2009-12-18 18:50) - PLID <34890> - varProvID is a var now.
				}
				rs->Close();
			}

			//Insert all the defaults into the PatientMedications Table
			//TES 2/10/2009 - PLID 33002 - Renamed Description to PatientExplanation, PillsPerBottle to Quantity
			// (a.vengrofski 2009-12-18 18:52) - PLID <34890> - Converted to a param query for the variant varProvID
			// (j.jones 2010-01-22 11:31) - PLID 37016 - supported InputByUserID
			// (j.armen 2013-05-24 15:22) - PLID 56863 - Patient Medications has an identity.
			rs = CreateParamRecordset(
				"SET NOCOUNT ON\r\n"
				"DECLARE @PatientMedication_ID TABLE(ID INT NOT NULL)\r\n"
				"INSERT INTO PatientMedications (\r\n"
				"	PatientID, MedicationID, PatientExplanation, EnglishDescription, PrescriptionDate,\r\n"
				"	RefillsAllowed, Quantity, ProviderID, LocationID, Unit,\r\n"
				"	InputByUserID)\r\n"
				"OUTPUT inserted.ID INTO @PatientMedication_ID\r\n"
				"SELECT\r\n"
				"	{INT}, {INT}, {STRING}, {STRING}, {STRING},\r\n"
				"	{STRING}, {STRING}, {VT_I4}, {INT}, {STRING},\r\n"
				"	{INT}\r\n"
				"SET NOCOUNT OFF\r\n"
				"SELECT ID FROM @PatientMedication_ID",
				GetActivePatientID(), nMedicationID, strPatientExplanation, GetLatinToEnglishConversion(strPatientExplanation), strTimeToInsert,
				strRefills, strPills, varProvID, nLocID, strUnit, 
				GetCurrentUserID());

			long nID = AdoFldLong(rs, "ID");

			//auditing
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientPrescriptionCreated, GetActivePatientID(), "", strName, aepMedium, aetCreated);

			// (c.haag 2010-02-18 17:22) - PLID 37384 - Let the user apply the prescriptions to the current medications list.
			// (j.jones 2010-08-23 09:23) - PLID 40178 - this is user-created so the NewCropGUID is empty
			// (j.jones 2011-05-02 15:39) - PLID 43450 - pass in the patient explanation as the sig
			// (j.jones 2013-01-09 11:55) - PLID 54530 - renamed the function, it also no longer needs the Sig nor NewCropGUID
			//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
			CDWordArray arNewCDSInterventions;
			ReconcileCurrentMedicationsWithOneNewPrescription(GetActivePatientID(), nID, GetSysColor(COLOR_BTNFACE), this, arNewCDSInterventions);
			GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

			if (GetMainFrame()->GetActiveView())
				GetMainFrame()->GetActiveView()->UpdateView();

			///////////////////////////////////////
			//now merge it
			CWaitCursor pWait;

			// (j.jones 2008-06-05 11:54) - PLID 29154 - now we support different default templates
			// based on how many prescriptions are printed
			BOOL bExactCountFound = FALSE;
			BOOL bOtherCountFound = FALSE;
			CString strDefTemplate = GetDefaultPrescriptionTemplateByCount(1, bExactCountFound, bOtherCountFound);

			if(strDefTemplate.IsEmpty()) {
				AfxMessageBox("There is no default template for prescriptions.\n"
					"The prescription was created in the patient's medications tab, but could not be merged to Word.");
				return;
			}
			//if no template was found for the exact count, and there are some for other counts,
			//ask if they wish to continue or not (will use the standard default otherwise)
			else if(!bExactCountFound && bOtherCountFound) {
				if(IDNO == MessageBox("There is no default template configured for use with one prescription, "
					"but there are templates configured for other counts of prescriptions.\n\n"
					"Would you like to continue merging using the standard prescription template?",
					"Practice", MB_ICONQUESTION|MB_YESNO)) {

					AfxMessageBox("The prescription was created in the patient's medications tab, but will not be be printed.");
					return;
				}
			}

			// At this point, we know we want to do it by word merge, so make sure word exists
			if (!GetWPManager()->CheckWordProcessorInstalled()) {
				return;
			}
			
			/// Generate the temp table
			CString strSql;
			strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", GetActivePatientID());
			CString strMergeT = CreateTempIDTable(strSql, "ID");
			
			// Merge
			CMergeEngine mi;

			// (z.manning, 03/06/2008) - PLID 29131 - Need to load the sender merge fields
			if(!mi.LoadSenderInfo(TRUE)) {
				return;
			}
			
			//add this prescriptions to the merge
			mi.m_arydwPrescriptionIDs.Add(nID);

			if (g_bMergeAllFields)
				mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;

			mi.m_nFlags |= BMS_SAVE_FILE_NO_HISTORY; //save the file, do not save in history

			try {
				// Do the merge
				// (z.manning 2016-06-03 8:41) - NX-100806 - Check if the merge was successful
				if (mi.MergeToWord(GetTemplatePath("Forms", strDefTemplate), std::vector<CString>(), strMergeT))
				{
					//Update patient history here, because multiple merges per patient ID
					//will screw up the merge engine's method of doing it. But hey,
					//we get to make the description a lot better as a result!

					CString strDescription = "Prescription printed for ";
					// (c.haag 2007-02-02 17:57) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
					_RecordsetPtr rs = CreateRecordset("SELECT PatientMedications.*, EMRDataT.Data AS Name FROM PatientMedications LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
						"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
						"WHERE PatientMedications.ID = %li", (long)nID);
					if (!rs->eof) {
						CString MedicationName = CString(rs->Fields->Item["Name"]->Value.bstrVal);
						long Refills = rs->Fields->Item["RefillsAllowed"]->Value.lVal;
						//TES 2/10/2009 - PLID 33002 - Renamed PillsPerBottle to Quantity.  Also, this was treating the
						// field as a long, which it wasn't (though because they weren't using AdoFldLong it wouldn't have
						// been caught, tsk, tsk), so I fixed it to be a string.
						CString strQuantity = AdoFldString(rs, "Quantity");
						CString strUnit = AdoFldString(rs, "Unit");
						CString strExtra;
						strExtra.Format("%s, Quantity: %li %s, Refills: %li", MedicationName, strQuantity, strUnit, Refills);
						strDescription += strExtra;
					}
					rs->Close();

					// (j.jones 2008-09-04 16:39) - PLID 30288 - converted to use CreateNewMailSentEntry,
					// which creates the data in one batch and sends a tablechecker
					// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
					CreateNewMailSentEntry(GetActivePatientID(), strDescription, SELECTION_WORDDOC, mi.m_strSavedAs, GetCurrentUserName(), "", GetCurrentLocationID());
				}

			} NxCatchAll("CMainFrame::PrintPrescription");
		}
	
	}NxCatchAll("Error in CMainFrame::OnPrescription()");
}

// (j.jones 2013-01-09 13:26) - PLID 54530 - moved medication reconciliation functions to ReconcileMedicationsUtils

void CMainFrame::OnBatchMerge()
{
	try {
		if(!g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrUse)) {
			MsgBox("You must purchase the NexTrack License to use this feature.");
			return;
		}
		// (e.lally 2009-06-08) PLID 34498 - Check permissions - Write patient tracking
		if(CheckCurrentUserPermissions(bioPatientTracking, sptWrite)) {
			CBatchMergeTrackingDlg dlg(this);
			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-09-22 16:23) - PLID 40640 - Educational materials
void CMainFrame::OnEducationalMaterials()
{
	try {
		if(!CheckCurrentUserPermissions(bioPatientHistory, sptWrite)) {
			return;
		}
		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}
		CEducationalMaterialsDlg dlg(this);
		dlg.m_nPatientID = GetActivePatientID();
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}


void CMainFrame::OnUpdateNextContactBtn(CCmdUI* pCmdUI) 
{//These look messy but are really very simple

	bool enable = (m_contactToolBar.m_toolBarCombo->GetRowCount() 
		> m_contactToolBar.m_toolBarCombo->GetCurSel() + 1);
	pCmdUI->Enable(enable);
}

void CMainFrame::OnUpdatePrevContactBtn(CCmdUI* pCmdUI) 
{
	bool enable = (m_contactToolBar.m_toolBarCombo->GetRowCount() 
		&& m_contactToolBar.m_toolBarCombo->GetCurSel());
	pCmdUI->Enable(enable);
}

void CMainFrame::OnNextContactBtn() 
{
	CNxTabView *ptab = (CNxTabView *)GetOpenView(CONTACT_MODULE_NAME);

	//save what's currently there
	if (ptab) {
		ptab->GetActiveSheet()->StoreDetails();
	}

	m_contactToolBar.m_toolBarCombo->SetSelByColumn(0,
		m_contactToolBar.m_toolBarCombo->GetValue(
			m_contactToolBar.m_toolBarCombo->GetCurSel() + 1,0));
	m_contactToolBar.m_ActiveContact = m_contactToolBar.m_toolBarCombo->GetValue(m_contactToolBar.m_toolBarCombo->GetCurSel(),0).lVal;
	RefreshContactsModule();
	m_contactToolBar.OnIdleUpdateCmdUI(TRUE, 0L);

	// (k.messina 2010-04-12 11:16) - PLID 37957 lock internal contact notes
	if(IsNexTechInternal()) {
		((CContactView*)GetOpenView(CONTACT_MODULE_NAME))->CheckViewNotesPermissions();
	}
}

void CMainFrame::OnPrevContactBtn() 
{
	CNxTabView *ptab = (CNxTabView *)GetOpenView(CONTACT_MODULE_NAME);
	//save what's currently there
	if (ptab) {
		ptab->GetActiveSheet()->StoreDetails();
	}

	m_contactToolBar.m_toolBarCombo->SetSelByColumn(0,
		m_contactToolBar.m_toolBarCombo->GetValue(
			m_contactToolBar.m_toolBarCombo->GetCurSel() - 1,0));
	m_contactToolBar.m_ActiveContact = m_contactToolBar.m_toolBarCombo->GetValue(m_contactToolBar.m_toolBarCombo->GetCurSel(),0).lVal;
	RefreshContactsModule();
	m_contactToolBar.OnIdleUpdateCmdUI(TRUE, 0L);

	// (k.messina 2010-04-12 11:16) - PLID 37957 lock internal contact notes
	if(IsNexTechInternal()) {
		((CContactView*)GetOpenView(CONTACT_MODULE_NAME))->CheckViewNotesPermissions();
	}
}

/// <summary>
/// Alters the state of the program based on a received extended table checker message
/// </summary>
/// <param name="wParam">The integer parameter of the message</param>
/// <param name="lParam">The long parameter of the message</param>
void CMainFrame::ProcessExtendedTableCheckerMessage(WPARAM wParam, LPARAM lParam)
{
	NetUtils::ERefreshTable table = (NetUtils::ERefreshTable)(wParam & 0x7FFFFFFF);
	TableCheckerStats::Entry statEntry(table);
	//
	// (c.haag 2005-07-01 09:06) - PLID 15431 - Extended table checker
	// messages have detailed information in them that can be used for
	// the purpose of avoiding database accesses that give you the same
	// information.
	//
	CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;

	// Run the mainframe handlers first, since views may also respond and they 
	// will likely expect any global stuff do alread be done.
	{
		switch (table) {

		case NetUtils::CustomFieldName:
			// (b.cardillo 2006-07-06 16:12) - PLID 20737 - Handle the custom 
			// field name change by updating the global cache.
			try {
				SetCustomFieldNameCachedValue(VarLong(pDetails->GetDetailData(1), -1), VarString(pDetails->GetDetailData(2), ""));
			}
			catch (...) {
				// Ignore any exceptions, because failure here is unimportant.  One cached entry not being 
				// reset until closing and re-opening Practice is no big deal.
			}
			break;

			// (j.jones 2014-08-26 11:38) - PLID 63222 - supported Todo Ex tablecheckers
		case NetUtils::TodoList:
		{
			try {

				long nTaskID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiTaskID), -1);
				long nAssignedUserID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiAssignedUser), -1);
				TableCheckerDetailIndex::Todo_Status eTodoStatus
					= (TableCheckerDetailIndex::Todo_Status)VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiTodoStatus), (long)TableCheckerDetailIndex::Todo_Status::tddisChanged);

				//if the status is deleted, reset task ID to -1, because there is nothing to query
				if (eTodoStatus == TableCheckerDetailIndex::Todo_Status::tddisDeleted) {
					nTaskID = -1;
				}
				// (s.tullis 2015-02-20 09:05) - PLID 64602 - Reassigning a Todo task to another user sends a tablechecker for the new user only, and is never removed from the old user's To-Do Alarm unless they close and reopen.
				long nTodoListRowIndex = -1;
				if (m_dlgToDoAlarm) {// We need to see if the todo exists in the todolist
					nTodoListRowIndex = m_dlgToDoAlarm.m_List->FindByColumn(0 /*ToDoList TaskID Column */, nTaskID, -1, FALSE);
				}
				//Ignore this task if the assigned user is for somebody else.
				//if the nAssignedUserID is -1, it means it's assigned to multiple users.
				// Check to see if the todo ID is in the filtered todolist, if so reset the timer. 
				if (nAssignedUserID == -1 || nAssignedUserID == GetCurrentUserID() || nTodoListRowIndex != -1) {

					TryResetTodoTimer(nTaskID);
				}

			} NxCatchAll("Error in CMainFrm::WindowProc:WM_TABLE_CHANGED_EX:TodoList");

			break;
		}
		//TES 8/13/2014 - PLID 63520 - Added support for EX tablecheckers
		case NetUtils::PatCombo:
		{
			//TES 8/13/2014 - PLID 63520 - This is mostly copied from the old handler, except instead of checking the data we can just check our lParam
			BOOL bChangePatient = FALSE;
			long nPatientID = VarLong(pDetails->GetDetailData(CClient::pcdiPersonID), -1);

			// (c.haag 2003-08-04 16:04) - The patient is deleted. Update Outlook's appearance.
			// (a.walling 2010-10-14 15:28) - PLID 40965 - Use ReturnsRecordsParam
			if (VarLong(pDetails->GetDetailData(CClient::pcdiDeleted), 0))
			{
				PPCRefreshAppts(nPatientID);

				// (c.haag 2003-09-29 10:08) - Make sure we don't have the deleted patient selected
				if (nPatientID == GetActivePatientID())
				{
					MsgBox("The actively selected patient has been deleted by another user. Practice will select the next patient in the patient list.");
					UnselectDeletedPatientFromToolbar();
					UpdateAllViews();
				}
			}

			// If we're not looking at all patients and prospects
			if (m_Search != 3)
			{
				// Find out if this is a patient or a prospect
				CClient::PatCombo_StatusType pcst = (CClient::PatCombo_StatusType)VarLong(pDetails->GetDetailData(CClient::pcdiStatus));
				// If the status of the person matches that of our filter,
				// then we're good to update the patient dropdown
				if (((pcst == CClient::pcstPatient || pcst == CClient::pcstPatientProspect) && m_Search == 1)
					|| (pcst == CClient::pcstProspect && m_Search == 2)
					|| (pcst == CClient::pcstUnchanged && m_patToolBar.IsPersonCurrentlyInList(nPatientID))) {
					bChangePatient = TRUE;
				}
				else {
					//if the status doesn't match our filter, 
					//check to see if the patient is in our filter,
					//and if so, remove the patient (if it is not the current patient!)
					if (GetActivePatientID() != nPatientID) {

						//TES 1/6/2010 - PLID 36761 - Encapsulated in CPatientToolbar
						m_patToolBar.RemoveRowByPersonID(nPatientID);
						m_patToolBar.UpdateStatusText();
					}
				}
			}

			if (m_Include == 1) { //Active
				CClient::PatCombo_ActiveType pcat = (CClient::PatCombo_ActiveType)VarLong(pDetails->GetDetailData(CClient::pcdiActive));
				//if the patient is active, then we can update the dropdown
				if (pcat == CClient::pcatActive || (pcat == CClient::pcatUnchanged && m_patToolBar.IsPersonCurrentlyInList(nPatientID))) {
					bChangePatient = TRUE;
				}
				else if (GetActivePatientID() != nPatientID) {
					//the patient is inactive, so remove it from the list, but only if it is NOT the current patient
					//TES 1/6/2010 - PLID 36761 - Encapsulated in CPatientToolbar
					m_patToolBar.RemoveRowByPersonID(nPatientID);
					m_patToolBar.UpdateStatusText();
				}
			}

			//TES 5/14/2008 - PLID 27960 - This was checking for m_Include == 0, which it never does.
			// m_Include == 2 means they're showing "All"
			if (m_Search == 3 && m_Include == 2)
			{
				// We're looking at all patients and prospects, and all active/inactive patients,
				// so we know it's safe to change the patient
				bChangePatient = TRUE;
			}

			if (bChangePatient) {
				// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
				m_patToolBar.UpdatePatient(nPatientID);
			}

			//TES 8/13/2014 - PLID 63520 - Don't forget the rest of what the non-EX handler does
			// (c.haag 2006-05-17 16:24) - PLID 20644 - Update
			// the Send Yak Message patient dropdown
			ASSERT(m_pdlgSendYakMessage != NULL);
			if (m_pdlgSendYakMessage)
				m_pdlgSendYakMessage->HandleTableChange(nPatientID);


			// CAH 11/20/01: Unlike the other links,
			// Mirror is picky about when it refreshes
			// its lists. This code will ensure that
			// next time the mirror link is opened up,
			// it will refresh the patient lists.
			if (m_pMirrorDlg)
				m_pMirrorDlg->SetOtherUserChanged();
		}
		default:
			break;
		}
	}

	// (s.tullis 2014-08-26 09:27) - PLID 63226 - Declarations
	//if (m_dlgFollowup.GetSafeHwnd()){

	//m_dlgFollowup.OnTableChangedEx(table, lParam);
	//}


	// (j.jones 2006-10-02 15:39) - PLID 22604 - let the room manager know of changes
	if (m_pRoomManagerDlg) {
		if (m_pRoomManagerDlg->m_hWnd) {
			m_pRoomManagerDlg->OnTableChangedEx(table, lParam);
		}
	}

	//always send ex messages to the todo alarm, even if it is not visible
	if (m_dlgToDoAlarm.GetSafeHwnd()) {
		m_dlgToDoAlarm.OnTableChangedEx(table, lParam);
	}

	// (r.gonet 08/26/2014) - PLID 63221 - Send table checkers to the Labs Follow-Up
	// window if it's open
	if (m_pLabFollowupDlg) {
		if (IsWindow(m_pLabFollowupDlg->GetSafeHwnd())) {
			m_pLabFollowupDlg->SendMessage(WM_TABLE_CHANGED_EX, wParam, lParam);
		}
	}

	// (j.jones 2014-08-04 16:16) - PLID 63181 - pass Ex tablecheckers to all open PIC windows
	{
		POSITION pos = m_lstpPicContainerDlgs.GetHeadPosition();
		while (pos) {
			CPicContainerDlg *pPicContainer = m_lstpPicContainerDlgs.GetNext(pos);
			if (pPicContainer->GetSafeHwnd()) {
				pPicContainer->OnTableChangedEx(table, lParam);
			}
		}
	}

	//TES 8/13/2014 - PLID 63519 - The Problem List dialog also needs this
	if (m_pEMRProblemListDlg) {
		if (m_pEMRProblemListDlg->m_hWnd) {
			m_pEMRProblemListDlg->OnTableChangedEx(table, lParam);
		}
	}

	// (z.manning, 03/03/2008) - PLID 29177 - Make sure any windows that are registered to receive
	// table checker messages get this message.
	CArray<HWND, HWND> m_aryhwndDestroyed;
	for (int nCheckerWndIndex = 0; nCheckerWndIndex < m_arCheckerWnds.GetSize(); nCheckerWndIndex++) {
		CWnd* pWnd = FromHandlePermanent(m_arCheckerWnds.GetAt(nCheckerWndIndex));
		if (pWnd) {
			pWnd->SendMessage(WM_TABLE_CHANGED_EX, table, lParam);
		}
		else {
			//They seem to have exited without telling us.  Bastards.
			m_aryhwndDestroyed.Add(m_arCheckerWnds.GetAt(nCheckerWndIndex));
		}
	}
	for (int nDestroyedWndIndex = m_aryhwndDestroyed.GetSize() - 1; nDestroyedWndIndex >= 0; nDestroyedWndIndex--) {
		UnrequestTableCheckerMessages(m_arCheckerWnds.GetAt(nDestroyedWndIndex));
	}

	// Now pass the mesage on to all the views
	int nCount = m_pView.GetSize();
	for (int i = 0; i<nCount; i++) {
		// Loop through all open views
		CView* pView = (CView *)m_pView[i];
		switch (pView->SendMessage(WM_TABLE_CHANGED_EX, table, lParam)) {
		case 1:
			i = nCount; // Exit the loop
			break;
		case 0:
		default:
			break;
		}
	}

	// (c.haag 2004-09-03 10:54) - Tell NxServer we are done processing
	// our table checker message
	// (c.haag 2008-09-11 09:09) - PLID 31333 - This is now handled in packet reception
	// messages
	delete pDetails;
}

/// <summary>
/// Alters the state of the program based on a received table checker message
/// </summary>
/// <param name="wParam">The integer parameter of the message</param>
/// <param name="lParam">The long parameter of the message</param>
void CMainFrame::ProcessTableCheckerMessage(WPARAM wParam, LPARAM lParam)
{
	NetUtils::ERefreshTable table = (NetUtils::ERefreshTable)(wParam & 0x7FFFFFFF);
	TableCheckerStats::Entry statEntry(table);

	switch (table)
	{
	case NetUtils::UserDefinedSecurityObject:
		try {
			// (c.haag 2006-05-23 09:14) - PLID 20609 - Unlike any other table checker message at
			// the present time, it is a guarantee that if we get here, we got a table checker from
			// someone else and not ourselves
			LoadUserDefinedPermissions((long)lParam);
		} NxCatchAll("Error in CMainFrm::WindowProc:UserDefinedSecurityObject");
		break;

	case NetUtils::EMRMasterT:
		try {
			if (m_pActiveEMRDlg) {
				m_pActiveEMRDlg.Refresh();
				// (j.gruber 2014-01-30 14:39) - PLID 60561 - just refresh, don't bring it to the front
				//m_pActiveEMRDlg.BringWindowToTop();
				//m_pActiveEMRDlg.SetForegroundWindow();
			}
		} NxCatchAll("Error in CMainFrm::WindowProc:EMRMasterT");
		break;

	case NetUtils::AppointmentsT:
		// (c.haag 2003-08-04 13:47) - Update the appointment in Outlook only.
		// (c.haag 2006-01-19 17:29) - PLID 18711 - This is no longer necessary because the
		// NexPDA link gets table checkers, too.
		//if ((long)lParam > 0)
		//	PPCRefreshAppt((long)lParam);
		break;

	case NetUtils::TodoList:
	{
		try {

			// (j.jones 2014-08-26 11:48) - PLID 63222 - Moved this code to a modular function.
			// This is always called because we do not know if the todo was for the current user or not.
			TryResetTodoTimer(lParam);

		} NxCatchAll("Error in CMainFrm::WindowProc:WM_TABLE_CHANGED:TodoList");

		break;
	}

	case NetUtils::Coordinators:
	{
		try {

			if (m_pDocToolBar->m_hWnd) {
				m_pDocToolBar->OnTableChanged(table, lParam);

			}
		} NxCatchAll("Error in CMainFrm::WindowProc:Coordinators");
		break;

	}break;

	case NetUtils::CustomContacts:
		try {
			if (lParam != -1)
				m_contactToolBar.ChangeContact(lParam);
			else
				m_contactToolBar.Requery();
		} NxCatchAll("Error in CMainFrm::WindowProc:CustomContacts");
		break;
		// (s.tullis 2015-10-26 14:58) - PLID 67337 - Need to update pat dropdown if a Birthdate has changed
	case NetUtils::PatientBDate:
	{
		try {
			if (lParam != -1)
			{
				if (m_patToolBar.IsPersonCurrentlyInList(lParam)) {
					m_patToolBar.UpdatePatient(lParam);
				}
			}
			else {
				//we should not be sending this without patID 
				ASSERT(FALSE);
			}
		}NxCatchAll("Error in CMainFrm::WindowProc:PatientBDate")
			break;
	}
	case NetUtils::PatCombo:
	{
		try {
			if (lParam != -1)
			{
				//TES 8/13/2014 - PLID 63520 - If we get the non-EX tablechecker, then we know that nothing about this patient changed to affect
				// its visibility (it wasn't deleted, and didn't have its Active flag or Status changed). So if it's there, we can just go ahead and update it.
				if (m_patToolBar.IsPersonCurrentlyInList(lParam)) {
					m_patToolBar.UpdatePatient(lParam);
				}

			}
			else
				m_patToolBar.Requery();

			// (c.haag 2006-05-17 16:24) - PLID 20644 - Update
			// the Send Yak Message patient dropdown
			ASSERT(m_pdlgSendYakMessage != NULL);
			if (m_pdlgSendYakMessage)
				m_pdlgSendYakMessage->HandleTableChange(lParam);


			// CAH 11/20/01: Unlike the other links,
			// Mirror is picky about when it refreshes
			// its lists. This code will ensure that
			// next time the mirror link is opened up,
			// it will refresh the patient lists.
			if (m_pMirrorDlg)
				m_pMirrorDlg->SetOtherUserChanged();

		} NxCatchAll("Error in CMainFrm::WindowProc:PatCombo");
		break;
	}
	case NetUtils::MirrorPatients:
	{
		try {
			if (m_pMirrorDlg)
				m_pMirrorDlg->SetOtherUserChanged();
		} NxCatchAll("Error in CMainFrm::WindowProc:MirrorPatients");
		break;
	}
	case NetUtils::Providers:
	{
		try {
			if (m_pDocToolBar->m_hWnd) {
				m_pDocToolBar->OnTableChanged(table, lParam);
			}
		} NxCatchAll("Error in CMainFrm::WindowProc:Providers");

		break;
	}
	case NetUtils::CPTCategories:
	{
		try {
			if (m_pDocToolBar->m_hWnd) {
				m_pDocToolBar->OnTableChanged(table, lParam);
			}
		} NxCatchAll("Error in CMainFrm::WindowProc:CPTCategories");
		break;
	}

	case NetUtils::LocationsT:
	{
		try {
			if (m_pDocToolBar->m_hWnd) {
				m_pDocToolBar->OnTableChanged(table, lParam);
			}
		} NxCatchAll("Error in CMainFrm::WindowProc:LocationsT");
		break;
	}
	case NetUtils::FiltersT:
	{
		try {
			if (m_pDocToolBar->m_hWnd) {
				m_pDocToolBar->OnTableChanged(table, lParam);
			}
		} NxCatchAll("Error in CMainFrm::WindowProc:FiltersT");
		break;
	}

	case NetUtils::HL7SettingsT:
	{
		try {
			//TES 12/4/2006 - PLID 23737 - Tell the global cache that we need to reload this group from data.
			//TES 6/24/2011 - PLID 44261 - New functions for accessing HL7 Settings
			RefreshHL7Group(lParam);

			// (z.manning 2008-07-16 15:46) - PLID 30490 - Need to update the schedule groups we track.
			if (lParam == -1) {
				LoadScheduleHL7GroupIDs();
			}
			else {
				// (z.manning 2008-07-16 15:51) - PLID 30490 - The call below to GetExportAppts
				// will result in a data access to reload the HL7 group since we just unloaded 
				// it, however, once it's loaded it get cached (until it changes again) which
				// means that we won't have to load it the next time we do something in the schedule
				// that results in sending an HL7 message.
				// (z.manning 2008-08-19 12:35) - PLID 30490 - We also need to make sure this settings
				// group still exists!
				_RecordsetPtr prsExists = CreateParamRecordset(
					"SELECT ID FROM HL7SettingsT WHERE ID = {INT}"
					, lParam);
				//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
				if (!prsExists->eof && GetHL7SettingBit(lParam, "ExportAppts")) {
					EnsureInScheduleHL7GroupIDs(lParam);
				}
				else {
					EnsureNotInScheduleHL7GroupIDs(lParam);
				}
			}
		} NxCatchAll("Error in CMainFrm::WindowProc:HL7SettingsT");
		break;
	}

	case NetUtils::HL7MessageQueueT:
	{
		try {
			// (j.jones 2008-04-21 14:55) - PLID 29597 - see if we need to
			// notify the user of pending messages
			TryNotifyUserPendingHL7Messages();
		} NxCatchAll("Error in CMainFrm::WindowProc:HL7MessageQueueT");
		break;
	}

	case NetUtils::TimeLogStatus:
	{
		// r.galicki  6/10/2008	-	PLID 26192	- if user's log has been modified, update flag
		try {
			long nUserID = (long)lParam;
			if (GetCurrentUserID() == nUserID)
			{
				_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM UserTimesT WHERE UserID = {INT} AND Checkout IS NULL", nUserID);
				if (rs->eof)
				{
					m_bLoggingTime = false;
				}
				else
				{
					m_bLoggingTime = true;
				}
			}
		} NxCatchAll("Error in CMainFrm::WindowProc:TimeLogStatus");
		break;
	}

	case NetUtils::SecurityGroupsT:
		try {

			//TES 1/12/2010 - PLID 36761 - We can no longer trust our cached list of groups that we don't have access to.
			m_bBlockedGroupsValid = false;

			// (j.jones 2010-01-14 15:02) - PLID 35775 - do not clear m_arEmergencyAccessedPatientIDs here,
			// as once they acquired access to the patient through the emergency access ability, they keep it
			// until they log out of Practice

			//TES 1/18/2010 - PLID 36895 - Need to refresh the patient toolbar, as some demographic information may now
			// be visible.
			m_patToolBar.Requery();

			// (j.gruber 2010-10-04 14:27) - PLID 40415 - also need to refresh the resentry for the same reason, not sure why this wasn't done before
			//but now we definately need it in case they are showing security groups
			CSchedulerView* pView = (CSchedulerView*)GetOpenView(SCHEDULER_MODULE_NAME);
			if (pView && pView->GetResEntry()) {
				pView->GetResEntry()->RequeryPersons();
			}

		}NxCatchAll("Error in CMainFrm::WindowProc::SecurityGroupsT");
		break;

		// (j.jones 2010-02-10 17:32) - PLID 37224 - if EMRImageStampsT changed, simply clear our existing cache,
		// it will be reloaded the next time it is needed
	case NetUtils::EMRImageStampsT:

		ClearCachedEMRImageStamps();
		m_bEMRImageStampsLoaded = FALSE;
		break;

		// (z.manning 2011-07-07 10:19) - PLID 44421 - Similar to stamps, coding groups are cached so if they changed
		// then simply clear the cache and they will be reloaded when needed.
	case NetUtils::EmrCodingGroupsT:
		ClearCachedEmrCodingGroups();
		break;
	case NetUtils::PatientMedications:
		try {
			// (j.fouts 2013-01-28 09:02) - PLID 54472 - Refresh when this table changes
			RefreshNexERxNeedingAttention();
		} NxCatchAll("Error in MainFrm::OnTableChanged:PatientMedications");
		break;
	case NetUtils::RenewalRequests:
		try {
			// (j.fouts 2013-01-28 09:02) - PLID 54472 - Refresh when this table changes
			RefreshNexERxNeedingAttention();
		} NxCatchAll("Error in MainFrm::OnTableChanged:RenewalRequests");
		break;
	}

	// Send the message to all the views
	int nCount = m_pView.GetSize();
	for (int i = 0; i<nCount; i++) {
		// Loop through all open views
		CView* pView = (CView *)m_pView[i];
		switch (pView->SendMessage(WM_TABLE_CHANGED, table, lParam)) {
		case 1:
			i = nCount; // Exit the loop
			break;
		case 0:
		default:
			break;
		}
	}

	// (j.jones 2006-10-02 15:39) - PLID 22604 - let the room manager know of changes
	if (m_pRoomManagerDlg) {
		if (m_pRoomManagerDlg->m_hWnd) {
			m_pRoomManagerDlg->OnTableChanged(table, lParam);
		}
	}

	// (j.jones 2008-07-17 08:59) - PLID 30730 - let the problem list know of changes
	if (m_pEMRProblemListDlg) {
		if (m_pEMRProblemListDlg->m_hWnd) {
			m_pEMRProblemListDlg->OnTableChanged(table, lParam);
		}
	}

	//always send tablecheckers to the todo alarm, even if it is not visible
	if (m_dlgToDoAlarm.GetSafeHwnd()) {
		m_dlgToDoAlarm.OnTableChanged(table, lParam);
	}

	// (c.haag 2010-07-20 13:50) - PLID 30894 - Send table checkers to the Labs Follow-Up
	// window if it's open
	if (m_pLabFollowupDlg) {
		if (IsWindow(m_pLabFollowupDlg->GetSafeHwnd())) {
			m_pLabFollowupDlg->SendMessage(WM_TABLE_CHANGED, wParam, lParam);
		}
	}

	// (j.armen 2012-02-27 12:23) - PLID 48303
	if (m_pRecallsNeedingAttentionDlg) {
		if (IsWindow(m_pRecallsNeedingAttentionDlg->GetSafeHwnd())) {
			m_pRecallsNeedingAttentionDlg->SendMessage(WM_TABLE_CHANGED, wParam, lParam);
		}
	}
	// (d.lange 2011-06-02 10:18) - PLID 43253 - Allow for the Device Import dialog to receive table checkers
	if (DeviceImport::GetMonitor().GetImportDlg()) {
		if (IsWindow(DeviceImport::GetMonitor().GetImportDlg()->GetSafeHwnd())) {
			DeviceImport::GetMonitor().GetImportDlg()->SendMessage(WM_TABLE_CHANGED, wParam, lParam);
		}
	}
	// (c.haag 2006-03-09 17:40) - PLID 19208 - This is not necessary
	//#pragma TODO("(t.schneider 1/13/2006) PLID 19208 - Implement this (if needed) in the new EMR.")
	/*//TES 7/1/2004: Send it to any open EMRs.
	for (i = 0; i < m_arOpenEMRGroups.GetSize(); i++) {
	if(m_arOpenEMRGroups.GetAt(i)->GetSafeHwnd()) {
	m_arOpenEMRGroups.GetAt(i)->SendMessage(message, table, lParam);
	}
	}

	if(m_pEmnTemplateDlg)
	m_pEmnTemplateDlg->PostMessage(WM_TABLE_CHANGED, table, lParam);*/

	// (b.cardillo 2006-06-08 17:51) - PLID 14292 - Instead of just the one PIC 
	// container, we now notify all PIC containers of the message.
	{
		POSITION pos = m_lstpPicContainerDlgs.GetHeadPosition();
		while (pos) {
			CPicContainerDlg *pPicContainer = m_lstpPicContainerDlgs.GetNext(pos);
			if (pPicContainer->GetSafeHwnd()) {
				pPicContainer->OnTableChanged(table, lParam);
			}
		}
	}

	//Now, send the message to anyone else who wants it.
	CArray<HWND, HWND> m_aryhwndDestroyed;
	for (int nCheckerWndIndex = 0; nCheckerWndIndex < m_arCheckerWnds.GetSize(); nCheckerWndIndex++) {
		CWnd* pWnd = FromHandlePermanent(m_arCheckerWnds.GetAt(nCheckerWndIndex));
		if (pWnd) {
			pWnd->SendMessage(WM_TABLE_CHANGED, table, lParam);
		}
		else {
			//They seem to have exited without telling us.  Bastards.
			m_aryhwndDestroyed.Add(m_arCheckerWnds.GetAt(nCheckerWndIndex));
		}
	}
	for (int nDestroyedWndIndex = m_aryhwndDestroyed.GetSize() - 1; nDestroyedWndIndex >= 0; nDestroyedWndIndex--) {
		UnrequestTableCheckerMessages(m_arCheckerWnds.GetAt(nDestroyedWndIndex));
	}
}

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	NxSocketUtils::HandleMessage(GetSafeHwnd(), message, wParam, lParam);
	NxBackupUtils::HandleMessage(GetSafeHwnd(), message, wParam, lParam);
	// (a.walling 2009-06-08 12:47) - PLID 34512 - This is all handled in CWinApp::PreTranslateMessage and CWinApp::ProcessMessageFilter
	//TryResetAutoLogOffTimer(message);

	switch (message)
	{
		case WM_MENUCHAR: //TODO figure out what this does -BVB
		{	LRESULT res = CNxMDIFrameWnd::WindowProc(message, wParam, lParam);
			if (res == 0) 
				res = -1;
			return res;
		}

		case WM_TCPIP:	// Network code
			CClient::ProcessMessage(wParam, lParam);
			break;

		case NXM_SOCKET_DISCONNECTED:
			try {
				NxSocketUtils::Disconnect(g_hNxServerSocket);
			}
			catch (CNxException* e) {
				e->Delete();
			}
			catch (...)
			{
			}
			g_hNxServerSocket = NULL;
			g_propManager.SetNxServerSocket(NULL);
			// (c.haag 2006-10-12 12:38) - PLID 22731 - Notify the PracYakker that
			// we were disconnected from NxServer
			ASSERT(m_pdlgMessager);
			if (m_pdlgMessager)
				m_pdlgMessager->SendMessage(NXM_NXSERVER_DISCONNECTED);

			// (c.haag 2006-10-12 12:03) - PLID 22731 - When we lose our connection
			// to NxServer, we need to try to reconnect at a random time interval
			// within five minutes. By staggering reconnection times, we minimize the
			// burden on NxServer.
			//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly cast as a UINT
			srand((UINT)time(NULL));
			SetTimer(IDT_NXSERVER_RECONNECT, (rand() % (5*60*1000)), NULL);
			break;

		// (c.haag 2004-01-06 09:35) - Right now we use the NxSocketUtils
		// engine strictly for backup messages. Until we fully migrate NxSocketUtils
		// into practice, we have to at least make sure that when our NxSocketUtils
		// connection gets a table checker message, that it bounces back a processed
		// indicator so NxServer doesn't unnecessarily delay.
		case NXM_PACKET_RECEIVED:
			{
				try {
					CNxSocket* pSocket = (CNxSocket*)wParam;
					_NxHeader* pPacket = (_NxHeader*)lParam;
					void* pActualPacket = (void*)(pPacket+1);
					switch (pPacket->type)
					{
					case PACKET_TYPE_REFRESH_TABLE: {
						// (c.haag 2006-05-25 14:35) - PLID 20812 - We now process table checkers
						// (c.haag 2008-09-11 10:05) - PLID 31333 - Set the last parameter to TRUE so it will send a packet to NxServer when finished
						// (a.walling 2009-07-10 09:37) - PLID 33644 - Only need to ACK if we are using the old stagger method
						CClient::OnRefreshTable(pSocket->GetIP32(), (PACKET_REFRESH_TABLE *)pActualPacket, pPacket->wSize, m_bNxServerBroadcastStagger ? TRUE : FALSE);
						} break;
					//
					// (c.haag 2006-05-25 14:07) - PLID 20812 - I copied this code over from Client.cpp.
					// Many of the functions were dead code, so I did not include them
					//
					case PACKET_TYPE_IM_MESSAGE:
						// CAH 11/30: This needs to be SendMessage because who knows what will be in pActualPacket between now
						// and when the message is gotten!!

						// (a.walling 2007-05-04 10:34) - PLID 4850 - Messager dialog is now a pointer
						// if we don't have the Messager dialog, then we are in the middle of switching users, and can safely ignore
						if (m_pdlgMessager)
							m_pdlgMessager->SendMessage(NXM_IM_MESSAGE, (WPARAM)pPacket->wSize, (LPARAM)pActualPacket);
						break;

					case PACKET_TYPE_IM_USERLIST:
						// CAH 11/30: This needs to be SendMessage because who knows what will be in pActualPacket between now
						// and when the message is gotten!!

						// (a.walling 2007-05-04 10:34) - PLID 4850 - Messager dialog is now a pointer
						// if we don't have the Messager dialog, then we are in the middle of switching users, and can safely ignore
						// (a.walling 2009-07-10 09:39) - PLID 33644 - Does not cause any data access, does not need to be staggered
						if (m_pdlgMessager)
							m_pdlgMessager->SendMessage(NXM_IM_USERLIST, (WPARAM)pPacket->wSize, (LPARAM)pActualPacket);
						// (c.haag 2008-09-11 12:57) - PLID 31347 - Acknowledge a response to NxServer
						// (a.walling 2009-07-10 09:37) - PLID 33644
						if (m_bNxServerBroadcastStagger) {
							try {
								CClient::Send(PACKET_TYPE_IMUL_PROCESSED, NULL, 0);
							}
							NxCatchAll("Error in CMainFrm::WindowProc:NXM_PACKET_RECEIVED:NXM_IM_USERLIST");	
						}					
						break;

					case PACKET_TYPE_CHAT_MESSAGE:
						// (a.walling 2007-05-04 10:34) - PLID 4850 - Messager dialog is now a pointer
						// if we don't have the Messager dialog, then we are in the middle of switching users, and can safely ignore
						if (m_pdlgMessager)
							m_pdlgMessager->SendMessage(NXM_CHAT_MESSAGE, (WPARAM)pPacket->wSize, (LPARAM)pActualPacket);
						break;

					case PACKET_TYPE_ALERT:
						{
							_ALERT_MESSAGE* pAlert = (_ALERT_MESSAGE*)pActualPacket;
							//TES 6/9/2008 - PLID 23243 - Copy the resource ids from the packet into a variable length array
							// that we can pass through to the function.
							CDWordArray dwaResourceIDs;
							for(int i = 0; i < 5; i++) {
								if(pAlert->dwResourceIDs[i] != 0) dwaResourceIDs.Add(pAlert->dwResourceIDs[i]);
							}
							AddAlert(pAlert->szMsg, dwaResourceIDs, pAlert->bCheckData, (long)pAlert->dwAppointmentID, pAlert->eAlert, pAlert->ip);

						}
						break;

					case PACKET_TYPE_CONFIGRT_UPDATE:
						// (c.haag 2005-11-01 15:56) - PLID 18091 - We now dispatch ConfigRT update messages
						// to the CNxPropManager. These messages are sent by the CNxPropManager to update cache
						// without actually changing data.
						g_propManager.OnConfigRTPacket(pPacket);

						// (c.haag 2005-11-14 11:04) - Now, for legacy support, post a ConfigRT table change
						// message
						// (j.jones 2014-08-08 13:34) - PLID 63250 - we no longer have a ConfigRT tablechecker
						//PostMessage(WM_TABLE_CHANGED, NetUtils::ConfigRT, -1);
						break;
					// (r.gonet 12/03/2012) - PLID 54117 - Added handling.
					case PACKET_TYPE_HL7_EXPORT_RESPONSE:
						{
							// (r.gonet 12/03/2012) - PLID 54117 WM_TABLE_CHANGED- Pass the packet to our asynchronous client, so it can handle it and get us the right response.
							_PACKET_HL7_EXPORT_RESPONSE* p = (_PACKET_HL7_EXPORT_RESPONSE*)pActualPacket;
							m_pHL7Client->OnHL7ExportResponsePacket(p);
						}
						break;

					//TES 11/12/2015 - PLID 67500 - Handle the MULTIPLE packet type
					case PACKET_TYPE_HL7_MULTIPLE_EXPORT_RESPONSE:
					{
						_PACKET_HL7_MULTIPLE_EXPORT_RESPONSE* p = (_PACKET_HL7_MULTIPLE_EXPORT_RESPONSE*)pActualPacket;
						m_pHL7Client->OnHL7MultipleExportResponsePacket(p);
					}
					break;

					default:
						break;
					}
				} NxCatchAll("Error in CMainFrm::WindowProc:NXM_PACKET_RECEIVED");
			}
			break;

			// CH 4/5/01
		case WM_BARCODE_OPEN_FAILED:
		case WM_BARCODE_SCAN:
		case WM_BARCODE_OVERFLOW:
		case WM_ENQUEUE_BARCODE:	// (j.jones 2008-12-23 13:28) - PLID 32545
			OnBarcodeEvent(message, wParam, lParam);
			break;

		// (j.jones 2008-12-30 11:33) - PLID 32585 - just for total coverage,
		// handle the WM_KICKIDLE event here for when MainFrm is idle
		case WM_KICKIDLE:
			//the lParam is the time
			Idle((int)lParam);
			break;

		// (a.wetta 2007-03-15 17:19) - PLID 25234 - Handle messages being sent by the OPOS MSR device
		case WM_MSR_DATA_EVENT:
		case WM_MSR_INIT_COMPLETE:
		case WM_MSR_STATUS_INFO:
			OnOPOSMSRDeviceEvent(message, wParam, lParam);
			break;
		
		//(a.wilson 2012-1-10) PLID 47517 - possible messages sent for the scanner device thread.
		case NXM_BARSCAN_DATA_EVENT:
		case NXM_BARSCAN_INIT_COMPLETE:
			OnOPOSBarcodeScannerEvent(message, wParam, lParam);
			break;

		case WM_PRIVATETAPIEVENT:
			OnTapiEvent((TAPI_EVENT)wParam, (IDispatch*)lParam);
			break;

		// (r.gonet 2016-05-19 18:17) - NX-100695 - Handle when a user connects or disconnects
		// locally or remotely.
		case WM_WTSSESSION_CHANGE:
		{
			switch (wParam) {
			case WTS_CONSOLE_CONNECT:
			case WTS_REMOTE_CONNECT:
			case WTS_CONSOLE_DISCONNECT:
			case WTS_REMOTE_DISCONNECT:
				// (r.gonet 2016-05-19 18:17) - NX-100695 - Certain connection specific things may have changed
				// such as the client computer name or protocol. Thus the cache is now invalid.
				RdsApi::Instance().InvalidateCache();
				// And the ConfigRT cache may also be invalid since some settings are machine specific.
				g_propManager.FlushRemotePropertyCache();
				
				// (r.gonet 2016-05-24 15:54) - NX-100732 - Update the patients view. The NexPhoto tab at least needs to know the new system name.
				CNxTabView* pPatientsView = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
				if (pPatientsView != NULL) {
					pPatientsView->SendMessage(WM_SYSTEM_NAME_UPDATED);
				}

				break;
			}
		}
			break;

		// (j.gruber 2007-07-17 13:04) - PLID 26710 - making pin pads work with Practice
		case WM_PINPAD_MESSAGE_READ_TRACK_DONE:
		case WM_PINPAD_MESSAGE_INTERAC_DEBIT_DONE:
		case WM_PINPAD_OPEN_FAILED:
		case NXM_INITIALIZE_PINPAD:
			OnPinPadDeviceEvent(message, wParam, lParam);
			break;


		case NXM_TWAIN_XFERDONE:
			// (b.cardillo 2006-04-05 10:26) - PLID 17605 - If a CSelectImageDlg has registered 
			// itself as on-screen, then we need to forward the message from NxTwain on to it.
			// NOTE: This is not to be used for general code, it is only a work-around for plid 
			// 17605.  Once NxTwain is fixed up via 20009 this will no longer be necessary 
			// because it can be done the correct way.  This limit is why I've named the member 
			// variable and functions specifically containing "SelectImageDlg", so it won't be 
			// tempting to use in more generally.  If you find yourself wanting to use this for 
			// some other kind of dialog or some other situation, please refer to pl item 2009 
			// for correct usage of NxTwain.
			// (a.walling 2008-07-24 17:50) - PLID 30836 - Made this more generic by supporting
			// any CWnd rather than a specific class
			{
				POSITION p = m_mapTwainDlgsWaiting.GetStartPosition();
				while (p) {
					CWnd *pWnd = NULL;
					LONG nCountForThisDlg = 0;
					m_mapTwainDlgsWaiting.GetNextAssoc(p, pWnd, nCountForThisDlg);
					if (nCountForThisDlg > 0 && IsWindow(pWnd->GetSafeHwnd())) {
						pWnd->SendMessage(NXM_TWAIN_XFERDONE, wParam, lParam);
					}
				}
			}
			
			OnUpdateView();
			break;

		case NXM_G1THUMB_PROCESSING_COMPLETE:
			{
				// Send the message to all the views
				int nCount = m_pView.GetSize();
				for (int i=0; i<nCount; i++) {
					// Loop through all open views
					CView* pView = (CView *)m_pView[i];
					pView->SendMessage(message, wParam, lParam);
				}
			}
			break;

		case NXM_CALLERIDNUMBER:
			{
				BSTR str = (BSTR)lParam;
				SysFreeString((BSTR)lParam);

				//update the global string with the value passed from the lParam
				m_strIncomingCallNum = str;

				if (m_strIncomingCallNum.GetLength())
					m_strIncomingCallNum = "(" + m_strIncomingCallNum.Left(3) + ")" + m_strIncomingCallNum.Mid(3, 3) + "-" + m_strIncomingCallNum.Right(4);
				else
					m_strIncomingCallNum = "(private)";
				
				//we must de-allocate the space for the string because the thread that posted this message
				//is most likely no longer running
				SysFreeString(str);

			break;
			}
		
		case NXM_BRING_WINDOW_TO_TOP:
			{
				if (lParam) {
					((CWnd *)lParam)->BringWindowToTop();
				} else {
					BringWindowToTop();
				}
			}
			break;

		case NXM_OUTLOOKLINK_APPTMODIFIED:
			// An appointment was added, edited or deleted
			// from Microsoft Outlook
			// (a.wilson 2014-08-12 11:23) - PLID 63199 - use ex table checker.
			CClient::RefreshAppointmentTable(wParam);
			break;

		// (a.walling 2009-07-10 09:44) - PLID 33644 - This message indicates a table checker is available
		case WM_PENDING_TABLE_CHANGED_EX:
		case WM_PENDING_TABLE_CHANGED:
		{
			try {

				//convert the pending tablechanged message to an actual tablechanged message
				UINT messageToSend = WM_TABLE_CHANGED;
				if (message == WM_PENDING_TABLE_CHANGED_EX) {
					messageToSend = WM_TABLE_CHANGED_EX;
				}

				// (j.jones 2014-08-20 09:40) - PLID 63427 - Given a tablechecker type,
				// returns true if the tablechecker should be processed immediately or
				// added to our staggered queue.
				// Will always return true if the local stagger is disabled.
				if (ShouldImmediatelySendTablechecker((NetUtils::ERefreshTable)wParam)) {
					// revert to old behaviour, since NxServer should be staggering for us
					// (or we explicitly disabled the client-side staggering)
					return SendMessage(messageToSend, wParam, lParam);
				} else {					
					return QueuePendingMessage(messageToSend, wParam, lParam);
				}
			} NxCatchAll("Error in CMainFrm::WindowProc:WM_PENDING_TABLE_CHANGED|WM_PENDING_TABLE_CHANGED_EX");
		}
		break;

		case WM_TABLE_CHANGED_EX:
		{
			// (a.walling 2009-07-13 08:57) - PLID 33644 - Log if performance is slow
			CPerformanceWarning pw(FormatString("TableChangedEx(0x%08x, 0x%08x)", wParam, lParam), 2000);
			// (c.haag 2008-09-11 10:01) - PLID 31333 - We now have a flag that indicates whether
			// to send a table checker received response to NxServer. This flag is the MSB of wParam.
			NetUtils::ERefreshTable table = (NetUtils::ERefreshTable)(wParam & 0x7FFFFFFF);
			BOOL bAckToNxServer = (wParam & 0x80000000) ? TRUE : FALSE;
			
			// (a.walling 2009-07-14 12:30) - PLID 33644 - Add this message to the logging for PLID 34803
			CClient::PLID34803_AddTableChecker(table);

			try 
			{
				// Only process table checker messages if a user is logged in
				if (NULL != GetCurrentUserHandle())
				{
					ProcessExtendedTableCheckerMessage(wParam, lParam);
				}

			} NxCatchAll("Error in CMainFrm::WindowProc:WM_TABLE_CHANGED_EX");

			// (c.haag 2008-09-11 09:58) - PLID 31333 - Confirm that we're done processing the table
			// checker to NxServer 
			if (bAckToNxServer) {
				try {
					TableCheckerStats::Entry statEntry((NetUtils::ERefreshTable)(0xffff0000 | table));
					CClient::Send(PACKET_TYPE_TC_PROCESSED, NULL, 0);
				}
				NxCatchAll("Error in CMainFrm::WindowProc:WM_TABLE_CHANGED_EX:AckToNxServer");
			}
			break;
		}

		case WM_TABLE_CHANGED:
		{
			// (a.walling 2009-07-13 08:57) - PLID 33644 - Log if performance is slow
			CPerformanceWarning pw(FormatString("TableChanged(0x%08x, 0x%08x)", wParam, lParam), 2000);
			// (c.haag 2008-09-11 10:01) - PLID 31333 - We now have a flag that indicates whether
			// to send a table checker received response to NxServer. This flag is the MSB of wParam.
			NetUtils::ERefreshTable table = (NetUtils::ERefreshTable)(wParam & 0x7FFFFFFF);
			BOOL bAckToNxServer = (wParam & 0x80000000) ? TRUE : FALSE;
			
			// (a.walling 2009-07-14 12:30) - PLID 33644 - Add this message to the logging for PLID 34803
			CClient::PLID34803_AddTableChecker(table);

			try 
			{
				// Only process table checker messages if a user is logged in
				if (NULL != GetCurrentUserHandle())
				{
					ProcessTableCheckerMessage(wParam, lParam);
				}
						
				// (c.haag 2004-09-03 10:54) - Tell NxServer we are done processing
				// our table checker message.
				// (c.haag 2008-09-11 09:09) - PLID 31333 - This is now handled in packet reception
				// messages

			} NxCatchAll("Error in CMainFrm::WindowProc:WM_TABLE_CHANGED");

			// (c.haag 2008-09-11 09:58) - PLID 31333 - Confirm that we're done processing the table
			// checker to NxServer 
			if (bAckToNxServer) {
				try {
					TableCheckerStats::Entry statEntry((NetUtils::ERefreshTable)(0xffff0000 | table));
					CClient::Send(PACKET_TYPE_TC_PROCESSED, NULL, 0);
				}
				NxCatchAll("Error in CMainFrm::WindowProc:WM_TABLE_CHANGED:AckToNxServer");
			}
			break;
		} // case WM_TABLE_CHANGED:
		
		case WM_ACTIVATE:
		{
			// (b.cardillo 2005-03-03 17:26) - PLID 15827 - If we're being activated and there are 
			// notifications waiting, the refresh the notification dialog.  This is to ensure that 
			// if they happened while we weren't active and the user didn't want to be bothered, 
			// then we tell the user now.
			// (j.jones 2011-03-15 15:32) - PLID 42738 - renamed HasNotifications() to HasAnyNotifications()
			if (LOWORD(wParam) != WA_INACTIVE && HIWORD(wParam) == 0 && m_hWnd && 
				m_pNotificationDlg && m_pNotificationDlg->GetSafeHwnd() && m_pNotificationDlg->HasAnyNotifications()) 
			{
				m_pNotificationDlg->Refresh();
			}
			// And then proceed with the default behavior.
			break;
		}
		break;

		case NXM_PIC_CONTAINER_CLOSED:
		{
			// (b.cardillo 2006-06-14 09:13) - PLID 14292 - We've been told a PIC that WE spawned 
			// has been closed by the user.  So it's our job to destroy the window, free the 
			// memory we allocated for it, and remove it from our list of PICs.
			CPicContainerDlg *ppcd = (CPicContainerDlg *)lParam;
			if (ppcd) {
				ASSERT_VALID((CObject *)lParam);
				// (a.walling 2008-09-10 13:23) - PLID 31334 - Ensure the last active PIC is not pointing here
				// (a.walling 2009-06-22 10:48) - PLID 34635 - Dead/useless code
				/*
				if (m_pLastActivePIC == ppcd) {
					SetLastActivePIC(NULL);
				}
				*/
				try {
					// First thing, take them out of our list of PIC containers
					POSITION posPicEntry = m_lstpPicContainerDlgs.Find(ppcd);
					ASSERT(posPicEntry != NULL);
					if (posPicEntry) {
						m_lstpPicContainerDlgs.RemoveAt(posPicEntry);
					} else {
						// If they weren't in our list, then something is screwy and we should 
						// fail instead of proceeding with the destruction.
						ThrowNxException("Could not find the specified pic container 0x%08x in the shared CMainFrame list!", ppcd);
					}
				} NxCatchAll("CMainFrame::WindowProc:NXM_PIC_CONTAINER_CLOSED");
			}

			// (j.jones 2006-12-19 10:19) - PLID 23912 - added a missing break statement!
			break;
		}

		// (c.haag 2010-07-15 17:23) - PLID 34338 - Handle closing lab entry dialogs
		case NXM_LAB_ENTRY_DLG_CLOSED:
		{			
			CLabEntryDlg *pDlg = (CLabEntryDlg *)lParam;
			if (pDlg) {
				ASSERT_VALID((CObject*)pDlg);
				try {
					// Dismiss the lab follow up and todo dialogs if they're open if the user wants to
					// print preview the lab
					if (pDlg->HasOpenedReport()) {
						if (NULL != m_pLabFollowupDlg && IsWindow(m_pLabFollowupDlg->GetSafeHwnd())) {
							m_pLabFollowupDlg->PostMessage(WM_COMMAND, IDOK);
						}
						if (IsWindow(m_dlgToDoAlarm.GetSafeHwnd())) {
							m_dlgToDoAlarm.ShowWindow(SW_MINIMIZE);
						}
					}

					// Remove the lab entry dialog from our list
					POSITION pos = m_lstpLabEntryDlgs.Find(pDlg);
					ASSERT(pos != NULL);
					if (pos) {
						m_lstpLabEntryDlgs.RemoveAt(pos);
					} else {
						// If they weren't in our list, then something is screwy and we should 
						// fail instead of proceeding with the destruction.
						ThrowNxException("Could not find the specified lab entry dialog 0x%08x in the shared CMainFrame list!", pDlg);
					}
					// Now destroy the window and free the object
					if (IsWindow(pDlg->GetSafeHwnd())) {
						pDlg->DestroyWindow();
					}
					delete pDlg;
				} 
				NxCatchAll("CMainFrame::WindowProc:NXM_LAB_ENTRY_DLG_CLOSED");
			}
			break;
		}

		// (c.haag 2010-11-15 13:17) - PLID 41124 - Handle closing VisionWeb order dialogs
		case NXM_VISIONWEB_ORDER_DLG_CLOSED:
		{
			// wParam = The dialog "return value"
			// lParam = The dialog
			CVisionWebOrderDlg *pDlg = (CVisionWebOrderDlg *)lParam;
			if (pDlg) {
				ASSERT_VALID((CObject*)pDlg);
				try {
					// Remove the order from our list
					POSITION pos = m_lstpVisionWebOrderDlgs.Find(pDlg);
					ASSERT(pos != NULL);
					if (pos) {
						m_lstpVisionWebOrderDlgs.RemoveAt(pos);
					} else {
						// If they weren't in our list, then something is screwy and we should 
						// fail instead of proceeding with the destruction.
						ThrowNxException("Could not find the specified VisionWeb order 0x%08x in the shared CMainFrame list!", pDlg);
					}
					// Now destroy the window and free the object
					if (IsWindow(pDlg->GetSafeHwnd())) {
						pDlg->DestroyWindow();
					}
					//TES 12/10/2010 - PLID 41109 - Unregister this dialog for barcode scans
					UnregisterForBarcodeScan(pDlg);
					delete pDlg;
				} 
				NxCatchAll("CMainFrame::WindowProc:NXM_VISIONWEB_ORDER_DLG_CLOSED");
			}
			break;
		}

		// (j.dinatale 2012-03-21 16:11) - PLID 49079 - handle closing contact lens order forms
		case NXM_CONTACT_ORDER_DLG_CLOSED:
		{
			// wParam = The dialog "return value"
			// lParam = The dialog
			CContactLensOrderForm *pDlg = (CContactLensOrderForm *)lParam;
			if (pDlg) {
				ASSERT_VALID((CObject*)pDlg);
				try {
					// Remove the order from our list
					POSITION pos = m_lstpContactLensOrderForms.Find(pDlg);
					ASSERT(pos != NULL);
					if (pos) {
						m_lstpContactLensOrderForms.RemoveAt(pos);
					} else {
						// If they weren't in our list, then something is screwy and we should 
						// fail instead of proceeding with the destruction.
						ThrowNxException("Could not find the specified Contact Lens Order 0x%08x in the shared CMainFrame list!", pDlg);
					}
					// Now destroy the window and free the object
					if (IsWindow(pDlg->GetSafeHwnd())) {
						pDlg->DestroyWindow();
					}

					delete pDlg;
				} 
				NxCatchAll("CMainFrame::WindowProc:NXM_CONTACT_ORDER_DLG_CLOSED");
			}
			break;
		}

		case NXM_ROOM_MANAGER_CLOSED:
		{
			if (m_pRoomManagerDlg) {
				try {
					if(m_pRoomManagerDlg->m_hWnd) {
						m_pRoomManagerDlg->DestroyWindow();
					}
					delete m_pRoomManagerDlg;
					m_pRoomManagerDlg = NULL;
				} NxCatchAll("CMainFrame::WindowProc:NXM_ROOM_MANAGER_CLOSED");
			}
		}
		break;

		case WM_TIMECHANGE: {
			FireTimeChanged();
			break;
		}

		// (j.jones 2006-12-19 10:01) - PLID 22794 - open room manager
		case NXM_OPEN_ROOM_MANAGER: {
			ShowRoomManager((long)wParam, (BOOL)lParam);
			break;
		}

			// (a.walling 2008-07-09 15:23) - PLID 30605 - Handle this event
		case NXM_TB_BTN_MOUSE_MESSAGE: {
			HandleToolbarButtonMouseMessage(wParam, lParam);
			break;
		}

		// (j.jones 2008-07-17 09:01) - PLID 30730 - added message for the problem list closing
		case NXM_EMR_PROBLEM_LIST_CLOSED:
		{
			if (m_pEMRProblemListDlg) {
				try {
					
					DestroyEMRProblemList();
					
				} NxCatchAll("CMainFrame::WindowProc:NXM_EMR_PROBLEM_LIST_CLOSED");
			}
			break;
		}


		// (j.jones 2008-07-17 09:01) - PLID 30730 - added message for the problem list opening
		case NXM_OPEN_EMR_PROBLEM_LIST: {
			ShowEMRProblemList((long)wParam);
			break;
		}

		// (j.jones 2008-07-18 14:31) - PLID 30773 - added a message to forcibly close the problem list
		case NXM_EMR_DESTROY_PROBLEM_LIST: {
			DestroyEMRProblemList();
			break;
		}

		// (j.jones 2008-10-21 17:58) - PLID 14411 - added a message to safely close the EMR Analysis screen
		case NXM_EMR_ANALYSIS_CLOSED: {
			if (m_pEMRAnalysisDlg) {
				try {
					
					DestroyEMRAnalysisDlg();
					
				} NxCatchAll("CMainFrame::WindowProc:NXM_EMR_ANALYSIS_CLOSED");
			}
			break;
		}

		// (j.jones 2012-05-25 10:55) - PLID 44367 - added E-Remittance as modeless
		case NXM_EOB_CLOSED: {
			if (m_pEOBDlg) {
				try {
					
					DestroyEOBDlg();

					//update all views to reflect changes (which are possible even if they cancelled)
					UpdateAllViews();
					
				} NxCatchAll("CMainFrame::WindowProc:NXM_EOB_CLOSED");
			}
			break;
		}

		// (j.jones 2010-06-01 11:32) - PLID 37976 - respond to device plugin folder changes
		case NXM_DEVICE_PLUGIN_FOLDER_CHANGED: {
			// (a.walling 2012-12-04 16:12) - PLID 54037 - Trigger device import timers
			//DeviceImport::Logger.Log("CMainFrame::WndProc : NXM_DEVICE_PLUGIN_FOLDER_CHANGED");
			// (a.walling 2014-01-22 15:55) - PLID 60271 - a WPARAM and LPARAM both -1 means we are ready to import; otherwise just a folder changed notification as usual
			if (wParam == (WPARAM)-1 && lParam == (LPARAM)-1) {
				DeviceImport::GetMonitor().TriggerReadyToImport();
			} else {
				DeviceImport::GetMonitor().TriggerFolderChanged();
			} 
			break;
		}
		// (r.farnworth 2014-10-01 09:13) - PLID 63378 - The preference "Notify me when the device import screen has files already in the list when I log in"  does not work
		case NXM_DEVICE_PLUGIN_FOLDER_BEGIN_WATCH: {
			if (wParam == (WPARAM)-1 && lParam == (LPARAM)-1) {
				DeviceImport::GetMonitor().TriggerOnLogin();
			}
			else {
				DeviceImport::GetMonitor().TriggerFolderChanged();
			}
			break;
		}

	   // (s.dhole 07/23/2012) PLID 48693 
		case NXM_ELIGIBILITYREQUEST_CLOSED:{
			BOOL bCEligibilityRequestDetailDlg = FALSE;
			if(m_pEligibilityRequestDlg != NULL){
				try {
					// if EligibilityRequestDlg modless then show EligibilityRequestDetailDlg
					bCEligibilityRequestDetailDlg = TRUE ;					
					

					if(IsWindow(m_pEligibilityRequestDlg->GetSafeHwnd())) {
						m_pEligibilityRequestDlg->DestroyWindow();
					}
					delete m_pEligibilityRequestDlg;
					m_pEligibilityRequestDlg = NULL;
				} NxCatchAll("CMainFrame::WindowProc:NXM_ELIGIBILITYREQUEST_CLOSED");
			}
			// (s.dhole 09/11/2012) PLID 52414 
			// if EligibilityRequestDlg return ok along with id then load EligibilityRequestDetailDlg
			if (IDOK == wParam){
				long nID =(long)lParam;
				if (nID >0)
				{
					// (j.jones 2016-05-19 10:22) - NX-100685 - added a universal function for getting the default E-Billing / E-Eligibility format
					long nFormatID = GetDefaultEbillingANSIFormatID();

					CEEligibility dlg(this);
					dlg.m_FormatID = nFormatID;
					
					dlg.m_bUseRealTimeElig = (GetRemotePropertyInt("GEDIEligibilityRealTime_Enabled", 0, 0, "<None>", true) == 1);
					//add this request
					dlg.m_aryRequestIDsToExport.Add(nID );
					dlg.m_bCEligibilityRequestDetailDlg = bCEligibilityRequestDetailDlg ;
					dlg.DoModal();
				}
			}
			// (s.dhole 09/11/2012) PLID 48693 
			// check if finncial module is active than update view and refresh list
			CNxView* pView = GetOpenView(FINANCIAL_MODULE_NAME);
			// (b.cardillo 2013-06-13 21:00) - PLID 56862 - Make sure the financial view not only is there but also is the active view
			if (pView != NULL && pView == GetActiveView()) {
				pView->UpdateView();
			}
		}
		break;

		// (j.jones 2012-10-26 09:35) - PLID 53322 - called after an EMR closes and asks to prompt
		// to update PatientsT.HasNoAllergies
		case NXM_PROMPT_PATIENT_HAS_NO_ALLERGIES:
			PromptPatientHasNoAllergies((long)wParam);
			break;

		// (j.jones 2012-10-26 16:31) - PLID 53324 - called after an EMR closes and asks to prompt
		// to update PatientsT.HasNoMeds
		case NXM_PROMPT_PATIENT_HAS_NO_CURRENT_MEDS:
			PromptPatientHasNoCurrentMedications((long)wParam);
			break;

		// (d.singleton 2014-07-11 11:03) - PLID 62862 - called after the lockbox import dialog is closed
		case NXM_LOCKBOX_PAYMENT_IMPORT_CLOSED:
		{
			try {
				if (m_pLockboxPaymentImportDlg) {
					DestroyLockboxPaymentImportDlg();
					//do i actually need to call this?
					UpdateAllViews();
				}
			}NxCatchAll("CMainFrame::WindowProc:NXM_LOCKBOX_PAYMENT_IMPORT_CLOSED");
			break;
		}

		// (r.goldschmidt 2014-10-15 14:11) - PLID 62644 - called after eligibility request detail is closed
		case NXM_ELIGIBILITYREQUEST_DETAIL_CLOSED:
			
			try {
				if (m_pEligibilityRequestDetailDlg){
					DestroyEligibilityRequestDetailDlg();
				}
			} NxCatchAll("CMainFrame::WindowProc:NXM_ELIGIBILITYREQUEST_DETAIL_CLOSED");
			
			break;
		
	}

	return CNxMDIFrameWnd::WindowProc(message, wParam, lParam);
}

void CMainFrame::OnUpdateBoolLogFile(CCmdUI* pCmdUI) 
{
	if (GetRemotePropertyInt ("UseLog", 0)) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void CMainFrame::OnLogFile()
{
	m_pMenu = GetMenu();

	if (m_pMenu) {
		UINT state = m_pMenu->GetMenuState(ID_BOOL_LOG_FILE, MF_BYCOMMAND);

		if (state == 0xFFFFFFFF) return;

		if (state == MF_CHECKED) {// is checked
			SetRemotePropertyInt ("UseLog", 0);
			ClosePracticeLogFile();
		} else if (state == MF_UNCHECKED) {// is not checked
			SetRemotePropertyInt ("UseLog", -1);
			OpenLogFile();
		}
	}
}

void CMainFrame::OnApplyCareCredit()
{
	// (z.manning, 01/03/2007) - PLID 24056 - Open CCWare with the most recently selected patient.
	try {

		NxCareCredit::OpenCCWare(GetActivePatientID());

	}NxCatchAll("CMainFrame::OnApplyCareCredit");
}

void CMainFrame::OnLinkToMirror()
{
	if (UserPermission(MirrorIntegration))
	{
		CMirrorLink *pMirror = GetMirrorLink();
		if (pMirror) {
			pMirror->DoFakeModal();
		}
	}
}

void CMainFrame::OnLinkToUnited()
{
	if (UserPermission(UnitedIntegration))
	{
		CUnitedLink *pUnited = GetUnitedLink();
		if (pUnited) {
			pUnited->OpenDlg();
		}
	}
}

void CMainFrame::OnLinkToQuickbooks()
{
	if(CheckCurrentUserPermissions(bioQuickbooksLink,sptView))
	{
		CQuickbooksLink dlg(this);
		dlg.DoModal();
	}

	CNxTabView *pActiveView = GetActiveView();
	if (pActiveView) {
		pActiveView->UpdateView();
	}
}

void CMainFrame::OnLinkToCodeLink()
{
	CCodeLinkDlg dlg(this);
	dlg.DoModal();

	CNxTabView *pActiveView = GetActiveView();
	if (pActiveView) {
		pActiveView->UpdateView();
	}
}

void CMainFrame::OnConfigStatement()
{
	CStatementSetup Setup(this);
	Setup.DoModal();	
}

void CMainFrame::OnConfigReportGroups()
{
	if (CheckCurrentUserPermissions(bioConfigureReportGroups, sptView)) {
		CReportGroupsDlg dlg(this);
		dlg.DoModal();	
	}
}


void CMainFrame::OnChangeLocation()
{
	CChangeLocationDlg dlg(this);
	dlg.DoModal();
}

void CMainFrame::OnEditReports() {
	
	if (CheckCurrentUserPermissions(bioReportDesigner, sptRead)) {
	
		CEditNoCatReportsDlg  dlg(this);
		dlg.DoModal();
	}
}

void CMainFrame::OnTrackSerializedItems()
{
	try {
		// (e.lally 2009-06-08) PLID 34497 - View Patients module
		if(CheckCurrentUserPermissions(bioPatientsModule, sptView)) {
			CInvSerialTrackerDlg dlg(this);
			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnGenerateBarcodes()
{
	if (CheckCurrentUserPermissions(bioInvItem, sptWrite)) {
		CGenProductBarcodesDlg dlg(this);
		dlg.DoModal();
		UpdateAllViews();
	}
}

void CMainFrame::OnPatientSerializedItems()
{
	try {
		// (e.lally 2009-06-08) PLID 34497 - Permission - View patients module
		if(CheckCurrentUserPermissions(bioPatientsModule, sptView)) {
			CInvSerialTrackerDlg dlg(this);
			dlg.m_nDefaultPatientID = GetActivePatientID();
			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnUpdatePrefixes() {

	if (CheckCurrentUserPermissions(bioPatient, sptWrite)) {


		CUpdatePrefixesDlg dlg(this);
		dlg.DoModal();
	}
}



void CMainFrame::OnNewContact() 
{
	// (d.lange 2011-06-24 17:55) - PLID 20751 - Added context menu to the add new contact button
	try {
		enum {
			miProvider = -10,
			miRefPhys = -11,
			miUser = -12,
			miSupplier = -13,
			miOther = -14,
		};

		long nIndex = 0;
		CMenu mnu;
		
		mnu.m_hMenu = CreatePopupMenu();
		mnu.InsertMenu(nIndex++, MF_BYPOSITION|MF_ENABLED, miProvider, "Create New &Provider");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION|MF_ENABLED, miRefPhys, "Create New &Referring Physician");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION|MF_ENABLED, miUser, "Create New &User");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION|MF_ENABLED, miSupplier, "Create New &Supplier");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION|MF_ENABLED, miOther, "Create New &Other Contact");

		long nResult = 0;
		CPoint pt;
		GetCursorPos(&pt);
		nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);

		switch(nResult)
		{
			case miProvider:
				AddContact(dctProvider);
				break;
			case miRefPhys:
				AddContact(dctRefPhys);
				break;
			case miUser:
				AddContact(dctEmployer);
				break;
			case miSupplier:
				AddContact(dctSupplier);
				break;
			case miOther:
				AddContact(dctOther);
				break;
		}
		
	}NxCatchAll(__FUNCTION__);
}

// this is used if we call want to create a contact outside of the contact module, we pass with it the default type for
// the new contact dlg
// if the contact is created successfully, the ID of the new contact is returned
// else -1 is returned if there was a problem creating the contact
long CMainFrame::AddContact(DefaultContactType dctType, BOOL bSaveEditEnable /*=TRUE*/)
{
	try {

		if (!UserPermission(NewContact)){
			return -1;
		}
		CNewContact dlg(this);
		dlg.m_bSaveEditEnable = bSaveEditEnable;
		long id = -1;
		switch (dlg.DoModal(&id, dctType))
		{	case 0://Cancel
			break;
			case 1://Saved and resume
			{	
				// refresh the contact toolbar
				COleVariant bookmark;
				if(m_contactToolBar.m_toolBarCombo->GetCurSel() != -1)
					bookmark = m_contactToolBar.m_toolBarCombo->GetValue(m_contactToolBar.m_toolBarCombo->GetCurSel(),0);
				m_contactToolBar.m_toolBarCombo->Requery();//.RefreshContents();
				m_contactToolBar.m_toolBarCombo->SetSelByColumn(0,bookmark);
				m_contactToolBar.m_ActiveContact = m_contactToolBar.m_toolBarCombo->GetValue(m_contactToolBar.m_toolBarCombo->GetCurSel(),0).lVal;
				UpdateAllViews();
				break;
			}
			case 2://Save and Edit Further
			{	
				CNxTabView *pView;
				m_contactToolBar.m_toolBarCombo->Requery();//RefreshContents();
				m_contactToolBar.m_toolBarCombo->SetSelByColumn(0,(COleVariant)id);
				m_contactToolBar.m_ActiveContact = VarLong(id);
				if (m_contactToolBar.m_toolBarCombo->GetCurSel() == -1)
				{	m_contactToolBar.m_all.SetCheck(true);
					//clear radio buttons on all other buttons in case we were previously filtering one of these
					m_contactToolBar.m_employee.SetCheck(false);
					m_contactToolBar.m_main.SetCheck(false);
					m_contactToolBar.m_other.SetCheck(false);
					m_contactToolBar.m_referring.SetCheck(false);
					m_contactToolBar.m_supplier.SetCheck(false);
					OnAllContactsSearchClicked();
					m_contactToolBar.m_toolBarCombo->SetSelByColumn(0,(COleVariant)id);
				}

				if(m_contactToolBar.m_toolBarCombo->GetCurSel() != -1) {
					m_contactToolBar.m_ActiveContact = m_contactToolBar.m_toolBarCombo->GetValue(m_contactToolBar.m_toolBarCombo->GetCurSel(),0).lVal;
				}
				if(FlipToModule(CONTACT_MODULE_NAME)) {
					pView = (CNxTabView *)GetOpenView(CONTACT_MODULE_NAME);
					if (pView) {
						if(pView->GetActiveTab() == 0)
							pView->UpdateView();
						else
							pView->SetActiveTab(0);
					}

					// (k.messina 2010-04-12 11:16) - PLID 37957 lock internal contact notes
					if(IsNexTechInternal()) {
						((CContactView*)GetOpenView(CONTACT_MODULE_NAME))->CheckViewNotesPermissions();
					}
				}
				break;
			}
		}
		return id;

	}NxCatchAll("Error adding contact.");

	return -1;
}



void CMainFrame::OnMainPhysSearchClicked()
{
	bool m_bNoPermission = false;
	//this will throw a message box if they don't have permission, but the users are already filtered out, 
	//so we need the rest of the code to be executed
	if(!CheckCurrentUserPermissions(bioContactsProviders, sptRead))
		m_bNoPermission = true;

	m_contactToolBar.SetComboWhereClause("Main = 1");
	RefreshContactsModule();

	if(m_contactToolBar.GetActiveContactStatus() == -1 || m_contactToolBar.m_toolBarCombo->GetRowCount() == 0)
	{
		if(!m_bNoPermission)
			MessageBox("There are currently no contacts of this type.  Resetting the filter to All Contacts");
		m_contactToolBar.m_all.SetCheck(true);
		m_contactToolBar.m_main.SetCheck(false);
		OnAllContactsSearchClicked();
	}
}

void CMainFrame::OnReferringPhysSearchClicked()
{
	bool m_bNoPermission = false;
	//this will throw a message box if they don't have permission, but the users are already filtered out, 
	//so we need the rest of the code to be executed
	if(!CheckCurrentUserPermissions(bioContactsRefPhys, sptRead))
		m_bNoPermission = true;

	m_contactToolBar.SetComboWhereClause("Refer = 1");
	RefreshContactsModule();
	if(m_contactToolBar.GetActiveContactStatus() == -1 || m_contactToolBar.m_toolBarCombo->GetRowCount() == 0)
	{
		if(!m_bNoPermission)
			MessageBox("There are currently no contacts of this type.  Resetting the filter to All Contacts");
		m_contactToolBar.m_all.SetCheck(true);
		m_contactToolBar.m_referring.SetCheck(false);
		OnAllContactsSearchClicked();
	}
}

void CMainFrame::OnEmployeesSearchClicked()
{
	bool m_bNoPermission = false;
	//this will throw a message box if they don't have permission, but the users are already filtered out, 
	//so we need the rest of the code to be executed
	if(!CheckCurrentUserPermissions(bioContactsUsers, sptRead))
		m_bNoPermission = true;

	
	m_contactToolBar.SetComboWhereClause("Employee = 1");
	RefreshContactsModule();
	if(m_contactToolBar.GetActiveContactStatus() == -1 || m_contactToolBar.m_toolBarCombo->GetRowCount() == 0)
	{
		if(!m_bNoPermission)
			MessageBox("There are currently no contacts of this type.  Resetting the filter to All Contacts");
		m_contactToolBar.m_all.SetCheck(true);
		m_contactToolBar.m_employee.SetCheck(false);
		OnAllContactsSearchClicked();
	}
}

void CMainFrame::OnOtherContactsSearchClicked()
{
	m_contactToolBar.SetComboWhereClause("Main = 0 AND Refer = 0 AND Employee = 0 AND Supplier = 0");
	RefreshContactsModule();
	if(m_contactToolBar.GetActiveContactStatus() == -1 || m_contactToolBar.m_toolBarCombo->GetRowCount() == 0)
	{
		MessageBox("There are currently no contacts of this type.  Resetting the filter to All Contacts");
		m_contactToolBar.m_all.SetCheck(true);
		m_contactToolBar.m_other.SetCheck(false);
		OnAllContactsSearchClicked();
	}
}

void CMainFrame::OnAllContactsSearchClicked()
{
	m_contactToolBar.SetComboWhereClause("");
	RefreshContactsModule();

	//TODO: SET THE SQL TO THIS:
	//TODO: DO NOT FORGET TO FIX THE "Status" and "RecordID"
	/*m_contactToolBar.SetComboSQL("SELECT ID, [Last Name], [First Name], [Title], [City], [Company], [Refer], [Main], [Employee] FROM (SELECT PersonT.ID, PersonT.First AS [First Name], PersonT.Middle AS [Middle Name], PersonT.Last AS [Last Name], PersonT.Title, PersonT.Address1 AS [Address 1], PersonT.Address2 AS [Address 2], PersonT.City, PersonT.State AS StateProv, PersonT.Zip AS PostalCode, PersonT.HomePhone AS [Home Phone], PersonT.WorkPhone AS [Work Phone], PersonT.Extension, PersonT.Pager, PersonT.OtherPhone AS [Other Phone], PersonT.Email AS [Email Address], PersonT.Gender, PersonT.Prefix, ContactsT.Company, ContactsT.Spouse, PersonT.CellPhone, ContactsT.OtherFirst AS OtherCFName, ContactsT.OtherLast AS OtherCLName, ContactsT.OtherRelation, ContactsT.OtherHomePhone AS OtherCHPhone, ContactsT.OtherWorkPhone AS OtherCWPhone, PersonT.Note AS [Memo], '' AS Status, '' AS RecordID FROM PersonT INNER JOIN ContactsT ON PersonT.ID = ContactsT.PersonID) AS ContactGeneralTabQ INNER JOIN (SELECT IIf([ReferringPhyST].[ID],True,False) AS Refer, IIf([Provider ID],True,False) AS Main, IIf([EmployeeID],True,False) AS Employee, [Contact Info].ID AS ContactID FROM (([Contact Info] LEFT JOIN ReferringPhyST ON [Contact Info].ID = ReferringPhyST.ContactID) LEFT JOIN [Doctors and Providers] ON [Contact Info].ID = [Doctors and Providers].[Contact ID]) LEFT JOIN Employees ON [Contact Info].ID = Employees.[Contact ID]) AS "
								"ContactStatusQ ON ContactGeneralTabQ.ID = ContactStatusQ.ContactID");*/
}

void CMainFrame::OnSupplierSearchClicked()
{
	bool m_bNoPermission = false;
	//this will throw a message box if they don't have permission, but the users are already filtered out, 
	//so we need the rest of the code to be executed
	if(!CheckCurrentUserPermissions(bioContactsSuppliers, sptRead))
		m_bNoPermission = true;

	m_contactToolBar.SetComboWhereClause("Supplier = 1");
	RefreshContactsModule();

	if(m_contactToolBar.GetActiveContactStatus() == -1 || m_contactToolBar.m_toolBarCombo->GetRowCount() == 0)
	{
		if(!m_bNoPermission)
			MessageBox("There are currently no contacts of this type.  Resetting the filter to All Contacts");
		m_contactToolBar.m_all.SetCheck(true);
		m_contactToolBar.m_supplier.SetCheck(false);
		OnAllContactsSearchClicked();
	}
}

void CMainFrame::OnActiveContacts()
{
	BOOL check = (0 != m_contactToolBar.m_hideInactiveContacts.GetCheck());
	
	CString strNewWhere = "";

	// (c.haag 2009-08-05 12:49) - PLID 20750 - Remember this selection for this user
	SetRemotePropertyInt("HideInactiveContacts", check ? 1 : 0, 0, GetCurrentUserName());
	
	// hide inactive
	if(check){
		CString strCurWhere = m_contactToolBar.GetComboWhereClause();
		strNewWhere.Format("%s AND Archived = 0", strCurWhere);
	}
	// display all, but find out which radio button they had selected
	else{
		if(m_contactToolBar.m_main.GetCheck()){
			strNewWhere = "Main = 1";
		}
		else if(m_contactToolBar.m_referring.GetCheck()){
			strNewWhere = "Refer = 1";
		}
		else if(m_contactToolBar.m_employee.GetCheck()){
			strNewWhere = "Employee = 1";
		}
		else if(m_contactToolBar.m_other.GetCheck()){
			strNewWhere = "Main = 0 AND Refer = 0 AND Employee = 0 AND Supplier = 0";
		}
		else if(m_contactToolBar.m_supplier.GetCheck()){
			strNewWhere = "Supplier = 1";
		}
		else if(m_contactToolBar.m_all.GetCheck()){

		}
	}
	
	m_contactToolBar.SetComboWhereClause(strNewWhere);
	RefreshContactsModule();
}

void CMainFrame::RefreshContactsModule()
{
	CNxTabView *ptab = (CNxTabView *)GetOpenView(CONTACT_MODULE_NAME);
	if (ptab)
	{	ptab->UpdateView();
		//ptab->GetActiveSheet()->StoreDetails();
		//ptab->GetActiveSheet()->RecallDetails();
	}
}

void CMainFrame::OnContactDelete() 
{
	// (d.singleton 2013-03-21 15:27) - PLID 55817 - need to add a try catch as alot of code was not in a try catch
	try {

		CWaitCursor cuWait;
		if (!UserPermission(DeleteContact))
			return;

		long AuditItem;

		_RecordsetPtr rs;
		int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();

		long nContactID = GetActiveContactID();

		if(nStatus & 0x0)
		{
			// ContactsT
			try
			{
				CString strMessage;
				_RecordsetPtr prs = CreateParamRecordset("SELECT Count(ID) AS ItemCount FROM AdvanceDirectivesT WHERE CustodianID = {INT}", nContactID);
				long nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					strMessage.Format("You cannot delete this contact because it is the custodian of %li advance directive%s.",
						nCount, nCount > 1 ? "s" : "");
					MessageBox(strMessage);
					return;
				}
			}NxCatchAll("CMainFrame::OnContactDelete (contacts)");
		}
		if(nStatus & 0x1)
		{	//delete RefPhyST
			if (MessageBox("This is an unrecoverable operation! Are you absolutely sure you wish to delete this contact?", "Confirm Delete", MB_ICONEXCLAMATION|MB_YESNO) != IDYES )
				return;

			// (z.manning 2008-11-19 12:00) - PLID 31687 - Added handling for ProcInfoT.CoSurgeonID. Since you
			// can't delete providers that are used as a co-surgeon, I'm doing the same for referring physicians.
			CString strMessage;
			_RecordsetPtr prs = CreateParamRecordset("SELECT Count(CoSurgeonID) AS ItemCount FROM ProcInfoT WHERE CoSurgeonID = {INT}", GetActiveContactID());
			long nCount = AdoFldLong(prs, "ItemCount");
			if(nCount > 0) {
				strMessage += FormatString("  - %li procedure information records as the co-surgeon.\r\n", nCount);
			}
			prs->Close();

			// (z.manning 2009-05-06 17:54) - PLID 28529 - No deleting ref phys that have a referral order
			prs = CreateParamRecordset("SELECT COUNT(*) AS Count FROM ReferralOrdersT WHERE ReferToID = {INT}", GetActiveContactID());
			nCount = AdoFldLong(prs->GetFields(), "Count");
			if(nCount > 0) {
				strMessage += FormatString("  - %li referral orders", nCount);
			}
			prs->Close();


			// (j.gruber 2011-10-21 15:02) - PLID 45356 - affiliate physicians, intentionally including deleted
			prs = CreateParamRecordset("SELECT COUNT(*) AS Count FROM BillsT WHERE AffiliatePhysID = {INT}", GetActiveContactID());		
			nCount = AdoFldLong(prs->GetFields(), "Count", 0);
			if(nCount > 0) {
				strMessage += FormatString("  - %li bills as the affiliate physician", nCount);
			}
			prs->Close();

			// (j.jones 2014-04-22 16:26) - PLID 61836 - check charges ReferringProviderID, OrderingProviderID, SupervisingProviderID
			prs = CreateParamRecordset("SELECT Count(ID) AS Count FROM ChargesT "
				"WHERE ReferringProviderID = {INT} OR OrderingProviderID = {INT} OR SupervisingProviderID = {INT}",
				GetActiveContactID(), GetActiveContactID(), GetActiveContactID());
			nCount = AdoFldLong(prs->GetFields(), "Count", 0);
			if(nCount > 0) {
				strMessage += FormatString("  - %li charges as a referring, ordering, or supervising physician.", nCount);
			}
			prs->Close();

			// (j.jones 2014-05-02 11:36) - PLID 61839 - check HCFADocumentsT
			prs = CreateParamRecordset("SELECT Count(FirstDocumentID) AS Count FROM HCFADocumentsT WHERE Box17PersonID = {INT}", GetActiveContactID());
			nCount = AdoFldLong(prs->GetFields(), "Count", 0);
			if (nCount > 0) {
				strMessage += FormatString("  - %li saved HCFA forms as a referring, ordering, or supervising physician.", nCount);
			}
			prs->Close();

			// (j.gruber 2013-01-17 11:45) - PLID 54483 - check before deletion
			prs = CreateParamRecordset("SELECT COUNT(*) AS Count FROM AppointmentsT WHERE RefPhysID = {INT}", GetActiveContactID());		
			nCount = AdoFldLong(prs->GetFields(), "Count", 0);
			if(nCount > 0) {
				strMessage += FormatString("  - %li appointments as the referring physician", nCount);
			}
			prs->Close();

			if(!strMessage.IsEmpty()) {
				strMessage = "You cannot delete this referring physician because it is linked with the following data: \r\n" + strMessage;
				MessageBox(strMessage);
				return;
			}

			//(e.lally 2009-05-11) PLID 28553 - Added check for associated data with Order Set Templates.
			// (j.gruber 2011-09-27 16:09) - PLID 45357 - affiliatephysicians
			rs = CreateRecordset("SELECT * FROM (SELECT ToDoList.PersonID FROM ToDoList UNION SELECT PatientsT.PCP FROM PatientsT UNION SELECT Notes.PersonID FROM Notes UNION SELECT CustomFieldDataT.PersonID FROM CustomFieldDataT UNION (SELECT CustomFieldDataT.IntParam FROM CustomFieldDataT WHERE FieldID = 31) UNION SELECT GroupDetailsT.PersonID FROM GroupDetailsT UNION SELECT MailSent.PersonID FROM MailSent UNION (SELECT BillsT.RefPhyID FROM BillsT WHERE BillsT.RefPhyID Is Not Null) UNION (SELECT PatientsT.DefaultReferringPhyID FROM PatientsT WHERE PatientsT.DefaultReferringPhyID Is Not Null) UNION (SELECT PatientsT.AffiliatePhysID FROM PatientsT WHERE PatientsT.AffiliatePhysID Is Not Null) UNION (SELECT RefPhysProcLinkT.RefPhysID FROM RefPhysProcLinkT) UNION (SELECT ReferringPhyID FROM OrderSetTemplateReferralsT)) SubQ WHERE PersonID = %li", GetActiveContactID());

			//TES 3/22/2004: Don't say we succeeded if we didn't.
			bool bError = false;
			if(!rs->eof)
			{	//contact has data... clean it out first
				if(MessageBox("This contact currently has data associated with it.  Removing this contact will remove ALL related data, are you SURE you wish to do this?", "Confirm Delete", MB_ICONEXCLAMATION|MB_YESNO) != IDYES)
					return;
				rs->Close();
				BEGIN_TRANS("ContactDelete") {
					bError = true;
					DeleteRefPhys();
					DeleteCommonContactInfo();
					bError = false;
				} END_TRANS_CATCH_ALL("ContactDelete");

			}
			else
				rs->Close();
			//delete from ReferringPhyST
			//delete from PersonT
			try{
				
				//b.spivey - November 13th, 2013 - PLID 59022 - Delete this when deleting contacts. 
				ExecuteParamSql("SET NOCOUNT ON "
					"	"
					"DECLARE @ContactID INT "
					"SET @ContactID = {INT} "
					"	"
					"DELETE DirectAddressUserT FROM DirectAddressUserT DAUT "
					"INNER JOIN DirectAddressFromT DAFT ON DAUT.DirectAddressFromID = DAFT.ID "
					"WHERE DAFT.PersonID = @ContactID "
					"	"
					"DELETE FROM DirectAddressFromT WHERE PersonID = @ContactID "
					"	"
					"SET NOCOUNT OFF ", GetActiveContactID());

				//(e.lally 2011-05-05) PLID 43481 - remove NexWeb display setup
				ExecuteParamSql("DELETE FROM NexWebDisplayT WHERE ReferringPhysicianID = {INT} OR PriCarePhysID = {INT} ", GetActiveContactID(), GetActiveContactID());

				//TES 5/23/2011 - PLID 41353 - Referring physicians may have HL7 links
				ExecuteSql("DELETE FROM HL7IDLinkT WHERE PersonID = %li", GetActiveContactID());

				ExecuteSql("DELETE FROM ReferringPhyST WHERE ReferringPhyST.PersonID = %li", GetActiveContactID());
				ExecuteSql("DELETE FROM PersonT WHERE PersonT.ID = %li", GetActiveContactID());
				ExecuteSql("UPDATE PalmContactsInfoT SET Status = 4 WHERE PersonID = %li", GetActiveContactID());

				AuditItem = aeiRefPhysDeleted;

			}NxCatchAllCall("Error in CMainFrame::OnContactDelete", bError = true;);

			CClient::RefreshTable(NetUtils::RefPhys); // Network stuff

			if(!bError)
				MsgBox("Contact has been successfully deleted!");
		}

		else if(nStatus & 0x2)
		{	//delete ProvidersT

			if (MessageBox("This is an unrecoverable operation! Are you absolutely sure you wish to delete this contact?", "Confirm Delete", MB_ICONEXCLAMATION|MB_YESNO) != IDYES )
				return;

			//TES 3/22/2004: Don't say we succeeded if we didn't.
			bool bError = false;

			//DRT 10/7/03 - PLID 9712 - Doing this was a good idea to stop bad data.  However, it didn't end up working too well because there
			//		have been numerous problems with people deleting things they should be able to.  So what we need to do is this:  
			//	Split up the checking into multiple categories.  A)  Things that refuse to let you delete.  These are things like "tied to patients", 
			//		"tied to bills", etc.  B)  Things that you should be warned about, but will still allow you.  Things like Notes, followups, history, etc.
			//		C)  Silently deleted things.  These are ones that are there in the background, and if they pass A and B, it should just delete these as needed
			//		and go on its way without annoying you.  These are things like InsuranceAcceptedT, etc.
			//	Also, we MUST create a list!  If you're getting stopped because 4 patients have that provider, tell them.  That way they can
			//		lookup that list and fix it w/o ever calling us.  The key needs to be that the items can be changed by the user.  If the
			//		data is in a table that is not user-editable, or in a very hidden setting (something in the admin, hcfa setup for example), 
			//		they should be in the silent category.
			long nContactID = GetActiveContactID();

			//A)  Presence of these items will stop a deletion from happening altogether.  There is no way around this.
			try {
				CString strMsg;
				CString str;
				_RecordsetPtr prs;

				prs = CreateRecordset("SELECT Count(ProviderID) AS ItemCount FROM CaseHistoryT WHERE ProviderID = %li", GetActiveContactID());
				long nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li case histories.\r\n", nCount);
					strMsg += str;
				}

				// (j.jones 2010-11-08 17:38) - PLID 31392 - include a check for ClaimProviderID
				// (j.jones 2014-04-22 16:26) - PLID 61836 - check the ReferringProviderID, OrderingProviderID, SupervisingProviderID
				prs = CreateParamRecordset("SELECT Count(ID) AS ItemCount FROM (SELECT ID FROM ChargesT "
					"WHERE DoctorsProviders = {INT} OR ClaimProviderID = {INT} OR ReferringProviderID = {INT} OR OrderingProviderID = {INT} OR SupervisingProviderID = {INT} "
					"UNION SELECT ID FROM PaymentsT WHERE ProviderID = {INT} "
					"UNION SELECT ID FROM PaymentTipsT WHERE ProvID = {INT}) AS Q",
					nContactID, nContactID, nContactID, nContactID, nContactID,
					nContactID,
					nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li charges or payments.\r\n", nCount);
					strMsg += str;
				}

				// (j.jones 2014-05-02 11:36) - PLID 61839 - check HCFADocumentsT
				prs = CreateParamRecordset("SELECT Count(FirstDocumentID) AS Count FROM HCFADocumentsT WHERE Box17PersonID = {INT}", nContactID);
				nCount = AdoFldLong(prs->GetFields(), "Count", 0);
				if (nCount > 0) {
					str.Format("  - %li saved HCFA forms as a referring, ordering, or supervising physician.", nCount);
					strMsg += str;
				}
				prs->Close();

				// (j.jones 2013-07-11 16:31) - PLID 57148 - check BillInvoiceNumbersT
				prs = CreateParamRecordset("SELECT Count(BillID) AS ItemCount FROM BillInvoiceNumbersT WHERE ProviderID = {INT}", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li bill invoices.\r\n", nCount);
					strMsg += str;
				}

				prs = CreateRecordset("SELECT Count(MainPhysician) AS ItemCount FROM PatientsT WHERE PatientsT.MainPhysician = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li patients.\r\n", nCount);
					strMsg += str;
				}

				prs = CreateRecordset("SELECT Count(InsuranceReferralsT.ProviderID) AS ItemCount FROM InsuranceReferralsT WHERE ProviderID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li insurance referrals.\r\n", nCount);
					strMsg += str;
				}

				prs = CreateRecordset("SELECT Count(PatientMedications.ProviderID) AS ItemCount FROM PatientMedications WHERE ProviderID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li patient medications.\r\n", nCount);
					strMsg += str;
				}

				// (b.savon 2013-02-08 14:47) - PLID 54656 - rename to OldAgentID
				// (a.walling 2009-06-29 11:24) - PLID 34052 - check for prescriber agents
				prs = CreateRecordset("SELECT Count(PatientMedications.OldAgentID) AS ItemCount FROM PatientMedications WHERE OldAgentID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - prescriber agent for %li patient medications.\r\n", nCount);
					strMsg += str;
				}

				// (a.walling 2009-06-29 11:24) - PLID 34052 - check for supervisors
				prs = CreateRecordset("SELECT Count(PatientMedications.SupervisorID) AS ItemCount FROM PatientMedications WHERE SupervisorID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - supervisor for %li patient medications.\r\n", nCount);
					strMsg += str;
				}

				// (a.wilson 2013-02-07 17:40) - PLID 51711 - check for renewal requests
				prs = CreateParamRecordset("SELECT Count(*) As ItemCount FROM RenewalRequestsT WHERE PrescriberID = {INT} OR SupervisorID = {INT}", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if (nCount > 0) {
					str.Format("  - prescriber or supervisor for %li renewal requests.\r\n", nCount);
					strMsg += str;
				}

				// (m.cable 6/15/2004 13:23) - PLID 13005 - You can no longer delete providers assigned to EMRs.
				// (z.manning, 09/15/2006) - PLID 22388 - We don't want to lose this data even on deleted EMNs.
				prs = CreateRecordset("SELECT Count(EmrProvidersT.ProviderID) AS ItemCount FROM EmrProvidersT WHERE ProviderID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li patient EMN(s).\r\n", nCount);
					strMsg += str;
				}

				// (j.gruber 2007-01-08 10:57) - PLID 23399 - Don't delete secondary provs on EMR's either!
				prs = CreateRecordset("SELECT Count(EmrSecondaryProvidersT.ProviderID) AS ItemCount FROM EmrSecondaryProvidersT WHERE ProviderID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li patient EMN(s) as secondary provider.\r\n", nCount);
					strMsg += str;
				}

				// (j.gruber 2009-05-07 17:13) - PLID 33688 - Don't delete other provs on EMR's!
				prs = CreateRecordset("SELECT Count(EmrOtherProvidersT.ProviderID) AS ItemCount FROM EmrOtherProvidersT WHERE ProviderID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li patient EMN(s) as an other provider.\r\n", nCount);
					strMsg += str;
				}


				// (a.walling 2007-01-17 16:27) - PLID 21003 - CoSurgeon on PIC records
				prs = CreateRecordset("SELECT Count(CoSurgeonID) AS ItemCount FROM ProcInfoT WHERE CoSurgeonID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li procedure information records as the co-surgeon.\r\n", nCount);
					strMsg += str;
				}

				// (a.walling 2007-01-17 16:27) - PLID 24290 - We should check for surgeon records in ProcInfoT before deleting or changing
				/*
				prs = CreateRecordset("SELECT Count(SurgeonID) AS ItemCount FROM ProcInfoT WHERE SurgeonID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
				str.Format("  - %li procedure information records as the surgeon.\r\n", nCount);
				strMsg += str;
				}
				*/

				//DRT 10/10/2005 - PLID 17838 - Can't delete providers linked to batch payments
				prs = CreateRecordset("SELECT COUNT(ID) AS ItemCount FROM BatchPaymentsT WHERE ProviderID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li batch payment(s).\r\n", nCount);
					strMsg += str;
				}

				//DRT 6/30/2006 - PLID 21290 - Can't delete providers used on a lab
				prs = CreateRecordset("SELECT COUNT(ID) AS ItemCount FROM LabMultiProviderT WHERE ProviderID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li lab(s).\r\n", nCount);
					strMsg += str;
				}

				// (j.dinatale 2012-05-07 16:07) - PLID 50219 - cant delete providers that are on glasses orders
				prs = CreateParamRecordset("SELECT Count(ID) AS ItemCount FROM GlassesOrderT WHERE ProviderID = {INT} OR OpticianID = {INT}", nContactID, nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li optical order(s).\r\n", nCount);
					strMsg += str;
				}

				// (j.jones 2007-05-23 15:02) - PLID 8993 - can't delete providers used on E-Eligibility Requests			
				prs = CreateRecordset("SELECT Count(ProviderID) AS ItemCount FROM EligibilityRequestsT WHERE ProviderID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li E-Eligibility Request(s).\r\n", nCount);
					strMsg += str;
				}

				// (j.jones 2006-12-01 11:02) - PLID 22110 - supported ClaimProviderID
				prs = CreateRecordset("SELECT Count(ClaimProviderID) AS ItemCount FROM ProvidersT WHERE ClaimProviderID = %li AND PersonID <> %li", nContactID, nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - Is the claim provider for %li other provider(s).\r\n", nCount);
					strMsg += str;
				}

				// (j.jones 2008-04-02 14:42) - PLID 28895 - supported the HCFAClaimProvidersT references
				prs = CreateParamRecordset("SELECT Count(ID) AS ItemCount FROM HCFAClaimProvidersT WHERE "
					"(ANSI_2010AA_ProviderID = {INT} OR ANSI_2310B_ProviderID = {INT} "
					"OR Box33_ProviderID = {INT} OR Box24J_ProviderID = {INT}) "
					"AND ProviderID <> {INT} ", nContactID, nContactID, nContactID, nContactID, nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - Is the HCFA claim provider for %li other provider(s).\r\n", nCount);
					strMsg += str;
				}

				// (j.jones 2008-12-11 10:44) - PLID 32407 - supported BillsT.SupervisingProviderID
				prs = CreateParamRecordset("SELECT Count(ID) AS ItemCount FROM BillsT WHERE SupervisingProviderID = {INT}", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - Is the Supervising Provider for %li bill(s).\r\n", nCount);
					strMsg += str;
				}

				// (j.jones 2008-03-05 11:57) - PLID 29201 - supported providers on allocations
				prs = CreateRecordset("SELECT COUNT(ProviderID) AS ItemCount FROM PatientInvAllocationsT WHERE ProviderID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li inventory allocation(s).\r\n", nCount);
					strMsg += str;
				}

				//(e.lally 2011-06-17) PLID 43177 - Prevent the delete if this is an actively licensed EMR provider.
				//	I am not aware of any potential problems by leaving this provider in the licensing info even though 
				//	they are hidden from view and the information no longer applies
				// (a.wilson 2012-08-08 16:58) - PLID 52043 - applied the use of a new function to clean up unneccessary code.
				if(g_pLicense->IsProviderLicensed(nContactID)){
					strMsg += "  - Licensed EMR Provider.\r\n";
				}

				// (b.savon 2013-03-01 14:26) - PLID 54578 - if this user has nexerx role
				if(ReturnsRecordsParam("SELECT TOP 1 PersonID FROM ProvidersT WHERE PersonID = {INT} AND NexERxProviderTypeID IS NOT NULL AND NexERxProviderTypeID > -1", nContactID)){
					strMsg += "  - NexERx Prescriber role.\r\n"; 
				}

				// (b.savon 2013-06-13 10:27) - PLID 56867 - If this prescriber is registered with nexerx
				if(ReturnsRecordsParam("SELECT TOP 1 ProviderID FROM NexERxPrescriberRegistrationT WHERE ProviderID = {INT}", nContactID)){
					strMsg += "  - NexERx registered prescriber.\r\n";
				}

				// (j.jones 2016-02-17 17:18) - PLID 68348 - check recalls
				prs = CreateParamRecordset("SELECT COUNT(ProviderID) AS ItemCount FROM RecallT WHERE ProviderID = {INT}", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if (nCount > 0) {
					str.Format("  - %li recall(s).\r\n", nCount);
					strMsg += str;
				}

				if(!strMsg.IsEmpty()) {
					strMsg = "You cannot delete this provider because it is linked with the following data: \r\n" + strMsg;
					MsgBox(strMsg);
					return;
				}
			}NxCatchAll("Error validating provider deletion (A)");

			//B)  Presence of these items will prompt you before deleting, but you will have the opportunity to wipe them out.
			try {
				CString strMsg;
				CString str;
				_RecordsetPtr prs;

				prs = CreateRecordset("SELECT Count(ToDoList.PersonID) AS ItemCount FROM ToDoList WHERE PersonID = %li", nContactID);
				long nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li contact followups.\r\n", nCount);
					strMsg += str;
				}

				prs = CreateRecordset("SELECT Count(Notes.PersonID) As ItemCount FROM Notes WHERE PersonID = %li",nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li contact notes.\r\n", nCount);
					strMsg += str;
				}

				prs = CreateRecordset("SELECT Count(MailSent.PersonID) AS ItemCount FROM MailSent WHERE PersonID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - %li history documents.\r\n", nCount);
					strMsg += str;
				}

				prs = CreateRecordset("SELECT Count(GroupDetailsT.PersonID) AS ItemCount FROM GroupDetailsT WHERE PersonID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - Is a member of %li letter writing groups.\r\n", nCount);
					strMsg += str;
				}

				prs = CreateRecordset("SELECT Count(DefaultProviderID) AS ItemCount FROM LocationsT WHERE DefaultProviderID = %li", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - Is the default provider for %li location(s).\r\n", nCount);
					strMsg += str;
				}

				// (j.jones 2009-08-24 12:13) - PLID 35124 - personnel are only in Preference Cards now
				prs = CreateParamRecordset("SELECT Count(PreferenceCardDetailsT.PersonID) AS ItemCount "
					"FROM PreferenceCardDetailsT WHERE PersonID = {INT}", nContactID);
				nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					str.Format("  - Is in %li preference cards.\r\n", nCount);
					strMsg += str;
				}

				// (j.jones 2008-06-23 10:21) - PLID 30434 - added EligibilitySetupT
				prs = CreateParamRecordset("SELECT * FROM (SELECT AdvHCFAPinT.ProviderID FROM AdvHCFAPinT "
					"UNION SELECT EbillingSetupT.ProviderID FROM EbillingSetupT "
					"UNION SELECT EligibilitySetupT.ProviderID FROM EligibilitySetupT "
					"UNION SELECT InsuranceNetworkID.ProviderID FROM InsuranceNetworkID "
					"UNION SELECT InsuranceBox51.ProviderID FROM InsuranceBox51 "
					"UNION SELECT InsuranceBox24J.ProviderID FROM InsuranceBox24J "
					"UNION SELECT InsuranceGroups.ProviderID FROM InsuranceGroups) SubQ WHERE ProviderID = {INT}", nContactID);
				if(!prs->eof) {
					str.Format("  - HCFA / UB92 / EBilling setup information.");
					strMsg += str;
				}

				//(r.wilson 7/29/2013) PLID 48684
				// (b.savon 2013-08-27 15:23) - PLID 58329 - Fix the syntax
				prs = CreateParamRecordset("SELECT Count(*) AS LinkedCount FROM ProvidersT WHERE EMRDefaultProviderID = {INT}", nContactID);
				long nLinkedCount = 0;
				if(!prs->eof){
					nLinkedCount = AdoFldLong(prs, "LinkedCount");
				}
				if(nLinkedCount > 0){
					CString strLinkedProviders;
					strLinkedProviders.Format("  - %li provider%s have this provider set as their default primary for EMR.\r\n", nLinkedCount, nLinkedCount < 2 ? "":"s");
				}

				if(!strMsg.IsEmpty()) {
					strMsg = "  - This provider is linked with the following data.  If you continue, this data will be erased.\r\n" + strMsg + "\r\nAre you sure you wish to continue?";
					if(MsgBox(MB_YESNO, strMsg) == IDNO)
						return;
				}
			}NxCatchAll("Error validating provider deletion (B)");

			//C)  These are items that we no longer check for.  They are settings or other items that cannot be changed easily by the user, and
			//		not important enough to prompt about.
			{
				// - To the user, this is just the same as deleting the address information, general demographics.
				//SELECT CustomFieldDataT.PersonID FROM CustomFieldDataT 
				// - Setup information
				//SELECT QBooksAcctsToProvidersT.ProviderID FROM QBooksAcctsToProvidersT 
				// - Setup information
				//SELECT SurgeryProvidersT.ProviderID FROM SurgeryProvidersT 
				// - Setup information
				//SELECT ProviderID FROM MultiFeeProvidersT 
				// - These are both setup for the HCFA that is not needed to save
				//SELECT ProviderID FROM HCFAT
				//SELECT HCFADocumentsT.ProviderIndex FROM HCFADocumentsT
			}

			/*	DRT 10/7/03 - This is the query of all things that were previously stopping all deletions from taking place.
			rs = CreateRecordset("SELECT * FROM (SELECT ToDoList.PersonID FROM ToDoList UNION SELECT Notes.PersonID FROM Notes UNION SELECT CustomFieldDataT.PersonID FROM CustomFieldDataT UNION SELECT GroupDetailsT.PersonID FROM GroupDetailsT UNION SELECT MailSent.PersonID FROM MailSent "
			"UNION SELECT QBooksAcctsToProvidersT.ProviderID FROM QBooksAcctsToProvidersT UNION SELECT SurgeryProvidersT.ProviderID FROM SurgeryProvidersT UNION SELECT SurgeryDetailsT.PersonID FROM SurgeryDetailsT UNION SELECT InsuranceReferralsT.ProviderID FROM InsuranceReferralsT UNION SELECT AdvHCFAPinT.ProviderID FROM AdvHCFAPinT "
			"UNION SELECT EbillingSetupT.ProviderID FROM EbillingSetupT UNION SELECT PatientMedications.ProviderID FROM PatientMedications UNION SELECT ProviderID FROM MultiFeeProvidersT UNION SELECT InsuranceNetworkID.ProviderID FROM InsuranceNetworkID UNION SELECT InsuranceBox51.ProviderID FROM InsuranceBox51 UNION SELECT InsuranceBox24J.ProviderID FROM InsuranceBox24J UNION SELECT InsuranceGroups.ProviderID FROM InsuranceGroups UNION (SELECT DefaultProviderID FROM LocationsT WHERE DefaultProviderID Is Not Null) "
			"UNION (SELECT ProviderID FROM HCFAT WHERE HCFAT.ProviderID Is Not Null) UNION (SELECT HCFADocumentsT.ProviderIndex FROM HCFADocumentsT WHERE HCFADocumentsT.ProviderIndex Is Not Null) UNION (SELECT ProviderID FROM PaymentsT WHERE PaymentsT.ProviderID IS Not Null) "
			"UNION (SELECT DoctorsProviders FROM ChargesT WHERE ChargesT.DoctorsProviders Is Not Null) UNION (SELECT MainPhysician FROM PatientsT WHERE PatientsT.MainPhysician Is Not Null)) SubQ WHERE PersonID = %li", GetActiveContactID());
			if(!rs->eof)
			{	//contact has data... clean it out first

			//DRT 3/14/03 - Due to a lot of problems that have come up with people deleting Providers, we're not going to allow them to do that
			//		anymore.  When we get the ability to inactivate providers, prompt them to do that instead of delete.
			MessageBox("This provider has important data associated with it.  You cannot delete a provider that has information linked to it.");
			return;

			//	DRT 3/13/03 - see above note
			//	rs->Close();
			//	BEGIN_TRANS("ContactDelete") {
			//		DeleteProvider();
			//		DeleteCommonContactInfo();
			//	} END_TRANS_CATCH_ALL("ContactDelete");			
			}
			else
			rs->Close();
			*/
			//delete from ProvidersT
			//delete from PersonT
			try {
				BEGIN_TRANS("ProviderDelete") {
					bError = true;
					//Section B) Deletes
					{
						CString strDelete, str;
						//(e.lally 2011-10-31) PLID 41195 - Disconnect the mailSent record from any FaxLogT entries
						str.Format("UPDATE FaxLogT Set MailID = NULL WHERE MailID IN (SELECT MailID FROM MailSent WHERE PersonID = %li); ", nContactID);
						strDelete += str;

						//b.spivey - November 13th, 2013 - PLID 59022 - Delete this when deleting contacts. 
						ExecuteParamSql("SET NOCOUNT ON "
							"	"
							"DECLARE @ContactID INT "
							"SET @ContactID = {INT} "
							"	"
							"DELETE DirectAddressUserT FROM DirectAddressUserT DAUT "
							"INNER JOIN DirectAddressFromT DAFT ON DAUT.DirectAddressFromID = DAFT.ID "
							"WHERE DAFT.PersonID = @ContactID "
							"	"
							"DELETE FROM DirectAddressFromT WHERE PersonID = @ContactID "
							"	"
							"SET NOCOUNT OFF ", GetActiveContactID());

						// (c.haag 2008-06-11 08:38) - PLID 30328 - We also have to delete from TodoAssignToT
						// (c.haag 2008-07-10 15:37) - PLID 30674 - Also delete from the EMR todo table
						strDelete += FormatString("DELETE FROM EMRTodosT WHERE TaskID IN (SELECT TaskID FROM TodoList WHERE PersonID = %d) ", nContactID);
						strDelete += FormatString("DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT TaskID FROM TodoList WHERE PersonID = %d) ", nContactID);
						str.Format("DELETE FROM ToDoList WHERE PersonID = %li; ", nContactID);
						strDelete += str;
						// (a.walling 2010-08-02 12:11) - PLID 39867 - Moved Notes metadata to NoteInfoT
						str.Format("DELETE FROM NoteDataT WHERE PersonID = %li; ", nContactID);
						strDelete += str;
						// (j.jones 2008-09-04 17:45) - PLID 30288 - supported MailSentNotesT
						str.Format("DELETE FROM MailSentNotesT WHERE MailID IN (SELECT MailID FROM MailSent WHERE PersonID = %li)", nContactID);
						strDelete += str;

						// (c.haag 2009-10-12 12:44) - PLID 35722 - More MailSent-related tables
						str.Format("DELETE FROM MailSentProcedureT WHERE MailSentID IN (SELECT MailID FROM MailSent WHERE PersonID = %li)", nContactID);
						strDelete += str;
						str.Format("DELETE FROM MailSentPictureTagT WHERE MailSentID IN (SELECT MailID FROM MailSent WHERE PersonID = %li)", nContactID);
						strDelete += str;
						str.Format("DELETE FROM MailSentServiceCptT WHERE MailSentID IN (SELECT MailID FROM MailSent WHERE PersonID = %li)", nContactID);
						strDelete += str;

						// (r.farnworth 2013-11-26 14:11) - PLID 59520 - Need to check for NexWebCCDAAccess before deleting MailSent items.
						str.Format("DELETE E FROM NexWebCcdaAccessHistoryT E INNER JOIN MailSent W ON E.MailSentMailID = W.MailID WHERE W.PersonID = %li; ", nContactID);
						strDelete += str;

						str.Format("DELETE FROM MailSent WHERE PersonID = %li; ", nContactID);
						strDelete += str;
						str.Format("DELETE FROM GroupDetailsT WHERE PersonID = %li; ", nContactID);
						strDelete += str;
						str.Format("UPDATE LocationsT SET DefaultProviderID = NULL WHERE DefaultProviderID = %li; ", nContactID);
						strDelete += str;
						// (j.jones 2009-08-24 12:13) - PLID 35124 - personnel are only in Preference Cards now
						str.Format("DELETE FROM PreferenceCardDetailsT WHERE PersonID = %li; ", nContactID);
						strDelete += str;
						str.Format("DELETE FROM AdvHCFAPinT WHERE ProviderID = %li; ", nContactID);
						strDelete += str;
						str.Format("DELETE FROM EBillingSetupT WHERE ProviderID = %li; ", nContactID);
						strDelete += str;
						// (j.jones 2008-06-23 10:21) - PLID 30434 - added EligibilitySetupT
						str.Format("DELETE FROM EligibilitySetupT WHERE ProviderID = %li; ", nContactID);
						strDelete += str;
						str.Format("DELETE FROM UB92EBillingSetupT WHERE ProviderID = %li; ", nContactID);
						strDelete += str;
						str.Format("DELETE FROM InsuranceNetworkID WHERE ProviderID = %li; ", nContactID);
						strDelete += str;
						str.Format("DELETE FROM InsuranceBox51 WHERE ProviderID = %li; ", nContactID);
						strDelete += str;
						str.Format("DELETE FROM InsuranceBox31 WHERE ProviderID = %li; ", nContactID);
						strDelete += str;
						str.Format("DELETE FROM InsuranceBox24J WHERE ProviderID = %li; ", nContactID);
						strDelete += str;
						str.Format("DELETE FROM InsuranceGroups WHERE ProviderID = %li; ", nContactID);
						strDelete += str;
						// (j.jones 2008-04-02 14:30) - PLID 28995 - ensure we delete from HCFAClaimProvidersT
						str.Format("DELETE FROM HCFAClaimProvidersT WHERE ProviderID = %li", nContactID);
						strDelete += str;
						// (j.jones 2009-02-09 09:40) - PLID 32951 - remove CLIA setup
						str.Format("DELETE FROM CLIANumbersT WHERE ProviderID = %li", nContactID);
						strDelete += str;
						// (j.jones 2010-07-06 15:21) - PLID 39506 - delete from RealTimeEligibilityLoginInfoT
						str.Format("DELETE FROM ClearinghouseLoginInfoT WHERE ProviderID = %li", nContactID);
						strDelete += str;

						// (j.luckoski 2013-03-27 17:15) - PLID 55913 - Clear providerID if its being deleted.
						str.Format("Update UsersT SET ProviderID = NULL WHERE ProviderID = %li", nContactID);
						strDelete += str;

						ExecuteSqlStd(strDelete);
						
						// (j.jones 2014-08-04 17:32) - PLID 63141 - send an Ex tablechecker with this person ID, but no MailID,
						// don't bother with the photo status since contacts do not have photos
						CClient::RefreshMailSentTable(nContactID, -1);
					}

					//Section C) Deletes
					{
						CString strDelete, str;
						//(e.lally 2005-11-28) PLID 18460 - We need to delete custom data that references the contact too.
						str.Format("DELETE FROM CustomFieldDataT WHERE PersonID  = %li "
							"OR (IntParam = %li AND FieldID = %li); ", nContactID, nContactID, 31);
						strDelete += str;
						str.Format("DELETE FROM QBooksAcctsToProvidersT WHERE ProviderID  = %li; ", nContactID);
						strDelete += str;
						// (j.jones 2009-08-24 11:36) - PLID 35124 - changed to PreferenceCardProvidersT
						str.Format("DELETE FROM PreferenceCardProvidersT WHERE ProviderID  = %li; ", nContactID);
						strDelete += str;
						str.Format("DELETE FROM MultiFeeProvidersT WHERE ProviderID = %li; ", nContactID);
						strDelete += str;
						// (j.jones 2008-06-17 09:03) - PLID 30410 - removed HCFAT from the program
						//str.Format("UPDATE HCFAT SET ProviderID = -1 WHERE ProviderID = %li; ", nContactID);
						//strDelete += str;
						str.Format("UPDATE HCFADocumentsT SET ProviderIndex = -1 WHERE ProviderIndex = %li; ", nContactID);
						strDelete += str;

						// (j.jones 2009-06-08 11:53) - PLID 34514 - clear out UsersT.NewCropProviderID
						str.Format("UPDATE UsersT SET NewCropProviderID = NULL WHERE NewCropProviderID = %li", nContactID);
						strDelete += str;
						// (j.jones 2009-08-24 17:38) - PLID 35203 - and NewCropSupervisingProviderID
						str.Format("UPDATE UsersT SET NewCropSupervisingProviderID = NULL WHERE NewCropSupervisingProviderID = %li", nContactID);
						strDelete += str;
						// (j.jones 2011-06-17 09:27) - PLID 41709 - and NewCropMidLevelProviderID
						str.Format("UPDATE UsersT SET NewCropMidLevelProviderID = NULL WHERE NewCropMidLevelProviderID = %li", nContactID);
						strDelete += str;

						//TES 10/20/2008 - PLID 31414 - Providers can now be referenced in HL7IDLinkT
						str.Format("DELETE FROM HL7IDLinkT WHERE PersonID = %li; ", nContactID);
						strDelete += str;
						// (r.gonet 09/03/2013) - PLID 56007 - Providers are referenced in medication history requests.
						//  Since this is read only data, is easy to reget, and is requested automatically, I decided to
						//  delete the medication history request records rather associated rather than prevent the deletion.
						str.Format(
							"DELETE FROM MedicationHistoryResponseT WHERE ID IN "
							"( "
								"SELECT MedicationHistoryResponseT.ID "
								"FROM MedicationHistoryResponseT "
								"INNER JOIN RxHistoryRequestT ON MedicationHistoryResponseT.RxHistoryRequestID = RxHistoryRequestT.ID "
								"INNER JOIN RxHistoryMasterRequestT ON RxHistoryRequestT.RxHistoryMasterRequestID = RxHistoryMasterRequestT.ID "
								"WHERE RxHistoryMasterRequestT.ProviderID = %li "
								") ", nContactID);
						strDelete += str;
						str.Format(
							"DELETE FROM RxHistoryRequestT WHERE ID IN "
							"( "
								"SELECT RxHistoryRequestT.ID "
								"FROM RxHistoryRequestT "
								"INNER JOIN RxHistoryMasterRequestT ON RxHistoryRequestT.RxHistoryMasterRequestID = RxHistoryMasterRequestT.ID "
								"WHERE RxHistoryMasterRequestT.ProviderID = %li "
							") ", nContactID);
						strDelete += str;
						str.Format(
							"DELETE FROM RxHistoryMasterRequestT WHERE ID IN "
							"( "
								"SELECT RxHistoryMasterRequestT.ID "
								"FROM RxHistoryMasterRequestT "
								"WHERE RxHistoryMasterRequestT.ProviderID = %li "
							") ", nContactID);
						strDelete += str;

						ExecuteSqlStd(strDelete);
					}

					//TES 6/15/2004: This is redundant, but this will ensure that everything gets deleted that should be.
					//See PLID 
					DeleteProvider();
					//Regular deletion
					//(r.wilson 7/29/2013) PLID 48684 - Remove default primary Emr links 
					ExecuteSql("Update ProvidersT SET EMRDefaultProviderID = NULL WHERE EMRDefaultProviderID = %li", nContactID);
					ExecuteSql("DELETE FROM ProvidersT WHERE ProvidersT.PersonID = %li",nContactID);
					ExecuteSql("DELETE FROM PersonT WHERE PersonT.ID = %li", nContactID);				

					bError = false;
				} END_TRANS_CATCH_ALL("ProviderDelete");

				AuditItem = aeiProviderDeleted;

			}NxCatchAllCall("Error in CMainFrame::OnContactDelete", bError = true;);

			// (a.walling 2007-08-06 12:33) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
			CClient::RefreshTable(NetUtils::Providers, nContactID); // Network stuff

			if(!bError)
				MsgBox("Contact has been successfully deleted!");
		}

		else if(nStatus & 0x4)
		{	//delete UsersT

			//TES 3/22/2004: Don't say we succeeded if we didn't.
			bool bError = false;

			if(GetActiveContactID() == GetCurrentUserID())
			{
				MessageBox("You cannot delete yourself from Practice.  Please use another account to accomplish this task.");
				return;
			}

			// (c.haag 2015-08-24) - PLID 67198 - Use a utility function to check all processing methods
			if (DoesUserHaveCreditCardTransactions(GetActiveContactID()))
			{
				MsgBox("This user is linked to Credit Card transactions and cannot be deleted.  Please inactivate this user instead.");
				return;
			}


			// (j.armen 2012-04-10 14:10) - PLID 48299 - Cleaned up into one recordset
			// (j.armen 2012-04-10 14:11) - PLID 48299 - Added check for recalls
			// (j.jones 2013-01-16 10:11) - PLID 54634 - added E/M coding progress tables
			// (b.savon 2013-02-08 16:16) - PLID 54656 - Add check for Nurse Staff on Prescriptions
			// (d.singleton 2013-03-21 15:27) - PLID 55817 - Need to select EMNID not ID from EMNEMChecklistCodingLevelProgressT and EMNEMChecklistRuleProgressT
			CSqlFragment sqlCanDeleteCheck(
				"DECLARE @UserID INT\r\n"
				"SET @UserID = {INT}\r\n"
				"SELECT\r\n"
				"	CASE WHEN EXISTS(SELECT Administrator FROM UsersT WHERE Administrator = 1 AND PersonID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS IsAdmin,\r\n"
				"	CASE WHEN NOT EXISTS(SELECT PersonID FROM UserLocationT INNER JOIN LocationsT ON UserLocationT.LocationID = LocationsT.ID WHERE PersonID <> @UserID AND Managed = 1)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS IsLastLoginUser,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ResultID FROM LabResultsT WHERE ResultSignedBy = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasLabResultSignatures,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ResultID FROM LabResultsT WHERE ResultCompletedBy = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasLabResultsCompleted,\r\n"
				"	CASE WHEN EXISTS(SELECT ExportedBy FROM ExportHistoryT WHERE ExportedBy = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasExportHistory,\r\n"
				"	CASE WHEN EXISTS(SELECT UserID FROM OutlookProfileT WHERE UserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasNexPDASync,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM LabsT WHERE InputBy = @UserID OR DeletedBy = @UserID OR MedAssistant = @UserID OR SignedBy = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasLabRecords,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ResultID FROM LabResultsT WHERE DeletedBy = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasLabResults,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM EMRProblemHistoryT WHERE UserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasEMRProblemHistory,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM RoomAppointmentsT WHERE LastUpdateUserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasRoomAppointment,\r\n"
				// (a.walling 2013-06-07 09:45) - PLID 57078 - RoomAppointmentHistoryT no longer has an ID column; use the same as the predicate
				"	CASE WHEN EXISTS(SELECT TOP 1 UpdateUserID FROM RoomAppointmentHistoryT WHERE UpdateUserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasRoomAppointmentHistory,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM EMRMasterSlipT WHERE UserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasEMRMasterSlip,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM EMRGroupsSlipT WHERE UserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasEMRGroupsSlip,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ResultID FROM LabResultsT WHERE AcknowledgedUserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasAcknowledgedLabResults,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ApprovedBy FROM EMChecklistCodingLevelsT WHERE ApprovedBy = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasApprovedEMChecklistCodingLevels,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ApprovedBy FROM EMChecklistRulesT WHERE ApprovedBy = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasEMCheckListRules,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM PatientInvAllocationsT WHERE CompletedBy = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasInvAllocations,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM LaddersT WHERE InputUserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasTrackingLadders,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM FaxLogT WHERE UserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasFaxes,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 PersonID FROM PatientsT WHERE AllergiesReviewedBy = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasReviewedPatAllergies,\r\n"
				"	CASE WHEN EXISTS(SELECT ID FROM InvReconciliationsT WHERE StartedBy = @UserID OR CompletedBy = @UserID OR CancelledBy = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasInvReconciliations,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM MailSentExportFieldT WHERE UserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasMailSent,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM ReferralOrdersT WHERE InputUserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasReferralOrders,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM PatientMedications WHERE InputByUserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasPatMedications,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM CurrentPatientMedsT WHERE InputByUserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasCurPatMedications,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM PatientImmunizationsT WHERE CreatedUserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasPatImmunizations,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 UserID FROM AptShowStateHistoryT WHERE UserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasAptShowStateHistory,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 AssignTo FROM ToDoList INNER JOIN TodoAssignToT ON TodoAssignToT.TaskID = ToDoList.TaskID WHERE AssignTo = @UserID AND Done IS NOT NULL)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasCompletedToDos,\r\n"
				"	CASE WHEN EXISTS(SELECT UserID FROM FinancialCloseHistoryT WHERE UserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasFinCloseHistory,\r\n"
				"	CASE WHEN EXISTS(SELECT COUNT(*) AS UserCount FROM NexWebEventToDoAssignToT WHERE NexWebEventID IN(SELECT NexWebEventID FROM NexWebEventToDoAssignToT WHERE UserID = @UserID) GROUP BY NexWebEventID HAVING COUNT(*) = 1)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS IsOnlyNexWebToDoUser,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM GlassesOrderT WHERE UserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasGlassesOrder,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM GlassesOrderHistoryT WHERE UserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasGlassesOrderHistory,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 PersonID FROM EmrTechniciansT WHERE PersonID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS IsEMRTechnician,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 InputUserID FROM BillCorrectionsT WHERE InputUserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasBillCorrections,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 InputUserID FROM LineItemCorrectionsT WHERE InputUserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasLineItemCorrections,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 UserID FROM UserLoginTokensT WHERE UserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS IsCurrentlyLoggedIn,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 ID FROM RecallT WHERE CreatedUserID = @UserID OR ModifiedUserID = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasRecalls,\r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 EMNID FROM EMNEMChecklistRuleProgressT WHERE ApprovedBy = @UserID)\r\n"
				"		OR EXISTS(SELECT TOP 1 EMNID FROM EMNEMChecklistCodingLevelProgressT WHERE ApprovedBy = @UserID)\r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasEMChecklistProgress, \r\n" 
				"	CASE WHEN EXISTS(SELECT TOP 1 NurseStaffID FROM PatientMedications WHERE NurseStaffID = @UserID) \r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasPrescription, \r\n"
				"	CASE WHEN EXISTS(SELECT TOP 1 UserID FROM NexERxNurseStaffPrescriberT WHERE UserID = @UserID) \r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasNexERxNurseStaffRole, \r\n"
				// (v.maida 2014-08-14 15:50) - PLID 62753 - Don't allow the contact to be deleted if it is a user that has been selected to be assigned a Todo alarm for diagnosis codes spawned in different diagnosis modes.
				"	CASE WHEN EXISTS (SELECT TOP 1 1 FROM dbo.StringToIntTable((SELECT MemoParam FROM ConfigRT WHERE Name = 'TodoUsersWhenDiagCodeSpawnedInDiffDiagMode'), ',') AS PrefUsersQ WHERE PrefUsersQ.Data = @UserID) \r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasDiffDiagModeTodoCreationPreference, \r\n"
				//(s.dhole 9/3/2014 12:00 PM ) - PLID 63552 Check is user used in PatientRemindersSentT
				"	CASE WHEN EXISTS(SELECT TOP 1 UserID FROM PatientRemindersSentT WHERE UserID = @UserID) \r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasPatientRemindersSentRecord, \r\n"
				// (j.jones 2014-11-26 16:00) - PLID 64272 - disallow deleting a user who override an appointment mix rule
				"	CASE WHEN EXISTS(SELECT TOP 1 UserID FROM AppointmentMixRuleOverridesT WHERE UserID = @UserID) \r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasOverriddenApptMixRule, \r\n"
				// (j.jones 2015-04-23 12:33) - PLID 65711 - disallow deleting a user who has transferred GC balances
				"	CASE WHEN EXISTS(SELECT TOP 1 InputByUserID FROM GiftCertificateTransfersT WHERE InputByUserID = @UserID) \r\n"
				"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasTransferredGCBalances \r\n",
				GetActiveContactID());

			if(IsNexTechInternal()) {
				sqlCanDeleteCheck += CSqlFragment(
					"	,\r\n"
					"	CASE WHEN EXISTS(SELECT UserID FROM AttendanceAppointmentsT WHERE UserID = @UserID OR ApprovedBy = @UserID)\r\n"
					"		THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS HasAttendanceData");
			}

			_RecordsetPtr prs = CreateParamRecordset(sqlCanDeleteCheck);

			//If the user is an Administrator, you CANNOT DELETE it
			if(AdoFldBool(prs, "IsAdmin")) {
				MsgBox("This user is an Administrator and cannot be deleted.\n\n"
					"You may, however, deactivate the user's administrator status and delete them,\n"
					"if another administrator user exists.");
				return;
			}

			//if we take away this user, there is noone else allowed to login
			if(AdoFldBool(prs, "IsLastLoginUser")) {
				MessageBox("You must have at least 1 user allowed to login to Practice.  Deleting this user would break that constraint.");
				return;
			}

			// (c.haag 2010-12-10 13:12) - PLID 37372 - Don't allow deleting a user who signed off on a lab result
			if (AdoFldBool(prs, "HasLabResultSignatures")) {
				MessageBox("This user is associated with lab result signatures and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			// (c.haag 2010-12-10 10:15) - PLID 41590 - Don't allow deleting a user who completed a lab
			if (AdoFldBool(prs, "HasLabResultsCompleted")) {
				MessageBox("This user is associated with completed lab results and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			// (c.haag 2005-10-10 15:22) - PLID 17846 - Do not let someone delete a user that has
			// export history records associated with them.
			if (AdoFldBool(prs, "HasExportHistory")) {
				MsgBox("This user has historical patient export records associated with it, and may not be deleted.");
				return;
			}

			// (c.haag 2005-10-10 15:22) - PLID 17847 - Check if the person is using the NexPDA link.
			// (z.manning 2009-11-17 12:31) - PLID 31879 - May also be a NexSync profile
			if (AdoFldBool(prs, "HasNexPDASync")) {
				MsgBox("This user is associated with at least one NexPDA or NexSync profile. Before deleting this user, you must delete their profile from NexSync or the NexPDA link.\n\n");
				return;
			}

			//DRT 6/30/2006 - PLID 21290 - Cannot delete users which have entered, deleted, or completed Labs
			// (a.walling 2006-07-25 12:59) - PLID 21571 - Also added users who are assistants.
			// (c.haag 2010-12-10 10:15) - PLID 41590 - Check SignedBy
			// (c.haag 2011-01-17) - PLID 41806 - Removed CompletedBy
			if(AdoFldBool(prs, "HasLabRecords")) {
				MsgBox("This user is associated with at least one lab record and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (j.gruber 2008-11-06 16:28) - PLID 31432 - check lab results
			if(AdoFldBool(prs, "HasLabResults")) {
				MsgBox("This user is associated with at least one lab result record and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (c.haag 2006-09-19 14:44) - PLID 22260 - Cannot delete users associated who created or modified EMR problems
			if (AdoFldBool(prs, "HasEMRProblemHistory")) {
				MsgBox("This user is associated with at least one EMR problem record, and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (j.jones 2006-09-25 15:00) - PLID 22604 - check Room details and history
			if (AdoFldBool(prs, "HasRoomAppointment") || AdoFldBool(prs, "HasRoomAppointmentHistory")) {
				MsgBox("This user is associated with at least one historical Scheduler Room record, and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (j.jones 2007-01-29 11:45) - PLID 24353 - can't delete users who have edited EMNs
			// (j.jones 2007-02-06 13:38) - PLID 24509 - or EMRs (though the odds of an EMR record without an EMN record are slim)
			if (AdoFldBool(prs, "HasEMRMasterSlip") || AdoFldBool(prs, "HasEMRGroupsSlip")) {
				MsgBox("This user is associated with at least one EMR record, and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (c.haag 2009-05-13 11:39) - PLID 28561 - Check for associations with lab reviews
			if (AdoFldBool(prs, "HasAcknowledgedLabResults")) {
				MsgBox("This user has reviewed at least one lab result, and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (j.jones 2007-08-23 08:42) - PLID 27055 - cannot delete users who approved E/M checklist information
			if (AdoFldBool(prs, "HasApprovedEMChecklistCodingLevels") || AdoFldBool(prs, "HasEMCheckListRules")) {
				MsgBox("This user is associated with at least one E/M Checklist record, and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (j.jones 2007-11-29 09:16) - PLID 28196 - can't delete users who have completed allocations
			if (AdoFldBool(prs, "HasInvAllocations")) {
				MsgBox("This user is associated with at least one completed inventory allocation, and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (z.manning 2009-01-27 09:25) - PLID 32187 - No deleting users who entered a tracking ladder
			if (AdoFldBool(prs, "HasTrackingLadders")) {
				MessageBox("This user is associated with tracking ladders and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (z.manning, 02/19/2008) - PLID 28216 - Don't allow deletion of users who have attendance data.
			if(IsNexTechInternal())
			{
				if(AdoFldBool(prs, "HasAttendanceData"))
				{
					MessageBox("This user is associated with attendance data and may not be deleted.");
					return;
				}
			}

			//DRT 7/3/2008 - PLID 30524 - Cannot delete once faxes have been logged.
			if(AdoFldBool(prs, "HasFaxes")) {
				MsgBox("This user has logged faxes associated, please inactivate this user instead.");
				return;
			}

			// (j.jones 2008-11-25 09:17) - PLID 28508 - can't delete any user who is tracked as having reviewed patient allergies
			if(AdoFldBool(prs, "HasReviewedPatAllergies")) {
				MsgBox("This user has reviewed patient allergies, and cannot be deleted. Please inactivate this user instead.");
				return;
			}

			// (j.jones 2009-01-13 17:31) - PLID 26141 - disallow if linked to an inv. reconciliation
			if(AdoFldBool(prs, "HasInvReconciliations")) {
				MsgBox("This user is linked to Inventory Reconciliations, and cannot be deleted. Please inactivate this user instead.");
				return;
			}

			// (d.thompson 2009-04-14 16:05) - PLID 33957 - Cannot delete users that have processed credit cards
			// (d.thompson 2010-12-20) - PLID 41897 - Chase too
			// (c.haag 2015-08-24) - PLID 67198 - Moved to a utility function called before the query was run

			// (c.haag 2009-10-12 13:02) - PLID 35722 - Cannot delete users that are assigned with exported MailSent records. Only applies
			// to NexPhoto exports right now.
			if(AdoFldBool(prs, "HasMailSent")) {
				MessageBox("This user is linked to exported patient photos and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			// (z.manning 2009-05-07 09:09) - PLID 28529 - Can't delete users tied to a referral order
			if(AdoFldBool(prs, "HasReferralOrders")) {
				MessageBox("This user is linked to referral orders and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			// (j.jones 2010-01-22 12:27) - PLID 37016 - can't delete users tied to a prescription or current medication
			if(AdoFldBool(prs, "HasPatMedications")) {
				MessageBox("This user is linked to patient prescriptions and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			if(AdoFldBool(prs, "HasCurPatMedications")) {
				MessageBox("This user is linked to patient current medications and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			// (d.thompson 2009-05-18) - PLID 34232 - Can't delete users tied to immunizations
			if(AdoFldBool(prs, "HasPatImmunizations")) {
				MessageBox("This user is linked to immunization history and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			// (j.jones 2009-09-11 17:01) - PLID 35145 - can't delete users tied to AptShowStateHistoryT
			if(AdoFldBool(prs, "HasAptShowStateHistory")) {
				MessageBox("This user is linked to appointment histories and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			// (j.jones 2009-09-21 13:56) - PLID 35595 - don't allow deleting a user with completed to-dos
			if(AdoFldBool(prs, "HasCompletedToDos")) {
				MessageBox("This user is linked to completed ToDo tasks and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			// (j.jones 2010-12-20 17:40) - PLID 41852 - check for FinancialCloseHistoryT records
			if (AdoFldBool(prs, "HasFinCloseHistory")) {
				MessageBox("This user is linked to a close of financial data and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			//(e.lally 2010-08-18) PLID 37982 - don't allow deleting a user if they are the only one assigned as the NexWeb ToDo owner
			//	silently remove otherwise.
			//(e.lally 2010-11-19) PLID 35819 - Migrated from NexWeb Leads AssignTo Users preference to its own table structure
			//(e.lally 2012-01-03) PLID 47136 - NexWeb_ToDo_Template_Assign_To_T is now NexWeb_Event_Assign_To_T
			if(AdoFldBool(prs, "IsOnlyNexWebToDoUser")){
				//this is the only user assigned, prevent the delete
				MessageBox("This user is the default owner of new ToDo Alarms for events in NexWeb. \r\n"
					"Please select a new user under the Links tab of the Administrator module before changing the contact type for this user.");
				return;
			}

			// (s.dhole 2011-02-24 12:24) - PLID 40535 check if user is link to any glasses order or order history
			// (s.dhole 2010-03-10 17:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
			// (j.dinatale 2012-04-09 18:36) - PLID 49332 - check the optician field also
			// (j.dinatale 2012-04-17 17:26) - PLID 49078 - changed glasses order to optical order
			if(AdoFldBool(prs, "HasGlassesOrder") || AdoFldBool(prs, "HasGlassesOrderHistory")) {
				MsgBox("This user is linked to Optical orders and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			// (d.lange 2011-03-24 16:46) - PLID 42943 - Can't delete users that are associated with EMNs as Assistant/Tech
			if(AdoFldBool(prs, "IsEMRTechnician")) {
				MsgBox("This user is associated with at least one EMN as the Assistant/Technician, therefore cannot be deleted.  "
					"Please inactivate this user instead.");
				return;
			}

			// (j.jones 2011-09-15 15:21) - PLID 44905 - disallow if the user is linked to BillCorrectionsT records
			if(AdoFldBool(prs, "HasBillCorrections")) {
				MsgBox("This user is associated with at least one corrected bill, and therefore cannot be deleted.  "
					"Please inactivate this user instead.");
				return;
			}

			// (j.jones 2011-09-15 15:21) - PLID 44905 - disallow if the user is linked to LineItemCorrectionsT records
			if(AdoFldBool(prs, "HasLineItemCorrections")) {
				MsgBox("This user is associated with at least one corrected line item in billing, and therefore cannot be deleted.  "
					"Please inactivate this user instead.");
				return;
			}

			// (j.jones 2011-10-06 16:43) - PLID 45828 - if this user is logged in from a device, you cannot delete the user
			if(AdoFldBool(prs, "IsCurrentlyLoggedIn")) {
				MsgBox("This user is currently logged in from a device, and cannot be deleted.");
				return;
			}

			// (j.armen 2012-04-03 13:19) - PLID 48299 - Don't allow deleting if the are associated w/ recalls
			if(AdoFldBool(prs, "HasRecalls")) {
				MsgBox("This user is associated with recalls, and cannot be deleted.");
				return;
			}

			// (j.jones 2013-01-16 10:11) - PLID 54634 - added E/M coding progress tables
			if(AdoFldBool(prs, "HasEMChecklistProgress")) {
				MsgBox("This user is linked to E/M Checklist progress on EMN records, and cannot be deleted. Please inactivate this user instead.");
				return;
			}


			// (b.savon 2013-02-08 16:18) - PLID 54656 - Dont allow deleting if they are associated with a prescription
			if( AdoFldBool(prs, "HasPrescription") ){
				MsgBox("This user is associated with at least one prescription, and cannot be deleted.");
				return;
			}

			// (b.savon 2013-03-01 14:24) - PLID 54578 - Dont allow deleting if they have a nurse/staff role
			if( AdoFldBool(prs, "HasNexERxNurseStaffRole") ){
				MsgBox("This user has a Nurse/Staff role and cannot be deleted.");
				return;
			}

			// (v.maida 2014-08-14 15:50) - PLID 62753 - Don't allow the contact to be deleted if it is a user that has been selected to be assigned a Todo alarm for diagnosis codes spawned in different diagnosis modes.
			if ( AdoFldBool(prs, "HasDiffDiagModeTodoCreationPreference") ){
				MsgBox("This user has been selected to be assigned Todo alarms when diagnosis codes are spawned while in a different global diagnosis mode, and thus cannot be deleted."
					"\r\nNavigate to Tools->Preferences...->Patients Module->NexEMR->EMR 3 tab->'Create a Todo when a diagnosis code is spawned while in a different global diagnosis mode' preference and uncheck this user's name from that preference's datalist to lift this restriction.");
				return;
			}

			//(s.dhole 9/3/2014 11:57 AM ) - PLID 63552 Check is user used in PatientRemindersSentT
			if (AdoFldBool(prs, "HasPatientRemindersSentRecord")) {
				MessageBox("This user is associated with patient reminder sent history and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			// (j.jones 2014-11-26 16:00) - PLID 64272 - disallow deleting a user who override an appointment mix rule
			if (AdoFldBool(prs, "HasOverriddenApptMixRule")) {
				MessageBox("This user has overridden appointments for scheduling mix rules and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			// (j.jones 2015-04-23 12:33) - PLID 65711 - disallow deleting a user who has transferred GC balances
			if (AdoFldBool(prs, "HasTransferredGCBalances")) {
				MessageBox("This user has transferred Gift Certificate balances, and for historical tracking purposes cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			/*
			Things allowed to be deleted go below here
			*/

			if (MessageBox("This is an unrecoverable operation! Are you absolutely sure you wish to delete this contact?", "Confirm Delete", MB_ICONEXCLAMATION|MB_YESNO) != IDYES )
				return;

			try {
				// (c.haag 2008-06-10 09:28) - PLID 11599 - Use TodoAssignToT
				// (j.jones 2009-08-24 12:12) - PLID 35124 - personnel are now only in Preference Cards, not surgeries
				// (j.gruber 2012-06-19 16:46) - PLID 44026 - dashboard
				rs = CreateParamRecordset("SELECT * FROM (SELECT BillsT.PatCoord AS PersonID FROM BillsT UNION SELECT ToDoList.PersonID FROM ToDoList UNION SELECT Notes.PersonID FROM Notes UNION SELECT CustomFieldDataT.PersonID FROM CustomFieldDataT UNION (SELECT CustomFieldDataT.IntParam FROM CustomFieldDataT WHERE FieldID = 31) UNION SELECT GroupDetailsT.PersonID FROM GroupDetailsT UNION SELECT MailSent.PersonID FROM MailSent "
					"UNION SELECT PreferenceCardDetailsT.PersonID FROM PreferenceCardDetailsT "
					"UNION SELECT UserLocationT.PersonID FROM UserLocationT UNION SELECT ID FROM UserPermissionsT UNION SELECT UserID FROM PersonT UNION SELECT ModifiedBy FROM OrderDetailsT UNION SELECT UserID FROM Notes UNION SELECT InputName FROM BillsT UNION SELECT CreatedBy FROM OrderT "
					"UNION SELECT UserID FROM ReturnedProductsT UNION SELECT Login FROM ProductAdjustmentsT UNION SELECT EnteredBy FROM ToDoList UNION SELECT AssignTo FROM TodoAssignToT UNION SELECT EmployeeID FROM PatientsT UNION SELECT UserID FROM ProductResponsibilityT UNION SELECT UserID FROM ResourceUserLinkT "
					"UNION SELECT UserID FROM FaxConfigT UNION SELECT UserID FROM PatientDashboardUserConfigT) SubQ WHERE PersonID = {INT}", GetActiveContactID());
			} NxCatchAllCall("Error deleting user", bError = true;);

			if(!rs->eof)
			{	//contact has data... clean it out first
				if(MessageBox("This contact currently has data associated with it.  Removing this contact will remove ALL related data, are you SURE you wish to do this?", "Confirm Delete", MB_ICONEXCLAMATION|MB_YESNO) != IDYES)
					return;

				rs->Close();

				//DeleteUser had messageboxes in it.  You can't have a message box inside a transaction!")

				//If they have any messages, find out whether to delete them.
				BOOL bDeleteMessages = FALSE;
				if(ReturnsRecords("SELECT MessageID FROM MessagesT WHERE SenderID = %li OR RecipientID = %li", GetActiveContactID(), GetActiveContactID())) {
					if (IDYES == MsgBox(MB_YESNO, "This user has messages associated with it in the PracYakker.  Would you like to delete them?\nIf you say \"Yes\", all messages sent to or sent by this user will be permanently deleted.  If you say \"No\", all messages associated with this user will be marked as <Deleted User>."))
						bDeleteMessages = TRUE;
					else
						bDeleteMessages = FALSE;
				}

				long nAssignStepsTo = -1;
				long nAssignTodosTo = -1;
				long nAssignEmrTodoActionsTo = -1; // (c.haag 2008-06-23 13:02) - PLID 30471
				// (z.manning 2008-10-13 08:36) - PLID 21108 - Also check for lab procedure steps
				// (j.jones 2008-11-26 13:30) - PLID 30830 - changed StepsT/StepTemplatesT references to support the multi-user tables
				//(e.lally 2010-08-27) PLID 40282 - Fixed a typo with the parentheses that was causing all users to get this prompt
				if(ReturnsRecords("SELECT UserID FROM StepsAssignToT WHERE UserID = %li UNION SELECT UserID FROM StepTemplatesAssignToT WHERE UserID = %li UNION SELECT UserID FROM LaddersT WHERE UserID = %li UNION SELECT UserID FROM LabProcedureStepTodoAssignToT WHERE UserID = %li", GetActiveContactID(), GetActiveContactID(), GetActiveContactID(), GetActiveContactID() )) {
					CSelectUserDlg dlg(this);
					dlg.m_nExcludeUser = GetActiveContactID();
					dlg.m_strCaption = "This user has Tracking steps assigned to them. Please select a user for these steps to be re-assigned to.";
					if(IDOK == dlg.DoModal()) {
						nAssignStepsTo = dlg.m_nSelectedUser;
					}
					else {
						return;
					}
				}

				// (a.walling 2006-09-11 13:48) - PLID 22458 - Prompt to reassign todo tasks
				// (c.haag 2008-06-10 09:28) - PLID 11599 - Use TodoAssignToT
				// (j.jones 2009-09-21 14:16) - PLID 35595 - we should not be able to get here
				// if the user has completed todos, so we do not need to filter on the completion status
				if(ReturnsRecords("SELECT AssignTo FROM TodoAssignToT WHERE AssignTo = %li", GetActiveContactID())) {
					CSelectUserDlg dlg(this);
					dlg.m_nExcludeUser = GetActiveContactID();
					dlg.m_bAllowNone = false;
					dlg.m_strCaption = "This user has unfinished todo alarms assigned to them. Please select a user for these alarms to be re-assigned to.";
					if (IDOK == dlg.DoModal()) {
						nAssignTodosTo = dlg.m_nSelectedUser;
					}
					else {
						return;
					}
				}

				// (c.haag 2008-06-23 12:59) - PLID 30471 - Prompt if any EMR todo actions involve this user
				if(ReturnsRecords("SELECT ActionID FROM EMRActionsTodoAssignToT WHERE AssignTo = %li", GetActiveContactID())) {
					CSelectUserDlg dlg(this);
					dlg.m_nExcludeUser = GetActiveContactID();
					dlg.m_bAllowNone = false;
					dlg.m_strCaption = "This user has EMR Todo spawning actions assigned to them. Please select a user for these actions to be re-assigned to.";
					if (IDOK == dlg.DoModal()) {
						nAssignEmrTodoActionsTo = dlg.m_nSelectedUser;
					}
					else {
						return;
					}
				}

				BEGIN_TRANS("ContactDelete") {
					bError = true;
					// (c.haag 2008-06-23 15:28) - PLID 30471 - Also reassign EMR Todo spawn actions
					DeleteUser(bDeleteMessages, nAssignStepsTo, nAssignTodosTo, nAssignEmrTodoActionsTo);
					DeleteCommonContactInfo();
					bError = false;
				} END_TRANS_CATCH_ALL("ContactDelete");			
			}
			else
				rs->Close();


			//delete from UsersT
			//delete from PersonT
			try{

				// (c.haag 2005-10-20 12:31) - PLID 18013 - This code is now obselete. All the deleting
				// for the NexPDA link with the new structure now takes place in CMainFrame::DeleteUser
				//
				//_RecordsetPtr prsOutlook = CreateRecordset("SELECT ID FROM OutlookSyncProfileT WHERE UserID = %d", GetActiveContactID());
				//while (!prsOutlook->eof)
				//{
				//	NxOutlookLink::DeleteProfile(AdoFldLong(prsOutlook, "ID"));
				//	prsOutlook->MoveNext();
				//}

				// (j.gruber 2007-11-07 12:37) - PLID 28026 - take off the EMRSpecialist if this is internal
				if (IsNexTechInternal()) {
					ExecuteParamSql("DELETE FROM EMRSpecialistT WHERE UserID = {INT}", GetActiveContactID());
					// (z.manning, 02/19/2008) - PLID 28216 - Also delete from attendance tables if internal.
					ExecuteParamSql("DELETE FROM UserDepartmentLinkT WHERE UserID = {INT}", GetActiveContactID());
					ExecuteParamSql("DELETE FROM DepartmentManagersT WHERE UserID = {INT}", GetActiveContactID());
					ExecuteParamSql("DELETE FROM AttendanceAllowanceHistoryT WHERE UserID = {INT}", GetActiveContactID());
					ExecuteParamSql("DELETE FROM AttendanceAdjustmentsT WHERE UserID = {INT}", GetActiveContactID());
				}


				//b.spivey - November 13th, 2013 - PLID 59022 - Delete this when deleting contacts. 
				ExecuteParamSql("SET NOCOUNT ON "
					"	"
					"DECLARE @ContactID INT "
					"SET @ContactID = {INT} "
					"	"
					"DELETE DirectAddressUserT FROM DirectAddressUserT DAUT "
					"INNER JOIN DirectAddressFromT DAFT ON DAUT.DirectAddressFromID = DAFT.ID "
					"WHERE DAFT.PersonID = @ContactID "
					"	"
					"DELETE FROM DirectAddressFromT WHERE PersonID = @ContactID "
					"	"
					"SET NOCOUNT OFF ", GetActiveContactID());

				// (c.haag 2014-02-21) - PLID 60953 - Delete from diagnosis quicklists
				ExecuteParamSql("DELETE FROM DiagnosisQuickListT WHERE UserID = {INT}", GetActiveContactID());
				ExecuteParamSql("DELETE FROM DiagnosisQuickListSharingT WHERE UserID = {INT} OR SharingUserID = {INT}"
					,GetActiveContactID(), GetActiveContactID());
				// (j.jones 2009-05-20 09:26) - PLID 33801 - delete from UserPasswordHistoryT
				ExecuteParamSql("DELETE FROM UserPasswordHistoryT WHERE UserID = {INT}", GetActiveContactID());
				// (j.gruber 2010-04-14 12:55) - PLID 37948 - delete from permission groups
				ExecuteParamSql("DELETE FROM UserGroupDetailsT WHERE UserID = {INT}", GetActiveContactID());
				// (a.walling 2010-11-11 17:13) - PLID 41459 - Clear out YakGroupDetailsT
				ExecuteParamSql("DELETE FROM YakGroupDetailsT WHERE PersonID = {INT}", GetActiveContactID());
				// (j.gruber 2010-04-14 12:55) - PLID 38201 - delete permissions
				ExecuteParamSql("DELETE FROM PermissionT WHERE UserGroupID = {INT}", GetActiveContactID());			
				//(e.lally 2010-11-19) PLID 35819 - delete from NexWeb ToDo template setup
				//(e.lally 2012-01-03) PLID 47136 - NexWeb_ToDo_Template_Assign_To_T is now NexWeb_Event_Assign_To_T
				ExecuteParamSql("DELETE FROM NexWebEventToDoAssignToT WHERE UserID = {INT}", GetActiveContactID());
				// (z.manning 2014-08-22 09:08) - PLID 63251 - Handle UserLocationResourceExclusionT
				ExecuteParamSql("DELETE FROM UserLocationResourceExclusionT WHERE UserID = {INT}", GetActiveContactID());
				// (z.manning 2014-09-09 10:18) - PLID 63260
				ExecuteParamSql("DELETE FROM UserRoomExclusionT WHERE UserID = {INT}", GetActiveContactID());
				ExecuteSql("DELETE FROM UserResourcesT WHERE UserID = %li", GetActiveContactID());
				ExecuteSql("DELETE FROM UsersT WHERE UsersT.PersonID = %li", GetActiveContactID());			
				ExecuteSql("DELETE FROM PersonT WHERE PersonT.ID = %li", GetActiveContactID());
				ExecuteSql("UPDATE PalmContactsInfoT SET Status = 4 WHERE PersonID = %li", GetActiveContactID());

				AuditItem = aeiUserDeleted;

			}NxCatchAllCall("Error in CMainFrame::OnContactDelete", bError = true;);

			CClient::RefreshTable(NetUtils::Coordinators); // Network stuff

			if(!bError)
				MsgBox("Contact has been successfully deleted!");
		}

		else if (nStatus & 0x8)
		{	//delete SupplierT
			//	Suppliers cannot be deleted if they have orders (OrdersT)

			//TES 3/22/2004: Don't say we succeeded if we didn't.
			bool bError = false;

			//if there is an order, we do not allow the delete
			rs = CreateRecordset("SELECT OrderT.Supplier FROM OrderT WHERE OrderT.Supplier = %li AND OrderT.ID Is Not Null", GetActiveContactID());

			if(!rs->eof)
			{
				MsgBox("This supplier currently has orders associated with it, please remove these relationships if you wish to remove this contact");
				rs->Close();
				return;
			}
			rs->Close();

			// (c.haag 2007-11-13 16:28) - PLID 27994 - Forbid the user from deleting suppliers
			// tied to inventory returns.
			if (ReturnsRecords("SELECT ID FROM SupplierReturnGroupsT WHERE SupplierID = %d", GetActiveContactID()))
			{
				MsgBox(MB_ICONSTOP|MB_OK,"This supplier currently has inventory returns associated with it. You may not delete this supplier; however, you may inactivate it if it will no longer be used.");
				return;			
			}

			//(c.copits 2011-10-25) PLID 45709 - Deleting a supplier that is used in the Glasses Catalog Setup will generate a FK constraint error.
			if (CheckGlassesSupplierDesigns())
			{
				MsgBox(MB_ICONSTOP|MB_OK,"This supplier currently has glasses designs associated with it. You may not delete this supplier; however, you may inactivate it if it will no longer be used.");
				return;			
			}

			if (CheckGlassesSupplierMaterials())
			{
				MsgBox(MB_ICONSTOP|MB_OK,"This supplier currently has glasses materials associated with it. You may not delete this supplier; however, you may inactivate it if it will no longer be used.");
				return;			
			}

			if (CheckGlassesSupplierTreatments())
			{
				MsgBox(MB_ICONSTOP|MB_OK,"This supplier currently has glasses treatments associated with it. You may not delete this supplier; however, you may inactivate it if it will no longer be used.");
				return;			
			}

			if (CheckGlassesSupplierFrames())
			{
				MsgBox(MB_ICONSTOP|MB_OK,"This supplier currently has glasses frame types associated with it. You may not delete this supplier; however, you may inactivate it if it will no longer be used.");
				return;			
			}

			// (j.dinatale 2012-05-16 19:48) - PLID 50443 - check GlassesOrderT before deleting a supplier, contact lens dont have a setup catalog.
			if(ReturnsRecordsParam("SELECT TOP 1 1 FROM GlassesOrderT WHERE SupplierID = {INT}", GetActiveContactID())){
				MsgBox(MB_ICONSTOP|MB_OK,"This supplier currently has optical orders associated with it. You may not delete this supplier; however, you may inactivate it if it will no longer be used.");
				return;
			}

			if(MessageBox("This is an unrecoverable operation! Are you absolutely sure you wish to delete this supplier?", "Confirm Delete", MB_ICONEXCLAMATION|MB_YESNO) != IDYES )
				return;

			//check for any other supplier data in the database
			rs = CreateRecordset("SELECT * FROM (SELECT CustomFieldDataT.PersonID FROM CustomFieldDataT UNION (SELECT CustomFieldDataT.IntParam FROM CustomFieldDataT WHERE FieldID = 31) UNION SELECT GroupDetailsT.PersonID FROM GroupDetailsT UNION SELECT ToDoList.PersonID FROM ToDoList UNION SELECT MailSent.PersonID FROM MailSent UNION SELECT Notes.PersonID FROM Notes UNION SELECT SupplierID FROM MultiSupplierT) SubQ WHERE PersonID = %li", GetActiveContactID());	

			if(!rs->eof)
			{
				if(MessageBox("This contact currently has data associated with it.  Removing this contact will remove ALL related data, are you SURE you wish to do this?", "Confirm Delete", MB_ICONEXCLAMATION|MB_YESNO) != IDYES )
					return;

				//we've warned them enough
				rs->Close();
				BEGIN_TRANS("ContactDelete") {
					bError = true;
					DeleteSupplier();
					DeleteCommonContactInfo();
					bError = false;
				} END_TRANS_CATCH_ALL("ContactDelete");			
			}
			else
				rs->Close();

			// (s.dhole 2010-11-18 17:26) - PLID 41548 - Forbid the user from deleting suppliers
			// tied to VisionWeb.
			rs = CreateParamRecordset("SELECT * FROM SupplierT WHERE VisionWebID<>''  AND PersonID = {INT}", GetActiveContactID());
			if(!rs->eof){
				MsgBox(MB_ICONSTOP|MB_OK,"This supplier currently has VisionWeb data associated with it. You may not delete this supplier; Please mark this supplier as inactive.");
				return;			
			}
			else
				rs->Close(); 




			//delete from SupplierT
			//delete from PersonT

			long id = GetActiveContactID();
			BEGIN_TRANS("ContactDelete") {
				bError = true;
				DeleteSupplier(); // (c.haag 2003-09-04 10:38) - They may still have ties in MultiSupplierT here.
				ExecuteSql("DELETE FROM SupplierT WHERE SupplierT.PersonID = %li", id);
				ExecuteSql("DELETE FROM PersonT WHERE PersonT.ID = %li", id);
				ExecuteSql("UPDATE PalmContactsInfoT SET Status = 4 WHERE PersonID = %li", id);

				AuditItem = aeiSupplierDeleted;

				bError = false;
			} END_TRANS_CATCH_ALL("ContactDelete");

			CClient::RefreshTable(NetUtils::Suppliers, id);

			if(!bError)
				MsgBox("Contact has been successfully deleted!");
		}

		else
		{	//delete ContactsT
			if (MessageBox("This is an unrecoverable operation! Are you absolutely sure you wish to delete this contact?", "Confirm Delete", MB_ICONEXCLAMATION|MB_YESNO) != IDYES )
				return;

			//TES 3/22/2004: Don't say we succeeded if we didn't.
			bool bError = false;

			// (j.jones 2009-08-24 12:12) - PLID 35124 - personnel are now only in Preference Cards, not surgeries
			rs = CreateParamRecordset("SELECT * FROM "
				"(SELECT ToDoList.PersonID FROM ToDoList "
				"UNION SELECT Notes.PersonID FROM Notes UNION SELECT CustomFieldDataT.PersonID FROM CustomFieldDataT "
				"UNION (SELECT CustomFieldDataT.IntParam FROM CustomFieldDataT WHERE FieldID = 31) "
				"UNION SELECT GroupDetailsT.PersonID FROM GroupDetailsT UNION SELECT MailSent.PersonID FROM MailSent "
				"UNION SELECT NurseID FROM ProcedureT UNION SELECT AnesthesiologistID FROM ProcedureT "
				"UNION SELECT NurseID FROM ProcInfoT UNION SELECT AnesthesiologistID FROM ProcInfoT "
				"UNION SELECT PreferenceCardDetailsT.PersonID FROM PreferenceCardDetailsT) SubQ "
				"WHERE PersonID = {INT}", GetActiveContactID());

			if(!rs->eof)
			{	//contact has data... clean it out first
				if(MessageBox("This contact currently has data associated with it.  Removing this contact will remove ALL related data, are you SURE you wish to do this?", "Confirm Delete", MB_ICONEXCLAMATION|MB_YESNO) != IDYES)
					return;
				rs->Close();
				BEGIN_TRANS("ContactDelete") {
					bError = true;
					DeleteOtherContact();
					DeleteCommonContactInfo();
					bError = false;
				} END_TRANS_CATCH_ALL("ContactDelete");			
			}
			else
				rs->Close();
			//delete from ContactsT
			//delete from PersonT
			BEGIN_TRANS("ContactDelete") {
				bError = true;
				ExecuteSql("DELETE FROM Tops_AnesthesiaSupervisorLinkT WHERE PracPersonID = %li", GetActiveContactID());
				ExecuteSql("DELETE FROM ContactsT WHERE ContactsT.PersonID = %li", GetActiveContactID());
				ExecuteSql("DELETE FROM PersonT WHERE PersonT.ID = %li", GetActiveContactID());
				ExecuteSql("UPDATE PalmContactsInfoT SET Status = 4 WHERE PersonID = %li", GetActiveContactID());

				AuditItem = aeiOtherDeleted;

				bError = false;
			} END_TRANS_CATCH_ALL("ContactDelete");	

			CClient::RefreshTable(NetUtils::ContactsT);

			if(!bError)
				MsgBox("Contact has been successfully deleted!");
		}

		try {
			//do the contact deletion auditing last
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID!=-1) {
				AuditEvent(-1, GetActiveContactName(),nAuditID,AuditItem,GetActiveContactID(),"0","1",aepHigh,aetDeleted);
			}
		} NxCatchAll("Error auditing contact deletion.");

		//Update Outlook.
		PPCModifyContact(nContactID);

		SetTabStates(true);

		CClient::RefreshTable(NetUtils::CustomContacts, nContactID);

		int pos = m_contactToolBar.m_toolBarCombo->GetCurSel();
		if (pos)//keeps from getting error by deleting last patient
			pos--;
		m_contactToolBar.m_toolBarCombo->RemoveRow(m_contactToolBar.m_toolBarCombo->GetCurSel());
		m_contactToolBar.m_toolBarCombo->PutCurSel(pos);
		m_contactToolBar.m_ActiveContact = m_contactToolBar.m_toolBarCombo->GetValue(pos,0).lVal;

		//the contact bar does not have a static control for this
		//	m_contactToolBar.m_text3.SetWindowText(str);
		UpdateAllViews();


		//we need to check to see if we're deleting the last contact of this filter
		if(m_contactToolBar.m_toolBarCombo->GetRowCount() == 0)
		{
			AfxMessageBox("You have deleted the last contact of this type, resetting the filter to all contacts");
			//reset the filter
			m_contactToolBar.m_all.SetCheck(true);
			m_contactToolBar.m_other.SetCheck(false);
			m_contactToolBar.m_referring.SetCheck(false);
			m_contactToolBar.m_supplier.SetCheck(false);
			m_contactToolBar.m_employee.SetCheck(false);
			m_contactToolBar.m_main.SetCheck(false);
			OnAllContactsSearchClicked();

			UpdateAllViews();

		}
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2008-07-02 13:56) - PLID 29757 - Handle the licensed views holder
#define UTBB_ADD_TO_ENABLED_VIEWS(button_bit_pos, licensed, allow_show)		if (licensed) { m_licensedViews |= button_bit_pos; } if (licensed && allow_show) { m_initialEnabledViews |= button_bit_pos; }

void CMainFrame::UpdateToolBarButtons(bool requery/*=false*/)
{
	// If requested, fill the defaults based on license and security settings
	if (requery) {
		// (a.walling 2008-07-02 13:56) - PLID 29757 - Create a seperate bitmask for views that are licensed
		m_licensedViews = 0;
		m_initialEnabledViews = 0;
		//DRT 11/24/2004 - PLID 14779 - See notes in PatientView::ShowTabs.  They are now able to get into
		//	the patients module if they have 'Patients' or 'Billing' selected in the license.
		// (a.walling 2009-06-11 13:43) - PLID 34601 - This was not properly disabling itself; I wrapped it in parenthesis to fix the macro expansion
		UTBB_ADD_TO_ENABLED_VIEWS(TB_PATIENT,		
			(g_pLicense->CheckForLicense(CLicense::lcPatient, CLicense::cflrSilent) || g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent) || g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)),
			CanCurrentUserViewObject(bioPatientsModule));
		UTBB_ADD_TO_ENABLED_VIEWS(TB_SCHEDULE,	
			//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
			g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent),
			CanCurrentUserViewObject(bioSchedulerModule));
		UTBB_ADD_TO_ENABLED_VIEWS(TB_DOCUMENT,		
			g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrSilent),
			CanCurrentUserViewObject(bioLetterWritingModule));
		UTBB_ADD_TO_ENABLED_VIEWS(TB_CONTACT,		
			TRUE,
			CanCurrentUserViewObject(bioContactsModule));
		//PLID 16078 added Retention only licensing support
		// (a.walling 2009-06-11 13:43) - PLID 34601 - This was not properly disabling itself; I wrapped it in parenthesis to fix the macro expansion
		UTBB_ADD_TO_ENABLED_VIEWS(TB_MARKET,		
			(g_pLicense->CheckForLicense(CLicense::lcMarket, CLicense::cflrSilent) || g_pLicense->CheckForLicense(CLicense::lcRetention, CLicense::cflrSilent)),
			CanCurrentUserViewObject(bioMarketingModule));
		UTBB_ADD_TO_ENABLED_VIEWS(TB_INVENTORY,		
			g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent),
			CanCurrentUserViewObject(bioInventoryModule));
		UTBB_ADD_TO_ENABLED_VIEWS(TB_FINANCIAL,		
			// (j.jones 2010-06-23 10:33) - PLID 39297 - check all possible licenses that show tabs in the financial module
			// (d.thompson 2010-09-02) - PLID 40371 - Any cc licensing satisfies
			(g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent) || g_pLicense->CheckForLicense(CLicense::lcHCFA, CLicense::cflrSilent)
			|| g_pLicense->CheckForLicense(CLicense::lcEbill, CLicense::cflrSilent) || g_pLicense->CheckForLicense(CLicense::lcEEligibility, CLicense::cflrSilent)
			|| g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent)),
			CanCurrentUserViewObject(bioFinancialModule));
		UTBB_ADD_TO_ENABLED_VIEWS(TB_REPORTS,		
			TRUE,
			CanCurrentUserViewObject(bioReportsModule));
		UTBB_ADD_TO_ENABLED_VIEWS(TB_ADMIN,			
			TRUE,
			CanCurrentUserViewObject(bioAdministratorModule));
		UTBB_ADD_TO_ENABLED_VIEWS(TB_NEWPATIENT,	
			TRUE,
			CheckCurrentUserPermissions(bioPatient, sptCreate, FALSE, 0, TRUE, TRUE));
		UTBB_ADD_TO_ENABLED_VIEWS(TB_ASC,
			g_pLicense->CheckForLicense(CLicense::lcSurgeryCenter, CLicense::cflrSilent),
			CanCurrentUserViewObject(bioASCModule));
		// (j.jones 2007-05-16 08:44) - PLID 25431 - you can't create an EMR without Create and Write permissions
		// (a.walling 2008-04-21 15:58) - PLID 29736 - They just need a license to be able to view the NexEMR tab.
		// (z.manning 2015-03-13 12:13) - NX-100432 - This button can now open the dashboard tab too, however,
		// it requires EMR read permission too so no need to change this.
		UTBB_ADD_TO_ENABLED_VIEWS(TB_EMR,			
			g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2,
			(CanCurrentUserViewObject(bioPatientsModule) && CheckCurrentUserPermissions(bioPatientEMR, sptRead, FALSE, 0, TRUE, TRUE)));
		UTBB_ADD_TO_ENABLED_VIEWS(TB_FFA,
			//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
			g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent),
			CanCurrentUserViewObject(bioSchedulerModule));
		// (z.manning, 01/10/2007) - PLID 24057 - Only disable the CareCredit button if it's expired. Even if they
		// aren't licensed for it, we want them to be able to click on it to see the marketing message.
		// (a.walling 2009-06-11 13:39) - PLID 34601 - If we want to disable it for those with no permission, use
		// CheckCurrentUserPermissions(bioCareCreditIntegration, sptView, FALSE, 0, TRUE, TRUE)
		UTBB_ADD_TO_ENABLED_VIEWS(TB_CARECREDIT,
			(NxCareCredit::GetCareCreditLicenseStatus() != cclsExpired),
			TRUE);
		// (d.thompson 2009-11-16) - PLID 36134 - Links module
		UTBB_ADD_TO_ENABLED_VIEWS(TB_LINKS,		
			TRUE,
			CanCurrentUserViewObject(bio3rdPartyLinks));
	}

	{
		// (a.walling 2010-11-26 13:08) - PLID 40444 - Update the module's availability
		PracticeModuleMap& modules(g_Modules.GetModules());

		for (PracticeModuleMap::iterator it = modules.begin(); it != modules.end(); it++) {
			PracticeModule& module(*it->second);

			WORD module_bit_pos = 0;

			switch (module.GetType()) {
				case Modules::Scheduler:
					module_bit_pos = TB_SCHEDULE;
					break;
				case Modules::Patients:
					module_bit_pos = TB_PATIENT;
					break;
				case Modules::Admin:
					module_bit_pos = TB_ADMIN;
					break;
				case Modules::Inventory:
					module_bit_pos = TB_INVENTORY;
					break;
				case Modules::Financial:
					module_bit_pos = TB_FINANCIAL;
					break;
				case Modules::Marketing:
					module_bit_pos = TB_MARKET;
					break;
				case Modules::Reports:
					module_bit_pos = TB_REPORTS;
					break;
				case Modules::Contacts:
					module_bit_pos = TB_CONTACT;
					break;
				case Modules::LetterWriting:
					module_bit_pos = TB_DOCUMENT;
					break;
				case Modules::SurgeryCenter:
					module_bit_pos = TB_ASC;
					break;
				case Modules::Links:
					module_bit_pos = TB_LINKS;
					break;
			}

			if (module_bit_pos != 0) {
				module.Enable((m_initialEnabledViews & module_bit_pos) ? true : false); 
				module.Show((m_licensedViews & module_bit_pos) ? true : false);
			}
		}
	}

	// Copy the defaults
	m_enableViews = m_initialEnabledViews;
	
	// Remove from the defaults the button corresponding to the active view
	CChildFrame *pCurFrame = GetActiveViewFrame();
	if (pCurFrame) {
		CString strView = pCurFrame->GetType();

		if (strView == PATIENT_MODULE_NAME) {
			// (a.walling 2008-05-07 13:57) - PLID 29951 - If the NexEMRTab is the active tab, then
			// set that button to 'disabled' so it is highlighted, and leave Patients normal.
			CView* pActiveView = GetActiveView();
			CNxTabView* pPatientView = NULL;
			if(pActiveView) {
				// (a.walling 2008-10-23 17:39) - PLID 31821 - This can crash; ensure the view is actually an CNxTabView
				pPatientView = pActiveView->IsKindOf(RUNTIME_CLASS(CNxTabView)) ? (CNxTabView*)pActiveView : NULL;
			}

			if (pPatientView != NULL)
			{
				// (z.manning 2015-03-13 12:31) - NX-100432 - The NexEMR icon may open the dashboard tab so account for that here
				if (pPatientView->GetActiveTab() == GetPrimaryEmrTab()) {
					m_enableViews &= ~TB_EMR;
				}
				else {
					m_enableViews &= ~TB_PATIENT;
				}
			}
		}
		else if (strView == SCHEDULER_MODULE_NAME)			m_enableViews &= ~TB_SCHEDULE;
		else if (strView == LETTER_MODULE_NAME)				m_enableViews &= ~TB_DOCUMENT;
		else if (strView == CONTACT_MODULE_NAME)			m_enableViews &= ~TB_CONTACT;
		else if (strView == MARKET_MODULE_NAME)				m_enableViews &= ~TB_MARKET;
		else if (strView == INVENTORY_MODULE_NAME)			m_enableViews &= ~TB_INVENTORY;
		else if (strView == FINANCIAL_MODULE_NAME)			m_enableViews &= ~TB_FINANCIAL;
		else if (strView == REPORT_MODULE_NAME)				m_enableViews &= ~TB_REPORTS;
		else if (strView == ADMIN_MODULE_NAME)				m_enableViews &= ~TB_ADMIN;
		else if (strView == SURGERY_CENTER_MODULE_NAME)		m_enableViews &= ~TB_ASC;
		// (d.thompson 2009-11-16) - PLID 36134
		else if (strView == LINKS_MODULE_NAME)				m_enableViews &= ~TB_LINKS;
	}

	// Now actually apply the settings on screen
	CToolBarCtrl *pbar = &m_wndToolBar.GetToolBarCtrl();
	if (pbar->GetSafeHwnd()) {
		pbar->EnableButton(ID_PATIENTS_MODULE,			(m_enableViews & TB_PATIENT)	? TRUE : FALSE);
		pbar->EnableButton(ID_SCHEDULER_MODULE,			(m_enableViews & TB_SCHEDULE)	? TRUE : FALSE);
		pbar->EnableButton(ID_LETTER_WRITING_MODULE,	(m_enableViews & TB_DOCUMENT)	? TRUE : FALSE);
		pbar->EnableButton(ID_CONTACTS_MODULE,			(m_enableViews & TB_CONTACT)	? TRUE : FALSE);
		pbar->EnableButton(ID_MARKETING_MODULE,			(m_enableViews & TB_MARKET)		? TRUE : FALSE);
		pbar->EnableButton(ID_INVENTORY_MODULE,			(m_enableViews & TB_INVENTORY)	? TRUE : FALSE);
		pbar->EnableButton(ID_FINANCIAL_MODULE,			(m_enableViews & TB_FINANCIAL)	? TRUE : FALSE);
		pbar->EnableButton(ID_REPORTS_MODULE,			(m_enableViews & TB_REPORTS)	? TRUE : FALSE);
		pbar->EnableButton(ID_ADMINISTRATOR_MODULE,		(m_enableViews & TB_ADMIN)		? TRUE : FALSE);
		pbar->EnableButton(ID_NEW_PATIENT_BTN,			(m_enableViews & TB_NEWPATIENT)	? TRUE : FALSE);
		//pbar->EnableButton(ID_PALM_PILOT,				m_bIncomingCall					? TRUE : FALSE);
		pbar->EnableButton(ID_SURGERYCENTER_MODULE,		(m_enableViews & TB_ASC)		? TRUE : FALSE);
		pbar->EnableButton(ID_TB_NEW_EMR,				(m_enableViews & TB_EMR)		? TRUE : FALSE);
		pbar->EnableButton(ID_FIRST_AVAILABLE_APPT,		(m_enableViews & TB_FFA)		? TRUE : FALSE);
		pbar->EnableButton(ID_CARECREDIT_BTN,			(m_enableViews & TB_CARECREDIT)	? TRUE : FALSE);
		// (d.thompson 2009-11-16) - PLID 36134
		pbar->EnableButton(ID_LINKS_MODULE,				(m_enableViews & TB_LINKS)		? TRUE : FALSE);
	}
}

// (j.jones 2008-12-30 12:24) - PLID 32585 - previously this was only called from CPracticeApp::OnIdle,
// but now is also called when either MainFrm or a NxDialog receives the WM_KICKIDLE message
BOOL CMainFrame::Idle (int time)
{
	switch (time)
	{	case 0:
			UpdateToolBarButtons();

			// (j.jones 2008-12-23 13:46) - PLID 32545 - process the next entry in our barcode queue when Idle			
			PumpNextQueuedBarcode();
			return TRUE;
			break;
	}

	return FALSE;
}

// (j.jones 2008-12-23 13:46) - PLID 32545 - this should be called when Idle() to post messages from our queue,
// and Idle() should be called from the WinApp's OnIdle(), or from a WM_KICKIDLE message on modal dialogs
void CMainFrame::PumpNextQueuedBarcode()
{
	try {

		// (j.jones 2008-12-29 15:22) - If you are coding barcode features and wonder why this code is never hit,
		// make sure your dialog is an NxDialog, which handles WM_KICKIDLE. Otherwise, your dialog will need to
		// handle WM_KICKIDLE, and call GetMainFrame()->Idle((int)lParam) in the OnKickIdle handler.
		// The WinApp's OnIdle() function is never hit when a modal dialog is open, but is hit for modeless dialogs,
		// and will also call the Idle() function, which then calls this function.

		//get the next barcode if one exists
		if(m_arystrQueuedBarcodes.GetSize() > 0) {

			TRACE("CMainFrame::PumpNextQueuedBarcode is sending the next barcode now.\n");
			
			CString strBarcode = m_arystrQueuedBarcodes.GetAt(0);
			
			//remove the code from the queue
			m_arystrQueuedBarcodes.RemoveAt(0);
			
			//now post the code using WM_BARCODE_SCAN
			BSTR bstr = strBarcode.AllocSysString();
			PostMessage(WM_BARCODE_SCAN, strBarcode.GetLength(), (LPARAM)bstr);
		}

	}NxCatchAll("Error in CMainFrame::PumpNextQueuedBarcode");
}

// (j.jones 2009-02-10 12:12) - PLID 32870 - added HasQueuedBarcodes, which will simply tell is if
// m_arystrQueuedBarcodes has any values in it
BOOL CMainFrame::HasQueuedBarcodes()
{
	try {

		return (m_arystrQueuedBarcodes.GetSize() > 0);

	}NxCatchAll("Error in CMainFrame::HasQueuedBarcodes");

	return FALSE;
}

void CMainFrame::OnInform() 
{
	if (UserPermission(InformIntegration))
	{	CInformLink dlg;
		dlg.DoModal();
	}
}

//TES 8/24/2004: These buttons don't exist any more.
/*void CMainFrame::OnNextDocBtn() 
{
	//DRT 2/4/2004 - PLID 9451 - Fixed a bug that was allowing you to keep clicking on the left arrow indefinitely 
	//	because the update_command_ui was not being fired off until a bunch of other code ran around.
	if(m_bMovingDocBar)
		return;

	SendMessage(CN_UPDATE_COMMAND_UI, ID_PREV_DOC_BTN, 0);

	m_bMovingDocBar = true;
	m_pDocToolBar->ScrollDoc(1);
	
	CNxTabView *pActiveView = GetActiveView();
	if (pActiveView) {
		pActiveView->UpdateView();
	}
	m_bMovingDocBar = false;
}

void CMainFrame::OnPrevDocBtn() 
{
	//DRT 2/4/2004 - PLID 9451 - Fixed a bug that was allowing you to keep clicking on the left arrow indefinitely 
	//	because the update_command_ui was not being fired off until a bunch of other code ran around.
	if(m_bMovingDocBar)
		return;

	PostMessage(CN_UPDATE_COMMAND_UI, ID_NEXT_DOC_BTN, 0);

	m_bMovingDocBar = true;
	m_pDocToolBar->ScrollDoc(-1);
	CNxTabView *pActiveView = GetActiveView();
	if (pActiveView) {
		pActiveView->UpdateView();
	}
	m_bMovingDocBar = false;
}

void CMainFrame::OnUpdateNextDocBtn(CCmdUI* pCmdUI) 
{
	if (m_pDocToolBar && m_pDocToolBar->GetSafeHwnd()) 
	{
		bool enable = (m_pDocToolBar->GetDocCount() > m_pDocToolBar->GetDocIndex() + 1);
		pCmdUI->Enable(enable);
	}
}*/

static void PrepareLookup(CString &value, const CString &field)
{
	if (value == "")
		value = "\"\" OR " + field + " Is NULL";
	else
	{	value.Insert(0, '\"');
		value += '\"';
	}
}

// Get nFilterId from the list in "FilterDetail.cpp"
void CMainFrame::LookupSimple(long nFilterId, FieldOperatorEnum nOperator, const CString &strInitValue)
{
	CString strFilter;
	if (nFilterId > 0) {
		strFilter.Format("{%li, %li, \"%s\"}", nFilterId, nOperator, strInitValue);
	}

	CFilterEditDlg dlg(NULL, fboPerson, CGroups::IsActionSupported, CGroups::CommitSubfilterAction);
	// (z.manning 2009-06-02 15:14) - PLID 34430 - We need to audit when doing a lookup by patient's
	// last name or birthday, so set a flag for the filter dialog to do so.
	dlg.m_bAuditForLookup = TRUE;
	if (dlg.EditFilter(FILTER_ID_TEMPORARY, strFilter) == IDOK) {
		m_patToolBar.m_strFilterFrom = dlg.m_strSqlFrom;
		m_patToolBar.m_strFilterWhere = dlg.m_strSqlWhere;
		m_patToolBar.m_strFilterString = dlg.m_strFilterString;
		// TODO: We used to run the sql and get the number of records returned, 
		// and IF there were none, we would call ResetFilter so as to put 
		// everything into the selection, but I think it should do so 
		// automatically now.  So if it does not do it automatically, we need to 
		// remember to do it here.
		m_patToolBar.m_butFilter.SetCheck(true);
		OnFilterSearchClicked();
	}
}

void CMainFrame::Lookup(int nID)
{
	// Lookup options
	switch (nID) {
		case ID_LOOKUP_CITY:
			LookupSimple(84, foBeginsWith, "");
			break;
		case ID_LOOKUP_STATE:
			LookupSimple(85, foBeginsWith, "");
			break;
		case ID_LOOKUP_ZIP:
			LookupSimple(86, foBeginsWith, "");
			break;
		case ID_LOOKUP_HOMEPHONE:
			LookupSimple(88, foBeginsWith, "");
			break;
		case ID_LOOKUP_WORKPHONE:
			LookupSimple(89, foBeginsWith, "");
			break;
		case ID_LOOKUP_GENERAL2_PATIENTTYPETEXT:
			LookupSimple(99, foBeginsWith, "");
			break;
		case ID_LOOKUP_GENERAL2_PATIENTTYPE:
			LookupSimple(98, foEqual, "");
			break;
		case ID_LOOKUP_HAS_PHOTO_FOR_PROCEDURE:
			LookupSimple(371, foEqual, "");
			break;
		case ID_LOOKUP_CUSTOM1:
			LookupSimple(101, foBeginsWith, "");
			break;
		case ID_LOOKUP_CUSTOM2:
			LookupSimple(102, foBeginsWith, "");
			break;
		case ID_LOOKUP_CUSTOM3:
			LookupSimple(103, foBeginsWith, "");
			break;
		case ID_LOOKUP_CUSTOM4:
			LookupSimple(104, foBeginsWith, "");
			break;
		case ID_LOOKUP_MAIN_PROVIDER:
			LookupSimple(159, foEqual, "");
			break;
		case ID_LOOKUP_FIRST_CONTACT:
			LookupSimple(156, foGreaterEqual, "");
			break;
		case ID_LOOKUP_REFERRALSOURCE:
			LookupSimple(97, foBeginsWith, "");
			break;
		case ID_LOOKUP_BLANKFILTER:
			LookupSimple(0, foEqual, "");
			break;
		case ID_LOOKUP_GENERAL2_OCCUPATION:
			LookupSimple(95, foBeginsWith, "");
			break;
		case ID_LOOKUP_GENERAL2_COMPANYSCHOOL:
			LookupSimple(66, foBeginsWith, "");
			break;
		case ID_LOOKUP_GENERAL2_DEFAULTREFERRINGPHYSICAN:
			LookupSimple(106, foEqual, "");
			break;
		case ID_LOOKUP_GENERAL2_LOCATION:
			LookupSimple(174, foEqual, "");
			break;
		case ID_LOOKUP_GENERAL2_EMPLOYMENT:
			LookupSimple(173, foEqual, "");
			break;
		case ID_LOOKUP_GENERAL2_REFFERINGPATIENT:
			LookupSimple(192, foEqual, "");
			break;
		case ID_LOOKUP_EXISTINGGROUP:
			LookupSimple(FILTER_FIELD_SPECIAL_GROUP, foIn, "");
			break;
		case ID_LOOKUP_TODAY:
			{
				CString strFilter = "{277, 2048, \"0-{278, 1, \"\"Today\"\"}\"}";
				CString strFrom, strWhere;
				if(CFilter::ConvertFilterStringToClause(-1, strFilter, fboPerson, &strWhere, &strFrom)) {
					m_patToolBar.m_strFilterFrom = strFrom;
					m_patToolBar.m_strFilterWhere = strWhere;
					// TODO: We used to run the sql and get the number of records returned, 
					// and IF there were none, we would call ResetFilter so as to put 
					// everything into the selection, but I think it should do so 
					// automatically now.  So if it does not do it automatically, we need to 
					// remember to do it here.
					m_patToolBar.m_butFilter.SetCheck(true);
					OnFilterSearchClicked();
				}
			}
			break;
		case ID_LOOKUP_CURRENT:
			{
				CFilterEditDlg dlg(NULL, fboPerson, CGroups::IsActionSupported, CGroups::CommitSubfilterAction);
				CString strFilter = m_patToolBar.m_strFilterString;
				// (z.manning 2009-06-17 11:33) - PLID 34430 - We need to audit when doing a lookup by patient's
				// last name or birthday, so set a flag for the filter dialog to do so.
				dlg.m_bAuditForLookup = TRUE;
				if (dlg.EditFilter(FILTER_ID_TEMPORARY, strFilter) == IDOK) {
					m_patToolBar.m_strFilterFrom = dlg.m_strSqlFrom;
					m_patToolBar.m_strFilterWhere = dlg.m_strSqlWhere;
					m_patToolBar.m_strFilterString = dlg.m_strFilterString;
					// TODO: We used to run the sql and get the number of records returned, 
					// and IF there were none, we would call ResetFilter so as to put 
					// everything into the selection, but I think it should do so 
					// automatically now.  So if it does not do it automatically, we need to 
					// remember to do it here.
					m_patToolBar.m_butFilter.SetCheck(true);
					OnFilterSearchClicked();
				}
			}
			break;
		default:
			ASSERT(FALSE);
	}
}

long CMainFrame::GetOpenModuleCount()
{
	return m_pView.GetSize();
}

//TES 8/24/2004: These buttons don't exist any more.
/*void CMainFrame::OnUpdatePrevDocBtn(CCmdUI* pCmdUI) 
{
	if (m_pDocToolBar && m_pDocToolBar->GetSafeHwnd()) 
	{
		bool enable = (m_pDocToolBar->GetDocCount() && m_pDocToolBar->GetDocIndex());
		pCmdUI->Enable(enable);
	}
}*/

void CMainFrame::OnPracYakkerSend()
{
	//CSendMessageDlg dlg;
	//dlg.m_pMessageHome = &m_dlgMessager;
	//dlg.DoModal();

	//TES 12/19/2008 - PLID 32525 - They can't use the PracYakker if they're on Scheduler Standard
	// (v.maida 2014-12-27 10:19) - PLID 27381 - Check that the user has PracYakker permission.
	if (CheckCurrentUserPermissions(bioPracYakker, sptView)) {
		if (g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "PracYakker intra-office messaging", "PracYakker/pracyakker.htm")) {
			ASSERT(m_pdlgSendYakMessage != NULL);
			if (m_pdlgSendYakMessage)
				m_pdlgSendYakMessage->PopUpMessage();
		}
	}
}

void CMainFrame::OnHelpLicenseAgreement()
{
	CLicenseDlg dlg(this);
	dlg.m_bRequireAgreement = FALSE;
	dlg.DoModal();
}

// (a.pietrafesa 2015-06-22) - PLID 65910 - View Log File in Help menu
void CMainFrame::OnViewLogFile()
{
	try {
		SHELLEXECUTEINFO sei;
		memset(&sei, 0, sizeof(SHELLEXECUTEINFO));

		CString strParameter;
		strParameter.Format("\"%s\"", GetLogFilePathName());

		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.hwnd = (HWND)GetDesktopWindow();
		sei.lpFile = "notepad.exe";
		sei.lpParameters = strParameter;
		sei.nShow = SW_SHOW;
		sei.hInstApp = NULL;

		ShellExecuteEx(&sei);
	}
	NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnActivitiesUpdateInventoryPrices()
{
	if(!CheckCurrentUserPermissions(bioInventoryModule, sptView))
		return;

	CUpdatePriceDlg dlg(this);
	dlg.nType = 2;	//inventory items
	if(dlg.DoModal() == IDOK && GetActiveView()) {
		GetActiveView()->UpdateView();
	}
}

// (j.jones 2010-01-11 14:19) - PLID 26786 - added ability to mass-update product information
void CMainFrame::OnUpdateInformationForMultipleProducts()
{
	try {

		//this dialog serves no purpose but to update products,
		//so check now to see if they have permission to do so
		if(!CheckCurrentUserPermissions(bioInvItem, sptWrite)) {
			return;
		}

		CUpdateProductsDlg dlg(this);
		dlg.DoModal();

		//they may have updated the current product
		if(GetActiveView()) {
			GetActiveView()->UpdateView();
		}

	}NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnActivitiesPurchaseItemsFromConsignment()
{
	// (c.haag 2007-11-29 10:41) - PLID 28006 - Let the user purchase items
	// in consignment into regular inventory
	try {
		if(!CheckCurrentUserPermissions(bioInvItem, sptDynamic1)) {
			return;
		}
		CInvConsignmentPurchaseDlg dlg(this);
		dlg.DoModal();
	}
	NxCatchAll("Error in CMainFrame::OnActivitiesPurchaseItemsFromConsignment");
}

void CMainFrame::OnActivitiesTransferItemsToConsignment()
{
	// (c.haag 2007-11-29 10:41) - PLID 28050 - Let the user transfer items
	// from regular inventory to consignment
	try {
		if(!CheckCurrentUserPermissions(bioInvItem, sptDynamic1)) {
			return;
		}
		CInvConsignmentAssignDlg dlg(this);
		dlg.DoModal();
	}
	NxCatchAll("Error in CMainFrame::OnActivitiesTransferItemsToConsignment");
}
//DRT 10/25/02
void CMainFrame::OnToolsLogtimePrompt()
{
	//TES 12/18/2008 - PLID 32520 - This feature is blocked to Scheduler Standard users
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "User time logs")) {
		return;
	}

	int nCheck;
	if(GetRemotePropertyInt("LogTimePrompt", 0, 0, GetCurrentUserName(), true))
		nCheck = 1;
	else
		nCheck = 0;

	//if they are disabling the prompt, give them a "last chance" warning to let it prompt them at the end of this session
	if(nCheck) {	//we are turning it off
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM UserTimesT WHERE UserID = {INT} AND Checkout IS NULL", GetCurrentUserID());
		if(!rs->eof) {	//they are disabling the prompt, but there is an open item
			int res = AfxMessageBox("There is still an open time log for this user.  Would you like to prompt them one more time at the end of their session to close it?  "
						"Selecting 'No' will not prompt the user when they log out.  Selecting 'Yes' will prompt the user one time to end their logging.", MB_YESNO);
			if(res == IDYES) {
				SetRemotePropertyInt("LogTimePromptOneLastTime", 1, 0, GetCurrentUserName());
			}
		}
	}

	//set it to whatever it isn't
	SetRemotePropertyInt("LogTimePrompt", !nCheck, 0, GetCurrentUserName());
}

//DRT 1/21/03
void CMainFrame::OnToolsLogtimeLogManagement()
{
	//TES 12/18/2008 - PLID 32520 - This feature is blocked to Scheduler Standard users
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "User time logs")) {
		return;
	}

	//open the management dialog to the current user

	//if they have the "need password" permission set, we need to ask them for the password first
	if (!CheckCurrentUserPermissions(bioLogManage, sptWrite))
		return;

	CLogManagementDlg dlg(this);
	dlg.m_nUserID = GetCurrentUserID();
	dlg.DoModal();
}

//DRT 2/13/03 - Allow them to apply superbills to any charges
void CMainFrame::OnActivitiesApplySuperbills()
{
	// (e.lally 2009-06-10) PLID 34529 - Added try/catch
	try {
		if (!CheckCurrentUserPermissions(bioApplySuperbills, sptWrite))
			return;

		CAppliedSuperbillsDlg dlg(this);
		dlg.m_nCurrentPatientID = GetActivePatientID();
		dlg.m_nCurrentBillID = -1;
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

//DRT 2/13/03 - Allow them to Void superbills
void CMainFrame::OnActivitiesVoidSuperbills()
{
	// (e.lally 2009-06-10) PLID 34529 - added try/catch
	try {
		if (CheckCurrentUserPermissions(bioVoidSuperbills, sptWrite)) {
			CVoidSuperbillDlg dlg(this);
			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnActivitiesEyeGraph()
{
	if(IsRefractive()) {
		//open eye graph dialog
		CEyeGraphDlg dlg(this);
		dlg.DoModal();
	}
}

void CMainFrame::OnExportToMedinotes()
{
	try {
		// (e.lally 2009-06-10) PLID 34529 - Check permissions - View patients module
		if(CheckCurrentUserPermissions(bioPatientsModule, sptView)) {
			CMediNotesLink dlg(this);
			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);
}

extern BOOL g_bIgnoreRightAlt;
extern BOOL g_bIsRightAltStatusSet;

void CMainFrame::OnShowPreferences()
{
	try {
		//TES 6/23/2009 - PLID 34155 - Changed the View permission to Read
		if(CheckCurrentUserPermissions(bioPreferences,sptRead)) {
			CNxTabView* pActiveView = GetActiveView();
			long nPatToolbarColPlacements[] = 
			{
				GetRemotePropertyInt(CString("PtTB_Last Name"), 1, 0, GetCurrentUserName(), true),
				GetRemotePropertyInt(CString("PtTB_First Name"), 2, 0, GetCurrentUserName(), true),
				GetRemotePropertyInt(CString("PtTB_Middle Name"), 3, 0, GetCurrentUserName(), true),
				GetRemotePropertyInt(CString("PtTB_Patient ID"), 4, 0, GetCurrentUserName(), true),
				GetRemotePropertyInt(CString("PtTB_Birth Date"), 5, 0, GetCurrentUserName(), true),
				GetRemotePropertyInt(CString("PtTB_SSN"), 6, 0, GetCurrentUserName(), true),
				GetRemotePropertyInt(CString("PtTB_Company or School"), -7, 0, GetCurrentUserName(), true),
				// (j.jones 2010-05-04 10:55) - PLID 32325 - added OHIP Health Card Column
				GetRemotePropertyInt(CString("PtTB_OHIP_Health_Card"), 8, 0, GetCurrentUserName(), true),
				// (j.gruber 2010-10-04 14:27) - PLID 40415
				GetRemotePropertyInt(CString("PtTB_Security_Group"), -9, 0, GetCurrentUserName(), true),				
			};

			// (j.jones 2010-08-18 16:10) - PLID 32325 - cache the OHIP preference
			// (j.jones 2010-11-08 13:53) - PLID 39620 - same for Alberta
			BOOL bWasOHIPOrAlberta = UseOHIP() || UseAlbertaHLINK();

			int nReturn;
			if (pActiveView) nReturn = pActiveView->ShowPrefsDlg();
			else nReturn = ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase());

			if(nReturn == IDOK) {
				// (c.haag 2005-01-26 14:12) - PLID 15415 - Flush the remote property cache because one or
				// more preferences may have changed, which means ConfigRT changed without intervention from
				// the cache.
				// (c.haag 2005-11-07 16:27) - PLID 16595 - This is now obselete; but we still flush in regular intervals
				//FlushRemotePropertyCache();

				// (j.fouts 2012-06-06 11:01) - PLID 49611 - Let Patients Module know that preferences have changes
				CNxTabView* pPatientsView = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
				// (j.fouts 2012-06-08 16:26) - PLID 49611 - If we have not loaded the module yet no need to update it
				if(pPatientsView != NULL)
				{
					pPatientsView->SendMessage(WM_PREFERENCE_UPDATED);
				}

				// (a.walling 2010-08-19 08:19) - PLID 17768 - Notify the patient toolbar
				m_patToolBar.OnPreferencesChanged();

				// (c.haag 2006-05-16 12:49) - PLID 20621 - Make sure the yakker colors are up to date
				ASSERT(m_pdlgMessager != NULL);
				if (m_pdlgMessager) {
					m_pdlgMessager->RefreshColors();
					// (b.cardillo 2006-07-05 15:53) - PLID 20948 - In case the icons have changed for this user, refresh them.
					m_pdlgMessager->RefreshIcons();
				}

				ASSERT(m_pdlgSendYakMessage != NULL);
				if (m_pdlgSendYakMessage)
					m_pdlgSendYakMessage->RefreshColors();

				long nNewPatToolbarColPlacements[] = 
				{
					GetRemotePropertyInt(CString("PtTB_Last Name"), 1, 0, GetCurrentUserName(), true),
					GetRemotePropertyInt(CString("PtTB_First Name"), 2, 0, GetCurrentUserName(), true),
					GetRemotePropertyInt(CString("PtTB_Middle Name"), 3, 0, GetCurrentUserName(), true),
					GetRemotePropertyInt(CString("PtTB_Patient ID"), 4, 0, GetCurrentUserName(), true),
					GetRemotePropertyInt(CString("PtTB_Birth Date"), 5, 0, GetCurrentUserName(), true),
					GetRemotePropertyInt(CString("PtTB_SSN"), 6, 0, GetCurrentUserName(), true),
					//m.hancock - 5/8/2006 - PLID 20462 - Add the company name as an option to the new patient toolbar
					GetRemotePropertyInt(CString("PtTB_Company or School"), -7, 0, GetCurrentUserName(), true),
					// (j.jones 2010-05-04 10:55) - PLID 32325 - added OHIP Health Card Column
					GetRemotePropertyInt(CString("PtTB_OHIP_Health_Card"), 8, 0, GetCurrentUserName(), true),
					// (j.gruber 2010-10-04 14:27) - PLID 40415
					GetRemotePropertyInt(CString("PtTB_Security_Group"), -9, 0, GetCurrentUserName(), true),
				};

				// (j.jones 2010-08-18 16:10) - PLID 32325 - if OHIP preference changed, we need to rebuild
				// the patient combo, just like we do if the patient combo columns changed?
				// (j.jones 2010-11-08 13:53) - PLID 39620 - same for Alberta
				BOOL bHasOHIPOrAlberta = UseOHIP() || UseAlbertaHLINK();
				if (memcmp(nPatToolbarColPlacements, nNewPatToolbarColPlacements,
					sizeof(nNewPatToolbarColPlacements))
					|| bWasOHIPOrAlberta != bHasOHIPOrAlberta)
				{
					m_patToolBar.PopulateColumns();
					m_patToolBar.Requery();
				}
				SetShowAreaCode(GetRemotePropertyInt("UseAreaCode", 1, 0, "<None>", true) == 0 ? false : true);//This may have changed in ConfigRT.

				UpdateAllViews();

				// (j.jones 2005-05-02 16:21) - PLID 16411 - reload the IgnoreRightAlt status
				g_bIgnoreRightAlt = GetRemotePropertyInt("IgnoreRightAlt", 0, 0, "<None>", true);
				g_bIsRightAltStatusSet = TRUE;

				// CAH 5/16/03 - There is one preference that affects the query used in
				// the patient dropdown in the ResEntryDlg. To be safe, we need to update
				// that dropdown.

				// (m.cable 2004-06-02 11:40) - There is also a preference that affects the query
				// used in the Type dropdown in the ResEntryDlg.

				// This probably won't be the only case where we will have to do this in
				// the future. Maybe we should make a NXM_ONPREFERENCESCHANGED message
				// to broadcast to all the open views....perhaps the CNxTabView class could
				// have an overrideable function that calls UpdateView() by default upon
				// reception of the message.
				CSchedulerView* pView = (CSchedulerView*)GetOpenView(SCHEDULER_MODULE_NAME);
				if (pView && pView->GetResEntry()){
					pView->GetResEntry()->RequeryPersons();
					pView->GetResEntry()->RequeryAptTypes();
				}

				// (j.jones 2010-08-27 10:20) - PLID 36975 - if the room manager is open,
				// tell it that the patient name preference could have changed (it caches
				// this preference in the dialog already so we do not need to)
				if(m_pRoomManagerDlg && m_pRoomManagerDlg->GetSafeHwnd()) {
					m_pRoomManagerDlg->OnPreferencesChanged();
				}

				// (j.jones 2011-03-11 14:41) - PLID 42328 - tell the device import if preferences changed
				if(DeviceImport::GetMonitor().GetImportDlg()->GetSafeHwnd()) {
					DeviceImport::GetMonitor().GetImportDlg()->OnPreferencesChanged();
				}

				// (z.manning 2016-05-09 9:25) - NX-100500 - Make sure the Word processor type is up to date
				// (z.manning 2016-05-26 15:20) - NX-100500 - The Word preference is now hidden so this is unnecessary
				//GetWPManager()->SetWordProcessorType(GetWordProcessorType());
			}

		}
	}NxCatchAll("Error displaying preferences.");
}

void CMainFrame::OnSearchNotes()
{
	try {
		// (e.lally 2009-06-08) PLID 34497 - Permission - Read Patient Notes
		if(CheckCurrentUserPermissions(bioPatientNotes, sptRead)) {
			CSearchNotesDlg dlg(this);
			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnSearchAllowables()
{
	try {
		// (e.lally 2009-06-08) PLID 34497 - Permissions - view Billing Tab
		if(CheckCurrentUserPermissions(bioPatientBilling, sptRead)) {
			// (j.jones 2006-11-29 16:02) - PLID 22293 - added the allowed amount search
			CAllowedAmountSearchDlg dlg(this);
			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnChangePassword()
{
	if(CheckCurrentUserPermissions(bioEditSelfPassword, sptWrite)) {
		CChangePasswordDlg dlg(this);
		dlg.DoModal();
	}
}

void CMainFrame::OnViewCurrentAlerts()
{
	//View the alerts currently stored
	if (!m_dlgAlert.m_hWnd) {
		MsgBox("There are currently no alerts for this session.");
	}
	else {
		m_dlgAlert.ShowWindow(SW_SHOWNORMAL);
		m_dlgAlert.SetForegroundWindow();
	}
}

void CMainFrame::OnActivitiesSearchCheckCCNumbers()
{
	//DRT 7/29/03 - Let the user search for check / cc numbers

	//TODO - Put permissions on this... which ones?  
	//sptView on bioPayment

	if(CheckCurrentUserPermissions(bioPayment, sptRead)) {
		CSearchChecksDlg dlg(this);
		dlg.DoModal();
	}
}

/*	DRT 8/1/03 - See comments in ChangeLoginDlg.cpp for the reasons this is commented out
void CMainFrame::OnToolsChangeCurrentUser()
{
	//DRT 7/31/03 - Lets them change the currently logged in user and location
	//	I tried to use the CLoginDlg for this, but it's setup not to work as a 
	//modal dialog.
	// (a.walling 2007-05-04 10:43) - PLID 4850 - CLoginDlg worked fine for me as
	// a modal dialog, just needed a bit of care. Huge changes in the past 4 years,
	// though. The CChangeLoginDlg was not very aesthetically pleasing, so for consistency
	// I've decided to user the standard login dialog anyway. Users will be less confused
	// and already familiar with the interface.
	// (a.walling 2007-05-04 12:09) - I put the menu item under File for now, since Tools is 
	// already very cluttered, and that way it is nearby Close and Exit.

	CChangeLoginDlg dlg;
	dlg.DoModal();
}
*/

void CMainFrame::OnActivitiesSearchReportDescriptions()
{
	//DRT 8/4/03
	//OK, we need to pass in a pointer to the report view, so that the dialog
	//can pass a message up to add the report to the current batch.  We are pretty
	//sure we will find a report view, because this menu item can only be accessed
	//from the reports module.
	CReportView* pRView = NULL;
	for(int i = 0; i < m_pView.GetSize(); i++) {
		CView* pView = (CView*)m_pView[i];
		if(pView && pView->IsKindOf(RUNTIME_CLASS(CReportView))) {
			pRView = (CReportView*)pView;
			break;
		}
	}
	CSearchReportDescDlg dlg(this);
	dlg.m_pReportView = pRView;	//this could be null
	dlg.DoModal();
}

void CMainFrame::OnToolsHL7Settings()
{
	//TES 6/24/2009 - PLID 34283 - I noticed that this didn't have a try...catch, so I added it.
	try {
		//TES 5/27/2009 - PLID 34282 - Check their permission
		if(CheckCurrentUserPermissions(bioHL7Settings, sptView)) {
			CHL7SettingsDlg dlg(this);
			dlg.DoModal();
		}
	} NxCatchAll("Error in CMainFrame::OnToolsHL7Settings()");
}

void CMainFrame::OnToolsProcessHL7Messages()
{
	try {
		//TES 6/24/2009 - PLID 34283 - Pop up the HL7 Batch dialog.
		//TES 6/30/2009 - PLID 34283 - Check permissions
		if(CheckCurrentUserPermissions(bioHL7BatchDlg, sptRead)) {
			CHL7BatchDlg hl7(this);
			hl7.m_bIsModal = true;
			CNxModalParentDlg dlg(this, &hl7, CString("Process HL7 Messages"));
			dlg.DoModal();
		}
	} NxCatchAll("Error in CMainFrame::OnToolsProcessHL7Messages()");
}

// (j.jones 2008-04-14 09:29) - PLID 29596 - this dialog was removed in lieu
// of the new HL7 tab in the Financial module
/*
void CMainFrame::OnModulesImportFromHL7()
{
	try // (z.manning, 08/22/2007) - PLID 27033 - Added exception handling.
	{
		CHL7ImportDlg dlg;
		dlg.DoModal();

	}NxCatchAll("CMainFrame::OnModulesImportFromHL7");
}
*/

void CMainFrame::OnImportFromNexWeb() 
{

	CNexWebImportDlg dlg(this);
	dlg.DoModal();
}

//(e.lally 2007-07-30) - This function call has been removed so I am taking
	//out the Look Your Best source files from the project
/*
void CMainFrame::OnImportFromLYB() 
{

	CLookYourBestImportDlg dlg;
	dlg.DoModal();
}
*/

void CMainFrame::OnScanMultipleDocs()
{
	try {
		// (e.lally 2009-06-08) PLID 34497 - Permission - History tab write.
		if(CheckCurrentUserPermissions(bioPatientHistory, sptWrite)) {
			CScanMultiDocDlg dlg(this);
			// (r.galicki 2008-09-05 11:52) - PLID 31242 - Added support to load current patient in the patient list
			dlg.m_nPatientID = GetActivePatientID();
			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);
}

//DRT 2/5/2004 - PLID 3598
void CMainFrame::OnActivitiesMergePatient()
{
	if(!CheckCurrentUserPermissions(bioMergePatients, sptWrite) || !CheckCurrentUserPermissions(bioPatient, sptDelete))
		return;

	CMergePatientsDlg dlg(this);
	dlg.DoModal();
}

//JMM 8/3/2005 - PLID 16527
void CMainFrame::OnActivitiesInactivateMultiplePatients()
{
	CInactivateMultiplePatientsDlg dlg(this);
	dlg.DoModal();
}

// (j.gruber 2008-07-14 09:18) - PLID 30695 - Configuration Dialog for Report Segments
void CMainFrame::OnActivitiesConfigureReportSegments()
{
	try {
		CConfigureReportSegmentsDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll("Error in CMainFrame::OnActivitiesConfigureReportSegments()");
}


void CMainFrame::OnActivitiesAddGiftCertificate()
{
	try {
		if(!IsSpa(TRUE)) {
			MsgBox("You must purchase the NexSpa License to use this feature.");
			return;
		}
		// (e.lally 2009-06-08) PLID 34498 - Get permissions - Create new bill
		//	This is just a safety, dlg will check permission before saving.
		else if(GetCurrentUserPermissions(bioBill) & sptCreate|sptCreateWithPass) {
			CGCEntryDlg dlg(this);
			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnActivitiesFindGiftCertificate()
{
	try {
		if(!IsSpa(TRUE)) {
			MsgBox("You must purchase the NexSpa License to use this feature.");
			return;
		}
		// (e.lally 2009-06-08) PLID 34498 - Check permissions - Read patient billing tabs
		else if(CheckCurrentUserPermissions(bioPatientBilling, sptRead)) {
			CGCSearchDlg dlg(this);
			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnActivitiesNexSpaEditCashDrawerSessions()
{
	try {
		if(!IsSpa(TRUE)){
			MsgBox("You must purchase the NexSpa License to use this feature.");
			return;
		}
		// (e.lally 2009-06-08) PLID 34498 - Check Permissions - Write cash drawer
		else if(CheckCurrentUserPermissions(bioCashDrawers, sptWrite)) {
			CEditDrawersDlg dlg(this);
			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnModulesExportToTelevox()
{
	if(CheckCurrentUserPermissions(bioTelevoxLink, sptView)) {
		CTelevoxExportDlg dlg(this);
		dlg.DoModal();
	}
}

void CMainFrame::OnActivitiesSchedulingProductivity()
{
	if(!g_pLicense->CheckForLicense(CLicense::lcMarket, CLicense::cflrUse)) {
		MsgBox("You must purchase the Marketing License to use this feature.");
		return;
	}

	if (UserPermission(MarketingModuleItem)) {
		CSchedulingProductivityDlg dlg(this);
		dlg.DoModal();
	}
}

void CMainFrame::OnViewInquiries()
{
	CViewInquiries dlg(this);
	dlg.DoModal();
}

// (d.singleton 5/24/2011 - 12:02:00) - PLID 43340 - have the importer use the shared path rather than install path for sales DB's
void CMainFrame::OnImportEmrContent()
{
	try
	{
		//d.singleton - created function to handle both the importer and exporter
		OpenEmrExe("NexEMRImporter.exe", FALSE);
	}NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnExportEmrContent()
{
	try
	{
		//d.singelton - created function to handle both the importer and exporter
		OpenEmrExe("NexEMRExporter.exe", TRUE);
	}NxCatchAll(__FUNCTION__);
}

void CMainFrame::OpenEmrExe(LPCTSTR strExeName, BOOL bExport)
{
	if(g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
		MsgBox("You must purchase the EMR License to use this feature.");
	}
	else {
		if(CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
			CString strRegKey = GetSubRegistryKey();
			// (d.singleton 5/24/2011 - 12:02:00) - PLID 43340 - have the importer use the shared path rather than install path for sales DB's
			// created ConfigRT value to determine if its a sales DB that should use the shared path
			long nSalesImporter = GetRemotePropertyInt("EMRImporterUseSharedPath", 0, 0, "<None>", true);
			CString strExeFile, strDirectory;
			if(nSalesImporter == 1)
			{
				//use shared path as exe directory
				strDirectory = GetSharedPath();
				strExeFile = strDirectory ^ strExeName;
				if(!DoesExist(strExeFile)) {
					strExeFile.Empty();
				}
			}
			
			if(strExeFile.IsEmpty())
			{
				//this is not a sales DB so use install path like normal
				strDirectory = NxRegUtils::ReadString(GetRegistryBase() ^ "InstallPath");
				strExeFile = strDirectory ^ strExeName;
			}
			if(!DoesExist(strExeFile)) {
				MsgBox("%s could not be found.  Please contact NexTech for assistance.", strExeName);
			}
			else {
				//if bExport is false than its NOT the Export.exe we are opening,  so display the Importer.exe disclaimer
				if(!bExport)
				{
					if(IDYES == MsgBox(MB_YESNO, "This utility will import the latest version of the EMR content.  "
						"This may result in your changes to the EMR being overwritten, and such changes can NOT be undone.\n"
						"Are you sure you wish to continue?")) {
							if(!strRegKey.IsEmpty()) strRegKey = "r:" + strRegKey;
							// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
							ShellExecute(NULL, NULL, strExeFile, strRegKey, strDirectory, SW_SHOW);
					}
				}
				else
				{
					if(!strRegKey.IsEmpty()) strRegKey = "r:" + strRegKey;
					// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
					ShellExecute(NULL, NULL, strExeFile, strRegKey, strDirectory, SW_SHOW);
				}
			}
		}
	}
}

void CMainFrame::OnToolsImportNexFormsContent()
{
	CWaitCursor wc;
	try
	{
		// (z.manning, 07/17/2007) - PLID 18359 - Open the NexForms importer. Note: this option should not
		// even exist if they aren't licensed for NexForms.
		// (z.manning, 07/24/2007) - PLID 26370 - Check the permission.
		if(CheckCurrentUserPermissions(bioAdminImportNexForms, sptView))
		{
			CNexFormsImportWizardMasterDlg dlg(this);
			dlg.DoModal();
		}

	}NxCatchAll("CMainFrame::OnToolsImportNexFormsContent");
}

//DRT 11/3/2008 - PLID 31789
void CMainFrame::OnToolsImportNxCycleOfCareContent()
{
	try {
		if(g_pLicense->CheckForLicense(CLicense::lcCycleOfCare, CLicense::cflrUse)) {
			//Additionally, ensure that support is up to date.  We're not doing any fancy
			//	processing on the server's end, so we need to confirm it here.
			if(g_pLicense->GetExpirationDate() < COleDateTime::GetCurrentTime()) {
				//Send them to our support staff.  Note that I intentionally check via the local
				//	computer time here, just in case some support issue comes up, we can work
				//	around it.  We have to trust something, and it's not much more work for a user
				//	to change the server time if they really wanted to fake it.
				AfxMessageBox("Unable to download NexTech Cycle of Care data.  Please contact NexTech "
					"Technical Support.");
				return;
			}
			CNxCoCWizardMasterDlg dlg(this);
			dlg.DoModal();
		}
	} NxCatchAll("Error in OnToolsImportNxCycleOfCareContent");
}

// (z.manning 2009-04-09 10:20) - PLID 33934
void CMainFrame::OnToolsImportEmrStandardContent()
{
	try
	{
		// (j.armen 2012-06-19 08:53) - PLID 50688 - This also needs to work for EMR Cosmetic Licensing
		if(g_pLicense->HasEMR(CLicense::cflrUse) == 2)
		{
			//Additionally, ensure that support is up to date.  We're not doing any fancy
			//	processing on the server's end, so we need to confirm it here.
			if(g_pLicense->GetExpirationDate() < COleDateTime::GetCurrentTime()) {
				//Send them to our support staff.  Note that I intentionally check via the local
				//	computer time here, just in case some support issue comes up, we can work
				//	around it.  We have to trust something, and it's not much more work for a user
				//	to change the server time if they really wanted to fake it.
				AfxMessageBox("Unable to import NexTech EMR Standard data.  Please contact NexTech "
					"Technical Support.");
				return;
			}

			CNxCoCWizardMasterDlg dlg(this);
			dlg.m_bEmrStandard = TRUE;
			dlg.DoModal();
		}

	}NxCatchAll("CMainFrame::OnToolsImportEmrStandardContent");
}

BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD nID;
	//since most of these are one line functions, this can save a lot of space
	if (HIWORD(wParam) == 0 || HIWORD(wParam) == 1)//menu or accelerator
	switch (nID = LOWORD (wParam))//id of control
	{	case ID_PATIENTS_MODULE: OnPatientsModule(); break;
		case ID_SCHEDULER_MODULE: OnSchedulerModule(); break;
		case IDM_UPDATE_VIEW: OnUpdateView(); break;
		case ID_VIEW_TOOLBARS_PATIENTS_BAR: OnViewToolbarsPatientsBar(); break;
		case ID_VIEW_TOOLBARS_STANDARD_BAR: OnViewToolbarsStandardBar(); break;
		case ID_VIEW_HIGHLIGHT_SORTED_COLUMN: OnViewHighlightSortedColumn(); break;
		case ID_VIEW_ENHANCED_NXCOLOR: {
			// (a.walling 2008-04-03 14:58) - PLID 29497
			m_bUseEnhancedNxColor = m_bUseEnhancedNxColor ? FALSE : TRUE;
			NxRegUtils::WriteLong("HKEY_CURRENT_USER\\Software\\NexTech\\NxColor\\Advanced\\EnhancedDrawing", m_bUseEnhancedNxColor, true);
			// Tell the user they have to re-open practice
			MsgBox(MB_ICONINFORMATION|MB_OK, "This change will take effect the next time you start Practice.");
			break;
		}
		case ID_VIEW_SHOW_TOOLBAR_TEXT: {
			// (a.walling 2008-04-21 13:34) - PLID 29731 - Toggle text labels on toolbar
			long nToolbarTextHeight = m_bToolbarText ? 0 : 16;
			SetRemotePropertyInt("ToolbarTextHeight", nToolbarTextHeight, 0, GetCurrentUserName());
			m_bToolbarText = !m_bToolbarText;
			// Tell the user they have to re-open practice
			MsgBox(MB_ICONINFORMATION|MB_OK, "This change will take effect the next time you start Practice.");
			break;
		}
		// (a.walling 2012-03-02 10:43) - PLID 48589 - No more toolbar borders
		//case ID_VIEW_SHOW_TOOLBAR_BORDERS: {
		//	// (a.walling 2008-10-07 12:13) - PLID 31575 - Toggle toolbar borders
		//	m_bToolbarBorders = !m_bToolbarBorders;
		//	SetRemotePropertyInt("ToolbarBorders", m_bToolbarBorders, 0, GetCurrentUserName());
		//	// Tell the user they have to re-open practice
		//	MsgBox(MB_ICONINFORMATION|MB_OK, "This change will take effect the next time you start Practice.");
		//	break;
		//}
		case ID_CHOOSE_PATIENT: OnChoosePatient(); break;
		case ID_GOTO_FIRST_PATIENT: OnGotoFirstPatient(); break;
		case ID_GOTO_LAST_PATIENT: OnGotoLastPatient(); break;
		case ID_GOTO_PREVIOUS_PATIENT: OnGotoPreviousPatient(); break;
		case ID_GOTO_NEXT_PATIENT: OnGotoNextPatient(); break;
		case ID_BUTTON_BREADCRUMBS: OnPatientBreadcrumbs(); break; // (a.walling 2010-05-21 10:12) - PLID 17768
		case ID_NEW_PATIENT_BTN: OnNewPatientBtn(); break;
		case ID_PALM_PILOT: OnPalmPilot(); break;
		case ID_FIRST_AVAILABLE_APPT: OnFirstAvailableAppt(); break;
		case ID_SHOW_MOVEUP_LIST:  // (a.walling 2011-01-17 13:06) - PLID 40444 - Handle show waitinglist message in the mainframe
			{
				if(FlipToModule(SCHEDULER_MODULE_NAME)) {
					CNxTabView* pView = (CNxTabView *)GetOpenView(SCHEDULER_MODULE_NAME);
					if (pView) {
						pView->SendMessage(WM_COMMAND, wParam, lParam);
					}
				}
			}
			break;
		case ID_PATIENT_DELETE: OnPatientDelete(); break;
		case ID_GO_BACK: OnGoBack(); break;
		case ID_ADMINISTRATOR_MODULE: OnAdministratorModule(); break;
		case ID_CONTACTS_MODULE: OnContactsModule(); break;
		case ID_FINANCIAL_MODULE: OnFinancialModule(); break;
		case ID_INVENTORY_MODULE: OnInventoryModule(); break;
		case ID_LETTER_WRITING_MODULE: OnLetterWritingModule(); break;
		case ID_MARKETING_MODULE: OnMarketingModule(); break;
		case ID_REPORTS_MODULE: OnReportsModule(); break;
		case ID_SURGERYCENTER_MODULE: OnSurgeryCenterModule(); break;
		// (d.thompson 2009-11-16) - PLID 36134
		case ID_LINKS_MODULE: OnLinksModule(); break;
		case ID_CARECREDIT_BTN: OnApplyCareCredit(); break;
		case ID_TEMPLATING_BTN: OnTemplatingBtn(); break;
		// (z.manning 2014-12-01 13:50) - PLID 64205
		case ID_TEMPLATE_COLLECTIONS_BTN: OnTemplateCollectionsBtn(); break;
		//TES 6/19/2010 - PLID 5888
		case ID_RESOURCEAVAIL_TEMPLATING: OnResourceAvailTemplating(); break;
		case ID_ROOM_MANAGER: OnRoomManagerBtn(); break; 
		case ID_RESCHEDULING_QUEUE: OnReschedulingQueue(); break; // (a.walling 2014-12-22 12:07) - PLID 64369 - Rescheduling Queue command in menus
		case ID_APT_BOOKING_ALARMS: OnAptBookingAlarmsBtn(); break;
		case ID_OPEN_TODO_LIST: // Pop up the to do list
			ShowTodoList();
			break;
		case ID_SHOW_ACTIVE_EMR:
			ShowActiveEMR();
			break;
		case ID_LOCK_EMR:
			ShowLockManager();
			break;
		case ID_MENU_EMRSEARCH:
			ShowEMRSearch();
			break;
		case ID_MENU_EMRWRITABLE:
			// (a.walling 2008-06-26 16:02) - PLID 30531
			ShowEMRWritableReview();
			break;
		case ID_EMR_FINALIZEPATIENTEMNSFROMNEXWEB:
			// (d.thompson 2009-11-04) - PLID 35811
			{
				if(CheckCurrentUserPermissions(bioFinalizeNexWebEMNs, sptRead)) {
					CForceNexWebEMNDlg dlg(this);
					dlg.DoModal();
				}
			}
			break;
		case ID_MENU_EMRSEENTODAY:
			ShowSeenToday();
			break;
		case ID_MANAGE_EMRPROVIDERS:	// (a.walling 2006-12-19 13:01) - PLID 23397 - Show the EMR Provider licensing dialog
			{
				// (z.manning 2013-02-04 09:23) - PLID 55001 - Set the dialog type to emr provider
				CLicenseEMRProvidersDlg dlg(lmdtEmrProviders, this);
				dlg.DoModal();
			}
			break;
		case ID_MANAGE_EMRPROVIDER_LINKS:
			{
				CConfigurePrimaryEMNProviderDlg dlg(this); //(r.wilson 7/29/2013) PLID 48684 - Configure Primary EMN Provider Links dialog 
				dlg.DoModal();
				break;
			}
		case ID_MANAGE_NUANCE_USERS: // (z.manning 2013-02-04 11:08) - PLID 55001
			{
				CLicenseEMRProvidersDlg dlg(lmdtNuanceUsers, this);
				dlg.DoModal();
			}
			break;
		case ID_MANAGE_PORTAL_PROVIDERS: // (z.manning 2015-06-18 15:17) - PLID 66280
			{
				CLicenseEMRProvidersDlg dlg(lmdtPortalProviders, this);
				dlg.DoModal();
			}
			break;
		case ID_PAPERBATCH: 
			OnPaperBatch();
			break;
		// j.anspach 04/11/2005 PLID 13663 - Removing the ability to do your own download from Practice.
		//case IDC_DOWNLOAD: OnDownload(); break;
		case ID_FILTER: OnFilter(); break;
		case ID_VIEW_TOOLBARS_GUI: OnViewToolbarsGui(); break;
		case ID_MANUAL_BTN: OnManualBtn(); break;
		// (z.manning 2009-03-04 12:29) - PLID 33104 - Separate function for the toolbar button
		case ID_MANUAL_TOOLBAR_BTN: OnManualToolbarBtn(); break;
		case ID_HELP_HELPMANUALPDF:	OnPDFManual();	break;
		case ID_SUPERBILLALL: OnSuperbillall(); break;
		//DRT 6/11/2008 - PLID 30306 - Added a dialog to configure superbill default templates
		case ID_ACTIVITIES_SUPERBILLS_CONFIGUREDEFAULTSUPERBILLTEMPLATES:
			{
				CConfigureSuperbillTemplatesDlg dlg(this);
				dlg.DoModal();
			}
			break;
		case ID_NEXT_CONTACT_BTN: OnNextContactBtn(); break;
		case ID_PREV_CONTACT_BTN: OnPrevContactBtn(); break;
		case ID_APPLY_CARECREDIT: OnApplyCareCredit(); break;
		case ID_MIRROR_LINK: OnLinkToMirror(); break;
		case ID_UNITED: OnLinkToUnited(); break;
		case ID_QUICKBOOKS_LINK: OnLinkToQuickbooks(); break;
		case ID_LINK_TO_CODELINK: OnLinkToCodeLink(); break;
		case ID_BOOL_LOG_FILE: OnLogFile(); break;
		case ID_TOOLS_ENABLEBARCODESCANNER: 
			{
				//(a.wilson 2012-1-12) PLID 47517 - kill thread so that we can restart it in the dlg.
				if (m_pOPOSBarcodeScannerThread) {
					m_pOPOSBarcodeScannerThread->Shutdown();
					m_pOPOSBarcodeScannerThread = NULL;
					m_bOPOSBarcodeScannerInitComplete = false;
				}

				//(a.wilson 2012-1-18) PLID 47517 - create and destroy the dialog appropriately
				try {
					CBarcodeSetupDlg dlg(this);
					m_pBarcodeSetupDlg = &dlg;
					dlg.DoModal();
				} NxCatchAll("ID_TOOLS_ENABLEBARCODESCANNER");
				m_pBarcodeSetupDlg = NULL;

				break;
			}
		case ID_TOOLS_MSRSETTINGS: 
			{
				// (a.wetta 2007-03-16 11:07) - PLID 25236 - Open up the MSR device settings dialog
				CSwiperSetupDlg dlgSwiperSetup(m_pOPOSMSRThread, this);
				dlgSwiperSetup.DoModal(); 

				// (a.wetta 2007-07-05 16:38) - PLID 26547 - Get the pointer back in case they have made changes to the thread
				m_pOPOSMSRThread = dlgSwiperSetup.m_pOPOSMSRThread;

				break;
			}
		case ID_ACTIVITIES_UPDATEPASTPENDINGAPPTS: OnUpdatePastAppts(); break;
		case ID_TOOLS_ENABLECALLERID: OnToggleCallerID(); break;
		// (j.gruber 2007-07-03 09:36) - PLID 26535 - Credit Card Processing Setup
		case IDM_CREDIT_CARD_PROCESSING_SETUP: OnCreditCardProcessingSetup(); break;
		// (j.gruber 2007-07-09 17:06) - PLID 26584 - Credit Card Batch Processing
		// (d.thompson 2009-04-16) - PLID 33973 - Removed this menu option
		//case IDM_CC_BATCH_PROCESSING: OnCreditCardBatchProcessing(); break;
		// (j.gruber 2007-08-16 13:05) - PLID 27091 - tracking config dialog
		case IDM_CONFIGURE_TRACKING_SUMMARY_REPORT: OnTrackingConversionsConfigDlg(); break;
		// (j.camacho 2013-07-24 12:24) - PLID 57518
		case IDM_CARECREDIT_SETUP: OnCareCreditSetupDlg(); break;
		case ID_TOOLS_TWAIN_USELEGACY: OnToggleTWAINUseLegacy(); break; // (a.walling 2010-01-28 16:01) - PLID 37107 - Support fallback to legacy TWAIN 1.9
		case ID_TOOLS_TWAIN_SHOWTWAINUI: OnToggleTWAINShowUI(); break;
		case ID_VIEW_SHOWIMAGESINGENERAL1: OnToggleShowG1Images(); break;
		case ID_VIEW_SHOWMIRRORIMAGESINGENERAL1: OnToggleShowMirrorImages(); break; //TES 11/17/2009 - PLID 35709
		case ID_VIEW_SHOWWARNINGDETAILSINGENERAL2: OnToggleShowG2WarningDetails(); break;
		case ID_VIEW_SHOWPRIMARYIMAGEONLY: OnToggleShowPrimaryOnly(); break;
		case ID_PRACYAKKER_ENABLE:
			if (CheckCurrentUserPermissions(bioPracYakker, sptView)) { // (v.maida 2014-12-27 10:19) - PLID 27381 - permission for using PracYakker.
					ASSERT(m_pdlgMessager);
					if (m_pdlgMessager)
						m_pdlgMessager->ChangeLogonStatus();
			}
			break;
		case ID_PRACYAKKER_OPEN: 
			{
				if (CheckCurrentUserPermissions(bioPracYakker, sptView)) { // (v.maida 2014-12-27 10:19) - PLID 27381 - permission for using PracYakker.
					//TES 12/19/2008 - PLID 32525 - They can't use the PracYakker if they're on Scheduler Standard
					ASSERT(m_pdlgMessager);
					if (g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "PracYakker intra-office messaging", "PracYakker/pracyakker.htm")) {
						if (m_pdlgMessager) m_pdlgMessager->PopupYakker();
					}
				}
			}
			break;
		case ID_TOOLS_AUTOCALLER: OnAutoCaller(); break;
		case ID_TOOLS_PALMSETTINGS:  m_PalmPilotDlg.DoModal(); break;
		case ID_TOOLS_NEXPDASETTINGS: OnNexPDALinkSettings(); break;
		case IDM_TOOLS_NEXSYNC_SETTINGS: OnNexSyncSettings(); break; // (z.manning 2009-08-26 14:39) - PLID 35345
		case ID_TOOLS_APPOINTMENTREMINDERSETTINGS: OnAppointmentReminderSettings(); break; // (c.haag 2010-04-01 16:48) - PLID 38018
		case ID_TOOLS_ANCHOR_SETTINGS: CAnchorSettingsDlg(this).DoModal(); break;	// (j.armen 2013-08-27 14:24) - PLID 58034
		case ID_TOOLS_RUNSAMPLECALLERID:  OnTestCallerID(); break;
		case ID_CONFIG_STATEMENT: OnConfigStatement(); break;
		case IDM_CONFIGURE_REPORT_GROUPS: OnConfigReportGroups(); break;
		case ID_ACTIVITIES_CCHITREPORTS:	// (d.thompson 2010-01-18) - PLID 36927
			{
				//Use EMR Read permission, as most of these statistics come out of EMR
				if(CheckCurrentUserPermissions(bioPatientEMR, sptRead)) {
					// (j.gruber 2011-11-01 09:58) - PLID 46219 - make dialog modeless
					ShowCCHITReportsDlg();					
				}
			} break;
		case ID_ACTIVITIES_CCHITDETAILED:	// (j.dinatale 2012-10-31 12:08) - PLID 53502 - MU detailed report
			ShowMUDetailedReport();
			break;
		case ID_CHANGE_LOCATION: OnChangeLocation(); break;
		case IDM_EDITREPORTS: OnEditReports(); break;
		// (j.gruber 2006-11-27 17:34) - PLID 23673 - added dialog
		case IDM_CONFIGURE_AAFPRS: OnConfigureAAFPRSSurvey(); break;
		case ID_ACTIVITIES_TRACKSERIALIZEDITEMS: OnTrackSerializedItems(); break;
		case ID_ACTIVITIES_GENERATEBARCODES: OnGenerateBarcodes(); break;
		case ID_PATIENT_SERIALIZED_ITEMS: OnPatientSerializedItems(); break;
		case ID_UPDATE_PREFIX: OnUpdatePrefixes(); break;
		case ID_IDPA_SETUP: OnIDPASetup(); break;
		case ID_NYWC_FORM_SETUP: OnNYWCSetup(); break;
		// (j.jones 2007-05-09 17:47) - PLID 25550 - added MICR Setup
		case ID_MICR_FORM_SETUP: OnMICRSetup(); break;
		// (k.messina 2010-07-15 16:12) - PLID 39685 - added NY Medicaid Setup
		case ID_NYMEDICAID_FORM_SETUP: OnNYMedicaidSetup(); break;
		case ID_MNU_SSRS_SETUP: 
		{
			OnSSRSSetup();
		}
		break;
		case ID_CALC: OnCalc(); break;
		case ID_CALC_FINANCE_CHARGES: OnCalcFinanceCharges(); break;
		// (j.jones 2013-08-01 13:19) - PLID 53317 - added ability to undo finance charges
		case ID_UNDO_FINANCE_CHARGES: OnUndoFinanceCharges(); break;
		case ID_HELP_CONNECTTOTECHNICALSUPPORT: OnConnectToTechSupport(); break;
		case ID_ACTIVITIES_CPTEDITOR: 
			AdminCPTEdit(); 
			break;
		case ID_NEW_CONTACT: OnNewContact(); break;
		case ID_CONTACT_ADD:	OnNewContact(); break;
		case ID_CONTACT_DELETE: OnContactDelete(); break;
		case ID_CONTACT_CHANGE:	OnContactChange(); break;
		case ID_INFORM: OnInform(); break;
		// (<j.gruber> 2006-11-01 10:06) - PLID 23208 - added link to webinars 
		case IDM_VIEW_WEBINARS: OnViewWebinars(); break;
		//TES 8/24/2004: These buttons no longer exist.
		/*case ID_NEXT_DOC_BTN: OnNextDocBtn(); break;
		case ID_PREV_DOC_BTN: OnPrevDocBtn(); break;*/
		case ID_SUBMIT_TO_UNICORN: OnSubmitToChaseHealthAdvance(); break;
			// (a.walling 2007-05-17 16:04) - PLID 26058 - Cash drawer config
		case ID_TOOLS_CASHDRAWER_CONFIG: OnCashDrawerSettings();break;
		// (j.gruber 2007-05-07 15:02) - PLID 25772 - receipt printer configuration
		case ID_TOOLS_RECEIPT_PRINTER_CONFIG: OnReceiptPrinterSettings();break;
		// (j.gruber 2007-05-08 12:27) - PLID 25931 - Receipt Settings
		case ID_TOOLS_RECEIPT_CONFIG: OnConfigureReceipts(); break;

		case ID_TOOLS_TWAIN_SELECTSOURCE:
			NXTWAINlib::SelectSource();			
			break;


		case ID_HELP_WHATSNEW:
			//(j.anspach 06-09-2005 10:56 PLID 16681) - Removing the "What's New?" from all the help menus.
			// (a.vengrofski 2009-11-11 17:56) - PLID <36104> - Adding the "What's New.." dialog back in.
			/*OpenManual("Nextech Practice 2004 Help.chm", "New_In_Practice_2002.htm");*/
			ShowWhatsNew(TRUE); // (a.vengrofski 2009-11-11 17:59) - PLID <36104> - Force the "What's New" dialog to appear.
			break;

		case ID_FILE_PRINT:
		case ID_FILE_PRINT_PREVIEW:
			// Let it be handled by the view
			break;

		case ID_IMPORT_FILE: ImportPatientFile(); break;

		// (f.dinatale 2010-09-23) - PLID 40646 - Option added to bring up PQRIDlg
		case ID_ACTIVITIES_PQRSCQM_REPORTING:
			{

			// (b.eyers 2016-04-29 14:59) - NX-100350 - open pqrs exporter
			OpenPQRSExporter(GetSubRegistryKey());
			} break;

		case ID_TOOLS_LOGTIME_STARTLOGGING:
			StartLogTime();
			break;
		case ID_TOOLS_LOGTIME_ENDLOGGING:
			EndLogTime();
			break;

		// All the lookup ones just call Lookup, passing in the command ID
		case ID_LOOKUP_CITY://lookup options
		case ID_LOOKUP_STATE:
		case ID_LOOKUP_ZIP:
		case ID_LOOKUP_HOMEPHONE:
		case ID_LOOKUP_WORKPHONE:
		case ID_LOOKUP_GENERAL2_PATIENTTYPETEXT:
		case ID_LOOKUP_GENERAL2_PATIENTTYPE:
		case ID_LOOKUP_HAS_PHOTO_FOR_PROCEDURE:
		case ID_LOOKUP_REFERRALSOURCE:
		case ID_LOOKUP_CUSTOM1:
		case ID_LOOKUP_CUSTOM2:
		case ID_LOOKUP_CUSTOM3:
		case ID_LOOKUP_CUSTOM4:
		case ID_LOOKUP_MAIN_PROVIDER:
		case ID_LOOKUP_FIRST_CONTACT:
		case ID_LOOKUP_BLANKFILTER:
		case ID_LOOKUP_GENERAL2_OCCUPATION:
		case ID_LOOKUP_GENERAL2_COMPANYSCHOOL:
		case ID_LOOKUP_GENERAL2_DEFAULTREFERRINGPHYSICAN:
		case ID_LOOKUP_GENERAL2_LOCATION:
		case ID_LOOKUP_GENERAL2_EMPLOYMENT:
		case ID_LOOKUP_GENERAL2_REFFERINGPATIENT:
		case ID_LOOKUP_EXISTINGGROUP:
		case ID_LOOKUP_TODAY:
		case ID_LOOKUP_CURRENT:
			Lookup(nID);
			break;
		case ID_LOOKUP_PATIENT_LISTS:
			{
				// (r.gonet 09/30/2013) - PLID 56236 - Open the patient lists dialog.
				CPatientListsDlg dlg(this);
				dlg.DoModal();
			}
			break;
		case ID_TRANSFER_TODOS:
			TransferTodos();
			break;
		case ID_MODULES_EXPORTTOHL7:
			{
				//TES 5/18/2009 - PLID 34282 - Check that they have permission to export patients.
				if(CheckCurrentUserPermissions(bioHL7Patients, sptDynamic0)) {
					m_dlgHL7Export.DoModal(hprtPatient);
				}
			}
			break;
		case ID_MODULES_EXPORTTOHL7_APPTS:
			{
				//TES 5/18/2009 - PLID 34282 - Check that they have permission to export patients.
				if(CheckCurrentUserPermissions(bioHL7Appointments, sptDynamic0)) {
					// (z.manning 2008-07-21 09:39) - PLID 30782 - Added menu option for appt HL7 export
					m_dlgHL7Export.DoModal(hprtAppt);
				}
			}
			break;
		case ID_MODULES_EXPORTTOHL7_EMN_BILLS:
			{
				//TES 7/10/2009 - PLID 25154 - Check that they have permission to export bills.
				if(CheckCurrentUserPermissions(bioHL7EmnBills, sptDynamic0)) {
					//TES 7/10/2009 - PLID 34845 - Added menu option for exporting EMN Bills
					m_dlgHL7Export.DoModal(hprtEmnBill);
				}
			}
			break;
		case ID_MODULES_EXPORTTOHL7_LABRESULTS:
			{
				// (d.singleton 2012-12-20 10:08) - PLID 53041 need options for lab results and locked emns
				if(GetRemotePropertyInt("HL7ExportLabResults", 0, 0, "<none>", true)) {
					m_dlgHL7Export.DoModal(hprtLabResult);
				}
			}
			break;
		case ID_MODULES_EXPORTTOHL7_LOCKEDEMNS:
			{
				// (d.singleton 2012-12-20 10:08) - PLID 53041 need options for lab results and locked emns
				if(GetRemotePropertyInt("HL7ExportLockedEMNs", 0, 0, "<none>", true)) {
					m_dlgHL7Export.DoModal(hprtLockedEmn);
				}
			}
			break;
		case ID_POST_TO_BILL_ID:
			{
				PostToBillID();				
			}
			break;
			//DRT 7/3/2008 - PLID 19596 - Setup online fax service.
			//(e.lally 2011-03-18) PLID 42767 - Changed ID to have "outgoing"
		case ID_TOOLS_FAXING_CONFIGUREOUTGOINGONLINEFAXSERVICE:
			{
				//DRT 8/1/2008 - PLID 30915 - To better clarify this option, if the user does
				//	not have an active license, we'll give them a nice message box which explains
				//	the usage and directs them to contact their sales rep.
				if(g_pLicense->CheckForLicense(CLicense::lcFaxing, CLicense::cflrSilent)) {
					CFaxSetupDlg dlg(this);
					dlg.DoModal();
				}
				else {
					//(e.lally 2011-03-18) PLID 42767 - Updated to be "send and receive"
					AfxMessageBox("NexTech is now integrated with MyFax online faxing service.  This service will allow "
						"you to send and receive faxes via the internet.  You are not currently licensed to use the MyFax integration.\r\n\r\n"
						"If you would like more information on this product, please contact your sales representative at "
						"(800) 490-0821.");
				}
			}
			break;
			//(e.lally 2011-03-18) PLID 42767 - Configure Online Fax Service Downloads
		case ID_TOOLS_FAXING_CONFIGUREINCOMINGONLINEFAXSERVICE:
			{
				//(e.lally 2011-03-18) PLID 42767 - To better clarify this option, if the user does
				//	not have an active license, we'll give them a nice message box which explains
				//	the usage and directs them to contact their sales rep.
				if(g_pLicense->CheckForLicense(CLicense::lcFaxing, CLicense::cflrSilent)) {
					CFaxServiceSetupDlg dlg(this);
					dlg.DoModal();
				}
				else {
					AfxMessageBox("NexTech is now integrated with MyFax online faxing service.  This service will allow "
						"you to send and receive faxes via the internet.  You are not currently licensed to use the MyFax integration.\r\n\r\n"
						"If you would like more information on this product, please contact your sales representative at "
						"(800) 490-0821.");
				}
			}
			break;
			//DRT 7/3/2008 - PLID 19596 - Send a generic fax.
		case ID_TOOLS_FAXING_SENDFAX:
			{
				//DRT 8/19/2008 - PLID 30915 - Need to confirm license.  If they are on a trial usage, this will be enabled, and
				//	we need to make it take a usage.
				if(g_pLicense->CheckForLicense(CLicense::lcFaxing, CLicense::cflrUse)) {
					CFaxSendDlg dlg(this);
					dlg.DoModal();
				}
			}
			break;

		// (j.jones 2009-01-07 13:46) - PLID 26141 - added inventory reconciliation
		case ID_ACTIVITIES_RECONCILE_INV:
			{
				//use the regular Inventory license, not Adv. Inventory
				if(g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrUse)) {

					// (j.jones 2009-07-09 12:53) - PLID 34826 - ensure we have the ability to view inventory
					if(CheckCurrentUserPermissions(bioInvItem, sptRead)) {

						// (j.jones 2009-01-14 16:12) - PLID 32707 - pull up the history dialog
						CInvReconcileHistoryDlg dlg(this);
						dlg.DoModal();
					}
				}
			}
			break;

		//(e.lally 2009-05-11) PLID 28553 - Added Order Set Template setup dlg
		case ID_ACTIVITIES_ORDER_SET_TEMPLATES: { COrderSetTemplateDlg dlg(this); dlg.DoModal(); break; }
		// (a.vengrofski 2009-11-13 16:08) - PLID <36104> - Added a help menu option to launch a browser to the NexTech changes web site.
		case ID_HELP_FEATURESBYVERSION:
			{
				ShellExecute(NULL, NULL, "http://www.nextech.com/Training/Changes.aspx", NULL, NULL, SW_SHOW);
			}
			break;
		// (d.thompson 2010-03-15) - PLID 37729 - Purge cardholder data
		case ID_FINANCIAL_PURGECREDITCARDHOLDERDATA:
			{
				if(CheckCurrentUserPermissions(bioPurgeCardholderData, sptWrite)) {
					CPurgeCardholderDataDlg dlg(this);
					dlg.DoModal();
				}
			}
			break;
		// (a.walling 2010-03-15 14:10) - PLID 37755 - Manual encryption key update requested
		case ID_FINANCIAL_UPDATEENCRYPTIONKEY:
			{
				// Admin only
				if(CheckCurrentUserPermissions(bioUpdateEncryptionKey, sptWrite)) {
					OnUpdateEncryptionKey();
				}
			}
			break;

			// (a.vengrofski 2010-04-20 15:13) - PLID <38205>
		case ID_ACTIVITIES_MANAGEQUICKBOOKSIDS:
			{
				if (CheckCurrentUserPermissions(bioManageQuickBooksRepID, sptView))
				{
					// (a.vengrofski 2010-04-19 11:46) - PLID <38205>
					CQuickBookRepID dlg(this);
					dlg.DoModal();
				}
			}
			break;
			
		case ID_ACTIVITIES_MDIHL7ACTIVITYITEM:
			{			
				ShowHL7ToBeBilledDlg();				
			}
			break;
			// (j.dinatale 2010-11-05) - PLID 39226
		case ID_BILLINGTABCOLUMNSET:
			{
				if(CheckCurrentUserPermissions(bioPatientBilling, sptRead))
				{
					// (j.jones 2013-07-12 16:52) - PLID 57550 - first save the current billing tab column widths
					// if the billing tab is opened and configured to remember column widths
					CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
					if (pView && pView->IsKindOf(RUNTIME_CLASS(CPatientView))) { 
						CPatientView* pPatientView = (CPatientView*)pView;
						if (pPatientView->GetFinancialDlg()->GetSafeHwnd()
							&& pPatientView->GetFinancialDlg()->m_bRememberColWidth) {
							pPatientView->GetFinancialDlg()->SaveColumnWidths();
						}
					}

					CBillTabSettingsDlg dlg(this);
					dlg.DoModal();
				}
			}
			break;

		// (j.jones 2010-12-22 10:56) - PLID 41911 - added financial close ability
		case ID_ACTIVITIES_PERFORMAFINANCIALCLOSE:
			//this is already disabled if you don't have permission,
			//the permission is not actively checked any further until
			//you actually try to perform a close within this dialog
			OnPerformFinancialClose();
			break;

		// (j.jones 2011-03-21 15:33) - PLID 42917 - these menu items just open
		// shared folders on the server
		case ID_VIEWOLDEOBFILES:
			OnViewOldEOBFiles();
			break;

		case ID_VIEWOLDEOBWARNINGSFILES:
			OnViewOldEOBWarningsFiles();
			break;

		// (j.jones 2011-03-28 14:24) - PLID 43012 - added ability to mass-configure
		// the billable status of service codes
		case ID_FINANCIAL_NONBILLABLE_CODES:
			OnEditNonbillableCPTCodes();
			break;

		// (z.manning 2015-07-22 15:34) - PLID 67241
		case ID_MANAGE_PAYMENT_PROFILES:
			// (z.manning 2015-07-22 15:35) - PLID 67241 - This menu option is only in the patients module so
			// okay to use the active patient ID.
			OpenPaymentProfileDlg(GetActivePatientID(), this);
			break;

		// (d.lange 2011-12-07 17:25) - PLID 20751 - Added the various contact types
		case ID_NEWCONTACT_CREATENEWPROVIDER:
			AddContact(dctProvider);
			break;

		case ID_NEWCONTACT_CREATENEWREFERRINGPHYSICIAN:
			AddContact(dctRefPhys);
			break;

		case ID_NEWCONTACT_CREATENEWUSER:
			AddContact(dctEmployer);
			break;
		
		case ID_NEWCONTACT_CREATENEWSUPPLIER:
			AddContact(dctSupplier);
			break;

		case ID_NEWCONTACT_CREATENEWOTHERCONTACT:
			AddContact(dctOther);
			break;

		//TES 3/2/2015 - PLID 64736
		case ID_ENABLE_DEBUG_MODE:
			OnEnableDebugMode();
			break;

		// (d.lange 2015-07-22 14:52) - PLID 67188
		case IDM_ICCP_SETTINGS:
			OnICCPSettings();
			break;
	}
	return CNxMDIFrameWnd::OnCommand(wParam, lParam);
}

// (j.politis 2015-04-20 13:19) - PLID 65484 - The mainframe menu option for the Import feature is disabled if you dont have create permissions for Patients, Contacts, AND Service Codes.
// Copied the permissions code from importtypedlg.  If either of these are true, we'll allow the import, otherwise, DE-NEIN!!!

/*
	IF ANY CHANGES ARE MADE TO THIS FUNCTION, THEN PLEASE REVIEW CImportWizardTypeDlg::OnInitDialog()
	IF NECESSARY, MAKE SURE YOUR CHANGES ARE APPLIED THERE AS WELL.
*/

bool CMainFrame::ImportShouldBeAllowed()
{
	if ((GetCurrentUserPermissions(bioPatient) & SPT____C_______ANDPASS)) {
		return true;
	}

	if ((GetCurrentUserPermissions(bioContact) & SPT____C_______ANDPASS)) {
		return true;
	}

	if ((GetCurrentUserPermissions(bioServiceCodes) & SPT____C_______ANDPASS)) {
		return true;
	}

	if ((GetCurrentUserPermissions(bioAdminScheduler) & SPT___W________ANDPASS)) {
		return true;
	}

	if ((GetCurrentUserPermissions(bioInvItem) & SPT____C_______ANDPASS)) {
		return true;
	}

	if ((GetCurrentUserPermissions(bioRecallSystem) & SPT____C_______ANDPASS)){
		return true;
	}

	if ((GetCurrentUserPermissions(bioInsuranceCo) & SPT____C_______ANDPASS)) {
		return true;
	}

	if ((GetCurrentUserPermissions(bioAppointment) & SPT____C_______ANDPASS)){
		return true;
	}

	if ((GetCurrentUserPermissions(bioPatientNotes) & SPT____C_______ANDPASS)) {
		return true;
	}

	if ((GetCurrentUserPermissions(bioPatientInsurance) & SPT____C_______ANDPASS)) {
		return true;
	}
	return false;
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{	//again, if these are one liners, lets avoid to much code
	if (nCode == CN_UPDATE_COMMAND_UI)
	{	CCmdUI *pCmdUI = (CCmdUI *)pExtra;
		BOOL bLicenseErrorsFound = FALSE;
		CLicense::ELicensedComponent lc;
		switch (nID) {				
			case ID_TOOLS_LOGTIME_PROMPT: 
				if(GetRemotePropertyInt("LogTimePrompt", 0, 0, GetCurrentUserName(), true))
					pCmdUI->SetCheck(true);
				else
					pCmdUI->SetCheck(false);
				
				//enable if they have permission
				if (GetCurrentUserPermissions(bioLogPrompt) & SPT___W_______)
					pCmdUI->Enable(true);

				break;
			case ID_TOOLS_LOGTIME_LOGMANAGEMENT:
				if (GetCurrentUserPermissions(bioLogManage) & SPT___W________ANDPASS)
					pCmdUI->Enable(true);
				break;
			case ID_TOOLS_LOGTIME_STARTLOGGING:
				if(!m_bLoggingTime )
					pCmdUI->Enable(true);
				break;
			case ID_TOOLS_LOGTIME_ENDLOGGING:
				if(m_bLoggingTime )
					pCmdUI->Enable(true);
				break;
			case ID_VIEW_TOOLBARS_PATIENTS_BAR: OnUpdateViewToolbarsPatientsBar(pCmdUI); break;
			case ID_VIEW_TOOLBARS_STANDARD_BAR: OnUpdateViewToolbarsStandardBar(pCmdUI); break;
			case ID_VIEW_TOOLBARS_CONTACTS_BAR: OnUpdateViewToolbarsContactsBar(pCmdUI); break;
			case ID_VIEW_HIGHLIGHT_SORTED_COLUMN: OnUpdateViewHighlightSortedColumn(pCmdUI); break;
			case ID_VIEW_ENHANCED_NXCOLOR: {
				// (a.walling 2008-04-03 14:56) - PLID 29497
				pCmdUI->Enable(TRUE);
				pCmdUI->SetCheck(m_bUseEnhancedNxColor ? 1 : 0);
				break;
			}
			case ID_VIEW_SHOW_TOOLBAR_TEXT: {
				pCmdUI->Enable(TRUE);
				pCmdUI->SetCheck(m_bToolbarText ? 1 : 0);
				break;
			}
			// (a.walling 2012-03-02 10:43) - PLID 48589 - No more toolbar borders
			//case ID_VIEW_SHOW_TOOLBAR_BORDERS: {
			//	// (a.walling 2008-10-07 12:18) - PLID 31575
			//	pCmdUI->Enable(TRUE);
			//	pCmdUI->SetCheck(m_bToolbarBorders ? 1 : 0);
			//	break;
			//}
			case ID_CHOOSE_PATIENT: OnUpdateChoosePatient(pCmdUI); break;
			case ID_GOTO_FIRST_PATIENT: OnUpdateGotoFirstPatient(pCmdUI); break;
			case ID_GOTO_LAST_PATIENT: OnUpdateGotoLastPatient(pCmdUI); break;
			case ID_GOTO_PREVIOUS_PATIENT: OnUpdateGotoPreviousPatient(pCmdUI); break;
			case ID_GOTO_NEXT_PATIENT: OnUpdateGotoNextPatient(pCmdUI); break;
			case ID_BUTTON_BREADCRUMBS: pCmdUI->Enable(TRUE); break; // (a.walling 2010-05-21 10:12) - PLID 17768
			case ID_PATIENT_DELETE: OnUpdatePatientDelete(pCmdUI); break;
			case ID_APPLY_CARECREDIT:
				// (z.manning, 01/10/2007) - PLID 24057 - Only enable the menu option if usage of CareCredit has not expired.
				// (z.manning, 03/02/2007) - PLID 25044 - Let's just hide it altogether.
				if(NxCareCredit::GetCareCreditLicenseStatus() == cclsExpired) {
					RemoveEntryFromSubMenu(GetMainFrame()->GetMenu(), "Modules", ID_APPLY_CARECREDIT, FALSE);
				}
				else {
					// (z.manning, 03/02/2007) - PLID 25044 - Ok, it's not expired, so let's enable it. It is
					// intentional that we enable even if they aren't licensed for it so they get the "marketing" dialog.
					pCmdUI->Enable(TRUE);
				}
				break;
			case ID_IMPORT_FILE:
				// (j.jones 2010-10-19 09:23) - PLID 34571 - disable importing if they don't have permission
				// to create anything from the import
				// (j.politis 2015-04-16 16:12) - PLID 65484
				if (ImportShouldBeAllowed()) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;
			case ID_MIRROR_LINK: 
				lc = CLicense::lcMirror;
				pCmdUI->Enable(g_pLicense->CheckForLicense(lc, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound));
				break;
			case ID_QUICKBOOKS_LINK: 
				lc = CLicense::lcQuickBooks;
				pCmdUI->Enable(g_pLicense->CheckForLicense(lc, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound));
				break;
			case ID_INFORM: 
				lc = CLicense::lcInform;
				// (j.jones 2009-12-16 14:34) - PLID 36275 - if you do not have an inform license, remove the menu option,
				// so people without the license do not know that the ability exists
				if(g_pLicense->CheckForLicense(CLicense::lcInform, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					RemoveEntryFromSubMenu(GetMainFrame()->GetMenu(), "Modules", ID_INFORM, FALSE);
				}
				break;
			case ID_SUBMIT_TO_UNICORN: 
				{
					BOOL bEnable = FALSE;
					// (e.lally 2009-06-10) PLID 34529 - Get permissions - View Unicorn financial module
					lc = CLicense::lcUnicorn;
					if(g_pLicense->CheckForLicense(CLicense::lcUnicorn, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound) &&
						(GetCurrentUserPermissions(bioChaseHealthAdvance) & (sptView|sptViewWithPass))){
							bEnable = TRUE;
					}
					pCmdUI->Enable(bEnable);
					break;
				}
			case ID_SCHEDULER_MODULE: 
				pCmdUI->Enable(m_enableViews & TB_SCHEDULE);
				break;
			case ID_PATIENTS_MODULE: 
				pCmdUI->Enable(m_enableViews & TB_PATIENT);
				break;
			case ID_ADMINISTRATOR_MODULE: 
				pCmdUI->Enable(m_enableViews & TB_ADMIN);
				break;
			case ID_CONTACTS_MODULE: 	
				pCmdUI->Enable(m_enableViews & TB_CONTACT);
				break;
			case ID_FINANCIAL_MODULE:
				pCmdUI->Enable(m_enableViews & TB_FINANCIAL);
				break;
			case ID_INVENTORY_MODULE: 
				pCmdUI->Enable(m_enableViews & TB_INVENTORY);
				break;
			case ID_LETTER_WRITING_MODULE:
				pCmdUI->Enable(m_enableViews & TB_DOCUMENT);
				break;
			case ID_MARKETING_MODULE: 	
				pCmdUI->Enable(m_enableViews & TB_MARKET);
				break;
			case ID_REPORTS_MODULE: 
				pCmdUI->Enable(m_enableViews & TB_REPORTS); 
				break;
			case ID_SURGERYCENTER_MODULE:
				pCmdUI->Enable(m_enableViews & TB_ASC);
				break;
			// (d.thompson 2009-11-16) - PLID 36134
			case ID_LINKS_MODULE:
				pCmdUI->Enable(m_enableViews & TB_LINKS);
				break;
			case ID_NEW_PATIENT_BTN:
				pCmdUI->Enable(m_enableViews & TB_NEWPATIENT); 
				break;
			case ID_TB_NEW_EMR:
				pCmdUI->Enable(m_enableViews & TB_EMR);
				break;
			case ID_FIRST_AVAILABLE_APPT:
				pCmdUI->Enable(m_enableViews & TB_FFA);
				break;
			case ID_CARECREDIT_BTN:
				pCmdUI->Enable(m_enableViews & TB_CARECREDIT);
				break;

			case ID_ACTIVITIES_CREATENEWAPPOINTMENT:
			case ID_TEMPLATING_BTN:
			case ID_TEMPLATE_COLLECTIONS_BTN: // (z.manning 2014-12-01 13:41) - PLID 64205
			case ID_RESOURCEAVAIL_TEMPLATING:
			case ID_APT_BOOKING_ALARMS:
			case ID_SHOW_MOVEUP_LIST:
				//use FFA since that will never be disabled unless the license forbids it
				pCmdUI->Enable(m_enableViews & TB_FFA);
				break;

			case ID_ACTIVITIES_EYEGRAPH:
				if(IsRefractive()) 
					pCmdUI->Enable(true);
				else
					pCmdUI->Enable(false);
				break;
			case ID_PALM_PILOT:
				if(m_bIncomingCall)
					pCmdUI->Enable(true);
				else
					pCmdUI->Enable(false);
				break;
			case ID_ACTIVITIES_IMPORTCCD:
			case ID_ACTIVITIES_CREATECCD:
				// (a.walling 2009-05-21 12:46) - PLID 34318 - Same for creating a CCD
				// (a.walling 2009-05-14 10:23) - PLID 34243 - Disable this menu item if not licensed for EMR
				if (g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
					// (a.walling 2009-06-08 15:19) - PLID 34243 - Need to check user permissions
					if (CheckCurrentUserPermissions(bioPatientHistory, sptWrite, FALSE, 0, TRUE, TRUE)) {
						pCmdUI->Enable(TRUE);
					} else {
						pCmdUI->Enable(FALSE);
					}
				} else {
					pCmdUI->Enable(FALSE);
				}
				break;
			// (j.jones 2008-07-08 12:06) - PLID 24624 - added ability to run the patient summary
			case ID_PATIENT_SUMMARY: OnUpdatePatientSummary(pCmdUI); break;				
			case ID_SUPERBILL: OnUpdateSuperbill(pCmdUI); break;
			case ID_ENVELOPE: OnUpdateEnvelope(pCmdUI); break;
			case ID_EDUCATIONAL_MATERIALS: OnUpdateEducationalMaterials(pCmdUI); break;
			case ID_PRESCRIPTION: OnUpdatePrescription(pCmdUI); break;
			case ID_TOOLS_APPOINTMENTREMINDERSETTINGS: OnUpdateAppointmentReminderSettings(pCmdUI); break; // (c.haag 2010-06-14 11:31) - PLID 38018
			case ID_TOOLS_ANCHOR_SETTINGS: pCmdUI->Enable(IsCurrentUserAdministrator()); break;	// (j.armen 2013-08-27 14:24) - PLID 58034
			case ID_ACTIVITIES_TRACKSERIALIZEDITEMS: OnUpdateTrackSerializedItems(pCmdUI); break;
			case ID_ACTIVITIES_GENERATEBARCODES: OnUpdateGenerateBarcodes(pCmdUI); break;
			case ID_PATIENT_SERIALIZED_ITEMS: OnUpdatePatientSerializedItems(pCmdUI); break;
			case ID_VIEW_TOOLBARS_GUI: OnUpdateViewToolbarsGui(pCmdUI); break;
			case ID_NEXT_CONTACT_BTN: OnUpdateNextContactBtn(pCmdUI); break;
			case ID_PREV_CONTACT_BTN: OnUpdatePrevContactBtn(pCmdUI); break;
			case ID_BOOL_LOG_FILE: OnUpdateBoolLogFile(pCmdUI); break;
			case ID_TOOLS_ENABLEBARCODESCANNER: OnUpdateBarcodeID(pCmdUI); break;
			case ID_UNITED: OnUpdateUnitedID(pCmdUI); break;
			case ID_ACTIVITIES_UPDATEPASTPENDINGAPPTS: OnUpdatePastApptsID(pCmdUI); break;
			case ID_TOOLS_MSRSETTINGS: OnUpdateSwiperID(pCmdUI); break;
			case ID_TOOLS_ENABLECALLERID: OnUpdateCallerID(pCmdUI); break;
			case ID_TOOLS_PALMSETTINGS: OnUpdatePalmSettings(pCmdUI); break;
			case ID_PRACYAKKER_ENABLE: OnUpdatePracYakker(pCmdUI); break;
			case ID_TOOLS_RUNSAMPLECALLERID: OnUpdateTestCallerID(pCmdUI);  break;
			case ID_TOOLS_TWAIN_USELEGACY: OnUpdateTWAINUseLegacy(pCmdUI); break;
			case ID_TOOLS_TWAIN_SHOWTWAINUI: OnUpdateShowTWAINUI(pCmdUI); break;
			case ID_VIEW_SHOWIMAGESINGENERAL1: OnUpdateShowG1Images(pCmdUI); break;
			case ID_VIEW_SHOWMIRRORIMAGESINGENERAL1: OnUpdateShowMirrorImages(pCmdUI); break; //TES 11/17/2009 - PLID 35709
			case ID_VIEW_SHOWWARNINGDETAILSINGENERAL2: OnUpdateShowG2WarningDetails(pCmdUI); break;
			case ID_VIEW_SHOWPRIMARYIMAGEONLY: OnUpdateShowPrimaryImage(pCmdUI); break;
			case ID_TOOLS_AUTOCALLER: OnUpdateAutoCaller(pCmdUI);  break;
			case ID_TOOLS_NEXPDASETTINGS: OnUpdatePocketPC(pCmdUI); break;
			case IDM_TOOLS_NEXSYNC_SETTINGS: OnUpdateNexSyncSettings(pCmdUI); break; // (z.manning 2009-08-26 14:35) - PLID 35345
			//TES 8/24/2004: These buttons no longer exist.
			/*case ID_NEXT_DOC_BTN: OnUpdateNextDocBtn(pCmdUI); break;
			case ID_PREV_DOC_BTN: OnUpdatePrevDocBtn(pCmdUI); break;*/
				//pCmdUI->Enable(TRUE);
				//break;

			// These two are always available
			//(j.anspach 06-09-2005 12:40 PLID 16681) - Removing What's New? from all the menus.
			//case ID_HELP_WHATSNEW:
			case ID_HELP_LICENSEAGREEMENT:
				pCmdUI->Enable(TRUE);
				break;
				// (a.pietrafesa 2015-06-22) - PLID 65910 - View Log File in Help menu
			case ID_HELP_VIEWLOGFILE:
				pCmdUI->Enable(TRUE);
				break;

			case ID_FILE_PRINT:
			case ID_FILE_PRINT_PREVIEW:
			case IDM_UPDATE_VIEW: // (z.manning 2010-07-16 17:41) - PLID 39222
				// Allow the current frame/view to handle its own print/preview buttons and menue items
				return CNxMDIFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
				break;
			//TES 4/1/2008 - PLID 29509 - Due to the combination of a bug in MFC, and the fact that these IDCs were
			// defined as values larger than a WORD, these cases were never getting hit.  That bug is resolved in VS 2008,
			// so I'm commenting out these cases to keep the behavior consistent in VS 2008.
			/*case IDC_ACTIVE_TOOLBAR://don't change from MFC framework
			case IDC_PROSPECT_SEARCH:
			case IDC_PATIENTS_SEARCH:
			case IDC_ALL_SEARCH:
			case IDC_ALL_TOOLBAR:
				break;*/
			case ID_MERGE_ALL_FIELDS:
			case ID_SHOW_TEMPLATE_NAMES:
			case ID_SHOW_SCHEDULED_PATIENTS:
			case ID_SHOW_PATIENT_WARNING:
			case ID_VIEW_SHOW_APPT_LOCATIONS:
			case ID_VIEW_QUICK_SCHEDULE_NAVIGATION:
			case ID_VIEW_SHOWPATIENTINFORMATION:
			case ID_VIEW_SHOW_APPT_RESOURCES: 
				// Todo: It seems to me (RAC) that this should be the default instead of pCmdUI->Enable();  What do you really want here, BVB?
				//BVB - windows disables by default, we want to do the opposite
				return CNxMDIFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
				break;
			case ID_SHOW_PREFERENCES:
				//TES 6/23/2009 - PLID 34155 - Changed the View permission to Read
				if(GetCurrentUserPermissions(bioPreferences) & SPT__R_________ANDPASS) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				//TES 3/2/2015 - PLID 64734 - In the handling for ID_ENABLE_DEBUG_MODE, we remove the menu if we're not currently the NexTech Technical Support user.
				// However, if the user then switches to the NTS user, ID_ENABLE_DEBUG_MODE is gone, so it'll never get the UPDATE_COMMAND_UI. So, let's check here,
				// if we are the NTS user, and the Enable Debug Mode menu is not present, let's add it. We always want it as the second-to-last entry in the Tools menu.
				//TES 3/12/2015 - PLID 64734 - pCmdUI doesn't always come with a pointer to the menu
				if (GetCurrentUserID() == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID && pCmdUI->m_pMenu != NULL) {
					//TES 3/2/2015 - PLID 64734 - Is the menu here already?
					int nCount = pCmdUI->m_pMenu->GetMenuItemCount();
					bool bFound = false;
					for (int i = 0; i < nCount && !bFound; i++) {
						if (pCmdUI->m_pMenu->GetMenuItemID(i) == ID_ENABLE_DEBUG_MODE) {
							bFound = true;
						}
					}
					if (!bFound) {
						//TES 3/2/2015 - PLID 64734 - Let's add it, checked and enabled just as it would be in the ID_ENABLE_DEBUG_MODE handler elsewhere in this function
						UINT nFlags = MF_BYPOSITION;
						extern CPracticeApp theApp;
						if (theApp.GetDebugModeEnabled()) {
							nFlags |= MF_CHECKED;
							//TES 3/2/2015 - PLID 64736 - Disable if we've changed from off to on in this session, but the debug mode is still not active. We'll have told the 
							// user that it will take effect when they restart
							if (!theApp.GetDebugModeActive() && theApp.GetDebugModeChangedThisSession()) {
								nFlags |= MF_DISABLED;
							}
						}
						else {
							//TES 3/2/2015 - PLID 64737 - Disable if we've changed from on to off in this session, but the debug mode is still active. We'll have told the 
							// user that it will take effect when they restart
							if (theApp.GetDebugModeActive() && theApp.GetDebugModeChangedThisSession()) {
								nFlags += MF_DISABLED;
							}
						}
						pCmdUI->m_pMenu->InsertMenu(nCount - 1, nFlags, ID_ENABLE_DEBUG_MODE, "Enable Debug Mode");
					}
				}
				break;

			case ID_CHANGE_PASSWORD:
				if(GetCurrentUserPermissions(bioEditSelfPassword) & SPT___W________ANDPASS) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;
			case ID_VIEW_CURRENTALERTS:
				{
					if(m_dlgAlert.GetAlertCount() > 0)
						pCmdUI->Enable(TRUE);
					else
						pCmdUI->Enable(FALSE);
				}
				break;
			//
			//NexSpa Licensed items
			case ID_ACTIVITIES_FINDGIFTCERTIFICATE:
				{
					// (e.lally 2009-06-08) PLID 34498
					BOOL bEnable = FALSE;
					//check license and disable if not allowed
					//DRT 6/27/2005 - PLID 16082 - Don't use the license here, it's a menu!  All the components should use the license when they are
					//	invoked.
					if(IsSpa(FALSE)){
						// (e.lally 2009-06-08) PLID 34498 - Get permissions - Read patient billing
						if(GetCurrentUserPermissions(bioPatientBilling) & (sptRead|sptReadWithPass)) {
							bEnable = TRUE;
						}
					}
					pCmdUI->Enable(bEnable);
				}
				break;
			case ID_ACTIVITIES_ADDGIFTCERTIFICATE:
				{
					// (e.lally 2009-06-08) PLID 34498
					BOOL bEnable = FALSE;
					//check license and disable if not allowed
					//DRT 6/27/2005 - PLID 16082 - Don't use the license here, it's a menu!  All the components should use the license when they are
					//	invoked.
					if(IsSpa(FALSE)){
						// (e.lally 2009-06-08) PLID 34498 - Get permissions - Create new bill
						if(GetCurrentUserPermissions(bioBill) & (sptCreate|sptCreateWithPass)) {
							bEnable = TRUE;
						}
					}
					pCmdUI->Enable(bEnable);
				}
				break;
			case ID_ACTIVITIES_NEXSPA_EDITCASHDRAWERSESSIONS:
				{
					// (e.lally 2009-06-08) PLID 34498
					BOOL bEnable = FALSE;
					//check license and disable if not allowed
					//DRT 6/27/2005 - PLID 16082 - Don't use the license here, it's a menu!  All the components should use the license when they are
					//	invoked.
					if(IsSpa(FALSE)){
						// (e.lally 2009-06-08) PLID 34498 - Get permissions - Write cash drawers
						if(GetCurrentUserPermissions(bioCashDrawers) & (sptWrite|sptWriteWithPass)) {
							bEnable = TRUE;
						}
					}
					pCmdUI->Enable(bEnable);
				}
				break;

			case ID_IMPORT_EMR_CONTENT:
				{
					if(GetCurrentUserPermissions(bioAdminEMR) & SPT__R_________ANDPASS) 
						pCmdUI->Enable(TRUE);
					else
						pCmdUI->Enable(FALSE);
				}
				break;

			case ID_TOOLS_IMPORTEMRSTANDARDCONTENT:
				// (z.manning 2009-04-09 11:05) - PLID 33934
				OnUpdateImportEmrStandardContentMenu(pCmdUI);
				break;

			// (z.manning, 07/24/2007) - PLID 26370 - Disable the menu option if they don't have permission for the NexForms importer.
			case ID_TOOLS_IMPORT_NEXFORMS_CONTENT:
				{
					if(GetCurrentUserPermissions(bioAdminImportNexForms) & (sptView|sptViewWithPass)) {
						pCmdUI->Enable(TRUE);
					}
					else {
						pCmdUI->Enable(FALSE);
					}
				}
				break;

			//DRT 11/3/2008 - PLID 31789 - Disable if no license
			case ID_TOOLS_IMPORTNXCYCLEOFCARECONTENT:
				{
					if(g_pLicense->CheckForLicense(CLicense::lcCycleOfCare, CLicense::cflrSilent, TRUE)) {
						pCmdUI->Enable(TRUE);
					}
					else {
						pCmdUI->Enable(FALSE);
					}
				}
				break;
				
			case ID_MNU_SSRS_SETUP:
			{
				if (IsCurrentUserAdministrator()) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
			}
			break;
			
			case ID_SHOW_ACTIVE_EMR:
			case ID_LOCK_EMR:
			case ID_EMR_PATIENTS_TO_BE_BILLED:
			case ID_MENU_EMRSEARCH:
			case ID_MENU_EMRSEENTODAY:
			case ID_MENU_EMRWRITABLE: // (a.walling 2008-06-26 16:01) - PLID 30531 - Writable EMN/Template review dialog
			case ID_EMR_GRAPHPATIENTEMRDATA:	// (d.thompson 2009-05-29) - PLID 28486 - Graph Options dialog
			case ID_ACTIVITIES_CONFIGURE_CLINICAL_SUPPORT_RULES: // (j.gruber 2010-02-23 16:09) - PLID 37509
			case ID_ACTIVITIES_CONFIGUREEMNCHARGESANDDIAGNOSISCODESBYEMNREPORT: // (j.gruber 2010-03-09 09:52) - PLID 37660
			case ID_VIEW_CDS_INTERVENTIONS: //TES 1/13/2014 - PLID 59871
				{
					// (a.walling 2006-12-18 12:12) - PLID 22682 - Disable these menu items if user does not have EMR Read permission
					// (j.armen 2012-05-31 14:39) - PLID 50688 - Allow these menu items if they have either EMR or EMR Cosmetic
					lc = CLicense::lcNexEMR;
					BOOL bEMRLicensed = g_pLicense->CheckForLicense(CLicense::lcNexEMR, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound);

					if(!bEMRLicensed && !bLicenseErrorsFound)
					{
						lc = CLicense::lcNexEMRCosmetic;
						bEMRLicensed = g_pLicense->CheckForLicense(CLicense::lcNexEMRCosmetic, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound);
					}

					BOOL bEMRPermission = CheckCurrentUserPermissions(bioPatientEMR, sptRead, FALSE, 0, TRUE, TRUE); // assume password is known
					
					if(bEMRLicensed && bEMRPermission) {
						//TES 11/10/2013 - PLID 59400 - CDS Rules have an extra permission to check here
						if(nID != ID_ACTIVITIES_CONFIGURE_CLINICAL_SUPPORT_RULES || GetCurrentUserPermissions(bioCDSRules) & (sptWrite|sptWriteWithPass)) {
							pCmdUI->Enable(TRUE);
						}
						else {
							pCmdUI->Enable(FALSE);
						}
					}
					else {
						pCmdUI->Enable(FALSE);
					}
				}
				break;
			case ID_ACTIVITIES_BOLDSETTINGS:
				// (j.gruber 2010-06-04 15:16) - PLID 38935 - Added BOLD Licensing
				lc = CLicense::lcBold;
				pCmdUI->Enable(g_pLicense->CheckForLicense(lc, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound));
			break;

			
			// (b.spivey, September 10, 2012) - PLID 52568 - Uses the EMR license for OMR stuff. 
			case ID_ACTIVITIES_OMRFORMEDITING:
			case ID_ACTIVITIES_OMRPROCESSING:
			case ID_MANAGE_EMRPROVIDERS:
				{
					// (a.walling 2006-12-19 12:58) - PLID 23397 - Only enable the configure EMR Provider license dialog menu item 
					//		if they are licensed for nexemr at least.
					// (j.armen 2012-05-31 14:39) - PLID 50688 - Allow these menu items if they have either EMR or EMR Cosmetic
					lc = CLicense::lcNexEMR;

					BOOL bEMRLicensed = g_pLicense->CheckForLicense(CLicense::lcNexEMR, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound);

					if(!bEMRLicensed && !bLicenseErrorsFound)
					{
						lc = CLicense::lcNexEMRCosmetic;
						bEMRLicensed = g_pLicense->CheckForLicense(CLicense::lcNexEMRCosmetic, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound);
					}
					
					pCmdUI->Enable(bEMRLicensed);
				}
				break;

			case ID_MANAGE_NUANCE_USERS: // (z.manning 2013-02-04 11:15) - PLID 55001
				{
					int nNuanceUsersAllowed = g_pLicense->GetNuanceUserCountAllowed();
					pCmdUI->Enable(nNuanceUsersAllowed > 0);
				}
				break;

			case ID_MANAGE_PORTAL_PROVIDERS: // (z.manning 2015-06-18 15:18) - PLID 66280
				{
					int nPortalProvidersAllowed = g_pLicense->GetPortalProviderCountAllowed();
					pCmdUI->Enable(nPortalProvidersAllowed > 0);
				}
				break;

			// (j.anspach 04-28-2005 16:48 - PLID 16377) - This should check for marketing permissions since
			// it involves financial information with the office (amount of prepayments collected).
			case ID_ACTIVITIES_SCHEDULING_PRODUCTIVITY:
				{
					lc = CLicense::lcMarket;
					if(GetCurrentUserPermissions(bioMarketingModule))
						pCmdUI->Enable(g_pLicense->CheckForLicense(CLicense::lcMarket, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound));
					else
						pCmdUI->Enable(FALSE);
				}
				break;


			case ID_TOOLS_HL7SETTINGS:
				{
					lc = lc = CLicense::lcHL7;
					//TES 5/27/2009 - PLID 34282 - Check permissions
					pCmdUI->Enable((GetCurrentUserPermissions(bioHL7Settings) & SPT_V__________ANDPASS) && g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound));
				}
				break;
			case ID_TOOLS_PROCESS_HL7_MESSAGES:
				{
					lc = CLicense::lcHL7;
					//TES 6/30/2009 - PLID 34283 - Check permissions
					pCmdUI->Enable((GetCurrentUserPermissions(bioHL7BatchDlg) & SPT__R_________ANDPASS) && g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound));
				}
				break;
			// (b.savon 2011-10-04 11:22) - PLID 39890 - Give the same permissions as HL7 Settings.
			case ID_TOOLS_HL7_UNLINKHL73RDPARTYIDS:
				{
					lc = CLicense::lcHL7;
					//TES 5/27/2009 - PLID 34282 - Check permissions
					pCmdUI->Enable((GetCurrentUserPermissions(bioHL7Settings) & SPT_V__________ANDPASS) && g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound));

				}
				break;
			// (j.jones 2008-04-14 09:29) - PLID 29596 - this dialog was removed in lieu
			// of the new HL7 tab in the Financial module
			/*
			case ID_MODULES_IMPORTFROMHL7:
				{
					lc = lc = CLicense::lcHL7;
					pCmdUI->Enable(g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound));
				}
				break;
			*/
			case ID_MODULES_EXPORTTOHL7:
				{
					lc = lc = CLicense::lcHL7;
					//TES 5/18/2009 - PLID 34282 - Check permissions
					pCmdUI->Enable((GetCurrentUserPermissions(bioHL7Patients) & SPT______0_____ANDPASS) && g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound));
				}
				break;
			case ID_MODULES_EXPORTTOHL7_APPTS: // (z.manning 2008-07-21 09:39) - PLID 30782
				{
					lc = lc = CLicense::lcHL7;
					//TES 5/18/2009 - PLID 34282 - Check permissions
					pCmdUI->Enable((GetCurrentUserPermissions(bioHL7Appointments) & SPT______0_____ANDPASS) && g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound));
				}
				break;
			case ID_MODULES_EXPORTTOHL7_EMN_BILLS: //TES 7/10/2009 - PLID 34845
				{
					lc = lc = CLicense::lcHL7;
					//TES 5/18/2009 - PLID 34282 - Check permissions
					pCmdUI->Enable((GetCurrentUserPermissions(bioHL7EmnBills) & SPT______0_____ANDPASS) && g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound) && g_pLicense->HasEMR(CLicense::cflrSilent) == 2);
				}
				break;
			case ID_MODULES_EXPORTTOHL7_LABRESULTS:
				{
					lc = lc = CLicense::lcHL7;
					// (d.singleton 2012-12-20 10:19) - PLID 53041 check permission	
					pCmdUI->Enable((GetRemotePropertyInt("HL7ExportLabResults", 0, 0, "<none>", true)) && g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound) &&  g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound));
				}
				break;
			case ID_MODULES_EXPORTTOHL7_LOCKEDEMNS:
				{
					lc = lc = CLicense::lcHL7;
					// (d.singleton 2012-12-20 10:19) - PLID 53041 check permission
					pCmdUI->Enable((GetRemotePropertyInt("HL7ExportLockedEMNs", 0, 0, "<none>", true)) && g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound) && g_pLicense->HasEMR(CLicense::cflrSilent) == 2);
				}
				break;
			case ID_MODULES_IMPORTFROMNEXWEB:
				{
					// (j.gruber 2009-12-01 09:04) - PLID 36455 - change to new licensing
					lc = lc = CLicense::lcNexWebLeads;
					pCmdUI->Enable(g_pLicense->CheckForLicense(CLicense::lcNexWebLeads, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound));
				}
				// (e.lally 2009-06-24) PLID 34720 - Break!
				break;
			case ID_EMR_FINALIZEPATIENTEMNSFROMNEXWEB:		// (d.thompson 2009-11-04) - PLID 35811 - Disable based on NexWeb license
				{
					// (j.gruber 2009-12-01 09:04) - PLID 36455 - change to new licensing
					lc = lc = CLicense::lcNexWebPortal;
					pCmdUI->Enable(g_pLicense->CheckForLicense(CLicense::lcNexWebPortal, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound));
				}
				// (e.lally 2009-06-24) PLID 34720 - Break!
				break;
			case ID_BATCH_MERGE:
				{
					// (e.lally 2009-06-08) PLID 34498 - Get permissions - Write patient tracking
					BOOL bEnable = FALSE;
					lc = CLicense::lcNexTrak;
					//TES 12/21/2006 - PLID 19278 - This is only viable if they have the NexTrack license.
					if(g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound) &&
					  (GetCurrentUserPermissions(bioPatientTracking) & (sptWrite|sptWriteWithPass))) {				
							bEnable = TRUE;
					}
					pCmdUI->Enable(bEnable);
				}
				break;

			case IDM_CREDIT_CARD_PROCESSING_SETUP:
				{
					// (d.thompson 2010-09-02) - PLID 40371 - This menu option displays if any cc license is enabled.  The 
					//	OnClick handler determines which one is actually licensed, and pops up the appropriate dialog.
					//In the process, I disabled the "recheck for license errors".  None of the code following this (for several years)
					//	seems to implement it, and we've had no issues.

					// (c.haag 2015-07-30) - PLID 67190 - Removed credit card processing setup
					if (IsICCPEnabled()) 
					{
						RemoveEntryFromSubMenu(GetMainFrame()->GetMenu(), "Tools", IDM_CREDIT_CARD_PROCESSING_SETUP, FALSE);
					}
					else 
					{
						// (a.walling 2007-08-03 09:36) - PLID 26899 - Credit card processing license check
						pCmdUI->Enable(g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent));
					}
				}
				break;

			// (d.thompson 2009-04-16) - PLID 33973 - Removed this menu option
			/*case IDM_CC_BATCH_PROCESSING:
				{
					lc = CLicense::lcCCProcessing;
					// (a.walling 2007-08-03 09:41) - PLID 26899 - Credit card processing license check
					// (a.walling 2007-08-03 17:25) - PLID 26922 - And check permissions
					// check silently, assume they know the password, since this will be called often.
					BOOL bPermitted = CheckCurrentUserPermissions(bioCCBatchDialog, sptView, FALSE, 0, TRUE, TRUE);
					BOOL bLicensed = g_pLicense->CheckForLicense(CLicense::lcCCProcessing, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound);
					pCmdUI->Enable(bPermitted && bLicensed);
				}
				break;*/

			// (a.walling 2008-02-18 16:19) - PLID 28946 - disable these menu items if unlicensed
			case ID_ACTIVITIES_CONSIGNMENT_PURCHASEITEMSFROMCONSIGNMENT:
			case ID_ACTIVITIES_CONSIGNMENT_TRANSFERITEMSTOCONSIGNMENT:
				{
					//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
					// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
					pCmdUI->Enable(g_pLicense->HasCandAModule(CLicense::cflrSilent));
				}
				break;
			
			case ID_APPTS_WO_ALLOCATIONS:
				{
					//TES 6/16/2008 - PLID 30394 - Check the license.
					// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
					// (e.lally 2009-06-08) PLID 34497 - Permission - Read Inventory Allocations Tab
					BOOL bEnable = FALSE;
					if(g_pLicense->HasCandAModule(CLicense::cflrSilent) &&
						(GetCurrentUserPermissions(bioInventoryAllocation) & (sptRead|sptReadWithPass))) {
							bEnable = TRUE;
					}
					pCmdUI->Enable(bEnable);
				}
				break;

			// (a.walling 2008-07-16 17:18) - PLID 30751 - Access transcription parsing
			case IDM_PARSE_TRANSCRIPTIONS:
				{
					// (a.walling 2008-07-17 14:33) - PLID 30768 - Safety check for license
					// (e.lally 2009-06-08) PLID 34497 - Permission - History tab write
					BOOL bEnable = FALSE;
					if(g_pLicense->HasTranscriptions() &&
						(GetCurrentUserPermissions(bioPatientHistory) & (sptWrite|sptWriteWithPass))) {
							bEnable = TRUE;
					}
					pCmdUI->Enable(bEnable);
				}
				break;
				//DRT 8/1/2008 - PLID 30915 - I left this here to comment the reasoning.  Since we've
				//	decided to put some "marketing" info out for modules, I wanted to do the same here.
				//	So the configure button is always enabled, and if you click it, there is code in 
				//	the "on..." handler which will give them a nice message about calling sales.
				//(e.lally 2011-03-18) PLID 42767 - changed ID to have "outgoing"
			case ID_TOOLS_FAXING_CONFIGUREOUTGOINGONLINEFAXSERVICE:
				{
					pCmdUI->Enable(TRUE);
				}
				break;
				//(e.lally 2011-03-18) PLID 42767 - Left this here to follow suite with the configure online fax service option
			case ID_TOOLS_FAXING_CONFIGUREINCOMINGONLINEFAXSERVICE:
				{
					pCmdUI->Enable(TRUE);
				}
				break;
				//DRT 8/1/2008 - PLID 30915
			case ID_TOOLS_FAXING_SENDFAX:
				{
					if(g_pLicense->CheckForLicense(CLicense::lcFaxing, CLicense::cflrSilent)) {
						pCmdUI->Enable(TRUE);
					}
					else {
						pCmdUI->Enable(FALSE);
					}
				}
				break;

			// (j.jones 2009-01-07 13:46) - PLID 26141 - added inventory reconciliation
			case ID_ACTIVITIES_RECONCILE_INV:
				{
					//use the regular Inventory license, not Adv. Inventory
					// (j.jones 2009-07-09 12:53) - PLID 34826 - ensure we have the ability to view inventory
					if(g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent)
						&& (GetCurrentUserPermissions(bioInvItem) & (sptRead|sptReadWithPass))) {
						pCmdUI->Enable(TRUE);
					}
					else {
						pCmdUI->Enable(FALSE);
					}
				}
				break;

			// (j.jones 2009-03-24 14:44) - PLID 33652 - protected e-prescribing settings with the NewCrop license
			/*case ID_TOOLS_ELECTRONICPRESCRIBINGSETTINGS:
				{
					//use the NewCrop license
					//TES 4/2/2009 - PLID 33376 - Actually, we want to just check whether they have ANY e-Prescribing
					// licenses, then the handler function will figure out which settings to display.
					//TES 4/7/2009 - PLID 33882 - Changed this function to return an enum
					if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) != CLicense::eptNone) {
						pCmdUI->Enable(TRUE);
					}
					else {
						pCmdUI->Enable(FALSE);
					}
				}
				break;*/
			//TES 4/10/2009 - PLID 33889 - Replaced the E-Prescribing Settings with menu options broken down by each type.
			// (j.gruber 2009-05-15 10:33) - PLID 28541 - new crop renewal requests
			case ID_NEWCROP_SETTINGS:
				{
					// (e.lally 2009-06-10) PLID 34529 - leave as just a check for licensing
					if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptNewCrop) {
						pCmdUI->Enable(TRUE);
					}
					else {
						pCmdUI->Enable(FALSE);
					}
				}
				break;
			case ID_ELECTRONICPRESCRIBING_VIEW_NEWCROP_RENEWAL_REQUESTS:
				{
					// (e.lally 2009-06-10) PLID 34529 - Get permissions - Read patient medication
					BOOL bEnable = FALSE;
					if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptNewCrop &&
						(GetCurrentUserPermissions(bioPatientMedication) & (sptRead|sptReadWithPass))){
						bEnable = TRUE;
					}
					pCmdUI->Enable(bEnable);
				}
				break;

            
				

			case ID_SURESCRIPTS_SETTINGS:
				{
					// (e.lally 2009-06-10) PLID 34529 - leave as just a check for licensing
					if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts && IsCurrentUserAdministrator()) {
						pCmdUI->Enable(TRUE);
					}
					else {
						pCmdUI->Enable(FALSE);
					}
				}
				break;
				
			case ID_ELECTRONICPRESCRIBING_REGISTERNEXERXCLIENTANDPRESCRIBERS:
			{
				// (b.savon 2012-11-01 11:40) - PLID 53559 - Only let the admin user modify these settings
				BOOL bEnable = FALSE;
				if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts &&
					IsCurrentUserAdministrator()){
					bEnable = TRUE;
				}
				pCmdUI->Enable(bEnable);
			}
			break;

			// (e.lally 2009-06-08) PLID 34497 - Get Permissions - Read patient notes tab
			case ID_SEARCH_NOTES:
				if(GetCurrentUserPermissions(bioPatientNotes) & (sptRead|sptReadWithPass)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;
			
			// (e.lally 2009-06-08) PLID 34497 - Get Permissions - Read Payments
			case ID_ACTIVITIES_SEARCHCHECKCCNUMBERS:
				if(GetCurrentUserPermissions(bioPayment) & (sptRead|sptReadWithPass)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;
			
			// (e.lally 2009-06-08) PLID 34497 - Get Permissions - Patient history tab write
			case ID_TOOLS_SCANMULTIPLEDOCUMENTS:
				if(GetCurrentUserPermissions(bioPatientHistory) & (sptWrite|sptWriteWithPass)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (e.lally 2009-06-08) PLID 34497 - Get Permissions - Read patient billing tab
			case ID_SEARCH_ALLOWABLES:
				if(GetCurrentUserPermissions(bioPatientBilling) & (sptRead|sptReadWithPass)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (e.lally 2009-06-08) PLID 34498 - Get permissions - View scheduler module
			case ID_SUPERBILLALL:
			case ID_ACTIVITIES_REPRINTSUPERBILLS:
				if(GetCurrentUserPermissions(bioSchedulerModule) & (sptView|sptViewWithPass)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (e.lally 2009-06-10) PLID 34529 - Get permissions - View patients module
			case ID_TRANSFER_TODOS:
				if(GetCurrentUserPermissions(bioPatientsModule) & (sptView|sptViewWithPass)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (e.lally 2009-06-10) PLID 34529 - Get permissions - view patients module
			case ID_EXPORT_TO_MEDINOTES:
				if(GetCurrentUserPermissions(bioPatientsModule) & (sptView|sptViewWithPass)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (e.lally 2009-06-10) PLID 34529 - Get permissions - view Financial Module
			case ID_PAPERBATCH:
				if(GetCurrentUserPermissions(bioFinancialModule) & (sptView|sptViewWithPass)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (e.lally 2009-06-10) PLID 34529 - Get permissions - view Financial Module
			case ID_BATCH_PAYS:
				if(GetCurrentUserPermissions(bioFinancialModule) & (sptView|sptViewWithPass)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (e.lally 2009-06-10) PLID 34529 - Get permissions - Edit Payment
			case ID_POST_TO_BILL_ID:
				// (j.jones 2010-10-18 11:58) - PLID 39950 - remove this option if they don't have billing
				if(!g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) {

					CMenu *pRootMenu = GetMainFrame()->GetMenu();
					if (pRootMenu && IsMenu(pRootMenu->m_hMenu)) {
						// Find the Activities submenu
						BOOL bFound = FALSE;
						// (a.walling 2014-04-28 13:13) - PLID 61945 - VS2013 - GetMenuItemCount returns int now
						for (int i = 0; i < pRootMenu->GetMenuItemCount() && !bFound; i++) {
							CString strLabel;
							pRootMenu->GetMenuString(i, strLabel, MF_BYPOSITION);
							if (strLabel.Find("Activities") != -1) {
								// Now, find the given entry (we have to loop through and find it, because
								// we want to remove the preceding separator.
								CMenu *pActivitiesMenu = pRootMenu->GetSubMenu(i);
								bFound = TRUE;
								RemoveEntryFromSubMenu(pActivitiesMenu, "Financial", ID_POST_TO_BILL_ID, FALSE);
							}
						}
					}
				}
				else if(GetCurrentUserPermissions(bioPayment) & (sptWrite|sptWriteWithPass)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (e.lally 2009-06-10) PLID 34529 - Get permissions - Write Apply Superbills
			case ID_ACTIVITIES_APPLYSUPERBILLS:
				if(GetCurrentUserPermissions(bioApplySuperbills) & (sptWrite|sptWriteWithPass)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (e.lally 2009-06-10) PLID 34529 - Get permissions - Write Void Superbills
			case ID_ACTIVITIES_VOIDSUPERBILLS:
				if(GetCurrentUserPermissions(bioVoidSuperbills) & (sptWrite|sptWriteWithPass)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (e.lally 2009-06-10) PLID 34529 - Get permissions - view Scheduler Module
			case ID_ROOM_MANAGER:
				if(GetCurrentUserPermissions(bioSchedulerModule) & (sptView|sptViewWithPass)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (a.walling 2014-12-22 12:07) - PLID 64369 - Rescheduling Queue command in menus
			case ID_RESCHEDULING_QUEUE:
				if(GetCurrentUserPermissions(bioSchedulerModule) & (sptView|sptViewWithPass)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (z.manning 2009-07-21 18:32) - PLID 34857 - License off new CCHIT features as EMR.
			// (j.gruber 2013-11-05 12:25) - PLID 59323 - add CCDA Export
			case IDM_PATIENT_ORDER_SETS:
			case ID_ACTIVITIES_ORDER_SET_TEMPLATES:
			case IDM_REFERRAL_ORDERS:
			case ID_ACTIVITIES_IMMUNIZATIONS:
			case ID_ACTIVITIES_ADVANCE_DIRECTIVES:
			case ID_ACTIVITIES_EXPORT_SUMMARY_OF_CARE:
				
				if(g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

				// (j.gruber 2013-10-08 13:14) - PLID 59062
				case ID_TOOLS_CCDA_SETTINGS:
				break;
			// (j.jones 2009-12-28 08:59) - PLID 32150 - added ability to delete unused service codes
			case ID_DELETE_UNUSED_SERVICE_CODES:

				if(GetCurrentUserPermissions(bioServiceCodes) & SPT_____D______ANDPASS) {
					pCmdUI->Enable(TRUE);
				}
				else{
					pCmdUI->Enable(FALSE);
				}				
				break;

			// (j.jones 2012-03-27 08:59) - PLID 45752 - added ability to delete unused diagnosis codes
			case ID_DELETE_UNUSED_DIAG_CODES:
				if(GetCurrentUserPermissions(bioDiagCodes) & SPT_____D______ANDPASS) {
					pCmdUI->Enable(TRUE);
				}
				else{
					pCmdUI->Enable(FALSE);
				}				
				break;

			// (j.jones 2012-03-27 09:20) - PLID 47448 - added ability to delete unused modifiers
			case ID_DELETE_UNUSED_MODIFIERS:
				//this should not be useable if the Alberta preference is enabled
				if(!UseAlbertaHLINK() && GetCurrentUserPermissions(bioServiceModifiers) & SPT_____D______ANDPASS) {
					pCmdUI->Enable(TRUE);
				}
				else{
					pCmdUI->Enable(FALSE);
				}
				break;

			// (b.cardillo 2010-02-05 12:22) - PLID 16649 - permission for deactivate workstation
			case ID_DEACTIVATE_WORKSTATIONS:
				if (CheckCurrentUserPermissions(bioLicenseDeactivateWorkstation, sptView, 0, 0, TRUE, TRUE)) {
					pCmdUI->Enable(TRUE);
				} else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (b.cardillo 2010-02-05 12:22) - PLID 16649 - permission for updating license
			case ID_UPDATE_LICENSE:
				if (CheckCurrentUserPermissions(bioLicenseEnterUpdateCode, sptView, 0, 0, TRUE, TRUE)) {
					pCmdUI->Enable(TRUE);
				} else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (d.thompson 2010-03-15) - PLID 37729 - Purge cardholder data dialog
			case ID_FINANCIAL_PURGECREDITCARDHOLDERDATA:
				if((GetCurrentUserPermissions(bioPurgeCardholderData) & (sptWrite|sptWriteWithPass))) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;
			
			// (a.walling 2010-03-15 14:10) - PLID 37755 - Manual encryption key update
			case ID_FINANCIAL_UPDATEENCRYPTIONKEY:
				if((GetCurrentUserPermissions(bioUpdateEncryptionKey) & (sptWrite|sptWriteWithPass))) {
					pCmdUI->Enable(TRUE);
				} else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (a.vengrofski 2010-04-20 15:10) - PLID <38205>
			case ID_ACTIVITIES_MANAGEQUICKBOOKSIDS:
				{
					if(!IsNexTechInternal()) {//Should not be seen outside of internal.
						RemoveEntryFromSubMenu(GetMainFrame()->GetMenu(), "Activities", ID_ACTIVITIES_MANAGEQUICKBOOKSIDS, FALSE);
					} else {
						pCmdUI->Enable(TRUE);//It seems that the item is greyed out by default, make it clickable.
					}
				}
				break;

				// (j.dinatale 2010-12-22) - PLID 39226
			case ID_BILLINGTABCOLUMNSET:
				{
					if(GetCurrentUserPermissions(bioPatientBilling) & sptRead){
						pCmdUI->Enable(TRUE);
					}else{
						pCmdUI->Enable(FALSE);
					}
				}
				break;

			// (j.jones 2010-12-22 10:56) - PLID 41911 - added financial close ability,
			// which has a password-based permission only
			case ID_ACTIVITIES_PERFORMAFINANCIALCLOSE:
				if(GetCurrentUserPermissions(bioPerformFinancialClose) & sptWriteWithPass) {
					pCmdUI->Enable(TRUE);
				}else{
					pCmdUI->Enable(FALSE);
				}
				break;

			// (j.jones 2011-03-21 15:33) - PLID 42917 - these menu items just open
			// shared folders on the server, so no permission is needed, just disable
			// them if they don't have the e-remit license
			case ID_VIEWOLDEOBFILES:
			case ID_VIEWOLDEOBWARNINGSFILES:
				if(g_pLicense->CheckForLicense(CLicense::lcEEligibility, CLicense::cflrSilent)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (j.jones 2011-03-28 14:24) - PLID 43012 - added ability to mass-configure
			// the billable status of service codes
			case ID_FINANCIAL_NONBILLABLE_CODES:
				if(GetCurrentUserPermissions(bioServiceCodes) & (sptWrite|sptWriteWithPass)) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

			// (z.manning 2015-07-22 15:36) - PLID 67241 - Disable this if no ICCP license
			case ID_MANAGE_PAYMENT_PROFILES:
				if (IsICCPEnabled()) {
					pCmdUI->Enable(TRUE);
				}
				else {
					pCmdUI->Enable(FALSE);
				}
				break;

			//(j.camacho 2016-02-01) plid 68010
			// Don't bother enabling the IntelleChart Encounters To Be Billed option, if HL7 isn't enabled
			case ID_ACTIVITIES_MDIHL7ACTIVITYITEM:
				
				if (IsIntellechartToBeBilledEnabled() && g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent))
				{
					pCmdUI->Enable(TRUE);
				}
				else
				{
					pCmdUI->Enable(FALSE);
				}

				break;

			// (d.singleton 5/24/2011 - 12:02:00) - PLID 43340 - Create "Export EMR Content option in Admin > tools menu,  only for internal use so use ConfigRT value in order to show or hide.
			case ID_EXPORT_EMR_CONTENT: 
				{
					long nShowExportEmrContent = GetRemotePropertyInt("ShowExportEmrContent", 0, 0, "<None>", true);
					if(nShowExportEmrContent == 0)
					{
						RemoveEntryFromSubMenu(GetMainFrame()->GetMenu(), "Tools", ID_EXPORT_EMR_CONTENT, false);
					}
				}
				break;
			
			// (j.luckoski 2012-04-10 17:15) - PLID 49491 - Remove the Specialty Travel editor when not internal.
			case ID_ACTIVITIES_EDITSPECIALTYTRAINING:
				{
					if(!IsNexTechInternal()) {
						RemoveEntryFromSubMenu(GetMainFrame()->GetMenu(), "Activities", ID_ACTIVITIES_EDITSPECIALTYTRAINING, false);
					}
				}
				break;

			// (d.singleton 2012-04-26 12:14) - PLID 49086 - remove is they arent licenses for codecorrect
			case ID_FINANCIAL_CODECORRECTSETUP:
				{
					if(!g_pLicense->CheckForLicense(CLicense::lcCodeCorrect, CLicense::cflrSilent)) {
						RemoveEntryFromSubMenu(GetMainFrame()->GetMenu(), "Financial", ID_FINANCIAL_CODECORRECTSETUP, false);
					}
				}
				break;
			case ID_ACTIVITIES_CCHITREPORTS:
				{
					// (j.armen 2012-05-31 14:39) - PLID 50688 - Restrict access if the user has NexEMRCosmetic without NexEMR
					lc = CLicense::lcNexEMR;
					BOOL bEMRLicensed = g_pLicense->CheckForLicense(CLicense::lcNexEMR, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound);

					if(!bEMRLicensed && !bLicenseErrorsFound)
					{
						lc = CLicense::lcNexEMRCosmetic;
						bEMRLicensed = !g_pLicense->CheckForLicense(CLicense::lcNexEMRCosmetic, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound);
					}

					BOOL bEMRPermission = CheckCurrentUserPermissions(bioPatientEMR, sptRead, FALSE, 0, TRUE, TRUE); // assume password is known
					
					if(bEMRLicensed && bEMRPermission) 
						pCmdUI->Enable(TRUE);
					else 
						pCmdUI->Enable(FALSE);
				}
				break;
			// (j.dinatale 2012-10-31 12:08) - PLID 53502 - MU detailed report
			case ID_ACTIVITIES_CCHITDETAILED:
				{
					lc = CLicense::lcNexEMR;
					BOOL bEMRLicensed = g_pLicense->CheckForLicense(CLicense::lcNexEMR, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound);

					if(!bEMRLicensed && !bLicenseErrorsFound)
					{
						lc = CLicense::lcNexEMRCosmetic;
						bEMRLicensed = !g_pLicense->CheckForLicense(CLicense::lcNexEMRCosmetic, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound);
					}

					BOOL bEMRPermission = CheckCurrentUserPermissions(bioPatientEMR, sptRead, FALSE, 0, TRUE, TRUE); // assume password is known
					
					if(bEMRLicensed && bEMRPermission) 
						pCmdUI->Enable(TRUE);
					else 
						pCmdUI->Enable(FALSE);
				}
				break;
			// (j.fouts 2013-01-17 14:40) - PLID 53840 - Hide this if they are not licensed for NexERx
			case ID_ACTIVITIES_PRESCRIPTIONS_NEEDING_ATTENTION:
				{
					if(!SureScripts::IsEnabled())
					{
						RemoveEntryFromSubMenu(GetMainFrame()->GetMenu(), "Activities", ID_ACTIVITIES_PRESCRIPTIONS_NEEDING_ATTENTION, false);
					}
					else
					{
						// (b.savon 2014-01-14 07:44) - PLID 59628 - The permission for Rx Needing Attention is incorrectly
						// under the permission for "Edit Medication List". It should be read
						if(GetCurrentUserPermissions(bioPatientMedication) & (sptRead|sptReadWithPass))
						{
							pCmdUI->Enable(TRUE);
						}
						else
						{
							pCmdUI->Enable(FALSE);
						}
					}
				}
				break;
				//(r.wilson 7/29/2013) PLID 48684 - Menu is always enabled now
				case ID_MANAGE_EMRPROVIDER_LINKS:
				  {
						pCmdUI->Enable(TRUE);				  
				  }
				  break;

				// (j.jones 2013-08-01 13:21) - PLID 53317 - ensured we check permissions to enable/disable
				// finance charge features
				case ID_CALC_FINANCE_CHARGES:
					if(GetCurrentUserPermissions(bioFinanceCharges) & (sptWrite|sptWriteWithPass)) {
						pCmdUI->Enable(TRUE);
					}
					else {
						pCmdUI->Enable(FALSE);
					}
				break;
				case ID_UNDO_FINANCE_CHARGES:
					if(GetCurrentUserPermissions(bioFinanceCharges) & (sptDelete|sptDeleteWithPass)) {
						pCmdUI->Enable(TRUE);
					}
					else {
						pCmdUI->Enable(FALSE);
					}
				break;

				case ID_ENABLE_DEBUG_MODE:
					//TES 2/23/2015 - PLID 64734 - This menu option is only available for the built-in NexTech Technical Support user
					if (GetCurrentUserID() != BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID) {
						pCmdUI->m_pParentMenu->RemoveMenu(ID_ENABLE_DEBUG_MODE, MF_BYCOMMAND);
					}
					else {
						extern CPracticeApp theApp;
						if (theApp.GetDebugModeEnabled()) {
							pCmdUI->SetCheck(TRUE);
							//TES 3/2/2015 - PLID 64736 - Disable if we've changed from off to on in this session, but the debug mode is still not active. We'll have told the 
							// user that it will take effect when they restart
							pCmdUI->Enable(theApp.GetDebugModeActive() || !theApp.GetDebugModeChangedThisSession());
						}
						else {
							pCmdUI->SetCheck(FALSE);
							//TES 3/2/2015 - PLID 64737 - Disable if we've changed from on to off in this session, but the debug mode is still active. We'll have told the 
							// user that it will take effect when they restart
							pCmdUI->Enable(!theApp.GetDebugModeActive() || !theApp.GetDebugModeChangedThisSession());
						}
					}
					break;

				case IDM_ICCP_SETTINGS:
				{
					// (d.lange 2015-07-22 14:56) - PLID 67188
					lc = CLicense::lcICCP;
					if (g_pLicense->CheckForLicense(lc, CLicense::cflrSilent, TRUE, &bLicenseErrorsFound)) {
						pCmdUI->Enable(TRUE);
					}
					else {
						pCmdUI->Enable(FALSE);
					}
				}
				break;

			default://enable items by default, this is inconsistent with Microsoft, but disabling by default seems stupid to me -BVB
				pCmdUI->Enable();
		}
		if(bLicenseErrorsFound) {
			PostMessage(NXM_RECHECK_LICENSE, nID, lc);
		}
		return TRUE;
	}
	else return CNxMDIFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

// (<j.gruber> 2006-11-01 10:08) - PLID 23208 - added viewing webinars to help menu
void CMainFrame::OnViewWebinars() {

	// (j.gruber 2007-01-29 17:40) - PLID 24450 - make it auto-login with their license key
	CWaitCursor wc;

	const LPCTSTR caryLetters[] = {"00", "ba", "be", "bi", "bo", "bu", "ca", "ce", "ci", "co", "cu", "da", "de", "di", "do", "du", "fa", "fe", "fi", "fo", "fu", "ga", 
			"ge", "gi", "go", "gu", "ha", "he", "hi", "ho", "hu", "ja", "je", "ji", "jo", "ju", "ka", "ke", "ki", "ko", "ku", "la", "le", 
			"li", "lo", "lu", "ma", "me", "mi", "mo", "mu", "na", "ne", "ni", "no", "nu", "pa", "pe", "pi", "po", "pu", "qa", "qe", 
			"qi", "qo", "ra", "re", "ri", "ro", "ru", "sa", "se", "si", "so", "su", "ta", "te", "ti", "to", "tu", "va", "ve", "vi", "vo", 
			"vu", "wa", "we", "wi", "wo", "wu", "xa", "xe", "xi", "xo", "xu", "ya", "ye", "yi", "yo", "yu", "za", "ze", "zi", "zo", "zu",
			"ab", "ac", "ad", "ae", "af", "ag", "ah", "aj", "ak", "al", "am", "an", "ap", "aq", "ar", "as", "at", "av", "aw", "ax", "ay", 
			"az", "eb", "ec", "ed", "ef", "eg", "eh", "ej", "ek", "el", "em", "en", "ep", "eq", "er", "es", "et", "ev", "ew", "ex", "ey", 
			"ez", "ib", "ic", "id", "if", "ig", "ih", "ij", "ik", "il", "im", "in", "ip", "iq", "ir", "is", "it", "iv", "iw", "ix", "iy",
			"iz", "ob", "oc", "od", "of", "og", "oh", "oj", "ok", "ol", "om", "on", "op", "oq", "or", "os", "ot", "ov", "ow", "ox", "oy", 
			"oz", "ub", "uc", "ud", "uf", "ug", "uh", "uj", "uk", "ul", "um", "un", "up", "uq", "ur", "us", "ut", "uv", "uw", "ux", "uy", "uz",
			"1a", "1b", "1c", "1d", "1e", "1f", "1g", "1h", "1i", "1j", "1k", "1l", "1m", "1n", "1o", "1p", "1q", "1r", "1s", "1t", "1u", 
			"1v", "1x", "1y", "1z", "2a", "2b", "2c", "2d", "2e", "2f", "2g", "2h", "2i", "2j", "2k", "2l", "2m", "2n", "2o", "2p", "2q", 
			"2r", "2s", "2t", "2u", "2v", "2w", "2x", "2y", "2z", "3a", "3b", "3c", "3d", "3e", "3f", "3g", "3h", "3i", "3j", "3k", "3l",
			"3m", "3n", "3o", "3p", "3q", "3r", "3s", "3t", "3u", "3v", "3w", "3x", "3y", "3z", "4a", "4b", "4c", "4d", "4e", "4f", "4g", 
			"4h", "4i", "4j", "4k", "4l", "4m", "4n", "4o", "4p", "4q", "4r", "4s", "4t", "4u", "4v", "4w", "4x", "4y", "4z"};

	//get their license key
	long nLicenseKey = g_pLicense->GetLicenseKey();
	CString strLicenseKey;
	strLicenseKey.Format("LoginWithLicenseKey:%li", nLicenseKey);

	CString strEncrypt = "";
	for (int i = 0; i < strLicenseKey.GetLength(); i++) {
		
		long nChar  = ((long)((strLicenseKey.GetAt(i) + 125) ^ 77));
		
		strEncrypt += caryLetters[nChar];
	}
	CString strLink;
	strLink.Format("http://www.nextech.com/index.asp?webinars.asp?%s", strEncrypt);

	ShellExecute(NULL, NULL, strLink, NULL, NULL, SW_SHOW);


}

void CMainFrame::RecalcLayout(BOOL bNotify) 
{
	CNxMDIFrameWnd::RecalcLayout(bNotify);
}

void CMainFrame::AdminCPTEdit()
{
	CNxView* ptrCurrentView;

	if (!CheckCurrentUserPermissions(bioAdministratorModule, sptView))
		return;

	if(!CheckCurrentUserPermissions(bioAdminBilling, sptView))
		return;

	//we don't need to check for the billing tab, the code to change the tab does that for us
	//if(!CheckCurrentUserPermissions(bioAdminBilling,sptRead))
	//	return;

	if(FlipToModule(ADMIN_MODULE_NAME)) {
		ptrCurrentView = GetOpenView(ADMIN_MODULE_NAME);
		if (ptrCurrentView == NULL)
		{
			return;
		}
		if(ptrCurrentView->IsKindOf(RUNTIME_CLASS(CAdminView) ))
		{
			((CAdminView*) ptrCurrentView)->GoToBilling();
		} 
	}
}

BOOL CMainFrame::OnQueryNewPalette()
{
	CPalette *newPal, *oldPal;
	CDC *pdc = GetWindowDC();

	newPal = &theApp.m_palette;
	oldPal = pdc->SelectPalette(newPal, FALSE);
	pdc->RealizePalette();
	pdc->SelectPalette(oldPal, FALSE);
	return TRUE;
}

void CMainFrame::OnPaletteChanged(CWnd* pFocusWnd)
{
	// Do nothing if this function is currently running already or if the given palette 
	// was changed by one of our windows in the first place.  Without these checks we 
	// very quickly end up with an infinite loop of messages because our invalidation 
	// itself can cause the palettechanged message to be sent.
	if (!m_bIsPaletteChangedHandlerRunning && !::IsChild(m_hWnd, (HWND)pFocusWnd->GetSafeHwnd())) {
		m_bIsPaletteChangedHandlerRunning = true;
		
		// Invalidate all the views, or if there is a m_pViewActive, just invalidate that because 
		// it means there's a print preview taking up the whole screen right now.
		if (m_pViewActive) {
			m_pViewActive->Invalidate(FALSE);
		} else {
			for (long i=0; i<m_nOpenDocCnt; i++) {
				CNxView *pView = ((CNxView *)m_pView[i]);
				pView->Invalidate(FALSE);
			}
		}
		
		// Also invalidate the pop-up dialog if there is one
		CWnd *pTopWindow = GetLastActivePopup();
		if (pTopWindow && pTopWindow->GetSafeHwnd() != m_hWnd) {
			// Invalidate
			pTopWindow->Invalidate(FALSE);
			// For some reason if we don't update the popup window immediately we end up 
			// with an infinite loop of invalidating.
			pTopWindow->UpdateWindow();
		}
		m_bIsPaletteChangedHandlerRunning = false;
	}
}

void CMainFrame::OnViewResettoolbars() 
{
	if (IsScreen640x480())
		CNxMDIFrameWnd::ShowControlBar(&m_wndToolBar,	FALSE, FALSE);
	else
		CNxMDIFrameWnd::ShowControlBar(&m_wndToolBar,	TRUE, FALSE);

	if (IsScreen640x480())
		CNxMDIFrameWnd::ShowControlBar(&m_GUIToolBar,	FALSE, FALSE);
	else
		CNxMDIFrameWnd::ShowControlBar(&m_GUIToolBar,	TRUE, FALSE);

	CNxMDIFrameWnd::ShowControlBar(&m_patToolBar,		TRUE, FALSE);
	CNxMDIFrameWnd::ShowControlBar(&m_contactToolBar, TRUE, FALSE);
	CNxMDIFrameWnd::ShowControlBar(m_pDocToolBar,		TRUE, FALSE);
//	CNxMDIFrameWnd::ShowControlBar(&m_dateToolBar,	TRUE, FALSE);
	
	CNxView *p = GetActiveView();

	if (p)
		p->ShowToolBars();
	else
	{	ShowPatientBar(FALSE);
		ShowContactBar(FALSE);
//		ShowDateBar(FALSE);
		ShowDoctorBar(FALSE);
	}
	RestoreDocking();
}

void CMainFrame::OnUpdateViewResettoolbars(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable();	
}

void CMainFrame::OnGoBack()
{
	CNxTabView *ptab = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
	if (ptab) {
		ptab->GetActiveSheet()->StoreDetails();
	}

	//TES 1/6/2010 - PLID 36761 - Encapsulated in CPatientToolbar
	m_patToolBar.SelectLastPatient();
}

// Tries to delete the given file, and if the attempt 
// fails and the file exists, it is queued for deletion later
// Return value indicates whether the file exists upon completion
BOOL CMainFrame::DeleteFileWhenPossible(LPCTSTR strFilePathName)
{
	// Try to delete the specified file
	if (DeleteFile(strFilePathName)) {
		// Deletion was successful so return that the file doesn't exist
		return FALSE;
	} else if (DoesExist(strFilePathName)) {
		// The file exists and couldn't be deleted so enqueue it
		m_arystrDeleteFiles.Insert(strFilePathName);
		SetMemberTimer(IDT_DELETE_QUEUED_FILES_TIMER, 20000);
		// Return TRUE to indicate that the file exists still
		return TRUE;
	} else {
		// The file doesn't exist so return FALSE to indicate that fact
		return FALSE;
	}
}

//does not attempt to delete a file until nDelayInMS has elapsed
BOOL CMainFrame::DeleteFileWhenPossible(LPCTSTR strFilePathName, long nDelayInMS)
{
	if (DoesExist(strFilePathName)) {
		// The file exists and couldn't be deleted so enqueue it
		m_arystrDeleteFiles.Insert(strFilePathName);
		SetMemberTimer(IDT_DELETE_QUEUED_FILES_TIMER, nDelayInMS);
		// Return TRUE to indicate that the file exists still
		return TRUE;
	}
	else {
		// The file doesn't exist so return FALSE to indicate that fact
		return FALSE;
	}
}

// Loops through all the enqueued files attempting to delete each 
// of them.  If deletion was successful or they do not exist for 
// some other reason, they are dequeued
void CMainFrame::DeleteQueuedFiles(BOOL bForceDeQueue /*= FALSE*/)
{
	CString strFile;
	for (long i=0; i<m_arystrDeleteFiles.GetSize(); ) {
		// Get the file path name referred to by i
		strFile = m_arystrDeleteFiles.GetAt(i);

		// If the file is valid, exists, and can't be deleted, just move to the 
		// next item in the list.  If any of those conditions fail, however, the 
		// item doesn't belong in the list so dequeue it
		if (!strFile.IsEmpty() && DoesExist(strFile) && !DeleteFile(strFile)) {
			if (bForceDeQueue) {
				// We're dequeueing everything so as a last ditch 
				// effort delete the file using MoveFileOnStartup
				MoveFileAtStartup(strFile, NULL);
				m_arystrDeleteFiles.RemoveAt(i);
			} else {
				// Move to the next entry
				i++;
			}
		} else {
		   // Dequeue this entry and therefore do not move to the next
			m_arystrDeleteFiles.RemoveAt(i);
		}
	}

	// If there's nothing else in the list, we can go ahead and stop the timer
	if (m_arystrDeleteFiles.GetSize() == 0) {
		KillMemberTimer(IDT_DELETE_QUEUED_FILES_TIMER);
	}
}

//nStatus should be the contact status (from the toolbar)
//Returns true if they have write permission for that type, false 
//	otherwise.  User will be warned or prompted for passwords on 
//	failure & password requirements.
bool HasWritePermissionForContactType(long nStatus)
{
	//Code design here requires that all objects we test have 'sptWrite' available
	EBuiltInObjectIDs bioToCheck;
	switch(nStatus) {
		case 0:
			//other
			bioToCheck = bioContactsOther;
			break;
		case 1:
			//Ref phys
			bioToCheck = bioContactsRefPhys;
			break;
		case 2:
			//Provider
			bioToCheck = bioContactsProviders;
			break;
		case 4:
			//User
			bioToCheck = bioContactsUsers;
			break;
		case 8:
			//Supplier
			bioToCheck = bioContactsSuppliers;
			break;
	}

	if(!CheckCurrentUserPermissions(bioToCheck, sptWrite)) {
		return false;
	}

	return true;
}


void CMainFrame::OnContactChange()
{

	long m_nId = GetActiveContactID();

	//we can't let them change the type of the user currently logged in because it would delete that user
	if (m_nId == GetCurrentUserID()) {
		MsgBox("You may not change the type of the currently logged in user. \nPlease login as a different user and then change the type of this contact.");
	}
	else {
		//go to town
		CString oldstatus, newstatus;

		// (d.thompson 2009-09-21) - PLID 34570 - Check permissions before allowing the change.  This
		//	tests that we are allowed to write to the original type.  We'll also need write to the dest
		//	type once that is chosen.
		int m_nStatus = m_contactToolBar.GetActiveContactStatus();
		if(!HasWritePermissionForContactType(m_nStatus)) {
			return;
		}

		if(MessageBox("Changing the contact type will lose all contact specific information.  The general information (name, address, etc), Letter writing, Notes, Custom information, and ToDo items will be transferred.  Are you SURE you wish to do this?", "Change Contact Type", MB_YESNO) == IDNO)
			return;

		int  m_nNewStatus;
		int	 iDlgRes = IDCANCEL;


		// (a.walling 2009-06-05 17:19) - PLID 34410 - Contacts may be associated as custodians of an advance directive
		if (m_nStatus == 0) {
			try
			{
				CString strMessage;
				_RecordsetPtr prs = CreateParamRecordset("SELECT Count(ID) AS ItemCount FROM AdvanceDirectivesT WHERE CustodianID = {INT}", GetActiveContactID());
				long nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					strMessage.Format("You cannot change the type of this contact because it is the custodian of %li advance directive%s.",
						nCount, nCount > 1 ? "s" : "");
					MessageBox(strMessage);
					return;
				}
			}NxCatchAll("CMainFrame::OnContactChange (contacts)");
		}
		// (z.manning 2008-11-19 12:34) - PLID 31687 - Since you can't change providers that are used
		// as the co-surgeon on a PIC, I'm doing the same thing for referring physicians.
		if(m_nStatus == 1)
		{
			try
			{
				CString strMessage;
				_RecordsetPtr prs = CreateParamRecordset("SELECT Count(CoSurgeonID) AS ItemCount FROM ProcInfoT WHERE CoSurgeonID = {INT}", GetActiveContactID());
				long nCount = AdoFldLong(prs, "ItemCount");
				if(nCount > 0) {
					strMessage += FormatString("  - %li procedure information records as the co-surgeon.\r\n", nCount);
				}

				// (z.manning 2009-05-06 17:54) - PLID 28529 - No deleting ref phys that have a referral order
				prs = CreateParamRecordset("SELECT COUNT(*) AS Count FROM ReferralOrdersT WHERE ReferToID = {INT}", GetActiveContactID());
				nCount = AdoFldLong(prs->GetFields(), "Count");
				if(nCount > 0) {
					strMessage += FormatString("  - %li referral orders", nCount);
				}				

				// (j.gruber 2011-10-21 15:05) - PLID 45356 - affiliate physicians on bills
				prs = CreateParamRecordset("SELECT COUNT(*) AS Count FROM BillsT WHERE AffiliatePhysID = {INT}", GetActiveContactID());
				nCount = AdoFldLong(prs->GetFields(), "Count", 0);
				if(nCount > 0) {
					strMessage += FormatString("  - %li bills as the affiliate physician", nCount);
				}
				prs->Close();

				// (j.jones 2014-04-22 16:26) - PLID 61836 - check charges ReferringProviderID, OrderingProviderID, SupervisingProviderID
				prs = CreateParamRecordset("SELECT Count(ID) AS Count FROM ChargesT "
					"WHERE ReferringProviderID = {INT} OR OrderingProviderID = {INT} OR SupervisingProviderID = {INT}",
					GetActiveContactID(), GetActiveContactID(), GetActiveContactID());
				nCount = AdoFldLong(prs->GetFields(), "Count", 0);
				if(nCount > 0) {
					strMessage += FormatString("  - %li charges as a referring, ordering, or supervising physician.", nCount);
				}
				prs->Close();

				// (j.jones 2014-05-02 11:36) - PLID 61839 - check PLID 61839 - 
				prs = CreateParamRecordset("SELECT Count(FirstDocumentID) AS Count FROM HCFADocumentsT WHERE Box17PersonID = {INT}", GetActiveContactID());
				nCount = AdoFldLong(prs->GetFields(), "Count", 0);
				if (nCount > 0) {
					strMessage += FormatString("  - %li saved HCFA forms as a referring, ordering, or supervising physician.", nCount);
				}
				prs->Close();

				// (j.gruber 2013-01-17 11:46) - PLID 54483 - appointment referring physicians
				prs = CreateParamRecordset("SELECT COUNT(*) AS Count FROM AppointmentsT WHERE RefPhysID = {INT}", GetActiveContactID());
				nCount = AdoFldLong(prs->GetFields(), "Count", 0);
				if(nCount > 0) {
					strMessage += FormatString("  - %li appointments as the referring physician", nCount);
				}
				prs->Close();

				if(!strMessage.IsEmpty()) {
					strMessage = "You cannot change the type of this referring physician because it is linked with the following data: \r\n" + strMessage;
					MessageBox(strMessage);
					return;
				}
			}NxCatchAll("CMainFrame::OnContactChange (ref phys)");
		}
		if(m_nStatus == 2) {
			try {
				//DRT 3/14/03 - If the provider has some important information, we really don't want to delete him - it causes lots of problems.
				//		When we have the ability to inactivate, prompt them to do that instead.
				//DRT 6/30/2006 - PLID 21290 - Added requirement of no LabsT references
				// (z.manning, 09/15/2006) - We don't want to be able to change an EMN's provider ID even if it's deleted.
				// (j.jones 2006-12-01 11:05) - PLID 22110 - supported ClaimProviderID
				// (j.gruber 2007-01-08 10:58) - PLID 23399 - Secondary Providers on EMR
				// (a.walling 2007-01-17 16:34) - PLID 21003 - CoSurgeons on ProcInfo
				// (a.walling 2007-08-08 09:42) - No PLID, just tired of always coming here for offices to see why they can't
				// remove or change a provider. Simply added a 'Type' column that describes where the data is coming from. You
				// can copy this and run in NxQuery to quickly see where the data exists.
				// (j.jones 2007-10-01 16:33) - PLID 8993 - check for eligibility requests
				// (j.jones 2008-03-05 11:57) - PLID 29201 - supported providers on allocations
				// (j.jones 2008-04-02 14:42) - PLID 28895 - supported the HCFAClaimProvidersT references
				// (j.jones 2008-06-23 10:21) - PLID 30434 - added EligibilitySetupT
				// (j.jones 2008-12-11 10:44) - PLID 32407 - supported BillsT.SupervisingProviderID
				// (j.gruber 2009-05-07 17:14) - PLID 33688 - Other Providers on an EMR
				// (a.walling 2009-06-29 11:27) - PLID 34052 - Supervisor and Prescriber Agents for Patient Medications
				// (b.savon 2013-02-08 14:48) - PLID 54656 - renamed to oldagentid
				// (j.jones 2013-07-11 16:31) - PLID 57148 - check BillInvoiceNumbersT
				_RecordsetPtr rs = CreateParamRecordset(
					"SELECT * FROM ( "
					"SELECT 'CustomFieldDataT' AS Type, CustomFieldDataT.PersonID "
					"FROM    CustomFieldDataT "
					"UNION "
					"SELECT  'GroupDetailsT' AS Type, GroupDetailsT.PersonID "
					"FROM    GroupDetailsT "
					"UNION "
					"SELECT  'AdvHCFAPinT' AS Type, AdvHCFAPinT.ProviderID "
					"FROM    AdvHCFAPinT "
					"UNION "
							"(SELECT 'ClaimProvider' AS Type, ClaimProviderID AS ProviderID "
							"FROM    ProvidersT "
							"WHERE   ClaimProviderID <> PersonID"
							") "
					"UNION "
					"SELECT  'EbillingSetupT' AS Type, EbillingSetupT.ProviderID "
					"FROM    EbillingSetupT "
					"UNION "
					"SELECT  'EligibilitySetupT' AS Type, EligibilitySetupT.ProviderID "
					"FROM    EligibilitySetupT "
					"UNION "
					"SELECT  'PatientMedications' AS Type, PatientMedications.ProviderID "
					"FROM    PatientMedications "
					"UNION "
					"SELECT  'PatientMedicationsAgent' AS Type, PatientMedications.OldAgentID "
					"FROM    PatientMedications "
					"UNION "
					"SELECT  'PatientMedicationsSupervisor' AS Type, PatientMedications.SupervisorID "
					"FROM    PatientMedications "
					"UNION "
					"SELECT  'MultiFeeProvidersT' AS Type, ProviderID "
					"FROM    MultiFeeProvidersT "
					"UNION "
					"SELECT  'InsuranceNetworkID' AS Type, InsuranceNetworkID.ProviderID "
					"FROM    InsuranceNetworkID "
					"UNION "
					"SELECT  'InsuranceBox51' AS Type, InsuranceBox51.ProviderID "
					"FROM    InsuranceBox51 "
					"UNION "
					"SELECT  'InsuranceBox24J' AS Type, InsuranceBox24J.ProviderID "
					"FROM    InsuranceBox24J "
					"UNION "
					"SELECT  'InsuranceGroups' AS Type, InsuranceGroups.ProviderID "
					"FROM    InsuranceGroups "
					"UNION (SELECT 'DefaultProviderID' AS Type, DefaultProviderID FROM LocationsT WHERE DefaultProviderID Is Not Null) "
					// (j.jones 2008-06-17 09:03) - PLID 30410 - removed HCFAT from the program
					//"UNION (SELECT 'HCFATProviderID' AS Type, ProviderID FROM HCFAT WHERE HCFAT.ProviderID Is Not Null) "
					"UNION "
							"(SELECT 'HCFADocumentsT' AS Type, HCFADocumentsT.ProviderIndex "
							"FROM    HCFADocumentsT "
							"WHERE   HCFADocumentsT.ProviderIndex Is Not Null"
							") "
					"UNION (SELECT 'PaymentsT' AS Type, ProviderID FROM PaymentsT WHERE PaymentsT.ProviderID IS Not Null) "
					"UNION (SELECT 'PaymentTipsT' AS Type, ProvID FROM PaymentTipsT WHERE PaymentTipsT.ProvID IS Not Null) "
					"UNION (SELECT 'ProcInfoT' AS Type, CoSurgeonID FROM ProcInfoT WHERE CoSurgeonID IS NOT NULL) "
					"UNION "
							"(SELECT 'ChargesT_DoctorsProviders' AS Type, DoctorsProviders "
							"FROM    ChargesT "
							"WHERE   ChargesT.DoctorsProviders Is Not Null"
							") "
					// (j.jones 2010-11-08 17:40) - PLID 31392 - check the ClaimProviderID
					"UNION "
							"(SELECT 'ChargesT_ClaimProviderID' AS Type, ClaimProviderID "
							"FROM    ChargesT "
							"WHERE   ChargesT.ClaimProviderID Is Not Null"
							") "
					// (j.jones 2014-04-22 16:26) - PLID 61836 - check the ReferringProviderID, OrderingProviderID, SupervisingProviderID
					"UNION "
							"(SELECT 'ChargesT_ReferringProviderID' AS Type, ReferringProviderID "
							"FROM    ChargesT "
							"WHERE   ChargesT.ReferringProviderID Is Not Null"
							") "
					"UNION "
							"(SELECT 'ChargesT_OrderingProviderID' AS Type, OrderingProviderID "
							"FROM    ChargesT "
							"WHERE   ChargesT.OrderingProviderID Is Not Null"
							") "
					"UNION "
							"(SELECT 'ChargesT_SupervisingProviderID' AS Type, SupervisingProviderID "
							"FROM    ChargesT "
							"WHERE   ChargesT.SupervisingProviderID Is Not Null"
							") "
					// (j.jones 2014-05-02 11:36) - PLID 61839 - check HCFADocumentsT
					"UNION "
						"(SELECT 'HCFADocumentsT' AS Type, Box17PersonID "
						"FROM    HCFADocumentsT "
						"WHERE   HCFADocumentsT.Box17PersonID Is Not Null"
						") "
					"UNION (SELECT 'SupervisingProviderID' AS Type, SupervisingProviderID FROM BillsT WHERE BillsT.SupervisingProviderID IS Not Null) "
					"UNION (SELECT 'MainPhysician' AS Type, MainPhysician FROM PatientsT WHERE PatientsT.MainPhysician Is Not Null) "
					"UNION (SELECT 'EmrProvidersT' AS Type, ProviderID FROM EmrProvidersT) "
					"UNION (SELECT 'EmrSecondaryProvidersT' AS Type, ProviderID FROM EmrSecondaryProvidersT) "
					"UNION (SELECT 'BatchPaymentsT' AS Type, ProviderID FROM BatchPaymentsT) "
					"UNION (SELECT 'LabMultiProviderT' AS Type, ProviderID FROM LabMultiProviderT) "
					"UNION (SELECT 'EligibilityRequestsT' AS Type, ProviderID FROM EligibilityRequestsT) "
					"UNION (SELECT 'PatientInvAllocationsT' AS Type, ProviderID FROM PatientInvAllocationsT) "
					"UNION (SELECT 'HCFAClaimProvidersT_2010AA' AS Type, ANSI_2010AA_ProviderID FROM HCFAClaimProvidersT) "
					"UNION (SELECT 'HCFAClaimProvidersT_2310B' AS Type, ANSI_2310B_ProviderID FROM HCFAClaimProvidersT) "
					"UNION (SELECT 'HCFAClaimProvidersT_Box33' AS Type, Box33_ProviderID FROM HCFAClaimProvidersT) "
					"UNION (SELECT 'HCFAClaimProvidersT_Box24J' AS Type, Box24J_ProviderID FROM HCFAClaimProvidersT) "
					"UNION (SELECT 'EmrOtherProvidersT' AS Type, ProviderID FROM EmrOtherProvidersT) "
					// (j.dinatale 2012-05-07 16:59) - PLID 50219 - check glassesorderT 
					"UNION (SELECT 'GlassesOrderT_Optician' AS Type, OpticianID FROM GlassesOrderT WHERE OpticianID IS NOT NULL) "
					"UNION (SELECT 'GlassesOrderT_Provider' AS Type, ProviderID FROM GlassesOrderT WHERE ProviderID IS NOT NULL) "
					"UNION (SELECT 'Bill Invoices' AS Type, ProviderID FROM BillInvoiceNumbersT WHERE ProviderID IS NOT NULL) "
					// (j.jones 2016-02-17 17:18) - PLID 68348 - check recalls
					"UNION (SELECT 'Recalls' AS Type, ProviderID FROM RecallT WHERE ProviderID IS NOT NULL) "
					") SubQ WHERE PersonID = {INT}", GetActiveContactID());

				if(!rs->eof) {
					//there is data
					AfxMessageBox("There is critical data associated with this provider.  You cannot delete a provider which has data.");
					return;
				}

				//(e.lally 2011-06-17) PLID 43177 - Prevent the change if this is an actively licensed EMR provider.
				//	I am not aware of any potential problems by leaving this provider in the licensing info even though 
				//	they are hidden from view and the information no longer applies
				// (a.wilson 2012-08-08 16:59) - PLID 52043 - applied the use of a new function to clean up extra code.
				if(g_pLicense->IsProviderLicensed(GetActiveContactID())) {
					AfxMessageBox("This is a licensed EMR Provider. You must first inactivate this provider license by going to:\r\n"
						"Tools->Licensing->Manage EMR Provider Licenses");
					return;
				}

				// (b.savon 2013-03-01 14:26) - PLID 54578 - if this user has nexerx role
				if(ReturnsRecordsParam("SELECT TOP 1 PersonID FROM ProvidersT WHERE PersonID = {INT} AND NexERxProviderTypeID IS NOT NULL AND NexERxProviderTypeID > -1", GetActiveContactID())){
					MsgBox("This provider has a NexERx role and cannot be changed. Please deactivate their NexERx license before changing the contact type.");
					return;
				}

				// (b.savon 2013-06-13 10:31) - PLID 56867 - if this user is a registered prescriber
				if(ReturnsRecordsParam("SELECT TOP 1 ProviderID FROM NexERxPrescriberRegistrationT WHERE ProviderID = {INT}", GetActiveContactID())){
					MsgBox("This provider is registered as a NexERx prescriber and cannot be changed.");
					return;
				}

			} NxCatchAll("Error determining Provider status.");
		}

		if (m_nStatus == 8) { // Supplier

			//(c.copits 2011-10-06) PLID 45709 - Changing a supplier type that is used in the Glasses Catalog Setup will generate a FK constraint error.
			if (CheckGlassesSupplierDesigns())
			{
				MsgBox(MB_ICONSTOP|MB_OK,"This supplier currently has glasses designs associated with it. You may not change this contact type.");
				return;			
			}

			if (CheckGlassesSupplierMaterials())
			{
				MsgBox(MB_ICONSTOP|MB_OK,"This supplier currently has glasses materials associated with it. You may not change this contact type.");
				return;			
			}

			if (CheckGlassesSupplierTreatments())
			{
				MsgBox(MB_ICONSTOP|MB_OK,"This supplier currently has glasses treatments associated with it. You may not change this contact type.");
				return;			
			}

			if (CheckGlassesSupplierFrames())
			{
				MsgBox(MB_ICONSTOP|MB_OK,"This supplier currently has glasses frame types associated with it. You may not change this contact type.");
				return;			
			}

			// (j.dinatale 2012-05-16 19:48) - PLID 50443 - check GlassesOrderT before deleting a supplier, contact lens dont have a setup catalog.
			if(ReturnsRecordsParam("SELECT TOP 1 1 FROM GlassesOrderT WHERE SupplierID = {INT}", GetActiveContactID())){
				MsgBox(MB_ICONSTOP|MB_OK,"This supplier currently has optical orders associated with it. You may not change this contact type.");
				return;
			}
		}

		CChangeTypeDlg dlg(this);
		CUserPropsDlg propDlg(this);
		dlg.nStatus = m_nStatus;
		//this will let us know if there was an error inside the transaction
		bool bError = true;

		if(dlg.DoModal() != IDOK)
			return;

		m_nNewStatus = dlg.nStatus;

		if(m_nNewStatus == m_nStatus)
		{	//type has not been changed
			AfxMessageBox("You have selected the same contact type.  Please select a new type if you wish to use this feature.");
			return;
		}

		// (d.thompson 2009-09-21) - PLID 34570 - Second permission check.  Now that we have a destination
		//	type, ensure we have permission to write to that group as well.
		if(!HasWritePermissionForContactType(m_nNewStatus)) {
			return;
		}

		//DRT 1/30/03 - Moved the UserPermission box inside the if statement ... previously if you did not have permission, it never returned (!!!), 
		//		so it would delete the old contact, but (due to another odd issue with the way this is setup), did not have IDOK as the iDlgRes value, 
		//		so the code never ran to insert the new record, meaning we had a PersonT record in limbo.
		if (m_nNewStatus == 4) {	//user

			//DRT 1/30/03 - I don't know what 'EmployeeConfig' tries to convert into, but it does not appear to work at all.
			//if(UserPermission(EmployeeConfig)) {
			if(CheckCurrentUserPermissions(bioContactsUsers, sptWrite)) {
				propDlg.m_nUserID = m_nId;
				// (j.gruber 2010-04-14 13:51) - PLID 38186 - don't show the configure groups button
				propDlg.m_bShowConfigureGroups = FALSE;
				iDlgRes = propDlg.DoModal();
				if (iDlgRes == IDCANCEL)
					return;
			}
			else {
				//no permission
				return;
			}
		}

		BOOL bDeleteMessages = FALSE;
		long nAssignStepsTo = -1;
		long nAssignTodosTo = -1;
		long nAssignEmrTodoActionsTo = -1; // (c.haag 2008-06-23 13:02) - PLID 30471
		if(m_nStatus == 4) {
			//DeleteUser had these messageboxes in it.  You can't have a message box inside a transaction!
			
			//If the user is an Administrator, you CANNOT DELETE it
			if(ReturnsRecords("SELECT Administrator FROM UsersT WHERE Administrator = 1 AND PersonID = %li",m_nId)) {
				MsgBox("This user is an Administrator and cannot be deleted.\n\n"
					"You may, however, deactivate the user's administrator status and delete them,\n"
					"if another administrator user exists.");
				return;
			}

			// (c.haag 2005-10-10 15:22) - PLID 17846 - Do not let someone change a user that has
			// export history records associated with them.
			if (ReturnsRecords("SELECT ExportedBy FROM ExportHistoryT WHERE ExportedBy = %d", GetActiveContactID()))
			{
				MsgBox("This user has historical patient export records associated with it, and may not be changed.");
				return;
			}

			// (c.haag 2009-10-12 13:02) - PLID 35722 - Cannot delete users that are assigned with exported MailSent records. Only applies
			// to NexPhoto exports right now.
			if (ReturnsRecords("SELECT ID FROM MailSentExportFieldT WHERE UserID = %d", GetActiveContactID()))
			{
				MessageBox("This user is linked to exported patient photos and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			// (c.haag 2005-10-10 15:22) - PLID 17847 - Check if the person is using the NexPDA link.
			if (ReturnsRecords("SELECT UserID FROM OutlookProfileT WHERE UserID = %d", GetActiveContactID()))
			{
				// (z.manning 2009-11-17 12:31) - PLID 31879 - May also be a NexSync profile
				MsgBox("This user is associated with at least one NexPDA or NexSync profile. Before deleting this user, you must delete their profile from NexSync or the NexPDA link.\n\n");
				return;
			}

			//DRT 6/30/2006 - PLID 21290 - May not delete a user tied to labs
			// (c.haag 2010-12-10 10:15) - PLID 41590 - Check SignedBy
			// (c.haag 2011-01-17) - PLID 41806 - Removed CompletedBy
			if(ReturnsRecords("SELECT TOP 1 ID FROM LabsT WHERE InputBy = %li OR DeletedBy = %li OR SignedBy = %li", m_nId, m_nId, m_nId)) {
				MsgBox("This user is associated with at least one lab record and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (c.haag 2009-05-13 11:39) - PLID 28561 - Check for associations with lab reviews
			if (ReturnsRecords("SELECT TOP 1 ResultID FROM LabResultsT WHERE AcknowledgedUserID = %d", m_nId))
			{
				MsgBox("This user has reviewed at least one lab result, and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (j.gruber 2008-11-06 16:29) - PLID 31432 - LabResults	
			// (c.haag 2010-12-10 10:15) - PLID 41590 - More fields
			if(ReturnsRecords("SELECT TOP 1 ResultID FROM LabResultsT WHERE DeletedBy = %li OR ResultSignedBy = %li OR ResultCompletedBy = %li", m_nId)) {
				MsgBox("This user is associated with at least one lab result record and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (c.haag 2006-09-19 14:44) - PLID 22260 - Cannot delete users associated who created or modified EMR problems
			if (ReturnsRecords("SELECT TOP 1 ID FROM EMRProblemHistoryT WHERE UserID = %d", GetActiveContactID())) {
				MsgBox("This user is associated with at least one EMR problem record, and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (j.jones 2006-09-25 15:00) - PLID 22604 - check Room details and history
			if (ReturnsRecords("SELECT TOP 1 ID FROM RoomAppointmentsT WHERE LastUpdateUserID = %d", GetActiveContactID())
				// (a.walling 2013-06-07 09:45) - PLID 57078 - RoomAppointmentHistoryT no longer has an ID column; use the same as the predicate
				|| ReturnsRecords("SELECT TOP 1 UpdateUserID FROM RoomAppointmentHistoryT WHERE UpdateUserID = %d", GetActiveContactID())) {
				MsgBox("This user is associated with at least one historical Scheduler Room record, and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (j.jones 2007-01-29 11:45) - PLID 24353 - can't delete users who have edited EMNs
			// (j.jones 2007-02-06 13:38) - PLID 24509 - or EMRs (though the odds of an EMR record without an EMN record are slim)
			if (ReturnsRecords("SELECT TOP 1 ID FROM EMRMasterSlipT WHERE UserID = %d", GetActiveContactID())
				|| ReturnsRecords("SELECT TOP 1 ID FROM EMRGroupsSlipT WHERE UserID = %d", GetActiveContactID())) {
				MsgBox("This user is associated with at least one EMR record, and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (j.jones 2008-11-25 09:17) - PLID 28508 - can't delete any user who is tracked as having reviewed patient allergies
			if(ReturnsRecords("SELECT TOP 1 PersonID FROM PatientsT WHERE AllergiesReviewedBy = %li", GetActiveContactID())) {
				MsgBox("This user has reviewed patient allergies, and cannot be deleted. Please inactivate this user instead.");
				return;
			}

			// (z.manning 2009-01-27 09:25) - PLID 32187 - No deleting users who entered a tracking ladder
			if (ReturnsRecords("SELECT TOP 1 ID FROM LaddersT WHERE InputUserID = %li", GetActiveContactID())) {
				MessageBox("This user is associated with tracking ladders and may not be deleted.  Please mark this user as inactive.");
				return;
			}

			// (d.thompson 2009-04-14 16:05) - PLID 33957 - Cannot delete users that have processed credit cards
			// (d.thompson 2010-12-20) - PLID 41897 - Chase too
			// (c.haag 2015-08-24) - PLID 67198 - Use a utility function to check all processing methods
			if (DoesUserHaveCreditCardTransactions(GetActiveContactID()))
			{
				MsgBox("This user is linked to Credit Card transactions and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			// (z.manning 2009-05-07 09:09) - PLID 28529 - Can't delete users tied to a referral order
			{
				_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM ReferralOrdersT WHERE InputUserID = {INT}", GetActiveContactID());
				if(!prs->eof) {
					MessageBox("This user is linked to referral orders and cannot be deleted.  Please inactivate this user instead.");
					return;
				}
			}

			// (j.jones 2010-01-22 12:27) - PLID 37016 - can't delete users tied to a prescription or current medication
			{
				_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM PatientMedications WHERE InputByUserID = {INT}", GetActiveContactID());
				if(!prs->eof) {
					MessageBox("This user is linked to patient prescriptions and cannot be deleted.  Please inactivate this user instead.");
					return;
				}
			}

			{
				_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM CurrentPatientMedsT WHERE InputByUserID = {INT}", GetActiveContactID());
				if(!prs->eof) {
					MessageBox("This user is linked to patient current medications and cannot be deleted.  Please inactivate this user instead.");
					return;
				}
			}

			// (d.thompson 2009-05-18) - PLID 34232 - Can't delete users tied to immunizations
			{
				_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM PatientImmunizationsT WHERE CreatedUserID = {INT}", GetActiveContactID());
				if(!prs->eof) {
					MessageBox("This user is linked to immunization history and cannot be deleted.  Please inactivate this user instead.");
					return;
				}
			}

			// (j.jones 2009-09-11 17:01) - PLID 35145 - can't delete users tied to AptShowStateHistoryT
			{
				_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 UserID FROM AptShowStateHistoryT WHERE UserID = {INT}", GetActiveContactID());
				if(!prs->eof) {
					MessageBox("This user is linked to appointment histories and cannot be deleted.  Please inactivate this user instead.");
					return;
				}
			}

			// (j.jones 2009-09-21 13:56) - PLID 35595 - don't allow deleting a user with completed to-dos
			{
				_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 AssignTo "
					"FROM ToDoList "
					"INNER JOIN TodoAssignToT ON TodoAssignToT.TaskID = ToDoList.TaskID "
					"WHERE AssignTo = {INT} AND Done Is Not Null", GetActiveContactID());
				if(!prs->eof) {
					MessageBox("This user is linked to completed ToDo tasks and cannot be deleted.  Please inactivate this user instead.");
					return;
				}
			}

			// (j.jones 2010-12-20 17:40) - PLID 41852 - check for FinancialCloseHistoryT records
			if (ReturnsRecordsParam("SELECT UserID FROM FinancialCloseHistoryT WHERE UserID = {INT}", GetActiveContactID()))
			{
				MessageBox("This user is linked to a close of financial data and cannot be changed.  Please inactivate this user instead.");
				return;
			}

			// (s.dhole 2011-02-24 12:24) - PLID 40535 check if user is link to any glasses order or order history
			// (s.dhole 2010-03-10 17:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
			// (j.dinatale 2012-04-17 17:26) - PLID 49078 - changed glasses order to optical order
			if(ReturnsRecords("SELECT TOP 1 ID FROM GlassesOrderT WHERE UserID = %li ", GetActiveContactID()) || ReturnsRecords("SELECT TOP 1 ID FROM GlassesOrderHistoryT WHERE UserID = %li ", GetActiveContactID()) ) {
				MsgBox("This user is linked to an Optical order and cannot be changed.  Please inactivate this user instead.");
				return;
			}

			// (d.lange 2011-03-24 16:46) - PLID 42943 - check users that are associated with EMNs as Assistant/Tech
			if(ReturnsRecords("SELECT TOP 1 PersonID FROM EmrTechniciansT WHERE PersonID = %li", GetActiveContactID())) {
				MsgBox("This user is associated with at least one EMN as the Assistant/Technician, therefore cannot be changed.  "
					"Please inactivate this user instead.");
				return;
			}

			// (j.jones 2011-09-15 15:21) - PLID 44905 - disallow if the user is linked to BillCorrectionsT records
			if(ReturnsRecordsParam("SELECT TOP 1 InputUserID FROM BillCorrectionsT WHERE InputUserID = {INT}", GetActiveContactID())) {
				MsgBox("This user is associated with at least one corrected bill, and therefore cannot be changed.  "
					"Please inactivate this user instead.");
				return;
			}

			// (j.jones 2011-09-15 15:21) - PLID 44905 - disallow if the user is linked to LineItemCorrectionsT records
			if(ReturnsRecordsParam("SELECT TOP 1 InputUserID FROM LineItemCorrectionsT WHERE InputUserID = {INT}", GetActiveContactID())) {
				MsgBox("This user is associated with at least one corrected line item in billing, and therefore cannot be changed.  "
					"Please inactivate this user instead.");
				return;
			}

			// (j.armen 2012-06-15 09:29) - PLID 48299 - Don't allow deletion if user has worked with recalls
			if(ReturnsRecordsParam("SELECT TOP 1 ID FROM RecallT WHERE CreatedUserID = {INT} OR ModifiedUserID = {INT}", GetActiveContactID(), GetActiveContactID())) {
				MsgBox("This user is associated with at least one patient recall, and therefore cannot be changed.  "
					"Please inactivate this user instead.");
				return;
			}

			// (j.jones 2011-10-06 16:43) - PLID 45828 - if this user is logged in from a device, you cannot delete the user
			if(ReturnsRecordsParam("SELECT TOP 1 UserID FROM UserLoginTokensT WHERE UserID = {INT}", GetActiveContactID())) {
				MsgBox("This user is currently logged in from a device, and cannot be deleted.");
				return;
			}

			// (j.jones 2013-01-16 10:11) - PLID 54634 - added E/M coding progress tables
			// (d.singleton 2013-24-4 12:14) - PLID 55817 - change ID column to EMNID
			{
				_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 EMNID FROM EMNEMChecklistRuleProgressT WHERE ApprovedBy = {INT} "
					"UNION SELECT TOP 1 EMNID FROM EMNEMChecklistCodingLevelProgressT WHERE ApprovedBy = {INT};", GetActiveContactID(), GetActiveContactID());
				if(!prs->eof) {
					MsgBox("This user is linked to E/M Checklist progress on EMN records. Please inactivate this user instead.");
					return;
				}
			}

			// (b.savon 2013-02-08 16:10) - PLID 54656 - if the user is on a rx
			if(ReturnsRecordsParam("SELECT TOP 1 NurseStaffID FROM PatientMedications WHERE NurseStaffID = {INT}", GetActiveContactID())){
				MsgBox("This user is associated with at least one prescription, and therefore cannot be changed.  "
						"Please inactivate this user instead.");
				return;
			}

			// (b.savon 2013-03-01 14:26) - PLID 54578 - if this user has a nurse/staff role
			if(ReturnsRecordsParam("SELECT TOP 1 PersonID FROM UsersT WHERE PersonID = {INT} AND NexERxUserTypeID IS NOT NULL AND NexERxUserTypeID > -1", GetActiveContactID())){
				MsgBox("This user has a NexERx role and cannot be changed. Please deactivate their NexERx license before changing the contact type.");
				return;
			}

			//(s.dhole 9/3/2014 12:00 PM ) - PLID 63552 Check is user used in PatientRemindersSentT
			if (ReturnsRecordsParam("SELECT TOP 1 UserID FROM PatientRemindersSentT WHERE UserID = {INT} AND Deleted = 0 ", GetActiveContactID())){
				MsgBox("This user is associated with patient reminder sent history and cannot be changed.  Please inactivate this user instead.");
				return;
			}

			// (j.jones 2014-11-26 16:00) - PLID 64272 - disallow deleting a user who override an appointment mix rule
			if (ReturnsRecordsParam("SELECT TOP 1 UserID FROM AppointmentMixRuleOverridesT WHERE UserID = {INT}", GetActiveContactID())){
				MsgBox("This user has overridden appointments for scheduling mix rules and cannot be deleted.  Please inactivate this user instead.");
				return;
			}

			// (j.jones 2015-04-23 12:33) - PLID 65711 - disallow deleting a user who has transferred GC balances
			if (ReturnsRecordsParam("SELECT TOP 1 InputByUserID FROM GiftCertificateTransfersT WHERE InputByUserID = {INT}", GetActiveContactID())){
				MsgBox("This user has transferred Gift Certificate balances, and for historical tracking purposes cannot be deleted. Please inactivate this user instead.");
				return;
			}

			//(e.lally 2010-08-18) PLID 37982 - don't allow changing a user type if they are the only one assigned as the NexWeb ToDo owner
			//	silently remove otherwise.
			//(e.lally 2010-11-19) PLID 35819 - Migrated from NexWeb Leads AssignTo Users preference to its own table structure
			//(e.lally 2012-01-03) PLID 47136 - NexWeb_ToDo_Template_Assign_To_T is now NexWeb_Event_Assign_To_T
			{
				_RecordsetPtr rs = CreateParamRecordset("SELECT COUNT(*) AS UserCount FROM NexWebEventToDoAssignToT "
						"WHERE NexWebEventID IN(SELECT NexWebEventID FROM NexWebEventToDoAssignToT WHERE UserID = {INT}) "
						"GROUP BY NexWebEventID "
						"HAVING COUNT(*) = 1 ", GetActiveContactID());
					if(!rs->eof){
						//this is the only user assigned, prevent the change
						MessageBox("This user is the default owner of new ToDo Alarms for events in NexWeb. \r\n"
						"Please select a new user under the Links tab of the Administrator module before changing the contact type for this user.");
						return;
				}
			}

			//If they have any messages, find out whether to delete them.			
			if(ReturnsRecords("SELECT MessageID FROM MessagesT WHERE SenderID = %li OR RecipientID = %li", m_nId, m_nId)) {
				if (IDYES == MsgBox(MB_YESNO, "This user has messages associated with it in the PracYakker.  Would you like to delete them?\nIf you say \"Yes\", all messages sent to or sent by this user will be permanently deleted.  If you say \"No\", all messages associated with this user will be marked as <Deleted User>."))
					bDeleteMessages = TRUE;
				else
					bDeleteMessages = FALSE;
			}
			
			
			// (j.jones 2008-11-26 13:30) - PLID 30830 - changed StepsT/StepTemplatesT references to support the multi-user tables
			if(ReturnsRecords("SELECT UserID FROM StepsAssignToT WHERE UserID = %li UNION SELECT UserID FROM StepTemplatesAssignToT WHERE UserID = %li UNION SELECT UserID FROM LaddersT WHERE UserID = %li", GetActiveContactID(), GetActiveContactID(), GetActiveContactID()) ) {
				CSelectUserDlg dlg(this);
				dlg.m_nExcludeUser = GetActiveContactID();
				dlg.m_strCaption = "This user has Tracking steps assigned to them. Please select a user for these steps to be re-assigned to.";
				if(IDOK == dlg.DoModal()) {
					nAssignStepsTo = dlg.m_nSelectedUser;
				}
				else {
					return;
				}
			}
			
			// (a.walling 2006-09-11 13:48) - PLID 22458 - Prompt to reassign todo tasks
			// (c.haag 2008-06-10 09:24) - PLID 11599 - Use TodoAssignToT
			// (j.jones 2009-09-21 14:16) - PLID 35595 - we should not be able to get here
			// if the user has completed todos, so we do not need to filter on the completion status
			if(ReturnsRecords("SELECT AssignTo FROM TodoAssignToT WHERE AssignTo = %li", GetActiveContactID())) {
				CSelectUserDlg dlg(this);
				dlg.m_nExcludeUser = GetActiveContactID();
				dlg.m_bAllowNone = false;
				dlg.m_strCaption = "This user has unfinished todo alarms assigned to them. Please select a user for these alarms to be re-assigned to.";
				if (IDOK == dlg.DoModal()) {
					nAssignTodosTo = dlg.m_nSelectedUser;
				}
				else {
					return;
				}
			}

			// (c.haag 2008-06-23 12:59) - PLID 30471 - Prompt if any EMR todo actions involve this user
			if(ReturnsRecords("SELECT ActionID FROM EMRActionsTodoAssignToT WHERE AssignTo = %li", GetActiveContactID())) {
				CSelectUserDlg dlg(this);
				dlg.m_nExcludeUser = GetActiveContactID();
				dlg.m_bAllowNone = false;
				dlg.m_strCaption = "This user has EMR Todo spawning actions assigned to them. Please select a user for these actions to be re-assigned to.";
				if (IDOK == dlg.DoModal()) {
					nAssignEmrTodoActionsTo = dlg.m_nSelectedUser;
				}
				else {
					return;
				}
			}

			// (v.maida 2014-08-15 09:50) - PLID 62753 - Don't allow the contact type to be changed if it is a user that has been selected to be assigned a Todo alarm for diagnosis codes spawned in different diagnosis modes.
			if (ReturnsRecords("SELECT TOP 1 1 FROM dbo.StringToIntTable((SELECT MemoParam FROM ConfigRT WHERE Name = 'TodoUsersWhenDiagCodeSpawnedInDiffDiagMode'), ',') AS PrefUsersQ WHERE PrefUsersQ.Data = %li", GetActiveContactID())) {
				MsgBox("This user has been selected to be assigned Todo alarms when diagnosis codes are spawned while in a different global diagnosis mode, and thus cannot have its contact type changed."
					"\r\nNavigate to Tools->Preferences...->Patients Module->NexEMR->EMR 3 tab->'Create a Todo when a diagnosis code is spawned while in a different global diagnosis mode' preference and uncheck this user's name from that preference's datalist to lift this restriction.");
				return;
			}

			//(r.wilson 7/29/2013) PLID 48684 - Make sure no secondary providers are linked to this provider
			if(ReturnsRecords("SELECT PersonID FROM ProvidersT WHERE EMRDefaultProviderID = %li", GetActiveContactID())){
			
				if(IDNO == MsgBox(MB_YESNO, "This provider is linked to one or more secondary providers. By changing the contact type all links will be lost.?\nAre you sure you want to continue?"))
				{
					return;
				}
			}

		}

		try {
			// (a.walling 2010-09-08 13:26) - PLID 40377 - Using CSqlTransaction
			CSqlTransaction trans("ContactChange");
			trans.Begin();

			//(b.spivey, December 13th, 2013) - PLID 59022 - call this first lest we get a foreign key violation.
			UpdateDirectMessageAddressData(m_nId, m_nNewStatus); 

			//change all of the data that currently exists to the new Person ID
			switch(m_nStatus)
			{
			case 1:	//ref phys
				DeleteRefPhys();	//delete from all tables that are Ref Phys specific, we cannot keep this information
				ExecuteSql("DELETE FROM ReferringPhyST WHERE PersonID = %li", m_nId);
				oldstatus = "Referring Phys.";
				break;
			case 2:	//provider
				DeleteProvider();
				ExecuteSql("DELETE FROM ProvidersT WHERE PersonID = %li", m_nId);
				oldstatus = "Provider";			
				break;
			case 4:	//user
				{
					// (c.haag 2005-10-20 12:31) - PLID 18013 - This code is now obselete. All the deleting
					// for the NexPDA link with the new structure now takes place in CMainFrame::DeleteUser
					//
					//_RecordsetPtr prsOutlook = CreateRecordset("SELECT ID FROM OutlookSyncProfileT WHERE UserID = %d", m_nId);
					//while (!prsOutlook->eof)
					//{
					//	NxOutlookLink::DeleteProfile(AdoFldLong(prsOutlook, "ID"));
					//	prsOutlook->MoveNext();
					//}
					// (j.gruber 2007-11-07 12:37) - PLID 28026 - take off the EMRSpecialist if this is internal
					if (IsNexTechInternal()) {
						ExecuteParamSql("DELETE FROM EMRSpecialistT WHERE UserID = {INT}", GetActiveContactID());
					}
					// (c.haag 2014-02-21) - PLID 60953 - Delete from diagnosis quicklists
					ExecuteParamSql("DELETE FROM DiagnosisQuickListT WHERE UserID = {INT}", GetActiveContactID());
					ExecuteParamSql("DELETE FROM DiagnosisQuickListSharingT WHERE UserID = {INT} OR SharingUserID = {INT}"
						,GetActiveContactID(), GetActiveContactID());
					// (j.jones 2009-05-20 09:26) - PLID 33801 - delete from UserPasswordHistoryT
					ExecuteParamSql("DELETE FROM UserPasswordHistoryT WHERE UserID = {INT}", GetActiveContactID());
					ExecuteSql("DELETE FROM UserResourcesT WHERE UserID = %li", m_nId);
					// (j.gruber 2010-04-14 12:55) - PLID 37948 - delete from permission groups
					ExecuteParamSql("DELETE FROM UserGroupDetailsT WHERE UserID = {INT}", GetActiveContactID());
					// (j.gruber 2010-04-14 12:55) - PLID 38201 - delete permissions
					ExecuteParamSql("DELETE FROM PermissionT WHERE UserGroupID = {INT}", GetActiveContactID());
					// (a.walling 2010-11-11 17:13) - PLID 41459 - Clear out YakGroupDetailsT
					ExecuteParamSql("DELETE FROM YakGroupDetailsT WHERE PersonID = {INT}", GetActiveContactID());
					//(e.lally 2010-11-19) PLID 35819 - delete from NexWeb ToDo template setup
					//(e.lally 2012-01-03) PLID 47136 - NexWeb_ToDo_Template_Assign_To_T is now NexWeb_Event_Assign_To_T
					ExecuteParamSql("DELETE FROM NexWebEventToDoAssignToT WHERE UserID = {INT}", GetActiveContactID());
					// (z.manning 2014-08-22 09:08) - PLID 63251 - Handle UserLocationResourceExclusionT
					ExecuteParamSql("DELETE FROM UserLocationResourceExclusionT WHERE UserID = {INT}", GetActiveContactID());
					// (z.manning 2014-09-09 10:18) - PLID 63260
					ExecuteParamSql("DELETE FROM UserRoomExclusionT WHERE UserID = {INT}", GetActiveContactID());
					// (c.haag 2008-06-23 15:28) - PLID 30471 - Also reassign EMR Todo spawn actions
					DeleteUser(bDeleteMessages, nAssignStepsTo, nAssignTodosTo, nAssignEmrTodoActionsTo);
					ExecuteSql("DELETE FROM UsersT WHERE PersonID = %li", m_nId);					
					oldstatus = "User";
					//users currently have the username in the toolbar, change that
					_RecordsetPtr prs = CreateRecordset("SELECT Company FROM PersonT WHERE ID = %li", m_nId);
					if(!prs->eof) {
						_variant_t var = prs->Fields->Item["Company"]->Value;
						if(var.vt == VT_BSTR)
							m_contactToolBar.m_toolBarCombo->PutValue(m_contactToolBar.m_toolBarCombo->CurSel, 10, _bstr_t(VarString(var)));
					}
				}
				break;
			case 8:	//supplier
				{
				//if there is an order, we do not allow the change
				_RecordsetPtr rs = CreateRecordset("SELECT OrderT.Supplier FROM OrderT WHERE OrderT.Supplier = %li AND OrderT.ID Is Not Null", GetActiveContactID());
				if(!rs->eof)
				{
					// (a.walling 2010-09-08 13:26) - PLID 40377 - Using CSqlTransaction - Rollback is implicit. But we must rollback before messagebox!
					trans.Rollback();
					MsgBox("This supplier currently has orders associated with it, please remove these relationships if you wish to remove this contact");
					rs->Close();
					return;
				}
				rs->Close();
				// (s.dhole 2010-11-18 17:26) - PLID 41548 - Forbid the user to change  suppliers if it use in visionweb
				// tied to VisionWeb.
				rs = CreateParamRecordset("SELECT * FROM SupplierT WHERE VisionWebID<>'' AND  PersonID = {INT}", GetActiveContactID());
				if(!rs->eof){
					MsgBox(MB_ICONSTOP|MB_OK,"This supplier currently has VisionWeb data associated with it, and may not be changed.");
					
					rs->Close(); 
					return;			
				}
				else
					rs->Close(); 
				DeleteSupplier();
				ExecuteSql("DELETE FROM SupplierT WHERE PersonID = %li", m_nId);
				oldstatus = "Supplier";
				}
				break;
			case 0:	//other
				DeleteOtherContact();
				ExecuteSql("DELETE FROM ContactsT WHERE PersonID = %li", m_nId);
				oldstatus = "Other";
				break;
			default:	//no status, or no contact loaded
				AfxMessageBox("Error changing contact, please refresh and try again.");
				break;
			}

			//add a record to the table of the new type
			switch(m_nNewStatus)
			{
			case 0:
				ExecuteSql("INSERT INTO ContactsT (PersonID) SELECT %li", m_nId);
				break;
			case 1:
				ExecuteSql("INSERT INTO ReferringPhyST (PersonID) SELECT %li", m_nId);
				newstatus = "Referring Phys.";
				break;
			case 2:
			{
				// (j.jones 2006-12-01 10:09) - PLID 22110 - changed to support ClaimProviderID
				// (j.jones 2007-02-20 08:45) - PLID 24718 - ensured the TaxonomyCode is filled
				//TES 9/8/2008 - PLID 27727 - Dropped DefLocationID.
				// (j.jones 2009-06-26 09:25) - PLID 34292 - ensured the OHIP Specialty is filled
				//(e.lally 2012-04-06) PLID 48264 - Added default EMR MU progress settings. Changed to a batch.
				// (s.tullis 2015-02-23 13:43) - PLID 50955 - Remove any default taxonomy codes that we fill in for providers. Now that we support multiple specialities this can lead to billing errors.
				CSqlBatch sqlBatch;
				sqlBatch.Declare("SET XACT_ABORT ON ");
				sqlBatch.Add("INSERT INTO ProvidersT (PersonID, ClaimProviderID, TaxonomyCode, OHIPSpecialty) VALUES (%li, %li, '', '08')", m_nId, m_nId);
				sqlBatch.Add("INSERT INTO InsuranceAcceptedT (InsuranceCoID, ProviderID, Accepted) SELECT PersonID, %li AS ProviderID, %li AS Accepted FROM InsuranceCoT",m_nId,GetRemotePropertyInt("DefaultInsAcceptAssignment",1,0,"<None>",TRUE) == 1 ? 1 : 0);
				sqlBatch.Add("INSERT INTO ProviderMUMeasureOptionT (ProviderID, DateOptionNum, StartDate, ExcludeSecondaryProv) VALUES(%li, 4, NULL, 0) ", m_nId);
				for(int nMeasure = 18; nMeasure <= 36; nMeasure ++){
					sqlBatch.Add("INSERT INTO ProviderMUMeasureSelectionT (ProviderID, MeasureNum, Selected) VALUES(%li, %li, 1)", m_nId, nMeasure);
				}

				sqlBatch.Execute();
				newstatus = "Provider";
				break;
			}

			case 4:
			{

				// (r.farnworth 2015-12-29 11:12) - PLID 67719 - Change Practice to call our new ChangeContactInformation API method instead of calling SQL from C++ code.
				NexTech_Accessor::_UserPropertiesPtr pUserProperties(__uuidof(NexTech_Accessor::UserProperties));
				NexTech_Accessor::_ChangePasswordPtr pChangePassword(__uuidof(NexTech_Accessor::ChangePassword));
				NexTech_Accessor::_UserPtr pUser(__uuidof(NexTech_Accessor::User));

				CString strUserID, strLocID;
				strUserID.Format("%li", GetActiveContactID());
				strLocID.Format("%li", GetCurrentLocationID());
				pUser->PutID(AsBstr(strUserID));
				pUser->Putusername(AsBstr(propDlg.m_name));
				pUser->PutlocationID(AsBstr(strLocID));

				pChangePassword->User = pUser;
				pChangePassword->PutNewPassword(AsBstr(propDlg.m_pass));
				pChangePassword->PutCurrentPassword(AsBstr(propDlg.m_pass));

				pUserProperties->ChangePassword = pChangePassword;
				pUserProperties->PutReason(NexTech_Accessor::UserPropertyChangeReason::UserPropertyChangeReason_ChangeContact);
				pUserProperties->PutSavePassword((VARIANT_BOOL)propDlg.m_bMemory);
				pUserProperties->PutAdministrator((VARIANT_BOOL)propDlg.m_bAdministrator);
				pUserProperties->PutAutoLogoff((VARIANT_BOOL)propDlg.m_bInactivity);
				pUserProperties->PutAutoLogoffDuration(propDlg.m_nInactivityMinutes);
				pUserProperties->PutPasswordExpires((VARIANT_BOOL)propDlg.m_bExpires);
				pUserProperties->PutPasswordExpireDays(propDlg.m_nPwExpireDays);
				COleDateTime dtNow = COleDateTime::GetCurrentTime();
				dtNow.SetDate(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay());
				pUserProperties->PutPasswordPivotDate(dtNow);
				pUserProperties->PutPasswordExpireNextLogin((VARIANT_BOOL)(propDlg.m_bPasswordExpiresNextLogin ? 1 : 0));
				pUserProperties->PutAllLocations((VARIANT_BOOL)propDlg.m_bAllLocations);
				pUserProperties->PutproviderID(propDlg.m_nNewProviderID);
				pUserProperties->PutOldStatus(AsBstr(oldstatus));

				GetAPI()->UpdateUserProperties(GetAPISubkey(), GetAPILoginToken(), pUserProperties);
	
				m_contactToolBar.m_toolBarCombo->PutValue(m_contactToolBar.m_toolBarCombo->CurSel, 10, _bstr_t("< " + propDlg.m_name + " >"));
							
			}
				break;
			case 8:
				ExecuteSql("INSERT INTO SupplierT (PersonID) SELECT %li", m_nId);
				newstatus = "Supplier";
				break;
			default:
				ThrowNxException("Error saving new contact type");
				break;
			}

			// (c.haag 2006-10-06 14:16) - PLID 22863 - Update the contact toolbar contact type so that
			// the proper UI shows up during a refresh
			m_contactToolBar.SetExistingContactStatus(m_nId, m_nNewStatus);

			long AuditID = -1;
			AuditID = BeginNewAuditEvent();
			// (r.farnworth 2016-01-06 12:04) - PLID 67704 - We will audit in the API for users
			if(AuditID!=-1 && m_nNewStatus != 4) {
				AuditEvent(-1, GetActiveContactName(), AuditID, aeiTypeChanged, m_nId, oldstatus, newstatus, aepHigh, aetChanged);
			}

			//update the palm status so that the contact will have his type updated in a sync
			SetPalmRecordStatus("PalmContactsInfoT", m_nId, 1);

			//Update Outlook.
			PPCModifyContact(m_nId);

			trans.Commit();

			//if it got here, it didn't error out
			bError = false;

		} NxCatchAll("ContactChange");

		//after the transaction, see if they want to configure this user into a group
		if (m_nNewStatus == 4 && !propDlg.m_bAdministrator) {
			// (j.gruber 2010-04-14 13:21) - PLID 38186 - see if they want to configure a group for this user now
			//check permission before letting them in
			if (CheckCurrentUserPermissions(bioEditOtherUserPermissions, sptWrite, 0, 0, TRUE)) {				
				if (IDYES == MsgBox(MB_YESNO, "Would you like to configure permission groups for this user?")) {
					CConfigurePermissionGroupsDlg dlg(TRUE, m_nId, this);
					dlg.DoModal();
				}
			}
		}


		//make sure that we didn't get an error
		if (!bError) {
			// Now tell everybody about your changes AFTER THE TRANSACTION IS DONE.
			// Note: This will happen whether or not the transaction is a success.
			switch(m_nNewStatus)
			{
				case 0: //Other Contact
					CClient::RefreshTable(NetUtils::ContactsT, m_nId);
					break;
				case 1:	//ref phys
					CClient::RefreshTable(NetUtils::RefPhys, m_nId);
					break;
				case 2:	//provider
					CClient::RefreshTable(NetUtils::Providers, m_nId);
					break;
				case 4:	//user
					CClient::RefreshTable(NetUtils::Coordinators, m_nId);
					break;
				case 8:	//supplier
					CClient::RefreshTable(NetUtils::Suppliers, m_nId);
					break;
			}

			//also refresh the list of contacts that you just removed an entry from!
			switch(m_nStatus)
			{
				case 0: //Other Contact
					CClient::RefreshTable(NetUtils::ContactsT, m_nId);
					break;
				case 1:	//ref phys
					CClient::RefreshTable(NetUtils::RefPhys, m_nId);
					break;
				case 2:	//provider
					CClient::RefreshTable(NetUtils::Providers, m_nId);
					break;
				case 4:	//user
					CClient::RefreshTable(NetUtils::Coordinators, m_nId);
					break;
				case 8:	//supplier
					CClient::RefreshTable(NetUtils::Suppliers, m_nId);
					break;
			}
			CClient::RefreshTable(NetUtils::CustomContacts, m_nId);

			//reset the filter
			m_contactToolBar.m_all.SetCheck(true);
			m_contactToolBar.m_other.SetCheck(false);
			m_contactToolBar.m_referring.SetCheck(false);
			m_contactToolBar.m_supplier.SetCheck(false);
			m_contactToolBar.m_employee.SetCheck(false);
			m_contactToolBar.m_main.SetCheck(false);
			OnAllContactsSearchClicked();
			
			UpdateAllViews();
			AfxMessageBox("Contact has been successfully transferred!");
		}
		else {

			MsgBox("The Contact was not transferred");
		}
	}

	
}

//this function deletes all data from tables which contain an ID related to a referring physician, excluding PersonT and RefPhysT
void CMainFrame::DeleteRefPhys()
{

		//fix these fields
		//(e.lally 2009-05-11) PLID 28553 - Combine these into one execute
	// (j.gruber 2011-09-27 16:07) - PLID 45357 - clear out affiliate physician
		ExecuteParamSql(GetRemoteData(), "DECLARE @nRefPhyID INT\r\n"
			"SET @nRefPhyID = {INT} \r\n"
			"UPDATE BillsT SET RefPhyID = -1 WHERE BillsT.RefPhyID = @nRefPhyID \r\n"
			"UPDATE PatientsT SET DefaultReferringPhyID = NULL WHERE PatientsT.DefaultReferringPhyID = @nRefPhyID \r\n"
			"UPDATE PatientsT SET PCP = NULL WHERE PatientsT.PCP = @nRefPhyID \r\n"
			"UPDATE PatientsT SET AffiliatePhysID = NULL WHERE PatientsT.AffiliatePhysID = @nRefPhyID \r\n"
			// (a.wilson 2014-5-5) PLID 61831 - clear out ChargeLevelProviderConfigT where refphys was used.
			"Update ChargeLevelProviderConfigT SET ReferringProviderID = NULL WHERE ReferringProviderID = @nRefPhyID \r\n"
			"Update ChargeLevelProviderConfigT SET OrderingProviderID = NULL WHERE OrderingProviderID = @nRefPhyID \r\n"
			"Update ChargeLevelProviderConfigT SET SupervisingProviderID = NULL WHERE SupervisingProviderID = @nRefPhyID \r\n"
			"DELETE FROM RefPhysProcLinkT WHERE RefPhysID = @nRefPhyID \r\n"
			//(e.lally 2009-05-11) PLID 28553 - Delete from OrderSetTemplateReferralsT. The user has already
			//	been warned about associated data being removed.
			"DELETE FROM  OrderSetTemplateReferralsT WHERE ReferringPhyID = @nRefPhyID \r\n"
			//(e.lally 2011-05-05) PLID 43481 - remove NexWeb display setup
			"DELETE FROM NexWebDisplayT WHERE ReferringPhysicianID = @nRefPhyID OR PriCarePhysID = @nRefPhyID \r\n"
			//TES 6/10/2011 - PLID 41353 - Referring physicians may have HL7 links
			"DELETE FROM HL7IDLinkT WHERE PersonID = @nRefPhyID \r\n"
			, GetActiveContactID());
}

//this function deletes all data from tables which contain an ID related to a provider, excluding PersonT and ProvidersT
void CMainFrame::DeleteProvider()
{
		//delete records

		//(e.lally 2012-04-06) PLID 48264 - Use a batch for all this
		CParamSqlBatch paramBatch;
		paramBatch.Declare("SET XACT_ABORT ON ");
		paramBatch.Declare("DECLARE @nDeleteProviderID INT ");
		paramBatch.Add("SET @nDeleteProviderID = {INT} ", GetActiveContactID());
		// (j.jones 2009-08-24 12:13) - PLID 35124 - personnel are only in Preference Cards now
		paramBatch.Add("DELETE FROM PreferenceCardDetailsT WHERE PersonID = @nDeleteProviderID");
		// (j.jones 2009-12-14 17:07) - PLID 35124 - changed to PreferenceCardProvidersT
		paramBatch.Add("DELETE FROM PreferenceCardProvidersT WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM MultiFeeProvidersT WHERE ProviderID = @nDeleteProviderID");
		// (j.jones 2008-06-23 10:21) - PLID 30434 - added EligibilitySetupT
		paramBatch.Add("DELETE FROM EligibilitySetupT WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM EbillingSetupT WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM UB92EbillingSetupT WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM InsuranceGroups WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM InsuranceBox24J WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM InsuranceBox51 WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM InsuranceBox31 WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM InsuranceNetworkID WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM InsuranceAcceptedT WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM AdvHCFAPinT WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM ProviderSchedDefDurationDetailT WHERE ProviderSchedDefDurationID IN (SELECT ID FROM ProviderSchedDefDurationT WHERE ProviderID = @nDeleteProviderID)");
		paramBatch.Add("DELETE FROM ProviderSchedDefDurationT WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM QBooksAcctsToProvidersT WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM CredentialedCPTCodesT WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM CredentialedProceduresT WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM PersonCertificationsT WHERE PersonID = @nDeleteProviderID");		
		// (a.wetta 2007-03-30 10:22) - PLID 24872 - Also delete any rule links associated with the provider
		paramBatch.Add("DELETE FROM CommissionRulesLinkT WHERE ProvID = @nDeleteProviderID");
		// (j.jones 2009-11-18 14:57) - PLID 29046 - delete from TieredCommissionRulesLinkT
		paramBatch.Add("DELETE FROM TieredCommissionRulesLinkT WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM CommissionT WHERE ProvID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM ResourceProviderLinkT WHERE ProviderID = @nDeleteProviderID");
		// (j.jones 2008-04-02 14:30) - PLID 28995 - ensure we delete from HCFAClaimProvidersT
		paramBatch.Add("DELETE FROM HCFAClaimProvidersT WHERE ProviderID = @nDeleteProviderID");
		// (j.jones 2009-02-09 09:40) - PLID 32951 - remove CLIA setup
		paramBatch.Add("DELETE FROM CLIANumbersT WHERE ProviderID = @nDeleteProviderID");
		// (j.jones 2010-07-06 15:21) - PLID 39506 - delete from ClearinghouseLoginInfoT
		paramBatch.Add("DELETE FROM ClearinghouseLoginInfoT WHERE ProviderID = @nDeleteProviderID");
		// (z.manning 2011-04-06 10:32) - PLID 42337 - Handle provider float data
		paramBatch.Add("DELETE FROM EmrProviderFloatDataT WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM EmrProviderFloatTableDropdownT WHERE ProviderID = @nDeleteProviderID");
		// (r.gonet 10/31/2011) - PLID 45367 - Delete facility code overrides for specific providers
		paramBatch.Add("DELETE FROM HL7OverrideFacilityCodesT WHERE ProviderID = @nDeleteProviderID");
		// (r.gonet 09/03/2013) - PLID 56007 - Providers are referenced in medication history requests.
		//  Since this is read only data, is easy to reget, and is requested automatically, I decided to
		//  delete the medication history request records rather associated rather than prevent the deletion.
		paramBatch.Add(
			"DELETE FROM MedicationHistoryResponseT WHERE ID IN "
			"( "
				"SELECT MedicationHistoryResponseT.ID "
				"FROM MedicationHistoryResponseT "
				"INNER JOIN RxHistoryRequestT ON MedicationHistoryResponseT.RxHistoryRequestID = RxHistoryRequestT.ID "
				"INNER JOIN RxHistoryMasterRequestT ON RxHistoryRequestT.RxHistoryMasterRequestID = RxHistoryMasterRequestT.ID "
				"WHERE RxHistoryMasterRequestT.ProviderID = @nDeleteProviderID "
			") ");
		paramBatch.Add(
			"DELETE FROM RxHistoryRequestT WHERE ID IN "
			"( "
				"SELECT RxHistoryRequestT.ID "
				"FROM RxHistoryRequestT "
				"INNER JOIN RxHistoryMasterRequestT ON RxHistoryRequestT.RxHistoryMasterRequestID = RxHistoryMasterRequestT.ID "
				"WHERE RxHistoryMasterRequestT.ProviderID = @nDeleteProviderID "
			") ");
		paramBatch.Add(
			"DELETE FROM RxHistoryMasterRequestT WHERE ID IN "
			"( "
				"SELECT RxHistoryMasterRequestT.ID "
				"FROM RxHistoryMasterRequestT "
				"WHERE RxHistoryMasterRequestT.ProviderID = @nDeleteProviderID "
			") ");

		//(e.lally 2012-04-06) PLID 48264 - Delete EMR MU Progress settings
		paramBatch.Add("DELETE FROM ProviderMUMeasureSelectionT WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("DELETE FROM ProviderMUMeasureOptionT WHERE ProviderID = @nDeleteProviderID");
		//(a.wilson 2014-5-5) PLID 61831 - delete records from ChargeLevelProviderConfigT where the provider was in the providerid field.
		paramBatch.Add("DELETE FROM ChargeLevelProviderConfigT WHERE ProviderID = @nDeleteProviderID");

		//fix these records
		paramBatch.Add("UPDATE ServiceT SET ProviderID = NULL WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("UPDATE InsuranceReferralsT SET ProviderID = NULL WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("UPDATE PatientMedications SET ProviderID = NULL WHERE ProviderID = @nDeleteProviderID");		
		paramBatch.Add("UPDATE LocationsT SET DefaultProviderID = NULL WHERE DefaultProviderID = @nDeleteProviderID");
		// (j.jones 2008-06-17 09:03) - PLID 30410 - removed HCFAT from the program
		//ExecuteSql("UPDATE HCFAT SET ProviderID = -1 WHERE ProviderID = @nDeleteProviderID");
		paramBatch.Add("UPDATE HCFADocumentsT SET ProviderIndex = -1 WHERE ProviderIndex = @nDeleteProviderID");
		paramBatch.Add("UPDATE PaymentsT SET ProviderID = NULL WHERE ProviderID = @nDeleteProviderID");
		// (c.haag 2016-05-02 11:00) - NX-100403 - DoctorsProviders can be NULL
		paramBatch.Add("UPDATE ChargesT SET DoctorsProviders = NULL WHERE DoctorsProviders = @nDeleteProviderID");
		// (j.jones 2008-12-11 10:44) - PLID 32407 - supported BillsT.SupervisingProviderID, though I believe you
		// cannot get to this line if there really are bills with this provider
		paramBatch.Add("UPDATE BillsT SET SupervisingProviderID = NULL WHERE SupervisingProviderID = @nDeleteProviderID");
		// (z.manning, 09/15/2006) - PLID 22388 - Although all places that call this function as of now 
		// already prevent you from deleting a provider associated with an EMN, we should not have this
		// line of code just to be safe since it could potentially update saved and even locked EMNs.
		//ExecuteSql("UPDATE EMRMasterT SET ProviderID = NULL WHERE ProviderID = %li", GetActiveContactID());
		paramBatch.Add("UPDATE PalmContactsInfoT SET Status = 4 WHERE PersonID = @nDeleteProviderID");
		paramBatch.Add("UPDATE PatientsT SET MainPhysician = NULL WHERE MainPhysician = @nDeleteProviderID");
		paramBatch.Add("UPDATE BatchPaymentsT SET ProviderID = NULL WHERE ProviderID = @nDeleteProviderID");
		// (z.manning 2009-05-06 17:20) - PLID 28529 - Clear out any references to this provider
		// in referral orders
		paramBatch.Add("UPDATE ReferralOrdersT SET ReferredByID = NULL WHERE ReferredByID = @nDeleteProviderID");
		// (j.jones 2009-06-08 11:53) - PLID 34514 - clear out UsersT.NewCropProviderID
		paramBatch.Add("UPDATE UsersT SET NewCropProviderID = NULL WHERE NewCropProviderID = @nDeleteProviderID");
		// (j.jones 2009-08-24 17:38) - PLID 35203 - and NewCropSupervisingProviderID
		paramBatch.Add("UPDATE UsersT SET NewCropSupervisingProviderID = NULL WHERE NewCropSupervisingProviderID = @nDeleteProviderID");
		// (j.jones 2011-06-17 09:27) - PLID 41709 - and NewCropMidLevelProviderID
		paramBatch.Add("UPDATE UsersT SET NewCropMidLevelProviderID = NULL WHERE NewCropMidLevelProviderID = @nDeleteProviderID");
		// (j.luckoski 2013-03-27 17:23) - PLID 55913 - Set to null when we don't delete the providerID
		paramBatch.Add("UPDATE UsersT SET ProviderID = NULL WHERE ProviderID = @nDeleteProviderID");
		//(r.wilson 7/29/2013) PLID 48684 - and EMRDefaultProviderID column from ProvidersT
		// (b.savon 2013-08-27 15:59) - PLID 58329 - It is @nDeleteProviderID
		paramBatch.Add("Update ProvidersT SET EMRDefaultProviderID = NULL WHERE EMRDefaultProviderID = @nDeleteProviderID");
		//(a.wilson 2014-5-5) PLID 61831 - update records from ChargeLevelProviderConfigT where the provider was in the referring, ordering or supervising provider field.
		paramBatch.Add("Update ChargeLevelProviderConfigT SET ReferringProviderID = NULL WHERE ReferringProviderID = @nDeleteProviderID");
		paramBatch.Add("Update ChargeLevelProviderConfigT SET OrderingProviderID = NULL WHERE OrderingProviderID = @nDeleteProviderID");
		paramBatch.Add("Update ChargeLevelProviderConfigT SET SupervisingProviderID = NULL WHERE SupervisingProviderID = @nDeleteProviderID");

		paramBatch.Execute(GetRemoteData());

		//Moved to the end, after everything has executed
		UpdateASCLicenseToDos();
}

//this function deletes all data from tables which contain an ID related to users, excluding PersonT and UsersT
void CMainFrame::DeleteUser(BOOL bDeleteMessages, long nAssignStepsTo, long nAssignTodosTo, long nAssignEmrTodoActionsTo)
{
	if (bDeleteMessages) {
		ExecuteSql("DELETE FROM MessagesT WHERE SenderID = %li OR RecipientID = %li", GetActiveContactID(), GetActiveContactID());
	}
	else {
		ExecuteSql("UPDATE MessagesT SET SenderID = NULL WHERE SenderID = %li", GetActiveContactID());
		ExecuteSql("UPDATE MessagesT SET RecipientID = NULL WHERE RecipientID = %li", GetActiveContactID());
	}

	//delete records

	// (c.haag 2005-10-10 15:22) - PLID 17847 - Delete NexPDA info
	ExecuteSql("DELETE FROM OutlookUpdateT WHERE FolderID IN (SELECT ID FROM OutlookFolderT WHERE UserID = %d)", GetActiveContactID());
	ExecuteSql("DELETE FROM OutlookAirlockT WHERE FolderID IN (SELECT ID FROM OutlookFolderT WHERE UserID = %d)", GetActiveContactID());
	ExecuteSql("DELETE FROM OutlookAddressBookT WHERE UserID = %d", GetActiveContactID());
	ExecuteSql("DELETE FROM OutlookAptResourceT WHERE CategoryID IN (SELECT ID FROM OutlookCategoryT WHERE UserID = %d)", GetActiveContactID());
	ExecuteSql("DELETE FROM OutlookAptTypeT WHERE CategoryID IN (SELECT ID FROM OutlookCategoryT WHERE UserID = %d)", GetActiveContactID());
	ExecuteSql("DELETE FROM OutlookCalendarT WHERE CategoryID IN (SELECT ID FROM OutlookCategoryT WHERE UserID = %d)", GetActiveContactID());
	ExecuteSql("DELETE FROM OutlookCategoryT WHERE UserID = %d", GetActiveContactID());
	// (z.manning 2009-10-29 15:08) - PLID 35428 - Delete from NexSyncEventsT
	ExecuteSql("DELETE FROM NexSyncEventsT WHERE OutlookFolderID IN (SELECT ID FROM OutlookFolderT WHERE UserID = %d)", GetActiveContactID());
	ExecuteSql("DELETE FROM OutlookFolderT WHERE UserID = %d", GetActiveContactID());
	ExecuteSql("DELETE FROM OutlookProfileT WHERE UserID = %d", GetActiveContactID());
	// (j.jones 2009-08-24 12:13) - PLID 35124 - personnel are only in Preference Cards now
	ExecuteParamSql("DELETE FROM PreferenceCardDetailsT WHERE PersonID = {INT}", GetActiveContactID());
	ExecuteSql("DELETE FROM UserPermissionsT WHERE UserPermissionsT.ID = %li", GetActiveContactID());
	ExecuteSql("DELETE FROM UserLocationT WHERE UserLocationT.PersonID = %li", GetActiveContactID());
	// (c.haag 2008-06-11 17:55) - PLID 11599 - Changed statement to use new todo table structure.
	// (c.haag 2008-07-02 16:15) - Don't set the ID's to null unless the user we're deleting is the only
	// one assigned to them
	ExecuteParamSql("UPDATE EMRMasterT SET FollowUpTaskID = NULL "
		"WHERE FollowUpTaskID IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo = {INT}) "
		"AND FollowUpTaskID NOT IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo <> {INT})", 
		GetActiveContactID(), GetActiveContactID());
	ExecuteParamSql("UPDATE EMRMasterT SET ProcedureTaskID = NULL "
		"WHERE ProcedureTaskID IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo = {INT}) "
		"AND ProcedureTaskID NOT IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo <> {INT})",
		GetActiveContactID(), GetActiveContactID());
// (a.walling 2006-09-11 14:06) - PLID 22458 - Reassign rather than delete todos
//	ExecuteSql("DELETE FROM ToDoList WHERE ToDoList.AssignTo = %li", GetActiveContactID());
	ExecuteSql("DELETE FROM ProductResponsibilityT WHERE UserID = %li", GetActiveContactID());
	ExecuteSql("DELETE FROM UserTimesT WHERE UserID = %li", GetActiveContactID());
	ExecuteSql("DELETE FROM ChatMessagesT WHERE SenderID = %li", GetActiveContactID());
	ExecuteSql("DELETE FROM PalmRecordT WHERE PalmSettingsTID IN (SELECT ID FROM PalmSettingsT WHERE UserID = %li)", GetActiveContactID());
	ExecuteSql("DELETE FROM PalmSettingsAptTypeT WHERE PalmSettingsTID IN (SELECT ID FROM PalmSettingsT WHERE UserID = %li)", GetActiveContactID());
	ExecuteSql("DELETE FROM PalmSettingsResourceT WHERE PalmSettingsTID IN (SELECT ID FROM PalmSettingsT WHERE UserID = %li)", GetActiveContactID());
	ExecuteSql("DELETE FROM PalmSyncT WHERE PalmSettingsTID IN (SELECT ID FROM PalmSettingsT WHERE UserID = %li)", GetActiveContactID());
	ExecuteSql("DELETE FROM PalmSettingsT WHERE UserID = %li", GetActiveContactID());
	ExecuteSql("DELETE FROM MergeCommandT WHERE UserID = %li", GetActiveContactID());
	ExecuteSql("DELETE FROM BookmarkCommandT WHERE UserID = %li", GetActiveContactID());
	// (j.jones 2005-03-03 17:52) - not currently used on users but may be at some point, so leave it in
	ExecuteSql("DELETE FROM PersonCertificationsT WHERE PersonID = %li", GetActiveContactID());
	// (a.wetta 2007-01-11 14:34) - PLID 16065 - Delete link to resource
	ExecuteSql("DELETE FROM ResourceUserLinkT WHERE UserID = %li", GetActiveContactID());
	UpdateASCLicenseToDos();
	//DRT 7/29/2008 - PLID 30524 - Delete the fax configuration info for this user.
	ExecuteParamSql("DELETE FROM FaxConfigT WHERE UserID = {INT}", GetActiveContactID());
	// (j.dinatale 2010-10-29) - PLID 28773 - Need to delete Billing Tab Column Width information for the user
	ExecuteParamSql("DELETE FROM BillingColumnsT WHERE UserID = {INT} \r\n"
		"DELETE FROM RespTypeColumnWidthsT WHERE UserID = {INT}", GetActiveContactID(), GetActiveContactID());
	// (j.gruber 2012-06-19 16:26) - PLID 44026 - delete the users dashboard
	ExecuteParamSql("DELETE FROM PatientDashboardUserConfigT WHERE UserID = {INT}", GetActiveContactID());
	

	// (j.jones 2005-01-31 17:20) - if this user was the default ordering user, reset the value
	long nDefProductUserID = GetRemotePropertyInt("DefaultProductOrderingUser",-1,0,"<None>",TRUE);
	if(nDefProductUserID == GetActiveContactID()) {
		SetRemotePropertyInt("DefaultProductOrderingUser",-1,0,"<None>");
	}
	
	// (s.dhole 2010-10-05 11:51) - PLID 40690 Delete all preferences
	// need to cosider "-" if sobudy added in user id
	{
		CString strSqlBatch = BeginSqlBatch();
		CNxParamSqlArray aryParams;
		//Type tables
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
			"DECLARE @UserName VARCHAR(100) \r\n"
			" Select @UserName = UserName FROM UsersT WHERE  PersonID = {INT} \r\n"
			" DELETE FROM ConfigRT WHERE UserName = @UserName  \r\n"
			" DELETE from ConfigRT where username like @Username + '-%' and username not like @Username + '-'  and username not like  @Username + '--%' \r\n"
			, GetActiveContactID());
		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
	}
	//fix these records
	ExecuteSql("UPDATE PersonT SET WarningUserID = NULL WHERE PersonT.WarningUserID = %li", GetActiveContactID());
	ExecuteSql("UPDATE PersonT SET UserID = NULL WHERE PersonT.UserID = %li", GetActiveContactID());
	ExecuteSql("UPDATE OrderDetailsT SET ModifiedBy = NULL WHERE OrderDetailsT.ModifiedBy = %li", GetActiveContactID());
	ExecuteSql("UPDATE Notes SET UserID = NULL WHERE Notes.UserID = %li", GetActiveContactID());
	ExecuteSql("UPDATE BillsT SET InputName = NULL WHERE BillsT.InputName = %li", GetActiveContactID());
	ExecuteSql("UPDATE BillsT SET PatCoord = NULL WHERE BillsT.PatCoord = %li", GetActiveContactID());
	ExecuteSql("UPDATE ChargesT SET PatCoordID = NULL WHERE PatCoordID = %li", GetActiveContactID());
	ExecuteSql("UPDATE OrderT SET CreatedBy = NULL WHERE OrderT.CreatedBy = %li", GetActiveContactID());
	ExecuteSql("UPDATE ReturnedProductsT SET UserID = NULL WHERE ReturnedProductsT.UserID = %li", GetActiveContactID());
	ExecuteSql("UPDATE ProductAdjustmentsT SET Login = NULL WHERE ProductAdjustmentsT.Login = %li", GetActiveContactID());
	ExecuteSql("UPDATE ProductLocationTransfersT SET UserID = NULL WHERE ProductLocationTransfersT.UserID = %li", GetActiveContactID(), GetActiveContactID());
	ExecuteSql("UPDATE ToDoList SET EnteredBy = NULL WHERE ToDoList.EnteredBy = %li", GetActiveContactID());
	ExecuteSql("UPDATE PatientsT SET EmployeeID = NULL WHERE PatientsT.EmployeeID = %li", GetActiveContactID());
	// (r.gonet 09/03/2013) - PLID 56007 - null out the requesting user on the med history requests.
	ExecuteParamSql("UPDATE RxHistoryMasterRequestT SET RequestedByUserID = NULL WHERE RxHistoryMasterRequestT.RequestedByUserID = {INT}", GetActiveContactID());

	// (a.walling 2006-09-11 14:06) - PLID 22458 - Reassign rather than delete todos
	// if nAssignTodosTo is -1, this will fail the foreign key constraint and raise an error
	// (c.haag 2008-06-11 12:21) - PLID 30321 - Transfer the todo alarms using a global utility
	if (nAssignTodosTo > -1) {
		TodoTransferAssignTo(GetActiveContactID(), nAssignTodosTo);
	}

	// (c.haag 2008-06-23 13:06) - PLID 30471 - Reassign EMR todo action users
	if (nAssignEmrTodoActionsTo > -1) {
		TodoTransferEmrActionAssignTo(GetActiveContactID(), nAssignEmrTodoActionsTo);
	}

	if(nAssignStepsTo != -1) {
		CString strSqlBatch;
		AddStatementToSqlBatch(strSqlBatch, "UPDATE LaddersT SET UserID = %li WHERE UserID = %li", nAssignStepsTo, GetActiveContactID());
		// (j.jones 2008-11-26 15:30) - PLID 30830 - changed to support multiple users per step
		AddStatementToSqlBatch(strSqlBatch, "UPDATE StepTemplatesAssignToT SET UserID = %li WHERE UserID = %li", nAssignStepsTo, GetActiveContactID());
		AddStatementToSqlBatch(strSqlBatch, "UPDATE StepsAssignToT SET UserID = %li WHERE UserID = %li", nAssignStepsTo, GetActiveContactID());
		// (z.manning 2008-10-13 08:40) - PLID 21108 - Lab step user ID
		AddStatementToSqlBatch(strSqlBatch, "UPDATE LabProcedureStepTodoAssignToT SET UserID = %li WHERE UserID = %li", nAssignStepsTo, GetActiveContactID());
		ExecuteSqlBatch(strSqlBatch);
	}
	else {
		CString strSqlBatch;
		AddStatementToSqlBatch(strSqlBatch, "UPDATE LaddersT SET UserID = NULL WHERE UserID = %li", GetActiveContactID());
		// (j.jones 2008-11-26 15:30) - PLID 30830 - changed to support multiple users per step
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM StepTemplatesAssignToT WHERE UserID = %li", GetActiveContactID());
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM StepsAssignToT WHERE UserID = %li", GetActiveContactID());
		// (z.manning 2008-10-13 08:40) - PLID 21108 - Lab step user ID
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM LabProcedureStepTodoAssignToT WHERE UserID = %li", GetActiveContactID());
		ExecuteSqlBatch(strSqlBatch);
	}


//no longer needed in new audit structure
//	ExecuteSql("UPDATE AuditDetailsT SET PersonID = NULL WHERE AuditDetailsT.PersonID = %li", GetActiveContactID());

}

//this function deletes all data from tables which contain an ID related to suppliers, excluding PersonT and SupplierT
void CMainFrame::DeleteSupplier()
{
	ExecuteSql("UPDATE ProductT SET DefaultMultiSupplierID = NULL WHERE DefaultMultiSupplierID IN (SELECT ID FROM MultiSupplierT WHERE SupplierID = %li)", GetActiveContactID());
	ExecuteSql("DELETE FROM MultiSupplierT WHERE SupplierID = %li", GetActiveContactID());	
}

//this function deletes all data from tables which contain an ID related to other contacts, excluding PersonT and ContactsT
void CMainFrame::DeleteOtherContact()
{	
	ExecuteSql("DELETE FROM Tops_AnesthesiaSupervisorLinkT WHERE PracPersonID = %li", GetActiveContactID());
	// (j.jones 2009-08-24 12:13) - PLID 35124 - personnel are only in Preference Cards now
	ExecuteParamSql("DELETE FROM PreferenceCardDetailsT WHERE PersonID = {INT}", GetActiveContactID());
	ExecuteSql("DELETE FROM PersonCertificationsT WHERE PersonID = %li", GetActiveContactID());
	UpdateASCLicenseToDos();

	ExecuteSql("UPDATE ProcedureT SET NurseID = NULL WHERE NurseID = %li", GetActiveContactID());
	ExecuteSql("UPDATE ProcedureT SET AnesthesiologistID = NULL WHERE AnesthesiologistID = %li", GetActiveContactID());
	ExecuteSql("UPDATE ProcInfoT SET NurseID = NULL WHERE NurseID = %li", GetActiveContactID());
	ExecuteSql("UPDATE ProcInfoT SET AnesthesiologistID = NULL WHERE AnesthesiologistID = %li", GetActiveContactID());
	
}

CTableChecker g_ToDoChecker(NetUtils::TodoList);

//this function deletes the information that is common to all types of contacts
void CMainFrame::DeleteCommonContactInfo()
{	
		// (z.manning, 02/19/2008) - PLID 28216 - Need to delete from attendance-todo linking table
		if(IsNexTechInternal()) {
			ExecuteParamSql("DELETE FROM AttendanceToDoLinkT WHERE TodoID = {INT}", GetActiveContactID());
		}
		//(e.lally 2005-11-28) PLID 18460 - We need to delete custom data that references the contact too.
		// (c.haag 2008-06-11 08:39) - PLID 30328 - Use the new global function to delete todo's
		TodoDelete(FormatString("PersonID = %d", GetActiveContactID()));
		// (a.walling 2010-08-02 12:11) - PLID 39867 - Moved Notes metadata to NoteInfoT
		ExecuteSql("DELETE FROM NoteDataT WHERE PersonID = %li", GetActiveContactID());
		ExecuteSql("DELETE FROM CustomFieldDataT WHERE CustomFieldDataT.PersonID = %li "
			"OR (CustomFieldDataT.IntParam = %li AND FieldID = %li)", GetActiveContactID(), GetActiveContactID(), 31);
		ExecuteSql("DELETE FROM GroupDetailsT WHERE GroupDetailsT.PersonID = %li", GetActiveContactID());
		// (j.jones 2008-09-04 17:45) - PLID 30288 - supported MailSentNotesT
		// (c.haag 2009-10-12 12:43) - PLID 35722 - We now contain deleting MailSent records in a function
		DeleteMailSentRecords(FormatString("SELECT MailID FROM MailSent WHERE PersonID = %li", GetActiveContactID()));
		ExecuteSql("UPDATE PalmContactsInfoT SET Status = 4 WHERE PersonID = %li", GetActiveContactID());
		
		// (j.jones 2014-08-04 17:32) - PLID 63141 - send an Ex tablechecker with this person ID, but no MailID,
		// don't bother with the photo status since contacts do not have photos
		CClient::RefreshMailSentTable(GetActiveContactID(), -1);

		// (c.haag 2010-06-10 17:19) - PLID 39057 - Send table checkers. Because todo alarms can now link with MailSent records,
		// we have to send those as well.
		CClient::RefreshTable(NetUtils::TodoList, -1);

		//DRT 3/11/03 - since we've cleared out todolist, send a table checker message to update the alarm if they happen to have it open
		g_ToDoChecker.Refresh();
}

void CMainFrame::OnPaperBatch()
{
	// (j.jones 2010-06-23 10:33) - PLID 39297 - check this license, do not "use", because
	// actually entering the tab will "use" it
	if(!g_pLicense->CheckForLicense(CLicense::lcHCFA, CLicense::cflrSilent)) {
		MsgBox("You are not licensed to access this feature.");
		return;
	}

	if (UserPermission(FinancialModuleItem)) {
		OpenModule(FINANCIAL_MODULE_NAME);
		CNxTabView* pView = (CNxTabView *)GetOpenView(FINANCIAL_MODULE_NAME);
		if (pView) 
		{	if(pView->GetActiveTab() != FinancialModule::PaperBatchTab)
				pView->SetActiveTab(FinancialModule::PaperBatchTab);
		}
	}
}

// (a.walling 2010-01-28 16:04) - PLID 37107 - Support fallback to legacy TWAIN 1.9
void CMainFrame::OnToggleTWAINUseLegacy()
{
	if (m_pMenu = GetMenu()) {
		UINT state = m_pMenu->GetMenuState(ID_TOOLS_TWAIN_USELEGACY, MF_BYCOMMAND);

		if (state == 0xFFFFFFFF) return;

		// (j.armen 2011-10-26 16:38) - PLID 46136 - References to ConfigRT
		// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
		// than the license object.
		CString strPath = g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT);

		if (state == MF_UNCHECKED) {// is unchecked
			m_pMenu->CheckMenuItem(ID_TOOLS_TWAIN_USELEGACY, MF_CHECKED);
			SetRemotePropertyInt("TWAIN_UseTwain2", FALSE, 0, strPath);
		} else if (state == MF_CHECKED) {// is checked
			m_pMenu->CheckMenuItem(ID_TOOLS_TWAIN_USELEGACY, MF_UNCHECKED);
			SetRemotePropertyInt("TWAIN_UseTwain2", TRUE, 0, strPath);
		}

		MessageBox("Your changes will take effect when you restart Practice.", NULL, MB_ICONINFORMATION);
	}
}

void CMainFrame::OnToggleTWAINShowUI()
{
	if (m_pMenu = GetMenu()) {
		UINT state = m_pMenu->GetMenuState(ID_TOOLS_TWAIN_SHOWTWAINUI, MF_BYCOMMAND);

		if (state == 0xFFFFFFFF) return;

		if (state == MF_UNCHECKED) {// is unchecked
			m_pMenu->CheckMenuItem(ID_TOOLS_TWAIN_SHOWTWAINUI, MF_CHECKED);
			SetPropertyInt("TWAINShowUI", 1);
		} else if (state == MF_CHECKED) {// is checked
			m_pMenu->CheckMenuItem(ID_TOOLS_TWAIN_SHOWTWAINUI, MF_UNCHECKED);
			SetPropertyInt("TWAINShowUI", 0);
		}
	}
}

void CMainFrame::OnToggleShowG1Images()
{
	if (m_pMenu = GetMenu()) {
		UINT state = m_pMenu->GetMenuState(ID_VIEW_SHOWIMAGESINGENERAL1, MF_BYCOMMAND);

		if (state == 0xFFFFFFFF) return;

		if (state == MF_UNCHECKED) {// is unchecked
			m_pMenu->CheckMenuItem(ID_VIEW_SHOWIMAGESINGENERAL1, MF_CHECKED);
			SetPropertyInt("PracticeShowImages", 1);
		} else if (state == MF_CHECKED) {// is checked
			m_pMenu->CheckMenuItem(ID_VIEW_SHOWIMAGESINGENERAL1, MF_UNCHECKED);
			SetPropertyInt("PracticeShowImages", 0);
		}
	}
	if (GetActiveView())
		GetActiveView()->UpdateView();
}

void CMainFrame::OnToggleShowMirrorImages()
{
	try {
		if (m_pMenu = GetMenu()) {
			UINT state = m_pMenu->GetMenuState(ID_VIEW_SHOWMIRRORIMAGESINGENERAL1, MF_BYCOMMAND);

			if (state == 0xFFFFFFFF) return;

			//TES 11/17/2009 - PLID 35709 - Just toggle our ConfigRT setting
			if (state == MF_UNCHECKED) {// is unchecked
				m_pMenu->CheckMenuItem(ID_VIEW_SHOWMIRRORIMAGESINGENERAL1, MF_CHECKED);
				SetPropertyInt("ShowMirrorImagesInG1", 1);
			} else if (state == MF_CHECKED) {// is checked
				m_pMenu->CheckMenuItem(ID_VIEW_SHOWMIRRORIMAGESINGENERAL1, MF_UNCHECKED);
				SetPropertyInt("ShowMirrorImagesInG1", 0);
			}
		}
		if (GetActiveView())
			GetActiveView()->UpdateView();
	}NxCatchAll("ERror in CMainFrame::OnToggleShowMirrorImages()");
}

void CMainFrame::OnToggleShowG2WarningDetails()
{
	if (m_pMenu = GetMenu()) {
		UINT state = m_pMenu->GetMenuState(ID_VIEW_SHOWWARNINGDETAILSINGENERAL2, MF_BYCOMMAND);

		if (state == 0xFFFFFFFF) return;

		if (state == MF_UNCHECKED) {// is unchecked
			m_pMenu->CheckMenuItem(ID_VIEW_SHOWWARNINGDETAILSINGENERAL2, MF_CHECKED);
			SetPropertyInt("G2ShowWarningStats", 1);
		} else if (state == MF_CHECKED) {// is checked
			m_pMenu->CheckMenuItem(ID_VIEW_SHOWWARNINGDETAILSINGENERAL2, MF_UNCHECKED);
			SetPropertyInt("G2ShowWarningStats", 0);
		}
	}
	if (GetActiveView())
		GetActiveView()->UpdateView();
}

void CMainFrame::OnToggleShowPrimaryOnly()
{
	if (m_pMenu = GetMenu()) {
		UINT state = m_pMenu->GetMenuState(ID_VIEW_SHOWPRIMARYIMAGEONLY, MF_BYCOMMAND);

		if (state == 0xFFFFFFFF) return;

		if (state == MF_UNCHECKED) {// is unchecked
			m_pMenu->CheckMenuItem(ID_VIEW_SHOWIMAGESINGENERAL1, MF_CHECKED);
			SetPropertyInt("PracticeShowPrimaryImageOnly", 1);
		} else if (state == MF_CHECKED) {// is checked
			m_pMenu->CheckMenuItem(ID_VIEW_SHOWIMAGESINGENERAL1, MF_UNCHECKED);
			SetPropertyInt("PracticeShowPrimaryImageOnly", 0);
		}
	}
	if (GetActiveView())
		GetActiveView()->UpdateView();
}

void CMainFrame::OnToggleCallerID()
{
	//OnTestCallerID();
	//return;

	m_pMenu = GetMenu();

	if (m_pMenu) {
		UINT state = m_pMenu->GetMenuState(ID_TOOLS_ENABLECALLERID, MF_BYCOMMAND);

		if (state == 0xFFFFFFFF) return;

		if (state == MF_UNCHECKED) {// is unchecked
			m_pMenu->CheckMenuItem(ID_TOOLS_ENABLECALLERID, MF_CHECKED);
			SetPropertyInt ("UseCallerID", 1);
		//	if (!SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
			if(!SUCCEEDED(CoInitialize(0)))
			{
				return;
			}

			// do all tapi initialization
			if (S_OK != InitializeTapi())
			{
				return;
			}

		} else if (state == MF_CHECKED) {// is checked
			m_pMenu->CheckMenuItem(ID_TOOLS_ENABLECALLERID, MF_UNCHECKED);
			SetPropertyInt ("UseCallerID", 0);
			// clean up tapi
			ShutdownTapi();
		}
	}

}

// (j.gruber 2007-08-16 13:08) - PLID 27091 - Tracking Conversions Config
void CMainFrame::OnTrackingConversionsConfigDlg() {

	try {
		CTrackingConversionConfigDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll("Error in CMainFrame::OnTrackingConversionsConfigDlg");


}

// (j.gruber 2007-07-09 17:07) - PLID 26584 - Credit Card Batch Processing
void CMainFrame::OnCreditCardBatchProcessing() {

	// (a.walling 2007-08-03 09:17) - PLID 26899 - Check for license
	// (d.thompson 2010-09-02) - PLID 40371 - Valid for any cc licensing (although I think this is dead code)
	if (!g_pLicense || !g_pLicense->HasCreditCardProc_Any(CLicense::cflrUse))
		return;

	// (a.walling 2007-08-03 17:25) - PLID 26922 - Check permissions, prompt for PW if needed
	// (d.thompson 2009-07-01) - PLID 34230 - Removed these permissions, this dialog should not be in live use
	AfxThrowNxException("CCProcessingSummary is not valid for use.");
	//if (!CheckCurrentUserPermissions(bioCCBatchDialog, sptView))
	//	return;
	
	CCreditCardProcessingSummaryDlg dlg(this);
	dlg.DoModal();
}

// (j.gruber 2007-07-03 09:36) - PLID 26535 - Credit Card Processing Setup
void CMainFrame::OnCreditCardProcessingSetup()
{
	try {
		// (d.thompson 2010-09-02) - PLID 40371 - Check each licensing option here to determine which setup dialog is displayed

		// (a.walling 2007-08-03 09:17) - PLID 26899 - Check for license
		if (g_pLicense && g_pLicense->HasCreditCardProc_QBMS(CLicense::cflrUse)) {

			// (d.thompson 2009-07-02) - PLID 34690 - Permission
			if(!CheckCurrentUserPermissions(bioCCProcessingSetup, sptView)) {
				return;
			}

			CQBMSProcessingSetupDlg dlg(this);
			dlg.DoModal();
		}
		else if (g_pLicense && g_pLicense->HasCreditCardProc_Chase(CLicense::cflrUse)) {
			// (d.lange 2010-09-08 14:00) - PLID 40309 - If the Chase license is enabled, display the setup dialog
			CChaseProcessingSetupDlg dlg(this);
			dlg.DoModal();
		}
		else {
			//unknown, failure
			AfxThrowNxException("Invalid credit card licensing type setup.");
		}
	} NxCatchAll("Error in OnCreditCardProcessingSetup");
}

// (j.camacho 2013-07-24 12:25) - PLID 57518
void CMainFrame::OnCareCreditSetupDlg() {

	try {
		CCareCreditSetupDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll("Error in CMainFrame::OnCareCreditSetupDlg");


}

// (j.camacho 2013-07-24 12:25) - PLID 58678
void CMainFrame::OnUMLSLoginDlg() {

	try {
		CUMLSLoginDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll("Error in CMainFrame::CUMLSLoginDlg");


}

void CMainFrame::OnUpdateBarcodeID(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdateSwiperID(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdateUnitedID(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(g_pLicense->CheckForLicense(CLicense::lcUnited, CLicense::cflrSilent));
}

void CMainFrame::OnUpdatePastApptsID(CCmdUI* pCmdUI)
{
	//use FFA since that will never be disabled unless the license forbids it
	pCmdUI->Enable(m_enableViews & TB_FFA);
}

// (j.armen 2011-10-25 11:21) - PLID 46136 - GetPracPath is referencing ConfigRT
void CMainFrame::OnUpdateCallerID(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
	// than the license object.
	CString strUserParam = g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT);
	if (GetRemotePropertyInt ("UseCallerID", 0, 0, strUserParam, false)) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void CMainFrame::OnUpdatePalmSettings(CCmdUI* pCmdUI)
{
	BOOL bEnabled = g_pLicense->GetPalmCountAllowed() > 0;
	if(bEnabled) {
		pCmdUI->Enable(TRUE);
	}
	else {
		RemoveEntryFromSubMenu(GetMainFrame()->GetMenu(), "Tools", ID_TOOLS_PALMSETTINGS, FALSE);
	}
}

// (z.manning 2009-04-09 11:02) - PLID 33934
void CMainFrame::OnUpdateImportEmrStandardContentMenu(CCmdUI* pCmdUI)
{
	try
	{
		if(g_pLicense->CheckForLicense(CLicense::lcEMRStandard, CLicense::cflrSilent)) {
			pCmdUI->Enable(TRUE);
		}
		else {
			// (z.manning 2009-04-09 11:06) - PLID 33934 - Since EMR Standard isn't something we're really
			// pushing and likely not too many clients will have it, I'm going to hide the menu option if
			// the client doesn't have a license for it.
			RemoveEntryFromSubMenu(GetMainFrame()->GetMenu(), "Tools", ID_TOOLS_IMPORTEMRSTANDARDCONTENT, FALSE);
		}

	}NxCatchAll("CMainFrame::OnUpdateImportEmrStandardContentMenu");
}

// (a.walling 2010-01-28 16:06) - PLID 37107 - Support fallback to legacy TWAIN 1.9
// (j.armen 2011-10-25 11:19) - PLID 46136 - GetPracPath is referencing ConfigRT
void CMainFrame::OnUpdateTWAINUseLegacy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
	// than the license object.
	CString strUserParam = g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT);
	if (GetRemotePropertyInt("TWAIN_UseTwain2", 1, 0, strUserParam, false)) {
		pCmdUI->SetCheck(0);
	} else {
		pCmdUI->SetCheck(1);
	}
}

// (j.armen 2011-10-25 11:22) - PLID 46136 - GetPracPath is referencing ConfigRT
void CMainFrame::OnUpdateShowTWAINUI(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
	// than the license object.
	CString strUserParam = g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT);
	if (GetRemotePropertyInt("TWAINShowUI", 1, 0, strUserParam, false)) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void CMainFrame::OnUpdateShowG1Images(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	if (GetPropertyInt("PracticeShowImages", 1, 0)) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void CMainFrame::OnUpdateShowMirrorImages(CCmdUI* pCmdUI)
{
	try {
		//TES 11/17/2009 - PLID 35709 - First, check for the license
		if(g_pLicense->CheckForLicense(CLicense::lcMirror, CLicense::cflrSilent)) {
			//TES 11/17/2009 - PLID 35709 - If they're not showing Mirror images at all,
			// then disable the menu item (but still go ahead and display the checkmark,
			// it will apply if and when they decide to show images again).
			if(GetPropertyInt("MirrorImageDisplay", 1, 0)) {
				pCmdUI->Enable(TRUE);
			}
			else {
				pCmdUI->Enable(FALSE);
			}
			//TES 11/17/2009 - PLID 35709 - Check our ConfigRT setting (defaults to the PracticeShowImages value)
			if (GetPropertyInt("ShowMirrorImagesInG1", GetPropertyInt("PracticeShowImages",1,0), 0)) {
				pCmdUI->SetCheck(1);
			}
			else {
				pCmdUI->SetCheck(0);
			}
		}
		else {
			//TES 11/17/2009 - PLID 35709 - They're not licensed, so just disable and uncheck.
			pCmdUI->Enable(FALSE);
			pCmdUI->SetCheck(0);
		}
	}NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnUpdateShowG2WarningDetails(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	if (GetPropertyInt("G2ShowWarningStats", 1, 0)) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void CMainFrame::OnUpdateShowPrimaryImage(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	if (GetPropertyInt("PracticeShowPrimaryImageOnly", 0, 0)) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void CMainFrame::OnUpdateTestCallerID(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdatePracYakker(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(m_pdlgMessager != NULL ? m_pdlgMessager->m_bEnabled : FALSE);
}

void CMainFrame::OnUpdatePocketPC(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

// (z.manning 2009-08-26 14:37) - PLID 35345
void CMainFrame::OnUpdateNexSyncSettings(CCmdUI* pCmdUI)
{
	try
	{
		pCmdUI->Enable(TRUE);

	}NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnUpdateAutoCaller(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}
	

/////////////////////////////////////
//  TAPI and Caller ID Functions   //
/////////////////////////////////////

//global TAPI variables
ITTAPI *                gpTapi;
ITBasicCallControl *    gpCall;
CTAPIEventNotification      * gpTAPIEventNotification;
ULONG                         gulAdvise;

//////////////////////////////////////////////////////////////
// InitializeTapi
//
// Various tapi initializations
///////////////////////////////////////////////////////////////

HRESULT
CMainFrame::InitializeTapi()
{
    HRESULT         hr;

    //
    // cocreate the TAPI object
    //

    hr = CoCreateInstance(
                          CLSID_TAPI,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ITTAPI,
                          (LPVOID *)&gpTapi
                         );

    if ( FAILED(hr) )
    {
        AfxMessageBox("CoCreateInstance on TAPI failed");
        return hr;
    }

    //
    // call initialize.  this must be called before
    // any other tapi functions are called.
    //

    hr = gpTapi->Initialize();

    if ( FAILED(hr) )
    {
        AfxMessageBox("TAPI failed to initialize");

        gpTapi->Release();
        gpTapi = NULL;
        
        return hr;
    }

    //
    // Create our own event notification object and register it
    // see callnot.h and callnot.cpp
    //

    gpTAPIEventNotification = new CTAPIEventNotification;
    

    hr = RegisterTapiEventInterface();

    // Set the Event filter to only give us only the events we process
	// (b.cardillo 2003-10-17 14:17) - Notice we include all known events here, and then we just let the 
	// event handler filter out the ones we don't care about.  The reason is that if we exclude certain 
	// events here, then other events (events we want) will automatically get excluded occasionally for 
	// some reason.  For example, if we don't include TE_CALLHUB we only get every third TE_CALLSTATE.
    gpTapi->put_EventFilter(
				TE_TAPIOBJECT |
				TE_ADDRESS |
				TE_CALLNOTIFICATION |
				TE_CALLSTATE |
				TE_CALLMEDIA |
				TE_CALLHUB |
				TE_CALLINFOCHANGE |
				TE_PRIVATE |
				TE_REQUEST |
				TE_AGENT |
				TE_AGENTSESSION |
				TE_QOSEVENT |
				TE_AGENTHANDLER |
				TE_ACDGROUP |
				TE_QUEUE |
				TE_DIGITEVENT |
				TE_GENERATEEVENT |
				TE_ASRTERMINAL |
				TE_TTSTERMINAL |
				TE_FILETERMINAL |
				TE_TONETERMINAL |
				TE_PHONEEVENT |
				TE_TONEEVENT |
				TE_GATHERDIGITS |
				TE_ADDRESSDEVSPECIFIC |
				TE_PHONEDEVSPECIFIC 
		);
        

    //
    // find all address objects that
    // we will use to listen for calls on
    //

    hr = ListenOnAddresses();

    if ( FAILED(hr) )
    {
        AfxMessageBox("Could not find any addresses to listen on");

        gpTapi->Release();
        gpTapi = NULL;

        return hr;
    }

    return S_OK;
}


///////////////////////////////////////////////////////////////
// ShutdownTapi
///////////////////////////////////////////////////////////////

void
CMainFrame::ShutdownTapi()
{
    //
    // if there is still a call, release it
    //

    if (NULL != gpCall)
    {
        gpCall->Release();
        gpCall = NULL;
    }

    //
    // release main object.
    //

    if (NULL != gpTapi)
    {
        gpTapi->Shutdown();
        gpTapi->Release();
		gpTapi = NULL;
    }

	if(NULL != gpTAPIEventNotification) {
		delete gpTAPIEventNotification;
		gpTAPIEventNotification = NULL;
	}

}

///////////////////////////////////////////////////////////////////////////
// RegisterTapiEventInterface()
///////////////////////////////////////////////////////////////////////////

HRESULT
CMainFrame::RegisterTapiEventInterface()
{
    HRESULT                       hr = S_OK;
    IConnectionPointContainer   * pCPC;
    IConnectionPoint            * pCP;
    

    hr = gpTapi->QueryInterface(
                                IID_IConnectionPointContainer,
                                (void **)&pCPC
                               );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pCPC->FindConnectionPoint(
                                   IID_ITTAPIEventNotification,
                                   &pCP
                                  );

    pCPC->Release();
        
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pCP->Advise(
                      gpTAPIEventNotification,
                      &gulAdvise
                     );

    pCP->Release();

    
    return hr;

}

CString GetTapiAddressName(ITAddress *pAddress)
{
	BSTR bstrName;
	HRESULT hr = pAddress->get_AddressName(&bstrName);
	if (hr == S_OK) {
		return (LPCTSTR)_bstr_t(bstrName, false);
	} else {
		return "<unknown address>";
	}
}

////////////////////////////////////////////////////////////////////////
// ListenOnAddresses
//
// This procedure will find all addresses that support audioin and audioout
// and will call ListenOnThisAddress to start listening on it.
////////////////////////////////////////////////////////////////////////

HRESULT
CMainFrame::ListenOnAddresses()
{
    HRESULT             hr = S_OK;
    IEnumAddress *      pEnumAddress;
    ITAddress *         pAddress;

    //
    // enumerate the addresses
    //

    hr = gpTapi->EnumerateAddresses( &pEnumAddress );

    if ( FAILED(hr) )
    {
        return hr;
    }

    while ( TRUE )
    {
        //
        // get the next address
        //

        hr = pEnumAddress->Next( 1, &pAddress, NULL );

        if (S_OK != hr)
        {
            break;
        }

        //
        // does the address support audio?
        //

		BSTR strAddress;
		pAddress->get_AddressName(&strAddress);

		if ( AddressSupportsMediaType(pAddress, TAPIMEDIATYPE_AUDIO) )
        {
            //
            // If it does then we'll listen.
            //

            hr = ListenOnThisAddress( pAddress );

            if ( FAILED(hr) )
            {

/*				if(hr & E_POINTER)
					DoMessage(L"E_POINTER - The plRegister parameter is not a valid pointer.");

//				if(hr & TAPI_E_NOT_INITIALIZED)
//					DoMessage(L"TAPI_E_NOT_INITIALIZED");

				if(hr & E_OUTOFMEMORY)
					DoMessage(L"E_OUTOFMEMORY - Insufficient memory exists to perform the operation");

				if(hr & S_OK)
					DoMessage(L"Success!");
*/
                Log("Listen failed on an address '%s'", GetTapiAddressName(pAddress));
            }
        }

        pAddress->Release();

    }

    pEnumAddress->Release();
    
    return S_OK;
}


///////////////////////////////////////////////////////////////////
// ListenOnThisAddress
//
// We call RegisterCallNotifications to inform TAPI that we want
// notifications of calls on this address. We already resistered
// our notification interface with TAPI, so now we are just telling
// TAPI that we want calls from this address to trigger events on
// our existing notification interface.
//    
///////////////////////////////////////////////////////////////////

HRESULT
CMainFrame::ListenOnThisAddress(
                    ITAddress * pAddress
                   )
{
    
    //
    // RegisterCallNotifications takes a media type bitmap indicating
    // the set of media types we are interested in. We know the
    // address supports audio, but let's add in video as well
    // if the address supports it.
    //

	//TAPIMEDIATYPE_AUDIO
	//TAPIMEDIATYPE_VIDEO
	//TAPIMEDIATYPE_DATAMODEM
	
	long lMediaTypes = TAPIMEDIATYPE_AUDIO;

/*	if( AddressSupportsMediaType(pAddress, TAPIMEDIATYPE_AUDIO) )
		lMediaTypes |= TAPIMEDIATYPE_AUDIO;
	else
		DoMessage(L"Does not support audio");


    if ( AddressSupportsMediaType(pAddress, TAPIMEDIATYPE_VIDEO) )
        lMediaTypes |= TAPIMEDIATYPE_VIDEO;
	else
		DoMessage(L"Does not support video");
*/

    HRESULT  hr;
    long     lRegister;

    hr = gpTapi->RegisterCallNotifications(
                                           pAddress,
                                           VARIANT_TRUE,
                                           VARIANT_TRUE,
                                           lMediaTypes,
                                           gulAdvise,
                                           &lRegister
                                          );

    return hr;
}

//////////////////////////////////////////////////////////////
// AddressSupportsMediaType
//
// Finds out if the given address supports the given media
// type, and returns TRUE if it does.
//////////////////////////////////////////////////////////////

BOOL
CMainFrame::AddressSupportsMediaType(
                         ITAddress * pAddress,
                         long        lMediaType
                        )
{
    VARIANT_BOOL     bSupport = VARIANT_FALSE;
    ITMediaSupport * pMediaSupport;
    
    if ( SUCCEEDED( pAddress->QueryInterface( IID_ITMediaSupport,
                                              (void **)&pMediaSupport ) ) )
    {
        //
        // does it support this media type?
        //

        pMediaSupport->QueryMediaType(
                                      lMediaType,
                                      &bSupport
                                     );
    
        pMediaSupport->Release();
    }

    return (bSupport == VARIANT_TRUE);
}


/////////////////////////////////////////
//  End TAPI and Caller ID Functions   //
/////////////////////////////////////////

//TES 11/7/2007 - PLID 27979 - VS2008 - This is already defined in VS 2008
#if _MSC_VER <= 1300
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif


BOOL CMainFrame::OnNeedText( UINT nID, NMHDR * pNMHDR, LRESULT * pResult )
{
	//codeguru code - http://codeguru.earthweb.com/toolbar/tb_custmsg_tips.shtml
		ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);
			
			TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
			TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
			
			CString strTipText;
			if ( GetToolText( pNMHDR->idFrom, strTipText ) )
			{
		#ifndef _UNICODE
				if (pNMHDR->code == TTN_NEEDTEXTA)
					lstrcpyn(pTTTA->szText, strTipText, _countof(pTTTA->szText));
				else
					_mbstowcsz(pTTTW->szText, strTipText, _countof(pTTTW->szText));
		#else
				if (pNMHDR->code == TTN_NEEDTEXTA)
					_wcstombsz(pTTTA->szText, strTipText, _countof(pTTTA->szText));
				else
					lstrcpyn(pTTTW->szText, strTipText, _countof(pTTTW->szText));
		#endif
				return TRUE;
			}

	//end codeguru code


	return CNxMDIFrameWnd::OnToolTipText( nID, pNMHDR, pResult );
}

//helper function
//specify the ID and text you want to modify, otherwise it will let the message
//pass through and take the default tool tip
BOOL CMainFrame::GetToolText( UINT nID, CString& strTipText )
{
	switch(nID) {
	case ID_PALM_PILOT:
		if(m_bIncomingCall)
			strTipText = _T("Incoming call from:  " + m_strIncomingCallNum);
		else
			strTipText = _T("No Calls Incoming");
		return true;
		break;
	}
	return FALSE;
}

void CMainFrame::OnAutoCaller()
{
	CAutoCallerDlg dlg(this);

	dlg.DoModal();
}

void CMainFrame::InitPracYakker(){
	//Load up the system tray
	// (a.walling 2012-04-02 08:32) - PLID 46648 - Dialogs must set a parent!
	if (m_pdlgMessager == NULL) // (a.walling 2007-05-04 10:35) - PLID 4850 - Construct the dialog if it is NULL
		m_pdlgMessager = new CMessagerDlg(this);
	// (a.walling 2013-02-11 17:25) - PLID 54087 - PracYakker, Room Manager should always stay on top of the main Practice window.
	m_pdlgMessager->Create(IDD_MESSAGE_HOME, this);

	// (a.walling 2007-05-07 08:24) - PLID 4850 - If we are already connected to the NxServer, simulate a NxServer connected message
	if (g_hNxServerSocket != NULL) {
		// this calls the message handler, which will set m_bNetworkEnabled to true and then call LogOnTCP
		// however, since m_bEnabled is already set to false, that will have no effect. The actual logon
		// will occur when OnEnablePracYakker is called below. This line could be replaced by m_bNetworkEnabled = true,
		// but I'll send the message just in case our logon process changes.
		m_pdlgMessager->SendMessage(NXM_NXSERVER_CONNECTED, NULL, NULL);
	}

	// (v.maida 2014-12-27 10:19) - PLID 27381 - Don't enable PracYakker unless the user has permissions for it. If they have password permissions, then they'll be prompted to enter a
	// password when they choose a PracYakker-related menu item.
	// Yakker-related dialogs are still created / initialized within this function, and the messager dialog still sets its network enabled flag, so as to avoid assertion errors later on.
	if (CheckCurrentUserPermissions(bioPracYakker, sptView, 0, 0, TRUE)) {
			if (GetRemotePropertyInt("YakkerEnabled", 1, -1, GetCurrentUserName(), true) == 1){//Set up PracYakker
				m_pdlgMessager->m_bEnabled = false;  //I know, I know, but if it's already enabled, OnEnable won't do anything
				m_pdlgMessager->OnEnablePracYakker();
			}
			else{
				m_pdlgMessager->m_bEnabled = true; //See above.
				m_pdlgMessager->OnDisableYakker();
			}
	}
	else{
		m_pdlgMessager->m_bEnabled = true; //See above.
		m_pdlgMessager->OnDisableYakker();
	}

	// (c.haag 2006-05-16 13:25) - PLID 20621 - We now have a
	// persistent message sending window
	// (a.walling 2012-04-02 08:32) - PLID 46648 - Dialogs must set a parent!
	// (a.walling 2012-10-05 09:36) - PLID 53027 - Fix mainframe activation (pass NULL as parent)
	// (a.walling 2013-02-11 17:25) - PLID 54087 - PracYakker, Room Manager should always stay on top of the main Practice window.
	if (m_pdlgSendYakMessage == NULL) // (a.walling 2007-05-04 10:35) - PLID 4850 - Construct the dialog if it is NULL
		m_pdlgSendYakMessage = new CSendMessageDlg(this);
	m_pdlgSendYakMessage->Create(IDD_SEND_MESSAGE_DLG, this);
	m_pdlgSendYakMessage->m_pMessageHome = m_pdlgMessager;
}

BOOL CMainFrame::InitNxServer()
{
	BOOL bException = FALSE;
	try {
		if (NULL == g_hNxServerSocket) {
			CWaitCursor wc;
			g_hNxServerSocket = NxSocketUtils::Connect(GetSafeHwnd(), NxRegUtils::ReadString(GetRegistryBase() + "NxServerIP"));
			g_propManager.SetNxServerSocket(((CNxSocket*)g_hNxServerSocket)->GetSocket());

			// (c.haag 2004-01-06 09:35) - Make sure NxServer associates our connection
			// with our database so table checker messages get filtered to the right places.
			CString strDefaultDatabase = (LPCTSTR)GetRemoteData()->GetDefaultDatabase();
			char* szDefaultDatabase = new char[strDefaultDatabase.GetLength()+1];
			strcpy(szDefaultDatabase, strDefaultDatabase);
			NxSocketUtils::Send(g_hNxServerSocket, PACKET_TYPE_DATABASE_NAME, (void*)szDefaultDatabase, strDefaultDatabase.GetLength()+1);
			delete szDefaultDatabase;

			ASSERT(m_pdlgMessager);
			if (m_pdlgMessager)
				m_pdlgMessager->SendMessage(NXM_NXSERVER_CONNECTED, NULL, NULL);
		}
		return TRUE;
	}
	// (z.manning 2010-05-27 10:45) - PLID 38912 - Use this macro instead of catch(...) to avoid memory leaks with CExceptions.
	NxCatchAllCallIgnore(
		bException = TRUE;
	);

	if(bException)
	{
		// (c.haag 2003-12-11 17:07) - We are already warned earlier that we can't connect
		// to nxserver back we connected within the scope of a single function call to check
		// backup information.
		try {
			NxSocketUtils::Disconnect(g_hNxServerSocket);
		}
		catch (CNxException* e) {
			e->Delete();
		}
		catch (...)
		{
		}
		g_hNxServerSocket = NULL;
		// (c.haag 2006-10-12 12:25) - PLID 22731 - Try again in five minutes
		Log("Could not connect to NxServer. Trying again in five minutes.");
		SetTimer(IDT_NXSERVER_RECONNECT, 5*60*1000, NULL);
	}
	return FALSE;
}

void CMainFrame::OnTestCallerID()
{
//	OnReceivedCallerIdNumber("1234567890");

	m_bIncomingCall = true;

	srand((unsigned) time(NULL));

	try{

		CString strTemp;
		_RecordsetPtr rs;
		rs = CreateRecordset("SELECT PersonT.First + ', ' + PersonT.Last + ' ' + PersonT.Middle AS PatName, HomePhone FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE HomePhone <> ''");

		for(long i = 0; i < rand() % rs->GetRecordCount(); i++)
			rs->MoveNext();

		m_strIncomingCallNum = AdoFldString(rs, "HomePhone");

		strTemp = AdoFldString(rs, "PatName");
		m_strIncomingCallNum = m_strIncomingCallNum + " - " + strTemp;

		SetTimer(IDT_TEST_CALLERID, 10000, NULL);
	}NxCatchAll("Error in OnTestCallerID()");

}

void CMainFrame::OnUpdatePastAppts()
{
	CUpdatePastPendingApptsDlg dlg(this);
	dlg.DoModal();
}

void CMainFrame::OnHelpFinder()
{
	OpenManual("", "");
}

void CMainFrame::WinHelp(DWORD dwData, UINT nCmd)
{
	// TODO: I don't know if we will someday want to override this with a call to HtmlHelp
	CNxMDIFrameWnd::WinHelp(dwData, nCmd);
}

CMirrorLink *CMainFrame::GetMirrorLink()
{
	if (!m_pMirrorDlg) {
		m_pMirrorDlg = new CMirrorLink;
	}
	return m_pMirrorDlg;
}

CUnitedLink *CMainFrame::GetUnitedLink()
{
	if (!m_pUnitedDlg) {
		m_pUnitedDlg = new CUnitedLink(this);
	}
	return m_pUnitedDlg;
}


void CMainFrame::LinkToEvent(PhaseTracking::EventType nType, long nID)
{
	CNxTabView* pView;
	switch(nType) {
	case PhaseTracking::ET_BillCreated:
		//Open the Patients module
		pView = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);	
		//Now call the function.
		((CPatientView*)pView)->OpenBill(nID);
		break;
	case PhaseTracking::ET_MarkedDone:
		//Open the Patients module
		pView = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
		//Now switch to that tab (there's nothing specific to open).
		((CPatientView*)pView)->SetActiveTab(PatientsModule::ProcedureTab);
		break;
	case PhaseTracking::ET_QuoteCreated:
		//Open the Patients module
		pView = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
		//Now call the function.
		((CPatientView*)pView)->OpenQuote(nID);
		break;
	case PhaseTracking::ET_AppointmentCreated:
	case PhaseTracking::ET_ActualAppointment:
		//Open the Scheduler module
		if(FlipToModule(SCHEDULER_MODULE_NAME)) {
			pView = (CNxTabView *)GetOpenView(SCHEDULER_MODULE_NAME);
			//Now call the function.
			((CSchedulerView*)pView)->OpenAppointment(nID, GetRemotePropertyInt("ApptAutoOpenResEntry", 1, 0, GetCurrentUserName(), true) ? TRUE : FALSE);
		}
		break;
	case PhaseTracking::ET_TemplateSent:
		//Open the Patients module
		pView = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
		//Now call the function.
		((CPatientView*)pView)->OpenLetter(nID);
		break;
	case PhaseTracking::ET_PacketSent:
		//Open the Patients module
		pView = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
		//Now call the function.
		((CPatientView*)pView)->OpenPacket(nID);
		break;
	case PhaseTracking::ET_PaymentApplied:
		//Open the Patients module
		pView= (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
		//Now call the function.
		((CPatientView*)pView)->OpenPayment(nID);
		break;
	case PhaseTracking::ET_EMRCreated:
		//Open the Patients module
		pView = (CNxTabView*)GetOpenView(PATIENT_MODULE_NAME);
		//Now call the function
		((CPatientView*)pView)->OpenEMR(nID);
		break;
	}
}

void CMainFrame::ShowWhatsNew(BOOL bForceWhatsNew /*= FALSE*/)
{
	// Don't show what's new on a screen that is much too small for it
	if (IsScreen640x480())
		return;

	//m.hancock - 2/16/2006 - PLID 19312 - Present the list of changes in an HTML file
	try {
		// Make sure the object exists
		if (m_pWhatsNewDlg == NULL) {
			m_pWhatsNewDlg = new CWhatsNewHTMLDlg(this);
		}

		// Make sure it isn't already created
		if (m_pWhatsNewDlg->GetSafeHwnd()) {
			m_pWhatsNewDlg->DestroyWindow();
		}
		
		// Open the dialog
		m_pWhatsNewDlg->DoModeless(bForceWhatsNew);

	} NxCatchAll("CMainFrame::ShowWhatsNew(): Failure displaying list of changes.");
}

void CMainFrame::OnBatchPayments()
{
	// (j.jones 2010-06-23 10:33) - PLID 39297 - check this license, do not "use", because
	// actually entering the tab will "use" it
	if(!g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) {
		MsgBox("You are not licensed to access this feature.");
		return;
	}

	if (UserPermission(FinancialModuleItem)) {
		OpenModule(FINANCIAL_MODULE_NAME);
		CNxTabView* pView = (CNxTabView *)GetOpenView(FINANCIAL_MODULE_NAME);
		if (pView) 
		{	if(pView->GetActiveTab() != FinancialModule::BatchPayTab)
				pView->SetActiveTab(FinancialModule::BatchPayTab);
		}
	}
}

BOOL IsHighlightSortedColumn()
{
	// Check the datalist's registry value to see what color scheme it's using
	// (a.walling 2010-02-09 09:49) - PLID 37277 - Use a default param
	int nRegColorStyle = NxRegUtils::ReadLong("HKEY_CURRENT_USER\\Software\\NexTech\\NxDataList\\ColorScheme\\ColorStyleSortColumn", 1);
	switch (nRegColorStyle) {
	case 0:
		// Use windows colors
		return FALSE;
		break;
	case 1:
		// Use custom colors (TODO: Since there's no interface for custom colors, we assume this also means to use the windows colors)
		return FALSE;
		break;
	case 2:
		// Use a NxDatalist-defined set of colors
		return TRUE;
		break;
	default:
		// Invalid color style
		ASSERT(FALSE);
		return FALSE;
		break;
	}
}

void SetHighlightSortedColumn(BOOL bHighlight)
{
	// Set the datalist's registry value appropriately
	if (bHighlight) {
		NxRegUtils::WriteLong("HKEY_CURRENT_USER\\Software\\NexTech\\NxDataList\\ColorScheme\\ColorStyleSortColumn", 2, true);
	} else {
		NxRegUtils::WriteLong("HKEY_CURRENT_USER\\Software\\NexTech\\NxDataList\\ColorScheme\\ColorStyleSortColumn", 1, true);
	}
}

void CMainFrame::OnUpdateViewHighlightSortedColumn(CCmdUI* pCmdUI)
{
	// See if we need to look up the status
	if (m_nHighlightSortedColumn == 2) {
		// We do need to look up the status
		if (IsHighlightSortedColumn()) {
			m_nHighlightSortedColumn = 1;
		} else {
			m_nHighlightSortedColumn = 0;
		}
	}

	pCmdUI->Enable(TRUE);
	if (m_nHighlightSortedColumn == 1) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void CMainFrame::OnViewHighlightSortedColumn()
{
	if (IsHighlightSortedColumn()) {
		// It was true, so set it false
		SetHighlightSortedColumn(FALSE);
		// Store the new value
		m_nHighlightSortedColumn = 0;
	} else {
		// It was false, so set it true
		SetHighlightSortedColumn(TRUE);
		// Store the new value
		m_nHighlightSortedColumn = 1;
	}

	// Tell the user they have to re-open practice
	MsgBox(MB_ICONINFORMATION|MB_OK, "This change will take effect the next time you start Practice.");
}

void CMainFrame::ResetInactivityTimer()
{
}

// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
int CMainFrame::ShellExecuteModal(IN LPCTSTR strFile, IN LPCTSTR strParameters /*= ""*/, IN OPTIONAL LPCTSTR strWorkingPath /*= NULL*/, IN OPTIONAL LPCTSTR strOperation /*= NULL*/)
{
	// Make it so while we wait, we set the cursor to the hourglass
	m_bIsWaitingForProcess = TRUE;
	
	// Do the shell execute and wait for the app it spawns to be closed by the user
	int nAns = ShellExecuteAppModal(m_hWnd, strFile, strParameters, strWorkingPath, strOperation);
	
	// We're no longer waiting so go back to no hourglass
	m_bIsWaitingForProcess = FALSE;
	

	// Return the success value that was given to us by ShellExecuteAppModal
	return nAns;
}

BOOL CMainFrame::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (m_bIsWaitingForProcess) {
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		return TRUE;
	} else {
		return CNxMDIFrameWnd::OnSetCursor(pWnd, nHitTest, message);
	}
}

void CMainFrame::StartLogTime() {

	try {
		//TES 12/18/2008 - PLID 32520 - This feature is blocked to Scheduler Standard users
		if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "User time logs")) {
			return;
		}

		//make sure one doesn't already exist
		// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
		if(ReturnsRecordsParam("SELECT ID FROM UserTimesT WHERE UserID = {INT} AND Checkout IS NULL", GetCurrentUserID())) {
			AfxMessageBox("You are already logging your time.  Please end the current one before beginning a new log.");
			return;
		}

		//create a record for this user in UserTimesT
		long nNewNum = NewNumber("UserTimesT", "ID");
		// (j.gruber 2008-06-25 12:35) - PLID 26136 - added location
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (b.eyers 2016-01-18) - PLID 67542 - checkin time is now gotten from the server
		ExecuteParamSql("INSERT INTO UserTimesT (ID, UserID, Checkin, Checkout, locationID) values ({INT}, {INT}, GETDATE(), NULL, {INT})", nNewNum, GetCurrentUserID(), GetCurrentLocationID());

		//we've started logging, so change our flag
		m_bLoggingTime  = true;

	} NxCatchAll("Error beginning time logging.");
}

void CMainFrame::EndLogTime() {

	try {
		//TES 12/18/2008 - PLID 32520 - This feature is blocked to Scheduler Standard users
		if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "User time logs")) {
			return;
		}

		//end the record for this user
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM UserTimesT WHERE UserID = {INT} AND Checkout IS NULL", GetCurrentUserID());
		if(rs->eof) {
			AfxMessageBox("There were no open time logs found.  Please start a log before ending one.");
			return;
		}

		long nID = AdoFldLong(rs, "ID");

		//insert the end record
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (b.eyers 2016-01-18) - PLID 67542 - checkout time is now gotten from the server
		ExecuteParamSql("UPDATE UserTimesT SET Checkout = GETDATE() WHERE ID = {INT}", nID);

		//we've ended, so change our flag
		m_bLoggingTime  = false;

	} NxCatchAll("Error ending time logging.");
}

// PLID 26192	6/6/08	r.galicki	-	Added functionality for multi-user log management
void CMainFrame::OnToolsLogTimeLogOtherUser()
{
	try {
		//TES 12/18/2008 - PLID 32520 - This feature is blocked to Scheduler Standard users
		if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "User time logs")) {
			return;
		}
		
		CMultiUserLogDlg dlgMultiUserLog(this);

		dlgMultiUserLog.DoModal();

	} NxCatchAll("Error in CMainFrame::OnToolsLogTimeLogOtherUser");
}

void CMainFrame::ImportPatientFile() {
	
	try {
		
		// (j.jones 2010-10-19 09:23) - PLID 34571 - don't let them in if they don't have permission
		// to create anything from the import
		// (j.politis 2015-06-30 12:00) - PLID 65484 - The mainframe menu option for the Import feature is disabled if you dont have create permissions for Patients, Contacts, AND Service Codes.
		if (!ImportShouldBeAllowed()) {

			//warn and return
			MessageBox("You do not have permission to create or import any of the items covered by this function. "
				"The import feature cannot be used without these permissions.", "Practice", MB_ICONEXCLAMATION|MB_OK);
			return;
		}
		
		CImportWizardDlg dlgImportWizard;
		dlgImportWizard.DoModal();
	
	}NxCatchAll("Error while using the Import Wizard.");

}

//DRT - 11/20/02 - Added mostly for Charlie to better keep track of support
//Exports to "<pracpath>\ClientData.csv"
void CMainFrame::NxClientReport() {
	//TES 6/20/2005 - This hasn't been accessible for years, and the data structure's changed, so I'm commenting it out.

	/*//only run in internal mode
	if(!IsNexTechInternal())
		return;

	try {

		_RecordsetPtr rs;
		rs = CreateRecordset("SELECT NxClientsT.PersonID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, City, State, Zip, "
				"WorkPhone, Fax, Email, LicenseBought, LicenseUsed, DoctorsBought, DoctorsUsed, PalmPilotBought, "
				"PalmPilotUsed, PatientsBought, PatientsUsed, SchedulerBought, SchedulerUsed, BillingBought, "
				"BillingUsed, HCFABought, HCFAUsed, LettersBought, LettersUsed, QuotesBought, QuotesUsed, "
				"MarketingBought, MarketingUsed, InvBought, InvUsed, EBillingBought, EBillingUsed, MirrorLinkBought, "
				"MirrorLinkUsed, InformLinkBought, InformLinkUsed, UnitedLinkBought, UnitedLinkUsed, PurchAgrSign, "
				"LicAgrSign, InstalledOn, Trainer, SupportExpires, "
				"VerSent, VerCur, AllowExpiredSupport, GroupTypes.GroupName "
				"FROM NxClientsT INNER JOIN PersonT ON NxClientsT.PersonID = PersonT.ID INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"INNER JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex "
				"WHERE PatientsT.CurrentStatus = 1 AND PersonT.Archived = 0");

		FieldsPtr pflds = rs->Fields;
		CString strOut;

		//
		//Print a header on the top for ease of reading
		strOut += "First,Middle,Last,Address1,Address2,City,State,Zip,WorkPhone,Fax,Email,LicenseBought,LicenseUsed,DocBought,DocUsed,"
				"PalmBought,PalmUsed,PatBought,PatUsed,SchedBought,SchedUsed,BillingBought,BillingUsed,HCFABought,HCFAUsed,LWBought,LWUsed,"
				"QuotesBought,QuotesUsed,MarketingBought,MarketingUsed,InvBought,InvUsed,EBillBought,EBillUsed,MirrorBought,MirrorUsed,"
				"InformBought,InformUsed,UnitedBought,UnitedUsed,AllowSupport,PurchAgrment,LicAgrment,InstalledOn,Trainer,SupportExpires,"
				"VerSent,VerCur,Status\r\n";
		//Now loop through all the records and add the client info to be written
		//

		//open up the file for writing
		CString strFile;
		strFile = GetPracPath(true);
		strFile.TrimRight("\\");
		strFile += "\\ClientData.csv";

		CFile fOut(strFile, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone);

		//write the header
		fOut.Write(strOut, strlen(strOut));

		while(!rs->eof) {

			//reset our output string
			strOut = "";

			//create a string with all our fields seperated by commas
			strOut += "\"" + AdoFldString(pflds, "First", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "Middle", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "Last", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "Address1", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "Address2", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "City", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "State", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "Zip", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "WorkPhone", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "Fax", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "Email", "") + "\"";
			strOut += ",";

			
			strOut += "\"" + AdoFldString(pflds, "LicenseBought", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "LicenseUsed", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "DoctorsBought", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "DoctorsUsed", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "PalmPilotBought", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "PalmPilotUsed", "") + "\"";
			strOut += ",";

			strOut += "\"" + (CString)(AdoFldBool(pflds, "PatientsBought", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "PatientsUsed", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "SchedulerBought", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "SchedulerUsed", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "BillingBought", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "BillingUsed", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "HCFABought", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "HCFAUsed", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "LettersBought", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "LettersUsed", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "QuotesBought", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "QuotesUsed", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "MarketingBought", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "MarketingUsed", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "InvBought", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "InvUsed", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "EBillingBought", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "EBillingUsed", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "MirrorLinkBought", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "MirrorLinkUsed", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "InformLinkBought", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "InformLinkUsed", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "UnitedLinkBought", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "UnitedLinkUsed", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";
			strOut += "\"" + (CString)(AdoFldBool(pflds, "AllowExpiredSupport", false) != FALSE ? "1" : "0") + "\"";
			strOut += ",";

			//dates can be null, rather have them blank than a default date
			if(pflds->Item["PurchAgrSign"]->Value.vt == VT_DATE)
				strOut += "\"" + AdoFldDateTime(pflds, "PurchAgrSign").Format("%m/%d/%Y") + "\"";
			else
				strOut += "\"\"";
			strOut += ",";

			if(pflds->Item["LicAgrSign"]->Value.vt == VT_DATE)
				strOut += "\"" + AdoFldDateTime(pflds, "LicAgrSign").Format("%m/%d/%Y") + "\"";
			else
				strOut += "\"\"";
			strOut += ",";
	
			if(pflds->Item["InstalledOn"]->Value.vt == VT_DATE)
				strOut += "\"" + AdoFldDateTime(pflds, "InstalledOn").Format("%m/%d/%Y") + "\"";
			else
				strOut += "\"\"";
			strOut += ",";

			strOut += "\"" + AdoFldString(pflds, "Trainer", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "SupportExpires", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "VerSent", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "VerCur", "") + "\"";
			strOut += ",";
			strOut += "\"" + AdoFldString(pflds, "GroupName", "") + "\"";

			//add a blank line
			strOut += "\r\n";

			//strOut is setup, write it to the file
			fOut.Write(strOut, strlen(strOut));

			rs->MoveNext();
		}

		AfxMessageBox("File created successfully at " + strFile);

	} NxCatchAll("Error in Client Report");*/

}

void CMainFrame::PrintHistoryReport(BOOL bPreview, CPrintInfo *pInfo) {

	CHistoryFilterDlg dlg(this);
	if(dlg.DoModal() == IDCANCEL)
		return;

	CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(231)]);
	infReport.nPatient = GetActivePatientID();

	//get the filters if needed
	if(dlg.UseDates()) {
		infReport.nDateRange = 1;	//enable date filter
		infReport.nDateFilter = 1;	//use TDate
		infReport.DateFrom = dlg.GetDateFrom();
		infReport.DateTo = dlg.GetDateTo();
	}
	else {
		infReport.nDateRange = -1;
	}

	//add a parameter called "ShowLocation" to the report
	CRParameterInfo* paramInfo;
	CPtrArray paParams;
	CString tmp;
	paramInfo = new CRParameterInfo;
	tmp.Format ("%s", dlg.GetLocationName());
	paramInfo->m_Data = tmp;
	paramInfo->m_Name = "ShowLocation";
	paParams.Add(paramInfo);

	//Made new function for running reports - JMM 5-28-04
	// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
	RunReport(&infReport, &paParams, true, (CWnd *)this, "Patient Financial History", pInfo, TRUE);
	ClearRPIParameterList(&paParams);	//DRT - PLID 18085 - Cleanup after ourselves
}

CString PrepareChaseHealthAdvanceString(CString str, BOOL IsPhoneNum = FALSE) {

	//DRT 10/13/2008 - We no longer need this since we are doing an HTTP POST, we do not want to format
	//	the text.  We do still want the phone number formatting.
	/*
	for(int i=0;i<str.GetLength();i++) {
		TCHAR ch = str.GetAt(i);
		if((ch >= '0' && ch <= '9') || ((ch >= 'A' && ch <= 'z') && !IsPhoneNum) || (ch == '-' && !IsPhoneNum))
			strReturn += ch;
		else if(ch == ' ' && !IsPhoneNum)
			strReturn += "%20";
	}
	*/

	if(IsPhoneNum) {
		CString strReturn;
		for(int i=0;i<str.GetLength();i++) {
			TCHAR ch = str.GetAt(i);
			if((ch >= '0' && ch <= '9') || ((ch >= 'A' && ch <= 'z') && !IsPhoneNum) || (ch == '-' && !IsPhoneNum))
				strReturn += ch;
		}

		//ensure it is formatted properly - ###-###-####
		if(strReturn.GetLength() > 3)
			strReturn.Insert(3,"-");
		if(strReturn.GetLength() > 7)
			strReturn.Insert(7,"-");

		return strReturn;
	}
	else 
		return str;
}

//DRT 10/13/2008 - I combined PrepareUnicornString into this function, to make the main function easier to read.
CString FormatChaseHealthAdvanceHiddenElement(CString strType, CString strData, BOOL bIsPhoneNum = FALSE)
{
	CString strOutput;
	strOutput.Format("<input type=hidden name=\"%s\" value=\"%s\">\r\n", strType, PrepareChaseHealthAdvanceString(strData, bIsPhoneNum));

	return strOutput;
}

// (d.thompson 2009-08-11) - PLID 35166 - I renamed all occurrences of "unicorn link" to "ChaseHealthAdvance", since the
//	product has been re-branded on their end.
void CMainFrame::OnSubmitToChaseHealthAdvance() 
{
	if(!CheckCurrentUserPermissions(bioChaseHealthAdvance, sptView))
		return;

	if(IDNO == MessageBox("Do you wish to submit a credit application to ChaseHealthAdvance?\n"
		"(You will get a chance to edit the information before finalizing the submission.)","Practice",MB_ICONQUESTION|MB_YESNO))
		return;

	try {

		_RecordsetPtr prs = CreateParamRecordset("SELECT * "
			"FROM PersonT "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"WHERE PatientsT.PersonID = {INT}", GetActivePatientID());

		if(!prs->eof) {
			CString strSubmit;

			_variant_t var;

			//we submit a string with the following web address and 21 identifiers.
			//even if the data is blank, the identifier must still be sent

			//all text fields may have only numbers, letters, and hyphens
			//spaces are replaced with %20
			//numeric fields are a certain format (I control that in this function)

			//test website
			//strSubmit = "http://asp.personalfinancing.net/services/nextech/";

			//real website
			//DRT 8/29/2008 - PLID 30863 - Changed destination website and some of the field data slightly.  Unicorn
			//	was bought out by Chase Financial Services.
			//strSubmit = "https://asp.unicornfinancial.com/services/embeddedapp/";
			strSubmit = "https://www.healthadvance-online.com/services/embeddedapp/default.asp";

			//DRT 10/13/2008 - PLID 31674 - Instead of forming a URL which contains all the data, we have been asked to 
			//	instead generate a local HTML file with a javascript auto-submit on it to send the data.

			CString strHTML;
			strHTML.Format("<html><form id=\"form1\" method=POST action=\"%s\">\r\n", strSubmit);

			//Now each line will get added looking like:
			//	<input type=hidden name=”FN” value=”First Name”>

			//First Name
			strHTML += FormatChaseHealthAdvanceHiddenElement("FN", AdoFldString(prs, "First",""));

			//Middle Initial
			strHTML += FormatChaseHealthAdvanceHiddenElement("MI", AdoFldString(prs, "Middle",""));
			
			//Last Name
			strHTML += FormatChaseHealthAdvanceHiddenElement("LN", AdoFldString(prs, "Last",""));

			//Suffix
			//we don't have this
			strHTML += FormatChaseHealthAdvanceHiddenElement("SF", "");

			//Address
			strHTML += FormatChaseHealthAdvanceHiddenElement("AD", AdoFldString(prs, "Address1",""));

			//Apt#/Suite
			strHTML += FormatChaseHealthAdvanceHiddenElement("AP", AdoFldString(prs, "Address2",""));

			//City
			strHTML += FormatChaseHealthAdvanceHiddenElement("CT", AdoFldString(prs, "City",""));

			//State
			strHTML += FormatChaseHealthAdvanceHiddenElement("ST", AdoFldString(prs, "State",""));

			//Zipcode
			//DRT 9/2/2008 - PLID 30863 - Zip4 was removed from the spec
			CString strZip = AdoFldString(prs, "Zip","");
			CString strZip5 = "";
			if(strZip.GetLength() < 5) {
				strZip5 = strZip;
			}
			else {
				strZip5 = strZip.Left(5);	
			}

			//Zip
			strHTML += FormatChaseHealthAdvanceHiddenElement("Z5", strZip5);

			//SSN
			CString strSSN = PrepareChaseHealthAdvanceString(AdoFldString(prs, "SocialSecurity",""));
			//SSN has no dashes
			strSSN.Replace("-","");
			strHTML += FormatChaseHealthAdvanceHiddenElement("SN", strSSN);

			//Date of Birth
			//birthdate is MM-DD-YYYY
			CString strDOB;
			var = prs->Fields->Item["BirthDate"]->Value;
			if(var.vt == VT_DATE) {
				COleDateTime dt =  var.date;
				strDOB = dt.Format("%m-%d-%Y");
			}
			strHTML += FormatChaseHealthAdvanceHiddenElement("DB", strDOB);

			//Home Phone
			strHTML += FormatChaseHealthAdvanceHiddenElement("HP", AdoFldString(prs, "HomePhone",""), TRUE);

			//Other Phone
			strHTML += FormatChaseHealthAdvanceHiddenElement("OP", AdoFldString(prs, "OtherPhone",""), TRUE);

			//Total Gross Monthy Income
			strHTML += FormatChaseHealthAdvanceHiddenElement("GM", "");
			//we don't have this

			//Residential Status
			strHTML += FormatChaseHealthAdvanceHiddenElement("HO", "");
			//we don't have this

			//Income Source
			strHTML += FormatChaseHealthAdvanceHiddenElement("IS", "");
			//we don't have this

			//Employer
			strHTML += FormatChaseHealthAdvanceHiddenElement("EM", AdoFldString(prs, "Company",""));

			//Work Phone
			strHTML += FormatChaseHealthAdvanceHiddenElement("WP", AdoFldString(prs, "WorkPhone",""), TRUE);

			//Personal Reference
			strHTML += FormatChaseHealthAdvanceHiddenElement("PR", "");
			//we don't have this

			//Reference Phone
			strHTML += FormatChaseHealthAdvanceHiddenElement("RP", "");
			//we don't have this

			//Partner Code
			strHTML += FormatChaseHealthAdvanceHiddenElement("PC", "NX");
			//static NX identifying us as the intermediary


			//Wrap up our HTML
			strHTML += 
				"</form> "
				"<script type=\"text/javascript\"> "
				"function loadform () { "
				"var frm = document.getElementById(\"form1\"); "
				"frm.submit(); "
				"} "
				"window.onload = loadform; "
				"</script>"
				"</html>";

			//
			//Now write the HTML to a temporary file.
			CString strFilename;
			HANDLE hFile = FileUtils::CreateTempFile("", "html", &strFilename, TRUE);
			CFile fOut(hFile);
			fOut.Write(strHTML, strHTML.GetLength());
			fOut.Close();

			//done, let's open the file.  The javascript should automatically take over and submit to the proper
			//	URL then.
			// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
			if ((int)ShellExecute(GetSafeHwnd(), NULL, strFilename, NULL, "", SW_MAXIMIZE) < 32)
				AfxMessageBox("Could not submit data to ChaseHealthAdvance.");
		}
	}NxCatchAll("Error submitting to ChaseHealthAdvance Financial.");
}

// (a.walling 2010-10-11 17:08) - PLID 40731 - Handle an ID and alert type (only for NT_DEFAULT)
void CMainFrame::NotifyUser(int nNotificationType, const CString &strMessage, bool bImmediatePopup /*= true*/, long nAssociatedID, EAlert alertType) 
{
	//First off, let's initialize the alert dialog if necessary.
	if(nNotificationType == NT_DEFAULT) {
		if (!m_dlgAlert.m_hWnd) {
			m_dlgAlert.Create(IDD_ALERT_DLG, CWnd::GetDesktopWindow()); // (a.walling 2012-07-10 13:43) - PLID 46648 - Transient alert window should not have a parent to steal focus
		}
		// (a.walling 2010-10-11 17:11) - PLID 40731 - Pass in the associated ID and alert type
		m_dlgAlert.SetAlertMessage(strMessage, nAssociatedID, alertType);
	}

	if(!bImmediatePopup) {
		m_nWaitingType = WT_WAIT_ALWAYS;
	}
	else if(IsPopupAllowed()) {
		//Well, we don't know of any reason not to pop up.  Let's ask the active sheet.
		CNxTabView *pView = GetActiveView();
		CNxDialog* pSheet = NULL;
		if(pView && pView->IsKindOf(RUNTIME_CLASS(CNxTabView))) pSheet = pView->GetActiveSheet();
		if(pSheet && pSheet->IsKindOf(RUNTIME_CLASS(CNxDialog))) {
			m_nWaitingType = pSheet->AllowPopup();
			if(m_nWaitingType == WT_DONT_WAIT) {
				//OK, now, do they want us to ever pop anything up?
				if(GetRemotePropertyInt("AllowAlertPopup", 2, 0, GetCurrentUserName(), true)) {
					//Yes, so, let's pop it up.
					switch(nNotificationType) {
					case NT_TODO:
						ShowTodoList();
						return;
						break;
					case NT_YAK:
						ASSERT(m_pdlgMessager);
						if (m_pdlgMessager)
							m_pdlgMessager->PopupYakker();
						return;
						break;
					//TES 11/1/2013 - PLID 59276 - Added CDS notifications
					case NT_CDS_INTERVENTION:
						OpenInterventionsList();
						return;
						break;
					// (j.jones 2008-04-22 12:35) - PLID 29597 - added HL7 notifications,
					// but in this case we always fire the notification window
					// (and it should always be called with bImmediatePopup set to true,
					// so technically we shouldn't get to this code)
					// (j.jones 2011-03-15 15:14) - PLID 42738 - same for the device import
					case NT_HL7:
					case NT_DEVICE_IMPORT:
						break;
					
					default:
						ShowAlertDialog();
						return; 
						break;
					}

				}
				else {
					m_nWaitingType = WT_WAIT_ALWAYS;
				}
			}
		}
		else {
			//Well, we couldn't get a valid sheet, so who knows what's going on, let's just pop up.
			if(GetRemotePropertyInt("AllowAlertPopup", 2, 0, GetCurrentUserName(), true)) {
				//Yes, so, let's pop it up.
				switch(nNotificationType) {
				case NT_TODO:
					ShowTodoList();
					return;
					break;
				case NT_YAK:
					ASSERT(m_pdlgMessager);
					if (m_pdlgMessager)
						m_pdlgMessager->PopupYakker();
					return;
					break;
				//TES 11/1/2013 - PLID 59276 - Added CDS notifications
				case NT_CDS_INTERVENTION:
					OpenInterventionsList();
					return;
					break;
				// (j.jones 2008-04-22 12:35) - PLID 29597 - added HL7 notifications,
				// but in this case we always fire the notification window
				// (and it should always be called with bImmediatePopup set to true,
				// so technically we shouldn't get to this code)
				case NT_HL7:
					break;
				default:
					ShowAlertDialog();
					return;
					break;
				}
				return;
			}
			else {
				m_nWaitingType = WT_WAIT_ALWAYS;
			}
		}
	}
	else {
		m_nWaitingType = WT_WAIT_FOR_SELF;
	}
	//If we got here, then we did not popup the todo alarm.
	//First of all, set up the notification.
	// (a.vengrofski 2010-08-18 10:26) - PLID <38919> - Added an overload for HL7 messages
	if (nNotificationType == NT_HL7 || nNotificationType == NT_HL7_LAB)
	{
		AddToNotification(nNotificationType, strMessage);
	} else {
		// (j.jones 2016-02-03 16:50) - PLID 68118 - split this message into two types
		if(nNotificationType == NT_SS_RENEWALS_NEEDING_ATTENTION || nNotificationType == NT_RX_NEEDING_ATTENTION)
		{
			m_strSureScriptsNotificationMessage = strMessage;
		}
		AddToNotification(nNotificationType); //This will update the icon or whatever.  Also, it will ensure that, when the time comes, we will know what we need to popup.
	}

	//Now, regardless of the notification type, we want to set up any needed timers.
	switch(m_nWaitingType) {
	case WT_WAIT_FOR_SELF:
		//Do nothing, this will be handled by the AllowPopup() function.
		break;
	case WT_WAIT_FOR_SHEET_PASSIVE:
		//Do nothing, this will be handled by a message from the sheet.
		
		/*
		TES 5-8-03: If you hit this ASSERTion, it is because you are the first person to implement 
		WT_WAIT_FOR_SHEET_PASSIVE.  Therefore, you have an extra responsibility to test this code (in particular,
		the handling of NXM_ALLOW_POPUP), which should work fine, but has never really  been tested.  Then you can
		remove this assertion.
		*/
		ASSERT(FALSE);

		break;
	case WT_WAIT_FOR_SHEET_ACTIVE:
		//Start a timer.
		SetTimer(IDT_WAIT_FOR_SHEET_TIMER, 500, NULL);
		break;
	case WT_WAIT_ALWAYS:
		//Do nothing, this will be handled when they click on that thing.
		break;
	default:
		//This should never happen (if it's WT_DONT_WAIT, it should have popped up and returned by now).
		ASSERT(FALSE);
		break;
	}
}

void CMainFrame::UnNotifyUser(int nNotificationType)
{
	if(m_pNotificationDlg && m_pNotificationDlg->GetSafeHwnd()) {
		m_pNotificationDlg->ClearNotifications(nNotificationType);
	}
}

//TES 6/9/2008 - PLID 23243 - Added more parameters to determine whether this should actually be displayed.
void CMainFrame::AddAlert(const char* szAlert, const CDWordArray &dwaResourceIDs, BOOL bCheckData, long nAppointmentID, EAlert alert, DWORD ip)
{
	if (alert == PatientIsIn)
	{
		//TES 6/9/2008 - PLID 23243 - If they don't want to be modified at all, we're done.
		// (a.wilson 2012-06-14 11:36) - PLID 47966
		if (!GetRemotePropertyInt("NotifyMeWhenPatientMarkedIn", GetPropertyInt("NotifyMeWhenPatientMarkedIn", 0, 0, false), 0, GetCurrentUserName())) {
			return;
		}
		else {
			//TES 6/9/2008 - PLID 23243 - Do they care which resource this is for?
			CArray<int,int> arRequestedResources;
			GetRemotePropertyArray("MarkedInNotificationResources", arRequestedResources, 0, GetCurrentUserName());
			if(arRequestedResources.GetSize()) {
				//TES 6/9/2008 - PLID 23243 - They only want to be notified on certain resources, see if we have any matches.
				if(bCheckData) {
					//TES 6/9/2008 - PLID 23243 - We need to pull from data anyway, so do it in one query.
					// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
					if(!ReturnsRecordsParam(FormatString("SELECT ID FROM ResourceT WHERE ID IN ({INTSTRING}) AND ID IN "
						"(SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT})", 
						ArrayAsString(arRequestedResources, false)), nAppointmentID)) {
						//TES 6/9/2008 - PLID 23243 - This isn't for any resource they're interested in.
						return;
					}
				}
				else {
					//TES 6/9/2008 - PLID 23243 - We've got everything we need, just compare each array.
					bool bFound = false;
					for(int i = 0; i < dwaResourceIDs.GetSize() && !bFound; i++) {
						for(int j = 0; j < arRequestedResources.GetSize() && !bFound; j++) {
							if(dwaResourceIDs[i] == (DWORD)arRequestedResources[j]) bFound = true;
						}
					}
					if(!bFound) {
						//TES 6/9/2008 - PLID 23243 - This isn't for any resource they're interested in.
						return;
					}
				}
			}
		}

		// (c.haag 2003-08-08 12:19) - You should not get your own alert
		// (c.haag 2004-03-31 10:54) - Past c.haag didn't consider the fact
		// this logic fails if you are in a Terminal services environment. Future
		// c.haag will fix it when we have the new network engine fully integrated
		// in the exe because there is no ad-hoc way to correctly fix this.
		//if (ip == GetHostIP().S_un.S_addr)
		//	return;
	}
	// (a.walling 2010-10-11 17:08) - PLID 40731 - Also pass the appointment ID and alert type
	NotifyUser(NT_DEFAULT, szAlert, true, nAppointmentID, alert);
}

void CMainFrame::AddToNotification(int nNotificationType)
{
	Notification stNotification;
	stNotification.nNotificationType = nNotificationType;
	switch(nNotificationType) {
	case NT_TODO:
		try {
			// (c.haag 2008-06-10 09:23) - PLID 11599 - Use TodoAssignToT
			_RecordsetPtr rsTodoCount = CreateParamRecordset("SELECT COUNT(ToDoList.TaskID) AS TaskCount FROM ToDoList "
				"	INNER JOIN TodoAssignToT ON TodoAssignToT.TaskID = ToDoList.TaskID "
				"	LEFT JOIN PersonT ON ToDoList.PersonID = PersonT.ID "
				"	LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"   WHERE (PatientsT.PersonID IS NULL OR PatientsT.CurrentStatus <> 4) "
				"	AND (((ToDoList.Done Is Null) AND (ToDoList.Remind <= GetDate()) AND ((TodoAssignToT.AssignTo)={INT})))", GetCurrentUserID());
			if(!rsTodoCount->eof) {
				stNotification.strMessage.Format("You have %li ToDo Alarm(s)", AdoFldLong(rsTodoCount, "TaskCount"));
			}
			else {
				ASSERT(FALSE);
				stNotification.strMessage = "";
			}
			stNotification.nCount = 1;
			stNotification.bIncrement = false;
		}NxCatchAll("Error in CMainFrame::AddToNotification():NT_TODO");
		break;
	case NT_YAK:
		try {
			_RecordsetPtr rsYakCount = CreateRecordset("SELECT Count(MessageID) AS YakCount FROM MessagesT WHERE Viewed = 0 AND RecipientID = %li AND DeletedByRecipient = 0", GetCurrentUserID());
			if(!rsYakCount->eof) {
				stNotification.strMessage.Format("You have %li Yak(s)", AdoFldLong(rsYakCount, "YakCount"));
			}
			else {
				ASSERT(FALSE);
				stNotification.strMessage = "";
			}
			stNotification.nCount = 1;
			stNotification.bIncrement = false;
		}NxCatchAll("Error in CMainFrame::AddToNotification():NT_YAK");
		break;
	//TES 4/10/2009 - PLID 33889 - Added an option to notify the user about SureScripts messages
	// (j.jones 2016-02-03 16:50) - PLID 68118 - split this message into two types, only one checks the permission
	case NT_SS_RENEWALS_NEEDING_ATTENTION:
	case NT_RX_NEEDING_ATTENTION:
	
		// (e.lally 2009-06-10) PLID 34529 - Get permissions - Read patient medications
		// (e.lally 2009-07-10) PLID 34039 - Changed to get sureScripts messages read permission
		if(nNotificationType == NT_SS_RENEWALS_NEEDING_ATTENTION
			&& !(GetCurrentUserPermissions(bioSureScriptsMessages) & (sptRead|sptReadWithPass))){
			//Don't even show the user this notification
			return;
		}

		stNotification.strMessage = m_strSureScriptsNotificationMessage;
		stNotification.nCount = 1;
		stNotification.bIncrement = false;
		break;

		// (a.vengrofski 2010-08-18 13:27) - PLID <38919> - Moved the NT_HL7 to the overload AddToNotification function.
	// (j.jones 2011-03-15 12:24) - PLID 42738 - supported the device importer
	case NT_DEVICE_IMPORT: {
		//there are no permissions for the device importer

		//this is the count of patient elements, not the count of their child records,
		//which, despite the naming suggesting otherwise, is what we really want
		long nRecordCount = DeviceImport::GetMonitor().GetDeviceImportRecordCount();
		if(nRecordCount <= 0) {
			//there are no files to import - they've already been imported
			return;
		}

		if(nRecordCount == 1) {
			stNotification.strMessage = "A file is ready to import from your devices.";
		}
		else {
			stNotification.strMessage.Format("There are %li files ready to import from your devices.", nRecordCount);
		}
		stNotification.nCount = 1;
		stNotification.bIncrement = false;

		break;
	}		
	//TES 11/1/2013 - PLID 59276 - Added CDS notifications
	case NT_CDS_INTERVENTION:
	{
		stNotification.strMessage = "New CDS Intervention";
		stNotification.nCount = 1;
		stNotification.bIncrement = false;
		break;
	}

	case NT_DEFAULT:
	default:
		stNotification.strMessage = "You have #NOTIFICATION_COUNT# alert(s).";
		stNotification.nCount = 1;
		stNotification.bIncrement = true;
		break;
	}
	EnsureNotificationDlg();
	m_pNotificationDlg->AddNotification(stNotification);
}

void CMainFrame::AddToNotification(int nNotificationType, const CString &strMessage)
{
	Notification stNotification;
	try{
		switch (nNotificationType)
	{
	case NT_HL7:
	case NT_HL7_LAB:
	// (a.vengrofski 2010-08-18 10:15) - PLID <38919> - added HL7_LAB
	// (j.jones 2008-04-22 12:35) - PLID 29597 - added HL7 notifications
		// (e.lally 2009-06-10) PLID 34529 - Get permissions - Read HL7 tab
		if(!(GetCurrentUserPermissions(bioHL7BatchDlg) & (sptRead|sptReadWithPass))){
			//Don't even show the user this notification
			return;
		}
		
		if (nNotificationType == (NT_HL7 & NT_HL7_LAB))//Priority given to labs
		{
			stNotification.nNotificationType = NT_HL7_LAB;
		} else {
			stNotification.nNotificationType = nNotificationType;
		}

		stNotification.strMessage = strMessage;
		stNotification.nCount = 1;
		stNotification.bIncrement = false;
		break;
	default:
		// (a.vengrofski 2010-08-18 09:42) - PLID <38919> - If you have hit this assert then you need to add a case above.
		ASSERT(FALSE);
	}
	EnsureNotificationDlg();
	m_pNotificationDlg->AddNotification(stNotification);
	}NxCatchAll("Error in CMainFrame::AddToNotification():NT_HL7");
}

// (j.jones 2011-03-15 15:33) - PLID 42738 - returns TRUE if the requested
// notification type exists in our notification dialog already
BOOL CMainFrame::IsNotificationTypeInQueue(int nNotificationType)
{
	try {

		if(m_pNotificationDlg && m_pNotificationDlg->GetSafeHwnd()) {
			return m_pNotificationDlg->HasNotification(nNotificationType);
		}

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

LRESULT CMainFrame::OnNotificationClicked(WPARAM wParam, LPARAM lParam)
{
	try
	{
		// (b.savon 2011-09-23 12:26) - PLID 44954 - Prevent the user from opening the plugin import dlg
		//											 until all the plugin files have been processed.
		if( !DeviceImport::GetMonitor().IsPluginProcessComplete() ){
			MessageBox("Please wait until all the plugin files are finished processing.", "Device Plugin Files Processing...", MB_ICONWARNING|MB_OK);
			return 0;
		}

		// (a.walling 2012-10-05 09:36) - PLID 53027 - Fix mainframe activation - individually decide whether to bring mainframe to top

		WPARAM dwTypes = wParam;
		//TES 1/4/2007 - PLID 24119 - I don't know how we could get here without m_pNotificationDlg being 
		// valid, but it couldn't hurt to check.


		// (j.gruber 2010-01-11 13:48) - PLID 36140 - check before we clear
		BOOL bClearNotifications = TRUE;
		if ((dwTypes & NT_NEW_PATIENT) && m_bNewPatientOpen) {
			bClearNotifications = FALSE;
		}

		if (bClearNotifications) {
			EnsureNotificationDlg();	
			m_pNotificationDlg->ClearNotifications();
		}

		m_nWaitingType = WT_DONT_WAIT;

		if(dwTypes & NT_TODO) {
			ShowTodoList();
		}
		if(dwTypes & NT_YAK) {
			ASSERT(m_pdlgMessager);
			if (m_pdlgMessager)
				m_pdlgMessager->PopupYakker();
		}
		// (j.jones 2008-04-22 12:35) - PLID 29597 - added HL7 notifications
		if(dwTypes & NT_HL7) {
			//switch to the HL7 tab

			//it should be impossible to have the notification without importing an HL7 file,
			//and it should be impossible to import an HL7 file without the HL7 license,
			//but check the license silently anyways
			if(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent)) {

				BringWindowToTop();

				// (d.thompson 2009-11-16) - PLID 36301 - Moved to 'Links' module instead of 'Financial'
				if(FlipToModule(LINKS_MODULE_NAME)) {

					CNxTabView *pView = (CNxTabView *)GetOpenView(LINKS_MODULE_NAME);
					if (pView) 
					{	
						if(pView->GetActiveTab() == LinksModule::HL7Tab) {
							pView->UpdateView();
						}
						else {
							pView->SetActiveTab(LinksModule::HL7Tab);
						}
					}
				}
			}
		}
		// (a.vengrofski 2010-08-18 11:36) - PLID <38919> - Added Lab Result notifications
		if(dwTypes & NT_HL7_LAB) {
			if(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent)) {

				BringWindowToTop();

				// (d.thompson 2009-11-16) - PLID 36301 - Moved to 'Links' module instead of 'Financial'
				if(FlipToModule(LINKS_MODULE_NAME)) {

					CNxTabView *pView = (CNxTabView *)GetOpenView(LINKS_MODULE_NAME);
					if (pView) 
					{	
						if(pView->GetActiveTab() == LinksModule::ReceiveLabsTab) {
							pView->UpdateView();
						}
						else {
							pView->SetActiveTab(LinksModule::ReceiveLabsTab);
						}
					}
				}
			}
		}

		// (j.jones 2011-03-15 14:18) - PLID 42738 - supported the device importer
		if(dwTypes & NT_DEVICE_IMPORT) {
			DeviceImport::GetMonitor().ShowDevicePluginImportDlg();
		}

		if(dwTypes & NT_DEFAULT) {
			ShowAlertDialog();
		}
		if (dwTypes & NT_GOTOPATIENT) {
			if (m_nPatientIDToGoToOnClickNotification != -1) {
				BringWindowToTop();
				m_nPatientIDToGoToOnClickNotification = -1;
				if(m_patToolBar.TrySetActivePatientID(m_nPatientIDToGoToOnClickNotification)) {
					UpdateAllViews();
					FlipToModule(PATIENT_MODULE_NAME);
				}
			}
		}
		// (j.jones 2016-02-03 16:50) - PLID 68118 - split this message into two types
		if(dwTypes & NT_SS_RENEWALS_NEEDING_ATTENTION
			|| dwTypes & NT_RX_NEEDING_ATTENTION) {
			BringWindowToTop();
			//TES 4/9/2009 - PLID 33889 - Open up the SureScripts Messages dialog.
			//TES 4/17/2009 - PLID 33889 - Tell it to pre-filter the dialog based on the providers the user prefers to be
			// notified for.
			// (j.fouts 2013-01-07 17:20) - PLID 54472 - Open the prescriptions needing attention dlg
			// (j.jones 2016-02-03 16:54) - PLID 68118 - now we tell it which screen to display
			OpenPrescriptionsRenewalsNeedingAttention((dwTypes & NT_SS_RENEWALS_NEEDING_ATTENTION) ? true : false);
		}

		// (j.gruber 2010-01-07 15:05) - PLID 36648 - added ability to make new patient with referral
		if (dwTypes & NT_NEW_PATIENT) {
			BringWindowToTop();
			//open the new patient dialog
			if (m_bNewPatientOpen) {
				//it's already open once, it can't be opened again, so pop up a message telling them that and then put it back in the list
				MsgBox("Please close the New Patient Dialog before creating a new patient from Caller ID");
				//we didn't clear the notification above, so we are good to go
			}
			else {
				long nTempRefID = m_nReferralIDForNewPatient;
				m_nReferralIDForNewPatient = -1;
				OnNewPatientBtn(nTempRefID);						
			}
		}

		//TES 11/1/2013 - PLID 59276 - Added CDS notifications
		if(dwTypes & NT_CDS_INTERVENTION) {
			BringWindowToTop();
			OpenInterventionsList();
		}

		KillTimer(IDT_WAIT_FOR_SHEET_TIMER);
	}
	NxCatchAll(__FUNCTION__);

	return 0;
}

void CMainFrame::AllowPopup()
{
	if(m_nCountPopupDisallows > 0) InterlockedDecrement(&m_nCountPopupDisallows);
	if(m_nCountPopupDisallows <= 0 && m_nWaitingType != WT_DONT_WAIT && m_nWaitingType != WT_WAIT_ALWAYS) {
		EnsureNotificationDlg();
		m_pNotificationDlg->ReleaseNotifications();
	}
}

void CMainFrame::DisallowPopup()
{
	InterlockedIncrement(&m_nCountPopupDisallows);
}

BOOL CMainFrame::IsPopupAllowed()
{
	if(m_nCountPopupDisallows > 0 || m_nWaitingType == WT_WAIT_ALWAYS) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

LRESULT CMainFrame::OnMessageAllowPopup(WPARAM wParam, LPARAM lParam)
{
	if(m_nWaitingType != WT_WAIT_ALWAYS) {
		CNxTabView *pView = GetActiveView();
		CNxDialog* pSheet = NULL;
		if(pView && pView->IsKindOf(RUNTIME_CLASS(CNxTabView))) pSheet = pView->GetActiveSheet();
		if(pSheet && pSheet->IsKindOf(RUNTIME_CLASS(CNxDialog))) {
			m_nWaitingType = pSheet->AllowPopup();
			if(m_nWaitingType == WT_DONT_WAIT) {
				if(IsPopupAllowed()) {
					//Hooray!
					//TES 1/4/2007 - PLID 24119 - I don't know how we could get here without m_pNotificationDlg being 
					// valid, but it couldn't hurt to check.
					EnsureNotificationDlg();
					m_pNotificationDlg->ReleaseNotifications();
				}
				else {
					m_nWaitingType = WT_WAIT_FOR_SELF;
					KillTimer(IDT_WAIT_FOR_SHEET_TIMER);
				}
			}
			else if(m_nWaitingType == WT_WAIT_FOR_SHEET_ACTIVE) {
				SetTimer(IDT_WAIT_FOR_SHEET_TIMER, 500, NULL);
			}
		}
		else {
			//If we couldn't get an active sheet, just check ourselves.
			if(IsPopupAllowed()) {
				//Hooray!
				//TES 1/4/2007 - PLID 24119 - I don't know how we could get here without m_pNotificationDlg being 
				// valid, but it couldn't hurt to check.
				EnsureNotificationDlg();
				m_pNotificationDlg->ReleaseNotifications();
			}
			else {
				m_nWaitingType = WT_WAIT_FOR_SELF;
				KillTimer(IDT_WAIT_FOR_SHEET_TIMER);
			}
		}
	}
	return 0;
}

void CMainFrame::OnIDPASetup() {

	CIDPASetupDlg dlg(this);
	dlg.DoModal();
}

void CMainFrame::OnNYWCSetup() {

	CNYWCSetupDlg dlg(this);
	dlg.DoModal();
}

// (j.jones 2007-05-09 17:42) - PLID 25550 - added MICR Setup
void CMainFrame::OnMICRSetup() {

	CMICRSetupDlg dlg(this);
	dlg.DoModal();
}

// (k.messina 2010-07-15 16:12) - PLID 39685 - added NY Medicaid Setup
void CMainFrame::OnNYMedicaidSetup() {

	CNYMedicaidSetupDlg dlg(this);
	dlg.DoModal();
}

void CMainFrame::OnSSRSSetup() 
{
	try {
		CSSRSSetupDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}

void CMainFrame::RequestTableCheckerMessages(HWND hwndMessageRecipient)
{
	bool bAddedAlready = false;
	for(int i = 0; i < m_arCheckerWnds.GetSize(); i++) {
		if(m_arCheckerWnds.GetAt(i) == hwndMessageRecipient) {
			bAddedAlready = true;
		}
	}
	if(!bAddedAlready) m_arCheckerWnds.Add(hwndMessageRecipient);
}

void CMainFrame::UnrequestTableCheckerMessages(HWND hwndMessageRecipient)
{
	//Make new array, copy everything but what was passed in, clear member, copy back.
	CArray<HWND, HWND> arTmp;
	for(int i = 0; i < m_arCheckerWnds.GetSize(); i++) {
		if(m_arCheckerWnds.GetAt(i) != hwndMessageRecipient) {
			arTmp.Add(m_arCheckerWnds.GetAt(i));
		}
	}
	m_arCheckerWnds.RemoveAll();
	for(i = 0; i < arTmp.GetSize(); i++) {
		m_arCheckerWnds.Add(arTmp.GetAt(i));
	}
}

void CMainFrame::TransferTodos()
{
	try {
		// (e.lally 2009-06-10) PLID 34529 - Check permissions - View patients module
		if(CheckCurrentUserPermissions(bioPatientsModule, sptView)) {
			CTransferTodosDlg dlg(this);
			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnCalc()
{
	try {
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		// (j.jones 2010-09-02 15:44) - PLID 40388 - fixed crash caused by passing in "this" as a parent
		ShellExecute((HWND)GetDesktopWindow(), NULL, "calc.exe", NULL, NULL, SW_SHOW);
	}NxCatchAll("Error opening calculator.");
}

// (d.thompson 2009-10-21) - PLID 36015 - This has been removed.  Please just create your own
//	local CProductItemsDlg if you need to use it.  This is a followup to my note below from 2007
//	where I recommended people quit using it.  It's just too difficult/impossible to maintain.
/*
CProductItemsDlg& CMainFrame::GetProductItemsDlg()
{
	//DRT 11/7/2007 - NOTE:  From what I can tell, we originally made this product items
	//	dialog a member of the mainfrm purely so we could get barcode scanning messages to
	//	it.  Some time ago (2004, I think), we came up with a better way of just letting
	//	windows register themselves.  I have now applied that same methodology to the
	//	CProductItemsDlg (PLID 28020).  So, while this member still exists, and is still used in a 
	//	number of places, I highly recommend that any new code does not use this, and 
	//	instead just create your own local dialog.

	// (c.haag 2003-08-01 17:51) - Assign default values
	m_dlgProductItems.m_bUseSerial = TRUE;
	m_dlgProductItems.m_bUseExpDate = TRUE;
	m_dlgProductItems.m_NewItemCount = 0;
	m_dlgProductItems.m_EntryType = PI_ENTER_DATA;
	m_dlgProductItems.m_CountOfItemsNeeded = 1;
	m_dlgProductItems.m_strWhere = "";
	m_dlgProductItems.m_bAllowQtyGrow = FALSE;
	m_dlgProductItems.m_bDisallowQtyChange = FALSE;
	m_dlgProductItems.m_bIsAdjustment = FALSE;
	m_dlgProductItems.m_adwProductItemIDs.RemoveAll();
	m_dlgProductItems.m_bUseUU = FALSE;
	m_dlgProductItems.m_bSerialPerUO = FALSE;
	m_dlgProductItems.m_nConversion = 1;
	m_dlgProductItems.m_nLocationID = -1;
	// (j.jones 2009-04-01 09:39) - PLID 33559 - make sure this is set to false by default
	m_dlgProductItems.m_bSaveDataEntryQuery = false;	
	m_dlgProductItems.m_strCreatingAdjustmentID = "NULL"; // (j.jones 2009-04-08 08:40) - PLID 33096
	m_dlgProductItems.m_bDeclareNewProductItemID = TRUE;	// (j.jones 2009-07-09 17:09) - PLID 32684	
	m_dlgProductItems.m_strSavedDataEntryQuery = ""; // (j.jones 2009-07-09 17:59) - PLID 34842 - make sure this gets cleared!
	return m_dlgProductItems;
}
*/

LRESULT CMainFrame::OnPromptMoveup(WPARAM wParam, LPARAM lParam)
{
	try {
		// (c.haag 2004-03-01 12:46) - PLID 10535 The cancelled appointment will stay visible
		// on the scheduler while the message box is active unless we do an explicit UpdateView.
		//GetActiveView()->UpdateView();

		if (IDYES == MsgBox(MB_YESNO, "Would you like to search for an appointment to move up to the time of the cancelled appointment?"))
		{
			long nReservationID = (long)wParam;
			CDWordArray* padwResources = new CDWordArray;
			_RecordsetPtr prsRes = CreateRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = %d",
				nReservationID);
			while (!prsRes->eof)
			{
				padwResources->Add(AdoFldLong(prsRes, "ResourceID"));
				prsRes->MoveNext();
			}						
			PostMessage(NXM_OPEN_MOVEUP_DIALOG, (WPARAM)lParam /* this is a pointer to a COleDateTime
				variable that is the start time of the appointment */, (LPARAM)padwResources);
		}
		else {
			// (z.manning 2010-10-21 09:38) - PLID 41049 - Need to clean up memory
			COleDateTime* pdtStart = (COleDateTime*)lParam;
			delete pdtStart;
			pdtStart = NULL;
		}
	}
	NxCatchAll("Error in OnPromptMoveup");
	return 0;
}

LRESULT CMainFrame::OnMessageOpenMoveupDialog(WPARAM wParam, LPARAM lParam)
{
	COleDateTime* pdtStart = (COleDateTime*)wParam;
	CDWordArray* padwResources = (CDWordArray*)lParam;
	try
	{		
		BOOL bSuccess = FALSE;
		CNxTabView *pView = GetActiveView();
		if (pView && !pView->IsKindOf(RUNTIME_CLASS(CSchedulerView))) {
			if(!FlipToModule(SCHEDULER_MODULE_NAME)) {
				// (z.manning 2010-10-21 09:41) - PLID 41049 - Need to clean up memory if we ever got here.
				delete pdtStart;
				delete padwResources;
				return 0;
			}
		}
		pView = GetActiveView();
		if (pView && pView->IsKindOf(RUNTIME_CLASS(CSchedulerView)))
		{
			((CSchedulerView*)pView)->ShowMoveUpList(*pdtStart, *padwResources);
			delete padwResources;
			bSuccess = TRUE;
		}
		if (!bSuccess)
			MsgBox("Failed to open the Move Up window");
	}
	NxCatchAll("Error opening the Move Up window"); 
	delete pdtStart;
	return 0;
}

void CMainFrame::OpenAppointment(long nResID)
{
	try {		
		if (!UserPermission(SchedulerModuleItem))
			return;

		// (c.haag 2004-04-21 16:06) - PLID 12050 - Don't use FlipToModule because it posts
		// a message to close the resentry as soon as it's open!
		CChildFrame *pFrame;

		// Check to see if this type of module is already open
		pFrame = GetOpenViewFrame(SCHEDULER_MODULE_NAME);
		if (pFrame) {
			// If so, make sure it is the active view
			if (!GetActiveViewFrame()->IsOfType(SCHEDULER_MODULE_NAME)) {
				pFrame->MDIActivate();
				if(pFrame->GetActiveView()) {
					((CNxView *)pFrame->GetActiveView())->ShowToolBars();//bvb
				}
				UpdateToolBarButtons();
			}
		} else {
			// If the module isn't open yet, open it
			OpenModule(SCHEDULER_MODULE_NAME);

		}

		//if(FlipToModule(SCHEDULER_MODULE_NAME)) {
			CNxTabView *pView = GetActiveView();
			if (pView && pView->IsKindOf(RUNTIME_CLASS(CSchedulerView))) {
				((CSchedulerView *)pView)->OpenAppointment(nResID, TRUE);
			}
		//}
	}
	NxCatchAll("Error in OpenAppointment()");
}

CNotificationDlg *CMainFrame::GetNotificationDlg()
{
	EnsureNotificationDlg();
	return m_pNotificationDlg;
}

LRESULT CMainFrame::OnExplicitDestroyView(WPARAM wParam, LPARAM lParam)
{
	try {
		CNxView* pView = (CNxView*)lParam;

		// (z.manning 2008-11-17 10:00) - PLID 32049 - This view that was passed in here may have already
		// been destroyed, so let's only destroy it here if it's still in the list of the main frame's views.
		BOOL bViewInMainFrame = FALSE;
		for(int nViewIndex = 0; nViewIndex < m_pView.GetSize() && !bViewInMainFrame; nViewIndex++) {
			if(m_pView.GetAt(nViewIndex) == pView) {
				bViewInMainFrame = TRUE;
			}
		}

		if(pView != NULL && bViewInMainFrame) {
			CMDIChildWnd* pChild = (CMDIChildWnd*)pView->GetParent();
			if (pChild)
				pChild->MDIDestroy();
		}
	} NxCatchAll("Error destroying view.");
	return 0;
}

// Called by our tapi utilities whenever a caller id number is given to us
void CMainFrame::OnReceivedCallerIdNumber(LPCTSTR strCallerIdNumber)
{
	try {
		// Clean up the number so we have just the digits
		CString strCleanNumber(strCallerIdNumber);
		strCleanNumber.TrimLeft();
		strCleanNumber.TrimRight();
		strCleanNumber.Replace("(", "");
		strCleanNumber.Replace(")", "");
		strCleanNumber.Replace("-", "");
		strCleanNumber.Replace(" ", "");
		// After cleaning it, do we even have a number?
		if (!strCleanNumber.IsEmpty() && !ContainsNonDigits(strCleanNumber)) {
			// We do have a number, so we're going to send a notification
			// (j.gruber 2010-01-07 14:36) - PLID 36648 - check our preference to see if we are looking up a patient or referral source
			Notification stNotification;

			// Format the caller id number nicely
			CString strFormattedPhone;
			FormatText(strCleanNumber, strFormattedPhone, "(nnn) nnn-nnnn");


			// (j.gruber 2010-01-12 11:39) - PLID 36646 - check the preference
			if (GetRemotePropertyInt("CallerIDLookupBehavior", 0, 0, "<None>") == 0) {
			
				stNotification.nNotificationType = NT_GOTOPATIENT;
				// For the notification string, see if we can find out the patient this phone number is connected to
				_RecordsetPtr prs = CreateRecordset(
					"SELECT ID, First, Last FROM PersonT WHERE ID = ("
					"	SELECT MIN(ID) FROM PersonT WHERE '%s' IN ("
					"		REPLACE(REPLACE(REPLACE(REPLACE(RTRIM(LTRIM(HomePhone)), '(', ''), ')', ''), '-', ''), ' ', ''), "
					"		REPLACE(REPLACE(REPLACE(REPLACE(RTRIM(LTRIM(WorkPhone)), '(', ''), ')', ''), '-', ''), ' ', '')) "
					")", _Q(strCleanNumber));			
				
				if (!prs->eof) {
					// Found the patient, so remember the patient id and include the patient name in the notification string
					m_nPatientIDToGoToOnClickNotification = AdoFldLong(prs->GetFields(), "ID");
					FieldsPtr pflds = prs->GetFields();
					stNotification.strMessage.Format(
						"Caller Id: %s\r\n"
						"Patient: %s %s", strFormattedPhone, AdoFldString(pflds, "First", ""), AdoFldString(pflds, "Last", ""));
				} else {
					// Couldn't find the patient, so don't store a patient id and say "no patient found" in the notification string
					m_nPatientIDToGoToOnClickNotification = -1;
					stNotification.strMessage.Format(
						"Caller Id: %s\r\n"
						"(No patient found)", strFormattedPhone);
				}
			}
			else {
				stNotification.nNotificationType = NT_NEW_PATIENT;

				//let's see if we can find the referral source this goes with
				_RecordsetPtr rs = CreateParamRecordset(" SELECT PersonID, Name "
				 " FROM ReferralSourceT INNER JOIN PersonT ON ReferralSourceT.PersonID = PersonT.ID "
				 " WHERE {STRING} = REPLACE(REPLACE(REPLACE(REPLACE(RTRIM(LTRIM(WorkPhone)), '(', ''), ')', ''), '-', ''), ' ', '') ",
				 strCleanNumber);

				if (!rs->eof) {
					m_nReferralIDForNewPatient = AdoFldLong(rs, "PersonID");
					stNotification.strMessage.Format(
						"Caller Id: %s\r\n"
						"New Patient Referral: %s", strFormattedPhone, AdoFldString(rs, "Name"));
				}
				else {
					m_nReferralIDForNewPatient = -1;
					stNotification.strMessage.Format(
						"Caller Id: %s\r\n"
						"Referral Source: Not Found", strFormattedPhone);
				}
			}
			
			// Finalize and send the notification
			stNotification.nCount = 1;
			stNotification.bIncrement = false;
			EnsureNotificationDlg();
			m_pNotificationDlg->AddNotification(stNotification);
		}
	} NxCatchAll("CMainFrame::OnReceivedCallerIdNumber");
}

BOOL CMainFrame::CheckAllowCreatePatient()
{
	long nPatCountLimit = g_pLicense->GetPatientCountLimit();
	if (nPatCountLimit != -1) {
		// Limited to a certain number.  Check the data to see if we're past that number.
		long nCurPatientCount = AdoFldLong(CreateRecordsetStd("SELECT COUNT(*) AS CurPatCount FROM PatientsT WHERE PersonID <> -25")->GetFields()->GetItem("CurPatCount"));
		if (nCurPatientCount >= nPatCountLimit) {
			// They're past the limit
			CString strMsg;
			strMsg.Format(
				"Your current license limits you to %li patients.  Since you already have %li patients in your "
				"database you may not add any more.\r\n\r\nPlease contact NexTech at 1-800-247-2587 to purchase a full license.",
				nPatCountLimit, nCurPatientCount);
			AfxMessageBox(strMsg, MB_OK|MB_ICONHAND);
			// Not allowed to create a new patient
			return FALSE;
		} else {
			// They're still within the limit, but warn them that there is a limit
			CString strMsg;
			strMsg.Format(
				"Your current license limits you to %li patients.  You have %li patients in your database right now.  Once "
				"you reach the limit you will no longer be able to add new patients.  Please contact NexTech at 1-800-247-2587 to purchase a full license.\r\n\r\nWould you like to proceed?",
				nPatCountLimit, nCurPatientCount);
			int nResult = DontShowMeAgain(this, strMsg, "LicensePatCountLimitWarning", "Limited License", FALSE, TRUE);
			if (nResult == IDYES) {
				// They got the message and clicked YES
				return TRUE;
			} else if (nResult == IDOK) {
				// Last time they said "don't remind me again" so this time the prompt never popped up, which results in IDOK
				return TRUE;
			} else {
				// They got the message and clicked NO
				return FALSE;
			}
		}
	} else {
		// Unlimited so don't bother checking
		return TRUE;
	}
}

LRESULT CMainFrame::OnOutlookSyncFinished(WPARAM wParam, LPARAM lParam)
{
	UpdateAllViews();
	return 0;
}

CRPEngine* CMainFrame::GetReportEngine()
{
	if(!m_pReportEngine) {
		m_pReportEngine = new CRPEngine;
	}
	return m_pReportEngine;
}

void CMainFrame::ClearReportEngine()
{
	// (a.walling 2009-10-14 13:20) - PLID 35941 - The report engine destructor can result in the mainframe's ClearReportEngine being called again!
	if(m_pReportEngine) {
		if (!m_bReportEngineClosing) {
			m_bReportEngineClosing = true;
			delete m_pReportEngine;
			m_pReportEngine = NULL;
			m_bReportEngineClosing = false;
		}
	}
}

void CMainFrame::PostToBillID()
{
	try {

		if(!g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrUse))
			return;

		if (!UserPermission(EditPayment))
			return;

		// (j.jones 2010-05-18 17:53) - PLID 38685 - this feature is now always available
		/*
		if(GetRemotePropertyInt("SearchByBillID", 0, 0, "<None>", TRUE) == 0) {
			AfxMessageBox("To use this feature, you must first enable the preference under the\n"
						  "'Financial' settings, called 'Enable looking up claims by claim number'.");
			return;
		}
		*/
		
		long nBillID;
		CString strID;
		if (InputBox(this, "Enter a Bill / Claim ID", strID, "") == IDOK) {
			nBillID = atol(strID);
			if(nBillID == 0) {
				AfxMessageBox("Please enter a valid Bill ID.");
				return;
			}
		}
		else {
			return;
		}

		long nPatientID = -1;

		_RecordsetPtr rs = CreateParamRecordset("SELECT ID, PatientID FROM BillsT WHERE Deleted = 0 AND ID = {INT}",nBillID);
		if(rs->eof) {
			AfxMessageBox("That Bill ID does not exist in the system.");
			return;
		}
		else {
			nPatientID = AdoFldLong(rs, "PatientID",-1);
		}

		// (j.jones 2011-09-13 15:37) - PLID 44887 - you can't apply to this bill
		// if it only has original/void charges
		if(IsVoidedBill(nBillID)) {
			AfxMessageBox("This bill has been corrected, and can not be modified.");
			return;
		}
		else if(!DoesBillHaveUncorrectedCharges(nBillID)) {
			AfxMessageBox("All charges in this bill have been corrected, and can not be modified.");
			return;
		}

		if (nPatientID != -1) {
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {

				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
					if(IDNO == MessageBox("This bill's patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
				//TES 1/7/2010 - PLID 36761 - This function may fail now
				if(!pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {
					return;
				}

				//Now just flip to the patient's module and set the active Patient
				pMainFrame->FlipToModule(PATIENT_MODULE_NAME);

				//open billing tab

				CNxTabView * pView = GetMainFrame()->GetActiveView();
				if (pView) {
					if (GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME)) {
						pView->UpdateView();
						pView->SetActiveTab(PatientsModule::BillingTab);
					}
				}
				CFinancialLineItemPostingDlg dlg(this);
				// (j.jones 2012-08-16 10:20) - PLID 52162 - changed the parameters of line item posting,
				// this call only needs the billID now
				// (j.jones 2014-06-27 10:43) - PLID 62548 - added PayType, always medical here
				dlg.DoModal(eMedicalPayment, nBillID);

				pView = pMainFrame->GetActiveView();
				if(pView)
					pView->UpdateView();

			}//end if MainFrame
			else {
				MsgBox(MB_ICONSTOP|MB_OK, "ERROR: Cannot open mainframe!");
			}//end else pMainFrame
		}//end if nPatientID

	}NxCatchAll("Error posting to selected bill ID.");
}

void CMainFrame::OnActivityTimeoutSet()
{
	if (IsWindow(m_dlgAutoLogoff.GetSafeHwnd()))
		m_dlgAutoLogoff.ResetInactivityTimer();
}

void CMainFrame::TryResetAutoLogOffTimer(UINT message)
{
	if (!IsWindow(m_dlgAutoLogoff.GetSafeHwnd()))
		return;

	/*
	if (message == WM_PAINT || message == WM_SYNCPAINT ||
		message == WM_NCPAINT || message == WM_DRAWITEM ||
		message == WM_ERASEBKGND || message == WM_TIMER ||
		message == 0x363)
	{
		return;
	}
	*/
	// (a.walling 2009-06-05 16:56) - PLID 34512
	/*
	our old method simply checked for any message that was not:
	WM_PAINT, WM_SYNCPAINT, WM_NCPAINT, WM_DRAWITEM, WM_ERASEBKGND, WM_TIMER, 0x363.

	Note I have no idea what 0x363 is. It was added by chris in 9672 version 578 with 
	the description 'Fixed bug in the auto-logoff mechanism'. WM_DRAGON perhaps.
	(update -- not as cool as WM_DRAGON, it's actually WM_IDLEUPDATECMDUI. I had
	found that WM_AFXFIRST is 0x0360 and WM_AFXLAST is 0x037f, and this range is full
	of neat MFC messages. Kudos to wine (http://winehq.org) )

	Anyway, I had messed around with this a long while ago researching a client's 
	complaint about the feature. I have a function called IsInputMessage. So we now 
	do the opposite. Rather than assume we know every message that is NOT an input
	message (which is insane), we assume we know every message that IS an input message, 
	which I think is pretty sane. 
	Basically the WM_(NC)?[LRMX]BUTTON(DOWN)|(UP)|(DBCLK) messages, WM_MOUSEWHEEL and 
	WM_MOUSEHWHEEL, and WM_MOUSEMOVE and WM_NCMOUSEMOVE for mouse messages. 
	And WM_(SYS)?KEY(UP)|(DOWN) for keyboard, along with WM_SYSCOMMAND and 
	WM_CONTEXTMENU just to be safe.
	*/
	if (IsInputMsg(message)) {
		m_dlgAutoLogoff.ExtendInactivityTimer();
	}
}

LRESULT CMainFrame::OnNewPayment(WPARAM wParam, LPARAM lParam)
{
	NewPaymentMessageInfo *pPaymentMessageInfo = NULL;
	try {

		//wParam is the ID of the bill or charge we are auto-applying to
		//it is -1 if we are not auto-applying to anything

		// (j.jones 2009-08-25 12:44) - PLID 31549 - we now can support two IDs
		// (z.manning 2010-02-22 16:22) - PLID 37479 - We could only support 2 IDs previously if
		// both of them were less than 65,635. Now we can support to long values.
		pPaymentMessageInfo = (NewPaymentMessageInfo*)wParam;
		long nID1 = pPaymentMessageInfo->nID1;
		long nID2 = pPaymentMessageInfo->nID2;

		if(nID1 == 0) {
			nID1 = -1;
		}
		if(nID2 == 0) {
			nID2 = -1;
		}

		//lParam is the "type" of ID
		//1 - Bill, 2 - Charge, 3 - Quote/PIC, 4 - PatientID, 0 - Nothing
		// (a.walling 2007-10-08 15:25) - PLID 9801 - Get the type and previous reasons info
		WORD Type = pPaymentMessageInfo->nType;

		//EOpenDrawerReason
		WORD odr = pPaymentMessageInfo->odr;

		CPaymentDlg dlg(this);

		// (a.walling 2007-10-08 15:37) - PLID 9801 - Pass the previous reasons info into the dialog
		dlg.m_odrPreviousReasons = odr;

		CString str;

		if(Type == 1 && nID1 > 0) {
			//applying to a bill			

			//get the patient ID
			long nPatientID = -1;
			_RecordsetPtr rs = CreateRecordset("SELECT PatientID FROM BillsT WHERE ID = %li",nID1);
			if(!rs->eof) {
				nPatientID = AdoFldLong(rs, "PatientID",-1);
				dlg.m_PatientID = nPatientID;
			}
			rs->Close();

			COleCurrency cyCharges, cyPayments, cyAdjustments, cyRefunds, cyInsurance;
			GetBillTotals(nID1, nPatientID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance);

			COleCurrency cyPatBalance = cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance;

			COleCurrency cyBillBalance = COleCurrency(0,0);
			rs = CreateRecordset("SELECT Sum(ChargeRespT.Amount) - Sum(CASE WHEN AppliesQ.AppliedAmount Is Null THEN 0 ELSE AppliesQ.AppliedAmount END) AS Balance FROM BillsT "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
				"LEFT JOIN (SELECT RespID, Sum(Amount) AS AppliedAmount FROM AppliesT GROUP BY RespID) AS AppliesQ "
				"ON ChargeRespT.ID = AppliesQ.RespID "
				"WHERE LineItemT.Deleted = 0 AND BillsT.ID = %li", nID1);
			if(!rs->eof) {
				cyBillBalance = AdoFldCurrency(rs, "Balance");
			}
			rs->Close();

			if(cyBillBalance <= COleCurrency(0,0)) {
				if(IDNO == MessageBox("This bill now has a zero balance. The new payment will not be applied to this bill.\n"
					"Do you still wish to make a new unapplied payment?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					delete pPaymentMessageInfo;
					return 0;
				}
			}
			else {
				//assume patient resp. by default
				dlg.m_cyFinalAmount = cyPatBalance;
				dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;

				dlg.m_varBillID = nID1;
				dlg.m_ApplyOnOK = TRUE;
			}
		}
		else if(Type == 2 && nID1 > 0) {
			//applying to a charge

			//get the patient ID
			long nPatientID = -1;
			_RecordsetPtr rs = CreateRecordset("SELECT PatientID FROM LineItemT WHERE ID = %li",nID1);
			if(!rs->eof) {
				nPatientID = AdoFldLong(rs, "PatientID",-1);
				dlg.m_PatientID = nPatientID;
			}
			rs->Close();

			COleCurrency cyCharges, cyPayments, cyAdjustments, cyRefunds, cyInsurance;
			GetChargeTotals(nID1, nPatientID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance);
			
			COleCurrency cyPatBalance = cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance;

			COleCurrency cyChargeBalance = COleCurrency(0,0);
			rs = CreateRecordset("SELECT Sum(ChargeRespT.Amount) - Sum(CASE WHEN AppliesQ.AppliedAmount Is Null THEN 0 ELSE AppliesQ.AppliedAmount END) AS Balance FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
				"LEFT JOIN (SELECT RespID, Sum(Amount) AS AppliedAmount FROM AppliesT GROUP BY RespID) AS AppliesQ "
				"ON ChargeRespT.ID = AppliesQ.RespID "
				"WHERE LineItemT.Deleted = 0 AND ChargesT.ID = %li",nID1);
			if(!rs->eof) {
				cyChargeBalance = AdoFldCurrency(rs, "Balance");
			}
			rs->Close();

			if(cyChargeBalance <= COleCurrency(0,0)) {
				if(IDNO == MessageBox("This charge now has a zero balance. The new payment will not be applied to this charge.\n"
					"Do you still wish to make a new unapplied payment?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					delete pPaymentMessageInfo;
					return 0;
				}
			}
			else {
				//assume patient resp. by default
				dlg.m_cyFinalAmount = cyPatBalance;
				dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;

				dlg.m_varChargeID = nID1;
				dlg.m_ApplyOnOK = TRUE;
			}			
		}
		// (j.jones 2009-08-25 12:37) - PLID 31549 - in this case we also support ProcInfoID
		//nID1 = QuoteID
		//nID2 = ProcInfoID
		else if(Type == 3 && (nID1 > 0 || nID2 > 0)) {
			
			//prepayment for a quote or PIC

			long nQuoteID = nID1;
			long nProcInfoID = nID2;
			
			BOOL bIsPackage = FALSE;
			if(nQuoteID > 0) {

				COleCurrency cyTotalQuote = COleCurrency(0,0);
				COleCurrency cyQuoteBalance = COleCurrency(0,0);

				_RecordsetPtr rs = CreateParamRecordset("SELECT PatientID, CurrentAmount, dbo.GetQuoteTotal(ID,0) AS QuoteTotal FROM BillsT "
					"LEFT JOIN PackagesT ON BillsT.ID = PackagesT.QuoteID "
					"WHERE BillsT.ID = {INT}", nQuoteID);
				if(!rs->eof) {
					if(rs->Fields->Item["CurrentAmount"]->Value.vt == VT_NULL)
						//not a package, get total quote
						cyTotalQuote = AdoFldCurrency(rs, "QuoteTotal",COleCurrency(0,0));
					else {
						//is a package, get remaining amount
						cyTotalQuote = AdoFldCurrency(rs, "CurrentAmount",COleCurrency(0,0));
						bIsPackage = TRUE;
					}
					dlg.m_PatientID = AdoFldLong(rs, "PatientID",-1);
				}
				rs->Close();

				rs = CreateParamRecordset("SELECT Coalesce(Sum(LineItemT.Amount) - Sum(CASE WHEN AppliesQ.AppliedAmount Is Null THEN 0 ELSE AppliesQ.AppliedAmount END),0) AS Balance FROM LineItemT "
					"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
					"LEFT JOIN (SELECT SourceID, Sum(Amount) AS AppliedAmount FROM AppliesT GROUP BY SourceID) AS AppliesQ "
					"ON PaymentsT.ID = AppliesQ.SourceID "
					"WHERE LineItemT.Deleted = 0 AND PaymentsT.QuoteID = {INT}", nQuoteID);
				if(!rs->eof) {
					cyQuoteBalance = cyTotalQuote - AdoFldCurrency(rs, "Balance");
				}
				rs->Close();

				if(cyQuoteBalance <= COleCurrency(0,0)) {
					if(IDNO == MessageBox("This quote already has enough prepayments to cover the balance.\n"
						"Do you still wish to make a new prepayment?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						delete pPaymentMessageInfo;
						return 0;
					}
				}
				else {
					//assume patient resp. by default
					dlg.m_cyFinalAmount = cyQuoteBalance;
					dlg.m_cyMaxAmount = cyTotalQuote;

					dlg.m_varBillID = nQuoteID;
					dlg.m_QuoteID = nQuoteID;
					dlg.m_nProcInfoID = nProcInfoID;
					dlg.m_bIsPrePayment = TRUE;
				}
			}
			else if(nProcInfoID > 0) {
				// (j.jones 2009-08-25 12:37) - PLID 31549 - get the patient ID from the PIC
				_RecordsetPtr rs = CreateParamRecordset("SELECT PatientID FROM ProcInfoT WHERE ID = {INT}", nProcInfoID);
				if(!rs->eof) {
					dlg.m_PatientID = AdoFldLong(rs, "PatientID",-1);					
					dlg.m_nProcInfoID = nProcInfoID;
					dlg.m_bIsPrePayment = TRUE;
				}
				rs->Close();
			}
		}
		else if(Type == 4 && nID1 > 0) {
			//a standalone payment for a patient
			dlg.m_PatientID = nID1;
		}

		dlg.DoModal(__FUNCTION__, __LINE__);

		//update the view
		UpdateAllViews();

	}NxCatchAll("Error creating new payment.");

	// (z.manning 2010-02-22 16:28) - PLID 37479 - Clean up our message info
	if(pPaymentMessageInfo != NULL) {
		delete pPaymentMessageInfo;
	}

	return 0;
}

BOOL CMainFrame::RegisterForBarcodeScan(CWnd* pWnd)
{
	try {
		// (a.walling 2008-02-21 09:14) - PLID 29043 - ASSERT if there are duplicates, and also store the
		// window text in debug mode to assist with debugging.
#ifdef _DEBUG
		for (int i = 0; i < m_aryBarcodeWindows.GetSize(); i++) {
			_ASSERTE((pWnd->GetSafeHwnd() != m_aryBarcodeWindows[i]->GetSafeHwnd()) && (pWnd != m_aryBarcodeWindows[i]));
		}
		
		CString strWindowText;
		pWnd->GetWindowText(strWindowText);
		m_saBarcodeWindowText.Add(strWindowText);

		TRACE("* RegisterForBarcodeScan:\tpWnd(0x%08x) HWND(0x%08x) \"%s\" added to array at index %li\n", pWnd, pWnd->GetSafeHwnd(), strWindowText, m_aryBarcodeWindows.GetSize());
#endif
		m_aryBarcodeWindows.Add(pWnd);
		return TRUE;
	} NxCatchAll("Error registering window for barcode scanner messages.");

	return FALSE;
}

BOOL CMainFrame::UnregisterForBarcodeScan(CWnd* pWnd)
{
	//loop through to find it
	for(int i = 0; i < m_aryBarcodeWindows.GetSize(); i++) {
		if( (m_aryBarcodeWindows.GetAt(i)->GetSafeHwnd() == pWnd->GetSafeHwnd()) || (m_aryBarcodeWindows.GetAt(i) == pWnd)) {
			m_aryBarcodeWindows.RemoveAt(i);
#ifdef _DEBUG
			CString strWindowText = m_saBarcodeWindowText[i];
			
			// (a.walling 2008-02-21 09:15) - PLID 29043 - Keep a list of window text for debugging
			m_saBarcodeWindowText.RemoveAt(i);

			TRACE("* UnregisterForBarcodeScan:\tpWnd(0x%08x) HWND(0x%08x) \"%s\" removed from array at index %li (new size %li)\n", pWnd, pWnd->GetSafeHwnd(), strWindowText, i, m_aryBarcodeWindows.GetSize());

			// now also check to see if there are duplicates
			for (int o = 0; o < m_aryBarcodeWindows.GetSize(); o++) {
				_ASSERTE(m_aryBarcodeWindows.GetAt(o)->GetSafeHwnd() != pWnd->GetSafeHwnd());
			}
#endif
			return TRUE;
		}
	}

	//not found, so it was not removed
	return FALSE;
}

// (a.walling 2008-10-28 13:05) - PLID 31334 - when launched from WIA auto-start
LRESULT CMainFrame::OnWIASourceLaunch(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2008-10-28 13:12) - PLID 31334 - We may be switching users, in which case we can't just open the module!
		if (GetCurrentUserHandle() == NULL) {

			theApp.m_Twain.ResetWIAActivate(TRUE);

			return 0;
		}
		CWaitCursor wc;
		long nFileNum = wParam;

		// Get the real temp path
		CString strTempPath;
		{
			DWORD dwNeedLen = ::GetTempPath(0, NULL);
			::GetTempPath(dwNeedLen, strTempPath.GetBuffer(dwNeedLen+1));
			strTempPath.ReleaseBuffer();
		}

		CString strFileName;
		strFileName.Format("NxWIA%lu", nFileNum);

		CString strDevice;

		HANDLE hFile = ::CreateFile(strTempPath ^ strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			DWORD dwSize = ::GetFileSize(hFile, NULL);

			LPSTR sz = strDevice.GetBuffer(dwSize + 1);
			DWORD dwRead = 0;
			::ReadFile(hFile, sz, dwSize, &dwRead, NULL);
			::CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;

			sz[dwSize] = '\0';
			strDevice.ReleaseBuffer();

			ASSERT(dwRead == strDevice.GetLength());

			::DeleteFile(strTempPath ^ strFileName);
		}

		if(FlipToModule(PATIENT_MODULE_NAME)) {
			CNxTabView* pView = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
			if (pView) {
				pView->SetActiveTab(PatientsModule::HistoryTab);

				// Don't aquire if lParam is non-zero
				if (!lParam)
					((CPatientView*)pView)->m_HistorySheet.WIAAcquireFromDevice(strDevice);
			}
		}
	} NxCatchAll("Error handling WIA event");

	return 0;
}

LRESULT CMainFrame::OnTWAINSourceLaunch(WPARAM wParam, LPARAM lParam)
{
	// (a.walling 2008-10-28 13:12) - PLID 31334 - We may be switching users, in which case we can't just open the module!
	if (GetCurrentUserHandle() == NULL) {

		theApp.m_Twain.ResetCameraActivate(TRUE);

		return 0;
	}
	CWaitCursor wc;
	IStillImage* isi = NULL;
	long nDeviceID = (long)wParam;
	CString strDeviceName;

	// Get the device name
	if ((long)wParam > -1)
	{
		// First make sure sti is available  on this machine
		if (g_dllSti.IsFunctionExported(StiCreateInstance) != NULL) {
			HRESULT hRes = g_dllSti.StiCreateInstance(AfxGetApp()->m_hInstance, STI_VERSION, &isi, NULL);
			if (hRes == S_OK)
			{
				DWORD nDevices = 0;
				STI_DEVICE_INFORMATION* pDevInfo;
				if (S_OK == (hRes = isi->GetDeviceList(NULL, NULL, &nDevices, (void**)&pDevInfo)) &&
					(long)nDevices > nDeviceID)
				{
					strDeviceName = pDevInfo[nDeviceID].pszDeviceDescription;
				}
				LocalFree(pDevInfo);
			}
		}
	}

	if(FlipToModule(PATIENT_MODULE_NAME)) {
		CNxTabView* pView = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
		if (pView) {
			pView->SetActiveTab(PatientsModule::HistoryTab);

			// Don't aquire if lParam is non-zero
			if (!lParam)
				((CPatientView*)pView)->m_HistorySheet.TWAINAcquireFromDevice(strDeviceName);
		}
	}

	if (isi != NULL) isi->Release();
	return 0;
}

// (a.walling 2013-01-17 12:08) - PLID 54666 - Returns CPicContainerDlg*
CPicContainerDlg* CMainFrame::TryOpenExistingPic(long nPicID)
{
	// (c.haag 2006-10-10 10:09) - PLID 22884 - Check if we have an existing PIC open. If so, make it
	// the foreground window. We will not notify the user with a message box; it seems like an extra 
	// click which serves no purpose.
	if (nPicID > -1 && !m_lstpPicContainerDlgs.IsEmpty()) {
		POSITION pos = m_lstpPicContainerDlgs.GetHeadPosition();
		// Traverse the PIC list looking for the one we want to open
		while (pos) {
			CPicContainerDlg *pPicContainer = m_lstpPicContainerDlgs.GetNext(pos);
			if (IsWindow(pPicContainer->GetSafeHwnd()) && pPicContainer->IsWindowVisible() &&
				pPicContainer->GetCurrentPicID() == nPicID) {
				// Jackpot. We found it. Bring the window up if it's minimized
				WINDOWPLACEMENT wp;
				wp.length = sizeof(WINDOWPLACEMENT);
				if (pPicContainer->GetWindowPlacement(&wp)) {
					if (pPicContainer->IsIconic()) {
						if (wp.flags & WPF_RESTORETOMAXIMIZED) {
							wp.showCmd = SW_MAXIMIZE;
						} else {
							wp.showCmd = SW_RESTORE;
						}
						pPicContainer->SetWindowPlacement(&wp);
					}
				}
				// Bring the window to the foreground so the user is sure to see it
				pPicContainer->SetForegroundWindow();
				// Break out of the loop and this function by telling our caller that it activated
				// an existing PIC window
				return pPicContainer;
			}
		}
	}
	return NULL; // We did not find any existing PIC windows
}

// (a.walling 2007-08-24 17:43) - PLID 26195 - Get count of open unsaved EMRs.
long CMainFrame::GetUnsavedEMRCount()
{
	long nCount = 0;

	POSITION pos = m_lstpPicContainerDlgs.GetHeadPosition();

	while (pos) {
		CPicContainerDlg *pPicContainer = m_lstpPicContainerDlgs.GetNext(pos);

		if (pPicContainer && IsWindow(pPicContainer->GetSafeHwnd())) {
			nCount += (pPicContainer->IsEMRUnsaved() ? 1 : 0);
		}

	}

	return nCount;
}

// (a.walling 2013-01-17 12:08) - PLID 54666 - Returns CPicContainerDlg*
CPicContainerDlg* CMainFrame::StartNewEMRRecord(long nPatientID, long nPicID, long nEmnTemplateID)
{
	// (c.haag 2006-04-04 09:40) - PLID 19890 - We now check for create permissions first
	// (j.jones 2007-05-16 08:45) - PLID 25431 - you can't create an EMR
	// without Create and Write permissions, this function cleanly checks for both
	// with only one password prompt or denial message
	if(!CheckHasEMRCreateAndWritePermissions()) {
	//if (!CheckCurrentUserPermissions(bioPatientEMR, sptCreate)) {
		return NULL;
	}

	CWaitCursor wc;
	//TES 7/1/2004: We're going to FlipTo the Patients module, because we need it to be initialized so we can get its
	//m_BillingDlg.  Maybe we can come up with a better way of doing this someday.
	if(FlipToModule(PATIENT_MODULE_NAME)) {
		CPatientView* pView = (CPatientView *)GetOpenView(PATIENT_MODULE_NAME);

		// (c.haag 2006-10-10 09:50) - PLID 22884 - Try to open an existing PIC
		if (CPicContainerDlg* pExisting = TryOpenExistingPic(nPicID)) {			
			if (nEmnTemplateID == -1) {
				return pExisting;
			}

			// (a.walling 2013-01-17 12:08) - PLID 54666 - Even if there is an existing PIC, we still want to create a new EMN
			if (CEMN* pEMN = pExisting->AddEMNFromTemplate(nEmnTemplateID)) {
				pExisting->ShowEMN(pEMN);
			}
			return pExisting;
		}

		// (b.cardillo 2006-06-14 09:14) - PLID 14292 - We now create an object on the heap 
		// instead of just having it in local scope.  We store the pointer in our list and 
		// leave it there until it posts a message to us indicating it has completed, at which 
		// point we release it ourselves (see our NXM_PIC_CONTAINER_CLOSED handler).
		CPicContainerDlg &dlg = *(new CPicContainerDlg);
		// (c.haag 2007-03-07 16:01) - PLID 25110 - Assign the patient ID
		dlg.SetPatientID(nPatientID);
		// (b.cardillo 2006-06-14 09:18) - PLID 14292 - Now that dlg is a reference to an 
		// object allocated on the heap, we can't rely on the stack unwinder to free it if 
		// there's an exception.  So we do it ourselves before the exception escapes.
		try {
			dlg.m_BillingDlg = pView->m_BillingDlg;

			// (a.walling 2013-02-28 14:48) - PLID 55389 - The UpdateView call will reload the NexEMR tab, which is just more burden on the server
			// and client even though it mostly runs in its own thread. This is unnecessary when the patient does not need to change.
			if (nPatientID != GetActivePatientID()) {
				//TES 1/7/2010 - PLID 36761 - This function may fail now
				if(!m_patToolBar.TrySetActivePatientID(nPatientID)) {
					ThrowNxException("Failed to set active patient ID");
					return NULL;
				}

				// (j.jones 2009-09-25 15:03) - PLID 35665 - don't forget to update the current tab
				// to reflect the changed patient
				if(GetActiveView()) {
					GetActiveView()->UpdateView();
				}
			}

			dlg.m_nColor = GetNxColor(GNC_PATIENT_STATUS, m_patToolBar.GetActivePatientStatus());

			m_lstpPicContainerDlgs.AddTail(&dlg);
			//When opening this, allow for an optional nPicID to join the new EMR to, as well as a
			//	TemplateID to start the EMN as.
			// (a.walling 2012-05-01 15:31) - PLID 50117 - Tab numbers don't make sense anymore, now it is bShowPIC
			dlg.OpenPic(nPicID, false, -1, nEmnTemplateID);

			return &dlg;
		} catch (...) {
			delete &dlg;
			throw;
		}

	}

	return NULL;
}

LRESULT CMainFrame::OnEMRGroupClosing(WPARAM wParam, LPARAM lParam)
{
	// (c.haag 2006-03-09 17:40) - PLID 19208 - This is not necessary
//#pragma TODO("(t.schneider 1/13/2006) PLID 19208 - Implement this (if needed) in the new EMR")
	/*CArray<CEMRGroupDlg*,CEMRGroupDlg*> arStillOpen;
	for(int i = 0; i < m_arOpenEMRGroups.GetSize(); i++) {
		if(m_arOpenEMRGroups.GetAt(i)->GetSafeHwnd() != (HWND)wParam) {
			arStillOpen.Add(m_arOpenEMRGroups.GetAt(i));
		}
		else {
			m_arOpenEMRGroups.GetAt(i)->DestroyWindow();
			delete m_arOpenEMRGroups.GetAt(i);
		}
	}
	m_arOpenEMRGroups.RemoveAll();
	for(i = 0; i < arStillOpen.GetSize(); i++) {
		m_arOpenEMRGroups.Add(arStillOpen.GetAt(i));
	}*/
	return 0;
}

// (z.manning 2011-10-28 17:46) - PLID 44594 - This now returns a pointer to the PIC dialog
// Also added a param to open invisibly.
CPicContainerDlg* CMainFrame::EditEmrRecord(long nPicID, long nEmnID /*= -1*/, long nNewEmnTemplateID /*= -1*/)
{
	CWaitCursor wc;
	BOOL bOpen = FALSE;
	BOOL bReadOnly = TRUE;
	//Do they have write permission?
	if(GetCurrentUserPermissions(bioPatientEMR) & (SPT___W________ANDPASS)) {
		bReadOnly = FALSE;
		bOpen = CheckCurrentUserPermissions(bioPatientEMR, sptWrite);
	}
	else {
		bOpen = CheckCurrentUserPermissions(bioPatientEMR, sptRead);
	}

	// (a.walling 2007-11-28 11:32) - PLID 28044 - Check for expired
	if (g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
		bReadOnly = TRUE;
	}
	
	if(bOpen)
	{
		if(FlipToModule(PATIENT_MODULE_NAME))
		{
			CPatientView* pView = (CPatientView *)GetOpenView(PATIENT_MODULE_NAME);
			// (b.cardillo 2006-06-14 09:14) - PLID 14292 - We now create an object on the heap 
			// instead of just having it in local scope.  We store the pointer in our list and 
			// leave it there until it posts a message to us indicating it has completed, at which 
			// point we release it ourselves (see our NXM_PIC_CONTAINER_CLOSED handler).

			// (j.gruber 2012-06-20 11:56) - PLID 51090 - move the PIcID check up further so that we have it before checking for the open EMNs
			try {
				if (-1 == nPicID) 
				{
					// First, make sure the EMR is "bad". This query was abridged from KBID 1741.
					// I took out EmrMasterT.Deleted = 0 because if the EMN was somehow deleted 
					// in the run-up to this function being called, but its EMR has a PIC, then we
					// don't want to create a second PIC for it by accident.
					if (nEmnID > 0) {
						_RecordsetPtr prsPIC = CreateParamRecordset("SELECT PicT.ID FROM PicT "
							"INNER JOIN EMRGroupsT ON EMRGroupsT.ID = PicT.EMRGroupID "
							"INNER JOIN EmrMasterT ON EmrMasterT.EmrGroupID = EmrGroupsT.ID "
							"WHERE EmrMasterT.ID = {INT}", nEmnID);
						if (prsPIC->eof) 
						{
							// Confirmed that there is no PIC. This query will fix the problem. This is the prescribed fix in KBID 1741.
							prsPIC->Close();
							// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
							_RecordsetPtr prs = CreateParamRecordset(
								"SET NOCOUNT ON\r\n "
								"INSERT INTO PicT (ProcInfoID, EmrGroupID, IsCommitted) "
								"SELECT NULL, EmrGroupsT.ID, 1 "
								"FROM EmrMasterT "
								"INNER JOIN EMRGroupsT ON EmrMasterT.EmrGroupID = EmrGroupsT.ID\r\n "
								"WHERE EmrMasterT.ID = {INT} "
								"SET NOCOUNT OFF\r\n "
								"DECLARE @PicID INT\r\n"
								"SET @PicID = SCOPE_IDENTITY()\r\n"
								"SELECT @PicID AS NewPicID", nEmnID
								);
							if (!prs->eof) 
							{
								nPicID = AdoFldLong(prs, "NewPicID");
							}
							else {
								// This could happen if the EMRMasterT record or EMRGroupsT record didn't
								// exist in data, or some other catastrophic failure occured
								AfxThrowNxException("EditEmrRecord failed to auto-create a valid PIC for the EMR containing EMN ID (%d)!", nEmnID);
							}
						}
						else {
							// This -could- happen if someone else made the PIC from another machine and you
							// try to edit the EMR.
							nPicID = AdoFldLong(prsPIC, "ID");
						}
					}
					else {
						// No PIC, no EMN...legacy behavior will fail anyway 
						AfxThrowNxException("EditEmrRecord called without a valid PicID or EMN ID!");
					}

				} // if (-1 == nPicID) {
				else {
					// If we get here, we *expect* to have a valid PIC. Defer to legacy behavior.
				}

			}NxCatchAllCall("CMainFrame::EditEmrRecord - Error Getting PicID",
				return NULL);

			// (c.haag 2006-10-10 09:50) - PLID 22884 - Try to open an existing PIC
			// (a.walling 2013-01-17 12:08) - PLID 54666 - Even if there is an existing PIC, we still want to create a new EMN
			if (CPicContainerDlg* pExisting = TryOpenExistingPic(nPicID)) {
				if (nNewEmnTemplateID == -1) {
					return pExisting;
				}
				if (CEMN* pEMN = pExisting->AddEMNFromTemplate(nNewEmnTemplateID)) {
					pExisting->ShowEMN(pEMN);
				}
				return pExisting;
			}
			CPicContainerDlg &dlg = *(new CPicContainerDlg);
			// (b.cardillo 2006-06-14 09:18) - PLID 14292 - Now that dlg is a reference to an 
			// object allocated on the heap, we can't rely on the stack unwinder to free it if 
			// there's an exception.  So we do it ourselves before the exception escapes.
			try
			{
				dlg.m_BillingDlg = pView->m_BillingDlg;

				// (c.haag 2010-08-04 15:47) - PLID 39980 - If the PicID is -1, we need to create one on the fly. The back story
				// is this: Some databases have EMRs that are not connected to PICs. At the item of this writing, we don't know
				// why. However, they need to be visible, and editable. In order to make them editable, we must make a PIC on
				// the spot.
				
				//DRT 8/24/2005 - PLID 17347 - Fixed up code here -- we were checking for EOF, asserting, then going ahead and 
				//	reading from a known EOF recordset.  I put all the reading code in the else, and put in a message box
				//	explaining that you're in a bad situation.
				// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
				_RecordsetPtr rsPatient = CreateParamRecordset("SELECT PatientID, EMRGroupsT.ID AS EMRID FROM EMRGroupsT INNER JOIN PicT ON EMRGroupsT.ID = PicT.EmrGroupID WHERE PicT.ID = {INT}", nPicID);
				if(rsPatient->eof) {
					//DRT 8/24/2005 - With recent changes between PIC's and EMR's, this case should not be possible.  It's like a bill without a charge (says t.schneider).
					//	So if this happens, throw a good exception so we can come find this and fix the data for them.
					AfxThrowNxException("Could not find EMR for PIC record with ID %li!", nPicID);
				}
				else
				{
					// (a.walling 2013-02-28 14:48) - PLID 55389 - The UpdateView call will reload the NexEMR tab, which is just more burden on the server
					// and client even though it mostly runs in its own thread. This is unnecessary when the patient does not need to change.
                    long nPatientID = AdoFldLong(rsPatient, "PatientID");
					if (nPatientID != GetActivePatientID()) {
						//TES 1/7/2010 - PLID 36761 - This function may fail now
						if(!m_patToolBar.TrySetActivePatientID(nPatientID)) {
							delete &dlg;
							return NULL;
						}

						// (j.jones 2009-09-25 15:03) - PLID 35665 - don't forget to update the current tab
						// to reflect the changed patient
						if(GetActiveView()) {
							GetActiveView()->UpdateView();
						}
					}

					dlg.m_nColor = GetNxColor(GNC_PATIENT_STATUS, m_patToolBar.GetActivePatientStatus());

					// (c.haag 2007-03-07 16:02) - PLID 25110 - Assign the patient ID
					dlg.SetPatientID(nPatientID);

					// (j.jones 2006-02-08 09:04) - PLID 18746 - ensure that you cannot modify the
					// server system time to bypass auditing
					long nEMRID = AdoFldLong(rsPatient, "EMRID",-1);
					if(!ValidateEMRTimestamp(nEMRID)) {
						return NULL;
					}

					rsPatient->Close();
					m_lstpPicContainerDlgs.AddTail(&dlg);
					// (a.walling 2012-05-01 15:31) - PLID 50117 - Tab numbers don't make sense anymore, now it is bShowPIC
					dlg.OpenPic(nPicID, false, nEmnID, nNewEmnTemplateID);

                    // v.arth 05/18/09 - PLID 28569, CCHIT audit compliance
                    try
                    {
                        _RecordsetPtr emrDescriptionRecordSet = CreateParamRecordset("SELECT EMRGroupsT.Description, PersonT.First, PersonT.Middle, PersonT.Last "
                                                                                     "FROM EMRGroupsT "
                                                                                         "INNER JOIN PersonT "
                                                                                             "ON PersonT.ID = EMRGroupsT.PatientID "
                                                                                     "WHERE EMRGroupsT.PatientID = {INT} AND EMRGroupsT.ID = {INT}", nPatientID, nEMRID);
                        if (!emrDescriptionRecordSet->eof)
                        {
                            CString emrDescription = AdoFldString(emrDescriptionRecordSet, "Description");
                            
                            // Get the patient's name
                            CString patientFirstName = "";
                            if (emrDescriptionRecordSet->Fields->Item["First"]->Value.vt != VT_NULL &&
                                emrDescriptionRecordSet->Fields->Item["First"]->Value.vt != VT_EMPTY)
                            {
                                // Get the data in the First field
                                patientFirstName = AdoFldString(emrDescriptionRecordSet, "First");
                            }
                            CString patientMiddleName = "";
                            if (emrDescriptionRecordSet->Fields->Item["Middle"]->Value.vt != VT_NULL &&
                                emrDescriptionRecordSet->Fields->Item["Middle"]->Value.vt != VT_EMPTY)
                            {
                                // Get the data in the First field
                                patientMiddleName = AdoFldString(emrDescriptionRecordSet, "Middle");
                            }
                            CString patientLastName = "";
                            if (emrDescriptionRecordSet->Fields->Item["Last"]->Value.vt != VT_NULL &&
                                emrDescriptionRecordSet->Fields->Item["Last"]->Value.vt != VT_EMPTY)
                            {
                                // Get the data in the First field
                                patientLastName = AdoFldString(emrDescriptionRecordSet, "Last");
                            }
                            CString patientFullName = patientLastName + ", " + patientFirstName + " " + patientMiddleName;

                            // Create the audit event
                            long nAuditID = BeginNewAuditEvent();
                            if (nAuditID != -1)
                            {
                                AuditEvent(nPatientID, patientFullName, nAuditID, aeiEMROpened, nPatientID, emrDescription, "", aepMedium, aetOpened);
                            }

							return &dlg;
                        }
                        else
                        {
                            ThrowNxException("CMainFrame::EditEmrRecord - Cannot track the opening of the EMR.");
                        }
                    }
                    NxCatchAll("CMainFrame::EditEmrRecord - Error tracking EMR access");
				}
			}
			catch (...) {
				delete &dlg;
				throw;
			}
		}
	}

	return NULL;
}

void CMainFrame::OpenPIC(long nPicID)
{
	if(FlipToModule(PATIENT_MODULE_NAME)) {
		CPatientView* pView = (CPatientView *)GetOpenView(PATIENT_MODULE_NAME);

		// (c.haag 2006-10-10 09:50) - PLID 22884 - Try to open an existing PIC
		if (TryOpenExistingPic(nPicID))
			return;

		// (b.cardillo 2006-06-14 09:14) - PLID 14292 - We now create an object on the heap 
		// instead of just having it in local scope.  We store the pointer in our list and 
		// leave it there until it posts a message to us indicating it has completed, at which 
		// point we release it ourselves (see our NXM_PIC_CONTAINER_CLOSED handler).
		CPicContainerDlg &dlg = *(new CPicContainerDlg);
		// (b.cardillo 2006-06-14 09:18) - PLID 14292 - Now that dlg is a reference to an 
		// object allocated on the heap, we can't rely on the stack unwinder to free it if 
		// there's an exception.  So we do it ourselves before the exception escapes.
		try {
			dlg.m_BillingDlg = pView->m_BillingDlg;

			//DRT 8/24/2005 - PLID 17347 - Fixed up code here -- we were checking for EOF, asserting, then going ahead and 
			//	reading from a known EOF recordset.  I put all the reading code in the else, and put in a message box
			//	explaining that you're in a bad situation.
			_RecordsetPtr rsPatient = CreateRecordset("SELECT PatientID FROM ProcInfoT INNER JOIN PicT ON ProcInfoT.ID = PicT.ProcInfoID WHERE PicT.ID = %li", nPicID);
			if(rsPatient->eof) {
				//DRT 8/24/2005 - With recent changes between PIC's and EMR's, this case should not be possible.  It's like a bill without a charge (says t.schneider).
				//	So if this happens, throw a good exception so we can come find this and fix the data for them.
				AfxThrowNxException("Could not find PIC record with ID %li!", nPicID);
			}
			else {
				// (a.walling 2013-02-28 14:48) - PLID 55389 - The UpdateView call will reload the NexEMR tab, which is just more burden on the server
				// and client even though it mostly runs in its own thread. This is unnecessary when the patient does not need to change.
                long nPatientID = AdoFldLong(rsPatient, "PatientID");
				if (nPatientID != GetActivePatientID()) {
					//TES 1/7/2010 - PLID 36761 - This function may fail now
					if(!m_patToolBar.TrySetActivePatientID(AdoFldLong(rsPatient, "PatientID"))) {
						return;
					}

					// (j.jones 2009-09-25 15:03) - PLID 35665 - don't forget to update the current tab
					// to reflect the changed patient
					if(GetActiveView()) {
						GetActiveView()->UpdateView();
					}
				}

				dlg.m_nColor = GetNxColor(GNC_PATIENT_STATUS, m_patToolBar.GetActivePatientStatus());

				// (c.haag 2007-03-07 16:05) - PLID 25110 - Assign the patient ID
				dlg.SetPatientID(nPatientID);

				rsPatient->Close();
				m_lstpPicContainerDlgs.AddTail(&dlg);
				// (a.walling 2012-05-01 15:31) - PLID 50117 - Tab numbers don't make sense anymore, now it is bShowPIC
				dlg.OpenPic(nPicID, true, -1, -1);
			}
		} catch (...) {
			delete &dlg;
			throw;
		}
	}
}

// (a.walling 2008-04-21 15:58) - PLID 29736 - Go directly to the NexEMR tab in the patients module.
void CMainFrame::OnNewEmr()
{
	// (a.walling 2008-04-21 15:58) - PLID 29736 - They just need a license to be able to view the NexEMR tab.
	if (!g_pLicense || g_pLicense->HasEMROrExpired(CLicense::cflrSilent) != 2) {
		ASSERT(FALSE);
	}
	else {
		OpenModule(PATIENT_MODULE_NAME);
		CNxTabView* pView;
		pView = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
		if (pView) {
			// (z.manning 2015-03-13 14:19) - NX-100432 - We now have a preference to potentially open the dashboard instead
			pView->SetActiveTab(GetPrimaryEmrTab());
		}
	}
}

// (c.haag 2010-07-15 17:23) - PLID 34338 - This function will allocate and return a new lab entry dialog
CLabEntryDlg* CMainFrame::AllocateLab(CWnd* pParentWnd)
{
	CLabEntryDlg* pDlg = new CLabEntryDlg(pParentWnd);
	m_lstpLabEntryDlgs.AddTail(pDlg); // Add it to the list of open labs
	return pDlg;
}


// (c.haag 2010-11-15 12:57) - PLID 41124 - This function will allocate a new VisionWeb order dialog
CVisionWebOrderDlg* CMainFrame::AllocateVisionWebOrderDlg(CWnd* pParentWnd)
{
	CVisionWebOrderDlg* pDlg = new CVisionWebOrderDlg(pParentWnd);
	m_lstpVisionWebOrderDlgs.AddTail(pDlg);
	return pDlg;
}

// (c.haag 2010-07-15 17:23) - PLID 34338 - This function will open this lab on the screen. You should use this instead of DoModal().
// (j.jones 2010-09-01 09:40) - PLID 40094 - added a location ID to be used on new labs that are created
// (b.spivey - January 20, 2014) - PLID 46370 - Added ResultID
int CMainFrame::OpenLab(CWnd* pParentWnd, long nPatientID, long nProcedureID, LabType ltType, long nInitialLabID, long nOrderSetID, const CString& strToBeOrdered, long nPicID, BOOL bNextLabWillOpenOnClose, BOOL bModal, HWND hwndNotify, long nNewLabLocationID /*= GetCurrentLocationID()*/, long nResultID /*= -1*/)
{
	// Try to open the existing lab first
	if (nInitialLabID > -1 && !m_lstpLabEntryDlgs.IsEmpty()) {

		// (c.haag 2010-07-28 11:59) - PLID 34338 - Get the form number. That's the unique identifier we really need to use. What
		// we won't do, though, is change the user's selection in the lab tree to fit the specific lab ID. We can always add that as a
		// feature in another item.
		CString strInitialLabFormNumber;
		_RecordsetPtr prs = CreateParamRecordset("SELECT FormNumberTextID FROM LabsT WHERE ID = {INT}", nInitialLabID);
		if (!prs->eof) {
			strInitialLabFormNumber = AdoFldString(prs, "FormNumberTextID");			
		}
		prs->Close();

		POSITION pos = m_lstpLabEntryDlgs.GetHeadPosition();
		// Traverse the PIC list looking for the one we want to open
		while (pos) {
			CLabEntryDlg *pDlg = m_lstpLabEntryDlgs.GetNext(pos);
			if (IsWindow(pDlg->GetSafeHwnd()) && pDlg->IsWindowVisible() &&
				pDlg->GetInitialLabID() == nInitialLabID ||
				pDlg->GetSavedFormNumber() == strInitialLabFormNumber) 
			{
				// We found it. Here are the possible combinations we need to handle:
				//
				// pDlg is modeless, bModal is false: Just bring it to the front.
				// pDlg is modal, bModal is false: Just bring it to the front (I don't see how this could ever happen 
				//	because the lab is already open modally)
				// pDlg is modeless, bModal is true: We can't run a modal loop on an already-open window. Bring the window
				//	to the front but fail this function call. Also inform the user that changes they make to the open lab will
				//	not reflect in whatever they're presently in.
				// pDlg is modal, bModal is true: Disallow because we can't run a modal loop on an already-open window
				//
				if (bModal) {
					if (IsWindowModal(pDlg->GetSafeHwnd())) {
						AfxMessageBox("This lab is already open in another window. You may not access it from here.", MB_ICONSTOP | MB_OK);
					}
					else {
						AfxMessageBox("This lab was opened from another area in Practice.\n\nThe lab will be brought to the front of your screen, "
							"but any changes you make to it will not be reflected in the module you are currently in.", MB_ICONWARNING, MB_OK);
						BringDialogToFront(pDlg);
					}
					return IDCANCEL;
				} else {
					BringDialogToFront(pDlg);
					// Break out of the loop and this function by telling our caller that it activated
					// an existing PIC window
					return IDOK;
				}
			}
		} // while (pos) {
	} // if (nInitialLabID > -1 && !m_lstpLabEntryDlgs.IsEmpty()) {
	
	// If we get here, there is no existing window for the lab and we need to make one.
	CLabEntryDlg &dlg = *AllocateLab(pParentWnd);
	try
	{
		// (b.spivey - January 20, 2014) - PLID 46370 - Added ResultID
		return dlg.OpenLab(nPatientID, nProcedureID, ltType, nInitialLabID, nOrderSetID, strToBeOrdered, nPicID, bNextLabWillOpenOnClose, bModal, hwndNotify, nNewLabLocationID, nResultID);
	}
	catch (...) 
	{
		delete &dlg;
		throw;
	}
}

// (c.haag 2010-07-15 17:23) - PLID 34338 - This will bring a lab to the front of the screen
// (c.haag 2010-11-15 12:57) - PLID 41124 - This is now universal
void CMainFrame::BringDialogToFront(CDialog* pDlg)
{
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	if (pDlg->GetWindowPlacement(&wp)) {
		if (pDlg->IsIconic()) {
			if (wp.flags & WPF_RESTORETOMAXIMIZED) {
				wp.showCmd = SW_MAXIMIZE;
			} else {
				wp.showCmd = SW_RESTORE;
			}
			pDlg->SetWindowPlacement(&wp);
		}
	}

	// Bring the window to the foreground so the user is sure to see it
	pDlg->SetForegroundWindow();
}

// (c.haag 2010-11-15 12:57) - PLID 41124 - This function will open a VisionWeb order dialog. You should use this instead of DoModal().
//TES 5/24/2011 - PLID 43737 - Added a parameter for the type of order
int CMainFrame::OpenVisionWebOrderDlg(CWnd* pParentWnd, long nOrderID, VisionWebOrderType vwot /*vwotSpectacleLens*/)
{
	// Try to open the existing order first
	if (nOrderID > -1 && !m_lstpVisionWebOrderDlgs.IsEmpty()) {

		POSITION pos = m_lstpVisionWebOrderDlgs.GetHeadPosition();
		// Traverse the list looking for the one we want to open
		while (pos) {
			CVisionWebOrderDlg *pDlg = m_lstpVisionWebOrderDlgs.GetNext(pos);
			if (IsWindow(pDlg->GetSafeHwnd()) && pDlg->IsWindowVisible() &&
				pDlg->m_nOrderID == nOrderID) 
			{
				// We found it. Here are the possible combinations we need to handle:
				//
				// pDlg is modeless: Just bring it to the front.
				// pDlg is modal: Just bring it to the front (I don't see how this could ever happen 
				//	because the order window should never be modal)
				//
				BringDialogToFront(pDlg);
				// Break out of the loop and this function by telling our caller that it activated
				// an existing window
				return IDOK;
			}
		} // while (pos) {
	} // if (nOrderID > -1 && !m_lstpVisionWebOrderDlgs.IsEmpty()) {
	
	// If we get here, there is no existing window for the order and we need to make one.
	CVisionWebOrderDlg &dlg = *AllocateVisionWebOrderDlg(pParentWnd);
	try
	{
		dlg.m_nOrderID = nOrderID;
		dlg.m_vwot = vwot;
		dlg.Create(CVisionWebOrderDlg::IDD);
		dlg.ModifyStyle(0, WS_MINIMIZEBOX);
		dlg.ShowWindow(SW_SHOW);
		return IDOK;
	}
	catch (...) 
	{
		delete &dlg;
		throw;
	}
}

LRESULT CMainFrame::OnPrintEMNReport(WPARAM wParam, LPARAM lParam)
{
	PrintEMNReport((long)wParam);
	return 0;
}

//TES 8/24/2004: These two functions collaborate with the menu bar to simulate menu behavior
void CMainFrame::OnDropDown(NMHDR *pNotifyStruct, LRESULT *lResult)
{
	NMTOOLBAR* pNMToolBar = ( NMTOOLBAR* )pNotifyStruct;


	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_MAINFRAME));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	
	CRect rc;
    ::SendMessage( pNMToolBar->hdr.hwndFrom, 
        TB_GETRECT, pNMToolBar->iItem, ( LPARAM )&rc );
    rc.top = rc.bottom;
    ::ClientToScreen( pNMToolBar->hdr.hwndFrom, &rc.TopLeft() );
    pPopup->TrackPopupMenu(TPM_RIGHTALIGN |
		TPM_RIGHTBUTTON, rc.left, rc.top, this);



	*lResult = TBDDRET_DEFAULT;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	/*if (m_wndMenuBar.TranslateFrameMessage(pMsg))
		return TRUE;*/

	return CNxMDIFrameWnd::PreTranslateMessage(pMsg);
}

void CMainFrame::PostCreate()
{
//	m_wndMenuBar.PostCreate();
	CleanupEMRTempFiles(FALSE);
}

void CMainFrame::OnCalcFinanceCharges()
{
	if(!CheckCurrentUserPermissions(bioFinanceCharges,sptWrite))
		return;

	// (j.jones 2007-02-20 10:06) - PLID 24293 - removed warning message
	// from this function, moved it to be inside CalculateFinanceCharges()

	CalculateFinanceCharges();
}

// (j.jones 2013-08-01 13:19) - PLID 53317 - added ability to undo finance charges
void CMainFrame::OnUndoFinanceCharges()
{
	try {

		//permissions are checked inside this function
		UndoFinanceCharges();

	}NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnConnectToTechSupport()
{
	CWaitCursor wc;
	ShellExecute(NULL, NULL, "http://www.nextech.com/nxrs/nxrswrap.exe", NULL, NULL, SW_SHOW);
}


void CMainFrame::OnConfigureReceipts() {
	// (a.wetta 2007-06-14 17:13) - PLID 25163 - Make sure they have the Billing license
	if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrUse)) {
		CPOSReceiptConfigDlg dlg(this);
		dlg.DoModal();
	}
	else {
		MsgBox("You must purchase the Billing License to use this feature.");
	}
}

void CMainFrame::OnReceiptPrinterSettings() 
{
	try {
		// (a.wetta 2007-06-14 17:13) - PLID 25163 - Make sure they have the Billing license
		if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrUse)) {

			if (!m_pPOSPrinterDevice) {
				m_pPOSPrinterDevice = new COPOSPrinterDevice(this);
			}

			if (!m_pPOSPrinterDevice) {
				ThrowNxException("Failed to create POS Printer object");
			}

			CPOSPrinterSettingsDlg dlg(m_pPOSPrinterDevice, this);
			dlg.DoModal(); 
			// (a.walling 2007-05-17 16:03) - Changed this to reference Printer rather than MSR
			// This dialog may have changed the pointer to the Printer device, so let's get the new pointer
			//m_pPOSPrinterDevice = dlg.GetOPOSPrinterDevice();
		}
		else {
			MsgBox("You must purchase the Billing License to use this feature.");
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2007-05-17 15:56) - PLID 26058 - Open the config dialog for the cash drawer
void CMainFrame::OnCashDrawerSettings()
{	
	// (a.wetta 2007-05-24 10:15) -  PLID 25960 - Make sure they have the NexSpa license
	if (IsSpa(TRUE)) {
		POSCashDrawerConfigDlg dlg(m_pPOSCashDrawerDevice, this);
		dlg.DoModal(); 
		// This dialog may have changed the pointer to the Cash Drawer device, so let's get the new pointer
		m_pPOSCashDrawerDevice = dlg.GetPOSCashDrawerDevice();
	}
	else {
		MsgBox("You must purchase the NexSpa License to use this feature.");
	}
}

void CMainFrame::ShowActiveEMR()
{
	//DRT 1/20/2005 - PLID 15340 - This should use a usage count!
	//Also fixed a bug in which this function was telling the user "You can't use this feature", 
	//	then opening the dialog anyways.
	if(g_pLicense->HasEMR(CLicense::cflrUse) != 2) {
		return;
	}

	if (!CheckCurrentUserPermissions(bioPatientEMR, sptRead)) {
		return;
	}

	// (a.walling 2006-09-19 12:50) - PLID 22577 - Fixed the way this was being displayed to prevent multiple instances
	if (m_pActiveEMRDlg.GetSafeHwnd() == NULL)
		m_pActiveEMRDlg.Create(IDD_ACTIVE_EMR_DLG,GetActiveWindow());

	m_pActiveEMRDlg.ShowWindow(SW_SHOWNORMAL);
}

void CMainFrame::ShowLockManager()
{
// (a.walling 2006-09-18 09:35) - PLID 18475 - Manages unlocked EMNs (Generally over 30 days old)
	if(g_pLicense->HasEMR(CLicense::cflrUse) != 2) {
		return;
	}
	
	// (r.goldschmidt 2014-07-15 18:17) - PLID 62649 - Make the EMR Lock Manager resizable and enable the maximize button
	if (m_pEMRLockManagerDlg.GetSafeHwnd() == NULL)
	{
		m_pEMRLockManagerDlg.Create(IDD_LOCKMANAGER,GetActiveWindow());
		m_pEMRLockManagerDlg.ShowWindow(SW_SHOW);
}
	else {
		if (m_pEMRLockManagerDlg.IsIconic()) {
			m_pEMRLockManagerDlg.ShowWindow(SW_RESTORE);
		}
	}
}

// (a.walling 2006-09-22 13:34) - PLID 19598 - show the EMR Search/review
// (r.goldschmidt 2014-07-14 16:21) - PLID 62648 - Enable Maximize button on the Patient's Seen dialog
void CMainFrame::ShowEMRSearch()
{
	// (j.jones 2006-12-21 12:53) - PLID 23938 - check permissions prior to
	// attempting to open the dialog
	if (!CheckCurrentUserPermissions(bioPatientEMR, sptRead)) {
		return;
	}
	
	if (m_pEMRSearchDlg.GetSafeHwnd() == NULL)
	{
		// (a.walling 2008-06-26 16:03) - PLID 30531 - This should be the mainframe, not GetActiveWindow
		// (a.walling 2008-08-12 13:03) - PLID 30531 - Reverted to GetActiveWindow for now
		m_pEMRSearchDlg.Create(IDD_EMR_SEARCH,GetActiveWindow());
		m_pEMRSearchDlg.ShowWindow(SW_SHOW);
		m_pEMRSearchDlg.SetSeenToday(false);
	}
	else {
		if (m_pEMRSearchDlg.IsIconic()) {
			m_pEMRSearchDlg.ShowWindow(SW_RESTORE);
		}
		m_pEMRSearchDlg.SetSeenToday(false);
	}
}

// (a.walling 2006-09-22 13:34) - PLID 18215 - show the patients seen today dialog!
// (r.goldschmidt 2014-07-14 16:21) - PLID 62648 - Enable Maximize button on the Patient's Seen dialog
void CMainFrame::ShowSeenToday()
{
	// (j.jones 2006-12-21 12:53) - PLID 23938 - check permissions prior to
	// attempting to open the dialog
	if (!CheckCurrentUserPermissions(bioPatientEMR, sptRead)) {
		return;
	}

	if (m_pEMRSearchDlg.GetSafeHwnd() == NULL)
	{
		m_pEMRSearchDlg.Create(IDD_EMR_SEARCH,GetActiveWindow());
		m_pEMRSearchDlg.ShowWindow(SW_SHOW);
		m_pEMRSearchDlg.SetSeenToday(true);
	}
	else {
		if (m_pEMRSearchDlg.IsIconic()) {
			m_pEMRSearchDlg.ShowWindow(SW_RESTORE);
		}
		m_pEMRSearchDlg.SetSeenToday(true);
	}
}

void CMainFrame::EditEmnTemplate(long nTemplateID)
{
	if(g_pLicense->HasEMR(CLicense::cflrUse) == 2 && CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {

		// (a.walling 2012-04-10 15:56) - PLID 48469 - Moved the NexWeb template check into LaunchWithTemplate (was copy/pasted two other places)

		// (a.walling 2012-02-29 06:50) - PLID 48469 - Handle new template editor
		CEmrTemplateFrameWnd::LaunchWithTemplate(nTemplateID);
	}
}

// (a.walling 2012-03-02 11:42) - PLID 48469
void CMainFrame::CreateNewEmnTemplate(long nCollectionID)
{
	if(g_pLicense->HasEMR(CLicense::cflrUse) == 2 && CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
		CEmrTemplateFrameWnd::CreateNewTemplate(nCollectionID);
	}
}

void CMainFrame::OnUpdateLicense()
{
	// (b.cardillo 2010-02-05 12:22) - PLID 16649 - permission for updating license
	if (CheckCurrentUserPermissions(bioLicenseEnterUpdateCode, sptView)) {
		g_pLicense->PromptForUpdateCode();
	}
}

void CMainFrame::OnDeactivateWorkstations()
{
	// (b.cardillo 2010-02-05 12:22) - PLID 16649 - permission for deactivate workstation
	if (CheckCurrentUserPermissions(bioLicenseDeactivateWorkstation, sptView)) {
		CDeactivateWorkstationsDlg dlg(this);
		dlg.DoModal();
	}
}

void CMainFrame::UpdateOpenModuleList(CNxView *pNewView)
{
	//All we need to do is try to remove it at all times,
	//and then re-add to the end. Simple!

	RemoveFromOpenModuleList(pNewView);	
	m_paryOpenModules.Add(pNewView);
}

void CMainFrame::RemoveFromOpenModuleList(CNxView *pView)
{
	//simply find the view and remove it
	BOOL bFound = FALSE;
	for(int i=0; i<m_paryOpenModules.GetSize() && !bFound; i++) {
		if(((CNxView*)m_paryOpenModules.GetAt(i)) == pView) {
			bFound = TRUE;
			m_paryOpenModules.RemoveAt(i);
		}
	}
}

void CMainFrame::TryCloseNthModule()
{
	try {

		if(m_bCloseNthModule) {

			if(m_nMaxOpenModules <= 0) {
				//should be impossible, the preferences should have never allowed it
				ASSERT(FALSE);
				return;
			}

			//we currently  don't allow m_nMaxOpenModules to be updated after Practice starts,
			//so this while loop should be unlikely to run more than once, but it's set to
			//handle running many times if it has to
			while(m_paryOpenModules.GetSize() > m_nMaxOpenModules) {
				//remove the top module
				PostMessage(NXM_EXPLICIT_DESTROY_VIEW, 0, (LPARAM)m_paryOpenModules.GetAt(0));
				m_paryOpenModules.RemoveAt(0);
			}
		}

	}NxCatchAll("Error in CMainFrame::TryCloseNthModule()");
}

LRESULT CMainFrame::OnForciblyClose(WPARAM wParam, LPARAM lParam)
{
	m_bForciblyClosing = true;
	OnClose();
	return 0;
}

LRESULT CMainFrame::OnRecheckLicense(WPARAM wParam, LPARAM lParam)
{
	if(g_pLicense->CheckForLicense((CLicense::ELicensedComponent)lParam, CLicense::cflrSilent)) {
		//Well, it failed the first time, but it seems to be working now.
		//Find the menu item we were checking, and re-enable it.
		GetMenu()->EnableMenuItem(wParam, MF_ENABLED);
	}
	else {
		//It was already disabled, and the CheckForLicense will have given a message this time, so we're done.
	}
	return 0;
}


LRESULT CMainFrame::OnPreviewPrinted(WPARAM wParam, LPARAM lParam)
{
	try {

		CChildFrame* p = GetActiveViewFrame();
		if(p) {

			// (j.jones 2012-08-14 14:56) - PLID 51063 - if an NxTabView, update the view,
			// because any prior attempts to update the view may not have succeeded while
			// our preview was open
			CView *pActiveView = p->GetActiveView();
			if(pActiveView && pActiveView->IsKindOf(RUNTIME_CLASS(CNxTabView))) {
				((CNxTabView*)pActiveView)->UpdateView();
			}

			if(p->IsOfType(PATIENT_MODULE_NAME)) {
				if(pActiveView) {
					pActiveView->PostMessage(NXM_PREVIEW_PRINTED);
				}
			}
		}

	}NxCatchAll(__FUNCTION__);

	
	return 0;
}

LRESULT CMainFrame::OnPreviewClosed(WPARAM wParam, LPARAM lParam)
{
	try {

		CChildFrame* p = GetActiveViewFrame();
		if(p) {

			// (j.jones 2012-08-08 10:17) - PLID 51063 - if an NxTabView, update the view,
			// because any prior attempts to update the view may not have succeeded while
			// our preview was open
			CView *pActiveView = p->GetActiveView();
			if(pActiveView && pActiveView->IsKindOf(RUNTIME_CLASS(CNxTabView))) {
				((CNxTabView*)pActiveView)->UpdateView();
			}

			if(p->IsOfType(PATIENT_MODULE_NAME)) {
				if(pActiveView) {
					pActiveView->PostMessage(NXM_PREVIEW_CLOSED);
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
		
	return 0;
}

void CMainFrame::ShowPatientEMNsToBeBilled()
{

	// (j.jones 2010-04-21 08:42) - PLID 38279 - do not allow opening
	// this screen if a bill is already open
	// (a.walling 2010-04-26 13:21) - PLID 38364 - Modeless now
	/*
	if (GetMainFrame()->IsBillingModuleOpen(true)) {
		return;
	}
	*/	
	
	if (m_EMNToBeBilledDlg.GetSafeHwnd() == NULL)
	{
		m_EMNToBeBilledDlg.Create(CEMNToBeBilledDlg::IDD,GetMainFrame());
		m_EMNToBeBilledDlg.CenterWindow();
		m_EMNToBeBilledDlg.ShowWindow(SW_SHOWNORMAL);
	}
	else {
		m_EMNToBeBilledDlg.ShowWindow(SW_SHOWNORMAL);
	}
}

NxSocketUtils::HCLIENT CMainFrame::GetNxServerSocket()
{
	return g_hNxServerSocket;
}

// (r.gonet 12/11/2012) - PLID 54117 - Handle the responses that come back from the async
// CHL7Client. This doesn't affect flow, it just displays a message box.
void CMainFrame::HandleHL7Response(CHL7Client *pClient, HL7ResponsePtr pResponse)
{
	if(pResponse == NULL) {
		return;
	}

	CString strGroupName = GetHL7GroupName(pResponse->nHL7GroupID);
	CString strDescription = GetHL7ExportMessageTypeDescription(pResponse->hemtMessageType);
	CString strTab;
	if(pResponse->hemtMessageType == hemtNewLab) {
		strTab = "Send Labs";
	} else {
		strTab = "HL7";
	}
	
	// We must account for a description beginning with any letter, so...
	CString strAn;
	if(strDescription.GetLength() > 0) {
		if(strDescription.Left(1).FindOneOf("aeiouAEIOU") != -1) {
			strAn = "n";
		}
	}

	if(pResponse->hmssSendStatus == hmssFailure_NotBatched) {
		//TES 6/7/2013 - PLID 55732 - Before reporting an error, check whether one has already been reported for this group
		if(pClient) {
			if(!pClient->ShouldReportResponseError(pResponse)) {
				return;
			}
		}
		MessageBox(FormatString(
			"An error was reported while exporting a%s %s HL7 message to HL7 Link '%s'.\r\n"
			"This %s has not been exported.\r\n"
			"\r\n"
			"%s",
			strAn, strDescription, strGroupName, strDescription, pResponse->strErrorMessage), "Error", MB_ICONERROR);
	} else if(pResponse->hmssSendStatus == hmssFailure_Batched) {
		//TES 6/7/2013 - PLID 55732 - Before reporting an error, check whether one has already been reported for this group
		if(pClient) {
			if(!pClient->ShouldReportResponseError(pResponse)) {
				return;
			}
		}
		MessageBox(FormatString(
			"An error was reported while exporting a%s %s HL7 message to HL7 Link '%s'.\r\n"
			"This %s has not been exported, but instead has been batched for a future export in the %s tab of the Links module.\r\n"
			"\r\n" 
			"%s",
			strAn, strDescription, strGroupName, strDescription, strTab, pResponse->strErrorMessage), "Error", MB_ICONERROR);

	} else if(pResponse->hmssSendStatus == hmssBatched 
		|| pResponse->hmssSendStatus == hmssSent 
		|| pResponse->hmssSendStatus == hmssSent_AckReceived) {
			// Great. We are finished with this message or we are waiting on an ACK.
			// (a.wilson 2013-06-11 15:12) - PLID 57117 - if this was a emnbill then we need to insert or update the record in data.
			if (pResponse->hemtMessageType == hemtNewEmnBill) {
				ExecuteParamSql("INSERT INTO EMNBillsSentToHL7T (EMNID) "
					"SELECT PracticeRecordID FROM HL7MessageLogT WHERE HL7MessageLogT.MessageID = {INT} "
					"AND HL7MessageLogT.PracticeRecordID NOT IN (SELECT EMNID FROM EMNBillsSentToHL7T) "
					, pResponse->nMessageID);
			}
	} else if(pResponse->hmssSendStatus == hmssSent_AckFailure) {
		//TES 6/7/2013 - PLID 55732 - Before reporting an error, check whether one has already been reported for this group
		if(pClient) {
			if(!pClient->ShouldReportResponseError(pResponse)) {
				return;
			}
		}
		MessageBox(FormatString(
			"Error while waiting for an acknowledgement from HL7 Link '%s' for a%s %s HL7 message.\r\n"
			"The %s may not have been properly exported through your HL7 Link.\r\n"
			"\r\n"
			"%s",
			strGroupName, strAn, strDescription, strDescription, pResponse->strErrorMessage), "Error", MB_ICONERROR);
	} else if(pResponse->hmssSendStatus == hmssPending) {
		// Don't care
	} else {
		// Unrecognized status.
		ASSERT(FALSE);
	}
}

LRESULT CMainFrame::OnEmrItemEntryDlgItemSaved(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2008-01-29 15:21) - PLID 14982 - Tell the function we are calling in response to a message
		PostEmrItemEntryDlgItemSaved((CEMNDetail*)wParam, TRUE);
	} NxCatchAll("Error in OnEmrItemEntryDlgItemSaved");

	return 0;
}

// (a.walling 2008-07-24 17:50) - PLID 30836 - Made this more generic by supporting
// any CWnd rather than a specific class
void CMainFrame::RegisterTwainDlgWaiting(CWnd* pWnd)
{
	LONG nCount;
	if (!m_mapTwainDlgsWaiting.Lookup(pWnd, nCount)) {
		nCount = 0;
	}
	if (nCount >= 0 && nCount < LONG_MAX) {
		m_mapTwainDlgsWaiting.SetAt(pWnd, nCount + 1);
	} else {
		ThrowNxException("CMainFrame::RegisterSelectImageDlgWaiting: Cannot register one given dialog more than LONG_MAX times!");
	}
}

// (a.walling 2008-07-24 17:50) - PLID 30836 - Made this more generic by supporting
// any CWnd rather than a specific class
void CMainFrame::UnregisterTwainDlgWaiting(CWnd* pWnd)
{
	LONG nCount;
	if (m_mapTwainDlgsWaiting.Lookup(pWnd, nCount)) {
		if (nCount > 1) {
			m_mapTwainDlgsWaiting.SetAt(pWnd, nCount - 1);
		} else {
			// No more references, so drop it altogether
			m_mapTwainDlgsWaiting.RemoveKey(pWnd);
		}
	} else {
		// Wasn't found to begin with.  This function shouldn't be called for something that wasn't registered already
		ThrowNxException("CMainFrame::UnregisterSelectImageDlgWaiting: Cannot unregister the specified dialog because it hasn't been registered!");
	}
}

LRESULT CMainFrame::OnDisplayErrorMessage(WPARAM wParam, LPARAM lParam)
{
	// (b.cardillo 2006-11-15 10:50) - This used to call an overloaded version of the 
	// ShowErrorMessage() function that just took a single string as the parameters.  But it 
	// didn't take into account any of the other parameters that the poster of this message 
	// might have intended to pass, such as help strings, message title, etc.  So since I 
	// was adding parameters and generally cleaning up the ShowErrorMessage() function for 
	// plid 15422 anyway, I also cleaned this up so it would be able to call it with a full 
	// set of parameters.  In order to implement that as effectively as possible, I changed 
	// the overloaded version of the function into a global handler for the 
	// NXM_DISPLAY_ERROR_MESSAGE message.  This way, the handler is implemented right along 
	// with the official ShowErrorMessage() function itself so it's clear in that code how 
	// the parameters are used, and we don't have to worry about it here.
	return Global_OnShowErrorMessage(wParam, lParam);
}

// (b.savon 2013-06-27 09:46) - PLID 57344 - Post the message
void CMainFrame::PopUpMessageBoxAsync(const CString &strMessage, const UINT messageBoxType)
{
	// This is Thread Safe.

	// Create the error info and load it with our parameters
	CPopUpMessageBoxInfo *nam = new CPopUpMessageBoxInfo;
	nam->strMessage = strMessage;
	nam->unMessageBoxType = messageBoxType;

	// Post the message to our main window's thread.  Pass in the error info we created
	// Since we're posting the message to the main thread, were handing off ownership to it.
	// The main thread must clean up after itself when its done handling the message.
	AfxGetMainWnd()->PostMessage(NXM_POP_UP_MESSAGEBOX, 0, (LPARAM)nam);
}

// (b.savon 2013-06-27 10:02) - PLID 57344 - Handle NXM_AFXMESSAGEBOX
LRESULT CMainFrame::OnPopUpMessageBox(WPARAM wParam, LPARAM lParam)
{
	try{
		// We know the lParam is a pointer to a NxAfxMessage, and we own it since we're handling this message
		shared_ptr<CPopUpMessageBoxInfo> nam(reinterpret_cast<CPopUpMessageBoxInfo*>(lParam));
		AfxMessageBox(nam->strMessage, nam->unMessageBoxType);

		return (LRESULT)0;

	}NxCatchAll(__FUNCTION__);

	return (LRESULT)-1;
}

void CMainFrame::FireTimeChanged()
{
	// (a.walling 2010-07-12 13:50) - PLID 39608 - Our offset from the server time is no longer reliable
	InvalidateRemoteServerTime();

	if(m_pRoomManagerDlg) {
		m_pRoomManagerDlg->FireTimeChanged();
	}

	// (j.jones 2007-02-06 15:22) - PLID 24493 - check all open EMRs
	{
		POSITION pos = m_lstpPicContainerDlgs.GetHeadPosition();
		while (pos) {
			CPicContainerDlg *pPicContainer = m_lstpPicContainerDlgs.GetNext(pos);
			if (pPicContainer->GetSafeHwnd()) {
				pPicContainer->FireTimeChanged();
			}
		}
	}
}

void CMainFrame::OnConfigureAAFPRSSurvey() {

	CConfigureAAFPRSSurveyDlg dlg(this);
	dlg.DoModal();


}

void CMainFrame::EnsureNotificationDlg()
{
	//TES 1/4/2007 - PLID 24119 - If m_pNotificationDlg is NULL, create it, and set it to topmost.
	if(!m_pNotificationDlg) {
		// (a.walling 2012-10-05 09:36) - PLID 53027 - Fix mainframe activation - (pass NULL as parent)
		m_pNotificationDlg = new CNotificationDlg(NULL);
		m_pNotificationDlg->Create(IDD_NOTIFICATION, CWnd::GetDesktopWindow());
		m_pNotificationDlg->SetWindowPos(&wndTopMost,0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
	}
}

// (a.walling 2007-05-15 14:39) - PLID 9801 - Return the cash drawer window (or NULL)
COPOSCashDrawerDevice* CMainFrame::GetCashDrawerDevice()
{
	return m_pPOSCashDrawerDevice;
}

// (a.wetta 2007-07-03 17:50) - PLID 26547 - Determine if the OPOS MSR device exists
BOOL CMainFrame::DoesOPOSMSRDeviceExist()
{
	try {
		if (m_pOPOSMSRThread && m_pOPOSMSRThread->m_pMainWnd->GetSafeHwnd() && m_bOPOSMSRDeviceInitComplete)
			return TRUE;

	}NxCatchAll("Error in CMainFrame::DoesOPOSMSRDeviceExist");
	// It doesn't exist
	return FALSE;
}

void CMainFrame::OnFileSwitchUser() // (a.walling 2007-05-04 12:07) - PLID 4850 - Switch users
{
	try {
		// (a.walling 2007-05-04 10:37) - PLID 4850 - Switch users! First call TryCloseAll, which will ask
		// the user if they want to close all their windows and exit. Will return FALSE if something is preventing
		// us from exiting/switching users.
		CClosingDlg dlgClosing(this);
		dlgClosing.SetClosingTitle("Switching users");
		if (TryCloseAll(&dlgClosing, TRUE)) {// calls CheckAllowExit
			HandleUserLogout(TRUE, &dlgClosing);

			dlgClosing.DestroyWindow();

			LoadTitleBarText("Please login");

			CLoginDlg dlg(this); // create our modal Login dialog.
			dlg.m_bRelogin = TRUE;  // we MUST tell it that we are switching users and not logging in initially.
			dlg.DoModal(); // cancelling will exit the application; see logindlg.cpp comments
		}
	} NxCatchAll("Error encountered while switching users!");
}

// (a.walling 2007-05-14 17:13) - PLID 4850 - Remember which module was open just before closing the program
void CMainFrame::SaveLastOpenModule()
{
	// Remember which module was open just before closing the program
	//Cannot assume this is a NxTabView, but may assume an NxView
	CChildFrame *p = GetActiveViewFrame();
	if (p) {
		CNxTabView *pView = (CNxTabView *)p->GetActiveView();
		CChildFrame *pFrame = GetActiveViewFrame();
		if (pView && 
			pFrame->IsOfType(SCHEDULER_MODULE_NAME) ||
			pFrame->IsOfType(PATIENT_MODULE_NAME) ||
			pFrame->IsOfType(INVENTORY_MODULE_NAME) ||
			pFrame->IsOfType(FINANCIAL_MODULE_NAME) ||
			pFrame->IsOfType(MARKET_MODULE_NAME) ||
			pFrame->IsOfType(CONTACT_MODULE_NAME) ||
			pFrame->IsOfType(REPORT_MODULE_NAME) ||
			pFrame->IsOfType(LETTER_MODULE_NAME) ||
			pFrame->IsOfType(ADMIN_MODULE_NAME) ||
			pFrame->IsOfType(SURGERY_CENTER_MODULE_NAME)
			|| pFrame->IsOfType(LINKS_MODULE_NAME)
			)
		{
			//this should eventually remember the tab too
			SetRemotePropertyText("DefModule", p->GetType(), 0, GetCurrentUserName());
		}
	}
}

// (a.walling 2007-07-24 12:48) - PLID 26787 - Cache common properties when logging in
void CMainFrame::CacheLoginProperties()
{
	// (j.jones 2006-04-14 08:52) - PLID 20138 - Load all common mainframe startup properties into the
	// NxPropManager cache
	// (b.cardillo 2006-07-18 09:02) - PLID 21509 - Consolidated two calls to cache different properties 
	// into a single call.  Notice each where clause is unchanged in itself, but the two are combined 
	// using an OR operation so both sets of records are returned.
	// (j.jones 2007-03-30 13:19) - PLID 24819 - added cached info. for finance charges
	// (a.walling 2007-06-14 16:32) - Any changes here should be both in OnCreate and HandleUserLogin
	// (a.walling 2007-07-24 13:50) - PLID 26787 - Moved to a single shared function,
	//		also added AutoGotoApptInFFAListSearch, MSR_UseDevice, POSCashDrawer_UseDevice, POSPrinter_UseDevice
	//		CCProcessingUsingPinPad	
	// (d.thompson 2009-03-19) - PLID 33585 - UseOHIP
	// (j.gruber 2010-01-11 13:16) - PLID 36646 - CallerIDLookupBehavior

	// (a.walling 2010-08-05 13:21) - PLID 18081 - Cache patient warning properties. Also use BulkCache for single recordset.
	// (j.gruber 2010-09-09 10:21) - PLID 40413 - Cache the new property
	// (a.walling 2010-10-04 09:47) - PLID 40573 - Cache zip code properties
	// (j.armen 2011-10-24 14:21) - PLID 46136 - GetPracPath is using ConfigRT
	// (j.gruber 2012-06-07 09:28) - PLID 48690 - cache if we are using the dashboard
	// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
	// than the license object.
	g_propManager.BulkCache("MainFrm-Login", propbitText | propbitNumber | propbitDateTime | propbitImage, 
		"("

		"(Username = '<None>' OR Username = '%s') AND ("

		// These are all number values
		"Name = 'DefaultPatientModuleSearch' OR "
		"Name = 'DefaultPatientModuleInclude' OR "
		"Name = 'CurrentPatient' OR "
		"Name = 'PatientBreadcrumbTrailMax' OR " // (a.walling 2010-05-21 10:12) - PLID 17768
		"Name = 'PatientBreadcrumbTrailPersist' OR "
		"Name = 'lastContact' OR "
		"Name = 'EnableNthModuleClosing' OR "
		"Name = 'MaxModulesAllowedOpen' OR "
		"Name = 'EnablePendingUpdate' OR "
		"Name = 'DefaultASCLicenseWarnUser' OR "
		"Name = 'DefaultASCLicenseWarnDayRange' OR "
		"Name = 'PopupProductivity' OR "
		"Name = 'InactivatePatientsUser' OR "
		"Name = 'ConfigRTCacheExpireMinutes' OR "
		"Name = 'LogTimePrompt' OR "
		"Name = 'AllowAlertPopup' OR "
		"Name = 'ActiveAppPWExpireMinutes' OR "
		"Name = 'RememberTODOColumns' OR "
		//"Name = 'LastFinanceChargeProcess' OR "	// (j.jones 2008-09-11 12:49) - 31335 - that's a date, not a number
		"Name = 'FCPromptApplyFinanceCharges' OR "
		"Name = 'FCPromptUserID' OR "
		"Name = 'AutoGotoApptInFFAListSearch' OR "
		// (j.jones 2007-08-09 16:35) - PLID 27036 - added ability to keep the current patient/contact when switching users
		"Name = 'SwitchUser_KeepCurrentPatientContact' OR "
		// (a.walling 2008-04-21 13:29) - PLID 29731 - Toolbar text height
		"Name = 'ToolbarTextHeight' OR "
		// (a.walling 2008-10-07 12:19) - PLID 31575 - Toolbar borders
		// (a.walling 2012-03-02 10:43) - PLID 48589 - No more toolbar borders
		//"Name = 'ToolbarBorders' OR "
		// (j.jones 2008-04-23 15:22) - PLID 29597 - added HL7ImportNotifyMe_Patients
		"Name = 'HL7ImportNotifyMe_Patients' OR "
		"Name = 'HL7ImportNotifyMe_Appointments' OR " // (z.manning 2010-06-29 17:31) - PLID 34572
		"Name = 'HL7ImportNotifyMe_PatientDocuments' OR " //TES 3/14/2013 - PLID 55226
		"Name = 'HL7ImportNotifyMe_OpticalPrescriptions' OR " //TES 11/5/2015 - PLID 66371
		// (a.walling 2008-05-08 13:40) - PLID 29963 - Gradients for toolbar etc
		"Name = 'DisplayGradients' OR "
		// (a.walling 2008-07-02 13:56) - PLID 29757
		"Name = 'ToolbarDemoPages' OR "
		// (d.thompson 2009-03-19) - PLID 33585
		"Name = 'UseOHIP' OR "
		// (a.walling 2009-07-10 09:29) - PLID 33644
		"Name = 'NxServerBroadcastStagger' OR "
		"Name = 'TableCheckerStaggerMin' OR "
		"Name = 'TableCheckerStaggerMax' OR "
		// (j.jones 2014-08-19 10:21) - PLID 63412 - the batch limit also has a timer now,
		// the count is now a minmum
		"Name = 'TableCheckerQueueBatchLimit' OR "
		"Name = 'TableCheckerQueueBatchTimeLimitMS' OR "
		// (c.haag 2009-08-06 15:27) - PLID 25943
		"Name = 'DisplayNoShowVoidSuperbillPrompt' OR "
		"Name = 'FirstDataBank_LastNDCValidation' OR "
		"Name = 'RelayAuditingToSyslog' OR " // (a.walling 2010-03-16 11:05) - PLID 36626 - Cache these properties here
		"Name = 'RelayAuditingToSyslog_IP' OR "
		"Name = 'DefaultWarningCategoryColor' OR "
		"Name = 'NewPatientOpenSecurityGroups' OR "
		//(e.lally 2011-08-26) PLID 44950 - cache more common preferences
		"Name = 'LabDisplayNeedAttn' OR "
		"Name = 'RemindForApptsWithoutAllocations' OR "
		"Name = 'EMRProblemPromptUserWarning' OR "
		"Name = 'NewCrop_ProductionStatusOverride' OR "
		"Name = 'IgnoreRightAlt' OR "
		"Name = 'PracticePromptOnClose' OR "
		"Name = 'ClassicYakIcon' OR "
		"Name = 'YakTheme' OR "
		"Name = 'ActiveViewPWExpireMinutes' OR "
		"Name = 'UpdatePendingUser' OR "
		"Name = 'HL7ExportLabResults' OR " // (d.singleton 2012-12-20 12:54) - PLID 53041 cashe values
		"Name = 'HL7ExportLockedEMNs' OR " // (d.singleton 2012-12-20 12:54) - PLID 53041 cashe values
		//"Name = 'ShowPatientDashboard' OR " // (a.wilson 2012-07-05 11:32) - PLID 51332 - only used for beta development of dashboard.		
		"Name = 'UMLSLogin' OR "// (j.camacho 2013-10-25 15:53) - PLID 58678
		"Name = 'UMLSPassword' OR " // (j.camacho 2013-10-25 15:53) - PLID 58678
		"Name = 'UMLSWebsite' OR " // (j.camacho 2013-10-30 17:23) - PLID 58678
		// (j.jones 2014-03-03 15:00) - PLID 61136 - added the diag PCS subpreference
		"Name = 'DiagGlobalSearchDisplayOption' OR "
		"Name = 'DiagGlobalSearchDisplayOption_IncludePCS' OR "		
		"Name = 'ApptNotesDefaultCategory' OR " // (j.gruber 2015-02-04 10:28) - PLID 64392 - Rescheduling Queue - In Tools > Preferences > Scheduling > Display 2, add a Global preference that reads “Default Appointment History Notes Category”. Inside the preference, there should be a drop down menu with a list of Categories. The default selection should be < No Category >.
		"Name = 'WordProcessorType' OR " // (z.manning 2016-03-03 10:30) - NX-100500
		"Name = 'NexEMRIconTabToOpen' OR " // (z.manning 2015-03-13 12:01) - NX-100432
		"Name = 'ICCPEnabled' OR "

		// (j.dinatale 2012-02-01 15:24) - PLID 45511 - this is now a non beta feature
		// (j.jones 2011-11-23 16:34) - PLID 44905 - cached our Beta flag for corrections
		//"Name = 'IsLineItemCorrectionsEnabled_Beta' OR "


		//these are all Image values
		"Name = 'YakIconDisabled' OR "
			
		
		// These are all date time values
		//"Name = 'LastFinanceChargeProcess' OR "	// (j.jones 2013-07-31 15:14) - PLID 53317 - obsolete
		"Name = 'LastPendingApptsUpdate' OR "
		"Name = 'LastProductivityPopup' OR "
		"Name = 'InactivatePatientsLastPopup' OR "
		"Name = 'LastApptsWithoutAllocationsPopup' OR "
		"Name = 'LastInactiveInsuredPartyProcess' OR "
		"Name = 'timestamp ShowChanges' OR "
		
		// These are all text values
		"Name = 'DefaultWarningCategoryName' OR "
		"Name = 'DefModule' OR "
		"Name = 'UnitedDataPath' OR "
		"Name = 'MirrorDataPath' OR "
		"Name = 'CardConnectApiBaseUrl' OR "

		// Zip code props
		"Name = 'InputZipCodeFile' OR " // text
		"Name = 'ZipCodes_CheckedStaticIDs' OR " // int

		// (j.jones 2010-10-25 17:01) - PLID 41068 - device plugin auto-popup
		// (j.jones 2011-03-10 15:50) - PLID 41349 - removed this preference
		//"Name = 'DeviceImport_AutoPopupWithChangedFiles' OR "

		// (j.jones 2010-11-03 15:08) - PLID 39620 - added Alberta option
		"Name = 'UseAlbertaHLINK' OR "
		"Name = 'Alberta_PatientULICustomField' OR "

		// (j.jones 2011-03-15 16:29) - PLID 42850 - added DeviceImport_NotifyOnStartup
		"Name = 'DeviceImport_NotifyOnStartup' OR "
		"Name = 'FreeBigDataListsDelay' OR " // (a.walling 2011-08-01 17:34) - PLID 44788 - After a certain amount of time, free the memory of hidden, big datalists
		"Name = 'Emr_UseAutoQuantumBatch' OR " // (a.walling 2014-02-06 09:26) - PLID 60547 - NxAutoQuantum - Check configrt global enable state

		// (j.jones 2013-01-09 13:25) - PLID 54530 - added reconciliation cache here,
		//as a lot of places in code can call it that didn't have this cached
		"Name = 'ReconcileNewRxWithCurMeds' OR "
		"Name = 'ReconcileNewRxWithCurMeds_SkipEMRTable' " // (j.jones 2013-01-11 13:07) - PLID 54462
		// (s.tullis 2016-01-28 17:46) - PLID 68090
		"OR Name = 'PrescriptionNeedingAttentionReminderSelection' "
		"OR Name = 'PrescriptionNeedingAttentionReminderValue' "
		// (s.tullis 2016-01-28 17:46) - PLID 68090
		"OR Name = 'RenewalsReminderSelection' "
		"OR Name = 'RenewalsReminderValue' "

		") "
		") OR ("

		//*************per workstation preferences*************

		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'lastDoc' OR "
		"Name = 'lastLoc' OR "
		"Name = 'lastCat' OR "
		"Name = 'UseBarcodeScanner' OR "	// (j.jones 2013-05-15 13:16) - PLID 25479 - this is obsolete now
		"Name = 'UseCallerID' OR "
		"Name = 'MirrorShowSoftware' OR "
		"Name = 'LastTimeOption' OR "
		"Name = 'LastTimeOption_User' OR "	// (j.dinatale 2012-10-23 09:32) - PLID 52393 - bulk cache the todo dialog user preferences
		"Name = 'DontRemind' OR "
		"Name = 'DontRemind_User' OR "	// (j.dinatale 2012-10-23 09:32) - PLID 52393 - bulk cache the todo dialog user preferences
		"Name = 'LastOtherTime' OR "
		"Name = 'LastOtherTime_User' OR "	// (j.dinatale 2012-10-23 09:32) - PLID 52393 - bulk cache the todo dialog user preferences
		"Name = 'CCProcessingUsingPinPad' OR "
		"Name = 'POSPrinter_UseDevice' OR "
		"Name = 'POSCashDrawer_UseDevice' OR "
		"Name = 'MSR_UseDevice' OR "
		"Name = 'ShowMirrorImagesInG1' OR " //TES 11/17/2009 - PLID 35709
		"Name = 'CallerIDLookupBehavior' OR "
		"Name = 'EMRImporterUseSharedPath' OR "
		"Name = 'ShowExportEmrContent' OR "
		"Name = 'TWAIN_UseTwain2' OR "
		"Name = 'OPOSPrinter_ClaimTimeout' OR " // (a.walling 2011-06-09 17:21) - PLID 42931
		//(e.lally 2011-08-26) PLID 44950 - cache more common preferences
		"Name = 'UnitedDisable' OR "
		"Name = 'MirrorDisable' OR "
		"Name = 'MirrorAllow61Link' OR "
		"Name = 'MirrorAllowAsyncOperations' OR "
		//"Name = 'HoldAdobeAcrobatReference' " // (a.walling 2011-06-27 14:40) - PLID 44260
		"Name = 'DontHoldAdobeAcrobatReference' " // (a.walling 2011-09-13 14:42) - PLID 45468
		//(a.wilson 2012-1-11) PLID 47517 - cache opos barcode scanner prefs
		// (j.jones 2013-05-15 13:16) - PLID 25479 - the barcode scanning prefs per machine are obsolete now
		"OR Name = 'UseOposBarcodeScanner' "	
		"OR Name = 'OposBarcodeScannerName' "
		"OR Name = 'OposBarcodeScannerTimeout' "
		"OR Name = 'BarcodeScannerPort' "
		"OR Name = 'BarcodeScannerParity' "
		"OR Name = 'BarcodeScannerStopBits' "
		"OR Name = 'DeviceImport_UseTrayIcon' "
		"OR Name = 'DeviceImport_OnlyUniquePaths' " // (a.walling 2014-01-22 16:16) - PLID 60438
		")"

		") OR ("

		//************* per-workstation preferences (no path) *************
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'IngenicoComPortPerMachine' OR "
		"Name = 'CardConnectIngenicoDevicePropsPerMachine' "
		")"

		") OR ("

		//************* per-workstation per-Windows user *************
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'IngenicoComPort' OR "
		"Name = 'CardConnectIngenicoDeviceProps' "
		")"

		") OR ("

		//  *****************per user/computer preferences ****************************
		// (j.jones 2010-04-23 09:41) - PLID 11596 - supported per user/computer preferences
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'TodoDoNotOpenOnLogin' OR "
		// (s.tullis 2015-10-21 17:43) - PLID 62087 - Option where To-Do does not pop up ever
		"Name = 'TodoDoNotOpenToRemind' OR "
		"Name = 'NxSettingsData' OR " // (a.walling 2012-06-04 16:30) - PLID 49829 - Gotta cache 'em all!
		// (a.wilson 2012-06-14 11:41) - PLID 47966 adding NotifyMeWhenPatientMarkedIn to cache
		"Name = 'NotifyMeWhenPatientMarkedIn' "
		// (j.jones 2013-05-15 13:16) - PLID 25479 - all barcode settings are now per-user, per-workstation
		"OR Name = 'UseBarcodeScanner_UserWS' "
		"OR Name = 'UseOposBarcodeScanner_UserWS' "
		"OR Name = 'OposBarcodeScannerTimeout_UserWS' "
		"OR Name = 'BarcodeScannerPort_UserWS' "
		"OR Name = 'BarcodeScannerParity_UserWS' "
		"OR Name = 'BarcodeScannerStopBits_UserWS' "
		"OR Name = 'OposBarcodeScannerName_UserWS' "
		") "
		")",
		_Q(GetCurrentUserName()),
		_Q(g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT)),
		_Q(g_propManager.GetSystemName()),
		_Q(GetAdvancedUsername()),
		_Q(GetCurrentUserComputerName()));
}

// (a.walling 2007-07-25 12:07) - PLID 26261 - Ensure all EMR temp files have been deleted, pending the move when bPend is true
void CMainFrame::CleanupEMRTempFiles(BOOL bPend)
{
	try {
		CFileFind ff;
		CString strTempPath = GetNxTempPath();

		BOOL bFileFound = ff.FindFile(strTempPath ^ "nexemrt_*.*");
		while (bFileFound) {
			bFileFound = ff.FindNextFile();
			if (!ff.IsDots() && !ff.IsDirectory()) {
				if (bPend)
					DeleteFileWhenPossible(ff.GetFilePath());
				else
					DeleteFile(ff.GetFilePath());
			}
		}
	} NxCatchAll("Error cleaning up temp files");
}

void CMainFrame::OnApptsWithoutAllocations()
{
	try {
		//TES 6/16/2008 - PLID 30394 - Confirm their license.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(g_pLicense->HasCandAModule(CLicense::cflrUse)) {
			// (e.lally 2009-06-08) PLID 34497 - Permission - Read Inventory Allocation tab
			if(CheckCurrentUserPermissions(bioInventoryAllocation, sptRead)) {
				//TES 6/16/2008 - PLID 30394 - Send FALSE for the bSilent parameter, if they selected the menu option, they will 
				// want to know why nothing comes up, if that happens.
				SendMessage(NXM_POPUP_APPTS_WO_ALLOCATIONS, (LPARAM)FALSE);
			}
		}
	}NxCatchAll("Error in CMainFrame::OnApptsWithoutAllocations()");
}

// (j.jones 2008-07-08 12:06) - PLID 24624 - added ability to run the patient summary
void CMainFrame::OnPatientSummary()
{
	try {
		// (e.lally 2009-06-08) PLID 34497 - Permissions - View Patients module (dlg checks individual component permissions)
		if(CheckCurrentUserPermissions(bioPatientsModule, sptView)) {
			//the patient summary dlg defaults to the active patient ID if we don't
			//specifically give it one, and in this case that is exactly what we want
			CPatientSummaryDlg dlg(this);
			dlg.DoModal();
		}
	}NxCatchAll("Error in CMainFrame::OnPatientSummary()");
}

LRESULT CMainFrame::OnPopupApptsWoAllocations(WPARAM wParam, LPARAM lParam)
{
	try {
	
		//TES 6/16/2008 - PLID 30394 - Is there anything to pop up?
		if(!InvUtils::DoAppointmentsWithoutAllocationsExist()) {
			//TES 6/16/2008 - PLID 30394 - Check the bSilent parameter.
			if(!(BOOL)wParam) {
				MsgBox("There are no upcoming appointments that require an allocation (or order), but do not have one.");
			}
			return 0;
		}
		else {
			//TES 6/16/2008 - PLID 30394 - There are some valid appointments, pop up the list.
			CApptsWithoutAllocationsDlg dlg(this);
			dlg.DoModal();
		}
	}NxCatchAll("Error in CMainFrame::OnPopupApptsWoAllocations()");
	return 0;
}

// (a.walling 2008-06-26 13:13) - PLID 30531 - Handler for editing a template or EMN
LRESULT CMainFrame::OnMsgEditEMROrTemplate(WPARAM wParam, LPARAM lParam)
{
	try {
		if (lParam == 0) {
			//template
			EditEmnTemplate(wParam);
		} else if (lParam == 1) {
			//emn
			long nEmnID = wParam;
			long nPicID = -1;

			ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT PicT.ID FROM EMRMasterT INNER JOIN EMRGroupsT ON EMRMasterT.EmrGroupID = EMRGroupsT.ID INNER JOIN PicT ON PicT.EmrGroupID = EMRGroupsT.ID WHERE EMRMasterT.ID = {INT}", nEmnID);

			if (!prs->eof) {
				nPicID = AdoFldLong(prs, "ID");
			}
			prs->Close();

			// (c.haag 2010-08-04 12:20) - PLID 39980 - This is acceptable now, albeit a very rare occurrence
			//if (nPicID == -1) {
			//	ThrowNxException("No PIC record associated with this EMN!");
			//}

			EditEmrRecord(nPicID, nEmnID);
		} else if (lParam == 2) {
			// (a.walling 2012-03-02 11:42) - PLID 48469

			CreateNewEmnTemplate((long)wParam);

		} else {
			ASSERT(FALSE);
		}

	} NxCatchAll("Error editing EMR / Template record!");

	return 0;
}

// (a.walling 2008-06-26 14:11) - PLID 30531 - Shared function to simply go to a patient in the patients module
// (a.walling 2010-08-18 16:28) - PLID 17768 - Flag to check for deleted patients
void CMainFrame::GotoPatient(long nPatID, bool bCheckForDeleted) {
	try {
		if (nPatID != -1) {
			//Set the active patient
			if(!m_patToolBar.DoesPatientExistInList(nPatID)) {
				// (a.walling 2010-08-18 16:29) - PLID 17768 - Check for deleted patient
				if (bCheckForDeleted && !ReturnsRecordsParam("SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE ID = {INT}", nPatID)) {
					MessageBox("This patient cannot be found in the system. The patient record may have been deleted or merged into another account.", NULL, MB_ICONSTOP);
					return;
				}

				if(IDNO == MessageBox("This patient is not in the current lookup. \n"
					"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					return;
				}
			}
			//TES 1/7/2010 - PLID 36761 - This function may fail now
			if(!m_patToolBar.TrySetActivePatientID(nPatID)) {
				return;
			}

			//Now just flip to the patient's module and set the active Patient
			FlipToModule(PATIENT_MODULE_NAME);
			CNxTabView *pView = GetActiveView();
			if(pView)
				pView->UpdateView();
		}//end if nPatientID
	}NxCatchAll("Error in CMainFrame::GotoPatient");
}

void CMainFrame::ShowEMRWritableReview()
{
	// (a.walling 2008-08-12 13:02) - PLID 30531 - Add exception handling
	try {
		// (j.jones 2006-12-21 12:53) - PLID 23938 - check permissions prior to
		// attempting to open the dialog
		if (!CheckCurrentUserPermissions(bioPatientEMR, sptRead)) {
			return;
		}
		
		if (m_EMRWritableReviewDlg.GetSafeHwnd() == NULL)
		{
			m_EMRWritableReviewDlg.Create(IDD_EMR_WRITABLE_REVIEW,this);
			m_EMRWritableReviewDlg.CenterWindow();
		}
		
		m_EMRWritableReviewDlg.ShowWindow(SW_SHOWNORMAL);
	} NxCatchAll("CMainFrame::ShowEMRWritableReview");
}

// (a.walling 2008-07-02 13:57) - PLID 29757 - Handle a click on the toolbar or get button info, show a demo page if necessary
// returns TRUE if button is not licensed
// (a.walling 2008-07-09 14:11) - PLID 30605 - Some updates to make this more modular for different types of practices
static long g_nToolbarDemoPages = -1; 
BOOL CMainFrame::HandleToolbarButtonMouseMessage(int nID, BOOL bDoAction)
{
#define HANDLE_TBCLICK(nViewFlag, strSection) if (!(m_licensedViews & nViewFlag)) { strUrl += strSection; bDisplay = TRUE; }
	try {
		if (g_nToolbarDemoPages == -1) {
			g_nToolbarDemoPages = GetRemotePropertyInt("ToolbarDemoPages", 1, 0, "<None>", true);
		}

		if (g_nToolbarDemoPages == 0) {
			return FALSE;
		}

		CString strUrl;
		BOOL bDisplay = FALSE;
		switch (nID) {
		case ID_PATIENTS_MODULE:
			{
				HANDLE_TBCLICK(TB_PATIENT, "Patients");
			} break;
		case ID_SCHEDULER_MODULE:
			{
				HANDLE_TBCLICK(TB_SCHEDULE, "Scheduler");
			} break;
		case ID_LETTER_WRITING_MODULE:
			{
				HANDLE_TBCLICK(TB_DOCUMENT, "LetterWriting");
			} break;
		case ID_MARKETING_MODULE:
			{
				HANDLE_TBCLICK(TB_MARKET, "Marketing");
			} break;
		case ID_INVENTORY_MODULE:
			{
				HANDLE_TBCLICK(TB_INVENTORY, "Inventory");
			} break;
		case ID_SURGERYCENTER_MODULE:
			{
				HANDLE_TBCLICK(TB_ASC, "ASC");
			} break;
		case ID_TB_NEW_EMR:	
			{
				HANDLE_TBCLICK(TB_EMR, "EMR");
			} break;
		// (d.thompson 2009-11-16) - PLID 36134
		case ID_LINKS_MODULE:	
			{
				HANDLE_TBCLICK(TB_LINKS, "Links");
			} break;

		default:
			return FALSE;
		};

		// 0 would mean this feature is disabled
		// 1 would mean it is enabled, but the type is undefined
		// 2 would mean 'Plastic'
		// 3 would mean 'Derm'
		if (g_nToolbarDemoPages > 0) {
			if (bDisplay && bDoAction) {
			// support a preference in case people start complaining and want it OUT!

				// this means they are clicking, so it is probably acceptible to do this. In most cases this will
				// just be a hash map lookup. However in case another user changed the preference, we want to be
				// sure that we have the most up to date value.
				g_nToolbarDemoPages = GetRemotePropertyInt("ToolbarDemoPages", 1, 0, "<None>", true);

				// (a.walling 2008-07-09 14:11) - PLID 30605 - We need to determine their type of practice
				if (g_nToolbarDemoPages == 1) {
					// need to figure out which one to display!
					CSelectDlg dlg(this);
					dlg.m_strTitle = "Select demo type";
					dlg.m_strCaption = "Several online demos are available; please choose the option that best applies to this practice.";
					dlg.m_strFromClause = "(SELECT 2 AS ID, 'Plastic / Cosmetic' AS Name UNION SELECT 3 AS ID, 'Dermatology' AS Name) PracticeTypesQ";

					dlg.AddColumn("ID", "ID", FALSE, FALSE);
					dlg.AddColumn("Name", "Type", TRUE, FALSE);

					if (IDOK == dlg.DoModal()) {
						ASSERT(dlg.m_arSelectedValues.GetSize() == 2);
						long nPracticeType = dlg.m_arSelectedValues[0];
						ASSERT(nPracticeType == 2 || nPracticeType == 3);
						g_nToolbarDemoPages = nPracticeType;
						SetRemotePropertyInt("ToolbarDemoPages", nPracticeType, 0, "<None>");
					} else {
						// cancelled, get out of here
						return FALSE;
					}
				}

				DWORD dwMask = 0xC0FFEE83;
				CString strSubDomain;
				switch (g_nToolbarDemoPages) {
				case 2:
					strSubDomain = "plastic";
					break;
				case 3:
					strSubDomain = "dermatology";
					break;
				default:
					return FALSE;
				}
				CString strTotalUrl = FormatString("http://%s.nextech.com/RegisterOnlineDemo.asp?key=%lu&view=%s", strSubDomain, ((DWORD)g_pLicense->GetLicenseKey() ^ dwMask), strUrl);
		
				SHELLEXECUTEINFO ExecInfo;
				memset(&ExecInfo, 0, sizeof(SHELLEXECUTEINFO));
				ExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
				ExecInfo.fMask = SEE_MASK_FLAG_NO_UI;
				// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
				ExecInfo.lpVerb = NULL;
				ExecInfo.lpFile = strTotalUrl;
				ExecInfo.nShow = SW_SHOWNORMAL;
				long nResult = ShellExecuteEx(&ExecInfo);

				// ignore nResult; we don't want them to have error messages.
			}
			
			return bDisplay ? TRUE : FALSE;
		} else {
			return FALSE;
		}

	} NxCatchAllCallIgnore({LogDetail("Error handling toolbar button message (%li)!", nID);};);

	return FALSE;
}

void CMainFrame::LoadScheduleHL7GroupIDs()
{
	if(!g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent)) {
		return;
	}

	m_arynScheduleHL7GroupIDs.RemoveAll();
	//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 Settings
	GetHL7SettingsGroupsBySetting("ExportAppts", TRUE, m_arynScheduleHL7GroupIDs);
}

void CMainFrame::EnsureInScheduleHL7GroupIDs(const long nID)
{
	for(int nIndex = 0; nIndex < m_arynScheduleHL7GroupIDs.GetSize(); nIndex++)
	{
		if(m_arynScheduleHL7GroupIDs.GetAt(nIndex) == nID) {
			// (z.manning 2008-07-16 15:35) - PLID 30490 - We already have this ID, so we're good
			return;
		}
	}

	m_arynScheduleHL7GroupIDs.Add(nID);
}

void CMainFrame::EnsureNotInScheduleHL7GroupIDs(const long nID)
{
	for(int nIndex = 0; nIndex < m_arynScheduleHL7GroupIDs.GetSize(); nIndex++)
	{
		if(m_arynScheduleHL7GroupIDs.GetAt(nIndex) == nID) {
			// (z.manning 2008-07-16 15:37) - PLID 30490 - We found this ID, so get rid of it.
			m_arynScheduleHL7GroupIDs.RemoveAt(nIndex);
			nIndex--;
		}
	}
}

// (a.walling 2008-07-16 17:18) - PLID 30751 - Access transcription parsing
void CMainFrame::OnParseTranscriptions()
{
	try {
		// (a.walling 2008-07-17 14:36) - PLID 30768 - exit if unlicensed
		// (e.lally 2009-06-08) PLID 34497 - Permission - history tab write.
		if (!g_pLicense->HasTranscriptions() || 
			!CheckCurrentUserPermissions(bioPatientHistory, sptWrite)){
			return;
		}

		if (m_pParseTranscriptionsDlg == NULL) {
			// (a.walling 2012-07-16 12:13) - PLID 46648 - Dialogs must set a parent
			m_pParseTranscriptionsDlg = new CParseTranscriptionsDlg(CWnd::GetDesktopWindow());
			m_pParseTranscriptionsDlg->Create(IDD_PARSE_TRANSCRIPTIONS_DLG, CWnd::GetDesktopWindow());
			m_pParseTranscriptionsDlg->CenterWindow();
		}

		m_pParseTranscriptionsDlg->ShowWindow(SW_SHOW);
	} NxCatchAll("Error in OnParseTranscriptions");
}

void CMainFrame::OnReprintSuperbills()
{
	try {
		// (e.lally 2009-06-08) PLID 34498 - Check permissions - View scheduler module
		if(CheckCurrentUserPermissions(bioSchedulerModule, sptView)) {
			//TES 9/3/2008 - PLID 31222 - Added a menu option to Reprint Past Superbills, we just need to launch the dialog.
			CPastSuperbillsDlg dlg(this);
			dlg.DoModal();
		}
	}NxCatchAll("Error in CMainFrame::OnReprintSuperbills()");
}

// (a.walling 2008-09-10 13:32) - PLID 31334 - WIA event recieved, forward to PIC or PatientView
// (a.walling 2009-06-22 10:48) - PLID 34635 - Dead/useless code
/*
LRESULT CMainFrame::OnWIAEvent(WPARAM wParam, LPARAM lParam)
{
	try {
		if (m_pLastActivePIC != NULL && ::IsWindow(m_pLastActivePIC->GetSafeHwnd())) {
			return m_pLastActivePIC->SendMessage(NXM_WIA_EVENT, wParam, lParam);
		} else {
			//OpenModule(PATIENT_MODULE_NAME);
			CNxTabView* pView;
			pView = (CNxTabView *)GetOpenView(PATIENT_MODULE_NAME);
			if (pView) {
				return pView->SendMessage(NXM_WIA_EVENT, wParam, lParam);
			}
		}
	}NxCatchAll("CMainFrame::OnWIAEvent");

	return (LRESULT)-1;
}
*/

// (a.walling 2008-09-18 10:24) - PLID 26781 - Generic function to broadcast message to PIC windows
void CMainFrame::BroadcastPostMessageToPICs(UINT msg, WPARAM wParam, LPARAM lParam, CWnd* pExclude)
{
	POSITION pos = m_lstpPicContainerDlgs.GetHeadPosition();

	while (pos) {
		CPicContainerDlg* pPic = m_lstpPicContainerDlgs.GetNext(pos);

		if (pExclude == NULL || (pExclude != pPic && !pExclude->IsChild(pPic)) ) {
			pPic->PostMessage(msg, wParam, lParam);
		}
	}
}

// (r.galicki 2008-10-13 16:06) - PLID 31373 - Display Labs Needing attention dialog
BOOL CMainFrame::ShowLabFollowUp(BOOL bShowAlways) {
	try {
		// (r.galicki 2008-10-15 14:42) - PLID 31373 - Relocated license/perms/pref checks
		if(g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)) { //license?
			if(GetCurrentUserPermissions(bioPatientLabs) & (sptRead|sptReadWithPass)) {	//permission?
				if(bShowAlways || (GetRemotePropertyInt("LabDisplayNeedAttn", 1, 0, GetCurrentUserName()) && HasLabsNeedingAttention())) { 
					//if not always shown, check preference, and do not open window if no follow-ups exist
					
					// (r.galicki 2008-11-11 09:32) - PLID 31373 - Design: if they have read w/ pass permissions, don't open the dialog upon login
					if(bShowAlways) {
						if(!CheckCurrentUserPermissions(bioPatientLabs, sptRead)) {
							return FALSE;
						}
					}
					else {
						//if they don't have read permissions, must be read w/ pass only, return false
						if(!(GetCurrentUserPermissions(bioPatientLabs) & sptRead)) {
							return FALSE;
						}
					}

					if (!m_pLabFollowupDlg) {
						// (a.walling 2012-07-16 12:15) - PLID 46648 - Dialogs must set a parent
						m_pLabFollowupDlg = new CLabFollowUpDlg(CWnd::GetDesktopWindow());
						m_pLabFollowupDlg->Create(IDD_LAB_FOLLOW_UP_DLG, CWnd::GetDesktopWindow());
						if(!m_pLabFollowupDlg) { //may have been cancelled in OnInitDialog
							return FALSE;
						}
					}
					// (j.armen 2012-06-05 16:04 ) - PLID 50805 - No longer need to center as dlg will remember size and position
					// (z.manning 2008-11-07 12:24) - PLID 31955 - I changed the show cmd to SW_RESTORE
					// so that this also works when the dialog is minimized.
					m_pLabFollowupDlg->ShowWindow(SW_RESTORE);
					m_pLabFollowupDlg->BringWindowToTop();
					return TRUE;
				}
			}
		}
		return FALSE;
	} NxCatchAll("Error in CMainFrame::ShowLabFollowUp");

	return FALSE;
}
// (r.galicki 2008-10-15 15:52) - PLID 31373 - Determine if there are labs needing followup
BOOL CMainFrame::HasLabsNeedingAttention() 
{
	// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam (although not really necessary, this could take advantage of cached queries when that is enabled)
	// (c.haag 2010-12-13 12:10) - PLID 41806 - Use GetLabCompletedDate. It will return a valid date if the
	// lab has at least one result, and if every lab result is completed.
	//TES 8/5/2011 - PLID 44901 - Filter on permissioned locations
	return ReturnsRecordsParam("SELECT TOP 1 ID from LabsT WHERE LabsT.Deleted = 0 "
		"AND (LabsT.ID NOT IN (SELECT LabID FROM LabResultsT WHERE Deleted = 0) OR LabsT.ID IN (SELECT LabID FROM LabResultsT WHERE Deleted = 0 AND ResultCompletedDate IS NULL))"
		" AND {SQLFRAGMENT}", GetAllowedLocationClause_Param("LabsT.LocationID"));
}

// (j.fouts 2013-02-28 14:24) - PLID 54429 - Returns the number of interactions now, and -1 on failure
// (j.fouts 2012-09-06 09:01) - PLID 52482 - Needed to make this dlg modeless
// (j.jones 2012-09-26 14:09) - PLID 52872 - added patient ID
// (j.jones 2012-11-30 11:09) - PLID 53194 - Accurately renamed bForceShow to be bForceShowEvenIfBlank,
// which shows the dialog even if there are no interactions, and added bForceShowEvenIfUnchanged, which
// will show the dialog even if its current interactions have not changed since the last popup.
long CMainFrame::ShowDrugInteractions(long nPatientID, bool bRequery /*= true*/,
									  bool bForceShowEvenIfBlank /*= false*/, bool bForceShowEvenIfUnchanged /*= false*/)
{
	try
	{
		// (j.fouts 2013-05-30 10:25) - PLID 56807 - Drug Interactions is tied to NexERx
		if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts)
		{
			// (b.savon 2012-11-30 10:30) - PLID 53773 - Consolidate
			if(!IsDrugInteractionDlgCreated()){
				return -1;
			}

			// (j.jones 2012-09-26 14:12) - PLID 52872 - We now pass in the patient ID.
			m_pDrugInteractionDlg->ShowOnInteraction(nPatientID, bRequery, bForceShowEvenIfBlank, bForceShowEvenIfUnchanged);
			return m_pDrugInteractionDlg->InteractionCount();
		}
		return 0/*They don't have the license report 0 interactions.*/;
	}
	NxCatchAll(__FUNCTION__);
	return -1;
}

// (j.fouts 2013-02-28 14:24) - PLID 54429 - Returns the number of interactions now, and -1 on failure
// (j.fouts 2012-11-14 11:42) - PLID 53573 - Created a ShowOnInteractions to show without calling the API, this will use the arrays
// of interactions that are passed
// (j.jones 2012-11-30 11:09) - PLID 53194 - Accurately renamed bForceShow to be bForceShowEvenIfBlank,
// which shows the dialog even if there are no interactions, and added bForceShowEvenIfUnchanged, which
// will show the dialog even if its current interactions have not changed since the last popup.
long CMainFrame::ShowDrugInteractions(Nx::SafeArray<IUnknown*> saryDrugDrugInteracts,
									  Nx::SafeArray<IUnknown*> saryDrugAllergyInteracts,
									  Nx::SafeArray<IUnknown*> saryDrugDiagnosisInteracts,
									  long nPatientID,
									  bool bForceShowEvenIfBlank /*= false*/, bool bForceShowEvenIfUnchanged /*= false*/)
{
	try
	{
		// (j.fouts 2013-05-30 10:25) - PLID 56807 - Drug Interactions is tied to NexERx
		if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts)
		{
			// (b.savon 2012-11-30 10:30) - PLID 53773 - Consolidate
			if(!IsDrugInteractionDlgCreated()){
				return -1;
			}

			m_pDrugInteractionDlg->ShowOnInteraction(saryDrugDrugInteracts, saryDrugAllergyInteracts, saryDrugDiagnosisInteracts,
				nPatientID, bForceShowEvenIfBlank, bForceShowEvenIfUnchanged);
			return m_pDrugInteractionDlg->InteractionCount();
		}
		return 0/*They don't have the license report 0 interactions.*/;
	}
	NxCatchAll(__FUNCTION__);
	return -1;
}

// (j.fouts 2013-09-17 16:53) - PLID 58496 - Made this accessable
long CMainFrame::GetDrugInteractionsFilteredCount()
{
	try
	{
		// (j.fouts 2013-05-30 10:25) - PLID 56807 - Drug Interactions is tied to NexERx
		if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts)
		{
			// (b.savon 2012-11-30 10:30) - PLID 53773 - Consolidate
			if(!IsDrugInteractionDlgCreated()){
				return -1;
			}

			return m_pDrugInteractionDlg->GetFilteredInteractionCount();
		}
		return 0/*They don't have the license report 0 interactions.*/;
	}
	NxCatchAll(__FUNCTION__);
	return -1;
}

// (b.savon 2012-11-30 09:17) - PLID 53773 - Consolidate
bool CMainFrame::IsDrugInteractionDlgCreated()
{
	try{
		if(!m_pDrugInteractionDlg)
		{
			m_pDrugInteractionDlg = new CDrugInteractionDlg(this, eipMedications);
			m_pDrugInteractionDlg->Create(IDD_DRUG_INTERACTION_DLG, this);
			if(!m_pDrugInteractionDlg) { //may have been cancelled in OnInitDialog
				return false;
			}
		}

		return true;

	}NxCatchAll(__FUNCTION__);

	return false;
}

// (j.armen 2012-02-27 12:24) - PLID 48303
bool CMainFrame::ShowRecallsNeedingAttention(const bool &bUsePatientID /*= false*/)
{
	try {
		// (j.armen 2012-03-28 11:06) - PLID 48480 - Use the license to show the recalls needing atten
		if(!g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrUse)) {
			return false;
		}

		//(a.wilson 2012-3-23) PLID 48472 - checking whether the current user has read permission.
		BOOL bReadPerm = (GetCurrentUserPermissions(bioRecallSystem) & (sptRead));
		BOOL bReadPermWithPass = (GetCurrentUserPermissions(bioRecallSystem) & (sptReadWithPass));

		if (!bReadPerm && !bReadPermWithPass) {
			PermissionsFailedMessageBox();
			return false;
		} else if (!bReadPerm && bReadPermWithPass) {
			if (!CheckCurrentUserPassword()) {
				return false;
			}
		}

		if (!m_pRecallsNeedingAttentionDlg) {
			m_pRecallsNeedingAttentionDlg = new CRecallsNeedingAttentionDlg(this);
			m_pRecallsNeedingAttentionDlg->Create(IDD_RECALLS_NEEDING_ATTENTION_DLG, this);
			if(!m_pRecallsNeedingAttentionDlg) { //may have been cancelled in OnInitDialog
				return false;
			}
		}

		// (j.armen 2012-06-05 16:04 ) - PLID 50805 - No longer need to center as dlg will remember size and position
		// (a.walling 2013-11-22 16:05) - PLID 60008 - Don't call DoFilter if we are already visible unless the patientid setting changed.
		bool needsRefresh = (!m_pRecallsNeedingAttentionDlg->IsWindowVisible() || (!!m_pRecallsNeedingAttentionDlg->m_bUsePatientID != !!bUsePatientID) );
		m_pRecallsNeedingAttentionDlg->m_bUsePatientID = bUsePatientID;
		if (needsRefresh) {
			m_pRecallsNeedingAttentionDlg->DoFilter();
		}
		m_pRecallsNeedingAttentionDlg->ShowWindow(SW_RESTORE);
		m_pRecallsNeedingAttentionDlg->BringWindowToTop();
		return true;
	}NxCatchAll(__FUNCTION__);
	return false;
}

// (z.manning 2015-11-05 12:10) - PLID 57109
void CMainFrame::HandleRecallChanged()
{
	// (z.manning 2015-11-05 13:43) - PLID 57109 - If the recalls needing attention dialog has
	// been created then make sure to refresh it.
	if (m_pRecallsNeedingAttentionDlg != NULL)
	{
		m_pRecallsNeedingAttentionDlg->SetNeedsRefresh();
	}
}

// (j.jones 2009-02-27 12:21) - PLID 33265 - added e-prescribing settings
//TES 4/10/2009 - PLID 33889 - Broke this function down into different functions for each type.
void CMainFrame::OnNewCropSettings()
{
	try {

		//require admin. access for this
		if(!IsCurrentUserAdministrator()) {
			AfxMessageBox("You do not have permission to access this setup.\n"
				"You must be an Administrator to change E-Prescribing settings.");
			return;
		}

		//TES 4/7/2009 - PLID 33882 - Changed the HasEPrescribing function to return an enum.
		CLicense::EPrescribingType ept = g_pLicense->HasEPrescribing(CLicense::cflrUse);
		if(ept == CLicense::eptNewCrop) {
			CNewCropSetupDlg dlg(this);
			dlg.DoModal();
		}
	}NxCatchAll("Error in CMainFrame::OnNewCropSettings");
}

// (j.gruber 2009-05-15 10:26) - PLID 28541 - view renewal requests
void CMainFrame::OnNewCropRenewalRequests() 
{
	try {
		// (e.lally 2009-06-10) PLID 34529 - Check permissions - Read patient medication
		CLicense::EPrescribingType ept = g_pLicense->HasEPrescribing(CLicense::cflrUse);
		if(ept == CLicense::eptNewCrop && CheckCurrentUserPermissions(bioPatientMedication, sptRead)) {
			CPrescriptionRenewalRequestDlg dlg(this);
			dlg.DoModal();
		}

	}NxCatchAll("Error in OnNewCropRenewalRequests()");
}

//TES 4/10/2009 - PLID 33889 - Split out from OnEPrescribingSettings()
void CMainFrame::OnSureScriptsSettings()
{
	try {

		//TES 7/6/2009 - PLID 34087 - This is handled inside the dialog now.
		/*//require admin. access for this
		if(!IsCurrentUserAdministrator()) {
			AfxMessageBox("You do not have permission to access this setup.\n"
				"You must be an Administrator to change E-Prescribing settings.");
			return;
		}*/

		//TES 4/7/2009 - PLID 33882 - Changed the HasEPrescribing function to return an enum.
		CLicense::EPrescribingType ept = g_pLicense->HasEPrescribing(CLicense::cflrUse);
		if(ept == CLicense::eptSureScripts && IsCurrentUserAdministrator()) {
			// (b.savon 2013-01-11 09:59) - PLID 54578 - Create NexERx Provider/User Role Setup dlg
			CNexERxSetupDlg dlg(this);
			dlg.DoModal();
		}
	}NxCatchAll("Error in CMainFrame::OnSureScriptsSettings");
}

void CMainFrame::OnRegisterNexERxClientAndPrescribers()
{
	try{
		// (b.savon 2012-11-01 12:09) - PLID 53559
		CLicense::EPrescribingType ept = g_pLicense->HasEPrescribing(CLicense::cflrSilent);
		if(ept == CLicense::eptSureScripts && IsCurrentUserAdministrator()){
			DisplayRegisterNexERx();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2009-03-02 14:40) - PLID 33053 - added ability to open the NewCrop web browser from anywhere
// (j.gruber 2009-03-30 11:30) - PLID 33736 - added nEMNID and nPatientID
// (j.gruber 2009-03-30 17:23) - PLID 33728 - need window to pass the closing message to
// (j.gruber 2009-05-15 17:11) - PLID 28541 - added strXML for renewal responses
// (j.gruber 2009-06-08 12:00) - PLID 34515 - added user role
// (j.jones 2013-01-04 08:52) - PLID 49624 - removed role, and added UserDefinedID
void CMainFrame::OpenNewCropBrowser(CString strDefaultWindowText, NewCropActionType ncatDefaultAction, long nActionID, long nPatientPersonID, long nPatientUserDefinedID, long nEMNID, CWnd *pWndToSendClosingMsgTo, OPTIONAL IN CString strXML)
{
	try {

		// (j.jones 2009-08-13 14:14) - PLID 35213 - added a mutex for NewCrop patient account access,
		// this applies to the NewCrop browser actions only (such as ncatAccessPatientAccount and
		// ncatProcessRenewalRequest), not any follow-up SOAP calls we may be attempting
		if(GetIsBrowsingNewCropPatientAccount()) {
			//we actually don't believe this is even possible,
			//so out of curiosity, we'd like to know if it can happen,
			//and if so, how can it happen
			ASSERT(FALSE);

			//since we don't believe this is possible, we don't need to warn,
			//just silently return
			return;
		}

		SetIsBrowsingNewCropPatientAccount(TRUE);

		if (m_pNewCropBrowserDlg == NULL) {
			// (a.walling 2012-05-25 13:11) - PLID 46648 - Dialogs must set a parent!
			// (j.jones 2013-01-04 08:52) - PLID 49624 - removed role, and added UserDefinedID
			m_pNewCropBrowserDlg = new CNewCropBrowserDlg(nPatientPersonID, nPatientUserDefinedID, nEMNID, pWndToSendClosingMsgTo, strXML, pWndToSendClosingMsgTo);

			m_pNewCropBrowserDlg->m_ncatDefaultAction = ncatDefaultAction;
			m_pNewCropBrowserDlg->m_nActionID = nActionID;
			m_pNewCropBrowserDlg->m_strDefaultWindowText = strDefaultWindowText;
			m_pNewCropBrowserDlg->m_bIsPopupWindow = FALSE;

			// (a.walling 2012-05-25 13:11) - PLID 46648 - Dialogs must set a parent!
			m_pNewCropBrowserDlg->Create(IDD_NEWCROP_BROWSER_DLG, pWndToSendClosingMsgTo);
		}
		else {
			//the NewCrop browser should have forced itself to be modal,
			//and self deleted upon closing and resuming MainFrm control,
			//therefore it should be impossible to call this function
			//while the browser still exists

			// (j.jones 2009-08-13 14:36) - PLID 35213 - it should be doubly
			// impossible now with our mutex, but even so, if we were to get here,
			// the correct thing to do is assert and merely show the active window
			ASSERT(FALSE);
		}

		m_pNewCropBrowserDlg->ShowWindow(SW_SHOW);

	}NxCatchAll("Error in CMainFrame::OpenNewCropBrowser");
}

// (j.fouts 2013-01-28 09:02) - PLID 54472 - Refresh NexERx messages needing attn.
// (s.tullis 2016-01-28 17:46) - PLID 68090
// (s.tullis 2016-01-28 17:46) - PLID 67965
// added login flag
void CMainFrame::RefreshNexERxNeedingAttention(BOOL bLogin /* = FALSE */)
{
	try
	{
		if(!SureScripts::IsEnabled())
		{
			return;
		}

		m_bNexERxNeedingAttentionLoaded = true;

		if (!GetRemotePropertyInt("SureScriptsNotify", 1, 0, GetCurrentUserName())) {
			//they do not need to be notified
			return;
		}
		
		struct NexERxRenewalNeedingAttention {
			long nPrescriberID;
			CString nRequestMessageGUID;
			COleDateTime dtSentTime;
		};

		struct NexERxPrescriptionAttention {
			long nPrescriberID;
			long nPrescriptionID;
			COleDateTime dtPrescriptionDate;
		};

		CArray<NexERxRenewalNeedingAttention,NexERxRenewalNeedingAttention&> arryRenewalsNeedingAttention;
		CArray<NexERxPrescriptionAttention,NexERxPrescriptionAttention&> arryPrescriptionsNeedingAttention;
		
		// (j.jones 2016-02-04 09:20) - PLID 68159 - The logic didn't change here, but was never documented well:
		// the user is notified for prescriptions and renewals for all providers they are linked to in the eRx Setup.
		// Their remembered provider in the popup dialogs has no effect here. As a result, they may get a higher
		// number in the notification popup than they actually see by default in the dialog. This is ok.
		boost::container::flat_set<long> aryProvidersToNotify;

		// (j.jones 2016-02-03 14:44) - PLID 68159 - for renewals, we need to only count the renewals that
		// that are in the dialog's <Renewals Requiring Action> filter, which uses the same enum type as prescriptions, but a different list
		// (j.jones 2016-02-03 14:44) - PLID 68159 - for prescriptions needing attention, we need to only count the prescriptions
		// that are in the dialog's < Prescriptions Requiring Action > filter, and within the dialog's default date filter of one week
		_RecordsetPtr prs = CreateParamRecordset(
			//renewals needing attention
			"SELECT RenewalRequestsT.PrescriberID, RenewalRequestsT.RequestMessageID, RenewalRequestsT.ReceivedDate "
			"FROM RenewalRequestsT "
			"LEFT JOIN RenewalResponsesT ON RenewalRequestsT.ID = RenewalResponsesT.RequestID "
			"WHERE (RenewalResponsesT.ID IS NULL OR RenewalResponsesT.QueueStatus IN ({CONST_INT}, {CONST_INT}, {CONST_INT})) \r\n"
			""
			//prescriptions needing attention
			"SELECT PatientMedications.ProviderID, PatientMedications.ID, PatientMedications.PrescriptionDate "
			"FROM PatientMedications "
			"WHERE PatientMedications.QueueStatus IN ({CONST_INT}, {CONST_INT}, {CONST_INT}, {CONST_INT}, {CONST_INT}) "
			"AND CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, PatientMedications.PrescriptionDate))) >= DateAdd(day, -7, CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, GetDate())))) "
			"AND CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, PatientMedications.PrescriptionDate))) <= CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, GetDate()))) "
			"AND PatientMedications.Deleted = 0 \r\n"
			""
			//get all the providers this user is linked to for eRx purposes
			"DECLARE @UserID INT "
			"SET @UserID = {INT} "
			"SELECT NexERxProviderID AS ProviderID " 
			"FROM UsersT WHERE PersonID = @UserID AND NexERxProviderID IS NOT NULL "
			"UNION "
			"SELECT NurseStaffPrescriberID AS ProviderID FROM NexERxNurseStaffPrescriberT WHERE UserID = @UserID "
			"UNION "
			"SELECT SupervisingPersonID AS ProviderID FROM NexERxSupervisingProviderT WHERE UserID = @UserID",
			(long)PrescriptionQueueStatus::pqsIncomplete, (long)PrescriptionQueueStatus::pqseTransmitAuthorized, (long)PrescriptionQueueStatus::pqseTransmitError,
			(long)PrescriptionQueueStatus::pqsIncomplete, (long)PrescriptionQueueStatus::pqsOnHold, (long)PrescriptionQueueStatus::pqseTransmitAuthorized, (long)PrescriptionQueueStatus::pqseTransmitError, (long)PrescriptionQueueStatus::pqseReadyForDoctorReview,
			GetCurrentUserID());
		while(!prs->eof)
		{
			NexERxRenewalNeedingAttention renewalNeedingAttn;
			renewalNeedingAttn.nPrescriberID = AdoFldLong(prs, "PrescriberID", -1);
			renewalNeedingAttn.nRequestMessageGUID = AdoFldString(prs, "RequestMessageID");
			renewalNeedingAttn.dtSentTime = AdoFldDateTime(prs, "ReceivedDate");
			arryRenewalsNeedingAttention.Add(renewalNeedingAttn);
			prs->MoveNext();
		}

		prs = prs->NextRecordset(NULL);

		while(!prs->eof)
		{
			NexERxPrescriptionAttention prescriptionNeedingAttn;
			prescriptionNeedingAttn.nPrescriberID = AdoFldLong(prs, "ProviderID", -1);
			prescriptionNeedingAttn.nPrescriptionID = AdoFldLong(prs, "ID");
			prescriptionNeedingAttn.dtPrescriptionDate = AdoFldDateTime(prs, "PrescriptionDate", COleDateTime(0, 0, 0, 0, 0, 0));
			arryPrescriptionsNeedingAttention.Add(prescriptionNeedingAttn);
			prs->MoveNext();
		}

		prs = prs->NextRecordset(NULL);

		while(!prs->eof)
		{
			long nProviderID = AdoFldLong(prs, "ProviderID");
			aryProvidersToNotify.insert(nProviderID);
			prs->MoveNext();
		}

		//Notify/Unnotify
		long nRenewalCount = 0;
		long nPrescriptionCount = 0;
		// (s.tullis 2016-01-28 17:45) - PLID 67965 - See if we want see Renewal Notices
		BOOL bNotifyRenewal = CanShowRenewalNotifications(bLogin);
		// (s.tullis 2016-01-28 17:46) - PLID 68090 - See if we want to see Prescription Needing Attention notices
		BOOL bNotifyPrescription = CanShowRxAttentionNotifications(bLogin);

		std::map<long,COleDateTime> mapProvIDToRenewalTime;
		std::map<long,COleDateTime> mapProvIDToPrescriptionTime;

		foreach(NexERxRenewalNeedingAttention renewalNeedingAttn, arryRenewalsNeedingAttention)
		{
			foreach(int nProviderID, aryProvidersToNotify)
			{
				if(renewalNeedingAttn.nPrescriberID == nProviderID)
				{
					if(!m_mapProvIDToRenewalTime.count(nProviderID) ||
						m_mapProvIDToRenewalTime[nProviderID] < renewalNeedingAttn.dtSentTime)
					{
						//You should be notified
						// (s.tullis 2016-01-28 17:45) - PLID 67965 - Compare agaist our value earlier
						bNotifyRenewal = bNotifyRenewal && TRUE;
						if(!mapProvIDToRenewalTime.count(nProviderID) || mapProvIDToRenewalTime[nProviderID] < renewalNeedingAttn.dtSentTime)
						{
							mapProvIDToRenewalTime[nProviderID] = renewalNeedingAttn.dtSentTime;
						}						
					}
					++nRenewalCount;
				}
			}
		}

		foreach(NexERxPrescriptionAttention prescriptionNeedingAttn, arryPrescriptionsNeedingAttention)
		{
			foreach(int nProviderID, aryProvidersToNotify)
			{
				if(prescriptionNeedingAttn.nPrescriberID == nProviderID)
				{
					if(!m_mapProvIDToPrescriptionTime.count(nProviderID) ||
						m_mapProvIDToPrescriptionTime[nProviderID] < prescriptionNeedingAttn.dtPrescriptionDate)
					{
						//You should be notified
						// (s.tullis 2016-01-28 17:46) - PLID 68090 - Compare agaist our earlier value
						bNotifyPrescription =bNotifyPrescription && TRUE;
						if(!mapProvIDToPrescriptionTime.count(nProviderID) ||
							mapProvIDToPrescriptionTime[nProviderID] < prescriptionNeedingAttn.dtPrescriptionDate)
						{
							mapProvIDToPrescriptionTime[nProviderID] = prescriptionNeedingAttn.dtPrescriptionDate;
						}
					}
					++nPrescriptionCount;
				}				
			}
		}

		// (s.tullis 2016-01-28 17:46) - PLID 68090 
		// (s.tullis 2016-01-28 17:45) - PLID 67965 
		// (j.jones 2016-02-03 16:57) - PLID 68118 - split this up into two different notifications
		if(bNotifyPrescription)
		{
			SetCanShowRxAttentionMessages(bNotifyPrescription);

			if(bNotifyPrescription && nPrescriptionCount > 0)
			{
				CString strMessage = "";
				strMessage.Format("You have %li NexERx Prescription%s that need%s attention.", nPrescriptionCount, nPrescriptionCount > 1? "s" : "", nPrescriptionCount > 1? "" : "s");
				NotifyUser(NT_RX_NEEDING_ATTENTION, strMessage, false);
			}
		}
		else if(nPrescriptionCount == 0)
		{
			UnNotifyUser(NT_RX_NEEDING_ATTENTION);
		}

		if (bNotifyRenewal)
		{
			SetCanShowRenewalMessages(bNotifyRenewal);

			if (bNotifyRenewal && nRenewalCount > 0)
			{
				CString strMessage = "";
				strMessage.Format("You have %li NexERx Renewal%s that need%s attention.", nRenewalCount, nRenewalCount > 1 ? "s" : "", nRenewalCount > 1 ? "" : "s");
				NotifyUser(NT_SS_RENEWALS_NEEDING_ATTENTION, strMessage, false);
			}
		}
		else if (nRenewalCount == 0)
		{
			UnNotifyUser(NT_SS_RENEWALS_NEEDING_ATTENTION);
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CMainFrame::DisplayRegisterNexERx()
{
	try {
		// (b.savon 2012-11-01 12:12) - PLID 53559
		//License
		if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts &&
			IsCurrentUserAdministrator()) {
			
			CNexERxRegistrationDlg dlg(this);
			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.gruber 2009-05-11 11:46) - PLID 34202 - provider types
void CMainFrame::OnEditProviderTypes() {

	try {
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 66, "Edit Combo Box").DoModal();
	}NxCatchAll("Error in CMainFrame::EditProviderTypes");
}

// (a.walling 2009-05-13 15:44) - PLID 34243 - Import CCD documents
void CMainFrame::OnImportCCD() {
	try {
		// (a.walling 2009-06-08 15:20) - PLID 34243 - Check user permissions
		if (!CheckCurrentUserPermissions(bioPatientHistory, sptWrite)) {
			return;
		}
		if(FlipToModule(PATIENT_MODULE_NAME)) {
			CPatientView* pView = (CPatientView*)(GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME));
			if (pView) {
				pView->SetActiveTab(PatientsModule::HistoryTab);

				CNxDialog* pDialog = pView->GetActiveSheet();

				if (pDialog->IsKindOf(RUNTIME_CLASS(CHistoryDlg))) {
					((CHistoryDlg*)pDialog)->ImportCCDs();
				}
			}
		}
	} NxCatchAll("CHistoryDlg::OnImportCCD");
}

// (a.walling 2009-05-21 08:41) - PLID 34318 - Create a CCD summary document
void CMainFrame::OnCreateCCD() {
	try {		
		// (a.walling 2009-06-08 15:20) - PLID 34243 - Check user permissions
		if (!CheckCurrentUserPermissions(bioPatientHistory, sptWrite)) {
			return;
		}

		// (a.walling 2010-01-19 14:09) - PLID 36972 - Validation via NIST web service removed (service out of date)
		{
			long nPatientID = GetActivePatientID();

			if (nPatientID != -1) {
				CreateCCD(nPatientID, this);
			}
		}
	} NxCatchAll("Could not create CCD");
}

// (a.walling 2009-06-05 13:15) - PLID 34176
void CMainFrame::CreateCCD(long nPatientID, CWnd* pParent)
{
	ENSURE(pParent != NULL);
	ENSURE(nPatientID != -1);

	CWaitCursor cws;

	//(e.lally 2010-02-18) PLID 37438 - Pass in our connection ptr, our "minimal" preference, userID, and locationID
	bool bGenerateOptionalSections = GetRemotePropertyInt("CCD_Optional", FALSE, 0, "<None>", true) ? true : false;
	CCD::ClinicalDocument Document(GetRemoteData(), bGenerateOptionalSections, GetCurrentUserID(), GetCurrentLocationID());
	Document.Generate(nPatientID);

	CString strTitle, strDisplay;
	CCD::CTimeRange tr;
	CString strDefault = "Generated Summary";
	CString strDescription;
	CCD::GetDescriptionInfo(Document.GetDocument(), strDefault, strTitle, strDisplay, strDescription, tr);

	COleDateTime dtDate;
	if (tr.GetSingleTime().GetStatus() == COleDateTime::valid) {
		dtDate = tr.GetSingleTime();
	} else {
		// (c.haag 2010-01-28 11:00) - PLID 37086 - We want to write the server's current time, not the
		// workstation's current time. So, set dtDate to null.
		dtDate.SetStatus(COleDateTime::null);
	}
	
	CString strWarnings = Document.GetWarnings();
	if (!strWarnings.IsEmpty()) {
		if (IDNO == pParent->MessageBox(FormatString("The following warnings were encountered while generating the document:\r\n\r\n%s\r\n\r\nDo you want to continue?", strWarnings), NULL, MB_YESNO)) {
			return;
		}
	}

	CString strFileName = GetPatientDocumentName(nPatientID, "xml");
	if (strFileName.IsEmpty()) {
		ThrowNxException("Could not get filename to save to history");
	}

	CString strFilePath = GetPatientDocumentPath(nPatientID) ^ strFileName;

	Document.SaveToFile(strFilePath);

	// (d.singleton 2013-11-15 11:23) - PLID 59513 - need to insert the CCDAType when generating a CCDA
	CreateNewMailSentEntry(nPatientID, strDescription, SELECTION_CCD, strFileName, GetCurrentUserName(), "", GetCurrentLocationID(), dtDate, ctNone);

	CString strMessage;
	strMessage.Format("A CCD summary document has been successfully created and attached to history for %s.", GetExistingPatientName(nPatientID));

	pParent->MessageBox(strMessage, "Practice", MB_ICONINFORMATION);
}

// (z.manning 2009-05-19 12:37) - PLID 28512
void CMainFrame::HandleActivePatientChange()
{
	// (z.manning 2009-05-19 12:37) - PLID 28512 - The patients seen today dialog now has the
	// option to filter on the active patient, so refresh it if necessary.
	if(IsWindow(m_pEMRSearchDlg.GetSafeHwnd()) && m_pEMRSearchDlg.IsWindowVisible()) {
		m_pEMRSearchDlg.RefreshList();
	}

	// (d.lange 2011-03-21 10:29) - PLID 41010 - Notify the device import dialog that the active patient has changed
	DeviceImport::GetMonitor().NotifyDeviceImportPatientChanged();

	// (j.fouts 2012-09-07 13:18) - PLID 52482 - This needs to refresh when the current patient changes
	// (j.jones 2012-09-26 11:28) - PLID 52872 - only if the window is currently visible
	if(m_pDrugInteractionDlg != NULL && m_pDrugInteractionDlg->IsWindowVisible())
	{
		m_pDrugInteractionDlg->HandleActivePatientChange();
	}

	// (j.armen 2012-02-27 17:24) - PLID 48303
	// (a.walling 2013-11-22 16:05) - PLID 60008 - Don't call DoFilter if we are not visible, and even then only if we are showing only the active patient.
	if(m_pRecallsNeedingAttentionDlg && m_pRecallsNeedingAttentionDlg->m_bUsePatientID && ::IsWindow(m_pRecallsNeedingAttentionDlg->GetSafeHwnd()) && m_pRecallsNeedingAttentionDlg->IsWindowVisible()) {
		m_pRecallsNeedingAttentionDlg->DoFilter();
	}
}

// (a.walling 2009-06-12 13:23) - PLID 34176 - Generic modeless window tracker
void CMainFrame::RegisterModelessWindow(HWND hwnd)
{
	m_mapModelessWindows[hwnd]++;	
}

void CMainFrame::UnregisterModelessWindow(HWND hwnd)
{
	m_mapModelessWindows.RemoveKey(hwnd);
}

// (j.jones 2014-08-20 09:40) - PLID 63427 - Given a tablechecker type,
// returns true if the tablechecker should be processed immediately or
// added to our staggered queue.
// Will always return true if the local stagger is disabled.
bool CMainFrame::ShouldImmediatelySendTablechecker(NetUtils::ERefreshTable ertTableType)
{
	//If m_bNxServerBroadcastStagger is true, then we do not
	//stagger tablechecker processing on the client side.
	//This is not enabled by default.
	if (m_bNxServerBroadcastStagger) {
		return true;
	}
	
	//If the stagger times are both zero, then there is no point in queuing up
	//the stagger, because everything processes immediately.
	//This is not typical.
	if (m_nTableCheckerStaggerMin == 0 && m_nTableCheckerStaggerMax == 0) {
		return true;
	}

	//If the tablechecker is specifically excluded from the stagger,
	//we will always send it immediately.
	if (CClient::IsImmediateTablechecker(ertTableType)) {
		return true;
	}

	//If we get here, the requested tablechecker is not immediate,
	//so return false. This is the most common return value.
	return false;
}

// (a.walling 2009-07-10 09:54) - PLID 33644
LRESULT CMainFrame::QueuePendingMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	bool bAdded = false;
	if (message == WM_TABLE_CHANGED_EX) {

		// (j.jones 2014-08-19 09:44) - PLID 63411 - added duplication checks for Ex messages
		
		//Ex tablechecker, we will not add it to the queue if the same
		//TC exists with the same detail list

		bool bFound = false;

		POSITION posMessage = m_listPendingTableCheckerMessages.GetHeadPosition();
		while (posMessage != NULL && !bFound) {
			CPendingMessage* pExistingMessage = m_listPendingTableCheckerMessages.GetNext(posMessage);
			if (pExistingMessage->msg == message && pExistingMessage->wParam == wParam) {
				//this is an Ex checker for the same table, now see if the detail lists match

				CTableCheckerDetails* pNewDetails = (CTableCheckerDetails*)lParam;
				CTableCheckerDetails* pQueuedDetails = (CTableCheckerDetails*)pExistingMessage->lParam;

				if (pNewDetails != NULL && pQueuedDetails != NULL) {

					if (pNewDetails->GetStartPosition() == NULL) {
						//the Ex message had no details (which shouldn't happen),
						//let's see if the existing message also has no details
						if (pQueuedDetails->GetStartPosition() == NULL) {
							//it also has no details, so this is a match
							bFound = true;
							continue;
						}
						else {
							//the existing message has details, so these do not match
							continue;
						}
					}
					else {
						long nCount = 0;
						POSITION posDetail = pNewDetails->GetStartPosition();
						COleVariant v;

						bool bDetailsIdentical = true;

						while (posDetail != NULL && bDetailsIdentical)
						{
							nCount++;

							//this is used just for iteration, we will not use the values returned
							{
								unsigned short nID;
								COleVariant v;
								pNewDetails->GetNextAssoc(posDetail, nID, v);
							}

							//these are 1-based, not 0-based
							_variant_t v1 = pNewDetails->GetDetailData(nCount);
							_variant_t v2 = pQueuedDetails->GetDetailData(nCount);

							if (v1.vt != v2.vt || VariantCompare(&v1, &v2) != VARCMP_EQ) {
								//these details have different content
								bDetailsIdentical = false;
								break;
							}
						}

						if (bDetailsIdentical) {
							//all the details were the same, so we can skip this message
							bFound = true;

							//delete the details
							delete pNewDetails;

							continue;
						}
					}
				}
			}
		}

		if (!bFound) {
			m_listPendingTableCheckerMessages.AddTail(new CPendingMessage(message, wParam, lParam));
			bAdded = true;
		}
	}
	else if (message == WM_TABLE_CHANGED) {

		//normal tablechecker, we will not add it to the queue if the same
		//TC exists with the same record ID

		bool bFound = false;
		POSITION pos = m_listPendingTableCheckerMessages.GetHeadPosition();		
		while (pos != NULL && !bFound) {
			CPendingMessage* pExistingMessage = m_listPendingTableCheckerMessages.GetNext(pos);
			if (pExistingMessage->msg == message && pExistingMessage->wParam == wParam && pExistingMessage->lParam == lParam) {
				bFound = true;
			}
		}

		if (!bFound) {
			m_listPendingTableCheckerMessages.AddTail(new CPendingMessage(message, wParam, lParam));
			bAdded = true;
		}
	}

	if (bAdded) {
		//We added a message to the queue.
		//If the timer is not active, start it now.
		if (m_nQueuedMessageTimerID == 0) {
			ResetQueuedMessagesTimer();
		}
	}
	
	return 0;
}

void CMainFrame::ResetQueuedMessagesTimer()
{
	// default min is 200

	// we definitely need to stagger more if there are lots of workstations.
	// Let's assume a minimum of 50 is good and probably does not need to change. The number to change
	// is probably the upper max. This would be dependent on number of workstations. The idea here is
	// to limit the strain on the server if everyone tries to refresh at once.
	//
	// Let's look a few options, the max * # workstations:
	//
	//					*1000		*750		*500	*250		*125
	//
	// Tiny 1-5			1 - 5		.75-3.75	.5-2.5	.25-1.75	.125-.625
	//
	// Small ~10		10			7.5			5		2.5			1.25
	//
	// Medium 10-20		10 - 20		7.5-15		5-10	2.5-5		1.25-2.5
	//
	// Large 20-30		20 - 30		15-22.5		10-15	5-7.5		2.5-3.75
	//
	// Super 30-40		30 - 40		22.5-30		15-20	7.5-10		3.75-5
	//
	// Massive 40-50	40 - 50		30-37.5		20-25	10-12.5		5-6.25
	//
	// Betelgeuse 50+	50+			37.5+		25+		12.5+		6.25+
	//
	// I shall use 200, which I think should be acceptable. I think the refreshing can be done
	// even in larger offices within that period of time.

	// GetWorkstationCountUsed()
	int nWorkstations = g_pLicense->GetWorkstationCountUsed();

	// m_nTableCheckerStaggerMax is * # of workstations

	// get a good random double so we don't have to worry about exceeding RAND_MAX, a critique
	// which also applied to various other parts of our code.	
	DWORD dwMaxDelay;
	if (m_nTableCheckerStaggerMax > 0 && nWorkstations > 0) {
		dwMaxDelay = m_nTableCheckerStaggerMax * nWorkstations;	
	} else {
		dwMaxDelay = m_nTableCheckerStaggerMin * 2;
	}	

	UINT nElapse = 0;
	{	// I got this code from http://www.azillionmonkeys.com/qed/random.html 'Misconceptions about rand()'
		double d; // 53 bit mantissa, 45 bits of precision
#define RS_SCALE (1.0 / (1.0 + RAND_MAX))
		do {
			d = (((rand () * RS_SCALE) + rand ()) * RS_SCALE + rand ()) * RS_SCALE;
		} while (d >= 1);

		nElapse = (UINT)(dwMaxDelay * d);
		nElapse += m_nTableCheckerStaggerMin; // add the minimum time
	}

	SetTimer(IDT_QUEUEDMESSAGE_TIMER, nElapse, NULL);
	m_nQueuedMessageTimerID = IDT_QUEUEDMESSAGE_TIMER;
}

// (j.jones 2014-08-19 10:19) - PLID 63412 - the tablechecker queue now works off of an elapsed time,
// and will keep processing until a set period of time has passed
void CMainFrame::BeginProcessingQueuedMessages()
{
	try {

		// (j.jones 2014-08-19 10:21) - PLID 63412 - This function will now just fire
		// NXM_POST_NEXT_QUEUED_TABLECHECKER with a tick count of -1.
		// ProcessNextQueuedMessage will then get the current tick count,
		// and keep firing that message until time elapses.

		m_nQueuedMessageTimerID = 0;
		KillTimer(IDT_QUEUEDMESSAGE_TIMER);

		if (!m_listPendingTableCheckerMessages.IsEmpty()) {

			//don't provide a tick count yet, it will be calculated once we send the
			//first tablechecker in the queue
			PostMessage(NXM_POST_NEXT_QUEUED_TABLECHECKER, 0, 0);
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-08-19 10:34) - PLID 63412 - event to process the next queued tablechecker
LRESULT CMainFrame::OnPostNextQueuedTablechecker(WPARAM wParam, LPARAM lParam)
{
	try {

		ProcessNextQueuedMessage(wParam, lParam);

	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.jones 2014-08-19 10:19) - PLID 63412 - the tablechecker queue now works off of an elapsed time,
// and will keep processing until a set period of time has passed
void CMainFrame::ProcessNextQueuedMessage(DWORD dwInitialTickCount, long nCountProcessed)
{
	try {
		
		// (j.jones 2014-08-19 10:21) - PLID 63412 - the batch limit also has a timer now,
		// the count is now a minmum
		const long cnBatchMinRecordLimit = m_nQueuedMessageBatchMinLimit;
		const long cnBatchTimeLimitMS = m_nQueuedMessageBatchTimeLimitMS;

		m_nQueuedMessageTimerID = 0;
		KillTimer(IDT_QUEUEDMESSAGE_TIMER);

		long nCountTCs = m_listPendingTableCheckerMessages.GetCount();

		if (nCountTCs > 0) {

			DWORD dwTickCount = ::GetTickCount();

			//The first message will not have had a tick count provided.
			//If it is <= 0, use the current tick count to start the timer.
			if (dwInitialTickCount <= 0) {
				dwInitialTickCount = dwTickCount;
			}

			bool bKeepPosting = true;
			if ((long)(dwTickCount - dwInitialTickCount) > cnBatchTimeLimitMS) {
				//we've exceeded our time, so we'll stop posting
				bKeepPosting = false;

				//unless we only processed a couple records, in which case we should
				//keep processing until we have surpassed the minimum
				if (nCountProcessed < cnBatchMinRecordLimit) {
					//let's keep posting until we hit the minimum record count
					bKeepPosting = true;
				}
			}

			if (bKeepPosting) {
				//process the next tablechecker
				CPendingMessage* pMessage = m_listPendingTableCheckerMessages.RemoveHead();
				PostMessage(pMessage->msg, pMessage->wParam, pMessage->lParam);
				delete pMessage;

				nCountProcessed++;

				//now post that we're ready for the next tablechecker,
				//sending the original tick count from when this process began
				PostMessage(NXM_POST_NEXT_QUEUED_TABLECHECKER, dwInitialTickCount, nCountProcessed);
			}
			else {
				LogDetail("There are %li pending table checker messages! (%li processed in last batch.)", nCountTCs, nCountProcessed);
				ResetQueuedMessagesTimer();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CMainFrame::CleanupAllQueuedMessages()
{
	try {
		POSITION pos = m_listPendingTableCheckerMessages.GetHeadPosition();
		while (pos) {
			CPendingMessage* pMessage = m_listPendingTableCheckerMessages.GetNext(pos);
			delete pMessage;
		}

		m_listPendingTableCheckerMessages.RemoveAll();
	} NxCatchAllIgnore();
}

// (j.jones 2009-08-13 14:14) - PLID 35213 - added a mutex for NewCrop patient account access,
// this applies to the NewCrop browser actions only (such as ncatAccessPatientAccount and
// ncatProcessRenewalRequest), not any follow-up SOAP calls we may be attempting
BOOL CMainFrame::GetIsBrowsingNewCropPatientAccount()
{
	return m_bIsBrowsingNewCropPatientAccount;
}

void CMainFrame::SetIsBrowsingNewCropPatientAccount(BOOL bNewValue)
{
	m_bIsBrowsingNewCropPatientAccount = bNewValue;
}

// (z.manning 2009-08-25 10:55) - PLID 31944 - Added a debug-only option to the system menu
// to size Practice to 1024x768 which can be helpful during interface development.
void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
	try
	{
		switch(nID) {
			case DEBUGMENU_ID_SIZE_TO_1024_768: {
				ShowWindow(SW_RESTORE);
				MoveWindow(0, 0, 1024, 768);
			}
			break;
			
		// (a.walling 2009-10-23 09:46) - PLID 36046 - Ability to dump all EMNDetail objects in memory with reference count history
#ifdef WATCH_EMNDETAIL_REFCOUNTS
			case DEBUGMENU_ID_DUMP_ALL_EMNDETAILS: {
				g_EMNDetailManager.DumpAll();
			}
			break;
#endif
		}

		// (v.maida 2016-02-19 16:34) - PLID 68385 - If we're minimizing Practice, then set a flag indicating that we are.
		if (nID == SC_MINIMIZE) {
			m_bPracticeMinimized = TRUE;
		}
		else {
			m_bPracticeMinimized = FALSE;
		}

		CNxMDIFrameWnd::OnSysCommand(nID, lParam);

	}NxCatchAll(__FUNCTION__);
}



// (j.gruber 2013-10-08 13:08) - PLID 59062
void CMainFrame::OnCCDASettings()
{
	try {		
		CCCDAConfigDlg dlg;
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}


// (j.gruber 2010-01-11 15:48) - PLID 36647 - Configure Referral Source Phone numbers
void CMainFrame::OnConfigureReferralSourcePhoneNumbers() 
{
	CConfigureReferralSourcePhoneNumbersDlg dlg(this);
	dlg.DoModal();
}

// (j.gruber 2010-03-09 09:50) - PLID 37660
void CMainFrame::OnConfigureEmnChargesDiagCodesReport()
{
	try {
		CConfigEMNChargeDiagCodeReportDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-04-14 09:33) - PLID 37948
void CMainFrame::OnConfigurePermissionGroups() 
{
	try {
		//open it with groups, select the first one
		CConfigurePermissionGroupsDlg dlg(FALSE, -1, this);
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-05-21 11:04) - PLID 38817 - Bold setup dialog
void CMainFrame::OnConfigureBold() 
{
	try {		
		CBoldSettingsDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-09-22 11:18) - PLID 40629 - Configure patient education templates
void CMainFrame::OnPatientEducationSetup()
{
	try {
		CEducationalMaterialSetupDlg dlg(this);
		dlg.DoModal();
	}
	NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/03/2012) - PLID 51947 - Open up the Wound Care Coding Config dialog.
void CMainFrame::OnConfigureWoundCareCoding()
{
	try {
		CWoundCareSetupDlg dlg(this);
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-02-23 16:10) - PLID 37509 
void CMainFrame::OnConfigureClinicalSupportDecisionRules()
{
	try {
		// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
		
		//TES 11/10/2013 - PLID 59400 - Check permissions
		if(!CheckCurrentUserPermissions(bioCDSRules, sptWrite)) {
			return;
		}
		CInterventionTemplatesDlg dlg(this);
		dlg.OpenDecisionSupportConfiguration();
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2009-12-28 08:59) - PLID 32150 - added ability to delete unused service codes
void CMainFrame::OnDeleteUnusedServiceCodes()
{
	long nAuditTransactionID = -1;

	try {

		if(!CheckCurrentUserPermissions(bioServiceCodes, sptDelete)) {
			return;
		}

		CString strTempTableName;
		strTempTableName.Format("#TempDeleteServicesT_%lu", GetTickCount());

		//this recordset will return all service codes that are not in use
		//in a table that prevents deletion as specified in CCPTCodes::OnDeleteCpt()

		// (j.jones 2012-04-12 11:05) - PLID 49604 - Added handling for bad data where a ServiceT
		// record was used as both a CPTCode and a Product. It skips trying to delete these bad records.
		_RecordsetPtr rs = CreateRecordset("SET NOCOUNT ON "
			""
			"CREATE TABLE %s ("
			"ServiceID INT NOT NULL PRIMARY KEY, "
			"Code nvarchar(50) NOT NULL DEFAULT ('') "
			") "
			""
			"INSERT INTO %s (ServiceID, Code) "
			"SELECT ServiceT.ID, CPTCodeT.Code "
			"FROM CPTCodeT WITH(NOLOCK) "
			"INNER JOIN ServiceT WITH(NOLOCK) ON CPTCodeT.ID = ServiceT.ID "
			"WHERE ServiceT.ID NOT IN ("
			"	SELECT ServiceID FROM ChargesT WITH(NOLOCK) "
			"	UNION SELECT ID FROM ProductT WITH(NOLOCK) "
			"	UNION SELECT ID FROM AdministrativeFeesT WITH(NOLOCK) "
			"	UNION SELECT ServiceID FROM GCTypesT WITH(NOLOCK) "
			"	UNION SELECT ServiceID FROM MailSentServiceCptT WITH(NOLOCK) "
			"	UNION SELECT ServiceID FROM EMRChargesT WITH(NOLOCK) "
			"	UNION SELECT ServiceID FROM EMRTemplateChargesT WITH(NOLOCK) "
			"	UNION SELECT ServiceID FROM SurgeryDetailsT WITH(NOLOCK) "
			"	UNION SELECT ServiceID FROM PreferenceCardDetailsT WITH(NOLOCK) WHERE ServiceID Is Not Null "
			"	UNION SELECT ServiceID FROM InsuranceReferralCPTCodesT WITH(NOLOCK) "
			"	UNION SELECT ServiceID FROM EligibilityRequestsT WITH(NOLOCK) WHERE ServiceID Is Not Null "
			"	UNION SELECT MasterServiceID AS ServiceID FROM SuggestedSalesT WITH(NOLOCK) "
			"	UNION SELECT ServiceID FROM EMChecklistCodingLevelsT WITH(NOLOCK) "
			"	UNION SELECT UB92Box44 FROM BillsT WITH(NOLOCK) WHERE UB92Box44 Is Not Null "
			"	UNION SELECT CPTID FROM ServiceToProductLinkT WITH(NOLOCK) "
			//TES 4/25/2011 - PLID 41113 - Can't delete items that are in the "Items to Bill" section of a Glasses Order
			"	UNION SELECT ServiceID FROM GlassesOrderServiceT WITH(NOLOCK) "
			// (z.manning 2011-07-05 10:47) - PLID 44421 - Can't delete codes in EmrCodingGroupDetailsT
			"	UNION SELECT CptCodeID FROM EmrCodingGroupDetailsT WITH(NOLOCK) \r\n"
			// (s.dhole 2012-04-11 15:54) - PLID 43849 Can't delete Service codes if it is link to glasses items
			"	UNION SELECT CptId FROM GlassesCatalogDesignsCptT WITH(NOLOCK) \r\n"
			"	UNION SELECT CptId FROM GlassesCatalogMaterialsCptT WITH(NOLOCK) \r\n"
			"	UNION SELECT CptId FROM GlassesCatalogTreatmentsCptT WITH(NOLOCK) \r\n"
			") "
			""
			"SET NOCOUNT OFF "
			"SELECT ServiceID, Code FROM %s",
			strTempTableName, strTempTableName, strTempTableName);

		long nCount = rs->GetRecordCount();
		if(nCount == 0) {
			AfxMessageBox("There are no service codes that are not currently in use.");
			ExecuteSql("DROP TABLE %s", strTempTableName);
			return;
		}
		else {
			CString str;
			if(nCount == 1) {
				str.Format("There is one service code that is not currently in use.\n\n"
					"Are you SURE you want to delete this code? This action is not recoverable!");
			}
			else {
				str.Format("There are %li service codes that are not currently in use.\n\n"
					"Are you SURE you want to delete these codes? This action is not recoverable!", nCount);
			}

			if(IDNO == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
				ExecuteSql("DROP TABLE %s", strTempTableName);
				return;
			}
		}

		//if we're still here, we need to delete all the returned codes

		CWaitCursor pWait;

		//audit each code
		while(!rs->eof) {

			long nServiceID = AdoFldLong(rs, "ServiceID");
			CString strCode = AdoFldString(rs, "Code", "");

			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(-1, "", nAuditTransactionID, aeiCPTDelete, nServiceID, strCode, "<Deleted>",aepMedium, aetDeleted);

			rs->MoveNext();
		}
		rs->Close();

		CString strSqlBatch, strInClause;

		strInClause.Format("SELECT ServiceID FROM %s", strTempTableName);

		long nTimeout = GetRemoteData()->CommandTimeout;
		GetRemoteData()->CommandTimeout = 600;

		//the deletion queries are in a global function
		if(!strInClause.IsEmpty()) {
			DeleteServiceCodes(strSqlBatch, strInClause);
		}

		//drop the table at the end of the batch
		AddStatementToSqlBatch(strSqlBatch, "DROP TABLE %s", strTempTableName);

		if(!strSqlBatch.IsEmpty()) {
			//the batch is all one transaction
			
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(10000);
			ExecuteSqlBatch(strSqlBatch);
		}
		
		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		GetRemoteData()->CommandTimeout = nTimeout;

		//refresh all codes
		CClient::RefreshTable(NetUtils::CPTCodeT);

		AfxMessageBox("All unused service codes have been successfully deleted.");

	}NxCatchAllCall("Error in CMainFrame::OnDeleteUnusedServiceCodes()",
		if (nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

// (j.jones 2012-03-27 08:59) - PLID 45752 - added ability to delete unused diagnosis codes
// (j.armen 2012-04-10 14:12) - PLID 48299 - Added check for diags used in recalls
void CMainFrame::OnDeleteUnusedDiagCodes()
{
	long nAuditTransactionID = -1;

	try {

		if(!CheckCurrentUserPermissions(bioDiagCodes, sptDelete)) {
			return;
		}

		CWaitCursor pWait1;

		CIncreaseCommandTimeout ict(600);

		//This recordset will return all diagnosis codes that are not in use
		//in a table that prevents deletion as specified in CCPTCodes::DeleteICD9().
		//There's one exception: If an EMR action can spawn a diagnosis code, we won't delete
		//that diagnosis code. Manually deleting a code permits deleting spawnable codes, but
		//mass deleting will not. However we don't care if a diag code spawns something itself.
		// (j.jones 2014-02-18 15:50) - PLID 60719 - added handling for patients' default ICD10 codes
		// (j.jones 2014-02-26 10:31) - PLID 60781 - added handling for EMR Problems' ICD-10 field
		// (a.walling 2014-02-28 08:42) - PLID 61085 - BillDiagCodeT - Handle deleting patient / bill, handle deleting unused diag codes.
		// (j.armen 2014-03-10 11:37) - PLID 61212 - Run in snapshot.  Calculate tables to check by foreign key constraints.
		// (r.gonet 03/12/2014) - PLID 60778 - We're stepping on each other's toes here. Fixed the CPTDiagnosisGroupsT and BlockedDiagnosisCodesT table names (tables were renamed the other day).
		// See Exclusion list for tables that may have data, but are not considered "In Use".  Deletion code must handle those foreign key constraints
		// (b.savon 2014-07-14 14:15) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
		// (c.haag 2015-06-24) - PLID 66018 - Exclude custom crosswalks; they are only used in searches and not PHI.
		_RecordsetPtr prs = CreateRecordset(GetRemoteDataSnapshot(),
			"SET NOCOUNT ON\r\n"
			"DECLARE @cmd NVARCHAR(MAX)\r\n"
			"SELECT @cmd = '\r\n"
				"SELECT ID, CodeNumber\r\n"
				"FROM DiagCodes\r\n"
				"WHERE ID NOT IN (\r\n"
				"	-- Manual Foreign Key Constraints\r\n"
				"	SELECT [DiagCodeID_ICD9] FROM [EMRActionsT] INNER JOIN EmrActionDiagnosisDataT ON EMRActionsT.ID = EmrActionDiagnosisDataT.EmrActionID WHERE [Deleted] = 0 AND [DestType] = %li\r\n"
				"	UNION ALL "
				"	SELECT [DiagCodeID_ICD10] FROM [EMRActionsT] INNER JOIN EmrActionDiagnosisDataT ON EMRActionsT.ID = EmrActionDiagnosisDataT.EmrActionID WHERE [Deleted] = 0 AND [DestType] = %li\r\n"
				"	UNION SELECT DiagID FROM InsuranceReferralDiagsT\r\n"
				"	' +\r\n"
				"	STUFF ((\r\n"
				"		SELECT CHAR(10) + CHAR(9) + 'UNION SELECT [' + c.name + '] FROM [' + OBJECT_NAME(c.object_id) + ']' + CASE WHEN c.is_nullable = 1 THEN ' WHERE [' + c.name + '] IS NOT NULL' ELSE '' END\r\n"
				"		FROM sys.foreign_keys fk\r\n"
				"		INNER JOIN sys.foreign_key_columns fkc ON fk.object_id = fkc.constraint_object_id\r\n"
				"		INNER JOIN sys.columns c ON fkc.parent_object_id = c.object_id AND fkc.parent_column_id = c.column_id\r\n"
				"		WHERE fkc.referenced_object_id = OBJECT_ID('DiagCodes')\r\n"
				"			AND c.object_id NOT IN (\r\n"
				"				-- Tables to be excluded from this lookup\r\n"
				"				OBJECT_ID('EmrActionDiagnosisDataT'), \r\n"		
				"				OBJECT_ID('DiagnosisQuickListT'),\r\n"
				"				OBJECT_ID('DiagCodeToEMRInfoT'),\r\n"
				"				OBJECT_ID('LabDiagnosisT'),\r\n"
				"				OBJECT_ID('DiagInsNotesT'),\r\n"
				"				OBJECT_ID('CPTDiagnosisGroupsT'),\r\n"
				"				OBJECT_ID('BlockedDiagnosisCodesT'),\r\n"
				"				OBJECT_ID('DiagCodeCustomCrosswalksT'))\r\n"
				"		FOR XML PATH('')\r\n"
				"		), 1, 2, '') +\r\n"
				"	CHAR(10) + ')'\r\n"
			"\r\n"
			"DECLARE @Codes TABLE(ID INT, CodeNumber NVARCHAR(MAX))\r\n"
			"INSERT INTO @Codes\r\n"
			"exec sp_executesql @cmd\r\n"
			"\r\n"
			"SET NOCOUNT OFF\r\n"
			"SELECT ID, CodeNumber FROM @Codes\r\n"
			"SELECT (SELECT ID AS [@ID] FROM @Codes FOR XML PATH('row'), ROOT) AS UnusedIDs\r\n",
			eaoDiagnosis/*eaoDiag*/, eaoDiagnosis);


		long nCount = prs->GetRecordCount();
		if(nCount == 0) {
			AfxMessageBox("There are no diagnosis codes that are not currently in use.");
			return;
		}
		else {
			CString str;
			if(nCount == 1) {
				str.Format("There is one diagnosis code that is not currently in use.\n\n"
					"Are you SURE you want to delete this code? This action is not recoverable!");
			}
			else {
				str.Format("There are %li diagnosis codes that are not currently in use.\n\n"
					"Are you SURE you want to delete these codes? This action is not recoverable!", nCount);
			}

			if(IDNO == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		//if we're still here, we need to delete all the returned codes

		CWaitCursor pWait2;

		//audit each code
		for(; !prs->eof; prs->MoveNext()) {

			long nID = AdoFldLong(prs, "ID");
			CString strCode = AdoFldString(prs, "CodeNumber");

			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(-1, "", nAuditTransactionID, aeiICD9Delete, nID, strCode, "<Deleted>", aepMedium, aetDeleted);

		}

		// (j.jones 2012-04-30 09:19) - PLID 45752 - Every approach we took for deleting
		// ~16,000 diagnosis codes failed on one machine at some point.
		// Initially selecting the IDs into the temp table timed out, deleting in batches
		// using {INTARRAY} timed out, etc. The most effective solution was to fill the
		// temp table with IDs in batches, then delete from the temp table.
		// (j.armen 2014-03-10 11:37) - PLID 61212 - Implemented the XML approach.  This is much faster.
		// I was able to delete ~40,000 codes in under 5 seconds, with fewer queries.

		prs = prs->NextRecordset(NULL);

		CString strIDXml = AdoFldString(prs, "UnusedIDs", "");

		CParamSqlBatch sqlBatch;

		sqlBatch.Declare(
			"IF OBJECT_ID('tempdb..#DiagsToDelete') IS NOT NULL\r\n"
			"	DROP TABLE #DiagsToDelete\r\n");
		sqlBatch.Declare("CREATE TABLE #DiagsToDelete(DiagID INT)");
		sqlBatch.Declare("DECLARE @xml XML");
		sqlBatch.Add("SET @xml = {STRING}", strIDXml);
		sqlBatch.Declare("INSERT INTO #DiagsToDelete\r\n"
			"SELECT c.value('@ID', 'INT') FROM @xml.nodes('root/row') T(c)");

		//now execute the batch
		NxAdo::PushMaxRecordsWarningLimit pmr(20000);
		DeleteDiagnosisCodes(sqlBatch, CSqlFragment("SELECT DiagID FROM #DiagsToDelete"));
		sqlBatch.Execute(GetRemoteData());

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		//refresh all codes
		CClient::RefreshTable(NetUtils::DiagCodes);

		if(GetActiveView()) {
			GetActiveView()->UpdateView();
		}

		AfxMessageBox("All unused diagnosis codes have been successfully deleted.");

	}NxCatchAllCall(__FUNCTION__,
		if (nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

// (j.jones 2012-03-27 09:20) - PLID 47448 - added ability to delete unused modifiers
void CMainFrame::OnDeleteUnusedModifiers()
{
	long nAuditTransactionID = -1;

	try {

		//this should not be accessible if the Alberta preference is enabled,
		//but if we manage to get here, leave now
		if(UseAlbertaHLINK()) {
			AfxMessageBox("All modifiers are required for Alberta billing. Deletion of unused modifiers is not permitted.");
			return;
		}

		if(!CheckCurrentUserPermissions(bioServiceModifiers, sptDelete)) {
			return;
		}

		CWaitCursor pWait1;

		CString strTempTableName;
		strTempTableName.Format("#TempDeleteCPTModifierT_%lu", GetTickCount());

		long nTimeout = GetRemoteData()->CommandTimeout;
		GetRemoteData()->CommandTimeout = 600;

		//this recordset will return all modifiers that are not in use
		//in a table that prevents deletion as specified in CCPTCodes::DeleteModifier()
		_RecordsetPtr rs = CreateRecordset("SET NOCOUNT ON "
			""
			"CREATE TABLE %s ("
			"Number nvarchar(10) NOT NULL PRIMARY KEY "
			") "
			""
			"INSERT INTO %s (Number) "
			"SELECT Number "
			"FROM CPTModifierT WITH(NOLOCK) "
			"WHERE Number NOT IN ("
			"	SELECT Mod FROM AlbertaCPTModLinkT WITH(NOLOCK) "
			"	UNION SELECT CPTModifier FROM ChargesT WITH(NOLOCK) WHERE CPTModifier Is Not Null "
			"	UNION SELECT CPTModifier2 FROM ChargesT WITH(NOLOCK) WHERE CPTModifier2 Is Not Null "
			"	UNION SELECT CPTModifier3 FROM ChargesT WITH(NOLOCK) WHERE CPTModifier3 Is Not Null "
			"	UNION SELECT CPTModifier4 FROM ChargesT WITH(NOLOCK) WHERE CPTModifier4 Is Not Null "
			"	UNION SELECT CLIAModifier FROM InsuranceCoT WITH(NOLOCK) WHERE CLIAModifier Is Not Null "
			"	UNION SELECT CPTModifier1 FROM EMRChargesT WITH(NOLOCK) WHERE CPTModifier1 Is Not Null "
			"	UNION SELECT CPTModifier2 FROM EMRChargesT WITH(NOLOCK) WHERE CPTModifier2 Is Not Null "
			"	UNION SELECT CPTModifier3 FROM EMRChargesT WITH(NOLOCK) WHERE CPTModifier3 Is Not Null "
			"	UNION SELECT CPTModifier4 FROM EMRChargesT WITH(NOLOCK) WHERE CPTModifier4 Is Not Null "
			"	UNION SELECT CPTModifier1 FROM EMRTemplateChargesT WITH(NOLOCK) WHERE CPTModifier1 Is Not Null "
			"	UNION SELECT CPTModifier2 FROM EMRTemplateChargesT WITH(NOLOCK) WHERE CPTModifier2 Is Not Null "
			"	UNION SELECT CPTModifier3 FROM EMRTemplateChargesT WITH(NOLOCK) WHERE CPTModifier3 Is Not Null "
			"	UNION SELECT CPTModifier4 FROM EMRTemplateChargesT WITH(NOLOCK) WHERE CPTModifier4 Is Not Null "
			"	UNION SELECT Modifier1Number FROM EMRActionChargeDataT WITH(NOLOCK) WHERE Modifier1Number Is Not Null "
			"	UNION SELECT Modifier2Number FROM EMRActionChargeDataT WITH(NOLOCK) WHERE Modifier2Number Is Not Null "
			"	UNION SELECT Modifier3Number FROM EMRActionChargeDataT WITH(NOLOCK) WHERE Modifier3Number Is Not Null "
			"	UNION SELECT Modifier4Number FROM EMRActionChargeDataT WITH(NOLOCK) WHERE Modifier4Number Is Not Null "
			"	UNION SELECT Modifier1 FROM EligibilityRequestsT WITH(NOLOCK) WHERE Modifier1 Is Not Null "
			"	UNION SELECT Modifier2 FROM EligibilityRequestsT WITH(NOLOCK) WHERE Modifier2 Is Not Null "
			"	UNION SELECT Modifier3 FROM EligibilityRequestsT WITH(NOLOCK) WHERE Modifier3 Is Not Null "
			"	UNION SELECT Modifier4 FROM EligibilityRequestsT WITH(NOLOCK) WHERE Modifier4 Is Not Null "
			") "
			""
			"SET NOCOUNT OFF "
			"SELECT Number FROM %s",
			strTempTableName, strTempTableName, strTempTableName);

		long nCount = rs->GetRecordCount();
		if(nCount == 0) {
			AfxMessageBox("There are no service code modifiers that are not currently in use.");
			ExecuteSql("DROP TABLE %s", strTempTableName);
			GetRemoteData()->CommandTimeout = nTimeout;
			return;
		}
		else {
			CString str;
			if(nCount == 1) {
				str.Format("There is one service code modifier that is not currently in use.\n\n"
					"Are you SURE you want to delete this modifier? This action is not recoverable!");
			}
			else {
				str.Format("There are %li service code modifiers that are not currently in use.\n\n"
					"Are you SURE you want to delete these modifiers? This action is not recoverable!", nCount);
			}

			if(IDNO == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
				ExecuteSql("DROP TABLE %s", strTempTableName);
				GetRemoteData()->CommandTimeout = nTimeout;
				return;
			}
		}

		//if we're still here, we need to delete all the returned codes

		CWaitCursor pWait2;

		//audit each code
		while(!rs->eof) {

			CString strModifier = AdoFldString(rs, "Number", "");

			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(-1, "", nAuditTransactionID, aeiModifierDelete, -1, strModifier, "<Deleted>", aepMedium, aetDeleted);

			rs->MoveNext();
		}
		rs->Close();

		CString strSqlBatch, strInClause;

		strInClause.Format("SELECT Number FROM %s", strTempTableName);

		//the deletion queries are in a global function
		if(!strInClause.IsEmpty()) {
			DeleteCPTModifiers(strSqlBatch, strInClause);
		}

		//drop the table at the end of the batch
		AddStatementToSqlBatch(strSqlBatch, "DROP TABLE %s", strTempTableName);

		if(!strSqlBatch.IsEmpty()) {
			//the batch is all one transaction
			
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(10000);
			ExecuteSqlBatch(strSqlBatch);
		}
		
		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		GetRemoteData()->CommandTimeout = nTimeout;

		//refresh all codes
		CClient::RefreshTable(NetUtils::CPTModifierT);

		if(GetActiveView()) {
			GetActiveView()->UpdateView();
		}

		AfxMessageBox("All unused service code modifiers have been successfully deleted.");

	}NxCatchAllCall(__FUNCTION__,
		if (nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

// (a.walling 2009-12-22 17:17) - PLID 7002 - Is a billing window already active?
bool CMainFrame::IsBillingModuleOpen(bool bShowMessage, CWnd** ppReturnBillingModule /*= NULL*/)
{
	CPatientView* pView = (CPatientView *)GetOpenView(PATIENT_MODULE_NAME);
	if (pView == NULL) {
		return false;
	}

	CBillingModuleDlg* pBillingModuleDlg = pView->GetBillingDlg();

	if (pBillingModuleDlg == NULL) {
		return false;
	}

	if (!pBillingModuleDlg->GetSafeHwnd() || !::IsWindow(pBillingModuleDlg->GetSafeHwnd())) {
		return false;
	}

	if (!pBillingModuleDlg->IsWindowVisible()) {
		return false;
	}

	if (ppReturnBillingModule) {
		*ppReturnBillingModule = pBillingModuleDlg;
	}

	if (bShowMessage) {
		WINDOWPLACEMENT wp;
		::ZeroMemory(&wp, sizeof(wp));
		if (pBillingModuleDlg->GetWindowPlacement(&wp)) {
			if (pBillingModuleDlg->IsIconic()) {
				wp.showCmd = SW_RESTORE;
				pBillingModuleDlg->SetWindowPlacement(&wp);
			}
		}
		pBillingModuleDlg->BringWindowToTop();
		pBillingModuleDlg->MessageBox("Please finish editing this bill before continuing.", NULL, MB_ICONSTOP);
	}

	return true;
}

// (a.walling 2009-10-23 09:46) - PLID 36046 - Fix the system menu to enable these items (pre-existing bug)
void CMainFrame::OnInitMenu(CMenu* pMenu)
{
#ifdef _DEBUG
	pMenu->EnableMenuItem(DEBUGMENU_ID_SIZE_TO_1024_768, MF_BYCOMMAND|MF_ENABLED);
#ifdef WATCH_EMNDETAIL_REFCOUNTS
	pMenu->EnableMenuItem(DEBUGMENU_ID_DUMP_ALL_EMNDETAILS, MF_BYCOMMAND|MF_ENABLED);
#endif
#endif
}

//TES 1/11/2010 - PLID 36761 - Does the current user have permission to access the given patient?
bool CMainFrame::CanAccessPatient(long nPatientID, bool bSilent)
{
	CArray<IdName,IdName> arBlockedGroups;
	GetBlockedGroups(arBlockedGroups);
	if(arBlockedGroups.GetSize() == 0) {
		//TES 1/11/2010 - PLID 36761 - If they aren't blocked from any groups, then they can certainly access this patient.
		return true;
	}

	// If non-patient sentinel ID (i.e. -25) then it can't be in a security group anyway, so we will allow it
	if (nPatientID < 0)
	{
		return true;
	}

	// (j.jones 2010-01-14 14:14) - PLID 35775 - if the user gains emergency access to a patient,
	// they have access for the remainder of their Practice session, so check and see if this
	// is one of those patients
	for(int i=0; i<m_arEmergencyAccessedPatientIDs.GetSize(); i++) {
		if(m_arEmergencyAccessedPatientIDs.GetAt(i) == nPatientID) {
			//they have previously gained access to this patient,
			//no need to check again
			return true;
		}
	}

	CArray<long, long> arynBlockedGroupIDs;
	for(int i = 0; i < arBlockedGroups.GetSize(); i++) {
		arynBlockedGroupIDs.Add(arBlockedGroups[i].nID);
	}
	
	if(bSilent) {
		//TES 1/12/2010 - PLID 36761 - If it's silent, we can just check whether the patient is in any of our blocked groups		
		// (j.jones 2010-01-14 14:06) - PLID 35775 - if it is silent, we will not provide the ability for emergency access
		// (z.manning 2015-11-06 08:59) - PLID 67543 - Fully parameterized
		_RecordsetPtr rs = CreateParamRecordset(
			"SELECT TOP 1 SecurityGroupID \r\n"
			"FROM SecurityGroupDetailsT \r\n"
			"WHERE PatientID = {INT} AND SecurityGroupID IN ({INTARRAY}) \r\n"
			, nPatientID, arynBlockedGroupIDs);
		if(!rs->eof) {
			return false;
		}
		else {
			return true;
		}
	}
	else {
		//TES 1/12/2010 - PLID 36761 - Not silent, so we need to check each group individually, in case a password is needed.
		// (z.manning 2015-11-06 08:59) - PLID 67543 - Fully parameterized
		_RecordsetPtr rsGroups = CreateParamRecordset(
			"SELECT SecurityGroupID \r\n"
			"FROM SecurityGroupDetailsT \r\n"
			"WHERE PatientID = {INT} AND SecurityGroupID IN ({INTARRAY})"
			, nPatientID, arynBlockedGroupIDs);

		// (j.jones 2010-01-14 14:08) - PLID 35775 - find the lowest common denominator of permissions for
		// all applicable security groups for this patient

		enum EAccessLevel {

			eNoAccess = 0,
			eCanAccessEmergWithPass = 1,
			eCanAccessEmerg = 2,
			eCanAccessWithPass = 3,
			eCanAccess = 4,			
		};

		EAccessLevel eacTotalAccessLevel = eCanAccess;

		//check each group to see which permissions we have,
		//stop checking if they have no possible way in
		while(!rsGroups->eof && eacTotalAccessLevel > eNoAccess) {

			EAccessLevel eacCurAccessLevel = eCanAccess;

			//find the highest level of access for this group

			//do they have access without a password?
			if(eacCurAccessLevel >= eCanAccess && !(GetCurrentUserPermissions(bioIndivSecurityGroups, TRUE, AdoFldLong(rsGroups, "SecurityGroupID")) & sptRead)) {
				//do they have access with a password?
				if(eacCurAccessLevel >= eCanAccessWithPass && !(GetCurrentUserPermissions(bioIndivSecurityGroups, TRUE, AdoFldLong(rsGroups, "SecurityGroupID")) & sptReadWithPass)) {
					//do they have emergency access without a password?
					if(eacCurAccessLevel >= eCanAccessEmerg && !(GetCurrentUserPermissions(bioIndivSecurityGroups, TRUE, AdoFldLong(rsGroups, "SecurityGroupID")) & sptDynamic0)) {
						//do they have emergency access with a password?
						if(eacCurAccessLevel >= eCanAccessEmergWithPass && !(GetCurrentUserPermissions(bioIndivSecurityGroups, TRUE, AdoFldLong(rsGroups, "SecurityGroupID")) & sptDynamic0WithPass)) {
							//if we get this far, they have NO access at all
							eacCurAccessLevel = eNoAccess;
						}
						else {
							eacCurAccessLevel = eCanAccessEmergWithPass;
						}
					}
					else {
						eacCurAccessLevel = eCanAccessEmerg;
					}
				}
				else {
					eacCurAccessLevel = eCanAccessWithPass;
				}
			}
			else {
				eacCurAccessLevel = eCanAccess;
			}

			//if the highest level of access for this group is lower
			//than our highest level of access for all groups, then
			//we have to reduce our total access level
			if(eacCurAccessLevel < eacTotalAccessLevel) {
				eacTotalAccessLevel = eacCurAccessLevel;
			}

			rsGroups->MoveNext();
		}

		//if the user has full access, let them on in
		if(eacTotalAccessLevel == eCanAccess) {
			return true;
		}

		//if the user has password access, check the password now, fail if it is wrong
		if(eacTotalAccessLevel == eCanAccessWithPass) {
			//if they enter it in, let them in, else, fail
			if(CheckCurrentUserPassword()) {

				//it's not really an emergency, but once we enter the password
				//then we track the patient the same way, such that the password
				//is not required a second time
				m_arEmergencyAccessedPatientIDs.Add(nPatientID);
				return true;
			}
			else {
				return false;
			}
		}

		// (j.jones 2010-01-14 14:08) - PLID 35775 - If they get this far, they are not allowed
		// access to at least one of the security groups. Now check and see if they have
		// emergency access to this patient.				
		if(eacTotalAccessLevel == eCanAccessEmerg || eacTotalAccessLevel == eCanAccessEmergWithPass) {
			if(IDNO == MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "You do not have permission to access this patient's records. "
				"However, you do have permission to acquire temporary emergency access to this patient, if needed.\n\n"
				"If you do continue and access this patient's infomation, you will retain access for the remainder "
				"of your current Practice session.\n\n"
				"Do you wish to acquire emergency access to this patient's account?")) {

				//they declined access, how noble of them!
				return false;
			}
			else {
				//if they require a password for emergency access, check it now,
				//otherwise let them in now
				if(eacTotalAccessLevel == eCanAccessEmerg || (eacTotalAccessLevel == eCanAccessEmergWithPass && CheckCurrentUserPassword())) {
					//they can get in, but audit that we let them in
					// (z.manning 2015-11-10 11:18) - PLID 67552 - Use the API for this now
					GetAPI()->AcquireEmergencyAccessToPatient(GetAPISubkey(), GetAPILoginToken(), _bstr_t(nPatientID));

					//track that this account is now accessible for the rest of the Practice session
					m_arEmergencyAccessedPatientIDs.Add(nPatientID);

					//TES 1/18/2010 - PLID 36895 - Need to refresh the combo, this patient's demographics are now visible.
					CClient::RefreshTable(NetUtils::PatCombo, nPatientID);
					return true;
				}
				else {
					return false;
				}
			}
		}

		//if they get here, they really have no way in
		ASSERT(eacTotalAccessLevel == eNoAccess);
		MsgBox(MB_ICONEXCLAMATION|MB_OK, "You do not have permission to access this patient's records.");
		return false;
	}
	return true;
}

//TES 1/11/2010 - PLID 36761 - Which Security Groups does the current user not have access to?
void CMainFrame::GetBlockedGroups(OUT CArray<IdName,IdName> &arBlockedGroups)
{
	//TES 1/11/2010 - PLID 36761 - If we can use our cached value, do so.
	if(m_bBlockedGroupsValid) {
		for(int i = 0; i < m_arBlockedGroups.GetSize(); i++) {
			arBlockedGroups.Add(m_arBlockedGroups[i]);
		}
		return;
	}
	else {
		//TES 1/11/2010 - PLID 36761 - We can't, so reload the list.
		m_arBlockedGroups.RemoveAll();
		_RecordsetPtr rsGroups = CreateParamRecordset("SELECT ID, Name FROM SecurityGroupsT");
		while(!rsGroups->eof) {
			// (j.jones 2010-01-14 14:30) - PLID 35775 - We currently load all blocked groups, regardless
			// of whether the user also has emergency access to them. Remember this, incase the 
			// logic changes in the future to the point where we also wish to cache that info.
			if(!CheckCurrentUserPermissions(bioIndivSecurityGroups, sptRead, TRUE, AdoFldLong(rsGroups, "ID"), TRUE)) {
				IdName idn;
				idn.nID = AdoFldLong(rsGroups, "ID");
				idn.strName = AdoFldString(rsGroups, "Name");
				m_arBlockedGroups.Add(idn);
				arBlockedGroups.Add(idn);
			}
			rsGroups->MoveNext();
		}
		m_bBlockedGroupsValid = true;
	}
}

//TES 1/15/2010 - PLID 36762 - Which patients has the user been granted emergency access to?
void CMainFrame::GetEmergencyAccessPatients(OUT CArray<long,long> &arPatientIDs)
{
	arPatientIDs.RemoveAll();
	arPatientIDs.Append(m_arEmergencyAccessedPatientIDs);
}

// (a.walling 2010-01-27 16:46) - PLID 37089 - Manually handle updating the frame title. We want to use the title of the active child.
void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	CMDIChildWnd* pActiveChild = MDIGetActive();

	LPCTSTR lpstrTitle = NULL;
	CString strTitle;

	if (pActiveChild != NULL &&
		(pActiveChild->GetStyle() & WS_MAXIMIZE) == 0)
	{
		strTitle = pActiveChild->GetTitle();
		if (!strTitle.IsEmpty())
			lpstrTitle = strTitle;
	}
	UpdateFrameTitleForDocument(lpstrTitle);
}

// (j.jones 2010-02-10 11:33) - PLID 37224 - functions for loading and accessing EMR image stamps
void CMainFrame::LoadEMRImageStamps(BOOL bForceReload /*= FALSE*/)
{
	//throw exceptions to the caller

	if(!m_bEMRImageStampsLoaded || bForceReload) {

		//clear the existing list
		ClearCachedEMRImageStamps();

		// (j.jones 2010-02-16 12:08) - PLID 37365 - added SmartStampTableSpawnRule
		// (j.jones 2010-02-16 12:15) - PLID 37377 - load inactive stamps, but track their status
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (z.manning 2012-01-26 17:11) - PLID 47592 - Added ShowDot
		// (r.gonet 05/02/2012) - PLID 49949 - Added image
		// (b.spivey, August 14, 2012) - PLID 52130 - Added CategoryID
		// (a.walling 2012-08-28 10:23) - PLID 51742 - Category name
		ADODB::_RecordsetPtr rs = CreateParamRecordset(
			"SELECT EmrImageStampsT.ID, EMRImageStampsT.StampText, "
				"EMRImageStampsT.TypeName, EMRImageStampsT.Description, "
				"EMRImageStampsT.Color, EMRImageStampsT.SmartStampTableSpawnRule, "
				"EMRImageStampsT.Inactive, EMRImageStampsT.ShowDot, "
				"EMRImageStampsT.[Image], EMRImageStampsT.CategoryID, "
				"EMRImageStampCategoryT.Description AS CategoryName \r\n"
			"FROM EMRImageStampsT \r\n"
			"LEFT JOIN EMRImageStampCategoryT ON EMRImageStampsT.CategoryID = EMRImageStampCategoryT.ID \r\n"
			"ORDER BY EMRImageStampsT.StampText, EmrImageStampsT.ID \r\n"
			"{SQL}",
			GetLoadActionInfoQuery(CSqlFragment("WHERE EmrActionsT.SourceType = {CONST}", eaoSmartStamp))
			);
		long nPreviousStampID = -1;
		
		// (a.walling 2012-08-28 10:23) - PLID 51742 - Use the SharedReferencer so category names only use one string in memory with multiple references
		SharedReferencer<> sharedRef;

		while(!rs->eof) {
			EMRImageStamp *eis = new EMRImageStamp;
			eis->nID = AdoFldLong(rs, "ID");
			eis->strStampText = AdoFldString(rs, "StampText", "");
			eis->strTypeName = AdoFldString(rs, "TypeName", "");
			eis->strDescription = AdoFldString(rs, "Description", "");
			eis->nTextColor = AdoFldLong(rs, "Color", 0);
			eis->eSmartStampTableSpawnRule = (EMRSmartStampTableSpawnRule)AdoFldLong(rs, "SmartStampTableSpawnRule", (long)esstsrAddNewRow);
			eis->bInactive = AdoFldBool(rs, "Inactive", FALSE);
			eis->bShowDot = AdoFldBool(rs, "ShowDot", TRUE); // (z.manning 2012-01-26 17:16) - PLID 47592
			{
				// (r.gonet 05/02/2012) - PLID 49949 - This next value is going to be a safearray of type UI1, convert it to a BYTE array
				_variant_t varBinary = rs->Fields->Item["Image"]->Value;
				long nImageNumBytes = 0;
				BYTE *arImageBytes = NULL;

				if (varBinary.vt == (VT_ARRAY | VT_UI1))
				{
					SafeArrayAccessData(varBinary.parray,(void **) &arImageBytes);
					nImageNumBytes = varBinary.parray->rgsabound[0].cElements;
					eis->m_ImageInfo.arImageBytes = new BYTE[nImageNumBytes];
					memcpy(eis->m_ImageInfo.arImageBytes, arImageBytes, sizeof(BYTE) * nImageNumBytes);
					eis->m_ImageInfo.nNumImageBytes = nImageNumBytes;
					SafeArrayUnaccessData(varBinary.parray);
				} else {
					// (r.gonet 05/02/2012) - PLID 49949 - There is no image.
					eis->m_ImageInfo.arImageBytes = NULL;
					eis->m_ImageInfo.nNumImageBytes = 0;
				}
			}
			eis->nCategoryID = AdoFldLong(rs, "CategoryID", -1); 
			eis->strCategoryName = sharedRef[AdoFldString(rs, "CategoryName", "")];

			m_aryEMRImageStamps.Add(eis);

			// (j.jones 2013-06-21 14:42) - PLID 57269 - added a map to look up stamps by ID
			m_mapEMRImageStamps.SetAt(eis->nID, eis);
			rs->MoveNext();
		}

		m_bEMRImageStampsLoaded = TRUE;

		rs = rs->NextRecordset(NULL);

		// (z.manning 2010-03-01 16:45) - PLID 37571 - Load actions
		// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
		MFCArray<EmrAction> aryActions;
		::FillActionArray(rs, GetRemoteData(), aryActions);
		for(int nActionIndex = 0; nActionIndex < aryActions.GetSize(); nActionIndex++) {
			EmrAction ea = aryActions.GetAt(nActionIndex);
			EMRImageStamp *pStamp = GetEMRImageStampByID(ea.nSourceID);
			// (j.jones 2013-07-23 16:39) - PLID 57269 - It is possible for the stamp
			// to not be found if orphan actions exist for missing stamps.
			// Don't throw an exception if the stamp doesn't exist, just skip it.
			if(pStamp != NULL) {
				pStamp->aryActions.Add(ea);
			}
		}

		rs->Close();
	}
}

// (j.jones 2010-02-10 11:33) - PLID 37224 - functions for loading and accessing EMR image stamps
void CMainFrame::ClearCachedEMRImageStamps()
{
	//throw exceptions to the caller

	// (j.jones 2013-06-21 14:42) - PLID 57269 - clear our map
	m_mapEMRImageStamps.RemoveAll();

	//clear the array
	for(int i=0;i<m_aryEMRImageStamps.GetSize();i++) {
		EMRImageStamp *eis = (EMRImageStamp*)m_aryEMRImageStamps.GetAt(i);
		// (r.gonet 05/02/2012) - PLID 49949 - Free the image memory.
		if(eis->m_ImageInfo.arImageBytes != NULL) {
			delete [] eis->m_ImageInfo.arImageBytes;
			eis->m_ImageInfo.arImageBytes = NULL;
			eis->m_ImageInfo.nNumImageBytes = 0;
		} else {
			// (r.gonet 05/02/2012) - PLID 49949 - No image to free
		}
		delete eis;
		eis = NULL;
	}
	m_aryEMRImageStamps.RemoveAll();

	m_bEMRImageStampsLoaded = FALSE;
}

// (j.jones 2010-02-10 11:33) - PLID 37224 - EditEMRImageStamps will return TRUE
// if anything was changed
// (a.walling 2012-03-12 10:06) - PLID 46648 - Dialogs must set a parent!
BOOL CMainFrame::EditEMRImageStamps(CWnd* pParent)
{
	try {

		CEMRImageStampSetupDlg dlg(pParent);
		dlg.DoModal();
		if(dlg.m_bChanged) {
			//the dialog should be responsible for clearing the existing cache,
			//as well as sending a tablechecker to tell others (and ourself) to do the same

			//the next time the stamps are needed they will be reloaded

			//if this is called from an EMR, it is the caller's responsibility to refresh
			//any and all loaded images with the current stamp list
			return TRUE;
		}
		else {
			return FALSE;
		}

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

// (j.jones 2010-02-16 15:01) - PLID 37224 - be able to return one stamp pointer by ID,
// returns NULL if no stamp exists with that ID
EMRImageStamp* CMainFrame::GetEMRImageStampByID(long nStampID)
{
	try {

		//load stamps if we have not already done so
		LoadEMRImageStamps();

		// (j.jones 2013-06-21 14:42) - PLID 57269 - added a map to look up stamps by ID
		EMRImageStamp *eis = NULL;
		if(m_mapEMRImageStamps.Lookup(nStampID, eis) && eis != NULL) {
			return eis;
		}

		//If we're still here, this stamp ID doesn't exist in our cached array,
		//which could possibly mean bad data, or a problem with caching.
		//We assume that in any case where a NULL result is possible, the caller
		//is equipped to handle that result.
		return NULL;

	}NxCatchAll(__FUNCTION__);

	return NULL;
}

// (z.manning 2010-03-01 17:14) - PLID 37571 - Function to get actions by stamp ID
void CMainFrame::GetEmrActionsByStampID(const long nStampID, BOOL bIncludeDeleted, OUT MFCArray<EmrAction> &aryActions)
{
	//load stamps if we have not already done so
	LoadEMRImageStamps();

	EMRImageStamp *pStamp = GetEMRImageStampByID(nStampID);
	if(pStamp != NULL) {
		for(int nActionIndex = 0; nActionIndex < pStamp->aryActions.GetSize(); nActionIndex++) {
			EmrAction ea = pStamp->aryActions.GetAt(nActionIndex);
			if(bIncludeDeleted || !ea.bDeleted) {
				aryActions.Add(ea);
			}
		}
	}
}

// (a.walling 2010-03-15 14:18) - PLID 37755 - Manual encryption key update requested
void CMainFrame::OnUpdateEncryptionKey()
{
	try {
		if (IDYES == MessageBox("Updating the encryption key requires updating all encrypted data within the database. This may take several minutes. "
			"This is automatically performed periodically by the NexTech Network Server and should only be manually performed in extreme situations.\r\n\r\n"
			"Are you sure you want to continue?", NULL, MB_ICONWARNING | MB_YESNO)) {
			IProgressDialog* pProgressDialog = NULL;
			HMODULE hMod = NULL;
			{
				CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_ALL, IID_IProgressDialog, (void **)&pProgressDialog);
				
				if (pProgressDialog) {
					pProgressDialog->SetTitle(_bstr_t("NexTech Practice"));
					pProgressDialog->SetCancelMsg(_bstr_t("This operation cannot be cancelled."), NULL);   // Will only be displayed if Cancel button is pressed.

					pProgressDialog->SetLine(1, _bstr_t("Updating encrypted data, please wait..."), FALSE, NULL);

					pProgressDialog->SetLine(2, _bstr_t("This process may take several minutes."), FALSE, NULL);
														
					#define PROGDLG_MARQUEEPROGRESS	0x00000020		// Use marquee progress (comctl32 v6 required)
					DWORD dwFlags = PROGDLG_NOTIME | PROGDLG_NOPROGRESSBAR | PROGDLG_MARQUEEPROGRESS;

					pProgressDialog->StartProgressDialog(NULL, NULL, dwFlags, NULL); // Display and enable automatic estimated time remaining.

					hMod = LoadLibrary("Shell32.dll");
					if (hMod) {
						pProgressDialog->SetAnimation(hMod, 166);
					}
				}
			}

			try {
				// (a.walling 2010-03-15 15:13) - PLID 37755 - Audit when manually performed
				long nOriginalKeyIndex = NxCryptosaur.GetLatestKeyIndex();				
				NxCryptosaur.CreateNewKey();
				NxCryptosaur.Load();
				AuditEvent(-1, "", BeginNewAuditEvent(), aeiManuallyUpdatedEncryptionKey, NxCryptosaur.GetLatestKeyIndex(), AsString(nOriginalKeyIndex), AsString((long)NxCryptosaur.GetLatestKeyIndex() ), aepMedium, aetChanged);
				NxCryptosaur.UpdateDataWithLatestKey();
			} catch(...) {
				if (pProgressDialog) {
					pProgressDialog->StopProgressDialog();
					pProgressDialog->Release();
					pProgressDialog = NULL;
				}
				if (hMod) {
					FreeLibrary(hMod);
					hMod = NULL;
				}

				throw;
			}
			
			if (pProgressDialog) {
				pProgressDialog->StopProgressDialog();
				pProgressDialog->Release();
				pProgressDialog = NULL;
			}
			if (hMod) {
				FreeLibrary(hMod);
				hMod = NULL;
			}

			MessageBox("All encrypted data has been updated successfully.", NULL, MB_ICONINFORMATION);
		}
	} NxCatchAll("Error updating encryption key!");
}

// (j.jones 2010-03-31 17:24) - PLID 37980 - This function will find an open EMR for a given patient,
// provided that the EMR has an EMN selected that is writeable, and has an open topic.
// It will return the PicContainer object if found.
// Will prompt if multiple qualifying EMRs are open for the same patient, using strMultiEMNPromptText
CPicContainerDlg* CMainFrame::GetOpenPatientEMR_WithWriteableTopic(long nPatientID, CString strMultiEMNPromptText)
{
	try {

		if(strMultiEMNPromptText.IsEmpty()) {
			strMultiEMNPromptText = "Select an EMN to use:";
		}

		CArray<CPicContainerDlg*, CPicContainerDlg*> aryMatchingPICs;

		if(!m_lstpPicContainerDlgs.IsEmpty()) {			
			POSITION pos = m_lstpPicContainerDlgs.GetHeadPosition();
			while(pos) {
				CPicContainerDlg *pPicContainer = m_lstpPicContainerDlgs.GetNext(pos);
				if(IsWindow(pPicContainer->GetSafeHwnd()) && pPicContainer->IsWindowVisible()) {
					if(pPicContainer->GetPatientID() == nPatientID
						&& pPicContainer->HasWriteableEMRTopicOpen()) {

						//this PicContainer is for our patient and has an EMR loaded
						aryMatchingPICs.Add(pPicContainer);
					}
				}
			}
		}

		if(aryMatchingPICs.GetSize() == 0) {
			return NULL;
		}
		else if(aryMatchingPICs.GetSize() == 1) {
			return (CPicContainerDlg*)aryMatchingPICs.GetAt(0);
		}
		else {
			//multiple matching PICs exist

			CSingleSelectDlg dlg(this);
			CString strSelectQ;
			for(int i=0; i<aryMatchingPICs.GetSize(); i++) {
				CPicContainerDlg *pPicContainer = (CPicContainerDlg*)aryMatchingPICs.GetAt(i);
				CEMN *pEMN = pPicContainer->GetActiveEMN();
				if(pEMN) {
					CString strEMNRecord;
					strEMNRecord.Format("SELECT %li AS ID, '%s' AS Description ",
						(long)pPicContainer, _Q(pEMN->GetDescription() + " - " + FormatDateTimeForInterface(pEMN->GetEMNDate(), NULL, dtoDate)));

					if(!strSelectQ.IsEmpty()) {
						strSelectQ += "UNION ALL ";
					}
					strSelectQ += strEMNRecord;
				}
			}

			//using the same location filter from earlier
			if(!strSelectQ.IsEmpty()) {
				CString strFrom;
				strFrom.Format("(%s) AS SelectQ", strSelectQ);
				if(IDOK == dlg.Open(strFrom, "", "ID", "Description", strMultiEMNPromptText, true)) {
					CPicContainerDlg *pPicContainer = (CPicContainerDlg*)dlg.GetSelectedID();
					return pPicContainer;
				}
			}
		}

	}NxCatchAll(__FUNCTION__);

	return NULL;
}

// (d.lange 2011-03-09 17:13) - PLID 41010 - Need to determine the patient ID who has an EMN open
void CMainFrame::GetOpenEMRPatientIDs(OUT CArray<long,long> &aryOpenEMRPatientIDs)
{
	try {
		if(!m_lstpPicContainerDlgs.IsEmpty()) {			
			POSITION pos = m_lstpPicContainerDlgs.GetHeadPosition();
			while(pos) {
				CPicContainerDlg *pPicContainer = m_lstpPicContainerDlgs.GetNext(pos);
				if(IsWindow(pPicContainer->GetSafeHwnd()) && pPicContainer->IsWindowVisible() && !pPicContainer->IsIconic()) {
					aryOpenEMRPatientIDs.Add(GetExistingPatientUserDefinedID(pPicContainer->GetPatientID()));
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-06-08 17:55) - PLID 38898 - Shows or hides the NexPhoto modeless import form
void CMainFrame::ShowNexPhotoImportForm(BOOL bShow)
{
	if (NULL == m_pNxPhotoImportForm) {
		m_pNxPhotoImportForm.CreateInstance(__uuidof(NxManagedWrapperLib::NxPhotoImportForm));
		if (NULL == m_pNxPhotoImportForm) {
			ThrowNxException("Could not instantiate the NexPhoto Import Form");
		}
		// (c.haag 2010-06-22 13:37) - PLID 39295 - Listen for NexPhoto import form events
		m_NxPhotoEventSink.EnsureSink(m_pNxPhotoImportForm);
	}
	if (bShow) {
		m_pNxPhotoImportForm->Show((OLE_HANDLE)GetSafeHwnd());
	}
	else {
		m_pNxPhotoImportForm->Hide();
	}
}

// (c.haag 2010-06-28 15:11) - PLID 39392 - Returns true if the NexPhoto Import window is open.
BOOL CMainFrame::IsNexPhotoImportFormOpen() const
{
	if (NULL == m_pNxPhotoImportForm) {
		return FALSE;
	}
	else {
		return (VARIANT_FALSE == m_pNxPhotoImportForm->Visible) ? FALSE : TRUE;
	}
}

// (j.jones 2010-12-22 10:45) - PLID 41911 - added financial close ability
void CMainFrame::OnPerformFinancialClose()
{
	try {

		//the menu item for this is already disabled if you don't have
		//permission, the permission is not actively checked any further
		//until you actually try to perform a close within this dialog
		CFinancialCloseDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-03-21 15:33) - PLID 42917 - added abilities to open shared folders on the server
void CMainFrame::OnViewOldEOBFiles()
{
	try {
		
		//calculate the server's EOB path
		CString strServerPath = GetNxConvertedEOBsPath();

		if(!DoesExist(strServerPath)) {
			ThrowNxException("The converted EOB path '%s' could not be found.", strServerPath);
			return;
		}

		//stupid ShellExecute often crashes in VS9 debug mode... lame
		ShellExecute(GetSafeHwnd(), NULL, strServerPath, NULL, NULL, SW_SHOWNORMAL);
	
	}NxCatchAll(__FUNCTION__);
}

void CMainFrame::OnViewOldEOBWarningsFiles()
{
	try {
		
		//calculate the server's log path
		CString strServerPath = GetNxEOBWarningLogsPath();

		if(!DoesExist(strServerPath)) {
			ThrowNxException("The EOB warning path '%s' could not be found.", strServerPath);
			return;
		}

		//stupid ShellExecute often crashes in VS9 debug mode... lame
		ShellExecute(GetSafeHwnd(), NULL, strServerPath, NULL, NULL, SW_SHOWNORMAL);
	
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-03-28 14:24) - PLID 43012 - added ability to mass-configure
// the billable status of service codes
void CMainFrame::OnEditNonbillableCPTCodes()
{
	try {
		
		if(!CheckCurrentUserPermissions(bioServiceCodes, sptWrite)) {
			return;
		}

		CBillableCPTCodesDlg dlg(this);
		// (j.jones 2012-08-08 10:11) - PLID 51063 - added check that the active view is non-null
		if(dlg.DoModal() == IDOK && GetActiveView()) {
			//if something changed, update the current view
			GetActiveView()->UpdateView();
		}
	
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2011-04-12 10:47) - PLID 41444 - cleaned up the function since now we dont care about continuing old exports
// (j.dinatale 2011-03-30 11:31) - PLID 42982 - Shows the modeless E-Statements Patient Select Window
// (a.walling 2013-08-09 09:59) - PLID 57948 - Always reload the EStatement patient selection info
// (c.haag 2016-05-19 14:08) - PLID-68687 - In the past the report SQL was generated, then run, and
// all of the patient ID's in the result set were passed into this function. After that, the report
// SQL was re-generated (identically to the original one) with a filter on the selected patients. Now
// we just take in the original report SQL and do all the work in this function where it can be done
// more efficiently.
void CMainFrame::ShowEStatementsPatientSelect(CReportInfo *pReport, BOOL bSummary, const CComplexReportQuery& reportQuery)
{
	// if we dont have one, create one
	if (!m_pEStatementPatientSelectDlg){
		m_pEStatementPatientSelectDlg = new CEStatementPatientSelectDlg(this);
		// (j.dinatale 2011-04-08 11:04) - PLID 42982 - need to have a parent of this, so the dialog doesnt vanish when the entire window is taskbared etc.
		m_pEStatementPatientSelectDlg->Create(IDD_ESTATEMENT_PATIENTSELECT, this);
		m_pEStatementPatientSelectDlg->CenterWindow();
	}

	// if the window is not visible
	//if(!m_pEStatementPatientSelectDlg->IsWindowVisible())
	{
		// (j.dinatale 2011-04-12 10:46) - PLID 41444 - no longer really care if the user wants to continue the last export,
		//		we maintain a cache of unselected patients so its better to just give them up to date information instead of prompting
		// (c.haag 2016-05-19 14:08) - PLID-68687 - We don't pass in the patient ID's anymore; instead we pass in the SQL that they came from
		m_pEStatementPatientSelectDlg->SetReport(pReport, bSummary, reportQuery);
		m_pEStatementPatientSelectDlg->ReloadPatientList();
	}

	// use SW_RESTORE so that way if the dialog is minimized, then pop it up
	// (j.dinatale 2011-04-08 11:04) - PLID 42982 - Use SW_SHOWNORMAL instead
	m_pEStatementPatientSelectDlg->ShowWindow(SW_SHOWNORMAL);
}

// (z.manning 2011-06-15 09:37) - PLID 44120
CFirstAvailList* CMainFrame::GetFirstAvailList()
{
	if(m_pdlgFirstAvailList == NULL)
	{
		m_pdlgFirstAvailList = new CFirstAvailList(this);
		if (!m_pdlgFirstAvailList->Create(IDD_FIRST_AVAIL_LIST, this)) {
			// (a.walling 2010-10-11 16:41) - PLID 36504
			ThrowNxException("Unable to create FFA list dialog!");
		}
	}

	return m_pdlgFirstAvailList;
}

// (a.walling 2011-06-22 11:59) - PLID 44260 - Acquire and hold a reference to an adobe acrobat object to prevent premature DLL unloading
void CMainFrame::HoldAdobeAcrobatReference()
{
	try {
		static bool bTriedOnce = false;

		if (bTriedOnce) {
			return;
		}

		// (a.walling 2011-09-13 14:42) - PLID 45468 - Always hold the reference
		if (GetPropertyInt("DontHoldAdobeAcrobatReference", FALSE)) {
			bTriedOnce = true;
			return;
		}

		if (!m_pHeldAdobeAcrobatReference) {
			bTriedOnce = true;

			try {
				HR(m_pHeldAdobeAcrobatReference.CreateInstance("AcroPDF.PDF"));
				//TES 1/16/2014 - PLID 59209 - Ignore any exceptions here, since we don't yet even know whether they actually want to display any .pdfs.
				// If they try to, and it fails, we will give them a clean message explaining that they need to install Adobe Reader.
			} NxCatchAllIgnore();
		}
	} NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-07-05 12:56) - PLID 44421
CEmrCodingGroupArray* CMainFrame::GetEmrCodingGroupManager()
{
	if(m_parypEmrCodingGroupManager == NULL)
	{
		m_parypEmrCodingGroupManager = new CEmrCodingGroupArray;
		m_parypEmrCodingGroupManager->ReloadAllGroups();
	}

	return m_parypEmrCodingGroupManager;
}

// (z.manning 2011-07-05 12:56) - PLID 44421
void CMainFrame::ClearCachedEmrCodingGroups()
{
	if(m_parypEmrCodingGroupManager == NULL) {
		return;
	}

	m_parypEmrCodingGroupManager->ClearAll();
	delete m_parypEmrCodingGroupManager;
	m_parypEmrCodingGroupManager = NULL;
}

// (b.savon 2011-10-04 12:01) - PLID 39890
void CMainFrame::OnMenuClickUnlinkHL7ThirdPartyID()
{
	CUnlinkHL7ThirdPartyID dlgUnlink(this);
	dlgUnlink.DoModal();
}

//(c.copits 2011-10-25) PLID 45709 - Deleting a supplier that is used in the Glasses Catalog Setup will generate a FK constraint error.
BOOL CMainFrame::CheckGlassesSupplierDesigns()
{
	BOOL bResult = TRUE;

	try {
		bResult = ReturnsRecords("SELECT ID FROM GlassesSupplierCatalogDesignsT WHERE SupplierID = %d", GetActiveContactID());
	} NxCatchAll(__FUNCTION__);

	return bResult;
}

BOOL CMainFrame::CheckGlassesSupplierMaterials()
{
	BOOL bResult = TRUE;

	try {
		bResult = ReturnsRecords("SELECT ID FROM GlassesSupplierCatalogMaterialsT WHERE SupplierID = %d", GetActiveContactID());
	} NxCatchAll(__FUNCTION__);

	return bResult;
}

BOOL CMainFrame::CheckGlassesSupplierTreatments()
{
	BOOL bResult = TRUE;

	try {
		bResult = ReturnsRecords("SELECT ID FROM GlassesSupplierCatalogTreatmentsT WHERE SupplierID = %d", GetActiveContactID());
	} NxCatchAll(__FUNCTION__);

	return bResult;
}

BOOL CMainFrame::CheckGlassesSupplierFrames()
{
	BOOL bResult = TRUE;

	try {
		bResult = ReturnsRecords("SELECT ID FROM GlassesSupplierCatalogFrameTypesT WHERE SupplierID = %d", GetActiveContactID());
	} NxCatchAll(__FUNCTION__);
	
	return bResult;

}

// (j.gruber 2011-11-01 10:00) - PLID 46219 - make cchit reports dlg modeless
void CMainFrame::ShowCCHITReportsDlg()
{
	try {
		// if we dont have one, create one
		if (!m_pCCHITReportsDlg){
			m_pCCHITReportsDlg = new CCCHITReportsDlg(this);
			// (j.dinatale 2011-04-08 11:04) - PLID 42982 - need to have a parent of this, so the dialog doesnt vanish when the entire window is taskbared etc.
			m_pCCHITReportsDlg->Create(IDD_CCHIT_REPORTS_DLG, this);
			m_pCCHITReportsDlg->CenterWindow();
		}	
		
		m_pCCHITReportsDlg->ShowWindow(SW_SHOWNORMAL);
	}NxCatchAll(__FUNCTION__);

}

// (r.goldschmidt 2014-10-08 16:44) - PLID 62644 - make eligibility review dialog modeless
void CMainFrame::ShowEligibilityReviewDlg()
{
	try {
		if (!m_pEligibilityReviewDlg){ // if we dont have one, create one
			m_pEligibilityReviewDlg = new CEligibilityReviewDlg(this);
			m_pEligibilityReviewDlg->Create(IDD_ELIGIBILITY_REVIEW_DLG);
			m_pEligibilityReviewDlg->CenterWindow();
		}
		else { // if we have one, update the datalist
			m_pEligibilityReviewDlg->m_EligibilityList->Requery();
		}
		m_pEligibilityReviewDlg->ShowWindow(SW_SHOWNORMAL); 
	}NxCatchAll(__FUNCTION__);
}

//(J.camacho 2016-01-27) PLID 68001
void CMainFrame::ShowHL7ToBeBilledDlg()
{
	try {

		if (m_HL7ToBeBilledDlg.GetSafeHwnd() == NULL) { // if we dont have one, create one
			m_HL7ToBeBilledDlg.Create(IDD_HL7_TO_BE_BILLED_DLG, GetMainFrame());
			m_HL7ToBeBilledDlg.CenterWindow();
			m_HL7ToBeBilledDlg.ShowWindow(SW_SHOWNORMAL);
		}
		else {
			m_HL7ToBeBilledDlg.CenterWindow();
			m_HL7ToBeBilledDlg.ShowWindow(SW_SHOWNORMAL);
		}
	}NxCatchAll(__FUNCTION__);
}

//this version takes in just one request to show
void CMainFrame::ShowEligibilityRequestDetailDlg(long nRequestID)
{
	try {

		std::vector<long> aryRequestIDsUpdated;
		aryRequestIDsUpdated.push_back(nRequestID);
		//this overload does not require response filtering
		std::vector<long> aryResponseIDsReturned;
		ShowEligibilityRequestDetailDlg(aryRequestIDsUpdated, aryResponseIDsReturned);

	}NxCatchAll(__FUNCTION__);
}

//this always takes in a list of request IDs to show, and optionally a list of response IDs,
//the response IDs are only filled if we want to filter on specific responses
void CMainFrame::ShowEligibilityRequestDetailDlg(std::vector<long> &aryRequestIDsUpdated, bool bSilentlyCloseExistingDialog /* = false*/)
{
	try {

		//this overload does not require response filtering
		std::vector<long> aryResponseIDsReturned;
		ShowEligibilityRequestDetailDlg(aryRequestIDsUpdated, aryResponseIDsReturned, bSilentlyCloseExistingDialog);

	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2014-10-10 16:24) - PLID 62644 - make eligibility request detail dialog modeless
// (r.goldschmidt 2015-11-12 12:51) - PLID 65363 - add option to reset the modeless dialogs and reopen silently
//this always takes in a list of request IDs to show, and optionally a list of response IDs,
//the response IDs are only filled if we want to filter on specific responses
void CMainFrame::ShowEligibilityRequestDetailDlg(std::vector<long> &aryRequestIDsUpdated, std::vector<long> &aryResponseIDsReturned, bool bSilentlyCloseExistingDialog /* = false*/)
{
	try {
		// Don't allow modeless eligibility request dialog and eligibility request detail dialog at same time
		if (m_pEligibilityRequestDlg) {
			if (bSilentlyCloseExistingDialog) {
				// just escape from this dialog so the newly opened detail dialog isn't thought to be related to this dialog
				PostMessage(NXM_ELIGIBILITYREQUEST_CLOSED, IDCANCEL, (LPARAM)-1L);
			}
			else {
				//bring back loaded dialog
				BringDialogToFront(m_pEligibilityRequestDlg);
				m_pEligibilityRequestDlg->MessageBox("Please finish editing this eligibility request before reviewing a request detail.", "Practice", MB_ICONSTOP);
				return;
			}
		}

		// Make sure we want to reload with new info if there is already an open window with info
		if (m_pEligibilityRequestDetailDlg){
			if (bSilentlyCloseExistingDialog) {
				//close the review dialog right now, wait for it to finish closing
				SendMessage(NXM_ELIGIBILITYREQUEST_DETAIL_CLOSED, IDCANCEL, (LPARAM)-1L);
			}
			else {
				BringDialogToFront(m_pEligibilityRequestDetailDlg);
				m_pEligibilityRequestDetailDlg->MessageBox("Please close the this eligibility review dialog before reviewing another request detail.", "Practice", MB_ICONSTOP);
				return;
			}
		}
		
		//recreate the dialog
		ASSERT(m_pEligibilityRequestDetailDlg == NULL);
		m_pEligibilityRequestDetailDlg = new CEligibilityRequestDetailDlg(this);
		m_pEligibilityRequestDetailDlg->Create(IDD_ELIGIBILITY_REQUEST_DETAIL_DLG);
		m_pEligibilityRequestDetailDlg->CenterWindow();

		// reset the requested details
		m_pEligibilityRequestDetailDlg->m_aryAllRequestIDs.clear();
		for (int i = 0; i<(long)aryRequestIDsUpdated.size(); i++) {
			m_pEligibilityRequestDetailDlg->m_aryAllRequestIDs.push_back((long)aryRequestIDsUpdated.at(i));
		}
		// and responses, if any
		m_pEligibilityRequestDetailDlg->m_aryAllResponseIDs.clear();
		for (int i = 0; i<(long)aryResponseIDsReturned.size(); i++) {
			m_pEligibilityRequestDetailDlg->m_aryAllResponseIDs.push_back((long)aryResponseIDsReturned.at(i));
		}

		// reload the dialog and ensure controls
		m_pEligibilityRequestDetailDlg->EnsureControls();
		m_pEligibilityRequestDetailDlg->ShowWindow(SW_SHOWNORMAL);
		BringDialogToFront(m_pEligibilityRequestDetailDlg);

	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2014-10-15 14:11) - PLID 62644 - called after eligibility request detail is closed
void CMainFrame::DestroyEligibilityRequestDetailDlg()
{
	try {

		if (m_pEligibilityRequestDetailDlg) {
//			if (m_pEligibilityRequestDetailDlg->m_hWnd) {
				m_pEligibilityRequestDetailDlg->DestroyWindow();
//			}
			delete m_pEligibilityRequestDetailDlg;
			m_pEligibilityRequestDetailDlg = NULL;
		}

	}NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-1-10) PLID 47517 - to initialize the barcode scanner thread.
LRESULT CMainFrame::OnInitiateOPOSBarcodeScanner(WPARAM wParam, LPARAM lParam)
{
	try {
		if (m_pOPOSBarcodeScannerThread) {
			m_pOPOSBarcodeScannerThread->Shutdown();
			m_pOPOSBarcodeScannerThread = NULL;
		}

		// (j.jones 2013-05-15 13:54) - PLID 25479 - converted these settings to per user, per workstation,
		// pulling the defaults from their old per-workstation value
		if (m_pOPOSBarcodeScannerThread == NULL &&
			GetRemotePropertyInt("UseOposBarcodeScanner_UserWS", GetPropertyInt("UseOposBarcodeScanner", 0, 0), 0, GetCurrentUserComputerName(), true) == TRUE) {

			long nTimeout = GetRemotePropertyInt("OposBarcodeScannerTimeout_UserWS", GetPropertyInt("OposBarcodeScannerTimeout", 10, 0), 0, GetCurrentUserComputerName(), true) * 1000;
			m_pOPOSBarcodeScannerThread = new COPOSBarcodeScannerThread(GetSafeHwnd(), 
				GetRemotePropertyText("OposBarcodeScannerName_UserWS", GetPropertyText("OposBarcodeScannerName", "", 0), 0, GetCurrentUserComputerName(), true),
				nTimeout);
			m_pOPOSBarcodeScannerThread->CreateThread();
			m_bOPOSBarcodeScannerInitComplete = false;
			SetTimer(IDT_OPOS_BARSCAN_INIT_TIMER, nTimeout, NULL);	//set timer for timeout
		}

	}NxCatchAll(__FUNCTION__);

	return 0;
}
//(a.wilson 2012-1-10) PLID 47517 - checks scanner events and handles them appropriately.
void CMainFrame::OnOPOSBarcodeScannerEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	try {
		switch(message) {
			case NXM_BARSCAN_DATA_EVENT:
				//check to ensure its a 2d barcode
				if ((long)wParam == (long)OPOS_SUCCESS) {
					//send the message to the appropriate dialog
					if (GetActiveWindow() && GetActiveWindow() != GetMainFrame()) {
						GetActiveWindow()->SendMessage(message, wParam, lParam);
					} else if (GetActiveView()) {
						GetActiveView()->SendMessage(message, wParam, lParam);
					}
				}
				break;
			case NXM_BARSCAN_INIT_COMPLETE:	//event to tell the main thread about initialization success or issues.
				if ((long)wParam == OPOS_SUCCESS) {
					m_bOPOSBarcodeScannerInitComplete = true;
					KillTimer(IDT_OPOS_BARSCAN_INIT_TIMER);
				} else {
					CString strError = "";
					KillTimer(IDT_OPOS_BARSCAN_INIT_TIMER);
					strError = FormatString("\r\n\r\nError: %s", OPOS::GetMessage((long)wParam));
					MessageBox(FormatString("The OPOS barcode scanner device could not be initialized.  Make sure that the settings are correct "
						   "by going to Tools->POS Tools->Barcode Scanner Settings.  Ensure that the device has been "
						   "correctly installed and that nothing else is currently using the barcode scanner device.%s", strError), 
						   "Barcode Scanner Device Error", MB_ICONWARNING|MB_OK);
					if (m_pOPOSBarcodeScannerThread) {
						m_pOPOSBarcodeScannerThread->Shutdown();
						m_pOPOSBarcodeScannerThread = NULL;
					}
				}
				break;
		}
	} NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-11-1) PLID 47517 - check to see if the barcode scanner is active.
BOOL CMainFrame::DoesOPOSBarcodeScannerExist()
{
	try {
		if (m_pOPOSBarcodeScannerThread && m_bOPOSBarcodeScannerInitComplete)
			return TRUE;

	}NxCatchAll("Error in CMainFrame::DoesOPOSBarcodeScannerExist");
	// It doesn't exist
	return FALSE;
}

// (d.singleton 2012-04-02 17:06) - PLID 49336 pop up the code correct setup dialog
void CMainFrame::OnMenuClickCodeCorrectSetup()
{
	try {
	CCodeCorrectSetupDlg dlg; 
	dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}
// (j.dinatale 2012-03-21 14:53) - PLID 49079 - allocate a new contact lens order form
CContactLensOrderForm* CMainFrame::AllocateContactLensOrderDlg(CWnd* pParentWnd, long nOrderID)
{
	CContactLensOrderForm* pDlg = new CContactLensOrderForm(nOrderID, pParentWnd);
	m_lstpContactLensOrderForms.AddTail(pDlg);
	return pDlg;
}

// (j.dinatale 2012-03-21 11:41) - PLID 49079 - open a contact lens order form
int CMainFrame::OpenContactLensOrderForm(CWnd* pParentWnd, long nOrderID)
{
		if (nOrderID > -1 && !m_lstpContactLensOrderForms.IsEmpty()) {
			POSITION pos = m_lstpContactLensOrderForms.GetHeadPosition();
			// Traverse the list looking for the one we want to open
			while (pos) {
				CContactLensOrderForm *pDlg = m_lstpContactLensOrderForms.GetNext(pos);

				if (IsWindow(pDlg->GetSafeHwnd()) && pDlg->IsWindowVisible() &&
					pDlg->GetOrderID() == nOrderID) 
				{
					// We found it. Here are the possible combinations we need to handle:
					//
					// pDlg is modeless: Just bring it to the front.
					// pDlg is modal: Just bring it to the front (I don't see how this could ever happen 
					//	because the order window should never be modal)
					//
					BringDialogToFront(pDlg);
					// Break out of the loop and this function by telling our caller that it activated
					// an existing window
					return IDOK;
				}
			}
		}

		CContactLensOrderForm &dlg = *AllocateContactLensOrderDlg(pParentWnd, nOrderID);
		try
		{
			dlg.Create(CContactLensOrderForm::IDD);
			dlg.ModifyStyle(0, WS_MINIMIZEBOX);
			dlg.ShowWindow(SW_SHOW);
			return IDOK;
		}
		catch (...) 
		{
			delete &dlg;
			throw;
		}
}

// (j.luckoski 2012-04-10 17:16) - PLID 49491 - Show the specialty travel editor dialog.
void CMainFrame::OnMenuClickSpecialtyDlg() {
	try {
		if(GetCurrentUserPermissions(bioSpecialtyList, sptWrite)) {
		CSpecialtyConfigDlg dlg;
		dlg.DoModal();
	} else {
		AfxMessageBox("You don't have permission to access this function! Please see your administrator for help.");
	}
	} NxCatchAll(__FUNCTION__);
}

// (s.dhole 07/23/2012) PLID 48693 Function to load dialog
void CMainFrame::ShowEligibilityRequestDlg(CWnd* pParentWnd, long nID ,long nFormatID,long nDefaultInsuredPartyID,OLE_COLOR clrBackground  )
{
	try {
		//Check if dialog is loaded 
		if (m_pEligibilityRequestDlg) {
			//bring back loaded dialog
			BringDialogToFront(m_pEligibilityRequestDlg);
			m_pEligibilityRequestDlg->MessageBox("Please finish editing this eligibility request before continuing.", NULL, MB_ICONSTOP);
		}
		// (r.goldschmidt 2014-10-14 15:46) - PLID 62644 - don't allow modeless eligibility request dialog and eligibility request detail dialog at same time
		else if (m_pEligibilityRequestDetailDlg){
		//else if (m_pEligibilityRequestDetailDlg && IsWindow(m_pEligibilityRequestDetailDlg->GetSafeHwnd()) && m_pEligibilityRequestDetailDlg->IsWindowVisible()){
			//bring back eligibility request detail dialog
			BringDialogToFront(m_pEligibilityRequestDetailDlg);
			m_pEligibilityRequestDetailDlg->MessageBox("Please finish reviewing this eligibility request detail before creating or editing another eligibility request.", NULL, MB_ICONSTOP);
		}
		else{
			//Create new 
			m_pEligibilityRequestDlg = new CEligibilityRequestDlg(pParentWnd);
			m_pEligibilityRequestDlg->m_nID =nID;
			m_pEligibilityRequestDlg->m_nFormatID = nFormatID;
			m_pEligibilityRequestDlg->m_nDefaultInsuredPartyID = nDefaultInsuredPartyID;
			
			m_pEligibilityRequestDlg->m_bModeless = TRUE;
			// if color value is not set then do not overide back color
			if (clrBackground !=0){
				m_pEligibilityRequestDlg->m_clrBackground = clrBackground;
			}
			m_pEligibilityRequestDlg->Create(CEligibilityRequestDlg::IDD);
			m_pEligibilityRequestDlg->ShowWindow(SW_SHOW);
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-08-13 15:47) - PLID 51941
void CMainFrame::OnOMRProcessing()
{
	try{
		if (m_OMRProcessingDlg.GetSafeHwnd() == NULL) {
			m_OMRProcessingDlg.Create(COMRScanDlg::IDD, GetMainFrame());
			m_OMRProcessingDlg.CenterWindow();
			m_OMRProcessingDlg.ShowWindow(SW_SHOWNORMAL);
		}
		else {
			m_OMRProcessingDlg.ShowWindow(SW_SHOWNORMAL);
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, August 31, 2012) - PLID 51928 - Launch the dialog from the menu. 
void CMainFrame::OnRemarkOMRFormEditting()
{
	try {
		CEMROMRMapperDlg dlg;
		dlg.DoModal(); 
	}
	NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-10 16:46) - PLID 54567 - Add a public facing
void CMainFrame::ShowPrescriptionsNeedingAttention()
{
	OnPrescriptionsNeedingAttention();
}

// (j.fouts 2012-12-31 15:11) - PLID 53156 - Added a a way to open Prescriptions Needing Attention
void CMainFrame::OnPrescriptionsNeedingAttention()
{
	try
	{
		// (j.jones 2016-02-03 16:46) - PLID 68118 - this is Rx Needing Attention only, not renewals
		OpenPrescriptionsRenewalsNeedingAttention(false);
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-02-06 09:41) - PLID 54472 - Open the needing attention with the option to show the messages that the user was notified for
// (j.jones 2016-02-03 16:23) - PLID 68118 - repurposed the parameter to state when we want to see renewals needing attention,
// otherwise we want prescriptions needing attention
void CMainFrame::OpenPrescriptionsRenewalsNeedingAttention(bool bShowRenewalsNeedingAttention)
{
	// (b.savon 2014-01-14 07:40) - PLID 59628 - The permission for Rx Needing Attention is incorrectly under the
	// permission for "Edit Medication List".  It should be "Read"
	if(!CheckCurrentUserPermissions(bioPatientMedication, sptRead)) {
		return;
	}

	// (a.wilson 2013-01-09 14:00) - PLID 54535 - insert the queue into a modeless parent dialog.
	if (!m_pPrescriptionQueueParentDlg) {
		m_pPrescriptionQueueParentDlg.reset(new CNxModelessParentDlg(this, CString()));
		m_pPrescriptionQueueParentDlg->CreateWithChildDialog(std::make_unique<CPrescriptionQueueDlg>(nullptr), IDD_PRESCRIPTION_QUEUE, 
			IDI_SCRIPT_ATTENTION, "Prescriptions Needing Attention", this);
	}
	
	CPrescriptionQueueDlg* pQueueDlg = dynamic_cast<CPrescriptionQueueDlg*>(m_pPrescriptionQueueParentDlg->m_pChild.get());

	// (j.jones 2016-02-03 16:27) - PLID 68118 - now we explicitly tell the dialog whether to load
	// renewals or prescriptions needing attention
	if(pQueueDlg)
	{
		pQueueDlg->LoadFromNotificationClick(bShowRenewalsNeedingAttention);
	}

	// (j.fouts 2012-12-31 15:11) - PLID 53156 - Restore if the window is minimized
	m_pPrescriptionQueueParentDlg->ShowWindow(SW_RESTORE);
	m_pPrescriptionQueueParentDlg->BringWindowToTop();
}

// (j.jones 2012-09-25 12:01) - PLID 37269 - added OnSize
void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	try {

		CNxMDIFrameWnd::OnSize(nType, cx, cy);

		// (j.jones 2012-09-25 12:04) - PLID 37269 - if the app. width is greater than 1000px, look to see if
		// the GUI toolbar is on a separate row, and if it is, move it back to the top row
		CRect rWindow;
		GetWindowRect(rWindow);
		if(rWindow.Width() >= 1000 && m_wndToolBar.GetSafeHwnd() && m_GUIToolBar.GetSafeHwnd()) {
			CRect rcMainToolbar, rcGUIToolbar;
			m_wndToolBar.GetToolBarCtrl().GetWindowRect(rcMainToolbar);
			m_GUIToolBar.GetToolBarCtrl().GetWindowRect(rcGUIToolbar);
			if(rcMainToolbar.top < rcGUIToolbar.top) {
				//the GUI toolbar is below the main toolbar, and it should not be
				DockControlBarLeftOf(&m_GUIToolBar, &m_wndToolBar);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-10-31 12:08) - PLID 53502 - MU detailed report
void CMainFrame::ShowMUDetailedReport()
{
	try{
		//Use EMR Read permission, as most of these statistics come out of EMR
		if(!CheckCurrentUserPermissions(bioPatientEMR, sptRead)) {
			return;
		}

		// (j.armen 2013-08-02 14:55) - PLID 57803 - This is a smart pointer now
		if(!m_pMUDetailedReport)
			m_pMUDetailedReport.reset(new CMUDetailedReportDlg(this));

		if (!m_pMUDetailedReport->GetSafeHwnd()) {
			m_pMUDetailedReport->Create(CMUDetailedReportDlg::IDD, GetMainFrame());
			m_pMUDetailedReport->CenterWindow();
		}

		m_pMUDetailedReport->ShowWindow(SW_SHOW);

	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2013-02-05 12:24) - PLID 55019 - Mainfrm needs to coordinate with owned CNxModelessDialog windows to prevent owner activation.
void CMainFrame::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	try {
		if (!(lpwndpos->flags & SWP_NOZORDER)) {
			// only deal with this if the active window is a descendant of the mainframe
			HWND hwnd = ::GetActiveWindow();
			if (hwnd && WindowUtils::IsDescendantDirect(*this, hwnd)) {
				// now, walk the hierarchy of owned windows from the active window to mainframe.
				// if we encounter a CNxModelessOwnedDialog, then we should prevent z-order changes.
				do
				{
					if (dynamic_cast<CNxModelessOwnedDialog*>(CWnd::FromHandlePermanent(hwnd))) {
						lpwndpos->flags |= SWP_NOZORDER;
						break;
					}

					hwnd = WindowUtils::GetParentOwnerDirect(hwnd);

				} while (hwnd && hwnd != *this);
			}
		}
	} NxCatchAllIgnore();

	__super::OnWindowPosChanging(lpwndpos);
}

// (a.walling 2014-06-09 09:37) - PLID 57388 - Placeholders - useful for debugging without rebuilding everything
void CMainFrame::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized)
{
	__super::OnActivate(nState, pWndOther, bMinimized);
}

void CMainFrame::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
	__super::OnActivateApp(bActive, dwThreadID);
}

LRESULT CMainFrame::OnActivateTopLevel(WPARAM wParam, LPARAM lParam)
{
	return __super::OnActivateTopLevel(wParam, lParam);
}

// (d.lange 2010-06-07 23:52) - PLID 38850 - We will show the DevicePluginImportDlg when the folder monitoring is triggered
void CMainFrame::ShowDevicePluginImportDlg()
{
	DeviceImport::GetMonitor().ShowDevicePluginImportDlg();
}

long CMainFrame::GetDeviceImportRecordCount()
{
	return DeviceImport::GetMonitor().GetDeviceImportRecordCount();
}

void CMainFrame::NotifyDeviceImportPatientChanged()
{
	DeviceImport::GetMonitor().NotifyDeviceImportPatientChanged();
}

CList<CPicContainerDlg *, CPicContainerDlg *>& CMainFrame::GetPicContainers()
{
	return m_lstpPicContainerDlgs;
}

//(d.singleton 2013-01-05 15:58) - PLID 56520 - need to get the com port the sig pad is installed on and store it
void CMainFrame::OnTopazSigPadSettings()
{
	try {		
		CTopazSigPadSettingsDlg dlg;
		//return if canceled or turned off
		// (b.spivey, May 03, 2013) - PLID 56542 - If the status is 0, we're not connected. 
		if(dlg.DoModal() == IDCANCEL) {
			return;
		}
		else if (dlg.m_nCurrentStatus == 0) {
			m_bIsTopazConnected = FALSE;
			return;
		}		
		long nComPort = dlg.m_nCurrentPort;					
		//test the com port to make sure we connect successfully
		// (a.walling 2014-07-28 10:36) - PLID 62825 - Use TopazSigPad::IsTabletConnected
			m_bIsTopazConnected = FALSE;
		if (!TopazSigPad::IsTabletConnected(this, nComPort)) {
			AfxMessageBox("Practice was unable to connect to your Topaz Signature Pad.");
		}
		else {
			m_bIsTopazConnected = TRUE;
			MessageBox("Practice has connected to your Topaz Signature Pad.", "Practice", MB_OK|MB_ICONINFORMATION);
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, May 02, 2013) - PLID 56542 - Accessor
BOOL CMainFrame::GetIsTopazConnected() 
{
	return m_bIsTopazConnected;
}

// (b.spivey, May 02, 2013) - PLID 56542 - Mutator
void CMainFrame::SetIsTopazConnected(BOOL bIsConnected) 
{
	m_bIsTopazConnected = bIsConnected; 
}

// (b.spivey, May 06, 2013) - PLID 56542 - event handler to check topaz connection. 
LRESULT CMainFrame::OnInitiateTopazSigPadDevice(WPARAM wParam, LPARAM lParam)
{
	try {
		// (b.spivey, May 02, 2013) - PLID 56542 - This should fire when practice first starts, so we check if the sigpad is on
		//	  if it is, then we get the comport and see if we can connect to it. 
		if (GetPropertyInt("IsTopazSigPadOn", FALSE, 0, true)) {
			long nComPort = GetPropertyInt("TopazSigPadPortNumber", 0, 0, true); //Get This from the database. 
			// (a.walling 2014-07-28 10:36) - PLID 62825 - Use TopazSigPad::IsTabletConnected
			m_bIsTopazConnected = FALSE;
			if (TopazSigPad::IsTabletConnected(this, nComPort)) {
				m_bIsTopazConnected = TRUE;
			}
			else {
				AfxMessageBox("Practice could not connect to your Topaz Signature Pad device. Check your settings and try again.", MB_OK|MB_ICONWARNING);
			}
		}
		else {
			m_bIsTopazConnected = FALSE;
		}
	} NxCatchAll(__FUNCTION__);
	return 0;
}

//TES 10/31/2013 - PLID 59251 - Notify the user that the interventions identified in the array (DecisionRuleInterventionT.IDs) have been triggered
void CMainFrame::DisplayNewInterventions(const CDWordArray &arInterventionIDs)
{
	if(arInterventionIDs.GetCount() == 0) {
		return;
	}

	m_arPendingInterventionIDs.Append(arInterventionIDs);

	//TES 10/31/2013 - PLID 59276 - Call the Notify architecture
	NotifyUser(NT_CDS_INTERVENTION, "");
}

//TES 11/1/2013 - PLID 59276 - Displays all pending interventions, and then clears out the list
void CMainFrame::OpenInterventionsList()
{
	try {
		if(m_arPendingInterventionIDs.GetCount() == 0) {
			return;
		}

		CCDSInterventionListDlg dlg;
		dlg.OpenWithNewInterventions(m_arPendingInterventionIDs);
		m_arPendingInterventionIDs.RemoveAll();
		

	}NxCatchAll(__FUNCTION__);
}

//(b.spivey, December 13th, 2013) - PLID 59022 - Function to clean up data based on contact type. 
void CMainFrame::UpdateDirectMessageAddressData(long nID, long nStatus) 
{
	CSqlFragment sql = ""; 
	bool bUpdate = false; 
	switch(nStatus)
	{
		case 0: //Other
		case 8: //Supplier
			sql += "DELETE FROM DirectAddressUserT WHERE DirectAddressFromID = @DirectAddressID "
				"DELETE FROM DirectAddressFromT WHERE PersonID = @PersonID "; 
			bUpdate = true; 
			break;
		case 1: //Referring Phys. 
		case 4: //User
			
			sql += "DELETE FROM DirectAddressUserT WHERE DirectAddressFromID = @DirectAddressID ";
			bUpdate = true; 
			break;

		case 2: //Provider
		default:
			//Do nothing. 
			break;
	}

	if(bUpdate) {
		ExecuteParamSql(
				"DECLARE @PersonID INT " 
				"DECLARE @DirectAddressID INT " 
				"	"
				"SET @PersonID = {INT} "
				"SET @DirectAddressID = (SELECT ID FROM DirectAddressFromT WHERE PersonID = @PersonID) "
				"	"
				"{SQL} ",
				nID, sql); 
	}
}

void CMainFrame::OnViewCdsInterventions()
{
	try {
		//TES 1/13/2014 - PLID 59871 - Just pop up the interventions dialog for the current patient.
		CCDSInterventionListDlg dlg;
		dlg.OpenWithPatientInterventions(GetActivePatientID(), GetActivePatientName());
	}NxCatchAll(__FUNCTION__);
}

// (d.thompson 2014-02-04) - PLID 60638
CDiagSearchConfig* CMainFrame::GetDiagPreferenceSearchConfig()
{
	return m_pDiagPreferenceSearchConfig;
}

// (d.thompson 2014-02-04) - PLID 60638
CDiagSearchConfig* CMainFrame::GetDiagDualSearchConfig()
{
	return m_pDiagDualSearchConfig;
}

// (d.thompson 2014-03-07) - PLID 60638 - Calling this will completely reset all the diag search configuration
//	settings.  Typically this should be used when switching users and logging in a new one only.
void CMainFrame::ResetDiagSearchSettings()
{
	if(m_pDiagPreferenceSearchConfig) {
		delete m_pDiagPreferenceSearchConfig;
		m_pDiagPreferenceSearchConfig = NULL;
	}
	//NOTE:  Developers should never query this preference directly.  It really should only exist here and nowhere
	//	else.  Please use DiagSearchUtils::GetPreferenceSearchStyle() to determine the current search option.
	// (d.thompson 2014-03-07) - PLID 60638 - We changed our mind and made this global.  I renamed it to avoid confusing
	//	data internally from DiagSearchDisplayOption to DiagGlobalSearchDisplayOption.  We also changed the default to ICD-9.
	// (b.eyers 2015-02-05) - PLID 64819 - changed default from icd9 to nexgems
	long nPreferenceOption = GetRemotePropertyInt("DiagGlobalSearchDisplayOption", 1, 0, "<None>", true);

	// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
	bool bIncludePCS = GetRemotePropertyInt("DiagGlobalSearchDisplayOption_IncludePCS", 0, 0, "<None>", true) == 1 ? true : false;

	switch(nPreferenceOption) {
		case 0:
			//ICD9 search type
			//(s.dhole 3/6/2015 10:42 AM ) - PLID 64549
			m_pDiagPreferenceSearchConfig = dynamic_cast<CDiagSearchConfig *>(new CDiagSearchConfig_ManagedICD9Search(bIncludePCS));
			break;
		case 1:
			//Crosswalk type
			//(s.dhole 3/6/2015 10:42 AM ) - PLID 64549
			m_pDiagPreferenceSearchConfig = dynamic_cast<CDiagSearchConfig *>(new CDiagSearchConfig_Crosswalk(bIncludePCS));
			break;
		case 2:
			//ICD10 search type
			//(s.dhole 3/6/2015 10:42 AM ) - PLID 64549
			m_pDiagPreferenceSearchConfig = dynamic_cast<CDiagSearchConfig *>(new CDiagSearchConfig_ManagedICD10Search(bIncludePCS));
			break;
		default:
			ThrowNxException("Unknown diag search preference!");
			break;
	}

	// (d.thompson 2014-02-07) - Always build the dual search as well, it's not part of the preference options.
	// (d.thompson 2014-03-03) - If we're switching user, there's no reason to rebuild this.
	// (d.thompson 2014-03-10) - Now that we have the PCS preference, which could change, we need to rebuild
	if(m_pDiagDualSearchConfig) {
		delete m_pDiagDualSearchConfig;
		m_pDiagDualSearchConfig = NULL;
	}
	//(s.dhole 3/6/2015 10:42 AM ) - PLID 64549
	m_pDiagDualSearchConfig = dynamic_cast<CDiagSearchConfig *>(new CDiagSearchConfig_FullICD9or10Search(bIncludePCS));
}

//(s.dhole 12/4/2014 10:58 AM ) - PLID 64337 Call Mass Assign Security Codes Dialog
void CMainFrame::OnMenuClickedMassAssignSecurityCodes()
{
	try {

		if (!g_pLicense->CheckForLicense(CLicense::lcNexWebPortal, CLicense::cflrSilent))
		{
			AfxMessageBox("You do not have permission to access this function. Please see your office manager for assistance.", MB_OK | MB_ICONWARNING);
			return;
		}
		if (CheckCurrentUserPermissions(bioPatientNexWebLogin, sptDynamic1)) {
			CMassAssignSecurityCodesDlg dlg(this);
			dlg.DoModal();
		}
		else {
		// nothing	
		}
		
	}NxCatchAll(__FUNCTION__);
	
}

void CMainFrame::OnEnableDebugMode()
{
	extern CPracticeApp theApp;
	if (theApp.GetDebugModeEnabled()) {
		//TES 3/2/2015 - PLID 64737 - Turn off debug mode
		theApp.SetDebugMode(FALSE);
		//TES 3/2/2015 - PLID 64737 - If that didn't work, notify the user
		if (theApp.GetDebugModeActive()) {
			AfxMessageBox("This change will take effect after restarting Practice.");
		}
	}
	else {
		//TES 3/2/2015 - PLID 64736 - Turn on debug mode
		theApp.SetDebugMode(TRUE);
		//TES 3/2/2015 - PLID 64736 - GetDebugModeActive() won't return TRUE if we call it immediately, so check in a second to see if it worked
		SetTimer(IDT_CHECK_DEBUG_MODE_TIMER, 1000, NULL);
	}
}

// (z.manning 2015-03-13 15:35) - NX-100432 - Function to get the tab that should be used when clicking the EMR toolbar icon
PatientsModule::Tab CMainFrame::GetPrimaryEmrTab()
{
	int nNexEMRIconPref = GetRemotePropertyInt("NexEMRIconTabToOpen", 1, 0, GetCurrentUserName());
	switch (nNexEMRIconPref)
	{
		case 2:
			return PatientsModule::PatientDashboard;
			break;

		default:
			return PatientsModule::NexEMRTab;
			break;
	}
}

// (d.lange 2015-07-27 09:04) - PLID 67188
void CMainFrame::OnICCPSettings()
{
	try{
		CICCPSetupDlg dlg(this);
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-08-04 15:00) - PLID 67221
void CMainFrame::EnsureICCPDeviceManager()
{
	EnsureICCPDeviceManager(FALSE);
}

// (z.manning 2015-09-04 09:08) - PLID 67236 - Added overload
void CMainFrame::EnsureICCPDeviceManager(BOOL bIgnorePreference)
{
	if (IsICCPEnabled(bIgnorePreference) && m_pICCPDeviceManager == NULL)
	{
		CString strCardConnectUrlOverride = GetRemotePropertyText("CardConnectApiBaseUrl", "", 0, "<None>");
		CString strComPort = ICCP::GetComPortSetting(&g_propManager);
		CString strIngenicoDeviceProps = ICCP::GetDevicePropertiesSetting(&g_propManager);
		m_pICCPDeviceManager = new CICCPDeviceManager(strCardConnectUrlOverride, strComPort, strIngenicoDeviceProps);
	}
}

// (z.manning 2015-09-30 17:03) - PLID 67255
void CMainFrame::RefreshICCPDeviceManager()
{
	if (m_pICCPDeviceManager != NULL)
	{
		CWaitCursor wc;

		CString strCardConnectUrlOverride = GetRemotePropertyText("CardConnectApiBaseUrl", "", 0, "<None>");
		CString strComPort = ICCP::GetComPortSetting(&g_propManager);
		CString strIngenicoDeviceProps = ICCP::GetDevicePropertiesSetting(&g_propManager);
		m_pICCPDeviceManager->GetDevice()->RefreshDevice(_bstr_t(strCardConnectUrlOverride), _bstr_t(strComPort), _bstr_t(strIngenicoDeviceProps));
	}
}
// (s.tullis 2016-01-28 17:46) - PLID 68090
// (s.tullis 2016-01-28 17:46) - PLID 67965
// timer invoked ,login, or message recieved we need to see if they want to see reminders
BOOL CMainFrame::CanShowAttentionRenewalMessages(CString strProp, CString strPropValue, UINT_PTR uTimerID, BOOL &bTimerActive,BOOL bLogin)
{
	BOOL bCanShowMessages = TRUE;
	try {
		// if we have a timer active wait till it's invoked to show reminders
		if (bTimerActive && !bLogin)
		{
			return FALSE;
		}
		ReminderOptions radioselect = (ReminderOptions)GetRemotePropertyInt(strProp, (int)ReminderOptions::Login, 0, GetCurrentUserName());
		long nValue = GetRemotePropertyInt(strPropValue, 0, 0, GetCurrentUserName());
		switch (radioselect)
		{
			case ReminderOptions::Hour:
			{
				UINT nMilliseconds = nValue * 3600000;
				SetTimer(uTimerID, nMilliseconds, NULL);
				bTimerActive = TRUE;
			}
			break;
			case ReminderOptions::Minute:
			{
				UINT nMilliseconds = nValue * 60000;
				SetTimer(uTimerID, nMilliseconds, NULL);
				bTimerActive = TRUE;
			}
			break;
			//if were not logging in we dont want notices
			case ReminderOptions::Login:
			{
				if (!bLogin)
				{
					bCanShowMessages = FALSE;
				}
			}
			break;
			case ReminderOptions::Never:
			{
				bCanShowMessages = FALSE;
			}
			break;
		}
	}NxCatchAll(__FUNCTION__)
		return bCanShowMessages;
}
// (s.tullis 2016-01-29 09:41) - PLID 68090 
void CMainFrame::ResetRxAttentionTimer()
{
	try {
		if (m_bTimerGoingErxNeedingAttentionMessages)
		{
			KillTimer(IDT_REMINDER_PRESCRIPTION_NEEDING_ATTNETION);
		}

		SetTimer(IDT_REMINDER_PRESCRIPTION_NEEDING_ATTNETION, 100, NULL);
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2016-01-29 09:42) - PLID 67965 
void CMainFrame::ResetRenewalTimer()
{
	try {
		if (m_bTimerGoingRecieveRenewalMessages)
		{
			KillTimer(IDT_REMINDER_RENEWALS);
		}

		SetTimer(IDT_REMINDER_RENEWALS, 100, NULL);
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2016-01-29 09:41) - PLID 68090 
BOOL CMainFrame::CanShowRxAttentionNotifications(BOOL bLogin /*=FALSE*/ ) {
	return  CanShowAttentionRenewalMessages("PrescriptionNeedingAttentionReminderSelection", "PrescriptionNeedingAttentionReminderValue", IDT_REMINDER_PRESCRIPTION_NEEDING_ATTNETION, m_bTimerGoingErxNeedingAttentionMessages, bLogin);
}

// (s.tullis 2016-01-29 09:42) - PLID 67965 
BOOL CMainFrame::CanShowRenewalNotifications(BOOL bLogin /*=FALSE*/) {
	return CanShowAttentionRenewalMessages("RenewalsReminderSelection", "RenewalsReminderValue", IDT_REMINDER_RENEWALS, m_bTimerGoingRecieveRenewalMessages, bLogin);
}

// (s.tullis 2016-01-29 09:41) - PLID 68090 
BOOL CMainFrame::GetCanShowRxAttentionMessages()
{
	return m_bCanShowAttentionMessages;
}
void CMainFrame::SetCanShowRxAttentionMessages(BOOL bValue)
{
	m_bCanShowAttentionMessages = bValue;
}
// (s.tullis 2016-01-29 09:42) - PLID 67965 
BOOL CMainFrame::GetCanShowRenewalMessages()
{
	return m_bCanShowRenewalMessages;
}
void CMainFrame::SetCanShowRenewalMessages(BOOL bValue)
{
	m_bCanShowRenewalMessages = bValue;
}

// (c.haag 2016-05-31 11:51) - NX-100789 - Associates NXB_TYPE's with application resource ID's
void CMainFrame::InitializeNexTechIconButtonIcons()
{
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_OK, IDI_CHECKMARK);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_CANCEL, IDI_CANCEL);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_PRINT, IDR_PNG_PRINT);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_PRINT_PREV, IDR_PNG_PRINTPREVIEW);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_NEW, IDI_PLUS);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_DELETE, IDI_GRAY_X);

	// NXB_NOTIFY has no icon

	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_UP, IDI_UARROW, IDI_UDISARROW);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_DOWN, IDI_DARROW, IDI_DDISARROW);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_LEFT, IDI_LARROW, IDI_LDISARROW);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_RIGHT, IDI_RARROW, IDI_RDISARROW);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_LLEFT, IDI_LLARROW, IDI_LLDISARROW);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_RRIGHT, IDI_RRARROW, IDI_RRDISARROW);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_LLLEFT, IDI_LLLARROW, IDI_LLLARROW);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_RRRIGHT, IDI_RRRARROW, IDI_RRRARROW);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_UUP, IDI_UUARROW, IDI_UUDISARROW);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_DDOWN, IDI_DDARROW, IDI_DDDISARROW);

	// NXB_MARKET has no icon

	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_TIME, IDI_CLOCK);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_CLOSE, IDI_CLOSE);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_SPELLCHECK, IDI_SPELLCHECK_ICON);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_EXPORT, IDI_EXPORT);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_MERGE, IDI_WORD);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_INSPECT, IDI_INSPECT);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_PROBLEM, IDI_EMR_PROBLEM_FLAG);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_REFRESH, IDR_PNG_REFRESH);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_CADUCEUS, IDR_PNG_CADUCEUS);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_ROOMMANAGER, IDR_PNG_ROOMMANAGER);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_IMPORTBOX, IDR_PNG_IMPORTBOX);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_IMPORTPENDING, IDR_PNG_IMPORTPENDING);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_NOTES, IDI_TOPIC_OPEN);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_EXTRANOTES, IDI_BILL_NOTES);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_CAMERA, IDR_PNG_CAMERA);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_EYE, IDR_PNG_EYE);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_RECALL, IDB_PNG_RECALL);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_LOCK, IDB_PNG_LOCK);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_SENDTO, IDB_PNG_SENDTO);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_QUESTION, IDI_BLUE_QUESTION);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_INTERACTIONS, IDI_DRUG_INTERACTIONS);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_WARNING, IDI_WARNING_ICO);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_PILLBOTTLE, IDI_ERX);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_SCRIPT_ATTENTION, IDI_SCRIPT_ATTENTION);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_IMPORT_QUICK_LIST, IDB_PNG_IMPORT_QUICK_LIST);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_FAX, IDB_PNG_FAX);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_ERX, IDB_PNG_ERX);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_NEXFORMULARY, IDB_NEXFORMULARY);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_WRITE, IDB_WRITE);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_MEDICATION_HISTORY, IDI_MEDICATION_HISTORY);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_ENCOUNTER, IDI_PATIENT_ENCOUNTER);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_FFAHISTORY, IDI_FFA_APPT_HIST_CON);
	CNexTechIconButton::SetAutoSetIconResourceIDs(NXB_SSRS, IDB_SSRS);
}