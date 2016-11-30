#pragma once


//(s.dhole 2/2/2015 5:02 PM ) - PLID 64610 Added new base class to support more generic searchtool framework implimentation
// This is an abstract class.  You cannot instantiate this class itself, but should instead use the derived 
//	classes.  If you need to define a new search format, you should derive a new class from this one.  
namespace LookupSearch
{

	class CSearch_DataListProvider : public NXDATALIST2Lib::IDataListProvider
	{
	public:
		CSearch_DataListProvider();
		virtual ~CSearch_DataListProvider();

		//This is the actual Load() called by the datalist.  We abstract it away from our derived classes due to 
		//	memory management and type safety issues.  It actually wants a _RecordsetPtr, but for various reasons
		//	(see the datalist implementation), we have to lower our interface to an IDispatch*.  This is confusing
		//	to anyone who may derive and not know what is needed.  Instead, we'll implement Load() and we'll let
		//	the derived classes implement the more strongly typed LoadListData().
		//See .cpp for memory management concerns.
		IDispatch * __stdcall Load(BSTR bstrSearchString);

		//Derive me!  Simply return a recordset that will be used by the datalist to load the dropdown list in a search.
		virtual ADODB::_RecordsetPtr LoadListData(BSTR bstrSearchString) = 0;

	public:

		// COM  implimentation
		//We apparently need to implement these ourselves when deriving from IUnknown.  I just copied them from CAuditing_JITCellValue in Practice
		HRESULT STDMETHODCALLTYPE QueryInterface(/* [in] */ REFIID riid, /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
		ULONG STDMETHODCALLTYPE AddRef(void);
		ULONG STDMETHODCALLTYPE Release(void);
		LONG m_nRefCount; // Reference count


	private:

		//Maintain the module state when the object is created
		AFX_MODULE_STATE* m_pModuleState;
	};
	
	//(s.dhole 3/6/2015 9:42 AM ) - PLID 64610 This class  handel most of linking between column filed and api search field value
	class CSearch_Api_DataListProvider : public CSearch_DataListProvider
	{
	public:
		CSearch_Api_DataListProvider();
		virtual ~CSearch_Api_DataListProvider();

		/*override*/ ADODB::_RecordsetPtr LoadListData(BSTR bstrSearchString);

		NXDATALIST2Lib::_DNxDataListPtr m_pdl;

		void Register(NXDATALIST2Lib::_DNxDataListPtr pdl);

	public:
		// Execute the search based on the given search string and return all results in the form of 
		// a safearray of IUnknown *'s
		virtual SAFEARRAY *Search(BSTR bstrSearchString) = 0;

		// Populate the array of field values for a row to be added based on the given result.
		// NOTE: The order of values placed in the array must exactly match the datalist column order.
		virtual void GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResult) = 0;

		// Populate the array of field values for a single row that will be used if the search 
		// yielded no results. If you don't want to have a "no results" row, then return E_NOTIMPL. 
		// Otherwise, populate aryFieldValues as desired for the "no results" row and return S_OK.
		// NOTE: The order of values placed in the array must exactly match the datalist column order.
		virtual HRESULT GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount) = 0;

	};
};

