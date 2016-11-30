// (r.gonet 11/11/2011) - PLID 44212 - Support the generation of 2D barcodes for lab orders.
//  This is currently specific to LabCorp. If this ever needs modified to not be specific to LabCorp,
//  then the LabBarcodePartsT table would need modified to take a LabID or an HL7GroupID.

#pragma once

#include "LabCustomField.h"

using namespace ADODB;

// (r.gonet 12/12/2011) - PLID 44212 - Set this to 1 if you are creating or editing the structure 
//  of the barcode. You'll need to put a call to InitStructure somewhere as well.
#define BARCODE_STRUCTURE_SETUP 0

// (r.gonet 11/11/2011) - PLID 44212 - A lab barcode part may be filled in the following ways:
enum ELabBarcodePartFillType
{
	lbpftAuto = 0, // Automatically filled by code at runtime
	lbpftText, // Filled by text enterable by the user
	lbpftField, // Filled by a lab custom field
	lbpftContainer, // Contained components fill the value
};

// (r.gonet 11/11/2011) - PLID 44212 - Smart shared pointer for barcode parts. Counts references and frees the barcode part object
//  when no references exist any longer.
typedef boost::shared_ptr<class CLabBarcodePart> CLabBarcodePartPtr;

// (r.gonet 11/11/2011) - PLID 44212 - Barcode component, recursively defined to represent segments, fields, and components.
//  We don't need the distinction between segments, fields, and components. There used to be classes for all three
//  but that got cumbersome and there was no point to the distinction. I used an approach similar to XML
//  where every token of the XML is a node. Now everything is just a 'part' of the whole barcode and each part is a 
//  member of a hierarchy.
//  Though the code doesn't care, there are still distinct parts for LabCorp type 2D barcodes:
//    Root - This contains all segments and is purely a container for them. There can be only one root. The root has no parent.
//    Segment - These contain fields and are purely containers. There can be many of them and are 
//      direct children of the root. Each segment is separated from each other by a line break.
//    Field - These are atomic parts in that they may hold values and are direct chilren of segments. The 0th field in each segment should be the segment identifier.
//      They are separated from each other by vertical bars. Instead of values, they may optionally contain components instead of values, in which case they are containers.
//    Component - These are subatomic parts. Their direct parents are fields. They can have no children. They always hold values. Components are
//      separated by carets. Each component relates closely in some way to the field they are a part of.
class CLabBarcodePart : public enable_shared_from_this<CLabBarcodePart>
{
public:
	enum EModificationStatus
	{
		// Part is unchanged and won't be saved.
		emsNone,
		// Part is new and must be inserted into the database.
		emsNew,
		// Part is changed and must be updated in the database.
		emsModified,
	};
protected:
	// (r.gonet 11/11/2011) - PLID 44212 - ID of the barcode part in the database. -1 if new or root.
	long m_nID;
	// (r.gonet 11/11/2011) - PLID 44212 - Globally Unique Identifier that spans over all databases, so we may identify this part anywhere. Blank if root.
	CString m_strUID;
	// (r.gonet 11/11/2011) - PLID 44212 - The order in which the part appears in its container. -1 if the part is root.
	long m_nIndex;
	// (r.gonet 11/11/2011) - PLID 44212 - Like a file path, defines the ancestors of this part and then the part name itself.
	//  Each ancestor is separated by a period (.). The right most ancestor is the part itself.
	//  This format allows us to define part locations in the hierarchy naturally and retains the ability to locate them at random.
	//  A example is P.18.1, which is the P segment, 18th field, 1st component. Root's path is the empty string.
	CString m_strPath;
	// (r.gonet 11/11/2011) - PLID 44212 - A short description of the barcode part, mainly from specification documents.
	CString m_strDescription;
	// (r.gonet 11/11/2011) - PLID 44212 - How this part will be filled or if it is a container.
	ELabBarcodePartFillType m_lbpftFillType;
	// (r.gonet 11/11/2011) - PLID 44212 - The textual value of a field or component. Here are the rules:
	//   If this is an lbpftAuto part, then this will be filled at runtime when the barcode is generated.
	//   If this is an lbpftText part, then this should be filled by the user in a setup dialog (or sometimes prefilled).
	//   If this is an lbpftField part, then this will be filled at runtime by a custom field's value. Which custom field to use is defined by the user in a setup dialog.
	//   If this is an lbpftContainer part, then this will just be an empty string.
	CString m_strTextValue;
	// (r.gonet 11/11/2011) - PLID 44212 - The lab custom field the user has setup to fill this part with.
	long m_nLabCustomFieldID;
	// (r.gonet 11/11/2011) - PLID 44212 - The maximum length of this part's textual value, if it has one.
	//   Note that if the textual value goes over this maximum length, it will be truncated.
	long m_nMaxLength;
	// (r.gonet 11/11/2011) - PLID 44212 - Removes the last child separator in ToString()
	// (r.gonet 03/08/2014) - PLID 61190 - Renamed since I added a similar setting
	bool m_bTrimFinalSeparator;
	// (r.gonet 03/08/2014) - PLID 61190 - Removes all consecutive trailing separators.
	bool m_bTrimTrailingSeparators;

	// (r.gonet 11/11/2011) - PLID 44212 - A flag saying that this part has been modified or is new.
	EModificationStatus m_emsModificationStatus;

	// (r.gonet 11/11/2011) - PLID 44212 - Each part may have children parts of their own if they are a lbpftContainer part.
	CArray<CLabBarcodePartPtr, CLabBarcodePartPtr> m_aryChildren;
	// (r.gonet 11/11/2011) - PLID 44212 - The string that will separate each of this part's child parts from one another. 
	CString m_strChildSeparator;

	//TES 7/24/2012 - PLID 50393 - Added LabProcedureID
	long m_nLabProcedureID;

public:
	// (r.gonet 11/11/2011) - PLID 44212 - Creates a barcode part that has no corresponding part saved in the database.
	//   Defaults should be nMaxLength = -1, strTextValue = "", nLabCustomField = -1, strChildSeparator = ""
	//TES 7/24/2012 - PLID 50393 - Added LabProcedureID
	// (r.gonet 03/08/2014) - PLID 61190 - Added TrimTrailingSeparators to cause this barcode part to trim all consecutive, trailing separators
	CLabBarcodePart(CString strPath, long nPartIndex, CString strDescription, 
		ELabBarcodePartFillType fillType, long nMaxLength, CString strTextValue, long nLabCustomFieldID, CString strChildSeparator, long nLabProcedureID, bool bTrimFinalSeparator = true, bool bTrimTrailingSeparators = false);
	// (r.gonet 11/11/2011) - PLID 44212 - Creates a barcode part based on an existing part in the database.
	//TES 7/24/2012 - PLID 50393 - Added LabProcedureID
	// (r.gonet 03/08/2014) - PLID 61190 - Added TrimTralingSeparators to cause this barcode part to trim all consecutive, trailing separators
	CLabBarcodePart(long nID, CString strUID, CString strPath, long nPartIndex, CString strDescription, 
		ELabBarcodePartFillType fillType, long nMaxLength, CString strTextValue, long nLabCustomFieldID, CString strChildSeparator, long nLabProcedureID, bool bTrimTrailingSeparator = true, bool bTrimTrailingSeparators = false);
	// (r.gonet 11/11/2011) - PLID 44212 - Recursively constructs a string representation of this part, whether this be the root or a component.
	//   Roots for instance will construct the whole barcode. Components will construct only its own value.
	CString ToString();
	// (r.gonet 11/11/2011) - PLID 44212 - Adds a child part to this part.
	void AddChild(CLabBarcodePartPtr pNewChild);
	// (r.gonet 11/11/2011) - PLID 44212 - Returns the number of children in this part.
	inline long GetChildCount()
	{
		return m_aryChildren.GetSize();
	}
	//TES 7/24/2012 - PLID 50393 - Added.
	inline CLabBarcodePartPtr GetChild(int nIndex) {
		return m_aryChildren[nIndex];
	}
	// (r.gonet 11/11/2011) - PLID 44212 - Commits this part to the database, assuming it was changed.
	void Save(CParamSqlBatch &sqlBatch);
	// (r.gonet 11/11/2011) - PLID 44212 - Sets the modifiacion status of this part, which will affect saving.
	inline void SetModificationStatus(EModificationStatus emsModificationStatus)
	{
		m_emsModificationStatus = emsModificationStatus;
	}
	
	// Accessors and Mutators

	// (r.gonet 11/11/2011) - PLID 44212 - Gets the database ID. -1 if not saved to the database yet.
	long GetID();
	// (r.gonet 11/11/2011) - PLID 44212 - Gets the path of the part. Empty string for root.
	CString GetPath();
	// (r.gonet 11/11/2011) - PLID 44212 - Gets a description of the part.
	CString GetDescription();
	// (r.gonet 11/11/2011) - PLID 44212 - Gets how this part is filled.
	ELabBarcodePartFillType GetFillType();
	// (r.gonet 11/11/2011) - PLID 44212 - Gets how many characters this part can hold.
	long GetMaxLength();
	// (r.gonet 11/11/2011) - PLID 44212 - Gets the textual value for this part. Returns empty string if this is a container part.
	CString GetTextValue();
	// (r.gonet 11/11/2011) - PLID 44212 - Gets the lab custom field the user has associated with this part. -1 if no field associated.
	long GetLabCustomField();

	//TES 7/24/2012 - PLID 50393 - Added LabProcedureID
	inline long GetLabProcedureID() {
		return m_nLabProcedureID;
	}

	// (r.gonet 11/11/2011) - PLID 44212 - Sets the database ID for this part.
	void SetID(long nID);
	// (r.gonet 11/11/2011) - PLID 44212 - Sets the hierarchical path of the part.
	void SetPath(CString strPath);
	// (r.gonet 11/11/2011) - PLID 44212 - Sets the user visible description of this part.
	void SetDescription(CString strDescription);
	// (r.gonet 11/11/2011) - PLID 44212 - Sets how this part is filled, by text, by program, by field, or by container.
	void SetFillType(ELabBarcodePartFillType lbpftFillType);
	// (r.gonet 11/11/2011) - PLID 44212 - Sets the maximum number of characters this part may contain if it is not a container part. -1 means undefined.
	void SetMaxLength(long nMaxLength);
	// (r.gonet 11/11/2011) - PLID 44212 - Sets the textual value of this part. Container parts should have no text.
	void SetTextValue(CString strTextValue);
	// (r.gonet 11/11/2011) - PLID 44212 - Sets a custom field to fill this part. -1 means undefined.
	void SetLabCustomField(long nLabCustomFieldID);
	//TES 7/24/2012 - PLID 50393 - Added LabProcedureID
	void SetLabProcedureID(long nLabProcedureID);


	// (r.gonet 11/11/2011) - PLID 44212 - Traverses this part and its children, calling a caller specified function on each part.
	//   The caller may specify an argument that will be passed in the callback function along with the part.
	void PreOrderTraverse(void (fnCallback)(CLabBarcodePartPtr, void *), void *pArg);

	//TES 7/24/2012 - PLID 50393 - Added, used to clear parts that are associated with a different lab procedure.
	void ClearValues();

};

// (r.gonet 11/11/2011) - PLID 44212 - A barcode that represents a whole lot of information about a patient's lab and that patient. Has random access to each part of it.
class CLabBarcode
{
	friend class CLabBarcodeSetupDlg;
private:
	// (r.gonet 11/11/2011) - PLID 44212 - Maps a barcode part's path to the part itself.
	CMap<CString, LPCTSTR, CLabBarcodePartPtr, CLabBarcodePartPtr> m_mapParts;
protected:
	// (r.gonet 11/11/2011) - PLID 44212 - Gets the containing part for a particular child's path. NULL if it doesn't exist.
	CLabBarcodePartPtr GetParentPart(CString strChildPath);
	// (r.gonet 11/11/2011) - PLID 44212 - Gets a part from a path.
	CLabBarcodePartPtr GetPart(CString strPath);
	// (r.gonet 11/11/2011) - PLID 44212 - Gets the root part that contains all segments.
	CLabBarcodePartPtr GetRoot();

#if BARCODE_STRUCTURE_SETUP == 1
	// (r.gonet 11/11/2011) - PLID 44212 - Used in construction of a barcode definition. Not for production.
	//   Retaining though, one, to be reviewed, and, two, in case we need to define a new part or barcode.
	// (r.gonet 03/08/2014) - PLID 61190 - Added TrimTrailingSeparators to cause this barcode part container to trim trailing, consecutive separators
	CLabBarcodePartPtr AddPart_Container(CString strPath, CString strDescription, CString strChildSeparator, bool bTrimTrailingSeparators = false);
	CLabBarcodePartPtr AddPart_Auto(CString strPath, CString strDescription, long nMaxLength);
	CLabBarcodePartPtr AddPart_Text(CString strPath, CString strDescription, long nMaxLength, CString strText);
	CLabBarcodePartPtr AddPart_CustomField(CString strPath, CString strDescription, long nMaxLength, long nLabCustomFieldID = -1);
#endif

	// (r.gonet 11/11/2011) - PLID 44212 - Gets the textual value at a particular part path.
	CString GetValue(CString strPath);
	// (r.gonet 11/11/2011) - PLID 44212 - Sets the textual value at a particular part path.
	//  Set trim to false to not trim whitespace.
	void SetValue(CString strPath, CString strValue, bool bTrim = true);

	// (r.gonet 11/11/2011) - PLID 44212 - Formats a date field from a recordset to the desired barcode format (similar to HL7)
	CString AsBarcodeDate(_RecordsetPtr prs, CString strField);
	// (r.gonet 11/11/2011) - PLID 44212 - Formats a time field from a recordset to the desired barcode format (similar to HL7)
	CString AsBarcodeTime(_RecordsetPtr prs, CString strField);
	// (r.gonet 11/11/2011) - PLID 44212 - Strips dashes from an SSN
	CString FormatBarcodeSSN(CString strSSN);
	// (r.gonet 11/11/2011) - PLID 44212 - Strips dashes, spaces, and parens from a phone number.
	CString FormatBarcodePhone(CString strPhone);

	// (r.gonet 11/11/2011) - PLID 44212 - Merges the custom fields the user has setup to go with each part to the parts' text values.
	void MergeLabCustomFields(long nLabID);
	// (r.gonet 11/11/2011) - PLID 44212 - Takes a lab custom field ID and an instance it is in, then returns the string representation of its instance value.
	CString ConvertLabCustomFieldToString(long nFieldID, CCFTemplateInstance &cfTemplateInstance);
	// (r.gonet 11/11/2011) - PLID 44212 - Helper function for the shorter version of CovertLabCustomFieldToString.
	CString ConvertLabCustomFieldToString(_variant_t varValue, ELabCustomFieldType lcftFieldType, ELabCustomFieldDisplayType lcdtDisplayType, CLabCustomFieldInstance *pInstance);
	// (r.gonet 11/11/2011) - PLID 44212 - We need to fill some custom field filled parts manually because they require special formatting.
	//   Implementations should overwrite the value in a part with their own manually formatted value from the custom field instance.
	virtual void OverwriteCustomFields(CCFTemplateInstance &cfTemplateIntance);
public:
	// (r.gonet 11/11/2011) - PLID 44212 - Constructs a new barcode.
	CLabBarcode();
	virtual ~CLabBarcode();

#if BARCODE_STRUCTURE_SETUP == 1
	// (r.gonet 11/11/2011) - PLID 44212 - Creates an empty barcode structure. 
	//   Should only be called when barcode is under development since Load() 
	//   will do the same except it draws from the database. This should be called
	//   when the database does not have the barcode structure yet.
	//  Not referenced in live code.
	virtual void InitStructure() = 0;
#endif

	// (r.gonet 11/11/2011) - PLID 44212 - Loads the barcode definition from the database.
	void Load();
	// (r.gonet 11/11/2011) - PLID 44212 - Saves the barcode definition to the database.
	void Save();
	
	// (r.gonet 11/11/2011) - PLID 44212 - Fills the barcode's structure by merging custom fields and filling auto-fill fields.
	virtual void FillBarcode(long nPatientID, CString strFormNumberTextID, long nHL7GroupID = -1) = 0;
	// (r.gonet 11/11/2011) - PLID 44212 - Traverses the barcode parts with a callback function and allows an argument to be passed along.
	void PreOrderTraverse(void (fnCallback)(CLabBarcodePartPtr, void *), void *pArg);

	// (r.gonet 11/11/2011) - PLID 44212 - A barcode may need to be encoded. This is where that happens.
	virtual void Encode(OUT CArray<CString, CString> &aryBarcodes);
};

// LabCorp specific barcode
class CLabCorpLabBarcode : public CLabBarcode
{
private:
	// (r.gonet 11/11/2011) - PLID 46434 - I'm afraid this one is very specific and kind of long. Figures out what value should go into P.18 and sets a few output flags.
	CString CalculateP18BillCode(_RecordsetPtr prs, OUT bool &bFillMedicareNumberInP19, OUT bool &bFillSubscriberNumberInP40, OUT bool &bFillSubscriberNumberInP53);
	// (r.gonet 11/11/2011) - PLID 46434 - LabCorp specific. Splits the barcode into two parts if it is over a certain character length. 
	void SplitBarcode(OUT CArray<CString, CString> &aryBarcodes);

	// (r.gonet 12/12/2011) - PLID 46434 - Returns an age in years, months, and days.
	void GetAgeInParts(COleDateTime &dtBirthDate, COleDateTime &dtNow, OUT long &nYearAge, OUT long &nMonthAge, OUT long &nDayAge);
protected:
	virtual void OverwriteCustomFields(CCFTemplateInstance &cfTemplateIntance);
public:
	CLabCorpLabBarcode();
	~CLabCorpLabBarcode();

#if BARCODE_STRUCTURE_SETUP == 1
	// (r.gonet 11/11/2011) - PLID 46434 - Constructs this barcode with the LabCorp specific structure.
	//   Should only be called when barcode is under development since Load() 
	//   will do the same except it draws from the database. This should be called
	//   when the database does not have the barcode structure yet.
	// Not referenced in live code.
	virtual void InitStructure();
#endif
	// (r.gonet 11/11/2011) - PLID 46434 - Fills the barcode per LabCorp specs for a lab order.
	virtual void FillBarcode(long nPatientID, CString strFormNumberTextID, long nHL7GroupID = -1);

	// (r.gonet 11/11/2011) - PLID 46434 - Encode into PDF417
	virtual void Encode(OUT CArray<CString, CString> &aryBarcodes);
};