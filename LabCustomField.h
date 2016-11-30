// (r.gonet 08/22/2011) - PLID 45617 - Added

#pragma once

#include "LabCustomFieldControlManager.h"
// (c.haag 2015-02-23) - PLID 63751 - ELabCustomFieldType and ELabCustomFieldDisplayType are already declared here
#include "NxPracticeSharedLib/LabCustomFieldTypes.h"

////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////

class CLabCustomFieldInstance;
typedef boost::shared_ptr<class CLabCustomField> CLabCustomFieldPtr;
// (r.gonet 09/21/2011) - PLID 45583 - Define a shared pointer type for list fields.
typedef boost::shared_ptr<class CLabCustomListField> CLabCustomListFieldPtr;
typedef boost::shared_ptr<class CCFTemplateField> CCFTemplateFieldPtr;

////////////////////////////////////////////////////////////////////////////////
// Custom Field Abstract Base Class
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 08/22/2011) - PLID 45617 - A CLabCustomField (lab custom field) is an abstract base class for all lab custom field types.
//  It defines common properties between all fields and handles saving, loading, accessors and mutators
//  and a number of other responsibilities. The lab custom field defines a dynamic field that is user
//  customizable and can be displayed on the screen as a normal control. Instances of the field can be
//  created as CLabCustomFieldInstances (lab custom field instances), which in turn can store a value
//  that was entered into this control. If no instance is created, values entered into this control
//  can be stored as a default value to be remembered for all field instances. This class serves as an
//  active record for LabCustomFieldsT.
class CLabCustomField : public enable_shared_from_this<CLabCustomField>
{
public:
	// Defines modes that will determine how a field is sized on the screen.
	enum ECustomFieldSizingMode
	{
		cfsmAutoSize = 0, // Control created from this field is sized automatically and the screen width determines its size.
		cfsmFixedSize = 1, // Control created from this field is sized based on a user defined width and height.
	};

	// Defines modification statuses that enable tracking of changes made to the field.
	enum EModificationStatus
	{
		emsNone, // No mods. Not saved.
		emsNew, // Has just been created. Will be inserted.
		emsModified, // Has just been modified. Will be updated.
		emsDeleted,	 // Has just been deleted. Will be marked as deleted. Unused currently.
	};

protected:
	// (r.gonet 08/22/2011) - PLID 45617 - Database ID of the custom field. References LabCustomFieldsT(ID). May be negative if field has never been saved before.
	long m_nID;
	// (r.gonet 09/21/2011) - PLID 45557 - A GUID allowing the unique identifying of this field across all databases.
	CString m_strGUID;
	// (r.gonet 08/22/2011) - PLID 45617 - The internal name of this field.
	CString m_strName;
	// (r.gonet 08/22/2011) - PLID 45617 - The displayed name of this field.
	CString m_strFriendlyName;
	// (r.gonet 08/22/2011) - PLID 45617 - The description of this field.
	CString m_strDescription;
	// (r.gonet 08/22/2011) - PLID 45617 - The type of value stored in this field.
	ELabCustomFieldType m_lcftType;
	// (r.gonet 08/22/2011) - PLID 45617 - How the field displays the value as a control.
	ELabCustomFieldDisplayType m_lcdtDisplayType;
	// (r.gonet 08/22/2011) - PLID 45617 - The default value to place in a generated input control when no instance of this field exists.
	_variant_t m_varDefaultValue;
	// (r.gonet 08/22/2011) - PLID 45617 - The modification status of this field.
	EModificationStatus m_emsModificationStatus;
	// (r.gonet 08/22/2011) - PLID 45617 - Whether changes to this field are tracked or not by m_emsModificationStatus
	bool m_bTrackChanges;
	// (r.gonet 08/22/2011) - PLID 45617 - The mode for sizing the generated input control.
	ECustomFieldSizingMode m_cfsmSizeMode;
	// (r.gonet 08/22/2011) - PLID 45617 - Width of the input control when in cfsmFixedSize mode. VT_NULL if not in that mode.
	_variant_t m_varWidth;
	// (r.gonet 08/22/2011) - PLID 45617 - Height of the input control when in cfsmFixedSize mode. VT_NULL if not in that mode.
	_variant_t m_varHeight;
	// (r.gonet 08/22/2011) - PLID 45617 - Maximum character length enterable into the input control.  Currently applicable to text fields only. VT_NULL if not applicable.
	_variant_t m_varMaxLength;

	void CreateHeaderControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance = NULL);
	virtual void PerformDBInsert();
	virtual void AppendUpdateSql(CParamSqlBatch &sqlBatch);
	static CLabCustomFieldPtr LoadField(ADODB::_RecordsetPtr prs);

public:
	// Creates a new empty field that will be inserted to the database if saved. Note this class is abstract,
	//  so this constructor can only be called through its derived classes.
	CLabCustomField();
	CLabCustomField(const CLabCustomField &other);
	virtual ~CLabCustomField();

	long GetID() const;
	void SetID(long nID);

	// (r.gonet 09/21/2011) - PLID 45557
	CString GetGUID() const;
	void SetGUID(CString strGUID);

	CString GetName() const;
	void SetName(CString strName);
	CString GetFriendlyName() const;
	void SetFriendlyName(CString strFriendlyName);
	CString GetDescription() const;
	void SetDescription(CString strDescription);
	_variant_t GetDefaultValue() const;
	void SetDefaultValue(_variant_t varDefaultValue);

	ELabCustomFieldType GetFieldType() const;
	void SetFieldType(ELabCustomFieldType lcftType);
	ELabCustomFieldDisplayType GetDisplayType() const;
	void SetDisplayType(ELabCustomFieldDisplayType lcdtType);

	void TrackChanges(bool bTrackChanges);
	EModificationStatus GetModificationStatus() const;
	void SetModificationStatus(EModificationStatus emsModificationStatus);

	virtual void CreateControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance = NULL) = 0;
	virtual bool SetValueFromControl(CLabCustomFieldControlManager::CFieldCell *pFieldCell, CString &strErrorString, CLabCustomFieldInstance *pInstance = NULL) = 0;
	virtual void Save(CParamSqlBatch &sqlBatch);
	static CLabCustomFieldPtr Load(long nFieldID);
	static bool IsInUse(long nFieldID);

	ECustomFieldSizingMode GetSizeMode() const;
	void SetSizeMode(ECustomFieldSizingMode cfsmSizingMode);
	long GetWidth() const;
	void SetWidth(long nWidth);
	long GetHeight() const;
	void SetHeight(long nHeight);
};

////////////////////////////////////////////////////////////////////////////////
// Dummy Empty Lab Custom Field
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45581 - A CLabCustomEmptyField (empty lab custom field) is a special case to replace the use of null for custom fields.
//  No controls can be created from this and this field cannot be saved.
class CLabCustomEmptyField : public CLabCustomField
{
public:
	CLabCustomEmptyField();
	CLabCustomEmptyField(const CLabCustomField &other);
	virtual ~CLabCustomEmptyField();

	virtual void CreateControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance = NULL);
	virtual bool SetValueFromControl(CLabCustomFieldControlManager::CFieldCell *pFieldCell, CString &strErrorString, CLabCustomFieldInstance *pInstance = NULL);
	virtual void Save(CParamSqlBatch &sqlBatch);
};

////////////////////////////////////////////////////////////////////////////////
// Boolean Lab Custom Field
////////////////////////////////////////////////////////////////////////////////

// CLabCustomBooleanField (boolean lab custom field) is a custom field that defines a boolean value and can create a 
//  checkbox control for it.
class CLabCustomBooleanField : public CLabCustomField
{
public:
	CLabCustomBooleanField();
	CLabCustomBooleanField(const CLabCustomField &other);
	virtual ~CLabCustomBooleanField();

	virtual void CreateControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance = NULL);
	virtual bool SetValueFromControl(CLabCustomFieldControlManager::CFieldCell *pFieldCell, CString &strErrorString, CLabCustomFieldInstance *pInstance = NULL);
};

////////////////////////////////////////////////////////////////////////////////
// Integer Lab Custom Field
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45581 - CLabCustomIntegerField (integer lab custom field) is a custom field that defines an integer value and can create a
//  textbox control (that accepts only ints) for it.
class CLabCustomIntegerField : public CLabCustomField
{
public:
	CLabCustomIntegerField();
	CLabCustomIntegerField(const CLabCustomField &other);
	virtual ~CLabCustomIntegerField();

	virtual void CreateControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance = NULL);
	virtual bool SetValueFromControl(CLabCustomFieldControlManager::CFieldCell *pFieldCell, CString &strErrorString, CLabCustomFieldInstance *pInstance = NULL);
};

////////////////////////////////////////////////////////////////////////////////
// Float Lab Custom Field
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45581 - CLabCustomFloatField (float lab custom field) is a custom field that defines a float value and can create a
//  textbox control (that accepts only double values) for it.
class CLabCustomFloatField : public CLabCustomField
{
public:
	CLabCustomFloatField();
	CLabCustomFloatField(const CLabCustomField &other);
	virtual ~CLabCustomFloatField();

	virtual void CreateControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance = NULL);
	virtual bool SetValueFromControl(CLabCustomFieldControlManager::CFieldCell *pFieldCell, CString &strErrorString, CLabCustomFieldInstance *pInstance = NULL);
};

////////////////////////////////////////////////////////////////////////////////
// Text Lab Custom Field
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45582 - CLabCustomTextField (text lab custom field) is a custom field that defines a string value and can create a
//  textbox control (that accepts only strings) for it. This one is interesting because depending on the maximum
//  length of the field, set by the user, the value is either stored in a smaller nvarchar database field or a larger ntext field.
class CLabCustomTextField : public CLabCustomField
{
public:
	CLabCustomTextField(ELabCustomFieldType lcftType = lcftText);
	CLabCustomTextField(const CLabCustomField &other, ELabCustomFieldType lcftType = lcftText);
	virtual ~CLabCustomTextField();

	long GetMaxLength() const;
	void SetMaxLength(long nMaxLength);

	virtual void CreateControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance = NULL);
	virtual bool SetValueFromControl(CLabCustomFieldControlManager::CFieldCell *pFieldCell, CString &strErrorString, CLabCustomFieldInstance *pInstance = NULL);
	virtual void Save(CParamSqlBatch &sqlBatch);
};

////////////////////////////////////////////////////////////////////////////////
// Date/Time Lab Custom Field
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45581 - CLabCustomDateTimeField (date/time lab custom field) is a custom field that defines a datetime value and can create a
//  NxTime control for it.
class CLabCustomDateTimeField : public CLabCustomField
{
public:
	CLabCustomDateTimeField();
	CLabCustomDateTimeField(const CLabCustomField &other);
	~CLabCustomDateTimeField();

	void CreateControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance = NULL);
	bool SetValueFromControl(CLabCustomFieldControlManager::CFieldCell *pFieldCell, CString &strErrorString, CLabCustomFieldInstance *pInstance = NULL);
};

////////////////////////////////////////////////////////////////////////////////
// List Lab Custom Field Item
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45583 - CLabCustomListFieldItem (list lab custom field item) is a subatomic class that defines the items in a 
//  list field. It is an active record for LabCustomFieldListItemsT.
class CLabCustomListFieldItem
{
public:
	// Defines modification statuses for this item.
	enum EModificationStatus
	{
		emsNone, // No mods. Will not be saved.
		emsNew, // Has just been created. Will be inserted.
		emsDeleted, // Has just been deleted. Will be marked as deleted in the database.
		emsModified, // Has been modified since loading. Will be updated.
	};
private:
	// (r.gonet 09/21/2011) - PLID 45583 - Database ID for the list item. References LabCustomFieldListItemsT(ID). May be negative if the item has never been saved before.
	long m_nID;
	// (r.gonet 09/21/2011) - PLID 45583 - The internal value for this item.
	_variant_t m_varValue;
	// (r.gonet 09/21/2011) - PLID 45583 - The internal value type. List type is not allowed.
	ELabCustomFieldType m_lcftType;
	// (r.gonet 09/21/2011) - PLID 45583 - The displayed description for this item.
	CString m_strDescription;
	// (r.gonet 09/21/2011) - PLID 45583 - The order this item appears in a list field.
	long m_nSortOrder;
	// (r.gonet 09/21/2011) - PLID 45583 - Whether this item has been marked deleted in the database. If true, this is a historical value.
	bool m_bDeleted;
	// (r.gonet 09/21/2011) - PLID 45557 - A GUID that lets this list item be identified across all databases.
	CString m_strGUID;
	// (r.gonet 09/21/2011) - PLID 45583 - The modification status for this item. Affects saving.
	EModificationStatus m_emsModificationStatus;
public:
	CLabCustomListFieldItem();
	CLabCustomListFieldItem(long nID, _variant_t varValue, ELabCustomFieldType lcftType, CString strDescription, long nSortOrder);
	CLabCustomListFieldItem(CLabCustomListFieldItem &other);

	void CloneFrom(CLabCustomListFieldItem &other);
	void Save(CParamSqlBatch &sqlBatch, long nFieldID);
	static bool IsInUse(long nListItemID);
	void SetModificationStatus(EModificationStatus emsModificationStatus);
	EModificationStatus GetModificationStatus() const;
	long GetID() const;
	_variant_t GetValue() const;
	void SetValue(_variant_t varValue);
	ELabCustomFieldType GetType() const;
	void SetType(ELabCustomFieldType lcftType);
	CString GetDescription() const;
	void SetDescription(CString strDescription);
	long GetSortOrder();
	void SetSortOrder(long nSortOrder);
	// (r.gonet 09/21/2011) - PLID 45558
	bool GetDeleted() const;
	void SetDeleted(bool bDeleted);
	// (r.gonet 09/21/2011) - PLID 45557
	void SetGUID(CString strGUID);
};

////////////////////////////////////////////////////////////////////////////////
// List Lab Custom Field
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45583 - CLabCustomDateTimeField (date/time lab custom field) is a custom field that defines a list of items that it contains
//  and that will be displayed in a combo box when this field is printed to the screen. This field's value and default value
//  is the ID of one of its items.
class CLabCustomListField : public CLabCustomField
{
public:
	// Defines columns for the dynamically created combo box control.
	enum ELabCustomListFieldColumns
	{
		lcflID = 0,
		lcflSortOrder,
		lcflDescription,
	};

private:
	// (r.gonet 09/21/2011) - PLID 45583 - Collection of all items in the list field. Item ID goes to the Item object.
	std::map<long, CLabCustomListFieldItem *> m_mapItems;
public:
	CLabCustomListField();
	CLabCustomListField(const CLabCustomField &other);
	virtual ~CLabCustomListField();

	virtual void CreateControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance = NULL);
	virtual bool SetValueFromControl(CLabCustomFieldControlManager::CFieldCell *pFieldCell, CString &strErrorString, CLabCustomFieldInstance *pInstance = NULL);

	static void LoadItem(ADODB::_RecordsetPtr prs, CLabCustomListFieldPtr pListField);
	virtual void PerformDBInsert();
	virtual void AppendUpdateSql(CParamSqlBatch &sqlBatch);

	CLabCustomListFieldItem * AddItem(CLabCustomListFieldItem *pItem);
	CLabCustomListFieldItem * AddItem(_variant_t varValue, ELabCustomFieldType lcftType, CString strDescription);
	CLabCustomListFieldItem * AddItem(long nID, _variant_t varValue, ELabCustomFieldType lcftType, CString strDecription, long nSortOrder);
	CLabCustomListFieldItem * GetItem(long nID);
	long GetItemCount() const;
	void RemoveItem(long nID);
	void RemoveAllItems();
	std::map<long, CLabCustomListFieldItem *>::iterator GetItemsBegin();
	std::map<long, CLabCustomListFieldItem *>::iterator GetItemsEnd();
};


///////////////////////////////////////////////////////////////////////////////
// Lab Custom Field Instance
///////////////////////////////////////////////////////////////////////////////

class CCFTemplateInstance;

// (r.gonet 09/21/2011) - PLID 45124 - A CLabCustomFieldInstance (lab custom field instance) is an instantiation of a particular 
//  lab custom field. It exists on a certain template and must reference a wrapped lab custom 
//  field. It holds the user entered value for a lab custom field for some lab. 
//  Active record for CFTemplateInstanceT.
class CLabCustomFieldInstance
{
public:
	// Defines some modification statuses which effect saving.
	enum EModificationStatus
	{
		emsNone, // No mods. Will not be saved.
		emsNew, // Has just been created. Will be inserted.
		emsModified, // Has been modified since being loaded. Will be updated. 
	};
private:
	// (r.gonet 09/21/2011) - PLID 45124 - Database ID of the template field. References CFTemplateInstanceT(ID). May be negative if the instance has never been saved before.
	long m_nID;
	// (r.gonet 09/21/2011) - PLID 45124 - What field are we an instance of
	CCFTemplateFieldPtr m_pTemplateField;
	// (r.gonet 09/21/2011) - PLID 45124 - What template instance do we belong to
	CCFTemplateInstance *m_pTemplateInstance;
	// (r.gonet 09/21/2011) - PLID 45124 - The user entered value for this field
	_variant_t m_varValue;
	// (r.gonet 09/21/2011) - PLID 45124 - The modificaton status for this instance.
	EModificationStatus m_emsModificationStatus;

public:
	CLabCustomFieldInstance();
	CLabCustomFieldInstance(CCFTemplateFieldPtr pTemplateField, CCFTemplateInstance *pTemplateInstance);
	CLabCustomFieldInstance(long nID, CCFTemplateFieldPtr pTemplateField, CCFTemplateInstance *pTemplateInstance);

	CCFTemplateFieldPtr GetTemplateField() const;

	long GetID() const;
	_variant_t GetValue() const;
	void SetValue(_variant_t &varValue);

	void SetModificationStatus(EModificationStatus emsModificationStatus);
	EModificationStatus GetModificationStatus() const;

	void Save(CParamSqlBatch &sqlBatch);
	void PerformDBInsert();
	void AppendUpdateSql(CParamSqlBatch &sqlBatch);
	// (r.gonet 10/07/2011) - PLID 44212 - Required for lab barcode merging of the custom fields
	CLabCustomFieldPtr GetField();
};