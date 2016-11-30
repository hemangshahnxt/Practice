#include "stdafx.h"
#include "Search_DataListProvider.h"
#include "DiagSearchUtils.h"
#include "NxAPI.h"

using namespace ADODB;
//(s.dhole 2/2/2015 5:02 PM ) - PLID 64610 Added new base class to support more generic searchtool framework implimentation
// This is an abstract class.  You cannot instantiate this class itself, but should instead use the derived 
//	classes.  If you need to define a new search format, you should derive a new class from this one.  
// Most of the code  moved from  CDiagSearch_DataListProvide
namespace LookupSearch
{
	CSearch_DataListProvider::CSearch_DataListProvider()
		:m_pModuleState(AfxGetModuleState())
	{
		m_nRefCount = 1;
	}


	//(s.dhole 2/27/2015 9:13 AM ) - PLID 64610 make sure we destroy any icon , if it is created in datalist
	CSearch_DataListProvider::~CSearch_DataListProvider()
	{
	}


	//This is the actual Load() called by the datalist.  We abstract it away from our derived classes due to 
	//	memory management and type safety issues.  It actually wants a _RecordsetPtr, but for various reasons
	//	(see the datalist implementation), we have to lower our interface to an IDispatch*.  This is confusing
	//	to anyone who may derive and not know what is needed.  Instead, we'll implement Load() and we'll let
	//	the derived classes implement the more strongly typed LoadListData().
	//
	//Also, we will handle the memory management so all the derived classes don't need to remember this stuff.  We
	//	are giving up ownership of the memory when passing it off to the datalist.
	IDispatch * __stdcall CSearch_DataListProvider::Load(BSTR bstrSearchString)
	{
		AFX_MANAGE_STATE(m_pModuleState);

		try {
			//Call the virtual function that will be implemented by the derived classes.
			_RecordsetPtr prs = LoadListData(bstrSearchString);

			//We must relinquish ownership of the memory to the datalist, otherwise it won't be able to work with the results.
			//	We are NOT in charge of deallocating that memory.
			IDispatch* pDisp = prs.Detach();
			return pDisp;
		}
		catch (...) {
			//Remember this code is probably being executed by the datalist, not by Practice itself.  We don't want any exceptions
			//	to wander off into NxDatalist code.  Let's catch them here and just fail outright.
			::MessageBox(NULL, NxCatch::GetErrorMessage(__FUNCTION__), "Query error", MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_SETFOREGROUND | MB_TOPMOST);
			return NULL;
		}
	}

	//We apparently need to implement these ourselves when deriving from IUnknown.  I just copied them from CAuditing_JITCellValue in Practice
	HRESULT STDMETHODCALLTYPE CSearch_DataListProvider::QueryInterface(/* [in] */ REFIID riid, /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		if (riid == IID_IUnknown || riid == __uuidof(NXDATALIST2Lib::IDataListProvider)) {
			AddRef();
			*ppvObject = this;
			return S_OK;
		}
		else {
			return E_NOINTERFACE;
		}
	}

	//We apparently need to implement these ourselves when deriving from IUnknown.  I just copied them from CAuditing_JITCellValue in Practice
	ULONG STDMETHODCALLTYPE CSearch_DataListProvider::AddRef(void)
	{
		return InterlockedIncrement(&m_nRefCount);
	}

	//We apparently need to implement these ourselves when deriving from IUnknown.  I just copied them from CAuditing_JITCellValue in Practice
	ULONG STDMETHODCALLTYPE CSearch_DataListProvider::Release(void)
	{
		if (m_nRefCount == 0) {
			return 0;
		}
		LONG lResult = InterlockedDecrement(&m_nRefCount);
		if (lResult == 0) {
			delete this;
		}
		return lResult;
	}

	
	CSearch_Api_DataListProvider::CSearch_Api_DataListProvider()
	{
	}

	CSearch_Api_DataListProvider::~CSearch_Api_DataListProvider()
	{
	}

	//(s.dhole 3/6/2015 9:42 AM ) - PLID 64610 This class  handel most of linking between column filed and api search field value
	void CSearch_Api_DataListProvider::Register(NXDATALIST2Lib::_DNxDataListPtr pdl)
	{
		pdl->PutDataListProvider(this);
		m_pdl = pdl;
	}

	//(s.dhole 3/6/2015 9:46 AM ) - PLID 64610 Try to interpret  ado data type from Datlist column data type
	ADODB::DataTypeEnum InterpolateColumnAdoFieldInfo(NXDATALIST2Lib::IColumnSettingsPtr pCol, OUT long &nLen)
	{
		switch (pCol->GetFieldType()) {
		case NXDATALIST2Lib::EColumnFieldType::cftBool10:
		case NXDATALIST2Lib::EColumnFieldType::cftBoolCheckbox:
		case NXDATALIST2Lib::EColumnFieldType::cftBoolTrueFalse:
		case NXDATALIST2Lib::EColumnFieldType::cftBoolYesNo:
			nLen = 2;
			return adBoolean;
		case NXDATALIST2Lib::EColumnFieldType::cftNumberBasic:
		case NXDATALIST2Lib::EColumnFieldType::cftNumberCommas:
		case NXDATALIST2Lib::EColumnFieldType::cftBitmapBuiltIn:
		case NXDATALIST2Lib::EColumnFieldType::cftSetRowForeColor:
		case NXDATALIST2Lib::EColumnFieldType::cftSetRowBackColor:
		case NXDATALIST2Lib::EColumnFieldType::cftSetRowForeColorSel:
		case NXDATALIST2Lib::EColumnFieldType::cftSetRowBackColorSel:
			nLen = 4;
			return adInteger;
		case NXDATALIST2Lib::EColumnFieldType::cftNumberCurrency:
			nLen = 8;
			return adCurrency;
		case NXDATALIST2Lib::EColumnFieldType::cftDateLong:
		case NXDATALIST2Lib::EColumnFieldType::cftDateShort:
		case NXDATALIST2Lib::EColumnFieldType::cftDateAndTime:
		case NXDATALIST2Lib::EColumnFieldType::cftDateNatural:
		case NXDATALIST2Lib::EColumnFieldType::cftTime:
			nLen = 8;
			return adDate;
		case NXDATALIST2Lib::EColumnFieldType::cftTextRichText:
		case NXDATALIST2Lib::EColumnFieldType::cftTextWordWrap:
		case NXDATALIST2Lib::EColumnFieldType::cftTextWordWrapLink:
			nLen = -1; 
			return adVarChar;
		case NXDATALIST2Lib::EColumnFieldType::cftTextSingleLine:
		case NXDATALIST2Lib::EColumnFieldType::cftTextSingleLineLink:
			if (pCol->GetDataType() == VT_I4) {
				nLen = 4;
				return adInteger;
			}
			else if (pCol->GetDataType() == VT_BSTR) {
				nLen = -1; 
				return adVarChar;
			}
			// (j.jones 2015-03-24 12:07) - PLID 65425 - supported currency
			else if (pCol->GetDataType() == VT_CY) {
				nLen = 8;
				return adCurrency;
			}
			else {
				ThrowNxException("Could not interpolate ADO type information from datalist column field type and data type.");
			}
			break;
		default:
			ThrowNxException("Could not interpolate ADO type information from datalist column field type.");
		}
	}


	//(s.dhole 3/6/2015 9:46 AM ) - PLID 64610 Load search result result
	/*override*/ ADODB::_RecordsetPtr CSearch_Api_DataListProvider::LoadListData(BSTR bstrSearchString)
	{
		CWaitCursor pWait;

		// Do the search and get the results in memory
		// (b.cardillo 2014-03-28 15:09) - PLID 61595 - Fixed memory leak by only calling pCrosswalkResults->SearchResults once
		SAFEARRAY *pSearchResults = Search(bstrSearchString);

		//we now have to convert these results into a recordset
		// (b.cardillo 2014-03-27 14:13) - PLID 61564 - Several speed improvements for converting into the recordset

		// Create the recordset object to have fields based on the datalist we're registered with
		_RecordsetPtr rs;
		rs.CreateInstance(__uuidof(ADODB::Recordset));
		rs->CursorLocation = adUseClient;
		FieldsPtr flds = rs->Fields;
		for (int i = 0, count = m_pdl->GetColumnCount(); i < count; i++) {
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pdl->GetColumn(i);
			long nLen = -1;
			ADODB::DataTypeEnum edt = InterpolateColumnAdoFieldInfo(pCol, OUT nLen);
			flds->Append(pCol->GetFieldName(), edt, nLen, adFldIsNullable);
		}
		rs->Open(g_cvarMissing, g_cvarMissing, adOpenUnspecified, adLockUnspecified, adCmdUnspecified);

		// (b.cardillo 2014-03-27 14:13) - PLID 61564 - We rely on setting the field values in order anyway, so we might 
		// as well use ordinals instead of field names for maximum performance.
		// Prepare a safearray of the field ordinals.
		_variant_t varFieldList;
		{
			Nx::SafeArray<VARIANT> saryFieldList;
			for (int i = 0, count = flds->GetCount(); i<count; i++) {
				saryFieldList.Add((long)i);
			}
			varFieldList.Attach(saryFieldList.AsVariant().Detach());
		}

		long nCountResults = 0;

		if (pSearchResults != NULL) {
			Nx::SafeArray<IUnknown *> saResults(pSearchResults);

			// (b.cardillo 2014-03-27 14:13) - PLID 61564 - Calc only once for speed boost
			// Prepare the bounds struct ahead of time as it's the same for every iteration
			SAFEARRAYBOUND bound;
			bound.cElements = flds->GetCount();
			bound.lLbound = 0;

			for each(IUnknown *pResult in saResults) {

				nCountResults++;

				//add all our values into a safearray
				// (b.cardillo 2014-03-27 14:13) - PLID 61564 - Speed boost: create all elements in the array at once
				_variant_t varValues;
				{
					LPSAFEARRAY lpsaValues = ::SafeArrayCreate(VT_VARIANT, 1, &bound);
					if (lpsaValues != NULL) {
						VARIANT *ary;
						HRESULT hr = SafeArrayAccessData(lpsaValues, (void **)&ary);
						if (hr == S_OK) {
							// Write values to the safearray elements
							GetFieldValues(ary, bound.cElements, pResult);

							// Hand the elements into the safe array
							hr = SafeArrayUnaccessData(lpsaValues);
							if (hr == S_OK) {
								// Success, move the whole safearray object into our variant for use (and destruction) later
								varValues.vt = VT_ARRAY | VT_VARIANT;
								varValues.parray = lpsaValues;
							} else {
								::SafeArrayDestroy(lpsaValues);
							}
						} else {
							::SafeArrayDestroy(lpsaValues);
						}
					}
				}

				//add the new record using our field list and value list
				rs->AddNew(varFieldList, varValues);
			}
		}

		if (nCountResults == 0) {
			//add a "no results found" row
			SAFEARRAYBOUND bound;
			bound.cElements = flds->GetCount();
			bound.lLbound = 0;
			_variant_t varValues;
			{
				LPSAFEARRAY lpsaValues = ::SafeArrayCreate(VT_VARIANT, 1, &bound);
				if (lpsaValues != NULL) {
					VARIANT *ary;
					HRESULT hr = SafeArrayAccessData(lpsaValues, (void **)&ary);
					if (hr == S_OK) {
						// Write values to the safearray elements
						HRESULT hrIncludeNoResultsRow = GetFieldValues_IfNoResults(ary, bound.cElements);

						// Hand the elements into the safe array
						hr = SafeArrayUnaccessData(lpsaValues);
						if (hr == S_OK && hrIncludeNoResultsRow == S_OK) {
							// Success, move the whole safearray object into our variant for use (and destruction) later
							varValues.vt = VT_ARRAY | VT_VARIANT;
							varValues.parray = lpsaValues;
						} else {
							::SafeArrayDestroy(lpsaValues);
						}
					} else {
						::SafeArrayDestroy(lpsaValues);
					}
				}
			}

			//add the new record using our field list and value list
			if (varValues.vt != VT_EMPTY) {
				rs->AddNew(varFieldList, varValues);
			}
		}

		//start back at the beginning of the recordset
		rs->MoveFirst();

		return rs;
	}
}