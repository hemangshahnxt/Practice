//ExportUtils.h
//TES 4/22/2005 - Mostly just declares enums that the export stuff uses.
#ifndef EXPORTUTILS_H
#define EXPORTUTILS_H

#pragma once



enum ExportRecordType
{
	ertPatients,
	ertAppointments,
	ertCharges,
	ertPayments,
	ertEMNs,
	ertHistory, // (z.manning 2009-12-10 13:48) - PLID 36519
};

enum ExportOutputType
{
	eotCharacterSeparated,
	eotFixedWidth,
};

enum ExportFieldType
{
	eftPlaceholder, //Length|Right-justified|fill character|Text for the placeholder
					//5     |0              |_             |""
	eftGenericText, //Length|Right-justified|fill character|Capitalized
					//10    |0              |_             |0
	eftGenericNumber, //Length|Right-justified|Fill character|Fixed?|# of places|Decimal character|Use ()
					  //10    |1              |0             |0     |2          |.                |0
	eftDateTime, //Length|Right-justified|Fill character|Format
				 //10    |0              |_             |%c
	eftCurrency, //Length|Right-justified|Fill character|Fixed?|# of places|Decimal character|Use ()|Currency Symbol|Symbol Placement
				 //10    |1              |_             |1     |2          |.                |1     |$              |1
	eftPhoneNumber, //Length|Right-justified|Fill character|Format
					//14    |0              |_             |(###) ###-####
	eftSSN, //Length|Right-justified|Fill character|Include hyphens
			//11    |0              |_             |1
	eftBool, //Length|Right-justified|Fill character|True value|False value|Unknown value
			 //1     |0              |_             |T         |F          |U
	eftGender, //Length|Right-justified|Fill character|Male value|Female value|Unknown value
			   //1     |0              |_             |M         |F           |U
	eftMarital, //Length|Right-justified|Fill character|Single value|Married value|Unknown value
				//1     |0              |_             |S           |M            |U
	eftDiag, //Length|Right-justified|Fill character|Fixed?|# of places|Decimal character
			 //6     |0              |_             |0     |2          |.
	eftAdvanced, //Length|Right-justified|fill character|Field
				 //5     |0              |_             |""
	eftGenericNtext, //Length|Right-justified|fill character|Capitalized
					//10    |0              |_             |0
	eftEmnItem,	//Length|Right-justified|fill character|Sentence format
				//10    |0              |_             |0
};

enum ExportFilterOption
{
	efoAllNew, //This type refers to all records created since the last time the export was run

	// (z.manning 2009-12-14 10:15) - PLID 36576 - These have been combined into one option so these
	// 2 enums values should no longer be used.
	efoDateFilter_DO_NOT_USE,
	efoLwFilter_DO_NOT_USE,

	efoManual,
	//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate appointment exports based on created date or modified date
	efoAllNewModified, //This type refers to all records modified since the last time the export was run
	// (z.manning 2009-12-14 09:09) - PLID 36576 - Date and LW filters are now combined into one option
	efoDateOrLwFilter,
};

// (z.manning 2009-12-14 09:21) - PLID 36576 - Used in ExportT.FilterOptions. These are bitwise flags so make
// sure you increase future values by a power of 2.
enum ExportFilterFlags
{
	effNone = 0x0000,
	effDate = 0x0001,
	effLetterWriting = 0x0002,
};

enum DateFilterOption
{
	dfoToday,
	dfoYesterday,
	dfoTomorrow,
	dfoFirstOfWeek,
	dfoFirstOfMonth,
	dfoFirstOfYear,
	dfoLastOfWeek,
	dfoLastOfMonth,
	dfoLastOfYear,
	dfoFirstOfLastMonth,
	dfoOneMonthAgo,
	dfoLastOfLastMonth,
	dfoFirstOfLastYear,
	dfoOneYearAgo,
	dfoLastOfLastYear,
	dfoCustom,
};

enum FilterableDate
{
	fdFirstContactDate,
	fdHasAppointmentDate,
	fdNextAppointmentDate,
	fdLastAppointmentDate,
	fdAppointmentDate,
	fdAppointmentInputDate,
	fdServiceDate,
	fdInputDate,
	fdBillDate,
	fdAttachDate, // (z.manning 2009-12-10 14:30) - PLID 36519
};

enum DateInterval
{
	diDays,
	diWeeks,
	diMonths,
};

#define EFS_PATIENTS		0x0001
#define EFS_APPOINTMENTS	0x0002
#define EFS_CHARGES			0x0004
#define EFS_PAYMENTS		0x0008
#define EFS_EMNS			0x0010

class CExportField
{
public:

	CExportField(int nID, CString strDisplayName, CString strField, ExportFieldType eftType, DWORD dwSupportedRecordTypes, BOOL bHasAJoinOrFromClause);

	int nID;
	ExportFieldType eftType;
	DWORD dwSupportedRecordTypes;
	BOOL bHasAJoinOrFromClause;

	CString GetDisplayName();
	// (a.walling 2011-03-11 15:24) - PLID 42786 - Need to mask SSN for all exports with patient info rather than simply patient exports!
	// It is much simpler to just handle it here rather than mess with the query, since the SSNMask thing is not dependent on data at all.
	// Now GetField() will format as appropriate if the ExportFieldType is eftSSN.
	CString GetField();

protected:
	CString strDisplayName;
	CString strField;
};

extern const CExportField g_arExportFields[];
extern const long g_nExportFieldCount;

CExportField GetFieldByID(int nID);
//TES 6/29/2007 - PLID 26396 - Similar to GetFieldByID(), this function returns the index in the global array of the
// given ID.  At the moment, this is only used in an ASSERT statement to check for duplicated Field IDs.
int GetFieldIndex(int nID);

struct SelectedExportField {
	long nID; //References the g_arExportFields id.
	CString strFormat;
	long nDynamicID;
	BOOL bHasAJoinOrFromClause;
};

struct SpecialChar {
	CString strSourceChar;
	CString strReplaceChar;
};

struct SortField {
	long nID; //References the g_arExportFields.id.
	bool bSortDescending; //Which way to sort.
	CString strExtraInfo; //Only used for the {Advanced} sort field.
};

CString GetExportFromClause(ExportRecordType ertBasedOn);

//TES 6/5/2007 - PLID 26125 - This returns a statement that should always be included in the WHERE clause for this type
// (for example, "LineChargesT.Deleted=0", note that it doesn't include the "WHERE").  Also, this function will return
// an atomic statement, meaning that you don't have to use extra parentheses for it.
CString GetBaseWhereClause(ExportRecordType ertBasedOn);

CString FormatExportField(_variant_t varField, ExportFieldType eft, const CString &strFormat, bool bFixedWidth, CArray<SpecialChar,SpecialChar> &arSpecialChars);

//Gets whatever's between the nth and n+1th pipes (|) in the given string; if there aren't n pipes returns strDefault.
CString GetNthField(const CString &strFull, int n, CString strDefault);

class CPatientExportRestrictions
{
public:
	CPatientExportRestrictions();

	// (a.walling 2010-10-04 13:30) - PLID 40738 - Is patient export restricted?
	// (a.walling 2010-10-05 13:25) - PLID 40822 - bSecureCheck will contact the license activation server to verify the timestamp
	bool Enabled(bool bSecureCheck);

	// (a.walling 2010-10-04 13:30) - PLID 40738 - Is this field allowed in a restricted export?
	bool IsFieldRestricted(int nID);

	// (a.walling 2010-10-04 13:30) - PLID 40738 - Explain the current export restrictions
	CString Description();

	long Limit();

	// (a.walling 2010-10-05 13:25) - PLID 40822 - Set the 'sunrise' when restrictions come back into effect
	void LiftRestrictions();

protected:
	
	long m_nLimit;

	// (a.walling 2010-10-05 13:25) - PLID 40822 - The 'sunrise' when restrictions come back into effect
	COleDateTime m_dtUtcSunrise;

	// (z.manning 2011-02-14 15:29) - PLID 42443
	bool m_bEnabled;

	// (a.walling 2010-10-05 13:25) - PLID 40822 - Load the limit and the sunrise time
	void LoadSettings();
};

CPatientExportRestrictions& GetPatientExportRestrictions();
// (a.walling 2010-10-05 13:25) - PLID 40822 - Set the 'sunrise' when restrictions come back into effect
void LiftPatientExportRestrictionsUntil(const COleDateTime& dtUtcSunrise);

#endif