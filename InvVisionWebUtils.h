#pragma once

// (j.dinatale 2013-04-03 15:24) - PLID 56075 - Need a new enum for the prompting stuff and also some handy functions
namespace OrderServiceListType {
	enum ServiceListType{
		Designs,
		Materials,
		Treatments,
	};

	CString GetCptTableName(ServiceListType slt);
	CString GetCptColumnName(ServiceListType slt);
	CString GetDisplayName(ServiceListType slt);
};

// (s.dhole 2010-09-24 12:47) - PLID 40540 VisionWeb
class CInvVisionWebUtils
{
private:
	

	CString GetDateTimeValue(COleDateTime dt);
	void ShowErrors(const CString &strOutputString,const long nOrderID);
	CString  ConvertFloatToString(DOUBLE  val,BOOL IsSign= FALSE); // (s.dhole 2010-11-02 15:04) - PLID 41125 Formating floating  string
	CString  ConvertLongToString(long  val);
	CString  ConvertDecimalStringFromString(CString val);
	ADODB::_RecordsetPtr  GetOrderRecordset(long nOrderID);
	
	CString GetvwOrderErrorLensDetail(LPCTSTR strResult);
	// (s.dhole 2010-11-02 15:04) - PLID 40728
	CString GetvwOrderErrorDesignMaterialDetail(LPCTSTR strResult,LPCTSTR strType, LPCTSTR strCode ,LPCTSTR strDescription);
	// (s.dhole 2010-11-02 15:04) - PLID 40728
	BOOL UpdateCatalogInfo(MSXML2::IXMLDOMDocument2Ptr pDoc,LPCTSTR strType,LPCTSTR strTable, LPCTSTR strCode, LPCTSTR strDescription);
	


protected:

	//this want  use many  places, best way to declare in include
	MSXML2::IXMLDOMDocument2Ptr pDoc;
	MSXML2::IXMLDOMNodePtr CreateNode(LPCTSTR szName) throw(...); // creates a XML element
	MSXML2::IXMLDOMNodePtr CreateElementAttribute(LPCTSTR strType,LPCTSTR strValue ); // Add a XML element attributes
	MSXML2::IXMLDOMAttributePtr FindChildAttribut(MSXML2::IXMLDOMNamedNodeMapPtr lpParent, LPCTSTR strChildName); // Find Attribut 

	MSXML2::IXMLDOMNodePtr SubmitOrderToVisionWeb(MSXML2::IXMLDOMNodePtr  pOrderxml); 
	
	bool  SubmitOrderTovw(long nOrderID);

	BOOL IsResultHasError(MSXML2::IXMLDOMNode *lpParent,OUT CString &strErrorCode, OUT CString &strErrorDescription);
	
	MSXML2::IXMLDOMNodePtr CallSoapFunction(const CString &strURL, const CString &strURI,  const CString &strParamXml, const CString& strUsername = "", const CString& strPassword = "" , const BOOL IsWebService = FALSE );
	
	CString BuildSoapCallXmlXml(const CString &strParamXml);
	CString BuildSoapServiceCallXmlXml(const CString &strParamXml);
	MSXML2::IXMLDOMNodePtr GetErrorForDesignMaterialFromvw(const CString &strURL , const CString &strUserID,const CString &strPSW,
		const CString &strRefID,const CString &strSupplierID,const CString &strDesignCode,const CString &strMaterialCode);
	
	BOOL ValidatevwOrderXML(const CString& strXMLOrder);
	CString  GetvwOrderErrorDetail(const CString &strSupplierID,const  CString &strLeftDesignCode,const  CString &strLeftMaterialCode,
		const  CString &strRightDesignCode,const  CString &strRightMaterialCode,const  CString &strSupplierInfo="",const CString &strLocationInfo ="",
		const  CString &strLeftDesignInfo="",const  CString &strRightDesignInfo="",const  CString &strLeftMaterialInfo="",
		const  CString &strRightMaterialInfo="");	
	CString  GetvwErrorDescription(const CString &strErrorCode,const CString &strSupplierID ="",const  CString &strLeftDesignCode="",const  CString &strLeftMaterialCode="",
		const  CString &strRightDesignCode="",const  CString &strRightMaterialCode="",const  CString &strSupplierInfo="",const CString &strLocationInfo="",
		const  CString &strLeftDesignInfo="",const  CString &strRightDesignInfo="",const  CString &strLeftMaterialInfo="",
		const  CString &strRightMaterialInfo="", BOOL ISFormated =FALSE);

	CString XMLDecode(LPCTSTR str);
	MSXML2::IXMLDOMNodePtr  GetVisionOrderStatusByOrderID(LPCTSTR strExchangeID);

	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLHeader(LPCTSTR strLoginID,LPCTSTR strOrderType,LPCTSTR strPassword,LPCTSTR strSubmittedDate 
		, LPCTSTR strSubmitterGuid, LPCTSTR strSubmitterId, LPCTSTR strSubmitterOrderId, LPCTSTR strSupplierId, LPCTSTR strVersionNum);

	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLSP_ORDER(const CString& strCreationDate,const CString& strPoNum,
														   const CString& strType ="",const CString&  strStatus="");

	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLSP_OrderRedo_Information(LPCTSTR  strOriginalOrderNumber,
														LPCTSTR  strSupplierInvoiceNumber,LPCTSTR  strLmsCode,  
														LPCTSTR  strVwCode,LPCTSTR  strEye);

	MSXML2::IXMLDOMNodePtr  CreateNodevwOrderXMLAccount( const CString& strCbsId,
													const CString& strDropShipAccountId ="");

	void  AppendNodevwOrderXMLpatient( MSXML2::IXMLDOMNodePtr pPatient,const CString&  strLastName,
																		  const CString&  strFirstName="");
	// (s.dhole 2010-11-02 15:04) - PLID //we are waiting for this item to be implmentd by VisionWeb 
	//MSXML2::IXMLDOMNodePtr  CreateNodevwOrderXMLPersonalizedData(const CString&  strPatientInitials)	;

	// (s.dhole 2010-11-02 15:04) - PLID  
	//we are waiting for this item to be implmentd by VisionWeb 
	/*MSXML2::IXMLDOMNodePtr  CreateNodevwOrderXMLSpecialData(  const CString&  strName,  
												const CString&  strValue);	*/
	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLJob(  const CString&  strType,  const CString&  strInstruction="");

	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLFrame( const CString& strNumber,  const CString&  strBrand  ="",
														const CString&  strModel ="",  const CString&  strColor="",
												   const CString&  strEyeSize="",  const CString&  strSKU="",
												   const CString& strLmsCode="",  const CString&  strVwCode="",
												   const CString&  strTempletType="",  const CString&  strTypeName="", 
												   const CString&  strLength="");

	MSXML2::IXMLDOMNodePtr  CreateNodevwOrderXMLFrameSupplier(LPCTSTR strName, LPCTSTR strCode);
	
	MSXML2::IXMLDOMNodePtr  CreateNodevwOrderXMLFramePackage(  const CString&  strName,  const CString&  strCode,  
													const CString&  strSafety="");

	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLPosition(const CString& strEye,  const CString& strFarHalfPd,
													     const CString& strSegHeight, const CString& strNearHalfPd ="",  
														 const CString& strOpticalCenter="");

	MSXML2::IXMLDOMNodePtr  CreateNodevwOrderXMLPrescription( LPCTSTR strSphere);

	MSXML2::IXMLDOMNodePtr  CreateNodevwOrderXMLPrescriptionCylinder(LPCTSTR strCylinderValue,LPCTSTR strCylinderAxis);

	MSXML2::IXMLDOMNodePtr  CreateNodevwOrderXMLPrescriptionAddition(LPCTSTR strAdditionalValue);
	// (s.dhole 2013-06-11 15:55) - PLID 57125 Add ISOD
	MSXML2::IXMLDOMNodePtr  CreateNodevwOrderXMLPrescriptionPrism(BOOL IsOD, const CString&  strPrismValue,
																	 const CString& strPrismAxis,
																	 const CString&  strPrismAxisStr= ""); // OD Right
	MSXML2::IXMLDOMNodePtr  CreateNodevwOrderXMLPrescriptionBaseCurve(LPCTSTR strBaseCurveValue);
	MSXML2::IXMLDOMNodePtr  CreateNodevwOrderXMLPrescriptionEquithin(LPCTSTR strEquiThinValue);
	MSXML2::IXMLDOMNodePtr  CreateNodevwOrderXMLPrescriptionCribbing(LPCTSTR strCribbingValue);
	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLPositionLens(const CString& strName="");
	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLPositionLensDiameter(LPCTSTR strCommercial,LPCTSTR strPhysical);
	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLPositionLensDesign(const CString& strVwCode,const CString& strLmsCode ="");
	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLPositionLensMaterial(const CString& strVwCode, const CString& strLmsCode ="");
	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLPositionLensThickness(LPCTSTR strValue,LPCTSTR strType);
	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLPositionLensSegment(LPCTSTR strX,LPCTSTR strY);
	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLPositionLensTreaments(const CString&  strComments="");
	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLPositionLensTreamentsTreament(const CString&  strVwCode,const CString&  strLmsCode ="",const CString&  strValue="");

	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLPositionShape(  const CString& strA,const CString& strB,  const CString& strHalfDbl ,
																				const CString& strED = "",
																			   const CString& strPoints = "",  const CString& strTracerID = "");
	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLRx_DetailOtherOma(const CString& strData);
	MSXML2::IXMLDOMNodePtr CreateNodevwOrderXMLRx_DetailOtherXYPoint(LPCTSTR strX,LPCTSTR strY,LPCTSTR strZ);
	CString GstXmlTD(CString  strData,CString  strFormatClass,BOOL IsRequired , long nColspan=1 );
	// (s.dhole 2011-04-12 09:06) - PLID 43237 error log and show error
	CString GetCatalogXmlTD(CString strData,CString  strFormatClass,long nProcessID  ,CString strDisplayField, OUT CString &strErrorInfo , long nColspan =1);
	// (s.dhole 2012-07-20 11:12) - PLID 51327 
	CString  ConvertDBLForVisionWeb(const CString& strHalfDbl);

public:
	CString  GetvwTrackingCodeDescription(const CString &strTrackingCode);
	BOOL CheckOrderResoponse(MSXML2::IXMLDOMNode *lpParent,OUT CString &strExchangeId,OUT CString &strCreationDate,  OUT CString &strNumber  ,OUT CString &strReference ,
									 OUT CString &strType, OUT CString &strErrorCode, OUT CString &strErrorDescription, OUT CString &strErrormessage);

	BOOL CheckOrderStatus(MSXML2::IXMLDOMNode *lpParent,OUT CString &strSupplierTrackingId,OUT CString &strType,OUT CString &strEstimatedDelivery,  
				OUT CString &strvwTrackingCode ,OUT CString &strSupplierTimeStamp, OUT CString &strErrorCode, OUT CString &strErrorDescription, OUT CString &strErrormessage);
	MSXML2::IXMLDOMNodePtr FindChildNode(MSXML2::IXMLDOMNode *lpParent, LPCTSTR strChildName);
	
	BOOL CheckSubmittedOrderStatus(long strOrderId ,OUT CString &strMessage , OUT COleDateTime &dt);
	// (s.dhole 2010-11-02 15:04) - PLID 40728
	BOOL UpdateVisionWebCatalog();
	BOOL UpdateOrderStatus(long nOrderId, long nUserID,long OrderType,long OrderStatus,LPCTSTR strvwOrderId,
										   LPCTSTR strURL,LPCTSTR strUserID,LPCTSTR strPSW ,LPCTSTR strRefId,
										   OUT CString &strMessage,OUT COleDateTime &dt ,ADODB::_ConnectionPtr pvwOrderConn );
	MSXML2::IXMLDOMNodePtr   GetvwCatalog();
	MSXML2::IXMLDOMNodePtr GetvwCatalogByDesign(const CString& DesignCode );
	MSXML2::IXMLDOMNodePtr  GetvwOrderStatus(LPCTSTR strvwOrderId,LPCTSTR strURL, LPCTSTR strUserID,LPCTSTR strPSW, LPCTSTR strRefId);
	long GetOrderTypeFromString(const CString &strType);
	MSXML2::IXMLDOMNodePtr  GetOrderXML(long nOrderID,LPCTSTR strUserId,LPCTSTR strPassword ,LPCTSTR strRefId ,
					OUT CString &strSupplierId, OUT CString &strLeftDesignCode,  OUT CString &strLeftMaterialCode, 
					OUT CString &strRightDesignCode, OUT CString &strRightMaterialCode, OUT CString &strSupplierInfo,OUT CString &strLocationInfo,
					OUT CString &strLeftDesignInfo,OUT CString &strRightDesignInfo,OUT CString &strLeftMaterialInfo,
					OUT CString &strRightMaterialInfo,OUT long &nOrderType);
	
	bool  UploadOrderToVisionWeb(long nOrderID);
	// (s.dhole 2011-04-12 09:06) - PLID 43237 added error flag
	CString   GetHTML(long nOrderId,OUT BOOL &ISError);
	
	CInvVisionWebUtils(void);
	~CInvVisionWebUtils(void);
};

//TES 9/27/2010 - PLID 40539 - An enum identifying different Order Types sent to VisionWeb
// STORED TO DATA, DO NOT CHANGE!
enum VisionWebOrderType
{
	vwotSpectacleLens = 1,
	vwotFrame = 2,
	vwotContactLensPatient = 3,
	vwotContactLensOffice = 4,
	vwotSpectacleLensFrame = 5,
	vwot_LastEnum = 6,
};

//TES 9/27/2010 - PLID 40539 - Returns the user-friendly description of this Order Type
CString GetVisionWebOrderTypeDescription(VisionWebOrderType vwot);
//TES 9/27/2010 - PLID 40539 - Returns the datalist color to use for this Order Type
long GetVisionWebOrderTypeColor(VisionWebOrderType vwot);

//TES 9/27/2010 - PLID 40539 - An enum identifying different Job Types sent to VisionWeb
// (s.dhole 2010-12-08 14:02) - PLID 41675 Add on changes support only 4 types
// (s.dhole 2012-02-21 09:39) - PLID 47979 added vwjtNone = 0,
enum VisionWebJobType
{
	vwjtNone = 0,
	vwjtFrameToCome = 1,
	vwjtUncut = 2,
	vwjtLensesOnly = 3,
	vwjtToBePurchased = 4,
	vwjt_LastEnum = 5,
	/*vwjtFastUncut = 3,
	vwjtRemoteEdging = 4,
	vwjtFramePackages = 5,
	vwjtSafetyPackages = 6,
	vwjtSupplyFrame = 7,
	vwjt_LastEnum = 8,*/
};

//TES 9/27/2010 - PLID 40539 - Returns the three-letter VisionWeb code for this Job Type
// THIS CODE IS SAVED TO DATA, DO NOT CHANGE!
CString GetVisionWebJobTypeCode(VisionWebJobType vwjt);
//TES 9/27/2010 - PLID 40539 - Returns the user-friendly description for this Job Type
CString GetVisionWebJobTypeDescription(VisionWebJobType vwjt);
CString GetVisionWebPasword();

//TES 3/11/2010 - PLID 42757 - An enum for OD/OS lens.  
//SAVED IN DATA, DO NOT CHANGE!
enum GlassesOrderLens
{
	golInvalid = -1,
	golOD = 0,
	golOS = 1,
};

//TES 3/11/2011 - PLID 42757 - EMR Items will have a Glasses Order Data Type, which will be one of the values in this enum.  Therefore,
// DO NOT CHANGE!
enum GlassesOrderDataType
{
	//TES 4/6/2012 - PLID 49367 - This enum is not saved to data, used as a workaround so that CEmrItemEntryDlg can give different Rx Numbers
	// depending on whether or not it's a contact lens order
	godtContactLensRxNumber = -2,
	godtInvalid = -1,
	godtLens,		//GlassesOrderDataID = GlassesOrderLens enum
	godtRxNumber,	//GlassesOrderDataID = GlassesOrderRxNumber enum
	godtDesign,		//GlassesCatalogDesignsT.ID
	godtMaterial,	//GlassesCatalogMaterialsT.ID
	godtTreatment,	//GlassesCatalogTreatmentsT.ID
};

//TES 3/11/2011 - PLID 42757 - Enum representing each of the possible fields in a Lens prescription
//TES 4/6/2012 - PLID 49367 - Added entries for fields used on ContactLens orders
// STORED TO DATA, DO NOT CHANGE!
enum GlassesOrderRxNumber
{
	gornInvalid = -1,
	gornSphere,
	gornCylinder,
	gornAxis,
	gornAddition,
	gornPrism,
	gornBase,
	gornDistPD,
	gornNearPD,
	gornHeight,

	gornBC,
	gornDiameter,
	gornColor,
	gornQuantity,

	// (j.dinatale 2013-03-19 10:03) - PLID 53120 - need the brand on a contact lens order
	gornManufacturer,

	gorn_LastEnum,
};

//TES 4/6/2012 - PLID 49367 - A function for determining whether the given entry appears on Glasses Orders, Contact Lens Orders, or both
BOOL IsRxNumberSupported(GlassesOrderRxNumber gorn, BOOL bIsContactLens);

//TES 3/11/2010 - PLID 42757 - Gets a user-friendly description for the enum value
CString GetGlassesOrderLensDescription(GlassesOrderLens gol);
CString GetGlassesOrderRecordDescription(GlassesOrderDataType godt, long nRecordID, bool bIncludeTypeDescription);
CString GetGlassesOrderRxNumberDescription(GlassesOrderRxNumber gorn);
CString GetGlassesOrderDataTypeDescription(GlassesOrderDataType godt);

//TES 3/18/2011 - PLID 42762 - EMR will fill these structs to pass on to a Glasses Order
struct GlassesOrderLensDetails {
	CString strSphere;
	CString strCylinder;
	CString strAxis;
	CString strAddition;
	CString strPrism;
	CString strBase;
	CString strDistPD;
	CString strNearPD;
	CString strHeight;

	long nDesignID;
	long nMaterialID;
	CArray<long,long> arTreatmentIDs;

	GlassesOrderLensDetails() {
		nDesignID = -1;
		nMaterialID = -1;
	}

	//TES 4/17/2012 - PLID 49746 - Added the = operator
	void operator =(GlassesOrderLensDetails &goldSource) {
		strSphere = goldSource.strSphere;
		strCylinder = goldSource.strCylinder;
		strAxis = goldSource.strAxis;
		strAddition = goldSource.strAddition;
		strPrism = goldSource.strPrism;
		strBase = goldSource.strBase;
		strDistPD = goldSource.strDistPD;
		strNearPD = goldSource.strNearPD;
		strHeight = goldSource.strHeight;
		nDesignID = goldSource.nDesignID;
		nMaterialID = goldSource.nMaterialID;
		arTreatmentIDs.RemoveAll();
		arTreatmentIDs.Append(goldSource.arTreatmentIDs);
	}
};

struct GlassesOrder {
	//TES 4/5/2011 - PLID 42762 - Actually, this is unnecessary, at least for now the struct always comes with a CEMN, so we can just
	// pull the Patient ID from there.
	//long nPatientID;
	GlassesOrderLensDetails golOD;
	GlassesOrderLensDetails golOS;
};

//TES 4/11/2012 - PLID 49621 - EMR will fill these structs to pass on to a Contact Lens order
struct ContactLensOrderLensDetails {
	CString strSphere;
	CString strCylinder;
	CString strAxis;
	CString strAddition;
	CString strBC;
	CString strDiameter;
	CString strColor;
	CString strQuantity;
	CString strManufacturer;	// (j.dinatale 2013-03-20 14:24) - PLID 55766

	//TES 4/17/2012 - PLID 49746 - Added the = operator
	void operator =(ContactLensOrderLensDetails &cordSource) {
		strSphere = cordSource.strSphere;
		strCylinder = cordSource.strCylinder;
		strAxis = cordSource.strAxis;
		strAddition = cordSource.strAddition;
		strBC = cordSource.strBC;
		strDiameter = cordSource.strDiameter;
		strColor = cordSource.strColor;
		strQuantity = cordSource.strQuantity;
		strManufacturer = cordSource.strManufacturer;	// (j.dinatale 2013-03-20 14:24) - PLID 55766
	}
};

struct ContactLensOrder {
	ContactLensOrderLensDetails clolOD;
	ContactLensOrderLensDetails clolOS;
};

//TES 10/16/2015 - PLID 66372 - Moved PrescriptionNumberFormat, IsValidPrescriptionNumber and AsPrescriptionNumber to OpticalUtils.h

//TES 6/29/2011 - PLID 44381 - Used to tie Glasses Order charges to the appropriate Cost field, for profit analysis
//SAVED TO DATA, DO NOT CHANGE!
enum GlassesOrderServiceType {
	gostOther = 0, //Not tied to any Cost field
	gostFrame = 1, //Tied to the Frame Cost field
	gostLens = 2, //Tied to the Lens Cost field
	gostContactLens = 3,	//Tied to the Cost field on Contact Lens orders
};

CString GetGOServiceTypeDescription(GlassesOrderServiceType gost);	

//r.wilson (4/30/2012) PLID 43741
long GetOrderStatusAsInt(CString strStatus);
// (7/9/2012) r.wilson PLID 51423- Prints a glasses or contact lens prescriptions
void ShowPrescriptionReport(long nOrderId, BOOL nOrderType, CWnd *pParentWindow);


