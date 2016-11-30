#include "stdafx.h"
#include "PhoneNumber.h"

// Properties
CString PhoneNumber::GetCountryCode()
{
	return this->countryCode;
}
void PhoneNumber::SetCountryCode(CString countryCode)
{
	this->countryCode = countryCode;
}
CString PhoneNumber::GetAreaCode()
{
	return this->areaCode;
}
void PhoneNumber::SetAreaCode(CString areaCode)
{
	this->areaCode = areaCode;
}
CString PhoneNumber::GetLocalNumber()
{
	return this->localNumber;
}
void PhoneNumber::SetLocalNumber(CString localNumber)
{
	this->localNumber = localNumber;
}
CString PhoneNumber::GetExtension()
{
	return this->extension;
}
void PhoneNumber::SetExtension(CString extension)
{
	this->extension = extension;
}
