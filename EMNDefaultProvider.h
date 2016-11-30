// (d.thompson 2013-08-16) - PLID 57809 - Class to encapsulate all the (confusing) logic for calculating
//	the default EMN providers (primary and secondary).
#pragma once

class CEMR;		//Referenced
class CEMN;		//Referenced

class CEMNDefaultProvider
{
public:
	CEMNDefaultProvider(CEMR *pEMR, CEMN *pEMN);
	~CEMNDefaultProvider(void);

	//Call to calculate the default EMN providers.  This should typically be called when creating a new EMN.
	void CalculateDefaultEMNProviders(OUT CArray<long,long> &arMainProviderIDs, OUT CArray<long,long> &arySecondaryProviders);

protected:
	//Necessary to pull some data about the EMN to properly calculate
	CEMR *m_pEMR;
	CEMN *m_pEMN;

	//Simplify readability
	typedef std::set<long> ProviderSet;

	// (d.thompson 2013-08-16) - PLID 57809 - Lookup all licensed providers from our system licensing
	void CEMNDefaultProvider::FillLicensedEMRProviders(OUT ProviderSet &licensed);
	// (d.thompson 2013-08-16) - PLID 57809 - Determine the providers on the appt linked to this EMN.
	void FindLinkedAppointmentProviders(IN OUT ProviderSet &setApptProviders);
	// (d.thompson 2013-08-16) - PLID 57809 - Determine the g1 provider for the current patient
	void FindGeneral1Provider(IN OUT ProviderSet &g1Provider);
	// (d.thompson 2013-08-16) - PLID 57809 - Find the providers linked to the current user.
	void FindCurrentUserProviders(IN OUT ProviderSet &setProviders);
	// (d.thompson 2013-08-16) - PLID 57809 - Find the primary providers that were used on the previous EMN on this EMR.
	void FindPreviousEMNPrimaryProviders(IN OUT ProviderSet &setProviders);
};
