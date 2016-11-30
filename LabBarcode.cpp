#include "stdafx.h"
#include "LabBarcode.h"
#include "MiscSystemUtils.h"
#include "SharedInsuranceUtils.h"
#include "LabCustomFieldTemplate.h"
#include "LabCustomField.h"
#include "HL7ParseUtils.h"

using namespace ADODB;

#import "IDAutomationPDF417.dll"
using namespace PDF417Lib;

// (r.gonet 11/11/2011) - PLID 44212 - Creates a barcode part that has no corresponding part saved in the database.
//   Defaults should be nMaxLength = -1, strTextValue = "", nLabCustomField = -1, strChildSeparator = ""
//TES 7/24/2012 - PLID 50393 - Added nLabProcedureID
// (r.gonet 03/08/2014) - PLID 61190 - Added TrimTrailingSeparators to cause this barcode part to trim trailing, consecutive separators
CLabBarcodePart::CLabBarcodePart(CString strPath, long nPartIndex, CString strDescription, 
								 ELabBarcodePartFillType fillType, long nMaxLength, CString strTextValue, long nLabCustomFieldID, CString strChildSeparator, long nLabProcedureID, bool bTrimFinalSeparator/* = true*/, bool bTrimTrailingSeparators/* = false*/)
{
	m_nID = -1;
	m_nIndex = nPartIndex;
	m_strPath = strPath;
	m_strDescription = strDescription;
	m_lbpftFillType = fillType;
	m_nMaxLength = nMaxLength;
	m_strTextValue = strTextValue;
	m_nLabCustomFieldID = nLabCustomFieldID;
	m_strChildSeparator = strChildSeparator;
	m_bTrimFinalSeparator = bTrimFinalSeparator;
	m_bTrimTrailingSeparators = bTrimTrailingSeparators;
	m_emsModificationStatus = emsNew;
	m_strUID = NewUUID();
	m_nLabProcedureID = nLabProcedureID;
}

// (r.gonet 11/11/2011) - PLID 44212 - Creates a barcode part based on an existing part in the database.
//TES 7/24/2012 - PLID 50393 - Added nLabProcedureID
// (r.gonet 03/08/2014) - PLID 61190 - Added TrimTrailingSeparators to cause this barcode part container to trim trailing, consecutive separators
CLabBarcodePart::CLabBarcodePart(long nID, CString strUID, CString strPath, long nPartIndex, CString strDescription, 
								 ELabBarcodePartFillType fillType, long nMaxLength, CString strTextValue, long nLabCustomFieldID, CString strChildSeparator, long nLabProcedureID, bool bTrimFinalSeparator/* = true*/, bool bTrimTrailingSeparators/* = false*/)
{
	m_nID = nID;
	m_nIndex = nPartIndex;
	m_strPath = strPath;
	m_strDescription = strDescription;
	m_lbpftFillType = fillType;
	m_nMaxLength = nMaxLength;
	m_strTextValue = strTextValue;
	m_nLabCustomFieldID = nLabCustomFieldID;
	m_strChildSeparator = strChildSeparator;
	m_bTrimFinalSeparator = bTrimFinalSeparator;
	m_bTrimTrailingSeparators = bTrimTrailingSeparators;
	m_emsModificationStatus = emsNone;
	m_strUID = strUID;
	m_nLabProcedureID = nLabProcedureID;
}

// (r.gonet 11/11/2011) - PLID 44212 - Recursively constructs a string representation of this part, whether this be the root or a component.
//   Roots for instance will construct the whole barcode. Components will construct only its own value.
CString CLabBarcodePart::ToString()
{
	if(m_lbpftFillType == lbpftContainer) {
		CString strValues;
		for(int i = 0; i < m_aryChildren.GetSize(); i++) {
			CString strValue = m_aryChildren[i]->ToString();
			strValues += strValue + m_strChildSeparator;
		}
		if(m_bTrimFinalSeparator) {
			// (r.gonet 03/08/2014) - PLID 61190 - Remove the last separator
			strValues = strValues.Left(strValues.GetLength() - m_strChildSeparator.GetLength());
		}
		if(m_bTrimTrailingSeparators) {
			// (r.gonet 03/08/2014) - PLID 61190 - We can trim any consecutive trailing separators (i.e. if they contain empty values)
			while(m_strChildSeparator != "" && strValues.GetLength() >= m_strChildSeparator.GetLength() && strValues.Right(m_strChildSeparator.GetLength()) == m_strChildSeparator) {
				strValues = strValues.Left(strValues.GetLength() - m_strChildSeparator.GetLength());
			}
		}

		return strValues;
	} else {
		// We won't allow special characters
		return _H(m_strTextValue);
	}
}

// (r.gonet 11/11/2011) - PLID 44212 - Adds a child part to this part as the last child.
void CLabBarcodePart::AddChild(CLabBarcodePartPtr pNewChild)
{
	int i = 0;
	CLabBarcodePartPtr pPreviousChild;
	for(; i < m_aryChildren.GetSize(); i++) {
		CLabBarcodePartPtr pCurrentChild = m_aryChildren[i];
		if(pPreviousChild == NULL) {
			if(pNewChild->m_nIndex < pCurrentChild->m_nIndex) {
				break;
			}
		} else if(pNewChild->m_nIndex > pPreviousChild->m_nIndex && pNewChild->m_nIndex <= pCurrentChild->m_nIndex) {
			break;
		}
		pPreviousChild = pCurrentChild;
	}
	m_aryChildren.InsertAt(i, pNewChild);
}

// (r.gonet 11/11/2011) - PLID 44212 - Commits this part to the database, assuming it was changed.
void CLabBarcodePart::Save(CParamSqlBatch &sqlBatch)
{
	// We use -1 as No Field. But SQL demands we use NULL. So get a variant together.
	_variant_t varLabCustomFieldID = (m_nLabCustomFieldID == -1 ? g_cvarNull : _variant_t((long)m_nLabCustomFieldID));
	//TES 7/24/2012 - PLID 50393 - Added LabProcedureID
	// (r.gonet 03/08/2014) - PLID 61190 - Moved up to get it in the conditionally compiled INSERT statement
	_variant_t varLabProcedureID = g_cvarNull;
	if(m_nLabProcedureID != -1) {
		varLabProcedureID = m_nLabProcedureID;
	}
	
// Barcode parts cannot be new unles we are setting up a structure.
#if BARCODE_STRUCTURE_SETUP == 1
	if(m_emsModificationStatus == emsNew) {
		// (r.gonet 03/08/2014) - PLID 61190 - Added LabProcedureID (which was missing before) and TrimTrailingSeparators
		sqlBatch.Add(
			"INSERT INTO LabBarcodePartsT (Path, PartIndex, Description, MaxLength, FillType, TextValue, LabCustomFieldID, ChildSeparator, UID, LabProcedureID, TrimTrailingSeparators) "
			"VALUES " 
			"({STRING}, {INT}, {STRING}, {INT}, {INT}, {STRING}, {VT_I4}, {STRING}, {STRING}, {VT_I4}, {BIT}); ",
			m_strPath, m_nIndex, m_strDescription, m_nMaxLength, (long)m_lbpftFillType, m_strTextValue, varLabCustomFieldID, m_strChildSeparator, m_strUID, varLabProcedureID, m_bTrimTrailingSeparators);
	} else
#endif
	if(m_emsModificationStatus == emsModified) {
		// We shouldn't be updating much.
		
		sqlBatch.Add(
			"UPDATE LabBarcodePartsT "
			"SET TextValue = {STRING}, "
			"	 LabCustomFieldID = {VT_I4}, "
			"	LabProcedureID = {VT_I4} "
			"WHERE UID = {STRING}; ",
			m_strTextValue, varLabCustomFieldID, varLabProcedureID, m_strUID);
	}
	m_emsModificationStatus = emsNone;
}

// (r.gonet 11/11/2011) - PLID 44212 - Gets the database ID. -1 if not saved to the database yet.
long CLabBarcodePart::GetID()
{
	return m_nID;
}

// (r.gonet 11/11/2011) - PLID 44212 - Gets the path of the part. Empty string for root.
CString CLabBarcodePart::GetPath()
{
	return m_strPath;
}

// (r.gonet 11/11/2011) - PLID 44212 - Gets a description of the part.
CString CLabBarcodePart::GetDescription()
{
	return m_strDescription;
}

// (r.gonet 11/11/2011) - PLID 44212 - Gets how this part is filled.
ELabBarcodePartFillType CLabBarcodePart::GetFillType()
{
	return m_lbpftFillType;
}

// (r.gonet 11/11/2011) - PLID 44212 - Gets how many characters this part can hold.
long CLabBarcodePart::GetMaxLength()
{
	return m_nMaxLength;
}

// (r.gonet 11/11/2011) - PLID 44212 - Gets the textual value for this part. Returns empty string if this is a container part.
CString CLabBarcodePart::GetTextValue()
{
	return m_strTextValue;
}

// (r.gonet 11/11/2011) - PLID 44212 - Gets the lab custom field the user has associated with this part. -1 if no field associated.
long CLabBarcodePart::GetLabCustomField()
{
	return m_nLabCustomFieldID;
}

// (r.gonet 11/11/2011) - PLID 44212 - Sets the database ID for this part.
void CLabBarcodePart::SetID(long nID)
{
	m_nID = nID;
}

// (r.gonet 11/11/2011) - PLID 44212 - Sets the hierarchical path of the part.
void CLabBarcodePart::SetPath(CString strPath)
{
	m_strPath = strPath;
}

// (r.gonet 11/11/2011) - PLID 44212 - Sets the user visible description of this part.
void CLabBarcodePart::SetDescription(CString strDescription)
{
	m_strDescription = strDescription;
}

// (r.gonet 11/11/2011) - PLID 44212 - Sets how this part is filled, by text, by program, by field, or by container.
void CLabBarcodePart::SetFillType(ELabBarcodePartFillType lbpftFillType)
{
	m_lbpftFillType = lbpftFillType;
}

// (r.gonet 11/11/2011) - PLID 44212 - Sets the maximum number of characters this part may contain if it is not a container part. -1 means undefined.
void CLabBarcodePart::SetMaxLength(long nMaxLength)
{
	m_nMaxLength = nMaxLength;
}

// (r.gonet 11/11/2011) - PLID 44212 - Sets the textual value of this part. Container parts should have no text. Text will be truncated to the max length.
void CLabBarcodePart::SetTextValue(CString strTextValue)
{
	if(m_lbpftFillType == lbpftContainer) {
		ASSERT(FALSE);
		ThrowNxException("CLabBarcodePart::SetTextValue : Attempted to fill a container part with a textual value. Container parts may not have text values.");
	}

	if(m_nMaxLength != -1) {
		strTextValue = strTextValue.Left(m_nMaxLength);
	}
	m_strTextValue = strTextValue;
}

// (r.gonet 11/11/2011) - PLID 44212 - Sets a custom field to fill this part. -1 means undefined.
void CLabBarcodePart::SetLabCustomField(long nLabCustomFieldID)
{
	m_nLabCustomFieldID = nLabCustomFieldID;
}

//TES 7/24/2012 - PLID 50393 - Added LabProcedureID
void CLabBarcodePart::SetLabProcedureID(long nLabProcedureID)
{
	m_nLabProcedureID = nLabProcedureID;
}

// (r.gonet 11/11/2011) - PLID 44212 - Traverses this part and its children, calling a caller specified function on each part.
//   The caller may specify an argument that will be passed in the callback function along with the part.
void CLabBarcodePart::PreOrderTraverse(void (fnCallback)(CLabBarcodePartPtr , void *), void *pArg)
{
	if(m_strPath != "") {
		(*fnCallback)(shared_from_this(), pArg);
	}
	if(m_lbpftFillType == lbpftContainer) {
		for(int i = 0; i < m_aryChildren.GetSize(); i++) {
			m_aryChildren[i]->PreOrderTraverse(fnCallback, pArg);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// CLabBarcode
///////////////////////////////////////////////////////////////////////////////

// (r.gonet 11/11/2011) - PLID 44212 - Constructs a new barcode.
CLabBarcode::CLabBarcode()
{
}

CLabBarcode::~CLabBarcode()
{
}

// (r.gonet 11/11/2011) - PLID 44212 - Gets the containing part for a particular child's path. NULL if it doesn't exist.
CLabBarcodePartPtr CLabBarcode::GetParentPart(CString strChildPath)
{
	long nIndex = strChildPath.ReverseFind('.');
	if(nIndex == -1) {
		// Has no parent, at least not a real one.
		return this->GetRoot();
	} else {
		strChildPath = strChildPath.Left(nIndex);
		CLabBarcodePartPtr pParent;
		if(!m_mapParts.Lookup(strChildPath, pParent)) {
			return CLabBarcodePartPtr();
		} else {
			return pParent;
		}
	}
}

// (r.gonet 11/11/2011) - PLID 44212 - Gets a part from a path.
CLabBarcodePartPtr CLabBarcode::GetPart(CString strPath)
{
	CLabBarcodePartPtr pPart;
	if(!m_mapParts.Lookup(strPath, pPart)) {
		return CLabBarcodePartPtr();
	} else {
		return pPart;
	}
}

// (r.gonet 11/11/2011) - PLID 44212 - Gets the root part that contains all segments.
CLabBarcodePartPtr CLabBarcode::GetRoot()
{
	return GetPart("");
}

#if BARCODE_STRUCTURE_SETUP == 1

// (r.gonet 11/11/2011) - PLID 44212 - Adds a container part to the barcode. Containers, such as segments or fields, hold other parts, such as fields or components.
// (r.gonet 03/08/2014) - PLID 61190 - Added TrimTrailingSeparators to cause this barcode part container to trim trailing, consecutive separators
CLabBarcodePartPtr CLabBarcode::AddPart_Container(CString strPath, CString strDescription, CString strChildSeparator, bool bTrimTrailingSeparators/* = false*/)
{
	CLabBarcodePartPtr pPart;
	if((pPart = GetPart(strPath)) != NULL) {
		return pPart;
	}
	// Determine the order of this part by its path
	long nPartIndex;
	long nLastPathSeparator = strPath.ReverseFind('.');
	if(nLastPathSeparator == -1) {
		// We are a segment and so the order is defined by how many other segments there are so far.
		nPartIndex = GetRoot()->GetChildCount();
	} else {
		// Dots serve as path separation characters.
		nPartIndex = atol(strPath.Right(strPath.GetLength() - (strPath.ReverseFind('.') + 1)));
	}
	// Create a new part and add it to its parent's collection
	// (r.gonet 03/08/2014) - PLID 61190 - Added -1 for the the nLabProcedureID to get this conditionally compiled code working again.
	pPart = CLabBarcodePartPtr(new CLabBarcodePart(strPath, nPartIndex, strDescription, lbpftContainer, -1, "", -1, strChildSeparator, -1, true, bTrimTrailingSeparators));
	pPart->SetModificationStatus(CLabBarcodePart::emsNew);
	CLabBarcodePartPtr pParent = GetParentPart(strPath);
	if(pParent) {
		pParent->AddChild(pPart);
	}
	// Also add it to our lookup table
	m_mapParts[strPath] = pPart;

	return pPart;
}

// (r.gonet 11/11/2011) - PLID 44212 - Adds a barcode part that will be filled by the code.
CLabBarcodePartPtr CLabBarcode::AddPart_Auto(CString strPath, CString strDescription, long nMaxLength)
{
	CLabBarcodePartPtr pPart;
	if((pPart = GetPart(strPath)) != NULL) {
		return pPart;
	}
	long nPartIndex;
	long nLastPathSeparator = strPath.ReverseFind('.');
	if(nLastPathSeparator == -1) {
		// We are a segment
		nPartIndex = GetRoot()->GetChildCount();
	} else {
		nPartIndex = atol(strPath.Right(strPath.GetLength() - (strPath.ReverseFind('.') + 1)));
	}
	// Create a new part and add it to its parent's collection
	// (r.gonet 03/08/2014) - PLID 61190 - Added -1 for the the nLabProcedureID to get this conditionally compiled code working again.
	pPart = boost::make_shared<CLabBarcodePart>(strPath, nPartIndex, strDescription, lbpftAuto, nMaxLength, "", -1, "", -1);
	pPart->SetModificationStatus(CLabBarcodePart::emsNew);
	CLabBarcodePartPtr pParent = GetParentPart(strPath);
	if(pParent) {
		pParent->AddChild(pPart);
	}
	// Also add it to our lookup table
	m_mapParts[strPath] = pPart;

	return pPart;
}

// (r.gonet 11/11/2011) - PLID 44212 - Adds a barcode part to the barcode that is filled by user entered text in the barcode setup dlg
CLabBarcodePartPtr CLabBarcode::AddPart_Text(CString strPath, CString strDescription, long nMaxLength, CString strText)
{
	CLabBarcodePartPtr pPart;
	if((pPart = GetPart(strPath)) != NULL) {
		return pPart;
	}
	long nPartIndex;
	long nLastPathSeparator = strPath.ReverseFind('.');
	if(nLastPathSeparator == -1) {
		// We are a segment
		nPartIndex = GetRoot()->GetChildCount();
	} else {
		nPartIndex = atol(strPath.Right(strPath.GetLength() - (strPath.ReverseFind('.') + 1)));
	}
	// (r.gonet 03/08/2014) - PLID 61190 - Added -1 for the the nLabProcedureID to get this conditionally compiled code working again.
	pPart = boost::make_shared<CLabBarcodePart>(strPath, nPartIndex, strDescription, lbpftText, nMaxLength, strText, -1, "", -1);
	pPart->SetModificationStatus(CLabBarcodePart::emsNew);
	CLabBarcodePartPtr pParent = GetParentPart(strPath);
	if(pParent) {
		pParent->AddChild(pPart);
	}
	m_mapParts[strPath] = pPart;

	return pPart;
}

// (r.gonet 11/11/2011) - PLID 44212 - Adds a part that is filled by a custom field to the barcode
CLabBarcodePartPtr CLabBarcode::AddPart_CustomField(CString strPath, CString strDescription, long nMaxLength, long nLabCustomFieldID/*= -1*/)
{
	CLabBarcodePartPtr pPart;
	if((pPart = GetPart(strPath)) != NULL) {
		return pPart;
	}
	long nPartIndex;
	long nLastPathSeparator = strPath.ReverseFind('.');
	if(nLastPathSeparator == -1) {
		// We are a segment
		nPartIndex = GetRoot()->GetChildCount();
	} else {
		nPartIndex = atol(strPath.Right(strPath.GetLength() - (strPath.ReverseFind('.') + 1)));
	}
	// (r.gonet 03/08/2014) - PLID 61190 - Added -1 for the the nLabProcedureID to get this conditionally compiled code working again.
	pPart = boost::make_shared<CLabBarcodePart>(strPath, nPartIndex, strDescription, lbpftField, nMaxLength, "", nLabCustomFieldID, "", -1);
	pPart->SetModificationStatus(CLabBarcodePart::emsNew);
	CLabBarcodePartPtr pParent = GetParentPart(strPath);
	if(pParent) {
		// This is a non-root segment
		pParent->AddChild(pPart);
	}
	m_mapParts[strPath] = pPart;

	return pPart;
}

#endif

// (r.gonet 11/11/2011) - PLID 44212 - Loads the barcode definition from the database.
void CLabBarcode::Load()
{
	m_mapParts.RemoveAll();
	// Make a dummy root node that contains all segments.
	//TES 7/24/2012 - PLID 50393 - Added LabProcedureID
	CLabBarcodePartPtr pRoot(new CLabBarcodePart(-1, "", "", 0, "ROOT", lbpftContainer, -1, "", -1, "\r", -1, false));
	m_mapParts[""] = pRoot;

	// Go through all of the parts
	//TES 7/24/2012 - PLID 50393 - Added LabProcedureID
	// (r.gonet 03/08/2014) - PLID 61190 - Added TrimTrailingSeparators
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT ID, Path, PartIndex, Description, MaxLength, FillType, TextValue, LabCustomFieldID, ChildSeparator, UID, LabProcedureID, TrimTrailingSeparators "
		"FROM LabBarcodePartsT "
		"ORDER BY Path ASC; ");

	while(!prs->eof) {
		long nID = VarLong(prs->Fields->Item["ID"]->Value);
		CString strPath = VarString(prs->Fields->Item["Path"]->Value);
		long nPartIndex = VarLong(prs->Fields->Item["PartIndex"]->Value);
		CString strDescription = VarString(prs->Fields->Item["Description"]->Value, "");
		ELabBarcodePartFillType fillType = (ELabBarcodePartFillType)VarLong(prs->Fields->Item["FillType"]->Value, lbpftContainer);
		long nMaxLength = VarLong(prs->Fields->Item["MaxLength"]->Value, -1);
		CString strTextValue = VarString(prs->Fields->Item["TextValue"]->Value, "");
		long nLabCustomFieldID = VarLong(prs->Fields->Item["LabCustomFieldID"]->Value, -1);
		CString strChildSeparator = VarString(prs->Fields->Item["ChildSeparator"]->Value, "");
		CString strUID = VarString(prs->Fields->Item["UID"]->Value);
		//TES 7/24/2012 - PLID 50393 - Set nLabProcedureID
		long nLabProcedureID = VarLong(prs->Fields->Item["LabProcedureID"]->Value, -1);
		// (r.gonet 03/08/2014) - PLID 61190 - Set TrimTrailingSeparators
		bool bTrimTrailingSeparators = VarBool(prs->Fields->Item["TrimTrailingSeparators"]->Value, FALSE) ? true : false;

		// Make a new part object from these
		CLabBarcodePartPtr pPart;
		CLabBarcodePartPtr pParent = GetParentPart(strPath);
		if(pParent) {
			bool bTrimFinalSeparator = pParent != GetRoot();
			pPart = CLabBarcodePartPtr(new CLabBarcodePart(nID, strUID, strPath, nPartIndex, strDescription, fillType, nMaxLength, strTextValue, nLabCustomFieldID, strChildSeparator, nLabProcedureID, bTrimFinalSeparator, bTrimTrailingSeparators));
			pParent->AddChild(pPart);
		}
		// Add it to our lookup map.
		m_mapParts[strPath] = pPart;

		prs->MoveNext();
	}
	prs->Close();
}

// (r.gonet 11/11/2011) - PLID 44212 - Saves the barcode definition to the database.
void CLabBarcode::Save()
{
	CParamSqlBatch sqlBatch;

	CLabBarcodePartPtr pPart;
	CString strPath;
	POSITION pos = m_mapParts.GetStartPosition();
	while(pos) {
		m_mapParts.GetNextAssoc(pos, strPath, pPart);
		pPart->Save(sqlBatch);
	}
	sqlBatch.Execute(GetRemoteData());
}

// (r.gonet 11/11/2011) - PLID 44212 - Gets the textual value at a particular part path. Empty string if part doesn't exist.
CString CLabBarcode::GetValue(CString strPath)
{
	CLabBarcodePartPtr pPart;
	if(!m_mapParts.Lookup(strPath, pPart) || pPart == NULL) {
		return "";
	} else {
		return pPart->GetTextValue();
	}
}

// (r.gonet 11/11/2011) - PLID 44212 - Sets the value of a part at a path. Has no effect if the path is not valid. Trim causes the value to be trimmed of whitespace before being set.
void CLabBarcode::SetValue(CString strPath, CString strValue, bool bTrim/*= true*/)
{
	CLabBarcodePartPtr pPart;
	if(!m_mapParts.Lookup(strPath, pPart) || pPart == NULL) {
		return;
	} else {
		if(bTrim) {
			strValue = Trim(strValue);
		}
		pPart->SetTextValue(strValue);
	}
}

// (r.gonet 11/11/2011) - PLID 44212 - Formats a date field from a recordset to the desired barcode format (similar to HL7)
CString CLabBarcode::AsBarcodeDate(_RecordsetPtr prs, CString strField)
{
	_variant_t varDateTime = prs->Fields->Item[_bstr_t(strField)]->Value;
	if(varDateTime.vt != VT_DATE) {
		return "";
	}
	COleDateTime dt = VarDateTime(varDateTime);
	if(dt.GetStatus() == COleDateTime::valid) {
		return dt.Format("%Y%m%d");
	} else {
		return "";
	}
}

// (r.gonet 11/11/2011) - PLID 44212 - Formats a time field from a recordset to the desired barcode format (similar to HL7)
CString CLabBarcode::AsBarcodeTime(_RecordsetPtr prs, CString strField)
{
	_variant_t varDateTime = prs->Fields->Item[_bstr_t(strField)]->Value;
	if(varDateTime.vt != VT_DATE) {
		return "";
	}
	COleDateTime dt = VarDateTime(varDateTime);
	if(dt.GetStatus() == COleDateTime::valid) {
		return dt.Format("%H%M%S");
	} else {
		return "";
	}
}

// (r.gonet 11/11/2011) - PLID 44212 - Strips dashes from an SSN
CString CLabBarcode::FormatBarcodeSSN(CString strSSN)
{
	CString strResultSSN = "";
	for(int i = 0; i < strSSN.GetLength(); i++) {
		if(isdigit(strSSN[i])) {
			strResultSSN += strSSN[i];
		}
	}
	return strResultSSN;
}

// (r.gonet 11/11/2011) - PLID 44212 - Strips dashes, spaces, and parens from a phone number.
CString CLabBarcode::FormatBarcodePhone(CString strPhone)
{
	strPhone.Remove('(');
	strPhone.Remove(')');
	strPhone.Remove(' ');
	strPhone.Remove('-');
	return strPhone;
}

// (r.gonet 11/11/2011) - PLID 44212 - Traverses the barcode parts with a callback function and allows an argument to be passed along.
void CLabBarcode::PreOrderTraverse(void (fnCallback)(CLabBarcodePartPtr , void *), void *pArg)
{
	if(GetRoot() != NULL) {
		GetRoot()->PreOrderTraverse(fnCallback, pArg);
	}
}

// (r.gonet 11/11/2011) - PLID 44212 - A barcode may need to be encoded. This is where it happens.
void CLabBarcode::Encode(OUT CArray<CString, CString> &aryBarcodes)
{
	CString strBarcode = GetRoot()->ToString();
	aryBarcodes.Add(strBarcode);
}

// (r.gonet 11/11/2011) - PLID 44212 - Merges the custom fields the user has setup to go with each part to the parts' text values.
void CLabBarcode::MergeLabCustomFields(long nLabID)
{
	CCFTemplateInstance cfTemplateInstance;
	cfTemplateInstance.LoadByLabID(nLabID);
	// Most of the fields require no special formatting, so do everything right now and then overwrite for specifics afterward.
	POSITION pos = m_mapParts.GetStartPosition();
	while(pos) {
		CString strPath;
		CLabBarcodePartPtr pPart;
		m_mapParts.GetNextAssoc(pos, strPath, pPart);
		if(pPart->GetFillType() != lbpftField) {
			continue;
		}

		long nFieldID = pPart->GetLabCustomField();
		pPart->SetTextValue(ConvertLabCustomFieldToString(nFieldID, cfTemplateInstance));
	}

	// (r.gonet 11/11/2011) - PLID 46434 - We have some fields that require very specific formatting, which we do now.
	OverwriteCustomFields(cfTemplateInstance);
}

// (r.gonet 11/11/2011) - PLID 44212 - Takes a lab custom field ID and an instance it is in, then returns the string representation of its instance value suitable for barcodes.
CString CLabBarcode::ConvertLabCustomFieldToString(long nFieldID, CCFTemplateInstance &cfTemplateInstance)
{
	if(nFieldID == -1) {
		// This field has not been chosen by the user yet, so leave this field's value blank
		return "";
	} else {
		CLabCustomFieldInstance *pFieldInstance = cfTemplateInstance.GetFieldInstanceByFieldID(nFieldID);
		if(!pFieldInstance) {
			return "";
		}
		ELabCustomFieldType lcftFieldType = pFieldInstance->GetField()->GetFieldType();
		_variant_t varValue = pFieldInstance->GetValue();
		// Pass on to the recursive version
		return ConvertLabCustomFieldToString(varValue, lcftFieldType, pFieldInstance->GetField()->GetDisplayType(), pFieldInstance);
	}
}

// (r.gonet 11/11/2011) - PLID 44212 - Recursively converts a custom field to a barcode suitable string format.
CString CLabBarcode::ConvertLabCustomFieldToString(_variant_t varValue,
												   ELabCustomFieldType lcftFieldType, ELabCustomFieldDisplayType lcdtDisplayType, 
												   CLabCustomFieldInstance *pInstance)
{
	CString strOutput = "";
	if(varValue.vt == VT_NULL) {
		// No filled value means no value for the barcode field
		strOutput = "";
	} else if(lcftFieldType == lcftBool && varValue.vt == VT_BOOL) {
		// Boolean fields
		BOOL bValue = VarBool(varValue);
		if(bValue) {
			strOutput = "Y";
		} else {
			strOutput = "N";
		}
	} else if(lcftFieldType == lcftInteger && varValue.vt == VT_I4) {
		// Integer fields
		long nValue = VarLong(varValue);
		strOutput = FormatString("%li", nValue);
	} else if(lcftFieldType == lcftFloat && varValue.vt == VT_R8) {
		// Float fields
		double dValue = VarDouble(varValue);
		strOutput = FormatString("%f", dValue);
	} else if(lcftFieldType == lcftDateTime && varValue.vt == VT_DATE) {
		// Date Time fields, with a few different display types
		COleDateTime dtValue = VarDateTime(varValue);
		if(dtValue.GetStatus() != COleDateTime::valid) {
			// What?
			strOutput = "";
		} else if(lcdtDisplayType == lcdtDefault || lcdtDisplayType == lcdtDateAndTime) {
			strOutput = dtValue.Format("%Y%m%d%H%M%S");
		} else if(lcdtDisplayType == lcdtDate) {
			strOutput = dtValue.Format("%Y%m%d");
		} else if(lcdtDisplayType == lcdtTime) {
			strOutput = dtValue.Format("%H%M%S");
		} else {
			ASSERT(FALSE);
			strOutput = "";
		}
	} else if((lcftFieldType == lcftText || lcftFieldType == lcftMemo) && varValue.vt == VT_BSTR) {
		// String fields are easy
		CString strValue = VarString(varValue);
		strOutput = strValue;
	} else if(lcftFieldType == lcftSingleSelectList && varValue.vt == VT_I4) {
		// List fields require us to get the selected item's value and do this same function on it now.
		long nItemID = VarLong(varValue);
		CLabCustomFieldPtr pField = pInstance->GetField();
		CLabCustomListFieldPtr pListField = boost::dynamic_pointer_cast<CLabCustomListField>(pField);
		if(pListField == NULL) {
			ASSERT(FALSE);
		} else {
			CLabCustomListFieldItem *pItem = pListField->GetItem(nItemID);
			if(pItem == NULL) {
				ASSERT(FALSE);
			}
			strOutput = ConvertLabCustomFieldToString(pItem->GetValue(), pItem->GetType(), lcdtDefault, NULL);
		}
	} else {
		// We have a field that has an unhandled type. Either the code's author forgot to add the support for it here or
		//  there is bad data.
		ASSERT(FALSE);
	}

	return strOutput;
}

// (r.gonet 11/11/2011) - PLID 44212 - We need to fill some custom field filled parts manually because they require special formatting.
//   Implementations should overwrite the value in a part with their own manually formatted value from the custom field instance.
void CLabBarcode::OverwriteCustomFields(CCFTemplateInstance &cfTemplateIntance)
{
}

///////////////////////////////////////////////////////////////////////////////
// LabCorp Lab Barcode
///////////////////////////////////////////////////////////////////////////////

CLabCorpLabBarcode::CLabCorpLabBarcode()
	: CLabBarcode()
{
}

CLabCorpLabBarcode::~CLabCorpLabBarcode()
{
}

// (r.gonet 11/11/2011) - PLID 46434 - I'm afraid this one is very specific and kind of long. Figures out what value should go into P.18 and sets a few output flags.
CString CLabCorpLabBarcode::CalculateP18BillCode(_RecordsetPtr prs, OUT bool &bFillMedicareNumberInP19, OUT bool &bFillSubscriberNumberInP40, OUT bool &bFillSubscriberNumberInP53)
{
	/* Bill Code
	· Use: Type of billing for order
	· Valid Values:
	‘03’ = Account/Client Billing
	
	‘04’ = Patient Billing
	
	‘XI’ = Third Party Insurance** (If carrier code is
	greater than 2 characters) Populate subscriber
	number in P40
	
	‘05’ = Medicare Populate Medicare number in
	P19.
	
	‘Z ’ = If carrier code is not 2 or 5 characters**
	Populate subscriber number in P40.
	
	‘M1' = Medicare with ABN required *
	
	‘??’ = If Medicaid, this field will contain the 2
	character state abbreviation. Populate Medicaid
	number in P53 if primary insurance
	
	‘??’ = If HMO, this field will contain a LabCorp
	assigned 2 character HMO Bill Code. Populate
	the subscriber number in P53 if primary
	insurance
	
	‘CS’ – Cash Sales (not used by EDI clients)
	
	‘XN’ – Indigent Billing (Requires contractual
	commitment from both client and third party
	vendor prior to submitting)
	
	* ‘M1’ bill code is supported through LCM
	systems only and not valid for EDI. 
	*/

	ASSERT(prs && !prs->eof);
	CString strP18Value;
	long nInsuredPartyID = VarLong(prs->Fields->Item["LabInsuredPartyID"]->Value, -1);
	if(nInsuredPartyID == -1) {
		// '04' = Patient Billing	

		// This could actually be either the client gets billed or the patient gets billed, but we don't distinguish between them. So assume patient.
		strP18Value = "04";
	} else {
		InsuranceTypeCode itcInsuranceType = (InsuranceTypeCode)VarLong(prs->Fields->Item["LabInsuredPartyInsuranceType"]->Value, -1);
		if(itcInsuranceType == itcMedicaid) {
			// '??' = If Medicaid, this field will contain the 2
			//  character state abbreviation. Populate Medicaid
			//  number in P53 if primary insurance
			CString strInsuredPartyInsCoState = VarString(prs->Fields->Item["LabInsuredPartyInsCoState"]->Value, "");
			strP18Value = strInsuredPartyInsCoState.Left(2).MakeUpper();
			if(VarLong(prs->Fields->Item["LabInsuredPartyRespTypePriority"]->Value) == 1) {
				bFillSubscriberNumberInP53 = true;
			}
		} else if(itcInsuranceType == itcHMO || itcInsuranceType == itcHMO_MedicareRisk) {
			// '??' = If HMO, this field will contain a LabCorp
			//  assigned 2 character HMO Bill Code. Populate
			//  the subscriber number in P53 if primary
			//  insurance
			CString strLabCorpHMOCode = VarString(prs->Fields->Item["LabInsPartyInsCoLabCorpCode"]->Value, "");
			strP18Value = strLabCorpHMOCode;
			if(VarLong(prs->Fields->Item["LabInsuredPartyRespTypePriority"]->Value) == 1) {
				bFillSubscriberNumberInP53 = true;
			}
		} else if(itcInsuranceType == itcMedicarePartA || itcInsuranceType == itcMedicarePartB) {
			// '05' = Medicare Populate Medicare number in
			//  P19.

			// LabCorp wants us to distinguish between medicare with ABN required and medicare without it required, 
			//  but we have no way of doing this currently. So just assume Medicare.
			strP18Value = "05";
			bFillMedicareNumberInP19 = true;
		} else {
			CString strCarrierCode = VarString(prs->Fields->Item["LabInsPartyInsCoLabCorpCode"]->Value, "");
			if(strCarrierCode.GetLength() != 2 && strCarrierCode.GetLength() != 5) {
				// 'Z' = If carrier code is not 2 or 5 characters**
				//  Populate subscriber number in P40.
				strP18Value = "Z";
				bFillSubscriberNumberInP40 = true;
			} else {
				// 'XI' = Third Party Insurance** (If carrier code is
				//  greater than 2 characters) Populate subscriber
				//  number in P40
				strP18Value = "XI";
				if(strCarrierCode.GetLength() > 2) {
					bFillSubscriberNumberInP40 = true;
				}
			}
		}
	}
	return strP18Value;
}

// (r.gonet 11/11/2011) - PLID 46434 - We need to fill some custom field filled parts manually because they require special formatting.
void CLabCorpLabBarcode::OverwriteCustomFields(CCFTemplateInstance &cfTemplateInstance)
{
	CLabBarcodePartPtr pPart;
	CLabCustomFieldInstance *pFieldInstance = NULL;

	// Zero pad to 2 characters
	if((pPart = GetPart("C.24.2")) != NULL) {
		pFieldInstance = cfTemplateInstance.GetFieldInstanceByFieldID(pPart->GetLabCustomField());
		if(pFieldInstance) {
			_variant_t varValue = pFieldInstance->GetValue();
			SetValue("C.24.2", (varValue.vt == VT_I4 || varValue.vt == VT_NULL ? FormatString("%02li", VarLong(varValue, 0)) : ""));
		} else {
			// The lab's template doesn't have this field. So that's fine.
		}
	} else {
		// The user has made the lab request print barcodes, but the structure has not been setup yet by NexTech. So we can't do anything
		return;
	}

	// This is a multi value field. We do this by using components but when one component is blank, they should be 0. Oh and we always need a : between them.
	if(GetPart("A.1.1") != NULL && GetPart("A.1.2") != NULL) {
		CString strA_1_1_Value = GetValue("A.1.1");
		CString strA_1_2_Value = GetValue("A.1.2");
		if(!strA_1_1_Value.IsEmpty() && strA_1_2_Value.IsEmpty()) {
			// We have the number of weeks but not the number of days. Fill in 0 for the days.
			SetValue("A.1.2", "0");
		} else if(strA_1_1_Value.IsEmpty() && !strA_1_2_Value.IsEmpty()) {
			// We have the number days but not the number of weeks. Fill in 0 for the weeks.
			SetValue("A.1.1", "0");
		} else {
			// Either we have no gestational age at all or both values are populated already.
		}

		strA_1_1_Value = GetValue("A.1.1");
		strA_1_2_Value = GetValue("A.1.2");
		if(!strA_1_1_Value.IsEmpty() && !strA_1_2_Value.IsEmpty()) {
			// We have both a weeks value and a days value. Put a separator between them.
			SetValue("A.1.1", strA_1_1_Value + ":");
		} else {
			// The gestational age is not present, so we don't want a separator.
		}
	} else {
		// The user has made the lab request print barcodes, but the structure has not been setup yet by NexTech. So we can't do anything
		return;
	}

	// Requires padding of 0s if less than 3 and just one decimal point.
	if((pPart = GetPart("A.21.1")) != NULL) {
		pFieldInstance = cfTemplateInstance.GetFieldInstanceByFieldID(pPart->GetLabCustomField());
		if(pFieldInstance) {
			_variant_t varValue = pFieldInstance->GetValue();
			SetValue("A.21.1", (varValue.vt == VT_R8 || varValue.vt == VT_NULL ? FormatString("%03.1f", VarDouble(varValue, 0.0)) : ""));
		} else {
			// The lab's template doesn't have this field. So that's fine.
		}
	} else {
		// The user has made the lab request print barcodes, but the structure has not been setup yet by NexTech. So we can't do anything
		return;
	}
	// Requires padding of 0s if less than 3 and just one decimal point.
	if((pPart = GetPart("A.21.3")) != NULL) {
		pFieldInstance = cfTemplateInstance.GetFieldInstanceByFieldID(pPart->GetLabCustomField());
		if(pFieldInstance) {
			_variant_t varValue = pFieldInstance->GetValue();
			SetValue("A.21.3", (varValue.vt == VT_R8 || varValue.vt == VT_NULL ? FormatString("%03.1f", VarDouble(varValue, 0.0)) : ""));
		} else {
			// The lab's template doesn't have this field. So that's fine.
		}
	} else {
		// The user has made the lab request print barcodes, but the structure has not been setup yet by NexTech. So we can't do anything
		return;
	}
	// Requires padding of 0s if less than 3 and just one decimal point.
	if((pPart = GetPart("A.22.1")) != NULL) {
		pFieldInstance = cfTemplateInstance.GetFieldInstanceByFieldID(pPart->GetLabCustomField());
		if(pFieldInstance) {
			_variant_t varValue = pFieldInstance->GetValue();
			SetValue("A.22.1", (varValue.vt == VT_R8 || varValue.vt == VT_NULL ? FormatString("%03.1f", VarDouble(varValue, 0.0)) : ""));
		} else {
			// The lab's template doesn't have this field. So that's fine.
		}
	} else {
		// The user has made the lab request print barcodes, but the structure has not been setup yet by NexTech. So we can't do anything
		return;
	}
	// Requires padding of 0s if less than 3 and just one decimal point.
	if((pPart = GetPart("A.22.2")) != NULL) {
		pFieldInstance = cfTemplateInstance.GetFieldInstanceByFieldID(pPart->GetLabCustomField());
		if(pFieldInstance) {
			_variant_t varValue = pFieldInstance->GetValue();
			SetValue("A.22.2", (varValue.vt == VT_R8 || varValue.vt == VT_NULL ? FormatString("%03.1f", VarDouble(varValue, 0.0)) : ""));
		} else {
			// The lab's template doesn't have this field. So that's fine.
		}
	} else {
		// The user has made the lab request print barcodes, but the structure has not been setup yet by NexTech. So we can't do anything
		return;
	}
}

#if BARCODE_STRUCTURE_SETUP == 1
// (r.gonet 11/11/2011) - PLID 46434 - Creates in memory the correct structure for a labcorp barcode.
void CLabCorpLabBarcode::InitStructure()
{
	// Segments
	AddPart_Container("H", "Header Record", "|");
		AddPart_Text("H.0", "Header Record", 1, "H");
		AddPart_Text("H.1", "Product & Version Number", 12, "");
		AddPart_Auto("H.2", "Date Requisition Printed", 8); 
		AddPart_Text("H.3", "EDI Identifier", 1, "E");
		AddPart_Container("H.4", "Travel Log Identification", "^"); // Unknown
			AddPart_Auto("H.4.1", "HRID Number", 5); // Unknown
			AddPart_Auto("H.4.2", "Log Date", 7); // Unknown
			AddPart_Auto("H.4.3", "Sequence Number", 2); // Unknown
		AddPart_Auto("H.5", "Version Control Field", 2); // unused

	AddPart_Container("P", "Patient Detail Record", "|");
		AddPart_Text("P.0", "Patient Record", 1, "P");
		AddPart_Auto("P.1", "Patient ID", 10);
		AddPart_Auto("P.2", "Hospital ID / Alternate Patient ID", 20); // Unused
		AddPart_Auto("P.3", "Location Code", 10); // Unused
		AddPart_Auto("P.4", "Physician Signature", 1); // Unused
		AddPart_Auto("P.5", "Assigned Patient Location (Room)", 10); // Unused
		AddPart_Auto("P.6", "Ward/Floor", 10); // Unused
		AddPart_Text("P.7", "Account/Client Number", 8, "");
		AddPart_Auto("P.8", "Control ID/Requisition Number", 11);
		AddPart_Container("P.9", "Patient Name", "^");
			AddPart_Auto("P.9.1", "Patient Last Name", 25);
			AddPart_Auto("P.9.2", "Patient First Name", 15);
			AddPart_Auto("P.9.3", "Patient Middle Initial", 1);
		AddPart_Auto("P.10", "Patient DOB", 8);
		AddPart_Auto("P.11", "Patient Gender", 2);
		AddPart_Auto("P.12", "Patient Social Security Number", 9);
		AddPart_Auto("P.13", "Patient Address (Street)", 35);
		AddPart_Auto("P.14", "Patient Address (City)", 16);
		AddPart_Auto("P.15", "Patient Address (State)", 2);
		AddPart_Auto("P.16", "Patient Address (Zip Code)", 9);
		AddPart_Auto("P.17", "Patient Phone Number", 10);
		AddPart_Auto("P.18", "Patient Bill Code", 2); // What is this?
		AddPart_Auto("P.19", "Medicare Number", 13);
		AddPart_Container("P.20", "Responsible Party Name", "^");
			AddPart_Auto("P.20.1", "Responsible Party's Last Name", 12);
			AddPart_Auto("P.20.2", "Responsible Party's First Name", 9);
			AddPart_Auto("P.20.3", "Responsible Party's Middle Initial", 1);
		AddPart_Auto("P.21", "Responsible Party's SS#", 9);
		AddPart_Auto("P.22", "Responsible Party's Address (Street)", 35);
		AddPart_Auto("P.23", "Responsible Party's Address (City)", 16);
		AddPart_Auto("P.24", "Responsible Party's Address (State", 2);
		AddPart_Auto("P.25", "Responsible Party's Address (Zip Code)", 9);
		AddPart_Auto("P.26", "Responsible Party's Employer Name", 15);
		AddPart_Auto("P.27", "Relation to Subscriber", 1);
		AddPart_Auto("P.28", "Local Physician ID", 10);
		AddPart_Container("P.29", "Physician Name", "^");
			AddPart_Auto("P.29.1", "Physician Last Name", 25);
			AddPart_Auto("P.29.2", "Physician First Name", 15);
			AddPart_Auto("P.29.3", "Physician Middle Initial", 1);
		AddPart_Auto("P.30", "Physician Provider Id (Primary Carrier)", 10); // What is this?
		AddPart_Auto("P.31", "Physician Provider Id (Secondary Carrier)", 10); // What is this?
		AddPart_Auto("P.32", "Physician UPIN Number", 6);
		AddPart_Auto("P.33", "Not Used", 1);
		AddPart_Auto("P.34", "Primary Insurance Payer Code", 5); // Labcorp's code
		AddPart_Auto("P.35", "Primary Insurance Name", 30);
		AddPart_Container("P.36", "Primary Insurance Address (Street)", "^");
			AddPart_Auto("P.36.1", "Primary Insurance Address 1", 30);
			AddPart_Auto("P.36.2", "Primary Insurance Address 2", 30);
		AddPart_Auto("P.37", "Primary Insurance Address (City)", 16);
		AddPart_Auto("P.38", "Primary Insurance Address (State)", 2);
		AddPart_Auto("P.39", "Primary Insurance Address (Zip Code)", 9);
		AddPart_Auto("P.40", "Primary Insurance Subscriber", 25); // This is the primary insurance policy number, not a name
		AddPart_Auto("P.41", "Primary Insurance Group Number", 30);
		AddPart_Auto("P.42", "Primary Insurance Group Name", 30);
		AddPart_Auto("P.43", "Secondary Insurance Payer Code", 5); // Labcorp's code
		AddPart_Auto("P.44", "Secondary Insurance Name", 30);
		AddPart_Container("P.45", "Secondary Insurance Address (Street)", "^");
			AddPart_Auto("P.45.1", "Secondary Insurance Address 1", 30);
			AddPart_Auto("P.45.2", "Secondary Insurance Address 2", 30);
		AddPart_Auto("P.46", "Secondary Insurance Address (City)", 16);
		AddPart_Auto("P.47", "Secondary Insurance Address (State)", 2);
		AddPart_Auto("P.48", "Secondary Insurance Address (Zip Code)", 9);
		AddPart_Auto("P.49", "Secondary Insurance Subscriber", 25); // This is the primary insurance policy number, not a name
		AddPart_Auto("P.50", "Secondary Insurance Group Number", 30);
		AddPart_Auto("P.51", "Secondary Insurance Group Name", 30);
		AddPart_Auto("P.52", "Worker's Compensation", 1);
		AddPart_Auto("P.53", "Medicaid Number/HMO Number", 30);
		AddPart_Container("P.54", "1-15 Specimen Containers", "^"); // Not used
			AddPart_Auto("P.54.1", "Specimen Container 1", 1);
			AddPart_Auto("P.54.2", "Specimen Container 2", 1);
			AddPart_Auto("P.54.3", "Specimen Container 3", 1);
			AddPart_Auto("P.54.4", "Specimen Container 4", 1);
			AddPart_Auto("P.54.5", "Specimen Container 5", 1);
			AddPart_Auto("P.54.6", "Specimen Container 6", 1);
			AddPart_Auto("P.54.7", "Specimen Container 7", 1);
			AddPart_Auto("P.54.8", "Specimen Container 8", 1);
			AddPart_Auto("P.54.9", "Specimen Container 9", 1);
			AddPart_Auto("P.54.10", "Specimen Container 10", 1);
			AddPart_Auto("P.54.11", "Specimen Container 11", 1);
			AddPart_Auto("P.54.12", "Specimen Container 12", 1);
			AddPart_Auto("P.54.13", "Specimen Container 13", 1);
			AddPart_Auto("P.54.14", "Specimen Container 14", 1);
			AddPart_Auto("P.54.15", "Specimen Container 15", 1);
		AddPart_Container("P.55", "ICD-9 Codes", "^"); // Maximum of 8, variable number
			AddPart_Auto("P.55.1", "ICD-9 Code 1", 8);
			AddPart_Auto("P.55.2", "ICD-9 Code 2", 8);
			AddPart_Auto("P.55.3", "ICD-9 Code 3", 8);
			AddPart_Auto("P.55.4", "ICD-9 Code 4", 8);
			AddPart_Auto("P.55.5", "ICD-9 Code 5", 8);
			AddPart_Auto("P.55.6", "ICD-9 Code 6", 8);
			AddPart_Auto("P.55.7", "ICD-9 Code 7", 8);
			AddPart_Auto("P.55.8", "ICD-9 Code 8", 8);
		AddPart_Auto("P.56", "Responsible Party's Phone Number", 10);
		AddPart_Auto("P.57", "External System Accession Number", 30); // This corresponds to OBR-2
		AddPart_Auto("P.58", "External Patient ID", 20);
		AddPart_Auto("P.59", "Diagnosis Text field", 74);
		AddPart_Auto("P.60", "Sale Price (Agreed Upon Price) field", 6); // unused
		AddPart_Auto("P.61", "Payment Amount (Cash Payment) field", 6); // unused
		AddPart_Auto("P.62", "Receipt Number field", 10);
		AddPart_Auto("P.63", "Patient Age field: Years", 3);
		AddPart_Auto("P.64", "Patient Age field: Months", 2);
		AddPart_Auto("P.65", "Patient Age field: Days", 2);
		AddPart_Auto("P.66", "Patient Service Technician ID", 5);
		AddPart_Auto("P.67", "Discount Amount (%)", 3);
		AddPart_CustomField("P.68", "Weight (pounds)", 3);
		AddPart_CustomField("P.69", "Weight (ounces)", 2);
		AddPart_CustomField("P.70", "Height (inches)", 2);
		AddPart_Auto("P.71", "Physician NPI Number", 10);
		AddPart_Container("P.72", "Courtesy Copy 1", "^");
			AddPart_Auto("P.72.1", "Type", 2);
			AddPart_Auto("P.72.2", "Text", 19);
			AddPart_Auto("P.72.3", "Attention", 29);
		AddPart_Container("P.73", "Courtesy Copy 2", "^");
			AddPart_Auto("P.73.1", "Type", 2);
			AddPart_Auto("P.73.2", "Text", 19);
			AddPart_Auto("P.73.3", "Attention", 29);
		AddPart_Container("P.74", "Courtesy Copy 3", "^");
			AddPart_Auto("P.74.1", "Type",2);
			AddPart_Auto("P.74.2", "Text", 19);
			AddPart_Auto("P.74.3", "Attention", 29);
		AddPart_Container("P.75", "Courtesy Copy 4", "^");
			AddPart_Auto("P.75.1", "Type", 2);
			AddPart_Auto("P.75.2", "Text", 19);
			AddPart_Auto("P.75.3", "Attention", 29);
		AddPart_Auto("P.76", "Diagnosis Text 2 field", 74);
		AddPart_Auto("P.77", "ABN Type", 1); // unused
		AddPart_CustomField("P.78", "Race (Required for Litholink)", 1);
		AddPart_Container("P.79", "LabCorp Assigned Specimen Number", "^"); // Unused
			AddPart_Auto("P.79.1", "LabCorp Assigned Specimen Number", 11); // Unused
			AddPart_Auto("P.79.2", "LabCorp Specimen Number Year", 4); // Unused
		AddPart_Auto("P.80", "Number of Attachments", 1); // Unused
		AddPart_Auto("P.81", "Internal Problem Codes", 3); // Unused
		AddPart_Auto("P.82", "Splitting Flag", 1); // Unused
		AddPart_Auto("P.83", "Patient Waiver Flag", 1); // Unused
		AddPart_Auto("P.84", "Problem (PRB) Identified Flag", 1); // Unused
	
	AddPart_Container("C", "Cytology / Additional Information Record (Specific to the LCLS)", "|");
		AddPart_Text("C.0", "Record Identifier", 1, "C");
		AddPart_Container("C.1", "Gyn Source", ""); // This really isn't a field with components in it, but what is called a "fixed position list", so I hacked it with a ""
			AddPart_CustomField("C.1.1", "Cervical (Y/N)", 1);
			AddPart_CustomField("C.1.2", "CVE (Group) (Y/N)", 1);
			AddPart_CustomField("C.1.3", "Endocervical (Y/N)", 1);
			AddPart_CustomField("C.1.4", "Endometrial (Y/N)", 1);
			AddPart_CustomField("C.1.5", "Labia/Vulva (Y/N)", 1);
			AddPart_CustomField("C.1.6", "Vaginal (Y/N)", 1);
			AddPart_CustomField("C.1.7", "Hysterectomy, Supracervical (Y/N)", 1);
		AddPart_Container("C.2", "Collection Technique", "");
			AddPart_CustomField("C.2.1", "Brush (Y/N)", 1);
			AddPart_CustomField("C.2.2", "Brush/Spatula (Y/N)", 1);
			AddPart_CustomField("C.2.3", "CX Broom only (Y/N)", 1);
			AddPart_CustomField("C.2.4", "Other (Y/N)", 1);
			AddPart_CustomField("C.2.5", "Spatula only (Y/N)", 1);
			AddPart_CustomField("C.2.6", "Swab/Spatula (Y/N)", 1);
		AddPart_CustomField("C.3", "LMP - Meno Date", 8);
		AddPart_Container("C.4", "Previous Treatment", "");
			AddPart_CustomField("C.4.1", "Colp & BX (Y/N)", 1);
			AddPart_CustomField("C.4.2", "Conization (Y/N)", 1);
			AddPart_CustomField("C.4.3", "CRYO (Y/N)", 1);
			AddPart_CustomField("C.4.4", "HYST (Y/N)", 1);
			AddPart_CustomField("C.4.5", "Laser Vap (Y/N)", 1);
			AddPart_CustomField("C.4.6", "None (Y/N)", 1);
			AddPart_CustomField("C.4.7", "Radiation (Y/N)", 1);
		AddPart_CustomField("C.5", "Dates-Results", 25);
		AddPart_CustomField("C.6", "Pregnant", 1);
		AddPart_CustomField("C.7", "Lactating", 1);
		AddPart_CustomField("C.8", "Oral Contraceptives", 1);
		AddPart_CustomField("C.9", "Menopausal", 1);
		AddPart_CustomField("C.10", "Estro - RX", 1);
		AddPart_CustomField("C.11", "PMP-Bleeding (Y/N)", 1);
		AddPart_CustomField("C.12", "Post-Part (Y/N)", 1);
		AddPart_CustomField("C.13", "IUD (Y/N)", 1);
		AddPart_CustomField("C.14", "All-Other-Patients", 1);
		AddPart_Container("C.15", "Previous Cyto Values", "");
			AddPart_CustomField("C.15.1", "Atypical (Y/N)", 1);
			AddPart_CustomField("C.15.2", "CA IN-SITU (Y/N)", 1);
			AddPart_CustomField("C.15.3", "Dysplasia (Y/N)", 1);
			AddPart_CustomField("C.15.4", "Invasive CA (Y/N)", 1);
			AddPart_CustomField("C.15.5", "Negative (Y/N)", 1);
			AddPart_CustomField("C.15.6", "Other (Y/N)", 1);
		AddPart_CustomField("C.16", "Stat/Call Back", 1);
		AddPart_Auto("C.17", "Specimen Collection Date", 8);
		AddPart_CustomField("C.18", "Collection Volume", 4);
		AddPart_Container("C.19", "Clinical Information", "^"); // The format of this field is pretty strange.
			AddPart_Auto("C.19.1", "Dummy field", 0);
			AddPart_Auto("C.19.2", "Clinical Information", 69);
		AddPart_Auto("C.20", "Source of Specimen (1)", 6); // unused
		AddPart_Auto("C.21", "Source of Specimen (2)", 6); // unused
		AddPart_Auto("C.22", "Requisition Form Code", 4); // In the specs, this is of length 3, but they want a code of EREQ put here, sooo...
		AddPart_Text("C.23", "Transmit Code", 4, "");
		AddPart_Container("C.24", "Fasting", ""); // Another hack for a labcorp multivalue field
			AddPart_CustomField("C.24.1", "Fasting", 1);
			AddPart_CustomField("C.24.2", "Fasting # of Hours", 2);
		AddPart_Auto("C.25", "Collection Time", 6);
		AddPart_CustomField("C.26", "Dates and Results of Treatment", 25);

	AddPart_Container("A", "AFP Record", "|");
		AddPart_Text("A.0", "Record Identifier", 1, "A");
		AddPart_Container("A.1", "Gestational Age", ""); // A hack for another multi value field
			AddPart_CustomField("A.1.1", "Gestational Age (Weeks)", 3);
			// (r.gonet 03/08/2014) - PLID 61190 - Added a description and maxlength to get this conditionally compiled code working again.
			AddPart_Text("A.1.2", "", 1, ".");
			AddPart_CustomField("A.1.3", "Gestational Age (Days)", 1);
		AddPart_CustomField("A.2", "Gestational Age Calculation Type", 3);
		AddPart_CustomField("A.3", "Weight", 3);
		AddPart_CustomField("A.4", "Type of Pregnancy", 1);
		AddPart_CustomField("A.5", "Race", 1);
		AddPart_CustomField("A.6", "Insulin Dependent", 2);
		AddPart_CustomField("A.7", "Routine Screening", 2);
		AddPart_CustomField("A.8", "Previous Neural Tube Defects", 1);
		AddPart_CustomField("A.9", "Advanced Maternal Age", 1);
		AddPart_CustomField("A.10", "History of Down Syndrome", 1);
		AddPart_CustomField("A.11", "Other Indications", 1);
		AddPart_CustomField("A.12", "Estimated Date of Delivery or Confinement (EDD/EDC)", 8);
		AddPart_CustomField("A.13", "Last Menstrual Period Date", 8);
		AddPart_CustomField("A.14", "Ultrasound Date", 8);
		AddPart_CustomField("A.15", "History of Cystic Fibrosis", 1);
		AddPart_CustomField("A.16", "Handwritten AFP Info", 20);
		AddPart_CustomField("A.17", "Reason for Repeat: Previously Elevated AFP", 1);
		AddPart_CustomField("A.18", "Gestational Age Date", 8);
		// (r.gonet 03/08/2014) - PLID 61190 - Added a maxlength to get this conditionally compiled code working again.
		AddPart_Text("A.19", "Reason for Repeat: Early Gestational Age", 1, "");
		// (r.gonet 03/08/2014) - PLID 61190 - Added a maxlength to get this conditionally compiled code working again.
		AddPart_Text("A.20", "Reason for Repeat: Hemolyzed", 1, "");
		AddPart_Container("A.21", "Ultrasound Measurement", "^");
			AddPart_CustomField("A.21.1", "Crown Rump Length (mm)", 4);
			AddPart_CustomField("A.21.2", "Crown Rump Length Date", 8);
			AddPart_CustomField("A.21.3", "Crown Rump Length for Twin (mm)", 4);
		AddPart_Container("A.22", "Nuchal Translucency", "^");
			AddPart_CustomField("A.22.1", "Nuchal Translucency for First Fetus(mm)", 4);
			AddPart_CustomField("A.22.2", "Nuchal Translucency for Twin(mm)", 4);
		AddPart_Container("A.23", "Donor Egg", "^");
			AddPart_CustomField("A.23.1", "Donor Egg", 1);
			AddPart_CustomField("A.23.2", "Age of Donor Egg", 2);
		AddPart_CustomField("A.24", "Prior Down Syndrome/ONTD Screening", 1);
		AddPart_CustomField("A.25", "Prior First Trimaster Screening", 1);
		AddPart_CustomField("A.26", "Prior Second Trimaster Screening", 1);
		AddPart_CustomField("A.27", "Family History of Neural Tube Defect (FHX NTD)", 1);
		AddPart_CustomField("A.28", "Prior Pregnancy with Down Syndrome", 1);
		AddPart_Container("A.29", "Chorionicity (twins only)", "^");
			AddPart_CustomField("A.29.1", "Monochorianic", 1);
			AddPart_CustomField("A.29.2", "Dichorianic", 1);
			AddPart_CustomField("A.29.3", "Unknown", 1);
		AddPart_Container("A.30", "Sonographer Information", "^");
			AddPart_CustomField("A.30.1", "Sonographer Last Name", 25);
			AddPart_CustomField("A.30.2", "Sonographer First Name", 15);
			AddPart_CustomField("A.30.3", "Sonographer Number (certification number)", 20);
			AddPart_CustomField("A.30.4", "Credentialed by NTQR", 1);
			AddPart_CustomField("A.30.5", "Credentialed by FMF", 1);
			AddPart_CustomField("A.30.6", "Credentialed by Other Organization", 1);
		AddPart_CustomField("A.31", "Site Number", 20);
		AddPart_CustomField("A.32", "Reading Physician ID", 20);

	AddPart_Container("M", "Metal Record", "|");
		AddPart_Text("M.0", "Record Identifier", 1, "M");
		AddPart_CustomField("M.1", "Race", 1);
		AddPart_CustomField("M.2", "Hispanic", 1);
		AddPart_CustomField("M.3", "Type of Sample", 1);
		AddPart_CustomField("M.4", "Purpose of Test", 1);
		AddPart_CustomField("M.5", "County Code", 2);

	AddPart_Container("B", "ABN - CPT Code Record (Unused)", "|"); // Not required for EDI, but we still have to put the empty structure.
		AddPart_Text("B.0", "Record Identifier", 1, "B");
		AddPart_Auto("B.1", "CPT Code 1", 5);
		AddPart_Auto("B.2", "CPT Code 2", 5);
		AddPart_Auto("B.3", "CPT Code 3", 5);
		AddPart_Auto("B.4", "CPT Code 4", 5);
		AddPart_Auto("B.5", "CPT Code 5", 5);
		AddPart_Auto("B.6", "CPT Code 6", 5);
		AddPart_Auto("B.7", "CPT Code 7", 5);
		AddPart_Auto("B.8", "CPT Code 8", 5);
		AddPart_Auto("B.9", "CPT Code 9", 5);
		AddPart_Auto("B.10", "CPT Code 10", 5);
		AddPart_Auto("B.11", "CPT Code 11", 5);
		AddPart_Auto("B.12", "CPT Code 12", 5);
		AddPart_Auto("B.13", "CPT Code 13", 5);
		AddPart_Auto("B.14", "CPT Code 14", 5);
		AddPart_Auto("B.15", "CPT Code 15", 5);
		AddPart_Auto("B.16", "CPT Code 16", 5);
		AddPart_Auto("B.17", "CPT Code 17", 5);
		AddPart_Auto("B.18", "CPT Code 18", 5);
		AddPart_Auto("B.19", "CPT Code 19", 5);
		AddPart_Auto("B.20", "CPT Code 20", 5);

	AddPart_Container("K", "Chronic Kidney Disease Record (Unused)", "|");
		AddPart_Text("K.0", "Record Identifier", 1, "K");
		AddPart_Container("K.1", "Blood Pressure", "^");
			AddPart_Auto("K.1.1", "Systolic", 3);
			AddPart_Auto("K.1.2", "Diastolic", 3);
		AddPart_Auto("K.2", "Diabetes", 1);
		AddPart_Auto("K.3", "Phosphate Binder", 1);
		AddPart_Auto("K.4", "Calcium Based Binder", 1);
		AddPart_Auto("K.5", "Active Vitamin D", 1);
		AddPart_Auto("K.6", "Vitamin D", 1);
		AddPart_Auto("K.7", "Statin", 1);
		AddPart_Auto("K.8", "Fibrate", 1);
		AddPart_Auto("K.9", "Niacin (nicotinic acid)", 1);
		AddPart_Auto("K.10", "ESA (Erythropoiesis Stimulating Agent)", 1);
		AddPart_Auto("K.11", "Iron", 1);
		AddPart_Auto("K.12", "Alkali", 1);
		AddPart_Auto("K.13", "ACE Inhibitor (ACEI) or Angiotensin II Receptor Blockers (ARB)", 1);
		AddPart_Auto("K.14", "Low Phosphate Diet", 1);
		AddPart_Auto("K.15", "Diuretic", 1);
		AddPart_Container("K.16", "Chronic Kidney Disease Diagnosis", "^");
			AddPart_Auto("K.16.1", "CKD Stage", 3);
			AddPart_Auto("K.16.2", "CKD Stage Number", 3);
			AddPart_Auto("K.16.3", "CKD Unspecified", 3);
			AddPart_Auto("K.16.4", "Other Indicator", 3);
			AddPart_Auto("K.16.5", "Other Text", 30);
		AddPart_Auto("K.17", "Fe/TIBC Diagnosis (CKD Co-morbidity)", 30);
		AddPart_Auto("K.18", "Ferritin Diagnosis (CKD Co-morbitity)", 30);
		AddPart_Auto("K.19", "Lipid Panel Diagnosis", 30);
		AddPart_Auto("K.20", "Hemoglobin Diagnosis", 30);
		AddPart_Auto("K.21", "Physician's Phone Number", 10);

	AddPart_Container("I", "Inventory Record (Unused)", "|");
		AddPart_Text("I.0", "Record Identifier", 1, "I");
		AddPart_Container("I.1", "Inventory", "^");
			AddPart_Auto("I.1.1", "Quantity", 2);
			AddPart_Auto("I.1.2", "Storage", 1);
			AddPart_Auto("I.1.3", "Specimen Type", 3);
		AddPart_Container("I.2", "Inventory", "^");
			AddPart_Auto("I.2.1", "Quantity", 2);
			AddPart_Auto("I.2.2", "Storage", 1);
			AddPart_Auto("I.2.3", "Specimen Type", 3);
		AddPart_Container("I.3", "Inventory", "^");
			AddPart_Auto("I.3.1", "Quantity", 2);
			AddPart_Auto("I.3.2", "Storage", 1);
			AddPart_Auto("I.3.3", "Specimen Type", 3);
		AddPart_Container("I.4", "Inventory", "^");
			AddPart_Auto("I.4.1", "Quantity", 2);
			AddPart_Auto("I.4.2", "Storage", 1);
			AddPart_Auto("I.4.3", "Specimen Type", 3);
		AddPart_Container("I.5", "Inventory", "^");
			AddPart_Auto("I.5.1", "Quantity", 2);
			AddPart_Auto("I.5.2", "Storage", 1);
			AddPart_Auto("I.5.3", "Specimen Type", 3);
		AddPart_Container("I.6", "Inventory", "^");
			AddPart_Auto("I.6.1", "Quantity", 2);
			AddPart_Auto("I.6.2", "Storage", 1);
			AddPart_Auto("I.6.3", "Specimen Type", 3);
		AddPart_Container("I.7", "Inventory", "^");
			AddPart_Auto("I.7.1", "Quantity", 2);
			AddPart_Auto("I.7.2", "Storage", 1);
			AddPart_Auto("I.7.3", "Specimen Type", 3);
		AddPart_Container("I.8", "Inventory", "^");
			AddPart_Auto("I.8.1", "Quantity", 2);
			AddPart_Auto("I.8.2", "Storage", 1);
			AddPart_Auto("I.8.3", "Specimen Type", 3);


	AddPart_Container("T", "Test Code Record", "|");
		AddPart_Text("T.0", "Record Identifier", 1, "T");
		AddPart_Auto("T.1", "LabCorp Test Code 1", 6);
		AddPart_Auto("T.2", "LabCorp Test Code 2", 6);
		AddPart_Auto("T.3", "LabCorp Test Code 3", 6);
		AddPart_Auto("T.4", "LabCorp Test Code 4", 6);
		AddPart_Auto("T.5", "LabCorp Test Code 5", 6);
		AddPart_Auto("T.6", "LabCorp Test Code 6", 6);
		AddPart_Auto("T.7", "LabCorp Test Code 7", 6);
		AddPart_Auto("T.8", "LabCorp Test Code 8", 6);
		AddPart_Auto("T.9", "LabCorp Test Code 9", 6);
		AddPart_Auto("T.10", "LabCorp Test Code 10", 6);
		AddPart_Auto("T.11", "LabCorp Test Code 11", 6);
		AddPart_Auto("T.12", "LabCorp Test Code 12", 6);
		AddPart_Auto("T.13", "LabCorp Test Code 13", 6);
		AddPart_Auto("T.14", "LabCorp Test Code 14", 6);
		AddPart_Auto("T.15", "LabCorp Test Code 15", 6);
		AddPart_Auto("T.16", "LabCorp Test Code 16", 6);
		AddPart_Auto("T.17", "LabCorp Test Code 17", 6);
		AddPart_Auto("T.18", "LabCorp Test Code 18", 6);
		AddPart_Auto("T.19", "LabCorp Test Code 19", 6);
		AddPart_Auto("T.20", "LabCorp Test Code 20", 6);
		AddPart_Auto("T.21", "LabCorp Test Code 21", 6);
		AddPart_Auto("T.22", "LabCorp Test Code 22", 6);
		AddPart_Auto("T.23", "LabCorp Test Code 23", 6);
		AddPart_Auto("T.24", "LabCorp Test Code 24", 6);
		AddPart_Auto("T.25", "LabCorp Test Code 25", 6);
		AddPart_Auto("T.26", "LabCorp Test Code 26", 6);
		AddPart_Auto("T.27", "LabCorp Test Code 27", 6);
		AddPart_Auto("T.28", "LabCorp Test Code 28", 6);
		AddPart_Auto("T.29", "LabCorp Test Code 29", 6);
		AddPart_Auto("T.30", "LabCorp Test Code 30", 6);
		AddPart_Auto("T.31", "LabCorp Test Code 31", 6);
		AddPart_Auto("T.32", "LabCorp Test Code 32", 6);
		AddPart_Auto("T.33", "LabCorp Test Code 33", 6);
		AddPart_Auto("T.34", "LabCorp Test Code 34", 6);
		AddPart_Auto("T.35", "LabCorp Test Code 35", 6);
		AddPart_Auto("T.36", "LabCorp Test Code 36", 6);
		AddPart_Auto("T.37", "LabCorp Test Code 37", 6);
		AddPart_Auto("T.38", "LabCorp Test Code 38", 6);
		AddPart_Auto("T.39", "LabCorp Test Code 39", 6);
		AddPart_Auto("T.40", "LabCorp Test Code 40", 6);

	// (r.gonet 03/08/2014) - PLID 61190 - Added the D segment which can contain ICD-10 codes.
	AddPart_Container("D", "Diagnosis Code Record", "|");
		AddPart_Text("D.0", "Record Identifier", 1, "D");
		// (r.gonet 03/08/2014) - PLID 61190 - D.1 is the only place in the barcode spec that wants us to trim the trailings separators
		AddPart_Container("D.1", "Diagnosis Codes", "^", true);
			AddPart_Auto("D.1.1", "Diagnosis Code 1", 10);
			AddPart_Auto("D.1.2", "Diagnosis Code 2", 10);
			AddPart_Auto("D.1.3", "Diagnosis Code 3", 10);
			AddPart_Auto("D.1.4", "Diagnosis Code 4", 10);
			AddPart_Auto("D.1.5", "Diagnosis Code 5", 10);
			AddPart_Auto("D.1.6", "Diagnosis Code 6", 10);
			AddPart_Auto("D.1.7", "Diagnosis Code 7", 10);
			AddPart_Auto("D.1.8", "Diagnosis Code 8", 10);
			AddPart_Auto("D.1.9", "Diagnosis Code 9", 10);
			AddPart_Auto("D.1.10", "Diagnosis Code 10", 10);
			AddPart_Auto("D.1.11", "Diagnosis Code 11", 10);
			AddPart_Auto("D.1.12", "Diagnosis Code 12", 10);
			AddPart_Auto("D.1.13", "Diagnosis Code 13", 10);
			AddPart_Auto("D.1.14", "Diagnosis Code 14", 10);
			AddPart_Auto("D.1.15", "Diagnosis Code 15", 10);
			AddPart_Auto("D.1.16", "Diagnosis Code 16", 10);
			AddPart_Auto("D.1.17", "Diagnosis Code 17", 10);
			AddPart_Auto("D.1.18", "Diagnosis Code 18", 10);
			AddPart_Auto("D.1.19", "Diagnosis Code 19", 10);
			AddPart_Auto("D.1.20", "Diagnosis Code 20", 10);
			AddPart_Auto("D.1.21", "Diagnosis Code 21", 10);
			AddPart_Auto("D.1.22", "Diagnosis Code 22", 10);
			AddPart_Auto("D.1.23", "Diagnosis Code 23", 10);
			AddPart_Auto("D.1.24", "Diagnosis Code 24", 10);
			AddPart_Auto("D.1.25", "Diagnosis Code 25", 10);
			AddPart_Auto("D.1.26", "Diagnosis Code 26", 10);
			AddPart_Auto("D.1.27", "Diagnosis Code 27", 10);
			AddPart_Auto("D.1.28", "Diagnosis Code 28", 10);
			AddPart_Auto("D.1.29", "Diagnosis Code 29", 10);
			AddPart_Auto("D.1.30", "Diagnosis Code 30", 10);
			AddPart_Auto("D.1.31", "Diagnosis Code 31", 10);
			AddPart_Auto("D.1.32", "Diagnosis Code 32", 10);
			AddPart_Auto("D.1.33", "Diagnosis Code 33", 10);
			AddPart_Auto("D.1.34", "Diagnosis Code 34", 10);
			AddPart_Auto("D.1.35", "Diagnosis Code 35", 10);
			AddPart_Auto("D.1.36", "Diagnosis Code 36", 10);
			AddPart_Auto("D.1.37", "Diagnosis Code 37", 10);
			AddPart_Auto("D.1.38", "Diagnosis Code 38", 10);
			AddPart_Auto("D.1.39", "Diagnosis Code 39", 10);
			AddPart_Auto("D.1.40", "Diagnosis Code 40", 10);
			AddPart_Auto("D.1.41", "Diagnosis Code 41", 10);
			AddPart_Auto("D.1.42", "Diagnosis Code 42", 10);
			AddPart_Auto("D.1.43", "Diagnosis Code 43", 10);
			AddPart_Auto("D.1.44", "Diagnosis Code 44", 10);
			AddPart_Auto("D.1.45", "Diagnosis Code 45", 10);
			AddPart_Auto("D.1.46", "Diagnosis Code 46", 10);
			AddPart_Auto("D.1.47", "Diagnosis Code 47", 10);
			AddPart_Auto("D.1.48", "Diagnosis Code 48", 10);
			AddPart_Auto("D.1.49", "Diagnosis Code 49", 10);
			AddPart_Auto("D.1.50", "Diagnosis Code 50", 10);
			AddPart_Auto("D.1.51", "Diagnosis Code 51", 10);
			AddPart_Auto("D.1.52", "Diagnosis Code 52", 10);
			AddPart_Auto("D.1.53", "Diagnosis Code 53", 10);
			AddPart_Auto("D.1.54", "Diagnosis Code 54", 10);
			AddPart_Auto("D.1.55", "Diagnosis Code 55", 10);
			AddPart_Auto("D.1.56", "Diagnosis Code 56", 10);
			AddPart_Auto("D.1.57", "Diagnosis Code 57", 10);
			AddPart_Auto("D.1.58", "Diagnosis Code 58", 10);
			AddPart_Auto("D.1.59", "Diagnosis Code 59", 10);
			AddPart_Auto("D.1.60", "Diagnosis Code 60", 10);
			AddPart_Auto("D.1.61", "Diagnosis Code 61", 10);
			AddPart_Auto("D.1.62", "Diagnosis Code 62", 10);
			AddPart_Auto("D.1.63", "Diagnosis Code 63", 10);
			AddPart_Auto("D.1.64", "Diagnosis Code 64", 10);
			AddPart_Auto("D.1.65", "Diagnosis Code 65", 10);
			AddPart_Auto("D.1.66", "Diagnosis Code 66", 10);
			AddPart_Auto("D.1.67", "Diagnosis Code 67", 10);
			AddPart_Auto("D.1.68", "Diagnosis Code 68", 10);
			AddPart_Auto("D.1.69", "Diagnosis Code 69", 10);
			AddPart_Auto("D.1.70", "Diagnosis Code 70", 10);
			AddPart_Auto("D.1.71", "Diagnosis Code 71", 10);
			AddPart_Auto("D.1.72", "Diagnosis Code 72", 10);
			AddPart_Auto("D.1.73", "Diagnosis Code 73", 10);
			AddPart_Auto("D.1.74", "Diagnosis Code 74", 10);
			AddPart_Auto("D.1.75", "Diagnosis Code 75", 10);
			AddPart_Auto("D.1.76", "Diagnosis Code 76", 10);
			AddPart_Auto("D.1.77", "Diagnosis Code 77", 10);
			AddPart_Auto("D.1.78", "Diagnosis Code 78", 10);
			AddPart_Auto("D.1.79", "Diagnosis Code 79", 10);
			AddPart_Auto("D.1.80", "Diagnosis Code 80", 10);
			AddPart_Auto("D.1.81", "Diagnosis Code 81", 10);
			AddPart_Auto("D.1.82", "Diagnosis Code 82", 10);
			AddPart_Auto("D.1.83", "Diagnosis Code 83", 10);
			AddPart_Auto("D.1.84", "Diagnosis Code 84", 10);
			AddPart_Auto("D.1.85", "Diagnosis Code 85", 10);
			AddPart_Auto("D.1.86", "Diagnosis Code 86", 10);
			AddPart_Auto("D.1.87", "Diagnosis Code 87", 10);
			AddPart_Auto("D.1.88", "Diagnosis Code 88", 10);
			AddPart_Auto("D.1.89", "Diagnosis Code 89", 10);
			AddPart_Auto("D.1.90", "Diagnosis Code 90", 10);
			AddPart_Auto("D.1.91", "Diagnosis Code 91", 10);
			AddPart_Auto("D.1.92", "Diagnosis Code 92", 10);
			AddPart_Auto("D.1.93", "Diagnosis Code 93", 10);
			AddPart_Auto("D.1.94", "Diagnosis Code 94", 10);
			AddPart_Auto("D.1.95", "Diagnosis Code 95", 10);
			AddPart_Auto("D.1.96", "Diagnosis Code 96", 10);
			AddPart_Auto("D.1.97", "Diagnosis Code 97", 10);
			AddPart_Auto("D.1.98", "Diagnosis Code 98", 10);
			AddPart_Auto("D.1.99", "Diagnosis Code 99", 10);
			AddPart_Auto("D.1.100", "Diagnosis Code 100", 10);
			AddPart_Auto("D.1.101", "Diagnosis Code 101", 10);
			AddPart_Auto("D.1.102", "Diagnosis Code 102", 10);
			AddPart_Auto("D.1.103", "Diagnosis Code 103", 10);
			AddPart_Auto("D.1.104", "Diagnosis Code 104", 10);
			AddPart_Auto("D.1.105", "Diagnosis Code 105", 10);
			AddPart_Auto("D.1.106", "Diagnosis Code 106", 10);
			AddPart_Auto("D.1.107", "Diagnosis Code 107", 10);
			AddPart_Auto("D.1.108", "Diagnosis Code 108", 10);
			AddPart_Auto("D.1.109", "Diagnosis Code 109", 10);
			AddPart_Auto("D.1.110", "Diagnosis Code 110", 10);
			AddPart_Auto("D.1.111", "Diagnosis Code 111", 10);
			AddPart_Auto("D.1.112", "Diagnosis Code 112", 10);
			AddPart_Auto("D.1.113", "Diagnosis Code 113", 10);
			AddPart_Auto("D.1.114", "Diagnosis Code 114", 10);
			AddPart_Auto("D.1.115", "Diagnosis Code 115", 10);
			AddPart_Auto("D.1.116", "Diagnosis Code 116", 10);
			AddPart_Auto("D.1.117", "Diagnosis Code 117", 10);
			AddPart_Auto("D.1.118", "Diagnosis Code 118", 10);
			AddPart_Auto("D.1.119", "Diagnosis Code 119", 10);
			AddPart_Auto("D.1.120", "Diagnosis Code 120", 10);
		AddPart_Auto("D.2", "Diagnosis Text", 500);
	
	AddPart_Container("L", "Length Record", "|");
		AddPart_Text("L.0", "Record Identifier", 1, "L");
		AddPart_Auto("L.1", "Total Character Count", 5);
		
	AddPart_Container("E", "Error Code Record", "|");
		AddPart_Text("E.0", "Record Identifier", 1, "E");
		AddPart_Auto("E.1", "Error Code", 2);
}
#endif

// (r.gonet 11/11/2011) - PLID 46434 - Fills the barcode's structure by merging custom fields and filling auto-fill fields.
//   This is specific to LabCorp at this point. I'd recommend subclassing the barcode class if you need to create a different barcode.
void CLabCorpLabBarcode::FillBarcode(long nPatientID, CString strFormNumberTextID, long nHL7GroupID/*= -1*/)
{
	if(GetRoot() == NULL) {
		// No structure exists, so what can we do but return?
		return;
	}

	long nLabID = -1;
	CStringArray saLOINCCodes, saDiagnosisCodes;
	// (r.gonet 03/08/2014) - PLID 61190 - Corresponds to saDiagnosisCodes. Holds whether the diag code is an ICD-10 or not.
	CArray<BOOL> aryDiagnosisCodesICD10;
	// Business logic flags
	bool bFillMedicareNumberInP19 = false;
	bool bFillSubscriberNumberInP40 = false;
	bool bFillSubscriberNumberInP53 = false;

	_RecordsetPtr prs = CreateParamRecordset(
		// Get the values associated with all of the lab orders with this form number
		"SELECT LabsT.ID, LabsT.LOINC_Code, LabsT.InitialDiagnosis FROM LabsT WHERE PatientID = {INT} AND FormNumberTextID = {STRING} AND Deleted = 0 ORDER BY ID ASC ",
		nPatientID, strFormNumberTextID);
	if(prs->eof) {
		// The lab doesn't exist
		return;
	}
	while(!prs->eof) {
		// Get the first lab in the order.
		if(nLabID == -1) {
			nLabID = VarLong(prs->Fields->Item["ID"]->Value);
		} else {
			// We just needed the top 1 lab id because we don't have a table for Requisitions that all LabsT rows with the same form number point to.
			//  So yes, we kludge it.
		}
		// Collect all of the LOINC codes from each lab in the order.
		CString strLOINCCode = VarString(prs->Fields->Item["LOINC_Code"]->Value, "");
		if(!strLOINCCode.IsEmpty()) {
			saLOINCCodes.Add(strLOINCCode);
		} else {
			// The LabsT record doesn't have any LOINC code set. This is fine I think for a row not to have ordered a test. Maybe a biopsy?
		}
		// Collect all diagnosis codes from each lab in the order.
		//  This is really really bad. But we don't have discrete diagnosis codes yet.
		//  Follow Tom's parsing in HL7 for the initial diagnosis.
		CString strInitialDiagnosis = VarString(prs->Fields->Item["InitialDiagnosis"]->Value, "");
		if(!strInitialDiagnosis.IsEmpty()) {
			CString strFullDiagnosis = AdoFldString(prs->Fields, "InitialDiagnosis", "");
			CString strDiagDescription = strFullDiagnosis;
			CString strDiagCode;
			// (r.gonet 03/08/2014) - PLID 61190 - We need to determine whether the code is an ICD10 or not.
			BOOL bICD10;
			int nHyphen = strFullDiagnosis.Find(" - ");
			// (r.gonet 03/08/2014) - PLID 61190 - Removed the constraint on the hyphen being in the first 10 chars. The
			// check was pointless if you think about it since it has to match an ICD code in our system anyway.
			if(nHyphen > 0) {
				CString strPossibleCode = strFullDiagnosis.Left(nHyphen);
				// (r.gonet 03/08/2014) - PLID 61190 - Changed ReturnsRecordsParam to a recordset in order to get the ICD10 flag.
				_RecordsetPtr prsDiags = CreateParamRecordset("SELECT TOP 1 ID, ICD10 FROM DiagCodes WHERE CodeNumber = {STRING} ORDER BY ICD10 DESC", strPossibleCode);
				if(!prsDiags->eof) {
					// (r.gonet 03/08/2014) - PLID 61190 - A single match. Good.
					strDiagCode = strPossibleCode;
					strDiagDescription = strFullDiagnosis.Mid(nHyphen+3);
					bICD10 = AdoFldBool(prsDiags->Fields, "ICD10");
					saDiagnosisCodes.Add(strDiagCode);
					aryDiagnosisCodesICD10.Add(bICD10);
				} else {
					// So there is some text that looks like it could be a diagnosis code but it doesn't match up
					//  to anything in our database. I guess it isn't a code. Jeese this is bad. We need discrete diagnosis codes for labs.
				}
				prsDiags->Close();
			} else {
				// The initial diagnosis doesn't look like an ICD code, so don't attempt to parse one out.
			}
		} else {
			// No initial diagnosis set, whatever.
		}
		prs->MoveNext();
	}
	prs->Close();

	// Merge the custom fields to the barcode first.
	MergeLabCustomFields(nLabID);

	// Now get all necessary information about this lab.
	// We get the top 1 because the join on the HL7CodeLinkT could produce multiple rows if there are multiple codes associated with
	//  this insurance company. There shouldn't be but it is possible.
	//TES 11/12/2012 - PLID 53665 - Added PatientRace
	//TES 11/12/2012 - PLID 53708 - Added ToBeOrdered
	//TES 11/12/2012 - PLID 53709 - Added Specimen
	// (b.spivey, May 29, 2013) - PLID 56871 - Changed race name. 
	prs = CreateParamRecordset(
		"DECLARE @HL7GroupID INT; "
		"DECLARE @LabID INT; "
		"SET @HL7GroupID = {INT}; "
		"SET @LabID = {INT}; "
		"SELECT TOP 1 FormNumberTextID, Specimen, PatientsT.UserDefinedID, PatientLast, PatientFirst, PatientMiddle, "
			"PatientPersonT.BirthDate AS PatientDOB, PatientPersonT.Gender AS PatientGender, PatientPersonT.SocialSecurity AS PatientSSN, "
			"PatientPersonT.Address1 AS PatientAddress1, PatientPersonT.Address2 AS PatientAddress2, "
			"PatientPersonT.City AS PatientCity, PatientPersonT.State AS PatientState, PatientPersonT.Zip AS PatientZip, "
			"PatientPersonT.HomePhone AS PatientHomePhone, "
			"RaceSubQ.RaceName AS PatientRace, "
			// -- P18 values
			"LabsT.InsuredPartyID AS LabInsuredPartyID, LabInsuranceCoT.InsType AS LabInsuredPartyInsuranceType, "
			"LabInsCoPersonT.State AS LabInsuredPartyInsCoState, LabInsuredPartyRespTypeT.Priority AS LabInsuredPartyRespTypePriority, LabHL7CodeLinkT.ThirdPartyCode AS LabInsPartyInsCoLabCorpCode, "
			// --
			"LabInsuredPartyT.IDForInsurance AS LabInsuredPartyPolicyNumber, "
			// -- Responsible Party
			"COALESCE(LabInsuredPartyPersonT.Last, PatientPersonT.Last) AS ResponsiblePartyLastName, "
			"COALESCE(LabInsuredPartyPersonT.First, PatientPersonT.First) AS ResponsiblePartyFirstName, "
			"COALESCE(LabInsuredPartyPersonT.Middle, PatientPersonT.Middle) AS ResponsiblePartyMiddleName, "
			"COALESCE(LabInsuredPartyPersonT.SocialSecurity, PatientPersonT.SocialSecurity) AS ResponsiblePartySSN, "
			"COALESCE(LabInsuredPartyPersonT.Address1, PatientPersonT.Address1) AS ResponsiblePartyAddress1, "
			"COALESCE(LabInsuredPartyPersonT.Address2, PatientPersonT.Address2) AS ResponsiblePartyAddress2, "
			"COALESCE(LabInsuredPartyPersonT.City, PatientPersonT.City) AS ResponsiblePartyCity, "
			"COALESCE(LabInsuredPartyPersonT.State, PatientPersonT.State) AS ResponsiblePartyState, "
			"COALESCE(LabInsuredPartyPersonT.Zip, PatientPersonT.Zip) AS ResponsiblePartyZip, "
			"COALESCE(LabInsuredPartyPersonT.HomePhone, PatientPersonT.HomePhone) AS RespPartyHomePhone, "
			"COALESCE(LabInsuredPartyT.Employer, PatientPersonT.Company) AS ResponsiblePartyEmployer, "
			"COALESCE(LabInsuredPartyT.RelationToPatient, 'Self') AS RespPartyRelationToPatient, "
			// -- Ordering Provider
			"LabMultiProviderT.ProviderID AS LabOrderingProviderID, LabProviderPersonT.Last AS LabOrderingProviderLastName, LabProviderPersonT.First AS LabOrderingProviderFirstName, LabProviderPersonT.Middle AS LabOrderingProviderMiddleName, "
			"LabProvidersT.NPI AS LabOrderingProviderNPI, LabProvidersT.UPIN AS LabOrderingProviderUPIN, "
			// -- Primary Insurance
			"PriHL7CodeLinkT.ThirdPartyCode AS PrimaryInsPayerCode, PriInsuranceCoT.Name AS PrimaryInsName, "
			"PriInsCoPersonT.Address1 AS PrimaryInsAddress1, PriInsCoPersonT.Address2 AS PrimaryInsAddress2, PriInsCoPersonT.City PrimaryInsCity, PriInsCoPersonT.State AS PrimaryInsState, PriInsCoPersonT.Zip AS PrimaryInsZip, "
			"PriInsuredPartyT.IdForInsurance AS PrimaryInsPolicyNumber, PriInsuredPartyT.PolicyGroupNum AS PrimaryInsGroupNumber, '' AS PrimaryInsGroupName, "
			// -- Secondary Insurance
			"SecHL7CodeLinkT.ThirdPartyCode AS SecondaryInsPayerCode, SecInsuranceCoT.Name AS SecondaryInsName, "
			"SecInsCoPersonT.Address1 AS SecondaryInsAddress1, SecInsCoPersonT.Address2 AS SecondaryInsAddress2, SecInsCoPersonT.City AS SecondaryInsCity, SecInsCoPersonT.State AS SecondaryInsState, SecInsCoPersonT.Zip AS SecondaryInsZip, "
			"SecInsuredPartyT.IdForInsurance AS SecondaryInsPolicyNumber, SecInsuredPartyT.PolicyGroupNum AS SecondaryInsGroupNumber, '' AS SecondaryInsGroupName, "
			// -- Misc
			"LabInsuranceCoT.WorkersComp, LabsT.InitialDiagnosis, "
			"LabsT.CC_Patient, PatientPersonT.Fax AS PatientFaxNumber, COALESCE(PatientPrefixT.Prefix, '') + ' ' + LabsT.PatientFirst + ' ' + LabsT.PatientLast AS PatientAttention, "
			"LabsT.CC_RefPhys, ReferringPhysPersonT.Fax AS RefPhysFaxNumber, COALESCE(ReferringPhysPrefixT.Prefix, '') + ' ' + ReferringPhysPersonT.First + ' ' + ReferringPhysPersonT.Last AS RefPhysAttention, "
			"LabsT.CC_PCP, PCPPersonT.Fax AS PCPFaxNumber, COALESCE(ReferringPhysPrefixT.Prefix, '') + ' ' + PCPPersonT.First + ' ' + PCPPersonT.Last AS PCPAttention, "
			"LabsT.BiopsyDate, LabsT.ClinicalData, LabAnatomyT.Description AS AnatomicLocation, LabProcCFTemplatesT.Name AS CFTemplateName, LabsT.LOINC_Code, "
			//TES 7/24/2012 - PLID 50393 - We need the LabProcedure ID for this Lab
			"LabsT.LabProcedureID, LabsT.ToBeOrdered "
			   ""
		"FROM LabsT "
			"LEFT JOIN LabAnatomyT ON LabsT.AnatomyID = LabAnatomyT.ID "
			"LEFT JOIN CFTemplateInstancesT ON LabsT.CFTemplateInstanceID = CFTemplateInstancesT.ID "
			"LEFT JOIN LabProcCFTemplatesT ON CFTemplateInstancesT.LabProcCFTemplateID = LabProcCFTemplatesT.ID "
			"INNER JOIN PatientsT ON LabsT.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT PatientPersonT ON PatientsT.PersonID = PatientPersonT.ID "
			"LEFT JOIN PrefixT PatientPrefixT ON PatientPersonT.PrefixID = PatientPrefixT.ID "
			"	CROSS APPLY "
			"	( "
			"		SELECT ( " 
			"			SELECT RT.Name + ', ' "
			"			FROM PersonRaceT PRT "
			"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID "
			"			WHERE PRT.PersonID = PatientPersonT.ID "
			"			FOR XML PATH(''), TYPE "
			"		).value('/', 'nvarchar(max)') "
			"	) RaceSubQ (RaceName) "
			"INNER JOIN LabMultiProviderT ON LabsT.ID = LabMultiProviderT.LabID "
			"INNER JOIN ProvidersT LabProvidersT ON LabMultiProviderT.ProviderID = LabProvidersT.PersonID "
			"INNER JOIN PersonT LabProviderPersonT ON LabProvidersT.PersonID = LabProviderPersonT.ID "
			"LEFT JOIN PersonT ReferringPhysPersonT ON PatientsT.DefaultReferringPhyID = ReferringPhysPersonT.ID "
			"LEFT JOIN PrefixT ReferringPhysPrefixT ON ReferringPhysPersonT.PrefixID = ReferringPhysPrefixT.ID "
			"LEFT JOIN PersonT PCPPersonT ON PatientsT.PCP = PCPPersonT.ID "
			"LEFT JOIN PrefixT PCPPrefixT ON PCPPersonT.PrefixID = PCPPrefixT.ID "
			   ""
			"LEFT JOIN InsuredPartyT LabInsuredPartyT ON PatientsT.PersonID = LabInsuredPartyT.PatientID "
				"AND LabInsuredPartyT.PersonID = LabsT.InsuredPartyID "
			"LEFT JOIN RespTypeT LabInsuredPartyRespTypeT ON LabInsuredPartyT.RespTypeID = LabInsuredPartyRespTypeT.ID "
			"LEFT JOIN PersonT LabInsuredPartyPersonT ON LabInsuredPartyT.PersonID = LabInsuredPartyPersonT.ID "
			"LEFT JOIN InsurancePlansT LabInsurancePlanT ON LabInsuredPartyT.InsPlan = LabInsurancePlanT.ID "
			"LEFT JOIN InsuranceCoT LabInsuranceCoT ON LabInsuredPartyT.InsuranceCoID = LabInsuranceCoT.PersonID "
			"LEFT JOIN PersonT LabInsCoPersonT ON LabInsuranceCoT.PersonID = LabInsCoPersonT.ID "
			"LEFT JOIN HL7CodeLinkT LabHL7CodeLinkT ON LabHL7CodeLinkT.HL7GroupID = @HL7GroupID AND LabHL7CodeLinkT.Type = 5 AND LabHL7CodeLinkT.PracticeID = LabInsuranceCoT.PersonID "
			   ""
			"LEFT JOIN InsuredPartyT PriInsuredPartyT ON PatientsT.PersonID = PriInsuredPartyT.PatientID "
				"AND PriInsuredPartyT.RespTypeID = (SELECT ID FROM RespTypeT WHERE Priority = 1) "
			"LEFT JOIN RespTypeT PriInsuredPartyRespTypeT ON PriInsuredPartyT.RespTypeID = PriInsuredPartyRespTypeT.ID "
			"LEFT JOIN PersonT PriInsuredPartyPersonT ON PriInsuredPartyT.PersonID = PriInsuredPartyPersonT.ID "
			"LEFT JOIN InsurancePlansT PriInsurancePlanT ON PriInsuredPartyT.InsPlan = PriInsurancePlanT.ID "
			"LEFT JOIN InsuranceCoT PriInsuranceCoT ON PriInsuredPartyT.InsuranceCoID = PriInsuranceCoT.PersonID "
			"LEFT JOIN PersonT PriInsCoPersonT ON PriInsuranceCoT.PersonID = PriInsCoPersonT.ID "
			"LEFT JOIN HL7CodeLinkT PriHL7CodeLinkT ON PriHL7CodeLinkT.HL7GroupID = @HL7GroupID AND PriHL7CodeLinkT.Type = 5 AND PriHL7CodeLinkT.PracticeID = PriInsuranceCoT.PersonID "
			   ""
			"LEFT JOIN InsuredPartyT SecInsuredPartyT ON PatientsT.PersonID = SecInsuredPartyT.PatientID "
				"AND SecInsuredPartyT.RespTypeID = (SELECT ID FROM RespTypeT WHERE Priority = 2) "
			"LEFT JOIN RespTypeT SecInsuredPartyRespTypeT ON SecInsuredPartyT.RespTypeID = SecInsuredPartyRespTypeT.ID "
			"LEFT JOIN PersonT SecInsuredPartyPersonT ON SecInsuredPartyT.PersonID = SecInsuredPartyPersonT.ID "
			"LEFT JOIN InsurancePlansT SecInsurancePlanT ON SecInsuredPartyT.InsPlan = SecInsurancePlanT.ID "
			"LEFT JOIN InsuranceCoT SecInsuranceCoT ON SecInsuredPartyT.InsuranceCoID = SecInsuranceCoT.PersonID "
			"LEFT JOIN PersonT SecInsCoPersonT ON SecInsuranceCoT.PersonID = SecInsCoPersonT.ID "
			"LEFT JOIN HL7CodeLinkT SecHL7CodeLinkT ON SecHL7CodeLinkT.HL7GroupID = @HL7GroupID AND SecHL7CodeLinkT.Type = 5 AND SecHL7CodeLinkT.PracticeID = SecInsuranceCoT.PersonID "
		"WHERE LabsT.ID = @LabID AND LabsT.Deleted = 0",
		nHL7GroupID, nLabID);
	if(prs->eof) {
		ThrowNxException("CLabBarcode::FillBarcode : No labs exist for this form number.");
		return;
	}

	// go through all of the fields set to auto (at least the ones we want to fill anyway) and fill them in in the barcode.
	SetValue("H.1", GetValue("H.1") + "01.00");
	SetValue("H.2", GetCurrentSystemTime().Format("%Y%m%d"));

	SetValue("P.1", AsString(prs->Fields->Item["UserDefinedID"]->Value));
	SetValue("P.8", AsString(prs->Fields->Item["FormNumberTextID"]->Value));
	SetValue("P.9.1", AsString(prs->Fields->Item["PatientLast"]->Value));
	SetValue("P.9.2", AsString(prs->Fields->Item["PatientFirst"]->Value));
	SetValue("P.9.3", AsString(prs->Fields->Item["PatientMiddle"]->Value));
	SetValue("P.10", AsBarcodeDate(prs, "PatientDOB"));
	CString strGender;
	_variant_t varGender = prs->Fields->Item["PatientGender"]->Value;
	if (varGender.vt != VT_NULL)
	{	
		if (VarByte(varGender,0) == 1) {
			strGender = "M";
		} else if (VarByte(varGender,0) == 2) {
			strGender = "F";
		} else {
			strGender = "NI";
		}
	} else {
		// No gender
		strGender = "NI";
	}
	SetValue("P.11", strGender);
	SetValue("P.12", FormatBarcodeSSN(AsString(prs->Fields->Item["PatientSSN"]->Value)));
	SetValue("P.13", Trim(AsString(prs->Fields->Item["PatientAddress1"]->Value) + " " + AsString(prs->Fields->Item["PatientAddress2"]->Value)));
	SetValue("P.14", AsString(prs->Fields->Item["PatientCity"]->Value));
	SetValue("P.15", AsString(prs->Fields->Item["PatientState"]->Value));
	SetValue("P.16", AsString(prs->Fields->Item["PatientZip"]->Value));
	SetValue("P.17", FormatBarcodePhone(AsString(prs->Fields->Item["PatientHomePhone"]->Value)));
	SetValue("P.18", CalculateP18BillCode(prs, bFillMedicareNumberInP19, bFillSubscriberNumberInP40, bFillSubscriberNumberInP53));	
	SetValue("P.19", (bFillMedicareNumberInP19 ? AsString(prs->Fields->Item["LabInsuredPartyPolicyNumber"]->Value) : ""));
	// Responsible party is the one who pays for the lab. Could be the Insurance Company or Patient.
	SetValue("P.20.1", AsString(prs->Fields->Item["ResponsiblePartyLastName"]->Value));
	SetValue("P.20.2", AsString(prs->Fields->Item["ResponsiblePartyFirstName"]->Value));
	SetValue("P.20.3", AsString(prs->Fields->Item["ResponsiblePartyMiddleName"]->Value));
	SetValue("P.21", FormatBarcodeSSN(AsString(prs->Fields->Item["ResponsiblePartySSN"]->Value)));
	SetValue("P.22", Trim(AsString(prs->Fields->Item["ResponsiblePartyAddress1"]->Value) + " " + AsString(prs->Fields->Item["ResponsiblePartyAddress2"]->Value)));
	SetValue("P.23", AsString(prs->Fields->Item["ResponsiblePartyCity"]->Value));
	SetValue("P.24", AsString(prs->Fields->Item["ResponsiblePartyState"]->Value));
	SetValue("P.25", AsString(prs->Fields->Item["ResponsiblePartyZip"]->Value));
	SetValue("P.26", AsString(prs->Fields->Item["ResponsiblePartyEmployer"]->Value));
	CString strResponsiblePartyRelation = VarString(prs->Fields->Item["RespPartyRelationToPatient"]->Value, "Self");
	CString strResponsiblePartyRelationCode;
	if(strResponsiblePartyRelation.CompareNoCase("Self") == 0) {
		strResponsiblePartyRelationCode = "1";
	} else if (strResponsiblePartyRelation.CompareNoCase("Spouse") == 0) {
		strResponsiblePartyRelationCode = "2";
	} else {
		strResponsiblePartyRelationCode = "3";
	}
	SetValue("P.27", strResponsiblePartyRelationCode);
	SetValue("P.28", AsString(prs->Fields->Item["LabOrderingProviderID"]->Value));
	SetValue("P.29.1", AsString(prs->Fields->Item["LabOrderingProviderLastName"]->Value));
	SetValue("P.29.2", AsString(prs->Fields->Item["LabOrderingProviderFirstName"]->Value));
	SetValue("P.29.3", AsString(prs->Fields->Item["LabOrderingProviderMiddleName"]->Value));
	SetValue("P.30", AsString(prs->Fields->Item["LabOrderingProviderNPI"]->Value));
	SetValue("P.31", AsString(prs->Fields->Item["LabOrderingProviderNPI"]->Value));
	SetValue("P.32", AsString(prs->Fields->Item["LabOrderingProviderUPIN"]->Value));
	SetValue("P.34", AsString(prs->Fields->Item["PrimaryInsPayerCode"]->Value));
	SetValue("P.35", AsString(prs->Fields->Item["PrimaryInsName"]->Value));
	SetValue("P.36.1", AsString(prs->Fields->Item["PrimaryInsAddress1"]->Value));
	SetValue("P.36.2", AsString(prs->Fields->Item["PrimaryInsAddress2"]->Value));
	SetValue("P.37", AsString(prs->Fields->Item["PrimaryInsCity"]->Value));
	SetValue("P.38", AsString(prs->Fields->Item["PrimaryInsState"]->Value));
	SetValue("P.39", AsString(prs->Fields->Item["PrimaryInsZip"]->Value));
	SetValue("P.40", (bFillSubscriberNumberInP40 ? AsString(prs->Fields->Item["PrimaryInsPolicyNumber"]->Value) : ""));
	SetValue("P.41", AsString(prs->Fields->Item["PrimaryInsGroupNumber"]->Value));
	SetValue("P.42", AsString(prs->Fields->Item["PrimaryInsGroupName"]->Value));
	CString strSecondaryInsPayerCode = VarString(prs->Fields->Item["SecondaryInsPayerCode"]->Value, "");
	if(strSecondaryInsPayerCode.GetLength() > 2) {
		SetValue("P.43", strSecondaryInsPayerCode);
	} else {
		// LabCorp only requires P.43 filled with the code in the case when InsPayerCode is more than two characters, so don't fill it.
		// Note that this is different than the Primary Ins Payer Code, which we always fill in.
	}
	SetValue("P.44", AsString(prs->Fields->Item["SecondaryInsName"]->Value));
	SetValue("P.45.1", AsString(prs->Fields->Item["SecondaryInsAddress1"]->Value));
	SetValue("P.45.2", AsString(prs->Fields->Item["SecondaryInsAddress2"]->Value));
	SetValue("P.46", AsString(prs->Fields->Item["SecondaryInsCity"]->Value));
	SetValue("P.47", AsString(prs->Fields->Item["SecondaryInsState"]->Value));
	SetValue("P.48", AsString(prs->Fields->Item["SecondaryInsZip"]->Value));
	SetValue("P.49", AsString(prs->Fields->Item["SecondaryInsPolicyNumber"]->Value));
	SetValue("P.50", AsString(prs->Fields->Item["SecondaryInsGroupNumber"]->Value));
	SetValue("P.51", AsString(prs->Fields->Item["SecondaryInsGroupName"]->Value));
	SetValue("P.52", VarBool(prs->Fields->Item["WorkersComp"]->Value, FALSE) != FALSE ? "Y" : "N");
	if(bFillSubscriberNumberInP53) {
		// Medicaid number should be the subscriber number then. They only want the number here if it is the primary ins, so this
		//  will only get hit in that case.
		SetValue("P.53", AsString(prs->Fields->Item["PrimaryInsPolicyNumber"]->Value));
	} else {
		// Per the dictates of the specification, the value in P.18 determined that nothing goes in P.53.
	}
	// Skip P.54
	// Well, since we don't have discrete diagnosis codes, this is going to be rough.
	//  Following HL7Utils.cpp's lead in parsing out a diagnosis code.
	// (r.gonet 03/08/2014) - PLID 61190 - Only output the ICD-9 codes here. This field has been deprecated by LabCorp.
	for(int i = 0, nP55Counter = 0; i < saDiagnosisCodes.GetCount() && nP55Counter < 8; i++) {
		if(!aryDiagnosisCodesICD10[i]) {
			// (r.gonet 03/08/2014) - PLID 61190 - Only if it is ICD-9.
			CString strDiagnosisCode = saDiagnosisCodes[i];
			SetValue(FormatString("P.55.%li", ++nP55Counter), strDiagnosisCode);
		}		
	}
	SetValue("P.56", FormatBarcodePhone(AsString(prs->Fields->Item["RespPartyHomePhone"]->Value)));
	//TES 11/12/2012 - PLID 53709 - The order number in the barcode needs to include the specimen, because the order number in the HL7 message does.
	CString strOrderNumber = AdoFldString(prs, "FormNumberTextID");
	CString strSpecimen = AdoFldString(prs, "Specimen");
	if(!strSpecimen.IsEmpty()) {
		strOrderNumber += " - " + strSpecimen;
	}
	SetValue("P.57", strOrderNumber);
	SetValue("P.58", AsString(prs->Fields->Item["UserDefinedID"]->Value));
	SetValue("P.59", AsString(prs->Fields->Item["InitialDiagnosis"]->Value));
	_variant_t varBirthDate = prs->Fields->Item["PatientDOB"]->Value;
	if(varBirthDate.vt == VT_DATE && VarDateTime(varBirthDate).GetStatus() == COleDateTime::valid) {
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTime dtBirthDate = VarDateTime(varBirthDate);
		long nYearAge, nMonthAge, nDayAge;
		GetAgeInParts(dtBirthDate, dtNow, nYearAge, nMonthAge, nDayAge);
		SetValue("P.63", FormatString("%li", nYearAge));
		SetValue("P.64", FormatString("%li", nMonthAge));
		SetValue("P.65", FormatString("%li", nDayAge));
	} else {
		// Not a valid birthdate for this patient.
	}
	SetValue("P.71", AsString(prs->Fields->Item["LabOrderingProviderNPI"]->Value));
	BOOL bCCPatient = VarBool(prs->Fields->Item["CC_Patient"]->Value);
	if(bCCPatient) {
		SetValue("P.72.1", "P");
		// CC Patient to their address doesn't need a fax number or Attention name.
	} else {
		// Not CCing the patient about the lab results.
	}
	BOOL bCCRefPhys = VarBool(prs->Fields->Item["CC_RefPhys"]->Value);
	if(bCCRefPhys) {
		SetValue("P.73.1", "F");
		SetValue("P.73.2", FormatBarcodePhone(AsString(prs->Fields->Item["RefPhysFaxNumber"]->Value)));
		SetValue("P.73.3", AsString(prs->Fields->Item["RefPhysAttention"]->Value));
	} else {
		// Not CCing the Referring Physician about the lab results.
	}
	BOOL bCCPCP = VarBool(prs->Fields->Item["CC_PCP"]->Value);
	if(bCCPCP) {
		SetValue("P.74.1", "F");
		SetValue("P.74.2", FormatBarcodePhone(AsString(prs->Fields->Item["PCPFaxNumber"]->Value)));
		SetValue("P.74.3", AsString(prs->Fields->Item["PCPAttention"]->Value));
	} else {
		// Not CCing the Primary Care Physician about the lab results.
	}

	//TES 11/12/2012 - PLID 53665 - Pull the patient's race (this was trying to use custom fields before).
	//TES 12/6/2012 - PLID 53665 - Undoing this change for the time being
	//TES 1/3/2013 - PLID 53665 - Well, heck, this is hardcoded anyway, let's just hardcode to the LabCorp race codes.
	CString strRace = AdoFldString(prs, "PatientRace", "");
	CString strRaceChar, strRaceNum;
	if(strRace.CompareNoCase("White") == 0 || strRace.CompareNoCase("Caucasian") == 0) {
		strRaceChar = "C";
		strRaceNum = "1";
	}
	else if(strRace.CompareNoCase("Black") == 0 || strRace.CompareNoCase("African American") == 0 || strRace.CompareNoCase("African-American") == 0) {
		strRaceChar = "B";
		strRaceNum = "2";
	}
	else if(strRace.CompareNoCase("American Indian") == 0 || strRace.CompareNoCase("Native American") == 0) {
		strRaceChar = "I";
		strRaceNum = "3";
	}
	else if(strRace.CompareNoCase("Asian") == 0) {
		strRaceChar = "A";
		strRaceNum = "4";
	}
	else if(strRace.CompareNoCase("Hispanic") == 0) {
		strRaceChar = "H";
		//There isn't a character code for Hispanic, so just use the "Other" value, which is 5
		strRaceNum = "5";
	}
	else if(!strRace.IsEmpty()) {
		//Other
		strRaceChar = "O";
		strRaceNum = "5";
	}
	else {
		//Unspecified
		strRaceChar = "X";
		strRaceNum = "9";
	}
	SetValue("P.78", strRaceChar);
	SetValue("A.5", strRaceChar);
	SetValue("M.1", strRaceNum);

	SetValue("C.17", AsBarcodeDate(prs, "BiopsyDate"));
	// Oh boy. They totally flipped from the usual HL7-like standard and now we're doing fixed position.
	//  They request a second source and second clinical info, but we don't do this, so leave them blank.
	//TES 11/12/2012 - PLID 53708 - This should be ToBeOrdered, not ClinicalData
	CString strClinicalInfo = VarString(prs->Fields->Item["ToBeOrdered"]->Value, "");
	CString strAnatomicLocation = VarString(prs->Fields->Item["AnatomicLocation"]->Value, "");
	CString strC19Value = FormatString("%-24s  SRC:%-5s                 SRC:             ", strClinicalInfo.Left(24), strAnatomicLocation.Left(5));
	SetValue("C.19.2", strC19Value, false);

	//CString strCFTemplateName = VarString(prs->Fields->Item["CFTemplateName"]->Value, "");
	/*if(strCFTemplateName.MakeLower().Find("cytology") >= 0) {
		SetValue("C.22", "120");
	} else {*/
	SetValue("C.22", "EREQ"); // Apparently, do this always.
	/*}*/
	SetValue("C.25", AsBarcodeTime(prs, "BiopsyDate"));

	for(int i = 0; i < saLOINCCodes.GetCount() && i < 40; i++) {
		CString strLOINCCode = saLOINCCodes[i];
		SetValue(FormatString("T.%li", i + 1), strLOINCCode);
	}

	//TES 7/24/2012 - PLID 50393 - Now, clear out any parts that are assigned to a specific lab procedure, other than the one this lab is for.
	long nLabProcedureID = AdoFldLong(prs, "LabProcedureID", -1);
	CLabBarcodePartPtr pRoot = GetRoot();
	for(int nRecord = 1; nRecord < pRoot->GetChildCount(); nRecord++) {
		CLabBarcodePartPtr pRecord = pRoot->GetChild(nRecord);
		if(pRecord->GetLabProcedureID() != -1 && pRecord->GetLabProcedureID() != nLabProcedureID) {
			//TES 9/7/2012 - PLID 50393 - We want to clear out every value except the first one, that identifies the record
			for(int nValue = 1; nValue < pRecord->GetChildCount(); nValue++) {
				pRecord->GetChild(nValue)->ClearValues();
			}
		}
	}

	// (r.gonet 03/08/2014) - PLID 61190 - Set the D segment diagnosis codes. Up to 120. No indication about what code system is used.
	// Guess they are relying on the general uniqueness of the codes between systems.
	for(int i = 0; i < saDiagnosisCodes.GetCount() && i < 120; i++) {
		CString strDiagnosisCode = saDiagnosisCodes[i];
		SetValue(FormatString("D.1.%li", i + 1), strDiagnosisCode);
	} 
	// Free text diagnosis. This is what we do in P.59.
	SetValue("D.2", AsString(prs->Fields->Item["InitialDiagnosis"]->Value));


	// Yes, we cheat with the padded 0s. But come on, them using atol is easier for us.
	// We'll set the real length further down once we know it.
	SetValue("L.1", "0000");

	// Here is something strange, since we don't know the length yet, we don't know what to put here.
	//  But we have to put something otherwise the length will not calculate properly.
	SetValue("E.1", "0");

	// Now this is just strange! We have to put the length of the barcode string here, but we have to include
	//  the length itself in the calculation. Not so good when the rest of the string is 999.
	CString strBarcode = GetRoot()->ToString();
	long nBarcodeLength = strBarcode.GetLength();

	// The error code basically defines what we have cut out of the barcode. We have to do this before we have cut it!
	CString strErrorCode = "0";
	if(nBarcodeLength > 1040) {
		// We need to figure out what can be cut. We have available to us T, C, and A.
		strBarcode = GetRoot()->ToString();

		// Get the T segment
		long nTBegin = strBarcode.Find("\rT|");
		nTBegin = nTBegin >= 0 ? nTBegin + 1 : -1;
		long nTEnd = nTBegin >= 0 ? strBarcode.Find("\r", nTBegin) : 0;
		CString strTSegment = nTBegin >= 0 ? strBarcode.Mid(nTBegin, nTEnd - nTBegin + 1) : "";

		// Get the C segment
		long nCBegin = strBarcode.Find("\rC|");
		nCBegin = nCBegin >= 0 ? nCBegin + 1 : -1;
		long nCEnd = nCBegin >= 0 ? strBarcode.Find("\r", nCBegin) : 0;
		CString strCSegment = nCBegin >= 0 ? strBarcode.Mid(nCBegin, nCEnd - nCBegin + 1) : "";

		// Get the A segment
		long nABegin = strBarcode.Find("\rA|");
		nABegin = nABegin >= 0 ? nABegin + 1 : -1;
		long nAEnd = nABegin >= 0 ? strBarcode.Find("\r", nABegin) : 0;
		CString strASegment = nABegin >= 0 ? strBarcode.Mid(nABegin, nAEnd - nABegin + 1) : "";

		long nFirstBarcodeLength = nBarcodeLength;
		nFirstBarcodeLength -= strTSegment.GetLength(); // Remove T and see if we are under the limit
		strErrorCode = "1"; // T was moved to overflow barcode
		if(nFirstBarcodeLength > 1040) {
			nFirstBarcodeLength -= strCSegment.GetLength(); // Remove C and see if we are under the limit
			strErrorCode = "2"; // T and C were moved to overflow barcode
			if(nFirstBarcodeLength > 1040) {
				nFirstBarcodeLength -= strASegment.GetLength(); // Remove A and see if we are under the limit
				strErrorCode = "3"; // T, C, and A were moved to overflow barcode
				if(nFirstBarcodeLength > 1040) {
					// There is no other behavior defined in the LabCorp specification and this should not occur with the other character length limits
					//  set on barcode parts.
					ThrowNxException("Barcode's length is greater than maximum and cannot be shrunk further.");
				}
			}
		}
		// Fill the length of the primary barcode now that we know what it is going to be
		SetValue("L.1", FormatString("%04li", nFirstBarcodeLength));
	} else {
		// Fill the length of the barcode now that we know what it is going to be
		SetValue("L.1", FormatString("%04li", nBarcodeLength));
	}
	// If we are moving any segments to the overflow barcode, then this will be non-zero, else it is zero.
	SetValue("E.1", strErrorCode);
}

// (r.gonet 11/11/2011) - PLID 46434 - LabCorp specific. Splits the barcode into two parts if it is over a certain character length. 
void CLabCorpLabBarcode::SplitBarcode(OUT CArray<CString, CString> &aryBarcodes)
{
	if(GetRoot() == NULL) {
		return;
	}

	CString strBarcode = GetRoot()->ToString();
	if(strBarcode.GetLength() == 0) {
		// (r.gonet 11/30/2011) - PLID 46434 - An empty barcode is still a barcode and will be encoded like it actually has data in it.
		//  Let's not let that happen.
		return;
	}
	CString strSecondBarcode = "";
	CString strSecondLSegment = "L|0000|\r";
	CString strSecondESegment = "E|0|\r";
	if(GetPart("E.1") != NULL && GetValue("E.1") != "0") {
		long nFirstBarcodeLength = strBarcode.GetLength();

		// Keep trying to get below the 1040 limit by moving segments to the overflow barcode.
		// Try to remove T to the overflow barcode.
		long nTBegin = strBarcode.Find("\rT|");
		if(nTBegin >= 0) {
			nTBegin = nTBegin + 1;
			long nTEnd = nTBegin >= 0 ? strBarcode.Find("\r", nTBegin) : 0;
			CString strTSegment = nTBegin >= 0 ? strBarcode.Mid(nTBegin, nTEnd - nTBegin + 1) : "";

			nFirstBarcodeLength = strBarcode.GetLength();
			nFirstBarcodeLength -= strTSegment.GetLength();
			strBarcode = strBarcode.Left(nTBegin) + strBarcode.Mid(nTEnd + 1);
			strSecondBarcode += strTSegment;
		}
		// Is that short enough now?
		if(nFirstBarcodeLength > 1040) {
			// No? Try to remove C to the overflow barcode.
			long nCBegin = strBarcode.Find("\rC|");
			if(nCBegin >= 0) {
				nCBegin = nCBegin + 1;
				long nCEnd = nCBegin >= 0 ? strBarcode.Find("\r", nCBegin) : 0;
				CString strCSegment = nCBegin >= 0 ? strBarcode.Mid(nCBegin, nCEnd - nCBegin + 1) : "";
				
				nFirstBarcodeLength -= strCSegment.GetLength();
				strBarcode = strBarcode.Left(nCBegin) + strBarcode.Mid(nCEnd + 1);
				strSecondBarcode += strCSegment;
			}
			// Is that short enough now?
			if(nFirstBarcodeLength > 1040) {
				// No? Try to remove A to the overflow barcode.
				long nABegin = strBarcode.Find("\rA|");
				if(nABegin >= 0) {
					nABegin = nABegin + 1;
					long nAEnd = nABegin >= 0 ? strBarcode.Find("\r", nABegin) : 0;
					CString strASegment = nABegin >= 0 ? strBarcode.Mid(nABegin, nAEnd - nABegin + 1) : "";
					
					nFirstBarcodeLength -= strASegment.GetLength();
					strBarcode = strBarcode.Left(nABegin) + strBarcode.Mid(nAEnd + 1);
					strSecondBarcode += strASegment;
				}
				// That has to be short enough now or we are out of luck.
				if(nFirstBarcodeLength > 1040) {
					// What? I don't know what to do any longer.
					ASSERT(FALSE);
					ThrowNxException("Barcode's length is greater than maximum and cannot be shrunk further.");
				}
			}
		}

		// Calculate the new length of the overflow barcode.
		long nBarcodeLength = strSecondBarcode.GetLength() + strSecondESegment.GetLength() + strSecondLSegment.GetLength();
		strSecondLSegment = FormatString("L|%04li|\r", nBarcodeLength);

		strSecondBarcode += strSecondLSegment;
		strSecondBarcode += strSecondESegment;
		aryBarcodes.Add(strSecondBarcode);
	}

	aryBarcodes.InsertAt(0, strBarcode);
}



// (r.gonet 12/12/2011) - PLID 46434 - Returns an age in years, months, and days.
void CLabCorpLabBarcode::GetAgeInParts(COleDateTime &dtBirthDate, COleDateTime &dtNow, OUT long &nYearAge, OUT long &nMonthAge, OUT long &nDayAge)
{
	long nYearDOB = dtBirthDate.GetYear();
	long nMonthDOB = dtBirthDate.GetMonth();
	long nDayDOB = dtBirthDate.GetDay();

	long nYearNow = dtNow.GetYear();
	long nMonthNow = dtNow.GetMonth();
	long nDayNow = dtNow.GetDay();

	nYearAge = nYearNow - nYearDOB;

	if(nMonthNow >= nMonthDOB) {
		nMonthAge = nMonthNow - nMonthDOB;
	} else {
		nYearAge--;
		nMonthAge = 12 + nMonthNow - nMonthDOB;
	}

	if(nDayNow >= nDayDOB) {
		nDayAge = nDayNow - nDayDOB;
	} else {
		nMonthAge--;
		nDayAge = 31 + nDayNow - nDayDOB;

		if(nMonthAge < 0) {
			nMonthAge = 11;
			nYearAge--;
		}
	}
}

// (r.gonet 11/11/2011) - PLID 46434 - Encode the barcode into PDF417
void CLabCorpLabBarcode::Encode(OUT CArray<CString, CString> &aryBarcodes)
{
	SplitBarcode(aryBarcodes);
	PDF417Lib::IPDFPtr pdf417Encoder(__uuidof(PDF417Lib::PDF));
	// We should always have at least one barcode if the barcode is not blank
	if(aryBarcodes.GetSize() > 0) {
		CString strBarcode = aryBarcodes[0];
		BSTR bstrOutputBuffer = NULL;
		if(!SUCCEEDED(pdf417Encoder->FontEncode(_bstr_t(strBarcode), 4, 19, 0, 0, PDF417Lib::Text, 0, &bstrOutputBuffer))) {
			ThrowNxException("CLabBarcode::Encode : Error trying to encode barcode.");
		}
		strBarcode = CString(static_cast<const char*>(_bstr_t(bstrOutputBuffer)));
		SysFreeString(bstrOutputBuffer);
		aryBarcodes[0] = strBarcode;
	} else {
		// We have no primary barcode. Must be that the barcode is blank.
	}

	// Overflow barcode
	if(aryBarcodes.GetSize() > 1) {
		CString strBarcode = aryBarcodes[1];
		BSTR bstrOutputBuffer = NULL;
		if(!SUCCEEDED(pdf417Encoder->FontEncode(_bstr_t(strBarcode), 4, 4, 0, 0, PDF417Lib::Text, 0, &bstrOutputBuffer))) {
			ThrowNxException("CLabBarcode::Encode : Error trying to encode overflow barcode.");
		}
		strBarcode = CString(static_cast<const char*>(_bstr_t(bstrOutputBuffer)));
		SysFreeString(bstrOutputBuffer);
		aryBarcodes[1] = strBarcode;
	} else {
		// We have no overflow barcode. Must be that the primary barcode was either blank or not > 1040 characters.
	}
}

//TES 7/24/2012 - PLID 50393 - Added, used to clear parts that are associated with a different lab procedure.
void CLabBarcodePart::ClearValues()
{
	m_strTextValue = "";
	for(int i = 0; i < m_aryChildren.GetCount(); i++ ) {
		m_aryChildren[i]->ClearValues();
	}
}