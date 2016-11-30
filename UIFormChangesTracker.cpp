// (c.haag 2010-10-13 13:00) - PLID 40637 - This class is designed for the developer
// to easily track changes in a form that apply to one or more auto-numbered tables,
// and auto-generate the SQL needed to update the data.
#include "stdafx.h"
#include "UIFormChangesTracker.h"

// Removes all records from an array in the changes tracker
void CUIFormChangesTracker::ClearRecords(CArray<Record*,Record*>& aRecords)
{
	for (int i=0; i < aRecords.GetSize(); i++) {
		delete aRecords[i];
	}
	aRecords.RemoveAll();
}

// Adds a SQL statement to a batch
void CUIFormChangesTracker::AddToSqlBatch(CString& strSqlBatch, const CString& strSql)
{
	// (a.walling 2010-10-21 15:00) - PLID 34813 - Changing the way we use batches somewhat; this is duplicated code anyway.
	AddStatementToSqlBatch(strSqlBatch, "%s", strSql);
}

// Calculates and returns the SQL statement for creating all new records
CString CUIFormChangesTracker::GetCreateSql()
{
	CString strSql;
	for (int i=0; i < m_CreatedRecords.GetSize(); i++) {
		AddToSqlBatch(strSql, m_CreatedRecords[i]->GetCreateSql(m_strTableName));
	}
	return strSql;
}

// Calculates and returns the SQL statement for modifying existing records
CString CUIFormChangesTracker::GetModifySql()
{
	CString strSql;
	for (int i=0; i < m_ModifiedRecords.GetSize(); i++) {
		AddToSqlBatch(strSql, m_ModifiedRecords[i]->GetModifySql(m_strTableName));
	}
	return strSql;
}

// Calculates and returns the SQL statement for deleting existing records
CString CUIFormChangesTracker::GetDeleteSql()
{
	CString strSql;
	for (int i=0; i < m_DeletedRecords.GetSize(); i++) {
		AddToSqlBatch(strSql, Record::GetDeleteSql(m_strTableName, m_DeletedRecords[i]));
	}
	return strSql;
}

// This function should be called when the caller desires to add a new record
// to the database upon saving. Returns a placeholder ID for future reference.
long CUIFormChangesTracker::SetCreated(const CStringArray& astrFieldNames, const CArray<_variant_t,_variant_t&>& aValues)
{
	Record* p = new Record;
	p->nID = m_nNextID--;
	for (int i=0; i < aValues.GetSize(); i++) {
		p->SetValue(FieldItem(astrFieldNames[i], aValues[i]));
	}
	m_CreatedRecords.Add(p);
	return p->nID;
}

// This function should be called when the caller wants to modify an existing
// record in the database upon saving. If the record was already used in a prior
// call to this function, then any non-existent fields are added, and existing fields
// are overwritten.
void CUIFormChangesTracker::SetModified(long nRecordID, const CString& strFieldName, const _variant_t& value)
{
	int i;

	// We can't modify a record after it has been deleted
	for (i=0; i < m_DeletedRecords.GetSize(); i++) {
		if (m_DeletedRecords[i] == nRecordID) {
			ThrowNxException("Attempted to modify a deleted record!");
		}
	}

	// If this record doesn't exist in data, look to our created records heap
	if (nRecordID <= m_nFirstNewID)
	{
		for (i=0; i < m_CreatedRecords.GetSize(); i++) {
			if (m_CreatedRecords[i]->nID == nRecordID) {
				// Found it. Update the values.
				m_CreatedRecords[i]->SetValue(FieldItem(strFieldName,value));
				return;
			}
		}
		// It's a new ID but it doesn't exist in the created records table. Should not
		// be possible!
		ThrowNxException("Attempted to modify non-existent record!");
	}
	else {
		// If the record is already in data, then update it in our modified record heap
		for (i=0; i < m_ModifiedRecords.GetSize(); i++) {
			if (m_ModifiedRecords[i]->nID == nRecordID) {
				m_ModifiedRecords[i]->SetValue(FieldItem(strFieldName,value));
				return;
			}
		}
		// If we get here, it doesn't exist in m_ModifiedRecords. Add it now.
		Record* p = new Record;
		p->nID = nRecordID;
		p->SetValue(FieldItem(strFieldName,value));
		m_ModifiedRecords.Add(p);
	}		
}

// This function should be called when the caller wants to delete an
// existing record in the database upon saving.
void CUIFormChangesTracker::SetDeleted(long nRecordID)
{
	// If this is a new record, remove it from the created list. Since it was never
	// in data, there's nothing more to do.
	if (nRecordID <= m_nFirstNewID)
	{
		for (int i=0; i < m_CreatedRecords.GetSize(); i++) {
			if (m_CreatedRecords[i]->nID == nRecordID) {
				delete m_CreatedRecords[i];
				m_CreatedRecords.RemoveAt(i);
				return;
			}
		}
	}
	else {
		int i;
		// If this was modified earlier on before it was deleted, then
		// remove the corresponding entry from the modified array.
		// The earlier modifications are moot now.
		for (i=0; i < m_ModifiedRecords.GetSize(); i++) {
			if (m_ModifiedRecords[i]->nID == nRecordID) {
				delete m_ModifiedRecords[i];
				m_ModifiedRecords.RemoveAt(i);
				break;
			}
		}
		for (i=0; i < m_DeletedRecords.GetSize(); i++) {
			if (m_DeletedRecords[i] == nRecordID) {
				return;
			}
		}
		m_DeletedRecords.Add(nRecordID);
	}
}

// This function will generate and return the complete SQL statement for applying all changes
// to data
CString CUIFormChangesTracker::GenerateUpdateSql()
{
	CString strBatch = GetCreateSql() + GetModifySql() + GetDeleteSql();
	if (!strBatch.IsEmpty()) {
		// (a.walling 2010-10-21 15:00) - PLID 34813 - Changing the way we use batches somewhat; this is duplicated code anyway.
		return TransactionUtils::Wrap(strBatch);
	} else {
		return "";
	}
}

// This function will generate and run the query to update the data
void CUIFormChangesTracker::DoDatabaseUpdate()
{
	// Generate the save SQL
	const CString strSql = GenerateUpdateSql();
	// Do the save
	if (!strSql.IsEmpty()) {
		ExecuteSqlStd(strSql);
	}
}