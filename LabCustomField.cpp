// (r.gonet 08/22/2011) - PLID 45617 - Added

#include "stdafx.h"
#include "GlobalDataUtils.h"
#include "MiscSystemUtils.h"
#include "LabCustomField.h"
#include "LabCustomFieldTemplate.h"
#include "LabCustomFieldControlManager.h"
#include <sstream>

using namespace ADODB;
using namespace NXDATALISTLib;

extern CPracticeApp theApp;

////////////////////////////////////////////////////////////////////////////////
// Custom Field Abstract Base Class
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 08/22/2011) - PLID 45617 - Creates a new empty field that will be inserted to the database if saved. Note this class is abstract,
//  so this constructor can only be called through its derived classes.
CLabCustomField::CLabCustomField()
{
	m_nID = -1;
	// (r.gonet 09/21/2011) - PLID 45557 - Init the field's GUID
	m_strGUID = NewUUID();
	m_strName = "";
	m_strFriendlyName = "";
	m_strDescription = "";
	m_varDefaultValue = g_cvarNull;
	m_lcftType = lcftUndefined;
	m_lcdtDisplayType = lcdtDefault;
	m_cfsmSizeMode = cfsmAutoSize;
	m_varWidth = g_cvarNull;
	m_varHeight = g_cvarNull;
	m_varMaxLength = g_cvarNull;
	m_emsModificationStatus = emsNone;
	m_bTrackChanges = true;
}

// (r.gonet 08/22/2011) - PLID 45617 - Creates a copy of an existing field. Note that this class is abstract and cannot be instantiated.
CLabCustomField::CLabCustomField(const CLabCustomField &other)
{
	m_nID = other.m_nID;
	// (r.gonet 09/21/2011) - PLID 45557 - Init the field's GUID
	m_strGUID = other.m_strGUID;
	m_strName = other.m_strName;
	m_strFriendlyName = other.m_strFriendlyName;
	m_strDescription = other.m_strDescription;
	m_varDefaultValue = other.m_varDefaultValue;
	m_lcftType = lcftUndefined; // We don't copy the field type, we'll let the subclass copy constructor do that.
	m_lcdtDisplayType = lcdtDefault; // Nor the display type
	m_cfsmSizeMode = other.m_cfsmSizeMode;
	m_varWidth = other.m_varWidth;
	m_varHeight = other.m_varHeight;
	m_varMaxLength = other.m_varMaxLength;
	m_emsModificationStatus = other.m_emsModificationStatus;
	m_bTrackChanges = true;
}

// (r.gonet 08/22/2011) - PLID 45617 - Empty base descructor for fields. No dynamic data in fields, so no need to destroy anything.
CLabCustomField::~CLabCustomField()
{
}

// (r.gonet 08/22/2011) - PLID 45617 - Gets the custom field's database ID. If the field has not been saved or loaded yet, this returns -1.
long CLabCustomField::GetID() const
{
	return m_nID;
}

// (r.gonet 08/22/2011) - PLID 45617 - Sets the cutsom field's database ID. Calling this method twice is an error. This is purely for initialization
//  and is not tracked as a modifying action.
void CLabCustomField::SetID(long nID)
{
	// It is not possible to change this within the database, so this can only be used on a uninitialized field
	if(m_nID > 0) {
		ThrowNxException("CLabCustomField::SetID : Attempted to set the ID of a field that already has an ID. SetID should only be used in initialization.");
	}
	m_nID = nID;
}

// (r.gonet 08/22/2011) - PLID 45617 - Gets the GUID of the field. If the field has been already saved to the database, getting the GUID before the field is
//  loaded results in an undefined GUID.
CString CLabCustomField::GetGUID() const
{
	return m_strGUID;
}

// (r.gonet 09/21/2011) - PLID 45557 - Set the GUID of this field. Should only be used during initialization. Not saved during updates, only inserts.
void CLabCustomField::SetGUID(CString strGUID)
{
	m_strGUID = strGUID;
}

// (r.gonet 08/22/2011) - PLID 45617 - Returns the name of the field.
CString CLabCustomField::GetName() const
{
	return m_strName;
}

// (r.gonet 08/22/2011) - PLID 45617 - Sets the name of the field
void CLabCustomField::SetName(CString strName)
{
	m_strName = strName;
	SetModificationStatus(emsModified);
}

// (r.gonet 08/22/2011) - PLID 45617 - Gets a more user friendly version of the field's name. Often just the name.
CString CLabCustomField::GetFriendlyName() const
{
	return m_strFriendlyName;
}

// (r.gonet 08/22/2011) - PLID 45617 - Sets the user friendly verion of the field's name.
void CLabCustomField::SetFriendlyName(CString strFriendlyName)
{
	m_strFriendlyName = strFriendlyName;
	SetModificationStatus(emsModified);
}

// (r.gonet 08/22/2011) - PLID 45617 - Gets the description of the field.
CString CLabCustomField::GetDescription() const
{
	return m_strDescription;
}

// (r.gonet 08/22/2011) - PLID 45617 - Sets the description of the field.
void CLabCustomField::SetDescription(CString strDescription)
{
	m_strDescription = strDescription;
	SetModificationStatus(emsModified);
}

// (r.gonet 08/22/2011) - PLID 45617 - Returns the default value of the field, whose variant type will depend on the field's type.
//  If the field has no default value, the return value will be a variant of type VT_NULL.
_variant_t CLabCustomField::GetDefaultValue() const
{
	return m_varDefaultValue;
}

// (r.gonet 08/22/2011) - PLID 45617 - Sets the default value of the field. You should set it appropriate to the field type.
//  TODO: Check input here to ensure it is the right type. Perhaps have this method virtual.
void CLabCustomField::SetDefaultValue(_variant_t varDefaultValue)
{
	m_varDefaultValue = varDefaultValue;
	SetModificationStatus(emsModified);
}

// (r.gonet 08/22/2011) - PLID 45617 - Gets the type of the field. For example, boolean, text, or list.
ELabCustomFieldType CLabCustomField::GetFieldType() const
{
	return m_lcftType;
}

// (r.gonet 08/22/2011) - PLID 45617 - Sets the field's type. This affects how this field is stored in and loaded from the database.
void CLabCustomField::SetFieldType(ELabCustomFieldType lcftType)
{
	m_lcftType = lcftType;
	SetModificationStatus(emsModified);
}

// (r.gonet 08/22/2011) - PLID 45617 - Gets how the field is displayed on the screen.
ELabCustomFieldDisplayType CLabCustomField::GetDisplayType() const
{
	return m_lcdtDisplayType;
}

// (r.gonet 08/22/2011) - PLID 45617 - Sets how the field is displayed on the screen. Not all display types are supported for each field type.
void CLabCustomField::SetDisplayType(ELabCustomFieldDisplayType lcdtType)
{
	m_lcdtDisplayType = lcdtType;
	SetModificationStatus(emsModified);
}

// (r.gonet 08/22/2011) - PLID 45617 - Sets on or off modification tracking for the custom field. For instance, use this to initialize the field.
void CLabCustomField::TrackChanges(bool bTrackChanges)
{
	m_bTrackChanges = bTrackChanges;
}

// (r.gonet 08/22/2011) - PLID 45617 - Gets the modification status of the field. It may be unchanged, new, modified, or deleted.
CLabCustomField::EModificationStatus CLabCustomField::GetModificationStatus() const
{
	return m_emsModificationStatus;
}

// (r.gonet 08/22/2011) - PLID 45617 - Sets the modification status for the field. There is an order of priority here. A field can only be
//  marked as modified though from a state of unchanged. All others take priority over modified.
void CLabCustomField::SetModificationStatus(EModificationStatus emsModificationStatus)
{
	if(!m_bTrackChanges) {
		return;
	}
	if(emsModificationStatus == emsModified) {
		if(m_emsModificationStatus == emsNone) {
			m_emsModificationStatus = emsModificationStatus;
		}
	} else {
		m_emsModificationStatus = emsModificationStatus;
	}
}

// (r.gonet 08/22/2011) - PLID 45617 - Persists the field to the database if it has been modified since it was loaded or created.
void CLabCustomField::Save(CParamSqlBatch &sqlBatch)
{
	if(GetModificationStatus() == emsNew) {
		PerformDBInsert();
	} else if(GetModificationStatus() == emsModified) {
		AppendUpdateSql(sqlBatch);
	}
}

// (r.gonet 08/22/2011) - PLID 45617 - Static function. Loads a field from the database using its database ID.
CLabCustomFieldPtr CLabCustomField::Load(long nFieldID)
{
	CLabCustomFieldPtr pField;
	CParamSqlBatch sqlLoadingBatch;
	sqlLoadingBatch.Declare("DECLARE @FieldID INT; ");
	sqlLoadingBatch.Add("SET @FieldID = {INT}; ", nFieldID);
	sqlLoadingBatch.Add(
		// Get the custom fields for this procedure type
		// (r.gonet 09/21/2011) - PLID 45557 - Also get the field's GUID
		"SELECT LabCustomFieldsT.ID, LabCustomFieldsT.UID, LabCustomFieldsT.Name, LabCustomFieldsT.FriendlyName, LabCustomFieldsT.Description, "
		// Field settings
		"	LabCustomFieldsT.DataTypeID, LabCustomFieldsT.DisplayTypeID, LabCustomFieldsT.MaxLength, "
		"	LabCustomFieldsT.SizeMode, LabCustomFieldsT.Width, LabCustomFieldsT.Height, "
		// Field defaults
		"	LabCustomFieldsT.DefaultSelectionID, LabCustomFieldsT.DefaultIntParam, LabCustomFieldsT.DefaultFloatParam, "
		"	LabCustomFieldsT.DefaultTextParam, LabCustomFieldsT.DefaultMemoParam, LabCustomFieldsT.DefaultDateTimeParam "
		"FROM LabCustomFieldsT "
		"WHERE LabCustomFieldsT.ID = @FieldID; ");
	sqlLoadingBatch.Add(
		// Get the list items for this field
		// (r.gonet 09/21/2011) - PLID 45557 - Also get the list items' GUID
		"SELECT LabCustomFieldListItemsT.ID, LabCustomFieldListItemsT.FieldID, LabCustomFieldListItemsT.Description, LabCustomFieldListItemsT.DataTypeID, "
		"	LabCustomFieldListItemsT.IntParam, LabCustomFieldListItemsT.FloatParam, LabCustomFieldListItemsT.TextParam, "
		"	LabCustomFieldListItemsT.MemoParam, LabCustomFieldListItemsT.DateTimeParam, "
		"	LabCustomFieldListItemsT.SortOrder, LabCustomFieldListItemsT.Deleted AS ListItemDeleted, LabCustomFieldListItemsT.UID "
		"FROM LabCustomFieldsT "
		"	INNER JOIN "
		"LabCustomFieldListItemsT ON LabCustomFieldsT.ID = LabCustomFieldListItemsT.FieldID "
		"WHERE LabCustomFieldsT.ID = @FieldID "
		"ORDER BY LabCustomFieldListItemsT.SortOrder ASC; ");
	
	_RecordsetPtr prs = sqlLoadingBatch.CreateRecordset(GetRemoteData());
	if(prs->eof) {
		prs->Close();
		ThrowNxException("CLabCustomField::Load : Attempted to load a non-existent field.");
	} else {
		// Load the field's common values.
		pField = LoadField(prs);
	}
	// (r.gonet 09/21/2011) - PLID 45583 - Next recordset of the list items
	prs = prs->NextRecordset(NULL);
	// (r.gonet 09/21/2011) - PLID 45583 - We should only have list items if we are a list. If there are list items and we are not a list, that is data corruption.
	ASSERT(prs->RecordCount == 0 || pField->m_lcftType == lcftSingleSelectList);
	if(pField->m_lcftType == lcftSingleSelectList) {
		CLabCustomListFieldPtr pListField = boost::dynamic_pointer_cast<CLabCustomListField>(pField);
		if(pListField == NULL) {
			// (r.gonet 09/21/2011) - PLID 45583 - You shouldn't be able to get here since if the field has a datatype of list, a list field is created for it right above.
			ThrowNxException("CLabCustomField::Load : Could not cast field as a list field.");
		}
		// (r.gonet 09/21/2011) - PLID 45583 - Load the list field with all of its items.
		while(!prs->eof) {
			CLabCustomListField::LoadItem(prs, pListField);
			prs->MoveNext();
		}
	}
	prs->Close();

	pField->TrackChanges(true);
	return pField;
}

// (r.gonet 08/22/2011) - PLID 45617 - Loads a field (minus derived class specific values) from a recordset.
CLabCustomFieldPtr CLabCustomField::LoadField(_RecordsetPtr prs)
{
	// This is a factory method, so we have to switch.
	CLabCustomFieldPtr pField;
	long nID = VarLong(prs->Fields->Item["ID"]->Value);
	ELabCustomFieldType lcftType = (ELabCustomFieldType)VarLong(prs->Fields->Item["DataTypeID"]->Value, -1);			
	_variant_t varValue, varDefault;
	switch(lcftType) {
		// (r.gonet 09/21/2011) - PLID 45581 - Added support for boolean fields.
		case lcftBool:
			pField = make_shared<CLabCustomBooleanField>();
			varDefault = VarLong(prs->Fields->Item["DefaultIntParam"]->Value, 0) != 0 ? g_cvarTrue : g_cvarFalse;
			break;
		// (r.gonet 09/21/2011) - PLID 45581 - Added support for integer fields.
		case lcftInteger:
			pField = make_shared<CLabCustomIntegerField>();
			varDefault = prs->Fields->Item["DefaultIntParam"]->Value;
			break;
		// (r.gonet 09/21/2011) - PLID 45581 - Added support for float fields.
		case lcftFloat:
			pField = make_shared<CLabCustomFloatField>();
			varDefault = prs->Fields->Item["DefaultFloatParam"]->Value;
			break;
		// (r.gonet 09/21/2011) - PLID 45581 - Added support for datetime fields.
		case lcftDateTime:
			pField = make_shared<CLabCustomDateTimeField>();
			varDefault = prs->Fields->Item["DefaultDateTimeParam"]->Value;
			break;
		// (r.gonet 09/21/2011) - PLID 45582 - Added support for text fields.
		case lcftText:
			pField = make_shared<CLabCustomTextField>();
			varDefault = prs->Fields->Item["DefaultTextParam"]->Value;
			break;
		// (r.gonet 09/21/2011) - PLID 45582 - Added support for memo (big text) fields.
		case lcftMemo:
			pField = make_shared<CLabCustomTextField>();
			varDefault = prs->Fields->Item["DefaultMemoParam"]->Value;
			break;
		// (r.gonet 09/21/2011) - PLID 45583 - Added support for list fields.
		case lcftSingleSelectList:
			pField = make_shared<CLabCustomListField>();
			varDefault = prs->Fields->Item["DefaultSelectionID"]->Value;
			break;
		default:
			// We attempted to load a type but we don't know what this type is.
			// Either this is bad data or somebody needs to add a handler here.
			prs->Close();
			ASSERT(false);
			ThrowNxException("CLabCustomField::LoadField : Attempted to load a custom field with an unrecognized type.");
	}
	// We know now that pField is non null.
	ASSERT(pField != NULL);
	// Now, since we are initializing this field, we don't want it to track our changes.
	pField->TrackChanges(false);
	pField->SetID(nID);
	// (r.gonet 09/21/2011) - PLID 45557 - Init the field's GUID
	pField->SetGUID(VarString(prs->Fields->Item["UID"]->Value));
	pField->SetFieldType(lcftType);
	pField->SetDisplayType((ELabCustomFieldDisplayType)VarLong(prs->Fields->Item["DisplayTypeID"]->Value));
	pField->SetName(VarString(prs->Fields->Item["Name"]->Value));
	pField->SetFriendlyName(VarString(prs->Fields->Item["FriendlyName"]->Value));
	pField->SetDescription(VarString(prs->Fields->Item["Description"]->Value));
	pField->SetDefaultValue(varDefault);
	long nWidth = VarLong(prs->Fields->Item["Width"]->Value, -1);
	long nHeight = VarLong(prs->Fields->Item["Height"]->Value, -1);
	switch(VarLong(prs->Fields->Item["SizeMode"]->Value)) {
		case CLabCustomField::cfsmFixedSize:
			pField->SetSizeMode(CLabCustomField::cfsmFixedSize);
			// We now expect the width and height fields to hold a value other than NULL.
			ASSERT(nWidth > 0 && nHeight > 0); // Why would we have no values here with the field being a fixed size?
			if(nWidth <= 0 || nHeight <= 0) {
				// Revert to auto size since fixed size is not defined.
				pField->SetSizeMode(CLabCustomField::cfsmAutoSize);
			} else {
				pField->SetWidth(nWidth);
				pField->SetHeight(nHeight);
			}
			break;
		case CLabCustomField::cfsmAutoSize:
		default:
			pField->SetSizeMode(CLabCustomField::cfsmAutoSize);
			break;
	}
	// (r.gonet 09/21/2011) - PLID 45582 - We may have a maximum length if we are a text field.
	if(pField->GetFieldType() == lcftText || pField->GetFieldType() == lcftMemo) {
		shared_ptr<CLabCustomTextField> pTextField = boost::dynamic_pointer_cast<CLabCustomTextField>(pField);
		if(pTextField) {
			pTextField->SetMaxLength(VarLong(prs->Fields->Item["MaxLength"]->Value, 100));
		}
	}

	return pField;
}

// (r.gonet 08/22/2011) - PLID 45617 - If this field is in use somewhere in past entered template instances, returns true. Otherwise returns false.
bool CLabCustomField::IsInUse(long nFieldID)
{
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT COUNT(*) AS InUseCount "
		"FROM LabCustomFieldInstancesT "
		"WHERE FieldID = {INT}; ",
		nFieldID);
	long nInUseCount = VarLong(prs->Fields->Item["InUseCount"]->Value);
	prs->Close();
	return nInUseCount > 0;
}

// (r.gonet 08/22/2011) - PLID 45617 - Gets the mode determining how this field's size is calculated when the field is displayed.
CLabCustomField::ECustomFieldSizingMode CLabCustomField::GetSizeMode() const
{
	return m_cfsmSizeMode;
}

// (r.gonet 08/22/2011) - PLID 45617 - Sets the mode determining how this field's size is calculated when the field is displayed.
void CLabCustomField::SetSizeMode(CLabCustomField::ECustomFieldSizingMode cfsmSizeMode)
{
	m_cfsmSizeMode = cfsmSizeMode;
	if(m_cfsmSizeMode != cfsmFixedSize) {
		m_varWidth.vt = VT_NULL;
		m_varHeight.vt = VT_NULL;
	}
	SetModificationStatus(emsModified);
}

// (r.gonet 08/22/2011) - PLID 45617 - Gets the stored width of the field. Should only be used in conjuction with cfsmFixedSize. Otherwise
// the result is a reasonable width for any field.
long CLabCustomField::GetWidth() const
{
	return VarLong(m_varWidth, -1);
}

// (r.gonet 08/22/2011) - PLID 45617 - Sets the stored width of the field for use in fixed size mode.
void CLabCustomField::SetWidth(long nWidth)
{
	m_varWidth = _variant_t(nWidth);
	SetModificationStatus(emsModified);
}

// (r.gonet 08/22/2011) - PLID 45617 - Gets the stored height of the field. Should only be used in conjuction with cfsmFixedSize. Otherwise
// the result is a reasonable height for any field.
long CLabCustomField::GetHeight() const
{
	return VarLong(m_varHeight, -1);
}

// (r.gonet 08/22/2011) - PLID 45617 - Sets the stored height of the field for use in fixed size mode.
void CLabCustomField::SetHeight(long nHeight)
{
	m_varHeight = _variant_t(nHeight);
	SetModificationStatus(emsModified);
}

// (r.gonet 08/22/2011) - PLID 45617 - Persists a new field to the database. Performed immediately and updates the field's database ID.
// Resets the modification status of this field.
void CLabCustomField::PerformDBInsert()
{
	// (r.gonet 09/21/2011) - PLID 45557 - Insert the GUID we generated.
	_RecordsetPtr prs = CreateParamRecordset(
		"SET NOCOUNT ON; "
		"INSERT INTO LabCustomFieldsT (Name, FriendlyName, Description, "
		"	DataTypeID, DisplayTypeID, MaxLength, "
		"	DefaultIntParam, DefaultFloatParam, DefaultTextParam, DefaultMemoParam, DefaultDateTimeParam, "
		"	SizeMode, Width, Height, UID) "
		"VALUES "
		"({STRING}, {STRING}, {STRING}, "
		"	{INT}, {INT}, {VT_I4}, "
		"	{VT_I4}, {VT_R8}, {VT_BSTR}, {VT_BSTR}, {VT_DATE}, "
		"	{INT}, {VT_I4}, {VT_I4}, {STRING}); "
		"SET NOCOUNT OFF; "
		""
		"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS NewID; ",
		m_strName, m_strFriendlyName, m_strDescription,
		m_lcftType, m_lcdtDisplayType, 
		m_varMaxLength,
		(m_lcftType == lcftInteger ? m_varDefaultValue : 
			(m_lcftType == lcftBool && m_varDefaultValue.vt == VT_BOOL ? _variant_t((long)VarBool(m_varDefaultValue)) : g_cvarNull)),
		(m_lcftType == lcftFloat ? m_varDefaultValue : g_cvarNull),
		(m_lcftType == lcftText ? m_varDefaultValue : g_cvarNull),
		(m_lcftType == lcftMemo ? m_varDefaultValue : g_cvarNull),
		(m_lcftType == lcftDateTime && m_varDefaultValue.vt == VT_DATE ? m_varDefaultValue : g_cvarNull),
		(long)m_cfsmSizeMode,
		(m_cfsmSizeMode == cfsmFixedSize ? m_varWidth : g_cvarNull),
		(m_cfsmSizeMode == cfsmFixedSize ? m_varHeight : g_cvarNull),
		m_strGUID);
	if(!prs->eof) {
		m_nID = VarLong(prs->Fields->Item["NewID"]->Value);
	} else {
		prs->Close();
		AfxThrowNxException("CLabCustomField::PerformDBInsert: Field creation query failed to return records.");
	}
	prs->Close();

	SetModificationStatus(emsNone);
}

// (r.gonet 08/22/2011) - PLID 45617 - Gets a query updating this field's persistant state in the data to what is in memory.
// Resets the field's modification status.
void CLabCustomField::AppendUpdateSql(CParamSqlBatch &sqlBatch)
{
	sqlBatch.Add(
		"UPDATE LabCustomFieldsT SET "
		"	Name = {STRING}, "
		"	FriendlyName = {STRING}, "
		"	Description = {STRING}, "
		"	DataTypeID = {INT}, "
		"	DisplayTypeID = {INT}, "
		"	MaxLength = {VT_I4}, "
		"	DefaultSelectionID = NULL, "
		"	DefaultIntParam = {VT_I4}, "
		"	DefaultFloatParam = {VT_R8}, "
		"	DefaultTextParam = {VT_BSTR}, "
		"	DefaultMemoParam = {VT_BSTR}, "
		"	DefaultDateTimeParam = {VT_DATE}, "
		"	SizeMode = {INT}, "
		"	Width = {VT_I4}, "
		"	Height = {VT_I4} "
		"WHERE LabCustomFieldsT.ID = {INT}; ",
		m_strName, m_strFriendlyName, m_strDescription,
		(int)m_lcftType, (int)m_lcdtDisplayType, m_varMaxLength,
		(m_lcftType == lcftInteger ? m_varDefaultValue :
			(m_lcftType == lcftBool && m_varDefaultValue.vt == VT_BOOL ? _variant_t((long)VarBool(m_varDefaultValue)) : g_cvarNull)),
		(m_lcftType == lcftFloat ? m_varDefaultValue : g_cvarNull),
		(m_lcftType == lcftText ? m_varDefaultValue : g_cvarNull),
		(m_lcftType == lcftMemo ? m_varDefaultValue : g_cvarNull),
		(m_lcftType == lcftDateTime && m_varDefaultValue.vt == VT_DATE ? m_varDefaultValue : g_cvarNull),
		(long)m_cfsmSizeMode,
		(m_cfsmSizeMode == cfsmFixedSize ? m_varWidth : g_cvarNull),
		(m_cfsmSizeMode == cfsmFixedSize ? m_varHeight : g_cvarNull),
		m_nID);
	// (r.gonet 09/21/2011) - PLID 45583 - If we have items but are not a list, we must delete them
	if(this->GetFieldType() != lcftSingleSelectList) {
		sqlBatch.Add(
			"DELETE FROM LabCustomFieldListItemsT "
			"WHERE FieldID = {INT}; ",
			m_nID, m_nID);
	}
	SetModificationStatus(emsNone);
}

// (r.gonet 08/22/2011) - PLID 45617 - Creates to a control manager the header fields name and description of the field. 
//  Also marks the name with (Required) if the field is required to be filled on this template.
//  pFieldInstance can be NULL if the field itself is being edited and previewed.
void CLabCustomField::CreateHeaderControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance/*= NULL*/)
{
	// Possibly this field is required
	bool bRequired = false;
	if(pFieldInstance && pFieldInstance->GetTemplateField()->GetRequired()) {
		bRequired = true;
	}

	// Create a label for the name
	CNxLabel *pNxlName = new CNxLabel();
	int success = pNxlName->Create(CString(GetFriendlyName() + (bRequired ? " (Required)" : "")), WS_CHILD|WS_VISIBLE|SS_LEFT|SS_NOTIFY|WS_GROUP, 
		CRect(cmControlManager.GetBoundingBoxBottomLeft(), CSize(10,10)), 
		cmControlManager.GetParentWindow(), cmControlManager.GetNewControlID());
	pNxlName->SetText(GetFriendlyName() + (bRequired ? " (Required)" : ""));
	pNxlName->SetType(dtsText);
	pNxlName->SetFont(&theApp.m_boldFont);
	pNxlName->ModifyStyleEx(0, WS_EX_TRANSPARENT);
	cmControlManager.SetIdealSize(pNxlName);
	if(bRequired) {
		pNxlName->SetColor(RGB(255, 0, 0));
	}
	cmControlManager.AddControl(pNxlName);

	// Now create a label for the description
	if(!GetDescription().IsEmpty()) {
		CNxEdit *pNxeDescription = new CNxEdit();
		pNxeDescription->Create(WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL|WS_DISABLED|WS_GROUP,
			CRect(cmControlManager.GetBoundingBoxBottomLeft(), CSize(cmControlManager.GetDrawingSize().cx - 20, 20)),
			cmControlManager.GetParentWindow(), cmControlManager.GetNewControlID());
		pNxeDescription->SetWindowText(GetDescription());
		pNxeDescription->SetReadOnly(TRUE);
		pNxeDescription->ModifyStyleEx(0, WS_EX_TRANSPARENT);
		pNxeDescription->SetFont(&theApp.m_notboldFont);
		
		// we need to size the height of this by how many pixels of height the font is and the number of lines it takes up.
		CDC dc;
		dc.Attach(::GetDC(cmControlManager.GetParentWindow()->GetSafeHwnd()));

		CSize szDescription = dc.GetTextExtent(GetDescription());
		CRect rcControl;
		pNxeDescription->GetWindowRect(&rcControl);
		cmControlManager.GetParentWindow()->ScreenToClient(&rcControl);
		pNxeDescription->MoveWindow(rcControl.left, rcControl.top, rcControl.Width(), pNxeDescription->GetLineCount() * szDescription.cy);

		dc.Detach();
		cmControlManager.SetIdealSize(pNxeDescription, true);

		cmControlManager.AddControl(pNxeDescription);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Dummy Empty Lab Custom Field
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45581 - Creates an empty field, which is a special case to replace the use of null for custom fields.
CLabCustomEmptyField::CLabCustomEmptyField()
: CLabCustomField()
{
	m_lcftType = lcftUndefined;
}

// (r.gonet 09/21/2011) - PLID 45581 - Copy constructs an empty field from some other field. This is a useless move left for consistency.
CLabCustomEmptyField::CLabCustomEmptyField(const CLabCustomField &other)
: CLabCustomField(other)
{
	m_lcftType = lcftUndefined; // CLabCustomField will set this for us to the same value, but for symmetry set it here too
}

// (r.gonet 09/21/2011) - PLID 45581 - Empty virtual destructor.
CLabCustomEmptyField::~CLabCustomEmptyField()
{
}

// (r.gonet 09/21/2011) - PLID 45581 - Empty field creates no controls.
void CLabCustomEmptyField::CreateControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance/*= NULL*/)
{
	// Don't do anything
}

// Empty fields take and store no values.
bool CLabCustomEmptyField::SetValueFromControl(CLabCustomFieldControlManager::CFieldCell *pFieldCell, CString &strErrorString, CLabCustomFieldInstance *pInstance/*= NULL*/)
{
	// Success by default
	return true;
}

// (r.gonet 09/21/2011) - PLID 45581 - To prevent callers from trying to save an empty field, we simply don't let them.
void CLabCustomEmptyField::Save(CParamSqlBatch &sqlBatch)
{
	// It is an error to attempt to save an empty field
	ASSERT(FALSE);
	ThrowNxException("CLabCustomEmptyField::Save : Saving for empty fields is not implemented.");
}

////////////////////////////////////////////////////////////////////////////////
// Boolean Lab Custom Field
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45581 - Creates a new boolean lab custom field. Represented by a checkbox. Stores true/false values.
CLabCustomBooleanField::CLabCustomBooleanField()
: CLabCustomField()
{
}

// (r.gonet 09/21/2011) - PLID 45581 - Copies an existing lab custom field as a boolean custom field.
// - other is another lab custom field. It need not be a boolean custom field.
CLabCustomBooleanField::CLabCustomBooleanField(const CLabCustomField &other)
: CLabCustomField(other)
{
	m_lcftType = lcftBool;
	if(other.GetFieldType() != this->GetFieldType()) {
		m_varDefaultValue.Clear();
		m_varDefaultValue = g_cvarNull;
	}
}

// (r.gonet 09/21/2011) - PLID 45581 - Empty virtual destructor.
CLabCustomBooleanField::~CLabCustomBooleanField()
{
}

// (r.gonet 09/21/2011) - PLID 45581 - Creates the controls necessary to represent this boolean field on the screen. Creates header labels and a checkbox. The control and the field are only
//  synced when Saved in CLabCustomFieldControlManager.
// - cmControlManager is the control manager that is storing all of the controls for the present view of the fields.
// - pFieldInstance is the field value instance derived from this field which is currently being worked on. 
//    pFieldInstance can be null when the field itself is being edited and not its value, in which case the default value is loaded and displayed.
void CLabCustomBooleanField::CreateControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance/*= NULL*/)
{
	// (r.gonet 09/21/2011) - PLID 45555 - Begin a cell
	cmControlManager.BeginField(shared_from_this(), pFieldInstance, false);
	CreateHeaderControls(cmControlManager, pFieldInstance);
	NxButton *pCheckbox = new NxButton();
	BOOL bSuccess = pCheckbox->Create("", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_EX_TRANSPARENT | WS_TABSTOP | WS_GROUP, 
		CRect(cmControlManager.GetBoundingBoxBottomLeft() + CPoint(10, 5), CSize(10, 10)),
		cmControlManager.GetParentWindow(), cmControlManager.GetNewControlID());
	if(!bSuccess) {
		ThrowNxException("CLabCustomBooleanField : Attempt to create a custom boolean field failed.");
	}
	cmControlManager.SetIdealSize(pCheckbox);
	if(pFieldInstance != NULL && pFieldInstance->GetValue().vt == VT_BOOL) {
		pCheckbox->SetCheck(VarBool(pFieldInstance->GetValue()) != FALSE);
	} else if((pFieldInstance == NULL || pFieldInstance->GetModificationStatus() == CLabCustomFieldInstance::emsNew) && 
		this->GetDefaultValue().vt == VT_BOOL) {
		pCheckbox->SetCheck(VarBool(this->GetDefaultValue()) != FALSE);
	}
	// (r.gonet 09/21/2011) - PLID 45555
	cmControlManager.AddInputControl(pCheckbox);

	pCheckbox->ShowWindow(SW_SHOW);
	// (r.gonet 09/21/2011) - PLID 45555
	cmControlManager.EndField();
}

// (r.gonet 09/21/2011) - PLID 45581 - Using the control representing the input control created by CreateControls, sets the field's value, effectively syncing the screen with the in memory object.
// - pInputControl is the control containing the value to sync this field with. It may not be null and must be a valid window.
// - pInstance is the field value instance derived from this field that being worked on. If pInstance is null then the default value is set instead of the instance value.
bool CLabCustomBooleanField::SetValueFromControl(CLabCustomFieldControlManager::CFieldCell *pFieldCell, CString &strErrorString, CLabCustomFieldInstance *pInstance/*= NULL*/)
{
	ASSERT(pFieldCell);
	if(pFieldCell->GetInputControlCount() == 0) {
		return true;
	}

	CWnd *pInputControl = pFieldCell->GetInputControl(0);
	ASSERT(pInputControl != NULL);
	CWnd *pOwner = pInputControl->GetOwner();
	_variant_t varValue = pOwner->IsDlgButtonChecked(pInputControl->GetDlgCtrlID()) ? g_cvarTrue : g_cvarFalse;
	if(pInstance != NULL) {
		pInstance->SetValue(varValue);
	} else {
		this->SetDefaultValue(varValue);
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Integer Lab Custom Field
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45581 - Creates a new integer lab custom field. Represented by a textbox. Stores whole number values.
CLabCustomIntegerField::CLabCustomIntegerField()
: CLabCustomField()
{
}

// (r.gonet 09/21/2011) - PLID 45581 - Copies an existing lab custom field as an integer custom field.
// - other is an exsting lab custom field. It need not be an integer lab custom field.
CLabCustomIntegerField::CLabCustomIntegerField(const CLabCustomField &other)
: CLabCustomField(other)
{
	m_lcftType = lcftInteger;
	if(other.GetFieldType() != lcftFloat && other.GetFieldType() != this->GetFieldType()) {
		m_varDefaultValue.Clear();
		m_varDefaultValue = g_cvarNull;
	}

	// We can convert from floats
	if(other.GetFieldType() == lcftFloat) {
		m_varDefaultValue = _variant_t((long)AsDouble(other.GetDefaultValue()));
	}
}

// (r.gonet 09/21/2011) - PLID 45581 - Empty virtual destructor
CLabCustomIntegerField::~CLabCustomIntegerField()
{
}

// (r.gonet 09/21/2011) - PLID 45581 - Creates the controls necessary to represent this integer field on the screen. Creates header labels and a textbox. The control and the field are only
//  synced when Saved in CLabCustomFieldControlManager.
// - cmControlManager is the control manager that is storing all of the controls for the present view of the fields.
// - pFieldInstance is the field value instance derived from this field which is currently being worked on. 
//    pFieldInstance can be null when the field itself is being edited and not its value, in which case the default value is loaded and displayed.
void CLabCustomIntegerField::CreateControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance/*= NULL*/)
{	
	// (r.gonet 09/21/2011) - PLID 45555 - Begin a cell
	cmControlManager.BeginField(shared_from_this(), pFieldInstance, this->GetSizeMode() == cfsmAutoSize);
	CreateHeaderControls(cmControlManager, pFieldInstance); // Make the field's labels

	// Calculate the size of the input box
	CRect rcIntBox;
	switch(m_cfsmSizeMode) {
		case cfsmFixedSize:
			rcIntBox = CRect(cmControlManager.GetBoundingBoxBottomLeft() + CPoint(10, 5), CSize(VarLong(m_varWidth, cmControlManager.GetDrawingSize().cx - 20), VarLong(m_varHeight, 20)));
			break;
		case cfsmAutoSize:
		default:		
			rcIntBox = CRect(cmControlManager.GetBoundingBoxBottomLeft() + CPoint(10, 5), CSize(cmControlManager.GetDrawingSize().cx - 20, 20));
			break;
	}

	// Create the input control
	CNxEdit *pNxEdit = new CNxEdit();
	int success = pNxEdit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | WS_GROUP, 
		rcIntBox, 
		cmControlManager.GetParentWindow(), cmControlManager.GetNewControlID());
	if(!success) {
		ThrowNxException("CLabCustomIntegerField : Attempt to create a custom integer field failed.");
	}
	pNxEdit->LimitText(11); // The minimum number of characters necessary to hold LONG_MIN
	// Set its value as our default or the instance's stored value.
	if(pFieldInstance != NULL && pFieldInstance->GetValue().vt == VT_I4) {
		CString strValue = FormatString("%li", VarLong(pFieldInstance->GetValue()));
		pNxEdit->SetWindowText(_bstr_t(strValue));
	} else if((pFieldInstance == NULL || pFieldInstance->GetModificationStatus() == CLabCustomFieldInstance::emsNew) && 
		this->GetDefaultValue().vt == VT_I4) {
		CString strValue = FormatString("%li", VarLong(m_varDefaultValue));
		pNxEdit->SetWindowText(_bstr_t(strValue));
	}
	// (r.gonet 09/21/2011) - PLID 45555 - Add it to management
	cmControlManager.AddInputControl(pNxEdit);
	cmControlManager.EndField();
}

// (r.gonet 09/21/2011) - PLID 45581 - Using the control representing the input control created by CreateControls, sets the field's value, effectively syncing the screen with the in memory object.
// - pInputControl is the control containing the value to sync this field with. It may not be null and must be a valid window.
// - pInstance is the field value instance derived from this field that being worked on. If pInstance is null then the default value is set instead of the instance value.
bool CLabCustomIntegerField::SetValueFromControl(CLabCustomFieldControlManager::CFieldCell *pFieldCell, CString &strErrorString, CLabCustomFieldInstance *pInstance/*= NULL*/)
{
	ASSERT(pFieldCell);
	if(pFieldCell->GetInputControlCount() == 0) {
		return true;
	}

	CWnd *pInputControl = pFieldCell->GetInputControl(0);
	CString strValue;
	pInputControl->GetWindowText(strValue);
	
	// What if they entered in bad characters though?
	char *szBadString;
	ASSERT(strlen(strValue) < 4096);
	strtol(strValue, &szBadString, 10);
	if(strValue.GetLength() > 0 && strlen(szBadString) > 0) {
		strErrorString.Format("%s must be a whole number.", m_strFriendlyName);
		return false;
	}


	// To see if the user entered a value that exceeded our ability to store it, we must go a step beyond to long long.
	if(strValue.GetLength() > 0) {
		std::stringstream sstr((LPCTSTR)strValue);
		__int64 nnTest;
		sstr >> nnTest;
		if(nnTest > LONG_MAX) {
			strErrorString.Format("%s contains too big of a number. The largest number allowed is %li.", m_strFriendlyName, LONG_MAX);
			return false;
		} else if(nnTest < LONG_MIN) {
			strErrorString.Format("%s contains too small of a number. The smallest number allowed is %li.", m_strFriendlyName, LONG_MIN);
			return false;
		}
	}

	_variant_t varValue = g_cvarNull;
	if(strValue.Trim().GetLength() > 0) {
		long nValue = atol(strValue);
		varValue = _variant_t(nValue, VT_I4);
	}
	if(pInstance != NULL) {
		pInstance->SetValue(varValue);
	} else {
		this->SetDefaultValue(varValue);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Float Lab Custom Field
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45581 - Creates a new float lab custom field. Represented by a textbox. Stores number values with double precision.
CLabCustomFloatField::CLabCustomFloatField()
: CLabCustomField()
{
}

// (r.gonet 09/21/2011) - PLID 45581 - Copies an existing lab custom field as a float custom field.
// - other is an exsting lab custom field. It need not be an float lab custom field.
CLabCustomFloatField::CLabCustomFloatField(const CLabCustomField &other)
: CLabCustomField(other)
{
	m_lcftType = lcftFloat;
	if(other.GetFieldType() != lcftInteger && other.GetFieldType() != this->GetFieldType()) {
		m_varDefaultValue.Clear();
		m_varDefaultValue = g_cvarNull;
	}

	// We can convert from ints
	if(other.GetFieldType() == lcftInteger) {
		m_varDefaultValue = _variant_t((double)AsLong(other.GetDefaultValue()));
	}
}

// (r.gonet 09/21/2011) - PLID 45581 - Empty virtual destructor
CLabCustomFloatField::~CLabCustomFloatField()
{
}

// (r.gonet 09/21/2011) - PLID 45581 - Creates the controls necessary to represent this float field on the screen. Creates header labels and a textbox. The control and the field are only
//  synced when Saved in CLabCustomFieldControlManager.
// - cmControlManager is the control manager that is storing all of the controls for the present view of the fields.
// - pFieldInstance is the field value instance derived from this field which is currently being worked on. 
//    pFieldInstance can be null when the field itself is being edited and not its value, in which case the default value is loaded and displayed.
void CLabCustomFloatField::CreateControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance/*= NULL*/)
{
	// (r.gonet 09/21/2011) - PLID 45555 - Begin a cell
	cmControlManager.BeginField(shared_from_this(), pFieldInstance, this->GetSizeMode() == cfsmAutoSize);
	CreateHeaderControls(cmControlManager, pFieldInstance); // Add our labels

	// Calculate the size of the input control
	CRect rcFloatBox;
	switch(m_cfsmSizeMode) {
		case cfsmFixedSize:
			rcFloatBox = CRect(cmControlManager.GetBoundingBoxBottomLeft() + CPoint(10, 5), CSize(VarLong(m_varWidth, cmControlManager.GetDrawingSize().cx - 20), VarLong(m_varHeight, 20)));
			break;
		case cfsmAutoSize:
		default:		
			rcFloatBox = CRect(cmControlManager.GetBoundingBoxBottomLeft() + CPoint(10, 5), CSize(cmControlManager.GetDrawingSize().cx - 20, 20));
			break;
	}

	// Create the input control
	CNxEdit *pNxEdit = new CNxEdit();
	BOOL bSuccess = pNxEdit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | WS_GROUP, 
		rcFloatBox, 
		cmControlManager.GetParentWindow(), cmControlManager.GetNewControlID());
	if(!bSuccess) {
		ThrowNxException("CLabCustomFloatField : Attempt to create a custom float field failed.");
	}
	pNxEdit->LimitText(15);
	// Set the input control's value to our default or its stored instance value.
	if(pFieldInstance != NULL && pFieldInstance->GetValue().vt == VT_R8) {
		CString strValue = FormatString("%.6f", VarDouble(pFieldInstance->GetValue()));
		pNxEdit->SetWindowText(_bstr_t(strValue));
	} else if((pFieldInstance == NULL || pFieldInstance->GetModificationStatus() == CLabCustomFieldInstance::emsNew) && 
		this->GetDefaultValue().vt == VT_R8) {
		CString strValue = FormatString("%.6f",  VarDouble(m_varDefaultValue));
		pNxEdit->SetWindowText(_bstr_t(strValue));
	}
	// (r.gonet 09/21/2011) - PLID 45555 - Add control to management
	cmControlManager.AddInputControl(pNxEdit);
	cmControlManager.EndField();
}

// (r.gonet 09/21/2011) - PLID 45581 - Using the control representing the input control created by CreateControls, sets the field's value, effectively syncing the screen with the in memory object.
// - pInputControl is the control containing the value to sync this field with. It may not be null and must be a valid window.
// - pInstance is the field value instance derived from this field that being worked on. If pInstance is null then the default value is set instead of the instance value.
bool CLabCustomFloatField::SetValueFromControl(CLabCustomFieldControlManager::CFieldCell *pFieldCell, CString &strErrorString, CLabCustomFieldInstance *pInstance/*= NULL*/)
{
	ASSERT(pFieldCell);
	if(pFieldCell->GetInputControlCount() == 0) {
		return true;
	}

	CWnd *pInputControl = pFieldCell->GetInputControl(0);
	CString strValue;
	pInputControl->GetWindowText(strValue);

	if(strValue.GetLength() > 0 && !IsValidDouble(strValue)) {
		strErrorString.Format("%s must be a real number.", m_strFriendlyName);
		return false;
	}

	_variant_t varValue = g_cvarNull;
	// If the value the user entered is blank, then we assume they entered nothing. Don't convert to 0.
	if(strValue.Trim().GetLength() != 0) {
		double dValue = atof(strValue);
		varValue = _variant_t(dValue, VT_R8);
	}
	
	if(pInstance != NULL) {
		pInstance->SetValue(varValue);
	} else {
		this->SetDefaultValue(varValue);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Text Lab Custom Field
////////////////////////////////////////////////////////////////////////////////

// Creates a new text lab custom field. Represented by a textbox, single line or multiline. Stores textual values.
CLabCustomTextField::CLabCustomTextField(ELabCustomFieldType lcftType/*= lcftText*/)
: CLabCustomField()
{
	// Text fields use a max character length. Init it here to 100 chars max.
	m_varMaxLength = _variant_t((long)100, VT_I4);
	if(lcftType == lcftText || lcftType == lcftMemo) {
		m_lcftType = lcftType;
	}
}

// (r.gonet 09/21/2011) - PLID 45582 - Copies an existing lab custom field as a text custom field. Tries to retain the old value as the new text value.
// - other is an exsting lab custom field. It need not be an text lab custom field.
CLabCustomTextField::CLabCustomTextField(const CLabCustomField &other, ELabCustomFieldType lcftType/*= lcftText*/)
: CLabCustomField(other)
{
	m_lcftType = lcftText;
	if(other.GetFieldType() == lcftSingleSelectList) {
		m_varDefaultValue.Clear();
		m_varDefaultValue = g_cvarNull;
	}

	// We may be able to use a couple special fields from the other field
	if(other.GetFieldType() != lcftText && other.GetFieldType() != lcftMemo) {
		m_varMaxLength = _variant_t((long)100, VT_I4);
	}

	// We can convert from anything except lists
	if(other.GetFieldType() != lcftSingleSelectList) {
		m_varDefaultValue = _variant_t(AsString(other.GetDefaultValue()));
	}

	if(lcftType == lcftText || lcftType == lcftMemo) {
		m_lcftType = lcftType;
	}
}

// (r.gonet 09/21/2011) - PLID 45582 - Empty virtual destructor
CLabCustomTextField::~CLabCustomTextField()
{
}

// (r.gonet 09/21/2011) - PLID 45582 - Gets the maximum number of characters allowed in this text field.
long CLabCustomTextField::GetMaxLength() const
{
	if(m_varMaxLength.vt == VT_I4) {
		return VarLong(m_varMaxLength, 100);
	} else {
		return 100;
	}
}

// (r.gonet 09/21/2011) - PLID 45582 - Sets the maximum number of characters allowed in this text field. 
// - nMaxLength is the maximum number of characters to allow entered. <= 255 has this field store as a nvarchar. > 255 has this field store as a ntext.
void CLabCustomTextField::SetMaxLength(long nMaxLength)
{
	m_varMaxLength = _variant_t(nMaxLength);
	// But we may have gone over the limit here. 255 is the cutoff for text and the start of memos.
	if(nMaxLength <= 255) {
		m_lcftType = lcftText;
	} else {
		m_lcftType = lcftMemo;
	}
	SetModificationStatus(emsModified);
}

// (r.gonet 09/21/2011) - PLID 45582 - Creates the controls necessary to represent this text field on the screen. Creates header labels and a textbox. The control and the field are only
//  synced when Saved in CLabCustomFieldControlManager.
// - cmControlManager is the control manager that is storing all of the controls for the present view of the fields.
// - pFieldInstance is the field value instance derived from this field which is currently being worked on. 
//    pFieldInstance can be null when the field itself is being edited and not its value, in which case the default value is loaded and displayed.
void CLabCustomTextField::CreateControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance/*= NULL*/)
{
	cmControlManager.BeginField(shared_from_this(), pFieldInstance, this->GetSizeMode() == cfsmAutoSize);
	CreateHeaderControls(cmControlManager, pFieldInstance); // Create the field labels
	
	// We may take two different routes here depending on the display type of this field. We can draw a single line text box or a multi line one that accepts returns.
	CNxEdit *pNxEdit = new CNxEdit();
	if(m_lcdtDisplayType == lcdtDefault || m_lcdtDisplayType == lcdtSingleLineEdit) {
		// Draw the single line version.
		// Calculate the textbox size
		CRect rcTextBox;
		switch(m_cfsmSizeMode) {
			case cfsmFixedSize:
				rcTextBox = CRect(cmControlManager.GetBoundingBoxBottomLeft() + CPoint(10, 5), CSize(VarLong(m_varWidth, cmControlManager.GetDrawingSize().cx - 20), VarLong(m_varHeight, 20)));
				break;
			case cfsmAutoSize:
			default:		
				rcTextBox = CRect(cmControlManager.GetBoundingBoxBottomLeft() + CPoint(10, 5), CSize(cmControlManager.GetDrawingSize().cx - 20, 20));
				break;
		}
		// Create the textbox
		BOOL bSuccess = pNxEdit->Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP | WS_GROUP, 
			rcTextBox,
			cmControlManager.GetParentWindow(), cmControlManager.GetNewControlID());
		if(!bSuccess) {
			ThrowNxException("CLabCustomTextField : Attempt to create a custom single line text field failed.");
		}
	} else if(m_lcdtDisplayType == lcdtMultiLineEdit) {
		CRect rcTextBox;
		switch(m_cfsmSizeMode) {
			case cfsmFixedSize:
				rcTextBox = CRect(cmControlManager.GetBoundingBoxBottomLeft() + CPoint(10, 5), CSize(VarLong(m_varWidth, cmControlManager.GetDrawingSize().cx - 20), VarLong(m_varHeight, 60)));
				break;
			case cfsmAutoSize:
			default:		
				rcTextBox = CRect(cmControlManager.GetBoundingBoxBottomLeft() + CPoint(10, 5), CSize(cmControlManager.GetDrawingSize().cx - 20, 60));
				break;
		}
		// Create the textbox
		BOOL bSuccess = pNxEdit->Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL | WS_BORDER | WS_TABSTOP | WS_GROUP, 
			rcTextBox,
			cmControlManager.GetParentWindow(), cmControlManager.GetNewControlID());
		if(!bSuccess) {
			ThrowNxException("CLabCustomTextField : Attempt to create a custom multi line text field failed.");
		}
	} else {
		ThrowNxException("CLabCustomTextField::CreateControls : Unhandled display type.");
	}

	// Set the maximum character limit.
	pNxEdit->SetLimitText(this->GetMaxLength());

	// Set the value we have as a default or stored in the instance value. Note that the maximum character size does not cause truncation here, just in case it would have caused data loss.
	if(pFieldInstance != NULL && pFieldInstance->GetValue().vt == VT_BSTR) {
		CString strText = VarString(pFieldInstance->GetValue());
		pNxEdit->SetWindowText(strText);
	} else if((pFieldInstance == NULL || pFieldInstance->GetModificationStatus() == CLabCustomFieldInstance::emsNew) && 
		this->GetDefaultValue().vt == VT_BSTR) {
		CString strText = VarString(m_varDefaultValue);
		pNxEdit->SetWindowText(strText);
	}

	// (r.gonet 09/21/2011) - PLID 45555 - Add the control to management.
	cmControlManager.AddInputControl(pNxEdit);
	pNxEdit->ShowWindow(SW_SHOW);
	cmControlManager.EndField();
}

// (r.gonet 09/21/2011) - PLID 45582 - Using the control representing the input control created by CreateControls, sets the field's value, effectively syncing the screen with the in memory object.
// - pInputControl is the control containing the value to sync this field with. It may not be null and must be a valid window.
// - pInstance is the field value instance derived from this field that being worked on. If pInstance is null then the default value is set instead of the instance value.
bool CLabCustomTextField::SetValueFromControl(CLabCustomFieldControlManager::CFieldCell *pFieldCell, CString &strErrorString, CLabCustomFieldInstance *pInstance/*= NULL*/)
{
	ASSERT(pFieldCell);
	if(pFieldCell->GetInputControlCount() == 0) {
		return true;
	}

	CWnd *pInputControl = pFieldCell->GetInputControl(0);
	CString strValue;
	pInputControl->GetWindowText(strValue);
	_variant_t varValue = g_cvarNull;
	// We consider text of 0 length to be not filled.
	if(strValue.GetLength() > 0) {
		varValue = _variant_t(strValue);
	}
	if(pInstance != NULL) {
		pInstance->SetValue(varValue);
	} else {
		this->SetDefaultValue(varValue);
	}

	return true;
}

// (r.gonet 09/21/2011) - PLID 45582 - Commits the text field to the database. Checks if it should save as a text or memo field and then calls parent's saving method.
// - sqlBatch is the batch to write the save statement to.
void CLabCustomTextField::Save(CParamSqlBatch &sqlBatch)
{
	// Text fields require some special handling to be able to store large values
	if(m_lcftType == lcftText && VarLong(m_varMaxLength, 0) > 255) {
		m_lcftType = lcftMemo;
	} else if(m_lcftType == lcftMemo && VarLong(m_varMaxLength, 0) <= 255) {
		m_lcftType = lcftText;
	}

	CLabCustomField::Save(sqlBatch);
}

////////////////////////////////////////////////////////////////////////////////
// Date/Time Lab Custom Field
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45581 - Creates a new text date/time custom field. Represented by an NxTime control. Stores dates, times, or datetime values.
CLabCustomDateTimeField::CLabCustomDateTimeField()
{
}

// (r.gonet 09/21/2011) - PLID 45581 - Copies an existing lab custom field as a datetime custom field.
// - other is an exsting lab custom field. It need not be an datetime lab custom field.
CLabCustomDateTimeField::CLabCustomDateTimeField(const CLabCustomField &other)
: CLabCustomField(other)
{
	m_lcftType = lcftDateTime;
	if(other.GetFieldType() != this->GetFieldType()) {
		m_varDefaultValue.Clear();
		m_varDefaultValue = g_cvarNull;
	}

}

// (r.gonet 09/21/2011) - PLID 45581 - Empty virtual destructor
CLabCustomDateTimeField::~CLabCustomDateTimeField()
{
}

// (r.gonet 09/21/2011) - PLID 45581 - Creates the controls necessary to represent this text field on the screen. Creates header labels and a textbox. The control and the field are only
//  synced when Saved in CLabCustomFieldControlManager.
// - cmControlManager is the control manager that is storing all of the controls for the present view of the fields.
// - pFieldInstance is the field value instance derived from this field which is currently being worked on. 
//    pFieldInstance can be null when the field itself is being edited and not its value, in which case the default value is loaded and displayed.
void CLabCustomDateTimeField::CreateControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance/*= NULL*/)
{
	// (r.gonet 09/21/2011) - PLID 45555 - Begin a cell
	cmControlManager.BeginField(shared_from_this(), pFieldInstance, this->GetSizeMode() == cfsmAutoSize);
	CreateHeaderControls(cmControlManager, pFieldInstance); // Add our field labels.

	// Calculate the size of the datetime control.
	CRect rcDateTimeBox;
	switch(m_cfsmSizeMode) {
		case cfsmFixedSize:
			rcDateTimeBox = CRect(cmControlManager.GetBoundingBoxBottomLeft() + CPoint(10, 5), CSize(VarLong(m_varWidth, cmControlManager.GetDrawingSize().cx - 20), VarLong(m_varHeight, 20)));
			break;
		case cfsmAutoSize:
		default:		
			rcDateTimeBox = CRect(cmControlManager.GetBoundingBoxBottomLeft() + CPoint(10, 5), CSize(cmControlManager.GetDrawingSize().cx - 20, 20));
			break;
	}

	// Create the datetime control
	CWnd *pWnd = new CWnd();
	BOOL bSuccess = pWnd->CreateControl(_T("NXTIME.NxTimeCtrl.1"), "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | WS_GROUP,
		rcDateTimeBox,
		cmControlManager.GetParentWindow(), cmControlManager.GetNewControlID());
	if(!bSuccess) {
		ThrowNxException("CLabCustomDateTimeField::CreateControls() : Attempt to create a custom date time field failed.");
	}

	NXTIMELib::_DNxTimePtr pNxTime = pWnd->GetControlUnknown();
	if(!pNxTime) {
		ThrowNxException("CLabCustomDateTimeField::CreateControls() : Attempt to create a custom date time field failed.");
	}

	pNxTime->Enabled = true;
	// According to the display type, we can either be just date, just time, or date and time. Date and time is the default.
	if(m_lcdtDisplayType == lcdtDefault || m_lcdtDisplayType == lcdtDateAndTime) {
		pNxTime->ReturnType = 2;
	} else if(m_lcdtDisplayType == lcdtDate) {
		pNxTime->ReturnType = 0;
	} else if(m_lcdtDisplayType == lcdtTime) {
		pNxTime->ReturnType = 1;
	}

	// Fill our input control with the field's default value or the instance's stored value.
	if(pFieldInstance != NULL && pFieldInstance->GetValue().vt == VT_DATE) {
		pNxTime->SetDateTime(VarDateTime(pFieldInstance->GetValue()));
	} else if((pFieldInstance == NULL || pFieldInstance->GetModificationStatus() == CLabCustomFieldInstance::emsNew) && 
		m_varDefaultValue.vt == VT_DATE) {
		pNxTime->SetDateTime(VarDateTime(m_varDefaultValue));
	}

	// (r.gonet 09/21/2011) - PLID 45555 - Add the input control to management.
	cmControlManager.AddInputControl(pWnd);
	cmControlManager.EndField();
}

// (r.gonet 09/21/2011) - PLID 45581 - Using the control representing the input control created by CreateControls, sets the field's value, effectively syncing the screen with the in memory object.
// - pInputControl is the control containing the value to sync this field with. It may not be null and must be a valid window.
// - pInstance is the field value instance derived from this field that being worked on. If pInstance is null then the default value is set instead of the instance value.
bool CLabCustomDateTimeField::SetValueFromControl(CLabCustomFieldControlManager::CFieldCell *pFieldCell, CString &strErrorString, CLabCustomFieldInstance *pInstance/*= NULL*/)
{
	ASSERT(pFieldCell);
	if(pFieldCell->GetInputControlCount() == 0) {
		return true;
	}

	CWnd *pInputControl = pFieldCell->GetInputControl(0);
	NXTIMELib::_DNxTimePtr pNxTime = pInputControl->GetControlUnknown();
	if(!pNxTime) {
		ThrowNxException("CLabCustomDateTimeField::SetValueFromControls : Control is of an unexpected type.");
	}

	_variant_t varDateTimeValue = g_cvarNull;
	if(pNxTime->GetStatus() == 1) { // Is the datetime in the control valid?
		COleDateTime dtValue = pNxTime->GetDateTime();		

		// (r.gonet 12/07/2011) - PLID 45581 - Yes, somebody actually put in something back in 1300, 
		//  so now we check the range of dates that can be saved to SQL.
		if(dtValue.GetYear() < 1753) {
			strErrorString.Format("%s has a date that is too far in the past. The oldest possible date that can be used is 01-01-1753.", m_strFriendlyName);
			return false;
		} else if(dtValue.GetYear() > 9999) {
			// Is this even possible?
			strErrorString.Format("%s has a date that is too far in the future. The maximum possible date that can be used is 12-31-9999.", m_strFriendlyName);
			return false;
		}

		varDateTimeValue = _variant_t(dtValue, VT_DATE);
	} else if(pNxTime->GetStatus() != 3) {
		// We can be displayed alternatively in three different formats, make sure we report on the right one.
		CString strDisplayType = "date and time";
		if(m_lcdtDisplayType == lcdtDefault || m_lcdtDisplayType == lcdtDateAndTime) {
			strDisplayType = "date and time";
		} else if(m_lcdtDisplayType == lcdtDate) {
			strDisplayType = "date";
		} else if(m_lcdtDisplayType == lcdtTime) {
			strDisplayType = "time";
		}
		strErrorString.Format("%s must be a valid %s.", m_strFriendlyName, strDisplayType);
		return false;
	}

	if(pInstance != NULL) {
		pInstance->SetValue(varDateTimeValue);
	} else {
		this->SetDefaultValue(varDateTimeValue);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// List Lab Custom Field Item
////////////////////////////////////////////////////////////////////////////////

// Creates a new list field item, which is an element within the list. It can be selected and holds an database ID, value, and descrption.
// The default modification status here is New.
CLabCustomListFieldItem::CLabCustomListFieldItem()
{
	m_nID = -1;
	// Initially, we have no type. It will have to be defined.
	m_lcftType = lcftUndefined;
	m_strDescription = "";
	m_nSortOrder = 0;
	m_varValue = g_cvarNull;
	m_emsModificationStatus = emsNew;
	// (r.gonet 09/21/2011) - PLID 45558 - Naturally, a new list item is not deleted.
	m_bDeleted = false;
	// (r.gonet 09/21/2011) - PLID 45557 - Initially, create a new GUID in case we are indeed new and don't get loaded later.
	m_strGUID = NewUUID();
}

// Creates an existing list field item, one that presumably has already been save to the database.
// - nID is the database ID of the existing list item.
// - varValue is the internal value of the list item.
// - lcftType is the type of value this list item holds.
// - strDescription is the description of this item and what will be displayed to the user for this item in a dropdown row.
// - nSortOrder is the order in which this item appears in its parent list field.
CLabCustomListFieldItem::CLabCustomListFieldItem(long nID, _variant_t varValue, ELabCustomFieldType lcftType, CString strDescription, long nSortOrder)
{
	m_nID = nID;
	m_varValue = varValue;
	m_lcftType = lcftType;
	m_strDescription = strDescription;
	m_nSortOrder = nSortOrder;
	m_emsModificationStatus = emsNone;
	// (r.gonet 09/21/2011) - PLID 45558 - By default a new list item is not deleted. It will have to be set as such later if desired.
	m_bDeleted = false;
	// (r.gonet 09/21/2011) - PLID 45557 - Init the field's GUID
	m_strGUID = NewUUID();
}

// (r.gonet 09/21/2011) - PLID 45583 - Copy constructs from a second list item.
// - other is another list item.
CLabCustomListFieldItem::CLabCustomListFieldItem(CLabCustomListFieldItem &other)
{
	CloneFrom(other);
}

// (r.gonet 09/21/2011) - PLID 45583 - Callable copy constructor-like method.
// - other is another list item to copy from.
void CLabCustomListFieldItem::CloneFrom(CLabCustomListFieldItem &other)
{
	m_nID = other.m_nID;
	m_varValue = other.m_varValue;
	m_lcftType = other.m_lcftType;
	m_strDescription = other.m_strDescription;
	m_nSortOrder = other.m_nSortOrder;
	m_emsModificationStatus = other.m_emsModificationStatus;
	// (r.gonet 09/21/2011) - PLID 45558 - Should this item be deleted as well?
	m_bDeleted = other.m_bDeleted;
	// (r.gonet 09/21/2011) - PLID 45557 - Copy the other field's GUID.
	m_strGUID = other.m_strGUID;
}

// (r.gonet 09/21/2011) - PLID 45583 - Saves the list item to the database. Will save according to what the item's modification status is and then resets that status.
//  Note that new items are saved immediately and are not added to the batch.
// - sqlBatch is a batch object that this item will be saved to. This batch must be executed in order for the item's changes to be committed.
// - nFieldID is the parent list field's database ID. It must be the ID of an existing field.
void CLabCustomListFieldItem::Save(CParamSqlBatch &sqlBatch, long nFieldID)
{
	ASSERT(nFieldID > 0);
	if(m_emsModificationStatus == emsNew) {
		// (r.gonet 09/21/2011) - PLID 45557 - Also save the item's GUID that we created
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON; "
			"INSERT INTO LabCustomFieldListItemsT (FieldID, Description, DataTypeID, "
			"	IntParam, FloatParam, TextParam, MemoParam, DateTimeParam, "
			"	SortOrder, UID) "
			"VALUES "
			"({INT}, {STRING}, {INT}, "
			"	{VT_I4}, {VT_R8}, {VT_BSTR}, {VT_BSTR}, {VT_DATE}, "
			"	{INT}, {STRING}); "
			"SET NOCOUNT OFF; "
			""
			"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS NewID; ",
			nFieldID, m_strDescription, (int)m_lcftType, 
			(m_lcftType == lcftInteger ? m_varValue : (m_lcftType == lcftBool ? _variant_t(AsLong(m_varValue)) : g_cvarNull)),
			(m_lcftType == lcftFloat ? m_varValue : g_cvarNull),
			(m_lcftType == lcftText ? m_varValue : g_cvarNull),
			(m_lcftType == lcftMemo ? m_varValue : g_cvarNull),
			(m_lcftType == lcftDateTime && m_varValue.vt == VT_DATE ? m_varValue : g_cvarNull),
			m_nSortOrder, m_strGUID);
		if(!prs->eof) {
			m_nID = VarLong(prs->Fields->Item["NewID"]->Value);
		} else {
			prs->Close();
			ThrowNxException("CLabCustomListFieldItem::Save : Field creation query failed to return records.");
		}
		prs->Close();
	} else if(m_emsModificationStatus == emsModified) {
		sqlBatch.Add(
		"UPDATE LabCustomFieldListItemsT SET "
		"	Description = {STRING}, "
		"	DataTypeID = {INT}, "
		"	IntParam = {VT_I4}, "
		"	FloatParam = {VT_R8}, "
		"	TextParam = {VT_BSTR}, "
		"	DateTimeParam = {VT_DATE}, "
		"	SortOrder = {INT} "
		"WHERE LabCustomFieldListItemsT.ID = {INT}; ",
		m_strDescription, (int)m_lcftType,
		(m_lcftType == lcftInteger ? m_varValue : (m_lcftType == lcftBool ? _variant_t(AsLong(m_varValue)) : g_cvarNull)),
		(m_lcftType == lcftFloat ? m_varValue : g_cvarNull),
		(m_lcftType == lcftText ? m_varValue : g_cvarNull),
		(m_lcftType == lcftDateTime && m_varValue.vt == VT_DATE ? m_varValue : g_cvarNull),
		m_nSortOrder,
		m_nID);
	} else if(m_emsModificationStatus == emsDeleted) {
		sqlBatch.Add(
			"UPDATE LabCustomFieldListItemsT "
			"SET Deleted = 1 "
			"WHERE ID = {INT}; ",
			m_nID);
		// (r.gonet 09/21/2011) - PLID 45558 - Ok, now the item has been marked deleted in the database. Carry that status back to this object.
		m_bDeleted = true;
	}
	m_emsModificationStatus = emsNone;
}

// (r.gonet 09/21/2011) - PLID 45583 - Static method. Returns true if this item is in use (ie selected) somewhere in the database. False otherwise.
// - The item database ID to check.
bool CLabCustomListFieldItem::IsInUse(long nListItemID)
{
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT COUNT(*) AS InUseCount "
		"FROM LabCustomFieldInstancesT "
		"WHERE SelectionID = {INT}; ",
		nListItemID);
	long nInUseCount = VarLong(prs->Fields->Item["InUseCount"]->Value);
	prs->Close();
	return nInUseCount > 0;
}

// Sets the modification status of the item. 
// - The new modification status.
void CLabCustomListFieldItem::SetModificationStatus(CLabCustomListFieldItem::EModificationStatus emsModificationStatus)
{
	m_emsModificationStatus = emsModificationStatus;
}

// (r.gonet 09/21/2011) - PLID 45583 - Gets the modification status of this item.
CLabCustomListFieldItem::EModificationStatus CLabCustomListFieldItem::GetModificationStatus() const
{
	return m_emsModificationStatus;
}

// (r.gonet 09/21/2011) - PLID 45583 - Gets the database ID of this list item.
long CLabCustomListFieldItem::GetID() const
{
	return m_nID;
}

// Gets the internal value of this list item.
_variant_t CLabCustomListFieldItem::GetValue() const
{
	return m_varValue;
}

// (r.gonet 09/21/2011) - PLID 45583 - Sets the internal value of this list item.
// - varValue is the new internal value. It should correspond to the list item's type.
void CLabCustomListFieldItem::SetValue(_variant_t varValue)
{
	if(m_emsModificationStatus == emsNone) {
		m_emsModificationStatus = emsModified;
	}
	m_varValue = varValue;
}

// (r.gonet 09/21/2011) - PLID 45583 - Gets the type of this item. 
ELabCustomFieldType CLabCustomListFieldItem::GetType() const
{
	return m_lcftType;
}

// (r.gonet 09/21/2011) - PLID 45583 - Sets the item type. Recursive lists are not allowed.
// - lcftType is the new type of the list item. May not be lcftSingleSelectList.
void CLabCustomListFieldItem::SetType(ELabCustomFieldType lcftType)
{
	if(lcftType == lcftSingleSelectList) {
		ThrowNxException("CLabCustomListFieldItem::SetType : Attempted to set the item type to list. Recursive lists are not allowed.");
	}
	if(m_emsModificationStatus == emsNone) {
		m_emsModificationStatus = emsModified;
	}
	m_lcftType = lcftType;

}

// (r.gonet 09/21/2011) - PLID 45583 - Gets the description of the list item, what will show to the user.
CString CLabCustomListFieldItem::GetDescription() const
{
	return m_strDescription;
}

// (r.gonet 09/21/2011) - PLID 45583 - Sets the description of the list item, what will show to the user.
// - strDescription is the new field description.
void CLabCustomListFieldItem::SetDescription(CString strDescription)
{
	if(m_emsModificationStatus == emsNone) {
		m_emsModificationStatus = emsModified;
	}
	m_strDescription = strDescription;
}

// (r.gonet 09/21/2011) - PLID 45583 - Gets the order in which this item appears in the parent list field, asc. This value is guaranteed to be monitonically increasing.
long CLabCustomListFieldItem::GetSortOrder()
{
	return m_nSortOrder;
}

// (r.gonet 09/21/2011) - PLID 45583 - Sets the sort order.
// - nSortOrder is the order in which this item appears in the parent list field.
void CLabCustomListFieldItem::SetSortOrder(long nSortOrder)
{
	if(m_emsModificationStatus == emsNone) {
		m_emsModificationStatus = emsModified;
	}
	m_nSortOrder = nSortOrder;
}

// (r.gonet 09/21/2011) - PLID 45558 - Gets whether this item has been marked deleted from the database. It will only appear in lists then when it is set as the selected value.
bool CLabCustomListFieldItem::GetDeleted() const
{
	return m_bDeleted;
}

// (r.gonet 09/21/2011) - PLID 45558 - Sets this item as deleted within the database. This should be called during loading from the database. It is not an in memory operation.
// bDeleted is true if the item has been deleted. False otherwise.
void CLabCustomListFieldItem::SetDeleted(bool bDeleted)
{
	m_bDeleted = bDeleted;
}

// (r.gonet 09/21/2011) - PLID 45557 - Sets the item's GUID. Should only be used during initialization. Not saved in updates.
void CLabCustomListFieldItem::SetGUID(CString strGUID)
{
	m_strGUID = strGUID;
}

////////////////////////////////////////////////////////////////////////////////
// List Lab Custom Field
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45583 - Creates a new empty list field
CLabCustomListField::CLabCustomListField()
: CLabCustomField()
{
}

// (r.gonet 09/21/2011) - PLID 45583 - Copies another field as a list field
// - other is another field. Not necessarily a list field, but if so, this constructor copies its values.
CLabCustomListField::CLabCustomListField(const CLabCustomField &other)
: CLabCustomField(other)
{
	m_lcftType = lcftSingleSelectList;
	if(other.GetFieldType() != this->GetFieldType()) {
		m_varDefaultValue.Clear();
		m_varDefaultValue = g_cvarNull;
	}
	// We may be able to use a couple special fields from the other field
	if(other.GetFieldType() == lcftSingleSelectList) {
		// copy over the items
		CLabCustomListField *pOtherListField = &(CLabCustomListField)other;
		std::map<long, CLabCustomListFieldItem *>::iterator iter;
		for(iter = m_mapItems.begin(); iter != m_mapItems.end(); iter++) {
			CLabCustomListFieldItem *pItem = iter->second;
			CLabCustomListFieldItem *pNewItem = new CLabCustomListFieldItem(*pItem);
			pItem->SetModificationStatus(CLabCustomListFieldItem::emsNone);
			AddItem(pItem);
		}
	}
}

// (r.gonet 09/21/2011) - PLID 45583 - Virtual destructor which rids us of our list items.
CLabCustomListField::~CLabCustomListField()
{
	std::map<long, CLabCustomListFieldItem *>::iterator iter;
	for(iter = m_mapItems.begin(); iter != m_mapItems.end(); iter++) {
		delete iter->second;
	}
	m_mapItems.clear();
}

// (r.gonet 09/21/2011) - PLID 45583 - Global function that compares two list items by sort order
//  Returns -1 if pA < pB. Returns 0 if pA == pB. Returns 1 if pA > pB.
// - pA is a void pointer to a list item pointer
// - pB is a void pointer to a list item pointer
int CompareLabCustomListFieldItems(const void * pA, const void * pB)
{
	CLabCustomListFieldItem **ppItemA = (CLabCustomListFieldItem **)pA;
	CLabCustomListFieldItem **ppItemB = (CLabCustomListFieldItem **)pB;
	ASSERT(ppItemA && ppItemB && *ppItemA && *ppItemB);
	int nResult = (*ppItemA)->GetSortOrder() < (*ppItemB)->GetSortOrder() ? -1 : ((*ppItemA)->GetSortOrder() > (*ppItemB)->GetSortOrder() ? 1 : 0);
	return nResult;
}

// (r.gonet 09/21/2011) - PLID 45583 - Creates the controls necessary to represent this list field on the screen. Creates header labels and a datalist combo box. The control and the field are only
//  synced when Saved in CLabCustomFieldControlManager.
// - cmControlManager is the control manager that is storing all of the controls for the present view of the fields.
// - pFieldInstance is the field value instance derived from this field which is currently being worked on. 
//    pFieldInstance can be null when the field itself is being edited and not its value, in which case the default value is loaded and displayed.
void CLabCustomListField::CreateControls(CLabCustomFieldControlManager &cmControlManager, CLabCustomFieldInstance *pFieldInstance/*= NULL*/)
{
	// (r.gonet 09/21/2011) - PLID 45555 - Begin a new field cell
	cmControlManager.BeginField(shared_from_this(), pFieldInstance, (this->GetSizeMode() == cfsmAutoSize && m_lcdtDisplayType == lcdtDropDown));
	CreateHeaderControls(cmControlManager, pFieldInstance); // Add the field labels

	// Calculate the combo box control's size.
	CRect rcList;
	switch(m_cfsmSizeMode) {
		case cfsmFixedSize:
			rcList = CRect(cmControlManager.GetBoundingBoxBottomLeft() + CPoint(10, 5), CSize(VarLong(m_varWidth, cmControlManager.GetDrawingSize().cx - 20), VarLong(m_varHeight, 20)));
			break;
		case cfsmAutoSize:
		default:		
			rcList = CRect(cmControlManager.GetBoundingBoxBottomLeft() + CPoint(10, 5), CSize(cmControlManager.GetDrawingSize().cx - 20, 20));
			break;
	}
	if(m_lcdtDisplayType == lcdtDropDown) {
		// Create the control.
		CWnd *pListWnd = new CWnd();
		BOOL bSuccess = pListWnd->CreateControl(_T("NXDATALIST2.NxDataListCtrl.1"), "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP,
			rcList,
			cmControlManager.GetParentWindow(), cmControlManager.GetNewControlID());
		if(!bSuccess) {
			ThrowNxException("CLabCustomListField::CreateControls : Attempt to create a custom list field failed.");
		}

		NXDATALIST2Lib::_DNxDataListPtr pList = pListWnd->GetControlUnknown();
		if(!pList) {
			ThrowNxException("CLabCustomListField::CreateControls : Attempt to create a custom list field failed.");
		}

		// Set a ton of settings
		// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - GetRemoteDataOrDataSnapshot -> GetRemoteDataSnapshot
		pList->IsComboBox = TRUE;
		pList->AdoConnection = NULL;//GetRemoteDataSnapshot(); // This really isn't important but I believe it is required.
		pList->AllowSort = FALSE;
		pList->GridVisible = FALSE;
		pList->InsertColumn(lcflID, _T("0"), _T("ID"), 0, csVisible|csFixedWidth);
		pList->GetColumn(lcflID)->DataType = VT_I4;
		pList->InsertColumn(lcflSortOrder, _T("0"), _T("SortOrder"), 0, csVisible|csFixedWidth);
		pList->GetColumn(lcflSortOrder)->DataType = VT_I4;
		pList->GetColumn(lcflSortOrder)->SortPriority = 0;
		pList->InsertColumn(lcflDescription, _T("0"), _T("Description"), 0, csVisible|csWidthAuto);
		pList->GetColumn(lcflDescription)->DataType = VT_BSTR;
		pList->GetColumn(lcflDescription)->FieldType = NXDATALIST2Lib::cftTextSingleLine;

		pList->DisplayColumn = _bstr_t(FormatString("[%li]", lcflDescription));
		pList->TextSearchCol = lcflDescription;
		pList->DropDownWidth = rcList.Width();
		pList->AutoDropDown = TRUE;

		// Now for the rows
		std::map<long, CLabCustomListFieldItem *>::iterator iter;
		for(iter = m_mapItems.begin(); iter != m_mapItems.end(); iter++) {
			CLabCustomListFieldItem *pItem = iter->second;
			// Don't add this item if it is scheduled to be deleted, or has been deleted and is not the default selected value
			// (r.gonet 09/21/2011) - PLID 45558 - Don't show deleted list items.
			// If this item is deleted but this field has it selected, show it anyway
			bool bShowItem = false;
			if(!pItem->GetDeleted() && pItem->GetModificationStatus() != CLabCustomListFieldItem::emsDeleted) {
				bShowItem = true;
			} else if(pItem->GetDeleted() && pFieldInstance != NULL && 
				(pFieldInstance->GetValue().vt == VT_I4 && VarLong(pFieldInstance->GetValue()) == pItem->GetID())) {
				bShowItem = true;
			}
			if(bShowItem) {
				NXDATALIST2Lib::IRowSettingsPtr pNewRow = pList->GetNewRow();
				pNewRow->PutValue(lcflID, pItem->GetID());
				pNewRow->PutValue(lcflSortOrder, pItem->GetSortOrder());
				pNewRow->PutValue(lcflDescription, _bstr_t(pItem->GetDescription()));
				pList->AddRowSorted(pNewRow, NULL);
			}
		}

		// Set the selected row if there is one
		if(pFieldInstance != NULL && pFieldInstance->GetValue().vt == VT_I4) {
			pList->SetSelByColumn(lcflID, VarLong(pFieldInstance->GetValue()));
		} else if((pFieldInstance == NULL || pFieldInstance->GetModificationStatus() == CLabCustomFieldInstance::emsNew) && 
			this->m_varDefaultValue.vt == VT_I4) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = pList->SetSelByColumn(lcflID, VarLong(this->m_varDefaultValue));
		}

		// (r.gonet 09/21/2011) - PLID 45555 - Add the combo box to management
		cmControlManager.AddInputControl(pListWnd);
	} else if(m_lcdtDisplayType == lcdtDefault || m_lcdtDisplayType == lcdtRadioButtons) {
		// (r.gonet 10/30/2011) - PLID 45583 - Radio buttons are now available and are the default option instead of datalists, for less repaint time.
		CLabCustomListFieldItem **arypItems = new CLabCustomListFieldItem*[m_mapItems.size()];
		unsigned int nIndex = 0;
		std::map<long, CLabCustomListFieldItem *>::iterator iter;
		for(iter = m_mapItems.begin(); iter != m_mapItems.end(); iter++) {
			CLabCustomListFieldItem *pItem = iter->second;
			arypItems[nIndex++] = pItem;
		}
		qsort(arypItems, m_mapItems.size(), sizeof(CLabCustomListFieldItem *), CompareLabCustomListFieldItems);
		
		for(unsigned int i = 0; i < m_mapItems.size(); i++) {
			CLabCustomListFieldItem *pItem = arypItems[i];

			// Don't add this item if it is scheduled to be deleted, or has been deleted and is not the default selected value
			// (r.gonet 09/21/2011) - PLID 45558 - Don't show deleted list items.
			// If this item is deleted but this field has it selected, show it anyway
			bool bShowItem = false;
			if(!pItem->GetDeleted() && pItem->GetModificationStatus() != CLabCustomListFieldItem::emsDeleted) {
				bShowItem = true;
			} else if(pItem->GetDeleted() && pFieldInstance != NULL && 
				(pFieldInstance->GetValue().vt == VT_I4 && VarLong(pFieldInstance->GetValue()) == pItem->GetID())) {
				bShowItem = true;
			}
			if(bShowItem) {
				NxButton *pRadioButton = new NxButton();
				BOOL bSuccess = pRadioButton->Create(pItem->GetDescription(), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_EX_TRANSPARENT | WS_TABSTOP, 
					CRect(cmControlManager.GetBoundingBoxBottomLeft() + CPoint(10, 0), CSize(10, 10)),
					cmControlManager.GetParentWindow(), cmControlManager.GetNewControlID());
				if(!bSuccess) {
					ThrowNxException("CLabCustomListField::CreateControls : Attempt to create a custom list field failed.");
				}
				if(i == 0) {
					// First radio button of a radio button group
					pRadioButton->ModifyStyle(0, WS_GROUP);
				}
				cmControlManager.SetIdealSize(pRadioButton);
				// Set the selected row if there is one
				if(pFieldInstance != NULL && pFieldInstance->GetValue().vt == VT_I4) {
					pRadioButton->SetCheck(pItem->GetID() == VarLong(pFieldInstance->GetValue()));
				} else if((pFieldInstance == NULL || pFieldInstance->GetModificationStatus() == CLabCustomFieldInstance::emsNew) && 
					this->m_varDefaultValue.vt == VT_I4) {
					pRadioButton->SetCheck(pItem->GetID() == VarLong(this->m_varDefaultValue));
				}
				// (r.gonet 09/21/2011) - PLID 45555 - Add the radio button to the current field cell
				cmControlManager.AddInputControl(pRadioButton, _variant_t(pItem->GetID()));
			}
		}

		delete [] arypItems;
	}
	// (r.gonet 09/21/2011) - PLID 45555 - End the field cell
	cmControlManager.EndField();
}

// (r.gonet 09/21/2011) - PLID 45583 - Using the control representing the input control created by CreateControls, sets the field's value, effectively syncing the screen with the in memory object.
// - pInputControl is the control containing the value to sync this field with. It may not be null and must be a valid window.
// - pInstance is the field value instance derived from this field that being worked on. If pInstance is null then the default value is set instead of the instance value.
bool CLabCustomListField::SetValueFromControl(CLabCustomFieldControlManager::CFieldCell *pFieldCell, CString &strErrorString, CLabCustomFieldInstance *pInstance/*= NULL*/)
{
	ASSERT(pFieldCell);
	if(pFieldCell->GetInputControlCount() == 0) {
		return true;
	}

	// See if they selected anything
	if(m_lcdtDisplayType == lcdtDropDown) {
		CWnd *pInputControl = pFieldCell->GetInputControl(0);
		long nSelectionId = -1;
		NXDATALIST2Lib::_DNxDataListPtr pList = pInputControl->GetControlUnknown();
		ASSERT(pList);
		if(pList) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = pList->CurSel;
			if(pRow) {
				nSelectionId = VarLong(pRow->GetValue(lcflID), -1);
			}
		}

		_variant_t varValue = g_cvarNull;
		if(nSelectionId != -1) {
			// Yes, they've selected a row. In this case, the variant value is the item ID in the database.
			varValue = _variant_t(nSelectionId);
		}

		if(pInstance != NULL) {
			pInstance->SetValue(varValue);
		} else {
			this->SetDefaultValue(varValue);
		}
	} else if(m_lcdtDisplayType == lcdtDefault || m_lcdtDisplayType == lcdtRadioButtons) {
		// (r.gonet 10/30/2011) - PLID 45583 - Radio buttons are now possible
		_variant_t varValue = g_cvarNull;
		for(int i = 0; i < pFieldCell->GetInputControlCount(); i++) {
			CWnd *pItemControl = pFieldCell->GetInputControl(i);
			NxButton *pRadioButton = dynamic_cast<NxButton *>(pItemControl);
			if(!pRadioButton) {
				ThrowNxException("CLabCustomListField::SetValueFromControl : Attempted to downcast CWnd to NxButton but failed.");
			}
			long nItemID = VarLong(pFieldCell->GetInputControlTag(i), -1);
			ASSERT(nItemID != -1);
			
			if(pRadioButton->GetCheck()) {
				varValue = _variant_t(nItemID);
			}
		}

		if(pInstance != NULL) {
			pInstance->SetValue(varValue);
		} else {
			this->SetDefaultValue(varValue);
		}
	}

	return true;
}

// (r.gonet 09/21/2011) - PLID 45583 - Static method. Loads a single item from a recordset into the list.
// - prs is the recordset to retrieve the item from. All column names are agreed upon by this fuction and the caller. Tight coupling involved.
//    Gets a single row but does not move the cursor in the recordset.
// - pListField is the field to load the item into.
void CLabCustomListField::LoadItem(_RecordsetPtr prs, CLabCustomListFieldPtr pListField)
{
	long nID = VarLong(prs->Fields->Item["ID"]->Value);
	ELabCustomFieldType lcftType = (ELabCustomFieldType)VarLong(prs->Fields->Item["DataTypeID"]->Value, -1);
	long nSortOrder = VarLong(prs->Fields->Item["SortOrder"]->Value);

	_variant_t varValue;
	switch(lcftType) {
		case lcftBool:
			varValue = _variant_t(VarLong(prs->Fields->Item["IntParam"]->Value, 0) != 0);
			break;
		case lcftInteger:
			varValue = prs->Fields->Item["IntParam"]->Value;
			break;
		case lcftFloat:
			varValue = prs->Fields->Item["FloatParam"]->Value;
			break;
		case lcftDateTime:
			varValue = prs->Fields->Item["DateTimeParam"]->Value;
			break;
		case lcftText:
			varValue = prs->Fields->Item["TextParam"]->Value;
			break;
		default:
			// We attempted to load a type but we don't know what this type is.
			// Either this is bad data or somebody needs to add a handler here.
			prs->Close();
			ASSERT(false);
			ThrowNxException("CLabCustomListField::LoadItem : Attempted to load a list field with an unrecognized item type.");
	}

	CString strDescription = VarString(prs->Fields->Item["Description"]->Value, "");

	CLabCustomListFieldItem *pItem = pListField->AddItem(nID, varValue, lcftType, strDescription, nSortOrder);
	// (r.gonet 09/21/2011) - PLID 45558 - The item may have been "deleted" from the database, so set that status here in the object.
	pItem->SetDeleted(VarBool(prs->Fields->Item["ListItemDeleted"]->Value) != FALSE);
	// (r.gonet 09/21/2011) - PLID 45557 - Init the item's GUID
	pItem->SetGUID(VarString(prs->Fields->Item["UID"]->Value));
}

// (r.gonet 09/21/2011) - PLID 45583 - Inserts this list field into the database. Overrides the base custom lab field insert method in order to insert its items as well.
void CLabCustomListField::PerformDBInsert()
{
	CParamSqlBatch sqlBatch;
	// Call our base class's insert statement, we would need to do exactly the same thing anyway.
	CLabCustomField::PerformDBInsert();

	// We may be changing the map items, so iterate over a temporary copy.
	std::map<long, CLabCustomListFieldItem *> mapTempItems(m_mapItems);
	
	std::map<long, CLabCustomListFieldItem *>::iterator iter;
	for(iter = mapTempItems.begin(); iter != mapTempItems.end(); iter++) {
		CLabCustomListFieldItem *pItem = iter->second;
		long nOldID = pItem->GetID();
		// Save our item.
		pItem->Save(sqlBatch, this->GetID());
		if(VarLong(m_varDefaultValue, -1) == nOldID) {
			m_varDefaultValue = _variant_t(pItem->GetID());
		}
		// Since we give a new item a placeholder ID until it is saved, the ID might be different
		if(pItem->GetID() != nOldID) {
			m_mapItems.erase(nOldID);
			m_mapItems[pItem->GetID()] = pItem;
		}
	}
	sqlBatch.Execute(GetRemoteData());

	// Our base class didn't and couldn't set the default selection, so we do it after everything else is done.
	ExecuteParamSql(
		"UPDATE LabCustomFieldsT "
		"SET DefaultSelectionID = {VT_I4} "
		"WHERE LabCustomFieldsT.ID = {INT}; ",
		m_varDefaultValue, this->GetID());
	m_emsModificationStatus = emsNone;
}

// (r.gonet 09/21/2011) - PLID 45583 - Adds an update query statement to a batch that, once executed, commits any changes to this field to the database, then resets the modification status.
// - sqlBatch is the batch object to write the query statement to.
void CLabCustomListField::AppendUpdateSql(CParamSqlBatch &sqlBatch)
{
	CLabCustomField::AppendUpdateSql(sqlBatch);
	
	// We may be changing the map items, so iterate over a temporary copy.
	std::map<long, CLabCustomListFieldItem *> mapTempItems(m_mapItems);
	
	std::map<long, CLabCustomListFieldItem *>::iterator iter;
	for(iter = mapTempItems.begin(); iter != mapTempItems.end(); iter++) {
		CLabCustomListFieldItem *pItem = iter->second;
		long nOldID = pItem->GetID();
		// Save our item.
		pItem->Save(sqlBatch, this->GetID());
		if(VarLong(m_varDefaultValue, -1) == nOldID) {
			m_varDefaultValue = _variant_t(pItem->GetID());
		}
		// Since we give a new item a placeholder ID until it is saved, the ID might be different
		if(pItem->GetID() != nOldID) {
			m_mapItems.erase(nOldID);
			m_mapItems[pItem->GetID()] = pItem;
		}
	}

	// Our base class didn't and couldn't set the default selection, so we do it after everything else is done.
	sqlBatch.Add(
		"UPDATE LabCustomFieldsT "
		"SET DefaultSelectionID = {VT_I4} "
		"WHERE LabCustomFieldsT.ID = {INT}; ",
		m_varDefaultValue, this->GetID());
	m_emsModificationStatus = emsNone;
}

// (r.gonet 09/21/2011) - PLID 45583 - Adds an item already constructed to the list field's items.
// - An existing list item. May not be null.
CLabCustomListFieldItem * CLabCustomListField::AddItem(CLabCustomListFieldItem *pItem)
{
	if(pItem == NULL) {
		ThrowNxException("CLabCustomListField::AddItem : Attempted to add a NULL item.");
	}

	// Calculate the new sort order.
	if(pItem->GetSortOrder() < 0) {
		long nSortOrder = -1;
		std::map<long, CLabCustomListFieldItem *>::iterator iter;
		for(iter = m_mapItems.begin(); iter != m_mapItems.end(); iter++) {
			nSortOrder = max(iter->second->GetSortOrder(), nSortOrder);
		}
		pItem->SetSortOrder(nSortOrder + 1);
	}
	// Add the item.
	m_mapItems[pItem->GetID()] = pItem;
	
	return pItem;
}

// (r.gonet 09/21/2011) - PLID 45583 - Adds a new item to this list field's items. This new item will be inserted when the field is saved.
// - varValue the item's internal value.
// - lcftType is the item's data type.
// - strDescription is the item's visible description.
CLabCustomListFieldItem * CLabCustomListField::AddItem(_variant_t varValue, ELabCustomFieldType lcftType, CString strDescription)
{
	// We need a unique ID here, at least until this item is saved. Plus a sort order.
	long nMinID = -2, nSortOrder = -1;
	std::map<long, CLabCustomListFieldItem *>::iterator iter;
	for(iter = m_mapItems.begin(); iter != m_mapItems.end(); iter++) {
		CLabCustomListFieldItem *pItem = iter->second;
		ASSERT(pItem != NULL);
		nMinID = nMinID > pItem->GetID() ? pItem->GetID() : nMinID;
		nSortOrder = max(iter->second->GetSortOrder(), nSortOrder);
	}

	// Now create the item and add it.
	CLabCustomListFieldItem *pItem = new CLabCustomListFieldItem(nMinID - 1, varValue, lcftType, strDescription, nSortOrder + 1);
	pItem->SetModificationStatus(CLabCustomListFieldItem::emsNew);
	m_mapItems[pItem->GetID()] = pItem;
	return pItem;
}

// (r.gonet 09/21/2011) - PLID 45583 - Adds an already existing item to this list field's items.
// - nID is the database ID of the item.
// - varValue the item's internal value.
// - lcftType is the item's data type.
// - strDescription is the item's visible description.
// - nSortOrder is the order in which the item appears in this list field.
CLabCustomListFieldItem * CLabCustomListField::AddItem(long nID, _variant_t varValue, ELabCustomFieldType lcftType, CString strDescription, long nSortOrder)
{
	CLabCustomListFieldItem *pItem = new CLabCustomListFieldItem(nID, varValue, lcftType, strDescription, nSortOrder);
	m_mapItems[nID] = pItem;
	if(m_emsModificationStatus == emsNone) {
		m_emsModificationStatus = emsModified;
	}
	return pItem;
}

// (r.gonet 09/21/2011) - PLID 45583 - Gets an item from this list field by database ID. Returns null if cannot find it.
// - nID is the database ID of the list item.
CLabCustomListFieldItem * CLabCustomListField::GetItem(long nID)
{
	CLabCustomListFieldItem *pItem = NULL;
	std::map<long, CLabCustomListFieldItem *>::iterator iter;
	if((iter = m_mapItems.find(nID)) != m_mapItems.end()) {
		return iter->second;
	} else {
		return NULL;
	}
}

// Gets the number of items in the list.
long CLabCustomListField::GetItemCount() const
{
	return m_mapItems.size();
}

// (r.gonet 09/21/2011) - PLID 45583 - Removes an item from the list by database ID. Marks it as to be deleted.
// - nID is the database ID of the list item to be removed.
void CLabCustomListField::RemoveItem(long nID)
{
	std::map<long, CLabCustomListFieldItem *>::iterator iter;
	if((iter = m_mapItems.find(nID)) != m_mapItems.end()) {
		CLabCustomListFieldItem *pItem = iter->second;
		ASSERT(pItem);
		pItem->SetModificationStatus(CLabCustomListFieldItem::emsDeleted);
		if(m_varDefaultValue.vt == VT_I4 && VarLong(m_varDefaultValue) == pItem->GetID()) {
			// We deleted the default selection, so we no longer have a default selection.
			m_varDefaultValue = g_cvarNull;
		}
	}
}

// (r.gonet 09/21/2011) - PLID 45583 - Marks all items as to be deleted.
void CLabCustomListField::RemoveAllItems()
{
	std::map<long, CLabCustomListFieldItem *>::iterator iter;
	for(iter = m_mapItems.begin(); iter != m_mapItems.end(); iter++) {
		CLabCustomListFieldItem *pItem = iter->second;
		ASSERT(pItem);
		pItem->SetModificationStatus(CLabCustomListFieldItem::emsDeleted);
		if(m_varDefaultValue.vt == VT_I4 && VarLong(m_varDefaultValue) == pItem->GetID()) {
			// We deleted the default selection, so we no longer have a default selection.
			m_varDefaultValue = g_cvarNull;
		}
	}
}

// (r.gonet 09/21/2011) - PLID 45583 - Gets an iterator pointing to the start of the list field's items.
std::map<long, CLabCustomListFieldItem *>::iterator CLabCustomListField::GetItemsBegin()
{
	return m_mapItems.begin();
}

// (r.gonet 09/21/2011) - PLID 45583 - Gets an iterator pointing to just beyond the end of the list field's items.
std::map<long, CLabCustomListFieldItem *>::iterator CLabCustomListField::GetItemsEnd()
{
	return m_mapItems.end();
}

///////////////////////////////////////////////////////////////////////////////
// Custom Lab Field Instance
///////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45124 - Creates a new instance of a lab custom field, which may contain a value chosen by the user.
CLabCustomFieldInstance::CLabCustomFieldInstance()
{
	m_nID = -1;
	m_varValue = g_cvarNull;
	m_emsModificationStatus = emsNew;
}

// (r.gonet 09/21/2011) - PLID 45124 - Creates a new instance of a lab custom field, which may contain a value chosen by the user.
// - pTemplateField is the template field this instance is an instance of.
// - pTemplateInstance is the template instance this instance belongs on.
CLabCustomFieldInstance::CLabCustomFieldInstance(CCFTemplateFieldPtr pTemplateField, CCFTemplateInstance *pTemplateInstance)
{
	m_nID = -1;
	m_pTemplateField = pTemplateField;
	m_pTemplateInstance = pTemplateInstance;
	m_varValue = g_cvarNull;
	m_emsModificationStatus = emsNew;

	// We may have a default value
	CLabCustomFieldPtr pField;
	if(pTemplateField && (pField = pTemplateField->GetField()) != NULL) {
		_variant_t varDefaultValue = pField->GetDefaultValue();
		m_varValue = varDefaultValue;
	}
}

// (r.gonet 09/21/2011) - PLID 45124 - Creates an existing instance of a lab custom field.
// - nID is the database ID of the already existing field instance.
// - pTemplateField is the template field this instance is an instance of.
// - pTemplateInstance is the template instance this instance belongs on.
CLabCustomFieldInstance::CLabCustomFieldInstance(long nID, CCFTemplateFieldPtr pTemplateField, CCFTemplateInstance *pTemplateInstance)
{
	m_nID = nID;
	m_pTemplateField = pTemplateField;
	m_pTemplateInstance = pTemplateInstance;
	m_varValue = g_cvarNull;
	m_emsModificationStatus = emsNone;
}

// (r.gonet 09/21/2011) - PLID 45124 - Gets the template field this instance is an instance of. 
CCFTemplateFieldPtr CLabCustomFieldInstance::GetTemplateField() const
{
	return m_pTemplateField;
}

// (r.gonet 09/21/2011) - PLID 45124 - Gets the ID of this field instance
long CLabCustomFieldInstance::GetID() const
{
	return m_nID;
}

// (r.gonet 09/21/2011) - PLID 45124 - Gets the value the user has entered. May be VT_NULL if no value is entered.
_variant_t CLabCustomFieldInstance::GetValue() const
{
	return m_varValue;
}

// (r.gonet 09/21/2011) - PLID 45124 - Sets the value the user has entered.
// - varValue is the value the user entered. Should correspond to the field type. May be VT_NULL.
void CLabCustomFieldInstance::SetValue(_variant_t &varValue)
{
	m_varValue = varValue;
	if(m_emsModificationStatus == emsNone) {
		m_emsModificationStatus = emsModified;
	}
}

// (r.gonet 09/21/2011) - PLID 45124 - Sets the modification status, which affects how this field is saved.
// - emsModificationStatus is the new status.
void CLabCustomFieldInstance::SetModificationStatus(EModificationStatus emsModificationStatus)
{
	m_emsModificationStatus = emsModificationStatus;
}

// (r.gonet 12/05/2011) - PLID 45581 - Gets the modification status of the field instance, which affects how this field is saved.
CLabCustomFieldInstance::EModificationStatus CLabCustomFieldInstance::GetModificationStatus() const
{
	return m_emsModificationStatus;
}

// (r.gonet 09/21/2011) - PLID 45124 - Saves this field instance to the database, committing the user's choices. Note that inserts are done right away and not batched.
// - sqlBatch is the batch object to write the save statements to.
void CLabCustomFieldInstance::Save(CParamSqlBatch &sqlBatch)
{
	if(m_emsModificationStatus == emsNew) {
		PerformDBInsert();
	} else if(m_emsModificationStatus == emsModified) {
		AppendUpdateSql(sqlBatch);
	}
}

// (r.gonet 09/21/2011) - PLID 45124 - Inserts this field instance into the database. It must be associated with both an existing template field and a template instance in order to be saved.
void CLabCustomFieldInstance::PerformDBInsert()
{
	CLabCustomFieldPtr pField = m_pTemplateField->GetField();
	if(m_pTemplateField == NULL || pField->GetID() < 0) {
		ThrowNxException("CLabCustomFieldInstance::PerformDBInsert : Attempted to save a lab custom field instance with a NULL or non-existent custom field.");
	}

	if(m_pTemplateInstance == NULL || m_pTemplateInstance->GetID() < 0) {
		ThrowNxException("CLabCustomFieldInstance::PerformDBInsert : Attempted to save a lab custom field instance with a NULL or non-existent template instance.");
	}

	_RecordsetPtr prs = CreateParamRecordset(
		"SET NOCOUNT ON; "
		"INSERT INTO LabCustomFieldInstancesT (FieldID, CFTemplateInstanceID, TemplateFieldID, "
		"	SelectionID, IntParam, FloatParam, TextParam, MemoParam, DateTimeParam) "
		"VALUES "
		"	({INT}, {INT}, {INT}, {VT_I4}, {VT_I4}, {VT_R8}, {VT_BSTR}, {VT_BSTR}, {VT_DATE}); "
		"SET NOCOUNT OFF; "
		"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS NewID; ",
		pField->GetID(), m_pTemplateInstance->GetID(), m_pTemplateField->GetID(),
		(pField->GetFieldType() == lcftSingleSelectList ? m_varValue : g_cvarNull),
		(pField->GetFieldType() == lcftInteger ? m_varValue : 
			(pField->GetFieldType() == lcftBool && m_varValue.vt == VT_BOOL ? _variant_t((long)VarBool(m_varValue)) : g_cvarNull)),
		(pField->GetFieldType() == lcftFloat ? m_varValue : g_cvarNull),
		(pField->GetFieldType() == lcftText ? m_varValue : g_cvarNull),
		(pField->GetFieldType() == lcftMemo ? m_varValue : g_cvarNull),
		(pField->GetFieldType() == lcftDateTime && m_varValue.vt == VT_DATE ? m_varValue : g_cvarNull));
	m_nID = VarLong(prs->Fields->Item["NewID"]->Value);
	
	// Now we know we have saved successfully, so we have no changes
	m_emsModificationStatus = emsNone;
}

// (r.gonet 09/21/2011) - PLID 45124 - Adds a statement to update an already commited instance to a batch.
// - sqlBatch is the batch object to write the save statement to.
void CLabCustomFieldInstance::AppendUpdateSql(CParamSqlBatch &sqlBatch)
{
	CLabCustomFieldPtr pField = m_pTemplateField->GetField();
	if(this->GetID() < 0) {
		ThrowNxException("CLabCustomFieldInstance::AppendUpdateSql: Attempted to update a non-existent lab custom field instance.");
	}

	sqlBatch.Add(
		"UPDATE LabCustomFieldInstancesT "
		"SET SelectionID = {VT_I4}, "
		"	IntParam = {VT_I4}, "
		"	FloatParam = {VT_R8}, "
		"	TextParam = {VT_BSTR}, "
		"	MemoParam = {VT_BSTR}, "
		"	DateTimeParam = {VT_DATE} "
		"WHERE ID = {INT} ",
		(pField->GetFieldType() == lcftSingleSelectList ? m_varValue : g_cvarNull),
		(pField->GetFieldType() == lcftInteger ? m_varValue : 
			(pField->GetFieldType() == lcftBool && m_varValue.vt == VT_BOOL ? _variant_t((long)VarBool(m_varValue)) : g_cvarNull)),
		(pField->GetFieldType() == lcftFloat ? m_varValue : g_cvarNull),
		(pField->GetFieldType() == lcftText ? m_varValue : g_cvarNull),
		(pField->GetFieldType() == lcftMemo ? m_varValue : g_cvarNull),
		(pField->GetFieldType() == lcftDateTime && m_varValue.vt == VT_DATE ? m_varValue : g_cvarNull),
		m_nID);

	// We haven't saved yet, so let the caller reset our modification status
	// m_emsModificationStatus = emsNone;
}

// (r.gonet 10/07/2011) - PLID 44212 - Required for lab barcode merging of the custom fields
CLabCustomFieldPtr CLabCustomFieldInstance::GetField()
{
	if(m_pTemplateField != NULL) {
		return m_pTemplateField->GetField();
	} else {
		return CLabCustomFieldPtr();
	}
}