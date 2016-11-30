
////////////////
// DRT 8/6/03 - GetSqlPrintPreview() function from ReportInfoCallback
//

#include "stdafx.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "DateTimeUtils.h"
#include "SchedulerView.h"
#include "GlobalFinancialUtils.h"
#include "InternationalUtils.h"
#include "EmrUtils.h"
#include "MarketUtils.h"
#include "invutils.h"
#include "GlobalLabUtils.h"
#include "LabCustomField.h"
#include "HL7ParseUtils.h"
#include "HL7Utils.h"
#include <NxHL7Lib/HL7SettingsCache.h>

//Helper function for the Consult to Procedure preview reports. Takes the SQL query and passed in extra values array
//and replaces the placeholder strings with the filter values.
void ApplyConsultToProcedurePPFilters(CString& strSql, const CStringArray& saExtraValues)
{
	//(e.lally 2009-08-28) PLID 35331 - Apply the filters that were sent in as extra values.
	CString strPatientLWFilter, strPatientFilterDateFrom, strPatientFilterDateTo, strPatientLocation;
	CString strPatientProvider, strConsultDateFrom, strConsultDateTo;
	CString strConsultLocation, strConsultResource;
	CString strConsultProcedure;
	CString strConsultTypeList1 = "-1", strConsultTypeList2 = "-1", strSurgeryTypeList = "-1";
	CString strReferralDateFrom, strReferralDateTo;
	CString strPatPrimaryReferralSource, strPatMultiReferralSource;

	if(saExtraValues.GetSize() == 17){
		//(e.lally 2009-08-28) PLID 35331 - We have to get these out in the same order they were set since all we get
			//is this CStringArray to use.
			//See MarketView::OnPreparePrinting and MarketUtils::BuildDocbarFilterArray
		strPatientLWFilter = saExtraValues[0];
		strPatientFilterDateFrom = saExtraValues[1];
		strPatientFilterDateTo = saExtraValues[2];
		strPatientLocation = saExtraValues[3];
		strPatientProvider = saExtraValues[4];
		strConsultDateFrom = saExtraValues[5];
		strConsultDateTo = saExtraValues[6];
		strConsultLocation = saExtraValues[7];
		strConsultResource = saExtraValues[8];
		strConsultTypeList1 = saExtraValues[9];
		strConsultTypeList2 = saExtraValues[10];
		strSurgeryTypeList = saExtraValues[11];
		//(e.lally 2009-09-11) PLID 35521 - Added filter for procedure filtering
		strConsultProcedure = saExtraValues[12];
		//(e.lally 2009-09-24) PLID 35592 - Added multi referral date filters
		strReferralDateFrom = saExtraValues[13];
		strReferralDateTo = saExtraValues[14];
		//(e.lally 2009-09-28) PLID 35594 - Added primary referral and multi referral source filters
		strPatPrimaryReferralSource = saExtraValues[15];
		strPatMultiReferralSource = saExtraValues[16];
	}
	else if(saExtraValues.GetSize() != 0){
		//The expected number of filters were not passed in. Why not?
		ASSERT(FALSE);
	}

	strSql.Replace("[PatientLWFilter]", strPatientLWFilter);
	strSql.Replace("[PatientFilterDateFrom]", strPatientFilterDateFrom);
	strSql.Replace("[PatientFilterDateTo]", strPatientFilterDateTo);
	strSql.Replace("[PatientLocation]", strPatientLocation);
	strSql.Replace("[PatientProvider]", strPatientProvider);
	strSql.Replace("[ConsultDateFrom]", strConsultDateFrom);
	strSql.Replace("[ConsultDateTo]", strConsultDateTo);
	strSql.Replace("[ConsultLocation]", strConsultLocation);
	strSql.Replace("[ConsultResource]", strConsultResource);

	strSql.Replace(CONSULT_TYPE_PLACEHOLDER_1, strConsultTypeList1);
	strSql.Replace(CONSULT_TYPE_PLACEHOLDER_2, strConsultTypeList2);
	strSql.Replace(SURGERY_TYPE_PLACEHOLDER, strSurgeryTypeList);

	//(e.lally 2009-09-11) PLID 35521 - Added filter for procedure filtering
	strSql.Replace(PROCEDURE_FILTER_PLACEHOLDER, strConsultProcedure);

	//(e.lally 2009-09-24) PLID 35592 - Added multi referral date filters
	strSql.Replace("[MultiReferralDateFrom]", strReferralDateFrom);
	strSql.Replace("[MultiReferralDateTo]", strReferralDateTo);

	//(e.lally 2009-09-28) PLID 35594 - Added primary referral and multi referral source filters
	strSql.Replace(PRIMARY_REFERRAL_FILTER_PLACEHOLDER, strPatPrimaryReferralSource);
	strSql.Replace(MULTI_REFERRAL_FILTER_PLACEHOLDER, strPatMultiReferralSource);
}
//(a.wilson PLID 43338 - generic function to generate table insert for lab reports.
CString GenerateSignatureTableQuery(ADODB::_RecordsetPtr prsLabSignature)
{
	// (a.wilson 2011-9-27) PLID 43338 - adding signeddate to the report
	CString strImageInsert = "";
	COleDateTime dtNull;
	dtNull.SetStatus(COleDateTime::null);
	COleDateTime dtSigned(AdoFldDateTime(prsLabSignature, "SignedDate", dtNull));
	CString strImageFile = AdoFldString(prsLabSignature->GetFields(), "SignatureImageFile", "");
	_variant_t varInk = prsLabSignature->GetFields()->GetItem("SignatureInkData")->Value;
	// (j.jones 2010-04-15 12:23) - PLID 38222 - supported stamps as well
	_variant_t varTextData = prsLabSignature->GetFields()->GetItem("SignatureTextData")->Value;
	if (!strImageFile.IsEmpty())
	{
		// (a.walling 2010-01-05 09:23) - PLID 33887 - Prepend the images path if this is relative
		if (strImageFile.FindOneOf("\\/") == -1) { // does not contain a path separator
			// so include our images path
			CString strBasePath = GetSharedPath() ^ "Images";
			strImageFile = strBasePath ^ strImageFile;
		}

		// (z.manning 2008-11-26 17:34) - PLID 32125 - Load the contents of the image file
		HBITMAP hImage = NULL;
		if(NxGdi::LoadImageFile(strImageFile, hImage))
		{
			const long nLabID = AdoFldLong(prsLabSignature->GetFields(), "ID");

			// (z.manning 2008-11-26 17:34) - PLID 32125 - Draw the ink data on the bitmap
			// (j.jones 2010-04-15 12:02) - PLID 38222 - supported stamps as well
			DrawInkOnBitmap(varInk, varTextData, hImage);

			// (z.manning 2008-11-26 17:35) - PLID 32125 - Now save the contents of the bitmap
			// to a stream to beging the process of getting a valid byte stream to store in
			// the database.
			// (z.manning 2008-11-26 17:36) - PLID 32125 - Note: Crystal does not seem to accept
			// jpeg's for this (or at least not our version of it). PNG files worked, but the quality
			// was noticably worse than BMP so since we're only storing these bitmaps very
			// temporily in a table variable I decided to just use BMP.
			IStreamPtr pImageStream;									
			// (a.walling 2009-03-09 13:46) - no PLID - This is confusing intellisense, thinking all BOOLs are Gdiplus::BOOLs, etc
			// this was using NxGdi:: rather than Gdiplus:: to refer to the Bitmap
			Gdiplus::Bitmap *pBitmap = Gdiplus::Bitmap::FromHBITMAP(hImage, NULL);
			if(pBitmap != NULL && NxGdi::SaveToBMPStream(pBitmap, pImageStream) == Gdiplus::Ok)
			{
				// (z.manning 2008-11-26 17:36) - PLID 32125 - Seek back to the beginning
				// of the bitmap stream.
				LARGE_INTEGER dlibMove;
				dlibMove.HighPart = dlibMove.LowPart = 0;
				dlibMove.QuadPart = 0;
				pImageStream->Seek(dlibMove, STREAM_SEEK_SET, NULL);
				// (z.manning 2008-11-26 17:38) - PLID 32125 - Get the size of the bitmap in the stream.
				STATSTG stats;
				pImageStream->Stat(&stats, STATFLAG_NONAME);
				// (z.manning 2008-11-26 17:39) - PLID 32125 - Everywhere that we'll be using
				// size only takes a max of 2^32 bytes, so if the image is > 4 GB then taking
				// just the low part here will fail, but I'm willing to take that chance.
				DWORD dwSize = stats.cbSize.LowPart;
				BYTE *pImageBytes = NULL;
				COleSafeArray sa;
				// (z.manning 2008-11-26 17:40) - PLID 32125 - Create a variant array of bytes
				// to store the bitmap's bytes.
				sa.CreateOneDim(VT_UI1, dwSize);
				sa.AccessData((LPVOID*)&pImageBytes);
				// (z.manning 2008-11-26 17:41) - PLID 32125 - Read the bitmap into our
				// byte array that's attached to our variant array.
				ULONG nBytesRead;
				pImageStream->Read(pImageBytes, dwSize, &nBytesRead);
				ASSERT(nBytesRead == dwSize);
				sa.UnaccessData();
				delete pBitmap;

				// (z.manning 2008-11-26 17:42) - PLID 32125 - Now generate the SQL
				// to insert these images into a table variable that is declared
				//(a.wilson 2011-9-28) PLID 43338 - insert signeddate to the report
				// as part of the query for this report.
				strImageInsert = FormatString(
					"INSERT INTO @LabImagesT (LabID, SignatureImage, SignedDate) VALUES (%li, %s, '%s') \r\n", 
					nLabID, CreateByteStringFromSafeArrayVariant(sa.Detach()), FormatDateTimeForSql(dtSigned, dtoDateTime));
			}

			if(hImage != NULL) {
				DeleteObject(hImage);
			}
		}
	}
	return strImageInsert;
}

CString CReportInfo::GetSqlPrintPreview(long nSubLevel, long nSubRepNum) const
{
	CString strSQL, strArSql, strPatientID;

	// (f.dinatale 2010-10-15) - PLID 40876 - SSN Masking Permissions
	BOOL bSSNReadPermission = CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE);
	BOOL bSSNDisableMasking = CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE);

	switch (nID) {
	

	case 380:	//Daily Schedule - special case report
		return GetSqlDailySchedule(GetRemotePropertyInt("ClassicReptShowCancelledAppts", 0, 0, GetCurrentUserName()) != 0, nSubLevel, nSubRepNum);
		break;

	case 221:
		//Contact Notes (Print Preview)
		return _T("SELECT Notes.PersonID as PersonID, Notes.Date AS Date, Notes.Note,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, "
		"     NoteCatsF.Description AS Category, Notes.Category AS CatID "
		"FROM Notes LEFT JOIN NoteCatsF ON Notes.Category = NoteCatsF.ID LEFT JOIN PersonT ON Notes.PersonID = PersonT.ID "
		"WHERE (Notes.PersonID IS NOT NULL)");
		break;

	case 240:
			//Patient Notes
		//TES 10/29/03:  This report was changed a while back to use the same .rpt file as the
		//Patient Notes report from the Reports module.  Therefore, they need to use the same query.
		return GetSqlPatients(nSubLevel, nSubRepNum);
		break;
			/*return _T("SELECT PersonT.First, PersonT.Middle, PersonT.Last,   "
			"    PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,   "
			"    Notes.Date AS Date, Notes.Date AS Time, NoteCatsF.Description,  Notes.Note, LocationsT.Name AS Practice, LocationsT.ID AS LocID,  "
			"    PatientsT.MainPhysician AS ProvID, Notes.Category  "
			"FROM PersonT LEFT OUTER JOIN  "
			"    LocationsT ON   "
			"    PersonT.Location = LocationsT.ID RIGHT OUTER JOIN  "
			"    PatientsT ON   "
			"    PersonT.ID = PatientsT.PersonID LEFT OUTER JOIN  "
			"    Notes ON PatientsT.PersonID = Notes.PersonID LEFT JOIN "
			"    NoteCatsF ON Notes.Category = NoteCatsF.ID "
			"WHERE (PatientsT.PersonID > 0) AND (Notes.Note IS NOT NULL) ");
			break;*/
	


	case 231:
		//Patient Financial History (PP)
		/*	Version History
			DRT 3/13/03 - Added TopDate filter, and ability to filter on dates.  Also added a parameter that is sent in showing the name 
					of the location chosen when previewing.
			// (j.gruber 2007-05-29 14:31) - PLID 25979 - added discounts and discount categories
			// (j.gruber 2009-04-02 10:58) - PLID 33814 - update discount category strucutre
			// (j.gruber 2011-09-09 11:08) - PLID 45411 - fix voided pays not showing
		*/
		strSQL = _T("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
		"   PatientsT.UserDefinedID,  "
		"   FinHistorySubQ.TopID,  "
		"   FinHistorySubQ.MiddleID,  "
		"   FinHistorySubQ.ItemLevel, "
		"   FinHistorySubQ.PatID AS PatID, "
		"   FinHistorySubQ.TDate AS TDate,  "
		"   FinHistorySubQ.Description,  "
		"   Coalesce(FinHistorySubQ.ChargeAmount, 0) AS ChargeAmount, "
		"   Coalesce(FinHistorySubQ.PayAmount, 0) AS PayAmount,  "
		"   Coalesce(FinHistorySubQ.AdjAmount, 0) AS AdjAmount,  "
		"   Coalesce(FinHistorySubQ.RefAmount, 0) AS RefAmount, "
		"   FinHistorySubQ.Type, "
		"   FinHistorySubQ.TopDate AS TopDate,  "
		"   FinHistorySubQ.ProvId, "
		"	FinHistorySubQ.ProvFirst, "
		"   FinHistorySubq.ProvMiddle, "
		"	FinHistorySubq.ProvLast,		"
		"	FinHistorySubQ.ProvTitle, "
		"	PersonT.ID AS PatID,  "
		"   FinHistorySubQ.PercentOff as PercentOff, "
		"   FinHistorySubQ.DiscountAmt as DiscountAmt, "
		"   FinHistorySubQ.DiscountCategoryDescription "

		"FROM (PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID) INNER JOIN ( "
		" "
		"   /*TS: TopID: ID of the top-level lineitem with which this is associated "
		"         MiddleID:  ID of the second-level LineItem (charge) with which this is associated (-1 for top level) "
		"         ItemLevel: Level of this item (1, 2, or 3)*/ "
		"    "
		"   /*Top level items (Bills, and payments with unapplied amounts remaining*/ "
		"    "
		"   /*Payments, Adjustments and Refunds with unapplied amounts remaining*/ "
		"   SELECT LineItemT.ID AS TopID,  "
		"      -1 AS MiddleID,  "
		"      1 AS ItemLevel,  "
		"      LineItemT.PatientID AS PatID,  "
		"      LineItemT.Date AS TDate,  "
		"      LineItemT.Date AS TopDate, "
		"      CASE WHEN LineItemT.Type = 1 THEN 'Payment - ' ELSE CASE WHEN LineItemT.Type = 2 THEN 'Adjustment - ' ELSE 'Refund - ' END END + LineItemT.Description AS Description,  "
		"      Convert(money, 0) AS ChargeAmount,  "
		"      CASE WHEN LineItemT.Type = 1 THEN LineItemT.Amount - CASE WHEN AppliesFromSubQ.Amount Is Null THEN 0 ELSE AppliesFromSubQ.Amount END ELSE 0 END AS PayAmount, "
		"      CASE WHEN LineItemT.Type = 2 THEN LineItemT.Amount - CASE WHEN AppliesFromSubQ.Amount Is Null THEN 0 ELSE AppliesFromSubQ.Amount END ELSE 0 END AS AdjAmount, "
		"      CASE WHEN LineItemT.Type = 3 THEN LineItemT.Amount - CASE WHEN AppliesFromSubQ.Amount Is Null THEN 0 ELSE AppliesFromSubQ.Amount END ELSE 0 END AS RefAmount, "
		"      LineItemT.Type,	"
		"	   Providerst.ID AS ProvID,	"
		"	   ProvidersT.First AS ProvFirst,  "
		"	   ProvidersT.Middle AS ProvMiddle, "
		"	   ProvidersT.last AS ProvLast,	"
		"	   ProvidersT.Title AS ProvTitle, "
		"	   LineItemT.ID As GroupFixId,  "
		"      0 as PercentOff, "
		"      Convert(money, 0) AS DiscountAmt, "
		"      '' AS DiscountCategoryDescription "
		"   FROM LineItemT LEFT JOIN ( "
		"         SELECT Sum(AppliesT.Amount) AS Amount,  "
		"            AppliesT.SourceID  "
		"         FROM AppliesT  "
		"         GROUP BY AppliesT.SourceID) AS AppliesFromSubQ ON LineItemT.ID = AppliesFromSubQ.SourceID "
		"	Left Join PaymentsT ON LineItemT.ID = PaymentsT.ID Left Join(SELECT ID, First, Last, Middle, Title  FROM PersonT WHERE ID IN (SELECT PersonID FROM ProvidersT)) AS ProvidersT ON  "
		"	PaymentsT.ProviderID = ProvidersT.ID  "
		"   WHERE LineItemT.PatientID = @PatientID@ AND (LineItemT.Amount = 0 OR (LineItemT.Amount - CASE WHEN AppliesFromSubQ.Amount Is Null THEN 0 ELSE AppliesFromSubQ.Amount END <> 0)) AND (LineItemT.Type = 1 OR LineItemT.Type = 2 OR LineItemT.Type = 3) AND LineItemT.Deleted = 0 "
		"    "
		"   /*Bills, Bills, Bills*/ "
		"   UNION SELECT BillsT.ID AS TopID,  "
		"      -1 AS MiddleID,  "
		"      1 AS ItemLevel,  "
		"      BillsT.PatientID AS PatID,  "
		"      BillsT.Date AS TDate, "
		"      BillsT.Date AS TopDate, "
		"      'BILL - ' + BillsT.Description AS Description, "
		"      0 AS ChargeAmount, "
		"      0 AS PayAmount,  "
		"      0 AS AdjAmount, "
		"      0 AS RefAmount, "
		"      0 AS Type, "
		"	   0 AS ProvID,	"
		"	   '' AS ProvFirst,  "
		"	   '' AS ProvMiddle, "
		"	   '' AS ProvLast,	"
		"	   '' AS ProvTitle,  "
		"	   BillsT.Id AS GroupFixID,  "
		"      0 as PercentOff, "
		"      Convert(money, 0) AS DiscountAmt, "
		"      '' AS DiscountCategoryDescription "
		"   FROM BillsT "
		"   WHERE BillsT.PatientID = @PatientID@ AND BillsT.Deleted = 0 AND BillsT.EntryType = 1 "
		"    "
		"   /*Second level:  Charges, and refunds/adjustments applied to payments*/ "
		"    "
		"   /*Charges*/ "
		"   UNION SELECT ChargesT.BillID AS TopID, "
		"      LineItemT.ID AS MiddleID,  "
		"      2 AS ItemLevel, "
		"      LineItemT.PatientID AS PatID, "
		"      LineItemT.Date AS TDate, "
		"      (SELECT Date FROM BillsT WHERE ID = ChargesT.BillID) AS TopDate, "
		"      '   Charge - (' + ChargesT.ItemCode + ') ' + LineItemT.Description AS Description,  "
		"      dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount,   "
		"      0 AS PayAmount, "
		"      0 AS AdjAmount,  "
		"      0 AS RefAmount, "
		"      LineItemT.Type, "
		"	   Providerst.ID AS ProvID,	"
		"	   ProvidersT.First AS ProvFirst,  "
		"	   ProvidersT.Middle AS ProvMiddle, "
		"	   ProvidersT.last AS ProvLast,	"
		"	   ProvidersT.Title AS ProvTitle, "
		"	   LineItemT.ID AS GroupFixID,  "
		"      TotalPercentOff, "
		"      TotalDiscount, "
		"	dbo.GetChargeDiscountList(ChargesT.ID) AS DiscountCategoryDescription "

		"   FROM LineItemT INNER JOIN (ChargesT LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) ON LineItemT.ID = ChargesT.ID "
		"	LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
		"	LEFT JOIN (SELECT ID, first, Middle, Last, Title FROM PersonT WHERE ID IN (SELECT ID FROM ProvidersT)) AS ProvidersT ON    "
		"	ChargesT.DoctorsProviders = ProvidersT.ID  "
		"   WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND LineItemT.PatientID = @PatientID@ "
		"    "
		"   /*Refunds/Adjustments applied to payments*/ "
		"   UNION SELECT AppliesT.DestID AS TopID, "
		"      LineItemT.ID AS MiddleID, "
		"      2 AS ItemLevel,  "
		"      LineItemT.PatientID AS PatID, "
		"      LineItemT.Date AS TDate,  "
		"      (SELECT Date FROM LineItemT PayItems WHERE PayItems.ID = AppliesT.DestID) AS TopDate, "
		"      CASE WHEN LineItemT.Type = 1 THEN '   Payment - ' WHEN LineItemT.Type = 2 THEN '   Adjustment - ' ELSE '   Refund - ' END + LineItemT.Description AS Description,  "
		"      0 AS ChargeAmount,  "
		"	   CASE WHEN LineItemT.Type = 1 THEN AppliesT.Amount ELSE 0 END AS PayAmount,  "
		"      CASE WHEN LineItemT.Type = 2 THEN AppliesT.Amount ELSE 0 END AS AdjAmount,  "
		"      CASE WHEN LineItemT.Type = 3 THEN AppliesT.Amount ELSE 0 END AS RefAmount, "
		"      LineItemT.Type, "
		"	   Providerst.ID AS ProvID,	"
		"	   ProvidersT.First AS ProvFirst,  "
		"	   ProvidersT.Middle AS ProvMiddle, "
		"	   ProvidersT.last AS ProvLast,	"
		"	   ProvidersT.Title AS ProvTitle, "
		"	   AppliesT.ID As FixGroupID,  "
		"      0 as PercentOff, "
		"      Convert(money, 0) AS DiscountAmt, "
		"      '' AS DiscountCategoryDescription "
		"   FROM LineItemT INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
		"   INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"   LEFT JOIN (SELECT ID, First, Middle, Last, Title FROM PersonT WHERE ID IN (SELECT ID FROM ProvidersT)) AS ProvidersT ON   "
		"   PaymentsT.ProviderId = ProvidersT.ID   "
		"   WHERE (LineItemT.Type IN (1,2,3)) AND LineItemT.Deleted = 0 AND AppliesT.PointsToPayments = 1 AND LineItemT.PatientID = @PatientID@ "
		"       "
		"   /*Third Level:  Payments/Refunds/Adjustments applied to Charges*/ "
		"   UNION SELECT ChargesT.BillID AS TopID,  "
		"      ChargesT.ID AS MiddleID,  "
		"      3 AS ItemLevel, "
		"      LineItemT.PatientID AS PatID, "
		"      LineItemT.Date AS TDate, "
		"      (SELECT Date FROM BillsT WHERE BillsT.ID = ChargesT.BillID) AS TopDate, "
		"      '      ' + CASE WHEN InsuredPartyT.RespTypeID = 1 THEN '[Primary]' ELSE CASE WHEN InsuredPartyT.RespTypeID = 2 THEN '[Secondary]' ELSE CASE WHEN InsuredPartyT.RespTypeID = -1 THEN '[Inactive]' ELSE '' END END END + CASE WHEN LineItemT.Type = 1 THEN 'Payment - ' ELSE CASE WHEN LineItemT.Type = 2 THEN 'Adjustment - ' ELSE 'Refund - ' END END + LineItemT.Description AS Description, "
		"      0 AS ChargeAmount, "
		"      CASE WHEN LineItemT.Type = 1 THEN AppliesT.Amount ELSE 0 END AS PayAmount,  "
		"      CASE WHEN LineItemT.Type = 2 THEN AppliesT.Amount ELSE 0 END AS AdjAmount,  "
		"      CASE WHEN LineItemT.Type = 3 THEN AppliesT.Amount ELSE 0 END AS RefAmount, "
		"      LineItemT.Type, "
		"	   Providerst.ID AS ProvID,	"
		"	   ProvidersT.First AS ProvFirst,  "
		"	   ProvidersT.Middle AS ProvMiddle, "
		"	   ProvidersT.last AS ProvLast,	"
		"	   ProvidersT.Title AS ProvTitle, "
		"	   AppliesT.ID As GroupFixId,  " 
		"      0 as PercentOff, "
		"      Convert(money, 0) AS DiscountAmt, "
		"      '' AS DiscountCategoryDescription "
		"   FROM ((LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID) LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID) INNER JOIN (AppliesT INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID) ON LineItemT.ID = AppliesT.SourceID "
		"	LEFT JOIN (SELECT ID, First, Middle, Last, Title FROM PErsonT WHERE ID IN (SELECT ID From ProvidersT)) AS ProvidersT "
		"	ON PaymentsT.ProviderID = ProvidersT.ID  "
		"   WHERE LineItemT.Deleted = 0 AND AppliesT.PointsToPayments = 0 AND LineItemT.PatientID = @PatientID@ "
		") AS FinHistorySubQ ON PersonT.ID = FinHistorySubQ.PatID");
		strPatientID.Format("%li", nPatient);
		strSQL.Replace("@PatientID@", strPatientID);
		return strSQL;
		break;


	case 222:
		//Patient Appointments (Print Preview)
		/*	Version History
			DRT 6/19/03 - Removed references to AptPurposeID, which is obsolete.
			TES 8/4/03 - Included EndTime
			TES 3/4/04 - Implemented multi-resource support(!)
		*/
		return _T("SELECT AppointmentsT.StartTime, AppointmentsT.EndTime, "
		"    PersonT.First, PersonT.Middle, PersonT.Last,  "
		"    PersonT.Last + ', ' + PersonT.Middle + ' ' + PersonT.First AS PatName, "
		"    AppointmentsT.Date AS Date, "
		"    AppointmentsT.Notes, AptTypeT.Name, dbo.GetResourceString(AppointmentsT.ID) AS Item,  "
		"    AppointmentsT.AptTypeID, AppointmentsT.Status, PersonT.HomePhone,  "
		"    dbo.GetPurposeString(AppointmentsT.ID) AS Purpose,  "
		"    AptTypeT.ID, PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, PersonT.PrivHome, "
		"	 PersonT.PrivWork, AppointmentsT.Status, AppointmentsT.ShowState "
		"FROM PersonT INNER JOIN "
		"    PatientsT ON  "
		"    PersonT.ID = PatientsT.PersonID RIGHT OUTER JOIN "
		"    AppointmentsT ON  "
		"    PatientsT.PersonID = AppointmentsT.PatientID LEFT OUTER JOIN "
		"    AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
		"WHERE (PatientsT.PersonID > 0)");
		break;

	case 223:
		//Patient General (Print Preview)
		/*	Version History
			DRT 7/30/03 - Added Privacy fields
			JMM 8/1/03 - add PCP name
			TES 1/6/04 - add coordinator.
			// (j.jones 2009-10-19 17:44) - PLID 35994 - split race and ethnicity
			// (j.dinatale 2010-08-30) - PLID 33266 - added primary and secondary insurance information
			// (f.dinatale 2010-10-18) - PLID 40876 - Added SSN Masking.
			// (j.gruber 2011-09-28 09:39) - PLID 45357 - add Affiliate Phys
			// (d.thompson 2012-08-09) - PLID 52045 - Reworked ethnicity table structure, switched to practice name
			// (b.savon 2014-03-24 07:52) - PLID 61477 - Modify Patient General (PP) for ICD-10.
		*/
		strSQL.Format("SELECT "
			"PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, PatientsT.CurrentStatus, PatientsT.MaritalStatus, "
			"PatientsT.SpouseName, PatientsT.Nickname, "
			"PatientsT.MainPhysician, PatientsT.Occupation, PersonT.Company, PatientsT.Employment, PatientsT.EmployerFirst, "
			"PatientsT.EmployerMiddle, PatientsT.EmployerLast, PatientsT.EmployerAddress1, PatientsT.EmployerAddress2, PatientsT.EmployerCity, "
			"PatientsT.EmployerState, PatientsT.EmployerZip, PatientsT.TypeOfPatient, PatientsT.DefaultInjuryDate, "
			" DiagCodes1.CodeNumber AS ICD9Code1, "
			" DiagCodes2.CodeNumber AS ICD9Code2,  "
			" DiagCodes3.CodeNumber AS ICD9Code3,  "
			" DiagCodes4.CodeNumber AS ICD9Code4,  "
			" DiagCodes1_ICD10.CodeNumber AS ICD10Code1, "
			" DiagCodes2_ICD10.CodeNumber AS ICD10Code2,  "
			" DiagCodes3_ICD10.CodeNumber AS ICD10Code3,  "
			" DiagCodes4_ICD10.CodeNumber AS ICD10Code4,  "
			"PersonT.EmergFirst, "
			"PersonT.EmergLast, PersonT.EmergRelation, PersonT.EmergHPhone, PersonT.EmergWPhone, PatientsT.InLocation, "
			"PersonT.Location, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, "
			"PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Gender, PrefixT.Prefix, PersonT.Suffix, "
			"PersonT.Title, PersonT.HomePhone, PersonT.WorkPhone, PersonT.Extension, "
			"PersonT.CellPhone, PersonT.OtherPhone, PersonT.Email, PersonT.Pager, PersonT.Fax, PersonT.BirthDate, "
			"dbo.MaskSSN(PersonT.SocialSecurity, %s) AS SocialSecurity, PersonT.FirstContactDate, PersonT.InputDate, UsersT.UserName, "
			"PersonT.WarningMessage, PersonT.Note, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS DocName, "
			"PersonT2.Last + ', ' + PersonT2.First + ' ' + PersonT2.Middle AS RefPhysName, "
			"LocationsT.Name AS Practice, GroupTypes.GroupName, ReferralSourceT.Name AS ReferralName, PersonT.PrivHome, PersonT.PrivWork, "
			"Custom1SubQ.Name AS Field1Name, Custom1SubQ.TextParam AS Field1, "
			"Custom2SubQ.Name AS Field2Name, Custom2SubQ.TextParam AS Field2, "
			"Custom3SubQ.Name AS Field3Name, Custom3SubQ.TextParam AS Field3, "
			"Custom4SubQ.Name AS Field4Name, Custom4SubQ.TextParam AS Field4, "
			"PersonT.PrivFax, PersonT.PrivOther, PersonT.PrivPager, PersonT.PrivEmail, "
			"PCPPersonT.Last + ', ' + PCPPersonT.First + ' ' + PCPPersonT.Middle AS PCPName, "
			"PersonCoord.Last + ', ' + PersonCoord.First + ' ' + PersonCoord.Middle AS CoordName, "
			"PersonT.PrivCell, "
			// (b.spivey, May 28, 2013) - PLID 56871
			"	LEFT(RaceSubQ.RaceName, LEN(RaceSubQ.RaceName) -1) AS Race, "
			"	LEFT(OfficialRaceSubQ.OfficialName, LEN(OfficialRaceSubQ.OfficialName) -1) AS CDCRace, "
			"EthnicityT.Name AS CDCEthnicity, "
			"PrimaryInsuredPartyT.Name AS PrimInsName, "
			"PrimaryInsuredPartyT.PlanName AS PrimInsPlanName, "
			"PrimaryInsuredPartyT.IDForInsurance AS PrimInsID, "
			"PrimaryInsuredPartyT.PolicyGroupNum AS PrimInsGroupNum, "
			"PrimaryInsuredPartyT.Last AS PrimInsInsuredPartyLastName, "
			"PrimaryInsuredPartyT.Middle As PrimInsInsuredPartyMiddleName, "
			"PrimaryInsuredPartyT.First AS PrimInsInsuredPartyFirstName, "
			"PrimaryInsuredPartyT.RelationToPatient AS PrimInsInsuredPartyRelationTo, "
			"CASE WHEN PrimaryInsuredPartyT.Gender = 1 THEN 'Male' WHEN PrimaryInsuredPartyT.Gender = 2 THEN 'Female' ELSE '' END AS PrimInsInsuredPartyGender, "
			"PrimaryInsuredPartyT.Address1 AS PrimInsInsuredPartyAddress1, "
			"PrimaryInsuredPartyT.Address2 AS PrimInsInsuredPartyAddress2, "
			"PrimaryInsuredPartyT.City AS PrimInsInsuredPartyCity, "
			"PrimaryInsuredPartyT.State AS PrimInsInsuredPartyState, "
			"PrimaryInsuredPartyT.Zip AS PrimInsInsuredPartyZip, "
			"dbo.MaskSSN(PrimaryInsuredPartyT.SocialSecurity, %s) AS PrimInsInsuredPartySocSec, "
			"PrimaryInsuredPartyT.Prefix AS PrimInsInsuredPartyTitle, "
			"PrimaryInsuredPartyT.BirthDate AS PrimInsInsuredPartyBirthDate, "
			"SecondaryInsuredPartyT.Name AS SecInsName, "
			"SecondaryInsuredPartyT.PlanName AS SecInsPlanName, "
			"SecondaryInsuredPartyT.IDForInsurance AS SecInsID, "
			"SecondaryInsuredPartyT.PolicyGroupNum AS SecInsGroupNum, "
			"SecondaryInsuredPartyT.Last AS SecInsInsuredPartyLastName, "
			"SecondaryInsuredPartyT.Middle As SecInsInsuredPartyMiddleName, "
			"SecondaryInsuredPartyT.First AS SecInsInsuredPartyFirstName, "
			"SecondaryInsuredPartyT.RelationToPatient AS SecInsInsuredPartyRelationTo, "
			"CASE WHEN SecondaryInsuredPartyT.Gender = 1 THEN 'Male' WHEN SecondaryInsuredPartyT.Gender = 2 THEN 'Female' ELSE '' END AS SecInsInsuredPartyGender, "
			"SecondaryInsuredPartyT.Address1 AS SecInsInsuredPartyAddress1, "
			"SecondaryInsuredPartyT.Address2 AS SecInsInsuredPartyAddress2, "
			"SecondaryInsuredPartyT.City AS SecInsInsuredPartyCity, "
			"SecondaryInsuredPartyT.State AS SecInsInsuredPartyState, "
			"SecondaryInsuredPartyT.Zip AS SecInsInsuredPartyZip, "
			"dbo.MaskSSN(SecondaryInsuredPartyT.SocialSecurity, %s) AS SecInsInsuredPartySocSec, "
			"SecondaryInsuredPartyT.Prefix AS SecInsInsuredPartyTitle, "
			"SecondaryInsuredPartyT.BirthDate AS SecInsInsuredPartyBirthDate, "
			"AffPersonT.Last + ', ' + AffPersonT.First + ' ' + AffPersonT.Middle AS AffiliatePhysName "
			"FROM "
			"(PersonT LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID)  "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			" LEFT JOIN DiagCodes DiagCodes1 ON PatientsT.DefaultDiagID1 = DiagCodes1.ID "
			" LEFT JOIN DiagCodes DiagCodes2 ON PatientsT.DefaultDiagID2 = DiagCodes2.ID "
			" LEFT JOIN DiagCodes DiagCodes3 ON PatientsT.DefaultDiagID3 = DiagCodes3.ID "
			" LEFT JOIN DiagCodes DiagCodes4 ON PatientsT.DefaultDiagID4 = DiagCodes4.ID "
			" LEFT JOIN DiagCodes DiagCodes1_ICD10 ON PatientsT.DefaultICD10DiagID1 = DiagCodes1_ICD10.ID "
			" LEFT JOIN DiagCodes DiagCodes2_ICD10 ON PatientsT.DefaultICD10DiagID2 = DiagCodes2_ICD10.ID "
			" LEFT JOIN DiagCodes DiagCodes3_ICD10 ON PatientsT.DefaultICD10DiagID3 = DiagCodes3_ICD10.ID "
			" LEFT JOIN DiagCodes DiagCodes4_ICD10 ON PatientsT.DefaultICD10DiagID4 = DiagCodes4_ICD10.ID "
			"LEFT JOIN UsersT ON PersonT.UserID = UsersT.PersonID  "
			"LEFT JOIN PersonT PersonT1 ON PatientsT.MainPhysician = PersonT1.ID "
			"LEFT JOIN PersonT PersonT2 ON PatientsT.DefaultReferringPhyID = PersonT2.ID "
			"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
			"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
			"LEFT JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex  "
			"LEFT JOIN PersonT PCPPersonT ON PatientsT.PCP = PCPPersonT.ID "
			"LEFT JOIN PersonT PersonCoord ON PatientsT.EmployeeID = PersonCoord.ID "
			// (b.spivey, May 28, 2013) - PLID 56871 - creates the list of CDC and custom races. 
			"	CROSS APPLY "
			"	( "
			"		SELECT ( "
			"			SELECT RT.Name + ', ' "
			"			FROM PersonRaceT PRT "
			"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID "
			"			WHERE PRT.PersonID = PersonT.ID "
			"			FOR XML PATH(''), TYPE "
			"		).value('/', 'nvarchar(max)') "
			"	) RaceSubQ (RaceName) "
			"	CROSS APPLY "
			"	( "
			"		SELECT ( " 
			"			SELECT RCT.OfficialRaceName + ', ' "
			"			FROM PersonRaceT PRT "
			"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID  "
			"			INNER JOIN RaceCodesT RCT ON RCT.ID = RT.RaceCodeID "
			"			WHERE PRT.PersonID = PersonT.ID  "
			"			FOR XML PATH(''), TYPE "
			"		).value('/', 'nvarchar(max)') "
			"	) OfficialRaceSubQ (OfficialName) "
			"LEFT JOIN EthnicityT ON PersonT.Ethnicity = EthnicityT.ID "
			"LEFT JOIN EthnicityCodesT ON EthnicityT.EthnicityCodeID = EthnicityCodesT.ID "
			"LEFT JOIN PersonT AffPersonT ON PatientsT.AffiliatePhysID = AffPersonT.ID "
			"/* custom stuff */ "
			"LEFT JOIN  "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam  "
			"FROM "
			"CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID  "
			"WHERE CustomFieldsT.ID = 1) Custom1SubQ ON PersonT.ID = Custom1SubQ.PersonID "
			"LEFT JOIN  "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam  "
			"FROM "
			"CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID  "
			"WHERE CustomFieldsT.ID = 2) Custom2SubQ ON PersonT.ID = Custom2SubQ.PersonID "
			"LEFT JOIN "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam  "
			"FROM "
			"CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID  "
			"WHERE CustomFieldsT.ID = 3) Custom3SubQ ON PersonT.ID = Custom3SubQ.PersonID "
			"LEFT JOIN "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam  "
			"FROM "
			"CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID  "
			"WHERE CustomFieldsT.ID = 4) Custom4SubQ ON PersonT.ID = Custom4SubQ.PersonID "
			"/* end custom stuff */ "
			"/* start insurance stuff */ "
			"LEFT JOIN (SELECT * FROM (SELECT PersonID AS InsuredID, InsPlan, IDForInsurance, PatientID, InsuranceCoID, RespTypeID, RelationToPatient, PolicyGroupNum FROM InsuredPartyT) AS InsuredPartyT "
			"INNER JOIN (SELECT ID AS TypeID, Priority FROM RespTypeT) AS RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.TypeID "
			"INNER JOIN (SELECT PersonID AS PID, Name FROM InsuranceCoT) AS InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PID "
			"INNER JOIN PersonT ON InsuredPartyT.InsuredID = PersonT.ID "
			"LEFT JOIN (SELECT ID as TitleID, Prefix FROM PrefixT) AS PrefixT ON PersonT.PrefixID = PrefixT.TitleID "
			"LEFT JOIN (SELECT ID AS PlanID, PlanName FROM InsurancePlansT) AS InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.PlanID WHERE Priority = 1 ) "
			"AS PrimaryInsuredPartyT ON PrimaryInsuredPartyT.PatientID = PatientsT.PersonID "
			"LEFT JOIN (SELECT * FROM (SELECT PersonID AS InsuredID, InsPlan, IDForInsurance, PatientID, InsuranceCoID, RespTypeID, RelationToPatient, PolicyGroupNum FROM InsuredPartyT) AS InsuredPartyT "
			"INNER JOIN (SELECT ID AS TypeID, Priority FROM RespTypeT) AS RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.TypeID "
			"INNER JOIN (SELECT PersonID AS PID, Name FROM InsuranceCoT) AS InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PID "
			"INNER JOIN PersonT ON InsuredPartyT.InsuredID = PersonT.ID "
			"LEFT JOIN (SELECT ID as TitleID, Prefix FROM PrefixT) AS PrefixT ON PersonT.PrefixID = PrefixT.TitleID "
			"LEFT JOIN (SELECT ID AS PlanID, PlanName FROM InsurancePlansT) AS InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.PlanID WHERE Priority = 2 ) "
			"AS SecondaryInsuredPartyT ON SecondaryInsuredPartyT.PatientID = PatientsT.PersonID "
			"/* end insurance stuff */ "
			"WHERE PersonT.ID > 0", 
			((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"), 
			((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"),
			((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"));
			return _T(strSQL);
		break;
	

	case 228:
		//Paper Claim List (PP)
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
		// (j.jones 2014-07-09 10:26) - PLID 62568 - on hold claims are hidden
		return _T(FormatString("SELECT PatientsT.UserDefinedID AS PatID, "
				"PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS PatName, "
				"BillsT.Date AS BillDate, "
				"BillsT.Description AS BillDesc, "
				"dbo.GetClaimTotal(BillsT.ID) AS BillAmount, HCFATrackT.CurrentSelect, BillsT.ID, BillsT.FormType, "
				"InsuranceCoT.Name AS InsCoName, RespTypeT.TypeName AS RespName, "
				"dbo.GetClaimProviderList(BillsT.ID) AS DocName, "

				"(SELECT Count(ChargesT.ID) FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"	WHERE Deleted = 0 "
				"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"	AND Batched = 1 AND ChargesT.BillID = BillsT.ID) AS CountOfChargesBatched, "

				"(SELECT Count(ChargesT.ID) FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"	WHERE Deleted = 0 "
				"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"	AND ChargesT.BillID = BillsT.ID) AS CountOfCharges "

				"FROM BillsT "
				"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
				"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN (PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID) ON BillsT.PatientID = PersonT.ID "
				"RIGHT JOIN HCFATrackT ON BillsT.ID = HCFATrackT.BillID "
				"WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND HCFATrackT.Batch = 1 "
				"AND Coalesce(BillStatusT.Type, -1) != %li "
				"GROUP BY PatientsT.UserDefinedID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle], BillsT.Date, BillsT.Description, HCFATrackT.CurrentSelect, BillsT.ID, BillsT.FormType, InsuranceCoT.Name, RespTypeT.TypeName "
				"ORDER BY PatName ", EBillStatusType::OnHold));
		break;

	case 230:
		//EBilling Batch List (PP)
		// (j.jones 2011-08-17 09:40) - PLID 44805 - we will filter out "original" and "void" charges
		// (j.jones 2012-03-26 10:17) - PLID 49197 - added claim provider field
		// (j.jones 2014-07-09 10:26) - PLID 62568 - on hold claims are hidden
		// (r.goldschmidt 2014-12-15 17:51) - PLID 40152 - added location id/name and last sent date
		return _T(FormatString("SELECT PatientsT.UserDefinedID AS PatID, "
				"PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS PatName, "
				"BillsT.Date AS BillDate, "
				"BillsT.Description AS BillDesc, "
				"dbo.GetClaimTotal(BillsT.ID) AS BillAmount, HCFATrackT.CurrentSelect, BillsT.ID, BillsT.FormType, "
				"InsuranceCoT.Name AS InsCoName, RespTypeT.TypeName AS RespName, "
				"dbo.GetClaimProviderList(BillsT.ID) AS DocName, "
				"dbo.GetClaim2010AAProviderList(BillsT.ID) AS ClaimProviderName, "
				"(SELECT Count(ChargesT.ID) FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"	WHERE Deleted = 0 AND Batched = 1 "
				"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"	AND ChargesT.BillID = BillsT.ID) AS CountOfChargesBatched, "
				"(SELECT Count(ChargesT.ID) FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"	WHERE Deleted = 0 "
				"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"	AND ChargesT.BillID = BillsT.ID) AS CountOfCharges, "
				"LocationsT.ID AS LocID, "
				"LocationsT.Name AS LocName, "
				"LastClaimQ.LastSentDate "
				"FROM HCFATrackT "
				"INNER JOIN BillsT ON HCFATrackT.BillID = BillsT.ID "
				"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
				"INNER JOIN(SELECT ID, BillID AS ChargesBillID FROM ChargesT "
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"	WHERE Batched = 1 "
				"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID IS NULL "
				"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID IS NULL) AS ChargesQ ON BillsT.ID = ChargesQ.ChargesBillID "
				"INNER JOIN LineItemT ON ChargesQ.ID = LineItemT.ID "
				"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
				"LEFT JOIN (SELECT Max(DATE) AS LastSentDate, BillID AS LastClaimBillID FROM ClaimHistoryT "
				"	WHERE SendType >= %li "
				"	GROUP BY BillID) AS LastClaimQ ON BillsT.ID = LastClaimQ.LastClaimBillID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
				"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND BillsT.EntryType = 1 AND HCFATrackT.Batch = 2 "
				"AND Coalesce(BillStatusT.Type, -1) != %li "
				"GROUP BY PatientsT.UserDefinedID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle], BillsT.Date, BillsT.Description, HCFATrackT.CurrentSelect, BillsT.ID, BillsT.FormType, InsuranceCoT.Name, RespTypeT.TypeName, LocationsT.ID, LocationsT.Name, LastClaimQ.LastSentDate "
				"ORDER BY PatName ", ClaimSendType::Electronic, EBillStatusType::OnHold));
		break;

	case 229: {
		//Deposit Slip
		/*	Version History
			DRT 7/25/03 - Made report editable, added the patient's SSN field in case they need some name-privacy (PLID 3326)
			DRT 3/2/2004 - PLID 11189 - Added the location name and provider of each line item to the query to be used in an 
					editable report.
			DRT 3/10/2004 - PLID 11256 - Added the tips to the report that prints out from deposit tab.
			JMM 5/18/2004 - PLID 12412 - Changed Tips to only separate them when they have a different payment type than the payment
			JMM 11/2/2005 - PLID 13548 - Added ability to have refunds on every type
			JMJ 10/11/2006 - PLID 22955 - Supported Adjusted Batch Payments by ignoring them
			// (j.gruber 2007-05-01 17:22) - PLID 25745 - only show the last 4 digits of the ccnumber
			(e.lally 2007-07-13) PLID 26649 - Replaced CCType with link to CardName, aliased as CCType.
			// (j.gruber 2007-08-10 09:21) - PLID 27023 - fixed problem with tips attached to $0 payments not showing
			// (j.jones 2008-05-12 09:31) - PLID 25338 - dropped CurrentSelect, the report now filters on passed-in IDs
			// (j.jones 2008-05-12 10:50) - PLID 30010 - properly handled cash tips, so that they always show separately from
			// their main cash payment - also made it so separated tips only will show/hide based on the 'BankingIncludeTips' setting
			(d.thompson 2009-07-06) - PLID 34791 - Added CC Processing data.  Not added to tips b/c they are always added with their
				parent.  If the parent is CC, they'll be run with that record.  If the parent is not CC, it's not possible to run.
			(d.thompson 2009-07-08) - PLID 17140 - Added PayCategoryID and PayCategoryName as available fields.  Not in the default report.
			// (f.dinatale 2010-10-18) - PLID 40876 - Added SSN masking.
			(d.thompson 2010-12-20) - PLID 41897 - Updated to also support Chase cc processing
			// (j.jones 2011-09-15 16:48) - PLID 45202 - we now hide void and corrected payments, only showing the originals
			// (b.spivey, July 24, 2012) - PLID 44450 - Added InputDate
			// (r.gonet 2015-05-04 16:36) - PLID 65870 - Exclude Gift Certificate Refunds here. They're not deposits.
			(c.haag 2015-09-09) - PLID 67194 - Supported Integrated Credit Card Processing. CardConnect_CreditTransactionT
			always has the original authorization so we join on that. Note: In the past, IsApproved would be NULL if the transaction
			was not approved. I'm keeping with historic behavior there.
		*/
		CString str;
		COleDateTime dtCurrent = COleDateTime::GetCurrentTime();
		COleDateTime dt = GetRemotePropertyDateTime("CurrentDepositDate",&dtCurrent,0,GetCurrentUserName(),TRUE);
		CString strRefunds, strBatchRefunds;
		if (GetRemotePropertyInt("BankingIncludeRefunds", 1, 0, GetCurrentUserName(), true)) {
			strRefunds = "OR (LineItemT.Type = 3 AND PaymentsT.PayMethod IN (7,8,9))";
			strBatchRefunds = "Type <> 2 AND ";
		}
		else {
			strRefunds = "";
			strBatchRefunds = "Type = 1 AND ";
		}
		
		BOOL bIncludeTips = GetRemotePropertyInt("BankingIncludeTips", 0, 0, GetCurrentUserName(), true);

		str.Format("SELECT * FROM (SELECT * FROM (SELECT "
		"/*Payment Info*/ "
		"PaymentsT.ID AS PaymentID, NULL AS TipID, NULL AS BatchPaymentID, "
		"PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"PaymentPlansT.CheckNo, CreditCardNamesT.CardName AS CCType, CASE WHEN Len(PaymentPlansT.CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(PaymentPlansT.CCNumber, 4) END  as CCNumber, "
		"LineItemT.Amount + (SELECT CASE WHEN Sum(Amount) IS NULL THEN 0 ELSE Sum(Amount) END FROM PaymentTipsT WHERE PaymentID = PaymentsT.ID AND PaymentTipsT.PayMethod = PaymentsT.PayMethod AND PaymentTipsT.PayMethod <> 1) AS Amount, "
		"PaymentsT.PayMethod, "
		"PaymentPlansT.BankNo AS BankName, PaymentPlansT.BankRoutingNum, PaymentPlansT.CheckAcctNo, "
		"dbo.MaskSSN(PersonT.SocialSecurity, %s) AS SocialSecurity, PaymentsT.DepositDate AS TDate, Convert(datetime, '%s') AS DepositDate, "
		"LocationsT.Name AS LocationName, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, "
		"CASE WHEN ISNULL( ISNULL( QBMS_CreditTransactionsT.ID, Chase_CreditTransactionsT.ID), CardConnect_CreditTransactionT.ID) IS NULL THEN 0 ELSE 1 END AS CCAttemptedProcessing, "
		"ISNULL( ISNULL( Chase_CreditTransactionsT.IsApproved, QBMS_CreditTransactionsT.IsApproved), CASE WHEN CardConnect_CreditTransactionT.ID IS NULL THEN NULL ELSE CONVERT(BIT,1) END) AS IsApproved, "
		"PaymentsT.PaymentGroupID AS PayCategoryID, PaymentGroupsT.GroupName AS PayCategoryName, "
		"LineItemT.InputDate AS InputDate	" 
		"FROM "
		"PatientsT LEFT OUTER JOIN LineItemT ON "
		"PatientsT.PersonID = LineItemT.PatientID INNER JOIN PaymentsT ON "
		"LineItemT.ID = PaymentsT.ID LEFT OUTER JOIN PaymentPlansT ON "
		"PaymentsT.ID = PaymentPlansT.ID INNER JOIN PersonT ON "
		"PatientsT.PersonID = PersonT.ID "
		"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
		"LEFT JOIN PersonT PersonProv ON PaymentsT.ProviderID = PersonProv.ID "
		"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
		"LEFT JOIN QBMS_CreditTransactionsT ON LineItemT.ID = QBMS_CreditTransactionsT.ID "
		"LEFT JOIN Chase_CreditTransactionsT ON LineItemT.ID = Chase_CreditTransactionsT.ID "
		"LEFT JOIN CardConnect_CreditTransactionT ON LineItemT.ID = CardConnect_CreditTransactionT.ID "
		"LEFT JOIN PaymentGroupsT ON PaymentsT.PaymentGroupID = PaymentGroupsT.ID "
		"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
		"LEFT JOIN LineItemCorrectionsT CorrectedLineItemsT ON LineItemT.ID = CorrectedLineItemsT.NewLineItemID "
		"WHERE (PatientsT.PersonID > 0) AND (PaymentsT.Deposited = 0) AND "
		"(LineItemT.Deleted = 0) AND (LineItemT.Type = 1 %s) "
		"AND CorrectedLineItemsT.NewLineItemID Is Null AND VoidingLineItemsT.VoidingLineItemID Is Null "
		"AND PaymentsT.BatchPaymentID Is Null AND PaymentsT.PayMethod NOT IN (4,10)) SubQ WHERE SubQ.Amount <> 0  "
		"UNION ALL SELECT "
		"/*BatchPaymentInfo */ "
		"NULL AS PaymentID, NULL AS TipID, BatchPaymentsT.ID AS BatchPaymentID, "
		"NULL AS PatID, NULL AS UserDefinedID, InsuranceCoT.Name, CheckNo, NULL AS CCType, NULL AS CCNumber, CASE WHEN Type <> 1 THEN -1 * Amount ELSE Amount END, CASE WHEN Type = 1 THEN 2 WHEN Type = 3 THEN 8 ELSE 0 END AS PayMethod, "
		"BankName, BankRoutingNum, CheckAcctNo, NULL AS SocialSecurity, DepositDate AS TDate, Convert(datetime, '%s') AS DepositDate, "
		"LocationsT.Name AS LocationName, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, "
		"0 AS CCAttemptedProcessing, 0 AS IsApproved, BatchPaymentsT.PayCatID AS PayCategoryID, PaymentGroupsT.GroupName AS PayCategoryName, "
		"NULL AS InputDate "
		"FROM BatchPaymentsT INNER JOIN InsuranceCoT ON BatchPaymentsT.InsuranceCoID = InsuranceCoT.PersonID "
		"LEFT JOIN LocationsT ON Location = LocationsT.ID "
		"LEFT JOIN PersonT PersonProv ON ProviderID = PersonProv.ID "
		"LEFT JOIN PaymentGroupsT ON BatchPaymentsT.PayCatID = PaymentGroupsT.ID "
		"WHERE Deleted = 0 AND Deposited = 0 AND %s Amount <> 0 "
		"/*Tips*/"
		"UNION ALL SELECT "
		"NULL AS PaymentID, PaymentTipsT.ID AS TipID, NULL AS BatchPaymentID, "
		"PersonT.ID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle + '  (Tip)' AS PatName, "
		"'' AS CheckNo, '' AS CCType, '' AS CCNumber, PaymentTipsT.Amount, "
		"	PaymentTipsT.PayMethod, "
		"'' AS BankName, '' AS RoutingNum, "
		"'' AS CheckAcctNo, dbo.MaskSSN(PersonT.SocialSecurity, %s), PaymentTipsT.DepositDate AS TDate, Convert(datetime, '%s') AS DepositDate, "
		"LocationsT.Name AS LocationName, PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, "
		"0 AS CCAttemptedProcessing, 0 AS IsApproved, NULL AS PayCategoryID, NULL AS PayCategoryName, LineItemT.InputDate AS InputDate "
		"FROM PaymentTipsT INNER JOIN PaymentsT ON PaymentTipsT.PaymentID = PaymentsT.ID "
		"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
		"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
		"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
		"LEFT JOIN PersonT PersonProv ON PaymentTipsT.ProvID = PersonProv.ID "
		"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
		"LEFT JOIN LineItemCorrectionsT CorrectedLineItemsT ON LineItemT.ID = CorrectedLineItemsT.NewLineItemID "
		"WHERE LineItemT.Deleted = 0 AND PaymentTipsT.Deposited = 0 "
		"AND (PaymentTipsT.PayMethod = 1 OR PaymentsT.PayMethod <> PaymentTipsT.PayMethod) %s "
		"AND CorrectedLineItemsT.NewLineItemID Is Null AND VoidingLineItemsT.VoidingLineItemID Is Null "
		") AS BatchPaySubQ "
		"%s ",
		((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"), 
		FormatDateTimeForSql(dt,dtoDate), strRefunds,
		FormatDateTimeForSql(dt,dtoDate), strBatchRefunds,
		((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"),
		FormatDateTimeForSql(dt,dtoDate), bIncludeTips ? "" : "AND PaymentTipsT.ID = -1",
		strExtraText);

		return _T(str);
		}
		break;

	case 245:
		// Order (PP)
		/* Version History
		   (j.jones 2007-02-20 17:30) - PLID 24544 - added fields UseUnitOfOrder, UnitOfOrderDesc, UnitOfUsageDesc, UOConversion
		   e.lally 2007-05-08 - PLID 24506 - Added barcode field (and formula)
		   // (j.gruber 2008-02-28 15:12) - PLID 29127 - added a bunch of fields including sold to/ship to, purchase order, contact name, payment information, and order destination type
		   // (j.jones 2008-05-27 09:59) - PLID 30167 - added SupplierT.AccountName
		   TES 5/27/2008 - PLID 29455 - Added UserDefinedID, PatientInitials, and ApptDate, all pulled from the linked appointment.
				NOTE: This assumes only one linked appointment, which, at the moment, is all the interface allows.
			// (j.jones 2008-06-19 15:48) - PLID 30446 - added PercentOff and Discount
			// (j.jones 2009-01-23 10:54) - PLID 32822 - added OrderMethod
			// (d.thompson 2009-01-23) - PLID 32823 - Added vendor confirmation fields
		*/
		return _T("SELECT "
		"/*Order Information*/ "
		"OrderT.ID, OrderT.Description, OrderT.TrackingID, LocationsT.Name, LocationsT.Address1, LocationsT.Address2, LocationsT.City, LocationsT.State, LocationsT.Zip, LocationsT.Phone, LocationsT.Fax, "
		"OrderT.Date, ((OrderT.Tax - 1.00) * 100) AS Tax, (CASE WHEN TaxShipping = 1 THEN OrderT.Tax * OrderT.ExtraCost ELSE OrderT.ExtraCost END) AS OrderExtraCost, OrderT.DateDue, OrderT.Notes, OrderDetailsT.Catalog, "
		"/*Supplier Information*/ "
		"PersonT.Company, PersonT.Address1 AS SupAddr1, PersonT.Address2 AS SupAddr2, PersonT.City AS SupCity, PersonT.State AS SupState, PersonT.Zip AS SupZip, SupplierT.CCNumber, PersonT.CompanyID, PersonT.Fax AS SupFax,  "
		"PersonT.WorkPhone, SupplierT.AccountName, "
		"/*Order Details Information*/ "
		"ServiceT.Name AS Product, ProductT.UnitDesc, (CASE WHEN OrderDetailsT.UseUU = 1 THEN Round(QuantityOrdered / Convert(float,OrderDetailsT.Conversion),0) ELSE QuantityOrdered END) AS QuantityOrdered, "
		"OrderDetailsT.DateReceived, OrderDetailsT.Amount, OrderDetailsT.ExtraCost, "
		"Round(Convert(money,((((OrderDetailsT.ExtraCost + ((CASE WHEN OrderDetailsT.UseUU = 1 THEN Convert(int,OrderDetailsT.QuantityOrdered / Convert(float,OrderDetailsT.Conversion)) ELSE OrderDetailsT.QuantityOrdered END) * OrderDetailsT.Amount)) * ((100-Convert(float,PercentOff))/100)) - Discount))),2) AS LineAmount, "
		"OrderDetailsT.PercentOff, OrderDetailsT.Discount, "
		"OrderDetailsT.UseUU AS UseUnitOfOrder, "
		"CASE WHEN OrderDetailsT.UseUU = 1 THEN ProductT.UnitOfOrder ELSE '' END AS UnitOfOrderDesc, "
		"CASE WHEN OrderDetailsT.UseUU = 1 THEN ProductT.UnitOfUsage ELSE '' END AS UnitOfUsageDesc, "
		"CASE WHEN OrderDetailsT.UseUU = 1 THEN OrderDetailsT.Conversion ELSE 1 END AS UOConversion, "
		"ServiceT.Barcode,  "
		" OrderT.PurchaseOrderNum, OrderT.ContactName, OrderT.CCName, CreditCardNamesT.CardName, CCExpDate, ShippingMethodT.Description as ShippingMethod, "
		" SoldToLocationT.Name as SoldToLocation, SoldToLocationT.Address1 as SoldToAdd1, SoldToLocationT.Address2 as SoldToAdd2, "
		" SoldToLocationT.City as SoldToCity, SoldToLocationT.State as SoldToState, SoldToLocationT.Zip as SoldToZip, "
		" SoldToLocationT.Phone as SoldToPhone, SoldToLocationT.Fax as SoldToFax, "
		" OrderDetailsT.ForStatus, "
		" PatientsT.UserDefinedID, SUBSTRING(PersonPat.First,1,1) + SUBSTRING(PersonPat.Middle,1,1) + SUBSTRING(PersonPat.Last,1,1) AS PatientInitials, "
		" AppointmentsT.Date AS ApptDate, OrderMethodsT.Method AS OrderMethod, OrderT.VendorConfirmed, OrderT.ConfirmationDate, OrderT.ConfirmationNumber "
		"FROM "
		"OrderT LEFT JOIN OrderDetailsT ON OrderT.ID = OrderDetailsT.OrderID "
		"LEFT JOIN OrderAppointmentsT ON OrderT.ID = OrderAppointmentsT.OrderID "
		"LEFT JOIN AppointmentsT ON OrderAppointmentsT.AppointmentID = AppointmentsT.ID "
		"LEFT JOIN PersonT PersonPat ON AppointmentsT.PatientID = PersonPat.ID "
		"LEFT JOIN PatientsT ON PersonPat.ID = PatientsT.PersonID "
		"LEFT JOIN ServiceT ON OrderDetailsT.ProductID = ServiceT.ID "
		"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
		"LEFT JOIN LocationsT ON OrderT.LocationID = LocationsT.ID "
		"LEFT JOIN SupplierT ON OrderT.Supplier = SupplierT.PersonID "
		"INNER JOIN PersonT ON SupplierT.PersonID = PersonT.ID "
		" LEFT JOIN LocationsT SoldToLocationT ON OrderT.LocationSoldTo = SoldToLocationT.ID "
		" LEFT JOIN CreditCardNamesT ON OrderT.CCTypeID = CreditCardNamesT.ID "
		" LEFT JOIN ShippingMethodT ON OrderT.ShipMethodID = ShippingMethodT.ID "
		"LEFT JOIN OrderMethodsT ON OrderT.OrderMethodID = OrderMethodsT.ID "
		"WHERE (OrderT.Deleted = 0) AND (OrderDetailsT.Deleted = 0)");
		break;


	case 224:
		//Pat Ins Info (PP)
		// (s.tullis 2014-06-26 10:38) - PLID 35638 - would like patient cell phone as an available field in the PatInsInfo report
		/*	Version History
			DRT 7/23/03 - Rewrote the query.  And boy did the old one suck, not to mention it wasn't updated with the
				newer Resp Type stuff.  I also dropped it from 178 lines to 30 lines in length.  (PLID 6418)
				// (f.dinatale 2010-10-18) - PLID 40876 - Added SSN Masking
		*/
		strSQL.Format("SELECT "
			"/* Patient information */ "
			"PatientsT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.CellPhone, "
			"PersonT.WorkPhone, PersonT.BirthDate, dbo.MaskSSN(PersonT.SocialSecurity, %s), CASE WHEN PersonT.Gender = 1 THEN 'Male'  "
			"WHEN PersonT.Gender = 2 THEN 'Female' ELSE '' END AS Gender, CASE WHEN PatientsT.MaritalStatus = 1 THEN 'Single' "
			"WHEN PatientsT.MaritalStatus = 2 THEN 'Married' WHEN MaritalStatus = 3 THEN 'Other' ELSE '' END AS MaritalStatus, "
			"LocationsT.Name PracticeName, PersonT.Company, PatientsT.UserDefinedID,  "
			"/* Insurance Information */ "
			"InsPerson.Last AS GuarLast, InsPerson.Middle AS GuarMiddle, "
			"InsPerson.First AS GuarFirst, InsPerson.Address1 AS GuarAddr1, InsPerson.Address2 AS GuarAddr2, InsPerson.City AS GuarCity,  "
			"InsPerson.State AS GuarState, InsPerson.Zip AS GuarZip, InsPerson.HomePhone AS GuarHPhone, InsPerson.BirthDate AS GuarBDate,  "
			"InsuredPartyT.Employer AS GuarCompany, InsPerson.Note AS GuarNote, InsuranceCoT.Name AS InsCoName, PersonContact.Last AS ContactLast, "
			"PersonContact.First AS ContactFirst, PersonInsCo.Address1 AS InsAddr, PersonInsCo.Address2 AS InsAddr2,  "
			"PersonInsCo.City AS InsCity, PersonInsCo.State AS InsState, PersonInsCo.Zip AS InsZip, PersonContact.WorkPhone AS InsPhone,  "
			"InsurancePlansT.PlanName, InsurancePlansT.PlanType, InsuredPartyT.PolicyGroupNum, InsuredPartyT.IDforInsurance,  "
			"InsuredPartyT.RespTypeID, RespTypeT.TypeName AS RespTypeName, InsuredPartyT.PersonID AS InsPartyID "
			"FROM "
			"PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
			"LEFT JOIN InsuredPartyT ON PatientsT.PersonID = InsuredPartyT.PatientID "
			"LEFT JOIN PersonT InsPerson ON InsuredPartyT.PersonID = InsPerson.ID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN PersonT PersonInsCo ON InsuranceCoT.PersonID = PersonInsCo.ID "
			"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
			"LEFT JOIN InsuranceContactsT ON InsuredPartyT.InsuranceContactID = InsuranceContactsT.PersonID "
			"LEFT JOIN PersonT PersonContact ON InsuranceContactsT.PersonID = PersonContact.ID "
			"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE  "
			"PatientsT.PersonID > 0 "
			"ORDER BY (CASE WHEN RespTypeT.Priority > 0 THEN RespTypeT.Priority ELSE (SELECT Max(Priority) + 1 FROM RespTypeT) END) ",
			((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"));
		return _T(strSQL);
		break;

	case 227:  {
		//Quote 
		/*	Version History
			TES 9/5/03: In the .ttx file, don't forget to make sure that a.) the TopText and BottomText are memos, 
				and b.) that ReportTitle and SubReportTitle have lengths of 255.
			DRT 7/9/2004 - PLID 13394 - There was an issue with the DiscountAmt field, basically it was wrong.  It was calculating 
				as (Amount + OtherAmount * % Disc * Qty) + $ Discount.  But discounts take things like modifiers into account, so
				I had to change the discount query to be ([Full PracBillFee query] + [Full OtherFee query]) * % Discount + $ Discount
			DRT 9/27/2004 - PLID 14714 - Made a number of big changes (again).  The quote was starting at the beginning and trying to
				add everything up on its own to get the totals.  However, there is a lot of rounding and crystal calculations going
				on, so the totals were not accurately matching up.  I added some fields to this query, so now we get the actual total
				of the line, total of the practice fees, total of each of the 4 tax values (prac1, prac2, other1, other2).  Then
				we can just sum those up to get real totals on the report, and calculate backwards to line up everything else that
				is questionable.  I also changed the discount column so it no longer includes tax amt saved.  That really never
				made sense, so we agreed to just toss it so that the actual amount discounted shows up.
				New Fields:
					DiscountAmt:  This is the actual amount that was discounted (unit cost * qty * % off + $ off
					GetChargeTotalAmt:  The whole line total as used in billing - dbo.GetChargeTotal().  This includes the OthrBillFee column.
					PracOnlyLineTotal:  The practice fee.  Calculates using amt * qty * mods - discounts * tax.  Ignores the OthrBillFee column.
					PracOnlyTax1Total:  The total amt tax1 adds onto the practice fee.  Does not look at OtherFee.
					PracOnlyTax2Total:	Same as above, but for Tax2.
					OtherOnlyTax1Total:	The total amt tax2 adds onto the Other fee.  Does not look at practice fee.
					OtherOnlyTax2Total:	Same as above, but for Tax2.
			JMM 03/10/2005 - Rounded the values so they add up correctly
			JMJ 06/27/2005 - added TotalLinkedPrepayments, which will show the total linked prepays for the quote (even though it is returned as field in each charge)
			JMM 06/30/2005 - added Procedure Description
			JMM 07/06/2005 - added ability to separate facility fee, anesthesia fee, cpt, and inventory items
			JMM 02/06/2005 - PLID 19105 - changed how surgery date field is gotten
			JMM 02/08/2005 - PLID 18578 - added boolean to say whether procdesc field was in use
			JMM 08/02/2006 - PLID 21272 - added primary insurance company name
			MDH 11/02/2006 - PLID 21274 - added primary and secondary insured party deductible amounts
			MDH 11/02/2006 - PLID 21273 - added anesthesia and facility time fields to query
			// (j.gruber 2006-12-29 15:14) - PLID 23972 - take deductible out
			// (j.gruber 2006-12-29 15:14) - PLID 23972 - Put in deductibleLeftToMeet, OOPLeftToMeet, and CoInsurance
			// (j.gruber 2007-02-27 13:41) - PLID 24972 - added surgery time, preop date and time
			// (j.gruber 2007-02-28 15:13) - PLID 23407 - added prepayments subreport
			// (j.gruber 2007-04-05 16:02) - PLID 11944 - added discount category information
			(e.lally 2007-07-13) PLID 26649 - Replaced CCType with link to CardName in a group by clause of a subquery
			// (j.gruber 2008-08-27 17:28) - PLID 31185 - fix the quote to not show inventory items marked as facility fees as facility fees and not inv items
			// (j.gruber 2009-03-30 11:46) - PLID 33349 - added discounts subreport
			// (j.jones 2009-09-22 17:28) - PLID 35196 - added primary and secondary insurance allowed amounts
			// (j.gruber 2009-10-16 09:29) - PLID 35963 - added secondary insurance company name
			// (j.gruber 2009-10-20 09:39) - PLID 36003 - added Charge Allowable
			// (j.jones 2009-10-23 10:09) - PLID 36011 - added insurance IDs
			// (j.gruber 2010-01-04 14:37) - PLID 18788 - added some pt. coord fields
			// (j.gruber 2010-02-02 14:56) - PLID 37171 - added prac discount amt and other discount amt
			// (j.gruber 2014-03-25 13:25) - PLID 61478 - updated for ICD-10
		*/		
		
		switch (nSubLevel) {

			case 0: //Main Report 
				{
		
			// (a.walling 2010-03-02 10:33) - PLID 37129 - Converted NTEXT MemoParam to NVARCHAR(4000) for the purposes of this report
			CString strSql;
			strSql.Format("SELECT QuoteQ.BillID, QuoteQ.QuoteDescription, QuoteQ.LineID, QuoteQ.PatID, QuoteQ.UserDefinedID, QuoteQ.PatName, QuoteQ.Address1, QuoteQ.Address2, QuoteQ.City, QuoteQ.State,  "
			"QuoteQ.Zip, QuoteQ.PracName, QuoteQ.PracAddr1, QuoteQ.PracAddr2, QuoteQ.PracCity, QuoteQ.PracState, QuoteQ.PracZip, QuoteQ.PracPhone,  "
			"QuoteQ.PracFax, QuoteQ.EmpName, QuoteQ.OnLineAddress, QuoteQ.Date, QuoteQ.DoctorName, QuoteQ.ItemCode, QuoteQ.ItemSubCode, QuoteQ.Description,  "
			"QuoteQ.Quantity, QuoteQ.TotalPercentOff as PercentOff, QuoteQ.TaxRate, QuoteQ.UnitCost, QuoteQ.PracBillFee, QuoteQ.OtherBillFee, QuoteQ.DiscountAmt, GetChargeTotalAmt, PracOnlyLineTotal, PracOnlyTax1Total, PracOnlyTax2Total, OtherOnlyTax1Total, OtherOnlyTax2Total, TotalLinkedPrepayments, (SELECT CONVERT(NVARCHAR(4000), ConfigRT.MemoParam) AS MemoParam FROM ConfigRT WHERE ConfigRT.Name = 'QuoteTopText') AS TopText, (SELECT CONVERT(NVARCHAR(4000), ConfigRT.MemoParam) FROM ConfigRT WHERE ConfigRT.Name = 'QuoteBottomText') AS BottomText, '" + _Q(GetRemotePropertyText("QuoteReportTitle", "Surgery Proposal", 0, "<None>", true)) + "' AS ReportTitle, '" + _Q(GetRemotePropertyText("QuoteSubReportTitle", "Surgery and Related Services", 0, "<None>", true)) + "' AS SubReportTitle, OrderID,  \r\n "

			"QuoteQ.ICD9Code1, \r\n "
			"QuoteQ.ICD9Code2, \r\n "
			"QuoteQ.ICD9Code3, \r\n "
			"QuoteQ.ICD9Code4, \r\n "

			"QuoteQ.ICD10Code1, \r\n "
			"QuoteQ.ICD10Code2, \r\n "
			"QuoteQ.ICD10Code3, \r\n "
			"QuoteQ.ICD10Code4, \r\n "

			"QuoteQ.TotalAmount, QuoteQ.TotalCount, QuoteQ.CurrentAmount, QuoteQ.CurrentCount, QuoteQ.ChOthrBillFee, QuoteQ.TaxRate2, "
			"QuoteQ.PatFirst, QuoteQ.PatMiddle, QuoteQ.PatLast, QuoteQ.ExpDate, QuoteQ.ProcDescription, QuoteQ.SuppressDesc, "
			"QuoteQ.IsCPTCode, QuoteQ.IsInvItem, QuoteQ.Anesthesia, QuoteQ.FacilityFee, QuoteQ.POSID, QuoteQ.POSName, "
			"QuoteQ.AnesthesiaPayTo, QuoteQ.FacilityPayTo, QuoteQ.SurgeryDate, QuoteQ.PriInsCoName AS PrimaryInsCoName, "
			/*"QuoteQ.PriInsDeductible, QuoteQ.SecInsDeductible, "*/
			"QuoteQ.AnesthStartTime, QuoteQ.AnesthEndTime, QuoteQ.FacilityStartTime, QuoteQ.FacilityEndTime, QuoteQ.AnesthesiaMinutes, QuoteQ.FacilityMinutes, "
			"QuoteQ.DeductibleLeftToMeet, QuoteQ.CoInsurance, QuoteQ.OOPLeftToMeet, "
			"QuoteQ.SurgeryDateTime, QuoteQ.PreopDateTime, QuoteQ.DiscountCategoryDescription, QuoteQ.ChargeID, "
			"QuoteQ.PrimaryInsuranceAllowable, QuoteQ.SecondaryInsuranceAllowable, QuoteQ.SecondaryInsCoName, QuoteQ.ChargeAllowable, "
			"QuoteQ.PrimaryIDForInsurance, QuoteQ.PrimaryPolicyGroupNum, QuoteQ.SecondaryIDForInsurance, QuoteQ.SecondaryPolicyGroupNum, "
			" QuoteQ.PatCoordFirst, QuoteQ.PatCoordLast, QuoteQ.PatCoordTitle, QuoteQ.PatCoordWPhone, QuoteQ.PatCoordExt, QuoteQ.PatCoordEmail, "
			" QuoteQ.PracPercentDiscountAmt, QuoteQ.OtherPercentDiscountAmt, QuoteQ.TotalDollarDiscount "
			"FROM "
			"( "
			"SELECT QuoteSubQ.BillID, QuoteSubQ.QuoteDescription, QuoteSubQ.LineID, QuoteSubQ.PatID, QuoteSubQ.UserDefinedID, QuoteSubQ.PatName,  "
			"QuoteSubQ.Address1, QuoteSubQ.Address2, QuoteSubQ.City, QuoteSubQ.State, QuoteSubQ.Zip, QuoteSubQ.PracName,  "
			"QuoteSubQ.PracAddr1, QuoteSubQ.PracAddr2, QuoteSubQ.PracCity, QuoteSubQ.PracState, QuoteSubQ.PracZip,  "
			"QuoteSubQ.PracPhone, QuoteSubQ.PracFax, QuoteSubQ.EmpName, QuoteSubQ.OnLineAddress, QuoteSubQ.Date, QuoteSubQ.DoctorName,  "
			"QuoteSubQ.ItemCode, QuoteSubQ.ItemSubCode, QuoteSubQ.Description, QuoteSubQ.Quantity, QuoteSubQ.TaxRate,  "
			"QuoteSubQ.UnitCost, QuoteSubQ.PracBillFee, QuoteSubQ.OtherBillFee, QuoteSubQ.DiscountAmt, GetChargeTotalAmt, PracOnlyLineTotal, PracOnlyTax1Total, PracOnlyTax2Total, OtherOnlyTax1Total, OtherOnlyTax2Total, TotalLinkedPrepayments, QuoteSubQ.OrderID, QuoteSubQ.TotalPercentOff, \r\n "

			"QuoteSubQ.ICD9Code1, \r\n "
			"QuoteSubQ.ICD9Code2, \r\n "
			"QuoteSubQ.ICD9Code3, \r\n "
			"QuoteSubQ.ICD9Code4, \r\n "

			"QuoteSubQ.ICD10Code1, \r\n "
			"QuoteSubQ.ICD10Code2, \r\n "
			"QuoteSubQ.ICD10Code3, \r\n "
			"QuoteSubQ.ICD10Code4, \r\n "

			"QuoteSubQ.CurrentCount, QuoteSubQ.CurrentAmount, QuoteSubQ.TotalCount, QuoteSubQ.TotalAmount, QuoteSubQ.ChOthrBillFee, QuoteSubQ.TaxRate2, "
			"QuoteSubQ.PatFirst, QuoteSubQ.PatMiddle, QuoteSubQ.PatLast, QuoteSubQ.ExpDate, QuoteSubQ.ProcedureDescription as ProcDescription, "
			"QuoteSubQ.SuppressDesc, "
			"QuoteSubQ.IsCPTCode, QuoteSubQ.IsInvItem, QuoteSubQ.Anesthesia, QuoteSubQ.FacilityFee, QuoteSubQ.POSID, QuoteSubQ.POSName, "
			"QuoteSubQ.AnesthesiaPayTo, QuoteSubQ.FacilityPayTo, QuoteSubQ.SurgeryDate, QuoteSubQ.PriInsCoName, "
			/*"QuoteSubQ.PriInsDeductible, QuoteSubQ.SecInsDeductible, "*/
			"QuoteSubQ.AnesthStartTime, QuoteSubQ.AnesthEndTime, QuoteSubQ.FacilityStartTime, QuoteSubQ.FacilityEndTime, QuoteSubQ.AnesthesiaMinutes, QuoteSubQ.FacilityMinutes, "
			"QuoteSubQ.DeductibleLeftToMeet, QuoteSubQ.CoInsurance, QuoteSubQ.OOPLeftToMeet, "
			"QuoteSubQ.SurgeryDateTime, QuoteSubQ.PreOPDateTime, QuoteSubQ.DiscountCategoryDescription, QuoteSubQ.ChargeID, "
			"CASE WHEN QuoteSubQ.PrimaryInsuranceCoID Is Null THEN Null ELSE dbo.GetChargeAllowableForInsuranceCo(QuoteSubQ.ChargeID, QuoteSubQ.PrimaryInsuranceCoID) END AS PrimaryInsuranceAllowable, "
			"CASE WHEN QuoteSubQ.SecondaryInsuranceCoID Is Null THEN Null ELSE dbo.GetChargeAllowableForInsuranceCo(QuoteSubQ.ChargeID, QuoteSubQ.SecondaryInsuranceCoID) END AS SecondaryInsuranceAllowable, "
			"CASE WHEN QuoteSubQ.SecondaryInsuranceCoID Is Null THEN Null ELSE QuoteSubQ.SecondaryInsuranceCoName END as SecondaryInsCoName, "
			"QuoteSubQ.ChargeAllowable, "
			"QuoteSubQ.PrimaryIDForInsurance, QuoteSubQ.PrimaryPolicyGroupNum, "
			"QuoteSubQ.SecondaryIDForInsurance, QuoteSubQ.SecondaryPolicyGroupNum, "
			" QuoteSubQ.PatCoordFirst, QuoteSubQ.PatCoordLast, QuoteSubQ.PatCoordTitle, QuoteSubQ.PatCoordWPhone, QuoteSubQ.PatCoordExt, QuoteSubQ.PatCoordEmail, "
			" QuoteSubQ.PracPercentDiscountAmt, QuoteSubQ.OtherPercentDiscountAmt, QuoteSubQ.TotalDollarDiscount "
			"FROM "
			"( "
			"SELECT BillsT.ID AS BillID, BillsT.Description AS QuoteDescription, LineItemT.ID AS LineID, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,  "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"PersonT.Address1, PersonT.Address2,  "
			"PersonT.City, PersonT.State, PersonT.Zip, LocationsT.Name AS PracName, LocationsT.Address1 AS PracAddr1, "
			"LocationsT.Address2 AS PracAddr2, LocationsT.City AS PracCity, LocationsT.State AS PracState, LocationsT.Zip AS PracZip, "
			"LocationsT.Phone AS PracPhone, LocationsT.Fax AS PracFax,  "
			"BillsT.DeductibleLeftToMeet, BillsT.OutOfPocketLeftToMeet as OOPLeftToMeet, BillsT.CoInsurance, "
			"(%s) AS EmpName, LocationsT.OnLineAddress, "
			"BillsT.Date, PersonDoctor.Last + ', ' + PersonDoctor.First + ' ' + PersonDoctor.Middle AS DoctorName, "
			"BillsT.AnesthStartTime, BillsT.AnesthEndTime, BillsT.FacilityStartTime, BillsT.FacilityEndTime, BillsT.AnesthesiaMinutes, BillsT.FacilityMinutes, "
			"ChargesT.ItemCode, ChargesT.ItemSubCode, LineItemT.Description, ChargesT.Quantity, ChargesT.TaxRate, ChargesT.TaxRate2,  "
			"Round(CONVERT(money, LineItemT.Amount * (CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)),2) AS UnitCost,  "
			"COALESCE(TotalPercentOff, 0) as TotalPercentOff, "
			"Round(CONVERT(money, ChargesT.OthrBillFee * (CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)),2) AS ChOthrBillFee, "
			"Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)),2) AS PracBillFee, "
			"Round(Convert(money,((CASE WHEN ChargesT.OthrBillFee Is Null THEN 0 ELSE ChargesT.OthrBillFee End)*[Quantity]*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END))),2) AS OtherBillFee, "

			
			" (Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)),2) " 
			" + " 
			" Round(Convert(money,((CASE WHEN ChargesT.OthrBillFee Is Null THEN 0 ELSE ChargesT.OthrBillFee End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END))),2)) "
			" - "
			"Round(Convert(money, ((((CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]* "
			"(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*"
			"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*"
			"(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*"
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END))"
			"+ "
			"((CASE WHEN ChargesT.OthrBillFee Is Null THEN 0 ELSE ChargesT.OthrBillFee End)*[Quantity]* "
			"(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)* "
			"(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)* "
			"(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END))))"
			"*CASE WHEN COALESCE([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,COALESCE([TotalPercentOff],0))/100.0)) END "
			" - Coalesce([TotalDiscount], CONVERT(money, 0))), 2) AS DiscountAmt, "

			"Round(convert(money, dbo.GetChargeTotal(LineItemT.ID)),2) AS GetChargeTotalAmt, "

			"Round(Convert(money, ((( "
			"/* Base Calculation */ "
			"(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*  "
			"(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
			"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
			"(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			")* /* Discount 1 */ "
			"CASE WHEN COALESCE([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,COALESCE([totalPercentOff],0))/100.0)) END "
			") - /* Discount 2 */ "
			"CASE WHEN Amount > 0 OR OthrBillFee = 0 THEN COALESCE([TotalDiscount], CONVERT(money,0)) ELSE 0 END "
			")* /* Tax */ "
			"(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)  "
			"), 2) AS PracOnlyLineTotal, "

			"Round(Convert(money, ((( "
			"/* Base Calculation */ "
			"(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*  "
			"(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
			"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
			"(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			")* /* Discount 1 */ "
			"CASE WHEN Coalesce([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,Coalesce([TotalPercentOff],0))/100.0)) END "
			") - /* Discount 2 */ "
			"CASE WHEN LineItemT.Amount > 0 OR OthrBillFee = 0 THEN Coalesce([TotalDiscount], convert(money, 0)) ELSE 0 END "
			")* /* Tax */ "
			" (ChargesT.[TaxRate]-1)  "
			"),2) AS PracOnlyTax1Total,  "

			"Round(Convert(money, ((( "
			"/* Base Calculation */ "
			"(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*  "
			"(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
			"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
			"(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			")* /* Discount 1 */ "
			"CASE WHEN [TotalPercentOff] IS NULL THEN 1 ELSE (1.0 - (Convert(float,[TotalPercentOff])/100.0)) END "
			") - /* Discount 2 */ "
			"CASE WHEN LineItemT.Amount > 0 OR OthrBillFee = 0 THEN COALESCE([TotalDiscount], Convert(money,0)) ELSE 0 END "
			")* /* Tax */ "
			" (ChargesT.[TaxRate2]-1)  "
			"),2) AS PracOnlyTax2Total,  "

			"Round(Convert(money, ((( "
			"/* Base Calculation */ "
			"(CASE WHEN [ChargesT].[OthrBillFee] Is Null THEN 0 ELSE [ChargesT].[OthrBillFee] End)*[Quantity]*  "
			"(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
			"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
			"(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			")* /* Discount 1 */ "
			"CASE WHEN Coalesce([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,Coalesce([TotalPercentOff], 0))/100.0)) END "
			") - /* Discount 2 */ "
			"CASE WHEN LineItemT.Amount = 0 AND OthrBillFee > 0 THEN Coalesce([TotalDiscount], convert(money,0)) ELSE 0 END "
			")* /* Tax */ "
			"(ChargesT.[TaxRate]-1)  "
			"),2) AS OtherOnlyTax1Total, "

			"Round(Convert(money, ((( "
			"/* Base Calculation */ "
			"(CASE WHEN [ChargesT].[OthrBillFee] Is Null THEN 0 ELSE [ChargesT].[OthrBillFee] End)*[Quantity]*  "
			"(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
			"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
			"(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			")* /* Discount 1 */ "
			"CASE WHEN Coalesce([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,Coalesce([TotalPercentOff], 0))/100.0)) END "
			") - /* Discount 2 */ "
			"CASE WHEN LineItemT.Amount = 0 AND OthrBillFee > 0 THEN Coalesce([TotalDiscount], convert(money,0)) ELSE 0 END "
			")* /* Tax */ "
			"(ChargesT.[TaxRate2]-1)  "
			"),2) AS OtherOnlyTax2Total, "

			"Round(Convert(money, (SELECT Coalesce(Sum(Amount), 0) AS TotalPrePays FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID WHERE Deleted = 0 AND PaymentsT.QuoteID = BillsT.ID)), 2) AS TotalLinkedPrepayments, "

			" (Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)),2)  )			 "
			"	 -  "
			"  Round(Convert(money, ((((CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*  "
			"	(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)* "
			"	(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
			"	(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)* "
			"	(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)) "
			"	)) "
			"	*CASE WHEN COALESCE([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,COALESCE([TotalPercentOff],0))/100.0)) END  "
			"	 /*- Coalesce([TotalDiscount], CONVERT(money, 0))*/), 2) AS PracPercentDiscountAmt,  "
			" "
			" (Round(Convert(money,((CASE WHEN ChargesT.OthrBillFee Is Null THEN 0 ELSE ChargesT.OthrBillFee End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END))),2))  "
			"  - 		 "
			" Round(Convert(money, ((((CASE WHEN ChargesT.OthrBillFee Is Null THEN 0 ELSE ChargesT.OthrBillFee End)*[Quantity]*  "
			" (CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*  "
			" (CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*  "
			" (CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*  "
			" (CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)))) "
			" *CASE WHEN COALESCE([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,COALESCE([TotalPercentOff],0))/100.0)) END  "
			"  /*- Coalesce([TotalDiscount], CONVERT(money, 0))*/), 2) AS OtherPercentDiscountAmt, "			

			"Round (Convert(money, Coalesce([TotalDiscount], CONVERT(money, 0))), 2) as TotalDollarDiscount, "


			"ChargesT.LineID AS OrderID, \r\n "

			"ICD9T1.CodeNumber as ICD9Code1, \r\n "
			"ICD9T2.CodeNumber as ICD9Code2, \r\n "
			"ICD9T3.CodeNumber as ICD9Code3, \r\n "
			"ICD9T4.CodeNumber as ICD9Code4, \r\n "

			"ICD10T1.CodeNumber as ICD10Code1, \r\n "
			"ICD10T2.CodeNumber as ICD10Code2, \r\n "
			"ICD10T3.CodeNumber as ICD10Code3, \r\n "
			"ICD10T4.CodeNumber as ICD10Code4, \r\n "
			
			"PackagesT.TotalAmount, PackagesT.TotalCount, PackagesT.CurrentAmount, PackagesT.CurrentCount,  "
			"PersonT.First AS PatFirst, PersonT.Middle AS PatMiddle, PersonT.Last AS PatLast, "
			"(CASE WHEN BillsT.UseExp = 0 THEN NULL ELSE Convert(datetime,Convert(nvarchar,DATEADD(day,BillsT.ExpDays,BillsT.Date),1)) END) AS ExpDate, "
			" ServiceT.ProcedureDescription, "
			" CASE WHEN Convert(nvarchar, ServiceT.ProcedureDescription) = ''  OR ServiceT.ProcedureDescription IS NULL THEN 1 ELSE 0 END AS SuppressDesc, "
			" CASE WHEN CPTCodeT.ID IS NULL THEN 0 ELSE 1 END AS IsCPTCode, "
			" CASE WHEN ProductT.ID IS NULL THEN 0 ELSE CASE WHEN ServiceT.FacilityFee = 0 then 1 ELSE 0 END END AS IsInvItem, "
			" ServiceT.Anesthesia, ServiceT.FacilityFee, BillsT.Location AS POSID, POST.Name AS POSName, "
			" POST.AnesthesiaPayTo, POST.FacilityPayTo, "
			" (SELECT Top 1 AppointmentsT.Date FROM AppointmentsT INNER JOIN ProcInfoT ON AppointmentsT.ID = ProcInfoT.SurgeryApptID "
			" WHERE ProcInfoT.ActiveQuoteID = BillsT.ID) AS SurgeryDate, "
			" (SELECT Name FROM InsuranceCoT WHERE PersonID IN (SELECT InsuranceCoID FROM InsuredPartyT WHERE PatientID = PatientsT.PersonID AND RespTypeID = 1)) AS PriInsCoName, "
			/*" (SELECT Deductible FROM InsuredPartyT WHERE PatientID = PatientsT.PersonID AND RespTypeID = 1) AS PriInsDeductible, "
			" (SELECT Deductible FROM InsuredPartyT WHERE PatientID = PatientsT.PersonID AND RespTypeID = 2) AS SecInsDeductible "*/
			" (SELECT Top 1 AppointmentsT.StartTime FROM AppointmentsT INNER JOIN ProcInfoT ON AppointmentsT.ID = ProcInfoT.SurgeryApptID "
			" WHERE ProcInfoT.ActiveQuoteID = BillsT.ID) AS SurgeryDateTime, "
			" (SELECT Min(AppointmentsQ.StartTime) FROM "
			" (SELECT ID, PatientID, StartTime FROM AppointmentsT WHERE AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 2) AND Status <> 4 AND ShowState <> 3 AND AppointmentsT.PatientID = BillsT.PatientID AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoID IN (SELECT ID FROM ProcInfoT WHERE ProcInfoT.ActiveQuoteID = BillsT.ID))) "
			"  ) AppointmentsQ) As PreopDateTime, "
			
			"dbo.GetChargeDiscountList(ChargesT.ID) AS DiscountCategoryDescription, ChargesT.ID as ChargeID, "
			"PrimaryInsuredPartyQ.InsuranceCoID AS PrimaryInsuranceCoID, SecondaryInsuredPartyQ.InsuranceCoID AS SecondaryInsuranceCoID, "
			"SecondaryInsuredPartyQ.Name as SecondaryInsuranceCoName, ChargesT.Allowable as ChargeAllowable, "
			"PrimaryInsuredPartyQ.IDForInsurance AS PrimaryIDForInsurance, PrimaryInsuredPartyQ.PolicyGroupNum AS PrimaryPolicyGroupNum, "
			"SecondaryInsuredPartyQ.IDForInsurance AS SecondaryIDForInsurance, SecondaryInsuredPartyQ.PolicyGroupNum AS SecondaryPolicyGroupNum, "

			" PersonEmp.First as PatCoordFirst, PersonEmp.Last as PatCoordLast, PersonEmp.Title as PatCoordTitle, PersonEmp.WorkPhone as PatCoordWPhone, PersonEmp.Extension as PatCoordExt, "
			" PersonEmp.Email as PatCoordEmail "

			"FROM "
			"BillsT LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
			"LEFT JOIN "
			"	(SELECT PackagesT.QuoteID, TotalAmount, CurrentAmount, Type, "
			"	(CASE WHEN Type = 1 THEN Convert(float,PackagesT.TotalCount) WHEN Type = 2 THEN PackageChargesQ.MultiUseTotalCount ELSE 0 END) AS TotalCount, "
			"	(CASE WHEN Type = 1 THEN Convert(float,PackagesT.CurrentCount) WHEN Type = 2 THEN PackageChargesQ.MultiUseCurrentCount ELSE 0 END) AS CurrentCount "
			"	FROM PackagesT "
			"	LEFT JOIN "
			"		(SELECT ChargesT.BillID, Sum(Quantity) AS MultiUseTotalCount, Sum(PackageQtyRemaining) AS MultiUseCurrentCount "
			"		FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE Deleted = 0 "
			"		AND (LineItemT.Amount > Convert(money,0) OR ChargesT.OthrBillFee = Convert(money,0)) "
			"		GROUP BY BillID "
			"		) AS PackageChargesQ ON PackagesT.QuoteID = PackageChargesQ.BillID "
			"	) AS PackagesT ON BillsT.ID = PackagesT.QuoteID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT OUTER JOIN LocationsT ON "
			"LineItemT.LocationID = LocationsT.ID LEFT OUTER JOIN UsersT ON "
			"BillsT.PatCoord = UsersT.PersonID LEFT OUTER JOIN PersonT PersonEmp ON "
			"UsersT.PersonID = PersonEmp.ID LEFT OUTER JOIN PersonT PersonDoctor ON "
			"ChargesT.DoctorsProviders = PersonDoctor.ID "
			" LEFT JOIN DiagCodes ICD9T1 ON PatientsT.DefaultDiagID1 = ICD9T1.ID \r\n"
			" LEFT JOIN DiagCodes ICD9T2 ON PatientsT.DefaultDiagID2 = ICD9T2.ID \r\n"
			" LEFT JOIN DiagCodes ICD9T3 ON PatientsT.DefaultDiagID3 = ICD9T3.ID \r\n"
			" LEFT JOIN DiagCodes ICD9T4 ON PatientsT.DefaultDiagID4 = ICD9T4.ID \r\n"
			" LEFT JOIN DiagCodes ICD10T1 ON PatientsT.DefaultICD10DiagID1 = ICD10T1.ID \r\n"
			" LEFT JOIN DiagCodes ICD10T2 ON PatientsT.DefaultICD10DiagID2 = ICD10T2.ID \r\n"
			" LEFT JOIN DiagCodes ICD10T3 ON PatientsT.DefaultICD10DiagID3 = ICD10T3.ID \r\n"
			" LEFT JOIN DiagCodes ICD10T4 ON PatientsT.DefaultICD10DiagID4 = ICD10T4.ID \r\n"
			" LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			" LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
			" LEFT JOIN LocationsT POST ON BillsT.Location = POST.ID "
			" LEFT JOIN (SELECT PatientID, InsuranceCoID, IDForInsurance, PolicyGroupNum FROM InsuredPartyT WHERE RespTypeID = 1) AS PrimaryInsuredPartyQ ON PatientsT.PersonID = PrimaryInsuredPartyQ.PatientID "
			" LEFT JOIN (SELECT PatientID, InsuranceCoID, IDForInsurance, PolicyGroupNum, InsuranceCoT.Name FROM InsuredPartyT LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID WHERE RespTypeID = 2) AS SecondaryInsuredPartyQ ON PatientsT.PersonID = SecondaryInsuredPartyQ.PatientID "
			"WHERE (PatientsT.PersonID > 0) AND (LineItemT.Deleted = 0) AND (BillsT.Deleted = 0) AND  "
			"(BillsT.EntryType = 2) AND (LineItemT.Type = 11)) AS QuoteSubQ "
			") AS QuoteQ ORDER BY OrderID", GetQuoteEmpName());
			return _T(strSql);
			}
			break;

			case 1:  

				switch (nSubRepNum) {
					

						// (j.gruber 2009-03-30 11:47) - PLID 33349- discounts subreport
					case 0: {
						CString str;
						str.Format("SELECT ID, ChargeID, BillID, PercentOff, Discount, DiscountCategoryDescription FROM ( "
							"SELECT ID, ChargeID, (SELECT BillID FROM ChargesT WHERE ID = ChargeDiscountsT.ChargeID) AS BillID, PercentOff, Discount, CASE WHEN ChargeDiscountsT.DiscountCategoryID IS NULL THEN '' ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -1 THEN ChargeDiscountsT.CustomDiscountDesc ELSE "
							" CASE WHEN ChargeDiscountsT.DiscountCategoryID = -2 THEN (SELECT Description FROM CouponsT WHERE ID = ChargeDiscountsT.CouponID) ELSE "
							" (SELECT Description FROM DiscountCategoriesT WHERE ID = ChargeDiscountsT.DiscountCategoryID) END END END AS DiscountCategoryDescription "
							" FROM ChargeDiscountsT WHERE DELETED = 0 ) Q");
						
							return _T(str);
							}
						break;

					case 1:
						{
							CString str;
							str.Format("SELECT ChargeID, ChargesT.BillID AS BillID, "
							" CASE WHEN ChargeDiscountsT.DiscountCategoryID IS NULL THEN '' ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -1 THEN ChargeDiscountsT.CustomDiscountDesc ELSE "
							" CASE WHEN ChargeDiscountsT.DiscountCategoryID = -2 THEN (SELECT Description FROM CouponsT WHERE ID = ChargeDiscountsT.CouponID) ELSE "
							" (SELECT Description FROM DiscountCategoriesT WHERE ID = ChargeDiscountsT.DiscountCategoryID) END END END AS DiscountCategoryDescription, "
								" (Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)),2) " 
								" + " 
								" Round(Convert(money,((CASE WHEN ChargesT.OthrBillFee Is Null THEN 0 ELSE ChargesT.OthrBillFee End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END))),2)) "
								" - "
								"Round(Convert(money, ((((CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]* "
								"(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*"
								"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*"
								"(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*"
								"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END))"
								"+ "
								"((CASE WHEN ChargesT.OthrBillFee Is Null THEN 0 ELSE ChargesT.OthrBillFee End)*[Quantity]* "
								"(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)* "
								"(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)* "
								"(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
								"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END))))"
								"*CASE WHEN [PercentOff] IS NULL THEN 1 ELSE (1.0 - (Convert(float,[PercentOff])/100.0)) END "
								" -[Discount]), 2) AS DiscountAmt "
								"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
								" LEFT JOIN ChargeDiscountsT ON ChargesT.ID = ChargeDiscountsT.ChargeID "
								" WHERE LineItemT.Deleted = 0 AND ChargeDiscountsT.Deleted = 0");

							return _T(str);


						}
						break;

						case 2: //linked prepays
						{
							CString str;
							str.Format("SELECT PrePaysQ.Description, PrePaysQ.Amount, PrePaysQ.TDate as TDate, "
								" PrePaysQ.IDate as IDate, PrePaysQ.BillID AS BillID "
								" FROM "
								" (SELECT    LineItemT.Description,   " 
								" (CASE WHEN PrepayAppliedToQ.ID IS NULL THEN    "
								"     (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount END)   "
								" ELSE   "
								"     (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END) AS Amount,   "
								"     LineItemT.Date AS TDate, LineItemT.InputDate AS IDate, "
								"     PaymentsT.QuoteID as BillID "
								" FROM PaymentsT INNER JOIN   "
								"     (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON PaymentsT.ID = LineItemT.ID INNER JOIN   "
								"     PatientsT ON    "
								"     LineItemT.PatientID = PatientsT.PersonID INNER JOIN   "
								"     PersonT ON PatientsT.PersonID = PersonT.ID LEFT JOIN   "
								"     PersonT PersonT1 ON    "
								"     PaymentsT.ProviderID = PersonT1.ID LEFT OUTER JOIN   "
								"     AppliesT AppliesT1 ON    "
								"     LineItemT.ID = AppliesT1.DestID LEFT OUTER JOIN   "
								"     AppliesT ON    "
								"     LineItemT.ID = AppliesT.SourceID LEFT OUTER JOIN   "
								"     PaymentPlansT ON    "
								"     PaymentsT.ID = PaymentPlansT.ID   "
								" LEFT JOIN   "
								" /* This will total everything applied to a prepayment */   "
								" ( SELECT SUM( AppliesT.Amount * -1 ) AS Amount, AppliesT.DestID AS ID   "
								" FROM   "
								" LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
								" LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.DestID   "
								" LEFT JOIN LineItemT LineItemT1 ON AppliesT.SourceID = LineItemT1.ID   "
								" WHERE (LineItemT.Deleted = 0)  " 
								" GROUP BY AppliesT.DestID   "
								" ) PrepayAppliedToQ ON LineItemT.ID = PrepayAppliedToQ.ID   "
								" LEFT JOIN   "
								" /* This will total everything that the prepayment is applied to */   "
								" ( SELECT SUM(AppliesT.Amount ) AS Amount, AppliesT.SourceID AS ID   "
								" FROM  "
								" LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
								" LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID   "
								" LEFT JOIN LineItemT LineItemT1 ON AppliesT.DestID = LineItemT1.ID   "
								" WHERE LineItemT.Deleted = 0   "
								" GROUP BY AppliesT.SourceID   "
								" ) PrepayAppliesQ ON LineItemT.ID = PrepayAppliesQ.ID   "
								"  WHERE (LineItemT.Deleted = 0)  "
								" AND (PaymentsT.Prepayment = 1)  "
								" AND  "
								" (CASE WHEN PrepayAppliedToQ.ID IS NULL THEN    "
								"      (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount END)   "
								" ELSE   "
								"     (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END) <> 0  "
								" GROUP BY LineItemT.ID, PatientsT.UserDefinedID, PatientsT.PersonID,    "
								"     PaymentsT.ProviderID, PaymentsT.PayMethod,    "
								"     LineItemT.Description,   "
								" (CASE WHEN PrepayAppliedToQ.ID IS NULL THEN    "
								"     (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount END)   "
								" ELSE   "
								"     (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END),   "
								"     LineItemT.Date, PaymentPlansT.CheckNo,    "
								"     PaymentPlansT.CreditCardID, LineItemT.InputDate,    "
								"     PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
								"      PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle,   "
								" LineItemT.LocationID,   "
								" LocationsT.Name, PaymentsT.QuoteID) PrePaysQ INNER JOIN BillsT ON PrePaysQ.BillID = BillsT.ID  ");
							return _T(str);
						}
						break;

						default:
							return "";
						break;
				}
				break;

				default:
					return "";
				break;
			}
		}
		break;
				
	case 226:
			//Billing Dialog
			/*	Version History
				1/16/03 - DRT - Added the BillsT.Description to the SELECT statement, it was not previously added.  At this time it is only used for the
						BillingDialogInternal report, for Najila's use.
				11/1/2005 - JMM - PLID 18150: Added first, middle, and last name fields for the patient
				// (j.gruber 2007-05-29 12:38) - PLID 25979 - add discount categories
				// (j.gruber 2009-04-02 10:21) - PLID 33814 - update discount structure, added subreport
				// (j.jones 2009-07-08 15:36) - PLID 34097 - added revenue codes, in the event that a service has multiple codes,
				// we mimic the UB form and use only the code tied to BillsT.InsuredPartyID
				// (j.gruber 2014-03-25 14:53) - PLID 61479 - update for ICD-10
			*/
		switch (nSubLevel) {

			case 0: //Main Report 
				return _T("SELECT BillsT.ID AS BillID, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,  "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
				"PersonT.Address1, PersonT.Address2, "
				"PersonT.City, PersonT.State, PersonT.Zip, LocationsT.Name AS PracName, LocationsT.Address1 AS PracAddr1, "
				"LocationsT.Address2 AS PracAddr2, LocationsT.City AS PracCity, LocationsT.State AS PracState, LocationsT.Zip AS PracZip, "
				"LocationsT.Phone AS PracPhone, LocationsT.Fax AS PracFax,  "
				"PersonEmp.Last + ', ' + PersonEmp.First + ' ' + PersonEmp.Middle AS EmpName, "
				"LineItemT.Date, PersonDoctor.Last + ', ' + PersonDoctor.First + ' ' + PersonDoctor.Middle AS DoctorName, "
				"ChargesT.ItemCode, ChargesT.ItemSubCode, LineItemT.Description, ChargesT.Quantity, (ChargesT.TaxRate - 1)*100 AS TaxRate1, (ChargesT.TaxRate2 - 1) * 100 AS TaxRate2, "
				"ChargesT.LineID, "
				"LineItemT.Amount AS UnitCost,  "
				"dbo.GetChargeTotal(ChargesT.ID) AS Total, "
				"Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))*(ChargesT.TaxRate-1)))),2) AS TaxTotal1, "
				"Round(convert(money, Convert(money,((LineItemT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))*(ChargesT.TaxRate2-1)))),2) AS TaxTotal2, \r\n "
				
				"ICD9T1.CodeNumber as ICD9Code1, \r\n "
				"ICD9T2.CodeNumber as ICD9Code2, \r\n "
				"ICD9T3.CodeNumber as ICD9Code3, \r\n "
				"ICD9T4.CodeNumber as ICD9Code4, \r\n "

				"ICD10T1.CodeNumber as ICD10Code1, \r\n "
				"ICD10T2.CodeNumber as ICD10Code2, \r\n "
				"ICD10T3.CodeNumber as ICD10Code3, \r\n "
				"ICD10T4.CodeNumber as ICD10Code4, \r\n "

				"ICD9T1.CodeDesc as ICD9CodeDesc1, \r\n "
				"ICD9T2.CodeDesc as ICD9CodeDesc2, \r\n "
				"ICD9T3.CodeDesc as ICD9CodeDesc3, \r\n "
				"ICD9T4.CodeDesc as ICD9CodeDesc4, \r\n "

				"ICD10T1.CodeDesc as ICD10CodeDesc1, \r\n "
				"ICD10T2.CodeDesc as ICD10CodeDesc2, \r\n "
				"ICD10T3.CodeDesc as ICD10CodeDesc3, \r\n "
				"ICD10T4.CodeDesc as ICD10CodeDesc4,  \r\n "

				"BillsT.Description AS BillDescription, "
				"POSLoc.Name AS POS, PlaceofServiceCodesT.PlaceCodes AS POSDesignation, "
				"COALESCE(TotalPercentOff, 0) as PercentOff, COALESCE(TotalDiscount, convert(money,0)) as Discount, "
				"PersonT.Last As PatLast, PersonT.Middle as PatMiddle, PersonT.First as PatFirst, "
				"ChargesT.CPTModifier, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4, "
				
				" dbo.GetChargeDiscountList(ChargesT.ID) AS DiscountCategoryDescription, ChargesT.ID as ChargeID, "

				"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Code WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Code ELSE NULL END) AS RevenueCode, "
				"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Name WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Name ELSE NULL END) AS RevenueCodeDesc "

				"FROM "
				"BillsT LEFT OUTER JOIN ChargesT ON "
				"BillsT.ID = ChargesT.BillID "
				"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
				"INNER JOIN LineItemT ON "
				"ChargesT.ID = LineItemT.ID INNER JOIN PatientsT ON "
				"LineItemT.PatientID = PatientsT.PersonID INNER JOIN PersonT ON "
				"PatientsT.PersonID = PersonT.ID LEFT OUTER JOIN LocationsT ON "
				"LineItemT.LocationID = LocationsT.ID LEFT OUTER JOIN UsersT ON "
				"BillsT.PatCoord = UsersT.PersonID LEFT OUTER JOIN PersonT PersonEmp ON "
				"UsersT.PersonID = PersonEmp.ID LEFT OUTER JOIN PersonT PersonDoctor ON "
				"ChargesT.DoctorsProviders = PersonDoctor.ID \r\n "
				
				"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n "
				"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
				"LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
				"LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
				"LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
				"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
				"LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
				"LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
				"LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "

				"LEFT JOIN LocationsT POSLoc ON BillsT.Location = POSLoc.ID "
				"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
				"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
				"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON BillsT.InsuredPartyID = ServiceRevCodesT.InsuredPartyID AND ServiceT.ID = ServiceRevCodesT.ServiceID "
				"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
				"WHERE (PatientsT.PersonID > 0) AND (LineItemT.Deleted = 0) AND (BillsT.Deleted = 0) "
				"ORDER BY ChargesT.LineID");
				break;

			case 1:
				switch (nSubRepNum) {

					case 0: {
						CString str;
						str.Format("SELECT ID, ChargeID, BillID, PercentOff, Discount, DiscountCategoryDescription FROM ( "
							"SELECT ID, ChargeID, (SELECT BillID FROM ChargesT WHERE ID = ChargeDiscountsT.ChargeID) AS BillID, PercentOff, Discount, CASE WHEN ChargeDiscountsT.DiscountCategoryID IS NULL THEN '' ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -1 THEN ChargeDiscountsT.CustomDiscountDesc ELSE "
							" CASE WHEN ChargeDiscountsT.DiscountCategoryID = -2 THEN (SELECT Description FROM CouponsT WHERE ID = ChargeDiscountsT.CouponID) ELSE "
							" (SELECT Description FROM DiscountCategoriesT WHERE ID = ChargeDiscountsT.DiscountCategoryID) END END END AS DiscountCategoryDescription "
							" FROM ChargeDiscountsT WHERE DELETED = 0 ) Q");
						
							return _T(str);
							}
						break;
					break;

					default :
						return _T("");
					break;
				}
			break;

			default:
				return _T("");
			break;

		}
		break;

		case 235:
			//Use thequery from the reports module.
			return GetSqlPayments(nSubLevel, nSubRepNum);
			break;
	
//	case 239:
	case 416: //resource view report
			{

			//Scheduling reports
			/*	Version History
				DRT 6/19/03 - Removed references to AptPurposeID, which is obsolete.
				DRT 7/30/03 - Added PrivFax field
				TES 4/15/05 - Added Custom1-4
				// (j.gruber 2008-09-17 12:59) - PLID 30284 - added allocations
				//(e.lally 2008-11-18) PLID 32070 - Fixed exception when filtering on appointment type
				// (z.manning 2009-02-24 08:57) - PLID 32809 - Added cell phone
				// (j.jones 2011-04-05 09:57) - PLID 39448 - added up to 6 insco names
				// (j.armen 2011-07-06 11:09) - PLID 44205 - added confirmed by
				// (j.gruber 2012-09-11 10:07) - PLID 52033 - added appt ins fields
			*/

			CString strEnd;
			// (c.haag 2004-02-02 10:23) - I can't believe this has never been here since August!
			if (!GetRemotePropertyInt("ClassicReptShowCancelledAppts",0,0,GetCurrentUserName())) {
				
				strEnd += " WHERE (AppointmentsT.Status <> 4) ";
			}
			else { 
				// (j.gruber 2008-09-17 12:43) - PLID 30284 - added this because otherwise if you had both checked it used the join 
				//and I couldn't use that with the allocation stuff
				strEnd += " WHERE (1 = 1) ";
			}


			//DRT 2/9/2004 - PLID 10848 - Give the user the option to filter on the current view
			if(nID == 416 && GetRemotePropertyInt("SchedViewedResources", 0, 0, GetCurrentUserName())) {
				CString strView;
				CNxTabView *pView = GetMainFrame()->GetActiveView();

				//We must ensure that this is the SchedulerView before trying to get the resource out of it.  This code should not
				//be called from anywhere but the scheduler.
				if(!pView || !pView->IsKindOf(RUNTIME_CLASS(CSchedulerView))) {
					ASSERT(FALSE);
					return "";
				}

				long nViewID = ((CSchedulerView*)pView)->m_nLastLoadedResourceViewID;
				if(nViewID > 0) {
					strView.Format(" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE AppointmentResourceT.ResourceID IN "
						"(SELECT ResourceID FROM ResourceViewDetailsT WHERE Relevence >= 0 AND ResourceViewID = %li)) ", nViewID);
				} else {
					strView.Format(" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE AppointmentResourceT.ResourceID IN "
						"(SELECT ResourceID FROM UserResourcesT WHERE Relevence >= 0 AND UserID = %li)) ", GetCurrentUserID());
				}

				strEnd += strView;

			}
			

			CString str;

			switch (nSubLevel) {
				case 0: 
				
					str = "SELECT PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, PersonT.First,  "
					"    PersonT.Middle, PersonT.Last, ResourceT.Item,  "
					"    AppointmentsT.Date AS Date,  "
					"    AppointmentsT.Notes, AptTypeT.Name AS Type,  "
					"    dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, PersonT.HomePhone, PersonT.CellPhone, PersonT.Email, PersonT.Fax, "
					"    PersonT.BirthDate, "
					"    CASE WHEN AppointmentsT.Status = 4 THEN 'Cancelled' ELSE AptShowStateT.Name END AS StateText, "
					"    PersonT.WorkPhone, AppointmentsT.Confirmed, AppointmentsT.ConfirmedBy, "
					"    convert(datetime, convert(varchar, AppointmentsT.StartTime, 14)) AS StartTime, "
					"    convert(datetime, convert(varchar, AppointmentsT.EndTime, 14)) AS EndTime, PersonT.PrivHome, PersonT.PrivWork, PersonT.PrivCell, "
					"    ResourceT.ID AS ID, AppointmentsT.LocationID AS LocID, LocationsT.Name AS Location, ResourceT.ID AS ProvID /*Used for scheduler module*/, ResourceT.ID AS ResourceID, PrivFax, "
					"    (SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 1) AS CustomText1, "
					"    (SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 2) AS CustomText2, "
					"    (SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 3) AS CustomText3, "
					"    (SELECT TextParam FROM CustomFieldDataT WHERE PersonID = AppointmentsT.PatientID AND FieldID = 4) AS CustomText4,  " 
					"    (SELECT TOP 1 ID FROM PatientInvAllocationsT WHERE AppointmentID = AppointmentsT.ID AND Status != " + AsString((long)InvUtils::iasDeleted) + " ) AS AllocationID, "
					"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 1)) AS PriInsCoName, "
					"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 2)) AS SecInsCoName, "
					"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 3)) AS ThirdInsCoName, "
					"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 4)) AS FourthInsCoName, "
					"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 5)) AS FifthInsCoName, "
					"    (SELECT Name From InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"		WHERE PatientID = AppointmentsT.PatientID AND RespTypeT.Priority = 6)) AS SixthInsCoName, "
					"    apptPriInsQ.InsCoName as ApptPriIns, \r\n"
					"    apptSecInsQ.InsCoName as ApptSecIns, \r\n"
					"    CASE WHEN ApptPriInsQ.InsCoID IS NULL THEN '' ELSE dbo.GetApptInsuranceList(AppointmentsT.ID) END as ApptInsList \r\n" 

					"FROM AptTypeT RIGHT OUTER JOIN "
					"    (AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID) ON  "
					"    AptTypeT.ID = AppointmentsT.AptTypeID LEFT OUTER JOIN "
					"    (AppointmentResourceT INNER JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID) ON  "
					"    AppointmentsT.ID = AppointmentResourceT.AppointmentID LEFT OUTER JOIN "
					"    PersonT INNER JOIN "
					"    PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
					"    AppointmentsT.PatientID = PatientsT.PersonID LEFT JOIN AptShowStateT ON AptShowStateT.ID = AppointmentsT.ShowState "
					"			 LEFT JOIN (SELECT AptInsQ.AppointmentID, InsuranceCoT.Name as InsCoName, InsuranceCoT.PersonID as InsCoID FROM \r\n"
					"			(SELECT * FROM AppointmentInsuredPartyT WHERE Placement = 1) AptInsQ \r\n"
					"			LEFT JOIN InsuredPartyT ON AptInsQ.InsuredPartyID = InsuredPartyT.PersonID \r\n"
					"			LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID ) ApptPriInsQ \r\n"
					"			ON AppointmentsT.ID = ApptPriInsQ.AppointmentID \r\n"
					"			 LEFT JOIN (SELECT AptInsQ.AppointmentID, InsuranceCoT.Name as InsCoName, InsuranceCoT.PersonID as InsCoID FROM \r\n"
					"			(SELECT * FROM AppointmentInsuredPartyT WHERE Placement = 2) AptInsQ \r\n"
					"			LEFT JOIN InsuredPartyT ON AptInsQ.InsuredPartyID = InsuredPartyT.PersonID \r\n"
					"			LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID ) ApptSecInsQ \r\n"
					"			ON AppointmentsT.ID = ApptSecInsQ.AppointmentID \r\n";

					str += strEnd;
				break;

				case 1:
	
					switch (nSubRepNum) {

				
						case 0: 

							//Inventory Allocations
							// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
							if (! g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
								//(e.lally 2008-11-18) PLID 32070 - Added additional joins so we can use the same subfilters without exceptions
									//The where clause of 1=0 as before should prevent a performance hit
								str = _T("SELECT -1 as AllocationID, -1 AS PatID, -1 AS LocID, "
								" -1 AS ApptID, '' AS AllocationNotes, -1 as AllocationStatus, GetDate() AS Date, "
								" -1 AS AllocationDetailID, -1 AS ProductID, -1 AS ProductItemID, -1 AS Quantity, -1 AS AllocationDetailStatus, "
								" '' AS AllocationDetailNotes, '' AS ProductName, '' AS SerialNum, getDate() as ExpDate, -1 as UserDefinedID, '' as FullName, "
								" '' AS PersonLocation, -1 AS ProvID, getDate() as StartTime, "
								" GetDAte() AS EndTime, '' AS ApptType, '' AS ApptPurpose, -1 AS StatusType, '' AS StatusText, "
								" getDate() AS AllocationStatusDate, "
								" -1  AS DetailStatusType, "
								" '' DetailStatusText, "
								" '' AS ProvName, "
								"  -1 as AllocLocID, -1 as ID, -1 as ShowState "
								"FROM "
								"AppointmentsT LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
								"LEFT JOIN AptTypeT ON AptTypeT.ID = AppointmentsT.AptTypeID "
								"LEFT JOIN AppointmentResourceT ON  AppointmentsT.ID = AppointmentResourceT.AppointmentID "
								"LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
								"LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
								"LEFT JOIN PersonT ON PersonT.ID = PatientsT.PersonID "
								"LEFT JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID "
								"WHERE 1 = 0 ");
							}
							else {
								//(e.lally 2008-11-18) PLID 32070 - Added join to AptTypeT for when the user filters on Appointment type

								str = ("SELECT AllocationID, ApptPatID AS PatID, ApptLocID AS LocID, "
									"AppointmentID AS ApptID, AllocationNotes, AllocationStatus, ApptStartTime AS Date, "
									"AllocationDetailID, ProductID, ProductItemID, Quantity, AllocationDetailStatus, "
									"AllocationDetailNotes, ProductName, SerialNum, ExpDate, UserDefinedID, FullName, "
									"LocationName AS PersonLocation, ResourceID AS ProvID, AllocationListSubQ.StartTime AS StartTime, "
									"EndTime, ApptType, ApptPurpose, AllocationListSubQ.StatusType AS StatusType, StatusText, "
									"AllocationListSubQ.AllocationStatusDate AS AllocationStatusDate, "
									"AllocationListSubQ.DetailStatusType AS DetailStatusType, "
									"AllocationListSubQ.DetailStatusText AS DetailStatusText, "
									"Coalesce(AllocationListSubQ.ProviderName, '') AS ProvName, "
									"LocationID as AllocLocID, "
									"ResourceID AS ID, "
									" ShowState AS ShowState "
									
									"FROM (SELECT "
									"PatientInvAllocationsT.ID AS AllocationID, "
									"PatientInvAllocationsT.PatientID , "
									"PatientInvAllocationsT.LocationID, "
									"PatientInvAllocationsT.AppointmentID, "
									"PatientInvAllocationsT.Notes AS AllocationNotes, "
									"PatientInvAllocationsT.Status AS AllocationStatus, "
									"PatientInvAllocationsT.InputDate, "
									"PatientInvAllocationsT.ProviderID, "

									"PatientInvAllocationDetailsT.ID AS AllocationDetailID, "
									"PatientInvAllocationDetailsT.ProductID, "
									"PatientInvAllocationDetailsT.ProductItemID, "
									"PatientInvAllocationDetailsT.Quantity, "
									"PatientInvAllocationDetailsT.Status AS AllocationDetailStatus, "
									"PatientInvAllocationDetailsT.Notes AS AllocationDetailNotes, "

									"ServiceT.Name AS ProductName, "
									"ProductItemsT.SerialNum, ProductItemsT.ExpDate, "

									"PatientsT.UserDefinedID, "
									"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, "
									"LocationsT.Name AS LocationName, "

									"ProviderPersonT.Last + ', ' + ProviderPersonT.First + ' ' + ProviderPersonT.Middle AS ProviderName, "

									"AppointmentsT.StartTime, AppointmentsT.EndTime, "
									"AptTypeT.Name AS ApptType, "
									"dbo.GetPurposeString(AppointmentsT.ID) AS ApptPurpose, "

									/* (c.haag 2008-01-08 16:15) - This is the granular version */
									"CASE WHEN (BilledCaseHistoryQ.ID Is Not Null AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + ") OR "
									"PatientInvAllocationDetailsT.ID IN (SELECT AllocationDetailID FROM ChargedAllocationDetailsT) THEN 1 "
									"WHEN PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + " THEN 2 "
									"WHEN PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsReleased) + " THEN 3 "
									"ELSE 0 END AS DetailStatusType, "

									"CASE WHEN (BilledCaseHistoryQ.ID Is Not Null AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + ") OR "
									"PatientInvAllocationDetailsT.ID IN (SELECT AllocationDetailID FROM ChargedAllocationDetailsT) THEN 'Completed & Billed' "
									"WHEN PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) + " THEN 'Completed & Not Billed' "
									"WHEN PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsReleased) + " THEN 'Released' "
									"ELSE 'Active' END AS DetailStatusText, "
									
									/* (c.haag 2008-01-08 16:15) - This is the "consistent with the Allocations tab" version */
									"CASE WHEN PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + " AND (PatientInvAllocationDetailsT.Status IS NULL OR PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsActive) + " OR PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsOrder) + ") THEN 0 "
									"WHEN "
									//is linked to a billed case history
									"BilledCaseHistoryQ.ID Is Not Null OR "
									//has no unbilled products that are billable
									"(PatientInvAllocationsT.ID NOT IN (SELECT InvAllocDetailQ.AllocationID FROM PatientInvAllocationDetailsT InvAllocDetailQ WHERE InvAllocDetailQ.Status = " + AsString((long)InvUtils::iadsUsed) + " "
									"			AND InvAllocDetailQ.ID NOT IN (SELECT ChargeQ.AllocationDetailID FROM ChargedAllocationDetailsT ChargeQ "
									"			WHERE ChargeQ.ChargeID IN (SELECT LineItemQ.ID FROM LineItemT LineItemQ WHERE LineItemQ.Deleted = 0)) "
									"			AND InvAllocDetailQ.ProductID IN (SELECT ProductLocQ.ProductID FROM ProductLocationInfoT ProductLocQ WHERE ProductLocQ.Billable = 1 AND ProductLocQ.LocationID = PatientInvAllocationsT.LocationID)) "
									//does not have only released products
									"AND PatientInvAllocationsT.ID NOT IN (SELECT InvAllocQ.ID FROM PatientInvAllocationsT InvAllocQ WHERE InvAllocQ.Status = " + AsString((long)InvUtils::iasCompleted) + " "
									"			AND InvAllocQ.ID NOT IN (SELECT InvAllocDetailQ.AllocationID FROM PatientInvAllocationDetailsT InvAllocDetailQ WHERE InvAllocDetailQ.Status = " + AsString((long)InvUtils::iadsUsed) + "))"
									//does not have only non-billable products
									"AND PatientInvAllocationsT.ID NOT IN (SELECT InvAllocQ.ID FROM PatientInvAllocationsT InvAllocQ WHERE InvAllocQ.Status = " + AsString((long)InvUtils::iasCompleted) + " "
									"			AND InvAllocQ.ID NOT IN (SELECT InvAllocDetailQ.AllocationID FROM PatientInvAllocationDetailsT InvAllocDetailQ WHERE InvAllocDetailQ.ProductID IN "
									"				(SELECT ProductLocQ.ProductID FROM ProductLocationInfoT ProductLocQ WHERE ProductLocQ.Billable = 1 AND ProductLocQ.LocationID = PatientInvAllocationsT.LocationID))) "
									") "
									"THEN 1 "
									"ELSE 2 END AS StatusType, "

									"CASE WHEN PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + " AND (PatientInvAllocationDetailsT.Status IS NULL OR PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsActive) + ") THEN 'Active' "
									"WHEN PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsOrder) + " THEN 'To Be Ordered' "
									"WHEN "
									//is linked to a billed case history
									"BilledCaseHistoryQ.ID Is Not Null OR "
									//has no unbilled products that are billable
									"(PatientInvAllocationsT.ID NOT IN (SELECT InvAllocDetailQ.AllocationID FROM PatientInvAllocationDetailsT InvAllocDetailQ WHERE InvAllocDetailQ.Status = " + AsString((long)InvUtils::iadsUsed) + " "
									"			AND InvAllocDetailQ.ID NOT IN (SELECT ChargeQ.AllocationDetailID FROM ChargedAllocationDetailsT ChargeQ "
									"			WHERE ChargeQ.ChargeID IN (SELECT LineItemQ.ID FROM LineItemT LineItemQ WHERE LineItemQ.Deleted = 0)) "
									"			AND InvAllocDetailQ.ProductID IN (SELECT ProductLocQ.ProductID FROM ProductLocationInfoT ProductLocQ WHERE ProductLocQ.Billable = 1 AND ProductLocQ.LocationID = PatientInvAllocationsT.LocationID)) "
									//does not have only released products
									"AND PatientInvAllocationsT.ID NOT IN (SELECT InvAllocQ.ID FROM PatientInvAllocationsT InvAllocQ WHERE InvAllocQ.Status = " + AsString((long)InvUtils::iasCompleted) + " "
									"			AND InvAllocQ.ID NOT IN (SELECT InvAllocDetailQ.AllocationID FROM PatientInvAllocationDetailsT InvAllocDetailQ WHERE InvAllocDetailQ.Status = " + AsString((long)InvUtils::iadsUsed) + ")) "
									//does not have only non-billable products
									"AND PatientInvAllocationsT.ID NOT IN (SELECT InvAllocQ.ID FROM PatientInvAllocationsT InvAllocQ WHERE InvAllocQ.Status = " + AsString((long)InvUtils::iasCompleted) + " "
									"			AND InvAllocQ.ID NOT IN (SELECT InvAllocDetailQ.AllocationID FROM PatientInvAllocationDetailsT InvAllocDetailQ WHERE InvAllocDetailQ.ProductID IN "
									"				(SELECT ProductLocQ.ProductID FROM ProductLocationInfoT ProductLocQ WHERE ProductLocQ.Billable = 1 AND ProductLocQ.LocationID = PatientInvAllocationsT.LocationID))) "
									") "
									"THEN 'Completed & Billed' "
									"ELSE 'Completed & Not Billed' END AS StatusText, "

									"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN SupplierReturnGroupsT.ReturnDate "
									"WHEN ProductItemsT.Deleted = 1 THEN ProductAdjustmentsT.Date "
									"WHEN LineItemT.ID Is Not Null THEN LineItemT.Date "
									"WHEN BilledCaseHistoryQ.ID Is Not Null THEN BilledCaseHistoryQ.Date "
									"WHEN PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasCompleted) + " THEN COALESCE(AppointmentsT.StartTime,PatientInvAllocationsT.CompletionDate) "
									"ELSE PatientInvAllocationsT.InputDate END AS AllocationStatusDate, "
									"AppointmentsT.PatientID as ApptPatID, AppointmentsT.LocationID as ApptLocID, "
									"AppointmentsT.StartTime as ApptStartTime, "
									"AppointmentResourceT.ResourceID as ResourceID, "
									"AppointmentsT.ShowState AS ShowState, "
									"AppointmentsT.AptTypeID AS AptTypeID "

									"FROM AppointmentsT INNER JOIN PatientInvAllocationsT ON AppointmentsT.ID = PatientInvAllocationsT.AppointmentID "
									"INNER JOIN PatientsT ON PatientInvAllocationsT.PatientID = PatientsT.PersonID "
									"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
									"INNER JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "				
									"LEFT JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
									"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
									"LEFT JOIN ProductItemsT ON PatientInvAllocationDetailsT.ProductItemID = ProductItemsT.ID "
									"LEFT JOIN ProductAdjustmentsT ON ProductItemsT.AdjustmentID = ProductAdjustmentsT.ID "
									"LEFT JOIN SupplierReturnItemsT ON ProductItemsT.ID = SupplierReturnItemsT.ProductItemID "
									"LEFT JOIN SupplierReturnGroupsT ON SupplierReturnItemsT.ReturnGroupID = SupplierReturnGroupsT.ID "
									"LEFT JOIN ServiceT ON PatientInvAllocationDetailsT.ProductID = ServiceT.ID "
									"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "				
									"LEFT JOIN (SELECT * FROM LineItemT WHERE Deleted = 0) AS LineItemT ON ChargedProductItemsT.ChargeID = LineItemT.ID "

									// (j.jones 2009-08-06 09:50) - PLID 35120 - added BilledCaseHistoriesT
									"LEFT JOIN (SELECT CaseHistoryT.ID, BillsT.Date, AllocationID "
									"	FROM CaseHistoryAllocationLinkT "
									"	INNER JOIN CaseHistoryT ON CaseHistoryAllocationLinkT.CaseHistoryID = CaseHistoryT.ID "
									"	INNER JOIN BilledCaseHistoriesT ON CaseHistoryT.ID = BilledCaseHistoriesT.CaseHistoryID "
									"	INNER JOIN BillsT ON BilledCaseHistoriesT.BillID = BillsT.ID "
									"	WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND CaseHistoryT.CompletedDate Is Not Null "
									") AS BilledCaseHistoryQ ON PatientInvAllocationsT.ID = BilledCaseHistoryQ.AllocationID "

									"LEFT JOIN PersonT ProviderPersonT ON PatientInvAllocationsT.ProviderID = ProviderPersonT.ID "
									"LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "

									" " + str + " AND PatientInvAllocationsT.Status <> " + AsString((long)InvUtils::iasDeleted) + " AND PatientInvAllocationDetailsT.Status <> " + AsString((long)InvUtils::iadsDeleted) + " "
									"AND (((PatientInvAllocationDetailsT.Status IN (" + AsString((long)InvUtils::iadsActive) + "," + AsString((long)InvUtils::iadsOrder) + ") AND PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) + ") "
									"OR (PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasCompleted) + ")))) AllocationListSubQ "
									"LEFT JOIN AptTypeT ON AllocationListSubQ.AptTypeID = AptTypeT.ID "
									);

							}
						break;

						default:
							return "";
						break;
					}
				break;

				default:
					return "";
				break;

				}
				return str;
			
		}
	break;
	case 257:
			//EMR / OpReport
			/*
				Version History
				TES 6/29/04 - PLID 9722 - Implemented multi-procedure support
				TES 9/21/2004 - PLID 14154 - Implemented EmrDetailElementsT
				CAH 8/25/2005 - PLID 15142 - Implemented dbo.GetEMRTableSentence
				JMJ 8/9/2006 - PLID 21872 - Removed EmrDetailElementsT
				// (j.gruber 2007-01-04 13:04) - PLID 24035 - support multiple providers
				// (c.haag 2008-06-17 18:25) - PLID 30319 - Changed name calculation to factor in EMR text macros
			*/
			return _T("SELECT dbo.GetEMNProviderList(EMRMasterT.ID) AS DocName, "
			"EMRMasterT.PatientID AS PatID, (PatientPersonT.Last + ', ' + PatientPersonT.First + ' ' + PatientPersonT.Middle + ' ' + PatientPersonT.Title) AS PatientName, "
			"PatientPersonT.Address1 AS PatAddress1, "
			"PatientPersonT.Address2 AS PatAddress2, "
			"PatientPersonT.City AS PatCity, "
			"PatientPersonT.State AS PatState, "
			"PatientPersonT.Zip AS PatZip, "
			"EMRMasterT.PatientAge, "
			"EMRMasterT.PatientGender, "
			"EMRMasterT.ID AS EMRID, "
			"dbo.GetEmrString(EmrMasterT.ID) AS ProcedureName, "
			"EMRDetailsT.ID AS DetailID, "
			"CASE WHEN EmrInfoT.ID = " + AsString((long)EMR_BUILT_IN_INFO__TEXT_MACRO) + " THEN EMRDetailsT.MacroName ELSE EmrInfoT.Name END AS DetailName, "
			"(CASE WHEN EMRInfoT.DataType=1 THEN EmrDetailsT.Text WHEN EMRInfoT.DataType=4 THEN '<Image>' WHEN EMRInfoT.DataType=7 THEN dbo.GetEMRTableSentence(EmrDetailsT.ID) ELSE EMRDataT.Data END) AS DetailData, "
			"EMRDetailsT.OrderID, "
			"EMRMasterT.Date AS TDate, "
			"EMRMasterT.InputDate AS IDate, "
			"EMRMasterT.LocationID AS LocID, "
			"LocationsT.Name AS LocationName, "
			"LocationsT.Address1 AS LocAddress1, "
			"LocationsT.Address2 AS LocAddress2, "
			"LocationsT.City AS LocCity, "
			"LocationsT.State AS LocState, "
			"LocationsT.Zip AS LocZip, "
			"LocationsT.Phone AS LocPhone "
			"FROM (SELECT * FROM EMRMasterT WHERE Deleted = 0) AS EMRMasterT "
			"LEFT JOIN (SELECT * FROM EMRDetailsT WHERE Deleted = 0) AS EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
			"LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
			"LEFT JOIN EMRSelectT ON EMRDetailsT.ID = EMRSelectT.EMRDetailID "
			"LEFT JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
			"LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
			"LEFT JOIN (SELECT * FROM PersonT) AS PatientPersonT ON EMRMasterT.PatientID = PatientPersonT.ID "
			"ORDER BY PatID, EMRID, OrderID");
			break;
	/*case 294:
		//Prescription
	
		return _T("SELECT PersonT.ID, PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last AS PatientName, "
		   "PersonT.HomePhone AS HomePhone, PatientMedications.PrescriptionDate, DrugList.Name, "
		   "PatientMedications.Description, PatientMedications.RefillsAllowed, PatientMedications.PillsPerBottle, "
		   "PatientsT.MainPhysician, PersonT.Address1, PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip AS PatCityStateZip, "
		   "PersonT.Location "
		   "FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID INNER JOIN PatientMedications ON "
		   "PatientsT.PersonID = PatientMedications.PatientID INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID");
		break;*/

	case 365:
		{
		//Billing Followup
		/* Revision History
			--TS 12/19/02: Filtered out quotes.
			DRT 5/29/03 - The query generated by the followup tab is passed in via the strExtendedSql, and then input as a subquery into this function.
					Generating a .ttx file is a pain in the ass, I had to copy the code out of billingfollowup.cpp, and then make fake filters that
					work (ttx files only care for the field names/types, not the data that results) and stuff it in here to generate it.
			DRT 7/16/03 - This query really sucks.  Someone (Josh!!) has foolishly decided to place numerous *'s in the select clause.  Which means that
					this report is constantly getting messed up anytime someone adds or removes a field from the database, or changes it's type at all.
					I removed these *'s, and replaced them with much more friendly fields.  The tables that were previous used are:  BillsT.*, ChargesT.*,
					LineItemT.*, PersonT.*, SubQ.*.  Using *'s also has a side pitfall of not being able to make LW groups out of it.
				I also recreated the ttx file at the bottom and upped the query version.  Remember to do that in the future if you change the field definitions!
			DRT 7/16/03 - Added InsuranceCoT into the from and InsCoName into the select clause, rather silly we didn't have that before.
			DRT 7/30/03 - Added Privacy fields
			JMJ 4/18/05 - Added Last Sent Date
			// (j.gruber 2008-03-18 10:18) - PLID 29303 - changed the values that should've been pulling from the insurance contact to pull from there
			// (j.jones 2009-08-11 09:11) - PLID 28131 - added LocationID and name
			// (j.jones 2010-04-07 17:29) - PLID 15224 - removed ChargesT.EMG, obsolete field
			// (f.dinatale 2010-10-18) - PLID 40876 - Added SSN Masking.
			//(r.wilson 10/2/2012) plid 53082 - Replace hardcoded SendTypes with enumerated values
			// (j.jones 2013-08-20 08:50) - PLID 42319 - added RespAssignmentDate
			// (j.gruber 2014-03-28 11:44) - PLID 61483 - updated for ICD-10
			// (r.gonet 2016-04-07) - NX-100072 - FirstConditionDate was split into multiple columns. Changed to a case.
		*/
		CString strRet;

		strRet.Format("SELECT "
//		"SubQ.*, "
				"SubQ.BillID, SubQ.PatientID, SubQ.LocationID, SubQ.LocName, SubQ.TC, SubQ.TP, SubQ.Bal, SubQ.InsuredPartyID, SubQ.HCFASetupGroupID, SubQ.LastDate, SubQ.PatName, SubQ.BillDate, "
				"SubQ.InputDate, SubQ.LastTracerDate, "
				"BillsT.PatientID AS PatID,  LineItemT.Description AS ChargeDesc, "
//		"BillsT.*, "
				"BillsT.Date AS BillDate, BillsT.RelatedToEmp, BillsT.RelatedToAutoAcc, BillsT.State AS AccidentState, BillsT.RelatedToOther, BillsT.ConditionDate, "
				"CASE BillsT.ConditionDateType "
				"	WHEN %li THEN BillsT.FirstVisitOrConsultationDate "
				"	WHEN %li THEN BillsT.InitialTreatmentDate "
				"	WHEN %li THEN BillsT.LastSeenDate "
				"	WHEN %li THEN BillsT.AcuteManifestationDate "
				"	WHEN %li THEN BillsT.LastXRayDate "
				"	WHEN %li THEN BillsT.HearingAndPrescriptionDate "
				"	WHEN %li THEN BillsT.AssumedCareDate "
				"	WHEN %li THEN BillsT.RelinquishedCareDate "
				"	WHEN %li THEN BillsT.AccidentDate "
				"	ELSE NULL "
				"END AS FirstConditionDate, "
				"BillsT.NoWorkFrom, BillsT.NoWorkTo, BillsT.HospFrom, BillsT.HospTo, BillsT.OutsideLab, BillsT.OutsideLabCharges, "
				"BillsT.MedicaidResubmission, BillsT.OriginalRefNo, BillsT.PriorAuthNum, BillsT.HCFABlock19, BillsT.Note, BillsT.InputDate, BillsT.Description AS BillDesc, "
				"BillsT.FormType, "
				
				"ICD9T1.CodeNumber as ICD9Code1, \r\n "
				"ICD9T2.CodeNumber as ICD9Code2, \r\n "
				"ICD9T3.CodeNumber as ICD9Code3, \r\n "
				"ICD9T4.CodeNumber as ICD9Code4, \r\n "

				"ICD10T1.CodeNumber as ICD10Code1, \r\n "
				"ICD10T2.CodeNumber as ICD10Code2, \r\n "
				"ICD10T3.CodeNumber as ICD10Code3, \r\n "
				"ICD10T4.CodeNumber as ICD10Code4, \r\n "

				"ICD9T1.CodeDesc as ICD9CodeDesc1, \r\n "
				"ICD9T2.CodeDesc as ICD9CodeDesc2, \r\n "
				"ICD9T3.CodeDesc as ICD9CodeDesc3, \r\n "
				"ICD9T4.CodeDesc as ICD9CodeDesc4, \r\n "

				"ICD10T1.CodeDesc as ICD10CodeDesc1, \r\n "
				"ICD10T2.CodeDesc as ICD10CodeDesc2, \r\n "
				"ICD10T3.CodeDesc as ICD10CodeDesc3, \r\n "
				"ICD10T4.CodeDesc as ICD10CodeDesc4,  \r\n "

//		"ChargesT.*, "
				"ChargesT.CPTModifier, ChargesT.CPTModifier2, ChargeWhichCodesFlatV.WhichCodes, ChargesT.TaxRate, ChargesT.Quantity, ChargesT.ServiceDateFrom, ChargesT.ServiceDateTo, "
				"PlaceOfServiceCodesT.PlaceCodes AS POSDesignation, ChargesT.ServiceType AS TOS, ChargesT.EPSDT, ChargesT.COB, ChargesT.TaxRate2, "
//		"LineItemT.*, "
				"LineItemT.Type, LineItemT.Amount AS LineAmount, LineItemT.Description AS LineDesc, LineItemT.Date AS LineDate, LineItemT.InputDate, LineItemT.InputName, "
//		"PersonT.*, "
				"PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Gender, PersonT.Suffix, "
				"PersonT.Title, PersonT.HomePhone, PersonT.PrivHome, PersonT.WorkPhone, PersonT.PrivWork, PersonT.Extension, PersonT.CellPhone, PersonT.OtherPhone, "
				"PersonT.Email, PersonT.Pager, PersonT.Fax, PersonT.BirthDate, dbo.MaskSSN(PersonT.SocialSecurity, %s), PersonT.FirstContactDate, PersonT.InputDate AS PersonInputDate, "
				"PersonT.WarningMessage, PersonT.DisplayWarning, PersonT.Note AS PersonNote, PersonT.Company AS PersonCompany, PersonT.EmergFirst, PersonT.EmergLast, "
				"PersonT.EmergHPhone, PersonT.EmergWPhone, PersonT.EmergRelation, PersonT.Spouse, PersonT.PrivCell, "
				"PersonT.PrivFax, PersonT.PrivOther, PersonT.PrivPager, PersonT.PrivEmail, "
//		done replacing stupid *'s
				"InsContactT.Last AS InsCoLastName, InsContactT.First AS InsCoFirstName, "
				"InsurancePersonT.Address1 AS InsuranceAddress1, InsurancePersonT.Address2 AS InsuranceAddress2, "
				"InsurancePersonT.City AS InsuranceCity, InsurancePersonT.State AS InsuranceState, InsurancePersonT.Zip AS InsuranceZip, "
				"InsContactT.WorkPhone AS InsurancePhone, InsContactT.Extension AS InsuranceExt, InsuredPartyT.IDForInsurance AS InsuranceID, "
				"InsuredPartyT.Employer AS Employer, InsuredPartyT.PolicyGroupNum AS PolicyGroupNum, "
				"InsuredPartyPersonT.First AS InsuredFirstName, InsuredPartyPersonT.Last AS InsuredLastName, InsuredPartyPersonT.Middle AS InsuredMiddleName, "
				"InsuredPartyPersonT.Address1 AS InsuredAddress1, InsuredPartyPersonT.Address2 AS InsuredAddress2, InsuredPartyPersonT.City AS InsuredCity, "
				"InsuredPartyPersonT.State AS InsuredState, InsuredPartyPersonT.Zip AS InsuredZip, InsuredPartyPersonT.Birthdate AS InsuredBirthdate, "
				"LocationsT.Name AS LocName, LocationsT.Address1 AS LocAddress1, LocationsT.Address2 AS LocAddress2, LocationsT.City AS LocCity, LocationsT.State AS LocState, "
				"LocationsT.Zip AS LocZip, LocationsT.Phone AS LocPhone, LocationsT.Phone2 AS LocPhone2, LocationsT.Fax AS LocFax, LocationsT.EIN AS LocTaxID, "
				"LocationsT.TollFreePhone AS LocTollFreePhone, "
				"PlaceOfServiceT.Name AS POSName, PlaceOfServiceT.Address1 AS POSAddress1, PlaceOfServiceT.Address2 AS POSAddress2, PlaceOfServiceT.City AS POSCity, "
				"PlaceOfServiceT.State AS POSState, PlaceOfServiceT.Zip AS POSZip, PlaceOfServiceT.Phone AS POSPhone, PlaceOfServiceT.Phone2 AS POSPhone2, "
				"PlaceOfServiceT.Fax AS POSFax, PlaceOfServiceT.EIN AS POSTaxID, PlaceOfServiceT.TollFreePhone AS POSTollFreePhone, "
				"DoctorsT.First + ' ' + DoctorsT.Middle + ' ' + DoctorsT.Last + ' ' + DoctorsT.Title AS DocFullName, DoctorsT.First AS DocFirstName, DoctorsT.Last AS DocLastName, DoctorsT.Middle AS DocMiddleName, DoctorsT.Title AS DocTitle, "
				"DoctorsT.Address1 AS DocAddress1, DoctorsT.Address2 AS DocAddress2, DoctorsT.City AS DocCity, "
				"DoctorsT.State AS DocState, DoctorsT.Zip AS DocZip, DoctorsT.Birthdate AS DocBirthdate, ProvidersT.[Fed Employer ID] AS DocEIN, ProvidersT.UPIN AS DocUPIN, PatientsT.UserDefinedID, "
				"dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount, "
				"CPTCodeT.Code AS ItemCode, LineItemT.ID AS LineID, InsuranceCoT.Name AS InsCoName, "
				"HistoryQ.LastDate AS LastSentDate, SubQ.RespAssignmentDate "
				" "
				"FROM %s "
				" "
				"INNER JOIN BillsT ON SubQ.BillID = BillsT.ID "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number "
				"LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"LEFT JOIN InsuredPartyT ON SubQ.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN PersonT AS InsuredPartyPersonT ON InsuredPartyT.PersonID = InsuredPartyPersonT.ID "
				"LEFT JOIN PersonT AS InsurancePersonT ON InsuredPartyT.InsuranceCoID = InsurancePersonT.ID "
				"LEFT JOIN PersonT AS InsContactT ON InsuredPartyT.InsuranceContactID = InsContactT.ID "
				"LEFT JOIN PersonT AS DoctorsT ON ChargesT.DoctorsProviders = DoctorsT.ID "
				"LEFT JOIN ProvidersT ON DoctorsT.ID = ProvidersT.PersonID "
				"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
				"LEFT JOIN LocationsT AS PlaceOfServiceT ON BillsT.Location = PlaceOfServiceT.ID "
				
				"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID \r\n "
				"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat4V.ICD9Diag1ID = ICD9T1.ID \r\n "
				"LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat4V.ICD9Diag2ID = ICD9T2.ID \r\n "
				"LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat4V.ICD9Diag3ID = ICD9T3.ID \r\n "
				"LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat4V.ICD9Diag4ID = ICD9T4.ID \r\n "
				"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat4V.ICD10Diag1ID = ICD10T1.ID \r\n"
				"LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat4V.ICD10Diag2ID = ICD10T2.ID \r\n "
				"LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat4V.ICD10Diag3ID = ICD10T3.ID \r\n "
				"LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat4V.ICD10Diag4ID = ICD10T4.ID \r\n "

				"LEFT JOIN ChargeWhichCodesFlatV ON ChargesT.ID = ChargeWhichCodesFlatV.ChargeID \r\n "

				"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
				"LEFT JOIN "
				"	(SELECT Max(Date) AS LastDate, ClaimHistoryT.BillID, InsuredPartyID "
				"	FROM ClaimHistoryT "
				//(r.wilson 10/2/2012) plid 53082 - Line Below use to be "   WHERE SendType >= 0 "
				"	WHERE SendType >= %li " 
				"	GROUP BY ClaimHistoryT.BillID, InsuredPartyID "
				"	) HistoryQ "
				"ON SubQ.BillID = HistoryQ.BillID AND SubQ.InsuredPartyID = HistoryQ.InsuredPartyID "
				"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND BillsT.EntryType = 1", 
				ConditionDateType::cdtFirstVisitOrConsultation444,
				ConditionDateType::cdtInitialTreatmentDate454,
				ConditionDateType::cdtLastSeenDate304,
				ConditionDateType::cdtAcuteManifestation453,
				ConditionDateType::cdtLastXray455,
				ConditionDateType::cdtHearingAndPrescription471,
				ConditionDateType::cdtAssumedCare090,
				ConditionDateType::cdtRelinquishedCare91,
				ConditionDateType::cdtAccident439,
				((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"),
				strExtendedSql,
				ClaimSendType::Electronic);
		return _T(strRet);
		}
		break;

	case 443:
		{
		//Billing Followup List

		/* Revision History
			JMJ 9/23/05 - Added Last Sent Date
			JMJ 4/9/2006 - Added RespTypeName, InputDate, TotalCharges, TotalPayments
			(j.jones 2011-03-10 11:21) - PLID 41792 - added the patient's primary insurance company name
			// (j.jones 2011-04-01 14:34) - PLID 43110 - the primary insurance company is now the primary
			// company for the same type (Medical or Vision) as the displayed responsibility,
			// or just Medical if we are showing patient resp.
			//(r.wilson 10/2/2012) plid 53082 - Replace hardcoded SendTypes with enumerated values
			// (j.jones 2013-08-20 08:50) - PLID 42319 - added RespAssignmentDate
			// (s.tullis 2014-06-27 16:55) - PLID 62578 - Update Billing Follow Up report to include the “Status” field, "On Hold" field, and "Status Date" field. None of these need to be on the report by default.
			// (r.gonet 07/07/2014) - PLID 62571 - Replaced hard coded BillStatusT.Type value with a value from an enumeration
			// (a.wilson 2014-07-10 11:35) - PLID 62519 - added status note and last status note date.
		*/
		CString strRet;

		strRet.Format("SELECT BillsT.ID, BillsT.Date, PersonT.Last, PersonT.Middle, PersonT.First, "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, SubQ.Bal, "
				"SubQ.TC AS TotalCharges, SubQ.TP AS TotalPayments, SubQ.InputDate, "
				"InsuranceCoT.Name AS InsCoName, InsurancePersonT.WorkPhone AS InsPhone, HistoryQ.LastDate AS LastSentDate, "
				"CASE WHEN RespTypeT.TypeName Is Null THEN 'Patient' ELSE RespTypeT.TypeName END AS RespTypeName, "
				"PrimaryInsuranceCoQ.Name AS PriInsCoName, SubQ.RespAssignmentDate, "
				"CASE WHEN BillStatusT.Type=%li THEN 'Yes' ELSE 'No' END AS OnHold, BillStatusT.Name as BillStatus, BillsT.StatusModifiedDate as StatusDate, "
				"NoteDataT.Note AS StatusNote, NoteDataT.NoteInputDate as LastStatusNoteDate "
				//"FROM (SELECT BillsT.*, ID AS BillID, convert(money, 0) AS Bal, convert(money, 0) AS TC, convert(money, 0) AS TP FROM BillsT) AS SubQ "
				"FROM %s "
				"INNER JOIN BillsT ON SubQ.BillID = BillsT.ID "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"LEFT JOIN BillStatusT ON BillStatusT.ID= BillsT.StatusID "
				"LEFT JOIN NoteDataT ON BillsT.StatusNoteID = NoteDataT.ID "
				"LEFT JOIN InsuredPartyT ON SubQ.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"LEFT JOIN PersonT AS InsurancePersonT ON InsuredPartyT.InsuranceContactID = InsurancePersonT.ID "
				"LEFT JOIN "
				"	(SELECT InsuredPartyT.PatientID, InsuranceCoT.Name, RespTypeT.CategoryType "
				"	FROM InsuranceCoT "
				"	INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
				"	INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"	WHERE RespTypeT.CategoryPlacement = 1 "
				"	) AS PrimaryInsuranceCoQ ON "
				"		PatientsT.PersonID = PrimaryInsuranceCoQ.PatientID "
				"		AND Coalesce(RespTypeT.CategoryType, 1) = PrimaryInsuranceCoQ.CategoryType "
				"LEFT JOIN "
				"	(SELECT Max(Date) AS LastDate, ClaimHistoryT.BillID, InsuredPartyID "
				"	FROM ClaimHistoryT "
				//(r.wilson 10/2/2012) plid 53082 - Line below use to be "   WHERE SendType >= 0 "
				"	WHERE SendType >= %li " 
				"	GROUP BY ClaimHistoryT.BillID, InsuredPartyID "
				"	) HistoryQ "
				"ON SubQ.BillID = HistoryQ.BillID AND SubQ.InsuredPartyID = HistoryQ.InsuredPartyID "
				"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND BillsT.EntryType = 1 "
				"GROUP BY BillsT.ID, BillsT.Date, PersonT.Last, PersonT.Middle, PersonT.First, "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, "
				"SubQ.Bal, SubQ.TC, SubQ.TP, SubQ.InputDate, SubQ.RespAssignmentDate, InsuranceCoT.Name, InsurancePersonT.WorkPhone, "
				"HistoryQ.LastDate, RespTypeT.TypeName, PrimaryInsuranceCoQ.Name, BillStatusT.Type, BillStatusT.Name,BillsT.StatusModifiedDate, NoteDataT.NoteInputDate, NoteDataT.Note ", 
				(long)EBillStatusType::OnHold, strExtendedSql, ClaimSendType::Electronic);
		return _T(strRet);
		}
		break;


	case 393:
	case 394:
		//Pull this out of the scheduler stuff, since they use the same .rpt file.
		return GetSqlScheduler(nSubLevel, nSubRepNum);
		break;

	// (j.jones 2010-04-28 12:13) - PLID 35591 - moved from Preview to Patients tab of reports
	//case 407: //History Tab (PP)

	case 444:
		//Medications Tab (PP)
		/*	Version History
			JMJ 9/5/03 - Created.
			JMM 2/3/04 - Added RefillsAllowed and PillsperBottle and made it editable
			(c.haag 2007-02-02 17:12) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
			TES 2/10/2009 - PLID 33002 - Renamed Description to PatientExplanation and PillsPerBottle to Quantity, but
					kept them aliased to the old names to avoid messing up any custom reports out there.
			TES 3/5/2009 - PLID 33068 - Added SureScripts fields
			(d.thompson 2009-04-02) - PLID 33571 - added strength unit
			TES 4/2/2009 - PLID 33750 - Strength and DosageForm now pull from DrugList
			TES 5/11/2009 - PLID 28519 - Added SampleExpirationDate
		*/
		return _T("SELECT PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"LEFT(EMRDataT.Data,255) AS MedName, PatientMedications.PatientExplanation AS Description, RefillsAllowed, Quantity AS PillsPerBottle, PatientMedications.Unit, PatientMedications.MedicationID AS MedID, "
			"PatientMedications.PrescriptionDate AS Date, PersonT.Location AS LocID, PatientsT.MainPhysician AS ProvID, "
			"PatientMedications.DaysSupply, PatientMedications.NoteToPharmacist, PatientMedications.AllowSubstitutions, "
			"PatientMedications.PriorAuthorization, PatientMedications.PriorAuthorizationIsSample, DrugList.Strength, "
			"DrugDosageFormsT.Name AS DosageForm, dbo.GetPrescriptionDiagList(PatientMedications.ID) AS DiagnosisCodeList, "
			"StrengthUnitT.Name AS StrengthUnit, PatientMedications.SampleExpirationDate "
			"FROM PatientMedications "
			"INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
			"INNER JOIN PersonT ON PatientMedications.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN DrugDosageFormsT ON DrugList.DosageFormID = DrugDosageFormsT.ID "
			"LEFT JOIN DrugStrengthUnitsT AS StrengthUnitT ON DrugList.StrengthUnitID = StrengthUnitT.ID "
			"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"WHERE PatientMedications.Deleted = 0 "
			"ORDER BY PrescriptionDate Desc");
		break;

	case 448:
		//Contacts General - Providers (PP)
		/*	Version History
			DRT 10/7/03 - Created.
			TES 9/8/2008 - PLID 27727 - Dropped DefLocationID, now uses PersonT.Location.
		*/
		return _T("SELECT PersonT.ID AS ProviderID, Company, Last, First, Middle, Last + ', ' + First + ' ' + Middle AS FullName, PrefixT.Prefix, Title, PersonT.Address1, PersonT.Address2,  "
			"PersonT.City, PersonT.State, PersonT.Zip, Spouse, BirthDate, CASE WHEN PersonT.Gender = 1 THEN 'Male' WHEN PersonT.Gender = 2 THEN 'Female' ELSE '' END AS Gender,  "
			"SocialSecurity, HomePhone, WorkPhone, Extension, CellPhone, OtherPhone, Pager, PersonT.Fax, Email, EmergFirst, EmergLast, EmergHPhone, EmergWPhone,  "
			"Note, LocationsT.Name AS DefLocation,  "
			"/*Contact Specific*/ "
			"[Fed Employer ID], [DEA Number], [BCBS Number], [Medicare Number], [Medicaid Number], [Workers Comp Number], [Other ID Number], [Other ID Description],  "
			"UPIN, ProvidersT.EIN, License, AccountNumber,  "
			"/*Custom Info*/ "
			"Custom1SubQ.Name AS Field1Name, Custom1SubQ.TextParam AS Field1,  "
			"Custom2SubQ.Name AS Field2Name, Custom2SubQ.TextParam AS Field2,  "
			"Custom3SubQ.Name AS Field3Name, Custom3SubQ.TextParam AS Field3,  "
			"Custom4SubQ.Name AS Field4Name, Custom4SubQ.TextParam AS Field4 "
			"FROM  "
			"PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
			"LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID "
			"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
			"/* custom stuff */  "
			"LEFT JOIN   "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 6) Custom1SubQ ON PersonT.ID = Custom1SubQ.PersonID  "
			"LEFT JOIN   "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 7) Custom2SubQ ON PersonT.ID = Custom2SubQ.PersonID  "
			"LEFT JOIN  "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 8) Custom3SubQ ON PersonT.ID = Custom3SubQ.PersonID  "
			"LEFT JOIN  "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 9) Custom4SubQ ON PersonT.ID = Custom4SubQ.PersonID  "
			"/* end custom stuff */");
		break;

	case 449:
		//Contacts General - Users (PP)
		/*	Version History
			DRT 10/7/03 - Created.
			TES 9/8/2008 - PLID 27727 - Added DefLocation
		*/
		return _T("SELECT PersonT.ID AS ProviderID, Company, Last, First, Middle, Last + ', ' + First + ' ' + Middle AS FullName, PrefixT.Prefix, Title, PersonT.Address1, PersonT.Address2,  "
			"PersonT.City, PersonT.State, PersonT.Zip, Spouse, BirthDate, CASE WHEN PersonT.Gender = 1 THEN 'Male' WHEN PersonT.Gender = 2 THEN 'Female' ELSE '' END AS Gender,  "
			"SocialSecurity, HomePhone, WorkPhone, Extension, CellPhone, OtherPhone, Pager, PersonT.Fax, Email, EmergFirst, EmergLast, EmergHPhone, EmergWPhone,  "
			"Note, LocationsT.Name AS DefLocation, "
			"/*Contact Specific*/ "
			"UsersT.NationalEmplNumber, PatientCoordinator, UserName, Administrator,  "
			"/*Custom Info*/ "
			"Custom1SubQ.Name AS Field1Name, Custom1SubQ.TextParam AS Field1,  "
			"Custom2SubQ.Name AS Field2Name, Custom2SubQ.TextParam AS Field2,  "
			"Custom3SubQ.Name AS Field3Name, Custom3SubQ.TextParam AS Field3,  "
			"Custom4SubQ.Name AS Field4Name, Custom4SubQ.TextParam AS Field4 "
			"FROM  "
			"PersonT INNER JOIN UsersT ON PersonT.ID = UsersT.PersonID AND UsersT.PersonID > 0 "
			"LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID "
			"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
			"/* custom stuff */  "
			"LEFT JOIN   "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 6) Custom1SubQ ON PersonT.ID = Custom1SubQ.PersonID  "
			"LEFT JOIN   "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 7) Custom2SubQ ON PersonT.ID = Custom2SubQ.PersonID  "
			"LEFT JOIN  "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 8) Custom3SubQ ON PersonT.ID = Custom3SubQ.PersonID  "
			"LEFT JOIN  "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 9) Custom4SubQ ON PersonT.ID = Custom4SubQ.PersonID  "
			"/* end custom stuff */");
		break;

	case 450:
		//Contacts General - Suppliers (PP)
		/*	Version History
			DRT 10/7/03 - Created.
			TES 9/8/2008 - PLID 27727 - Added DefLocation
		*/
		return _T("SELECT PersonT.ID AS ProviderID, Company, Last, First, Middle, Last + ', ' + First + ' ' + Middle AS FullName, PrefixT.Prefix, Title, PersonT.Address1, PersonT.Address2,  "
			"PersonT.City, PersonT.State, PersonT.Zip, Spouse, BirthDate, CASE WHEN PersonT.Gender = 1 THEN 'Male' WHEN PersonT.Gender = 2 THEN 'Female' ELSE '' END AS Gender,  "
			"SocialSecurity, HomePhone, WorkPhone, Extension, CellPhone, OtherPhone, Pager, PersonT.Fax, Email, EmergFirst, EmergLast, EmergHPhone, EmergWPhone,  "
			"Note, LocationsT.Name AS DefLocation,  "
			"/*Contact Specific*/ "
			"SupplierT.CCNumber AS PaymentMethod, "
			"/*Custom Info*/ "
			"Custom1SubQ.Name AS Field1Name, Custom1SubQ.TextParam AS Field1,  "
			"Custom2SubQ.Name AS Field2Name, Custom2SubQ.TextParam AS Field2,  "
			"Custom3SubQ.Name AS Field3Name, Custom3SubQ.TextParam AS Field3,  "
			"Custom4SubQ.Name AS Field4Name, Custom4SubQ.TextParam AS Field4 "
			"FROM  "
			"PersonT INNER JOIN SupplierT ON PersonT.ID = SupplierT.PersonID "
			"LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID "
			"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
			"/* custom stuff */  "
			"LEFT JOIN   "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 6) Custom1SubQ ON PersonT.ID = Custom1SubQ.PersonID  "
			"LEFT JOIN   "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 7) Custom2SubQ ON PersonT.ID = Custom2SubQ.PersonID  "
			"LEFT JOIN  "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 8) Custom3SubQ ON PersonT.ID = Custom3SubQ.PersonID  "
			"LEFT JOIN  "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 9) Custom4SubQ ON PersonT.ID = Custom4SubQ.PersonID  "
			"/* end custom stuff */");
		break;

	case 451:
		//Contacts General - Other Contacts (PP)
		/*	Version History
			DRT 10/7/03 - Created.
			TES 9/8/2008 - PLID 27727 - Added DefLocation
		*/
		return _T("SELECT PersonT.ID AS ProviderID, Company, Last, First, Middle, Last + ', ' + First + ' ' + Middle AS FullName, PrefixT.Prefix, Title, PersonT.Address1, PersonT.Address2,  "
			"PersonT.City, PersonT.State, PersonT.Zip, Spouse, BirthDate, CASE WHEN PersonT.Gender = 1 THEN 'Male' WHEN PersonT.Gender = 2 THEN 'Female' ELSE '' END AS Gender,  "
			"SocialSecurity, HomePhone, WorkPhone, Extension, CellPhone, OtherPhone, Pager, PersonT.Fax, Email, EmergFirst, EmergLast, EmergHPhone, EmergWPhone,  "
			"Note, LocationsT.Name AS DefLocation,  "
			"/*Contact Specific*/ "
			"ContactsT.Nurse, ContactsT.Anesthesiologist,  "
			"/*Custom Info*/ "
			"Custom1SubQ.Name AS Field1Name, Custom1SubQ.TextParam AS Field1,  "
			"Custom2SubQ.Name AS Field2Name, Custom2SubQ.TextParam AS Field2,  "
			"Custom3SubQ.Name AS Field3Name, Custom3SubQ.TextParam AS Field3,  "
			"Custom4SubQ.Name AS Field4Name, Custom4SubQ.TextParam AS Field4 "
			"FROM  "
			"PersonT INNER JOIN ContactsT ON PersonT.ID = ContactsT.PersonID "
			"LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID "
			"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
			"/* custom stuff */  "
			"LEFT JOIN   "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 6) Custom1SubQ ON PersonT.ID = Custom1SubQ.PersonID  "
			"LEFT JOIN   "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 7) Custom2SubQ ON PersonT.ID = Custom2SubQ.PersonID  "
			"LEFT JOIN  "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 8) Custom3SubQ ON PersonT.ID = Custom3SubQ.PersonID  "
			"LEFT JOIN  "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 9) Custom4SubQ ON PersonT.ID = Custom4SubQ.PersonID  "
			"/* end custom stuff */");
		break;

	case 452:
		//Contacts General - Ref Phys (PP)
		/*	Version History
			DRT 10/7/03 - Created.
			// (j.gruber 2006-11-08 11:16) - PLID 23353 - added NPI to the report
			TES 9/8/2008 - PLID 27727 - Added DefLocation
		*/
 		return _T("SELECT PersonT.ID AS ProviderID, Company, Last, First, Middle, Last + ', ' + First + ' ' + Middle AS FullName, PrefixT.Prefix, Title, PersonT.Address1, PersonT.Address2,  "
			"PersonT.City, PersonT.State, PersonT.Zip, Spouse, BirthDate, CASE WHEN PersonT.Gender = 1 THEN 'Male' WHEN PersonT.Gender = 2 THEN 'Female' ELSE '' END AS Gender,  "
			"SocialSecurity, HomePhone, WorkPhone, Extension, CellPhone, OtherPhone, Pager, PersonT.Fax, Email, EmergFirst, EmergLast, EmergHPhone, EmergWPhone,  "
			"Note, LocationsT.Name AS DefLocation,  "
			"/*Contact Specific*/ "
			"ReferringPhysT.ReferringPhyID, ReferringPhysT.UPIN, ReferringPhysT.BlueShieldID, ReferringPhysT.FedEmployerID, ReferringPhysT.DEANumber, ReferringPhysT.MedicareNumber,  "
			"ReferringPhysT.WorkersCompNumber, ReferringPhysT.OtherIDNumber, ReferringPhysT.License, ReferringPhysT.MedicaidNumber, ReferringPhysT.NPI, "
			"/*Custom Info*/ "
			"Custom1SubQ.Name AS Field1Name, Custom1SubQ.TextParam AS Field1,  "
			"Custom2SubQ.Name AS Field2Name, Custom2SubQ.TextParam AS Field2,  "
			"Custom3SubQ.Name AS Field3Name, Custom3SubQ.TextParam AS Field3,  "
			"Custom4SubQ.Name AS Field4Name, Custom4SubQ.TextParam AS Field4 "
			"FROM  "
			"PersonT INNER JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
			"LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID "
			"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
			"/* custom stuff */  "
			"LEFT JOIN   "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 6) Custom1SubQ ON PersonT.ID = Custom1SubQ.PersonID  "
			"LEFT JOIN   "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 7) Custom2SubQ ON PersonT.ID = Custom2SubQ.PersonID  "
			"LEFT JOIN  "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 8) Custom3SubQ ON PersonT.ID = Custom3SubQ.PersonID  "
			"LEFT JOIN  "
			"(SELECT CustomFieldsT.Name, PersonID, Textparam   "
			"FROM CustomFieldsT INNER JOIN CustomFieldDataT ON CustomFieldsT.ID = CustomFieldDataT.FieldID   "
			"WHERE CustomFieldsT.ID = 9) Custom4SubQ ON PersonT.ID = Custom4SubQ.PersonID  "
			"/* end custom stuff */");
		break;

	case 453:
		//To-Do Alarm Preview (PP)
		//Note:  Make sure you up the size of the 'Notes' field to memo / 4000 when making a .ttx file.
		/*	Version History
			DRT 10/15/03 - Created.  Modified from Patient todo list report query because that report does not include
					contacts, so cannot really be used to preview from the todo alarm.
			DRT 10/28/2003 - PLID 9928 - Added UserName field.  I think it's better to show that (since it shows in the dialog itself) than
					to show the Last, First setup.  Also changed the grouping order on this report.
			DRT 5/26/2004 - PLID 12592 - Removed Inquiries.
			TES 5/2/2008 - PLID 28588 - Changed the JOIN with PersonT from INNER to LEFT, you can have ToDos without persons
					associated now, if it's an inventory item with no supplier.
			(c.haag 2008-06-30 12:31) - PLID 30565 - Updated to new todo multi-assignee structure
			(b.spivey August 21, 2014) - PLID 57119 - Added BirthDate
			(d.thompson 2016-08-22) - NX-101901 - Added categoryID for filtering 
		*/
		return _T("SELECT TaskType, Task, Priority AS Priority, UserDefinedID, SubQ.PatID AS PatID, NULL AS AssignID,  "
			"SubQ.Date AS Date, Notes, Patient, BirthDate, SubQ.Done AS Done, SubQ.LocID AS LocID, SubQ.Location, SubQ.ProvID AS ProvID, AssignedTo, SubQ.StateID AS StateID,  "
			"EnteredByName, SubQ.HomePhone AS HomePhone, SubQ.Email AS Email, SubQ.EnteredByID AS EnteredByID,  "
			"SubQ.Remind AS RemindDate, SubQ.UserName, SubQ.TaskID, SubQ.AssignFullName, SubQ.AssignIDs  "
			"FROM  "
			"	(SELECT NoteCatsF.Description AS TaskType, ToDoList.Task,  "
			"	CASE WHEN ToDoList.Priority = 1 THEN 'High' ELSE  "
			"	CASE WHEN Priority = 2 THEN 'Medium' ELSE  "
			"	CASE WHEN Priority = 3 THEN 'Low' END END END AS Priority,   "
			"	PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignIDs,   "
			"	ToDoList.Deadline AS Date, ToDoList.Notes,   "
			"	PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS Patient,  "
			"	PersonT1.BirthDate, "
			"	ToDoList.Done,  "
			"	PersonT1.Location AS LocID, LocationsT.Name AS Location,  "
			"	PatientsT.MainPhysician AS ProvID, dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignedTo,  "
			"	CASE WHEN ToDoList.Done Is Null THEN 1 ELSE 2 END AS StateID,  "
			"	EnteredPersonT.Last + ', ' + EnteredPersonT.First + ' ' + EnteredPersonT.Middle AS EnteredByName, PersonT1.HomePhone AS HomePhone, PersonT1.Email AS Email,  "
			"	EnteredPersonT.ID AS EnteredByID,  "
			"	ToDoList.Remind, dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS UserName, TaskID, "
			"	dbo.GetTodoAssignToFullNamesString(ToDoList.TaskID) AS AssignFullName, ToDoList.CategoryID "
			"	FROM ToDoList "
			"	LEFT JOIN PersonT PersonT1 ON ToDoList.PersonID = PersonT1.ID "
			"	LEFT JOIN PatientsT ON PersonT1.ID = PatientsT.PersonID "
			"	LEFT JOIN LocationsT ON PersonT1.Location = LocationsT.ID "
			"	LEFT JOIN NoteCatsF ON ToDoList.CategoryID = NoteCatsF.ID  "
			"	LEFT JOIN UsersT AssignByT ON ToDoList.EnteredBy = AssignByT.PersonID  "
			"	LEFT JOIN PersonT EnteredPersonT ON ToDoList.EnteredBy = EnteredPersonT.ID "
			"	WHERE PatientsT.PersonID IS NULL OR PatientsT.CurrentStatus <> 4 "
			"	) AS SubQ");
		break;


	case 481:
		//Multi Fee Preview
		/*	Version History
			DRT 2/24/2004 - PLID 10854 - Created.  Shows all cpt codes per group, even ones that haven't changed.
			// (j.jones 2009-10-23 09:01) - PLID 18558 - exposed UsePOS for proper labeling, and filtered locations accordingly
			// (j.jones 2013-04-15 13:28) - PLID 12136 - supported products in fee schedules
		*/
		switch(nSubLevel){
		case 0:
			{
				CString str;
				str.Format("SELECT Name, ID AS ID, Note "
					"FROM MultiFeeGroupsT "
					"WHERE ID = %li", nExtraID);
				return _T(str);
			}
			break;
		case 1:
			switch(nSubRepNum){
			case 0:
				return _T("SELECT MultiFeeProvidersT.FeeGroupID,  "
					"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS DocName "
					"FROM MultiFeeProvidersT INNER JOIN "
					"    PersonT ON MultiFeeProvidersT.ProviderID = PersonT.ID");
				break;
			case 1:
				return _T("SELECT MultiFeeInsuranceT.FeeGroupID,  "
					"    InsuranceCoT.Name "
					"FROM MultiFeeInsuranceT INNER JOIN "
					"    InsuranceCoT ON  "
					"    MultiFeeInsuranceT.InsuranceCoID = InsuranceCoT.PersonID");
				break;
			case 2:
				return _T("SELECT MultiFeeLocationsT.FeeGroupID, LocationsT.Name, MultiFeeGroupsT.UsePOS "
					"FROM MultiFeeLocationsT "
					"INNER JOIN MultiFeeGroupsT ON MultiFeeLocationsT.FeeGroupID = MultiFeeGroupsT.ID "
					"INNER JOIN LocationsT ON MultiFeeLocationsT.LocationID = LocationsT.ID "
					"WHERE LocationsT.Active = 1 AND TypeID = 1 AND (UsePOS = 1 OR Managed = 1)");
				break;
			case 3:
				{
					CString strRet;
					strRet.Format("SELECT %li AS FeeGroupID, "
						"CASE WHEN ProductT.ID Is Not Null THEN ProductT.InsCode ELSE CPTCodeT.Code END AS Code, "
						"CASE WHEN ProductT.ID Is Not Null THEN '' ELSE CPTCodeT.SubCode END AS SubCode, ServiceT.Name, "
						"ServiceT.Price AS OldPrice, "
						"MultiFeeItemsT.Price AS NewPrice, MultiFeeItemsT.Allowable "
						"FROM MultiFeeItemsT "
						"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
						"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
						"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
						"WHERE MultiFeeItemsT.FeeGroupID = %li "
						"AND (CPTCodeT.ID Is Not Null OR ProductT.ID Is Not Null)", nExtraID, nExtraID);
					return _T(strRet);
				}
				break;
			default:
				return _T("");
				break;
			}
			break;
		default:
			return _T("");
			break;
		}
		break;

	case 490: //Individual PracYakker Message
		/* Version History
			TES 3/10/04 - PLID 5844 - Created
		*/
		//Manually alter .ttx to make Text field a memo.
		return _T("SELECT Text, "
			"CASE WHEN MessagesT.RegardingID = -1 THEN '' ELSE PersonRegarding.Last + ', ' + PersonRegarding.First + ' ' + PersonRegarding.Middle END AS Regarding, "
			"CASE WHEN Priority = 1 THEN 'Low' WHEN Priority = 3 THEN 'Urgent' ELSE 'Medium' END AS Priority, DateSent, "
			"MessageGroupID, Sender.UserName AS Sender, Recipient.UserName AS Recipient"
			" "
			"FROM MessagesT LEFT JOIN PersonT PersonRegarding ON MessagesT.RegardingID = PersonRegarding.ID "
			"INNER JOIN UsersT Sender ON MessagesT.SenderID = Sender.PersonID "
			"INNER JOIN UsersT Recipient ON MessagesT.RecipientID = Recipient.PersonID "
			" "
			"WHERE (MessagesT.DeletedBySender = 0 OR MessagesT.DeletedByRecipient = 0)");
		break;

	case 497: //Gift Certificate (PP)
		/*	Version History
			4/5/2004 - Created.
			// (j.dinatale 2011-09-30 16:33) - PLID 45773 - take into account quantity when calculating GC balance since it cant be negative
			// (j.jones 2015-04-27 09:46) - PLID 65389 - we now pull the value calculation from a shared function,
			// which added support for the new Value field, Refunds, and GC Transfers
		*/
		return _T("SELECT GiftCertificatesT.GiftID, PurchaseDate, ExpDate, PurchT.ID AS PurchasedByID, "
			"PurchT.Last + ', ' + PurchT.First + ' ' + PurchT.Middle AS PurchasedByName, "
			"RecT.ID AS ReceivedID, RecT.Last + ', ' + RecT.First + ' ' + RecT.Middle AS ReceivedByName, "
			"ServiceT.ID AS DefaultTypeID, ServiceT.Name AS DefaultTypeName, "
			"GCBalanceQ.TotalValue AS TotalAmt, "
			"LocationsT.ID AS LocID, LocationsT.Name AS LocName, LocationsT.Address1, LocationsT.Address2, "
			"LocationsT.City, LocationsT.State, LocationsT.Zip, LocationsT.Phone, LocationsT.Phone2, "
			"LocationsT.Fax, LocationsT.OnlineAddress, LocationsT.Website, LocationsT.TollFreePhone "
			"FROM GiftCertificatesT "
			"INNER JOIN (" + GetGiftCertificateValueQuery() + ") AS GCBalanceQ ON GiftCertificatesT.ID = GCBalanceQ.ID "
			"LEFT JOIN PersonT PurchT ON GiftCertificatesT.PurchasedBy = PurchT.ID "
			"LEFT JOIN PersonT RecT ON GiftCertificatesT.ReceivedBy = RecT.ID "
			"LEFT JOIN GCTypesT ON GiftCertificatesT.DefaultTypeID = GCTypesT.ServiceID "
			"LEFT JOIN ServiceT ON GCTypesT.ServiceID = ServiceT.ID "
			"LEFT JOIN LocationsT ON GiftCertificatesT.LocationID = LocationsT.ID ");
		break;

		case 502: //revenue by referral source
			{
				// (j.jones 2012-06-19 11:15) - PLID 48481 - This always reported 0 as Cost,
				// it should be NULL on each patient line, then with the actual costs joined at the end.
				// The report should only summarize the cost, as it is meaningless on each patient entry.
				// Also, costs are not joined with referring patient or referring physician queries.
				// (d.thompson 2013-01-21) - PLID 54697 - Added Email Address
				// (r.gonet 2015-05-05 14:38) - PLID 66305 - Exclude Gift Certificate Refunds

				CString strSql;
				//figure out what we are running this by

				if (GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>") == 1) {
						strSql.Format("	SELECT ReferralID, CAST (Sum(Payments) AS MONEY) AS Payments, NULL AS Cost, Name,"
							" UserDefinedID, First, Last, Middle, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone, Email "
							" FROM (	 "
							" /*Payments Query*/	 "
								" SELECT 	ReferralSourceT.PersonID AS ReferralID,  Sum(PaymentsByReferralSourceSubQ.Amount) AS Payments,		 "
								" ReferralSourceT.Name AS Name, TransProvID, TransLocID, PatLocID, PatProvID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, "
								" UserDefinedID, First, Last, Middle, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone, Email "
								" FROM ((SELECT * FROM (SELECT LinePaymentsT.ID, Amount = CASE	 "
									" WHEN [_PartiallyAppliedPaysQ].[ID] Is Null	 "
									" THEN CASE	 "
											" WHEN [LineChargesT].[ID] Is Null	 "
											" THEN [LinePaymentsT].[Amount]	 "
											" ELSE [AppliesT].[Amount]	 "
											" End		 "
										" ELSE  [AppliesT].[Amount]	 "
										" End,	 "
									" TransProvID = CASE		 "
										" WHEN [DoctorsProviders] Is Null		 "
										" THEN PaymentsT.ProviderID	 "
										" ELSE [DoctorsProviders]		 "
										" End,	 "
									" LinePaymentsT.InputDate AS SourceIDate,		 "
									" LinePaymentsT.Date AS SourceTDate,		 "
									" PatientsT.ReferralID,	 "
									" TransLocID = CASE	 "
										" WHEN LineChargesT.ID IS NULL	 "
										" THEN LinePaymentsT.LocationID	 "
										" ELSE LineChargesT.LocationID	 "
										" END,	 "
									" LineChargesT.InputDate AS DestIDate,	 "
									" LineChargesT.Date AS DestTDate,	 "
									" BillsT.Date AS BillDate,	 "
									" PatientsT.MainPhysician AS PatProvID,	 "
									" PersonT.Location AS PatLocID, "
									" PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
									" PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, PersonT.Email	 "
									" FROM (((((((LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT	 "
									" ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN	 "
									" (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,	 "
									" Sum(AppliesT.Amount) AS ApplyAmt,	 "
									" /*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,	 "
									" LineItemT_1.PatientID	 "
									" FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID)	 "
									 " ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID	 "
									" WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))	 "
									" GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID	 "
									" HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))	 "
									" ) AS _PartiallyAppliedPaysQ ON LinePaymentsT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID)	 "
									" LEFT JOIN LineItemT AS LineChargesT ON AppliesT.DestID = LineChargesT.ID) LEFT JOIN ChargesT ON LineChargesT.ID = ChargesT.ID)	 "
									" LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID	 "
								   " LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
									" LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LinePaymentsT.PatientID = PatientsT.PersonID)	 "
									" WHERE (((PaymentsT.ID) Is Not Null) AND ((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))	 "
									" %s	 " // Filter 1 [Resp] [PatCoord] [Category] [From] [To] 
									" ) AS _PaymentsByReferralSourceFullQ		 "
									" WHERE (1=1) %s  " //Filter 2 [Prov] [Loc1]
									" UNION ALL	 "
									" SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,	 "
									" [_PartiallyAppliedPaysQ].Total AS Amount,	 "
									" PaymentsT.ProviderID AS TransProvID,	 "
									" LinePaymentsT.InputDate AS IDate,	 "
									" LinePaymentsT.Date AS TDate,	 "
									" PatientsT.ReferralID,	 "
									" LinePaymentsT.LocationID AS TransLocID,	 "
									" LinePaymentsT.InputDate AS OtherIDate,	 "
									" LinePaymentsT.Date AS OtherTDate,	 "
									" LinePaymentsT.Date AS BillDate,	 "
									" PatientsT.MainPhysician as PatProvID,	 "
									" PersonT.Location as PatLocID, "
									" PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
									" PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, PersonT.Email			 "
									" FROM ((((SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,	 "
									" Sum(AppliesT.Amount) AS ApplyAmt,	 "
									" /*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,	 "
									" LineItemT_1.PatientID	 "
									" FROM LineItemT AS LineItemT_1 LEFT JOIN		 "
									" (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID)	 "
									" ON LineItemT_1.ID = PaymentsT.ID	 "
									" WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))	 "
									" GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID	 "
									" HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))	 "
									" ) AS _PartiallyAppliedPaysQ		 "
									" INNER JOIN (LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID)	 "
									" ON [_PartiallyAppliedPaysQ].ID = LinePaymentsT.ID)	 "
									" INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)	 "
									" ON LinePaymentsT.PatientID = PatientsT.PersonID)	 "
									" WHERE (((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))		 "
									" %s   "  //Filter3 [Resp] [PatCoord] [Category2] [From] [To] 
									" ) AS _PaymentsByReferralSourcePartQ		 "
  									" WHERE (1=1)  %s " //Filter 2 [Prov] [Loc1]  
									" ) AS PaymentsByReferralSourceSubQ	 "
									" LEFT JOIN ReferralSourceT ON PaymentsByReferralSourceSubQ.ReferralID = ReferralSourceT.PersonID)	 "
									" WHERE ReferralSourceT.PersonID IS NOT NULL	 "
									" GROUP BY ReferralSourceT.PersonID, ReferralSourceT.Name, TransProvID, TransLocID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, PatProvID, PatLocID, "
									" UserdefinedID, First,  Last, Middle, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, Cellphone, Email "
							" UNION	 "
							" /*Refund Query*/	 "
								" SELECT 	ReferralSourceT.PersonID AS ReferralID,  Sum(PaymentsByReferralSourceSubQ.Amount) AS Payments,		 "
								" ReferralSourceT.Name AS Name, TransProvID, TransLocID, PatProvID, PatLocID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate,  "
								" UserdefinedID, First,  Last, Middle, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, Cellphone, Email "
									" FROM ((SELECT * FROM (SELECT LinePaymentsT.ID, Amount = CASE	 "
										" WHEN [_PartiallyAppliedRefsQ].[ID] Is Null	 "
										" THEN CASE	 "
											" WHEN [LinePaymentsT].[ID] Is Null	 "
											" THEN [LineRefundsT].[Amount]	 "
											" ELSE [AppliesT].[Amount]	 "
											" End		 "
										" ELSE  [AppliesT].[Amount]	 "
										" End,	 "
									" TransProvID = CASE		 "
										" WHEN [RefundsT].[ProviderID] Is Null	 "
										" THEN [PaymentsT].[ProviderID]	 "
										" ELSE [RefundsT].[ProviderID]	 "
										" End,	 "
									" LineRefundsT.InputDate AS SourceIDate,		 "
									" LineRefundsT.Date AS SourceTDate,		 "
									" PatientsT.ReferralID,	 "
									" TransLocID = CASE	 "
										" WHEN [LinePaymentsT].[ID] Is Null	 "
										" THEN [LineRefundsT].[LocationID]	 "
										" ELSE [LinePaymentsT].[LocationID]	 "
										" End,	 "
									" LinePaymentsT.InputDate AS DestIDate,	 "
									" LinePaymentsT.Date AS DestTDate,	 "
									" LineRefundsT.Date AS BillDate,	 "
									" PatientsT.MainPhysician AS PatProvID,	 "
									" PersonT.Location AS PatLocID, "
									" PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
									" PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, PersonT.Email		 "
									" FROM (((((((LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT RefundsT	 "
									" ON LineRefundsT.ID = RefundsT.ID) LEFT JOIN	 "
									" (SELECT LineRefs.ID, LineRefs.Amount AS RefAmt,		 "
									" Sum(AppliesT.Amount) AS ApplyAmt,	 "
									" /*First a Amount*/Min([LineRefs].[Amount])-Sum([AppliesT].[Amount]) AS Total,	 "
									" LineRefs.PatientID		 "
									" FROM LineItemT AS LineRefs LEFT JOIN (PaymentsT RefundsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePays ON AppliesT.DestID = LinePays.ID)	 "
									 " ON RefundsT.ID = AppliesT.SourceID) ON LineRefs.ID = RefundsT.ID	 "
									" WHERE (((LineRefs.Deleted)=0)) AND (RefundsT.PayMethod NOT IN (4,10)) AND LineRefs.Type = 3	 "
									" GROUP BY LineRefs.ID, LineRefs.Amount, LineRefs.PatientID	 "
									" HAVING (((LineRefs.ID) is not  Null) AND ((MIN([LineRefs].[Amount])-Sum([AppliesT].[Amount])) <> 0))	 "
									" ) AS _PartiallyAppliedRefsQ ON LineRefundsT.ID = [_PartiallyAppliedRefsQ].ID) LEFT JOIN AppliesT ON RefundsT.ID = AppliesT.SourceID)	 "
									" LEFT JOIN LineItemT AS LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID) LEFT JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID)		 "
									" LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineRefundsT.PatientID = PatientsT.PersonID)	 "
									" WHERE (((RefundsT.ID) Is Not Null) AND ((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (RefundsT.PayMethod NOT IN (4,10))		 "
									" %s   "  //Filter 4 [Resp2] [PatCoord] [Category2] [From2] [To2]
									" ) AS _PaymentsByReferralSourceFullQ		 "
     									" WHERE (1=1)  %s  "   //Filter 2 [Prov] [Loc1] 
									" UNION ALL	 "
									" SELECT * FROM (SELECT [_PartiallyAppliedRefsQ].ID,	 "
									" [_PartiallyAppliedRefsQ].Total AS Amount,	 "
									" RefundsT.ProviderID AS TransProvID,		 "
									" LineRefundsT.InputDate AS IDate,	 "
									" LineRefundsT.Date AS TDate,	 "
									" PatientsT.ReferralID,	 "
									" LineRefundsT.LocationID as TransLocID,	 "
									" LineRefundsT.InputDate as OtherIDate,	 "
									" LineRefundsT.Date as OtherTDate,	 "
									" LineRefundsT.Date as BillDate,	 "
									" PatientsT.MainPhysician as PatProvID,	 "
									" PersonT.Location as PatLocID, "
									" PatientsT.UserDefinedID, PersonT.First,  PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
									" PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, PersonT.Email		 "
									" FROM ((((SELECT LineRefs.ID, LineRefs.Amount AS PayAmt,		 "
									" Sum(AppliesT.Amount) AS ApplyAmt,	 "
									" /*First a Amount*/Min([LineRefs].[Amount])-Sum([AppliesT].[Amount]) AS Total,	 "
									" LineRefs.PatientID		 "
									" FROM LineItemT AS LineRefs LEFT JOIN	 "
									" (PaymentsT RefundsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePays ON AppliesT.DestID = LinePays.ID) ON RefundsT.ID = AppliesT.SourceID)	 "
									" ON LineRefs.ID = RefundsT.ID	 "
									" WHERE (((LineRefs.Deleted)=0)) AND (RefundsT.PayMethod NOT IN (4,10)) AND (LineRefs.Type = 3)	 "
									" GROUP BY LineRefs.ID, LineRefs.Amount, LineRefs.PatientID	 "
									" HAVING (((LineRefs.ID) is not  Null) AND ((MIN([LineRefs].[Amount])-Sum([AppliesT].[Amount])) <> 0))	 "
									" ) AS _PartiallyAppliedRefsQ		 "
									" INNER JOIN (LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID)	 "
									" ON [_PartiallyAppliedRefsQ].ID = LineRefundsT.ID)	 "
									" INNER JOIN PaymentsT RefundsT ON LineRefundsT.ID = RefundsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)	 "
									" ON LineRefundsT.PatientID = PatientsT.PersonID)	 "
									" WHERE (((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (RefundsT.PayMethod NOT IN (4,10))	 "
											" %s  "  //Filter 4 [Resp2] [PatCoord] [Category2] [From2] [To2]
									" ) AS _PaymentsByReferralSourcePartQ		 "
    									" WHERE (1=1)  %s    "  //Filter 2 [Prov] [Loc1]
									" ) AS PaymentsByReferralSourceSubQ	 "
									" LEFT JOIN ReferralSourceT ON PaymentsByReferralSourceSubQ.ReferralID = ReferralSourceT.PersonID)	 "
									" WHERE ReferralSourceT.PersonID IS NOT NULL	 "
									" GROUP BY ReferralSourceT.PersonID, ReferralSourceT.Name, TransProvID, TransLocID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, PAtProvID, PatLocID, "
									" UserdefinedID, First,  Last, Middle, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, Cellphone, Email	 "
							" ) RevenueQ	 "
							" GROUP BY RevenueQ.ReferralID, RevenueQ.Name, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone, Email " 
							
							// (j.jones 2012-06-19 10:06) - PLID 48481 - unioned the marketing costs
							" UNION ALL "

							"SELECT ReferralSource AS ReferralID, NULL AS Payments, "
							"CAST (SUM (CostSubQ.Cost) AS MONEY) AS Cost, Name, "
							"NULL AS UserDefinedID, NULL AS First, NULL AS Last, NULL AS Middle, NULL AS Address1, NULL AS Address2, NULL AS City, NULL AS State, NULL AS Zip, NULL AS HomePhone, NULL AS WorkPhone, NULL AS CellPhone, NULL AS Email "
							"FROM "
							"("
							"SELECT Cost1+Cost2+Cost3+Cost4 AS Cost, ReferralSource, Name "
							"FROM ( "
							"  SELECT "
							"    (CASE WHEN (EffectiveFrom >= '%s' AND EffectiveFrom < '%s' AND EffectiveTo >= '%s' AND EffectiveTo < '%s') "
							"	  THEN (Amount) ELSE 0 END) AS Cost1, "
							"    (CASE WHEN (EffectiveFrom < '%s' AND EffectiveFrom > '%s' AND EffectiveTo >= '%s') "
							"	  THEN (Amount * DATEDIFF(day, '%s', EffectiveFrom)) / DATEDIFF(day, EffectiveTo, EffectiveFrom) ELSE 0 END) AS Cost2, "
							"    (CASE WHEN (EffectiveFrom <= '%s' AND EffectiveTo >= '%s' AND (EffectiveFrom <> '%s' OR EffectiveTo <> '%s')) "
							"	  THEN (Amount * DATEDIFF(day, '%s', '%s')) / DATEDIFF(day, EffectiveTo, EffectiveFrom) ELSE 0 END) AS Cost3, "
							"    (CASE WHEN (EffectiveFrom < '%s' AND EffectiveTo > '%s' AND EffectiveTo < '%s') "
							"	  THEN (Amount * DATEDIFF(day, EffectiveTo, '%s')) / DATEDIFF (day, EffectiveTo, EffectiveFrom) ELSE 0 END) AS Cost4, "
							"    ReferralSource, Name "
							"  FROM MarketingCostsT "
							"  LEFT JOIN ReferralSourceT ON MarketingCostsT.ReferralSource = ReferralSourceT.PersonID "
							"  WHERE (1=1) %s " //[referral2] [Loc2]
							") RangedCostsQ "

							") AS CostSubQ "
							"GROUP BY ReferralSource, Name",
							saExtraValues.GetSize()?saExtraValues.GetSize()?saExtraValues[0]:"":""/*Passed in by marketview.cpp*/, strExtraText, strFilterField, strExtraText, strExtraField, strExtraText, strExtraField, strExtraText,
							FormatDateTimeForSql(DateFrom), FormatDateTimeForSql(DateTo), FormatDateTimeForSql(DateFrom), FormatDateTimeForSql(DateTo),
							FormatDateTimeForSql(DateTo), FormatDateTimeForSql(DateFrom), FormatDateTimeForSql(DateTo), FormatDateTimeForSql(DateTo),
							FormatDateTimeForSql(DateFrom), FormatDateTimeForSql(DateTo), FormatDateTimeForSql(DateFrom), FormatDateTimeForSql(DateTo),
							FormatDateTimeForSql(DateTo), FormatDateTimeForSql(DateFrom),
							FormatDateTimeForSql(DateFrom), FormatDateTimeForSql(DateFrom), FormatDateTimeForSql(DateTo),
							FormatDateTimeForSql(DateFrom),
							strExtendedSql);
					}
					else if (GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>") == 2) {
						strSql.Format("SELECT RefPhysID as ReferralID, CAST (Sum(Payments) AS MONEY) AS Payments, NULL AS Cost, Name,  "
							" UserDefinedID, First, Last, Middle, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone, Email "
							" FROM (	  "
							 " /*Payments Query*/	  "
				 					" SELECT 	ReferringPhysT.PersonID AS RefPhysID,  Sum(PaymentsByReferralSourceSubQ.Amount) AS Payments,		  "
								 " PersonReferringPhysT.Last + ', ' + PersonReferringPhysT.First  + ' ' + PersonReferringPhysT.Middle AS Name, TransProvID, TransLocID, PatLocID, PatProvID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, "
								 " PaymentsByReferralSourceSubQ.UserDefinedID, PaymentsByReferralSourceSubQ.First, PaymentsByReferralSourceSubQ.Last, PaymentsByReferralSourceSubQ.Middle, PaymentsByReferralSourceSubQ.Address1,  "
								 " PaymentsByReferralSourceSubQ.Address2, PaymentsByReferralSourceSubQ.City, PaymentsByReferralSourceSubQ.State, PaymentsByReferralSourceSubQ.Zip,  "
								 " PaymentsByReferralSourceSubQ.HomePhone, PaymentsByReferralSourceSubQ.WorkPhone, PaymentsByReferralSourceSubQ.CellPhone, PaymentsByReferralSourceSubQ.Email "
								 " FROM ((SELECT * FROM (SELECT LinePaymentsT.ID, Amount = CASE	    "
									 " WHEN [_PartiallyAppliedPaysQ].[ID] Is Null	    "
									 " THEN CASE	    "
											 " WHEN [LineChargesT].[ID] Is Null	    "
											 " THEN [LinePaymentsT].[Amount]	    "
											 " ELSE [AppliesT].[Amount]	    "
											 " End		    "
										 " ELSE  [AppliesT].[Amount]	    "
										 " End,	    "
									 " TransProvID = CASE		    "
										 " WHEN [DoctorsProviders] Is Null		    "
										 " THEN PaymentsT.ProviderID	    "
										 " ELSE [DoctorsProviders]		    "
										 " End,	    "
									 " LinePaymentsT.InputDate AS SourceIDate,		    "
									 " LinePaymentsT.Date AS SourceTDate,		  "
									 " PatientsT.DefaultReferringPhyID,	  "
						  				" TransLocID = CASE	  "
						 					" WHEN LineChargesT.ID IS NULL	  "
						 					" THEN LinePaymentsT.LocationID	  "
						 					" ELSE LineChargesT.LocationID	  "
						 					" END,	  "
					 					" LineChargesT.InputDate AS DestIDate,	  "
					 					" LineChargesT.Date AS DestTDate,	  "
					 					" BillsT.Date AS BillDate,	  "
					 					" PatientsT.MainPhysician AS PatProvID,	  "
					 					" PersonT.Location AS PatLocID, "
										" PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City,  "
										" PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.Workphone, PersonT.CellPhone, PersonT.Email "
					 					" FROM (((((((LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT	  "
					 					" ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN	  "
					 					" (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,	  "
					 					" Sum(AppliesT.Amount) AS ApplyAmt,	  "
					 					" /*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,	  "
					 					" LineItemT_1.PatientID	  "
					 					" FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID)	  "
						 				 " ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID	  "
					 					" WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))	  "
					 					" GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID	  "
					 					" HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))	  "
					 					" ) AS _PartiallyAppliedPaysQ ON LinePaymentsT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID)	  "
					 					" LEFT JOIN LineItemT AS LineChargesT ON AppliesT.DestID = LineChargesT.ID) LEFT JOIN ChargesT ON LineChargesT.ID = ChargesT.ID)	  "
					 					" LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID	  "
										   " LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
					 					" LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LinePaymentsT.PatientID = PatientsT.PersonID)	  "
					 					" WHERE (((PaymentsT.ID) Is Not Null) AND ((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))	  "
										 " %s" /* Filter1 [Resp] [PatCoord] [Category] [From] [To] [RefPhys]*/ 
					 					" ) AS _PaymentsByReferralSourceFullQ		  "
									   " WHERE (1=1) %s " /*Filter2 [Prov] [Loc1]   */ 
					 					" UNION ALL	  "
					 					" SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,	  "
					 					" [_PartiallyAppliedPaysQ].Total AS Amount,	  "
					 					" PaymentsT.ProviderID AS TransProvID,	  "
					 					" LinePaymentsT.InputDate AS IDate,	  "
					 					" LinePaymentsT.Date AS TDate,	  "
					 					" PatientsT.DefaultReferringPhyID,  "
					 					" LinePaymentsT.LocationID as TransLocID,	  "
					 					" LinePaymentsT.InputDate AS OtherIDate,	  "
					 					" LinePaymentsT.Date AS OtherTDate,	  "
					 					" LinePaymentsT.Date AS BillDate,	  "
					 					" PatientsT.MainPhysician as PatProvID,	  "
					 					" PersonT.Location as PatLocID, "
										" PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City,  "
										" PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.Workphone, PersonT.CellPhone, PersonT.Email "
					 					" FROM ((((SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,	  "
					 					" Sum(AppliesT.Amount) AS ApplyAmt,	  "
					 					" /*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,	  "
					 					" LineItemT_1.PatientID	  "
					 					" FROM LineItemT AS LineItemT_1 LEFT JOIN		  "
					 					" (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID)	  "
					 					" ON LineItemT_1.ID = PaymentsT.ID	  "
					 					" WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))	  "
					 					" GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID	  "
					 					" HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))	  "
					 					" ) AS _PartiallyAppliedPaysQ		  "
					 					" INNER JOIN (LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID)	  "
					 					" ON [_PartiallyAppliedPaysQ].ID = LinePaymentsT.ID)	  "
					 					" INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)	  "
					 					" ON LinePaymentsT.PatientID = PatientsT.PersonID)	  "
					 					" WHERE (((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))		  "
									    " %s " /*Filter 3 [Resp] [PatCoord] [Category2] [From] [To]  [RefPhys]*/  
					 					" ) AS _PaymentsByReferralSourcePartQ		  "
									   " WHERE (1=1)  %s " /* Filter 2 [Prov] [Loc1] */  
					 					" ) AS PaymentsByReferralSourceSubQ	  "
					 					" LEFT JOIN ReferringPhysT ON PaymentsByReferralSourceSubQ.DefaultReferringPhyID = ReferringPhysT.PersonID  "
					 					" LEFT JOIN PersonT PersonReferringPhysT ON ReferringPhysT.PersonID = PersonReferringPhysT.ID)	  "
					 					" WHERE ReferringPhysT.PersonID IS NOT NULL	  "
					 					" GROUP BY ReferringPhysT.PersonID, PersonReferringPhysT.First, PersonReferringPhysT.Last, PersonReferringPhysT.Middle, TransProvID, TransLocID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, PatProvID, PatLocID, "
										" PaymentsByReferralSourceSubQ.UserDefinedID, PaymentsByReferralSourceSubQ.First, PaymentsByReferralSourceSubQ.Last, PaymentsByReferralSourceSubQ.Middle, PaymentsByReferralSourceSubQ.Address1, PaymentsByReferralSourceSubQ.Address2, PaymentsByReferralSourceSubQ.City,  "
										" PaymentsByReferralSourceSubQ.State, PaymentsByReferralSourceSubQ.Zip, PaymentsByReferralSourceSubQ.HomePhone, PaymentsByReferralSourceSubQ.Workphone, PaymentsByReferralSourceSubQ.CellPhone, PaymentsByReferralSourceSubQ.Email	  "
			 					" UNION	  "
			 					" /*Refund Query*/	  "
				 					" SELECT 	ReferringPhysT.PersonID AS RefPhysID,  Sum(PaymentsByReferralSourceSubQ.Amount) AS Payments,		  "
				 					" PersonReferringPhysT.Last + ', ' + PersonReferringPhysT.First  + ' ' + PersonReferringPhysT.Middle AS Name, TransProvID, TransLocID, PatLocID, PatProvID, DestIDate,  DestTDate, SourceIDate, SourceTDate, BillDate, "
									" PaymentsByReferralSourceSubQ.UserDefinedID, PaymentsByReferralSourceSubQ.First, PaymentsByReferralSourceSubQ.Last, PaymentsByReferralSourceSubQ.Middle, PaymentsByReferralSourceSubQ.Address1,  "
									" PaymentsByReferralSourceSubQ.Address2, PaymentsByReferralSourceSubQ.City, PaymentsByReferralSourceSubQ.State, PaymentsByReferralSourceSubQ.Zip,  "
									" PaymentsByReferralSourceSubQ.HomePhone, PaymentsByReferralSourceSubQ.WorkPhone, PaymentsByReferralSourceSubQ.CellPhone, PaymentsByReferralSourceSubQ.Email "
					 					" FROM ((SELECT * FROM (SELECT LinePaymentsT.ID, Amount = CASE	  "
						 					" WHEN [_PartiallyAppliedRefsQ].[ID] Is Null	  "
						 					" THEN CASE	  "
							 					" WHEN [LinePaymentsT].[ID] Is Null	  "
							 					" THEN [LineRefundsT].[Amount]	  "
							 					" ELSE [AppliesT].[Amount]	  "
							 					" End		  "
						 					" ELSE  [AppliesT].[Amount]	  "
						 					" End,	  "
					 					" TransProvID = CASE		  "
						 					" WHEN [RefundsT].[ProviderID] Is Null	  "
						 					" THEN [PaymentsT].[ProviderID]	  "
						 					" ELSE [RefundsT].[ProviderID]	  "
						 					" End,	  "
					 					" LineRefundsT.InputDate AS SourceIDate,		  "
					 					" LineRefundsT.Date AS SourceTDate,		  "
					 					" PatientsT.DefaultReferringPhyID,	  "
					 					" TransLocID = CASE	  "
						 					" WHEN [LinePaymentsT].[LocationID] Is Null	  "
						 					" THEN [LineRefundsT].[LocationID]	  "
						 					" ELSE [LinePaymentsT].[LocationID]	  "
						 					" End,	  "
					 					" LinePaymentsT.InputDate AS DestIDate,	  "
					 					" LinePaymentsT.Date AS DestTDate,	  "
					 					" LineRefundsT.Date AS BillDate,	  "
					 					" PatientsT.MainPhysician AS PatProvID,	  "
					 					" PersonT.Location AS PatLocID, "
										" PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City,  "
										" PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.Workphone, PersonT.CellPhone, PersonT.Email	  "
					 					" FROM (((((((LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT RefundsT	  "
					 					" ON LineRefundsT.ID = RefundsT.ID) LEFT JOIN	  "
					 					" (SELECT LineRefs.ID, LineRefs.Amount AS RefAmt,		  "
					 					" Sum(AppliesT.Amount) AS ApplyAmt,	  "
					 					" /*First a Amount*/Min([LineRefs].[Amount])-Sum([AppliesT].[Amount]) AS Total,	  "
					 					" LineRefs.PatientID		  "
					 					" FROM LineItemT AS LineRefs LEFT JOIN (PaymentsT RefundsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePays ON AppliesT.DestID = LinePays.ID)	  "
						 				 " ON RefundsT.ID = AppliesT.SourceID) ON LineRefs.ID = RefundsT.ID	  "
					 					" WHERE (((LineRefs.Deleted)=0)) AND (RefundsT.PayMethod NOT IN (4,10)) AND LineRefs.Type = 3	  "
					 					" GROUP BY LineRefs.ID, LineRefs.Amount, LineRefs.PatientID	  "
					 					" HAVING (((LineRefs.ID) is not  Null) AND ((MIN([LineRefs].[Amount])-Sum([AppliesT].[Amount])) <> 0))	  "
					 					" ) AS _PartiallyAppliedRefsQ ON LineRefundsT.ID = [_PartiallyAppliedRefsQ].ID) LEFT JOIN AppliesT ON RefundsT.ID = AppliesT.SourceID)	  "
					 					" LEFT JOIN LineItemT AS LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID) LEFT JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID)		  "
					 					" LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineRefundsT.PatientID = PatientsT.PersonID)	  "
					 					" WHERE (((RefundsT.ID) Is Not Null) AND ((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (PaymentsT.PayMethod NOT IN (4,10))		  "
									   "  %s " /* Filter 4 - [Resp2] [PatCoord] [Category2] [From2] [To2] [RefPhys]  */ 
					 					" ) AS _PaymentsByReferralSourceFullQ		  "
									   " WHERE (1=1)  %s " /*Filter 2 - [Prov] [Loc1]  */ 
					 					" UNION ALL	  "
					 					" SELECT * FROM (SELECT [_PartiallyAppliedRefsQ].ID,	  "
					 					" [_PartiallyAppliedRefsQ].Total AS Amount,	  "
					 					" RefundsT.ProviderID AS TransProvID,		  "
					 					" LineRefundsT.InputDate AS IDate,	  "
					 					" LineRefundsT.Date AS TDate,	  "
					 					" PatientsT.DefaultReferringPhyID,	  "
					 					" LineRefundsT.LocationID as TransLocID,	  "
					 					" LineRefundsT.InputDate as OtherIDate,	  "
					 					" LineRefundsT.Date as OtherTDate,	  "
					 					" LineRefundsT.Date as BillDate,	  "
					 					" PatientsT.MainPhysician as PatProvID,	  "
					 					" PersonT.Location as PatLocID, "
										" PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City,  "
										" PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.Workphone, PersonT.CellPhone, PersonT.Email	  "
					 					" FROM ((((SELECT LineRefs.ID, LineRefs.Amount AS PayAmt,		  "
					 					" Sum(AppliesT.Amount) AS ApplyAmt,	  "
					 					" /*First a Amount*/Min([LineRefs].[Amount])-Sum([AppliesT].[Amount]) AS Total,	  "
					 					" LineRefs.PatientID		  "
					 					" FROM LineItemT AS LineRefs LEFT JOIN	  "
					 					" (PaymentsT RefundsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePays ON AppliesT.DestID = LinePays.ID) ON RefundsT.ID = AppliesT.SourceID)	  "
					 					" ON LineRefs.ID = RefundsT.ID	  "
					 					" WHERE (((LineRefs.Deleted)=0)) AND (RefundsT.PayMethod NOT IN (4,10)) AND (LineRefs.Type = 3)	  "
					 					" GROUP BY LineRefs.ID, LineRefs.Amount, LineRefs.PatientID	  "
					 					" HAVING (((LineRefs.ID) is not  Null) AND ((MIN([LineRefs].[Amount])-Sum([AppliesT].[Amount])) <> 0))	  "
					 					" ) AS _PartiallyAppliedRefsQ		  "
					 					" INNER JOIN (LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID)	  "
					 					" ON [_PartiallyAppliedRefsQ].ID = LineRefundsT.ID)	  "
					 					" INNER JOIN PaymentsT RefundsT ON LineRefundsT.ID = RefundsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)	  "
					 					" ON LineRefundsT.PatientID = PatientsT.PersonID)	  "
					 					" WHERE (((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (RefundsT.PayMethod NOT IN (4,10))	  "
									   "   %s " /*Filter 4 [Resp2] [PatCoord] [Category2] [From2] [To2] [RefPhys] */ 
					 					" ) AS _PaymentsByReferralSourcePartQ		  "
									   " WHERE (1=1)  %s "/* Filter 2 [Prov] [Loc1]   */ 
					 					" ) AS PaymentsByReferralSourceSubQ	  "
					 					" LEFT JOIN ReferringPhysT ON PaymentsByReferralSourceSubQ.DefaultReferringPhyID = ReferringPhysT.PersonID  "
					    				" LEFT JOIN PersonT PersonReferringPhysT ON ReferringPhysT.PersonID = PersonReferringPhysT.ID)	  "
					 					" WHERE ReferringPhysT.PersonID IS NOT NULL	  "
					 					" GROUP BY ReferringPhysT.PersonID, PersonREferringPhysT.First, PersonREferringPhysT.Last,PersonREferringPhysT.Middle, TransProvID, TransLocID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, PAtProvID, PatLocID, "
										" PaymentsByReferralSourceSubQ.UserDefinedID, PaymentsByReferralSourceSubQ.First, PaymentsByReferralSourceSubQ.Last, PaymentsByReferralSourceSubQ.Middle, PaymentsByReferralSourceSubQ.Address1,  "
															" PaymentsByReferralSourceSubQ.Address2, PaymentsByReferralSourceSubQ.City, PaymentsByReferralSourceSubQ.State, PaymentsByReferralSourceSubQ.Zip,  "
															" PaymentsByReferralSourceSubQ.HomePhone, PaymentsByReferralSourceSubQ.WorkPhone, PaymentsByReferralSourceSubQ.CellPhone, PaymentsByReferralSourceSubQ.Email	  "
			 					" ) RevenueQ	  "
			 					" GROUP BY RevenueQ.RefPhysID, RevenueQ.Name, UserDefinedID, First, Last, Middle, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, Cellphone, Email ",
							saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, strExtraText, strFilterField, strExtraText, strExtraField, strExtraText, strExtraField, strExtraText);
					}
					else {
						// (r.gonet 2015-05-05 14:38) - PLID 66305 - Exclude Gift Certificate Refunds
						strSql.Format("SELECT RefPatID as ReferralID, CAST (Sum(Payments) AS MONEY) AS Payments, NULL AS Cost, Name, "
										" UserDefinedID, First, Last, Middle, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone, Email FROM (	 "
										 " /*Payments Query*/	  "
												" SELECT 	PersonReferringPatT.ID AS RefPatID,  Sum(PaymentsByReferralSourceSubQ.Amount) AS Payments,		  "
												" PersonReferringPatT.Last + ', ' + PersonReferringPatT.First + ' ' + PersonReferringPatT.Middle AS Name,  "
												" TransProvID, TransLocID, PatLocID, PatProvID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, "
												" PaymentsByReferralSourceSubQ.UserDefinedID, PaymentsByReferralSourceSubQ.First, PaymentsByReferralSourceSubQ.Last, "
												" PaymentsByReferralSourceSubQ.Middle, PaymentsByReferralSourceSubQ.Address1, PaymentsByReferralSourceSubQ.Address2, "
												" PaymentsByReferralSourceSubQ.City, PaymentsByReferralSourceSubQ.State, PaymentsByReferralSourceSubQ.Zip, "
												" PaymentsByReferralSourceSubQ.HomePhone, PaymentsByReferralSourceSubQ.WorkPhone, PaymentsByReferralSourceSubQ.CellPhone, PaymentsByReferralSourceSubQ.Email "
												" FROM ((SELECT * FROM (SELECT LinePaymentsT.ID, Amount = CASE	  "
 												" WHEN [_PartiallyAppliedPaysQ].[ID] Is Null	  "
 												" THEN CASE	  "
		 												" WHEN [LineChargesT].[ID] Is Null	  "
		 												" THEN [LinePaymentsT].[Amount]	  "
		 												" ELSE [AppliesT].[Amount]	  "
		 												" End		  "
	 												" ELSE  [AppliesT].[Amount]	  "
	 												" End,	  "
 												" TransProvID = CASE		  "
	 												" WHEN [DoctorsProviders] Is Null		  "
	 												" THEN PaymentsT.ProviderID	  "
	 												" ELSE [DoctorsProviders]		  "
	 												" End,	  "
 												" LinePaymentsT.InputDate AS SourceIDate,		  "
 												" LinePaymentsT.Date AS SourceTDate,		  "
 												" PatientsT.ReferringPatientID,	  "
 												" TransLocID = CASE	  "
	 												" WHEN LineChargesT.ID IS NULL	  "
	 												" THEN LinePaymentsT.LocationID	  "
	 												" ELSE LineChargesT.LocationID	  "
	 												" END,	  "
 												" LineChargesT.InputDate AS DestIDate,	  "
 												" LineChargesT.Date AS DestTDate,	  "
 												" BillsT.Date AS BillDate,	  "
 												" PatientsT.MainPhysician AS PatProvID,	  "
 												" PersonT.Location AS PatLocID, "
												" PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.STate, "
												" PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, PersonT.Email	  "
 												" FROM (((((((LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT	  "
 												" ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN	  "
 												" (SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,	  "
 												" Sum(AppliesT.Amount) AS ApplyAmt,	  "
 												" /*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,	  "
 												" LineItemT_1.PatientID	  "
 												" FROM LineItemT AS LineItemT_1 LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID)	  "
	 											 " ON PaymentsT.ID = AppliesT.SourceID) ON LineItemT_1.ID = PaymentsT.ID	  "
 												" WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))	  "
 												" GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID	  "
 												" HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))	  "
 												" ) AS _PartiallyAppliedPaysQ ON LinePaymentsT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID)	  "
 												" LEFT JOIN LineItemT AS LineChargesT ON AppliesT.DestID = LineChargesT.ID) LEFT JOIN ChargesT ON LineChargesT.ID = ChargesT.ID)	  "
 												" LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID	  "
											   " LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
 												" LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LinePaymentsT.PatientID = PatientsT.PersonID)	  "
 												" WHERE (((PaymentsT.ID) Is Not Null) AND ((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))	  "
												  " %s " /* Filter 1 - [Resp] [PatCoord] [Category] [From] [To] [RefPat] */ 
 												" ) AS _PaymentsByReferralSourceFullQ		  "
											   " WHERE (1=1) %s " /*Filter 2 - [Prov] [Loc1]   */ 
 												" UNION ALL	  "
 												" SELECT * FROM (SELECT [_PartiallyAppliedPaysQ].ID,	  "
 												" [_PartiallyAppliedPaysQ].Total AS Amount,	  "
 												" PaymentsT.ProviderID AS TransProvID,	  "
 												" LinePaymentsT.InputDate AS IDate,	  "
 												" LinePaymentsT.Date AS TDate,	  "
 												" PatientsT.ReferringPatientID,  "
 												" LinePaymentsT.LocationID as TransLocID,	  "
 												" LinePaymentsT.InputDate AS OtherIDate,	  "
 												" LinePaymentsT.Date AS OtherTDate,	  "
 												" LinePaymentsT.Date AS BillDate,	  "
 												" PatientsT.MainPhysician as PatProvID,	  "
 												" PersonT.Location as PatLocID, "
												" PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, "
												" PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, "
												" PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, PersonT.Email	  "
 												" FROM ((((SELECT LineItemT_1.ID, LineItemT_1.Amount AS PayAmt,	  "
 												" Sum(AppliesT.Amount) AS ApplyAmt,	  "
 												" /*First a Amount*/Min([LineItemT_1].[Amount])-Sum([AppliesT].[Amount]) AS Total,	  "
 												" LineItemT_1.PatientID	  "
 												" FROM LineItemT AS LineItemT_1 LEFT JOIN		  "
 												" (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT ON AppliesT.DestID = LineItemT.ID) ON PaymentsT.ID = AppliesT.SourceID)	  "
 												" ON LineItemT_1.ID = PaymentsT.ID	  "
 												" WHERE (((LineItemT_1.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))	  "
 												" GROUP BY LineItemT_1.ID, LineItemT_1.Amount, LineItemT_1.PatientID	  "
 												" HAVING (((LineItemT_1.ID) is not  Null) AND ((MIN([LineItemT_1].[Amount])-Sum([AppliesT].[Amount])) <> 0))	  "
 												" ) AS _PartiallyAppliedPaysQ		  "
 												" INNER JOIN (LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID)	  "
 												" ON [_PartiallyAppliedPaysQ].ID = LinePaymentsT.ID)	  "
 												" INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)	  "
 												" ON LinePaymentsT.PatientID = PatientsT.PersonID)	  "
 												" WHERE (((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))		  "
												" %s " /*Filter 3 - [Resp] [PatCoord] [Category2] [From] [To] [RefPat]  */ 
 												" ) AS _PaymentsByReferralSourcePartQ		  "
											   " WHERE (1=1) %s " /* Filter 2 - [Prov] [Loc1]  */ 
"  "
 												" ) AS PaymentsByReferralSourceSubQ	  "
 												" LEFT JOIN PersonT PersonReferringPatT ON PaymentsByReferralSourceSubQ.ReferringPatientID = PersonReferringPatT.ID)	  "
 												" WHERE PersonReferringPatT.ID IS NOT NULL	  "
 												" GROUP BY PersonReferringPatT.ID, PersonReferringPatT.First, PersonReferringPatT.Last, PersonReferringPatT.Middle, TransProvID, TransLocID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, PatProvID, PatLocID, "
												" PaymentsByReferralSourceSubQ.UserDefinedID, PaymentsByReferralSourceSubQ.First, PaymentsByReferralSourceSubQ.Last, "
												" PaymentsByReferralSourceSubQ.Middle, PaymentsByReferralSourceSubQ.Address1, PaymentsByReferralSourceSubQ.Address2, "
												" PaymentsByReferralSourceSubQ.City, PaymentsByReferralSourceSubQ.State, PaymentsByReferralSourceSubQ.Zip, "
												" PaymentsByReferralSourceSubQ.HomePhone, PaymentsByReferralSourceSubQ.WorkPhone, PaymentsByReferralSourceSubQ.CellPhone, PaymentsByReferralSourceSubQ.Email	  "
											" UNION	  "
											" /*Refund Query*/	  "
												" SELECT 	PersonReferringPatT.ID AS RefPatID,  Sum(PaymentsByReferralSourceSubQ.Amount) AS Payments,		  "
												" PersonReferringPatT.Last + ', ' + PersonReferringPatT.First + ' ' + PersonReferringPatT.Middle AS Name,  "
												" TransProvID, TransLocID, PatLocID, PatProvID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, "
												" PaymentsByReferralSourceSubQ.UserDefinedID, PaymentsByReferralSourceSubQ.First, PaymentsByReferralSourceSubQ.Last, "
												" PaymentsByReferralSourceSubQ.Middle, PaymentsByReferralSourceSubQ.Address1, PaymentsByReferralSourceSubQ.Address2, "
												" PaymentsByReferralSourceSubQ.City, PaymentsByReferralSourceSubQ.State, PaymentsByReferralSourceSubQ.Zip, "
												" PaymentsByReferralSourceSubQ.HomePhone, PaymentsByReferralSourceSubQ.WorkPhone, PaymentsByReferralSourceSubQ.CellPhone, PaymentsByReferralSourceSubQ.Email  "
 												" FROM ((SELECT * FROM (SELECT LinePaymentsT.ID, Amount = CASE	  "
	 												" WHEN [_PartiallyAppliedRefsQ].[ID] Is Null	  "
	 												" THEN CASE	  "
		 												" WHEN [LinePaymentsT].[ID] Is Null	  "
		 												" THEN [LineRefundsT].[Amount]	  "
		 												" ELSE [AppliesT].[Amount]	  "
		 												" End		  "
	 												" ELSE  [AppliesT].[Amount]	  "
	 												" End,	  "
 												" TransProvID = CASE		  "
	 												" WHEN [RefundsT].[ProviderID] Is Null	  "
	 												" THEN [PaymentsT].[ProviderID]	  "
	 												" ELSE [RefundsT].[ProviderID]	  "
	 												" End,	  "
 												" LineRefundsT.InputDate AS SourceIDate,		  "
 												" LineRefundsT.Date AS SourceTDate,		  "
 												" PatientsT.ReferringPatientID,	  "
 												" TransLocID = CASE	  "
	 												" WHEN [LinePaymentsT].[LocationID] Is Null	  "
	 												" THEN [LineRefundsT].[LocationID]	  "
	 												" ELSE [LinePaymentsT].[LocationID]	  "
	 												" End,	  "
 												" LinePaymentsT.InputDate AS DestIDate,	  "
 												" LinePaymentsT.Date AS DestTDate,	  "
 												" LineRefundsT.Date AS BillDate,	  "
 												" PatientsT.MainPhysician AS PatProvID,	  "
 												" PersonT.Location AS PatLocID, "
												" PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.STate, "
												" PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, PersonT.Email	 	  "
 												" FROM (((((((LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT RefundsT	  "
 												" ON LineRefundsT.ID = RefundsT.ID) LEFT JOIN	  "
 												" (SELECT LineRefs.ID, LineRefs.Amount AS RefAmt,		  "
 												" Sum(AppliesT.Amount) AS ApplyAmt,	  "
 												" /*First a Amount*/Min([LineRefs].[Amount])-Sum([AppliesT].[Amount]) AS Total,	  "
 												" LineRefs.PatientID		  "
 												" FROM LineItemT AS LineRefs LEFT JOIN (PaymentsT RefundsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePays ON AppliesT.DestID = LinePays.ID)	  "
	 											 " ON RefundsT.ID = AppliesT.SourceID) ON LineRefs.ID = RefundsT.ID	  "
 												" WHERE (((LineRefs.Deleted)=0)) AND (RefundsT.PayMethod NOT IN (4,10)) AND LineRefs.Type = 3	  "
 												" GROUP BY LineRefs.ID, LineRefs.Amount, LineRefs.PatientID	  "
 												" HAVING (((LineRefs.ID) is not  Null) AND ((MIN([LineRefs].[Amount])-Sum([AppliesT].[Amount])) <> 0))	  "
 												" ) AS _PartiallyAppliedRefsQ ON LineRefundsT.ID = [_PartiallyAppliedRefsQ].ID) LEFT JOIN AppliesT ON RefundsT.ID = AppliesT.SourceID)	  "
 												" LEFT JOIN LineItemT AS LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID) LEFT JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID)		  "
 												" LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineRefundsT.PatientID = PatientsT.PersonID)	  "
 												" WHERE (((RefundsT.ID) Is Not Null) AND ((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (PaymentsT.PayMethod NOT IN (4,10))		  "
												" %s " /*Filter 4 - [Resp2] [PatCoord] [Category2] [From2] [To2] [RefPat] */ 
 												" ) AS _PaymentsByReferralSourceFullQ		  "
											   " WHERE (1=1)  %s " /*Filter 2 - [Prov] [Loc1]   */ 
 												" UNION ALL	  "
 												" SELECT * FROM (SELECT [_PartiallyAppliedRefsQ].ID,	  "
 												" [_PartiallyAppliedRefsQ].Total AS Amount,	  "
 												" RefundsT.ProviderID AS TransProvID,		  "
 												" LineRefundsT.InputDate AS IDate,	  "
 												" LineRefundsT.Date AS TDate,	  "
 												" PatientsT.ReferringPatientID,	  "
 												" LineRefundsT.LocationID as TransLocID,	  "
 												" LineRefundsT.InputDate as OtherIDate,	  "
 												" LineRefundsT.Date as OtherTDate,	  "
 												" LineRefundsT.Date as BillDate,	  "
 												" PatientsT.MainPhysician as PatProvID,	  "
 												" PersonT.Location as PatLocID, "
												" PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.STate, "
												" PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, PersonT.Email	 	  "
 												" FROM ((((SELECT LineRefs.ID, LineRefs.Amount AS PayAmt,		  "
 												" Sum(AppliesT.Amount) AS ApplyAmt,	  "
 												" /*First a Amount*/Min([LineRefs].[Amount])-Sum([AppliesT].[Amount]) AS Total,	  "
 												" LineRefs.PatientID		  "
 												" FROM LineItemT AS LineRefs LEFT JOIN	  "
 												" (PaymentsT RefundsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePays ON AppliesT.DestID = LinePays.ID) ON RefundsT.ID = AppliesT.SourceID)	  "
 												" ON LineRefs.ID = RefundsT.ID	  "
 												" WHERE (((LineRefs.Deleted)=0)) AND (RefundsT.PayMethod NOT IN (4,10)) AND (LineRefs.Type = 3)	  "
 												" GROUP BY LineRefs.ID, LineRefs.Amount, LineRefs.PatientID	  "
 												" HAVING (((LineRefs.ID) is not  Null) AND ((MIN([LineRefs].[Amount])-Sum([AppliesT].[Amount])) <> 0))	  "
 												" ) AS _PartiallyAppliedRefsQ		  "
 												" INNER JOIN (LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID)	  "
 												" ON [_PartiallyAppliedRefsQ].ID = LineRefundsT.ID)	  "
 												" INNER JOIN PaymentsT RefundsT ON LineRefundsT.ID = RefundsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID)	  "
 												" ON LineRefundsT.PatientID = PatientsT.PersonID)	  "
 												" WHERE (((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (RefundsT.PayMethod NOT IN (4,10))	  "
												" %s " /* Filter 4 -  [Resp2] [PatCoord] [Category2] [From2] [To2] [RefPat] */ 
 												" ) AS _PaymentsByReferralSourcePartQ		  "
											   " WHERE (1=1)  %s " /*Filter 2 - [Prov] [Loc1]   */ 
 												" ) AS PaymentsByReferralSourceSubQ	  "
 												" LEFT JOIN PersonT PersonReferringPatT ON PaymentsByReferralSourceSubQ.ReferringPatientID = PErsonReferringPatT.ID)  "
 												" WHERE PersonReferringPatT.ID IS NOT NULL	  "
 												" GROUP BY PersonReferringPatT.ID, PersonReferringPatT.First, PersonReferringPatT.Last, PersonReferringPatT.Middle,  TransProvID, TransLocID, DestIDate, DestTDate, SourceIDate, SourceTDate, BillDate, PAtProvID, PatLocID, "
												" PaymentsByReferralSourceSubQ.UserDefinedID, PaymentsByReferralSourceSubQ.First, PaymentsByReferralSourceSubQ.Last, "
												" PaymentsByReferralSourceSubQ.Middle, PaymentsByReferralSourceSubQ.Address1, PaymentsByReferralSourceSubQ.Address2, "
												" PaymentsByReferralSourceSubQ.City, PaymentsByReferralSourceSubQ.State, PaymentsByReferralSourceSubQ.Zip, "
												" PaymentsByReferralSourceSubQ.HomePhone, PaymentsByReferralSourceSubQ.WorkPhone, PaymentsByReferralSourceSubQ.CellPhone, PaymentsByReferralSourceSubQ.Email	 "
											" ) RevenueQ	  "
											" GROUP BY RevenueQ.RefPatID, RevenueQ.Name, UserDefinedID, First, Last, Middle, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone, Email ",
											saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, strExtraText, strFilterField, strExtraText, strExtraField, strExtraText, strExtraField, strExtraText);

					}
				return strSql;
			}
		break;

		/*
			{
				CString strSql;
				if (GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>") == 1) {
					strSql.Format("SELECT PatientsT.ReferralID, ReferralSourceT.Name, "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 As UnionType "
						"	FROM PatientsT  "
						"	INNER JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID  "
						"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	WHERE PersonT.ID IN (SELECT AppointmentsT.PatientID  "
						"		FROM AppointmentsT  "
						"		INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"		WHERE (AptTypeT.Category = 4 	 "
						"		OR AptTypeT.Category = 3 		 "
						"		OR AptTypeT.Category = 6) 		 "
						"		AND AppointmentsT.ShowState <> 3   "
						"		AND AppointmentsT.Status <> 4  "
						"		GROUP BY AppointmentsT.PatientID)  "
						"		%s   "
						"	GROUP BY PatientsT.ReferralID, ReferralSourceT.Name,	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone "
						" UNION "
						" SELECT PatientsT.ReferralID, ReferralSourceT.Name, "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 2 As UnionType "
						"	FROM PatientsT  "
						"	INNER JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID  "
						"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	WHERE PersonT.ID IN (SELECT AppointmentsT.PatientID "
						"   FROM AppointmentsT  "
						"		INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"		WHERE (AptTypeT.Category = 1)   "
						"		AND AppointmentsT.ShowState <> 3 "
						"		AND AppointmentsT.STatus <> 4   "
						"		GROUP BY AppointmentsT.PatientID  "
						"	)   "
						"	%s   "
						"	GROUP BY PatientsT.ReferralID, ReferralSourceT.Name,	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone ", strExtraValue, strExtraValue);
					
					
				}
				else if (GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>") == 2) {
					strSql.Format("SELECT PatientsT.DefaultReferringPhyID, RefPhysT.Last + ', ' + RefPhysT.First + ' ' + RefPhysT.Middle AS Name, "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 As UnionType "
						"	FROM PatientsT  "
						"	INNER JOIN ReferringPhysT ON PatientsT.DefaultReferringPhyID = ReferringPhysT.PersonID  "
						"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	LEFT JOIN PersonT RefPhysT ON ReferringPhysT.PersonID = RefPhysT.ID "
						"	WHERE PersonT.ID IN (SELECT AppointmentsT.PatientID  "
						"		FROM AppointmentsT  "
						"		INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"		WHERE (AptTypeT.Category = 4 	 "
						"		OR AptTypeT.Category = 3 		 "
						"		OR AptTypeT.Category = 6) 		 "
						"		AND AppointmentsT.ShowState <> 3   "
						"		AND AppointmentsT.Status <> 4  "
						" 		GROUP BY AppointmentsT.PatientID) "
						"	%s	"
						"	GROUP BY PatientsT.DefaultReferringPhyID, RefPhysT.First, RefPhysT.Middle, RefPhysT.Last, PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone "
						"    UNION "
						"   SELECT PatientsT.DefaultReferringPhyID, RefPhysT.Last + ', ' + RefPhysT.First + ' ' + RefPhysT.Middle AS Name, "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 As UnionType "
						"	FROM PatientsT  "
						"	INNER JOIN ReferringPhysT ON PatientsT.DefaultReferringPhyID = ReferringPhysT.PersonID  "
						"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	LEFT JOIN PersonT RefPhysT ON ReferringPhysT.PersonID = RefPhysT.ID "
						"	WHERE PersonT.ID IN (SELECT AppointmentsT.PatientID "
						"		FROM AppointmentsT  "
						"		INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"		WHERE (AptTypeT.Category = 1)   "
						"		AND AppointmentsT.ShowState <> 3 "
						"		AND AppointmentsT.STatus <> 4   "
						"		GROUP BY AppointmentsT.PatientID  "
						"  )   "
						"	%s	"
						"	GROUP BY PatientsT.DefaultReferringPhyID, RefPhysT.First, RefPhysT.Middle, RefPhysT.Last, PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone ", strExtraValue, strExtraValue);
					
				}
				else {
					strSql.Format(" SELECT PatientsT.ReferringPatientID, PersonRefPat.Last + ', ' + PersonRefPat.First + ' ' + PersonRefPat.Middle AS Name, "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 As UnionType "
						"	FROM PatientsT  "
						"	INNER JOIN PersonT AS PersonRefPat ON PatientsT.ReferringPatientID = PersonRefPat.ID  "
						"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	WHERE PersonT.ID IN (SELECT AppointmentsT.PatientID  "
						"		FROM AppointmentsT  "
						"		INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"		WHERE (AptTypeT.Category = 4 	 "
						"		OR AptTypeT.Category = 3 		 "
						"		OR AptTypeT.Category = 6) 		 "
						"		AND AppointmentsT.ShowState <> 3   "
						"		AND AppointmentsT.Status <> 4  "
						"		GROUP BY AppointmentsT.PatientID)  "
						"	%s	 "
						"	GROUP BY PatientsT.ReferringPatientID, PersonRefPat.First, PersonRefPat.Middle, PersonRefPat.Last, PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone "
						"UNION "
						" SELECT PatientsT.ReferringPatientID, PersonRefPat.Last + ', ' + PersonRefPat.First + ' ' + PersonRefPat.Middle AS Name, "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 As UnionType "
						"	FROM PatientsT  "
						"	INNER JOIN PersonT AS PersonRefPat ON PatientsT.ReferringPatientID = PersonRefPat.ID  "
						"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	WHERE PersonT.ID IN (SELECT AppointmentsT.PatientID "
						"		FROM AppointmentsT  "
						"		INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"		WHERE (AptTypeT.Category = 1)   "
						"		AND AppointmentsT.ShowState <> 3 "
						"		AND AppointmentsT.STatus <> 4   "
						"		GROUP BY AppointmentsT.PatientID  "
						"	)   "
						"	%s"
						"	GROUP BY PatientsT.ReferringPatientID, PersonRefPat.First, PersonRefPat.Middle, PersonRefPat.Last, PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone", strExtraValue, strExtraValue);
				}
				return strSql;
			}
		break;*/
		case 680: //Conversion Rate by Referral Source (primary referral source)
			/* Version History
				//(e.lally 2009-09-24) PLID 35053 - Recreated the report under a new ID
			*/
			{
				CString strSql;
				strSql = "SELECT ConversionQ.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, "
					"PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, "
					"ConversionQ.PatientMainPhysicianID, ConversionQ.MainPhysicianFirst, ConversionQ.MainPhysicianLast, ConversionQ.MainPhysicianMiddle, "
					"ConversionQ.PatientLocationID, ConversionQ.PatientLocationName, "
					"ConversionQ.PatientCoordinatorID, CoordUsersT.Username as PatCoordUsername, PatientCoordT.First as PatCoordFirst, "
					"PatientCoordT.Last as PatCoordLast, PatientCoordT.Middle as PatCoordMiddle, "
					"ConversionQ.MasterProcedureID, ConversionQ.MasterProcedureName, "
					"ConversionQ.ConsultApptID, ConsultLocationID, ConsultLocationName, ConsultCreatedDate, ConsultCreatedLogin,  "
					"ConsultDate, ConsultStartTime, ConsultEndTime, ConsultAptTypeID, ConsultAptTypeName, "
					"\r\n"
					"ConversionQ.SurgeryApptID, SurgeryLocationID, SurgeryLocationName, SurgeryCreatedDate, SurgeryCreatedLogin, "
					"SurgeryDate, SurgeryStartTime, SurgeryEndTime, SurgeryAptTypeID, SurgeryAptTypeName, "
					"SurgeryDetailProcedureSubQ.ProcedureID AS SurgeryDetailedProcedureID, SurgeryDetailProcedureSubQ.ProcedureName AS SurgeryDetailedProcedureName, "
					"ConversionQ.PrimaryReferralID AS ReferralID, ReferralSourceT.Name AS ReferralName, MultiReferralsT.Date AS ReferralDate, "
					"ConversionQ.DefaultReferringPhyID AS RefPhyID, PersonT_RefPhys.First AS RefPhyFirst, PersonT_RefPhys.Middle AS RefPhyMiddle, PersonT_RefPhys.Last AS RefPhyLast, "
					"ConversionQ.ReferringPatientID AS RefPatID, PersonT_RefPatient.First AS RefPatFirst, PersonT_RefPatient.Middle AS RefPatMiddle, PersonT_RefPatient.Last AS RefPatLast "

					"\r\n\r\n"

					"FROM (" + 
						GetConsultToProcedureConversionRateBaseSql() + ") ConversionQ\r\n"
					"INNER JOIN PersonT WITH(NOLOCK) ON ConversionQ.PatientID = PersonT.ID \r\n"
					"LEFT JOIN UsersT CoordUsersT WITH(NOLOCK) ON ConversionQ.PatientCoordinatorID = CoordUsersT.PersonID "
						"AND CoordUsersT.PatientCoordinator <> 0 \r\n"
					"LEFT JOIN PersonT PatientCoordT WITH(NOLOCK) ON CoordUsersT.PersonID = PatientCoordT.ID \r\n"
					"LEFT JOIN PatientsT WITH(NOLOCK) ON ConversionQ.PatientID = PatientsT.PersonID \r\n"
					"LEFT JOIN ReferralSourceT WITH(NOLOCK) ON PatientsT.ReferralID = ReferralSourceT.PersonID \r\n"
					"LEFT JOIN MultiReferralsT WITH(NOLOCK) ON PatientsT.PersonID = MultiReferralsT.PatientID "
						"AND PatientsT.ReferralID = MultiReferralsT.ReferralID \r\n"
					"LEFT JOIN PersonT PersonT_RefPhys WITH(NOLOCK) ON ConversionQ.DefaultReferringPhyID = PersonT_RefPhys.ID \r\n"
					"LEFT JOIN PersonT PersonT_RefPatient WITH(NOLOCK) ON ConversionQ.ReferringPatientID = PersonT_RefPatient.ID \r\n"
					"LEFT JOIN "
						"(SELECT SurgeryDetailedProcedureT.Name AS ProcedureName, SurgeryDetailedProcedureT.ID AS ProcedureID, MasterProcedureID, AppointmentID "
						"FROM ProcedureT SurgeryDetailedProcedureT WITH(NOLOCK) \r\n"
						"INNER JOIN AppointmentPurposeT SurgeryPurposeT WITH(NOLOCK) ON SurgeryDetailedProcedureT.ID = SurgeryPurposeT.PurposeID "
					") SurgeryDetailProcedureSubQ \r\n"
						"ON ConversionQ.SurgeryApptID = SurgeryDetailProcedureSubQ.AppointmentID "
						"AND ConversionQ.SurgeryMasterProcedureID = COALESCE(SurgeryDetailProcedureSubQ.MasterProcedureID, SurgeryDetailProcedureSubQ.ProcedureID) "
					"";

				//(e.lally 2009-12-09) PLID 35654 - Added better handling for filtering based on the type of referral being viewed.
				//(e.lally 2010-03-12) PLID 37709 - This report is now going to show all the referral sources for each preview option,
					//but the referring physician and referring patient views are going to break down the special referral IDs into the 
					//detailed person.
				/*
				long nRefType = GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>");
				//These are the special mappings for the referral source used to indicate it was a referring physican or referred by patient.
				long nRefPatID = GetRemotePropertyInt("DefaultPatientReferral", -1, 0, "<None>", true);
				long nRefPhysID = GetRemotePropertyInt("DefaultPhysicianReferral", -1, 0, "<None>", true);
				CString strWhere="";
				switch(nRefType){
					case 1:
						strWhere.Format(" WHERE (ReferralSourceT.PersonID IS NULL OR ReferralSourceT.PersonID NOT IN(%li, %li)) ", nRefPatID, nRefPhysID);
					break;
					case 2:
						strWhere.Format(" WHERE ConversionQ.PrimaryReferralID = %li "
							"OR (ConversionQ.PrimaryReferralID IS NULL AND ConversionQ.DefaultReferringPhyID IS NOT NULL) ", nRefPhysID);
					break;
					case 3:
						strWhere.Format(" WHERE ConversionQ.PrimaryReferralID = %li "
							"OR (ConversionQ.PrimaryReferralID IS NULL AND ConversionQ.ReferringPatientID IS NOT NULL) ", nRefPatID);
					break;
				}

				strSql += strWhere;
				*/

				//Apply the filters that were sent in as extra values.
				ApplyConsultToProcedurePPFilters(strSql, saExtraValues);

				/*Passed in by marketview.cpp*/
				return strSql;
			}
			break;
		case 505:   //Patients by Referral Source
			/*	Version History
				???? - Created
				DRT 5/22/2008 - PLID 29966 - Added a join to MultiReferralsT for new date filtering options.  Did not change 
					the query output fields.  This only applies to the referrals section, ref phys and ref pat don't have
					referral dates, thus fall back to FCD.
				DRT 7/10/2008 - PLID 29966 - I added a MultiReferralsT query (not the table, but needs same name) to the ref phys
					and ref pat portions.  This enables those sections to properly use the referral date when you have the preference
					enabled to link them together.  If you do not, it falls back to the FCD for ref phys and ref pat.
				TES 8/4/2009 - PLID 35047 - Filtered out category 6 (Other Procedural)
			*/
			{
				CString strSql;
				if (GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>") == 1) {
					strSql.Format("	 SELECT PatientsT.ReferralID, ReferralSourceT.Name, "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, SubConsultQ.UnionType As HadConsult, SubQ.UnionType as HasSurgery "
						"	FROM PatientsT  "
						"	INNER JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID  "
						"	INNER JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID AND ReferralSourceT.PersonID = MultiReferralsT.ReferralID "
						"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	LEFT JOIN (  "
						"		SELECT AppointmentsT.PatientID, 1 as UnionType  "
						"		FROM AppointmentsT  "
						"		INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"		WHERE (AptTypeT.Category = 4 	 "
						"		OR AptTypeT.Category = 3) 	 "
						"		AND AppointmentsT.ShowState <> 3  "
						"		AND AppointmentsT.Status <> 4  "
						"		%s " //Filter 2
						"		GROUP BY AppointmentsT.PatientID  "
						"	) AS SubQ ON PatientsT.PersonID = SubQ.PatientID  "
						"	LEFT JOIN (  "
						"		SELECT  "
						"		AppointmentsT.PatientID, 1 as UnionType "
						"		FROM  "
						"		AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"		WHERE (AptTypeT.Category = 1 	 "
						"			AND AppointmentsT.ShowState <> 3  "
						"		AND AppointmentsT.Status <> 4 %s)  " //filter 2
						"	GROUP BY AppointmentsT.PatientID) SubConsultQ ON PatientsT.PersonID = SubConsultQ.PatientID  "
						"	WHERE (1 = 1)  "
						"	%s "
						"	GROUP BY PatientsT.ReferralID,ReferralSourceT.Name, 	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, SubConsultQ.UnionType, SubQ.UnionType", strExtraText, strExtraText, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/); 
				}
				else if (GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>") == 2) {
					//DRT 7/10/2008 - Used in new join to MultiReferralsT query.  See comments above.
					CString strReferralDateFilter = " AND (0=1)";
					long nReferralID = GetRemotePropertyInt("DefaultPhysicianReferral", -1, 0, "<None>", true);
					if(nReferralID != -1) {
						strReferralDateFilter = Descendants(nReferralID, "AND ReferralID");
					}

					strSql.Format("	 SELECT PatientsT.DefaultReferringPhyID as ReferralID, RefPhysT.Last + ', ' + RefPhysT.First + ' ' + RefPhysT.Middle AS Name, "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, SubConsultQ.UnionType As HadConsult, SubQ.UnionType as HasSurgery "
						"	FROM PatientsT  "
						"	INNER JOIN ReferringPhysT ON PatientsT.DefaultReferringPhyID = ReferringPhysT.PersonID  "
						"	INNER JOIN PersonT RefPhysT ON ReferringPhysT.PersonID = RefPhysT.ID "
						"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	LEFT JOIN (  "
						"		SELECT AppointmentsT.PatientID, 1 as UnionType  "
						"		FROM AppointmentsT  "
						"		INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"		WHERE (AptTypeT.Category = 4 	 "
						"		OR AptTypeT.Category = 3) 	 "
						"		AND AppointmentsT.ShowState <> 3  "
						"		AND AppointmentsT.Status <> 4  %s "
						"		GROUP BY AppointmentsT.PatientID  "
						"	) AS SubQ ON PatientsT.PersonID = SubQ.PatientID  "
						"	LEFT JOIN (  "
						"		SELECT  "
						"		AppointmentsT.PatientID, 1 as UnionType "
						"		FROM  "
						"		AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"		WHERE (AptTypeT.Category = 1 	 "
						"			AND AppointmentsT.ShowState <> 3  "
						"		AND AppointmentsT.Status <> 4 %s )  "
						"	GROUP BY AppointmentsT.PatientID) SubConsultQ ON PatientsT.PersonID = SubConsultQ.PatientID "
						"	LEFT JOIN "
						"		(SELECT MultiReferralsT.PatientID, MultiReferralsT.Date FROM MultiReferralsT WHERE (1=1) %s "
						"		) MultiReferralsT  "
						"	ON PatientsT.PersonID = MultiReferralsT.PatientID "
						"   WHERE (PatientsT.CurrentStatus <> 4) AND (1 = 1)  "
						"	%s  "
						"	GROUP BY PatientsT.DefaultReferringPhyID,RefPhysT.First, RefPhysT.Middle, RefPhysT.Last, PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, SubConsultQ.UnionType, SubQ.UnionType", 
						strExtraText, strExtraText, strReferralDateFilter, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/); 
				}
				else {
					//DRT 7/10/2008 - Used in new join to MultiReferralsT query.  See comments above.
					CString strReferralDateFilter = " AND (0=1)";
					long nReferralID = GetRemotePropertyInt("DefaultPatientReferral", -1, 0, "<None>", true);
					if(nReferralID != -1) {
						strReferralDateFilter = Descendants(nReferralID, "AND ReferralID");
					}

					strSql.Format("	SELECT PatientsT.ReferringPatientID as ReferralID, PersonRefPat.Last + ', ' + PersonRefPat.First + ' ' + PersonRefPat.Middle AS Name, "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, SubConsultQ.UnionType As HadConsult, SubQ.UnionType as HasSurgery "
						"	FROM PatientsT  "
						"	INNER JOIN PersonT AS PersonRefPat ON PatientsT.ReferringPatientID = PersonRefPat.ID  "
						"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	LEFT JOIN (  "
						"		SELECT AppointmentsT.PatientID, 1 as UnionType  "
						"		FROM AppointmentsT  "
						"		INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"		WHERE (AptTypeT.Category = 4 	 "
						"		OR AptTypeT.Category = 3) 	 "
						"		AND AppointmentsT.ShowState <> 3  "
						"		AND AppointmentsT.Status <> 4  %s "
						"		GROUP BY AppointmentsT.PatientID  "
						"	) AS SubQ ON PatientsT.PersonID = SubQ.PatientID  "
						"	LEFT JOIN (  "
						"		SELECT  "
						"		AppointmentsT.PatientID, 1 as UnionType "
						"		FROM  "
						"		AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"		WHERE (AptTypeT.Category = 1 	 "
						"			AND AppointmentsT.ShowState <> 3  "
						"		AND AppointmentsT.Status <> 4 %s)  "
						"	GROUP BY AppointmentsT.PatientID) SubConsultQ ON PatientsT.PersonID = SubConsultQ.PatientID  "
						"	LEFT JOIN "
						"		(SELECT MultiReferralsT.PatientID, MultiReferralsT.Date FROM MultiReferralsT WHERE (1=1) %s "
						"		) MultiReferralsT  "
						"	ON PatientsT.PersonID = MultiReferralsT.PatientID "
						"	WHERE (PatientsT.CurrentStatus <> 4) AND  (1 = 1)  "
						"	%s  "
						"	GROUP BY PatientsT.ReferringPatientID, PersonRefPat.First, PersonRefPat.Middle, PersonRefPat.Last, PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, SubConsultQ.UnionType, SubQ.UnionType", 
						strExtraText, strExtraText, strReferralDateFilter, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/);
				}
				return strSql;
			}
		break;
		case 506:   //No shows and Cancellations by Referral Source
			{
				CString strSql;
				if (GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>") == 1) {
					strSql.Format("SELECT MultiReferralsT.ReferralID, ReferralSourceT.Name, "
						" PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						" PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 As UnionType "
						" FROM AppointmentsT  LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID   "
						" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID    "
						" LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID  "
						" LEFT JOIN ReferralSourceT ON MultiReferralsT.ReferralID = ReferralSourceT.PersonID "
						" WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 3 AND MultiReferralsT.ReferralID IS NOT NULL  "
						" AND AppointmentsT.StartTime <= GetDate()   "
						"  %s"
						"  Group By MultiReferralsT.ReferralID, ReferralSourceT.Name, "
						" PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						" PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AppointmentsT.ID "
						" UNION ALL SELECT MultiReferralsT.ReferralID, ReferralSourceT.Name, "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 2 As UnionType "
						" FROM AppointmentsT  LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID   "
						" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID    "
						" LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID  "
						" LEFT JOIN ReferralSourceT ON MultiReferralsT.ReferralID = ReferralSourceT.PersonID "
						" WHERE AppointmentsT.Status = 4 AND AppointmentsT.ShowState <> 3 AND MultiReferralsT.ReferralID IS NOT NULL  "
						" AND AppointmentsT.StartTime <= GetDate()   "
						" %s"
						"  Group By MultiReferralsT.ReferralID, ReferralSourceT.Name, "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AppointmentsT.ID", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/); 
						
				}
				else if (GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>") == 2) {
					strSql.Format("SELECT PatientsT.DefaultReferringPhyID as ReferralID, RefPhysT.Last + ', ' + RefPhysT.First + ' ' + RefPhysT.Middle AS Name, "
						"PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 As UnionType "
						" FROM AppointmentsT  LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID   "
						" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID    "
						"LEFT JOIN PersonT RefPhysT ON PatientsT.DefaultReferringPhyID = RefPhysT.ID "
						" WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 3 AND PatientsT.DefaultReferringPhyID IS NOT NULL AND PatientsT.DefaultReferringPhyID > 0 "
						" AND AppointmentsT.StartTime <= GetDate()   "
						"	%s "
						"  Group By PatientsT.DefaultReferringPhyID, RefPhysT.Last, RefPhysT.First, RefPhysT.Middle, "
						"PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AppointmentsT.ID "
						" UNION ALL SELECT  PatientsT.DefaultReferringPhyID, RefPhysT.Last + ', ' + RefPhysT.First + ' ' + RefPhysT.Middle AS Name, "
						"PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 2 As UnionType "
						" FROM AppointmentsT  LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID   "
						" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID    "
						"LEFT JOIN PersonT RefPhysT ON PatientsT.DefaultReferringPhyID = RefPhysT.ID "
						" WHERE AppointmentsT.Status = 4 AND AppointmentsT.ShowState <> 3 AND PatientsT.DefaultReferringPhyID IS NOT NULL  AND PatientsT.DefaultReferringPhyID > 0"
						" AND AppointmentsT.StartTime <= GetDate()   "
						"   %s "
						"  Group By PatientsT.DefaultReferringPhyID, RefPhysT.Last, RefPhysT.First, RefPhysT.Middle, "
						"PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AppointmentsT.ID", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/); 
				}
				else {
					strSql.Format("SELECT PatientsT.ReferringPatientID as ReferralID, RefPatT.Last + ', ' + RefPatT.First + ' ' + RefPatT.Middle AS Name, "
						"PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 As UnionType "
						" FROM AppointmentsT  LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID   "
						" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID    "
						"LEFT JOIN PersonT RefPatT ON PatientsT.ReferringPatientID = RefPatT.ID "
						" WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 3 AND PatientsT.ReferringPatientID IS NOT NULL  "
						" AND AppointmentsT.StartTime <= GetDate()   "
						"	%s "
						"  Group By PatientsT.ReferringPatientID, RefPatT.Last, RefPatT.First, RefPatT.Middle, "
						"PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AppointmentsT.ID "
						" UNION ALL SELECT  PatientsT.ReferringPatientID, RefPatT.Last + ', ' + RefPatT.First + ' ' + RefPatT.Middle AS Name, "
						"PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 2 As UnionType "
						" FROM AppointmentsT  LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID   "
						" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID    "
						"LEFT JOIN PersonT RefPatT ON PatientsT.ReferringPatientID = RefPatT.ID "
						" WHERE AppointmentsT.Status = 4 AND AppointmentsT.ShowState <> 3 AND PatientsT.ReferringPatientID IS NOT NULL  "
						" AND AppointmentsT.StartTime <= GetDate()   "
						"   %s "
						"  Group By PatientsT.ReferringPatientID, RefPatT.Last, RefPatT.First, RefPatT.Middle, "
						"PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AppointmentsT.ID", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/); 
				}
				return strSql;
			}
		break;
		case 507:   //Inquiries To Consults By Referral Source
			{
				CString strSql;
				strSql.Format("	 SELECT ProsWithConsQ.ReferralID, *	 "
						"	FROM (  "
						"	 SELECT PatientsT.PersonID, MultiReferralsT.ReferralID, ReferralSourceT.Name, "
						"	 PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"  	 PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 As UnionType   "
						"	 FROM PatientsT LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID  "
						"	 INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	 LEFT JOIN ReferralSourceT ON MultiReferralsT.ReferralID = ReferralSourceT.PersonID "
						"	 WHERE CurrentStatus = 4   "
						"	%s"
						"	 UNION     "
						"	 SELECT PatientsT.PersonID, MultiReferralsT.ReferralID, ReferralSourceT.Name, "
						"	 PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"  	 PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 As UnionType   "
						"	 FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID  "
						"	 LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID  "
						"	 INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	 LEFT JOIN ReferralSourceT ON MultiReferralsT.ReferralID = ReferralSourceT.PersonID "
						"	 WHERE OldStatus = 4  "
						"	%s "
						"	 ) AS ProsWithConsQ   "
						"	 WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID    "
						"	 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)   "
						"	  AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4  %s ) AND ReferralID IS NOT NULL  "
						"	 GROUP BY PersonID, ReferralID,  	 UserDefinedID, First, Last, Middle, Address1, Address2, City, State, Zip,  "
						"  	 HomePhone, WorkPhone, CellPhone, UnionType, Name ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, strExtraText);
				return strSql;
			}
		break;
		case 508:   //Prospects to Consults By Referral Source
			{
				CString strSql;
				if (GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>") == 1) {
					strSql.Format("	 SELECT ProsWithConsQ.ReferralID, Name ,UserDefinedID, First, Last, Middle, Address1, Address2, City, State, Zip,  "
						"  	 HomePhone, WorkPhone, CellPhone "
						"	 FROM (  "
						"	 SELECT PatientsT.PersonID, MultiReferralsT.ReferralID, ReferralSourceT.Name, "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone "
						"	 FROM PatientsT LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID  "
						"	 INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	 LEFT JOIN ReferralSourceT ON MultiReferralsT.ReferralID = ReferralSourceT.PersonID "
						"	 WHERE CurrentStatus = 2  "
						"		%s "
						"	 UNION     "
						"	 SELECT PatientsT.PersonID, MultiReferralsT.ReferralID, ReferralSourceT.Name, "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone "
						"	 FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID LEFT JOIN MultiReferralsT ON   "
						"	 PatientsT.PersonID = MultiReferralsT.PatientID  "
						"	 INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	 LEFT JOIN ReferralSourceT ON MultiReferralsT.ReferralID = ReferralSourceT.PersonID "
						"	 WHERE OldStatus = 2  "
						"		%s "
						"	 ) AS ProsWithConsQ   "
						"	 WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID    "
						"	 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)   "
						"	  AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4  %s ) AND ReferralID IS NOT NULL  "
						"	 GROUP BY ReferralID, Name, UserDefinedID, First, Last, Middle, Address1, Address2, City, State, Zip,   "
						"  	 HomePhone, WorkPhone, CellPhone", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, strExtraText); 
				}
				else if (GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>") == 2) {
					strSql.Format("	 SELECT ProsWithConsQ.DefaultReferringPhyID as ReferralID, Name ,UserDefinedID, First, Last, Middle, Address1, Address2, City, State, Zip,   "
						"  	 HomePhone, WorkPhone, CellPhone FROM (  "
						"	  SELECT PatientsT.PersonID, PatientsT.DefaultReferringPhyID, RefPhysT.Last + ', ' + RefPhysT.First + ' ' + RefPhysT.Middle AS Name,  "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,   "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone  "
						"	 FROM PatientsT  "
						"	 INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	  LEFT JOIN PersonT RefPhysT ON PatientsT.DefaultReferringPhyID = RefPhysT.ID "
						"	 WHERE CurrentStatus = 2   "
						"		%s  "
						"	 UNION     "
						"	 SELECT PatientsT.PersonID, DefaultReferringPhyID, RefPhysT.Last + ', ' + RefPhysT.First + RefPhysT.Middle AS Name,  "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,   "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone  "
						"	 FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID   "
						"	 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
						"	  LEFT JOIN PersonT RefPhysT ON PatientsT.DefaultReferringPhyID = RefPhysT.ID "
						"	 WHERE OldStatus = 2  "
						"	%s "
						"	 ) AS ProsWithConsQ   "
						"	 WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID    "
						"	 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)   "
						"	  AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4  ) AND DefaultReferringPhyID IS NOT NULL  "
						"	 GROUP BY DefaultReferringPhyID,  Name ,UserDefinedID, First, Last, Middle, Address1, Address2, City, State, Zip,   "
						"  	 HomePhone, WorkPhone, CellPhone ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/);
				}
				else {
					strSql.Format("	 SELECT  ProsWithConsQ.ReferringPatientID as ReferralID, Name ,UserDefinedID, First, Last, Middle, Address1, Address2, City, State, Zip,    "
						"  	 HomePhone, WorkPhone, CellPhone  FROM (  "
						"	  SELECT PatientsT.PersonID, PatientsT.ReferringPatientID, RefPatT.Last + ', ' + RefPatT.First + RefPatT.Middle AS Name, "
						"	 PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,    "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone     "
						"	 FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	LEFT JOIN PersonT RefPatT ON PatientsT.ReferringPatientID = RefPatT.Id "
						"	 WHERE CurrentStatus = 2   "
						"		%s  "
						" UNION     "
						"	  SELECT PatientsT.PersonID, PatientsT.ReferringPatientID, RefPatT.Last + ', ' + RefPatT.First + RefPatT.Middle AS Name, "
						"	 PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,    "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone     "
						"	 FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID   "
						"	 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"	LEFT JOIN PersonT RefPatT ON PatientsT.ReferringPatientID = RefPatT.Id "
						"	 WHERE OldStatus = 2  "
						"		%s"
						"	 ) AS ProsWithConsQ   "
						"	 WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID    "
						"	 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)   "
						"	  AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4  ) AND ReferringPatientID IS NOT NULL  "
						" 	 GROUP BY ReferringPatientID,  Name ,UserDefinedID, First, Last, Middle, Address1, Address2, City, State, Zip,   "
						"  	 HomePhone, WorkPhone, CellPhone ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/);
				}
				return strSql;
			}
		break;
		case 509:    //Procedures Performed Vs. Closed By Referral Source
			/* Version History
				TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
				// (j.jones 2010-09-22 14:50) - PLID 34140 - made it so we ignore ProcInfoPaymentsT records if they are fully refunded/adjusted
			*/
			{
				CString strSql;
				if (GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>") == 1) {
					strSql.Format("SELECT  Count(AppointmentPurposeT.AppointmentID) AS ProcsClosed, 0 AS ProcsPerformed,  MultiReferralsT.ReferralID, ReferralSourceT.Name, PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,     "
						"    	 PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 AS Uniontype     "
						"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON   "
						"	 AppointmentsT.PatientID = PatientsT.PersonID     "
						"	 LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID   "
						"	 LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID  "
						"	 LEFT JOIN ReferralSourceT ON MultiReferralsT.ReferralID = ReferralSourceT.PersonID "
						"	 WHERE (PersonT.ID > 0)  AND AppointmentsT.ID IN (SELECT SurgeryApptID FROM ProcInfoT "
						"	 INNER JOIN (SELECT * FROM ProcInfoPaymentsT WHERE PayID NOT IN ( "
						"		SELECT LineItemT.ID FROM LineItemT "
						"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
						"		INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID "
						"		GROUP BY LineItemT.ID, LineItemT.Amount "
						"		HAVING LineItemT.Amount = Sum(-AppliesT.Amount))) AS ProcInfoPaymentsT ON ProcInfoT.ID = ProcInfoPaymentsT.ProcInfoID) "
						"	%s  "
						"	 AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3  AND MultiReferralsT.ReferralID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4)  "
						"	 GROUP BY MultiReferralsT.ReferralID, ReferralSourceT.Name, PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,     "
						"    	 PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone      "
						"	 UNION ALL SELECT 0 AS ProcsClosed, Count(AppointmentPurposeT.AppointmentID) AS ProcsPerformed, MultiReferralsT.ReferralID, ReferralSourceT.Name, PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,     "
						"    	 PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 2 As UnionType      "
						"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON   "
						" 	 AppointmentsT.PatientID = PatientsT.PersonID    "
						"	  LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID   "
						"	 LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID  "
						"	 LEFT JOIN ReferralSourceT ON MultiReferralsT.ReferralID = ReferralSourceT.PersonID "
						"	  WHERE (PersonT.ID > 0)  "
						"		%s "
						"	  AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 2 AND MultiReferralsT.ReferralID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4)  "
						"	  GROUP BY MultiReferralsT.ReferralID, ReferralSourceT.Name, PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,     "
						"    	 PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone  ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/); 
				}
				else if (GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>") == 2) {
					strSql.Format("SELECT Count(AppointmentPurposeT.AppointmentID) AS ProcsClosed, 0 AS ProcsPerformed, PatientsT.DefaultReferringPhyID as ReferralID,RefPhysT.Last + ', ' + RefPhysT.First + ' ' + RefPhysT.Middle AS Name,   "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,    "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 As UnionType "
						"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON   "
						"	 AppointmentsT.PatientID = PatientsT.PersonID     "
						"	 LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID   "
						"	LEFT JOIN PersonT RefPhysT ON  PatientsT.DefaultReferringPhyID = RefPhysT.ID "
						"	 WHERE (PersonT.ID > 0)  AND AppointmentsT.ID IN (SELECT SurgeryApptID FROM ProcInfoT "
						"	 INNER JOIN (SELECT * FROM ProcInfoPaymentsT WHERE PayID NOT IN ( "
						"		SELECT LineItemT.ID FROM LineItemT "
						"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
						"		INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID "
						"		GROUP BY LineItemT.ID, LineItemT.Amount "
						"		HAVING LineItemT.Amount = Sum(-AppliesT.Amount))) AS ProcInfoPaymentsT ON ProcInfoT.ID = ProcInfoPaymentsT.ProcInfoID) "
						"		%s "
						"	 AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND PatientsT.DefaultReferringPhyID IS NOT NULL AND PatientsT.DefaultReferringPhyID > 0 AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4)  "
						"	 GROUP BY PatientsT.DefaultReferringPhyID,RefPhysT.Last, RefPhysT.First, RefPhysT.Middle,   "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,    "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone "
						"	 UNION ALL SELECT 0 AS ProcsClosed, Count(AppointmentPurposeT.AppointmentID) AS ProcsPerformed, PatientsT.DefaultReferringPhyID, RefPhysT.Last + ', ' + RefPhysT.First + ' ' + RefPhysT.Middle AS Name,   "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,    "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 2 As UnionType "
						"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON   "
						"		 AppointmentsT.PatientID = PatientsT.PersonID "
						"	  LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID   "
						"	LEFT JOIN PersonT RefPhysT ON  PatientsT.DefaultReferringPhyID = RefPhysT.ID "
						"	  WHERE (PersonT.ID > 0)  "
						"	%s"
						"	  AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 2 AND PatientsT.DefaultReferringPhyID IS NOT NULL AND PatientsT.DefaultReferringPhyID > 0 AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4)  "
						"	  GROUP BY PatientsT.DefaultReferringPhyID,RefPhysT.Last, RefPhysT.First, RefPhysT.Middle,    "
						"		PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,    "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/); 
				}
				else {
					strSql.Format("	SELECT Count(AppointmentPurposeT.AppointmentID) AS ProcsClosed, 0 AS ProcsPerformed, PatientsT.ReferringPatientID as ReferralID, RefPatT.Last + ', ' + RefPatT.First + ' ' + RefPatT.Middle AS Name,    "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,     "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 As UnionType  "
						"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON   "
						"	 AppointmentsT.PatientID = PatientsT.PersonID "
						"	 LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID   "
						"	 LEFT JOIN PersonT RefPatT ON PatientsT.ReferringPatientID = RefPatT.ID "
						"	 WHERE (PersonT.ID > 0)  AND AppointmentsT.ID IN (SELECT SurgeryApptID FROM ProcInfoT "
						"		INNER JOIN (SELECT * FROM ProcInfoPaymentsT WHERE PayID NOT IN ( "
						"			SELECT LineItemT.ID FROM LineItemT "
						"			INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
						"			INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID "
						"			GROUP BY LineItemT.ID, LineItemT.Amount "
						"			HAVING LineItemT.Amount = Sum(-AppliesT.Amount))) AS ProcInfoPaymentsT ON ProcInfoT.ID = ProcInfoPaymentsT.ProcInfoID) "
						"	%s "
						"	 AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND PatientsT.ReferringPatientID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4)  "
						"	 GROUP BY PatientsT.ReferringPatientID, "
						"	RefPatT.Last, RefPatT.First, RefPatT.Middle,    "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,     "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone "
						"	 UNION ALL SELECT 0 AS ProcsClosed, Count(AppointmentPurposeT.AppointmentID) AS ProcsPerformed, PatientsT.ReferringPatientID, RefPatT.Last + ', ' + RefPatT.First + ' ' + RefPatT.Middle AS Name,    "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,     "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 2 As UnionType  "
						"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
						"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON   "
						" 	 AppointmentsT.PatientID = PatientsT.PersonID    "
						"	  LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID   "
						"	 LEFT JOIN PersonT RefPatT ON PatientsT.ReferringPatientID = RefPatT.ID "
						"	  WHERE (PersonT.ID > 0)  "
						"		%s"
						"	  AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 2 AND PatientsT.ReferringPatientID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4)  "
						"	  GROUP BY PatientsT.ReferringPatientID, RefPatT.Last, RefPatT.First, RefPatT.Middle,    "
						"	PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,     "
						"	PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/); 
				}
				return strSql;
			}
		break;
			
		case 510:	//Revenue By Procedure 
			/* Version History
			PLID 18191 - 11/03/2005 - JMM - added support for products
			PLID 
			// (j.gruber 2009-03-25 15:21) - PLID 33358 - changed discount structure
			*/
			{
			CString strSql;
			strSql.Format("	SELECT CASE WHEN ProcedureID IS NULL THEN -1 ELSE ServiceT.ProcedureID END AS ProcID,   "
				" 		 SUM(AppliesT.Amount) AS SumMoney,    "
				" 		 Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate - 1)) AS Tax1,   "
				" 		Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate2 - 1)) AS Tax2, ProcedureT.Name, "
				" 		PatientsT.PersonID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, "
				"	    CASE WHEN CPTCodeT.Code IS NULL THEN ServiceT.Name ELSE CPTCodeT.Code END AS Code, "
				"       CASE WHEN CPTCodeT.SubCode IS NULL THEN '' ELSE CPTCodeT.SubCode END AS SubCode, "
				"       PatientsT.UserDefinedID "
				"		 FROM LineItemT LinePaymentsT INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID     "
				"		  LEFT JOIN AppliesT ON LinePaymentsT.ID = AppliesT.SourceID LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID    "
				"		  LEFT JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID    "
				"		 LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
				"		  LEFT JOIN PatientsT ON LinePaymentsT.PatientID = PatientsT.PersonID LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID    "
				"		  LEFT JOIN PersonT CoordPersonT ON ChargesT.PatCoordID = CoordPersonT.ID    "
				"		  LEFT JOIN PersonT ProvPersonT ON ChargesT.DoctorsProviders = ProvPersonT.ID LEFT JOIN LocationsT ON LineChargesT.LocationID = LocationsT.ID     "
				"		 LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
				"		 LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID  "
				"		  LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCOdeT.ID "
				"		LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "	
				"		 WHERE LinePaymentsT.Deleted = 0 AND LineChargesT.Deleted = 0 AND LinePaymentsT.Type = 1 AND LineChargesT.Type = 10 AND    "
				"		 AppliesT.ID IS NOT NULL  AND ServiceT.ProcedureID IS NOT NULL AND ProcedureT.MasterProcedureID Is Null  "
				"        %s      "
				"		 GROUP BY ServiceT.ProcedureID, ServiceT.Name, ProcedureT.Name, PatientsT.PersonID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, CPTCodeT.Code, CPTCodeT.SubCode, PatientsT.UserDefinedID "
				" UNION "
				"		 SELECT CASE WHEN ServiceT.ProcedureID IS NULL THEN -1 ELSE MasterProcT.ID END AS ProcID,   "
				" 		 SUM(AppliesT.Amount) AS SumMoney,    "
				" 		 Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate - 1)) AS Tax1,   "
				" 		Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate2 - 1)) AS Tax2, "
				"		MasterProcT.Name, "
				" 		PatientsT.PersonID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, "
				"	    CASE WHEN CPTCodeT.Code IS NULL THEN ServiceT.Name ELSE CPTCodeT.Code END AS Code, "
				"       CASE WHEN CPTCodeT.SubCode IS NULL THEN '' ELSE CPTCodeT.SubCode END AS SubCode, "
				"		PatientsT.UserDefinedID "
				"		 FROM LineItemT LinePaymentsT INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID     "
				"		  LEFT JOIN AppliesT ON LinePaymentsT.ID = AppliesT.SourceID LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID    "
				"		  LEFT JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID    "
				"		  LEFT JOIN PatientsT ON LinePaymentsT.PatientID = PatientsT.PersonID LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID    "
				"		LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
				"		  LEFT JOIN PersonT CoordPersonT ON ChargesT.PatCoordID = CoordPersonT.ID    "
				"		  LEFT JOIN PersonT ProvPersonT ON ChargesT.DoctorsProviders = ProvPersonT.ID LEFT JOIN LocationsT ON LineChargesT.LocationID = LocationsT.ID     "
				"		 LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
				"		 LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID  "
				"        LEFT JOIN ProcedureT MasterProcT ON ProcedureT.MasterProcedureID = MasterProcT.ID "
				"		 LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
				"		 LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
				"		 WHERE LinePaymentsT.Deleted = 0 AND LineChargesT.Deleted = 0 AND LinePaymentsT.Type = 1 AND LineChargesT.Type = 10 AND    "
				"		 AppliesT.ID IS NOT NULL  AND ServiceT.ProcedureID IS NOT NULL AND ProcedureT.MasterProcedureID Is NOT Null  "
				"        %s      "
				"		 GROUP BY ServiceT.ProcedureID, MasterProcT.Name, MasterProcT.ID, ServiceT.Name, ProcedureT.Name, PatientsT.PersonID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, CPTCodeT.Code, CPTCodeT.SubCode, PatientsT.UserDefinedID "
				, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/);

			return strSql;
			}
		break;

		case 673:     //Conversion Rate By Procedure 
			/* Version History
				TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
				//(e.lally 2009-08-27) PLID 35330 - Recreated the report and query under a new ID, updated to use new base query.
				//(e.lally 2009-09-08) PLID 35330 - Added primary referral source
			*/
			{
				CString strSql;
				strSql = "SELECT ConversionQ.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, "
					"PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, "
					"ConversionQ.PatientMainPhysicianID, ConversionQ.MainPhysicianFirst, ConversionQ.MainPhysicianLast, ConversionQ.MainPhysicianMiddle, "
					"ConversionQ.PatientLocationID, ConversionQ.PatientLocationName, "
					"ConversionQ.PatientCoordinatorID, CoordUsersT.Username as PatCoordUsername, PatientCoordT.First as PatCoordFirst, "
					"PatientCoordT.Last as PatCoordLast, PatientCoordT.Middle as PatCoordMiddle, "
					"ConversionQ.MasterProcedureID, ConversionQ.MasterProcedureName, "
					"ConversionQ.ConsultApptID, ConsultLocationID, ConsultLocationName, ConsultCreatedDate, ConsultCreatedLogin,  "
					"ConsultDate, ConsultStartTime, ConsultEndTime, ConsultAptTypeID, ConsultAptTypeName, "
					"\r\n"
					"ConversionQ.SurgeryApptID, SurgeryLocationID, SurgeryLocationName, SurgeryCreatedDate, SurgeryCreatedLogin, "
					"SurgeryDate, SurgeryStartTime, SurgeryEndTime, SurgeryAptTypeID, SurgeryAptTypeName, "
					"SurgeryDetailProcedureSubQ.ProcedureID AS SurgeryDetailedProcedureID, SurgeryDetailProcedureSubQ.ProcedureName AS SurgeryDetailedProcedureName, "
					"ReferralSourceT.PersonID AS PatPrimaryReferralID, ReferralSourceT.Name AS PatPrimaryReferral, MultiReferralsT.Date AS PatPrimaryReferralDate "
					"\r\n\r\n"

					"FROM (" + 
						GetConsultToProcedureConversionRateBaseSql() + ") ConversionQ\r\n"
					"INNER JOIN PersonT WITH(NOLOCK) ON ConversionQ.PatientID = PersonT.ID \r\n"
					"LEFT JOIN UsersT CoordUsersT WITH(NOLOCK) ON ConversionQ.PatientCoordinatorID = CoordUsersT.PersonID "
						"AND CoordUsersT.PatientCoordinator <> 0 \r\n"
					"LEFT JOIN PersonT PatientCoordT WITH(NOLOCK) ON CoordUsersT.PersonID = PatientCoordT.ID \r\n"
					"LEFT JOIN PatientsT WITH(NOLOCK) ON ConversionQ.PatientID = PatientsT.PersonID \r\n"
					"LEFT JOIN ReferralSourceT WITH(NOLOCK) ON PatientsT.ReferralID = ReferralSourceT.PersonID \r\n"
					"LEFT JOIN MultiReferralsT WITH(NOLOCK) ON PatientsT.PersonID = MultiReferralsT.PatientID "
						"AND PatientsT.ReferralID = MultiReferralsT.ReferralID \r\n"
					"LEFT JOIN "
						"(SELECT SurgeryDetailedProcedureT.Name AS ProcedureName, SurgeryDetailedProcedureT.ID AS ProcedureID, MasterProcedureID, AppointmentID "
						"FROM ProcedureT SurgeryDetailedProcedureT WITH(NOLOCK) \r\n"
						"INNER JOIN AppointmentPurposeT SurgeryPurposeT WITH(NOLOCK) ON SurgeryDetailedProcedureT.ID = SurgeryPurposeT.PurposeID "
					") SurgeryDetailProcedureSubQ \r\n"
						"ON ConversionQ.SurgeryApptID = SurgeryDetailProcedureSubQ.AppointmentID "
						"AND ConversionQ.SurgeryMasterProcedureID = COALESCE(SurgeryDetailProcedureSubQ.MasterProcedureID, SurgeryDetailProcedureSubQ.ProcedureID) "
					"";

				//(e.lally 2009-08-27) PLID 35330 - Apply the filters that were sent in as extra values.
				ApplyConsultToProcedurePPFilters(strSql, saExtraValues);
				
				/*Passed in by marketview.cpp*/
				return strSql;
			}
		break;

		case 512: {    //Patients By Procedure

			CString strSql;			
			 strSql.Format("SELECT PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, ProcInfoDetailsT.ProcedureID, ProcedureT.Name, 1 AS UnionType   "
				 "			 FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID     "
				 "			 LEFT JOIN ProcInfoT ON PatientsT.PersonID = ProcInfoT.PatientID LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID  "
				 "			 LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID  "
				 "			 WHERE CurrentStatus = 1  AND ProcInfoDetailsT.ProcedureID IS NOT NULL AND ProcedureT.MasterProcedureID Is Null    "
				 "			  %s     "
				 "			 GROUP BY PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, ProcInfoDetailsT.ProcedureID, ProcedureT.Name, ProcInfoT.ID "
				 "			 UNION ALL SELECT PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, ProcInfoDetailsT.ProcedureID, ProcedureT.Name, 2 AS UnionType   "
				 "			 FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID    "
				 "			 LEFT JOIN ProcInfoT ON PatientsT.PersonID = ProcInfoT.PatientID LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID  "
				 "			 LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID  "
				 "			  WHERE CurrentStatus = 2 AND ProcInfoDetailsT.ProcedureID IS NOT NULL  AND ProcedureT.MasterProcedureID Is Null  "
				 "		      %s     "
				 "			 GROUP BY PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, ProcInfoDetailsT.ProcedureID, ProcedureT.Name, ProcInfoT.ID ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/);
			 return strSql;
				  }
			break;			
			 
		case 513:    //No shows and Cancellations by Procedure
			{
				CString strSql;
				strSql.Format(" SELECT PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State,  "
					" 	PersonT.Zip, AptTypeT.Name As TypeName, PersonT.HomePhone, PersonT.CellPhone, PersonT.WorkPhone,  "
					"    CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AppointmentPurposeT.PurposeID ELSE ProcedureT.MasterProcedureID END AS PurposeID,  "
					"    CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AptPurposeT.Name ELSE (SELECT Name FROM ProcedureT ProcInner WHERE ProcInner.ID = ProcedureT.MasterProcedureID) END AS Name, "
					"   1 AS UnionType "
					"		 FROM AppointmentsT LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  "
					" 		 LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
					" 		 LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID  "
					" 		 LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
					"		 INNER JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID "
					" 		 WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 3 AND AppointmentPurposeT.PurposeID IS NOT NULL  "
					"			%s "
					" 		 Group By PAtientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, " 
					"	PersonT.Zip, AptTypeT.Name, PersonT.HomePhone, PersonT.CellPhone, PersonT.WorkPhone, AppointmentPurposeT.PurposeID, AptPurposeT.Name, AppointmentsT.ID, ProcedureT.MasterProcedureID "
					"  	UNION ALL SELECT PAtientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State,  "
					" 	PersonT.Zip, AptTypeT.Name As TypeName, PersonT.HomePhone, PersonT.CellPhone, PersonT.WorkPhone, "
					"    CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AppointmentPurposeT.PurposeID ELSE ProcedureT.MasterProcedureID END AS PurposeID,  "
					"    CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AptPurposeT.Name ELSE (SELECT Name FROM ProcedureT ProcInner WHERE ProcInner.ID = ProcedureT.MasterProcedureID) END AS Name, "
					"   2 AS UnionType "
					" 		 FROM AppointmentsT LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  " 
					" 		 LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
					" 		 LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID  " 
					" 		 LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID " 
					"		 INNER JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID "
					" 		 WHERE AppointmentsT.Status = 4 AND AppointmentsT.ShowState <> 3 AND AppointmentPurposeT.PurposeID IS NOT NULL "
					"		 %s "
					"			 Group By PAtientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State,  "
					"	PersonT.Zip, AptTypeT.Name, PersonT.HomePhone, PersonT.CellPhone, PersonT.WorkPhone, AppointmentPurposeT.PurposeID, AptPurposeT.Name, AppointmentsT.ID, ProcedureT.MasterProcedureID", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/); 
				return strSql;
			}
			break;
			
		case 514:  //Inquiries to Consult By Procedure
			{
			CString strSql;
			strSql.Format("SELECT  PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, "
				"    CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AppointmentPurposeT.PurposeID ELSE ProcedureT.MasterProcedureID END AS PurposeID,  "
		        "    CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AptPurposeT.Name ELSE (SELECT Name FROM ProcedureT ProcInner WHERE ProcInner.ID = ProcedureT.MasterProcedureID) END AS Name, "
				"	 PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.Workphone, PersonT.Cellphone "
				"	 FROM AppointmentsT   "
				"	 LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
				"	 LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  "
				"	 LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID  "
				"	 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
				"	 LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID  "
				"	 INNER JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID "
				"	 WHERE (AptTypeT.Category = 1) AND PatientID IN (SELECT PersonID FROM PatientStatusHistoryT WHERE OldStatus = 4)  AND (AppointmentPurposeT.PurposeID IS NOT NULL) "
				"	 AND AppointmentsT.ShowState <> 3   "
				"	 AND AppointmentsT.Status <> 4    "
				"	 AND  PersonID IN (	 SELECT PersonID  FROM  "
				"	 PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID LEFT JOIN ProcInfoT ON PersonT.ID = ProcInfoT.PatientID  "
				"	 LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID  "
				"	 LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID  "
				"	 WHERE ProcInfoDetailsT.ProcedureID IN (SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = AppointmentsT.ID)) "
				"	%s "
				"    GROUP BY ProcedureT.MasterProcedureID, AppointmentPurposeT.PurposeID, AptPurposeT.Name, PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, 	 PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.Workphone, PersonT.Cellphone ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/);

			return strSql;
			}
			break;
		case 515: //Prospects To Consults By Procedure
			{
				CString strSql;
				strSql.Format("SELECT PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, " 
					"    PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, ProcedureID, Name "
					"	 FROM PersonT LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
					"	 LEFT JOIN ProcInfoT ON PersonT.ID = ProcInfoT.PatientID LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID  "
					"	 LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID  "
					"	 WHERE ProcedureT.MasterProcedureID Is Null AND (PatientsT.CurrentStatus = 2 OR PatientsT.PersonID IN (SELECT PersonID FROM PatientStatusHistoryT WHERE OldStatus = 2))  "
					"	 AND PersonID IN (	 SELECT PatientID  "
					"	 FROM AppointmentsT LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
					"	 LEFT JOIN PatientsT ON AppointmentsT.PatientId = PatientsT.PersonID   "
					"	 LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  "
					"	 LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID  "
					"	 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
					"	 WHERE (AptTypeT.Category = 1) AND AppointmentsT.ShowState <> 3 AND AppointmentsT.STatus <> 4 AND (PatientsT.CurrentStatus = 2 OR PatientsT.PersonID IN "
					"    (SELECT PersonID FROM PatientStatusHistoryT WHERE OldStatus = 2))   "
					"	AND ProcInfoDetailsT.ProcedureID IN (SELECT CASE WHEN ProcInner.MasterProcedureID IS NULL THEN PurposeID ELSE ProcInner.MasterProcedureID END AS PurposeID FROM AppointmentPurposeT INNER JOIN ProcedureT ProcInner ON AppointmentPurposeT.PurposeID = ProcInner.ID WHERE AppointmentID = AppointmentsT.ID) "
					"	 %s) "
					"	 %s "
					"	 GROUP BY PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, "
					"    PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, procedureID, name", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/);
				return strSql;
			}
		break;

		case 516:

			//Procedure Closed Vs. Performed
			/* Version History
				TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
				// (j.jones 2010-09-22 14:50) - PLID 34140 - made it so we ignore ProcInfoPaymentsT records if they are fully refunded/adjusted
			*/
			{
				CString strSql;
				strSql.Format(" SELECT  Count(AppointmentsT.ID) AS ProcsClosed, 0 As ProcsPerformed, PatientsT.UserdefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
					"   PersonT.HomePhone, PersonT.CellPhone, PersonT.WorkPhone, AptTypeT.Name As TypeName, "
					" CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AptPurposeT.Name ELSE (SELECT Name FROM ProcedureT ProcInner WHERE ProcInner.ID = ProcedureT.MasterProcedureID) END AS Name, "
					"   1 As UnionType "
					"	FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
					"	LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
					"	AppointmentsT.PatientID = PatientsT.PersonID   "
					"	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID  "
					"	LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID  "
					"   INNER JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID "
					"	WHERE AppointmentsT.ID IN (SELECT SurgeryApptID FROM ProcInfoT "
					"		INNER JOIN (SELECT * FROM ProcInfoPaymentsT WHERE PayID NOT IN ( "
					"			SELECT LineItemT.ID FROM LineItemT "
					"			INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
					"			INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID "
					"			GROUP BY LineItemT.ID, LineItemT.Amount "
					"			HAVING LineItemT.Amount = Sum(-AppliesT.Amount))) AS ProcInfoPaymentsT ON ProcInfoT.ID = ProcInfoPaymentsT.ProcInfoID) "
					"   %s "
					" 	AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND AppointmentPurposeT.PurposeID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4)  "
					"	GROUP BY PatientsT.UserdefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
					"   PersonT.HomePhone, PersonT.CellPhone, PersonT.WorkPhone, AptTypeT.Name, AptPurposeT.Name, AppointmentsT.ID, ProcedureT.MasterProcedureID  "
					" UNION ALL "
					"   SELECT 0 As ProcsClosed, Count(AppointmentsT.ID) AS ProcsPerformed, PatientsT.UserdefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
					"   PersonT.HomePhone, PersonT.CellPhone, PersonT.WorkPhone, AptTypeT.Name As TypeName,  "
					"   CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN AptPurposeT.Name ELSE (SELECT Name FROM ProcedureT ProcInner WHERE ProcInner.ID = ProcedureT.MasterProcedureID) END AS Name, "
					"   2 AS UnionType   "
					"	FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
					"	LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
					" 	AppointmentsT.PatientID = PatientsT.PersonID "
					" 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID  "
					" 	LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID  "
					"	INNER JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID "
					"	WHERE AppointmentsT.Status <> 4 "
					"   %s "
					"	AND AppointmentsT.ShowState = 2 AND AppointmentPurposeT.PurposeID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4) "
					"	GROUP BY PatientsT.UserdefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
					"   PersonT.HomePhone, PersonT.CellPhone, PersonT.WorkPhone, AptTypeT.Name, AptPurposeT.Name, AppointmentsT.ID, ProcedureT.MasterProcedureID ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/); 
				return strSql;
			}
		break;

		case 517:
			//Revenue by Category
			// (j.gruber 2009-03-25 15:24) - PLID 33696 - changed discount structure
			{
				CString strSql;
				strSql.Format("SELECT CASE WHEN ServiceT.Category IS NULL THEN -1 ELSE ServiceT.Category END AS CategoryID,  "
					" SUM(AppliesT.Amount) AS SumMoney,   "
					" Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate - 1)) AS Tax1,  "
					" Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate2 - 1)) AS Tax2, CategoriesT.Name, "
					" PatientsT.PersonID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, CPTCodeT.Code, CPTCodeT.SubCode, PatientsT.UserDefinedID "
					" FROM LineItemT LinePaymentsT INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID    "
					"  LEFT JOIN AppliesT ON LinePaymentsT.ID = AppliesT.SourceID LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID   "
					"  LEFT JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID   "
					"  LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
					"  LEFT JOIN PatientsT ON LinePaymentsT.PatientID = PatientsT.PersonID LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
					"  LEFT JOIN PersonT CoordPersonT ON ChargesT.PatCoordID = CoordPersonT.ID   "
					"  LEFT JOIN PersonT ProvPersonT ON ChargesT.DoctorsProviders = ProvPersonT.ID LEFT JOIN LocationsT ON LineChargesT.LocationID = LocationsT.ID    "
					"  LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
					" LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "					
					"  LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
					" WHERE LinePaymentsT.Deleted = 0 AND LineChargesT.Deleted = 0 AND LinePaymentsT.Type = 1 AND LineChargesT.Type = 10 AND   "
					" AppliesT.ID IS NOT NULL   AND ServiceT.Category <> -1 "
					" %s "
					" GROUP BY ServiceT.Category, CategoriesT.Name, PatientsT.PersonID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, CPTCodeT.Code, CPTCodeT.SubCode, PatientsT.UserDefinedID ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/);

				return strSql;
			}
		break;
		


		case 518:  //Revenue By Patient Coordinator
			// (j.gruber 2009-03-25 15:25) - PLID 33696 - updated discount structure
			{
				CString strSql;
				strSql.Format("SELECT CASE WHEN PatCoordID IS NULL THEN -1 ELSE ChargesT.PatCoordID END AS CoordID,  "
					"			 SUM(AppliesT.Amount) AS SumMoney,    "
					"			  Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate - 1)) AS Tax1,  "
					"			 Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate2 - 1)) AS Tax2, CoordPersonT.First AS CoordFirst, CoordPersonT.Last AS CoordLast, CoordPersonT.Middle AS CoordMiddle, "
					"			PatientsT.PersonID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, CPTCodeT.Code, CPTCodeT.SubCode, PatientsT.UserDefinedID  "
					"			 FROM LineItemT LinePaymentsT INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID    "
					"			  LEFT JOIN AppliesT ON LinePaymentsT.ID = AppliesT.SourceID LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID    "
					"			  LEFT JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID    "
					"			 LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
					"			  LEFT JOIN PatientsT ON LinePaymentsT.PatientID = PatientsT.PersonID LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID    "
					"			  LEFT JOIN PersonT CoordPersonT ON ChargesT.PatCoordID = CoordPersonT.ID    "
					"			  LEFT JOIN PersonT ProvPersonT ON ChargesT.DoctorsProviders = ProvPersonT.ID LEFT JOIN LocationsT ON LineChargesT.LocationID = LocationsT.ID     "
					"			  LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
					"			  LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
					"			 WHERE LinePaymentsT.Deleted = 0 AND LineChargesT.Deleted = 0 AND LinePaymentsT.Type = 1 AND LineChargesT.Type = 10 AND    "
					"			 AppliesT.ID IS NOT NULL   AND PatCoordID <> -1 AND PatCoordID IS NOT NULL"
					"			%s "
					"			  GROUP BY LinePaymentsT.ID, AppliesT.ID, PatCoordID, CoordPersonT.First, CoordPersonT.Last, CoordPersonT.Middle, "
					" PatientsT.PersonID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, CPTCodeT.Code, CPTCodeT.SubCode, PatientsT.UserDefinedID ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/); 
				return strSql;
			}
		break;
		case 676:
			//Conversion Rate by Pat Coord
			/* Version History
				TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
				//(e.lally 2009-08-28) PLID 35331 - Recreated the report and query under a new ID, updated to use new base query.
				//(e.lally 2009-09-08) PLID 35331 - Added primary referral source
			*/
			{
				CString strSql;
				strSql = "SELECT ConversionQ.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, "
					"PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, "
					"ConversionQ.PatientMainPhysicianID, ConversionQ.MainPhysicianFirst, ConversionQ.MainPhysicianLast, ConversionQ.MainPhysicianMiddle, "
					"ConversionQ.PatientLocationID, ConversionQ.PatientLocationName, "
					"ConversionQ.PatientCoordinatorID, CoordUsersT.Username as PatCoordUsername, PatientCoordT.First as PatCoordFirst, "
					"PatientCoordT.Last as PatCoordLast, PatientCoordT.Middle as PatCoordMiddle, "
					"ConversionQ.MasterProcedureID, ConversionQ.MasterProcedureName, "
					"ConversionQ.ConsultApptID, ConsultLocationID, ConsultLocationName, ConsultCreatedDate, ConsultCreatedLogin,  "
					"ConsultDate, ConsultStartTime, ConsultEndTime, ConsultAptTypeID, ConsultAptTypeName, "
					"\r\n"
					"ConversionQ.SurgeryApptID, SurgeryLocationID, SurgeryLocationName, SurgeryCreatedDate, SurgeryCreatedLogin, "
					"SurgeryDate, SurgeryStartTime, SurgeryEndTime, SurgeryAptTypeID, SurgeryAptTypeName, "
					"SurgeryDetailProcedureSubQ.ProcedureID AS SurgeryDetailedProcedureID, SurgeryDetailProcedureSubQ.ProcedureName AS SurgeryDetailedProcedureName, "
					"ReferralSourceT.PersonID AS PatPrimaryReferralID, ReferralSourceT.Name AS PatPrimaryReferral, MultiReferralsT.Date AS PatPrimaryReferralDate "
					"\r\n\r\n"

					"FROM (" + 
						GetConsultToProcedureConversionRateBaseSql() + ") ConversionQ\r\n"
					"INNER JOIN PersonT WITH(NOLOCK) ON ConversionQ.PatientID = PersonT.ID \r\n"
					"LEFT JOIN UsersT CoordUsersT WITH(NOLOCK) ON ConversionQ.PatientCoordinatorID = CoordUsersT.PersonID "
						"AND CoordUsersT.PatientCoordinator <> 0 \r\n"
					"LEFT JOIN PersonT PatientCoordT WITH(NOLOCK) ON CoordUsersT.PersonID = PatientCoordT.ID \r\n"
					"LEFT JOIN PatientsT WITH(NOLOCK) ON ConversionQ.PatientID = PatientsT.PersonID \r\n"
					"LEFT JOIN ReferralSourceT WITH(NOLOCK) ON PatientsT.ReferralID = ReferralSourceT.PersonID \r\n"
					"LEFT JOIN MultiReferralsT WITH(NOLOCK) ON PatientsT.PersonID = MultiReferralsT.PatientID "
						"AND PatientsT.ReferralID = MultiReferralsT.ReferralID \r\n"
					"LEFT JOIN "
						"(SELECT SurgeryDetailedProcedureT.Name AS ProcedureName, SurgeryDetailedProcedureT.ID AS ProcedureID, MasterProcedureID, AppointmentID "
						"FROM ProcedureT SurgeryDetailedProcedureT WITH(NOLOCK) \r\n"
						"INNER JOIN AppointmentPurposeT SurgeryPurposeT WITH(NOLOCK) ON SurgeryDetailedProcedureT.ID = SurgeryPurposeT.PurposeID "
					") SurgeryDetailProcedureSubQ \r\n"
						"ON ConversionQ.SurgeryApptID = SurgeryDetailProcedureSubQ.AppointmentID "
						"AND ConversionQ.SurgeryMasterProcedureID = COALESCE(SurgeryDetailProcedureSubQ.MasterProcedureID, SurgeryDetailProcedureSubQ.ProcedureID) "
					"";

				//(e.lally 2009-08-28) PLID 35331 - Apply the filters that were sent in as extra values.
				ApplyConsultToProcedurePPFilters(strSql, saExtraValues);

				/*Passed in by marketview.cpp*/
				return strSql;
			}
			break;
		case 520:
			//Patients by Pat Coord
			{
				CString strSql;
				strSql.Format("	 SELECT EmployeeID, PatCoordT.First as CoordFirst, PatCoordT.Middle As CoordMiddle, PatCoordT.Last as CoordLast, "
					"	PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.last, PersonT.Address1, PersonT.Address2, PersonT.City, PErsonT.State, PersonT.ZIp, PersonT.HomePhone, PersonT.WorkPhone, PErsonT.CellPhone, 1 As unionType "
					"	 FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
					"	 LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID  "
					"	 WHERE CurrentStatus = 1 AND EmployeeID IS NOT NULL   "
					"	%s "
					"	 GROUP BY EmployeeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last, 	PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.last, PersonT.Address1, PersonT.Address2, PersonT.City, PErsonT.State, PersonT.ZIp, PersonT.HomePhone, PersonT.WorkPhone, PErsonT.CellPhone "
					"	 UNION  "
					" 	 SELECT EmployeeID,PatCoordT.First as CoordFirst, PatCoordT.Middle As CoordMiddle, PatCoordT.Last as CoordLast, 	PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.last, PersonT.Address1, PersonT.Address2, PersonT.City, PErsonT.State, PersonT.ZIp, PersonT.HomePhone, PersonT.WorkPhone, PErsonT.CellPhone, 2 AS UnionType "
					"	 FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
					"	 LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID  "
					"	 WHERE CurrentStatus = 2 AND EmployeeID IS NOT NULL  "
					"	%s "
					"	 GROUP BY EmployeeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last, 	PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.last, PersonT.Address1, PersonT.Address2, PersonT.City, PErsonT.State, PersonT.ZIp, PersonT.HomePhone, PersonT.WorkPhone, PErsonT.CellPhone", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/); 
			return strSql;
			}
		break;
		case 521:
			//No shows and Cancellations by Pat Coord
			{
				CString strSql;
				strSql.Format(" SELECT PatientsT.EmployeeID,PatCoordT.First as CoordFirst, PatCoordT.Middle as CoordMiddle, PatCoordT.Last AS CoordLast,"
					"PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 As UnionType, AptTypeT.Name as TypeName "
					" 			 FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
					"			 LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
					"			 LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID  "
					"			 LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID	"
					"			 WHERE AppointmentsT.Status = 4  AND PatientsT.EmployeeID IS NOT NULL  "
					"			%s"
					"			 Group By PatientsT.EmployeeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last, PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AptTypeT.Name, AppointmentsT.ID "
					"			UNION ALL SELECT PatientsT.EmployeeID,  "
					"		PatCoordT.First as CoordFirst, PatCoordT.Middle as CoordMiddle, PatCoordT.Last AS CoordLast, "
					" PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 2 As UnionType, AptTypeT.Name as TypeName "
					" FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID  "
					"			 LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
					"			 LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
					"			 LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID	"
					"			 WHERE AppointmentsT.Status <> 4  AND AppointmentsT.ShowState = 3 AND "
					"			 PatientsT.EmployeeID IS NOT NULL  "
					"	%s  "
					"			 Group By PatientsT.EmployeeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last, PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AptTypeT.Name, AppointmentsT.ID ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/);
			 return strSql;
			}
		case 522: {
			//Inquiries to Prospects by Patient Coordinator
			CString strSql;
			strSql.Format("	 SELECT ProsWithConsQ.UserID, CoordFirst, CoordMiddle, CoordLast, UserDefinedID, First, Last, Middle, "
				"	Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone  FROM (  "
				"	  SELECT PatientsT.PersonID, PersonT.UserID, PatCoordT.First As CoordFirst, PatCoordT.Middle AS CoordMiddle, PatCoordT.Last AS CoordLast, "
				"	PatientsT.UserDefinedID, PersonT.First, PErsonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone "
				"	 FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
				"	 LEFT JOIN PersonT PatCoordT ON PersonT.UserID = PatCoordT.ID 			  "
				"	 WHERE CurrentStatus = 4     "
				"    %s"
				"	 UNION     "
				"	 SELECT PatientsT.PersonID, PersonT.UserID, PatCoordT.First AS CoordFirst, PatCoordT.Middle As CoordMiddle, PatCoordT.Last AS CoordLast, "
				"	PatientsT.UserDefinedID, PersonT.First, PErsonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone "
				"	 FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
				"	 LEFT JOIN PersonT PatCoordT ON PersonT.UserID = PatCoordT.ID  "
				"	 WHERE OldStatus = 4  "
				" %s"
				"	 ) AS ProsWithConsQ   "
				"	 WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID    "
				"	 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)   "
				"	 AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4 %s ) AND UserID IS NOT NULL  "
				"	 GROUP BY UserID, CoordFirst, CoordMiddle, CoordLast, UserDefinedID, First, Last, Middle, "
				"	Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, strExtraText);
			return strSql;
			}
		break;
		case 523:
			{
			//Prospects with Consults
			CString strSql;
			strSql.Format(" SELECT ProsWithConsQ.EmployeeID, 0 As TotalPros, CoordFirst, CoordMiddle, CoordLast, UserDefinedID, First, Last, Middle, "
				"	Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone  FROM ( "
				"		  SELECT PatientsT.PersonID, PatientsT.EmployeeID, PatCoordT.First As CoordFirst, PatCoordT.Middle AS CoordMiddle, PatCoordT.Last AS CoordLast,"
				"		PatientsT.UserDefinedID, PersonT.First, PErsonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone  "
				"			 FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
				"			 LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID 			  "
				"			 WHERE CurrentStatus = 2     "
				"			 %s "
				"			 UNION     "
				"			 SELECT PatientsT.PersonID, PatientsT.EmployeeID, PatCoordT.First AS CoordFirst, PatCoordT.Middle As CoordMiddle, PatCoordT.Last AS CoordLast, "
				"	PatientsT.UserDefinedID, PersonT.First, PErsonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone  "
				"			 FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
				"			 LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID  "
				"			 WHERE OldStatus = 2  "
				"  %s "
				"			 ) AS ProsWithConsQ   "
				"			 WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID    "
				"			 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)   "
				"			 AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4  %s  ) AND EmployeeID IS NOT NULL  "
				"			 GROUP BY EmployeeID, CoordFirst, CoordMiddle, CoordLast, UserDefinedID, First, Last, Middle,  "
				"			Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, strExtraText); 
			return strSql;
			}
			break;

		case 524:   //Procedures Performed Vs. Scheduled
			/* Version History
				TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
				// (j.jones 2010-09-22 14:50) - PLID 34140 - made it so we ignore ProcInfoPaymentsT records if they are fully refunded/adjusted
			*/
			{
				CString strSql;
				strSql.Format("SELECT CAST(Count(AppointmentPurposeT.AppointmentID) AS FLOAT) AS ProcsClosed, EmployeeID, 0 AS ProcsPerformed, PatCoordT.First AS CoordFirst, PatCoordT.Middle AS CoordMiddle, PatCoordT.Last AS CoordLast,"
					"	PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 As UnionType "
					"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
					"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON   "
					" 	 AppointmentsT.PatientID = PatientsT.PersonID     "
					" 	 LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID  "
					"    LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID  "
					"	 WHERE (PersonT.ID > 0)  AND AppointmentsT.ID IN (SELECT SurgeryApptID FROM ProcInfoT "
					"		INNER JOIN (SELECT * FROM ProcInfoPaymentsT WHERE PayID NOT IN ( "
					"			SELECT LineItemT.ID FROM LineItemT "
					"			INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
					"			INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID "
					"			GROUP BY LineItemT.ID, LineItemT.Amount "
					"			HAVING LineItemT.Amount = Sum(-AppliesT.Amount))) AS ProcInfoPaymentsT ON ProcInfoT.ID = ProcInfoPaymentsT.ProcInfoID) "
					"	%s"
					"	 AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3  AND EmployeeID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4)  "
					" 	 GROUP BY EmployeeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last, 	PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AppointmentsT.ID  "
					" 	 UNION ALL SELECT 0 AS ProcsClosed, EmployeeID, CAST(Count(AppointmentPurposeT.AppointmentID) AS FLOAT) AS ProcsPerformed, PatCoordT.First AS CoordFirst, PatCoordT.Middle AS CoordMiddle, PatCoordT.Last AS CoordLast, "
					"		PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 2 As UnionType "
					" 	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
					"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON   "
					"	 AppointmentsT.PatientID = PatientsT.PersonID "
					"	  LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID    "
					"     LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID  "
					"	  WHERE (PersonT.ID > 0)  "
					"	%s "
					"	  AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 2 AND EmployeeID IS NOT NULL AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4)  "
					"	  GROUP BY EmployeeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last, 	PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AppointmentsT.ID", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/); 
				return strSql;
			}
			
		break;

		case 525:
			{
			//Revenue By Date
			// (r.gonet 2015-05-05 14:38) - PLID 66305 - Exclude Gift Certificate Refunds
			CString strSql;
			strSql.Format(" SELECT CAST (Sum(SumofAmount) AS FLOAT) as TotalRevenue, TMonthYear AS MonthYear, "
						  " UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone, "
		                  " DateName(Month, CAST(Left(SubQ.TMonthYear, Len(TMonthYear) - 4)+'/1/'+ Right(SubQ.TMonthYear, 4) AS DATETIME)) + ' - ' + Right(SubQ.TMonthYear, 4)  AS Name "
		                  "	FROM ( "
					 " SELECT Sum(PaymentsByDateSubQ.Amount) AS SumOfAmount,     "
					 " /*LTRIM(STR(DATEPART(MM, PaymentsByDateSubQ.IDate ))) + LTRIM(STR(DATEPART(YYYY, PaymentsByDateSubQ.IDate ))) AS IMonthYear,*/  "
					 " LTRIM(STR(DATEPART(MM, PaymentsByDateSubQ.TDate ))) + LTRIM(STR(DATEPART(YYYY, PaymentsByDateSubQ.TDate ))) AS TMonthYear, "
					 " PaymentsByDateSubQ.UserDefinedID, PaymentsByDateSubQ.First, PaymentsByDateSubQ.Middle, PaymentsByDateSubQ.Last, "
					 " PaymentsByDateSubQ.Address1, PaymentsByDateSubQ.Address2, PaymentsByDateSubQ.City, PaymentsByDateSubQ.State, "
					 " PaymentsByDateSubQ.Zip, PaymentsByDateSubQ.HomePhone, PaymentsByDateSubQ.WorkPhone, PaymentsByDateSubQ.CellPhone "
					 " FROM    "
						 " (SELECT * FROM    "
							 " (SELECT Amount = CASE    "
								 " WHEN [_PartiallyAppliedPaysQ].[ID] Is Null    "
								 " THEN CASE    "
									 " WHEN [LineChargesT].[ID] Is Null    "
									 " THEN [LinePaymentsT].[Amount]    "
									 " ELSE [AppliesT].[Amount]    "
									 " End    "
								 " ELSE [AppliesT].[Amount]    "
								 " End,     "
							 " TransProvID = CASE     "
								 " WHEN [DoctorsProviders] Is Null    "
								 " THEN [PaymentsT].[ProviderID]    "
								 " ELSE [DoctorsProviders]    "
								 " End,    "
							 " PatProvID = PatientsT.MainPhysician,  "
							 " LinePaymentsT.InputDate AS IDate,     "
							 " LinePaymentsT.Date,     "
							 " LinePaymentsT.Date  AS TDate,    "
							 " CASE WHEN LineChargesT.LocationID Is Null THEN LinePaymentsT.LocationID ELSE LineChargesT.LocationID END AS TransLocID,  "
							 " PersonT.Location as PatLocID, "
							 " PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, "
											 " PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone "
							 " FROM ((((((LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN    "
								 " (SELECT LinePaysInnerQ.ID, LinePaysInnerQ.Amount AS PayAmt,     "
								 " Sum(AppliesT.Amount) AS ApplyAmt,     "
								 " /*First a Amount*/Min([LinePaysInnerQ].[Amount])-Sum([AppliesT].[Amount]) AS Total,     "
								 " LinePaysInnerQ.PatientID     "
								 " FROM LineItemT AS LinePaysInnerQ LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LineChargesInnerQ ON AppliesT.DestID = LineChargesInnerQ.ID) ON PaymentsT.ID = AppliesT.SourceID)    "
								 " ON LinePaysInnerQ.ID = PaymentsT.ID    "
								 " WHERE (((LinePaysInnerQ.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))    "
								 " GROUP BY LinePaysInnerQ.ID, LinePaysInnerQ.Amount, LinePaysInnerQ.PatientID    "
								 " HAVING (((LinePaysInnerQ.ID) is not  Null) AND ((MIN([LinePaysInnerQ].[Amount])-Sum([AppliesT].[Amount])) <> 0))    "
								 " ) AS _PartiallyAppliedPaysQ    "
							 " ON LinePaymentsT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LineChargesT ON AppliesT.DestID = LineChargesT.ID)    "
							 " LEFT JOIN ChargesT ON LineChargesT.ID = ChargesT.ID)   "
							 " LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
							 " LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LinePaymentsT.PatientID = PatientsT.PersonID    "
							 " LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID   "
							 " WHERE (((PaymentsT.ID) Is Not Null) AND ((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))    "
							"  %s " //Filter1
							 " ) AS PaymentsByDateFullQ    "
							 " WHERE (1=1) %s   "  //Filter 2
						 " UNION ALL  "
						 " SELECT * FROM    "
							 " (SELECT [_PartiallyAppliedPaysQ].Total AS Amount,  PaymentsT.ProviderID AS TransProvID, PatientsT.PersonID as PatProvID,  "
							 " LinePaymentsT.InputDate AS IDate,  LinePaymentsT.Date, LinePaymentsT.Date AS TDate,    "
							 " LinePaymentsT.LocationID AS TransLocID, PersonT.Location AS PatLocID, "
							 " PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, "
											 " PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone "
							 " FROM ((   "
								 " (SELECT LinePaysInnerQ.ID, LinePaysInnerQ.Amount AS PayAmt,     "
								 " Sum(AppliesT.Amount) AS ApplyAmt,  /*First a Amount*/Min([LinePaysInnerQ].[Amount])-Sum([AppliesT].[Amount]) AS Total,     "
								 " LinePaysInnerQ.PatientID     "
								 " FROM LineItemT AS LinePaysInnerQ LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LineChargesInnerQ ON AppliesT.DestID = LineChargesInnerQ.ID) ON PaymentsT.ID = AppliesT.SourceID)    "
								 " ON LinePaysInnerQ.ID = PaymentsT.ID    "
								 " WHERE (((LinePaysInnerQ.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))    "
								 " GROUP BY LinePaysInnerQ.ID, LinePaysInnerQ.Amount, LinePaysInnerQ.PatientID    "
								 " HAVING (((LinePaysInnerQ.ID) is not  Null) AND ((MIN([LinePaysInnerQ].[Amount])-Sum([AppliesT].[Amount])) <> 0))    "
								 " ) AS _PartiallyAppliedPaysQ    "
							 " INNER JOIN (LineItemT LinePaymentsT LEFT JOIN LocationsT ON LinePaymentsT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LinePaymentsT.ID) INNER JOIN PaymentsT ON LinePaymentsT.ID =    "
							 " PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LinePaymentsT.PatientID = PatientsT.PersonID    "
							 " LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID   "
							 " WHERE (((LinePaymentsT.Deleted)=0) AND ((LinePaymentsT.Type)=1)) AND (PaymentsT.PayMethod NOT IN (4,10))    "
						" 	   %s " //Filter 4 [Resp] [PatCoord] [Category2] [From] [To] 
							 " ) AS PaymentsByDatePartQ   "
							 " WHERE (1=1) %s " // Filter 2[Prov] [Loc1]  
						 " ) AS PaymentsByDateSubQ    "
					 " GROUP BY PaymentsByDateSubQ.IDate,  PaymentsByDateSubQ.TDate,  PaymentsByDateSubQ.UserDefinedID, PaymentsByDateSubQ.First, PaymentsByDateSubQ.Middle, PaymentsByDateSubQ.Last, "
					 " PaymentsByDateSubQ.Address1, PaymentsByDateSubQ.Address2, PaymentsByDateSubQ.City, PaymentsByDateSubQ.State, "
					 " PaymentsByDateSubQ.Zip, PaymentsByDateSubQ.HomePhone, PaymentsByDateSubQ.WorkPhone, PaymentsByDateSubQ.CellPhone  "
					 " UNION ALL "
					 " /*Refunds*/  "
					 " SELECT Sum(PaymentsByDateSubQ.Amount) AS SumOfAmount,     "
					 " /*LTRIM(STR(DATEPART(MM, PaymentsByDateSubQ.IDate ))) + LTRIM(STR(DATEPART(YYYY, PaymentsByDateSubQ.IDate ))) AS IMonthYear,*/  "
					 " LTRIM(STR(DATEPART(MM, PaymentsByDateSubQ.TDate ))) + LTRIM(STR(DATEPART(YYYY, PaymentsByDateSubQ.TDate ))) AS TMonthYear, "
					 " PaymentsByDateSubQ.UserDefinedID, PaymentsByDateSubQ.First, PaymentsByDateSubQ.Middle, PaymentsByDateSubQ.Last, "
					 " PaymentsByDateSubQ.Address1, PaymentsByDateSubQ.Address2, PaymentsByDateSubQ.City, PaymentsByDateSubQ.State, "
					 " PaymentsByDateSubQ.Zip, PaymentsByDateSubQ.HomePhone, PaymentsByDateSubQ.WorkPhone, PaymentsByDateSubQ.CellPhone "
					 " FROM    "
						 " (SELECT * FROM    "
							 " (SELECT Amount = CASE    "
								 " WHEN [_PartiallyAppliedPaysQ].[ID] Is Null    "
								 " THEN CASE    "
									 " WHEN [LinePaymentsT].[ID] Is Null    "
									 " THEN [LineRefundsT].[Amount]    "
									 " ELSE [AppliesT].[Amount]    "
									 " End    "
								 " ELSE [AppliesT].[Amount]    "
								 " End,     "
							 " TransProvID = CASE     "
								 " WHEN PaymentsT.ProviderID Is Null    "
								 " THEN RefundsT.ProviderID  "
								 " ELSE PaymentsT.ProviderID   "
								 " End,    "
							 " PatientsT.MainPhysician AS PatProvID,  "
							 " LineRefundsT.InputDate AS IDate,     "
							 " LineRefundsT.Date,    "
							 " LineRefundsT.Date AS TDate,    "
							 " CASE WHEN LinePaymentsT.LocationID Is Null THEN LineRefundsT.LocationID ELSE LinePaymentsT.LocationID END AS TransLocID,  "
							 " PersonT.Location AS PatLocID, "
							 " PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, "
											 " PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone  "
							 " FROM ((((((LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID) LEFT JOIN PaymentsT RefundsT ON LineRefundsT.ID = RefundsT.ID) LEFT JOIN    "
								 " (SELECT LineRefsInnerQ.ID, LineRefsInnerQ.Amount AS PayAmt,     "
								 " Sum(AppliesT.Amount) AS ApplyAmt,     "
								 " /*First a Amount*/Min([LineRefsInnerQ].[Amount])-Sum([AppliesT].[Amount]) AS Total,     "
								 " LineRefsInnerQ.PatientID     "
								 " FROM LineItemT AS LineRefsInnerQ LEFT JOIN (PaymentsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePaysInnerQ ON AppliesT.DestID = LinePaysInnerQ.ID) ON PaymentsT.ID = AppliesT.SourceID)    "
								 " ON LineRefsInnerQ.ID = PaymentsT.ID    "
								 " WHERE (((LineRefsInnerQ.Deleted)=0)) AND (PaymentsT.PayMethod NOT IN (4,10))    "
								 " GROUP BY LineRefsInnerQ.ID, LineRefsInnerQ.Amount, LineRefsInnerQ.PatientID    "
								 " HAVING (((LineRefsInnerQ.ID) is not  Null) AND ((MIN([LineRefsInnerQ].[Amount])-Sum([AppliesT].[Amount])) <> 0))    "
								 " ) AS _PartiallyAppliedPaysQ    "
							 " ON LineRefundsT.ID = [_PartiallyAppliedPaysQ].ID) LEFT JOIN AppliesT ON RefundsT.ID = AppliesT.SourceID) LEFT JOIN LineItemT AS LinePaymentsT ON AppliesT.DestID = LinePaymentsT.ID)    "
							 " LEFT JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID) ON LineRefundsT.PatientID = PatientsT.PersonID    "
							 " LEFT JOIN PaymentPlansT ON RefundsT.ID = PaymentPlansT.ID   "
							 " WHERE (((RefundsT.ID) Is Not Null) AND ((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (RefundsT.PayMethod NOT IN (4,10))    "
						" 	 %s "  //Filter 3  [Resp2] [PatCoord] [Category2] [From2] [To2]  
							 " ) AS PaymentsByDateFullQ    "
							  " WHERE (1=1)  %s "  //Filter 2[Prov] [Loc1] 
						 " UNION ALL  "
						 " SELECT * FROM    "
							 " (SELECT [_PartiallyAppliedPaysQ].Total AS Amount,  RefundsT.ProviderID AS TransProvID,  PatientsT.MainPhysician as PatProvID,   "
							 " LineRefundsT.InputDate AS IDate, LineRefundsT.Date, LineRefundsT.Date AS TDate,    "
							 " LineRefundsT.LocationID AS TransLocID, PersonT.Location AS PatLocID, "
							 " PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, "
											 " PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone "
							 " FROM ((   "
								 " (SELECT LineRefsInnerQ.ID, LineRefsInnerQ.Amount AS PayAmt,     "
								 " Sum(AppliesT.Amount) AS ApplyAmt,  /*First a Amount*/Min([LineRefsInnerQ].[Amount])-Sum([AppliesT].[Amount]) AS Total,     "
								 " LineRefsInnerQ.PatientID     "
								 " FROM LineItemT AS LineRefsInnerQ LEFT JOIN (PaymentsT RefsT LEFT JOIN (AppliesT LEFT JOIN LineItemT LinePaysInnerQ ON AppliesT.DestID = LinePaysInnerQ.ID) ON RefsT.ID = AppliesT.SourceID)    "
								 " ON LineRefsInnerQ.ID = RefsT.ID    "
								 " WHERE (((LineRefsInnerQ.Deleted)=0)) AND (RefsT.PayMethod NOT IN (4,10))    "
								 " GROUP BY LineRefsInnerQ.ID, LineRefsInnerQ.Amount, LineRefsInnerQ.PatientID    "
								 " HAVING (((LineRefsInnerQ.ID) is not  Null) AND ((MIN([LineRefsInnerQ].[Amount])-Sum([AppliesT].[Amount])) <> 0))    "
								 " ) AS _PartiallyAppliedPaysQ    "
							 " INNER JOIN (LineItemT LineRefundsT LEFT JOIN LocationsT ON LineRefundsT.LocationID = LocationsT.ID) ON [_PartiallyAppliedPaysQ].ID = LineRefundsT.ID) INNER JOIN PaymentsT RefundsT ON LineRefundsT.ID =    "
							 " RefundsT.ID) LEFT JOIN (PatientsT INNER JOIN PersonT On PatientsT.PersonID = PersonT.ID) ON LineRefundsT.PatientID = PatientsT.PersonID    "
							 " LEFT JOIN PaymentPlansT ON RefundsT.ID = PaymentPlansT.ID   "
							 " WHERE (((LineRefundsT.Deleted)=0) AND ((LineRefundsT.Type)=3)) AND (RefundsT.PayMethod NOT IN (4,10))    "
						" 	   %s "  //Filter 3[Resp2] [PatCoord] [Category2] [From2] [To2]
							 " ) AS PaymentsByDatePartQ   "
							  " WHERE (1=1)  %s " //Filter 2
						 " ) AS PaymentsByDateSubQ    "
					 " GROUP BY PaymentsByDateSubQ.IDate,  PaymentsByDateSubQ.TDate, PaymentsByDateSubQ.UserDefinedID, PaymentsByDateSubQ.First, PaymentsByDateSubQ.Middle, PaymentsByDateSubQ.Last, "
					 " PaymentsByDateSubQ.Address1, PaymentsByDateSubQ.Address2, PaymentsByDateSubQ.City, PaymentsByDateSubQ.State, "
					 " PaymentsByDateSubQ.Zip, PaymentsByDateSubQ.HomePhone, PaymentsByDateSubQ.WorkPhone, PaymentsByDateSubQ.CellPhone "
					 " )  "
					 " SubQ  "
					 " Group By TMonthYear, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, strExtraText, strFilterField, strExtraText, strExtraField, strExtraText, strExtraField, strExtraText);
			return strSql;
			}
		break;
		case 677:
			//Conversion Rate By Date
			//(e.lally 2009-09-08) PLID 35332
			{
				CString strSql = "SELECT DateName(Month, ConversionQ.ConsultDate) + ' - ' + Convert(NVARCHAR(4), DatePart(YYYY, ConversionQ.ConsultDate)) AS MonthNameYear, \r\n"
					"Convert(NVARCHAR(4), DATEPART(YYYY, ConversionQ.ConsultDate)) + Right('0' + Convert(NVARCHAR(2), DATEPART(MM, ConversionQ.ConsultDate)) , 2) AS YearMonth, \r\n"
					"ConversionQ.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, "
					"PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, "
					"ConversionQ.PatientMainPhysicianID, ConversionQ.MainPhysicianFirst, ConversionQ.MainPhysicianLast, ConversionQ.MainPhysicianMiddle, "
					"ConversionQ.PatientLocationID, ConversionQ.PatientLocationName, "
					"ConversionQ.PatientCoordinatorID, CoordUsersT.Username as PatCoordUsername, PatientCoordT.First as PatCoordFirst, "
					"PatientCoordT.Last as PatCoordLast, PatientCoordT.Middle as PatCoordMiddle, "
					"ConversionQ.MasterProcedureID, ConversionQ.MasterProcedureName, "
					"ConversionQ.ConsultApptID, ConsultLocationID, ConsultLocationName, ConsultCreatedDate, ConsultCreatedLogin,  "
					"ConsultDate, ConsultStartTime, ConsultEndTime, ConsultAptTypeID, ConsultAptTypeName, "
					"\r\n"
					"ConversionQ.SurgeryApptID, SurgeryLocationID, SurgeryLocationName, SurgeryCreatedDate, SurgeryCreatedLogin, "
					"SurgeryDate, SurgeryStartTime, SurgeryEndTime, SurgeryAptTypeID, SurgeryAptTypeName, "
					"SurgeryDetailProcedureSubQ.ProcedureID AS SurgeryDetailedProcedureID, SurgeryDetailProcedureSubQ.ProcedureName AS SurgeryDetailedProcedureName, "
					"ReferralSourceT.PersonID AS PatPrimaryReferralID, ReferralSourceT.Name AS PatPrimaryReferral, MultiReferralsT.Date AS PatPrimaryReferralDate "
					"\r\n\r\n"

					"FROM (" + 
						GetConsultToProcedureConversionRateBaseSql() + ") ConversionQ\r\n"
					"INNER JOIN PersonT WITH(NOLOCK) ON ConversionQ.PatientID = PersonT.ID \r\n"
					"LEFT JOIN UsersT CoordUsersT WITH(NOLOCK) ON ConversionQ.PatientCoordinatorID = CoordUsersT.PersonID "
						"AND CoordUsersT.PatientCoordinator <> 0 \r\n"
					"LEFT JOIN PersonT PatientCoordT WITH(NOLOCK) ON CoordUsersT.PersonID = PatientCoordT.ID \r\n"
					"LEFT JOIN PatientsT WITH(NOLOCK) ON ConversionQ.PatientID = PatientsT.PersonID \r\n"
					"LEFT JOIN ReferralSourceT WITH(NOLOCK) ON PatientsT.ReferralID = ReferralSourceT.PersonID \r\n"
					"LEFT JOIN MultiReferralsT WITH(NOLOCK) ON PatientsT.PersonID = MultiReferralsT.PatientID "
						"AND PatientsT.ReferralID = MultiReferralsT.ReferralID \r\n"
					"LEFT JOIN "
						"(SELECT SurgeryDetailedProcedureT.Name AS ProcedureName, SurgeryDetailedProcedureT.ID AS ProcedureID, MasterProcedureID, AppointmentID "
						"FROM ProcedureT SurgeryDetailedProcedureT WITH(NOLOCK) \r\n"
						"INNER JOIN AppointmentPurposeT SurgeryPurposeT WITH(NOLOCK) ON SurgeryDetailedProcedureT.ID = SurgeryPurposeT.PurposeID "
					") SurgeryDetailProcedureSubQ \r\n"
						"ON ConversionQ.SurgeryApptID = SurgeryDetailProcedureSubQ.AppointmentID "
						"AND ConversionQ.SurgeryMasterProcedureID = COALESCE(SurgeryDetailProcedureSubQ.MasterProcedureID, SurgeryDetailProcedureSubQ.ProcedureID) "
					"";

				//(e.lally 2009-09-08) PLID 35332 - Apply the filters that were sent in as extra values.
				ApplyConsultToProcedurePPFilters(strSql, saExtraValues);
			
			return strSql;
			//return saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/;
			}
		break;
		case 527: {
			//Patients by Date
			CString strSql;
			strSql.Format("SELECT *, Left(SubQ.MonthYear, Len(MonthYear) - 4) AS MonthNum, Right(SubQ.MonthYear, 4) AS YearNum, DateName(Month, CAST(Left(SubQ.MonthYear, Len(MonthYear) - 4)+'/1/'+ Right(SubQ.MonthYear, 4) AS DATETIME)) + ' - ' + Right(SubQ.MonthYear, 4)  AS Name  "
				" FROM (SELECT LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear,  "
				" PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 as UnionType "
				" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
				" WHERE CurrentStatus = 1 "
				"	%s" 
				" GROUP BY LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))),  "
				"  PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone "
				"  UNION  "
				" SELECT LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear,  "
				" PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 2 AS UnionType "
				" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID    "
				"  WHERE CurrentStatus = 2 "
				"	%s "
				"  GROUP BY LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))), " 
				"  PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone " 
				" ) SubQ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/);
			return strSql;
			}
		break;
		case 528: //No shows and cancellations by date
			{
				CString strSql;
				strSql.Format("SELECT *, Left(SubQ.MonthYear, Len(MonthYear) - 4) AS MonthNum, Right(SubQ.MonthYear, 4) AS YearNum, DateName(Month, CAST(Left(SubQ.MonthYear, Len(MonthYear) - 4)+'/1/'+ Right(SubQ.MonthYear, 4) AS DATETIME)) + ' - ' + Right(SubQ.MonthYear, 4)  AS Name "
					" FROM ( "
					" SELECT  LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) AS MonthYear,  "
					" PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 1 as UnionType  "
					"  FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
					"  WHERE AppointmentsT.Status = 4    "
					"  %s "
					" Group By LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))), "
					" PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AppointmentsT.ID "
					"  UNION ALL SELECT LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) AS MonthYear, "
					" PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, 2 as UnionType    "
					" FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
					" WHERE AppointmentsT.Status <> 4  AND AppointmentsT.ShowState = 3   "
					" %s "
					"  Group By LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))), "
					" PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AppointmentsT.ID "
					" ) SubQ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/);
				return strSql;
			}
		break;
		case 529:  //Inq to Cons by Date
			{
				CString strSql;
				strSql.Format("SELECT *, DateName(Month, CAST(Left(SubQ.MonthYear, Len(MonthYear) - 4)+'/1/'+ Right(SubQ.MonthYear, 4) AS DATETIME)) + ' - ' + Right(SubQ.MonthYear, 4)  AS Name FROM ( "
					"  SELECT ProsWithConsQ.MonthYear, UserDefinedId, First, Middle, Last, Address1, Address2, City, State, Zip, Homephone, Workphone, CellPhone FROM (  "
					"  SELECT PatientsT.PersonID, LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear, " 
					" UserDefinedId, First, Middle, Last, Address1, Address2, City, State, Zip, Homephone, Workphone, CellPhone     "
					"  FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE CurrentStatus = 4  %s  " 
					"  UNION     "
					"  SELECT PatientsT.PersonID, LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear, "
					"  UserDefinedId, First, Middle, Last, Address1, Address2, City, State, Zip, Homephone, Workphone, CellPhone    " 
					"   FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID  INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  " 
					"    WHERE OldStatus = 4  %s ) AS ProsWithConsQ   " 
					"   WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID    " 
					"  	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)   "
					"  AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4  %s )   " 
					"    GROUP BY MonthYear, UserDefinedId, First, Middle, Last, Address1, Address2, City, State, Zip, Homephone, Workphone, CellPhone " 
					"  ) AS SubQ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, strExtraText);
				return strSql;
			}
		break;
		case 530:  //Pros to Cons By Date
			{
				CString strSql;
				strSql.Format("SELECT *, DateName(Month, CAST(Left(SubQ.MonthYear, Len(MonthYear) - 4)+'/1/'+ Right(SubQ.MonthYear, 4) AS DATETIME)) + ' - ' + Right(SubQ.MonthYear, 4)  AS Name FROM (  "
					"  SELECT ProsWithConsQ.MonthYear, UserDefinedId, First, Middle, Last, Address1, Address2, City, State, Zip, Homephone, Workphone, CellPhone FROM (   "
					" 	  SELECT PatientsT.PersonID, LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear, "
					" 	  UserDefinedId, First, Middle, Last, Address1, Address2, City, State, Zip, Homephone, Workphone, CellPhone     " 
					"	 FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE CurrentStatus = 2    "
					"    %s "
					"	 UNION     "
					" 	 SELECT PatientsT.PersonID, LTRIM(STR(DATEPART(MM, PersonT.FirstContactDate))) + LTRIM(STR(DATEPART(YYYY, PersonT.FirstContactDate))) AS MonthYear, "
					" 	UserDefinedId, First, Middle, Last, Address1, Address2, City, State, Zip, Homephone, Workphone, CellPhone   " 
					" 	 FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID  INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
					"	  WHERE OldStatus = 2  "
					"  %s "
					"	) AS ProsWithConsQ   "
					"	 WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID    "
					" 	 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID WHERE (AptTypeT.Category = 1)   "
					"	  AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4 %s )   "
					" GROUP BY MonthYear, UserDefinedId, First, Middle, Last, Address1, Address2, City, State, Zip, Homephone, Workphone, CellPhone ) SubQ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, strExtraText); 
				return strSql;
			}
		break;
		

		case 531:  //Procedures Closed Vs Performed
			/* Version History
				TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
				// (j.jones 2010-09-22 14:50) - PLID 34140 - made it so we ignore ProcInfoPaymentsT records if they are fully refunded/adjusted
			*/
			{
				CString strSql;
				strSql.Format("SELECT *, DateName(Month, CAST(Left(SubQ.MonthYear, Len(MonthYear) - 4)+'/1/'+ Right(SubQ.MonthYear, 4) AS DATETIME)) + ' - ' + Right(SubQ.MonthYear, 4)  AS Name FROM (   "
					"SELECT Count(AppointmentPurposeT.AppointmentID) AS ProcClosed, 0 As ProcsPerformed, LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) AS MonthYear, "
					" 	UserDefinedId, First, Middle, Last, Address1, Address2, City, State, Zip, Homephone, Workphone, CellPhone, 1 AS UnionType     "
					"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
					" 	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
					"	 AppointmentsT.PatientID = PatientsT.PersonID "
					"	 LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID  "
					"	 WHERE (PersonT.ID > 0) AND AppointmentsT.ID IN (SELECT SurgeryApptID FROM ProcInfoT "
					"		INNER JOIN (SELECT * FROM ProcInfoPaymentsT WHERE PayID NOT IN ( "
					"			SELECT LineItemT.ID FROM LineItemT "
					"			INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
					"			INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.DestID "
					"			GROUP BY LineItemT.ID, LineItemT.Amount "
					"			HAVING LineItemT.Amount = Sum(-AppliesT.Amount))) AS ProcInfoPaymentsT ON ProcInfoT.ID = ProcInfoPaymentsT.ProcInfoID)  AND "
					"	 AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4)  "
					"	 %s  %s"
					"	 GROUP BY LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))), "
					"	UserDefinedId, First, Middle, Last, Address1, Address2, City, State, Zip, Homephone, Workphone, CellPhone "
					"	 UNION ALL SELECT 0 as ProcsClosed, Count(AppointmentPurposeT.AppointmentID) As ProcsPerformed, LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) AS MonthYear, "
					"	UserDefinedId, First, Middle, Last, Address1, Address2, City, State, Zip, Homephone, Workphone, CellPhone, 2 As Uniontype    "
					"	 FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
					"	 LEFT JOIN PersonT LEFT JOIN   PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
					"	 AppointmentsT.PatientID = PatientsT.PersonID   "
					"	  LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID  = AppointmentPurposeT.AppointmentID   "
					"	  WHERE (PersonT.ID > 0) AND  "
					"	  AppointmentsT.Status <> 4 AND AppointmentsT.ShowState = 2 AND (AptTypeT.Category = 3 OR AptTypeT.Category = 4)  "
					"		%s %s "
					"	  GROUP BY LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))), "
					"	UserDefinedId, First, Middle, Last, Address1, Address2, City, State, Zip, Homephone, Workphone, CellPhone  "
					"  )SubQ", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, strExtraText, saExtraValues.GetSize()?saExtraValues[0]:"", strExtraText);
				return strSql;
			}
		break;

		case 707:
			{
				/*Version History
					// (j.gruber 2011-05-06 15:21) - PLID 43584 - Appt to Charge Conversion Rate - Created

				*/
				CString str;
				str.Format( " SELECT *, DateName(Month, CAST(Left(SubQ.MonthYear, Len(MonthYear) - 4)+'/1/'+ Right(SubQ.MonthYear, 4) AS DATETIME)) + ' - ' + Right(SubQ.MonthYear, 4)  AS Name FROM (  "
					"  SELECT AppointmentsT.ID as ApptID, PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, "
					"  LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) AS MonthYear,  "
					" 	 PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, PersonT.Email, "
					" 	 AppointmentsT.StartTime, AppointmentsT.EndTime, AppointmentsT.Notes as ApptNotes, dbo.GetPurposeString(AppointmentsT.ID) as Purpose, dbo.GetResourceString(AppointmentsT.ID) as Resources, AptTypeT.Name as AptTypeName, "
					" 	 LineItemT.Date as TDate, ChargeResp.SumAmount, LineItemT.Description as ChargeDesc, ApptServiceConvGroupsT.Name as ConvGroup, 'Converted' as Type, ServiceT.Name as ServiceName,  "
					" 	 CPTCodeT.Code as ServiceCode, CPTCodeT.SubCode as SubCode "
					" 	 FROM AppointmentsT   "
					" 	 LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID   "
					" 	 LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID    "
					" 	 INNER JOIN ApptServiceConvTypesT ON AppointmentsT.AptTypeID = ApptServiceConvTypesT.ApptTypeID  "
					" 	 INNER JOIN ApptServiceConvGroupsT ON ApptServiceConvTypesT.GroupID = ApptServiceConvGroupsT.ID  "
					" 	 INNER JOIN ApptServiceConvServicesT ON ApptServiceConvGroupsT.ID =  ApptServiceConvServicesT.GroupID  "
					" 	 INNER JOIN AptTypeT ON ApptServiceConvTypesT.ApptTypeID = AptTypeT.ID "
					" 	 INNER JOIN ChargesT ON ApptServiceConvServicesT.ServiceID = ChargesT.ServiceID  "
					" 	 LEFT JOIN (SELECT ChargeID, Sum(Amount) as SumAmount FROM ChargeRespT GROUP BY ChargeID) ChargeResp ON ChargeResp.ChargeID = ChargesT.ID "
					" 	 LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
					" 	 LEFT JOIN ServiceT ON ChargeST.ServiceID = ServiceT.ID "
					" 	 LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
					" 	 WHERE AppointmentsT.Status <> 4 and AppointmentsT.ShowState <> 3  "
					"    AND AppointmentsT.PatientID = LineItemT.PatientID "
					" 	 AND (LineItemT.Deleted = 0) "
					" 	 %s %s %s "
					" 	  AND (1 = (CASE WHEN ApptServiceConvGroupsT.ConversionDayLimit = 0 then CASE WHEN DateDiff(day, AppointmentsT.Date, LineItemT.Date) >= 0 THEN 1 ELSE 0 END ELSE CASE WHEN DateDiff(day, AppointmentsT.Date, LineItemT.Date) >= 0 AND DateDiff(day, AppointmentsT.Date, LineItemT.Date) <= ApptServiceConvGroupsT.ConversionDayLimit THEN 1 ELSE 0 END END))	  "
					" UNION  "
					" 	 SELECT AppointmentsT.ID, PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, "
					"  LTRIM(STR(DATEPART(MM, AppointmentsT.Date))) + LTRIM(STR(DATEPART(YYYY, AppointmentsT.Date))) AS MonthYear,  "
					" 	 PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, PersonT.Email, "
					" 	 AppointmentsT.StartTime, AppointmentsT.EndTime, AppointmentsT.Notes as ApptNotes, dbo.GetPurposeString(AppointmentsT.ID) as Purpose, dbo.GetResourceString(AppointmentsT.ID) as Resources, AptTypeT.Name as AptTypeName, "
					" 	 LineItemT.Date as TDate,  0 as SumAmount, LineItemT.Description as ChargeDesc, ApptServiceConvGroupsT.Name as ConvGroup, 'Unconverted' as Type, '' as ServiceName,  "
					" 	 '' as ServiceCode, '' as SubCode "
					" 	 FROM AppointmentsT   "
					" 	 LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID   "
					" 	 LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID    "
					" 	 INNER JOIN ApptServiceConvTypesT ON AppointmentsT.AptTypeID = ApptServiceConvTypesT.ApptTypeID  "
					" 	 INNER JOIN ApptServiceConvGroupsT ON ApptServiceConvTypesT.GroupID = ApptServiceConvGroupsT.ID  "
					" 	 LEFT JOIN ApptServiceConvServicesT ON ApptServiceConvGroupsT.ID =  ApptServiceConvServicesT.GroupID  "
					" 	 INNER JOIN AptTypeT ON ApptServiceConvTypesT.ApptTypeID = AptTypeT.ID "
					" 	 LEFT JOIN (SELECT LineItemT.*, ChargesT.ServiceID FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID) LineItemT ON ApptServiceConvServicesT.ServiceID = LineItemT.ServiceID 	  "					
					"	 AND AppointmentsT.PatientID = LineItemT.PatientID 	   	   "
					" 	 WHERE AppointmentsT.Status <> 4 and AppointmentsT.ShowState <> 3  "					
					" 	 AND AppointmentsT.ID NOT IN  "
					" 		( SELECT AppointmentsT.ID FROM AppointmentsT   "
					" 	 LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID   "
					" 	 LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID    "
					" 	 INNER JOIN ApptServiceConvTypesT ON AppointmentsT.AptTypeID = ApptServiceConvTypesT.ApptTypeID  "
					" 	 INNER JOIN ApptServiceConvGroupsT ON ApptServiceConvTypesT.GroupID = ApptServiceConvGroupsT.ID  "
					" 	 INNER JOIN ApptServiceConvServicesT ON ApptServiceConvGroupsT.ID =  ApptServiceConvServicesT.GroupID  "
					" 	 INNER JOIN AptTypeT ON ApptServiceConvTypesT.ApptTypeID = AptTypeT.ID "
					" 	 INNER JOIN ChargesT ON ApptServiceConvServicesT.ServiceID = ChargesT.ServiceID 	  "
					" 	 LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "					
					" 	 WHERE AppointmentsT.Status <> 4 and AppointmentsT.ShowState <> 3  "
					"    AND AppointmentsT.PatientID = LineItemT.PatientID "
					" 	 AND (LineItemT.Deleted = 0) "
					" 	 %s %s %s "
					" 	  AND (1 = (CASE WHEN ApptServiceConvGroupsT.ConversionDayLimit = 0 then CASE WHEN DateDiff(day, AppointmentsT.Date, LineItemT.Date) >= 0 THEN 1 ELSE 0 END ELSE CASE WHEN DateDiff(day, AppointmentsT.Date, LineItemT.Date) >= 0 AND DateDiff(day, AppointmentsT.Date, LineItemT.Date) <= ApptServiceConvGroupsT.ConversionDayLimit THEN 1 ELSE 0 END END))	 ) "
					" 	 %s %s %s "
					" ) SubQ " ,
				saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, strExtraText,  strDateCaption, 
				saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, strExtraText,  strDateCaption, 
				saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/, strExtraText,  strDateCaption);
				return str;
			}			
			break;
			
		
		case 533:   //No Show By Reason
			{
				CString strSql;
				strSql.Format("SELECT  CASE WHEN AppointmentsT.NoshowReasonID IS NULL THEN -1 ELSE AppointmentsT.NoShowReasonID END AS ReasonID, "
					" CASE WHEN AppointmentsT.NoshowReasonID IS NULL THEN 'Other' ELSE AptNoShowReasonT.Description END AS ReasonName, "
					" UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone "
					"  FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
					" LEFT JOIN AptNoShowReasonT ON AppointmentsT.NoShowReasonID = AptNoShowReasonT.ID "
					" WHERE AppointmentsT.Status <>  4  AND ShowState = 3  "
					" %s"
					" Group By AppointmentsT.NoShowReasonID, AptNoShowReasonT.Description, UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AppointmentsT.ID", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/); 
				return strSql;
			}
		break;
		case 532:   //Cancel by Reason
			{
				CString strSql;
				strSql.Format("SELECT  CASE WHEN AppointmentsT.CancelReasonID IS NULL THEN -1 ELSE AppointmentsT.CanCelReasonID END AS ReasonID, "
					" CASE WHEN AppointmentsT.CAncelReasonID IS NULL THEN 'Other' ELSE AptCAncelReasonT.Description END AS ReasonName, "
					" UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone "
					" FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
					" LEFT JOIN AptCancelReasonT ON AppointmentsT.CancelReasonID = AptCancelReasonT.ID "
					" WHERE AppointmentsT.Status =  4    "
					"  %s "
					" Group By AppointmentsT.CanCelReasonID, AptCancelReasonT.Description, UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AppointmentsT.ID", saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/); 
				return strSql;
			}
		break;
		case 534:   // Performance indices
			{
				// The entire query is calculated in MarketBaselineDlg.cpp
				return saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/;
			}
		break;

			case 535:   //Retained Patients
			case 536:   //Unretained Patients
			{
				return saExtraValues.GetSize()?saExtraValues[0]:""/*Passed in by marketview.cpp*/;
			}
		break;

	case 541:
		{
		//EOB Preview

			return _T(strExtendedSql);
		}
		break;
			
	case 548:
			//NewWEb Import Info
			return "SELECT ID FROM PersonT WHERE 0 = 1";
	break;

	// (z.manning, 09/20/2007) - PLID 27467 - Procedure content (ID = 570) was moved to ReportInfoAdmin.cpp.		

	case 567:	//Lab Results Form
		{
			/*	Version History
				DRT 6/26/2006 - Created.  Kidnapped the "saExtraValues" array to pass in the form text ID.  If you want to print just
					for a certain form, pass in that value.
				DRT 7/24/2006 - PLID 21573 - Added results to the form, suppressed if creating the request form, not otherwise
				// (j.gruber 2007-02-20 09:21) - PLID 24740 - Upping version because reverfiying the report - no change to query
				// (j.jones 2007-07-20 10:30) - PLID 26751 - removed subreports, added DiagnosisDesc and ClinicalDiagnosisDesc
				(e.lally 2007-08-08) PLID 26978 - Patient name uses the name stored on the lab now.
				// (j.gruber 2008-10-16 16:34) - PLID 31433 - added LabResults
				// (z.manning 2008-10-30 14:35) - PLID 31864 - Added type and to be ordered
				// (z.manning 2008-11-25 09:35) - PLID 26794 - Added melanoma history and previous biopsy fields
				// as well as many addition patient demographic and insurance fields. Also added basic provider
				// demographics
				// (z.manning 2008-11-26 17:25) - PLID 32125 - Added code to load the signature file and ink
				// and then included it in a table variable in the main report query.
				//TES 12/3/2008 - PLID 32302 - Accounted for NULL AnatomyIDs.
				//TES 12/8/2008 - PLID 32359 - Added the Result Status to the results subreport.
				//TES 3/12/2009 - PLID 33467 - Added Ins Co Address information, and Ethnicity information, to the main report
				//TES 3/12/2009 - PLID 33468 - Changed name to Lab Results Form, this is now used only for results, not request.
				//TES 4/20/2009 - PLID 33467 - Added Ins Co Address2 information.
				// (z.manning 2009-05-01 09:22) - PLID 28560 - Added result comments
				TES 5/14/2009 - PLID 33792 - Added InitialDiagnosis
				TES 8/3/2009 - PLID 33910 - Added LabProcedure
				// (j.jones 2009-09-11 13:06) - PLID 35503 - added LastApptDate and Instructions
				// (j.jones 2009-09-18 09:11) - PLID 24629 - added ProviderNames field,
				// and added a provider info subreport
				// (j.jones 2009-10-19 17:51) - PLID 35994 - split race & ethnicity
				TES 11/11/2009 - PLID 36260 - Replaced AnatomySide with AnatomyQualifierID
				//(e.lally 2009-11-12) PLID 36273 - Put the general 1 Provider fields back in, renamed the subreport fields for lab providers
					//to LabProvider[field] and LabProviderNames.
				TES 12/8/2009 - PLID 36512 - Restored AnatomySide
				TES 5/14/2010 - PLID 38664 - Added Primary Care Physician and Referring Physician info
				TES 9/7/2010 - PLID 40379 - Added AcknowledgedByUser and AcknowledgedDate to the Results subreport
				(c.haag 2010-09-08 11:35) - PLID 38494 - Added custom fields
				TES 4/25/2011 - PLID 43427 - Added DatePerformed to the results subreport
				TES 4/28/2011 - PLID 43426 - Added DateReceivedByLab to the results subreport
				TES 4/29/2011 - PLID 43484 - Added ClientAccountNumber to the results subreport
				TES 5/2/2011 - PLID 43428 - Added OrderStatus
				//TES 5/5/2011 - PLID 43575 - Added FillerOrderNumber
				TES 5/9/2011 - PLID 41077 - Added ResultValueAsString to the results subreport
				TES 7/11/2011 - PLID 36445 - Added CC fields
				TES 7/12/2011 - PLID 44107 - Added LOINC fields
				TES 8/5/2011 - PLID 44901 - Filter on permissioned locations
				// (d.thompson 2012-08-09) - PLID 52045 - Reworked ethnicity table structure, switched to practice name
				// (r.gonet 01/15/2013) - PLID 34824 - Added the first lab provider to the main report.
				//TES 9/10/2013 - PLID 58511 - Added LabWasReplaced and ResultWasReplaced
			*/
			
			switch (nSubLevel) {

			
				case 0: //Main Report 
				{
					CString strFormNumberTextID;
					// (z.manning 2008-11-26 17:27) - PLID 32125 - Before we run the query for the report,
					// let's gather all the signature images for any labs that will be included in the report.
					CString strImageInserts;
					// (r.gonet 01/15/2013) - PLID 34824 - The first lab provider from LabMultiProviderT for this lab. -1 means no provider, which would be invalid.
					long nPriLabProviderID = -1;
					
					if(saExtraValues.GetSize() > 0)
					{
						strFormNumberTextID = saExtraValues.GetAt(0);
						//(a.wilson 2011-11-22 PLID 43338 - moved this code to a function to prevent code repitition.
						// (c.haag 2015-02-23) - PLID 63751 - Pass in a CNxAdoConnection
						strImageInserts = GenerateRequestSignatureTableQuery(CreateThreadSnapshotConnection(), strFormNumberTextID);

						// (r.gonet 01/15/2013) - PLID 34824 - Get the first ordering provider since we'll be including that on the main report.
						ADODB::_RecordsetPtr prsPriLabProvider = CreateParamRecordset(GetRemoteData(),
							"SELECT LabsT.LocationID, PriLabProviderQ.PriLabProviderID "
							"FROM LabsT "
							"INNER JOIN "
							"( "
								"SELECT LabMultiProviderT.LabID, MIN(LabMultiProviderT.ProviderID) AS PriLabProviderID "
								"FROM LabMultiProviderT "
								"GROUP BY LabMultiProviderT.LabID "
							") PriLabProviderQ ON LabsT.ID = PriLabProviderQ.LabID "
							"WHERE LabsT.FormNumberTextID = {STRING}",
							strFormNumberTextID);
						if(!prsPriLabProvider->eof) {
							nPriLabProviderID = AdoFldLong(prsPriLabProvider->Fields, "PriLabProviderID");
						}
						prsPriLabProvider->Close();
					}
					else {
						// (z.manning 2008-11-26 16:06) - PLID 32125 - If we ever have another way of running this
						// report where the FormNumberTextID isn't passed in we'll need to handle the signature
						// for that as well.
						// (z.manning 2009-02-04 12:06) - PLID 32125 - Bah, we hit this when verifying.  Bummer,
						// but I guess if we ever have another way of running this report that person will just
						// need to make sure this gets handled.
						//ASSERT(FALSE);
					}
						
					// (z.manning 2010-04-30 15:46) - PLID 37553 - Pull anatomic location from view
					// (f.dinatale 2010-10-18) - PLID 40876 - Added SSN Masking.
					// (c.haag 2010-12-14 9:39) - PLID 41806 - Replaced Completed fields with functions
					// (d.lange 2011-01-14 14:35) - PLID 29065 - Added Biopsy Type
					// (a.wilson 2011-9-27) PLID 43338 - Added SignedDate
					// TES 5/2/2011 - PLID 43428 - Added OrderStatus
					// (d.thompson 2012-08-09) - PLID 52045 - Reworked ethnicity table structure, changed to practice name
					// (r.gonet 01/15/2013) - PLID 34824 - Added fields for the first lab provider to the main report.
					// (r.gonet 01/15/2013) - PLID 41799 - Added individual patient name fields.
					// (r.gonet 01/15/2013) - PLID 54642 - Added Responsible Party fields.
					//TES 2/25/2013 - PLID 54876 - Added Performing Lab fields
					//TES 2/26/2013 - PLID 55321 - Added MedicalDirector
					// (j.dinatale 2013-03-26 13:33) - PLID 55871 - @MaskSSN was declared incorrectly here too!
					//TES 5/17/2013 - PLID 56286 - PerformingLabID is now part of LabResultsT instead of LabsT.  I kept the fields here
					// for existing reports, and because certain lab companies (like Quest) treat it as an order-level field.  It 
					// just selects one arbitrarily, if you expect to have multiple performing labs on the same order then you shouldn't 
					// add the fields here, but in the Results subreport
					// (r.goldschmidt 2014-11-14 11:34) - PLID 63982 - Get signed date from labst instead of temp table
					CString str;		
					str.Format(
						"SET NOCOUNT ON \r\n"
						"DECLARE @MaskSSN INT; \r\n"
						"SET @MaskSSN = %s; \r\n"
						"DECLARE @LabImagesT TABLE (LabID int null, SignatureImage image null, SignedDate datetime null) \r\n"
						"%s "
						"SET NOCOUNT OFF \r\n"
						"SELECT LabsT.ID AS LabID, PersonT.ID AS PatID, PatientsT.UserDefinedID, LabsT.PatientLast + ', ' + LabsT.PatientFirst + ' ' + LabsT.PatientMiddle AS PatientName,  "
						"LabsT.PatientFirst, LabsT.PatientMiddle, LabsT.PatientLast, "
						"PersonT.Gender, PersonT.BirthDate, PersonT.Address1, PersonT.Address2, PersonT.City, "
						"PersonT.State, PersonT.Zip, PersonT.Suffix, PersonT.Title, PersonT.HomePhone, PersonT.WorkPhone, "
						"PersonT.Extension, PersonT.CellPhone, PersonT.OtherPhone, PersonT.Email, PersonT.Pager, PersonT.Fax, "
						"dbo.MaskSSN(PersonT.SocialSecurity, @MaskSSN) AS SSN, PersonT.EmergFirst, PersonT.EmergLast, PersonT.EmergHPhone, PersonT.EmergWPhone, "
						"PersonT.EmergRelation, PatientsT.MaritalStatus, PersonT.Company, PatientsT.Occupation, PatientsT.EmployerFirst, "
						"PatientsT.EmployerMiddle, PatientsT.EmployerLast, PatientsT.EmployerAddress1, PatientsT.EmployerAddress2, "
						"PatientsT.EmployerCity, PatientsT.EmployerState, PatientsT.EmployerZip, "
						// (b.spivey, May 28, 2013) - PLID 56871
						"	LEFT(RaceSubQ.RaceName, LEN(RaceSubQ.RaceName) -1) AS Race, "
						"	LEFT(OfficialRaceSubQ.OfficialName, LEN(OfficialRaceSubQ.OfficialName) -1) AS CDCRace, "
						"EthnicityT.Name AS CDCEthnicity, "
						"LabsT.FormNumberTextID, LabsT.BiopsyDate, LabsT.InputDate,  "
						"LabsT.ClinicalData, LabsT.Specimen, "
						"dbo.GetLabCompletedDate(LabsT.ID) AS CompletedDate, UsersT.Username AS CompletedBy,  "
						"LabAnatomicLocationQ.AnatomicLocation AS AnatomyLocation, LabBiopsyTypeT.Description AS BiopsyType, "
						"LabsT.ToBeOrdered, LabsT.Type, LabsT.MelanomaHistory, LabsT.PreviousBiopsy, "
						"LabImages.SignatureImage AS SignatureImage, "
						"LabsT.InitialDiagnosis, "
						"LabsT.CC_Patient, LabsT.CC_RefPhys, LabsT.CC_PCP, "
						"LabsT.LOINC_Code, LabsT.LOINC_Description, "
						" "
						"Provider.First AS ProviderFirst, Provider.Middle AS ProviderMiddle, Provider.Last AS ProviderLast, "
						"Provider.Title AS ProviderTitle, Provider.Address1 AS ProviderAddress1, "
						"Provider.Address2 AS ProviderAddress2, Provider.City AS ProviderCity, Provider.State AS ProviderState, "
						"Provider.Zip AS ProviderZip, Provider.WorkPhone AS ProviderWorkPhone, Provider.Extension AS ProviderExtension, "
						"Provider.HomePhone AS ProviderHomePhone, Provider.CellPhone AS ProviderCellPhone, Provider.SocialSecurity AS ProviderSSN, "
						"Provider.Email AS ProviderEmail, Provider.Fax AS ProviderFax, "
						" "
						"dbo.GetLabProviderString(LabsT.ID) AS LabProviderNames, "
						" "
						"LabProviderPerson.ID AS PriLabProviderID, LabProviderPerson.First AS PriLabProviderFirst, LabProviderPerson.Middle AS PriLabProviderMiddle, LabProviderPerson.Last AS PriLabProviderLast, "
						"LabProviderPerson.Title AS PriLabProviderTitle, LabProviderPerson.Address1 AS PriLabProviderAddress1, "
						"LabProviderPerson.Address2 AS PriLabProviderAddress2, LabProviderPerson.City AS PriLabProviderCity, LabProviderPerson.State AS PriLabProviderState, "
						"LabProviderPerson.Zip AS PriLabProviderZip, LabProviderPerson.WorkPhone AS PriLabProviderWorkPhone, LabProviderPerson.Extension AS PriLabProviderExtension, "
						"LabProviderPerson.HomePhone AS PriLabProviderHomePhone, LabProviderPerson.CellPhone AS PriLabProviderCellPhone, LabProviderPerson.SocialSecurity AS PriLabProviderSSN, "
						"LabProviderPerson.Email AS PriLabProviderEmail, LabProvider.NPI AS PriLabProviderNPI, LabProvider.UPIN AS PriLabProviderUPIN, "
						" "
						"PracLocT.Name AS PracLocName, PracLocT.Address1 AS PracLocAddress1,  "
						"PracLocT.Address2 AS PracLocAddress2, PracLocT.City AS PracLocCity, PracLocT.State AS PracLocState, PracLocT.Zip AS PracLocZip,  "
						"PracLocT.Phone AS PracLocMainPhone, PracLocT.Phone2 AS PracLocOtherPhone, PracLocT.Fax AS PracLocFax,  "
						"PracLocT.OnLineAddress AS PracLocEmail, "
						" "
						"LabLocT.Name AS LabLocName, LabLocT.Address1 AS LabLocAddress1,  "
						"LabLocT.Address2 AS LabLocAddress2, LabLocT.City AS LabLocCity, LabLocT.State AS LabLocState, LabLocT.Zip AS LabLocZip,  "
						"LabLocT.Phone AS LabLocMainPhone, LabLocT.Phone2 AS LabLocOtherPhone, LabLocT.Fax AS LabLocFax,  "
						"LabLocT.OnLineAddress AS LabLocEmail, "
						" "
						" "
						"PerformingLabLocT.Name AS PerformingLabLocName, PerformingLabLocT.Address1 AS PerformingLabLocAddress1,  "
						"PerformingLabLocT.Address2 AS PerformingLabLocAddress2, PerformingLabLocT.City AS PerformingLabLocCity, "
						"PerformingLabLocT.State AS PerformingLabLocState, PerformingLabLocT.Zip AS PerformingLabLocZip,  "
						"PerformingLabLocT.Phone AS PerformingLabLocMainPhone, PerformingLabLocT.Phone2 AS PerformingLabLocOtherPhone, "
						"PerformingLabLocT.Fax AS PerformingLabLocFax,  PerformingLabLocT.OnLineAddress AS PerformingLabLocEmail, "
						"LabsT.MedicalDirector, "
						" "
						"LabsT.InsuredPartyID, InsuranceCoT.Name AS InsuranceCoName, "
						"InsuredPartyT.IDForInsurance AS PolicyID, InsuredPartyT.PolicyGroupNum AS GroupNum, "
						"InsurancePlansT.PlanName AS PlanName, "
						"InsuredPartyT.RelationToPatient, "
						"InsuredPerson.First AS InsuredFirstName, InsuredPerson.Middle AS InsuredMiddleName, InsuredPerson.Last AS InsuredLastName, "
						"InsuredPerson.Last + ', ' + InsuredPerson.First + ' ' + InsuredPerson.Middle AS InsuredFullName, "
						"InsuredPerson.Address1 AS InsuredAddress1, InsuredPerson.Address2 AS InsuredAddress2, InsuredPerson.City AS InsuredCity, "
						"InsuredPerson.State AS InsuredState, InsuredPerson.Zip AS InsuredZip, InsuredPerson.HomePhone AS InsuredHomePhone, "
						"InsuredPerson.WorkPhone AS InsuredWorkPhone, InsuredPerson.BirthDate AS InsuredBirthDate, "
						"dbo.MaskSSN(InsuredPerson.SocialSecurity, @MaskSSN) AS InsuredSSN, "
						" "
						"PriInsCo.Name AS PriInsCoName, "
						"PriInsCoPerson.Address1 AS PriInsCoAddress1, PriInsCoPerson.Address2 AS PriInsCoAddress2, PriInsCoPerson.City AS PriInsCoCity, "
						"PriInsCoPerson.State AS PriInsCoState, PriInsCoPerson.Zip AS PriInsCoZip, "
						"PriInsParty.IDForInsurance AS PriPolicyID, PriInsParty.PolicyGroupNum AS PriGroupNum, "
						"PriInsPlan.PlanName AS PriPlanName, PriInsParty.RelationToPatient AS PriRelationToPatient, "
						"PriInsPerson.First AS PriInsuredFirstName, PriInsPerson.Middle AS PriInsuredMiddleName, PriInsPerson.Last AS PriInsuredLastName, "
						"PriInsPerson.Last + ', ' + PriInsPerson.First + ' ' + PriInsPerson.Middle AS PriInsuredFullName, "
						"PriInsPerson.Address1 AS PriInsuredAddress1, PriInsPerson.Address2 AS PriInsuredAddress2, PriInsPerson.City AS PriInsuredCity, "
						"PriInsPerson.State AS PriInsuredState, PriInsPerson.Zip AS PriInsuredZip, PriInsPerson.HomePhone AS PriInsuredHomePhone, "
						"PriInsPerson.WorkPhone AS PriInsuredWorkPhone, PriInsPerson.BirthDate AS PriInsuredBirthDate, "
						"dbo.MaskSSN(PriInsPerson.SocialSecurity, @MaskSSN) AS PriInsuredSSN, "
						" "
						"SecInsCo.Name AS SecInsCoName, "
						"SecInsCoPerson.Address1 AS SecInsCoAddress1, SecInsCoPerson.Address2 AS SecInsCoAddress2, SecInsCoPerson.City AS SecInsCoCity, "
						"SecInsCoPerson.State AS SecInsCoState, SecInsCoPerson.Zip AS SecInsCoZip, "
						"SecInsParty.IDForInsurance AS SecPolicyID, SecInsParty.PolicyGroupNum AS SecGroupNum, "
						"SecInsPlan.PlanName AS SecPlanName, SecInsParty.RelationToPatient AS SecRelationToPatient, "
						"SecInsPerson.First AS SecInsuredFirstName, SecInsPerson.Middle AS SecInsuredMiddleName, SecInsPerson.Last AS SecInsuredLastName, "
						"SecInsPerson.Last + ', ' + SecInsPerson.First + ' ' + SecInsPerson.Middle AS SecInsuredFullName, "
						"SecInsPerson.Address1 AS SecInsuredAddress1, SecInsPerson.Address2 AS SecInsuredAddress2, SecInsPerson.City AS SecInsuredCity, "
						"SecInsPerson.State AS SecInsuredState, SecInsPerson.Zip AS SecInsuredZip, SecInsPerson.HomePhone AS SecInsuredHomePhone, "
						"SecInsPerson.WorkPhone AS SecInsuredWorkPhone, SecInsPerson.BirthDate AS SecInsuredBirthDate, "
						"dbo.MaskSSN(SecInsPerson.SocialSecurity, @MaskSSN) AS SecInsuredSSN, "
						" "
						"TerInsCo.Name AS TerInsCoName, "
						"TerInsCoPerson.Address1 AS TerInsCoAddress1, TerInsCoPerson.Address2 AS TerInsCoAddress2, TerInsCoPerson.City AS TerInsCoCity, "
						"TerInsCoPerson.State AS TerInsCoState, TerInsCoPerson.Zip AS TerInsCoZip, "
						"TerInsParty.IDForInsurance AS TerPolicyID, TerInsParty.PolicyGroupNum AS TerGroupNum, "
						"TerInsPlan.PlanName AS TerPlanName, TerInsParty.RelationToPatient AS TerRelationToPatient, "
						"TerInsPerson.First AS TerInsuredFirstName, TerInsPerson.Middle AS TerInsuredMiddleName, TerInsPerson.Last AS TerInsuredLastName, "
						"TerInsPerson.Last + ', ' + TerInsPerson.First + ' ' + TerInsPerson.Middle AS TerInsuredFullName, "
						"TerInsPerson.Address1 AS TerInsuredAddress1, TerInsPerson.Address2 AS TerInsuredAddress2, TerInsPerson.City AS TerInsuredCity, "
						"TerInsPerson.State AS TerInsuredState, TerInsPerson.Zip AS TerInsuredZip, TerInsPerson.HomePhone AS TerInsuredHomePhone, "
						"TerInsPerson.WorkPhone AS TerInsuredWorkPhone, TerInsPerson.BirthDate AS TerInsuredBirthDate, "
						"dbo.MaskSSN(TerInsPerson.SocialSecurity, @MaskSSN) AS TerInsuredSSN, "
						" "
						"ResponsiblePartyT.RelationToPatient AS PriRespPartyRelationToPatient, "
						"RespPartyPerson.First AS PriRespPartyFirstName, RespPartyPerson.Middle AS PriRespPartyMiddleName, RespPartyPerson.Last AS PriRespPartyLastName, "
						"RespPartyPerson.Last + ', ' + RespPartyPerson.First + ' ' + RespPartyPerson.Middle AS PriRespPartyFullName, "
						"RespPartyPerson.Address1 AS PriRespPartyddress1, RespPartyPerson.Address2 AS PriRespPartyAddress2, RespPartyPerson.City AS PriRespPartyCity, "
						"RespPartyPerson.State AS PriRespPartyState, RespPartyPerson.Zip AS PriRespPartyZip, RespPartyPerson.HomePhone AS PriRespPartyPhone, "
						"RespPartyPerson.BirthDate AS PriRespPartyBirthDate, "
						"dbo.MaskSSN(RespPartyPerson.SocialSecurity, @MaskSSN) AS PriRespPartySSN, ResponsiblePartyT.Employer AS PriRespPartyEmployer, "
						" "
						"PCPPerson.First AS PCPFirst, PCPPerson.Middle AS PCPMiddle, PCPPerson.Last AS PCPLast, "
						"PCPPerson.Title AS PCPTitle, PCPPerson.Address1 AS PCPAddress1, "
						"PCPPerson.Address2 AS PCPAddress2, PCPPerson.City AS PCPCity, PCPPerson.State AS PCPState, "
						"PCPPerson.Zip AS PCPZip, PCPPerson.WorkPhone AS PCPWorkPhone, PCPPerson.Extension AS PCPExtension, "
						"PCPPerson.HomePhone AS PCPHomePhone, PCPPerson.CellPhone AS PCPCellPhone, PCPPerson.SocialSecurity AS PCPSSN, "
						"PCPPerson.Email AS PCPEmail, PCPPerson.Fax AS PCPFax, "
						" "
						"RefPhysPerson.First AS RefPhysFirst, RefPhysPerson.Middle AS RefPhysMiddle, RefPhysPerson.Last AS RefPhysLast, "
						"RefPhysPerson.Title AS RefPhysTitle, RefPhysPerson.Address1 AS RefPhysAddress1, "
						"RefPhysPerson.Address2 AS RefPhysAddress2, RefPhysPerson.City AS RefPhysCity, RefPhysPerson.State AS RefPhysState, "
						"RefPhysPerson.Zip AS RefPhysZip, RefPhysPerson.WorkPhone AS RefPhysWorkPhone, RefPhysPerson.Extension AS RefPhysExtension, "
						"RefPhysPerson.HomePhone AS RefPhysHomePhone, RefPhysPerson.CellPhone AS RefPhysCellPhone, RefPhysPerson.SocialSecurity AS RefPhysSSN, "
						"RefPhysPerson.Email AS RefPhysEmail, RefPhysPerson.Fax AS RefPhysFax, "
						" "
						"LabProceduresT.Name AS LabProcedure, "
						"(SELECT Max(AppointmentsT.Date) FROM AppointmentsT WHERE AppointmentsT.PatientID = PersonT.ID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4 AND dbo.AsDateNoTime(AppointmentsT.Date) <= dbo.AsDateNoTime(LabsT.InputDate)) AS LastApptDate, "
						"LabsT.Instructions, "
						" "
						"(SELECT TextParam FROM CustomFieldDataT WHERE CustomFieldDataT.PersonID = LabsT.PatientID AND CustomFieldDataT.FieldID = 1) AS Custom1, "
						"(SELECT TextParam FROM CustomFieldDataT WHERE CustomFieldDataT.PersonID = LabsT.PatientID AND CustomFieldDataT.FieldID = 2) AS Custom2, "
						"(SELECT TextParam FROM CustomFieldDataT WHERE CustomFieldDataT.PersonID = LabsT.PatientID AND CustomFieldDataT.FieldID = 3) AS Custom3, "
						"(SELECT TextParam FROM CustomFieldDataT WHERE CustomFieldDataT.PersonID = LabsT.PatientID AND CustomFieldDataT.FieldID = 4) AS Custom4, "
						"LabOrderStatusT.Description AS OrderStatus, LabsT.FillerOrderNumber, "
						" "
						"LabsT.SignedDate AS DateSigned, "
						"convert(bit, CASE WHEN LinkedLabsQ.LinkedLabID Is Null THEN 0 ELSE 1 END) AS LabWasReplaced "
						" "
						"FROM LabsT "
						"INNER JOIN PersonT ON LabsT.PatientID = PersonT.ID "
						"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
						"LEFT JOIN PersonT Provider ON PatientsT.MainPhysician = Provider.ID " 
						// (b.spivey, May 28, 2013) - PLID 56871 - creates the list of CDC and custom races. 
						"	CROSS APPLY "
						"	( "
						"		SELECT ( "
						"			SELECT RT.Name + ', ' "
						"			FROM PersonRaceT PRT "
						"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID "
						"			WHERE PRT.PersonID = PersonT.ID "
						"			FOR XML PATH(''), TYPE "
						"		).value('/', 'nvarchar(max)') "
						"	) RaceSubQ (RaceName) "
						"	CROSS APPLY "
						"	( "
						"		SELECT ( "
						"			SELECT RCT.OfficialRaceName + ', ' "
						"			FROM PersonRaceT PRT "
						"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID  "
						"			INNER JOIN RaceCodesT RCT ON RCT.ID = RT.RaceCodeID "
						"			WHERE PRT.PersonID = PersonT.ID  "
						"			FOR XML PATH(''), TYPE  "
						"		).value('/', 'nvarchar(max)') "
						"	) OfficialRaceSubQ (OfficialName) "
						"LEFT JOIN EthnicityT ON PersonT.Ethnicity = EthnicityT.ID "
						"LEFT JOIN EthnicityCodesT ON EthnicityT.EthnicityCodeID = EthnicityCodesT.ID "
						"LEFT JOIN LocationsT PracLocT ON LabsT.LocationID = PracLocT.ID "
						"LEFT JOIN LocationsT LabLocT ON LabsT.LabLocationID = LabLocT.ID "
						"LEFT JOIN LabAnatomyT ON LabsT.AnatomyID = LabAnatomyT.ID "
						"LEFT JOIN AnatomyQualifiersT ON LabsT.AnatomyQualifierID = AnatomyQualifiersT.ID "
						"LEFT JOIN UsersT ON dbo.GetLabCompletedBy(LabsT.ID) = UsersT.PersonID "
						"LEFT JOIN InsuredPartyT ON LabsT.InsuredPartyID = InsuredPartyT.PersonID "
						"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
						"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
						"LEFT JOIN PersonT InsuredPerson ON LabsT.InsuredPartyID = InsuredPerson.ID "
						"LEFT JOIN @LabImagesT LabImages ON LabsT.ID = LabImages.LabID "
						"LEFT JOIN LabAnatomicLocationQ ON LabsT.ID = LabAnatomicLocationQ.LabID "
						"\r\n"
						"LEFT JOIN InsuredPartyT PriInsParty ON PersonT.ID = PriInsParty.PatientID \r\n"
						"	AND PriInsParty.RespTypeID = (SELECT ID FROM RespTypeT WHERE Priority = 1) \r\n"
						"LEFT JOIN InsuranceCoT PriInsCo ON PriInsParty.InsuranceCoID = PriInsCo.PersonID \r\n"
						"LEFT JOIN PersonT PriInsCoPerson ON PriInsCo.PersonID = PriInsCoPerson.ID \r\n"
						"LEFT JOIN InsurancePlansT PriInsPlan ON PriInsParty.InsPlan = PriInsPlan.ID \r\n"
						"LEFT JOIN PersonT PriInsPerson ON PriInsParty.PersonID = PriInsPerson.ID \r\n"
						"\r\n"
						"LEFT JOIN InsuredPartyT SecInsParty ON PersonT.ID = SecInsParty.PatientID \r\n"
						"	AND SecInsParty.RespTypeID = (SELECT ID FROM RespTypeT WHERE Priority = 2) \r\n"
						"LEFT JOIN InsuranceCoT SecInsCo ON SecInsParty.InsuranceCoID = SecInsCo.PersonID \r\n"
						"LEFT JOIN PersonT SecInsCoPerson ON SecInsCo.PersonID = SecInsCoPerson.ID \r\n"
						"LEFT JOIN InsurancePlansT SecInsPlan ON SecInsParty.InsPlan = SecInsPlan.ID \r\n"
						"LEFT JOIN PersonT SecInsPerson ON SecInsParty.PersonID = SecInsPerson.ID \r\n"
						"\r\n"
						"LEFT JOIN InsuredPartyT TerInsParty ON PersonT.ID = TerInsParty.PatientID \r\n"
						"	AND TerInsParty.RespTypeID = (SELECT ID FROM RespTypeT WHERE Priority = 3) \r\n"
						"LEFT JOIN InsuranceCoT TerInsCo ON TerInsParty.InsuranceCoID = TerInsCo.PersonID \r\n"
						"LEFT JOIN PersonT TerInsCoPerson ON TerInsCo.PersonID = TerInsCoPerson.ID \r\n"
						"LEFT JOIN InsurancePlansT TerInsPlan ON TerInsParty.InsPlan = TerInsPlan.ID \r\n"
						"LEFT JOIN PersonT TerInsPerson ON TerInsParty.PersonID = TerInsPerson.ID \r\n"
						"\r\n"
						"LEFT JOIN ResponsiblePartyT ON ResponsiblePartyT.PersonID = PatientsT.PrimaryRespPartyID \r\n"
						"LEFT JOIN PersonT RespPartyPerson ON ResponsiblePartyT.PersonID = RespPartyPerson.ID \r\n"
						"\r\n"
						"LEFT JOIN PersonT PCPPerson ON PatientsT.PCP = PCPPerson.ID "
						"LEFT JOIN PersonT RefPhysPerson ON PatientsT.DefaultReferringPhyID = RefPhysPerson.ID "
						"\r\n"
						"LEFT JOIN LabProceduresT ON LabsT.LabProcedureID = LabProceduresT.ID \r\n"
						"LEFT JOIN LabBiopsyTypeT ON LabsT.BiopsyTypeID = LabBiopsyTypeT.ID \r\n"
						"LEFT JOIN LabOrderStatusT ON LabsT.OrderStatusID = LabOrderStatusT.ID \r\n"
						"LEFT JOIN (SELECT LabID, Max(PerformingLabID) AS PerformingLabID FROM LabResultsT WHERE Deleted = 0 GROUP BY LabID) AS PerformingLabQ \r\n"
						"	ON LabsT.ID = PerformingLabQ.LabID \r\n"
						"LEFT JOIN LocationsT PerformingLabLocT ON PerformingLabQ.PerformingLabID = PerformingLabLocT.ID \r\n"
						"LEFT JOIN ProvidersT LabProvider ON LabProvider.PersonID = %li \r\n"
						"LEFT JOIN PersonT LabProviderPerson ON LabProvider.PersonID = LabProviderPerson.ID \r\n"
						"LEFT JOIN (SELECT LinkedLabID FROM LabResultsT WHERE Deleted = 0 GROUP BY LinkedLabID) LinkedLabsQ ON LabsT.ID = LinkedLabsQ.LinkedLabID "
						"WHERE LabsT.Deleted = 0 AND %s" 
						, ((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"), 
						strImageInserts, 
						nPriLabProviderID,
						GetAllowedLocationClause("LabsT.LocationID"));
						if(saExtraValues.GetSize() > 0) {
							//We only accept 1 parameter.
							str += "AND LabsT.FormNumberTextID = '" + saExtraValues[0] + "'";
						}

						return _T(str);
					
					break;
				}
				case 1:  
				{
					

					switch (nSubRepNum) {
						case 1: //results 
						{
							//(a.wilson 2011-11-22) PLID 43338 - adding the signature image and date signed to the results subreport.
							CString strImageInserts;
							if(saExtraValues.GetSize() > 0)
							{
								//(a.wilson 2011-11-22 PLID 43338 - moved this code to a function to prevent code repitition.
								strImageInserts = GenerateResultSignatureTableQuery(CreateThreadSnapshotConnection(), saExtraValues.GetAt(0));
							} else {	
								//ASSERT(FALSE);
							}
							// (z.manning 2009-05-01 09:25) - PLID 28560 - Added result comments
							// (c.haag 2009-06-22 11:12) - PLID 34189 - Added result units
							//TES 4/25/2011 - PLID 43427 - Added DatePerformed
							//TES 4/28/2011 - PLID 43426 - Added DateReceivedByLab
							//TES 4/29/2011 - PLID 43484 - Added ClientAccountNumber
							//TES 5/9/2011 - PLID 41077 - Added ResultValueAsString
							//TES 5/17/2013 - PLID 56286 - Added PerformingLab fields
							//TES 11/5/2013 - PLID 59320 - Updated the ResultWasReplaced calculation, it now includes results that were replaced as part of a lab being replaced
							// (r.goldschmidt 2014-11-14 11:39) - PLID 63982 - Get Result Signed Date from LabResultsT instead of temp table
							CString str;
							str.Format(
							"SET NOCOUNT ON \r\n"
							"DECLARE @LabResultImagesT TABLE (LabID int null, SignatureImage image null, SignedDate datetime null) \r\n"
							"%s "
							"SELECT LabsT.ID AS LabID, PersonT.ID AS PatID, "
							"LabsT.FormNumberTextID, LabResultsT.SlideTextID, "
							"LabResultsT.DiagnosisDesc, LabResultsT.ClinicalDiagnosisDesc, "
							"LabResultsT.DateReceived, LabResultFlagsT.Name AS FlagName, "
							"   LabResultsT.Name as ResultName, "
							"   LabResultsT.DateReceived as ResultReceived, "
							"   LabResultsT.Value as ResultValue, "
							"	LabResultsT.Units as ResultUnits, "
							"   LabResultsT.Reference as ResultReference, "
							"	LabResultStatusT.Description AS ResultStatus, "
							"	LEFT(LabResultsT.Comments, 255) AS ResultComments, "
							"	LabResultsT.Comments AS FullResultComments, "
							"	AcknowledgeUser.UserName AS AcknowledgedByUser, "
							"	LabResultsT.AcknowledgedDate, "
							"	LabResultsT.DatePerformed, "
							"	LabResultsT.DateReceivedByLab, "
							"	LabResultsT.ClientAccountNumber, "
							"	convert(nvarchar(255), LabResultsT.Value) AS ResultValueAsString, "
							"	LabResultsT.ResultSignedDate AS ResultDateSigned, "
							"	LabResultImages.SignatureImage AS ResultSignatureImage, "
							"	PerformingLabLocT.Name AS PerformingLabLocName, PerformingLabLocT.Address1 AS PerformingLabLocAddress1,  "
							"	PerformingLabLocT.Address2 AS PerformingLabLocAddress2, PerformingLabLocT.City AS PerformingLabLocCity, "
							"	PerformingLabLocT.State AS PerformingLabLocState, PerformingLabLocT.Zip AS PerformingLabLocZip,  "
							"	PerformingLabLocT.Phone AS PerformingLabLocMainPhone, PerformingLabLocT.Phone2 AS PerformingLabLocOtherPhone, "
							"	PerformingLabLocT.Fax AS PerformingLabLocFax,  PerformingLabLocT.OnLineAddress AS PerformingLabLocEmail, "							
							"convert(bit, CASE WHEN LinkedResultsQ.LinkedResultID Is Null THEN CASE WHEN LabResultsT.HL7MessageID NOT IN (SELECT HL7MessageID FROM LabResultsT WHERE LinkedLabID Is Not Null) AND LinkedLabsQ.LinkedLabID Is Not Null THEN 1 ELSE 0 END ELSE 1 END) AS ResultWasReplaced "
							"FROM LabsT "
							"LEFT JOIN (SELECT LinkedLabID FROM LabResultsT WHERE Deleted = 0 GROUP BY LinkedLabID) LinkedLabsQ ON LabsT.ID = LinkedLabsQ.LinkedLabID "
							" LEFT JOIN LabResultsT ON LabsT.ID = LabResultsT.LabID "
							"INNER JOIN PersonT ON LabsT.PatientID = PersonT.ID "
							"LEFT JOIN LabResultFlagsT ON LabResultsT.FlagID = LabResultFlagsT.ID "
							"LEFT JOIN LabResultStatusT ON LabResultsT.StatusID = LabResultStatusT.ID "
							"LEFT JOIN UsersT AcknowledgeUser ON LabResultsT.AcknowledgedUserID = AcknowledgeUser.PersonID "
							"LEFT JOIN @LabResultImagesT LabResultImages ON LabResultsT.ResultID = LabResultImages.LabID "
							"LEFT JOIN LocationsT PerformingLabLocT ON LabResultsT.PerformingLabID = PerformingLabLocT.ID "
							"LEFT JOIN (SELECT LinkedResultID FROM LabResultsT WHERE Deleted = 0 GROUP BY LinkedResultID) LinkedResultsQ ON LabResultsT.ResultID = LinkedResultsQ.LinkedResultID "
							" "
							"WHERE LabsT.Deleted = 0 AND LabResultsT.Deleted = 0 AND %s", strImageInserts, GetAllowedLocationClause("LabsT.LocationID"));
							if(saExtraValues.GetSize() > 0) {
								//We only accept 1 parameter.
								str += "AND LabsT.FormNumberTextID = '" + saExtraValues[0] + "'";
							}

							return str;
							break;
						}

						case 0: //providers 
						{
							CString str;
							str.Format("SELECT LabsT.ID AS LabID, LabsT.PatientID AS PatID, "
							"Provider.First AS LabProviderFirst, Provider.Middle AS LabProviderMiddle, Provider.Last AS LabProviderLast, "
							"Provider.Title AS LabProviderTitle, Provider.Address1 AS LabProviderAddress1, "
							"Provider.Address2 AS LabProviderAddress2, Provider.City AS LabProviderCity, Provider.State AS LabProviderState, "
							"Provider.Zip AS LabProviderZip, Provider.WorkPhone AS LabProviderWorkPhone, Provider.Extension AS LabProviderExtension, "
							"Provider.HomePhone AS LabProviderHomePhone, Provider.CellPhone AS LabProviderCellPhone, Provider.SocialSecurity AS LabProviderSSN, "
							"Provider.Email AS LabProviderEmail "
							"FROM LabsT "
							"INNER JOIN LabMultiProviderT ON LabsT.ID = LabMultiProviderT.LabID "
							"INNER JOIN PersonT Provider ON LabMultiProviderT.ProviderID = Provider.ID "
							"WHERE LabsT.Deleted = 0 AND %s", GetAllowedLocationClause("LabsT.LocationID"));
							if(saExtraValues.GetSize() > 0) {
								//We only accept 1 parameter.
								str += " AND LabsT.FormNumberTextID = '" + saExtraValues[0] + "'";
							}

							return str;
							break;
						}

						default: 
							return "";
						break;
					}	
				}

				default:
					return "";
				break;
			}
					
		}
		break;

	case 658:	//Lab Request Form
		/*	Version History
			TES 3/12/2009 - PLID 33468 - Created.  Copied from the Lab Results Form (the two used to be the same report),
			and just took out the sub report
			TES 4/20/2009 - PLID 33467 - Added insurance company Address2 fields.
			TES 5/14/2009 - PLID 33792 - Added InitialDiagnosis
			TES 8/3/2009 - PLID 33910 - Added LabProcedure
			// (j.jones 2009-09-11 13:06) - PLID 35503 - added LastApptDate and Instructions
			// (j.jones 2009-09-18 09:11) - PLID 24629 - added ProviderNames field,
			// and added a provider info subreport
			// (j.jones 2009-10-19 17:51) - PLID 35994 - split race & ethnicity
			TES 11/11/2009 - PLID 36260 - Replaced AnatomySide with AnatomyQualifierID
			//(e.lally 2009-11-12) PLID 36273 - Put the general 1 provider fields back in and renamed the subreport fields for lab providers
			//to LabProvider[field].
			TES 12/8/2009 - PLID 36512 - Restored AnatomySide
			TES 5/13/2010 - PLID 38649 - Added individual fields for AnatomicLocation, AnatomicQualifier, and AnatomicSide
			TES 5/14/2010 - PLID 38664 - Added Primary Care Physician and Referring Physician info
			(c.haag 2010-09-08 11:35) - PLID 38494 - Added custom fields
			TES 5/2/2011 - PLID 43428 - Added OrderStatus
			TES 6/15/2011 - PLID 41922 - Added ProviderUPIN, ProviderNPI, PCPUPIN, PCPNPI, RefPhysUPIN, and RefPhysNPI to the main report,
			and LabProviderUPIN and LabProviderNPI to the providers subreport
			TES 7/11/2011 - PLID 36445 - Added CC fields
			TES 7/12/2011 - PLID 44107 - Added LOINC fields
			TES 8/5/2011 - PLID 44901 - Filter on permissioned locations
			(r.gonet 10/11/2011) - PLID 46437 - Added Barcode1, Barcode1Length, Barcode2, Barcode2Length
			(r.gonet 10/19/2011) - PLID 46039 - Added HL7 third party insurance codes.
			// (d.thompson 2012-08-09) - PLID 52045 - Reworked ethnicity table structure, changed to practice name
			// (r.gonet 01/15/2013) - PLID 41864 - Added HL7 Sending/Receiving Application/Facility Codes.
			(c.haag 2015-02-23) - PLID 63751 - Moved to SharedReportInfo.cpp
			*/
		return _T(__super::GetSqlPrintPreview(CreateThreadSnapshotConnection(), (LPCTSTR)GetAPILoginToken(), nSubLevel, nSubRepNum));

	case 573: // Marketing effectiveness (a.walling 2006-08-09 15:35) - PLID 3897
		{
			CString strFields = "ChargesAndAdjustmentsQ.PatientID AS PatID, ChargesAndAdjustmentsQ.FullName AS FullName, ChargesAndAdjustmentsQ.LastProcedureDate AS Date, ChargesAndAdjustmentsQ.Description AS Description, ChargesAndAdjustmentsQ.Charges AS TotalCharges";
			CString strMainSql = GetExtraValue();

			if (strMainSql.IsEmpty()) {
				strMainSql = "(SELECT 0 AS PatientID, 'FullName' AS FullName, CAST('1/1/1999' AS datetime) AS LastProcedureDate, 'Description' AS Description, CAST(0 AS money) AS Charges) ChargesAndAdjustmentsQ";
			}

			CString strSql;
			strSql.Format("SELECT %s FROM %s ORDER BY FullName", strFields, strMainSql);


			return _T(strSql);
			break;
		}

	case 574: //Quote List (PP) // (a.walling 2006-08-10 14:59) - PLID 3897
		// (j.gruber 2009-03-25 15:29) - PLID 33696 - updated discount structure
		{
			CString strPerson, strSql;
			strPerson.Format("AND PersonT.ID = %li ", nPatient);

			strSql = " SELECT"
				" BillsT.ID BillID," 
				" ChargesT.ID ChargeID," 
				" PatientsT.PersonID AS PatID," 
				" PatientsT.UserDefinedID," 
				" BillsT.Date AS TDate," 
				" BillsT.InputDate AS IDate," 
				" BillsT.Description AS Description," 
				" (CASE WHEN PackagesT.Type Is Null THEN 'Quotes' WHEN PackagesT.Type = 1 THEN 'Repeatable Packages' WHEN PackagesT.Type = 2 THEN 'Multi-Use Packages' END) AS PackageType," 
				" PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName," 
				" UsersT.PersonID AS PatCoordID," 
				" CASE WHEN (SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) IS NULL THEN 0 ELSE (SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END AS PercentOff," 
				" LineItemT.Description AS ChargeDesc," 
				" ChargesT.ItemCode," 
				" CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) IS NULL THEN Convert(money, 0) ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END as Discount," 
				" ChargesT.OthrBillFee," 
				" ChargesT.TaxRate," 
				" Sum(Round(Convert(money,(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100.0) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1))),2)) AS Total," 
				" LineItemT.LocationID AS LocID, " 
				" BillsT.PatCoord AS CoordID, " 
				" ChargesT.DoctorsProviders AS ProvID,   " 
				" (CASE WHEN PackagesT.Type = 2 THEN ChargesT.Quantity ELSE Null END) AS PackageChargeQty," 
				" (CASE WHEN PackagesT.Type = 1 THEN PackagesT.TotalCount ELSE Null END) AS PackageQty," 
				" COALESCE (COUNT(BilledQuotesQ.BilledQuoteID), 0) AS BilledQuoteNum" 

				" FROM " 
				" BillsT" 
				" LEFT OUTER JOIN ChargesT ON BillsT.ID = ChargesT.BillID" 
				" INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID" 
				" INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID" 
				" INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID" 
				" LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
				" LEFT OUTER JOIN PackagesT ON BillsT.ID = PackagesT.QuoteID" 
				" LEFT OUTER JOIN LocationsT ON BillsT.Location = LocationsT.ID" 
				" LEFT OUTER JOIN UsersT ON BillsT.PatCoord = UsersT.PersonID" 
				" LEFT JOIN (SELECT BilledQuotesT.BillID As BilledQuoteID, BilledQuotesT.QuoteID As QuoteID FROM BilledQuotesT WHERE BilledQuotesT.BillID IN (SELECT ID FROM BillsT WHERE EntryType = 1 AND Deleted = 0)) BilledQuotesQ ON BilledQuotesQ.QuoteID = BillsT.ID" 

				" WHERE" 
				" BillsT.EntryType = 2" 
				" AND BillsT.Deleted = 0" 
				" AND BillsT.Active = 1" 
				" AND LineItemT.Deleted = 0" 
				" {PersonFilter}"

				" GROUP BY BillsT.ID, ChargesT.ID, PatientsT.PersonID, PatientsT.UserDefinedID, BillsT.Date, BillsT.InputDate, BillsT.Description, PersonT.Location, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, UsersT.PersonID, LineItemT.Description, Chargest.ItemCode, ChargesT.OthrBillFee, ChargesT.DoctorsProviders, Chargest.TaxRate, LineItemT.LocationID, BillsT.PatCoord, PackagesT.TotalCount, PackagesT.Type, ChargesT.Quantity";

			strSql.Replace("{PersonFilter}", strPerson);

			return _T(strSql);
		}
		case 576: //EMR Summary (PP) // (a.walling 2006-08-10 14:59) - PLID 3897
			/* Version History */
			// (j.gruber 2007-01-04 11:14) - PLID 24035 - support multiple providers
			// (j.gruber 2007-01-08 15:52) - PLID 24161 - add secondary provider names
			// (j.gruber 2007-02-26 17:44) - PLID 24950 - fixed alias in order so this complies with the rest of the reports
			// (c.haag 2008-06-17 18:25) - PLID 30319 - Changed name calculation to factor in EMR text macros
			// (z.manning 2011-05-23 09:51) - PLID 33114 - Handle charting permissions
			// (j.jones 2011-07-05 11:46) - PLID 43603 - supported EMRStatusListT
		{
			CString strSql;
			// Detailed
			strSql.Format("SELECT "
				"EMRGroupsT.ID AS EMRID,  "
				"EMRMasterT.ID AS EMNID,  "
				"PicT.ID AS PicID,  "
				"CASE WHEN EMRGroupsT.Description = '' THEN '[No EMR Description]' ELSE EMRGroupsT.Description END AS EMRDescription,   "
				"CASE WHEN EMRMasterT.Description = '' THEN '[No EMN Description]' ELSE EMRMasterT.Description END AS EMNDescription,   "
				"EMRMasterT.PatientID AS PatID,  "
				"COALESCE(EMRMasterT.PatientFirst, PersonQ.First) AS PatFirst,  "
				"COALESCE(EMRMasterT.PatientMiddle, PersonQ.Middle) AS PatMiddle,  "
				"COALESCE(EMRMasterT.PatientLast, PersonQ.Last) AS PatLast,  "
				"dbo.GetEMNProviderList(EMRMasterT.ID) AS DocName, "
				"EMRMasterT.Date, "
				"EMRMasterT.InputDate as EMNInputDate,  "
				"EMRMasterT.ModifiedDate as EMNLastModified,  "
				"EMRMasterT.PatientAge,  "
				"EMRMasterT.PatientGender,  "
				"EMRMasterT.AdditionalNotes,  "
				"EMRTopicsT.ID AS TopicID,  "
				"EMRTopicsT.Name AS TopicName,  "
				"EMRInfoT.ID AS InfoID,  "
				"CASE WHEN EmrInfoT.ID = " + AsString((long)EMR_BUILT_IN_INFO__TEXT_MACRO) + " THEN EMRDetailsT.MacroName ELSE EmrInfoT.Name END AS ItemName, "
				"EMRInfoT.DataType,  "
				"CASE WHEN EMRInfoT.DataType = 2 THEN EMRDataT.Data WHEN EMRInfoT.DataType = 3 THEN EMRDataT.Data WHEN EMRInfoT.DataType = 5 THEN CONVERT(nvarchar, EMRDetailsT.SliderValue) WHEN EMRInfoT.DataType = 1 THEN EMRDetailsT.Text WHEN EMRInfoT.DataType = 4 THEN '[Image]' WHEN EMRInfoT.DataType = 6 THEN '[Narrative]' WHEN EMRInfoT.DataType = 7 THEN '[Table]' ELSE '[N/A]' END AS Data,  "
				"EMRStatusListT.Name AS Status,   "
				"EMRGroupsT.InputDate as EMRDate, "
				"dbo.GetEMNSecondaryProviderList(EMRMasterT.ID) AS SecondaryDocName "

				"FROM EMRGroupsT  "
				"LEFT JOIN EMRMasterT ON EMRGroupsT.ID = EMRMasterT.EMRGroupID  "
				"LEFT JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID  "
				"LEFT JOIN EMRTopicsT ON EMRTopicsT.ID = EMRDetailsT.EMRTopicID  "
				"LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
				"LEFT JOIN EMRSelectT ON EMRDetailsT.ID = EMRSelectT.EMRDetailID  "
				"LEFT JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
				"LEFT JOIN PersonT PersonQ ON PersonQ.ID = EMRMasterT.PatientID  "
				"LEFT JOIN PicT ON PicT.EMRGroupID = EMRGroupsT.ID  "
				"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
				"LEFT JOIN EMRStatusListT ON EMRMasterT.Status = EMRStatusListT.ID "
				"WHERE (EMRMasterT.Deleted = 0 AND EMRGroupsT.Deleted = 0 AND EMRDetailsT.Deleted = 0 AND EMRTopicsT.Deleted = 0  "
				"AND (PicT.IsCommitted = 1 OR PicT.IsCommitted IS NULL)  "
				"AND PicT.ID IS NOT NULL )  " + GetEmrChartPermissionFilter().Flatten() + " \r\n"

				"ORDER BY EMRGroupsT.InputDate DESC, EMRID, EMRMasterT.ID, EMRInfoT.ID");

			return _T(strSql);
		}
		
		case 579: { //Payments Under Allowed Amount (PP)

			// (j.jones 2006-12-04 09:06) - PLID 23703 - Created
			// (j.jones 2011-08-17 11:00) - PLID 44888 - ignore "original" and "void" line items

			CString strSql;
			strSql.Format("SELECT PatientsT.UserDefinedID, "
				"Last + ', ' + First + ' ' + Middle AS PatientName, "
				"InsuranceCoT.PersonID AS InsCoID, InsuranceCoT.Name AS InsCoName, "
				"PaymentLineItemT.Date AS PaymentServiceDate, PaymentLineItemT.InputDate AS PaymentInputDate, "
				"ChargeLineItemT.Date AS ChargeServiceDate, ChargeLineItemT.InputDate AS ChargeInputDate, "
				"ChargeLineItemT.Amount AS UnitPrice, Quantity, dbo.GetChargeTotal(ChargesT.ID) AS TotalCharge, "
				"Sum(AppliesT.Amount) TotalInsApplies "
				"FROM ChargesT "
				"INNER JOIN (SELECT LineItemT.* FROM LineItemT "
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
				"	WHERE Deleted = 0 "
				"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"	) AS ChargeLineItemT ON ChargesT.ID = ChargeLineItemT.ID "
				"INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
				"INNER JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID "
				"INNER JOIN PersonT ON ChargeLineItemT.PatientID = PersonT.ID "
				"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"INNER JOIN (SELECT LineItemT.* FROM LineItemT "
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaymentsQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID " 
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentsQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID " 
				"	WHERE Type = 1 AND Deleted = 0 "
				"	AND LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID Is Null "
				"	AND LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID Is Null "
				"	) AS PaymentLineItemT ON PaymentsT.ID = PaymentLineItemT.ID "
				"INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"%s "
				"GROUP BY PatientsT.UserDefinedID, PersonT.Last, PersonT.First, PersonT.Middle, "
				"InsuranceCoT.PersonID, InsuranceCoT.Name, ChargesT.ID, ItemCode, ItemSubCode, "
				"ChargeLineItemT.Amount, ChargesT.Quantity, ChargesT.CPTMultiplier1, ChargesT.CPTMultiplier2, "
				"ChargesT.CPTMultiplier3, ChargesT.CPTMultiplier4, InsuranceCoT.PersonID, "
				"PaymentLineItemT.Date, PaymentLineItemT.InputDate, "
				"ChargeLineItemT.Date, ChargeLineItemT.InputDate "
				"%s", strExtraText, strExtraField);

			return _T(strSql);
			break;
		}

		case 581: 
			{
			/* EMR Search
			// Note: This should be kept in sync with the query in CEMRSearch::RefreshList
			Version History:   
			 // (j.gruber 2006-12-29 10:28) - PLID 24034 - Created
			 // (z.manning, 02/29/2008) - PLID 29158 - Added appt start time, show state, appt resource(s), and has been billed fields.
			 // (z.manning 2008-11-12 15:24) - PLID 26167 - Added secondary prvoiders
			 // (z.manning 2009-05-20 15:54) - PLID 34314 - Added age and gender
			 // (j.jones 2009-12-29 10:35) - PLID 35795 - ensure the -25 patient is filtered out
			 // (j.gruber 2010-05-19 12:00) - PLID 38765 - optimize the GC balance query
			 // (d.lange 2011-04-25 09:41) - PLID 43381 - Added Assistant/Technician
			 // (z.manning 2011-05-23 09:40) - PLID 33114 - Added a join so this could filter on EMR chart permissions
			 // (j.jones 2011-07-05 17:49) - PLID 44432 - supported custom statuses
			 // (r.wilson 10/12/2012) plid 52586 - Added StatusID column 
			 // (j.jones 2013-07-15 16:51) - PLID 57477 - added tab categories for filtering only, not as displayed fields
			 // (s.dhole 2013-08-21 14:59) - PLID 57391 Added appointment primary and secondary insurance name
			 // (b.eyers 2015-05-19) - PLID 49999 - emns with only unbillable charges should show as <no charges> 
			 // (j.gruber 2016-01-22 12:14) - PLID 68027 - EMR Search is showing EMN's multiple times
			 */

			CString strReturn;

			strReturn.Format("SELECT EMRGroupsT.ID as GroupID, EMRMasterT.ID as EMRID, PicT.ID AS PicID, "
				" PatientsT.UserDefinedID AS PatientID, EMRMasterT.PatientID as PatID, dbo.GetEmnProviderList(EmrMasterT.ID) as ProvName, COALESCE(PatientLast, PatientQ.Last) as PatientLast,  "
				" COALESCE(PatientFirst, PatientQ.First) as PatientFirst, COALESCE(PatientMiddle, PatientQ.Middle) as PatientMiddle, "
				" EmrMasterT.PatientAge, PatientQ.BirthDate, COALESCE(EmrMasterT.PatientGender, PatientQ.Gender) AS PatientGender, "
				" EMRMasterT.Description, "
				" EMRStatusListT.ID AS StatusID, "
				" EMRStatusListT.Name AS Status, "
				" LocationsT.Name as LocName, EMRMasterT.Date, EMRMasterT.InputDate, EMRMasterT.ModifiedDate, "
				" CASE WHEN EmrMasterT.ID IN (SELECT EmrID FROM EmrChargesT WHERE Deleted = 0) THEN "
				"	CASE WHEN EmrMasterT.ID IN (SELECT EmnID FROM BilledEmnsT WHERE BillID IN "
				"		(SELECT ID FROM BillsT WHERE EntryType = 1 AND Deleted = 0)) THEN 'Yes' "
				"	WHEN BillableCPTQ.EmrID IS NOT NULL THEN 'No' "
				"	WHEN BillableProductQ.EmrID IS NOT NULL THEN 'No' "
				"	ELSE '<No Charges>' END "
				"	ELSE '<No Charges>' END AS HasBeenBilled, "
				" AppointmentsT.ID AS ApptID, AppointmentsT.StartTime, "
				" dbo.GetResourceString(AppointmentsT.ID) AS Resource, "
				" '(' + AptShowStateT.Symbol + ')' AS ShowState, "
				" dbo.GetEmnSecondaryProviderList(EmrMasterT.ID) as SecProvName, "
				" dbo.GetEmnTechniciansT(EmrMasterT.ID) as TechnicianName, "
				" ISNULL(InsuranceCoPrimQ.Name,'') as ApptPrimaryInsCo, "
				" ISNULL(InsuranceCoSecQ.Name,'') as ApptSecondaryInsCo "
				" FROM EMRGroupsT LEFT JOIN EMRMasterT ON EMRGroupsT.ID = EMRMasterT.EMRGroupID "
				" LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
				" LEFT JOIN EmnTabCategoriesLinkT ON EmrMasterT.ID = EmnTabCategoriesLinkT.EmnID "
				" LEFT JOIN PicT ON PicT.EMRGroupID = EMRGroupsT.ID LEFT JOIN PersonT PatientQ ON "
				" EMRMasterT.PatientID = PatientQ.ID LEFT JOIN PatientsT ON PatientQ.ID = PatientsT.PersonID "
				" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
				" LEFT JOIN AppointmentsT ON AppointmentsT.ID = ( "
				"	SELECT TOP 1 ID FROM AppointmentsT "
				"	WHERE AppointmentsT.Date = EmrMasterT.Date AND AppointmentsT.PatientID = EmrMasterT.PatientID "
				"	AND Status <> 4 AND ShowState <> 3 "
				"	AND AppointmentsT.PatientID <> -25 "
				"	) "
				"LEFT JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID "
				" LEFT JOIN EMRStatusListT ON EMRMasterT.Status = EMRStatusListT.ID "
				"LEFT JOIN ( "
				"	SELECT EmrChargesT.EmrID FROM EmrChargesT "
				"	LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
				"	WHERE EmrChargesT.Deleted = 0 AND CPTCodeT.Billable = 1 "
				"	GROUP BY EmrChargesT.EmrID "
				"	) AS BillableCPTQ ON EMRMasterT.ID = BillableCPTQ.EmrID "
				"LEFT JOIN ( "
				"	SELECT EmrChargesT.EmrID, ProductLocationInfoT.LocationID FROM EmrChargesT "
				"	LEFT JOIN ProductLocationInfoT ON EmrChargesT.ServiceID = ProductLocationInfoT.ProductID "
				"	WHERE EmrChargesT.Deleted = 0 AND ProductLocationInfoT.Billable = 1 "
				"   GROUP BY EMRChargesT.EMRID, ProductLocationInfoT.LocationID "
				"	) AS BillableProductQ ON EMRMasterT.ID = BillableProductQ.EmrID AND BillableProductQ.LocationID = EmrMasterT.LocationID "
				"LEFT OUTER JOIN AppointmentInsuredPartyT AS AppointmentInsuredPartyPrimQ "
				" ON ((AppointmentInsuredPartyPrimQ.AppointmentID = dbo.EMRMasterT.AppointmentID) AND  "
                " (AppointmentInsuredPartyPrimQ.Placement = 1)) "
				" LEFT OUTER JOIN InsuredPartyT AS InsuredPartyPrimQ "
				" ON AppointmentInsuredPartyPrimQ.InsuredPartyID = InsuredPartyPrimQ.PersonID "
				"LEFT OUTER JOIN InsuranceCoT AS InsuranceCoPrimQ "
				"ON InsuranceCoPrimQ.PersonID = InsuredPartyPrimQ.InsuranceCoID "
				"LEFT OUTER JOIN AppointmentInsuredPartyT AS AppointmentInsuredPartySecQ "
				"ON ((AppointmentInsuredPartySecQ.AppointmentID = dbo.EMRMasterT.AppointmentID) AND "
				" (AppointmentInsuredPartySecQ.Placement = 2)) "
				" LEFT OUTER JOIN InsuredPartyT AS InsuredPartySecQ "
				" ON AppointmentInsuredPartySecQ.InsuredPartyID = InsuredPartySecQ.PersonID "
				" LEFT OUTER JOIN InsuranceCoT AS InsuranceCoSecQ "
				" ON InsuranceCoSecQ.PersonID = InsuredPartySecQ.InsuranceCoID "
				" WHERE (1 = 1) %s ", strExtraText);
			
			return _T(strReturn);
			break;
			}

		case 585:  
			//Sales Receipt (PP)
		case 587:

			switch (nSubLevel) {
			case 0:
				{
				/* Sales Receipts
					// (j.gruber 2007-03-14 12:16) - PLID 23619 - Created
					// (j.gruber 2007-03-29 14:50) - PLID 9802 - added receipt printer format
					// (j.gruber 2007-05-01 17:24) - PLID 25745  - only show the last 4 digits of the ccnumber
					// (j.gruber 2007-05-15 09:23) - PLID 25987 - blank out CCExpDate
					// (j.gruber 2007-05-29 14:50) - PLID 25979 - added discount category
					(e.lally 2007-07-13) PLID 26649 - Replaced CCType with link to CardName, aliased as CCType.
					// (a.walling 2008-12-11 09:35) - PLID 32410 - added BillID to sales receipt
					// (j.gruber 2009-04-01 14:10) - PLID 33795 - updated discount structure
					// (z.manning 2009-09-01 08:26) - PLID 30148 - Added NextApptDate
					// (j.gruber 2009-10-01 10:48) - PLID 35193 - additional fields
					// (j.gruber 2009-10-27 14:33) - PLID 36041 - added charge applied amount
					// (j.gruber 2009-11-05 10:38) - PLID 36202 - added responsible party information
					// (j.jones 2009-11-10 09:52) - PLID 34165 - added "other applies" fields
					// (j.gruber 2009-12-18 12:41) - PLID 28609 - added gift certificate balance for payments that used gift certificates
					// (j.jones 2010-05-27 08:47) - PLID 34002 - added diagnosis codes, including the top 4 "additional" codes exposed with names 5 though 8
					// (j.dinatale 2011-09-30 16:53) - PLID 45773 - take into account the quantity when calculating GC balances
					// (b.savon 2012-02-06 12:06) - PLID 44056 - Optimize the Daily Sales Receipt - Replace UNION with UNION ALL
					// (d.thompson 2013-01-30) - PLID 46773 - Added title and prefix data
					// (j.jones 2013-07-15 08:28) - PLID 57504 - added InvoiceNumber
					// (j.gruber 2014-03-21 14:39) - PLID 61484 - updated for ICD-10
					// (j.jones 2015-03-09 11:18) - PLID 65148 - if the description begins with 'Corrected Charge',
					// 'Corrected Payment', etc., strip that off
					// (r.gonet 2015-06-04 10:52) - PLID 65657 - Updated to use the new Gift Certificates Balance query.
					*/
					CString strReturn;

				strReturn.Format(" SELECT LineItemT.ID, LineItemT.Type, "
					"(CASE WHEN LineItemCorrectionsT_VoidedCharge.VoidingLineItemID Is Not Null "
					"	AND Left(LineItemT.Description, Len('Voided Charge')) = 'Voided Charge' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description) - Len('Voided Charge'))) "
					" WHEN LineItemCorrectionsT_NewCharge.NewLineItemID Is Not Null "
					"	AND Left(LineItemT.Description, Len('Corrected Charge')) = 'Corrected Charge' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description) - Len('Corrected Charge'))) "
					"ELSE LineItemT.Description END) AS Description, "
					"LineItemT.Date, LineItemT.InputDate, LineItemT.InputName, LineItemT.Amount, "
					" ChargesT.ItemCode, ChargesT.ItemSubCode, ChargesT.TaxRate, ChargesT.TaxRate2, COALESCE(TotalPercentOff, 0) as PercentOff, COALESCE(TotalDiscount,0) as Discount, ChargesT.Quantity, "
					" 0 as PrePayment, -1 as PayMethod, '' as CheckNo, '' as BankNo, '' as CheckAcctNo, '' as CCType,  "
					" '' as CCNumber, '' as CCHoldersName, Convert(datetime, NULL) AS CCExpDate, '' as CCAuthNo, '' AS BankRoutingNum, "
					" Convert(money, 0) AS CashReceived, "
					" ProvPersonT.First as ProvFirst, ProvPersonT.Middle as ProvMiddle, ProvPersonT.Last as ProvLast, ProvPersonT.Title as ProvTitle,  "
					" ProvPersonPrefixT.Prefix as ProvPrefix, LocationsT.Name,  "
					" LocationsT.Address1 as LocAddress1, LocationsT.Address2 as LocAddress2, LocationsT.City as LocCity, "
					" LocationsT.State as LocState, LocationsT.Zip as LocZip, LocationsT.Phone as LocPhone, "
					" CoordPersonT.First as CoordFirst, CoordPersonT.Middle as CoordMiddle, CoordPersonT.Last as CoordLast, CoordPersonT.Title as CoordTitle,  "
					" BillsT.Date as BillDate, "
					" BillsT.Description AS BillDescription, "
					" PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, "
					" PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, "
					
					" Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)),2) AS ChargeTotalNoDiscountsWithTax, "
					
					" Round(CONVERT(money, LineItemT.Amount * (CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)),2) AS ChargeAmount,  "

										
					" (Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)),2)) - "
					" Round(Convert(money, ((((CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]* "
					" (CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*"
					" (CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*"
					" (CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*"
					" (CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)))) "
					" *CASE WHEN COALESCE([TotalPercentOff], 0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,COALESCE([TotalPercentOff],0))/100.0)) END "
					" - COALESCE([TotalDiscount],0)), 2) AS DiscountAmt, "

					" Round(Convert(money, ((( "
					" /* Base Calculation */ "
					" (CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*  "
					" (CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
					" (CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
					" (CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
					" (CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
					" )* /* Discount 1 */ "
					" CASE WHEN COALESCE([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,COALESCE([TotalPercentOff],0))/100.0)) END "
					" ) - /* Discount 2 */ "
					" CASE WHEN Amount > 0 OR OthrBillFee = 0 THEN COALESCE([TotalDiscount],0) ELSE 0 END "
					" )* /* Tax */ "
					" (ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)  "
					" ), 2) AS TotalChargeWithDiscounts, "

					" Round(Convert(money, ((( "
					" /* Base Calculation */ "
					" (CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*  "
					" (CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
					" (CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
					" (CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
					" (CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
					" )* /* Discount 1 */ "
					" CASE WHEN COALESCE([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,COALESCE([TotalPercentOff],0))/100.0)) END "
					" ) - /* Discount 2 */ "
					" CASE WHEN LineItemT.Amount > 0 OR OthrBillFee = 0 THEN COALESCE([TotalDiscount],0) ELSE 0 END "
					" )* /* Tax */ "
					"  (ChargesT.[TaxRate]-1)  "
					" ),2) AS Tax1Total,  "

					" Round(Convert(money, ((( "
					" /* Base Calculation */ "
					" (CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*  "
					" (CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
					" (CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
					" (CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
					" (CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
					" )* /* Discount 1 */ "
					" CASE WHEN COALESCE([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,COALESCE([TotalPercentOff],0))/100.0)) END "
					" ) - /* Discount 2 */ "
					" CASE WHEN LineItemT.Amount > 0 OR OthrBillFee = 0 THEN COALESCE([TotalDiscount],0) ELSE 0 END "
					" )* /* Tax */ "
					"  (ChargesT.[TaxRate2]-1)  "
					" ),2) AS Tax2Total,  "

					" Round(Convert(money, ((( "
					" /* Base Calculation */ "
					" (CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*  "
					" (CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
					" (CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
					" (CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
					" (CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
					" )* /* Discount 1 */ "
					" CASE WHEN COALESCE([TotalPercentOff],0) IS NULL THEN 1 ELSE (1.0 - (Convert(float,COALESCE([TotalPercentOff],0))/100.0)) END "
					" ) - /* Discount 2 */ "
					" CASE WHEN Amount > 0 OR OthrBillFee = 0 THEN COALESCE([TotalDiscount],0) ELSE 0 END "
					" ) "
					" ), 2) AS TotalChargeWithDiscountsNoTax, "

					" Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)),2) AS ChargeTotalNoDiscountsNoTax, "
					" 0 as CountTips, "

					" dbo.GetChargeDiscountList(ChargesT.ID) AS DiscountCategoryDescription,"
					" BillsT.ID AS BillID, " 
					" (SELECT Min(StartTime) FROM AppointmentsT WHERE Status <> 4 AND Date > getdate() AND PatientID = PersonT.ID) AS NextApptDate, "

					" ProvidersT.NPI as ProvNPI, PersonRefPhysT.First as RefPhysFirst, PersonRefPhysT.Middle as RefPhysMiddle, PersonRefPhysT.Last as RefPhysLast, "
					" PersonRefPhysT.Title as RefPhysTitle, ReferringPhysT.NPI as RefPhysNPI, "
					
					" PriInsPartyT.First as PriInsFirst, PriInsPartyT.Middle as PriInsMiddle, PriInsPartyT.Last as PriInsLast, PriInsPartyT.IdForInsurance as PriInsID, PriInsPartyT.PolicyGroupNum as PriInsGroupNum, PriInsPartyT.BirthDate as PriInsBirthDate, "
					" PriInsPartyT.Name as PriInsCoName, PriInsPartyT.Address1 as PriInsCoAdd1, PriInsPartyT.Address2 as PriInsCoAdd2, PriInsPartyT.City as PriInsCoCity, PriInsPartyT.State as PriInsCoState, PriInsPartyT.Zip as PriInsCoZip, PriInsPartyT.PhoneNumber as PriInsCoPhone, "

					" SecInsPartyT.First as SecInsFirst, SecInsPartyT.Middle as SecInsMiddle, SecInsPartyT.Last as SecInsLast, SecInsPartyT.IdForInsurance as SecInsID, SecInsPartyT.PolicyGroupNum as SecInsGroupNum, SecInsPartyT.BirthDate as SecInsBirthDate, "
					" SecInsPartyT.Name as SecInsCoName, SecInsPartyT.Address1 as SecInsCoAdd1, SecInsPartyT.Address2 as SecInsCoAdd2, SecInsPartyT.City as SecInsCoCity, SecInsPartyT.State as SecInsCoState, SecInsPartyT.Zip as SecInsCoZip, SecInsPartyT.PhoneNumber as SecInsCoPhone, "
					
					" POsLocationT.Name as POSLocationName, "

					" PersonT.BirthDate as PatBirthDate, "

					" NULL as ReceiptID, "

					" InsuranceReferralsT.AuthNum, InsuranceReferralsT.StartDate as InsRefStart, InsuranceReferralsT.EndDate as InsRefEnd, InsuranceReferralsT.NumVisits as InsRefNumVisits, InsuranceReferralsT.Comments as InsrefComments,  "
					" InsRefLocationsT.Name as InsRefLocName,  InsRefProvPersonT.First as InsRefProvFirst, InsRefProvPersonT.Middle as InsRefProvMiddle, InsRefProvPersonT.Last as InsRefProvLast, InsRefProvPersonT.Title as InsRefProvTitle, "

					" dbo.GetChargeAllowableForInsuranceCo(ChargesT.ID, PriInsPartyT.InsuranceCoID) as PriAllowedAmount, "
					" dbo.GetChargeAllowableForInsuranceCo(ChargesT.ID, SecInsPartyT.InsuranceCoID) as SecAllowedAmount, "
					" (SELECT Sum(Amount) FROM AppliesT WHERE DestID = ChargesT.ID) AS ChargeAppliedAmt, "					
					" Convert(money,0) AS OtherChargeApplies, "
					" Coalesce(OtherPaymentAppliesQ.ApplyAmt, Convert(money,0)) AS OtherPaymentApplies, "
					" RespPartyPersonT.First as RespPartyFirst, RespPartyPersonT.Middle as RespPartyMiddle, RespPartyPersonT.Last as RespPartyLast, "
					" RespPartyPersonT.Address1 as RespPartyAdd1, RespPartyPersonT.Address2 as RespPartyAdd2, RespPartyPersonT.City as RespPartyCity, "
					" RespPartyPersonT.State as RespPartyState, RespPartyPersonT.Zip as RespPartyZip, NULL as GCBalance, "

					"ICD9T1.CodeNumber as ICD9Code1, \r\n "
					"ICD9T2.CodeNumber as ICD9Code2, \r\n "
					"ICD9T3.CodeNumber as ICD9Code3, \r\n "
					"ICD9T4.CodeNumber as ICD9Code4, \r\n "
					"ICD9T5.CodeNumber as ICD9Code5, \r\n "
					"ICD9T6.CodeNumber as ICD9Code6, \r\n "
					"ICD9T7.CodeNumber as ICD9Code7, \r\n "
					"ICD9T8.CodeNumber as ICD9Code8, \r\n "

					"ICD10T1.CodeNumber as ICD10Code1, \r\n "
					"ICD10T2.CodeNumber as ICD10Code2, \r\n "
					"ICD10T3.CodeNumber as ICD10Code3, \r\n "
					"ICD10T4.CodeNumber as ICD10Code4, \r\n "
					"ICD10T5.CodeNumber as ICD10Code5, \r\n "
					"ICD10T6.CodeNumber as ICD10Code6, \r\n "
					"ICD10T7.CodeNumber as ICD10Code7, \r\n "
					"ICD10T8.CodeNumber as ICD10Code8, \r\n "

					"ICD9T1.CodeDesc as ICD9CodeDesc1, \r\n "
					"ICD9T2.CodeDesc as ICD9CodeDesc2, \r\n "
					"ICD9T3.CodeDesc as ICD9CodeDesc3, \r\n "
					"ICD9T4.CodeDesc as ICD9CodeDesc4, \r\n "
					"ICD9T5.CodeDesc as ICD9CodeDesc5, \r\n "
					"ICD9T6.CodeDesc as ICD9CodeDesc6, \r\n "
					"ICD9T7.CodeDesc as ICD9CodeDesc7, \r\n "
					"ICD9T8.CodeDesc as ICD9CodeDesc8, \r\n "

					"ICD10T1.CodeDesc as ICD10CodeDesc1, \r\n "
					"ICD10T2.CodeDesc as ICD10CodeDesc2, \r\n "
					"ICD10T3.CodeDesc as ICD10CodeDesc3, \r\n "
					"ICD10T4.CodeDesc as ICD10CodeDesc4,  \r\n "
					"ICD10T5.CodeDesc as ICD10CodeDesc5, \r\n "
					"ICD10T6.CodeDesc as ICD10CodeDesc6, \r\n "
					"ICD10T7.CodeDesc as ICD10CodeDesc7, \r\n "
					"ICD10T8.CodeDesc as ICD10CodeDesc8,  \r\n "

					" PatientPrefixT.Prefix AS PatientPrefix, PersonT.Title AS PatientTitle, "
					" BillInvoiceNumbersT.InvoiceID AS InvoiceNumber "

					" FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
					" LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
					" LEFT JOIN BillInvoiceNumbersT ON BillsT.ID = BillInvoiceNumbersT.BillID "
					" LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
					" LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
					" LEFT JOIN PersonT ProvPersonT ON ProvidersT.PersonID = ProvPersonT.ID "
					" LEFT JOIN PrefixT ProvPersonPrefixT ON ProvPersonT.PrefixID = ProvPersonPrefixT.ID "
					" LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
					" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					" LEFT JOIN PrefixT PatientPrefixT ON PersonT.PrefixID = PatientPrefixT.ID "
					" LEFT JOIN PersonT CoordPersonT ON PatientsT.EmployeeID = CoordPersonT.ID "
					" LEFT JOIN LocationsT ON LineItemT.LocationId = LocationsT.ID "
					" LEFT JOIN ReferringPhysT ON PatientsT.DefaultReferringPhyID = ReferringPhysT.PersonID "
					" LEFT JOIN PersonT PersonRefPhysT ON ReferringPhysT.PersonID = PersonRefPhysT.ID "
					" LEFT JOIN InsuranceReferralsT ON BillsT.InsuranceReferralID = InsuranceReferralsT.ID "
					" LEFT JOIN PersonT InsRefProvPersonT ON InsuranceReferralsT.ProviderID = InsRefProvPersonT.ID "
					" LEFT JOIN LocationsT InsRefLocationsT ON InsuranceReferralsT.LocationID = InsRefLocationsT.ID "
					" LEFT JOIN (SELECT InsuredPartyT.InsuranceCoID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.BirthDate, IDForInsurance, PolicyGroupNum, PatientID, InsuranceCoT.Name, PersonInsCoT.Address1, PersonInscoT.Address2, PersonInsCoT.City, PersonInsCoT.State, PersonInsCoT.Zip, PersonInsContactT.WorkPhone as PhoneNumber FROM InsuredPartyT INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoId = InsuranceCoT.PersonID LEFT JOIN PersonT PersonInsCoT ON InsuranceCoT.PersonID = PersonInsCoT.ID LEFT JOIN PersonT PersonInsContactT ON InsuredPartyT.InsuranceContactID = PersonInsContactT.ID WHERE InsuredPartyT.RespTypeID = 1) PriInsPartyT ON PatientsT.PersonID = PriInsPartyT.PatientID "
					" LEFT JOIN (SELECT InsuredPartyT.InsuranceCoID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.BirthDate, IDForInsurance, PolicyGroupNum, PatientID, InsuranceCoT.Name, PersonInsCoT.Address1, PersonInscoT.Address2, PersonInsCoT.City, PersonInsCoT.State, PersonInsCoT.Zip, PersonInsContactT.WorkPhone as PhoneNumber FROM InsuredPartyT INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoId = InsuranceCoT.PersonID LEFT JOIN PersonT PersonInsCoT ON InsuranceCoT.PersonID = PersonInsCoT.ID LEFT JOIN PersonT PersonInsContactT ON InsuredPartyT.InsuranceContactID = PersonInsContactT.ID WHERE InsuredPartyT.RespTypeID = 2) SecInsPartyT ON PatientsT.PersonID = SecInsPartyT.PatientID "
					" LEFT JOIN LocationsT POSLocationT ON BillsT.Location = POSLocationT.ID "					
					" LEFT JOIN ResponsiblePartyT ON PatientsT.PrimaryRespPartyID = ResponsiblePartyT.PersonID "
					" LEFT JOIN PersonT RespPartyPersonT ON ResponsiblePartyT.PersonID = RespPartyPersonT.ID "
					" LEFT JOIN "
						"(SELECT AppliesT.DestID, Sum(AppliesT.Amount) AS ApplyAmt "
						"FROM AppliesT INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
						"WHERE Deleted = 0 AND SourceID NOT IN (%s -1) "
						"GROUP BY DestID "
						") OtherPaymentAppliesQ "
					" ON LineItemT.ID = OtherPaymentAppliesQ.DestID "
					"LEFT JOIN BillDiagCodeFlat12V ON BillsT.ID = BillDiagCodeFlat12V.BillID \r\n "
					"LEFT JOIN DiagCodes ICD9T1 ON BillDiagCodeFlat12V.ICD9Diag1ID = ICD9T1.ID \r\n "
					"LEFT JOIN DiagCodes ICD9T2 ON BillDiagCodeFlat12V.ICD9Diag2ID = ICD9T2.ID \r\n "
					"LEFT JOIN DiagCodes ICD9T3 ON BillDiagCodeFlat12V.ICD9Diag3ID = ICD9T3.ID \r\n "
					"LEFT JOIN DiagCodes ICD9T4 ON BillDiagCodeFlat12V.ICD9Diag4ID = ICD9T4.ID \r\n "
					"LEFT JOIN DiagCodes ICD9T5 ON BillDiagCodeFlat12V.ICD9Diag5ID = ICD9T5.ID \r\n "
					"LEFT JOIN DiagCodes ICD9T6 ON BillDiagCodeFlat12V.ICD9Diag6ID = ICD9T6.ID \r\n "
					"LEFT JOIN DiagCodes ICD9T7 ON BillDiagCodeFlat12V.ICD9Diag7ID = ICD9T7.ID \r\n "
					"LEFT JOIN DiagCodes ICD9T8 ON BillDiagCodeFlat12V.ICD9Diag8ID = ICD9T8.ID \r\n "

					"LEFT JOIN DiagCodes ICD10T1 ON BillDiagCodeFlat12V.ICD10Diag1ID = ICD10T1.ID \r\n"
					"LEFT JOIN DiagCodes ICD10T2 ON BillDiagCodeFlat12V.ICD10Diag2ID = ICD10T2.ID \r\n "
					"LEFT JOIN DiagCodes ICD10T3 ON BillDiagCodeFlat12V.ICD10Diag3ID = ICD10T3.ID \r\n "
					"LEFT JOIN DiagCodes ICD10T4 ON BillDiagCodeFlat12V.ICD10Diag4ID = ICD10T4.ID \r\n "					
					"LEFT JOIN DiagCodes ICD10T5 ON BillDiagCodeFlat12V.ICD10Diag5ID = ICD10T5.ID \r\n"
					"LEFT JOIN DiagCodes ICD10T6 ON BillDiagCodeFlat12V.ICD10Diag6ID = ICD10T6.ID \r\n "
					"LEFT JOIN DiagCodes ICD10T7 ON BillDiagCodeFlat12V.ICD10Diag7ID = ICD10T7.ID \r\n "
					"LEFT JOIN DiagCodes ICD10T8 ON BillDiagCodeFlat12V.ICD10Diag8ID = ICD10T8.ID \r\n "

					"LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_VoidedCharge ON ChargesT.ID = LineItemCorrectionsT_VoidedCharge.VoidingLineItemID \r\n"
					"LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewCharge ON ChargesT.ID = LineItemCorrectionsT_NewCharge.NewLineItemID \r\n"
					
					" WHERE LineItemT.Type = 10 AND LineItemT.Deleted = 0 AND BillsT.Deleted = 0 AND LineItemT.ID IN (%s -1) "
					" "
					" UNION ALL "
					" SELECT LineItemT.ID, LineItemT.Type, "
					" (CASE WHEN LineItemCorrectionsT_VoidedPay.VoidingLineItemID Is Not Null AND LineItemT.Type = 1 "
					"	AND Left(LineItemT.Description, Len('Voided Payment')) = 'Voided Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Voided Payment'))) "
					"  WHEN LineItemCorrectionsT_VoidedPay.VoidingLineItemID Is Not Null AND LineItemT.Type = 2 "
					"	AND Left(LineItemT.Description, Len('Voided Adjustment')) = 'Voided Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Voided Adjustment'))) "
					"  WHEN LineItemCorrectionsT_VoidedPay.VoidingLineItemID Is Not Null AND LineItemT.Type = 3 "
					"	AND Left(LineItemT.Description, Len('Voided Refund')) = 'Voided Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Voided Refund'))) "
					"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 1 "
					"	AND Left(LineItemT.Description, Len('Corrected Payment')) = 'Corrected Payment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Payment'))) "
					"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 2 "
					"	AND Left(LineItemT.Description, Len('Corrected Adjustment')) = 'Corrected Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Adjustment'))) "
					"  WHEN LineItemCorrectionsT_NewPay.NewLineItemID Is Not Null AND LineItemT.Type = 3 "
					"	AND Left(LineItemT.Description, Len('Corrected Refund')) = 'Corrected Refund' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Corrected Refund'))) "
					"  WHEN LineItemCorrectionsBalancingAdjT.BalancingAdjID Is Not Null AND LineItemT.Type = 2 "
					"	AND Left(LineItemT.Description, Len('Voided Charge Adjustment')) = 'Voided Charge Adjustment' THEN LTRIM(Right(LineItemT.Description, Len(LineItemT.Description)-Len('Voided Charge Adjustment'))) "
					"ELSE LineItemT.Description END) AS Description, "
					" LineItemT.Date, LineItemT.InputDate, LineItemT.InputName, LineItemT.Amount, "
					" ''As ItemCode, '' AS ItemSubCode, -1 as TaxRate, -1 as TaxRate2, -1 as PercentOff, convert(money, -1) as Discount, -1 as Quantity, "
					" PaymentsT.PrePayment, PaymentsT.PayMethod, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, CreditCardNamesT.CardName AS CCType,  "
					" CASE WHEN Len(PaymentPlansT.CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(PaymentPlansT.CCNumber, 4) END  as CCNumber, PaymentPlansT.CCHoldersName, Convert(datetime, NULL) AS CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, "
					" PaymentsT.CashReceived, "
					" ProvPersonT.First as ProvFirst, ProvPersonT.Middle as ProvMiddle, ProvPersonT.Last as ProvLast, ProvPersonT.Title as ProvTitle,  "
					" ProvPersonPrefixT.Prefix as ProvPrefix, LocationsT.Name,  "
					" LocationsT.Address1, LocationsT.Address2, LocationsT.City, "
					" LocationsT.State, LocationsT.Zip, LocationsT.Phone, "
					" CoordPersonT.First as CoordFirst, CoordPersonT.Middle as CoordMiddle, CoordPersonT.Last as CoordLast, CoordPersonT.Title as CoordTitle,  "
					" NULL as BillDate, '' AS BillDescription,  "
					" PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, "
					" PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, "
					" Convert(money, 0), Convert(money, 0), Convert(money, 0), Convert(money, 0), Convert(money, 0), Convert(money, 0), Convert(money, 0), Convert(money, 0),   "
					" (SELECT Count(ID) FROM PaymentTipsT WHERE PaymentTipsT.PaymentID = LineItemT.ID) As CountTips, " 
					" '' as DiscountCategoryDescription,"
					" NULL AS BillID, " 
					" (SELECT Min(StartTime) FROM AppointmentsT WHERE Status <> 4 AND Date > getdate() AND PatientID = PersonT.ID) AS NextApptDate, "

					" ProvidersT.NPI as ProvNPI, PersonRefPhysT.First as RefPhysFirst, PersonRefPhysT.Middle as RefPhysMiddle, PersonRefPhysT.Last as RefPhysLast, "
					" PersonRefPhysT.Title as RefPhysTitle, ReferringPhysT.NPI as RefPhysNPI, "
					
					" PriInsPartyT.First as PriInsFirst, PriInsPartyT.Middle as PriInsMiddle, PriInsPartyT.Last as PriInsLast, PriInsPartyT.IdForInsurance as PriInsID, PriInsPartyT.PolicyGroupNum as PriInsGroupNum, PriInsPartyT.BirthDate as PriInsBirthDate, "
					" PriInsPartyT.Name as PriInsCoName, PriInsPartyT.Address1 as PriInsCoAdd1, PriInsPartyT.Address2 as PriInsCoAdd2, PriInsPartyT.City as PriInsCoCity, PriInsPartyT.State as PriInsCoState, PriInsPartyT.Zip as PriInsCoZip, PriInsPartyT.PhoneNumber as PriInsCoPhone, "

					" SecInsPartyT.First as SecInsFirst, SecInsPartyT.Middle as SecInsMiddle, SecInsPartyT.Last as SecInsLast, SecInsPartyT.IdForInsurance as SecInsID, SecInsPartyT.PolicyGroupNum as SecInsGroupNum, SecInsPartyT.BirthDate as SecInsBirthDate, "
					" SecInsPartyT.Name as SecInsCoName, SecInsPartyT.Address1 as SecInsCoAdd1, SecInsPartyT.Address2 as SecInsCoAdd2, SecInsPartyT.City as SecInsCoCity, SecInsPartyT.State as SecInsCoState, SecInsPartyT.Zip as SecInsCoZip, SecInsPartyT.PhoneNumber as SecInsCoPhone, "

					" '' as POSLocationName, "

					" PersonT.BirthDate as PatBirthDate, "

					" PaymentsT.ID as ReceiptID, "

					" '' as AuthNum, NULL as InsRefStart, NULL as InsRefEnd, '' as InsRefNumVisits, '' as InsRefComments, "
					" '' as InsRefLocName,  '' as InsRefProvFirst, '' as InsRefProvMiddle, '' as InsRefProvLast, '' as InsRefProvTitle, "

					" Null as PriAllowedAmount, Null as SecAllowedAmount, NULL as ChargeAppliedAmt, "					
					" Coalesce(OtherChargeAppliesQ.ApplyAmt, Convert(money,0)) AS OtherChargeApplies, "
					" Coalesce(OtherPaymentAppliesQ.ApplyAmt, Convert(money,0)) AS OtherPaymentApplies, "
					" RespPartyPersonT.First as RespPartyFirst, RespPartyPersonT.Middle as RespPartyMiddle, RespPartyPersonT.Last as RespPartyLast, "
					" RespPartyPersonT.Address1 as RespPartyAdd1, RespPartyPersonT.Address2 as RespPartyAdd2, RespPartyPersonT.City as RespPartyCity, "
					" RespPartyPersonT.State as RespPartyState, RespPartyPersonT.Zip as RespPartyZip, "

					" GiftCertificatesQ.Balance AS GCBalance, "

					"NULL as ICD9Code1, \r\n "
					"NULL as ICD9Code2, \r\n "
					"NULL as ICD9Code3, \r\n "
					"NULL as ICD9Code4, \r\n "
					"NULL as ICD9Code5, \r\n "
					"NULL as ICD9Code6, \r\n "
					"NULL as ICD9Code7, \r\n "
					"NULL as ICD9Code8, \r\n "

					"NULL as ICD10Code1, \r\n "
					"NULL as ICD10Code2, \r\n "
					"NULL as ICD10Code3, \r\n "
					"NULL as ICD10Code4, \r\n "
					"NULL as ICD10Code5, \r\n "
					"NULL as ICD10Code6, \r\n "
					"NULL as ICD10Code7, \r\n "
					"NULL as ICD10Code8, \r\n "

					"NULL as ICD9CodeDesc1, \r\n "
					"NULL as ICD9CodeDesc2, \r\n "
					"NULL as ICD9CodeDesc3, \r\n "
					"NULL as ICD9CodeDesc4, \r\n "
					"NULL as ICD9CodeDesc5, \r\n "
					"NULL as ICD9CodeDesc6, \r\n "
					"NULL as ICD9CodeDesc7, \r\n "
					"NULL as ICD9CodeDesc8, \r\n "

					"NULL as ICD10CodeDesc1, \r\n "
					"NULL as ICD10CodeDesc2, \r\n "
					"NULL as ICD10CodeDesc3, \r\n "
					"NULL as ICD10CodeDesc4,  \r\n "
					"NULL as ICD10CodeDesc5, \r\n "
					"NULL as ICD10CodeDesc6, \r\n "
					"NULL as ICD10CodeDesc7, \r\n "
					"NULL as ICD10CodeDesc8,  \r\n "

					" PatientPrefixT.Prefix AS PatientPrefix, PersonT.Title AS PatientTitle, "
					" NULL AS InvoiceNumber "

					" FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
					" INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
					" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
					" LEFT JOIN ProvidersT ON PaymentsT.ProviderID = ProvidersT.PersonID "
					" LEFT JOIN PersonT ProvPersonT ON ProvidersT.PersonID = ProvPersonT.ID "
					" LEFT JOIN PrefixT ProvPersonPrefixT ON ProvPersonT.PrefixID = ProvPersonPrefixT.ID "
					" LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
					" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					" LEFT JOIN PrefixT PatientPrefixT ON PersonT.PrefixID = PatientPrefixT.ID "
					" LEFT JOIN PersonT CoordPersonT ON PatientsT.EmployeeID = CoordPersonT.ID "
					" LEFT JOIN LocationsT ON LineItemT.LocationId = LocationsT.ID "
					" LEFT JOIN ReferringPhysT ON PatientsT.DefaultReferringPhyID = ReferringPhysT.PersonID "
					" LEFT JOIN PersonT PersonRefPhysT ON ReferringPhysT.PersonID = PersonRefPhysT.ID "					
					" LEFT JOIN (SELECT InsuredPartyT.InsuranceCoID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.BirthDate, IDForInsurance, PolicyGroupNum, PatientID, InsuranceCoT.Name, PersonInsCoT.Address1, PersonInscoT.Address2, PersonInsCoT.City, PersonInsCoT.State, PersonInsCoT.Zip, PersonInsContactT.WorkPhone as PhoneNumber FROM InsuredPartyT INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoId = InsuranceCoT.PersonID LEFT JOIN PersonT PersonInsCoT ON InsuranceCoT.PersonID = PersonInsCoT.ID LEFT JOIN PersonT PersonInsContactT ON InsuredPartyT.InsuranceContactID = PersonInsContactT.ID WHERE InsuredPartyT.RespTypeID = 1) PriInsPartyT ON PatientsT.PersonID = PriInsPartyT.PatientID "
					" LEFT JOIN (SELECT InsuredPartyT.InsuranceCoID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.BirthDate, IDForInsurance, PolicyGroupNum, PatientID, InsuranceCoT.Name, PersonInsCoT.Address1, PersonInscoT.Address2, PersonInsCoT.City, PersonInsCoT.State, PersonInsCoT.Zip, PersonInsContactT.WorkPhone as PhoneNumber FROM InsuredPartyT INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoId = InsuranceCoT.PersonID LEFT JOIN PersonT PersonInsCoT ON InsuranceCoT.PersonID = PersonInsCoT.ID LEFT JOIN PersonT PersonInsContactT ON InsuredPartyT.InsuranceContactID = PersonInsContactT.ID WHERE InsuredPartyT.RespTypeID = 2) SecInsPartyT ON PatientsT.PersonID = SecInsPartyT.PatientID "
					" LEFT JOIN ResponsiblePartyT ON PatientsT.PrimaryRespPartyID = ResponsiblePartyT.PersonID "
					" LEFT JOIN PersonT RespPartyPersonT ON ResponsiblePartyT.PersonID = RespPartyPersonT.ID "
					" LEFT JOIN "
						//find other ChargesT records this payment is applied to
						"(SELECT AppliesT.SourceID, Sum(AppliesT.Amount) AS ApplyAmt, PatientID "
						"FROM AppliesT INNER JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
						"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
						"WHERE Deleted = 0 AND ChargesT.ID NOT IN (%s -1) "						
						"GROUP BY SourceID, PatientID "
						") OtherChargeAppliesQ "
					" ON LineItemT.ID = OtherChargeAppliesQ.SourceID "
					" LEFT JOIN "
						"(SELECT PaymentID, Sum(ApplySubAmt) AS ApplyAmt, PatientID FROM "
							//find other PaymentsT records applied to this payment
							"(SELECT AppliesT.DestID AS PaymentID, Sum(AppliesT.Amount) AS ApplySubAmt, PatientID "
							"FROM AppliesT INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
							"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
							"WHERE Deleted = 0 AND PaymentsT.ID NOT IN (%s -1) "
							"GROUP BY DestID, PatientID "
							"UNION ALL "
							//find other PaymentsT records this payment is applied to
							"SELECT AppliesT.SourceID AS PaymentID, Sum(-AppliesT.Amount) AS ApplySubAmt, PatientID "
							"FROM AppliesT INNER JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
							"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
							"WHERE Deleted = 0 AND PaymentsT.ID NOT IN (%s -1) "						
							"GROUP BY SourceID, PatientID "
							") OtherPaymentAppliesSubQ "
							"GROUP BY PaymentID, PatientID "
						") OtherPaymentAppliesQ "
					" ON LineItemT.ID = OtherPaymentAppliesQ.PaymentID "

					"LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_VoidedPay ON PaymentsT.ID = LineItemCorrectionsT_VoidedPay.VoidingLineItemID "
					"LEFT JOIN LineItemCorrectionsT LineItemCorrectionsT_NewPay ON PaymentsT.ID = LineItemCorrectionsT_NewPay.NewLineItemID "
					"LEFT JOIN LineItemCorrectionsBalancingAdjT ON PaymentsT.ID = LineItemCorrectionsBalancingAdjT.BalancingAdjID "
					"LEFT JOIN ( "
					+ GetGiftCertificateValueQuery() +
					") GiftCertificatesQ ON LineItemT.GiftID = GiftCertificatesQ.ID "

					" WHERE LineItemT.Type < 10 AND LineItemT.Deleted = 0 AND LineItemT.ID IN (%s -1) ", strExtraText, strExtraText, strExtraText, strExtraText, strExtraText, strExtraText);
					return strReturn;
				}
			break;

			case 1:
				switch (nSubRepNum) {


				case 0: { //discounts
						CString str;
						str.Format("SELECT ID, ChargeID, BillID, PercentOff, Discount, DiscountCategoryDescription, DiscountTotal FROM (  "
							" 		SELECT ChargeDiscountsT.ID, ChargeDiscountsT.ChargeID, ChargesT.BillID, PercentOff, Discount, CASE WHEN ChargeDiscountsT.DiscountCategoryID IS NULL THEN '' ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -1 THEN ChargeDiscountsT.CustomDiscountDesc ELSE  "
							" 		 CASE WHEN ChargeDiscountsT.DiscountCategoryID = -2 THEN (SELECT Description FROM CouponsT WHERE ID = ChargeDiscountsT.CouponID) ELSE  "
							" 		 (SELECT Description FROM DiscountCategoriesT WHERE ID = ChargeDiscountsT.DiscountCategoryID) END END END AS DiscountCategoryDescription, "
							" 		 Round(Convert(money,  "
							" 			((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([PercentOff] Is Null) THEN 0 ELSE [PercentOff] END)/100)+(CASE WHEN([Discount] Is Null) THEN 0 ELSE [Discount] END)))  "
							" 			),2) AS DiscountTotal "
							" 		 FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN ChargeDiscountsT ON ChargesT.ID = ChargeDiscountsT.ChargeID "
							" WHERE LineItemT.Deleted = 0 AND ChargeDiscountsT.DELETED = 0 ) Q"); 
						
							return _T(str);
						}
					break;



					case 1:  //payment Tips
						{
						CString strReturn;
						strReturn.Format("SELECT PaymentID, Amount, PayMethod,  "
							" ProvT.First as ProvFirst, ProvT.Middle AS ProvMiddle, "
							" ProvT.Last AS ProvLast, ProvT.Title AS ProvTitle "
							" FROM "
							" PaymentTipsT LEFT JOIN ProvidersT ON  "
							" PaymentTipsT.ProvID = ProvidersT.PersonID "
							" LEFT JOIN PersonT ProvT ON ProvidersT.PersonID = ProvT.ID "
							" WHERE (1 = 1) %s ", strExtendedSql);
						return strReturn;
						}
					break;

					default:
						return _T("");
					break;
				}
			break;

			default:
				return _T("");
			break;
		}


		break;


		case 604:
		case 605:
			{
				/*Credit Card Customer and Merchant Copies
				Version History:
					// (j.gruber 2007-07-26 17:03) - PLID 26720 - Created
					(d.thompson 2009-07-01) - PLID 34764 - Updated for QBMS
					(d.thompson 2010-07-12) - PLID 39581 - Added the patient name, in case it differs from the cc holder
					// (d.thompson 2010-12-20) - PLID 41897 - Supported Chase data too
					(c.haag 2015-09-09) - PLID 67194 - Supported Integrated Credit Card Processing. CardConnect_CreditTransactionT 
					always has the original authorization so we join on that. With ICCP, "TransactionType" is actually a parameter set from 
					PrintCreditCardReceipt(), and the field itself has different meanings among old CC tables. So we leave our TransactionType
					field value as NULL. Note: In the past, IsApproved would be NULL if the transaction was not approved. I'm keeping with 
					historic behavior there.
					// (j.jones 2015-09-30 10:56) - PLID 67180 - added signature, passed in as strExtraText
				*/

				// (j.jones 2015-09-30 10:56) - PLID 67180 - strSignature should be 'NULL' if empty (such as when validating)
				CString strSignature = strExtraText;
				if (strSignature.IsEmpty()) {
					strSignature = "NULL";
				}
				CString strHasSignature = "Convert(bit, 0)";
				if (strSignature.CompareNoCase("NULL") != 0) {
					strHasSignature = "Convert(bit, 1)";
				}

				//we are using extraID as the paymentID
				CString strSql;
				strSql.Format(R"(
SELECT LocationsT.Name as LocName, LocationsT.Address1 as LocAddress1, LocationsT.Address2 as LocAddress2, LocationsT.City as LocCity,
LocationsT.State as LocState, LocationsT.Zip as LocZip, LocationsT.Phone as LocPhone,
CASE WHEN Len(PaymentPlansT.CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(PaymentPlansT.CCNumber, 4) END  as CCNumber, 
PaymentPlansT.CCHoldersName, PaymentPlansT.CCAuthNo AS AuthCode, CreditCardNamesT.CardName, TipsQ.TotalTipAmount, 
ISNULL( ISNULL( Chase_CreditTransactionsT.TotalAmount, QBMS_CreditTransactionsT.TotalAmount), CardConnect_CreditTransactionT.AuthAmount) AS TransactionAmount, 
ISNULL( ISNULL( Chase_CreditTransactionsT.IsApproved, QBMS_CreditTransactionsT.IsApproved), CASE WHEN CardConnect_CreditTransactionT.ID IS NULL THEN NULL ELSE CONVERT(BIT,1) END) AS IsApproved, 
ISNULL( ISNULL( Chase_CreditTransactionsT.TransactionType, QBMS_CreditTransactionsT.TransactionType), NULL) AS TransactionType, 
ISNULL( ISNULL( Chase_CreditTransactionsT.LocalApprovalDateTime, QBMS_CreditTransactionsT.LocalApprovalDateTime), CardConnect_CreditTransactionT.Date) AS LocalApprovalDateTime, 
LineItemT.InputDate, LineItemT.Description, LineItemT.Date, LineItemT.InputName, LineItemT.Amount AS PayAmount, 
PersonT.First, PersonT.Middle, PersonT.Last, %s AS Signature, %s AS HasSignature
FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID 
INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID 
LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID 
LEFT JOIN QBMS_CreditTransactionsT ON PaymentsT.ID = QBMS_CreditTransactionsT.ID 
LEFT JOIN Chase_CreditTransactionsT ON PaymentsT.ID = Chase_CreditTransactionsT.ID 
LEFT JOIN CardConnect_CreditTransactionT ON PaymentsT.ID = CardConnect_CreditTransactionT.ID
LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID 
LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID 
LEFT JOIN  
	(SELECT PaymentTipsT.PaymentID, SUM(PaymentTipsT.Amount) AS TotalTipAmount 
	FROM PaymentTipsT 
	GROUP BY PaymentID 
	) TipsQ  
ON PaymentsT.ID = TipsQ.PaymentID 
WHERE LineItemT.ID = %li
)" 
, strSignature, strHasSignature, nExtraID);
				return _T(strSql);
			}
		break;
	
		case 704:	
		{
				/*Glasses Order 
				Version History:
				// (s.dhole 2011-03-21 18:56) - PLID 42898 Create  Glasses Order report 
				TES 4/11/2011 - PLID 43058 - Added Provider and Date
				//(e.lally 2011-05-31) PLID 43840 - Added IsContactLensOrder, LensQuantity, LensOrderNotes
				// (s.dhole 2011-06-17 11:22) - PLID 44160  - Added RxExpirationDate
				// (b.spivey, November 15, 2011) - PLID 44918 - Added RightSecondaryPrism and LeftSecondaryPrism
				// (s.dhole 2011-12-08 13:12) - PLID 46883 added LeftEyeSecondarPrismAxis ,LeftEyeSecondarPrismAxisStr	, RightEyeSecondaryPrismAxis	, RightEyeSecondaryPrismAxisStr	
				// (s.dhole 2012-02-20 13:05) - PLID 48249 location can be optional
				//(s.dhole 2012-02-08 17:18) - PLID 48033 Supplier can be optional
				// r.wilson 3-5-2012 PLID 43702 -> Added 'ShipToPatient' Field
				// r.wilson 4-11-2012 PLID 43741 -> Changed 'vwOrderT.GlassesOrderStatus AS GlassesOrderStatusID' to 'vwOrderT.GlassesOrderStatusID AS GlassesOrderStatusID'
				// (s.dhole 2012-05-08 10:06) - PLID  50131 Added InvoiceNo
				// (s.dhole 2012-05-21 16:35) - PLID 50547 Remove LensQuantity, LensOrderNotes	 Added 
				// Added OpticianLastName, OpticianFirstName, OpticianMiddleName, OpticianFullName,  SelectedInsurance,	SelectedInsuranceType	
				// Add  UnitPrice, Quantity, ItemPrice, ItemDiscount,ItemPriceAfterDiscount	, PatientRespAmt,VisionRespAmt, UnitCost
				// (s.dhole 2012-05-24 10:29) - PLID 48249 Location is required
				// (s.dhole 2012-06-05 12:33) - PLID 50777 Added Category to subrport 0 and rename  VisionRespAmt to  InsuranceRespAmt (Subreport 3)
				// (s.dhole 2012-06-14 12:33) - PLID 50929 Replacing Replacing  statment with " CASE WHEN vwOrderT.GlassesOrderType = 3 THEN CONVERT(BIT, 1)   ELSE CONVERT(BIT, 0) END AS IsContactLensOrder, "
					CASE WHEN vwOrderT.ContactLensOrderInfoID IS NULL THEN CONVERT(BIT, 0)   ELSE CONVERT(BIT, 1) END AS IsContactLensOrder, "					
				// (j.gruber 2012-07-25 14:34) - PLID 51779 - more insurance categories
				*/
			switch  (nSubLevel) 
			{
				case 0: 
				{	//Main Report 
					//the main report is the Order

					CString strSql;

					strSql.Format(" SELECT vwOrderT.ID, vwOrderT.VisionWebOrderExchangeID, vwOrderT.LocationID, vwOrderT.PersonID AS PatientID, vwOrderT.GlassesOrderType,   "
					" CASE vwOrderT.GlassesOrderType WHEN 1 THEN 'Spectacle Lens Order' WHEN 2 THEN 'Frame Order' WHEN 3 THEN 'Contact Lens Patient' WHEN 4 THEN 'Contact Lens Office Order'  "
					" WHEN 5 THEN 'Spectacle Lens Frame Order' END AS OrderType, vwOrderT.OrderCreateDate, vwOrderT.GlassesSupplierLocationID,   "
					" vwOrderT.GlassesCatalogFrameTypeID, vwOrderT.SupplierID, vwOrderT.GlassesOrderStatusID, vwOrderT.GlassesMessage AS OrderMessage,   "
					" vwOrderT.MessageType, vwOrderT.GlassesJobType AS JobType,   "
					" CASE WHEN vwOrderT.GlassesJobType = 'FTC' THEN 'Frame To Come' WHEN vwOrderT.GlassesJobType = 'UNC' THEN 'Uncut' WHEN vwOrderT.GlassesJobType = 'RED'  "
					" THEN 'Lenses Only' WHEN vwOrderT.GlassesJobType = 'TBP' THEN 'To Be Purchased' END AS JobTypeDescription, vwOrderT.GlassesJobNote AS JobNote,   "
					" PersonT_Supplier.Location AS SupplierLocation, PersonT_Supplier.WorkPhone AS SupplierWorkPhone, PersonT_Supplier.Fax AS SupplierFax,   "
					" PersonT_Supplier.Company AS SupplierCompanyName, PersonT_Supplier.Address1 AS SupplierAddress1, PersonT_Supplier.Address2 AS SupplierAddress2,   "
					" PersonT_Supplier.City AS SupplierCity, PersonT_Supplier.State AS SupplierState, PersonT_Supplier.Zip AS SupplierZip, PersonT.BirthDate AS PatientBirthDate,   "
					" vwOrderT.GlassesOrderNumber, vwOrderT.SupplierTrackingId, vwOrderT.Description, UsersT.UserName, LocationsT.Name AS LocationName,   "
					" LocationsT.Address1 AS LocationAddress1, LocationsT.Address2 AS LocationAddress2, LocationsT.City AS LocationCity,   "
					" LocationsT.State AS LocationState, LocationsT.Zip AS LocationsZip, LocationsT.Phone AS LocationMainPhone,   "
					" LocationsT.Phone2 AS LocationsPhone, LocationsT.Fax AS LocationsFax, GlassesSupplierLocationsT.VisionWebAccountID,   "
					" PatientsT.UserDefinedID AS PatientUserDefinedID, PersonT.First AS PatientFirstName, PersonT.Middle AS PatientMiddleName,   "
					" PersonT.Last AS PatientLastName, PersonT.Address1 AS PatientAddress1, PersonT.Address2 AS PatientAddress2, PersonT.City AS PatientCity,   "
					" PersonT.State AS PatientState, PersonT.Zip AS PatientZip, PersonT.HomePhone AS PatientHomePhone, PersonT.WorkPhone AS PatientWorkPhone,   "
					" RightLensRxT.RightLensDetailRxID, LeftLensRxT.LeftLensDetailRxID, LeftLensDetailRxT.FarHalfPd AS LeftEyeFarHalfPd,   "
					" LeftLensDetailRxT.NearHalfPd AS LeftEyeNearHalfPd, LeftLensDetailRxT.SegHeight AS LeftEyeSegHeight, LeftLensDetailRxT.OpticalCenter AS LeftEyeOpticalCenter,   "
					" LeftLensDetailRxT.PrescriptionSphere AS LeftEyePrescriptionSphere, LeftLensDetailRxT.CylinderValue AS LeftEyeCylinderValue,   "
					" LeftLensDetailRxT.CylinderAxis AS LeftEyeCylinderAxis, LeftLensDetailRxT.AdditionValue AS LeftEyeAdditionValue,   "
					" LeftLensDetailRxT.PrismValue AS LeftEyePrismValue, LeftLensDetailRxT.PrismAxis AS LeftEyePrismAxis, LeftLensDetailRxT.PrismAxisStr AS LeftEyePrismAxisStr,   "
					" LeftvwoOtherInfoT.GlassesCatalogDesignsID AS LeftEyeDesignsID, LeftvwoOtherInfoT.GlassesCatalogMaterialsID AS LeftEyeMaterialsID,   "
					" vwOrderT.LeftGlassesOrderOtherInfoID AS LeftEyeOtherInfoID, LeftvwoOtherInfoT.ThicknessValue AS LeftEyeLensThickness,   "
					" CASE WHEN LeftvwoOtherInfoT.ThicknessValue IS NULL THEN NULL ELSE LeftvwoOtherInfoT.ThicknessType END AS LeftEyeThicknessType,   "
					" LeftvwoOtherInfoT.TreatmentsComment AS LeftEyeTreatmentComment, LeftvwcDesignsT.DesignName AS LeftEyeDesignName,   "
					" LeftvwcDesignsT.DesignCode AS LeftEyeDesignCode, LeftvwcMaterialsT.MaterialName AS LeftEyeMaterialName,   "
					" LeftvwcMaterialsT.MaterialCode AS LeftEyeMaterialCode, vwOrderT.RightGlassesOrderOtherInfoID AS RightEyeOtherInfoID,   "
					" RightvwoOtherInfoT.ThicknessValue AS RightEyeThickness, CASE WHEN RightvwoOtherInfoT.ThicknessValue IS NULL THEN NULL   "
					" ELSE RightvwoOtherInfoT.ThicknessType END AS RightEyeThicknessType, RightvwoOtherInfoT.TreatmentsComment AS RightEyeTreatmentComment,   "
					" RightvwcDesignsT.DesignName AS RightEyeDesignName, RightvwcDesignsT.DesignCode AS RightEyeDesignCode,   "
					" RightvwcMaterialsT.MaterialName AS RightEyeMaterialName, RightvwcMaterialsT.MaterialCode AS RightEyeMaterialCode,   "
					" RightLensDetailRxT.FarHalfPd AS RightEyeFarHalfPd, RightLensDetailRxT.NearHalfPd AS RightEyeNearHalfPd,   "
					" RightLensDetailRxT.SegHeight AS RightEyeSegHeight, RightLensDetailRxT.OpticalCenter AS RightEyeOpticalCenter,   "
					" RightLensDetailRxT.PrescriptionSphere AS RightEyePrescriptionSphere, RightLensDetailRxT.CylinderValue AS RightEyeCylinderValue,   "
					" RightLensDetailRxT.CylinderAxis AS RightEyeCylinderAxis, RightLensDetailRxT.AdditionValue AS RightEyeAdditionValue,   "
					" RightLensDetailRxT.PrismValue AS RightEyePrismValue, RightLensDetailRxT.PrismAxis AS RightEyePrismAxis,   "
					" RightLensDetailRxT.PrismAxisStr AS RightEyePrismAxisStr, SupplierT.VisionWebID, RightvwoOtherInfoT.GlassesCatalogMaterialsID AS RightEyeMaterialID,   "
					" RightvwoOtherInfoT.GlassesCatalogDesignsID AS RightEyecDesignsID, vwOrderT.LensRxID, vwOrderT.ShapeA AS BoxA, vwOrderT.ShapeB AS BoxB,   "
					" vwOrderT.ShapeED AS ED, vwOrderT.ShapeHalfDbl, vwOrderT.GlassesFramesDataID, GlassesFramesDataT.FPC, GlassesFramesDataT.StyleName,   "
					" GlassesFramesDataT.ColorDescription, GlassesFramesDataT.Eye, GlassesFramesDataT.Bridge, GlassesFramesDataT.Temple,   "
					" GlassesFramesDataT.ManufacturerName, GlassesFramesDataT.BrandName, GlassesFramesDataT.SKU,   "
					" GlassesCatalogFrameTypesT.FrameTypeName, GlassesCatalogFrameTypesT.FrameTypeCode, vwOrderT.Date,   "
					" PersonProvider.Last + ', ' + PersonProvider.First + ' ' + PersonProvider.Middle AS Provider, CASE WHEN LeftLensRxT.LeftLensDetailRxID IS NULL   "
					" THEN RightLensRxT.RxDate ELSE LeftLensRxT.RxDate END AS LensRxDate, "
					" CASE WHEN vwOrderT.GlassesOrderType = 3 THEN CONVERT(BIT, 1)   ELSE CONVERT(BIT, 0) END AS IsContactLensOrder, "
					" CASE WHEN LeftLensRxT.LeftLensDetailRxID IS NULL   "
					" THEN RightLensRxT.RxExpirationDate ELSE LeftLensRxT.RxExpirationDate END AS RxExpirationDate,   "
					" LeftLensDetailRxT.SecondaryPrismValue AS LeftEyeSecondaryPrismValue, RightLensDetailRxT.SecondaryPrismValue AS RightEyeSecondaryPrismValue,   "
					" LeftLensDetailRxT.PrismAxis2 AS LeftEyeSecondarPrismAxis, LeftLensDetailRxT.PrismAxisStr2 AS LeftEyeSecondarPrismAxisStr,   "
					" RightLensDetailRxT.PrismAxis2 AS RightEyeSecondarPrismAxis, RightLensDetailRxT.PrismAxisStr2, vwOrderT.ShipToPatient, vwOrderT.InvoiceNo,   "
					" ProviderOptician.Last AS OpticianLastName, ProviderOptician.First AS OpticianFirstName, ProviderOptician.Middle AS OpticianMiddleName,   "
					" ISNULL(ProviderOptician.Last, '') + ', ' + ISNULL(ProviderOptician.First, '') + ' ' + ISNULL(ProviderOptician.Middle, '') AS OpticianFullName,   "
					" InsuranceCoT.Name AS SelectedInsurance, RespTypeT.TypeName AS SelectedInsuranceType  "
					" FROM         PersonT AS PersonProvider RIGHT OUTER JOIN  "
					" PersonT AS ProviderOptician RIGHT OUTER JOIN  "
					" GlassesOrderT AS vwOrderT INNER JOIN  "
					" PersonT ON vwOrderT.PersonID = PersonT.ID LEFT OUTER JOIN  "
					" SupplierT ON vwOrderT.SupplierID = SupplierT.PersonID LEFT OUTER JOIN  "
					" PersonT AS PersonT_Supplier ON SupplierT.PersonID = PersonT_Supplier.ID INNER JOIN  "
					" UsersT ON vwOrderT.UserID = UsersT.PersonID INNER JOIN  "
					" LocationsT ON vwOrderT.LocationID = LocationsT.ID INNER JOIN  "
					" PatientsT ON vwOrderT.PersonID = PatientsT.PersonID ON ProviderOptician.ID = vwOrderT.OpticianID ON   "
					" PersonProvider.ID = vwOrderT.ProviderID LEFT OUTER JOIN  "
					" RespTypeT INNER JOIN  "
					" InsuredPartyT ON RespTypeT.ID = InsuredPartyT.RespTypeID RIGHT OUTER JOIN  "
					" InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID ON vwOrderT.InsuredPartyID = InsuredPartyT.PersonID LEFT OUTER JOIN  "
					" LensDetailRxT AS RightLensDetailRxT INNER JOIN  "
					" LensRxT AS RightLensRxT ON RightLensDetailRxT.ID = RightLensRxT.RightLensDetailRxID ON vwOrderT.LensRxID = RightLensRxT.ID LEFT OUTER JOIN  "
					" LensRxT AS LeftLensRxT INNER JOIN  "
					" LensDetailRxT AS LeftLensDetailRxT ON LeftLensRxT.LeftLensDetailRxID = LeftLensDetailRxT.ID ON vwOrderT.LensRxID = LeftLensRxT.ID LEFT OUTER JOIN  "
					" GlassesSupplierLocationsT ON vwOrderT.GlassesSupplierLocationID = GlassesSupplierLocationsT.ID LEFT OUTER JOIN  "
					" GlassesCatalogFrameTypesT ON vwOrderT.GlassesCatalogFrameTypeID = GlassesCatalogFrameTypesT.ID LEFT OUTER JOIN  "
					" GlassesFramesDataT ON vwOrderT.GlassesFramesDataID = GlassesFramesDataT.ID LEFT OUTER JOIN  "
					" GlassesCatalogDesignsT AS LeftvwcDesignsT RIGHT OUTER JOIN  "
					" GlassesCatalogMaterialsT AS LeftvwcMaterialsT RIGHT OUTER JOIN  "
					" GlassesOrderOtherInfoT AS LeftvwoOtherInfoT ON LeftvwcMaterialsT.ID = LeftvwoOtherInfoT.GlassesCatalogMaterialsID ON   "
					" LeftvwcDesignsT.ID = LeftvwoOtherInfoT.GlassesCatalogDesignsID ON vwOrderT.LeftGlassesOrderOtherInfoID = LeftvwoOtherInfoT.ID LEFT OUTER JOIN  "
					" GlassesCatalogDesignsT AS RightvwcDesignsT RIGHT OUTER JOIN  "
					" GlassesOrderOtherInfoT AS RightvwoOtherInfoT ON RightvwcDesignsT.ID = RightvwoOtherInfoT.GlassesCatalogDesignsID LEFT OUTER JOIN  "
					" GlassesCatalogMaterialsT AS RightvwcMaterialsT ON RightvwoOtherInfoT.GlassesCatalogMaterialsID = RightvwcMaterialsT.ID ON   "
					" vwOrderT.RightGlassesOrderOtherInfoID = RightvwoOtherInfoT.ID  "
					" WHERE     (vwOrderT.IsDelete = 0) AND (vwOrderT.GlassesOrderType = 1)  AND  vwOrderT.ID = %li", nExtraID);  

					return _T(strSql);
				}
				break;
				case 1:  
				{
					switch (nSubRepNum) 
					{

						case 0: 
						{
							//Insurance
							CString strSql;
							strSql.Format( " SELECT InsuranceCoT.Name, RespTypeT.TypeName AS ResponsibilityType, \r\n"
							" CASE WHEN Priority IN (1,2) THEN 'Medical' WHEN Priority = -1 THEN '' ELSE  "
							" CASE WHEN CategoryType = 2 THEN 'Vision' "
							" WHEN CategoryType = 3 THEN 'Auto' "
							" WHEN CategoryType = 4 THEN 'Workers'' Comp.' "
							" WHEN CategoryType = 5 THEN 'Dental' "
							" WHEN CategoryType = 6 THEN 'Study' "
							" WHEN CategoryType = 7 THEN 'Letter of Protection' "
							" WHEN CategoryType = 8 THEN 'Letter of Agreement' "
							" ELSE 'Medical' END END AS Category \r\n"
							" FROM InsuranceCoT INNER JOIN \r\n"
							" InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID INNER JOIN \r\n"
							" RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID \r\n"
							" INNER JOIN GlassesOrderT ON InsuredPartyT.personid = dbo.GlassesOrderT.InsuredPartyID  \r\n"
							" WHERE    GlassesOrderT.ID = %li", nExtraID);  
							 //strExtraText is an optional resource filter
							return _T(strSql);
						}
						break;
						case 1: 
						{
							//Left Treatment
							CString strSql;
							strSql.Format(" SELECT   GlassesCatalogTreatmentsT.TreatmentName,  \r\n"
								"   GlassesCatalogTreatmentsT.TreatmentCode \r\n"
								"  FROM GlassesOrderTreatmentsT AS vwoT INNER JOIN  \r\n"
								"  GlassesCatalogTreatmentsT ON vwoT.GlassesCatalogTreatmentID = GlassesCatalogTreatmentsT.ID INNER JOIN  \r\n"
								"  GlassesOrderOtherInfoT ON vwoT.GlassesOrderOtherInfoID = GlassesOrderOtherInfoT.ID INNER JOIN  \r\n"
								"  GlassesOrderT ON GlassesOrderOtherInfoT.ID = GlassesOrderT.RightGlassesOrderOtherInfoID  \r\n"
								"  WHERE     (GlassesOrderT.ID =  %li)", nExtraID );

							return _T(strSql);
						}
						break;
						case 2: 
						{
							//right Treatment
							CString strSql;
							strSql.Format( " SELECT GlassesCatalogTreatmentsT.TreatmentName,  \r\n"
								"   GlassesCatalogTreatmentsT.TreatmentCode \r\n"
								"  FROM GlassesOrderTreatmentsT AS vwoT INNER JOIN  \r\n"
								"  GlassesCatalogTreatmentsT ON vwoT.GlassesCatalogTreatmentID = GlassesCatalogTreatmentsT.ID INNER JOIN  \r\n"
								"  GlassesOrderOtherInfoT ON vwoT.GlassesOrderOtherInfoID = GlassesOrderOtherInfoT.ID INNER JOIN  \r\n"
								"  GlassesOrderT ON GlassesOrderOtherInfoT.ID = GlassesOrderT.LeftGlassesOrderOtherInfoID  \r\n"
								"  WHERE     (GlassesOrderT.ID =  %li)", nExtraID );
							return _T(strSql);
						}
						break;
				
						case 3: 
						{
							//Procedures Treatment
							//(e.lally 2011-05-31) PLID 43840 - Added ServiceQuantity
							CString strSql;
							strSql.Format( " SELECT GlassesOrderServiceT.GlassesOrderID AS  CLServiceID, CPTCodeT.Code,   "
								" ISNULL(ServiceT.Name,'') AS ItemName ,   "
								" GlassesOrderServiceT.Price AS UnitPrice,GlassesOrderServiceT.Quantity, CONVERT(money,    "
								" GlassesOrderServiceT.Price * GlassesOrderServiceT.Quantity, 2) AS ItemPrice, CONVERT(money, ISNULL(ChargeDiscountQ.DiscountTotal, 0), 2)    "
								" AS ItemDiscount, CONVERT(money, GlassesOrderServiceT.Price * GlassesOrderServiceT.Quantity, 2) - CONVERT(money,    "
								" ISNULL(ChargeDiscountQ.DiscountTotal, 0), 2) AS ItemPriceAfterDiscount ,   "
								" GlassesOrderServiceT.PatientRespAmt, GlassesOrderServiceT.VisionRespAmt AS InsuranceRespAmt,GlassesOrderServiceT.OpticalLineItemCost AS UnitCost   "
								" FROM GlassesOrderServiceT INNER JOIN   "
								" ServiceT ON GlassesOrderServiceT.ServiceID = ServiceT.ID LEFT OUTER JOIN   "
								" CPTCodeT ON ServiceT.ID = CPTCodeT.ID LEFT OUTER JOIN   "
								" (SELECT GlassesOrderServiceDiscountsT.GlassesOrderServiceID, SUM(ROUND(CONVERT(money,    "
								" GlassesOrderServiceT_1.Price * GlassesOrderServiceT_1.Quantity), 2) - ROUND(CONVERT(money,    "
								" GlassesOrderServiceT_1.Price * GlassesOrderServiceT_1.Quantity * (CASE WHEN ([PercentOff] IS NULL) THEN 1 ELSE ((100 - CONVERT(float,    "
								" [PercentOff])) / 100) END) - (CASE WHEN [Discount] IS NULL THEN 0 ELSE [Discount] END)), 2)) AS DiscountTotal   "
								" FROM GlassesOrderServiceT AS GlassesOrderServiceT_1 INNER JOIN   "
								" GlassesOrderServiceDiscountsT ON GlassesOrderServiceT_1.ID = GlassesOrderServiceDiscountsT.GlassesOrderServiceID   "
								" GROUP BY GlassesOrderServiceDiscountsT.GlassesOrderServiceID) AS ChargeDiscountQ ON    "
								" ChargeDiscountQ.GlassesOrderServiceID = GlassesOrderServiceT.ID "
								" WHERE GlassesOrderServiceT.GlassesOrderID = %li", nExtraID );
							return _T(strSql);
						}
						break;
						
						default:
							return _T("");
						break;
					}					
				}
				break;
				default:
				{
					return _T("");
				}
				break;
			}
		}
		break;


// (r.wilson 7/10/2012) PLID 51423 - This report will no longer be used. It was replaced by report 737
	
	//	case 706:
	//		{
				/*Eye Prescription Report
				Version History:
					// (s.dhole 2011-05-03 17:43) - PLID 42953 new Glasses RX  report
					Added  RightPrismAxisStr, LeftEyePrismAxisStr 
					// (s.dhole 2011-06-17 11:22) - PLID 44160  - Added RxExpirationDate
					// (b.spivey, November 15, 2011) - PLID 44918 - Added RightSecondaryPrism and LeftSecondaryPrism
					// (s.dhole 2011-12-08 13:12) - PLID 46883 added LeftEyeSecondarPrismAxis ,LeftEyeSecondarPrismAxisStr	, RightEyeSecondaryPrismAxis	, RightEyeSecondaryPrismAxisStr	
					// (s.dhole 2012-02-20 13:05) - PLID 48249 location can be optional
					// (s.dhole 2012-05-24 10:32) - PLID 48249 location is Required
					
				*/
				/*
				CString strSql;
				strSql.Format("SELECT LensRxT.ID, LensRxT.PersonID, LensRxT.LeftLensDetailRxID, "
					" LensRxT.RightLensDetailRxID, LensRxT.RxDate, RightLensDetailRxT.ID AS RightLensRxID, "
					" RightLensDetailRxT.FarHalfPd AS RightEyeFarHalfPd, RightLensDetailRxT.NearHalfPd AS RightEyeNearPd, " 
					" RightLensDetailRxT.SegHeight AS RightEyeSegHeight,  "
					" RightLensDetailRxT.PrescriptionSphere AS RightEyeSphere, RightLensDetailRxT.CylinderValue AS RightCylinder,  "
					" RightLensDetailRxT.CylinderAxis AS RightCylinderAxis, RightLensDetailRxT.AdditionValue AS RightAddition,  "
					" RightLensDetailRxT.PrismValue AS RightPrism, RightLensDetailRxT.PrismAxis AS RightPrismAxis,    "
					" LeftLensDetailRxT.FarHalfPd AS LeftEyeFarHalfPd, LeftLensDetailRxT.NearHalfPd AS LeftEyeNearPd,  "
					" LeftLensDetailRxT.SegHeight AS LeftEyeSegHeight,  "
					" LeftLensDetailRxT.PrescriptionSphere AS LeftEyeSphere, LeftLensDetailRxT.CylinderValue AS LeftCylinder, LeftLensDetailRxT.CylinderAxis AS LeftCylinderAxis,   "
					" LeftLensDetailRxT.AdditionValue AS LeftAddition, LeftLensDetailRxT.PrismValue AS LeftPrism, LeftLensDetailRxT.PrismAxis AS LeftPrismAxis, "
					" PatientsT.UserDefinedID, PersonT.First AS PatientFirstName,  "
					" PersonT.Middle AS PatientMiddleName, PersonT.Last AS PatientLastName, PersonT.Address1 AS PatientAddress1,  "
					" PersonT.Address2 AS PatientAddress2, PersonT.City AS PatientCity, PersonT.State AS PatientState, PersonT.Zip AS PatientZip,  "
					" PersonT.Gender AS PatientGender, PersonT.HomePhone AS PatientHomePhone, PersonT.WorkPhone AS PatientWorkPhone,  "
					" PersonT.Extension AS PatientExtension, PersonT.CellPhone AS PatientCellPhone, PersonT.BirthDate AS PatientBirthDate,  "
					" PersonT.SocialSecurity AS PatientSocialSecurity, GlassesOrderT.ProviderID, Providerdetail.First AS ProviderFirstName,  "
					" Providerdetail.Middle AS ProviderMiddleName, Providerdetail.Last AS ProviderLastName, PrefixT.Prefix AS ProviderPrefix, Providerdetail.Title AS ProviderTitle,  "
					" LocationsT.Name AS Location, LocationsT.Address1 AS LocationAddress1, LocationsT.Address2 AS LocationAddress2,  "
					" LocationsT.City AS LocationCity, LocationsT.State AS LocationState, LocationsT.Zip AS LocationZip, LocationsT.Phone AS LocationPhone,  "
					" LocationsT.Fax AS LocationFax ,RightLensDetailRxT.PrismAxisStr AS RightPrismAxisStr, LeftLensDetailRxT.PrismAxisStr AS LeftEyePrismAxisStr, "
					" LensRxT.RxExpirationDate  AS RxExpirationDate, "
					" RightLensDetailRxT.SecondaryPrismValue AS RightSecondaryPrism, LeftLensDetailRxT.SecondaryPrismValue AS LeftSecondaryPrism, "
					" LeftLensDetailRxT.PrismAxis2 AS LeftEyeSecondarPrismAxis, LeftLensDetailRxT.PrismAxisStr2 AS LeftEyeSecondarPrismAxisStr,  "   
					" RightLensDetailRxT.PrismAxis2 AS RightEyeSecondarPrismAxis, RightLensDetailRxT.PrismAxisStr2 AS LeftEyeSecondarPrismAxisStr  "   
					"FROM         dbo.PersonT AS Providerdetail INNER JOIN "
                   " ProvidersT ON Providerdetail.ID =  ProvidersT.PersonID INNER JOIN "
                   " LocationsT  INNER JOIN "
                   " LensRxT INNER JOIN "
                   " PatientsT ON  LensRxT.PersonID =  PatientsT.PersonID INNER JOIN "
                   " PersonT ON  PatientsT.PersonID =  PersonT.ID INNER JOIN "
                   " GlassesOrderT ON  LensRxT.ID =  GlassesOrderT.LensRxID ON   "
				   " LocationsT.ID =  GlassesOrderT.LocationID ON  "
                   " ProvidersT.PersonID =  GlassesOrderT.ProviderID LEFT OUTER JOIN "
                   " LensDetailRxT AS LeftLensDetailRxT ON  LensRxT.LeftLensDetailRxID = LeftLensDetailRxT.ID LEFT OUTER JOIN "
                   " LensDetailRxT AS RightLensDetailRxT ON  LensRxT.RightLensDetailRxID = RightLensDetailRxT.ID LEFT OUTER JOIN "
                  "  PrefixT ON Providerdetail.PrefixID = PrefixT.ID " 
					" WHERE LensRxT.ID = %li", nExtraID);
				return _T(strSql);
			}
		break;

		*/

		case 729:	
		{
				/*Contact Lens Order 
				Version History:
				// (s.dhole 2012-05-21 11:25) - PLID 50531 Create Contact Lens Order 
				// (s.dhole 2012-06-05 12:33) - PLID 50777 Added Category to subrport 0 and rename  VisionRespAmt to  InsuranceRespAmt (Subreport 1)
				// (s.dhole 2012-06-14 12:33) - PLID 50929 Replacing Replacing  statment with " CASE WHEN vwOrderT.GlassesOrderType = 3 THEN CONVERT(BIT, 1)   ELSE CONVERT(BIT, 0) END AS IsContactLensOrder, "
				// Remove order note and provider
					" CASE WHEN vwOrderT.ContactLensOrderInfoID IS NULL THEN CONVERT(BIT, 0) ELSE CONVERT(BIT, 1) END AS IsContactLensOrder,  "
				// (j.gruber 2012-07-25 14:38) - PLID 51779 - support more insurance resps
				// (j.dinatale 2013-04-12 11:42) - PLID 55862 - include the Doc Ins fields for both lenses
				*/
			switch  (nSubLevel) 
			{
				case 0: 
				{	//Main Report 
					//the main report is the Order			
					CString strSql;
					strSql.Format( " SELECT vwOrderT.ID, vwOrderT.LocationID, vwOrderT.PersonID AS PatientID, vwOrderT.InvoiceNo, vwOrderT.OrderCreateDate, vwOrderT.GlassesSupplierLocationID,  "
					" vwOrderT.GlassesCatalogFrameTypeID, vwOrderT.SupplierID,vwOrderT.GlassesMessage AS OrderMessage , vwOrderT.GlassesOrderStatusID,   "
					" PersonT_Supplier.Location AS SupplierLocation, PersonT_Supplier.WorkPhone AS SupplierWorkPhone, PersonT_Supplier.Fax AS SupplierFax,  "
					" PersonT_Supplier.Company AS SupplierCompanyName, PersonT_Supplier.Address1 AS SupplierAddress1, PersonT_Supplier.Address2 AS SupplierAddress2,  "
					" PersonT_Supplier.City AS SupplierCity, PersonT_Supplier.State AS SupplierState, PersonT_Supplier.Zip AS SupplierZip, PersonT.BirthDate AS PatientBirthDate,  "
					" vwOrderT.SupplierTrackingId, vwOrderT.Description, UsersT.UserName, LocationsT.Name AS LocationName, LocationsT.Address1 AS LocationAddress1,  "
					" LocationsT.Address2 AS LocationAddress2, LocationsT.City AS LocationCity, LocationsT.State AS LocationState, LocationsT.Zip AS LocationsZip,  "
					" LocationsT.Phone AS LocationMainPhone, LocationsT.Phone2 AS LocationsPhone, LocationsT.Fax AS LocationsFax,  "
					" PatientsT.UserDefinedID AS PatientUserDefinedID, PersonT.First AS PatientFirstName, PersonT.Middle AS PatientMiddleName,  "
					" PersonT.Last AS PatientLastName, PersonT.Address1 AS PatientAddress1, PersonT.Address2 AS PatientAddress2, PersonT.City AS PatientCity,  "
					" PersonT.State AS PatientState, PersonT.Zip AS PatientZip, PersonT.HomePhone AS PatientHomePhone, PersonT.WorkPhone AS PatientWorkPhone,  "
					" RightLensRxT.RightLensDetailRxID, LeftLensRxT.LeftLensDetailRxID, LeftLensDetailRxT.PrescriptionSphere AS LeftEyePrescriptionSphere,  "
					" LeftLensDetailRxT.CylinderValue AS LeftEyeCylinderValue, LeftLensDetailRxT.CylinderAxis AS LeftEyeCylinderAxis, LeftLensDetailRxT.BC AS LeftLensBC,  "
					" LeftLensDetailRxT.Diameter AS LeftLensDiameter, LeftLensDetailRxT.Color AS LeftLensColor, LeftLensDetailRxT.Note AS LeftLensNote,  "
					" vwOrderT.LeftGlassesOrderOtherInfoID AS LeftEyeOtherInfoID, vwOrderT.RightGlassesOrderOtherInfoID AS RightEyeOtherInfoID,  "
					" RightLensDetailRxT.PrescriptionSphere AS RightEyePrescriptionSphere, RightLensDetailRxT.CylinderValue AS RightEyeCylinderValue,  "
					" RightLensDetailRxT.CylinderAxis AS RightEyeCylinderAxis, RightLensDetailRxT.BC AS RightLensBC, RightLensDetailRxT.Diameter AS RightLensDiameter,  "
					" RightLensDetailRxT.Color AS RightLensColor, RightLensDetailRxT.Note AS RightLensNote, RightLensDetailRxT.AdditionValue AS RightEyeAdditionValue,  "
					" vwOrderT.LensRxID, vwOrderT.Date, "
					" CASE WHEN LeftLensRxT.LeftLensDetailRxID IS NULL THEN RightLensRxT.RxDate ELSE LeftLensRxT.RxDate END AS LensRxDate,  "
					" CASE WHEN vwOrderT.GlassesOrderType = 3 THEN CONVERT(BIT, 1)   ELSE CONVERT(BIT, 0) END AS IsContactLensOrder, "
					" CASE WHEN LeftLensRxT.LeftLensDetailRxID IS NULL THEN RightLensRxT.RxExpirationDate ELSE LeftLensRxT.RxExpirationDate END AS RxExpirationDate,  "
					" CASE WHEN LeftLensRxT.LeftLensDetailRxID IS NULL THEN RightLensRxT.RxIssueDate ELSE LeftLensRxT.RxIssueDate END AS RxIssueDate,  "
					" vwOrderT.ShipToPatient, ProviderOptician.First AS OpticianFirstName, ProviderOptician.Middle AS OpticianMiddleName ,"
					" ISNULL(ProviderOptician.Last, '') + ', ' + ISNULL(ProviderOptician.First, '') + ' ' + ISNULL(ProviderOptician.Middle, '') AS OpticianFullName , "
					" ProviderOptician.Last AS OpticianLastName,  "
					" PersonProvider.First AS ProviderFirstName, PersonProvider.Middle AS ProviderMiddleName, PersonProvider.Last AS ProviderLastName,  "
					" ISNULL(PersonProvider.Last, '') + ', ' + ISNULL(PersonProvider.First, '') + ' ' + ISNULL(PersonProvider.Middle, '') AS ProviderFullName , "
					" LeftLensDetailRxT.AdditionValue AS LeftEyeAdditionValue, InsuranceCoT.Name AS SelectedInsurance , RespTypeT.TypeName  AS SelectedInsuranceType, "
					" LeftLensDetailRxT.DocIns AS LeftDocIns, RightLensDetailRxT.DocIns AS RightDocIns "
					" FROM InsuranceCoT INNER JOIN  "
					" InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID INNER JOIN  "
					" RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID RIGHT OUTER JOIN  "
					" GlassesOrderT AS vwOrderT INNER JOIN  "
					" PersonT ON vwOrderT.PersonID = PersonT.ID INNER JOIN  "
					" UsersT ON vwOrderT.UserID = UsersT.PersonID INNER JOIN  "
					" PatientsT ON vwOrderT.PersonID = PatientsT.PersonID ON InsuredPartyT.PersonID = vwOrderT.InsuredPartyID LEFT OUTER JOIN  "
					" PersonT AS PersonT_Supplier ON vwOrderT.SupplierID = PersonT_Supplier.ID LEFT OUTER JOIN  "
					" PersonT AS ProviderOptician ON vwOrderT.OpticianID = ProviderOptician.ID LEFT OUTER JOIN  "
					" PersonT AS PersonProvider ON vwOrderT.ProviderID = PersonProvider.ID LEFT OUTER JOIN  "
					" LocationsT ON vwOrderT.LocationID = LocationsT.ID LEFT OUTER JOIN  "
					" LensDetailRxT AS RightLensDetailRxT INNER JOIN  "
					" LensRxT AS RightLensRxT ON RightLensDetailRxT.ID = RightLensRxT.RightLensDetailRxID ON vwOrderT.LensRxID = RightLensRxT.ID LEFT OUTER JOIN  "
					" LensRxT AS LeftLensRxT INNER JOIN  "
					" LensDetailRxT AS LeftLensDetailRxT ON LeftLensRxT.LeftLensDetailRxID = LeftLensDetailRxT.ID ON vwOrderT.LensRxID = LeftLensRxT.ID  "
					" WHERE (vwOrderT.IsDelete = 0) AND (vwOrderT.GlassesOrderType = 3) AND vwOrderT.ID = %li", nExtraID);  			
					return _T(strSql);
				}
				break;
				case 1:  
				{
					switch (nSubRepNum) 
					{
						
						case 0: 
						{
							//Insurance
							CString strSql;
							strSql.Format( " SELECT InsuranceCoT.Name, RespTypeT.TypeName AS ResponsibilityType, \r\n"
							" CASE WHEN Priority IN (1,2) THEN 'Medical' WHEN Priority = -1 THEN '' ELSE "
							" CASE WHEN CategoryType = 2 THEN 'Vision' "
							" WHEN CategoryType = 3 THEN 'Auto' "
							" WHEN CategoryType = 4 THEN 'Workers'' Comp.' "
							" WHEN CategoryType = 5 THEN 'Dental' "
							" WHEN CategoryType = 6 THEN 'Study' "
							" WHEN CategoryType = 7 THEN 'Letter of Protection' "
							" WHEN CategoryType = 8 THEN 'Letter of Agreement' "
							" ELSE 'Medical' END END AS Category \r\n"
							" FROM InsuranceCoT INNER JOIN \r\n"
							" InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID INNER JOIN \r\n"
							" RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID \r\n"
							" INNER JOIN GlassesOrderT ON InsuredPartyT.personid = dbo.GlassesOrderT.InsuredPartyID  \r\n"
							" WHERE  GlassesOrderT.ID = %li", nExtraID);  
							 //strExtraText is an optional resource filter
							return _T(strSql);
						}
						break;

						case 1: 
						{
							//Service Items
							CString strSql;
							strSql.Format( " SELECT GlassesOrderServiceT.GlassesOrderID AS  CLServiceID, CPTCodeT.Code,   "
							" ISNULL(ServiceT.Name,'') AS ItemName ,   "
							" GlassesOrderServiceT.Price AS UnitPrice,GlassesOrderServiceT.Quantity, CONVERT(money,    "
							" GlassesOrderServiceT.Price * GlassesOrderServiceT.Quantity, 2) AS ItemPrice, CONVERT(money, ISNULL(ChargeDiscountQ.DiscountTotal, 0), 2)    "
							" AS ItemDiscount, CONVERT(money, GlassesOrderServiceT.Price * GlassesOrderServiceT.Quantity, 2) - CONVERT(money,    "
							" ISNULL(ChargeDiscountQ.DiscountTotal, 0), 2) AS ItemPriceAfterDiscount ,   "
							" GlassesOrderServiceT.PatientRespAmt, GlassesOrderServiceT.VisionRespAmt AS InsuranceRespAmt ,GlassesOrderServiceT.OpticalLineItemCost AS UnitCost "
							" FROM GlassesOrderServiceT INNER JOIN   "
							" ServiceT ON GlassesOrderServiceT.ServiceID = ServiceT.ID LEFT OUTER JOIN   "
							" CPTCodeT ON ServiceT.ID = CPTCodeT.ID LEFT OUTER JOIN   "
							" (SELECT GlassesOrderServiceDiscountsT.GlassesOrderServiceID, SUM(ROUND(CONVERT(money,    "
							" GlassesOrderServiceT_1.Price * GlassesOrderServiceT_1.Quantity), 2) - ROUND(CONVERT(money,    "
							" GlassesOrderServiceT_1.Price * GlassesOrderServiceT_1.Quantity * (CASE WHEN ([PercentOff] IS NULL) THEN 1 ELSE ((100 - CONVERT(float,    "
							" [PercentOff])) / 100) END) - (CASE WHEN [Discount] IS NULL THEN 0 ELSE [Discount] END)), 2)) AS DiscountTotal   "
							" FROM GlassesOrderServiceT AS GlassesOrderServiceT_1 INNER JOIN   "
							" GlassesOrderServiceDiscountsT ON GlassesOrderServiceT_1.ID = GlassesOrderServiceDiscountsT.GlassesOrderServiceID   "
							" GROUP BY GlassesOrderServiceDiscountsT.GlassesOrderServiceID) AS ChargeDiscountQ ON    "
							" ChargeDiscountQ.GlassesOrderServiceID = GlassesOrderServiceT.ID "
							" WHERE   GlassesOrderServiceT.GlassesOrderID = %li Order BY GlassesOrderServiceT.IsDefaultProduct DESC,bODEye DESC ,bOSEye DESC ", nExtraID);  
							 //strExtraText is an optional resource filter
							return _T(strSql);
						}
						break;
						
						default:
							return _T("");
						break;
					}					
				}
				break;
				default:
				{
					return _T("");
				}
				break;
			}
		}
		break;


		case 737:
			{
				/*Optical Prescription Report
				Version History:
				(r.wilson 5/18/2012) PLID 48952 - Created Report
				*/
			/*	
				CString strSql;
				strSql.Format("SELECT LensRxT.ID, LensRxT.PersonID, LensRxT.LeftLensDetailRxID,  "
			"  LensRxT.RightLensDetailRxID, LensRxT.RxDate, RightLensDetailRxT.ID AS RightLensRxID,  "
			"  RightLensDetailRxT.FarHalfPd AS RightEyeFarHalfPd, RightLensDetailRxT.NearHalfPd AS RightEyeNearPd,  "
			"  RightLensDetailRxT.SegHeight AS RightEyeSegHeight,   "
			"  RightLensDetailRxT.PrescriptionSphere AS RightEyeSphere, RightLensDetailRxT.CylinderValue AS RightCylinder,   "
			"  RightLensDetailRxT.CylinderAxis AS RightCylinderAxis, RightLensDetailRxT.AdditionValue AS RightAddition,   "
			"  RightLensDetailRxT.PrismValue AS RightPrism, RightLensDetailRxT.PrismAxis AS RightPrismAxis,     "
			"  LeftLensDetailRxT.FarHalfPd AS LeftEyeFarHalfPd, LeftLensDetailRxT.NearHalfPd AS LeftEyeNearPd,   "
			"  LeftLensDetailRxT.SegHeight AS LeftEyeSegHeight,   "
			"  LeftLensDetailRxT.PrescriptionSphere AS LeftEyeSphere, LeftLensDetailRxT.CylinderValue AS LeftCylinder, LeftLensDetailRxT.CylinderAxis AS LeftCylinderAxis,    "
			"  LeftLensDetailRxT.AdditionValue AS LeftAddition, LeftLensDetailRxT.PrismValue AS LeftPrism, LeftLensDetailRxT.PrismAxis AS LeftPrismAxis,  "
			"  PatientsT.UserDefinedID, PersonT.First AS PatientFirstName,   "
			"  PersonT.Middle AS PatientMiddleName, PersonT.Last AS PatientLastName, PersonT.Address1 AS PatientAddress1,   "
			"  PersonT.Address2 AS PatientAddress2, PersonT.City AS PatientCity, PersonT.State AS PatientState, PersonT.Zip AS PatientZip,   "
			"  PersonT.Gender AS PatientGender, PersonT.HomePhone AS PatientHomePhone, PersonT.WorkPhone AS PatientWorkPhone,   "
			"  PersonT.Extension AS PatientExtension, PersonT.CellPhone AS PatientCellPhone, PersonT.BirthDate AS PatientBirthDate,   "
			"  PersonT.SocialSecurity AS PatientSocialSecurity, GlassesOrderT.ProviderID, Providerdetail.First AS ProviderFirstName,   "
			"  Providerdetail.Middle AS ProviderMiddleName, Providerdetail.Last AS ProviderLastName, PrefixT.Prefix AS ProviderPrefix, Providerdetail.Title AS ProviderTitle,   "
			"  LocationsT.Name AS Location, LocationsT.Address1 AS LocationAddress1, LocationsT.Address2 AS LocationAddress2,   "
			"  LocationsT.City AS LocationCity, LocationsT.State AS LocationState, LocationsT.Zip AS LocationZip, LocationsT.Phone AS LocationPhone,   "
			"  LocationsT.Fax AS LocationFax ,RightLensDetailRxT.PrismAxisStr AS RightPrismAxisStr, LeftLensDetailRxT.PrismAxisStr AS LeftEyePrismAxisStr,  "
			"  LensRxT.RxExpirationDate  AS RxExpirationDate,  "
			"  RightLensDetailRxT.SecondaryPrismValue AS RightSecondaryPrism, LeftLensDetailRxT.SecondaryPrismValue AS LeftSecondaryPrism,  "
			"  LeftLensDetailRxT.PrismAxis2 AS LeftEyeSecondarPrismAxis, LeftLensDetailRxT.PrismAxisStr2 AS LeftEyeSecondarPrismAxisStr,   "
			"  RightLensDetailRxT.PrismAxis2 AS RightEyeSecondarPrismAxis, RightLensDetailRxT.PrismAxisStr2 AS LeftEyeSecondarPrismAxisStr, "*/
			//"  /*Contact Lens*/ "
			/*"  ConLensLensRxT, ConLensDetailRxID,  "
			"  ConLensRightLensDetailRxID, ConLensRxDate, ConLensRightLensRxID,  "
			"  ConLensRightEyeFarHalfPd, ConLensRightEyeNearPd,  "
			"  ConLensRightEyeSegHeight,   "
			"  ConLensRightEyeSphere, ConLensRightCylinder,   "
			"  ConLensRightCylinderAxis, ConLensRightAddition,   "
			"  ConLensRightPrism, ConLensRightPrismAxis,     "
			"  ConLensLeftEyeFarHalfPd, ConLensLeftEyeNearPd,   "
			"  ConLensLeftEyeSegHeight,   "
			"  ConLensLeftEyeSphere, ConLensLeftCylinder, ConLensLeftCylinderAxis,    "
			"  ConLensLeftAddition, ConLensLeftPrism, ConLensLeftPrismAxis,  "
			"  ConLensRightPrismAxisStr, ConLensLeftEyePrismAxisStr,  "
			"  ConLensRxExpirationDate,  "
			"  ConLensRightSecondaryPrism, ConLensLeftSecondaryPrism,  "
			"  ConLensLeftEyeSecondarPrismAxis, ConLensPrismAxisStr2,   "
			"  ConLensRightEyeSecondarPrismAxis, ConLensLeftEyeSecondarPrismAxisStr     "
			"FROM         dbo.PersonT AS Providerdetail INNER JOIN  "
			" ProvidersT ON Providerdetail.ID =  ProvidersT.PersonID RIGHT OUTER JOIN  "
			" LocationsT RIGHT OUTER JOIN  "
			" LensRxT INNER JOIN  "
			" PatientsT ON  LensRxT.PersonID =  PatientsT.PersonID INNER JOIN  "
			" PersonT ON  PatientsT.PersonID =  PersonT.ID INNER JOIN  "
			" GlassesOrderT ON  LensRxT.ID =  GlassesOrderT.LensRxID ON    "
			" LocationsT.ID =  GlassesOrderT.LocationID ON   "
			" ProvidersT.PersonID =  GlassesOrderT.ProviderID LEFT OUTER JOIN  "
			" LensDetailRxT AS LeftLensDetailRxT ON  LensRxT.LeftLensDetailRxID = LeftLensDetailRxT.ID LEFT OUTER JOIN  "
			" LensDetailRxT AS RightLensDetailRxT ON  LensRxT.RightLensDetailRxID = RightLensDetailRxT.ID LEFT OUTER JOIN  "
			" PrefixT ON Providerdetail.PrefixID = PrefixT.ID  "
			" LEFT JOIN  "
			" ( "
			" SELECT PatientsT.PersonID as ConLensPersonID, LensRxT.ID AS ConLensLensRxT, LensRxT.LeftLensDetailRxID AS ConLensDetailRxID,  "
			"  LensRxT.RightLensDetailRxID AS ConLensRightLensDetailRxID, LensRxT.RxDate AS ConLensRxDate, RightLensDetailRxT.ID AS ConLensRightLensRxID,  "
			"  RightLensDetailRxT.FarHalfPd AS ConLensRightEyeFarHalfPd, RightLensDetailRxT.NearHalfPd AS ConLensRightEyeNearPd,  "
			"  RightLensDetailRxT.SegHeight AS ConLensRightEyeSegHeight,   "
			"  RightLensDetailRxT.PrescriptionSphere AS ConLensRightEyeSphere, RightLensDetailRxT.CylinderValue AS ConLensRightCylinder,   "
			"  RightLensDetailRxT.CylinderAxis AS ConLensRightCylinderAxis, RightLensDetailRxT.AdditionValue AS ConLensRightAddition,   "
			"  RightLensDetailRxT.PrismValue AS ConLensRightPrism, RightLensDetailRxT.PrismAxis AS ConLensRightPrismAxis,     "
			"  LeftLensDetailRxT.FarHalfPd AS ConLensLeftEyeFarHalfPd, LeftLensDetailRxT.NearHalfPd AS ConLensLeftEyeNearPd,   "
			"  LeftLensDetailRxT.SegHeight AS ConLensLeftEyeSegHeight,   "
			"  LeftLensDetailRxT.PrescriptionSphere AS ConLensLeftEyeSphere, LeftLensDetailRxT.CylinderValue AS ConLensLeftCylinder, LeftLensDetailRxT.CylinderAxis AS ConLensLeftCylinderAxis,    "
			"  LeftLensDetailRxT.AdditionValue AS ConLensLeftAddition, LeftLensDetailRxT.PrismValue AS ConLensLeftPrism, LeftLensDetailRxT.PrismAxis AS ConLensLeftPrismAxis,  "
			"  RightLensDetailRxT.PrismAxisStr AS ConLensRightPrismAxisStr, LeftLensDetailRxT.PrismAxisStr AS ConLensLeftEyePrismAxisStr,  "
			"  LensRxT.RxExpirationDate  AS ConLensRxExpirationDate,  "
			"  RightLensDetailRxT.SecondaryPrismValue AS ConLensRightSecondaryPrism, LeftLensDetailRxT.SecondaryPrismValue AS ConLensLeftSecondaryPrism,  "
			"  LeftLensDetailRxT.PrismAxis2 AS ConLensLeftEyeSecondarPrismAxis, LeftLensDetailRxT.PrismAxisStr2 AS ConLensPrismAxisStr2,   "
			"  RightLensDetailRxT.PrismAxis2 AS ConLensRightEyeSecondarPrismAxis, RightLensDetailRxT.PrismAxisStr2 AS ConLensLeftEyeSecondarPrismAxisStr   "
			"FROM          "
			" LensRxT INNER JOIN  GlassesOrderT ON  LensRxT.ID =  GlassesOrderT.LensRxID LEFT OUTER JOIN "
			" LensDetailRxT AS LeftLensDetailRxT ON  LensRxT.LeftLensDetailRxID = LeftLensDetailRxT.ID LEFT OUTER JOIN  "
			" LensDetailRxT AS RightLensDetailRxT ON  LensRxT.RightLensDetailRxID = RightLensDetailRxT.ID LEFT OUTER JOIN   "
			" PatientsT ON  LensRxT.PersonID =  PatientsT.PersonID  "
			" WHERE LensRxT.ID = %li "
			" ) AS ConLensQ ON ConlensQ.ConLensPersonID = LensRxT.PersonID "			
			" WHERE LensRxT.ID = %li "
			""
			, nExtraID,nExtraID);*/
			
			// use addhack methode to pass tem table SQL to Cryatal report
			return strListBoxSQL;
			

			/*CString strSql;
			strSql = 
				"SELECT '-1' AS ID, '15970' AS PersonID, '-1' AS LeftLensDetailRxID, '-1' AS RightLensDetailRXID, 'RxDate' AS RxDate, '-1' AS RightLensRxID, 'R(1/2)PH' AS RightLensFarHalfPd,  '' AS RightEyeNearPd, 'R-Eye-Seg-Height' AS RightEyeSegHeight, '' AS RightEyeSphere, '' AS RightCylinder, 'Right-Cyl-Axis' AS RightCylinderAxis , '' AS RightAddition, '' AS RightPrism,  '' AS RightPrismAxis, 'L-Far-PD' AS LeftEyeFarHalfPd, '' AS LeftEyeNearPd, 'L-Eye-Seg-Height' AS LeftEyeSegHeight, '' AS LeftEyeSphere, '' AS LeftCylinder, 'Left-Cyl-Axis' AS LeftCylinderAxis,   '' AS LeftPrismAxis, '2635' AS UserDefinedID, 'Caroline' AS PatientFirstName, '' AS PatientMiddleName, 'Matlock' AS PatientLastName, '4096 Lowes St' AS PatientAddress1, '' AS PatientAddress2,  'Pawtucket' AS PatientCity, 'RI' AS PatientState, '02860' AS PatientZip, '2' AS PatientGender, '(316)513-2600' AS PatientHomePhone, '' AS PatientWorkPhone, '' AS PatientExtension,  '(877)227-2383' AS PatientCellPhone, 'Invalid DateTime' AS PatientBirthDate, '984-73-4934' AS PatientSocialSecurity, '17578' AS ProviderID, 'Steve' AS ProviderFirstName, '' AS ProviderMiddleName,  'Demko' AS ProviderLastName, 'Dr.' AS ProviderPrefix, 'MD' AS ProviderTitle, 'Gulf Coast Eye Surgery' AS Location, '1850 Bluegrass Avenue' AS LocationAddress1, ' ' AS LocationAddress2, 'Louisville' AS LocationCity,  'KY' AS LocationState, '40215' AS LocationZip, '(502) 502-5252' AS LocationPhone, '(502) 502-5252' AS LocationFax, '' AS RightPrismAxisStr, '' AS LeftPrismAxisStr, 'RxExpirationDate' AS RxExpirationDate,  '' AS RightSecondaryPrism, '' AS LeftSecondaryPrism, 'L-EYE-Pri-Axis' AS LeftEyeSecondaryPrismAxis, 'LeftEyeSecondaryPrismAxis-STR' AS LeftEyeSecondaryPrismAxisStr, 'RightEyeSecondaryPrismAxis' AS RightEyeSecondaryPrismAxis, 'RightEyeSecondaryPrismAxis-STR' AS RightEyeSecondaryPrismAxisStr, '-1' AS ConLensLeftLensDetailRxID, '-1' AS ConLensRightLensDetailRXID, 'RxDate' AS ConLensRxDate, '-1' AS ConLensRightLensRxID, '' AS ConLensRightDiameter, '+0.50' AS ConLensRightEyeSphere, '-1.75' AS ConLensRightCylinder, '80' AS ConLensRightCylinderAxis , '' AS ConLensRightAddition,  '' AS ConLensLeftDiameter, '+1.00' AS ConLensLeftEyeSphere, '' AS ConLensLeftCylinder, '0' AS ConLensLeftAxis, '' AS ConLensLeftAddition , 'RxExpirationDate' AS ConLensRxExpirationDate ";
				return strSql;*/
			}
		break;


		default:
			return GetMoreSqlPrintPreview(nSubLevel, nSubRepNum);
			break;
		}
}

CString CReportInfo::GetMoreSqlPrintPreview(long nSubLevel, long nSubRepNum) const
{
	CString strSQL, strArSql, strPatientID;
		
	// (f.dinatale 2010-10-15) - PLID 40876 - SSN Masking Permissions
	BOOL bSSNReadPermission = CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE);
	BOOL bSSNDisableMasking = CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE);

		switch (nID) {

			case 608: //package quote
			{
				/*Version History:
				// (j.gruber 2007-08-13 14:54) - PLID 25851 - Quote just for packages*/
				// (j.gruber 2008-08-27 17:28) - PLID 31185 - fix the quote to not show inventory items marked as facility fees as facility fees and not inv items
				// (j.gruber 2009-04-02 11:47) - PLID 33358 - updated for new discount structure
				// (j.jones 2009-09-22 17:28) - PLID 35196 - added primary and secondary insurance allowed amounts
				// (j.jones 2009-10-23 10:09) - PLID 36011 - added insurance IDs
				// (j.gruber 2009-11-25 14:37) - PLID 36438 - add itemcode
				switch (nSubLevel) {

					case 0: //Main Report 
					{
			
					// (a.walling 2010-03-02 10:33) - PLID 37129 - Converted NTEXT MemoParam to NVARCHAR(4000) for the purposes of this report
					CString strSql;
					strSql.Format("SELECT PackagesT.QuoteID as QuoteID, PackagesT.TotalCount, PackagesT.TotalAmount, PackagesT.CurrentCount, PackagesT.CurrentAmount, PackagesT.Type, PackagesT.OriginalCurrentAmount, PackagesT.OriginalCurrentCount, "
						" LineItemT.Amount, TotalPercentOff as PercentOff, TotalDiscount as Discount, ChargesT.TaxRate, ChargesT.TaxRate2, ChargesT.Quantity,  "
						" ChargesT.PackageQtyRemaining, chargesT.OriginalPackageQtyRemaining,  "
						" LineItemT.Description as LineDescription, BillsT.Description as QuoteDescription, BillsT.PatientID,  "
						" LineItemT.Date As TDate, ChargesT.OthrBillFee, "
						" ServiceT.ProcedureDescription, "
						" CASE WHEN Convert(nvarchar, ServiceT.ProcedureDescription) = ''  OR ServiceT.ProcedureDescription IS NULL THEN 1 ELSE 0 END AS SuppressDesc, "
						"  "
						" PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, "
						" PersonT.State as PatState, PersonT.Zip as PatZip, PatientsT.UserdefinedID as PatID, "
						" ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast,  "
						" PatCoordT.First as PatCoordFirst, PatCoordT.Middle as PatCoordMiddle, PatCoordT.Last as PatCoordLast, "

						" LocationsT.Name as LocName, LocationsT.Address1 as LocAdd1, LocationsT.Address2 as LocAdd2, LocationsT.City as LocCity, "
						" LocationsT.State as LocState, LocationsT.Zip as LocZip, LocationsT.Phone as LocPhone, LocationsT.Fax As LocFax, LocationsT.OnlineAddress as LocWebsite, "
						"  "
						" POST.Name as PosName, POST.Address1 as PosAdd1, POST.Address2 as PosAdd2, POST.City as PosCity, POST.State as PosState, "
						" POST.Zip as PosZip, POST.Phone as POSTPhone, POST.Fax as POSTFax, POST.OnlineAddress as POSTEmail, "
						" POST.AnesthesiaPayTo, POST.FacilityPayTo, "
						"  "
						" CASE WHEN CPTCodeT.ID IS NULL then 0 else 1 END as IsCPTCode, "
						" CASE WHEN ProductT.ID IS NULL then 0 ELSE CASE WHEN ServiceT.FacilityFee = 0 then 1 ELSE 0 END END as IsProduct, "
						" CASE WHEN ServiceT.Anesthesia = 0 then 0 else 1 END AS IsAnesthesia, "
						" CASE WHEN ServiceT.FacilityFee = 0 then 0 ELSE 1 END as IsFacFee, "
						" "
						"  CASE WHEN CPTMultiplier1 IS NULL then 1 ELSE CPTMultiplier1 END AS CPTMultiplier1, "
						" CASE WHEN CPTMultiplier2 IS NULL then 1 ELSE CPTMultiplier2 END AS CPTMultiplier2, "
						" CASE WHEN CPTMultiplier3 IS NULL then 1 ELSE CPTMultiplier3 END AS CPTMultiplier3, "
						" CASE WHEN CPTMultiplier4 IS NULL then 1 ELSE CPTMultiplier4 END AS CPTMultiplier4, "
						" (SELECT CONVERT(NVARCHAR(4000), ConfigRT.MemoParam) AS MemoParam FROM ConfigRT WHERE ConfigRT.Name = 'QuoteTopText') AS TopText,  "
						" (SELECT CONVERT(NVARCHAR(4000), ConfigRT.MemoParam) AS MemoParam FROM ConfigRT WHERE ConfigRT.Name = 'QuoteBottomText') AS BottomText,  "
						" '" + _Q(GetRemotePropertyText("QuoteReportTitle", "Surgery Proposal", 0, "<None>", true)) + "' AS ReportTitle, '" + _Q(GetRemotePropertyText("QuoteSubReportTitle", "Surgery and Related Services", 0, "<None>", true)) + "' AS SubReportTitle, "
						" %s as Tax1Total, %s as Tax2Total, "
						" CASE WHEN PrimaryInsuredPartyQ.InsuranceCoID Is Null THEN Null ELSE dbo.GetChargeAllowableForInsuranceCo(ChargesT.ID, PrimaryInsuredPartyQ.InsuranceCoID) END AS PrimaryInsuranceAllowable, "
						" CASE WHEN SecondaryInsuredPartyQ.InsuranceCoID Is Null THEN Null ELSE dbo.GetChargeAllowableForInsuranceCo(ChargesT.ID, SecondaryInsuredPartyQ.InsuranceCoID) END AS SecondaryInsuranceAllowable, "
						" PrimaryInsuredPartyQ.IDForInsurance AS PrimaryIDForInsurance, PrimaryInsuredPartyQ.PolicyGroupNum AS PrimaryPolicyGroupNum, "
						" SecondaryInsuredPartyQ.IDForInsurance AS SecondaryIDForInsurance, SecondaryInsuredPartyQ.PolicyGroupNum AS SecondaryPolicyGroupNum, "
						" ChargesT.ItemCode as CPTCode "
						" "
						" FROM PackagesT INNER JOIN BillsT ON PackagesT.QuoteID = BillsT.ID "
						" LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
						" LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
						" LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						" LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
						" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
						" LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
						" LEFT JOIN PersonT ProvT ON ChargesT.DoctorsProviders = ProvT.ID "
						" LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
						" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
						" LEFT JOIN PersonT PatCoordT ON ChargesT.PatCoordID = PatCoordT.ID "
						" LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
						" LEFT JOIN LocationsT POST ON BillsT.Location = POST.ID "
						" LEFT JOIN (SELECT PatientID, InsuranceCoID, IDForInsurance, PolicyGroupNum FROM InsuredPartyT WHERE RespTypeID = 1) AS PrimaryInsuredPartyQ ON PatientsT.PersonID = PrimaryInsuredPartyQ.PatientID "
						" LEFT JOIN (SELECT PatientID, InsuranceCoID, IDForInsurance, PolicyGroupNum FROM InsuredPartyT WHERE RespTypeID = 2) AS SecondaryInsuredPartyQ ON PatientsT.PersonID = SecondaryInsuredPartyQ.PatientID "
						" WHERE LineItemT.Deleted = 0 AND BillsT.Deleted = 0 AND BillsT.EntryType = 2 AND LineItemT.Type = 11 "
						// (z.manning 2008-12-19 12:49) - PLID 32538 - We need to use FormatCurrencyForSql, not
						// FormatCurrencyForInterface
						, FormatCurrencyForSql(CalculateOriginalPackageValueWithTax(nExtraID, 1))
						, FormatCurrencyForSql(CalculateOriginalPackageValueWithTax(nExtraID, 2))
						);

						return _T(strSql);
			}
			break;

			case 1:  

				switch (nSubRepNum) {
					case 0: //linked prepays
						{
							CString str;
							str.Format("SELECT PrePaysQ.Description, PrePaysQ.Amount, PrePaysQ.TDate as TDate, "
								" PrePaysQ.IDate as IDate, PrePaysQ.QuoteID AS QuoteID "
								" FROM "
								" (SELECT    LineItemT.Description,   " 
								" (CASE WHEN PrepayAppliedToQ.ID IS NULL THEN    "
								"     (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount END)   "
								" ELSE   "
								"     (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END) AS Amount,   "
								"     LineItemT.Date AS TDate, LineItemT.InputDate AS IDate, "
								"     PaymentsT.QuoteID as QuoteID "
								" FROM PaymentsT INNER JOIN   "
								"     (LineItemT LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID) ON PaymentsT.ID = LineItemT.ID INNER JOIN   "
								"     PatientsT ON    "
								"     LineItemT.PatientID = PatientsT.PersonID INNER JOIN   "
								"     PersonT ON PatientsT.PersonID = PersonT.ID LEFT JOIN   "
								"     PersonT PersonT1 ON    "
								"     PaymentsT.ProviderID = PersonT1.ID LEFT OUTER JOIN   "
								"     AppliesT AppliesT1 ON    "
								"     LineItemT.ID = AppliesT1.DestID LEFT OUTER JOIN   "
								"     AppliesT ON    "
								"     LineItemT.ID = AppliesT.SourceID LEFT OUTER JOIN   "
								"     PaymentPlansT ON    "
								"     PaymentsT.ID = PaymentPlansT.ID   "
								" LEFT JOIN   "
								" /* This will total everything applied to a prepayment */   "
								" ( SELECT SUM( AppliesT.Amount * -1 ) AS Amount, AppliesT.DestID AS ID   "
								" FROM   "
								" LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
								" LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.DestID   "
								" LEFT JOIN LineItemT LineItemT1 ON AppliesT.SourceID = LineItemT1.ID   "
								" WHERE (LineItemT.Deleted = 0)  " 
								" GROUP BY AppliesT.DestID   "
								" ) PrepayAppliedToQ ON LineItemT.ID = PrepayAppliedToQ.ID   "
								" LEFT JOIN   "
								" /* This will total everything that the prepayment is applied to */   "
								" ( SELECT SUM(AppliesT.Amount ) AS Amount, AppliesT.SourceID AS ID   "
								" FROM  "
								" LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID   "
								" LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID   "
								" LEFT JOIN LineItemT LineItemT1 ON AppliesT.DestID = LineItemT1.ID   "
								" WHERE LineItemT.Deleted = 0   "
								" GROUP BY AppliesT.SourceID   "
								" ) PrepayAppliesQ ON LineItemT.ID = PrepayAppliesQ.ID   "
								"  WHERE (LineItemT.Deleted = 0)  "
								" AND (PaymentsT.Prepayment = 1)  "
								" AND  "
								" (CASE WHEN PrepayAppliedToQ.ID IS NULL THEN    "
								"      (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount END)   "
								" ELSE   "
								"     (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END) <> 0  "
								" GROUP BY LineItemT.ID, PatientsT.UserDefinedID, PatientsT.PersonID,    "
								"     PaymentsT.ProviderID, PaymentsT.PayMethod,    "
								"     LineItemT.Description,   "
								" (CASE WHEN PrepayAppliedToQ.ID IS NULL THEN    "
								"     (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount END)   "
								" ELSE   "
								"     (CASE WHEN PrepayAppliesQ.ID IS NULL THEN LineItemT.Amount-PrepayAppliedToQ.Amount ELSE LineItemT.Amount - PrepayAppliesQ.Amount-PrepayAppliedToQ.Amount END) END),   "
								"     LineItemT.Date, PaymentPlansT.CheckNo,    "
								"     PaymentPlansT.CreditCardID, LineItemT.InputDate,    "
								"     PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle,  "
								"      PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle,   "
								" LineItemT.LocationID,   "
								" LocationsT.Name, PaymentsT.QuoteID) PrePaysQ INNER JOIN BillsT ON PrePaysQ.QuoteID = BillsT.ID  ");
							return _T(str);
						}
						break;

						default:
							return "";
						break;
				}
				break;

				default:
					return "";
				break;
			}
		}
		break;

		case 627:
			/* Version History 
			// (j.gruber 2008-06-03 13:19) - PLID 25447 - Inquiries Report
			*/
			{

			CString strSql;
			strSql.Format("SELECT PatientsT.PersonID, UserDefinedID, First, Middle, Last, Email, Note, dbo.CalcProcInfoName(ProcInfoT.ID) as ProcList, ReferralID, "
				" ReferralSourceT.Name as RefSource, LocationsT.ID as LocID, LocationsT.Name as LocName, PersonT.InputDate, UsersT.UserName, PersonT.UserID as InputEmpID "
				" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				" LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
				" LEFT JOIN ProcInfoT ON PatientsT.PersonID = ProcInfoT.PatientID "
				" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
				" LEFT JOIN UsersT ON PersonT.UserID = UsersT.PersonID "
				" WHERE CurrentStatus = 4 %s", strExtraField);

			return _T(strSql);
			}
		break;	

		case 584: 
			{
			/* EMR Summary Screen Report
			Version History:   
			 // (j.gruber 2007-02-23 12:29) - PLID 24609 - Created
			 // (c.haag 2007-04-04 12:05) - PLID 25482 - We now get the allergy name from EmrDataT since AllergyT.Name has been depreciated
			 // (j.jones 2008-07-02 10:23) - PLID 30580 - completely gutted and rebuilt this report such that it is now
			 // a raw data dump from the EMR Summary's datalists
			 */
		
			if(strExtendedSql.IsEmpty()) {
				//maybe this was called from the ttx creator?
				return _T("SELECT ID, LeftText, RightText, IsLeftHeader, IsRightHeader, IsDivider FROM "
					"(SELECT Convert(int, 1) AS ID, Convert(ntext, '') AS LeftText, Convert(ntext, '') AS RightText, "
					"Convert(bit, 0) AS IsLeftHeader, Convert(bit, 0) AS IsRightHeader, Convert(bit, 0) AS IsDivider "
					") AS EMRSummaryQ "
					"ORDER BY ID");
			}
			else {
				//the EMR Summary will always send strExtendedSql at runtime, as the temp table name
				CString strSql;
				strSql.Format("SELECT ID, LeftText, RightText, IsLeftHeader, IsRightHeader, IsDivider FROM "
					"(SELECT ID, LeftText, RightText, IsLeftHeader, IsRightHeader, IsDivider FROM %s) AS EMRSummaryQ "
					"ORDER BY ID ASC", strExtendedSql);
				return _T(strSql);
			}
			
			break;
			}


		case 652:	//Problem List
			{
			/*	Version History
				(e.lally 2008-07-23) PLID 30732 - Created - This is the same as the Problem List by Patient (651) so we may want to combine them into
					//a global function unless we forsee having different field needs.
				(e.lally 2008-07-30) PLID 30732 - Added EMR ID, EMR Date, EMN ID, Topic ID fields.
					- Had to move the bulk of the query inside its own subquery labeled as ProblemsQ so the additional
					  filters the dialog adds are compatible with the shared query building in the dialog itself
				(e.lally 2008-08-06) PLID 30965 - Added diagnosis code description to the code number to reflect the changes to the dialog. 
				// (j.jones 2009-05-26 08:49) - PLID 34151 - added problem diagnosis, chronicity, and supported multiple items per problem
				// (j.jones 2009-05-27 09:31) - PLID 34352 - supported lab problems
				// (j.jones 2009-06-02 09:47) - PLID 34441 - added ProblemID and RegardingTypeName
				// (j.jones 2014-03-26 13:55) - PLID 61413 - added ICD-10 support, which also removed and renamed the previous diagnosis fields
			*/
			CString strSql;
			strSql.Format("SELECT PatID, PatientName, EMRName, EmrID, EMRDate, EMNName, EmnID, EMNDate, "
				"TopicName, TopicID, ProblemItem, ProblemDescription, EnteredDate, OnsetDate, Status, ModifiedDate, UserModifiedBy, "
				"ICD9Code, ICD10Code, ICD9CodeDesc, ICD10CodeDesc, Chronicity, ProblemID, RegardingTypeName "
				"FROM "
				"(SELECT "
				"EMRProblemsT.ID AS ProblemID, PatientsT.PersonID AS PatID, "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, \r\n"
				//determine what object was flagged as a problem and grab the corresponding joined field
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMR*/ THEN EMRProblemGroup.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN EMRMasterGroup.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRTopicGroup.Description "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailGroup.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis code*/ THEN EMRDiagGroup.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Service code*/ THEN EMRChargeGroup.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Medication*/ THEN EMRMedicationGroup.Description "
				"ELSE '<None>' END AS EMRName, \r\n"
				""
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMR*/ THEN EMRProblemGroup.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN EMRMasterGroup.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRTopicGroup.ID "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailGroup.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis code*/ THEN EMRDiagGroup.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Service code*/ THEN EMRChargeGroup.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Medication*/ THEN EMRMedicationGroup.ID "
				"ELSE '' END AS EmrID, \r\n"
				""
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMR*/ THEN EMRProblemGroup.InputDate "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN EMRMasterGroup.InputDate "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRTopicGroup.InputDate "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailGroup.InputDate "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis code*/ THEN EMRDiagGroup.InputDate "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Service code*/ THEN EMRChargeGroup.InputDate "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Medication*/ THEN EMRMedicationGroup.InputDate "
				"ELSE '' END AS EMRDate, \r\n"
				""
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN EMRProblemMaster.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRTopicMaster.Description "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailMaster.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis code*/ THEN EMRDiagMaster.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Service code*/ THEN EMRChargeMaster.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Medication*/ THEN EMRMedicationMaster.Description "
				"ELSE '<None>' END AS EMNName, \r\n"
				""
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN EMRProblemMaster.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRTopicMaster.ID "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailMaster.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis code*/ THEN EMRDiagMaster.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Service code*/ THEN EMRChargeMaster.ID "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Medication*/ THEN EMRMedicationMaster.ID "
				"ELSE '' END AS EmnID, \r\n"
				""
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN EMRProblemMaster.Date "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRTopicMaster.Date "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailMaster.Date "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis code*/ THEN EMRDiagMaster.Date "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Service code*/ THEN EMRChargeMaster.Date "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Medication*/ THEN EMRMedicationMaster.Date "
				"ELSE NULL END AS EMNDate, \r\n"
				""
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRProblemTopic.Name "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailTopic.Name "
					//Topics for Diag code, Service Code, and Medication are always More Info so we are ignoring them
				"ELSE '' END AS TopicName, \r\n"
				""
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRProblemTopic.ID "
				"WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ OR EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/) "
				"	THEN EMRDetailTopic.ID "
					//Topics for Diag code, Service Code, and Medication are always More Info so we are ignoring them
				"ELSE '' END AS TopicID, \r\n"
				""
				//Get the name of whatever *it* is that was added as a problem.
				"CONVERT(NVARCHAR(2000), "
				//Labs use a special combination of data for their description
				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*Lab*/ THEN COALESCE(LabsT.FormNumberTextID, '') + '-' + COALESCE(LabsT.Specimen, '') + ' - ' + CASE WHEN LabsT.Type = %li THEN COALESCE(LabAnatomyT.Description, '') ELSE LabsT.ToBeOrdered END "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Patient*/ THEN PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle "
				//Since EMRs, EMNs, and Topics are just the name of those things themselves pick from one of those.
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMR*/ THEN EMRProblemGroup.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN EMRProblemMaster.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN EMRProblemTopic.Name "
				""
				//Or get the diagnosis code, Charge description, or medication name.
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis*/ THEN "
				"	CASE WHEN EMRDiagCodes9.ID Is Not Null AND EMRDiagCodes10.ID Is Not Null THEN EMRDiagCodes10.CodeNumber + ' - ' + EMRDiagCodes10.CodeDesc + ' (' + EMRDiagCodes9.CodeNumber + ')' "
				"	ELSE Coalesce(EMRDiagCodes10.CodeNumber + ' - ' + EMRDiagCodes10.CodeDesc, EMRDiagCodes9.CodeNumber + ' - ' + EMRDiagCodes9.CodeDesc) END "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*service code*/ THEN EMRChargesT.Description "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*medication*/ THEN DrugDataT.Data "
				//Details and Detail elements have that funky override field to worry about, otherwise grab their regular Name.
				"ELSE "
				"	CASE WHEN (EMRProblemLinkT.EMRRegardingType = %li /*Item*/ OR EMRProblemLinkT.EMRRegardingType = %li /*data item*/) THEN "
				"		CASE WHEN EmrDetailsT.MergeOverride IS NULL THEN ("
				"			CASE WHEN EMRInfoT.ID = -27 THEN EmrDetailsT.MacroName "
				"			ELSE EMRInfoT.Name END) "
				"		ELSE EmrDetailsT.MergeOverride END "
				"	ELSE '' END "
				"END) AS ProblemItem, \r\n"

				"CASE WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMR*/ THEN 'EMR' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*EMN*/ THEN 'EMN' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Topic*/ THEN 'Topic' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Detail*/ THEN 'EMR Item' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*DetailElement*/ THEN 'EMR List Item' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Diagnosis code*/ THEN 'Diagnosis Code' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Service code*/ THEN 'Charge' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Medication*/ THEN 'Medication' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Patient*/ THEN 'Patient' "
				"WHEN EMRProblemLinkT.EMRRegardingType = %li /*Lab*/ THEN 'Lab' "
				"ELSE '' END AS RegardingTypeName, \r\n"

				"EmrProblemsT.Description AS ProblemDescription, "
				"EmrProblemsT.EnteredDate AS EnteredDate, "
				"EmrProblemsT.OnsetDate AS OnsetDate, "
				"EMRProblemLinkT.EMRRegardingType, "
				"EMRProblemLinkT.EMRRegardingID, "
				"EmrProblemStatusT.ID AS StatusID, "
				"EmrProblemStatusT.Name as Status, "
				"EmrProblemsT.ModifiedDate AS ModifiedDate, "
				"UsersT.Username AS UserModifiedBy, "
				"ICD9T.CodeNumber as ICD9Code, "				
				"ICD10T.CodeNumber as ICD10Code, "
				"ICD9T.CodeDesc as ICD9CodeDesc, "
				"ICD10T.CodeDesc as ICD10CodeDesc, "
				"EMRProblemChronicityT.Name AS Chronicity "
				""
				"FROM EMRProblemsT  "
				//if multiple links exist, the problem will show on the report multiple times
				"INNER JOIN EMRProblemLinkT ON EMRProblemsT.ID = EMRProblemLinkT.EMRProblemID "
				"INNER JOIN PatientsT ON EMRProblemsT.PatientID = PatientsT.PersonID "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				//Find the most recent modified date for each problem ID
				"LEFT JOIN  (SELECT ProblemID, Max(Date) AS Date FROM EMRProblemHistoryT GROUP BY ProblemID) Max_EMRProblemHistoryT "
				"	ON EMRPRoblemsT.ID = Max_EMRProblemHistoryT.ProblemID "
				//rejoin the regular problem history table using the max modified date for each problem to get the rest of the fields.
				"LEFT JOIN EMRProblemHistoryT ON Max_EMRProblemHistoryT.Date = EMRProblemHistoryT.Date "
				"	AND Max_EMRProblemHistoryT.ProblemID = EMRProblemHistoryT.ProblemID "
				"LEFT JOIN EMRProblemStatusT ON EMRProblemHistoryT.StatusID = EMRProblemStatusT.ID "
				"LEFT JOIN EmrDetailsT ON EMRProblemLinkT.EMRRegardingID = EmrDetailsT.ID "
				"LEFT JOIN EMRInfoT ON EmrDetailsT.EmrInfoID = EmrInfoT.ID "
				"LEFT JOIN UsersT ON EMRProblemHistoryT.UserID = UsersT.PersonID "

				//If the Detail was flagged as a problem, we need to link the EMR/EMN on its ID
				"LEFT JOIN EMRTopicsT AS EMRDetailTopic ON EMRDetailsT.EMRTopicID = EMRDetailTopic.ID "
				"LEFT JOIN EMRMasterT AS EMRDetailMaster ON EMRDetailsT.EMRID = EMRDetailMaster.ID "
				"LEFT JOIN EMRGroupsT AS EMRDetailGroup ON EMRDetailMaster.EMRGroupID = EMRDetailGroup.ID "
				
				//If the Topic was flagged as a problem, we need to link the EMR/EMN on its ID
				"LEFT JOIN EMRTopicsT AS EMRProblemTopic ON EMRProblemLinkT.EMRRegardingID = EMRProblemTopic.ID "
				"LEFT JOIN EMRMasterT AS EMRTopicMaster ON EMRProblemTopic.EMRID = EMRTopicMaster.ID "
				"LEFT JOIN EMRGroupsT AS EMRTopicGroup ON EMRTopicMaster.EMRGroupID = EMRTopicGroup.ID "
				""
				//If the EMN was flagged as a problem, we need to link the EMR on its ID
				"LEFT JOIN EMRMasterT AS EMRProblemMaster ON EMRProblemLinkT.EMRRegardingID = EMRProblemMaster.ID "
				"LEFT JOIN EMRGroupsT AS EMRMasterGroup ON EMRProblemMaster.EMRGroupID = EMRMasterGroup.ID "
				
				//If the EMR was flagged as a problem, we need to link nothing (just itself)
				"LEFT JOIN EMRGroupsT AS EMRProblemGroup ON EMRProblemLinkT.EMRRegardingID = EMRProblemGroup.ID "
				
				//If the Diagnosis code was flagged as a problem, we need to link the EMR/EMN on its ID 
					//(we don't care about the more info topic)
				"LEFT JOIN EMRDiagCodesT ON EMRProblemLinkT.EMRRegardingID = EMRDiagCodesT.ID "
				"LEFT JOIN DiagCodes EMRDiagCodes9 ON EMRDiagCodes9.ID = EMRDiagCodesT.DiagCodeID "
				"LEFT JOIN DiagCodes EMRDiagCodes10 ON EMRDiagCodes10.ID = EMRDiagCodesT.DiagCodeID_ICD10 "
				"LEFT JOIN EMRMasterT AS EMRDiagMaster ON EMRDiagMaster.ID = EMRDiagCodesT.EMRID "
				"LEFT JOIN EMRGroupsT AS EMRDiagGroup ON EMRDiagGroup.ID = EMRDiagMaster.EMRGroupID "
				
				//If the service code was flagged as a problem, we need to link the EMR/EMN on its ID 
					//(we don't care about the more info topic)
				"LEFT JOIN EMRChargesT ON EMRProblemLinkT.EMRRegardingID = EMRChargesT.ID "
				"LEFT JOIN EMRMasterT AS EMRChargeMaster ON EMRChargeMaster.ID = EMRChargesT.EMRID "
				"LEFT JOIN EMRGroupsT AS EMRChargeGroup ON EMRChargeGroup.ID = EMRChargeMaster.EMRGroupID "
				
				//If the Medication was flagged as a problem, we need to link the EMR/EMN on its ID 
					//(we don't care about the more info topic)
				"LEFT JOIN EMRMedicationsT ON EMRProblemLinkT.EMRRegardingID = EMRMedicationsT.MedicationID "
				"LEFT JOIN PatientMedications ON EMRMedicationsT.MedicationID = PatientMedications.ID "
				"LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
				"LEFT JOIN EMRDataT AS DrugDataT ON DrugList.EMRDataID = DrugDataT.ID "
				"LEFT JOIN EMRMasterT AS EMRMedicationMaster ON EMRMedicationMaster.ID = EMRMedicationsT.EMRID "
				"LEFT JOIN EMRGroupsT AS EMRMedicationGroup ON EMRMedicationGroup.ID = EMRMedicationMaster.EMRGroupID "
				""
				//If a lab is flagged as a problem, link LabsT on its ID
				"LEFT JOIN LabsT ON EMRProblemLinkT.EMRRegardingID = LabsT.ID "
				"LEFT JOIN LabAnatomyT ON LabAnatomyT.ID = LabsT.AnatomyID "
				""
				//a problem itself can have a diagnosis code
				"LEFT JOIN DiagCodes ICD9T ON EMRProblemsT.DiagCodeID = ICD9T.ID "
				"LEFT JOIN DiagCodes ICD10T ON EMRProblemsT.DiagCodeID_ICD10 = ICD10T.ID "
				""
				"LEFT JOIN EMRProblemChronicityT ON EMRProblemsT.ChronicityID = EMRProblemChronicityT.ID "
				""
				"WHERE EMRProblemsT.Deleted = 0) ProblemsQ WHERE ProblemsQ.PatID > 0 %s ", 
				eprtEmrEMR, eprtEmrEMN, eprtEmrTopic, eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication,
				eprtEmrEMR, eprtEmrEMN, eprtEmrTopic, eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication,
				eprtEmrEMR, eprtEmrEMN, eprtEmrTopic, eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication,
				eprtEmrEMN, eprtEmrTopic, eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication,
				eprtEmrEMN, eprtEmrTopic, eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication,
				eprtEmrEMN, eprtEmrTopic, eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication,
				eprtEmrTopic, eprtEmrItem, eprtEmrDataItem,
				eprtEmrTopic, eprtEmrItem, eprtEmrDataItem,
				eprtLab, ltBiopsy, eprtUnassigned, eprtEmrEMR, eprtEmrEMN, eprtEmrTopic, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication, eprtEmrItem, eprtEmrDataItem,
				eprtEmrEMR, eprtEmrEMN, eprtEmrTopic, eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication, eprtUnassigned, eprtLab,
				strExtraText);
			return _T(strSql);
			}
		break;

		case 654: // Lab Specimen Labels
			// Version history
			// (z.manning 2008-11-03 12:27) - PLID 31904 - Created
			//TES 5/14/2009 - PLID 33792 - Added InitialDiagnosis
			// (j.jones 2009-09-11 13:06) - PLID 35503 - fixed PlanName, and added LastApptDate and Instructions
			//TES 11/11/2009 - PLID 36260 - Replaced AnatomySide with AnatomyQualifierID
			//TES 12/8/2009 - PLID 36512 - Restored AnatomySide
			//TES 5/17/2010 - PLID 38677 - Added gender, insurance company address fields, and secondary insurance information.
			// (j.jones 2010-05-18 08:39) - PLID 38185 - added SSN
			// (c.haag 2010-09-08 11:35) - PLID 38494 - Added custom fields
			// (f.dinatale 2010-10-18) - PLID 80476 - Added SSN Masking.
			// (c.haag 2010-12-14 9:39) - PLID 41806 - Replaced Completed fields with functions
			// (d.lange 2011-01-14 12:14) - PLID 29065 - Added Biopsy Type
			//TES 8/5/2011 - PLID 44901 - Filter on permissioned locations
			// (c.haag 2015-02-23) - PLID 63751 - Moved to SharedReportInfo.cpp
			return _T(__super::GetMoreSqlPrintPreview(CreateThreadSnapshotConnection(), (LPCTSTR)GetAPILoginToken(), nSubLevel, nSubRepNum));

		case 661:
			//Graphical Lab Results By Patient
			/*Version History 
				// (j.gruber 2009-05-07 15:15) - PLID 28556 - created
				// (z.manning 2009-06-11 16:53) - PLOD 34604 - Allowed caller to specify addition where
				// clause using strExtraText
			*/
			return FormatString(
				"SELECT Name, DateReceived, CASE WHEN IsNumeric(CONVERT(nvarchar, Value) + 'e0') = 1 THEN CONVERT(float, CONVERT(nvarchar, Value)) ELSE NULL END as NumericValue, "
				" PatientID as PatID, PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PatientsT.UserDefinedID "
				" FROM LabsT LEFT JOIN LabResultsT ON LabsT.ID = LabResultsT.LabID "
				" LEFT JOIN PersonT ON LabsT.PatientID = PersonT.ID "
				" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				" WHERE LabsT.DELETED = 0 AND LabResultsT.Deleted = 0 %s "
				, strExtraText);
		break;

		case 662:
			// Referral Orders
			/* Version History
				// (z.manning 2009-05-12 12:04) - PLID 34219 - Created
				// (f.dinatale 2010-10-18) - PLID 80476 - Added SSN Masking
			*/
			strSQL.Format(
				"SELECT ReferralOrdersT.ID AS RefOrderID, ReferralOrdersT.Date AS RefOrderDate, ReferralOrdersT.PatientID AS PatID \r\n"
				"	, Reason, ReferralOrdersT.InputDate, InputUserID, UsersT.Username AS InputUsername, UserDefinedID AS PatientID \r\n"
				"	-- Patient info \r\n"
				"	, Patient.First AS PatFirst, Patient.Middle AS PatMiddle, Patient.Last AS PatLast, Patient.Address1 AS PatAddress1 \r\n"
				"	, Patient.Address2 AS PatAddress2, Patient.City AS PatCity, Patient.State AS PatState, Patient.Zip AS PatZip \r\n"
				"	, Patient.Title AS PatTile, Patient.HomePhone AS PatHomePhone, Patient.WorkPhone AS PatWorkPhone \r\n"
				"	, Patient.Extension AS PatExtension, Patient.CellPhone AS PatCellPhone, Patient.Email AS PatEmail \r\n"
				"	, Patient.Fax AS PatFax, Patient.BirthDate AS PatBirthDate, dbo.MaskSSN(Patient.SocialSecurity, %s) AS PatSSN \r\n"
				"	-- Referring provider info \r\n"
				"	, Provider.First AS ProvFirst, Provider.Middle AS ProvMiddle, Provider.Last AS ProvLast, Provider.Address1 AS ProvAddress1 \r\n"
				"	, Provider.Address2 AS ProvAddress2, Provider.City AS ProvCity, Provider.State AS ProvState, Provider.Zip AS ProvZip \r\n"
				"	, Provider.Title AS ProvTile, Provider.HomePhone AS ProvHomePhone, Provider.WorkPhone AS ProvWorkPhone \r\n"
				"	, Provider.Extension AS ProvExtension, Provider.CellPhone AS ProvCellPhone, Provider.Email AS ProvEmail \r\n"
				"	, Provider.Fax AS ProvFax, Provider.BirthDate AS ProvBirthDate, Provider.SocialSecurity AS ProvSSN \r\n"
				"	-- Referring to doctor info \r\n"
				"	, RefPhys.First AS RefPhysFirst, RefPhys.Middle AS RefPhysMiddle, RefPhys.Last AS RefPhysLast, RefPhys.Address1 AS RefPhysAddress1 \r\n"
				"	, RefPhys.Address2 AS RefPhysAddress2, RefPhys.City AS RefPhysCity, RefPhys.State AS RefPhysState, RefPhys.Zip AS RefPhysZip \r\n"
				"	, RefPhys.Title AS RefPhysTile, RefPhys.HomePhone AS RefPhysHomePhone, RefPhys.WorkPhone AS RefPhysWorkPhone \r\n"
				"	, RefPhys.Extension AS RefPhysExtension, RefPhys.CellPhone AS RefPhysCellPhone, RefPhys.Email AS RefPhysEmail \r\n"
				"	, RefPhys.Fax AS RefPhysFax, RefPhys.BirthDate AS RefPhysBirthDate, RefPhys.SocialSecurity AS RefPhysSSN \r\n"
				"FROM ReferralOrdersT \r\n"
				"LEFT JOIN PersonT Patient ON ReferralOrdersT.PatientID = Patient.ID \r\n"
				"LEFT JOIN PatientsT ON Patient.ID = PatientsT.PersonID \r\n"
				"LEFT JOIN PersonT Provider ON ReferralOrdersT.ReferredByID = Provider.ID \r\n"
				"LEFT JOIN PersonT RefPhys ON ReferralOrdersT.ReferToID = RefPhys.ID \r\n"
				"LEFT JOIN UsersT ON ReferralOrdersT.InputUserID = UsersT.PersonID \r\n", 
				((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"));
			return _T(strSQL);
			break;

		case 663:
			{
			//Patient Graph Data (PP)
			/*	Version History
				(d.thompson) 2009-05-21 - PLID 34322 - Created, runs from the Graph Patient Data dialog.  saExtraValues is
					used to provide the list of EMRInfoMasterIDs we need to filter on.  strExtraText is hijacked to provide
					a WHERE CLAUSE addition for all provided options.
					// (j.gruber 2010-02-10 11:00) - PLID 37148 - Added sql for formulas, strExtraField hijacked for tables sql
				//(e.lally 2012-01-11) PLID 44774 - Replaced IsNumeric with new AsNumericString function
			*/


			// (j.gruber 2010-02-10 11:29) - PLID 37148 - change how patient filtering works so that it doesn't put this is a subquery
			CString strPatientFilter;
			strPatientFilter.Format(" EMRMasterT.PatientID = %li", nPatient);

			CString strFilter = " AND EMRInfoMasterID IN (";
			//Just in case, and for TTX generation
			if(saExtraValues.GetSize() == 0) {
				strFilter += "0, ";
			}
			for (int i = 0; i < saExtraValues.GetSize(); i++) {
				strFilter += saExtraValues.GetAt(i) + ", ";
			}
			//truncate the last comma and end the IN
			strFilter = strFilter.Left(strFilter.GetLength() - 2);
			strFilter += ")";

			//If the text data is non-numeric, we select it as 0.
			//DRT 6/19/2009 - Note that we add the + 'e0' to confirm it's a floating point convertable number.  IsNumeric
			//	will return true for things like '$10' or '4,200', which are then not convertable to int/float/etc.
			//(e.lally 2012-01-11) PLID 44774 - Use the AsNumericString function now to more accurately reflect how the EMR formulas work.
				//See additional PLID comments/documentation for examples.
			CString strQuery;
			strQuery.Format(" %s "
				" SELECT EMRMasterT.ID AS EMNID, EMRMasterT.PatientID AS PatID, EMRMasterT.LocationID AS LocID, EMRMasterT.InputDate AS IDate, EMRMasterT.Date AS TDate, "
				"PatientPersonT.Last + ', ' + PatientPersonT.First + ' ' + PatientPersonT.Middle AS PatName,  "
				"CASE WHEN EmrInfoT.ID =  -27 THEN EMRDetailsT.MacroName ELSE EmrInfoT.Name END AS DetailName,  "
				"COALESCE(CASE WHEN EMRInfoT.Datatype = 1 THEN (CASE WHEN dbo.AsNumericString(EMRDetailsT.Text) <> '' THEN convert(float, dbo.AsNumericString(EMRDetailsT.Text)) ELSE 0.0 END) "
				"	WHEN EMRInfoT.Datatype = 2 THEN (CASE WHEN dbo.AsNumericString(EMRDataT.Data) <> '' THEN convert(float, dbo.AsNumericString(EMRDataT.Data)) ELSE 0.0 END) "
				"	WHEN EMRInfoT.DataType = 5 THEN EMRDetailsT.SliderValue "
				"	ELSE 0.0 END, 0.0) AS DetailData, "
				"EMRInfoT.EMRInfoMasterID, 2 as DecimalPlaces "
				" "
				"FROM EMRMasterT  "
				"LEFT JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID  "
				"LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
				"LEFT JOIN PersonT AS PatientPersonT ON EMRMasterT.PatientID = PatientPersonT.ID  "
				"LEFT JOIN EMRSelectT ON EmrDetailsT.ID = EmrSelectT.EmrDetailID  "
				"LEFT JOIN EMRDataT ON EmrSelectT.EMRDataID = EMRDataT.ID  "
				" "
				"WHERE EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 AND EMRInfoT.DataType NOT IN (3, 4, 6, 7) /*Multi-select, Image, Rich Text, Tables*/ "
				"AND %s %s %s"
				" UNION "
				" SELECT EMNID, PatID, LocID, IDate, Date as TDate, PatName, DetailName, Round(PlotValue, DecimalPlaces) as DetailData, EMRInfoMasterID, DecimalPlaces \r\n "
				" FROM %s VTable INNER JOIN %s ITable ON VTable.ID = ITable.ID \r\n "				
				, strExtraField, strPatientFilter, strFilter, strExtraText, strExtendedSql, strExternalFilter);

			return _T(strQuery);
			}
			break;

		case 693:
		{
			//Patient Graph By Age (PP)
			/*	Version History
				// (j.gruber 2010-02-17 10:56) - PLID 37378 - Created based off of Patient Graph Data
				//(e.lally 2012-01-11) PLID 44774 - Replaced IsNumeric with new AsNumericString function
			*/

			CString strPatientFilter;
			strPatientFilter.Format(" EMRMasterT.PatientID = %li", nPatient);

			CString strFilter = " AND EMRInfoMasterID IN (";
			//Just in case, and for TTX generation
			if(saExtraValues.GetSize() == 0) {
				strFilter += "0, ";
			}
			for (int i = 0; i < saExtraValues.GetSize(); i++) {
				strFilter += saExtraValues.GetAt(i) + ", ";
			}
			//truncate the last comma and end the IN
			strFilter = strFilter.Left(strFilter.GetLength() - 2);
			strFilter += ")";		
			

			//(e.lally 2012-01-11) PLID 44774 - Use the AsNumericString function now to more accurately reflect how the EMR formulas work.
				//See additional PLID comments/documentation for examples.
			CString strQuery;
			strQuery.Format(" SELECT DataQ.EMNID as EMNID, DataQ.PatID as PatID, DataQ.LocID as LocID,  DataQ.IDate as IDate, DataQ.TDate as TDate, "
				" DataQ.PatName, DataQ.DetailName, DataQ.DetailData, DataQ.EMRInfoMasterID, DataQ.DecimalPlaces, "
				" (((YEAR(DataQ.TDate) - YEAR(PersonT.BirthDate) - "
				" 	CASE WHEN MONTH(PersonT.BirthDate) > MONTH(DataQ.TDate) THEN 1  "
				" 	WHEN MONTH(PersonT.BirthDate) < MONTH(DataQ.TDate) THEN 0  "
				" 	WHEN DAY(PersonT.BirthDate) > DAY(DataQ.TDate) THEN 1  "
				" 	ELSE 0 END) * 12)  "
				" 	+ (DATEDIff(month, PersonT.Birthdate, DataQ.TDate) -  "
				" 	CASE WHEN Day(PersonT.BirthDate) > Day(DataQ.TDate) THEN 1 "
				" 	ELSE 0 END) %% 12) + "
				" 	CASE WHEN (DateDiff(day, DateAdd(month, (((YEAR(DataQ.TDate) - YEAR(PersonT.BirthDate) -  "
				" 	CASE WHEN MONTH(PersonT.BirthDate) > MONTH(DataQ.TDate) THEN 1  "
				" 	WHEN MONTH(PersonT.BirthDate) < MONTH(DataQ.TDate) THEN 0  "
				" 	WHEN DAY(PersonT.BirthDate) > DAY(DataQ.TDate) THEN 1 "
				" 	ELSE 0 END) * 12)  "
				" 	+ (DATEDIff(month, PersonT.Birthdate, DataQ.TDate) -  "
				" 	CASE WHEN Day(PersonT.BirthDate) > Day(DataQ.TDate) THEN 1 "
				" 	ELSE 0 END) %% 12), PersonT.BirthDate), DataQ.TDate)) = 0 THEN 0 else .5 END  "
				" 	as AgeInMonths,"
				" -1 as Percentile, PersonT.Birthdate "
				" FROM ( "
				" SELECT EMRMasterT.ID AS EMNID, EMRMasterT.PatientID AS PatID, EMRMasterT.LocationID AS LocID, EMRMasterT.InputDate AS IDate, EMRMasterT.Date AS TDate, "
				"PatientPersonT.Last + ', ' + PatientPersonT.First + ' ' + PatientPersonT.Middle AS PatName,  "
				"CASE WHEN EmrInfoT.ID =  -27 THEN EMRDetailsT.MacroName ELSE EmrInfoT.Name END AS DetailName,  "
				"COALESCE(CASE WHEN EMRInfoT.Datatype = 1 THEN (CASE WHEN dbo.AsNumericString(EMRDetailsT.Text) <> '' THEN convert(float, dbo.AsNumericString(EMRDetailsT.Text)) ELSE 0.0 END) "
				"	WHEN EMRInfoT.Datatype = 2 THEN (CASE WHEN dbo.AsNumericString(EMRDataT.Data) <> '' THEN convert(float, dbo.AsNumericString(EMRDataT.Data)) ELSE 0.0 END) "
				"	WHEN EMRInfoT.DataType = 5 THEN EMRDetailsT.SliderValue "
				"	ELSE 0.0 END, 0.0) AS DetailData, "
				"EMRInfoT.EMRInfoMasterID, 2 as DecimalPlaces "
				" "
				"FROM EMRMasterT  "
				"LEFT JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID  "
				"LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
				"LEFT JOIN PersonT AS PatientPersonT ON EMRMasterT.PatientID = PatientPersonT.ID  "
				"LEFT JOIN EMRSelectT ON EmrDetailsT.ID = EmrSelectT.EmrDetailID  "
				"LEFT JOIN EMRDataT ON EmrSelectT.EMRDataID = EMRDataT.ID  "
				" "
				"WHERE EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 AND EMRInfoT.DataType NOT IN (3, 4, 6, 7) /*Multi-select, Image, Rich Text, Tables*/ "
				"AND %s %s %s"
				" UNION " 
				" SELECT EMNID, PatID, LocID, IDate, Date as TDate, PatName, DetailName, Round(PlotValue, DecimalPlaces) as DetailData, EMRInfoMasterID, DecimalPlaces \r\n "
				" FROM %s VTable INNER JOIN %s ITable ON VTable.ID = ITable.ID \r\n "				
				" ) DataQ LEFT JOIN PersonT ON DataQ.PatID = PersonT.ID "
				" UNION ALL "
				" SELECT -1 as EMNID, -1 as PatID, -1 as LocID, NULL as IDate, NULL as TDate, '' as PatName, 'StatData' as DetailName, "
				" Value as DetailData, -1 as EMRMasterID, 8 as DecimalPlaces, AgeInMonths, Percentile, NULL "
				" FROM StatisticalNormDetailsT WHERE StatisticalNormID = %li AND Gender = %li AND  "
				" CONVERT(int, AgeInMonths) %% 12 = 0"
				, strPatientFilter, strFilter, strExtraText, strExtendedSql, strExternalFilter, nExtraID, nFilterID);

			return _T(strQuery);
			}
			break;

		case 664:
			// Immunization Record (PP)
			/* Version History
			   v.arth 2009-05-21 - PLID 34321 - Created report
			   // (z.manning 2009-06-09 11:30) - PLID 34544 - Added username
			   //(e.lally 2010-01-04) PLID 35768 - Added reaction
			   // (d.singleton 2013-07-11 12:33) - PLID 57515 - added Administrative Notes
			*/
			return _T("SELECT PatientImmunizationsT.ImmunizationId, ImmunizationsT.Type as ImmunizationType, PatientImmunizationsT.Dosage, "
					  "       PatientImmunizationsT.DateAdministered, ImmunizationRoutesT.Name as RouteName, ImmunizationSitesT.Name as SiteName, "
					  "		  PatientImmunizationsT.LotNumber, PatientImmunizationsT.ExpirationDate, PatientImmunizationsT.Manufacturer, "
					  "		  PatientImmunizationsT.Reaction, PatientImmunizationsT.AdministrativeNotes, "
					  "		  PersonT.Id as PatId, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, "
					  "		  PersonT.City, PersonT.State, PersonT.Zip, PersonT.Homephone, UsersT.Username AS CreatedUsername "
					  "FROM PatientImmunizationsT "
					  "    LEFT JOIN ImmunizationsT "
					  "        ON PatientImmunizationsT.ImmunizationId = ImmunizationsT.Id "
					  "    LEFT JOIN ImmunizationRoutesT "
					  "        ON PatientImmunizationsT.RouteId = ImmunizationRoutesT.Id "
					  "    LEFT JOIN ImmunizationSitesT "
					  "        ON PatientImmunizationsT.SiteId = ImmunizationSitesT.Id "
					  "    LEFT JOIN PersonT "
					  "        ON PatientImmunizationsT.PersonId = PersonT.Id "
					  "	   LEFT JOIN UsersT ON PatientImmunizationsT.CreatedUserID = UsersT.PersonID "
					  );
			break;

		case 665:
			/*Patient Wellness Alert Guidelines (PP)
				// (j.gruber 2009-05-29 14:22) - PLID 34401 - created
				//hijacked the extraText, extrafield, and strExtended Sql
				//(e.lally 2010-09-10) PLID 40488 - Added PatientsT.PersonID value as PatID. Comes from nExtraID
				*/{
			CString strContent;
			strContent.Format("SELECT '%s' as Guideline, "
				" '%s' as AlertName, '%s' as PatName, %li as PatID ",
				_Q(strExtraText), _Q(strExtraField), _Q(strExtendedSql), nExtraID);
			return _T(strContent);
			}
		break;

		case 666:
			/*Patient Wellness Alert Reference (PP)
				// (j.gruber 2009-05-29 14:22) - PLID 34401 - created
				//hijacked the extraText, extrafield, and strExtended Sql
				//(e.lally 2010-09-10) PLID 40488 - Added PatientsT.PersonID value as PatID. Comes from nExtraID
				*/{
			CString strContent;
			strContent.Format("SELECT '%s' as Reference, "
				" '%s' as AlertName, '%s' as PatName, %li as PatID ",
				_Q(strExtraText), _Q(strExtraField), _Q(strExtendedSql), nExtraID);
			return _T(strContent);
			}
		break;

		case 671:	//Inventory Reconciliation Adjustments (PP)
			{
			/*
			Version History:   
			 // (j.jones 2009-07-10 09:55) - PLID 34843 - created
			*/
		
			if(strExtendedSql.IsEmpty()) {
				//maybe this was called from the ttx creator?
				return _T("SELECT ProductID, ProductName, CalculatedAmt, UserCount, WillAdjust, CountAvailable "
					"FROM "
					"(SELECT Convert(int, 1) AS ProductID, '' AS ProductName, Convert(float, 0.0) AS CalculatedAmt, "
					"Convert(float, 0.0) AS UserCount, Convert(bit, 1) AS WillAdjust, Convert(float, 0.0) AS CountAvailable "
					") AS InvRecAdjustmentsQ");
			}
			else {
				//the CInvReconciliationAdjustDlg will always send strExtendedSql at runtime, as the temp table name
				CString strSql;
				strSql.Format("SELECT ProductID, ProductName, CalculatedAmt, UserCount, WillAdjust, CountAvailable "
					"FROM "
					"(SELECT ProductID, ProductName, CalculatedAmt, UserCount, WillAdjust, CountAvailable "
					"FROM %s) AS InvRecAdjustmentsQ", strExtendedSql);
				return _T(strSql);
			}
			
			break;
			}

		case 679: {	//Room Manager (PP)

			// (j.jones 2009-09-21 16:05) - PLID 25232 - created
			//TES 1/15/2014 - PLID 55751 - Updated to use the new Waiting Room structure introduced by 38597
			
			switch (nSubLevel) {

			case 0: {	//Main Report 

				//the main report is the room list
				CString strSql;
				strSql.Format("SELECT PersonT.ID AS PatID, PatientsT.UserDefinedID, "
					"RoomsT.LocationID AS LocID, RoomsT.Name AS RoomName, "
					"Last + ', '  + First + ' ' + Middle AS PatientName, "
					"AppointmentsT.StartTime AS ApptStartTime, "
					"(SELECT MAX(TimeStamp) FROM AptShowStateHistoryT "
						"WHERE AptShowStateHistoryT.AppointmentID = AppointmentsT.ID AND ShowStateID = 1) AS ArrivalTime, "
					"RoomAppointmentsT.CheckInTime, "
					""
					"	(SELECT Min(UpdateTime) AS FirstTimeWithoutPerson FROM RoomAppointmentHistoryT "
					"	WHERE StatusID IN (SELECT ID FROM RoomStatusT WHERE WithPerson = 0) "
					"	AND RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID "
					"	AND UpdateTime > (SELECT Max(UpdateTime) AS LastStartTimeWithPerson FROM RoomAppointmentHistoryT "
					"	WHERE StatusID IN (SELECT ID FROM RoomStatusT WHERE WithPerson = 1) "
					"	AND RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID)) "
					"	AS TimeLastSeenByPerson, "
					""
					"(CASE WHEN RoomStatusT.WithPerson = 1 THEN NULL ELSE "
					"	DateAdd(second, "
					"		DateDiff(second, "
					"			Coalesce((SELECT Min(UpdateTime) AS FirstTimeWithoutPerson FROM RoomAppointmentHistoryT "
					"			WHERE StatusID IN (SELECT ID FROM RoomStatusT WHERE WithPerson = 0) "
					"			AND RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID "
					"			AND UpdateTime > (SELECT Max(UpdateTime) AS LastStartTimeWithPerson FROM RoomAppointmentHistoryT "
					"			WHERE StatusID IN (SELECT ID FROM RoomStatusT WHERE WithPerson = 1) "
					"			AND RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID)),RoomAppointmentsT.CheckInTime), "
					"		GetDate()), "
					"	dbo.AsDateNoTime(GetDate())) "
					"END) AS WaitingMinutes, "
					""
					"RoomStatusT.Name AS Status, "
					"RoomStatusT.WithPerson, UsersT.Username AS LastUpdatedBy, "					
					"AptTypeT.Name AS ApptType, "
					"dbo.GetPurposeString(AppointmentsT.ID) AS ApptPurpose, "
					"AptTypeT.Name + ' - ' + dbo.GetPurposeString(AppointmentsT.ID) AS ApptTypePurpose "
					"FROM RoomsT "
					"LEFT JOIN (SELECT * FROM RoomAppointmentsT WHERE StatusID NOT IN (0, -1)) AS RoomAppointmentsT ON RoomsT.ID = RoomAppointmentsT.RoomID "
					"LEFT JOIN AppointmentsT ON RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
					"LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
					"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"LEFT JOIN RoomStatusT ON RoomAppointmentsT.StatusID = RoomStatusT.ID "
					"LEFT JOIN UsersT ON RoomAppointmentsT.LastUpdateUserID = UsersT.PersonID "
					"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
					"WHERE RoomsT.Inactive = 0 AND RoomsT.WaitingRoom = 0 ");

				//strExtraText is an optional resource filter, but the rooms do not filter
				//by resource, so we do not need to use it in the main report query

				return _T(strSql);
			}
			break;

			case 1:  

				switch (nSubRepNum) {

					case 0: {
						//waiting area subreport
						CString strSql;
						strSql.Format("SELECT "
							"AppointmentsT.PatientID AS PatID, PatientsT.UserDefinedID, "
							"AppointmentsT.LocationID AS LocID, "
							"Last + ', '  + First + ' ' + Middle AS PatientName, "
							"AppointmentsT.StartTime AS ApptStartTime, "
							"TimeMarkedIn, CheckedInBy, "
							""
							"(SELECT Max(UpdateTime) AS FirstTimeWithoutPerson FROM RoomAppointmentHistoryT "
							"	WHERE RoomAppointmentHistoryT.RoomAppointmentID IN (SELECT RoomAppointmentsT.ID FROM RoomAppointmentsT INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID WHERE RoomsT.WaitingRoom = 0 AND AppointmentID = AppointmentsT.ID)) "
							"	AS TimeLastSeenByPerson, "
							""
							"DateAdd(second, "
							"	DateDiff(second, "
							"		Coalesce(Coalesce("
							"			(SELECT Max(UpdateTime) AS FirstTimeWithoutPerson FROM RoomAppointmentHistoryT "
							"			WHERE RoomAppointmentHistoryT.RoomAppointmentID IN (SELECT ID FROM RoomAppointmentsT WHERE AppointmentID = AppointmentsT.ID)), "
							"		TimeMarkedIn),AppointmentsT.StartTime), "
							"	GetDate()), "
							"dbo.AsDateNoTime(GetDate())) AS WaitingMinutes, "
							""
							"AptTypeT.Name AS ApptType, "
							"dbo.GetPurposeString(AppointmentsT.ID) AS ApptPurpose, "
							"AptTypeT.Name + ' - ' + dbo.GetPurposeString(AppointmentsT.ID) AS ApptTypePurpose "
							"FROM AppointmentsT "
							"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
							"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
							"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "		
							"LEFT JOIN (SELECT AppointmentID, Max(TimeStamp) AS TimeMarkedIn, Max(UsersT.UserName) AS CheckedInBy "
							"	FROM AptShowStateHistoryT "
							"	LEFT JOIN UsersT ON AptShowStateHistoryT.UserID = UsersT.PersonID "
							"	WHERE ShowStateID = 1 GROUP BY AppointmentID, ShowStateID) "
							"	AS AptShowStateHistoryQ ON AppointmentsT.ID = AptShowStateHistoryQ.AppointmentID "
							"WHERE AppointmentsT.Status <> 4 "
							"AND Convert(datetime, Convert(nvarchar, AppointmentsT.Date, 1)) = Convert(datetime, Convert(nvarchar, GetDate(), 1)) "
							"AND ("
								//appts. not in a non-waiting room
							"	(AppointmentsT.ShowState IN (SELECT ID FROM AptShowStateT WHERE WaitingArea = CONVERT(BIT,1)) "
							"	AND "
							"	AppointmentsT.ID NOT IN (SELECT RoomAppointmentsT.AppointmentID FROM RoomAppointmentsT "
							"		INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
							"		WHERE RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
							"		AND RoomsT.WaitingRoom = 0) "
							"	) "
							"	OR "
								//appts. in a waiting room
							"	AppointmentsT.ID IN (SELECT RoomAppointmentsT.AppointmentID FROM RoomAppointmentsT "
							"		INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
							"		WHERE RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
							"		AND RoomsT.WaitingRoom = 1 "
							"		AND RoomAppointmentsT.StatusID NOT IN (0, -1) "
							"	) "
							") "
							"%s", strExtraText); //strExtraText is an optional resource filter

						return _T(strSql);
					}
					break;

					case 1: {
						//checkout list subreport
						CString strSql;
						strSql.Format("SELECT "
							"AppointmentsT.PatientID AS PatID, PatientsT.UserDefinedID, "
							"AppointmentsT.LocationID AS LocID, "
							"Last + ', '  + First + ' ' + Middle AS PatientName, "
							"AppointmentsT.StartTime AS ApptStartTime, TimeMarkedIn, "
							""
							"	(SELECT Max(UpdateTime) AS TimeLeftRoom FROM RoomAppointmentHistoryT "
							"	WHERE RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID) "
							"	AS TimeLeftRoom, "
							""
							"DateAdd(second, "
							"	DateDiff(second, "
							"		Coalesce(Coalesce("
							"			(SELECT Max(UpdateTime) AS TimeLeftRoom FROM RoomAppointmentHistoryT "
							"			WHERE RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID), "
							"		TimeMarkedIn),AppointmentsT.StartTime), "
							"	GetDate()), "
							"dbo.AsDateNoTime(GetDate())) AS WaitingMinutes, "
							""
							"UsersT.Username AS LastUpdatedBy, "							
							"AptTypeT.Name AS ApptType, "
							"dbo.GetPurposeString(AppointmentsT.ID) AS ApptPurpose, "
							"AptTypeT.Name + ' - ' + dbo.GetPurposeString(AppointmentsT.ID) AS ApptTypePurpose "
							"FROM AppointmentsT "
							"INNER JOIN RoomAppointmentsT ON AppointmentsT.ID = RoomAppointmentsT.AppointmentID "
							"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
							"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
							"LEFT JOIN (SELECT AppointmentID, Max(TimeStamp) AS TimeMarkedIn "
							"	FROM AptShowStateHistoryT WHERE ShowStateID = 1 GROUP BY AppointmentID, ShowStateID) "
							"	AS AptShowStateHistoryQ ON AppointmentsT.ID = AptShowStateHistoryQ.AppointmentID "
							"LEFT JOIN UsersT ON RoomAppointmentsT.LastUpdateUserID = UsersT.PersonID "
							"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
							"WHERE AppointmentsT.Status <> 4 "
							"AND Convert(datetime, Convert(nvarchar, AppointmentsT.Date, 1)) = Convert(datetime, Convert(nvarchar, GetDate(), 1)) "
							"AND RoomAppointmentsT.StatusID = 0 "
							"%s", strExtraText); //strExtraText is an optional resource filter

						return _T(strSql);
					}
					break;

					default:
						return "";
						break;
				}
				break;

				default:
					return "";
					break;
			}
		}
		break;
		case 681: //Referral To Consult To Procedure (PP)
			/* Version History
				//(e.lally 2009-09-25) PLID 35654 - Created
			*/
			{
				CString strSql;
				strSql = "SELECT DISTINCT ConversionQ.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, "
					"PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, "
					"ConversionQ.PatientMainPhysicianID, ConversionQ.MainPhysicianFirst, ConversionQ.MainPhysicianLast, ConversionQ.MainPhysicianMiddle, "
					"ConversionQ.PatientLocationID, ConversionQ.PatientLocationName, "
					"ConversionQ.PatientCoordinatorID, CoordUsersT.Username as PatCoordUsername, PatientCoordT.First as PatCoordFirst, "
					"PatientCoordT.Last as PatCoordLast, PatientCoordT.Middle as PatCoordMiddle, "
					"ConversionQ.MasterProcedureID, ConversionQ.MasterProcedureName, "
					"ConversionQ.ConsultApptID, ConsultLocationID, ConsultLocationName, ConsultCreatedDate, ConsultCreatedLogin,  "
					"ConsultDate, ConsultStartTime, ConsultEndTime, ConsultAptTypeID, ConsultAptTypeName, "
					"\r\n"
					"ConversionQ.SurgeryApptID, SurgeryLocationID, SurgeryLocationName, SurgeryCreatedDate, SurgeryCreatedLogin, "
					"SurgeryDate, SurgeryStartTime, SurgeryEndTime, SurgeryAptTypeID, SurgeryAptTypeName, "
					"SurgeryDetailProcedureSubQ.ProcedureID AS SurgeryDetailedProcedureID, SurgeryDetailProcedureSubQ.ProcedureName AS SurgeryDetailedProcedureName, "
					"ConversionQ.MultiReferralID AS ReferralID, ReferralSourceT.Name AS ReferralName, MultiReferralsT.Date AS ReferralDate, "
					"ConversionQ.DefaultReferringPhyID AS RefPhyID, PersonT_RefPhys.First AS RefPhyFirst, PersonT_RefPhys.Middle AS RefPhyMiddle, PersonT_RefPhys.Last AS RefPhyLast, "
					"ConversionQ.ReferringPatientID AS RefPatID, PersonT_RefPatient.First AS RefPatFirst, PersonT_RefPatient.Middle AS RefPatMiddle, PersonT_RefPatient.Last AS RefPatLast "

					"\r\n\r\n"

					"FROM (" + 
						GetConsultToProcedureConversionRateBaseSql(TRUE) + ") ConversionQ\r\n"
					"INNER JOIN PersonT WITH(NOLOCK) ON ConversionQ.PatientID = PersonT.ID \r\n"
					"LEFT JOIN UsersT CoordUsersT WITH(NOLOCK) ON ConversionQ.PatientCoordinatorID = CoordUsersT.PersonID "
						"AND CoordUsersT.PatientCoordinator <> 0 \r\n"
					"LEFT JOIN PersonT PatientCoordT WITH(NOLOCK) ON CoordUsersT.PersonID = PatientCoordT.ID \r\n"
					"LEFT JOIN PatientsT WITH(NOLOCK) ON ConversionQ.PatientID = PatientsT.PersonID \r\n"
					"LEFT JOIN MultiReferralsT WITH(NOLOCK) ON ConversionQ.MultiReferralID = MultiReferralsT.ReferralID "
						" AND ConversionQ.PatientID = MultiReferralsT.PatientID\r\n"
					"LEFT JOIN ReferralSourceT WITH(NOLOCK) ON ConversionQ.MultiReferralID = ReferralSourceT.PersonID \r\n"
					"LEFT JOIN PersonT PersonT_RefPhys WITH(NOLOCK) ON ConversionQ.DefaultReferringPhyID = PersonT_RefPhys.ID \r\n"
					"LEFT JOIN PersonT PersonT_RefPatient WITH(NOLOCK) ON ConversionQ.ReferringPatientID = PersonT_RefPatient.ID \r\n"
					"LEFT JOIN "
						"(SELECT SurgeryDetailedProcedureT.Name AS ProcedureName, SurgeryDetailedProcedureT.ID AS ProcedureID, MasterProcedureID, AppointmentID "
						"FROM ProcedureT SurgeryDetailedProcedureT WITH(NOLOCK) \r\n"
						"INNER JOIN AppointmentPurposeT SurgeryPurposeT WITH(NOLOCK) ON SurgeryDetailedProcedureT.ID = SurgeryPurposeT.PurposeID "
					") SurgeryDetailProcedureSubQ \r\n"
						"ON ConversionQ.SurgeryApptID = SurgeryDetailProcedureSubQ.AppointmentID "
						"AND ConversionQ.SurgeryMasterProcedureID = COALESCE(SurgeryDetailProcedureSubQ.MasterProcedureID, SurgeryDetailProcedureSubQ.ProcedureID) "
					"WHERE PersonT.ID <> -25  [MultiReferralDateFrom] [MultiReferralDateTo] "
					//(e.lally 2010-03-12) PLID 37709 - Moved up here. Only show records with a referral source since we need the referral date.
					"AND (ReferralSourceT.PersonID IS NOT NULL) ";

				//(e.lally 2009-12-09) PLID 35654 - Added better handling for filtering based on the type of referral being viewed.
				//(e.lally 2010-03-12) PLID 37709 - This report is now going to show all the referral sources for each preview option,
					//but the referring physician and referring patient views are going to break down the special referral IDs into the 
					//detailed person.
				/*
				long nRefType = GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>");
				//These are the special mappings for the referral source used to indicate it was a referring physican or referred by patient.
				long nRefPatID = GetRemotePropertyInt("DefaultPatientReferral", -1, 0, "<None>", true);
				long nRefPhysID = GetRemotePropertyInt("DefaultPhysicianReferral", -1, 0, "<None>", true);
				CString strExtraWhere="";
				switch(nRefType){
					case 1:
						strExtraWhere.Format(" AND (ReferralSourceT.PersonID IS NOT NULL) ");
					break;
					case 2:
						strExtraWhere.Format(" AND (ReferralSourceT.PersonID = %li) ", nRefPhysID);
					break;
					case 3:
						strExtraWhere.Format(" AND (ReferralSourceT.PersonID = %li) ", nRefPatID);
					break;
				}

				strSql += strExtraWhere;
				*/
				

				//Apply the filters that were sent in as extra values.
				ApplyConsultToProcedurePPFilters(strSql, saExtraValues);

				/*Passed in by marketview.cpp*/
				return strSql;
			}
			break;
		case 698: //print Avery 8167 labels from the order screen by right clicking order 
			/* Version History
				// (s.dhole 2010-07-16 16:47) - PLID 28183  created
			*/
			// use addhack methode to pass tem table SQL to Cryatal report
			return strListBoxSQL;
			break;

		case 606:
			{
			/*Credit Card Batch Processing
			Version History:
			// (j.gruber 2007-07-31 15:07) - PLID 26719
			// (j.jones 2010-08-27 13:11) - PLID 40244 - moved Credit Card Batch Processing to ReportInfoPreview
			*/
			//we are using extraID as the batchID
			CString strSql;
			CString strCCFilter;
			if (strFilterField.IsEmpty()) {
				strCCFilter = "";
			}
			else {
				strCCFilter.Format(" AND CreditTransactionsT.BatchID = (SELECT ID FROM TransactionBatchT WHERE BatchNumber = '%s') ", strFilterField);
			}
			strSql.Format("  SELECT UserDefinedID, PersonT.First as PatFirst, PersonT.Last as PatLast, CreditTransactionsT.ID, PaymentPlansT.CCHoldersName AS CardHolderName,  "
				"    CASE WHEN CreditTransactionsT.TransactionCode = 1 then 'Credit Sale' ELSE   "
				"    CASE WHEN CreditTransactionsT.TransactionCode = 2 then 'Credit Authorization' ELSE  "
				"    CASE WHEN CreditTransactionsT.TransactionCode = 6 then 'Credit Return' ELSE  "
				"    CASE WHEN CreditTransactionsT.TransactionCode = 3 then 'Credit Forced Sale' ELSE  " 
				"    CASE WHEN CreditTransactionsT.TransactionCode = 27 then 'Interac Sale' ELSE  "
				"    CASE WHEN CreditTransactionsT.TransactionCode = 28 then 'Interac Sale W/Cash Back' ELSE  "
				"    CASE WHEN CreditTransactionsT.TransactionCode = 29 then 'Interac Return' END END END END END END END AS TransType,  "
				"    CreditCardNamesT.CardName, CASE WHEN TransactionCode IN (6,29) THEN -1 * CreditTransactionsT.TransactionAmount ELSE CreditTransactionsT.TransactionAmount END AS TransactionAmount, IsProcessed,   "
				"    CASE WHEN IsProcessed = 1 THEN CASE WHEN CreditTransactionsT.IsApproved = 1 then 'Approved' else 'Declined'   " 
				"    END ELSE '' END AS Result,  "
				"    CASE WHEN IsProcessed = 1 THEN CASE WHEN CreditTransactionsT.IsApproved= 1 then '' else RespMessage END   "
				"    ELSE '' END AS ReasonMessage, CreditCardNamesT.CardType, TransactionBatchT.BatchNumber, TransactionBatchT.BatchOpenDate, TransactionBatchT.BatchCloseDate as BatchCloseDate, TransactionBatchT.ID "
				"    FROM CreditTransactionsT INNER JOIN PaymentPlansT ON    "
				"    CreditTransactionsT.ID = PaymentPlansT.ID   "
				"    INNER JOIN LineItemT ON CreditTransactionsT.ID = LineItemT.ID  "
				"    INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID  "
				"    INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
				"    LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID  "
				"    LEFT JOIN TransactionBatchT ON CreditTransactionsT.BatchID = TransactionBatchT.ID "
				"    WHERE (1=1) %s "
				"    AND (LineItemT.Deleted = 0) AND (CreditTransactionsT.IsVoid = 0)  "
				"    ", strCCFilter);

			return _T(strSql);
			} 
		break;

		case 717: //print a frames version of the Avery labels on the order tab with a right click. 
			/* Version History
				// (b.spivey, October 21, 2011) - PLID 46072 - created
			*/
			return strListBoxSQL;
			break;
	
		case 718:
			//Meaningful Use Dialog
			/*Version History
			// (j.gruber 2011-11-03 15:08) - PLID 44993 - overwrote the variable
			*/		
			
			return strListBoxSQL;
		break;

		case 720:
			/*MU Measures
			// (j.gruber 2011-11-09 11:37) - PLID 45689
			*/
			return strListBoxSQL;
		break;
	default:
		return _T("");
		break;
	}


}
