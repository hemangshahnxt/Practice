#pragma once

#include "SoapUtils.h"
#include "GenericBrowserDlg.h"

// (a.walling 2009-05-12 11:03) - PLID 29924 - Initial creation of utility namespace for CCD documents
//(e.lally 2010-02-18) PLID 37438 - Moved all the practice/interface dependencies into their own source file
namespace CCD
{
	#pragma region CCD Viewing And Parsing
	// (a.walling 2010-01-06 14:03) - PLID 36809 - Take a document pointer and handle CCD/CCR
	IStreamPtr GetOutputHTMLStream(MSXML2::IXMLDOMDocument2Ptr pDocument) throw(...);
	CString GetCCDXSLResourcePath() throw(...);
	// (b.spivey, August 13, 2013) - PLID 57964 - get CCDA XSL
	CString GetCCDAXSLResourcePath() throw(...); 
	// (a.walling 2010-01-06 13:27) - PLID 36809 - XSL file for CCR
	CString GetCCRXSLResourcePath() throw(...);	
	
	struct CTimeRange {
		CTimeRange() {
			dtLow.SetStatus(COleDateTime::invalid);
			dtHigh.SetStatus(COleDateTime::invalid);
		}

		void LoadFromElement(MSXML2::IXMLDOMNodePtr pNode);
		static COleDateTime ParseFromString(const CString& strTime);

		// will be null if explicitly set to 0
		// will be invalid if they do not exist in the data
		COleDateTime dtLow; 
		COleDateTime dtHigh;

		COleDateTime GetSingleTime() const;
		void SetSingleTime(const COleDateTime& dt);
		void SetTimeRange(const COleDateTime& dtNewLow, const COleDateTime& dtNewHigh);
	};

	// (j.jones 2014-08-04 13:33) - PLID 63157 - this now needs a patient ID
	void UpdateCCDInformation(const CString& strFile, long nMailID, long nPatientID) throw(...);

	// (j.jones 2010-06-30 10:35) - PLID 38031 - moved a number of XML file validation functions
	// into NxXMLUtils

	// (a.walling 2009-05-21 09:25) - PLID 34176 - Consolidated some CCD description functions
	void GetDescriptionInfo(const CString& strFile, const CString& strDefault, OUT CString& strTitle, OUT CString& strDisplay, OUT CString& strSuggestedDescription, OUT CTimeRange& trTime) throw(...);
	void GetDescriptionInfo(MSXML2::IXMLDOMDocument2Ptr pDocument, const CString& strDefault, OUT CString& strTitle, OUT CString& strDisplay, OUT CString& strSuggestedDescription, OUT CTimeRange& trTime) throw(...);

	// (a.walling 2010-01-06 13:11) - PLID 36809 - Get description info from a CCD
	void GetCCDDescriptionInfo(MSXML2::IXMLDOMDocument2Ptr pDocument, const CString& strDefault, OUT CString& strTitle, OUT CString& strDisplay, OUT CString& strSuggestedDescription, OUT CTimeRange& trTime) throw(...);
	// (a.walling 2010-01-06 13:11) - PLID 36809 - Get description info from a CCD
	void GetCCRDescriptionInfo(MSXML2::IXMLDOMDocument2Ptr pDocument, const CString& strDefault, OUT CString& strTitle, OUT CString& strDisplay, OUT CString& strSuggestedDescription, OUT CTimeRange& trTime) throw(...);
	// (j.gruber 2013-11-08 11:02) - PLID 59376
	void GetCCDADescriptionInfo(MSXML2::IXMLDOMDocument2Ptr pDocument, const CString& strDefault, OUT CString& strTitle, OUT CString& strDisplay, OUT CString& strSuggestedDescription, OUT CTimeRange& trTime) throw(...);

	// (a.walling 2010-01-06 13:06) - PLID 36809 - Get the date time from the CCR ISO 8601 string
	COleDateTime GetCCRDateTimeFromNode(MSXML2::IXMLDOMNodePtr pNode);

	// (a.walling 2009-05-13 12:34) - PLID 34243 - Review the CCD and choose a matching patient
	long ReviewAndMatchPatient(const CString& strFile, long nCurPatientID, CWnd* pParentWnd) throw(...);
	// (a.walling 2010-01-06 12:21) - PLID 36809 - Get matching patient from CCD or CCR
	void GetMatchingPatient(MSXML2::IXMLDOMDocument2Ptr pDocument, CArray<long, long>& arPatientIDs) throw(...);
	// (a.walling 2009-05-13 08:41) - PLID 34243 - Get matching patient from CCD information
	void GetMatchingCCDPatient(MSXML2::IXMLDOMDocument2Ptr pDocument, CArray<long, long>& arPatientIDs) throw(...);
	// (a.walling 2010-01-06 12:21) - PLID 36809 - Get matching patient from CCR
	void GetMatchingCCRPatient(MSXML2::IXMLDOMDocument2Ptr pDocument, CArray<long, long>& arPatientIDs) throw(...);
	// (j.gruber 2013-11-08 10:22) - PLID 59376 - match CCDA patient
	void GetMatchingCCDAPatient(MSXML2::IXMLDOMDocument2Ptr pDocument, CArray<long, long>& arPatientIDs)throw(...);
	// (a.walling 2010-01-06 13:03) - PLID 36809 - Get maching patient from given demographics
	void GetMatchingPatientFromDemographics(const CString& strFirst, const CString& strLast, const COleDateTime& dtBirthDate, long nGender, CArray<long, long>& arPatientIDs) throw(...);

	class CReviewCCDDlg : public CGenericBrowserDlg
	{	
		DECLARE_DYNAMIC(CReviewCCDDlg)

	public:
		CReviewCCDDlg(CWnd* pParent) : CGenericBrowserDlg(pParent) {
			m_nPatientID = -1;
			m_nCurrentPatientID = -1;
		};

	protected:
		afx_msg virtual LRESULT OnLabelClicked(WPARAM wParam, LPARAM lParam);
		virtual void InitializeControls();
		virtual void UpdateControls();
		virtual void OnCancel();

		CString GetCaption();

	public:
		long m_nPatientID;
		long m_nCurrentPatientID;
	};
#pragma endregion

// (j.jones 2010-06-30 11:41) - PLID 38031 - moved the GenericXMLBrowserDlg into its own class

}