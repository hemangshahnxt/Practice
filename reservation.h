#pragma once

// (c.haag 2010-02-11 09:52) - PLID 37221 - The singleday import now happens from this source file. We rename the
// Reservation object to guarantee no legacy code is directly accessing reservations like we used to.
#import "SingleDay.tlb" rename("Delete", "DeleteRes") rename("IReservation", "_P_IReservation") rename("_DSingleDay", "_P_DSingleDay")

// (c.haag 2010-02-04 12:04) - PLID 37221 - Wrapper class for explicitly accessing SingleDay Reservation objects
class CReservation
{
private:
	SINGLEDAYLib::_P_IReservationPtr m_pRes; // The reservation this class encapsulates	
	CString m_strOwner; // (c.haag 2010-04-30 11:32) - PLID 38379 - We now retain the name of the owner

private:
	// (c.haag 2010-04-30 12:03) - PLID 38379 - One-time initialization
	void Init();

public:
	// Construction
	CReservation(const CString& strOwner)
	{
		Init();
		m_strOwner = strOwner;
		m_pRes = NULL;
	}
	CReservation(const CString& strOwner, SINGLEDAYLib::_P_IReservationPtr pRes)
	{
		Init();
		m_strOwner = strOwner;
		m_pRes = pRes;
	}
	// Destruction
	~CReservation();

public:
	// Compares this object with a dispatch
	BOOL operator ==(LPDISPATCH p) {
		return (m_pRes == p);
	}
	BOOL operator !=(LPDISPATCH p) {
		return (m_pRes != p);
	}
	// Compares this object with another wrapper object
	BOOL operator ==(const CReservation& p) {
		return (m_pRes == p.m_pRes);
	}
	BOOL operator !=(const CReservation& p) {
		return (m_pRes != p.m_pRes);
	}

public:
	// (c.haag 2010-04-30 12:25) - PLID 38379 - Returns the owner of the reservation
	const CString& GetOwner() const { return m_strOwner; }
	// (c.haag 2010-05-26 15:00) - PLID 38379 - Sets the owner of the reservation. We
	// should only use this when reservations are member variables of class objects.
	void SetOwner(const CString& strOwner) { m_strOwner = strOwner; }

public:
	// Dispatch property
	operator LPDISPATCH() { return (LPDISPATCH)m_pRes; }
	LPDISPATCH GetDispatch() { return m_pRes; }
	void SetDispatch(LPDISPATCH p) { m_pRes = p; }

public:
	// (c.haag 2010-04-30 11:32) - PLID 38379 - Returns TRUE if m_pRes is NULL
	BOOL IsNull() { return (NULL == m_pRes) ? TRUE : FALSE; }

public:
	// Returns the singleday control that the reservation belongs to
	IDispatchPtr GetSingleDayCtrl() { return m_pRes->GetSingleDayCtrl(); }

public:
	// Explicit referencing/dereferencing
	void AddRef() {
		// (c.haag 2010-03-19 11:47) - PLID 37221 - This function is intended to encapsulate the
		// call to ISmartPtr->AddRef(). I don't know the difference between ISmartPtr.AddRef() and
		// ISmartPtr->AddRef(), so I cannot comment on that unfortunately. Old calls to:
		//
		// IReservationPtr pRes = foo;
		// pRes->AddRef()
		//
		// Were replaced with:
		//
		// CReservation res = foo;
		// res.AddRef();
		//
		// Only call CReservation::AddRef if you're sure you know what you're doing.
		//
		m_pRes->AddRef();
	}

	void Release() { 
		// (c.haag 2010-03-19 11:32) - PLID 37221 - Smart pointers have two kinds of releases:
		//
		// p->Release(): This decrements the ref count but leaves the internal pointer intact
		// p.Release(): This decrements the reference count and clears the internal pointer
		//
		// This function is intended to emulate p->Release(). Where in Practice you used to see:
		//
		// IReservationPtr pRes = foo;
		// pRes->Release();
		//
		// You will now see
		//
		// CReservation res = foo;
		// res.Release();
		//
		// Only call CReservation::Release() if you're sure you know what you're doing.
		//
		m_pRes->Release(); 
	}

	// (c.haag 2010-05-06) - PLID 38655 - Smart pointers have two kinds of releases:
	//
	// p->Release(): This decrements the ref count but leaves the internal pointer intact
	// p.Release(): This decrements the reference count and clears the internal pointer
	//
	// This function is intended to emulate p.Release()
	void ReleaseAndClear()
	{
		m_pRes.Release();
	}

private:
	// (c.haag 2010-07-12 17:47) - PLID 39614 - Logs and traces output
	void LogAndTrace(const CString& strOut);

public:
	// Detach and delete methods

	// (c.haag 2010-07-12 17:47) - PLID 39614 - This will delete a reservation and log if a function
	// that isn't the caller also holds a reference to it.
	// (j.gruber 2011-02-17 14:56) - PLID 42425 - added the option not to resolve
	HRESULT DeleteRes(const CString& strCaller, BOOL bResolve = TRUE);

	SINGLEDAYLib::_P_IReservation* Detach() { return m_pRes.Detach(); }
	
	// Every method as of here are pass-throughs to the internal object
public:
	// (a.walling 2013-01-21 16:48) - PLID 54747 - Access to Data[] property map
	__declspec(property(get=GetData,put=PutData)) _variant_t Data[];
	_variant_t GetData (_bstr_t key) { return m_pRes->GetData(key); }
	void PutData(_bstr_t key, const _variant_t& value) { m_pRes->PutData(key, value); }

public:
	long GetDay() { return m_pRes->GetDay(); }
	long GetTopSlot() { return m_pRes->GetTopSlot(); }
	long GetBottomSlot() { return m_pRes->GetBottomSlot(); }
	OLE_HANDLE GethWnd() { return m_pRes->GethWnd(); }

public:
	void PutAllowDrag(long n) { m_pRes->PutAllowDrag(n); }
	void PutAllowResize(long n) { m_pRes->PutAllowResize(n); }
	void PutMouseEnabled(long n) { m_pRes->PutMouseEnabled(n); }

public:
	long GetPersonID() { return m_pRes->GetPersonID(); }
	void PutPersonID(long n) { m_pRes->PutPersonID(n); }

	long GetReservationID() { return m_pRes->GetReservationID(); }
	void PutReservationID(long n) { m_pRes->PutReservationID(n); }

	long GetAptTypeID() { return m_pRes->GetAptTypeID(); }
	void PutAptTypeID(long n) { m_pRes->PutAptTypeID(n); }

	long GetTemplateItemID() { return m_pRes->GetTemplateItemID(); }
	void PutTemplateItemID(long n) { m_pRes->PutTemplateItemID(n); }

	_bstr_t GetText() { return m_pRes->GetText(); }
	void PutText(_bstr_t t) { m_pRes->PutText(t); }

	// (c.haag 2010-03-19 12:25) - This class is intended to be a pass-through to
	// the inner reservation. We need to use our types consistently.
	OLE_COLOR GetBorderColor() { return m_pRes->GetBorderColor(); }
	void PutBorderColor(OLE_COLOR c) { m_pRes->PutBorderColor(c); }

	OLE_COLOR GetBackColor() { return m_pRes->GetBackColor(); }
	void PutBackColor(OLE_COLOR c) { m_pRes->PutBackColor(c); }

	OLE_COLOR GetForeColor() { return m_pRes->GetForeColor(); }
	void PutForeColor(OLE_COLOR c) { m_pRes->PutForeColor(c); }

	DATE GetStartTime() { return m_pRes->GetStartTime(); }
	void PutStartTime(DATE d) { m_pRes->PutStartTime(d); }

	DATE GetEndTime() { return m_pRes->GetEndTime(); }
	void PutEndTime(DATE d) { m_pRes->PutEndTime(d); }
};

class CSingleDay
{
public:
	SINGLEDAYLib::_P_DSingleDayPtr m_pCtrl;

private:
	// (c.haag 2010-04-30 11:32) - PLID 38379 - Logs and traces output
	void LogAndTrace(const CString& strOut);

	// (c.haag 2010-04-30 11:32) - PLID 38379 - Checks to see if Practice owns a reference to
	// any reservations. This is called in functions where we expect Practice to have none.
	void WarnOfOpenReservations(const CString& strGrandCaller, const CString& strCaller);

public:
	// Construction
	CSingleDay()
	{
		m_pCtrl = NULL;
	}
	CSingleDay(SINGLEDAYLib::_P_DSingleDayPtr pCtrl)
	{
		m_pCtrl = pCtrl;
	}
	// Destruction
	~CSingleDay();

public:
	// Compares this object with a dispatch
	BOOL operator ==(LPDISPATCH p) {
		return (m_pCtrl == p);
	}
	BOOL operator !=(LPDISPATCH p) {
		return (m_pCtrl != p);
	}
	// Compares this object with another wrapper object
	BOOL operator ==(const CSingleDay& p) {
		return (m_pCtrl == p.m_pCtrl);
	}
	BOOL operator !=(const CSingleDay& p) {
		return (m_pCtrl != p.m_pCtrl);
	}

public:
	// Dispatch property
	operator LPDISPATCH() { return (LPDISPATCH)m_pCtrl; }
	LPDISPATCH GetDispatch() { return m_pCtrl; }
	void SetDispatch(LPDISPATCH p) { m_pCtrl = p; }

public:
	// Explicit referencing/dereferencing
	void AddRef() {
		// This function is intended to encapsulate the call to ISmartPtr->AddRef(). I don't know the difference 
		// between ISmartPtr.AddRef() and ISmartPtr->AddRef(), so I cannot comment on that unfortunately. Old calls to:
		//
		// ISingleDayPtr pRes = foo;
		// pRes->AddRef()
		//
		// Were replaced with:
		//
		// CSingleDay res = foo;
		// res.AddRef();
		//
		// Only call CSingleDay::AddRef if you're sure you know what you're doing.
		//
		m_pCtrl->AddRef();
	}

	void Release() { 
		// Smart pointers have two kinds of releases:
		//
		// p->Release(): This decrements the ref count but leaves the internal pointer intact
		// p.Release(): This decrements the reference count and clears the internal pointer
		//
		// This function is intended to emulate p->Release(). Where in Practice you used to see:
		//
		// CSingleDay pRes = foo;
		// pRes->Release();
		//
		// You will now see
		//
		// CSingleDay res = foo;
		// res.Release();
		//
		// Only call CSingleDay::Release() if you're sure you know what you're doing.
		//
		m_pCtrl->Release(); 
	}

public:
	// Returns TRUE if a reservation exists. Call this wherever GetReservation() is called such
	// that the result is only compared with NULL and discarded.
	BOOL HasReservation(long Day, long Index);

	// Just like GetReservation but returns an LPDISPATCH; an unencapsulated reservation object.
    LPDISPATCH GetReservationRaw(long Day, long Index);

	// Every method as of here are pass-throughs to the internal object
public:
	HRESULT Refresh () { return m_pCtrl->Refresh(); }
    long DayPos (
		long Index ) { return m_pCtrl->DayPos(Index); }
    long SlotPos (
		long Index ) { return m_pCtrl->SlotPos(Index); }
    _bstr_t SlotTime (
		long Index ) { return m_pCtrl->SlotTime(Index); }
	HRESULT ResolveHorizontal ( ) { return m_pCtrl->ResolveHorizontal(); }
	HRESULT ResolveVertical ( ) { return m_pCtrl->ResolveVertical(); }
	HRESULT Resolve ( ) { return m_pCtrl->Resolve(); }
	long AvailableWidth ( ) { return m_pCtrl->AvailableWidth(); }
    HRESULT MakeSlotVisible (
		long SlotNum ) { return m_pCtrl->MakeSlotVisible(SlotNum); }
    long GetSlotVisible (
		long SlotNum ) { return m_pCtrl->GetSlotVisible(SlotNum); }
    CReservation GetReservation (const CString& strOwner,
        long Day,
		long Index) { return CReservation(strOwner, m_pCtrl->GetReservation(Day, Index)); }
	HRESULT RefreshHorizontal ( ) { return m_pCtrl->RefreshHorizontal(); }
    HRESULT Print (
        IUnknown * printerDC,
		IUnknown * printInfo ) { return m_pCtrl->Print(printerDC, printInfo); }
	HRESULT Clear (const CString& strCaller);
	// (c.haag 2010-04-30 12:03) - PLID 38379 - We now have a parameter for the owner name
    CReservation AddReservation (
		const CString& strOwner,
        short Day,
        DATE StartTime,
        DATE EndTime,
        OLE_COLOR Color,
		VARIANT_BOOL Batch) { return CReservation(strOwner, m_pCtrl->AddReservation(Day, StartTime, EndTime, Color, Batch)); }
    short GetTimeSlot (
		DATE theTime ) { return m_pCtrl->GetTimeSlot(theTime); }
	HRESULT RefreshBoth ( ) { return m_pCtrl->RefreshBoth(); }
    HRESULT Shift ( const CString& strCaller,
		short nNumDays );
    long AddBlock (
        short nDay,
        DATE dtStartTime,
        DATE dtEndTime,
        OLE_COLOR nColor,
        _bstr_t strText,
		VARIANT_BOOL bBatch ) { return m_pCtrl->AddBlock(nDay, dtStartTime, dtEndTime, nColor, strText, bBatch); }
	HRESULT HideBlock ( long nIndex ) { return m_pCtrl->HideBlock(nIndex); }
	long RemoveBlock ( long nIndex ) { return m_pCtrl->RemoveBlock(nIndex); }
    HRESULT ShowBlock (
		long nIndex ) { return m_pCtrl->ShowBlock(nIndex); }
    HRESULT RefreshBlock (
		long nIndex ) { return m_pCtrl->RefreshBlock(nIndex); }
    HRESULT SetTimeButtonColors (
        DATE dtStartTime,
        DATE dtEndTime,
		OLE_COLOR nColor ) { return m_pCtrl->SetTimeButtonColors(dtStartTime, dtEndTime, nColor); }
    long GetReservationCount (
		short nDay ) { return m_pCtrl->GetReservationCount(nDay); }
    OLE_COLOR GetBlockTextColor (
		OLE_COLOR clrBlockBackground ) { return m_pCtrl->GetBlockTextColor(clrBlockBackground); }
	HRESULT AboutBox ( ) { return m_pCtrl->AboutBox(); }

public:
    // Properties:
	long GetSlotHeight ( ) const { return m_pCtrl->GetSlotHeight(); }
	void PutSlotHeight ( long _val ) { m_pCtrl->PutSlotHeight(_val); }
	long GetDayWidth ( ) const { return m_pCtrl->GetDayWidth(); }
	void PutDayWidth ( long _val ) { m_pCtrl->PutDayWidth(_val); }
	long GetTimeButtonWidth ( ) const { return m_pCtrl->GetTimeButtonWidth(); }
	void PutTimeButtonWidth ( long _val ) { m_pCtrl->PutTimeButtonWidth(_val); }
	VARIANT_BOOL GetTimeButtonVisible ( ) const { return m_pCtrl->GetTimeButtonVisible(); }
	void PutTimeButtonVisible ( VARIANT_BOOL _val ) { m_pCtrl->PutTimeButtonVisible(_val); }
	long GetSlotTotalCount ( ) const { return m_pCtrl->GetSlotTotalCount(); }
	void PutSlotTotalCount ( long _val ) { m_pCtrl->PutSlotTotalCount(_val); }
	long GetSlotVisibleCount ( ) const { return m_pCtrl->GetSlotVisibleCount(); }
	void PutSlotVisibleCount ( long _val ) { m_pCtrl->PutSlotVisibleCount(_val); }
	long GetDayVisibleCount ( ) const { return m_pCtrl->GetDayVisibleCount(); }
	void PutDayVisibleCount ( long _val ) { m_pCtrl->PutDayVisibleCount(_val); }
	long GetDayTotalCount ( ) const { return m_pCtrl->GetDayTotalCount(); }
	void PutDayTotalCount ( const CString& strCaller, long _val );
	long GetInterval ( ) const { return m_pCtrl->GetInterval(); }
	void PutInterval ( long _val ) { m_pCtrl->PutInterval(_val); }
	_bstr_t GetBeginTime ( ) const { return m_pCtrl->GetBeginTime(); }
	void PutBeginTime ( _bstr_t _val ) { m_pCtrl->PutBeginTime(_val); }
	_bstr_t GetEndTime ( ) const { return m_pCtrl->GetEndTime(); }
	void PutEndTime ( _bstr_t _val ) { m_pCtrl->PutEndTime(_val); }
	OLE_HANDLE GethWnd ( ) const { return m_pCtrl->GethWnd(); }
	void PuthWnd ( OLE_HANDLE _val ) { m_pCtrl->PuthWnd(_val); }
	long GetTopSlot ( ) const { return m_pCtrl->GetTopSlot(); }
	void PutTopSlot ( long _val ) { m_pCtrl->PutTopSlot(_val); }
	inline long GetLeftDay ( ) const { return m_pCtrl->GetLeftDay(); }
	void PutLeftDay ( long _val ) { m_pCtrl->PutLeftDay(_val); }
	long GetSelDay ( ) const  { return m_pCtrl->GetSelDay(); }
	void PutSelDay ( long _val ) { m_pCtrl->PutSelDay(_val); }
	long GetFirstSelSlot ( ) const { return m_pCtrl->GetFirstSelSlot(); }
	void PutFirstSelSlot ( long _val ) { m_pCtrl->PutFirstSelSlot(_val); }
	long GetLastSelSlot ( ) const { return m_pCtrl->GetLastSelSlot(); }
	void PutLastSelSlot ( long _val ) { m_pCtrl->PutLastSelSlot(_val); }
	long GetSelLeft ( ) const { return m_pCtrl->GetSelLeft(); }
	void PutSelLeft ( long _val ) { m_pCtrl->PutSelLeft(_val); }
	long GetSelTop ( ) const { return m_pCtrl->GetSelTop(); }
	void PutSelTop ( long _val ) { m_pCtrl->PutSelTop(_val); }
	long GetSelWidth ( ) const { return m_pCtrl->GetSelWidth(); }
	void PutSelWidth ( long _val ) { m_pCtrl->PutSelWidth(_val); }
	long GetSelHeight ( ) const { return m_pCtrl->GetSelHeight(); }
	void PutSelHeight ( long _val ) { m_pCtrl->PutSelHeight(_val); }
	VARIANT_BOOL GetAutoRefresh ( ) const { return m_pCtrl->GetAutoRefresh(); }
	void PutAutoRefresh ( VARIANT_BOOL _val ) { m_pCtrl->PutAutoRefresh(_val); }
	OLE_COLOR GetDefaultReservationBackColor ( ) const { return m_pCtrl->GetDefaultReservationBackColor(); }
	void PutDefaultReservationBackColor ( OLE_COLOR _val ) { m_pCtrl->PutDefaultReservationBackColor(_val); }
	OLE_COLOR GetDefaultReservationBorderColor ( ) const { return m_pCtrl->GetDefaultReservationBorderColor(); }
	void PutDefaultReservationBorderColor ( OLE_COLOR _val ) { m_pCtrl->PutDefaultReservationBorderColor(_val); }
	short GetDefaultReservationBorderWidth ( ) const { return m_pCtrl->GetDefaultReservationBorderWidth(); }
	void PutDefaultReservationBorderWidth ( short _val ) { m_pCtrl->PutDefaultReservationBorderWidth(_val); }
	VARIANT_BOOL GetReadOnly ( ) const { return m_pCtrl->GetReadOnly(); }
	void PutReadOnly ( VARIANT_BOOL _val ) { m_pCtrl->PutReadOnly(_val); }
	VARIANT_BOOL GetSnapReservationsToGrid ( ) const { return m_pCtrl->GetSnapReservationsToGrid(); }
	void PutSnapReservationsToGrid ( VARIANT_BOOL _val ) { m_pCtrl->PutSnapReservationsToGrid(_val); }
	VARIANT_BOOL GetShowTooltips ( ) const { return m_pCtrl->GetShowTooltips(); }
	void PutShowTooltips ( VARIANT_BOOL _val ) { m_pCtrl->PutShowTooltips(_val); }
	VARIANT_BOOL GetShowArrows ( ) const { return m_pCtrl->GetShowArrows(); }
	void PutShowArrows ( VARIANT_BOOL _val ) { m_pCtrl->PutShowArrows(_val); }
	VARIANT_BOOL GetRepositionText ( )  const { return m_pCtrl->GetRepositionText(); }
	void PutRepositionText ( VARIANT_BOOL _val ) { m_pCtrl->PutRepositionText(_val); }
	VARIANT_BOOL GetEnabled ( ) const { return m_pCtrl->GetEnabled(); }
	void PutEnabled ( VARIANT_BOOL _val ) { m_pCtrl->PutEnabled(_val); }
	VARIANT_BOOL GetCompactBlockText ( ) const { return m_pCtrl->GetCompactBlockText(); }
	void PutCompactBlockText ( VARIANT_BOOL _val ) { m_pCtrl->PutCompactBlockText(_val); }
	VARIANT_BOOL GetUseSimpleForeColors ( ) const { return m_pCtrl->GetUseSimpleForeColors(); }
	void PutUseSimpleForeColors ( VARIANT_BOOL _val ) { m_pCtrl->PutUseSimpleForeColors(_val); }
	OLE_COLOR GetHighlightColor ( ) const { return m_pCtrl->GetHighlightColor(); } //TES 3/8/2011 - PLID 41519
	void PutHighlightColor ( OLE_COLOR _val ) { m_pCtrl->PutHighlightColor(_val); } //TES 3/8/2011 - PLID 41519
	void SetHighlightRes ( LPDISPATCH _val ) const { m_pCtrl->SetHighlightRes(_val); } //TES 3/8/2011 - PLID 41519
	LPDISPATCH GetHighlightRes ( ) const { return m_pCtrl->GetHighlightRes(); } //TES 3/8/2011 - PLID 41519

};

// (c.haag 2010-04-30 12:03) - PLID 38379 - This is a special wrapper that lets a member function of a class
// temporarily claim ownership of a reservation that is owned by the class object. We do this strictly for debugging
// purposes. It doesn't help to see a report that cites CResEntryDlg::m_res is owned by the CResEntryDlg class; we
// want to know what function was using it last.
class CClaimReservation
{
protected:
	CReservation& m_Res;
	CString strOldOwner;
public:
	CClaimReservation(CReservation& res, const CString& strNewOwner) : m_Res(res)
	{
		strOldOwner = res.GetOwner();
		m_Res.SetOwner(strNewOwner);
	}
	~CClaimReservation()
	{
		m_Res.SetOwner(strOldOwner);
	}
};
