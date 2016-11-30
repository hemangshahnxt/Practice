// EbillingANSI4010.cpp : implementation file
// Contains all claim generation functions for ANSI 4010 claims

// (j.jones 2010-10-13 10:43) - PLID 40913 - created, moved code from ebilling.cpp

#include "stdafx.h"
#include "Ebilling.h"
#include "GlobalUtils.h"
#include "GlobalFinancialUtils.h"
#include "GlobalDataUtils.h"
#include "InternationalUtils.h"
#include "InsuranceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

//ANSI X12 4010

//The following functions, ANSI_4010_1000A() to ANSI_4010_2440(), represent individual 'loops' of a generated ANSI claim file.
//Each function will generate its own line and write it to the output file.

int CEbilling::ANSI_4010_Header() {

	//Header

#ifdef _DEBUG

	CString str;
	str = "\r\n4010 HEADER\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							HEADER

//62		005		ST		Transaction Set Header					R		1

		OutputString = "ST";

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//ST01	143			Transaction Set Identifier Code			M	ID	3/3
		
		//static "837"
		OutputString += ParseANSIField("837",3,3);

		//ST02	329			Transaction Set Control Number			M	AN	4/9
		
		//m_strBatchNumber
		OutputString += ParseANSIField(m_strBatchNumber,4,9,TRUE,'R','0');

		EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

//63		010		BHT		Beginning of Hierarchical Transaction	R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "BHT";

		//BHT01	1005		Hierarchical Structure Code				M	ID	4/4
		
		//static "0019"
		OutputString += ParseANSIField("0019",4,4);

		//BHT02	353			Transaction Set Purpose Code			M	ID	2/2
		
		//TODO: use "18" for a re-issue, "00" for a new claim
		//static "00"
		OutputString += ParseANSIField("00",2,2);

		//BHT03	127			Reference Identification				O	AN	1/30

		//batch number
		OutputString += ParseANSIField(m_strBatchNumber,1,30,TRUE,'R','0');

		//BHT04	373			Date									O	DT	8/8

		//current date YYYY/MM/DD
		COleDateTime dt = COleDateTime::GetCurrentTime();
		str = dt.Format("%Y%m%d");
		OutputString += ParseANSIField(str,8,8);
	
		//BHT05	337			Time									O	TM	4/8

		//current time	
		str = dt.Format("%H%M%S");
		OutputString += ParseANSIField(str,4,8);

		//BHT06	640			Transaction Type Code					O	ID	2/2

		//static "CH"
		OutputString += ParseANSIField("CH",2,2);

		EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

//66		015		REF		Transmission Type Identification		R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01	128			Reference Identification Qualifier		M	ID	2/3
		
		//static "87"
		OutputString += ParseANSIField("87",2,3);

		//REF02	127			Reference Identification				X	AN	1/30
							//Syntax: R0203
		
		//004010X098A1 - HCFA, Professional
		//004010X096A1 - UB92, Institutional

		//TEST/PROD
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if (GetRemotePropertyInt ("EnvoyProduction", 0, 0, "<None>") == 0) { // test
			if(m_actClaimType != actInst)
				str = "004010X098DA1";
			else
				str = "004010X096DA1";
		}
		else { // production
			if(m_actClaimType != actInst)
				str = "004010X098A1";
			else
				str = "004010X096A1";
		}

		OutputString += ParseANSIField(str,1,30);

		//REF03	NOT USED

		OutputString += "*";

		//REF04 NOT USED

		OutputString += "*";
		
		EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_Header");

	return Error_Other;

}

int CEbilling::ANSI_4010_1000A() {

	//Submitter Name

#ifdef _DEBUG

	CString str;

	str = "\r\n1000A\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

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

//67		020		NM1		Submitter Name							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M	ID	2/3

		//static "41"
		OutputString += ParseANSIField("41",2,3);		

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//static "2"
		OutputString += ParseANSIField("2",1,1);

		//NM103	1035		Name Last or Organization Name			O	AN	1/35

		//LocationsT.Name
		str = GetFieldFromRecordset(rsLocations, "Name");
		if(str.GetLength() > 35) {
			str = str.Left(35);							//first part is first 35 chars
		}
		OutputString += ParseANSIField(str, 1, 35);

		//NM104	1036		Name First								O	AN	1/25

		OutputString += "*";

		//NM105	1037		Name Middle								O	AN	1/25

		OutputString += "*";

		//NM106 NOT USED
		OutputString += "*";

		//NM107 NOT USED
		OutputString += "*";

		//NM108	66			Identification Code Qualifier			X	ID	1/2

		CString strSubmitterID = m_strSubmitter1000AID;
		CString strSubmitterQual = m_strSubmitter1000AQual;
		
		//if there is no ID code in NM109, then don't output a number here
		if(strSubmitterID == "")
			str = "";
		else
			str = strSubmitterQual;

		OutputString += ParseANSIField(str,1,25);

		//NM109	67			Identification Code						X	AN	2/80

		//EbillingFormatsT.Submitter1000AID
		str = strSubmitterID;
		OutputString += ParseANSIField(str,1,25);

		//NM110 NOT USED
 		OutputString += "*";

		//NM111 NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

		/* removed in the A1 Addendum
		//not used if an Institutional claim
		if(m_actClaimType != actInst) {

//70		025		N2		Additional Submitter Name Information	S		1

			OutputString = "N2";

			//Ref.	Data		Name									Attributes
			//Des.	Element

			//N201	93			Name									M	AN	1/60
			str = GetFieldFromRecordset(rs, "Name");

			if(str.GetLength() > 35) {	//then this segment is required
				str = str.Right(str.GetLength() - 35);		//2nd part is 36 on
				
				OutputString += ParseANSIField(str, 1, 60);

				//N202	NOT USED
				OutputString += "*";

				EndANSISegment(OutputString);

			}
		}
		*/

///////////////////////////////////////////////////////////////////////////////

//71		045		PER		Submitter EDI Contact Information		R		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "PER";

		//PER01	366			Contact Function Code					M	ID	2/2

		//static "IC";
		OutputString += ParseANSIField("IC",2,2);

		//PER02	93			Name									O	AN	1/60

		//EbillingFormatsT.Contact
		str = m_strEbillingFormatContact;
		OutputString += ParseANSIField(str,1,60);

		//PER03	365			Communication Number Qualifier			X	ID	2/2

		//static "TE"
		str = GetFieldFromRecordset(rsLocations,"Phone");
		str = StripNonNum(str);
		if(str != "")
			OutputString += ParseANSIField("TE",2,2);
		else
			OutputString += "*";

		//PER04	364			Communication Number					X	AN	1/80

		//LocationsT.Phone
		if(str != "")
			OutputString += ParseANSIField(str,1,80);
		else
			OutputString += "*";

		//PER05	365			Communication Number Qualifier			X	ID	2/2		

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

		//PER06	364			Communication Number					X	AN	1/80

		// (j.jones 2007-04-05 10:49) - PLID 25506 - will be either
		// LocationsT.Fax or EbillingFormatsT.PER06ID_1000A
		OutputString += ParseANSIField(strPER06ID,1,80);

		//PER07	365			Communication Number Qualifier			X	ID	2/2

		OutputString += "*";

		//PER08	364			Communication Number					X	AN	1/80

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

	} NxCatchAll("Error in Ebilling::ANSI_4010_1000A");

	return Error_Other;
}

int CEbilling::ANSI_4010_1000B() {

	//Receiver Name

#ifdef _DEBUG

	CString str;

	str = "\r\n1000B\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		// (j.jones 2008-05-06 11:34) - PLID 29937 - removed the EbillingFormatsT recordset as all of it is cached now

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 1000B - RECEIVER NAME									1

//74		020		NM1		Receiver Name							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M	ID	2/3

		//static "40"
		OutputString += ParseANSIField("40",2,3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1
		
		//static "2"
		OutputString += ParseANSIField("2",1,1);

		//NM103	1035		Name Last or Organization Name			O	AN	1/35

		//EbillingFormatsT.Name
		str = m_strEbillingFormatName;
		OutputString += ParseANSIField(str,1,35);

		//NM104 NOT USED
		OutputString += "*";
		
		//NM105	NOT USED
		OutputString += "*";
		
		//NM106	NOT USED
		OutputString += "*";
		
		//NM107	NOT USED
		OutputString += "*";

		//NM108	66			Identification Code Qualifier			X	ID	1/2

		CString strReceiverQual = m_strReceiver1000BQual;
		CString strReceiverID = m_strReceiver1000BID;

		//default "46"
		str = strReceiverID;
		if(str != "")
			OutputString += ParseANSIField(strReceiverQual,1,2);
		else
			OutputString += "*";

		//NM109	67			Identification Code						X	AN	2/80

		//EbillingFormatsT.strReceiverID		
		if(str != "")
			OutputString += ParseANSIField(str,2,80);
		else
			OutputString += "*";

		//NM110	NOT USED
		OutputString += "*";

		//NM111	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_1000B");

	return Error_Other;

}

int CEbilling::ANSI_4010_2000A() {

	//Billing/Pay-To Provider Hierarchical Level

#ifdef _DEBUG

	CString str;

	str = "\r\n2000A\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

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

//77		001		HL		Billing/Pay-to Prov. Hierarch. Level	R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HL";

		//HL01	638			Hierarchical ID Number					M	AN	1/12

		//m_ANSIHLCount

		str.Format("%li",m_ANSIHLCount);
		OutputString += ParseANSIField(str,1,12);

		//HL02 NOT USED
		OutputString += "*";

		//HL03	735			Hierarchical Level Code					M	ID	1/2

		//static "20"
		OutputString += ParseANSIField("20",1,12);

		//HL04	736			Hierarchical Child Code					O	ID	1/1

		//static "1"
		OutputString += ParseANSIField("1",1,12);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//79		003		PRV		Billing/Pay-to Prov. Specialty Info.	S		1

		//This segment is used when the Rendering Provider is the same entity
		//as the Billing Provider and/or the Pay-To Provider.
		//If this segment is used, then Loop 2310B is not used.

		//This segment is not used when the Billing or Pay-To Provider is a group
		//and the individual Rendering Provider is in Loop 2310B.

		//This segment will not be used when m_bIsGroup is TRUE.

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "PRV";

		//PRV01	1221		Provider Code							M	ID	1/3

		//"BI" for Billing, "PT" for Pay-To

		//for now, lets assume it is always Billing
		OutputString += ParseANSIField("BI",1,3);

		CString strTaxonomy = "";
		// (j.jones 2007-05-10 12:24) - PLID 25948 - support the UB referring physician
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore,
		// so if they are sending the referring physician in this loop (extremely unlikely), we cannot
		// send a taxonomy code.
		bool bIsReferringPhysicianOnUB = (m_actClaimType == actInst && m_pEBillingInfo->Box82Setup == 3);
		if(!bIsReferringPhysicianOnUB) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT TaxonomyCode FROM ProvidersT WHERE PersonID = {INT}",m_pEBillingInfo->ProviderID);
			if(!rs->eof) {
				strTaxonomy = AdoFldString(rs, "TaxonomyCode","");
				strTaxonomy.TrimLeft();
				strTaxonomy.TrimRight();
			}
			rs->Close();
		}

		// (j.jones 2008-05-01 10:47) - PLID 28691 - don't output if the taxonomy code is blank
		if(!strTaxonomy.IsEmpty()) {

			//PRV02	128			Reference/Identification Qualifier		M	ID	2/3

			//static "ZZ"
			OutputString += ParseANSIField("ZZ",2,3);
			
			//PRV03	127			Reference Identification				M	AN	1/30

			//ProvidersT.TaxonomyCode
			//This comes from a published HIPAA list.
			//"2086S0122X" is for Plastic and Reconstructive Surgery
			OutputString += ParseANSIField(strTaxonomy,1,30);

			//PRV04	NOT USED
			OutputString += "*";

			//PRV05	NOT USED
			OutputString += "*";

			//PRV06	NOT USED
			OutputString += "*";

			//if it's a group, don't output this segment
			if((!m_bIsGroup && m_HCFAInfo.Use2000APRVSegment == 1) || m_HCFAInfo.Use2000APRVSegment == 2) {
				EndANSISegment(OutputString);
			}
		}

///////////////////////////////////////////////////////////////////////////////

//81		010		CUR		Foreign Currency Information			S		1

		//not used

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2000A");

	return Error_Other;
}

int CEbilling::ANSI_4010_2010AA() {

	//Billing Provider Name

#ifdef _DEBUG

	CString str;

	str = "\r\n2010AA\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;
		
		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
			// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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

//84		015		NM1		Billing Provider Name					R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M	ID	2/3

		//static "85"
		OutputString += ParseANSIField("85",2,3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//1 - Individual
		//2 - Group

		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2008-01-03 10:56) - PLID 28454 - needed to submit as a group if using an override,
		// because otherwise we have to send a first name field, but that also means we need to check
		// the Box33Name override here first

		// (j.jones 2007-05-07 16:48) - PLID 25922 - added Box33Name override
		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		CString strBox33NameOver = "", strBox33Address1_Over = "", strBox33Address2_Over = "",
			strBox33City_Over = "", strBox33State_Over = "", strBox33Zip_Over = "";
		BOOL bUseBox33NameOver = FALSE;
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType != actInst) {
			// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
			_RecordsetPtr rsBox33Over = CreateParamRecordset("SELECT Box33Name, "
				"Box33_Address1, Box33_Address2, Box33_City, Box33_State, Box33_Zip "
				"FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
			if(!rsBox33Over->eof) {

				//the Box 33 name override is only supported if the "Use Override" Box 33 setting is in use
				if(m_HCFAInfo.Box33Setup == 3) {
					strBox33NameOver = AdoFldString(rsBox33Over, "Box33Name","");
					strBox33NameOver.TrimLeft();
					strBox33NameOver.TrimRight();

					if(!strBox33NameOver.IsEmpty())
						bUseBox33NameOver = TRUE;
				}

				// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
				// (these do not need a boolean as well)
				strBox33Address1_Over = AdoFldString(rsBox33Over, "Box33_Address1","");
				strBox33Address1_Over.TrimLeft();
				strBox33Address1_Over.TrimRight();
				strBox33Address2_Over = AdoFldString(rsBox33Over, "Box33_Address2","");
				strBox33Address2_Over.TrimLeft();
				strBox33Address2_Over.TrimRight();
				strBox33City_Over = AdoFldString(rsBox33Over, "Box33_City","");
				strBox33City_Over.TrimLeft();
				strBox33City_Over.TrimRight();
				strBox33State_Over = AdoFldString(rsBox33Over, "Box33_State","");
				strBox33State_Over.TrimLeft();
				strBox33State_Over.TrimRight();
				strBox33Zip_Over = AdoFldString(rsBox33Over, "Box33_Zip","");
				strBox33Zip_Over.TrimLeft();
				strBox33Zip_Over.TrimRight();
			}
			rsBox33Over->Close();
		}

		CString strProviderCompany = "";
		BOOL bUseProviderCompanyOnClaims = FALSE;

		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
		OutputString += ParseANSIField(str,1,1);

		//NM103	1035		Name Last or Organization Name			O	AN	1/35

		//show the Location Name as the group name if m_bIsGroup is TRUE.

		str = "";
		// (j.jones 2008-01-03 11:14) - PLID 28454 - check for overrides first, as they are
		// allowed even when sending as a group
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
		OutputString += ParseANSIField(str,1,35);

		//NM104	1036		Name First								O	AN	1/25

		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
		OutputString += ParseANSIField(str,1,25);

		//NM105	1037		Name Middle								O	AN	1/25

		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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

		//NM107	1039		Name Suffix								O	AN	1/10

		//PersonT.Title
		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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

		//NM108	66			Identification Code	Qualifier			X	ID	1/2

		CString strID, strIdent;

		// (j.jones 2006-11-14 08:55) - PLID 23413 - determine whether the
		// NM108/109 elements should show the NPI or the EIN/SSN
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		long nNM109_IDType = 0;		
		if(m_actClaimType == actInst)
			// (j.jones 2006-11-14 08:56) - PLID 23446 - check the UB92 setup
			nNM109_IDType = m_UB92Info.NM109_IDType;
		else
			nNM109_IDType = m_HCFAInfo.NM109_IDType;

		if(nNM109_IDType == 1) {

			// (j.jones 2006-11-14 08:57) - PLID 23413, 23446 - if 1, then
			// use the EIN or SSN, depending on the group or UB92 usage

			//"24" for EIN, "34" for SSN - from HCFASetupT.Box25

			//If a group, show the location's EIN, ID of 24")

			// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
			// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
			if(m_bIsGroup || m_actClaimType == actInst || m_HCFAInfo.Box33Setup == 4) {
				//a group
				strIdent = "24";
				strID = GetFieldFromRecordset(rs,"EIN");
			}
			else {
				//individual doctor

				if(m_HCFAInfo.Box25==1)
					strIdent = "34";
				else
					strIdent = "24";

				if(m_HCFAInfo.Box25==1) {
					var = fields->Item["SocialSecurity"]->Value;
					if(var.vt == VT_BSTR)
						strID = CString(var.bstrVal);
					else
						strID = "";
				}
				else {
					var = fields->Item["Fed Employer ID"]->Value;
					if(var.vt == VT_BSTR)
						strID = CString(var.bstrVal);
					else
						strID = "";
				}
			}
		}
		else {

			//"XX" for NPI
			strIdent = "XX";

			strID = "";

			// (j.jones 2006-11-14 08:58) - PLID 23413, 23446 - if 0,
			// then use the NPI, may be location NPI if group or UB92

			// (j.jones 2007-01-18 11:51) - PLID 24264 - supported the
			// NPI selection in HCFA Box 33, to determine whether to use
			// the Location NPI or Provider NPI. If a UB92 or a Group,
			// always use location (would already be loaded in the active
			// recordset), if a non-Group HCFA, we need to load the location
			// NPI number.

			// (j.jones 2007-08-08 13:14) - PLID 25395 - if there is a 33a override in AdvHCFAPinT, use it here
			// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
							strID = strOver;
					}
				}
				rsOver->Close();
			}

			if(strID.IsEmpty()) {

				// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_HCFAInfo.LocationNPIUsage == 1 && !m_bIsGroup && m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 4) {
					//a non-group HCFA, meaning that the open recordset does
					//not contain the location NPI, and we want it
					
					// (j.jones 2008-05-06 14:06) - PLID 29937 - use the NPI from the loaded claim pointer
					strID = m_pEBillingInfo->strBillLocationNPI;
				}
				else {
					//whether it's provider or not, the open recordset has
					//the NPI we are looking for
					strID = VarString(fields->Item["NPI"]->Value, "");
				}
			}
		}

		strID = StripNonNum(strID);

		if(strID != "")
			OutputString += ParseANSIField(strIdent,1,2);
		else
			OutputString += "*";

		//NM109	67			Identification Code						X	AN	2/80

		//ProvidersT.Fed Employer ID or PersonT.SocialSecurity
		//depends on setting in HCFASetupT.Box25

		if(strID != "")		
			OutputString += ParseANSIField(strID,2,80);
		else
			OutputString += "*";

		//NM110 NOT USED
		OutputString += "*";

		//NM111	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//88		025		N3		Billing Provider Address				R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//if DocAddress == 1, and we're loading a provider, load the Location address,
		//else use the address in the already open recordset
		//(which will be Contacts if a provider, or else a Location)
		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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

		//N301	166			Address Information						M	AN	1/55

		//PersonT.Address1

		var = fields->Item["Address1"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33Address1_Over.IsEmpty()) {
			str = strBox33Address1_Over;
		}

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str,1,55);

		//N302	166			Address Information						O	AN	1/55

		//PersonT.Address2

		//This should not be used if the address 2 is blank.
		//EndANSISegment will trim off the *.

		var = fields->Item["Address2"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33Address2_Over.IsEmpty()) {
			str = strBox33Address2_Over;
		}

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str,1,55);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//89		030		N4		Billing Provider City/State/Zip			R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O	AN	2/30

		//PersonT.City

		var = fields->Item["City"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33City_Over.IsEmpty()) {
			str = strBox33City_Over;
		}

		OutputString += ParseANSIField(str,2,30);

		//N402	156			State or Province Code					O	ID	2/2

		//PersonT.State

		var = fields->Item["State"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33State_Over.IsEmpty()) {
			str = strBox33State_Over;
		}

		OutputString += ParseANSIField(str,2,2);

		//N403	116			Postal Code								O	ID	3/15

		//PersonT.Zip

		var = fields->Item["Zip"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33Zip_Over.IsEmpty()) {
			str = strBox33Zip_Over;
		}

		str = StripNonNum(str);
		OutputString += ParseANSIField(str,3,15);

		//N404	26			Country Code							O	ID	2/3

				//this is only required if the addess is outside the US
				//as of right now, we can't support it
		
		OutputString += "*";

		//N405	NOT USED
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

		rs->Close();

///////////////////////////////////////////////////////////////////////////////

//91		035		REF		Billing Provider Secondary Information	S		8

		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2008-05-02 09:46) - PLID 27478 - we need to silently skip
		// adding additional REF segments that have the same qualifier, so
		// track them in an array
		CStringArray arystrQualifiers;

		//check to see if we are exporting all IDs from the batch or not

		// (j.jones 2006-12-06 15:50) - PLID 23631 - do not do this if we are separating batches by insurance company
		// (j.jones 2008-05-06 11:37) - PLID 29937 - cached m_ExportAll2010AAIDs and m_bSeparateBatchesByInsCo
		if(m_bExportAll2010AAIDs && !m_bSeparateBatchesByInsCo) {

			// (j.jones 2008-05-02 09:47) - PLID 27478 - pass in our array, so we can track the qualifiers we used
			ANSI_4010_Output2010AAProviderIDs(m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillLocation, arystrQualifiers);
		}
		else {
			//just export one ID

			OutputString = "REF";

			strIdent = "";
			strID = "";
			CString strLoadedFrom = "";
			// (j.jones 2010-04-14 09:04) - PLID 38194 - the ID is now calculated in a shared function
			// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
			EBilling_Calculate2010_REF(FALSE, strIdent, strID, strLoadedFrom, m_actClaimType == actInst,
				m_pEBillingInfo->ProviderID, m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->BillLocation, m_pEBillingInfo->InsuredPartyID,
				m_pEBillingInfo->HCFASetupID, m_HCFAInfo.Box33GRP,
				m_pEBillingInfo->UB92SetupID, m_UB92Info.UB04Box76Qual, m_UB92Info.Box82Num, m_pEBillingInfo->Box82Setup);

			// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strIdent.CompareNoCase("XX") == 0) {
				strIdent = "";
				strID = "";
				//log that this happened
				Log("Ebilling file tried to export 2010AA REF*XX in for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
			}

			//REF01	128			Reference Identification Qualifier		M	ID	2/3

			m_strLast2010AAQual = strIdent;

			OutputString += ParseANSIField(strIdent,2,3);

			//REF02	127			Reference Identification				X	AN	1/30

			OutputString += ParseANSIField(strID,1,30);

			//REF03	NOT USED
			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			// (j.jones 2007-09-20 10:51) - PLID 27455 - do not send this segment if the
			// qualifier is blank
			if(strIdent != "" && strID != "") {
				EndANSISegment(OutputString);

				// (j.jones 2008-05-02 09:47) - PLID 27478 - track the qualifier we used
				arystrQualifiers.Add(strIdent);
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

			//REF01	128			Reference Identification Qualifier		M	ID	2/3
			
			strIdent = strAddnl2010AAQual;
			strID = strAddnl2010AA;

			OutputString += ParseANSIField(strIdent,2,3);

			//REF02	127			Reference Identification				X	AN	1/30

			OutputString += ParseANSIField(strID,1,30);

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

			if(strIdent != "" && strID != "") {
				EndANSISegment(OutputString);

				// (j.jones 2008-05-02 09:50) - PLID 27478 - add this qualifier to our array
				arystrQualifiers.Add(strIdent);
			}
		}

		// (j.jones 2006-11-14 11:48) - PLID 23413 - the HCFA Group now allows the option
		// to append an EIN/SSN, or NPI as an additional REF segment

		//reopen the proper recordset

		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
			// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
		long nExtraREF_IDType = 2;		
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType == actInst)
			// (j.jones 2006-11-14 11:49) - PLID 23446 - the UB92 Group has an option as well
			nExtraREF_IDType = m_UB92Info.ExtraREF_IDType;
		else
			nExtraREF_IDType = m_HCFAInfo.ExtraREF_IDType;

		//0 - EIN/SSN, 1 - NPI, 2 - none
		if(nExtraREF_IDType != 2) {

			//export the extra ID

			OutputString = "REF";

			//REF01	128			Reference Identification Qualifier		M	ID	2/3
			
			strIdent = "";
			strID = "";

			if(nExtraREF_IDType == 0) {
				//EIN/SSN

				//"EI" for EIN, "SY" for SSN - from HCFASetupT.Box25

				//If a group, show the location's EIN, ID of EI")

				// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_bIsGroup || m_actClaimType == actInst || m_HCFAInfo.Box33Setup == 4) {
					//a group
					strIdent = "EI";
					strID = GetFieldFromRecordset(rs,"EIN");
				}
				else {
					//individual doctor

					if(m_HCFAInfo.Box25==1)
						strIdent = "SY";
					else
						strIdent = "EI";

					if(m_HCFAInfo.Box25==1) {
						var = fields->Item["SocialSecurity"]->Value;
						if(var.vt == VT_BSTR)
							strID = CString(var.bstrVal);
						else
							strID = "";
					}
					else {
						var = fields->Item["Fed Employer ID"]->Value;
						if(var.vt == VT_BSTR)
							strID = CString(var.bstrVal);
						else
							strID = "";
					}
				}
			}
			else {
				//NPI

				//"XX" for NPI
				strIdent = "XX";
				strID = "";

				// (j.jones 2006-11-14 08:58) - PLID 23413, 23446 - if 0,
				// then use the NPI, may be location NPI if group or UB92

				// (j.jones 2007-01-18 11:51) - PLID 24264 - supported the
				// NPI selection in HCFA Box 33, to determine whether to use
				// the Location NPI or Provider NPI. If a UB92 or a Group,
				// always use location (would already be loaded in the active
				// recordset), if a non-Group HCFA, we need to load the location
				// NPI number.

				// (j.jones 2007-08-08 13:14) - PLID 25395 - if there is a 33a override in AdvHCFAPinT, use it here
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
								strID = strOver;
						}
					}
					rsOver->Close();
				}

				if(strID.IsEmpty()) {

					// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
					// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
					if(m_HCFAInfo.LocationNPIUsage == 1 && !m_bIsGroup && m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 4) {
						//a non-group HCFA, meaning that the open recordset does
						//not contain the location NPI, and we want it

						// (j.jones 2008-05-06 14:06) - PLID 29937 - now the NPI is in the loaded claim pointer
						strID = m_pEBillingInfo->strBillLocationNPI;
					}
					else {
						//whether it's provider or not, the open recordset has
						//the NPI we are looking for
						strID = VarString(fields->Item["NPI"]->Value, "");
					}
				}
			}

			strID = StripNonNum(strID);

			OutputString += ParseANSIField(strIdent,2,3);

			//REF02	127			Reference Identification				X	AN	1/30

			OutputString += ParseANSIField(strID,1,30);

			//REF03	NOT USED
			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strIdent.CompareNoCase("XX") == 0) {
				strIdent = "";
				strID = "";
				//log that this happened
				Log("Ebilling file tried to export 2010AA Additional REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
			}

			// (j.jones 2008-05-02 09:56) - PLID 27478 - disallow sending the additional REF
			// segment if the qualifier has already been used
			if(strIdent != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdent)) {
				EndANSISegment(OutputString);

				// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
				arystrQualifiers.Add(strIdent);
			}
		}

		rs->Close();

///////////////////////////////////////////////////////////////////////////////

//94		035		REF		Credit/Debit Card Billing Information	S		8

		//not used

///////////////////////////////////////////////////////////////////////////////

//96		040		PER		Billing Provider Contact Information	S		2

		//This is used if the contact information differs from the contact info. in
		//the PER segment of 1000A. In that segment, we use the location information.
		//In this segment, we would do that same, so we won't export anything here
		//until we determine that we need to.
///////////////////////////////////////////////////////////////////////////////		

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2010AA");

	return Error_Other;
}

int CEbilling::ANSI_4010_2010AB() {

	//Pay-To Provider Name

	//JJ - This loop is only used if the pay-to provider is different from the billing
	//provider. We assume the billing provider/pay-to provider/rendering provider are all
	//one and the same - the provider on the bill. However, some clearinghouses are non-compliant,
	//and still want this loop. There is a setting to enable it.


#ifdef _DEBUG

	CString str;

	str = "\r\n2010AB\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;
		
		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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

//							LOOP ID - 2010AB - PAY-TO PROVIDER NAME						1

///////////////////////////////////////////////////////////////////////////////

//99		015		NM1		Pay-To Provider Name					S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M	ID	2/3

		//static "87"
		OutputString += ParseANSIField("87",2,3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//1 - Individual
		//2 - Group

		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2008-01-03 10:56) - PLID 28454 - needed to submit as a group if using an override,
		// because otherwise we have to send a first name field, but that also means we need to check
		// the Box33Name override here first

		// (j.jones 2007-05-07 16:48) - PLID 25922 - added Box33Name override
		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		CString strBox33NameOver = "", strBox33Address1_Over = "", strBox33Address2_Over = "",
			strBox33City_Over = "", strBox33State_Over = "", strBox33Zip_Over = "";
		BOOL bUseBox33NameOver = FALSE;
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType != actInst) {
			// (j.jones 2008-05-06 14:06) - PLID 27453 - parameterized
			_RecordsetPtr rsBox33Over = CreateParamRecordset("SELECT Box33Name, "
				"Box33_Address1, Box33_Address2, Box33_City, Box33_State, Box33_Zip "
				"FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
			if(!rsBox33Over->eof) {

				//the Box 33 name override is only supported if the "Use Override" Box 33 setting is in use
				if(m_HCFAInfo.Box33Setup == 3) {
					strBox33NameOver = AdoFldString(rsBox33Over, "Box33Name","");
					strBox33NameOver.TrimLeft();
					strBox33NameOver.TrimRight();

					if(!strBox33NameOver.IsEmpty())
						bUseBox33NameOver = TRUE;
				}

				// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
				// (these do not need a boolean as well)
				strBox33Address1_Over = AdoFldString(rsBox33Over, "Box33_Address1","");
				strBox33Address1_Over.TrimLeft();
				strBox33Address1_Over.TrimRight();
				strBox33Address2_Over = AdoFldString(rsBox33Over, "Box33_Address2","");
				strBox33Address2_Over.TrimLeft();
				strBox33Address2_Over.TrimRight();
				strBox33City_Over = AdoFldString(rsBox33Over, "Box33_City","");
				strBox33City_Over.TrimLeft();
				strBox33City_Over.TrimRight();
				strBox33State_Over = AdoFldString(rsBox33Over, "Box33_State","");
				strBox33State_Over.TrimLeft();
				strBox33State_Over.TrimRight();
				strBox33Zip_Over = AdoFldString(rsBox33Over, "Box33_Zip","");
				strBox33Zip_Over.TrimLeft();
				strBox33Zip_Over.TrimRight();
			}
			rsBox33Over->Close();
		}

		CString strProviderCompany = "";
		BOOL bUseProviderCompanyOnClaims = FALSE;

		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
		OutputString += ParseANSIField(str,1,1);

		//NM103	1035		Name Last or Organization Name			O	AN	1/35

		//show the Location Name as the group name if m_bIsGroup is TRUE.

		str = "";
		// (j.jones 2008-01-03 11:14) - PLID 28454 - check for overrides first, as they are
		// allowed even when sending as a group
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
		OutputString += ParseANSIField(str,1,35);

		//NM104	1036		Name First								O	AN	1/25

		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
		OutputString += ParseANSIField(str,1,25);

		//NM105	1037		Name Middle								O	AN	1/25

		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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

		//NM107	1039		Name Suffix								O	AN	1/10

		//PersonT.Title
		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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

		//NM108	66			Identification Code	Qualifier			X	ID	1/2

		CString strID, strIdent;

		// (j.jones 2006-11-14 08:55) - PLID 23413 - determine whether the
		// NM108/109 elements should show the NPI or the EIN/SSN
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		long nNM109_IDType = 0;		
		if(m_actClaimType == actInst)
			// (j.jones 2006-11-14 08:56) - PLID 23446 - check the UB92 setup
			nNM109_IDType = m_UB92Info.NM109_IDType;
		else
			nNM109_IDType = m_HCFAInfo.NM109_IDType;

		if(nNM109_IDType == 1) {

			// (j.jones 2006-11-14 08:57) - PLID 23413, 23446 - if 1, then
			// use the EIN or SSN, depending on the group or UB92 usage

			//"24" for EIN, "34" for SSN - from HCFASetupT.Box25

			//If a group, show the location's EIN, ID of 24")

			// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
			// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
			if(m_bIsGroup || m_actClaimType == actInst || m_HCFAInfo.Box33Setup == 4) {
				//a group
				strIdent = "24";
				strID = GetFieldFromRecordset(rs,"EIN");
			}
			else {
				//individual doctor

				if(m_HCFAInfo.Box25==1)
					strIdent = "34";
				else
					strIdent = "24";

				if(m_HCFAInfo.Box25==1) {
					var = fields->Item["SocialSecurity"]->Value;
					if(var.vt == VT_BSTR)
						strID = CString(var.bstrVal);
					else
						strID = "";
				}
				else {
					var = fields->Item["Fed Employer ID"]->Value;
					if(var.vt == VT_BSTR)
						strID = CString(var.bstrVal);
					else
						strID = "";
				}
			}
		}
		else {

			//"XX" for NPI
			strIdent = "XX";
			strID = "";

			// (j.jones 2006-11-14 08:58) - PLID 23413, 23446 - if 0,
			// then use the NPI, may be location NPI if group or UB92

			// (j.jones 2007-01-18 11:51) - PLID 24264 - supported the
			// NPI selection in HCFA Box 33, to determine whether to use
			// the Location NPI or Provider NPI. If a UB92 or a Group,
			// always use location (would already be loaded in the active
			// recordset), if a non-Group HCFA, we need to load the location
			// NPI number.

			// (j.jones 2007-08-08 13:14) - PLID 25395 - if there is a 33a override in AdvHCFAPinT, use it here
			// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
							strID = strOver;
					}
				}
				rsOver->Close();
			}

			if(strID.IsEmpty()) {

				// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_HCFAInfo.LocationNPIUsage == 1 && !m_bIsGroup && m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 4) {
					//a non-group HCFA, meaning that the open recordset does
					//not contain the location NPI, and we want it
					
					// (j.jones 2008-05-06 14:06) - PLID 29937 - now the NPI is in the loaded claim pointer
					strID = m_pEBillingInfo->strBillLocationNPI;
				}
				else {
					//whether it's provider or not, the open recordset has
					//the NPI we are looking for
					strID = VarString(fields->Item["NPI"]->Value, "");
				}
			}
		}

		strID = StripNonNum(strID);

		if(strID != "")
			OutputString += ParseANSIField(strIdent,1,2);
		else
			OutputString += "*";

		//NM109	67			Identification Code						X	AN	2/80

		//ProvidersT.Fed Employer ID or PersonT.SocialSecurity
		//depends on setting in HCFASetupT.Box25

		if(strID != "")		
			OutputString += ParseANSIField(strID,2,80);
		else
			OutputString += "*";

		//NM110 NOT USED
		OutputString += "*";

		//NM111	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//103		025		N3		Pay-To Provider Address					R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//if DocAddress == 1, and we're loading a provider, load the Location address,
		//else use the address in the already open recordset
		//(which will be Contacts if a provider, or else a Location)

		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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

		//N301	166			Address Information						M	AN	1/55

		//PersonT.Address1

		var = fields->Item["Address1"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33Address1_Over.IsEmpty()) {
			str = strBox33Address1_Over;
		}

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str,1,55);

		//N302	166			Address Information						O	AN	1/55

		//PersonT.Address2

		//This should not be used if the address 2 is blank.
		//EndANSISegment will trim off the *.

		var = fields->Item["Address2"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33Address2_Over.IsEmpty()) {
			str = strBox33Address2_Over;
		}

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str,1,55);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//104		030		N4		Pay-To Provider City/State/Zip			R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O	AN	2/30

		//PersonT.City

		var = fields->Item["City"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33City_Over.IsEmpty()) {
			str = strBox33City_Over;
		}

		OutputString += ParseANSIField(str,2,30);

		//N402	156			State or Province Code					O	ID	2/2

		//PersonT.State

		var = fields->Item["State"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33State_Over.IsEmpty()) {
			str = strBox33State_Over;
		}

		OutputString += ParseANSIField(str,2,2);

		//N403	116			Postal Code								O	ID	3/15

		//PersonT.Zip

		var = fields->Item["Zip"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		// (j.jones 2010-02-01 10:24) - PLID 37138 - supported Box33 address overrides
		if(!strBox33Zip_Over.IsEmpty()) {
			str = strBox33Zip_Over;
		}

		str = StripNonNum(str);
		OutputString += ParseANSIField(str,3,15);

		//N404	26			Country Code							O	ID	2/3

				//this is only required if the addess is outside the US
				//as of right now, we can't support it
		
		OutputString += "*";

		//N405	NOT USED
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//106		035		REF		Pay-To Provider Secondary Information	S		8

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		// (j.jones 2008-05-02 10:09) - PLID 27478 - we need to silently skip
		// adding additional REF segments that have the same qualifier, so
		// track them in an array
		CStringArray arystrQualifiers;

		//REF01	128			Reference Identification Qualifier		M	ID	2/3

		//Box33PinANSI will give us the qualifier AND ID, even if the ID is blank
		strIdent = "";
		strID = "";
		CString strLoadedFrom = "";
		// (j.jones 2010-04-14 09:04) - PLID 38194 - the ID is now calculated in a shared function
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		EBilling_Calculate2010_REF(TRUE, strIdent, strID, strLoadedFrom, m_actClaimType == actInst,
			m_pEBillingInfo->ProviderID, m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->BillLocation, m_pEBillingInfo->InsuredPartyID,
			m_pEBillingInfo->HCFASetupID, m_HCFAInfo.Box33GRP,
			m_pEBillingInfo->UB92SetupID, m_UB92Info.UB04Box76Qual, m_UB92Info.Box82Num, m_pEBillingInfo->Box82Setup);

		// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
		if(m_bDisableREFXX && strIdent.CompareNoCase("XX") == 0) {
			strIdent = "";
			strID = "";
			//log that this happened
				Log("Ebilling file tried to export 2010AB REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
		}

		OutputString += ParseANSIField(strIdent,2,3);

		//REF02	127			Reference Identification				X	AN	1/30

		OutputString += ParseANSIField(strID,1,30);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		// (j.jones 2007-09-20 10:51) - PLID 27455 - do not send this segment if the
		// qualifier is blank
		if(strIdent != "" && strID != "") {
			EndANSISegment(OutputString);

			// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
			arystrQualifiers.Add(strIdent);
		}

		rs->Close();		

		// (j.jones 2006-11-14 11:48) - PLID 23413 - the HCFA Group now allows the option
		// to append an EIN/SSN, or NPI as an additional REF segment

		//reopen the proper recordset

		// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(!m_bIsGroup && m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 4) {
			//if individual
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			rs = CreateParamRecordset("SELECT * FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID WHERE ID = {INT}",m_pEBillingInfo->ProviderID);

			if(rs->eof) {
				//if the recordset is empty, there is no doctor. So halt everything!!!
				rs->Close();
				// (j.jones 2008-05-06 15:49) - PLID 27453 - reworked the way we gather the patient name
				CString strPatientName = GetExistingPatientName(m_pEBillingInfo->PatientID);
				if(!strPatientName.IsEmpty()) {
					// (j.jones 2009-09-16 17:16) - PLID 35561 - added notations to the error warnings
					str.Format("Could not open provider information for patient '%s', Bill ID %li. (ANSI_2010AB 2)", strPatientName, m_pEBillingInfo->BillID);
				}
				else
					//serious problems if you get here
					str = "Could not open provider information. (ANSI_2010AB 2)";

				// (j.jones 2007-05-10 12:07) - PLID 25948 - tweaked the error to be more accurate
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
			// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
			if(m_actClaimType != actInst && m_HCFAInfo.Box33Setup == 3) {
				// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM LocationsT CROSS JOIN (SELECT * FROM PersonT WHERE PersonT.ID = {INT}) AS PersonT WHERE LocationsT.ID = {INT}", m_HCFAInfo.Box33Num, m_pEBillingInfo->BillLocation);
			}
			else {
				// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
				rs = CreateParamRecordset("SELECT * FROM LocationsT WHERE ID = {INT}",m_pEBillingInfo->BillLocation);
			}

			if(rs->eof) {
				rs->Close();
				//if end of file, select from LocationsT, and the recordset will just pull from the first entry
				// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
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
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		long nExtraREF_IDType = 2;		
		if(m_actClaimType == actInst)
			// (j.jones 2006-11-14 11:49) - PLID 23446 - the UB92 Group has an option as well
			nExtraREF_IDType = m_UB92Info.ExtraREF_IDType;
		else
			nExtraREF_IDType = m_HCFAInfo.ExtraREF_IDType;

		//0 - EIN/SSN, 1 - NPI, 2 - none
		if(nExtraREF_IDType != 2) {

			//export the extra ID

			OutputString = "REF";

			//REF01	128			Reference Identification Qualifier		M	ID	2/3
			
			strIdent = "";
			strID = "";

			if(nExtraREF_IDType == 0) {
				//EIN/SSN

				//"EI" for EIN, "SY" for SSN - from HCFASetupT.Box25

				//If a group, show the location's EIN, ID of 24")

				// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_bIsGroup || m_actClaimType == actInst || m_HCFAInfo.Box33Setup == 4) {
					//a group
					strIdent = "EI";
					strID = GetFieldFromRecordset(rs,"EIN");
				}
				else {
					//individual doctor

					if(m_HCFAInfo.Box25==1)
						strIdent = "SY";
					else
						strIdent = "EI";

					if(m_HCFAInfo.Box25==1) {
						var = fields->Item["SocialSecurity"]->Value;
						if(var.vt == VT_BSTR)
							strID = CString(var.bstrVal);
						else
							strID = "";
					}
					else {
						var = fields->Item["Fed Employer ID"]->Value;
						if(var.vt == VT_BSTR)
							strID = CString(var.bstrVal);
						else
							strID = "";
					}
				}
			}
			else {
				//NPI

				//"XX" for NPI
				strIdent = "XX";
				strID = "";

				// (j.jones 2006-11-14 08:58) - PLID 23413, 23446 - if 0,
				// then use the NPI, may be location NPI if group or UB92

				// (j.jones 2007-01-18 11:51) - PLID 24264 - supported the
				// NPI selection in HCFA Box 33, to determine whether to use
				// the Location NPI or Provider NPI. If a UB92 or a Group,
				// always use location (would already be loaded in the active
				// recordset), if a non-Group HCFA, we need to load the location
				// NPI number.

				// (j.jones 2007-08-08 13:14) - PLID 25395 - if there is a 33a override in AdvHCFAPinT, use it here
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType != actInst) {
					// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
					_RecordsetPtr rsOver = CreateParamRecordset("SELECT Box33aNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
					if(!rsOver->eof) {
						var = rsOver->Fields->Item["Box33aNPI"]->Value;
						if(var.vt == VT_BSTR) {
							CString strOver = VarString(var,"");
							strOver.TrimLeft();
							strOver.TrimRight();
							if(!strOver.IsEmpty())
								strID = strOver;
						}
					}
					rsOver->Close();
				}

				if(strID.IsEmpty()) {

					// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
					// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
					if(m_HCFAInfo.LocationNPIUsage == 1 && !m_bIsGroup && m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 4) {
						//a non-group HCFA, meaning that the open recordset does
						//not contain the location NPI, and we want it
						
						// (j.jones 2008-05-06 15:19) - PLID 29937 - now the NPI is in the loaded claim pointer
						strID = m_pEBillingInfo->strBillLocationNPI;
					}
					else {
						//whether it's provider or not, the open recordset has
						//the NPI we are looking for
						strID = VarString(fields->Item["NPI"]->Value, "");
					}
				}
			}

			strID = StripNonNum(strID);

			OutputString += ParseANSIField(strIdent,2,3);

			//REF02	127			Reference Identification				X	AN	1/30

			OutputString += ParseANSIField(strID,1,30);

			//REF03	NOT USED
			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strIdent.CompareNoCase("XX") == 0) {
				strIdent = "";
				strID = "";
				//log that this happened
				Log("Ebilling file tried to export 2010AB Additional REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
			}

			// (j.jones 2008-05-02 10:02) - PLID 27478 - disallow sending the additional REF
			// segment if the qualifier has already been used
			if(strIdent != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdent)) {
				EndANSISegment(OutputString);

				// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
				arystrQualifiers.Add(strIdent);
			}
		}

		rs->Close();

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2010AB");

	return Error_Other;
}

int CEbilling::ANSI_4010_2000B() {

	//Subscriber Hierachical Level

	//If the insured and the patient are the same person, then the patient loop (2000C)
	//is not needed. So the 2000B loop will always be run, and 2000C will only be run if
	//the relation to patient is not "self".

#ifdef _DEBUG

	CString str;

	str = "\r\n2000B\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif


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
					"RelationToPatient, RespTypeID, PolicyGroupNum, InsurancePlansT.PlanName, InsType, "
					"InsuredPartyT.SecondaryReasonCode "
					"FROM InsuredPartyT INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
					"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
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

//108		001		HL		Subscriber Hierarchical Level			R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HL";

		//HL01	628			Hierarch ID Number						M	AN	1/12

		//m_ANSIHLCount
		str.Format("%li", m_ANSIHLCount);
		OutputString += ParseANSIField(str, 1, 12);

		//HL02	734			Hierarch Parent ID						O	AN	1/12

		//m_ANSIProviderCount
		str.Format("%li", m_ANSICurrProviderHL);
		OutputString += ParseANSIField(str, 1, 12);

		//HL03	735			Hierarch Level Code						M	ID	1/2

		//static value "22"
		OutputString += ParseANSIField("22", 1, 2);

		//HL04	736			Hierarch Child Code						O	ID	1/1

		var = fields->Item["RelationToPatient"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		if(str == "Self")
			str = "0";
		else
			str = "1";

		OutputString += ParseANSIField(str, 1, 1);

		EndANSISegment(OutputString);
		
///////////////////////////////////////////////////////////////////////////////

//110		005		SBR		Subscriber Information					R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "SBR";

		//SBR01	1138		Payer Resp Seq No Code					M	ID	1/1

		//JJ - 5/14/2003 - The description of this code is
		//"Code identifying the insurance carrier's level of 
		//responsibility for a payment of a claim"
		//So it's really not the RespTypeID, it is going to always be primary because
		//this function represents the insurance company that is primarily
		//reponsible for this claim.

		str = "P";

		// (j.jones 2006-12-06 11:46) - PLID 23787 - this code has to be secondary
		// if we are sending the previous payer's payments for a non-primary responsibility.
		// You can argue that it should always be "S" when non-primary, but I don't trust
		// the system enough to make that change until specifically required, as this is.

		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(!m_pEBillingInfo->IsPrimary && 
			((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1)
			|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1))) {

			str = "S";
		}

		/*
		var = fields->Item["RespTypeID"]->Value;

		if(var.vt == VT_I4) {
			if(var.lVal == 1) 
				str = "P";
			else if (var.lVal == 2) 
				str = "S";
			else
				str = "T";
		}
		*/

		OutputString += ParseANSIField(str, 1, 1);

		//SBR02	1069		Individual Relat Code					O	ID	2/2

		var = fields->Item["RelationToPatient"]->Value;
		if(var.vt == VT_BSTR) {
			if(CString(var.bstrVal) == "Self")
				str = "18";		//static value
			else
				str = "";
		}

		OutputString += ParseANSIField(str, 2, 2);				

		//SBR03	127			Reference Ident							O	AN	1/30

		//InsuredPartyT.PolicyGroupNum
		str = GetFieldFromRecordset(rs,"PolicyGroupNum");
		OutputString += ParseANSIField(str,1,30);

		//SBR04	93			Name									O	AN	1/60

		//InsurancePlansT.PlanName
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType == actInst && str != "")
			//on UB92s only, don't export the name if the group number exists
			str = "";
		else
			str = GetFieldFromRecordset(rs,"PlanName");
		OutputString += ParseANSIField(str,1,60);

		//SBR05	1336		Insurance Type Code						O	ID	1/3
		
		// (j.jones 2007-04-26 09:09) - PLID 25800 - This is only used when
		// Medicare is secondary, and we're sending S in SBR01. It is also
		// not used on the UB92.

		// (j.jones 2008-09-09 10:08) - PLID 18695 - converted NSF Code to InsType
		//InsType is used in SBR09, but we need it now
		InsuranceTypeCode eInsType = (InsuranceTypeCode)AdoFldLong(rs, "InsType", (long)itcInvalid);

		str = "";

		// (j.jones 2010-10-15 14:49) - PLID 40953 - added Part A as well
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(!m_pEBillingInfo->IsPrimary && m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1
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

		//SBR09	1032		Claim File Ind Code						O	ID	1/2

		//InsuranceCoT.InsType
		
		// (j.jones 2008-09-09 10:10) - PLID 18695 - we finally track proper
		// Insurance Types per company, so simply call GetANSISBR09CodeFromInsuranceType()
		// to get the ANSI code for our type.

		//we loaded the InsType earlier in SBR05
		// (j.jones 2010-10-15 14:47) - PLID 40953 - function name changed, content stays the same
		str = GetANSI4010_SBR09CodeFromInsuranceType(eInsType);
		if(str.IsEmpty()) {
			str = "ZZ";
		}

		OutputString += ParseANSIField(str,1,2);

		EndANSISegment(OutputString);


///////////////////////////////////////////////////////////////////////////////

//114		007		PAT		Patient Information						S		1
		//this is birth and death information - nothing we will ever use

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2000B");

	return Error_Other;
}

int CEbilling::ANSI_4010_2010BA() {

	//Subscriber Name

#ifdef _DEBUG

	CString str;

	str = "\r\n2010BA\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		rs = CreateParamRecordset("SELECT * FROM InsuredPartyT INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID WHERE PersonID = {INT}", 
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

//117		015		NM1		Subscriber Name							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M	ID	2/3

		//static value
		OutputString += ParseANSIField("IL", 2, 3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//1 - individual
		//2 - non-person entity (company

		BOOL bIsCompany = FALSE;
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType == actInst && m_UB92Info.ShowCompanyAsInsurer == 1) {
			bIsCompany = TRUE;
		}

		str = "1";
		if(bIsCompany)
			str = "2";

		OutputString += ParseANSIField(str, 1, 1);

		//NM103	1035		Name Last/Org Name						O	AN	1/35

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
		OutputString += ParseANSIField(str, 1, 35);

		//NM104	1036		Name First								O	AN	1/25

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
		OutputString += ParseANSIField(str, 1, 25);

		//NM105	1037		Name Middle								O	AN	1/25

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

		//NM107	1039		Name Suffix								O	AN	1/10

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

		//NM108	66			ID Code Qualifer						X	ID	1/2

		//static value MI

		str = GetFieldFromRecordset(rs,"IDForInsurance");
		str = StripPunctuation(str);
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

		EndANSISegment(OutputString);

		//everything after NM1 is required if the subscriber is the patient (relation = "self")

///////////////////////////////////////////////////////////////////////////////

//121		025		N3		Subscriber Address						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";
		
		//N301	166			Address Information						M	AN	1/55

		var = fields->Item["Address1"]->Value;		//insured party addr1

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information					M	AN	1/55

		var = fields->Item["Address2"]->Value;		//insured party addr2

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//122		030		N4		Subscriber City/State/Zip				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O	AN	2/30

		var = fields->Item["City"]->Value;			//insured party City

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State of Prov Code						O	ID	2/2

		var = fields->Item["State"]->Value;			//insured party state

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O	ID	3/15

		var = fields->Item["Zip"]->Value;			//insured party Zip code

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str = StripNonNum(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O	ID	2/3

		OutputString += "*";

		//N405	NOT USED

		OutputString += "*";

		//N406	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//124		032		DMG		Subscriber Demographic Information		S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DMG";

		//DMG01	1250		Date Time format Qual					X	ID	2/3

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

		//DMG02	1251		Date Time Period						X	AN	1/35
		
		OutputString += ParseANSIField(str, 1, 35);

		//DMG03	1068		Gender Code								O	ID	1/1

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
		
		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//126		035		REF		Subscriber Secondary Information		S		4

		//This is an optional segment, and we're not sure if it's really necessary,
		//but since we have the data, we'll use it.

				
		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01	128			Reference Ident Qual					M	ID	2/3

		//"IG" - Policy Group Number
		//"SY" - SSN
		CString strQual, strNum;
		// (j.jones 2008-05-06 11:39) - PLID 29937 - cached m_bUseSSN
		if(m_bUseSSN) {
			strQual = "SY";
		}
		else {
			strQual = "IG";
		}

		OutputString += ParseANSIField(strQual, 2, 3);

		//REF02	127			Reference Ident							X	AN	1/30

		//InsuredPartyT.PolicyGroupNum

		// (j.jones 2008-05-06 11:39) - PLID 29937 - cached m_bUseSSN
		if(m_bUseSSN) {
			var = fields->Item["SocialSecurity"]->Value;
		}
		else {
			var = fields->Item["PolicyGroupNum"]->Value;
		}

		if(var.vt == VT_BSTR)
			strNum = CString(var.bstrVal);
		else
			strNum = "";

		strNum.TrimRight();

		OutputString += ParseANSIField(strNum, 1, 30);

		//REF03	NOT USED

		OutputString += "*";

		//REF04	NOT USED

		OutputString += "*";

		//if these is no number, don't output
		if(strNum != "")
			EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//128		035		REF		Property and Casualty Claim Number		S		1

		//not used
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2010BA");

	return Error_Other;
}

int CEbilling::ANSI_4010_2010BB() {

	//Payer Name

	//note: on the UB92, Institutional Format, this is actually the 2010BC loop

#ifdef _DEBUG

	CString str;

	str = "\r\n2010BB\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

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

//130		015		NM1		Payer Name								R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M	ID	2/3

		//static value

		OutputString += ParseANSIField("PR", 2, 3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//static value

		OutputString += ParseANSIField("2", 1, 1);

		//NM103	1035		Name Last / Org Name					O	AN	1/35

		var = fields->Item["Name"]->Value;				//InsuranceCoT.Name

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		OutputString += ParseANSIField(str, 1, 35);

		//NM104	NOT USED

		OutputString += "*";

		//NM105	NOT USED

		OutputString += "*";

		//NM106	NOT USED

		OutputString += "*";

		//NM107	NOT USED

		OutputString += "*";

		//NM108	66			ID Code Qualifier						X	ID	1/2

		//static "PI"

		// (j.jones 2009-08-05 10:20) - PLID 34467 - this is now in the loaded claim info
		// (j.jones 2009-12-16 15:54) - PLID 36237 - split out into HCFA and UB payer IDs
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType == actInst) {
			str = m_pEBillingInfo->strUBPayerID;
		}
		else {
			str = m_pEBillingInfo->strHCFAPayerID;
		}

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

		//NM109	67			ID Code									X	AN	2/80

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

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//134		025		N3		Payer Address							S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//N301	166			Address Information						M	AN	1/55

		var = fields->Item["Address1"]->Value;			//insurance co address1

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information						O	AN	1/55

		var = fields->Item["Address2"]->Value;			//insurance co address2

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//135		030		N4		Payer City/State/Zip					S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O	AN	2/30

		var = fields->Item["City"]->Value;				//insurance co city

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State or Prov Code						O	ID	2/2

		var = fields->Item["State"]->Value;				//insurance co state

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O	ID	3/15

		var = fields->Item["Zip"]->Value;				//insurance co zip

		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str = StripNonNum(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O	ID	2/3

		//we don't use this
		OutputString += "*";

		//N405	NOT USED
		
		OutputString += "*";

		//N406	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//137		035		REF		Payer Secondary Information				S		3

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01	128			Reference Ident Qual					M	ID	2/3

		//static "FY"
		OutputString += ParseANSIField("FY",2,3);

		//REF02	127			Reference Ident							X	AN	1/30

		//InsuranceCoT.EBillingClaimOffice
		str = GetFieldFromRecordset(rs,"EBillingClaimOffice");
		OutputString += ParseANSIField(str,1,30);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		if(str != "")
			EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2010BB");

	return Error_Other;
}

/*
int CEbilling::ANSI_4010_2010BC() {

	//Responsible Party Name

	//note: on the UB92, Institutional Format, this is actually the 2010BD loop

	//JJ - this is only used when the responsible party for the claim is neither
	//the subscriber nor the patient. We can't and won't do this in our software.

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2010BC - RESPONSIBLE PARTY NAME						1

//139		015		NM1		Responsible Party Name					S		1
//143		025		N3		Responsible Party Address				R		1
//144		030		N4		Responsible Party City/State/Zip		R		1

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2010BC");

	return Error_Other;
}
*/

/*
int CEbilling::ANSI_4010_2010BD() {

	//Credit/Debit Card Holder Name

	//note: on the UB92, Institutional Format, this is actually the 2010BB loop

	//JJ - Not used. Not only is it not a function of our system, but ANSI recommends this
	//information never be sent to a payer.

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2010BD - CREDIT/DEBIT CARD HOLDER NAME				1

//146		015		NM1		Credit/Debit Card Holder Name			S		1
//150		035		REF		Credit/Debit Card Information			S		2

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2010BD");

	return Error_Other;
}
*/

int CEbilling::ANSI_4010_2000C() {

	//Patient Hierarchical Level

	//If the insured and the patient are the same person, then this is not needed.
	//So the 2000B loop will always be run, and 2000C & 2010CA will only be run if
	//the relation to patient is not "self".

#ifdef _DEBUG

	CString str;

	str = "\r\n2000C\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

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


//152		001		HL		Patient Hierarchical Level				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HL";

		//HL01	628			Hierarch ID Number						M	AN	1/12

		//m_ANSIHLCount
		str.Format("%li", m_ANSIHLCount);

		OutputString += ParseANSIField(str, 1, 12);

		//HL02	734			Hierarch Parent ID						O	AN	1/12

		//m_ANSICurrPatientParent
		str.Format("%li", m_ANSICurrPatientParent);

		OutputString += ParseANSIField(str, 1, 12);

		//HL03	735			Hierarch Level Code						M	ID	1/2

		//static value "23"

		OutputString += ParseANSIField("23", 1, 2);

		//HL04	736			Hierarch Child Code						O	ID	1/1

		//static value "0"

		OutputString += ParseANSIField("0", 1, 1);

		EndANSISegment(OutputString);

//154		007		PAT		Patient Information						R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "PAT";

		//PAT01	1069		Individual Relat Code					O	ID	2/2

		//The values are all listed on page 155

		var = fields->Item["RelationToPatient"]->Value;

		if(var.vt == VT_BSTR) {
			str = CString(var.bstrVal);

			if(str == "Child")
				str = "19";
			else if (str == "Spouse")
				str = "01";
			else if (str == "Other")
				str = "G8";
			else if (str == "Employee")
				str = "20";
			else if (str == "Unknown")
				str = "21";
			else if (str == "Organ Donor")
				str = "39";
			else if (str == "Cadaver Donor")
				str = "40";
			else if (str == "Life Partner")
				str = "53";
			else
				str = "21";

			// (j.jones 2011-06-15 17:00) - PLID 40959 - the following are no longer valid entries in our system,
			// and no longer exist in data
			/*
			else if (str == "Grandparent")
				str = "04";
			else if (str == "Grandchild")
				str = "05";
			else if (str == "Nephew Or Niece")
				str = "07";
			else if (str == "Adopted Child")
				str = "09";
			else if (str == "Foster Child")
				str = "10";
			else if (str == "Ward")
				str = "15";
			else if (str == "Stepchild")
				str = "17";
			else if (str == "Handicapped Dependent")
				str = "22";
			else if (str == "Sponsored Dependent")
				str = "23";
			else if (str == "Dependent of a Minor Dependent")
				str = "24";
			else if (str == "Significant Other")
				str = "29";
			else if (str == "Mother")
				str = "32";
			else if (str == "Father")
				str = "33";
			else if (str == "Other Adult")
				str = "34";
			else if (str == "Emancipated Minor")
				str = "36";
			else if (str == "Injured Plaintiff")  // (s.dhole 2010-08-31 15:13) - PLID 40114 All our relationship lists say "Injured Plantiff" instead of "Injured Plaintiff"
				str = "41";
			else if (str == "Child Where Insured Has No Financial Responsibility")
				str = "43";
			*/
		}

		OutputString += ParseANSIField(str, 2, 2);

		//PAT02	NOT USED

		OutputString += "*";

		//PAT03	NOT USED

		OutputString += "*";

		//PAT04	NOT USED

		OutputString += "*";

		//PAT05	1250		Date Time format Qual					X	ID	2/3

		//not used by us - only if patient is deceased
		//not used on the UB92

		OutputString += "*";

		//PAT06	1251		Date Time Period						X	AN	1/35

		//not used by us - only if patient is deceased
		//not used on the UB92

		OutputString += "*";

		//PAT07	355			Unit/basis Meas Code					X	ID	2/2

		//not used by us - only if birth

		OutputString += "*";

		//PAT08	81			Weight									X	R	1/10

		//not used by us - only if birth

		OutputString += "*";

		//PAT09	1073		Yes/No Cond Resp Code					O	ID	1/1

		//not used by us - only if pregnancy

		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2000C");

	return Error_Other;
}

int CEbilling::ANSI_4010_2010CA() {

	//Patient Name

	//If the insured and the patient are the same person, then this is not needed.
	//So the 2000B loop will always be run, and 2000C & 2010CA will only be run if
	//the relation to patient is not "self".

#ifdef _DEBUG

	CString str;

	str = "\r\n2010CA\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		rs = CreateParamRecordset("SELECT InsuredPartyT.RelationToPatient, PersonT2.Last, PersonT2.First, PersonT2.Middle, PersonT2.Title,  "
						"PersonT2.Address1, PersonT2.Address2, PersonT2.City, PersonT2.State, PersonT2.Zip, PersonT2.BirthDate, PersonT2.Gender, "
						"InsuredPartyT.Patient_IDForInsurance "
						"FROM InsuredPartyT INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
						"LEFT JOIN PersonT PersonT2 ON InsuredPartyT.PatientID = PersonT2.ID "
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

//157		015		NM1		Patient Name							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M	ID	2/3

		//static value "QC"

		OutputString += ParseANSIField("QC", 2, 3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//static value "1"

		OutputString += ParseANSIField("1", 1, 1);

		//NM103	1035		Name Last / Org Name					O	AN	1/35

		str = GetFieldFromRecordset(rs, "Last");		//patient last name

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 35);

		//NM104	1036		Name First								O	AN	1/25

		str = GetFieldFromRecordset(rs, "First");		//patient first name

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 25);

		//NM105	1037		Name Middle								O	AN	1/25

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

		//NM108	66			ID Code Qualifer						X	ID	1/2

		//static value MI

		// (j.jones 2010-05-18 11:05) - PLID 37788 - this is only sent
		// if the patient has a different ID For Insurance than the subscriber,
		// which is not an option in our system unless a preference is enabled
		// to provide two options

		str = "";

		if(GetRemotePropertyInt("ShowPatientIDForInsurance", 0, 0, "<None>", true) == 1) {
			str = GetFieldFromRecordset(rs,"Patient_IDForInsurance");
			str = StripPunctuation(str);
		}
		
		//if no ID, do not output
		if(str != "")
			OutputString += ParseANSIField("MI", 1, 2);
		else
			OutputString += "*";

		//NM109	67			ID Code									X	AN	2/80

		//InsuredPartyT.Patient_IDForInsurance
		
		//if no ID, do not output
		if(str != "")
			OutputString += ParseANSIField(str,2,80);
		else
			OutputString += "*";

		//NM110	NOT USED

		OutputString += "*";

		//NM111	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

//161		025		N3		Patient Address							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//N301	166			Address Information						M	AN	1/55

		str = GetFieldFromRecordset(rs, "Address1");		//patient address1

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information						O	AN	1/55

		str = GetFieldFromRecordset(rs, "Address2");		//patient address2

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

//162		030		N4		Patient City/State/Zip					R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O	AN	2/30

		str = GetFieldFromRecordset(rs, "City");		//patient city

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State or Prov Code						O	ID	2/2

		str = GetFieldFromRecordset(rs, "State");		//patients state
		
		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O	ID	3/15

		str = GetFieldFromRecordset(rs, "Zip");			//patients zip
		str = StripNonNum(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O	ID	3/15

		//not used by us
		OutputString += "*";

		//N405	NOT USED
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

//164		032		DMG		Patient Demographic Information			R		1

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

		EndANSISegment(OutputString);

//166		035		REF		Patient Secondary Information			S		5

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//This information will already be in the subscriber loop,
		//so we don't need this segment.

		/*
		OutputString = "REF";

		//REF01	128			Reference Ident Qual					X	AN	1/30

			
		//REF02	127			Reference Ident							X	AN	1/30

			
		//REF03	NOT USED

		OutputString += "*";

		//REF04	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);
		*/

//168		035		REF		Property and Casualty Claim Number		S		1

		//not used

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2010CA");

	return Error_Other;
}

// (j.jones 2011-03-07 14:09) - PLID 42660 - the CLIA number is now calculated by our caller and is passed in
// (j.jones 2011-04-05 14:31) - PLID 42372 - now we pass in a struct
// (j.jones 2011-04-20 10:45) - PLID 41490 - we now pass in info. on skipping diagnosis codes
int CEbilling::ANSI_4010_2300_Prof(ECLIANumber eCLIANumber)
{

	//Claim Information

	//if the patient is the subscriber, 2300 loop follows the 2010BD loop
	//if the patient is not the subscriber, 2300 loop follows 2010CA

#ifdef _DEBUG

	CString str;

	str = "\r\n2300\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	//increment the count for the current claim
	m_ANSIClaimCount++;

	try {

		CString OutputString,str;
		_variant_t var;

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		// (j.jones 2009-03-09 09:41) - PLID 33354 - added InsuredPartyT accident information
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		// (j.gruber 2014-03-17 10:33) - PLID 61394 - support new billing structure for 4010 - no changes for ICD-10
		// (a.walling 2014-03-19 10:11) - PLID 61418 - EBilling - ANSI4010 - Get diag info from bill
		// (j.jones 2016-04-11 17:22) - NX-100149 - Box15Date is now calculated from ConditionDateType
		_RecordsetPtr rs = CreateParamRecordset("SELECT BillsT.*, PlaceOfServiceCodesT.PlaceCodes, "
			"InsuredPartyT.AccidentType, InsuredPartyT.AccidentState, InsuredPartyT.DateOfCurAcc, "
			"CASE BillsT.ConditionDateType "
			"WHEN {CONST_INT} THEN BillsT.FirstVisitOrConsultationDate "
			"WHEN {CONST_INT} THEN BillsT.InitialTreatmentDate "
			"WHEN {CONST_INT} THEN BillsT.LastSeenDate "
			"WHEN {CONST_INT} THEN BillsT.AcuteManifestationDate "
			"WHEN {CONST_INT} THEN BillsT.LastXRayDate "
			"WHEN {CONST_INT} THEN BillsT.HearingAndPrescriptionDate "
			"WHEN {CONST_INT} THEN BillsT.AssumedCareDate "
			"WHEN {CONST_INT} THEN BillsT.RelinquishedCareDate "
			"WHEN {CONST_INT} THEN BillsT.AccidentDate "
			"ELSE NULL END AS Box15Date "
			"FROM BillsT "
			"INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE BillsT.ID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null",
			ConditionDateType::cdtFirstVisitOrConsultation444,
			ConditionDateType::cdtInitialTreatmentDate454,
			ConditionDateType::cdtLastSeenDate304,
			ConditionDateType::cdtAcuteManifestation453,
			ConditionDateType::cdtLastXray455,
			ConditionDateType::cdtHearingAndPrescription471,
			ConditionDateType::cdtAssumedCare090,
			ConditionDateType::cdtRelinquishedCare91,
			ConditionDateType::cdtAccident439,
			m_pEBillingInfo->BillID);
			
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

//170		130		CLM		Claim Information						R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "CLM";

		//CLM01	1028		Claim Submitter's Identifier			M	AN	1/38

		//PatientsT.UserDefinedID
		
		str.Format("%li",m_pEBillingInfo->UserDefinedID);

		// (j.jones 2009-10-01 14:21) - PLID 35711 - added the ability to insert text in front of the patient ID
		if(m_bPrependPatientID) {
			str = m_strPrependPatientIDCode + str;
		}

		OutputString += ParseANSIField(str,1,38);

		//CLM02	782			Monetary Amount							O	R	1/18

		//this is the total charge amount for the claim

		CString strDocProv = "";
		//if using the bill provider, filter only on that doctor's charges, otherwise get all charges
		//in ANSI, we're using the bill provider if we are using anything but the G1 provider

		// (j.jones 2006-12-01 15:50) - PLID 22110 - supported the ClaimProviderID
		// (j.jones 2008-04-03 09:23) - PLID 28995 - supported the HCFAClaimProvidersT setup
		// (j.jones 2008-08-01 10:17) - PLID 30917 - HCFAClaimProvidersT is now per insurance company
		// (d.thompson 2011-09-12) - PLID 45393 - Properly follow ClaimProviderID on the charge when filtering the charges
		if(m_HCFAInfo.Box33Setup != 2) {
			strDocProv.Format("AND (ChargesT.ClaimProviderID = %li OR (ClaimProviderID IS NULL AND DoctorsProviders IN "
				"(SELECT PersonID FROM "
				"	(SELECT ProvidersT.PersonID, "
				"	(CASE WHEN ANSI_2010AA_ProviderID Is Null THEN ProvidersT.ClaimProviderID ELSE ANSI_2010AA_ProviderID END) AS ProviderIDToUse "
				"	FROM ProvidersT "
				"	LEFT JOIN (SELECT * FROM HCFAClaimProvidersT WHERE InsuranceCoID = %li) AS HCFAClaimProvidersT ON ProvidersT.PersonID = HCFAClaimProvidersT.ProviderID) SubQ "
				"WHERE ProviderIDToUse = %li))) ", m_pEBillingInfo->ProviderID, m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);
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

			//see if we need to trim the right zeros
			// (j.jones 2008-05-06 11:42) - PLID 29937 - cached m_bTruncateCurrency
			if(m_bTruncateCurrency) {
				str.TrimRight("0");	//first the zeros
				str.TrimRight(".");	//then the decimal, if necessary
			}

			str.Replace(GetCurrencySymbol(),"");
			str.Replace(GetThousandsSeparator(),"");
		}
		else
			str = "0";
		OutputString += ParseANSIField(str,1,18);

		rsCharges->Close();

		//CLM03	NOT USED
		OutputString += "*";

		//CLM04	NOT USED
		OutputString += "*";

		//CLM05	C023		Health Care Service Location Info.		O

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

		//CLM05-2	NOT USED

		//only output if there is a place of service code
		if(str != "")
			OutputString += ":";

		//CLM05-3	1325	Claim Frequency Type Code				O	ID	1/1

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

		//CLM06	1073		Yes/No Cond Resp Code					O	ID	1/1

		//static "Y" (Signature On File)
		OutputString += ParseANSIField("Y",1,1);

		//CLM07	1359		Provider Accept Code					O	ID	1/1

		//InsuranceAcceptedT.Accepted
		str = "C";
		
		// (j.jones 2010-07-23 15:02) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsCo now
		BOOL bAccepted = GetAcceptAssignment_ByInsCo(m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);

		if(bAccepted) {
			str = "A";
		}
		OutputString += ParseANSIField(str,1,1);

		//CLM08	1073		Yes/No Cond Resp Code					O	ID	1/1

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

		//CLM09	1363		Release of Information Code					O	ID	1/1

		//Code indicating whether the provider has on file a signed statement by the patient
		//authorizing the release of medical data to other organizations

		//A -	Appropriate Release of Information on File at Health
		//		Care Service Provider or at Utilization Review Organization
		//I -	Informed Consent to Release Medical Information
		//		for Conditions or Diagnoses Regulated by Federal Statutes
		//M -	The Provider has Limited or Restricted Ability to
		//		Release Data Related to a Claim
		//N -	No, Provider is Not Allowed to Release Data
		//O -	On file at Payor or at Plan Sponsor
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
			str = "N";
		}
		OutputString += ParseANSIField(str,1,1);

		//CLM10	1351		Patient Signature Source Code			O	ID	1/1

		//Code indicating how the patient or subscriber authorization signatures were
		//obtained and how they are being retained by the provider

		//B -	Signed signature authorization form or forms for
		//		both HCFA-1500 Claim Form block 12 and block 13
		//		are on file
		//C -	Signed HCFA-1500 Claim Form on file
		//M -	Signed signature authorization form for HCFA-1500
		//		Claim Form block 13 on file
		//P	-	Signature generated by provider because the patient
		//		was not physically present for services
		//S -	Signed signature authorization form for HCFA-1500
		//		Claim Form block 12 on file

		//CLM10 is required except in cases where code ‘‘N’’ is used in CLM09.

		// (j.jones 2010-07-23 11:24) - PLID 39795 - we used to always send C,
		// but now we will send nothing if Box 12 is not set to be filled in

		if(bFillBox12) {
			str = "C";
		}
		else {
			str = "";
		}
		OutputString += ParseANSIField(str,1,1);

		//CLM11	C024		Related Causes Information				O

		//CLM11-1	1362	Related-Causes Code						M	ID	2/3

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
			
		//CLM11-2	1362	Related-Causes Code						O	ID	2/3
			//not used
		if(str != "")
			OutputString += ":";

		//CLM11-3	1362	Related-Causes Code						O	ID	2/3
			//not used
		if(str != "")
			OutputString += ":";

		//CLM11-4	156		State or Province Code					O	ID	2/2

		//BillsT.State
		//only used if BillsT.RelatedToAutoAcc is TRUE

		if(str != "") {
			OutputString += ":";
			//str has the state in it
			OutputString += str;
		}

		//CLM11-5	26		Country Code							O	ID	2/3
		OutputString += "*";

		//CLM12	1366		Special Program Code					O	ID	2/3
			//not used
		OutputString += "*";

		//CLM13	NOT USED
		OutputString += "*";
		//CLM14	NOT USED
		OutputString += "*";
		//CLM15	NOT USED
		OutputString += "*";

		//CLM16	1360		Provider Agreement Code					O	ID	1/1
			//not used
		OutputString += "*";

		//CLM17	NOT USED
		OutputString += "*";
		//CLM18	NOT USED
		OutputString += "*";
		//CLM19	NOT USED
		OutputString += "*";

		//CLM20	1514		Delay Reason Code						O	ID	1/2
			//not used
		OutputString += "*";

		EndANSISegment(OutputString);

//180		135		DTP		Date - Order Date						S		1
//182		135		DTP		Date - Initial Treatment				S		1

		//not used

//184		135		DTP		Date - Referral Date					S		1

		//??? I think we need this but have no data for it
		//TODO: Investigate and figure out what to put here.
		//The book claims "Required when claim includes a referral."

		//DTP01	374			Date/Time Qualifier						M	ID	3/3

		//DTP02	1250		Date Time format Qual					M	AN	1/35

		//DTP03	1251		Date Time Period						M	AN	1/35

//186		135		DTP		Date - Date Last Seen					S		1

		//not used

//188		135		DTP		Date - Onset of Current Illness/Symptom	S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M	ID	3/3

		//static "431"
		OutputString += ParseANSIField("431",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

		//static "D8"
		OutputString += ParseANSIField("D8",2,3);

		//DTP03	1251		Date Time Period						M	AN	1/35

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

//190		135		DTP		Date - Acute Manifestation				S		5

		//not used

//192		135		DTP		Date - Similar Illness/Symptom Onset	S		10

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M	ID	3/3

		//static "438"
		OutputString += ParseANSIField("438",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

		//static "D8"
		OutputString += ParseANSIField("D8",2,3);

		//DTP03	1251		Date Time Period						M	AN	1/35

		// (j.jones 2016-04-11 17:22) - NX-100149 - this is now the calculated Box 15 date,
		// based on the value of BillsT.ConditionDateType
		var = fields->Item["Box15Date"]->Value;
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

//194		135		DTP		Date - Accident							S		10

		//only used if CLM-11 has AA, AB, AP, or OA

		if(bRelatedToAutoAcc || bRelatedToOther) {

			OutputString = "DTP";

			//DTP01	374			Date/Time Qualifier						M	ID	3/3

			//static "439"
			OutputString += ParseANSIField("439",3,3);

			//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

			//static "D8"
			OutputString += ParseANSIField("D8",2,3);

			//DTP03	1251		Date Time Period						M	AN	1/35

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
		}

//196		135		DTP		Date - Last Menstrual Period			S		1
//197		135		DTP		Date - Last X-Ray						S		1
//199		135		DTP		Date - Estimated Date Of Birth			S		1
//200		135		DTP		Date - Hearing&Vision Prescription Date S		1

		//not used

//201		135		DTP		Date - Disability Begin					S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M	ID	3/3

		//static "360"
		OutputString += ParseANSIField("360",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

		//static "D8"
		OutputString += ParseANSIField("D8",2,3);

		//DTP03	1251		Date Time Period						M	AN	1/35

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

//203		135		DTP		Date - Disability End					S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M	ID	3/3

		//static "361"
		OutputString += ParseANSIField("361",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

		//static "D8"
		OutputString += ParseANSIField("D8",2,3);

		//DTP03	1251		Date Time Period						M	AN	1/35

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

//205		135		DTP		Date - Last Worked						S		1

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M	ID	3/3

		//static "297"
		OutputString += ParseANSIField("297",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

		//static "D8"
		OutputString += ParseANSIField("D8",2,3);

		//DTP03	1251		Date Time Period						M	AN	1/35

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

//206		135		DTP		Date - Authorized Return To Work		S		1

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M	ID	3/3

		//static "296"
		OutputString += ParseANSIField("296",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

		//static "D8"
		OutputString += ParseANSIField("D8",2,3);

		//DTP03	1251		Date Time Period						M	AN	1/35

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

//208		135		DTP		Date - Admission						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M	ID	3/3

		//static "435"
		OutputString += ParseANSIField("435",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

		//static "D8"
		OutputString += ParseANSIField("D8",2,3);

		//DTP03	1251		Date Time Period						M	AN	1/35

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

//210		135		DTP		Date - Discharge						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M	ID	3/3

		//static "096"
		OutputString += ParseANSIField("096",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

		//static "D8"
		OutputString += ParseANSIField("D8",2,3);

		//DTP03	1251		Date Time Period						M	AN	1/35

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

//212		135		DTP		Date - Assumed and Relinq. Care Dates	S		2
//214		155		PWK		Claim Supplemental Information			S		10

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

			//PWK01 755		Report Type Code						M	ID	2/2
			/*
				AS Admission Summary
				B2 Prescription
				B3 Physician Order
				B4 Referral Form
				CT Certification
				DA Dental Models
				DG Diagnostic Report
				DS Discharge Summary
				EB Explanation of Benefits (Coordination of Benefits or
					Medicare Secondary Payor)
				MT Models
				NN Nursing Notes
				OB Operative Note
				OZ Support Data for Claim
				PN Physical Therapy Notes
				PO Prosthetics or Orthotic Certification
				PZ Physical Therapy Certification
				RB Radiology Films
				RR Radiology Reports
				RT Report of Tests and Analysis Report
			*/

			OutputString += ParseANSIField(AdoFldString(fields, "PaperworkType", ""), 2, 2);

			//PWK02	756		Report Transmission Code				O	ID	1/2
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
			*/

			OutputString += ParseANSIField(AdoFldString(fields, "PaperworkTx", ""), 1, 2);

			//PWK03 757		Report Copies Needed					O	N0	1/2
			// not used
			OutputString += ParseANSIField("", 1, 2);

			//PWK04 98		Entity Identifier Code					O	ID	2/3
			// not used
			OutputString += ParseANSIField("", 2, 3);

			//PWK05 66		Identification Code Qualifier			X	ID	1/2
			// required when PWK02 != 'AA'. Can be used anytime when Provider wants
			// to send a document control number for an attachment remaining at the
			// Provider's office.
			// AC	Attachment Control Number.

			OutputString += ParseANSIField("AC", 1, 2);

			//PWK06 67		Identification Code						X	AN	2/80
			// code identifying a party or other code. We will send the Bill's ID.

			CString strIDCode;
			strIDCode.Format("%lu", m_pEBillingInfo->BillID);
			OutputString += ParseANSIField(strIDCode, 2, 80);

			//PWK07 352		Description								O	AN	1/80
			// not used (although theoretically used on Institutional)
			// A free-form description to clarify the related data elements
			// advisory: under most circumstances, this element is not sent.

			// we will not be sending this

			//PWK08 C002	ACTIONS INDICATED						O
			// not used

			//PWK09	1525	Request Category Code					O	ID	1/2
			// not used

			EndANSISegment(OutputString);
		}
		
//217		160		CN1		Contract Information					S		1

		// (j.jones 2006-11-24 15:08) - PLID 23415 - use the contract information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending contract information
		if(!m_pEBillingInfo->IsPrimary && m_HCFAInfo.ANSI_EnablePaymentInfo == 1
			&& m_HCFAInfo.ANSI_2300Contract == 1) {

			OutputString = "CN1";

			//Ref.	Data		Name									Attributes
			//Des.	Element

			//CN101 1166	Contract Type Code						M	ID	2/2

			//02 - Per Diem
			//03 - Variable Per Diem
			//04 - Flat
			//05 - Capitated
			//06 - Percent
			//09 - Other

			//apparently, this only needs to be 09 and the claim total
			str = "09";
			OutputString += ParseANSIField(str,2,2);

			//CN102 782		Monetary Amount							O	R	1/18

			//use the cyChargeTotal from earlier, to show the claim total
			str = FormatCurrencyForInterface(cyChargeTotal,FALSE,FALSE);

			//see if we need to trim the right zeros
			// (j.jones 2008-05-06 11:42) - PLID 29937 - cached m_bTruncateCurrency
			if(m_bTruncateCurrency) {
				str.TrimRight("0");	//first the zeros
				str.TrimRight(".");	//then the decimal, if necessary
			}

			str.Replace(GetCurrencySymbol(),"");
			str.Replace(GetThousandsSeparator(),"");

			OutputString += ParseANSIField(str,1,18);

			//CN103 332		Percent									O	R	1/6

			//we don't use this

			OutputString += ParseANSIField("",1,6);

			//CN104 127		Reference Identification				O	AN	1/30

			//we don't use this

			OutputString += ParseANSIField("",1,30);

			//CN105 338		Terms Discount Percent					O	R	1/6

			//we don't use this

			OutputString += ParseANSIField("",1,6);

			//CN106 799		Version Identifier						O	AN	1/30

			//we don't use this

			OutputString += ParseANSIField("",1,30);

			EndANSISegment(OutputString);
		}

//219		175		AMT		Credit/Debit Card Maximum Amount		S		1

		//not used

//220		175		AMT		Patient Amount Paid						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "AMT";

		//AMT01	522			Amount Qualifier Code					M	ID	1/3

		//static "F5"
		OutputString += ParseANSIField("F5",1,3);

		//AMT02	782			Monetary Amount							M	R	1/18

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
				"%s",m_pEBillingInfo->BillID, strWhere);

			if(!rsApplies->eof) {

				COleCurrency cyAmt = AdoFldCurrency(rsApplies, "Total",COleCurrency(0,0));

				if(cyAmt != COleCurrency(0,0)) {
					bIsNonZero = TRUE;
				}

				str = FormatCurrencyForInterface(cyAmt, FALSE, FALSE);

				//see if we need to trim the right zeros
				// (j.jones 2008-05-06 11:42) - PLID 29937 - cached m_bTruncateCurrency
				if(m_bTruncateCurrency) {
					str.TrimRight("0");	//first the zeros
					str.TrimRight(".");	//then the decimal, if necessary
				}
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

//221		175		AMT		Total Purchased Service Amount			S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "AMT";

		//AMT01	522			Amount Qualifier Code					M	ID	1/3

		//static "NE"
		OutputString += ParseANSIField("NE",1,3);

		//AMT02	782			Monetary Amount							M	R	1/18

		//BillsT.OutsideLabCharges
		var = fields->Item["OutsideLabCharges"]->Value;
		if(var.vt == VT_CY) {
			str = FormatCurrencyForInterface(var.cyVal,FALSE,FALSE);

			//see if we need to trim the right zeros
			// (j.jones 2008-05-06 11:42) - PLID 29937 - cached m_bTruncateCurrency
			if(m_bTruncateCurrency) {
				str.TrimRight("0");	//first the zeros
				str.TrimRight(".");	//then the decimal, if necessary
			}
		}
		else
			str = "";
		str.Replace(GetCurrencySymbol(),"");
		str.Replace(GetThousandsSeparator(),"");
		OutputString += ParseANSIField(str,1,18);

		//AMT03	NOT USED

		//as this is an optional segment, don't output it if there is no data
		if(str != "" && str != "0" && str != "0.00")
			EndANSISegment(OutputString);

//222		180		REF		Service Auth. Exception Code			S		1
//224		180		REF		Mandatory Medicare Crossover Indicator	S		1
//226		180		REF		Mammography Certification Number		S		1

		//not used

//227		180		REF		Prior Authorization or Referral Number	S		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01	128			Reference Identification Qualifier		M	ID	2/3
		//G1 - Prior Authorization Number
		//9F - Referral Number
		
		// (j.jones 2009-11-24 15:47) - PLID 36411 - use the qualifier from the HCFA Setup
		// (j.jones 2010-03-02 16:18) - PLID 37584 - this logic has changed, now we can
		// choose Prior Auth. or Referral Number from the bill, and the HCFA Setup acts
		// as an override, which can now be blank.
		CString strPriorAuthQual = m_HCFAInfo.PriorAuthQualifier;
		strPriorAuthQual.TrimLeft();
		strPriorAuthQual.TrimRight();
		if(strPriorAuthQual.IsEmpty()) {
			//if empty, then no override is in place,
			//so pick the qualifier from BillsT.PriorAuthType
			PriorAuthType patPriorAuthType = (PriorAuthType)VarLong(fields->Item["PriorAuthType"]->Value, (long)patPriorAuthNumber);
			if(patPriorAuthType == patReferralNumber) {
				strPriorAuthQual = "9F";
			}
			else {
				strPriorAuthQual = "G1";
			}
		}
		OutputString += ParseANSIField(strPriorAuthQual,2,3);

		//REF02	127			Reference Identification				X	AN	1/30

		//BillsT.PriorAuthNum
		var = fields->Item["PriorAuthNum"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		//use the override from the HCFA setup
		if(str == "")
			str = m_HCFAInfo.DefaultPriorAuthNum;

		OutputString += ParseANSIField(str,1,30);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		//as this is an optional segment, don't output it if there is no data
		if(str != "")
			EndANSISegment(OutputString);

		str = "";

		if(m_HCFAInfo.ANSI_SendRefPhyIn2300 == 1 && 
			!IsRecordsetEmpty("SELECT ID FROM BillsT INNER JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
					"WHERE ID = %li",m_pEBillingInfo->BillID)) {

			//they want to send this segment again, with the referring physician, so do it

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

			//198		180		REF		Prior Authorization or Referral Number	S		2

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "REF";

			//REF01 128			Reference Identification Qualifier		M	ID	2/3
			//G1 - Prior Authorization Number
			//9F - Referral Number
			OutputString += ParseANSIField("9F",2,3);
			
			//REF02 127			Reference Identification				X	AN	1/30
			
			//Box17aANSI returns the qualifier and the ID, but here we only want the ID

			var = rs->Fields->Item["ID"]->Value;
			CString strIdentifier, strID;
			// (j.jones 2010-10-20 15:42) - PLID 40936 - moved Box17aANSI to GlobalFinancialUtils and renamed it
			EBilling_Box17aANSI(m_HCFAInfo.Box17a, m_HCFAInfo.Box17aQual, strIdentifier,strID,var.lVal);

			OutputString += ParseANSIField(strID,1,30);

			//REF03 NOT USED
			//REF04 NOT USED

			//don't output if blank
			if(strID != "")
				EndANSISegment(OutputString);

			rs->Close();
		}

//229		180		REF		Original Reference Number (ICN/DCN)		S		1
		// (a.walling 2007-07-24 09:23) - PLID 26780 - Include Medicaid Resubmission code or Original Reference Number
		{
			CString strMedicaidResubmission = AdoFldString(fields, "MedicaidResubmission");
			CString strOriginalRefNo = AdoFldString(fields, "OriginalRefNo");

			CString strNum;

			if ( (strMedicaidResubmission.GetLength() > 0) && (strOriginalRefNo.GetLength() > 0) ) {
				strNum = strMedicaidResubmission;
			} else if (strMedicaidResubmission.GetLength() > 0) {
				strNum = strMedicaidResubmission;
			} else if (strOriginalRefNo.GetLength() > 0) {
				strNum = strOriginalRefNo;
			}

			if (strNum.GetLength()) {
				// we have a non-empty code here, so export it!

				//Ref.	Data		Name									Attributes
				//Des.	Element

				OutputString = "REF";

				//REF01 128			Reference Identification Qualifier		M	ID	2/3
				//F8 - Original Reference Number (ICN/DCN)
				OutputString += ParseANSIField("F8", 2, 3);
				
				//REF02 127			Reference Identification				X	AN	1/30
				
				OutputString += ParseANSIField(strNum, 1, 30);

				//REF03 NOT USED
				//REF04 NOT USED
				
				// already checked for length of strMedicaidResubmission above
				EndANSISegment(OutputString);
			}
		}


//231		180		REF		CLIA Number								S		3

		// (j.jones 2006-10-31 08:49) - PLID 23285 - if the CLIA number should be used
		// on this claim, here is where it needs to be exported

		// (j.jones 2011-03-07 14:09) - PLID 42660 - the CLIA number is now calculated
		// by our caller and is passed in
		if(eCLIANumber.bUseCLIANumber && !eCLIANumber.strCLIANumber.IsEmpty()) {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "REF";

			//REF01 128			Reference Identification Qualifier		M	ID	2/3
			//X4 - Clinical Laboratory Improvement Amendment Number
			OutputString += ParseANSIField("X4",2,3);
			
			//REF02 127			Reference Identification				X	AN	1/30
			
			//use the CLIA number returned earlier
			OutputString += ParseANSIField(eCLIANumber.strCLIANumber,1,30);

			//REF03 NOT USED
			//REF04 NOT USED

			//don't output if blank
			if(eCLIANumber.strCLIANumber != "")
				EndANSISegment(OutputString);
		}

//233		180		REF		Repriced Claim Number					S		1
//235		180		REF		Adjusted Repriced Claim Number			S		1
//236		180		REF		Investigational Device Exemption Number	S		1
//238		180		REF		Claim ID Number for Clearinghouses		S		1
//240		180		REF		Ambulatory Patient Group (APG)			S		4
//241		180		REF		Medical Record Number					S		1
//242		180		REF		Demonstration Project Identifier		S		1
//244		185		K3		File Information						S		10

//246		190		NTE		Claim Note								S		1

		// (j.jones 2008-10-03 10:13) - PLID 31578 - HCFABox19 may be sendable in the 2300 or 2400 NTE
		// (j.jones 2008-10-06 10:43) - PLID 31580 - same with the SendCorrespondence feature

		BOOL bSendCorrespondence = ReturnsRecords("SELECT SendCorrespondence FROM BillsT WHERE SendCorrespondence = 1 AND ID = %li",m_pEBillingInfo->BillID);

		if((m_HCFAInfo.ANSI_SendBox19 == 1 && m_HCFAInfo.ANSI_SendBox19Segment == 0)
			|| (m_HCFAInfo.ANSI_SendCorrespSegment == 0 && bSendCorrespondence)) {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "NTE";

			//NTE01 363			Note Reference Code						O	ID	3/3

			//ADD - Additional Information
			//CER - Certification Narrative
			//DCP - Goals, Rehabilitation Potential, or Discharge Plans
			//DGN - Diagnosis Description
			//PMT - Payment
			//TPO - Third Party Organization Notes

			OutputString += ParseANSIField("ADD",3,3);

			//NTE02 352			Description								M	AN	1/80

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

//248		195		CR1		Ambulance Transport Information			S		1
//251		200		CR2		Spinal Manipulation Service Information	S		1
//257		220		CRC		Ambulance Certification					S		3
//260		220		CRC		Patient Condition Information: Vision	S		3
//263		220		CRC		Homebound Indicator						S		1

		//not used

//265		231		HI		Health Care Diagnosis Code				S		1
		

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HI";

		BOOL bOutput = TRUE;

		//HI01	C022		Health Care Code Info.					M	?	?

		//HI01-1	1270	Code List Qualifier Code				M	ID	1/3

		//static "BK" (principal diagnosis)
		OutputString += ParseANSIField("BK",1,3);

		//HI01-2	1271	Industry Code							M	AN	1/30

		//BillsT.DiagCode1

		// (a.walling 2014-03-19 10:11) - PLID 61418 - EBilling - ANSI4010 - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(0).number;

		str.Replace(".","");
		str.TrimRight();

		if(str == "")
			bOutput = FALSE;

		OutputString += ":";
		OutputString += str;

		//HI01-3	NOT USED
		//HI01-4	NOT USED
		//HI01-5	NOT USED
		//HI01-6	NOT USED
		//HI01-7	NOT USED

		//HI02	C022		Health Care Code Info.					O

		//with no easy way to backtrack, we won't output this element at all
		//if the diag code is blank

		// (a.walling 2014-03-19 10:11) - PLID 61418 - EBilling - ANSI4010 - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(1).number;

		str.Replace(".","");
		str.TrimRight();

		//HI02-1	1270	Code List Qualifier Code				M	ID	1/3

		//static "BF" (diagnosis)
		if(str != "")
			OutputString += ParseANSIField("BF",1,3);

		//HI02-2	1271	Industry Code							M	AN	1/30

		//BillsT.DiagCode2

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//HI02-3	NOT USED
		//HI02-4	NOT USED
		//HI02-5	NOT USED
		//HI02-6	NOT USED
		//HI02-7	NOT USED

		//HI03	C022		Health Care Code Info.					O

		//with no easy way to backtrack, we won't output this element at all
		//if the diag code is blank

		// (a.walling 2014-03-19 10:11) - PLID 61418 - EBilling - ANSI4010 - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(2).number;

		str.Replace(".","");
		str.TrimRight();

		//HI03-2	1270	Code List Qualifier Code				M	ID	1/3

		//static "BF" (diagnosis)
		if(str != "")
			OutputString += ParseANSIField("BF",1,3);

		//HI03-2	1271	Industry Code							M	AN	1/30

		//BillsT.DiagCode3

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//HI03-3	NOT USED
		//HI03-4	NOT USED
		//HI03-5	NOT USED
		//HI03-6	NOT USED
		//HI03-7	NOT USED

		//HI04	C022		Health Care Code Info.					O

		//with no easy way to backtrack, we won't output this element at all
		//if the diag code is blank

		// (a.walling 2014-03-19 10:11) - PLID 61418 - EBilling - ANSI4010 - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(3).number;

		str.Replace(".","");
		str.TrimRight();

		//HI04-2	1270	Code List Qualifier Code				M	ID	1/3

		//static "BF" (diagnosis)
		if(str != "")
			OutputString += ParseANSIField("BF",1,3);

		//HI04-2	1271	Industry Code							M	AN	1/30

		//BillsT.DiagCode4

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//HI04-3	NOT USED
		//HI04-4	NOT USED
		//HI04-5	NOT USED
		//HI04-6	NOT USED
		//HI04-7	NOT USED

		//HI05	C022		Health Care Code Info.					O
		//HI06	C022		Health Care Code Info.					O
		//HI07	C022		Health Care Code Info.					O
		//HI08	C022		Health Care Code Info.					O

		// (j.jones 2009-03-25 17:28) - PLID 33669 - the rest of these we'll just do in a loop,
		// exporting the top 4 "extra" codes in the bill, for a total of 8 codes

		// (j.jones 2011-04-20 10:45) - PLID 41490 - if bSkipDiagnosisCodes is TRUE,
		// we need to pull our diagnosis codes from aryNewDiagnosisCodesToExport
		// (a.walling 2014-03-19 10:11) - PLID 61418 - EBilling - ANSI4010 - Get diag info from bill
		for (int i = 4; i < (int)m_pEBillingInfo->billDiags.size() && i < 8; ++i) {
			str = m_pEBillingInfo->billDiags[i].number;
			str.Replace(".","");
			str.TrimRight();

			//HI0#-2	1270	Code List Qualifier Code				M	ID	1/3

			//static "BF" (diagnosis)
			if(str != "")
				OutputString += ParseANSIField("BF",1,3);

			//HI0#-2	1271	Industry Code							M	AN	1/30

			if(str != "") {
				OutputString += ":";
				OutputString += str;
			}
		}

		//HI09	NOT USED
		//HI10	NOT USED
		//HI11	NOT USED
		//HI12	NOT USED

		if(bOutput)
			EndANSISegment(OutputString);

//271		241		HCP		Claim Pricing/Repricing Information		S		1

		//not used

///////////////////////////////////////////////////////////////////////////////

		rs->Close();

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2300_Prof");

	return Error_Other;
}

int CEbilling::ANSI_4010_2300_Inst() {

	//Claim Information

	//if the patient is the subscriber, 2300 loop follows the 2010BD loop
	//if the patient is not the subscriber, 2300 loop follows 2010CA

#ifdef _DEBUG

	CString str;

	str = "\r\n2300\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	//increment the count for the current claim
	m_ANSIClaimCount++;

	try {

		CString OutputString,str;
		_variant_t var;
		
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		// (j.gruber 2014-03-17 11:48) - PLID 61394 - support new billing structure
		// (a.walling 2014-03-19 10:11) - PLID 61418 - EBilling - ANSI4010 - Get diag info from bill
		_RecordsetPtr rs = CreateParamRecordset("SELECT BillsT.*, PlaceofServiceCodesT.PlaceCodes "
			"FROM BillsT LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
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
				str.Format("Could not open bill information for '%s', Bill ID %li. (ANSI_2300_Inst)", strPatientName, m_pEBillingInfo->BillID);
			}
			else
				//serious problems if you get here
				str = "Could not open bill information. (ANSI_2300_Inst)";
			
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

		//CLM01	1028		Claim Submitter's Identifier			M	AN	1/38

		//PatientsT.UserDefinedID
		
		str.Format("%li",m_pEBillingInfo->UserDefinedID);

		// (j.jones 2009-10-01 14:21) - PLID 35711 - added the ability to insert text in front of the patient ID
		if(m_bPrependPatientID) {
			str = m_strPrependPatientIDCode + str;
		}

		OutputString += ParseANSIField(str,1,38);

		//CLM02	782			Monetary Amount							O	R	1/18

		//this is the total charge amount for the claim

		CString strDocProv = "";
		//if using the bill provider, filter only on that doctor's charges, otherwise get all charges
		//in ANSI, we're using the bill provider if we are using anything but the G1 provider

		// (j.jones 2006-12-01 15:50) - PLID 22110 - supported the ClaimProviderID
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		// (d.thompson 2011-09-12) - PLID 45393 - Properly fixed the calculation to limit charges when the charge Claim provider is in use
		if(m_actClaimType == actInst && m_UB92Info.Box82Setup == 1) {
			strDocProv.Format("AND (ChargesT.ClaimProviderID = %li OR (ClaimProviderID IS NULL AND DoctorsProviders IN (SELECT PersonID FROM ProvidersT WHERE ClaimProviderID = %li))) ", m_pEBillingInfo->ProviderID, m_pEBillingInfo->ProviderID);
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

			//see if we need to trim the right zeros
			// (j.jones 2008-05-06 11:42) - PLID 29937 - cached m_bTruncateCurrency
			if(m_bTruncateCurrency) {
				str.TrimRight("0");	//first the zeros
				str.TrimRight(".");	//then the decimal, if necessary
			}

			str.Replace(GetCurrencySymbol(),"");
			str.Replace(GetThousandsSeparator(),"");

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

		//CLM05	C023		Health Care Service Location Info.		O

		// (j.jones 2004-10-20 16:49) - This is absurd. Essentially CLM05 is
		//the three digit number from the UB92 Box 4. However, it's sent as
		//the first two digits, an A, and then the third digit.
		//So if UB92 Box 4 is 741, we send 74:A:1

		//We allow for a default for Box 4 but cannot reliably track when it changed.
		//We may need to add support for this (on the bill?) in the future.

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

		CString strBox4Left = "", strBox4Right = "";
		if(strBox4.GetLength() >= 2) {
			strBox4Left = strBox4.Left(2);
			strBox4Right = "1";
		}
		if(strBox4.GetLength() >= 3) {
			strBox4Right = strBox4.Right(1);
		}

		//CLM05-1	1331	Facility Code Value						M	AN	1/2

		//left two digits from UB92 Box 4		
		OutputString += ParseANSIField(strBox4Left,1,2);

		//CLM05-2	1332	Facility Code Qualifier					O	ID	1/2

		//only output if there is a place of service code
		if(strBox4Left != "")
			//static A, separated by a :
			OutputString += ":A";

		//CLM05-3	1325	Claim Frequency Type Code				O	ID	1/1

		//static "1"

		//only output if there is a place of service code
		if(strBox4Left != "") {
			//it should always be 1, so make it 1 if that's the only holdup
			if(strBox4Right == "")
				strBox4Right = "1";
			OutputString += ":";
			OutputString += strBox4Right;
		}

		//CLM06	1073		Yes/No Cond Resp Code					O	ID	1/1

		//static "Y" (Signature On File)
		OutputString += ParseANSIField("Y",1,1);

		//CLM07	1359		Provider Accept Code					O	ID	1/1

		//InsuranceAcceptedT.Accepted
		BOOL bAccepted = TRUE;
		str = "C";
		if(m_pEBillingInfo->Box82Setup != 3) {
			// (j.jones 2010-07-23 15:02) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsCo now
			bAccepted = GetAcceptAssignment_ByInsCo(m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);
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

		//CLM08	1073		Yes/No Cond Resp Code					O	ID	1/1

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

		//CLM09	1363		Release of Info Code					O	ID	1/1

		//static "Y"
		OutputString += ParseANSIField("Y",1,1);

		//CLM10	NOT USED
		OutputString += "*";

		//CLM11	C024		Related Causes Information				O

		//CLM11-1	1362	Related-Causes Code						M	ID	2/3

		//BillsT.RelatedToAutoAcc, BillsT.RelatedToOther, BillsT.RelatedToEmp
		_variant_t varAcc, varOther, varEmp;
		varAcc = fields->Item["RelatedToAutoAcc"]->Value;
		varOther = fields->Item["RelatedToOther"]->Value;
		varEmp = fields->Item["RelatedToEmp"]->Value;
		if(varAcc.vt == VT_BOOL && varAcc.boolVal)
			str = "AA";
		else if (varOther.vt == VT_BOOL && varOther.boolVal)
			str = "OA";
		else if (varEmp.vt == VT_BOOL && varEmp.boolVal)
			str = "EM";
		else
			str = "";
		OutputString += ParseANSIField(str,2,3);

		//now check if we need the state before outputting the rest of this combo

		if(str == "AA") {
			var = fields->Item["State"]->Value;
			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";
		str.TrimRight(" ");

		//at this point str is either blank or the state
			
		//CLM11-2	1362	Related-Causes Code						O	ID	2/3
			//not used
		if(str != "")
			OutputString += ":";

		//CLM11-3	1362	Related-Causes Code						O	ID	2/3
			//not used
		if(str != "")
			OutputString += ":";

		//CLM11-4	156		State or Province Code					O	ID	2/2

		//BillsT.State
		//only used if BillsT.RelatedToAutoAcc is TRUE

		if(str != "") {
			OutputString += ":";
			//str has the state in it
			OutputString += str;
		}

		//CLM11-5	26		Country Code							O	ID	2/3
		//skip this element - we don't use it, and it's the last part of the composite,
		//so do nothing

		//CLM12	1366		Special Program Code					O	ID	2/3
			//not used
		OutputString += "*";

		//CLM13	NOT USED
		OutputString += "*";
		//CLM14	NOT USED
		OutputString += "*";
		//CLM15	NOT USED
		OutputString += "*";

		//CLM16	1360		Provider Agreement Code					O	ID	1/1
			//not used
		OutputString += "*";

		//CLM17	NOT USED
		OutputString += "*";
		
		//CLM18	1073		Yes/No Condition or Response Code		O	ID	1/1
		
		//this is the EOB indicator, as in 'do they want one'

		//static "Y"
		OutputString += ParseANSIField("Y",1,1);

		//CLM19	NOT USED
		OutputString += "*";

		//CLM20	1514		Delay Reason Code						O	ID	1/2
			//not used
		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//165		135		DTP		Discharge Hour							S		1
		//not used now, but I have the suspicion we might need this at some point

///////////////////////////////////////////////////////////////////////////////

//167		135		DTP		Statement Dates							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M	ID	3/3

		//static "434"
		OutputString += ParseANSIField("434",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

		CString strDateQual = "D8";
		CString strDate;

		// (j.jones 2010-03-12 16:40) - PLID 37740 - send the service date range, if they span multiple days
		// (j.jones 2010-04-19 12:25) - PLID 38265 - if ANSI_StatementDateRange = 1, we will always send
		// a date range even if both dates are the same
		if(dtMinServiceDate == dtMaxServiceDate && m_UB92Info.ANSI_StatementDateRange != 1) {
			strDateQual = "D8";
			strDate = dtMinServiceDate.Format("%Y%m%d");
		}
		else {
			strDateQual = "RD8";
			strDate.Format("%s-%s",dtMinServiceDate.Format("%Y%m%d"), dtMaxServiceDate.Format("%Y%m%d"));
		}

		//static "D8" for one date, "RD8" for two dates
		OutputString += ParseANSIField(strDateQual,2,3);

		//DTP03	1251		Date Time Period						M	AN	1/35

		//If one date: CCYYMMDD, if two dates: CCYYMMDD-CCYYMMDD
		OutputString += ParseANSIField(strDate,1,35);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//169		135		DTP		Admission Date/Hour						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2008-06-09 15:47) - PLID 30229 - this should be the HospFrom date

		_variant_t varHospFrom = rs->Fields->Item["HospFrom"]->Value;
		if(varHospFrom.vt == VT_DATE) {

			OutputString = "DTP";

			//DTP01	374			Date/Time Qualifier						M	ID	3/3

			//static "435"
			OutputString += ParseANSIField("435",3,3);

			//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

			//static "DT" - (CCYYMMDDHHMM)
			OutputString += ParseANSIField("DT",2,3);

			//DTP03	1251		Date Time Period						M	AN	1/35

			//BillsT.HospFrom + 9am (let's fake it for now)
			if(varHospFrom.vt == VT_DATE) {
				 COleDateTime dt = varHospFrom.date;
				 str = dt.Format("%Y%m%d");
				 str += "0900";
			}
			else
				str = "";
			OutputString += ParseANSIField(str,1,35);

			if(!str.IsEmpty()) {
				EndANSISegment(OutputString);
			}
		}

///////////////////////////////////////////////////////////////////////////////

//171		140		CL1		Institutional Claim Code				S		1
		
		//not used

///////////////////////////////////////////////////////////////////////////////

//173		155		PWK		Claim Supplemental Information			S		10

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

			//PWK01 755		Report Type Code						M	ID	2/2
			/*
				AS Admission Summary
				B2 Prescription
				B3 Physician Order
				B4 Referral Form
				CT Certification
				DA Dental Models
				DG Diagnostic Report
				DS Discharge Summary
				EB Explanation of Benefits (Coordination of Benefits or
					Medicare Secondary Payor)
				MT Models
				NN Nursing Notes
				OB Operative Note
				OZ Support Data for Claim
				PN Physical Therapy Notes
				PO Prosthetics or Orthotic Certification
				PZ Physical Therapy Certification
				RB Radiology Films
				RR Radiology Reports
				RT Report of Tests and Analysis Report
			*/

			OutputString += ParseANSIField(AdoFldString(fields, "PaperworkType", ""), 2, 2);

			//PWK02	756		Report Transmission Code				O	ID	1/2
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
			*/

			OutputString += ParseANSIField(AdoFldString(fields, "PaperworkTx", ""), 1, 2);

			//PWK03 757		Report Copies Needed					O	N0	1/2
			// not used
			OutputString += ParseANSIField("", 1, 2);

			//PWK04 98		Entity Identifier Code					O	ID	2/3
			// not used
			OutputString += ParseANSIField("", 2, 3);

			//PWK05 66		Identification Code Qualifier			X	ID	1/2
			// required when PWK02 != 'AA'. Can be used anytime when Provider wants
			// to send a document control number for an attachment remaining at the
			// Provider's office.
			// AC	Attachment Control Number.

			OutputString += ParseANSIField("AC", 1, 2);

			//PWK06 67		Identification Code						X	AN	2/80
			// code identifying a party or other code. We will send the Bill's ID.

			CString strIDCode;
			strIDCode.Format("%lu", m_pEBillingInfo->BillID);
			OutputString += ParseANSIField(strIDCode, 2, 80);

			//PWK07 352		Description								O	AN	1/80
			// A free-form description to clarify the related data elements
			// advisory: under most circumstances, this element is not sent.

			// we will not be sending this

			//PWK08 C002	ACTIONS INDICATED						O
			// not used

			//PWK09	1525	Request Category Code					O	ID	1/2
			// not used

			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//176		160		CN1		Contract Information					S		1

		// (j.jones 2006-11-27 17:01) - PLID 23652 - use the contract information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending contract information
		if(!m_pEBillingInfo->IsPrimary && m_UB92Info.ANSI_EnablePaymentInfo == 1
			&& m_UB92Info.ANSI_2300Contract == 1) {

			OutputString = "CN1";

			//Ref.	Data		Name									Attributes
			//Des.	Element

			//CN101 1166	Contract Type Code						M	ID	2/2

			//02 - Per Diem
			//03 - Variable Per Diem
			//04 - Flat
			//05 - Capitated
			//06 - Percent
			//09 - Other

			//apparently, this only needs to be 09 and the claim total
			str = "09";
			OutputString += ParseANSIField(str,2,2);

			//CN102 782		Monetary Amount							O	R	1/18

			//use the cyChargeTotal from earlier, to show the claim total
			str = FormatCurrencyForInterface(cyChargeTotal,FALSE,FALSE);

			//see if we need to trim the right zeros
			// (j.jones 2008-05-06 11:42) - PLID 29937 - cached m_bTruncateCurrency
			if(m_bTruncateCurrency) {
				str.TrimRight("0");	//first the zeros
				str.TrimRight(".");	//then the decimal, if necessary
			}

			str.Replace(GetCurrencySymbol(),"");
			str.Replace(GetThousandsSeparator(),"");

			OutputString += ParseANSIField(str,1,18);

			//CN103 332		Percent									O	R	1/6

			//we don't use this

			OutputString += ParseANSIField("",1,6);

			//CN104 127		Reference Identification				O	AN	1/30

			//we don't use this

			OutputString += ParseANSIField("",1,30);

			//CN105 338		Terms Discount Percent					O	R	1/6

			//we don't use this

			OutputString += ParseANSIField("",1,6);

			//CN106 799		Version Identifier						O	AN	1/30

			//we don't use this

			OutputString += ParseANSIField("",1,30);

			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//178		175		AMT		Payer Estimated Amount Due				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "AMT";

		//AMT01	522			Amount Qualifier Code					M	ID	1/3

		//static "C5"
		OutputString += ParseANSIField("C5",1,3);

		//AMT02	782			Monetary Amount							M	R	1/18

		//this is the total charge amount for the claim

		strDocProv = "";
		//if using the bill provider, filter only on that doctor's charges, otherwise get all charges
		//in ANSI, we're using the bill provider if we are using anything but the G1 provider

		// (j.jones 2006-12-01 15:50) - PLID 22110 - supported the ClaimProviderID
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		// (d.thompson 2011-09-12) - PLID 45393 - Properly filter the charge list when using the per-charge claim provider id
		if(m_actClaimType == actInst && m_UB92Info.Box82Setup == 1) {
			strDocProv.Format("AND (ClaimProviderID = %li OR (ClaimProviderID IS NULL AND DoctorsProviders IN (SELECT PersonID FROM ProvidersT WHERE ClaimProviderID = %li)))", m_pEBillingInfo->ProviderID, m_pEBillingInfo->ProviderID);
		}

		//this cannot be parameterized
		// (j.jones 2010-03-12 16:31) - PLID 37740 - need the earliest ServiceDateFrom and latest ServiceDateTo
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		_RecordsetPtr rsChargeTot = CreateRecordset("SELECT Sum(dbo.GetChargeTotal(ChargesT.ID)) AS ChargeTotal FROM "
					"ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"WHERE ChargesT.BillID = %li AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 %s "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					"GROUP BY ChargesT.BillID",m_pEBillingInfo->BillID, strDocProv);

		if(!rsChargeTot->eof) {
			var = rsChargeTot->Fields->Item["ChargeTotal"]->Value;
			if(var.vt == VT_CY) {
				str = FormatCurrencyForInterface(var.cyVal,FALSE,FALSE);

				//see if we need to trim the right zeros
				// (j.jones 2008-05-06 11:42) - PLID 29937 - cached m_bTruncateCurrency
				if(m_bTruncateCurrency) {
					str.TrimRight("0");	//first the zeros
					str.TrimRight(".");	//then the decimal, if necessary
				}
			}
			else
				str = "0";
			str.Replace(GetCurrencySymbol(),"");
			str.Replace(GetThousandsSeparator(),"");
		}
		else
			str = "0";
		OutputString += ParseANSIField(str,1,18);

		rsChargeTot->Close();

		//AMT03	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//180		175		AMT		Patient Estimated Amount Due			S		1

///////////////////////////////////////////////////////////////////////////////

//182		175		AMT		Patient Paid Amount						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2008-09-09 16:48) - PLID 26482 - if we ever choose to
		// output this record, check the m_bHidePatAmtPaid value first

		/*
		if(!m_bHidePatAmtPaid) {

			OutputString = "AMT";

			//AMT01	522			Amount Qualifier Code					M	ID	1/3

			//static "F5"
			OutputString += ParseANSIField("F5",1,3);

			//AMT02	782			Monetary Amount							M	R	1/18

			//This will be the total applied patient payments to this bill.

			OutputString += ParseANSIField("0.00",1,18);

			//TODO: Determine if people WANT this value to be non-zero (they usually do not).
			//until then, this will be zero, and we will not output the record.

			//AMT03	NOT USED

			OutputString += "*";

			//EndANSISegment(OutputString);
		}
		*/

///////////////////////////////////////////////////////////////////////////////

//184		175		AMT		Credit/Debit Card Maximum Amount		S		1
//185		180		REF		Adjusted Repriced Claim Number			S		1
//186		180		REF		Repriced Claim Number					S		1
//187		180		REF		Claim Ident. Nmbr. For Clearinghouses	S		1
//189		180		REF		Document Identification Code			S		1
	// not used

//191		180		REF		Original Reference Number (ICN/DCN)		S		1
		// (a.walling 2007-07-24 09:23) - PLID 26780 - Include Medicaid Resubmission code or Original Reference Number
		{
			CString strMedicaidResubmission = AdoFldString(fields, "MedicaidResubmission");
			CString strOriginalRefNo = AdoFldString(fields, "OriginalRefNo");

			CString strNum;

			if ( (strMedicaidResubmission.GetLength() > 0) && (strOriginalRefNo.GetLength() > 0) ) {
				strNum = strMedicaidResubmission;
			} else if (strMedicaidResubmission.GetLength() > 0) {
				strNum = strMedicaidResubmission;
			} else if (strOriginalRefNo.GetLength() > 0) {
				strNum = strOriginalRefNo;
			}

			if (strNum.GetLength()) {
				// we have a non-empty code here, so export it!

				//Ref.	Data		Name									Attributes
				//Des.	Element

				OutputString = "REF";

				//REF01 128			Reference Identification Qualifier		M	ID	2/3
				//F8 - Original Reference Number (ICN/DCN)
				OutputString += ParseANSIField("F8", 2, 3);
				
				//REF02 127			Reference Identification				X	AN	1/30
				
				OutputString += ParseANSIField(strNum, 1, 30);

				//REF03 NOT USED
				//REF04 NOT USED
				
				// already checked for length of strMedicaidResubmission above
				EndANSISegment(OutputString);
			}
		}

//193		180		REF		Investigational Device Exemption Number	S		1
		//not used

//195		180		REF		Service Authorization Exception Code	S		1
		//JJ - 4/25/2003 - This is really only used for NY State Medicaid,
		//and amusingly they don't support ANSI right now. But,
		//we may need to fill this element in the future.

//197		180		REF		Peer Review Org. (PRO) Approval Number	S		1
		//not used

///////////////////////////////////////////////////////////////////////////////

//198		180		REF		Prior Authorization or Referral Number	S		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01 128			Reference Identification Qualifier		M	ID	2/3
		//G1 - Prior Authorization Number
		//9F - Referral Number
		
		// (j.jones 2009-11-24 15:47) - PLID 36411 - use the qualifier from the HCFA Setup
		// (j.jones 2010-03-02 16:18) - PLID 37584 - this logic has changed, now we can
		// choose Prior Auth. or Referral Number from the bill, and the HCFA Setup acts
		// as an override, which can now be blank.
		CString strPriorAuthQual = m_UB92Info.PriorAuthQualifier;
		strPriorAuthQual.TrimLeft();
		strPriorAuthQual.TrimRight();
		if(strPriorAuthQual.IsEmpty()) {
			//if empty, then no override is in place,
			//so pick the qualifier from BillsT.PriorAuthType
			PriorAuthType patPriorAuthType = (PriorAuthType)VarLong(fields->Item["PriorAuthType"]->Value, (long)patPriorAuthNumber);
			if(patPriorAuthType == patReferralNumber) {
				strPriorAuthQual = "9F";
			}
			else {
				strPriorAuthQual = "G1";
			}
		}
		OutputString += ParseANSIField(strPriorAuthQual,2,3);
		
		//REF02 127			Reference Identification				X	AN	1/30
		//BillsT.PriorAuthNum
		var = fields->Item["PriorAuthNum"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";

		//use the override from the HCFA setup
		if(str == "")
			str = m_HCFAInfo.DefaultPriorAuthNum;

		OutputString += ParseANSIField(str,1,30);

		//REF03 NOT USED
		//REF04 NOT USED

		//don't output if blank
		if(str != "")
			EndANSISegment(OutputString);

		// (j.jones 2008-05-23 09:06) - PLID 29939 - fixed to use the UB92 setting for this
		if(m_UB92Info.ANSI_SendRefPhyIn2300 == 1 && 
			!IsRecordsetEmpty("SELECT ID FROM BillsT INNER JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
					"WHERE ID = %li",m_pEBillingInfo->BillID)) {

			//they want to send this segment again, with the referring physician, so do it

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

			//198		180		REF		Prior Authorization or Referral Number	S		2

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "REF";

			//REF01 128			Reference Identification Qualifier		M	ID	2/3
			//G1 - Prior Authorization Number
			//9F - Referral Number
			OutputString += ParseANSIField("9F",2,3);
			
			//REF02 127			Reference Identification				X	AN	1/30
			
			//always send the NPI number
			CString strNPI = AdoFldString(rs, "NPI", "");
			OutputString += ParseANSIField(strNPI,1,30);

			//REF03 NOT USED
			//REF04 NOT USED

			//don't output if blank
			if(!strNPI.IsEmpty()) {
				EndANSISegment(OutputString);
			}

			rs->Close();
		}

///////////////////////////////////////////////////////////////////////////////

//200		180		REF		Medical Record Number					S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01 128			Reference Identification Qualifier		M	ID	2/3
		//EA - Medical Record Identification Number
		OutputString += ParseANSIField("EA",2,3);
		
		//REF02 127			Reference Identification				X	AN	1/30
		//PatientsT.UserDefinedID
		str.Format("%li",m_pEBillingInfo->UserDefinedID);

		// (j.jones 2009-10-01 14:21) - PLID 35711 - added the ability to insert text in front of the patient ID
		if(m_bPrependPatientID) {
			str = m_strPrependPatientIDCode + str;
		}

		OutputString += ParseANSIField(str,1,30);

		//REF03 NOT USED
		//REF04 NOT USED

		EndANSISegment(OutputString);

//202		180		REF		Demonstration Project Identifier		S		1
//204		185		K3		File Information						S		10
//205		190		NTE		Claim Note								S		10
//208		190		NTE		Billing Note							S		1
//210		216		CR6		Home Health Care Information			S		1
//218		220		CRC		Home Health Functional Limitations		S		3
//221		220		CRC		Home Health Activities Permitted		S		3
//224		220		CRC		Home Health Mental Status				S		2
		//not used

///////////////////////////////////////////////////////////////////////////////

//227		231		HI		Principal, Admitting, E-Code and Patient Reason For Visit Diagnosis Information		R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HI";

		BOOL bOutput = TRUE;

		//HI01	C022		Health Care Code Info.					M	?	?

		//HI01-1	1270	Code List Qualifier Code				M	ID	1/3

		//static "BK" (principal diagnosis)
		OutputString += ParseANSIField("BK",1,3);

		//HI01-2	1271	Industry Code							M	AN	1/30

		//BillsT.DiagCode1

		// (a.walling 2014-03-19 10:11) - PLID 61418 - EBilling - ANSI4010 - Get diag info from bill
		CString strDiagCode1 = m_pEBillingInfo->GetSafeBillDiag(0).number;

		strDiagCode1.Replace(".","");
		strDiagCode1.TrimRight();

		if(strDiagCode1 == "")
			bOutput = FALSE;

		OutputString += ":";
		OutputString += strDiagCode1;

		//HI01-3	NOT USED
		//HI01-4	NOT USED
		//HI01-5	NOT USED
		//HI01-6	NOT USED
		//HI01-7	NOT USED

		//HI02	C022		Health Care Code Info.					O

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

		//HI02-1	1270	Code List Qualifier Code				M	ID	1/3

		//static "BJ"
		if(str != "") {
			OutputString += ParseANSIField("BJ",1,3);
		}

		//HI02-2	1271	Industry Code							M	AN	1/30

		//BillsT.DiagCode2

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//HI02-3	NOT USED
		//HI02-4	NOT USED
		//HI02-5	NOT USED
		//HI02-6	NOT USED
		//HI02-7	NOT USED

		//HI03	C022		Health Care Code Info.					O

		//with no easy way to backtrack, we won't output this element at all
		//if the diag code is blank

		//This is Box 77, E-Code, and we don't currently fill it in,
		//it is not necessarily DiagCode3!
		str = "";

		//HI03-2	1270	Code List Qualifier Code				M	ID	1/3

		//static "BN"
		if(str != "")
			OutputString += ParseANSIField("BN",1,3);

		//HI03-2	1271	Industry Code							M	AN	1/30

		//BillsT.DiagCode3

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//HI03-3	NOT USED
		//HI03-4	NOT USED
		//HI03-5	NOT USED
		//HI03-6	NOT USED
		//HI03-7	NOT USED

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

//230		231		HI		Diagnosis Related Group (DRG) Info.		S		1
		//not used

///////////////////////////////////////////////////////////////////////////////

//232		231		HI		Other Diagnosis Information				S		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HI";

		bOutput = TRUE;

		//HI01	C022		Health Care Code Info.					M	?	?

		//HI01-1	1270	Code List Qualifier Code				M	ID	1/3

		//static "BF" (diagnosis)
		OutputString += ParseANSIField("BF",1,3);

		//HI01-2	1271	Industry Code							M	AN	1/30

		//ChargesT.DiagCode2

		// (a.walling 2014-03-19 10:11) - PLID 61418 - EBilling - ANSI4010 - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(1).number;

		str.Replace(".","");
		str.TrimRight();

		if(str == "")
			bOutput = FALSE;

		OutputString += ":";
		OutputString += str;

		//HI01-3	NOT USED
		//HI01-4	NOT USED
		//HI01-5	NOT USED
		//HI01-6	NOT USED
		//HI01-7	NOT USED

		//HI02	C022		Health Care Code Info.					O

		//with no easy way to backtrack, we won't output this element at all
		//if the diag code is blank

	// (a.walling 2014-03-19 10:11) - PLID 61418 - EBilling - ANSI4010 - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(2).number;

		str.Replace(".","");
		str.TrimRight();

		//HI02-1	1270	Code List Qualifier Code				M	ID	1/3

		//static "BF"
		if(str != "")
			OutputString += ParseANSIField("BF",1,3);

		//HI02-2	1271	Industry Code							M	AN	1/30

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//HI02-3	NOT USED
		//HI02-4	NOT USED
		//HI02-5	NOT USED
		//HI02-6	NOT USED
		//HI02-7	NOT USED

		//HI03	C022		Health Care Code Info.					O

		//with no easy way to backtrack, we won't output this element at all
		//if the diag code is blank

		// (a.walling 2014-03-19 10:11) - PLID 61418 - EBilling - ANSI4010 - Get diag info from bill
		str = m_pEBillingInfo->GetSafeBillDiag(3).number;

		str.Replace(".","");
		str.TrimRight();

		//HI03-2	1270	Code List Qualifier Code				M	ID	1/3

		//static "BF"
		if(str != "")
			OutputString += ParseANSIField("BF",1,3);

		//HI03-2	1271	Industry Code							M	AN	1/30

		//BillsT.DiagCode4

		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//HI03-3	NOT USED
		//HI03-4	NOT USED
		//HI03-5	NOT USED
		//HI03-6	NOT USED
		//HI03-7	NOT USED

		//HI04	C022		Health Care Code Info.					M	?	?
		//HI05	C022		Health Care Code Info.					M	?	?
		//HI06	C022		Health Care Code Info.					M	?	?
		//HI07	C022		Health Care Code Info.					M	?	?
		//HI08	C022		Health Care Code Info.					M	?	?
		//HI09	C022		Health Care Code Info.					M	?	?
		//HI10	C022		Health Care Code Info.					M	?	?
		//HI11	C022		Health Care Code Info.					M	?	?
		//HI12	C022		Health Care Code Info.					M	?	?
		
		//this format supports a total of 12 additional diag codes

		// (j.jones 2009-03-25 17:40) - PLID 33669 - the rest of these we'll just do in a loop,
		// exporting the top 9 "extra" codes in the bill, for a total of 12 "additional" codes,
		// and 13 total codes on the claim.
		// (j.gruber 2014-03-17 11:51) - PLID 61394 - update ANSI4010 for billing structure
		// (a.walling 2014-03-19 10:11) - PLID 61418 - EBilling - ANSI4010 - Get diag info from bill
		for (int i = 4; i < (int)m_pEBillingInfo->billDiags.size() && i < 13; ++i) {

			str = m_pEBillingInfo->billDiags[i].number;
			str.Replace(".","");
			str.TrimRight();

			//HI0#-2	1270	Code List Qualifier Code				M	ID	1/3

			//static "BF" (diagnosis)
			if(str != "")
				OutputString += ParseANSIField("BF",1,3);

			//HI0#-2	1271	Industry Code							M	AN	1/30

			if(str != "") {
				OutputString += ":";
				OutputString += str;
			}
		}

		if(bOutput)
			EndANSISegment(OutputString);


///////////////////////////////////////////////////////////////////////////////

//242		231		HI		Principal Procedure Information			S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HI";

		bOutput = TRUE;

		//HI01	C022		Health Care Code Info.					M	?	?

		//HI01-1	1270	Code List Qualifier Code				M	ID	1/3

		//static "BP" (principal procedure code) or "BR" (ICD-9)
		// (j.jones 2009-04-24 11:12) - PLID 34070 - we need to handle the case where this is supposed to be blank
		if(m_UB92Info.Box80Number == 0) {
			//CPT Code
			str = "BP";
		}
		else if(m_UB92Info.Box80Number == 1) {
			//ICD-9 Code
			str = "BR";
		}
		else {
			str = "";
			bOutput = FALSE;
		}
		OutputString += ParseANSIField(str,1,3);

		//HI01-2	1271	Industry Code							M	AN	1/30

		//ChargesT.ItemCode OR DiagCodes.CodeNumber
		//DRT 4/10/2006 - PLID 11734 - Removed ProcCode
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		// (j.jones 2009-04-24 11:19) - PLID 34070 - this recordset won't be needed right now if this field is set to be blank,
		// but it is needed for the "Other Procedure Information" below
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		// (j.gruber 2014-03-17 11:46) - PLID 61394 - update the new billing structure
		// (a.walling 2014-03-19 10:11) - PLID 61418 - EBilling - ANSI4010 - Get diag info from bill
		_RecordsetPtr rsCPTCode = CreateParamRecordset("SELECT ChargesT.ItemCode, LineItemT.Date FROM ChargesT "
						"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
						"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
						"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
						"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
						"WHERE ChargesT.BillID = {INT} AND CPTCodeT.ID IS NOT NULL AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
						"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
						"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
						"ORDER BY LineID", m_pEBillingInfo->BillID);
		if(!rsCPTCode->eof) {
			if(m_UB92Info.Box80Number == 0) {
				//CPT Code
				var = rsCPTCode->Fields->Item["ItemCode"]->Value;
			}
			// (j.jones 2009-04-24 11:12) - PLID 34070 - we need to handle the case where this is supposed to be blank
			else if(m_UB92Info.Box80Number == 1) {
				//ICD-9 Code
				// (a.walling 2014-03-19 10:11) - PLID 61418 - EBilling - ANSI4010 - Get diag info from bill
				var = (LPCTSTR)m_pEBillingInfo->GetSafeBillDiag(0).number;
			}

			if(var.vt == VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "";
		}
		else
			str = "";

		// (j.jones 2011-06-17 12:30) - PLID 42268 - trim spaces
		str.TrimRight();

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

//244		231		HI		Other Procedure Information				S		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2009-06-09 17:30) - PLID 34556 - only send the other procedure
		// information if we sent a CPT in the primary procedure information,
		// as we do not support ICD-9s here currently
		if(m_UB92Info.Box80Number == 0) {

			OutputString = "HI";

			bOutput = TRUE;
		
			if(rsCPTCode->eof) {
				bOutput = FALSE;
			}
			//advance to the next code
			// (j.jones 2009-04-24 11:16) - PLID 34070 - only advance if
			// we sent the CPT in the previous segment
			else if(m_UB92Info.Box80Number == 0) {
				rsCPTCode->MoveNext();
			}

			if(rsCPTCode->eof) {
				bOutput = FALSE;
			}

			//loop up to 12 times, this is HI01-12

			int i = 0;
			while(!rsCPTCode->eof && i < 12) {

				//ChargesT.ItemCode
				CString strCode;
				if(!rsCPTCode->eof) {
					var = rsCPTCode->Fields->Item["ItemCode"]->Value;
					if(var.vt == VT_BSTR)
						strCode = CString(var.bstrVal);
					else
						strCode = "";
				}
				else
					strCode = "";

				strCode.TrimRight();

				// (j.jones 2011-06-17 12:21) - PLID 42268 - do not export this line if ItemCode is empty
				if(strCode.IsEmpty()) {
					rsCPTCode->MoveNext();
					continue;
				}

				i++;

				//HI01	C022		Health Care Code Info.					M	?	?

				//HI01-1	1270	Code List Qualifier Code				M	ID	1/3

				//static "BO" (other procedure code)

				// (j.jones 2009-06-09 17:32) - note, we can also send BQ here for ICD-9 codes,
				// we just don't actually do it yet

				OutputString += ParseANSIField("BO",1,3);

				//HI01-2	1271	Industry Code							M	AN	1/30

				OutputString += ":";
				OutputString += strCode;

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

				rsCPTCode->MoveNext();
			}

			if(bOutput)
				EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

//256		231		HI		Occurrence Span Information				S		2
		
		//Box 36, dates from and to

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HI";

		BOOL bOutputHI01_1 = TRUE;
		BOOL bOutputHI01_2 = TRUE;

		//HI01	C022		Health Care Code Info.					M	?	?

		//HI01-1	1270	Code List Qualifier Code				M	ID	1/3

		//static "BI"
		str = "BI";
		OutputString += ParseANSIField(str,1,3);

		//HI01-2	1271	Industry Code							M	AN	1/30

		//BillsT.UB92Box36

		//store the 32 - 35 values for later
		CString strBox32 = "";
		CString strBox33 = "";
		CString strBox34 = "";
		CString strBox35 = "";
		CString strBox36 = "";
		CString strUB04Box36 = "";
		CString strDateRange = "";
		CString strFirstDate = "";
		CString strConditionDate = "";
		
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		// (j.jones 2009-12-22 09:54) - PLID 27131 - included ConditionDate to use in Box 31
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		// (a.walling 2016-03-09 16:31) - PLID 68562 - UB04 Enhancements - update ANSI4010 for new data structure
		rs = CreateParamRecordset("; WITH BillsQ AS ( "
			"SELECT "
				"COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[1]', 'nvarchar(20)'), '') as UB92Box32 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[2]', 'nvarchar(20)'), '') as UB92Box33 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[3]', 'nvarchar(20)'), '') as UB92Box34 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[4]', 'nvarchar(20)'), '') as UB92Box35 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrenceSpans/occurrenceSpan/@code)[1]', 'nvarchar(20)'), '') as UB92Box36 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrenceSpans/occurrenceSpan/@code)[2]', 'nvarchar(20)'), '') as UB04Box36 "
				", LineItemT.Date as LineItemDate "
				", BillsT.ConditionDate "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE BillID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			") "
			"SELECT "
				"UB92Box32, UB92Box33, UB92Box34, UB92Box35, UB92Box36, UB04Box36 "
				", Min(LineItemDate) AS FirstDate, Max(LineItemDate) AS LastDate "
				", ConditionDate "
			"FROM BillsQ "
			"GROUP BY ConditionDate, UB92Box32, UB92Box33, UB92Box34, UB92Box35, UB92Box36, UB04Box36 ",
			m_pEBillingInfo->BillID);
		if(!rs->eof) {
			strBox32 = AdoFldString(rs, "UB92Box32","");
			strBox33 = AdoFldString(rs, "UB92Box33","");
			strBox34 = AdoFldString(rs, "UB92Box34","");
			strBox35 = AdoFldString(rs, "UB92Box35","");
			strBox36 = AdoFldString(rs, "UB92Box36","");
			strUB04Box36 = AdoFldString(rs, "UB04Box36","");
			COleDateTime dtFirst = AdoFldDateTime(rs, "FirstDate");
			COleDateTime dtLast = AdoFldDateTime(rs, "LastDate");
			strFirstDate = dtFirst.Format("%Y%m%d");
			strDateRange = strFirstDate + "-" + dtLast.Format("%Y%m%d");
			// (j.jones 2009-12-22 09:54) - PLID 27131 - included ConditionDate to use in Box 31			
			if(rs->Fields->Item["ConditionDate"]->Value.vt == VT_DATE) {
				COleDateTime dtCondition = AdoFldDateTime(rs, "ConditionDate");
				if(dtCondition.GetStatus() != COleDateTime::invalid) {
					strConditionDate = dtCondition.Format("%Y%m%d");
				}
			}
		}
		rs->Close();

		// (j.jones 2007-03-20 14:49) - PLID 23939 - if the UB04 Box 36 is filled
		// and UB92Box36 is not (which is Box 35 on the UB04), send the UB04Box36 first
		if(strBox36 == "") {
			strBox36 = strUB04Box36;
			strUB04Box36 = "";
			bOutputHI01_2 = FALSE;
		}

		if(strBox36 == "")
			bOutputHI01_1 = FALSE;

		if(strUB04Box36 == "")
			bOutputHI01_2 = FALSE;


		OutputString += ":";
		OutputString += strBox36;


		//HI01-3	1250	Date Time Period Format Qualifier		X	ID	2/3
		
		//static "RD8"
		OutputString += ":RD8";

		//HI01-4	1251	Date Time Period						X	AN	1/35

		//First/Last LineItemT.Date
		OutputString += ":";
		OutputString += strDateRange;
		
		//HI01-5	NOT USED
		//HI01-6	NOT USED
		//HI01-7	NOT USED

		//only output the second half if we have another date range
		if(bOutputHI01_2) {

			//HI02-1	1270	Code List Qualifier Code				M	ID	1/3

			//static "BI"
			str = "BI";
			OutputString += ParseANSIField(str,1,3);

			//HI02-2	1271	Industry Code							M	AN	1/30

			//BillsT.UB92Box36

			OutputString += ":";
			OutputString += strBox36;


			//HI02-3	1250	Date Time Period Format Qualifier		X	ID	2/3
			
			//static "RD8"
			OutputString += ":RD8";

			//HI02-4	1251	Date Time Period						X	AN	1/35

			//First/Last LineItemT.Date
			OutputString += ":";
			OutputString += strDateRange;
			
			//HI02-5	NOT USED
			//HI02-6	NOT USED
			//HI02-7	NOT USED
		}

		if(bOutputHI01_1)
			EndANSISegment(OutputString);


//267		231		HI		Occurrence Information					S		2
		//Boxes 32 - 35

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "HI*";

		bOutput = FALSE;

		//HI01	C022		Health Care Code Info.					M	?	?

		//HI01-1	1270	Code List Qualifier Code				M	ID	1/3

		//static "BH"
		str = "BH";

		//HI01-2	1271	Industry Code							M	AN	1/30

		//BillsT.UB92Box32

		str += ":";
		str += strBox32;

		//HI01-3	1250	Date Time Period Format Qualifier		X	ID	2/3
		
		//static "D8"
		str += ":D8";

		//HI01-4	1251	Date Time Period						X	AN	1/35

		// (j.jones 2009-12-22 09:53) - PLID 27131 - supported the option to show
		// the date of current accident in box 31, instead of the first charge date
		
		// This is confusing because in the UB92, the occurrence dates started with
		// Box 31, and in the UB04 they renumbered the boxes to start with Box 32.
		// Both our field names, and the ANSI specs, reflect the UB04 codes.
		// So we really want the Box 31 field, when we mean UB04 Box 32.

		if(m_UB92Info.UB04UseBox31Date == 1) {
			//Date Of Current Accident
			str += ":";
			str += strConditionDate;
		}
		else {
			//First LineItemT.Date
			str += ":";
			str += strFirstDate;
		}
		
		//HI01-5	NOT USED
		//HI01-6	NOT USED
		//HI01-7	NOT USED

		// (j.jones 2009-12-22 09:53) - PLID 27131 - if using accident date, don't output if it is blank
		if(strBox32 != "" && (m_UB92Info.UB04UseBox31Date == 0 || (m_UB92Info.UB04UseBox31Date == 1 && !strConditionDate.IsEmpty()))) {
			OutputString += str;
			OutputString += "*";
			bOutput = TRUE;
		}

		//HI01	C022		Health Care Code Info.					M	?	?

		//HI01-1	1270	Code List Qualifier Code				M	ID	1/3

		//static "BH"
		str = "BH";

		//HI01-2	1271	Industry Code							M	AN	1/30

		//BillsT.UB92Box33

		str += ":";
		str += strBox33;

		//HI01-3	1250	Date Time Period Format Qualifier		X	ID	2/3
		
		//static "D8"
		str += ":D8";

		//HI01-4	1251	Date Time Period						X	AN	1/35

		//First LineItemT.Date
		str += ":";
		str += strFirstDate;
		
		//HI01-5	NOT USED
		//HI01-6	NOT USED
		//HI01-7	NOT USED

		if(strBox33 != "") {
			OutputString += str;
			OutputString += "*";
			bOutput = TRUE;
		}

		//HI01	C022		Health Care Code Info.					M	?	?

		//HI01-1	1270	Code List Qualifier Code				M	ID	1/3

		//static "BH"
		str = "BH";

		//HI01-2	1271	Industry Code							M	AN	1/30

		//BillsT.UB92Box34

		str += ":";
		str += strBox34;

		//HI01-3	1250	Date Time Period Format Qualifier		X	ID	2/3
		
		//static "D8"
		str += ":D8";

		//HI01-4	1251	Date Time Period						X	AN	1/35

		//First LineItemT.Date
		str += ":";
		str += strFirstDate;
		
		//HI01-5	NOT USED
		//HI01-6	NOT USED
		//HI01-7	NOT USED

		if(strBox34 != "") {
			OutputString += str;
			OutputString += "*";
			bOutput = TRUE;
		}

		//HI01	C022		Health Care Code Info.					M	?	?

		//HI01-1	1270	Code List Qualifier Code				M	ID	1/3

		//static "BH"
		str = "BH";

		//HI01-2	1271	Industry Code							M	AN	1/30

		//BillsT.UB92Box35

		str += ":";
		str += strBox35;

		//HI01-3	1250	Date Time Period Format Qualifier		X	ID	2/3
		
		//static "D8"
		str += ":D8";

		//HI01-4	1251	Date Time Period						X	AN	1/35

		//First LineItemT.Date
		str += ":";
		str += strFirstDate;
		
		//HI01-5	NOT USED
		//HI01-6	NOT USED
		//HI01-7	NOT USED

		if(strBox35 != "") {
			OutputString += str;
			OutputString += "*";
			bOutput = TRUE;
		}

		if(bOutput)
			EndANSISegment(OutputString);

//280		231		HI		Value Information						S		2
		//Boxes 39 - 41. We don't fill this in!

//290		231		HI		Condition Information					S		2
		//Boxes 24 - 30, We don't fill this in!

//299		231		HI		Treatment Code Information				S		2
		//not used

//306		240		QTY		Claim Quantity							S		4
		//Boxes 7 - 10, not used.

//308		241		HCP		Claim Pricing/Repricing Information		S		1
		//not used that we know of

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2300_Inst");

	return Error_Other;
}

/*
int CEbilling::ANSI_4010_2305() {

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

	} NxCatchAll("Error in Ebilling::ANSI_4010_2305");

	return Error_Other;
}
*/

// (j.jones 2011-03-07 14:17) - PLID 42260 - added ability to force this function to load
// the 2310B IDs of the rendering provider
int CEbilling::ANSI_4010_2310A_Prof(BOOL bUseRenderingProvider) {

	//Referring Provider Name

#ifdef _DEBUG

	CString str;

	str = "\r\n2310A\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

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

//282		250		NM1		Referring Provider Name					S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M	ID	2/3

		//static value "DN" (referring physician)

		OutputString += ParseANSIField("DN", 2, 3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

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

		//NM103	1035		Name Last / Org Name					O	AN	1/35

		if(bUseProviderCompanyOnClaims) {
			str = strProviderCompany;
		}
		else {
			str = GetFieldFromRecordset(rs, "Last");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 35);

		//NM104	1036		Name First								O	AN	1/25

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "First");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 25);

		//NM105	1037		Name Middle								O	AN	1/25

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

		//NM107	1039		Name Suffix								O	AN	1/10

		// (j.jones 2007-08-03 11:44) - PLID 26934 - changed from Suffix to Title
		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "Title");
		}

		OutputString += ParseANSIField(str, 1, 10);

		//NM108	66			ID Code Qualifier						X	ID	1/2

		//Allowed Values:
		//	XX	- NPI
		//	24	- Employer ID
		//	34	- SSN

		CString strIdent, strID;

		// (j.jones 2006-11-14 08:55) - PLID 23413 - determine whether the
		// NM108/109 elements should show the NPI or the EIN/SSN
		long nNM109_IDType = m_HCFAInfo.NM109_IDType;

		// (j.jones 2011-03-07 14:19) - PLID 42660 - if bUseRenderingProvider is true,
		// export the rendering provider's info instead of the referring physician
		if(bUseRenderingProvider) {

			if(nNM109_IDType == 1) {

				// (j.jones 2006-11-14 08:57) - PLID 23413 - if 1,
				// then use the fed. employer ID or SSN, based on Box25

				//"24" for EIN, "34" for SSN - from HCFASetupT.Box25
				if(m_HCFAInfo.Box25==1)
					strIdent = "34";
				else
					strIdent = "24";

				if(m_HCFAInfo.Box25 == 1) {
					strID = GetFieldFromRecordset(rs, "SocialSecurity");
				}
				else {
					strID = GetFieldFromRecordset(rs, "Fed Employer ID");
				}
			}
			// (j.jones 2009-01-06 10:00) - PLID 32614 - supported hiding Box 17b NPI, which also applies to ANSI
			else if(m_HCFAInfo.HideBox17b == 0) {

				// (j.jones 2006-11-14 08:58) - PLID 23413 - if 0,
				// then use the NPI

				//"XX" for NPI
				strIdent = "XX";

				// (j.jones 2007-08-08 13:14) - PLID 25395 - if there is a 24J override in AdvHCFAPinT, use it here
				// (j.jones 2008-04-03 09:55) - PLID 28995 - this function now uses the ANSI_RenderingProviderID value, NOT the Box24J_ProviderID
				// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
				_RecordsetPtr rsOver = CreateParamRecordset("SELECT Box24JNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ANSI_RenderingProviderID, m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
				if(!rsOver->eof) {
					var = rsOver->Fields->Item["Box24JNPI"]->Value;
					if(var.vt == VT_BSTR) {
						CString strOver = VarString(var,"");
						strOver.TrimLeft();
						strOver.TrimRight();
						if(!strOver.IsEmpty())
							strID = strOver;
					}
				}
				rsOver->Close();

				// (j.jones 2011-06-23 13:56) - PLID 44295 - we checked the wrong field for IsEmpty!
				if(strID.IsEmpty()) {
					strID = GetFieldFromRecordset(rs, "NPI");
				}
			}
		}
		else {
			//normal case, send the referring physician info

			if(nNM109_IDType == 1) {

				// (j.jones 2006-11-14 08:57) - PLID 23413 - if 1,
				// then use the fed. employer ID.

				//"24" for employer ID
				strIdent = "24";
				strID = GetFieldFromRecordset(rs, "FedEmployerID");
			}
			else {

				// (j.jones 2006-11-14 08:58) - PLID 23413 - if 0,
				// then use the NPI

				// (j.jones 2009-01-06 10:00) - PLID 32614 - supported hiding Box 17b NPI, which also applies to ANSI
				if(m_HCFAInfo.HideBox17b == 0) {
					//"XX" for NPI
					strIdent = "XX";
					strID = GetFieldFromRecordset(rs, "NPI");
				}
			}
		}

		strID = StripNonNum(strID);

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

		EndANSISegment(OutputString);

//285		255		PRV		Referring Provider Specialty Info.		S		1

		//JMJ 3/11/2004 - we never exported this PRV segment before and never had trouble with it,
		//but now we support Taxonomy Codes for Referring Physicians.
		//I'm going to make it so we only export the PRV segment here if we have a taxonomy code.
		CString strTaxonomy = "";
		// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore
		if(bUseRenderingProvider) {
			strTaxonomy = AdoFldString(rs, "TaxonomyCode","");
			strTaxonomy.TrimLeft();
			strTaxonomy.TrimRight();
		}

		if(strTaxonomy != "") {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "PRV";

			//PRV01	1221		Provider Code							M	ID	1/3

			//static value "RF"

			OutputString += ParseANSIField("RF", 1, 3);		

			//PRV02	128			Reference Identification Qualifier		M	ID	2/3

			//static value "ZZ"
			OutputString += ParseANSIField("ZZ",2,3);

			//PRV03	127			Reference Identification				M	AN	1/30

			//ProvidersT.TaxonomyCode
			//This comes from a published HIPAA list.
			//"2086S0122X" is for Plastic and Reconstructive Surgery
			OutputString += ParseANSIField(strTaxonomy,1,30);

			//PRV04	NOT USED

			OutputString += "*";

			//PRV05	NOT USED

			OutputString += "*";

			//PRV06	NOT USED

			OutputString += "*";

			EndANSISegment(OutputString);
		}

//288		271		REF		Referring Provider Secondary Info.		S		5

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		// (j.jones 2008-05-02 10:09) - PLID 27478 - we need to silently skip
		// adding additional REF segments that have the same qualifier, so
		// track them in an array
		CStringArray arystrQualifiers;

		//REF01	128			Reference Identification Qualifier		M	ID	2/3

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
				Log("Ebilling file tried to export 2310A REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
			}
		}
		else {
			//normal case, send the referring physician info

			//Box17aANSI will give us the qualifier AND ID, even if the ID is blank

			var = rs->Fields->Item["ID"]->Value;
			strIdent = "", strID = "";
			// (j.jones 2010-10-20 15:42) - PLID 40936 - moved Box17aANSI to GlobalFinancialUtils and renamed it
			EBilling_Box17aANSI(m_HCFAInfo.Box17a, m_HCFAInfo.Box17aQual, strIdent,strID,var.lVal);

			// (j.jones 2006-11-28 16:01) - PLID 23651 - removed ANSIRefPhyQual as an override,
			// since HCFA Groups now have Box 17a qualifier fields
		}
		
		OutputString += ParseANSIField(strIdent,2,3);

		//REF02	127			Reference Identification				X	AN	1/30

		OutputString += ParseANSIField(strID,1,30);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		// (j.jones 2007-09-20 10:51) - PLID 27455 - do not send this segment if the
		// qualifier or ID are blank
		if(strIdent != "" && strID != "") {
			EndANSISegment(OutputString);

			// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
			arystrQualifiers.Add(strIdent);
		}

		// (j.jones 2006-11-14 11:48) - PLID 23413 - the HCFA Group now allows the option
		// to append an EIN, or NPI as an additional REF segment

		long nExtraREF_IDType = m_HCFAInfo.ExtraREF_IDType;

		//0 - EIN/SSN, 1 - NPI, 2 - none
		if(nExtraREF_IDType != 2) {

			//export the extra ID

			OutputString = "REF";

			//REF01	128			Reference Identification Qualifier		M	ID	2/3
			
			strIdent = "";
			strID = "";

			// (j.jones 2011-03-07 14:19) - PLID 42660 - if bUseRenderingProvider is true,
			// export the rendering provider's info instead of the referring physician
			if(bUseRenderingProvider) {

				if(nExtraREF_IDType == 0) {
					// (j.jones 2006-11-14 08:57) - PLID 23413 - if 1,
					// then use the fed. employer ID or SSN, based on Box25

					//"EI" for EIN, "SY" for SSN - from HCFASetupT.Box25
					if(m_HCFAInfo.Box25==1)
						strIdent = "SY";
					else
						strIdent = "EI";

					if(m_HCFAInfo.Box25 == 1) {
						strID = GetFieldFromRecordset(rs, "SocialSecurity");
					}
					else {
						strID = GetFieldFromRecordset(rs, "Fed Employer ID");
					}
				}
				else {
					//NPI

					//"XX" for NPI
					strIdent = "XX";

					// (j.jones 2007-08-08 13:14) - PLID 25395 - if there is a 24J override in AdvHCFAPinT, use it here
					// (j.jones 2008-04-03 09:55) - PLID 28995 - this function now uses the ANSI_RenderingProviderID value, NOT Box24J_ProviderID
					// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
					_RecordsetPtr rsOver = CreateParamRecordset("SELECT Box24JNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ANSI_RenderingProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
					if(!rsOver->eof) {
						var = rsOver->Fields->Item["Box24JNPI"]->Value;
						if(var.vt == VT_BSTR) {
							CString strOver = VarString(var,"");
							strOver.TrimLeft();
							strOver.TrimRight();
							if(!strOver.IsEmpty())
								strID = strOver;
						}
					}
					rsOver->Close();

					if(strID.IsEmpty()) {
						strID = AdoFldString(rs, "NPI", "");
					}
				}
			}
			else {
				//normal case, export the referring physician info

				if(nExtraREF_IDType == 0) {
					//EIN

					//"EI" for employer ID
					strIdent = "EI";
					strID = GetFieldFromRecordset(rs, "FedEmployerID");
				}
				else {
					//NPI

					// (j.jones 2009-01-06 10:00) - PLID 32614 - supported hiding Box 17b NPI, which also applies to ANSI
					if(m_HCFAInfo.HideBox17b == 0) {
						//"XX" for NPI
						strIdent = "XX";
						strID = AdoFldString(rs, "NPI", "");
					}
				}
			}

			strID = StripNonNum(strID);

			OutputString += ParseANSIField(strIdent,2,3);

			//REF02	127			Reference Identification				X	AN	1/30

			OutputString += ParseANSIField(strID,1,30);

			//REF03	NOT USED
			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strIdent.CompareNoCase("XX") == 0) {
				strIdent = "";
				strID = "";
				//log that this happened
				Log("Ebilling file tried to export 2310A Additional REF*XX for bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->BillID);
			}

			// (j.jones 2008-05-02 10:02) - PLID 27478 - disallow sending the additional REF
			// segment if the qualifier has already been used
			if(strIdent != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdent)) {
				EndANSISegment(OutputString);

				// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
				arystrQualifiers.Add(strIdent);
			}
		}

		rs->Close();


///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2310A_Prof");

	return Error_Other;
}

int CEbilling::ANSI_4010_2310A_Inst() {

	//Attending Physician Name

	//This loop is required on all inpatient claims

	//This loop represents the provider listed in Box 82 on the UB92

#ifdef _DEBUG

	CString str;

	str = "\r\n2310A\r\n";
	m_OutputFile.Write(str,str.GetLength());

#endif

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310A - ATTENDING PHYSICIAN NAME						1

//321		250		NM1		Attending Physician Name				S		1

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
				"WHERE PersonT.ID = {INT}", m_pEBillingInfo->ProviderID);
		}
		else {
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			// (j.jones 2011-11-07 14:44) - PLID 46299 - added support for ProvidersT.UseCompanyOnClaims
			rs = CreateParamRecordset("SELECT PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, PersonT.SocialSecurity, PersonT.Company, "
				"ProvidersT.* "
				"FROM PersonT LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
				"WHERE PersonT.ID = {INT}", m_pEBillingInfo->ProviderID);
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

		//NM101	98			Entity ID Code							M	ID	2/3

		//static value "71"

		OutputString += ParseANSIField("71", 2, 3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

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

		//NM103	1035		Name Last / Org Name					O	AN	1/35

		if(bUseProviderCompanyOnClaims) {
			str = strProviderCompany;
		}
		else {
			str = GetFieldFromRecordset(rs, "Last");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 35);

		//NM104	1036		Name First								O	AN	1/25

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "First");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 25);

		//NM105	1037		Name Middle								O	AN	1/25

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

		//NM107	1039		Name Suffix								O	AN	1/10

		// (j.jones 2007-08-03 11:44) - PLID 26934 - changed from Suffix to Title
		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "Title");
		}

		OutputString += ParseANSIField(str, 1, 10);

		//NM108	66			ID Code Qualifier						X	ID	1/2

		//Allowed Values:
		//		24		Employer's ID Number
		//		34		SSN
		//		XX		Health Care Financing National Provider ID

		CString strQual;
		str = "";

		// (j.jones 2006-11-14 08:55) - PLID 23446 - determine whether the
		// NM108/109 elements should show the NPI or the EIN
		long nNM109_IDType = m_UB92Info.NM109_IDType;

		if(nNM109_IDType == 1) {

			// (j.jones 2006-11-14 08:57) - PLID 23446 - if 1,
			// then use the fed. employer ID.

			//"24" for employer ID
			strQual = "24";
			// (j.jones 2007-05-10 15:06) - PLID 25948 - supported referring physician
			if(m_pEBillingInfo->Box82Setup == 3)
				str = GetFieldFromRecordset(rs, "FedEmployerID");
			else
				str = GetFieldFromRecordset(rs, "Fed Employer ID");
		}
		else {

			// (j.jones 2006-11-14 08:58) - PLID 23446 - if 0,
			// then use the NPI

			//"XX" for NPI
			strQual = "XX";
			str = GetFieldFromRecordset(rs, "NPI");
		}

		str = StripNonNum(str);

		if(str != "")
			OutputString += ParseANSIField(strQual, 1, 2);
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

		EndANSISegment(OutputString);

//324		255		PRV		Attending Physician Specialty Info.		S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "PRV";

		//PRV01	1221		Provider Code							M	ID	1/3

		//static value "AT"

		OutputString += ParseANSIField("AT", 1, 3);

		CString strTaxonomy = "";
		// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore
		if(m_pEBillingInfo->Box82Setup != 3) {
			strTaxonomy = AdoFldString(rs, "TaxonomyCode","");
			strTaxonomy.TrimLeft();
			strTaxonomy.TrimRight();
		}

		// (j.jones 2008-05-01 10:47) - PLID 28691 - don't output if the taxonomy code is blank
		if(!strTaxonomy.IsEmpty()) {

			//PRV02	128			Reference Identification Qualifier		M	ID	2/3

			//static value "ZZ"
			OutputString += ParseANSIField("ZZ",2,3);

			//PRV03	127			Reference Identification				M	AN	1/30

			//ProvidersT.TaxonomyCode
			//This comes from a published HIPAA list.
			//"2086S0122X" is for Plastic and Reconstructive Surgery
			OutputString += ParseANSIField(strTaxonomy,1,30);

			//PRV04	NOT USED

			OutputString += "*";

			//PRV05	NOT USED

			OutputString += "*";

			//PRV06	NOT USED

			OutputString += "*";

			EndANSISegment(OutputString);
		}

//326		271		REF		Attending Physician Secondary Info.		S		5

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		// (j.jones 2008-05-02 10:09) - PLID 27478 - we need to silently skip
		// adding additional REF segments that have the same qualifier, so
		// track them in an array
		CStringArray arystrQualifiers;

		//REF01	128			Reference Identification Qualifier		M	ID	2/3
		
		// (j.jones 2010-04-14 10:43) - PLID 38194 - the ID is now calculated in a shared function
		CString strIdentifier = "", strID = "", strLoadedFrom = "";
		EBilling_Calculate2310A_REF(strIdentifier,strID,strLoadedFrom,m_pEBillingInfo->ProviderID,
			m_pEBillingInfo->UB92SetupID, m_UB92Info.UB04Box76Qual, m_UB92Info.Box82Num, m_UB92Info.Box82Setup,
			m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->InsuredPartyID, m_pEBillingInfo->BillLocation);

		// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
		if(m_bDisableREFXX && strIdentifier.CompareNoCase("XX") == 0) {
			strIdentifier = "";
			strID = "";
			//log that this happened
				Log("Ebilling file tried to export 2310A REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
		}
		
		OutputString += ParseANSIField(strIdentifier,2,3);

		//REF02	127			Reference Identification				X	AN	1/30

		OutputString += ParseANSIField(strID,1,30);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		// (j.jones 2007-09-20 10:51) - PLID 27455 - do not send this segment if the
		// qualifier or ID are blank
		if(strIdentifier != "" && strID != "") {
			EndANSISegment(OutputString);

			// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
			arystrQualifiers.Add(strIdentifier);
		}

		// (j.jones 2006-11-14 11:48) - PLID 23446 - the UB92 Group now allows the option
		// to append an EIN or NPI as an additional REF segment

		long nExtraREF_IDType = m_UB92Info.ExtraREF_IDType;

		//0 - EIN/SSN, 1 - NPI, 2 - none
		if(nExtraREF_IDType != 2) {

			//export the extra ID

			OutputString = "REF";

			//REF01	128			Reference Identification Qualifier		M	ID	2/3
			
			strIdentifier = "";
			strID = "";

			if(nExtraREF_IDType == 0) {
				//EIN

				//"EI" for employer ID
				strIdentifier = "EI";
				// (j.jones 2007-05-10 15:06) - PLID 25948 - supported referring physician
				if(m_pEBillingInfo->Box82Setup == 3)
					strID = GetFieldFromRecordset(rs, "FedEmployerID");
				else
					strID = GetFieldFromRecordset(rs, "Fed Employer ID");
			}
			else {
				//NPI

				//"XX" for NPI
				strIdentifier = "XX";
				strID = AdoFldString(rs, "NPI", "");
			}

			strID = StripNonNum(strID);

			OutputString += ParseANSIField(strIdentifier,2,3);

			//REF02	127			Reference Identification				X	AN	1/30

			OutputString += ParseANSIField(strID,1,30);

			//REF03	NOT USED
			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strIdentifier.CompareNoCase("XX") == 0) {
				strIdentifier = "";
				strID = "";
				//log that this happened
				Log("Ebilling file tried to export 2310A Additional REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
			}

			// (j.jones 2008-05-02 10:02) - PLID 27478 - disallow sending the additional REF
			// segment if the qualifier has already been used
			if(strIdentifier != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdentifier)) {
				EndANSISegment(OutputString);

				// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
				arystrQualifiers.Add(strIdentifier);
			}
		}

		rs->Close();

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2310A_Inst");

	return Error_Other;
}

int CEbilling::ANSI_4010_2310B_Prof() {

	//Rendering Provider Name

	//This loop is not used when 2000A's PRV segment is used.
	//This loop will not be used when the Rendering Provider is the same entity
	//as the Billing Provider and/or the Pay-To Provider.

	//This loop will be used when the Billing or Pay-To Provider is a group.
	//This loop will then represent the individual Rendering Provider.

	//This loop will be used if m_bIsGroup is TRUE.

#ifdef _DEBUG

	CString str;

	str = "\r\n2310B\r\n";
	m_OutputFile.Write(str,str.GetLength());

#endif

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310B - RENDERING PROVIDER NAME						1

//290		250		NM1		Rendering Provider Name					S		1

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

		//NM101	98			Entity ID Code							M	ID	2/3

		//static value "82"

		OutputString += ParseANSIField("82", 2, 3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

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

		//NM103	1035		Name Last / Org Name					O	AN	1/35

		if(bUseProviderCompanyOnClaims) {
			str = strProviderCompany;
		}
		else {
			str = GetFieldFromRecordset(rs, "Last");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 35);

		//NM104	1036		Name First								O	AN	1/25

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "First");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 25);

		//NM105	1037		Name Middle								O	AN	1/25

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

		//NM107	1039		Name Suffix								O	AN	1/10

		// (j.jones 2007-08-03 11:44) - PLID 26934 - changed from Suffix to Title
		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "Title");
		}

		OutputString += ParseANSIField(str, 1, 10);

		//NM108	66			ID Code Qualifier						X	ID	1/2

		//Allowed Values:
		//		24		Employer's ID Number
		//		34		SSN
		//		XX		Health Care Financing National Provider ID

		CString strQual;
		str = "";

		// (j.jones 2006-11-14 08:55) - PLID 23413 - determine whether the
		// NM108/109 elements should show the NPI or the EIN/SSN
		long nNM109_IDType = m_HCFAInfo.NM109_IDType;

		if(nNM109_IDType == 1) {

			// (j.jones 2006-11-14 08:57) - PLID 23413 - if 1,
			// then use the fed. employer ID or SSN, based on Box25

			//"24" for EIN, "34" for SSN - from HCFASetupT.Box25
			if(m_HCFAInfo.Box25==1)
				strQual = "34";
			else
				strQual = "24";

			if(m_HCFAInfo.Box25 == 1) {
				str = GetFieldFromRecordset(rs, "SocialSecurity");
			}
			else {
				str = GetFieldFromRecordset(rs, "Fed Employer ID");
			}
		}
		else {

			// (j.jones 2006-11-14 08:58) - PLID 23413 - if 0,
			// then use the NPI

			//"XX" for NPI
			strQual = "XX";

			// (j.jones 2007-08-08 13:14) - PLID 25395 - if there is a 24J override in AdvHCFAPinT, use it here
			// (j.jones 2008-04-03 09:55) - PLID 28995 - this function now uses the ANSI_RenderingProviderID value, NOT the Box24J_ProviderID
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
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
		}

		str = StripNonNum(str);

		if(str != "")
			OutputString += ParseANSIField(strQual, 1, 2);
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

		EndANSISegment(OutputString);

//293		255		PRV		Rendering Provider Specialty Info.		R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2005-09-29 10:15) - According to THIN, if we are sending 2310B for a non-group,
		// then the PRV taxonomy code segment should not be sent, because we already sent it in 2000A,
		// so we will not send UNLESS it's a group or is forced by the ANSI Properties to do so,
		// which is consistent with the 2000A code which does not send the PRV segment if it is a group

		OutputString = "PRV";

		//PRV01	1221		Provider Code							M	ID	1/3

		//static value "PE"

		OutputString += ParseANSIField("PE", 1, 3);

		CString strTaxonomy = AdoFldString(rs, "TaxonomyCode","");
		strTaxonomy.TrimLeft();
		strTaxonomy.TrimRight();

		// (j.jones 2008-05-01 10:47) - PLID 28691 - don't output if the taxonomy code is blank
		if(!strTaxonomy.IsEmpty()) {

			//PRV02	128			Reference Identification Qualifier		M	ID	2/3

			//static value "ZZ"
			OutputString += ParseANSIField("ZZ",2,3);

			//PRV03	127			Reference Identification				M	AN	1/30

			//ProvidersT.TaxonomyCode
			//This comes from a published HIPAA list.
			//"2086S0122X" is for Plastic and Reconstructive Surgery
			OutputString += ParseANSIField(strTaxonomy,1,30);

			//PRV04	NOT USED

			OutputString += "*";

			//PRV05	NOT USED

			OutputString += "*";

			//PRV06	NOT USED

			OutputString += "*";

			// (j.jones 2005-09-29 10:16) - only send if a group or forced by the ANSI Properties

			if(m_bIsGroup || m_HCFAInfo.Use2310BPRVSegment) {
				EndANSISegment(OutputString);
			}
		}

//296		271		REF		Rendering Provider Secondary Info.		S		5

		//Ref.	Data		Name									Attributes
		//Des.	Element

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
				Log("Ebilling file tried to export 2310B REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
		}

		//REF01	128			Reference Identification Qualifier		M	ID	2/3
		
		OutputString += ParseANSIField(strIdentifier,2,3);

		//REF02	127			Reference Identification				X	AN	1/30

		OutputString += ParseANSIField(strID,1,30);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		// (j.jones 2007-09-20 10:51) - PLID 27455 - do not send this segment if the
		// qualifier or ID are blank
		if(strIdentifier != "" && strID != "") {
			EndANSISegment(OutputString);

			// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
			arystrQualifiers.Add(strIdentifier);
		}

		// (j.jones 2006-11-14 11:48) - PLID 23413 - the HCFA Group now allows the option
		// to append an EIN/SSN, or NPI as an additional REF segment

		long nExtraREF_IDType = m_HCFAInfo.ExtraREF_IDType;

		//0 - EIN/SSN, 1 - NPI, 2 - none
		if(nExtraREF_IDType != 2) {

			//export the extra ID

			OutputString = "REF";

			//REF01	128			Reference Identification Qualifier		M	ID	2/3
			
			strIdentifier = "";
			strID = "";

			if(nExtraREF_IDType == 0) {
				// (j.jones 2006-11-14 08:57) - PLID 23413 - if 1,
				// then use the fed. employer ID or SSN, based on Box25

				//"EI" for EIN, "SY" for SSN - from HCFASetupT.Box25
				if(m_HCFAInfo.Box25==1)
					strIdentifier = "SY";
				else
					strIdentifier = "EI";

				if(m_HCFAInfo.Box25 == 1) {
					strID = GetFieldFromRecordset(rs, "SocialSecurity");
				}
				else {
					strID = GetFieldFromRecordset(rs, "Fed Employer ID");
				}
			}
			else {
				//NPI

				//"XX" for NPI
				strIdentifier = "XX";

				// (j.jones 2007-08-08 13:14) - PLID 25395 - if there is a 24J override in AdvHCFAPinT, use it here
				// (j.jones 2008-04-03 09:55) - PLID 28995 - this function now uses the ANSI_RenderingProviderID value, NOT Box24J_ProviderID
				// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
				_RecordsetPtr rsOver = CreateParamRecordset("SELECT Box24JNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",m_pEBillingInfo->ANSI_RenderingProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
				if(!rsOver->eof) {
					var = rsOver->Fields->Item["Box24JNPI"]->Value;
					if(var.vt == VT_BSTR) {
						CString strOver = VarString(var,"");
						strOver.TrimLeft();
						strOver.TrimRight();
						if(!strOver.IsEmpty())
							strID = strOver;
					}
				}
				rsOver->Close();

				if(strID.IsEmpty()) {
					strID = AdoFldString(rs, "NPI", "");
				}
			}

			strID = StripNonNum(strID);

			OutputString += ParseANSIField(strIdentifier,2,3);

			//REF02	127			Reference Identification				X	AN	1/30

			OutputString += ParseANSIField(strID,1,30);

			//REF03	NOT USED
			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strIdentifier.CompareNoCase("XX") == 0) {
				strIdentifier = "";
				strID = "";
				//log that this happened
				Log("Ebilling file tried to export 2310B Additional REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
			}

			// (j.jones 2008-05-02 10:02) - PLID 27478 - disallow sending the additional REF
			// segment if the qualifier has already been used
			if(strIdentifier != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdentifier)) {
				EndANSISegment(OutputString);

				// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
				arystrQualifiers.Add(strIdentifier);
			}
		}

		rs->Close();

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2310B_Prof");

	return Error_Other;
}

// (j.jones 2011-08-19 10:00) - PLID 44984 - supported 2310B_Inst
int CEbilling::ANSI_4010_2310B_Inst() {

	//Operating Physician Name

	//Supposedly this is needed on all claims with a surgical procedure code.
	//We simply look at the Box 77 setup.

	CString OutputString,str;

	try {

#ifdef _DEBUG

	str = "\r\n2310B\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310B - OPERATING PHYSICIAN NAME						1

//328		250		NM1		Operating Physician Name				S		1

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

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "Middle");
			str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		}
		OutputString += ParseANSIField(str, 1, 25);

		//NM106	NOT USED

		OutputString += "*";

		//NM107	1039		Name Suffix								O	AN	1/10

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "Title");
		}
		OutputString += ParseANSIField(str, 1, 10);

		//NM108	66			ID Code Qualifier						X	ID	1/2

		//Allowed Values:
		//		24		Employer's ID Number
		//		34		SSN
		//		XX		Health Care Financing National Provider ID

		CString strQual;
		str = "";

		long nNM109_IDType = m_UB92Info.NM109_IDType;

		if(nNM109_IDType == 1) {
			//"24" for employer ID
			strQual = "24";
			if(m_pEBillingInfo->Box77Setup == 3)
				str = GetFieldFromRecordset(rs, "FedEmployerID");
			else
				str = GetFieldFromRecordset(rs, "Fed Employer ID");
		}
		else {
			//"XX" for NPI
			strQual = "XX";
			str = GetFieldFromRecordset(rs, "NPI");
		}

		str = StripNonNum(str);

		if(str != "")
			OutputString += ParseANSIField(strQual, 1, 2);
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

		EndANSISegment(OutputString);

		// (j.jones 2011-10-12 10:30) - PLID 45918 - removed the PRV segment, it does not exist in 4010A1

//333	271		REF		Operating Physician Secondary Info.		S		5

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
				Log("Ebilling file tried to export 2310B REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
		}
		
		OutputString += ParseANSIField(strIdentifier,2,3);

		//REF02	127			Reference Identification				X	AN	1/30

		OutputString += ParseANSIField(strID,1,30);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		// do not send this segment if the
		// qualifier or ID are blank
		if(strIdentifier != "" && strID != "") {
			EndANSISegment(OutputString);

			//add this qualifier to our array
			arystrQualifiers.Add(strIdentifier);
		}

		// the UB92 Group now allows the option
		// to append an EIN or NPI as an additional REF segment

		long nExtraREF_IDType = m_UB92Info.ExtraREF_IDType;

		//0 - EIN/SSN, 1 - NPI, 2 - none
		if(nExtraREF_IDType != 2) {

			//export the extra ID

			OutputString = "REF";

			//REF01	128			Reference Identification Qualifier		M	ID	2/3
			
			strIdentifier = "";
			strID = "";

			if(nExtraREF_IDType == 0) {
				//EIN

				//"EI" for employer ID
				strIdentifier = "EI";
				if(m_pEBillingInfo->Box77Setup == 3)
					strID = GetFieldFromRecordset(rs, "FedEmployerID");
				else
					strID = GetFieldFromRecordset(rs, "Fed Employer ID");
			}
			else {
				//NPI

				//"XX" for NPI
				strIdentifier = "XX";
				strID = AdoFldString(rs, "NPI", "");
			}

			strID = StripNonNum(strID);

			OutputString += ParseANSIField(strIdentifier,2,3);

			//REF02	127			Reference Identification				X	AN	1/30

			OutputString += ParseANSIField(strID,1,30);

			//REF03	NOT USED
			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			//if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strIdentifier.CompareNoCase("XX") == 0) {
				strIdentifier = "";
				strID = "";
				//log that this happened
				Log("Ebilling file tried to export 2310B Additional REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
			}

			// disallow sending the additional REF
			// segment if the qualifier has already been used
			if(strIdentifier != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdentifier)) {
				EndANSISegment(OutputString);

				//add this qualifier to our array
				arystrQualifiers.Add(strIdentifier);
			}
		}

		rs->Close();

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2310B_Inst");

	return Error_Other;
}

/*
int CEbilling::ANSI_4010_2310C_Prof() {

	//Purchased Service Provider Name

	//This loop is not used. A Purchased Service Provider is not a
	//concept addressed in Practice.

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310C	- PURCHASED SERVICE PROVIDER NAME				1

//298		250		NM1		Purchased Service Provider Name			S		1
//301		271		REF		Purchased Service Prov. Secondary Info.	S		5

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2310C_Prof");

	return Error_Other;
}
*/

// (j.jones 2011-08-19 10:00) - PLID 44984 - supported 2310C_Inst
int CEbilling::ANSI_4010_2310C_Inst() {

	//Other Provider Name

	//We fill this based on Box 78.

	CString OutputString,str;

	try {

#ifdef _DEBUG

	CString str;

	str = "\r\n2310C\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310C - OTHER PROVIDER NAME						1

//335		250		NM1		Other Provider Name						S		1

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

		//static value "73"

		OutputString += ParseANSIField("73", 2, 3);

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

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "Middle");
			str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		}
		OutputString += ParseANSIField(str, 1, 25);

		//NM106	NOT USED

		OutputString += "*";

		//NM107	1039		Name Suffix								O	AN	1/10

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "Title");
		}
		OutputString += ParseANSIField(str, 1, 10);

		//NM108	66			ID Code Qualifier						X	ID	1/2

		//Allowed Values:
		//		24		Employer's ID Number
		//		34		SSN
		//		XX		Health Care Financing National Provider ID

		CString strQual;
		str = "";

		long nNM109_IDType = m_UB92Info.NM109_IDType;

		if(nNM109_IDType == 1) {
			//"24" for employer ID
			strQual = "24";
			if(m_pEBillingInfo->Box78Setup == 3)
				str = GetFieldFromRecordset(rs, "FedEmployerID");
			else
				str = GetFieldFromRecordset(rs, "Fed Employer ID");
		}
		else {
			//"XX" for NPI
			strQual = "XX";
			str = GetFieldFromRecordset(rs, "NPI");
		}

		str = StripNonNum(str);

		if(str != "")
			OutputString += ParseANSIField(strQual, 1, 2);
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

		EndANSISegment(OutputString);

		// (j.jones 2011-10-12 10:30) - PLID 45918 - removed the PRV segment, it does not exist in 4010A1

//340	271		REF		Other Provider Secondary Info.		S		5

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

		//if the qualifier is XX, send nothing
		if(m_bDisableREFXX && strIdentifier.CompareNoCase("XX") == 0) {
			strIdentifier = "";
			strID = "";
			//log that this happened
				Log("Ebilling file tried to export 2310C REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
		}
		
		OutputString += ParseANSIField(strIdentifier,2,3);

		//REF02	127			Reference Identification				X	AN	1/30

		OutputString += ParseANSIField(strID,1,30);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		// do not send this segment if the
		// qualifier or ID are blank
		if(strIdentifier != "" && strID != "") {
			EndANSISegment(OutputString);

			//add this qualifier to our array
			arystrQualifiers.Add(strIdentifier);
		}

		// the UB92 Group now allows the option
		// to append an EIN or NPI as an additional REF segment

		long nExtraREF_IDType = m_UB92Info.ExtraREF_IDType;

		//0 - EIN/SSN, 1 - NPI, 2 - none
		if(nExtraREF_IDType != 2) {

			//export the extra ID

			OutputString = "REF";

			//REF01	128			Reference Identification Qualifier		M	ID	2/3
			
			strIdentifier = "";
			strID = "";

			if(nExtraREF_IDType == 0) {
				//EIN

				//"EI" for employer ID
				strIdentifier = "EI";
				if(m_pEBillingInfo->Box78Setup == 3)
					strID = GetFieldFromRecordset(rs, "FedEmployerID");
				else
					strID = GetFieldFromRecordset(rs, "Fed Employer ID");
			}
			else {
				//NPI

				//"XX" for NPI
				strIdentifier = "XX";
				strID = AdoFldString(rs, "NPI", "");
			}

			strID = StripNonNum(strID);

			OutputString += ParseANSIField(strIdentifier,2,3);

			//REF02	127			Reference Identification				X	AN	1/30

			OutputString += ParseANSIField(strID,1,30);

			//REF03	NOT USED
			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			//if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strIdentifier.CompareNoCase("XX") == 0) {
				strIdentifier = "";
				strID = "";
				//log that this happened
				Log("Ebilling file tried to export 2310C Additional REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
			}

			// disallow sending the additional REF
			// segment if the qualifier has already been used
			if(strIdentifier != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdentifier)) {
				EndANSISegment(OutputString);

				//add this qualifier to our array
				arystrQualifiers.Add(strIdentifier);
			}
		}

		rs->Close();

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2310C_Inst");

	return Error_Other;
}

int CEbilling::ANSI_4010_2310D_Prof() {

	//Service Facility Location

	//this is 2310E on the UB92

	//JJ - This is not used if the Place Of Service is not the same location used
	//in loop 2010AA - Billing Provider. That location should be the location of the bill
	//(make sure it is!). So, this is not used if the POS and the bill location are the same.
	
	//when 2000A is used (always!) this is not used on a UB92

#ifdef _DEBUG

	CString str;

	str = "\r\n2310D\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		//get the location where the service was performed
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT LocationsT.* FROM LocationsT "
			"INNER JOIN BillsT ON LocationsT.ID = BillsT.Location "
			"WHERE BillsT.ID = {INT}",m_pEBillingInfo->BillID);
		if(rs->eof) {
			rs->Close();
			//if end of file, select from LocationsT, and the recordset will just pull from the first entry
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			rs = CreateParamRecordset("SELECT * FROM LocationsT WHERE TypeID = 1");
			if(rs->eof) {
				str = "Error opening location information.";
				rs->Close();
				AfxMessageBox(str);
				return Error_Missing_Info;
			}
		}

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310D - SERVICE FACILITY LOCATION						1

//303		250		NM1		Service Facility Location				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M	ID	2/3

		//static "FA"
		OutputString += ParseANSIField("FA",2,3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//static "2"
		OutputString += ParseANSIField("2",1,1);

		//NM103	1035		Name Last / Org Name					O	AN	1/35

		//LocationsT.Name
		str = GetFieldFromRecordset(rs,"Name");
		OutputString += ParseANSIField(str,1,35);

		//NM104	NOT USED
		OutputString += "*";

		//NM105	NOT USED
		OutputString += "*";

		//NM106	NOT USED
		OutputString += "*";

		//NM107	NOT USED
		OutputString += "*";

		//NM108	66			ID Code Qualifier						X	ID	1/2

		CString strIdent = "", strID = "";

		// (j.jones 2006-11-14 08:55) - PLID 23413 - determine whether the
		// NM108/109 elements should show the NPI (otherwise show nothing)
		long nNM109_IDType = m_HCFAInfo.NM109_IDType;

		if(nNM109_IDType == 0) {
			// (j.jones 2006-11-14 08:58) - PLID 23413 - if 0,
			// then use the NPI

			//"XX" for NPI
			strIdent = "XX";
			strID = GetFieldFromRecordset(rs, "NPI");
		}

		strID = StripNonNum(strID);

		//do not output if the ID is blank
		if(strID != "")
			OutputString += ParseANSIField(strIdent, 1, 2);
		else
			OutputString += "*";

		//NM109	67			ID Code									X	AN	2/80

		//LocationsT.NPI
		if(strID != "")
			OutputString += ParseANSIField(strID, 2, 80);
		else
			OutputString += "*";

		//NM110	NOT USED
		OutputString += "*";

		//NM111	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

//307		265		N3		Service Facility Location Address		R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//N301	166			Address Information						M	AN	1/55

		//LocationsT.Address1
		str = GetFieldFromRecordset(rs, "Address1");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information						O	AN	1/55

		//LocationsT.Address2
		str = GetFieldFromRecordset(rs, "Address2");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

//308		270		N4		Service Facility Loc. City/State/Zip	R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O	AN	2/30

		//LocationsT.City
		str = GetFieldFromRecordset(rs, "City");

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State or Prov Code						O	ID	2/2
		
		//LocationsT.State
		str = GetFieldFromRecordset(rs, "State");

		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O	ID	3/15
		str = GetFieldFromRecordset(rs, "Zip");
		str = StripNonNum(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O	ID	2/3
		OutputString += "*";

		//N405	NOT USED
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

//310		271		REF		Service Facility Loc. Secondary Info.	S		5

		//Ref.	Data		Name									Attributes
		//Des.	Element


		//JJ - the qualifier is 1J for Facility ID for UB92, but this loop isn't used on the UB92
		
		//there is no appropriate qualifier (that I know of) for HCFA claims, but THIN said you just
		//need the qualifier for the given insurance type - such as 1C for Medicare
		//so we will use the qualifier from the 2010AA loop.

		OutputString = "REF";

		//REF01	128			Reference Ident Qual					M	ID	2/3

		// (a.walling 2007-08-16 15:24) - PLID 27094 - This should check the POS, not the Location!
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsFacID = CreateParamRecordset("SELECT FacilityID, Qualifier FROM InsuranceFacilityID WHERE LocationID = {INT} AND InsCoID = {INT}",m_pEBillingInfo->nPOSID,m_pEBillingInfo->InsuranceCoID);
		
		str = "";
		CString strQual = "";
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
			//1A - Blue Cross Provider Number
			//1B - Blue Shield Provider Number
			//1C - Medicare Provider Number
			//1D - Medicaid Provider Number
			//1G - Provider UPIN Number
			//1H - CHAMPUS Identification Number
			//G2 - Provider Commercial Number
			//LU - Location Number
			//N5 - Provider Plan Network Identification Number
			//TJ - Federal Taxpayer’s Identification Number
			//X4 - Clinical Laboratory Improvement Amendment Number
			//X5 - State Industrial Accident Provider Number
			//XX - NPI Number

			//so make sure of it
			if(strQual != "0B" && strQual != "1A" && strQual != "1B" && strQual != "1C" && 
				strQual != "1D" && strQual != "1G" && strQual != "1H" && strQual != "G2" && 
				strQual != "LU" && strQual != "N5" && strQual != "TJ" && strQual != "X4" &&
				strQual != "X5" && strQual != "XX") {
				//we're just guessing now
				strQual = "LU";
			}
		}

		if(str != "" && strQual != "")
			OutputString += ParseANSIField(strQual, 2, 3);

		//REF02	127			Reference Ident							X	AN	1/30
		
		if(str != "" && strQual != "")
			OutputString += ParseANSIField(str, 1, 30);
		
		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		if(str != "" && strQual != "")
			EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

		rs->Close();

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2310D_Prof");

	return Error_Other;
}

// (j.jones 2008-12-11 13:20) - PLID 32401 - supported 2310E
int CEbilling::ANSI_4010_2310E_Prof() {

	//Supervising Provider Name

	// (j.jones 2008-12-11 13:21) - This will send the Supervising Provider from the bill, if one is selected.

#ifdef _DEBUG

	CString str;

	str = "\r\n2310E\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

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

//							LOOP ID - 2310E - SUPERVISING PROVIDER NAME						1

//312		250		NM1		Supervising Provider Name				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M	ID	2/3

		//static "DQ"
		OutputString += ParseANSIField("DQ",2,3);

		//NM102 1065		Entity Type Qualifier					M	ID	1/1
		
		//static "1" for "Person" (organization is not permitted)
		OutputString += ParseANSIField("1",1,1);

		//NM103 1035		Name Last or OrganizationName			O	AN	1/35

		str = GetFieldFromRecordset(rs, "Last");
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 35);

		//NM104 1036		Name First								O	AN	1/25

		str = GetFieldFromRecordset(rs, "First");
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 25);
		
		//NM105 1037		Name Middle								O	AN	1/25

		str = GetFieldFromRecordset(rs, "Middle");
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 25);

		//NM106	NOT USED

		OutputString += "*";
		
		//NM107 1039		Name Suffix								O	AN	1/10

		str = GetFieldFromRecordset(rs, "Title");
		OutputString += ParseANSIField(str, 1, 10);

		//NM108 66			Identification Code Qualifier			X	ID	1/2
		
		//24 - Employer’s Identification Number
		//34 - Social Security Number
		//XX - Health Care Financing Administration National Provider Identifier

		CString strIdent, strID;

		//determine whether the NM108/109 elements should show the NPI or the EIN/SSN
		long nNM109_IDType = m_HCFAInfo.NM109_IDType;

		if(nNM109_IDType == 1) {

			//if 1, then use the fed. employer ID.

			//"24" for employer ID
			strIdent = "24";
			// (j.jones 2010-04-20 17:02) - PLID 38286 - it's [Fed Employer ID], not FedEmployerID
			strID = GetFieldFromRecordset(rs, "Fed Employer ID");
		}
		else {

			//if 0, then use the NPI

			//"XX" for NPI
			strIdent = "XX";
			strID = GetFieldFromRecordset(rs, "NPI");
		}

		strID = StripNonNum(strID);

		//do not output if the ID is blank
		if(strID != "") {
			OutputString += ParseANSIField(strIdent, 1, 2);
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

		EndANSISegment(OutputString);

//316		271		REF		Supervising Prov. Secondary Info.		S		5

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//we need to silently skip adding additional REF segments
		//that have the same qualifier, so track them in an array
		CStringArray arystrQualifiers;

		//REF01	128			Reference Identification Qualifier		M	ID	2/3

		// (j.jones 2010-04-14 10:20) - PLID 38194 - the ID is now calculated in a shared function
		strIdent = "";
		strID = "";
		CString strLoadedFrom = "";
		EBilling_Calculate2310E_REF(strIdent,strID,strLoadedFrom, m_pEBillingInfo->nSupervisingProviderID,m_pEBillingInfo->HCFASetupID,
			m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->InsuredPartyID, m_pEBillingInfo->BillLocation);

		// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
		if(m_bDisableREFXX && strIdent.CompareNoCase("XX") == 0) {
			strIdent = "";
			strID = "";
			//log that this happened
			Log("Ebilling file tried to export 2310E REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
		}
		
		OutputString += ParseANSIField(strIdent,2,3);

		//REF02	127			Reference Identification				X	AN	1/30

		OutputString += ParseANSIField(strID,1,30);

		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		//do not send this segment if the qualifier or ID are blank
		if(strIdent != "" && strID != "") {
			EndANSISegment(OutputString);

			//add this qualifier to our array
			arystrQualifiers.Add(strIdent);
		}

		//the HCFA Group allows the option to append an EIN/SSN, or NPI as an additional REF segment
		long nExtraREF_IDType = m_HCFAInfo.ExtraREF_IDType;

		//0 - EIN/SSN, 1 - NPI, 2 - none
		if(nExtraREF_IDType != 2) {

			//export the extra ID

			OutputString = "REF";

			//REF01	128			Reference Identification Qualifier		M	ID	2/3
			
			strIdent = "";
			strID = "";

			if(nExtraREF_IDType == 0) {
				//if 1, then use the fed. employer ID or SSN, based on Box25

				//"EI" for EIN, "SY" for SSN - from HCFASetupT.Box25
				if(m_HCFAInfo.Box25 == 1) {
					strIdent = "SY";
				}
				else {
					strIdent = "EI";
				}

				if(m_HCFAInfo.Box25 == 1) {
					strID = GetFieldFromRecordset(rs, "SocialSecurity");
				}
				else {
					strID = GetFieldFromRecordset(rs, "Fed Employer ID");
				}
			}
			else {
				//NPI

				//"XX" for NPI
				strIdent = "XX";

				//if there is a 24J override in AdvHCFAPinT, let's use it here, as we would do in 2310B
				_RecordsetPtr rsOver = CreateParamRecordset("SELECT Box24JNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}", m_pEBillingInfo->nSupervisingProviderID, m_pEBillingInfo->BillLocation, m_pEBillingInfo->HCFASetupID);
				if(!rsOver->eof) {
					var = rsOver->Fields->Item["Box24JNPI"]->Value;
					if(var.vt == VT_BSTR) {
						CString strOver = VarString(var,"");
						strOver.TrimLeft();
						strOver.TrimRight();
						if(!strOver.IsEmpty()) {
							strID = strOver;
						}
					}
				}
				rsOver->Close();

				if(strID.IsEmpty()) {
					strID = AdoFldString(rs, "NPI", "");
				}
			}

			strID = StripNonNum(strID);

			OutputString += ParseANSIField(strIdent,2,3);

			//REF02	127			Reference Identification				X	AN	1/30

			OutputString += ParseANSIField(strID,1,30);

			//REF03	NOT USED
			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strIdent.CompareNoCase("XX") == 0) {
				strIdent = "";
				strID = "";
				//log that this happened
				Log("Ebilling file tried to export 2310E Additional REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
			}

			//disallow sending the additional REF segment if the qualifier has already been used
			if(strIdent != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdent)) {
				EndANSISegment(OutputString);

				//add this qualifier to our array (although, really, we're not adding more)
				arystrQualifiers.Add(strIdent);
			}
		}

		rs->Close();
		
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2310E_Prof");

	return Error_Other;
}

int CEbilling::ANSI_4010_2310E_Inst() {

	//Service Facility Name

	//JJ - This is not used if the Place Of Service is not the same location used
	//in loop 2010AA - Billing Provider. That location should be the location of the bill
	//(make sure it is!). So, this is not used if the POS and the bill location are the same.
	
	//when 2000A is used (always!) this is not used on a UB92

#ifdef _DEBUG

	CString str;

	str = "\r\n2310E\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		//get the location where the service was performed
		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT LocationsT.* FROM LocationsT "
			"INNER JOIN BillsT ON LocationsT.ID = BillsT.Location "
			"WHERE BillsT.ID = {INT}",m_pEBillingInfo->BillID);
		if(rs->eof) {
			rs->Close();
			//if end of file, select from LocationsT, and the recordset will just pull from the first entry
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			rs = CreateParamRecordset("SELECT * FROM LocationsT WHERE TypeID = 1");
			if(rs->eof) {
				str = "Error opening location information.";
				rs->Close();
				AfxMessageBox(str);
				return Error_Missing_Info;
			}
		}

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2310E - SERVICE FACILITY NAME					1

//349		250		NM1		Service Facility Name					S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M	ID	2/3

		//static "FA"
		OutputString += ParseANSIField("FA",2,3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//static "2"
		OutputString += ParseANSIField("2",1,1);

		//NM103	1035		Name Last / Org Name					O	AN	1/35

		//LocationsT.Name
		str = GetFieldFromRecordset(rs,"Name");
		OutputString += ParseANSIField(str,1,35);

		//NM104	NOT USED
		OutputString += "*";

		//NM105	NOT USED
		OutputString += "*";

		//NM106	NOT USED
		OutputString += "*";

		//NM107	NOT USED
		OutputString += "*";

		//NM108	66			ID Code Qualifier						X	ID	1/2
		//not used
		OutputString += "*";

		//NM109	67			ID Code									X	AN	2/80
		//not used
		OutputString += "*";

		//NM110	NOT USED
		OutputString += "*";

		//NM111	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

//354		265		N3		Service Facility Location Address		R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//N301	166			Address Information						M	AN	1/55

		//LocationsT.Address1
		str = GetFieldFromRecordset(rs, "Address1");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information						O	AN	1/55

		//LocationsT.Address2
		str = GetFieldFromRecordset(rs, "Address2");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

//355		270		N4		Service Facility Loc. City/State/Zip	R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O	AN	2/30

		//LocationsT.City
		str = GetFieldFromRecordset(rs, "City");

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State or Prov Code						O	ID	2/2
		
		//LocationsT.State
		str = GetFieldFromRecordset(rs, "State");

		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O	ID	3/15
		str = GetFieldFromRecordset(rs, "Zip");
		str = StripNonNum(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O	ID	2/3
		OutputString += "*";

		//N405	NOT USED
		OutputString += "*";

		//N406	NOT USED
		OutputString += "*";

		EndANSISegment(OutputString);

//357		271		REF		Service Facility Loc. Secondary Info.	S		5

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01	128			Reference Ident Qual					M	ID	2/3

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		_RecordsetPtr rsFacID = CreateParamRecordset("SELECT FacilityID, Qualifier FROM InsuranceFacilityID WHERE LocationID = {INT} AND InsCoID = {INT}",m_pEBillingInfo->BillLocation,m_pEBillingInfo->InsuranceCoID);
		
		str = "";
		CString strQual = "";
		if(!rsFacID->eof) {
			str = AdoFldString(rsFacID, "FacilityID","");
			strQual = AdoFldString(rsFacID, "Qualifier","");
			strQual.TrimRight();
		}
		rsFacID->Close();
		
		//if empty, use 1J - Facility ID
		if(strQual.IsEmpty())
			strQual = "1J";

		if(str != "" && strQual != "")
			OutputString += ParseANSIField("1J", 2, 3);

		//REF02	127			Reference Ident							X	AN	1/30
		
		if(str != "" && strQual != "")
			OutputString += ParseANSIField(str, 1, 30);
		
		//REF03	NOT USED
		OutputString += "*";

		//REF04	NOT USED
		OutputString += "*";

		if(str != "")
			EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

		rs->Close();

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2310E_Inst");

	return Error_Other;
}

int CEbilling::ANSI_4010_2320() {

	//Other Subscriber Information

	//JJ - This loop will be used if there is an "other insurance company"
	//selected on the bill.

#ifdef _DEBUG

	CString str;

	str = "\r\n2320\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		rs = CreateParamRecordset("SELECT InsuredPartyT.RespTypeID, InsuredPartyT.RelationToPatient, InsuredPartyT.PolicyGroupNum, "
			"InsurancePlansT.PlanName, InsuranceCoT.InsType, PersonT.BirthDate, PersonT.Gender "
			"FROM InsuredPartyT LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
			"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
			"WHERE InsuredPartyT.PersonID = {INT}", m_pEBillingInfo->OthrInsuredPartyID);

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

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2320 - OTHER SUBSCRIBER INFORMATION					10

//318		290		SBR		Other Subscriber Information			S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "SBR";

		//SBR01	1138		Payer Resp Seq No Code					M	ID	1/1

		//JJ - 5/14/2003 - The description of this code is
		//"Code identifying the insurance carrier's level of 
		//responsibility for a payment of a claim"
		//So it's really not the RespTypeID, it is going to always be secondary because
		//this function represents the insurance company that is secondarily
		//reponsible for this claim.

		str = "S";

		// (j.jones 2006-12-06 11:46) - PLID 23787 - this code has to be primary
		// if we are sending this payer's payments for a non-primary responsibility.
		// You can argue that it should always be "P" when the main payer is non-primary,
		// but I don't trust the system enough to make that change until specifically required, as this is.

		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(!m_pEBillingInfo->IsPrimary && 
			((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1)
			|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1))) {

			str = "P";
		}

		/*
		var = fields->Item["RespTypeID"]->Value;			//InsuredPartyT.RespTypeID

		if(var.vt == VT_I4) {
			long type = long(var.lVal);

			if(type == 1)
				str = "P";
			else if (type == 2)
				str = "S";
			else
				str = "T";
		}
		*/		

		OutputString += ParseANSIField(str, 1, 1);

		//SBR02	1069		Individual Relat Code					O	ID	2/2

		str = GetFieldFromRecordset(rs, "RelationToPatient");

		//The values are on page 319

		if(str == "Child")
			str = "19";
		else if(str == "Self")
			str = "18";
		else if (str == "Spouse")
			str = "01";
		else if (str == "Other")
			str = "G8";
		else if (str == "Employee")
			str = "20";
		else if (str == "Unknown")
			str = "21";
		else if (str == "Organ Donor")
			str = "39";
		else if (str == "Cadaver Donor")
			str = "40";
		else if (str == "Life Partner")
			str = "53";
		else
			str = "21";

		// (j.jones 2011-06-15 17:00) - PLID 40959 - the following are no longer valid entries in our system,
		// and no longer exist in data
		/*
		else if (str == "Grandparent")
			str = "04";
		else if (str == "Grandchild")
			str = "05";
		else if (str == "Nephew Or Niece")
			str = "07";
		else if (str == "Adopted Child")
			str = "09";
		else if (str == "Foster Child")
			str = "10";
		else if (str == "Ward")
			str = "15";
		else if (str == "Stepchild")
			str = "17";
		else if (str == "Handicapped Dependent")
			str = "22";
		else if (str == "Sponsored Dependent")
			str = "23";
		else if (str == "Dependent of a Minor Dependent")
			str = "24";
		else if (str == "Significant Other")
			str = "29";
		else if (str == "Mother")
			str = "32";
		else if (str == "Father")
			str = "33";
		else if (str == "Other Adult")
			str = "34";
		else if (str == "Emancipated Minor")
			str = "36";
		else if (str == "Injured Plaintiff") // (s.dhole 2010-08-31 15:13) - PLID 40114 All our relationship lists say "Injured Plantiff" instead of "Injured Plaintiff"
			str = "41";
		else if (str == "Child Where Insured Has No Financial Responsibility")
			str = "43";
		*/
		
		OutputString += ParseANSIField(str, 2, 2);

		//SBR03	127			Reference Identification				O	AN	1/30

		//InsuredPartyT.PolicyGroupNum
		str = GetFieldFromRecordset(rs,"PolicyGroupNum");
		OutputString += ParseANSIField(str,1,30);

		//SBR04	93			Name									O	AN	1/60

		//InsurancePlansT.PlanName
		str = GetFieldFromRecordset(rs,"PlanName");
		OutputString += ParseANSIField(str,1,60);

		//SBR05	1336		Insurance Type Code						O	ID	1/3

		//This is required, so use the NSF selection to figure it out

		// (j.jones 2008-02-06 17:15) - PLID 28843 - we now have the option for the primary
		// insurance to override this, such that if bSecondaryInsCodeUsage is set,
		// then we send strSecondaryInsCode
		InsuranceTypeCode eInsType = (InsuranceTypeCode)AdoFldLong(rs, "InsType", (long)itcInvalid);
		str = "";
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType != actInst && m_HCFAInfo.bSecondaryInsCodeUsage && !m_HCFAInfo.strSecondaryInsCode.IsEmpty()) {
			str = m_HCFAInfo.strSecondaryInsCode;
		}
		else {

			// (j.jones 2008-09-09 10:10) - PLID 18695 - we finally track proper
			// Insurance Types per company, so simply call GetANSISBR09CodeFromInsuranceType()
			// to get the ANSI code for our type.
			// (j.jones 2009-01-16 12:24) - PLID 32762 - added GetANSI_2320SBR05_CodeFromInsuranceType
			// for this field, because it has a different code list
			str = GetANSI4010_2320SBR05_CodeFromInsuranceType(eInsType);
			if(str.IsEmpty()) {
				str = "OT";
			}
		}

		//not used on the UB92
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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

		//SBR09	1032		Claim File Ind Code						O	ID	1/2

		//InsuranceCoT.InsType
		
		// (j.jones 2008-09-09 10:10) - PLID 18695 - we finally track proper
		// Insurance Types per company, so simply call GetANSISBR09CodeFromInsuranceType()
		// to get the ANSI code for our type.

		//we loaded the InsType earlier in SBR05
		// (j.jones 2010-10-15 14:47) - PLID 40953 - function name changed, content stays the same
		str = GetANSI4010_SBR09CodeFromInsuranceType(eInsType);
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
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		// (d.thompson 2011-09-09) - PLID 45393 - The provider filter must use the claim provider from the charge, which
		//	may differ from the DoctorsProviders value.
		if(m_actClaimType != actInst && m_HCFAInfo.Box33Setup != 2) {
			strDocProv.Format("AND (ChargesT.ClaimProviderID = %li OR (ChargesT.ClaimProviderID IS NULL AND DoctorsProviders IN "
				"(SELECT PersonID FROM "
				"	(SELECT ProvidersT.PersonID, "
				"	(CASE WHEN ANSI_2010AA_ProviderID Is Null THEN ProvidersT.ClaimProviderID ELSE ANSI_2010AA_ProviderID END) AS ProviderIDToUse "
				"	FROM ProvidersT "
				"	LEFT JOIN (SELECT * FROM HCFAClaimProvidersT WHERE InsuranceCoID = %li) AS HCFAClaimProvidersT ON ProvidersT.PersonID = HCFAClaimProvidersT.ProviderID) SubQ "
				"WHERE ProviderIDToUse = %li))) ", m_pEBillingInfo->ProviderID, m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);
		}
		else if(m_actClaimType == actInst && m_UB92Info.Box82Setup == 1) {
			strDocProv.Format("AND (ChargesT.ClaimProviderID = %li OR (ChargesT.ClaimProviderID IS NULL AND DoctorsProviders IN (SELECT PersonID FROM ProvidersT WHERE ClaimProviderID = %li))) ", m_pEBillingInfo->ProviderID, m_pEBillingInfo->ProviderID);
		}

		// (j.jones 2008-02-25 09:53) - PLID 29077 - calculate the allowed amount now,
		// as it may be used as the approved amount or in adjustments

		//cyTotalAllowableToSend will be the value we ultimately send
		COleCurrency cyTotalAllowableToSend = COleCurrency(0,0);
		COleCurrency cyTotalAllowableAdjustmentsToInclude = COleCurrency(0,0);

		//we will send an allowed amount if sending primary payment info on a secondary claim,
		//and we are either sending the allowed as the approved amount or sending an allowed amount
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(!m_pEBillingInfo->IsPrimary && m_HCFAInfo.ANSI_EnablePaymentInfo == 1 &&
			//do we need this as the approved amount?
			((m_actClaimType != actInst && m_HCFAInfo.ANSI_2320Approved == 1 && m_HCFAInfo.ANSI_SecondaryApprovedCalc == 3)
			//do we need this as the allowed amount?
			|| ((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1 && m_HCFAInfo.ANSI_2320Allowed == 1)
			|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1 && m_UB92Info.ANSI_2320Allowed == 1)))) {

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
				m_pEBillingInfo->OthrInsuredPartyID, m_pEBillingInfo->InsuredPartyID,
				m_pEBillingInfo->HCFASetupID, m_pEBillingInfo->OthrInsuredPartyID,
				m_pEBillingInfo->UB92SetupID, m_pEBillingInfo->OthrInsuredPartyID,
				m_pEBillingInfo->BillID);

			//we have to calculate the amount per charge, and total it up, because
			//this has to match the individual charge amounts that aren't calculated until later
			while(!rsTotals->eof) {

				//track our individual amounts
				COleCurrency cyPaymentTotal = AdoFldCurrency(rsTotals, "PaymentTotal", COleCurrency(0,0));
				COleCurrency cyOtherInsTotal = AdoFldCurrency(rsTotals, "OtherInsTotal", COleCurrency(0,0));

				// (j.jones 2007-03-29 10:54) - PLID 25409 - now decide what value we should be sending

				COleCurrency cyPaymentAndOtherIns = cyPaymentTotal + cyOtherInsTotal;

				// (j.jones 2009-08-28 17:43) - PLID 32993 - Goodbye, old code that ECP misled us into calculating!
				// We no longer utilize the fee schedule allowables, period.
				/*
				// (j.jones 2009-03-16 15:07) - PLID 33544 - enforce that we don't add
				// an allowable greater than the charge total
				COleCurrency cyChargeTotal = AdoFldCurrency(rsAllowable, "ChargeTotal", COleCurrency(0,0));
				COleCurrency cyAllowableToAdd = COleCurrency(0,0);

				long nSecondaryAllowableCalc = 3;
				if(m_actClaimType != actInst)
					nSecondaryAllowableCalc = m_HCFAInfo.ANSI_SecondaryAllowableCalc;
				else
					nSecondaryAllowableCalc = m_UB92Info.ANSI_SecondaryAllowableCalc;

				switch(nSecondaryAllowableCalc) {
					case 1:	//only send the Fee Schedule allowable
						cyAllowableToAdd += cyAllowable;
						break;
					case 2: //send the greater of the allowable or the primary paid amount
						if(cyAllowable > cyPaymentTotal)
							cyAllowableToAdd += cyAllowable;
						else
							cyAllowableToAdd += cyPaymentTotal;
						break;
					case 3: //send the greater of the allowable or the primary paid amount + secondary resp.
					default:
						if(cyAllowable > cyPaymentAndOtherIns)
							cyAllowableToAdd += cyAllowable;
						else
							cyAllowableToAdd += cyPaymentAndOtherIns;
						break;
				}

				// (j.jones 2009-03-16 15:05) - PLID 33544 - ensured our allowable cannot be greater than the charge total
				if(cyAllowableToAdd > cyChargeTotal) {
					cyAllowableToAdd = cyChargeTotal;
				}
				*/
				
				// (j.jones 2009-08-28 17:44) - PLID 32993 - we will always send the payment + other insurance here
				cyTotalAllowableToSend += cyPaymentAndOtherIns;

				// (j.jones 2010-02-03 09:32) - PLID 37170 - supported including specified adjustments in the allowed total,
				// these will already have been calculated in the recordset
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType == actInst) {
					cyTotalAllowableAdjustmentsToInclude += AdoFldCurrency(rsTotals, "UB_AllowedAdjustmentTotal", COleCurrency(0,0));
				}
				else {
					cyTotalAllowableAdjustmentsToInclude += AdoFldCurrency(rsTotals, "HCFA_AllowedAdjustmentTotal", COleCurrency(0,0));
				}

				rsTotals->MoveNext();
			}
			rsTotals->Close();
		}

//323		295		CAS		Claim Level Adjustments					S		5

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
			"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
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
			m_pEBillingInfo->BillID, strDocProv, m_pEBillingInfo->OthrInsuredPartyID);
		
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

		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(!m_pEBillingInfo->IsPrimary && 
			((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1 && m_HCFAInfo.ANSI_AdjustmentLoop == 0)
			|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1 && m_UB92Info.ANSI_AdjustmentLoop == 0))) {

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
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 2 "
				"AND PaymentsT.InsuredPartyID = %li "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"ORDER BY AdjustmentGroupCodesT.Code ", _Q(strDefaultInsAdjGroupCode), _Q(strDefaultInsAdjReasonCode),
				m_pEBillingInfo->BillID, strDocProv, m_pEBillingInfo->OthrInsuredPartyID);

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

				COleCurrency cyAllowedAmount = COleCurrency(0,0);
				COleCurrency cyAllowedWithIncludedAdjustments = COleCurrency(0,0);

				//are we going to send an allowed amount?
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(!m_pEBillingInfo->IsPrimary &&
					((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1 && m_HCFAInfo.ANSI_2320Allowed == 1)
					|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1 && m_UB92Info.ANSI_2320Allowed == 1))) {
					
					cyAllowedAmount = cyTotalAllowableToSend;
					// (j.jones 2010-02-03 10:10) - PLID 37170 - the actual allowed amount that will be sent
					// can also include adjustments, but will not be used for the cyPatientAdj calculation
					cyAllowedWithIncludedAdjustments = cyTotalAllowableAdjustmentsToInclude;
				}

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
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				BOOL bAlwaysSendPRSegment = ((m_actClaimType != actInst && m_HCFAInfo.bANSI_SendCASPR) || (m_actClaimType == actInst && m_UB92Info.bANSI_SendCASPR));

				COleCurrency cyRegularAdjustmentAmountToSend = cyNonPatientAdj;
				COleCurrency cyPRAdjustmentAmountToSend = cyPatientAdj;

				// (j.jones 2009-08-28 16:46) - PLID 35006 - always send PR, or at least try to
				// (PR is not sent if a UB claim)
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
					// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
					// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
			ANSI_4010_OutputCASSegments(aryCASSegments);
		}

//////////////////////////////////////////////////////////////////////////////

//332		300		AMT		COB Payer Paid Amount					S		1

		// (j.jones 2006-11-24 15:16) - PLID 23415 - Use the payment information
		// if not sending to primary, and is using secondary sending.
		// When using the secondary stuff, it is REQUIRED to send payments here.

		// (j.jones 2006-11-27 17:07) - PLID 23652 - supported this on the UB92
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum

		if(!m_pEBillingInfo->IsPrimary &&
			((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1)
			|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1))) {

			// (j.jones 2006-11-21 11:21) - PLID 23415 - this is different between
			// HCFA and UB92, albeit only slightly so, both versions show the
			// total paid by this payer

			//we already have cyPaid

			// (j.jones 2007-07-09 08:51) - PLID 26572 - changed to send at all times,
			// even with a $0.00 primary paid amount
			//if(cyPaid > COleCurrency(0,0)) {

				OutputString = "AMT";

				//HCFA

				//Ref.	Data		Name									Attributes
				//Des.	Element

				//AMT01	522			Amount Qualifier Code					M	ID	1/3

				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType != actInst) {

					//HCFA
				
					//D - Payor Amount Paid

					str = "D";
				}
				else {

					//UB92
				
					//C4 - Prior Payment - Actual

					str = "C4";
				}

				OutputString += ParseANSIField(str, 1, 3);					

				//AMT02	782			Monetary Amount							M	R	1/18

				str = FormatCurrencyForInterface(cyPaid, FALSE, FALSE);
				//see if we need to trim the right zeros
				// (j.jones 2008-05-06 11:42) - PLID 29937 - cached m_bTruncateCurrency
				if(m_bTruncateCurrency) {
					str.TrimRight("0");	//first the zeros
					str.TrimRight(".");	//then the decimal, if necessary
				}
				OutputString += ParseANSIField(str, 1, 18);

				//AMT03	NOT USED

				EndANSISegment(OutputString);
			//}
		}

		//the approved amount is HCFA-only
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType != actInst) {
//////////////////////////////////////////////////////////////////////////////

//333		300		AMT		COB Approved Amount						S		1

			// (j.jones 2006-11-24 15:17) - PLID 23415 - use the approved information
			// if not sending to primary, and is using secondary sending,
			// and enabled sending aproved information
			
			if(!m_pEBillingInfo->IsPrimary && m_HCFAInfo.ANSI_EnablePaymentInfo == 1
				&& m_HCFAInfo.ANSI_2320Approved == 1) {

				//Ref.	Data		Name									Attributes
				//Des.	Element

				//officially the description of this is:
				//"The approved amount equals the amount for the total claim that was
				//approved by the payer sending this 837 to another payer."
				
				OutputString = "AMT";

				//AMT01 522			Amount Qualifier Code					M	ID	1/3

				//AAE - Approved Amount
				str = "AAE";
				OutputString += ParseANSIField(str, 1, 3);			

				//AMT02 782			Monetary Amount							M	R	1/18

				// (j.jones 2008-02-25 09:58) - PLID 29077 - we now have settings for how this
				// field should be calculated
				COleCurrency cyApprovedAmount = COleCurrency(0,0);
				
				//we already have these calculated values to use, so just
				//decide which value to send
				if(m_HCFAInfo.ANSI_SecondaryApprovedCalc == 1) {
					//send the balance of the claim total minus the primary amount paid					
					cyApprovedAmount = cyClaimTotal - cyPaid;
				}
				else if(m_HCFAInfo.ANSI_SecondaryApprovedCalc == 2) {
					//send the claim total
					cyApprovedAmount = cyClaimTotal;
				}
				else {
					//send the allowed amount

					//Note: to our knowledge, this is the most likely value to send at all times
					// (j.jones 2010-02-03 10:11) - PLID 37170 - we will also include cyTotalAllowableAdjustmentsToInclude,
					// which may include adjustment totals
					cyApprovedAmount = cyTotalAllowableToSend + cyTotalAllowableAdjustmentsToInclude;
				}

				str = FormatCurrencyForInterface(cyApprovedAmount, FALSE, FALSE);
				//see if we need to trim the right zeros
				// (j.jones 2008-05-06 11:42) - PLID 29937 - cached m_bTruncateCurrency
				if(m_bTruncateCurrency) {
					str.TrimRight("0");	//first the zeros
					str.TrimRight(".");	//then the decimal, if necessary
				}
				OutputString += ParseANSIField(str, 1, 18);

				//AMT03 NOT USED

				EndANSISegment(OutputString);

			}

		}

//334		300		AMT		COB Allowed Amount						S		1

		// (j.jones 2006-11-24 15:17) - PLID 23415 - use the allowed information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending contract information

		// (j.jones 2006-11-27 17:10) - PLID 23652 - supported this on the UB92
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum

		if(!m_pEBillingInfo->IsPrimary &&
			((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1 && m_HCFAInfo.ANSI_2320Allowed == 1)
			|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1 && m_UB92Info.ANSI_2320Allowed == 1))) {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			//officially the description of this is:
			//"The allowed amount equals the amount for the total claim that was
			//allowed by the payer sending this 837 to another payer."
			
			OutputString = "AMT";

			//AMT01 522			Amount Qualifier Code					M	ID	1/3

			//B6 - Allowed - Actual
			str = "B6";
			OutputString += ParseANSIField(str, 1, 3);			

			//AMT02 782			Monetary Amount							M	R	1/18

			//cyTotalAllowableToSend was calculated earlier in this function
			// (j.jones 2010-02-03 10:11) - PLID 37170 - we will also include cyTotalAllowableAdjustmentsToInclude,
			// which may include adjustment totals
			str = FormatCurrencyForInterface(cyTotalAllowableToSend + cyTotalAllowableAdjustmentsToInclude, FALSE, FALSE);
			//see if we need to trim the right zeros
			// (j.jones 2008-05-06 11:42) - PLID 29937 - cached m_bTruncateCurrency
			if(m_bTruncateCurrency) {
				str.TrimRight("0");	//first the zeros
				str.TrimRight(".");	//then the decimal, if necessary
			}
			OutputString += ParseANSIField(str, 1, 18);

			//AMT03 NOT USED

			EndANSISegment(OutputString);
		}

//335		300		AMT		COB Patient Responsibility Amount		S		1
//336		300		AMT		COB Covered Amount						S		1
//337		300		AMT		COB Discount Amount						S		1
//338		300		AMT		COB Per Day Limit Amount				S		1
//339		300		AMT		COB Patient Paid Amount					S		1
//340		300		AMT		COB (Coord. Of Benefits) Tax Amount		S		1
//341		300		AMT		COB Total Claim Before Taxes Amount		S		1

		//not used

		//note: these records are slighly different for the UB92,
		//but we still don't use them

//////////////////////////////////////////////////////////////////////////////

//342		305		DMG		Subscriber Demographic Information		S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DMG";

		//get birthdate first
		COleDateTime dt;
		var = rs->Fields->Item["BirthDate"]->Value;
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

		OutputString += ParseANSIField(str, 1, 35);

		//DMG03	1068		Gender Code								O	ID	1/1

		var = rs->Fields->Item["Gender"]->Value;
		long gender = long(VarByte(var,0));
		if(gender == 1)
			str = "M";
		else if (gender == 2)
			str = "F";
		else
			str = "";

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

		EndANSISegment(OutputString);

//344		310		OI		Other Insurance Coverage Information	R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "OI";

		//OI01	NOT USED

		OutputString += "*";

		//OI02	NOT USED

		OutputString += "*";

		//OI03	1073		Yes/No Cond Resp Code					O	ID	1/1

		//InsuranceAcceptedT.Accepted
		BOOL bAccepted = TRUE;
		str = "N";
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType != actInst || m_pEBillingInfo->Box82Setup != 3) {
			// (j.jones 2010-07-23 15:02) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsCo now
			bAccepted = GetAcceptAssignment_ByInsCo(m_pEBillingInfo->InsuranceCoID, m_pEBillingInfo->ProviderID);
		}
		// (j.jones 2007-05-10 14:46) - PLID 25948 - if referring physician, we don't know, so accept if any of our providers accept
		else if(m_actClaimType == actInst && m_pEBillingInfo->Box82Setup == 3) {
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
			str = "Y";

		OutputString += ParseANSIField(str,1,1);

		//OI04	1351		Patient Sig Source Code					O	ID	1/1

		//not used on the UB92
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum

		if(m_actClaimType != actInst)
			//static "C" - signature on file
			OutputString += ParseANSIField("C",1,1);
		else
			OutputString += "*";

		//OI05	NOT USED

		OutputString += "*";

		//OI06	1363		Release of Info Code					O	ID	1/1

		//static "Y"
		OutputString += ParseANSIField("Y",1,1);

		EndANSISegment(OutputString);

//347		320		MOA		Medicare Outpatient Adjucation Info.	S		1

		//not used

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2320");

	return Error_Other;
}

int CEbilling::ANSI_4010_2330A() {

	//Other Subscriber Name

#ifdef _DEBUG

	CString str;

	str = "\r\n2330A\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		_RecordsetPtr rs;

		// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
		rs = CreateParamRecordset("SELECT PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, PersonT.Address1, PersonT.Address2, "
					"PersonT.City, PersonT.State, PersonT.Zip, InsuredPartyT.IDForInsurance, InsuredPartyT.PolicyGroupNum, PersonT.SocialSecurity "
					"FROM InsuredPartyT INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
					"WHERE InsuredPartyT.PersonID = {INT}", m_pEBillingInfo->OthrInsuredPartyID);

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

//350		325		NM1		Other Subscriber Name					R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M	ID	2/3

		//static value "IL"

		OutputString += ParseANSIField("IL", 2, 3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//static value "1" - person, "2" - non person

		OutputString += ParseANSIField("1", 1, 1);

		//NM103	1035		Name Last / Org Name					O	AN	1/35

		str = GetFieldFromRecordset(rs, "Last");		//PersonT.Last

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 35);

		//NM104	1036		Name First								O	AN	1/25

		str = GetFieldFromRecordset(rs, "First");		//PersonT.First

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 25);

		//NM105	1037		Name Middle								O	AN	1/25

		str = GetFieldFromRecordset(rs, "Middle");

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str, 1, 25);

		//NM106	NOT USED

		OutputString += "*";

		//NM107	1039		Name Suffix								O	AN	1/10

		// (j.jones 2007-08-03 11:44) - PLID 26934 - changed from Suffix to Title
		str = GetFieldFromRecordset(rs, "Title");

		OutputString += ParseANSIField(str, 1, 10);

		//NM108	66			ID Code Qualifier						X	ID	1/2

		//static "MI"
		str = GetFieldFromRecordset(rs,"IDForInsurance");
		str = StripPunctuation(str);
		//do not output if the ID is blank
		if(str != "")
			OutputString += ParseANSIField("MI",1,2);
		else
			OutputString += "*";

		//NM109	67			ID Code									X	AN	2/80

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

		EndANSISegment(OutputString);

//354		332		N3		Other Subscriber Address				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//N301	166			Address Information						M	AN	1/55

		str = GetFieldFromRecordset(rs, "Address1");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information						O	AN	1/55

		str = GetFieldFromRecordset(rs, "Address2");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

//355		340		N4		Other Subscriber City/State/Zip			S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O	AN	2/30

		str = GetFieldFromRecordset(rs, "City");

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State or Province						O	ID	2/2

		str = GetFieldFromRecordset(rs, "State");

		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O	ID	3/15

		str = GetFieldFromRecordset(rs, "Zip");
		str = StripNonNum(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O	ID	2/3

		//not used by us

		OutputString += "*";

		//N405	NOT USED

		OutputString += "*";

		//N406	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

//357		355		REF		Other Subscriber Secondary Info.		S		3

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01	128			Reference Ident Qual					M	ID	2/3

		// (j.jones 2008-04-01 16:42) - PLID 29486 - added ability to suppress this field
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if((m_actClaimType == actInst && m_UB92Info.ANSI_Hide2330AREF == 0)
			|| (m_actClaimType != actInst && m_HCFAInfo.ANSI_Hide2330AREF == 0)) {

			//"IG" - Policy Group Number
			//"SY" - SSN
			CString strQual, strNum;
			// (j.jones 2008-05-06 11:39) - PLID 29937 - cached m_bUseSSN
			if(m_bUseSSN) {
				strQual = "SY";
			}
			else {
				strQual = "IG";
			}

			OutputString += ParseANSIField(strQual, 2, 3);

			//REF02	127			Reference Ident							X	AN	1/30

			//InsuredPartyT.PolicyGroupNum

			// (j.jones 2008-05-06 11:39) - PLID 29937 - cached m_bUseSSN
			if(m_bUseSSN) {
				strNum = GetFieldFromRecordset(rs,"SocialSecurity");
			}
			else {
				strNum = GetFieldFromRecordset(rs,"PolicyGroupNum");
			}

			strNum.TrimRight();

			OutputString += ParseANSIField(strNum, 1, 30);

			//REF03	NOT USED

			OutputString += "*";

			//REF04	NOT USED

			OutputString += "*";

			//if these is no number, don't output
			if(strNum != "") {
				EndANSISegment(OutputString);
			}
		}

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2330A");
///////////////////////////////////////////////////////////////////////////////

	return Error_Other;
}


int CEbilling::ANSI_4010_2330B() {

	//Other Payer Name

#ifdef _DEBUG

	CString str;

	str = "\r\n2330B\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

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
			"WHERE InsuredPartyT.PersonID = {INT}", m_pEBillingInfo->OthrInsuredPartyID);

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

//359		325		NM1		Other Payer Name						R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity ID Code							M	ID	2/3

		//static value "PR"

		OutputString += ParseANSIField("PR",2,3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

		//static value "2"

		OutputString += ParseANSIField("2",1,1);

		//NM103	1035		Name Last / Org Name					O	AN	1/35

		str = GetFieldFromRecordset(rs, "Name");			//InsuranceCoT.Name

		OutputString += ParseANSIField(str, 1, 35);

		//NM104	NOT USED

		OutputString += "*";

		//NM105	NOT USED

		OutputString += "*";

		//NM106	NOT USED

		OutputString += "*";

		//NM107	NOT USED

		OutputString += "*";

		//NM108	66			ID Code Qualifier						X	ID	1/2

		// (j.jones 2007-09-18 17:49) - PLID 27428 - if CrossoverCode is non-empty, send the CrossoverCode,
		// otherwise send the payer ID
		
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
			// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
			// (j.jones 2012-03-21 15:23) - PLID 48870 - moved secondary info to a pointer
			if(m_actClaimType == actInst) {
				str = m_pEBillingInfo->pOtherInsuranceInfo->strUBPayerID;
			}
			else {
				str = m_pEBillingInfo->pOtherInsuranceInfo->strHCFAPayerID;
			}

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
			CString strTPLCode = m_pEBillingInfo->pOtherInsuranceInfo->strTPLCode;
			strTPLCode.TrimLeft();
			strTPLCode.TrimRight();
			if(!strTPLCode.IsEmpty()) {
				str = strTPLCode;
			}
		}

		//static "PI"
		//if no ID, don't output
		if(str != "")
			OutputString += ParseANSIField("PI",1,2);
		else
			OutputString += "*";

		//NM109	67			ID Code									X	AN	2/80

		//Either the payer ID or InsuranceCoT.CrossoverCode
		if(str != "")
			OutputString += ParseANSIField(str,2,80);
		else
			OutputString += "*";

		//NM110	NOT USED

		OutputString += "*";

		//NM111	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

		// (j.jones 2008-02-19 11:42) - PLID 29004 - added option to always
		// send the 2330B address for HCFA claims (it always was sent anyways
		// for UB claims)
		if(m_actClaimType == actInst || m_bSend2330BAddress) {

			//only used if an Institutional Claim

			//...except now Trizetto told a client they had to send it
			//for HCFA claims, even though the address doesn't exist at all
			//in the ANSI specs for 2330B for Professional forms

//412		332		N3		Other Subscriber Address				S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N3";

		//N301	166			Address Information						M	AN	1/55

		str = GetFieldFromRecordset(rs, "Address1");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		//N302	166			Address Information						O	AN	1/55

		str = GetFieldFromRecordset(rs, "Address2");

		str = StripPunctuation(str);
		OutputString += ParseANSIField(str, 1, 55);

		EndANSISegment(OutputString);

//355		340		N4		Other Subscriber City/State/Zip			S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "N4";

		//N401	19			City Name								O	AN	2/30

		str = GetFieldFromRecordset(rs, "City");

		OutputString += ParseANSIField(str, 2, 30);

		//N402	156			State or Province						O	ID	2/2

		str = GetFieldFromRecordset(rs, "State");

		OutputString += ParseANSIField(str, 2, 2);

		//N403	116			Postal Code								O	ID	3/15

		str = GetFieldFromRecordset(rs, "Zip");
		str = StripNonNum(str);
		OutputString += ParseANSIField(str, 3, 15);

		//N404	26			Country Code							O	ID	2/3

		//not used by us

		OutputString += "*";

		//N405	NOT USED

		OutputString += "*";

		//N406	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);

		}

		//not used if an Institutional claim or based on ANSI property setting
		// (j.jones 2008-05-06 11:45) - PLID 29937 - cached m_bExport2330BPER
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType != actInst && m_bExport2330BPER) {

//363		345		PER		Other Payer Contact Information			S		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "PER";

		//PER01	366			Contact Function Code					M	ID	2/2

		//static value "IC"

		OutputString += ParseANSIField("IC", 2, 2);

		//PER02	93			Name									O	AN	1/60

		str = GetFieldFromRecordset(rs, "ContactName");

		OutputString += ParseANSIField(str, 1, 60);

		//what we output next depends if the Phone2 and/or Fax was filled in
		BOOL bWorkPhone = FALSE;
		BOOL bFax = FALSE;
		CString strWorkPhone, strFax;
		strWorkPhone = GetFieldFromRecordset(rs,"WorkPhone");
		strWorkPhone = StripNonNum(strWorkPhone);
		if(strWorkPhone != "")
			bWorkPhone = TRUE;
		strFax = GetFieldFromRecordset(rs,"Fax");
		strFax = StripNonNum(strFax);
		if(strFax != "")
			bFax = TRUE;

		//PER03	365			Communication Number Qualifier			X	ID	2/2		

		//static "TE" or "FX"
		if(bWorkPhone)
			OutputString += ParseANSIField("TE",2,2);
		else if(!bWorkPhone && bFax)
			OutputString += ParseANSIField("FX",2,2);
		else
			OutputString += "*";

		//PER04	364			Communication Number					X	AN	1/80

		//PersonT.WorkPhone
		if(bWorkPhone)
			OutputString += ParseANSIField(strWorkPhone,1,80);
		else if(!bWorkPhone && bFax)
			OutputString += ParseANSIField(strFax,1,80);
		else
			OutputString += "*";

		//PER05	365			Communication Number Qualifier			X	ID	2/2

		//if PersonT.WorkPhone not empty and PersonT.Fax not empty
		//static "FX"
		if(bWorkPhone && bFax)
			OutputString += ParseANSIField("FX",2,2);
		else
			OutputString += "*";

		//PER06	364			Communication Number					X	AN	1/80

		//if PersonT.Fax not empty
		//PersonT.Fax
		if(bWorkPhone && bFax)
			OutputString += ParseANSIField(strFax,1,80);
		else
			OutputString += "*";

		//PER07	365			Comm Number Qual						X	ID	2/2

		//unused

		OutputString += "*";

		//PER08	364			Comm Number								X	AN	1/80

		//unused

		OutputString += "*";

		//PER09	NOT USED

		OutputString += "*";

		// (j.jones 2008-09-02 13:45) - PLID 31114 - if the segment is still only PER*IC, then
		// don't bother exporting anything
		CString strTest = OutputString;
		strTest.TrimRight("*");
		if(strTest != "PER*IC") {
			EndANSISegment(OutputString);
		}

		}

//366		345		DTP		Claim Adjudication Date					S		1
		
		// (j.jones 2006-11-27 10:27) - PLID 23415 - use the date information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending adjustment information in the 2320 loop

		// (j.jones 2006-11-27 17:17) - PLID 23652 - supported this on the UB92 
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
				"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"	WHERE BillID = {INT} AND ChargesT.Batched = 1 AND LineItemT.Deleted = 0 "
				"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null)) "
				"AND PaymentsT.InsuredPartyID <> -1 "
				"AND PaymentsT.InsuredPartyID = {INT} "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"GROUP BY LineItemT.Date " //unique dates only
				"ORDER BY LineItemT.Date ",	
				m_pEBillingInfo->BillID, m_pEBillingInfo->OthrInsuredPartyID);

			if(!rsDates->eof) {

				//Ref.	Data		Name									Attributes
				//Des.	Element

				OutputString = "DTP";

				//DTP01	374			Date/Time Qualifier						M	ID	3/3

				//573 - Date Claim Paid

				str = "573";
				OutputString += ParseANSIField(str, 3, 3);

				//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

				//D8 - date is CCYYMMDD

				str = "D8";
				OutputString += ParseANSIField(str, 2, 3);

				//DTP03	1251		Date Time Period						M	AN	1/35

				//LineItemT.Date

				COleDateTime dt = AdoFldDateTime(rsDates, "Date");
				str = dt.Format("%Y%m%d");
				OutputString += ParseANSIField(str, 1, 35);

				EndANSISegment(OutputString);
			}
			rsDates->Close();
		}

//368		355		REF		Other Payer Secondary Identifier		S		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		//REF01	128			Reference Ident Qual					M	ID	2/3

		//static "FY"
		OutputString += ParseANSIField("FY",2,3);

		//REF02	127			Reference Ident							X	AN	1/30

		//InsuranceCoT.EBillingClaimOffice
		str = GetFieldFromRecordset(rs,"EBillingClaimOffice");
		OutputString += ParseANSIField(str,1,30);

		//REF03	NOT USED

		OutputString += "*";

		//REF04	NOT USED

		OutputString += "*";

		if(str != "")
			EndANSISegment(OutputString);

//370		355		REF		Other Payer Prior Auth. or Referral Num	S		2

		//Ref.	Data		Name									Attributes
		//Des.	Element

		//we do not store a prior auth. number for a secondary payer,
		//so we don't need to export this segment

		/*
		OutputString = "REF";

		//REF01	128			Reference Ident Qual					M	ID	2/3

		//REF02	127			Reference Ident							X	AN	1/30

		//REF03	NOT USED

		OutputString += "*";

		//REF04	NOT USED

		OutputString += "*";

		EndANSISegment(OutputString);
		*/

//372		355		REF		Other Payer Claim Adjustment Indicator	S		2

		//not used

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2330B");

	return Error_Other;
}

/*	All this other payer stuff is ridiculous. We will not use this.
Each loop is only used if the patient/provider/location etc. is different
for the different payer.

int CEbilling::ANSI_4010_2330C() {

	//Other Payer Patient Information

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2330C - OTHER PAYER PATIENT INFORMATION				1

//374		325		NM1		Other Payer Patient Information			S		1
//376		355		REF		Other Payer Patient Identification		S		3

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2330C");

	return Error_Other;
}

int CEbilling::ANSI_4010_2330D() {

	//Other Payer Referring Provider

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2330D - OTHER PAYER REFERRING PROVIDER				2

//378		325		NM1		Other Payer Referring Provider			S		1
//380		355		REF		Other Payer Referring Provider Ident.	R		3

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2330D");

	return Error_Other;
}

int CEbilling::ANSI_4010_2330E() {

	//Other Payer Rendering Provider

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2330E - OTHER PAYER RENDERING PROVIDER				1

//382		325		NM1		Other Payer Rendering Provider			S		1
//384		355		REF		Othr Payer Rend. Prov. Secondary Ident.	R		3

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2330E");

	return Error_Other;
}

int CEbilling::ANSI_4010_2330F() {

	//Other Payer Purchased Service Provider

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2330F - OTHER PAYER PURCHASED SERVICE PROVIDER		1

//386		325		NM1		Other Payer Purchased Service Provider	S		1
//388		355		REF		Other Payer Purch. Service Prov. Ident.	R		3

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2330F");

	return Error_Other;
}

int CEbilling::ANSI_4010_2330G() {

	//Other Payer Service Facility Location

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2330G - OTHER PAYER SERVICE FACILITY LOCATION			1

//390		325		NM1		Other Payer Service Facility Location	S		1
//392		355		REF		Other Payer Serv. Facility Loc. Ident.	R		3

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2330G");

	return Error_Other;
}

int CEbilling::ANSI_4010_2330H() {

	//Other Payer Supervising Provider

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2330H - OTHER PAYER SUPERVISING PROVIDER				1

//394		325		NM1		Other Payer Supervising Provider		S		1
//396		355		REF		Other Payer Supervising Prov. Ident.	R		3

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2330H");

	return Error_Other;
}

*/

// (j.jones 2008-05-23 10:48) - PLID 29084 - added parameters for allowables
// (j.jones 2011-04-20 10:45) - PLID 41490 - we now pass in info. on skipping diagnosis codes
int CEbilling::ANSI_4010_2400_Prof(ADODB::_RecordsetPtr &rsCharges, BOOL &bSentAllowedAmount, COleCurrency &cyAllowableSent)
{

	//Service Line

#ifdef _DEBUG

	CString str;

	str = "\r\n2400\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	//increment the count for the current service line
	m_ANSIServiceCount++;

	FieldsPtr fields = rsCharges->Fields;

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2400 - SERVICE LINE									50

//398		365		LX		Service Line							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "LX";

		//LX01	554			Assigned Number							M	N0	1/6

		//m_ANSIServiceCount
		str.Format("%li",m_ANSIServiceCount);
		OutputString += ParseANSIField(str,1,6);

		EndANSISegment(OutputString);

//400		370		SV1		Professional Service					R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "SV1";

		//SV101	C003		Composite Medical Procedure Identifier	M

		//SV101-1	235		Product/Service ID Qualifier			M	ID	2/2

		//static "HC"
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
		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//SV101-4	1339	Procedure Modifier						O	AN	2/2
		
		//ChargesT.CPTModifier2
		str = GetFieldFromRecordset(rsCharges,"CPTModifier2");
		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//SV101-5	1339	Procedure Modifier						O	AN	2/2

		//ChargesT.CPTModifier3
		str = GetFieldFromRecordset(rsCharges,"CPTModifier3");
		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//SV101-6	1339	Procedure Modifier						O	AN	2/2
			
		//ChargesT.CPTModifier4
		str = GetFieldFromRecordset(rsCharges,"CPTModifier4");
		if(str != "") {
			OutputString += ":";
			OutputString += str;
		}

		//SV101-7	NOT USED

		//SV102	782			Monetary Amount							O	R	1/18

		//LineTotal is the calculated amount * qty * tax * modifiers
		COleCurrency cyLineTotal = VarCurrency(fields->Item["LineTotal"]->Value, COleCurrency(0,0));
		str = FormatCurrencyForInterface(cyLineTotal,FALSE,FALSE);

		//see if we need to trim the right zeros
		// (j.jones 2008-05-06 11:42) - PLID 29937 - cached m_bTruncateCurrency
		if(m_bTruncateCurrency) {
			str.TrimRight("0");	//first the zeros
			str.TrimRight(".");	//then the decimal, if necessary
		}

		str.Replace(GetCurrencySymbol(),"");
		str.Replace(GetThousandsSeparator(),"");

		OutputString += ParseANSIField(str,1,18);

		//SV103	355			Unit or Basis for Measurement Code		X	ID	2/2

		//UN - Unit
		//MJ - Minutes
		//check ServiceT.Anesthesia. If it is true, then the quantity is minutes, not units
		str = "UN";
		if(VarBool(fields->Item["Anesthesia"]->Value, FALSE))
			str = "MJ";
		OutputString += ParseANSIField(str,2,2);

		//SV104	380			Quantity								X	R	1/15

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

		//SV105	1331		Facility Code							O	AN	1/2
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

		//SV107	C004		Composite Diagnosis Code Pointer		O

		//SV107-1	1328	Diagnosis Code Pointer					M	N0	1/2
		//SV107-1	1328	Diagnosis Code Pointer					O	N0	1/2
		//SV107-1	1328	Diagnosis Code Pointer					O	N0	1/2
		//SV107-1	1328	Diagnosis Code Pointer					O	N0	1/2

		//ChargesT.WhichCodes
		//the format in which we store "WhichCodes" will fill in all 4
		//fields appropriately, once the commas are replaced with colons
		
		// (j.jones 2009-03-25 17:35) - PLID 33669 - this should not need to change to support
		// up to 8 codes, because the bill saving should not have allowed more than 4 pointers
		// (j.gruber 2014-03-17 10:56) - PLID 61394 - update for new billing structure
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

//406	//SV109	1073		Yes/No Condition or Response Code		O	ID	1/1

		// (j.jones 2008-11-13 15:22) - PLID 31980 - We need to support the Box24C value here.
		//Per the ANSI specs:
		//SV109 is the emergency-related indicator; a “Y” value indicates service
		//provided was emergency related; an “N” value indicates service provided was not
		//emergency related.

		//since this is optional, and the 24C default is blank, we will send nothing
		//unless the 24C setting is used, in which case we send Y or N
		str = "";

		// (j.jones 2010-04-08 13:46) - PLID 38095 - ChargesT.IsEmergency determines whether
		// we send the setting on this charge, or use the HCFA Setup value
		ChargeIsEmergencyType eIsEmergency = (ChargeIsEmergencyType)VarLong(fields->Item["IsEmergency"]->Value);
		if(eIsEmergency == cietNo || (eIsEmergency == cietUseDefault && m_HCFAInfo.Box24C == 2)) {
			str = "N";
		}
		else if(eIsEmergency == cietYes || (eIsEmergency == cietUseDefault && m_HCFAInfo.Box24C == 1)) {
			str = "Y";
		}
		//whether eIsEmergency is cietBlank, or m_HCFAInfo.Box24C is not 1 or 2, send nothing
		else {
			str = "";
		}
		OutputString += ParseANSIField(str,1,1);

		//SV110	NOT USED
		OutputString += "*";

		//SV111	1073		Yes/No Condition or Response Code		O	ID	1/1
				//this is for EPSDT, which we don't use
		OutputString += "*";

		
		//SV112	1073		Yes/No Condition or Response Code		O	ID	1/1
				//this is for family planning, which we don't use
		OutputString += "*";

		//SV113	NOT USED
		OutputString += "*";
		//SV114	NOT USED
		OutputString += "*";

		//SV115	1327		Copay Status Code						O	ID	1/1
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

//408		385		SV4		Prescription Number						S		1
//410		420		PWK		DMERC CMN Indicator						S		1
//412		425		CR1		Ambulance Transport Information			S		1
//415		430		CR2		Spinal Manipulation Service Information	S		5
//421		435		CR3		Durable Medical Equipment Certification	S		1
//423		445		CR5		Home Oxygen Therapy Information			S		1
//427		450		CRC		Ambulance Certification					S		3
//430		450		CRC		Hospice Employee Indicator				S		1
//432		450		CRC		DMERC Condition Indicator				S		2

		//not used

//435		455		DTP		Date - Service Date						R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M	ID	3/3
		
		//static "472"
		OutputString += ParseANSIField("472",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

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

		//DTP03	1251		Date Time Period						M	AN	1/35

		OutputString += ParseANSIField(strDate,1,35);

		EndANSISegment(OutputString);

//437		455		DTP		Date - Certification Revision Date		S		1
//439		455		DTP		Date - Referral Date					S		1
//440		455		DTP		Date - Begin Therapy Date				S		1
//442		455		DTP		Date - Last Certification Date			S		1
//444		455		DTP		Date - Order Date						S		1
//445		455		DTP		Date - Date Last Seen					S		1
//447		455		DTP		Date - Test								S		2
//449		455		DTP		Date - Oxygen Saturation / Blood Test	S		3
//451		455		DTP		Date - Shipped							S		1
//452		455		DTP		Date - Onset of Current Symptom/Illness	S		1
//454		455		DTP		Date - Last X-Ray						S		1
//456		455		DTP		Date - Acute Manifestation				S		1
//458		455		DTP		Date - Initial Treatment				S		1
//460		455		DTP		Date - Similar Illness/Symptom Onset	S		1
//462		460		QTY		Anesthesia Modifying Units				S		5

//464		462		MEA		Test Result								S		20

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

			//MEA01 737		Measurement Reference ID Code				O	ID	2/2

			//OG - Original Starting dosage
			//TR - Test Results

			CString strID = AdoFldString(rsCharges, "TestResultID", "");
			strID.TrimLeft();
			strID.TrimRight();

			OutputString += ParseANSIField(strID,2,2);

			//MEA02 738		Measurement Qualifier						O	ID	1/3

			//CON - Concentration
			//GRA - Gas Test Rate
			//HT - Height
			//R1 - Hemoglobin
			//R2 - Hematocrit
			//R3 - Epoetin Starting Dosage
			//R4 - Creatin
			//ZO - Oxygen

			CString strType = AdoFldString(rsCharges, "TestResultType", "");
			strType.TrimLeft();
			strType.TrimRight();

			OutputString += ParseANSIField(strType,1,3);

			//MEA03 739		Measurement Value							X	R	1/20

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

//466		465		CN1		Contract Information					S		1

		// (j.jones 2006-11-24 15:20) - PLID 23415 - use the contract information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending contract information
		if(!m_pEBillingInfo->IsPrimary && m_HCFAInfo.ANSI_EnablePaymentInfo == 1
			&& m_HCFAInfo.ANSI_2400Contract == 1) {

			OutputString = "CN1";

			//Ref.	Data		Name									Attributes
			//Des.	Element

			//CN101 1166	Contract Type Code						M	ID	2/2

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

			//CN102 782		Monetary Amount							O	R	1/18

			//use the cyLineTotal from earlier, to show the charge total
			str = FormatCurrencyForInterface(cyLineTotal,FALSE,FALSE);

			//see if we need to trim the right zeros
			// (j.jones 2008-05-06 11:42) - PLID 29937 - cached m_bTruncateCurrency
			if(m_bTruncateCurrency) {
				str.TrimRight("0");	//first the zeros
				str.TrimRight(".");	//then the decimal, if necessary
			}

			str.Replace(GetCurrencySymbol(),"");
			str.Replace(GetThousandsSeparator(),"");

			OutputString += ParseANSIField(str,1,18);

			//CN103 332		Percent									O	R	1/6

			//we don't use this

			OutputString += ParseANSIField("",1,6);

			//CN104 127		Reference Identification				O	AN	1/30

			//we don't use this

			OutputString += ParseANSIField("",1,30);

			//CN105 338		Terms Discount Percent					O	R	1/6

			//we don't use this

			OutputString += ParseANSIField("",1,6);

			//CN106 799		Version Identifier						O	AN	1/30

			//we don't use this

			OutputString += ParseANSIField("",1,30);

			EndANSISegment(OutputString);
		}

//468		470		REF		Repriced Line Item Reference Number		S		1
//469		470		REF		Adjusted Repriced Line Item Ref. Num.	S		1
//470		470		REF		Prior Authorization or Referral Number	S		2
//472		470		REF		Line Item Control Number				S		1
//474		470		REF		Mammography Certification Number		S		1
//475		470		REF		CLIA Identification						S		1
//477		470		REF		Referring CLIA Facility Identification	S		1
//478		470		REF		Immunization Batch Number				S		1
//479		470		REF		Ambulatory Patient Group (APG)			S		4
//480		470		REF		Oxygen Flow Rate						S		1
//482		470		REF		Universal Product Number (UPN)			S		1

		//not used

//484		475		AMT		Sales Tax Amount						S		1

		//TODO: This is supposed to show not the tax rate, but the actual
		//monetary tax amount. Research and find out if we need this, as most
		//(or all?) insurance companies don't pay tax.

		//The book says that this field is required if the submitter is required
		//to report this information to the receiver.

//485		475		AMT		Approved Amount							S		1

		// (j.jones 2007-02-15 10:26) - PLID 24762 - use the allowed information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending allowed information in 2400

		if(!m_pEBillingInfo->IsPrimary && m_HCFAInfo.ANSI_EnablePaymentInfo == 1
			&& m_HCFAInfo.ANSI_2400Allowed == 1) {

			//Ref.	Data		Name									Attributes
			//Des.	Element

			//officially the description of this is:
			//"The allowed amount equals the amount for the service line that was
			//approved by the payer sending this 837 to another payer."
			
			OutputString = "AMT";

			//AMT01 522			Amount Qualifier Code					M	ID	1/3

			//"AAE" - Approved Amount
			str = "AAE";
			OutputString += ParseANSIField(str, 1, 3);			

			//AMT02 782			Monetary Amount							M	R	1/18

			//this should presumably be the allowed amount from the fee schedule,
			//for the charge provider, OtherInsuranceCompany, and Bill Location

			//calculate in a query

			// (j.jones 2006-12-01 15:55) - PLID 22110 - this should not use the ClaimProviderID
			// (j.jones 2007-02-22 14:03) - PLID 24884 - if the allowable is less than the paid amount per charge,
			// we have to send the paid amount for that charge.
			// (j.jones 2007-03-29 10:51) - PLID 25409 - added options to determine what to send as the allowable,
			// either the fee schedule allowable, the greater of the allowable or payment, or the greater of the
			// allowable or payment + secondary responsibility
			// (j.jones 2009-08-28 17:39) - PLID 32993 - we now completely drop the allowable calculation from
			// this process, as it has since been proven time and time again that fee schedule allowables have
			// absolutely no bearing on this calculation
			// (j.jones 2010-02-03 09:24) - PLID 37170 - include selected adjustments

			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			// (j.jones 2010-09-23 15:50) - PLID 40653 - adjustment group & reason codes are IDs now
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges & applies
			_RecordsetPtr rsTotals = CreateParamRecordset("SELECT "
				"Coalesce(AppliesQ.PaymentAmount, Convert(money,'0.00')) AS PaymentTotal, "
				"Coalesce(OtherInsTotalQ.OtherInsTotal, Convert(money,'0.00')) AS OtherInsTotal, "
				"Coalesce(HCFA_AllowedAdjustmentsQ.AdjustmentTotal, Convert(money,'0.00')) AS HCFA_AllowedAdjustmentTotal "
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
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND ChargesT.ID = {INT} AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null",
				m_pEBillingInfo->OthrInsuredPartyID, m_pEBillingInfo->InsuredPartyID,
				m_pEBillingInfo->HCFASetupID, m_pEBillingInfo->OthrInsuredPartyID,
				AdoFldLong(rsCharges, "ChargeID"));

			//track our individual amounts
			COleCurrency cyPaymentTotal = COleCurrency(0,0);
			COleCurrency cyOtherInsTotal = COleCurrency(0,0);
			COleCurrency cyAllowedAdjustmentTotal = COleCurrency(0,0);

			if(!rsTotals->eof) {
				cyPaymentTotal = AdoFldCurrency(rsTotals, "PaymentTotal", COleCurrency(0,0));
				cyOtherInsTotal = AdoFldCurrency(rsTotals, "OtherInsTotal", COleCurrency(0,0));

				// (j.jones 2010-02-03 09:32) - PLID 37170 - supported including specified adjustments in the allowed total,
				// these are calculated in the recordset
				cyAllowedAdjustmentTotal = AdoFldCurrency(rsTotals, "HCFA_AllowedAdjustmentTotal", COleCurrency(0,0));
			}
			rsTotals->Close();

			// (j.jones 2007-03-29 10:54) - PLID 25409 - now decide what value we should be sending

			COleCurrency cyPaymentAndOtherIns = cyPaymentTotal + cyOtherInsTotal;
			
			// (j.jones 2009-08-28 17:43) - PLID 32993 - Goodbye, old code that ECP misled us into calculating!
			// We no longer utilize the fee schedule allowables, period.
			/*
			long nSecondaryAllowableCalc = m_HCFAInfo.ANSI_SecondaryAllowableCalc;

			switch(nSecondaryAllowableCalc) {
				case 1:	//only send the Fee Schedule allowable
					cyAllowableSent = cyAllowable;
					break;
				case 2: //send the greater of the allowable or the primary paid amount
					if(cyAllowable > cyPaymentTotal)
						cyAllowableSent = cyAllowable;
					else
						cyAllowableSent = cyPaymentTotal;
					break;
				case 3: //send the greater of the allowable or the primary paid amount + secondary resp.
				default:
					if(cyAllowable > cyPaymentAndOtherIns)
						cyAllowableSent = cyAllowable;
					else
						cyAllowableSent = cyPaymentAndOtherIns;
					break;
			}

			// (j.jones 2009-03-16 15:05) - PLID 33544 - ensured our allowable cannot be greater than the charge total
			if(cyAllowableSent > cyLineTotal) {
				cyAllowableSent = cyLineTotal;
			}
			*/

			// (j.jones 2009-08-28 17:44) - PLID 32993 - we will always send the payment + other insurance here
			cyAllowableSent = cyPaymentAndOtherIns;

			// (j.jones 2010-02-03 09:32) - PLID 37170 - supported including specified adjustments in the allowed total,
			// this will already have been calculated earlier (it is not tracked in cyAllowableSent, however)
			str = FormatCurrencyForInterface(cyAllowableSent + cyAllowedAdjustmentTotal, FALSE, FALSE);
			//see if we need to trim the right zeros
			// (j.jones 2008-05-06 11:42) - PLID 29937 - cached m_bTruncateCurrency
			if(m_bTruncateCurrency) {
				str.TrimRight("0");	//first the zeros
				str.TrimRight(".");	//then the decimal, if necessary
			}
			OutputString += ParseANSIField(str, 1, 18);

			//AMT03 NOT USED

			EndANSISegment(OutputString);

			bSentAllowedAmount = TRUE;
		}

//486		475		AMT		Postage Claimed Amount					S		1
//487		480		K3		File Information						S		10

//488		485		NTE		Line Note								S		1

		// (j.jones 2011-07-06 13:57) - PLID 44327 - now anesthesia times can optionally
		// export here, and if they do they will be sent instead of the normal NTE output
		CString strNTE = "";

		if(m_pEBillingInfo->AnesthesiaTimes && AdoFldBool(rsCharges, "Anesthesia",FALSE)) {
			
			strNTE = CalculateAnesthesiaNoteForANSI(m_pEBillingInfo->BillID);
		}

		// (j.jones 2011-09-16 11:25) - PLID 45526 - if the charge has a Notes record that is configured
		// to send on the claim, send it here - this overrides all other settings to fill the 2400 NTE
		//*except* the anesthesia note
		if(strNTE.IsEmpty()) {
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
			//PMT - Payment
			//TPO - Third Party Organization Notes

			OutputString += ParseANSIField("ADD",3,3);

			//NTE02 352			Description								M	AN	1/80

			OutputString += ParseANSIField(strNTE,1,80);

			EndANSISegment(OutputString);
		}

//489		488		PS1		Purchased Service Information			S		1
//491		491		HSD		Health Care Services Delivery			S		1
//495		492		HCP		Line Pricing/Repricing Information		S		1

		//not used

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2400_Prof");

	return Error_Other;
}

int CEbilling::ANSI_4010_2400_Inst(ADODB::_RecordsetPtr &rsCharges) {

	//Service Line

#ifdef _DEBUG

	CString str;

	str = "\r\n2400\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	//increment the count for the current service line
	m_ANSIServiceCount++;

	FieldsPtr fields = rsCharges->Fields;

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2400 - SERVICE LINE									50

//444		365		LX		Service Line							R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "LX";

		//LX01	554			Assigned Number							M	N0	1/6

		//m_ANSIServiceCount
		str.Format("%li",m_ANSIServiceCount);
		OutputString += ParseANSIField(str,1,6);

		EndANSISegment(OutputString);

//445		370		SV2		Institutional Service Line				R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "SV2";

		//SV201		234		Product/Service ID						X	AN	1/48

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
		
		//SV202	C003		Composite Medical Procedure Identifier	M

		//SV201-1	235		Product/Service ID Qualifier			M	ID	2/2

		//static "HC"

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

			//SV202-3	1339	Procedure Modifier						O	AN	2/2

			//ChargesT.CPTModifier
			str = GetFieldFromRecordset(rsCharges,"CPTModifier");
			if(str != "") {
				strCPTOut += ":";
				strCPTOut += str;
			}

			//SV202-4	1339	Procedure Modifier						O	AN	2/2
			
			//ChargesT.CPTModifier2
			str = GetFieldFromRecordset(rsCharges,"CPTModifier2");
			if(str != "") {
				strCPTOut += ":";
				strCPTOut += str;
			}

			//SV202-5	1339	Procedure Modifier						O	AN	2/2

			//ChargesT.CPTModifier3
			str = GetFieldFromRecordset(rsCharges,"CPTModifier3");
			if(str != "") {
				strCPTOut += ":";
				strCPTOut += str;
			}

			//SV202-6	1339	Procedure Modifier						O	AN	2/2
				
			//ChargesT.CPTModifier4
			str = GetFieldFromRecordset(rsCharges,"CPTModifier4");
			if(str != "") {
				strCPTOut += ":";
				strCPTOut += str;
			}

			OutputString += strCPTOut;
		}
		else {
			OutputString += "*";
		}

		//SV202-7	NOT USED

		//SV203	782			Monetary Amount							O	R	1/18

		//note: when CPT codes are not used, we would group by RevCode and in turn
		//the amount and quantity would add up based on RevCode. We don't do this right now.

		//LineTotal is the amount * qty * tax * modifiers
		var = fields->Item["LineTotal"]->Value;
		if(var.vt == VT_CY) {
			str = FormatCurrencyForInterface(var.cyVal,FALSE,FALSE);

			//see if we need to trim the right zeros
			// (j.jones 2008-05-06 11:45) - PLID 29937 - cached m_bTruncateCurrency
			if(m_bTruncateCurrency) {
				str.TrimRight("0");	//first the zeros
				str.TrimRight(".");	//then the decimal, if necessary
			}
		}
		else
			str = "0";
		str.Replace(GetCurrencySymbol(),"");
		str.Replace(GetThousandsSeparator(),"");
		OutputString += ParseANSIField(str,1,18);

		//SV204	355			Unit or Basis for Measurement Code		X	ID	2/2

		//UN - Unit
		OutputString += ParseANSIField("UN",2,2);

		//SV205	380			Quantity								X	R	1/15

		//ChargesT.Quantity
		var = fields->Item["Quantity"]->Value;
		if(var.vt == VT_R8)
			str.Format("%g",var.dblVal);
		else
			str = "1";
		OutputString += ParseANSIField(str,1,15);

		//SV206	1371		Unit Rate								O	R	1/10
		//not used
		OutputString += "*";

		//SV207	782			Monetary Amount							O	R	1/18
		//Box 44, Non-Covered Charges - we don't use this
		OutputString += "*";
				
		//SV208	NOT USED
		//SV209	NOT USED
		//SV210 NOT USED

		EndANSISegment(OutputString);

//450		385		SV4		Prescription Number						S		1
//452		420		PWK		Line Supplemental Information			S		1
		//not used

//456		455		DTP		Service Line Date						R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "DTP";

		//DTP01	374			Date/Time Qualifier						M	ID	3/3
		
		//static "472"
		OutputString += ParseANSIField("472",3,3);

		//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

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

		//DTP03	1251		Date Time Period						M	AN	1/35

		OutputString += ParseANSIField(strDate,1,35);

		EndANSISegment(OutputString);

///////////////////////////////////////////////////////////////////////////////

//458		455		DTP		Assessment Date							S		1
		//not used

//460		475		AMT		Service Tax Amount						S		1
//461		475		AMT		Facility Tax Amount						S		1
		
		//TODO: These are supposed to show not the tax rate, but the actual
		//monetary tax amount. Research and find out if we need either line, as most
		//(or all?) insurance companies don't pay tax.

		//The book says that each field is required if a service tax/surcharge
		//applies to the service reported in SV201

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2400_Inst");

	return Error_Other;
}

// (j.jones 2008-05-28 14:14) - PLID 30176 - added support for Loop 2410 and NDC Codes
// (j.jones 2009-08-12 16:48) - PLID 35096 - added support for unit information and prescription number
int CEbilling::ANSI_4010_2410(CString strNDCCode, COleCurrency cyDrugUnitPrice, CString strUnitType, double dblUnitQty, CString strPrescriptionNumber) {
	
	//Drug Identification

	try {

		CString str;

#ifdef _DEBUG

		str = "\r\n2410\r\n";
		m_OutputFile.Write(str,str.GetLength());	
#endif

		CString OutputString;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2410 - DRUG IDENTIFICATION							25

//Addenda Page
//A71		494		LIN		Drug Identification						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "LIN";

		//LIN01	NOT USED;

		OutputString += "*";

		//LIN02 235			Product/Service ID Qualifier			M	ID	2/2

		//static "N4"

		//N4 - National Drug Code in 5-4-2 Format
		OutputString += ParseANSIField("N4",2,2);

		//LIN03 234			Product/Service ID						M	AN	1/48

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

//A74		495		CTP		Drug Pricing							S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2009-08-12 16:30) - PLID 35096 - supported this segment

		OutputString = "CTP";

		//CTP01	NOT USED
		OutputString += "*";		
		//CTP02	NOT USED
		OutputString += "*";

		//CTP03	212			Unit Price								X	R	1/17
		
		str = FormatCurrencyForInterface(cyDrugUnitPrice, FALSE, FALSE);
		//see if we need to trim the right zeros
		if(m_bTruncateCurrency) {
			str.TrimRight("0");	//first the zeros
			str.TrimRight(".");	//then the decimal, if necessary
		}
		OutputString += ParseANSIField(str, 1, 17);

		//CTP04	380			Quantity								X	R	1/15

		str.Format("%g", dblUnitQty);
		OutputString += ParseANSIField(str,1,15);

		//CTP05	C001		COMPOSITE UNIT OF MEASURE				X

		//CTP05 - 1 355		Unit or Basis for Measurement Code		M	D	2/2

		//F2 - International Unit
		//GR - Gram
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

//A77		496		REF		Prescription Number						S		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		// (j.jones 2009-08-12 16:30) - PLID 35096 - supported this segment

		OutputString = "REF";

		//REF01	128			Reference Identification Qualifier		M	ID	2/3
		
		//XZ - Pharmacy Prescription Number
		OutputString += ParseANSIField("XZ",2,3);

		//REF02	127			Reference Identification				X	AN	1/30

		strPrescriptionNumber.TrimLeft();
		strPrescriptionNumber.TrimRight();
		OutputString += ParseANSIField(strPrescriptionNumber,1,30);

		//REF03	NOT USED
		OutputString += "*";
		//REF04	NOT USED
		OutputString += "*";

		if(strPrescriptionNumber != "") {
			EndANSISegment(OutputString);
		}

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2410");

	return Error_Other;
}

int CEbilling::ANSI_4010_2420A(long ProviderID) {

	//Rendering Provider Name

	//JJ - We CAN have different providers per charge, so this loop can be used.

	//First check to see if the charge line provider is different from the provider
	//we chose to use as the claim provider. If it's different, then use this loop.

#ifdef _DEBUG

	CString str;

	str = "\r\n2420A\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

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

//501		500		NM1		Rendering Provider Name					S		1

//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "NM1";

		//NM101	98			Entity Identifier Code					M	ID	2/3

		//static "82"
		OutputString += ParseANSIField("82",2,3);

		//NM102	1065		Entity Type Qualifier					M	ID	1/1

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

		//NM103	1035		Name Last or Organization Name			O	AN	1/35

		//PersonT.Last

		if(bUseProviderCompanyOnClaims) {
			str = strProviderCompany;
		}
		else {
			str = GetFieldFromRecordset(rs, "Last");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,35);

		//NM104	1036		Name First								O	AN	1/25

		//PersonT.First

		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "First");
		}

		// (j.jones 2007-08-06 10:34) - PLID 26951 - do not un-punctuate names
		str = NormalizeName(str); // (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
		OutputString += ParseANSIField(str,1,25);

		//NM105	1037		Name Middle								O	AN	1/25

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

		//NM107	1039		Name Suffix								O	AN	1/10

		//PersonT.Title
		// (j.jones 2007-08-03 11:44) - PLID 26934 - changed from Suffix to Title
		if(bUseProviderCompanyOnClaims) {
			str = "";
		}
		else {
			str = GetFieldFromRecordset(rs, "Title");
		}

		OutputString += ParseANSIField(str,1,10);

		//NM108	66			Identification Code	Qualifier			X	ID	1/2

		CString strQual;
		str = "";

		// (j.jones 2006-11-14 08:55) - PLID 23413 - determine whether the
		// NM108/109 elements should show the NPI or the EIN/SSN
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		long nNM109_IDType = 0;		
		if(m_actClaimType == actInst)
			// (j.jones 2006-11-14 08:56) - PLID 23446 - check the UB92 setup
			nNM109_IDType = m_UB92Info.NM109_IDType;
		else
			nNM109_IDType = m_HCFAInfo.NM109_IDType;

		if(nNM109_IDType == 1) {

			// (j.jones 2006-11-14 08:57) - PLID 23413 - if 1,
			// then use the fed. employer ID or SSN, based on Box25
			// (j.jones 2006-11-14 10:47) - PLID 23446 - just use fed. emp. id.

			//"24" for EIN, "34" for SSN - from HCFASetupT.Box25
			if(m_HCFAInfo.Box25==1 && m_actClaimType != actInst)
				strQual = "34";
			else
				strQual = "24";

			if(m_HCFAInfo.Box25 == 1 && m_actClaimType != actInst) {
				str = GetFieldFromRecordset(rs, "SocialSecurity");
			}
			else {
				str = GetFieldFromRecordset(rs, "Fed Employer ID");
			}
		}
		else {

			// (j.jones 2006-11-14 08:58) - PLID 23413, 23446 - if 0,
			// then use the NPI

			//"XX" for NPI
			strQual = "XX";

			// (j.jones 2007-08-08 13:14) - PLID 25395 - if there is a 24J override in AdvHCFAPinT, use it here
			// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
		}

		str = StripNonNum(str);
		
		if(str != "")
			OutputString += ParseANSIField(strQual,1,2);
		else
			OutputString += "*";

		//NM109	67			Identification Code						X	AN	2/80

		//ProvidersT.Fed Employer ID or PersonT.SocialSecurity
		//depends on setting in HCFASetupT.Box25

		if(str != "")
			OutputString += ParseANSIField(str,2,80);
		else
			OutputString += "*";

		//NM110 NOT USED
		//NM111	NOT USED

		EndANSISegment(OutputString);

//504		505		PRV		Rendering Provider Specialty Info.		R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "PRV";

		//PRV01	1221		Provider Code							M	ID	1/3

		//static "PE" (performing)
		OutputString += ParseANSIField("PE",1,3);

		CString strTaxonomy = AdoFldString(rs, "TaxonomyCode","");
		strTaxonomy.TrimLeft();
		strTaxonomy.TrimRight();

		// (j.jones 2008-05-01 10:47) - PLID 28691 - don't output if the taxonomy code is blank
		if(!strTaxonomy.IsEmpty()) {

			//PRV02	128			Reference Identification Qualifier		M	ID	2/3

			//static value "ZZ"
			OutputString += ParseANSIField("ZZ",2,3);

			//PRV03	127			Reference Identification				M	AN	1/30

			//ProvidersT.TaxonomyCode
			//This comes from a published HIPAA list.
			//"2086S0122X" is for Plastic and Reconstructive Surgery
			OutputString += ParseANSIField(strTaxonomy,1,30);

			//PRV04	NOT USED
			OutputString += "*";
			//PRV05	NOT USED
			OutputString += "*";
			//PRV06	NOT USED
			OutputString += "*";

			EndANSISegment(OutputString);
		}

//507		525		REF		Rendering Provider Secondary Ident.		S		5

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "REF";

		// (j.jones 2008-05-02 10:09) - PLID 27478 - we need to silently skip
		// adding additional REF segments that have the same qualifier, so
		// track them in an array
		CStringArray arystrQualifiers;

		//REF01	128			Reference Identification Qualifier		M	ID	2/3

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

		//REF02	127			Reference Identification				X	AN	1/30

		OutputString += ParseANSIField(strID,1,30);

		//REF03	NOT USED
		OutputString += "*";
		//REF04	NOT USED
		OutputString += "*";

		// (j.jones 2007-09-20 10:51) - PLID 27455 - do not send this segment if the
		// qualifier or ID are blank
		if(strIdentifier != "" && strID != "") {
			EndANSISegment(OutputString);

			// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
			arystrQualifiers.Add(strIdentifier);
		}

		// (j.jones 2006-11-14 11:48) - PLID 23413 - the HCFA Group now allows the option
		// to append an EIN/SSN, or NPI as an additional REF segment

		long nExtraREF_IDType = m_HCFAInfo.ExtraREF_IDType;

		//0 - EIN/SSN, 1 - NPI, 2 - none
		if(nExtraREF_IDType != 2) {

			//export the extra ID

			OutputString = "REF";

			//REF01	128			Reference Identification Qualifier		M	ID	2/3
			
			strIdentifier = "";
			strID = "";

			if(nExtraREF_IDType == 0) {
				// (j.jones 2006-11-14 08:57) - PLID 23413 - if 1,
				// then use the fed. employer ID or SSN, based on Box25

				//"EI" for EIN, "SY" for SSN - from HCFASetupT.Box25
				if(m_HCFAInfo.Box25==1)
					strIdentifier = "SY";
				else
					strIdentifier = "EI";

				if(m_HCFAInfo.Box25 == 1) {
					strID = GetFieldFromRecordset(rs, "SocialSecurity");
				}
				else {
					strID = GetFieldFromRecordset(rs, "Fed Employer ID");
				}
			}
			else {
				//NPI

				//"XX" for NPI
				strIdentifier = "XX";

				// (j.jones 2007-08-08 13:14) - PLID 25395 - if there is a 24J override in AdvHCFAPinT, use it here
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType != actInst) {
					// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
					_RecordsetPtr rsOver = CreateParamRecordset("SELECT Box24JNPI FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}", ProviderID,m_pEBillingInfo->BillLocation,m_pEBillingInfo->HCFASetupID);
					if(!rsOver->eof) {
						var = rsOver->Fields->Item["Box24JNPI"]->Value;
						if(var.vt == VT_BSTR) {
							CString strOver = VarString(var,"");
							strOver.TrimLeft();
							strOver.TrimRight();
							if(!strOver.IsEmpty())
								strID = strOver;
						}
					}
					rsOver->Close();
				}

				if(strID.IsEmpty()) {
					strID = AdoFldString(rs, "NPI", "");
				}
			}

			strID = StripNonNum(strID);

			OutputString += ParseANSIField(strIdentifier,2,3);

			//REF02	127			Reference Identification				X	AN	1/30

			OutputString += ParseANSIField(strID,1,30);

			//REF03	NOT USED
			OutputString += "*";

			//REF04	NOT USED
			OutputString += "*";

			// (j.jones 2010-04-16 11:21) - PLID 38225 - if the qualifier is XX, send nothing
			if(m_bDisableREFXX && strIdentifier.CompareNoCase("XX") == 0) {
				strIdentifier = "";
				strID = "";
				//log that this happened
				Log("Ebilling file tried to export 2420A Additional REF*XX for provider ID %li, bill ID %li, but REF*XX is disabled.", m_pEBillingInfo->ProviderID, m_pEBillingInfo->BillID);
			}

			// (j.jones 2008-05-02 10:02) - PLID 27478 - disallow sending the additional REF
			// segment if the qualifier has already been used
			if(strIdentifier != "" && strID != "" && !IsQualifierInArray(arystrQualifiers, strIdentifier)) {
				EndANSISegment(OutputString);

				// (j.jones 2008-05-02 10:05) - PLID 27478 - add this qualifier to our array
				arystrQualifiers.Add(strIdentifier);
			}
		}

		rs->Close();

///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2420A");

	return Error_Other;
}

/* These loops all pertain to different providers/locations per charge line. We won't use them.

int CEbilling::ANSI_4010_2420B() {

	//Purchased Service Provider Name

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2420B - PURCHASED SERVICE PROVIDER NAME				1

//509		500		NM1		Purchased Service Provider Name			S		1
//512		525		REF		Purchased Service Prov. Secondary Info.	S		5

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2420B");

	return Error_Other;
}

int CEbilling::ANSI_4010_2420C() {

	//Service Facility Location

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2420C - SERVICE FACILITY INFORMATION					1

//514		500		NM1		Service Facility Location				S		1
//518		514		N3		Service Facility Location Address		R		1
//519		520		N4		Service Facility Loc. City/State/Zip	R		1
//521		525		REF		Service Facility Loc. Secondary Ident.	S		5

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2420C");

	return Error_Other;
}

int CEbilling::ANSI_4010_2420D() {

	//Supervising Provider Name

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2420D - SUPERVISING PROVIDER NAME						1

//523		500		NM1		Supervising Provider Name				S		1
//527		525		REF		Supervising Provider Secondary Info.	S		5

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2420D");

	return Error_Other;
}

int CEbilling::ANSI_4010_2420E() {

	//Ordering Provider Name

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2420E	- ORDERING PROVIDER NAME						1

//529		500		NM1		Ordering Provider Name					S		1
//533		514		N3		Ordering Provider Address				S		1
//534		520		N4		Ordering Provider City/State/Zip		S		1
//536		525		REF		Ordering Provider Secondary Information	S		5
//538		530		PER		Ordering Provider Contact Information	S		1

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2420E");

	return Error_Other;
}

int CEbilling::ANSI_4010_2420F() {

	//Referring Provider Name

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2420F - REFERRING PROVIDER NAME						2

//541		500		NM1		Referring Provider Name					S		1
//544		505		PRV		Referring Provider Specialty Info.		S		1
//547		525		REF		Referring Provider Secondary Ident.		S		5

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2420F");

	return Error_Other;
}

*/

/*

int CEbilling::ANSI_4010_2420G() {

	//Other Payer Prior Authorization Or Referral Number

	//JJ - We don't believe we will need this.

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2420G - OTHER PAYER PRIOR AUTH OR REFERRAL NUMBER		4

//549		500		NM1		Other Payer Prior Auth. / Referral Num.	S		1
//552		525		REF		Other Payer Prior Auth. / Referral Num.	R		2

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2420G");

	return Error_Other;
}

*/

// (j.jones 2008-05-23 10:48) - PLID 29084 - added parameters for allowables
// (j.jones 2010-03-31 15:17) - PLID 37918 - added parameter for total count of charges being submitted
int CEbilling::ANSI_4010_2430(_RecordsetPtr &rsCharges, BOOL bSentAllowedAmount, COleCurrency cyAllowableSent, long nCountOfCharges) {

	//Line Adjudication Information

	// (j.jones 2006-11-21 10:10) - PLID 23415 - enabled usage of this loop

#ifdef _DEBUG

	CString str;

	str = "\r\n2430\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2430 - LINE ADJUDICATION INFORMATION					25

//554		540		SVD		Line Adjudication Information			S		>1

		// (j.jones 2006-11-24 15:20) - PLID 23415 - use the payment information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending adjustment information in this loop.
		// The reason is that payments are optional here unless adjustments are
		// present, in which case the payments have to be here to balance against
		// the adjustments.

		// (j.jones 2006-11-27 17:21) - PLID 23652 - supported this on the UB92

		COleCurrency cyAmtPaid = COleCurrency(0,0);

		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(!m_pEBillingInfo->IsPrimary &&
			((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1 && m_HCFAInfo.ANSI_AdjustmentLoop == 1)
			|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1 && m_UB92Info.ANSI_AdjustmentLoop == 1))) {

			// (j.jones 2006-11-21 13:55) - PLID 23415 - shows payment information per charge
			// (j.jones 2006-11-27 17:21) - PLID 23652 - this is slightly different for the UB92

			//we need the total payment amount for this charge by the OthrInsurance on the claim,
			//the sum should return 0 if there are no payments

			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			// (j.jones 2011-08-16 17:22) - PLID 44805 - ignore original & void line items
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
				AdoFldLong(rsCharges, "ChargeID"), m_pEBillingInfo->OthrInsuredPartyID);

			if(!rsPayTotal->eof) {

				OutputString = "SVD";

				//Ref.	Data		Name									Attributes
				//Des.	Element

				//SVD01 67			Identification Code						M	AN	2/80

				//has to be identical to NM109 from 2330B
				
				// (j.jones 2009-08-05 10:20) - PLID 34467 - this is now in the loaded claim info
				// (j.jones 2009-12-16 15:54) - PLID 36237 - split out into HCFA and UB payer IDs
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				// (j.jones 2012-03-21 15:23) - PLID 48870 - moved secondary info to a pointer
				if(m_actClaimType == actInst) {
					str = m_pEBillingInfo->pOtherInsuranceInfo->strUBPayerID;
				}
				else {
					str = m_pEBillingInfo->pOtherInsuranceInfo->strHCFAPayerID;
				}

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
					CString strTPLCode = m_pEBillingInfo->pOtherInsuranceInfo->strTPLCode;
					strTPLCode.TrimLeft();
					strTPLCode.TrimRight();
					if(!strTPLCode.IsEmpty()) {
						str = strTPLCode;
					}
				}

				OutputString += ParseANSIField(str, 2, 80);

				//SVD02 782			Monetary Amount							M	R	1/18

				COleCurrency cyAmt = COleCurrency(0,0);

				//add up the payment amount so we know how much to adjust, but then
				//only actually show the payment information if the option is enabled, else zero				
				cyAmt = AdoFldCurrency(rsPayTotal, "PaymentAmount", COleCurrency(0,0));
				//store this for later, for the adjustments
				cyAmtPaid += cyAmt;

				str = FormatCurrencyForInterface(cyAmt, FALSE, FALSE);
				//see if we need to trim the right zeros
				// (j.jones 2008-05-06 11:42) - PLID 29937 - cached m_bTruncateCurrency
				if(m_bTruncateCurrency) {
					str.TrimRight("0");	//first the zeros
					str.TrimRight(".");	//then the decimal, if necessary
				}				

				OutputString += ParseANSIField(str, 1, 18);

				//SVD03 C003		COMPOSITE MEDICAL PROCEDURE IDENTIFIER	O

				//SVD03-1	235		Product/Service ID Qualifier			M	ID	2/2

				//static "HC"
				CString strQual = "HC";

				// (j.jones 2007-07-27 09:17) - PLID 26839 - changed so we don't export
				// the HC composite segment if we don't have a CPT code, UB claims only

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

				//not used
				/*
				OutputString += ":";
				OutputString += ParseANSIField("", 1, 80);
				*/

				//SVD04	234			Product/Service ID						O	AN	1/48

				//SVD04 is not filled on the HCFA
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
				if(m_actClaimType == actInst) {

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
				}
				else {
					OutputString += ParseANSIField("",1,48);
				}

				//SVD05 380			Quantity								O	R	1/15

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

				//SVD06 554			Assigned Number							O	N0	1/6

				//has to be the same number used in the LX portion of the 2400 loop

				// (j.jones 2010-03-31 15:19) - PLID 37918 - added the ANSI_Hide2430_SVD06_OneCharge
				// setting, which if enabled will cause this field to be blank if there is only
				// one charge being submitted on the claim
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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

//558		545		CAS		Line Adjustment							S		99

		// (j.jones 2006-11-24 15:20) - PLID 23415 - use the adjustment information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending adjustment information in this loop

		COleCurrency cyAmtAdjusted = COleCurrency(0,0);

		// (j.jones 2006-11-27 17:23) - PLID 23652 - supported this on the UB92
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum

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
			// (j.jones 2011-08-16 17:22) - PLID 44805 - ignore original & void line items
			_RecordsetPtr rsAdj = CreateParamRecordset("SELECT "
				"CASE WHEN AdjustmentGroupCodesT.Code Is Null THEN {STRING} ELSE AdjustmentGroupCodesT.Code END AS GroupCode, "
				"CASE WHEN AdjustmentReasonCodesT.Code Is Null THEN {STRING} ELSE AdjustmentReasonCodesT.Code END AS ReasonCode, "
				"AppliedQ.AdjAmount "
				"FROM PaymentsT "
				"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"INNER JOIN (SELECT Sum(Amount) AS AdjAmount, SourceID "
				"	FROM AppliesT "
				"	WHERE DestID = {INT} "
				"	GROUP BY SourceID) AS AppliedQ ON PaymentsT.ID = AppliedQ.SourceID "
				"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID "
				"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 2 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND PaymentsT.InsuredPartyID <> -1 "
				"AND PaymentsT.InsuredPartyID = {INT} "
				"ORDER BY AdjustmentGroupCodesT.Code ", strDefaultInsAdjGroupCode, strDefaultInsAdjReasonCode,
				AdoFldLong(rsCharges, "ChargeID"), m_pEBillingInfo->OthrInsuredPartyID);

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
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
	
					// (j.jones 2007-06-15 11:46) - PLID 26309 - Now we have a setting that lets the user
					// configure a default group code.
					// (j.jones 2009-03-11 10:39) - PLID 33446 - We added a different setting for a default
					// for "real" adjustments, as opposed to this existing setting for "fake" adjustments.
					// This setting has been renamed to ANSI_DefaultRemAdjGroupCode/ReasonCode.
					//default to OA for other adjustment
					// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
				// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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
			ANSI_4010_OutputCASSegments(aryCASSegments);
		}

//566		550		DTP		Line Adjudication Date					R		1

		// (j.jones 2006-11-24 15:20) - PLID 23415 - use the date information
		// if not sending to primary, and is using secondary sending,
		// and enabled sending adjustment information in this loop

		// (j.jones 2006-11-27 17:23) - PLID 23652 - supported this on the UB92
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum

		if(!m_pEBillingInfo->IsPrimary &&
			((m_actClaimType != actInst && m_HCFAInfo.ANSI_EnablePaymentInfo == 1 && m_HCFAInfo.ANSI_AdjustmentLoop == 1)
			|| (m_actClaimType == actInst && m_UB92Info.ANSI_EnablePaymentInfo == 1 && m_UB92Info.ANSI_AdjustmentLoop == 1))) {

			// (j.jones 2006-11-21 10:44) - PLID 23415, 23652 - output the date the claim was paid by insurance

			//since we send payments and adjustments in this loop, just find the first
			//payment or adjustment date (in theory, should all be the same date)
			// (j.jones 2008-05-06 15:20) - PLID 27453 - parameterized
			// (j.jones 2011-08-16 17:22) - PLID 44805 - ignore original & void line items
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
				AdoFldLong(rsCharges, "ChargeID"), m_pEBillingInfo->OthrInsuredPartyID);

			//it is possible we may have no actual applied item, so send today's date otherwise
			COleDateTime dtDate = COleDateTime::GetCurrentTime();
			
			if(!rsDates->eof) {
				dtDate = AdoFldDateTime(rsDates, "Date");
			}
			rsDates->Close();

			//Ref.	Data		Name									Attributes
			//Des.	Element

			OutputString = "DTP";

			//DTP01	374			Date/Time Qualifier						M	ID	3/3

			//573 - Date Claim Paid

			str = "573";
			OutputString += ParseANSIField(str, 3, 3);

			//DTP02	1250		Date Time Period Format Qualifier		M	ID	2/3

			//D8 - date is CCYYMMDD

			str = "D8";
			OutputString += ParseANSIField(str, 2, 3);

			//DTP03	1251		Date Time Period						M	AN	1/35

			//LineItemT.Date

			str = dtDate.Format("%Y%m%d");
			OutputString += ParseANSIField(str, 1, 35);

			EndANSISegment(OutputString);
		}
		
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2430");

	return Error_Other;
}

/*
int CEbilling::ANSI_4010_2440() {

	//Form Identification Code

	//JJ - I don't believe this is needed.

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							LOOP ID - 2440 - FORM IDENTIFICATION CODE						5

//567		551		LQ		Form Identification Code				S		1
//569		552		FRM		Supporting Documentation				R		99

		//EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_2440");

	return Error_Other;
}
*/

int CEbilling::ANSI_4010_Trailer() {

	//Trailer

#ifdef _DEBUG

	CString str;

	str = "\r\nTRAILER\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

//Page #	Pos.#	Seg.ID	Name									Usage	Repeat	Loop Repeat

//							TRAILER

//572		555		SE		Transaction Set Trailer					R		1

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "SE";		

		//SE01	96			Number Of Included Segments				M	N0	1/10

		//m_ANSISegmentCount

		//DRT - I changed this from incrementing the count, to just printing 1 higher
		//		otherwise it would increment here, and then it would again increment
		//		in the EndANSISegment().  While in all cases so far the value doesn't 
		//		matter past this point, you never know when it will, so I figure this
		//		was a little more robust.
		str.Format("%li",m_ANSISegmentCount + 1);
		OutputString += ParseANSIField(str,1,10);

		//SE02	329			Transaction Set Control Number			M	AN	4/9

		//m_strBatchNumber
		OutputString += ParseANSIField(m_strBatchNumber,4,9,TRUE,'R','0');

		EndANSISegment(OutputString);
///////////////////////////////////////////////////////////////////////////////

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_Trailer");

	return Error_Other;
}

int CEbilling::ANSI_4010_InterchangeHeader() {

	//Interchange Control Header (page B.3)

	//The Interchange Control Header is one of the few records with a required length limit.
	//As a result, each element will be filled to a set size, even if the entire element is blank.
	//This will be the only time we really need to se bForceFill to TRUE in the ParseANSIField() function.

#ifdef _DEBUG

	CString str;

	str = "\r\nInterchange Control Header\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		// (j.jones 2008-05-06 11:49) - PLID 29937 - removed the EbillingFormatsT recordset, as everything in it is cached now

		CString OutputString,str;
		_variant_t var;

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "ISA";

		//ISA01	I01			Authorization Information Qualifier		M	ID	2/2

		CString strISA01Qual = m_strISA01Qual;

		//EbillingFormatsT.ISA01Qual
		OutputString += ParseANSIField(strISA01Qual,2,2,TRUE);

		//ISA02	I02			Authorization Information				M	AN	10/10
		
		//(I01 and I02 are advised to be 00 and empty for now)
		CString strISA02 = m_strISA02;

		//EbillingFormatsT.ISA02
		OutputString += ParseANSIField(strISA02,10,10,TRUE);

		//ISA03	I03			Security Information Qualifier			M	ID	2/2

		CString strISA03Qual = m_strISA03Qual;

		//EbillingFormatsT.ISA03Qual
		OutputString += ParseANSIField(strISA03Qual,2,2,TRUE);

		//ISA04	I04			Security Information					M	AN	10/10

		//(I03 and I04 are advised to be 00 and empty for now)
		CString strISA04 = m_strISA04;

		//EbillingFormatsT.ISA04
		OutputString += ParseANSIField(strISA04,10,10,TRUE);

		//ISA05	I05			Interchange ID Qualifier				M	ID	2/2

		//default "ZZ" ("ZZ" is "Mutually Defined")
		CString strSubmitterQual = m_strSubmitterISA05Qual;
		OutputString += ParseANSIField(strSubmitterQual,2,2,TRUE);

		//ISA06	I06			Interchange Sender ID					M	AN	15/15

		CString strSubmitterID = m_strSubmitterISA06ID;

		//EbillingFormatsT.SubmitterISA06ID
		OutputString += ParseANSIField(strSubmitterID,15,15,TRUE);

		//ISA07	I05			Interchange ID Qualifier				M	ID	2/2

		//default "ZZ" ("ZZ" is "Mutually Defined")
		CString strReceiverQual = m_strReceiverISA07Qual;
		OutputString += ParseANSIField(strReceiverQual,2,2,TRUE);

		//ISA08	I07			Interchange Receiver ID					M	AN	15/15

		CString strReceiverID = m_strReceiverISA08ID;

		//EbillingFormatsT.ReceiverISA08ID
		str = strReceiverID;
		OutputString += ParseANSIField(str,15,15,TRUE);

		//ISA09	I08			Interchange Date						M	DT	6/6

		//current date YYMMDD
		COleDateTime dt = COleDateTime::GetCurrentTime();
		str = dt.Format("%y%m%d");
		OutputString += ParseANSIField(str,6,6,TRUE);

		//ISA10	I09			Interchange Time						M	TM	4/4

		//current time HHMM
		str = dt.Format("%H%M");
		OutputString += ParseANSIField(str,4,4,TRUE);

		//ISA11	I10			Interchange Control Standards Ident.	M	ID	1/1

		//static "U"
		OutputString += ParseANSIField("U",1,1,TRUE);

		//ISA12	I11			Interchange Control Version Number		M	ID	5/5

		//static "00401"
		OutputString += ParseANSIField("00401",5,5,TRUE);

		//ISA13	I12			Interchange Control Number				M	N0	9/9

		//m_strBatchNumber
		OutputString += ParseANSIField(m_strBatchNumber,9,9,TRUE,'R','0');

		//ISA14	I13			Acknowledgement Requested				M	ID	1/1

		//static "0" (set this to 1 if we want acknowledgement)
		OutputString += ParseANSIField("0",1,1,TRUE);

		//ISA15	I14			Usage Indicator							M	ID	1/1

		//Prod/Test
		if (GetRemotePropertyInt ("EnvoyProduction", 0, 0, "<None>") == 0) { //test
			str = "T";
		}
		else { //production
			str = "P";
		}
		OutputString += ParseANSIField(str,1,1,TRUE);

		//ISA16	I15			Component Element Separator				M		1/1

		//static ":"
		OutputString += ParseANSIField(":",1,1,TRUE);
		
		EndANSISegment(OutputString);

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_InterchangeHeader");

	return Error_Other;

}

int CEbilling::ANSI_4010_InterchangeTrailer() {

	//Interchange Control Trailer (page B.7)

#ifdef _DEBUG

	CString str;

	str = "\r\nInterchange Control Trailer\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "IEA";

		//IEA01	I16			Number Of Included Functional Groups	M	N0	1/5

		//JJ - I don't foresee us supporting multiple functional groups. I don't
		//quite know why they would be needed. If we do use them, however, we must
		//make this another member counter.

		//static "1"
		OutputString += ParseANSIField("1",1,5,TRUE,'R','0');

		//IEA02	I12			Interchange Control Number				M	N0	9/9
		
		//m_strBatchNumber
		OutputString += ParseANSIField(m_strBatchNumber,9,9,TRUE,'R','0');
		
		EndANSISegment(OutputString);

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_InterchangeTrailer");

	return Error_Other;

}

int CEbilling::ANSI_4010_FunctionalGroupHeader() {

	//Functional Group Header (page B.9)

#ifdef _DEBUG

	CString str;

	str = "\r\nFunctional Group Header\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "GS";

		//GS01	479			Functional Identifier Code				M	ID	2/2

		//static "HC"
		OutputString += ParseANSIField("HC",2,2);

		//GS02	142			Application Sender's Code				M	AN	2/15

		// (j.jones 2008-05-06 11:51) - PLID 29937 - cached SubmitterGS02ID
		CString strSubmitterID = m_strSubmitterGS02ID;

		//EbillingFormatsT.SubmitterGS02ID
		OutputString += ParseANSIField(strSubmitterID,2,15);

		//GS03	124			Application Receiver's Code				M	AN	2/15

		// (j.jones 2008-05-06 11:51) - PLID 29937 - cached ReceiverGS03ID
		CString strReceiverID = m_strReceiverGS03ID;

		//EbillingFormatsT.ReceiverGS03ID
		str = strReceiverID;
		OutputString += ParseANSIField(str,2,15);

		//GS04	373			Date									M	DT	8/8

		//current date CCYYMMDD
		COleDateTime dt = COleDateTime::GetCurrentTime();
		str = dt.Format("%Y%m%d");
		OutputString += ParseANSIField(str,8,8);

		//GS05	337			Time									M	TM	4/8

		//current time HHMM
		str = dt.Format("%H%M");
		OutputString += ParseANSIField(str,4,8);

		//GS06	28			Group Control Number					M	N0	1/9

		//m_strBatchNumber

		//in this instance, trim out leading zeros
		m_strBatchNumber.TrimLeft("0");

		OutputString += ParseANSIField(m_strBatchNumber,1,9);

		//GS07	455			Responsible Agency Code					M	ID	1/2

		//static "X"
		OutputString += ParseANSIField("X",1,2);

		//GS08	480			Version/Release/Industry Ident. Code	M	AN	1/12

		//004010X098A1 - HCFA, Professional
		//004010X096A1 - UB92, Institutional

		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
		if(m_actClaimType != actInst)
			str = "004010X098A1";
		else
			str = "004010X096A1";
		
		OutputString += ParseANSIField(str,1,12);
		
		EndANSISegment(OutputString);

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_FunctionalGroupHeader");

	return Error_Other;

}

int CEbilling::ANSI_4010_FunctionalGroupTrailer() {

	//Functional Group Trailer (page B.10)

#ifdef _DEBUG

	CString str;

	str = "\r\nFunctional Group Trailer\r\n";
	m_OutputFile.Write(str,str.GetLength());
	
#endif

	try {

		CString OutputString,str;
		_variant_t var;

		//Ref.	Data		Name									Attributes
		//Des.	Element

		OutputString = "GE";

		//GE01	97			Number of Transaction Sets Included		M	N0	1/6

		//JJ - I don't foresee us supporting multiple transaction sets, though it's
		//probably more likely than using multiple functional groups.
		//If we do use them, however, we must make this another member counter.

		//static "1"
		OutputString += ParseANSIField("1",1,6);

		//GE02	28			Group Control Number					M	N0	1/9

		//m_strBatchNumber

		//in this instance, trim out leading zeros
		m_strBatchNumber.TrimLeft("0");

		OutputString += ParseANSIField(m_strBatchNumber,1,9);

		EndANSISegment(OutputString);

	return Success;

	} NxCatchAll("Error in Ebilling::ANSI_4010_FunctionalGroupTrailer");

	return Error_Other;

}

// (j.jones 2007-01-30 09:05) - PLID 24411 - allowed ability to separate by location
// (j.jones 2008-05-02 09:48) - PLID 27478 - now we are given an array so the calling function
// can track which qualifiers we used
void CEbilling::ANSI_4010_Output2010AAProviderIDs(long nProviderID, long nLocationID, CStringArray &arystrQualifiers)
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
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
				"LEFT JOIN ProvidersT ChargeProvidersT ON ChargesT.DoctorsProviders = ChargeProvidersT.PersonID "
				"LEFT JOIN ProvidersT PatientProvidersT ON PatientsT.MainPhysician = PatientProvidersT.PersonID "
				"LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
				"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
				"LEFT JOIN UB92SetupT ON InsuranceCoT.UB92SetupGroupID = UB92SetupT.ID "
				"LEFT JOIN HCFAClaimProvidersT ON InsuranceCoT.PersonID = HCFAClaimProvidersT.InsuranceCoID AND (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.PersonID ELSE ChargeProvidersT.PersonID END) = HCFAClaimProvidersT.ProviderID "

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
			// (j.jones 2010-10-18 15:59) - PLID 40346 - changed HCFA/UB boolean to an enum
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

		//REF01	128			Reference Identification Qualifier		M	ID	2/3

		m_strLast2010AAQual = ebProvID->strIdent;

		OutputString += ParseANSIField(ebProvID->strIdent,2,3);

		//REF02	127			Reference Identification				X	AN	1/30

		OutputString += ParseANSIField(ebProvID->strID,1,30);

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
void CEbilling::ANSI_4010_OutputCASSegments(CArray<ANSI_CAS_Info*, ANSI_CAS_Info*> &aryCASSegments)
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

		//CAS01	1033		Claim Adjustment Group Code				M	ID	1/2
		OutputString += ParseANSIField(pInfo->strGroupCode, 1, 2);

		if(pInfo->aryDetails.GetSize() == 0) {
			//this should be impossible
			ASSERT(FALSE);
			continue;
		}

		for(j=0; j<pInfo->aryDetails.GetSize(); j++) {
			ANSI_CAS_Detail* pDetail = (ANSI_CAS_Detail*)pInfo->aryDetails.GetAt(j);

			//CAS02	1034		Claim Adjustment Reason Code			M	ID	1/5
			OutputString += ParseANSIField(pDetail->strReasonCode, 1, 5);

			//CAS03	782			Monetary Amount							M	R	1/18
			str = FormatCurrencyForInterface(pDetail->cyAmount, FALSE, FALSE);
			//see if we need to trim the right zeros
			if(m_bTruncateCurrency) {
				str.TrimRight("0");	//first the zeros
				str.TrimRight(".");	//then the decimal, if necessary
			}
			OutputString += ParseANSIField(str, 1, 18);

			//CAS04	380			Quantity								O	R	1/15
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