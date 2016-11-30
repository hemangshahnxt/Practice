#pragma once

#include "LabCustomField.h"
#include "LabCustomFieldControlManager.h"

class CLabCustomFieldInstance;
class CLabCustomFieldControlManager;
typedef boost::shared_ptr<class CLabProcCFTemplate> CLabProcCFTemplatePtr;
typedef boost::shared_ptr<class CLabCustomField> CLabCustomFieldPtr;

// (r.gonet 09/21/2011) - PLID 45586 - A CCFTemplateField (template field) is a wrapper for a CLabCustomField (lab custom field) class. 
//  While the CLabCustomFieldClass's values are universal, the CCFTemplateField's additional values 
//  are specific for a certain template. A CLabProcCFTemplate (lab custom fields template) may contain multiple
//  template fields. The field is an active record for the LabProcCFTemplateFieldsT table.
class CCFTemplateField
{
public:
	// The modification status type for this template field
	enum EModificationStatus
	{
		emsNone, // No mods. Won't be saved.
		emsNew, // Is a new template field. Will be inserted.
		emsDeleted, // Is marked to be deleted. Will be marked deleted in the database.
		emsModified, // Has been modified. Will be updated.
	};
private:
	// (r.gonet 09/21/2011) - PLID 45586 - The database ID. References LabProcCFTemplateFieldsT(ID). Can be negative if not saved yet.
	long m_nID;
	// (r.gonet 09/21/2011) - PLID 45586 - The lab custom field that this class wraps. Should not be null unless we are initializing this object.
	CLabCustomFieldPtr m_pField;
	// (r.gonet 09/21/2011) - PLID 45586 - The custom fields template this template field is part of.
	weak_ptr<CLabProcCFTemplate> m_pTemplate;
	// (r.gonet 09/21/2011) - PLID 45586 - The order in which this field appears on the template. Ascending. Not guaranteed to be continuous.
	long m_nSortOrder;
	// (r.gonet 09/21/2011) - PLID 45586 - Whether this field is required to be filled by the user or not when a template instance is saved.
	bool m_bRequired;
	// (r.gonet 09/21/2011) - PLID 45586 - Whether the field has been removed from its associated template. We are then marked deleted rather than actually deleted so we don't lose historical data.
	bool m_bDeleted;
	// (r.gonet 09/21/2011) - PLID 45586 - Modification status of this template field.
	EModificationStatus m_emsModificationStatus;
public:
	CCFTemplateField(CLabCustomFieldPtr pField, CLabProcCFTemplatePtr pTemplate);
	CCFTemplateField(long nID, CLabCustomFieldPtr pField, CLabProcCFTemplatePtr pTemplate);

	EModificationStatus GetModificationStatus() const;
	void SetModificationStatus(EModificationStatus emsModificationStatus);
	long GetID() const;
	void SetID(long nID);
	CLabCustomFieldPtr GetField() const;
	void SetField(CLabCustomFieldPtr pField);
	CLabProcCFTemplatePtr GetTemplate() const;
	long GetSortOrder() const;
	void SetSortOrder(long nSortOrder);
	bool GetRequired() const;
	void SetRequired(bool bRequired);
	bool GetDeleted() const;
	void SetDeleted(bool bDeleted);

	void Save(CParamSqlBatch &sqlBatch);
};

typedef boost::shared_ptr<CCFTemplateField> CCFTemplateFieldPtr;

// (r.gonet 09/21/2011) - PLID 45586 - A CLabProcCFTemplate (lab custom fields template) is a class that stores 
//  and defines what CLabCustomField's (lab custom fields) 
//  (through CCFTemplateField wrappers) will go in its collection. 
//  Can be instantiated as a CCFTemplateInstance (custom fields template instance).
//  Can be drawn to the screen in the form of field controls, which may then sync
//  their values back to the fields' default values. This class is an active 
//  record for the LabProcCFTemplatesT table.
class CLabProcCFTemplate : public enable_shared_from_this<CLabProcCFTemplate>
{
public:
	// The modification status type for this template
	enum EModificationStatus
	{
		emsNone, // No mods. Won't be saved.
		emsNew, // Has just been created. Will be inserted.
		emsDeleted, // Has just been deleted. Will be marked deleted.
		emsModified, // Has been modified. Will be updated.
	};
private:
	// (r.gonet 09/21/2011) - PLID 45586 - Database ID for this template. References LabProcCFTemplatesT(ID). Negative if this template has not been saved yet.
	long m_nID;
	// (r.gonet 09/21/2011) - PLID 45586 - Human readable name of the template.
	CString m_strName;
	// (r.gonet 09/21/2011) - PLID 45586 - The template fields this template defines. May store deleted template fields in it. A CCFTemplateField's ID maps to its object.
	std::map<long, CCFTemplateFieldPtr> m_mapTemplateFields;
	// (r.gonet 09/21/2011) - PLID 45586 - An array containing any fields that have been removed during editing of this template. They will be marked deleted come saving.
	CArray<CCFTemplateFieldPtr, CCFTemplateFieldPtr> m_aryRemovedFields;
	// (r.gonet 09/21/2011) - PLID 45586 - Whether this template has been deleted. We retain the template record though for historical value and referencing.
	bool m_bDeleted;
	// (r.gonet 09/21/2011) - PLID 45557 - A GUID unique identifier that uniquely defines this template across all databases.
	CString m_strGUID;
	// (r.gonet 09/21/2011) - PLID 45586 - The modification status of this template. Changes to it are tracked here.
	EModificationStatus m_emsModificationStatus;
	// (r.gonet 09/21/2011) - PLID 45586 - A down counter (starting at -2) that is used to set temporary IDs of newly added fields to guarantee they are unique.
	long m_nNextFieldID;

	void LoadTemplateFields(ADODB::_RecordsetPtr prs);
	void LoadListFieldItems(ADODB::_RecordsetPtr prs);

public:
	CLabProcCFTemplate();
	virtual ~CLabProcCFTemplate();

	long GetID() const;
	void SetModificationStatus(EModificationStatus emsModificationStatus);
	EModificationStatus GetModificationStatus() const;

	bool LoadByLabProcedureID(long nLabProcedureID);
	void Load(long nTemplateID);
	void Save();

	CArray<CLabCustomFieldPtr> GetSortedFields();
	CCFTemplateFieldPtr GetTemplateField(long nTemplateFieldID);
	void SetName(CString strName);
	void AddField(CCFTemplateFieldPtr pTemplateField);
	void ReplaceField(long nTemplateFieldID, CLabCustomFieldPtr pNewField);
	void RemoveField(long nTemplateFieldID);

	void CreateControls(CLabCustomFieldControlManager &cmControlManager);
	bool GetDeleted() const;
	void SetDeleted(bool bDeleted);

	std::map<long, CCFTemplateFieldPtr>::iterator GetBeginIterator();
	std::map<long, CCFTemplateFieldPtr>::iterator GetEndIterator();
};

// (r.gonet 09/21/2011) - PLID 45124 - A CCFTemplateInstance (custom fields template instance) is an instantiation of a CLabProcCFTemplate
//  (custom fields template) that stores all of the instances of the template's CLabCustomField's 
//  (lab custom fields) as CLabCustomFieldInstance's (lab custom field instances). Note that this may
//  not be a current instantiation of the template since that template's template fields may have changed
//  since this instance was created. You can draw all fields to the screen with this instance class and 
//  have the control values synced back to the CLabCustomFieldInstance values. More or less an active
//  record for the table CFTemplateInstanceT.
class CCFTemplateInstance
{
public:
	// The modification status type for the instance.
	enum EModificationStatus
	{
		emsNone, // No mods. Will not be saved.
		emsNew, // Was just created. Will be inserted.
		emsDeleted, // Was just deleted. Will be marked as deleted in the database.
		emsModified, // Was modified since being loaded. Will be updated.
	};
private:
	// (r.gonet 09/21/2011) - PLID 45124 - Database ID for this template instance. References CFTemplateInstanceT(ID). Negative if the instance has not been saved yet.
	long m_nID;
	// (r.gonet 09/21/2011) - PLID 45124 - The template this instance was instantiated from.
	CLabProcCFTemplatePtr m_pTemplate;
	// (r.gonet 09/21/2011) - PLID 45124 - Collection of lab custom field instances. A CLabCustomField's ID maps to its Instance.
	std::map<long, CLabCustomFieldInstance *> m_mapFieldInstances;
	// (r.gonet 09/21/2011) - PLID 45124 - The modification status of this instance.
	EModificationStatus m_emsModificationStatus;

	CLabCustomFieldInstance * LoadFieldInstance(ADODB::_RecordsetPtr prs);
public:
	CCFTemplateInstance();
	CCFTemplateInstance(CLabProcCFTemplatePtr pTemplate);
	~CCFTemplateInstance();

	long GetID() const;
	CLabProcCFTemplatePtr GetTemplate();
	long GetFieldInstanceCount() const;
	long GetUnsetRequiredFieldCount();
	void GetUnsetRequiredFields(OUT CArray<CLabCustomFieldPtr, CLabCustomFieldPtr> &aryFields);
	
	void Load(long nTemplateInstanceID);
	bool LoadByLabID(long nLabID);
	bool Save();

	void AddFieldInstance(CLabCustomFieldInstance *pFieldInstance);
	void SetModificationStatus(EModificationStatus emsModificationStatus);
	void CreateControls(CLabCustomFieldControlManager &cmControlManager);
	// (r.gonet 10/07/2011) - PLID 44212 - Required for lab barcode merging of the custom fields
	CLabCustomFieldInstance *GetFieldInstanceByFieldID(long nFieldID);
};