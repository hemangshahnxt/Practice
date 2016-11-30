

#pragma once



class PhoneNumber
{
	// Global variables
	private:
		CString countryCode;
		CString areaCode;
		CString localNumber;
		CString extension;

	// Properties
	public:
		CString GetCountryCode();
		void SetCountryCode(CString countryCode);
		CString GetAreaCode();
		void SetAreaCode(CString areaCode);
		CString GetLocalNumber();
		void SetLocalNumber(CString localNumber);
		CString GetExtension();
		void SetExtension(CString extension);
};