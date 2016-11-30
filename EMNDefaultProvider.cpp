// (d.thompson 2013-08-16) - PLID 57809 - Class to encapsulate all the (confusing) logic for calculating
//	the default EMN providers (primary and secondary).  All this code was previously in CEMN::LoadDefaultProviderIDs()
#include "stdafx.h"
#include "EMNDefaultProvider.h"
#include "NxAdo.h"
#include "EMR.h"
#include "EMN.h"

using namespace ADODB;

namespace Emr
{
	namespace DefaultProvider
	{
		enum Type
		{
			  MainPhysician = 0
			, Appointment = 1
			, None = 2
			, CreatedUser = 3 // (j.luckoski 2013-04-01 16:27) - PLID 55910
		};
	}
}

CEMNDefaultProvider::CEMNDefaultProvider(CEMR *pEMR, CEMN *pEMN)
{
	m_pEMR = pEMR;
	m_pEMN = pEMN;
}

CEMNDefaultProvider::~CEMNDefaultProvider(void)
{
}

//	This function goes to the licensing and asks for all the licensed provider IDs.  We then load them
//	in the 'licensed' parameter for your pleasure.
void CEMNDefaultProvider::FillLicensedEMRProviders(OUT ProviderSet &licensed)
{
	CDWordArray dwaLicensedProviders;
	g_pLicense->GetUsedEMRProviders(dwaLicensedProviders);
	foreach(DWORD dw, dwaLicensedProviders) {
		licensed.insert((long)dw);
	}
}

//Finds all providers linked to the currently logged in user
void CEMNDefaultProvider::FindCurrentUserProviders(IN OUT ProviderSet &setProviders)
{
	_RecordsetPtr prs = CreateParamRecordset("SELECT ProviderID from UsersT INNER JOIN PersonT on UsersT.ProviderID = PersonT.ID WHERE  PersonT.Archived = 0 AND UsersT.PersonID = {INT}", GetCurrentUserID());
	while (!prs->eof) {
		setProviders.insert(AdoFldLong(prs, "ProviderID"));
		prs->MoveNext();
	}
}

//Load all providers available on the appointment linked to this EMN.
void CEMNDefaultProvider::FindLinkedAppointmentProviders(IN OUT ProviderSet &setApptProviders)
{
	if(m_pEMR && m_pEMN) {
		EMNAppointment appt = m_pEMN->GetAppointment();

		// first check the appointment if we were passed one
		if (-1 != appt.nID) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT ResourceProviderLinkT.ProviderID "
				"FROM ResourceProviderLinkT "
				"INNER JOIN AppointmentResourceT ON ResourceProviderLinkT.ResourceID = AppointmentResourceT.ResourceID "
				"INNER JOIN PersonT ON ResourceProviderLinkT.ProviderID = PersonT.ID "
				"INNER JOIN AppointmentsT ON AppointmentResourceT.AppointmentID = AppointmentsT.ID "
				"WHERE ResourceProviderLinkT.ProviderID Is Not Null "
				"AND PersonT.Archived = 0 "			
				"AND AppointmentsT.ID = {INT} "		
				"AND AppointmentsT.PatientID = {INT} "
				"AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 "
				"GROUP BY ResourceProviderLinkT.ProviderID", appt.nID, m_pEMR->GetPatientID());
			while (!rs->eof) {
				setApptProviders.insert(AdoFldLong(rs, "ProviderID"));
				rs->MoveNext();
			}
		}
	}
	else {
		//No EMR, no patient.
		//No EMN, no appointment.
	}

	// (d.thompson 2013-05-20) - PLID 56794 - Before now, if no provider is found, we would just fall back and
	//	pick every provider the patient is seeing today.  This should not happen.  If the user has an appointment
	//	selected, we should tie it to that provider.  If there isn't one, then we shouldn't load a provider at all.
}

//Look at the previous EMN on this EMR.  If there is one, get its primary providers.
void CEMNDefaultProvider::FindPreviousEMNPrimaryProviders(IN OUT ProviderSet &setProviders)
{
	if(m_pEMR && m_pEMR->GetEMNCount() > 0) {
		CEMN *pLastEMN = m_pEMR->GetEMN(m_pEMR->GetEMNCount() - 1);
		if(pLastEMN) {
			CArray<long,long> arProviders;
			pLastEMN->GetProviders(arProviders);
			setProviders.insert(boost::begin(arProviders), boost::end(arProviders));
		}
	}
}

//Lookup the patient's general 1 provider
//	This remains a set to be consistent with the rest of the licensing use, but at present, this can only ever be at most 1 item.
void CEMNDefaultProvider::FindGeneral1Provider(IN OUT ProviderSet &g1Provider)
{
	if(m_pEMR) {
		_RecordsetPtr rs = CreateParamRecordset("SELECT MainPhysician FROM PatientsT WHERE PersonID = {INT} AND MainPhysician IS NOT NULL", m_pEMR->GetPatientID());
		if (!rs->eof) {
			g1Provider.insert(AdoFldLong(rs, "MainPhysician"));
		}
	}
	else {
		//We have no EMR???  Well, we can't get the patient then.  Therefore there is no g1 provider for no patient.
	}
}

void CEMNDefaultProvider::CalculateDefaultEMNProviders(OUT CArray<long,long> &arMainProviderIDs, OUT CArray<long,long> &arySecondaryProviders)
{
	//TES 12/22/2006 - PLID 23398 - Don't default to a non-licensed provider.
	//TES 12/26/2006 - PLID 23400 - Updated function to potentially return multiple providers.
	// (a.walling 2013-01-22 10:00) - PLID 55079 - Refactored a bit. Use linked appointment's provider(s) if available, fallback to old search if not
	// (a.walling 2013-02-15 09:55) - PLID 55079 - Only use the appointment if the preference is set to do so
	// (d.thompson 2013-08-14) - PLID 57809 - Further updated logic, heavily updated the comments throughout this function.  I also refactored so that 
	//	we do all the "loading possible sets" first, and "figure out what to actually choose" second.  Makes life easier than mixing it all together.
	//	I also renamed a couple of sets for readability.
	//Here is the new logic:  (please keep this in sync with NexTech.Practice - LoadEMNDefaultProviderIDs)
	//
	//	1)  Evaluate the "provider(s)" to be selected.  We don't care about primary or secondary.  This is done by looking at the 
	//		'EMNDefaultProviderType' preference and choosing based on its configuration.
	//	2)  Based on the providers selected, determine which are licensed primary providers and which are secondary providers.
	//	3)  Include the secondary provider from the preference 'EMNDefaultSecondaryProviderType'
	//	4)  Follow the 'Link EMN Secondaries' options (Tools -> Licensing) to all the secondary providers, which may add more primary providers.
	//	5)  Apply the "1 and only 1" rule -- if this office has only 1 licensed provider allowed, just use them, always.
	//	6)  LEGACY.  If we still don't have a primary provider chosen, look at the last EMN on this EMR and use whatever it had.
	//	7)	If there are still no primary providers, evaluate the "fallback" preference 'EMNDefaultProviderType_Fallback'.  This will tell us
	//		if we should just leave it as-is, or if we should load the general 1 provider (if they are a primary provider - we do not
	//		choose them if they are a secondary provider only).

	//What preference is enabled?
	Emr::DefaultProvider::Type defaultType = (Emr::DefaultProvider::Type)GetRemotePropertyInt("EMNDefaultProviderType", Emr::DefaultProvider::MainPhysician, 0, "<None>", true);

	//
	//Step 1.  Evaluate the "provider(s)" to be selected.  We don't care about primary or secondary.  This is done by looking at the 
	//		'EMNDefaultProviderType' preference and choosing based on its configuration.
	//Our goal is to fill this set:
	//
	ProviderSet setDefaultProviders;
	{
		// (j.jones 2011-07-08 12:12) - PLID 44267 - added option for no default (2)
		//	This lives outside the other checks because we do not want the fallback "if only 1 provider" to override this option.
		if(defaultType == Emr::DefaultProvider::None) {
			//Leave the set of providers blank
		}
		else if(defaultType == Emr::DefaultProvider::CreatedUser) {
			//The preference wants us to find out who is linked to the creating user
			FindCurrentUserProviders(setDefaultProviders);
		}
		else if(defaultType == Emr::DefaultProvider::Appointment) {
			//The preference wants us to look at the providers tied to resources for the appt tied to this EMN
			FindLinkedAppointmentProviders(setDefaultProviders);
		}
		else if (defaultType == Emr::DefaultProvider::MainPhysician) {
			//We like general 1, let's use that one.
			//TODO:  We're going to need this later?
			FindGeneral1Provider(setDefaultProviders);
		}
	}

	//
	//Step 2.  Based on the providers selected, determine which are licensed primary providers and which are secondary providers.
	//
	//	Remember that 'setDefaultProviders' is the set of providers that we've decided to move forward with.  We will fill out 2
	//	new sets:
	ProviderSet setPrimaryProviders;
	ProviderSet setSecondaryProviders;
	//	We will also calculate the set of all possible Licensed EMR Providers.  These are people that are both Providers (contacts module) 
	//	and Licensed (Tools -> Manage EMR Provider Licenses).  We'll keep this to use later in the function.
	ProviderSet poolAllLicensedProviders;
	{
		//We will fill the 'poolAllLicensedProviders' set by asking the licensing.
		FillLicensedEMRProviders(poolAllLicensedProviders);

		//Move our licensed providers into primary by intersecting the sets
		boost::set_intersection(setDefaultProviders, poolAllLicensedProviders, inserter(setPrimaryProviders, setPrimaryProviders.end()));
		//Move our unlicensed providers into secondary by diff'ing the sets
		boost::set_difference(setDefaultProviders, poolAllLicensedProviders, inserter(setSecondaryProviders, setSecondaryProviders.end()));
	}

	//
	//Step 3.  Include the secondary provider from the preference 'EMNDefaultSecondaryProviderType'
	//
	{
		if (defaultType == Emr::DefaultProvider::CreatedUser) {
			//Short circuit.  If the "default" preference says to use the created user... then we already did this
			//	and there's no point looking it up again, we'll just add the same provider.
		} else if(GetRemotePropertyInt("EMNDefaultSecondaryProviderType", 0,0, "<None>",true) != 0) {
			//Load them up into the secondary provider set (even if they're allowed as a licensed primary)
			FindCurrentUserProviders(setSecondaryProviders);
		}
	}

	//
	//Step 4.  Follow the 'Link EMN Secondaries' options (Tools -> Licensing) to all the secondary providers, which may add more primary providers.
	//
	{
		if(!setSecondaryProviders.empty()) {
			//(r.wilson 7/29/2013) PLID 48684 - If we determined that there are no primary providers to be selected, we now have the
			//	ability to select a default primary for the given secondary providers.  Note this this is intentionally not executed
			//	if any primary providers were selected above.
			// (d.thompson 2013-08-16) - PLID 57809 - The original implementation of this was deemed insufficient.  This setup applies
			//	to all secondaries regardless of what we've found with primary providers.

			//Gather all the primary providers setup for these secondaries
			// (d.thompson 2013-08-16) - PLID 57809 - CreateParamRecordset now accepts sets.
			_RecordsetPtr rs = CreateParamRecordset("SELECT EMRDefaultProviderID FROM ProvidersT WHERE PersonID IN ({INTSET}) AND EMRDefaultProviderID IS NOT NULL", setSecondaryProviders);
			while(!rs->eof) {
				long nEmrDefaultProviderID = AdoFldLong(rs, "EMRDefaultProviderID");
				//Important:  The configuration data is not cleansed when licensed providers changed.  To be safe, we need to double check that 
				//	all the chosen primaries are still licensed providers.  If they are not, just silently discard them.
				if(poolAllLicensedProviders.find(nEmrDefaultProviderID) != poolAllLicensedProviders.end()){
					setPrimaryProviders.insert(nEmrDefaultProviderID);
				}
				rs->MoveNext();
			}
		}
	}

	//
	//Step 5.  Apply the "1 and only 1" rule -- if this office has only 1 licensed provider allowed, just use them, always.
	//
	{
		// (s.dhole 2013-03-28 14:24) - PLID 55908 if Primary is empty and only one licensed provider then we should select as primary
		if (setPrimaryProviders.empty() && poolAllLicensedProviders.size()==1)
		{
			setPrimaryProviders.insert(*poolAllLicensedProviders.begin());
		}
	}

	//
	//Step 6.  LEGACY.  If we still don't have a primary provider chosen, look at the last EMN on this EMR and use whatever it had.
	//
	{
		if (setPrimaryProviders.empty()) {
			//Fill them directly into the primary provider set
			FindPreviousEMNPrimaryProviders(setPrimaryProviders);
		}
	}

	//
	//Step 7.  If there are still no primary providers, evaluate the "fallback" preference 'EMNDefaultProviderType_Fallback'.  This will tell us
	//	if we should just leave it as-is, or if we should load the general 1 provider (if they are a primary provider - we do not
	//	choose them if they are a secondary provider only).
	//
	{
		//Only operate if there's no primary found
		if (setPrimaryProviders.empty()) {
			// (d.thompson 2013-08-16) - PLID 57809 - This is the new functionality for this PLID.
			long nFallbackPref = GetRemotePropertyInt("EMNDefaultProviderType_Fallback", 1, 0, "<None>", true);
			if(nFallbackPref == 1) {
				if (defaultType == Emr::DefaultProvider::MainPhysician) {
					//Short circuit - If our default type is to use the g1 provider, then we already tried and failed, so there's no point trying again.
				}
				else {
					//We like general 1, let's use that one.
					ProviderSet g1Prov;
					FindGeneral1Provider(g1Prov);

					//If it's licensed, add it to our primary set.  Note that we do not want to add the g1 provider if it's a secondary.
					boost::set_intersection(g1Prov, poolAllLicensedProviders, inserter(setPrimaryProviders, setPrimaryProviders.end()));
				}
			}
			else {
				//Option 0 is "don't do anything".  Gladly.
			}
		}
	}

	//
	//FINAL.  Copy to arrays
	//
	foreach(long id, setPrimaryProviders) {
		arMainProviderIDs.Add(id);
	}
	// (j.luckoski 2013-04-01 16:23) - PLID 55910 - Prevent setting secondary if already primary
	foreach(long id, setSecondaryProviders) {
		if (setPrimaryProviders.count(id)) {
			continue;
		}

		arySecondaryProviders.Add(id);
	}
}
