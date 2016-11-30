#include "stdafx.h"
#include "MiscSystemUtils.h"
#include "LabCustomFieldTemplate.h"
#include "LabCustomField.h"

using namespace ADODB;

////////////////////////////////////////////////////////////////////////////////
// Custom Fields Template Field
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45586 - Creates a new template field, which is a lab custom field on a particular template. Template Fields
//  have some additional properties that are template specific.
// - pField is the lab custom field we are associated with and a kind of
// - pTemplate is the template we are associated with and specific to
CCFTemplateField::CCFTemplateField(CLabCustomFieldPtr pField, CLabProcCFTemplatePtr pTemplate)
{
	m_nID = -1;
	m_pField = pField;
	m_pTemplate = pTemplate;
	m_nSortOrder = 0;
	m_bRequired = false;
	m_emsModificationStatus = emsNew;
	// (r.gonet 09/21/2011) - PLID 45558 - Naturally, a new template field is not deleted.
	m_bDeleted = false;
}

// (r.gonet 09/21/2011) - PLID 45586 - Creates an existing template field, which is a lab custom field on a particular template. Template Fields
//  have some additional properties that are template specific.
// - pField is the lab custom field we are associated with and a kind of
// - pTemplate is the template we are associated with and specific to  
CCFTemplateField::CCFTemplateField(long nID, CLabCustomFieldPtr pField, CLabProcCFTemplatePtr pTemplate)
{
	m_nID = nID;
	m_pField = pField;
	m_pTemplate = pTemplate;
	m_nSortOrder = 0;
	m_bRequired = false;
	m_emsModificationStatus = emsNone;
	// (r.gonet 09/21/2011) - PLID 45558 - Default to not deleted. Make the caller change it if they really want.
	m_bDeleted = false;
}

// (r.gonet 09/21/2011) - PLID 45586 - Gets the state of modification of this field. Tells you if this field has been modified or is new, etc
CCFTemplateField::EModificationStatus CCFTemplateField::GetModificationStatus() const
{
	return m_emsModificationStatus;
}

// (r.gonet 09/21/2011) - PLID 45586 - Sets the state of modification of this field. For things that couldn't be known by the field internally.
// - emsModificationStatus is the new status of the field.
void CCFTemplateField::SetModificationStatus(CCFTemplateField::EModificationStatus emsModificationStatus)
{
	m_emsModificationStatus = emsModificationStatus;
}

// (r.gonet 09/21/2011) - PLID 45586 - Gets the database ID of this field. -1 if not saved yet.
long CCFTemplateField::GetID() const
{
	return m_nID;
}

// (r.gonet 09/21/2011) - PLID 45586 - Sets the database ID of this template field. Not tracked.
void CCFTemplateField::SetID(long nID)
{
	m_nID = nID;
}

// (r.gonet 09/21/2011) - PLID 45586 - Gets the associated custom field.
CLabCustomFieldPtr CCFTemplateField::GetField() const
{
	return m_pField;
}

// (r.gonet 09/21/2011) - PLID 45586 - Sets the associated custom field.
void CCFTemplateField::SetField(CLabCustomFieldPtr pField)
{
	if(m_emsModificationStatus == emsNone) {
		m_emsModificationStatus = emsModified;
	}
	m_pField = pField;
}

// (r.gonet 09/21/2011) - PLID 45586 - Gets the associated template.
CLabProcCFTemplatePtr CCFTemplateField::GetTemplate() const
{
	return m_pTemplate.lock();
}

// (r.gonet 09/21/2011) - PLID 45586 - Gets the sort order, which position this field will appear in relation to all other fields on the template.
long CCFTemplateField::GetSortOrder() const
{
	return m_nSortOrder;
}

// (r.gonet 09/21/2011) - PLID 45586 - Sets the sort order, which position this field will appear in relation to all other fields on the template.
// - nSortOrder is the new order. Sort order ascends.
void CCFTemplateField::SetSortOrder(long nSortOrder)
{
	if(m_emsModificationStatus == emsNone) {
		m_emsModificationStatus = emsModified;
	}
	m_nSortOrder = nSortOrder;
}

// (r.gonet 09/21/2011) - PLID 45586 - Gets whether this field is required to be filled on this template. Returns true if this field is required to be filled. False otherwise.
bool CCFTemplateField::GetRequired() const
{
	return m_bRequired;
}

// (r.gonet 09/21/2011) - PLID 45586 - Sets whether this field is required to be filled on this template.
// - bRequired is the required flag. It should be true if the field is to be required to be filled. False otherwise.
void CCFTemplateField::SetRequired(bool bRequired)
{
	if(m_emsModificationStatus == emsNone) {
		m_emsModificationStatus = emsModified;
	}
	m_bRequired = bRequired;
}

// (r.gonet 09/21/2011) - PLID 45558 - Gets whether this template field was marked as deleted from the database. True means that it has and that the field
//  should only show up on historical lab records where it was in use. Newly instantiated templates will not have the associated field.
bool CCFTemplateField::GetDeleted() const
{
	return m_bDeleted;
}

// (r.gonet 09/21/2011) - PLID 45558 - Sets whether this template field was marked as deleted from the database. Should only be called during initialization.
// - bDeleted is the state of deletion. True means that it has and that the field
//  should only show up on historical lab records where it was in use. Newly instantiated templates will not have the associated field.
void CCFTemplateField::SetDeleted(bool bDeleted)
{
	m_bDeleted = bDeleted;
}

// (r.gonet 09/21/2011) - PLID 45586 - Writes a query statement to a batch that once executed, will save this template field to the database. Then resets the modification status.
//  Note that this query is not automatically run, except for cases of INSERT, and then the statement is not written to the batch.
// - sqlBatch is the batch object to write the save statement to.
void CCFTemplateField::Save(CParamSqlBatch &sqlBatch)
{
	if(m_pField == NULL || m_pField->GetID() < 0) {
		// All template fields must be linked to an existing custom field.
		ThrowNxException("CCFTemplateField::Save : Attempted to save a template field with a NULL or non-existent custom field.");
	}

	shared_ptr<CLabProcCFTemplate> pTemplate = m_pTemplate.lock();
	if(pTemplate == NULL || m_pField->GetID() < 0) {
		// All template fields must belong to an existing template.
		ThrowNxException("CCFTemplateField::Save : Attempted to save a template field with a NULL or non-existent template.");
	}

	if(m_emsModificationStatus == emsNew) {
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON; "
			"INSERT INTO LabProcCFTemplateFieldsT "
			"(FieldID, LabProcCFTemplateID, SortOrder, Required) "
			"VALUES "
			"({INT}, {INT}, {INT}, {BIT}); "
			"SET NOCOUNT OFF; "
			"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS NewID; ", 
			m_pField->GetID(), pTemplate->GetID(), m_nSortOrder, m_bRequired);
		m_nID = VarLong(prs->Fields->Item["NewID"]->Value);
		prs->Close();
	} else if(m_emsModificationStatus == emsModified) {
		sqlBatch.Add(
			"UPDATE LabProcCFTemplateFieldsT SET "
			"	SortOrder = {INT}, "
			"	Required = {BIT} "
			"WHERE ID = {INT}; ",
			m_nSortOrder, m_bRequired, m_nID);
	} else if(m_emsModificationStatus == emsDeleted) {
		// (r.gonet 09/21/2011) - PLID 45558 - Mark as deleted instead of actually deleting. Preserves referential integrity.
		sqlBatch.Add( 
			"UPDATE LabProcCFTemplateFieldsT "
			"SET Deleted = 1 " 
			"WHERE ID = {INT}; ",
			m_nID);
		// (r.gonet 09/21/2011) - PLID 45558 - Might as well mark it in the object as well.
		m_bDeleted = true;
	}
	m_emsModificationStatus = emsNone;
};

////////////////////////////////////////////////////////////////////////////////
// Custom Fields Template
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45586 - Creates a new custom fields template that will hold lab custom fields and can be instantiated.
//  The template itself may be edited to modify the fields held on it.
CLabProcCFTemplate::CLabProcCFTemplate()
{
	m_nID = -1;
	m_strName = "";
	m_emsModificationStatus = emsNew;
	// (r.gonet 09/21/2011) - PLID 45558 - Naturally, all new templates are not deleted.
	m_bDeleted = false;
	// (r.gonet 09/21/2011) - PLID 45557 - Init the template's GUID
	m_strGUID = NewUUID();
	m_nNextFieldID = -2;
}

CLabProcCFTemplate::~CLabProcCFTemplate()
{
}

// (r.gonet 09/21/2011) - PLID 45586 - Gets the database ID of this template. -1 if the template is not saved yet.
long CLabProcCFTemplate::GetID() const
{
	return m_nID;
}

// (r.gonet 09/21/2011) - PLID 45586 - Sets the state of modification of this template. For things that couldn't be known by the field internally.
// - emsModificationStatus is the new status of the field.
void CLabProcCFTemplate::SetModificationStatus(EModificationStatus emsModificationStatus)
{
	m_emsModificationStatus = emsModificationStatus;
}

// (r.gonet 09/21/2011) - PLID 45586 - Gets the state of modification of this template. Tells you if this field has been modified or is new, etc
CLabProcCFTemplate::EModificationStatus CLabProcCFTemplate::GetModificationStatus() const
{
	return m_emsModificationStatus;
}

// (r.gonet 09/21/2011) - PLID 45586 - Loads all of the fields and settings that are associated with the active template assigned to a certain lab procedure.
//  Returns true on a successful load and false otherwise.
// nLabProcedureID is the ID of a Lab Procedure record in the database. It must be a valid ID or the template will not be loaded.
bool CLabProcCFTemplate::LoadByLabProcedureID(long nLabProcedureID)
{
	if(nLabProcedureID <= 0) {
		return false;
	}

	// Find the labs lab proc custom field template ID, if it has one
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT LabProceduresT.LabProcCFTemplateID "
		"FROM LabProceduresT "
		"WHERE LabProceduresT.ID = {INT} AND LabProceduresT.LabProcCFTemplateID IS NOT NULL ",
		nLabProcedureID);
	if(prs->eof) {
		// This lab doesn't have a custom fields template, so quit
		return false;
	} else {
		this->Load(VarLong(prs->Fields->Item["LabProcCFTemplateID"]->Value));
		return true;
	}
}

// (r.gonet 09/21/2011) - PLID 45586 - Loads all of the fields and settings that are associated with a saved template.
// - nTemplateID is the ID of the template in the database to load. It must be a valid ID.
void CLabProcCFTemplate::Load(long nTemplateID)
{
	CParamSqlBatch sqlLoadingBatch;
	sqlLoadingBatch.Declare(_T("DECLARE @TemplateID INT; "));
	sqlLoadingBatch.Add("SET @TemplateID = {INT}; ", nTemplateID);
	sqlLoadingBatch.Add(
		// See if we even have a template
		// (r.gonet 09/21/2011) - PLID 45558 - Added getting the deleted flag
		// (r.gonet 09/21/2011) - PLID 45557 - Also get the GUID of this template.
		"SELECT ID, Name, Deleted, UID "
		"FROM LabProcCFTemplatesT "
		"WHERE ID = @TemplateID; ");
	sqlLoadingBatch.Add(
		// Get the custom fields for this procedure type
		// (r.gonet 09/21/2011) - PLID 45557 - Also get the GUID of the field.
		"SELECT LabCustomFieldsT.ID, LabCustomFieldsT.UID, LabCustomFieldsT.Name, LabCustomFieldsT.FriendlyName, LabCustomFieldsT.Description, "
		// Field settings
		// (r.gonet 09/21/2011) - PLID 45558 - Added getting the deleted flag for lab custom fields and template fields
		"	LabCustomFieldsT.DataTypeID, LabCustomFieldsT.DisplayTypeID, LabCustomFieldsT.MaxLength, "
		"	LabCustomFieldsT.SizeMode, LabCustomFieldsT.Width, LabCustomFieldsT.Height, "
		"	LabProcCFTemplateFieldsT.ID AS TemplateFieldID, LabProcCFTemplateFieldsT.SortOrder, LabProcCFTemplateFieldsT.Required, "
		"	LabProcCFTemplateFieldsT.Deleted AS TemplateFieldDeleted, "
		// Field defaults
		"	LabCustomFieldsT.DefaultSelectionID, LabCustomFieldsT.DefaultIntParam, LabCustomFieldsT.DefaultFloatParam, "
		"	LabCustomFieldsT.DefaultTextParam, LabCustomFieldsT.DefaultMemoParam, LabCustomFieldsT.DefaultDateTimeParam "
		"FROM LabProcCFTemplatesT "
		"	INNER JOIN "
		"LabProcCFTemplateFieldsT ON LabProcCFTemplatesT.ID = LabProcCFTemplateFieldsT.LabProcCFTemplateID "
		"	INNER JOIN "
		"LabCustomFieldsT ON LabProcCFTemplateFieldsT.FieldID = LabCustomFieldsT.ID "
		"WHERE LabProcCFTemplatesT.ID = @TemplateID "
		"ORDER BY LabProcCFTemplateFieldsT.SortOrder ASC; ");
	sqlLoadingBatch.Add(
		// (r.gonet 09/21/2011) - PLID 45583 - Get the list items for all list fields for this procedure type
		// (r.gonet 09/21/2011) - PLID 45558 - Added getting the deleted flag for items
		// (r.gonet 09/21/2011) - PLID 45557 - Get the GUIDs of the list items as well.
		"SELECT LabProcCFTemplateFieldsT.ID AS TemplateFieldID, LabCustomFieldListItemsT.ID, LabCustomFieldListItemsT.FieldID, LabCustomFieldListItemsT.Description, LabCustomFieldListItemsT.DataTypeID, "
		"	LabCustomFieldListItemsT.IntParam, LabCustomFieldListItemsT.FloatParam, LabCustomFieldListItemsT.TextParam, "
		"	LabCustomFieldListItemsT.MemoParam, LabCustomFieldListItemsT.DateTimeParam, "
		"	LabCustomFieldListItemsT.SortOrder, LabCustomFieldListItemsT.Deleted AS ListItemDeleted, LabCustomFieldListItemsT.UID "
		"FROM LabProcCFTemplatesT "
		"	INNER JOIN "
		"LabProcCFTemplateFieldsT ON LabProcCFTemplatesT.ID = LabProcCFTemplateFieldsT.LabProcCFTemplateID "
		"	INNER JOIN "
		"LabCustomFieldsT ON LabProcCFTemplateFieldsT.FieldID = LabCustomFieldsT.ID "
		"	INNER JOIN "
		"LabCustomFieldListItemsT ON LabCustomFieldsT.ID = LabCustomFieldListItemsT.FieldID "
		"WHERE LabProcCFTemplatesT.ID = @TemplateID "
		"ORDER BY LabProcCFTemplateFieldsT.SortOrder ASC; ");

	_RecordsetPtr prs = sqlLoadingBatch.CreateRecordset(GetRemoteData());
	
	// Do we have a template by this id?
	if(prs->eof) {
		prs->Close();
		ThrowNxException("CLabProcCFTemplate::Load : Attempted to load a non-existent template.");
	} else {
		m_nID = VarLong(prs->Fields->Item["ID"]->Value);
		m_strName = VarString(prs->Fields->Item["Name"]->Value);
		// (r.gonet 09/21/2011) - PLID 45558 - Init the deleted flag for the template
		m_bDeleted = VarBool(prs->Fields->Item["Deleted"]->Value) != FALSE;
		// (r.gonet 09/21/2011) - PLID 45557 - Init the template's GUID
		m_strGUID = VarString(prs->Fields->Item["UID"]->Value);
	}

	prs = prs->NextRecordset(NULL);
	// Load the template with its fields
	LoadTemplateFields(prs);
	prs = prs->NextRecordset(NULL);
	// (r.gonet 09/21/2011) - PLID 45583 - Load the template fields with their list items.
	LoadListFieldItems(prs);
	prs->Close();

	m_emsModificationStatus = emsNone;
}

// (r.gonet 09/21/2011) - PLID 45586 - Loads the template fields by a recordset. Should only be called when loading and the column names must be in agreement.
//  Eats all records in the current recordset.
// - prs is the recordset to load the fields from.
void CLabProcCFTemplate::LoadTemplateFields(_RecordsetPtr prs)
{
	ASSERT(prs);
	while(!prs->eof) {
		CLabCustomFieldPtr pField;
		long nID = VarLong(prs->Fields->Item["ID"]->Value);
		// This next part is a factory. Based on the type we have stored here, Create the right kind of field.
		ELabCustomFieldType lcftType = (ELabCustomFieldType)VarLong(prs->Fields->Item["DataTypeID"]->Value, -1);
		// Templates are not instances, so get the default value, which we choose based on the field type.
		_variant_t varDefault;
		switch(lcftType) {
			// (r.gonet 09/21/2011) - PLID 45581 - Added support for loading boolean values.
			case lcftBool:
				pField = make_shared<CLabCustomBooleanField>();
				// We store bools as ints in the database....
				varDefault = _variant_t(VarLong(prs->Fields->Item["DefaultIntParam"]->Value, 0) != 0);
				break;
			// (r.gonet 09/21/2011) - PLID 45581 - Added support for loading integer values.
			case lcftInteger:
				pField = make_shared<CLabCustomIntegerField>();
				varDefault = prs->Fields->Item["DefaultIntParam"]->Value;
				break;
			// (r.gonet 09/21/2011) - PLID 45581 - Added support for loading float values.
			case lcftFloat:
				pField = make_shared<CLabCustomFloatField>();
				varDefault = prs->Fields->Item["DefaultFloatParam"]->Value;
				break;
			// (r.gonet 09/21/2011) - PLID 45581 - Added support for loading datetime values.
			case lcftDateTime:
				pField = make_shared<CLabCustomDateTimeField>();
				varDefault = prs->Fields->Item["DefaultDateTimeParam"]->Value;
				break;
			// (r.gonet 09/21/2011) - PLID 45582 - Added support for loading text values.
			case lcftText:
				pField = make_shared<CLabCustomTextField>();
				varDefault = prs->Fields->Item["DefaultTextParam"]->Value;
				break;
			// (r.gonet 09/21/2011) - PLID 45582 - Added support for loading memo values.
			case lcftMemo:
				pField = make_shared<CLabCustomTextField>();
				varDefault = prs->Fields->Item["DefaultMemoParam"]->Value;
				break;
			// (r.gonet 09/21/2011) - PLID 45583 - Add support for loading list values. This is the selected item ID
			case lcftSingleSelectList:
				pField = make_shared<CLabCustomListField>();
				varDefault = prs->Fields->Item["DefaultSelectionID"]->Value;
				break;
			default:
				// We attempted to load a type but we don't know what this type is.
				// Either this is bad data or somebody needs to add a handler here.
				prs->Close();
				ASSERT(FALSE);
				ThrowNxException("CLabProcCFTemplate::LoadTemplateFields : Attempted to load a custom field with an unknown type.");
		}

		ASSERT(pField != NULL);
		// Now set all the settings, don't track these as modifications though.
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
		// (r.gonet 09/21/2011) - PLID 45582 - This may be a text field, in which case fill the max character length
		if(pField->GetFieldType() == lcftText || pField->GetFieldType() == lcftMemo) {
			shared_ptr<CLabCustomTextField> pTextField = boost::dynamic_pointer_cast<CLabCustomTextField>(pField);
			if(pTextField) {
				pTextField->SetMaxLength(VarLong(prs->Fields->Item["MaxLength"]->Value, 100));
			}
		}

		pField->TrackChanges(true);

		// Now that was only half the story (a big half). That field is wrapped in a Template Field which stores
		//  template specific values for the field. So create that and that is what we'll store. Not the field directly.
		long nTemplateFieldID = VarLong(prs->Fields->Item["TemplateFieldID"]->Value, -1);
		if(nTemplateFieldID == -1) {
			ThrowNxException("CLabProcCFTemplate::LoadTemplateFields : Template Field does not exist for a field on the template.");
		}
		CCFTemplateFieldPtr pTemplateField = make_shared<CCFTemplateField>(nTemplateFieldID, pField, shared_from_this());
		pTemplateField->SetSortOrder(VarLong(prs->Fields->Item["SortOrder"]->Value));
		pTemplateField->SetRequired(VarBool(prs->Fields->Item["Required"]->Value) != FALSE);
		// (r.gonet 09/21/2011) - PLID 45558 - Init the deleted flag for this template field. "Does this field show up on new template instances?"
		pTemplateField->SetDeleted(VarBool(prs->Fields->Item["TemplateFieldDeleted"]->Value) != FALSE);
		pTemplateField->SetModificationStatus(CCFTemplateField::emsNone);
		
		// Map the template field by the template field's ID.
		m_mapTemplateFields[pTemplateField->GetID()] = pTemplateField;
		
		prs->MoveNext();
	}
}

// (r.gonet 09/21/2011) - PLID 45583 - Loads all the template's fields' list items from a recordset. Should only be called from a loading method and then the field names must be agreed upon.
// - prs is the recordset to load the list items from.
void CLabProcCFTemplate::LoadListFieldItems(_RecordsetPtr prs)
{
	ASSERT(prs);
	while(!prs->eof) {
		// What field are we on?
		long nTemplateFieldID = VarLong(prs->Fields->Item["TemplateFieldID"]->Value);
		CLabCustomFieldPtr pField = m_mapTemplateFields[nTemplateFieldID]->GetField();
		// Ok, so now determine if this is a list field
		if(pField->GetFieldType() == lcftSingleSelectList) {
			CLabCustomListFieldPtr pListField = boost::dynamic_pointer_cast<CLabCustomListField>(pField);
			if(pListField == NULL) {
				// You shouldn't be able to get here since if the field has a datatype of list, a list field is created for it right above.
				ThrowNxException("CLabProcCFTemplate::LoadListFieldItems : Could not cast field as a list field.");
			}
			
			// It is. So load all list item related values.
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
				case lcftMemo:
					varValue = prs->Fields->Item["MemoParam"]->Value;
					break;
				default:
					// We attempted to load a type but we don't know what this type is.
					// Either this is bad data or somebody needs to add a handler here.
					prs->Close();
					ASSERT(FALSE);
					ThrowNxException("CLabProcCFTemplate::LoadListFieldItems : Attempted to load a list field item with an unknown type.");
			}

			CString strDescription = VarString(prs->Fields->Item["Description"]->Value, "");

			// Throw the item into the list field.
			pListField->TrackChanges(false);
			CLabCustomListFieldItem *pItem = pListField->AddItem(nID, varValue, lcftType, strDescription, nSortOrder);
			ASSERT(pItem != NULL);
			// (r.gonet 09/21/2011) - PLID 45558 - Init the deleted flag for the list item. "Does this item show up on the most up to date list field?"
			pItem->SetDeleted(VarBool(prs->Fields->Item["ListItemDeleted"]->Value) != FALSE);
			// (r.gonet 09/21/2011) - PLID 45557 - Init the item's GUID
			pItem->SetGUID(VarString(prs->Fields->Item["UID"]->Value));
			pListField->TrackChanges(true);
		}
		
		prs->MoveNext();
	}
}

// (r.gonet 09/21/2011) - PLID 45586 - Commit the template's changes to the database.
void CLabProcCFTemplate::Save()
{
	// Save the template itself.
	CParamSqlBatch sqlBatch;
	if(m_emsModificationStatus == emsNew) {
		// (r.gonet 09/21/2011) - PLID 45557 - Insert the GUID that we generated when creating this template.
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON; "
			"INSERT INTO LabProcCFTemplatesT (Name, UID) VALUES ({STRING}, {STRING}); "
			"SET NOCOUNT OFF; "
			"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS NewID; ",
			m_strName, m_strGUID);
		m_nID = VarLong(prs->Fields->Item["NewID"]->Value);
		prs->Close();
	} else if(m_emsModificationStatus == emsModified) {
		sqlBatch.Add(
			"UPDATE LabProcCFTemplatesT "
			"SET Name = {STRING} "
			"WHERE ID = {INT}; ",
			m_strName,
			m_nID);
	} else if(m_emsModificationStatus == emsDeleted) {
		std::map<long, CCFTemplateFieldPtr>::iterator iter;
		for(iter = m_mapTemplateFields.begin(); iter != m_mapTemplateFields.end(); iter++) {
			iter->second->SetModificationStatus(CCFTemplateField::emsDeleted);
			m_aryRemovedFields.Add(iter->second);
		}
		m_mapTemplateFields.clear();

		sqlBatch.Add(
			"UPDATE LabProceduresT SET LabProcCFTemplateID = NULL WHERE LabProcCFTemplateID = {INT}; ",
			m_nID);
		// (r.gonet 09/21/2011) - PLID 45558 - Mark it deleted. Don't actually delete.
		sqlBatch.Add(
			"UPDATE LabProcCFTemplatesT "
			"SET Deleted = 1 "
			"WHERE ID = {INT}; ",
			m_nID);
		// (r.gonet 09/21/2011) - PLID 45558 - Might as well update the in-memory object as well
		m_bDeleted = true;
	}

	// Copy our fields into temporary map because we may end up changing the hashing index of some of the fields
	//  in the main map.
	std::map<long, CCFTemplateFieldPtr> mapTempTemplateFields(m_mapTemplateFields.begin(), m_mapTemplateFields.end());

	// Save the template fields
	std::map<long, CCFTemplateFieldPtr>::iterator iter;
	for(iter = mapTempTemplateFields.begin(); iter != mapTempTemplateFields.end(); iter++) {
		CCFTemplateFieldPtr pTemplateField = iter->second;
		long nOldID = pTemplateField->GetID();
		pTemplateField->Save(sqlBatch);
		long nNewID = pTemplateField->GetID();

		// Check if the ID was finalized. If so, we need to update the hash position.
		if(nOldID != nNewID) {
			m_mapTemplateFields.erase(nOldID);
			m_mapTemplateFields[nNewID] = pTemplateField;
		}
	}

	// Go through the deleted fields and save (delete) those as well
	for(int i = 0; i < m_aryRemovedFields.GetSize(); i++) {
		m_aryRemovedFields[i]->Save(sqlBatch);
	}

	sqlBatch.Execute(GetRemoteData());

	// We have now saved, so reset our modification status.
	m_emsModificationStatus = emsNone;
}

// (r.gonet 09/21/2011) - PLID 45586 - Gets a template field contained in the template by ID. If the template field does not exist
//  in the template then this method returns a NULL shared pointer (NULL essentially).
// - nTemplateID is the ID of the template field to retrieve.
CCFTemplateFieldPtr CLabProcCFTemplate::GetTemplateField(long nTemplateFieldID)
{
	std::map<long, CCFTemplateFieldPtr>::iterator iter;
	if((iter = m_mapTemplateFields.find(nTemplateFieldID)) != m_mapTemplateFields.end()) {
		return iter->second;
	} else {
		// Return null
		return CCFTemplateFieldPtr();
	}
}

// (r.gonet 09/21/2011) - PLID 45586 - Sets the name of the template that will be visible on the interface
// - strName is the name of the template
void CLabProcCFTemplate::SetName(CString strName)
{
	m_strName = strName;
	if(m_emsModificationStatus == emsNone) {
		m_emsModificationStatus = emsModified;
	}
}

// (r.gonet 09/21/2011) - PLID 45586 - Adds a template field to this template.
// - pTemplateField is the template field to add. If it's ID is negative and thus invalid, it is made
//  unique.
void CLabProcCFTemplate::AddField(CCFTemplateFieldPtr pTemplateField)
{
	if(pTemplateField) {
		// We must check the ID. If it is negative, then it must be set to a unique value.
		if(pTemplateField->GetID() < 0) {
			pTemplateField->SetID(m_nNextFieldID--);
		}

		// Find the next sort order
		long nMaxSortOrder = 0;
		std::map<long, CCFTemplateFieldPtr>::iterator iter;
		for(iter = m_mapTemplateFields.begin(); iter != m_mapTemplateFields.end(); iter++) {
			nMaxSortOrder = max(iter->second->GetSortOrder(), nMaxSortOrder);
		}
		pTemplateField->SetSortOrder(nMaxSortOrder + 1);
		m_mapTemplateFields[pTemplateField->GetID()] = pTemplateField;
		if(m_emsModificationStatus == emsNone) {
			m_emsModificationStatus = emsModified;
		}
	} else {
		AfxThrowNxException("CLabProcCFTemplate::AddField : Attempted to insert NULL into the custom fields list.");
	}
}

// (r.gonet 09/21/2011) - PLID 45586 - Replaces a custom field on the template with a different one.
// - nTemplateFieldID is the ID of the template that wraps the field to replace.
// - pNewField is the field to replace with.
void CLabProcCFTemplate::ReplaceField(long nTemplateFieldID, CLabCustomFieldPtr pNewField)
{
	if(pNewField) {
		CCFTemplateFieldPtr pTemplateField = m_mapTemplateFields[nTemplateFieldID];
		pTemplateField->SetField(pNewField);
	} else {
		AfxThrowNxException("CLabProcCFTemplate::ReplaceField : Attempted to replace a field with NULL.");
	}
}

// (r.gonet 09/21/2011) - PLID 45586 - Removes a template field from the template by template field ID. Will delete it once the template is saved.
void CLabProcCFTemplate::RemoveField(long nTemplateFieldID)
{
	// Just remove the field from the template, don't delete it, since fields can exist independent of templates.
	if(m_emsModificationStatus == emsNone) {
		m_emsModificationStatus = emsModified;
	}

	// We will need to delete the link between the field and the template though
	std::map<long, CCFTemplateFieldPtr>::iterator templateFieldIter = m_mapTemplateFields.find(nTemplateFieldID);
	CCFTemplateFieldPtr pTemplateField = templateFieldIter->second;
	m_mapTemplateFields.erase(templateFieldIter);
	pTemplateField->SetModificationStatus(CCFTemplateField::emsDeleted);
	m_aryRemovedFields.Add(pTemplateField);
}

// Global function that compares two lab custom field template fields. 
//  Returns -1 if pA < pB. Returns 0 if pA == pB. Returns 1 if pA > pB.
// - pA is a void pointer to a template field
// - pB is a void pointer to a second template field
int CompareTemplateFieldRecords(const void * pA, const void * pB)
{
	CCFTemplateFieldPtr *ppTemplateFieldA = (CCFTemplateFieldPtr *)pA;
	CCFTemplateFieldPtr *ppTemplateFieldB = (CCFTemplateFieldPtr *)pB;
	ASSERT(ppTemplateFieldA && ppTemplateFieldB && *ppTemplateFieldA && *ppTemplateFieldB);
	int nResult = (*ppTemplateFieldA)->GetSortOrder() < (*ppTemplateFieldB)->GetSortOrder() ? -1 : ((*ppTemplateFieldA)->GetSortOrder() > (*ppTemplateFieldB)->GetSortOrder() ? 1 : 0);
	return nResult;
}

// Draws all of the controls of the fields on this template to the screen. Does not draw deleted template fields.
// - cmControlManager is the object monopolizing and managing custom field drawing on the screen.
void CLabProcCFTemplate::CreateControls(CLabCustomFieldControlManager &cmControlManager)
{
	// We need to sort these in the order specified by the sortorder field
	long nFieldCount = m_mapTemplateFields.size();
	CCFTemplateFieldPtr *arypTemplateFields = new CCFTemplateFieldPtr[nFieldCount];
	int nIndex = 0;
	std::map<long, CCFTemplateFieldPtr>::iterator iter;
	for(iter = m_mapTemplateFields.begin(); iter != m_mapTemplateFields.end(); iter++) {
		CCFTemplateFieldPtr pTemplateField = iter->second;
		arypTemplateFields[nIndex++] = pTemplateField;
	}
	qsort(arypTemplateFields, nFieldCount, sizeof(CCFTemplateFieldPtr), CompareTemplateFieldRecords);

	// Now draw each of the sorted template fields
	for(int i = 0; i < nFieldCount; i++) {
		ASSERT(arypTemplateFields[i] != NULL);
		CCFTemplateFieldPtr pTemplateField = arypTemplateFields[i];
		// (r.gonet 09/21/2011) - PLID 45558 - Since templates draw only the most up to date fields, we don't want deleted fields.
		if(!pTemplateField->GetDeleted() && pTemplateField->GetModificationStatus() != CCFTemplateField::emsDeleted) {
			// Don't draw controls for deleted template fields.
			pTemplateField->GetField()->CreateControls(cmControlManager, NULL);
		}
	}

	delete [] arypTemplateFields;
}

// (r.gonet 09/21/2011) - PLID 45558 - Gets whether this template has been deleted in the database. Returns true if it has been deleted and false otherwise.
bool CLabProcCFTemplate::GetDeleted() const
{
	return m_bDeleted;
}

// (r.gonet 09/21/2011) - PLID 45558 - Sets whether this template has been deleted in the database. Really only for initialization, since the method has no action behind it.
// - bDeleted is true if the template has been deleted and false otherwise.
void CLabProcCFTemplate::SetDeleted(bool bDeleted)
{
	m_bDeleted = bDeleted;
}

// (r.gonet 09/21/2011) - PLID 45586 - Gets an iterator pointing to the first template field in the template.
std::map<long, CCFTemplateFieldPtr>::iterator CLabProcCFTemplate::GetBeginIterator()
{
	return m_mapTemplateFields.begin();
}

// (r.gonet 09/21/2011) - PLID 45586 - Gets an iterator pointing to just beyond the last template field in the template.
std::map<long, CCFTemplateFieldPtr>::iterator CLabProcCFTemplate::GetEndIterator()
{
	return m_mapTemplateFields.end();
}

////////////////////////////////////////////////////////////////////////////////
// Custom Fields Template Instance
////////////////////////////////////////////////////////////////////////////////

// (r.gonet 09/21/2011) - PLID 45124 - Creates a new custom fields template instance which is an instantiation of a custom fields template.
//  It can hold instances of custom fields, which may in turn hold user entered values for the fields.
CCFTemplateInstance::CCFTemplateInstance()
{
	m_nID = -1;
	m_emsModificationStatus = emsNew;
}

// (r.gonet 09/21/2011) - PLID 45124 - Creates a new custom fields template based on a template. This instance is then made to be an instance
//  of that template. All custom fields referenced on the template will be instantiated themselves when this
//  constructor is called.
// - pTemplate is the custom fields template to create an instance of.
CCFTemplateInstance::CCFTemplateInstance(CLabProcCFTemplatePtr pTemplate)
{
	m_nID = -1;
	m_pTemplate = pTemplate;
	m_emsModificationStatus = emsNew;

	// Create an instance for each of our template's fields.
	std::map<long, CCFTemplateFieldPtr>::iterator templateFieldIter;
	for(templateFieldIter = pTemplate->GetBeginIterator(); templateFieldIter != pTemplate->GetEndIterator(); templateFieldIter++) {
		CCFTemplateFieldPtr pTemplateField = templateFieldIter->second;
		// (r.gonet 09/21/2011) - PLID 45558 - Get all but the deleted template fields. We are a NEW instance, not a historical one.
		if(!pTemplateField->GetDeleted()) {
			CLabCustomFieldInstance *pFieldInstance = new CLabCustomFieldInstance(pTemplateField, this);
			this->AddFieldInstance(pFieldInstance);
		}
	}
}

// (r.gonet 09/21/2011) - PLID 45124 - Destructor for the template instance. Frees all field instances.
CCFTemplateInstance::~CCFTemplateInstance()
{
	std::map<long, CLabCustomFieldInstance *>::iterator iter;
	for(iter = m_mapFieldInstances.begin(); iter != m_mapFieldInstances.end(); iter++) {
		delete iter->second;
	}
	m_mapFieldInstances.clear();
}

// (r.gonet 09/21/2011) - PLID 45586 - Gets the database ID of this template instance. -1 if not saved yet.
long CCFTemplateInstance::GetID() const
{
	return m_nID;
}

// (r.gonet 09/21/2011) - PLID 45124 - Gets the number of instantiated fields in this template instance.
long CCFTemplateInstance::GetFieldInstanceCount() const
{
	return m_mapFieldInstances.size();
}

// (r.gonet 09/21/2011) - PLID 45124 - Loads a specific historical/existing template instance from the database.
// - nTemplateInstanceID is the database ID of the template instance to load. It must be valid within the context of the database.
void CCFTemplateInstance::Load(long nTemplateInstanceID)
{
	CParamSqlBatch sqlLoadBatch;
	// First we need the template, so go get it
	sqlLoadBatch.Add(
		"SELECT LabProcCFTemplateID "
		"FROM CFTemplateInstancesT "
		"WHERE ID = {INT}; ",
		nTemplateInstanceID);

	// Now get the field instances we own
	sqlLoadBatch.Add(
		"SELECT LabCustomFieldInstancesT.ID, LabCustomFieldsT.ID AS FieldID, LabCustomFieldsT.DataTypeID, "
		"	LabCustomFieldInstancesT.SelectionID, LabCustomFieldInstancesT.IntParam, LabCustomFieldInstancesT.FloatParam, "
		"	LabCustomFieldInstancesT.TextParam, LabCustomFieldInstancesT.MemoParam, LabCustomFieldInstancesT.DateTimeParam, "
		"	LabProcCFTemplateFieldsT.ID AS TemplateFieldID "
		"FROM CFTemplateInstancesT "
		"	INNER JOIN "
		"LabCustomFieldInstancesT ON CFTemplateInstancesT.ID = LabCustomFieldInstancesT.CFTemplateInstanceID "
		"	INNER JOIN "
		"LabProcCFTemplateFieldsT ON LabProcCFTemplateFieldsT.ID = LabCustomFieldInstancesT.TemplateFieldID "
		"	INNER JOIN "
		"LabCustomFieldsT ON LabCustomFieldInstancesT.FieldID = LabCustomFieldsT.ID "
		"WHERE CFTemplateInstancesT.ID = {INT}; ",
		nTemplateInstanceID);

	_RecordsetPtr prs = sqlLoadBatch.CreateRecordset(GetRemoteData());
	if(prs->eof) {
		prs->Close();
		ThrowNxException("CCFTemplateInstance::Load : Attempted to load a non-existent Custom Fields Template Instance.");
	} else {
		// Ok, our instance exists. Let's set our properties and also ensure the template we are instantiated from is created.
		m_nID = nTemplateInstanceID;
		long nTemplateID = VarLong(prs->Fields->Item["LabProcCFTemplateID"]->Value);
		m_pTemplate = make_shared<CLabProcCFTemplate>();
		m_pTemplate->Load(nTemplateID);
	}
	
	prs = prs->NextRecordset(NULL);

	// Fill in the field instances we own with values and properties
	while(!prs->eof) {
		CLabCustomFieldInstance *pFieldInstance = LoadFieldInstance(prs);
		m_mapFieldInstances[pFieldInstance->GetTemplateField()->GetField()->GetID()] = pFieldInstance;
		prs->MoveNext();
	}
	prs->Close();

	// This template instance has been in existence already, so no modifications took place technically.
	m_emsModificationStatus = emsNone;
}

// (r.gonet 09/21/2011) - PLID 45124 - Loads a single existing instance of a field from a recordset. Returns that field instance.
// - prs is the recordset containing the field instance values. No records are eaten.
CLabCustomFieldInstance * CCFTemplateInstance::LoadFieldInstance(_RecordsetPtr prs)
{
	// When creating the instance, get the template field from the template. This may get a deleted template field,
	//  but that is intended to cope with historical data.
	long nInstanceID = VarLong(prs->Fields->Item["ID"]->Value);
	long nTemplateFieldID = VarLong(prs->Fields->Item["TemplateFieldID"]->Value);
	CCFTemplateFieldPtr pTemplateField = m_pTemplate->GetTemplateField(nTemplateFieldID);
	if(!pTemplateField) {
		// This should be impossible since we have a foreign key relationship.
		ASSERT(FALSE);
		prs->Close();
		ThrowNxException("CCFTemplateInstance::LoadFieldInstance : Attempted to load a non-existent field.");
	}
	CLabCustomFieldInstance *pFieldInstance = new CLabCustomFieldInstance(nInstanceID, pTemplateField, this);

	// Now that we have a field instance, we need to get its value, if it has anything set.
	ELabCustomFieldType lcftType = (ELabCustomFieldType)VarLong(prs->Fields->Item["DataTypeID"]->Value, -1);			
	_variant_t varValue;
	switch(lcftType) {
		// (r.gonet 09/21/2011) - PLID 45581 - Added support for loading boolean values.
		case lcftBool:
			varValue = VarLong(prs->Fields->Item["IntParam"]->Value, 0) != 0 ? g_cvarTrue : g_cvarFalse;
			break;
		// (r.gonet 09/21/2011) - PLID 45581 - Added support for loading integer values.
		case lcftInteger:
			varValue = prs->Fields->Item["IntParam"]->Value;
			break;
		// (r.gonet 09/21/2011) - PLID 45581 - Added support for loading float values.
		case lcftFloat:
			varValue = prs->Fields->Item["FloatParam"]->Value;
			break;
		// (r.gonet 09/21/2011) - PLID 45581 - Added support for loading datetime values.
		case lcftDateTime:
			varValue = prs->Fields->Item["DateTimeParam"]->Value;
			break;
		// (r.gonet 09/21/2011) - PLID 45582 - Added support for loading text values.
		case lcftText:
			varValue = prs->Fields->Item["TextParam"]->Value;
			break;
		// (r.gonet 09/21/2011) - PLID 45582 - Added support for loading memo values.
		case lcftMemo:
			varValue = prs->Fields->Item["MemoParam"]->Value;
			break;
		// (r.gonet 09/21/2011) - PLID 45583 - Added support for list fields
		case lcftSingleSelectList:
			varValue = prs->Fields->Item["SelectionID"]->Value;
			break;
		default:
			// We attempted to load a type but we don't know what this type is.
			// Either this is bad data or somebody needs to add a handler here.
			ASSERT(FALSE);
			prs->Close();
			ThrowNxException("CCFTemplateInstance::LoadFieldInstance : Attempted to load a field instance with an unknown data type.");
	}
	pFieldInstance->SetValue(varValue);
	
	// Since this is an existing field instance, all we have done is just initialize the instance, not modified it.
	pFieldInstance->SetModificationStatus(CLabCustomFieldInstance::emsNone);
	return pFieldInstance;
}

// (r.gonet 09/21/2011) - PLID 45124 - Loads this template instance's values by retrieving the instance already associated with an existing lab.
bool CCFTemplateInstance::LoadByLabID(long nLabID)
{
	if(nLabID <= 0) {
		// The lab must exist if we are to load an existing custom fields template instance. This one doesn't.
		return false;
	}

	// Find this lab's template instance, if it has one.
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT LabsT.CFTemplateInstanceID "
		"FROM LabsT "
		"WHERE LabsT.ID = {INT} AND LabsT.CFTemplateInstanceID IS NOT NULL",
		nLabID);
	if(prs->eof) {
		// This lab doesn't have a custom fields template instance, so quit.
		return false;
	} else {
		// Ok, we now know the ID of the template instance we need, so go load it.
		long nTemplateInstanceID = VarLong(prs->Fields->Item["CFTemplateInstanceID"]->Value);
		this->Load(nTemplateInstanceID);
		return true;
	}
}

// (r.gonet 09/21/2011) - PLID 45124 - Commit this template instance to the database. Resets its modification status.
bool CCFTemplateInstance::Save()
{
	if(m_pTemplate == NULL) {
		// It is invalid to try to save a template instance without a corresponding template
		ThrowNxException("CCFTemplateInstance::Save : Attempted to save a template instance without a corresponding template.");
	}

	// Save a new instance
	if(m_emsModificationStatus == emsNew) {
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON; "
			"INSERT INTO CFTemplateInstancesT (LabProcCFTemplateID) VALUES ({INT}); "
			"SET NOCOUNT OFF; "
			"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS NewID; ",
			m_pTemplate->GetID());
		m_nID = VarLong(prs->Fields->Item["NewID"]->Value);
		prs->Close();

		// Reset the modification status so we don't insert another record.
		SetModificationStatus(emsNone);
	}

	// We really can't be modified other than created so there is nothing to UPDATE or DELETE

	CParamSqlBatch sqlSaveBatch;
	// Go through each of the fields and see if they are new, have changed, or have been deleted
	std::map<long, CLabCustomFieldInstance *>::iterator iter;
	for(iter = m_mapFieldInstances.begin(); iter != m_mapFieldInstances.end(); iter++) {
		iter->second->Save(sqlSaveBatch);
	}

	// Don't make another round trip if there is nothing to update.
	if(!sqlSaveBatch.IsEmpty()) {
		sqlSaveBatch.Execute(GetRemoteData());
	}

	// Reset the modification status of the field instances now that we know the save went through correctly.
	for(iter = m_mapFieldInstances.begin(); iter != m_mapFieldInstances.end(); iter++) {
		iter->second->SetModificationStatus(CLabCustomFieldInstance::emsNone);
	}

	return true;
}

// (r.gonet 09/21/2011) - PLID 45124 - Adds a new field instance to this template
// - pFieldInstance is the field instance to add to our control.
void CCFTemplateInstance::AddFieldInstance(CLabCustomFieldInstance *pFieldInstance)
{
	ASSERT(pFieldInstance != NULL && pFieldInstance->GetTemplateField() && pFieldInstance->GetTemplateField()->GetField() != NULL);
	m_mapFieldInstances[pFieldInstance->GetTemplateField()->GetField()->GetID()] = pFieldInstance;
}

// (r.gonet 09/21/2011) - PLID 45124 - Sets the modification status for this instance. This value tracks modifications for this object and affects how it is saved.
void CCFTemplateInstance::SetModificationStatus(EModificationStatus emsModificationStatus)
{
	m_emsModificationStatus = emsModificationStatus;
}

// (r.gonet 09/21/2011) - PLID 45124 - Gets the template we are an instance of.
CLabProcCFTemplatePtr CCFTemplateInstance::GetTemplate()
{
	return m_pTemplate;
}

// Draws all of the controls of the fields on this template instance to the screen.
// - cmControlManager is the object monopolizing and managing custom field drawing on the screen.
void CCFTemplateInstance::CreateControls(CLabCustomFieldControlManager &cmControlManager)
{
	// We need to sort these in the order specified by the sortorder field
	long nFieldCount = this->GetFieldInstanceCount();
	CCFTemplateFieldPtr *arypTemplateFields = new CCFTemplateFieldPtr[nFieldCount];
	int nIndex = 0;
	std::map<long, CLabCustomFieldInstance *>::iterator iter;
	for(iter = m_mapFieldInstances.begin(); iter != m_mapFieldInstances.end(); iter++) {
		CLabCustomFieldInstance *pFieldInstance = iter->second;
		CCFTemplateFieldPtr pTemplateField = pFieldInstance->GetTemplateField();
		arypTemplateFields[nIndex++] = pTemplateField;
	}
	qsort(arypTemplateFields, this->GetFieldInstanceCount(), sizeof(CCFTemplateFieldPtr), CompareTemplateFieldRecords);

	// Now draw each of the sorted template fields
	for(int i = 0; i < nFieldCount; i++) {
		ASSERT(arypTemplateFields[i] != NULL);
		CCFTemplateFieldPtr pTemplateField = arypTemplateFields[i];
		if(pTemplateField->GetModificationStatus() == CCFTemplateField::emsDeleted) {
			// Don't draw controls for deleted template fields.
			continue;
		}
		CLabCustomFieldPtr pField = arypTemplateFields[i]->GetField();
		if(pField) {
			CLabCustomFieldInstance *pFieldInstance = m_mapFieldInstances[pField->GetID()];
			pField->CreateControls(cmControlManager, pFieldInstance);
		} else {
			// It should not be possible to have a template field without a corresponding custom field.
			ASSERT(FALSE);
		}
	}

	delete [] arypTemplateFields;
}

// (r.gonet 09/21/2011) - PLID 45124 - Gets the number of fields that have not been filled in but are marked as being required.
//  Blank values are treated as null and not filled.
long CCFTemplateInstance::GetUnsetRequiredFieldCount()
{
	long nCount = 0;
	std::map<long, CLabCustomFieldInstance *>::iterator iter;
	for(iter = m_mapFieldInstances.begin(); iter != m_mapFieldInstances.end(); iter++) {
		CLabCustomFieldInstance *pFieldInstance = iter->second;
		CCFTemplateFieldPtr pTemplateField = pFieldInstance->GetTemplateField();
		ASSERT(pTemplateField);
		if(pTemplateField->GetRequired() && pFieldInstance->GetValue().vt == VT_NULL) {
			nCount++;
		}
	}

	return nCount;
}

void CCFTemplateInstance::GetUnsetRequiredFields(OUT CArray<CLabCustomFieldPtr, CLabCustomFieldPtr> &aryFields)
{
	std::map<long, CLabCustomFieldInstance *>::iterator iter;
	for(iter = m_mapFieldInstances.begin(); iter != m_mapFieldInstances.end(); iter++) {
		CLabCustomFieldInstance *pFieldInstance = iter->second;
		CCFTemplateFieldPtr pTemplateField = pFieldInstance->GetTemplateField();
		ASSERT(pTemplateField);
		if(pTemplateField->GetRequired() && pFieldInstance->GetValue().vt == VT_NULL) {
			CLabCustomFieldPtr pField = pTemplateField->GetField();
			ASSERT(pField);
			aryFields.Add(pField);
		}
	}
}

// (r.gonet 10/07/2011) - PLID 44212 - Required for lab barcode merging of the custom fields
CLabCustomFieldInstance *CCFTemplateInstance::GetFieldInstanceByFieldID(long nFieldID)
{
	std::map<long, CLabCustomFieldInstance *>::iterator findIter = m_mapFieldInstances.find(nFieldID);
	if(findIter == m_mapFieldInstances.end()) {
		return NULL;
	} else {
		return findIter->second;
	}
}