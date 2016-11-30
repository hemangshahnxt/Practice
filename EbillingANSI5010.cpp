// EbillingANSI5010.cpp : implementation file
// Contains all claim generation functions for ANSI 5010 claims

// (j.jones 2010-10-13 11:23) - PLID 32848 - created

#include "stdafx.h"
#include "Ebilling.h"
#include "GlobalUtils.h"
#include "GlobalFinancialUtils.h"
#include "GlobalDataUtils.h"
#include "InternationalUtils.h"
#include "InsuranceDlg.h"
#include "UB04Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

//ANSI X12 5010

//The following functions, ANSI_5010_1000A() to ANSI_5010_2440(), represent individual 'loops' of a generated ANSI claim file.
//Each function will generate its own line and write it to the output file.

int CEbilling::ANSI_5010_Header() {

	//Header

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {
		CString str;
		str = "\r\n5010 HEADER\r\n";
		m_OutputFile.Write(str, str.GetLength());
	}

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							HEADER

//70		005		ST		Transaction Set Header					R		1

		OutputString = "ST";

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//ST01	143			Transaction Set Identifier Code			M 1	ID	3/3
		
		//static "837"
		OutputString += ParseANSIField("837",3,3);

		//ST02	329			Transaction Set Control Number			M 1	AN	4/9
		
		//m_strBatchNumber
		OutputString += ParseANSIField(m_strBatchNumber,4,9,TRUE,'R','0');

		//ST03	1705		Implementation Convention Reference		O 1	AN 1/35

		//005010X222A1 - HCFA, Professional
		//005010X223A2 - UB, Institutional
		//005010X224A1 - Dental

		//this value is the same in GS08, be sure the code is identical

		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType != actInst) {
			//HCFA
			str = "005010X222A1";
		}
		else {
			//UB
			str = "005010X223A2";
		}

		OutputString += ParseANSIField(str,1,12);

		EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

//71		010		BHT		Beginning of Hierarchical Transaction	R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "BHT";

		//BHT01	1005		Hierarchical Structure Code				M 1	ID	4/4
		
		//static "0019"
		OutputString += ParseANSIField("0019",4,4);

		//BHT02	353			Transaction Set Purpose Code			M 1	ID	2/2
		
		//use "18" for a re-issue, "00" for a new claim
		//we just send "00"
		OutputString += ParseANSIField("00",2,2);

		//BHT03	127			Reference Identification				O 1	AN	1/50

		//batch number

		//strangely, the specs upped the official length to 50,
		//but then say "This field is limited to 30 characters."
		//Our batch number would never be that big, so this code
		//is just for posterity.
		if(m_strBatchNumber.GetLength() > 30) {
			m_strBatchNumber = m_strBatchNumber.Right(30);
		}
		OutputString += ParseANSIField(m_strBatchNumber,1,50,TRUE,'R','0');

		//BHT04	373			Date									O 1	DT	8/8

		//current date YYYY/MM/DD
		COleDateTime dt = COleDateTime::GetCurrentTime();
		str = dt.Format("%Y%m%d");
		OutputString += ParseANSIField(str,8,8);
	
		//BHT05	337			Time									O 1	TM	4/8

		//current time	
		str = dt.Format("%H%M%S");
		OutputString += ParseANSIField(str,4,8);

		//BHT06	640			Transaction Type Code					O 1	ID	2/2

		//31 - Subrogation Demand
		//CH - Chargeable
		//RP - Reporting

		//we send "CH"
		OutputString += ParseANSIField("CH",2,2);

		EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_Header");

	return Error_Other;

}

int CEbilling::ANSI_5010_1000A() {

	//Submitter Name

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n1000A\r\n";
		m_OutputFile.Write(str, str.GetLength());

	}

	try {

		CString OutputString,str;
		_variant_t var;

		// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
		_RecordsetPtr rsLocations = CreateParamRecordset("SELECT Name, Phone, Fax "
			"FROM LocationsT WHERE ID = {INT}", GetCurrentLocationID());
		if(rsLocations->eof) {
			rsLocations->Close();
			AfxMessageBox("The current location information could not be loaded. This export will be cancelled.");
			return Error_Missing_Info;
		}

		// (j.jones 2007-04-05 10:37) - PLID 25506 - moved the EbillingFormats load up here
		// so we only open its recordset once
		// (j.jones 2008-05-06 11:33) - PLID 29937 - removed the recordset altogether,
		// we've now cached all the EbillingFormatsT information

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 1000A - SUBMITTER NAME								1

//74		020		NM1		Submitter Name							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M 1	ID	2/3

		//static "41"
		OutputString += ParseANSIField("41",2,3);		

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//1 - Person
		//2 - Non-Person Entity

		//we send "2"
		OutputString += ParseANSIField("2",1,1);

		//NM103	1035		Name Last or Organization Name			O 1	AN	1/60

		//LocationsT.Name
		str = GetFieldFromRecordset(rsLocations, "Name");
		if(str.GetLength() > 60) {
			str = str.Left(60);							//use first 60 chars
		}
		OutputString += ParseANSIField(str, 1, 60);

		//NM104	1036		Name First								O 1	AN	1/35

		OutputString += "*";

		//NM105	1037		Name Middle								O 1	AN	1/25

		OutputString += "*";

		//NM106 NOT USED
		OutputString += "*";

		//NM107 NOT USED
		OutputString += "*";

		//NM108	66			Identification Code Qualifier			X 1	ID	1/2

		//should be 46 in most cases

		CString strSubmitterID = m_strSubmitter1000AID;
		CString strSubmitterQual = m_strSubmitter1000AQual;
		
		//if there is no ID code in NM109, then don't output a number here
		if(strSubmitterID == "")
			str = "";
		else
			str = strSubmitterQual;

		OutputString += ParseANSIField(str,1,25);

		//NM109	67			Identification Code						X 1	AN	2/80

		//EbillingFormatsT.Submitter1000AID
		str = strSubmitterID;
		OutputString += ParseANSIField(str,1,25);

		//NM110 NOT USED
 		OutputString += "*";

		//NM111 NOT USED
		OutputString += "*";

		//NM112 NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

//76		045		PER		Submitter EDI Contact Information		R		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "PER";

		//PER01	366			Contact Function Code					M 1	ID	2/2

		//static "IC";
		OutputString += ParseANSIField("IC",2,2);

		//PER02	93			Name									O 1	AN	1/60

		//EbillingFormatsT.Contact
		str = m_strEbillingFormatContact;
		OutputString += ParseANSIField(str,1,60);

		//PER03	365			Communication Number Qualifier			X 1	ID	2/2

		//static "TE"
		str = GetFieldFromRecordset(rsLocations,"Phone");
		str = StripNonNum(str);
		if(str != "")
			OutputString += ParseANSIField("TE",2,2);
		else
			OutputString += "*";

		//PER04	364			Communication Number					X 1	AN	1/256

		//LocationsT.Phone
		if(str != "")
			OutputString += ParseANSIField(str,1,256);
		else
			OutputString += "*";

		//PER05	365			Communication Number Qualifier			X 1	ID	2/2		

		// (j.jones 2007-04-05 10:44) - PLID 25506 - If EbillingFormatsT.Use1000APER is checked,
		// then we want to output PER05Qual_1000A and PER06ID_1000A.
		// Otherwise, we send the fax number.

		CString strPER05Qual = "";
		CString strPER06ID = "";

		if(m_bUse1000APER) {
			strPER05Qual = m_strPER05Qual_1000A;
			strPER06ID = m_strPER06ID_1000A;
		}
		else {
			//output the fax number

			//supposedly, you can't have two TE qualifiers, so we can't output Phone2

			//what we output next depends if Fax was filled in
			BOOL bFax = FALSE;
			CString strFax;
			strFax = GetFieldFromRecordset(rsLocations,"Fax");
			strFax = StripNonNum(strFax);
			if(strFax != "")
				bFax = TRUE;

			//if LocationsT.Fax not empty
			//static "FX"

			if(bFax) {
				strPER05Qual = "FX";
				strPER06ID = strFax;
			}
		}

		//do not output anything if the ID is blank
		strPER06ID.TrimLeft();
		strPER06ID.TrimRight();
		if(strPER06ID.IsEmpty())
			strPER05Qual = "";

		OutputString += ParseANSIField(strPER05Qual,2,2);

		//PER06	364			Communication Number					X 1	AN	1/256

		// (j.jones 2007-04-05 10:49) - PLID 25506 - will be either
		// LocationsT.Fax or EbillingFormatsT.PER06ID_1000A
		OutputString += ParseANSIField(strPER06ID,1,256);

		//PER07	365			Communication Number Qualifier			X 1	ID	2/2

		OutputString += "*";

		//PER08	364			Communication Number					X 1	AN	1/256

		OutputString += "*";

		//PER09	NOT USED

		// (j.jones 2008-09-02 13:45) - PLID 31114 - if the segment is still only PER*IC, then
		// don't bother exporting anything
		CString strTest = OutputString;
		strTest.TrimRight("*");
		if(strTest != "PER*IC") {
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

		rsLocations->Close();

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_1000A");

	return Error_Other;
}

int CEbilling::ANSI_5010_1000B() {

	//Receiver Name

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n1000B\r\n";
		m_OutputFile.Write(str, str.GetLength());

	}

	try {

		CString OutputString,str;
		_variant_t var;

		// (j.jones 2008-05-06 11:34) - PLID 29937 - removed the EbillingFormatsT recordset as all of it is cached now

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 1000B - RECEIVER NAME									1

//79		020		NM1		Receiver Name							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M 1	ID	2/3

		//static "40"
		OutputString += ParseANSIField("40",2,3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1
		
		//static "2" - non-person entity
		OutputString += ParseANSIField("2",1,1);

		//NM103	1035		Name Last or Organization Name			O 1	AN	1/60

		//EbillingFormatsT.Name
		str = m_strEbillingFormatName;
		OutputString += ParseANSIField(str,1,60);

		//NM104 NOT USED
		OutputString += "*";
		
		//NM105	NOT USED
		OutputString += "*";
		
		//NM106	NOT USED
		OutputString += "*";
		
		//NM107	NOT USED
		OutputString += "*";

		//NM108	66			Identification Code Qualifier			X 1	ID	1/2

		CString strReceiverQual = m_strReceiver1000BQual;
		CString strReceiverID = m_strReceiver1000BID;

		//default "46"
		str = strReceiverID;
		if(str != "")
			OutputString += ParseANSIField(strReceiverQual,1,2);
		else
			OutputString += "*";

		//NM109	67			Identification Code						X 1	AN	2/80

		//EbillingFormatsT.strReceiverID		
		if(str != "")
			OutputString += ParseANSIField(str,2,80);
		else
			OutputString += "*";

		//NM110	NOT USED
		OutputString += "*";

		//NM111	NOT USED
		OutputString += "*";

		//NM112	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_1000B");

	return Error_Other;

}

int CEbilling::ANSI_5010_2000A() {

	//Billing/Pay-To Provider Hierarchical Level

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2000A\r\n";
		m_OutputFile.Write(str, str.GetLength());

	}

	//increment the HL count
	m_ANSIHLCount++;

	//increment the count for the current provider
	m_ANSICurrProviderHL = m_ANSIHLCount;

	try {

		CString OutputString,str;
		_variant_t var;

//							DETAIL, BILLING/PAY-TO PROVIDER HIERARCHICAL LEVEL

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2000A - BILLING/PAY-TO PROVIDER HIERARCHICAL LEVEL	>1

//81		001		HL		Billing/Pay-to Prov. Hierarch. Level	R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HL";

		//HL01	638			Hierarchical ID Number					M 1	AN	1/12

		//m_ANSIHLCount

		str.Format("%li",m_ANSIHLCount);
		OutputString += ParseANSIField(str,1,12);

		//HL02 NOT USED
		OutputString += "*";

		//HL03	735			Hierarchical Level Code					M 1	ID	1/2

		//static "20"
		OutputString += ParseANSIField("20",1,12);

		//HL04	736			Hierarchical Child Code					O 1	ID	1/1

		//static "1"
		OutputString += ParseANSIField("1",1,12);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//83		003		PRV		Billing/Pay-to Prov. Specialty Info.	S		1

		//This segment is used when the Rendering Provider is the same entity
		//as the Billing Provider and/or the Pay-To Provider.
		//If this segment is used, then Loop 2310B is not used.

		//This segment is not used when the Billing or Pay-To Provider is a group
		//and the individual Rendering Provider is in Loop 2310B.

		//This segment will not be used when m_bIsGroup is TRUE.

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "PRV";

		//PRV01	1221		Provider Code							M 1	ID	1/3

		//"BI" - Billing
		OutputString += ParseANSIField("BI",1,3);

		CString strTaxonomy = "";

		bool bIsReferringPhysicianOnUB = (m_actClaimType == actInst && m_pEBillingInfo->Box82Setup == 3);
		
		// (j.jones 2012-03-26 14:46) - PLID 49175 - supported Use2000APRVSegment for UB
		if((m_actClaimType == actProf && ((!m_bIsGroup && m_HCFAInfo.Use2000APRVSegment == 1) || m_HCFAInfo.Use2000APRVSegment == 2))
			|| (m_actClaimType == actInst && ((!m_bIsGroup && m_UB92Info.Use2000APRVSegment == 1) || m_UB92Info.Use2000APRVSegment == 2))) {

			// (j.jones 2012-03-23 16:38) - PLID 49176 - support pulling the location taxonomy code,
			// 0 for provider (default), 1 for location
			// (j.jones 2012-03-26 14:46) - PLID 49175 - supported this same setting for UB
			if((m_actClaimType == actProf && m_HCFAInfo.PRV2000ACode == 1)
				|| (m_actClaimType == actInst && m_UB92Info.PRV2000ACode == 1)){
				_RecordsetPtr rs = CreateParamRecordset("SELECT TaxonomyCode FROM LocationsT WHERE ID = {INT}", m_pEBillingInfo->BillLocation);
				if(!rs->eof) {
					strTaxonomy = AdoFldString(rs, "TaxonomyCode","");
					strTaxonomy.TrimLeft();
					strTaxonomy.TrimRight();
				}
				rs->Close();
			}
			else {
				// (j.jones 2007-05-10 12:24) - PLID 25948 - support the UB referring physician
				// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
				// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore,
				// so if they are sending the referring physician in this loop (extremely unlikely), we cannot
				// send a taxonomy code.
				if(!bIsReferringPhysicianOnUB) {
					_RecordsetPtr rs = CreateParamRecordset("SELECT TaxonomyCode FROM ProvidersT WHERE PersonID = {INT}", m_pEBillingInfo->ProviderID);
					if(!rs->eof) {
						strTaxonomy = AdoFldString(rs, "TaxonomyCode","");
						strTaxonomy.TrimLeft();
						strTaxonomy.TrimRight();
					}
				}
			}			
		}

		// (j.jones 2013-09-05 11:48) - PLID 58252 - the 2000A taxonomy code override will
		// be used only if one exists, and even if the 2000A settings say to send nothing
		// (j.gruber 2013-12-03 12:45) - PLID 59885 - put this in its own if statement
		if (m_actClaimType == actProf || !bIsReferringPhysicianOnUB)
		{
			_RecordsetPtr rsTaxonomyOver;
			if(m_actClaimType == actProf) {
				rsTaxonomyOver = CreateParamRecordset("SELECT ANSI_2000A_Taxonomy AS TaxonomyCode "
					"FROM EbillingSetupT "
					"WHERE ANSI_2000A_Taxonomy <> '' AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
					m_pEBillingInfo->HCFASetupID, m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillLocation);
			}
			//cannot send a taxonomy code if they try to send a referring physician in this loop
			else if(m_actClaimType == actInst && !bIsReferringPhysicianOnUB) {
				rsTaxonomyOver = CreateParamRecordset("SELECT ANSI_2000A_Taxonomy AS TaxonomyCode "
					"FROM UB92EbillingSetupT "
					"WHERE ANSI_2000A_Taxonomy <> '' AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
					m_pEBillingInfo->UB92SetupID, m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillLocation);
			}			
			
			if(!rsTaxonomyOver->eof) {
				CString strTaxonomyOverride = AdoFldString(rsTaxonomyOver, "TaxonomyCode","");
				strTaxonomyOverride.TrimLeft();
				strTaxonomyOverride.TrimRight();
				//we don't permit overriding with nothing, so only use this
				//if there really is an override value
				if(!strTaxonomyOverride.IsEmpty()) {
					strTaxonomy = strTaxonomyOverride;
				}
			}			
		}

		// (j.jones 2008-05-01 10:47) - PLID 28691 - don't output if the taxonomy code is blank
		if(!strTaxonomy.IsEmpty()) {

			//PRV02	128			Reference/Identification Qualifier		M 1	ID	2/3

			//PXC - Health Care Provider Taxonomy Code
			OutputString += ParseANSIField("PXC",2,3);
			
			//PRV03	127			Reference Identification				M 1	AN	1/50

			//ProvidersT.TaxonomyCode
			//This comes from a published HIPAA list.
			//"2086S0122X" is for Plastic and Reconstructive Surgery
			OutputString += ParseANSIField(strTaxonomy,1,50);

			//PRV04	NOT USED
			OutputString += "*";

			//PRV05	NOT USED
			OutputString += "*";

			//PRV06	NOT USED
			OutputString += "*";

			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//84		010		CUR		Foreign Currency Information			S		1

		//Required when the amounts represented in this transaction are currencies
		//other than the United States dollar. If not required by this implementation
		//guide, do not send.

		//we don't support this

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2000A");

	return Error_Other;
}

int CEbilling::ANSI_5010_2010AA() {

	//Billing Provider Name

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2010AA\r\n";
		m_OutputFile.Write(str, str.GetLength());

	}

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;
		
		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(!m_bIsGroup && m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 4) {
			//if individual
			// (j.jones 2006-11-14 17:53) - PLID 23532 - supported the HCFA Group Box 33 Override
			if(m_HCFAInfo.Box33Setup == 3) {
				// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM ProvidersT CROSS JOIN (SELECT * FROM PersonT WHERE PersonT.ID = {INT}) AS PersonT WHERE ProvidersT.PersonID = {INT}", m_HCFAInfo.Box33Num, m_pEBillingInfo->ProviderID);
			}
			else {
				// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID WHERE ID = {INT}",m_pEBillingInfo->ProviderID);
			}

			if(rs->eof) {
				//if the recordset is empty, there is no doctor. So halt everything!!!
				rs->Close();
				// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
				CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
				if(!strPatientName.IsEmpty()) {
					// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
					str.Format("Could not open provider information for patient '%s', Bill ID %li. (ANSI_2010AA 1)", strPatientName, m_pEBillingInfo->BillID);
				}
				else
					//serious problems if you get here
					str = "Could not open provider information. (ANSI_2010AA 1)";

				// (j.jones 2007-05-10 12:07) - PLID 25948 - tweaked the error to be more accurate
				// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
				if((m_actClaimType != actInst && m_pEBillingInfo->Box33Setup == 2) || (m_actClaimType == actInst && m_pEBillingInfo->Box82Setup == 2))
					str += "\nIt is possible that you have no provider selected in General 1 for this patient.";
				else if(m_actClaimType == actInst && m_pEBillingInfo->Box82Setup == 3)
					str += "\nIt is possible that you have no referring physician selected on this patient's bill.";
				else
					str += "\nIt is possible that you have no provider selected on this patient's bill.";
				
				AfxMessageBox(str);
				return Error_Missing_Info;
			}
		}
		else {

			//get the location where the service was performed

			// (j.jones 2008-01-03 11:22) - PLID 28454 - supported the HCFA Group Box 33 Override
			// even when sent as a group
			// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
			if(m_actClaimType != actInst && m_HCFAInfo.Box33Setup == 3) {
				// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM LocationsT CROSS JOIN (SELECT * FROM PersonT WHERE PersonT.ID = {INT}) AS PersonT WHERE LocationsT.ID = {INT}", m_HCFAInfo.Box33Num, m_pEBillingInfo->BillLocation);
			}
			else {
				// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM LocationsT WHERE ID = {INT}",m_pEBillingInfo->BillLocation);
			}

			if(rs->eof) {
				rs->Close();
				//if end of file, select from LocationsT, and the recordset will just pull from the first entry
				// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM LocationsT WHERE TypeID = 1");
				if(rs->eof) {
					str = "Error opening location information.";
					rs->Close();
					AfxMessageBox(str);
					return Error_Missing_Info;
				}
			}
		}

		FieldsPtr fields = rs->Fields;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2010AA - BILLING PROVIDER NAME						1

///////////////////////////////////////////////////////////////////////////////

//87		015		NM1		Billing Provider Name					R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M 1	ID	2/3

		//static "85"
		OutputString += ParseANSIField("85",2,3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//1 - Individual
		//2 - Group

		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2008-01-03 10:56) - PLID 28454 - needed to submit as a group if using an override,
		// because otherwise we have to send a first name field, but that also means we need to check
		// the Box33Name override here first

		// (j.jones 2007-05-07 16:48) - PLID 25922 - added Box33Name override
		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		// (j.jones 2011-11-16 16:36) - PLID 46489 - this now only used for the name, not address
		CString strBox33NameOver = "";
		BOOL bUseBox33NameOver = FALSE;
		
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType != actInst) {
			// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
			_RecordsetPtr rsBox33Over = CreateParamRecordset("SELECT Box33Name "
				"FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
			if(!rsBox33Over->eof) {

				//the Box 33 name override is only supported if the "Use Override" Box 33 setting is in use
				if(m_HCFAInfo.Box33Setup == 3) {
					strBox33NameOver = AdoFldString(rsBox33Over, "Box33Name","");
					strBox33NameOver.TrimLeft();
					strBox33NameOver.TrimRight();

					if(!strBox33NameOver.IsEmpty()) {
						bUseBox33NameOver = TRUE;
					}
				}
			}
			rsBox33Over->Close();
		}

		// (j.jones 2011-11-16 16:37) - PLID 46489 - we now have a specific 2010AA override
		CString strAddress1_Over = "", strAddress2_Over = "", strCity_Over = "", strState_Over = "", strZip_Over = "";
		_RecordsetPtr rsAddressOverride;

		if(m_actClaimType == actInst) {
			//UB			
			rsAddressOverride = CreateParamRecordset("SELECT PayTo2010AA_Address1, PayTo2010AA_Address2, PayTo2010AA_City, PayTo2010AA_State, PayTo2010AA_Zip "
				"FROM UB92EbillingSetupT "
				"WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT} "
				"AND "
				"("
				"(PayTo2010AA_Address1 Is Not Null AND PayTo2010AA_Address1 <> '') "
				"OR (PayTo2010AA_Address2 Is Not Null AND PayTo2010AA_Address2 <> '') "
				"OR (PayTo2010AA_City Is Not Null AND PayTo2010AA_City <> '') "
				"OR (PayTo2010AA_State Is Not Null AND PayTo2010AA_State <> '') "
				"OR (PayTo2010AA_Zip Is Not Null AND PayTo2010AA_Zip <> '')"
				")",
				m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillLocation, m_pEBillingInfo->UB92SetupID);
		}
		else {
			//HCFA
			rsAddressOverride = CreateParamRecordset("SELECT PayTo2010AA_Address1, PayTo2010AA_Address2, PayTo2010AA_City, PayTo2010AA_State, PayTo2010AA_Zip "
				"FROM EbillingSetupT "
				"WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT} "
				"AND "
				"("
				"(PayTo2010AA_Address1 Is Not Null AND PayTo2010AA_Address1 <> '') "
				"OR (PayTo2010AA_Address2 Is Not Null AND PayTo2010AA_Address2 <> '') "
				"OR (PayTo2010AA_City Is Not Null AND PayTo2010AA_City <> '') "
				"OR (PayTo2010AA_State Is Not Null AND PayTo2010AA_State <> '') "
				"OR (PayTo2010AA_Zip Is Not Null AND PayTo2010AA_Zip <> '')"
				")",
				m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillLocation, m_pEBillingInfo->HCFASetupID);
		}

		if(!rsAddressOverride->eof) {
			strAddress1_Over = AdoFldString(rsAddressOverride, "PayTo2010AA_Address1","");
			strAddress1_Over.TrimLeft();
			strAddress1_Over.TrimRight();
			strAddress2_Over = AdoFldString(rsAddressOverride, "PayTo2010AA_Address2","");
			strAddress2_Over.TrimLeft();
			strAddress2_Over.TrimRight();
			strCity_Over = AdoFldString(rsAddressOverride, "PayTo2010AA_City","");
			strCity_Over.TrimLeft();
			strCity_Over.TrimRight();
			strState_Over = AdoFldString(rsAddressOverride, "PayTo2010AA_State","");
			strState_Over.TrimLeft();
			strState_Over.TrimRight();
			strZip_Over = AdoFldString(rsAddressOverride, "PayTo2010AA_Zip","");
			strZip_Over.TrimLeft();
			strZip_Over.TrimRight();
		}
		rsAddressOverride->Close();

		CString strProviderCompany = "";
		BOOL bUseProviderCompanyOnClaims = FALSE;

		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_bIsGroup || m_actClaimType == actInst || m_HCFAInfo.Box33Setup == 4 || m_HCFAInfo.Box33Setup == 3 || bUseBox33NameOver) {
			str = "2";
		}
		else {			
			// (j.jones 2011-11-07 14:46) - PLID 46299 - check the UseCompanyOnClaims setting to send the provider as an organization
			bUseProviderCompanyOnClaims = VarBool(rs->Fields->Item["UseCompanyOnClaims"]->Value, FALSE);
			strProviderCompany = VarString(rs->Fields->Item["Company"]->Value, "");
			strProviderCompany.TrimLeft();
			strProviderCompany.TrimRight();
			if(strProviderCompany.IsEmpty()) {
				//can't use the company if it is blank
				bUseProviderCompanyOnClaims = FALSE;
			}

			if(bUseProviderCompanyOnClaims) {
				str = "2";
			}
			else {
				str = "1";
			}
		}

		// (j.jones 2012-01-06 15:06) - PLID 47351 - store this value
		m_strLast2010AA_NM102 = str;

		OutputString += ParseANSIField(str,1,1);

		//NM103	1035		Name Last or Organization Name			O 1	AN	1/60

		//show the Location Name as the group name if m_bIsGroup is TRUE.

		str = "";
		// (j.jones 2008-01-03 11:14) - PLID 28454 - check for overrides first, as they are
		// allowed even when sending as a group
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType != actInst && bUseBox33NameOver) {
			str = strBox33NameOver;
		}
		else if(m_actClaimType != actInst && m_HCFAInfo.Box33Setup == 3) {
			// (j.jones 2006-11-14 17:56) - PLID 23532 - if an override, we have one long name
			str = VarString(fields->Item["Note"]->Value,"");
		}
		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		else if(m_bIsGroup || m_actClaimType == actInst || m_HCFAInfo.Box33Setup == 4) {
			//LocationsT.Name
			var = fields->Item["Name"]->Value;
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";		
		}
		// (j.jones 2011-11-07 14:46) - PLID 46299 - check the UseCompanyOnClaims setting to send the provider as an organization
		else if(bUseProviderCompanyOnClaims) {
			str = strProviderCompany;
		}
		else {
			//PersonT.Last
			var = fields->Item["Last"]->Value;
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation

		// (j.jones 2012-01-06 15:06) - PLID 47351 - store this value
		m_strLast2010AA_NM103 = str;

		OutputString += ParseANSIField(str,1,60);

		//NM104	1036		Name First								O 1	AN	1/35

		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_bIsGroup || m_actClaimType == actInst || m_HCFAInfo.Box33Setup == 4 || m_HCFAInfo.Box33Setup == 3 || bUseBox33NameOver || bUseProviderCompanyOnClaims)
			str = "";
		else {
			//PersonT.First
			var = fields->Item["First"]->Value;
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,35);

		//NM105	1037		Name Middle								O 1	AN	1/25

		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_bIsGroup || m_actClaimType == actInst || m_HCFAInfo.Box33Setup == 4 || m_HCFAInfo.Box33Setup == 3 || bUseBox33NameOver || bUseProviderCompanyOnClaims)
			str = "";
		else {
			//PersonT.Middle
			var = fields->Item["Middle"]->Value;
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,25);

		//NM106	NOT USED
		OutputString += "*";

		//NM107	1039		Name Suffix								O 1	AN	1/10

		//PersonT.Title
		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_bIsGroup || m_actClaimType == actInst || m_HCFAInfo.Box33Setup == 4 || m_HCFAInfo.Box33Setup == 3 || bUseBox33NameOver || bUseProviderCompanyOnClaims)
			str = "";
		else {
			// (j.jones 2007-08-03 11:44) - PLID 26934 - changed from Suffix to Title
			var = fields->Item["Title"]->Value;
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";			
		}
		OutputString += ParseANSIField(str,1,10);

		//NM108	66			Identification Code	Qualifier			X 1	ID	1/2

		//in 5010 this is ALWAYS the NPI

		CString strNM109ID, strNM108Ident;		

		//"XX" for NPI
		strNM108Ident = "XX";

		strNM109ID = "";

		// (j.jones 2007-08-08 13:14) - PLID 25395 - if there is a 33a override in AdvHCFAPinT, use it here
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType != actInst) {
			// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
			_RecordsetPtr rsOver = CreateParamRecordset("SELECT Box33aNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
			if(!rsOver->eof) {
				var = rsOver->Fields->Item["Box33aNPI"]->Value;
				if(var.vt == VT_BSTR) {
					CString strOver = VarString(var,"");
					strOver.TrimLeft();
					strOver.TrimRight();
					if(!strOver.IsEmpty())
						strNM109ID = strOver;
				}
			}
			rsOver->Close();
		}

		if(strNM109ID.IsEmpty()) {

			// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
			// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
			if(m_HCFAInfo.LocationNPIUsage == 1 && !m_bIsGroup && m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 4) {
				//a non-group HCFA, meaning that the open recordset does
				//not contain the location NPI, and we want it
				
				// (j.jones 2008-05-06 14:06) - PLID 29937 - use the NPI from the loaded claim pointer
				strNM109ID = m_pEBillingInfo->strBillLocationNPI;
			}
			else {
				//whether it's provider or not, the open recordset has
				//the NPI we are looking for
				strNM109ID = VarString(fields->Item["NPI"]->Value, "");
			}
		}

		// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens
		strNM109ID = StripSpacesAndHyphens(strNM109ID);

		if(strNM109ID != "")
			OutputString += ParseANSIField(strNM108Ident,1,2);
		else
			OutputString += "*";

		//NM109	67			Identification Code						X	AN	2/80

		//in 5010 this is ALWAYS the NPI

		if(strNM109ID != "")		
			OutputString += ParseANSIField(strNM109ID,2,80);
		else
			OutputString += "*";

		//NM110 NOT USED
		OutputString += "*";

		//NM111	NOT USED
		OutputString += "*";

		//NM112	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//91		025		N3		Billing Provider Address				R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//if DocAddress == 1, and we're loading a provider, load the Location address,
		//else use the address in the already open recordset
		//(which will be Contacts if a provider, or else a Location)
		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(!m_bIsGroup && m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 4 && m_HCFAInfo.Box33Setup != 3 &&
			m_HCFAInfo.DocAddress == 1) {
			rs->Close();
			// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
			rs = CreateParamRecordset("SELECT * FROM LocationsT WHERE ID = {INT}",m_pEBillingInfo->BillLocation);
			if(rs->eof) {
				rs->Close();
				//if end of file, select from LocationsT, and the recordset will just pull from the first entry
				// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM LocationsT WHERE TypeID = 1");
				if(rs->eof) {
					str = "Error opening location information.";
					rs->Close();
					AfxMessageBox(str);
					return Error_Missing_Info;
				}
			}

			fields = rs->Fields;
		}

		//N301	166			Address Information						M 1	AN	1/55

		//PersonT.Address1

		var = fields->Item["Address1"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		// (j.jones 2011-11-16 16:40) - PLID 46498 - this is now a unique 2010AA override
		if(!strAddress1_Over.IsEmpty()) {
			str = strAddress1_Over;
		}

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str,1,55);

		//N302	166			Address Information						O 1	AN	1/55

		//PersonT.Address2

		//This should not be used if the address 2 is blank.
		//EndANSISegment will trim off the *.

		var = fields->Item["Address2"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		// (j.jones 2011-11-16 16:40) - PLID 46498 - this is now a unique 2010AA override
		if(!strAddress2_Over.IsEmpty()) {
			str = strAddress2_Over;
		}

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str,1,55);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//92		030		N4		Billing Provider City/State/Zip			R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O 1	AN	2/30

		//PersonT.City

		var = fields->Item["City"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		// (j.jones 2011-11-16 16:40) - PLID 46498 - this is now a unique 2010AA override
		if(!strCity_Over.IsEmpty()) {
			str = strCity_Over;
		}

		OutputString += ParseANSIField(str,2,30);

		//N402	156			State or Province Code					O 1	ID	2/2

		//PersonT.State

		var = fields->Item["State"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		// (j.jones 2011-11-16 16:40) - PLID 46498 - this is now a unique 2010AA override
		if(!strState_Over.IsEmpty()) {
			str = strState_Over;
		}

		// (j.jones 2012-01-11 10:48) - PLID 47461 - trim and auto-capitalize this
		str.TrimLeft(); str.TrimRight();
		str.MakeUpper();
		OutputString += ParseANSIField(str,2,2);

		//N403	116			Postal Code								O 1	ID	3/15

		//PersonT.Zip

		//in 5010, this must be 9 digits
		var = fields->Item["Zip"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		// (j.jones 2011-11-16 16:40) - PLID 46498 - this is now a unique 2010AA override
		if(!strZip_Over.IsEmpty()) {
			str = strZip_Over;
		}

		// (j.jones 2013-05-06 17:48) - PLID 55100 - don't strip all numbers, just spaces and hyphens
		str = StripSpacesAndHyphens(str);
		OutputString += ParseANSIField(str,3,15);

		//N404	26			Country Code							O 1	ID	2/3

		//this is only required if the addess is outside the US
		//as of right now, we can't support it
		
		OutputString += "*";

		//N405	NOT USED
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		//N407	1715		Country Subdivision Code				X 1	ID	1/3

		//an even more detailed optional value for country, since we
		//do not support countries, we don't support this either
		
		OutputString += "*";

		EndANSISegment(OutputString);

		rs->Close();

///////////////////////////////////////////////////////////////////////////////

		// (j.jones 2008-05-02 09:46) - PLID 27478 - we need to silently skip
		// adding additional REF segments that have the same qualifier, so
		// track them in an array
		CStringArray arystrQualifiers;

//94		035		REF		Billing Provider Tax Identification		R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//In 5010, REF with EI or SY is its own dedicated REF segment

		//open the proper recordset

		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(!m_bIsGroup && m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 4) {
			//if individual
			// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
			rs = CreateParamRecordset("SELECT * FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID WHERE ID = {INT}",m_pEBillingInfo->ProviderID);

			if(rs->eof) {
				//if the recordset is empty, there is no doctor. So halt everything!!!
				rs->Close();
				// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
				CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
				if(!strPatientName.IsEmpty()) {
					// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
					str.Format("Could not open provider information for patient '%s', Bill ID %li. (ANSI_2010AA 2)", strPatientName, m_pEBillingInfo->BillID);
				}
				else
					//serious problems if you get here
					str = "Could not open provider information. (ANSI_2010AA 2)";

				// (j.jones 2007-05-10 12:07) - PLID 25948 - tweaked the error to be more accurate
				// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
				if((m_actClaimType != actInst && m_pEBillingInfo->Box33Setup == 2) || (m_actClaimType == actInst && m_pEBillingInfo->Box82Setup == 2))
					str += "\nIt is possible that you have no provider selected in General 1 for this patient.";
				else if(m_actClaimType == actInst && m_pEBillingInfo->Box82Setup == 3)
					str += "\nIt is possible that you have no referring physician selected on this patient's bill.";
				else
					str += "\nIt is possible that you have no provider selected on this patient's bill.";
				
				AfxMessageBox(str);
				return Error_Missing_Info;
			}
		}
		else {

			//get the location where the service was performed

			// (j.jones 2008-01-03 11:22) - PLID 28454 - supported the HCFA Group Box 33 Override
			// even when sent as a group
			// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
			if(m_actClaimType != actInst && m_HCFAInfo.Box33Setup == 3) {
				// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM LocationsT CROSS JOIN (SELECT * FROM PersonT WHERE PersonT.ID = {INT}) AS PersonT WHERE LocationsT.ID = {INT}", m_HCFAInfo.Box33Num, m_pEBillingInfo->BillLocation);				
			}
			else {
				// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM LocationsT WHERE ID = {INT}",m_pEBillingInfo->BillLocation);
			}

			if(rs->eof) {
				rs->Close();
				//if end of file, select from LocationsT, and the recordset will just pull from the first entry
				// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM LocationsT WHERE TypeID = 1");
				if(rs->eof) {
					str = "Error opening location information.";
					rs->Close();
					AfxMessageBox(str);
					return Error_Missing_Info;
				}
			}
		}

		fields = rs->Fields;

		// (j.jones 2008-09-09 13:12) - PLID 27450 - changed the default to 2
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		long nExtraREF_IDType = 2;		
		if(m_actClaimType == actInst)
			// (j.jones 2006-11-14 11:49) - PLID 23446 - the UB92 Group has an option as well
			nExtraREF_IDType = m_UB92Info.ExtraREF_IDType;
		else
			nExtraREF_IDType = m_HCFAInfo.ExtraREF_IDType;

		CString strREFIdent = "", strREFID = "";
		CString strLoadedFrom = "";

		// (j.jones 2010-04-14 09:04) - PLID 38194 - the ID is now calculated in a shared function
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		// (j.jones 2012-03-07 14:35) - PLID 48685 - get the strREFIdent and strREFID now, if they are
		// using the EI/SY qualifier they will be sent now and override the ExtraREF if it tries to use
		// EI/SY, otherwise ExtraREF will send its own value and these loaded IDs will be used in the second
		// REF segment
		EBilling_Calculate2010_REF(FALSE, strREFIdent, strREFID, strLoadedFrom, m_actClaimType == actInst,
			m_pEBillingInfo->ProviderID, m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->BillLocation, m_pEBillingInfo->InsuredPartyID,
			m_pEBillingInfo->HCFASetupID, m_HCFAInfo.Box33GRP,
			m_pEBillingInfo->UB92SetupID, m_UB92Info.UB04Box76Qual, m_UB92Info.Box82Num, m_pEBillingInfo->Box82Setup);
		strREFIdent.TrimLeft(); strREFIdent.TrimRight();
		strREFID.TrimLeft(); strREFID.TrimRight();

		CString strExtraRefID = "", strExtraRefIdent = "";

		//0 - EIN/SSN, 1 - NPI, 2 - none
		if(nExtraREF_IDType != 2) {

			//export the extra ID			

			if(nExtraREF_IDType == 0) {
				//EIN/SSN

				//"EI" for EIN, "SY" for SSN - from HCFASetupT.Box25

				//If a group, show the location's EIN, ID of EI")

				// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
				// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_bIsGroup || m_actClaimType == actInst || m_HCFAInfo.Box33Setup == 4) {
					//a group
					strExtraRefIdent = "EI";
					strExtraRefID = GetFieldFromRecordset(rs,"EIN");
				}
				else {
					//individual doctor
					BOOL bUseSSN = (m_HCFAInfo.Box25 == 1);

					// (j.jones 2012-03-08 16:50) - PLID 33945 - we now support the Adv. ID Setup override
					if(m_actClaimType != actInst) {
						_RecordsetPtr rsOver = CreateParamRecordset("SELECT Box25Check FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
						if(!rsOver->eof) {
							long nBox25Check = VarLong(rsOver->Fields->Item["Box25Check"]->Value);
							if(nBox25Check == 1) {
								//this forces use of SSN
								bUseSSN = TRUE;
							}
						}
						rsOver->Close();
					}

					if(bUseSSN) {
						strExtraRefIdent = "SY";

						var = fields->Item["SocialSecurity"]->Value;
						if(var.vt == VT_BSTR)
							strExtraRefID = CString(var.bstrVal);
						else
							strExtraRefID = "";
						// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens,
						// although SSN should really be numeric
						// (j.jones 2016-03-01 09:56) - PLID 68430 - reverted to StripNonNum to get rid of ### signs,
						// you can't type letters into SSN in our system
						strExtraRefID = StripNonNum(strExtraRefID);
					}
					else {
						strExtraRefIdent = "EI";

						var = fields->Item["Fed Employer ID"]->Value;
						if(var.vt == VT_BSTR)
							strExtraRefID = CString(var.bstrVal);
						else
							strExtraRefID = "";
					}
				}
			}
			else {
				//NPI

				//"XX" for NPI
				strExtraRefIdent = "XX";
				strExtraRefID = "";

				// (j.jones 2006-11-14 08:58) - PLID 23413, 23446 - if 0,
				// then use the NPI, may be location NPI if group or UB92

				// (j.jones 2007-01-18 11:51) - PLID 24264 - supported the
				// NPI selection in HCFA Box 33, to determine whether to use
				// the Location NPI or Provider NPI. If a UB92 or a Group,
				// always use location (would already be loaded in the active
				// recordset), if a non-Group HCFA, we need to load the location
				// NPI number.

				// (j.jones 2007-08-08 13:14) - PLID 25395 - if there is a 33a override in AdvHCFAPinT, use it here
				// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType != actInst) {
					// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
					_RecordsetPtr rsOver = CreateParamRecordset("SELECT Box33aNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
					if(!rsOver->eof) {
						var = rsOver->Fields->Item["Box33aNPI"]->Value;
						if(var.vt == VT_BSTR) {
							CString strOver = VarString(var,"");
							strOver.TrimLeft();
							strOver.TrimRight();
							if(!strOver.IsEmpty())
								strExtraRefID = strOver;
						}
					}
					rsOver->Close();
				}

				if(strExtraRefID.IsEmpty()) {

					// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
					// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
					if(m_HCFAInfo.LocationNPIUsage == 1 && !m_bIsGroup && m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 4) {
						//a non-group HCFA, meaning that the open recordset does
						//not contain the location NPI, and we want it

						// (j.jones 2008-05-06 14:06) - PLID 29937 - now the NPI is in the loaded claim pointer
						strExtraRefID = m_pEBillingInfo->strBillLocationNPI;
					}
					else {
						//whether it's provider or not, the open recordset has
						//the NPI we are looking for
						strExtraRefID = VarString(fields->Item["NPI"]->Value, "");
					}
				}
			}

			// (j.jones 2012-03-07 14:42) - PLID 48685 - If a HCFA, then the REF override should
			// override the ID if its qualifier is the same qualifier we're using here. This is because
			// the first REF segment should only be EI or SY (XX will fail). The UB will override always
			// because there is only one REF in the UB format, so if they override with bad data, that's
			// their problem.
			if(m_actClaimType != actInst && strExtraRefIdent.CompareNoCase(strREFIdent) == 0) {
				//use the override ID
				strExtraRefID = strREFID;
				
				//now we've used the REFID, so clear it so we don't try to output it again
				//(it would be skipped anyways because it has the same qualifier)
				strREFID = "";
				strREFIdent = "";
			}
			else if(m_actClaimType == actInst) {
				// (j.jones 2012-01-06 08:47) - PLID 47336 - there is only one REF segment for UB claims,
				// so the override can only overwrite this segment, not the following segment
				_RecordsetPtr rsOver = CreateParamRecordset("SELECT ANSI_2010AA AS IDNum, ANSI_2010AA_Qual AS Qual "
					"FROM UB92EbillingSetupT "
					"WHERE Use_2010AA = 1 AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
					m_pEBillingInfo->UB92SetupID, m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillLocation);
				if(!rsOver->eof) {
					//this allows blank values,  so we must accept whatever is in the data, albeit with spaces trimmed
					strExtraRefID = AdoFldString(rsOver, "IDNum", "");
					strExtraRefID.TrimLeft();
					strExtraRefID.TrimRight();
					strExtraRefIdent = AdoFldString(rsOver, "Qual", "");
					strExtraRefIdent.TrimLeft();
					strExtraRefIdent.TrimRight();
				}
				rsOver->Close();
			}

			// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens
			strExtraRefID = StripSpacesAndHyphens(strExtraRefID);
		}

		// (j.jones 2008-05-02 09:56) - PLID 27478 - disallow sending the additional REF
		// segment if the qualifier has already been used
		if(strExtraRefIdent != "" && strExtraRefID != "" && !IsQualifierInArray(arystrQualifiers, strExtraRefIdent)) {

			// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strExtraRefIdent.CompareNoCase("XX") == 0) {
				strExtraRefIdent = "";
				strExtraRefID = "";
				//log that this happened
				Log("Ebilling file tried to export 2010AA Additional REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
			}
			else {

				OutputString = "REF";

				//REF01	128			Reference Identification Qualifier		M 1	ID	2/3

				OutputString += ParseANSIField(strExtraRefIdent,2,3);

				//REF02	127			Reference Identification				X 1	AN	1/50

				OutputString += ParseANSIField(strExtraRefID,1,50);

				//REF03	NOT USED
				OutputString += "*";

				//REF04	NOT USED
				OutputString += "*";

				EndANSISegment(OutputString);

				// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
				arystrQualifiers.Add(strExtraRefIdent);
			}
		}

		rs->Close();

//96		035		REF		Billing Provider UPIN/License Information		S		2

		//in 5010, extra REFs do not exist on Institutional claims
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType != actInst) {

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//In 5010, only 0B and 1G are legal values in this second REF loop.
		//For the time being, we are continuing to let them set up whatever they want.

		//check to see if we are exporting all IDs from the batch or not

		// (j.jones 2006-12-06 15:50) - PLID 23631 - do not do this if we are separating batches by insurance company
		// (j.jones 2008-05-06 11:37) - PLID 29937 - cached m_ExportAll2010AAIDs and m_bSeparateBatchesByInsCo
		if(m_bExportAll2010AAIDs && !m_bSeparateBatchesByInsCo) {

			// (j.jones 2008-05-02 09:47) - PLID 27478 - pass in our array, so we can track the qualifiers we used
			ANSI_5010_Output2010AAProviderIDs(m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillLocation, arystrQualifiers);
		}
		else {
			//just export one ID

			OutputString = "REF";

			// (j.jones 2012-03-07 14:33) - PLID 48685 - strREFIdent and strREFID are now filled earlier,
			// so we can split out EI/SY IDs versus other IDs

			// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strREFIdent.CompareNoCase("XX") == 0) {
				strREFIdent = "";
				strREFID = "";
				//log that this happened
				Log("Ebilling file tried to export 2010AA REF*XX in for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
			}

			//REF01	128			Reference Identification Qualifier		M 1	ID	2/3

			m_strLast2010AAQual = strREFIdent;

			OutputString += ParseANSIField(strREFIdent,2,3);

			//REF02	127			Reference Identification				X 1	AN	1/50

			OutputString += ParseANSIField(strREFID,1,50);

			//REF03	NOT USED
			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			// (j.jones 2007-09-20 10:51) - PLID 27455 - do not send this segment if the
			// qualifier is blank
			if(strREFIdent != "" && strREFID != "" && !IsQualifierInArray(arystrQualifiers, strREFIdent)) {
				EndANSISegment(OutputString);

				// (j.jones 2008-05-02 09:47) - PLID 27478 - track the qualifier we used
				arystrQualifiers.Add(strREFIdent);
			}
		}

		// (j.jones 2006-07-17 11:56) - PLID 21456 - we now support one additional REF segment,
		// from the ANSI Properties, incase they need to do something silly like send a location ID

		// (j.jones 2008-05-06 11:37) - PLID 29937 - cached bUseAddnl2010AA, m_strAddnl2010AAQual, and m_strAddnl2010AA

		BOOL bUseAddnl2010AA = m_bUse_Addnl_2010AA;
		CString strAddnl2010AAQual = m_strAddnl_2010AA_Qual;
		CString strAddnl2010AA = m_strAddnl_2010AA;

		// (j.jones 2008-05-02 09:56) - PLID 27478 - disallow using bUseAddnl2010AA
		// if the qualifier has already been used
		if(bUseAddnl2010AA && IsQualifierInArray(arystrQualifiers, strAddnl2010AAQual)) {
			bUseAddnl2010AA = FALSE;
		}

		if(bUseAddnl2010AA) {

			//export one more ID

			OutputString = "REF";

			//REF01	128			Reference Identification Qualifier		M 1	ID	2/3
			
			CString strIdent = strAddnl2010AAQual;
			CString strID = strAddnl2010AA;

			OutputString += ParseANSIField(strIdent,2,3);

			//REF02	127			Reference Identification				X 1	AN	1/50

			OutputString += ParseANSIField(strID,1,50);

			//REF03	NOT USED
			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strIdent.CompareNoCase("XX") == 0) {
				strIdent = "";
				strID = "";
				//log that this happened
				Log("Ebilling file tried to export 2010AA Addnl. REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
			}

			if(strIdent != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdent)) {
				EndANSISegment(OutputString);

				// (j.jones 2008-05-02 09:50) - PLID 27478 - add this qualifier to our array
				arystrQualifiers.Add(strIdent);
			}
		}

		}//end if(m_actClaimType != actInst)

///////////////////////////////////////////////////////////////////////////////

//98		040		PER		Billing Provider Contact Information	S		2

		//This is used if the contact information differs from the contact info. in
		//the PER segment of 1000A. In that segment, we use the location information.
		//In this segment, we would do that same, so we won't export anything here
		//until we determine that we need to.
///////////////////////////////////////////////////////////////////////////////		

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2010AA");

	return Error_Other;
}

// (j.jones 2010-11-01 15:33) - PLID 40919 - we now pass in rsAddressOverride, which contains
// the 2010AB override info (will often be eof)
int CEbilling::ANSI_5010_2010AB(ADODB::_RecordsetPtr rsAddressOverride)
{

	//Pay-To Address Name

	//Required when the address for payment is different than that of the Billing Provider.

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2010AB\r\n";
		m_OutputFile.Write(str, str.GetLength());

	}

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;
		
		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(!m_bIsGroup && m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 4) {
			//if individual

			// (j.jones 2006-11-14 17:53) - PLID 23532 - supported the HCFA Group Box 33 Override
			if(m_HCFAInfo.Box33Setup == 3) {
				// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM ProvidersT CROSS JOIN (SELECT * FROM PersonT WHERE PersonT.ID = {INT}) AS PersonT WHERE ProvidersT.PersonID = {INT}", m_HCFAInfo.Box33Num, m_pEBillingInfo->ProviderID);
			}
			else {
				rs = CreateParamRecordset("SELECT * FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID WHERE ID = {INT}",m_pEBillingInfo->ProviderID);
			}

			if(rs->eof) {
				//if the recordset is empty, there is no doctor. So halt everything!!!
				rs->Close();
				// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
				CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
				if(!strPatientName.IsEmpty()) {
					// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
					str.Format("Could not open provider information for patient '%s', Bill ID %li. (ANSI_2010AB 1)", strPatientName, m_pEBillingInfo->BillID);
				}
				else
					//serious problems if you get here
					str = "Could not open provider information. (ANSI_2010AB 1)";

				// (j.jones 2007-05-10 12:07) - PLID 25948 - tweaked the error to be more accurate
				// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
				if((m_actClaimType != actInst && m_pEBillingInfo->Box33Setup == 2) || (m_actClaimType == actInst && m_pEBillingInfo->Box82Setup == 2))
					str += "\nIt is possible that you have no provider selected in General 1 for this patient.";
				else if(m_actClaimType == actInst && m_pEBillingInfo->Box82Setup == 3)
					str += "\nIt is possible that you have no referring physician selected on this patient's bill.";
				else
					str += "\nIt is possible that you have no provider selected on this patient's bill.";
				
				AfxMessageBox(str);
				return Error_Missing_Info;
			}
		}
		else {

			//get the location where the service was performed

			// (j.jones 2008-01-03 11:22) - PLID 28454 - supported the HCFA Group Box 33 Override
			// even when sent as a group
			// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
			if(m_actClaimType != actInst && m_HCFAInfo.Box33Setup == 3) {
				// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM LocationsT CROSS JOIN (SELECT * FROM PersonT WHERE PersonT.ID = {INT}) AS PersonT WHERE LocationsT.ID = {INT}", m_HCFAInfo.Box33Num, m_pEBillingInfo->BillLocation);				
			}
			else {
				// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM LocationsT WHERE ID = {INT}",m_pEBillingInfo->BillLocation);
			}

			if(rs->eof) {
				rs->Close();
				//if end of file, select from LocationsT, and the recordset will just pull from the first entry
				// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM LocationsT WHERE TypeID = 1");
				if(rs->eof) {
					str = "Error opening location information.";
					rs->Close();
					AfxMessageBox(str);
					return Error_Missing_Info;
				}
			}
		}

		FieldsPtr fields = rs->Fields;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2010AB - PAY-TO ADDRESS NAME						1

///////////////////////////////////////////////////////////////////////////////

//101		015		NM1		Pay-To Address Name					S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M 1	ID	2/3

		//static "87"
		OutputString += ParseANSIField("87",2,3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//1 - Individual
		//2 - Group

		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2008-01-03 10:56) - PLID 28454 - needed to submit as a group if using an override,
		// because otherwise we have to send a first name field, but that also means we need to check
		// the Box33Name override here first

		// (j.jones 2010-11-01 15:24) - PLID 40919 - in 5010, the 2010AB record does NOT use the Box33 override,
		// instead we have a new override in the Adv. Ebilling Setup specifically for this loop
		CString strAddress1_Over = "", strAddress2_Over = "",
			strCity_Over = "", strState_Over = "", strZip_Over = "";

		//the recordset was already opened, and passed into this function
		if(!rsAddressOverride->eof) {
			strAddress1_Over = AdoFldString(rsAddressOverride, "PayTo2010AB_Address1","");
			strAddress1_Over.TrimLeft();
			strAddress1_Over.TrimRight();
			strAddress2_Over = AdoFldString(rsAddressOverride, "PayTo2010AB_Address2","");
			strAddress2_Over.TrimLeft();
			strAddress2_Over.TrimRight();
			strCity_Over = AdoFldString(rsAddressOverride, "PayTo2010AB_City","");
			strCity_Over.TrimLeft();
			strCity_Over.TrimRight();
			strState_Over = AdoFldString(rsAddressOverride, "PayTo2010AB_State","");
			strState_Over.TrimLeft();
			strState_Over.TrimRight();
			strZip_Over = AdoFldString(rsAddressOverride, "PayTo2010AB_Zip","");
			strZip_Over.TrimLeft();
			strZip_Over.TrimRight();
		}
		rsAddressOverride->Close();

		CString strProviderCompany = "";
		BOOL bUseProviderCompanyOnClaims = FALSE;

		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_bIsGroup || m_actClaimType == actInst || m_HCFAInfo.Box33Setup == 4 || m_HCFAInfo.Box33Setup == 3) {
			str = "2";
		}
		else {			
			// (j.jones 2011-11-07 14:46) - PLID 46299 - check the UseCompanyOnClaims setting to send the provider as an organization,
			// while this loop doesn't actually send the name, we never follow this setting if the company is blank
			bUseProviderCompanyOnClaims = VarBool(rs->Fields->Item["UseCompanyOnClaims"]->Value, FALSE);
			strProviderCompany = VarString(rs->Fields->Item["Company"]->Value, "");
			strProviderCompany.TrimLeft();
			strProviderCompany.TrimRight();
			if(strProviderCompany.IsEmpty()) {
				//can't use the company if it is blank
				bUseProviderCompanyOnClaims = FALSE;
			}

			if(bUseProviderCompanyOnClaims) {
				str = "2";
			}
			else {
				str = "1";
			}
		}
		OutputString += ParseANSIField(str,1,1);

		//NM103	NOT USED
		OutputString += "*";

		//NM104 NOT USED
		OutputString += "*";

		//NM105 NOT USED
		OutputString += "*";

		//NM106	NOT USED
		OutputString += "*";

		//NM107	NOT USED
		OutputString += "*";

		//NM108	NOT USED
		OutputString += "*";

		//NM109	NOT USED
		OutputString += "*";

		//NM110 NOT USED
		OutputString += "*";

		//NM111	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//103		025		N3		Pay-To Address Address					R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//if DocAddress == 1, and we're loading a provider, load the Location address,
		//else use the address in the already open recordset
		//(which will be Contacts if a provider, or else a Location)

		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(!m_bIsGroup && m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 4 && m_HCFAInfo.Box33Setup != 3 &&
			m_HCFAInfo.DocAddress == 1) {
			rs->Close();
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			rs = CreateParamRecordset("SELECT * FROM LocationsT WHERE ID = {INT}",m_pEBillingInfo->BillLocation);
			if(rs->eof) {
				rs->Close();
				//if end of file, select from LocationsT, and the recordset will just pull from the first entry
				// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM LocationsT WHERE typeID = 1");
				if(rs->eof) {
					str = "Error opening location information.";
					rs->Close();
					AfxMessageBox(str);
					return Error_Missing_Info;
				}
			}

			fields = rs->Fields;
		}

		//N301	166			Address Information						M 1	AN	1/55

		//PersonT.Address1

		var = fields->Item["Address1"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-11-01 15:28) - PLID 40919 - this uses the Adv. Ebilling Setup 2010AB override
		if(!strAddress1_Over.IsEmpty()) {
			str = strAddress1_Over;
		}

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str,1,55);

		//N302	166			Address Information						O 1	AN	1/55

		//PersonT.Address2

		//This should not be used if the address 2 is blank.
		//EndANSISegment will trim off the *.

		var = fields->Item["Address2"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-11-01 15:28) - PLID 40919 - this uses the Adv. Ebilling Setup 2010AB override
		if(!strAddress2_Over.IsEmpty()) {
			str = strAddress2_Over;
		}

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str,1,55);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//104		030		N4		Pay-To Address City/State/Zip			R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O 1	AN	2/30

		//PersonT.City

		var = fields->Item["City"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-11-01 15:28) - PLID 40919 - this uses the Adv. Ebilling Setup 2010AB override
		if(!strCity_Over.IsEmpty()) {
			str = strCity_Over;
		}

		OutputString += ParseANSIField(str,2,30);

		//N402	156			State or Province Code					O 1	ID	2/2

		//PersonT.State

		var = fields->Item["State"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-11-01 15:28) - PLID 40919 - this uses the Adv. Ebilling Setup 2010AB override
		if(!strState_Over.IsEmpty()) {
			str = strState_Over;
		}

		// (j.jones 2012-01-11 10:48) - PLID 47461 - trim and auto-capitalize this
		str.TrimLeft(); str.TrimRight();
		str.MakeUpper();
		OutputString += ParseANSIField(str,2,2);

		//N403	116			Postal Code								O 1	ID	3/15

		//PersonT.Zip

		var = fields->Item["Zip"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-11-01 15:28) - PLID 40919 - this uses the Adv. Ebilling Setup 2010AB override
		if(!strZip_Over.IsEmpty()) {
			str = strZip_Over;
		}

		// (j.jones 2013-05-06 17:48) - PLID 55100 - don't strip all numbers, just spaces and hyphens
		str = StripSpacesAndHyphens(str);
		OutputString += ParseANSIField(str,3,15);

		//N404	26			Country Code							O 1	ID	2/3

		//this is only required if the addess is outside the US
		//as of right now, we can't support it
		
		OutputString += "*";

		//N405	NOT USED
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		//N407	1715		Country Subdivision Code				X 1	ID	1/3

		//an even more detailed optional value for country, since we
		//do not support countries, we don't support this either
		
		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2010AB");

	return Error_Other;
}

/*
int CEbilling::ANSI_5010_2010AC() {

	//Pay-To Plan Name

	//Required when willing trading partners agree to use this implementation
	//for their subrogation payment requests.

	//This loop may only be used when BHT06 = 31.

	// (j.jones 2010-10-14 11:44) - we currently do not support this

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2010BC - RESPONSIBLE PARTY NAME						1

//106		0150	NM1		Pay-To Plan Name						S		1
//108		0250	N3		Pay-to Plan Address						R		1
//109		0300	N4		Pay-To Plan City, State, ZIP Code		R		1
//111		0350	REF		Pay-to Plan Secondary Identification	S		1
//113		0350	REF		Pay-To Plan Tax Identification Number	R		1

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2010AC");

	return Error_Other;
}
*/

int CEbilling::ANSI_5010_2000B() {

	//Subscriber Hierachical Level

	//If the insured and the patient are the same person, then the patient loop (2000C)
	//is not needed. So the 2000B loop will always be run, and 2000C will only be run if
	//the relation to patient is not "self".

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check.  
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2000B\r\n";
		m_OutputFile.Write(str, str.GetLength());

	}


	//increment the HL count
	m_ANSIHLCount++;

	//increment the count for the current subscriber
	m_ANSICurrSubscriberHL = m_ANSIHLCount;

	try {

		CString OutputString,str;
		_variant_t var;
		_RecordsetPtr rs;

		//TES 6/11/2007 - PLID 26257 - Added InsuredPartyT.SecondaryReasonCode
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		rs = CreateParamRecordset("SELECT PersonT.Last, PersonT.First, PersonT.Middle, InsuredPartyT.PersonID, "
					"RelationToPatient, RespTypeID, RespTypeT.Priority, PolicyGroupNum, InsurancePlansT.PlanName, InsType, "
					"InsuredPartyT.SecondaryReasonCode "
					"FROM InsuredPartyT "
					"INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
					"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
					"WHERE InsuredPartyT.PersonID = {INT}", m_pEBillingInfo->InsuredPartyID);

		if(rs->eof) {
			//if the recordset is empty, there is no insured party. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open primary insurance information from this patient's bill for '%s', Bill ID %li. (ANSI_2000B)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open primary insurance information from this patient's bill. (ANSI_2000B)";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		FieldsPtr fields;
		fields = rs->Fields;

//							DETAIL, SUBSCRIBER HIERARCHICAL LEVEL

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2000B - SUBSCRIBER HIERARCHICAL LEVEL					>1

///////////////////////////////////////////////////////////////////////////////

//114		001		HL		Subscriber Hierarchical Level			R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HL";

		//HL01	628			Hierarch ID Number						M 1	AN	1/12

		//m_ANSIHLCount
		str.Format("%li", m_ANSIHLCount);
		OutputString += ParseANSIField(str, 1, 12);

		//HL02	734			Hierarch Parent ID						O 1	AN	1/12

		//m_ANSIProviderCount
		str.Format("%li", m_ANSICurrProviderHL);
		OutputString += ParseANSIField(str, 1, 12);

		//HL03	735			Hierarch Level Code						M 1	ID	1/2

		//static value "22"
		OutputString += ParseANSIField("22", 1, 2);

		//HL04	736			Hierarch Child Code						O 1	ID	1/1

		var = fields->Item["RelationToPatient"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		if(str == "Self") {
			str = "0";
		}
		else {
			str = "1";
		}

		OutputString += ParseANSIField(str, 1, 1);

		EndANSISegment(OutputString);
		
///////////////////////////////////////////////////////////////////////////////

//116		005		SBR		Subscriber Information					R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "SBR";

		//SBR01	1138		Payer Responsibility Sequence Number Code	M 1	ID	1/1

		//Code identifying the insurance carrier's level of responsibility for a payment of a claim

		// (j.jones 2014-08-25 09:40) - PLID 54213 - this qualifier has already been calculated
		CString strSBR01 = OutputSBR01Qualifier(m_pEBillingInfo->sbr2000B_SBR01);
		OutputString += ParseANSIField(strSBR01, 1, 1);

		//SBR02	1069		Individual Relationship Code			O 1	ID	2/2

		var = fields->Item["RelationToPatient"]->Value;
		if(var.vt == VT_BSTR) {
			if(CString(var.bstrVal) == "Self")
				str = "18";		//static value
			else
				str = "";
		}

		OutputString += ParseANSIField(str, 2, 2);				

		//SBR03	127			Reference Identification				O 1	AN	1/50

		//InsuredPartyT.PolicyGroupNum
		
		// (j.jones 2012-10-04 11:13) - PLID 53018 - added ability to not send this element
		//0 - send, 1 - hide
		CString strGroupNum = "";
		if((m_actClaimType != actInst && m_HCFAInfo.ANSI_2000B_SBR03 == 1) || (m_actClaimType == actInst && m_UB92Info.ANSI_2000B_SBR03 == 1)) {
			strGroupNum = "";
		}
		else {
			strGroupNum = GetFieldFromRecordset(rs,"PolicyGroupNum");
		}
		OutputString += ParseANSIField(strGroupNum,1,50);

		//SBR04	93			Name									O 1	AN	1/60

		//InsurancePlansT.PlanName

		// (j.jones 2011-02-21 13:16) - PLID 32848 - Don't export the plan name
		// if the group number exists. This rule used to be only for UB claims,
		// but in 5010 it applies to both UB and HCFA claims.
		// (j.jones 2012-05-14 14:01) - PLID 50338 - The ANSI Specs state that this field
		//is required when the group number is blank, and optional otherwise.
		//Some companies still require it even when we have the group number, so it is now
		//a HCFA/UB group setting to send Always (1 - default) or when the group number is blank (0).
		// (j.jones 2012-10-04 11:10) - PLID 53018 - split the setting up for a 2000B-only version,
		// and added a Never (2) option
		BOOL bSendPlanName = TRUE;
		if((m_actClaimType != actInst && m_HCFAInfo.ANSI_2000B_SBR04 == 2) || (m_actClaimType == actInst && m_UB92Info.ANSI_2000B_SBR04 == 2)) {
			//never send
			bSendPlanName = FALSE;
		}
		else if((m_actClaimType != actInst && m_HCFAInfo.ANSI_2000B_SBR04 == 0) || (m_actClaimType == actInst && m_UB92Info.ANSI_2000B_SBR04 == 0)) {
			//do not send if the group number is filled
			if(!strGroupNum.IsEmpty()) {
				bSendPlanName = FALSE;
			}
		}
		if(!bSendPlanName) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs,"PlanName");
		}
		OutputString += ParseANSIField(str,1,60);

		//SBR05	1336		Insurance Type Code						O 1	ID	1/3
		
		// (j.jones 2007-04-26 09:09) - PLID 25800 - This is only used when
		// Medicare is secondary, and we're not sending P in SBR01. It is also
		// not used on the UB92.

		// (j.jones 2008-09-09 10:08) - PLID 18695 - converted NSF Code to InsType
		//InsType is used in SBR09, but we need it now
		InsuranceTypeCode eInsType = (InsuranceTypeCode)AdoFldLong(rs, "InsType", (long)itcInvalid);

		str = "";

		// (j.jones 2010-10-15 14:49) - PLID 40953 - added Part A as well
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if (m_pEBillingInfo->sbr2000B_SBR01 != SBR01Qualifier::sbrP && m_actClaimType != actInst
			&& (eInsType == itcMedicarePartB || eInsType == itcMedicarePartA)) {

			//it is Medicare Secondary, so send SBR05

			//12 - Medicare Secondary Working Aged Beneficiary or Spouse with Employer Group Health Plan
			//13 - Medicare Secondary End-Stage Renal Disease Beneficiary in the 12 month coordination period with an employer’s group health plan
			//14 - Medicare Secondary, No-fault Insurance including Auto is Primary
			//15 - Medicare Secondary Worker’s Compensation
			//16 - Medicare Secondary Public Health Service (PHS) or Other Federal Agency
			//41 - Medicare Secondary Black Lung
			//42 - Medicare Secondary Veteran’s Administration
			//43 - Medicare Secondary Disabled Beneficiary Under Age 65 with Large Group Health Plan (LGHP)
			//47 - Medicare Secondary, Other Liability Insurance is Primary

			// (j.jones 2007-04-26 09:13) - PLID 25800 - Just auto-assign 47.
			// The reasoning for this is nobody is going to know which to use, and I guarantee
			// nobody is going to care which number it is. If we put an option in the program
			// to set this up per insured party, noboby's going to fill it in, and we'd have
			// to pick a default anyways. So with that argument, we won't add a setup for this
			// until it becomes necessary.

			//TES 6/11/2007 - PLID 26257 - Sadly, Josh's comment above was overly optimistic.  Blackburn's office needs
			// to be able to set this per insured party.  So, we now pull it from InsuredPartyT.

			str = GetFieldFromRecordset(rs,"SecondaryReasonCode");
		}

		OutputString += ParseANSIField(str,1,3);

		//SBR06	NOT USED

		OutputString += "*";

		//SBR07	NOT USED

		OutputString += "*";

		//SBR08	NOT USED

		OutputString += "*";

		//SBR09	1032		Claim Filing Indicator Code				O 1	ID	1/2

		//InsuranceCoT.InsType
		
		// (j.jones 2008-09-09 10:10) - PLID 18695 - we finally track proper
		// Insurance Types per company, so simply call GetANSISBR09CodeFromInsuranceType()
		// to get the ANSI code for our type.

		//we loaded the InsType earlier in SBR05
		str = GetANSI5010_SBR09CodeFromInsuranceType(eInsType);
		if(str.IsEmpty()) {
			str = "ZZ";
		}

		OutputString += ParseANSIField(str,1,2);

		EndANSISegment(OutputString);


///////////////////////////////////////////////////////////////////////////////

//119		007		PAT		Patient Information						S		1
		//this is birth and death information - we support neither

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2000B");

	return Error_Other;
}

int CEbilling::ANSI_5010_2010BA() {

	//Subscriber Name

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2010BA\r\n";
		m_OutputFile.Write(str, str.GetLength());

	}

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		// (j.jones 2012-11-12 10:20) - PLID 53693 - added CountryCode
		rs = CreateParamRecordset("SELECT InsuredPartyT.*, PersonT.*, CountriesT.ISOCode AS CountryCode "
			"FROM InsuredPartyT "
			"INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
			"LEFT JOIN CountriesT ON PersonT.Country = CountriesT.CountryName "
			"WHERE PersonT.ID = {INT}", 
			m_pEBillingInfo->InsuredPartyID);

		if(rs->eof) {
			//if the recordset is empty, there is no subscriber. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open primary insurance information from this patient's bill for '%s', Bill ID %li. (ANSI_2010BA)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open primary insurance information from this patient's bill. (ANSI_2010BA)";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		FieldsPtr fields;
		fields = rs->Fields;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2010BA - SUBSCRIBER NAME								1

///////////////////////////////////////////////////////////////////////////////

//121		015		NM1		Subscriber Name							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M 1	ID	2/3

		//static value
		OutputString += ParseANSIField("IL", 2, 3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//1 - individual
		//2 - non-person entity (company)

		BOOL bIsCompany = FALSE;
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType == actInst && m_UB92Info.ShowCompanyAsInsurer == 1) {
			bIsCompany = TRUE;
		}

		str = "1";
		if(bIsCompany)
			str = "2";

		OutputString += ParseANSIField(str, 1, 1);

		//NM103	1035		Name Last or Organization Name			O 1	AN	1/60

		if(bIsCompany)
			var = fields->Item["Employer"]->Value;	//insured party employer
		else
			var = fields->Item["Last"]->Value;		//insured party last name

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		
		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 60);

		//NM104	1036		Name First								O 1	AN	1/35

		if(!bIsCompany) {
			var = fields->Item["First"]->Value;			//insured party first name

			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else 
			str = "";

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 35);

		//NM105	1037		Name Middle								O 1	AN	1/25

		if(!bIsCompany) {
			var = fields->Item["Middle"]->Value;		//insured party middle name

			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 25);

		//NM106	NOT USED

		OutputString += "*";

		//NM107	1039		Name Suffix								O 1	AN	1/10

		if(!bIsCompany) {
			// (j.jones 2007-08-03 11:44) - PLID 26934 - changed from Suffix to Title
			var = fields->Item["Title"]->Value;
			
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";

		OutputString += ParseANSIField(str, 1, 10);

		//NM108	66			ID Code Qualifer						X 1	ID	1/2

		//II - Standard Unique Health Identifier for each Individual in the United States
		//MI - Member Identification Number

		//we use MI		

		//this is not needed if sending for a company instead of a person,
		//which we only support on the UB
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType == actInst && m_UB92Info.ShowCompanyAsInsurer == 1) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs,"IDForInsurance");
			str = StripPunctuation(str);
		}

		//if no ID, do not output
		if(str != "")
			OutputString += ParseANSIField("MI", 1, 2);
		else
			OutputString += "*";

		//NM109	67			ID Code									X	AN	2/80

		//InsuredPartyT.IDForInsurance
		
		//if no ID, do not output
		if(str != "")
			OutputString += ParseANSIField(str,2,80);
		else
			OutputString += "*";

		//NM110	NOT USED

		OutputString += "*";

		//NM111	NOT USED

		OutputString += "*";

		//NM112	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//124		025		N3		Subscriber Address						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//do not send this unless the subscriber is the patient
		CString strRelation = VarString(fields->Item["RelationToPatient"]->Value, "");
		if(strRelation == "Self") {

			OutputString = "N3";
			
			//N301	166			Address Information						M 1	AN	1/55

			var = fields->Item["Address1"]->Value;		//insured party addr1

			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";

			str = StripPunctuation(str);
			OutputString += ParseANSIField(str, 1, 55);

			//N302	166			Address Information						M 1	AN	1/55

			var = fields->Item["Address2"]->Value;		//insured party addr2

			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";

			str = StripPunctuation(str);
			OutputString += ParseANSIField(str, 1, 55);

			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//125		030		N4		Subscriber City/State/Zip				R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2011-11-22 11:12) - PLID 46570 - do not send this unless the subscriber is the patient
		if(strRelation == "Self") {

			OutputString = "N4";

			//N401	19			City Name								O 1	AN	2/30

			var = fields->Item["City"]->Value;			//insured party City

			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";

			OutputString += ParseANSIField(str, 2, 30);

			//N402	156			State or Province Code					X 1	ID	2/2

			var = fields->Item["State"]->Value;			//insured party state

			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";

			// (j.jones 2012-01-11 10:48) - PLID 47461 - trim and auto-capitalize this
			str.TrimLeft(); str.TrimRight();
			str.MakeUpper();
			OutputString += ParseANSIField(str, 2, 2);

			//N403	116			Postal Code								O 1	ID	3/15

			var = fields->Item["Zip"]->Value;			//insured party Zip code

			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
			
			// (j.jones 2013-05-06 17:48) - PLID 55100 - don't strip all numbers, just spaces and hyphens
			str = StripSpacesAndHyphens(str);
			OutputString += ParseANSIField(str, 3, 15);

			//N404	26			Country Code							O 1	ID	2/3

			//this is only required if the addess is outside the US
			
			// (j.jones 2012-11-12 10:07) - PLID 53693 - use the insured party's country code, if not US
			CString strCountryCode = VarString(fields->Item["CountryCode"]->Value, "");
			if(strCountryCode.CompareNoCase("US") == 0) {
				//leave blank
				strCountryCode = "";
			}
			else {
				//our ISO codes should already be uppercase, force it just incase
				strCountryCode.MakeUpper();
			}
			OutputString += ParseANSIField(strCountryCode, 2, 3);

			//N405	NOT USED
			OutputString += "*";

			//N406	NOT USED
			OutputString += "*";

			//N407	1715		Country Subdivision Code				X 1	ID	1/3

			//an even more detailed optional value for country, since we
			//do not support countries, we don't support this either
			
			OutputString += "*";

			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//127		032		DMG		Subscriber Demographic Information		S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//do not send this unless the subscriber is the patient
		if(strRelation == "Self") {

			OutputString = "DMG";

			//DMG01	1250		Date Time format Qual					X 1	ID	2/3

			var = fields->Item["BirthDate"]->Value;		//insured party birthdate

			if(var.vt == VT_DATE) {
				COleDateTime dt;
				dt = var.date;
				str = dt.Format("%Y%m%d");
			}
			else
				str = "";

			if(str != "")
				//static value "D8"
				OutputString += ParseANSIField("D8", 2, 3);
			else
				OutputString += ParseANSIField("", 2, 3);

			//DMG02	1251		Date Time Period						X 1	AN	1/35
			
			OutputString += ParseANSIField(str, 1, 35);

			//DMG03	1068		Gender Code								O 1	ID	1/1

			var = fields->Item["Gender"]->Value;		//insured party gender

			long gender = VarByte(var,0);
			
			if(gender == 1)
				str = "M";
			else if (gender == 2)
				str = "F";
			else
				str = "U";

			OutputString += ParseANSIField(str, 1, 1);

			//DMG04	NOT USED

			OutputString += "*";

			//DMG05	NOT USED

			OutputString += "*";
			
			//DMG06	NOT USED

			OutputString += "*";

			//DMG07	NOT USED

			OutputString += "*";

			//DMG08	NOT USED

			OutputString += "*";

			//DMG09	NOT USED

			OutputString += "*";

			//DMG10	NOT USED

			OutputString += "*";

			//DMG11	NOT USED

			OutputString += "*";
			
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//129		035		REF		Subscriber Secondary Information		S		4

		//This is an optional segment, and we're not sure if it's really necessary,
		//but since we have the data, we'll use it.

		// (j.jones 2011-11-01 09:04) - PLID 46218 - in 5010 only, the ANSI_Hide2330AREF
		// option is labeled such that it now applies to 2330A and 2010BA
		// (j.jones 2012-01-18 15:02) - PLID 47627 - this now only applies when the insurance
		// company in the group is the one sent in this loop, but in this case that's always
		// the group loaded in m_HCFAInfo/m_UB92Info
		if((m_actClaimType == actInst && m_UB92Info.ANSI_Hide2330AREF == 0)
			|| (m_actClaimType != actInst && m_HCFAInfo.ANSI_Hide2330AREF == 0)) {
				
			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "REF";

			//REF01	128			Reference Ident Qual					M 1	ID	2/3

			//SY - SSN
			OutputString += ParseANSIField("SY", 2, 3);

			//REF02	127			Reference Ident							X 1	AN	1/50

			//InsuredPartyT.SocialSecurity
			str = VarString(fields->Item["SocialSecurity"]->Value);

			//no punctuation of any sort is allowed
			// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens,
			// although SSN should really be numeric
			// (j.jones 2016-03-01 09:56) - PLID 68430 - reverted to StripNonNum to get rid of ### signs,
			// you can't type letters into SSN in our system
			str = StripNonNum(str);

			OutputString += ParseANSIField(str, 1, 50);

			//REF03	NOT USED

			OutputString += "*";

			//REF04	NOT USED

			OutputString += "*";

			//if these is no number, don't output
			if(str != "") {
				EndANSISegment(OutputString);
			}
		}

///////////////////////////////////////////////////////////////////////////////

//130		035		REF		Property and Casualty Claim Number				S		1
//A1.19		035		REF		Property and Casualty Patient Identifier		S		1
//131		035		REF		Property and Casualty Subscriber Contact Info.	S		1

		//we do not support this
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2010BA");

	return Error_Other;
}

int CEbilling::ANSI_5010_2010BB() {

	//Payer Name

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2010BB\r\n";
		m_OutputFile.Write(str, str.GetLength());

	}

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		rs = CreateParamRecordset("SELECT InsuranceCoT.Name, PersonT2.Address1, PersonT2.Address2, PersonT2.City, PersonT2.State, PersonT2.Zip, InsuranceCoT.InsType, InsuranceCoT.EBillingClaimOffice "
						"FROM InsuredPartyT INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
						"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
						"LEFT JOIN PersonT PersonT2 ON InsuranceCoT.PersonID = PersonT2.ID "
						"WHERE InsuredPartyT.PersonID = {INT}", m_pEBillingInfo->InsuredPartyID);

		if(rs->eof) {
			//if the recordset is empty, there is no payer. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open primary insurance information from this patient's bill for '%s', Bill ID %li. (ANSI_2010BB)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open primary insurance information from this patient's bill. (ANSI_2010BB)";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}


		FieldsPtr fields;
		fields = rs->Fields;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2010BB - PAYER NAME									1

///////////////////////////////////////////////////////////////////////////////

//133		015		NM1		Payer Name								R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M 1	ID	2/3

		//static value

		OutputString += ParseANSIField("PR", 2, 3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//static value 2
		OutputString += ParseANSIField("2", 1, 1);

		//NM103	1035		Name Last / Org Name					O 1	AN	1/60

		var = fields->Item["Name"]->Value;				//InsuranceCoT.Name

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		OutputString += ParseANSIField(str, 1, 60);

		//NM104	NOT USED

		OutputString += "*";

		//NM105	NOT USED

		OutputString += "*";

		//NM106	NOT USED

		OutputString += "*";

		//NM107	NOT USED

		OutputString += "*";

		//NM108	66			ID Code Qualifier						X 1	ID	1/2

		//PI - Payor Identification
		//XV - Centers for Medicare and Medicaid Services PlanID

		//we use PI

		// (j.jones 2009-08-05 10:20) - PLID 34467 - this is now in the loaded claim info
		// (j.jones 2009-12-16 15:54) - PLID 36237 - split out into HCFA and UB payer IDs
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType == actInst) {
			str = m_pEBillingInfo->strUBPayerID;
		}
		else {
			str = m_pEBillingInfo->strHCFAPayerID;
		}

		// (b.spivey, May 07, 2013) - PLID 46573 - trimspaces, that's the same as being empty. 
		str = str.Trim(" "); 

		// (j.jones 2008-05-06 11:41) - PLID 29937 - cached m_bUseTHINPayerIDs and m_bPrependTHINPayerNSF
		// (j.jones 2009-08-04 11:34) - PLID 14573 - renamed to PrependTHINPayerNSF to PrependPayerNSF,
		// and removed m_bUseTHINPayerIDs

		//see if we need to prepend the NSF code
		if(m_bPrependPayerNSF) {

			// (j.jones 2008-09-09 10:18) - PLID 18695 - We converted the NSF Code to InsType,
			// but this code actually needs ye olde NSF Code. I am pretty sure this is obsolete
			// because THIN no longer exists, but for the near term let's calculate ye olde NSF code.

			InsuranceTypeCode eInsType = (InsuranceTypeCode)AdoFldLong(rs, "InsType", (long)itcInvalid);
			CString strNSF = GetNSFCodeFromInsuranceType(eInsType);
			if(strNSF.IsEmpty()) {
				strNSF = "Z";
			}
			
			str = strNSF + str;
		}

		//if no ID, do not output
		if(str != "")
			OutputString += ParseANSIField("PI",1,2);
		else
			OutputString += "*";

		//NM109	67			ID Code									X 1	AN	2/80

		// (j.jones 2012-08-06 14:52) - PLID 51916 - This is now either
		// InsuranceCoT.HCFAPayerID or InsuranceCoT.UBPayerID, both are
		// IDs referencing the EbillingInsCoIDs table.
		
		//if no ID, do not output
		if(str != "")
			OutputString += ParseANSIField(str,2,80);
		else
			OutputString += "*";

		//NM110	NOT USED

		OutputString += "*";

		//NM111	NOT USED

		OutputString += "*";

		//NM112	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//135		025		N3		Payer Address							S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//N301	166			Address Information						M 1	AN	1/55

		var = fields->Item["Address1"]->Value;			//insurance co address1

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information						O 1	AN	1/55

		var = fields->Item["Address2"]->Value;			//insurance co address2

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//136		030		N4		Payer City/State/Zip Code				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O 1	AN	2/30

		var = fields->Item["City"]->Value;				//insurance co city

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State or Prov Code						O 1	ID	2/2

		var = fields->Item["State"]->Value;				//insurance co state

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2012-01-11 10:48) - PLID 47461 - trim and auto-capitalize this
		str.TrimLeft(); str.TrimRight();
		str.MakeUpper();
		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O 1	ID	3/15

		var = fields->Item["Zip"]->Value;				//insurance co zip

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2013-05-06 17:48) - PLID 55100 - don't strip all numbers, just spaces and hyphens
		str = StripSpacesAndHyphens(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O 1	ID	2/3

		//this is only required if the addess is outside the US
		//as of right now, we can't support it
		
		OutputString += "*";

		//N405	NOT USED
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		//N407	1715		Country Subdivision Code				X 1	ID	1/3

		//an even more detailed optional value for country, since we
		//do not support countries, we don't support this either
		
		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//138		035		REF		Payer Secondary Information				S		3

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01	128			Reference Ident Qual					M 1	ID	2/3

		//2U - Payer Identification Number
		//EI - Employer’s Identification Number
		//FY - Claim Office Number
		//NF - National Association of Insurance Commissioners(NAIC) Code

		//we send FY
		OutputString += ParseANSIField("FY",2,3);

		//REF02	127			Reference Ident							X 1	AN	1/50

		//InsuranceCoT.EBillingClaimOffice
		str = GetFieldFromRecordset(rs,"EBillingClaimOffice");
		OutputString += ParseANSIField(str,1,50);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		if(str != "") {
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//UB ONLY:

//140		035		REF		Payer Secondary Info.					S		3

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//Only used if an NPI cannot be used and an alternate ID is required.
		//Seems like there is no reason to ever use this. Sure hope it stays that way.

		//OutputString = "REF";

		//REF01	128			Reference Ident Qual					M 1	ID	2/3

		//2U - Payer Identification Number
		//EI - Employer’s Identification Number
		//FY - Claim Office Number
		//NF - National Association of Insurance Commissioners (NAIC) Code

		//we send FY
		//OutputString += ParseANSIField(str,2,3);

		//REF02	127			Reference Ident							X 1	AN	1/50

		//OutputString += ParseANSIField(str,1,50);

		//REF03	NOT USED
		//OutputString += "*";

		//REF04	NOT USED
		//OutputString += "*";

		//EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//HCFA OR UB:

//140		035		REF		Billing Provider Secondary Info.		S		3

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//Only used if an NPI cannot be used and an alternate ID is required.
		//Seems like there is no reason to ever use this. Sure hope it stays that way.

		//OutputString = "REF";

		//REF01	128			Reference Ident Qual					M 1	ID	2/3

		//2U - Payer Identification Number
		//EI - Employer’s Identification Number
		//FY - Claim Office Number
		//NF - National Association of Insurance Commissioners (NAIC) Code

		//we send FY
		//OutputString += ParseANSIField(str,2,3);

		//REF02	127			Reference Ident							X 1	AN	1/50

		//OutputString += ParseANSIField(str,1,50);

		//REF03	NOT USED
		//OutputString += "*";

		//REF04	NOT USED
		//OutputString += "*";

		//EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2010BB");

	return Error_Other;
}

int CEbilling::ANSI_5010_2000C() {

	//Patient Hierarchical Level

	//If the insured and the patient are the same person, then this is not needed.
	//So the 2000B loop will always be run, and 2000C & 2010CA will only be run if
	//the relation to patient is not "self".

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2000C\r\n";
		m_OutputFile.Write(str, str.GetLength());

	}

	m_ANSIHLCount++;

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		rs = CreateParamRecordset("SELECT InsuredPartyT.RelationToPatient "
						"FROM InsuredPartyT INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
						"WHERE InsuredPartyT.PersonID = {INT}", m_pEBillingInfo->InsuredPartyID);

		if(rs->eof) {
			//if the recordset is empty, there is no patient. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open primary insurance information from this patient's bill for '%s', Bill ID %li. (ANSI_2000C)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open primary insurance information from this patient's bill. (ANSI_2000C)";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		FieldsPtr fields;
		fields = rs->Fields;

//							DETAIL, PATIENT HIERARCHICAL LEVEL

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2000C - PATIENT HIERARCHICAL LEVEL					>1


//142		001		HL		Patient Hierarchical Level				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HL";

		//HL01	628			Hierarch ID Number						M 1	AN	1/12

		//m_ANSIHLCount
		str.Format("%li", m_ANSIHLCount);

		OutputString += ParseANSIField(str, 1, 12);

		//HL02	734			Hierarch Parent ID						O 1	AN	1/12

		//m_ANSICurrPatientParent
		str.Format("%li", m_ANSICurrPatientParent);

		OutputString += ParseANSIField(str, 1, 12);

		//HL03	735			Hierarch Level Code						M 1	ID	1/2

		//static value "23"

		OutputString += ParseANSIField("23", 1, 2);

		//HL04	736			Hierarch Child Code						O 1	ID	1/1

		//static value "0"

		OutputString += ParseANSIField("0", 1, 1);

		EndANSISegment(OutputString);

//144		007		PAT		Patient Information						R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "PAT";

		//PAT01	1069		Individual Relat Code					O 1	ID	2/2

		//The values are all listed on page 155

		var = fields->Item["RelationToPatient"]->Value;

		if(var.vt == VT_BSTR) {
			str = CString(var.bstrVal);

			//in 5010 this list has been trimmed, all previously known, valid relationships
			//are now G8 - Other

			if(str == "Spouse") {
				str = "01";
			}
			else if(str == "Child") {
				str = "19";
			}
			else if(str == "Employee") {
				str = "20";
			}
			else if(str == "Organ Donor") {
				str = "39";
			}
			else if(str == "Cadaver Donor") {
				str = "40";
			}
			else if(str == "Life Partner") {
				str = "53";
			}
			//dump all other previously valid relationships into Other, not Unknown
			else if(str == "Other") {

				// (j.jones 2011-06-15 16:54) - PLID 40959 - These relationships are invalid in 5010,
				// and in turn, completely pointless. They have been corrected to "Other" in the data
				// and are no longer selectable options in Practice.
				/*
				|| str == "Other Relationship"
				|| str == "Grandparent"
				|| str == "Grandchild"
				|| str == "Nephew Or Niece"
				|| str == "Adopted Child"
				|| str == "Foster Child"
				|| str == "Ward"
				|| str == "Stepchild"
				|| str == "Handicapped Dependent"
				|| str == "Sponsored Dependent"
				|| str == "Dependent of a Minor Dependent"
				|| str == "Significant Other"
				|| str == "Mother"
				|| str == "Father"
				|| str == "Other Adult"
				|| str == "Emancipated Minor"
				|| str == "Injured Plaintiff"
				|| str == "Child Where Insured Has No Financial Responsibility"
				*/

				str = "G8";
			}
			else if(str == "Unknown") {
				str = "21";
			}
			else {
				//no known relationship
				str = "21";
			}
		}

		OutputString += ParseANSIField(str, 2, 2);

		//PAT02	NOT USED

		OutputString += "*";

		//PAT03	NOT USED

		OutputString += "*";

		//PAT04	NOT USED

		OutputString += "*";

		//PAT05	1250		Date Time format Qual					X 1	ID	2/3

		//not used by us - only if patient is deceased
		//not used on the UB92

		OutputString += "*";

		//PAT06	1251		Date Time Period						X 1	AN	1/35

		//not used by us - only if patient is deceased
		//not used on the UB92

		OutputString += "*";

		//PAT07	355			Unit/basis Meas Code					X 1	ID	2/2

		//not used by us - only if birth

		OutputString += "*";

		//PAT08	81			Weight									X 1	R	1/10

		//not used by us - only if birth

		OutputString += "*";

		//PAT09	1073		Yes/No Cond Resp Code					O 1	ID	1/1

		//not used by us - only if pregnancy

		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2000C");

	return Error_Other;
}

int CEbilling::ANSI_5010_2010CA() {

	//Patient Name

	//If the insured and the patient are the same person, then this is not needed.
	//So the 2000B loop will always be run, and 2000C & 2010CA will only be run if
	//the relation to patient is not "self".

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2010CA\r\n";
		m_OutputFile.Write(str, str.GetLength());

	}

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		// (j.jones 2012-11-12 10:20) - PLID 53693 - added CountryCode
		rs = CreateParamRecordset("SELECT InsuredPartyT.RelationToPatient, PersonT2.Last, PersonT2.First, PersonT2.Middle, PersonT2.Title,  "
						"PersonT2.Address1, PersonT2.Address2, PersonT2.City, PersonT2.State, PersonT2.Zip, PersonT2.BirthDate, PersonT2.Gender, "
						"InsuredPartyT.Patient_IDForInsurance, CountriesT.ISOCode AS CountryCode "
						"FROM InsuredPartyT "
						"INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
						"LEFT JOIN PersonT PersonT2 ON InsuredPartyT.PatientID = PersonT2.ID "
						"LEFT JOIN CountriesT ON PersonT2.Country = CountriesT.CountryName "
						"WHERE InsuredPartyT.PersonID = {INT}", m_pEBillingInfo->InsuredPartyID);

		if(rs->eof) {
			//if the recordset is empty, there is no patient. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open primary insurance information from this patient's bill for '%s', Bill ID %li. (ANSI_2010CA)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open primary insurance information from this patient's bill. (ANSI_2010CA)";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		FieldsPtr fields;
		fields = rs->Fields;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID	- 2010CA - PATIENT NAME									1

//147		015		NM1		Patient Name							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M 1	ID	2/3

		//static value "QC"

		OutputString += ParseANSIField("QC", 2, 3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//static value "1"

		OutputString += ParseANSIField("1", 1, 1);

		//NM103	1035		Name Last / Org Name					O 1	AN	1/60

		str = GetFieldFromRecordset(rs, "Last");		//patient last name

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 60);

		//NM104	1036		Name First								O 1	AN	1/35

		str = GetFieldFromRecordset(rs, "First");		//patient first name

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 35);

		//NM105	1037		Name Middle								O 1	AN	1/25

		str = GetFieldFromRecordset(rs, "Middle");		//patient middle name

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 25);

		//NM106	NOT USED

		OutputString += "*";

		//NM107	1039		Name Suffix								O	AN	1/10

		// (j.jones 2007-08-03 11:44) - PLID 26934 - changed from Suffix to Title
		str = GetFieldFromRecordset(rs, "Title");		//patient title

		OutputString += ParseANSIField(str, 1, 10);

		//NM108	NOT USED

		OutputString += "*";

		//NM109	NOT USED

		OutputString += "*";

		//NM110	NOT USED

		OutputString += "*";

		//NM111	NOT USED

		OutputString += "*";

		//NM112	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

//149		025		N3		Patient Address							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//N301	166			Address Information						M 1	AN	1/55

		str = GetFieldFromRecordset(rs, "Address1");		//patient address1

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information						O 1	AN	1/55

		str = GetFieldFromRecordset(rs, "Address2");		//patient address2

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

//150		030		N4		Patient City/State/Zip					R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O 1	AN	2/30

		str = GetFieldFromRecordset(rs, "City");		//patient city

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State or Prov Code						O 1	ID	2/2

		str = GetFieldFromRecordset(rs, "State");		//patients state
		
		// (j.jones 2012-01-11 10:48) - PLID 47461 - trim and auto-capitalize this
		str.TrimLeft(); str.TrimRight();
		str.MakeUpper();
		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O 1	ID	3/15

		str = GetFieldFromRecordset(rs, "Zip");			//patients zip
		// (j.jones 2013-05-06 17:48) - PLID 55100 - don't strip all numbers, just spaces and hyphens
		str = StripSpacesAndHyphens(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O 1	ID	2/3

		//this is only required if the addess is outside the US
		
		// (j.jones 2012-11-12 10:07) - PLID 53693 - use the patient's country code, if not US
		CString strCountryCode = VarString(fields->Item["CountryCode"]->Value, "");
		if(strCountryCode.CompareNoCase("US") == 0) {
			//leave blank
			strCountryCode = "";
		}
		else {
			//our ISO codes should already be uppercase, force it just incase
			strCountryCode.MakeUpper();
		}
		OutputString += ParseANSIField(strCountryCode, 2, 3);

		//N405	NOT USED
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		//N407	1715		Country Subdivision Code				X 1	ID	1/3

		//an even more detailed optional value for country, since we
		//do not support countries, we don't support this either
		
		OutputString += "*";

		EndANSISegment(OutputString);

//152		032		DMG		Patient Demographic Information			R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DMG";

		//get birthdate first

		COleDateTime dt;
		var = rs->Fields->Item["BirthDate"]->Value;		//patients birthdate
		if(var.vt == VT_DATE) {
			dt = var.date;
			str = dt.Format("%Y%m%d");
		}
		else
			str = "";

		//DMG01	1250		Date Time format Qual					X	ID	2/3

		//static value "D8"

		if(str != "")
			OutputString += ParseANSIField("D8", 2, 3);
		else
			OutputString += ParseANSIField("", 2, 3);

		//DMG02	1251		Date Time Period						X	AN	1/35

		OutputString += ParseANSIField(str,1,35);

		//DMG03	1068		Gender Code								O	ID	1/1

		//Allowed Values
		//	Female		"F"
		//	Male		"M"
		//	Unknown		"U"

		var = fields->Item["Gender"]->Value;

		long gender = long(VarByte(var));

		if(gender == 1)	//male
			str = "M";
		else if (gender == 2)	//female
			str = "F";
		else
			str = "U";

		OutputString += ParseANSIField(str, 1, 1);

		//DMG04	NOT USED

		OutputString += "*";

		//DMG05	NOT USED

		OutputString += "*";

		//DMG06	NOT USED

		OutputString += "*";

		//DMG07	NOT USED

		OutputString += "*";

		//DMG08	NOT USED

		OutputString += "*";

		//DMG09	NOT USED

		OutputString += "*";

		//DMG10	NOT USED

		OutputString += "*";

		//DMG11	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//154		035		REF		Property and Casualty Claim Number		S		1

		//we do not support this

///////////////////////////////////////////////////////////////////////////////

//155		035		REF		Property and Casualty Subscriber Contact Info.		S		1

		//we do not support this
///////////////////////////////////////////////////////////////////////////////
	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2010CA");

	return Error_Other;
}

// (j.jones 2011-03-07 14:09) - PLID 42660 - the CLIA number is now calculated by our caller and is passed in
// (j.jones 2011-04-05 14:31) - PLID 42372 - now we pass in a struct
// (j.jones 2011-04-20 10:45) - PLID 41490 - we now pass in info. on skipping diagnosis codes
int CEbilling::ANSI_5010_2300_Prof(ECLIANumber eCLIANumber)
{

	//Claim Information

	//if the patient is the subscriber, 2300 loop follows the 2010BD loop
	//if the patient is not the subscriber, 2300 loop follows 2010CA

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2300\r\n";
		m_OutputFile.Write(str, str.GetLength());

	}

	//increment the count for the current claim
	m_ANSIClaimCount++;

	try {

		CString OutputString,str;
		_variant_t var;

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		// (j.jones 2009-03-09 09:41) - PLID 33354 - added InsuredPartyT accident information
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		// (d.singleton 2014-03-05 12:29) - PLID 61195 - update ANSI5010 claim export to support ICD10 and new diagcode billing structure
		// (a.walling 2014-03-19 9:13) - PLID 61419 - EBilling - ANSI5010 - Get diag info from bill
		_RecordsetPtr rs = CreateParamRecordset("SELECT BillsT.*, PlaceOfServiceCodesT.PlaceCodes, "		
			"InsuredPartyT.AccidentType, InsuredPartyT.AccidentState, InsuredPartyT.DateOfCurAcc "
			"FROM BillsT "
			"INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE BillsT.ID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null",m_pEBillingInfo->BillID);
			
		if(rs->eof) {
			//if the recordset is empty, there is no bill. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open bill information for '%s', Bill ID %li. (ANSI_2300_Prof)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open bill information. (ANSI_2300_Prof)";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		FieldsPtr fields = rs->Fields;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2300 - CLAIM INFORMATION								100

//157		130		CLM		Claim Information						R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "CLM";

		//CLM01	1028		Claim Submitter's Identifier			M 1	AN	1/38

		//PatientsT.UserDefinedID
		
		str.Format("%li",m_pEBillingInfo->UserDefinedID);

		// (j.jones 2009-10-01 14:21) - PLID 35711 - added the ability to insert text in front of the patient ID
		if(m_bPrependPatientID) {
			str = m_strPrependPatientIDCode + str;
		}

		OutputString += ParseANSIField(str,1,38);

		//CLM02	782			Monetary Amount							O 1	R	1/18

		//this is the total charge amount for the claim

		CString strDocProv = "";
		//if using the bill provider, filter only on that doctor's charges, otherwise get all charges
		//in ANSI, we're using the bill provider if we are using anything but the G1 provider

		// (j.jones 2006-12-01 15:50) - PLID 22110 - supported the ClaimProviderID
		// (j.jones 2008-04-03 09:23) - PLID 28995 - supported the HCFAClaimProvidersT setup
		// (j.jones 2008-08-01 10:17) - PLID 30917 - HCFAClaimProvidersT is now per insurance company
		// (d.thompson 2011-09-12) - PLID 45393 - Properly follow ClaimProviderID on the charge when filtering the charges
		if(m_HCFAInfo.Box33Setup != 2) {
			// (j.jones 2015-03-04 10:37) - PLID 65003 - this needs to find the 2310B rendering provider, not the 2010AA provider
			strDocProv.Format("AND (ChargesT.ClaimProviderID = %li OR (ClaimProviderID IS NULL AND DoctorsProviders IN "
				"(SELECT PersonID FROM "
				"	(SELECT ProvidersT.PersonID, "
				"	(CASE WHEN ANSI_2310B_ProviderID Is Null THEN ProvidersT.ClaimProviderID ELSE ANSI_2310B_ProviderID END) AS ProviderIDToUse "
				"	FROM ProvidersT "
				"	LEFT JOIN (SELECT * FROM HCFAClaimProvidersT WHERE InsuranceCoID = %li) AS HCFAClaimProvidersT ON ProvidersT.PersonID = HCFAClaimProvidersT.ProviderID) SubQ "
				"WHERE ProviderIDToUse = %li))) ", m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ANSI_RenderingProviderID);
		}

		//this cannot be parameterized
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		_RecordsetPtr rsCharges = CreateRecordset("SELECT Sum(dbo.GetChargeTotal(ChargesT.ID)) AS ChargeTotal FROM "
					"ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"WHERE ChargesT.BillID = %li AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					"%s "
					"GROUP BY ChargesT.BillID",m_pEBillingInfo->BillID, strDocProv);

		COleCurrency cyChargeTotal = COleCurrency(0,0);
		if(!rsCharges->eof) {
			cyChargeTotal = AdoFldCurrency(rsCharges, "ChargeTotal", COleCurrency(0,0));
			str = FormatCurrencyForInterface(cyChargeTotal,FALSE,FALSE);

			//see if we need to trim the zeros
			// (j.jones 2012-05-25 16:29) - PLID 50657 - now a modular function, truncates if m_bTruncateCurrency is enabled
			str = FormatANSICurrency(str);
		}
		else
			str = "0";
		OutputString += ParseANSIField(str,1,18);

		rsCharges->Close();

		//CLM03	NOT USED
		OutputString += "*";

		//CLM04	NOT USED
		OutputString += "*";

		//CLM05	C023		Health Care Service Location Info.		O 1

		//CLM05-1	1331	Facility Code Value						M	AN	1/2

		//ChargesT.ServiceLocation
		var = fields->Item["PlaceCodes"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		if(str=="NULL")
			str = "";
		
		OutputString += ParseANSIField(str,1,2);

		//CLM05-2	1332	Facility Code Qualifier					O 1	ID	1/2

		//B - Place of Service Codes for Professional or Dental Services
		//only output if there is a place of service code
		if(str != "")
			OutputString += ":B";

		//CLM05-3	1325	Claim Frequency Type Code				O 1	ID	1/1

		//1 for original, 6 for corrected, 7 for replacement, 8 for void

		// (j.jones 2009-09-15 09:32) - PLID 35284 - we now support configuring
		// this code on the bill
		ANSI_ClaimTypeCode eClaimTypeCode = (ANSI_ClaimTypeCode)AdoFldLong(rs, "ANSI_ClaimTypeCode");
		//***the ANSI_ClaimTypeCode enum is not the same value as what must be submitted on the claim		
		// (j.jones 2016-05-24 14:21) - NX-100704 - this logic is now in a global function
		CString strClaimTypeCode = GetClaimTypeCode_HCFA(eClaimTypeCode);

		//we can only output this if there is a place of service code
		if(str != "") {
			OutputString += ":";
			OutputString += strClaimTypeCode;
		}

		//CLM06	1073		Yes/No Condition or Response Code		O 1	ID	1/1

		//Provider or Supplier Signature Indicator

		//static "Y" (Signature On File)
		OutputString += ParseANSIField("Y",1,1);

		//CLM07	1359		Provider Accept Assignment Code			O 1	ID	1/1

		//Assignment or Plan Participation Code

		//InsuranceAcceptedT.Accepted
		str = "C";
		
		// (j.jones 2010-07-23 15:02) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsCo now
		BOOL bAccepted = GetAcceptAssignment_ByInsCo(m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ANSI_RenderingProviderID);

		if(bAccepted) {
			str = "A";
		}
		OutputString += ParseANSIField(str,1,1);

		//CLM08	1073		Yes/No Condition or Response Code					O 1	ID	1/1

		//any changes here should also change the OI segment

		//Benefits Assignment Certification Indicator

		// (j.jones 2010-06-10 12:53) - PLID 39095 - this is the "Assignment of Benefits Indicator",
		//"indicates insured or authorized person authorizes benefits to be assigned to the provider".

		//This is normally "Y" if InsuranceAcceptedT.Accepted is true, but the bill now has an override
		//to optionally force this to be Yes or No.

		// (j.jones 2010-07-22 12:01) - PLID 39780 - fixed to follow the HCFA group setting for filling Box 13
		BOOL bFillBox13 = TRUE;
		if(m_HCFAInfo.Box13Accepted == 0) {
			//never fill
			bFillBox13 = FALSE;
		}
		else if(m_HCFAInfo.Box13Accepted == 1) {
			//fill if accepted
			bFillBox13 = bAccepted;
		}
		else if(m_HCFAInfo.Box13Accepted == 2) {
			//fill if not accepted
			bFillBox13 = !bAccepted;
		}
		else if(m_HCFAInfo.Box13Accepted == 3) {
			//always fill
			bFillBox13 = TRUE;
		}

		HCFABox13Over hb13Value = (HCFABox13Over)VarLong(fields->Item["HCFABox13Over"]->Value, (long)hb13_UseDefault);		
		if(hb13Value == hb13_No || (hb13Value == hb13_UseDefault && !bFillBox13)) {
			str = "N";
		}
		else {
			str = "Y";
		}

		OutputString += ParseANSIField(str,1,1);

		//CLM09	1363		Release of Information Code					O 1	ID	1/1

		//any changes here should also change the OI segment

		//Code indicating whether the provider has on file a signed statement by the patient
		//authorizing the release of medical data to other organizations

		//I -	Informed Consent to Release Medical Information
		//Y -	Yes, Provider has a Signed Statement Permitting
		//		Release of Medical Billing Data Related to a Claim

		// (j.jones 2010-07-23 11:24) - PLID 39795 - we used to always send Y,
		// but now we will send N if Box 12 is not set to be filled in

		BOOL bFillBox12 = TRUE;
		if(m_HCFAInfo.Box12Accepted == 0) {
			//never fill
			bFillBox12 = FALSE;
		}
		else if(m_HCFAInfo.Box12Accepted == 1) {
			//fill if accepted
			bFillBox12 = bAccepted;
		}
		else if(m_HCFAInfo.Box12Accepted == 2) {
			//fill if not accepted
			bFillBox12 = !bAccepted;
		}
		else if(m_HCFAInfo.Box12Accepted == 3) {
			//always fill
			bFillBox12 = TRUE;
		}

		if(bFillBox12) {
			str = "Y";
		}
		else {
			str = "I";
		}
		OutputString += ParseANSIField(str,1,1);

		//CLM10	1351		Patient Signature Source Code			O 1	ID	1/1

		//any changes here should also change the OI segment

		//Code indicating how the patient or subscriber authorization signatures were
		//obtained and how they are being retained by the provider

		//P	-	Signature generated by provider because the patient
		//		was not physically present for services

		//in 5010, we send nothing, because P makes no sense
		OutputString += ParseANSIField("",1,1);

		//CLM11	C024		Related Causes Information				O 1

		//CLM11-1	1362	Related-Causes Code						M 1	ID	2/3

		//BillsT.RelatedToAutoAcc, BillsT.RelatedToOther, BillsT.RelatedToEmp

		// (j.jones 2009-03-09 09:39) - PLID 33354 - supported the option to load the accident from the insured party

		BOOL bRelatedToEmp = FALSE;
		BOOL bRelatedToAutoAcc = FALSE;
		BOOL bRelatedToOther = FALSE;
		CString strState = "";
		
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//pull from the insured party, not the bill
			InsuredPartyAccidentType ipatAccType = (InsuredPartyAccidentType)VarLong(fields->Item["AccidentType"]->Value, (long)ipatNone);		
			bRelatedToEmp = (ipatAccType == ipatEmployment);
			bRelatedToAutoAcc = (ipatAccType == ipatAutoAcc);
			bRelatedToOther = (ipatAccType == ipatOtherAcc);
			strState = VarString(fields->Item["AccidentState"]->Value, "");
		}
		else {
			//pull from the bill
			bRelatedToEmp = VarBool(fields->Item["RelatedToEmp"]->Value, FALSE);
			bRelatedToAutoAcc = VarBool(fields->Item["RelatedToAutoAcc"]->Value, FALSE);
			bRelatedToOther = VarBool(fields->Item["RelatedToOther"]->Value, FALSE);
			strState = VarString(fields->Item["State"]->Value, "");
		}

		// (j.jones 2012-01-11 10:48) - PLID 47461 - trim and auto-capitalize the state
		strState.TrimLeft(); strState.TrimRight();
		strState.MakeUpper();

		if(bRelatedToAutoAcc) {
			str = "AA";
		}
		else if(bRelatedToOther) {
			str = "OA";
		}
		else if(bRelatedToEmp) {
			str = "EM";
		}
		else {
			str = "";
		}
		OutputString += ParseANSIField(str,2,3);

		//now check if we need the state before outputting the rest of this combo

		if(str == "AA") {
			str = strState;
		}
		else {
			str = "";
		}
		str.TrimRight(" ");

		//at this point str is either blank or the state
			
		//CLM11-2	1362	Related-Causes Code						O 1	ID	2/3
			//not used
		if(str != "")
			OutputString += ":";

		//CLM11-3	1362	Related-Causes Code						O 1	ID	2/3
			//not used
		if(str != "")
			OutputString += ":";

		//CLM11-4	156		State or Province Code					O 1	ID	2/2

		//BillsT.State
		//only used if BillsT.RelatedToAutoAcc is TRUE

		if(str != "") {
			OutputString += ":";
			//str has the state in it
			OutputString += str;
		}

		//CLM11-5	26		Country Code							O 1	ID	2/3
		OutputString += "*";

		//CLM12	1366		Special Program Code					O 1	ID	2/3
		
		//we don't support this
		OutputString += "*";

		//CLM13	NOT USED
		OutputString += "*";
		//CLM14	NOT USED
		OutputString += "*";
		//CLM15	NOT USED
		OutputString += "*";
		//CLM16	NOT USED
		OutputString += "*";
		//CLM17	NOT USED
		OutputString += "*";
		//CLM18	NOT USED
		OutputString += "*";
		//CLM19	NOT USED
		OutputString += "*";

		//CLM20	1514		Delay Reason Code						O 1	ID	1/2
		//we don't support this
		OutputString += "*";

		EndANSISegment(OutputString);

//164		135		DTP		Date - Onset of Current Illness/Symptom	S		1

		// (j.jones 2012-01-23 15:30) - PLID 47676 - added support for sending
		// this date in the Initial Treatment Date field instead of this one
		// (j.jones 2013-08-16 09:24) - PLID 58063 - reverted to always send ConditionDate here,
		// the ConditionDateType doesn't control this field anymore

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

		//static "431"
		OutputString += ParseANSIField("431",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

		//static "D8"
		OutputString += ParseANSIField("D8",2,3);

		//DTP03	1251		Date Time Period						M 1	AN	1/35

		//BillsT.ConditionDate

		// (j.jones 2009-03-09 09:39) - PLID 33354 - supported the option to load the accident from the insured party		
		if(m_HCFAInfo.PullCurrentAccInfo == 2) {
			//load from the insured party
			var = fields->Item["DateOfCurAcc"]->Value;
		}
		else {
			//load from the bill
			var = fields->Item["ConditionDate"]->Value;
		}

		if(var.vt == VT_DATE) {
			 COleDateTime dt = var.date;
			 str = dt.Format("%Y%m%d");
		}
		else
			str = "";
		OutputString += ParseANSIField(str,1,35);

		//as this is conditional, we shouldn't output it at all if we have no data
		if(str != "") {			
			EndANSISegment(OutputString);
		}

//165		135		DTP		Date - Initial Treatment Date			S		1

		// (j.jones 2012-01-23 15:30) - PLID 47676 - added support for sending
		// this field if they selected the option in the Insurance tab of the bill
		// (j.jones 2016-04-08 09:00) - NX-100070 - the ANSI export now ignores ConditionDateType,
		// if they filled out this date we will always send it
		{
			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "DTP";

			//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

			//static "454"
			OutputString += ParseANSIField("454",3,3);

			//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

			//static "D8"
			OutputString += ParseANSIField("D8",2,3);

			//DTP03	1251		Date Time Period						M 1	AN	1/35

			// (j.jones 2016-04-08 09:00) - NX-100070 - this is now
			// BillsT.InitialTreatmentDate
			var = fields->Item["InitialTreatmentDate"]->Value;
			if(var.vt == VT_DATE) {
				 COleDateTime dt = var.date;
				 str = dt.Format("%Y%m%d");
			}
			else
				str = "";
			OutputString += ParseANSIField(str,1,35);

			//as this is conditional, we shouldn't output it at all if we have no data
			if(str != "") {			
				EndANSISegment(OutputString);
			}
		}

//166		135		DTP		Date - Last Seen Date					S		1
		
		// (j.jones 2013-08-06 16:42) - PLID 57895 - supported Last Seen Date
		// (j.jones 2016-04-08 09:00) - NX-100070 - the ANSI export now ignores ConditionDateType,
		// if they filled out this date we will always send it
		{
			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "DTP";

			//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

			//static "304"
			OutputString += ParseANSIField("304",3,3);

			//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

			//static "D8"
			OutputString += ParseANSIField("D8",2,3);

			//DTP03	1251		Date Time Period						M 1	AN	1/35

			// (j.jones 2016-04-08 09:00) - NX-100070 - this is now
			// BillsT.LastSeenDate
			var = fields->Item["LastSeenDate"]->Value;
			if(var.vt == VT_DATE) {
				 COleDateTime dt = var.date;
				 str = dt.Format("%Y%m%d");
			}
			else
				str = "";
			OutputString += ParseANSIField(str,1,35);

			//as this is conditional, we shouldn't output it at all if we have no data
			if(str != "") {			
				EndANSISegment(OutputString);
			}
		}

//167		135		DTP		Date - Acute Manifestation				S		5
		
		// (j.jones 2013-08-16 13:55) - PLID 58069 - added support for sending
		// this field if they selected the option in the Insurance tab of the bill
		// (j.jones 2016-04-08 09:00) - NX-100070 - the ANSI export now ignores ConditionDateType,
		// if they filled out this date we will always send it
		{
			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "DTP";

			//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

			//static "453"
			OutputString += ParseANSIField("453",3,3);

			//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

			//static "D8"
			OutputString += ParseANSIField("D8",2,3);

			//DTP03	1251		Date Time Period						M 1	AN	1/35

			// (j.jones 2016-04-08 09:00) - NX-100070 - this is now
			// BillsT.AcuteManifestationDate
			var = fields->Item["AcuteManifestationDate"]->Value;
			if(var.vt == VT_DATE) {
				 COleDateTime dt = var.date;
				 str = dt.Format("%Y%m%d");
			}
			else
				str = "";
			OutputString += ParseANSIField(str,1,35);

			//as this is conditional, we shouldn't output it at all if we have no data
			if(str != "") {			
				EndANSISegment(OutputString);
			}
		}

//168		135		DTP		Date - Accident							S		10

		//only used if CLM-11 has AA or OA

		// (j.jones 2014-07-17 12:10) - PLID 62929 - ConditionDateType now has an option for
		// 439 - Accident Date.
		// (j.jones 2016-04-08 09:00) - NX-100070 - the ANSI export now ignores ConditionDateType,
		// if they filled out this date we will always send it, but this maintains the older,
		// strange rules for accident:
		//	- if the Accident date is filled, send it regardless of the accident type
		//	- if the Accident date is not filled, but ConditionDate is filled,
		//	and the accident type is Auto or Other, send ConditionDate as the Accident date
		//	***this logic is how it has always worked, prior to ConditionDateType existing,
		//	we are just maintaining it so older claims export the same way as always

		_variant_t varAccidentDate = fields->Item["AccidentDate"]->Value;
		//if Accident Date is not filled in, but accident type is auto or other, 
		//we'll try to send the ConditionDate
		if (varAccidentDate.vt != VT_DATE && (bRelatedToAutoAcc || bRelatedToOther)) {
			//they didn't fill in Accident Date, but they did pick an accident type
			//of Auto or Other, so revert to the legacy logic and send the condition
			//date as the accident date, if one was entered

			varAccidentDate = g_cvarNull;
			if (m_HCFAInfo.PullCurrentAccInfo == 2) {
				//load from the insured party
				varAccidentDate = fields->Item["DateOfCurAcc"]->Value;
			}
			else {
				//load from the bill
				varAccidentDate = fields->Item["ConditionDate"]->Value;
			}
		}
		
		COleDateTime dtAccidentDate = g_cdtInvalid;
		if (varAccidentDate.vt == VT_DATE) {
			dtAccidentDate = VarDateTime(varAccidentDate);
		}

		if (dtAccidentDate.GetStatus() == COleDateTime::valid) {

			OutputString = "DTP";

			//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

			//static "439"
			OutputString += ParseANSIField("439",3,3);

			//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

			//static "D8"
			OutputString += ParseANSIField("D8",2,3);

			//DTP03	1251		Date Time Period						M 1	AN	1/35

			// (j.jones 2014-07-17 12:28) - PLID 62929 - this has been calculated above
			str = dtAccidentDate.Format("%Y%m%d");
			OutputString += ParseANSIField(str,1,35);

			EndANSISegment(OutputString);
		}

//169		135		DTP		Date - Last Menstrual Period			S		1

		//not supported

//170		135		DTP		Date - Last X-Ray Date					S		1

		// (j.jones 2013-08-16 13:55) - PLID 58069 - added support for sending
		// this field if they selected the option in the Insurance tab of the bill
		// (j.jones 2016-04-08 09:00) - NX-100070 - the ANSI export now ignores ConditionDateType,
		// if they filled out this date we will always send it
		{
			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "DTP";

			//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

			//static "455"
			OutputString += ParseANSIField("455",3,3);

			//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

			//static "D8"
			OutputString += ParseANSIField("D8",2,3);

			//DTP03	1251		Date Time Period						M 1	AN	1/35

			// (j.jones 2016-04-08 09:00) - NX-100070 - this is now
			// BillsT.LastXRayDate
			var = fields->Item["LastXRayDate"]->Value;
			if(var.vt == VT_DATE) {
				 COleDateTime dt = var.date;
				 str = dt.Format("%Y%m%d");
			}
			else
				str = "";
			OutputString += ParseANSIField(str,1,35);

			//as this is conditional, we shouldn't output it at all if we have no data
			if(str != "") {			
				EndANSISegment(OutputString);
			}
		}

//171		135		DTP		Date - Hearing And Vision Prescription Date		S		1

		// (j.jones 2013-08-16 13:55) - PLID 58069 - added support for sending
		// this field if they selected the option in the Insurance tab of the bill
		// (j.jones 2016-04-08 09:00) - NX-100070 - the ANSI export now ignores ConditionDateType,
		// if they filled out this date we will always send it
		{
			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "DTP";

			//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

			//static "471"
			OutputString += ParseANSIField("471",3,3);

			//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

			//static "D8"
			OutputString += ParseANSIField("D8",2,3);

			//DTP03	1251		Date Time Period						M 1	AN	1/35

			// (j.jones 2016-04-08 09:00) - NX-100070 - this is now
			// BillsT.HearingAndPrescriptionDate
			var = fields->Item["HearingAndPrescriptionDate"]->Value;
			if(var.vt == VT_DATE) {
				 COleDateTime dt = var.date;
				 str = dt.Format("%Y%m%d");
			}
			else
				str = "";
			OutputString += ParseANSIField(str,1,35);

			//as this is conditional, we shouldn't output it at all if we have no data
			if(str != "") {			
				EndANSISegment(OutputString);
			}
		}

//172		135		DTP		Date - Disability Dates					S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

		//360 if we only have a start date
		//361 if we only have an end date
		//314 if we have both dates

		//BillsT.NoWorkFrom, BillsT.NoWorkTo

		CString strType = "", strQual = "", strDates = "";
		_variant_t varNoWorkFrom = fields->Item["NoWorkFrom"]->Value;
		_variant_t varNoWorkTo = fields->Item["NoWorkTo"]->Value;

		if(varNoWorkFrom.vt == VT_DATE && varNoWorkTo.vt == VT_DATE) {
			//we have both dates
			strType = "314";
			strQual = "RD8";
			COleDateTime dtFrom = VarDateTime(varNoWorkFrom);
			COleDateTime dtTo = VarDateTime(varNoWorkTo);
			CString strNoWorkFrom = dtFrom.Format("%Y%m%d");
			CString strNoWorkTo = dtTo.Format("%Y%m%d");
			strDates.Format("%s-%s", strNoWorkFrom, strNoWorkTo);
		}
		else if(varNoWorkFrom.vt == VT_DATE && varNoWorkTo.vt != VT_DATE) {
			//we have only a start date
			strType = "360";
			strQual = "D8";
			COleDateTime dtFrom = VarDateTime(varNoWorkFrom);
			CString strNoWorkFrom = dtFrom.Format("%Y%m%d");
			strDates = strNoWorkFrom;
		}
		else if(varNoWorkFrom.vt != VT_DATE && varNoWorkTo.vt == VT_DATE) {
			//we have only an end date
			strType = "361";
			strQual = "D8";
			COleDateTime dtTo = VarDateTime(varNoWorkTo);
			CString strNoWorkTo = dtTo.Format("%Y%m%d");
			strDates = strNoWorkTo;
		}

		OutputString += ParseANSIField(strType,3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

		OutputString += ParseANSIField(strQual,2,3);

		//DTP03	1251		Date Time Period						M 1	AN	1/35

		// (j.jones 2012-10-30 16:49) - PLID 53364 - do not strip punctuation here, RD8 dates can have hyphens
		OutputString += ParseANSIField(strDates, 1, 35, FALSE, 'L', ' ', TRUE);

		//as this is conditional, we shouldn't output it at all if we have no data
		if(strDates != "") {			
			EndANSISegment(OutputString);
		}

//174		135		DTP		Date - Last Worked						S		1

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

		//static "297"
		OutputString += ParseANSIField("297",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

		//static "D8"
		OutputString += ParseANSIField("D8",2,3);

		//DTP03	1251		Date Time Period						M 1	AN	1/35

		//BillsT.NoWorkFrom
		var = fields->Item["NoWorkFrom"]->Value;
		if(var.vt == VT_DATE) {
			 COleDateTime dt = var.date;
			 str = dt.Format("%Y%m%d");
		}
		else
			str = "";
		OutputString += ParseANSIField(str,1,35);

		//as this is conditional, we shouldn't output it at all if we have no data
		if(str != "") {			
			EndANSISegment(OutputString);
		}

//175		135		DTP		Date - Authorized Return To Work		S		1

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

		//static "296"
		OutputString += ParseANSIField("296",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

		//static "D8"
		OutputString += ParseANSIField("D8",2,3);

		//DTP03	1251		Date Time Period						M 1	AN	1/35

		//BillsT.NoWorkTo
		var = fields->Item["NoWorkTo"]->Value;
		if(var.vt == VT_DATE) {
			 COleDateTime dt = var.date;
			 str = dt.Format("%Y%m%d");
		}
		else
			str = "";
		OutputString += ParseANSIField(str,1,35);

		//as this is conditional, we shouldn't output it at all if we have no data
		if(str != "") {			
			EndANSISegment(OutputString);
		}

//176		135		DTP		Date - Admission						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

		//static "435"
		OutputString += ParseANSIField("435",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

		//static "D8"
		OutputString += ParseANSIField("D8",2,3);

		//DTP03	1251		Date Time Period						M 1	AN	1/35

		//BillsT.HospFrom
		var = fields->Item["HospFrom"]->Value;
		if(var.vt == VT_DATE) {
			 COleDateTime dt = var.date;
			 str = dt.Format("%Y%m%d");
		}
		else
			str = "";
		OutputString += ParseANSIField(str,1,35);

		//as this is conditional, we shouldn't output it at all if we have no data
		if(str != "") {			
			EndANSISegment(OutputString);
		}

//177		135		DTP		Date - Discharge						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

		//static "096"
		OutputString += ParseANSIField("096",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

		//static "D8"
		OutputString += ParseANSIField("D8",2,3);

		//DTP03	1251		Date Time Period						M 1	AN	1/35

		//BillsT.HospTo
		var = fields->Item["HospTo"]->Value;
		if(var.vt == VT_DATE) {
			 COleDateTime dt = var.date;
			 str = dt.Format("%Y%m%d");
		}
		else
			str = "";
		OutputString += ParseANSIField(str,1,35);

		//as this is conditional, we shouldn't output it at all if we have no data
		if(str != "") {			
			EndANSISegment(OutputString);
		}

//178		135		DTP		Date - Assumed and Relinq. Care Dates	S		2
		
		// (j.jones 2013-08-16 13:55) - PLID 58069 - added support for sending
		// this field if they selected the option in the Insurance tab of the bill,
		// but we only support one or the other (assumed OR relinquished) though
		// their definitions suggest this is typical use
		// (j.jones 2016-04-08 09:00) - NX-100070 - the ANSI export now ignores ConditionDateType,
		// if they filled out these date we will always send them - and this also means
		// we can support sending both, not one or the other
		{
			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "DTP";

			//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

			//static "090"
			OutputString += ParseANSIField("090",3,3);

			//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

			//static "D8"
			OutputString += ParseANSIField("D8",2,3);

			//DTP03	1251		Date Time Period						M 1	AN	1/35

			// (j.jones 2016-04-08 09:00) - NX-100070 - this is now
			// BillsT.AssumedCareDate
			var = fields->Item["AssumedCareDate"]->Value;
			if(var.vt == VT_DATE) {
				 COleDateTime dt = var.date;
				 str = dt.Format("%Y%m%d");
			}
			else
				str = "";
			OutputString += ParseANSIField(str,1,35);

			//as this is conditional, we shouldn't output it at all if we have no data
			if(str != "") {			
				EndANSISegment(OutputString);
			}
		}
		
		{
			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "DTP";

			//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

			//static "091"
			OutputString += ParseANSIField("091",3,3);

			//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

			//static "D8"
			OutputString += ParseANSIField("D8",2,3);

			//DTP03	1251		Date Time Period						M 1	AN	1/35

			// (j.jones 2016-04-08 09:00) - NX-100070 - this is now
			// BillsT.RelinquishedCareDate
			var = fields->Item["RelinquishedCareDate"]->Value;
			if(var.vt == VT_DATE) {
				 COleDateTime dt = var.date;
				 str = dt.Format("%Y%m%d");
			}
			else
				str = "";
			OutputString += ParseANSIField(str,1,35);

			//as this is conditional, we shouldn't output it at all if we have no data
			if(str != "") {			
				EndANSISegment(OutputString);
			}
		}

//180		135		DTP		Date - Property & Casualty Date Of First Contact		S		1
		
		// (j.jones 2013-08-16 13:55) - PLID 58069 - added support for sending
		// this field if they selected the option in the Insurance tab of the bill
		// (j.jones 2016-04-08 09:00) - NX-100070 - the ANSI export now ignores ConditionDateType,
		// if they filled out this date we will always send it
		{
			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "DTP";

			//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

			//static "444"
			OutputString += ParseANSIField("444",3,3);

			//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

			//static "D8"
			OutputString += ParseANSIField("D8",2,3);

			//DTP03	1251		Date Time Period						M 1	AN	1/35

			// (j.jones 2016-04-08 09:00) - NX-100070 - this is now
			// BillsT.FirstVisitOrConsultationDate
			var = fields->Item["FirstVisitOrConsultationDate"]->Value;
			if(var.vt == VT_DATE) {
				 COleDateTime dt = var.date;
				 str = dt.Format("%Y%m%d");
			}
			else
				str = "";
			OutputString += ParseANSIField(str,1,35);

			//as this is conditional, we shouldn't output it at all if we have no data
			if(str != "") {			
				EndANSISegment(OutputString);
			}
		}

//181		135		DTP		Date - Repricer Received Date			S		2
		//not used

//182		155		PWK		Claim Supplemental Information			S		10

		// (a.walling 2007-08-27 10:09) - PLID 27026
		BOOL bSendPaperwork = AdoFldBool(fields, "SendPaperwork", FALSE);

		if(bSendPaperwork) {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			CString strType = AdoFldString(fields, "PaperworkType", "");
			CString strTx = AdoFldString(fields, "PaperworkTx", "");

			if ((strType.GetLength() < 2) || (strTx.GetLength() < 1) ) {
				str.Format("If sending paperwork, both the type code and transmission code must be filled in correctly. Patient '%s', Bill ID %li.",CString(rs->Fields->Item["Name"]->Value.bstrVal),m_pEBillingInfo->BillID);

				AfxMessageBox(str);

				return Error_Missing_Info;
			}

			OutputString = "PWK";

			//PWK01 755		Report Type Code						M 1	ID	2/2
			/*
				03 Report Justifying Treatment Beyond Utilization Guidelines
				04 Drugs Administered
				05 Treatment Diagnosis
				06 Initial Assessment
				07 Functional Goals
				08 Plan of Treatment
				09 Progress Report
				10 Continued Treatment
				11 Chemical Analysis
				13 Certified Test Report
				15 Justification for Admission
				21 Recovery Plan
				A3 Allergies/Sensitivities Document
				A4 Autopsy Report
				AM Ambulance Certification
				AS Admission Summary
				B2 Prescription
				B3 Physician Order
				B4 Referral Form
				BR Benchmark Testing Results
				BS Baseline
				BT Blanket Test Results
				CB Chiropractic Justification
				CK Consent Form(s)
				CT Certification
				D2 Drug Profile Document
				DA Dental Models
				DB Durable Medical Equipment Prescription
				DG Diagnostic Report
				DJ Discharge Monitoring Report
				DS Discharge Summary
				EB Explanation of Benefits (Coordination of Benefits or
				Medicare Secondary Payor)
				HC Health Certificate
				HR Health Clinic Records
				I5 Immunization Record
				IR State School Immunization Records
				LA Laboratory Results
				M1 Medical Record Attachment
				MT Models
				NN Nursing Notes
				OB Operative Note
				OC Oxygen Content Averaging Report
				OD Orders and Treatments Document
				OE Objective Physical Examination (including vital
				signs) Document
				OX Oxygen Therapy Certification
				OZ Support Data for Claim
				P4 Pathology Report
				P5 Patient Medical History Document
				PE Parenteral or Enteral Certification
				PN Physical Therapy Notes
				PO Prosthetics or Orthotic Certification
				PQ Paramedical Results
				PY Physician’s Report
				PZ Physical Therapy Certification
				RB Radiology Films
				RR Radiology Reports
				RT Report of Tests and Analysis Report
				RX Renewable Oxygen Content Averaging Report
				SG Symptoms Document
				V5 Death Notification
				XP Photographs
			*/

			OutputString += ParseANSIField(strType, 2, 2);

			//PWK02	756		Report Transmission Code				O 1	ID	1/2
			/*
				AA Available on Request at Provider Site
					Paperwork is available at the provider’s site. This
					means that the paperwork is not being sent with the
					claim at this time. Instead, it is available to the payer
					(or appropriate entity) at his or her request.
				BM By Mail
				EL Electronically Only
				EM E-Mail
				FT File Transfer
				FX By Fax
			*/

			OutputString += ParseANSIField(strTx, 1, 2);

			//PWK03 NOT USED
			OutputString += "*";

			//PWK04 NOT USED
			OutputString += "*";

			//PWK05 66		Identification Code Qualifier			X 1	ID	1/2
			
			// required when PWK02 != 'AA'. Can be used anytime when Provider wants
			// to send a document control number for an attachment remaining at the
			// Provider's office.

			// AC - Attachment Control Number.

			str = "AC";
			if(strTx == "AA") {
				str = "";
			}
			OutputString += ParseANSIField(str, 1, 2);

			//PWK06 67		Identification Code						X 1	AN	2/80
			// code identifying a party or other code. We will send the Bill's ID.

			CString strIDCode;
			strIDCode.Format("%lu", m_pEBillingInfo->BillID);
			OutputString += ParseANSIField(strIDCode, 2, 80);

			//PWK07 NOT USED
			OutputString += "*";
			
			//PWK08 NOT USED
			OutputString += "*";
			
			//PWK09	NOT USED
			OutputString += "*";

			EndANSISegment(OutputString);
		}
		
//186		160		CN1		Contract Information					S		1

		// (j.jones 2006-11-24 15:08) - PLID 23415 - use the contract information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending contract information
		if(!m_pEBillingInfo->IsPrimary && m_HCFAInfo.ANSI_EnablePaymentInfo == 1
			&& m_HCFAInfo.ANSI_2300Contract == 1) {

			OutputString = "CN1";

			//Ref.	Data		Name									Attributes
			//Des.	Element

			//CN101 1166	Contract Type Code						M 1	ID	2/2

			//01 - Diagnosis Related Group (DRG)
			//02 - Per Diem
			//03 - Variable Per Diem
			//04 - Flat
			//05 - Capitated
			//06 - Percent
			//09 - Other

			//apparently, this only needs to be 09 and the claim total
			str = "09";
			OutputString += ParseANSIField(str,2,2);

			//CN102 782		Monetary Amount							O 1	R	1/18

			//use the cyChargeTotal from earlier, to show the claim total
			str = FormatCurrencyForInterface(cyChargeTotal,FALSE,FALSE);

			//see if we need to trim the zeros
			// (j.jones 2012-05-25 16:29) - PLID 50657 - now a modular function, truncates if m_bTruncateCurrency is enabled
			str = FormatANSICurrency(str);

			OutputString += ParseANSIField(str,1,18);

			//CN103 332		Percent, Decimal Format					O 1	R	1/6

			//we don't use this
			OutputString += ParseANSIField("",1,6);

			//CN104 127		Reference Identification				O 1	AN	1/50

			//we don't use this
			OutputString += ParseANSIField("",1,50);

			//CN105 338		Terms Discount Percent					O 1	R	1/6

			//we don't use this
			OutputString += ParseANSIField("",1,6);

			//CN106 799		Version Identifier						O 1	AN	1/30

			//we don't use this
			OutputString += ParseANSIField("",1,30);

			EndANSISegment(OutputString);
		}

//188		175		AMT		Patient Amount Paid						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "AMT";

		//AMT01	522			Amount Qualifier Code					M 1	ID	1/3

		//static "F5"
		OutputString += ParseANSIField("F5",1,3);

		//AMT02	782			Monetary Amount							M 1	R	1/18

		//This will be the applied patient payments to this bill.

		// (j.jones 2008-09-09 16:48) - PLID 26482 - if m_bHidePatAmtPaid is true,
		// do not send this segment if zero

		str = "0.00";

		BOOL bIsNonZero = FALSE;

		if(m_HCFAInfo.ShowPays) {

			CString strWhere = "";
			if(m_HCFAInfo.IgnoreAdjustments) {
				strWhere = " AND LineItemT2.Type = 1 ";
			}

			// (j.jones 2007-02-08 15:09) - PLID 24677 - changed to send only patient payments, not all payments
			// (will optionally include any patient adjustments although that is unlikely to be a factor)
			//this cannot be parameterized
			// (j.jones 2011-08-16 17:22) - PLID 44805 - We will filter out "original" and "void" charges & applies.
			// The applies part of this does not need to check LineItemCorrectionsBalancingAdjT,
			// because they are only applied to original and void charges.
			_RecordsetPtr rsApplies = CreateRecordset("SELECT Sum(Round(Convert(money,CASE WHEN AppliesT.Amount Is NULL THEN 0 ELSE AppliesT.Amount END),2)) AS Total "
				"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
				"LEFT JOIN LineItemT LineItemT2 ON AppliesT.SourceID = LineItemT2.ID "
				"LEFT JOIN PaymentsT ON LineItemT2.ID = PaymentsT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalCharges2Q ON LineItemT2.ID = LineItemCorrections_OriginalCharges2Q.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingCharges2Q ON LineItemT2.ID = LineItemCorrections_VoidingCharges2Q.VoidingLineItemID "
				"WHERE ChargesT.BillID = %li AND PaymentsT.InsuredPartyID = -1"
				"AND LineItemT.Deleted = 0 AND LineItemT2.Deleted = 0 "
				"AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND LineItemCorrections_OriginalCharges2Q.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingCharges2Q.VoidingLineItemID Is Null "
				"%s",m_pEBillingInfo->BillID,strWhere);

			if(!rsApplies->eof) {

				COleCurrency cyAmt = AdoFldCurrency(rsApplies, "Total",COleCurrency(0,0));

				if(cyAmt != COleCurrency(0,0)) {
					bIsNonZero = TRUE;
				}

				str = FormatCurrencyForInterface(cyAmt, FALSE, FALSE);

				//see if we need to trim the zeros
				// (j.jones 2012-05-25 16:29) - PLID 50657 - now a modular function, truncates if m_bTruncateCurrency is enabled
				str = FormatANSICurrency(str);
			}
			rsApplies->Close();
		}
		else {
			str = "0.00";
		}

		OutputString += ParseANSIField(str,1,18);

		//AMT03	NOT USED

		OutputString += "*";

		// (j.jones 2008-09-09 16:50) - PLID 26482 - send the segment
		// if ShowPays is true, and if either m_bHidePatAmtPaid is false
		// or the amount is nonzero
		if(m_HCFAInfo.ShowPays && (!m_bHidePatAmtPaid || bIsNonZero)) {
			EndANSISegment(OutputString);
		}

//189		180		REF		Service Auth. Exception Code			S		1
//191		180		REF		Mandatory Medicare Crossover Indicator	S		1
//192		180		REF		Mammography Certification Number		S		1

		//not used

//193		180		REF		Referral Number							S		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01	128			Reference Identification Qualifier			M 1	ID	2/3
		
		//9F - Referral Number

		CString strRefNumQual = "";
		CString strRefNumID = "";

		//If the default qualifier is 9F, use that and the Prior Auth number,
		//otherwise check the setting in the bill

		strRefNumQual = m_HCFAInfo.PriorAuthQualifier;
		strRefNumQual.TrimLeft();
		strRefNumQual.TrimRight();
		if(strRefNumQual.IsEmpty()) {
			//if empty, then no override is in place,
			//so pick the qualifier from BillsT.PriorAuthType
			PriorAuthType patPriorAuthType = (PriorAuthType)VarLong(fields->Item["PriorAuthType"]->Value, (long)patPriorAuthNumber);
			if(patPriorAuthType == patReferralNumber) {
				strRefNumQual = "9F";
			}
		}

		if(strRefNumQual == "9F") {
			//BillsT.PriorAuthNum
			var = fields->Item["PriorAuthNum"]->Value;
			if(var.vt == VT_BSTR)
				strRefNumID = CString(var.bstrVal);
			else
				strRefNumID = "";

			//use the override from the HCFA setup
			if(strRefNumID == "") {
				strRefNumID = m_HCFAInfo.DefaultPriorAuthNum;
			}
		}

		if(m_HCFAInfo.ANSI_SendRefPhyIn2300 == 1 && 
			!IsRecordsetEmpty("SELECT ID FROM BillsT INNER JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
					"WHERE ID = %li",m_pEBillingInfo->BillID)) {

			//they want to send this segment with the referring physician, so do it

			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT PersonT.ID, PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, PersonT.SocialSecurity, ReferringPhysT.* "
					"FROM BillsT LEFT JOIN PersonT ON BillsT.RefPhyID = PersonT.ID "
					"INNER JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
					"WHERE BillsT.ID = {INT}", m_pEBillingInfo->BillID);

			if(rs->eof) {
				//if the recordset is empty, there is no referring provider. So halt everything!!!
				rs->Close();
				// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
				CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
				if(!strPatientName.IsEmpty()) {
					// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
					str.Format("Could not open referring provider information for '%s', Bill ID %li. (ANSI_2300_Prof)", strPatientName, m_pEBillingInfo->BillID);
				}
				else
					//serious problems if you get here
					str = "Could not open referring provider information. (ANSI_2300_Prof)";
				
				AfxMessageBox(str);
				return Error_Missing_Info;
			}

			strRefNumQual = "9F";
			
			//Box17aANSI returns the qualifier and the ID, but here we only want the ID
			var = rs->Fields->Item["ID"]->Value;
			CString strIdentifier;
			// (j.jones 2010-10-20 15:42) - PLID 40936 - moved Box17aANSI to GlobalFinancialUtils and renamed it
			EBilling_Box17aANSI(m_HCFAInfo.Box17a, m_HCFAInfo.Box17aQual, strIdentifier,strRefNumID,VarLong(var));

			rs->Close();
		}

		OutputString += ParseANSIField(strRefNumQual,2,3);

		//REF02	127			Reference Identification					X 1	AN	1/50

		OutputString += ParseANSIField(strRefNumID,1,50);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		//only output if we loaded both a qualifier and an ID
		if(strRefNumQual == "9F" && strRefNumID != "") {
			EndANSISegment(OutputString);
		}

//194		180		REF		Prior Authorization						S		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01	128			Reference Identification Qualifier		M 1	ID	2/3
		
		//G1 - Prior Authorization Number

		CString strPriorAuthQual = "";
		CString strPriorAuthID = "";

		//If the default qualifier is G1, use that and the Prior Auth number,
		//otherwise check the setting in the bill

		strPriorAuthQual = m_HCFAInfo.PriorAuthQualifier;
		strPriorAuthQual.TrimLeft();
		strPriorAuthQual.TrimRight();
		if(strPriorAuthQual.IsEmpty()) {
			//if empty, then no override is in place,
			//so pick the qualifier from BillsT.PriorAuthType
			PriorAuthType patPriorAuthType = (PriorAuthType)VarLong(fields->Item["PriorAuthType"]->Value, (long)patPriorAuthNumber);
			if(patPriorAuthType != patReferralNumber) {
				strPriorAuthQual = "G1";
			}
		}

		if(strPriorAuthQual == "G1") {
			//BillsT.PriorAuthNum
			var = fields->Item["PriorAuthNum"]->Value;
			if(var.vt == VT_BSTR)
				strPriorAuthID = CString(var.bstrVal);
			else
				strPriorAuthID = "";

			//use the override from the HCFA setup
			if(strPriorAuthID == "") {
				strPriorAuthID = m_HCFAInfo.DefaultPriorAuthNum;
			}
		}
		OutputString += ParseANSIField(strPriorAuthQual,2,3);

		//REF02	127			Reference Identification				 X 1	AN	1/50		

		OutputString += ParseANSIField(strPriorAuthID,1,50);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		//only output if we loaded both a qualifier and an ID
		// (j.jones 2011-12-30 11:35) - PLID 47267 - can only be sent if it is G1
		if(strPriorAuthQual == "G1" && strPriorAuthID != "") {
			EndANSISegment(OutputString);
		}

//196		180		REF		Payer Claim Control Number			S		1

		// (a.walling 2007-07-24 09:23) - PLID 26780 - Include Medicaid Resubmission code or Original Reference Number
		
		// (j.jones 2013-06-20 12:27) - PLID 57245 - If OrigRefNo_2300 is 1, then we won't fill this value on "Original"
		// claims, instead we only fill if if this is a Corrected, Replacement, or Void claim.
		// If OrigRefNo_2300 is 0, then we fill this on all claims, if a number was filled out.
		//
		// Additionally, I moved the value used here into the cached claim info.
		// This field is almost always BillsT.OriginalRefNo, but if they fill out BillsT.MedicaidResubmission
		// instead, we'll use that. In the event they fill out both (which they should not be doing), we use
		// MedicaidResubmission instead of OriginalRefNo.
		if ((eClaimTypeCode != ctcOriginal || m_HCFAInfo.OrigRefNo_2300 == 0)
			&& m_pEBillingInfo->strOriginalReferenceNumber.GetLength() > 0) {
			// we have a non-empty code here, so export it!

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "REF";

			//REF01 128			Reference Identification Qualifier		M 1	ID	2/3
			//F8 - Original Reference Number (ICN/DCN)
			OutputString += ParseANSIField("F8", 2, 3);
			
			//REF02 127			Reference Identification				X 1	AN	1/50			
			OutputString += ParseANSIField(m_pEBillingInfo->strOriginalReferenceNumber, 1, 50);

			//REF03 NOT USED
			//REF04 NOT USED
			
			// already checked for length of strMedicaidResubmission above
			EndANSISegment(OutputString);
		}

//199		180		REF		CLIA Number								S		1

		// (j.jones 2006-10-31 08:49) - PLID 23285 - if the CLIA number should be used
		// on this claim, here is where it needs to be exported

		// (j.jones 2011-03-07 14:09) - PLID 42660 - the CLIA number is now calculated
		// by our caller and is passed in
		if(eCLIANumber.bUseCLIANumber && !eCLIANumber.strCLIANumber.IsEmpty()) {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "REF";

			//REF01 128			Reference Identification Qualifier		M 1	ID	2/3
			//X4 - Clinical Laboratory Improvement Amendment Number
			OutputString += ParseANSIField("X4",2,3);
			
			//REF02 127			Reference Identification				X 1	AN	1/50
			
			//use the CLIA number returned earlier
			OutputString += ParseANSIField(eCLIANumber.strCLIANumber,1,50);

			//REF03 NOT USED
			//REF04 NOT USED

			//don't output if blank
			if(eCLIANumber.strCLIANumber != "")
				EndANSISegment(OutputString);
		}

//199		180		REF		Repriced Claim Number					S		1
//201		180		REF		Investigational Device Exemption Number	S		1
//202		180		REF		Claim ID Number for Transmission Intermediaries		S		1
//204		180		REF		Medical Record Number					S		1
//205		180		REF		Demonstration Project Identifier		S		1
//206		180		REF		Care Plan Oversight						S		1
//207		185		K3		File Information						S		10

		//not supported

//209		190		NTE		Claim Note								S		1

		// (j.jones 2008-10-03 10:13) - PLID 31578 - HCFABox19 may be sendable in the 2300 or 2400 NTE
		// (j.jones 2008-10-06 10:43) - PLID 31580 - same with the SendCorrespondence feature

		BOOL bSendCorrespondence = ReturnsRecords("SELECT SendCorrespondence FROM BillsT WHERE SendCorrespondence = 1 AND ID = %li",m_pEBillingInfo->BillID);

		if((m_HCFAInfo.ANSI_SendBox19 == 1 && m_HCFAInfo.ANSI_SendBox19Segment == 0)
			|| (m_HCFAInfo.ANSI_SendCorrespSegment == 0 && bSendCorrespondence)) {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "NTE";

			//NTE01 363			Note Reference Code						O 1	ID	3/3

			//ADD - Additional Information
			//CER - Certification Narrative
			//DCP - Goals, Rehabilitation Potential, or Discharge Plans
			//DGN - Diagnosis Description
			//PMT - Payment
			//TPO - Third Party Organization Notes

			OutputString += ParseANSIField("ADD",3,3);

			//NTE02 352			Description								M 1	AN	1/80

			//we can only do one or the other, correspondence takes precedence
			// (j.jones 2008-10-06 10:43) - PLID 31580 - this setting can be configured to run
			// in either the 2300 or 2400 loop
			if((m_HCFAInfo.ANSI_SendCorrespSegment == 0 && bSendCorrespondence)) {
				//send correspondence date

				// (j.jones 2007-03-15 12:02) - PLID 25224 - do not default to using a colon after MAIL DOC

				CString strPreceding = "MAIL DOC";
				if(!m_HCFAInfo.strCorrespondenceNote.IsEmpty())
					strPreceding = m_HCFAInfo.strCorrespondenceNote;

				strPreceding.TrimRight();

				CString strOutput;
				strOutput.Format("%s %s",strPreceding,COleDateTime::GetCurrentTime().Format("%m-%d-%y"));

				// (j.jones 2007-03-15 12:03) - PLID 25224 - disallow colons
				strOutput.Replace(":"," ");

				OutputString += ParseANSIField(strOutput,1,80);

				EndANSISegment(OutputString);				
			}
			// (j.jones 2008-10-06 11:16) - PLID 31578 - HCFABox19 may be sendable in the 2300 or 2400 NTE
			else if(m_HCFAInfo.ANSI_SendBox19 == 1 && m_HCFAInfo.ANSI_SendBox19Segment == 0) {

				//send block 19
				CString strHCFABlock19 = "";

				_RecordsetPtr rs = CreateParamRecordset("SELECT HCFABlock19 FROM BillsT WHERE ID = {INT}", m_pEBillingInfo->BillID);
				if(!rs->eof) {
					strHCFABlock19 = AdoFldString(rs, "HCFABlock19","");
					strHCFABlock19.TrimLeft();
					strHCFABlock19.TrimRight();
				}
				rs->Close();

				// (j.jones 2007-03-15 12:03) - PLID 25224 - disallow colons
				strHCFABlock19.Replace(":"," ");

				OutputString += ParseANSIField(strHCFABlock19,1,80);

				if(!strHCFABlock19.IsEmpty())
					EndANSISegment(OutputString);
			}
		}

//211		195		CR1		Ambulance Transport Information			S		1
//214		200		CR2		Spinal Manipulation Service Information	S		1
//216		220		CRC		Ambulance Certification					S		3
//219		220		CRC		Patient Condition Information: Vision	S		3
//221		220		CRC		Homebound Indicator						S		1
//223		220		CRC		EPSDT Referral							S		1

		//not used

//226		231		HI		Health Care Diagnosis Code				S		1
		

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//HI01	C022		Health Care Code Info.					M 1
		//HI01-1	1270	Code List Qualifier Code				M 1	ID	1/3
		////HI01-2	1271	Industry Code							M 1	AN	1/30		
		////HI01-3	NOT USED
		////HI01-4	NOT USED
		////HI01-5	NOT USED
		////HI01-6	NOT USED
		////HI01-7	NOT USED
		////HI02	C022		Health Care Code Info.					O 1
		////HI02-1	1270	Code List Qualifier Code				M 1	ID	1/3
		////HI02-2	1271	Industry Code							M 1	AN	1/30		
		////HI02-3	NOT USED
		////HI02-4	NOT USED
		////HI02-5	NOT USED
		////HI02-6	NOT USED
		////HI02-7	NOT USED
		////HI03	C022		Health Care Code Info.					O 1		
		////HI03-2	1270	Code List Qualifier Code				M 1	ID	1/3
		////HI03-2	1271	Industry Code							M 1	AN	1/30
		////HI03-3	NOT USED
		////HI03-4	NOT USED
		////HI03-5	NOT USED
		////HI03-6	NOT USED
		////HI03-7	NOT USED
		////HI04	C022		Health Care Code Info.					O 1		
		////HI04-2	1270	Code List Qualifier Code				M 1	ID	1/3
		////HI04-2	1271	Industry Code							M 1	AN	1/30
		//HI04-3	NOT USED
		//HI04-4	NOT USED
		//HI04-5	NOT USED
		//HI04-6	NOT USED
		//HI04-7	NOT USED
		//HI05	C022		Health Care Code Info.					O 1
		//HI06	C022		Health Care Code Info.					O 1
		//HI07	C022		Health Care Code Info.					O 1
		//HI08	C022		Health Care Code Info.					O 1
		//HI09	C022		Health Care Code Info.					O 1
		//HI10	C022		Health Care Code Info.					O 1
		//HI11	C022		Health Care Code Info.					O 1
		//HI12	C022		Health Care Code Info.					O 1

		OutputString = "HI";

		BOOL bOutput = TRUE;

		bool bUseICD10 = ShouldUseICD10();

		// (j.jones 2011-04-20 10:45) - PLID 41490 - if bSkipDiagnosisCodes is TRUE,
		// we need to pull our diagnosis codes from aryNewDiagnosisCodesToExport
		// (d.singleton 2014-03-05 15:23) - PLID 61195 - update ANSI5010 claim export to support ICD10 and new diagcode billing structure
		// (a.walling 2014-03-19 9:13) - PLID 61419 - EBilling - ANSI5010 - Get diag info from bill
		for(int i = 0; i < (int)m_pEBillingInfo->billDiags.size() && i < 12; i++) {
			//use appropriate qualifier for primary code
			const Nx::DiagCode& diagCode = m_pEBillingInfo->billDiags[i];
			if(i == 0) {
				if(bUseICD10) {
					//HI01-1	1270	Code List Qualifier Code				M 1	ID	1/3

					//BK - Principal Diagnosis (ICD-9)
					//ABK - Principal Diagnosis (ICD-10)
					OutputString += ParseANSIField("ABK",1,3);
					str = diagCode.number;
					str.Replace(".","");
					str.TrimRight();
					if(str.IsEmpty()) {
						bOutput = FALSE;
					}
					//HI01-2	1271	Industry Code							M 1	AN	1/30
					OutputString += ":";
					OutputString += str;
				}
				else {
					//HI01-1	1270	Code List Qualifier Code				M 1	ID	1/3

					//BK - Principal Diagnosis (ICD-9)
					//ABK - Principal Diagnosis (ICD-10)
					OutputString += ParseANSIField("BK",1,3);
					str = diagCode.number;
					str.Replace(".", "");
					str.TrimRight();
					if(str.IsEmpty()) {
						bOutput = FALSE;
					}
					//HI01-2	1271	Industry Code							M 1	AN	1/30
					OutputString += ":";
					OutputString += str;
				}
			}
			else {
				//use appropriate qualifier for secondary codes
				if(bUseICD10) {					
					str = diagCode.number;
					str.Replace(".","");
					str.TrimRight();
					if(!str.IsEmpty()) {
						//HI0#-2	1270	Code List Qualifier Code				M 1	ID	1/3

						//BF - Diagnosis (ICD-9)
						//ABF - Diagnosis (ICD-10)
						OutputString += ParseANSIField("ABF",1,3);
						//HI0#-2	1271	Industry Code							M 1	AN	1/30
						OutputString += ":";
						OutputString += str;
					}					
				}
				else {					
					str = diagCode.number;
					str.Replace(".", "");
					str.TrimRight();
					if(!str.IsEmpty()) {
						//HI0#-2	1270	Code List Qualifier Code				M 1	ID	1/3

						//BF - Diagnosis (ICD-9)
						//ABF - Diagnosis (ICD-10)
						OutputString += ParseANSIField("BF",1,3);
						//HI0#-2	1271	Industry Code							M 1	AN	1/30
						OutputString += ":";
						OutputString += str;
					}
				}
			}
		}	

		if(bOutput) {
			EndANSISegment(OutputString);
		}
///////////////////////////////////////////////////////////////////////////////

//239		231		HI		Anesthesia Related Procedure				S		1
//242		231		HI		Condition Information						S		2
//252		241		HCP		Claim Pricing/Repricing Information			S		1
		//not supported

///////////////////////////////////////////////////////////////////////////////

		rs->Close();

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2300_Prof");

	return Error_Other;
}

// (j.jones 2010-10-18 16:17) - PLID 40346 - initial implementation of ANSI 5010 Institutional claim
int CEbilling::ANSI_5010_2300_Inst() {

	//Claim Information

	//if the patient is the subscriber, 2300 loop follows the 2010BD loop
	//if the patient is not the subscriber, 2300 loop follows 2010CA

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2300\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	//increment the count for the current claim
	m_ANSIClaimCount++;

	try {

		CString OutputString,str;
		_variant_t var;
		
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		// (d.singleton 2014-03-06 07:53) - PLID 61195 - update ANSI5010 claim export to support ICD10 and new diagcode billing structure
		_RecordsetPtr rs = CreateParamRecordset("SELECT BillsT.*, PlaceofServiceCodesT.PlaceCodes FROM BillsT LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "	
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
			"WHERE BillsT.ID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null ",m_pEBillingInfo->BillID);
			
		if(rs->eof) {
			//if the recordset is empty, there is no bill. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open bill information for '%s', Bill ID %li. (ANSI_2300_Inst)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open bill information. (ANSI_2300_Inst)";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		FieldsPtr fields = rs->Fields;

		UB04::ClaimInfo ub04ClaimInfo = UB04::ClaimInfo::FromXml(AdoFldString(rs, "UB04ClaimInfo", ""));

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2300 - CLAIM INFORMATION								100

//143		130		CLM		Claim Information						R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "CLM";

		//CLM01	1028		Claim Submitter's Identifier			M 1	AN	1/38

		//PatientsT.UserDefinedID
		
		str.Format("%li",m_pEBillingInfo->UserDefinedID);

		// (j.jones 2009-10-01 14:21) - PLID 35711 - added the ability to insert text in front of the patient ID
		if(m_bPrependPatientID) {
			str = m_strPrependPatientIDCode + str;
		}

		OutputString += ParseANSIField(str,1,38);

		//CLM02	782			Monetary Amount							O 1	R	1/18

		//this is the total charge amount for the claim

		CString strDocProv = "";
		//if using the bill provider, filter only on that doctor's charges, otherwise get all charges
		//in ANSI, we're using the bill provider if we are using anything but the G1 provider

		// (j.jones 2006-12-01 15:50) - PLID 22110 - supported the ClaimProviderID
		// (d.thompson 2011-09-12) - PLID 45393 - Properly follow ClaimProviderID on the charge when filtering the charges
		if(m_UB92Info.Box82Setup == 1) {
			strDocProv.Format("AND (ChargesT.ClaimProviderID = %li OR (ClaimProviderID IS NULL AND DoctorsProviders IN (SELECT PersonID FROM ProvidersT WHERE ClaimProviderID = %li))) ", m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->ANSI_RenderingProviderID);
		}

		//this cannot be parameterized
		// (j.jones 2010-03-12 16:31) - PLID 37740 - need the earliest ServiceDateFrom and latest ServiceDateTo
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		_RecordsetPtr rsCharges = CreateRecordset("SELECT Sum(dbo.GetChargeTotal(ChargesT.ID)) AS ChargeTotal, "
			"Min(ServiceDateFrom) AS MinServiceDateFrom, Max(ServiceDateTo) AS MaxServiceDateTo "
			"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE ChargesT.BillID = %li AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"%s "
			"GROUP BY ChargesT.BillID",m_pEBillingInfo->BillID, strDocProv);

		COleCurrency cyChargeTotal = COleCurrency(0,0);
		COleDateTime dtMinServiceDate, dtMaxServiceDate;
		dtMinServiceDate.SetStatus(COleDateTime::invalid);
		dtMaxServiceDate.SetStatus(COleDateTime::invalid);
		if(!rsCharges->eof) {
			cyChargeTotal = AdoFldCurrency(rsCharges, "ChargeTotal", COleCurrency(0,0));
			str = FormatCurrencyForInterface(cyChargeTotal,FALSE,FALSE);

			//see if we need to trim the zeros
			// (j.jones 2012-05-25 16:29) - PLID 50657 - now a modular function, truncates if m_bTruncateCurrency is enabled
			str = FormatANSICurrency(str);

			dtMinServiceDate = AdoFldDateTime(rsCharges, "MinServiceDateFrom");
			dtMaxServiceDate = AdoFldDateTime(rsCharges, "MaxServiceDateTo");
		}
		else
			str = "0";
		OutputString += ParseANSIField(str,1,18);

		rsCharges->Close();

		//CLM03	NOT USED
		OutputString += "*";

		//CLM04	NOT USED
		OutputString += "*";

		//CLM05	C023		Health Care Service Location Info.		O 1

		// (j.jones 2004-10-20 16:49) - This is absurd. Essentially CLM05 is
		//the three digit number from the UB92 Box 4. However, it's sent as
		//the first two digits, an A, and then the third digit.
		//So if UB92 Box 4 is 741, we send 74:A:1

		//We allow for a default for Box 4 but cannot reliably track when it changed.
		//We may need to add support for this (on the bill?) in the future.

		// (j.jones 2016-05-24 9:12) - NX-100706 - This logic has changed slightly.
		// The third digit for Box 4 is now the claim type dropdown on the bill.
		// The first two digits continue to come from the Box 4 setup.
		// However, on paper there can be a leading 0, in ANSI there cannot.
		// So a Box 4 setup of "831", "0831", "083", or "83" will still all export
		// as "83".

		CString strBox4 = m_UB92Info.Box4;
		strBox4.TrimLeft();
		strBox4.TrimRight();

		// (j.jones 2011-07-06 16:21) - PLID 44358 - see if any charge has a Box 4 override,
		// and if so, use the first one
		CSqlFragment sqlDocProv(strDocProv);
		_RecordsetPtr rsChargeBox4 = CreateParamRecordset("SELECT TOP 1 RTRIM(LTRIM(ServiceT.UBBox4)) AS Box4 "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE ChargesT.BillID = {INT} AND LineItemT.Deleted = 0 "
			"AND ChargesT.Batched = 1 {SQL} "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND RTRIM(LTRIM(ServiceT.UBBox4)) <> '' "
			"ORDER BY ChargesT.LineID", m_pEBillingInfo->BillID, sqlDocProv);
		if(!rsChargeBox4->eof) {
			strBox4 = VarString(rsChargeBox4->Fields->Item["Box4"]->Value);
		}
		rsChargeBox4->Close();
		
		//CLM05-1	1331	Facility Code Value						M	AN	1/2

		//left two digits from the Box 4 override, excluding leading zeroes,

		// (j.jones 2016-05-24 09:32) - NX-100706 - We no longer care
		// about the right-most digit, we just need the left two digits,
		// excluding any leading zeroes.
		// If we do not have two digits, replace with "83".
		strBox4.TrimLeft("0");
		CString strBox4Left = strBox4.Left(2);
		if (strBox4Left.GetLength() < 2) {
			//use the default of 83, even if we have
			//one number and not two
			strBox4Left = "83";
		}

		OutputString += ParseANSIField(strBox4Left,1,2);

		//CLM05-2	1332	Facility Code Qualifier					O	ID	1/2

		//static A, separated by a :
		OutputString += ":A";

		//CLM05-3	1325	Claim Frequency Type Code				O	ID	1/1

		// (j.jones 2016-05-24 09:34) - NX-100706 - This is now the
		// claim type dropdown value from the bill, 1 if unknown.
		// This is similar to, but not the same list as a HCFA.
		
		ANSI_ClaimTypeCode eClaimTypeCode = (ANSI_ClaimTypeCode)AdoFldLong(rs, "ANSI_ClaimTypeCode");
		//***the ANSI_ClaimTypeCode enum is not the same value as what must be submitted on the claim
		CString strClaimTypeCode = GetClaimTypeCode_UB(eClaimTypeCode);

		OutputString += ":";
		OutputString += strClaimTypeCode;

		//CLM06	NOT USED
		OutputString += "*";

		//CLM07	1359		Provider Accept Code					O 1	ID	1/1

		//InsuranceAcceptedT.Accepted
		BOOL bAccepted = TRUE;
		str = "C";
		if(m_pEBillingInfo->Box82Setup != 3) {
			// (j.jones 2010-07-23 15:02) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsCo now
			bAccepted = GetAcceptAssignment_ByInsCo(m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ANSI_RenderingProviderID);
		}
		// (j.jones 2007-05-10 14:46) - PLID 25948 - if referring physician, we don't know, so accept if any of our providers accept
		else if(m_pEBillingInfo->Box82Setup == 3) {
			// (j.jones 2010-07-23 15:09) - PLID 39783 - now only set accepted to false if no active providers accept,
			// so simply search for whether any provider accepts (which they would do if a InsuranceAcceptedT record was missing)
			_RecordsetPtr rsAcc = CreateParamRecordset("SELECT TOP 1 PersonID "
				"FROM ProvidersT "
				"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
				"LEFT JOIN (SELECT ProviderID, Accepted FROM InsuranceAcceptedT WHERE InsuranceCoID = {INT}) AS InsuranceAcceptedQ "
				"	ON ProvidersT.PersonID = InsuranceAcceptedQ.ProviderID "
				"WHERE PersonT.Archived = 0 "
				"AND (InsuranceAcceptedQ.Accepted = 1 OR InsuranceAcceptedQ.Accepted Is Null)", m_pEBillingInfo->InsuranceCoID);
			if(rsAcc->eof) {
				//empty, which means no active providers accept assignment
				bAccepted = FALSE;
			}
			rsAcc->Close();
		}

		if(bAccepted)
			str = "A";

		OutputString += ParseANSIField(str,1,1);

		//CLM08	1073		Yes/No Cond Resp Code					O 1	ID	1/1

		// (j.jones 2010-07-27 11:42) - PLID 39784 - this is the "Assignment of Benefits Indicator",
		//"indicates insured or authorized person authorizes benefits to be assigned to the provider".

		//This needs to follow the UB group's setting for Box 53, and can be potentially overridden
		//by a setting on the bill.

		BOOL bFillBox53 = TRUE;
		if(m_UB92Info.Box53Accepted == 0) {
			//fill if accepted
			bFillBox53 = bAccepted;
		}
		else if(m_HCFAInfo.Box13Accepted == 1) {
			//always yes
			bFillBox53 = TRUE;
		}
		else if(m_HCFAInfo.Box13Accepted == 2) {
			//always no
			bFillBox53 = FALSE;
		}

		//this field & enum is a HCFA name, but it applies to UBs as well
		HCFABox13Over hb13Value = (HCFABox13Over)VarLong(fields->Item["HCFABox13Over"]->Value, (long)hb13_UseDefault);		
		if(hb13Value == hb13_No || (hb13Value == hb13_UseDefault && !bFillBox53)) {
			str = "N";
		}
		else {
			str = "Y";
		}
		OutputString += ParseANSIField(str,1,1);

		//CLM09	1363		Release of Info Code					O 1	ID	1/1

		//static "Y"
		OutputString += ParseANSIField("Y",1,1);

		//CLM10	NOT USED
		OutputString += "*";
		//CLM11	NOT USED
		OutputString += "*";
		//CLM12	NOT USED
		OutputString += "*";
		//CLM13	NOT USED
		OutputString += "*";
		//CLM14	NOT USED
		OutputString += "*";
		//CLM15	NOT USED
		OutputString += "*";
		//CLM16	NOT USED
		OutputString += "*";
		//CLM17	NOT USED
		OutputString += "*";		
		//CLM18	NOT USED
		OutputString += "*";
		//CLM19	NOT USED
		OutputString += "*";

		//CLM20	1514		Delay Reason Code						O 1	ID	1/2
		//we do not support this
		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//149		135		DTP		Discharge Hour							S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2013-06-10 09:15) - PLID 41479 - supported discharge hour

		// BillsT.DischargeTime
		_variant_t varDischargeTime = rs->Fields->Item["DischargeTime"]->Value;
		if(varDischargeTime.vt == VT_DATE) {
			// (b.spivey July 8, 2015) PLID 66516 - Do not fill if we're not supposed to. 
			if (m_UB92Info.FillBox16) {			
				OutputString = "DTP";

				//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

				//static "096"
				OutputString += ParseANSIField("096",3,3);

				//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

				//static "TM" - (HHMM)
				OutputString += ParseANSIField("TM",2,3);

				//DTP03	1251		Date Time Period						M 1	AN	1/35

				COleDateTime dt = VarDateTime(varDischargeTime);
				str = dt.Format("%H%M");
				OutputString += ParseANSIField(str,1,35);

				EndANSISegment(OutputString);
			}
		}

///////////////////////////////////////////////////////////////////////////////

//150		135		DTP		Statement Dates							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

		//static "434"
		OutputString += ParseANSIField("434",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

		//in 5010, this is always two dates, even if they are the same date
		CString strDate;
		strDate.Format("%s-%s",dtMinServiceDate.Format("%Y%m%d"), dtMaxServiceDate.Format("%Y%m%d"));

		//"RD8" for two dates
		OutputString += ParseANSIField("RD8",2,3);

		//DTP03	1251		Date Time Period						M 1	AN	1/35

		//CCYYMMDD-CCYYMMDD
		// (j.jones 2012-10-30 16:49) - PLID 53364 - do not strip punctuation here, RD8 dates can have hyphens
		OutputString += ParseANSIField(strDate, 1, 35, FALSE, 'L', ' ', TRUE);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//151		135		DTP		Admission Date/Hour						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2008-06-09 15:47) - PLID 30229 - this should be the HospFrom date

		_variant_t varHospFrom = rs->Fields->Item["HospFrom"]->Value;
		if(varHospFrom.vt == VT_DATE) {
			// (b.spivey July 8, 2015) PLID 66516 - Do not fill if we're not supposed to. 
			if (m_UB92Info.FillBox12_13) {
				OutputString = "DTP";

				//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

				//static "435"
				OutputString += ParseANSIField("435", 3, 3);

				//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

				//static "DT" - (CCYYMMDDHHMM)
				OutputString += ParseANSIField("DT", 2, 3);

				//DTP03	1251		Date Time Period						M 1	AN	1/35

				// (j.jones 2013-06-10 08:53) - PLID 41479 - supported AdmissionTime
				_variant_t varAdmissionTime = rs->Fields->Item["AdmissionTime"]->Value;

				//BillsT.HospFrom + BillsT.AdmissionTime
				COleDateTime dt = VarDateTime(varHospFrom);
				str = dt.Format("%Y%m%d");
				
				if (varAdmissionTime.vt == VT_DATE) {
					//time is in 24 hour time, like 0900 or 1630.
					dt = VarDateTime(varAdmissionTime);
					str += dt.Format("%H%M");
				}
				OutputString += ParseANSIField(str, 1, 35);

				if (!str.IsEmpty()) {
					EndANSISegment(OutputString);
				}
			}
		}

///////////////////////////////////////////////////////////////////////////////

//152		135		DTP		Repricer Received Date					S		1
		//not supported

//153		140		CL1		Institutional Claim Code				R		1
		
		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "CL1";

		//CL101	1315		Admission Type Code						R 1	ID	1/1

		//Required when patient is being admitted for inpatient services.

		//The A2 addenda changed this to Required, and noted that the code set used here is called
		//"Priority (Type) of Admission or Visit"

		//The specs do not give us the valid values here, I had to Google them:
		//1 - Emergency -	The patient requires immediate medical intervention as a result of severe, 
		//					life threatening or potentially disabling conditions. Generally, the
		//					patient is admitted through the emergency room.
		//2 - Urgent -		The patient requires immediate attention for the care and treatment of a
		//					physical or mental disorder. Generally the patient is admitted to the
		//					first available and suitable accommodation.
		//3 - Elective -	The patient’s condition permits adequate time to schedule the availability
		//					of a suitable accommodation.
		//4 - Newborn -		Use of this code necessitates the use of a special Source of Admission code
		//5 - Trauma Center - Visit to a trauma center/hospital as licensed or designated by the state
		//					or local government authority authorized to do so, or as verified by the
		//					American College of Surgeons and involving a trauma activation.
		//9 - Information not Available

		//This is Box 14 on the UB04 claim - Admission Type.

		// (j.jones 2012-01-19 11:32) - PLID 47554 - pull from the UB Group's default Box 14 value
		// (stored as Box19 from the UB92 days), but send 9 if it is blank
		// (j.jones 2012-05-15 09:08) - PLID 50376 - The UB group's default now loads into the bill,
		// and the field is now editable on the bill itself. So pull from the bill, not from the group.
		str = VarString(rs->Fields->Item["UBBox14"]->Value, "");
		str.TrimLeft(); str.TrimRight();
		if(str.IsEmpty()) {
			str = "9";
		}
		OutputString += ParseANSIField(str,1,1);

		//CL102	1314		Admission Source Code					O 1	ID	1/1

		//Required for all inpatient and outpatient services.

		//the A2 addenda keeps this optional, but noted that the code set used here is called
		//"Point of Origin for Admission or Visit"

		//again, the specs do not give us valid values to use, but I found
		//these on the internet:
		
		//1 - Non-Health Care Facility Point of Origin
		//		The patient was admitted to this facility upon the order of a physician.
		//2 - Clinic
		//		The patient was admitted to this facility.
		//4 - Transfer from a Hospital (Different Facility)
		//		The patient was admitted to this facility as a hospital transfer from an acute care facility.
		//5 - Transfer from a Skilled Nursing Facility (SNF) or Intermediate Care Facility (ICF)
		//		The patient was admitted to this facility as a transfer from an SNF or ICF.
		//6 - Transfer from another Health Care Facility
		//		The patient was admitted to this facility as a transfer from another type of health care facility not defined elsewhere in this code list.
		//7 - Emergency Room
		//8 - Court/Law Enforcement
		//		The patient was admitted to this facility upon the direction of a court of law,
		//		or upon the request of a law enforcement agency representative.
		//9 - Information is Not Available
		//		The means by which the patient was admitted to this hospital is not known.
		//B - Transfer from another Home Health Agency
		//C - Readmission to same Home Health Agency
		//D - Transfer from One Distinct Unit of the Hospital to Another Distinct Unit of the Same Hospital
		//		The patient was admitted to this facility as a transfer from hospital inpatient within the hospital resulting in a separate claim to the payer.
		//E - Transfer from an Ambulatory Surgery Center
		//		The patient was admitted to this facility as a transfer from an ambulatory surgery center.
		//F - Transfer from Hospice and under Hospice Plan of Care
		//		The patient was admitted to this facility as a transfer from hospice.
		//**Code Structure for Newborn**
		//5 - Born Inside Hospital
		//		A baby born inside this hospital.
		//6 - Born Outside this Hospital
		//		A baby born outside of this hospital.

		//This is Box 15 on the UB04 claim - Admission Source.

		// (j.jones 2012-01-19 11:38) - PLID 47554 - pull from the UB Group's default Box 15 value
		// (stored as Box20 from the UB92 days), but send 1 if it is blank
		// (j.jones 2012-05-15 09:08) - PLID 50376 - The UB group's default now loads into the bill,
		// and the field is now editable on the bill itself. So pull from the bill, not from the group.
		str = VarString(rs->Fields->Item["UBBox15"]->Value, "");
		str.TrimLeft(); str.TrimRight();
		if(str.IsEmpty()) {
			str = "1";
		}
		OutputString += ParseANSIField(str,1,1);

		//CL103	1352		Patient Status Code						O 1	ID	1/2

		//same as above, no options given in the specs, found these online:
		//01 - Discharged to home or self-care (routine)
		//04 - Discharged to an Intermediate Care Facility
		//05 - Discharged to another type of institution for inpatient care or referred for
		//	   outpatient services to another institution
		//07 - Left against medical advice or discontinued care
		//30 - Still a patient

		//we do not support this, send 01 for now
		OutputString += ParseANSIField("01",1,2);

		//CL104	NOT USED
		OutputString += "*";
		
		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//154		155		PWK		Claim Supplemental Information			S		10

		// (a.walling 2007-08-27 10:09) - PLID 27026
		BOOL bSendPaperwork = AdoFldBool(fields, "SendPaperwork", FALSE);

		if (bSendPaperwork) {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			CString strType = AdoFldString(fields, "PaperworkType", ""), strTx = AdoFldString(fields, "PaperworkTx", "");

			if ( (strType.GetLength() < 2) || (strTx.GetLength() < 1) ) {
				str.Format("If sending paperwork, both the type code and transmission code must be filled in correctly. Patient '%s', Bill ID %li.",CString(rs->Fields->Item["Name"]->Value.bstrVal),m_pEBillingInfo->BillID);

				AfxMessageBox(str);

				return Error_Missing_Info;
			}

			OutputString = "PWK";

			//PWK01 755		Report Type Code						M 1	ID	2/2
			/*
				03 Report Justifying Treatment Beyond Utilization Guidelines
				04 Drugs Administered
				05 Treatment Diagnosis
				06 Initial Assessment
				07 Functional Goals
				08 Plan of Treatment
				09 Progress Report
				10 Continued Treatment
				11 Chemical Analysis
				13 Certified Test Report
				15 Justification for Admission
				21 Recovery Plan
				A3 Allergies/Sensitivities Document
				A4 Autopsy Report
				AM Ambulance Certification
				AS Admission Summary
				B2 Prescription
				B3 Physician Order
				B4 Referral Form
				BR Benchmark Testing Results
				BS Baseline
				BT Blanket Test Results
				CB Chiropractic Justification
				CK Consent Form(s)
				CT Certification
				D2 Drug Profile Document
				DA Dental Models
				DB Durable Medical Equipment Prescription
				DG Diagnostic Report
				DJ Discharge Monitoring Report
				DS Discharge Summary
				EB Explanation of Benefits (Coordination of Benefits or
				Medicare Secondary Payor)
				HC Health Certificate
				HR Health Clinic Records
				I5 Immunization Record
				IR State School Immunization Records
				LA Laboratory Results
				M1 Medical Record Attachment
				MT Models
				NN Nursing Notes
				OB Operative Note
				OC Oxygen Content Averaging Report
				OD Orders and Treatments Document
				OE Objective Physical Examination (including vital
				signs) Document
				OX Oxygen Therapy Certification
				OZ Support Data for Claim
				P4 Pathology Report
				P5 Patient Medical History Document
				PE Parenteral or Enteral Certification
				PN Physical Therapy Notes
				PO Prosthetics or Orthotic Certification
				PQ Paramedical Results
				PY Physician’s Report
				PZ Physical Therapy Certification
				RB Radiology Films
				RR Radiology Reports
				RT Report of Tests and Analysis Report
				RX Renewable Oxygen Content Averaging Report
				SG Symptoms Document
				V5 Death Notification
				XP Photographs
			*/

			OutputString += ParseANSIField(AdoFldString(fields, "PaperworkType", ""), 2, 2);

			//PWK02	756		Report Transmission Code				O 1	ID	1/2
			/*
				AA Available on Request at Provider Site
					Paperwork is available at the provider’s site. This
					means that the paperwork is not being sent with the
					claim at this time. Instead, it is available to the payer
					(or appropriate entity) at his or her request.
				BM By Mail
				EL Electronically Only
				EM E-Mail
				FX By Fax
				FT File Transfer
			*/

			OutputString += ParseANSIField(AdoFldString(fields, "PaperworkTx", ""), 1, 2);

			//PWK03 NOT USED
			OutputString += "*";

			//PWK04 NOT USED
			OutputString += "*";

			//PWK05 66		Identification Code Qualifier			X 1	ID	1/2
			// required when PWK02 != 'AA'. Can be used anytime when Provider wants
			// to send a document control number for an attachment remaining at the
			// Provider's office.
			// AC	Attachment Control Number.

			str = "AC";
			if(strTx == "AA") {
				str = "";
			}
			OutputString += ParseANSIField(str, 1, 2);

			//PWK06 67		Identification Code						X 1	AN	2/80
			// code identifying a party or other code. We will send the Bill's ID.

			CString strIDCode;
			strIDCode.Format("%lu", m_pEBillingInfo->BillID);
			OutputString += ParseANSIField(strIDCode, 2, 80);

			//PWK07 NOT USED
			OutputString += "*";
			//PWK08 NOT USED
			OutputString += "*";
			//PWK09	NOT USED
			OutputString += "*";

			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//158		160		CN1		Contract Information					S		1

		// (j.jones 2006-11-27 17:01) - PLID 23652 - use the contract information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending contract information
		if(!m_pEBillingInfo->IsPrimary && m_UB92Info.ANSI_EnablePaymentInfo == 1
			&& m_UB92Info.ANSI_2300Contract == 1) {

			OutputString = "CN1";

			//Ref.	Data		Name									Attributes
			//Des.	Element

			//CN101 1166	Contract Type Code						M 1	ID	2/2

			//01 - Diagnosis Related Group (DRG)
			//02 - Per Diem
			//03 - Variable Per Diem
			//04 - Flat
			//05 - Capitated
			//06 - Percent
			//09 - Other

			//apparently, this only needs to be 09 and the claim total
			str = "09";
			OutputString += ParseANSIField(str,2,2);

			//CN102 782		Monetary Amount							O 1	R	1/18

			//use the cyChargeTotal from earlier, to show the claim total
			str = FormatCurrencyForInterface(cyChargeTotal,FALSE,FALSE);

			//see if we need to trim the zeros
			// (j.jones 2012-05-25 16:29) - PLID 50657 - now a modular function, truncates if m_bTruncateCurrency is enabled
			str = FormatANSICurrency(str);

			OutputString += ParseANSIField(str,1,18);

			//CN103 332		Percent, Decimal Format					O 1	R	1/6

			//we don't use this

			OutputString += ParseANSIField("",1,6);

			//CN104 127		Reference Identification				O 1	AN	1/50

			//we don't use this

			OutputString += ParseANSIField("",1,50);

			//CN105 338		Terms Discount Percent					O 1	R	1/6

			//we don't use this

			OutputString += ParseANSIField("",1,6);

			//CN106 799		Version Identifier						O 1	AN	1/30

			//we don't use this

			OutputString += ParseANSIField("",1,30);

			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//160		175		AMT		Patient Estimated Amount Due			S		1

		//Required when the Patient Responsibility Amount is applicable to this claim.
		//AMT*F3 qualifier

		//we don't support this

///////////////////////////////////////////////////////////////////////////////

//161		180		REF		Service Authorization Exception Code	S		1
		
		//4N qualifier

		//we don't support this

///////////////////////////////////////////////////////////////////////////////

//163		180		REF		Referral Number							S		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01	128			Reference Identification Qualifier			M 1	ID	2/3
		
		//9F - Referral Number

		CString strRefNumQual = "";
		CString strRefNumID = "";

		//If the default qualifier is 9F, use that and the Prior Auth number,
		//otherwise check the setting in the bill

		strRefNumQual = m_UB92Info.PriorAuthQualifier;
		strRefNumQual.TrimLeft();
		strRefNumQual.TrimRight();
		if(strRefNumQual.IsEmpty()) {
			//if empty, then no override is in place,
			//so pick the qualifier from BillsT.PriorAuthType
			PriorAuthType patPriorAuthType = (PriorAuthType)VarLong(fields->Item["PriorAuthType"]->Value, (long)patPriorAuthNumber);
			if(patPriorAuthType == patReferralNumber) {
				strRefNumQual = "9F";
			}
		}

		if(strRefNumQual == "9F") {
			//BillsT.PriorAuthNum
			var = fields->Item["PriorAuthNum"]->Value;
			if(var.vt == VT_BSTR)
				strRefNumID = CString(var.bstrVal);
			else
				strRefNumID = "";
		}

		if(m_UB92Info.ANSI_SendRefPhyIn2300 == 1 && 
			!IsRecordsetEmpty("SELECT ID FROM BillsT INNER JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
					"WHERE ID = %li",m_pEBillingInfo->BillID)) {

			//they want to send this segment with the referring physician, so do it

			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT ReferringPhysT.NPI "
					"FROM BillsT LEFT JOIN PersonT ON BillsT.RefPhyID = PersonT.ID "
					"INNER JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
					"WHERE BillsT.ID = {INT}", m_pEBillingInfo->BillID);

			if(rs->eof) {
				//if the recordset is empty, there is no referring provider. So halt everything!!!
				rs->Close();
				// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
				CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
				if(!strPatientName.IsEmpty()) {
					// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
					str.Format("Could not open referring provider information for '%s', Bill ID %li. (ANSI_2300_Inst)", strPatientName, m_pEBillingInfo->BillID);
				}
				else
					//serious problems if you get here
					str = "Could not open referring provider information. (ANSI_2300_Inst)";
				
				AfxMessageBox(str);
				return Error_Missing_Info;
			}

			strRefNumQual = "9F";
			
			//always send NPI
			strRefNumID = AdoFldString(rs, "NPI", "");

			rs->Close();
		}

		OutputString += ParseANSIField(strRefNumQual,2,3);

		//REF02	127			Reference Identification					X 1	AN	1/50

		OutputString += ParseANSIField(strRefNumID,1,50);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		//only output if we loaded both a qualifier and an ID
		if(strRefNumQual == "9F" && strRefNumID != "") {
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//164		180		REF		Prior Authorization						S		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01	128			Reference Identification Qualifier		M 1	ID	2/3
		
		//G1 - Prior Authorization Number

		CString strPriorAuthQual = "";
		CString strPriorAuthID = "";

		//If the default qualifier is G1, use that and the Prior Auth number,
		//otherwise check the setting in the bill

		strPriorAuthQual = m_UB92Info.PriorAuthQualifier;
		strPriorAuthQual.TrimLeft();
		strPriorAuthQual.TrimRight();
		if(strPriorAuthQual.IsEmpty()) {
			//if empty, then no override is in place,
			//so pick the qualifier from BillsT.PriorAuthType
			PriorAuthType patPriorAuthType = (PriorAuthType)VarLong(fields->Item["PriorAuthType"]->Value, (long)patPriorAuthNumber);
			if(patPriorAuthType != patReferralNumber) {
				strPriorAuthQual = "G1";
			}
		}

		if(strPriorAuthQual == "G1") {
			//BillsT.PriorAuthNum
			var = fields->Item["PriorAuthNum"]->Value;
			if(var.vt == VT_BSTR)
				strPriorAuthID = CString(var.bstrVal);
			else
				strPriorAuthID = "";
		}
		OutputString += ParseANSIField(strPriorAuthQual,2,3);

		//REF02	127			Reference Identification				 X 1	AN	1/50		

		OutputString += ParseANSIField(strPriorAuthID,1,50);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		//only output if we loaded both a qualifier and an ID
		// (j.jones 2011-12-30 11:35) - PLID 47267 - can only be sent if it is G1
		if(strPriorAuthQual == "G1" && strPriorAuthID != "") {
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//166		180		REF		Payer Claim Control Number			S		1
		
		// (a.walling 2007-07-24 09:23) - PLID 26780 - Include Medicaid Resubmission code or Original Reference Number

		// (j.jones 2013-06-20 12:27) - PLID 57245 - If OrigRefNo_2300 is 1, then we won't fill this value, ever.
		// The similar HCFA setting applies only to "Original" claims, but on a UB it is just an on/off switch.
		//
		// Additionally, I moved the value used here into the cached claim info.
		// This field is almost always BillsT.OriginalRefNo, but if they fill out BillsT.MedicaidResubmission
		// instead, we'll use that. In the event they fill out both (which they should not be doing), we use
		// MedicaidResubmission instead of OriginalRefNo.
		// (j.jones 2016-05-24 15:43) - NX-100706 - UB claims now have corrected types
		if ((eClaimTypeCode != ctcOriginal || m_UB92Info.OrigRefNo_2300 == 0)
			&& m_pEBillingInfo->strOriginalReferenceNumber.GetLength() > 0) {
			// we have a non-empty code here, so export it!

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "REF";

			//REF01 128			Reference Identification Qualifier		M 1	ID	2/3
			//F8 - Original Reference Number (ICN/DCN)
			OutputString += ParseANSIField("F8", 2, 3);
			
			//REF02 127			Reference Identification				X 1	AN	1/50			
			OutputString += ParseANSIField(m_pEBillingInfo->strOriginalReferenceNumber, 1, 50);

			//REF03 NOT USED
			//REF04 NOT USED
			
			// already checked for length of strMedicaidResubmission above
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//167		180		REF		Repriced Claim Number					S		1
//168		180		REF		Adjusted Repriced Claim Number			S		1
//169		180		REF		Investigational Device Exemption Number	S		1
//170		180		REF		Claim Identifier For Transmission Intermediaries	S		1

		//not supported

///////////////////////////////////////////////////////////////////////////////

//172		180		REF		Auto Accident State						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		BOOL bRelatedToAutoAcc = VarBool(fields->Item["RelatedToAutoAcc"]->Value, FALSE);
		CString strState = VarString(fields->Item["State"]->Value, "");

		// (j.jones 2012-01-11 10:48) - PLID 47461 - trim and auto-capitalize the state
		strState.TrimLeft(); strState.TrimRight();
		strState.MakeUpper();

		if(bRelatedToAutoAcc && !strState.IsEmpty()) {

			OutputString = "REF";

			//REF01 128			Reference Identification Qualifier		M	ID	2/3
			//static LU
			OutputString += ParseANSIField("LU",2,3);
			
			//REF02 127			Reference Identification				X	AN	1/50
			//BillsT.State
			OutputString += ParseANSIField(strState,1,50);

			//REF03 NOT USED
			//REF04 NOT USED

			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//173		180		REF		Medical Record Number					S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01 128			Reference Identification Qualifier		M	ID	2/3
		//EA - Medical Record Identification Number
		OutputString += ParseANSIField("EA",2,3);
		
		//REF02 127			Reference Identification				X	AN	1/50
		//PatientsT.UserDefinedID
		str.Format("%li",m_pEBillingInfo->UserDefinedID);

		// (j.jones 2009-10-01 14:21) - PLID 35711 - added the ability to insert text in front of the patient ID
		if(m_bPrependPatientID) {
			str = m_strPrependPatientIDCode + str;
		}

		OutputString += ParseANSIField(str,1,50);

		//REF03 NOT USED
		//REF04 NOT USED

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//174		180		REF		Demonstration Project Identifier		S		1
//175		180		REF		Peer Review Org. (PRO) Approval Number	S		1
//176		185		K3		File Information						S		10
//178		190		NTE		Claim Note								S		10
		// b.spivey, March 7, 2016 -- PLID 68330 -- send remarks here- max 80 so enforce that by only taking the first 80 characters. 
		if (!ub04ClaimInfo.remarks.IsEmpty()) {
			OutputString = "NTE*ADD";
			// b.spivey, March 14, 2016 -- PLID 68330 -- trim new lines and punctuation 
			CString strRemarks = ub04ClaimInfo.remarks; 
			strRemarks.Replace("\r\n", " ");
			//incase a \r or \n is on its own, remove it
			strRemarks.Replace("\r", "");
			strRemarks.Replace("\n", "");
			strRemarks.TrimLeft();
			strRemarks.TrimRight(); 
			if (!strRemarks.IsEmpty()) {
				OutputString += FormatString("%s", ParseANSIField(strRemarks, 1, 80));
				EndANSISegment(OutputString);
			}
		}
//180		190		NTE		Billing Note							S		1
//181		220		CRC		EPSDT Referral							S		1

		//not supported

///////////////////////////////////////////////////////////////////////////////

//227		231		HI		Principal Diagnosis						R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HI";

		// (d.singleton 2014-03-06 07:53) - PLID 61195 - update ANSI5010 claim export to support ICD10 and new diagcode billing structure
		bool bUseICD10 = ShouldUseICD10();

		//HI01	C022		Health Care Code Info.					M 1

		//HI01-1	1270	Code List Qualifier Code				M	ID	1/3

		//BK - Principal Diagnosis (ICD-9)
		//ABK - Principal Diagnosis (ICD-10)

		//for now we use BK
		if(bUseICD10) {
			OutputString += ParseANSIField("ABK",1,3);	
		}
		else {
			OutputString += ParseANSIField("BK",1,3);
		}

		//HI01-2	1271	Industry Code							M	AN	1/30

		// (d.singleton 2014-03-06 07:53) - PLID 61195 - get our primary code for this bill
		// (a.walling 2014-03-19 9:13) - PLID 61419 - EBilling - ANSI5010 - Get diag info from bill
		CString strDiagCode1 = m_pEBillingInfo->GetSafeBillDiag(0).number;

		strDiagCode1.Replace(".","");
		strDiagCode1.TrimRight();

		OutputString += ":";
		OutputString += strDiagCode1;

		//HI01-3	NOT USED
		//HI01-4	NOT USED
		//HI01-5	NOT USED
		//HI01-6	NOT USED
		//HI01-7	NOT USED
		//HI01-8	NOT USED

		//HI01-9	1073	Yes/No Condition or Response Code		X ID 1/1

		//not supported

		//HI02 NOT USED
		//HI03 NOT USED
		//HI04 NOT USED
		//HI05 NOT USED
		//HI06 NOT USED
		//HI07 NOT USED
		//HI08 NOT USED
		//HI09 NOT USED
		//HI10 NOT USED
		//HI11 NOT USED
		//HI12 NOT USED

		if(strDiagCode1 != "") {
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//187		231		HI		Admitting Diagnosis						S		1

		OutputString = "HI";

		//HI01	C022		Health Care Code Info.					M 1

		//with no easy way to backtrack, we won't output this element at all
		//if the diag code is blank
		
		// (j.jones 2011-10-27 10:35) - PLID 45878 - This is Box 69 - Adm. Diag. Code,
		// formerly Box 76 on the UB92. If the setting is enabled to fill this box,
		// we use DiagCode 1 again, and this will look identical to the principal diagnosis
		// (BK qualifier), but use the BJ qualifier.
		str = "";
		if(m_UB92Info.Box76Show) {
			str = strDiagCode1;
		}

		//HI01-1	1270	Code List Qualifier Code				M	ID	1/3

		//BJ - Principal Diagnosis (ICD-9)
		//ABK - Principal Diagnosis (ICD-10)

		//for now we use BJ
		if(str != "") {
			// (d.singleton 2014-03-06 08:13) - PLID 61195 - update ANSI5010 claim export to support ICD10 and new diagcode billing structure
			if(bUseICD10) {
				OutputString += ParseANSIField("ABK",1,3);
			}
			else {
				OutputString += ParseANSIField("BJ",1,3);
			}
		}

		//HI01-2	1271	Industry Code							M	AN	1/30

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//HI01-3	NOT USED
		//HI01-4	NOT USED
		//HI01-5	NOT USED
		//HI01-6	NOT USED
		//HI01-7	NOT USED
		//HI01-8	NOT USED
		//HI01-9	NOT USED

		//HI02 NOT USED
		//HI03 NOT USED
		//HI04 NOT USED
		//HI05 NOT USED
		//HI06 NOT USED
		//HI07 NOT USED
		//HI08 NOT USED
		//HI09 NOT USED
		//HI10 NOT USED
		//HI11 NOT USED
		//HI12 NOT USED

		if(str != "") {
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//189		231		HI		Patient's Reason For Visit						S		1

		//PR or APR qualifier

		//we don't support this

///////////////////////////////////////////////////////////////////////////////

//193		231		HI		External Cause Of Injury				S		1

		//BN or ABN qualifier

		//E-Codes - we don't support this

///////////////////////////////////////////////////////////////////////////////

//218		231		HI		Diagnosis Related Group (DRG) Information		S		1

		//DR qualifier

		//we don't support this

///////////////////////////////////////////////////////////////////////////////

//220		231		HI		Other Diagnosis Information				S		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HI";

		BOOL bOutput = TRUE;

		//HI01	C022		Health Care Code Info.					M 1
		//HI01-2	1271	Industry Code							M	AN	1/30
		//HI01-3	NOT USED
		//HI01-4	NOT USED
		//HI01-5	NOT USED
		//HI01-6	NOT USED
		//HI01-7	NOT USED
		//HI01-8	NOT USED
		//HI01-9	1073	Yes/No Condition or Response Code		X ID 1/1
		//not supported
		//HI02	C022		Health Care Code Info.					O
		//HI02-1	1270	Code List Qualifier Code				M	ID	1/3
		//HI02-2	1271	Industry Code							M	AN	1/30
		//HI02-3	NOT USED
		//HI02-4	NOT USED
		//HI02-5	NOT USED
		//HI02-6	NOT USED
		//HI02-7	NOT USED
		//HI02-8	NOT USED
		//HI02-9	1073	Yes/No Condition or Response Code		X ID 1/1
		//not supported
		//HI03	C022		Health Care Code Info.					O
		//HI03-3	NOT USED
		//HI03-4	NOT USED
		//HI03-5	NOT USED
		//HI03-6	NOT USED
		//HI03-7	NOT USED
		//HI03-8	NOT USED
		//HI03-9	1073	Yes/No Condition or Response Code		X ID 1/1
		//not supported
		//HI04	C022		Health Care Code Info.					M	?	?
		//HI05	C022		Health Care Code Info.					M	?	?
		//HI06	C022		Health Care Code Info.					M	?	?
		//HI07	C022		Health Care Code Info.					M	?	?
		//HI08	C022		Health Care Code Info.					M	?	?
		//HI09	C022		Health Care Code Info.					M	?	?
		//HI10	C022		Health Care Code Info.					M	?	?
		//HI11	C022		Health Care Code Info.					M	?	?
		//HI12	C022		Health Care Code Info.					M	?	?
		//HI03-2	1270	Code List Qualifier Code				M	ID	1/3
		//HI03-2	1271	Industry Code							M	AN	1/30
		//HI0#-3	NOT USED
		//HI0#-4	NOT USED
		//HI0#-5	NOT USED
		//HI0#-6	NOT USED
		//HI0#-7	NOT USED
		//HI0#-8	NOT USED
		//HI0#-9	1073	Yes/No Condition or Response Code		X ID 1/1

		// (d.singleton 2014-03-06 08:20) - PLID 61195 - get recordset of our secondary codes ( 2 - 13 ) and loop through them. 13 total codes on claim
		// (a.walling 2014-03-19 9:13) - PLID 61419 - EBilling - ANSI5010 - Get diag info from bill
		for (int i = 1; i < (int)m_pEBillingInfo->billDiags.size() && i < 13; ++i) {
			long nIndex = i + 1;
			const Nx::DiagCode& diagCode = m_pEBillingInfo->billDiags[i];
			//different behavior for our first code
			if(nIndex == 2) {
				if(bUseICD10) {										
					//HI0#-#	1270	Code List Qualifier Code				M	ID	1/3

					//BF - Principal Diagnosis (ICD-9)
					//ABF - Principal Diagnosis (ICD-10)
					OutputString += ParseANSIField("ABF",1,3);
					str = diagCode.number;
					if(str.IsEmpty()) {
						bOutput = FALSE;
					}
					str.Replace(".", "");
					str.TrimRight();
					
					OutputString += ":";
					OutputString += str;								
				}
				else {
					//HI0#-#	1270	Code List Qualifier Code				M	ID	1/3

					//BF - Principal Diagnosis (ICD-9)
					//ABF - Principal Diagnosis (ICD-10)
					OutputString += ParseANSIField("BF",1,3);
					str = diagCode.number;
					if(str.IsEmpty()) {
						bOutput = FALSE;
					}
					str.Replace(".", "");
					str.TrimRight();
					
					OutputString += ":";
					OutputString += str;
				}
			}
			else {
				if(bUseICD10) {		
					str = diagCode.number;
					str.Replace(".","");
					str.TrimRight();

					//HI0#-2	1270	Code List Qualifier Code				M	ID	1/3

					//BF - Principal Diagnosis (ICD-9)
					//ABF - Principal Diagnosis (ICD-10)

					//for now we use BF
					if(!str.IsEmpty()) {
						OutputString += ParseANSIField("ABF",1,3);
						//HI0#-2	1271	Industry Code							M	AN	1/30
						OutputString += ":";
						OutputString += str;
					}
				}
				else {
					str = diagCode.number;
					str.Replace(".","");
					str.TrimRight();

					//HI0#-2	1270	Code List Qualifier Code				M	ID	1/3

					//BF - Principal Diagnosis (ICD-9)
					//ABF - Principal Diagnosis (ICD-10)

					//for now we use BF
					if(!str.IsEmpty()) {
						OutputString += ParseANSIField("BF",1,3);
						//HI0#-2	1271	Industry Code							M	AN	1/30
						OutputString += ":";
						OutputString += str;
					}
				}
			}
		}

		if(bOutput)
			EndANSISegment(OutputString);


///////////////////////////////////////////////////////////////////////////////

//239		231		HI		Principal Procedure Information			S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//Only required on inpatient claims.

		OutputString = "HI";

		bOutput = TRUE;

		//HI01	C022		Health Care Code Info.					M 1

		//HI01-1	1270	Code List Qualifier Code				M	ID	1/3

		//In the UB04, this is Box 74.

		//static "CAH" (principal procedure code) or "BR" (ICD-9)

		// (j.jones 2012-09-05 13:57) - PLID 52191 - added Box74Qual to override this qualifier,
		// 0 means use the default qualifier (CAH for CPT codes, BR for ICD codes)
		// 1 means always use CAH
		// 2 means always use BR (or BRR, if it's an ICD-10 diagnosis code)
		CString strBox74Qual = "";
		if(m_UB92Info.Box74Qual == 1) {
			//force CAH
			strBox74Qual = "CAH";
		}
		else if(m_UB92Info.Box74Qual == 2) {
			//force BR
			strBox74Qual = "BR";
		}
		//if Box74Qual is 0, then we use the default value, and let
		//the Box80 setting fill this in with the relevant qualifier
		//(Box80 is the old UB92 field for the same thing as Box 74)

		// (j.jones 2009-04-24 11:12) - PLID 34070 - we need to handle the case where this is supposed to be blank
		if(m_UB92Info.Box80Number == 0) {
			//CPT Code
			//CAH - Advanced Billing Concepts (ABC) Codes
			if(strBox74Qual.IsEmpty()) {
				strBox74Qual = "CAH";
			}
		}
		else if(m_UB92Info.Box80Number == 1) {

			//BR - ICD-9
			//BBR - ICD-10

			//ICD-9 Code
			if(strBox74Qual.IsEmpty()) {
				strBox74Qual = "BR";
			}
		}
		else {
			strBox74Qual = "";
			bOutput = FALSE;
		}
		OutputString += ParseANSIField(strBox74Qual,1,3);

		//HI01-2	1271	Industry Code							M	AN	1/30

		//ChargesT.ItemCode OR DiagCodes.CodeNumber
		//DRT 4/10/2006 - PLID 11734 - Removed ProcCode
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		// (j.jones 2009-04-24 11:19) - PLID 34070 - this recordset won't be needed right now if this field is set to be blank,
		// but it is needed for the "Other Procedure Information" below
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		// (d.singleton 2014-03-17 15:31) - PLID 61195 - update ANSI5010 claim export to support ICD10 and new diagcode billing structure
		// (a.walling 2014-03-19 9:13) - PLID 61419 - EBilling - ANSI5010 - Get diag info from bill
		_RecordsetPtr rsCPTCode = CreateParamRecordset("SELECT ChargesT.ItemCode, LineItemT.Date FROM ChargesT "
						"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
						"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
						"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
						"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
						"WHERE ChargesT.BillID = {INT} AND CPTCodeT.ID IS NOT NULL AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
						"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
						"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
						"ORDER BY LineID",
						m_pEBillingInfo->BillID);
		if(!rsCPTCode->eof) {
			if(m_UB92Info.Box80Number == 0) {
				//CPT Code
				str = AdoFldString(rsCPTCode, "ItemCode", "");
			}
			// (j.jones 2009-04-24 11:12) - PLID 34070 - we need to handle the case where this is supposed to be blank
			else if(m_UB92Info.Box80Number == 1) {
				//ICD-9 Code
				// (a.walling 2014-03-19 9:13) - PLID 61419 - EBilling - ANSI5010 - Get diag info from bill
				str = m_pEBillingInfo->GetSafeBillDiag(0).number;
				str.Replace(".","");
			}
			// (j.jones 2012-04-04 13:33) - PLID 49424 - removed errant code that tried to fill this with var,
			// which was the DiagCode4 value
			else {
				str = "";
			}
		}
		else
			str = "";

		if(str == "")
			bOutput = FALSE;

		OutputString += ":";
		OutputString += str;


		//this is required for ICD-9 codes (BR) and optional for CPT Codes,
		//but ClaimMD requires it for CPT Codes so we should just always send it

		//HI01-3	1250	Date Time Period Format Qualifier		X	ID	2/3

		//LineItemT.Date
		if(!rsCPTCode->eof) {
			var = rsCPTCode->Fields->Item["Date"]->Value;
			if(var.vt == VT_DATE) {
			 COleDateTime dt = var.date;
				 str = dt.Format("%Y%m%d");
			}
			else
				str = "";
		}
		else
			str = "";

		//static "D8"
		if(str != "")
			OutputString += ":D8";

		//HI01-4	1251	Date Time Period						X	AN	1/35

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}
		
		//HI01-5	NOT USED
		//HI01-6	NOT USED
		//HI01-7	NOT USED
		//HI01-8	NOT USED
		//HI01-9	NOT USED

		//HI02	NOT USED
		//HI03	NOT USED
		//HI04	NOT USED
		//HI05	NOT USED
		//HI06	NOT USED
		//HI07	NOT USED
		//HI08	NOT USED
		//HI09	NOT USED
		//HI10	NOT USED
		//HI11	NOT USED
		//HI12	NOT USED

		if(bOutput)
			EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//242		231		HI		Other Procedure Information				S		2

		//BQ - ICD-9
		//BBQ - ICD-10

		//only supports ICD-9s, we don't support them here currently

///////////////////////////////////////////////////////////////////////////////
//258		231		HI		Occurrence Span Information				S		2
		
		//Box 35, 36, dates from and to
		// b.spivey, March 8, 2016 -- PLID 68473 -- Occurence date spans and codes
		if (ub04ClaimInfo.occurrenceSpans.size() > 0) {
			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "HI";
			
			if (ub04ClaimInfo.occurrenceSpans.size() > 12) {
				ub04ClaimInfo.occurrenceSpans.resize(12); 
			}

			for (auto val : ub04ClaimInfo.occurrenceSpans) {
				if (!(val.code.GetLength() > 0 && val.from.GetStatus() == COleDateTime::valid && val.to.GetStatus() == COleDateTime::valid)) {
					continue;
				}

				//HI01-1	1270	Code List Qualifier Code				M	ID	1/3
				//static "BI"
				//HI01-2	1271	Industry Code							M	AN	1/30
				//Occurrence Span Code
				//HI01-3	1250	Date Time Period Format Qualifier		X	ID	2/3
				//static "RD8"
				//HI01-4	1251	Date Time Period						X	AN	1/35
				//Occurrence Span
				str.Format("*BI:%s:RD8:%s-%s", val.code.Left(30), val.from.Format("%Y%m%d"), val.to.Format("%Y%m%d"));

				//HI01-5	782		Monetary Amount							O	R	1/18
				//HI01-6	380		Quantity								O	R	1/15
				//HI01-7	799		Version Identifier						O	AN	1/30
				//HI01-8	1271	Industry Code							X	AN	1/30
				//HI01-9	1073	Yes/No Condition or Response Code		X	ID	1/1
				//NOT USED

				OutputString += str;
			}

			EndANSISegment(OutputString);

		}

//271		231		HI		Occurrence Information					S		2
		//Boxes 31 - 34
		//Ref.	Data		Name									Attributes
		//Des.	Element
		// b.spivey, March 7, 2016 -- PLID 68473 -- Occurence dates and codes, no spans. 
		if (ub04ClaimInfo.occurrences.size() > 0) {

			OutputString = "HI";

			if (ub04ClaimInfo.occurrences.size() > 12) {
				ub04ClaimInfo.occurrences.resize(12);
			}

			for (auto val : ub04ClaimInfo.occurrences) {
				if (!(val.code.GetLength() > 0 && val.date.GetStatus() == COleDateTime::valid)) {
					continue;
				}

				//HI01-1	1270	Code List Qualifier Code				M	ID	1/3
				//static "BH"
				//HI01-2	1271	Industry Code							M	AN	1/30
				//Occurrence Code
				//HI01-3	1250	Date Time Period Format Qualifier		X	ID	2/3
				//static "D8"
				//HI01-4	1251	Date Time Period						X	AN	1/35
				//Occurrence Code Date				
				str.Format("*BH:%s:D8:%s", val.code.Left(30), val.date.Format("%Y%m%d"));

				//HI01-5	782		Monetary Amount							O	R	1/18
				//HI01-6	380		Quantity								O	R	1/15
				//HI01-7	799		Version Identifier						O	AN	1/30
				//HI01-8	1271	Industry Code							X	AN	1/30
				//HI01-9	1073	Yes/No Condition or Response Code		X	ID	1/1
				//NOT USED

				OutputString += str;
			}

			EndANSISegment(OutputString);
		}

//284		231		HI		Value Information						S		2
		// b.spivey, March 7, 2016 -- PLID 68329 -- export value codes and monetary amounts 
		if (ub04ClaimInfo.values.size() > 0) {

			OutputString = "HI";

			if (ub04ClaimInfo.values.size() > 12) {
				ub04ClaimInfo.values.resize(12);
			}

			for (auto val : ub04ClaimInfo.values) {
				if (!(val.code.GetLength() > 0 && val.amount.GetStatus() == COleCurrency::valid)) {
					continue;
				}

				str = "";

				// (j.jones 2016-05-10 12:32) - NX-100589 - this segment was not exported properly,
				// the value code amount was sent in HI0#-4, it is supposed to be -5.
				// Added UB specs inline so the difference is clearer.

				//HI01-1	1270	Code List Qualifier Code				M	ID	1/3
				//static "BE"
				str += "*";
				str += "BE";

				//HI01-2	1271	Industry Code							M	AN	1/30

				//Value Code
				str += ":";
				str += val.code.Left(30);
				
				//HI01-3	1250	Date Time Period Format Qualifier		X	ID	2/3
				//HI01-4	1251	Date Time Period						X	AN	1/35
				//NOT USED
				str += "::";

				//HI01-5	782		Monetary Amount							O	R	1/18

				//Value Code Amount
				str += ":";
				str += FormatString("%lli", val.amount.m_cur.int64 / 100);

				//HI01-6	380		Quantity								O	R	1/15
				//HI01-7	799		Version Identifier						O	AN	1/30
				//HI01-8	1271	Industry Code							X	AN	1/30
				//HI01-9	1073	Yes/No Condition or Response Code		X	ID	1/1
				//NOT USED

				OutputString += str;
			}

			EndANSISegment(OutputString);

		}


//294		231		HI		Condition Information					S		2
		// b.spivey, March 3rd, 2016 -- PLID 68465 -- if we have condition codes we should export them. 
		//	 This vector should never be more than the maximum allowed to be exported. 
		if (ub04ClaimInfo.conditions.size() > 0) {

			OutputString = "HI";

			if (ub04ClaimInfo.conditions.size() > 12) {
				ub04ClaimInfo.conditions.resize(12);
			}

			for (auto val : ub04ClaimInfo.conditions) {
				if (val.GetLength() <= 0) {
					continue;
				}

				//HI01-1	1270	Code List Qualifier Code				M	ID	1/3
				//static "BG"

				//HI01-2	1271	Industry Code							M	AN	1/30
				//Condition Code
				str.Format("*BG:%s", val.Left(30));

				//HI01-3	1250	Date Time Period Format Qualifier		X	ID	2/3
				//HI01-4	1251	Date Time Period						X	AN	1/35
				//HI01-5	782		Monetary Amount							O	R	1/18
				//HI01-6	380		Quantity								O	R	1/15
				//HI01-7	799		Version Identifier						O	AN	1/30
				//HI01-8	1271	Industry Code							X	AN	1/30
				//HI01-9	1073	Yes/No Condition or Response Code		X	ID	1/1
				//NOT USED

				OutputString += str; 
			}

			EndANSISegment(OutputString);
		}

//304		231		HI		Treatment Code Information				S		2
		//not used

//313		241		HCP		Claim Pricing/Repricing Information		S		1
		//not used that we know of

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2300_Inst");

	return Error_Other;
}

/*
int CEbilling::ANSI_5010_2305() {

	//Home Health Care Plan Information

	//This loop is not used in our software.

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2305 - HOME HEALTH CARE PLAN INFORMATION				6

//276		242		CR7		Home Health Care Plan Information		S		1
//278		243		HSD		Health Care Services Delivery			S		3

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2305");

	return Error_Other;
}
*/

// (j.jones 2011-03-07 14:17) - PLID 42260 - added ability to force this function to load
// the 2310B IDs of the rendering provider
int CEbilling::ANSI_5010_2310A_Prof(BOOL bUseRenderingProvider) {

	//Referring Provider Name

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2310A\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;

		// (j.jones 2011-03-07 14:19) - PLID 42660 - if bUseRenderingProvider is true,
		// export the rendering provider's info instead of the referring physician
		if(bUseRenderingProvider) {

			// (j.jones 2011-11-07 14:44) - PLID 46299 - added support for ProvidersT.UseCompanyOnClaims
			rs = CreateParamRecordset("SELECT PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, PersonT.SocialSecurity, PersonT.Company, "
				"ProvidersT.* "
				"FROM PersonT LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
				"WHERE PersonT.ID = {INT}", m_pEBillingInfo->ANSI_RenderingProviderID);
			if(rs->eof) {
				//if the recordset is empty, there is no rendering provider. So halt everything!!!
				rs->Close();
				// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
				CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
				if(!strPatientName.IsEmpty()) {
					// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
					str.Format("Could not open rendering provider information for '%s', Bill ID %li. (ANSI_2310A_Prof)", strPatientName, m_pEBillingInfo->BillID);
				}
				else
					//serious problems if you get here
					str = "Could not open rendering provider information. (ANSI_2310A_Prof)";

				AfxMessageBox(str);
				return Error_Missing_Info;
			}

		}
		else {

			//normal case, send the referring physician

			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			rs = CreateParamRecordset("SELECT PersonT.ID, PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, "
				"PersonT.SocialSecurity, ReferringPhysT.* "
				"FROM BillsT LEFT JOIN PersonT ON BillsT.RefPhyID = PersonT.ID "
				"INNER JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
				"WHERE BillsT.ID = {INT}", m_pEBillingInfo->BillID);

			if(rs->eof) {
				//if the recordset is empty, there is no referring provider. So halt everything!!!
				rs->Close();
				// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
				CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
				if(!strPatientName.IsEmpty()) {
					// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
					str.Format("Could not open referring provider information for '%s', Bill ID %li. (ANSI_2310A_Prof)", strPatientName, m_pEBillingInfo->BillID);
				}
				else
					//serious problems if you get here
					str = "Could not open referring provider information. (ANSI_2310A_Prof)";

				AfxMessageBox(str);
				return Error_Missing_Info;
			}
		}


//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310A - REFERRING PROVIDER NAME						2

//257		250		NM1		Referring Provider Name					S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M	ID	2/3

		//static value "DN" (referring physician)

		OutputString += ParseANSIField("DN", 2, 3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//static value "1" for person, "2" for non-person entity
		str = "1";

		// (j.jones 2011-11-07 14:46) - PLID 46299 - check the UseCompanyOnClaims setting to send the provider as an organization
		CString strProviderCompany = "";
		BOOL bUseProviderCompanyOnClaims = FALSE;

		if(bUseRenderingProvider) {
			bUseProviderCompanyOnClaims = VarBool(rs->Fields->Item["UseCompanyOnClaims"]->Value, FALSE);
			strProviderCompany = VarString(rs->Fields->Item["Company"]->Value, "");
			strProviderCompany.TrimLeft();
			strProviderCompany.TrimRight();
			if(strProviderCompany.IsEmpty()) {
				//can't use the company if it is blank
				bUseProviderCompanyOnClaims = FALSE;
			}

			if(bUseProviderCompanyOnClaims) {
				str = "2";
			}
			else {
				str = "1";
			}
		}		
		OutputString += ParseANSIField(str, 1, 1);

		//NM103	1035		Name Last / Org Name					O 1	AN	1/60

		if(bUseProviderCompanyOnClaims) {
			str = strProviderCompany;
		}
		else {
			str = GetFieldFromRecordset(rs, "Last");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 60);

		//NM104	1036		Name First								O 1	AN	1/35

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "First");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 35);

		//NM105	1037		Name Middle								O 1	AN	1/25

		// (j.jones 2013-04-11 09:40) - PLID 56166 - if ANSI_HideRefPhyFields is enabled,
		// do not send the middle name in this loop, even if we happen to be sending the
		// rendering provider
		if(m_HCFAInfo.ANSI_HideRefPhyFields == 1) {
			str = "";
		}
		else {
			if(bUseProviderCompanyOnClaims) {
				str = "";
			}
			else {
				str = GetFieldFromRecordset(rs, "Middle");
			}
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 25);

		//NM106	NOT USED

		OutputString += "*";

		//NM107	1039		Name Suffix								O 1	AN	1/10

		// (j.jones 2013-04-11 09:40) - PLID 56166 - if ANSI_HideRefPhyFields is enabled,
		// do not send the suffix in this loop, even if we happen to be sending the
		// rendering provider
		if(m_HCFAInfo.ANSI_HideRefPhyFields == 1) {
			str = "";
		}
		else {
			if(bUseProviderCompanyOnClaims) {
				str = "";
			}
			else {
				// (j.jones 2007-08-03 11:44) - PLID 26934 - changed from Suffix to Title
				str = GetFieldFromRecordset(rs, "Title");
			}
		}

		OutputString += ParseANSIField(str, 1, 10);

		//NM108	66			ID Code Qualifier						X 1	ID	1/2

		//XX - NPI only

		CString strIdent, strID;

		// (j.jones 2009-01-06 10:00) - PLID 32614 - supported hiding Box 17b NPI, which also applies to ANSI
		if(m_HCFAInfo.HideBox17b == 0) {
			//"XX" for NPI
			strIdent = "XX";
			strID = "";
			// (j.jones 2013-06-10 16:53) - PLID 56255 - if they want to use the Ref. Phys. Group NPI,
			// try to load it, but revert to the regular NPI if the group is empty
			if(!bUseRenderingProvider && m_HCFAInfo.UseRefPhyGroupNPI == 1) {
				strID = GetFieldFromRecordset(rs, "GroupNPI");
				strID.TrimLeft(); strID.TrimRight();
			}
			if(strID.IsEmpty()) {
				strID = GetFieldFromRecordset(rs, "NPI");
			}
		}

		// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens,
		// although NPI should always be numeric
		strID = StripSpacesAndHyphens(strID);

		//do not output if the ID is blank
		if(strID != "")
			OutputString += ParseANSIField(strIdent, 1, 2);
		else
			OutputString += "*";

		//NM109	67			ID Code									X	AN	2/80

		//ReferringPhysT.FedEmployerID		
		if(strID != "")
			OutputString += ParseANSIField(strID, 2, 80);
		else
			OutputString += "*";

		//NM110	NOT USED

		OutputString += "*";

		//NM111	NOT USED

		OutputString += "*";

		//NM112	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

//260		271		REF		Referring Provider Secondary Info.		S		3

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//In 5010, only 0B, 1G, and G2 are valid. For now, we still let them
		//set up whatever they want.

		OutputString = "REF";

		// (j.jones 2008-05-02 10:09) - PLID 27478 - we need to silently skip
		// adding additional REF segments that have the same qualifier, so
		// track them in an array
		CStringArray arystrQualifiers;

		//REF01	128			Reference Identification Qualifier		M 1	ID	2/3

		// (j.jones 2011-03-07 14:19) - PLID 42660 - if bUseRenderingProvider is true,
		// export the rendering provider's info instead of the referring physician
		if(bUseRenderingProvider) {
			// (j.jones 2010-04-14 10:20) - PLID 38194 - the ID is now calculated in a shared function
			CString strLoadedFrom = "";
			EBilling_Calculate2310B_REF(strIdent,strID,strLoadedFrom, m_pEBillingInfo->ANSI_RenderingProviderID,m_pEBillingInfo->HCFASetupID,
				m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->InsuredPartyID, m_pEBillingInfo->BillLocation);

			// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strIdent.CompareNoCase("XX") == 0) {
				strIdent = "";
				strID = "";
				//log that this happened
				Log("Ebilling file tried to export 2310A REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->BillID);
			}
		}
		else {
			//normal case, send the referring physician info

			//Box17aANSI will give us the qualifier AND ID, even if the ID is blank

			var = rs->Fields->Item["ID"]->Value;
			strIdent = "", strID = "";
			// (j.jones 2010-10-20 15:42) - PLID 40936 - moved Box17aANSI to GlobalFinancialUtils and renamed it
			EBilling_Box17aANSI(m_HCFAInfo.Box17a, m_HCFAInfo.Box17aQual, strIdent,strID,var.lVal);

			// (j.jones 2010-10-20 15:31) - PLID 40936 - if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strIdent.CompareNoCase("XX") == 0) {
				strIdent = "";
				strID = "";
				//log that this happened
				Log("Ebilling file tried to export 2310A REF*XX for referring physician ID %li, bill ID %li, but REF*XX is disabled.", VarLong(var), m_pEBillingInfo->BillID);
			}
		}

		// (j.jones 2006-11-28 16:01) - PLID 23651 - removed ANSIRefPhyQual as an override,
		// since HCFA Groups now have Box 17a qualifier fields
		
		OutputString += ParseANSIField(strIdent,2,3);

		//REF02	127			Reference Identification				X 1	AN	1/50

		OutputString += ParseANSIField(strID,1,50);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		// (j.jones 2007-09-20 10:51) - PLID 27455 - do not send this segment if the
		// qualifier or ID are blank
		if(strIdent != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdent)) {
			EndANSISegment(OutputString);

			// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
			arystrQualifiers.Add(strIdent);
		}

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2310A_Prof");

	return Error_Other;
}

// (j.jones 2010-10-18 16:17) - PLID 40346 - initial implementation of ANSI 5010 Institutional claim
int CEbilling::ANSI_5010_2310A_Inst() {

	//Attending Physician Name

	//This loop is required on all inpatient claims

	//This loop represents the provider listed in Box 82 on the UB92

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2310A\r\n";
		m_OutputFile.Write(str,str.GetLength());

	}

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310A - ATTENDING PHYSICIAN NAME						1

//319		250		NM1		Attending Physician Name				S		1

		//
		//The attending Physician is chosen 1 of 2 ways.
		//1)  Provider drop down from General 1 Tab		- when m_UB92Info.Box82Setup = 2
		//2)  The provider of the first charge on a bill	- when m_UB92Info.Box82Setup = 1
		//		The first charge is the one for the current bill where LineID = 1
		//

		// (j.jones 2007-02-12 15:20) - PLID 22110 - LoadClaimInfo ensures that the ProviderID is correct,
		// either the G1 provider or the charge provider, and also uses ClaimProviderID
		_RecordsetPtr rs;
		// (j.jones 2007-05-10 15:05) - PLID 25948 - supported referring physician
		if(m_pEBillingInfo->Box82Setup == 3) {
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			rs = CreateParamRecordset("SELECT PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, PersonT.SocialSecurity, ReferringPhysT.* "
				"FROM PersonT LEFT JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
				"WHERE PersonT.ID = {INT}", m_pEBillingInfo->ANSI_RenderingProviderID);
		}
		else {
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			// (j.jones 2011-11-07 14:44) - PLID 46299 - added support for ProvidersT.UseCompanyOnClaims
			rs = CreateParamRecordset("SELECT PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, PersonT.SocialSecurity, PersonT.Company, "
				"ProvidersT.* "
				"FROM PersonT LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
				"WHERE PersonT.ID = {INT}", m_pEBillingInfo->ANSI_RenderingProviderID);
		}
		if(rs->eof) {
			//if the recordset is empty, there is no attending provider. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open attending provider information for '%s', Bill ID %li. (ANSI_2310A_Inst)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open attending provider information. (ANSI_2310A_Inst)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M 1	ID	2/3

		//static value "71"

		OutputString += ParseANSIField("71", 2, 3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//static value "1" for person, "2" for non-person entity
		str = "1";

		// (j.jones 2011-11-07 14:46) - PLID 46299 - check the UseCompanyOnClaims setting to send the provider as an organization
		CString strProviderCompany = "";
		BOOL bUseProviderCompanyOnClaims = FALSE;

		if(m_pEBillingInfo->Box82Setup != 3) {
			bUseProviderCompanyOnClaims = VarBool(rs->Fields->Item["UseCompanyOnClaims"]->Value, FALSE);
			strProviderCompany = VarString(rs->Fields->Item["Company"]->Value, "");
			strProviderCompany.TrimLeft();
			strProviderCompany.TrimRight();
			if(strProviderCompany.IsEmpty()) {
				//can't use the company if it is blank
				bUseProviderCompanyOnClaims = FALSE;
			}

			if(bUseProviderCompanyOnClaims) {
				str = "2";
			}
			else {
				str = "1";
			}
		}		
		OutputString += ParseANSIField(str, 1, 1);

		//NM103	1035		Name Last / Org Name					O 1	AN	1/60

		if(bUseProviderCompanyOnClaims) {
			str = strProviderCompany;
		}
		else {
			str = GetFieldFromRecordset(rs, "Last");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 60);

		//NM104	1036		Name First								O 1	AN	1/35

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "First");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 35);

		//NM105	1037		Name Middle								O 1	AN	1/25

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "Middle");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 25);

		//NM106	NOT USED

		OutputString += "*";

		//NM107	1039		Name Suffix								O 1	AN	1/10

		// (j.jones 2007-08-03 11:44) - PLID 26934 - changed from Suffix to Title
		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "Title");
		}

		OutputString += ParseANSIField(str, 1, 10);

		//NM108	66			ID Code Qualifier						X 1	ID	1/2

		//in 5010 this is always the NPI
		str = GetFieldFromRecordset(rs, "NPI");
		// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens,
		// although NPI should always be numeric
		str = StripSpacesAndHyphens(str);

		if(str != "")
			OutputString += ParseANSIField("XX", 1, 2);
		else
			OutputString += "*";

		//NM109	67			ID Code									X	AN	2/80

		if(str != "")
			OutputString += ParseANSIField(str, 2, 80);
		else
			OutputString += "*";

		//NM110	NOT USED
		OutputString += "*";

		//NM111	NOT USED
		OutputString += "*";

		//NM112	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

//322		255		PRV		Attending Physician Specialty Info.		S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "PRV";

		//PRV01	1221		Provider Code							M 1	ID	1/3

		//static value "AT"

		OutputString += ParseANSIField("AT", 1, 3);

		CString strTaxonomy = "";

		// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore
		if(m_pEBillingInfo->Box82Setup != 3) {
			strTaxonomy = AdoFldString(rs, "TaxonomyCode","");
			strTaxonomy.TrimLeft();
			strTaxonomy.TrimRight();

			// (j.jones 2013-09-05 11:48) - PLID 58252 - the taxonomy code override will
			// be used only if one exists, but never for referring physicians
			_RecordsetPtr rsTaxonomyOver= CreateParamRecordset("SELECT ANSI_2310A_Taxonomy AS TaxonomyCode "
				"FROM UB92EbillingSetupT "
				"WHERE ANSI_2310A_Taxonomy <> '' AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
				m_pEBillingInfo->UB92SetupID, m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->BillLocation);
			if(!rsTaxonomyOver->eof) {
				CString strTaxonomyOverride = AdoFldString(rsTaxonomyOver, "TaxonomyCode","");
				strTaxonomyOverride.TrimLeft();
				strTaxonomyOverride.TrimRight();
				//we don't permit overriding with nothing, so only use this
				//if there really is an override value
				if(!strTaxonomyOverride.IsEmpty()) {
					strTaxonomy = strTaxonomyOverride;
				}
			}
		}

		// (j.jones 2008-05-01 10:47) - PLID 28691 - don't output if the taxonomy code is blank
		if(!strTaxonomy.IsEmpty()) {

			//PRV02	128			Reference Identification Qualifier		M 1	ID	2/3

			//static value "PXC"
			OutputString += ParseANSIField("PXC",2,3);

			//PRV03	127			Reference Identification				M 1	AN	1/50

			//ProvidersT.TaxonomyCode
			//This comes from a published HIPAA list.
			//"2086S0122X" is for Plastic and Reconstructive Surgery
			OutputString += ParseANSIField(strTaxonomy,1,50);

			//PRV04	NOT USED
			OutputString += "*";
			//PRV05	NOT USED
			OutputString += "*";
			//PRV06	NOT USED
			OutputString += "*";

			EndANSISegment(OutputString);
		}

//324		271		REF		Attending Physician Secondary Info.		S		4

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		// (j.jones 2008-05-02 10:09) - PLID 27478 - we need to silently skip
		// adding additional REF segments that have the same qualifier, so
		// track them in an array
		CStringArray arystrQualifiers;

		//REF01	128			Reference Identification Qualifier		M 1	ID	2/3

		//technically only 0B, 1G, G2, and LU are valid, but we allow anything
		
		// (j.jones 2010-04-14 10:43) - PLID 38194 - the ID is now calculated in a shared function
		CString strIdentifier = "", strID = "", strLoadedFrom = "";
		EBilling_Calculate2310A_REF(strIdentifier, strID, strLoadedFrom, m_pEBillingInfo->ANSI_RenderingProviderID,
			m_pEBillingInfo->UB92SetupID, m_UB92Info.UB04Box76Qual, m_UB92Info.Box82Num, m_UB92Info.Box82Setup,
			m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->InsuredPartyID, m_pEBillingInfo->BillLocation);

		// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
		if(m_bDisableREFXX && strIdentifier.CompareNoCase("XX") == 0) {
			strIdentifier = "";
			strID = "";
			//log that this happened
			Log("Ebilling file tried to export 2310A REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->BillID);
		}
		
		OutputString += ParseANSIField(strIdentifier,2,3);

		//REF02	127			Reference Identification				X 1	AN	1/50

		OutputString += ParseANSIField(strID,1,50);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		// (j.jones 2007-09-20 10:51) - PLID 27455 - do not send this segment if the
		// qualifier or ID are blank
		if(strIdentifier != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdentifier)) {
			EndANSISegment(OutputString);

			// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
			arystrQualifiers.Add(strIdentifier);
		}

		rs->Close();

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2310A_Inst");

	return Error_Other;
}

int CEbilling::ANSI_5010_2310B_Prof() {

	//Rendering Provider Name

	//This loop is not used when 2000A's PRV segment is used.
	//This loop will not be used when the Rendering Provider is the same entity
	//as the Billing Provider and/or the Pay-To Provider.

	//This loop will be used when the Billing or Pay-To Provider is a group.
	//This loop will then represent the individual Rendering Provider.

	//This loop will be used if m_bIsGroup is TRUE.

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2310B\r\n";
		m_OutputFile.Write(str,str.GetLength());

	}

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310B - RENDERING PROVIDER NAME						1

//262		250		NM1		Rendering Provider Name					S		1

		//
		//The rendering provider is chosen 1 of 2 ways.
		//1)  Provider drop down from General 1 Tab		- when m_pEBillingInfo->Box33Setup = 2
		//2)  The provider of the first charge on a bill	- when m_pEBillingInfo->Box33Setup = 1
		//		The first charge is the one for the current bill where LineID = 1
		//

		// (j.jones 2007-02-12 15:20) - PLID 22110 - LoadClaimInfo ensures that the ProviderID is correct,
		// either the G1 provider or the charge provider, and also uses ClaimProviderID
		// (j.jones 2008-04-03 09:55) - PLID 28995 - this function now uses the ANSI_RenderingProviderID value
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		// (j.jones 2011-11-07 14:44) - PLID 46299 - added support for ProvidersT.UseCompanyOnClaims
		_RecordsetPtr rs = CreateParamRecordset("SELECT PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, PersonT.SocialSecurity, PersonT.Company, "
			"ProvidersT.* "
			"FROM PersonT LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
			"WHERE PersonT.ID = {INT}", m_pEBillingInfo->ANSI_RenderingProviderID);
		if(rs->eof) {
			//if the recordset is empty, there is no rendering provider. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open rendering provider information for '%s', Bill ID %li. (ANSI_2310B_Prof)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open rendering provider information. (ANSI_2310B_Prof)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M 1	ID	2/3

		//static value "82"

		OutputString += ParseANSIField("82", 2, 3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//static value "1" for person, "2" for non-person entity
		str = "1";

		// (j.jones 2011-11-07 14:46) - PLID 46299 - check the UseCompanyOnClaims setting to send the provider as an organization
		CString strProviderCompany = VarString(rs->Fields->Item["Company"]->Value, "");
		BOOL bUseProviderCompanyOnClaims = VarBool(rs->Fields->Item["UseCompanyOnClaims"]->Value, FALSE);

		if(bUseProviderCompanyOnClaims) {
			strProviderCompany.TrimLeft();
			strProviderCompany.TrimRight();
			if(strProviderCompany.IsEmpty()) {
				//can't use the company if it is blank
				bUseProviderCompanyOnClaims = FALSE;
			}

			if(bUseProviderCompanyOnClaims) {
				str = "2";
			}
			else {
				str = "1";
			}
		}
		OutputString += ParseANSIField(str, 1, 1);

		//NM103	1035		Name Last / Org Name					O 1	AN	1/60

		if(bUseProviderCompanyOnClaims) {
			str = strProviderCompany;
		}
		else {
			str = GetFieldFromRecordset(rs, "Last");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 60);

		//NM104	1036		Name First								O 1	AN	1/35

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "First");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 35);

		//NM105	1037		Name Middle								O 1	AN	1/25

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "Middle");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 25);

		//NM106	NOT USED

		OutputString += "*";

		//NM107	1039		Name Suffix								O 1	AN	1/10

		// (j.jones 2007-08-03 11:44) - PLID 26934 - changed from Suffix to Title
		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "Title");
		}

		OutputString += ParseANSIField(str, 1, 10);

		//NM108	66			ID Code Qualifier						X 1	ID	1/2

		//Always XX - NPI

		str = "";

		_RecordsetPtr rsOver = CreateParamRecordset("SELECT Box24JNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
		if(!rsOver->eof) {
			var = rsOver->Fields->Item["Box24JNPI"]->Value;
			if(var.vt == VT_BSTR) {
				CString strOver = VarString(var,"");
				strOver.TrimLeft();
				strOver.TrimRight();
				if(!strOver.IsEmpty())
					str = strOver;
			}
		}
		rsOver->Close();

		if(str.IsEmpty()) {
			str = GetFieldFromRecordset(rs, "NPI");
		}

		// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens,
		// although NPI should always be numeric
		str = StripSpacesAndHyphens(str);

		if(str != "")
			OutputString += ParseANSIField("XX", 1, 2);
		else
			OutputString += "*";

		//NM109	67			ID Code									X 1	AN	2/80

		if(str != "")
			OutputString += ParseANSIField(str, 2, 80);
		else
			OutputString += "*";

		//NM110	NOT USED

		OutputString += "*";

		//NM111	NOT USED

		OutputString += "*";

		//NM112	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

//265		255		PRV		Rendering Provider Specialty Info.		R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2005-09-29 10:15) - According to THIN, if we are sending 2310B for a non-group,
		// then the PRV taxonomy code segment should not be sent, because we already sent it in 2000A,
		// so we will not send UNLESS it's a group or is forced by the ANSI Properties to do so,
		// which is consistent with the 2000A code which does not send the PRV segment if it is a group

		OutputString = "PRV";

		//PRV01	1221		Provider Code							M 1	ID	1/3

		//static value "PE"
		OutputString += ParseANSIField("PE", 1, 3);

		CString strTaxonomy = "";
		
		// (j.jones 2005-09-29 10:16) - only send if a group or forced by the ANSI Properties
		if(m_bIsGroup || m_HCFAInfo.Use2310BPRVSegment) {				
			strTaxonomy = AdoFldString(rs, "TaxonomyCode","");
			strTaxonomy.TrimLeft();
			strTaxonomy.TrimRight();
		}

		// (j.jones 2013-09-05 11:48) - PLID 58252 - the taxonomy code override will
		// be used only if one exists, and will be used even if the settings say not
		// to send this segment
		_RecordsetPtr rsTaxonomyOver = CreateParamRecordset("SELECT ANSI_2310B_Taxonomy AS TaxonomyCode "
			"FROM EbillingSetupT "
			"WHERE ANSI_2310B_Taxonomy <> '' AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
			m_pEBillingInfo->HCFASetupID, m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->BillLocation);
		if(!rsTaxonomyOver->eof) {
			CString strTaxonomyOverride = AdoFldString(rsTaxonomyOver, "TaxonomyCode","");
			strTaxonomyOverride.TrimLeft();
			strTaxonomyOverride.TrimRight();
			//we don't permit overriding with nothing, so only use this
			//if there really is an override value
			if(!strTaxonomyOverride.IsEmpty()) {
				strTaxonomy = strTaxonomyOverride;
			}
		}

		// (j.jones 2008-05-01 10:47) - PLID 28691 - don't output if the taxonomy code is blank
		if(!strTaxonomy.IsEmpty()) {

			//PRV02	128			Reference Identification Qualifier		M 1	ID	2/3

			//static value "PXC"
			OutputString += ParseANSIField("PXC",2,3);

			//PRV03	127			Reference Identification				M 1	AN	1/50

			//ProvidersT.TaxonomyCode
			//This comes from a published HIPAA list.
			//"2086S0122X" is for Plastic and Reconstructive Surgery
			OutputString += ParseANSIField(strTaxonomy,1,50);

			//PRV04	NOT USED

			OutputString += "*";

			//PRV05	NOT USED

			OutputString += "*";

			//PRV06	NOT USED

			OutputString += "*";

			EndANSISegment(OutputString);
		}

//267		271		REF		Rendering Provider Secondary Info.		S		4

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//In 5010, the only valid qualifiers are 0B, 1G, G2, LU,
		//but we continue to allow them to set up whatever they want.

		OutputString = "REF";

		// (j.jones 2008-05-02 10:09) - PLID 27478 - we need to silently skip
		// adding additional REF segments that have the same qualifier, so
		// track them in an array
		CStringArray arystrQualifiers;
		
		// (j.jones 2010-04-14 10:20) - PLID 38194 - the ID is now calculated in a shared function
		CString strIdentifier = "", strID = "", strLoadedFrom = "";
		EBilling_Calculate2310B_REF(strIdentifier,strID,strLoadedFrom, m_pEBillingInfo->ANSI_RenderingProviderID,m_pEBillingInfo->HCFASetupID,
			m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->InsuredPartyID, m_pEBillingInfo->BillLocation);

		// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
		if(m_bDisableREFXX && strIdentifier.CompareNoCase("XX") == 0) {
			strIdentifier = "";
			strID = "";
			//log that this happened
			Log("Ebilling file tried to export 2310B REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->BillID);
		}

		//REF01	128			Reference Identification Qualifier		M 1	ID	2/3
		
		OutputString += ParseANSIField(strIdentifier,2,3);

		//REF02	127			Reference Identification				X 1	AN	1/50

		OutputString += ParseANSIField(strID,1,50);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		// (j.jones 2007-09-20 10:51) - PLID 27455 - do not send this segment if the
		// qualifier or ID are blank
		if(strIdentifier != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdentifier)) {
			EndANSISegment(OutputString);

			// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
			arystrQualifiers.Add(strIdentifier);
		}

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2310B_Prof");

	return Error_Other;
}

// (j.jones 2011-08-23 09:17) - PLID 44984 - supported 2310B_Inst
int CEbilling::ANSI_5010_2310B_Inst() {

	//Operating Physician Name

	//Supposedly this is needed on all claims with a surgical procedure code.
	//We simply look at the Box 77 setup.

	CString OutputString,str;

	try {

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		str = "\r\n2310B\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310B - OPERATING PHYSICIAN NAME						1

//326		250		NM1		Operating Physician Name				S		1

		//The provider sent in this loop is controlled by the Box 77 setup. The provider ID
		//has already been loaded in LoadClaimInfo.

		_RecordsetPtr rs;
		if(m_pEBillingInfo->Box77Setup == 3) {
			//referring physician
			rs = CreateParamRecordset("SELECT PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, PersonT.SocialSecurity, ReferringPhysT.* "
				"FROM PersonT LEFT JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
				"WHERE PersonT.ID = {INT}", m_pEBillingInfo->UB_2310B_ProviderID);
		}
		else {
			//regular provider (either bill prov. or G1)
			// (j.jones 2011-11-07 14:44) - PLID 46299 - added support for ProvidersT.UseCompanyOnClaims
			rs = CreateParamRecordset("SELECT PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, PersonT.SocialSecurity, PersonT.Company, "
				"ProvidersT.* "
				"FROM PersonT LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
				"WHERE PersonT.ID = {INT}", m_pEBillingInfo->UB_2310B_ProviderID);
		}

		if(rs->eof) {
			//if the recordset is empty, there is no operating provider. So halt everything!!!
			rs->Close();
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				str.Format("Could not open operating provider information for '%s', Bill ID %li. (ANSI_2310B_Inst)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open operating provider information. (ANSI_2310B_Inst)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M	ID	2/3

		//static value "72"

		OutputString += ParseANSIField("72", 2, 3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//static value "1" for person, "2" for non-person entity
		str = "1";

		// (j.jones 2011-11-07 14:46) - PLID 46299 - check the UseCompanyOnClaims setting to send the provider as an organization
		CString strProviderCompany = "";
		BOOL bUseProviderCompanyOnClaims = FALSE;

		if(m_pEBillingInfo->Box77Setup != 3) {
			bUseProviderCompanyOnClaims = VarBool(rs->Fields->Item["UseCompanyOnClaims"]->Value, FALSE);
			strProviderCompany = VarString(rs->Fields->Item["Company"]->Value, "");
			strProviderCompany.TrimLeft();
			strProviderCompany.TrimRight();
			if(strProviderCompany.IsEmpty()) {
				//can't use the company if it is blank
				bUseProviderCompanyOnClaims = FALSE;
			}

			if(bUseProviderCompanyOnClaims) {
				str = "2";
			}
			else {
				str = "1";
			}
		}		
		OutputString += ParseANSIField(str, 1, 1);

		//NM103	1035		Name Last / Org Name					O	AN	1/35

		if(bUseProviderCompanyOnClaims) {
			str = strProviderCompany;
		}
		else {
			str = GetFieldFromRecordset(rs, "Last");
			str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		}
		OutputString += ParseANSIField(str, 1, 35);

		//NM104	1036		Name First								O	AN	1/25

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "First");
			str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		}
		OutputString += ParseANSIField(str, 1, 25);

		//NM105	1037		Name Middle								O	AN	1/25

		// (j.jones 2013-04-11 09:40) - PLID 56166 - if ANSI_HideRefPhyFields is enabled,
		// do not send the middle name in this loop, even if we happen to be sending a
		// provider
		if(m_HCFAInfo.ANSI_HideRefPhyFields == 1) {
			str = "";
		}
		else {
			if(bUseProviderCompanyOnClaims) {
				str = "";
			}
			else {
				str = GetFieldFromRecordset(rs, "Middle");
				str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
			}
		}
		OutputString += ParseANSIField(str, 1, 25);

		//NM106	NOT USED

		OutputString += "*";

		//NM107	1039		Name Suffix								O	AN	1/10

		// (j.jones 2013-04-11 09:40) - PLID 56166 - if ANSI_HideRefPhyFields is enabled,
		// do not send the suffix in this loop, even if we happen to be sending a
		// provider
		if(m_HCFAInfo.ANSI_HideRefPhyFields == 1) {
			str = "";
		}
		else {
			if(bUseProviderCompanyOnClaims) {
				str = "";
			}
			else {
				str = GetFieldFromRecordset(rs, "Title");
			}
		}
		OutputString += ParseANSIField(str, 1, 10);

		//NM108	66			ID Code Qualifier						X 1	ID	1/2

		//in 5010 this is always the NPI
		str = GetFieldFromRecordset(rs, "NPI");
		// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens,
		// although NPI should always be numeric
		str = StripSpacesAndHyphens(str);

		if(str != "")
			OutputString += ParseANSIField("XX", 1, 2);
		else
			OutputString += "*";

		//NM109	67			ID Code									X	AN	2/80

		if(str != "")
			OutputString += ParseANSIField(str, 2, 80);
		else
			OutputString += "*";

		//NM110	NOT USED

		OutputString += "*";

		//NM111	NOT USED

		OutputString += "*";

		//NM112	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

//329	271		REF		Operating Physician Secondary Info.		S		5

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		// we need to silently skip adding additional REF segments
		// that have the same qualifier, so track them in an array
		CStringArray arystrQualifiers;

		//REF01	128			Reference Identification Qualifier		M	ID	2/3
		
		// (j.jones 2010-04-14 10:43) - PLID 38194 - the ID is now calculated in a shared function
		CString strIdentifier = "", strID = "", strLoadedFrom = "";
		//this re-uses the 2310A REF function, because we pass in our own number, setup option, and qualifier,
		//and we *do* want the 2310A override if it is in use
		EBilling_Calculate2310A_REF(strIdentifier,strID,strLoadedFrom,m_pEBillingInfo->UB_2310B_ProviderID,
			m_pEBillingInfo->UB92SetupID, m_UB92Info.UB04Box77Qual, m_UB92Info.UB04Box77Num, m_UB92Info.UB04Box77Setup,
			m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->InsuredPartyID, m_pEBillingInfo->BillLocation);

		//if the qualifier is XX, send nothing
		if(m_bDisableREFXX && strIdentifier.CompareNoCase("XX") == 0) {
			strIdentifier = "";
			strID = "";
			//log that this happened
			Log("Ebilling file tried to export 2310B REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->UB_2310B_ProviderID, m_pEBillingInfo->BillID);
		}
		
		OutputString += ParseANSIField(strIdentifier,2,3);

		//REF02	127			Reference Identification				X 1	AN	1/50

		OutputString += ParseANSIField(strID,1,50);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		// do not send this segment if the
		// qualifier or ID are blank
		if(strIdentifier != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdentifier)) {
			EndANSISegment(OutputString);

			// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
			arystrQualifiers.Add(strIdentifier);
		}

		rs->Close();

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2310B_Inst");

	return Error_Other;
}

// (j.jones 2011-08-23 09:17) - PLID 44984 - supported 2310C_Inst
int CEbilling::ANSI_5010_2310C_Inst() {

	//Other Provider Name

	//We fill this based on Box 78.

	CString OutputString,str;

	try {

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2310C\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310C - OTHER PROVIDER NAME						1

//331		250		NM1		Other Provider Name						S		1

		//The provider sent in this loop is controlled by the Box 78 setup. The provider ID
		//has already been loaded in LoadClaimInfo.

		_RecordsetPtr rs;
		if(m_pEBillingInfo->Box78Setup == 3) {
			//referring physician
			rs = CreateParamRecordset("SELECT PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, PersonT.SocialSecurity, ReferringPhysT.* "
				"FROM PersonT LEFT JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
				"WHERE PersonT.ID = {INT}", m_pEBillingInfo->UB_2310C_ProviderID);
		}
		else {
			//regular provider (either bill prov. or G1)
			// (j.jones 2011-11-07 14:44) - PLID 46299 - added support for ProvidersT.UseCompanyOnClaims
			rs = CreateParamRecordset("SELECT PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, PersonT.SocialSecurity, PersonT.Company, "
				"ProvidersT.* "
				"FROM PersonT LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
				"WHERE PersonT.ID = {INT}", m_pEBillingInfo->UB_2310C_ProviderID);
		}

		if(rs->eof) {
			//if the recordset is empty, there is no operating provider. So halt everything!!!
			rs->Close();
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				str.Format("Could not open other provider information for '%s', Bill ID %li. (ANSI_2310C_Inst)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open other provider information. (ANSI_2310C_Inst)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M	ID	2/3

		//static value "ZZ"

		OutputString += ParseANSIField("ZZ", 2, 3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//static value "1" for person, "2" for non-person entity
		str = "1";

		// (j.jones 2011-11-07 14:46) - PLID 46299 - check the UseCompanyOnClaims setting to send the provider as an organization
		CString strProviderCompany = "";
		BOOL bUseProviderCompanyOnClaims = FALSE;

		if(m_pEBillingInfo->Box78Setup != 3) {
			bUseProviderCompanyOnClaims = VarBool(rs->Fields->Item["UseCompanyOnClaims"]->Value, FALSE);
			strProviderCompany = VarString(rs->Fields->Item["Company"]->Value, "");
			strProviderCompany.TrimLeft();
			strProviderCompany.TrimRight();
			if(strProviderCompany.IsEmpty()) {
				//can't use the company if it is blank
				bUseProviderCompanyOnClaims = FALSE;
			}

			if(bUseProviderCompanyOnClaims) {
				str = "2";
			}
			else {
				str = "1";
			}
		}		
		OutputString += ParseANSIField(str, 1, 1);

		//NM103	1035		Name Last / Org Name					O	AN	1/35

		if(bUseProviderCompanyOnClaims) {
			str = strProviderCompany;
		}
		else {
			str = GetFieldFromRecordset(rs, "Last");
			str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		}
		OutputString += ParseANSIField(str, 1, 35);

		//NM104	1036		Name First								O	AN	1/25

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "First");
			str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		}
		OutputString += ParseANSIField(str, 1, 25);

		//NM105	1037		Name Middle								O	AN	1/25

		// (j.jones 2013-04-11 09:40) - PLID 56166 - if ANSI_HideRefPhyFields is enabled,
		// do not send the middle name in this loop if we are sending a referring physician
		if(m_HCFAInfo.ANSI_HideRefPhyFields == 1 && m_pEBillingInfo->Box78Setup == 3) {
			str = "";
		}
		else {
			if(bUseProviderCompanyOnClaims) {
				str = "";
			}
			else {
				str = GetFieldFromRecordset(rs, "Middle");
				str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
			}
		}
		OutputString += ParseANSIField(str, 1, 25);

		//NM106	NOT USED

		OutputString += "*";

		//NM107	1039		Name Suffix								O	AN	1/10

		// (j.jones 2013-04-11 09:40) - PLID 56166 - if ANSI_HideRefPhyFields is enabled,
		// do not send the suffix in this loop if we are sending a referring physician
		if(m_HCFAInfo.ANSI_HideRefPhyFields == 1 && m_pEBillingInfo->Box78Setup == 3) {
			str = "";
		}
		else {
			if(bUseProviderCompanyOnClaims) {
				str = "";
			}
			else {
				str = GetFieldFromRecordset(rs, "Title");
			}
		}
		OutputString += ParseANSIField(str, 1, 10);

		//NM108	66			Identification Code	Qualifier			X 1	ID	1/2

		//in 5010 this is always the NPI
		str = GetFieldFromRecordset(rs, "NPI");
		// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens,
		// although NPI should always be numeric
		str = StripSpacesAndHyphens(str);

		if(str != "")
			OutputString += ParseANSIField("XX", 1, 2);
		else
			OutputString += "*";

		//NM109	67			ID Code									X	AN	2/80

		if(str != "")
			OutputString += ParseANSIField(str, 2, 80);
		else
			OutputString += "*";

		//NM110	NOT USED

		OutputString += "*";

		//NM111	NOT USED

		OutputString += "*";

		//NM112	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

//334	271		REF		Other Provider Secondary Info.		S		5

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		// we need to silently skip adding additional REF segments
		// that have the same qualifier, so track them in an array
		CStringArray arystrQualifiers;

		//REF01	128			Reference Identification Qualifier		M	ID	2/3
		
		// (j.jones 2010-04-14 10:43) - PLID 38194 - the ID is now calculated in a shared function
		CString strIdentifier = "", strID = "", strLoadedFrom = "";
		//this re-uses the 2310A REF function, because we pass in our own number, setup option, and qualifier,
		//and we *do* want the 2310A override if it is in use
		EBilling_Calculate2310A_REF(strIdentifier,strID,strLoadedFrom,m_pEBillingInfo->UB_2310C_ProviderID,
			m_pEBillingInfo->UB92SetupID, m_UB92Info.UB04Box78Qual, m_UB92Info.Box83Num, m_UB92Info.Box83Setup,
			m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->InsuredPartyID, m_pEBillingInfo->BillLocation);

		// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
		if(m_bDisableREFXX && strIdentifier.CompareNoCase("XX") == 0) {
			strIdentifier = "";
			strID = "";
			//log that this happened
			Log("Ebilling file tried to export 2310A REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->UB_2310C_ProviderID, m_pEBillingInfo->BillID);
		}
		
		OutputString += ParseANSIField(strIdentifier,2,3);

		//REF02	127			Reference Identification				X 1	AN	1/50

		OutputString += ParseANSIField(strID,1,50);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		// (j.jones 2007-09-20 10:51) - PLID 27455 - do not send this segment if the
		// qualifier or ID are blank
		if(strIdentifier != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdentifier)) {
			EndANSISegment(OutputString);

			// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
			arystrQualifiers.Add(strIdentifier);
		}

		rs->Close();

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2310C_Inst");

	return Error_Other;
}

int CEbilling::ANSI_5010_2310C_Prof() {

	//Service Facility Location

	//this is 2310E on the UB92

	//JJ - This is not used if the Place Of Service is not the same location used
	//in loop 2010AA - Billing Provider. That location should be the location of the bill
	//(make sure it is!). So, this is not used if the POS and the bill location are the same.
	
	//when 2000A is used (always!) this is not used on a UB92

	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2310C\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	try {

		CString OutputString,str;
		_variant_t var;
		
		_RecordsetPtr rs;
		// (j.jones 2013-04-24 17:25) - PLID 55564 - supported the preference to use the patient's address
		// if the POS code is 12, this is assigned in LoadClaimInfo
		if(m_pEBillingInfo->bSendPatientAddressAsPOS) {
			rs = CreateParamRecordset("SELECT 'Patient Home' AS Name, '' AS NPI, Address1, Address2, City, State, Zip "
				"FROM PersonT "
				"WHERE ID = {INT}", m_pEBillingInfo->PatientID);
		}
		else {
			//get the location where the service was performed
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			rs = CreateParamRecordset("SELECT Name, NPI, Address1, Address2, City, State, Zip "
				"FROM LocationsT "
				"WHERE ID = {INT}",m_pEBillingInfo->nPOSID);
		}
		if(rs->eof) {
			str = "Error opening location information.";
			rs->Close();
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310C - SERVICE FACILITY LOCATION						1

//269		250		NM1		Service Facility Location				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M 1	ID	2/3

		//static "77"
		OutputString += ParseANSIField("77",2,3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//static "2"
		OutputString += ParseANSIField("2",1,1);

		//NM103	1035		Name Last / Org Name					O 1	AN	1/60

		//LocationsT.Name
		str = GetFieldFromRecordset(rs,"Name");
		OutputString += ParseANSIField(str,1,60);

		//NM104	NOT USED
		OutputString += "*";

		//NM105	NOT USED
		OutputString += "*";

		//NM106	NOT USED
		OutputString += "*";

		//NM107	NOT USED
		OutputString += "*";

		//NM108	66			ID Code Qualifier						X 1	ID	1/2

		//always XX - NPI
		CString strID = GetFieldFromRecordset(rs, "NPI");
		// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens,
		// although NPI should always be numeric
		strID = StripSpacesAndHyphens(strID);

		//do not output if the ID is blank
		if(strID != "")
			OutputString += ParseANSIField("XX", 1, 2);
		else
			OutputString += "*";

		//NM109	67			ID Code									X 1	AN	2/80

		//LocationsT.NPI
		if(strID != "")
			OutputString += ParseANSIField(strID, 2, 80);
		else
			OutputString += "*";

		//NM110	NOT USED
		OutputString += "*";

		//NM111	NOT USED
		OutputString += "*";

		//NM112	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

//272		265		N3		Service Facility Location Address		R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//N301	166			Address Information						M 1	AN	1/55

		//LocationsT.Address1
		str = GetFieldFromRecordset(rs, "Address1");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information						O 1	AN	1/55

		//LocationsT.Address2
		str = GetFieldFromRecordset(rs, "Address2");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

//273		270		N4		Service Facility Loc. City/State/Zip	R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O 1	AN	2/30

		//LocationsT.City
		str = GetFieldFromRecordset(rs, "City");

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State or Prov Code						O 1	ID	2/2
		
		//LocationsT.State
		str = GetFieldFromRecordset(rs, "State");

		// (j.jones 2012-01-11 10:48) - PLID 47461 - trim and auto-capitalize this
		str.TrimLeft(); str.TrimRight();
		str.MakeUpper();
		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O 1	ID	3/15

		//in 5010, this must be 9 digits
		str = GetFieldFromRecordset(rs, "Zip");
		// (j.jones 2013-05-06 17:48) - PLID 55100 - don't strip all numbers, just spaces and hyphens
		str = StripSpacesAndHyphens(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O 1	ID	2/3

		//this is only required if the addess is outside the US
		//as of right now, we can't support it
		
		OutputString += "*";

		//N405	NOT USED
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		//N407	1715		Country Subdivision Code				X 1	ID	1/3

		//an even more detailed optional value for country, since we
		//do not support countries, we don't support this either
		
		OutputString += "*";

		EndANSISegment(OutputString);

//275		271		REF		Service Facility Loc. Secondary Info.	S		3

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//0B - State License Number
		//G2 - Provider Commercial Number
		//LU - Location Number

		OutputString = "REF";

		//REF01	128			Reference Ident Qual					M 1	ID	2/3

		str = "";
		CString strQual = "";

		// (j.jones 2013-04-24 17:25) - PLID 55564 - do not send an ID if we are sending the patient address
		if(!m_pEBillingInfo->bSendPatientAddressAsPOS) {
			// (a.walling 2007-08-16 15:24) - PLID 27094 - This should check the POS, not the Location!
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			_RecordsetPtr rsFacID = CreateParamRecordset("SELECT FacilityID, Qualifier FROM InsuranceFacilityID WHERE LocationID = {INT} AND InsCoID = {INT}",m_pEBillingInfo->nPOSID,m_pEBillingInfo->InsuranceCoID);
			
			if(!rsFacID->eof) {
				str = AdoFldString(rsFacID, "FacilityID","");
				strQual = AdoFldString(rsFacID, "Qualifier","");
				strQual.TrimRight();
			}
			rsFacID->Close();

			if(strQual.IsEmpty()) {
				//if it is empty, validation will have warned them, but we should attempt to fill it in

				strQual = m_strLast2010AAQual;

				//JJ - we can't just outright use the qualifier from 2010AA
				//because it has to be from the following list
				
				//0B - State License Number
				//G2 - Provider Commercial Number
				//LU - Location Number

				//so make sure of it
				if(strQual != "0B" && strQual != "G2" && strQual != "LU") {
					//we're just guessing now
					strQual = "LU";
				}
			}
		}

		if(str != "" && strQual != "")
			OutputString += ParseANSIField(strQual, 2, 3);

		//REF02	127			Reference Ident							X 1	AN	1/50
		
		if(str != "" && strQual != "")
			OutputString += ParseANSIField(str, 1, 50);
		
		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		if(str != "" && strQual != "") {
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//277		345		PER		Service Facility Contact Information			S		1

		//seemingly not needed on any claim type we support

///////////////////////////////////////////////////////////////////////////////

		rs->Close();

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2310C_Prof");

	return Error_Other;
}

int CEbilling::ANSI_5010_2310D_Prof() {

	//Supervising Provider Name

	// (j.jones 2008-12-11 13:21) - This will send the Supervising Provider from the bill, if one is selected.
	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2310D\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs = CreateParamRecordset("SELECT PersonT.ID, PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, "
					"PersonT.SocialSecurity, ProvidersT.* "
					"FROM BillsT "
					"INNER JOIN PersonT ON BillsT.SupervisingProviderID = PersonT.ID "
					"INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
					"WHERE BillsT.ID = {INT}", m_pEBillingInfo->BillID);

		if(rs->eof) {
			//if the recordset is empty, there is no referring provider. So halt everything!!!
			rs->Close();
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open supervising provider information for '%s', Bill ID %li. (ANSI_2310E_Prof)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open supervising provider information. (ANSI_2310E_Prof)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310D - SUPERVISING PROVIDER NAME						1

//280		250		NM1		Supervising Provider Name				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M 1	ID	2/3

		//static "DQ"
		OutputString += ParseANSIField("DQ",2,3);

		//NM102 1065		Entity Type Qualifier					M 1	ID	1/1
		
		//static "1" for "Person" (organization is not permitted)
		OutputString += ParseANSIField("1",1,1);

		//NM103 1035		Name Last or OrganizationName			O 1	AN	1/60

		str = GetFieldFromRecordset(rs, "Last");
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 60);

		//NM104 1036		Name First								O 1	AN	1/35

		str = GetFieldFromRecordset(rs, "First");
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 35);
		
		//NM105 1037		Name Middle								O 1	AN	1/25

		str = GetFieldFromRecordset(rs, "Middle");
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 25);

		//NM106	NOT USED

		OutputString += "*";
		
		//NM107 1039		Name Suffix								O 1	AN	1/10

		str = GetFieldFromRecordset(rs, "Title");
		OutputString += ParseANSIField(str, 1, 10);

		//NM108 66			Identification Code Qualifier			X 1	ID	1/2
		
		//only XX - NPI is supported

		CString strID = GetFieldFromRecordset(rs, "NPI");
		// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens,
		// although NPI should always be numeric
		strID = StripSpacesAndHyphens(strID);

		//do not output if the ID is blank
		if(strID != "") {
			OutputString += ParseANSIField("XX", 1, 2);
		}
		else {
			OutputString += "*";
		}

		//NM109 67			Identification Code						X	AN	2/80

		if(strID != "") {
			OutputString += ParseANSIField(strID, 2, 80);
		}
		else {
			OutputString += "*";
		}

		//NM110	NOT USED

		OutputString += "*";

		//NM111	NOT USED

		OutputString += "*";

		//NM112	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

//283		271		REF		Supervising Prov. Secondary Info.		S		4

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//we need to silently skip adding additional REF segments
		//that have the same qualifier, so track them in an array
		CStringArray arystrQualifiers;

		//REF01	128			Reference Identification Qualifier		M 1	ID	2/3

		//only qualifiers 0B, 1G, G2, and LU are valid, but we allow them to set up what they wish, for now

		// (j.jones 2010-04-14 10:20) - PLID 38194 - the ID is now calculated in a shared function
		CString strIdent = "";
		strID = "";
		CString strLoadedFrom = "";
		EBilling_Calculate2310E_REF(strIdent,strID,strLoadedFrom, m_pEBillingInfo->nSupervisingProviderID,m_pEBillingInfo->HCFASetupID,
			m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->InsuredPartyID, m_pEBillingInfo->BillLocation);

		// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
		if(m_bDisableREFXX && strIdent.CompareNoCase("XX") == 0) {
			strIdent = "";
			strID = "";
			//log that this happened
			Log("Ebilling file tried to export 2310E REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->nSupervisingProviderID, m_pEBillingInfo->BillID);
		}
		
		OutputString += ParseANSIField(strIdent,2,3);

		//REF02	127			Reference Identification				X 1	AN	1/50

		OutputString += ParseANSIField(strID,1,50);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		//do not send this segment if the qualifier or ID are blank
		if(strIdent != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdent)) {
			EndANSISegment(OutputString);

			//add this qualifier to our array
			arystrQualifiers.Add(strIdent);
		}

		rs->Close();
		
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2310D_Prof");

	return Error_Other;
}

/*
int CEbilling::ANSI_5010_2310E_Prof() {

	//Ambulance Pickup Location

	//Required when billing for ambulance or non-emergency transportation services.

	// (j.jones 2010-10-14 16:15) - we currently do not support this

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310E - AMBULANCE PICK-UP LOCATION						1

//285		2500	NM1		Ambulance Pick-up Location				S		1
//287		2650	N3		Ambulance Pick-up Location Address		R		1
//288		2700	N4		Ambulance Pick-up Location City, State, ZIP Code	R		1

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2310E_Prof");

	return Error_Other;
}
*/

/*
int CEbilling::ANSI_5010_2310F_Prof() {

	//Ambulance Drop-Off Location

	//Required when billing for ambulance or non-emergency transportation services.

	// (j.jones 2010-10-14 16:15) - we currently do not support this

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310E - AMBULANCE PICK-UP LOCATION						1

//290		2500	NM1		Ambulance Drop-off Location					S	1
//292		2650	N3		Ambulance Drop-off Location Address			R	1
//293		2700	N4		Ambulance Drop-off Location City, State, ZIP Code	R	1

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2310F_Prof");

	return Error_Other;
}
*/

// (j.jones 2010-10-18 16:17) - PLID 40346 - initial implementation of ANSI 5010 Institutional claim
int CEbilling::ANSI_5010_2310E_Inst() {

	//Service Facility Name

	//JJ - This is not used if the Place Of Service is not the same location used
	//in loop 2010AA - Billing Provider. That location should be the location of the bill
	//(make sure it is!). So, this is not used if the POS and the bill location are the same.
	
	//when 2000A is used (always!) this is not used on a UB92
	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2310E\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	try {

		CString OutputString,str;
		_variant_t var;
		_RecordsetPtr rs;

		// (j.jones 2013-04-24 17:25) - PLID 55564 - supported the preference to use the patient's address
		// if the POS code is 12, this is assigned in LoadClaimInfo
		if(m_pEBillingInfo->bSendPatientAddressAsPOS) {
			rs = CreateParamRecordset("SELECT 'Patient Home' AS Name, '' AS NPI, Address1, Address2, City, State, Zip "
				"FROM PersonT "
				"WHERE ID = {INT}", m_pEBillingInfo->PatientID);
		}
		else {
			//get the location where the service was performed
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			rs = CreateParamRecordset("SELECT Name, NPI, Address1, Address2, City, State, Zip "
				"FROM LocationsT "
				"WHERE ID = {INT}",m_pEBillingInfo->nPOSID);
		}
		if(rs->eof) {
			str = "Error opening location information.";
			rs->Close();
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310E - SERVICE FACILITY NAME					1

//341		250		NM1		Service Facility Name					S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M 1	ID	2/3

		//static "77"
		OutputString += ParseANSIField("77",2,3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//static "2"
		OutputString += ParseANSIField("2",1,1);

		//NM103	1035		Name Last / Org Name					O 1	AN	1/60

		//LocationsT.Name
		str = GetFieldFromRecordset(rs,"Name");
		OutputString += ParseANSIField(str,1,60);

		//NM104	NOT USED
		OutputString += "*";

		//NM105	NOT USED
		OutputString += "*";

		//NM106	NOT USED
		OutputString += "*";

		//NM107	NOT USED
		OutputString += "*";

		//NM108	66			ID Code Qualifier						X 1	ID	1/2
		
		CString strNPI = AdoFldString(rs, "NPI", "");
		// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens,
		// although NPI should always be numeric
		strNPI = StripSpacesAndHyphens(strNPI);

		//static XX
		if(strNPI.IsEmpty()) {
			OutputString += "*";
		}
		else {
			OutputString += ParseANSIField("XX",1,1);
		}

		//NM109	67			ID Code									X 1	AN	2/80

		//LocationsT.NPI
		if(strNPI.IsEmpty()) {
			OutputString += "*";
		}
		else {
			OutputString += ParseANSIField(strNPI,2,80);
		}

		//NM110	NOT USED
		OutputString += "*";

		//NM111	NOT USED
		OutputString += "*";

		//NM112	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

//344		265		N3		Service Facility Location Address		R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//N301	166			Address Information						M 1	AN	1/55

		//LocationsT.Address1
		str = GetFieldFromRecordset(rs, "Address1");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information						O 1	AN	1/55

		//LocationsT.Address2
		str = GetFieldFromRecordset(rs, "Address2");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

//345		270		N4		Service Facility Loc. City/State/Zip	R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O 1	AN	2/30

		//LocationsT.City
		str = GetFieldFromRecordset(rs, "City");

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State or Prov Code						O 1	ID	2/2
		
		//LocationsT.State
		str = GetFieldFromRecordset(rs, "State");

		// (j.jones 2012-01-11 10:48) - PLID 47461 - trim and auto-capitalize this
		str.TrimLeft(); str.TrimRight();
		str.MakeUpper();
		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O 1	ID	3/15
		str = GetFieldFromRecordset(rs, "Zip");
		// (j.jones 2013-05-06 17:48) - PLID 55100 - don't strip all numbers, just spaces and hyphens
		str = StripSpacesAndHyphens(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O 1	ID	2/3

		//this is only required if the addess is outside the US
		//as of right now, we can't support it
		
		OutputString += "*";

		//N405	NOT USED
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		//N407	1715		Country Subdivision Code				X 1	ID	1/3

		//an even more detailed optional value for country, since we
		//do not support countries, we don't support this either
		
		OutputString += "*";

		EndANSISegment(OutputString);

//347		271		REF		Service Facility Loc. Secondary Info.	S		5

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01	128			Reference Ident Qual					M 1	ID	2/3

		str = "";
		CString strQual = "";

		// (j.jones 2013-04-24 17:25) - PLID 55564 - do not send an ID if we are sending the patient address
		if(!m_pEBillingInfo->bSendPatientAddressAsPOS) {

			//technically only 0B, G2, and LU are valid, but we allow anything

			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			_RecordsetPtr rsFacID = CreateParamRecordset("SELECT FacilityID, Qualifier FROM InsuranceFacilityID WHERE LocationID = {INT} AND InsCoID = {INT}",m_pEBillingInfo->BillLocation,m_pEBillingInfo->InsuranceCoID);
			
			if(!rsFacID->eof) {
				str = AdoFldString(rsFacID, "FacilityID","");
				strQual = AdoFldString(rsFacID, "Qualifier","");
				strQual.TrimRight();
			}
			rsFacID->Close();
			
			//if empty, use LU - Facility ID
			if(strQual.IsEmpty())
				strQual = "LU";
		}

		if(str != "" && strQual != "")
			OutputString += ParseANSIField(strQual, 2, 3);

		//REF02	127			Reference Ident							X 1	AN	1/50
		
		if(str != "" && strQual != "")
			OutputString += ParseANSIField(str, 1, 50);
		
		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		if(str != "")
			EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

		rs->Close();

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2310E_Inst");

	return Error_Other;
}

// (j.jones 2012-03-21 14:18) - PLID 48870 - this now requires an OtherInsuranceInfo struct
int CEbilling::ANSI_5010_2320(OtherInsuranceInfo oInfo) {

	//Other Subscriber Information

	//JJ - This loop will be used if there is an "other insurance company"
	//selected on the bill.
	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2320\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		rs = CreateParamRecordset("SELECT InsuranceCoT.PersonID AS OtherInsCoID, Box12Accepted, Box13Accepted, "
			"InsuredPartyT.RespTypeID, InsuredPartyT.RelationToPatient, InsuredPartyT.PolicyGroupNum, "
			"InsurancePlansT.PlanName, RespTypeT.Priority, InsuranceCoT.InsType, PersonT.BirthDate, PersonT.Gender, "
			"InsuredPartyT.SecondaryReasonCode "
			"FROM InsuredPartyT "
			"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
			"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
			"WHERE InsuredPartyT.PersonID = {INT}\r\n"
			" "
			"SELECT HCFABox13Over FROM BillsT WHERE ID = {INT}", oInfo.nInsuredPartyID, m_pEBillingInfo->BillID);

		if(rs->eof) {
			//if the recordset is empty, there is no secondary insurance. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open secondary insurance information from this patient's bill for '%s', Bill ID %li. (ANSI_2320)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open secondary insurance information from this patient's bill. (ANSI_2320)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		FieldsPtr fields = rs->Fields;

		long nBox12Accepted = VarLong(fields->Item["Box12Accepted"]->Value, 3);
		long nBox13Accepted = VarLong(fields->Item["Box13Accepted"]->Value, 3);
		long nOtherInsCoID = VarLong(fields->Item["OtherInsCoID"]->Value, -1);

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2320 - OTHER SUBSCRIBER INFORMATION					10

//295		290		SBR		Other Subscriber Information			S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "SBR";

		//SBR01	1138		Payer Resp Seq No Code					M 1	ID	1/1

		//Code identifying the insurance carrier's level of responsibility for a payment of a claim
		
		// (j.jones 2014-08-25 09:40) - PLID 54213 - this qualifier has already been calculated
		CString strSBR01 = OutputSBR01Qualifier(oInfo.sbr01Qual);
		OutputString += ParseANSIField(strSBR01, 1, 1);

		//SBR02	1069		Individual Relationship Code				O 1	ID	2/2

		str = GetFieldFromRecordset(rs, "RelationToPatient");

		//in 5010 this list has been trimmed, all previously known, valid relationships
		//are now G8 - Other

		if(str == "Spouse") {
			str = "01";
		}
		else if(str == "Child") {
			str = "19";
		}
		// (j.jones 2011-12-07 16:08) - PLID 40959 - Self is valid in 2320, it was errantly missing before
		else if(str == "Self") {
			str = "18";
		}
		else if(str == "Employee") {
			str = "20";
		}
		else if(str == "Organ Donor") {
			str = "39";
		}
		else if(str == "Cadaver Donor") {
			str = "40";
		}
		else if(str == "Life Partner") {
			str = "53";
		}
		//dump all other previously valid relationships into Other, not Unknown
		else if(str == "Other") {

			// (j.jones 2011-06-15 16:54) - PLID 40959 - These relationships are invalid in 5010,
			// and in turn, completely pointless. They have been corrected to "Other" in the data
			// and are no longer selectable options in Practice.
			/*
			|| str == "Other Relationship"
			|| str == "Grandparent"
			|| str == "Grandchild"
			|| str == "Nephew Or Niece"
			|| str == "Adopted Child"
			|| str == "Foster Child"
			|| str == "Ward"
			|| str == "Stepchild"
			|| str == "Handicapped Dependent"
			|| str == "Sponsored Dependent"
			|| str == "Dependent of a Minor Dependent"
			|| str == "Significant Other"
			|| str == "Mother"
			|| str == "Father"
			|| str == "Other Adult"
			|| str == "Emancipated Minor"
			|| str == "Injured Plaintiff"
			|| str == "Child Where Insured Has No Financial Responsibility"
			*/

			str = "G8";
		}
		else if(str == "Unknown") {
			str = "21";
		}
		else {
			//no known relationship
			str = "21";
		}
		
		OutputString += ParseANSIField(str, 2, 2);

		//SBR03	127			Reference Identification				O 1	AN	1/50

		//InsuredPartyT.PolicyGroupNum
		str = GetFieldFromRecordset(rs,"PolicyGroupNum");
		OutputString += ParseANSIField(str,1,50);

		//SBR04	93			Name									O 1	AN	1/60

		//InsurancePlansT.PlanName
		// (j.jones 2011-02-21 13:16) - PLID 32848 - Don't export the plan name
		// if the group number exists. This rule used to be only for UB claims,
		// but in 5010 it applies to both UB and HCFA claims.
		// (j.jones 2012-05-14 14:01) - PLID 50338 - The ANSI Specs state that this field
		//is required when the group number is blank, and optional otherwise.
		//Some companies still require it even when we have the group number, so it is now
		//a HCFA/UB group setting to send Always (1 - default) or when the group number is blank (0).
		// (j.jones 2012-10-04 11:10) - PLID 53018 - split the setting up for a 2320-only version,
		// and added a Never (2) option
		BOOL bSendPlanName = TRUE;
		if((m_actClaimType != actInst && m_HCFAInfo.ANSI_2320_SBR04 == 2) || (m_actClaimType == actInst && m_UB92Info.ANSI_2320_SBR04 == 2)) {
			//never send
			bSendPlanName = FALSE;
		}
		else if((m_actClaimType != actInst && m_HCFAInfo.ANSI_2320_SBR04 == 0) || (m_actClaimType == actInst && m_UB92Info.ANSI_2320_SBR04 == 0)) {
			//do not send if the group number is filled
			if(!str.IsEmpty()) {
				bSendPlanName = FALSE;
			}
		}
		if(!bSendPlanName) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs,"PlanName");
		}
		OutputString += ParseANSIField(str,1,60);
	
		//SBR05	1336		Insurance Type Code						O 1	ID	1/3

		// (j.jones 2007-04-26 09:09) - PLID 25800 - This is only used when
		// Medicare is secondary, and we're not sending P in SBR01. It is also
		// not used on the UB92.

		// (j.jones 2008-09-09 10:08) - PLID 18695 - converted NSF Code to InsType
		//InsType is used in SBR09, but we need it now
		InsuranceTypeCode eInsType = (InsuranceTypeCode)AdoFldLong(rs, "InsType", (long)itcInvalid);

		str = "";

		// (j.jones 2010-10-15 14:49) - PLID 40953 - added Part A as well
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if (oInfo.sbr01Qual != SBR01Qualifier::sbrP && m_actClaimType != actInst
			&& (eInsType == itcMedicarePartB || eInsType == itcMedicarePartA)) {

			//it is Medicare Secondary, so send SBR05

			//12 - Medicare Secondary Working Aged Beneficiary or Spouse with Employer Group Health Plan
			//13 - Medicare Secondary End-Stage Renal Disease Beneficiary in the 12 month coordination period with an employer’s group health plan
			//14 - Medicare Secondary, No-fault Insurance including Auto is Primary
			//15 - Medicare Secondary Worker’s Compensation
			//16 - Medicare Secondary Public Health Service (PHS) or Other Federal Agency
			//41 - Medicare Secondary Black Lung
			//42 - Medicare Secondary Veteran’s Administration
			//43 - Medicare Secondary Disabled Beneficiary Under Age 65 with Large Group Health Plan (LGHP)
			//47 - Medicare Secondary, Other Liability Insurance is Primary

			// (j.jones 2007-04-26 09:13) - PLID 25800 - Just auto-assign 47.
			// The reasoning for this is nobody is going to know which to use, and I guarantee
			// nobody is going to care which number it is. If we put an option in the program
			// to set this up per insured party, noboby's going to fill it in, and we'd have
			// to pick a default anyways. So with that argument, we won't add a setup for this
			// until it becomes necessary.

			//TES 6/11/2007 - PLID 26257 - Sadly, Josh's comment above was overly optimistic.  Blackburn's office needs
			// to be able to set this per insured party.  So, we now pull it from InsuredPartyT.

			str = GetFieldFromRecordset(rs,"SecondaryReasonCode");
		}

		//not used on the UB92
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType != actInst)
			OutputString += ParseANSIField(str,1,3);
		else
			OutputString += "*";
		
		//SBR06	NOT USED
		OutputString += "*";

		//SBR07	NOT USED
		OutputString += "*";

		//SBR08	NOT USED
		OutputString += "*";

		//SBR09	1032		Claim Filing Indicator Code					O 1	ID	1/2

		//InsuranceCoT.InsType
		
		// (j.jones 2008-09-09 10:10) - PLID 18695 - we finally track proper
		// Insurance Types per company, so simply call GetANSISBR09CodeFromInsuranceType()
		// to get the ANSI code for our type.

		//we loaded the InsType earlier in SBR05
		str = GetANSI5010_SBR09CodeFromInsuranceType(eInsType);
		if(str.IsEmpty()) {
			str = "ZZ";
		}

		OutputString += ParseANSIField(str,1,2);

		EndANSISegment(OutputString);

/*////////////////////////////////////////////////////////////////////////////

COB SECONDARY CLAIM INFORMATION

(j.jones 2006-11-27 09:20) - PLID 23415 - The logic, while convoluted, is
like so:

- if sending a secondary claim and you include primary payment information,
  that payment information has to exist in 2320
- if payments are sent in 2320, then adjustments need to exist somewhere,
  either in 2320 or 2430, but NOT BOTH
- if adjustments are in 2320, they need to balance against the payments in
  2320 such that they equal the claim total in 2300
  (ie. $100 claim, $60 payment, need to show $40 in adjustments even if the
  account doesn't really have that amount of adjustments
- if adjustments are in 2430, then payments also have to be reported in 2430,
  in addition to the payments that still have to be reported in 2320. The
  payments are reported in both locations in this case.
- if adjustments are in 2430, they need to balance against the payments in 2430
  such that they equal the charge total in 2400

// (j.jones 2006-11-27 17:05) - PLID 23652 - the UB92 follows the same logic
// as above but the implementation differs slightly (and has UB92SetupT-based controls)

////////////////////////////////////////////////////////////////////////////*/

		// (j.jones 2009-09-30 10:24) - PLID 35712 - moved the strDocProv calculation up higher
		// as it is used in many recordsets in this function
		CString strDocProv = "";
		//if using the bill provider, filter only on that doctor's charges, otherwise get all charges
		//in ANSI, we're using the bill provider if we are using anything but the G1 provider

		// (j.jones 2006-12-01 15:50) - PLID 22110 - supported the ClaimProviderID
		// (j.jones 2008-04-03 09:23) - PLID 28995 - supported the HCFAClaimProvidersT setup
		// (j.jones 2008-08-01 10:17) - PLID 30917 - HCFAClaimProvidersT is now per insurance company
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		// (d.thompson 2011-09-12) - PLID 45393 - Properly follow ClaimProviderID on the charge when filtering the charges
		if(m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 2) {
			// (j.jones 2015-03-04 10:37) - PLID 65003 - this needs to find the 2310B rendering provider, not the 2010AA provider
			strDocProv.Format("AND (ChargesT.ClaimProviderID = %li OR (ClaimProviderID IS NULL AND DoctorsProviders IN "
				"(SELECT PersonID FROM "
				"	(SELECT ProvidersT.PersonID, "
				"	(CASE WHEN ANSI_2310B_ProviderID Is Null THEN ProvidersT.ClaimProviderID ELSE ANSI_2310B_ProviderID END) AS ProviderIDToUse "
				"	FROM ProvidersT "
				"	LEFT JOIN (SELECT * FROM HCFAClaimProvidersT WHERE InsuranceCoID = %li) AS HCFAClaimProvidersT ON ProvidersT.PersonID = HCFAClaimProvidersT.ProviderID) SubQ "
				"WHERE ProviderIDToUse = %li))) ", m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ANSI_RenderingProviderID);
		}
		else if(m_actClaimType == actInst && m_UB92Info.Box82Setup == 1) {
			strDocProv.Format("AND (ChargesT.ClaimProviderID = %li OR (ClaimProviderID IS NULL AND DoctorsProviders IN (SELECT PersonID FROM ProvidersT WHERE ClaimProviderID = %li))) ", m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->ANSI_RenderingProviderID);
		}

//299		295		CAS		Claim Level Adjustments					S		5

		// (j.jones 2006-11-24 15:15) - PLID 23415, 23652 - use the adjustment information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending adjustment information in 2320

		//find the claim payment total first, because we need it for adjustment balancing
		COleCurrency cyClaimTotal = COleCurrency(0,0);
		COleCurrency cyPaid = COleCurrency(0,0);
		COleCurrency cyAmtAdjusted = COleCurrency(0,0);

		//find the total amount paid on this bill by this insured party
		// (j.jones 2008-04-30 10:02) - PLID 27946 - needs to filter on only batched charges
		//this cannot be parameterized
		// (j.jones 2011-08-16 17:22) - PLID 44805 - We will filter out "original" and "void" charges & applies.
		// The applies part of this does not need to check LineItemCorrectionsBalancingAdjT,
		// because they are only applied to original and void charges.
		_RecordsetPtr rsPayTotal = CreateRecordset("SELECT Sum(Amount) AS PaymentAmount "
			"FROM AppliesT "
			"WHERE DestID IN (SELECT ChargesT.ID FROM ChargesT "
			"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"	WHERE BillID = %li AND ChargesT.Batched = 1 "
			"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"	%s) "
			"AND SourceID IN (SELECT LineItemT.ID FROM LineItemT "
			"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"	WHERE Deleted = 0 "
			"	AND Type = 1 "
			"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"	AND LineItemT.ID IN (SELECT ID FROM PaymentsT WHERE InsuredPartyID = %li)) ",
			m_pEBillingInfo->BillID, strDocProv, oInfo.nInsuredPartyID);
		
		if(!rsPayTotal->eof) {

			cyPaid = AdoFldCurrency(rsPayTotal, "PaymentAmount", COleCurrency(0,0));
		}
		rsPayTotal->Close();

		//get total charge amount for the claim
		//this cannot be parameterized
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		_RecordsetPtr rsCharges = CreateRecordset("SELECT Sum(dbo.GetChargeTotal(ChargesT.ID)) AS ChargeTotal "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE ChargesT.BillID = %li AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"%s "
			"GROUP BY ChargesT.BillID",m_pEBillingInfo->BillID, strDocProv);

		if(!rsCharges->eof) {
			cyClaimTotal = AdoFldCurrency(rsCharges, "ChargeTotal", COleCurrency(0,0));
		}
		rsCharges->Close();

		// (j.jones 2006-11-27 17:03) - PLID 23652 - supported this on the UB92 as well
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum

		if(!m_pEBillingInfo->IsPrimary && 
			((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1 && m_HCFAInfo.ANSI_AdjustmentLoop == 0)
			|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1 && m_UB92Info.ANSI_AdjustmentLoop == 0))) {

			// (j.jones 2008-02-25 09:53) - PLID 29077 - calculate the allowed amount now,
			// as it may be used as the approved amount or in adjustments

			//cyTotalAllowableToSend will be the value we ultimately send
			COleCurrency cyTotalAllowableToSend = COleCurrency(0,0);
			COleCurrency cyTotalAllowableAdjustmentsToInclude = COleCurrency(0,0);

			//now calculate the allowed amount for the claim level

			//this should presumably be the sum of the allowed amounts from the fee schedule,
			//for the charge provider, OtherInsuranceCompany, and Bill Location

			//calculate in a query

			// (j.jones 2006-12-01 15:55) - PLID 22110 - this should not use the ClaimProviderID
			
			// (j.jones 2007-02-22 14:03) - PLID 24884 - if the allowable is less than the paid amount per charge,
			// we have to send the paid amount for that charge. So when we total here, we will need to determine
			// on a per-charge basis what value to use, and then total up the result.
			// (j.jones 2007-03-29 10:51) - PLID 25409 - added options to determine what to send as the allowable,
			// either the fee schedule allowable, the greater of the allowable or payment, or the greater of the
			// allowable or payment + secondary responsibility
			// (j.jones 2009-08-28 17:39) - PLID 32993 - we now completely drop the allowable calculation from
			// this process, as it has since been proven time and time again that fee schedule allowables have
			// absolutely no bearing on this calculation
			// (j.jones 2010-10-19 13:39) - PLID 32848 - in 5010, the allowed & approved segments are gone,
			// but we still use the allowed amount when calculating CAS*PR

			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			// (j.jones 2009-03-16 15:06) - PLID 33544 - added charge total to the query
			// (j.jones 2009-09-30 10:21) - PLID 35712 - properly filtered on provider
			// (j.jones 2010-02-03 09:24) - PLID 37170 - include selected adjustments
			// (j.jones 2010-09-23 15:19) - PLID 40653 - supported the group & reason code change to IDs
			// (j.jones 2011-08-16 17:22) - PLID 44805 - We will filter out "original" and "void" charges & applies.
			// The applies part of this does not need to check LineItemCorrectionsBalancingAdjT,
			// because they are only applied to original and void charges.
			_RecordsetPtr rsTotals = CreateParamRecordset(FormatString("SELECT "
				"Coalesce(AppliesQ.PaymentAmount, Convert(money,'0.00')) AS PaymentTotal, "
				"Coalesce(OtherInsTotalQ.OtherInsTotal, Convert(money,'0.00')) AS OtherInsTotal, "
				"Coalesce(HCFA_AllowedAdjustmentsQ.AdjustmentTotal, Convert(money,'0.00')) AS HCFA_AllowedAdjustmentTotal, "
				"Coalesce(UB_AllowedAdjustmentsQ.AdjustmentTotal, Convert(money,'0.00')) AS UB_AllowedAdjustmentTotal "
				"FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN "
				"	(SELECT DestID, Sum(Coalesce(Amount,0)) AS PaymentAmount FROM AppliesT "
				"	WHERE SourceID IN (SELECT LineItemT.ID FROM LineItemT "
				"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"		WHERE Deleted = 0 "
				"		AND Type = 1 "
				"		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"		AND LineItemT.ID IN (SELECT ID FROM PaymentsT WHERE InsuredPartyID = {INT})) GROUP BY DestID) "
				"	AS AppliesQ ON ChargesT.ID = AppliesQ.DestID "
				"LEFT JOIN (SELECT ChargeID, Sum(Amount) AS OtherInsTotal FROM ChargeRespT WHERE InsuredPartyID = {INT} "
				"	GROUP BY ChargeID) AS OtherInsTotalQ ON ChargesT.ID = OtherInsTotalQ.ChargeID "
				"LEFT JOIN "
				"	(SELECT DestID, Sum(Coalesce(AppliesT.Amount,0)) AS AdjustmentTotal "
				"	FROM AppliesT "
				"	INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
				"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"	INNER JOIN (SELECT * FROM HCFA_EbillingAllowedAdjCodesT WHERE HCFASetupID = {INT}) AS HCFA_EbillingAllowedAdjCodesT ON PaymentsT.GroupCodeID = HCFA_EbillingAllowedAdjCodesT.GroupCodeID AND PaymentsT.ReasonCodeID = HCFA_EbillingAllowedAdjCodesT.ReasonCodeID "
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 2 "
				"	AND PaymentsT.GroupCodeID Is Not Null AND PaymentsT.ReasonCodeID Is Not Null "
				"	AND PaymentsT.InsuredPartyID = {INT} "
				"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"	GROUP BY DestID) "
				"	AS HCFA_AllowedAdjustmentsQ ON ChargesT.ID = HCFA_AllowedAdjustmentsQ.DestID "
				"LEFT JOIN "
				"	(SELECT DestID, Sum(Coalesce(AppliesT.Amount,0)) AS AdjustmentTotal "
				"	FROM AppliesT "
				"	INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
				"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"	INNER JOIN (SELECT * FROM UB_EbillingAllowedAdjCodesT WHERE UBSetupID = {INT}) AS UB_EbillingAllowedAdjCodesT ON PaymentsT.GroupCodeID = UB_EbillingAllowedAdjCodesT.GroupCodeID AND PaymentsT.ReasonCodeID = UB_EbillingAllowedAdjCodesT.ReasonCodeID "
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 2 "
				"	AND PaymentsT.GroupCodeID Is Not Null AND PaymentsT.ReasonCodeID Is Not Null "
				"	AND PaymentsT.InsuredPartyID = {INT} "
				"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"	GROUP BY DestID) "
				"	AS UB_AllowedAdjustmentsQ ON ChargesT.ID = UB_AllowedAdjustmentsQ.DestID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = {INT} AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"%s", strDocProv),
				oInfo.nInsuredPartyID, m_pEBillingInfo->InsuredPartyID,
				m_pEBillingInfo->HCFASetupID, oInfo.nInsuredPartyID,
				m_pEBillingInfo->UB92SetupID, oInfo.nInsuredPartyID,
				m_pEBillingInfo->BillID);

			//we have to calculate the amount per charge, and total it up, because
			//this has to match the individual charge amounts that aren't calculated until later
			while(!rsTotals->eof) {

				//track our individual amounts
				COleCurrency cyPaymentTotal = AdoFldCurrency(rsTotals, "PaymentTotal", COleCurrency(0,0));
				COleCurrency cyOtherInsTotal = AdoFldCurrency(rsTotals, "OtherInsTotal", COleCurrency(0,0));

				// (j.jones 2007-03-29 10:54) - PLID 25409 - now decide what value we should be sending

				COleCurrency cyPaymentAndOtherIns = cyPaymentTotal + cyOtherInsTotal;
				
				// (j.jones 2009-08-28 17:44) - PLID 32993 - we will always send the payment + other insurance here
				cyTotalAllowableToSend += cyPaymentAndOtherIns;

				// (j.jones 2010-02-03 09:32) - PLID 37170 - supported including specified adjustments in the allowed total,
				// these will already have been calculated in the recordset
				// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType == actInst) {
					cyTotalAllowableAdjustmentsToInclude += AdoFldCurrency(rsTotals, "UB_AllowedAdjustmentTotal", COleCurrency(0,0));
				}
				else {
					cyTotalAllowableAdjustmentsToInclude += AdoFldCurrency(rsTotals, "HCFA_AllowedAdjustmentTotal", COleCurrency(0,0));
				}

				rsTotals->MoveNext();
			}
			rsTotals->Close();

			// (j.jones 2009-03-11 10:35) - PLID 33446 - load the default ins. adj. group/reason codes
			CString strDefaultInsAdjGroupCode = "CO", strDefaultInsAdjReasonCode = "45";
			if(m_actClaimType == actInst) {
				strDefaultInsAdjGroupCode = m_UB92Info.ANSI_DefaultInsAdjGroupCode;
				strDefaultInsAdjReasonCode = m_UB92Info.ANSI_DefaultInsAdjReasonCode;
			}
			else {
				strDefaultInsAdjGroupCode = m_HCFAInfo.ANSI_DefaultInsAdjGroupCode;
				strDefaultInsAdjReasonCode = m_HCFAInfo.ANSI_DefaultInsAdjReasonCode;
			}

			// (j.jones 2008-04-30 10:02) - PLID 27946 - needs to filter on only batched charges
			//this cannot be parameterized
			// (j.jones 2009-03-11 10:31) - PLID 33446 - we now include all adjustments, and use the HCFA/UB group's
			// default for insurance adjustments if the group or reason code is blank
			// (j.jones 2010-09-23 15:48) - PLID 40653 - group & reason codes are now IDs
			// (j.jones 2011-08-16 17:22) - PLID 44805 - We will filter out "original" and "void" charges & applies.
			// The applies part of this does not need to check LineItemCorrectionsBalancingAdjT,
			// because they are only applied to original and void charges.
			_RecordsetPtr rsAdj = CreateRecordset("SELECT "
				"CASE WHEN AdjustmentGroupCodesT.Code Is Null THEN '%s' ELSE AdjustmentGroupCodesT.Code END AS GroupCode, "
				"CASE WHEN AdjustmentReasonCodesT.Code Is Null THEN '%s' ELSE AdjustmentReasonCodesT.Code END AS ReasonCode, "
				"AppliedQ.AdjAmount "
				"FROM PaymentsT "
				"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"INNER JOIN (SELECT Sum(Amount) AS AdjAmount, SourceID "
				"	FROM AppliesT WHERE DestID IN (SELECT ChargesT.ID FROM ChargesT "
				"		INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"		WHERE BillID = %li AND ChargesT.Batched = 1 AND LineItemT.Deleted = 0 "
				"		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"		%s) "
				"	GROUP BY SourceID) AS AppliedQ ON PaymentsT.ID = AppliedQ.SourceID "
				"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID "
				"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID "
				"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 2 "
				"AND PaymentsT.InsuredPartyID = %li "
				"ORDER BY AdjustmentGroupCodesT.Code ", _Q(strDefaultInsAdjGroupCode), _Q(strDefaultInsAdjReasonCode),
				m_pEBillingInfo->BillID, strDocProv, oInfo.nInsuredPartyID);

			OutputString = "";

			// (j.jones 2008-11-12 13:34) - PLID 31740 - track all the CAS segments we wish to add
			CArray<ANSI_CAS_Info*, ANSI_CAS_Info*> aryCASSegments;

			while(!rsAdj->eof) {

				// (j.jones 2006-11-21 10:33) - PLID 23415, 23652 - we can send up to 99
				// CAS segments, each segment has one group code and up to 6 adjustments
				// using that code

				// (j.jones 2008-11-12 13:25) - PLID 31740 - we now use AddCASSegmentToMemory and OutputCASSegments
				// to properly handle our CAS segments

				COleCurrency cyAmt = AdoFldCurrency(rsAdj, "AdjAmount", COleCurrency(0,0));

				AddCASSegmentToMemory(AdoFldString(rsAdj, "GroupCode",""),
					AdoFldString(rsAdj, "ReasonCode",""),
					cyAmt, aryCASSegments);

				//save to calculate extra adjustment
				cyAmtAdjusted += cyAmt;

				rsAdj->MoveNext();
			}
			rsAdj->Close();

			//if adjustments on this level, then we have to balance with the payments
			//the claim total in 2300, so use cyAmtAdjusted and cyPaid to determine
			//if more adjustments are needed.

			//cyClaimTotal was calculated earlier	
			COleCurrency cyDiff = cyClaimTotal - cyPaid - cyAmtAdjusted;
			if(cyDiff > COleCurrency(0,0)) {

				//we need to artificially adjust cyDiff

				// (j.jones 2008-05-23 09:59) - PLID 29084 - if an allowed amount was sent (B6),
				// then we need to split the adjustments such that the "other adjustment" (OA, or the default)
				// is the difference between the allowed amount and the claim total, and the 
				// "patient responsibility" (PR) is the difference between the paid amount and the allowed amount
				// This does NOT use the patient resp. in Practice.

				// (j.jones 2009-06-23 10:50) - PLID 34693 - This logic has changed slightly, but the above description
				// is still accurate. We previously calculated the OA to be the difference between the allowed and
				// the balance, and the PR was the difference between paid and allowed, if there was a remaining balance.
				// This was wrong. We need to FIRST calculate the PR, then the OA is whatever the remaining balance is.

				// (j.jones 2010-10-19 13:39) - PLID 32848 - in 5010, the allowed & approved segments are gone,
				// but we still use the allowed amount when calculating CAS*PR

				COleCurrency cyAllowedAmount = cyTotalAllowableToSend;				
				// (j.jones 2010-02-03 10:10) - PLID 37170 - the actual allowed amount that will be sent
				// can also include adjustments, but will not be used for the cyPatientAdj calculation
				COleCurrency cyAllowedWithIncludedAdjustments = cyTotalAllowableAdjustmentsToInclude;

				COleCurrency cyPatientAdj = COleCurrency(0,0), cyNonPatientAdj = COleCurrency(0,0);

				// (j.jones 2009-06-22 17:47) - PLID 34693 - the patient adj. should be the difference
				// between paid and allowed, ignoring what was adjusted
				//if((cyAllowedAmount - cyAmtPaid - cyAmtAdjusted) > COleCurrency(0,0)) {
				if((cyAllowedAmount - cyPaid) > COleCurrency(0,0)) {
					//the "patient resp" should be the difference between the amount paid,
					//and the allowable, if there is a difference at all
					//cyPatientAdj = (cyAllowedAmount - cyPaid - cyAmtAdjusted);
					cyPatientAdj = (cyAllowedAmount - cyPaid);
				}

				// (j.jones 2009-06-23 10:53) - PLID 34693 - the OA is now whatever the remaining balance is
				// after the PR is calculated
				if((cyDiff - cyPatientAdj) > COleCurrency(0,0)) {
					//the "other adjustment" balances out the charge, taking the patient adjustment into account
					cyNonPatientAdj = (cyDiff - cyPatientAdj);
				}

				ASSERT(cyClaimTotal == (cyPaid + cyAmtAdjusted + cyNonPatientAdj + cyPatientAdj));

				// (j.jones 2008-11-12 11:25) - PLID 31912 - do not send the PR segment unless the
				// setting tells us to do so
				// (j.jones 2009-08-28 16:44) - PLID 35006 - now we always send the PR segment if we
				// have a value to send, the bANSI_SendCASPR setting now simply controls whether we
				// send it even if the value is zero
				// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
				BOOL bAlwaysSendPRSegment = ((m_actClaimType != actInst && m_HCFAInfo.bANSI_SendCASPR) || (m_actClaimType == actInst && m_UB92Info.bANSI_SendCASPR));

				COleCurrency cyRegularAdjustmentAmountToSend = cyNonPatientAdj;
				COleCurrency cyPRAdjustmentAmountToSend = cyPatientAdj;

				// (j.jones 2009-08-28 16:46) - PLID 35006 - always send PR, or at least try to
				// (PR is not sent if a UB claim)
				if(m_actClaimType == actInst) {
					//if not sending PR, send the full adjustable value
					cyRegularAdjustmentAmountToSend += cyPatientAdj;
					cyPRAdjustmentAmountToSend = COleCurrency(0,0);
				}

				if(cyRegularAdjustmentAmountToSend > COleCurrency(0,0)) {

					// (j.jones 2007-06-15 11:46) - PLID 26309 - Now we have a setting that lets the use
					// configure a default group code.
					// (j.jones 2009-03-11 10:39) - PLID 33446 - We added a different setting for a default
					// for "real" adjustments, as opposed to this existing setting for "fake" adjustments.
					// This setting has been renamed to ANSI_DefaultRemAdjGroupCode/ReasonCode.
					//default to OA for other adjustment
					// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
					CString strGroupCode = "OA";
					if(m_actClaimType == actInst && m_UB92Info.ANSI_DefaultRemAdjGroupCode != "")
						strGroupCode = m_UB92Info.ANSI_DefaultRemAdjGroupCode;
					else if(m_actClaimType != actInst && m_HCFAInfo.ANSI_DefaultRemAdjGroupCode != "")
						strGroupCode = m_HCFAInfo.ANSI_DefaultRemAdjGroupCode;
					
					// (j.jones 2007-04-26 09:55) - PLID 25802 - Use 42 because it is the
					// answer to life, the universe, and everything... AND also the ideal and
					// most common reason code: "Charges exceed our fee schedule or maximum allowable amount."
					// (j.jones 2007-06-13 11:17) - PLID 26307 - You have to be kidding me. Right after
					// clearinghouses forced me to default to 42, it EXPIRED and is no longer a valid code.
					// Use 2, "Co-Insurance", though a pending PL item will make this configurable.
					// (j.jones 2007-06-15 11:46) - PLID 26309 - Now we have a setting that lets the user
					// configure a default.
					// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
					CString strReasonCode = "2";
					if(m_actClaimType == actInst && m_UB92Info.ANSI_DefaultRemAdjReasonCode != "")
						strReasonCode = m_UB92Info.ANSI_DefaultRemAdjReasonCode;
					else if(m_actClaimType != actInst && m_HCFAInfo.ANSI_DefaultRemAdjReasonCode != "")
						strReasonCode = m_HCFAInfo.ANSI_DefaultRemAdjReasonCode;

					// (j.jones 2008-11-12 13:25) - PLID 31740 - we now use AddCASSegmentToMemory and OutputCASSegments
					// to properly handle our CAS segments
					AddCASSegmentToMemory(strGroupCode, strReasonCode, cyRegularAdjustmentAmountToSend, aryCASSegments);
				}

				// (j.jones 2008-05-23 10:10) - PLID 29084 - now report the patient resp
				// (j.jones 2008-11-12 11:30) - PLID 31912 - only if bSendPRSegment is true
				// (j.jones 2009-08-28 16:46) - PLID 35006 - now we always send PR if we have
				// an amount to send, or send zero if bANSI_SendCASPR is TRUE
				// (PR is not sent if a UB claim)
				// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType != actInst && (bAlwaysSendPRSegment || cyPRAdjustmentAmountToSend > COleCurrency(0,0))) {

					// (j.jones 2008-11-12 13:25) - PLID 31740 - we now use AddCASSegmentToMemory and OutputCASSegments
					// to properly handle our CAS segments

					//default group code to PR for patient responsibility
					//default reason code to 2
					// (j.jones 2009-08-28 16:57) - PLID 35006 - sending zero is only allowed
					// if bAlwaysSendPRSegment is true
					AddCASSegmentToMemory("PR", "2", cyPRAdjustmentAmountToSend, aryCASSegments, bAlwaysSendPRSegment);
				}
			}

			//if cyDiff is less than zero, that would mean they have more applied than
			//the charge is for, which should be impossible

			// (j.jones 2008-11-12 13:30) - PLID 31740 - now output our CAS segments, if we have any
			ANSI_5010_OutputCASSegments(aryCASSegments);
		}

//////////////////////////////////////////////////////////////////////////////

//305		300		AMT		COB Payer Paid Amount					S		1

		// (j.jones 2006-11-24 15:16) - PLID 23415 - Use the payment information
		// if not sending to primary, and is using secondary sending.
		// When using the secondary stuff, it is REQUIRED to send payments here.

		// (j.jones 2006-11-27 17:07) - PLID 23652 - supported this on the UB92
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum

		if(!m_pEBillingInfo->IsPrimary &&
			((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1)
			|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1))) {
			
			OutputString = "AMT";

			//Ref.	Data		Name									Attributes
			//Des.	Element

			//AMT01	522			Amount Qualifier Code					M 1	ID	1/3

			//in 5010, this is D for both HCFA and UB claims

			//D - Payor Amount Paid
			str = "D";
			OutputString += ParseANSIField(str, 1, 3);

			//AMT02	782			Monetary Amount							M 1	R	1/18

			//we already have cyPaid
			str = FormatCurrencyForInterface(cyPaid, FALSE, FALSE);
			
			//see if we need to trim the zeros
			// (j.jones 2012-05-25 16:29) - PLID 50657 - now a modular function, truncates if m_bTruncateCurrency is enabled
			str = FormatANSICurrency(str);
			OutputString += ParseANSIField(str, 1, 18);

			//AMT03	NOT USED

			EndANSISegment(OutputString);
		}

//////////////////////////////////////////////////////////////////////////////

//306		300		AMT		COB Non-Covered Amount					S		1

		//Required when the destination payer’s cost avoidance policy allows
		//providers to bypass claim submission to the otherwise prior payer
		//identified in Loop 2330B.

		//When this segment is used, the amount reported in AMT02 must equal
		//the total claim charge amount reported in CLM02. Neither the prior
		//payer paid AMT, nor any CAS segments are used as this claim has not
		//been adjudicated by this payer.

		//AMT*A8 qualifier
		
		//we don't support this, I can't fathom a need to do so

//307		300		AMT		COB Remaining Patient Liability			S		1

		//Appears to be used as the remaining amount to be paid
		//if we don't have prior payment information or adjustment
		//information, seemingly the equivalent of CAS*PR*2 if we
		//didn't send other CAS segments. Sounds like a good idea
		//but I doubt it will be correctly supported by clearinghouses
		//anytime soon.
				
		//AMT*EAF qualifier

		//we don't support this, for now

//308		300		OI		Other Insurance Coverage Information	R		1

		//"Yeah Yeah, Oi Oi" - Infant Sorrow

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "OI";

		//any changes here should also change the CLM segment

		//OI01	NOT USED
		OutputString += "*";
		//OI02	NOT USED
		OutputString += "*";

		//OI03	1073		Yes/No Condition or Response Code		O 1	ID	1/1

		//Benefits Assignment Certification Indicator
		//like CLM08, but for this 'other' insurance company

		//this is the "Assignment of Benefits Indicator"
		//"indicates insured or authorized person authorizes benefits to be assigned to the provider".

		//This is normally "Y" if InsuranceAcceptedT.Accepted is true, but the bill now has an override
		//to optionally force this to be Yes or No.

		// (j.jones 2010-07-23 15:02) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsCo now
		BOOL bAccepted = GetAcceptAssignment_ByInsCo(nOtherInsCoID, m_pEBillingInfo->ANSI_RenderingProviderID);

		//follows the HCFA group setting for filling Box 13
		BOOL bFillBox13 = TRUE;
		if(nBox13Accepted == 0) {
			//never fill
			bFillBox13 = FALSE;
		}
		else if(nBox13Accepted == 1) {
			//fill if accepted
			bFillBox13 = bAccepted;
		}
		else if(nBox13Accepted == 2) {
			//fill if not accepted
			bFillBox13 = !bAccepted;
		}
		else if(nBox13Accepted == 3) {
			//always fill
			bFillBox13 = TRUE;
		}

		HCFABox13Over hb13Value = hb13_UseDefault;

		rs = rs->NextRecordset(NULL);

		if(!rs->eof) {
			hb13Value = (HCFABox13Over)AdoFldLong(rs, "HCFABox13Over", (long)hb13_UseDefault);
		}
		rs->Close();

		if(hb13Value == hb13_No || (hb13Value == hb13_UseDefault && !bFillBox13)) {
			str = "N";
		}
		else {
			str = "Y";
		}

		OutputString += ParseANSIField(str, 1, 1);

		//OI04	1351		Patient Signature Source Code			O 1	ID	1/1

		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType == actInst) {
			//not used on UBs
			OutputString += "*";
		}
		else {
			//like CLM10, but for this 'other' insurance company

			//Code indicating how the patient or subscriber authorization signatures were
			//obtained and how they are being retained by the provider

			//P	-	Signature generated by provider because the patient
			//		was not physically present for services

			//in 5010, we send nothing, because P makes no sense
			OutputString += ParseANSIField("",1,1);
		}

		//OI05	NOT USED
		OutputString += "*";

		//OI06	1363		Release of Information Code				O 1	ID	1/1

		//like CLM09, but for this 'other' insurance company
		
		//Code indicating whether the provider has on file a signed statement by the patient
		//authorizing the release of medical data to other organizations

		//I -	Informed Consent to Release Medical Information
		//Y -	Yes, Provider has a Signed Statement Permitting
		//		Release of Medical Billing Data Related to a Claim

		// (j.jones 2010-07-23 11:24) - PLID 39795 - we used to always send Y,
		// but now we will send N if Box 12 is not set to be filled in

		BOOL bFillBox12 = TRUE;
		if(nBox12Accepted == 0) {
			//never fill
			bFillBox12 = FALSE;
		}
		else if(nBox12Accepted == 1) {
			//fill if accepted
			bFillBox12 = bAccepted;
		}
		else if(nBox12Accepted == 2) {
			//fill if not accepted
			bFillBox12 = !bAccepted;
		}
		else if(nBox12Accepted == 3) {
			//always fill
			bFillBox12 = TRUE;
		}

		if(bFillBox12) {
			str = "Y";
		}
		else {
			str = "I";
		}
		OutputString += ParseANSIField(str,1,1);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//MIA is UB only
//369		300		MIA		Outpatient Adjudication Information		S		1
		//not supported

//MOA is UB and HCFA
//310		300		MOA		Outpatient Adjudication Information		S		1
		//not supported

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2320");

	return Error_Other;
}

// (j.jones 2012-03-21 14:18) - PLID 48870 - this now requires an OtherInsuranceInfo struct
int CEbilling::ANSI_5010_2330A(OtherInsuranceInfo oInfo) {

	//Other Subscriber Name
	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2330A\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		// (j.jones 2012-11-12 10:20) - PLID 53693 - added CountryCode
		rs = CreateParamRecordset("SELECT PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, PersonT.Address1, PersonT.Address2, "
					"PersonT.City, PersonT.State, PersonT.Zip, InsuredPartyT.IDForInsurance, InsuredPartyT.PolicyGroupNum, PersonT.SocialSecurity, "
					"CountriesT.ISOCode AS CountryCode "
					"FROM InsuredPartyT "
					"INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
					"LEFT JOIN CountriesT ON PersonT.Country = CountriesT.CountryName "
					"WHERE InsuredPartyT.PersonID = {INT}", oInfo.nInsuredPartyID);

		if(rs->eof) {
			//if the recordset is empty, there is no secondary insurance. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open secondary insurance information from this patient's bill for '%s', Bill ID %li. (ANSI_2330A)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open secondary insurance information from this patient's bill. (ANSI_2330A)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2330A - OTHER SUBSCRIBER NAME							1

//313		325		NM1		Other Subscriber Name					R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M 1	ID	2/3

		//static value "IL"

		OutputString += ParseANSIField("IL", 2, 3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//static value "1" - person, "2" - non person

		OutputString += ParseANSIField("1", 1, 1);

		//NM103	1035		Name Last / Org Name					O 1	AN	1/60

		str = GetFieldFromRecordset(rs, "Last");		//PersonT.Last

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 60);

		//NM104	1036		Name First								O 1	AN	1/35

		str = GetFieldFromRecordset(rs, "First");		//PersonT.First

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 35);

		//NM105	1037		Name Middle								O 1	AN	1/25

		str = GetFieldFromRecordset(rs, "Middle");

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 25);

		//NM106	NOT USED

		OutputString += "*";

		//NM107	1039		Name Suffix								O 1	AN	1/10

		// (j.jones 2007-08-03 11:44) - PLID 26934 - changed from Suffix to Title
		str = GetFieldFromRecordset(rs, "Title");

		OutputString += ParseANSIField(str, 1, 10);

		//NM108	66			ID Code Qualifier						X 1	ID	1/2

		//II - Standard Unique Health Identifier for each Individual in the United States
		//MI - Member Identification Number

		//we use MI
		str = GetFieldFromRecordset(rs,"IDForInsurance");
		str = StripPunctuation(str);
		//do not output if the ID is blank
		if(str != "")
			OutputString += ParseANSIField("MI",1,2);
		else
			OutputString += "*";

		//NM109	67			ID Code									X 1	AN	2/80

		//InsuredPartyT.IDForInsurance
		
		//do not output if the ID is blank
		if(str != "")
			OutputString += ParseANSIField(str,2,80);
		else
			OutputString += "*";

		//NM110	NOT USED

		OutputString += "*";

		//NM111	NOT USED

		OutputString += "*";

		//NM112	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

//316		332		N3		Other Subscriber Address				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//N301	166			Address Information						M 1	AN	1/55

		str = GetFieldFromRecordset(rs, "Address1");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information						O 1	AN	1/55

		str = GetFieldFromRecordset(rs, "Address2");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

//317		340		N4		Other Subscriber City/State/Zip			S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O 1	AN	2/30

		str = GetFieldFromRecordset(rs, "City");

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State or Province						O	ID	2/2

		str = GetFieldFromRecordset(rs, "State");

		// (j.jones 2012-01-11 10:48) - PLID 47461 - trim and auto-capitalize this
		str.TrimLeft(); str.TrimRight();
		str.MakeUpper();
		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O	ID	3/15

		str = GetFieldFromRecordset(rs, "Zip");
		// (j.jones 2013-05-06 17:48) - PLID 55100 - don't strip all numbers, just spaces and hyphens
		str = StripSpacesAndHyphens(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O 1	ID	2/3

		//this is only required if the addess is outside the US
		
		// (j.jones 2012-11-12 10:07) - PLID 53693 - use the insured party's country code, if not US
		CString strCountryCode = GetFieldFromRecordset(rs, "CountryCode");
		if(strCountryCode.CompareNoCase("US") == 0) {
			//leave blank
			strCountryCode = "";
		}
		else {
			//our ISO codes should already be uppercase, force it just incase
			strCountryCode.MakeUpper();
		}
		OutputString += ParseANSIField(strCountryCode, 2, 3);

		//N405	NOT USED
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		//N407	1715		Country Subdivision Code				X 1	ID	1/3

		//an even more detailed optional value for country, since we
		//do not support countries, we don't support this either
		
		OutputString += "*";

		EndANSISegment(OutputString);

//319		355		REF		Other Subscriber Secondary Info.		S		3

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01	128			Reference Ident Qual					M 1	ID	2/3

		// (j.jones 2008-04-01 16:42) - PLID 29486 - added ability to suppress this field
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		// (j.jones 2012-01-18 15:02) - PLID 47627 - This now only applies when the insurance
		// company in the group is the one sent in this loop, which in this case is NOT the
		// group loaded in m_HCFAInfo/m_UB92Info. OtherANSI_Hide2330AREF is now filled from
		// the "other" company's HCFA group when HCFA, and the UB group when UB, so no form
		// filter is needed here.
		if(oInfo.nANSI_Hide2330AREF == 0) {

			OutputString = "REF";

			//REF01	128			Reference Ident Qual					M 1	ID	2/3

			//SY - SSN
			OutputString += ParseANSIField("SY", 2, 3);

			//REF02	127			Reference Ident							X 1	AN	1/50

			//InsuredPartyT.SocialSecurity
			str = AdoFldString(rs, "SocialSecurity", "");

			//no punctuation of any sort is allowed
			// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens,
			// although SSN should always be numeric
			// (j.jones 2016-03-01 09:56) - PLID 68430 - reverted to StripNonNum to get rid of ### signs,
			// you can't type letters into SSN in our system
			str = StripNonNum(str);

			OutputString += ParseANSIField(str, 1, 50);

			//REF03	NOT USED

			OutputString += "*";

			//REF04	NOT USED

			OutputString += "*";

			//if these is no number, don't output
			if(str != "") {
				EndANSISegment(OutputString);
			}
		}

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2330A");
///////////////////////////////////////////////////////////////////////////////

	return Error_Other;
}

// (j.jones 2012-03-21 14:18) - PLID 48870 - this now requires an OtherInsuranceInfo struct
int CEbilling::ANSI_5010_2330B(OtherInsuranceInfo oInfo) {

	//Other Payer Name
	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2330B\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;

		// (j.jones 2007-09-18 17:48) - PLID 27428 - added CrossoverCode
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		rs = CreateParamRecordset("SELECT InsuranceCoT.Name, InsuranceCoT.CrossoverCode, ContactPersonT.Last + ', ' + ContactPersonT.First  AS ContactName, ContactPersonT.WorkPhone, "
			"ContactPersonT.Fax, InsuranceCoT.InsType, InsuranceCoT.EBillingClaimOffice, "
			"InsCoPersonT.Address1, InsCoPersonT.Address2, InsCoPersonT.City, InsCoPersonT.State, InsCoPersonT.Zip "
			"FROM InsuredPartyT LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN InsuranceContactsT ON InsuredPartyT.InsuranceContactID = InsuranceContactsT.PersonID "
			"LEFT JOIN PersonT ContactPersonT ON InsuranceContactsT.PersonID = ContactPersonT.ID "
			"LEFT JOIN PersonT InsCoPersonT ON InsuranceCoT.PersonID = InsCoPersonT.ID "
			"WHERE InsuredPartyT.PersonID = {INT}", oInfo.nInsuredPartyID);

		if(rs->eof) {
			//if the recordset is empty, there is no secondary insurance. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open secondary insurance information from this patient's bill for '%s', Bill ID %li. (ANSI_2330B)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open secondary insurance information from this patient's bill. (ANSI_2330B)";

			AfxMessageBox(str);
			return Error_Missing_Info;
		}

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2330B - OTHER PAYER NAME								1

//320		325		NM1		Other Payer Name						R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M 1	ID	2/3

		//static value "PR"

		OutputString += ParseANSIField("PR",2,3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//static value "2"
		OutputString += ParseANSIField("2",1,1);

		//NM103	1035		Name Last / Org Name					O 1	AN	1/60

		str = GetFieldFromRecordset(rs, "Name");			//InsuranceCoT.Name

		OutputString += ParseANSIField(str, 1, 60);

		//NM104	NOT USED

		OutputString += "*";

		//NM105	NOT USED

		OutputString += "*";

		//NM106	NOT USED

		OutputString += "*";

		//NM107	NOT USED

		OutputString += "*";

		//NM108	66			ID Code Qualifier						X 1	ID	1/2

		// (j.jones 2007-09-18 17:49) - PLID 27428 - if CrossoverCode is non-empty, send the CrossoverCode,
		// otherwise send the payer ID.
		
		CString strCrossoverCode = GetFieldFromRecordset(rs,"CrossoverCode");

		strCrossoverCode.TrimLeft();
		strCrossoverCode.TrimRight();

		if(!strCrossoverCode.IsEmpty()) {
			//non-empty, so use the crossover code
			str = strCrossoverCode;
		}
		else {
			//crossover was empty, so send the payer ID
			// (j.jones 2009-08-05 10:20) - PLID 34467 - this is now in the loaded claim info
			// (j.jones 2009-12-16 15:54) - PLID 36237 - split out into HCFA and UB payer IDs
			// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
			if(m_actClaimType == actInst) {
				str = oInfo.strUBPayerID;
			}
			else {
				str = oInfo.strHCFAPayerID;
			}

			// (b.spivey, May 07, 2013) - PLID 46573 - trimspaces, that's the same as being empty. 
			str = str.Trim(" "); 

			// (j.jones 2008-05-06 11:44) - PLID 29937 - cached m_bUseTHINPayerIDs and m_bPrependTHINPayerNSF
			// (j.jones 2009-08-04 11:34) - PLID 14573 - renamed to PrependTHINPayerNSF to PrependPayerNSF,
			// and removed m_bUseTHINPayerIDs

			//see if we need to prepend the NSF code
			if(m_bPrependPayerNSF) {
			
				// (j.jones 2008-09-09 10:18) - PLID 18695 - We converted the NSF Code to InsType,
				// but this code actually needs ye olde NSF Code. I am pretty sure this is obsolete
				// because THIN no longer exists, but for the near term let's calculate ye olde NSF code.

				InsuranceTypeCode eInsType = (InsuranceTypeCode)AdoFldLong(rs, "InsType", (long)itcInvalid);
				CString strNSF = GetNSFCodeFromInsuranceType(eInsType);
				if(strNSF.IsEmpty()) {
					strNSF = "Z";
				}
				
				str = strNSF + str;
			}
		}

		// (j.jones 2010-08-30 17:12) - PLID 15025 - supported TPL Code,
		// we already loaded the HCFA or UB-specific setting into nSendTPLCode,
		// we need to override the payer ID only if configured to do so AND the TPL
		// is non-empty
		//1 - do not send
		//2 - send both
		//3 - send 2330B only
		//4 - send 2430 only
		if(m_pEBillingInfo->nSendTPLCode == 2 || m_pEBillingInfo->nSendTPLCode == 3) {
			CString strTPLCode = oInfo.strTPLCode;
			strTPLCode.TrimLeft();
			strTPLCode.TrimRight();
			if(!strTPLCode.IsEmpty()) {
				str = strTPLCode;
			}
		}

		//PI - Payor Identification
		//XV - Centers for Medicare and Medicaid Services PlanID

		//we use PI

		//if no ID, don't output
		if(str != "")
			OutputString += ParseANSIField("PI",1,2);
		else
			OutputString += "*";

		//NM109	67			ID Code									X 1	AN	2/80

		//Either the payer ID or InsuranceCoT.CrossoverCode
		if(str != "")
			OutputString += ParseANSIField(str,2,80);
		else
			OutputString += "*";

		//NM110	NOT USED

		OutputString += "*";

		//NM111	NOT USED

		OutputString += "*";

		//NM112	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

/////////////////////////////////////////////////////////////////////////////////////////

//322		332		N3		Other Payer Address				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//N301	166			Address Information						M 1	AN	1/55

		str = GetFieldFromRecordset(rs, "Address1");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information						O 1	AN	1/55

		str = GetFieldFromRecordset(rs, "Address2");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

//323		340		N4		Other Payer City/State/Zip				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O 1	AN	2/30

		str = GetFieldFromRecordset(rs, "City");

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State or Province						O 1	ID	2/2

		str = GetFieldFromRecordset(rs, "State");

		// (j.jones 2012-01-11 10:48) - PLID 47461 - trim and auto-capitalize this
		str.TrimLeft(); str.TrimRight();
		str.MakeUpper();
		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O 1	ID	3/15

		str = GetFieldFromRecordset(rs, "Zip");
		// (j.jones 2013-05-06 17:48) - PLID 55100 - don't strip all numbers, just spaces and hyphens
		str = StripSpacesAndHyphens(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O 1	ID	2/3

		//this is only required if the addess is outside the US
		//as of right now, we can't support it
		
		OutputString += "*";

		//N405	NOT USED
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		//N407	1715		Country Subdivision Code				X 1	ID	1/3

		//an even more detailed optional value for country, since we
		//do not support countries, we don't support this either
		
		OutputString += "*";

		EndANSISegment(OutputString);

/////////////////////////////////////////////////////////////////////////////////

//325		345		DTP		Claim Check Or Remittance Date			S		1
		
		// (j.jones 2006-11-27 10:27) - PLID 23415 - use the date information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending adjustment information in the 2320 loop

		// (j.jones 2006-11-27 17:17) - PLID 23652 - supported this on the UB92 
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(!m_pEBillingInfo->IsPrimary &&
			((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1 && m_HCFAInfo.ANSI_AdjustmentLoop == 0)
			|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1 && m_UB92Info.ANSI_AdjustmentLoop == 0))) {

			// (j.jones 2006-11-27 10:27) - PLID 23415, 23652 - output the date the claim was paid by insurance

			//since we are sending payments and adjustments in the 2320 loop,
			//just find the first payment or adjustment date (in theory, should all be the same date)
			// (j.jones 2008-04-30 10:03) - PLID 27946 - needs to filter on batched charges
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			// (j.jones 2011-08-16 17:22) - PLID 44805 - We will filter out "original" and "void" charges & applies.
			// The applies part of this does not need to check LineItemCorrectionsBalancingAdjT,
			// because they are only applied to original and void charges.
			_RecordsetPtr rsDates = CreateParamRecordset("SELECT TOP 1 LineItemT.Date "
				"FROM PaymentsT "
				"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "	
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND LineItemT.Type IN (1,2) "
				"AND PaymentsT.ID IN (SELECT SourceID FROM AppliesT WHERE DestID IN "
				"	(SELECT ChargesT.ID FROM ChargesT "
				"		INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"		WHERE BillID = {INT} AND ChargesT.Batched = 1 AND LineItemT.Deleted = 0 "
				"		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null)) "
				"AND PaymentsT.InsuredPartyID <> -1 "
				"AND PaymentsT.InsuredPartyID = {INT} "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"GROUP BY LineItemT.Date " //unique dates only
				"ORDER BY LineItemT.Date ",	
				m_pEBillingInfo->BillID, oInfo.nInsuredPartyID);

			if(!rsDates->eof) {

				//Ref.	Data		Name									Attributes
				//Des.	Element

				OutputString = "DTP";

				//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

				//573 - Date Claim Paid

				str = "573";
				OutputString += ParseANSIField(str, 3, 3);

				//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

				//D8 - date is CCYYMMDD

				str = "D8";
				OutputString += ParseANSIField(str, 2, 3);

				//DTP03	1251		Date Time Period						M 1	AN	1/35

				//LineItemT.Date

				COleDateTime dt = AdoFldDateTime(rsDates, "Date");
				str = dt.Format("%Y%m%d");
				OutputString += ParseANSIField(str, 1, 35);

				EndANSISegment(OutputString);
			}
			rsDates->Close();
		}

//326		355		REF		Other Payer Secondary Identifier		S		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01	128			Reference Ident Qual					M 1	ID	2/3

		//2U - Payer Identification Number
		//EI - Employer’s Identification Number
		//FY - Claim Office Number
		//NF - National Association of Insurance Commissioners (NAIC) Code

		//we use FY
		OutputString += ParseANSIField("FY",2,3);

		//REF02	127			Reference Ident							X 1	AN	1/50

		//InsuranceCoT.EBillingClaimOffice
		str = GetFieldFromRecordset(rs,"EBillingClaimOffice");
		OutputString += ParseANSIField(str,1,50);

		//REF03	NOT USED

		OutputString += "*";

		//REF04	NOT USED

		OutputString += "*";

		if(str != "")
			EndANSISegment(OutputString);

//328		355		REF		Other Payer Prior Auth Num			S		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//we do not store a prior auth. number for a secondary payer,
		//so we don't need to export this segment

		/*
		OutputString = "REF";

		//REF01	128			Reference Ident Qual					M 1	ID	2/3

		//REF02	127			Reference Ident							X 1	AN	1/50

		//REF03	NOT USED

		OutputString += "*";

		//REF04	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);
		*/

//329		355		REF		Other Payer Referral Num			S		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//we do not store a referral number for a secondary payer,
		//so we don't need to export this segment

		/*
		OutputString = "REF";

		//REF01	128			Reference Ident Qual					M 1	ID	2/3

		//REF02	127			Reference Ident							X 1	AN	1/50

		//REF03	NOT USED

		OutputString += "*";

		//REF04	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);
		*/

//330		355		REF		Other Payer Claim Adjustment Indicator	S		2

		//not used

//331		355		REF		Other Payer Claim Control Number		S		2

		// (j.jones 2013-06-20 12:23) - PLID 57245 - Supported sending the original reference
		// number in 2330B (using the same logic as 2300 that may also use the Medicaid Resubmission code),
		// only if we're sending non-primary in 2000B and that non-primary company's settings say
		// to send in 2330B (OrigRefNo_2330B == 1).
		//
		// The cached OriginalReferenceNumber field is almost always BillsT.OriginalRefNo, but if they fill
		// out BillsT.MedicaidResubmission instead, we'll use that. In the event they fill out both (which
		// they should not be doing), we use MedicaidResubmission instead of OriginalRefNo.
		if(!m_pEBillingInfo->IsPrimary &&
			((m_actClaimType != actInst && m_HCFAInfo.OrigRefNo_2330B == 1) || (m_actClaimType == actInst && m_UB92Info.OrigRefNo_2330B == 1))
			&& m_pEBillingInfo->strOriginalReferenceNumber.GetLength() > 0) {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "REF";

			//REF01 128			Reference Identification Qualifier		M 1	ID	2/3
			//F8 - Original Reference Number (ICN/DCN)
			OutputString += ParseANSIField("F8", 2, 3);
			
			//REF02 127			Reference Identification				X 1	AN	1/50
			OutputString += ParseANSIField(m_pEBillingInfo->strOriginalReferenceNumber, 1, 50);

			//REF03 NOT USED
			//REF04 NOT USED
			
			// already checked for length of strMedicaidResubmission above
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2330B");

	return Error_Other;
}

/*	All this other payer stuff is ridiculous. We will not use this.
Each loop is only used if the patient/provider/location etc. is different
for the different payer.

int CEbilling::ANSI_5010_2330C(long nInsuredPartyID) {

	//Other Payer Referring Provider

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2330C - OTHER PAYER REFERRING PROVIDER				2

//332		325		NM1		Other Payer Referring Provider			S		1
//334		355		REF		Other Payer Referring Provider Ident.	R		3

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2330C");

	return Error_Other;
}

int CEbilling::ANSI_5010_2330D(long nInsuredPartyID) {

	//Other Payer Rendering Provider

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2330D - OTHER PAYER RENDERING PROVIDER				1

//336		325		NM1		Other Payer Rendering Provider			S		1
//338		355		REF		Othr Payer Rend. Prov. Secondary Ident.	R		3

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2330D");

	return Error_Other;
}

int CEbilling::ANSI_5010_2330E(long nInsuredPartyID) {

	//Other Payer Service Facility Location

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2330E - OTHER PAYER SERVICE FACILITY LOCATION			1

//340		325		NM1		Other Payer Service Facility Location	S		1
//342		355		REF		Other Payer Serv. Facility Loc. Ident.	R		3

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2330E");

	return Error_Other;
}

int CEbilling::ANSI_5010_2330F(long nInsuredPartyID) {

	//Other Payer Supervising Provider

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2330F - OTHER PAYER SUPERVISING PROVIDER				1

//343		325		NM1		Other Payer Supervising Provider		S		1
//345		355		REF		Other Payer Supervising Prov. Ident.	R		3

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2330F");

	return Error_Other;
}

int CEbilling::ANSI_5010_2330G(long nInsuredPartyID) {

	//Other Payer Billing Provider

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2330G - OTHER PAYER BILLING PROVIDER				1

//347		325		NM1		Other Payer Billing Provider		S		1
//349		355		REF		Other Payer Billing Prov. Ident.	R		3

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2330G");

	return Error_Other;
}

*/

// (j.jones 2008-05-23 10:48) - PLID 29084 - added parameters for allowables
// (j.jones 2011-04-20 10:45) - PLID 41490 - we now pass in info. on skipping diagnosis codes
int CEbilling::ANSI_5010_2400_Prof(ADODB::_RecordsetPtr &rsCharges, BOOL &bSentAllowedAmount, COleCurrency &cyAllowableSent)
{

	//Service Line
	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2400\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	//increment the count for the current service line
	m_ANSIServiceCount++;

	FieldsPtr fields = rsCharges->Fields;

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2400 - SERVICE LINE									50

//350		365		LX		Service Line							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "LX";

		//LX01	554			Assigned Number							M 1	N0	1/6

		//m_ANSIServiceCount
		str.Format("%li",m_ANSIServiceCount);
		OutputString += ParseANSIField(str,1,6);

		EndANSISegment(OutputString);

//351		370		SV1		Professional Service					R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "SV1";

		//SV101	C003		Composite Medical Procedure Identifier	M 1

		//SV101-1	235		Product/Service ID Qualifier			M	ID	2/2

		//ER - Jurisdiction Specific Procedure and Supply Codes
		//HC - Health Care Financing Administration Common Procedural Coding System (HCPCS) Codes
		//IV - Home Infusion EDI Coalition (HIEC) Product/Service Code
		//WK - Advanced Billing Concepts (ABC) Codes

		//we use HC
		CString strQual = "HC";

		// (j.jones 2013-01-25 12:25) - PLID 54853 - IsValidServiceCodeQualifier
		// needs updated any time we support exporting a new qualifier type,
		// currently we only support HC. This ensures that we never send other 
		// qualifiers without updating this function.
		if(!IsValidServiceCodeQualifier(strQual)) {
			//You just tried to send a qualifier not supported by IsValidServiceCodeQualifier,
			//Go into that function and add your new qualifier to the supported list.
			ASSERT(FALSE);
			ThrowNxException("Loop 2400 tried to send invalid service qualifier %s.", strQual);			
		}

		OutputString += ParseANSIField(strQual,2,2);

		//SV101-2	234		Product/Service ID						M	AN	1/48

		//ChargesT.ItemCode
		str = GetFieldFromRecordset(rsCharges,"ItemCode");
		OutputString += ":";
		OutputString += str;

		//SV101-3	1339	Procedure Modifier						O	AN	2/2

		//ChargesT.CPTModifier
		str = GetFieldFromRecordset(rsCharges,"CPTModifier");
		OutputString += ":";
		OutputString += str;

		//SV101-4	1339	Procedure Modifier						O	AN	2/2
		
		//ChargesT.CPTModifier2
		str = GetFieldFromRecordset(rsCharges,"CPTModifier2");
		OutputString += ":";
		OutputString += str;

		//SV101-5	1339	Procedure Modifier						O	AN	2/2

		//ChargesT.CPTModifier3
		str = GetFieldFromRecordset(rsCharges,"CPTModifier3");
		OutputString += ":";
		OutputString += str;

		//SV101-6	1339	Procedure Modifier						O	AN	2/2
			
		//ChargesT.CPTModifier4
		str = GetFieldFromRecordset(rsCharges,"CPTModifier4");
		OutputString += ":";
		OutputString += str;

		//SV101-7	352		Description								O	AN	1/80

		//A free-form description to clarify the related data elements and their content
		
		//Required when, in the judgment of the submitter, the Procedure Code does not definitively
		//describe the service/product/supply and loop 2410 is not used.

		// (j.jones 2012-01-04 08:39) - PLID 47277 - this is only used when the CPT code is a "not otherwise classified" code,
		//which is qualified by having one of the following keywords in the CPT code's description:
		//- Not Otherwise Classified
		//- Not Otherwise
		//- Unlisted
		//- Not Listed
		//- Unspecified
		//- Unclassified
		//- Not Otherwise Specified
		//- Non-specified
		//- Not Elsewhere Specified
		//- Not Elsewhere
		//- Nos (Note: Includes "nos", "nos;", "nos,")
		//- Noc (Note: Includes "noc", "noc;", "noc,")

		//NOCCode is TRUE if the charge is a CPT code and has one of the above keywords in the CPT description
		//(NOT the charge description), but CPTCodeT.NOCType, which defaults to NULL, can be overridden to be TRUE/FALSE
		//to force a decision on that CPT code and ignore the keywords.
		BOOL bNOCCode = VarBool(rsCharges->Fields->Item["NOCCode"]->Value);
		if(bNOCCode) {
			//if an NOCCode, send the LineNote here, and not in NTE
			str = AdoFldString(rsCharges, "LineNote", "");
			str.Replace("\r\n"," ");
			//incase a \r or \n is on its own, remove it
			str.Replace("\r","");
			str.Replace("\n","");
			str.TrimLeft();
			str.TrimRight();
			OutputString += ":";
			OutputString += str;
		}

		OutputString.TrimRight(":");

		//SV101-8 NOT USED

		//SV102	782			Monetary Amount							O 1	R	1/18

		//LineTotal is the calculated amount * qty * tax * modifiers
		COleCurrency cyLineTotal = VarCurrency(fields->Item["LineTotal"]->Value, COleCurrency(0,0));
		str = FormatCurrencyForInterface(cyLineTotal,FALSE,FALSE);

		//see if we need to trim the zeros
		// (j.jones 2012-05-25 16:29) - PLID 50657 - now a modular function, truncates if m_bTruncateCurrency is enabled
		str = FormatANSICurrency(str);

		OutputString += ParseANSIField(str,1,18);

		//SV103	355			Unit or Basis for Measurement Code		X 1	ID	2/2

		//UN - Unit
		//MJ - Minutes
		//check ServiceT.Anesthesia. If it is true, then the quantity is minutes, not units
		str = "UN";
		if(VarBool(fields->Item["Anesthesia"]->Value, FALSE))
			str = "MJ";
		OutputString += ParseANSIField(str,2,2);

		//SV104	380			Quantity								X 1	R	1/15

		//ChargesT.Quantity
		var = fields->Item["Quantity"]->Value;
		if(var.vt == VT_R8)
			str.Format("%g",var.dblVal);
		else
			str = "1";

		// (j.jones 2007-10-15 15:05) - PLID 27757 - track the ServiceID
		long nServiceID = VarLong(fields->Item["ServiceID"]->Value);

		// (j.jones 2008-05-02 10:50) - PLID 28012 - removed the requirement for UseAnesthesiaBilling
			// to be checked for the service, and for a flat-fee
		if(m_HCFAInfo.ANSI_UseAnesthMinutesAsQty == 1 && VarBool(fields->Item["Anesthesia"]->Value, FALSE)
			/*&& VarBool(fields->Item["UseAnesthesiaBilling"]->Value, FALSE)
			// (j.jones 2007-10-15 14:57) - PLID 27757 - converted to use the new structure
			&& ReturnsRecords("SELECT ID FROM AnesthesiaSetupT WHERE ServiceID = %li AND LocationID = %li AND AnesthesiaFeeBillType <> 1", nServiceID, m_pEBillingInfo->POS)*/) {

			//they want to show minutes, and this is indeed an anesthesia charge that is based on time
			str.Format("%li",VarLong(fields->Item["AnesthesiaMinutes"]->Value, 0));
		}

		OutputString += ParseANSIField(str,1,15);

		//SV105	1331		Facility Code							O 1	AN	1/2
			//only needed if different from the claim as a whole, which is
			//not possible in our software, so we won't use this

		OutputString += "*";

		//SV106	NOT USED
		
		/* JJ - 04/22/2003

		//One has to just love incompatibility. Regence Blue Shield in WA needs SV106
		//used, which is the Type Of Service code. They only need it up until 10/16/2003.

		*/

		// (j.jones 2008-05-06 11:45) - PLID 29937 - cached UseSV106
		if(m_bUseSV106) {

			//MORE INCORRECT IMPLEMENTATION
			
			//ChargesT.ServiceType
			var = fields->Item["ServiceType"]->Value;
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
			OutputString += ParseANSIField(str,1,2);

			//END INCORRECT IMPLEMENTATION
		}
		else
			//CORRECT IMPLEMENTATION
			OutputString += "*";

		//SV107	C004		Composite Diagnosis Code Pointer		O 1

		//SV107-1	1328	Diagnosis Code Pointer					M	N0	1/2
		//SV107-1	1328	Diagnosis Code Pointer					O	N0	1/2
		//SV107-1	1328	Diagnosis Code Pointer					O	N0	1/2
		//SV107-1	1328	Diagnosis Code Pointer					O	N0	1/2

		//ChargesT.WhichCodes
		//the format in which we store "WhichCodes" will fill in all 4
		//fields appropriately, once the commas are replaced with colons
		
		// (j.jones 2009-03-25 17:35) - PLID 33669 - this should not need to change to support
		// up to 8 codes, because the bill saving should not have allowed more than 4 pointers
		// (d.singleton 2014-03-05 16:07) - PLID 61195 - update ANSI5010 claim export to support ICD10 and new diagcode billing structure
		var = fields->Item["ChargeWhichCodes"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		str.Replace(",",":");

		OutputString += "*";
		OutputString += str;

		//SV108	NOT USED
		OutputString += "*";

//406	//SV109	1073		Yes/No Condition or Response Code		O 1	ID	1/1

		//in 5010, N is not a valid value, it is Y or blank

		// (j.jones 2008-11-13 15:22) - PLID 31980 - We need to support the Box24C value here.
		//Per the ANSI specs:
		//SV109 is the emergency-related indicator; a “Y” value indicates service
		//provided was emergency related

		//since this is optional, and the 24C default is blank, we will send nothing
		//unless the 24C setting is used, in which case we can send send only T
		str = "";

		// (j.jones 2010-04-08 13:46) - PLID 38095 - ChargesT.IsEmergency determines whether
		// we send the setting on this charge, or use the HCFA Setup value
		ChargeIsEmergencyType eIsEmergency = (ChargeIsEmergencyType)VarLong(fields->Item["IsEmergency"]->Value);
		if(eIsEmergency == cietYes || (eIsEmergency == cietUseDefault && m_HCFAInfo.Box24C == 1)) {
			str = "Y";
		}
		//blank in all other cases
		else {
			str = "";
		}
		OutputString += ParseANSIField(str,1,1);

		//SV110	NOT USED
		OutputString += "*";

		//SV111	1073		Yes/No Condition or Response Code		O 1	ID	1/1
				//this is for EPSDT, which we don't use
		OutputString += "*";

		
		//SV112	1073		Yes/No Condition or Response Code		O 1	ID	1/1
				//this is for family planning, which we don't use
		OutputString += "*";

		//SV113	NOT USED
		OutputString += "*";
		//SV114	NOT USED
		OutputString += "*";

		//SV115	1327		Copay Status Code						O 1	ID	1/1
				//this is only if a patient was exempt from co-pay, which we don't use
		OutputString += "*";

		//SV116	NOT USED
		OutputString += "*";
		//SV117	NOT USED
		OutputString += "*";
		//SV118	NOT USED
		OutputString += "*";
		//SV119	NOT USED
		OutputString += "*";
		//SV120	NOT USED
		OutputString += "*";
		//SV121	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

//359		385		SV%		Durable Medical Equipment Service		S		1
//362		420		PWK		Line Supplemental Information			S		1
//366		420		PWK		Durable Medical Equipment Indicator		S		1
//368		425		CR1		Ambulance Transport Information			S		1
//371		435		CR3		Durable Medical Equipment Certification	S		1
//373		450		CRC		Ambulance Certification					S		3
//376		450		CRC		Hospice Employee Indicator				S		1
//378		450		CRC		Condition Indicator/Durable Medical Equipment		S		2

		//not used

//380		455		DTP		Date - Service Date						R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M 1	ID	3/3
		
		//static "472"
		OutputString += ParseANSIField("472",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

		// (j.jones 2007-04-30 12:35) - PLID 25848 - if ServiceDateTo
		// and ServiceDateFrom are the same, send D8 and ServiceDateFrom,
		// otherwise send RD8 and ServiceDateFrom-ServiceDateTo

		CString strDateQual = "D8";
		CString strDate;

		COleDateTime dtFrom = VarDateTime(fields->Item["ServiceDateFrom"]->Value);
		COleDateTime dtTo = VarDateTime(fields->Item["ServiceDateTo"]->Value);

		if(dtFrom == dtTo) {
			strDateQual = "D8";
			strDate = dtFrom.Format("%Y%m%d");
		}
		else {
			strDateQual = "RD8";
			strDate.Format("%s-%s",dtFrom.Format("%Y%m%d"), dtTo.Format("%Y%m%d"));
		}
		
		OutputString += ParseANSIField(strDateQual,2,3);

		//DTP03	1251		Date Time Period						M 1	AN	1/35

		// (j.jones 2012-10-30 16:49) - PLID 53364 - do not strip punctuation here, RD8 dates can have hyphens
		OutputString += ParseANSIField(strDate, 1, 35, FALSE, 'L', ' ', TRUE);

		EndANSISegment(OutputString);

//382		455		DTP		Date - Prescription Date				S		1
//383		455		DTP		Date - Certification Revision Date		S		1
//384		455		DTP		Date - Begin Therapy Date				S		1
//385		455		DTP		Date - Last Certification Date			S		1
//386		455		DTP		Date - Last Seen Date					S		1
//387		455		DTP		Date - Test	Date						S		2
//388		455		DTP		Date - Shipped Date						S		1
//389		455		DTP		Date - Last X-Ray Date					S		1
//390		455		DTP		Date - Initial Treatment Date			S		1
//391		460		QTY		Ambulance Patient Count					S		5
//392		460		QTY		Obstetric Anesthesia Addn'l Units		S		5

//393		462		MEA		Test Result								S		20

		// (j.jones 2008-05-14 15:25) - PLID 30044 - added support for the MEA line
		
		BOOL bSendTestResults = AdoFldBool(rsCharges, "SendTestResults", FALSE);
		CString strItemCode = GetFieldFromRecordset(rsCharges,"ItemCode");
		strItemCode.TrimLeft();
		strItemCode.TrimRight();

		//test results are only sent for codes J0881, J0882, J0885, J0886 and Q4081
		BOOL bCanSendForCode = FALSE;
		if(strItemCode.CompareNoCase("J0881") == 0) {
			bCanSendForCode = TRUE;
		}
		else if(strItemCode.CompareNoCase("J0882") == 0) {
			bCanSendForCode = TRUE;
		}
		else if(strItemCode.CompareNoCase("J0885") == 0) {
			bCanSendForCode = TRUE;
		}
		else if(strItemCode.CompareNoCase("J0886") == 0) {
			bCanSendForCode = TRUE;
		}
		else if(strItemCode.CompareNoCase("Q4081") == 0) {
			bCanSendForCode = TRUE;
		}

		if(bSendTestResults && bCanSendForCode) {

			OutputString = "MEA";

			//Ref.	Data		Name									Attributes
			//Des.	Element

			//MEA01 737		Measurement Reference ID Code				O 1	ID	2/2

			//OG - Original Starting dosage
			//TR - Test Results

			CString strID = AdoFldString(rsCharges, "TestResultID", "");
			strID.TrimLeft();
			strID.TrimRight();

			OutputString += ParseANSIField(strID,2,2);

			//MEA02 738		Measurement Qualifier						O 1	ID	1/3

			//HT - Height
			//R1 - Hemoglobin
			//R2 - Hematocrit
			//R3 - Epoetin Starting Dosage
			//R4 - Creatinine

			CString strType = AdoFldString(rsCharges, "TestResultType", "");
			strType.TrimLeft();
			strType.TrimRight();

			OutputString += ParseANSIField(strType,1,3);

			//MEA03 739		Measurement Value							X 1	R	1/20

			CString strResult = AdoFldString(rsCharges, "TestResult", "");
			strResult.TrimLeft();
			strResult.TrimRight();

			OutputString += ParseANSIField(strResult,1,20);

			//MEA04	NOT USED
			//MEA05	NOT USED
			//MEA06	NOT USED
			//MEA07	NOT USED
			//MEA08	NOT USED
			//MEA09	NOT USED
			//MEA10	NOT USED

			//do not send if any of the three fields are blank
			if(!strID.IsEmpty() && !strType.IsEmpty() && !strResult.IsEmpty()) {
				EndANSISegment(OutputString);
			}
		}

//395		465		CN1		Contract Information					S		1

		// (j.jones 2006-11-24 15:20) - PLID 23415 - use the contract information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending contract information
		if(!m_pEBillingInfo->IsPrimary && m_HCFAInfo.ANSI_EnablePaymentInfo == 1
			&& m_HCFAInfo.ANSI_2400Contract == 1) {

			OutputString = "CN1";

			//Ref.	Data		Name									Attributes
			//Des.	Element

			//CN101 1166	Contract Type Code						M 1	ID	2/2

			//01 - Diagnosis Related Group (DRG)
			//02 - Per Diem
			//03 - Variable Per Diem
			//04 - Flat
			//05 - Capitated
			//06 - Percent
			//09 - Other

			//apparently, this only needs to be 09 and the charge total
			str = "09";
			OutputString += ParseANSIField(str,2,2);

			//CN102 782		Monetary Amount							O 1	R	1/18

			//use the cyLineTotal from earlier, to show the charge total
			str = FormatCurrencyForInterface(cyLineTotal,FALSE,FALSE);

			//see if we need to trim the zeros
			// (j.jones 2012-05-25 16:29) - PLID 50657 - now a modular function, truncates if m_bTruncateCurrency is enabled
			str = FormatANSICurrency(str);

			OutputString += ParseANSIField(str,1,18);

			//CN103 332		Percent, Decimal Format					O 1	R	1/6

			//we don't use this

			OutputString += ParseANSIField("",1,6);

			//CN104 127		Reference Identification				O 1	AN	1/30

			//we don't use this

			OutputString += ParseANSIField("",1,30);

			//CN105 338		Terms Discount Percent					O 1	R	1/6

			//we don't use this

			OutputString += ParseANSIField("",1,6);

			//CN106 799		Version Identifier						O 1	AN	1/30

			//we don't use this

			OutputString += ParseANSIField("",1,30);

			EndANSISegment(OutputString);
		}

//397		470		REF		Repriced Line Item Reference Number		S		1
//398		470		REF		Adjusted Repriced Line Item Ref. Num.	S		1
//399		470		REF		Prior Authorization						S		2

		//not supported

//401		470		REF		Line Item Control Number				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2010-10-15 11:50) - at long last we have a way to send our
		// internal charge ID, which remit files are *required* to send back to us

		OutputString = "REF";

		//REF01	128			Reference Identification Qualifier		M 1	ID	2/3

		//static 6R
		OutputString += ParseANSIField("6R",2,3);

		//REF02	127			Reference Identification				X 1	AN	1/50

		//ChargesT.ID
		str.Format("%li", AdoFldLong(rsCharges, "ChargeID"));

		//strangely, this is limited to 30 characters, but we would never hit that amount,
		//so this is just for posterity, send the right-most 30 for an attempt at uniqueness
		if(str.GetLength() > 30) {
			str = str.Right(30);
		}
		OutputString += ParseANSIField(str,1,50);

		//REF03	NOT USED
		OutputString += "*";
		//REF04	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

//403		470		REF		Mammography Certification Number		S		1
//404		470		REF		CLIA Identification						S		1
//405		470		REF		Referring CLIA Facility Identification	S		1
//406		470		REF		Immunization Batch Number				S		1
//407		470		REF		Referral Number							S		2

		//not used

//409		475		AMT		Sales Tax Amount						S		1

		//TODO: This is supposed to show not the tax rate, but the actual
		//monetary tax amount. Research and find out if we need this, as most
		//(or all?) insurance companies don't pay tax.

		//The book says that this field is required if the submitter is required
		//to report this information to the receiver.

//410		475		AMT		Postage Claimed Amount					S		1
//411		480		K3		File Information						S		10

		//not used

//413		485		NTE		Line Note								S		1
				
		CString strNTE = "";
		BOOL bKeepNTEPunctuation = FALSE;

		// (j.jones 2014-08-01 10:49) - PLID 63105 - charge lab codes, if present,
		// take precedence over all other NTE features
		strNTE = AdoFldString(rsCharges, "LabTestCodes", "");
		strNTE.TrimLeft(); strNTE.TrimRight();
		if (!strNTE.IsEmpty()) {
			//lab codes will not have punctuation removed by our export code,
			//but they do need basic ANSI characters stripped out
			strNTE.Replace('*', ' ');
			strNTE.Replace(':', ' ');
			strNTE.Replace('~', ' ');
			strNTE.Replace('^', ' ');
			strNTE.Replace('`', ' ');
			strNTE.TrimLeft(); strNTE.TrimRight();
			bKeepNTEPunctuation = TRUE;
		}

		// (j.jones 2011-07-06 13:57) - PLID 44327 - now anesthesia times can optionally
		// export here, and if they do they will be sent instead of the normal NTE output
		if(strNTE.IsEmpty() && m_pEBillingInfo->AnesthesiaTimes && AdoFldBool(rsCharges, "Anesthesia",FALSE)) {
			
			strNTE = CalculateAnesthesiaNoteForANSI(m_pEBillingInfo->BillID);
		}

		// (j.jones 2011-09-16 11:25) - PLID 45526 - if the charge has a Notes record that is configured
		// to send on the claim, send it here - this overrides all other settings to fill the 2400 NTE
		//*except* the anesthesia note
		// (j.jones 2012-01-04 08:53) - PLID 47277 - if this is an NOCCode, we would have sent
		// the line note in SV101-7, and not here
		if(strNTE.IsEmpty() && !bNOCCode) {
			strNTE = AdoFldString(rsCharges, "LineNote", "");
			strNTE.Replace("\r\n"," ");
			//incase a \r or \n is on its own, remove it
			strNTE.Replace("\r","");
			strNTE.Replace("\n","");
			strNTE.TrimLeft();
			strNTE.TrimRight();
		}
		
		if(strNTE.IsEmpty()) {
			//if the anesthesia note was not sent, use the normal NTE calculations

			BOOL bSendCorrespondence = ReturnsRecords("SELECT SendCorrespondence FROM BillsT WHERE SendCorrespondence = 1 AND ID = %li",m_pEBillingInfo->BillID);

			if((m_HCFAInfo.ANSI_SendBox19 == 1 && m_HCFAInfo.ANSI_SendBox19Segment == 1)
				|| (m_HCFAInfo.ANSI_SendCorrespSegment == 1 && bSendCorrespondence)) {

				//we can only do one or the other, correspondence takes precedence
				// (j.jones 2008-10-06 10:43) - PLID 31580 - this setting can be configured to run
				// in either the 2300 or 2400 loop
				if((m_HCFAInfo.ANSI_SendCorrespSegment == 1 && bSendCorrespondence)) {
					//send correspondence date

					// (j.jones 2007-03-15 12:02) - PLID 25224 - do not default to using a colon after MAIL DOC

					CString strPreceding = "MAIL DOC";
					if(!m_HCFAInfo.strCorrespondenceNote.IsEmpty())
						strPreceding = m_HCFAInfo.strCorrespondenceNote;

					strPreceding.TrimRight();

					strNTE.Format("%s %s",strPreceding,COleDateTime::GetCurrentTime().Format("%m-%d-%y"));

					// (j.jones 2007-03-15 12:03) - PLID 25224 - disallow colons
					strNTE.Replace(":"," ");			
				}
				// (j.jones 2008-10-06 11:16) - PLID 31578 - HCFABox19 may be sendable in the 2300 or 2400 NTE
				else if(m_HCFAInfo.ANSI_SendBox19 == 1 && m_HCFAInfo.ANSI_SendBox19Segment == 1) {

					//send block 19
					CString strHCFABlock19 = "";

					// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
					_RecordsetPtr rs = CreateParamRecordset("SELECT HCFABlock19 FROM BillsT WHERE ID = {INT}",m_pEBillingInfo->BillID);
					if(!rs->eof) {
						strHCFABlock19 = AdoFldString(rs, "HCFABlock19","");
						strHCFABlock19.TrimLeft();
						strHCFABlock19.TrimRight();
					}
					rs->Close();

					// (j.jones 2007-03-15 12:03) - PLID 25224 - disallow colons
					strHCFABlock19.Replace(":"," ");
					strNTE = strHCFABlock19;
				}
			}
		}

		// (j.jones 2011-07-06 13:56) - PLID 44327 - moved the actual output lower in code,
		// after strNTE is calculated
		if(!strNTE.IsEmpty()) {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "NTE";

			//NTE01 363			Note Reference Code						O	ID	3/3

			//ADD - Additional Information
			//DCP - Goals, Rehabilitation Potential, or Discharge Plans

			OutputString += ParseANSIField("ADD",3,3);

			//NTE02 352			Description								M	AN	1/80

			// (j.jones 2014-08-01 16:30) - PLID 63105 - if the NTE is a lab code,
			// we will not unpunctuate it
			OutputString += ParseANSIField(strNTE, 1, 80, 0, 'L', ' ', bKeepNTEPunctuation);

			EndANSISegment(OutputString);
		}

//414		485		NTE		Third Party Organization Notes			S		1
//415		488		PS1		Purchased Service Information			S		1
//416		492		HCP		Line Pricing/Repricing Information		S		1

		//not used

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2400_Prof");

	return Error_Other;
}

// (j.jones 2010-10-18 16:17) - PLID 40346 - initial implementation of ANSI 5010 Institutional claim
int CEbilling::ANSI_5010_2400_Inst(ADODB::_RecordsetPtr &rsCharges) {

	//Service Line
	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2400\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	//increment the count for the current service line
	m_ANSIServiceCount++;

	FieldsPtr fields = rsCharges->Fields;

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2400 - SERVICE LINE									50

//423		365		LX		Service Line							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "LX";

		//LX01	554			Assigned Number							M 1	N0	1/6

		//m_ANSIServiceCount
		str.Format("%li",m_ANSIServiceCount);
		OutputString += ParseANSIField(str,1,6);

		EndANSISegment(OutputString);

//424		370		SV2		Institutional Service Line				R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "SV2";

		//SV201		234		Product/Service ID						X 1	AN	1/48

		//the revenue code

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsRevCode = CreateParamRecordset("SELECT (CASE WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Code ELSE UB92CategoriesT1.Code END) AS Code "
					"FROM ServiceT "
					"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
					"LEFT JOIN (SELECT ServiceRevCodesT.* FROM ServiceRevCodesT WHERE InsuranceCompanyID = {INT}) AS ServiceRevCodesT ON ServiceT.ID = ServiceRevCodesT.ServiceID "
					"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
					"WHERE ServiceT.ID = {INT}",m_pEBillingInfo->InsuranceCoID, AdoFldLong(rsCharges, "ServiceID",-1));

		if(!rsRevCode->eof) {
			str = AdoFldString(rsRevCode, "Code","");

			// (j.jones 2007-05-02 10:37) - PLID 25855 - If the option to
			// send 4-digit revenue codes is enabled, prepend with zeros,
			// such that "360" becomes "0360". Don't bother doing this
			// if the code is blank.
			// Note: technically, the code "could" be greater than 4 digits,
			// there's nothing in the program or the ANSI format that would
			// stop you from making one, but it would be an invalid code.
			// So just expand to 4, don't truncate from 4.
			str.TrimLeft();
			str.TrimRight();
			if(m_bFourDigitRevenueCode && !str.IsEmpty() && str.GetLength() < 4) {
				while(str.GetLength() < 4) {
					str = "0" + str;
				}
			}
		}
		else
			str = "";
		rsRevCode->Close();
		
		OutputString += ParseANSIField(str,1,48);
		
		//SV202	C003		Composite Medical Procedure Identifier	M 1

		//SV201-1	235		Product/Service ID Qualifier			M	ID	2/2

		//ER - Jurisdiction Specific Procedure and Supply Codes
		//HC - Health Care Financing Administration Common Procedural Coding System (HCPCS) Codes
		//HP - Health Insurance Prospective Payment System (HIPPS) Skilled Nursing Facility Rate Code
		//IV - Home Infusion EDI Coalition (HIEC) Product/Service Code
		//WK - Advanced Billing Concepts (ABC) Codes

		//Because the AMA’s CPT codes are also level 1 HCPCS codes, they are reported under HC

		// (j.jones 2007-07-27 09:17) - PLID 26839 - changed so we don't export
		// the HC composite segment if we don't have a CPT code

		CString strQual = "HC";

		// (j.jones 2013-01-25 12:25) - PLID 54853 - IsValidServiceCodeQualifier
		// needs updated any time we support exporting a new qualifier type,
		// currently we only support HC. This ensures that we never send other 
		// qualifiers without updating this function.
		if(!IsValidServiceCodeQualifier(strQual)) {
			//You just tried to send a qualifier not supported by IsValidServiceCodeQualifier,
			//Go into that function and add your new qualifier to the supported list.
			ASSERT(FALSE);
			ThrowNxException("Loop 2400 tried to send invalid service qualifier %s.", strQual);			
		}

		CString strCPTOut = ParseANSIField(strQual,2,2);
		BOOL bUseCPT = FALSE;

		//SV202-2	234		Product/Service ID						M	AN	1/48

		//ChargesT.ItemCode
		str = GetFieldFromRecordset(rsCharges,"ItemCode");
		if(str != "") {
			//now we will output this line
			bUseCPT = TRUE;

			strCPTOut += ":";
			strCPTOut += str;
		}

		//don't need to build up the rest of the HC segment unless we have a code
		if(bUseCPT) {

			// (j.jones 2014-08-08 15:59) - PLID 63106 - we need to export the :
			// for every field in case SV202-7 is filled

			//SV202-3	1339	Procedure Modifier						O	AN	2/2

			//ChargesT.CPTModifier
			strCPTOut += ":";
			str = GetFieldFromRecordset(rsCharges,"CPTModifier");
			if(str != "") {
				strCPTOut += str;
			}

			//SV202-4	1339	Procedure Modifier						O	AN	2/2
			
			//ChargesT.CPTModifier2
			strCPTOut += ":";
			str = GetFieldFromRecordset(rsCharges,"CPTModifier2");
			if(str != "") {
				strCPTOut += str;
			}

			//SV202-5	1339	Procedure Modifier						O	AN	2/2

			//ChargesT.CPTModifier3
			strCPTOut += ":";
			str = GetFieldFromRecordset(rsCharges,"CPTModifier3");
			if(str != "") {
				strCPTOut += str;
			}

			//SV202-6	1339	Procedure Modifier						O	AN	2/2
				
			//ChargesT.CPTModifier4
			strCPTOut += ":";
			str = GetFieldFromRecordset(rsCharges,"CPTModifier4");
			if(str != "") {				
				strCPTOut += str;
			}

			//SV202-7	352		Description								O	AN 1/80
			
			// (j.jones 2014-08-01 10:49) - PLID 63106 - Export charge lab codes here,
			// if we have any. This should take precedence over any other future
			// features that fill SV202-7 unless that new feature also takes precedence
			// over the lab codes on the paper UB form as well.
			strCPTOut += ":";
			str = GetFieldFromRecordset(rsCharges, "LabTestCodes");			
			//test codes will not have their punctuation removed, unless it's
			//a basic ANSI character
			str.Replace('*', ' ');
			str.Replace(':', ' ');
			str.Replace('~', ' ');
			str.Replace('^', ' ');
			str.Replace('`', ' ');
			str.TrimLeft(); str.TrimRight();

			if (str.GetLength() > 80) {
				str = str.Left(80);
			}

			if(!str.IsEmpty()) {
				strCPTOut += str;
			}

			//SV202-8	NOT USED

			// (j.jones 2014-08-08 16:00) - PLID 63106 - in the event we did not fill SV202-7,
			// trim off all the unnecessary : characters
			strCPTOut.TrimRight(":");

			OutputString += strCPTOut;
		}
		else {
			OutputString += "*";
		}

		//SV203	782			Monetary Amount							O 1	R	1/18

		//note: when CPT codes are not used, we would group by RevCode and in turn
		//the amount and quantity would add up based on RevCode. We don't do this right now.

		//LineTotal is the amount * qty * tax * modifiers
		var = fields->Item["LineTotal"]->Value;
		if(var.vt == VT_CY) {
			str = FormatCurrencyForInterface(var.cyVal,FALSE,FALSE);

			//see if we need to trim the zeros
			// (j.jones 2012-05-25 16:29) - PLID 50657 - now a modular function, truncates if m_bTruncateCurrency is enabled
			str = FormatANSICurrency(str);
		}
		else
			str = "0";
		str.Replace(GetCurrencySymbol(),"");
		str.Replace(GetThousandsSeparator(),"");
		OutputString += ParseANSIField(str,1,18);

		//SV204	355			Unit or Basis for Measurement Code		X 1	ID	2/2

		//DA - Days
		//UN - Unit

		//we only support UN
		OutputString += ParseANSIField("UN",2,2);

		//SV205	380			Quantity								X 1	R	1/15

		//ChargesT.Quantity
		var = fields->Item["Quantity"]->Value;
		if(var.vt == VT_R8)
			str.Format("%g",var.dblVal);
		else
			str = "1";
		OutputString += ParseANSIField(str,1,15);

		//SV206 NOT USED
		OutputString += "*";

		//SV207	782			Monetary Amount							O 1	R	1/18
		//Box 44, Non-Covered Charges - we don't use this
		OutputString += "*";
				
		//SV208	NOT USED
		//SV209	NOT USED
		//SV210 NOT USED

		EndANSISegment(OutputString);

//429		155		PWK		Line Supplemental Information			S		10
		//not supported

//433		455		DTP		Service Line Date						R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M 1	ID	3/3
		
		//static "472"
		OutputString += ParseANSIField("472",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

		// (j.jones 2007-04-30 12:35) - PLID 25848 - if ServiceDateTo
		// and ServiceDateFrom are the same, send D8 and ServiceDateFrom,
		// otherwise send RD8 and ServiceDateFrom-ServiceDateTo

		CString strDateQual = "D8";
		CString strDate;

		COleDateTime dtFrom = VarDateTime(fields->Item["ServiceDateFrom"]->Value);
		COleDateTime dtTo = VarDateTime(fields->Item["ServiceDateTo"]->Value);

		//RD8 is required only when the “To and From” dates are different.
		//However, at the discretion of the submitter, RD8 can also be used
		//when the “To and From” dates are the same.

		if(dtFrom == dtTo) {
			strDateQual = "D8";
			strDate = dtFrom.Format("%Y%m%d");
		}
		else {
			strDateQual = "RD8";
			strDate.Format("%s-%s",dtFrom.Format("%Y%m%d"), dtTo.Format("%Y%m%d"));
		}
		
		OutputString += ParseANSIField(strDateQual,2,3);

		//DTP03	1251		Date Time Period						M 1	AN	1/35

		// (j.jones 2012-10-30 16:49) - PLID 53364 - do not strip punctuation here, RD8 dates can have hyphens
  		OutputString += ParseANSIField(strDate, 1, 35, FALSE, 'L', ' ', TRUE);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//435		470		REF		Line Item Control Number				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2010-10-18 14:37) - at long last we have a way to send our
		// internal charge ID, which remit files are *required* to send back to us

		OutputString = "REF";

		//REF01	128			Reference Identification Qualifier		M 1	ID	2/3

		//static 6R
		OutputString += ParseANSIField("6R",2,3);

		//REF02	127			Reference Identification				X 1	AN	1/50

		//ChargesT.ID
		str.Format("%li", AdoFldLong(rsCharges, "ChargeID"));

		//strangely, this is limited to 30 characters, but we would never hit that amount,
		//so this is just for posterity, send the right-most 30 for an attempt at uniqueness
		if(str.GetLength() > 30) {
			str = str.Right(30);
		}
		OutputString += ParseANSIField(str,1,50);

		//REF03	NOT USED
		OutputString += "*";
		//REF04	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//437		496		REF		Repriced Line Item Reference Number		S		1
//438		496		REF		Adjusted Repriced Line Item Reference Number		S		1
//439		475		AMT		Service Tax Amount						S		1
//440		475		AMT		Facility Tax Amount						S		1
		
		//Required when a tax or surcharge applies to the service being
		//reported in SV201 and the submitter is required to report that information
		//to the receiver.

		/// (j.jones 2010-10-18 14:40) - Seems like these are used to support sending tax
		//if insurance requires tax (which is virtually never), the total charge amount we
		//sent in SV203 has to include the tax amount as well, with the tax-only amounts
		//represented here. The specs do not explain the difference between service tax
		//and facility tax. However, these fields also existed in 4010, and we have
		//never used them so far.

//441		190		NTE		Third Party Organization Notes			S		1
//442		241		HCP		Claim Pricing/Repricing Information		S		1

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2400_Inst");

	return Error_Other;
}

// (j.jones 2008-05-28 14:14) - PLID 30176 - added support for Loop 2410 and NDC Codes
// (j.jones 2009-08-12 16:48) - PLID 35096 - added support for unit information and prescription number
// (j.jones 2010-10-18 09:11) - PLID 32848 - Price is obsolete in 5010, so I removed the parameter
int CEbilling::ANSI_5010_2410(CString strNDCCode, CString strUnitType, double dblUnitQty, CString strPrescriptionNumber) {
	
	//Drug Identification

	try {

		CString str;
	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

			str = "\r\n2410\r\n";
			m_OutputFile.Write(str,str.GetLength());	
	}

		CString OutputString;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2410 - DRUG IDENTIFICATION							25

//Page
//426		494		LIN		Drug Identification						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "LIN";

		//LIN01	NOT USED;

		OutputString += "*";

		//LIN02 235			Product/Service ID Qualifier			M 1	ID	2/2

		//N4 - National Drug Code in 5-4-2 Format
		//EN - EAN/UCC - 13
		//EO - EAN/UCC - 8
		//HI - HIBC (Health Care Industry Bar Code)
		//ON - Customer Order Number
		//UK - GTIN (14-digit Data Structure)
		//UP - UCC - 12

		//we only support "N4"		
		OutputString += ParseANSIField("N4",2,2);

		//LIN03 234			Product/Service ID						M 1	AN	1/48

		//ChargesT.NDCCode

		//remove dashes
		strNDCCode.Replace("-", "");
		strNDCCode = StripPunctuation(strNDCCode);
		OutputString += ParseANSIField(strNDCCode,1,48);

		//LIN04	NOT USED
		//...
		//LIN31	NOT USED

		if(!strNDCCode.IsEmpty()) {
			EndANSISegment(OutputString);
		}
		else {
			//if the NDC code is empty, return now,
			//because we can't send this loop at all
			return Success;
		}

///////////////////////////////////////////////////////////////////////////////

//426		495		CTP		Drug Quantity							S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2009-08-12 16:30) - PLID 35096 - supported this segment

		OutputString = "CTP";

		//CTP01	NOT USED
		OutputString += "*";		
		//CTP02	NOT USED
		OutputString += "*";
		//CTP03	NOT USED
		OutputString += "*";

		//CTP04	380			Quantity								X 1	R	1/15

		str.Format("%g", dblUnitQty);
		OutputString += ParseANSIField(str,1,15);

		//CTP05	C001		COMPOSITE UNIT OF MEASURE				X 1

		//CTP05 - 1 355		Unit or Basis for Measurement Code		M	D	2/2

		//F2 - International Unit
		//GR - Gram
		//ME - Milligram
		//ML - Milliliter
		//UN - Unit
		OutputString += ParseANSIField(strUnitType,2,2);

		//CTP05 - 2 NOT USED
		//through
		//CTP05 - 15 NOT USED

		//CTP06 NOT USED
		OutputString += "*";
		//CTP07 NOT USED
		OutputString += "*";
		//CTP08 NOT USED
		OutputString += "*";
		//CTP09 NOT USED
		OutputString += "*";
		//CTP10 NOT USED
		OutputString += "*";
		//CTP11 NOT USED
		OutputString += "*";

		//if we don't have the unit type, we cannot export this segment, so don't even try
		if(!strUnitType.IsEmpty()) {
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//428		496		REF		Prescription Number						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2009-08-12 16:30) - PLID 35096 - supported this segment

		OutputString = "REF";

		//REF01	128			Reference Identification Qualifier		M 1	ID	2/3
		
		//VY - Link Sequence Number
		//XZ - Pharmacy Prescription Number

		//we use XZ
		OutputString += ParseANSIField("XZ",2,3);

		//REF02	127			Reference Identification				X 1	AN	1/50

		strPrescriptionNumber.TrimLeft();
		strPrescriptionNumber.TrimRight();
		OutputString += ParseANSIField(strPrescriptionNumber,1,50);

		//REF03	NOT USED
		OutputString += "*";
		//REF04	NOT USED
		OutputString += "*";

		if(strPrescriptionNumber != "") {
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2410");

	return Error_Other;
}

int CEbilling::ANSI_5010_2420A(long ProviderID) {

	//Rendering Provider Name

	//JJ - We CAN have different providers per charge, so this loop can be used.

	//First check to see if the charge line provider is different from the provider
	//we chose to use as the claim provider. If it's different, then use this loop.
	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2420A\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	try {

		CString OutputString,str;
		_variant_t var;

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT * FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID WHERE ID = {INT}",ProviderID);

		if(rs->eof) {
			//if the recordset is empty, there is no doctor. So halt everything!!!
			rs->Close();
			// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
				str.Format("Could not open provider information for patient '%s', Bill ID %li. (ANSI_2420A)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open provider information. (ANSI_2420A)"
					"\nIt is possible that you have no provider selected on this patient's bill.";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		FieldsPtr fields = rs->Fields;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2420A - RENDERING PROVIDER NAME						1

//460		500		NM1		Rendering Provider Name					S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M  1	ID	2/3

		//static "82"
		OutputString += ParseANSIField("82",2,3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//static value "1" for person, "2" for non-person entity
		str = "1";

		// (j.jones 2011-11-07 14:46) - PLID 46299 - check the UseCompanyOnClaims setting to send the provider as an organization
		CString strProviderCompany = VarString(rs->Fields->Item["Company"]->Value, "");
		BOOL bUseProviderCompanyOnClaims = VarBool(rs->Fields->Item["UseCompanyOnClaims"]->Value, FALSE);

		if(bUseProviderCompanyOnClaims) {
			strProviderCompany.TrimLeft();
			strProviderCompany.TrimRight();
			if(strProviderCompany.IsEmpty()) {
				//can't use the company if it is blank
				bUseProviderCompanyOnClaims = FALSE;
			}

			if(bUseProviderCompanyOnClaims) {
				str = "2";
			}
			else {
				str = "1";
			}
		}		
		OutputString += ParseANSIField(str, 1, 1);

		//NM103	1035		Name Last or Organization Name			O 1	AN	1/60

		//PersonT.Last

		if(bUseProviderCompanyOnClaims) {
			str = strProviderCompany;
		}
		else {
			str = GetFieldFromRecordset(rs, "Last");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,60);

		//NM104	1036		Name First								O 1	AN	1/35

		//PersonT.First

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "First");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,35);

		//NM105	1037		Name Middle								O 1	AN	1/25

		//PersonT.Middle

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "Middle");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,25);

		//NM106	NOT USED
		OutputString += "*";

		//NM107	1039		Name Suffix								O 1	AN	1/10

		//PersonT.Title
		// (j.jones 2007-08-03 11:44) - PLID 26934 - changed from Suffix to Title
		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "Title");
		}

		OutputString += ParseANSIField(str,1,10);

		//NM108	66			Identification Code	Qualifier			X 1	ID	1/2

		//always XX - NPI in 5010
		str = "";

		// (j.jones 2007-08-08 13:14) - PLID 25395 - if there is a 24J override in AdvHCFAPinT, use it here
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType != actInst) {
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			_RecordsetPtr rsOver = CreateParamRecordset("SELECT Box24JNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
			if(!rsOver->eof) {
				var = rsOver->Fields->Item["Box24JNPI"]->Value;
				if(var.vt == VT_BSTR) {
					CString strOver = VarString(var,"");
					strOver.TrimLeft();
					strOver.TrimRight();
					if(!strOver.IsEmpty())
						str = strOver;
				}
			}
			rsOver->Close();
		}

		if(str.IsEmpty()) {
			str = GetFieldFromRecordset(rs, "NPI");
		}

		// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens,
		// although NPI should always be numeric
		str = StripSpacesAndHyphens(str);
		
		if(str != "")
			OutputString += ParseANSIField("XX",1,2);
		else
			OutputString += "*";

		//NM109	67			Identification Code						X	AN	2/80

		if(str != "")
			OutputString += ParseANSIField(str,2,80);
		else
			OutputString += "*";

		//NM110 NOT USED
		//NM111	NOT USED
		//NM112	NOT USED

		EndANSISegment(OutputString);

//433		505		PRV		Rendering Provider Specialty Info.		R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "PRV";

		//PRV01	1221		Provider Code							M 1	ID	1/3

		//static "PE" (performing)
		OutputString += ParseANSIField("PE",1,3);

		CString strTaxonomy = AdoFldString(rs, "TaxonomyCode","");
		strTaxonomy.TrimLeft();
		strTaxonomy.TrimRight();

		// (j.jones 2013-09-05 11:48) - PLID 58252 - the taxonomy code override will
		// be used only if one exists
		_RecordsetPtr rsTaxonomyOver= CreateParamRecordset("SELECT ANSI_2420A_Taxonomy AS TaxonomyCode "
			"FROM EbillingSetupT "
			"WHERE ANSI_2420A_Taxonomy <> '' AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
			m_pEBillingInfo->HCFASetupID, ProviderID, m_pEBillingInfo->BillLocation);
		if(!rsTaxonomyOver->eof) {
			CString strTaxonomyOverride = AdoFldString(rsTaxonomyOver, "TaxonomyCode","");
			strTaxonomyOverride.TrimLeft();
			strTaxonomyOverride.TrimRight();
			//we don't permit overriding with nothing, so only use this
			//if there really is an override value
			if(!strTaxonomyOverride.IsEmpty()) {
				strTaxonomy = strTaxonomyOverride;
			}
		}

		// (j.jones 2008-05-01 10:47) - PLID 28691 - don't output if the taxonomy code is blank
		if(!strTaxonomy.IsEmpty()) {

			//PRV02	128			Reference Identification Qualifier		M 1	ID	2/3

			//static value "PXC"
			OutputString += ParseANSIField("PXC",2,3);

			//PRV03	127			Reference Identification				M 1	AN	1/50

			//ProvidersT.TaxonomyCode
			//This comes from a published HIPAA list.
			//"2086S0122X" is for Plastic and Reconstructive Surgery
			OutputString += ParseANSIField(strTaxonomy,1,50);

			//PRV04	NOT USED
			OutputString += "*";
			//PRV05	NOT USED
			OutputString += "*";
			//PRV06	NOT USED
			OutputString += "*";

			EndANSISegment(OutputString);
		}

//434		525		REF		Rendering Provider Secondary Ident.		S		5

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//in 5010, only 0B, 1G, G2, and LU are valid, but we continue to let them set up anything they want

		OutputString = "REF";

		// (j.jones 2008-05-02 10:09) - PLID 27478 - we need to silently skip
		// adding additional REF segments that have the same qualifier, so
		// track them in an array
		CStringArray arystrQualifiers;

		//REF01	128			Reference Identification Qualifier		M 1	ID	2/3

		// (j.jones 2010-04-14 10:20) - PLID 38194 - the ID is now calculated in a shared function
		CString strIdentifier = "", strID = "", strLoadedFrom = "";
		EBilling_Calculate2420A_REF(strIdentifier,strID,strLoadedFrom,ProviderID,m_pEBillingInfo->HCFASetupID,
			m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->InsuredPartyID, m_pEBillingInfo->BillLocation);

		// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
		if(m_bDisableREFXX && strIdentifier.CompareNoCase("XX") == 0) {
			strIdentifier = "";
			strID = "";
			//log that this happened
			Log("Ebilling file tried to export 2420A REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
		}
		
		OutputString += ParseANSIField(strIdentifier,2,3);

		//REF02	127			Reference Identification				X 1	AN	1/50

		OutputString += ParseANSIField(strID,1,50);

		//REF03	NOT USED
		OutputString += "*";
		//REF04	NOT USED
		OutputString += "*";

		// (j.jones 2007-09-20 10:51) - PLID 27455 - do not send this segment if the
		// qualifier or ID are blank
		if(strIdentifier != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdentifier)) {
			EndANSISegment(OutputString);

			// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
			arystrQualifiers.Add(strIdentifier);
		}

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2420A");

	return Error_Other;
}

/* These loops all pertain to different providers/locations per charge line. We won't use them.

int CEbilling::ANSI_5010_2420B() {

	//Purchased Service Provider Name

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2420B - PURCHASED SERVICE PROVIDER NAME				1

//436		500		NM1		Purchased Service Provider Name			S		1
//439		525		REF		Purchased Service Prov. Secondary Info.	S		5

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2420B");

	return Error_Other;
}

int CEbilling::ANSI_5010_2420C() {

	//Service Facility Location

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2420C - SERVICE FACILITY INFORMATION					1

//441		500		NM1		Service Facility Location				S		1
//444		514		N3		Service Facility Location Address		R		1
//445		520		N4		Service Facility Loc. City/State/Zip	R		1
//447		525		REF		Service Facility Loc. Secondary Ident.	S		5

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2420C");

	return Error_Other;
}
*/

// (j.jones 2014-04-23 14:51) - PLID 61834 - supported 2420D
int CEbilling::ANSI_5010_2420D_Prof(long nPersonID) {

	//Supervising Provider Name

	try {

		CString str;
		// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
		if (m_bHumanReadableFormat) {

				str = "\r\n2420D\r\n";
				m_OutputFile.Write(str,str.GetLength());
		}

		CString OutputString;
		_variant_t var;

		//this can be either a provider or a referring physician
		_RecordsetPtr rsDoctorInfo = CreateParamRecordset("SELECT Last, First, Middle, Title, "
			"CASE WHEN ProvidersT.PersonID Is Not Null THEN ProvidersT.NPI ELSE ReferringPhysT.NPI END AS NPI, "
			"Convert(bit, CASE WHEN ProvidersT.PersonID Is Not Null THEN 1 ELSE 0 END) AS IsProvider "
			"FROM PersonT "
			"LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
			"LEFT JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
			"WHERE PersonT.ID = {INT} "
			"AND (ProvidersT.PersonID Is Not Null OR ReferringPhysT.PersonID Is Not Null)", nPersonID);

		if(rsDoctorInfo->eof) {
			//if the recordset is empty, there is no doctor. So halt everything!!!
			rsDoctorInfo->Close();
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				str.Format("Could not open supervising provider information for patient '%s', Bill ID %li. (ANSI_2420D)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open supervising provider information. (ANSI_2420D)"
					"\nIt is possible that you have no supervising provider selected on this patient's bill.";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		FieldsPtr doctorFields = rsDoctorInfo->Fields;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2420D	- SUPERVISING PROVIDER NAME				1

//449		500		NM1		Supervising Provider Name				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M  1	ID	2/3

		//"DQ" - Supervising Physician
		OutputString += ParseANSIField("DQ",2,3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//static value "1" for person, "2" is not a valid value in this loop
		OutputString += ParseANSIField("1", 1, 1);

		//NM103	1035		Name Last or Organization Name			O 1	AN	1/60

		//PersonT.Last
		str = GetFieldFromRecordset(rsDoctorInfo, "Last");
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,60);

		//NM104	1036		Name First								O 1	AN	1/35

		//PersonT.First

		str = GetFieldFromRecordset(rsDoctorInfo, "First");
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,35);

		//NM105	1037		Name Middle								O 1	AN	1/25

		//PersonT.Middle
		str = GetFieldFromRecordset(rsDoctorInfo, "Middle");
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,25);

		//NM106	NOT USED
		OutputString += "*";

		//NM107	1039		Name Suffix								O 1	AN	1/10

		//PersonT.Title
		str = GetFieldFromRecordset(rsDoctorInfo, "Title");
		OutputString += ParseANSIField(str,1,10);

		//NM108	66			Identification Code	Qualifier			X 1	ID	1/2

		//always XX - NPI in 5010
		str = GetFieldFromRecordset(rsDoctorInfo, "NPI");
		// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens,
		// although NPI should always be numeric
		str = StripSpacesAndHyphens(str);
		
		if(str != "")
			OutputString += ParseANSIField("XX",1,2);
		else
			OutputString += "*";

		//NM109	67			Identification Code						X	AN	2/80

		if(str != "")
			OutputString += ParseANSIField(str,2,80);
		else
			OutputString += "*";

		//NM110 NOT USED
		//NM111	NOT USED
		//NM112	NOT USED

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//452	5250	REF		Supervising Provider Secondary Information	S		5

		//we do not support REF in this loop

		//only 0B, 1G, G2, and LU are valid

		OutputString = "REF";
		
		//REF01	128			Reference Identification Qualifier		M 1	ID	2/3

		CString strIdentifier = "";
		OutputString += ParseANSIField(strIdentifier,2,3);

		//REF02	127			Reference Identification				X 1	AN	1/50

		CString strID = "";
		OutputString += ParseANSIField(strID,1,50);

		//REF03	NOT USED
		OutputString += "*";
		//REF04	NOT USED
		OutputString += "*";

		//do not send this segment if the qualifier or ID are blank
		if(strIdentifier != "" && strID != "") {
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2420D_Prof");

	return Error_Other;
}

// (j.jones 2013-06-03 11:56) - PLID 54091 - supported 2420E
// (j.jones 2014-04-23 14:51) - PLID 61834 - added PersonID
int CEbilling::ANSI_5010_2420E_Prof(long nPersonID) {

	//Ordering Provider Name

	try {

		CString str;
		// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
		if (m_bHumanReadableFormat) {

				str = "\r\n2420E\r\n";
				m_OutputFile.Write(str,str.GetLength());	
		}

		CString OutputString;
		_variant_t var;

		// (j.jones 2014-04-23 15:26) - PLID 61834 - this can now potentially be a referring physician
		_RecordsetPtr rsDoctorInfo = CreateParamRecordset("SELECT Last, First, Middle, Title, "
			"WorkPhone, Fax, Address1, Address2, City, State, Zip, "
			"CASE WHEN ProvidersT.PersonID Is Not Null THEN ProvidersT.NPI ELSE ReferringPhysT.NPI END AS NPI, "
			"Convert(bit, CASE WHEN ProvidersT.PersonID Is Not Null THEN 1 ELSE 0 END) AS IsProvider "
			"FROM PersonT "
			"LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
			"LEFT JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
			"WHERE PersonT.ID = {INT} "
			"AND (ProvidersT.PersonID Is Not Null OR ReferringPhysT.PersonID Is Not Null)", nPersonID);

		if(rsDoctorInfo->eof) {
			//if the recordset is empty, there is no doctor. So halt everything!!!
			rsDoctorInfo->Close();
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				str.Format("Could not open ordering provider information for patient '%s', Bill ID %li. (ANSI_2420E)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open ordering provider information. (ANSI_2420E)"
					"\nIt is possible that you have no ordering provider selected on this patient's bill.";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		FieldsPtr doctorFields = rsDoctorInfo->Fields;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2420E	- ORDERING PROVIDER NAME						1

//454		500		NM1		Ordering Provider Name					S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M  1	ID	2/3

		//static "DK" - Ordering Physician
		OutputString += ParseANSIField("DK",2,3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//static value "1" for person, "2" is not a valid value in this loop
		OutputString += ParseANSIField("1", 1, 1);

		//NM103	1035		Name Last or Organization Name			O 1	AN	1/60

		//PersonT.Last
		str = GetFieldFromRecordset(rsDoctorInfo, "Last");
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,60);

		//NM104	1036		Name First								O 1	AN	1/35

		//PersonT.First

		str = GetFieldFromRecordset(rsDoctorInfo, "First");
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,35);

		//NM105	1037		Name Middle								O 1	AN	1/25

		//PersonT.Middle
		str = GetFieldFromRecordset(rsDoctorInfo, "Middle");
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,25);

		//NM106	NOT USED
		OutputString += "*";

		//NM107	1039		Name Suffix								O 1	AN	1/10

		//PersonT.Title
		str = GetFieldFromRecordset(rsDoctorInfo, "Title");
		OutputString += ParseANSIField(str,1,10);

		//NM108	66			Identification Code	Qualifier			X 1	ID	1/2

		//always XX - NPI in 5010
		str = "";

		// (j.jones 2014-04-23 15:32) - PLID 61834 - for the AdvHCFAPinT check, and the address,
		// we need to know whether this is a provider or referring physician
		bool bIsProvider = VarBool(rsDoctorInfo->Fields->Item["IsProvider"]->Value) ? true : false;

		//if there is a 24J override in AdvHCFAPinT, use it here
		//this is a HCFA-only function but I'm keeping the Inst check incase we re-use this function in the future
		if(m_actClaimType != actInst && bIsProvider) {
			_RecordsetPtr rsOver = CreateParamRecordset("SELECT Box24JNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}", nPersonID, m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
			if(!rsOver->eof) {
				var = rsOver->Fields->Item["Box24JNPI"]->Value;
				if(var.vt == VT_BSTR) {
					CString strOver = VarString(var,"");
					strOver.TrimLeft();
					strOver.TrimRight();
					if(!strOver.IsEmpty())
						str = strOver;
				}
			}
			rsOver->Close();
		}

		if(str.IsEmpty()) {
			str = GetFieldFromRecordset(rsDoctorInfo, "NPI");
		}

		// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens,
		// although NPI should always be numeric
		str = StripSpacesAndHyphens(str);
		
		if(str != "")
			OutputString += ParseANSIField("XX",1,2);
		else
			OutputString += "*";

		//NM109	67			Identification Code						X	AN	2/80

		if(str != "")
			OutputString += ParseANSIField(str,2,80);
		else
			OutputString += "*";

		//NM110 NOT USED
		//NM111	NOT USED
		//NM112	NOT USED

		EndANSISegment(OutputString);

//457		514		N3		Ordering Provider Address				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element
		// (b.spivey January 26, 2015) - PLID 64452 - Option to not send N3 and N4
		if (m_HCFAInfo.SendN3N4PERSegment) {
			OutputString = "N3";

			//if DocAddress == 1, and we're loading a provider, load the Location address,
			//else use the contact address in the already open recordset
			_RecordsetPtr rsLoc;
			FieldsPtr addressFields = doctorFields;

			// (j.jones 2014-04-23 15:32) - PLID 61834 - only do this if the person is a provider
			if (bIsProvider && m_HCFAInfo.DocAddress == 1) {
				rsLoc = CreateParamRecordset("SELECT * FROM LocationsT WHERE ID = {INT}", m_pEBillingInfo->BillLocation);
				if (rsLoc->eof) {
					rsLoc->Close();
					//if end of file, select from LocationsT, and the recordset will just pull from the first entry
					rsLoc = CreateParamRecordset("SELECT * FROM LocationsT WHERE TypeID = 1");
					if (rsLoc->eof) {
						str = "Error opening location information.";
						rsLoc->Close();
						AfxMessageBox(str);
						return Error_Missing_Info;
					}
				}

				addressFields = rsLoc->Fields;
			}

			//N301	166			Address Information						M 1	AN	1/55

			//PersonT.Address1
			var = addressFields->Item["Address1"]->Value;
			if (var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";

			str = StripPunctuation(str);
			OutputString += ParseANSIField(str, 1, 55);

			//N302	166			Address Information						O 1	AN	1/55

			//PersonT.Address2
			var = addressFields->Item["Address2"]->Value;
			if (var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";

			str = StripPunctuation(str);
			OutputString += ParseANSIField(str, 1, 55);

			EndANSISegment(OutputString);

			//458		520		N4		Ordering Provider City/State/Zip		S		1

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "N4";

			//N401	19			City Name								O 1	AN	2/30

			//PersonT.City
			var = addressFields->Item["City"]->Value;
			if (var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";

			OutputString += ParseANSIField(str, 2, 30);

			//N402	156			State or Province Code					O 1	ID	2/2

			//PersonT.State
			var = addressFields->Item["State"]->Value;
			if (var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";

			//trim and auto-capitalize this
			str.TrimLeft(); str.TrimRight();
			str.MakeUpper();
			OutputString += ParseANSIField(str, 2, 2);

			//N403	116			Postal Code								O 1	ID	3/15

			//PersonT.Zip
			var = addressFields->Item["Zip"]->Value;
			if (var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";

			//don't strip all numbers, just spaces and hyphens
			str = StripSpacesAndHyphens(str);
			OutputString += ParseANSIField(str, 3, 15);

			//N404	26			Country Code							O 1	ID	2/3

			//this is only required if the addess is outside the US
			//as of right now, we can't support it

			OutputString += "*";

			//N405	NOT USED
			OutputString += "*";

			//N406	NOT USED
			OutputString += "*";

			//N407	1715		Country Subdivision Code				X 1	ID	1/3

			//an even more detailed optional value for country, since we
			//do not support countries, we don't support this either

			OutputString += "*";

			EndANSISegment(OutputString);
		}

//460		525		REF		Ordering Provider Secondary Information	S		5

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//in 5010, only 0B, 1G, G2, and LU are valid, but we continue to let them set up anything they want

		OutputString = "REF";
		
		//REF01	128			Reference Identification Qualifier		M 1	ID	2/3

		// (j.jones 2013-06-03 12:24) - PLID 54091 - Overrides are rarely needed anymore, but if they need
		// an override in 2420A, use the same override here, such that 2420E is essentially a copy of 2420A.
		// We do not currently need a unique override for 2420E.
		CString strIdentifier = "", strID = "", strLoadedFrom = "";
		EBilling_Calculate2420A_REF(strIdentifier, strID, strLoadedFrom, m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->HCFASetupID,
			m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->InsuredPartyID, m_pEBillingInfo->BillLocation);

		//if the qualifier is XX, send nothing
		if(m_bDisableREFXX && strIdentifier.CompareNoCase("XX") == 0) {
			strIdentifier = "";
			strID = "";
			//log that this happened
			Log("Ebilling file tried to export 2420E REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->BillID);
		}
		
		OutputString += ParseANSIField(strIdentifier,2,3);

		//REF02	127			Reference Identification				X 1	AN	1/50

		OutputString += ParseANSIField(strID,1,50);

		//REF03	NOT USED
		OutputString += "*";
		//REF04	NOT USED
		OutputString += "*";

		//do not send this segment if the qualifier or ID are blank
		if(strIdentifier != "" && strID != "") {
			EndANSISegment(OutputString);
		}

//462		530		PER		Ordering Provider Contact Information	S		1
		// (b.spivey January 26, 2015) - PLID 64452 - Option to not send PER
		if (m_HCFAInfo.SendN3N4PERSegment) {
			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "PER";

			//PER01	366			Contact Function Code					M 1	ID	2/2

			//static "IC";
			OutputString += ParseANSIField("IC", 2, 2);

			//PER02	93			Name									O 1	AN	1/60

			//PersonT.First + PersonT.Last
			str = GetFieldFromRecordset(rsDoctorInfo, "First") + " " + GetFieldFromRecordset(rsDoctorInfo, "Last");
			str.TrimLeft();
			str.TrimRight();
			str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
			OutputString += ParseANSIField(str, 1, 60);

			//PER03	365			Communication Number Qualifier			X 1	ID	2/2

			//static "TE"
			str = GetFieldFromRecordset(rsDoctorInfo, "WorkPhone");
			str = StripNonNum(str);
			if (str != "")
				OutputString += ParseANSIField("TE", 2, 2);
			else
				OutputString += "*";

			//PER04	364			Communication Number					X 1	AN	1/256

			//PersonT.WorkPhone
			if (str != "")
				OutputString += ParseANSIField(str, 1, 256);
			else
				OutputString += "*";

			//PER05	365			Communication Number Qualifier			X 1	ID	2/2		

			//output the fax number, if it exists
			CString strPER05Qual = "FX";
			CString strPER06ID = GetFieldFromRecordset(rsDoctorInfo, "Fax");
			strPER06ID = StripNonNum(strPER06ID);

			//do not output anything if the ID is blank
			strPER06ID.TrimLeft();
			strPER06ID.TrimRight();
			if (strPER06ID.IsEmpty())
				strPER05Qual = "";

			OutputString += ParseANSIField(strPER05Qual, 2, 2);

			//PER06	364			Communication Number					X 1	AN	1/256

			// PersonT.Fax
			OutputString += ParseANSIField(strPER06ID, 1, 256);

			//PER07	365			Communication Number Qualifier			X 1	ID	2/2

			OutputString += "*";

			//PER08	364			Communication Number					X 1	AN	1/256

			OutputString += "*";

			//PER09	NOT USED

			//if the segment is still only PER*IC, then don't bother exporting anything
			CString strTest = OutputString;
			strTest.TrimRight("*");
			if (strTest != "PER*IC") {
				EndANSISegment(OutputString);
			}
		}

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2420E_Prof");

	return Error_Other;
}

// (j.jones 2014-04-23 14:51) - PLID 61834 - supported 2420F
int CEbilling::ANSI_5010_2420F_Prof(long nPersonID) {

	//Referring Provider Name

	try {

		CString str;
		// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
		if (m_bHumanReadableFormat) {

				str = "\r\n2420F\r\n";
				m_OutputFile.Write(str,str.GetLength());
		}

		CString OutputString;
		_variant_t var;

		//this can be either a provider or a referring physician
		_RecordsetPtr rsDoctorInfo = CreateParamRecordset("SELECT Last, First, Middle, Title, "
			"CASE WHEN ProvidersT.PersonID Is Not Null THEN ProvidersT.NPI ELSE ReferringPhysT.NPI END AS NPI, "
			"Convert(bit, CASE WHEN ProvidersT.PersonID Is Not Null THEN 1 ELSE 0 END) AS IsProvider "
			"FROM PersonT "
			"LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
			"LEFT JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
			"WHERE PersonT.ID = {INT} "
			"AND (ProvidersT.PersonID Is Not Null OR ReferringPhysT.PersonID Is Not Null)", nPersonID);

		if(rsDoctorInfo->eof) {
			//if the recordset is empty, there is no doctor. So halt everything!!!
			rsDoctorInfo->Close();
			CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
			if(!strPatientName.IsEmpty()) {
				str.Format("Could not open referring provider information for patient '%s', Bill ID %li. (ANSI_2420F)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open referring provider information. (ANSI_2420F)"
					"\nIt is possible that you have no referring provider selected on this patient's bill.";
			
			AfxMessageBox(str);
			return Error_Missing_Info;
		}

		FieldsPtr doctorFields = rsDoctorInfo->Fields;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2420F	- REFERRING PROVIDER NAME						1

//465		500		NM1		Referring Provider Name					S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M  1	ID	2/3

		//"DN" - Referring Physician
		//"P3" - Primary Care Provider

		//we only support DN
		OutputString += ParseANSIField("DN",2,3);

		//NM102	1065		Entity Type Qualifier					M 1	ID	1/1

		//static value "1" for person, "2" is not a valid value in this loop
		OutputString += ParseANSIField("1", 1, 1);

		//NM103	1035		Name Last or Organization Name			O 1	AN	1/60

		//PersonT.Last
		str = GetFieldFromRecordset(rsDoctorInfo, "Last");
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,60);

		//NM104	1036		Name First								O 1	AN	1/35

		//PersonT.First

		str = GetFieldFromRecordset(rsDoctorInfo, "First");
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,35);

		//NM105	1037		Name Middle								O 1	AN	1/25

		//PersonT.Middle
		str = GetFieldFromRecordset(rsDoctorInfo, "Middle");
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,25);

		//NM106	NOT USED
		OutputString += "*";

		//NM107	1039		Name Suffix								O 1	AN	1/10

		//PersonT.Title
		str = GetFieldFromRecordset(rsDoctorInfo, "Title");
		OutputString += ParseANSIField(str,1,10);

		//NM108	66			Identification Code	Qualifier			X 1	ID	1/2

		//always XX - NPI in 5010
		str = GetFieldFromRecordset(rsDoctorInfo, "NPI");
		// (j.jones 2015-02-18 14:04) - PLID 56515 - changed StripNonNum to StripSpacesAndHyphens,
		// although NPI should always be numeric
		str = StripSpacesAndHyphens(str);
		
		if(str != "")
			OutputString += ParseANSIField("XX",1,2);
		else
			OutputString += "*";

		//NM109	67			Identification Code						X	AN	2/80

		if(str != "")
			OutputString += ParseANSIField(str,2,80);
		else
			OutputString += "*";

		//NM110 NOT USED
		//NM111	NOT USED
		//NM112	NOT USED

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//468		5250	REF		Referring Provider Secondary Information	S		5

		//we do not support REF in this loop

		//only 0B, 1G, and G2 are valid

		OutputString = "REF";
		
		//REF01	128			Reference Identification Qualifier		M 1	ID	2/3

		CString strIdentifier = "";
		OutputString += ParseANSIField(strIdentifier,2,3);

		//REF02	127			Reference Identification				X 1	AN	1/50

		CString strID = "";
		OutputString += ParseANSIField(strID,1,50);

		//REF03	NOT USED
		OutputString += "*";
		//REF04	NOT USED
		OutputString += "*";

		//do not send this segment if the qualifier or ID are blank
		if(strIdentifier != "" && strID != "") {
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2420F_Prof");

	return Error_Other;
}

/*
int CEbilling::ANSI_5010_2310H_Prof() {

	//Ambulance Drop-Off Location

	//Required when billing for ambulance or non-emergency transportation services.

	// (j.jones 2010-10-14 16:15) - we currently do not support this

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310H - AMBULANCE PICK-UP LOCATION						1

//475		2500	NM1		Ambulance Drop-off Location					S	1
//477		2650	N3		Ambulance Drop-off Location Address			R	1
//478		2700	N4		Ambulance Drop-off Location City, State, ZIP Code	R	1

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2310H_Prof");

	return Error_Other;
}

*/

// (j.jones 2008-05-23 10:48) - PLID 29084 - added parameters for allowables
// (j.jones 2010-03-31 15:17) - PLID 37918 - added parameter for total count of charges being submitted
// (j.jones 2012-03-21 14:18) - PLID 48870 - this now requires an OtherInsuranceInfo struct
int CEbilling::ANSI_5010_2430(OtherInsuranceInfo oInfo, _RecordsetPtr &rsCharges, BOOL bSentAllowedAmount, COleCurrency cyAllowableSent, long nCountOfCharges) {

	//Line Adjudication Information
	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\n2430\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2430 - LINE ADJUDICATION INFORMATION					15

//480		540		SVD		Line Adjudication Information			S		1

		// (j.jones 2006-11-24 15:20) - PLID 23415 - use the payment information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending adjustment information in this loop.
		// The reason is that payments are optional here unless adjustments are
		// present, in which case the payments have to be here to balance against
		// the adjustments.

		// (j.jones 2006-11-27 17:21) - PLID 23652 - supported this on the UB92

		COleCurrency cyAmtPaid = COleCurrency(0,0);

		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(!m_pEBillingInfo->IsPrimary &&
			((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1 && m_HCFAInfo.ANSI_AdjustmentLoop == 1)
			|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1 && m_UB92Info.ANSI_AdjustmentLoop == 1))) {

			// (j.jones 2006-11-21 13:55) - PLID 23415 - shows payment information per charge
			// (j.jones 2006-11-27 17:21) - PLID 23652 - this is slightly different for the UB92

			//we need the total payment amount for this charge by the OthrInsurance on the claim,
			//the sum should return 0 if there are no payments

			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			// (j.jones 2011-08-16 17:22) - PLID 44805 - We will filter out "original" and "void" charges & applies.
			// The applies part of this does not need to check LineItemCorrectionsBalancingAdjT,
			// because they are only applied to original and void charges.
			_RecordsetPtr rsPayTotal = CreateParamRecordset("SELECT Sum(Coalesce(Amount,0)) AS PaymentAmount "
				"FROM AppliesT "
				"WHERE DestID = {INT} "
				"AND SourceID IN (SELECT LineItemT.ID FROM LineItemT "
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"	WHERE Deleted = 0 "
				"	AND Type = 1 "
				"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"	AND LineItemT.ID IN (SELECT ID FROM PaymentsT WHERE InsuredPartyID = {INT})) ",
				AdoFldLong(rsCharges, "ChargeID"), oInfo.nInsuredPartyID);

			if(!rsPayTotal->eof) {

				OutputString = "SVD";

				//Ref.	Data		Name									Attributes
				//Des.	Element

				//SVD01 67			Identification Code						M 1	AN	2/80

				//has to be identical to NM109 from 2330B
				
				// (j.jones 2009-08-05 10:20) - PLID 34467 - this is now in the loaded claim info
				// (j.jones 2009-12-16 15:54) - PLID 36237 - split out into HCFA and UB payer IDs
				// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType == actInst) {
					str = oInfo.strUBPayerID;
				}
				else {
					str = oInfo.strHCFAPayerID;
				}

				// (b.spivey, May 07, 2013) - PLID 46573 - trimspaces, that's the same as being empty. 
				str = str.Trim(" ");

				// (j.jones 2008-05-06 11:48) - PLID 29937 - cached m_bUseTHINPayerIDs and m_bPrependTHINPayerNSF
				// (j.jones 2009-08-04 11:34) - PLID 14573 - renamed to PrependTHINPayerNSF to PrependPayerNSF,
				// and removed m_bUseTHINPayerIDs
				
				//see if we need to prepend the NSF code
				if(m_bPrependPayerNSF) {
					
					// (j.jones 2008-09-09 10:18) - PLID 18695 - We converted the NSF Code to InsType,
					// but this code actually needs ye olde NSF Code. I am pretty sure this is obsolete
					// because THIN no longer exists, but for the near term let's calculate ye olde NSF code.

					InsuranceTypeCode eInsType = (InsuranceTypeCode)AdoFldLong(rsCharges, "InsType", (long)itcInvalid);
					CString strNSF = GetNSFCodeFromInsuranceType(eInsType);
					if(strNSF.IsEmpty()) {
						strNSF = "Z";
					}
					
					str = strNSF + str;
				}

				// (j.jones 2010-08-30 17:12) - PLID 15025 - supported TPL Code,
				// we already loaded the HCFA or UB-specific setting into nSendTPLCode,
				// we need to override the payer ID only if configured to do so AND the TPL
				// is non-empty
				//1 - do not send
				//2 - send both
				//3 - send 2330B only
				//4 - send 2430 only
				if(m_pEBillingInfo->nSendTPLCode == 2 || m_pEBillingInfo->nSendTPLCode == 4) {
					CString strTPLCode = oInfo.strTPLCode;
					strTPLCode.TrimLeft();
					strTPLCode.TrimRight();
					if(!strTPLCode.IsEmpty()) {
						str = strTPLCode;
					}
				}

				OutputString += ParseANSIField(str, 2, 80);

				//SVD02 782			Monetary Amount							M 1	R	1/18

				COleCurrency cyAmt = COleCurrency(0,0);

				//add up the payment amount so we know how much to adjust, but then
				//only actually show the payment information if the option is enabled, else zero				
				cyAmt = AdoFldCurrency(rsPayTotal, "PaymentAmount", COleCurrency(0,0));
				//store this for later, for the adjustments
				cyAmtPaid += cyAmt;

				str = FormatCurrencyForInterface(cyAmt, FALSE, FALSE);
				//see if we need to trim the zeros
				// (j.jones 2012-05-25 16:29) - PLID 50657 - now a modular function, truncates if m_bTruncateCurrency is enabled
				str = FormatANSICurrency(str);
				OutputString += ParseANSIField(str, 1, 18);

				//SVD03 C003		COMPOSITE MEDICAL PROCEDURE IDENTIFIER	O 1

				//SVD03-1	235		Product/Service ID Qualifier			M	ID	2/2

				//ER - Jurisdiction Specific Procedure and Supply Codes
				//HC - Health Care Financing Administration Common Procedural Coding System (HCPCS) Codes
				//IV - Home Infusion EDI Coalition (HIEC) Product/Service Code
				//WK - Advanced Billing Concepts (ABC) Codes

				//we use HC
				// (j.jones 2007-07-27 09:17) - PLID 26839 - changed so we don't export
				// the HC composite segment if we don't have a CPT code, UB claims only

				CString strQual = "HC";

				// (j.jones 2013-01-25 12:25) - PLID 54853 - IsValidServiceCodeQualifier
				// needs updated any time we support exporting a new qualifier type,
				// currently we only support HC. This ensures that we never send other 
				// qualifiers without updating this function.
				if(!IsValidServiceCodeQualifier(strQual)) {
					//You just tried to send a qualifier not supported by IsValidServiceCodeQualifier,
					//Go into that function and add your new qualifier to the supported list.
					ASSERT(FALSE);
					ThrowNxException("Loop 2430 tried to send invalid service qualifier %s.", strQual);			
				}

				CString strCPTOut = ParseANSIField(strQual,2,2);
				BOOL bUseCPT = FALSE;

				//if a HCFA, we have to send it
				// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType != actInst)
					bUseCPT = TRUE;

				//SVD03-2	234		Product/Service ID						M	AN	1/48

				//ChargesT.ItemCode
				str = GetFieldFromRecordset(rsCharges,"ItemCode");
				if(str != "") {
					//now we will output this line
					bUseCPT = TRUE;

					strCPTOut += ":";
					strCPTOut += str;
				}

				//SVD03-3	1339	Procedure Modifier						O	AN	2/2

				//ChargesT.CPTModifier
				str = GetFieldFromRecordset(rsCharges,"CPTModifier");
				if(str != "") {
					strCPTOut += ":";
					strCPTOut += str;
				}

				//SVD03-4	1339	Procedure Modifier						O	AN	2/2

				//ChargesT.CPTModifier2
				str = GetFieldFromRecordset(rsCharges,"CPTModifier2");
				if(str != "") {
					strCPTOut += ":";
					strCPTOut += str;
				}

				//SVD03-5	1339	Procedure Modifier						O	AN	2/2

				//ChargesT.CPTModifier3
				str = GetFieldFromRecordset(rsCharges,"CPTModifier3");
				if(str != "") {
					strCPTOut += ":";
					strCPTOut += str;
				}

				//SVD03-6	1339	Procedure Modifier						O	AN	2/2

				//ChargesT.CPTModifier4
				str = GetFieldFromRecordset(rsCharges,"CPTModifier4");
				if(str != "") {
					strCPTOut += ":";
					strCPTOut += str;
				}

				if(bUseCPT) {
					OutputString += strCPTOut;
				}
				else {
					OutputString += "*";
				}

				//SVD03-7	352		Description								O	AN	1/80

				//A free-form description to clarify the related data elements and their content
				
				//Required when, in the judgment of the submitter, the Procedure Code does not definitively
				//describe the service/product/supply and loop 2410 is not used.

				//we don't currently support this

				//SVD03-8 NOT USED

				//SVD04		234		Product/Service ID						X 1	AN	1/48

				str = "";

				//the revenue code, UB only
				if(m_actClaimType == actInst) {

					// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
					_RecordsetPtr rsRevCode = CreateParamRecordset("SELECT (CASE WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Code ELSE UB92CategoriesT1.Code END) AS Code "
								"FROM ServiceT "
								"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
								"LEFT JOIN (SELECT ServiceRevCodesT.* FROM ServiceRevCodesT "
								"	WHERE InsuranceCompanyID IN (SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = {INT})) AS ServiceRevCodesT ON ServiceT.ID = ServiceRevCodesT.ServiceID "
								"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
								"WHERE ServiceT.ID = {INT}", oInfo.nInsuredPartyID, AdoFldLong(rsCharges, "ServiceID",-1));

					if(!rsRevCode->eof) {
						str = AdoFldString(rsRevCode, "Code","");

						// (j.jones 2007-05-02 10:37) - PLID 25855 - If the option to
						// send 4-digit revenue codes is enabled, prepend with zeros,
						// such that "360" becomes "0360". Don't bother doing this
						// if the code is blank.
						// Note: technically, the code "could" be greater than 4 digits,
						// there's nothing in the program or the ANSI format that would
						// stop you from making one, but it would be an invalid code.
						// So just expand to 4, don't truncate from 4.
						str.TrimLeft();
						str.TrimRight();
						if(m_bFourDigitRevenueCode && !str.IsEmpty() && str.GetLength() < 4) {
							while(str.GetLength() < 4) {
								str = "0" + str;
							}
						}
					}
					else {
						str = "";
					}					
					rsRevCode->Close();
				}
				
				OutputString += ParseANSIField(str,1,48);

				//SVD05 380			Quantity								O 1	R	1/15

				//copied from the 2400 loop

				//ChargesT.Quantity
				var = rsCharges->Fields->Item["Quantity"]->Value;
				if(var.vt == VT_R8)
					str.Format("%g",var.dblVal);
				else
					str = "1";

				// (j.jones 2007-10-15 15:05) - PLID 27757 - track the ServiceID
				long nServiceID = AdoFldLong(rsCharges, "ServiceID");

				// (j.jones 2008-05-02 10:50) - PLID 28012 - removed the requirement for UseAnesthesiaBilling
				// to be checked for the service, and for a flat-fee
				if(m_HCFAInfo.ANSI_UseAnesthMinutesAsQty == 1 && AdoFldBool(rsCharges, "Anesthesia", FALSE)
					/*&& AdoFldBool(rsCharges, "UseAnesthesiaBilling", FALSE)
					// (j.jones 2007-10-15 14:57) - PLID 27757 - converted to use the new structure
					&& ReturnsRecords("SELECT ID FROM AnesthesiaSetupT WHERE ServiceID = %li AND LocationID = %li AND AnesthesiaFeeBillType <> 1", nServiceID, m_pEBillingInfo->POS)*/) {

					//they want to show minutes, and this is indeed an anesthesia charge that is based on time
					str.Format("%li",AdoFldLong(rsCharges, "AnesthesiaMinutes", 0));
				}

				OutputString += ParseANSIField(str,1,15);

				//SVD06 554			Assigned Number							O 1	N0	1/6

				//has to be the same number used in the LX portion of the 2400 loop

				// (j.jones 2010-03-31 15:19) - PLID 37918 - added the ANSI_Hide2430_SVD06_OneCharge
				// setting, which if enabled will cause this field to be blank if there is only
				// one charge being submitted on the claim
				// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(nCountOfCharges == 1 && 
					((m_actClaimType != actInst && m_HCFAInfo.ANSI_Hide2430_SVD06_OneCharge == 1)
					|| (m_actClaimType == actInst && m_UB92Info.ANSI_Hide2430_SVD06_OneCharge == 1))) {

					str = "";
				}
				else {
					//m_ANSIServiceCount
					str.Format("%li",m_ANSIServiceCount);
				}

				OutputString += ParseANSIField(str, 1, 6);

				EndANSISegment(OutputString);
			}
			rsPayTotal->Close();
		}

//484		545		CAS		Line Adjustment							S		5

		// (j.jones 2006-11-24 15:20) - PLID 23415 - use the adjustment information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending adjustment information in this loop

		COleCurrency cyAmtAdjusted = COleCurrency(0,0);

		// (j.jones 2006-11-27 17:23) - PLID 23652 - supported this on the UB92
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum

		if(!m_pEBillingInfo->IsPrimary &&
			((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1 && m_HCFAInfo.ANSI_AdjustmentLoop == 1)
			|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1 && m_UB92Info.ANSI_AdjustmentLoop == 1))) {

			// (j.jones 2006-11-21 10:11) - PLID 23415, 23652 - supported sending adjustment information

			//this has to balance against the payments above and the total charge amount in 2400 (LineTotal)

			// (j.jones 2009-03-11 10:35) - PLID 33446 - load the default ins. adj. group/reason codes
			CString strDefaultInsAdjGroupCode = "CO", strDefaultInsAdjReasonCode = "45";
			if(m_actClaimType == actInst) {
				strDefaultInsAdjGroupCode = m_UB92Info.ANSI_DefaultInsAdjGroupCode;
				strDefaultInsAdjReasonCode = m_UB92Info.ANSI_DefaultInsAdjReasonCode;
			}
			else {
				strDefaultInsAdjGroupCode = m_HCFAInfo.ANSI_DefaultInsAdjGroupCode;
				strDefaultInsAdjReasonCode = m_HCFAInfo.ANSI_DefaultInsAdjReasonCode;
			}

			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			// (j.jones 2009-03-11 10:31) - PLID 33446 - we now include all adjustments, and use the HCFA/UB group's
			// default for insurance adjustments if the group or reason code is blank
			// (j.jones 2010-09-23 15:48) - PLID 40653 - group & reason codes are now IDs
			// (j.jones 2011-08-16 17:22) - PLID 44805 - We will filter out "original" and "void" charges & applies.
			// The applies part of this does not need to check LineItemCorrectionsBalancingAdjT,
			// because they are only applied to original and void charges.
			_RecordsetPtr rsAdj = CreateParamRecordset("SELECT "
				"CASE WHEN AdjustmentGroupCodesT.Code Is Null THEN {STRING} ELSE AdjustmentGroupCodesT.Code END AS GroupCode, "
				"CASE WHEN AdjustmentReasonCodesT.Code Is Null THEN {STRING} ELSE AdjustmentReasonCodesT.Code END AS ReasonCode, "
				"AppliedQ.AdjAmount "
				"FROM PaymentsT "
				"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"INNER JOIN (SELECT Sum(Amount) AS AdjAmount, SourceID "
				"	FROM AppliesT WHERE DestID = {INT} "
				"	GROUP BY SourceID) AS AppliedQ ON PaymentsT.ID = AppliedQ.SourceID "
				"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID "
				"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 2 "
				"AND PaymentsT.InsuredPartyID <> -1 "
				"AND PaymentsT.InsuredPartyID = {INT} "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"ORDER BY AdjustmentGroupCodesT.Code ", strDefaultInsAdjGroupCode, strDefaultInsAdjReasonCode,
				AdoFldLong(rsCharges, "ChargeID"), oInfo.nInsuredPartyID);

			OutputString = "";
			
			// (j.jones 2008-11-12 13:34) - PLID 31740 - track all the CAS segments we wish to add
			CArray<ANSI_CAS_Info*, ANSI_CAS_Info*> aryCASSegments;

			while(!rsAdj->eof) {

				// (j.jones 2006-11-21 10:33) - PLID 23415, 23652 - we can send up to 99
				// CAS segments, each segment has one group code and up to 6 adjustments
				// using that code

				// (j.jones 2008-11-12 13:25) - PLID 31740 - we now use AddCASSegmentToMemory and OutputCASSegments
				// to properly handle our CAS segments

				COleCurrency cyAmt = AdoFldCurrency(rsAdj, "AdjAmount", COleCurrency(0,0));

				AddCASSegmentToMemory(AdoFldString(rsAdj, "GroupCode",""),
					AdoFldString(rsAdj, "ReasonCode",""),
					cyAmt, aryCASSegments);

				//calculate the total "real" adjustments
				cyAmtAdjusted += cyAmt;

				rsAdj->MoveNext();
			}
			rsAdj->Close();

			//export anything left over
			if(!OutputString.IsEmpty()) {
				EndANSISegment(OutputString);
			}

			//if we sent payments in the charge level, then this has to balance against the LineTotal in 2400,
			//so use cyAmtAdjusted and cyAmtPaid to determine if more adjustments are needed.
			//If we have no payments at all, we have to act like everything was adjusted
			
			COleCurrency cyLineTotal = AdoFldCurrency(rsCharges, "LineTotal", COleCurrency(0,0));

			COleCurrency cyDiff = cyLineTotal - cyAmtPaid - cyAmtAdjusted;
			if(cyDiff > COleCurrency(0,0)) {

				//we need to artificially adjust cyDiff
				
				// (j.jones 2008-05-23 09:59) - PLID 29084 - if an allowed amount was sent (AAE),
				// then we need to split the adjustments such that the "other adjustment" (OA, or the default)
				// is the difference between the allowed amount and the charge total, and the 
				// "patient responsibility" (PR) is the difference between the paid amount and the allowed amount
				// This does NOT use the patient resp. in Practice.
				
				// (j.jones 2009-06-23 10:50) - PLID 34693 - This logic has changed slightly, but the above description
				// is still accurate. We previously calculated the OA to be the difference between the allowed and
				// the balance, and the PR was the difference between paid and allowed, if there was a remaining balance.
				// This was wrong. We need to FIRST calculate the PR, then the OA is whatever the remaining balance is.

				// (j.jones 2010-02-03 09:32) - PLID 37170 - if the allowed amount was reported with included adjustments,
				// those adjustment totals are NOT included in cyAllowableSent, so that cyPatientAdj is calculated based off
				// of the pre-adjustment allowable total

				COleCurrency cyAllowedAmount = COleCurrency(0,0);
				if(bSentAllowedAmount) {
					cyAllowedAmount = cyAllowableSent;
				}

				COleCurrency cyPatientAdj = COleCurrency(0,0), cyNonPatientAdj = COleCurrency(0,0);

				// (j.jones 2009-06-22 17:47) - PLID 34693 - the patient adj. should be the difference
				// between paid and allowed, ignoring what was adjusted
				//if((cyAllowedAmount - cyAmtPaid - cyAmtAdjusted) > COleCurrency(0,0)) {
				if((cyAllowedAmount - cyAmtPaid) > COleCurrency(0,0)) {
					//the "patient resp" should be the difference between the amount paid,
					//and the allowable, if there is a difference at all
					//cyPatientAdj = (cyAllowedAmount - cyAmtPaid - cyAmtAdjusted);
					cyPatientAdj = (cyAllowedAmount - cyAmtPaid);
				}

				// (j.jones 2009-06-23 10:53) - PLID 34693 - the OA is now whatever the remaining balance is
				// after the PR is calculated
				if((cyDiff - cyPatientAdj) > COleCurrency(0,0)) {
					//the "other adjustment" balances out the charge, taking the patient adjustment into account
					cyNonPatientAdj = (cyDiff - cyPatientAdj);
				}

				ASSERT(cyLineTotal == (cyAmtPaid + cyAmtAdjusted + cyNonPatientAdj + cyPatientAdj));

				// (j.jones 2008-11-12 11:25) - PLID 31912 - do not send the PR segment unless the
				// setting tells us to do so
				// (j.jones 2009-08-28 16:44) - PLID 35006 - now we always send the PR segment if we
				// have a value to send, the bANSI_SendCASPR setting now simply controls whether we
				// send it even if the value is zero
				// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
				BOOL bAlwaysSendPRSegment = ((m_actClaimType != actInst && m_HCFAInfo.bANSI_SendCASPR) || (m_actClaimType == actInst && m_UB92Info.bANSI_SendCASPR));

				COleCurrency cyRegularAdjustmentAmountToSend = cyNonPatientAdj;
				COleCurrency cyPRAdjustmentAmountToSend = cyPatientAdj;

				// (j.jones 2009-08-28 16:46) - PLID 35006 - always send PR, or at least try to
				// (PR is not sent if a UB claim)
				// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType == actInst) {
					//if not sending PR, send the full adjustable value
					cyRegularAdjustmentAmountToSend += cyPatientAdj;
					cyPRAdjustmentAmountToSend = COleCurrency(0,0);
				}

				if(cyRegularAdjustmentAmountToSend > COleCurrency(0,0)) {
	
					// (j.jones 2007-06-15 11:46) - PLID 26309 - Now we have a setting that lets the user
					// configure a default group code.
					// (j.jones 2009-03-11 10:39) - PLID 33446 - We added a different setting for a default
					// for "real" adjustments, as opposed to this existing setting for "fake" adjustments.
					// This setting has been renamed to ANSI_DefaultRemAdjGroupCode/ReasonCode.
					//default to OA for other adjustment
					// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
					CString strGroupCode = "OA";
					if(m_actClaimType == actInst && m_UB92Info.ANSI_DefaultRemAdjGroupCode != "")
						strGroupCode = m_UB92Info.ANSI_DefaultRemAdjGroupCode;
					else if(m_actClaimType != actInst && m_HCFAInfo.ANSI_DefaultRemAdjGroupCode != "")
						strGroupCode = m_HCFAInfo.ANSI_DefaultRemAdjGroupCode;
					
					// (j.jones 2007-04-26 09:55) - PLID 25802 - Use 42 because it is the
					// answer to life, the universe, and everything... AND also the ideal and
					// most common reason code: "Charges exceed our fee schedule or maximum allowable amount."
					// (j.jones 2007-06-13 11:17) - PLID 26307 - You have to be kidding me. Right after
					// clearinghouses forced me to default to 42, it EXPIRED and is no longer a valid code.
					// Use 2, "Co-Insurance", though a pending PL item will make this configurable.
					// (j.jones 2007-06-15 11:46) - PLID 26309 - Now we have a setting that lets the user
					// configure a default.
					// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
					CString strReasonCode = "2";
					if(m_actClaimType == actInst && m_UB92Info.ANSI_DefaultRemAdjReasonCode != "")
						strReasonCode = m_UB92Info.ANSI_DefaultRemAdjReasonCode;
					else if(m_actClaimType != actInst && m_HCFAInfo.ANSI_DefaultRemAdjReasonCode != "")
						strReasonCode = m_HCFAInfo.ANSI_DefaultRemAdjReasonCode;
					
					// (j.jones 2008-11-12 13:25) - PLID 31740 - we now use AddCASSegmentToMemory and OutputCASSegments
					// to properly handle our CAS segments
					AddCASSegmentToMemory(strGroupCode, strReasonCode, cyRegularAdjustmentAmountToSend, aryCASSegments);
				}

				// (j.jones 2008-05-23 10:10) - PLID 29084 - now report the patient resp
				// (j.jones 2008-11-12 11:30) - PLID 31912 - only if bSendPRSegment is true
				// (j.jones 2009-08-28 16:46) - PLID 35006 - now we always send PR if we have
				// an amount to send, or send zero if bANSI_SendCASPR is TRUE
				// (PR is not sent if a UB claim)
				// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType != actInst && (bAlwaysSendPRSegment || cyPRAdjustmentAmountToSend > COleCurrency(0,0))) {

					// (j.jones 2008-11-12 13:25) - PLID 31740 - we now use AddCASSegmentToMemory and OutputCASSegments
					// to properly handle our CAS segments

					//default group code to PR for patient responsibility
					//default reason code to 2
					// (j.jones 2009-08-28 16:57) - PLID 35006 - sending zero is only allowed
					// if bAlwaysSendPRSegment is true
					AddCASSegmentToMemory("PR", "2", cyPRAdjustmentAmountToSend, aryCASSegments, bAlwaysSendPRSegment);
				}
			}

			//if cyDiff is less than zero, that would mean they have more applied than
			//the charge is for, which should be impossible

			// (j.jones 2008-11-12 13:30) - PLID 31740 - now output our CAS segments, if we have any
			ANSI_5010_OutputCASSegments(aryCASSegments);
		}

//490		550		DTP		Line Check Or Remittance Date				R		1

		// (j.jones 2006-11-24 15:20) - PLID 23415 - use the date information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending adjustment information in this loop

		// (j.jones 2006-11-27 17:23) - PLID 23652 - supported this on the UB92
		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum

		if(!m_pEBillingInfo->IsPrimary &&
			((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1 && m_HCFAInfo.ANSI_AdjustmentLoop == 1)
			|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1 && m_UB92Info.ANSI_AdjustmentLoop == 1))) {

			// (j.jones 2006-11-21 10:44) - PLID 23415, 23652 - output the date the claim was paid by insurance

			//since we send payments and adjustments in this loop, just find the first
			//payment or adjustment date (in theory, should all be the same date)
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			// (j.jones 2011-08-16 17:22) - PLID 44805 - We will filter out "original" and "void" charges & applies.
			// The applies part of this does not need to check LineItemCorrectionsBalancingAdjT,
			// because they are only applied to original and void charges.
			_RecordsetPtr rsDates = CreateParamRecordset("SELECT TOP 1 LineItemT.Date "
				"FROM PaymentsT "
				"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "	
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND LineItemT.Type IN (1,2) "
				"AND PaymentsT.ID IN (SELECT SourceID FROM AppliesT WHERE DestID = {INT}) "
				"AND PaymentsT.InsuredPartyID <> -1 "
				"AND PaymentsT.InsuredPartyID = {INT} "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"GROUP BY LineItemT.Date " //unique dates only
				"ORDER BY LineItemT.Date ",	
				AdoFldLong(rsCharges, "ChargeID"), oInfo.nInsuredPartyID);

			//it is possible we may have no actual applied item, so send today's date otherwise
			COleDateTime dtDate = COleDateTime::GetCurrentTime();
			
			if(!rsDates->eof) {
				dtDate = AdoFldDateTime(rsDates, "Date");
			}
			rsDates->Close();

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "DTP";

			//DTP01	374			Date/Time Qualifier						M 1	ID	3/3

			//573 - Date Claim Paid

			str = "573";
			OutputString += ParseANSIField(str, 3, 3);

			//DTP02	1250		Date Time Period Format Qualifier		M 1	ID	2/3

			//D8 - date is CCYYMMDD

			str = "D8";
			OutputString += ParseANSIField(str, 2, 3);

			//DTP03	1251		Date Time Period						M 1	AN	1/35

			//LineItemT.Date

			str = dtDate.Format("%Y%m%d");
			OutputString += ParseANSIField(str, 1, 35);

			EndANSISegment(OutputString);
		}
		
///////////////////////////////////////////////////////////////////////////////

//491		550		ANT		Remaining Patient Liability				S		1

		//we don't support this

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2430");

	return Error_Other;
}

/*
int CEbilling::ANSI_5010_2440() {

	//Form Identification Code

	//JJ - I don't believe this is needed.

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2440 - FORM IDENTIFICATION CODE						>1

//492		551		LQ		Form Identification Code				S		1
//494		552		FRM		Supporting Documentation				R		99

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_2440");

	return Error_Other;
}
*/

int CEbilling::ANSI_5010_Trailer() {

	//Trailer
	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\nTRAILER\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							TRAILER

//496		555		SE		Transaction Set Trailer					R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "SE";		

		//SE01	96			Number Of Included Segments				M 1	N0	1/10

		//m_ANSISegmentCount

		//DRT - I changed this from incrementing the count, to just printing 1 higher
		//		otherwise it would increment here, and then it would again increment
		//		in the EndANSISegment().  While in all cases so far the value doesn't 
		//		matter past this point, you never know when it will, so I figure this
		//		was a little more robust.
		str.Format("%li",m_ANSISegmentCount + 1);
		OutputString += ParseANSIField(str,1,10);

		//SE02	329			Transaction Set Control Number			M 1	AN	4/9

		//m_strBatchNumber
		OutputString += ParseANSIField(m_strBatchNumber,4,9,TRUE,'R','0');

		EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_Trailer");

	return Error_Other;
}

int CEbilling::ANSI_5010_InterchangeHeader() {

	//Interchange Control Header (page C.3 - or 637)

	//The Interchange Control Header is one of the few records with a required length limit.
	//As a result, each element will be filled to a set size, even if the entire element is blank.
	//This will be the only time we really need to se bForceFill to TRUE in the ParseANSIField() function.
	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\nInterchange Control Header\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	try {

		// (j.jones 2008-05-06 11:49) - PLID 29937 - removed the EbillingFormatsT recordset, as everything in it is cached now

		CString OutputString,str;
		_variant_t var;

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "ISA";

		//ISA01	I01			Authorization Information Qualifier		M 1	ID	2/2

		//00 and 03 are the only valid values

		CString strISA01Qual = m_strISA01Qual;

		//EbillingFormatsT.ISA01Qual
		OutputString += ParseANSIField(strISA01Qual,2,2,TRUE);

		//ISA02	I02			Authorization Information				M 1	AN	10/10
		
		//(I01 and I02 are advised to be 00 and empty for now)
		CString strISA02 = m_strISA02;

		//EbillingFormatsT.ISA02
		OutputString += ParseANSIField(strISA02,10,10,TRUE);

		//ISA03	I03			Security Information Qualifier			M 1	ID	2/2

		//00 and 01 are the only valid values

		CString strISA03Qual = m_strISA03Qual;

		//EbillingFormatsT.ISA03Qual
		OutputString += ParseANSIField(strISA03Qual,2,2,TRUE);

		//ISA04	I04			Security Information					M 1	AN	10/10

		//(I03 and I04 are advised to be 00 and empty for now)
		CString strISA04 = m_strISA04;

		//EbillingFormatsT.ISA04
		OutputString += ParseANSIField(strISA04,10,10,TRUE);

		//ISA05	I05			Interchange ID Qualifier				M 1	ID	2/2

		//default "ZZ" ("ZZ" is "Mutually Defined")
		CString strSubmitterQual = m_strSubmitterISA05Qual;
		OutputString += ParseANSIField(strSubmitterQual,2,2,TRUE);

		//ISA06	I06			Interchange Sender ID					M 1	AN	15/15

		CString strSubmitterID = m_strSubmitterISA06ID;

		//EbillingFormatsT.SubmitterISA06ID
		OutputString += ParseANSIField(strSubmitterID,15,15,TRUE);

		//ISA07	I05			Interchange ID Qualifier				M 1	ID	2/2

		//default "ZZ" ("ZZ" is "Mutually Defined")
		CString strReceiverQual = m_strReceiverISA07Qual;
		OutputString += ParseANSIField(strReceiverQual,2,2,TRUE);

		//ISA08	I07			Interchange Receiver ID					M 1	AN	15/15

		CString strReceiverID = m_strReceiverISA08ID;

		//EbillingFormatsT.ReceiverISA08ID
		str = strReceiverID;
		OutputString += ParseANSIField(str,15,15,TRUE);

		//ISA09	I08			Interchange Date						M 1	DT	6/6

		//current date YYMMDD
		COleDateTime dt = COleDateTime::GetCurrentTime();
		str = dt.Format("%y%m%d");
		OutputString += ParseANSIField(str,6,6,TRUE);

		//ISA10	I09			Interchange Time						M 1	TM	4/4

		//current time HHMM
		str = dt.Format("%H%M");
		OutputString += ParseANSIField(str,4,4,TRUE);

		//ISA11	I10			Repetition Separator					M	ID	1/1

		//This field provides the delimiter used to separate repeated occurrences
		//of a simple data element or a composite data structure; this value must be
		//different than the data element separator, component element separator, and the
		//segment terminator.

		//static "^", do not parse, just add raw
		OutputString += "*^";

		//ISA12	I11			Interchange Control Version Number		M 1	ID	5/5

		//static "0051"
		OutputString += ParseANSIField("00501",5,5,TRUE);

		//ISA13	I12			Interchange Control Number				M 1	N0	9/9

		//m_strBatchNumber
		OutputString += ParseANSIField(m_strBatchNumber,9,9,TRUE,'R','0');

		//ISA14	I13			Acknowledgement Requested				M 1	ID	1/1

		//static "0" (set this to 1 if we want acknowledgement)
		OutputString += ParseANSIField("0",1,1,TRUE);

		//ISA15	I14			Usage Indicator							M 1	ID	1/1

		//Prod/Test
		if (GetRemotePropertyInt ("EnvoyProduction", 0, 0, "<None>") == 0) { //test
			str = "T";
		}
		else { //production
			str = "P";
		}
		OutputString += ParseANSIField(str,1,1,TRUE);

		//ISA16	I15			Component Element Separator				M 1		1/1

		//static ":", do not parse, just add raw
		OutputString += "*:";
		
		EndANSISegment(OutputString);

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_InterchangeHeader");

	return Error_Other;

}

int CEbilling::ANSI_5010_InterchangeTrailer() {

	//Interchange Control Trailer (page C.10 - or page 644)
	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\nInterchange Control Trailer\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	try {

		CString OutputString,str;
		_variant_t var;

//Page	//Ref.	Data		Name									Attributes
//644	//Des.	Element

		OutputString = "IEA";

		//IEA01	I16			Number Of Included Functional Groups	M 1	N0	1/5

		//JJ - I don't foresee us supporting multiple functional groups. I don't
		//quite know why they would be needed. If we do use them, however, we must
		//make this another member counter.

		//static "1"
		OutputString += ParseANSIField("1",1,5,TRUE,'R','0');

		//IEA02	I12			Interchange Control Number				M 1	N0	9/9
		
		//m_strBatchNumber
		OutputString += ParseANSIField(m_strBatchNumber,9,9,TRUE,'R','0');
		
		EndANSISegment(OutputString);

		return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_InterchangeTrailer");

	return Error_Other;

}

int CEbilling::ANSI_5010_FunctionalGroupHeader() {

	//Functional Group Header (page C.7 or 641)
	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\nFunctional Group Header\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	try {

		CString OutputString,str;
		_variant_t var;

//Page# //Ref.	Data		Name									Attributes
//641	//Des.	Element

		OutputString = "GS";

		//GS01	479			Functional Identifier Code				M 1	ID	2/2

		//static "HC" for health care claim
		OutputString += ParseANSIField("HC",2,2);

		//GS02	142			Application Sender's Code				M 1	AN	2/15

		// (j.jones 2008-05-06 11:51) - PLID 29937 - cached SubmitterGS02ID
		CString strSubmitterID = m_strSubmitterGS02ID;

		//EbillingFormatsT.SubmitterGS02ID
		OutputString += ParseANSIField(strSubmitterID,2,15);

		//GS03	124			Application Receiver's Code				M 1	AN	2/15

		// (j.jones 2008-05-06 11:51) - PLID 29937 - cached ReceiverGS03ID
		CString strReceiverID = m_strReceiverGS03ID;

		//EbillingFormatsT.ReceiverGS03ID
		str = strReceiverID;
		OutputString += ParseANSIField(str,2,15);

		//GS04	373			Date									M 1	DT	8/8

		//current date CCYYMMDD
		COleDateTime dt = COleDateTime::GetCurrentTime();
		str = dt.Format("%Y%m%d");
		OutputString += ParseANSIField(str,8,8);

		//GS05	337			Time									M 1	TM	4/8

		//current time HHMM
		str = dt.Format("%H%M");
		OutputString += ParseANSIField(str,4,8);

		//GS06	28			Group Control Number					M 1	N0	1/9

		//m_strBatchNumber

		//in this instance, trim out leading zeros
		m_strBatchNumber.TrimLeft("0");

		OutputString += ParseANSIField(m_strBatchNumber,1,9);

		//GS07	455			Responsible Agency Code					M 1	ID	1/2

		//static "X"
		OutputString += ParseANSIField("X",1,2);

		//GS08	480			Version/Release/Industry Ident. Code	M 1	AN	1/12

		//005010X222A1 - HCFA, Professional
		//005010X223A2 - UB, Institutional
		//005010X224A1 - Dental

		//this value is the same in ST03, be sure the code is identical

		// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType != actInst) {
			//HCFA
			str = "005010X222A1";
		}
		else {
			//UB
			str = "005010X223A2";
		}
		
		OutputString += ParseANSIField(str,1,12);
		
		EndANSISegment(OutputString);

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_FunctionalGroupHeader");

	return Error_Other;

}

int CEbilling::ANSI_5010_FunctionalGroupTrailer() {

	//Functional Group Trailer (page B.10 - or 643)
	// (b.spivey, August 27th, 2014) - PLID 63492 - The variable replaces the debug check. 
	if (m_bHumanReadableFormat) {

		CString str;

		str = "\r\nFunctional Group Trailer\r\n";
		m_OutputFile.Write(str,str.GetLength());
	
	}

	try {

		CString OutputString,str;
		_variant_t var;

//Page	//Ref.	Data		Name									Attributes
//643	//Des.	Element

		OutputString = "GE";

		//GE01	97			Number of Transaction Sets Included		M 1	N0	1/6

		//JJ - I don't foresee us supporting multiple transaction sets, though it's
		//probably more likely than using multiple functional groups.
		//If we do use them, however, we must make this another member counter.

		//static "1"
		OutputString += ParseANSIField("1",1,6);

		//GE02	28			Group Control Number					M 1	N0	1/9

		//m_strBatchNumber

		//in this instance, trim out leading zeros
		m_strBatchNumber.TrimLeft("0");

		OutputString += ParseANSIField(m_strBatchNumber,1,9);

		EndANSISegment(OutputString);

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_5010_FunctionalGroupTrailer");

	return Error_Other;

}

// (j.jones 2007-01-30 09:05) - PLID 24411 - allowed ability to separate by location
// (j.jones 2008-05-02 09:48) - PLID 27478 - now we are given an array so the calling function
// can track which qualifiers we used
void CEbilling::ANSI_5010_Output2010AAProviderIDs(long nProviderID, long nLocationID, CStringArray &arystrQualifiers)
{
	//first generate all the IDs

	struct ProvID {
		CString strID;
		CString strIdent;
	};

	CPtrArray paryProvIDs;

	//find all possible IDs used for this provider on any claim in the batch

	// (j.jones 2006-12-01 15:50) - PLID 22110 - supported the ClaimProviderID
	// (j.jones 2007-01-30 09:20) - PLID 24411 - ensured we filtered by location
	// (j.jones 2007-02-27 09:25) - PLID 24905 - included HCFASetupT.Box33GRP and UB92SetupT.Box82Num so we do not use the member structs
	// (j.jones 2007-04-02 10:18) - PLID 25276 - included UB92SetupT.UB04Box76Qual
	// (j.jones 2008-04-03 09:28) - PLID 28995 - added ANSI_RenderingProviderID and Box24J_ProviderID fields
	// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
	// (j.jones 2008-08-01 10:17) - PLID 30917 - HCFAClaimProvidersT is now per insurance company
	// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
	// (j.jones 2014-07-09 10:08) - PLID 62568 - ignore on hold claims
	_RecordsetPtr rs = CreateParamRecordset("SELECT * FROM "
				"(SELECT HCFATrackT.ID AS HCFAID, BillsT.ID AS BillID, BillsT.PatientID, PatientsT.UserDefinedID, HCFASetupT.Box33Setup, "

				//find who the master (billing) provider is
				"(CASE WHEN BillsT.FormType = 2 "
				"THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"	WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
				"	ELSE ChargeProvidersT.ClaimProviderID END) "
				"WHEN BillsT.FormType = 1 "
				"	THEN (CASE WHEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID Is Not Null "
				"		THEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID "
				"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"			ELSE ChargeProvidersT.ClaimProviderID END) "
				"		END) "
				"ELSE ChargeProvidersT.ClaimProviderID END) AS ProviderID, "

				"BillsT.InsuredPartyID, BillsT.OthrInsuredPartyID, BillsT.Location, InsuranceCoT.PersonID AS InsuranceCoID, InsuranceCoT.HCFASetupGroupID, InsuranceCoT.UB92SetupGroupID, LineItemT.LocationID, "
				"HCFASetupT.Box33GRP, UB92SetupT.Box82Setup, UB92SetupT.Box82Num, UB92SetupT.UB04Box76Qual "
				"FROM HCFATrackT "
				"INNER JOIN BillsT ON HCFATrackT.BillID = BillsT.ID "
				"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
				"LEFT JOIN ProvidersT ChargeProvidersT ON ChargesT.DoctorsProviders = ChargeProvidersT.PersonID "
				"LEFT JOIN ProvidersT PatientProvidersT ON PatientsT.MainPhysician = PatientProvidersT.PersonID "
				"LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
				"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
				"LEFT JOIN UB92SetupT ON InsuranceCoT.UB92SetupGroupID = UB92SetupT.ID "
				"LEFT JOIN HCFAClaimProvidersT ON InsuranceCoT.PersonID = HCFAClaimProvidersT.InsuranceCoID AND (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.PersonID ELSE ChargeProvidersT.PersonID END) = HCFAClaimProvidersT.ProviderID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "

				"WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND Coalesce(BillStatusT.Type, -1) != {CONST_INT} "

				"GROUP BY HCFATrackT.ID, BillsT.ID, BillsT.PatientID, PatientsT.UserDefinedID, HCFASetupT.Box33Setup, "

				//we have to duplicate the provider ID logic here in the GROUP BY clause
				"(CASE WHEN BillsT.FormType = 2 "
				"THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"	WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
				"	ELSE ChargeProvidersT.ClaimProviderID END) "
				"WHEN BillsT.FormType = 1 "
				"	THEN (CASE WHEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID Is Not Null "
				"		THEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID "
				"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"			ELSE ChargeProvidersT.ClaimProviderID END) "
				"		END) "
				"ELSE ChargeProvidersT.ClaimProviderID END), "

				"HCFATrackT.Batch, HCFATrackT.CurrentSelect, BillsT.InsuredPartyID, BillsT.OthrInsuredPartyID, BillsT.Location, InsuranceCoT.PersonID, InsuranceCoT.HCFASetupGroupID, InsuranceCoT.UB92SetupGroupID, LineItemT.LocationID, "
				"HCFASetupT.Box33GRP, UB92SetupT.Box82Setup, UB92SetupT.Box82Num, UB92SetupT.UB04Box76Qual, "
				"Convert(int,BillsT.Deleted), Convert(int,LineItemT.Deleted) "

				"HAVING HCFATrackT.Batch = 2 AND HCFATrackT.CurrentSelect = 1 AND Convert(int,BillsT.Deleted) = 0 AND Convert(int,LineItemT.Deleted) = 0) AS HCFAEbillingQ "

				"WHERE ProviderID = {INT} AND LocationID = {INT} "
				"ORDER BY ProviderID, InsuredPartyID, HCFAID", EBillStatusType::OnHold, nProviderID, nLocationID);

	while(!rs->eof) {

		long nInsuredPartyID = AdoFldLong(rs, "InsuredPartyID",-1);
		long nInsuranceCoID = AdoFldLong(rs, "InsuranceCoID",-1);
		long nHCFASetupID = AdoFldLong(rs, "HCFASetupGroupID",-1);
		long nUB92SetupID = AdoFldLong(rs, "UB92SetupGroupID",-1);
		_variant_t var;

		if(nInsuredPartyID > 0) {

			//now get the ID like we normally would, but before adding to the array make sure it's not already in the array

			CString strIdent = "";
			CString strID = "";
			CString strLoadedFrom = "";
			// (j.jones 2010-04-14 09:04) - PLID 38194 - the ID is now calculated in a shared function
			// (j.jones 2010-10-18 16:11) - PLID 40346 - changed HCFA/UB boolean to an enum
			EBilling_Calculate2010_REF(FALSE, strIdent, strID, strLoadedFrom, m_actClaimType == actInst,
				nProviderID, nInsuranceCoID, nLocationID, nInsuredPartyID,
				nHCFASetupID, AdoFldString(rs, "Box33GRP",""),
				nUB92SetupID, AdoFldString(rs, "UB04Box76Qual",""), AdoFldLong(rs, "Box82Num",1),
				AdoFldLong(rs, "Box82Setup",1));

			// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strIdent.CompareNoCase("XX") == 0) {
				strIdent = "";
				strID = "";
				//log that this happened
				Log("Ebilling file tried to export 2010AA REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
			}

			//okay, we have our IDs, so let's try to add them to the array
			if(strIdent != "" && strID != "") {

				BOOL bFound = FALSE;

				if(paryProvIDs.GetSize() > 0) {
					for(int i=0;i<paryProvIDs.GetSize();i++) {
						ProvID *ebProvID = (ProvID*)paryProvIDs.GetAt(i);
						//you may not duplicate strIdent qualifiers
						if(ebProvID->strIdent == strIdent /*(&& ebProvID->strID == strID*/)
							bFound = TRUE;
					}
				}

				if(!bFound) {
					ProvID *ebProvID = new(ProvID);
					ebProvID->strIdent = strIdent;
					ebProvID->strID = strID;
					paryProvIDs.Add(ebProvID);
				}
			}
		}
		rs->MoveNext();
	}
	rs->Close();

	//now output the IDs

	for(int i=0;i<paryProvIDs.GetSize();i++) {

		ProvID *ebProvID = (ProvID*)paryProvIDs.GetAt(i);
	
		CString OutputString = "REF";

		//REF01	128			Reference Identification Qualifier		M 1	ID	2/3

		m_strLast2010AAQual = ebProvID->strIdent;

		OutputString += ParseANSIField(ebProvID->strIdent,2,3);

		//REF02	127			Reference Identification				X 1	AN	1/50

		OutputString += ParseANSIField(ebProvID->strID,1,50);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
		if(m_bDisableREFXX && ebProvID->strIdent.CompareNoCase("XX") == 0) {
			ebProvID->strIdent = "";
			ebProvID->strID = "";
			//log that this happened
				Log("Ebilling file tried to export 2010AA REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
		}

		// (j.jones 2008-05-02 09:56) - PLID 27478 - disallow sending the additional REF
		// segment if the qualifier has already been used
		if(ebProvID->strIdent != "" && ebProvID->strID != "" && !IsQualifierInArray(arystrQualifiers, ebProvID->strIdent)) {
			EndANSISegment(OutputString);

			// (j.jones 2008-05-02 09:50) - PLID 27478 - add this qualifier to our array
			arystrQualifiers.Add(ebProvID->strIdent);
		}
	}

	//now delete the array
	for(int z=paryProvIDs.GetSize()-1;z>=0;z--) {
		delete (ProvID*)paryProvIDs.GetAt(z);
		paryProvIDs.RemoveAt(z);
	}

}

// (j.jones 2008-11-12 12:33) - PLID 31740 - if we have any CAS segments in memory, output them now
void CEbilling::ANSI_5010_OutputCASSegments(CArray<ANSI_CAS_Info*, ANSI_CAS_Info*> &aryCASSegments)
{
	//throw any exceptions to the caller

	//The CAS segment has a Group Code, a Reason Code, an Amount, and a Quantity field (unused).
	//Group Codes cannot be duplicated. Reason Codes cannot be duplicated per Group.
	//A CAS segment can have one Group Code, then up to 6 instances of Reason Codes, Amount, & Quantity.
	
	//AddCASSegmentToMemory() will have already built up our CAS segments based on loaded data, updating
	//amounts when duplicates occur. This function needs to take the memory objects and export them
	//in the proper ANSI format. In the event that we have more than 6 reasons in a group, we have no
	//choice but to duplicate the group. This is highly unlikely to ever occur. This function will not
	//perform any logic other than just outputting what is stored in memory.

	CString OutputString,str;

	int i=0, j=0;

	for(i=0; i<aryCASSegments.GetSize(); i++) {
		ANSI_CAS_Info* pInfo = (ANSI_CAS_Info*)aryCASSegments.GetAt(i);
		
		OutputString = "CAS";

		//CAS01	1033		Claim Adjustment Group Code				M 1	ID	1/2
		OutputString += ParseANSIField(pInfo->strGroupCode, 1, 2);

		if(pInfo->aryDetails.GetSize() == 0) {
			//this should be impossible
			ASSERT(FALSE);
			continue;
		}

		for(j=0; j<pInfo->aryDetails.GetSize(); j++) {
			ANSI_CAS_Detail* pDetail = (ANSI_CAS_Detail*)pInfo->aryDetails.GetAt(j);

			//CAS02	1034		Claim Adjustment Reason Code			M 1	ID	1/5
			OutputString += ParseANSIField(pDetail->strReasonCode, 1, 5);

			//CAS03	782			Monetary Amount							M 1	R	1/18
			str = FormatCurrencyForInterface(pDetail->cyAmount, FALSE, FALSE);
			//see if we need to trim the zeros
			// (j.jones 2012-05-25 16:29) - PLID 50657 - now a modular function, truncates if m_bTruncateCurrency is enabled
			str = FormatANSICurrency(str);
			OutputString += ParseANSIField(str, 1, 18);

			//CAS04	380			Quantity								O 1	R	1/15
			//optional, we do not send this
			OutputString += ParseANSIField("", 1, 15);
		}

		EndANSISegment(OutputString);
	}

	//clear the memory
	for(i=aryCASSegments.GetSize()-1;i>=0;i--) {		
		ANSI_CAS_Info* pInfo = (ANSI_CAS_Info*)(aryCASSegments.GetAt(i));
		int j=0;
		for(j=pInfo->aryDetails.GetSize()-1;j>=0;j--) {		
			delete (ANSI_CAS_Detail*)(pInfo->aryDetails.GetAt(j));
		}
		pInfo->aryDetails.RemoveAll();

		delete (ANSI_CAS_Info*)pInfo;
	}
	aryCASSegments.RemoveAll();
}