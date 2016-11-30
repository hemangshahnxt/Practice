//EmrUtils.h

#ifndef EMRUTILS_H
#define EMRUTILS_H

#pragma once

#include <afxtempl.h>

// (a.walling 2010-05-19 08:26) - PLID 38558 - Include NxInkPictureImport.h rather than #import "NxInkPicture.tlb" so the proper tlb is chosen based on current SDK path (patch or main)
#include "NxInkPictureImport.h"


// (a.walling 2008-05-30 10:55) - PLID 22049 - Include advanced SQL error processing
#include "Sqloledb.h"
#include "NxAdo.h"
#include "nxexception.h"
#include "GlobalAuditUtils.h"
#include "DevicePluginUtils.h"
#include "SharedEmrUtils.h"
#include "InternationalUtils.h"
#include "EMRTableCellCodes.h"

// (a.walling 2014-03-12 12:31) - PLID 61334 - #import of API in stdafx causes crash in cl.exe
//#include "NxAPI.h"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/container/flat_map.hpp>
#include <map>
#include <vector>
#include <CxImage/ximage.h> // (a.walling 2013-05-08 16:15) - PLID 56610 - ximage.h now in CxImage/
#include <NxSystemUtilitiesLib/NxHandle.h>

#include "NxAutoQuantumFwd.h"


// (c.haag 2010-05-19 9:04) - PLID 38759 - If _DEBUG_EMR_VERBOSE_LOG is defined, then 
// verbose EMR logging will take place.
#ifdef _DEBUG_EMR_VERBOSE_LOG
#	define EMRLOG EmrLogVerbose
#	define EMRLOGINDENT EmrLogVerboseIndent
// (c.haag 2010-05-19 9:04) - PLID 38759 - This is used for verbose EMR logging. The
// input string content will be formatted and sent to the Output window.
void EmrLogVerbose(LPCTSTR szFmt, ...);
// (c.haag 2010-05-19 9:04) - PLID 38759 - This is used for verbose EMR logging. This
// function is identical to EmrLogVerbose except that the indentation of the logging will
// be shifted. If nIndent is positive, it will be shifted -after- the log is written. If nIndent
// is negative, it will be shifted -before- the log is written.
void EmrLogVerboseIndent(int nIdent, LPCTSTR szFmt, ...);
#else
#	define EMRLOG __noop
#	define EMRLOGINDENT __noop
#endif

// (a.walling 2010-07-30 10:00) - No longer necessary -- was only for VS6 and those with outdated SDKs.
//#ifndef DBINITCONSTANTS
//// (a.walling 2008-05-30 10:52) - PLID 22049 - Define this interface
//extern const GUID IID_ISQLServerErrorInfo;
//#endif

class CEMNDetail;
class CEMRTopic;
class CEMR;
class CEMN;
class EMNCharge;
class CEMNChargeArray;
class EMNDiagCode;
class EMNMedication;
class CMergeEngine;
class CDoomedEmrObject;
class SourceActionInfo;
class CLabRequisitionDlg;
class CEMRHotSpot;
class CEMRHotSpotArray;
struct TableRow;
struct TableRowID;
struct EmrDetailImageStamp;
struct TableElement;
class CEmrCodingRange;
class CEmnCodingGroupInfo;
class CUnspawningSource;
class CSpawningSourceInfo;
struct EmnPreviewPopup;
struct LinkedDetailStruct;

// (a.walling 2008-05-30 12:32) - PLID 22049 - Define some smart pointers for advanced error checking
_COM_SMARTPTR_TYPEDEF(IErrorRecords, __uuidof(IErrorRecords));
_COM_SMARTPTR_TYPEDEF(ISQLErrorInfo, __uuidof(ISQLErrorInfo));
_COM_SMARTPTR_TYPEDEF(ISQLServerErrorInfo, __uuidof(ISQLServerErrorInfo));

// (j.jones 2007-06-18 14:14) - PLID 26365 - Added defines
// for the EMR Ghostly colors to keep them solid across multiple files,
// the 16 bit colors were virtually unreadable in a 256-color TS session,
// so we needed to use different colors in those cases
#define EMR_GHOSTLY_16_BIT_GRAY RGB(192,192,192)
#define EMR_GHOSTLY_8_BIT_GRAY RGB(140,140,140)
#define EMR_GHOSTLY_8_BIT_RED RGB(248,0,0)
#define EMR_GHOSTLY_16_BIT_RED RGB(230,140,140)

// (j.jones 2008-06-04 16:24) - PLID 30255 - added a define for the colors of
// EMR charges that are also on quotes
#define EMR_SOME_CHARGES_ON_QUOTES RGB(246, 246, 191)
#define EMR_ALL_CHARGES_ON_QUOTES RGB(238, 238, 128)

class CMultiSelectDlg;
// (c.haag 2004-07-06 17:46) - MAX_MERGE_FIELD_LENGTH probably belongs in
// a merge engine header file
#define MAX_MERGE_FIELD_LENGTH			39
#define MAX_EMR_ITEM_NAME_MERGE_LENGTH	(MAX_MERGE_FIELD_LENGTH - sizeof("EMR_"))

// (a.walling 2007-04-10 13:22) - PLID 25548 - return values for NXM_EMN_PREVIEW_LOAD
// If the message is not handled by our custom handler, it returns 0 by default; I want
// to be able to distinguish among these possiblities.
#define EMR_PREVIEW_LOADED 0x000DECAF
#define EMR_PREVIEW_FAILED 0x00000BAD
#define EMR_PREVIEW_NOTHANDLED 0x00000000

// (a.walling 2008-06-30 15:26) - PLID 30570 - define our preview pane flags now. These are
// SAVED TO DATA -- do not modify (except for masks)
// (j.armen 2013-01-03 16:20) - PLID 54413 - Enumeration also defined in NexTech.Practice (EMRTypes.cs)
enum EPreviewFlags {
	epfHideTitle	= 0x00000001,
	epfHideItem		= 0x00000002,
	epfMaskHide		= 0x00000003,

	epfSubDetail	= 0x00000004, // (a.walling 2008-10-23 09:47) - PLID 27552 - Display this detail underneath the detail that spawned it

	/*
	epfFloatLeft	= 0x00000008, // (a.walling 2009-01-07 14:29) - PLID 31961 - Floating element support
	epfFloatRight	= 0x00000010,
	epfMaskFloat	= 0x00000018,
	*/
	// (a.walling 2009-07-06 08:32) - PLID 34793 - A different approach. There are some critical bugs with printing floating elements such that
	// they will not be printed at all, or cut off at page boundaries. As such we must do something different. Since the whole reason this is
	// used in the first place is to help with positioning, we will instead have two columns which can be set. FloatLeft will become column
	// one, and float right will become column two.
	epfColumnOne	= 0x00000008, // (a.walling 2009-07-06 08:33) - PLID 34793 - Column one
	epfColumnTwo	= 0x00000010, // (a.walling 2009-07-06 08:33) - PLID 34793 - Column two
	epfColumnMask	= 0x00000018, // (a.walling 2009-07-06 08:34) - PLID 34793 - Has a column set

	// (a.walling 2009-07-06 08:30) - PLID 34793 - All clear flags are deprecated!!
	/*
	DEPRECATED_epfClearLeft	= 0x00000020, // clear none is implied by float flags without any clear flags. clear both is silly and should not be used
	DEPRECATED_epfClearRight	= 0x00000040, // by floating elements in this situation in the first place.
	DEPRECATED_epfClearNone	= 0x00000060, // since clear both is default, setting both left and right is therefore 'none'
	*/

	epfTextRight	= 0x00000100, // (a.walling 2009-01-08 14:06) - PLID 32660 - Align text right

	// (a.walling 2009-07-06 12:25) - PLID 34793 - Grouping options for positioned elements
	// adjacent is the default (as in, not epfGroupAtEnd and not epfGroupAtBeginning)
	epfGroupBegin	= 0x00000200, // only valid for topics
	epfGroupEnd	= 0x00000400, // only valid for topics

	// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
	epfPageBreakBefore	= 0x00000800,
	epfPageBreakAfter	= 0x00001000,

	// (a.walling 2012-07-13 16:38) - PLID 48896
	epfHideIfIndirect = 0x00002000,

	// (j.armen 2013-01-02 16:39) - PLID 54412
	epfHideOnIPad = 0x00004000,
		
	epfAllFlags		= 0xFFFFFFFF,
};

// (a.walling 2008-08-12 12:35) - PLID 30570 - Get description for auditing
CString GetPreviewFlagsDescriptionForAudit(DWORD nPreviewFlags);

//This file contains various enums, and functions related to them, that are used by the EMR.

// (j.jones 2010-02-16 11:11) - PLID 37365 - this enum is tracked in EMRImageStampsT.SmartStampTableSpawnRule
// These values are stored in data and cannot be changed!
enum EMRSmartStampTableSpawnRule {

	esstsrAddNewRow = 1,
	esstsrIncreaseQuantity = 2,
	esstsrDoNotAddToTable = 3,	// (j.jones 2010-04-07 12:12) - PLID 38069
};

//The different options for formatting a list of emr items with a given category.
enum EmrCategoryFormat {
	ecfParagraph = 1,
	ecfList = 2,
	ecfNumberedList = 3,
	ecfBulletedList = 4,
};

//The types of objects that can spawn or be spawned in the EMR.
// (b.cardillo 2004-10-05 08:52) - THE VALUES HERE MUST NEVER CHANGE because these get written 
// to data.  If I write eaoEmrItem today, and read 3 back out tomorrow, the 3 had better match 
// up with eaoEmrItem!
enum EmrActionObject {
	eaoInvalid = -1,
	eaoCpt = 1,
	//eaoDiag = 2, // (b.savon 2014-07-14 09:56) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
	//TES 12/6/2006 - PLID 23724 - IMPORTANT NOTE: Now, EmrActions whose DestType is eaoEmrItem have a DestID which points
	// to EmrInfoMasterT.ID, NOT EmrInfoT.ID.  However, if the SourceType is eaoEmrItem, then the SourceID still points to 
	// EmrInfoT.ID.  Confusing, but if you think about it, it's correct.
	eaoEmrItem = 3,
	eaoEmrDataItem = 4,
	eaoProcedure = 5,
	eaoMint = 6,
	eaoMedication = 7,
	eaoAllergy = 8,
	eaoMintItems = 9,
	//DRT 1/17/2008 - PLID 28602 - Added ability to spawn from image hotspots.  This can never be a destination.
	eaoEmrImageHotSpot = 10,
	// (c.haag 2008-06-03 10:52) - PLID 30221 - Todo alarms
	eaoTodo = 11,
	// (z.manning 2008-10-01 14:47) - PLID 31556 - Labs
	eaoLab = 12,
	// (z.manning 2009-02-10 12:58) - PLID 33026 - Table dropdown elements can now be action sources
	eaoEmrTableDropDownItem = 13,
	// (z.manning 2010-02-15 10:28) - PLID 37226 - Smart stamps are the latest action source
	eaoSmartStamp = 14,
	// (r.gonet 08/03/2012) - PLID 51949 - The Wound Care CPT Coding Conditions are now sources of actions. Not liking the narrowness on this one,
	//  but for now we have no other similarities in order to integrate it into a concept.
	eaoWoundCareCodingCondition = 15,
	// (b.savon 2014-07-14 09:24) - PLID 62705 - Create a new DestType
	eaoDiagnosis = 16,
};

// The types of results when checking if an EMR item name or merge
// field name is a valid user entry
enum EmrInvalidNameReason
{
	einrOK = 0,
	einrTooLong = 1,
	einrReservedName = 2,
};

//The ways in which a multi-select list can be formatted when merged to Word.
//SAVED TO DATA!
enum EmrMultiSelectFormat
{
	emsfText = 0,
	emsfBulletList = 1,
	emsfNumberList = 2,
	emsfList = 3,
};

// (a.walling 2012-04-02 08:29) - PLID 49304 - Removed a lot of dead code regarding moving items

enum EmrTopicCompletionStatus {
	
	//TES 1/24/2007 - PLID 24377 - We need an etcsInvalid that variables can be initialized to.
	// Initialized value, holds no meaning as a status
	etcsInvalid = -1,
	
	// Has details but none of them are filled in
	etcsIncomplete = 0, // (z.manning, 04/04/2008) - PLID 29495 - Renamed to avoid confusion
	
	// Has at least one unfilled detail and one filled
	etcsPartiallyComplete = 1,	
	
	// Has details and all are filled in
	etcsComplete = 2,
	
	// (j.jones 2007-06-14 15:20) - PLID 26276 - added etcsReconstructed
	// Has any details marked as reconstructed (see logs, comments, pl items, and incidents for 
	// an understanding of detail reconstruction-- it's a legacy status special to certain data)
	etcsReconstructed = 3,

	// (z.manning, 04/04/2008) - PLID 29495 - Added status for topics without any details
	// Does not have any details.
	etcsBlank,

	// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
	// Has at least one *unfilled* *required* detail
	etcsRequired,
};

enum EmrSaveStatus {
	essFailed = -2,
	essFailedConcurrency = -1,
	essSuccess = 0,
	essSuccessWithWarnings = 1,
};

// (j.jones 2007-06-13 10:01) - PLID 26276 - possible return values for CalculateEMNCompletionStatus
// These values must never change since they get stored in data
enum EMNCompletionStatus {
	ecsNeedsCalculated = -1, //initial value - should never be saved to data,
	ecsEmpty = 0, //nothing on the EMN is filled in
	ecsPartiallyComplete = 1, //the EMN is partially filled in
	ecsComplete = 2, //the EMN is fully filled in
	ecsReconstructed = 3, //the EMN has ReconstructedDetails
	// (b.cardillo 2012-03-08 11:40) - PLID 42207 - I picked 5 here only for consistency with 
	// etcsRequired, even though that isn't explicitly set to any number.  I believe that someday 
	// it will be and when that day comes, the natural choice will be 5.
	ecsRequired = 5, //the EMN has some required details that haven't been filled in yet
};

// (a.walling 2007-07-13 10:55) - PLID 26640 - enum of values for MoreInfo preview generation
// (d.lange 2011-04-27 17:23) - PLID 43380 - added value for Assistant/Technician
enum EMNMoreInfoPreviewFields {
	mipNotes				= 0x00000001,
	mipProcedures			= 0x00000002,
	mipProviders			= 0x00000004,
	mipSecondaryProviders	= 0x00000008,
	mipDiagCodes			= 0x00000010,
	mipCharges				= 0x00000020,
	mipMedications			= 0x00000040,
	mipTechnicians			= 0x00000080,
	// (b.eyers 2016-02-23) - PLID 68322 - added values for asc and updated validfields
	mipASCTimes				= 0x00000100, 
	mipASCStatus			= 0x00000200, 
	mipValidFields			= 0x000003FF,

	// (a.walling 2010-11-11 15:47) - PLID 40848 - Display more info sections as topics
	mipDisplayAsTopics		= 0x80000000,
	// (a.walling 2010-11-11 15:47) - PLID 40848 - Show empty sections as empty topics
	mipShowEmpty			= 0x40000000,
};

// (b.savon 2012-05-23 16:16) - PLID 48092 - Add Meds and Allergies Flags
enum EMNMedicationsAllergiesOptions{
	maoDisplayMedications	= 0x00000001,
	maoDiscontinuedMeds		= 0x00000002,
	maoDisplayAllergies		= 0x00000004,
};

// (a.walling 2007-12-17 13:44) - PLID 28354 - Detail hiding options
enum EMNPreviewDisplayHideOptions {
	dhEmpty = 0x1,
	dhEmptyPrint = 0x2,
	dhNarrative = 0x4,
	dhNarrativePrint = 0x8,
	dhEmptyTopics = 0x10,
	dhEmptyTopicsPrint = 0x20,
	// (a.walling 2012-07-16 08:42) - PLID 48896
	dhAlwaysHideIfIndirect = 0x40, 
};
// (a.walling 2007-12-18 14:28) - PLID 28354 - Default to all options. I was considering just using 0xFF etc, but
// this seems cleaner.
const long g_dhPreviewDisplayHideDefaults = dhEmpty | dhEmptyPrint | dhNarrative | dhNarrativePrint | dhEmptyTopics | dhEmptyTopicsPrint;

// (j.jones 2007-08-14 12:07) - PLID 27053 - possible values for E/M coding methods
enum EMCodingTypes {

	emctUndefined = -1,	//represents unknown or unavailable, such when Text, Sliders, etc. have no special type
	emctMultiSelectOnePerDetail = 1, //one element for the detail, when the multi-select detail has data selected
	emctMultiSelectOnePerListItem = 2, //one element for each selected list item in a multi-select detail
	emctTableOnePerDetail = 3, //one element for the detail, when the table has any data filled
	emctTableOnePerDetail_SpecifyRow = 4, //one element for the detail, when the table has any data filled, if the data is in a specified row
	emctTableOnePerDetail_SpecifyColumn = 5, //one element for the detail, when the table has any data filled, if the data is in a specified column
	emctTableOnePerCell = 6, //one element for each cell filled in on the table
	emctTableOnePerCell_SpecifyRow = 7, //one element for each cell filled in, if the cell is in a specified row
	emctTableOnePerCell_SpecifyColumn = 8, //one element for each cell filled in, if the cell is in a specified column
	emctTableOnePerRow_SpecifyRow = 9, //one element for each row that has at least one cell filled in, if the cell is in a specified row
	emctTableOnePerColumn_SpecifyColumn = 10, //one element for each column that has at least one cell filled in, if the cell is in a specified column
	emctTableOnePerCompleteRow_SpecifyRow = 11, //one element for each row that has all cells filled in, if the row is specified
	emctTableOnePerCompleteColumn_SpecifyColumn = 12, //one element for each column that has all cells filled in, if the column is specified
	// (j.jones 2011-03-08 11:48) - PLID 42282 - added new rules for when a table has E/M categories configured per row or per column
	emctTableOnePerDetail_PerRowCategory = 13,	//one element for the detail, for each category selected on any row
	emctTableOnePerRow_PerRowCategory = 14,		//one element for each row that has at least one cell filled in, for the category selected on that row
	emctTableOnePerCell_PerRowCategory = 15,		//one element for each cell filled in per row, for the category selected on that row
	emctTableOnePerCompleteRow_PerRowCategory = 16, //one element for each row that has all cells filled in, for the category selected on that row
	emctTableOnePerDetail_PerColumnCategory = 17,	//one element for the detail, for each category selected on any column
	emctTableOnePerColumn_PerColumnCategory = 18,	//one element for each column that has at least one cell filled in, for the category selected on that column
	emctTableOnePerCell_PerColumnCategory = 19,		//one element for each cell filled in per column, for the category selected on that column
	emctTableOnePerCompleteColumn_PerColumnCategory = 20, //one element for each column that has all cells filled in, for the category selected on that column
};

// (j.jones 2011-03-08 11:48) - PLID 42282 - added enum for EMRInfoMasterT.EMCodeUseTableCategories
// do not change these values, they are stored in data
enum EMCodeUseTableCategories {
	
	emcutcNone = 0,			//use the category for the EMRInfoMasterT item (default)
	emcutcPerRow = 1,		//ignore the EMRInfoMasterT category, look at E/M categories per row
	emcutcPerColumn = 2,	//ignore the EMRInfoMasterT category, look at E/M categories per column
};

// (a.wilson 2013-05-13 17:44) - PLID 55963 - add contains operator for text details only.
// (j.jones 2008-10-15 14:22) - PLID 31692 - added enum for EMR Analysis data operator types
// these are stored in data and CANNOT be changed
enum EMRAnalysisDataOperatorType
{
	eadotExists = 0,
	eadotItemHasData = 1,
	eadotItemHasNoData = 2,
	eadotHasDataIn = 3,
	eadotHasNoDataIn = 4,
	eadotContains = 5,
};

// (j.jones 2008-10-15 14:22) - PLID 31692 - added enum for EMR Analysis display 'group by' types
// these are stored in data and CANNOT be changed
enum EMRAnalysisGroupByType
{
	eagbtPatient = 0,
	eagbtDate = 1,
	eagbtEMN = 2,
	eagbtEMR = 3,
};

// (c.haag 2009-02-24 17:54) - PLID 33187 - Added enum for column grouping
enum EMRAnalysisColumnGroupByType
{
	eacgbtOnePerItem = 0,	// Original value -- every item gets its own column (up to 255)
	eacgbtCondensed = 1,	// "Condensed" version where we add as few columns as possible based
							// on the filtered results. Column names are "Result 1", "Result 2", etc.
};

// (j.jones 2008-10-16 10:53) - PLID 31692 - added enum for EMR Analysis record filter types
// these are stored in data and CANNOT be changed
enum EMRAnalysisRecordFilterType
{
	earftPatient = 0,
	earftEMN = 1,
	earftEMR = 2,
};

// (c.haag 2008-06-25 09:26) - PLID 30505 - Special values for EMR todo spawning assigntos
enum EMRTodoSpecialAssignees
{
	etsaLoggedInUser = -1,
	etsaPatientCoordinator = -2
};

// (a.walling 2009-11-18 12:09) - PLID 36365 - Removed deprecated enum
// (c.haag 2008-06-05 10:47) - PLID 29476 - Enumerations for updating narrative recordsets
enum ENarrativeUpdateBehavior
{
	eUpdateRs,
	eDontUpdateRs,
};

// (z.manning 2011-03-21 09:40) - PLID 23662 - Added an enum for EMR table autofill option.
// These are saved to data and cannot be changed.
enum EmrTableAutofillType
{
	etatNone = 0,
	etatDateAndTime = 1,
	etatDate = 2,
	etatTime = 3,
};

// (j.jones 2012-09-27 14:16) - PLID 52820 - added enum for the EMRDrugInteractionChecks preference,
// these are data values, so they cannot be changed
enum EMRDrugInteractionCheckType
{
	edictNoPrompt = 0,					//no automated drug interaction prompt in EMR, they would have to manually perform a check
	edictSaveWarnWhenMedsChange = 1,	//save the EMN and check drug interactions when meds change, or when any other data changes
										//that affects interactions (diagnoses, allergies, etc.)
	edictWarnWhenSaving = 2,			//check for drug interactions after the EMN saves normally, depending on that preference
	edictWarnWhenClosingOrLocking = 3,	//check for drug interactions only when closing an EMN, or locking an EMN we just edited
};

// (b.savon 2014-03-06 09:21) - PLID 60824 - Define NexGEM Match Colors
enum NexGEMMatchColor {
	nxgmDone = RGB(255,255,255),
	nxgmNoMatch = RGB(233,150,122),
	nxgmManyMatch = RGB(255,255,137),	
};

// (z.manning 2011-03-21 10:05) - PLID 23662
CString GetAutofillTypeDescription(EmrTableAutofillType eAutofillType);

// (c.haag 2008-07-15 16:38) - PLID 25819 - Enumerations for EMR problem regarding types
// (j.jones 2008-07-16 10:52) - PLID 30731 - if you add a new enum here, be sure to update
// EMRProblemListDlg to reflect it in the Type filter
// (c.haag 2008-12-05 09:24) - PLID 28496 - Added eprtUnassigned for problems not tied to any
// specific kinds of patient records
enum EMRProblemRegardingTypes
{
	eprtInvalid = -1,
	eprtEmrItem = 1,
	eprtEmrDataItem = 2,
	eprtEmrTopic = 3,
	eprtEmrEMN = 4,
	eprtEmrEMR = 5,
	eprtEmrDiag = 6,
	eprtEmrCharge = 7,
	eprtEmrMedication = 8,
	eprtUnassigned = 9,
	eprtLab = 10, // (z.manning 2009-05-26 10:23) - PLID 34340
};

// (r.farnworth 2014-03-06 11:39) - PLID 60820 - enum for ICD-9 to 10 code MatchType
enum NexGEMMatchType {
	nexgemtDone = 0,
	nexgemtNoMatch = 1,
	nexgemtManyMatch = 2,
};

// (j.jones 2013-05-08 09:27) - PLID 56596 - moved CTopicArray here
typedef CArray<CEMRTopic*, CEMRTopic*> CTopicArray;

// (j.jones 2008-07-18 11:06) - PLID 30779 - added problem class

class CEmrProblem
{
private:
	// (c.haag 2009-05-16 11:01) - PLID 34277 - Reference counting support
	int m_nRefCnt;

public:
	long m_nID;	//-1 if a new problem
	long m_nPatientID; // (z.manning 2009-05-27 10:16) - PLID 34297
	CString m_strDescription;
	COleDateTime m_dtEnteredDate;
	COleDateTime m_dtModifiedDate;
	COleDateTime m_dtOnsetDate;
	long m_nStatusID;
	// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
	// (j.jones 2014-02-24 15:28) - PLID 61010 - EMR problems now have
	// an ICD-9 code and/or an ICD-10 code, no more than one pair.
	long m_nDiagICD9CodeID;
	long m_nDiagICD10CodeID;
	// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
	long m_nChronicityID;
	// (s.tullis 2015-02-23 15:34) - PLID 64723 - Do Not Show on CCDA
	BOOL m_bDoNotShowOnCCDA;
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - Flag to not show the problem in the prompt in the EMR tab.
	BOOL m_bDoNotShowOnProblemPrompt;
	// (b.spivey, October 22, 2013) - PLID 58677 - necessary to work in EMR, and auditing. 
	long m_nCodeID;
	
	BOOL m_bIsModified;

	BOOL m_bIsDeleted;

	// (c.haag 2008-07-21 14:33) - PLID 30725 - For spawning
	long m_nEmrProblemActionID;

	
	// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
	// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
	//for auditing
	CString m_strLastDescription;
	long m_nLastStatusID;
	COleDateTime m_dtLastOnsetDate;

	long m_nLastChronicityID;

	// (j.jones 2014-02-24 15:28) - PLID 61010 - EMR problems now have
	// an ICD-9 code and/or an ICD-10 code, no more than one pair.
	long m_nLastDiagICD9CodeID;
	long m_nLastDiagICD10CodeID;

	// (s.tullis 2015-02-23 15:34) - PLID 64723 - Do Not Show on CCDA
	BOOL m_bLastDoNotShowOnCCDA;
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - The previous value of the DoNotShowOnProblemPrompt field, so we can audit.
	BOOL m_bLastDoNotShowOnProblemPrompt;

	// (b.spivey, October 22, 2013) - PLID 58677 - necessary for auditing
	long m_nLastCodeID; 

	CEmrProblem();

	
	// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
	// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
	// (z.manning 2009-05-27 10:13) - PLID 34297 - Added patient ID
	// (c.haag 2009-05-28 09:42) - PLID 34277 - Removed deprecated constructor parameters
	// (b.spivey, October 22, 2013) - PLID 58677 - added codeID 
	// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
	// (s.tullis 2015-02-23 15:44) - PLID 64723
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added bDoNotShowOnProblemPrompt. True to have the problem show in the prompt when we switch patients
	// or go to the EMR tab. False to not show in the prompt.
	CEmrProblem(long nID, long nPatientID, CString strDescription, COleDateTime dtEnteredDate, COleDateTime dtModifiedDate, COleDateTime dtOnsetDate,
		long nStatusID, long nDiagCodeID_ICD9, long nDiagCodeID_ICD10, long nChronicityID, BOOL bIsModified, long nCodeID, BOOL bDoNotShowOnCCDA, BOOL bDoNotShowOnProblemPrompt);

	CEmrProblem(CEmrProblem *epSource);
	// (c.haag 2009-05-20 11:16) - PLID 34277 - Added overload to take in a recordset fields object
	CEmrProblem(ADODB::FieldsPtr& f);

	// (c.haag 2009-05-16 11:01) - PLID 34277 - Reference counting support
	int AddRef() { return ++m_nRefCnt; }
	int Release() { 
		if (m_nRefCnt <= 0) {
			ASSERT(FALSE);
			ThrowNxException("Attempted to release a problem with a reference count less than one");
		}
		else if (1 == m_nRefCnt) {
			delete this;
			return 0;
		} else {
			return --m_nRefCnt;
		}
	}
	int GetRefCnt() const { return m_nRefCnt; }

public:
	// (c.haag 2009-05-20 11:17) - PLID 34277 - Load from a recordset fields object
	void ReloadFromData(ADODB::FieldsPtr& f);

public:
	// (a.walling 2008-07-28 14:56) - PLID 30855 - Get the associated EMN query string
	static CString GetEMNParamQueryString(EMRProblemRegardingTypes nType);
	// (j.armen 2013-05-14 12:22) - PLID 56680 - Hand back sql fragments
	static CSqlFragment GetRegardingEMNFormatQueryString(EMRProblemRegardingTypes nType, long nRegardingID);
	// (z.manning 2009-07-01 17:32) - PLID 34765 - Option to include EMN IDs for EMR based problems
	// (will include all EMNs within that EMR)
	static CSqlFragment GetEMNQueryFromProblemID(long nProblemID, BOOL bIncludeEmrProblems);

	// (z.manning 2009-05-27 16:29) - PLID 34340 - Handy function for auditing
	void AuditNew(long &nAuditTransactionID);

public:
	// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
	void LogEmrObjectData(int nIndent);
};

// (c.haag 2008-07-18 16:00) - PLID 30784 - Array of problems
typedef CArray<CEmrProblem,CEmrProblem&> CEmrProblemAry;

// (c.haag 2009-05-16 10:56) - PLID 34277 - Now that we can link
// multiple items to one problem, we need to look at things a little
// differently.
//
// In the past, every EMR object would have an array of problems.
// For the purposes of data normalization, EMR objects now possess
// arrays of links to problems. Each link has a reference to a problem.
// 
class CEmrProblemLink
{
private:
	long m_nID;
	EMRProblemRegardingTypes m_eprtTypeID;
	long m_nEMRRegardingID;
	long m_nEMRDataID;

private:
	CEmrProblem* m_pProblem;

private:
	CEMNDetail *m_pDetail;
	CEMRTopic *m_pTopic;
	CEMN *m_pEMN;
	CEMR *m_pEMR;
	// (c.haag 2008-07-21 14:08) - PLID 30725 - More objects that can be associated with the problem
	EMNCharge* m_pCharge;
	EMNDiagCode* m_pDiagCode;
	EMNMedication* m_pMedication;

	// (z.manning 2009-05-28 11:07) - PLID 34345 - Needed for lab based problems
	CString m_strLabText;

private:
	// (c.haag 2009-05-21 13:22) - PLID 34298 - We require deletion tracking
	BOOL m_bIsDeleted;

public:
	// (c.haag 2009-05-28 12:04) - PLID 34298 - These two member variables are used only in the
	// EMR problem list for the purpose of relaying EMR object information to the problem edit
	// window.
	CString m_strDetailName;
	CString m_strDetailValue;

public:
	CEmrProblemLink()
	{
		m_pProblem = NULL;
		m_nID = -1;
		m_eprtTypeID = eprtInvalid;
		m_nEMRRegardingID = -1;
		m_nEMRDataID = -1;
		m_bIsDeleted = FALSE;

		// Reset pointers
		m_pDetail = NULL;
		m_pTopic = NULL;
		m_pEMN = NULL;
		m_pEMR = NULL;
		m_pCharge = NULL;
		m_pDiagCode = NULL;
		m_pMedication = NULL;
	}
	CEmrProblemLink(CEmrProblem* pProblem, long nEMRProblemLinkID, EMRProblemRegardingTypes type, long nEMRRegardingID, long nEMRDataID)
	{
		// Assign memebers
		if (NULL == pProblem) {
			ASSERT(FALSE);
			ThrowNxException("Attempted to create a problem link object without a problem");
		}
		m_pProblem = pProblem;
		m_nID = nEMRProblemLinkID;
		m_eprtTypeID = type;
		m_nEMRRegardingID = nEMRRegardingID;
		m_nEMRDataID = nEMRDataID;
		m_bIsDeleted = FALSE;

		// Reset pointers
		m_pDetail = NULL;
		m_pTopic = NULL;
		m_pEMN = NULL;
		m_pEMR = NULL;
		m_pCharge = NULL;
		m_pDiagCode = NULL;
		m_pMedication = NULL;

		// Now add a reference to the problem
		m_pProblem->AddRef();
	}
	CEmrProblemLink(const CEmrProblemLink& src)
	{
		m_pProblem = NULL;
		*this = src;
	}
	// (c.haag 2009-07-09 10:57) - PLID 34829 - Now optionally pass in an EMR object that
	// is to be the owner of this newly allocated problem. We must ensure that the problem
	// assigned to this link has the same EMR owner.
	CEmrProblemLink(const CEmrProblemLink* src, CEMR* pOwningEMR = NULL);

	~CEmrProblemLink()
	{
		// Release our reference to the problem
		m_pProblem->Release();
	}

public:
	void operator =(const CEmrProblemLink &src) {
		CEmrProblem* pOriginalProblem = m_pProblem;

		m_nID = src.m_nID;
		m_pProblem = src.m_pProblem;
		m_eprtTypeID = src.m_eprtTypeID;
		m_nEMRRegardingID = src.m_nEMRRegardingID;
		m_nEMRDataID = src.m_nEMRDataID;
		m_bIsDeleted = src.m_bIsDeleted;

		m_pDetail = src.m_pDetail;
		m_pTopic = src.m_pTopic;
		m_pEMN = src.m_pEMN;
		m_pEMR = src.m_pEMR;
		m_pCharge = src.m_pCharge;
		m_pDiagCode = src.m_pDiagCode;
		m_pMedication = src.m_pMedication;

		m_strLabText = src.m_strLabText; // (z.manning 2009-05-28 11:12) - PLID 34345

		// Now add a reference to the problem
		m_pProblem->AddRef();
		// Release our current reference to the problem
		if (NULL != pOriginalProblem) {
			pOriginalProblem->Release();
		}
	}

public:
	// (c.haag 2009-05-21 13:22) - PLID 34298 - We require deletion tracking
	BOOL IsDeleted() const { return m_bIsDeleted; }
	void SetDeleted() { m_bIsDeleted = TRUE; }

public:
	long GetID() const { return m_nID; }
	EMRProblemRegardingTypes GetType() const { return m_eprtTypeID; }
	long GetRegardingID() { return m_nEMRRegardingID; }
	CEmrProblem* GetProblem() const { return m_pProblem; }
	long GetDataID() const { return m_nEMRDataID; }
	BOOL GetIsDeleted() const { return m_bIsDeleted; }

public:
	// (c.haag 2009-07-09 11:28) - PLID 34829 - Made const
	CEMN* GetEMN() const { return m_pEMN; }
	// (c.haag 2009-05-21 14:46) - PLID 34298 - We need to access all these member variables
	CEMNDetail* GetDetail() const { return m_pDetail; }
	EMNDiagCode* GetDiagCode() const { return m_pDiagCode; }
	EMNCharge* GetCharge() const { return m_pCharge; }
	EMNMedication* GetMedication() const { return m_pMedication; }
	CEMRTopic* GetTopic() const { return m_pTopic; }
	CEMR* GetEMR() const { return m_pEMR; }
	CString GetLabText() const { return m_strLabText; } // (z.manning 2009-05-28 11:13) - PLID 34345

public:
	void SetID(const long nID) { m_nID = nID; }
	void SetRegardingID(long nID) { m_nEMRRegardingID = nID; }
	void SetLabText(const CString &strLabText) { m_strLabText = strLabText; }

public:
	//these functions attempt to populate the pointers of the problem	
	void UpdatePointersWithEMR(CEMR *pEMR);
	void UpdatePointersWithEMN(CEMN *pEMN);
	void UpdatePointersWithTopic(CEMRTopic *pTopic);
	void UpdatePointersWithDetail(CEMNDetail *pDetail);
	// (c.haag 2008-07-21 14:46) - PLID 30725
	void UpdatePointersWithCharge(CEMN* pEMN, EMNCharge* pCharge);
	void UpdatePointersWithDiagCode(CEMN* pEMN, EMNDiagCode* pDiagCode);
	void UpdatePointersWithMedication(CEMN* pEMN, EMNMedication* pMedication);

	void GetDetailNameAndValue(OUT CString &strDetailName, OUT CString &strDetailValue);

	void Audit(const AuditEventItems aei, long &nAuditTransactionID, const CString &strPatientName);

public:
	// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
	void LogEmrObjectData(int nIndent);
};

// (c.haag 2008-07-18 16:00) - PLID 30784 - Array of problems
// (c.haag 2009-05-20 11:18) - PLID 34277 - Now array of problem links
typedef CArray<CEmrProblemLink,CEmrProblemLink&> CEmrProblemLinkAry;

// (z.manning 2009-05-27 09:29) - PLID 34297
CString GetProblemTypeDescription(const EMRProblemRegardingTypes eType);

class CEmrItemEditContentInfo
{
public:
	CEMNDetail *pEMNDetail;
	int nDataID;
	CString strOldName;
	int nChangeType;
	int nNewIndex;
};

class CEmrItemValueChangedInfo 
{
public:
	IN _variant_t varNewState;
	IN _variant_t varOldState;
	//TES 11/15/2004 - This variable is never referenced any more, so I am changing the behavior of the message to occur
	//after the change has already been made.  If we need this functionality again someday, we'll put this back, and then
	//make a new message that is fired after the state has changed, for updating the narrative.
	//IN OUT BOOL bAcceptChange;

	// (a.walling 2007-07-11 15:23) - PLID 26261 - We request state changes for a detail even after it has gbeen deleted,
	// so bDeleted will be TRUE when the detail is being deleted and not just a normal state change.
	IN OUT BOOL bDeleted;
};

// (c.haag 2007-08-18 10:43) - PLID 27112 - This class should be used when
// extracting all elements from a table detail string
class CEmrTableStateIterator
{
private:
	const CString& m_strState; // The state we're traversing

private:
	int m_nStateLen; // The length of the state
	int m_nPos; // The current position in the traversal

public:
	CEmrTableStateIterator(const CString& strState);

public:
	// Reads the next unread element from the state. Returns FALSE if we did
	// not get a value because we've been through the whole state. Throws an
	// exception if the state is malformed.
	// (z.manning 2010-02-18 09:15) - PLID 37427 - Added nEmrDetailImageStampID
	// (z.manning 2011-03-02 14:49) - PLID 42335 - Added nStampID
	BOOL ReadNextElement(OUT long& nRow, OUT long& nColumn, OUT CString& strData, OUT long& nEmrDetailImageStampID, OUT long& nEmrDetailImageStampPointer, OUT long &nStampID);
};

class CExtraMergeFields
{
public:
	CString m_strHeaders;
	CString m_strData;
};

struct MergeField {
	CString strHeader;
	CString strData;
};

// (j.jones 2007-07-18 15:21) - PLID 26730 - added a simple HasInfoActions enum
enum EMNDetailHasInfoActionsStatus {
	ehiasUndetermined = -1,
	ehiasHasNoInfoItems = 0,
	ehiasHasInfoItems = 1,
};

// (j.jones 2007-08-27 08:52) - PLID 27056 - moved E/M Checklist structs to this header file

struct ChecklistColumnInfo {	//contains checklist column data

	long nID;					//the checklist column ID (EMChecklistColumnsT.ID)
	CString strName;			//the checklist column name (EMChecklistColumnsT.Name)
	long nOrderIndex;			//the checklist column order (EMChecklistColumnsT.OrderIndex)
	short nColumnIndex;			//the index of the column
	short nCheckColumnIndex;	//the index of the checkbox column, patient EMN checklists only ((j.jones 2007-08-28 13:14) - PLID 27056)
	short nBorderColumnIndex;	//the index of the border column, patient EMN checklists only (// (j.jones 2007-09-17 15:56) - PLID 27399)
};

struct ChecklistCodingLevelInfo {	//contains the coding level information for a checklist row

	long nID;					//the coding level ID (EMChecklistCodingLevelsT.ID)
	long nServiceID;			//the service ID for the CPT code on this coding level (EMChecklistCodingLevelsT.ServiceID)
	CString strCodeNumber;		//the text string of the CPT code
	CString strDescription;		//the description for this coding level (EMChecklistCodingLevelsT.Description)
	long nColumnsRequired;		//the number of columns required to be filled for this coding level to be satisfied (EMChecklistCodingLevelsT.MinColumns)
	// (j.jones 2007-09-17 10:21) - PLID 27396 - added minimum time
	long nMinimumTimeRequired;	//the amount of minutes required spent in the EMN, for this coding level to be satisfied (0 means not required) (EMChecklistCodingLevelsT.MinimumTime)
	BOOL bApproved;				//has this rule been approved? (EMChecklistCodingLevelsT.Approved)
	COleDateTime dtApproved;	//when was it approved? (EMChecklistCodingLevelsT.ApprovedDate)
	long nApprovalUserID;			//who approved it - user ID (EMChecklistCodingLevelsT.ApprovedBy)
	CString strApprovalUserName;	//who approved it - username

	NXDATALIST2Lib::IRowSettingsPtr pRow;	//the pointer to the datalist row this coding level relates to
};

struct ChecklistElementRuleDetailInfo {	//contains one rule detail for a checklist cell
	
	long nID;					//the element rule detail ID (EMChecklistRuleDetailsT.ID)
	long nMinElements;			//minimum elements required (EMChecklistRuleDetailsT.MinElements)
	long nCategoryID;			//EMCodeCategoryT.ID for the category we require elements in (EMChecklistRuleDetailsT.CategoryID)
	CString strCategoryName;	//the name of the category (EMCodeCategoryT.Name)
	BOOL bDeleted;				//have we deleted this detail?
};

struct ChecklistElementRuleInfo {	//contains all the information needed for a checklist cell

	long nID;					//the element rule ID (EMChecklistRulesT.ID)
	CString strDescription;		//the description of this rule	(EMChecklistRulesT.Description)	
	BOOL bRequireAllDetails;	//do require any detail, or all details? (EMChecklistRulesT.RequireAllDetails) ((j.jones 2007-09-18 14:44) - PLID 27397)
	CArray<ChecklistElementRuleDetailInfo*, ChecklistElementRuleDetailInfo*> paryDetails;	//the array of rule details
	BOOL bApproved;				//has this rule been approved? (EMChecklistRulesT.Approved)
	COleDateTime dtApproved;	//when was it approved? (EMChecklistRulesT.ApprovedDate)
	long nApprovalUserID;			//who approved it - user ID (EMChecklistRulesT.ApprovedBy)
	CString strApprovalUserName;	//who approved it - username

	BOOL bPassed;				//is there enough elements to satisfy this rule? (patient EMN checklists only) ((j.jones 2007-08-29 12:13) - PLID 27056)

	ChecklistColumnInfo *pColumnInfo;	//pointer to the parent column
	ChecklistCodingLevelInfo *pRowInfo;	//pointer to the parent row
};

struct ChecklistInfo {	//contains the entire contents of a checklist

	long nID;					//the checklist ID (EMChecklistsT.ID)

	//the array of columns for the checklist
	CArray<ChecklistColumnInfo*,ChecklistColumnInfo*> paryColumns;

	//the array of coding info rows for the checklist
	CArray<ChecklistCodingLevelInfo*,ChecklistCodingLevelInfo*> paryCodingLevelRows;

	//the array of element rule cells for the checklist
	CArray<ChecklistElementRuleInfo*,ChecklistElementRuleInfo*> paryRules;
};

// (j.jones 2007-09-27 17:50) - PLID 27547 - added tracked category information
struct ChecklistTrackedCategoryDetailInfo {

	CEMNDetail *pDetail;		//pointer to a detail that contributes to a category calculation
	long nElementsFound;		//cached return value of CEMNDetail::CalculateEMElements()
};

struct ChecklistTrackedCategoryInfo {

	long nCategoryID;	//EMCodeCategoryT.ID
	long nTotalElementsFound;	//the number of elements for this category on an EMN

	CArray<ChecklistTrackedCategoryDetailInfo*,ChecklistTrackedCategoryDetailInfo*> aryDetailInfo;
};

//Functions that give information related to EmrActionObjects;
CString GetEmrActionObjectName(EmrActionObject Type, BOOL bLevel2);
//TES 12/6/2006 - PLID 23724 - I renamed these three functions to put "Source" in the name, because these are only valid when
// called on SourceTypes and SourceIDs, not DestTypes (because eaoEmrItem now references a different table when it is the Dest).
// These were only being called on Sources anyway.
CString GetEmrActionObjectSourceTable(EmrActionObject Type);
CString GetEmrActionObjectSourceIdField(EmrActionObject Type);
CString GetEmrActionObjectSourceNameField(EmrActionObject Type);
// (j.jones 2012-08-27 12:17) - PLID 52091 - added a dest name function
CString GetEmrActionObjectDestNameField(EmrActionObject Type);

//Functions that check for invalid merge field or item names
EmrInvalidNameReason IsValidEMRMergeFieldName(const CString& strName);

//Functions needed when performing merges with Microsoft Words
void GetEmrCategoryMergeFieldList(CString& strFilledFields, CString& strEmptyFields);
void GetEmrItemMergeFieldList(const CString& strWhereClause, CString& strFilledFields, CString& strEmptyFields);

// Returns true when if nEMRInfoID is one-per-emn, our existing EMR template
// configuration would cause violations.
BOOL EmbeddedSpawnsViolateOnePerEmnRule(long nEMRInfoID, CStringArray& astrOffendingMints);
BOOL EmbeddedSpawnsViolateOnePerEmnRule(long nMintID, // The level 1 mint
										const CArray<long, long>& aEMRItemIDs, // The EMRItemID's of the level 1 mint
										const CArray<long, long>& aSpawningMints); // All of the mints that will be spawned

//TES 12/7/2006 - This function doesn't appear to be used anywhere.
// Functions needed when spawned MINTs need to be considered based
// on default values of MINTs. This function ensures every MINT
// in the array has its spawned counterpart in the array, too.
//void GetEMRMINTListWithActions(CDWordArray& adwEMRMINTIDs);

// Prints an EMN report
void PrintEMNReport(long nEMNID);

//Available fields to be displayed in the header of an EMR chart note.
enum EmrHeaderField {
	ehfPatientNameFML = 1,
	ehfPatientNameLFM = 2,
	ehfEmrDate = 3,
	ehfPatientBirthDate = 4,
	ehfPatientAge = 5,
	ehfProviderName = 6,
};

CString GetHeaderSql(long nEmrID);
CString GetHeaderFieldDataField(EmrHeaderField ehfField);
CString GetHeaderFieldDisplayName(EmrHeaderField ehfField);

#define		DATA_FIELD			"<Data>"
#define		AGE_FIELD			"<Age>"
#define		GENDER_UPPER_FIELD	"<Male/Female>"
#define		GENDER_LOWER_FIELD	"<male/female>"
#define		SUBJ_UPPER_FIELD	"<He/She>"
#define		SUBJ_LOWER_FIELD	"<he/she>"
#define		OBJ_UPPER_FIELD		"<Him/Her>"
#define		OBJ_LOWER_FIELD		"<him/her>"
#define		POSS_UPPER_FIELD	"<His/Her>"
#define		POSS_LOWER_FIELD	"<his/her>"
#define		SPAWNING_FIELD		"<Spawning Item>"
// (z.manning 2010-08-04 10:11) - PLID 39497 - Added fields for groups
#define		GROUP_BEGIN_FIELD	"<Group Begin>"
#define		GROUP_END_FIELD		"<Group End>"
#define		GROUP_PLACEHOLDER	"<|TMP_GROUP_PLACEHOLDER|>"
// (z.manning 2010-09-02 09:44) - PLID 40206 - Added a field for the row name (col name on flipped tables)
#define		ROW_NAME_FIELD		"<Row Name>"
#define		COLUMN_NAME_FIELD	"<Column Name>"
// (z.manning 2011-11-03 15:17) - PLID 42765 - Added field for spawned items
#define		SPAWNED_ITEMS_FIELD	"<Spawned Item(s)>"

//TES 2/22/2010 - PLID 37463 - Shareable utility function for pulling the values for the various demographic fields.
// NOTE: This only includes those fields that are valid for both Smart Stamp tables and regular tables.  That is, it does not include
// DATA_FIELD (which Smart Stamps don't use), as well as the 4 fields for Smart Stamps.
CString GetLongFormField(CEMNDetail *pDetail, const CString &strField, CEMN *pParentEMN);
// (z.manning 2010-07-29 16:20) - PLID 39842 - Function that will go through and replace all of the standard long form fields.
void ReplaceLongFormDemographicFields(IN OUT CString &strOutput, CEMNDetail *pDetail, CEMN *pParentEMN);

// (z.manning 2010-08-02 16:10) - PLID 39842 - Function to replace the <Data> field in a table's sentence format with
// the appropriate default column-based sentence format.
void ReplaceTableLongFormDataField(IN OUT CString &strOutput, CEMNDetail *pDetail);

// (z.manning 2010-08-10 12:52) - PLID 39497 - Will substitute any table sentence format "groups" with the
// applicable sentence format.
// (a.walling 2011-06-20 17:56) - PLID 44215 - Need to keep track of set of data IDs to pass on to the recursive GetTableDataOutputRaw call
//TES 3/22/2012 - PLID 48203 - Added an optional pParentEMN parameter
BOOL ReplaceSentenceFormatGroupsWithPlaceholders(CEMNDetail *pDetail, IN OUT CString &strLongForm, bool bUseHTML, OUT CStringArray &arystrReplacementText, TableRowID* pSingleTableRowID, CArray<long,long>* paTableEmrDataIDs, CEMN *pParentEMN = NULL);

// (z.manning 2011-04-05 10:37) - PLID 42337 - Moved EmrInfoType enum to SharedEmrUtils.h

//TES 10/12/2010 - PLID 40907 - Moved enum EmrInfoSubType to SharedEmrUtils.h

// (a.walling 2007-04-12 11:22) - PLID 25605 - Struct provides info for rendering images + ink without a merge engine, so we can
	// control the files themselves and the sizing.
struct CEmrPreviewImageInfo
{
	long nMaxWidth = 0; // the maximum width that does not cause upsampling
	CString strTempFile;
};

// Functions that give information related to EmrInfoTypes
// (a.walling 2007-03-12 09:44) - PLID 19884 - Return the name of the item type given the enum
// (a.walling 2007-04-05 16:33) - PLID 25454 - Optional support for subtypes
CString GetDataTypeName(EmrInfoType eitDataType, EmrInfoSubType eistSubType = eistNone);

// (a.walling 2007-04-03 15:41) - PLID 25454 - CSS class for datatype
// (a.walling 2007-04-03 15:41) - PLID 25454 - CSS class for topic completion status
// (b.cardillo 2012-03-28 21:54) - PLID 42207 - (additional) Removed this functionality because it's never called.

//TES 6/5/2006 - If loading from a thread, pass in the thread's connection.
// (j.jones 2007-08-02 10:14) - PLID 26912 - added lprsInfo as a required parameter
// (z.manning 2011-02-24 12:14) - PLID 42579 - This now takes an array of detail IDs
_variant_t LoadEMRDetailState(ADODB::_Recordset *lprsInfo, CArray<long,long> &arynDetailIDs, EmrInfoType nEMRInfoDatatype, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

// (z.manning 2011-10-21 16:59) - PLID 44649 - Added bUseUndefinedImageType
_variant_t LoadEMRDetailStateBlank(EmrInfoType nDatatype, BOOL bUseUndefinedImageType = FALSE);

// (j.jones 2007-08-01 14:06) - PLID 26905 - added lprsInfo as a required parameter
// (j.jones 2008-09-22 15:20) - PLID 31408 - added nEMRGroupID as a parameter
// (z.manning 2011-11-16 12:21) - PLID 38130 - Removed default parameters and added an output parameter for 
// the remembered detail ID from which we loaded the state.
_variant_t LoadEMRDetailStateDefault(ADODB::_Recordset *lprsInfo, long nEmrInfoID, long nPatientID, long nEMRGroupID, long nEmrTemplateDetailID, IN ADODB::_Connection *lpCon, OUT long &nRememberedDetailID);

// (j.jones 2006-08-22 12:12) - PLID 22157 - When remembering a past item's value, and perhaps other places in the future, 
// we would need to load an old InfoIDs state into a new InfoID. This function will do that.
// (z.manning 2011-02-24 12:15) - PLID 42579 - This now takes an array of detail IDs and info IDs
// (z.manning 2011-10-20 13:20) - PLID 44649 - Added info sub type param
_variant_t LoadEMRDetailStateFromOldInfoID(CArray<long,long> &arynOldEMRDetailIDs, CArray<long,long> &arynOldEMRInfoIDs, long nNewEMRInfoID, EmrInfoType nEMRInfoDatatype, EmrInfoSubType eEMRInfoSubType, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

// (z.manning 2011-02-24 17:06) - PLID 42579
// (z.manning 2011-10-20 13:20) - PLID 44649 - Added info sub type param
// (z.manning 2011-11-16 12:52) - PLID 38130 - Added output param for the remembered detail ID
BOOL TryLoadDetailStateFromExistingByInfoID(const long nEmrInfoID, const EmrInfoType eInfoType, const EmrInfoSubType eInfoSubType, const long nPatientID, const long nEmrGroupID, BOOL bRememberForEmr, ADODB::_Connection *lpCon, OUT _variant_t &varState, OUT long &nRememberedDetailID);

// (z.manning 2011-02-24 13:35) - PLID 42579
CString LoadTableStateFromRecordset(ADODB::_Recordset *prsTableStates, BOOL bFromOldInfoID, ADODB::_Connection *lpCon);


// (z.manning 2011-02-24 15:54) - PLID 42579
CString MapDropdownIDTextToNewDataID(LPCTSTR strDropdownData, const long nNewDataID_Y, ADODB::_Connection *lpCon);

//TES 12/5/2006 - PLID 23724 - This function is no longer necessary in the new structure.
//recursively generates a list of this Info ID plus all previous versions of that Info ID
//CString GeneratePastEMRInfoIDs(long nEmrInfoID);
//recursively generates a list of this Data ID plus all previous versions of that Data ID
CString GeneratePastEMRDataIDs(long nEmrDataID);

//returns true if the datatypes are the same or if the types are both lists
// (j.jones 2007-07-23 09:16) - PLID 26773 - added connection pointer for use in threads
//DRT 9/7/2007 - PLID 27330 - Send in the new data type, we already know it.
// (z.manning 2011-10-20 13:32) - PLID 44649 - Added data sub type param
BOOL AreInfoIDTypesCompatible(long nOldEMRInfoID, long nNewEMRInfoID, EmrInfoType eitNewInfoDataType, EmrInfoSubType eNewInfoDataSubType, OPTIONAL IN ADODB::_Connection *lpCon = NULL);
// (z.manning 2011-10-21 14:44) - PLID 44649 - This determines if an info item is the same type (which is no
// longer as simple as comparing EmrInfoT.DataType). This is not the same as AreInfoIDTypesCompatible
BOOL IsSameEmrInfoType(const EmrInfoType eOldDataType, const EmrInfoSubType eOldDataSubType, const EmrInfoType eNewDataType, const EmrInfoSubType eNewDataSubType);
// (z.manning, 07/26/2007) - PLID 26574 - Based on the old and new info types, determines if
// the change requires we clear the state for any details for the info item.
// (z.manning 2011-10-21 15:02) - PLID 44649 - Added subtypes
BOOL InfoTypeChangeRequiresDetailStateReset(EmrInfoType eOldInfoType, EmrInfoSubType eOldInfoSubType, EmrInfoType eNewInfoType, EmrInfoSubType eNewInfoSubType, CEMNDetail *pDetail);

//given an EMRInfoID and data ID, map it up to the newest version of that Info ID and data ID
// (j.jones 2007-07-23 09:16) - PLID 26773 - added connection pointer for use in threads
long CalculateMappedDataID(long nOldEMRDataID, long nNewEMRInfoID, OPTIONAL IN ADODB::_Connection *lpCon = NULL);
//given an EMRInfoID and table dropdown ID, map it up to the newest version of that Info ID and dropdown ID
// (j.jones 2007-07-23 09:16) - PLID 26773 - added connection pointer for use in threads
long CalculateMappedDropdownID(long nOldDropdownID, long nNewEMRDataID_Y, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

// (b.cardillo 2006-11-20 10:37) - PLID 22565 - Returns the appropriate color given a value 
// from ReconstructedEMRDetailsT.ReviewStatus.
COLORREF GetReconstructedEMRDetailReviewStateColor(long nReviewStatus);
//DRT 6/4/2008 - PLID 30269
HBRUSH GetReconstructedEMRDetailReviewStateBrush(long nReviewStatus);

#define EMR_BUILT_IN_INFO__IMAGE		(-26)
// (c.haag 2008-06-16 10:48) - PLID 30319 - We now support instantiating pre-filled simple text details
#define EMR_BUILT_IN_INFO__TEXT_MACRO	(-27)

// (j.jones 2013-08-07 15:33) - PLID 42958 - added an enum for use in the
// OnInsertStockEmrItem function and NXM_INSERT_STOCK_EMR_ITEM
enum StockEMRItem
{
	seiInvalid = 0,
	seiSignatureImage,
	seiAnotherUsersSignatureImage,
	seiTextMacro,
};

BOOL ValidateEMRInfoName(IN const CString &strName, OPTIONAL IN const CString &strOldName, long nEMRInfoID, IN bool bIsNewInfoName);
BOOL ValidateEMRInfoNameMergeField(IN const CString &strName, OPTIONAL IN const CString &strOldName, long nEMRInfoID, OUT BOOL &bPromptWasGiven);

//Written to data, so be careful.
enum FollowupIncrement {
	fiDays = 1,
	fiWeeks = 2,
	fiMonths = 3,
};

CString GetDisplayName(FollowupIncrement fiUnit);

//////////////////////////
// CEmrAction is essentially an initializeable struct to store the properties of an action
class CEmrAction
{
public:
	CEmrAction(long nDestType, long nDestID, long nSortOrder, bool bPopup) : m_nDestType(nDestType), m_nDestID(nDestID), m_nSortOrder(nSortOrder), m_bPopup(bPopup) { };
public:
	//TES 1/30/2007 - PLID 24474 - This function is never called any more.
	/*CString GenerateXml(OPTIONAL LPCTSTR strArbitraryValue = NULL) const
	{
		return FormatString("<A %s%s%sDestType=\"%li\" DestID=\"%li\" SortOrder=\"%li\" Popup=\"%i\" SpawnAsChild=\"%i\"/>\r\n", 
			strArbitraryValue ? "ArbVal=\"" : "", 
			strArbitraryValue ? ConvertToQuotableXMLString(strArbitraryValue) : "", 
			strArbitraryValue ? "\" " : "", 
			m_nDestType, m_nDestID, m_nSortOrder, m_bPopup?1:0, m_bSpawnAsChild?1:0);
	};*/
public:
	long m_nDestType;
	long m_nDestID;
	long m_nSortOrder;
	bool m_bPopup;
};

// CEmrActionArray is an array of pointers to CEmrAction objects; automatically frees the objects on destruction
class CEmrActionArray : public CArray<CEmrAction *, CEmrAction *> 
{
public:
	~CEmrActionArray();

public:
	// (a.walling 2010-03-09 14:25) - PLID 37640 - Moved to cpp
	void RemoveAllAndFreeEntries();
public:
	void AppendCopy(const CEmrActionArray &aryeiaCopyFrom);
public:
	long Find(long nDestType, long nDestID, long nSortOrder, bool bPopup) const;
	long Find(const CEmrAction &ActionToFind) const;
	//TES 1/30/2007 - PLID 24474 - This function is never called any more.
	/*CString GenerateXml(BOOL bIncludeIndexAsArbitraryVal = FALSE) const
	{
		CString strAns;
		for (long i=0; i<GetSize(); i++) {
			strAns += GetAt(i)->GenerateXml(bIncludeIndexAsArbitraryVal ? (LPCTSTR)AsString(i) : NULL);
		}
		return strAns;
	};*/
protected:
	void RemoveAll(); // Intentionally not impelmented because it doesn't deallocate the memory.  Call RemoveAllAndFreeEntries() instead.
};
//////////////////////////

//Begin CEmrDetailSource declaration
class CEmrDetailSource
{
public:
	//TES 7/7/2004: This enum is now stored to data.  So hands off!
	// All the possible sources of an EMR Detail
	enum EDetailSourceType {
		dstInvalid = 0,				// This value indicates an error condition, it has no meaning other than that.
		
		dstSpawnedByProcedure = 1,	// (m_nSourceID == <ProcedureT.ID>)		The user chose a template-less procedure and that procedure included this info item, so it was added to the emr.
		// (b.savon 2014-07-14 10:56) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
		//dstSpawnedByDiagCode = 2,	// (m_nSourceID == <DiagCodes.ID>)		The user chose an ICD-9 code and that ICD-9 code included this info item, so it was added to the emr.
		dstSpawnedByData = 3,		// (m_nSourceID == <EMRDataT.ID>)		The user chose a certain data element on some other detail and that data element had an action associated with it to spawn this detail.
		dstSpawnedByServiceCode = 4,// (m_nSourceID == <CptCodeT.ID>)		The user chose a service code and that service code included this info item, so it was added to the emr.
		dstSpawnedByInfo = 5,		// (m_nSourceID == <EMRInfoT.ID>)		The user chose some other detail and that detail's info had an action that asked the emr to add this detail.
		dstCopiedFromTemplate = 6,	// (m_nSourceID == <EMRTemplateT.ID>)	The user chose a template and so all the details specified by that template were added to the emr, and this detail is one of those.
		dstEnteredByUser = 7,		// (m_nSourceID == -1)					The user manually chose a new info item, thus creating this detail.
		dstPhoto = 8,				// (m_nSourceID == -1)					The user created this EMR by clicking on a photo in the Photos sub-tab in the History tab of Patients, and this detail represents that image.
		dstUnknownSource = 9,		// (m_nSourceID == -1)					Either this detail was created before we stored the source, or its link has been broken somehow (the latter isn't possible as of 7/7/04, but may be some day).
		//DRT 1/23/2008 - PLID 28690
		dstSpawnedByHotSpot = 10,	// (m_nSourceID == <EMRImageHotSpotsT.ID>)	The user clicked a hotspot on an image.
		// (z.manning 2009-02-10 15:39) - PLID 33026
		dstSpawnedByTableDropdownItem = 11, // (m_nSource == <EMRTableDropdownInfoT.ID>) The user selected an EMR table dropdown item.
		dstSpawnedBySmartStamp = 12,// (m_nSourceID == <EmrImageStampsT.ID>) The user stamped on a smart stamp image
		// (r.gonet 08/03/2012) - PLID 51949
		dstSpawnedByWoundCareCalculator = 13,	// (m_nSourceID == <WoundCareConditionT.ConditionID>)	The user chose to run Wound Care Auto Coding and certain conditions were met with a table being used with Wound Care Coding, so this was added.
		// (b.savon 2014-07-14 10:56) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
		dstSpawnedByDiagnosis = 14, // (m_nSourceID == <EmrActionDiagnosisDataT.EmrActionID>) The user chose and ICD-9, ICD-10, or ICD-9/ICD-10 combination code and that code included this info item, so it was added to the emr.

		// (b.cardillo 2004-06-16 13:59) - As time goes by we may offer other ways for emr details to be created.  Whenever a new way is invented, a corresponding enum value should be added here.
	};
	
public:
	CEmrDetailSource(EDetailSourceType edst, long nSourceID);
	CEmrDetailSource(EmrActionObject eaoSourceType, long nSourceID);

public:
	BOOL IsEqual(const CEmrDetailSource &eds) const;

	static EDetailSourceType CalcSourceType(EmrActionObject eaoSource);

	CString GetDisplayName();

public:
	EDetailSourceType m_edstSourceType;
	long m_nSourceID;
};
//////////////////////////////////////////////////////////////////////////

//Call these overrides if you have already loaded the information about the detail
// (c.haag 2007-03-16 11:52) - PLID 25242 - We now support an optional EMN parameter in case the detail has no parent topic
// (c.haag 2007-03-28 15:06) - PLID 25397 - We also optionally pass in the data output for optimizing the
// execution of functions that need to get both individually
// (a.walling 2007-04-12 11:41) - PLID 25605 - Pass the CEmrPreviewImageInfo to handle images without merge engines
// (c.haag 2008-02-22 10:56) - PLID 29064 - Pass in optional connection pointer
CString GetSentence(CEMNDetail *pDetail, CMergeEngine *pMi, bool bAllowHtml, bool bInHtml, CStringArray &saTempFiles, EmrCategoryFormat Format = ecfParagraph, CEMN* pParentEMN = NULL, LPCTSTR szDataOutput = NULL, CEmrPreviewImageInfo* pPreviewImageInfo = NULL, OPTIONAL IN ADODB::_Connection *lpCon = NULL);
// (a.walling 2010-03-26 18:04) - PLID 37923 - Limit on a specific table row
CString GetDataOutput(CEMNDetail *pDetail, CMergeEngine *pMi, bool bAllowHtml, bool bInHtml, CStringArray &saTempFiles, OPTIONAL OUT bool *pbDataIsHtml = NULL, EmrCategoryFormat Format = ecfParagraph, CEMN* pParentEMN = NULL, CEmrPreviewImageInfo* pPreviewImageInfo = NULL, OPTIONAL IN ADODB::_Connection *lpCon = NULL, TableRowID* pSingleTableRowID = NULL);

//Call these overrides if you just have the detail ID, it will load the information and call the above override.
// (c.haag 2007-03-16 11:52) - PLID 25242 - We now support an optional EMN parameter in case the detail has no parent topic
// (c.haag 2007-03-28 15:06) - PLID 25397 - We also optionally pass in the data output for optimizing the
// execution of functions that need to get both individually
// (a.walling 2007-04-12 11:42) - PLID 25605 - Pass the CEmrPreviewImageInfo to handle images without merge engines
// (c.haag 2008-02-22 10:56) - PLID 29064 - Pass in optional connection pointer
CString GetSentence(long nEmrDetailID, CMergeEngine *pMi, bool bAllowHtml, bool bInHtml, CStringArray &saTempFiles, EmrCategoryFormat Format = ecfParagraph, CEMN* pParentEMN = NULL, LPCTSTR szDataOutput = NULL, CEmrPreviewImageInfo* pPreviewImageInfo = NULL, OPTIONAL IN ADODB::_Connection *lpCon = NULL);
// (a.walling 2010-03-26 18:04) - PLID 37923 - Limit on a specific table row
CString GetDataOutput(long nDetailID, CMergeEngine *pMi, bool bAllowHtml, bool bInHtml, CStringArray &saTempFiles, OPTIONAL OUT bool *pbDataIsHtml = NULL, EmrCategoryFormat Format = ecfParagraph, CEMN* pParentEMN = NULL, CEmrPreviewImageInfo* pPreviewImageInfo = NULL, OPTIONAL IN ADODB::_Connection *lpCon = NULL, TableRowID* pSingleTableRowID = NULL);

// (c.haag 2008-02-22 10:47) - PLID 29064 - We now support an optional connection pointer
// (a.walling 2010-03-26 18:02) - PLID 37923 - Takes a TableRowID* to limit output to a specific table row
// (z.manning 2010-08-09 15:21) - PLID 39842 - Added bAllowHtml
//TES 3/22/2012 - PLID 48203 - Added an optional pParentEMN parameter
void GetTableDataOutput(CEMNDetail *pDetail, EmrMultiSelectFormat emsf, CString strSeparator, CString strSeparatorFinal, CString &strHTML, CString &strNonHTML, CMergeEngine *pMi, bool bAllowHtml, CStringArray &saTempFiles, OPTIONAL OUT bool *pbDataIsHtml = NULL, EmrCategoryFormat Format = ecfParagraph, OPTIONAL IN ADODB::_Connection *lpCon = NULL, TableRowID* pSingleTableRowID = NULL, CEMN* pParentEMN = NULL);

void ConvertToEMNDetail_Local(CEMNDetail *detail, CEMNDetail *pEMNDetail, long nPatientID, long nEmnID, BYTE cbGender, long nAge);

//This function will look at the given EMN ID, see if it's finished, partially finished, or not finished, and
//give you the color of that EMN.
// (j.jones 2007-06-14 09:10) - PLID 26276 - given an EMNCompletionStatus, and locked status,
// simply determine the appropriate color
// NOW OBSOLETE
//COLORREF CalculateEMNBackgroundColorFromStatus(EMNCompletionStatus ecsStatus, BOOL bIsEMNLocked);
// (j.jones 2006-06-01 10:39) - this accepts an alternative connection for when it is called in a thread
// (j.jones 2007-06-14 09:10) - PLID 26276 - given an ID of an EMN that has no status filled in,
// determine the status, save to data, then return the color
// NOW OBSOLETE
//EMNCompletionStatus CalculateEMNBackgroundStatusFromID(long nEMNID, ADODB::_ConnectionPtr pCon = GetRemoteData());
// (j.jones 2007-06-14 09:18) - PLID 26276 - given an ID of an EMN that has no status filled in,
// determine the status, and return it
// NOW OBSOLETE
//EMNCompletionStatus CalculateEMNCompletionStatus(long nEMNID, ADODB::_ConnectionPtr pCon = GetRemoteData());

// (a.walling 2009-01-13 14:53) - PLID 32107 - IsDetailStateSet is now deprecated; instead, call CEMNDetail::IsStateSet.
//BOOL IsDetailStateSet(EmrInfoType nDatatype, const _variant_t &varState);

CString ConvertToHeaderName(const CString &strPrefix, const CString &strHeaderBaseText, OPTIONAL OUT BOOL *pbTruncated = NULL);

CString GenerateXMLFromSemiColonDelimitedIDList(const CString &strSemiColonDelimitedIDList);
void FillArrayFromSemiColonDelimitedIDList(IN OUT CDWordArray &arydw, IN const CString &strSemiColonDelimitedIDList);
BOOL IsIDInSemiColonDelimitedIDList(IN const CString &strSemiColonDelimitedIDList, IN const long nLookForID);
long RemoveIDFromSemiColonDelimitedIDList(IN OUT CString &strSemiColonDelimitedIDList, IN const long nRemoveID);
long FindInList(const CDWordArray &arydw, DWORD dwFindElement);
void CalcChangedDataIDFromState(const _variant_t &varOldState, const _variant_t &varNewState, OUT CArray<long, long> &aryNewlySelDataID, OUT CArray<long, long> &aryNewlyUnselDataID);

enum EmrSaveObjectType {
	esotEMR = 1,
	esotEMN,
	esotCharge,
	esotPrescription,
	esotTopic,
	esotDetail,
	esotTableDefault,
	esotDetailData,
	esotDeletedEMN,
	esotProblem,
	esotDiagCode,	// (j.jones 2008-07-23 11:14) - PLID 30819
	esotProblemLink, // (z.manning 2009-05-21 17:05) - PLID 34297
	esotDetailImageStamp, // (z.manning 2010-02-18 14:41) - PLID 37404
};

Nx::Quantum::Batch GenerateEMRBatchSaveNewObjectTableDeclaration();

// (c.haag 2007-06-20 12:14) - PLID 26397 - We now store saved objects in a map for fast lookups
void AddNewEMRObjectToSqlBatch(IN OUT Nx::Quantum::Batch& strSaveString, EmrSaveObjectType esotSaveObject, long nObjectPtr, IN CMapPtrToPtr& mapSavedObjects);
//For DeletedEMN records, gives the id rather than the object pointer.
// (a.walling 2013-03-14 11:05) - PLID 55652 - Get the object pointer as well since we use that as a primary key in #NewObjectsT
void AddDeletedEMRObjectToSqlBatch(IN OUT Nx::Quantum::Batch& strSaveString, EmrSaveObjectType esotSaveObject, long nObjectPtr, long nObjectID);

long CreateNewEMRCollection();

//Saves the given EMR object to the database, updating its IDs if you ask it to, and of course doing whatever auditing/tracking etc. is appropriate.
EmrSaveStatus SaveEMRObject(EmrSaveObjectType esotSaveType, long nObjectPtr, BOOL bShowProgressBar);
// (j.jones 2012-09-27 15:11) - PLID 52820 - now we track a flag if something that contributed to drug interactions has changed,
// such as new or deleted prescriptions, or new or deleted diagnosis codes
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
EmrSaveStatus SaveEMRObject(EmrSaveObjectType esotSaveType, long nObjectPtr, BOOL bShowProgressBar, OUT BOOL &bDrugInteractionsChanged, IN OUT CDWordArray &arNewCDSInterventions);

// (j.jones 2009-08-12 08:55) - PLID 35189 - this takes in a datalist2 row now
BOOL CALLBACK AddItemToTabMultiSelectContextMenuProc(IN CMultiSelectDlg *pwndMultSelDlg, IN LPARAM pParam, IN NXDATALIST2Lib::IRowSettings *lpRow, IN CWnd* pContextWnd, IN const CPoint &point, IN OUT CArray<long, long> &m_aryOtherChangedMasterIDs);

//TES 12/7/2006 - PLID 23724 - Updated to copy the EmrInfoMasterT record, along with its ActiveEmrInfoID record.
//Copies the item specified by nInfoMasterID, returns TRUE if it was successfully copied, in which case nNewID and strNewName will identify the new item.
BOOL CopyEmrInfoItem(IN long nInfoMasterID, IN const CString &strName, BOOL bSilent, OPTIONAL OUT long * pnNewInfoMasterID = NULL, OPTIONAL OUT long *pnNewActiveInfoID = NULL, OPTIONAL OUT CString *pstrNewName = NULL);

//Call this after CopyEmrInfoItem to copy relationships in DiagCodeToEMRInfoT and
//ProcedureToEMRInfoT from the old to the new info record. Returns true on success.
BOOL CopyEmrInfoItemRelationships(IN long nOldInfoID, IN long nNewInfoID);

//Call this to inactivate an EMR Info item. Returns true on success.
//TES 12/14/2006 - PLID 23792 - It is now EmrInfoMasterT records that are activated/inactivate.
BOOL InactivateEmrInfoMasterItem(IN long nID);

//Call this to activate an EMR Info item. Returns true on success.
//TES 12/14/2006 - PLID 23792 - It is now EmrInfoMasterT records that are activated/inactivate.
BOOL ActivateEmrInfoMasterItem(IN long nID);

//TES 12/6/2006 - PLID 23724 - This is superceded by DeleteEmrInfoMasterItem
//Call this to delete an EMR Info item. Returns true on success.
//BOOL DeleteEmrInfoItem(IN long nID);

BOOL DeleteEmrInfoMasterItem(IN long nID);

// (z.manning 2013-03-11 10:42) - PLID 55554
void GetDeleteEmrDataSql(IN CSqlFragment &sqlEMRDataIDsForInClause, IN OUT CSqlFragment &sql);

// (j.jones 2007-05-21 10:54) - PLID 26061 - added a batch parameter
// (j.jones 2013-07-22 16:35) - PLID 57277 - this now returns a Sql fragment
CSqlFragment DeleteEMRAction(long nActionID);

// (z.manning 2009-03-17 10:53) - PLID 33242
// (z.manning 2013-03-11 10:52) - PLID 55554 - use a CSqlFragment
// (j.jones 2015-12-22 11:39) - PLID 67769 - moved to be inside GetDeleteEmrDataSql
//void GetSqlToClearSourceDataGroupIDsByDataID(IN CSqlFragment &sqlEMRDataIDsForInClause, IN OUT CSqlFragment &sql);

BOOL IsDataIDUsedBySpawnedObject(const CString strDataIDClause);

//recursively deletes template topics and details
void DeleteEMRTemplateTopic(long nEMRTemplateTopicID, Nx::Quantum::Batch &strSqlBatch);
// (j.jones 2007-01-12 10:54) - PLID 24027 - recursively deletes details and topics
void DeleteEMRTemplateDetail(long nEMRTemplateDetailID, Nx::Quantum::Batch &strSqlBatch);

enum EmrTreeRowType {
	etrtInvalid = -1, //No row/invalid row
	etrtEmn = 0, //Top-level EMN
	etrtTopic = 1, //Topic node
	etrtMoreInfo = 2, //More info node
	etrtPlaceholder = 3, //Placeholder for topics that haven't been loaded.
	etrtCodes = 4, //TES 2/12/2014 - PLID 60748 - <Codes> topic
};

// (c.haag 2008-07-17 13:49) - PLID 30723 - Structure for EMR problem actions. An EMR action
// may be associated with one or more of these.
struct EmrProblemAction
{
	long nID = -1;
	CString strDescription;
	long nStatus = -1;
	BOOL bSpawnToSourceItem = FALSE;
	long nSNOMEDCodeID = -1; // (c.haag 2014-07-22) - PLID 62789
	// (r.farnworth 2014-08-19 14:51) - PLID 62787
	long nDiagICD9CodeID;
	long nDiagICD10CodeID;
	// (s.tullis 2015-02-23 17:47) - PLID 64749 - Do Not show on CCDA
	BOOL bDoNotShowOnCCDA;
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added bDoNotShowOnProblemPrompt. True to have the problem show in the prompt when we switch patients
	// or go to the EMR tab. False to not show in the prompt.
	BOOL bDoNotShowOnProblemPrompt;
	// (a.walling 2014-07-01 13:40) - PLID 62697 - Broken assignment operator removed

	// (c.haag 2014-07-22) - PLID 62789 - Returns the SNOMEDCodeID as an output value for SQL
	CString GetSNOMEDValueForSQL() const
	{
		return (-1 == nSNOMEDCodeID) ? "NULL" : FormatString("%li",nSNOMEDCodeID);
	}
	// (c.haag 2014-07-22) - PLID 62789 - Returns the SNOMEDCodeID formatted as an XML attribute
	CString GetSNOMEDValueForXML() const
	{
		return (-1 == nSNOMEDCodeID) ? "" : FormatString("SNOMEDCodeID=\"%li\"", nSNOMEDCodeID);
	}
	// (c.haag 2014-07-22) - PLID 62789 - Returns if the content between this and another problem action match
	// (s.tullis 2015-03-05 15:30) - PLID 64724 - Added Do not show on ccda
	// (r.gonet 2015-03-10 14:48) - PLID 65013 - Also compare the DoNotShowOnProblemPrompt value.
	BOOL DoesContentMatch(const EmrProblemAction &epa) const
	{
		return (epa.bSpawnToSourceItem == bSpawnToSourceItem
			&& epa.nStatus == nStatus
			&& epa.strDescription == strDescription
			&& epa.nSNOMEDCodeID == nSNOMEDCodeID
			&& epa.bDoNotShowOnCCDA == bDoNotShowOnCCDA
			&& epa.bDoNotShowOnProblemPrompt == bDoNotShowOnProblemPrompt
			) ? TRUE : FALSE;
	}
};

// (a.walling 2014-07-01 13:40) - PLID 62697 - Now a vector
typedef std::vector<EmrProblemAction> CProblemActionAry;

// (b.savon 2014-07-14 09:34) - PLID 62705 - Created for EmrAction composition
struct Diagnosis{
	long nDiagCodeID_ICD9 = -1;
	long nDiagCodeID_ICD10 = -1;
};

// (a.walling 2014-08-04 09:39) - PLID 62683 - Laterality - common logical structures and functions
namespace Emr
{
	// see AnatomySide
	// unlike AnatomySide, we need to be able to combine 'no side' with the others, so it can't be zero.
	enum WhichAnatomySide {
		sideNull = 0,
		sideLeft = 1,		// == asLeft
		sideRight = 2,		// == asRight
		sideNone = 4,
		sideAny = (sideLeft | sideRight | sideNone), // == 7

		sideBilateral = (sideLeft | sideRight), // == 3
	};

	// (a.walling 2014-08-08 09:56) - PLID 62685 - Laterality - Side / Qualifier text description generation
	CString DescribeWhichAnatomySide(long side);

	struct AnatomicLocationFilter
	{
		long anatomicLocationID = -1;
		long qualifierID = -1; // -1 == null by convention == any qualifier; 0 == none == no qualifier
		long anatomySide = sideNull; // bitset WhichAnatomySide

		AnatomicLocationFilter()
		{}
				
		AnatomicLocationFilter(long anatomicLocationID, long qualifierID, long anatomySide = sideNull)
			: anatomicLocationID(anatomicLocationID)
			, qualifierID(qualifierID)
			, anatomySide(anatomySide)
		{}

		friend bool operator==(AnatomicLocationFilter l, AnatomicLocationFilter r) {
			return l.anatomicLocationID == r.anatomicLocationID && l.qualifierID == r.qualifierID && l.anatomySide == r.anatomySide;
		}

		friend bool operator<(AnatomicLocationFilter l, AnatomicLocationFilter r) {
			if (l.anatomicLocationID < r.anatomicLocationID) {
				return true;
			}
			if (l.anatomicLocationID > r.anatomicLocationID) {
				return false;
			}
			if (l.qualifierID < r.qualifierID) {
				return true;
			}
			if (l.qualifierID > r.qualifierID) {
				return false;
			}
			// anatomicLocationID + qualifierID should be unique, let's not include the side in the comparison
			/*if (l.anatomySide < r.anatomySide) {
				return true;
			}
			if (l.anatomySide > r.anatomySide) {
				return false;
			}*/
			return false;
		}
	};

	struct ActionFilter
	{
		typedef boost::container::flat_set<AnatomicLocationFilter> AnatomicLocationFilterSet;
		AnatomicLocationFilterSet anatomicLocationFilters;

		std::string ToXml() const;
		static ActionFilter FromXml(const char* sz);

		long GetAnyQualifierSide(long anatomicLocationID) const;
		long GetNoQualifierSide(long anatomicLocationID) const;

		size_t count(long anatomicLocationID) const
		{
			size_t num = 0;

			for (
				auto it = anatomicLocationFilters.lower_bound({ anatomicLocationID, -1 });
				it != anatomicLocationFilters.end() && it->anatomicLocationID == anatomicLocationID;
				++it) 
			{
				++num;
			}

			return num;
		}

		std::pair<AnatomicLocationFilterSet::const_iterator, AnatomicLocationFilterSet::const_iterator> equal_range(long anatomicLocationID) const
		{
			auto itBegin = anatomicLocationFilters.lower_bound({ anatomicLocationID, -1 });
			auto itEnd = itBegin;
			while (itEnd != anatomicLocationFilters.end() && itEnd->anatomicLocationID == anatomicLocationID) {
				++itEnd;
			}
			return{ itBegin, itEnd };
		}

		bool empty() const
		{
			return anatomicLocationFilters.empty();
		}

		explicit operator bool() const
		{
			return !empty();
		}
	};

	// (a.walling 2014-08-08 09:56) - PLID 62685 - Returns map of qualifier names
	typedef boost::container::flat_map<long, CString> AnatomyQualifierMap;
	AnatomyQualifierMap MakeAnatomyQualifiersMap();

	// (a.walling 2014-08-08 09:56) - PLID 62685 - Laterality - Side / Qualifier text description generation
	CString DescribeFilter(const ActionFilter& filter, const AnatomyQualifierMap& qualifierMap, long anatomicLocationID);
}

struct EmrAction {
	long nID = -1;
	EmrActionObject eaoSourceType = eaoInvalid;
	long nSourceID = -1;
	EmrActionObject eaoDestType = eaoInvalid;
	long nDestID = -1;
	BOOL bPopup = FALSE;
	long nSortOrder = -1;
	BOOL bSpawnAsChild = FALSE;
	// (j.jones 2007-01-22 11:00) - PLID 24356 - added Deleted variable
	BOOL bDeleted = FALSE;
	// (j.jones 2007-07-16 15:41) - PLID 26694 - added bOnePerEmn variable
	BOOL bOnePerEmn = FALSE;

	// (z.manning 2011-11-09 10:18) - PLID 46367 - Added group IDs
	long nSourceDataGroupID = -1;
	long nSourceHotSpotGroupID = -1;
	long nSourceTableDropdownGroupID = -1;

	// (b.savon 2014-07-14 09:34) - PLID 62705 - Create a new DestType and structure for EmrActions that spawn and store a tuple of diagnosis codes
	Diagnosis diaDiagnosis;

	//DRT 1/9/2007 - PLID 24181 - These fields are really charge-specific.  I wanted to make
	//	a derived class, but pretty much every use of the EmrAction struct is an array of
	//	objects, and I can't pass in a derived class in place.  These should only be written
	//	or read to eaoCpt type objects.  The saving code for actions currently only generates
	//	save data if the DestType is eaoCpt.
	BOOL bPrompt = FALSE;
	double dblDefaultQuantity = 1.0;
	CString strMod1;
	CString strMod2;
	CString strMod3;
	CString strMod4;

	// (j.jones 2007-07-31 15:59) - PLID 26898 - added strSourceName
	CString strSourceName;

	// (c.haag 2008-06-03 11:02) - PLID 30221 - Todo-specific fields. These should only be weritten
	// or read to eaoTodo type objects.
	long nTodoCategoryID = -1;
	CString strTodoMethod;
	CString strTodoNotes;
	// (a.walling 2014-07-01 13:40) - PLID 62697 - just use a vector rather than an opaque pointer to a dynamic array...
	std::vector<long> anTodoAssignTo;
	long nTodoPriority = -1;
	long nTodoRemindType = -1;
	long nTodoRemindInterval = -1;
	long nTodoDeadlineType = -1;
	long nTodoDeadlineInterval = -1;

	// (z.manning 2011-06-28 12:10) - PLID 44347 - Actions may now be associated with specific anatomic locations.
	// Let's keep track of any anatomic locations IDs in an array.
	// (a.walling 2014-07-01 13:40) - PLID 62697 - now a vector
	// (a.walling 2014-08-04 10:50) - PLID 62684 - and now a type of its own!
	Emr::ActionFilter filter;

	// (c.haag 2008-07-17 13:51) - PLID 30723 - Problems that spawn with the action
	CProblemActionAry aProblemActions;

	// (a.walling 2014-07-01 13:40) - PLID 62697 - Broken assignment operator removed
};

// (c.haag 2007-04-25 11:08) - PLID 25774 - We now use a connection object
// (a.walling 2010-11-01 12:38) - PLID 40965 - Note that strWhere is inserted TWICE!
// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
void LoadActionInfo(CSqlFragment sqlWhere, OUT MFCArray<EmrAction> &arActions, ADODB::_ConnectionPtr pCon = GetRemoteData());
// (z.manning 2010-03-01 16:15) - PLID 37571
// (a.walling 2010-11-01 12:38) - PLID 40965 - Note that strWhere is inserted TWICE!
CSqlFragment GetLoadActionInfoQuery(CSqlFragment sqlWhere);
// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
void FillActionArray(ADODB::_RecordsetPtr rsInfo, ADODB::_ConnectionPtr pCon, OUT MFCArray<EmrAction> &arActions);

//Returns the "name" of the action, which is the name of the thing that spawned it.
// (j.jones 2007-07-31 09:43) - PLID 26882 - added optional parameters for source type and source ID
//TES 3/18/2010 - PLID 37530 - Added a parameter for the index of the spawning stamp, it's needed to calculate the name.
// (a.walling 2010-05-19 16:02) - PLID 38750 - Pass in the detail stamp ID
CString GetEmrActionName(long nActionID, long nSourceStampIndex, OPTIONAL IN EmrActionObject eaoSourceType = eaoInvalid, OPTIONAL IN long nSourceID = -1, OPTIONAL IN long nSourceDetailStampID = -1);

// (z.manning 2009-02-11 09:04) - PLID 33029 - Set the fields of an EmrAction from the given fields pointer.
void SetActionFromFields(IN ADODB::FieldsPtr pflds, OUT EmrAction &ea);

//determines if the source action ID from the detail is the same as the passed in action ID
//or whether the previous version of the action's spawning item is the item that spawns the detail
// (j.jones 2013-07-15 10:51) - PLID 57243 - changed the action to a reference
BOOL IsDetailSourceActionIDEquivalent(CEMNDetail *pDetail, const EmrAction &ea);
//determines if the source action ID from the topic is the same as the passed in action ID
//or whether the previous version of the action's spawning item is the item that spawns the topic
// (j.jones 2013-07-15 10:51) - PLID 57243 - changed the action to a reference
BOOL IsTopicSourceActionIDEquivalent(CEMRTopic *pTopic, const EmrAction &ea);

//DRT 1/17/2008 - PLID 28602 - I pulled the central Action XML bits out of GenerateXML below.  We now need to generate
//	this for both EMRDataT records and EMRImageHotSpotsT records.
// (z.manning 2009-02-11 15:18) - PLID 33029 - Moved to EmrUtils from EmrItemEntryDlg
// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
CString GenerateActionXml(IN const MFCArray<EmrAction> *pActions, IN OUT long &nArbitraryActionIDValue);

///////////////////////////////////////////////////////////////////////////////////////
// (z.manning 2009-02-18 12:04) - PLID 33141 - We used to use a source detail to help uniquely
// identify an action when the same info item (and thus same actions) existed on an EMN
// more than once. However, for table dropdown spawning a source detail isn't enough because
// the same dropdown elements exist in the same detail multiple times. To be able to 
// uniquely identify these actions we use a SourceDataGroupID as well. Since EMR action
// source stuff is now more complicated, I added this class to help keep things organized.
class SourceActionInfo
{
public:
	long nSourceActionID;
	EmrActionObject eaoSourceType;

	long nSourceDetailID;

	// (z.manning 2010-02-24 16:08) - PLID 37532 - Instead of tracking just source data group ID, we now track
	// a table row pointer which has that info plus other possible source spawning info.
	//long nSourceDataGroupID;
protected:
	TableRow *ptrSourceTableRow;

public:
	CEMNDetail *pSourceDetail;
	//TES 2/16/2010 - PLID 37375 - Added a variable for the source HotSpot
	CEMRHotSpot *pSourceSpot;

	SourceActionInfo();
	SourceActionInfo(EmrAction *pAction, CEMNDetail *pSrcDetail);
	SourceActionInfo(EmrAction *pAction, CEMNDetail *pSrcDetail, TableRow *pSrcTableRow);
	//TES 2/16/2010 - PLID 37375 - Overload that takes a source HotSpot
	SourceActionInfo(EmrAction *pAction, CEMNDetail *pSrcDetail, CEMRHotSpot *pSrcSpot, TableRow *pSrcTableRow);
	SourceActionInfo(EmrActionObject eaoType, const long nActionID, CEMNDetail *pSrcDetail, TableRow *pSrcTableRow);
	// (z.manning 2009-03-04 15:35) - PLID 33338 - Yet another constructor
	SourceActionInfo(EmrActionObject eaoType, const long nActionID, const long nDetailID, TableRow *pSrcTableRow);

	//TES 2/16/2010 - PLID 37375 - We keep our own copy of pSourceSpot, so add a copy constructor and = operator to maintain that copy,
	// and a destructor to free it.
	SourceActionInfo(const SourceActionInfo &saiSource);
	void operator =(const SourceActionInfo &saiSource);
	~SourceActionInfo();

	// (j.jones 2012-11-15 10:51) - PLID 52819 - added GetSourceDetailID, which is more reliable
	// if our source detail pointer has an ID but somehow our action does not
	long GetSourceDetailID();
	
	// (z.manning 2010-02-24 18:10) - PLID 37532
	TableRow* GetTableRow();
	long GetDataGroupID();
	void SetDataGroupID(const long nDataGroupID);
	long GetDetailStampID();
	//TES 3/17/2010 - PLID 37530
	void SetDetailStampID(const long nDetailStampID);
	// (z.manning 2010-02-26 17:14) - PLID 37540
	EmrDetailImageStamp* GetDetailStampPointer();
	void SetDetailStampPointer(EmrDetailImageStamp *pSourceDetailStamp);

	//TES 3/17/2010 - PLID 37530
	void SetGlobalStampIDAndIndex(const long nStampID, const long nStampIndex);
	long GetStampID();
	long GetStampIndexInDetailByType();

	// (z.manning 2011-11-02 16:55) - PLID 45993 - This function used to be nearly identical to HasSameSource.
	// I changed it to now only check source details, as its name implies.
	BOOL HasSameSourceDetail(const SourceActionInfo *psai);

	// (z.manning 2009-03-23 15:43) - PLID 33089
	// (z.manning 2011-11-02 16:43) - PLID 45993 - Added bCheckActionID
	BOOL HasSameSource(const SourceActionInfo& sai, BOOL bCheckActionID = FALSE);

	// (z.manning 2010-03-04 09:11) - PLID 37571
	BOOL TableSourceMatches(const SourceActionInfo& sai);

	// (a.walling 2010-04-05 16:04) - PLID 38060
	BOOL TableRowMatches(const SourceActionInfo& sai);

	// (a.walling 2009-04-23 09:14) - PLID 28957
	BOOL IsBlank();

	// (z.manning 2010-03-11 15:07) - PLID 37571
	void UpdateDetailStampPointerIfMatch(EmrDetailImageStamp *pDetailStampToMatch, EmrDetailImageStamp *pNewDetailStamp);

	// (z.manning 2010-12-08 11:56) - PLID 41731 - Returns true if the source action info refers to a table
	// row that was created by a smart stamp.
	BOOL IsSmartStampTableRowAction();

	// (z.manning 2010-12-08 14:03) - PLID 41731 - Generates a name for the action based on the action's source type.
	CString GenerateEmrActionName(CEMN *pParentEmn);

private:
	//TES 2/16/2010 - PLID 37375 - Added a parameter for the source HotSpot
	void Init(EmrAction *pAction, CEMNDetail *pSrcDetail, TableRow *pSrcTableRow, CEMRHotSpot *pSrcSpot);
};
// End Class SourceActionInfo
//////////////////////////////////////////////////////////////////////////////////////////////

// (j.jones 2010-02-10 11:12) - PLID 37224 - define EMR Image Stamps
struct EMRImageStamp {
	long nID;
	CString strStampText;
	CString strTypeName;
	CString strDescription;
	long nTextColor;
	EMRSmartStampTableSpawnRule eSmartStampTableSpawnRule;	// (j.jones 2010-02-16 12:09) - PLID 37365
	BOOL bInactive; // (j.jones 2010-02-16 15:22) - PLID 37377
	BOOL bShowDot; // (z.manning 2012-01-26 17:11) - PLID 47592
	// (r.gonet 05/02/2012) - PLID 49949 - Small grouping of image related fields.
	struct SImageInfo {
		BYTE *arImageBytes; // (r.gonet 05/02/2012) - PLID 49949 - Bytes in image associated with stamp. NULL if no image.
		long nNumImageBytes; // (r.gonet 05/02/2012) - PLID 49949 - Number of bytes in image.
	} m_ImageInfo;

	// (z.manning 2010-03-01 16:55) - PLID 37571 - Array of actions
	// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
	MFCArray<EmrAction> aryActions;
	// (b.spivey, August 14, 2012) - PLID 52130 - Need categoryID to load.
	long nCategoryID; 
	// (a.walling 2012-08-28 10:23) - PLID 51742 - Category name
	CString strCategoryName;
};


// (z.manning 2011-11-04 16:23) - PLID 42765 - Created a class that can be used to find spawned items
// based on source action info as well as the spawning source.
class CSpawningSourceInfo
{
public:
	SourceActionInfo *m_psai;
	CUnspawningSource *m_pus;

	CSpawningSourceInfo(SourceActionInfo *psai, CUnspawningSource *pus);

	BOOL SpawnedDetail(CEMNDetail *pDetail);
};

//TES 12/7/2006 - This function is no longer needed in the new structure.
//determines if two InfoIDs are for the same version history of an item
//BOOL AreInfoIDsEquivalent(long nInfoID1, long nInfoID2);

void GetEmrItemCategories(IN long nEmrInfoID, OUT CArray<long,long> &arCategoryIDs);

BOOL ValidateEMRTimestamp(long nEMRID);
BOOL ValidateEMNTimestamp(long nEMNID);

// (a.walling 2008-01-29 15:20) - PLID 14982 - Added variable for safety check
void PostEmrItemEntryDlgItemSaved(CEMNDetail* pSrcDetail, BOOL bFromMessage);

// (c.haag 2006-07-03 16:53) - PLID 19977 - This function should be called when patient warnings
// should appear
// (a.walling 2009-06-05 12:59) - PLID 34496 - Include parent window
void PromptPatientEMRProblemWarning(long nPatientID, CWnd* pParent);

#define ITEM_ADD			0
//TES 2/22/2006 - These are not referenced anywhere any more, I'm commenting them out to avoid confusion.
/*#define ITEM_DELETE			1
#define ITEM_RENAME			2
#define ITEM_REORDER		3*/

// (z.manning 2009-03-05 09:05) - PLID 33338 - Moved the SkippedTopic struct to EMRTopic.h

struct EMRInfoChangedAction {

	long nOldActionID;
	long nNewActionID;
};

// (c.haag 2008-08-18 10:14) - PLID 30724
struct EMRInfoChangedProblemAction
{
	long nOldID; // EmrProblemActionsT.ID
	long nNewID;
};

// (c.haag 2011-03-15) - PLID 42821 - Changed custom buttons
struct EMRInfoChangedCommonList
{
	long nOldID; // EmrInfoCommonListT.ID
	long nNewID;
};

struct EMRInfoChangedCommonListItem
{
	long nOldID; // EmrInfoCommonListItemsT.ID
	long nNewID;
	long nOldListID; // EmrInfoCommonListItemsT.ListID
	long nNewListID;
	long nOldDataID; // EmrInfoCommonListItemsT.EmrDataID
	long nNewDataID; // EmrInfoCommonListItemsT.EmrDataID
};

//These EMRInfoChanged... structs are used when EmrItemEntryDlg migrates a locked item to a new item,
//and then an open EMN may use this information to update unsaved details
struct EMRInfoChangedDropdown {

	long nOldDropdownID;
	long nNewDropdownID;
	// (z.manning 2011-04-26 12:24) - PLID 37604 - Added member to store the old data value;
	CString strOldData;

	// (z.manning 2009-02-12 14:20) - PLID 33058 - Dropdown items can now have actions
	CArray<EMRInfoChangedAction*,EMRInfoChangedAction*> aryChangedActions;
	CArray<EMRInfoChangedProblemAction,EMRInfoChangedProblemAction&> aryChangedProblemActions;
};

struct EMRInfoChangedData {

	long nOldDataID;
	long nNewDataID;
	// (z.manning 2011-04-26 14:25) - PLID 37604 - Added members to store the old and new list types
	long nOldListType;
	long nNewListType;

	CArray<EMRInfoChangedAction*,EMRInfoChangedAction*> aryChangedActionIDs;
	CArray<EMRInfoChangedDropdown*,EMRInfoChangedDropdown*> aryChangedDropdownIDs;
	// (c.haag 2008-08-18 10:14) - PLID 30724
	CArray<EMRInfoChangedProblemAction,EMRInfoChangedProblemAction&> aryChangedPAIDs;

	// (z.manning 2009-02-12 14:23) - PLID 33058
	EMRInfoChangedDropdown* GetChangedDropdownByNewID(const long nNewDropdownID)
	{
		for(int i = 0; i < aryChangedDropdownIDs.GetSize(); i++) {
			EMRInfoChangedDropdown *pcdd = aryChangedDropdownIDs.GetAt(i);
			if(pcdd->nNewDropdownID == nNewDropdownID) {
				return pcdd;
			}
		}
		return NULL;
	}
};

//DRT 1/24/2008 - PLID 28602 - For changes to hotspots
struct EMRInfoChangedHotSpot {
	long nOldHotSpotID;
	long nNewHotSpotID;
	CArray<EMRInfoChangedAction*,EMRInfoChangedAction*> aryChangedActionIDs;
	// (c.haag 2008-08-18 10:14) - PLID 30724
	CArray<EMRInfoChangedProblemAction,EMRInfoChangedProblemAction&> aryChangedPAIDs;
};

struct EMRInfoChangedIDMap {

	CArray<EMRInfoChangedData*,EMRInfoChangedData*> aryChangedDataIDs;
	CArray<EMRInfoChangedAction*,EMRInfoChangedAction*> aryChangedActionIDs;
	CArray<EMRInfoChangedHotSpot*, EMRInfoChangedHotSpot*> aryChangedHotSpotIDs;
	// (c.haag 2008-08-18 10:14) - PLID 30724
	CArray<EMRInfoChangedProblemAction,EMRInfoChangedProblemAction&> aryChangedInfoPAIDs;
	// (c.haag 2011-03-15) - PLID 42821 - Changed custom buttons
	CArray<EMRInfoChangedCommonList,EMRInfoChangedCommonList&> aryChangedCommonListIDs;
	CArray<EMRInfoChangedCommonListItem,EMRInfoChangedCommonListItem&> aryChangedCommonListDetailIDs;
};

// (z.manning 2009-02-10 14:53) - PLID 33026 - Moved to EmrUtils.h from EmrTableDropdownEditorDlg.h
// (j.jones 2012-11-27 09:54) - PLID 53144 - moved the functions to the .cpp, from the .h
class CEmrTableDropDownItemArray;


// (z.manning 2011-10-12 11:52) - PLID 45728 - Added a sturct for the stamp info for a dropdown item
// (j.jones 2012-11-26 15:18) - PLID 53144 - removed IsDefault, now this struct is used for both
// dropdown stamp filters and dropdown stamp defaults
struct DropdownStampInfo
{
	long nStampID = -1;

	DropdownStampInfo(long nStampID = -1)
		: nStampID(nStampID)
	{}

	// (a.walling 2014-06-30 10:21) - PLID 62497 - Overload operator< and operator==
	friend bool operator<(const DropdownStampInfo& l, const DropdownStampInfo& r)
	{
		return l.nStampID < r.nStampID;
	}

	friend bool operator==(const DropdownStampInfo& l, const DropdownStampInfo& r)
	{
		return l.nStampID == r.nStampID;
	}
};

class CEmrTableDropDownItem
{
	friend class CEmrTableDropDownItemArray;
public:
	CEmrTableDropDownItem()
	{}

	CEmrTableDropDownItem(long nID)
		: nID_(nID)
	{}

	// (j.jones 2012-11-26 15:24) - PLID 53144 - added bStampDefaultsChanged
	CEmrTableDropDownItem(const CEmrTableDropDownItem &f);

	// (z.manning 2009-02-12 10:42) - PLID 33029 - The dropdown ID increment is now a required paramater.
	// I alos added an action ID increment since dropdown items can now have actions.
	CString GenerateXml(long nIDIncrement, IN OUT long &nActionIDIncrement) const;

	// (z.manning 2011-09-28 11:57) - PLID 45729 - Function to return the list of text stamps associated with this dropdown item
	// (j.jones 2012-11-26 17:31) - PLID 53144 - renamed to more accurately represent that this is the stamp filter only
	CString GetStampFilterText() const;

	// (j.jones 2012-11-27 11:08) - PLID 53144 - returns the list of stamps that use this dropdown by default
	CString GetStampDefaultsText() const;

	// (j.jones 2012-11-27 11:08) - PLID 53144 - used for the hyperlink in the filter/defaults setup,
	// returns the list of stamps in a filter and/or defaults, and < Choose Stamps >, < Choose Defaults > when empty
	CString GetStampFilterHyperlinkText() const;

protected:
	long nID_ = -1; // internal ID, should only be changed by CEmrTableDropDownItemArray::UpdateElementID
	
	void UpdateID(long nNewID) // should only be changed by CEmrTableDropDownItemArray::UpdateElementID after added to that array
	{
		nID_ = nNewID;
	}

public:
	__declspec(property(get = GetID)) long nID;
	long GetID() const
	{
		return nID_;
	}

	CString strData;
	long nSortOrder = -1;
	BOOL bInactive = FALSE;
	// (z.manning 2009-02-10 12:42) - PLID 33026 - Table dropdown items can now have actions.
	// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
	MFCArray<EmrAction> aryActions;
	//TES 3/15/2011 - PLID 42757 - Table dropdown items can be linked to Glasses Order records, we store the name for auditing
	long nGlassesOrderDataID = -1;
	CString strGlassesOrderDataName;

	// (j.gruber 2013-09-30 11:31) - PLID 58675 - add code array
	CEMRCodeArray aryCodes;
	BOOL bCodesChanged = FALSE;

	// (j.gruber 2013-09-30 11:35) - PLID 58475 - added GroupID
	long nDropDownGroupID = -1; 
	
	// (z.manning 2011-09-28 11:34) - PLID 45729
	// (j.jones 2012-11-26 15:18) - PLID 53144 - broke this up into two arrays, one for dropdown stamp filters
	// and one for dropdown stamp defaults
	// (a.walling 2014-06-30 10:21) - PLID 62497 - Using vectors
	std::vector<DropdownStampInfo> aryStampFilter;
	std::vector<DropdownStampInfo> aryStampDefaults;
	BOOL bStampFilterChanged = FALSE;
	BOOL bStampDefaultsChanged = FALSE;

	// (j.gruber 2014-07-22 12:48) - PLID 62627 - keyword variables
	BOOL bUseKeyword = FALSE;
	CString strKeywordOverride = "";

};

// CEmrTableDropDownItemArray is an array of pointers to CEmrTableDropDownItem objects; automatically frees the objects on destruction
// (z.manning 2009-02-10 14:53) - PLID 33026 - Moved to EmrUtils.h from EmrTableDropdownEditorDlg.h
// (a.walling 2014-06-30 10:21) - PLID 62497 - Using internal vector; no longer deriving from CArray
class CEmrTableDropDownItemArray
{
public:
	using Collection = std::vector < std::unique_ptr<CEmrTableDropDownItem> > ;

	CString GenerateXml() const;

	// (z.manning 2009-02-11 10:40) - PLID 33029
	CEmrTableDropDownItem* FindDropdownElement(long nSearchForID) const
	{
		auto it = m_idMap.find(nSearchForID);
		if (it == m_idMap.end()) {
			return nullptr;
		}

		return it->second;
	}

	void Add(std::unique_ptr<CEmrTableDropDownItem>&& newElement)
	{
		if (newElement->nID != -1) {
			auto ret = m_idMap.emplace(newElement->nID, newElement.get());
			ASSERT(ret.second);
		}
		m_data.push_back(std::move(newElement));
	}

	void Add(CEmrTableDropDownItem* newElement)
	{
		Add(std::unique_ptr<CEmrTableDropDownItem>(newElement));
	}

	void AppendCopy(const CEmrTableDropDownItemArray& r)
	{
		for (const auto& val : r.m_data) {
			Add(new CEmrTableDropDownItem(*val));
		}
	}

	// (a.walling 2014-06-30 10:21) - PLID 62497 - Updates to the ID requires an update to the map
	void UpdateElementID(CEmrTableDropDownItem* pddi, long nNewDropdownID)
	{
		m_idMap.erase(pddi->nID);
		pddi->UpdateID(nNewDropdownID);
		if (pddi->nID != -1) {
			auto ret = m_idMap.emplace(pddi->nID, pddi);
			ASSERT(ret.second);
		}
	}
	
	void clear()
	{
		m_idMap.clear();
		m_data.clear();
		//m_pSortOrderMap.reset();
	}

	bool empty() const
	{
		return m_data.empty();
	}

	const Collection& GetData() const
	{
		return m_data;
	}

	Collection& GetData()
	{
		return m_data;
	}

	void erase(Collection::const_iterator it)
	{
		m_idMap.erase(it->get()->nID);
		m_data.erase(it);
	}

	// (a.walling 2014-06-30 10:21) - PLID 62497 - Iterators
	Collection::iterator begin()
	{
		return m_data.begin();
	}

	Collection::const_iterator begin() const
	{
		return m_data.begin();
	}

	Collection::iterator end()
	{
		return m_data.end();
	}

	Collection::const_iterator end() const
	{
		return m_data.end();
	}
	
protected:

	// (a.walling 2014-06-30 10:21) - PLID 62497 - vector of unique_ptr handles memory deallocation
	std::vector<std::unique_ptr<CEmrTableDropDownItem>> m_data;

	mutable std::unordered_map<long, CEmrTableDropDownItem*> m_idMap;
};

///////////////////////////////////////////////////////////////////////////////////////

// (z.manning 2011-04-05 10:38) - PLID 42337 - Moved the LIST_TYPE_ #define's to SharedEmrUtils.h

// (z.manning 2010-02-11 16:47) - PLID 37320 - We now have list sub-types as well
// EmrDataT.ListSubType
enum EEmrDataElementListSubType
{
	lstDefault = 0,
	lstSmartStampType = 1,
	lstSmartStampLocation = 2,
	lstSmartStampQuantity = 3,
	lstSmartStampDescription = 4,
	lstSmartStampAutoNumber = 5, // (z.manning 2010-08-11 10:41) - PLID 40074
	lstCurrentMedicationSig = 6, // (j.jones 2011-05-03 16:02) - PLID 43527
	lstSmartStampInitialType = 7,	//TES 3/8/2012 - PLID 48728
};
// (z.manning 2010-02-12 11:23) - PLID 37320 - Returns true if the list sub type referes to a smart stamp table column
BOOL IsSmartStampListSubType(const BYTE nSubType);

// (j.jones 2011-05-03 16:07) - PLID 43527 - Returns true if the list sub type referes to a built-in current medication table column
BOOL IsCurrentMedicationListSubType(const BYTE nSubType);

// (z.manning 2010-08-11 12:47) - PLID 40074 - Options for the auto-number feature for the built-in smart stamp column
// SAVED TO DATA
enum EEmrTableAutoNumberType
{
	etantInvalid = -1,
	etantPerRow = 1,
	etantPerStamp = 2,
};

// (z.manning 2010-08-11 14:34) - PLID 40074
CString GetEmrAutoNumberTypeDescription(EEmrTableAutoNumberType eType);

//DRT 1/12/2007 - PLID 24178 - Prompts the user to link the given charge with
//	all diagnosis codes on the given EMN.
BOOL PromptToLinkDiagCodesToCharge(EMNCharge *pCharge, CEMN* pEMN, BOOL bIsSpawning);

//DRT 1/15/2007 - PLID 24179 - Prompt the user to link the given diagnosis code
//	with all charges spawned so far.
BOOL PromptToLinkChargesToDiagCode(EMNDiagCode *pDiag, CEMN* pEMN, BOOL bIsSpawning);

//TES 03/25/2009 - PLID 33262 - Prompts the user to link the given prescription with
//	all diagnosis codes on the given EMN.
BOOL PromptToLinkDiagCodesToPrescription(EMNMedication *pMed, CEMN* pEMN, BOOL bIsSpawning);

//TES 03/25/2009 - PLID 33262 - Prompt the user to link the given diagnosis code
//	with all prescriptions spawned so far.
BOOL PromptToLinkPrescriptionsToDiagCode(EMNDiagCode *pDiag, CEMN* pEMN, BOOL bIsSpawning);

// (c.haag 2007-01-25 10:27) - PLID 24396 - Returns the EmrInfoID of the active Current
// Medications info item.
long GetActiveCurrentMedicationsInfoID(OPTIONAL IN ADODB::_Connection *lpCon = NULL);

// (c.haag 2007-04-02 16:08) - PLID 25465 - Returns the EmrInfoID of the active
// Allergies info item
long GetActiveAllergiesInfoID(OPTIONAL IN ADODB::_Connection *lpCon = NULL);

// (j.jones 2010-06-21 11:30) - PLID 37981 - return the InfoID of the active generic table item
long GetActiveGenericTableInfoID(OPTIONAL IN ADODB::_Connection *lpCon = NULL);

// (j.jones 2009-09-18 11:08) - PLID 35367 - Get the date of the latest EMN for the given patient
COleDateTime GetLatestEMNDateForPatient(long nPatientID);

// (c.haag 2007-01-29 09:15) - PLID 24396 - This function populates two arrays
// with the data (row and column) values of the active Current Medications table
// info item
void GetCurrentMedicationTableElements(CArray<long, long>& anX, CArray<long, long>& anY,
									   CArray<long, long>& anYListType, CArray<long, long>& anYSortOrder,
									   long nCurMedEmrInfoID, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

// (c.haag 2007-04-05 11:39) - PLID 25516 - This function populates two arrays
// with the data (row and column) values of the active Allergies table
// info item
void GetAllergiesTableElements(CArray<long, long>& anX, CArray<long, long>& anY,
									   CArray<long, long>& anYListType, CArray<long, long>& anYSortOrder,
									   long nAllergiesEmrInfoID, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

// (c.haag 2007-01-23 15:04) - PLID 24396 - Generates a string that updates the
// CurrentPtMeds table with the latest Current Medications from an EMN
// (c.haag 2007-10-22 08:57) - PLID 27827 - Added nAuditTransactionID
// (c.haag 2011-01-25) - PLID 42222 - We now take in a map that maps current medication data ID's to NewCrop GUID's.
// (j.jones 2011-05-04 10:26) - PLID 43527 - added Sig maps, which maps a DataID
// to the Sig entered in the table or in the medications tab
// (j.jones 2012-10-17 13:44) - PLID 51713 - Added bCurHasNoMedsStatus, the current value of PatientsT.HasNoMeds.
// This function will clear that status if it adds a current medication.
CString GenerateCurrentMedsPropagationSaveString(CArray<long,long>& anEMNCurMedDataIDs,
												 CMap<long, long, CString, LPCTSTR> &mapEMNCurMedDataIDsToSig,
												 CArray<long,long>& anPtCurMedDataIDs,
												 CMap<long, long, CString, LPCTSTR> &mapPtCurMedDataIDsToSig,
												 long nPatientID,
												 long& nAuditTransactionID,
												 const CMap<long,long,CString,LPCTSTR>& mapNewEMRDataIDToNewCropGUID,
												 BOOL &bCurHasNoMedsStatus);

// (c.haag 2007-04-06 09:21) - PLID 25525 - When an EMR with an Allergies detail is saved,
// we need to update the PatientAllergyT table so that it is in perfect synchronization with the
// detail. By design, we onnly need any one pSourceDetail to do this because all of the Allergies
// in memory should always be in sync for an EMN
// (c.haag 2007-10-22 09:20) - PLID 27822 - Added nAuditTransactionID
// (j.jones 2012-10-17 09:32) - PLID 53179 - Added bCurHasNoAllergiesStatus, the current value of PatientsT.HasNoAllergies.
// This function will clear that status if it adds an allergy.
CString GenerateAllergyPropagationSaveString(CArray<long,long> &anEMNAllergyDataIDs, CArray<long,long> &anPtAllergyDataIDs,
											 long nPatientID, long &nAuditTransactionID, BOOL &bCurHasNoAllergiesStatus,
											 OUT CDWordArray &aryAllergyIngredToImport,
											 OUT CDWordArray &aryAllergyIngredToDelete);

void ImportAllergyIngredientsFromFDB(IN const CDWordArray &aryIngredToImport); // (j.luckoski 2012-11-20 10:35) - PLID 53825

void DeleteAllergyIngredientsFromFDB(IN const CDWordArray &aryIngredToDelete, long nPatientID); // (j.luckoski 2012-11-20 10:36) - PLID 53825

// (c.haag 2007-01-30 17:30) - PLID 24422 - Generates a new Current Medications info item.
// This should be called just prior to making changes to the official Current Medications
// info item if the item already exists on EMN's.
long BranchCurrentMedicationsInfoItem();

// (c.haag 2007-04-04 16:40) - PLID 25498 - Generates a new Allergies info item.
// This should be called just prior to making changes to the official Allergies info item
// if the item already exists on EMN's.
long BranchAllergiesInfoItem();

// (j.jones 2010-06-21 15:20) - PLID 37981 - branch the generic table item
long BranchGenericTableItem();

// (c.haag 2007-01-30 16:47) - PLID 24422 - Returns true if it's necessary to copy
// an existing EMR info item if it is to be edited
BOOL IsEMRInfoItemInUse(long nEmrInfoID);

// (a.walling 2013-03-27 09:47) - PLID 55898 - Update EmrInfoT.ModifiedDate (and vicariously, the .Revision rowversion)
void TouchEMRInfoItem(long nEmrInfoID); 

// (c.haag 2007-02-07 13:29) - PLID 24420 - Go through every patient EMN and warn the user
// if there are differences between the patient medications list and the official medications
// detail of any EMN. The "official medications" detail of an EMN refers to any Current Medications
// detail that corresponds to the latest version of the Current Medications info item. If
// there are multiple, they will all have the same state.
void WarnMedicationDiscrepanciesWithEMR(long nPatientID);

// (c.haag 2007-02-07 13:29) - PLID 25524 - Go through every patient EMN and warn the user
// if there are differences between the patient allergy list and the allergy
// detail of any EMN. The "official allergies" detail of an EMN refers to any Allergies
// detail that corresponds to the latest version of the Allergies info item. If
// there are multiple, they will all have the same state.
void WarnAllergyDiscrepanciesWithEMR(long nPatientID);

// (c.haag 2007-02-07 13:33) - PLID 24423 - Warns the user if EmrDataT and DrugList have referential
// integrity failures. This is done in the form of an exception so that the user knows it's a critical
// problem, and that we should be contacted right away
void WarnEmrDataDiscrepanciesWithDrugList();

// (c.haag 2007-04-03 09:37) - PLID 25468 - Warn the user if EmrDataT and AllergyT have referential
// integrity failures. This is done in the form of an exception so that the user knows it's a critical
// problem, and that we should be contacted right away
void WarnEmrDataDiscrepanciesWithAllergyT();

//TES 2/13/2007 - PLID 23401 - Determines whether the specified EMN's providers are all licensed.  If you already
// have the list of providers, you may pass it in here, otherwise the function will load from data.  The function will
// interact with the user, giving them the chance to make one of the providers licensed, and if it returns FALSE, it will
// have notified the user that the EMN cannot be locked, with an explanation.  If it returns TRUE, then the EMN has at least
// one associated provider, and all associated providers are licensed.
// strEmnDescription can be left blank, in a case (such as the <More Info> topic), where it is clear what EMN is being 
// dealt with, otherwise it will be used to add clarification to any messages.
// (z.manning 2011-11-01 09:20) - PLID 44594 - Added bSilent parameter
BOOL AreEmnProvidersLicensed(IN long nEmnID, const CString &strEmnDescription = "", OPTIONAL IN CArray<long,long> *paryProviders = NULL, BOOL bSilent = FALSE);

// (a.walling 2007-04-25 13:57) - PLID 25549 - Moved from EMN.cpp and added definition.
CRect GetDetailRectangle(CEMNDetail* pDetail);

// (a.walling 2013-10-01 11:45) - PLID 58829 - EMR object sorts now using a functor predicate


// (a.walling 2007-04-25 10:54) - PLID 25454 - Compare the topics based on order index
struct CompareTopicSortOrder
	: public std::binary_function<CEMRTopic*, CEMRTopic*, bool>
{
	bool operator()(CEMRTopic* pTopicL, CEMRTopic* pTopicR) const;
};

struct CompareRect
	: public std::binary_function<const RECT&, const RECT&, bool>
{
	bool operator()(const RECT& l, const RECT& r) const;
};

// (a.walling 2007-04-25 10:54) - PLID 25454 - Compare details based on top, then left
struct CompareDetailSortOrder
	: public std::binary_function<CEMNDetail*, CEMNDetail*, bool>
{
	bool operator()(CEMNDetail* pDetailL, CEMNDetail* pDetailR) const;
};

// (c.haag 2008-12-04 15:27) - PLID 31693 - This works like CompareDetailSortOrder but for LinkedDetailStruct objects
struct CompareLinkedDetailStructSortOrder
	: public std::binary_function<LinkedDetailStruct&, LinkedDetailStruct&, bool>
{
	bool operator()(LinkedDetailStruct& linkedDetailL, LinkedDetailStruct& linkedDetailR) const;
};

struct CompareDiagCodeOrderIndex
	: public std::binary_function<EMNDiagCode*, EMNDiagCode*, bool>
{
	bool operator()(EMNDiagCode* pDiagL, EMNDiagCode* pDiagR) const;
};

void SortTopicArray(CArray<CEMRTopic*, CEMRTopic*> &arTopics);
void SortDetailArray(CArray<CEMNDetail*, CEMNDetail*> &arDetails);
// (c.haag 2008-12-04 15:26) - PLID 31693 - Overload of SortDetailArray for LinkedDetailStruct objects
void SortDetailArray(CArray<struct LinkedDetailStruct,struct LinkedDetailStruct&> &arDetails);

// (a.walling 2007-09-26 11:43) - PLID 25548 - Helper functions to sort the diag code array
void SortDiagCodeArray(CArray<EMNDiagCode*, EMNDiagCode*> &arDiagCodes);

// (a.walling 2009-11-18 12:09) - PLID 36365 - Deprecated recordset-based narrtive functions
/*
// (c.haag 2007-08-16 09:03) - PLID 27087 - This function creates a connectionless
// recordset containing narrative fields.
ADODB::_RecordsetPtr CreateNarrativeFieldSet();

// (c.haag 2007-08-16 09:03) - PLID 27087 - This functon overwrites the
// contents of prsDest with prsSrc.
void CopyNarrativeFieldSet(ADODB::_RecordsetPtr& prsDest, ADODB::_RecordsetPtr& prsSrc);

// (c.haag 2007-08-16 09:03) - PLID 27087 - This function clears a
// narrative field set.
void ClearNarrativeFieldSet(ADODB::_RecordsetPtr& prs);

// (c.haag 2007-08-16 09:03) - PLID 27087 - This function gets the "Value"
// field of a narrative record. If the value has not been calculated already,
// we will calculate it on the fly by using CEMN functions on "Pending" values
CString GetNarrativeValueField(ADODB::_RecordsetPtr& prs);

// (c.haag 2007-08-16 09:03) - PLID 27087 - This function gets the "SentenceForm"
// field of a narrative record. If the value has not been calculated already,
// we will calculate it on the fly by using CEMN functions on "Pending" values
CString GetNarrativeSentenceFormField(ADODB::_RecordsetPtr& prs);

// (c.haag 2007-08-16 09:03) - PLID 27087 - This function returns NULL if a desired
// narrative merge field was not found, or it will return prs if it was
// found. If found, prs will be at the desired record.
ADODB::_RecordsetPtr SeekToNarrativeField(ADODB::_RecordsetPtr &prs, const CString &strField, const CString &strFlags);

// (j.jones 2008-04-04 12:08) - PLID 29547 - added overloaded version that takes in LW and EMR booleans
ADODB::_RecordsetPtr SeekToNarrativeField(ADODB::_RecordsetPtr &prs, const CString &strField, BOOL bIsLWMergeField, BOOL bIsEMRField);

// (j.jones 2008-04-04 11:45) - PLID 29547 - this function performs the search asked for by SeekToNarrativeField
BOOL TrySeekToNarrativeField(ADODB::_RecordsetPtr &prs, const CString &strField, BOOL bRequireLWField, BOOL bRequireEMRField);

// (c.haag 2007-08-16 09:03) - PLID 27087 - Clears a narrative field record
void ClearNarrativeField(ADODB::_RecordsetPtr& prs);
*/

// (j.jones 2007-05-15 16:44) - PLID 25431 - added function to check both
// Create & Write permissions on EMR, only prompting for a password once
BOOL CheckHasEMRCreateAndWritePermissions();

// (c.haag 2007-08-18 08:54) - PLID 27111 - This function appends a state string with a new element given
// the table row ID, column ID, and formatted data
// (z.manning 2010-02-18 09:09) - PLID 37427 - Added nEmrDetailImageStampID
// (z.manning 2011-03-02 14:47) - PLID 42335 - Added nStampID
void AppendTableStateWithFormattedElement(CString& strState, long nRowID, long nColumnID, const CString& strFormattedData, const long nEmrDetailImageStampID, const long nEmrDetailImageStampPointer, const long nStampID);

// (c.haag 2007-08-18 08:54) - PLID 27111 - This function appends a state string with a new element given
// the table row ID, column ID, and non-formatted data
// (z.manning 2010-02-18 09:09) - PLID 37427 - Added nEmrDetailImageStampID
// (z.manning 2011-03-02 14:47) - PLID 42335 - Added nStampID
inline void AppendTableStateWithUnformattedElement(CString& strState, long nRowID, long nColumnID, const CString& strUnformattedData, const long nEmrDetailImageStampID, const long nEmrDetailImageStampPointer, const long nStampID);

// (a.walling 2007-10-18 17:28) - PLID 27664 - This function used to be floating around EMRTopic.cpp somewhere, so now it's here.
inline BOOL IsTopicInArray(const CArray<CEMRTopic*,CEMRTopic*> &arTopics, CEMRTopic *pTopic)
{
	for(int i = 0; i < arTopics.GetSize(); i++) if(arTopics[i] == pTopic) return TRUE;
	return FALSE;
}

// (z.manning 2011-11-10 10:18) - PLID 46382 - The code to get available narrative fields to Practice from the rich text control
//TES 6/6/2012 - PLID 50855 - Version 3 adds DetailID and DataID
#define NARRATIVE_AVAILABLE_FIELD_VERSION 3

// (z.manning 2011-11-11 11:26) - PLID 37093
CString FormatNexFormsTextForNarrative(const CString &strNexFormsText);

// (j.jones 2008-01-16 09:55) - PLID 18709 - added NarrativeRequestLWMergeFieldData,
// so the OnRequestLWMergeFieldData() functions in CEmrItemAdvNarrativeDlg and
// CEMRItemAdvPopupWnd can share the same code base
// (a.walling 2009-11-18 12:09) - PLID 36365 - Pass in rich text
void NarrativeRequestLWMergeFieldData(CEMNDetail *pDetail, const CString& strRichText);

// (a.walling 2008-01-16 15:36) - PLID 14982 - function moved from EmrItemAdvPopupDlg.cpp
void BuildCurrentTableStateArray(class CEMRTopic* pTopic, long nEMRInfoID, CArray<CEMNDetail*,CEMNDetail*>& apDetails);

//DRT 2/26/2008 - PLID 28603
void BuildCurrentImageArray(CEMRTopic* pTopic, long nEMRInfoID, CArray<CEMNDetail*,CEMNDetail*>& apDetails);

// (a.walling 2011-09-14 15:51) - PLID 45498 - Get count of the stamps
// (a.wilson 2013-03-22 10:14) - PLID 55826 - pass in the detail to filter out the excluded stamps
long GetInkPictureStampCount(CEMNDetail *pDetail);

// (j.jones 2010-02-10 11:12) - PLID 37224 - parse EMR Image Stamps (loaded in MainFrame) into a string to send to NxInkPicture
// (z.manning 2011-10-25 12:53) - PLID 39401 - Added detail param
CString GenerateInkPictureStampInfo(CEMNDetail *pDetail);

// (z.manning, 01/23/2008) - PLID 28690 - Clears out any existing hot spots and re-adds them to
// the ink pic control based on the array stored in pDetail.
void RefreshHotSpots(NXINKPICTURELib::_DNxInkPicturePtr pInkPicture, CEMNDetail *pDetail);

// (a.walling 2008-03-24 10:24) - PLID 28811 - Returns whether the vt is valid for the info type
BOOL DataTypeMatchesState(const EmrInfoType eit, const long vt);

// (r.gonet 2013-04-08 17:48) - PLID 56150 - Returns the number of elements
//  in a safearray variant array. Argument should be a single dimension array containing bytes.
unsigned long GetElementCountFromSafeArrayVariant(const _variant_t &var);

// (z.manning 2008-06-06 09:14) - PLID 30155 - Returns the details value that should be used for
// any table calulations.
//TES 10/12/2012 - PLID 39000 - Added bValueHadUnusedNumbers.  This will be set to TRUE if one or more of the fields that was used for the 
// calculation was truncated, and the text that was truncated included digits.  So for example, if a value was 5'6", then it would be
// TRUE, but if the value was 66 in. it would be FALSE, because while the "in." is getting truncated, it does not include any numbers.
double GetValueForCalculation(CEMNDetail *pDetail, long nRow, short nCol, OUT BOOL &bValueWasBlank, OUT BOOL &bValueHadUnusedNumbers);

// (c.haag 2008-06-17 12:15) - PLID 17842 - This function takes an array of details, looks for
// any table details that have missing linked detail elements, and tries to fill them in. The
// motivation for this goes back to "Remember this detail (for the patient)". Table element
// states can remember details by their EmrDetailsT.ID. Unfortunately, that is useless to other
// EMN's that don't contain the same ID. So, we do the next best thing: Try to find linked details
// by the Emr Info name of that detail rather than the ID of the detail. bSearchState is TRUE if
// we want to search the state in addition to the table elements (slower for table details)
void DetectAndTryRepairMissingLinkedDetails(CArray<CEMNDetail*,CEMNDetail*>& apDetails, BOOL bSearchState);

// (c.haag 2008-07-02 15:44) - PLID 30221 - Fills an Emr Action object with values from EMRActionsTodoAssignToT
void FillEmrActionWithTodoAssignTos(ADODB::_ConnectionPtr& pCon, IN OUT EmrAction& ea);

// (c.haag 2008-07-17 15:17) - PLID 30550 - This function returns a query that must be
// run upon locking an EMN to properly lock todo alarms
CString GetEMRLockTodoAlarmSql(long nEMNID);

// (c.haag 2008-07-17 17:46) - PLID 30775 - Compares the data content of two EMR actions
// and returns TRUE if they are the same, or FALSE if they differ. This function calls all
// the "drill-down" functions listed below.
BOOL DoesEmrActionContentMatch(const EmrAction& a, const EmrAction& b);

// (c.haag 2008-07-17 17:16) - PLID 30775 - Compares the problem-related content of two
// EMR actions and returns TRUE if they are the same, or FALSE if they differ
BOOL DoesEmrActionProblemContentMatch(const EmrAction& a, const EmrAction& b);

// (c.haag 2008-07-18 09:21) - PLID 30775 - Compares two emr problem actions, and
// returns TRUE if they match, or FALSE if they don't.
BOOL DoesEmrActionProblemContentMatch(const EmrProblemAction& a, const EmrProblemAction& b);

// (c.haag 2008-07-17 17:16) - PLID 30775 - Compares the todo-related content of two
// EMR actions and returns TRUE if they are the same, or FALSE if they differ
extern int CompareTodoAssignToAryElements(const void *pa, const void *pb);
BOOL DoesEmrActionTodoContentMatch(const EmrAction& a, const EmrAction& b);

// (c.haag 2008-07-18 09:01) - PLID 30775 - Compares todo assign to values between
// two EMR actions and returns TRUE if they are the same, or FALSE if they differ
BOOL DoEmrActionTodoAssigneesMatch(const EmrAction& a, const EmrAction& b);

//(s.dhole 7/18/2014 1:38 PM ) - PLID 62724
// Diagnosis code actions and returns TRUE if they are the same, or FALSE if they differ
BOOL DoesEmrActionDiagContentMatch(const EmrAction& a, const EmrAction& b);

// (c.haag 2008-07-17 17:54) - PLID 30775 - Compares the charge-related content of two
// EMR actions and returns TRUE if they are the same, or FALSE if they differ
BOOL DoesEmrActionChargeContentMatch(const EmrAction& a, const EmrAction& b);

// (c.haag 2008-07-17 17:54) - PLID 30775 - Compares the basic content of two
// EMR actions and returns TRUE if they are the same, or FALSE if they differ.
// Exceptions include: nID, nSourceID, nDestID, bDeleted
BOOL DoesEmrActionBasicContentMatch(const EmrAction& a, const EmrAction& b);

// (c.haag 2008-07-18 09:08) - PLID 30724 - Given an array of Emr problem actions,
// determine which ones were created, which were deleted and which were modifed.
void GetEmrActionProblemDiffs(const EmrAction& eaOld, const EmrAction& eaNew,
							  OUT CProblemActionAry& aCreated, OUT CProblemActionAry& aDeleted,
							  OUT CProblemActionAry& aModified);

// (c.haag 2008-07-18 15:18) - PLID 30784 - This function returns the SQL text to use in a
// massive query for loading one or more EMNs
// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
// (j.jones 2012-07-31 15:17) - PLID 51750 - added sqlEMNSortOrder
// (j.jones 2013-07-02 08:56) - PLID 57271 - removed sqlEMNSortOrder, it always sorts by EMR ID now
CSqlFragment GetEMNProblemSql(CSqlFragment sqlLoadIDs);

// (j.jones 2014-02-24 16:49) - PLID 61010 - a number of places need the diagnosis code number,
// and rather than check one recordset per code, this function will get both
void GetICD9And10Codes(long nICD9CodeID, long nICD10CodeID, OUT CString &strICD9Code, OUT CString &strICD10Code);

// (j.jones 2014-02-24 16:49) - PLID 61019 - unifies auditing for ICD-9/10 codes on EMR problems
void GetProblemICD910AuditDescriptions(const CString &strOldICD9Code, const CString &strOldICD10Code, const CString &strNewICD9Code, const CString &strNewICD10Code,
									   OUT CString &strOldDiagCodeAudit, OUT CString &strNewDiagCodeAudit);

// (j.jones 2008-07-22 14:23) - PLID 30792 - made saving (and deleting) code modular
// (z.manning 2009-05-21 15:48) - PLID 34297 - Removed the object ID parameter
// (c.haag 2009-05-28 13:08) - PLID 34277 - We now have a notification window parameter
// (z.manning 2016-04-12 13:59) - NX-100140 - Added problem IDs to ignore param
void SaveProblem(Nx::Quantum::Batch &strSqlBatch, long &nAuditTransactionID, CEmrProblem *pProblem, IN CMapPtrToPtr& mapSavedObjects
	, long nPatientID, CString strPatientName, CWnd* pWndNotify, std::set<long> setProblemIDsToIgnore);
// (z.manning 2009-05-21 15:51) - PLID 34297 - Save a problem links
void SaveProblemLinkArray(Nx::Quantum::Batch &strSqlBatch, CArray<CEmrProblemLink*, CEmrProblemLink*> &aryProblemLinks, const CString strObjectID, IN CMapPtrToPtr &mapSavedObjects, long &nAuditTransactionID, const long nPatientID, const CString strPatientName);
void SaveProblemLink(Nx::Quantum::Batch &strSqlBatch, CEmrProblemLink *pProblemLink, const CString strObjectID, IN CMapPtrToPtr &mapSavedObjects, long &nAuditTransactionID, const long nPatientID, const CString strPatientName);

// (j.jones 2008-08-07 11:35) - PLID 30773 - this function will update the EMR appropriately
// when a problem memory object has been changed
// (c.haag 2009-05-26 15:51) - PLID 34298 - We now pass in problem links. We no longer need the EMN lock flag;
// we calculate it in the function now
// (j.jones 2009-06-05 09:16) - PLID 34487 - moved from EMRProblemListDlg
void UpdateEMRInterface(CWnd *pMessageWnd, CEmrProblemLink *pLink);

// (c.haag 2008-07-23 13:28) - PLID 30820 - This function will detect if there are any problems which exist in data and were
// not manually deleted, yet they belong to an EMR object that is about to be deleted. The user may not be aware that these
// problems are about to be deleted by virtue of their owner EMR object being deleted. So, a warning will appear if that is the
// case. The user must confirm that they want the problems deleted.
BOOL CheckForDeletedEmrProblems(EmrSaveObjectType esotSaveType, long nObjectPtr, BOOL bIncludeThisObject);

// (c.haag 2009-05-22 15:34) - PLID 34298 - Given an EMR problem and the EMR, this
// function will find all EMR problem links for the same problem in memory in the 
// same CEMR object. If the problem has a valid ID, it will also search data for 
// all problem links not already included in memory, and add those to both arrays.
// (j.jones 2009-05-29 12:22) - PLID 34301 - moved to EmrUtils
//TES 11/25/2009 - PLID 36191 - Changed from a CLabEntryDlg to a CLabRequisitionDlg
void PopulateProblemLinkArrays(CEmrProblem* pProblem, CEMR* pEMR,
	CArray<CEmrProblemLink*,CEmrProblemLink*>& apAllLinks,
	CArray<CEmrProblemLink*,CEmrProblemLink*>& apLinksFromData,
	NXDATALIST2Lib::_DNxDataListPtr pDataList = NULL,
	short nListProblemLinkPtrCol = -1,
	short nListProblemLinkIDCol = -1,
	short nListDetailNameCol = -1,
	short nListDetailValueCol = -1,
	CLabRequisitionDlg *pdlgLabRequisition = NULL);

// (c.haag 2009-05-28 12:23) - PLID 34298 - This function will get the detail name and values given a problem link object
// (j.jones 2009-05-29 12:22) - PLID 34301 - moved to EmrUtils
void GetDetailNameAndValue(CEmrProblemLink* pLink, CString& strDetailName, CString& strDetailValue, CString& strTopicName, CString& strEMNName, CString& strEMRName);

// (c.haag 2008-08-06 11:03) - PLID 30820 - Ensures a problem exists in a given array
void EnsureProblemInArray(CArray<CEmrProblem*,CEmrProblem*>& apProblems, CEmrProblem* pProblem);

// (j.jones 2009-05-29 09:53) - PLID 34301 - added EnsureProblemLinkInArray, same logic as EnsureProblemInArray
void EnsureProblemLinkInArray(CArray<CEmrProblemLink*,CEmrProblemLink*>& apProblemLinks, CEmrProblemLink* pProblemLink);

// (c.haag 2009-06-08 09:49) - PLID 34398 - Like above, but we ensure all the problems in apProblemLinks also exist in apProblems
void EnsureProblemsInArray(const CArray<CEmrProblemLink*,CEmrProblemLink*>& apProblemLinks,
						   CArray<CEmrProblem*,CEmrProblem*>& apProblems);

// (c.haag 2008-07-24 08:41) - PLID 30820 - This is the "from" clause for a datalist that uses the standard EMR problem
// list query. J.Jones is the original author, but I moved it to this function.
CString GetEmrProblemListFromClause();

// (c.haag 2008-07-24 09:40) - PLID 30826 - Returns TRUE if the currently logged in user can delete Emr problems
BOOL CanCurrentUserDeleteEmrProblems();

// (c.haag 2008-08-04 15:25) - PLID 30942 - Fill a problem array with problems that are bound to unspawning actions. The general
// plan is to go through the doomed object array, add all problems that correspond to object pointers contained in the array, and
// also add problems bound to the source object that correspond to the same action.
// (c.haag 2009-05-29 16:16) - PLID 34293 - Converted all references of problems to problem links
void FillUnspawnedProblemLinksArray(IN const CArray<CDoomedEmrObject, CDoomedEmrObject&>& aryAllDoomedObjects, 
								OUT CArray<CEmrProblemLink*,CEmrProblemLink*>& aryOutProblemLinks);

// (c.haag 2008-08-04 17:18) - PLID 30492 - Invoke a dialog that warns the user of the problems that will be deleted as a result of
// the collective unspawning to be done by this class.
// (c.haag 2009-05-29 16:28) - PLID 34398 - We now take in problem links
void WarnOfUnspawningProblemLinks(const CArray<CEmrProblemLink*,CEmrProblemLink*>& aryProblemLinks);

// (a.walling 2008-08-27 13:21) - PLID 30855 - Generic function for warning if concurrency issues may exist for an EMN(s)
// (z.manning 2009-07-01 12:10) - PLID 34765 - Added a parameter to make it optional to exclude the current user
// when checking concurrency issues.
// (j.armen 2013-05-14 12:23) - PLID 56680 - Takes in a CSqlFragment instead of a CString
BOOL WarnIfEMNConcurrencyIssuesExist(CWnd* pWnd, const CSqlFragment& sqlEMNQuery, const CString& strMessage, BOOL bExcludeCurrentUser = TRUE);

// (z.manning 2008-11-14 12:42) - PLID 32035 - Added a struct and function to sort topics in order of
// parent, meaning parent topics will always come before their children.
struct EmrTopicSortInfo
{
	long nTopicID;
	_variant_t varParentTopicID;
};
void SortTopicsByParent(IN OUT CArray<EmrTopicSortInfo,EmrTopicSortInfo&> &aryTopicSortInfo, OUT CArray<long,long> &arynSortedTopicIDs);

// (j.jones 2008-11-25 14:20) - PLID 28508 - UpdatePatientAllergyReviewStatus will update
// the given patient's AllergiesReviewedOn and AllergiesReviewedBy fields, and audit the change.
// The return value is the date that was stamped into data, will be invalid if bReviewedAllergies = FALSE.
COleDateTime UpdatePatientAllergyReviewStatus(long nPatientID, BOOL bReviewedAllergies);

// (j.jones 2012-10-26 09:35) - PLID 53322 - Typically called after an EMR closes and asks to prompt
// to update PatientsT.HasNoAllergies. Should only be called if we already know they have no allergies
// and know the "Has No Allergies" setting is not checked off.
void PromptPatientHasNoAllergies(long nPatientID);

// (j.jones 2012-10-26 16:52) - PLID 53324 - Typically called after an EMR closes and asks to prompt
// to update PatientsT.HasNoMeds. Should only be called if we already know they have no current medications
// and know the "Has No Medications" setting is not checked off.
void PromptPatientHasNoCurrentMedications(long nPatientID);

// (z.manning 2009-02-24 09:10) - PLID 33138 - This function is equivilent to logic in many of
// the CEMN::GetEmrObjectsToRevoke_ functions so I moved here to reduce the duplicated code.
// (j.jones 2013-07-15 10:51) - PLID 57243 - changed the action to a reference
BOOL IsSourceActionInfoOk(SourceActionInfo &saiSourceAction, SourceActionInfo &saiDestObject, const EmrAction &ea, long nDestObjectID);

// (b.savon 2014-07-22 16:52) - PLID 62996 - Handle unspawning of diagnosis tuples
BOOL IsSourceActionInfoOK_Essentials(SourceActionInfo &saiSourceAction, SourceActionInfo &saiDestObject, const EmrAction &ea);
BOOL IsSourceDetailOK(SourceActionInfo &saiSourceAction, SourceActionInfo &saiDestObject);
BOOL IsSourceSmartStampOK(SourceActionInfo &saiSourceAction, SourceActionInfo &saiDestObject, const EmrAction &ea);
BOOL IsSourceDataGroupIDOK(SourceActionInfo &saiSourceAction, SourceActionInfo &saiDestObject, const EmrAction &ea);

// (c.haag 2009-05-13 13:04) - PLID 34249 - This utility function populates anEMRProblemIDs with the ID's of all
// problems that are not linked with a given EMR-related item. Returns FALSE if there are no problems for the patient,
// or TRUE if there are problems for this patient. Even when this returns TRUE, anEMRProblemIDs can still be
// empty if all the problems for the patient are already linked with the item.
BOOL GetEmrProblemsNotLinkedWithItem(long nPatientID, EMRProblemRegardingTypes RegardingType, long nRegardingID, 
									 OUT CArray<long,long>& anEMRProblemIDs);

//TES 9/9/2009 - PLID 35495 - Moved here from EmrTreeWnd.h
// (a.walling 2008-04-29 09:17) - PLID 29815 - Added moreinfo icons
enum EmrTreeIcon {
	etiOpenSavedEmn = 0,
	etiFinishedSavedEmn,
	etiOpenUnsavedEmn,
	etiFinishedUnsavedEmn,
	etiReadOnlyEmn,		// (a.walling 2008-06-03 16:26) - PLID 23138
	etiReadOnlyUnsavedEmn,
	etiLockedEmn,
	etiReadOnlyLockedEmn, // (a.walling 2008-08-22 17:24) - PLID 23138
	etiOpenSavedTopic,
	etiClosedSavedTopic,
	etiOpenUnsavedTopic,
	etiClosedUnsavedTopic,

	// (a.walling 2008-07-21 17:53) - PLID 30790 - New icons for topics with problems
	etiOpenSavedTopicProblems,
	etiClosedSavedTopicProblems,
	etiOpenUnsavedTopicProblems,
	etiClosedUnsavedTopicProblems,
	
	etiOpenSavedTopicClosedProblems,
	etiClosedSavedTopicClosedProblems,
	etiOpenUnsavedTopicClosedProblems,
	etiClosedUnsavedTopicClosedProblems,

	// (a.walling 2008-07-22 17:33) - PLID 30790 - And for EMNs with problems
	etiOpenSavedEmnProblems,
	etiFinishedSavedEmnProblems,
	etiOpenUnsavedEmnProblems,
	etiFinishedUnsavedEmnProblems,
	etiReadOnlyEmnProblems,
	etiReadOnlyUnsavedEmnProblems,
	etiLockedEmnProblems,
	etiReadOnlyLockedEmnProblems, // (a.walling 2008-08-22 17:24) - PLID 2138
	
	etiOpenSavedEmnClosedProblems,
	etiFinishedSavedEmnClosedProblems,
	etiOpenUnsavedEmnClosedProblems,
	etiFinishedUnsavedEmnClosedProblems,
	etiReadOnlyEmnClosedProblems,
	etiReadOnlyUnsavedEmnClosedProblems,
	etiLockedEmnClosedProblems,
	etiReadOnlyLockedEmnClosedProblems, // (a.walling 2008-08-22 17:24) - PLID 2138

	etiOpenSavedMoreInfo,
	etiOpenUnsavedMoreInfo,
	etiClosedSavedMoreInfo,
	etiClosedUnsavedMoreInfo,

	//TES 2/12/2014 - PLID 60748 - Icons for the new Codes topic
	etiOpenSavedCodes,
	etiOpenUnsavedCodes,
	etiClosedSavedCodes,
	etiClosedUnsavedCodes,

	etiTreeIconCount
};
#define EMR_TREE_ICON_COUNT etiTreeIconCount

//TES 9/9/2009 - PLID 35495 - Moved here from CEmrTreeWnd::GetIcon()
// (a.walling 2008-04-29 09:18) - PLID 29815 - Icon size calculated per-emn. Handles stored in an array rather than individual member variables.
//TES 9/16/2009 - PLID 35495 - This now takes a single boolean parameter, for whether we're using the 24x24 or 32x32 icons.
HICON GetEmrTreeIcon(EmrTreeIcon eti, bool bUseSmallIcons);

class CEmrItemAdvNarrativeDlg;
//TES 9/16/2009 - PLID 35529 - Returns a pointer to a shareable, global, CEmrItemAdvNarrativeDlg for background loading.
// If pDetail is not NULL, this function will create the dialog if necessary, and point it to the given detail.
// If pDetail is NULL, this will be treated as a simple accessor.

// (a.walling 2012-02-22 14:53) - PLID 48320 - Global hidden parent window for controls
CWnd& GetHiddenDetailParentWnd();

// (a.walling 2009-12-08 15:22) - PLID 36225 - We now have two global CEmrItemAdvNarrativeDlgs, one for narrative loading, and
// the original one for windowless accessibility via the analysis and etc. Pass in the index to the one you want. The original
// is 0.
enum EGlobalNarrativeIndex {
	egiWindowless = 0,
	egiLoading = 1,
	egiCount = 2,
};
CEmrItemAdvNarrativeDlg* GetGlobalEmrItemAdvNarrativeDlg(CEMNDetail *pDetail, EGlobalNarrativeIndex eGlobalIndex = egiWindowless);
//TES 9/16/2009 - PLID 35529 - Destroys the global CEmrItemAdvNarrativeDlg
// (a.walling 2009-12-08 15:22) - PLID 36225 - Destroys all global CEmrItemAdvNarrativeDlgs
void EnsureNotGlobalEmrItemAdvNarrativeDlg();

//TES 9/2/2011 - PLID 37633 - We need a way that will ensure that the dialog is created, without passing in a detail, because otherwise
// we might end up creating the dialog in a thread, which causes issues.
void EnsureGlobalEmrItemAdvNarrativeDlg(EGlobalNarrativeIndex eGlobalIndex);

//(e.lally 2009-10-26) PLID 32503 - Faxing an EMN preview pane
void FaxEMNPreview(CEMN* pEmn);

// (z.manning 2012-09-11 12:52) - PLID 52543 - Will load the given EMN from data and re-generate its preview
void RegenerateEmnPreviewFromData(const long nEmnID, BOOL bFailIfExists);

// (z.manning 2010-02-16 18:04) - PLID 37230
enum AnatomySide;
CString FormatAnatomicLocation(CString strLocation, CString strQualifier, AnatomySide as);

//TES 2/25/2010 - PLID 37535 - Can the detail be linked on either narratives or tables?
// Note that at the present time, details which can be linked to tables are a subset of those that can be linked to narratives.
BOOL IsDetailLinkable(CEMNDetail *pDetail);
//TES 2/25/2010 - PLID 37535 - Can this detail be linked in tables?
BOOL IsDetailLinkableOnTable(CEMNDetail *pdetail);

//TES 4/15/2010 - PLID 24692 - Functions for working with our linked list of topic positions.
struct TopicPositionEntry;
//TES 4/15/2010 - PLID 24692 - Adds the given entry to the end of the list whose parent is the specified topic (-1 for top-level).  If it is already in the list,
// it will be detached from its current position and moved to the specified location.
//TES 4/19/2010 - PLID 24692 - Pass in the head of the list; the function will return the new head of the list, if for some reason
// this operation results in the head of the list being changed.
TopicPositionEntry* AddTopicPositionEntryAtEnd(TopicPositionEntry *pListHead, TopicPositionEntry *tpe, long nParentTopicID);
//TES 4/15/2010 - PLID 24692 - Adds the given entry to the end of the list whose parent is the specified topic (NULL for top-level).  If it is already in the list,
// it will be detached from its current position and moved to the specified location.
//TES 4/19/2010 - PLID 24692 - Pass in the head of the list; the function will return the new head of the list, if for some reason
// this operation results in the head of the list being changed.
TopicPositionEntry* AddTopicPositionEntryAtEnd(TopicPositionEntry *pListHead, TopicPositionEntry *tpe, TopicPositionEntry *pParent);
//TES 4/15/2010 - PLID 24692 - Inserts the given entry into our linked list before the specified entry.  If it is already in the list,
// it will be detached from its current position and moved to the specified location.
//TES 4/19/2010 - PLID 24692 - Pass in the head of the list; the function will return the new head of the list, if for some reason
// this operation results in the head of the list being changed.
TopicPositionEntry* InsertTopicPositionEntry(TopicPositionEntry *pListHead, TopicPositionEntry *tpe, TopicPositionEntry *tpeInsertBefore);
//TES 5/12/2010 - PLID 24692 - Detaches the given entry.  Pass in the head of the list; the function will return the new head of the list, 
// if this operation results in the head of the list being changed.
TopicPositionEntry* DetachTopicPositionEntry(TopicPositionEntry* pListHead, TopicPositionEntry* tpe);


// (j.jones 2010-06-21 14:49) - PLID 37981 - added ability to generate a state from a TableContent pointer
_variant_t GenerateStateFromGenericTableContent(long nEMRInfoID, DevicePluginUtils::TableContent *pGenericTableContent);

// (j.jones 2010-06-21 14:56) - PLID 37981 - will branch and add more EMRDataT records if needed,
// the EMRInfoID may change if it is branched
void EnsureGenericTableHasEnoughRecords(long &nEMRInfoID, long nRowsNeeded, long nColumnsNeeded);

// (c.haag 2010-07-10 12:07) - PLID 39467 - Returns TRUE if the given EMR info item has table dropdowns
// with duplicate SortOrder values.
BOOL EmrItemHasTableDropdownsWithDuplicateSortOrders(long nEmrInfoID);

// (j.jones 2011-03-09 09:05) - PLID 42283 - given a category ID, count of E/M elements, and a detail,
// track that information in an array
void AddEMElementsToCategoryArray(long nCategoryID, long nElementCount, CEMNDetail *pDetail,
						 CArray<ChecklistTrackedCategoryInfo*, ChecklistTrackedCategoryInfo*> &aryTrackedCategories);

// (z.manning 2011-04-06 12:24) - PLID 43160 - Moved this here from CRoomManagerDlg
// (z.manning 2012-09-10 15:47) - PLID 52543 - Changed to be an array of EmnPreviewPopup
void GetEmnIDsByPatientID(long nPatientID, OUT CArray<EmnPreviewPopup, EmnPreviewPopup&> &arynEMNs);

// (z.manning 2011-04-06 15:17) - PLID 43140
// (j.jones 2012-07-31 15:17) - PLID 51750 - added sqlEMNSortOrder
// (j.jones 2013-07-02 08:56) - PLID 57271 - removed sqlEMNSortOrder, it always sorts by EMR ID now
CSqlFragment GetEmrDetailListOrderSql(CSqlFragment sqlEmnIDInClause);

// (z.manning 2011-05-19 14:36) - PLID 33114 - Gets the IDs of all EMR charts the current user is licensed to view
// Returns true if they can see all charts
BOOL PollEmrChartPermissions(OUT CArray<long,long> &arynPermissionedChartIDs);

// (z.manning 2011-05-19 14:48) - PLID 33114 - Function to return SQL meant to filter on only charts the current
// used is permissioned for. The filters are meant for use in either the where clause or as part of the join.
CSqlFragment GetEmrChartPermissionFilter(BOOL bIncludeAnd = TRUE);
CSqlFragment GetEmrChartPermissionFilter(LPCTSTR strFieldToFilterOn, BOOL bIncludeAnd = TRUE);

// (z.manning 2011-05-19 15:26) - PLID 33114
ADODB::_RecordsetPtr GetEmrChartRecordset();

// (z.manning 2011-06-29 11:31) - PLID 37959
// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
void RemoveActionsBasedOnAnatomicLocation(IN OUT MFCArray<EmrAction> *paryActions, CEMNDetail *pImageDetail, EmrDetailImageStamp *pDetailStamp);
BOOL DoesActionApplyToHotSpot(EmrAction *pea, CEMRHotSpot *pHotSpot);

// (z.manning 2011-07-25 10:46) - PLID 44676
// (a.walling 2012-04-02 08:32) - PLID 46648 - Dialogs must set a parent!
void OpenHotSpotActionEditor(CWnd* pParent, CEMRHotSpot *pHotSpot, const long nInfoID, CString &strSourceItemName, CEMN *pCurrentEmn, CEMRHotSpotArray *parypChangedHotSpots);

// (z.manning 2011-07-11 15:22) - PLID 44469 - For a given coding group range and group quantity, will return what
// the charges should be. Also takes a param for all loaded charges so we can easily keep track of them in memory.
void GetChargesByCodingGroupQuantity(CEmrCodingRange *pCodingRange, const long nGroupQuantity, IN CEMNChargeArray *parypAllLoadedCharges, OUT CEMNChargeArray *parypNewCharges);

// (z.manning 2011-09-02 17:54) - PLID 32123
BOOL GetEmnStatusName(const long nEmnStatusID, OUT CString &strEmnStatus);

// (j.gruber 2012-05-31 10:46) - PLID 49046
BOOL CreateEMNFromTemplate(long nTemplateID, CWnd* pParent);
void GlobalLaunchPICEditorWithNewEMN(long nPicID, long nTemplateID);
void GlobalLaunchPICEditorWithNewEMR(long nPicID, long nTemplateID);

// (j.gruber 2012-05-31 15:18) - PLID 49054
void StartEMRTemplateWithCollection(int nEmrCollectionID, CWnd *pParent);

// (j.jones 2012-11-14 11:22) - PLID 52819 - Given an EMN ID, will return true if the
// current user has open, writeable access to that EMN at this time.
// The two OUT parameters pass the locked status and access status back to the caller
// for the purposes of context-sensitive messaging.
BOOL CanEditEMN(long nEMNID, OUT long &nStatus, OUT BOOL &bHasAccess);

// (z.manning 2012-11-19 09:46) - PLID 52262
BOOL IsSmartStampImageInUseOnEmnTemplate(const long nSmartStampImageInfoMasterID);

// (z.manning 2012-07-03 09:25) - PLID 50958 - Deprecated
/*
// (z.manning 2011-10-31 13:53) - PLID 44594
struct EmnSignInfo
{
	long nEmnID;
	long nPicID;
	CString strPatientName;
	CString strEmnDescription;
	_variant_t varEmnDate;

	CString GetDescription()
	{
		CString strDescription = strPatientName + " - ";
		if(varEmnDate.vt == VT_DATE) {
			strDescription += FormatDateTimeForInterface(VarDateTime(varEmnDate), dtoDate) + " - ";
		}
		strDescription += strEmnDescription;
		return strDescription;
	}
};
void SignEmns(CArray<EmnSignInfo,EmnSignInfo&> *paryEmnsToSign, CWnd *pwndParent);
*/

// (z.manning 2011-11-07 17:24) - PLID 46309
CString GetActualSpawnedItemsSepator(LPCTSTR strSpawnedItemsSeparator);


// (z.manning 2010-08-18 11:26) - PLID 39842 - Moved declaration to header file
class CSortedTableElementArray : public CArray<TableElement*, TableElement*>
{
public:
	void Sort(CEMNDetail* pDetail);

protected:
	static int __cdecl CompareRowsThenColumns(const TableElement** pp1, const TableElement** pp2);

	// (c.haag 2008-10-22 12:35) - PLID 31762 - Goes in order of columns then rows instead of
	// rows then columns
	static int __cdecl CompareColumnsThenRows(const TableElement** pp1, const TableElement** pp2);
};

// (a.walling 2013-06-27 13:15) - PLID 57348 - NxImageLib - More versatile replacement for g_EmrImageCache
// (c.haag 2012-06-11) - PLID 50806 - This dialog prompts the user if they're sure they want to forcefully take
// write access of an EMN from another user
int DoForcedWriteAcquisitionPrompt(CWnd* pParentWnd, const struct CWriteTokenInfo& wtInfo);

// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking - auto-assign appointment or prompt to choose
std::vector<long> FindEmnAppointments(long nPatientID, COleDateTime dtDate);
// (a.walling 2013-02-13 10:47) - PLID 55143 - Emr Appointment linking - UI - Added nExistingID for preselection
boost::optional<long> PromptForEmnAppointment(CWnd* pParent, long nPatientID, COleDateTime dtDate, long nExistingID = -1);

// (a.walling 2013-02-15 11:11) - PLID 54651 - Check for any appointments linked to EMNs
CString GetLinkedEMNDescriptionsFromAppointment(long nApptID);

// (c.haag 2013-03-19) - PLID 55697 - Populates a sorted string array with letter writing fields
void LoadEMRLetterWritingMergeFieldList(CStringSortedArrayNoCase &aryFieldList, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

// (j.gruber 2013-09-18 16:20) - PLID 58676 - moved here from EMRTableEditCalculatedFieldDlg
int CompareInfoDataElementsByVisibleIndex(const void *pDataElementA, const void *pDataElementB);

//TES 7/8/2013 - PLID 57415 - Determines whether the given diagnosis code should trigger a Cancer Case submission
BOOL IsCancerDiagnosis(const CString &strDiagCode);

// (b.savon 2014-02-27 07:25) - PLID 60808 - UPDATE - Write icd10 selections to data upon save of EMN
CString GenerateEMRDiagCodesTWhereCondition(const CString &strDiagCodeID, const CString &strDiagCodeID_ICD10);

// (b.savon 2014-02-27 14:10) - PLID 61065 - UPDATE - Write icd10 selections to data upon save of EMN - Auditing
CString GenerateEMRDiagCodesTAuditValue(const CString &strDiagCode, const CString &strDiagCode_ICD10);
void PopulateICD9AuditValue(CString &strAudit, const CString &strDiagCode);
void PopulateICD10AuditValue(CString &strAudit, const CString &strDiagCode_ICD10);

//TES 5/2/2014 - PLID 61855 - We now pass back information if a cancer case fails to be generated, using this enum and struct
enum CancerCaseFailureType {
	ccftError,
	ccftWarning,
};
struct CancerCaseFailureInfo {
	long nEmnID;
	CString strDisplayText;
	CancerCaseFailureType ccft;
};

//TES 11/14/2013 - PLID 57415 - Creates a Cancer Case submission for the given diagnosis, unless one has been created for this PIC since dtLastModified
//TES 11/21/2013 - PLID 57415 - Moved here from PicContainerDlg.h, added nPicID
// (j.jones 2014-02-27 14:10) - PLID 61069 - this now takes in a diag code ID, in addition to the code and description
//TES 5/1/2014 - PLID 61916 - Moved here from PatientNexEmrDlg, added nPatientID and pMessageParent
//TES 5/1/2015 - PLID 61916 - This now returns the filename (if any) that was created
//TES 5/2/2014 - PLID 61855 - Added an output variable, pFailureInfo, for if the function succeeds (we assume this will be non-null if pMessageParent is null), 
// as well as strEmnDescriptionForUser, so that pop-up messages can distinguish which EMN they're referring to.
//TES 5/7/2015 - PLID 65969 - Added strAdditionalFolder. If set, the file will be saved there in addition to the patient's history folder
CString CreateNewCancerCaseDocument(long nPatientID, long nPicID, long nEMNID, const long &nDiagCodeID, const CString &strDiagCode, const CString &strDiagDesc, const COleDateTime &dtLastModified, const COleDateTime &dtEmnDate, CWnd *pMessageParent, CancerCaseFailureInfo *pFailureInfo, const CString &strEmnDescriptionForUser, const CString &strAdditionalFolder = "");

// (r.farnworth 2014-09-03 12:07) - PLID 63425 - When a spawned diagnosis code that is spawned linked with a problem is changed in the codes topic, we need to update that diagnosis code in the problem dialog
void LinkDiagnosisCodeToProblem(EMNDiagCode *pDiag);

// (j.jones 2014-08-12 09:41) - PLID 63189 - sends an EMRTemplateT tablechecker,
// and tells our NexEMR tab to refresh, if it exists and is active
void RefreshEMRTemplateTable();

// (b.savon 2015-12-29 10:18) - PLID 58470 - Do not allow an EMN to be locked if it has an incomplete Rx associated with it.
bool HasUnsettledPrescriptions(long emnID, bool showMessage = true);

// (z.manning 2016-04-11 13:39) - NX-100140 - Determines if we should spawn a problem (may want to skip if it's a duplicate)
bool ShouldSpawnEmrProblemForPatient(long nPatientPersonID, long nEmrProblemActionID, std::set<long> setProblemIDsToIgnore);

// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about the EMR object we are saving to NxLog
void LogEmrObjectData(EmrSaveObjectType esotSaveType, long nObjectPtr, const Nx::Quantum::Batch& finalBatch);

// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
// nIndent - The number of spaces to indent this log line
// nID - The ID of the object, or a non-positive number if it isn't saved yet
// pAddress - The memory address of the object
// esotSaveObject - The type of object
// bNew - True if the object is flagged as new
// bModified - True if the object is flagged as modified
// bDeleted - True if the object is flagged as deleted
// strName - The name of the object
// szExtraDesc - Additional information for this object
void LogEmrObjectData(int nIndent, int nID, void* pAddress, EmrSaveObjectType esotSaveObject, BOOL bNew, BOOL bModified, BOOL bDeleted, const CString& strName, LPCTSTR szExtraDesc, ...);

#endif