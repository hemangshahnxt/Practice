// Contacts.cpp: implementation of the CContacts class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "practice.h"
#include "Contacts.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace ADODB;

long GetDefaultProviderID()
{
	long nAns = -1;
	_RecordsetPtr rsTemp;
	try {

		//find the default location, otherwise use the current location
		long LocationID = GetCurrentLocationID();
		rsTemp = CreateRecordset("SELECT ID FROM LocationsT WHERE Managed = 1 AND Active = 1 AND IsDefault = 1 AND TypeID = 1");
		if(!rsTemp->eof) {
			LocationID = AdoFldLong(rsTemp, "ID",GetCurrentLocationID());
		}
		rsTemp->Close();

		// Open the recordset as read-only
		rsTemp = CreateRecordset("SELECT DefaultProviderID FROM LocationsT WHERE LocationsT.ID = %li", LocationID);
		
		// Make sure there is at least one record
		if (!rsTemp->eof ) {
			// Get the value out of the one and only field (DefaultProviderID)
			_variant_t var;
			var = rsTemp->Fields->GetItem("DefaultProviderID")->Value;
			if (var.vt != VT_NULL && var.vt!= VT_EMPTY) {
				// If the value is valid, return it
				nAns = var.lVal;
			}
		}
		rsTemp->Close();
	} catch (_com_error e) {
		// If there was an error, just silently return a bogus ID
		nAns = -1;
	}

	// Make sure the recordset gets closed
	if (rsTemp->GetState() == adStateOpen) {
		rsTemp->Close();
	}

	// Return the answer
	return nAns;
}

long GetDefaultCoordinatorID()
{
	long nAns = -1;
	_RecordsetPtr rsTemp;
	try {
		// Open the recordset as read-only
		rsTemp = CreateRecordset("SELECT DefaultCoordinatorID "
			"FROM LocationsT WHERE LocationsT.ID = %li", GetCurrentLocation());
		
		// Make sure there is at least one record
		if (!rsTemp->eof ) {
			// Get the value out of the one and only field (DefaultCoordinatorID)
			_variant_t var;
			var = rsTemp->Fields->GetItem("DefaultCoordinatorID")->Value;
			if (var.vt != VT_NULL && var.vt!= VT_EMPTY) {
				// If the value is valid, return it
				nAns = var.lVal;
			}
		}
		rsTemp->Close();
	} catch (_com_error e) {
		// If there was an error, just silently return a bogus ID
		nAns = -1;
	}

	// Make sure the recordset gets closed
	if (rsTemp->GetState() == adStateOpen) {
		rsTemp->Close();
	}

	// Return the answer
	return nAns;
}