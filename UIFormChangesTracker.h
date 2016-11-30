#pragma once

// (c.haag 2010-10-13 13:00) - PLID 40637 - This class is designed for the developer
// to easily track changes in a form that apply to one or more auto-numbered tables,
// and auto-generate the SQL needed to update the data.
class CUIFormChangesTracker
{
public:
	// This structure represents a simple field name-field value pair 
	// (e.g. LineItemT.Amount / $20.00)
	struct FieldItem
	{
		// The field name
		CString strField;
		// The field value
		_variant_t value;

		FieldItem()
		{
		}
		FieldItem(const CString& strField, const _variant_t& value)
		{
			this->strField = strField;
			VariantCopy(&this->value, &value);
		}
		FieldItem(const FieldItem& item)
		{
			this->strField = item.strField;
			VariantCopy(&this->value, &item.value);
		}
	};

	// This structure represents a record in a table.
	struct Record
	{
		// The record ID
		long nID;

		// All the values in the record (this may not necessarily be all the
		// values in the actual data structure)
		CArray<FieldItem*,FieldItem*> aValues;

		Record()
		{
			nID = -1;
		}
		Record(Record& record)
		{
			nID = record.nID;
			SetValues(record.aValues);
		}
		~Record() { Clear();	}

		// Clears the record
		void Clear()
		{
			for (int i=0; i < aValues.GetSize(); i++) 
			{
				delete aValues[i];
			}
			aValues.RemoveAll();
		}

		// Assigns a value to the record
		void SetValue(FieldItem& item)
		{
			for (int i=0; i < aValues.GetSize(); i++) {
				FieldItem* f = aValues[i];
				if (f->strField == item.strField) {
					VariantCopy(&f->value, &item.value);
					return;
				}
			}
			aValues.Add(new FieldItem(item));
		}

		// Assigns multiple values to the record
		void SetValues(CArray<FieldItem*,FieldItem*>& items)
		{
			for (int i=0; i < items.GetSize(); i++) {
				if (NULL != items[i]) {
					SetValue(*items[i]);
				} else {
					ThrowNxException("A NULL value was encountered in SetValues!");
				}
			}
		}

		// Returns the query fragment for creating a new instance of this record in data
		CString GetCreateSql(const CString& strTable)
		{
			if (0 == aValues.GetSize()) {
				return "";
			}

			CString strSql = "INSERT INTO [" + strTable + "] (";
			int i;
			for (i=0; i < aValues.GetSize(); i++) 
			{
				strSql += "[" + aValues[i]->strField + "],";
			}
			strSql.TrimRight(',');
			strSql += ") VALUES (";
			for (i=0; i < aValues.GetSize(); i++)
			{	
				strSql += AsStringForSql(aValues[i]->value) + ",";
			}
			strSql.TrimRight(',');
			strSql += ")";
			return strSql;
		}

		// Returns the query fragment for modifying an existing record in data
        CString GetModifySql(const CString& strTable)
        {
			if (0 == aValues.GetSize()) {
				return "";
			}

            CString strSql = "UPDATE [" + strTable + "] SET ";
            for (int i=0; i < aValues.GetSize(); i++) 
            {
                strSql += "[" + aValues[i]->strField + "] = " + AsStringForSql(aValues[i]->value) + ", ";
            }
            strSql.TrimRight(' ');
            strSql.TrimRight(',');
            strSql += " WHERE ID = " + AsStringForSql(nID);
            return strSql;
        }

		// Returns the query fragment for deleting an existing record in data
		static CString GetDeleteSql(const CString& strTable, long nRecordID)
        {
			CString strSql;
			strSql.Format("DELETE FROM [" + strTable + "] WHERE ID = " + AsStringForSql(nRecordID));
			return strSql;
		}
	};

private:
	// The name of the table that this object is tracking changes to.
	CString m_strTableName;

	// The list of records that were created in this session. These records
    // will always have negative ID's.
	CArray<Record*,Record*> m_CreatedRecords;

    /// The list of records that were modified in this session. These records
    /// will always have non-negative ID's
	CArray<Record*,Record*> m_ModifiedRecords;

    /// The list of record ID's that were deleted in this session. These records
    /// will always have non-negative ID's
	CArray<long,long> m_DeletedRecords;

private:
	// The internal ID of the first record to be added to this table. This is a constant
	// that is used in determining whether a given record was generated in this class
	// or exists in live data.
	const long m_nFirstNewID;
	// The internal ID of the next record to be added to this table (when SetCreated
	// is called)
	long m_nNextID;

public:
	CUIFormChangesTracker(const CString& strTableName) :
	  m_nFirstNewID(-1)
	{
		m_strTableName = strTableName;
		m_nNextID = m_nFirstNewID;
	}
	~CUIFormChangesTracker()
	{
		ClearRecords(m_CreatedRecords);
		ClearRecords(m_ModifiedRecords);
	}

private:
	// Removes all records from an array in the changes tracker
	void ClearRecords(CArray<Record*,Record*>& aRecords);

	// Adds a SQL statement to a batch
    void AddToSqlBatch(CString& strSqlBatch, const CString& strSql);

	// Calculates and returns the SQL statement for creating all new records
	CString GetCreateSql();

	// Calculates and returns the SQL statement for modifying existing records
	CString GetModifySql();

	// Calculates and returns the SQL statement for deleting existing records
	CString GetDeleteSql();

public:
    // This function should be called when the caller desires to add a new record
    // to the database upon saving. Returns a placeholder ID for future reference.
	long SetCreated(const CStringArray& astrFieldNames, const CArray<_variant_t,_variant_t&>& aValues);

    // This function should be called when the caller wants to modify an existing
    // record in the database upon saving. If the record was already used in a prior
    // call to this function, then any non-existent fields are added, and existing fields
    // are overwritten.
	void SetModified(long nRecordID, const CString& strFieldName, const _variant_t& value);

    // This function should be called when the caller wants to delete an
    // existing record in the database upon saving.
	void SetDeleted(long nRecordID);

	// This function will generate and return the complete SQL statement for applying all 
	// changes to data
	CString GenerateUpdateSql();

	// This function will generate and run the query to update the data
	void DoDatabaseUpdate();
};
