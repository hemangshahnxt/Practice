#include "stdafx.h"

#include "pracprops.h"
#include "marketutils.h"
#include "NxStandard.h"
#include "client.h"
#include "DocBar.h"
#include "DateTimeUtils.h"
#include "GraphDescript.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;


/////////////////////////////////////////////////////////////////
// CTreeLong implementation
/////////////////////////////////////////////////////////////////

CTreeLong::CTreeLong(long nValue /*= 0*/, CPtrArray *paryptlChildren /*= NULL*/)
{
	m_nValue = nValue;
	m_paryptlChildren = paryptlChildren;
};

CTreeLong::~CTreeLong()
{
	Clear();
};

void CTreeLong::Clear()
{
	if (m_paryptlChildren) {
		for (long i=0; i<m_paryptlChildren->GetSize(); i++) {
			delete (CTreeLong *)(m_paryptlChildren->GetAt(i));
		}
		delete m_paryptlChildren;
		m_paryptlChildren = NULL;
	}
}

void CTreeLong::AddNewChild(long nValue /*= 0*/, CPtrArray *paryptlChildren /*= NULL*/)
{
	AddChild(new CTreeLong(nValue, paryptlChildren));
}

void CTreeLong::AddChild(CTreeLong *pChild)
{
	if (m_paryptlChildren == NULL) {
		m_paryptlChildren = new CPtrArray;
	}
	m_paryptlChildren->Add(pChild);
}

POSITION CTreeLong::GetFirstChildPosition() const
{
	if (m_paryptlChildren && m_paryptlChildren->GetSize() > 0) {
		return (POSITION)1;
	} else {
		return NULL;
	}
}

BOOL CTreeLong::GetNextChild(IN OUT POSITION *pp, OUT const CTreeLong **ppChild) const
{
	if (m_paryptlChildren && (*pp)) {
		long nIndex = (long)(*pp) - 1;
		if (nIndex >= 0 && nIndex < m_paryptlChildren->GetSize()) {
			*ppChild = (const CTreeLong *)m_paryptlChildren->GetAt(nIndex);
			(*pp)++;
			return TRUE;
		} else {
			return FALSE;
		}
	} else {
		return FALSE;
	}
}

BOOL CTreeLong::EnumDescendents(ENUMDESCENDENTSPROC lpEnumDescendentsProc, LPVOID pParam, BOOL bIncludeSelf) const
{
	// Optionally call the processing function on ourselves
	if (bIncludeSelf) {
		// Call the processing function on our own value
		if (!lpEnumDescendentsProc(m_nValue, pParam)) {
			// The processing function asked us to stop iterating
			return FALSE;
		}
	}
	
	// Now iterate through all our descendents, and call the processing function for each
	if (m_paryptlChildren) {
		for (long i=0; i<m_paryptlChildren->GetSize(); i++) {
			const CTreeLong *ptl = (const CTreeLong *)m_paryptlChildren->GetAt(i);
			if (ptl->m_nValue == 13198) {
				Sleep(0);
			}
			// Recurse, starting with the child itself (which is why we pass TRUE asking it to include itself)
			if (!ptl->EnumDescendents(lpEnumDescendentsProc, pParam, TRUE)) {
				// The processing function asked us to stop iterating
				return FALSE;
			}
		}
	}

	// If we made it here we finished processing all the descendents as requested, so we have success
	return TRUE;
}

CMapLongToTreePtr::CMapLongToTreePtr(long nRootKey)
{
	m_nRootKey = nRootKey;
}

CMapLongToTreePtr::~CMapLongToTreePtr()
{
	RemoveAll();
}

BOOL CMapLongToTreePtr::RemoveKey(long nKey)
{
	// If it's our root then deallocate it
	if (nKey == m_nRootKey) {
		CTreeLong *pRootTree;
		if (Lookup(nKey, pRootTree)) {
			// Found the root tree pointer, it better point to something
			ASSERT(pRootTree);
			delete pRootTree;
		}
	}

	// Then just clal the normal remove
	return CMap<long,long,CTreeLong*,CTreeLong*>::RemoveKey(nKey);
}

void CMapLongToTreePtr::RemoveAll()
{
	CTreeLong *pRootTree;
	if (Lookup(m_nRootKey, pRootTree)) {
		// Found the root tree pointer, it better point to something
		ASSERT(pRootTree);
		delete pRootTree;
	}
	CMap<long,long,CTreeLong*,CTreeLong*>::RemoveAll();
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

//TES 6/4/2008 - PLID 30206 - Moved some functions to be members of CMarketingDlg.

_RecordsetPtr g_rsReferrals = NULL, g_rsReferralsPlus = NULL, g_rsCategories;
//TES 6/7/2004: g_referralDataChecker tracks g_rsReferrals; g_referralDataPlusChecker tracks g_rsReferralsPlus, g_referralArrayChecker tracks g_parAllReferrals.
CTableChecker g_referralDataChecker(NetUtils::ReferralSourceT), g_referralDataPlusChecker(NetUtils::ReferralSourceT), g_referralArrayChecker(NetUtils::ReferralSourceT), 
	g_categoryChecker(NetUtils::CPTCategories), g_RefPhysChecker(NetUtils::RefPhys), g_RefPatChecker(NetUtils::PatCombo);

int clamp (int min, int max, int value)
{
	if (value < min)
		return min;
	else if (value > max)
		return max;
	else return value;
}

CArray<int, int> *g_parAllReferralsPlus = NULL;

CString Descendants(int id, CString field/*="Name"*/)
{
	BOOL bDeleteArray = FALSE;

	//returns a string for a WHERE clause using all descendants of the source id
	CString sql, strNumber;
	CArray<int,int> *array;
	//TES 4/8/2004:  Keep a filled-in list of all the referrals handy, to save time.
	if(id == -1 && g_parAllReferralsPlus && !g_referralArrayChecker.Changed() && !g_RefPhysChecker.Changed() && !g_RefPatChecker.Changed()) {
		array = g_parAllReferralsPlus;
	}
	else {
		array = new CArray<int, int>;
		array->Add(id);
		Descendants(*array);
		if(id == -1 && !g_parAllReferralsPlus) {
			//Set the global array.
			g_parAllReferralsPlus = array;
		}
		else {
			bDeleteArray = TRUE;
		}
	}

	if (!array->GetSize())
	{
		sql = field + " IN (-1)";
	}
	else
	{
		sql = field + " IN (";
		for (int i = 0; i < array->GetSize(); i++)
		{	strNumber.Format ("%i,", array->GetAt(i));
			sql += strNumber;
		}
		sql.SetAt(sql.GetLength() - 1, ')');//replace the last , with a )
	}

	if(bDeleteArray)
		delete array;

	return sql;
}

//TES 7/23/2009 - PLID 34227 - Added an overload that takes an array, for use when filtering the Marketing->Referral
// tab on multiple referral sources.
CString Descendants(const CDWordArray &arIDs, CString field/*="Name"*/)
{
	BOOL bDeleteArray = FALSE;

	//returns a string for a WHERE clause using all descendants of the source id
	CString sql, strNumber;
	CArray<int,int> arMaster;
	for(int i = 0; i < arIDs.GetSize(); i++) {
		CArray<int,int> *array;
		//TES 4/8/2004:  Keep a filled-in list of all the referrals handy, to save time.
		long id = (long)arIDs[i];
		if(id == -1 && g_parAllReferralsPlus && !g_referralArrayChecker.Changed() && !g_RefPhysChecker.Changed() && !g_RefPatChecker.Changed()) {
			array = g_parAllReferralsPlus;
		}
		else {
			array = new CArray<int, int>;
			array->Add(id);
			Descendants(*array);
			if(id == -1 && !g_parAllReferralsPlus) {
				//Set the global array.
				g_parAllReferralsPlus = array;
			}
			else {
				bDeleteArray = TRUE;
			}
		}

		arMaster.Append(*array);

		if(bDeleteArray)
			delete array;
	}

	if (!arMaster.GetSize())
	{
		sql = field + " IN (-1)";
	}
	else
	{
		sql = field + " IN (";
		for (int i = 0; i < arMaster.GetSize(); i++)
		{	strNumber.Format ("%i,", arMaster.GetAt(i));
			sql += strNumber;
		}
		sql.SetAt(sql.GetLength() - 1, ')');//replace the last , with a )
	}

	return sql;
}

CString Descendants(int id, CStringArray &saFieldNames)
{
	//returns a string for a WHERE clause using all descendants of the source id
	CStringArray saSqlParts;
	CString sql, strNumber;
	CArray<int,int> array;
	array.Add(id);
	Descendants(array);

	saSqlParts.SetSize(saFieldNames.GetSize());
	if (!array.GetSize())
	{
		for(int i = 0; i < saFieldNames.GetSize(); i++) {
			saSqlParts.SetAt(i, saFieldNames.GetAt(i) + " IN (-1)");
		}
	}
	else
	{
		for(int i = 0; i < saFieldNames.GetSize(); i++) {
			CString strPart = saFieldNames.GetAt(i) + " IN (";
			for (int j = 0; j < array.GetSize(); j++)
			{	
				strNumber.Format ("%i,", array[j]);
				strPart += strNumber;
			}
			strPart.SetAt(strPart.GetLength() - 1, ')');//replace the last , with a )
			saSqlParts.SetAt(i, strPart);
		}
	}
	sql = "(" + saSqlParts.GetAt(0);
	for(int i = 1; i < saSqlParts.GetSize(); i++) {
		sql += " OR " + saSqlParts.GetAt(i);
	}
	sql += ")";
	return sql;
}

void ReferralMultiRefresh(CTableChecker **p, long id /*=-1*/)
{
	g_referralDataChecker.MultiRefresh(p, id);
	g_referralArrayChecker.MultiRefresh(p, id);

}

//not really marketing related, but this code is copied from the referral tree, and may as well be here
void CategoryMultiRefresh(CTableChecker **p, long id /*=-1*/)
{
	g_categoryChecker.MultiRefresh(p, id);
}


void EnsureReferralData(bool force/*=false*/)
{
	if (g_rsReferrals == NULL)
	{
		g_rsReferrals = CreateRecordset(adOpenStatic,
											adLockReadOnly,
											"SELECT PersonID, Parent, Name FROM ReferralSourceT ORDER BY Name");
		g_rsReferrals->PutRefActiveConnection(NULL);//disconnect
	}
	else if (g_referralDataChecker.Changed() || force)
	{
		//reconnect and requery if someone changed something
		_ConnectionPtr pCon = GetRemoteData();
		if (pCon)
			g_rsReferrals->PutRefActiveConnection(pCon);
		
		//if pCon is invalid, calling this anyway will throw an exception, which is what we want
		g_rsReferrals->Requery(-1);
		g_rsReferrals->PutRefActiveConnection(NULL);
	}
}

CMapLongToTreePtr g_mtlReferralsPlusMap(-1);

void RefillReferralsPlusTree()
{
	// Empty the whole tree
	if (!g_mtlReferralsPlusMap.IsEmpty()) {
		// Call removeall which will automatically deallocate the root tree, which will deallocate all nodes.
		g_mtlReferralsPlusMap.RemoveAll();
	}

	// Now for each record in the referralsplus list, place it in the right node
	if (!g_rsReferralsPlus->eof || !g_rsReferralsPlus->bof) {
		// Move to the first record, just in case we're not already there
		FieldsPtr pflds = g_rsReferralsPlus->GetFields();
		FieldPtr fldPersonID = pflds->GetItem("PersonID");
		FieldPtr fldParent = pflds->GetItem("Parent");
		g_rsReferralsPlus->MoveFirst();
		while (!g_rsReferralsPlus->eof) {
			// Create the element if it doesn't already exist.  If it DOES exist, it may have children but it 
			// better not have a value of its own yet because the recordset should have all unique values in 
			// the PersonID column.
			CTreeLong *pThisElement = NULL;
			{
				long nPersonID = AdoFldLong(fldPersonID);
				if (g_mtlReferralsPlusMap.Lookup(nPersonID, pThisElement)) {
					// Got the existing element that may have children but musn't have its own value.  Set its value.
					ASSERT(pThisElement->m_nValue == 0);
					pThisElement->m_nValue = nPersonID;
				} else {
					// The element didn't already exist (this is usually the case), so create it with the right value and no children, and place it in the map in the right element location.
					pThisElement = new CTreeLong(nPersonID, NULL);
					g_mtlReferralsPlusMap.SetAt(pThisElement->m_nValue, pThisElement);
				}
			}
			// Also put it under its parent in the tree
			{
				// Get the parent element under which this element belongs
				long nParent = AdoFldLong(fldParent);
				CTreeLong *pParentElement;
				if (g_mtlReferralsPlusMap.Lookup(nParent, pParentElement)) {
					// It existed, so we're good to go
				} else {
					// It didn't already exist, so add it but don't leave its value 0 so that we can 
					// assert later that its value is unset.
					pParentElement = new CTreeLong;
					g_mtlReferralsPlusMap.SetAt(nParent, pParentElement);
				}
				pParentElement->AddChild(pThisElement);
			}
			// Next record
			g_rsReferralsPlus->MoveNext();
		}
	}
}

void EnsureReferralsPlus()
{
	if(g_rsReferralsPlus == NULL) {
		g_rsReferralsPlus = CreateRecordset(adOpenStatic, 
			adLockReadOnly, 
			"SELECT PersonID, Parent, Name FROM "
			"(SELECT PersonID, Parent, Name FROM ReferralSourceT "
			"UNION SELECT -2 AS PersonID, -1 AS Parent, 'Referring Physician' AS Name "
			"UNION SELECT ID AS PersonID, -2 AS Parent, Last + ', ' + First + ' ' + Middle AS Name "
			"	FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.DefaultReferringPhyID "
			"UNION SELECT -3 AS PersonID, -1 AS Parent, 'Referring Patient' AS Name "
			"UNION SELECT ID AS PersonID, -3 AS Parent, Last + ', ' + First + ' ' + Middle AS Name "
			"	FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.ReferringPatientID "
			") AS ReferralsQ ORDER BY Name");
		g_rsReferralsPlus->PutRefActiveConnection(NULL);//disconnect

		// (b.cardillo 2004-08-05 17:59) - PLID 13747 - Now fill our mapped tree for speed
		RefillReferralsPlusTree();
	}
	else if (g_referralDataPlusChecker.Changed() || g_RefPhysChecker.Changed() || g_RefPatChecker.Changed()) {
		//Reconnect and requery.
		_ConnectionPtr pCon = GetRemoteData();
		if(pCon)
			g_rsReferralsPlus->PutRefActiveConnection(pCon);
		g_rsReferralsPlus->Requery(-1);
		g_rsReferralsPlus->PutRefActiveConnection(NULL);

		// (b.cardillo 2004-08-05 17:59) - PLID 13747 - Now fill our mapped tree for speed
		RefillReferralsPlusTree();
	}
}

void EnsureCategoryData(bool force/*=false*/)
{
	if (g_rsCategories == NULL)
	{
		g_rsCategories = CreateRecordset(adOpenStatic, 
										adLockReadOnly, 
										"SELECT ID, Parent, Name FROM CategoriesT ORDER BY Name");
		g_rsCategories->PutRefActiveConnection(NULL);//disconnect
	}
	else if (g_categoryChecker.Changed() || force)
	{
		//reconnect and requery if someone changed something
		_ConnectionPtr pCon = GetRemoteData();
		if (pCon)
			g_rsCategories->PutRefActiveConnection(pCon);
		
		//if pCon is invalid, calling this anyway will throw an exception, which is what we want
		g_rsCategories->Requery(-1);
		g_rsCategories->PutRefActiveConnection(NULL);
	}
}

// Make a flat array out of the given tree
void CopyAllDescendentsToArray(IN const CTreeLong *ptl, IN OUT CArray<int,int> &array)
{
	const CTreeLong *ptlChild;
	for (POSITION p=ptl->GetFirstChildPosition(); ptl->GetNextChild(&p, &ptlChild); ) {
		ASSERT(ptlChild);
		array.Add(ptlChild->m_nValue);
		CopyAllDescendentsToArray(ptlChild, array);
	}
}

/* (b.cardillo 2004-08-05 18:01) - PLID 13747 - Leaving this stuff here just for reference 
// so we can see what it used to do.  Eventually (once we're completely finished with all 
// marketing speed optimizations and we've confirmed to our satisfaction that everything's 
// still correct) we can get rid of this old code.

void DescendantsOldWay(CArray<int,int> &array)
{
	EnsureReferralsPlus();

	if (g_rsReferralsPlus->eof && g_rsReferralsPlus->bof)
	{
		array.RemoveAll();
		return;
	}

	for (int i = 0; i < array.GetSize(); i++)
	{	
		g_rsReferralsPlus->MoveFirst();
		while (!g_rsReferralsPlus->eof)
		{	
			if (g_rsReferralsPlus->Fields->GetItem("Parent")->Value.lVal == array.GetAt(i))
				array.Add(g_rsReferralsPlus->Fields->GetItem("PersonID")->Value.lVal);
			g_rsReferralsPlus->MoveNext();
		}
	}
}

// Returns TRUE if neither array contains an elements that cannot be found in the other
BOOL CompareArraysIgnoreOrder(const CArray<int,int> &ary1, const CArray<int,int> &ary2)
{
	// For every element in ary1, make sure that element is found in ary2
	{
		for (long i=0; i<ary1.GetSize(); i++) {
			int nVal1 = ary1.GetAt(i);
			BOOL bFoundIn2 = FALSE;
			for (long j=0; j<ary2.GetSize(); j++) {
				if (ary2.GetAt(j) == nVal1) {
					bFoundIn2 = TRUE;
					break;
				}
			}
			if (bFoundIn2 == FALSE) {
				// Found one in ary1 that wasn't in ary2
				return FALSE;
			}
		}
	}
	// For every element in ary2, make sure that element is found in ary1
	{
		for (long i=0; i<ary2.GetSize(); i++) {
			int nVal2 = ary2.GetAt(i);
			BOOL bFoundIn1 = FALSE;
			for (long j=0; j<ary1.GetSize(); j++) {
				if (ary1.GetAt(j) == nVal2) {
					bFoundIn1 = TRUE;
					break;
				}
			}
			if (bFoundIn1 == FALSE) {
				// Found one in ary2 that wasn't in ary1
				return FALSE;
			}
		}
	}

	// If we made it here, the sets are identical
	return TRUE;
}

void CopyArray(OUT CArray<int,int> &aryCopyTo, IN const CArray<int,int> &aryCopyFrom)
{
	aryCopyTo.RemoveAll();
	for (long i=0; i<aryCopyFrom.GetSize(); i++) {
		aryCopyTo.Add(aryCopyFrom.GetAt(i));
	}
}

void DEBUG_CompareOldAndNewWay(const CArray<int,int> &array)
{
	CArray<int,int> aryOldWay;
	CopyArray(aryOldWay, array);
	DescendantsOldWay(aryOldWay);

	CArray<int,int> aryNewWay;
	CopyArray(aryNewWay, array);
	Descendants(aryNewWay);

	if (!CompareArraysIgnoreOrder(aryOldWay, aryNewWay)) {
		MessageBox(NULL, "bad!", "no!", MB_OK);
	}
}
*/

// (b.cardillo 2004-08-05 18:04) - PLID 13747 - This function deprecated, please use 
//   void Descendants(long nID, const CTreeLong **pptlTreeLong)
// instead because it's much faster (no copying of values).
void Descendants(CArray<int,int> &array)
{
	/* (b.cardillo 2004-08-05 18:03) - PLID 13747 - For debugging purposes compare the two 
	// ways to make sure they're identical.
	//DEBUG_CompareOldAndNewWay(array);
	*/

	// Now do it the new way because the program has to proceed
	EnsureReferralsPlus();

	if (!g_mtlReferralsPlusMap.IsEmpty()) {
		long nSize = array.GetSize();
		for (long i=0; i<nSize; i++) {
			// Get the tree that starts at the given ID
			CTreeLong *ptl;
			if (g_mtlReferralsPlusMap.Lookup(array.GetAt(i), ptl)) {
				// Got that node, all we have to do is copy all its descendents
				CopyAllDescendentsToArray(ptl, array);
			} else {
				// There was nothing there, that's unexpected
				ASSERT(FALSE);
			}
		}
	} else {
		array.RemoveAll();
	}
}

//(e.lally 2009-09-18) PLID 35300 - Added Include No Referral Entry which manually adds it to the
	//referral plus map, or ensures it is removed from the map.
void Descendants(long nID, const CTreeLong **pptlTreeLong, BOOL bIncludeNoReferralEntry /*=FALSE*/)
{
	EnsureReferralsPlus();

	if(bIncludeNoReferralEntry){
		//(e.lally 2009-09-18) PLID 35300 - The No Referral entry is supposed to be in the map
		CTreeLong *pElement = NULL;
		if(!g_mtlReferralsPlusMap.Lookup(NOREFERRAL_SENTINEL_ID, pElement)){
			//It wasn't, add it
			pElement = new CTreeLong(NOREFERRAL_SENTINEL_ID, NULL);
			g_mtlReferralsPlusMap.SetAt(pElement->m_nValue, pElement);
		}
	}
	else{
		//(e.lally 2009-09-18) PLID 35300 - The No Referral entry is not supposed to be in our map
		CTreeLong *pElement = NULL;
		if(g_mtlReferralsPlusMap.Lookup(NOREFERRAL_SENTINEL_ID, pElement)){
			//It was, remove it
			if(pElement){
				delete pElement;
				pElement = NULL;
			}
			g_mtlReferralsPlusMap.RemoveKey(NOREFERRAL_SENTINEL_ID);
		}
	}


	if (!g_mtlReferralsPlusMap.IsEmpty()) {
		// Get the tree that starts at the given ID
		CTreeLong *ptl;
		if (g_mtlReferralsPlusMap.Lookup(nID, ptl)) {
			// Got that node, pass it to our caller in const form so the caller can't change it.
			*pptlTreeLong = ptl;
		} else {
			// There was nothing there, that's unexpected because if EnsureReferralsPlus() did 
			// its job, there should be a map entry for every valid id.
			ASSERT(FALSE);
			*pptlTreeLong = NULL;
		}
	} else {
		// Again, if EnsureReferralsPlus() did its job, there should be a map entry for every 
		// valid id.  So either there are no valid ids and so why should this function be called 
		// at all, or there were valid ids but they aren't in the map.  Either way, this should 
		// never happen.
		ASSERT(FALSE);
		*pptlTreeLong = NULL;
	}
}


//copied from referral code

CArray<int, int> *g_parAllCategoriesPlus = NULL;
CString CategoryDescendants(int id, CString field/*="Name"*/)
{
	BOOL bDeleteArray = FALSE;

	//returns a string for a WHERE clause using all descendants of the source id
	CString sql, strNumber;
	CArray<int,int> *array;
	//TES 4/8/2004:  Keep a filled-in list of all the referrals handy, to save time.
	if(id == -1 && g_parAllCategoriesPlus && !g_categoryChecker.Changed()) {
		array = g_parAllCategoriesPlus;
	}
	else {
		array = new CArray<int, int>;
		array->Add(id);
		CategoryDescendants(*array);
		if(id == -1 && !g_parAllCategoriesPlus) {
			//Set the global array.
			g_parAllCategoriesPlus = array;
		}
		else {
			bDeleteArray = TRUE;
		}
	}

	if (!array->GetSize())
	{
		sql = field + " IN (-1)";
	}
	else
	{
		sql = field + " IN (";
		for (int i = 0; i < array->GetSize(); i++)
		{	strNumber.Format ("%i,", array->GetAt(i));
			sql += strNumber;
		}
		sql.SetAt(sql.GetLength() - 1, ')');//replace the last , with a )
	}

	if(bDeleteArray)
		delete array;

	return sql;
}


void CategoryDescendants(CArray<int,int> &array)
{
	EnsureCategoryData();

	if (g_rsCategories->eof && g_rsCategories->bof)
	{
		array.RemoveAll();
		return;
	}

	for (int i = 0; i < array.GetSize(); i++)
	{	g_rsCategories->MoveFirst();
		while (!g_rsCategories->eof)
		{	if (g_rsCategories->Fields->GetItem("Parent")->Value.lVal == array.GetAt(i))
				array.Add(g_rsCategories->Fields->GetItem("ID")->Value.lVal);
			g_rsCategories->MoveNext();
		}
	}
}

void Save (CString data)
{
	//DRT 12/15/2003 - PLID 10406 - Using this method is sometimes saving 'temp.txt' log files in patient document paths!
	//	It should just go to the log anyways, if it's something we want to save.
	// (c.haag 2015-07-08) - PLID 65912 - This method does nothing and the logging isn't helpful
	//Log("MarketUtils::Save():  " + data);

//	ofstream outfile("temp.txt");	outfile << data;
//	outfile.flush();

}

// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker
void GetDateText(CDateTimePicker &dt, CString &str)
{
	COleDateTime date;
	date = dt.GetDateTime();
	str = "'" + FormatDateTimeForSql(date, dtoDate) + "'";

}

void GetDateText(COleDateTime &date, CString &str)
{
	str = "'" + FormatDateTimeForSql(date, dtoDate) + "'";
}

void GetDateText(CDateTimeCtrl &dt, CString &str)
{
	COleDateTime date;
	dt.GetTime(date);
	str = "'" + FormatDateTimeForSql(date, dtoDate) + "'";
}

void GetDateTextFrom(CString &str)
{
//	GetDateText(GetMainFrame()->m_pDocToolBar->GetFrom(), str);
}

void GetDateTextTo(CString &str)
{
	//Add a day.
//	COleDateTime dtTo = GetMainFrame()->m_pDocToolBar->GetTo();
//	dtTo += COleDateTimeSpan(1,0,0,0);
//	str = "'" + FormatDateTimeForSql(dtTo,dtoDate) + "'";
}

long ReferralID(const CString &source)
{
	EnsureReferralsPlus();
	try
	{	g_rsReferralsPlus->MoveFirst();
		while (!g_rsReferralsPlus->eof) {
			// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - Use CString's Compare function rather than ambiguous == 
			if (source.Compare(_bstr_t(g_rsReferralsPlus->Fields->Item["Name"]->Value)) == 0)
				return g_rsReferralsPlus->Fields->Item["PersonID"]->Value.lVal;
			else g_rsReferralsPlus->MoveNext();
		}
	}NxCatchAll("could not find referral source id");
	return - 1;
}

_RecordsetPtr GetReferralData()
{
	// b.cardillo 03/16/2001 - I put this try/catch block in because if the connection is 
	// lost (or there are any other exceptions for that matter) the program crashes instead 
	// of giving an error message.
	try {
		EnsureReferralData(false);
	} NxCatchAll("GetReferralData");

	return g_rsReferrals;
}

_RecordsetPtr GetReferralPlusData()
{
	//TES 4/5/2004: Referral Sources plus Referring Physicians and Referring Patients.
	try {
		EnsureReferralsPlus();
	}
	NxCatchAll("GetReferralPlusData");

	return g_rsReferralsPlus;
}


//this function returns the sql queries for the given graph.  I did it this way because we are going to have a combination graph which
//they get to choose from a bunch of graphs they want to see, so, because of this, I needed the sqls in a central location that all tabs could access
//the flags field means different things depending on what graph it is
// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
CString GetGraphSql(MarketGraphType mktGraph, long nFlags, long nFilter, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable) {

	CString strSql;
	
	switch (mktGraph) {

	
	
	
	
	case CNVProsToSurgByProc:
	break;

	//***************************GRAPHS OF THE REFERRAL TAB***************************************************
	
	//Revenue By Referral
	case EFFMonByRefSour:

		//TES 4/5/2004: All referral source-based queries include ref phys and ref pats.

		//PLID 14894: take out the referral source if they have a referring physician and the referral source that practice 
		// defaultly selects with the preference
		// (r.gonet 2015-05-05 09:53) - PLID 66305 - Also exclude 10 - GiftCertificateRefunds

		if (nFlags == 1) {

				strSql.Format("	SELECT [ReplacePref] ReferralID, CAST (Sum(Payments) AS Float) AS Payments, CAST (0.00 AS FLOAT) AS Cost, Name FROM (	"
				"	/*Payments Query*/	"
				"		SELECT 	ReferralSourceT.PersonID AS ReferralID,  Sum(PaymentsByReferralSourceSubQ.Amount) AS Payments,		"
				"		ReferralSourceT.Name AS Name, TransProvID, TransLocID, PatLocID, PatProvID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate	"
				"		FROM ((SELECT * FROM (SELECT LinePaymentsT.ID, Amount = CASE	"
					"		WHEN [_PartiallyAppliedPaysQ].[ID] Is Null	"
					"		THEN CASE	"
				"					WHEN [LineChargesT].[ID] Is Null	"
							"		THEN [LinePaymentsT].[Amount]	"
							"		ELSE [AppliesT].[Amount]	"
							"		End		"
						"		ELSE  [AppliesT].[Amount]	"
						"		End,	"
					"		TransProvID = CASE		"
						"		WHEN [DoctorsProviders] Is Null		"
						"		THEN PaymentsT.ProviderID	"
						"		ELSE [DoctorsProviders]		"
						"		End,	"
					"		LinePaymentsT.InputDate AS SourceIDate,		"
					"		LinePaymentsT.Date AS SourceTDate,		"
					"		PatientsT.ReferralID,	"
					"		TransLocID = CASE	"
						"		WHEN LineChargesT.ID IS NULL	"
						"		THEN LinePaymentsT.LocationID	"
						"		ELSE LineChargesT.LocationID	"
						"		END,	"
					"		LineChargesT.InputDate AS DestIDate,	"
					"		LineChargesT.Date AS DestTDate,	"
					"		BillsT.Date AS BillDate,	"
					"		PatientsT.MainPhysician AS PatProvID,	"
					"		PersonT.Location AS PatLocID	"
					"		FROM (((((((LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT	"
					"		ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN	"
					"		(SELECT LinePaymentsT.ID, LinePaymentsT.Amount AS PayAmt,	"
					"		Sum(AppliesT.Amount) AS ApplyAmt,	"
					"		/*First a Amount*/Min([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,	"
					"		LinePaymentsT.PatientID	"
					"		FROM LineItemT AS LinePaymentsT LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LineChargesT ON AppliesT.DestID = LineChargesT.ID)	"
						"	 ON PaymentsT.ID = AppliesT.SourceID) ON LinePaymentsT.ID = PaymentsT.ID	"
						"   LEFT JOIN PersonT ON LinePaymentsT.PatientID = PersonT.ID "
					"		WHERE (((LinePaymentsT.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) [To] [From]	"
					"		GROUP BY LinePaymentsT.ID, LinePaymentsT.Amount, LinePaymentsT.PatientID	"
					"		HAVING (((LinePaymentsT.ID) is not  Null) AND ((MIN([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))	"
					"		) AS _PartiallyAppliedPaysQ ON LinePaymentsT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID)	"
					"		LEFT JOIN LineItemT AS LineChargesT ON AppliesT.DestID = LineChargesT.ID) LEFT JOIN ChargesT ON LineChargesT.ID = ChargesT.ID)	"
					"		LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID	"
					"       LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
					"		LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LinePaymentsT.PatientID = PatientsT.PersonID)	"
					"		WHERE (((PaymentsT.ID) Is Not Null) AND ((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))	"
					"		[Resp] [PatFilter] [Category] [From] [To] [referrals] [TakeOutForRefPhys] [TakeOutForRefPat] "
					"		) AS _PaymentsByReferralSourceFullQ		"
					"		WHERE (1=1) [Prov] [Loc1] "
					"		UNION ALL	"
					"		SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,	"
					"		[_PartiallyAppliedPaysQ].Total AS Amount,	"
					"		PaymentsT.ProviderID AS TransProvID,	"
					"		LinePaymentsT.InputDate AS IDate,	"
					"		LinePaymentsT.Date AS TDate,	"
					"		PatientsT.ReferralID,	"
					"		LinePaymentsT.LocationID AS TransLocID,	"
					"		LinePaymentsT.InputDate AS OtherIDate,	"
					"		LinePaymentsT.Date AS OtherTDate,	"
					"		LinePaymentsT.Date AS BillDate,	"
					"		PatientsT.MainPhysician as PatProvID,	"
					"		PersonT.Location as PatLocID	"
					"		FROM ((((SELECT LinePaymentsT.ID, LinePaymentsT.Amount AS PayAmt,	"
					"		Sum(AppliesT.Amount) AS ApplyAmt,	"
					"		/*First a Amount*/Min([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,	"
					"		LinePaymentsT.PatientID	"
					"		FROM LineItemT AS LinePaymentsT LEFT JOIN		"
					"		(PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LineChargesT ON AppliesT.DestID = LineChargesT.ID) ON PaymentsT.ID = AppliesT.SourceID)	"
					"		ON LinePaymentsT.ID = PaymentsT.ID	"
					"       LEFT JOIN PersonT ON LinePaymentsT.PatientID = PersonT.ID "
					"		WHERE (((LinePaymentsT.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))[To] [From]	"
					"		GROUP BY LinePaymentsT.ID, LinePaymentsT.Amount, LinePaymentsT.PatientID	"
					"		HAVING (((LinePaymentsT.ID) is not  Null) AND ((MIN([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))	"
					"		) AS _PartiallyAppliedPaysQ		"
					"		INNER JOIN (LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID)	"
					"		ON [_PartiallyAppliedPaysQ].ID = LinePaymentsT.ID)	"
					"		INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)	"
					"		ON LinePaymentsT.PatientID = PatientsT.PersonID)	"
					"		WHERE (((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))		"
					"       [Resp] [PatFilter] [Category2] [From4] [To4] [referrals] [TakeOutForRefPhys] [TakeOutForRefPat]  "
					"		) AS _PaymentsByReferralSourcePartQ		"
					"       WHERE (1=1)  [Prov] [Loc1]  "
					"		) AS PaymentsByReferralSourceSubQ	"
					"		LEFT JOIN ReferralSourceT ON PaymentsByReferralSourceSubQ.ReferralID = ReferralSourceT.PersonID)	"
					"		WHERE ReferralSourceT.PersonID IS NOT NULL	"
					"		GROUP BY ReferralSourceT.PersonID, ReferralSourceT.Name, TransProvID, TransLocID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, PatProvID, PatLocID	"
			"		UNION ALL	"
			"		/*Refund Query*/	"
				"		SELECT 	ReferralSourceT.PersonID AS ReferralID,  Sum(PaymentsByReferralSourceSubQ.Amount) AS Payments,		"
				"		ReferralSourceT.Name AS Name, TransProvID, TransLocID, PatLocID, PatProvID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate "
					"		FROM ((SELECT * FROM (SELECT LinePaymentsT.ID, Amount = CASE	"
						"		WHEN [_PartiallyAppliedRefsQ].[ID] Is Null	"
						"		THEN CASE	"
							"		WHEN [LinePaymentsT].[ID] Is Null	"
							"		THEN [LineRefundsT].[Amount]	"
							"		ELSE [AppliesT].[Amount]	"
							"		End		"
						"		ELSE  [AppliesT].[Amount]	"
						"		End,	"
					"		TransProvID = CASE		"
						"		WHEN [RefundsT].[ProviderID] Is Null	"
						"		THEN [PaymentsT].[ProviderID]	"
						"		ELSE [RefundsT].[ProviderID]	"
						"		End,	"
					"		LineRefundsT.InputDate AS SourceIDate,		"
					"		LineRefundsT.Date AS SourceTDate,		"
					"		PatientsT.ReferralID,	"
					"		TransLocID = CASE	"
						"		WHEN [LinePaymentsT].[ID] Is Null	"
						"		THEN [LineRefundsT].[LocationID]	"
						"		ELSE [LinePaymentsT].[LocationID]	"
						"		End,	"
					"		LinePaymentsT.InputDate AS DestIDate,	"
					"		LinePaymentsT.Date AS DestTDate,	"
					"		LineRefundsT.Date AS BillDate,	"
					"		PatientsT.MainPhysician AS PatProvID,	"
					"		PersonT.Location AS PatLocID	"
					"		FROM (((((((LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT RefundsT	"
					"		ON LineRefundsT.ID = RefundsT.ID) LEFT JOIN	"
					"		(SELECT LineRefundsT.ID, LineRefundsT.Amount AS RefAmt,		"
					"		Sum(AppliesT.Amount) AS ApplyAmt,	"
					"		/*First a Amount*/Min([LineRefundsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,	"
					"		LineRefundsT.PatientID		"
					"		FROM LineItemT AS LineRefundsT LEFT JOIN (PaymentsT RefundsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID)	"
						"	 ON RefundsT.ID = AppliesT.SourceID) ON LineRefundsT.ID = RefundsT.ID	"
						"   LEFT JOIN PersonT ON LineRefundsT.PatientID = PersonT.ID "
					"		WHERE (((LineRefundsT.Deleted)=0)) AND (RefundsT.PayMethod NOT IN (4,10)) AND LineRefundsT.Type = 3 [To2] [From2]	"
					"		GROUP BY LineRefundsT.ID, LineRefundsT.Amount, LineRefundsT.PatientID	"
					"		HAVING (((LineRefundsT.ID) is not  Null) AND ((MIN([LineRefundsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))	"
					"		) AS _PartiallyAppliedRefsQ ON LineRefundsT.ID = [_PartiallyAppliedRefsQ].ID) LEFT JOIN AppliesT ON RefundsT.ID = AppliesT.SourceID)	"
					"		LEFT JOIN LineItemT AS LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID) LEFT JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID)		"
					"		LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineRefundsT.PatientID = PatientsT.PersonID)	"
					"		WHERE (((RefundsT.ID) Is Not Null) AND ((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (RefundsT.PayMethod NOT IN (4,10))		"
					"       [Resp2] [PatFilter] [Category2] [From2] [To2] [referrals] [From3] [To3] [TakeOutForRefPhys] [TakeOutForRefPat] "
					"		) AS _PaymentsByReferralSourceFullQ		"
					"       WHERE (1=1)  [Prov] [Loc1]  "
					"		UNION ALL	"
					"		SELECT * FROM (SELECT [_PartiallyAppliedRefsQ].ID,	"
					"		[_PartiallyAppliedRefsQ].Total AS Amount,	"
					"		RefundsT.ProviderID AS TransProvID,		"
					"		LineRefundsT.InputDate AS IDate,	"
					"		LineRefundsT.Date AS TDate,	"
					"		PatientsT.ReferralID,	"
					"		LineRefundsT.LocationID as TransLocID,	"
					"		LineRefundsT.InputDate as OtherIDate,	"
					"		LineRefundsT.Date as OtherTDate,	"
					"		LineRefundsT.Date as BillDate,	"
					"		PatientsT.MainPhysician as PatProvID,	"
					"		PersonT.Location as PatLocID	"
					"		FROM ((((SELECT LineRefundsT.ID, LineRefundsT.Amount AS PayAmt,		"
					"		Sum(AppliesT.Amount) AS ApplyAmt,	"
					"		/*First a Amount*/Min([LineRefundsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,	"
					"		LineRefundsT.PatientID		"
					"		FROM LineItemT AS LineRefundsT LEFT JOIN	"
					"		(PaymentsT RefundsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID) ON RefundsT.ID = AppliesT.SourceID)	"
					"		ON LineRefundsT.ID = RefundsT.ID	"
					"		LEFT JOIN PersonT ON LineRefundsT.PatientID = PersonT.ID "
					"		WHERE (((LineRefundsT.Deleted)=0)) AND (RefundsT.PayMethod NOT IN (4,10)) AND (LineRefundsT.Type = 3) [To2] [From2]	"
					"		GROUP BY LineRefundsT.ID, LineRefundsT.Amount, LineRefundsT.PatientID	"
					"		HAVING (((LineRefundsT.ID) is not  Null) AND ((MIN([LineRefundsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))	"
					"		) AS _PartiallyAppliedRefsQ		"
					"		INNER JOIN (LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID)	"
					"		ON [_PartiallyAppliedRefsQ].ID = LineRefundsT.ID)	"
					"		INNER JOIN PaymentsT RefundsT ON LineRefundsT.ID = RefundsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)	"
					"		ON LineRefundsT.PatientID = PatientsT.PersonID)	"
					"		WHERE (((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (RefundsT.PayMethod NOT IN (4,10))	"
					"       [Resp2] [PatFilter] [Category2] [From2] [To2] [referrals] [From3] [To3] [TakeOutForRefPhys] [TakeOutForRefPat] "
					"		) AS _PaymentsByReferralSourcePartQ		"
					"       WHERE (1=1)  [Prov] [Loc1]   "
					"		) AS PaymentsByReferralSourceSubQ	"
					"		LEFT JOIN ReferralSourceT ON PaymentsByReferralSourceSubQ.ReferralID = ReferralSourceT.PersonID)	"
					"		WHERE ReferralSourceT.PersonID IS NOT NULL	"
					"		GROUP BY ReferralSourceT.PersonID, ReferralSourceT.Name, TransProvID, TransLocID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, PAtProvID, PatLocID	"
			"		) RevenueQ	"
			"		GROUP BY RevenueQ.ReferralID, RevenueQ.Name	");
		/*	
		//the flag here is whether to use referral sources, referring physicians, or referring patients, the filter is the category id
			strSql.Format("SELECT ReferralSourceT.PersonID AS ReferralID, "
			"CAST ((CASE WHEN PayQ.Payments IS NULL THEN 0 ELSE PayQ.Payments END) AS FLOAT) AS Payments, "
			"0 as Cost, ReferralSourceT.Name as Name "
			"FROM ReferralSourceT LEFT JOIN "
			"(	SELECT Sum(CASE WHEN (ChargesT.ID IS NULL OR %li = -1) THEN LineItemT.Amount ELSE AppliesT.Amount END) AS Payments, PatientsT.ReferralID AS ID "
				"FROM LineItemT "
				"INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
				"LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
				"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		//		"LEFT JOIN AppliesT AS AppliesT1 ON LineItemT.ID = AppliesT1.SourceID "
		//		"LEFT JOIN AppliesT AS AppliesT2 ON LineItemT.ID = AppliesT2.DestID "
				"WHERE LineItemT.Deleted = 0 AND (LineItemT.Type = 1 OR LineItemT.Type = 3) "		//DRT 6/5/02 - Added refunds so this graph matches the Effectiveness #'s
		//		"AND (PaymentsT.PrePayment = 0 OR AppliesT1.ID IS NULL AND AppliesT2.ID IS NULL) "
				" [OptionFrom] [OptionTo] "
				" [referrals] "
				" [doc3] "
				" [loc] "
				" [category] "
				" [PatFilter] "
				" [Resp] "
				"GROUP BY PatientsT.ReferralID "
			") AS PayQ ON ReferralSourceT.PersonID = PayQ.ID ", nFilter);

			*/
			//costs
			strSql +=
			" UNION ALL "

			"SELECT ReferralSource AS ReferralID, 0 AS Payments, "
			"CAST (SUM (CostSubQ.Cost) AS FLOAT) AS Cost, Name "
			"FROM "
			
			// (b.cardillo 2004-08-02 16:53) - 13747 - Optimized this section of the query by combining four 
			// statements (each of which was returning a single cost value) into one statement that returns 
			// all four cost values and then adds them back together to get the original sum.  This is a good 
			// deal faster because it saves SQL from having to filter on the [referral2] clause 4 times.
			"("
			"SELECT Cost1+Cost2+Cost3+Cost4 AS Cost, ReferralSource, Name "
			"FROM ( "
			"  SELECT "
			"    (CASE WHEN (EffectiveFrom >= [from] AND EffectiveFrom < [to] AND EffectiveTo >= [from] AND EffectiveTo < [to]) "
			"	  THEN (Amount) ELSE 0 END) AS Cost1, "
			"    (CASE WHEN (EffectiveFrom < [to] AND EffectiveFrom > [from] AND EffectiveTo >= [to]) "
			"	  THEN (Amount * DATEDIFF(day, [to], EffectiveFrom)) / DATEDIFF(day, EffectiveTo, EffectiveFrom) ELSE 0 END) AS Cost2, "
			"    (CASE WHEN (EffectiveFrom <= [from] AND EffectiveTo >= [to] AND (EffectiveFrom <> [from] OR EffectiveTo <> [to])) "
			"	  THEN (Amount * DATEDIFF(day, [to], [from])) / DATEDIFF(day, EffectiveTo, EffectiveFrom) ELSE 0 END) AS Cost3, "
			"    (CASE WHEN (EffectiveFrom < [from] AND EffectiveTo > [from] AND EffectiveTo < [to]) "
			"	  THEN (Amount * DATEDIFF(day, EffectiveTo, [from])) / DATEDIFF (day, EffectiveTo, EffectiveFrom) ELSE 0 END) AS Cost4, "
			"    ReferralSource, Name "
			"  FROM MarketingCostsT "
			"  LEFT JOIN ReferralSourceT ON MarketingCostsT.ReferralSource = ReferralSourceT.PersonID "
			"  WHERE (1=1) [referral2] [Loc2] "
			") RangedCostsQ "

			") AS CostSubQ "
			"GROUP BY ReferralSource, Name";

		}else if (nFlags == 2) {

				//Referring Physicians
				// (j.dinatale 2012-12-18 14:50) - PLID 54186 - need to add a new filter here, to determine if the primary referral source is the referring phys
				// (r.gonet 2015-05-05 09:53) - PLID 66305 - Also exclude 10 - GiftCertificateRefunds
				strSql.Format(" SELECT RefPhysID as ReferralID, CAST (Sum(Payments) AS FLOAT) AS Payments, CAST (0.00 AS FLOAT) AS Cost, Name FROM (	 "
				" /*Payments Query*/	 "
				" 		SELECT 	ReferringPhysT.PersonID AS RefPhysID,  Sum(PaymentsByReferralSourceSubQ.Amount) AS Payments,		 "
					" PersonReferringPhysT.Last + ', ' + PersonReferringPhysT.First  + ' ' + PersonReferringPhysT.Middle AS Name, TransProvID, TransLocID, PatLocID, PatProvID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate	   "
					" FROM ((SELECT * FROM (SELECT LinePaymentsT.ID, Amount = CASE	   "
						" WHEN [_PartiallyAppliedPaysQ].[ID] Is Null	   "
						" THEN CASE	   "
								" WHEN [LineChargesT].[ID] Is Null	   "
								" THEN [LinePaymentsT].[Amount]	   "
								" ELSE [AppliesT].[Amount]	   "
								" End		   "
							" ELSE  [AppliesT].[Amount]	   "
							" End,	 ""  "
						" TransProvID = CASE		   "
							" WHEN [DoctorsProviders] Is Null		   "
							" THEN PaymentsT.ProviderID	 ""  "
							" ELSE [DoctorsProviders]		   "
							" End,	   "
						" LinePaymentsT.InputDate AS SourceIDate,		   "
						" LinePaymentsT.Date AS SourceTDate,		 "
						" PatientsT.DefaultReferringPhyID,	 "
						"  	TransLocID = CASE	 "
						" 		WHEN LineChargesT.ID IS NULL	 "
						" 		THEN LinePaymentsT.LocationID	 "
						" 		ELSE LineChargesT.LocationID	 "
						" 		END,	 "
					" 		LineChargesT.InputDate AS DestIDate,	 "
					" 		LineChargesT.Date AS DestTDate,	 "
					" 		BillsT.Date AS BillDate,	 "
					" 		PatientsT.MainPhysician AS PatProvID,	 "
					" 		PersonT.Location AS PatLocID	 "
					" 		FROM (((((((LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT	 "
					" 		ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN	 "
					" 		(SELECT LinePaymentsT.ID, LinePaymentsT.Amount AS PayAmt,	 "
					" 		Sum(AppliesT.Amount) AS ApplyAmt,	 "
					" 		/*First a Amount*/Min([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,	 "
					" 		LinePaymentsT.PatientID	 "
					" 		FROM LineItemT AS LinePaymentsT LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LineChargesT ON AppliesT.DestID = LineChargesT.ID)	 "
						" 	 ON PaymentsT.ID = AppliesT.SourceID) ON LinePaymentsT.ID = PaymentsT.ID	 "
						"   LEFT JOIN PersonT ON LinePaymentsT.PatientID = PersonT.ID "
					" 		WHERE (((LinePaymentsT.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))	[To] [From] "
					" 		GROUP BY LinePaymentsT.ID, LinePaymentsT.Amount, LinePaymentsT.PatientID	 "
					" 		HAVING (((LinePaymentsT.ID) is not  Null) AND ((MIN([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))	 "
					" 		) AS _PartiallyAppliedPaysQ ON LinePaymentsT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID)	 "
					" 		LEFT JOIN LineItemT AS LineChargesT ON AppliesT.DestID = LineChargesT.ID) LEFT JOIN ChargesT ON LineChargesT.ID = ChargesT.ID)	 "
					" 		LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID	 "
					"       LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
					" 		LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LinePaymentsT.PatientID = PatientsT.PersonID)	 "
					" 		WHERE (((PaymentsT.ID) Is Not Null) AND ((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))	 "
					"		[Resp] [PatFilter] [Category] [From] [To] [RefPhys] [RefPhysPrimSource] "														 
					" 		) AS _PaymentsByReferralSourceFullQ		 "
				    "       WHERE (1=1) [Prov] [Loc1]   "
					" 		UNION ALL	 "
					" 		SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,	 "
					" 		[_PartiallyAppliedPaysQ].Total AS Amount,	 "
					" 		PaymentsT.ProviderID AS TransProvID,	 "
					" 		LinePaymentsT.InputDate AS IDate,	 "
					" 		LinePaymentsT.Date AS TDate,	 "
					" 		PatientsT.DefaultReferringPhyID, "
					" 		LinePaymentsT.LocationID as TransLocID,	 "
					" 		LinePaymentsT.InputDate AS OtherIDate,	 "
					" 		LinePaymentsT.Date AS OtherTDate,	 "
					" 		LinePaymentsT.Date AS BillDate,	 "
					" 		PatientsT.MainPhysician as PatProvID,	 "
					" 		PersonT.Location as PatLocID	 "
					" 		FROM ((((SELECT LinePaymentsT.ID, LinePaymentsT.Amount AS PayAmt,	 "
					" 		Sum(AppliesT.Amount) AS ApplyAmt,	 "
					" 		/*First a Amount*/Min([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,	 "
					" 		LinePaymentsT.PatientID	 "
					" 		FROM LineItemT AS LinePaymentsT LEFT JOIN		 "
					" 		(PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LineChargesT ON AppliesT.DestID = LineChargesT.ID) ON PaymentsT.ID = AppliesT.SourceID)	 "
					" 		ON LinePaymentsT.ID = PaymentsT.ID	 "
					"       LEFT JOIN PersonT ON LinePaymentsT.PatientID = PersonT.ID  " 
					" 		WHERE (((LinePaymentsT.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))	[To] [From] "
					" 		GROUP BY LinePaymentsT.ID, LinePaymentsT.Amount, LinePaymentsT.PatientID	 "
					" 		HAVING (((LinePaymentsT.ID) is not  Null) AND ((MIN([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))	 "
					" 		) AS _PartiallyAppliedPaysQ		 "
					" 		INNER JOIN (LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID)	 "
					" 		ON [_PartiallyAppliedPaysQ].ID = LinePaymentsT.ID)	 "
					" 		INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)	 "
					" 		ON LinePaymentsT.PatientID = PatientsT.PersonID)	 "
					" 		WHERE (((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))		 "
					"       [Resp] [PatFilter] [Category2] [From4] [To4]  [RefPhys] [RefPhysPrimSource] "
					" 		) AS _PaymentsByReferralSourcePartQ		 "
					"       WHERE (1=1)  [Prov] [Loc1]  "
					" 		) AS PaymentsByReferralSourceSubQ	 "
					" 		LEFT JOIN ReferringPhysT ON PaymentsByReferralSourceSubQ.DefaultReferringPhyID = ReferringPhysT.PersonID "
					" 		LEFT JOIN PersonT PersonReferringPhysT ON ReferringPhysT.PersonID = PersonReferringPhysT.ID)	 "
					" 		WHERE ReferringPhysT.PersonID IS NOT NULL	 "
					" 		GROUP BY ReferringPhysT.PersonID, PersonReferringPhysT.First, PersonReferringPhysT.Last, PersonReferringPhysT.Middle, TransProvID, TransLocID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, PatProvID, PatLocID	 "
			" 		UNION ALL	 "
			" 		/*Refund Query*/	 "
				" 		SELECT 	ReferringPhysT.PersonID AS RefPhysID,  Sum(PaymentsByReferralSourceSubQ.Amount) AS Payments,		 "
				" 		PersonReferringPhysT.Last + ', ' + PersonReferringPhysT.First  + ' ' + PersonReferringPhysT.Middle AS Name, TransProvID, TransLocID, PatLocID, PatProvID, DestIDate,  DestTDate, SourceIDate, SourceTDate, BillDate "
					" 		FROM ((SELECT * FROM (SELECT LinePaymentsT.ID, Amount = CASE	 "
						" 		WHEN [_PartiallyAppliedRefsQ].[ID] Is Null	 "
						" 		THEN CASE	 "
							" 		WHEN [LinePaymentsT].[ID] Is Null	 "
							" 		THEN [LineRefundsT].[Amount]	 "
							" 		ELSE [AppliesT].[Amount]	 "
							" 		End		 "
						" 		ELSE  [AppliesT].[Amount]	 "
						" 		End,	 "
					" 		TransProvID = CASE		 "
						" 		WHEN [RefundsT].[ProviderID] Is Null	 "
						" 		THEN [PaymentsT].[ProviderID]	 "
						" 		ELSE [RefundsT].[ProviderID]	 "
						" 		End,	 "
					" 		LineRefundsT.InputDate AS SourceIDate,		 "
					" 		LineRefundsT.Date AS SourceTDate,		 "
					" 		PatientsT.DefaultReferringPhyID,	 "
					" 		TransLocID = CASE	 "
						" 		WHEN [LinePaymentsT].[LocationID] Is Null	 "
						" 		THEN [LineRefundsT].[LocationID]	 "
						" 		ELSE [LinePaymentsT].[LocationID]	 "
						" 		End,	 "
					" 		LinePaymentsT.InputDate AS DestIDate,	 "
					" 		LinePaymentsT.Date AS DestTDate,	 "
					" 		LineRefundsT.Date AS BillDate,	 "
					" 		PatientsT.MainPhysician AS PatProvID,	 "
					" 		PersonT.Location AS PatLocID	 "
					" 		FROM (((((((LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT RefundsT	 "
					" 		ON LineRefundsT.ID = RefundsT.ID) LEFT JOIN	 "
					" 		(SELECT LineRefundsT.ID, LineRefundsT.Amount AS RefAmt,		 "
					" 		Sum(AppliesT.Amount) AS ApplyAmt,	 "
					" 		/*First a Amount*/Min([LineRefundsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,	 "
					" 		LineRefundsT.PatientID		 "
					" 		FROM LineItemT AS LineRefundsT LEFT JOIN (PaymentsT RefundsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID)	 "
						" 	 ON RefundsT.ID = AppliesT.SourceID) ON LineRefundsT.ID = RefundsT.ID	 "
						"   LEFT JOIN PersonT ON LineRefundsT.PatientID = PersonT.ID "
					" 		WHERE (((LineRefundsT.Deleted)=0)) AND (RefundsT.PayMethod NOT IN (4,10)) AND LineRefundsT.Type = 3	[To2] [From2] "
					" 		GROUP BY LineRefundsT.ID, LineRefundsT.Amount, LineRefundsT.PatientID	 "
					" 		HAVING (((LineRefundsT.ID) is not  Null) AND ((MIN([LineRefundsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))	 "
					" 		) AS _PartiallyAppliedRefsQ ON LineRefundsT.ID = [_PartiallyAppliedRefsQ].ID) LEFT JOIN AppliesT ON RefundsT.ID = AppliesT.SourceID)	 "
					" 		LEFT JOIN LineItemT AS LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID) LEFT JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID)		 "
					" 		LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineRefundsT.PatientID = PatientsT.PersonID)	 "
					" 		WHERE (((RefundsT.ID) Is Not Null) AND ((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (RefundsT.PayMethod NOT IN (4,10))		 "
					"       [Resp2] [PatFilter] [Category2] [From2] [To2] [RefPhys] [From3] [To3] [RefPhysPrimSource] "
					" 		) AS _PaymentsByReferralSourceFullQ		 "
					"       WHERE (1=1)  [Prov] [Loc1]  "
					" 		UNION ALL	 "
					" 		SELECT * FROM (SELECT [_PartiallyAppliedRefsQ].ID,	 "
					" 		[_PartiallyAppliedRefsQ].Total AS Amount,	 "
					" 		RefundsT.ProviderID AS TransProvID,		 "
					" 		LineRefundsT.InputDate AS IDate,	 "
					" 		LineRefundsT.Date AS TDate,	 "
					" 		PatientsT.DefaultReferringPhyID,	 "
					" 		LineRefundsT.LocationID as TransLocID,	 "
					" 		LineRefundsT.InputDate as OtherIDate,	 "
					" 		LineRefundsT.Date as OtherTDate,	 "
					" 		LineRefundsT.Date as BillDate,	 "
					" 		PatientsT.MainPhysician as PatProvID,	 "
					" 		PersonT.Location as PatLocID	 "
					" 		FROM ((((SELECT LineRefundsT.ID, LineRefundsT.Amount AS PayAmt,		 "
					" 		Sum(AppliesT.Amount) AS ApplyAmt,	 "
					" 		/*First a Amount*/Min([LineRefundsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,	 "
					" 		LineRefundsT.PatientID		 "
					" 		FROM LineItemT AS LineRefundsT LEFT JOIN	 "
					" 		(PaymentsT RefundsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID) ON RefundsT.ID = AppliesT.SourceID)	 "
					" 		ON LineRefundsT.ID = RefundsT.ID	 "
					"       LEFT JOIN PersonT ON LineRefundsT.PatientID = PersonT.ID "
					" 		WHERE (((LineRefundsT.Deleted)=0)) AND (RefundsT.PayMethod NOT IN (4,10)) AND (LineRefundsT.Type = 3) [To2] [From2]	 "
					" 		GROUP BY LineRefundsT.ID, LineRefundsT.Amount, LineRefundsT.PatientID	 "
					" 		HAVING (((LineRefundsT.ID) is not  Null) AND ((MIN([LineRefundsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))	 "
					" 		) AS _PartiallyAppliedRefsQ		 "
					" 		INNER JOIN (LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID)	 "
					" 		ON [_PartiallyAppliedRefsQ].ID = LineRefundsT.ID)	 "
					" 		INNER JOIN PaymentsT RefundsT ON LineRefundsT.ID = RefundsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)	 "
					" 		ON LineRefundsT.PatientID = PatientsT.PersonID)	 "
					" 		WHERE (((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (RefundsT.PayMethod NOT IN (4,10))	 "
					"       [Resp2] [PatFilter] [Category2] [From2] [To2] [RefPhys] [From3] [To3] [RefPhysPrimSource] "
					" 		) AS _PaymentsByReferralSourcePartQ		 "
					"       WHERE (1=1)  [Prov] [Loc1]   "
					" 		) AS PaymentsByReferralSourceSubQ	 "
					" 		LEFT JOIN ReferringPhysT ON PaymentsByReferralSourceSubQ.DefaultReferringPhyID = ReferringPhysT.PersonID "
					"    	LEFT JOIN PersonT PersonReferringPhysT ON ReferringPhysT.PersonID = PersonReferringPhysT.ID)	 "
					" 		WHERE ReferringPhysT.PersonID IS NOT NULL	 "
					" 		GROUP BY ReferringPhysT.PersonID, PersonREferringPhysT.First, PersonReferringPhysT.Middle, PersonReferringPhysT.Last, TransProvID, TransLocID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, PAtProvID, PatLocID	 "
			" 		) RevenueQ	 "
			" 		GROUP BY RevenueQ.RefPhysID, RevenueQ.Name ");
			
	 	}else if (nFlags == 3) { 

			//Referring Patients
			// (j.dinatale 2012-12-18 14:50) - PLID 54186 - need to add a new filter here, to determine if the primary referral source is the referring patient
			// (r.gonet 2015-05-05 09:53) - PLID 66305 - Also exclude 10 - GiftCertificateRefunds
			strSql.Format(" 	SELECT RefPatID as ReferralID, CAST (Sum(Payments) AS FLOAT) AS Payments, CAST (0.00 AS FLOAT) AS Cost, Name FROM (	"
					" /*Payments Query*/	 "
				" 		SELECT 	PersonReferringPatT.ID AS RefPatID,  Sum(PaymentsByReferralSourceSubQ.Amount) AS Payments,		 "
				" 		PersonReferringPatT.Last + ', ' + PersonReferringPatT.First + ' ' + PersonReferringPatT.Middle AS Name, "
				"      TransProvID, TransLocID, PatLocID, PatProvID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate	 "
				" 		FROM ((SELECT * FROM (SELECT LinePaymentsT.ID, Amount = CASE	 "
					" 		WHEN [_PartiallyAppliedPaysQ].[ID] Is Null	 "
					" 		THEN CASE	 "
							" 		WHEN [LineChargesT].[ID] Is Null	 "
							" 		THEN [LinePaymentsT].[Amount]	 "
							" 		ELSE [AppliesT].[Amount]	 "
							" 		End		 "
						" 		ELSE  [AppliesT].[Amount]	 "
						" 		End,	 "
					" 		TransProvID = CASE		 "
						" 		WHEN [DoctorsProviders] Is Null		 "
						" 		THEN PaymentsT.ProviderID	 "
						" 		ELSE [DoctorsProviders]		 "
						" 		End,	 "
					" 		LinePaymentsT.InputDate AS SourceIDate,		 "
					" 		LinePaymentsT.Date AS SourceTDate,		 "
					" 		PatientsT.ReferringPatientID,	 "
					" 		TransLocID = CASE	 "
						" 		WHEN LineChargesT.ID IS NULL	 "
						" 		THEN LinePaymentsT.LocationID	 "
						" 		ELSE LineChargesT.LocationID	 "
						" 		END,	 "
					" 		LineChargesT.InputDate AS DestIDate,	 "
					" 		LineChargesT.Date AS DestTDate,	 "
					" 		BillsT.Date AS BillDate,	 "
					" 		PatientsT.MainPhysician AS PatProvID,	 "
					" 		PersonT.Location AS PatLocID	 "
					" 		FROM (((((((LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT	 "
					" 		ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN	 "
					" 		(SELECT LinePaymentsT.ID, LinePaymentsT.Amount AS PayAmt,	 "
					" 		Sum(AppliesT.Amount) AS ApplyAmt,	 "
					" 		/*First a Amount*/Min([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,	 "
					" 		LinePaymentsT.PatientID	 "
					" 		FROM LineItemT AS LinePaymentsT LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LineChargesT ON AppliesT.DestID = LineChargesT.ID)	 "
						" 	 ON PaymentsT.ID = AppliesT.SourceID) ON LinePaymentsT.ID = PaymentsT.ID	 "
						"   LEFT JOIN PersonT ON LinePaymentsT.PatientID = PersonT.ID "
					" 		WHERE (((LinePaymentsT.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))	[To] [From] "
					" 		GROUP BY LinePaymentsT.ID, LinePaymentsT.Amount, LinePaymentsT.PatientID	 "
					" 		HAVING (((LinePaymentsT.ID) is not  Null) AND ((MIN([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))	 "
					" 		) AS _PartiallyAppliedPaysQ ON LinePaymentsT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID)	 "
					" 		LEFT JOIN LineItemT AS LineChargesT ON AppliesT.DestID = LineChargesT.ID) LEFT JOIN ChargesT ON LineChargesT.ID = ChargesT.ID)	 "
					" 		LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID	 "
					"       LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
					" 		LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LinePaymentsT.PatientID = PatientsT.PersonID)	 "
					" 		WHERE (((PaymentsT.ID) Is Not Null) AND ((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))	 "
					"       [Resp] [PatFilter] [Category] [From] [To] [RefPat] [PatientPrimSource] "
					" 		) AS _PaymentsByReferralSourceFullQ		 "
					"       WHERE (1=1) [Prov] [Loc1]   "
					" 		UNION ALL	 "
					" 		SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,	 "
					" 		[_PartiallyAppliedPaysQ].Total AS Amount,	 "
					" 		PaymentsT.ProviderID AS TransProvID,	 "
					" 		LinePaymentsT.InputDate AS IDate,	 "
					" 		LinePaymentsT.Date AS TDate,	 "
					" 		PatientsT.ReferringPatientID, "
					" 		LinePaymentsT.LocationID as TransLocID,	 "
					" 		LinePaymentsT.InputDate AS OtherIDate,	 "
					" 		LinePaymentsT.Date AS OtherTDate,	 "
					" 		LinePaymentsT.Date AS BillDate,	 "
					" 		PatientsT.MainPhysician as PatProvID,	 "
					" 		PersonT.Location as PatLocID	 "
					" 		FROM ((((SELECT LinePaymentsT.ID, LinePaymentsT.Amount AS PayAmt,	 "
					" 		Sum(AppliesT.Amount) AS ApplyAmt,	 "
					" 		/*First a Amount*/Min([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,	 "
					" 		LinePaymentsT.PatientID	 "
					" 		FROM LineItemT AS LinePaymentsT LEFT JOIN		 "
					" 		(PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LineChargesT ON AppliesT.DestID = LineChargesT.ID) ON PaymentsT.ID = AppliesT.SourceID)	 "
					" 		ON LinePaymentsT.ID = PaymentsT.ID	 "
					"		LEFT JOIN PersonT ON LinePaymentsT.PatientID = PersonT.ID "
					" 		WHERE (((LinePaymentsT.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) [To] [From]	 "
					" 		GROUP BY LinePaymentsT.ID, LinePaymentsT.Amount, LinePaymentsT.PatientID	 "
					" 		HAVING (((LinePaymentsT.ID) is not  Null) AND ((MIN([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))	 "
					" 		) AS _PartiallyAppliedPaysQ		 "
					" 		INNER JOIN (LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID)	 "
					" 		ON [_PartiallyAppliedPaysQ].ID = LinePaymentsT.ID)	 "
					" 		INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)	 "
					" 		ON LinePaymentsT.PatientID = PatientsT.PersonID)	 "
					" 		WHERE (((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))		 "
					"       [Resp] [PatFilter] [Category2] [From4] [To4] [RefPat] [PatientPrimSource] "
					" 		) AS _PaymentsByReferralSourcePartQ		 "
					"       WHERE (1=1)  [Prov] [Loc1]  "

					" 		) AS PaymentsByReferralSourceSubQ	 "
					" 		LEFT JOIN PersonT PersonReferringPatT ON PaymentsByReferralSourceSubQ.ReferringPatientID = PersonReferringPatT.ID)	 "
					" 		WHERE PersonReferringPatT.ID IS NOT NULL	 "
					" 		GROUP BY PersonReferringPatT.ID, PersonReferringPatT.First, PersonReferringPatT.Last, PersonReferringPatT.Middle, TransProvID, TransLocID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, PatProvID, PatLocID	 "
			" 		UNION All "
			" 		/*Refund Query*/	 "
				" 		SELECT 	PersonReferringPatT.ID AS RefPatID,  Sum(PaymentsByReferralSourceSubQ.Amount) AS Payments,		 "
				" 		PersonReferringPatT.Last + ', ' + PersonReferringPatT.First + ' ' + PersonReferringPatT.Middle AS Name, "
				" 		TransProvID, TransLocID, PatLocID, PatProvID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate "
					" 		FROM ((SELECT * FROM (SELECT LinePaymentsT.ID, Amount = CASE	 "
						" 		WHEN [_PartiallyAppliedRefsQ].[ID] Is Null	 "
						" 		THEN CASE	 "
							" 		WHEN [LinePaymentsT].[ID] Is Null	 "
							" 		THEN [LineRefundsT].[Amount]	 "
							" 		ELSE [AppliesT].[Amount]	 "
							" 		End		 "
						" 		ELSE  [AppliesT].[Amount]	 "
						" 		End,	 "
					" 		TransProvID = CASE		 "
						" 		WHEN [RefundsT].[ProviderID] Is Null	 "
						" 		THEN [PaymentsT].[ProviderID]	 "
						" 		ELSE [RefundsT].[ProviderID]	 "
						" 		End,	 "
					" 		LineRefundsT.InputDate AS SourceIDate,		 "
					" 		LineRefundsT.Date AS SourceTDate,		 "
					" 		PatientsT.ReferringPatientID,	 "
					" 		TransLocID = CASE	 "
						" 		WHEN [LinePaymentsT].[LocationID] Is Null	 "
						" 		THEN [LineRefundsT].[LocationID]	 "
						" 		ELSE [LinePaymentsT].[LocationID]	 "
						" 		End,	 "
					" 		LinePaymentsT.InputDate AS DestIDate,	 "
					" 		LinePaymentsT.Date AS DestTDate,	 "
					" 		LineRefundsT.Date AS BillDate,	 "
					" 		PatientsT.MainPhysician AS PatProvID,	 "
					" 		PersonT.Location AS PatLocID	 "
					" 		FROM (((((((LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT RefundsT	 "
					" 		ON LineRefundsT.ID = RefundsT.ID) LEFT JOIN	 "
					" 		(SELECT LineRefundsT.ID, LineRefundsT.Amount AS RefAmt,		 "
					" 		Sum(AppliesT.Amount) AS ApplyAmt,	 "
					" 		/*First a Amount*/Min([LineRefundsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,	 "
					" 		LineRefundsT.PatientID		 "
					" 		FROM LineItemT AS LineRefundsT LEFT JOIN (PaymentsT RefundsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID)	 "
						" 	 ON RefundsT.ID = AppliesT.SourceID) ON LineRefundsT.ID = RefundsT.ID	 "
						"   LEFT JOIN PersonT ON LineRefundsT.PatientID = PersonT.ID "
					" 		WHERE (((LineRefundsT.Deleted)=0)) AND (RefundsT.PayMethod NOT IN (4,10)) AND LineRefundsT.Type = 3	[To2] [From2] "
					" 		GROUP BY LineRefundsT.ID, LineRefundsT.Amount, LineRefundsT.PatientID	 "
					" 		HAVING (((LineRefundsT.ID) is not  Null) AND ((MIN([LineRefundsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))	 "
					" 		) AS _PartiallyAppliedRefsQ ON LineRefundsT.ID = [_PartiallyAppliedRefsQ].ID) LEFT JOIN AppliesT ON RefundsT.ID = AppliesT.SourceID)	 "
					" 		LEFT JOIN LineItemT AS LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID) LEFT JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID)		 "
					" 		LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineRefundsT.PatientID = PatientsT.PersonID)	 "
					" 		WHERE (((RefundsT.ID) Is Not Null) AND ((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (RefundsT.PayMethod NOT IN (4,10))		 "
					"       [Resp2] [PatFilter] [Category2] [From2] [To2] [RefPat] [From3] [To3] [PatientPrimSource] "
					" 		) AS _PaymentsByReferralSourceFullQ		 "
					"       WHERE (1=1)  [Prov] [Loc1]   "
					" 		UNION ALL	 "
					" 		SELECT * FROM (SELECT [_PartiallyAppliedRefsQ].ID,	 "
					" 		[_PartiallyAppliedRefsQ].Total AS Amount,	 "
					" 		RefundsT.ProviderID AS TransProvID,		 "
					" 		LineRefundsT.InputDate AS IDate,	 "
					" 		LineRefundsT.Date AS TDate,	 "
					" 		PatientsT.ReferringPatientID,	 "
					" 		LineRefundsT.LocationID as TransLocID,	 "
					" 		LineRefundsT.InputDate as OtherIDate,	 "
					" 		LineRefundsT.Date as OtherTDate,	 "
					" 		LineRefundsT.Date as BillDate,	 "
					" 		PatientsT.MainPhysician as PatProvID,	 "
					" 		PersonT.Location as PatLocID	 "
					" 		FROM ((((SELECT LineRefundsT.ID, LineRefundsT.Amount AS PayAmt,		 "
					" 		Sum(AppliesT.Amount) AS ApplyAmt,	 "
					" 		/*First a Amount*/Min([LineRefundsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,	 "
					" 		LineRefundsT.PatientID		 "
					" 		FROM LineItemT AS LineRefundsT LEFT JOIN	 "
					" 		(PaymentsT RefundsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID) ON RefundsT.ID = AppliesT.SourceID)	 "
					" 		ON LineRefundsT.ID = RefundsT.ID	 "
					"       LEFT JOIN PersonT ON LineRefundsT.PatientID = PersonT.ID "
					" 		WHERE (((LineRefundsT.Deleted)=0)) AND (RefundsT.PayMethod NOT IN (4,10)) AND (LineRefundsT.Type = 3) [To2] [From2]	 "
					" 		GROUP BY LineRefundsT.ID, LineRefundsT.Amount, LineRefundsT.PatientID	 "
					" 		HAVING (((LineRefundsT.ID) is not  Null) AND ((MIN([LineRefundsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))	 "
					" 		) AS _PartiallyAppliedRefsQ		 "
					" 		INNER JOIN (LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID)	 "
					" 		ON [_PartiallyAppliedRefsQ].ID = LineRefundsT.ID)	 "
					" 		INNER JOIN PaymentsT RefundsT ON LineRefundsT.ID = RefundsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)	 "
					" 		ON LineRefundsT.PatientID = PatientsT.PersonID)	 "
					" 		WHERE (((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (RefundsT.PayMethod NOT IN (4,10))	 "
					"       [Resp2] [PatFilter] [Category2] [From2] [To2] [RefPat] [From3] [To3] [PatientPrimSource] "
					" 		) AS _PaymentsByReferralSourcePartQ		 "
					"       WHERE (1=1)  [Prov] [Loc1]   "
					" 		) AS PaymentsByReferralSourceSubQ	 "
					" 		LEFT JOIN PersonT PersonReferringPatT ON PaymentsByReferralSourceSubQ.ReferringPatientID = PersonReferringPatT.ID) "
					" 		WHERE PersonReferringPatT.ID IS NOT NULL "
					" 		GROUP BY PersonReferringPatT.ID, PersonReferringPatT.First, PersonReferringPatT.Last, PersonReferringPatT.Middle,  TransProvID, TransLocID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, PAtProvID, PatLocID	 "
			" 		) RevenueQ	 "
			" 		GROUP BY RevenueQ.RefPatID, RevenueQ.Name"); 
		

		}
		

	

	break;


	//Referral to Consult, Referral To Procedure
	case CNVConsToSurgByRefSour:
		{
			//(e.lally 2009-09-18) PLID 35300 - Updated the referral tab to use the new base query for the consult to procedure conversion rate.
			CString strReferralID, strFrom, strWhere, strGroupBy;
			//These are the special mappings for the referral source used to indicate it was a referring physican or referred by patient.
			long nRefPatID = GetRemotePropertyInt("DefaultPatientReferral", -1, 0, "<None>", true);
			long nRefPhysID = GetRemotePropertyInt("DefaultPhysicianReferral", -1, 0, "<None>", true);

			//(e.lally 2009-12-09) PLID 35654 - Added better handling for filtering and displaying referrals depending on use
				//of referral date vs consult/first contact dates
			//Standard referrals
			if(nFilter == 1){
				if(nFlags == 4){
					//(e.lally 2010-03-12) PLID 37709 - Select either referral ID or no referral
					strReferralID.Format(
						"COALESCE(ConversionQ.PrimaryReferralID, %li) AS ReferralID, ", NOREFERRAL_SENTINEL_ID);
						/*
						"CASE WHEN ConversionQ.PrimaryReferralID IS NULL AND ConversionQ.DefaultReferringPhyID IS NULL "
						"AND ConversionQ.ReferringPatientID IS NULL THEN %li "
						"WHEN ConversionQ.PrimaryReferralID IS NULL AND ConversionQ.DefaultReferringPhyID IS NOT NULL THEN -2 "
						"WHEN ConversionQ.PrimaryReferralID IS NULL AND ConversionQ.ReferringPatientID IS NOT NULL THEN -3 "
						"ELSE ConversionQ.PrimaryReferralID END  AS ReferralID, ", NOREFERRAL_SENTINEL_ID);
						*/
					//exclude the referral sources linked to referring patient and referring physicians.
					//(e.lally 2010-03-12) PLID 37709 - Check the preferences for whether or not a linked referral source is in use or not
						//If it is in use, then the referral source must be selected for the referring phys or referring patient to be considered primary.
						//If it is NOT in use, we are going to union in the referring physician first, then check the referring patient, but only when there is not
						//already a primary referral source.
					strWhere.Format("((ConversionQ.PrimaryReferralID IS NULL %s %s) "
						"OR ReferralSourceT.PersonID NOT IN(%li, %li)) ", 
						CString(nRefPhysID == -1 ? " AND ConversionQ.DefaultReferringPhyID IS NULL " : ""),
						CString(nRefPatID == -1 ? " AND ConversionQ.ReferringPatientID IS NULL " : ""),
						nRefPatID, nRefPhysID);
					strGroupBy.Format(
						"COALESCE(ConversionQ.PrimaryReferralID, %li) ", NOREFERRAL_SENTINEL_ID);
						/*
						"CASE WHEN ConversionQ.PrimaryReferralID IS NULL AND ConversionQ.DefaultReferringPhyID IS NULL "
						"AND ConversionQ.ReferringPatientID IS NULL THEN %li "
						"WHEN ConversionQ.PrimaryReferralID IS NULL AND ConversionQ.DefaultReferringPhyID IS NOT NULL THEN -2 "
						"WHEN ConversionQ.PrimaryReferralID IS NULL AND ConversionQ.ReferringPatientID IS NOT NULL THEN -3 "
						"ELSE ConversionQ.PrimaryReferralID END ", NOREFERRAL_SENTINEL_ID);
						*/
				}
				else if(nFlags == 5){
					strReferralID.Format(
					"COALESCE(ReferralSourceT.PersonID, %li) AS ReferralID, ", NOREFERRAL_SENTINEL_ID);
					//exclude the referral sources linked to referring patient and referring physicians.
					strWhere.Format("(ReferralSourceT.PersonID NOT IN(%li, %li)) ", nRefPatID, nRefPhysID);
					strGroupBy.Format(
					"COALESCE(ReferralSourceT.PersonID, %li) ", NOREFERRAL_SENTINEL_ID);
				}
				
			}
			//Referring physician referrals
			else if(nFilter == 2){
				strReferralID = "COALESCE(ConversionQ.DefaultReferringPhyID, -2) AS ReferralID, ";
				if(nFlags == 4){
					//(e.lally 2010-03-12) PLID 37709 - Check the preferences for whether or not a linked referral source is in use or not
						//If it is in use, then the referral source must be selected for the referring phys or referring patient to be considered primary.
						//If it is NOT in use, we are going to union in the referring physician first, then check the referring patient, but only when there is not
						//already a primary referral source.
					strWhere.Format("ConversionQ.PrimaryReferralID = %li ", nRefPhysID);
					if(nRefPhysID == -1){
						//(e.lally 2010-03-12) PLID 37709 - We don't care if the referring patient is in use or not because this has higher precidence.
							//The precidence is higher only because we had to pick one. Otherwise, what is the primary referral source if a patient has
							//both a referring physician and a referring patient but no referral sources?
						strWhere += " OR (ConversionQ.PrimaryReferralID IS NULL AND ConversionQ.DefaultReferringPhyID IS NOT NULL) ";
					}
				}
				else if(nFlags == 5){
					strWhere.Format("(ConversionQ.MultiReferralID = %li) ", nRefPhysID);
				}
				strGroupBy = "COALESCE(ConversionQ.DefaultReferringPhyID, -2) ";
			}
			//Referring patient referrals
			else if(nFilter == 3){
				strReferralID = "CASE WHEN ConversionQ.ReferringPatientID IS NULL OR ConversionQ.ReferringPatientID = -2 THEN -3 "
					"ELSE ConversionQ.ReferringPatientID END AS ReferralID, ";
				if(nFlags == 4){
					//(e.lally 2010-03-12) PLID 37709 - Check the preferences for whether or not a linked referral source is in use or not
						//If it is in use, then the referral source must be selected for the referring phys or referring patient to be considered primary.
						//If it is NOT in use, we are going to union in the referring physician first, then check the referring patient, but only when there is not
						//already a primary referral source.
					strWhere.Format("ConversionQ.PrimaryReferralID = %li ", nRefPatID);
					if(nRefPatID == -1){
						strWhere +=  FormatString(" OR (ConversionQ.PrimaryReferralID IS NULL %s AND ConversionQ.ReferringPatientID IS NOT NULL) ",
							//(e.lally 2010-03-12) PLID 37709 - Only filter include patients with no referring physician if the linking preference is off.
							CString(nRefPhysID == -1 ? " AND ConversionQ.DefaultReferringPhyID IS NULL " : ""));
					}
				}
				else if(nFlags == 5){
					strWhere.Format("(ConversionQ.MultiReferralID = %li) ", nRefPatID);
				}
				strGroupBy = "CASE WHEN ConversionQ.ReferringPatientID IS NULL OR ConversionQ.ReferringPatientID = -2 THEN -3 "
					"ELSE ConversionQ.ReferringPatientID END ";
			}

			BOOL bUseMultiReferrals = FALSE;
			//Get the from clause
			if(nFlags == 4){
				strFrom = "FROM (\r\n" + GetConsultToProcedureConversionRateBaseSql() + "\r\n) ConversionQ \r\n"
				"LEFT JOIN ReferralSourceT WITH(NOLOCK) ON ConversionQ.PrimaryReferralID = ReferralSourceT.PersonID ";
			}
			//(e.lally 2009-09-24) PLID 35593 - Do the referral to consult to procedure graph
			else if(nFlags == 5){
				bUseMultiReferrals = TRUE;
				strFrom = "FROM (\r\n" + GetConsultToProcedureConversionRateBaseSql(bUseMultiReferrals) + "\r\n) ConversionQ \r\n"
				"INNER JOIN ReferralSourceT WITH(NOLOCK) ON ConversionQ.MultiReferralID = ReferralSourceT.PersonID ";
			}
			
			//(e.lally 2009-09-24) PLID 35593 - Send in the referral to consult to procedure flag
			strSql = "SELECT " + strReferralID + GetConsultToProcedureGraphStatFields(bUseMultiReferrals) + strFrom;

			if(!strWhere.IsEmpty()){
				strSql += "\r\nWHERE " + strWhere;
			}
			if(!strGroupBy.IsEmpty()){
				strSql += "\r\nGROUP BY " + strGroupBy;
			}

			ApplyDocbarGraphFilters(strSql, mktGraph, pCon, strPatientTempTable);
		}

	break;

	//Patients by Referrals
	case NUMPatByRefSour:
		{
			//TES 4/5/2004: Like all the referral source ones, we now need to do this in three parts.
			CString strSql1, strSql2, strSql3, strSql4;	

			//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.

			//nFlags is what type of referral it is
			strSql1 = " SELECT PatientsT.ReferralID, "
				"CAST (SUM(CASE WHEN SubQ.PatientID IS NULL THEN 0 ELSE 1 END) AS FLOAT) AS Conv, "
				"CAST (COUNT(DISTINCT PatientsT.PersonID) AS FLOAT) AS Referrals, "
				"CAST (SUM(CASE WHEN SubConsultQ.PatientID IS NULL THEN 0 ELSE 1 END) AS FLOAT) AS ConsConv, 0 As NumInquiries "
				"FROM PatientsT "
				"INNER JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
				//DRT 5/8/2008 - PLID 29966 - Need to join to MultiReferrals so we can filter the date
				"INNER JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID AND ReferralSourceT.PersonID = MultiReferralsT.ReferralID "


				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "

				"LEFT JOIN ( "
					"SELECT AppointmentsT.PatientID "
					"FROM AppointmentsT "
					"INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
					"WHERE (AptTypeT.Category = 4 "	//surgery
					"OR AptTypeT.Category = 3) "	//minor procedure
					"AND AppointmentsT.ShowState <> 3 "
					"AND AppointmentsT.Status <> 4 "
					" [ApptLoc] [ApptProv] ";			
			strSql2 = "SELECT PatientsT.DefaultReferringPhyID as ReferralID, "
				"CAST (SUM(CASE WHEN SubQ.PatientID IS NULL THEN 0 ELSE 1 END) AS FLOAT) AS Conv, "
				"CAST (COUNT(DISTINCT PatientsT.PersonID) AS FLOAT) AS Referrals, "
				"CAST (SUM(CASE WHEN SubConsultQ.PatientID IS NULL THEN 0 ELSE 1 END) AS FLOAT) AS ConsConv, 0 AS NumInquiries "
				"FROM PatientsT "
				"INNER JOIN ReferringPhysT ON PatientsT.DefaultReferringPhyID = ReferringPhysT.PersonID "


				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "

				"LEFT JOIN ( "
					"SELECT AppointmentsT.PatientID "
					"FROM AppointmentsT "
					"INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
					"WHERE (AptTypeT.Category = 4 "	//surgery
					"OR AptTypeT.Category = 3) "	//minor procedure
					"AND AppointmentsT.ShowState <> 3 "
					"AND AppointmentsT.Status <> 4 [ApptLoc] [ApptProv] ";
			strSql3 = "SELECT PatientsT.ReferringPatientID AS ReferralID, "
				"CAST (SUM(CASE WHEN SubQ.PatientID IS NULL THEN 0 ELSE 1 END) AS FLOAT) AS Conv, "
				"CAST (COUNT(DISTINCT PatientsT.PersonID) AS FLOAT) AS Referrals, "
				"CAST (SUM(CASE WHEN SubConsultQ.PatientID IS NULL THEN 0 ELSE 1 END) AS FLOAT) AS ConsConv, 0 As NumInquiries "
				"FROM PatientsT "
				"INNER JOIN PersonT AS PersonRefPat ON PatientsT.ReferringPatientID = PersonRefPat.ID "


				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "

				"LEFT JOIN ( "
					"SELECT AppointmentsT.PatientID "
					"FROM AppointmentsT "
					"INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
					"WHERE (AptTypeT.Category = 4 "	//surgery
					"OR AptTypeT.Category = 3) "	//minor procedure
					"AND AppointmentsT.ShowState <> 3 "
					"AND AppointmentsT.Status <> 4 [ApptLoc] [ApptProv] ";
			
				/*if(nFlags == 1) {
					//they only wish to see appointments previous to today
					strSql1 += " AND AppointmentsT.StartTime <= GetDate() ";
					strSql2 += " AND AppointmentsT.StartTime <= GetDate() ";
					strSql3 += " AND AppointmentsT.StartTime <= GetDate() ";
				}*/

			strSql1 += 
					"GROUP BY AppointmentsT.PatientID "
				") AS SubQ ON PatientsT.PersonID = SubQ.PatientID "

				"LEFT JOIN ( "
				"	SELECT "
				"	AppointmentsT.PatientID "
				"	FROM "
				"	AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"	WHERE (AptTypeT.Category = 1 "	//consult 
				"		AND AppointmentsT.ShowState <> 3 ";
			strSql2 += 
					"GROUP BY AppointmentsT.PatientID "
				") AS SubQ ON PatientsT.PersonID = SubQ.PatientID "

				"LEFT JOIN ( "
				"	SELECT "
				"	AppointmentsT.PatientID "
				"	FROM "
				"	AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"	WHERE (AptTypeT.Category = 1 "	//consult 
				"		AND AppointmentsT.ShowState <> 3 ";
			strSql3 += 
					"GROUP BY AppointmentsT.PatientID "
				") AS SubQ ON PatientsT.PersonID = SubQ.PatientID "

				"LEFT JOIN ( "
				"	SELECT "
				"	AppointmentsT.PatientID "
				"	FROM "
				"	AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"	WHERE (AptTypeT.Category = 1 "	//consult 
				"		AND AppointmentsT.ShowState <> 3 ";

				/*if(nFlags == 1) {
					//they only wish to see appointments previous to today
					strSql1 += " AND AppointmentsT.StartTime <= GetDate() ";
					strSql2 += " AND AppointmentsT.StartTime <= GetDate() ";
					strSql3 += " AND AppointmentsT.StartTime <= GetDate() ";
				}*/		

			strSql1 += 
				"		AND AppointmentsT.Status <> 4 [ApptLoc] [ApptProv] ) "
				"GROUP BY AppointmentsT.PatientID) SubConsultQ ON PatientsT.PersonID = SubConsultQ.PatientID "

				"WHERE (1 = 1) [from] "
				" [to] "
				"[Prov] "
				"[loc]  [PatFilter] [RefPhysPref] [RefPatPref] "
				"GROUP BY PatientsT.ReferralID "
				" UNION "
				" SELECT MultiReferralsT.ReferralID as RefID, 0,0,0, Count(PersonID) as NumInquiries "
				" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID "
				" WHERE PatientsT.CurrentStatus = 4 [from] [to] [loc] [PatFilter] [RefPhysPref] [RefPatPref] "
				" GROUP BY MultiReferralsT.ReferralID ";
			strSql2 += 
				"		AND AppointmentsT.Status <> 4 [ApptLoc] [ApptProv] ) "
				"GROUP BY AppointmentsT.PatientID) SubConsultQ ON PatientsT.PersonID = SubConsultQ.PatientID "

				//DRT 7/9/2008 - PLID 29966 - Alias as MultiReferralsT so we can use the existing filter, 
				//	and apply an available filter [RefPhysReferralDateFilter], so that we can filter the 
				//	subquery against a specific (set of) referrals.  This
				//	is used for the "tie a referral to a ref phys" feature to avoid duplicate counts.
				"	LEFT JOIN "
				"		(SELECT MultiReferralsT.PatientID, MultiReferralsT.Date FROM MultiReferralsT WHERE (1=1) [RefPhysReferralDateFilter] "
				"		) MultiReferralsT  "
				"	ON PatientsT.PersonID = MultiReferralsT.PatientID "

				"WHERE (PatientsT.CurrentStatus <> 4) AND  (1 = 1) [from] "
				" [to] "
				"[Prov] "
				"[loc]  [PatFilter] "
				"GROUP BY PatientsT.DefaultReferringPhyID ";
			strSql3 += 
				"		AND AppointmentsT.Status <> 4 [ApptLoc] [ApptProv] ) "
				"GROUP BY AppointmentsT.PatientID) SubConsultQ ON PatientsT.PersonID = SubConsultQ.PatientID "

				//DRT 7/9/2008 - PLID 29966 - Alias as MultiReferralsT so we can use the existing filter, 
				//	and apply an available filter [RefPatReferralDateFilter], so that we can filter the 
				//	subquery against a specific (set of) referrals.  This
				//	is used for the "tie a referral to a ref phys" feature to avoid duplicate counts.
				"	LEFT JOIN "
				"		(SELECT MultiReferralsT.PatientID, MultiReferralsT.Date FROM MultiReferralsT WHERE (1=1) [RefPatReferralDateFilter] "
				"		) MultiReferralsT  "
				"	ON PatientsT.PersonID = MultiReferralsT.PatientID "

				"WHERE (PatientsT.CurrentStatus <> 4) AND (1 = 1) [from] "
				" [to] "
				"[Prov] "
				"[loc]  [PatFilter] "
				"GROUP BY PatientsT.ReferringPatientID ";

			if (nFlags == 1) {
				strSql = " SELECT CAST (Sum(Conv) AS FLOAT) AS Conv, CAST (Sum(Referrals) AS FLOAT) AS Referrals, CAST(Sum(ConsConv) as FLOAT) AS ConsConv, CAST(Sum(NumInquiries) AS FLOAT) AS NumInquiries, [ReplacePref] ReferralID FROM (" + strSql1 + ") ReferralsQ WHERE (1=1) [ref] [TakeOutForRefPhys] [TakeOutForRefPat] GROUP BY ReferralID";
			}
			else if (nFlags == 2) {
				strSql = " SELECT CAST (Sum(Conv) AS FLOAT) AS Conv, CAST (Sum(Referrals) AS FLOAT) AS Referrals, CAST(Sum(ConsConv) as FLOAT) AS ConsConv, CAST(Sum(NumInquiries) AS FLOAT) AS NumInquiries, ReferralID FROM (" + strSql2 +  ") ReferralsQ WHERE (1=1) [ref] GROUP BY ReferralID";
			}
			else if (nFlags == 3) {
				strSql = " SELECT CAST (Sum(Conv) AS FLOAT) AS Conv, CAST (Sum(Referrals) AS FLOAT) AS Referrals, CAST(Sum(ConsConv) as FLOAT) AS ConsConv, CAST(Sum(NumInquiries) AS FLOAT) AS NumInquiries, ReferralID FROM (" + strSql3 + ") ReferralsQ WHERE (1=1) AND ReferralsQ.ReferralID <> -2 [ref] GROUP BY ReferralID";
			}

		}
	break;

	//Cancellations and No Shows By Referrals
	case REFNoShowByReferral:

		{
			//TES 4/14/2004: All referral source-based queries include ref physicians and ref patients.
			CString strSql1, strSql2, strSql3;
			strSql1 = " SELECT CAST(Sum(NumNoShow) AS FLOAT) AS NumNoShow,  ReferralID, CAST(Sum(NumCancel) AS FLOAT) AS NumCancel FROM (SELECT CAST(Count(AppointmentsT.ID) AS FLOAT) AS NumNoShow, MultiReferralsT.ReferralID, 0 AS NumCancel  "
				" FROM AppointmentsT  LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID  "
				" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  " 
				" LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID "
				" WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 3 AND MultiReferralsT.ReferralID IS NOT NULL "
				/*" AND AppointmentsT.StartTime <= GetDate()  "*/
				" [From] [To] [Prov] [Loc] [PatFilter] [RefPhysPref] [RefPatPref] "  
				"  Group By MultiReferralsT.ReferralID "
				" UNION SELECT 0 AS NumNoShow, MultiReferralsT.ReferralID, CAST(Count(AppointmentsT.ID) AS FLOAT) AS NumCancel  "
				" FROM AppointmentsT  LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID  "
				" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  " 
				" LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID "
				" WHERE AppointmentsT.Status = 4 AND AppointmentsT.ShowState <> 3 AND MultiReferralsT.ReferralID IS NOT NULL "
				/*" AND AppointmentsT.StartTime <= GetDate()  "*/
				" [From] [To] [Prov] [Loc] [PatFilter] " 
				"  Group By MultiReferralsT.ReferralID) SUBQ Group By ReferralID ";

			strSql2 = " SELECT CAST(Sum(NumNoShow) AS FLOAT) AS NumNoShow, DefaultReferringPhyID as ReferralID, CAST(Sum(NumCancel) AS FLOAT) AS NumCancel "
				" FROM (SELECT CAST(Count(AppointmentsT.ID) AS FLOAT) AS NumNoShow, PatientsT.DefaultReferringPhyID, 0 AS NumCancel  "
				" FROM AppointmentsT  LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID  "
				" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  " 
				" WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 3 AND PatientsT.DefaultReferringPhyID IS NOT NULL AND PatientsT.DefaultReferringPhyID > 0 "
				/*" AND AppointmentsT.StartTime <= GetDate()  "*/
				" [From] [To] [Prov] [Loc] [PatFilter] " 
				"  Group By PatientsT.DefaultReferringPhyID "
				" UNION SELECT 0 AS NumNoShow, PatientsT.DefaultReferringPhyID, CAST(Count(AppointmentsT.ID) AS FLOAT) AS NumCancel  "
				" FROM AppointmentsT  LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID  "
				" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  " 
				" WHERE AppointmentsT.Status = 4 AND AppointmentsT.ShowState <> 3 AND PatientsT.DefaultReferringPhyID IS NOT NULL AND PatientsT.DefaultReferringPhyID > 0 "
				/*" AND AppointmentsT.StartTime <= GetDate()  "*/
				" [From] [To] [Prov] [Loc] [PatFilter] " 
				"  Group By PatientsT.DefaultReferringPhyID) SUBQ Group By DefaultReferringPhyID ";

			strSql3 = " SELECT CAST(Sum(NumNoShow) AS FLOAT) AS NumNoShow, ReferringPatientID as ReferralID, CAST(Sum(NumCancel) AS FLOAT) AS NumCancel "
				" FROM (SELECT CAST(Count(AppointmentsT.ID) AS FLOAT) AS NumNoShow, PatientsT.ReferringPatientID, 0 AS NumCancel  "
				" FROM AppointmentsT  LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID  "
				" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  " 
				" WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 3 AND PatientsT.ReferringPatientID IS NOT NULL "
				/*" AND AppointmentsT.StartTime <= GetDate()  "*/
				" [From] [To] [Prov] [Loc] [PatFilter] " 
				"  Group By PatientsT.ReferringPatientID "
				" UNION SELECT 0 AS NumNoShow, PatientsT.ReferringPatientID, CAST(Count(AppointmentsT.ID) AS FLOAT) AS NumCancel  "
				" FROM AppointmentsT  LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID  "
				" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  " 
				" WHERE AppointmentsT.Status = 4 AND AppointmentsT.ShowState <> 3 AND PatientsT.ReferringPatientID IS NOT NULL "
				/*" AND AppointmentsT.StartTime <= GetDate()  "*/
				" [From] [To] [Prov] [Loc] [PatFilter] " 
				"  Group By PatientsT.ReferringPatientID) SUBQ Group By ReferringPatientID ";
		
			switch (nFlags) {
				case 1:
					strSql = "SELECT NumNoShow, [ReplacePref] ReferralID, NumCancel FROM (" + strSql1 + ") ReferralsQ WHERE (1=1) [ref] [TakeOutForRefPhys] [TakeOutForRefPat]";
				break;

				case 2:
					strSql = "SELECT * FROM (" + strSql2 + ") ReferralsQ WHERE (1=1) [ref] ";
				break;

				case 3:
					strSql = "SELECT * FROM (" + strSql3 + ") ReferralsQ WHERE (1=1) AND ReferralsQ.ReferralID <> -2 [ref] ";
				break;
			}

		}
	break;

	//Inquiry to Consult By Referral
	case REFInqtoCons:
		strSql = " SELECT CAST (100.0 * Sum(ProsWithCons) AS FLOAT) AS ProsWithCons, ReferralID, CAST (Sum(TotalPros) AS FLOAT) AS TotalPros, "
			" CAST (CASE WHEN Sum(TotalPros) = 0 THEN 0 ELSE SUM(ProsWithCons)/Sum(TotalPros) END AS FLOAT) AS ProsPercent FROM ( "
			"  SELECT 0 As ProsWithCons, TP.ReferralID, Count(TP.PersonID) AS TotalPros FROM  	 "
			" (SELECT PatientsT.PersonID, MultiReferralsT.ReferralID FROM PatientsT LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID  "
			" WHERE CurrentStatus = 4  	UNION  	 "
			" SELECT PatientsT.PersonID, MultiReferralsT.ReferralID FROM PatientStatusHistoryT INNER JOIN PatientsT  "
			" ON PatientStatusHistoryT.PersonID = PatientsT.PersonID LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID "
			" INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			" WHERE OldStatus = 4 "
			" [From] [To] [Prov] [Loc] [PatFilter] [Ref] " 
			" GROUP BY MultiReferralsT.ReferralID, PatientsT.PersonID ) TP " 
			" WHERE ReferralID IS NOT NULL "			
			" Group By Tp.ReferralID "
			" UNION "
			" SELECT Count(ProsWithConsQ.PersonID) AS ProsWithCons, ProsWithConsQ.ReferralID, 0 As TotalPros FROM ( "
			"  SELECT PatientsT.PersonID, MultiReferralsT.ReferralID  "
			" FROM PatientsT LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID "
			" INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			" WHERE CurrentStatus = 4  "
			" [From] [To] [Prov] [Loc] [PatFilter]  [Ref] " 
			" UNION    "
			" SELECT PatientsT.PersonID, MultiReferralsT.ReferralID " 
			" FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID "
			" LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID "
			" INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			" WHERE OldStatus = 4 "
			" [From] [To] [Prov] [Loc] [PatFilter]  [Ref] "
			" ) AS ProsWithConsQ  "
			" WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
			" 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)  "
			"  AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4  [ApptLoc] [ApptProv] ) AND ReferralID IS NOT NULL "
			" GROUP BY ReferralID  "
			" ) SubQ Group By ReferralID";
		
	break;

	//Prospects to Consults by Referral
	case REFProsToCons:
		{
			//TES 4/14/2004: All referral source-based queries include ref physicians and ref patients.
			CString strSql1, strSql2, strSql3;
			strSql1 = "SELECT 100.0 * ProsWithCons AS ProsWithCons, TotalPros, ReferralID, CAST (CASE WHEN TotalPros = 0 THEN 0 ELSE 100.0 * ((ProsWithCons)/(TotalPros)) END AS FLOAT) AS ProsPercent FROM ( "
				" SELECT CAST (Sum(ProsWithCons) AS FLOAT) AS ProsWithCons, ReferralID, CAST (Sum(TotalPros) AS FLOAT) AS TotalPros, "
				" CAST (CASE WHEN Sum(TotalPros) = 0 THEN 0 ELSE SUM(ProsWithCons)/Sum(TotalPros) END AS FLOAT) AS ProsPercent FROM ( "
				"  SELECT 0 As ProsWithCons, TP.ReferralID, Count(TP.PersonID) AS TotalPros FROM  	 "
				" (SELECT PatientsT.PersonID, MultiReferralsT.ReferralID FROM PatientsT LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID  "
				" INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE CurrentStatus = 2 "
				" [From] [To] [Prov] [Loc] [PatFilter] [RefPhysPref] [RefPatPref] "
				" UNION  	 "
				" SELECT PatientsT.PersonID, MultiReferralsT.ReferralID FROM PatientStatusHistoryT INNER JOIN PatientsT  "
				" ON PatientStatusHistoryT.PersonID = PatientsT.PersonID LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID "
				" INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE OldStatus = 2 "
				" [From] [To] [Prov] [Loc] [PatFilter] [RefPhysPref] [RefPatPref] "
				" GROUP BY MultiReferralsT.ReferralID, PatientsT.PersonID ) TP " 
				" WHERE ReferralID IS NOT NULL Group By Tp.ReferralID "
				" UNION "
				" SELECT Count(ProsWithConsQ.PersonID) AS ProsWithCons, ProsWithConsQ.ReferralID, 0 As TotalPros FROM ( "
				"  SELECT PatientsT.PersonID, MultiReferralsT.ReferralID  "
				" FROM PatientsT LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID "
				" INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE CurrentStatus = 2 "
				" [From] [To] [Prov] [Loc] [PatFilter] [RefPhysPref] [RefPatPref] "
				" UNION    "
				" SELECT PatientsT.PersonID, MultiReferralsT.ReferralID " 
				" FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID LEFT JOIN MultiReferralsT ON " 
				" PatientsT.PersonID = MultiReferralsT.PatientID "
				" INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE OldStatus = 2 "
				" [From] [To] [Prov] [Loc] [PatFilter] [RefPhysPref] [RefPatPref] "
				" ) AS ProsWithConsQ  "
				" WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
				" 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)  "
				"  AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4 [ApptLoc] [ApptProv] ) AND ReferralID IS NOT NULL "
				" GROUP BY ReferralID  "
				" ) SubQ Group By ReferralID) Q GROUP BY ReferralID, TotalPros, ProsWithCons";

			strSql2 = "SELECT 100.0 * ProsWithCons AS ProsWithCons, TotalPros, DefaultReferringPhyID as ReferralID, CAST (CASE WHEN TotalPros = 0 THEN 0 ELSE 100.0 * ((ProsWithCons)/(TotalPros)) END AS FLOAT) AS ProsPercent FROM ( "
				" SELECT CAST (Sum(ProsWithCons) AS FLOAT) AS ProsWithCons, DefaultReferringPhyID, CAST (Sum(TotalPros) AS FLOAT) AS TotalPros, "
				" CAST (CASE WHEN Sum(TotalPros) = 0 THEN 0 ELSE SUM(ProsWithCons)/Sum(TotalPros) END AS FLOAT) AS ProsPercent FROM ( "
				"  SELECT 0 As ProsWithCons, TP.DefaultReferringPhyID, Count(TP.PersonID) AS TotalPros FROM  	 "
				" (SELECT PatientsT.PersonID, PatientsT.DefaultReferringPhyID FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE CurrentStatus = 2  	"
				" [From] [To] [Prov] [Loc] [PatFilter] "
				"  UNION  	 "
				" SELECT PatientsT.PersonID, PatientsT.DefaultReferringPhyID FROM PatientStatusHistoryT INNER JOIN PatientsT  "
				" ON PatientStatusHistoryT.PersonID = PatientsT.PersonID  LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"  WHERE OldStatus = 2 "
				" [From] [To] [Prov] [Loc] [PatFilter] "
				" GROUP BY DefaultReferringPhyID, PatientsT.PersonID ) TP " 
				" WHERE DefaultReferringPhyID IS NOT NULL Group By Tp.DefaultReferringPhyID "
				" UNION "
				" SELECT Count(ProsWithConsQ.PersonID) AS ProsWithCons, ProsWithConsQ.DefaultReferringPhyID, 0 As TotalPros FROM ( "
				"  SELECT PatientsT.PersonID, PatientsT.DefaultReferringPhyID  "
				" FROM PatientsT "
				" INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE CurrentStatus = 2  "
				" [From] [To] [Prov] [Loc] [PatFilter] "
				" UNION    "
				" SELECT PatientsT.PersonID, DefaultReferringPhyID " 
				" FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID " 
				" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE OldStatus = 2 "
				" [From] [To] [Prov] [Loc] [PatFilter] "
				" ) AS ProsWithConsQ  "
				" WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
				" 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)  "
				"  AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4 [ApptLoc] [ApptProv] ) AND DefaultReferringPhyID IS NOT NULL "
				" GROUP BY DefaultReferringPhyID "
				" ) SubQ Group By DefaultReferringPhyID) Q GROUP BY DefaultReferringPhyID, TotalPros, ProsWithCons";

			strSql3 = "SELECT 100.0 * ProsWithCons AS ProsWithCons, TotalPros, ReferringPatientID as ReferralID, CAST (CASE WHEN TotalPros = 0 THEN 0 ELSE 100.0 * ((ProsWithCons)/(TotalPros)) END AS FLOAT) AS ProsPercent FROM ( "
				" SELECT CAST (Sum(ProsWithCons) AS FLOAT) AS ProsWithCons, ReferringPatientID, CAST (Sum(TotalPros) AS FLOAT) AS TotalPros, "
				" CAST (CASE WHEN Sum(TotalPros) = 0 THEN 0 ELSE SUM(ProsWithCons)/Sum(TotalPros) END AS FLOAT) AS ProsPercent FROM ( "
				"  SELECT 0 As ProsWithCons, TP.ReferringPatientID, Count(TP.PersonID) AS TotalPros FROM  	 "
				" (SELECT PatientsT.PersonID, PatientsT.ReferringPatientID FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE CurrentStatus = 2  	"
				" [From] [To] [Prov] [Loc] [PatFilter] "
				" UNION  	 "
				" SELECT PatientsT.PersonID, PatientsT.ReferringPatientID FROM PatientStatusHistoryT INNER JOIN PatientsT  "
				" ON PatientStatusHistoryT.PersonID = PatientsT.PersonID  LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"  WHERE OldStatus = 2 "
				" [From] [To] [Prov] [Loc] [PatFilter] "
				" GROUP BY ReferringPatientID, PatientsT.PersonID ) TP " 
				" WHERE ReferringPatientID IS NOT NULL Group By Tp.ReferringPatientID "
				" UNION "
				" SELECT Count(ProsWithConsQ.PersonID) AS ProsWithCons, ProsWithConsQ.ReferringPatientID, 0 As TotalPros FROM ( "
				"  SELECT PatientsT.PersonID, PatientsT.ReferringPatientID  "
				" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE CurrentStatus = 2  "
				" [From] [To] [Prov] [Loc] [PatFilter] "
				" UNION    "
				" SELECT PatientsT.PersonID, ReferringPatientID " 
				" FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID " 
				" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE OldStatus = 2 "
				" [From] [To] [Prov] [Loc] [PatFilter] "
				" ) AS ProsWithConsQ  "
				" WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
				" 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)  "
				"  AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4  [ApptLoc] [ApptProv] ) AND ReferringPatientID IS NOT NULL "
				" GROUP BY ReferringPatientID "
				" ) SubQ Group By ReferringPatientID) Q GROUP BY ReferringPatientID, TotalPros, ProsWithCons";

			switch(nFlags) {
				case 1:
					strSql = "SELECT ProsWithCons, TotalPros, [ReplacePref] ReferralID, ProsPercent FROM (" + strSql1 + ") ReferralsQ WHERE (1=1) [ref] [TakeOutForRefPhys] [TakeOutForRefPat] ";
				break;

				case 2:
					strSql = "SELECT * FROM (" + strSql2 + ") ReferralsQ WHERE (1=1) [ref] ";
				break;


				case 3:
					strSql = "SELECT * FROM (" + strSql3 + ") ReferralsQ WHERE (1=1) AND ReferralsQ.ReferralID <> -2 [ref] ";
				break;
			}

		}
	break;

	//Procedures Performed V Closed By Referral
	case REFSchedVClosed:
		{
			//TES 4/14/2004: All referral source-based queries include ref physicians and ref patients.
			CString strSql1, strSql2, strSql3;
			//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
			// (j.jones 2010-09-22 14:50) - PLID 34140 - made it so we ignore ProcInfoPaymentsT records if they are fully refunded/adjusted

			strSql1 = " SELECT CAST(Sum(ProcsClosed) AS FLOAT) AS ProcsClosed, ReferralID, CAST(Sum(ProcsPerformed) AS FLOAT) AS ProcsPerformed FROM  "
				"	(SELECT CAST(Count(AppointmentPurposeT.AppointmentID) AS FLOAT) AS ProcsClosed, MultiReferralsT.ReferralID, 0 AS ProcsPerformed  "
				"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
				"	 AppointmentsT.PatientID = PatientsT.PersonID "
				"	 LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID  "
				"	 LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID "
				"	 WHERE (PersonT.ID > 0) AND AppointmentsT.ID IN (SELECT SurgeryApptID FROM ProcInfoT "
				"	 INNER JOIN (SELECT * FROM ProcInfoPaymentsT WHERE PayID NOT IN ( "
				"		SELECT LineItemT.ID FROM LineItemT "
				"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"		INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID "
				"		GROUP BY LineItemT.ID, LineItemT.Amount "
				"		HAVING LineItemT.Amount = Sum(-AppliesT.Amount))) AS ProcInfoPaymentsT ON ProcInfoT.ID = ProcInfoPaymentsT.ProcInfoID) "
				"    [From] [To] [Prov] [Loc] [PatFilter] [RefPhysPref] [RefPatPref] "
				"	 AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND MultiReferralsT.ReferralID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4) "
				"	 GROUP BY MultiReferralsT.ReferralID "
				"	 UNION SELECT 0 AS ProcsClosed, MultiReferralsT.ReferralID, CAST(Count(AppointmentPurposeT.AppointmentID) AS FLOAT) AS ProcsPerformed   "
				"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
				" 	 AppointmentsT.PatientID = PatientsT.PersonID "
				"	  LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID  "
				"	 LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID "
				"	  WHERE (PersonT.ID > 0) "
				"	  [From] [To] [Prov] [Loc] [PatFilter] [RefPhysPref] [RefPatPref] "
				"	  AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 2 AND MultiReferralsT.ReferralID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4) "
				"	  GROUP BY MultiReferralsT.ReferralID) SUBQ Group By ReferralID";

			strSql2 = " SELECT CAST(Sum(ProcsClosed) AS FLOAT) AS ProcsClosed, DefaultReferringPhyID as ReferralID, CAST(Sum(ProcsPerformed) AS FLOAT) AS ProcsPerformed FROM  "
				"	(SELECT CAST(Count(AppointmentPurposeT.AppointmentID) AS FLOAT) AS ProcsClosed, PatientsT.DefaultReferringPhyID, 0 AS ProcsPerformed  "
				"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
				"	 AppointmentsT.PatientID = PatientsT.PersonID "
				"	 LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID  "
				"	 WHERE (PersonT.ID > 0) AND AppointmentsT.ID IN (SELECT SurgeryApptID FROM ProcInfoT "
				"	 INNER JOIN (SELECT * FROM ProcInfoPaymentsT WHERE PayID NOT IN ( "
				"		SELECT LineItemT.ID FROM LineItemT "
				"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"		INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID "
				"		GROUP BY LineItemT.ID, LineItemT.Amount "
				"		HAVING LineItemT.Amount = Sum(-AppliesT.Amount))) AS ProcInfoPaymentsT ON ProcInfoT.ID = ProcInfoPaymentsT.ProcInfoID) "
				"    [From] [To] [Prov] [Loc] [PatFilter] "
				"	 AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND PatientsT.DefaultReferringPhyID IS NOT NULL AND PatientsT.DefaultReferringPhyID > 0 AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4) "
				"	 GROUP BY PatientsT.DefaultReferringPhyID "
				"	 UNION SELECT 0 AS ProcsClosed, PatientsT.DefaultReferringPhyID, CAST(Count(AppointmentPurposeT.AppointmentID) AS FLOAT) AS ProcsPerformed   "
				"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
				" 	 AppointmentsT.PatientID = PatientsT.PersonID "
				"	  LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID  "
				"	  WHERE (PersonT.ID > 0) "
				"	  [From] [To] [Prov] [Loc] [PatFilter] "
				"	  AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 2 AND PatientsT.DefaultReferringPhyID IS NOT NULL AND PatientsT.DefaultReferringPhyID > 0 AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4) "
				"	  GROUP BY PatientsT.DefaultReferringPhyID) SUBQ Group By DefaultReferringPhyID ";

			strSql3 = " SELECT CAST(Sum(ProcsClosed) AS FLOAT) AS ProcsClosed, ReferringPatientID as ReferralID, CAST(Sum(ProcsPerformed) AS FLOAT) AS ProcsPerformed FROM  "
				"	(SELECT CAST(Count(AppointmentPurposeT.AppointmentID) AS FLOAT) AS ProcsClosed, PatientsT.ReferringPatientID, 0 AS ProcsPerformed  "
				"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
				"	 AppointmentsT.PatientID = PatientsT.PersonID "
				"	 LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID  "
				"	 WHERE (PersonT.ID > 0) AND AppointmentsT.ID IN (SELECT SurgeryApptID FROM ProcInfoT "
				"	 INNER JOIN (SELECT * FROM ProcInfoPaymentsT WHERE PayID NOT IN ( "
				"		SELECT LineItemT.ID FROM LineItemT "
				"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"		INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID "
				"		GROUP BY LineItemT.ID, LineItemT.Amount "
				"		HAVING LineItemT.Amount = Sum(-AppliesT.Amount))) AS ProcInfoPaymentsT ON ProcInfoT.ID = ProcInfoPaymentsT.ProcInfoID) "
				"    [From] [To] [Prov] [Loc] [PatFilter] "
				"	 AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3  AND PatientsT.ReferringPatientID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4) "
				"	 GROUP BY PatientsT.ReferringPatientID "
				"	 UNION SELECT 0 AS ProcsClosed, PatientsT.ReferringPatientID, CAST(Count(AppointmentPurposeT.AppointmentID) AS FLOAT) AS ProcsPerformed   "
				"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
				" 	 AppointmentsT.PatientID = PatientsT.PersonID "
				"	  LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID  "
				"	  WHERE (PersonT.ID > 0) "
				"	  [From] [To] [Prov] [Loc] [PatFilter] "
				"	  AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 2 AND PatientsT.ReferringPatientID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4) "
				"	  GROUP BY PatientsT.ReferringPatientID) SUBQ Group By ReferringPatientID ";
			switch (nFlags) {
				case 1:
					strSql = "SELECT ProcsClosed, [ReplacePref] ReferralID, ProcsPerformed FROM (" + strSql1 +") ReferralsQ WHERE (1=1) [ref] [TakeOutForRefPhys] [TakeOutForRefPat] ";
				break;

				case 2:
					strSql = "SELECT * FROM (" + strSql2 +") ReferralsQ WHERE (1=1) [ref] ";
				break;

				case 3:
					strSql = "SELECT * FROM (" + strSql3 +") ReferralsQ WHERE (1=1) AND ReferralsQ.ReferralID <> -2 [ref] ";
				break;
			}
		}
	break;

	/**************************************END GRAPHS OF THE REFERRALS TAB*********************************************************/







	/****************************************GRAPHS OF THE PROCEDURE TAB*************************************************/
	/*Revenue by Procedure*/
	case EFFMonByProc:
		// (j.gruber 2009-03-18 11:34) - PLID 33574 - changed discount structure
		strSql = "SELECT ProcID as PurposeID, CAST(Sum(SumMoney) AS FLOAT) AS SumMoney, CAST(Sum(CASE WHEN (SumMoney - Tax1 - Tax2) < 0 THEN 0 ELSE (SumMoney - Tax1 - Tax2) END)  AS FLOAT) AS SumNoTax, Name  "
			"     FROM  "
			"	 (SELECT CASE WHEN ProcedureID IS NULL THEN -1 ELSE ServiceT.ProcedureID END AS ProcID,  "
			"	 SUM(AppliesT.Amount) AS SumMoney,   "
			"	 Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate - 1)) AS Tax1,   "
			"    Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate2 - 1)) AS Tax2, ProcedureT.Name   "
			"    FROM LineItemT LinePaymentsT INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID     "
			"    LEFT JOIN AppliesT ON LinePaymentsT.ID = AppliesT.SourceID LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID    "
			"    LEFT JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID    "
			"	 LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID "
			"	 LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID "
			"    LEFT JOIN PatientsT ON LinePaymentsT.PatientID = PatientsT.PersonID LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID    "
			"    LEFT JOIN PersonT CoordPersonT ON ChargesT.PatCoordID = CoordPersonT.ID    "
			"    LEFT JOIN PersonT ProvPersonT ON ChargesT.DoctorsProviders = ProvPersonT.ID LEFT JOIN LocationsT ON LineChargesT.LocationID = LocationsT.ID     "
			" 	 LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
			" 	 LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID "
			"	 WHERE LinePaymentsT.Deleted = 0 AND LineChargesT.Deleted = 0 AND LinePaymentsT.Type = 1 AND LineChargesT.Type = 10 AND    "
			"	 AppliesT.ID IS NOT NULL  AND ServiceT.ProcedureID IS NOT NULL AND ProcedureT.MasterProcedureID IS NULL "
			"	 [From] [To] [Prov1] [Loc1] [PatFilter] [Resp] [Category]  "
			"	 GROUP BY ServiceT.ProcedureID, ProcedureT.Name "
			"  UNION  "
			"    SELECT CASE WHEN ProcedureID IS NULL THEN -1 ELSE MasterProcT.ID END AS ProcID,   "
			" 	 SUM(AppliesT.Amount) AS SumMoney,   "
			" 	 Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate - 1)) AS Tax1,  "
			"	 Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate2 - 1)) AS Tax2,  "
			"    MasterProcT.Name   "
			"	 FROM LineItemT LinePaymentsT INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID     "
			"    LEFT JOIN AppliesT ON LinePaymentsT.ID = AppliesT.SourceID LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID    "
			"    LEFT JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID    "
			"	 LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID "
			"	 LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID "
			"    LEFT JOIN PatientsT ON LinePaymentsT.PatientID = PatientsT.PersonID LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID    "
			"    LEFT JOIN PersonT CoordPersonT ON ChargesT.PatCoordID = CoordPersonT.ID    "
			"    LEFT JOIN PersonT ProvPersonT ON ChargesT.DoctorsProviders = ProvPersonT.ID LEFT JOIN LocationsT ON LineChargesT.LocationID = LocationsT.ID     "
			"    LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
			" 	 LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID "
			"    LEFT JOIN ProcedureT MasterProcT ON ProcedureT.MasterProcedureID = MasterProcT.ID "
			" 	 WHERE LinePaymentsT.Deleted = 0 AND LineChargesT.Deleted = 0 AND LinePaymentsT.Type = 1 AND LineChargesT.Type = 10 AND   "
			"	 AppliesT.ID IS NOT NULL  AND ServiceT.ProcedureID IS NOT NULL AND ProcedureT.MasterProcedureID IS NOT NULL "
			"	 [From] [To] [Prov1] [Loc1] [PatFilter] [Resp] [Category]  "
			" 	 GROUP BY ServiceT.ProcedureID, MasterProcT.ID, MasterProcT.Name)SubQ "
			"    GROUP BY ProcID, Name";
	break;

	/*Consult To Procedure By Procedure*/
	case PROCConsultToSurg:
		//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
		//(e.lally 2009-08-24) PLID 35298 - Updated the procedure tab to use the new base query for the consult to procedure conversion rate.
		//(e.lally 2009-08-31) PLID 35298 - Split the query up into the graph details and the graph summary. Made sure we are using distinct counts
		if(nFlags == 1){
			strSql = 
				//(e.lally 2009-08-24) PLID 35298 - So that we don't break backwards compatibility with the other graphs, 
					//we are re-using the PurposeID and Name aliases
				//(e.lally 2009-09-01) PLID 35429 - updated to use consult splitting preference
				"SELECT ConversionQ.MasterProcedureID AS PurposeID, ConversionQ.MasterProcedureName AS Name, \r\n" +
					GetConsultToProcedureGraphStatFields() +

				"FROM (\r\n" + GetConsultToProcedureConversionRateBaseSql() + "\r\n) ConversionQ " +
				"GROUP BY ConversionQ.MasterProcedureID, ConversionQ.MasterProcedureName ";
		}
		else if(nFlags == 2){
			//(e.lally 2009-08-31) PLID 35298 - We can't use a group by here or we will end up double counting the unique appts
			// (z.manning 2009-09-09 13:55) - PLID 35051 - Updated this to use the stat fields
			strSql = 
			"SELECT " +
				GetConsultToProcedureGraphStatFields() +
			"FROM (\r\n" + GetConsultToProcedureConversionRateBaseSql() + "\r\n) ConversionQ ";
		}
		else{
			ASSERT(FALSE);
		}
		//(e.lally 2009-08-25) PLID 35298 - Replace the filter placeholders with the actual filters selected before returning our query string.
		ApplyDocbarGraphFilters(strSql, mktGraph, pCon, strPatientTempTable);
	break;

	/*Patients By Procedure*/
	case PROCPatients:
		strSql = " SELECT CAST(Sum(PatCount) AS FLOAT) AS PatCount, ProcedureID AS PurposeID, CAST(Sum(InqCount) AS FLOAT) AS InqCount, CAST(Sum(ProsCount) AS FLOAT) AS ProsCount, Name FROM ( "
			" SELECT CAST (Count(PersonID) AS FLOAT) AS PatCount, ProcInfoDetailsT.ProcedureID, 0 As InqCount, 0 AS ProsCount, ProcedureT.Name  "
			" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID    "
			" LEFT JOIN ProcInfoT ON PatientsT.PersonID = ProcInfoT.PatientID LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID "
			" LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
			" WHERE CurrentStatus = 1  AND ProcInfoDetailsT.ProcedureID IS NOT NULL  AND ProcedureT.MasterProcedureID Is Null "
			"  [From] [To] [PatFilter] [Loc1] [Prov1] "
			" GROUP BY ProcInfoDetailsT.ProcedureID, ProcedureT.Name  "			
			" UNION ALL SELECT 0 AS PatCount, ProcInfoDetailsT.ProcedureID, 0 As InqCount, CAST (Count(PersonID) AS FLOAT) AS ProsCount, ProcedureT.Name  "
			" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
			" LEFT JOIN ProcInfoT ON PatientsT.PersonID = ProcInfoT.PatientID LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID "
			" LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
			"  WHERE CurrentStatus = 2 AND ProcInfoDetailsT.ProcedureID IS NOT NULL  AND ProcedureT.MasterProcedureID Is Null "
			"   [From] [To] [PatFilter] [Loc1] [Prov1] "
			" GROUP BY ProcInfoDetailsT.ProcedureID, ProcedureT.Name "
			
			"  UNION ALL SELECT 0 AS PatCount, ProcInfoDetailsT.ProcedureID, CAST(Count(PersonID) AS FLOAT)  As InqCount, 0 As ProsCount, ProcedureT.Name  "
			"  FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
			"  LEFT JOIN ProcInfoT ON PatientsT.PersonID = ProcInfoT.PatientID LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID "
			"  LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
			"  WHERE CurrentStatus = 4 AND ProcInfoDetailsT.ProcedureID IS NOT NULL  "
			"  [From] [To] [PatFilter] [Loc1] /*[Prov1]*/ "
			" GROUP BY ProcInfoDetailsT.ProcedureID, ProcedureT.Name "
			
			") SubQ GROUP BY ProcedureID, Name ";
	break;

	/*Cancellations and No Shows By Procedure*/
	case NUMNoShowByProc:

		strSql = " SELECT CAST(SUM(NumNoShow) AS FLOAT) AS NumNoShow, PurposeID, CAST(Sum(NumCancel) AS FLOAT) AS NumCancel, Name FROM ( "
		 " SELECT CAST(Count(AppointmentsT.ID) AS FLOAT) AS NumNoShow, "
		 " CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN  AppointmentPurposeT.PurposeID ELSE ProcedureT.MasterProcedureID END AS PurposeID, 0 As NumCancel, "
		 " CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AptPurposeT.Name ELSE (SELECT Name FROM ProcedureT ProcInner WHERE ProcInner.ID = ProcedureT.MasterProcedureID) END AS Name"
		 " FROM AppointmentsT LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
		 " LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		 " LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID "
		 " INNER JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID "
		 " WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 3 AND AppointmentPurposeT.PurposeID IS NOT NULL ";

		
		if (nFlags ==1) {
			//strSql += " AND AppointmentsT.StartTime <= GetDate() ";
		}

		strSql += " [From] [To] [Prov1] [Loc1] [PatFilter] [Loc2] [Prov2] "
			" Group By AppointmentPurposeT.PurposeID, AptPurposeT.Name, ProcedureT.MasterProcedureID ";

		strSql +=   " UNION ALL SELECT 0 AS NumNoShow,  "
		 " CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN  AppointmentPurposeT.PurposeID ELSE ProcedureT.MasterProcedureID END AS PurposeID,"
		 " Count(AppointmentID) As NumCancel, "
		 " CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AptPurposeT.Name ELSE (SELECT Name FROM ProcedureT ProcInner WHERE ProcInner.ID = ProcedureT.MasterProcedureID) END AS Name"
		 " FROM AppointmentsT LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
		 " LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		 " LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID "
		 " INNER JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID "
		 " WHERE AppointmentsT.Status = 4 AND AppointmentsT.ShowState <> 3 AND AppointmentPurposeT.PurposeID IS NOT NULL"  ;
		
		if (nFlags ==1) {
			//strSql += " AND AppointmentsT.StartTime <= GetDate() ";
		}

		strSql += " [From] [To] [Prov1] [Loc1] [PatFilter] [Loc2] [Prov2] "
			" Group By AppointmentPurposeT.PurposeID, AptPurposeT.Name, ProcedureT.MasterProcedureID) SubQ Group By PurposeID, Name ";

	break;

	/*Inquiries To Consults By Procedure*/
	case CNVInqToConsByProc:

		strSql = "SELECT CAST(SUM(NumInquiries) AS FLOAT) AS NumInquiries, CAST (SUM(ConsCount) AS FLOAT) * 100.0 AS ConsCount, ProcedureID AS PurposeID,  "
			" CAST(CASE WHEN SuM(NumInquiries) = 0 THEN 0 ELSE 100.0 * Sum(ConsCount)/Sum(NumInquiries) END AS FLOAT) as ConvRate, Name "
			" FROM ( "
			" SELECT Count(PersonID) AS NumInquiries, 0 AS ConsCount, ProcInfoDetailsT.ProcedureID, ProcedureT.Name  FROM "
			" PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID LEFT JOIN ProcInfoT ON PersonT.ID = ProcInfoT.PatientID LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID "
			" LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID " 
			" WHERE (PatientsT.CurrentStatus = 4 "
			" OR PatientsT.PersonID IN (SELECT PersonID FROM PatientStatusHistoryT WHERE OldStatus = 4)) "
			" [From] [To] [Loc1] [PatFilter] "
			" GROUP BY ProcedureID, Name "
			" UNION ALL "
			" SELECT 0, Count(AppointmentsT.ID) AS ConsCount, "
			" CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AppointmentPurposeT.PurposeID ELSE ProcedureT.MasterProcedureID END AS PurposeID,  "
			" CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AptPurposeT.Name ELSE (SELECT Name FROM ProcedureT ProcInner WHERE ProcInner.ID = ProcedureT.MasterProcedureID) END AS Name "
			" FROM AppointmentsT  "
			" LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
			" LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
			" LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
			" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			" LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID "
			" INNER JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID "
			" WHERE (AptTypeT.Category = 1) AND PatientID IN (SELECT PersonID FROM PatientStatusHistoryT WHERE OldStatus = 4)  "
			" AND AppointmentsT.ShowState <> 3  "
			" AND AppointmentsT.Status <> 4   "
			" [From] [To] [Loc1]  [PatFilter] [Loc2] [Prov2] [Prov1] "
			" GROUP BY AppointmentPurposeT.PurposeID, AptPurposeT.Name, ProcedureT.MasterProcedureID) SubQ "
			" WHERE ProcedureID IS NOT NULL "
			" Group By ProcedureID, Name ";
	break;

	/*Prospects to Consults By Procedure*/
	case CNVProsToConsByProc:
		strSql = "SELECT CAST(Sum(Consults) AS FLOAT) AS Consults, PurposeID, CAST(Sum(ProspectCount) AS Float) AS ProspectCount, Name "
			" FROM  "
			" ( "
			" SELECT 100 * Count(AppointmentsT.ID) AS Consults, "
			" CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AppointmentPurposeT.PurposeID ELSE ProcedureT.MasterProcedureID END AS PurposeID,  "
			" 0 AS ProspectCount, "
			" CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AptPurposeT.Name ELSE (SELECT Name FROM ProcedureT ProcInner WHERE ProcInner.ID = ProcedureT.MasterProcedureID) END AS Name "
			" FROM AppointmentsT LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			" LEFT JOIN PatientsT ON AppointmentsT.PatientId = PatientsT.PersonID  "
			" LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
			" LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID "
			" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			" INNER JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID "
			" WHERE (AptTypeT.Category = 1) AND AppointmentsT.ShowState <> 3 AND AppointmentsT.STatus <> 4 AND (PatientsT.CurrentStatus = 2 "
			" OR PatientsT.PersonID IN (SELECT PersonID FROM PatientStatusHistoryT WHERE OldStatus = 2)) "
			" [From] [To] [Prov1] [Loc1] [PatFilter] [Loc2] [Prov2] "
			" GROUP BY AppointmentPurposeT.PurposeID, AptPurposeT.Name, ProcedureT.MasterProcedureID "
			" "
			" UNION ALL SELECT 0 AS Consults, ProcInfoDetailsT.ProcedureID AS PurposeID,  Count(PersonT.ID) AS ProspectCount, ProcedureT.Name "
			" FROM PersonT LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			" LEFT JOIN ProcInfoT ON PersonT.ID = ProcInfoT.PatientID LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID "
			" LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
			" WHERE (PatientsT.CurrentStatus = 2 OR PatientsT.PersonID IN (SELECT PersonID FROM PatientStatusHistoryT WHERE OldStatus = 2)) "
			" [From] [To] [Prov1] [Loc1] [PatFilter] "
			" GROUP BY ProcInfoDetailsT.ProcedureID, ProcedureT.Name "
			") AS Query GROUP BY PurposeID, Name";  

	break;

	/*Procedures Performed Vs Closed By Procedure*/
	case PROCSchedVClosed:
		//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
		// (j.jones 2010-09-22 14:50) - PLID 34140 - made it so we ignore ProcInfoPaymentsT records if they are fully refunded/adjusted
		strSql = "SELECT CAST(Sum(ProcsClosed) AS FLOAT) AS ProcsClosed, PurposeID, CAST(Sum(ProcsPerformed) AS FLOAT) AS ProcsPerformed, Name FROM "
			"(SELECT CAST(Count(AppointmentPurposeT.AppointmentID) AS FLOAT) AS ProcsClosed, "
			" CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AppointmentPurposeT.PurposeID ELSE ProcedureT.MasterProcedureID END AS PurposeID, "
			" 0 AS ProcsPerformed,  "
			" CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AptPurposeT.Name ELSE (SELECT Name FROM ProcedureT ProcInner WHERE ProcInner.ID = ProcedureT.MasterProcedureID) END AS Name "
			"FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON "
			"AppointmentsT.PatientID = PatientsT.PersonID "
			"LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID "
			" LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID "
			" INNER JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID "
			"WHERE AppointmentsT.ID IN (SELECT SurgeryApptID FROM ProcInfoT "
			"	INNER JOIN (SELECT * FROM ProcInfoPaymentsT WHERE PayID NOT IN ( "
			"		SELECT LineItemT.ID FROM LineItemT "
			"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"		INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID "
			"		GROUP BY LineItemT.ID, LineItemT.Amount "
			"		HAVING LineItemT.Amount = Sum(-AppliesT.Amount))) AS ProcInfoPaymentsT ON ProcInfoT.ID = ProcInfoPaymentsT.ProcInfoID) "
			"[From] [To] [Prov1] [Loc1] [PatFilter] "
			"AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND AppointmentPurposeT.PurposeID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4) "
			"GROUP BY AppointmentPurposeT.PurposeID, AptPurposeT.Name, ProcedureT.MasterProcedureID "
			" UNION ALL "
			" SELECT 0 AS ProcsClosed, "
			" CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AppointmentPurposeT.PurposeID ELSE ProcedureT.MasterProcedureID END AS PurposeID,  "
			" CAST(Count(AppointmentPurposeT.AppointmentID) AS FLOAT) AS ProcsPerformed, "
			" CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AptPurposeT.Name ELSE (SELECT Name FROM ProcedureT ProcInner WHERE ProcInner.ID = ProcedureT.MasterProcedureID) END AS Name "
			" FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			" LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON "
			" AppointmentsT.PatientID = PatientsT.PersonID "
			" LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID "
			" LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID "
			" INNER JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID "
			" WHERE AppointmentsT.Status <> 4  "
			" [From] [To] [Prov1] [Loc1] [PatFilter] "
			" AND AppointmentsT.ShowState = 2 AND AppointmentPurposeT.PurposeID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4) "
			" GROUP BY AppointmentPurposeT.PurposeID, AptPurposeT.Name, ProcedureT.MasterProcedureID) SUBQ Group By PurposeID, Name";
	break;

	/*Revenue By Category*/
	// (j.gruber 2009-03-18 11:37) - PLID 33574 - updated discount structure
	case EFFMonByCategory:

		strSql = "SELECT CategoryID, CAST(SumMoney AS FLOAT) AS SumMoney, CAST(CASE WHEN (SumMoney - Tax1 - Tax2) < 0 THEN 0 ELSE (SumMoney - Tax1 - Tax2) END  AS FLOAT) AS SumNoTax, Name "
			" FROM  "
			" (SELECT CASE WHEN ServiceT.Category IS NULL THEN -1 ELSE ServiceT.Category END AS CategoryID,  "
			" SUM(AppliesT.Amount) AS SumMoney,   "
			" Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate - 1)) AS Tax1,  "
			" Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate2 - 1)) AS Tax2, CategoriesT.Name "
			" FROM LineItemT LinePaymentsT INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID    "
			"  LEFT JOIN AppliesT ON LinePaymentsT.ID = AppliesT.SourceID LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID   "
			"  LEFT JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID   "
			"  LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID "
			"  LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID "
			"  LEFT JOIN PatientsT ON LinePaymentsT.PatientID = PatientsT.PersonID LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
			"  LEFT JOIN PersonT CoordPersonT ON ChargesT.PatCoordID = CoordPersonT.ID   "
			"  LEFT JOIN PersonT ProvPersonT ON ChargesT.DoctorsProviders = ProvPersonT.ID LEFT JOIN LocationsT ON LineChargesT.LocationID = LocationsT.ID    "
			"  LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			" LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			" WHERE LinePaymentsT.Deleted = 0 AND LineChargesT.Deleted = 0 AND LinePaymentsT.Type = 1 AND LineChargesT.Type = 10 AND   "
			" AppliesT.ID IS NOT NULL   "
			" [Cat] [From] [To] [Prov1] [Loc1] [PatFilter] [Resp] [Category]  "
			" GROUP BY ServiceT.Category, CategoriesT.Name) SUBQ "; 
		
	break;
	/********************************END GRAPHS OF THE PROCEDURE TAB*****************************************************/

	/***********************************GRAPHS OF THE COORDINATOR TAB **************************************************/

	/*Revenue By Coordinator*/
	// (j.gruber 2009-03-18 11:43) - PLID 33574 - updated discount structure
	case EFFMonByPatCoord:

		strSql = "SELECT CoordID AS EmployeeID, CAST(SumMoney AS FLOAT) AS SumMoney, CAST(CASE WHEN (SumMoney - Tax1 - Tax2) < 0 THEN 0 ELSE (SumMoney - Tax1 - Tax2) END  AS FLOAT) AS SumNoTax, CoordFirst, CoordMiddle, CoordLast "
			" FROM "
			" (SELECT CASE WHEN PatCoordID IS NULL THEN -1 ELSE ChargesT.PatCoordID END AS CoordID,  "
			" SUM(AppliesT.Amount) AS SumMoney,   "
			"  Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate - 1)) AS Tax1, "
			" Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate2 - 1)) AS Tax2, CoordPersonT.First AS CoordFirst, CoordPersonT.Last AS CoordLast, CoordPersonT.Middle AS CoordMiddle"
			" FROM LineItemT LinePaymentsT INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID   "
			"  LEFT JOIN AppliesT ON LinePaymentsT.ID = AppliesT.SourceID LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID  " 
			"  LEFT JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID   "
			"  LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID "
			"  LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID "
			"  LEFT JOIN PatientsT ON LinePaymentsT.PatientID = PatientsT.PersonID LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID  " 
			"  LEFT JOIN PersonT CoordPersonT ON ChargesT.PatCoordID = CoordPersonT.ID   "
			"  LEFT JOIN PersonT ProvPersonT ON ChargesT.DoctorsProviders = ProvPersonT.ID LEFT JOIN LocationsT ON LineChargesT.LocationID = LocationsT.ID   " 
			"  LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			" WHERE LinePaymentsT.Deleted = 0 AND LineChargesT.Deleted = 0 AND LinePaymentsT.Type = 1 AND LineChargesT.Type = 10 AND   "
			" AppliesT.ID IS NOT NULL  "
			" [From] [To] [Prov1] [Loc1] [PatFilter] [Resp] [Category] "
			"  GROUP BY PatCoordID, CoordPersonT.First, CoordPersonT.Last, CoordPersonT.Middle) SUBQ ";
		
	break;

	/*Consults To Procedure By Coordinator*/
	case CNVConsToSurgByPatCoord:

		//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
		//(e.lally 2009-08-24) PLID 35299 - Updated the coordinator tab to use the new base query for the consult to procedure conversion rate.
		//(e.lally 2009-08-31) PLID 35299 - Made sure we are using distinct counts
		//(e.lally 2009-09-01) PLID 35429 - updated to use consult splitting preference
		strSql = 
			"SELECT " +	GetConsultToProcedureGraphStatFields() + ", "
			  //(e.lally 2009-08-24) PLID 35299 - In order to keep backwards compatibility, leave these aliases as they were.
				"PatCoordT.ID AS EmployeeID, COALESCE(PatCoordT.Last, '<No Coordinator>') AS CoordLast, \r\n"
				"COALESCE(PatCoordT.First, '') As CoordFirst, COALESCE(PatCoordT.Middle, '') As CoordMiddle   \r\n"
			"FROM (\r\n" + GetConsultToProcedureConversionRateBaseSql() + "\r\n) ConversionQ \r\n"
			"LEFT JOIN UsersT WITH(NOLOCK) ON ConversionQ.PatientCoordinatorID = UsersT.PersonID \r\n"
			"LEFT JOIN PersonT PatCoordT WITH(NOLOCK) ON UsersT.PersonID  = PatCoordT.ID \r\n"
			"GROUP BY PatCoordT.ID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last \r\n";

		//(e.lally 2009-08-25) PLID 35299 - Replace the filter placeholders with the actual filters selected before returning our query string.
		ApplyDocbarGraphFilters(strSql, mktGraph, pCon, strPatientTempTable);
	break;

	/*Patients By Coordinator*/
	case COORDPatients:
		strSql = " SELECT CAST(Sum(PatCount) AS FLOAT) AS PatCount, EmployeeID, "
			"			 CAST(Sum(ProsCount) AS FLOAT) AS ProsCount FROM ( "
			"			 SELECT CAST (Count(PersonID) AS FLOAT) AS PatCount, EmployeeID, 0 As ProsCount, PatCoordT.First as CoordFirst, PatCoordT.Middle As CoordMiddle, PatCoordT.Last as CoordLast "
			"			 FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"			 LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
			"			 WHERE CurrentStatus = 1 AND PersonT.ID > 0 AND EmployeeID IS NOT NULL  "
			"			[From] [To] [Prov1] [Loc1] [PatFilter] "
			"			 GROUP BY EmployeeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last  "
			"			 UNION "
			"			 SELECT 0 AS PatCount, EmployeeID, CAST (Count(PersonID) AS FLOAT) As ProsCount, PatCoordT.First as CoordFirst, PatCoordT.Middle As CoordMiddle, PatCoordT.Last as CoordLast "
			"			 FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"			 LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
			"			 WHERE CurrentStatus = 2 AND PersonT.ID > 0 AND EmployeeID IS NOT NULL "
			"			[From] [To] [Prov1] [Loc1] [PatFilter] "
			"			 GROUP BY EmployeeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last) SubQ Group By EmployeeID, CoordFirst, CoordMiddle, CoordLast ";
	break;

	/*Cancellations and No Shows By Patient Coodinator*/
	case NUMCanByPatCoord:

		strSql = "SELECT CAST (Sum(NumCons) AS FLOAT) AS NumCons, EmployeeID, "
			" CAST (Sum(NumNoShows) AS FLOAT) AS NumNoShows, CoordFirst, CoordMiddle, CoordLast FROM ("
			" SELECT CAST (Count(AppointmentsT.ID) AS FLOAT) AS NumCons, PatientsT.EmployeeID, 0 AS NumNoShows, PatCoordT.First as CoordFirst, PatCoordT.Middle as CoordMiddle, PatCoordT.Last AS CoordLast "
			" FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
			" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			" LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
			" WHERE AppointmentsT.Status = 4  AND PatientsT.EmployeeID IS NOT NULL ";
		if (nFlags ==1) {
			//strSql += " AND AppointmentsT.StartTime <= GetDate() ";
		}

		strSql += " [From] [To] [Prov1] [Loc1] [PatFilter] "
			" Group By PatientsT.EmployeeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last ";
		strSql += " UNION SELECT 0 AS NumCans, PatientsT.EmployeeID, "
			" CAST (Count(AppointmentsT.ID) AS FLOAT) AS NumNoShows, PatCoordT.First as CoordFirst, PatCoordT.Middle as CoordMiddle, PatCoordT.Last AS CoordLast "
			" FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
			" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			" LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
			" WHERE AppointmentsT.Status <> 4  AND AppointmentsT.ShowState = 3 AND "
			" PatientsT.EmployeeID IS NOT NULL  ";
		if (nFlags ==1) {
			//strSql += " AND AppointmentsT.StartTime <= GetDate() ";
		}

		strSql += " [From] [To] [Prov1] [Loc1] [PatFilter] "
			" Group By PatientsT.EmployeeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last) Sub Group By EmployeeID, CoordFirst, CoordMiddle, CoordLast ";
	break;


	/*Inquiry To Consult By Staff*/
	case CNVInqToConsByStaff:
		strSql = "	SELECT CAST (ProsWithCons AS FLOAT) * 100.0 AS ProsWithCons, UserID, CAST(TotalPros AS FLOAT) AS TotalPros, "
"			 100.0 * CAST (CASE WHEN TotalPros = 0 THEN 0 ELSE ProsWithCons/TotalPros END AS FLOAT) AS ProsPercent, CoordFirst, CoordMiddle, CoordLast FROM ( "
"			 SELECT CAST (Sum(ProsWithCons) AS FLOAT) AS ProsWithCons, UserID, CAST (Sum(TotalPros) AS FLOAT) AS TotalPros, "
"			 CAST (CASE WHEN Sum(TotalPros) = 0 THEN 0 ELSE SUM(ProsWithCons)/Sum(TotalPros) END AS FLOAT) AS ProsPercent, CoordFirst, CoordMiddle, CoordLast FROM ( "
"			  SELECT 0 As ProsWithCons, TP.UserID, Count(TP.PersonID) AS TotalPros, CoordFirst, CoordMiddle, CoordLast FROM  	 "
"			 (SELECT PatientsT.PersonID, Persont.UserID, PatCoordT.First as CoordFirst, PatCoordT.Middle as CoordMiddle, PatCoordT.Last As CoordLast "
"			 FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
"			 LEFT JOIN PersonT PatCoordT ON PersonT.UserID = PatCoordT.ID "
"			 WHERE CurrentStatus = 4  "
"			 [From] [To] [Prov1] [Loc1] [PatFilter] "
"			 UNION  	 "
"			 SELECT PatientsT.PersonID, PersonT.UserID, PatCoordT.First As CoordFirst, PatCoordT.Middle AS CoordMiddle, PatCoordT.Last AS CoordLast "
"			 FROM PatientStatusHistoryT INNER JOIN PatientsT  "
"			 ON PatientStatusHistoryT.PersonID = PatientsT.PersonID INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
"			 LEFT JOIN PersonT PatCoordT ON PersonT.UserID = PatCoordT.ID "
"			  WHERE OldStatus = 4 "
"			 [From] [To] [Prov1] [Loc1] [PatFilter] "
"			  GROUP BY PersonT.UserID, PatientsT.PersonID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last ) TP "
"			 WHERE UserID IS NOT NULL "
"			 Group By Tp.UserID, Tp.CoordFirst, Tp.CoordMiddle, Tp.CoordLast "
"			 UNION "
"			 SELECT Count(ProsWithConsQ.PersonID) AS ProsWithCons, ProsWithConsQ.UserID, 0 As TotalPros, CoordFirst, CoordMiddle, CoordLast FROM ( "
"			  SELECT PatientsT.PersonID, PersonT.UserID, PatCoordT.First As CoordFirst, PatCoordT.Middle AS CoordMiddle, PatCoordT.Last AS CoordLast  "
"			 FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
"			 LEFT JOIN PersonT PatCoordT ON PersonT.UserID = PatCoordT.ID 			 "
"			 WHERE CurrentStatus = 4    "
"			 [From] [To] [Prov1] [Loc1] [PatFilter] "
"			 UNION    "
"			 SELECT PatientsT.PersonID, PersonT.UserID, PatCoordT.First AS CoordFirst, PatCoordT.Middle As CoordMiddle, PatCoordT.Last AS CoordLast "
"			 FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
"			 LEFT JOIN PersonT PatCoordT ON PersonT.UserID = PatCoordT.ID "
"			 WHERE OldStatus = 4 "
"			 [From] [To] [Prov1] [Loc1] [PatFilter] "
"			 ) AS ProsWithConsQ  "
"			 WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
"			 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)  "
"			 AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4 [Loc2] [Prov2] ) AND UserID IS NOT NULL "
"			 GROUP BY UserID, CoordFirst, CoordMiddle, CoordLast "
"			 ) SubQ Group By UserID, CoordFirst, CoordMiddle, CoordLast  ) Q ";
	break;

	/*Prospects To Consults By Coordinator*/
	case COORDProsToCons:
		strSql = "SELECT CAST (ProsWithCons AS FLOAT) * 100.0 AS ProsWithCons, EmployeeID, CAST(TotalPros AS FLOAT) AS TotalPros, "
"			 100.0 * CAST (CASE WHEN TotalPros = 0 THEN 0 ELSE ProsWithCons/TotalPros END AS FLOAT) AS ProsPercent, CoordFirst, CoordMiddle, CoordLast FROM ( "
"			 SELECT CAST (Sum(ProsWithCons) AS FLOAT) AS ProsWithCons, EmployeeID, CAST (Sum(TotalPros) AS FLOAT) AS TotalPros, "
"			 CAST (CASE WHEN Sum(TotalPros) = 0 THEN 0 ELSE SUM(ProsWithCons)/Sum(TotalPros) END AS FLOAT) AS ProsPercent, CoordFirst, CoordMiddle, CoordLast FROM ( "
"			  SELECT 0 As ProsWithCons, TP.EmployeeID, Count(TP.PersonID) AS TotalPros, CoordFirst, CoordMiddle, CoordLast FROM  	 "
"			 (SELECT PatientsT.PersonID, PatientsT.EmployeeID, PatCoordT.First as CoordFirst, PatCoordT.Middle as CoordMiddle, PatCoordT.Last As CoordLast "
"			 FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
"			 LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
"			 WHERE CurrentStatus = 2  "
"			 [From] [To] [Prov1] [Loc1] [PatFilter] "
"			 UNION  	 "
"			 SELECT PatientsT.PersonID, PatientsT.EmployeeID, PatCoordT.First As CoordFirst, PatCoordT.Middle AS CoordMiddle, PatCoordT.Last AS CoordLast "
"			 FROM PatientStatusHistoryT INNER JOIN PatientsT  "
"			 ON PatientStatusHistoryT.PersonID = PatientsT.PersonID INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
"			 LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
"			  WHERE OldStatus = 2 "
"			 [From] [To] [Prov1] [Loc1] [PatFilter] "
"			  GROUP BY PatientsT.EmployeeID, PatientsT.PersonID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last ) TP "
"			 WHERE EmployeeID IS NOT NULL "
"			 Group By Tp.EmployeeID, Tp.CoordFirst, Tp.CoordMiddle, Tp.CoordLast "
"			 UNION "
"			 SELECT Count(ProsWithConsQ.PersonID) AS ProsWithCons, ProsWithConsQ.EmployeeID, 0 As TotalPros, CoordFirst, CoordMiddle, CoordLast FROM ( "
"			  SELECT PatientsT.PersonID, PatientsT.EmployeeID, PatCoordT.First As CoordFirst, PatCoordT.Middle AS CoordMiddle, PatCoordT.Last AS CoordLast  "
"			 FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
"			 LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID 			 "
"			 WHERE CurrentStatus = 2    "
"			 [From] [To] [Prov1] [Loc1] [PatFilter] "
"			 UNION    "
"			 SELECT PatientsT.PersonID, PatientsT.EmployeeID, PatCoordT.First AS CoordFirst, PatCoordT.Middle As CoordMiddle, PatCoordT.Last AS CoordLast "
"			 FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
"			 LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
"			 WHERE OldStatus = 2 "
"			 [From] [To] [Prov1] [Loc1] [PatFilter] "
"			 ) AS ProsWithConsQ  "
"			 WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
"			 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)  "
"			 AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4 [Loc2] [Prov2] ) AND EmployeeID IS NOT NULL "
"			 GROUP BY EmployeeID, CoordFirst, CoordMiddle, CoordLast "
"			 ) SubQ Group By EmployeeID, CoordFirst, CoordMiddle, CoordLast  ) Q ";
		/*strSql = " SELECT CAST (Sum(ProsWithCons) AS FLOAT) AS ProsWithCons, EmployeeID, CAST (Sum(TotalPros) AS FLOAT) AS TotalPros, "
			" CAST (CASE WHEN Sum(TotalPros) = 0 THEN 0 ELSE SUM(ProsWithCons)/Sum(TotalPros) END AS FLOAT) AS ProsPercent FROM ( "
			" SELECT 0 As ProsWithCons, TP.EmployeeID, Count(TP.PersonID) AS TotalPros FROM  	 "
			" (SELECT PatientsT.PersonID, EmployeeID FROM PatientsT  "
			" WHERE CurrentStatus = 2  	UNION  	 "
			" SELECT PatientsT.PersonID, EmployeeID FROM PatientStatusHistoryT INNER JOIN PatientsT  "
			" ON PatientStatusHistoryT.PersonID = PatientsT.PersonID  "
			"  WHERE OldStatus = 2 GROUP BY EmployeeID, PatientsT.PersonID ) TP "
			" WHERE EmployeeID IS NOT NULL Group By Tp.EmployeeID "
			" UNION "
			" SELECT Count(ProsWithConsQ.PersonID) AS ProsWithCons, ProsWithConsQ.EmployeeID, 0 As TotalPros FROM ( "
			"  SELECT PatientsT.PersonID, EmployeeID  "
			"  FROM PatientsT WHERE CurrentStatus = 2   "
			"  UNION    "
			" SELECT PatientsT.PersonID, EmployeeID "
			"  FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID  WHERE OldStatus = 2 ) AS ProsWithConsQ  "
			"  WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
			" 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)  "
			" AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4  ) AND EmployeeID IS NOT NULL "
			" GROUP BY EmployeeID "
			" ) SubQ Group By EmployeeID";*/
	break;

	/*Procedures Performed Vs. Closed By Coordinator*/
	case COORDSchedVClosed:
		//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
		// (j.jones 2010-09-22 14:50) - PLID 34140 - made it so we ignore ProcInfoPaymentsT records if they are fully refunded/adjusted
		strSql = "SELECT CAST(Sum(ProcsClosed) AS FLOAT) AS ProcsClosed, EmployeeID, CAST(Sum(ProcsPerformed) AS FLOAT) AS ProcsPerformed, CoordFirst, CoordMiddle, CoordLast FROM  "
			" (SELECT CAST(Count(AppointmentPurposeT.AppointmentID) AS FLOAT) AS ProcsClosed, EmployeeID, 0 AS ProcsPerformed, PatCoordT.First AS CoordFirst, PatCoordT.Middle AS CoordMiddle, PatCoordT.Last AS CoordLast  "
			"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
			" 	 AppointmentsT.PatientID = PatientsT.PersonID "
			" 	 LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID "
			"    LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
			"	 WHERE (PersonT.ID > 0) AND AppointmentsT.ID IN (SELECT SurgeryApptID FROM ProcInfoT "
			"		INNER JOIN (SELECT * FROM ProcInfoPaymentsT WHERE PayID NOT IN ( "
			"			SELECT LineItemT.ID FROM LineItemT "
			"			INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"			INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID "
			"			GROUP BY LineItemT.ID, LineItemT.Amount "
			"			HAVING LineItemT.Amount = Sum(-AppliesT.Amount))) AS ProcInfoPaymentsT ON ProcInfoT.ID = ProcInfoPaymentsT.ProcInfoID) "
			"	 [From] [To] [Prov1] [Loc1] [PatFilter]  "
			"	 AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3  AND EmployeeID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4) "
			" 	 GROUP BY EmployeeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last "
			" 	 UNION SELECT 0 AS ProcsClosed, EmployeeID, CAST(Count(AppointmentPurposeT.AppointmentID) AS FLOAT) AS ProcsPerformed, PatCoordT.First AS CoordFirst, PatCoordT.Middle AS CoordMiddle, PatCoordT.Last AS CoordLast   "
			" 	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
			"	 AppointmentsT.PatientID = PatientsT.PersonID "
			"	  LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID   "
			"     LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
			"	  WHERE (PersonT.ID > 0) "
			"	  [From] [To] [Prov1] [Loc1] [PatFilter]  "
			"	  AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 2 AND EmployeeID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4) "
			"	  GROUP BY EmployeeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last) SUBQ Group By EmployeeID, CoordFirst, CoordMiddle, CoordLast";
	break;
/****************************************END GRAPHS OF THE COORDINATOR TAB************************************************/

/*****************************************GRAPHS OF THE DATE TAB*************************************************************/

	/*Revenue By Date*/
	case DATERevByDate:

		// (r.gonet 2015-05-05 09:53) - PLID 66305 - Also exclude 10 - GiftCertificateRefunds
		strSql = " SELECT CAST (Sum(SumofAmount) AS FLOAT) as TotalRevenue, TMonthYear AS MonthYear FROM ( "
			" SELECT Sum(PaymentsByDateSubQ.Amount) AS SumOfAmount,    "
			" /*LTRIM(STR(DATEPART(MM, PaymentsByDateSubQ.IDate ))) + LTRIM(STR(DATEPART(YYYY, PaymentsByDateSubQ.IDate ))) AS IMonthYear,*/ "
			" LTRIM(STR(DATEPART(MM, PaymentsByDateSubQ.TDate ))) + LTRIM(STR(DATEPART(YYYY, PaymentsByDateSubQ.TDate ))) AS TMonthYear "
			" FROM   "
				" (SELECT * FROM   "
					" (SELECT Amount = CASE   "
						" WHEN [_PartiallyAppliedPaysQ].[ID] Is Null   "
						" THEN CASE   "
							" WHEN [LineChargesT].[ID] Is Null   "
							" THEN [LinePaymentsT].[Amount]   "
							" ELSE [AppliesT].[Amount]   "
							" End   "
						" ELSE [AppliesT].[Amount]   "
						" End,    "
					" TransProvID = CASE    "
						" WHEN [DoctorsProviders] Is Null   "
						" THEN [PaymentsT].[ProviderID]   "
						" ELSE [DoctorsProviders]   "
						" End,   "
					" PatProvID = PatientsT.MainPhysician, "
					" LinePaymentsT.InputDate AS IDate,    "
					" LinePaymentsT.Date,    "
					" LinePaymentsT.Date  AS TDate,   "
					" CASE WHEN LineChargesT.LocationID Is Null THEN LinePaymentsT.LocationID ELSE LineChargesT.LocationID END AS TransLocID, "
					" PersonT.Location as PatLocID "
					" FROM ((((((LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN   "
						" (SELECT LinePaymentsT.ID, LinePaymentsT.Amount AS PayAmt,    "
						" Sum(AppliesT.Amount) AS ApplyAmt,    "
						" /*First a Amount*/Min([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,    "
						" LinePaymentsT.PatientID    "
						" FROM LineItemT AS LinePaymentsT LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LineChargesT ON AppliesT.DestID = LineChargesT.ID) ON PaymentsT.ID = AppliesT.SourceID)   "
						" ON LinePaymentsT.ID = PaymentsT.ID   "
						" LEFT JOIN PersonT ON LinePaymentsT.PatientID = PersonT.ID "
						" WHERE (((LinePaymentsT.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) [To] [From]  "
						" GROUP BY LinePaymentsT.ID, LinePaymentsT.Amount, LinePaymentsT.PatientID   "
						" HAVING (((LinePaymentsT.ID) is not  Null) AND ((MIN([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))   "
						" ) AS _PartiallyAppliedPaysQ   "
					" ON LinePaymentsT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineChargesT ON AppliesT.DestID = LineChargesT.ID)   "
					" LEFT JOIN ChargesT ON LineChargesT.ID = ChargesT.ID)  "
					" LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
					" LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LinePaymentsT.PatientID = PatientsT.PersonID   "
					" LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID  "
					" WHERE (((PaymentsT.ID) Is Not Null) AND ((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))   "
					" [Resp] [PatFilter] [Category] [From] [To] "
					" ) AS PaymentsByDateFullQ   "
					" WHERE (1=1) [Prov1] [Loc1]   "
				" UNION ALL "
				" SELECT * FROM   "
					" (SELECT [_PartiallyAppliedPaysQ].Total AS Amount,  PaymentsT.ProviderID AS TransProvID, PatientsT.PersonID as PatProvID, "
					" LinePaymentsT.InputDate AS IDate,  LinePaymentsT.Date, LinePaymentsT.Date AS TDate,   "
					" LinePaymentsT.LocationID AS TransLocID, PersonT.Location AS PatLocID   "
					" FROM ((  "
						" (SELECT LinePaymentsT.ID, LinePaymentsT.Amount AS PayAmt,    "
						" Sum(AppliesT.Amount) AS ApplyAmt,  /*First a Amount*/Min([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,    "
						" LinePaymentsT.PatientID    "
						" FROM LineItemT AS LinePaymentsT LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LineChargesT ON AppliesT.DestID = LineChargesT.ID) ON PaymentsT.ID = AppliesT.SourceID)   "
						" ON LinePaymentsT.ID = PaymentsT.ID   "
						" LEFT JOIN PersonT ON LinePaymentsT.PatientID = PersonT.ID "
						" WHERE (((LinePaymentsT.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) [To] [From]  "
						" GROUP BY LinePaymentsT.ID, LinePaymentsT.Amount, LinePaymentsT.PatientID   "
						" HAVING (((LinePaymentsT.ID) is not  Null) AND ((MIN([LinePaymentsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))   "
						" ) AS _PartiallyAppliedPaysQ   "
					" INNER JOIN (LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LinePaymentsT.ID) INNER JOIN PaymentsT ON LinePaymentsT.ID =   "
					" PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LinePaymentsT.PatientID = PatientsT.PersonID   "
					" LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID  "
					" WHERE (((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))   "
					" [Resp] [PatFilter] [Category2] [From4] [To4]    "
					" ) AS PaymentsByDatePartQ  "
					" WHERE (1=1)  [Prov1] [Loc1]  "
				" ) AS PaymentsByDateSubQ   "
			" GROUP BY PaymentsByDateSubQ.IDate,  PaymentsByDateSubQ.TDate "
			" UNION ALL "
			" /*Refunds*/ "
			" SELECT Sum(PaymentsByDateSubQ.Amount) AS SumOfAmount,    "
			" /*LTRIM(STR(DATEPART(MM, PaymentsByDateSubQ.IDate ))) + LTRIM(STR(DATEPART(YYYY, PaymentsByDateSubQ.IDate ))) AS IMonthYear,*/ "
			" LTRIM(STR(DATEPART(MM, PaymentsByDateSubQ.TDate ))) + LTRIM(STR(DATEPART(YYYY, PaymentsByDateSubQ.TDate ))) AS IMonthYear "
			" FROM   "
				" (SELECT * FROM   "
					" (SELECT Amount = CASE   "
						" WHEN [_PartiallyAppliedPaysQ].[ID] Is Null   "
						" THEN CASE   "
							" WHEN [LinePaymentsT].[ID] Is Null   "
							" THEN [LineRefundsT].[Amount]   "
							" ELSE [AppliesT].[Amount]   "
							" End   "
						" ELSE [AppliesT].[Amount]   "
						" End,    "
					" TransProvID = CASE    "
						" WHEN PaymentsT.ProviderID Is Null   "
						" THEN RefundsT.ProviderID "
						" ELSE PaymentsT.ProviderID  "
						" End,   "
					" PatientsT.MainPhysician AS PatProvID, "
					" LineRefundsT.InputDate AS IDate,    "
					" LineRefundsT.Date,   "
					" LineRefundsT.Date AS TDate,   "
					" CASE WHEN LinePaymentsT.LocationID Is Null THEN LineRefundsT.LocationID ELSE LinePaymentsT.LocationID END AS TransLocID, "
					" PersonT.Location AS PatLocID "
					" FROM ((((((LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT RefundsT ON LineRefundsT.ID = RefundsT.ID) LEFT JOIN   "
						" (SELECT LineRefundsT.ID, LineRefundsT.Amount AS PayAmt,    "
						" Sum(AppliesT.Amount) AS ApplyAmt,    "
						" /*First a Amount*/Min([LineRefundsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,    "
						" LineRefundsT.PatientID    "
						" FROM LineItemT AS LineRefundsT LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID) ON PaymentsT.ID = AppliesT.SourceID)   "
						" ON LineRefundsT.ID = PaymentsT.ID   "
						" LEFT JOIN PersonT ON LineRefundsT.PatientID = PersonT.ID "
						" WHERE (((LineRefundsT.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10)) [To2] [From2]  "
						" GROUP BY LineRefundsT.ID, LineRefundsT.Amount, LineRefundsT.PatientID   "
						" HAVING (((LineRefundsT.ID) is not  Null) AND ((MIN([LineRefundsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))   "
						" ) AS _PartiallyAppliedPaysQ   "
					" ON LineRefundsT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON RefundsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID)   "
					" LEFT JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineRefundsT.PatientID = PatientsT.PersonID   "
					" LEFT JOIN PaymentPlansT ON RefundsT.ID = PaymentPlansT.ID  "
					" WHERE (((RefundsT.ID) Is Not Null) AND ((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (RefundsT.PayMethod NOT IN (4,10))   "
					"  [Resp2] [PatFilter] [Category2] [From2] [To2] [From3] [To3] "
					" ) AS PaymentsByDateFullQ   "
					"  WHERE (1=1)  [Prov1] [Loc1]   "
				" UNION ALL "
				" SELECT * FROM   "
					" (SELECT [_PartiallyAppliedPaysQ].Total AS Amount,  RefundsT.ProviderID AS TransProvID,  PatientsT.MainPhysician as PatProvID,  "
					" LineRefundsT.InputDate AS IDate, LineRefundsT.Date, LineRefundsT.Date AS TDate,   "
					" LineRefundsT.LocationID AS TransLocID, PersonT.Location AS PatLocID   "
					" FROM ((  "
						" (SELECT LineRefundsT.ID, LineRefundsT.Amount AS PayAmt,    "
						" Sum(AppliesT.Amount) AS ApplyAmt,  /*First a Amount*/Min([LineRefundsT].[Amount])-Sum([AppliesT].[Amount]) AS Total,    "
						" LineRefundsT.PatientID    "
						" FROM LineItemT AS LineRefundsT LEFT JOIN (PaymentsT RefsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID) ON RefsT.ID = AppliesT.SourceID)   "
						" ON LineRefundsT.ID = RefsT.ID   "
						" LEFT JOIN PersonT ON LineRefundsT.PatientID = PersonT.ID "
						" WHERE (((LineRefundsT.Deleted)=0)) AND (RefsT.PayMethod NOT IN (4,10)) [To2] [From2]  "
						" GROUP BY LineRefundsT.ID, LineRefundsT.Amount, LineRefundsT.PatientID   "
						" HAVING (((LineRefundsT.ID) is not  Null) AND ((MIN([LineRefundsT].[Amount])-Sum([AppliesT].[Amount])) <> 0))   "
						" ) AS _PartiallyAppliedPaysQ   "
					" INNER JOIN (LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineRefundsT.ID) INNER JOIN PaymentsT RefundsT ON LineRefundsT.ID =   "
					" RefundsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineRefundsT.PatientID = PatientsT.PersonID   "
					" LEFT JOIN PaymentPlansT ON RefundsT.ID = PaymentPlansT.ID  "
					" WHERE (((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (RefundsT.PayMethod NOT IN (4,10))   "
					"  [Resp2] [PatFilter] [Category2] [From2] [To2] [From3] [To3]"
					" ) AS PaymentsByDatePartQ  "
					"  WHERE (1=1)  [Prov1] [Loc1]   "
				" ) AS PaymentsByDateSubQ   "
			" GROUP BY PaymentsByDateSubQ.IDate,  PaymentsByDateSubQ.TDate) "
			" SubQ "
			" Group By TMonthYear"; 
	 break; 

	
	/*Conversion Rate By Date*/
	 //(e.lally 2009-09-01) PLID 35301 - Rewrote the query to use the new base query
	 //(e.lally 2009-09-01) PLID 35429 - Updated to use the consult splitting preference
	 //(e.lally 2009-09-08) PLID 35301 - Split the graph details and summary because the summary can't group without
		//possibly double-counting
    case CNVConstoSurgByDate:
		if(nFlags == 1){
			strSql = 
			"SELECT LTRIM(STR(DATEPART(MM, ConsultDate))) + LTRIM(STR(DATEPART(YYYY, ConsultDate))) AS MonthYear, \r\n" +
				GetConsultToProcedureGraphStatFields() +
			"FROM (\r\n" + GetConsultToProcedureConversionRateBaseSql() + "\r\n) ConversionQ " +
					"GROUP BY LTRIM(STR(DATEPART(MM, ConsultDate))) + LTRIM(STR(DATEPART(YYYY, ConsultDate))) /* as MonthYear*/ ";
		}
		//We can't use the group by here in case the consult was for multiple procedures and the surgeries were in different months,
			//it would duplicate our consult count numbers
		else if(nFlags == 2){
			strSql = 
			"SELECT " +
				GetConsultToProcedureGraphStatFields() +
			"FROM (\r\n" + GetConsultToProcedureConversionRateBaseSql() + "\r\n) ConversionQ ";
		}
		//(e.lally 2009-09-01) PLID 35301 - Replace the filter placeholders with the actual filters selected before returning our query string.
		ApplyDocbarGraphFilters(strSql, mktGraph, pCon, strPatientTempTable);
	break;

	/*Patients By Date*/
	case DATEPatients:
		strSql = "  SELECT CAST(Sum(PatCount) AS FLOAT) AS PatCount, MonthYear, CAST(Sum(InqCount) AS FLOAT) AS InqCount, CAST(Sum(ProsCount) AS FLOAT) AS ProsCount FROM (SELECT CAST (Count(PersonID) AS FLOAT) AS PatCount, LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear, 0 As InqCount, 0 AS ProsCount FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID   WHERE CurrentStatus = 1 [From] [To] [Prov1] [Loc1] [PatFilter] GROUP BY LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) "
			" UNION SELECT 0 AS PatCount, LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear, 0 As InqCount, CAST (Count(PersonID) AS FLOAT) AS ProsCount FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID   WHERE CurrentStatus = 2 [From] [To] [Prov1] [Loc1] [PatFilter] GROUP BY LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) "	
			" UNION  SELECT 0 AS PatCount, LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear, CAST(Count(PersonID) AS FLOAT)  As InqCount, 0 As ProsCount FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
			" WHERE CurrentStatus = 4 [From] [To] [Prov1] [Loc1] [PatFilter] GROUP BY LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) ) SubQ Group By MonthYear  ";
	break;

	/*Cancellations and No Shows By Date*/
	case DATENoShowCancel:
		strSql = "SELECT CAST (Sum(NumCons) AS FLOAT) AS NumCons,MonthYear, CAST (Sum(NumNoShows) AS FLOAT) AS NumNoShows FROM "
		" (SELECT CAST (Count(AppointmentsT.ID) AS FLOAT) AS NumCons, LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) AS MonthYear, 0 AS NumNoShows  "
		"  FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" WHERE AppointmentsT.Status = 4   "
		" [From] [To] [Prov1] [Loc1] [PatFilter] [Loc2] [Prov2] " 
		" Group By LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) "
		"  UNION SELECT 0 AS NumCans, LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) AS MonthYear, CAST (Count(AppointmentsT.ID) AS FLOAT) AS NumNoShows  "
		" FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" WHERE AppointmentsT.Status <> 4  AND AppointmentsT.ShowState = 3  "
		" [From] [To] [Prov1] [Loc1] [PatFilter] [Loc2] [Prov2] " 
		" Group By LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) ) Sub Group By MonthYear"; 
	break;

	/*Inquiries To Consult By Date*/
	case DATEInqToCons:
			strSql = " SELECT (ProsWithCons * 100.0) AS ProsWithCons, TotalPros, MonthYear, 100.0 * CAST (CASE WHEN TotalPros = 0 THEN 0 ELSE ProsWithCons/TotalPros END AS FLOAT) AS ProsPercent FROM ( "
			" SELECT CAST (Sum(ProsWithCons) AS FLOAT) AS ProsWithCons, MonthYear, CAST (Sum(TotalPros) AS FLOAT) AS TotalPros, "
			" CAST (CASE WHEN Sum(TotalPros) = 0 THEN 0 ELSE SUM(ProsWithCons)/Sum(TotalPros) END AS FLOAT) AS ProsPercent FROM ( "
			"  SELECT 0 As ProsWithCons, TP.MonthYear, Count(TP.PersonID) AS TotalPros FROM  	 "
			" (SELECT PatientsT.PersonID, LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear  FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			" WHERE CurrentStatus = 4  [From] [To] [Prov1] [Loc1] [PatFilter]	UNION  	 "
			" SELECT PatientsT.PersonID, LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear   FROM PatientStatusHistoryT INNER JOIN PatientsT  "
			" ON PatientStatusHistoryT.PersonID = PatientsT.PersonID  INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"  WHERE OldStatus = 4  [From] [To] [Prov1] [Loc1] [PatFilter] "
			"  GROUP BY LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))), PatientsT.PersonID ) TP "
			" Group By Tp.MonthYear "
			" UNION "
			" SELECT Count(ProsWithConsQ.PersonID) AS ProsWithCons, ProsWithConsQ.MonthYear, 0 As TotalPros FROM ( "
			"  SELECT PatientsT.PersonID, LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear    "
			" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE CurrentStatus = 4  [From] [To] [Prov1] [Loc1] [PatFilter] "
			" UNION    "
			" SELECT PatientsT.PersonID, LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear   "
			" FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID  INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"  WHERE OldStatus = 4  [From] [To] [Prov1] [Loc1] [PatFilter] ) AS ProsWithConsQ  "
			" WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
			" 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)  "
			"  AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4 [Loc2] [Prov2] )  "
			"  GROUP BY MonthYear "
			" ) SubQ Group By MonthYear) Q ";
	break;

	/*Prospects To Consults By Date*/
	case DATEProsToCons:
		strSql = "SELECT (ProsWithCons * 100.0) AS ProsWithCons, TotalPros, MonthYear, CAST (CASE WHEN TotalPros = 0 THEN 0 ELSE 100.0 * ((ProsWithCons)/(TotalPros)) END AS FLOAT) AS ProsPercent FROM ( "
			" SELECT CAST (Sum(ProsWithCons) AS FLOAT) AS ProsWithCons, MonthYear, CAST (Sum(TotalPros) AS FLOAT) AS TotalPros, "
			" CAST (CASE WHEN Sum(TotalPros) = 0 THEN 0 ELSE SUM(ProsWithCons)/Sum(TotalPros) END AS FLOAT) AS ProsPercent FROM ( "
			"  SELECT 0 As ProsWithCons, TP.MonthYear, Count(TP.PersonID) AS TotalPros FROM  	 "
			" (SELECT PatientsT.PersonID, LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear  FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			" WHERE CurrentStatus = 2  	"
			" [From] [To] [Prov1] [Loc1] [PatFilter] "
			"  UNION  	 "
			" SELECT PatientsT.PersonID, LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear   FROM PatientStatusHistoryT INNER JOIN PatientsT  "
			" ON PatientStatusHistoryT.PersonID = PatientsT.PersonID  INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"  WHERE OldStatus = 2 "
			" [From] [To] [Prov1] [Loc1] [PatFilter] " 
			" GROUP BY LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))), PatientsT.PersonID ) TP "
			" Group By Tp.MonthYear "
			" UNION "
			" SELECT Count(ProsWithConsQ.PersonID) AS ProsWithCons, ProsWithConsQ.MonthYear, 0 As TotalPros FROM ( "
			"  SELECT PatientsT.PersonID, LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear    "
			" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE CurrentStatus = 2   "
			" [From] [To] [Prov1] [Loc1] [PatFilter] "
			" UNION    "
			" SELECT PatientsT.PersonID, LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear   "
			" FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID  INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"  WHERE OldStatus = 2 "
			" [From] [To] [Prov1] [Loc1] [PatFilter] "
			") AS ProsWithConsQ  "
			" WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
			" 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)  "
			"  AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4 [Loc2] [Prov2] )  "
			"  GROUP BY MonthYear "
			" ) SubQ Group By MonthYear) Q Group By MonthYear, TotalPros, ProsWithCons ";
	break;


	/*Procedures Performed Vs. Closed By Date*/
	case DATECloseByProc:
		//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
		// (j.jones 2010-09-22 14:50) - PLID 34140 - made it so we ignore ProcInfoPaymentsT records if they are fully refunded/adjusted
		strSql = "SELECT CAST(Sum(ProcsClosed) AS FLOAT) AS ProcsClosed, MonthYear, CAST(Sum(ProcsPerformed) AS FLOAT) AS ProcsPerformed FROM (SELECT CAST(Count(AppointmentPurposeT.AppointmentID) AS FLOAT) AS ProcsClosed, LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) AS MonthYear, 0 AS ProcsPerformed "
			" FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			" LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON "
			" AppointmentsT.PatientID = PatientsT.PersonID "
			" LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID "
			" WHERE (PersonT.ID > 0) AND AppointmentsT.ID IN (SELECT SurgeryApptID FROM ProcInfoT "
			"	INNER JOIN (SELECT * FROM ProcInfoPaymentsT WHERE PayID NOT IN ( "
			"		SELECT LineItemT.ID FROM LineItemT "
			"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"		INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID "
			"		GROUP BY LineItemT.ID, LineItemT.Amount "
			"		HAVING LineItemT.Amount = Sum(-AppliesT.Amount))) AS ProcInfoPaymentsT ON ProcInfoT.ID = ProcInfoPaymentsT.ProcInfoID) AND "
			" AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3  AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4) "
			" [From] [To] [Prov1] [Loc1] [PatFilter] [Loc2] [Prov2] "
			" GROUP BY LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) ";
	
		strSql += " UNION SELECT 0 AS ProcsClosed, LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) AS MonthYear, CAST(Count(AppointmentPurposeT.AppointmentID) AS FLOAT) AS ProcsPerformed  "
			" FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			" LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON "
			" AppointmentsT.PatientID = PatientsT.PersonID "
			"  LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID  "
			"  WHERE (PersonT.ID > 0) AND "
			"  AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 2 AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4) "
			" [From] [To] [Prov1] [Loc1] [PatFilter] [Loc2] [Prov2] "
			"  GROUP BY LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date)))) SUBQ Group By MonthYear ";
	break;

	// (j.gruber 2011-05-03 16:37) - PLID 38153
	/*Appointments To Charges by Date */
	case DATEApptToCharge:	
		return " SELECT NumAppts, NumCharges, CASE WHEN NumAppts = 0 THEN CAST(0 as FLOAT) ELSE CAST((NumCharges/NumAppts) * 100 as FLOAT) END as ConvRate, MonthYear FROM  "
			" ( "
			" SELECT CAST(Sum(NumAppts) AS FLOAT) AS NumAppts, MonthYear, CAST(Sum(NumCharges) AS FLOAT) AS NumCharges FROM  "
			" ( "
			" (SELECT CAST (Count(AppointmentsT.ID) AS FLOAT) AS NumAppts,  "
			" LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) AS MonthYear, "
			" 0 as NumCharges "
			" FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
			" WHERE AppointmentsT.Status <> 4 and AppointmentsT.ShowState <> 3 "
			" AND AptTypeID IN (SELECT ApptTypeID FROM ApptServiceConvTypesT WHERE (1=1) [GroupFilterTypes]) "
			" [From] [To] [Prov1] [Loc1] [PatFilter] [Loc2] [Prov2]  "
			" Group By LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) "
			" ) "
			" UNION ALL "
			" (SELECT CAST(0 as FLOAT),  LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) AS MonthYear, "
			" CAST(Count(ID) AS FLOAT) "
			" FROM AppointmentsT WHERE ID IN( "
			" SELECT AppointmentsT.ID "
			" FROM AppointmentsT  "
			" LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID  "
			" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
			" INNER JOIN ApptServiceConvTypesT ON AppointmentsT.AptTypeID = ApptServiceConvTypesT.ApptTypeID "
			" INNER JOIN ApptServiceConvGroupsT ON ApptServiceConvTypesT.GroupID = ApptServiceConvGroupsT.ID "
			" LEFT JOIN ApptServiceConvServicesT ON ApptServiceConvGroupsT.ID =  ApptServiceConvServicesT.GroupID "
			" LEFT JOIN ChargesT ON ApptServiceConvServicesT.ServiceID = ChargesT.ServiceID "
			" LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			" WHERE AppointmentsT.Status <> 4 and AppointmentsT.ShowState <> 3 "
			" AND LineItemT.Deleted = 0 "
			" AND AppointmentsT.PatientID = LineItemT.PatientID "
			" [From] [To] [Prov1] [Loc1] [PatFilter] [Loc2] [Prov2] [GroupFilter] "
			"  AND 1 = (CASE WHEN ApptServiceConvGroupsT.ConversionDayLimit = 0 then CASE WHEN DateDiff(day, AppointmentsT.Date, LineItemT.Date) >= 0 THEN 1 ELSE 0 END ELSE CASE WHEN DateDiff(day, AppointmentsT.Date, LineItemT.Date) >= 0 AND DateDiff(day, AppointmentsT.Date, LineItemT.Date) <= ApptServiceConvGroupsT.ConversionDayLimit THEN 1 ELSE 0 END END)	 "
			" GROUP BY AppointmentsT.ID)  "
			" GROUP BY LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) "
			" ) "
			" )Q GROUP BY MonthYear  "
			" ) TotalQ ";
		break;

/**********************************************END GRAPHS OF THE DATE TAB*******************************************************/















	case EFFProfByProc:
	break;

	case EFFProfByPatCoord:
	break;

	
	case EFFRefEffect:
	break;

	
	
	case NUMPatNoShowsByProc:
		
		//nFlag is whether to show only completed appts or not
		strSql = " SELECT CAST(Count(PatientID) AS FLOAT) AS PatCount, PurposeID FROM  "
			" (SELECT PatientID, AppointmentPurposeT.PurposeID "
			" FROM AppointmentsT LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
			" LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			" WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 3 AND AppointmentPurposeT.PurposeID IS NOT NULL ";

		if (nFlags == 1) {
			//strSql += " AND AppointmentsT.StartTime <= GetDate() ";
		}

		strSql += " [From] [To] [Prov] [Loc] [PatFilter] "
			" Group By PatientID, AppointmentPurposeT.PurposeID) SubQ "
			" Group By PurposeID ";
	break;

	case NUMInqByProc:
 
		strSql = " SELECT CAST(Count(PersonID) AS FLOAT) AS NumCount, ProcInfoDetailsT.ProcedureID AS PurposeID FROM "
				" PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				" LEFT JOIN ProcInfoT ON PatientsT.PersonID = ProcInfoT.PatientID LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID "
				" LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID  "
				" WHERE PatientsT.CurrentStatus = 4 "
				" [From] [To] [Prov] [Loc]  "
				" GROUP BY ProcedureID ";
	break;

	
	
	case NUMCanByProc:
		strSql = "SELECT CAST (Count(AppointmentsT.ID) AS FLOAT) AS NumCancelled, AppointmentPurposeT.PurposeID "
			" FROM AppointmentsT LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
			" LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			" WHERE AppointmentsT.Status = 4  AND AppointmentPurposeT.PurposeID IS NOT NULL ";
		if (nFlags ==1) {
			//strSql += " AND AppointmentsT.StartTime <= GetDate() ";
		}

		strSql += " [From] [To] [Prov] [Loc] [PatFilter] "
			" Group By AppointmentPurposeT.PurposeID ";
	break;

	case NUMCanByReason:

		strSql = "SELECT CAST (Count(AppointmentsT.ID)AS FLOAT) AS NumCancel, CASE WHEN AppointmentsT.CancelReasonID IS NULL THEN -1 ELSE AppointmentsT.CancelReasonID END AS ReasonID"
			" FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			" WHERE AppointmentsT.Status =  4 ";
		if (nFlags ==1) {
			//strSql += " AND AppointmentsT.StartTime <= GetDate() ";
		}

		strSql += " [From] [To] [Prov1] [Loc1] [PatFilter] "
			" Group By AppointmentsT.CancelReasonID ";
	break;

	case NUMNoShowByReason:

		strSql = "SELECT CAST (Count(AppointmentsT.ID)AS FLOAT) AS NumNoShow, CASE WHEN AppointmentsT.NoshowReasonID IS NULL THEN -1 ELSE AppointmentsT.NoShowReasonID END AS ReasonID"
			" FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			" WHERE AppointmentsT.Status <>  4  AND ShowState = 3 ";
		if (nFlags ==1) {
			//strSql += " AND AppointmentsT.StartTime <= GetDate() ";
		}

		strSql += " [From] [To] [Prov1] [Loc1] [PatFilter] "
			" Group By AppointmentsT.NoShowReasonID ";
	break;

	case NUMConWithProsByPatCoord:
	break;

	case NUMConsByPatCoord:

		strSql = "SELECT CAST(Count(AppointmentsT.ID) AS FLOAT) AS NumCons, PatientsT.EmployeeID "
			" FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"  WHERE AppointmentsT.Status <> 4  AND PatientsT.EmployeeID IS NOT NULL "
			" AND AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) ";

			if (nFlags ==1) {
				strSql += " AND AppointmentsT.StartTime <= GetDate() ";
			}

			strSql += " [From] [To] [Prov] [Loc] [PatFilter] "
				" Group By PatientsT.EmployeeID ";


	break;

	
	case DATECountByProcs:
		strSql = " SELECT CAST(Count(AppointmentsT.ID) AS FLOAT) AS NumProcs, AppointmentPurposeT.PurposeID  "
			"  FROM AppointmentsT LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  "
			" LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
			" WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND AppointmentPurposeT.PurposeID IS NOT NULL "
			" AND AppointmentsT.StartTime <= GetDate()  "
			" [From] [To] [Prov] [Loc] [PatFilter]  "
			" Group By AppointmentPurposeT.PurposeID ";
	break;

	case REFInqByReferral:
		strSql = "SELECT CAST(Count(PatientsT.PersonID) AS FLOAT) AS NumInquiries, ReferralID FROM PatientsT "
			" WHERE CurrentStatus = 4 AND (ReferralID IS NOT NULL) AND (ReferralID <> -1) "
			" GROUP BY ReferralID ";

	break;

	
	case INTERNAL_IncPerCat:
		if(nFilter == -1) {
			//If the filter is -1, we want to show all items which have no parent (things just labeled as 'Upgrade', 'Question', etc., but we also
			//	want to add in their children - 'Question - Reports', 'Question - Billing', etc - these children need to show as part of the above.
			strSql = 
				"SELECT CASE WHEN CategoryID IS NULL THEN -1 ELSE CategoryID END AS CategoryID, "
				"convert(float, COUNT(DurationSubQ.ID)) AS IncidentCount, convert(float, SUM(DurationSubQ.IncidentDuration)) AS CategoryDuration "
				"FROM "
				"	(/*All the incidents which have a no parent (no subcategory)*/ "
				"	SELECT IssueT.ID, IssueT.CategoryID,  "
				"	SUM(convert(float, (DATEDIFF(ss, IssueSlipT.StartTime, IssueSlipT.EndTime) / 60.0) / 60.0)) AS IncidentDuration "
				"	FROM IssueT "
				"	LEFT JOIN IssueCategoryT ON IssueT.CategoryID = IssueCategoryT.ID "
				"	LEFT JOIN IssueSlipT ON IssueT.ID = IssueSlipT.IssueID "
				"	WHERE IssueCategoryT.ParentID IS NULL [from] [to]  "
				"	GROUP BY IssueT.ID, IssueT.CategoryID "
				""
				"	UNION ALL "
				"	/*All the incidents which have subcategories -- display as if we are the parent of this subcategory*/ "
				"	SELECT IssueT.ID, IssueCategoryT.ParentID AS CategoryID,  "
				"	SUM(convert(float, (DATEDIFF(ss, IssueSlipT.StartTime, IssueSlipT.EndTime) / 60.0) / 60.0)) AS IncidentDuration "
				"	FROM IssueT "
				"	LEFT JOIN IssueCategoryT ON IssueT.CategoryID = IssueCategoryT.ID "
				"	LEFT JOIN IssueSlipT ON IssueT.ID = IssueSlipT.IssueID "
				"	WHERE IssueCategoryT.ParentID IS NOT NULL [from] [to] "
				"	GROUP BY IssueT.ID, IssueCategoryT.ParentID "
				"	) DurationSubQ "
				"GROUP BY CategoryID";
		}
		else {
			//If the filter is not -1, then we're filtering on a specific category (like 'Question').  In this case, we want to see all
			//	the children of this item (things like 'Question - Reports' and 'Question - Billing').  We also need to show the items that don't
			//	have a child categorized (the category field will be the ID of the parent category we're filtering on.
			strSql.Format(
				"SELECT CASE WHEN CategoryID IS NULL THEN -1 ELSE CategoryID END AS CategoryID, "
				"convert(float, COUNT(DurationSubQ.ID)) AS IncidentCount, convert(float, SUM(DurationSubQ.IncidentDuration)) AS CategoryDuration "
				"FROM "
				"	(/*All the incidents which have a parent of the filter*/ "
				"	SELECT IssueT.ID, IssueT.CategoryID,  "
				"	SUM(convert(float, (DATEDIFF(ss, IssueSlipT.StartTime, IssueSlipT.EndTime) / 60.0) / 60.0)) AS IncidentDuration "
				"	FROM IssueT "
				"	LEFT JOIN IssueCategoryT ON IssueT.CategoryID = IssueCategoryT.ID "
				"	LEFT JOIN IssueSlipT ON IssueT.ID = IssueSlipT.IssueID "
				"	WHERE IssueCategoryT.ParentID = %li [from] [to] "
				"	GROUP BY IssueT.ID, IssueT.CategoryID "
				""
				"	UNION ALL "
				"	/*All the incidents which have a categoryID of the filter (these have no subcategory)*/ "
				"	SELECT IssueT.ID, -1 AS CategoryID,  "
				"	SUM(convert(float, (DATEDIFF(ss, IssueSlipT.StartTime, IssueSlipT.EndTime) / 60.0) / 60.0)) AS IncidentDuration "
				"	FROM IssueT "
				"	LEFT JOIN IssueCategoryT ON IssueT.CategoryID = IssueCategoryT.ID "
				"	LEFT JOIN IssueSlipT ON IssueT.ID = IssueSlipT.IssueID "
				"	WHERE IssueCategoryT.ID = %li [from] [to] "
				"	GROUP BY IssueT.ID, IssueT.CategoryID "
				"	) DurationSubQ "
				"GROUP BY CategoryID", 
			nFilter, nFilter);
		}
		break;

	case INTERNAL_IncPerPerson:
		if(nFilter == -1) {
			//no category filter - this is the same graph as the per category
			strSql.Format("SELECT CASE WHEN CategoryID IS NULL THEN 0 ELSE CategoryID END AS CategoryID, "
				"convert(float, COUNT(DurationSubQ.ID)) AS IncidentCount, convert(float, SUM(DurationSubQ.IncidentDuration)) AS TotalDuration, "
				"convert(float, convert(float, SUM(DurationSubQ.IncidentDuration)) / convert(float, COUNT(DurationSubQ.ID))) * 60.0 AS AvgDuration "
				"FROM "
				"	(/*All the incidents which have a no parent (no subcategory)*/ "
				"	SELECT IssueT.ID, IssueT.CategoryID,  "
				"	SUM(convert(float, (DATEDIFF(ss, IssueSlipT.StartTime, IssueSlipT.EndTime) / 60.0) / 60.0)) AS IncidentDuration "
				"	FROM IssueT "
				"	LEFT JOIN IssueCategoryT ON IssueT.CategoryID = IssueCategoryT.ID "
				"	LEFT JOIN IssueSlipT ON IssueT.ID = IssueSlipT.IssueID "
				"	WHERE IssueCategoryT.ParentID IS NULL [from] [to]  "
				"	GROUP BY IssueT.ID, IssueT.CategoryID "
				""
				"	UNION ALL "
				"	/*All the incidents which have subcategories -- display as if we are the parent of this subcategory*/ "
				"	SELECT IssueT.ID, IssueCategoryT.ParentID AS CategoryID,  "
				"	SUM(convert(float, (DATEDIFF(ss, IssueSlipT.StartTime, IssueSlipT.EndTime) / 60.0) / 60.0)) AS IncidentDuration "
				"	FROM IssueT "
				"	LEFT JOIN IssueCategoryT ON IssueT.CategoryID = IssueCategoryT.ID "
				"	LEFT JOIN IssueSlipT ON IssueT.ID = IssueSlipT.IssueID "
				"	WHERE IssueCategoryT.ParentID IS NOT NULL [from] [to] "
				"	GROUP BY IssueT.ID, IssueCategoryT.ParentID "
				"	) DurationSubQ "
				"GROUP BY CategoryID");
		}
		else {
			//Specific category filter -- we're filtering on a specific category (like 'Question', or 'Install'), and will return a recordset
			//	filled with each user, how many they've finished and total hours.
			//The dates are filtered on the finished date, not the detail dates.
			CString strCategoryFilter;
			if(nFilter == 0) {
				//uncategorized items
				strCategoryFilter.Format("(IssueT.CategoryID IS NULL OR IssueCategoryT.ParentID IS NULL)");
			}
			else {
				//categorized items of a specific variety
				strCategoryFilter.Format("(IssueT.CategoryID = %li OR IssueCategoryT.ParentID = %li)", nFilter, nFilter);
			}

			strSql.Format(
				"/*Make a table to hold all incidents finished, the person who finished them, and the date -- for multiple finishes, the "
				"	last person and date will be used*/ "
				"DECLARE @FinishedIncidentsTable TABLE (IssueID int NOT NULL PRIMARY KEY, UserID int NOT NULL, FinishedDate datetime NOT NULL); "
				" "
				"SET NOCOUNT ON; "
				"INSERT INTO @FinishedIncidentsTable  "
				"SELECT NoteQ.IssueID, Notes.UserID, IssueT.FinishedDate "
				"FROM "
				"	(/*This query selects each incident and the last note that was 'changed from x to finished' filtering on the finished date of the incident */ "
				"	SELECT IssueT.ID AS IssueID, CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, Notes.Date, 121) + ' ' + CONVERT(nvarchar, Notes.ID)), 25, 80)) AS LastNoteID "
				"	FROM  "
				"	IssueT   "
				"	LEFT JOIN IssueDetailsT ON IssueT.ID = IssueDetailsT.IssueID  "
				"	LEFT JOIN Notes ON IssueDetailsT.NoteID = Notes.ID  "
				"	WHERE Notes.Note LIKE 'Status changed from ''%%'' to ''Finished'''  AND FinishedDate IS NOT NULL "
				"	[from] [to]  "
				"	GROUP BY IssueT.ID "
				"	) NoteQ "
				"LEFT JOIN Notes ON NoteQ.LastNoteID = Notes.ID "
				"LEFT JOIN IssueT ON NoteQ.IssueID = IssueT.ID "
				"LEFT JOIN IssueCategoryT ON IssueT.CategoryID = IssueCategoryT.ID "
				"WHERE %s; "
				"SET NOCOUNT OFF; "
				" "
				"/* "
				"  This query will give us, for each category and user, the total duration and total number of incidents, as well "
				"  as the average minutes per finished "
				"*/ "
				"SELECT CASE WHEN CategoryID IS NULL THEN 0 ELSE CategoryID END AS CategoryID, CASE WHEN UserID IS NULL THEN -25 ELSE UserID END AS FinishedByUserID, convert(float, SUM(IncidentDuration)) AS TotalDuration,   "
				"convert(float, SUM(FinishedByMe)) AS IncidentCount,   "
				"CASE WHEN SUM(FinishedByMe) = 0 THEN 0 ELSE convert(float, convert(float, SUM(IncidentDuration)) / convert(float, SUM(FinishedByMe))) * 60.0 END AS AvgDuration   "
				" "
				"FROM  "
				"	/*  "
				"	  Select how much time was spent on each incident that has been finished in this time period and whether that person  "
				"	  closed it or not  "
				"	*/  "
				"	(SELECT FinIncTbl.IssueID, UsersT.PersonID AS UserID,  "
				"	/*CASE WHEN IssueT.CategoryID IS NULL THEN 0 ELSE IssueT.CategoryID END AS CategoryID, */ "
				"	CASE WHEN IssueCategoryT.ParentID IS NULL THEN IssueT.CategoryID ELSE IssueCategoryT.ParentID END AS CategoryID, "
				"	SUM(convert(float, (DATEDIFF(ss, IssueSlipT.StartTime, IssueSlipT.EndTime) / 60.0) / 60.0)) AS IncidentDuration,  "
				"	CASE WHEN FinIncTbl.UserID = UsersT.PersonID THEN 1 ELSE 0 END AS FinishedByMe  "
				"	FROM @FinishedIncidentsTable FinIncTbl  "
				"	LEFT JOIN IssueSlipT ON FinIncTbl.IssueID = IssueSlipT.IssueID  "
				"	LEFT JOIN UsersT ON IssueSlipT.Owner = UsersT.UserName  "
				"	LEFT JOIN IssueT ON FinIncTbl.IssueID = IssueT.ID  "
				"	LEFT JOIN IssueCategoryT ON IssueT.CategoryID = IssueCategoryT.ID "
				"	GROUP BY FinIncTbl.IssueID, UsersT.PersonID, CASE WHEN FinIncTbl.UserID = UsersT.PersonID THEN 1 ELSE 0 END,  "
				"	CASE WHEN IssueCategoryT.ParentID IS NULL THEN IssueT.CategoryID ELSE IssueCategoryT.ParentID END "
				"	) SubQ  "
				"  "
				"GROUP BY CategoryID, UserID", strCategoryFilter);
		}
		break;

	case INTERNAL_OpenPerWeek:
		//Note that this query uses a cursor (actually 3), I could not come up with a way to calculate this in 1 query.  Here's how it 
		//	works:
		// - cur:  Loop over all dates that are friday (end of each week).
		// - c2:  count all appts which have an EnteredDate before (1 + current date we're investigating) -- This gives us a count of how many
		//	incidents were entered before the saturday of this week.
		// - c3:  count all appts which have a FinishedDate before (1 + current date we're investigating) -- This gives us a count of how many
		//	incidents were finished before saturday of this week.
		//We then subtract entered - finished, and we've got a count of how many were open at the end of this week!
		//The flaw here:  It's hardcoded to Friday, if no incidents are entered on a given friday (holiday), then that whole week will be left out.
		//Also note that the "ModifiedDate is not null" filter is to avoid bad data -- often (there are a lot) incidents get "stuck" in creation
		//	and therefore have an entereddate, but no finished date, and no notes have happened.

		strSql.Format("DECLARE @TableCounts TABLE (AtDate datetime, CountEntered int, CountFinished int); "
			" "
			"DECLARE @curdate datetime; "
			"DECLARE @e_cnt int, @f_cnt int; "
			" "
			"DECLARE cur CURSOR FOR "
			" "
			"(SELECT convert(datetime, convert(nvarchar, EnteredDate, 101)) dt "
			"FROM IssueT "
			"WHERE DATENAME(dw, EnteredDate) = 'Friday' "
			"[from] "
			"[to] "
			"GROUP BY convert(datetime, convert(nvarchar, EnteredDate, 101)) "
			") "
			" "
			"OPEN cur "
			" "
			"FETCH NEXT FROM cur "
			"INTO @curdate "
			" "
			"WHILE @@FETCH_STATUS = 0 "
			"BEGIN "
			" "
			"   DECLARE c2 CURSOR FOR  "
			"   (SELECT COUNT(ID) col1 FROM IssueT WHERE EnteredDate < DATEADD(day, 1, @curdate) AND ModifiedDate IS NOT NULL) "
			" "
			"   open c2 "
			"   FETCH NEXT FROM c2 "
			"   INTO @e_cnt "
			" "
			"   CLOSE c2 "
			"   DEALLOCATE c2 "
			" "
			"   DECLARE c3 CURSOR FOR "
			"   (SELECT COUNT(ID) col1 FROM IssueT WHERE FinishedDate < DATEADD(day, 1, @curdate) AND ModifiedDate IS NOT NULL) "
			" "
			"   OPEN C3 "
			"   FETCH NEXT FROM c3 "
			"   INTO @f_cnt "
			" "
			"   CLOSE c3 "
			"   DEALLOCATE c3 "
			" "
			"	SET NOCOUNT ON; "
			"   INSERT INTO @TableCounts values (@curdate, @e_cnt, @f_cnt); "
			"	SET NOCOUNT OFF; "
			"    "
			"   FETCH NEXT FROM cur "
			"   into @curdate "
			"END "
			" "
			"CLOSE cur "
			"DEALLOCATE cur "
			" "
			"SELECT YEAR(AtDate) * 10000 + MONTH(AtDate) * 100 + DAY(AtDate) AS DateID, AtDate, CountEntered, CountFinished, convert(float, CountEntered - CountFinished) AS NumOpen FROM @TableCounts "
			"ORDER BY AtDate");
		break;

	case INTERNAL_IncPerClient:
		//TODO

	default:
		strSql = "";
	break;

	//Open at end of week


	}

	return strSql;
}

//(e.lally 2009-08-24) PLID 35297 - Redid the conversion rate logic to use this query as the base "view", each tab manipulating this data as needed.
//	The bracketed filters are placeholders for the advanced filtering. They must be replaced or removed from all uses of the base query.
//(e.lally 2009-08-24) PLID 35299 - Added PatientCoordinatorID
//(e.lally 2009-09-24) PLID 35593 - Added optional joins, fields, etc for the multi referrals table support which required doing some of the subquery grouping and joins
CString GetConsultToProcedureConversionRateBaseSql(BOOL bUseMultiReferrals /*= FALSE*/)
{
	CString strSql = 
	"SELECT ConversionRateBaseQ.* "
	"FROM "
		//(e.lally 2009-09-24) PLID 35593 - New alias for consult data
		"(SELECT FinalConsultsQ.*, AllSurgeriesSubQ.*, PatientInformationQ.* "
		"FROM  \r\n"
			//PatientInformationQ
			/* Get all the patient information */
			//(e.lally 2009-09-18) PLID 35300 - Added DefaultReferringPhyID, ReferringPatientID, PrimaryReferralID.
			//(e.lally 2009-09-24) PLID 35593 - Changed PersonT.ID alias to PatientID for clarity
			//(e.lally 2010-03-23) PLID 37709 - Use ReferralSourceT.PersonID to elimate bad primary ReferralID data (such as -1 and null both meaning null)
			"(SELECT "+ CString(bUseMultiReferrals == FALSE ? "" : "DISTINCT ") +"PersonT.ID as PatientID, PersonT.First as PatientFirst, PersonT.Last as PatientLast, PersonT.Middle as PatientMiddle,  \r\n"
				"PatientsT.UserdefinedID, PatientsT.MainPhysician as PatientMainPhysicianID, PrimaryReferralSourceT.PersonID AS PrimaryReferralID, PatientsT.DefaultReferringPhyID, PatientsT.ReferringPatientID, \r\n"
				"ProviderPersonT.First as MainPhysicianFirst, ProviderPersonT.Last as MainPhysicianLast, ProviderPersonT.Middle as MainPhysicianMiddle,  \r\n"
				"LocationsT.ID as PatientLocationID, LocationsT.Name as PatientLocationName, \r\n"
				"PatientsT.EmployeeID AS PatientCoordinatorID \r\n"
				//(e.lally 2009-09-24) PLID 35593 - Include option for multi-referrals
				+ CString(bUseMultiReferrals == FALSE ? 
				"" : ", MultiReferralsT.ReferralID AS MultiReferralID ") +
			"FROM PersonT WITH(NOLOCK)  \r\n"
			"INNER JOIN PatientsT WITH(NOLOCK) ON PersonT.ID = PatientsT.PersonID  \r\n"
			//(e.lally 2009-09-24) PLID 35593 - Option to inner join the multireferrals table
			+ CString(bUseMultiReferrals == FALSE ? "" : "INNER JOIN MultiReferralsT WITH(NOLOCK) ON PatientsT.PersonID = MultiReferralsT.PatientID \r\n") +
			"LEFT JOIN PersonT ProviderPersonT WITH(NOLOCK) ON PatientsT.MainPhysician = ProviderPersonT.ID  \r\n"
			"LEFT JOIN LocationsT WITH(NOLOCK) ON PersonT.Location = LocationsT.ID  \r\n"
			//(e.lally 2010-03-23) PLID 37709 - Use a left join on referralSourceT to ensure ReferralID was valid.
			"LEFT JOIN ReferralSourceT PrimaryReferralSourceT WITH(NOLOCK) ON PatientsT.ReferralID = PrimaryReferralSourceT.PersonID \r\n"
			"WHERE PersonT.ID <> -25  \r\n"
			//Advanced filters
			"\r\n [PatientLWFilter] [PatientFilterDateFrom] [PatientFilterDateTo] [PatientLocation] [PatientProvider] \r\n"
			//(e.lally 2009-09-24) PLID 35592 - Added multi referral date filters
			+ CString(bUseMultiReferrals == FALSE ? "" : "[MultiReferralDateFrom] [MultiReferralDateTo] \r\n") +
			//(e.lally 2009-09-28) PLID 35594 - Added filter for referral source filtering - both primary and multi could apply here
			PRIMARY_REFERRAL_FILTER_PLACEHOLDER + MULTI_REFERRAL_FILTER_PLACEHOLDER +" \r\n"
			") PatientInformationQ \r\n" 
			//(e.lally 2009-09-24) PLID 35593 - Changed the way we group the subqueries, we need to select all from the AllConsultsSubQ here now
			+ CString(bUseMultiReferrals == FALSE ? 
			//(e.lally 2009-09-24) PLID 35593 - Use a left join if the patient/multi-referral data is the base where we only want appt data if it exists
			"INNER JOIN ( \r\n" : "LEFT JOIN (\r\n") +
				"SELECT AllConsultsSubQ.* \r\n"
				"FROM \r\n"
			//AllConsultsSubQ
			//(e.lally 2009-08-27) PLID 35297 - The detailed procedure here also duplicates records, inflating the consult numbers, 
				//so it needs to be removed and we need to use the Distinct results.
				//Anything that needs the detailed procedures will have to query it outside of here.
			//(e.lally 2009-09-24) PLID 35593 - Changed ApppointmentsT.PatientID alias to ConsultPatientID for clarity
			/* Get a list of all consult appointments (no cancelled or No Shows) */
				"(SELECT DISTINCT AppointmentsT.PatientID AS ConsultPatientID, AppointmentsT.ID AS ConsultApptID,   \r\n"
					"COALESCE(ProcedureT.MasterProcedureID, ProcedureT.ID) AS MasterProcedureID,  \r\n"
					"COALESCE(MasterProcedureT.Name, ProcedureT.Name) AS MasterProcedureName,  \r\n"
					"AppointmentsT.LocationID AS ConsultLocationID, LocationsT.Name AS ConsultLocationName,  \r\n"
					"AppointmentsT.CreatedDate AS ConsultCreatedDate, AppointmentsT.CreatedLogin AS ConsultCreatedLogin,  \r\n"
					"AppointmentsT.Date AS ConsultDate, AppointmentsT.StartTime AS ConsultStartTime, AppointmentsT.EndTime AS ConsultEndTime,  \r\n"
					"AptTypeT.ID as ConsultAptTypeID, AptTypeT.Name as ConsultAptTypeName  \r\n"
				"FROM AppointmentsT WITH(NOLOCK)  \r\n"
					"INNER JOIN AptTypeT  WITH(NOLOCK) ON AppointmentsT.AptTypeID = AptTypeT.ID  \r\n"
					"INNER JOIN AppointmentPurposeT WITH(NOLOCK) ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  \r\n"
					"INNER JOIN ProcedureT WITH(NOLOCK) ON AppointmentPurposeT.PurposeID = ProcedureT.ID  \r\n"
					"INNER JOIN LocationsT WITH(NOLOCK) ON AppointmentsT.LocationID = LocationsT.ID  \r\n"
					"LEFT JOIN ProcedureT MasterProcedureT WITH(NOLOCK) ON ProcedureT.MasterProcedureID = MasterProcedureT.ID \r\n"
				/* Query Analyzer says it is much faster to not filter here 
				"--WHERE AptTypeT.Category = 1 \r\n"
					"--AND AppointmentsT.ShowState <> 3 \r\n" //Ignore no shows  
					"--AND AppointmentsT.Status    <> 4 \r\n" //Ignore cancelled  appts */
				 ") AllConsultsSubQ \r\n"
			 //(e.lally 2009-09-24) PLID 35593 - The consult and patient info join no longer happens here
			 //"ON PatientInformationQ.PatientID = AllConsultsSubQ.ConsultPatientID \r\n"

			/* join on the list of first consult appointment IDs - by Patient, master Procedure */
			"INNER JOIN  \r\n"
			//FirstConsultSubQ
				"(SELECT CONVERT(INT, SUBSTRING(MIN(CONVERT(NVARCHAR, AppointmentsT.StartTime, 121) + '-' + CONVERT(NVARCHAR, AppointmentsT.ID)), 25, 20)) "
					"AS FirstConsultApptID, \r\n"
					"COALESCE(ProcedureT.MasterProcedureID, ProcedureT.ID) AS MasterProcedureID "
					//(e.lally 2009-09-24) PLID 35593 - Include option for multi-referrals
					+ CString(bUseMultiReferrals == FALSE ? "" : ", MultiReferralsT.ReferralID ") +
				"FROM AppointmentsT WITH(NOLOCK) "
					"INNER JOIN AppointmentPurposeT WITH(NOLOCK) ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  \r\n"
					"INNER JOIN ProcedureT WITH(NOLOCK) ON AppointmentPurposeT.PurposeID = ProcedureT.ID \r\n"
					//(e.lally 2009-09-24) PLID 35593 - Option to inner join the multireferrals table
					+ CString(bUseMultiReferrals == FALSE ? "" : "INNER JOIN MultiReferralsT WITH(NOLOCK) ON AppointmentsT.PatientID = MultiReferralsT.PatientID \r\n") +
				//(e.lally 2009-09-01) PLID 35429 - Replace the appointment type categories with the placeholders for the user defined lists
				"WHERE (AppointmentsT.AptTypeID IN("+ CONSULT_TYPE_PLACEHOLDER_1 + ") OR AppointmentsT.AptTypeID IN("+ CONSULT_TYPE_PLACEHOLDER_2 + ") ) \r\n"
					"AND AppointmentsT.ShowState <> 3  \r\n"/* Ignore no shows */ 
					"AND AppointmentsT.Status    <> 4 \r\n"/*Ignore cancelled  appts */ 
					//(e.lally 2009-09-24) PLID 35593 - When using multi referrals, make sure the appt date is on or after the referral
					//(e.lally 2009-09-24) PLID 35592 - Added multi referral date filters
					+ CString(bUseMultiReferrals == FALSE ? "" : "AND AppointmentsT.Date >= MultiReferralsT.Date \r\n"
						"[MultiReferralDateFrom] [MultiReferralDateTo]  \r\n") +
					//(e.lally 2009-09-28) PLID 35594 - Added filter for referral source filtering - only multi referrals can apply here
					MULTI_REFERRAL_FILTER_PLACEHOLDER +" \r\n"
					//Advanced filters for consults
					"[ConsultDateFrom] [ConsultDateTo] "
					"[ConsultLocation] [ConsultResource] " +
					//(e.lally 2009-09-11) PLID 35521 - Added filter for procedure filtering
					PROCEDURE_FILTER_PLACEHOLDER + " \r\n"
				"GROUP BY AppointmentsT.PatientID, COALESCE(ProcedureT.MasterProcedureID, ProcedureT.ID) \r\n"
				//(e.lally 2009-09-24) PLID 35593 - Group by when including option for multi-referrals
				+ CString(bUseMultiReferrals == FALSE ? "" : ",MultiReferralsT.ReferralID ") +
				")FirstConsultSubQ \r\n"
			"ON AllConsultsSubQ.ConsultApptID = FirstConsultSubQ.FirstConsultApptID \r\n"
				"AND AllConsultsSubQ.MasterProcedureID = FirstConsultSubQ.MasterProcedureID \r\n"
			//(e.lally 2009-09-24) PLID 35593 - New alias for consult data
			")FinalConsultsQ \r\n"
			"ON PatientInformationQ.PatientID = FinalConsultsQ.ConsultPatientID \r\n"
			/* left join the surgeries that match this patient, procedure combination. They should all be the first ones after the first consult */
			"LEFT JOIN ( \r\n"
			//FirstSurgerySubQ
				/*Get the first surgeries after the first consult per patientID and master procedure */
				"(SELECT CONVERT(INT, SUBSTRING(MIN(CONVERT(NVARCHAR, AppointmentsT.StartTime, 121) + '-' + CONVERT(NVARCHAR, AppointmentsT.ID)), 25, 20)) AS FirstSurgeryApptID,  \r\n"
					"AppointmentsT.PatientID,  \r\n"
					"COALESCE(ProcedureT.MasterProcedureID, ProcedureT.ID) AS MasterProcedureID \r\n"
					//(e.lally 2009-09-24) PLID 35593 - Include option for multi-referrals
					+ CString(bUseMultiReferrals == FALSE ? "" : ", FirstConsultDatesSubQ.ReferralID AS MultiRefferalID \r\n") +
				"FROM AppointmentsT WITH(NOLOCK)  \r\n"
					"INNER JOIN AppointmentPurposeT WITH(NOLOCK) ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  \r\n"
					"INNER JOIN ProcedureT WITH(NOLOCK) ON AppointmentPurposeT.PurposeID = ProcedureT.ID \r\n"
					"INNER JOIN "
						/* We have to rejoin the consult information in order to compare our date.  We want the min surgery after the first consult - per patient per master procedure of couse */
						/* Notice we are just using the min startTime here because we don't need the appointment ID information for our comparison. */
						"(SELECT Min(StartTime) as MinStartTime, AppointmentsT.PatientID, COALESCE(ProcedureT.MasterProcedureID, ProcedureT.ID) AS MasterProcedureID \r\n"
							//(e.lally 2009-09-24) PLID 35593 - Include option for multi-referrals
							+ CString(bUseMultiReferrals == FALSE ? "" : ", MultiReferralsT.ReferralID \r\n") +
						"FROM AppointmentsT WITH(NOLOCK) \r\n"
							"INNER JOIN AppointmentPurposeT WITH(NOLOCK) ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  \r\n"
							"INNER JOIN ProcedureT WITH(NOLOCK) ON AppointmentPurposeT.PurposeID = ProcedureT.ID \r\n"
							//(e.lally 2009-09-24) PLID 35593 - Option to inner join the multireferrals table
							+ CString(bUseMultiReferrals == FALSE ? "" : "INNER JOIN MultiReferralsT WITH(NOLOCK) ON AppointmentsT.PatientID = MultiReferralsT.PatientID \r\n") +
						//(e.lally 2009-09-01) PLID 35429 - Replace the appointment type categories with the placeholders for the user defined lists
						"WHERE (AppointmentsT.AptTypeID IN("+ CONSULT_TYPE_PLACEHOLDER_1 + ") OR AppointmentsT.AptTypeID IN("+ CONSULT_TYPE_PLACEHOLDER_2 + ") ) \r\n"
							"AND AppointmentsT.ShowState <> 3 "/* Ignore no shows */ 
							"AND AppointmentsT.Status    <> 4 \r\n"/*Ignore cancelled  appts */ 
							//(e.lally 2009-09-24) PLID 35593 - When using multi referrals, make sure the appt date is on or after the referral
							+ CString(bUseMultiReferrals == FALSE ? "" : "AND AppointmentsT.Date >= MultiReferralsT.Date \r\n"
							//(e.lally 2009-09-24) PLID 35592 - Added multi referral date filters
							"[MultiReferralDateFrom] [MultiReferralDateTo] \r\n") +
							//(e.lally 2009-09-28) PLID 35594 - Added filter for referral source filtering - only multi referrals can apply here
							MULTI_REFERRAL_FILTER_PLACEHOLDER +" \r\n"
							//(e.lally 2009-09-08) PLID 35297 - Need to carry the consult filters through here too
							//Advanced filters for consults
							"[ConsultDateFrom] [ConsultDateTo] "
							"[ConsultLocation] [ConsultResource] " +
							//(e.lally 2009-09-11) PLID 35521 - Added filter for procedure filtering
							PROCEDURE_FILTER_PLACEHOLDER + " \r\n"

						"GROUP BY AppointmentsT.PatientID, COALESCE(ProcedureT.MasterProcedureID, ProcedureT.ID) \r\n"
							//(e.lally 2009-09-24) PLID 35593 - Group by when including option for multi-referrals
							+ CString(bUseMultiReferrals == FALSE ? "" : ",MultiReferralsT.ReferralID \r\n") +
						")FirstConsultDatesSubQ \r\n"
						/* We should only need to join on the patient ID and master procedureID because we are only taking the first surgery on this grouping */
						"ON  FirstConsultDatesSubQ.PatientID = AppointmentsT.PatientID  \r\n"
						"AND FirstConsultDatesSubQ.MasterProcedureID = COALESCE(ProcedureT.MasterProcedureID, ProcedureT.ID) \r\n"
				//(e.lally 2009-09-01) PLID 35429 - Replace the appointment type categories with the placeholder for the user defined lists
				"WHERE AppointmentsT.AptTypeID IN([SurgeryTypeList])  \r\n"/* Surgery and minor procedures */ 
					"AND AppointmentsT.ShowState <> 3  \r\n"/* Ignore no shows */ 
					"AND AppointmentsT.Status    <> 4  \r\n"/*Ignore cancelled  appts */ 
					"AND AppointmentsT.StartTime >= FirstConsultDatesSubQ.MinStartTime \r\n"
				"GROUP BY AppointmentsT.PatientID, COALESCE(ProcedureT.MasterProcedureID, ProcedureT.ID) \r\n"
					//(e.lally 2009-09-24) PLID 35593 - Group by when including option for multi-referrals
					+ CString(bUseMultiReferrals == FALSE ? "" : ",FirstConsultDatesSubQ.ReferralID \r\n") +
				")FirstSurgerySubQ \r\n"

				/* Now that we know what the first surgeries appointment IDs are, we can get the rest of the surgery appointment information. */
				"INNER JOIN  \r\n"
				//AllSurgeriesSubQ
					/* We need the Distinct clause here in cause the surgery is for multiple detailed procedures for the same master */
					"(SELECT DISTINCT AppointmentsT.ID AS SurgeryApptID,  \r\n"
						"AppointmentsT.LocationID AS SurgeryLocationID, LocationsT.Name AS SurgeryLocationName,  \r\n"
						"AppointmentsT.CreatedDate AS SurgeryCreatedDate, AppointmentsT.CreatedLogin AS SurgeryCreatedLogin,  \r\n"
						"AppointmentsT.Date AS SurgeryDate, AppointmentsT.StartTime AS SurgeryStartTime, AppointmentsT.EndTime AS SurgeryEndTime,  \r\n"
						"AptTypeT.ID as SurgeryAptTypeID, AptTypeT.Name as SurgeryAptTypeName,  \r\n"
						"COALESCE(ProcedureT.MasterProcedureID, ProcedureT.ID) AS SurgeryMasterProcedureID \r\n"
						//"ProcedureT.ID as SurgeryDetailedProcedureID, ProcedureT.Name AS SurgeryDetailedProcedureName \r\n"
					"FROM AppointmentsT WITH(NOLOCK)  \r\n"
						"INNER JOIN AptTypeT WITH(NOLOCK) ON AppointmentsT.AptTypeID = AptTypeT.ID  \r\n"
						"INNER JOIN AppointmentPurposeT WITH(NOLOCK) ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  \r\n"
						"INNER JOIN ProcedureT WITH(NOLOCK) ON AppointmentPurposeT.PurposeID = ProcedureT.ID  \r\n"
						"INNER JOIN LocationsT WITH(NOLOCK) ON AppointmentsT.LocationID = LocationsT.ID  \r\n"
						"LEFT JOIN ProcedureT MasterProcedureT WITH(NOLOCK) ON ProcedureT.MasterProcedureID = MasterProcedureT.ID \r\n"
					/* Query Analyzer says it is much faster to not filter here 
					"--WHERE AptTypeT.Category IN(3, 4) \r\n"// Minor Procedure, Surgery
						"--AND AppointmentsT.ShowState <> 3 \r\n"// Ignore no shows 
						"--AND AppointmentsT.Status    <> 4 \r\n"// Ignore cancelled  appts */
					") AllSurgeriesSubQ \r\n"
				"ON  FirstSurgerySubQ.FirstSurgeryApptID = AllSurgeriesSubQ.SurgeryApptID "
					"AND FirstSurgerySubQ.MasterProcedureID = AllSurgeriesSubQ.SurgeryMasterProcedureID \r\n"

				")\r\n"
			/* We have already filtered all the consults to be just the first ones via the inner join, so we can add these first surgeries based on the patient, master procedure grouping once more */
			"ON  FinalConsultsQ.ConsultPatientID = FirstSurgerySubQ.PatientID  \r\n"
			"AND FinalConsultsQ.MasterProcedureID = FirstSurgerySubQ.MasterProcedureID \r\n"
			//(e.lally 2009-09-24) PLID 35593 - Join the multireferral IDs in the join when including the multi referrals table option
			+ CString(bUseMultiReferrals == FALSE ? "" : "AND PatientInformationQ.MultiReferralID = FirstSurgerySubQ.MultiRefferalID \r\n") +

	")ConversionRateBaseQ  \r\n";
	return strSql;
}

//(e.lally 2009-09-01) PLID 35429 - Gets a string of the standardized list of statistics fields using the placeholders for the
	//consult and surgery appointment type ID list filtering.
//(e.lally 2009-09-08) PLID 35429 - Give splits for converted consults so we can compare separately
//(e.lally 2009-09-24) PLID 35593 - Added paramter for use referral rate fields
CString GetConsultToProcedureGraphStatFields(BOOL bUseReferralRateFields /*= FALSE*/)
{
	CString strFields = 
			"CAST (Count(DISTINCT SurgeryApptID) AS FLOAT) AS TotalConvertedConsultCount, \r\n"
			"CAST (Count(DISTINCT CASE WHEN ConsultAptTypeID IN("+ CONSULT_TYPE_PLACEHOLDER_1 + ") THEN SurgeryApptID END ) AS FLOAT) AS ConvertedConsultCount1, "
			"CAST (Count(DISTINCT CASE WHEN ConsultAptTypeID IN("+ CONSULT_TYPE_PLACEHOLDER_2 + ") THEN SurgeryApptID END ) AS FLOAT) AS ConvertedConsultCount2, "
			"CAST (Count(DISTINCT CASE WHEN ConsultAptTypeID IN("+ CONSULT_TYPE_PLACEHOLDER_1 + ") THEN ConsultApptID END ) AS FLOAT) AS ConsultCount1, \r\n"
			"CAST (Count(DISTINCT CASE WHEN ConsultAptTypeID IN("+ CONSULT_TYPE_PLACEHOLDER_2 + ") THEN ConsultApptID END ) AS FLOAT) AS ConsultCount2, \r\n"
			"CAST (Count(DISTINCT ConsultApptID) AS FLOAT) AS TotalConsultCount, \r\n"

			"CASE WHEN (CAST (Count(DISTINCT CASE WHEN ConsultAptTypeID IN("+ CONSULT_TYPE_PLACEHOLDER_1 + ") THEN ConsultApptID END) AS FLOAT) = 0.0) THEN 0 "
				"ELSE (CAST (Count(DISTINCT CASE WHEN ConsultAptTypeID IN("+ CONSULT_TYPE_PLACEHOLDER_1 + ") THEN SurgeryApptID END) AS FLOAT) "
				" / CAST (Count(DISTINCT CASE WHEN ConsultAptTypeID IN("+ CONSULT_TYPE_PLACEHOLDER_1 + ") THEN ConsultApptID END) AS FLOAT)) * 100  END AS ConversionRate1, \r\n"
			"CASE WHEN (CAST (Count(DISTINCT CASE WHEN ConsultAptTypeID IN("+ CONSULT_TYPE_PLACEHOLDER_2 + ") THEN ConsultApptID END) AS FLOAT) = 0.0) THEN 0 "
				"ELSE (CAST (Count(DISTINCT CASE WHEN ConsultAptTypeID IN("+ CONSULT_TYPE_PLACEHOLDER_2 + ") THEN SurgeryApptID END) AS FLOAT) "
				" / CAST (Count(DISTINCT CASE WHEN ConsultAptTypeID IN("+ CONSULT_TYPE_PLACEHOLDER_2 + ") THEN ConsultApptID END) AS FLOAT)) * 100  END AS ConversionRate2, \r\n"
			"CASE WHEN (CAST (Count(DISTINCT ConsultApptID) AS FLOAT) = 0.0) THEN 0 "
				"ELSE (CAST (Count(DISTINCT SurgeryApptID) AS FLOAT) "
				" / CAST (Count(DISTINCT ConsultApptID) AS FLOAT)) * 100  END AS TotalConversionRate \r\n";
	//(e.lally 2009-09-24) PLID 35593
	if(bUseReferralRateFields){
		CString strUniqueReferralCount = " CAST (Count(DISTINCT(CONVERT(NVARCHAR(50), MultiReferralID) + '_' + CONVERT(NVARCHAR(50), PatientID) ) ) AS FLOAT) ";
		CString strUniqueReferralConsultCount = " CAST (Count( DISTINCT(CONVERT(NVARCHAR(50), MultiReferralID) + '_' + CONVERT(NVARCHAR(50), ConsultApptID)) ) AS FLOAT) ";
		CString strUniqueReferralSurgeryCount = " CAST (Count( DISTINCT(CONVERT(NVARCHAR(50), MultiReferralID) + '_' + CONVERT(NVARCHAR(50), SurgeryApptID)) ) AS FLOAT) ";
		strFields +=
			", " + strUniqueReferralCount + " AS TotalReferralCount, \r\n"
			"CASE WHEN ("+ strUniqueReferralCount + ") = 0.0 THEN 0 ELSE (" + strUniqueReferralConsultCount + " / " + strUniqueReferralCount + " * 100)  END AS TotalReferralToConsultRate, \r\n"
			"CASE WHEN ("+ strUniqueReferralCount + ") = 0.0 THEN 0 ELSE (" + strUniqueReferralSurgeryCount + " / " + strUniqueReferralCount + " * 100)  END AS TotalReferralToProcedureRate  \r\n";
	}
	return strFields;
}

//(e.lally 2009-08-25) PLID 35297 - Gathers the appropriate parameters from the Docbar and applies them to the proper
//graph type. The base query to be used is passed in and updated
// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
void ApplyDocbarGraphFilters(IN OUT CString &strSql, MarketGraphType mktGraph, ADODB::_ConnectionPtr pCon, OUT CString &strPatientTempTable)
{
	switch(mktGraph){
		case PROCConsultToSurg:
		case CNVConsToSurgByPatCoord: 
		case CNVConstoSurgByDate:
		case CNVConsToSurgByRefSour: {
			//Get each of the individual filter strings and the replace the respective placeholders with the filters.
			//Some strings may be empty if that filter is not applicable.
			CString strPatientLWFilter = GetDocbarPatientLWFilter(mktGraph, pCon, strPatientTempTable);
			CString strPatientFilterDateFrom = GetDocbarPatientFilterDateFrom(mktGraph);
			CString strPatientFilterDateTo = GetDocbarPatientFilterDateTo(mktGraph);
			CString strPatientLocation = GetDocbarPatientLocationFilter(mktGraph);
			CString strPatientProvider = GetDocbarPatientProviderFilter(mktGraph);
			CString strConsultDateFrom = GetDocbarConsultDateFromFilter(mktGraph);
			CString strConsultDateTo = GetDocbarConsultDateToFilter(mktGraph);
			CString strConsultLocation = GetDocbarConsultLocation(mktGraph);
			CString strConsultResource = GetDocbarConsultResource(mktGraph);
			//(e.lally 2009-09-24) PLID 35592 - Multi-referral Date filtering (referrals tab only)
			CString strReferralDateFrom;
			CString strReferralDateTo;
			if(mktGraph == CNVConsToSurgByRefSour){
				strReferralDateFrom = GetDocbarReferralDateFromFilter(mktGraph);
				strReferralDateTo = GetDocbarReferralDateToFilter(mktGraph);
			}

			CString strConsultTypeList1, strConsultTypeList2, strSurgeryTypeList;
			//(e.lally 2009-09-08) PLID 35429 - Use the preference for consult types, splitting consults, and procedure types
				//to use on the graphs.
			GetUserDefinedConsToProcAptTypeLists(strConsultTypeList1, strConsultTypeList2, strSurgeryTypeList);

			CString strPurposeList = GetRemotePropertyText("CRGPurposeList", "", 0, "<None>", TRUE);


			//(e.lally 2009-08-25) PLID 35297 - Apply the filters to the SQL query string passed in.
			strSql.Replace("[PatientLWFilter]", strPatientLWFilter);
			strSql.Replace("[PatientFilterDateFrom]", strPatientFilterDateFrom);
			strSql.Replace("[PatientFilterDateTo]", strPatientFilterDateTo);
			strSql.Replace("[PatientLocation]", strPatientLocation);
			strSql.Replace("[PatientProvider]", strPatientProvider);
			strSql.Replace("[ConsultDateFrom]", strConsultDateFrom);
			strSql.Replace("[ConsultDateTo]", strConsultDateTo);
			strSql.Replace("[ConsultLocation]", strConsultLocation);
			strSql.Replace("[ConsultResource]", strConsultResource);

			//(e.lally 2009-09-11) PLID 35521 - Added filter for procedure filtering, only these graphs support it right now though
			switch(mktGraph){
				case PROCConsultToSurg:
				case CNVConstoSurgByDate:
					//do nothing, we have to let the local tab code handle this since the filter is not part of the docbar yet.
					break;
				default:
					//This filter is not supported yet, so we need to remove it
					strSql.Replace(PROCEDURE_FILTER_PLACEHOLDER, "");
					break;
			}
			//(e.lally 2009-09-28) PLID 35594 - Added filter for primary referral and multi referral source filtering, only
				//these graphs support it right now though
			switch(mktGraph){
				case CNVConsToSurgByRefSour:
					//do nothing, we have to let the local tab code handle this since the filter is not part of the docbar yet.
					break;
				default:
					//This filter is not supported yet, so we need to remove the placeholder
					strSql.Replace(PRIMARY_REFERRAL_FILTER_PLACEHOLDER, "");
					strSql.Replace(MULTI_REFERRAL_FILTER_PLACEHOLDER, "");
					break;
			}

			

			//(e.lally 2009-09-01) PLID 35429 - Replace the appointment type placeholders with the user defined lists
			strSql.Replace(CONSULT_TYPE_PLACEHOLDER_1, strConsultTypeList1);
			strSql.Replace(CONSULT_TYPE_PLACEHOLDER_2, strConsultTypeList2);
			strSql.Replace(SURGERY_TYPE_PLACEHOLDER, strSurgeryTypeList);

			//(e.lally 2009-09-24) PLID 35592 - Multi-referral date filters
			strSql.Replace("[MultiReferralDateFrom]", strReferralDateFrom);
			strSql.Replace("[MultiReferralDateTo]", strReferralDateTo);
			break;
		}
		default:
			ASSERT(FALSE);
			break;
	}
}

//(e.lally 2009-09-08) PLID 35429 - Sets the comma delimited appointment type ID lists for the userdefined
//New Patient Consults, Established Consults, and Procedure appt types to be used in marketing
void GetUserDefinedConsToProcAptTypeLists(CString& strConsultTypeList1, CString& strConsultTypeList2, CString& strSurgeryTypeList)
{
	//(e.lally 2009-09-08) PLID 35429 - Use the preference for consult types, splitting consults, and procedure types
		//to use on the graphs.
	strConsultTypeList1 = "-1";
	strConsultTypeList2 = "-1";
	BOOL bSplitConsults = GetRemotePropertyInt("CRGSplitConsults", 1, 0, "<None>", TRUE);
	CString strSplitConsultIDList = GetRemotePropertyText("CRGConsultList", "", 0, "<None>", TRUE);
	//(e.lally 2009-09-14) PLID 35527 - Make the single list of consult appt types a separate preference
	CString strSingleConsultIDList = GetRemotePropertyText("CRGSingleConsultList", "", 0, "<None>", TRUE);

	strSurgeryTypeList = GetRemotePropertyText("CRGSurgeryList", "", 0, "<None>", TRUE);
	if(strSurgeryTypeList.IsEmpty()){
		//We need some value here to avoid exceptions
		strSurgeryTypeList = "-1";
	}
	if(strSplitConsultIDList.IsEmpty()){
		//We need some value here to avoid exceptions
		strSplitConsultIDList = "-1";
	}
	if(strSingleConsultIDList.IsEmpty()){
		//(e.lally 2009-09-14) PLID 35527 - To make the transition a little smoother, check the split consult list and try to combine them
		if(!strSplitConsultIDList.IsEmpty()){
			CString strTemp = strSplitConsultIDList;
			strTemp.Replace("(", "");
			strTemp.Replace(")", "");
			strTemp.Replace("---", ",");
			strTemp.Replace(",,", ",");
			strTemp.TrimRight(",");
			//We should have one continuous list of IDs now.
			strSingleConsultIDList = "(" + strTemp + ")";
			//Save it as the default for this preference.
			SetRemotePropertyText("CRGSingleConsultList", strSingleConsultIDList, 0 , "<None>");
		}
		else {
			//Give up, we need some value here to avoid exceptions
			strSingleConsultIDList = "-1";
		}
	}

	//(e.lally 2009-09-14) PLID 35527 - Make the single list of consult appt types a separate preference
	if(bSplitConsults){
		long nResult = strSplitConsultIDList.Find("---");
		if(nResult >=0){
			strConsultTypeList1 = strSplitConsultIDList.Left(nResult);
			if(bSplitConsults){
				strConsultTypeList2 = strSplitConsultIDList.Right(strSplitConsultIDList.GetLength() - (nResult + 3));
			}
		}
		else {
			strConsultTypeList1 = strSplitConsultIDList;
		}
	}
	else{
		strConsultTypeList1 = strSingleConsultIDList;
	}
	//(e.lally 2009-09-14) Any extra parentheses throw an error, and the base query already contains them to make everything
	//a bit more readable
	strConsultTypeList1.Replace("(", "");
	strConsultTypeList1.Replace(")", "");

	strConsultTypeList2.Replace("(", "");
	strConsultTypeList2.Replace(")", "");

	strSurgeryTypeList.Replace("(", "");
	strSurgeryTypeList.Replace(")", "");

}

// (z.manning 2009-09-08 17:15) - PLID 35051
void GetUserDefinedConsToProcLabels(OUT CString &strCons1Label, OUT CString &strCons2Label, OUT CString &strSurgeryLabel)
{
	BOOL bSplitConsults = GetRemotePropertyInt("CRGSplitConsults", 1, 0, "<None>", TRUE);

	//(e.lally 2009-09-14) PLID 35527 - Make the single list of consult appt types a separate preference
		//Changed the default on the split consult label
	CString strSplitConsultLabelTmp = GetRemotePropertyText("CRGConsultLabels", "New Consults---Est. Consults", 0, "<None>", TRUE);
	CString strSingleConsultLabelTmp = GetRemotePropertyText("CRGSingleConsultLabels", "Consults", 0, "<None>", TRUE);

	if(bSplitConsults){
		int nResult = strSplitConsultLabelTmp.Find("---");
		if(nResult >= 0){
			strCons1Label = strSplitConsultLabelTmp.Left(nResult);
			if(bSplitConsults) {
				strCons2Label = strSplitConsultLabelTmp.Right(strSplitConsultLabelTmp.GetLength() - (nResult + 3));
			}
		}
		else{
			strCons1Label = strSplitConsultLabelTmp;
		}
	}
	else{
		strCons1Label = strSingleConsultLabelTmp;
	}

	//(e.lally 2009-09-14) PLID 35527 - Changed default to say Procedures since m.clark says clients think of them as
	// procedures, not converted consults (doing away with surgeries references)
	strSurgeryLabel = GetRemotePropertyText("CRGSurgeryLabel", "Procedures", 0, "<None>", TRUE);	
}

// (z.manning 2009-09-08 17:47) - PLID 35051
BOOL IsConsToProcSetupValid()
{
	BOOL bSplitConsults = GetRemotePropertyInt("CRGSplitConsults", 1, 0, "<None>", TRUE);
	//(e.lally 2009-09-14) PLID 35527 - Make the single list of consult appt types a separate preference
	CString strSplitConsultIDList = GetRemotePropertyText("CRGConsultList", "", 0, "<None>", TRUE);
	CString strSingleConsultIDList = GetRemotePropertyText("CRGSingleConsultList", "", 0, "<None>", TRUE);
	CString strSurgIDList = GetRemotePropertyText("CRGSurgeryList", "", 0, "<None>", TRUE);

	if (!bSplitConsults) {
		//make sure the id list is there so that we don't get any errors
		//(e.lally 2009-09-14) PLID 35527 - Make the single list of consult appt types a separate preference
		if (strSingleConsultIDList.IsEmpty() || strSurgIDList.IsEmpty()) {
			return FALSE;
		}
	}
	else {
		CString strCons1Tmp, strCons2Tmp;

		long nResult = strSplitConsultIDList.Find("---");
		if(nResult >= 0){
			strCons1Tmp = strSplitConsultIDList.Left(nResult);
			strCons2Tmp = strSplitConsultIDList.Right(strSplitConsultIDList.GetLength() - (nResult + 3));
		}

		//make sure the id list is there so that we don't get any errors
		if (strCons1Tmp.IsEmpty() || strCons2Tmp.IsEmpty() || strSurgIDList.IsEmpty()) {
			return FALSE;
		}
	}

	return TRUE;
}

void AddConsToProcDataToGraphDesc(const CString strSummarySql, GraphDescript *pDesc, BOOL bShowNumbers, BOOL bShowPercentages, BOOL bHandlePercentsManually /* = FALSE */)
{
	BOOL bSplitConsults = GetRemotePropertyInt("CRGSplitConsults", 1, 0, "<None>", TRUE);

	CString strCons1Label, strCons2Label, strSurgeryLabel;
	GetUserDefinedConsToProcLabels(strCons1Label, strCons2Label, strSurgeryLabel);

	if (!bSplitConsults)
	{
		//(e.lally 2009-08-31) PLID 35301 - updated to reflect statistics field names from the new base query
		CString strSql = FormatString(" SELECT CASE WHEN Sum(TotalConsultCount) = 0 THEN 0 ELSE 100 * Sum(TotalConvertedConsultCount)/Sum(TotalConsultCount) END AS ConvRate, "
			"SUM(TotalConsultCount) as SumConsult, "
			"Sum(TotalConvertedConsultCount) as SumSurgery "
			"FROM (\r\n %s \r\n) BASE ", strSummarySql);
		_RecordsetPtr rsYearPercent = CreateRecordsetStd(strSql);

		CString strYearPercent, strSumConsult, strSumSurgery;
		strYearPercent.Format(" (%.0f%%)", AdoFldDouble(rsYearPercent, "ConvRate", 0));
		strSumConsult.Format(" (%li Total)", (long)AdoFldDouble(rsYearPercent, "SumConsult", 0));
		strSumSurgery.Format(" (%li Total)", (long)AdoFldDouble(rsYearPercent, "SumSurgery", 0));

		//(e.lally 2009-08-31) PLID 35301 - updated to reflect statistics field names from the new base query
		//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph colors
		if (bShowNumbers) {
			pDesc->Add(strCons1Label + strSumConsult, "TotalConsultCount", GetMarketGraphColor(mgcBrightGreen), "TotalConsultCount");
			pDesc->Add(strSurgeryLabel + strSumSurgery, "TotalConvertedConsultCount", GetMarketGraphColor(mgcBrightRed), "TotalConvertedConsultCount");
		}
		if (bShowPercentages) {
			if(bHandlePercentsManually) {
				//(e.lally 2009-09-18) PLID 35300 - This needs to use the new percent operation because we can't add up the child percents
				//and the division one forces us to send in an inflated field1 amount which won't work either.
				pDesc->Add("Conversion Rate" + strYearPercent, "TotalConvertedConsultCount", GetMarketGraphColor(mgcBrightBlue), "TotalConsultCount", GraphDescript::GD_PERCENT);
			}
			else {
				pDesc->Add("Conversion Rate" + strYearPercent, "TotalConversionRate", GetMarketGraphColor(mgcBrightBlue), "TotalConversionRate");
			}
		}
	}
	else
	{
		//there are two sets of consult appts, so we need to add on a query, I suppose

		// (b.cardillo 2005-10-03 13:26) - PLID 17626 - Made it calculate the total field 
		// as a physical count of appointments rather than the sum of the consult sets.
		//(e.lally 2009-09-01) PLID 35301 - Updated to reflect the new statistic field names in the base query
		//(e.lally 2009-09-08) PLID 35429 - Compare splits to converted consult counterparts
		CString strSql = FormatString(" SELECT "
			" SUM(ConsultCount1) as TotalConsultCount1, "
			" SUM(ConvertedConsultCount1) AS TotalConvertedConsultCount1, "
			" CASE WHEN Sum(ConsultCount1) = 0 THEN 0 ELSE 100 * Sum(ConvertedConsultCount1)/Sum(ConsultCount1) END AS Consult1ConvRate, "
			" Sum(ConsultCount2) AS TotalConsultCount2, "
			" SUM(ConvertedConsultCount2) AS TotalConvertedConsultCount2, "
			" CASE WHEN Sum(ConsultCount2) = 0 THEN 0 ELSE 100 * Sum(ConvertedConsultCount2)/Sum(ConsultCount2) END AS Consult2ConvRate, "
			"Sum(TotalConsultCount) AS TotalConsultCount, " 
			"Sum(TotalConvertedConsultCount) as TotalConvertedConsults,  "
			" CASE WHEN Sum(TotalConsultCount) = 0 THEN 0 ELSE 100 * Sum(TotalConvertedConsultCount)/(Sum(TotalConsultCount)) END AS TotalConversionRate "
			" FROM (\r\n %s  \r\n) BASE ", strSummarySql);
		_RecordsetPtr rsGrandTotal = CreateRecordsetStd(strSql);

		CString strCons1Percent, strCons2Percent, strTotalPercent;
		CString strSumCons1, strSumCons2, strSumConsTotal;
		CString strSumConversionCnt1, strSumConversionCnt2, strSumConversionTotal;
		//(e.lally 2009-09-29) PLID 35526 - Change the grouping of the field totals
		//Consult type 1 totals
		strSumCons1.Format(" (%li Total)", (long)AdoFldDouble(rsGrandTotal, "TotalConsultCount1", 0));
		strSumConversionCnt1.Format(" (%li Total)", (long)AdoFldDouble(rsGrandTotal, "TotalConvertedConsultCount1", 0));
		strCons1Percent.Format(" (%li%%)", (long)AdoFldDouble(rsGrandTotal, "Consult1ConvRate", 0));

		//Consult type 2 totals
		strSumCons2.Format(" (%li Total)", (long)AdoFldDouble(rsGrandTotal, "TotalConsultCount2", 0));
		strSumConversionCnt2.Format(" (%li Total)", (long)AdoFldDouble(rsGrandTotal, "TotalConvertedConsultCount2", 0));
		strCons2Percent.Format(" (%li%%)", (long)AdoFldDouble(rsGrandTotal, "Consult2ConvRate", 0));

		strSumConsTotal.Format(" (%li Total)", (long)AdoFldDouble(rsGrandTotal, "TotalConsultCount", 0));
		strSumConversionTotal.Format(" (%li Total)", (long)AdoFldDouble(rsGrandTotal, "TotalConvertedConsults", 0));
		//(e.lally 2009-09-25) PLID 35301 - This needs to be rounded 0 decimal places (whole number)
		strTotalPercent.Format(" (%.0f%%)", AdoFldDouble(rsGrandTotal, "TotalConversionRate", 0));

		//(e.lally 2009-08-31) PLID 35301 - updated to reflect statistics field names from the new base query
		//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph colors
		//(e.lally 2009-09-29) PLID 35526 - Change the grouping of fields to show all consult 1 data then all consult 2 data. Changed some of the colors around.
		if (bShowNumbers) {
			pDesc->Add(strCons1Label + strSumCons1, "ConsultCount1", GetMarketGraphColor(mgcDarkGreen), "ConsultCount1");
			pDesc->Add(strSurgeryLabel + strSumConversionCnt1, "ConvertedConsultCount1", RGB(170,255,204), "ConvertedConsultCount1");
		}
		if (bShowPercentages) {
			if(bHandlePercentsManually) {
				//(e.lally 2009-09-18) PLID 35300 - This needs to use the new percent operation because we can't add up the child percents
				//and the division one forces us to send in an inflated field1 amount which won't work either.
				pDesc->Add(strCons1Label + " Conversion Rate" + strCons1Percent, "ConvertedConsultCount1", GetMarketGraphColor(mgcBrightGreen), "ConsultCount1", GraphDescript::GD_PERCENT);
			}
			else {
				pDesc->Add(strCons1Label + " Conversion Rate" + strCons1Percent, "ConversionRate1", GetMarketGraphColor(mgcBrightGreen), "ConversionRate1");
			}
		}
		//Now the second set
		if (bShowNumbers) {
			pDesc->Add(strCons2Label + strSumCons2, "ConsultCount2", GetMarketGraphColor(mgcBrightBlue), "ConsultCount2");
			pDesc->Add(strSurgeryLabel + strSumConversionCnt2, "ConvertedConsultCount2", GetMarketGraphColor(mgcLightBlue), "ConvertedConsultCount2");
		}
		if (bShowPercentages) {
			if(bHandlePercentsManually) {
				//(e.lally 2009-09-18) PLID 35300 - This needs to use the new percent operation because we can't add up the child percents
				//and the division one forces us to send in an inflated field1 amount which won't work either.
				pDesc->Add(strCons2Label + " Conversion Rate" + strCons2Percent, "ConvertedConsultCount2", GetMarketGraphColor(mgcBrightPurple), "ConsultCount2", GraphDescript::GD_PERCENT);
			}
			else {
				pDesc->Add(strCons2Label + " Conversion Rate" + strCons2Percent, "ConversionRate2", GetMarketGraphColor(mgcBrightPurple), "ConversionRate2");
			}
		}
		//Finally, the totals
		if (bShowNumbers && !bShowPercentages){
			//Show the number totals
			pDesc->Add("Total Consults" + strSumConsTotal, "TotalConsultCount", GetMarketGraphColor(mgcBrightOrange), "TotalConsultCount");
			pDesc->Add("Total " + strSurgeryLabel + strSumConversionTotal, "TotalConvertedConsultCount", GetMarketGraphColor(mgcBrightRed), "TotalConvertedConsultCount");
		}
		if(bShowPercentages){
			if(bHandlePercentsManually) {
				//(e.lally 2009-09-18) PLID 35300 - This needs to use the new percent operation because we can't add up the child percents
				//and the division one forces us to send in an inflated field1 amount which won't work either.
				pDesc->Add("Total Conversion Rate" + strTotalPercent, "TotalConvertedConsultCount", GetMarketGraphColor(mgcBrightRed), "TotalConsultCount", GraphDescript::GD_PERCENT);
			}
			else {
				pDesc->Add("Total Conversion Rate" + strTotalPercent, "TotalConversionRate", GetMarketGraphColor(mgcBrightRed), "TotalConversionRate");
			}
		}
	}
}

//(e.lally 2009-08-25) PLID 35297 - Adds the individual filter strings to the array passed in.
//	Since we only have a CStringArray to work with, the order of the strings is important for outside callers to know
//	what they match up to.
// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
void BuildDocbarFilterArray(OUT CStringArray& saryFilters, MarketGraphType mktGraph, ADODB::_ConnectionPtr pCon, OUT CString &strPatientTempTable)
{
	switch(mktGraph){
		//Supported consult to procedure conversion rate graphs
		case PROCConsultToSurg:
		case CNVConsToSurgByPatCoord: 
		case CNVConstoSurgByDate:
		case CNVConsToSurgByRefSour:{
			/***********************************************************************************************************
			*
			* (e.lally 2009-12-03) 
			* WARNING: only add to the END of this set of filters. Other areas depend on the order.
			*
			*************************************************************************************************************/
			saryFilters.Add(GetDocbarPatientLWFilter(mktGraph, pCon, strPatientTempTable));
			saryFilters.Add(GetDocbarPatientFilterDateFrom(mktGraph));
			saryFilters.Add(GetDocbarPatientFilterDateTo(mktGraph));
			saryFilters.Add(GetDocbarPatientLocationFilter(mktGraph));
			saryFilters.Add(GetDocbarPatientProviderFilter(mktGraph));
			saryFilters.Add(GetDocbarConsultDateFromFilter(mktGraph));
			saryFilters.Add(GetDocbarConsultDateToFilter(mktGraph));
			saryFilters.Add(GetDocbarConsultLocation(mktGraph));
			saryFilters.Add(GetDocbarConsultResource(mktGraph));

			//(e.lally 2009-09-08) PLID 35429 - Now the lists of appointment type IDs
			CString strConsultTypeList1, strConsultTypeList2, strSurgeryTypeList;
			GetUserDefinedConsToProcAptTypeLists(strConsultTypeList1, strConsultTypeList2, strSurgeryTypeList);
			saryFilters.Add(strConsultTypeList1);
			saryFilters.Add(strConsultTypeList2);
			saryFilters.Add(strSurgeryTypeList);


			//(e.lally 2009-09-11) PLID 35521 - Added filter for procedure filtering, 
				//only the PROCConsultToSurg and CNVConstoSurgByDate graphs support it right now though
			//(e.lally 2009-12-03) PLID 35521 - We must always add this and let the local tab code handle updating this index in the array
				//since the filter is not part of the docbar yet.
			saryFilters.Add("");//12

			//(e.lally 2009-09-24) PLID 35592 - Multi-referral date filters
			CString strReferralDateFrom, strReferralDateTo;
			if(mktGraph == CNVConsToSurgByRefSour){
				strReferralDateFrom = GetDocbarReferralDateFromFilter(mktGraph);
				strReferralDateTo = GetDocbarReferralDateToFilter(mktGraph);
			}
			saryFilters.Add(strReferralDateFrom);
			saryFilters.Add(strReferralDateTo);

			//(e.lally 2009-09-28) PLID 35594 - Added filter for primary and multi referral filtering, 
				//only the referral graph (CNVConsToSurgByRefSour) supports it right now though
			//(e.lally 2009-12-03) PLID 35521 - We must always add these and let the local tab code handle updating these indexes in the array
				//since the filters are not part of the docbar yet.
			saryFilters.Add("");//15
			saryFilters.Add("");//16

			break;
		}
		default:
			ASSERT(FALSE);
			break;
	}
}

//(e.lally 2009-08-25) PLID 35297 - Returns the filter string for the list of patients requested to be filtered on in the docbar
// (j.jones 2010-07-19 15:27) - PLID 39053 - require a connection pointer
CString GetDocbarPatientLWFilter(MarketGraphType mktGraph, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable)
{
	if(GetMainFrame() == NULL || GetMainFrame()->m_pDocToolBar == NULL){
		ASSERT(FALSE);
		ThrowNxException("GetDocbarPatientLWFilter : Invalid MainFrame pointer.");
	}
	CString strOut;
	CString strPatientFilter = GetMainFrame()->m_pDocToolBar->GetPatientFilterString(pCon, strPatientTempTable);
	if (strPatientFilter != ""){
		switch(mktGraph){
			case PROCConsultToSurg:
			case CNVConsToSurgByPatCoord:
			case CNVConstoSurgByDate:
			case CNVConsToSurgByRefSour:
				strOut.Format(" AND PatientsT.PersonID IN %s ", strPatientFilter);
				break;
			default:
				ASSERT(FALSE);
				break;
		}
	}
	return strOut;
}

//(e.lally 2009-08-25) PLID 35297 - Returns the filter string for the date range (From) to filter patients on, set in the docbar
CString GetDocbarPatientFilterDateFrom(MarketGraphType mktGraph)
{
	if(GetMainFrame() == NULL || GetMainFrame()->m_pDocToolBar == NULL){
		ASSERT(FALSE);
		ThrowNxException("GetDocbarPatientFilterDateFrom : Invalid MainFrame pointer.");
	}
	CString strOut;
	CString strDateFrom = GetMainFrame()->m_pDocToolBar->GetFromDate();
	MarketFilter mfDateFilter = GetMainFrame()->m_pDocToolBar->GetFilterType(mftDate);

	if (GetMainFrame()->m_pDocToolBar->UseFilter(mftDate) && mfDateFilter == mfFirstContactDate) {
		switch(mktGraph){
			case PROCConsultToSurg:
			case CNVConsToSurgByPatCoord: 
			case CNVConstoSurgByDate:
			case CNVConsToSurgByRefSour:
				strOut.Format(" AND PersonT.FirstContactDate >= '%s' ", strDateFrom);
				break;
			default:
				ASSERT(FALSE);
				break;
		}
	}
	return strOut;
}

//(e.lally 2009-08-25) PLID 35297 - Returns the filter string for the date range (To) to filter patients on, set in the docbar
CString GetDocbarPatientFilterDateTo(MarketGraphType mktGraph)
{
	if(GetMainFrame() == NULL || GetMainFrame()->m_pDocToolBar == NULL){
		ASSERT(FALSE);
		ThrowNxException("GetDocbarPatientFilterDateTo : Invalid MainFrame pointer.");
	}
	CString strOut;
	CString strDateTo = GetMainFrame()->m_pDocToolBar->GetToDate();
	MarketFilter mfDateFilter = GetMainFrame()->m_pDocToolBar->GetFilterType(mftDate);
	if (GetMainFrame()->m_pDocToolBar->UseFilter(mftDate) && mfDateFilter == mfFirstContactDate) {
		switch(mktGraph){
			case PROCConsultToSurg:
			case CNVConsToSurgByPatCoord:
			case CNVConstoSurgByDate:
			case CNVConsToSurgByRefSour:
				strOut.Format(" AND PersonT.FirstContactDate < DATEADD(day,1,'%s') ", strDateTo);
				break;
			default:
				ASSERT(FALSE);
				break;
		}
	}
	return strOut;
}

//(e.lally 2009-08-25) PLID 35297 - Returns the filter string for the locations to filter patients on, set in the docbar
CString GetDocbarPatientLocationFilter(MarketGraphType mktGraph)
{
	if(GetMainFrame() == NULL || GetMainFrame()->m_pDocToolBar == NULL){
		ASSERT(FALSE);
		ThrowNxException("GetDocbarPatientLocationFilter : Invalid MainFrame pointer.");
	}
	CString strOut;
	CString strLocationIDs = GetMainFrame()->m_pDocToolBar->GetLocationString();
	MarketFilter mfLocationFilter = GetMainFrame()->m_pDocToolBar->GetFilterType(mftLocation);

	if (GetMainFrame()->m_pDocToolBar->UseFilter(mftLocation)) {
		ASSERT(!strLocationIDs.IsEmpty());
		BOOL bFilterPatientLocation = FALSE;
		//Make sure the patient location filter is one in use
		switch(mfLocationFilter){
			case mfPatApptLocation:
			case mfPatNoApptLocation:
				bFilterPatientLocation = TRUE;
				break;
			case mfNoPatApptLocation:
				break;
			default:
				//They used a filter we don't support!
				ASSERT(FALSE);
				break;
		}
		if(bFilterPatientLocation){
			switch(mktGraph){
				case PROCConsultToSurg:
				case CNVConsToSurgByPatCoord:
				case CNVConstoSurgByDate:
				case CNVConsToSurgByRefSour:
					strOut.Format(" AND PersonT.Location IN %s ", strLocationIDs);
					break;
				default:
					ASSERT(FALSE);
					break;
			}
		}
	}
	return strOut;
}

//(e.lally 2009-08-25) PLID 35297 - Returns the filter string for the providers to filter patients on, set in the docbar
CString GetDocbarPatientProviderFilter(MarketGraphType mktGraph)
{
	if(GetMainFrame() == NULL || GetMainFrame()->m_pDocToolBar == NULL){
		ASSERT(FALSE);
		ThrowNxException("GetDocbarPatientProviderFilter : Invalid MainFrame pointer.");
	}
	CString strOut;
	CString strProviderIDs = GetMainFrame()->m_pDocToolBar->GetProviderString();
	MarketFilter mfProviderFilter = GetMainFrame()->m_pDocToolBar->GetFilterType(mftProvider);

	if (GetMainFrame()->m_pDocToolBar->UseFilter(mftProvider))  {
		ASSERT(!strProviderIDs.IsEmpty());
		BOOL bFilterPatientProvider = FALSE;
		//Make sure the patient provider filter is one in use
		switch(mfProviderFilter){
			case mfNoPatApptProvider:
				break;
			case mfPatNoApptProvider:
				bFilterPatientProvider = TRUE;
				break;
			default:
				//They used a filter we don't support!
				ASSERT(FALSE);
				break;
		}

		if(bFilterPatientProvider) {
			switch(mktGraph){
				case PROCConsultToSurg:
				case CNVConsToSurgByPatCoord:
				case CNVConstoSurgByDate:
				case CNVConsToSurgByRefSour:
					strOut.Format(" AND PatientsT.MainPhysician IN %s ", strProviderIDs);
					break;
				default:
					ASSERT(FALSE);
					break;
			}
		}
	}
	return strOut;
}

//(e.lally 2009-08-25) PLID 35297 - Returns the filter string for the date range (From) to filter consult appointments on, set in the docbar
CString GetDocbarConsultDateFromFilter(MarketGraphType mktGraph)
{
	if(GetMainFrame() == NULL || GetMainFrame()->m_pDocToolBar == NULL){
		ASSERT(FALSE);
		ThrowNxException("GetDocbarConsultDateFromFilter : Invalid MainFrame pointer.");
	}
	CString strOut;
	CString strDateFrom = GetMainFrame()->m_pDocToolBar->GetFromDate();
	CString strDateTo = GetMainFrame()->m_pDocToolBar->GetToDate();
	MarketFilter mfDateFilter = GetMainFrame()->m_pDocToolBar->GetFilterType(mftDate);
	if (GetMainFrame()->m_pDocToolBar->UseFilter(mftDate) && mfDateFilter == mfConsultDate) {
		switch(mktGraph){
			case PROCConsultToSurg:
			case CNVConsToSurgByPatCoord: 
			case CNVConstoSurgByDate:
			case CNVConsToSurgByRefSour:{
				strOut.Format(" AND AppointmentsT.Date >= '%s' ", strDateFrom);
				break;
			}
			default:
				ASSERT(FALSE);
				break;
		}
	}
	return strOut;
}

//(e.lally 2009-08-25) PLID 35297 - Returns the filter string for the date range (To) to filter consult appointments on, set in the docbar
CString GetDocbarConsultDateToFilter(MarketGraphType mktGraph)
{
	if(GetMainFrame() == NULL || GetMainFrame()->m_pDocToolBar == NULL){
		ASSERT(FALSE);
		ThrowNxException("GetDocbarConsultDateToFilter : Invalid MainFrame pointer.");
	}
	CString strOut;
	CString strDateTo = GetMainFrame()->m_pDocToolBar->GetToDate();
	MarketFilter mfDateFilter = GetMainFrame()->m_pDocToolBar->GetFilterType(mftDate);

	if (GetMainFrame()->m_pDocToolBar->UseFilter(mftDate) && mfDateFilter == mfConsultDate) {
		switch(mktGraph){
			case PROCConsultToSurg:
			case CNVConsToSurgByPatCoord:
			case CNVConstoSurgByDate:
			case CNVConsToSurgByRefSour:
				strOut.Format(" AND AppointmentsT.Date < DATEADD(day,1,'%s') ", strDateTo);
				break;
			default:
				ASSERT(FALSE);
				break;
		}
	}
	return strOut;
}

//(e.lally 2009-08-25) PLID 35297 - Returns the filter string for the locations to filter consult appointments on, set in the docbar
CString GetDocbarConsultLocation(MarketGraphType mktGraph)
{
	if(GetMainFrame() == NULL || GetMainFrame()->m_pDocToolBar == NULL){
		ASSERT(FALSE);
		ThrowNxException("GetDocbarConsultLocation : Invalid MainFrame pointer.");
	}
	CString strOut;
	CString strLocationIDs = GetMainFrame()->m_pDocToolBar->GetLocationString();
	MarketFilter mfLocationFilter = GetMainFrame()->m_pDocToolBar->GetFilterType(mftLocation);

	if (GetMainFrame()->m_pDocToolBar->UseFilter(mftLocation)) {
		ASSERT(!strLocationIDs.IsEmpty());
		BOOL bFilterApptLocation = FALSE;
		//Make sure the appointment location filter is one in use
		switch(mfLocationFilter){
			case mfPatApptLocation:
			case mfNoPatApptLocation:
				bFilterApptLocation = TRUE;
				break;
			case mfPatNoApptLocation:
				break;
			default:
				//They used a filter we don't support!
				ASSERT(FALSE);
				break;
		}
		if(bFilterApptLocation){
			switch(mktGraph){
				case PROCConsultToSurg:
				case CNVConsToSurgByPatCoord:
				case CNVConstoSurgByDate:
				case CNVConsToSurgByRefSour:
					strOut.Format(" AND AppointmentsT.LocationID IN %s ", strLocationIDs);
					break;
				default:
					ASSERT(FALSE);
					break;
			}
		}
	}
	return strOut;
}

//(e.lally 2009-08-25) PLID 35297 - Returns the filter string for the resources to filter consult appointments on, set in the docbar
CString GetDocbarConsultResource(MarketGraphType mktGraph)
{
	if(GetMainFrame() == NULL || GetMainFrame()->m_pDocToolBar == NULL){
		ASSERT(FALSE);
		ThrowNxException("GetDocbarConsultResource : Invalid MainFrame pointer.");
	}
	CString strOut;
	CString strProviderIDs = GetMainFrame()->m_pDocToolBar->GetProviderString();
	MarketFilter mfProviderFilter = GetMainFrame()->m_pDocToolBar->GetFilterType(mftProvider);

	if (GetMainFrame()->m_pDocToolBar->UseFilter(mftProvider))  {
		ASSERT(!strProviderIDs.IsEmpty());
		BOOL bFilterApptResource = FALSE;
		//Make sure the appointment resource filter is one in use
		switch(mfProviderFilter){
			case mfNoPatApptProvider:
				bFilterApptResource = TRUE;
				break;
			case mfPatNoApptProvider:
				break;
			default:
				//They used a filter we don't support!
				ASSERT(FALSE);
				break;
		}

		if(bFilterApptResource) {
			switch(mktGraph){
				case PROCConsultToSurg:
				case CNVConsToSurgByPatCoord:
				case CNVConstoSurgByDate:
				case CNVConsToSurgByRefSour:
					strOut.Format(" AND AppointmentsT.ID IN("
						"SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN %s ) ", strProviderIDs);
					break;
				default:
					ASSERT(FALSE);
					break;
			}
		}
	}
	return strOut;
}

//(e.lally 2009-09-24) PLID 35592 - Returns the filter string for the date range (From) to filter multi referral source date on, set in the docbar
CString GetDocbarReferralDateFromFilter(MarketGraphType mktGraph)
{
	if(GetMainFrame() == NULL || GetMainFrame()->m_pDocToolBar == NULL){
		ASSERT(FALSE);
		ThrowNxException("GetDocbarReferralDateFromFilter : Invalid MainFrame pointer.");
	}
	CString strOut;
	CString strDateFrom = GetMainFrame()->m_pDocToolBar->GetFromDate();
	CString strDateTo = GetMainFrame()->m_pDocToolBar->GetToDate();
	MarketFilter mfDateFilter = GetMainFrame()->m_pDocToolBar->GetFilterType(mftDate);
	if (GetMainFrame()->m_pDocToolBar->UseFilter(mftDate) && mfDateFilter == mfReferralDate) {
		switch(mktGraph){
			case CNVConsToSurgByRefSour:{
				strOut.Format(" AND MultiReferralsT.Date >= '%s' ", strDateFrom);
				break;
			}
			default:
				ASSERT(FALSE);
				break;
		}
	}
	return strOut;
}

//(e.lally 2009-09-24) PLID 35592 - Returns the filter string for the date range (To) to filter multi referral source date on, set in the docbar
CString GetDocbarReferralDateToFilter(MarketGraphType mktGraph)
{
	if(GetMainFrame() == NULL || GetMainFrame()->m_pDocToolBar == NULL){
		ASSERT(FALSE);
		ThrowNxException("GetDocbarReferralDateToFilter : Invalid MainFrame pointer.");
	}
	CString strOut;
	CString strDateTo = GetMainFrame()->m_pDocToolBar->GetToDate();
	MarketFilter mfDateFilter = GetMainFrame()->m_pDocToolBar->GetFilterType(mftDate);

	if (GetMainFrame()->m_pDocToolBar->UseFilter(mftDate) && mfDateFilter == mfReferralDate) {
		switch(mktGraph){
			case CNVConsToSurgByRefSour:
				strOut.Format(" AND MultiReferralsT.Date < DATEADD(day,1,'%s') ", strDateTo);
				break;
			default:
				ASSERT(FALSE);
				break;
		}
	}
	return strOut;
}

bool DoesGraphSupportFilter(MarketGraphType graph, MarketFilter filter, MarketFilterType mft /*=mftDate*/)
{
	switch(graph) {

		case CNVProsToSurgByProc:
			if(filter == mfFirstContactDate)
				return true;
		break;

		
		
		/*************************************GRAPHS OF THE REFERRALS TAB **********************************************/
		//Revenue by Referral
		case EFFMonByRefSour:
			if(filter == mfFirstContactDate ||
				filter == mfPaymentDate ||
				filter == mfChargeDate ||
				filter == mfTransCostLocation ||
				filter == mfPatCostLocation ||
				filter == mfPatNoCostLocation ||
				filter == mfTransNoCostLocation ||
				filter == mfTransProvider ||
				filter == mfPatientProvider)
				return true;
		break;

		//Consult to Procedure By Referral
		case CNVConsToSurgByRefSour:
			if(filter == mfFirstContactDate ||
			   filter == mfConsultDate ||			// (d.thompson 2009-08-27) - PLID 35050 - Support Consult Date
			   filter == mfReferralDate ||			//(e.lally 2009-09-24) PLID 35592 - Support Referral Date
			   filter == mfNoPatApptLocation ||
			   filter == mfPatNoApptLocation ||
			   filter == mfPatApptLocation ||
			   filter == mfPatNoApptProvider ||
			   filter == mfNoPatApptProvider)
				return true;
		break;

		//Patients By Referral Source
		case NUMPatByRefSour:
			if(filter == mfFirstContactDate ||
			   filter == mfPatNoApptLocation ||
			   filter == mfNoPatApptLocation ||
			   filter == mfPatApptLocation ||
			   filter == mfNoPatApptProvider ||
			   filter == mfPatNoApptProvider ||
			   filter == mfReferralDate)	//DRT 5/8/2008 - PLID 29966 - Added referral date filter
				return true;
		break;

		//Cancellations and No Shows By Referrals
		case REFNoShowByReferral:
			if(filter == mfFirstContactDate ||
				filter == mfApptDate ||
				filter == mfApptLocation ||
				filter == mfPatientLocation ||
				filter == mfApptProvider ||
				filter == mfPatientProvider)
				return true;
		break;

		//Inquiry to Consult By Referral
		case REFInqtoCons:
			if(filter == mfFirstContactDate ||
			   filter == mfPatNoApptLocation ||
			   filter == mfNoPatApptLocation ||
			   filter == mfPatApptLocation ||
			   filter == mfPatNoApptProvider ||
			   filter == mfNoPatApptProvider)
				return true;
		break;
		
		case REFProsToCons:
			if(filter == mfFirstContactDate ||
				filter == mfPatNoApptLocation ||
				filter == mfNoPatApptLocation ||
				filter == mfPatApptLocation ||
				filter == mfPatNoApptProvider ||
				filter == mfNoPatApptProvider)

			return true;
		break;

		case REFSchedVClosed:
			if(filter == mfFirstContactDate ||
				filter == mfApptInputDate ||
				filter == mfApptDate ||
				filter == mfPatientLocation ||
				filter == mfApptLocation ||
				filter == mfApptProvider ||
				filter == mfPatientProvider)
				return true;
		break;
		

		/*************************************END GRAPHS OF THE REFERRAL TAB****************************************/



		/****************************************GRAPHS OF THE PROCEDURE TAB*************************************************/
		/*Revenue By Procedure*/
		case EFFMonByProc:
			if(filter == mfFirstContactDate ||
				filter == mfChargeDate ||
				filter == mfPaymentDate ||
				filter == mfChargeLocation ||
				filter == mfPayLocation ||
				filter == mfPatientLocation  ||
				filter == mfChargeProvider ||
				filter == mfPayProvider ||
				filter == mfPatientProvider)
				return true;
		break;

		/*Consult to Procedure By Procedure*/
		case PROCConsultToSurg:
			if (filter == mfFirstContactDate ||
				filter == mfConsultDate ||
				filter == mfPatNoApptLocation ||
				filter == mfNoPatApptLocation ||
				filter == mfPatApptLocation ||
				filter == mfPatNoApptProvider ||
				filter == mfNoPatApptProvider) 
				return true;
		break;

		/*Patients By Procedure*/
		case PROCPatients:
			if(filter == mfFirstContactDate ||
			   filter == mfPatientLocation ||
			   filter == mfPatientProvider)
			   return true;
		break;

		/*Cancellations and No Shows By Procedure*/	
		case NUMNoShowByProc:
			if(filter == mfFirstContactDate ||
				filter == mfApptDate || 
				filter == mfApptLocation ||
				filter == mfPatientLocation ||
				filter == mfPatientProvider ||
				filter == mfApptProvider)
				return true;
		break;

		/*Inquiry To Consult By Procedure*/
		case CNVInqToConsByProc:
			if(filter == mfFirstContactDate ||
			   filter == mfPatNoApptLocation ||
			   filter == mfNoPatApptLocation ||
			   filter == mfPatApptLocation ||
			   filter == mfNoPatApptProvider ||
			   filter == mfPatNoApptProvider) 
				return true;
		break;

		/*Prospect to Consult By Procedure*/
		case CNVProsToConsByProc:
			if(filter == mfFirstContactDate ||
			   filter == mfPatNoApptLocation ||
			   filter == mfNoPatApptLocation ||
			   filter == mfPatApptLocation ||
			   filter == mfNoPatApptProvider ||
			   filter == mfPatNoApptProvider) 
				return true;
		break;

		/*Procedures Performed Vs. Closed By Procedure*/
		case PROCSchedVClosed:
			if(filter == mfFirstContactDate ||
				filter == mfApptInputDate ||
				filter == mfApptDate ||
				filter == mfApptLocation ||
				filter == mfPatientLocation ||
				filter == mfPatientProvider ||
				filter == mfApptProvider)
				return true;
		break;

		/*Revenue By Category*/
		case EFFMonByCategory:
			if(filter == mfFirstContactDate ||
				filter == mfChargeDate ||
				filter == mfPaymentDate ||
				filter == mfChargeLocation ||
				filter == mfPayLocation ||
				filter == mfChargeProvider ||
				filter == mfPayProvider ||
				filter == mfPatientProvider)
				return true;
		break;
		/*****************END GRAPHS OF THE PROCEDURE TAB***********************************************************/


		/*******************GRAPHS OF THE COORDINATOR TAB************************************************************/

		/*Revenue By Coordinator*/
		case EFFMonByPatCoord:
			if(filter == mfFirstContactDate ||
				filter == mfChargeDate ||
				filter == mfPaymentDate ||
				filter == mfChargeLocation ||
				filter == mfPayLocation ||
				filter == mfPatientLocation ||
				filter == mfPatientProvider ||
				filter == mfPayProvider ||
				filter == mfChargeProvider)
				return true;
		break;

		/*Consults to Procedures By Coordinator*/
		case CNVConsToSurgByPatCoord:
			if(filter == mfFirstContactDate || 
               filter == mfConsultDate ||
			   filter == mfPatNoApptLocation ||
			   filter == mfNoPatApptLocation ||
			   filter == mfPatApptLocation ||
			   filter == mfNoPatApptProvider ||
			   filter == mfPatNoApptProvider)

				return true;
		break;

		/*Patients By Coordinator*/
		case COORDPatients:
			if(filter == mfFirstContactDate ||
			   filter == mfPatientLocation ||
			   filter == mfPatientProvider)
				return true;
		break;

		/*Cancellations and No Shows By Coordinator*/
		case NUMCanByPatCoord:
			if(filter == mfFirstContactDate ||
				filter == mfApptDate ||
				filter == mfApptLocation ||
				filter == mfPatientLocation ||
				filter == mfApptProvider ||
				filter == mfPatientProvider)
				return true;
		break;


		/*Inquiry To Consult By Staff*/
		case CNVInqToConsByStaff:
			if(filter == mfFirstContactDate ||
			   filter == mfPatNoApptLocation ||
			   filter == mfNoPatApptLocation ||
			   filter == mfPatApptLocation ||
			   filter == mfPatNoApptProvider ||
			   filter == mfNoPatApptProvider)
				return true;	
			
		break;

		/*Prospects To Consults By Coordinator*/
		case COORDProsToCons:
			if(filter == mfFirstContactDate ||
			   filter == mfPatNoApptLocation ||
			   filter == mfNoPatApptLocation ||
			   filter == mfPatApptLocation ||
			   filter == mfPatNoApptProvider ||
			   filter == mfNoPatApptProvider)
				return true;
		break;

		/*Procedures Performed Vs Closed By Coordinator*/
		case COORDSchedVClosed:
			if(filter == mfFirstContactDate ||
				filter == mfApptInputDate ||
				filter == mfApptDate ||
				filter == mfApptLocation ||
				filter == mfPatientLocation ||
				filter == mfApptProvider ||
				filter == mfPatientProvider)
				return true;
		break;
/*********************************END GRAPHS OF THE COORDINATOR TAB**************************************************/

/*********************************GRAPHS OF THE DATE TAB*************************************************************/
		
		/*Revenue By Date*/
		case DATERevByDate:
			if(filter == mfFirstContactDate ||
				filter == mfPaymentDate ||
				filter == mfChargeDate ||
				filter == mfTransLocation ||
				filter == mfPatientLocation ||
				filter == mfTransProvider ||
				filter == mfPatientProvider)
				return true;
		break;

		/*Coversion Rate By Date*/
		case CNVConstoSurgByDate:
			if(filter == mfFirstContactDate ||
				filter == mfConsultDate ||
				filter == mfPatNoApptLocation ||
				filter == mfNoPatApptLocation ||
				filter == mfPatApptLocation ||
				filter == mfPatNoApptProvider ||
				filter == mfNoPatApptProvider)
				return true;
		break;

		/*Patients By Date*/
		case DATEPatients:
			if(filter == mfFirstContactDate ||
			   filter == mfPatientLocation ||
			   filter == mfPatientProvider)
				return true;
		break;

		/*Cancellations and No Shows By Date*/
		case DATENoShowCancel:
			if(filter == mfFirstContactDate ||
				filter == mfApptInputDate ||
				filter == mfApptDate ||
				filter == mfApptLocation ||
				filter == mfPatientLocation ||
				filter == mfApptProvider ||
				filter == mfPatientProvider)
				return true;
		break;

		/*Inquiries To Consults By Date*/
		case DATEInqToCons:
			if(filter == mfFirstContactDate ||
			   filter == mfPatNoApptLocation ||
			   filter == mfNoPatApptLocation ||
			   filter == mfPatApptLocation ||
			   filter == mfNoPatApptProvider ||
			   filter == mfPatNoApptProvider)
				return true;
		break;

		/*Prospects To Consults By Date*/
		case DATEProsToCons:
			if(filter == mfFirstContactDate ||
			   filter == mfPatNoApptLocation ||
			   filter == mfNoPatApptLocation ||
			   filter == mfPatApptLocation ||
			   filter == mfNoPatApptProvider ||
			   filter == mfPatNoApptProvider)
				return true;
		break;

		/*Procedures Performed Vs. Closed  By Date*/
		case DATECloseByProc:
			if(filter == mfFirstContactDate ||
				filter == mfApptInputDate ||
				filter == mfApptDate ||
				filter == mfApptLocation ||
				filter == mfPatientLocation ||
				filter == mfPatientProvider ||
				filter == mfApptProvider)
				return true;
		break;

		// (j.gruber 2011-05-03 16:37) - PLID 38153
		case DATEApptToCharge:
			if(filter == mfFirstContactDate ||
				filter == mfApptDate ||
				filter == mfApptLocation ||
				filter == mfPatientLocation ||
				filter == mfApptProvider ||
				filter == mfPatientProvider)
				return true;
		break;
/*******************************END GRAPHS OF THE DATE TAB***************************************************/


		case EFFProfByProc:
			if(filter == mfFirstContactDate ||
				filter == mfChargeDate)
				return true;
		break;

		case EFFProfByPatCoord:
			if(filter == mfFirstContactDate ||
				filter == mfChargeDate)
				return true;
		break;

		

		case EFFRefEffect:
			if(filter == mfFirstContactDate ||
				filter == mfChargeDate)
				return true;
		break;

		
		case NUMPatNoShowsByProc:
			if(filter == mfFirstContactDate ||
				filter == mfApptDate)
				return true;
		break;

		case NUMInqByProc:
			if(filter == mfFirstContactDate ||
				filter == mfApptDate)
				return true;
		break;

		
		
		case NUMNoShowbyPatCoord:
			if(filter == mfFirstContactDate ||
				filter == mfApptDate)
				return true;
		break;

		case NUMCanByProc:
			if(filter == mfFirstContactDate ||
				filter == mfApptDate)
				return true;
		break;

		case NUMCanByReason:
			if(filter == mfFirstContactDate ||
				filter == mfApptDate ||
				filter == mfPatientLocation ||
				filter == mfApptLocation ||
				filter == mfPatientProvider ||
				filter == mfApptProvider)
				return true;
		break;

		case NUMNoShowByReason:
			if(filter == mfFirstContactDate ||
				filter == mfApptDate ||
				filter == mfPatientLocation ||
				filter == mfApptLocation ||
				filter == mfPatientProvider ||
				filter == mfApptProvider)
				return true;
		break;

		case NUMConWithProsByPatCoord:
			if(filter == mfFirstContactDate ||
				filter == mfApptDate)
				return true;
		break;

		case NUMConsByPatCoord:
			if(filter == mfFirstContactDate ||
				filter == mfApptDate)
				return true;
		break;

		case NUMClosedByProc:
			if(filter == mfFirstContactDate ||
				filter == mfApptInputDate ||
				filter == mfApptDate)
				return true;
		break;

		case NUMPerfByProc:
			if(filter == mfFirstContactDate ||
				filter == mfApptDate)
				return true;
		break;

		
		case DATEPerfByProcs:
			if(filter == mfFirstContactDate ||
				filter == mfApptInputDate ||
				filter == mfApptDate ||
				filter == mfApptLocation ||
				filter == mfPatientLocation)
				return true;		
		break;

		case DATECountByProcs:
			if(filter == mfFirstContactDate ||
				filter == mfApptInputDate ||
				filter == mfApptDate)
				return true;
		break;

		case REFInqByReferral:
			if(filter == mfFirstContactDate ||
				filter == mfApptDate)
				return true;
		break;

		case RETENTIONGraph:
			if(filter == mfFirstContactDate ||
			   filter == mfPatientLocation ||
			   filter == mfPatientProvider ||
			   filter == mfApptLocation ||
			   filter == mfApptProvider)
				return true;
			break;

		case TRENDSGraph:
			if(filter == mfFirstContactDate)
				return true;
			break;

		case ZIPGraph:
			if(filter == mfFirstContactDate ||
			   filter == mfPatientLocation ||
			   filter == mfPatientProvider)
				return true;
			break;

		case BASELINEGraph:
			if (filter == mfApptDate ||
				filter == mfGraphDependant ||
				filter == mfDependantProvider)
				return true;
			break;
		case COSTS:
			if (filter == mfEffectivenessDate ||
				filter == mfCostDatePaid ||
				filter == mfCostLocation ||
				filter == mfDependantProvider) {
				return true;
			}
		break;
		case INTERNAL_IncPerCat:
		case INTERNAL_IncPerPerson:
		case INTERNAL_IncPerClient:
		case INTERNAL_OpenPerWeek:
			//The internal graphs use hardcoded date filters for now, I don't want to clutter up
			//	the normal code for our internal stuff.  Just say yes that we support everything.
			return true;
			break;

	}

	return false;
}

//JMM - 04/06/05 - I'm making this not just date filters anymore, bt basically any kind of filter
//since I'm adding location and provider filters and it'd be kinda silly to triplicate the code
CString GetFieldFromEnum(MarketFilter mf)
{
	switch(mf) {
		case mfFirstContactDate:
			return "PersonT.FirstContactDate";
		break;
		case mfChargeDate:
			return "LineChargesT.Date";
		break;
		case mfPaymentDate:
			return "LinePaymentsT.Date";
		break;
		case mfApptInputDate:
			return "AppointmentsT.CreatedDate";
		break;
		case mfApptDate:
			return "AppointmentsT.Date";
		break;
		case mfEffectivenessDate:
			return "MarketingCostsT.EffectivenessDate";
		break;

		case mfConsultDate:
			return "AppointmentsT.Date";
		break;

		case mfConsultInputDate:
			return "AppointmentsT.CreatedDate";
		break;


		case mfCostDatePaid:
			return "MarketingCostsT.DatePaid";
		break;

		case mfPatientLocation:
			if (GetType() == DATERevByDate) {
				return "PatLocID";
			}
			else {
				return "PersonT.Location";
			}
		break;

		case mfTransLocation:
			return "TransLocID";
		break;

		case mfApptLocation:
			return "AppointmentsT.LocationID";
		break;

		case mfCostLocation:
			return "MarketingCostsT.LocationID";
		break;

		case mfPatCostLocation:
			return "PatLocID\\MarketingCostsT.LocationID";
		break;

		case mfTransCostLocation:
			return "TransLocID\\MarketingCostsT.LocationID";
		break;

		case mfPatNoCostLocation:
			return "PatLocID\\<none>";
		break;

		case mfTransNoCostLocation:
			return "TransLocID\\<none>";
		break;

		case mfPatApptLocation:
			return "PersonT.Location\\AppointmentsT.LocationID";
		break;

		case mfNoPatApptLocation:
			return "<none>\\AppointmentsT.LocationID";
		break;

		case mfPatNoApptLocation:
			return "PersonT.Location\\<none>";
		break;

		case mfChargeLocation:
			return "LineChargesT.LocationID";
		break;

		case mfPayLocation:
			return "LinePaymentsT.LocationID";
		break;

		case mfGraphDependant:
			return "";
		break;

		case mfPatientProvider: {
			long nType = GetType();
			if (nType == EFFMonByRefSour || nType == DATERevByDate) {
				return "(PatProvID IN ";
			}
			else {
				return "(PatientsT.MainPhysician IN";
			}
								}
		break;

		case mfTransProvider:
			return "(TransProvID IN ";
		break;

		case mfApptProvider:
			return "AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN ";
		break;

/*		case mfPatApptProvider:
			return "PatientsT.MainPhysician\\AppointmentResourceT.ResourceID";
		break;*/

		case mfPatNoApptProvider:
			return "(PatientsT.MainPhysician IN \\<none>";
		break;

		case mfNoPatApptProvider:
			return "<none>\\AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN ";
		break;

		case mfChargeProvider:
			return "(ChargesT.DoctorsProviders IN ";
		break;

		case mfPayProvider:
			return "(PaymentsT.ProviderID IN ";
		break;

		case mfDependantProvider:
			return "Provider";
		break;

		//DRT 5/8/2008 - PLID 29966 - Added referral date
		//DRT 7/9/2008 - PLID 29966 - Switched it to a CASE statement filter.  All queries now should have a potential
		//	MultiReferralsT (it's a subquery in some cases) that can have a date.  But if there is no matched
		//	preference referral, we need to fall back to the FCD.
		case mfReferralDate:
			return "CASE WHEN MultiReferralsT.Date IS NULL THEN PersonT.FirstContactDate ELSE MultiReferralsT.Date END";
		break;

		

	}
	ASSERT(FALSE);
	return "";
}

// (j.jones 2010-07-19 15:27) - PLID 39053 - require a connection pointer
void GetParameters(CString &from, CString &to, CString &prov, CString &loc,CString &strPatCoord, CString &strDateField, CString &strLocationField, CString &strProvField, int &nCategory, int &nResp, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable)
{

	prov = GetMainFrame()->m_pDocToolBar->GetProviderString();
	
	
	strLocationField = GetMainFrame()->m_pDocToolBar->GetFilterField(mftLocation);

	strProvField = GetMainFrame()->m_pDocToolBar->GetFilterField(mftProvider);	
	
	loc = GetMainFrame()->m_pDocToolBar->GetLocationString();
	
	from = GetMainFrame()->m_pDocToolBar->GetFromDate();
	to = GetMainFrame()->m_pDocToolBar->GetToDate();
	
	strPatCoord = GetMainFrame()->m_pDocToolBar->GetPatientFilterString(pCon, strPatientTempTable);


	strDateField = GetMainFrame()->m_pDocToolBar->GetFilterField(mftDate);

	nCategory = GetMainFrame()->m_pDocToolBar->GetCategory();

	nResp = GetMainFrame()->m_pDocToolBar->GetResp();
	

}


//JMM - 04/06/05 - I'm making this not just date filters anymore, bt basically any kind of filter
//since I'm adding location and provider filters and it'd be kidan silly to triplicate the code
CString GetDisplayNameFromEnum(MarketFilter mf)
{
	switch(mf) {
		case mfFirstContactDate:
			return "First Contact Date";
		break;
		case mfChargeDate:
			return "Charge Date";
		break;
		case mfPaymentDate:
			return "Payment Date";
		break;
		case mfApptInputDate:
			return "Appt. Input Date";
		break;
		case mfApptDate:
			return "Appointment Date";
		break;
		case mfEffectivenessDate:
			return "Effectiveness Date";
		break;

		case mfConsultDate:
			return "Consult Date";
		break;

		case mfConsultInputDate:
			return "Consult Input Date";
		break;

		case mfCostDatePaid:
			return "Date Paid";
		break;

		case mfPatientLocation:
			return "Patient Location";
		break;

		case mfTransLocation:
			return "Transaction Location";
		break;

		case mfApptLocation:
			return "Appointment Location";
		break;

		case mfCostLocation:
			return "Cost Location";
		break;

		case mfPatCostLocation:
			return "Patient\\Cost Locations";
		break;

		case mfTransCostLocation:
			return "Transaction\\Cost Locations";
		break;

		case mfPatNoCostLocation:
			return "Patient\\<No Cost> Locations";
		break;

		case mfTransNoCostLocation:
			return "Charge\\<No Cost> Locations";
		break;
		case mfPatApptLocation:
			return "Patient and Appointment Locations";
		break;		

		case mfNoPatApptLocation:
			return "Only Appointment Location";
		break;		

		case mfPatNoApptLocation:
			return "Only Patient Location";
		break;		

		case mfChargeLocation:
			return "Charge Location";
		break;		

		case mfPayLocation:
			return "Payment Location";
		break;		

		case mfGraphDependant:
			return "Graph Location";
		break;		

		case mfPatientProvider:
			return "Patient Provider";
		break;

		case mfTransProvider:
			return "Transaction Provider";
		break;

		case mfApptProvider:
			return "Appointment Resource";
		break;

/*		case mfPatApptProvider:
			return "Patient Provider and Appt. Resource";
		break;*/

		case mfPatNoApptProvider:
			return "Patient Provider";
		break;

		case mfNoPatApptProvider:
			return "Appointment Resource";
		break;

		case mfChargeProvider:
			return "Charge Provider";
		break;

		case mfPayProvider:
			return "Payment Provider";
		break;

		case mfDependantProvider:
			return "Provider";
		break;

		//DRT 5/8/2008 - PLID 29966 - Referral Date filter
		case mfReferralDate:
			return "Referral Date";
		break;
		
	}
	ASSERT(FALSE);
	return "";
}

int GetType() 
{
	return GetMainFrame()->m_pDocToolBar->GetType();
}


//at the moment, this only parses 2 fiels because thats the max we have right now
BOOL ParseMultiFilter(CString strCompleteFilter, CString &strFilter1, CString &strFilter2) {

	long nResult = strCompleteFilter.Find("\\");
	if (nResult == -1) {
		//assert because we have multiple field we need to fill
		ASSERT(FALSE);
		return FALSE;
	}
	else {
		CString strLocField1, strLocField2;
		strFilter1 = strCompleteFilter.Left(nResult);
		strFilter2 = strCompleteFilter.Right(strCompleteFilter.GetLength() - (nResult + 1));
		
		//get rid of the <none> if it exists
		strFilter1.Replace("<none>", "");
		strFilter2.Replace("<none>", "");

		return TRUE;

	}

}


BOOL HasMultiFilter(CString strCompleteFilter) {
	
	long nResult = strCompleteFilter.Find("\\");
	if (nResult == -1) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}


////////////a.walling PLID 20695 5/18/06/////////////////////////
// CMarketRenderButtonStatus is a simple function that changes the Render button
// while rendering and then reverts the changes when it goes out of scope.
/////////////////////////////////////////////////////////////////

CMarketRenderButtonStatus::CMarketRenderButtonStatus(CWnd *twnd, bool bManualHide) {
	m_bManualHide = bManualHide;
	wnd = twnd;
	wnd->SetDlgItemText(IDC_GO, "Loading...");
	wnd->GetDlgItem(IDC_GO)->EnableWindow(FALSE);
	wnd->GetDlgItem(IDC_GO)->RedrawWindow();
}

CMarketRenderButtonStatus::~CMarketRenderButtonStatus() {
	if (!m_bManualHide)
		HideGo();
}

void CMarketRenderButtonStatus::HideGo() {
	ManualHide(wnd);
}

void CMarketRenderButtonStatus::ManualHide(CWnd *lwnd) {
	lwnd->GetDlgItem(IDC_GO)->ShowWindow(SW_HIDE);
	lwnd->SetDlgItemText(IDC_GO, "Reload Graph");
	lwnd->GetDlgItem(IDC_GO)->EnableWindow(TRUE);
}

//(e.lally 2009-09-24) PLID 35526 - Created a more modular way to set the marketing bar graph colors 
	//so we can more easily update them going forward. Also makes understanding the coloring more readable.
OLE_COLOR GetMarketGraphColor(MarketingGraphColors mgcColor)
{
	switch(mgcColor){
		case mgcBrightRed:
			return RGB(255,0,0);
		case mgcBrightGreen:
			return RGB(50,200,50);
		case mgcBrightBlue:
			return RGB(20,111,255);
		case mgcBrightOrange:
			return RGB(255,185,44);
		case mgcBrightPurple:
			return RGB(122, 122, 255);
		case mgcDarkRed:
			return RGB(190,5,5);
		case mgcDarkGreen:
			return RGB(0,164,66);
		case mgcDarkBlue:
			return RGB(49,106,197);
		case mgcLightRed:
			return RGB(255,185,185);
		case mgcLightGreen:
			return RGB(102,204,102);
		case mgcLightBlue:
			return RGB(153,204,255);
		case mgcLightPurple:
			return RGB(180,180,255);
		case mgcLightOrange:
			return RGB(250,200,145);
		case mgcTurquoise:
			return RGB(100,193,151);
		case mgcTan:
			return RGB(212,204,148);
		default:
			ASSERT(FALSE);
			return RGB(255, 255, 255); //White
	}
}