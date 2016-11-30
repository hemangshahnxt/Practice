#include "stdafx.h"
#include "SOAPUtils.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"
#include "Practice.h"
#include "InvVisionWebUtils.h"
#include "PracticeRc.h"
#include "VisionWebServiceSetupDlg.h"
#include "InternationalUtils.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#import "msxml.tlb"
#include <OpticalUtils.h>

using namespace ADODB;

// (j.dinatale 2013-04-03 15:24) - PLID 56075 - Need a new enum for the prompting stuff and also some handy functions
namespace OrderServiceListType {
	CString GetCptTableName(ServiceListType slt)
	{
		CString strName = "";
		switch(slt)
		{
			case OrderServiceListType::Designs:
				strName = "GlassesCatalogDesignsCptT";
				break;
			case OrderServiceListType::Materials:
				strName = "GlassesCatalogMaterialsCptT";
				break;
			case OrderServiceListType::Treatments:
				strName = "GlassesCatalogTreatmentsCptT";
				break;
			default: // opps we did something wrong
				ThrowNxException("Invalid OrderServiceListType in OrderServiceListType::GetCptTableName!");
		}

		return strName;
	}

	CString GetCptColumnName(ServiceListType slt)
	{
		CString strName = "";

		switch(slt)
		{
			case OrderServiceListType::Designs:
				strName = "GlassesCatalogDesignsID";
				break;
			case OrderServiceListType::Materials:
				strName = "GlassesCatalogMaterialsID";
				break;
			case OrderServiceListType::Treatments:
				strName = "GlassesCatalogTreatmentsID";
				break;
			default: // opps we did something wrong
				ThrowNxException("Invalid OrderServiceListType in OrderServiceListType::GetCptColumnName!");
		}

		return strName;
	}
	
	CString GetDisplayName(ServiceListType slt)
	{
		CString strName = "";

		switch(slt)
		{
			case OrderServiceListType::Designs:
				strName = "Design";
				break;
			case OrderServiceListType::Materials:
				strName = "Material";
				break;
			case OrderServiceListType::Treatments:
				strName = "Treatment";
				break;
			default: // opps we did something wrong
				ThrowNxException("Invalid OrderServiceListType in OrderServiceListType::GetDisplayName!");
		}

		return strName;
	}
};


// (s.dhole 2010-11-02 15:04) - PLID 40540 Get VisionWeb specific Date Format "YYYY-MM-DDThh:mm:ss"
CString CInvVisionWebUtils::GetDateTimeValue(COleDateTime dt)
{
	CString strDateTime;
		if (dt.GetStatus() == COleDateTime::valid) {
			strDateTime.Format(_T("%04hu-%02hu-%02huT%02hu:%02hu:%02hu"),
			dt.GetYear(),
			dt.GetMonth() ,
			dt.GetDay() ,
			dt.GetHour() ,
			dt.GetMinute() ,
			dt.GetSecond() );
		return strDateTime;
	}
	else {
		return "";
	}
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 Get Password
CString GetVisionWebPasword()
{
	CString strPassword= "";
	_variant_t vPass = GetRemotePropertyImage(VISIONWEBSERVICE_USER_PASSWORD, 0, "<None>", false);
	if (vPass.vt != VT_EMPTY && vPass.vt != VT_NULL) {
		strPassword = DecryptStringFromVariant(vPass);
	
	} else {
		strPassword = "";
	}	
	return strPassword;
}


// (s.dhole 2010-12-20 17:07) - PLID 41125 Common SQL for Order Summery and submitt to VissionWeb
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
// (s.dhole 2011-04-11 17:30) - PLID 43237 change sql due to changes required for custom order data also added GlassesOrderProcessTypeID field
// (s.dhole, 2011-12-08 12:29) - PLID 46941 - added  PrismAxis2, PrismAxisStr2 
// (r.wilson 4/11/2012) PLID 43741 - Changed GlassesOrderStatus to GlassesOrderStatusID
ADODB::_RecordsetPtr  CInvVisionWebUtils::GetOrderRecordset(long nOrderID)
{

	_RecordsetPtr rsOrder = CreateParamRecordset("SELECT  vwOrderT.ID, vwOrderT.VisionWebOrderExchangeID, vwOrderT.LocationID, vwOrderT.PersonID,   \r\n"
						" vwOrderT.GlassesOrderType,   \r\n"
						" CASE vwOrderT.GlassesOrderType WHEN 1 THEN 'SP' WHEN 2 THEN 'FR' WHEN 3 THEN 'CP' WHEN 4 THEN 'CO' WHEN 5 THEN 'CSF' END  \r\n"
						" AS OrderTypeName, vwOrderT.OrderCreateDate, vwOrderT.GlassesSupplierLocationID,   \r\n"
						" vwOrderT.GlassesCatalogFrameTypeID, vwOrderT.SupplierID, vwOrderT.GlassesOrderStatusID AS GlassesOrderStatus,   \r\n"
						" vwOrderT.GlassesMessage, vwOrderT.MessageType, vwOrderT.GlassesJobType,   \r\n"
						" vwOrderT.GlassesJobNote,  vwOrderT.LeftGlassesOrderOtherInfoID,   \r\n"
						" vwOrderT.RightGlassesOrderOtherInfoID, vwOrderT.ShapeA, vwOrderT.ShapeB,   \r\n"
						" vwOrderT.ShapeED, vwOrderT.ShapeHalfDbl, vwOrderT.RequestedDate,   \r\n"
						" LeftvwoOtherInfoT.ThicknessValue AS LThicknessValue, LeftvwoOtherInfoT.ThicknessType AS LThicknessType,   \r\n"
						" LeftvwoOtherInfoT.TreatmentsComment AS LTreatmentsComment, LeftvwcDesignsT.DesignName AS LDesignName, LeftvwcDesignsT.GlassesOrderProcessTypeID AS LDesignsProcessTypeID,   \r\n"
						" LeftvwcDesignsT.DesignCode AS LDesignCode, LeftvwcMaterialsT.MaterialName AS LMaterialName,  LeftvwcMaterialsT.GlassesOrderProcessTypeID AS LMaterialProcessTypeID,  \r\n"
						" LeftvwcMaterialsT.MaterialCode AS LMaterialCode, RightvwoOtherInfoT.ThicknessValue AS RThicknessValue,   \r\n"
						" RightvwoOtherInfoT.ThicknessType AS RThicknessType,   \r\n"
						" RightvwoOtherInfoT.TreatmentsComment AS RTreatmentsComment,  \r\n" 
						" RightvwcDesignsT.DesignName AS RDesignName, RightvwcDesignsT.DesignCode AS RDesignCode, RightvwcDesignsT.GlassesOrderProcessTypeID AS RDesignsProcessTypeID,  \r\n"
						" RightvwcMaterialsT.MaterialName AS RMaterialName, RightvwcMaterialsT.MaterialCode AS RMaterialCode,  RightvwcMaterialsT.GlassesOrderProcessTypeID  AS RMaterialProcessTypeID ,  \r\n"
						" GlassesSupplierLocationsT.VisionWebAccountID,  left(PersonT.First,30) AS First, PersonT.Middle, LEFT(PersonT.Last,30) AS Last, vwOrderT.LensRxID,   \r\n"
						" vwOrderT.GlassesFramesDataID, GlassesFramesDataT.IsCatalog, GlassesFramesDataT.FPC, GlassesFramesDataT.StyleName, GlassesFramesDataT.ColorDescription,  \r\n"
						" GlassesFramesDataT.Eye, GlassesFramesDataT.Bridge, GlassesFramesDataT.Temple, GlassesFramesDataT.ManufacturerName,  \r\n"
						" GlassesFramesDataT.BrandName, GlassesFramesDataT.SKU, GlassesCatalogFrameTypesT.FrameTypeName, GlassesCatalogFrameTypesT.GlassesOrderProcessTypeID  AS FrameTypeProcessTypeID,  \r\n"
						" GlassesCatalogFrameTypesT.FrameTypeCode, RightLensRxT.RightLensDetailRxID, LeftLensRxT.LeftLensDetailRxID,   \r\n"
						" LeftLensDetailRxT.FarHalfPd AS LFarHalfPd, LeftLensDetailRxT.NearHalfPd AS LNearHalfPd, LeftLensDetailRxT.SegHeight AS LSegHeight,   \r\n"
						" LeftLensDetailRxT.OpticalCenter AS LOpticalCenter, LeftLensDetailRxT.PrescriptionSphere AS LPrescriptionSphere,   \r\n"
						" LeftLensDetailRxT.CylinderValue AS LCylinderValue, LeftLensDetailRxT.CylinderAxis AS LeftCylinderAxis,   \r\n"
						" LeftLensDetailRxT.AdditionValue AS LAdditionValue, LeftLensDetailRxT.PrismValue AS LPrismValue,   \r\n"
						" LeftLensDetailRxT.PrismAxis AS LPrismAxis, LeftLensDetailRxT.PrismAxisStr AS LPrismAxisStr, LeftLensDetailRxT.SecondaryPrismValue  AS LPrismValue2, \r\n"
						" LeftLensDetailRxT.PrismAxis2 AS LPrismAxis2, LeftLensDetailRxT.PrismAxisStr2 AS LPrismAxisStr2,   \r\n"
						" RightLensDetailRxT.FarHalfPd AS RFarHalfPd, RightLensDetailRxT.NearHalfPd AS RNearHalfPd,   \r\n"
						" RightLensDetailRxT.SegHeight AS RSegHeight, RightLensDetailRxT.OpticalCenter AS ROpticalCenter,   \r\n"
						" RightLensDetailRxT.PrescriptionSphere AS RPrescriptionSphere, RightLensDetailRxT.CylinderValue AS RCylinderValue,   \r\n"
						" RightLensDetailRxT.CylinderAxis AS RCylinderAxis, RightLensDetailRxT.AdditionValue AS RAdditionValue,   \r\n"
						" RightLensDetailRxT.PrismValue AS RPrismValue, RightLensDetailRxT.PrismAxis AS RPrismAxis,   \r\n"
						" RightLensDetailRxT.PrismAxisStr AS RPrismAxisStr, GETDATE() AS currentDate, SupplierT.VisionWebID,  RightLensDetailRxT.SecondaryPrismValue  AS RPrismValue2,  \r\n"
						" RightLensDetailRxT.PrismAxis2 AS RPrismAxis2, RightLensDetailRxT.PrismAxisStr2 AS RPrismAxisStr2,  \r\n"
						" RightvwoOtherInfoT.GlassesCatalogMaterialsID AS RightvwcMaterialID, \r\n"
						" LeftvwoOtherInfoT.GlassesCatalogDesignsID AS LeftvwcDesignsID, \r\n"
						" LeftvwoOtherInfoT.GlassesCatalogMaterialsID AS LeftvwcMaterialsID,   \r\n"
						" RightvwoOtherInfoT.GlassesCatalogDesignsID AS RightvwcDesignsID,   \r\n"
						" PersonT_Supplier.Location AS SupplierLocation,   \r\n"
						" PersonT_Supplier.WorkPhone AS Supplier_WorkPhone, PersonT_Supplier.Fax AS Supplier_Fax, PersonT_Supplier.Company AS Supplier_Company,   \r\n"
						" PersonT_Supplier.Address1 AS SupplierAddress1, PersonT_Supplier.Address2 AS Supplier_Address2, PersonT_Supplier.City AS Supplier_City,   \r\n"
						" PersonT_Supplier.State AS Supplier_State, PersonT_Supplier.Zip AS Supplier_Zip, PersonT.BirthDate AS Patient_BirthDate,   \r\n"
						" PersonT.HomePhone, PersonT.WorkPhone AS Patient_WorkPhone, vwOrderT.GlassesOrderNumber,   \r\n"
						" vwOrderT.OrderUploadDate, vwOrderT.UpdateDate, vwOrderT.SupplierTrackingId,   \r\n"
						" vwOrderT.Description, UsersT.UserName, LocationsT.Name AS Locations_Name,   \r\n"
						" LocationsT.Address1 AS Locations_Address1, LocationsT.Address2 AS Locations_Address2, LocationsT.City AS Locations_City,   \r\n"
						" LocationsT.State AS Locations_State, LocationsT.Zip AS Locations_Zip, LocationsT.Phone, LocationsT.Phone2,   \r\n"
						" LocationsT.Fax AS Locations_FAX  \r\n"
						" FROM         GlassesOrderT AS vwOrderT INNER JOIN  \r\n"
						" PersonT ON vwOrderT.PersonID = PersonT.ID INNER JOIN  \r\n"
						" SupplierT ON vwOrderT.SupplierID = SupplierT.PersonID INNER JOIN  \r\n"
						" PersonT AS PersonT_Supplier ON SupplierT.PersonID = PersonT_Supplier.ID INNER JOIN  \r\n"
						" UsersT ON vwOrderT.UserID = UsersT.PersonID INNER JOIN  \r\n"
						" LocationsT ON vwOrderT.LocationID = LocationsT.ID LEFT OUTER JOIN  \r\n"
						" GlassesCatalogFrameTypesT ON   \r\n"
						" vwOrderT.GlassesCatalogFrameTypeID = GlassesCatalogFrameTypesT.ID LEFT OUTER JOIN  \r\n"
						" GlassesFramesDataT ON vwOrderT.GlassesFramesDataID = GlassesFramesDataT.ID LEFT OUTER JOIN  \r\n"
						" GlassesCatalogDesignsT AS LeftvwcDesignsT RIGHT OUTER JOIN \r\n"
						" GlassesCatalogMaterialsT AS LeftvwcMaterialsT RIGHT OUTER JOIN \r\n"
						" GlassesOrderOtherInfoT AS LeftvwoOtherInfoT ON  \r\n"
						" LeftvwcMaterialsT.ID = LeftvwoOtherInfoT.GlassesCatalogMaterialsID ON  \r\n"
						" LeftvwcDesignsT.ID = LeftvwoOtherInfoT.GlassesCatalogDesignsID ON  \r\n"
						" vwOrderT.LeftGlassesOrderOtherInfoID = LeftvwoOtherInfoT.ID LEFT OUTER JOIN \r\n"
						" LensRxT AS LeftLensRxT INNER JOIN \r\n"
						" LensDetailRxT AS LeftLensDetailRxT ON LeftLensRxT.LeftLensDetailRxID = LeftLensDetailRxT.ID ON  \r\n"
						" vwOrderT.LensRxID = LeftLensRxT.ID LEFT OUTER JOIN \r\n"
						" GlassesCatalogDesignsT AS RightvwcDesignsT RIGHT OUTER JOIN \r\n"
						" GlassesOrderOtherInfoT AS RightvwoOtherInfoT ON  \r\n"
						" RightvwcDesignsT.ID = RightvwoOtherInfoT.GlassesCatalogDesignsID LEFT OUTER JOIN \r\n"
						" GlassesCatalogMaterialsT AS RightvwcMaterialsT ON  \r\n"
						" RightvwoOtherInfoT.GlassesCatalogMaterialsID = RightvwcMaterialsT.ID ON  \r\n"
						" vwOrderT.RightGlassesOrderOtherInfoID= RightvwoOtherInfoT.ID LEFT OUTER JOIN \r\n"
						" LensDetailRxT AS RightLensDetailRxT INNER JOIN \r\n"
						" LensRxT AS RightLensRxT ON RightLensDetailRxT.ID = RightLensRxT.RightLensDetailRxID ON  \r\n"
						" vwOrderT.LensRxID = RightLensRxT.ID \r\n"
						"  LEFT OUTER JOIN \r\n"
						" GlassesSupplierLocationsT ON vwOrderT.GlassesSupplierLocationID  = GlassesSupplierLocationsT.ID \r\n"
						" WHERE     (vwOrderT.IsDelete = 0) AND (vwOrderT.ID = {INT})  \r\n",nOrderID);
	
	return rsOrder;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 return XML for selected order 
MSXML2::IXMLDOMNodePtr  CInvVisionWebUtils::GetOrderXML(long nOrderID,LPCTSTR strUserId,LPCTSTR strPassword ,LPCTSTR strRefId,
														OUT CString &strSupplierId, OUT CString &strLeftDesignCode,  
														OUT CString &strLeftMaterialCode, OUT CString &strRightDesignCode, 
														OUT CString &strRightMaterialCode, OUT CString &strSupplierInfo,OUT CString &strLocationInfo,
													OUT CString &strLeftDesignInfo,OUT CString &strRightDesignInfo,
													OUT CString &strLeftMaterialInfo,OUT CString &strRightMaterialInfo,
													OUT long &nOrderType)
{
	
	HR(pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60)));
	MSXML2::IXMLDOMNodePtr  pRoot =CreateNode("ORDER_MSG");
	_RecordsetPtr rsOrder = CInvVisionWebUtils::GetOrderRecordset(nOrderID) ;
	if(!rsOrder->eof) 
		{

			
			// Get server date
			COleDateTime dtNow = AdoFldDateTime(rsOrder->Fields, "currentDate") ;
			strSupplierId=AdoFldString(rsOrder->Fields, "VisionWebID", "");
			strSupplierInfo= FormatString("%s (%s) ", AdoFldString(rsOrder->Fields, "Supplier_Company", ""),strSupplierId)  ;
			// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses			
			nOrderType = AdoFldByte (rsOrder->Fields, "GlassesOrderType"); 
	
			//HEADER
			pRoot->appendChild(CreateNodevwOrderXMLHeader(strUserId ,
			AdoFldString(rsOrder->Fields, "OrderTypeName", ""),strPassword,GetDateTimeValue(dtNow)
			,"40B22264-C9B5-4078-81AC-91375827D6B6",strRefId
			,ConvertLongToString( AdoFldLong(rsOrder->Fields, "ID",-999)),strSupplierId,"2.01"));
			 
			//pSP_ORDER
			MSXML2::IXMLDOMNodePtr pSP_ORDER = CreateNodevwOrderXMLSP_ORDER(
				GetDateTimeValue(AdoFldDateTime(rsOrder->Fields, "OrderCreateDate",dtNow)),
				ConvertLongToString( AdoFldLong(rsOrder->Fields, "ID",-999)));
			//pSP_ORDER=>REDO_Information
		
			//pSP_ORDER=>ACCOUNT
			// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
			pSP_ORDER->appendChild(CreateNodevwOrderXMLAccount( ConvertLongToString( AdoFldLong( rsOrder->Fields,"VisionWebAccountID",-999))));
			strLocationInfo= FormatString("%s (%d) ", AdoFldString(rsOrder->Fields, "Locations_Name", ""),AdoFldLong( rsOrder->Fields,"VisionWebAccountID",-999))  ;
			//RX_DETAIL
			MSXML2::IXMLDOMNodePtr pRX_Details = CreateNode("RX_DETAILS");
			//SP_ORDER=>RX_DETAIL->PATIENT
			
			MSXML2::IXMLDOMNodePtr pPATIENT  = CreateNode("PATIENT");
			// (s.dhole 2010-12-13 12:15) - PLID Add customize data Add-on
			//we are waiting for this item to be implmentd by VisionWeb 
			//_RecordsetPtr rsCustomData = CreateParamRecordset("SELECT vwCP.GlassesCustomParameterID, vwOP.ParameterValue \r\n"
			//									" FROM  GlassesCustomParameterT AS vwCP  \r\n"
			//									" INNER JOIN GlassesOrderParameterT  AS vwOP ON  \r\n"
			//									" vwCP.GlassesCustomParameterID = vwOP.GlassesCustomParameterID \r\n"
			//									" WHERE (vwOP.GlassesOrderID = {INT}) ORDER BY vwCP.DisplayOrder DESC ", nOrderID);
			//if(!rsCustomData->eof) {
			//	CString strPInitial;
			//	CString strNM=AdoFldString(rsOrder->Fields, "Last", "");

			//	if (strNM!="")
			//	{
			//		strNM.TrimLeft();  
			//		strPInitial =strNM.Left(1) ;
			//	}
			//	strNM=AdoFldString(rsOrder->Fields, "First", "");
			//	if (strNM!="")
			//	{
			//		strNM.TrimLeft();
			//		strPInitial +=strNM.Left(1) ;
			//	}
			//	MSXML2::IXMLDOMNodePtr pPERSONALIZED_DATA = CreateNodevwOrderXMLPersonalizedData(strPInitial) ;
			//	
			//	while(!rsCustomData->eof) {
			//		if (AdoFldString(rsCustomData->Fields, "ParameterValue", "") !="")
			//		{
			//			pPERSONALIZED_DATA ->appendChild(CreateNodevwOrderXMLSpecialData(AdoFldString(rsCustomData->Fields, "VisionWebCustomParameterID", ""),
			//				AdoFldString(rsCustomData->Fields, "ParameterValue", "")));
			//		}
			//			rsCustomData->MoveNext();
			//	}
			//	pPATIENT->appendChild(pPERSONALIZED_DATA) ;
			//}
			
			AppendNodevwOrderXMLpatient(pPATIENT, AdoFldString(rsOrder->Fields, "Last", ""), AdoFldString(rsOrder->Fields, "First", "")) ;
				
			pRX_Details ->appendChild(pPATIENT);
			//SP_ORDER=>RX_DETAIL->JOB
			// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
			pRX_Details ->appendChild(CreateNodevwOrderXMLJob(  AdoFldString(rsOrder->Fields, "GlassesJobType", ""),  AdoFldString(rsOrder->Fields, "GlassesJobNote", "")));
			if (AdoFldLong(rsOrder->Fields, "GlassesFramesDataID", 0) != 0){
				//SP_ORDER=>RX_DETAIL=>FRAME
				// (j.dinatale 2013-04-18 17:20) - PLID 56308 - we weren't using this function correctly
				MSXML2::IXMLDOMNodePtr pFrame=CreateNodevwOrderXMLFrame(
					"0", // Number
					AdoFldString(rsOrder->Fields, "ManufacturerName", ""), // brand
					AdoFldString(rsOrder->Fields, "StyleName", ""), // model
					AdoFldString(rsOrder->Fields, "ColorDescription", ""),	// color
					AdoFldString(rsOrder->Fields, "Eye", ""), // eye size
					"", // sku
					AdoFldString(rsOrder->Fields, "FrameTypeName", ""), // lms code
					AdoFldString(rsOrder->Fields, "FrameTypeCode", ""), // vw code for frame
					"", // temple type
					"", // type name
					AdoFldString(rsOrder->Fields, "Temple", "") // temple length
				);

				//SP_ORDER=>RX_DETAIL=>FRAME=>SUPPLIER
				//pFrame->appendChild(CreateNodevwOrderXMLFrameSupplier(  "",  ""));
				//SP_ORDER=>RX_DETAIL->FRAME=>PACKAGE
				//pFrame->appendChild(CreateNodevwOrderXMLFramePackage(  "",  "",  ""));
				pRX_Details ->appendChild(pFrame);
			}
			
			//SP_ORDER=>RX_DETAIL=>POSITION
			
			// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		 	if ( AdoFldLong(rsOrder->Fields, "LeftGlassesOrderOtherInfoID", 0)>0 && AdoFldLong(rsOrder->Fields, "LeftLensDetailRxID", 0)>0)
			{
				MSXML2::IXMLDOMNodePtr pPosition=CreateNodevwOrderXMLPosition( "L",
						ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LFarHalfPd", 999.0)),
						ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LSegHeight", 999.0)),
						ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LNearHalfPd", 999.0)),
						ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LOpticalCenter", 999.0)));
				//SP_ORDER=>RX_DETAIL=>POSITION=>PRISCRIPTION
				MSXML2::IXMLDOMNodePtr pPrescriptionNode =CreateNodevwOrderXMLPrescription(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LPrescriptionSphere", 999.0),TRUE));
				
				if (AdoFldDouble(rsOrder->Fields, "LCylinderValue", 999.0)!=999)
				{
				//SP_ORDER=>RX_DETAIL=>POSITION=>PRISCRIPTION=>CYLINDER
				pPrescriptionNode->appendChild(CreateNodevwOrderXMLPrescriptionCylinder( ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LCylinderValue", 999.0),TRUE),
												ConvertLongToString(AdoFldLong(rsOrder->Fields, "LeftCylinderAxis", -999))));
				}
				//SP_ORDER=>RX_DETAIL=>POSITION=>PRISCRIPTION=>ADDITION
				if (AdoFldDouble(rsOrder->Fields, "LAdditionValue", -999)!=-999)
					pPrescriptionNode->appendChild(CreateNodevwOrderXMLPrescriptionAddition(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LAdditionValue", 999.0))));
				//SP_ORDER=>RX_DETAIL=>POSITION=>PRISCRIPTION=>PRISM
				// (s.dhole 2012-02-07 14:35) - PLID 48004 - Make sure 0 is valid value and -999  is invalid
				// (s.dhole 2013-06-11 15:55) - PLID 57125 paas od/os
				if  (AdoFldDouble(rsOrder->Fields, "LPrismValue", -999)!=-999  && 
					(AdoFldLong(rsOrder->Fields, "LPrismAxis", -999)!=-999 || AdoFldString(rsOrder->Fields, "LPrismAxisStr", "")!=""))
					pPrescriptionNode->appendChild(CreateNodevwOrderXMLPrescriptionPrism(FALSE,    ConvertFloatToString(AdoFldDouble (rsOrder->Fields, "LPrismValue", 999.0)),
								ConvertLongToString(AdoFldLong(rsOrder->Fields, "LPrismAxis", -999)),
								AdoFldString(rsOrder->Fields, "LPrismAxisStr", "")));
				// (s.dhole, 2011-12-08 12:29) - PLID 46941 - added  PrismAxis2, PrismAxisStr2
				// (s.dhole 2012-02-07 14:35) - PLID 48004 - Make sure 0 is valid value and -999  is invalid
				// (s.dhole 2013-06-11 15:55) - PLID 57125 paas od/os
				if  (AdoFldDouble(rsOrder->Fields, "LPrismValue2", -999)!=-999  && 
					( AdoFldLong(rsOrder->Fields, "LPrismAxis2", -999)!=-999 || AdoFldString(rsOrder->Fields, "LPrismAxisStr2", "")!="" ))
					pPrescriptionNode->appendChild(CreateNodevwOrderXMLPrescriptionPrism( FALSE, ConvertFloatToString(AdoFldDouble (rsOrder->Fields, "LPrismValue2", 999.0)),
								ConvertLongToString(AdoFldLong(rsOrder->Fields, "LPrismAxis2", -999)),
								AdoFldString(rsOrder->Fields, "LPrismAxisStr2", "")));
				
				pPosition->appendChild(pPrescriptionNode);
				//SP_ORDER=>RX_DETAIL=>POSITION->LENS
				MSXML2::IXMLDOMNodePtr pLens = CreateNodevwOrderXMLPositionLens();
				//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>DIAMETER
				//pLens->appendChild(CreateNodevwOrderXMLPositionLensDiameter( "80","80"));
				//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>DESIGN
				strLeftDesignCode= AdoFldString(rsOrder->Fields, "LDesignCode", "");
				strLeftDesignInfo = FormatString("%s (%s)" ,AdoFldString(rsOrder->Fields, "LDesignName", ""),strLeftDesignCode);
				pLens->appendChild(CreateNodevwOrderXMLPositionLensDesign(strLeftDesignCode));
				//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>MATERIAL
				strLeftMaterialCode= AdoFldString(rsOrder->Fields, "LMaterialCode", "");
				strLeftMaterialInfo= FormatString("%s (%s)" ,AdoFldString(rsOrder->Fields, "LMaterialName", ""),strLeftMaterialCode);
				pLens->appendChild(CreateNodevwOrderXMLPositionLensMaterial(strLeftMaterialCode));
				//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>SEGMENT
				// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
				_RecordsetPtr rsTreatment = CreateParamRecordset(" SELECT vwoT.GlassesOrderOtherInfoID, \r\n"
							"GlassesCatalogTreatmentsT.TreatmentName, \r\n"
							" GlassesCatalogTreatmentsT.TreatmentCode, vwoT.GlassesCatalogTreatmentID, \r\n"
							" GlassesCatalogTreatmentsT.GlassesOrderProcessTypeID  \r\n"
							" FROM GlassesOrderTreatmentsT AS vwoT INNER JOIN \r\n"
							" GlassesCatalogTreatmentsT ON  \r\n"
							" vwoT.GlassesCatalogTreatmentID = GlassesCatalogTreatmentsT.ID \r\n"
							" WHERE vwoT.GlassesOrderOtherInfoID = {INT}",AdoFldLong(rsOrder->Fields, "LeftGlassesOrderOtherInfoID", 0));
					if(!rsTreatment->eof) {
						//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>TREATMENTS
						MSXML2::IXMLDOMNodePtr pTreatments= CreateNodevwOrderXMLPositionLensTreaments(AdoFldString(rsOrder->Fields, "LTreatmentsComment", ""));
						while(!rsTreatment->eof) {
							//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>TREATMENTS->TREATMENT
							pTreatments ->appendChild(CreateNodevwOrderXMLPositionLensTreamentsTreament(AdoFldString(rsTreatment->Fields, "TreatmentCode", ""),
								AdoFldString(rsTreatment->Fields, "TreatmentName", ""),""));
							rsTreatment->MoveNext();
						}
						//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>TREATMENTS
						pLens->appendChild(pTreatments);
						rsTreatment->Close(); 
					}
					else{
						rsTreatment->Close();
					}
				
				if (AdoFldDouble(rsOrder->Fields, "LThicknessValue", 999.0)!=0 && AdoFldString(rsOrder->Fields, "LThicknessType", "")!="DRS" && AdoFldString(rsOrder->Fields, "LThicknessType", "")!="" )
				{
					//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>THICKNESS
					pLens->appendChild(CreateNodevwOrderXMLPositionLensThickness( ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LThicknessValue", 999.0)),
									AdoFldString(rsOrder->Fields, "LThicknessType", "")));
				
				}
				//SP_ORDER=>RX_DETAIL=>POSITION->LENS
				pPosition->appendChild(pLens);
				//POSITION=>LENS=>POSITION=>SHAPE
				pPosition->appendChild(CreateNodevwOrderXMLPositionShape(ConvertFloatToString(AdoFldDouble (rsOrder->Fields, "ShapeA", 999.0)),
				ConvertFloatToString(AdoFldDouble (rsOrder->Fields, "ShapeB", 999.0)),
				ConvertFloatToString(AdoFldDouble (rsOrder->Fields, "ShapeHalfDbl", 999.0)),
				ConvertFloatToString(AdoFldDouble (rsOrder->Fields, "ShapeED", 999.0)))) ;
				
				//POSITION=>LENS=>POSITION
				pRX_Details->appendChild(pPosition );

			}
			
			// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
			if ( AdoFldLong(rsOrder->Fields, "RightGlassesOrderOtherInfoID", 0)>0 && AdoFldLong(rsOrder->Fields, "RightLensDetailRxID", 0)>0)
			{
				MSXML2::IXMLDOMNodePtr pPosition=CreateNodevwOrderXMLPosition( "R",
						ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "RFarHalfPd", 999.0)),
						ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "RSegHeight", 999.0)),
						ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "RNearHalfPd", 999.0)),
						ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "ROpticalCenter", 999.0)));
				//SP_ORDER=>RX_DETAIL=>POSITION=>PRISCRIPTION
				MSXML2::IXMLDOMNodePtr pPrescriptionNode =CreateNodevwOrderXMLPrescription(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "RPrescriptionSphere", 999.0),TRUE));
				if (AdoFldDouble(rsOrder->Fields, "RCylinderValue", 999.0)!=999)
				{
				//SP_ORDER=>RX_DETAIL=>POSITION=>PRISCRIPTION=>CYLINDER
				pPrescriptionNode->appendChild(CreateNodevwOrderXMLPrescriptionCylinder( ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "RCylinderValue", 999.0),TRUE),
												ConvertLongToString(AdoFldLong(rsOrder->Fields, "RCylinderAxis", -999))));
				}
				//SP_ORDER=>RX_DETAIL=>POSITION=>PRISCRIPTION=>ADDITION
				if (AdoFldDouble(rsOrder->Fields, "RAdditionValue", -999)!=-999)
					pPrescriptionNode->appendChild(CreateNodevwOrderXMLPrescriptionAddition(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "RAdditionValue", 999.0))));
				//SP_ORDER=>RX_DETAIL=>POSITION=>PRISCRIPTION=>PRISM
				// (s.dhole 2012-02-07 14:35) - PLID 48004 - Make sure 0 is valid value and -999  is invalid
				// (s.dhole 2013-06-11 15:55) - PLID 57125 paas od/os
				if  (AdoFldDouble (rsOrder->Fields, "RPrismValue", -999)!=-999  && 
					(AdoFldLong(rsOrder->Fields, "RPrismAxis", -999)!=-999  || AdoFldString(rsOrder->Fields, "RPrismAxisStr", "") !="") )
					pPrescriptionNode->appendChild(CreateNodevwOrderXMLPrescriptionPrism( TRUE, ConvertFloatToString(   AdoFldDouble(rsOrder->Fields, "RPrismValue", 999.0)),
								ConvertLongToString( AdoFldLong(rsOrder->Fields, "RPrismAxis", -999)),
								AdoFldString(rsOrder->Fields, "RPrismAxisStr", "") ));
				// (s.dhole, 2011-12-08 12:29) - PLID 46941 - added  PrismAxis2, PrismAxisStr2
				// (s.dhole 2012-02-07 14:35) - PLID 48004 - Make sure 0 is valid value and -999  is invalid
				// (s.dhole 2013-06-11 15:55) - PLID 57125 paas od/os
				if  (AdoFldDouble (rsOrder->Fields, "RPrismValue2", -999)!=-999  && 
					( AdoFldLong(rsOrder->Fields, "RPrismAxis2", -999)!=-999 || AdoFldString(rsOrder->Fields, "RPrismAxisStr2", "")!="") )
					pPrescriptionNode->appendChild(CreateNodevwOrderXMLPrescriptionPrism(TRUE, 
					ConvertFloatToString(   AdoFldDouble(rsOrder->Fields, "RPrismValue2", 999.0)),
								ConvertLongToString( AdoFldLong(rsOrder->Fields, "RPrismAxis2", -999)),
								AdoFldString(rsOrder->Fields, "RPrismAxisStr2", "") ));
				pPosition->appendChild(pPrescriptionNode);
					//SP_ORDER=>RX_DETAIL=>POSITION->LENS
				MSXML2::IXMLDOMNodePtr pLens = CreateNodevwOrderXMLPositionLens();
				//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>DIAMETER
				//pLens->appendChild(CreateNodevwOrderXMLPositionLensDiameter( "80","80"));
				//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>DESIGN
				strRightDesignCode = AdoFldString(rsOrder->Fields, "RDesignCode", "");
				strRightDesignInfo  = FormatString("%s (%s)" ,AdoFldString(rsOrder->Fields, "RDesignName", ""),strRightDesignCode );
				pLens->appendChild(CreateNodevwOrderXMLPositionLensDesign(strRightDesignCode));
				strRightMaterialCode = AdoFldString(rsOrder->Fields, "RMaterialCode", "");
				strRightMaterialInfo= FormatString("%s (%s)" ,AdoFldString(rsOrder->Fields, "RMaterialName", ""),strRightMaterialCode );
				//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>MATERIAL
				pLens->appendChild(CreateNodevwOrderXMLPositionLensMaterial( strRightMaterialCode));
				//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>SEGMENT
				// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
				_RecordsetPtr rsTreatment = CreateParamRecordset(" SELECT vwoT.GlassesOrderOtherInfoID, \r\n"
							"GlassesCatalogTreatmentsT.TreatmentName, \r\n"
							" GlassesCatalogTreatmentsT.TreatmentCode, vwoT.GlassesCatalogTreatmentID, \r\n"
							" GlassesCatalogTreatmentsT.GlassesOrderProcessTypeID  \r\n"
							" FROM GlassesOrderTreatmentsT AS vwoT INNER JOIN \r\n"
							" GlassesCatalogTreatmentsT ON  \r\n"
							" vwoT.GlassesCatalogTreatmentID = GlassesCatalogTreatmentsT.ID \r\n"
							  " WHERE vwoT.GlassesOrderOtherInfoID = {INT}",AdoFldLong(rsOrder->Fields, "RightGlassesOrderOtherInfoID", 0));
					if(!rsTreatment->eof) {
						//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>TREATMENTS
						MSXML2::IXMLDOMNodePtr pTreatments= CreateNodevwOrderXMLPositionLensTreaments(AdoFldString(rsOrder->Fields, "RTreatmentsComment", ""));
						while(!rsTreatment->eof) {
							//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>TREATMENTS->TREATMENT
							pTreatments ->appendChild(CreateNodevwOrderXMLPositionLensTreamentsTreament(AdoFldString(rsTreatment->Fields, "TreatmentCode", ""),
								AdoFldString(rsTreatment->Fields, "TreatmentName", ""),""));
							rsTreatment->MoveNext();
						}
						//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>TREATMENTS
						pLens->appendChild(pTreatments);
						rsTreatment->Close();  
					}
					else{
						rsTreatment->Close();
					}
				
					if (AdoFldDouble (rsOrder->Fields, "RThicknessValue", 999.0)!=999.0 &&  AdoFldString(rsOrder->Fields, "RThicknessType", "")!="DRS" &&  AdoFldString(rsOrder->Fields, "RThicknessType", "")!="")
					{
						//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>THICKNESS
						pLens->appendChild(CreateNodevwOrderXMLPositionLensThickness(ConvertFloatToString( AdoFldDouble (rsOrder->Fields, "RThicknessValue", 999.0)),
										AdoFldString(rsOrder->Fields, "RThicknessType", "")));
					}
				//SP_ORDER=>RX_DETAIL=>POSITION->LENS
				pPosition->appendChild(pLens);
				//POSITION=>LENS=>POSITION=>SHAPE
				pPosition->appendChild(CreateNodevwOrderXMLPositionShape(ConvertFloatToString(AdoFldDouble (rsOrder->Fields, "ShapeA", 999.0)),
				ConvertFloatToString(AdoFldDouble (rsOrder->Fields, "ShapeB", 999.0)),
				ConvertFloatToString(AdoFldDouble (rsOrder->Fields, "ShapeHalfDbl", 999.0)),
				ConvertFloatToString(AdoFldDouble (rsOrder->Fields, "ShapeED", 999.0) )));
				//POSITION=>LENS=>POSITION
				pRX_Details->appendChild(pPosition );
			}
			rsOrder->Close();
		//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>OTHER
		pSP_ORDER ->appendChild(pRX_Details);
		
		pRoot->appendChild(pSP_ORDER );
		if (CInvVisionWebUtils::ValidatevwOrderXML((LPCTSTR)pRoot->Getxml())==TRUE)
			return pRoot; 
		else
			return NULL;
	}
	else{
		rsOrder->Close();
		return NULL;
		}
}



// (s.dhole 2010-11-02 15:04) - PLID 40540 Submit order to VisionWeb
bool  CInvVisionWebUtils::UploadOrderToVisionWeb(long nOrderID)
{
	CString strvwOrderingUrl= GetRemotePropertyText(VISIONWEBSERVICE_ORDER_URL, "", 0, "<None>", true);
	CString strUserId= GetRemotePropertyText(VISIONWEBSERVICE_USER_ID, "", 0, "<None>", true);
	CString strPassword= GetVisionWebPasword();
	CString strRefId= GetRemotePropertyText(VISIONWEBSERVICE_REFID, "", 0, "<None>", true);
	CString strSupplierId, strLeftDesignCode,  strLeftMaterialCode, strRightDesignCode, strRightMaterialCode;
	CString strSupplierInfo, strLocationInfo,strLeftDesignInfo,strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo;
	long nOrderType=1; // this is Spectacle Lens Order
	//url and ref id should be part of default data, this should not be a issus,but as stander validation i am checking these values  
	// (s.dhole 2011-01-28 17:26) -PLID 41125  Make sure we hav valid user id and password
	if (strUserId.IsEmpty() || strPassword.IsEmpty() || strRefId.IsEmpty() || strPassword.GetLength()<4 || strUserId.GetLength()<4){ 
		AfxMessageBox( "The VisionWeb Order submission failed. \n\n"
				"Please ensure your login information is correct.\n\n"
				"If this message persists, please contact NexTech Technical Support." );
		return FALSE;
	}
	MSXML2::IXMLDOMNodePtr  pRoot  =CInvVisionWebUtils::GetOrderXML(nOrderID,strUserId,strPassword,
									strRefId,strSupplierId, strLeftDesignCode,  
									strLeftMaterialCode, strRightDesignCode, strRightMaterialCode, strSupplierInfo,strLocationInfo,
									strLeftDesignInfo,strRightDesignInfo,strLeftMaterialInfo,
									strRightMaterialInfo,nOrderType);
		if (pRoot != NULL){
			MSXML2::IXMLDOMNodePtr xmlResponse = SubmitOrderToVisionWeb(pRoot);
			if (xmlResponse !=NULL){
				MSXML2::IXMLDOMNodePtr xmlOrderResult= CInvVisionWebUtils::FindChildNode(xmlResponse,"submitOrderByLoginReturn");
				CString strExchangeId,strCreationDate,  strNumber  ,strReference ,strType, strErrorCode,strErrorDescription, strErrormessage;
				long nUserID = (long)GetCurrentUserID();
				if  (CInvVisionWebUtils::CheckOrderResoponse( xmlOrderResult,strExchangeId,strCreationDate,  
							strNumber,strReference,strType, strErrorCode,strErrorDescription, strErrormessage)){
					if (strErrorCode.IsEmpty() ){
						CParamSqlBatch batch;
						// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
						batch.Add("UPDATE GlassesOrderHistoryT SET IsActive=0 WHERE GlassesOrderID={INT}", nOrderID);
						batch.Add("INSERT INTO  GlassesOrderHistoryT "
							"(GlassesOrderID, GlassesOrderType,GlassesOrderStatusID, MessageType,  Note,IsActive,UpdateDate,UserID)"
							"VALUES ({INT},{INT},{INT},{STRING},{STRING},1,Getdate(),{INT})"
							, nOrderID,nOrderType,2, strType, "Order status = Submit to VisionWeb Order System",nUserID);
						// (s.dhole 2011-04-26 15:57) - PLID 43077 added GlassesOrderProcessTypeID=2 VisionWeb process flag
						// (r.wilson 4/11/2012) PLID 43741 - Changed GlassesOrderStatus to GlassesOrderStatusID
						batch.Add("UPDATE GlassesOrderT SET  GlassesOrderNumber={STRING}, GlassesMessage={STRING} ,GlassesOrderStatusID=2,GlassesOrderProcessTypeID=2 , "
							" OrderUploadDate=GetDate(),UpdateDate=Getdate()  " 
							",UserID={INT}, VisionWebOrderExchangeID ={STRING}  WHERE ID={INT}" ,
							strNumber,"Order status = Submit to VisionWeb Order System", nUserID, strExchangeId, nOrderID);
						batch.Execute(GetRemoteData());
					}
					else
					{// error
						// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
						// (r.wilson 4/11/2012) PLID 43741 - Changed GlassesOrderStatus to GlassesOrderStatusID
						ExecuteParamSql( "UPDATE GlassesOrderT SET  GlassesMessage={STRING} ,GlassesOrderStatusID=3,UpdateDate=Getdate() ,UserID={INT}  WHERE ID={INT}" ,strErrorDescription,nUserID, nOrderID);
						// (s.dhole 2010-11-02 15:04) - PLID 41546 Get error description
						strErrorDescription +=	GetvwErrorDescription(strErrorCode,	strSupplierId, strLeftDesignCode, strLeftMaterialCode,
							strRightDesignCode, strRightMaterialCode,strSupplierInfo,strLocationInfo,strLeftDesignInfo,strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo,TRUE);
						CInvVisionWebUtils::ShowErrors(strErrorDescription ,nOrderID);
						AfxMessageBox(FormatString("The VisionWeb Order submission failed with the following error:\n\n%s\n",GetvwErrorDescription(strErrorCode)));
						return FALSE;
					}
				}
			else
				{// error
					// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
					// (r.wilson 4/11/2012) PLID 43741 - Changed GlassesOrderStatus to GlassesOrderStatusID
					ExecuteParamSql( "UPDATE GlassesOrderT SET  GlassesMessage={STRING} ,GlassesOrderStatusID=3, UpdateDate=Getdate(),UserID={INT} WHERE ID={INT}" ,strErrorDescription,nUserID, nOrderID);
					strErrorDescription += FormatString("\r\n  Order #: %d \r\n", nOrderID) ;
					// (s.dhole 2010-11-02 15:04) - PLID 41546 Get error description
					strErrorDescription +=	GetvwErrorDescription(strErrorCode,	strSupplierId, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,
						strSupplierInfo,strLocationInfo,strLeftDesignInfo,strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo, TRUE);
					CInvVisionWebUtils::ShowErrors(strErrorDescription ,nOrderID);
					AfxMessageBox("The VisionWeb Order submission failed:\n\n"
						"Please ensure your login information is correct, and that you have access to the internet.\n\n"
						"If this message persists, please contact NexTech Technical Support.");
					return FALSE;
				}
			}
			else{
				return FALSE;
			}
		}
	return TRUE;
}





// (s.dhole 2010-11-02 15:04) - PLID 40540  long into string This functiona will return formated string
CString  CInvVisionWebUtils::ConvertLongToString(long  val)
{
	CString  strTemp;
	if (val!=-999)
		strTemp.Format("%d",val); 
	return strTemp;
}


// (s.dhole 2010-11-02 15:04) - PLID 40540  fload/decimal into formated string
// This functiona will return formated string
CString  CInvVisionWebUtils::ConvertDecimalStringFromString(CString   val)
{
	CString  strTemp;
	if (!val.IsEmpty() )
		return ConvertFloatToString(atof( val),FALSE);
	return strTemp;
}


// (s.dhole 2010-11-02 15:04) - PLID 40540 
// SOAP functionality to submit order to Visionweb
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::SubmitOrderToVisionWeb(MSXML2::IXMLDOMNodePtr  pOrderxml ){
	if (pOrderxml!=NULL){
		CString strXMLOrder;
		CString strURL = GetRemotePropertyText(VISIONWEBSERVICE_ORDER_URL, "", 0, "<None>", true);
		CString strUserID = GetRemotePropertyText(VISIONWEBSERVICE_USER_ID, "", 0, "<None>", true);
		CString strPSW = GetVisionWebPasword();
		CString strRefId = GetRemotePropertyText(VISIONWEBSERVICE_REFID, "", 0, "<None>", true);
		// (s.dhole 2011-11-30 09:08) - PLID 46509 submitting refid node with Refid information not user id
		// visionWeb QA was accepting userid as ref id but now they change at both production and QA server
		strXMLOrder.Format(  "<submitOrderByLogin>"
			"<msg><![CDATA[%s]]></msg>" 
			"<UserName>%s</UserName>"
			"<pswd>%s</pswd>"
			"<refid>%s</refid>"
			"</submitOrderByLogin>" 
			,(LPCTSTR)pOrderxml->Getxml(),strUserID, strPSW, strRefId );
		return CInvVisionWebUtils::CallSoapFunction(strURL,strURL,strXMLOrder,strUserID ,strPSW);
	}
	else{
		return NULL;
	}
}

// (s.dhole 2010-11-02 15:04) - PLID 41546 If there is error in order we will read response from Visionweb and show it to client in notepad
void CInvVisionWebUtils::ShowErrors(const CString &strOutputString,const long nOrderID ) {
	try {
		CString pathname;
		CFile	OutputFile;
		// (j.armen 2011-10-25 15:12) - PLID 46137 - Save the visionweb error's along with each session
		pathname = GetPracPath(PracPath::SessionPath) + "\\VisionWebOrderErrorDetail.txt";
		if(!OutputFile.Open(pathname,CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
			CreateDirectory(GetPracPath(PracPath::SessionPath), NULL);
			if(!OutputFile.Open(pathname,CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
				AfxMessageBox("The VisionWeb order detail information file could not be created. Contact NexTech for assistance.");
			}
		}
		CString strDate, strUserName;
		COleDateTime dt;
		dt = COleDateTime::GetCurrentTime();
		strDate = FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoDateTime);
		strUserName = GetCurrentUserName();
		//add Header
		CString strHerader;
		strHerader.Format("				VisionWeb Order # %d \r\n\r\nUser: %s\t\t\t\t\tDate/Time: %s \r\n\r\n\r\n",nOrderID,strUserName,strDate);
		OutputFile.Write(strHerader,strHerader.GetLength());
		OutputFile.Write(strOutputString,strOutputString.GetLength());
		OutputFile.Close();
		int nResult = (int)ShellExecute ((HWND)this, NULL, "notepad.exe", ("'" + pathname + "'"), NULL, SW_SHOW);
	} NxCatchAll("Error in writing 'VisionWeb Order Detail' file.");
}

// (s.dhole 2010-11-02 15:04) - PLID 41546
//This functioan will call error detail services and get more information and return as string
CString  CInvVisionWebUtils::GetvwOrderErrorDetail(const CString &strSupplierID,const  CString &strLeftDesignCode,
														  const  CString &strLeftMaterialCode,const  CString &strRightDesignCode,
														  const  CString &strRightMaterialCode, const  CString &strSupplierInfo, const CString &strLocationInfo,
														  const  CString &strLeftDesignInfo,const  CString &strRightDesignInfo,
														  const  CString &strLeftMaterialInfo,const  CString &strRightMaterialInfo)
{
	CString strURL = GetRemotePropertyText(VISIONWEBSERVICE_ORDER_ERROR_DETAIL_URL, "", 0, "<None>", true);
	CString strUserID = GetRemotePropertyText(VISIONWEBSERVICE_USER_ID, "", 0, "<None>", true);
	CString strPSW = GetVisionWebPasword();
	CString strRefId = GetRemotePropertyText(VISIONWEBSERVICE_REFID, "", 0, "<None>", true);
	CString strErrorDescription;
	strErrorDescription += "\r\n\r\n\r\n\r\n";
	strErrorDescription += "\r\n\r\n********************************************************************************\r\n";
	strErrorDescription += "**************************** Additional Information ****************************\r\n";
	strErrorDescription += "********************************************************************************\r\n\r\n";
	// (s.dhole 2012-05-25 17:57) - PLID 50617
	strErrorDescription += "	Supplier/Lab : " ;
	strErrorDescription += strSupplierInfo ;
	strErrorDescription += "\r\n";
	strErrorDescription += "	Location : " ;
	strErrorDescription += strLocationInfo ;
	strErrorDescription += "\r\n\r\n--------------------------------------------------------------------------------\r\n";
	MSXML2::IXMLDOMNodePtr xmlResponse ;
	if (strRightDesignCode != ""  &&  strRightMaterialCode !=""){
		xmlResponse =GetErrorForDesignMaterialFromvw(strURL,strUserID,strPSW,strRefId,strSupplierID,strRightDesignCode,strRightMaterialCode);
		if (xmlResponse!=NULL){
			MSXML2::IXMLDOMNodePtr xmlOrderErroDetailResult= CInvVisionWebUtils::FindChildNode(xmlResponse,"getErrorSupportByLoginResponse");
			if (xmlOrderErroDetailResult!=NULL){
				if (strLeftDesignCode !=strRightDesignCode || strLeftMaterialCode !=strRightMaterialCode)
					strErrorDescription += "\r\n\r\n--------------------------- Right Eye(OD) Power Range --------------------------\r\n";
				else
					strErrorDescription += "\r\n\r\n-------------------------------- Eye Power Range -------------------------------\r\n";
				strErrorDescription += " Eye Power range  \r\nDesign = " ;
				strErrorDescription +=  strRightDesignInfo ;
				strErrorDescription += "  \r\nMaterial = " ;
				strErrorDescription +=  strRightMaterialInfo ;
				strErrorDescription += "\r\n";
				strErrorDescription += CInvVisionWebUtils::GetvwOrderErrorLensDetail((LPCTSTR)xmlOrderErroDetailResult->Getxml());
				strErrorDescription += "--------------------------------------------------------------------------------\r\n";
			}
		}
	}
	if (strRightDesignCode != "" ){
		xmlResponse =GetErrorForDesignMaterialFromvw(strURL,strUserID,strPSW,strRefId,strSupplierID,strRightDesignCode,"");
		if (xmlResponse!=NULL){
			MSXML2::IXMLDOMNodePtr xmlOrderErroDetailResult= CInvVisionWebUtils::FindChildNode(xmlResponse,"getErrorSupportByLoginResponse");
			if (xmlOrderErroDetailResult!=NULL){
				if (strLeftDesignCode !=strRightDesignCode || strLeftMaterialCode !=strRightMaterialCode)
					strErrorDescription += "\r\n\r\n------- List of associated materials for selected Design :-Right Eye(OD) -------\r\n";
				else
					strErrorDescription += "\r\n\r\n---------------- List of associated materials for selected Design --------------\r\n";
				strErrorDescription += "Selected Design = " ;
				strErrorDescription +=  strRightDesignInfo;
				strErrorDescription += "\r\n";
				strErrorDescription += CInvVisionWebUtils::GetvwOrderErrorDesignMaterialDetail((LPCTSTR)xmlOrderErroDetailResult->Getxml(),
					"MATERIALS","Code","Desc");
			}
		}
	}
	if (strRightMaterialCode !=""){
		xmlResponse =GetErrorForDesignMaterialFromvw(strURL,strUserID,strPSW,strRefId,strSupplierID,"",strRightMaterialCode);
		if (xmlResponse!=NULL){
			MSXML2::IXMLDOMNodePtr xmlOrderErroDetailResult= CInvVisionWebUtils::FindChildNode(xmlResponse,"getErrorSupportByLoginResponse");
			if (xmlOrderErroDetailResult!=NULL){
				if (strLeftDesignCode !=strRightDesignCode || strLeftMaterialCode !=strRightMaterialCode)
					strErrorDescription += "\r\n\r\n------ List of associated designs for selected Material :-Right Eye(OD) --------\r\n";
				else
					strErrorDescription += "\r\n\r\n--------------- List of associated designs for selected Material ---------------\r\n";
				strErrorDescription += "Selected Material = " ;
				strErrorDescription +=  strRightMaterialInfo;
				strErrorDescription += "\r\n";
				strErrorDescription += CInvVisionWebUtils::GetvwOrderErrorDesignMaterialDetail((LPCTSTR)xmlOrderErroDetailResult->Getxml(),"DESIGNS",
					"Code","Desc");
			}
		}
	}

	if (strLeftDesignCode != ""  &&  strLeftMaterialCode !=""){
		xmlResponse =GetErrorForDesignMaterialFromvw(strURL,strUserID,strPSW,strRefId,strSupplierID,strLeftDesignCode,strLeftMaterialCode);
		if (xmlResponse!=NULL){
			MSXML2::IXMLDOMNodePtr xmlOrderErroDetailResult= CInvVisionWebUtils::FindChildNode(xmlResponse,"getErrorSupportByLoginResponse");
			if (xmlOrderErroDetailResult!=NULL){
				if (strLeftDesignCode !=strRightDesignCode || strLeftMaterialCode !=strRightMaterialCode)
					strErrorDescription += "\r\n\r\n---------------------------- Left Eye(OS) Power Range --------------------------\r\n";
				else
					strErrorDescription += "\r\n\r\n-------------------------------- EyePower Range --------------------------------\r\n";
				strErrorDescription += " Eye Power range \r\nDesign  =" ;
				strErrorDescription +=  strLeftDesignInfo ;
				strErrorDescription += " and Material =" ;
				strErrorDescription +=  strLeftMaterialInfo ;
				strErrorDescription += "\r\n";
				strErrorDescription += CInvVisionWebUtils::GetvwOrderErrorLensDetail((LPCTSTR)xmlOrderErroDetailResult->Getxml());
				strErrorDescription += " -------------------------------------------------------------------------------\r\n";				
			}
		}
	}
	if (strLeftDesignCode != "" ){
		xmlResponse =GetErrorForDesignMaterialFromvw(strURL,strUserID,strPSW,strRefId,strSupplierID,strLeftDesignCode,"");
		if (xmlResponse!=NULL){
			MSXML2::IXMLDOMNodePtr xmlOrderErroDetailResult= CInvVisionWebUtils::FindChildNode(xmlResponse,"getErrorSupportByLoginResponse");
			if (xmlOrderErroDetailResult!=NULL){
				if (strLeftDesignCode !=strRightDesignCode || strLeftMaterialCode !=strRightMaterialCode)
					strErrorDescription += "\r\n\r\n------- List of associated materials for selected Design :- Left Eye(OS) -------\r\n";
				else
					strErrorDescription += "\r\n\r\n--------------- List of associated material for selected Design ----------------\r\n";
				strErrorDescription += "Design = " ;
				strErrorDescription +=  strLeftDesignInfo;
				strErrorDescription += "\r\n";
				strErrorDescription += CInvVisionWebUtils::GetvwOrderErrorDesignMaterialDetail((LPCTSTR)xmlOrderErroDetailResult->Getxml(),
					"MATERIALS","Code","Desc");
			}
		}
	}
	if ( strLeftMaterialCode !="")
	{
		xmlResponse =GetErrorForDesignMaterialFromvw(strURL,strUserID,strPSW,strRefId,strSupplierID,"",strLeftMaterialCode);
		if (xmlResponse!=NULL)
		{
			MSXML2::IXMLDOMNodePtr xmlOrderErroDetailResult= CInvVisionWebUtils::FindChildNode(xmlResponse,"getErrorSupportByLoginResponse");
			if (xmlOrderErroDetailResult!=NULL)
			{
				if (strLeftDesignCode !=strRightDesignCode || strLeftMaterialCode !=strRightMaterialCode)
					strErrorDescription += "\r\n\r\n------- List of associated designs for selected Material :-Left Eye(OS) --------\r\n";
				else
					strErrorDescription += "\r\n\r\n------------- List of associated designs for selected Material Code ------------\r\n";
				strErrorDescription += "Selected Material = " ;
				strErrorDescription +=  strLeftMaterialInfo ;
				strErrorDescription += "\r\n";
				strErrorDescription += CInvVisionWebUtils::GetvwOrderErrorDesignMaterialDetail((LPCTSTR)xmlOrderErroDetailResult->Getxml(),"DESIGNS",
					"Code","Desc");
			}
		}
	}
	strErrorDescription += "\r\n********************************************************************************\r\n\r\n";
	return strErrorDescription;
}

// (s.dhole 2010-11-02 15:04) - PLID 41546 
// This finction will parse error xml and return string
CString CInvVisionWebUtils::GetvwOrderErrorLensDetail(LPCTSTR strResult)
{
	CString  strDescription ;
	try{
		
		HR(pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60)));
		_bstr_t strXML= CInvVisionWebUtils::XMLDecode( strResult);
		pDoc->loadXML ( strXML) ;
		MSXML2::IXMLDOMNodePtr pNode;
		if (pDoc->GetchildNodes()->length>1) 
			pNode =CInvVisionWebUtils::FindChildNode(pDoc->GetchildNodes()->Getitem(1)     , "POWER_RANGES")   ;
		else
			pNode =CInvVisionWebUtils::FindChildNode(pDoc->GetfirstChild()   , "POWER_RANGES")   ;
		
		if (pNode!=NULL)
		{ 
			MSXML2::IXMLDOMNodeListPtr pChildeNodes  = pNode->GetchildNodes();
			long nCount = pChildeNodes->Getlength()  ;
			for (long i=0; i<nCount; i++) {
				MSXML2::IXMLDOMNodePtr pChildeNode = pChildeNodes->Getitem(i);
				strDescription += "\r\n        ";
				strDescription += (LPCTSTR)pChildeNode->GetbaseName()  ;
				MSXML2::IXMLDOMNamedNodeMapPtr pAttributes=pChildeNode->Getattributes();
				long nCountAttributes = pAttributes->Getlength();
				for (long nAtttribut=0; nAtttribut<nCountAttributes; nAtttribut++) {
					if (CString((LPCTSTR) pAttributes->Getitem(nAtttribut)->GetbaseName()).CompareNoCase("From")==0) 
					{
						//Add 20  hite spaces
					for(int i =0 ; i<= 20 -CString((LPCTSTR)pChildeNode->GetbaseName() ).GetLength() ;i++) 
						strDescription += " " ;

						strDescription += " From = ";
						strDescription +=(CString)(LPCTSTR)pAttributes->Getitem(nAtttribut)->Gettext()  ;
					}
					else if (CString((LPCTSTR) pAttributes->Getitem(nAtttribut)->GetbaseName()).CompareNoCase("To")==0)
					{
						strDescription += " To = ";
						strDescription +=(CString)(LPCTSTR)pAttributes->Getitem(nAtttribut)->Gettext();
					}
				}
			}
				strDescription += "\r\n";
		}
		return strDescription;
		}NxCatchAll("Error in CCInvVisionWebUtils::GetvwOrderErrorLensDetail");
	return strDescription;
}

// (s.dhole 2010-11-02 15:04) - PLID 40728 Update Catalog/Sync
CString CInvVisionWebUtils::GetvwOrderErrorDesignMaterialDetail(LPCTSTR strResult,LPCTSTR strType, LPCTSTR strCode ,LPCTSTR strDescription)
{
	CString  strDetail;
	try{
		
		HR(pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60)));
		_bstr_t strXML= CInvVisionWebUtils::XMLDecode( strResult);
		pDoc->loadXML ( strXML) ;
		MSXML2::IXMLDOMNodePtr pNode;
		if (pDoc->GetchildNodes()->length>1) 
			pNode =CInvVisionWebUtils::FindChildNode(pDoc->GetchildNodes()->Getitem(1)     , strType)   ;
		else
			pNode =CInvVisionWebUtils::FindChildNode(pDoc->GetfirstChild()   , strType)   ;
		
		if (pNode!=NULL){ 
			MSXML2::IXMLDOMNodeListPtr pChildeNodes  = pNode->GetchildNodes();
			long nCount = pChildeNodes->Getlength()  ;
			for (long i=0; i<nCount; i++) {
				MSXML2::IXMLDOMNodePtr pChildeNode = pChildeNodes->Getitem(i);
				strDetail += "    ";
				MSXML2::IXMLDOMNamedNodeMapPtr pAttributes=pChildeNode->Getattributes();
				long nCountAttributes = pAttributes->Getlength();
				for (long nAtttribut=0; nAtttribut<nCountAttributes; nAtttribut++) {
					if (CString((LPCTSTR) pAttributes->Getitem(nAtttribut)->GetbaseName()).CompareNoCase(strCode)==0) {
						strDetail += " Code = ";
						strDetail +=(CString)(LPCTSTR)pAttributes->Getitem(nAtttribut)->Gettext()  ;
					}
					else if (CString((LPCTSTR) pAttributes->Getitem(nAtttribut)->GetbaseName()).CompareNoCase(strDescription)==0){
						strDetail += "; Name = ";
						strDetail +=(CString)(LPCTSTR)pAttributes->Getitem(nAtttribut)->Gettext();
						strDetail += " \r\n";
					}
				}
			}
		}
		return strDetail;
		}NxCatchAll("Error in CCInvVisionWebUtils::GetvwOrderErrorDesignMaterialDetail");
	return strDetail;
}


// (s.dhole 2010-11-02 15:04) - PLID 40540 
// SOAP functionality to submit order to Visionweb
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::GetErrorForDesignMaterialFromvw(const CString &strURL , const CString &strUserID,const CString &strPSW,
		const CString &strRefID,const CString &strSupplierID,const CString &strDesignCode,const CString &strMaterialCode)
{
	CString strXML;
	strXML.Format(  "<getErrorSupportByLogin>"
		"<UserName>%s</UserName>"
		"<pswd>%s</pswd>"
		"<refid>%s</refid>"
		"<supid>%s</supid>"
		"<vwdesign>%s</vwdesign>"
		"<vwmaterial>%s</vwmaterial>"
		"</getErrorSupportByLogin>" 
		,strUserID,strPSW, strRefID,strSupplierID,strDesignCode,strMaterialCode);
	return CInvVisionWebUtils::CallSoapFunction(strURL,strURL,strXML,strUserID ,strPSW);
	
}

// (s.dhole 2010-11-15 13:12) - PLID 41397 
// this functiona will return standerd description for visionweb Error Code
CString  CInvVisionWebUtils::GetvwTrackingCodeDescription(const CString &strTrackingCode)
{	
	CString strTrackingCodeDescription;
	if (strTrackingCode=="TRK000")
		strTrackingCodeDescription += "Sending the order";
	else if (strTrackingCode=="TRK002")
		// (s.dhole 2012-05-25 17:57) - PLID 50617
		strTrackingCodeDescription += "Sent to supplier/lab";
	else if (strTrackingCode=="TRK005") 
		strTrackingCodeDescription += "Received";
	else if (strTrackingCode=="TRK007") 
		strTrackingCodeDescription += "Order in process";
	else if (strTrackingCode=="TRK010") 
		strTrackingCodeDescription += "Waiting for Information";
	else if (strTrackingCode=="TRK015") 
		strTrackingCodeDescription += "Waiting for frame";
	else if (strTrackingCode=="TRK020") 
		strTrackingCodeDescription += "Rx Launch";
	else if (strTrackingCode=="TRK025") 
		strTrackingCodeDescription += "Surfacing";
	else if (strTrackingCode=="TRK030") 
		strTrackingCodeDescription += "Treatment";
	else if (strTrackingCode=="TRK035") 
		strTrackingCodeDescription += "Tinting";
	else if (strTrackingCode=="TRK040") 
		strTrackingCodeDescription += "Finishing";
	else if (strTrackingCode=="TRK045") 
		strTrackingCodeDescription += "Inspection";
	else if (strTrackingCode=="TRK050") 
		strTrackingCodeDescription += "Breakage – Redo";
	else if (strTrackingCode=="TRK055") 
		strTrackingCodeDescription += "Shipping";
	else if (strTrackingCode=="TRK060") 
		strTrackingCodeDescription += "Shipped";
	else if (strTrackingCode=="TRK900")
		strTrackingCodeDescription += "Order Cancelled";
	else if (strTrackingCode=="TRK999")
		strTrackingCodeDescription += "Other Error";
	return strTrackingCodeDescription;
}

// (s.dhole 2010-11-02 15:04) - PLID 41546 
// Match Error code wityh error description, this is standerd visionweb error description table
// (s.dhole 2011-02-18 16:46) change formating and some correction
CString  CInvVisionWebUtils::GetvwErrorDescription(const CString &strErrorCode,const CString &strSupplierID,
														  const  CString &strLeftDesignCode,const  CString &strLeftMaterialCode,
														const  CString &strRightDesignCode,const  CString &strRightMaterialCode,
														const  CString &strSupplierInfo, const CString &strLocationInfo, const  CString &strLeftDesignInfo,
														const  CString &strRightDesignInfo,const  CString &strLeftMaterialInfo,
														const  CString &strRightMaterialInfo,BOOL ISFormated )
{
	CString strErrorDescription;
	if (ISFormated) 
		strErrorDescription += "\r\n\r\n\r\n****************************** Error Description *******************************\r\n\r\n";

	if (strErrorCode=="RCV001"){
		strErrorDescription += "Invalid HEADER data \r\n   " ;
		strErrorDescription += "If this message persists, please contact NexTech Technical Support. \r\n";
		if (ISFormated) 
			strErrorDescription += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="RCV002"){
		strErrorDescription += "Error in XML processing\r\n   " ;
		strErrorDescription += "If this message persists, please contact NexTech Technical Support. \r\n";
		if (ISFormated) 
			strErrorDescription += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="RCV003"){
		strErrorDescription += "Duplicate Message Id \r\n   " ;
		strErrorDescription += "Duplicate Order Message. \r\n";
		if (ISFormated) 
			strErrorDescription += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="SCH001"){
		strErrorDescription += "Invalid Schema \r\n   " ;
		strErrorDescription += "If this message persists, please contact NexTech Technical Support.\r\n";
		if (ISFormated) 
			strErrorDescription += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN001"){
		strErrorDescription += "Username is not valid. \r\n";
		if (ISFormated) 
			strErrorDescription += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN002"){
		strErrorDescription += "Username is disabled. \r\n";
		if (ISFormated) 
			strErrorDescription += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN003"){
		strErrorDescription += "Username is locked. \r\n";
		if (ISFormated) 
			strErrorDescription += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN004"){
		strErrorDescription += "Username is deleted. \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN005"){
		strErrorDescription += "Incorrect password \r\n   " ;
		strErrorDescription += "Password is not valid.  \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN006"){
		strErrorDescription += "Invalid CbsId \r\n   " ;
		strErrorDescription += "Invalid User - VisionWeb Account(Location VisionWeb code). \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN006A"){
		strErrorDescription += "Invalid SupplierId /CbsId\r\n   " ;
		// (s.dhole 2012-05-25 17:57) - PLID 50617
		strErrorDescription += "Supplier/Lab VisionWeb Code/VisionWeb Account(Location VisionWeb code). \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN007"){
		strErrorDescription += "Invalid Frame Type code \r\n   " ; 
		// (s.dhole 2012-05-25 17:57) - PLID 50617
		strErrorDescription += "Supplier/Lab won’t support a Frame Type you selected for an order. \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN008"){
		strErrorDescription += "Invalid Design Code \r\n   " ;
		// (s.dhole 2012-05-25 17:57) - PLID 50617
		strErrorDescription += "Lens Design: Supplier/Lab won’t support a Design you selected for an order.\r\n";
		if (ISFormated) {
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
		}
	}
	else if (strErrorCode=="TRN009"){
		strErrorDescription += "Material code not valid\r\n   " ;
		// (s.dhole 2012-05-25 17:57) - PLID 50617
		strErrorDescription += "Lens Material: Supplier/Lab won’t support a Material you selected for an order.\r\n";
		if (ISFormated) {
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
		}
	}
	else if (strErrorCode=="TRN010"){
		strErrorDescription += "Invalid Design/Material Code \r\n   " ;
		// (s.dhole 2012-05-25 17:57) - PLID 50617
		strErrorDescription += "Lens Design/Lens Material: Supplier/Lab won’t support a Design/Material you selected for an order.\r\n";
		if (ISFormated) {
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(	strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
		}
	}
	else if (strErrorCode=="TRN011"){
		strErrorDescription += "Invalid Treatment Code \r\n   " ;
		// (s.dhole 2012-05-25 17:57) - PLID 50617
		strErrorDescription += "Treatment Code: Supplier/Lab won’t support a Treatment you selected for an order.\r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN012"){
		strErrorDescription += "Invalid shape \r\n   " ;
		strErrorDescription += "Standard Shape: Not valid VisionWeb standard shape value. \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN013"){
		strErrorDescription += "Invalid Job Type \r\n   " ;
		strErrorDescription += "Job Type: Not valid supplier/lab Job Type \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN014"){
		strErrorDescription += "Patient Info required \r\n   " ;
		strErrorDescription += "Patient First/Last name required. \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN015"){
		strErrorDescription += "Duplicate order\r\n   " ;
		strErrorDescription += "Duplicate order, order exists with VisionWeb system. \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN016"){
		strErrorDescription += "Multiple orders with same Submitter Order Id. \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN017"){
		strErrorDescription += "Invalid Reason code \r\n   " ;
		strErrorDescription += "Supplied redo reason code is not valid. \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN018"){
		strErrorDescription += "Selected supplier/lab won’t support the Base curve. \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN019"){
		strErrorDescription += "Supplier/Lab won't support the Equithin \r\n   " ;
		strErrorDescription += "Selected supplier/lab won’t support the selected Equithin. \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN020"){
		strErrorDescription += "Supplier/Lab won’t support the Redo \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN021"){
		strErrorDescription += "Selected Treatments are incompatible \r\n   " ;
		strErrorDescription += "Selected treatment is not compatible with grouped treatments \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN022"){
		strErrorDescription += "Equithin is required for both eyes\r\n   " ;
		strErrorDescription += "Equithin value needs to be supplied for both eyes \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="TRN099"){
		strErrorDescription += "Fatal Error\r\n   " ;
		strErrorDescription += "Fatal Error occurred please contact VisionWeb or NexTech Technical Support. \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="DAT001"){
		strErrorDescription += "Right Eye (OD) : Far Half PD required \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="DAT002"){
		strErrorDescription += "Left Eye (OS) - :Far Half PD required \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="DAT003"){
		strErrorDescription += "Prescription: Sphere required \r\n";
		if (ISFormated) {
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(	strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
		}
	}
	else if (strErrorCode=="DAT004"){
		strErrorDescription += "Prescription: Right Eye (OD) : Cylinder Axis required \r\n";
		if (ISFormated) {
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(	strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
		}
	}
	else if (strErrorCode=="DAT005"){
		strErrorDescription += "Prescription: Left Eye(OS): Cylinder Axis required \r\n";
		if (ISFormated) {
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(	strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
		}
	}
	else if (strErrorCode=="DAT006"){
		strErrorDescription += "Prescription: WARNING: Opposite Spheres \r\n   " ;
		strErrorDescription += "Warning: Order contains opposite spheres \r\n";
		if (ISFormated) {
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(	strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
		}
	}
	else if (strErrorCode=="DAT007"){
		strErrorDescription += "Prescription: WARNING: Opposite cylinders \r\n   " ;
		strErrorDescription += "Warning: Order contains opposite cylinders \r\n";
		if (ISFormated) {
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(	strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
		}
	}
	else if (strErrorCode=="DAT008"){
		strErrorDescription += "Prescription: WARNING: Opposite Additions \r\n   " ;
		strErrorDescription += "Warning: Order contains opposite Additions \r\n";
		if (ISFormated) {
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(	strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
		}
	}
	else if (strErrorCode=="DAT009"){
		strErrorDescription += "Prescription: Prism- Axis values are same or opposite \r\n   " ;
		strErrorDescription += "Prism Axis value are same or opposite \r\n";
		if (ISFormated){ 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(	strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
		}
	}
	else if (strErrorCode=="DAT010"){
		strErrorDescription += "Shape: A is required \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="DAT011"){
		strErrorDescription += "Shape: B is required  \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="DAT012"){
		strErrorDescription += "Shape: Half Dbl is required  \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="DAT013"){
		strErrorDescription += "Shape: ED is required  \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="DAT014"){
		strErrorDescription += "Shape: ED must be greater than A  \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="DAT015"){
		strErrorDescription += "Far Half PD is out of bounds  \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="DAT016"){
		strErrorDescription += "Near Half PD is out of bounds \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="DAT017"){
		strErrorDescription += "Height is out of bounds \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="DAT018"){
		strErrorDescription += "Right Eye (OD): Seg Height is required.\r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="DAT099"){
		strErrorDescription += "Data Validation Error \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="VAL001"){
		strErrorDescription += "A Box is required \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="VAL002"){
		strErrorDescription += "B Box is required \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="VAL003"){
		strErrorDescription += "Height is required \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="VAL004"){
		strErrorDescription += "PD is required \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="VAL005"){
		strErrorDescription += "PD must be under the B Box \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="VAL006"){
		strErrorDescription += "Height must be under the B Box \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="VAL007"){
		strErrorDescription += "36 Points required \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="VAL008"){
		strErrorDescription += "Executive lenses Dia. calculation not implemented \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="VAL009"){
		strErrorDescription += "Generic lenses Dia. calculation not implemented \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="VAL010"){
		strErrorDescription += "Unknown Lens Type \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="VAL011"){
		strErrorDescription += "No Radius \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="VAL099"){
		strErrorDescription += "Shape Calculation Errors \r\n   " ;
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="CAT001"){
		strErrorDescription += "Lens Not available in Power range \r\n   " ;
		if (ISFormated) {
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(	strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
		}
	}
	else if (strErrorCode=="CAT001A"){
		strErrorDescription += "Right Eye (OD) - Lens Not available in Power range \r\n";
		if (ISFormated) {
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(	strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
		}

	}
	else if (strErrorCode=="CAT001B"){
		strErrorDescription += "Left Eye - Lens Not available in Power range \r\n";
		if (ISFormated){ 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(	strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
		}
	}
	else if (strErrorCode=="CAT002"){
		strErrorDescription += "Minimum calculated diameter is greater than product available. \r\n";
		if (ISFormated) {
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(	strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
			}
	}
	else if (strErrorCode=="CAT002A"){
		strErrorDescription += "Right Eye (OD) - Minimum calculated diameter is greater than product available \r\n";
		if (ISFormated){
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(	strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
		}
	}
	else if (strErrorCode=="CAT002B"){
		strErrorDescription += "Left Eye (OS) - Minimum calculated diameter is greater than product available \r\n";
		if (ISFormated){
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
			strErrorDescription += GetvwOrderErrorDetail(	strSupplierID, strLeftDesignCode, strLeftMaterialCode, strRightDesignCode, strRightMaterialCode,strSupplierInfo, strLocationInfo, strLeftDesignInfo,
														strRightDesignInfo,strLeftMaterialInfo,strRightMaterialInfo);
		}

	}
	else if (strErrorCode=="ORD001"){
		strErrorDescription += "Order Xml Schema failed\r\n   " ;
		strErrorDescription += "Please contact VisionWeb Support \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="ORD002"){
		strErrorDescription += "Error while saving to Database\r\n   " ;
		strErrorDescription += " Please contact VisionWeb Support \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else if (strErrorCode=="ORD003"){
		strErrorDescription += "Order Cancelled\r\n   " ;
		strErrorDescription += "Detail Description given by Supplier/Lab while cancelling the order. \r\n";
		if (ISFormated) 
			strErrorDescription  += "\r\n\r\n********************************************************************************\r\n\r\n";
	}
	else
	{// Unknown error code
		strErrorDescription +="Error Code:  ";
		strErrorDescription +=strErrorCode;
		strErrorDescription +="\r\n";
		 
	}

return strErrorDescription;
}


// (s.dhole 2010-11-02 15:04) - PLID 40540 
long CInvVisionWebUtils::GetOrderTypeFromString(const CString &strType)
{
	if (strType=="SP")
		return vwotSpectacleLens ;
	else if (strType=="FR")
		return vwotFrame ;
	else if (strType=="CP")
		return vwotContactLensPatient ;
	else if (strType== "CO")
		return vwotContactLensOffice ;
	else if (strType=="CSF")
		return vwotSpectacleLensFrame;
	else
		ASSERT(FALSE); // we should not get this this is not valid date
		return -1;
}

// (s.dhole 2010-11-02 15:04) - PLID 41397 
// to check singl order status
BOOL CInvVisionWebUtils::CheckSubmittedOrderStatus(long nOrderId,OUT CString &strMessage,OUT COleDateTime &dt)
{	// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
	// (r.wilson 4/11/2012) PLID 43741 - Changed GlassesOrderStatus to GlassesOrderStatusID
	_RecordsetPtr rsOrder = CreateParamRecordset("SELECT    ID, GlassesOrderStatusID, GlassesOrderType, GlassesOrderNumber "
		" FROM  GlassesOrderT WHERE ID = {INT} ",nOrderId);
		if(!rsOrder->eof) 
			{
			long nUserID = (long)GetCurrentUserID();	
			long OrderType=AdoFldByte (rsOrder->Fields, "GlassesOrderType");
			long OrderStatus=AdoFldLong(rsOrder->Fields, "GlassesOrderStatusID"); // (r.wilson 4/11/2012) PLID 43471 - Changed GlassesOrderStatus to GlassesOrderStatusID
			CString strvwOrderId=AdoFldString (rsOrder->Fields, "GlassesOrderNumber","");

			CString strURL = GetRemotePropertyText(VISIONWEBSERVICE_ORDER_STATUS_URL, "", 0, "<None>", true);
			CString strUserID = GetRemotePropertyText(VISIONWEBSERVICE_USER_ID, "", 0, "<None>", true);
			CString strPSW = GetVisionWebPasword();
			CString strRefId = GetRemotePropertyText(VISIONWEBSERVICE_REFID, "", 0, "<None>", true);
			_ConnectionPtr pvwOrderConn= GetRemoteData();
		
			return UpdateOrderStatus(nOrderId, nUserID,OrderType,OrderStatus,strvwOrderId,
										   strURL,strUserID,strPSW ,strRefId,strMessage ,dt,pvwOrderConn);
			}
	return FALSE;
}

// (s.dhole 2010-11-15 13:12) - PLID 41397 
//check individual order status from visionWeb web site
BOOL CInvVisionWebUtils::UpdateOrderStatus(long nOrderId, long nUserID,long OrderType,long OrderStatus,LPCTSTR strvwOrderId,
										   LPCTSTR strURL,LPCTSTR strUserID,LPCTSTR strPSW ,LPCTSTR strRefId,
										   OUT CString &strMessage,OUT COleDateTime &dt,_ConnectionPtr pvwOrderConn)
{
	CString strSupplierTrackingId,strEstimatedDelivery, strvwTrackingCode, strSupplierTimeStamp , strType , strErrorCode; 
	CString strErrorDescription,strErrormessage;
	MSXML2::IXMLDOMNodePtr  pResult= CInvVisionWebUtils::GetvwOrderStatus( strvwOrderId,
				strURL, strUserID,strPSW, strRefId);
	MSXML2::IXMLDOMNodePtr xmlOrderResult= CInvVisionWebUtils::FindChildNode(pResult,"GetStatusResult");
	if (pResult!=NULL)
		{
			CParamSqlBatch batch;
			if  (CInvVisionWebUtils::CheckOrderStatus(xmlOrderResult,strSupplierTrackingId,strType,strEstimatedDelivery,  
					strvwTrackingCode ,strSupplierTimeStamp, strErrorCode, strErrorDescription, strErrormessage))
				{
				if (strErrorCode.IsEmpty() )
				{
					strMessage += "Order Status = ";
					strMessage += GetvwTrackingCodeDescription(strvwTrackingCode.Trim() );
					if (strEstimatedDelivery!=""){
						strMessage += "/n/r Estimated Delivery = ";
						strMessage += strEstimatedDelivery;
					}
					//only update record if IsActive<>0
					// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
					batch.Add ("UPDATE GlassesOrderHistoryT SET IsActive=0 WHERE IsActive<>0  AND GlassesOrderID={INT}", nOrderId);
					batch.Add("INSERT INTO  GlassesOrderHistoryT "
					"(GlassesOrderID, GlassesOrderType,GlassesOrderStatusID, MessageType, SupplierTrackingId,"
					" SupplierTimeStamp,GlassesTrackingCode, Note,IsActive, UpdateDate,UserID)"
					"VALUES ({INT},{INT},{INT},{STRING},{STRING}, {STRING} ,{STRING},{STRING}, 1,Getdate(),{INT})"
					, nOrderId,OrderType,OrderStatus, strType,strSupplierTrackingId,  
					strSupplierTimeStamp,strvwTrackingCode, strMessage,nUserID);
					batch.Add( "UPDATE GlassesOrderT SET  GlassesMessage={STRING} ,UpdateDate=Getdate() "
						" WHERE ID={INT}",strMessage,nOrderId);
					batch.Execute(pvwOrderConn);
					_RecordsetPtr rsOrder = CreateParamRecordset(pvwOrderConn,"SELECT    UpdateDate "
						" FROM  GlassesOrderT WHERE ID = {INT} ",nOrderId);
					if(!rsOrder->eof) 
						dt= AdoFldDateTime( rsOrder->Fields,"UpdateDate");
					
					return TRUE;
				}
				else
				{// error
					//only update record if IsActive<>0
					// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
					batch.Add ("UPDATE GlassesOrderHistoryT SET IsActive=0 WHERE IsActive<>0  AND GlassesOrderID={INT}", nOrderId);
					batch.Add("INSERT INTO  GlassesOrderHistoryT "
					"(GlassesOrderID, GlassesOrderType,GlassesOrderStatusID, MessageType,  Note,IsActive, UpdateDate,UserID)"
					"VALUES ({INT},{INT},{INT},{STRING},{STRING}, 1,Getdate(),{INT})"
					, nOrderId,OrderType,OrderStatus, "ERROR",GetvwErrorDescription(strErrorCode  ) ,nUserID);
					batch.Add( "UPDATE GlassesOrderT SET  GlassesMessage={STRING} ,UpdateDate=Getdate() "
						" WHERE ID={INT}",GetvwErrorDescription(strErrorCode  ) ,nOrderId);
					batch.Execute(pvwOrderConn);
					_RecordsetPtr rsOrder = CreateParamRecordset(pvwOrderConn,"SELECT    UpdateDate "
						" FROM  GlassesOrderT WHERE ID = {INT} ",nOrderId);
						if(!rsOrder->eof){
							dt= AdoFldDateTime( rsOrder->Fields,"UpdateDate");
						}
						strMessage=GetvwErrorDescription(strErrorCode  ); // (s.dhole 2010-11-02 15:04) - PLID 41546
					return FALSE;
				}
			}
		else
		{//error
			//only update record if IsActive<>0
			// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
			batch.Add ("UPDATE GlassesOrderHistoryT SET IsActive=0 WHERE IsActive<>0  AND GlassesOrderID={INT}", nOrderId);
			batch.Add("INSERT INTO  GlassesOrderHistoryT "
			"(GlassesOrderID, GlassesOrderType,GlassesOrderStatusID, MessageType,  Note,IsActive, UpdateDate,UserID)"
			"VALUES ({INT},{INT},{INT},{STRING},{STRING}, 1,Getdate(),{INT})"
			, nOrderId,OrderType,OrderStatus, "ERROR",strErrorDescription ,nUserID);
			batch.Add( "UPDATE GlassesOrderT SET  GlassesMessage={STRING} ,UpdateDate=Getdate() "
				" WHERE ID={INT}",strErrorDescription ,nOrderId);
			batch.Execute(pvwOrderConn);
			_RecordsetPtr rsOrder = CreateParamRecordset(pvwOrderConn,"SELECT    UpdateDate "
				" FROM  GlassesOrderT WHERE ID = {INT} ",nOrderId);
				if(!rsOrder->eof) 
				{
					dt= AdoFldDateTime( rsOrder->Fields,"UpdateDate");
				}
			strMessage=strErrorDescription;
			return FALSE;
			}
	}
	return FALSE;
}

// GetOrder status by Exchange ID
// (s.dhole 2010-11-02 15:04) - PLID 41397 
MSXML2::IXMLDOMNodePtr  CInvVisionWebUtils::GetvwOrderStatus(LPCTSTR strvwOrderId,
																   LPCTSTR strURL, LPCTSTR strUserID,LPCTSTR strPSW, LPCTSTR strRefId)
{
	CString stXML;
	stXML.Format(  "<ser:GetStatus>"
	"<ser:orderId>%s</ser:orderId>"
	"<ser:username>%s</ser:username>"
	"<ser:password>%s</ser:password>"
	"<ser:refid>%s</ser:refid>"
	"</ser:GetStatus>"
	,strvwOrderId
	, strUserID
	, strPSW
	, strRefId);
	return  CInvVisionWebUtils::CallSoapFunction(strURL,"http://services.visionweb.com/GetStatus",stXML,strUserID,strPSW,TRUE);
	
}
// (s.dhole 2010-11-02 15:04) - PLID 40540 // (s.dhole 2010-11-02 15:04) - PLID 40540 
// GetOrder status by order ID
MSXML2::IXMLDOMNodePtr   CInvVisionWebUtils::GetVisionOrderStatusByOrderID(LPCTSTR strExchangeID)
{
	CString strURL = GetRemotePropertyText(VISIONWEBSERVICE_ORDER_STATUS_URL, "", 0, "<None>", true);
	CString strUserID = GetRemotePropertyText(VISIONWEBSERVICE_USER_ID, "", 0, "<None>", true);
	CString strPSW = GetVisionWebPasword();
	CString strRefId = GetRemotePropertyText(VISIONWEBSERVICE_REFID, "", 0, "<None>", true);
	CString stXML;
			stXML.Format(  "<getStatusBySubmitterOrdIDLogin>\r\n"
			"<extOrdID>%s</extOrdID>\r\n"
			"<username>%s</username>\r\n"
			"<password>%s</password>\r\n"
			"<REFID>%s</REFID>\r\n"
			"</getStatusBySubmitterOrdIDLogin>\r\n"
			,strExchangeID
			, strUserID
			, strPSW
			, strRefId);
	return  CInvVisionWebUtils::CallSoapFunction(strURL,strURL,stXML,strUserID,strPSW);
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
// return Catalog XML string
MSXML2::IXMLDOMNodePtr   CInvVisionWebUtils::GetvwCatalog()
{
	CString strURL = GetRemotePropertyText(VISIONWEBSERVICE_CATALOG_URL , "", 0, "<None>", true);
	CString strUserID = GetRemotePropertyText(VISIONWEBSERVICE_USER_ID, "", 0, "<None>", true);
	CString strPSW = GetVisionWebPasword();
	CString strRefId = GetRemotePropertyText(VISIONWEBSERVICE_REFID, "", 0, "<None>", true);
	CString stXML;
	stXML.Format( "<getStandardCatalogByLogin>\r\n"
		 "<username>%s</username>\r\n"
		"<password>%s</password>\r\n"
		"<REFID>%s</REFID>\r\n"
		"</getStandardCatalogByLogin>\r\n" 
		, strUserID
		, strPSW
		, strRefId);
	return CInvVisionWebUtils::CallSoapFunction(strURL,strURL,stXML,strUserID,strPSW);
}

// (s.dhole 2010-11-02 15:04) - PLID 40728 Update Catalog/Sync
BOOL CInvVisionWebUtils::UpdateVisionWebCatalog()
{
	HR(pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60)));
	MSXML2::IXMLDOMNodePtr  pNodeResult  = GetvwCatalog();

	if (pNodeResult==NULL){
		AfxMessageBox("The VisionWeb Catalog update failed with the following error :\n"
		"[Invalid VisionWeb catalog response!]\n\n"
		"Please ensure your login information is correct, and that you have access to the internet.\n"
		"If this message persists, please contact NexTech Technical Support.");
		return FALSE;
	}
	MSXML2::IXMLDOMNodePtr pNodeData = CInvVisionWebUtils::FindChildNode(pNodeResult  , "getStandardCatalogByLoginReturn") ;
	if (pNodeData==NULL){
		AfxMessageBox("The VisionWeb Catalog update failed with the following error :\n"
		"[Invalid VisionWeb catalog response!]\n\n"
		"Please ensure your login information is correct, and that you have access to the internet.\n"
		"If this message persists, please contact NexTech Technical Support.");
		return FALSE;
	}
	_bstr_t strCatalogXML =_bstr_t(  CInvVisionWebUtils::XMLDecode( (LPCTSTR)pNodeData->Getxml() ));
	pDoc->loadXML(strCatalogXML) ; 
	if (pDoc!=NULL		)
	{ 
		CString strErrorCode,  strErrorDescription;
		if (CInvVisionWebUtils::IsResultHasError(pDoc,strErrorCode, strErrorDescription)) // Check for any error
		{	// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
			if (CInvVisionWebUtils::UpdateCatalogInfo(pDoc ,"DESIGNS","GlassesCatalogDesignsT","DesignCode","DesignName") &&
				CInvVisionWebUtils::UpdateCatalogInfo(pDoc ,"MATERIALS","GlassesCatalogMaterialsT","MaterialCode","MaterialName") &&
				CInvVisionWebUtils::UpdateCatalogInfo(pDoc ,"TREATMENTS","GlassesCatalogTreatmentsT","TreatmentCode","TreatmentName") &&
				CInvVisionWebUtils::UpdateCatalogInfo(pDoc ,"FRAME_TYPES","GlassesCatalogFrameTypesT","FrameTypeCode","FrameTypeName") ){
					return TRUE;
			}
			else{
				AfxMessageBox( "The VisionWeb Catalog update failed with the following error :\n"
				"[Could not update some of the catalog information. Please try again.]\n\n"
				"If this message persists, please contact NexTech Technical Support.");
				return FALSE;
			}

		}
		else{
			AfxMessageBox( FormatString(  "The VisionWeb Catalog update failed with the following error :\n"
				"[%s]\n\n"
				"Please ensure your login information is correct, and that you have access to the internet.\n"
				"If this message persists, please contact NexTech Technical Support.",strErrorDescription));
			return FALSE;
		}
	}
	else{
		AfxMessageBox("The VisionWeb Catalog update failed with the following error :\n"
		"[Invalid VisionWeb catalog response!]\n\n"
		"Please ensure your login information is correct, and that you have access to the internet.\n"
		"If this message persists, please contact NexTech Technical Support.");
		return FALSE;
	}
	return FALSE;
}

// (s.dhole 2010-11-02 15:04) - PLID 40728 Update Catalog/Sync
BOOL CInvVisionWebUtils::UpdateCatalogInfo(MSXML2::IXMLDOMDocument2Ptr pDoc,LPCTSTR strType,  LPCTSTR strTable, LPCTSTR strCodeField, LPCTSTR strDescriptionField)
{
	try{
		MSXML2::IXMLDOMNodePtr pNode;
		if (pDoc->GetchildNodes()->length>1) 
			pNode =CInvVisionWebUtils::FindChildNode(pDoc->GetchildNodes()->Getitem(1)     , strType)   ;
		else
			pNode =CInvVisionWebUtils::FindChildNode(pDoc->GetfirstChild()   , strType)   ;
		CString strCode;
		CString strDescription;
		// There is no machanisum to remove item allways append  or change exist list
		//PLID 42835 Maintain Glasses Catalog custom items Added GlassesOrderProcessTypeID=2 for VisionWeb

		if (pNode!=NULL){ 
			CString strSql;
			strSql.Format(  "if not exists(SELECT * FROM  %s WHERE %s ={STRING})"
							" BEGIN  \r\n"
							" INSERT INTO %s (  %s ,%s,GlassesOrderProcessTypeID ) VALUES ({STRING}, {STRING},2)  \r\n"
							" END  \r\n"
							" ELSE  \r\n"
							" BEGIN  \r\n"
							" UPDATE  %s SET  %s ={STRING} ,%s ={STRING}  WHERE %s ={STRING}  AND GlassesOrderProcessTypeID=2 \r\n"
							" END  \r\n",
							strTable,strCodeField, strTable ,strCodeField,strDescriptionField
							,strTable,strCodeField,strDescriptionField,strCodeField);
			MSXML2::IXMLDOMNodeListPtr pChildeNodes  = pNode->GetchildNodes();
			long nCount = pChildeNodes->Getlength()  ;
			for (long i=0; i<nCount; i++) {
				MSXML2::IXMLDOMNodePtr pChildeNode = pChildeNodes->Getitem(i);
				MSXML2::IXMLDOMNamedNodeMapPtr pAttributes=pChildeNode->Getattributes();
				long nCountAttributes = pAttributes->Getlength();
				strDescription=strCode="";
				for (long nAtttribut=0; nAtttribut<nCountAttributes; nAtttribut++) {
					if ((CString((LPCTSTR) pAttributes->Getitem(nAtttribut)->GetbaseName()).CompareNoCase("VwCode")==0) ||
						(CString((LPCTSTR) pAttributes->Getitem(nAtttribut)->GetbaseName()).CompareNoCase("ID")==0)){
						strCode=(CString)(LPCTSTR)pAttributes->Getitem(nAtttribut)->Gettext()  ;
						strCode=CInvVisionWebUtils::XMLDecode(strCode);
					}
					else if (CString((LPCTSTR) pAttributes->Getitem(nAtttribut)->GetbaseName()).CompareNoCase("Description")==0){
						strDescription=(CString)(LPCTSTR)pAttributes->Getitem(nAtttribut)->Gettext();
						strDescription=CInvVisionWebUtils::XMLDecode(strDescription);
					}
				}
				if (!strCode.IsEmpty() && !strDescription.IsEmpty())
					ExecuteParamSql((LPCTSTR)strSql, strCode,  strCode, strDescription,strCode, strDescription,  strCode);
				strDescription=strCode="";
			}
			return TRUE;
		}
		return FALSE;
	}NxCatchAll("Error in CInvVisionWebUtils::UpdateCatalogInfo");
	return FALSE;
}

// (s.dhole 2010-11-15 13:12) - PLID 40540 checking any error tag from visionWeb response 
BOOL CInvVisionWebUtils::IsResultHasError(MSXML2::IXMLDOMNode *lpParent,OUT CString &strErrorCode, OUT CString &strErrorDescription)
{
	BOOL bResult=FALSE;
	MSXML2::IXMLDOMNodePtr pParent(lpParent);
	if (pParent != NULL) {
		bResult=TRUE;
		MSXML2::IXMLDOMNodePtr pErrorNode =CInvVisionWebUtils::FindChildNode(pDoc->GetfirstChild() , "ERROR")   ;
		if (pErrorNode != NULL) {
			bResult=FALSE;
			strErrorDescription = (CString )(LPCTSTR)pErrorNode->Gettext(); 
			MSXML2::IXMLDOMNamedNodeMapPtr pAttributes=pErrorNode ->Getattributes();
			long nCount = pAttributes->Getlength();
			for (long i=0; i<nCount; i++) {
				if (CString((LPCTSTR) pAttributes->Getitem(i)->GetbaseName()).CompareNoCase("code")==0)
						strErrorCode=(CString)(LPCTSTR)pAttributes->Getitem(i)->Gettext();
				else if (CString((LPCTSTR) pAttributes->Getitem(i)->GetbaseName()).CompareNoCase("Desc")==0)
					strErrorDescription +=(CString)(LPCTSTR)pAttributes->Getitem(i)->Gettext();
			}

		}
	}
	return bResult;
}

// (s.dhole 2010-11-15 13:12) - PLID 41397 
//check individual order status from visionWeb web site
BOOL CInvVisionWebUtils::CheckOrderStatus(MSXML2::IXMLDOMNode *lpParent,OUT CString &strSupplierTrackingId, OUT CString &strType,OUT CString &strEstimatedDelivery,  
				OUT CString &strvwTrackingCode ,OUT CString &strSupplierTimeStamp, OUT CString &strErrorCode, OUT CString &strErrorDescription, OUT CString &strErrormessage)
{
	BOOL orderStatus=FALSE;
	MSXML2::IXMLDOMNodePtr pParent(lpParent);
	if (pParent != NULL) {
		HR(pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60)));
		_bstr_t strXML= CInvVisionWebUtils::XMLDecode( (LPCTSTR)pParent->Getxml());
		pDoc->loadXML ( strXML) ; 
		if (pDoc!=NULL)
		{
			MSXML2::IXMLDOMNodePtr pNode =CInvVisionWebUtils::FindChildNode(pDoc, "HEADER")   ;
			if (pNode!= NULL)
			{
				// we need to store this id in database for track down order status
				MSXML2::IXMLDOMAttributePtr	pAttribute = CInvVisionWebUtils::FindChildAttribut(pNode->Getattributes(),"MessageType" );
				if (pAttribute!=NULL)
				{
					strType=  (CString)pAttribute->value; 
					if (strType.CompareNoCase("TRACKING")==0 ){
						orderStatus=TRUE;
						MSXML2::IXMLDOMNodePtr pOrderStatusItemNode =CInvVisionWebUtils::FindChildNode(pDoc->GetfirstChild() , "ITEM");
						if (pOrderStatusItemNode!= NULL)
						{
						MSXML2::IXMLDOMNamedNodeMapPtr pAttributes=pOrderStatusItemNode ->Getattributes();
						long nCount = pAttributes->Getlength();
						for (long i=0; i<nCount; i++) {
							if (CString((LPCTSTR) pAttributes->Getitem(i)->GetbaseName()).CompareNoCase("SupplierTrackingId")==0)
								strSupplierTrackingId=(CString)(LPCTSTR)pAttributes->Getitem(i)->Gettext()  ;
							
							else if (CString((LPCTSTR) pAttributes->Getitem(i)->GetbaseName()).CompareNoCase("EstimatedDelivery")==0)
								strEstimatedDelivery=(CString)(LPCTSTR)pAttributes->Getitem(i)->Gettext();
							}
						}
						MSXML2::IXMLDOMNodePtr pOrderStatusNode =CInvVisionWebUtils::FindChildNode(pDoc->GetfirstChild() , "STATUS");
						if (pOrderStatusNode!= NULL)
						{
						MSXML2::IXMLDOMNamedNodeMapPtr pAttributes=pOrderStatusNode ->Getattributes();
						long nCount = pAttributes->Getlength();
						for (long i=0; i<nCount; i++) {
							if (CString((LPCTSTR) pAttributes->Getitem(i)->GetbaseName()).CompareNoCase("Code")==0)
								strvwTrackingCode=(CString)(LPCTSTR)pAttributes->Getitem(i)->Gettext()  ;
							
							else if (CString((LPCTSTR) pAttributes->Getitem(i)->GetbaseName()).CompareNoCase("SupplierTimeStamp")==0)
								strSupplierTimeStamp=(CString)(LPCTSTR)pAttributes->Getitem(i)->Gettext();
							}
						}
					}
					else if (strType.CompareNoCase("ERROR")==0)
					{
						orderStatus=FALSE;
						MSXML2::IXMLDOMNodePtr pErrorNode =CInvVisionWebUtils::FindChildNode(pDoc->GetfirstChild() , "ERROR")   ;
						if (pNode!= NULL)
						{
							strErrormessage = (CString )(LPCTSTR)pErrorNode->Gettext(); 
							MSXML2::IXMLDOMNamedNodeMapPtr pAttributes=pErrorNode ->Getattributes();
							long nCount = pAttributes->Getlength();
							for (long i=0; i<nCount; i++) {
								if (CString((LPCTSTR) pAttributes->Getitem(i)->GetbaseName()).CompareNoCase("code")==0)
										strErrorCode=(CString)(LPCTSTR)pAttributes->Getitem(i)->Gettext();
								else if (CString((LPCTSTR) pAttributes->Getitem(i)->GetbaseName()).CompareNoCase("Desc")==0)
									strErrorDescription +=(CString)(LPCTSTR)pAttributes->Getitem(i)->Gettext();
							}
							// (c.haag 2015-11-18) - PLID 67575 - Helpful logging
							if (strErrorDescription.IsEmpty())
							{
								strErrorDescription = strErrormessage;
							}
						}
						else // (c.haag 2015-11-18) - PLID 67575 - If return response is not expected text
						{
							strErrorCode = "0";
							strErrorDescription = "Invalid VisionWeb order status error response";
							strErrormessage = (LPCTSTR)strXML;
						}
					}
					else // If return response is not expected text
					{
						strErrorCode="0";
						strErrorDescription="Invalid VisionWeb order status response";
						strErrormessage= (LPCTSTR)strXML;
					}
				}
				else // If return response is not expected text
					{
						strErrorCode="0";
						strErrorDescription="Invalid VisionWeb order status response";
						strErrormessage= (LPCTSTR)strXML;
					}
			}
			// (c.haag 2015-11-18) - PLID 67575 - We may get an error response with no header
			else if (nullptr != (pNode = CInvVisionWebUtils::FindChildNode(pDoc, "ERROR_MESSAGE")))
			{
				orderStatus = FALSE;
				MSXML2::IXMLDOMNodePtr pErrorNode = CInvVisionWebUtils::FindChildNode(pDoc->GetfirstChild(), "ERROR");
				if (pNode != NULL)
				{
					strErrormessage = (CString)(LPCTSTR)pErrorNode->Gettext();
					MSXML2::IXMLDOMNamedNodeMapPtr pAttributes = pErrorNode->Getattributes();
					long nCount = pAttributes->Getlength();
					for (long i = 0; i<nCount; i++) {
						if (CString((LPCTSTR)pAttributes->Getitem(i)->GetbaseName()).CompareNoCase("code") == 0)
							strErrorCode = (CString)(LPCTSTR)pAttributes->Getitem(i)->Gettext();
						else if (CString((LPCTSTR)pAttributes->Getitem(i)->GetbaseName()).CompareNoCase("Desc") == 0)
							strErrorDescription += (CString)(LPCTSTR)pAttributes->Getitem(i)->Gettext();
					}
					if (strErrorDescription.IsEmpty())
					{
						strErrorDescription = strErrormessage;
					}
				}
				else // If return response is not expected text
				{
					strErrorCode = "0";
					strErrorDescription = "Invalid VisionWeb order error status response";
					strErrormessage = (LPCTSTR)strXML;
				}
			}
			else // If return response is not expected text
			{
				strErrorCode="0";
				strErrorDescription="Invalid VisionWeb order status response";
				strErrormessage= (LPCTSTR)strXML;
			}
		}
		else{
			strErrorCode="-1";
			strErrorDescription="Invalid VisionWeb order status response";
			strErrormessage= (LPCTSTR)strXML;
		}
	}
	else{
		strErrorCode="-1";
		
		
		strErrorDescription="Invalid VisionWeb order status response";
	}

	// (c.haag 2015-11-18) - PLID 67575 - Helpful logging
	if (!strErrormessage.IsEmpty())
	{
		Log("CInvVisionWebUtils::CheckOrderStatus returned strErrormessage %s", strErrormessage);
	}

	return orderStatus;
}

// (s.dhole 2010-11-15 13:12) - PLID 40540
// this will parse response from order submital and return order status and order detail on sucsess or error description on errors
// pass order response to this function
BOOL CInvVisionWebUtils::CheckOrderResoponse(MSXML2::IXMLDOMNode *lpParent,OUT CString &strExchangeId,OUT CString &strCreationDate,  OUT CString &strNumber  ,OUT CString &strReference ,
									 OUT CString &strType, OUT CString &strErrorCode, OUT CString &strErrorDescription, OUT CString &strErrormessage)
{
	BOOL orderStatus=FALSE;
	MSXML2::IXMLDOMNodePtr pParent(lpParent);
	if (pParent != NULL) {
		HR(pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60)));
		_bstr_t strXML= CInvVisionWebUtils::XMLDecode( (LPCTSTR)pParent->Getxml());
		pDoc->loadXML ( strXML) ; 
		if (pDoc!=NULL)
		{
			MSXML2::IXMLDOMNodePtr pNode =CInvVisionWebUtils::FindChildNode(pDoc, "HEADER")   ;
			if (pNode!= NULL)
			{
				// we need to store this id in database for track down order status
				MSXML2::IXMLDOMAttributePtr  pAttribute = CInvVisionWebUtils::FindChildAttribut(pNode->Getattributes(),"ExchangeId");
				if (pAttribute!=NULL){
					strExchangeId=(CString)pAttribute->value; 
				}
				pAttribute = CInvVisionWebUtils::FindChildAttribut(pNode->Getattributes(),"MessageType" );
				if (pAttribute!=NULL)
				{
					CString strValue=  (CString)pAttribute->value; 
					strType=strValue;
					if (strValue.CompareNoCase("ORDER")==0 || strValue.CompareNoCase("WARN")==0|| strValue.CompareNoCase("CONFIRM")==0){
						orderStatus=TRUE;
						MSXML2::IXMLDOMNodePtr pOrderNode =CInvVisionWebUtils::FindChildNode(pDoc->GetfirstChild() , strType)   ;
						// this node always return data, but as standerd practice
						if (pNode!= NULL){
							MSXML2::IXMLDOMNamedNodeMapPtr pAttributes=pOrderNode->Getattributes();
							long nCount = pAttributes->Getlength();
							for (long i=0; i<nCount; i++) {
								if (CString((LPCTSTR) pAttributes->Getitem(i)->GetbaseName()).CompareNoCase("creationdate")==0)
									strCreationDate=(CString)(LPCTSTR)pAttributes->Getitem(i)->Gettext()  ;
								else if (CString((LPCTSTR) pAttributes->Getitem(i)->GetbaseName()).CompareNoCase("number")==0)
									strNumber=(CString)(LPCTSTR)pAttributes->Getitem(i)->Gettext();
								else if (CString((LPCTSTR) pAttributes->Getitem(i)->GetbaseName()).CompareNoCase("reference")==0)
									strReference=(CString)(LPCTSTR)pAttributes->Getitem(i)->Gettext();
								}
						}
					}

					else if (strValue.CompareNoCase("ERROR")==0){
						// (s.dhole 2010-11-02 15:04) - PLID 41546 Check error detail 
						orderStatus=TRUE; // Responce hase error message, This value will be true since we are getting responce from VisionWeb
						MSXML2::IXMLDOMNodePtr pErrorNode =CInvVisionWebUtils::FindChildNode(pDoc->GetfirstChild() , "ERROR")   ;
						// this node always return data, but as staderd i am checking for NULL
						if (pNode!= NULL){
							strErrormessage = (CString )(LPCTSTR)pErrorNode->Gettext(); 
							MSXML2::IXMLDOMNamedNodeMapPtr pAttributes=pErrorNode ->Getattributes();
							long nCount = pAttributes->Getlength();
							for (long i=0; i<nCount; i++) {
								if (CString((LPCTSTR) pAttributes->Getitem(i)->GetbaseName()).CompareNoCase("code")==0)
									strErrorCode=(CString)(LPCTSTR)pAttributes->Getitem(i)->Gettext();
								else if (CString((LPCTSTR) pAttributes->Getitem(i)->GetbaseName()).CompareNoCase("Desc")==0)
									strErrorDescription +=(CString)(LPCTSTR)pAttributes->Getitem(i)->Gettext();
							}
						}
						return orderStatus;
					}
					else {// If return response is not expected text
						strErrorCode="0";
						strErrorDescription="Invalid VisionWeb order response";
						strErrormessage= (LPCTSTR)strXML;
					}
				}
				else {// If return response is not expected text
						strErrorCode="0";
						strErrorDescription="Invalid VisionWeb order response";
						strErrormessage= (LPCTSTR)strXML;
					}
			}
			else {
				// If return response is not expected text
				strErrorCode="0";
				strErrorDescription="Invalid VisionWeb order response";
				strErrormessage= (LPCTSTR)strXML;
			}
		}
		else{
			strErrorCode="-1";
			strErrorDescription="Invalid VisionWeb order response";
			strErrormessage= (LPCTSTR)strXML;
		}
	}
	else{
		strErrorCode="-1";
		strErrorDescription="Invalid VisionWeb order response";
	}
	return orderStatus;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//return Attribut pointer from node 
// To check  element attribute value
MSXML2::IXMLDOMAttributePtr CInvVisionWebUtils::FindChildAttribut(MSXML2::IXMLDOMNamedNodeMapPtr lpParent, LPCTSTR strChildName)	{
	MSXML2::IXMLDOMNamedNodeMapPtr pAttributes(lpParent);
	if (pAttributes != NULL) {
		long nCount= pAttributes->Getlength() ;
		for (long i=0; i<nCount; i++) {
			MSXML2::IXMLDOMAttributePtr   pAttribute = pAttributes->Getitem(i) ;
			if ((CString((LPCTSTR)pAttribute->GetbaseName() )).CompareNoCase(strChildName) == 0) {
				return pAttribute;
			}
		}
	}
	return NULL;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//return first  childe node for given value
// this is extended function to get child value, which we have in SOAPUtils class
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::FindChildNode(MSXML2::IXMLDOMNode *lpParent, LPCTSTR strChildName)
{
	MSXML2::IXMLDOMNodePtr pParent(lpParent);
	if (pParent != NULL) {
		// Get the "elements" collection
		MSXML2::IXMLDOMNodeListPtr pElements = pParent->GetchildNodes();
		if (pElements) {
			// Got the "elements" collection, now loop through it
			long nCount = pElements->Getlength();
			for (long i=0; i<nCount; i++) {
				// Get this item
 			MSXML2::IXMLDOMNodePtr pNode = pElements->Getitem(i);
				// Compare this item's base name to the given child name
				if (CString((LPCTSTR)pNode->GetbaseName()).CompareNoCase(strChildName) == 0) {
					// Got a match, return success
					return pNode;
				}
				else {
					// Call recurring function to get value
					MSXML2::IXMLDOMNodePtr pChildeNode=CInvVisionWebUtils::FindChildNode(pNode,strChildName);
						if ((pChildeNode != NULL) && (CString((LPCTSTR)pChildeNode->GetbaseName()).CompareNoCase(strChildName) == 0))
							  return pChildeNode;
					}
			}
			// If we loop all the way through without finding it, then we failed
			return NULL;
		} else {
			// Couldn't get the elements collection for some reason
			return NULL;
		}
	} else {
		// Bad parent
		return NULL;
	}
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//This will validate Order Schema
BOOL CInvVisionWebUtils::ValidatevwOrderXML(const CString& strXMLOrder) {
	//create our array
	CString  strXML= strXMLOrder;
	strXML.Replace("<ORDER_MSG>","<X:ORDER_MSG xmlns:X=\"http://www.visionweb.com/vwservices/services/SPOrderService\">"  );
	strXML.Replace("</ORDER_MSG>","</X:ORDER_MSG>" ); 
	CPtrArray pSchemaArray;
	SchemaType *pSchema = new SchemaType;
	pSchema->nResourceID = IDR_VISIONWEB_ORDER_XSD;
	pSchema->strNameSpace = "http://www.visionweb.com/vwservices/services/SPOrderService";
	pSchema->strResourceFolder = "XSD";
	pSchemaArray.Add(pSchema);
	//throws its own errors
	BOOL bReturn = FALSE;
	try {
		bReturn = ValidateXMLWithSchema(strXML, &pSchemaArray);
	} catch (...) {
		//delete the array
		for(int i = 0; i < pSchemaArray.GetSize(); i++) {
			SchemaType* pSchema = (SchemaType*)pSchemaArray.GetAt(i);
			if(pSchema)
				delete pSchema;
		}
		pSchemaArray.RemoveAll();
		throw;
	};
	//now delete the array
	for(int i = 0; i < pSchemaArray.GetSize(); i++) {
		SchemaType* pSchema = (SchemaType*)pSchemaArray.GetAt(i);
		if(pSchema)
			delete pSchema;
	}
	pSchemaArray.RemoveAll();
	return bReturn;
}
// (s.dhole 2010-11-02 15:04) - PLID 40540 
//  may be we movre this to SopUdtils
CString CInvVisionWebUtils::XMLDecode(LPCTSTR str)
{
	CString strAns = str;
	strAns.Replace( "&amp;","&");
	strAns.Replace("&lt;","<");
	strAns.Replace( "&gt;",">");
	strAns.Replace("&apos;","'");
	strAns.Replace("&quot;","\"");
	return strAns;
}

// Return Catalog XML string
// (s.dhole 2010-11-02 15:04) - PLID 41397 
//this VisionWeb specific Soap body
CString CInvVisionWebUtils::BuildSoapCallXmlXml(const CString &strParamXml)
{
	// Configure the XML to be the body of the message.  This text should remain consistent for any soap message
	CString strXml;
	strXml = 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" 
		"<soap:Envelope " 
		"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" " 
		"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" " 
		"xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n";
		strXml += "  <soap:Body>\r\n " + 
				strParamXml + 
				"  </soap:Body>\r\n" 
				"</soap:Envelope>";
	return strXml;
}

// Return Catalog XML string
// (s.dhole 2010-11-02 15:04) - PLID 40540 
//this VisionWeb specific Soap body , use to hit non .asmx services
CString CInvVisionWebUtils::BuildSoapServiceCallXmlXml(const CString &strParamXml)
{
	// Configure the XML to be the body of the message.  This text should remain consistent for any soap message
	CString strXml;
	strXml = 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" 
		"<soapenv:Envelope " 
		"xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" " 
		"xmlns:ser=\"http://services.visionweb.com\">\r\n";
	strXml += "  <soapenv:Body>\r\n " + 
				strParamXml + 
				"  </soapenv:Body>\r\n" 
			"</soapenv:Envelope>";
	return strXml;
}


// (s.dhole 2010-11-02 15:04) - PLID 40540 
//this VisionWeb specific Soap Call to submit xml to visionweb
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CallSoapFunction(const CString &strURL, const CString &strURI, const CString &strParamXml, const CString& strUsername, const CString& strPassword,const BOOL IsWebService )
{
	MSXML2::IXMLHTTPRequestPtr req("Msxml2.ServerXMLHTTP.3.0");
	if (!strUsername.IsEmpty() || !strPassword.IsEmpty()) 
		req->open("POST", AsBstr(strURL), false, _bstr_t(strUsername), _bstr_t(strPassword));
	 else 
		req->open("POST", AsBstr(strURL), false);
	req->setRequestHeader("Content-Type", "text/xml"); 
	req->setRequestHeader("SOAPAction",(_bstr_t) strURI);

	CString strSOAP ;
	if (IsWebService) 
		strSOAP = BuildSoapServiceCallXmlXml(strParamXml);
	else
		strSOAP = BuildSoapCallXmlXml(strParamXml);

	_bstr_t bstrXml = AsBstr(strSOAP );
	//Actually submit order request to the server
	try{
		req->send(bstrXml);

		long nStatusCode = req->Getstatus();
		CString strHttpStatus;
		if (nStatusCode != 200) {
			// not 200 OK, so log it.
			_bstr_t bstrStatusText = req->GetstatusText();
			strHttpStatus.Format("SOAP Function call returned HTTP status %li (%s)\r\n", nStatusCode, (LPCTSTR)bstrStatusText);
			::OutputDebugString(strHttpStatus);
			// (c.haag 2015-11-18) - PLID 67575 - Log any non-200 status code for support purposes
			Log("%s", strHttpStatus);
		}
	}
	//(s.dhole 12/3/2014 10:23 AM ) - PLID 61750 Added error handling to show more information if connection time out or fail to connect website
	//we do have standard warning to check internet or credential, if this function failto return result
	catch (_com_error& e) {
		if (e.Error() == 0x80072EE2) {	//internet timeout
			AfxMessageBox("The VisionWeb order timed out waiting for a response.\n\n"
				"If this message persists, please contact NexTech Technical Support.");
			
		}
		else {
			// We still want to throw warning for unknown issue
			
			AfxMessageBox("Error submitting Glasses order request to VisionWeb.\n\n"
				"If this message persists, please contact NexTech Technical Support.");
		}
		Log("Timeout Submitting order to VisionWeb; Error Code: 0x%x  Error Description: %s   ", e.Error(), (LPCTSTR)e.Description());
	}
	MSXML2::IXMLDOMDocumentPtr pResponseDoc;
	{
		IDispatch *pBody = NULL;
		req->get_responseXML(&pBody);
		pResponseDoc = pBody;
		pBody->Release();
	}
return pResponseDoc;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//SOAP FUNCTIONALITY
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNode(LPCTSTR szName)
	{
		return pDoc->createNode(NODE_ELEMENT, szName, "");
	}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//SOAP FUNCTIONALITY
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateElementAttribute(LPCTSTR strType,LPCTSTR strValue )
{
	MSXML::IXMLDOMNodePtr attribute = pDoc->createNode(NODE_ATTRIBUTE, strType, "");
	attribute->put_nodeValue(_variant_t(strValue));
	return attribute ;
}


// (s.dhole 2010-11-02 15:04) - PLID 40540 
// This is header for order xml and it is required
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLHeader(LPCTSTR strLoginID,LPCTSTR strOrderType,LPCTSTR strPassword,LPCTSTR strSubmittedDate 
		, LPCTSTR strSubmitterGuid, LPCTSTR strSubmitterId, LPCTSTR strSubmitterOrderId, LPCTSTR strSupplierId, LPCTSTR strVersionNum){
		MSXML2::IXMLDOMNodePtr NewNode =CreateNode("HEADER");
		NewNode->attributes->setNamedItem(CreateElementAttribute("SubmitterId",XMLEncode(strSubmitterId)));
		NewNode->attributes->setNamedItem(CreateElementAttribute("OrderType",XMLEncode(strOrderType)));
		NewNode->attributes->setNamedItem(CreateElementAttribute("Login",XMLEncode(strLoginID)));
		NewNode->attributes->setNamedItem(CreateElementAttribute("Password",XMLEncode(strPassword)));
		NewNode->attributes->setNamedItem(CreateElementAttribute("SubmitterOrderId",XMLEncode(strSubmitterOrderId)));
		NewNode->attributes->setNamedItem(CreateElementAttribute("VersionNum",XMLEncode(strVersionNum)));
		NewNode->attributes->setNamedItem(CreateElementAttribute("SupplierId",XMLEncode(strSupplierId)));
		NewNode->attributes->setNamedItem(CreateElementAttribute("SubmitterGuid",XMLEncode(strSubmitterGuid)));
		NewNode->attributes->setNamedItem(CreateElementAttribute("SubmittedDate",XMLEncode(strSubmittedDate)));
		return NewNode;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
// This is Order Node and need to be call after Header,  required
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLSP_ORDER(const CString&  strCreationDate,const CString& strPoNum,const CString& strType,const CString& strStatus)	{
		MSXML2::IXMLDOMNodePtr NewNode =CreateNode("SP_ORDER");
		NewNode->attributes->setNamedItem(CreateElementAttribute("CreationDate",XMLEncode(strCreationDate)));
		NewNode->attributes->setNamedItem(CreateElementAttribute("PoNum",XMLEncode(strPoNum)));
		if (!strType.IsEmpty())
			NewNode->attributes->setNamedItem(CreateElementAttribute("Type",XMLEncode(strType)));
		if (! strStatus.IsEmpty())
			NewNode->attributes->setNamedItem(CreateElementAttribute("Status",XMLEncode(strStatus)));
		return NewNode; 
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
// This is Order Node and need to be call after Header,  Optional
//pSP_ORDER=>REDO_Information
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLSP_OrderRedo_Information(LPCTSTR strOriginalOrderNumber,
														LPCTSTR  strSupplierInvoiceNumber,LPCTSTR  strLmsCode,  
														LPCTSTR strVwCode,LPCTSTR  strEye)	{
		MSXML2::IXMLDOMNodePtr NewNode =CreateNode("REDO_INFORMATION");
		NewNode->attributes->setNamedItem(CreateElementAttribute("OriginalOrderNumber",XMLEncode(strOriginalOrderNumber)));
		NewNode->attributes->setNamedItem(CreateElementAttribute("SupplierInvoiceNumber",XMLEncode(strSupplierInvoiceNumber)));
		NewNode->attributes->setNamedItem(CreateElementAttribute("Eye",XMLEncode(strEye)));
		NewNode->attributes->setNamedItem(CreateElementAttribute("LmsCode",XMLEncode(strLmsCode)));
		NewNode->attributes->setNamedItem(CreateElementAttribute("VwCode",XMLEncode(strVwCode)));
		return NewNode; 
	}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
// This is Order Node and need to be call after Header,  Required
//pSP_ORDER=>ACCOUNT
MSXML2::IXMLDOMNodePtr  CInvVisionWebUtils::CreateNodevwOrderXMLAccount(const CString&  strCbsId,const CString&  strDropShipAccountId)	{
		MSXML2::IXMLDOMNodePtr NewNode =CreateNode("ACCOUNT");
		NewNode->attributes->setNamedItem(CreateElementAttribute("CbsId",XMLEncode(strCbsId)));
		if (! strDropShipAccountId.IsEmpty())
			NewNode->attributes->setNamedItem(CreateElementAttribute("DropShipAccountId",XMLEncode(strDropShipAccountId)));
		return NewNode;
	}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//Reqired
//SP_ORDER=>RX_DETAIL->PATIENT
void  CInvVisionWebUtils::AppendNodevwOrderXMLpatient( MSXML2::IXMLDOMNodePtr pPatient, const CString&  strLastName,
																		  const CString&  strFirstName)	{
		
		pPatient->attributes->setNamedItem(CreateElementAttribute("LastName",XMLEncode(strLastName)));
		if (! strFirstName.IsEmpty())
		pPatient->attributes->setNamedItem(CreateElementAttribute("FirstName",XMLEncode(strFirstName)));

	}


//// (s.dhole 2010-11-02 15:04) - PLID 
//we are waiting for this item to be implmentd by VisionWeb 
//SP_ORDER=>RX_DETAIL->PATIENT->PERSONALIZED_DATA-Patient_initials
//MSXML2::IXMLDOMNodePtr  CInvVisionWebUtils::CreateNodevwOrderXMLPersonalizedData(const CString&  strPatientInitials)	{
//		MSXML2::IXMLDOMNodePtr NewNode =CreateNode("PERSONALIZED_DATA");
//		
//	//	NewNode->attributes->setNamedItem(CreateElementAttribute("Patient_initials",XMLEncode(strPatientInitials)));
//		
//		return NewNode;
//	}

//// (s.dhole 2010-11-02 15:04) - PLID 
//we are waiting for this item to be implmentd by VisionWeb 
//SP_ORDER=>RX_DETAIL->PATIENT->PERSONALIZED_DATA
//MSXML2::IXMLDOMNodePtr  CInvVisionWebUtils::CreateNodevwOrderXMLSpecialData(const CString&  strName, 
//																			const CString&  strValue)	{
//		MSXML2::IXMLDOMNodePtr NewNode =CreateNode("SPECIAL_PARAMETER");
//		
//		NewNode->attributes->setNamedItem(CreateElementAttribute("Name",XMLEncode(strName)));
//		NewNode->attributes->setNamedItem(CreateElementAttribute("Value",XMLEncode(strValue)));
//		return NewNode;
//	}


// (s.dhole 2010-11-02 15:04) - PLID 40540 
//SP_ORDER=>RX_DETAIL->JOB
MSXML2::IXMLDOMNodePtr  CInvVisionWebUtils::CreateNodevwOrderXMLJob(  const CString&  strType,  const CString&  strInstruction)	{
		MSXML2::IXMLDOMNodePtr NewNode =CreateNode("JOB");
		NewNode->attributes->setNamedItem(CreateElementAttribute("Type",XMLEncode(strType)));
		if (! strInstruction.IsEmpty())
			NewNode->attributes->setNamedItem(CreateElementAttribute("SpecialInstructions",XMLEncode(strInstruction)));
		return NewNode;
	}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
// , Optional
//SP_ORDER=>RX_DETAIL=>FRAME
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLFrame(const CString&  strNumber,  const CString&  strBrand,
																	   const CString&  strModel,  const CString& strColor,
												   const CString&  strEyeSize,  const CString& strSKU,
												   const CString& strLmsCode,  const CString&  strVwCode,
												   const CString& strTempletType,  const CString&  strTypeName, 
												   const CString& strLength)	{
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("FRAME");
	NewNode->attributes->setNamedItem(CreateElementAttribute("Number",XMLEncode(strNumber)));
	if (! strBrand.IsEmpty())
		NewNode->attributes->setNamedItem(CreateElementAttribute("Brand",XMLEncode(strBrand.Left(20)  )));
	if (! strModel.IsEmpty())
		NewNode->attributes->setNamedItem(CreateElementAttribute("Model",XMLEncode(strModel.Left(50))));
	if (! strColor.IsEmpty())
		NewNode->attributes->setNamedItem(CreateElementAttribute("Color",XMLEncode(strColor.Left(50))));
	if (! strEyeSize.IsEmpty())
		NewNode->attributes->setNamedItem(CreateElementAttribute("EyeSize",XMLEncode(strEyeSize)));
	if (! strSKU.IsEmpty())
		NewNode->attributes->setNamedItem(CreateElementAttribute("SKU",XMLEncode(strSKU.Left(15))));
	if (! strTypeName.IsEmpty())
		NewNode->attributes->setNamedItem(CreateElementAttribute("TempleTypeName",XMLEncode(strTypeName)));
	// (j.dinatale 2013-04-19 09:19) - PLID 56308 - we were checking the wrong variable here (we were checking strVwCode) 
	if (! strTempletType.IsEmpty())
		NewNode->attributes->setNamedItem(CreateElementAttribute("TempleType",XMLEncode(strTempletType)));
	if (! strLength.IsEmpty())
		NewNode->attributes->setNamedItem(CreateElementAttribute("TempleLength",XMLEncode(strLength)));
	if (! strLmsCode.IsEmpty())
		NewNode->attributes->setNamedItem(CreateElementAttribute("LmsCode",XMLEncode(strLmsCode)));
	if (! strVwCode.IsEmpty())
		NewNode->attributes->setNamedItem(CreateElementAttribute("VwCode",XMLEncode(strVwCode)));
	return NewNode;

}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
// , Optional
//SP_ORDER=>RX_DETAIL=>FRAME=>SUPPLIER
MSXML2::IXMLDOMNodePtr  CInvVisionWebUtils::CreateNodevwOrderXMLFrameSupplier(LPCTSTR  strName,LPCTSTR strCode)	{
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("FRAME_SUPPLIER");
	NewNode->attributes->setNamedItem(CreateElementAttribute("Name",XMLEncode(strName)));
	NewNode->attributes->setNamedItem(CreateElementAttribute("Code",XMLEncode(strCode)));
	return NewNode;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
// , Optional
//SP_ORDER=>RX_DETAIL->FRAME=>PACKAGE
MSXML2::IXMLDOMNodePtr  CInvVisionWebUtils::CreateNodevwOrderXMLFramePackage(  const CString&  strName,  
																			   const CString&  strCode,  
																			   const CString&  strSafety)	{
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("PACKAGE");
	NewNode->attributes->setNamedItem(CreateElementAttribute("Name",XMLEncode(strName)));
	NewNode->attributes->setNamedItem(CreateElementAttribute("Code",XMLEncode(strCode)));
	if (! strSafety.IsEmpty())
		NewNode->attributes->setNamedItem(CreateElementAttribute("Safety",XMLEncode(strSafety)));
	return NewNode;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//Required
//SP_ORDER=>RX_DETAIL=>POSITION
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLPosition(  const CString& strEye,  const CString& strFarHalfPd,
													     const CString& strSegHeight, const CString& strNearHalfPd,  const CString& strOpticalCenter){
	MSXML2::IXMLDOMNodePtr NewNode = CreateNode("POSITION");
	NewNode ->attributes->setNamedItem(CreateElementAttribute("Eye",strEye));
	NewNode ->attributes->setNamedItem(CreateElementAttribute("FarHalfPd",XMLEncode(strFarHalfPd)));
	if (! strNearHalfPd.IsEmpty())
		NewNode ->attributes->setNamedItem(CreateElementAttribute("NearHalfPd",XMLEncode(strNearHalfPd)));
	NewNode ->attributes->setNamedItem(CreateElementAttribute("SegHeight",XMLEncode(strSegHeight)));
	if (! strOpticalCenter.IsEmpty())
		NewNode ->attributes->setNamedItem(CreateElementAttribute("OpticalCenter",XMLEncode(strOpticalCenter)));
	return NewNode;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//Reqired
//SP_ORDER=>RX_DETAIL=>POSITION=>PRISCRIPTION
MSXML2::IXMLDOMNodePtr  CInvVisionWebUtils::CreateNodevwOrderXMLPrescription(LPCTSTR strSphere)	{
	MSXML2::IXMLDOMNodePtr pPrescriptionNode =CreateNode("PRESCRIPTION");
	pPrescriptionNode ->attributes->setNamedItem(CreateElementAttribute("Sphere",XMLEncode(strSphere)));
	return pPrescriptionNode ;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//Optional
//SP_ORDER=>RX_DETAIL=>POSITION=>PRISCRIPTION=>CYLINDER
MSXML2::IXMLDOMNodePtr  CInvVisionWebUtils::CreateNodevwOrderXMLPrescriptionCylinder(LPCTSTR strCylinderValue,LPCTSTR strCylinderAxis)
{
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("CYLINDER");
	NewNode->attributes->setNamedItem(CreateElementAttribute("Value",XMLEncode(strCylinderValue)));
	NewNode->attributes->setNamedItem(CreateElementAttribute("Axis",XMLEncode(strCylinderAxis)));
	return NewNode;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//Optional
//SP_ORDER=>RX_DETAIL=>POSITION=>PRISCRIPTION=>ADDITION
MSXML2::IXMLDOMNodePtr  CInvVisionWebUtils::CreateNodevwOrderXMLPrescriptionAddition(LPCTSTR strAdditionalValue){
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("ADDITION");
	NewNode->attributes->setNamedItem(CreateElementAttribute("Value",XMLEncode(strAdditionalValue)));
	return NewNode;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//Optional
// (s.dhole, 2011-12-08 12:29) - PLID 46941 added base and axis value
//SP_ORDER=>RX_DETAIL=>POSITION=>PRISCRIPTION=>PRISM
// (s.dhole 2013-06-11 15:55) - PLID 57125 Add ISOD
 MSXML2::IXMLDOMNodePtr  CInvVisionWebUtils::CreateNodevwOrderXMLPrescriptionPrism(BOOL IsOD, const CString& strPrismValue,
												const CString& strPrismAxis,const CString& strPrismAxisStr){
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("PRISM");
	CString  sPrismAxis =strPrismAxis;
	if (strPrismAxisStr.CompareNoCase("UP")== 0) {
		sPrismAxis ="90";
	}
	else if (strPrismAxisStr.CompareNoCase("DOWN")== 0){ 
		sPrismAxis ="270";
	}
	else if (strPrismAxisStr.CompareNoCase("IN")== 0) {
		// (s.dhole 2013-06-11 15:55) - PLID 57125 
		//OD(RIGHT)  IN = 0 
		//OS(LEFT)  IN = 180
		if (IsOD){
			sPrismAxis ="0";
		}
		else{
			sPrismAxis ="180";
		}
	}
	else if (strPrismAxisStr.CompareNoCase("OUT")== 0) {
		// (s.dhole 2013-06-11 15:55) - PLID 57125 
		//OD(RIGHT)  OUT = 180 
		//OS(LEFT)  OUT= 0
		if (IsOD){
			sPrismAxis ="180";
		}
		else{
			sPrismAxis ="0";
		}
	}
	NewNode->attributes->setNamedItem(CreateElementAttribute("Value",XMLEncode(strPrismValue)));
	NewNode->attributes->setNamedItem(CreateElementAttribute("Axis",XMLEncode(sPrismAxis )));
	if (! strPrismAxisStr.IsEmpty())
	NewNode->attributes->setNamedItem(CreateElementAttribute("AxisStr",XMLEncode(strPrismAxisStr)));
	return NewNode ;
}

 // (s.dhole 2010-11-02 15:04) - PLID 40540 
//Optional
//SP_ORDER=>RX_DETAIL=>POSITION=>BASECURVE
 MSXML2::IXMLDOMNodePtr  CInvVisionWebUtils::CreateNodevwOrderXMLPrescriptionBaseCurve(LPCTSTR strBaseCurveValue){
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("BASECURVE");
	NewNode->attributes->setNamedItem(CreateElementAttribute("BaseCurveValue",XMLEncode(strBaseCurveValue)));
	return NewNode ;
}

 // (s.dhole 2010-11-02 15:04) - PLID 40540 
//Optional
//SP_ORDER=>RX_DETAIL=>POSITION=>EQUITHIN
 MSXML2::IXMLDOMNodePtr  CInvVisionWebUtils::CreateNodevwOrderXMLPrescriptionEquithin(LPCTSTR strEquiThinValue){
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("EQUITHIN");
	NewNode->attributes->setNamedItem(CreateElementAttribute("Value", XMLEncode(strEquiThinValue)));
	return NewNode ;
}

 // (s.dhole 2010-11-02 15:04) - PLID 40540 
//Optional
//SP_ORDER=>RX_DETAIL=>POSITION=>CRIBBING
 MSXML2::IXMLDOMNodePtr  CInvVisionWebUtils::CreateNodevwOrderXMLPrescriptionCribbing(LPCTSTR strCribbingValue){
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("CRIBBING");
	NewNode->attributes->setNamedItem(CreateElementAttribute("Value", XMLEncode(strCribbingValue)));
	return NewNode ;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
 //Required
 //SP_ORDER=>RX_DETAIL=>POSITION->LENS
 MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLPositionLens(const CString& strName) {
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("LENS");
	if (!strName.IsEmpty())
		NewNode->attributes->setNamedItem(CreateElementAttribute("Name", XMLEncode(strName)));
	return NewNode ;
 }

 // (s.dhole 2010-11-02 15:04) - PLID 40540 
//Optional
//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>DIAMETER
 MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLPositionLensDiameter(LPCTSTR strCommercial,LPCTSTR strPhysical) {
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("DIAMETER");
	NewNode->attributes->setNamedItem(CreateElementAttribute("Commercial", XMLEncode(strCommercial)));
	NewNode->attributes->setNamedItem(CreateElementAttribute("Physical", XMLEncode(strPhysical)));
	return NewNode ;
 }

// (s.dhole 2010-11-02 15:04) - PLID 40540 
 //Required
//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>DESIGN
  MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLPositionLensDesign(const CString& strVwCode,const CString& strLmsCode) {
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("DESIGN");
	if (! strLmsCode.IsEmpty() )
		NewNode->attributes->setNamedItem(CreateElementAttribute("LmsCode", XMLEncode(strLmsCode)));
	NewNode->attributes->setNamedItem(CreateElementAttribute("VwCode", XMLEncode(strVwCode)));
	return NewNode ;
 }

  // (s.dhole 2010-11-02 15:04) - PLID 40540 
//Required
//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>MATERIAL
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLPositionLensMaterial(const CString& strVwCode, const CString& strLmsCode) {
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("MATERIAL");
	if (! strLmsCode.IsEmpty() )
		NewNode->attributes->setNamedItem(CreateElementAttribute("LmsCode", XMLEncode(strLmsCode)));
	NewNode->attributes->setNamedItem(CreateElementAttribute("VwCode", XMLEncode(strVwCode)));
	return NewNode ;
}
  
// (s.dhole 2010-11-02 15:04) - PLID 40540 
//Optional
//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>THICKNESS
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLPositionLensThickness(LPCTSTR strValue,LPCTSTR strType) {
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("THICKNESS");
	NewNode->attributes->setNamedItem(CreateElementAttribute("Value", XMLEncode(strValue)));
	NewNode->attributes->setNamedItem(CreateElementAttribute("Type", XMLEncode(strType)));
	return NewNode ;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//Optional
//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>SEGMENT
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLPositionLensSegment(LPCTSTR strX,LPCTSTR strY) {
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("SEGMENT");
	NewNode->attributes->setNamedItem(CreateElementAttribute("X", XMLEncode(strX)));
	NewNode->attributes->setNamedItem(CreateElementAttribute("Y", XMLEncode(strY)));
	return NewNode ;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//Optional
//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>TREATMENTS
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLPositionLensTreaments(const CString&  strComments){
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("TREATMENTS");
	if (!strComments.IsEmpty() )
		NewNode->attributes->setNamedItem(CreateElementAttribute("Comment",XMLEncode(strComments)));
	return NewNode ;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//Optional
//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>TREATMENTS->TREATMENT
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLPositionLensTreamentsTreament(const CString&  strVwCode,const CString&  strLmsCode, const CString&  strValue) {
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("TREATMENT");
	if (! strLmsCode.IsEmpty() )
		NewNode->attributes->setNamedItem(CreateElementAttribute("LmsCode",XMLEncode(strLmsCode)));
	NewNode->attributes->setNamedItem(CreateElementAttribute("VwCode",XMLEncode(strVwCode)));
	if (!strValue.IsEmpty() )
		NewNode->attributes->setNamedItem(CreateElementAttribute("Value",XMLEncode(strValue)));
	return NewNode ;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//Required
//POSITION=>LENS=>POSITION=>SHAPE
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLPositionShape(   const CString& strA, const CString& strB,
																			   const CString& strHalfDbl, const CString& strED, 
																			   const CString& strPoints,   const CString& strTracerID){
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("SHAPE");
	NewNode->attributes->setNamedItem(CreateElementAttribute("A",XMLEncode(strA)));
	NewNode->attributes->setNamedItem(CreateElementAttribute("B",XMLEncode(strB)));
	// (s.dhole 2012-07-20 11:12) - PLID 51327 
	NewNode->attributes->setNamedItem(CreateElementAttribute("HalfDbl",XMLEncode(ConvertDBLForVisionWeb(strHalfDbl))));
	if (! strED.IsEmpty() )
		NewNode->attributes->setNamedItem(CreateElementAttribute("ED",XMLEncode(strED)));
	if (! strPoints.IsEmpty())
		NewNode->attributes->setNamedItem(CreateElementAttribute("Points",XMLEncode(strPoints)));
	if (!strTracerID.IsEmpty()) 
		NewNode->attributes->setNamedItem(CreateElementAttribute("TracerID",XMLEncode(strTracerID)));
	return NewNode ;
}


// (s.dhole 2012-07-20 11:12) - PLID 51327 When we submit this value we have to submitt as half value
CString  CInvVisionWebUtils::ConvertDBLForVisionWeb(const CString& strHalfDbl)
{ 
	CString strResult = strHalfDbl;
	double nHalfDbl = atof(strHalfDbl);
	if (nHalfDbl>0){
		nHalfDbl=(nHalfDbl/2.00);
		strResult = AsString(AsDoubleRounded(nHalfDbl,3)) ;
	}
	return strResult;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//Optional
//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>OTHER=>OMMA
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLRx_DetailOtherOma(const CString&  strData){
	MSXML2::IXMLDOMNodePtr NewNode =CreateNode("OMA");
	NewNode->attributes->setNamedItem(CreateElementAttribute("DATA",XMLEncode(strData)));
	return NewNode;
}

// (s.dhole 2010-11-02 15:04) - PLID 40540 
//Optional
//SP_ORDER=>RX_DETAIL=>POSITION=>LENS=>OTHER=>XYPOINTS=>XYPOINT
MSXML2::IXMLDOMNodePtr CInvVisionWebUtils::CreateNodevwOrderXMLRx_DetailOtherXYPoint(LPCTSTR strX,LPCTSTR strY,LPCTSTR strZ)
{
MSXML2::IXMLDOMNodePtr NewNode =CreateNode("XYPOINTTYPE");
NewNode->attributes->setNamedItem(CreateElementAttribute("X", XMLEncode( strX)));
NewNode->attributes->setNamedItem(CreateElementAttribute("Y",XMLEncode(strY)));
NewNode->attributes->setNamedItem(CreateElementAttribute("Z",XMLEncode(strZ)));
return NewNode ;
}
 
// (s.dhole 2010-11-02 15:04) - PLID 41125 / This function will return formated string
CString  CInvVisionWebUtils::ConvertFloatToString(DOUBLE  val,BOOL IsSign)
{
	CString  strTemp;
	// non of field can hold value more than 320 so 999 is invalid data
	if (val!=999){
		strTemp.Format("%5.2f",val); 
		if (IsSign==TRUE && strTemp.GetLength()<=5 ){
			if (val>=0){
				strTemp.Replace("+","0"); 
				strTemp= "+" + strTemp  ;  
			}
			else{
				strTemp.Replace("-","0"); 
				strTemp= "-" + strTemp  ;  
			}
		}
		strTemp.Replace(" ","0"); 
	}
	return strTemp;
}


// (s.dhole 2010-12-13 11:36) - PLID 41125 Create HTML Document to show user what they are submmitting to visionWeb
// added ConvertToHTMLEmbeddable 
// change Supplier and location Address format
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
// (s.dhole 2011-04-12 09:06) - PLID 43237 added error code , log and flag
CString   CInvVisionWebUtils::GetHTML(long nOrderId,OUT BOOL &ISError)
{ 
	CString strErrorInfo="";
	CString strHTMLHDR="";
	strHTMLHDR+="<html xmlns=\"http://www.w3.org/1999/xhtml\">\r\n";
	strHTMLHDR+="<head>\r\n";
	strHTMLHDR+="    <title></title>\r\n";
	strHTMLHDR+="    <style type=\"text/css\">\r\n";
	strHTMLHDR+="      body{\r\n";
	strHTMLHDR+="		font-family: Verdana;\r\n";
	strHTMLHDR+="		font-size: 10px;\r\n";
	strHTMLHDR+="    }\r\n";
	strHTMLHDR+="    td {\r\n";
	strHTMLHDR+="      font-weight: bold;\r\n";
	strHTMLHDR+="      font-size: 10px;\r\n";
	strHTMLHDR+="      border-collapse:collapse;\r\n";
	strHTMLHDR+="		vertical-align:top;\r\n";
	strHTMLHDR+="    }\r\n";
	strHTMLHDR+="    .tdNoBold {\r\n";
	strHTMLHDR+="      font-weight: normal;\r\n";
	strHTMLHDR+="      border: thin solid #000000 ;\r\n";
	strHTMLHDR+="      font-size: 10px;\r\n";
	strHTMLHDR+="      border-collapse:collapse;\r\n";
	strHTMLHDR+="		vertical-align:top;\r\n";
	strHTMLHDR+="    }\r\n";
	strHTMLHDR+="    .tdNoBoldCenter {\r\n";
	strHTMLHDR+="      font-weight: normal;\r\n";
	strHTMLHDR+="      border: thin solid #000000 ;\r\n";
	strHTMLHDR+="      font-size: 10px;\r\n";
	strHTMLHDR+="      border-collapse:collapse;\r\n";
	strHTMLHDR+="		vertical-align:top;\r\n";
	strHTMLHDR+="		text-align:center;\r\n";
	strHTMLHDR+="    }\r\n";

	strHTMLHDR+="    .tdNoBoldWithWidth {\r\n";
	strHTMLHDR+="      font-weight: normal;\r\n";
	strHTMLHDR+="      border: thin solid #000000 ;\r\n";
	strHTMLHDR+="      font-size: 10px;\r\n";
	strHTMLHDR+="      border-collapse:collapse;\r\n";
	strHTMLHDR+="		vertical-align:top;\r\n";
	strHTMLHDR+="		width:4%;\r\n";
	strHTMLHDR+="    }\r\n";

	strHTMLHDR+="    .tdTitle {\r\n";
	strHTMLHDR+="      border: thin solid #000000 ;\r\n";
	strHTMLHDR+="      font-weight: bold;\r\n";
	strHTMLHDR+="      font-size: 12px;\r\n";
	strHTMLHDR+="      text-align :center ;\r\n";
	strHTMLHDR+="      border-collapse:collapse;\r\n";
	strHTMLHDR+="    }\r\n";
	strHTMLHDR+="    .tdBorder {\r\n";
    strHTMLHDR+="      border: thin solid #000000 ;\r\n";
   	strHTMLHDR+="      font-size: 10px;\r\n";
	strHTMLHDR+="      border-collapse:collapse;\r\n";
	
	strHTMLHDR+="    }\r\n";
	strHTMLHDR+="    .tdBorderWithWidth {\r\n";
    strHTMLHDR+="      border: thin solid #000000 ;\r\n";
   	strHTMLHDR+="      font-size: 10px;\r\n";
	strHTMLHDR+="      border-collapse:collapse;\r\n";
	strHTMLHDR+="		width:4%;\r\n";
	strHTMLHDR+="    }\r\n";

	strHTMLHDR+="    .tdBorderNoWidth {\r\n";
    strHTMLHDR+="      border: thin solid #000000 ;\r\n";
   	strHTMLHDR+="      font-size: 10px;\r\n";
	strHTMLHDR+="      border-collapse:collapse;\r\n";
	
	strHTMLHDR+="    }\r\n";

	strHTMLHDR+="   .tdEmpty {\r\n";
	strHTMLHDR+="      border: thin solid #000000 ;\r\n";
	strHTMLHDR+="      background-color: #FFFF00;\r\n";
	strHTMLHDR+="		font-size: 10px;\r\n";
	strHTMLHDR+="      border-collapse:collapse;\r\n";
	
	strHTMLHDR+="    }\r\n";
	strHTMLHDR+="    .tdError{\r\n";
	strHTMLHDR+="      color : #FF0000;\r\n";
	strHTMLHDR+="      background-color: #C0C0C0;\r\n";
	strHTMLHDR+="      border-collapse:collapse;\r\n";
	strHTMLHDR+="    }\r\n";
	// (s.dhole 2011-04-12 09:06) - PLID 43237 style for error 
	strHTMLHDR+="    .tdMissingError{\r\n";
	strHTMLHDR+="      border: thin solid #000000 ;\r\n";
	strHTMLHDR+="      color : #FFFFFF;\r\n";
	strHTMLHDR+="      background-color: #F62217;\r\n";
	strHTMLHDR+="    }\r\n";
	strHTMLHDR+="    table {\r\n";
	strHTMLHDR+="      border: thin solid #000000 ;\r\n";
	strHTMLHDR+="		font-size: 10px;\r\n";
	strHTMLHDR+="		vertical-align:top;\r\n";
	strHTMLHDR+="      border-collapse:collapse;\r\n";
	strHTMLHDR+="		width:100%; \r\n";
	strHTMLHDR+="    }\r\n";
	strHTMLHDR+="    .Hlftable {\r\n";
	strHTMLHDR+="      border: thin solid #000000 ;\r\n";
	strHTMLHDR+="		font-size: 10px;\r\n";
	strHTMLHDR+="		vertical-align:top;\r\n";
	strHTMLHDR+="      border-collapse:collapse;\r\n";
	strHTMLHDR+="		width:50%; \r\n";
	strHTMLHDR+="    }\r\n";
	strHTMLHDR+="    .HlftableNoTableBorder {\r\n";
	strHTMLHDR+="      border: none \r\n";
	strHTMLHDR+="		font-size: 10px;\r\n";
	strHTMLHDR+="		vertical-align:top;\r\n";
	strHTMLHDR+="      border-collapse:collapse;\r\n";
	strHTMLHDR+="		width:50%; \r\n";
	strHTMLHDR+="    }\r\n";
	strHTMLHDR+="	.NoHlfTdBorder {\r\n";
	strHTMLHDR+="		vertical-align:top;\r\n";
	strHTMLHDR+="		width:50%; \r\n";
	strHTMLHDR+="    }\r\n";
	strHTMLHDR+="  .NoTableBorder{\r\n";
	strHTMLHDR+="      border: none \r\n";
	strHTMLHDR+="      border-collapse:collapse;\r\n";
	strHTMLHDR+="		vertical-align:top;\r\n";
	strHTMLHDR+="    }\r\n";
	// (s.dhole 2011-04-12 09:06) - PLID 43237 style for error
	strHTMLHDR+="    P.warningTitle{\r\n";
   	strHTMLHDR+="    color: #000000;\r\n";
	strHTMLHDR+="	font-size: 12px;\r\n";
   	strHTMLHDR+="    font-weight: bold; \r\n";
	strHTMLHDR+="    }\r\n";
	strHTMLHDR+="    P.warning {\r\n";
   	strHTMLHDR+="    color: #FF0000;\r\n";
   	strHTMLHDR+="    font-weight: bold; \r\n";
	strHTMLHDR+="    }\r\n";

	strHTMLHDR+="</style>\r\n";
	strHTMLHDR+="</head>\r\n";
	strHTMLHDR+="<body   >\r\n";
	CString  strHTMLBody="<table  >";
	_RecordsetPtr rsOrder = CInvVisionWebUtils::GetOrderRecordset(nOrderId) ;
	if(!rsOrder->eof) 
		{
			strHTMLBody+="<tr><td colspan='25' class=\"tdTitle\">Order #: ";
			strHTMLBody+=FormatString("%s  </td></tr> \r\n", ConvertLongToString( AdoFldLong(rsOrder->Fields, "ID",-999)));
			COleDateTime dtNow = AdoFldDateTime(rsOrder->Fields, "currentDate") ;
			strHTMLBody+=FormatString("<tr><td colspan='15'>Date: %s</td><td colspan='10'>User: %s</td></tr>" ,FormatDateTimeForInterface(  dtNow,NULL,dtoDate ),ConvertToHTMLEmbeddable(AdoFldString(rsOrder->Fields, "UserName", "")));
			strHTMLBody+="<tr><td  colspan='25'> ";
			strHTMLBody+="<table  class=\"NoTableBorder\"> ";
			strHTMLBody+="<tr><td  class=\"NoHlfTdBorder\" >";
			strHTMLBody+="<table  >";
			 // (s.dhole 2011-04-12 09:06) - PLID 43237 error log
			if (AdoFldString(rsOrder->Fields, "VisionWebID", "")=="")
				strErrorInfo += "Missing VisionWeb Supplier/Lab Code <BR>";
			if ( AdoFldLong(rsOrder->Fields, "VisionWebAccountID", 0)==0)
				strErrorInfo += "Missing VisionWeb Location Account Code <BR>";
			strHTMLBody+=FormatString("<tr><td   class=\"tdBorder\" >Supplier/Lab: </td>%s</tr>",GstXmlTD(AdoFldString(rsOrder->Fields, "Supplier_Company", ""),"class=\"tdNoBold\"",FALSE ));
			// (s.dhole 2011-04-12 09:06) - PLID 43237 error log
			strHTMLBody+=FormatString("<tr><td   class=\"tdBorder\" >VisionWeb Code: </td>%s</tr>",GstXmlTD(AdoFldString(rsOrder->Fields, "VisionWebID", ""),"class=\"tdNoBold\"",TRUE ));
			strHTMLBody+=FormatString("<tr><td  class=\"tdBorder\">Address: </td><td  class=\"tdNoBold\">%s %s &nbsp;</td></tr>",ConvertToHTMLEmbeddable(AdoFldString(rsOrder->Fields, "SupplierAddress1", ""))
								  ,ConvertToHTMLEmbeddable(AdoFldString(rsOrder->Fields, "Supplier_Address2", "")));
			CString strAddress2 = FormatString("%s, %s %s", AdoFldString(rsOrder->Fields, "Supplier_City", ""),AdoFldString(rsOrder->Fields, "Supplier_State", ""),AdoFldString(rsOrder->Fields, "Supplier_Zip", ""));
			strHTMLBody+=FormatString("<tr><td  class=\"tdBorder\">&nbsp;</td><td  class=\"tdNoBold\">%s&nbsp;</td></tr>",ConvertToHTMLEmbeddable(strAddress2));
		    strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Phone: </td>%s</tr>",GstXmlTD(AdoFldString(rsOrder->Fields, "Supplier_WorkPhone", " ")+ " ","class=\"tdNoBold\"",FALSE ));
			strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Fax: </td>%s</tr>",GstXmlTD(AdoFldString(rsOrder->Fields, "Supplier_Fax", " ") + " " ,"class=\"tdNoBold\"",FALSE ));
			strHTMLBody+="</table> </td>";
			strHTMLBody+="<td  class=\"NoHlfTdBorder\" >";
			strHTMLBody+="<table >";
			strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Location: </td>%s</tr>",GstXmlTD(AdoFldString(rsOrder->Fields, "Locations_Name", ""),"class=\"tdNoBold\"",FALSE ));
			// (s.dhole 2011-04-12 09:06) - PLID 43237 error log
			strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Account Code: </td>%s</tr>",GstXmlTD(ConvertLongToString( AdoFldLong( rsOrder->Fields,"VisionWebAccountID",-999)),"class=\"tdNoBold\"",TRUE ));
			strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Address: </td><td class=\"tdNoBold\">%s %s &nbsp;</td></tr>",ConvertToHTMLEmbeddable(AdoFldString(rsOrder->Fields, "Locations_Address1", "")),ConvertToHTMLEmbeddable(AdoFldString(rsOrder->Fields, "Locations_Address2", "")));
			strAddress2 = FormatString("%s, %s %s" ,AdoFldString(rsOrder->Fields, "Locations_City", ""),AdoFldString(rsOrder->Fields, "Locations_State", ""),AdoFldString(rsOrder->Fields, "Locations_Zip", ""));
			strHTMLBody+=FormatString("<tr><td  class=\"tdBorder\">&nbsp;</td><td  class=\"tdNoBold\">%s&nbsp;</td></tr>",ConvertToHTMLEmbeddable(strAddress2));
			
			strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Phone: </td>%s</tr>",GstXmlTD(AdoFldString(rsOrder->Fields, "Phone", "") + " ","class=\"tdNoBold\"",FALSE ));
			strHTMLBody+="</table> ";
			strHTMLBody+="</td></tr>";
			strHTMLBody+="</table  >";
			strHTMLBody+="</td></tr>";
			strHTMLBody+="<tr>";
			CString strDob="";
			COleDateTime  dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);
			COleDateTime dtDob= AdoFldDateTime (rsOrder->Fields, "Patient_BirthDate",dtInvalid);
			if (dtDob.GetStatus()==COleDateTime::valid ) 
			{
				strDob=FormatDateTimeForInterface(  dtDob,dtoDate );
			}
			// (s.dhole, 2011-12-08 12:29) - PLID 46941 - Validation to Prism1
			if (AdoFldDouble(rsOrder->Fields, "RPrismValue2",999.0)  != 999.0 &&
				AdoFldDouble(rsOrder->Fields, "RPrismValue",999.0) == 999.0) 
				strErrorInfo += "Missing Prescription Rx Prism1 value <BR> ";

			strHTMLBody+=FormatString("<td class=\"NoHlfTdBorder\" colspan='13'>Patient: %s, %s %s</td><td class=\"NoHlfTdBorder\" colspan='12'>&nbsp;Date of Birth: %s</td>",
								  ConvertToHTMLEmbeddable(AdoFldString(rsOrder->Fields, "Last", "")),ConvertToHTMLEmbeddable(AdoFldString(rsOrder->Fields, "First", ""))
								  ,ConvertToHTMLEmbeddable(AdoFldString(rsOrder->Fields, "Middle", "")),strDob);
			strHTMLBody+="</tr>";
			strHTMLBody+="<tr>";
			strHTMLBody+="<td colspan='25' class=\"tdTitle\">Prescription Rx</td></tr>";

			strHTMLBody+="<tr><td class=\"tdBorderWithWidth\" colspan='3' >Eye</td><td class=\"tdBorderWithWidth\" colspan='2'>Sphere</td><td class=\"tdBorderWithWidth\" colspan='2'>Cylinder</td><td class=\"tdBorderWithWidth\" colspan='2'>Axis</td><td class=\"tdBorderWithWidth\" colspan='2'>Addition</td><td class=\"tdBorderWithWidth\" colspan='2'>Prism1</td><td class=\"tdBorderWithWidth\" colspan='2'>Base1</td><td class=\"tdBorderWithWidth\" colspan='2'>Prism2</td><td class=\"tdBorderWithWidth\" colspan='2'>Base2</td><td class=\"tdBorderWithWidth\" colspan='2'>Dist. Pd</td><td class=\"tdBorderWithWidth\" colspan='2'>Near Pd</td><td class=\"tdBorderWithWidth\" colspan='2'>Height</td></tr>";
			strHTMLBody+="<tr><td class=\"tdBorderWithWidth\" colspan='3' >OD</td>";

			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "RPrescriptionSphere", 999.0),TRUE),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			
			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "RCylinderValue", 999.0),TRUE),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			strHTMLBody+=GstXmlTD(ConvertLongToString(AdoFldLong(rsOrder->Fields, "RCylinderAxis", -999)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "RAdditionValue", 999.0)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "RPrismValue",999.0)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			if (AdoFldLong(rsOrder->Fields, "RPrismAxis", -999) != -999)
				strHTMLBody+=GstXmlTD(ConvertLongToString(AdoFldLong(rsOrder->Fields, "RPrismAxis", -999)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			else 
				strHTMLBody+=GstXmlTD(AdoFldString(rsOrder->Fields, "RPrismAxisStr", ""),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "RPrismValue2",999.0)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			// (s.dhole, 2011-12-08 12:29) - PLID 46941 - Check isf user submit string
			if (AdoFldLong(rsOrder->Fields, "RPrismAxis2", -999) != -999)
				strHTMLBody+=GstXmlTD(ConvertLongToString(AdoFldLong(rsOrder->Fields, "RPrismAxis2", -999)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			else 
				strHTMLBody+=GstXmlTD(AdoFldString(rsOrder->Fields, "RPrismAxisStr2", ""),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "RFarHalfPd", 999.0)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "RNearHalfPd", 999.0)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "RSegHeight", 999.0)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			
			strHTMLBody+="</tr>";
			strHTMLBody+="<tr><td class=\"tdBorderWithWidth\" colspan='3' >OS</td>";
			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LPrescriptionSphere", 999.0),TRUE),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LCylinderValue", 999.0),TRUE),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			strHTMLBody+=GstXmlTD(ConvertLongToString(AdoFldLong(rsOrder->Fields, "LeftCylinderAxis", -999)),"class=\"tdNoBoldWithWidth\"",FALSE,2 );
			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LAdditionValue",999.0)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LPrismValue",999.0)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			if (AdoFldLong(rsOrder->Fields, "LPrismAxis", -999) != -999)
				strHTMLBody+=GstXmlTD(ConvertLongToString(AdoFldLong(rsOrder->Fields, "LPrismAxis", -999)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			else 
				strHTMLBody+=GstXmlTD(AdoFldString(rsOrder->Fields, "LPrismAxisStr", ""),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LPrismValue2",999.0)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			// (s.dhole, 2011-12-08 12:29) - PLID 46941 - Check isf user submit string
			if (AdoFldLong(rsOrder->Fields, "LPrismAxis2", -999) != -999)
				strHTMLBody+=GstXmlTD(ConvertLongToString(AdoFldLong(rsOrder->Fields, "LPrismAxis2", -999)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			else 
				strHTMLBody+=GstXmlTD(AdoFldString(rsOrder->Fields, "LPrismAxisStr2", ""),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LFarHalfPd", 999.0)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LNearHalfPd", 999.0)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			strHTMLBody+=GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LSegHeight", 999.0)),"class=\"tdNoBoldWithWidth\"",FALSE ,2);
			
			strHTMLBody+="</tr>";
			
			strHTMLBody+="<tr><td  colspan='6' class=\"tdBorder\">Job Type:&nbsp;</td>";
			strHTMLBody+=FormatString("%s",GstXmlTD(AdoFldString(rsOrder->Fields, "GlassesJobType", ""),"class=\"tdNoBold\"",FALSE ,6) );
			strHTMLBody+="<td colspan='4' class=\"tdBorder\">Job Instruction:&nbsp;</td >";
			strHTMLBody+=FormatString("%s",GstXmlTD(AdoFldString(rsOrder->Fields, "GlassesJobNote", ""),"class=\"tdNoBold\"",FALSE ,9) );
			strHTMLBody+="</tr>";
			strHTMLBody+="<tr><td colspan='25' class=\"tdTitle\">Lens Information </td></tr>";
			strHTMLBody+="<tr><td  colspan='25'> ";
			strHTMLBody+="<table  class=\"NoTableBorder\"> ";
			strHTMLBody+="<tr><td  class=\"NoHlfTdBorder\" >";
			_RecordsetPtr rsTreatment ;
			if (ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "RPrescriptionSphere", 999.0),TRUE)!="")
			{
				strHTMLBody+="<table> ";
				strHTMLBody+="<tr><td colspan=\"2\" class=\"tdTitle\">OD</td> </tr>";
				// (s.dhole 2011-04-12 09:06) - PLID 43237 error log and show error
				strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Design: </td>%s</tr>",GetCatalogXmlTD(AdoFldString(rsOrder->Fields, "RDesignName", ""), "class=\"tdNoBold\"",AdoFldLong(rsOrder->Fields, "RDesignsProcessTypeID", 1),"OD => Design ",strErrorInfo));
				strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Material: </td>%s</tr>",GetCatalogXmlTD(AdoFldString(rsOrder->Fields, "RMaterialName", ""),"class=\"tdNoBold\"",AdoFldLong(rsOrder->Fields, "RMaterialProcessTypeID", 1),"OD => Material ",strErrorInfo));
				// (s.dhole 2011-04-12 09:06) - PLID 43237 added GlassesOrderProcessTypeID
				_RecordsetPtr rsTreatment = CreateParamRecordset(" SELECT GlassesOrderTreatmentsT.GlassesOrderOtherInfoID, \r\n"
						"GlassesCatalogTreatmentsT.TreatmentName, \r\n"
						" GlassesCatalogTreatmentsT.TreatmentCode, GlassesOrderTreatmentsT.GlassesCatalogTreatmentID, \r\n"
						" GlassesCatalogTreatmentsT.GlassesOrderProcessTypeID  \r\n"
						" FROM GlassesOrderTreatmentsT INNER JOIN \r\n"
						" GlassesCatalogTreatmentsT ON  \r\n"
						" GlassesOrderTreatmentsT.GlassesCatalogTreatmentID = GlassesCatalogTreatmentsT.ID \r\n"
						" WHERE GlassesOrderTreatmentsT.GlassesOrderOtherInfoID = {INT}",AdoFldLong(rsOrder->Fields, "RightGlassesOrderOtherInfoID", 0));
				if(!rsTreatment->eof) {
					while(!rsTreatment->eof) {
						// (s.dhole 2011-04-12 09:06) - PLID 43237 error log and show error
						strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Treatment:&nbsp;</td>%s</tr>",GetCatalogXmlTD(AdoFldString(rsTreatment->Fields, "TreatmentName", ""), "class=\"tdNoBold\"",AdoFldLong(rsTreatment->Fields, "GlassesOrderProcessTypeID", 1),"OD => Treatment ",strErrorInfo)); 
						rsTreatment->MoveNext();
					}
					strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Treatment Note:&nbsp;</td>%s</tr>",GstXmlTD(AdoFldString(rsOrder->Fields, "RTreatmentsComment", ""),"class=\"tdNoBold\"",FALSE ));
					rsTreatment->Close(); 
				}
				else{
					rsTreatment->Close();
				}
				if (AdoFldDouble(rsOrder->Fields, "RThicknessValue", 999.0)!=0 && AdoFldString(rsOrder->Fields, "RThicknessType", "")!="DRS" && AdoFldString(rsOrder->Fields, "RThicknessType", "")!="")
				{
				strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Thickness Type:&nbsp;</td>%s</tr>",GstXmlTD(AdoFldString(rsOrder->Fields, "RThicknessType", ""),"class=\"tdNoBold\"",FALSE ));
				strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Thickness:&nbsp;</td>%s</tr>",GstXmlTD(ConvertFloatToString( AdoFldDouble (rsOrder->Fields, "RThicknessValue", 999.0)),"class=\"tdNoBold\"",FALSE ));
				}
				strHTMLBody+="</table> ";
			}
			else
			{
				 strHTMLBody+="&nbsp;";
			}
			strHTMLBody+="</td>";
			strHTMLBody+="<td  class=\"NoHlfTdBorder\"> ";
			//strHTMLBody+="</td> ";
			//strHTMLBody+="<td>&nbsp;</td>";
			//
			//strHTMLBody+="<td colspan='12'>  ";
			if (ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LPrescriptionSphere", 999.0),TRUE) !="")
			{
			strHTMLBody+="<table> ";
			strHTMLBody+="<tr><td colspan=\"2\" class=\"tdTitle\">OS</td> </tr>";
			// (s.dhole 2011-04-12 09:06) - PLID 43237 error log and show error
			strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Design:&nbsp;</td>%s</tr>",GetCatalogXmlTD(AdoFldString(rsOrder->Fields, "LDesignName", ""),"class=\"tdNoBold\"",AdoFldLong(rsOrder->Fields, "LDesignsProcessTypeID", 1),"OS => Design ",strErrorInfo));
			strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Material:&nbsp;</td>%s</tr>",GetCatalogXmlTD(AdoFldString(rsOrder->Fields, "LMaterialName", ""),"class=\"tdNoBold\"",AdoFldLong(rsOrder->Fields, "LMaterialProcessTypeID", 1),"OS => Material ",strErrorInfo));
			// (s.dhole 2011-04-12 09:06) - PLID 43237 added GlassesOrderProcessTypeID
			rsTreatment = CreateParamRecordset(" SELECT GlassesOrderTreatmentsT.GlassesOrderOtherInfoID, \r\n"
						"GlassesCatalogTreatmentsT.TreatmentName, \r\n"
						" GlassesCatalogTreatmentsT.TreatmentCode, GlassesOrderTreatmentsT.GlassesCatalogTreatmentID, \r\n"
						" GlassesCatalogTreatmentsT.GlassesOrderProcessTypeID  \r\n"
						" FROM GlassesOrderTreatmentsT INNER JOIN \r\n"
						" GlassesCatalogTreatmentsT ON  \r\n"
						" GlassesOrderTreatmentsT.GlassesCatalogTreatmentID = GlassesCatalogTreatmentsT.ID \r\n"
						" WHERE GlassesOrderTreatmentsT.GlassesOrderOtherInfoID = {INT}",AdoFldLong(rsOrder->Fields, "LeftGlassesOrderOtherInfoID", 0));
				if(!rsTreatment->eof) {
					while(!rsTreatment->eof) {
						// (s.dhole 2011-04-12 09:06) - PLID 43237 error log and show error
						strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Treatment:&nbsp;</td>%s</tr>",GetCatalogXmlTD(AdoFldString(rsTreatment->Fields, "TreatmentName", ""), "class=\"tdNoBold\"",AdoFldLong(rsTreatment->Fields, "GlassesOrderProcessTypeID", 1),"OS => Treatment ",strErrorInfo)); 
						rsTreatment->MoveNext();
					}
					strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Treatment Notes:&nbsp;</td>%s</tr>",GstXmlTD(AdoFldString(rsOrder->Fields, "LTreatmentsComment", ""),"class=\"tdNoBold\"",FALSE ));
					rsTreatment->Close(); 
				}
				else{
					rsTreatment->Close();
				}
				// (s.dhole 2012-02-24 12:21) - PLID 47979 Thickness can be null
				if (AdoFldDouble(rsOrder->Fields, "LThicknessValue", 999.0)!=0 && AdoFldString(rsOrder->Fields, "LThicknessType", "")!="DRS"  && AdoFldString(rsOrder->Fields, "LThicknessType", "")!="" )
				{
				strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Thickness Type:&nbsp;</td>%s</tr>",GstXmlTD(AdoFldString(rsOrder->Fields, "LThicknessType", ""),"class=\"tdNoBold\"",FALSE ));
				strHTMLBody+=FormatString("<tr><td class=\"tdBorder\">Thickness:&nbsp;</td>%s</tr>",GstXmlTD(ConvertFloatToString(AdoFldDouble(rsOrder->Fields, "LThicknessValue", 999.0)),"class=\"tdNoBold\"",FALSE ));
				}
				strHTMLBody+="</table>";
			}
			else{
			strHTMLBody+="&nbsp;";
			}
			strHTMLBody+="</td>";
			strHTMLBody+="</tr>";
			strHTMLBody+="</table>";
			strHTMLBody+="</td>";
			strHTMLBody+="</tr>";
			strHTMLBody+="<tr><td colspan='25'>";
			strHTMLBody+="<table  class=\"NoTableBorder\" ><tr>";
			strHTMLBody+="<td>";
			strHTMLBody+=FormatString("<table><tr><td class=\"tdBorder\">Box A:</td>%s</tr></table>",GstXmlTD(ConvertFloatToString(AdoFldDouble (rsOrder->Fields, "ShapeA", 999.0)),"class=\"tdNoBold\"",TRUE ));
			strHTMLBody+="</td><td>";
			strHTMLBody+=FormatString("<table><tr><td class=\"tdBorder\">B:</td>%s</tr></table>",GstXmlTD(ConvertFloatToString(AdoFldDouble (rsOrder->Fields, "ShapeB", 999.0)),"class=\"tdNoBold\"",TRUE ));
			strHTMLBody+="</td><td>";
			strHTMLBody+=FormatString("<table><tr><td class=\"tdBorder\">DBL:</td>%s</tr></table>",GstXmlTD(ConvertFloatToString(AdoFldDouble (rsOrder->Fields, "ShapeHalfDbl", 999.0)),"class=\"tdNoBold\"",TRUE ));
			strHTMLBody+="</td><td>";
			strHTMLBody+=FormatString("<table><tr><td class=\"tdBorder\">ED:</td>%s</tr></table>",GstXmlTD(ConvertFloatToString(AdoFldDouble (rsOrder->Fields, "ShapeED", 999.0)),"class=\"tdNoBold\"",FALSE ));
			strHTMLBody+="</td><td>";
			strHTMLBody+=FormatString("<table><tr><td class=\"tdBorder\">Temple length:</td>%s</tr></table>",GstXmlTD( AdoFldString(rsOrder->Fields, "Temple", ""),"class=\"tdNoBold\"",FALSE ));
			strHTMLBody+="</td><td>";
			strHTMLBody+=FormatString("<table><tr><td class=\"tdBorder\">Eye Size:</td>%s</tr></table>",GstXmlTD( AdoFldString(rsOrder->Fields, "Eye", ""),"class=\"tdNoBold\"",FALSE ));
			
			strHTMLBody+="</td>";
			strHTMLBody+="</tr>"; 
			strHTMLBody+="</table>";

			strHTMLBody+="</td></tr>";
			strHTMLBody+="<tr><td colspan='25' class=\"tdTitle\">Frame Information</td></tr>";
			strHTMLBody+=FormatString("<tr><td  class=\"tdBorderNoWidth\"  colspan='8'>Frame Type: </td>%s</tr>",GetCatalogXmlTD(AdoFldString(rsOrder->Fields, "FrameTypeName", ""), "class=\"tdNoBold\"",AdoFldLong(rsOrder->Fields, "FrameTypeProcessTypeID", 1),"Frame Type ",strErrorInfo,17)); 
			
			strHTMLBody+=FormatString("<tr><td class=\"tdBorderNoWidth\" colspan='8'>Frame: </td>%s</tr>",GstXmlTD(AdoFldString(rsOrder->Fields, "BrandName", "") ,"class=\"tdNoBold\"",FALSE ,17));
			strHTMLBody+=FormatString("<tr><td class=\"tdBorderNoWidth\" colspan='8'>Manufacturer: </td>%s</tr>",GstXmlTD(AdoFldString(rsOrder->Fields, "ManufacturerName", ""),"class=\"tdNoBold\"",FALSE ,17));
			strHTMLBody+=FormatString("<tr><td class=\"tdBorderNoWidth\" colspan='8'>Style: </td>%s</tr>",GstXmlTD(AdoFldString(rsOrder->Fields, "StyleName", ""),"class=\"tdNoBold\"",FALSE ,17));
			strHTMLBody+=FormatString("<tr><td class=\"tdBorderNoWidth\" colspan='8'>Color: </td>%s</tr>",GstXmlTD(AdoFldString(rsOrder->Fields, "ColorDescription", ""),"class=\"tdNoBold\"",FALSE ,17));
			
		 
			// (s.dhole 2010-12-13 12:15) - PLID  we are waiting for this item to be implmentd by VisionWeb
			//Add customize data Add-on 
			/*_RecordsetPtr rsCustomData = CreateParamRecordset("SELECT vwCP.ParameterName, vwOP.ParameterValue \r\n"
											" FROM  GlassesCustomParameterT AS vwCP  \r\n"
											" INNER JOIN GlassesOrderParameterT  AS vwOP ON  \r\n"
											" vwCP.GlassesCustomParameterID = vwOP.GlassesCustomParameterID \r\n"
											" WHERE (vwOP.GlassesOrderID = {INT}) ORDER BY vwCP.DisplayOrder DESC ", nOrderId);
			if(!rsCustomData->eof) {
				strHTMLBody+="<tr><td colspan='25' class=\"tdTitle\">Customization Data</td></tr>";
				while(!rsCustomData->eof) {
				strHTMLBody+=FormatString("<tr><td  class=\"tdBorder\" colspan='6'>" + AdoFldString(rsCustomData->Fields, "ParameterName", "") + " : </td>%s</tr>",GstXmlTD(AdoFldString(rsCustomData->Fields, "ParameterValue", ""),"class=\"tdNoBold\"",FALSE ,19));
				rsCustomData->MoveNext();
				}
			rsCustomData->Close(); 
			}*/
		}
		else{
			strHTMLBody+="<tr><td colspan='25' style=\"color: #FF0000; font-size: large; font-weight: bold; text-align: center; font-family: verdana;\">Incomplete order!";
			strHTMLBody+="</td></tr>";
			strHTMLBody+="<tr><td colspan='25' style=\"color: #FF0000; font-size: medium ; font-weight: bold ; text-align: center;font-family: verdana;\">";
			strHTMLBody+="Please verify all required data fields to process this order. ";
			strHTMLBody+="</td></tr>";
			ISError=TRUE;
			
		}
		strHTMLBody+="</table></body>\r\n";
		strHTMLBody+="</html>\r\n";
		// (s.dhole 2011-04-12 09:06) - PLID 43237 error log and show error
	CString strErroList="";
	if (!strErrorInfo.IsEmpty() )
	{
		 strErroList= "<P class='warningTitle' >Please correct the following errors before submitting this order </P> <P class='warning' > " 
			 + strErrorInfo  + "</P>";
		 ISError=TRUE;
	}

	return  strHTMLHDR  + strErroList + strHTMLBody ;
}
// (s.dhole 2010-12-13 11:36) - PLID 41125
CString  CInvVisionWebUtils::GstXmlTD(CString  strData,CString  strFormatClass,BOOL IsRequired , long nColspan )
{
	CString  strFormatEmptyClass="class=\"tdEmpty\"";
	//CString  strFormatErrorClass ="class=\"tdError\"";
	CString  strFormatErrorClass ="class=\"tdMissingError\"";
	CString strTD=""; 
	CString strFormat=strFormatClass; 
	if (strData.IsEmpty() && IsRequired  )
		strTD.Format("<td %s colspan='%li' >%s&nbsp;</td>",  strFormatErrorClass ,nColspan,ConvertToHTMLEmbeddable(strData));
	else if (strData.IsEmpty())  
		strTD.Format("<td %s colspan='%li'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>",  strFormatEmptyClass,nColspan);
	else
		strTD.Format("<td %s colspan='%li'>%s&nbsp;</td>",  strFormatClass,nColspan,ConvertToHTMLEmbeddable(strData));
	return strTD;
}

// (s.dhole 2011-04-12 09:06) - PLID 43237 error log and show error
CString  CInvVisionWebUtils::GetCatalogXmlTD(CString strData,CString  strFormatClass,long nProcessID  ,CString strDisplayField, OUT CString &strErrorInfo , long nColspan )
{
	CString  strFormatEmptyClass="class=\"tdEmpty\"";
	CString  strFormatErrorClass ="class=\"tdMissingError\"";
	CString strTD=""; 
	if (!strData.IsEmpty() && nProcessID ==2  ){
		strTD.Format("<td %s colspan='%li'>%s&nbsp;</td>",  strFormatClass,nColspan,ConvertToHTMLEmbeddable(strData));
	}
	else if (!strData.IsEmpty() && nProcessID !=2  ){
		strTD.Format("<td %s colspan='%li'>%s&nbsp;</td>",  strFormatErrorClass,nColspan,ConvertToHTMLEmbeddable(strData));
		strErrorInfo+=ConvertToHTMLEmbeddable(FormatString("Non-VisionWeb %s [%s] " ,strDisplayField, strData)) +"<BR>" ;
	}
	else{
		strTD.Format("<td %s colspan='%li'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>",  strFormatErrorClass,nColspan);
		strErrorInfo+=ConvertToHTMLEmbeddable(FormatString("Missing %s " ,strDisplayField, strData)) +"<BR>" ;
	}
	return strTD;
}



CInvVisionWebUtils::CInvVisionWebUtils(void)
{
}

CInvVisionWebUtils::~CInvVisionWebUtils(void)
{
}

//TES 9/27/2010 - PLID 40539 - Returns the user-friendly description of this Order Type
CString GetVisionWebOrderTypeDescription(VisionWebOrderType vwot)
{
	switch(vwot) {
		case vwotSpectacleLens:
			return "Spectacle Lens Order";
			break;
		case vwotFrame:
			return "Frame Order";
			break;
		case vwotContactLensPatient:
			return "Contact Lens Patient Order";
			break;
		case vwotContactLensOffice:
			return "Contact Lens Office Order";
			break;
		case vwotSpectacleLensFrame:
			return "Spectacle Lens Frame Order";
			break;
		default:
			ASSERT(FALSE);
			return "";
			break;
	}
}

//TES 9/27/2010 - PLID 40539 - Returns the datalist color to use for this Order Type
long GetVisionWebOrderTypeColor(VisionWebOrderType vwot)
{
	switch(vwot) {
		case vwotSpectacleLens:
			return 16777215;
			break;
		case vwotFrame:
			return 9764585;
			break;
		case vwotContactLensPatient:
			return 16774329;
			break;
		case vwotContactLensOffice:
			return 16699591;
			break;
		case vwotSpectacleLensFrame:
			return 9751038;
			break;
		default:
			ASSERT(FALSE);
			return 0;
			break;
	}
}

//TES 9/27/2010 - PLID 40539 - Returns the three-letter VisionWeb code for this Job Type
// THIS CODE IS SAVED TO DATA, DO NOT CHANGE!
// (s.dhole 2010-12-08 14:02) - PLID 41675 Add on changes support only 4 types
// (s.dhole 2012-02-21 09:39) - PLID 47979 added vwjtNone=""
CString GetVisionWebJobTypeCode(VisionWebJobType vwjt)
{
	switch(vwjt) {
		
		case vwjtNone:
			return "";
			break;
		case vwjtFrameToCome:
			return "FTC";
			break;
		case vwjtUncut:
			return "UNC";
			break;
		case vwjtLensesOnly:
			return "RED";
			break;
		case vwjtToBePurchased:
			return "TBP";
			break;
	/*	case vwjtFastUncut:
			return "FUN";
			break;
		case vwjtRemoteEdging:
			return "RED";
			break;
		case vwjtFramePackages:
			return "PAC";
			break;
		case vwjtSafetyPackages:
			return "SPA";
			break;
		case vwjtSupplyFrame:
			return "TBP";
			break;*/
		default:
			ASSERT(FALSE);
			return "";
			break;
	} 
}

//TES 9/27/2010 - PLID 40539 - Returns the user-friendly description for this Job Type
// (s.dhole 2010-12-08 14:02) - PLID 41675 Add on changes support only 4 types
// (s.dhole 2012-02-21 09:39) - PLID 47979 added <None>
CString GetVisionWebJobTypeDescription(VisionWebJobType vwjt)
{
	switch(vwjt) {
		case vwjtNone:
			return "<None>";
			break;
		case vwjtFrameToCome:
			return "Frame To Come";
			break;
		case vwjtUncut:
			return "Uncut";
			break;
		case vwjtLensesOnly:
			return "Lenses Only";
			break;
		case vwjtToBePurchased:
			return "To Be Purchased";
			break;
			

		//case vwjtFastUncut:
		//	return "Fast Uncut";
		//	break;
		//case vwjtRemoteEdging:
		//	return "Remote Edging";
		//	break;
		//case vwjtFramePackages:
		//	return "Frame Packages";
		//	break;
		//case vwjtSafetyPackages:
		//	return "Safety Packages";
		//	break;
		//case vwjtSupplyFrame:
		//	return "Supply Frame";
		//	break;
		default:
			ASSERT(FALSE);
			return "";
			break;
	}
}

//TES 3/11/2011 - PLID 42757 - Gets a user-friendly description for the enum value
CString GetGlassesOrderLensDescription(GlassesOrderLens gol)
{
	switch(gol) {
		case golInvalid:
			return "<None>";
			break;
		case golOD:
			return "OD";
			break;
		case golOS:
			return "OS";
			break;
	}
	ASSERT(FALSE);
	return "";
}

//TES 3/11/2011 - PLID 42757 - Gets a user-friendly description for the enum value
CString GetGlassesOrderRecordDescription(GlassesOrderDataType godt, long nRecordID, bool bIncludeTypeDescription)
{
	CString strTypeDescription = bIncludeTypeDescription?(GetGlassesOrderDataTypeDescription(godt) + " - "):"";
	switch(godt) {
		case godtLens:
			return strTypeDescription + GetGlassesOrderLensDescription((GlassesOrderLens)nRecordID);
			break;
		case godtRxNumber:
			return strTypeDescription + GetGlassesOrderRxNumberDescription((GlassesOrderRxNumber)nRecordID);
			break;
		case godtDesign:
			//TODO: See if we can avoid/reduce this data access
			return strTypeDescription + VarString(GetTableField("GlassesCatalogDesignsT", "DesignName", "ID", nRecordID),"<None>");
			break;
		case godtMaterial:
			//TODO: See if we can avoid/reduce this data access
			return strTypeDescription + VarString(GetTableField("GlassesCatalogMaterialsT", "MaterialName", "ID", nRecordID),"<None>");
			break;
		case godtTreatment:
			//TODO: See if we can avoid/reduce this data access
			return strTypeDescription + VarString(GetTableField("GlassesCatalogTreatmentsT", "TreatmentName", "ID", nRecordID),"<None>");
			break;
		case godtInvalid:
			return "<None>";
			break;
	}

	ASSERT(FALSE);
	return "";
}

//TES 3/11/2011 - PLID 42757 - Gets a user-friendly description for the enum value
CString GetGlassesOrderRxNumberDescription(GlassesOrderRxNumber gorn)
{
	switch(gorn) {
		case gornSphere:
			return "Sphere";
			break;
		case gornCylinder:
			return "Cylinder";
			break;
		case gornAxis:
			return "Axis";
			break;
		case gornAddition:
			return "Addition";
			break;
		case gornPrism:
			return "Prism";
			break;
		case gornBase:
			return "Base";
			break;
		case gornDistPD:
			return "Dist. PD";
			break;
		case gornNearPD:
			return "Near PD";
			break;
		case gornHeight:
			return "Height";
			break;

		//TES 4/6/2012 - PLID 49367 - Added entries for Contact Lens orders
		case gornBC:
			return "BC";
			break;
		case gornDiameter:
			return "Diameter";
			break;
		case gornColor:
			return "Color";
			break;
		case gornQuantity:
			return "Quantity";
			break;

		// (j.dinatale 2013-03-19 10:03) - PLID 53120 - need a new Contact Lens Brand field
		case gornManufacturer:
			return "Manufacturer";
			break;
			
		case gornInvalid:
			return "<None>";
			break;
	}
	
	ASSERT(FALSE);
	return "";
}

//TES 4/6/2012 - PLID 49367 - A function for determining whether the given entry appears on Glasses Orders, Contact Lens Orders, or both
BOOL IsRxNumberSupported(GlassesOrderRxNumber gorn, BOOL bContactLens)
{
	switch(gorn) {
		case gornSphere:
			return TRUE;
			break;
		case gornCylinder:
			return TRUE;
			break;
		case gornAxis:
			return TRUE;
			break;
		case gornAddition:
			return TRUE;
			break;
		case gornPrism:
			return !bContactLens;
			break;
		case gornBase:
			return !bContactLens;
			break;
		case gornDistPD:
			return !bContactLens;
			break;
		case gornNearPD:
			return !bContactLens;
			break;
		case gornHeight:
			return !bContactLens;
			break;

		case gornBC:
			return bContactLens;
			break;
		case gornDiameter:
			return bContactLens;
			break;
		case gornColor:
			return bContactLens;
			break;
		case gornQuantity:
			return bContactLens;
			break;

		// (j.dinatale 2013-03-19 10:03) - PLID 53120 - need a new Contact Lens Brand field
		case gornManufacturer:
			return bContactLens;
			break;
	}

	ASSERT(FALSE);
	return FALSE;
}


//TES 3/11/2011 - PLID 42757 - Gets a user-friendly description for the enum value
CString GetGlassesOrderDataTypeDescription(GlassesOrderDataType godt)
{
	switch(godt) {
	case godtLens:
		return "Lens";
		break;
	case godtRxNumber:
		return "Rx Field";
		break;
	case godtDesign:
		return "Design";
		break;
	case godtMaterial:
		return "Material";
		break;
	case godtTreatment:
		return "Treatment";
		break;
	case godtInvalid:
		return "<None>";
		break;
	}
	ASSERT(FALSE);
	return "";
}

//TES 10/16/2015 - PLID 66372 - Moved IsValidPrescriptionNumber and AsPrescriptionNumber to OpticalUtils.cpp

//TES 6/29/2011 - PLID 44381 - Added
CString GetGOServiceTypeDescription(GlassesOrderServiceType gost)
{
	switch(gost) {
		case gostFrame:
			return "Frame";
			break;
		case gostLens:
			return "Lens";
			break;
		case gostContactLens:
			return "Contact Lens";
		case gostOther:
			return "Other";
	}
	ASSERT(FALSE);
	return "<Unknown>";
}


//r.wilson (4/30/2012) PLID 43741
long GetOrderStatusAsInt(CString strStatus)
{			
	strStatus.MakeLower();

	CMap<CString,LPCSTR,long,long> *mapGlassesOrderStatus = &GetMainFrame()->m_mapGlassesOrderStatus;	

	if(mapGlassesOrderStatus->GetCount() == 0)
	{
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID,OrderStatusName FROM GlassesOrderStatusT");
		while (!rs->eof) {		
				CString tmpOrderStatusName = AdoFldString(rs,"OrderStatusName");
				 tmpOrderStatusName.MakeLower();
				long tmpOrderStatusID = AdoFldLong(rs,"ID");			
				mapGlassesOrderStatus->SetAt(tmpOrderStatusName,tmpOrderStatusID);
				rs->MoveNext();
			}
	}

	long nStatusId = 0;	
	mapGlassesOrderStatus->Lookup(strStatus,nStatusId);
	
	// r.wilson PLID 43741 is not a valid status in the database
	ASSERT(nStatusId != 0);

	return nStatusId;
}



//(7/9/2012 r.wilson) PLID 51423 - Prints Glasses report (OpticalRx.rpt)
void ShowPrescriptionReport(long nOrderId, BOOL nOrderType, CWnd *pParentWindow)
{
	try{			
	
			if(vwotSpectacleLens != 1 && vwotContactLensPatient != 3)
			{
				//(7/9/2012) r.wilson PLID 51423 - If the ordertype is not equal to one of these then we need to exit 
				return;
			}

			CString strQry = 
			"  SELECT  "
			"  PatientsT.PersonID AS PersonID, PersonT.First AS PatientFirstName,   "
			"  PersonT.Middle AS PatientMiddleName, PersonT.Last AS PatientLastName, PersonT.Address1 AS PatientAddress1,   "
			"  PersonT.Address2 AS PatientAddress2, PersonT.City AS PatientCity, PersonT.State AS PatientState, PersonT.Zip AS PatientZip,   "
			"  PersonT.Gender AS PatientGender, PersonT.HomePhone AS PatientHomePhone, PersonT.WorkPhone AS PatientWorkPhone, "
			"  PersonT.Extension AS PatientExtension, PersonT.CellPhone AS PatientCellPhone, PersonT.BirthDate AS PatientBirthDate,   "
			"  PersonT.SocialSecurity AS PatientSocialSecurity, LensRxT.ID AS ID,";

			if(nOrderType == vwotSpectacleLens)
			{
				//Glasses Order
				strQry += 
					" LensRxT.RxDate,RightLensDetailRxT.NearHalfPd AS RightEyeNearPd, RightLensDetailRxT.SegHeight AS RightEyeSegHeight,  "
					"  RightLensDetailRxT.PrescriptionSphere AS RightEyeSphere, RightLensDetailRxT.CylinderValue AS RightCylinder, RightLensDetailRxT.AdditionValue AS RightAddition, "
					"  RightLensDetailRxT.PrismValue AS RightPrism, RightLensDetailRxT.CylinderAxis AS RightPrismAxis, "
					"  Case WHEN RightLensDetailRxT.PrismAxis IS NULL THEN RightLensDetailRxT.PrismAxisStr ELSE CAST(RightLensDetailRxT.PrismAxis AS nvarchar(20)) END AS RightBase1, "
					"  Case WHEN RightLensDetailRxT.PrismAxis2 IS NULL THEN RightLensDetailRxT.PrismAxisStr2 ELSE CAST(RightLensDetailRxT.PrismAxis2 AS nvarchar(20)) END AS RightBase2,  "
					"  RightLensDetailRxT.FarHalfPd AS RightDistPD, "
					"  LeftLensDetailRxT.NearHalfPd AS LeftEyeNearPd, LeftLensDetailRxT.SegHeight AS LeftEyeSegHeight, "
					"  LeftLensDetailRxT.PrescriptionSphere AS LeftEyeSphere, LeftLensDetailRxT.CylinderValue AS LeftCylinder,  "
					"  LeftLensDetailRxT.AdditionValue AS LeftAddition, LeftLensDetailRxT.PrismValue AS LeftPrism, LeftLensDetailRxT.CylinderAxis AS LeftPrismAxis,   "
					"  Case WHEN  LeftLensDetailRxT.PrismAxis IS NULL THEN LeftLensDetailRxT.PrismAxisStr ELSE CAST(LeftLensDetailRxT.PrismAxis AS nvarchar(20)) END AS LeftBase1, "
					"  Case WHEN  LeftLensDetailRxT.PrismAxis2 IS NULL THEN LeftLensDetailRxT.PrismAxisStr2 ELSE CAST(LeftLensDetailRxT.PrismAxis2 AS nvarchar(20)) END AS LeftBase2, "
					"  LeftLensDetailRxT.FarHalfPd AS LeftDistPD, ";

				strQry += 
					"  PatientsT.UserDefinedID AS UserDefinedID, GlassesOrderT.ProviderID AS ProviderID, Providerdetail.First AS ProviderFirstName, Providerdetail.Middle AS ProviderMiddleName, Providerdetail.Last AS ProviderLastName,  "
					"  PrefixT.Prefix AS ProviderPrefix, Providerdetail.Title AS ProviderTitle, "
					"  LocationsT.Name AS LocationName, LocationsT.Address1 AS LocationAddress1, LocationsT.Address2 AS LocationAddress2,   "
					"  LocationsT.City AS LocationCity, LocationsT.State AS LocationState, LocationsT.Zip AS LocationZip, LocationsT.Phone AS LocationPhone,   "
					"  LocationsT.Fax AS LocationFax , "
					"  LensRxT.RxExpirationDate AS RxExpirationDate, RightLensDetailRxT.SecondaryPrismValue AS RightSecondaryPrism, LeftLensDetailRxT.SecondaryPrismValue AS LeftSecondaryPrism, ";

				//(7/9/2012 r.wilson) PLID 51423 - since this order is a glasses order we fill the contact lens info with nulls
				// (j.dinatale 2013-04-12 11:42) - PLID 55862 - include the Doc Ins fields for both lenses
				strQry += 
					"  NULL AS ConLensRxDate, NULL AS ConLensRightDiameter, NULL AS ConLensRightEyeSphere, "
					"  NULL AS ConLensRightCylinder, NULL AS ConLensRightCylinderAxis, "
					"  NULL AS ConLensRightBC, NULL AS ConLensRightAddition, NULL AS ConLensLeftDiameter, "
					"  NULL As ConLensLeftEyeSphere, NULL AS ConLensLeftCylinder, NULL AS ConLensLeftCylinderAxis, "
					"  NULL AS ConLensLeftAddition, NULL AS ConLensLeftBC, NULL AS ConLensRxExpirationDate, " 
					"  NULL AS ConLensRxRightDocIns, NULL AS ConLensRxLeftDocIns ";					
			}

			else if (nOrderType == vwotContactLensPatient)
			{
				//Contact Lens Order
				//(7/9/2012 r.wilson) PLID 51423 - since this order is a contact lens order we fill the glasses info with nulls
				strQry += 
					"  NULL AS RxDate, NULL AS RightEyeNearPd, NULL AS RightEyeSegHeight,  "
					"  NULL AS RightEyeSphere, NULL AS RightCylinder, NULL AS RightAddition, "
					"  NULL AS RightPrism, NULL AS RightPrismAxis, "
					"  NULL AS RightBase1, NULL AS RightBase2, NULL AS RightDistPD, "
					"  NULL AS LeftEyeNearPd, NULL AS LeftEyeSegHeight, "
					"  NULL AS LeftEyeSphere, NULL AS LeftCylinder,  "
					"  NULL AS LeftAddition, NULL AS LeftPrism, NULL AS LeftPrismAxis, "
					"  NULL AS LeftBase1, NULL AS LeftBase2, NULL AS LeftDistPD,";
				
				strQry += 
					"  PatientsT.UserDefinedID AS UserDefinedID, GlassesOrderT.ProviderID AS ProviderID, Providerdetail.First AS ProviderFirstName, Providerdetail.Middle AS ProviderMiddleName, Providerdetail.Last AS ProviderLastName,  "
					"  PrefixT.Prefix AS ProviderPrefix, Providerdetail.Title AS ProviderTitle, "
					"  LocationsT.Name AS LocationName, LocationsT.Address1 AS LocationAddress1, LocationsT.Address2 AS LocationAddress2,   "
					"  LocationsT.City AS LocationCity, LocationsT.State AS LocationState, LocationsT.Zip AS LocationZip, LocationsT.Phone AS LocationPhone,   "
					"  LocationsT.Fax AS LocationFax , "
					//r.wilson RxExpirationDate is NULL on purpose here 
					"  NULL AS RxExpirationDate, RightLensDetailRxT.SecondaryPrismValue AS RightSecondaryPrism, LeftLensDetailRxT.SecondaryPrismValue AS LeftSecondaryPrism, ";

				// (j.dinatale 2013-04-12 11:42) - PLID 55862 - include the Doc Ins fields for both lenses
				strQry +=
					"  LensRxT.RxDate AS ConLensRxDate, RightLensDetailRxT.Diameter AS ConLensRightDiameter, RightLensDetailRxT.PrescriptionSphere AS ConLensRightEyeSphere, "
					"  RightLensDetailRxT.CylinderValue AS ConLensRightCylinder, RightLensDetailRxT.CylinderAxis AS ConLensRightCylinderAxis, "
					"  RightLensDetailRxT.BC AS ConLensRightBC, RightLensDetailRxT.AdditionValue AS ConLensRightAddition, LeftLensDetailRxT.Diameter AS ConLensLeftDiameter, "
					"  LeftLensDetailRxT.PrescriptionSphere As ConLensLeftEyeSphere, LeftLensDetailRxT.CylinderValue AS ConLensLeftCylinder, LeftLensDetailRxT.CylinderAxis AS ConLensLeftCylinderAxis, "
					"  LeftLensDetailRxT.AdditionValue AS ConLensLeftAddition, LeftLensDetailRxT.BC AS ConLensLeftBC, LensRxT.RxExpirationDate  AS ConLensRxExpirationDate, "
					"  RightLensDetailRxT.DocIns AS ConLensRxRightDocIns, LeftLensDetailRxT.DocIns AS ConLensRxLeftDocIns ";
			}
			

			CString strFromAndWhere = "";
			strFromAndWhere.Format(
					" FROM "
					" GlassesOrderT INNER JOIN PatientsT ON GlassesOrderT.PersonID = PatientsT.PersonID "
					" INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					" INNER JOIN LensRxT ON GlassesOrderT.LensRxID = LensRxT.ID "
					" LEFT OUTER JOIN LensDetailRxT AS LeftLensDetailRxT ON  LensRxT.LeftLensDetailRxID = LeftLensDetailRxT.ID "
					" LEFT OUTER JOIN LensDetailRxT AS RightLensDetailRxT ON  LensRxT.RightLensDetailRxID = RightLensDetailRxT.ID "
					" INNER JOIN LocationsT ON GlassesOrderT.LocationID = LocationsT.ID "
					" LEFT JOIN PersonT AS Providerdetail ON GlassesOrderT.ProviderID = Providerdetail.ID  "
					" LEFT JOIN ProvidersT ON Providerdetail.ID = ProvidersT.PersonID  "
					" LEFT OUTER JOIN PrefixT ON Providerdetail.PrefixID = PrefixT.ID	 "
					" WHERE GlassesOrderT.ID = %li",
					nOrderId
				);


			//Add From Clause
			strQry += strFromAndWhere;

			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(737)]);
			infReport.strListBoxSQL = strQry;

			CPrintInfo prInfo;

			CPrintDialog* dlg;
			dlg = new CPrintDialog(FALSE);
			
			//(7/9/2012) r.wilson PLID 51423 - this will always be a print preview 
			prInfo.m_bPreview = true;
			
			prInfo.m_bDirect = false;
			prInfo.m_bDocObject = false;
			if(prInfo.m_pPD != NULL) {
				delete prInfo.m_pPD;
			}
			prInfo.m_pPD = dlg;			

			//(r.wilson 6/4/2012) PLID 48952 - Initialize empty params and run the report
			CPtrArray aryParams;	
			RunReport(&infReport, &aryParams, TRUE , pParentWindow, "Optical Prescriptions",  &prInfo);
			ClearRPIParameterList(&aryParams);		//clear our parameters now that the report is done
	}NxCatchAll(__FUNCTION__)
}