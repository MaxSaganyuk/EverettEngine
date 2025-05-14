#include "pch.h"
#include "AdString.h"

void AdString::CheckAndReadapt(StrType typeToBeUsed) const
{
	if (currentType != typeToBeUsed)
	{
		typeToBeUsed == StrType::Regular ? ToRegular() : ToWide();
	}
}

void AdString::ToRegular() const
{
	strPair.first = CT2A(strPair.second);
	strPair.second = _T("");
	currentType = StrType::Regular;
}

void AdString::ToWide() const
{
	strPair.second = CA2T(strPair.first.c_str());
	strPair.first = "";
	currentType = StrType::Wide;
}

AdString::AdString()
{
	currentType = StrType::Regular;
}

AdString::AdString(const char* cPtr)
{
	currentType = StrType::Regular;
	strPair.first = cPtr;
}

AdString::AdString(const std::string& str)
{
	currentType = StrType::Regular;
	strPair.first = str;
}

AdString::AdString(const CString& str)
{
	currentType = StrType::Wide;
	strPair.second = str;
}

AdString::AdString(LPCTSTR str)
{
	currentType = StrType::Wide;
	strPair.second = str;
}

bool AdString::empty() const
{
	return currentType == StrType::Regular ? strPair.first.empty() : strPair.second.IsEmpty();
}

template<char>
char AdString::operator[](size_t index) const
{
	return strPair.first[index];
}

template<wchar_t>
wchar_t AdString::operator[](size_t index) const
{
	return strPair.second[index];
}

void AdString::operator=(const std::string& str)
{
	CheckAndReadapt(StrType::Regular);

	strPair.first = str;
}

void AdString::operator=(const CString& str)
{
	CheckAndReadapt(StrType::Wide);

	strPair.second = str;
}

bool AdString::operator==(const std::string & str) const
{
	CheckAndReadapt(StrType::Regular);

	return strPair.first == str;
}

bool AdString::operator==(const char* cPtr) const
{
	CheckAndReadapt(StrType::Regular);

	return strPair.first == cPtr;
}

bool AdString::operator==(const CString& str) const
{
	CheckAndReadapt(StrType::Wide);

	return strPair.second == str;
}

bool AdString::operator==(LPCTSTR cPtr) const
{
	CheckAndReadapt(StrType::Wide);

	return strPair.second == cPtr;
}

bool AdString::operator==(const AdString& str) const
{
	CheckAndReadapt(str.currentType);

	return str.currentType == StrType::Regular ? strPair.first == str.strPair.first : strPair.second == str.strPair.second;
}

bool AdString::operator!=(const std::string& str) const
{
	return !(*this == str);
}

bool AdString::operator!=(const char* cPtr) const
{
	return !(*this == cPtr);
}

bool AdString::operator!=(const CString& str) const
{
	return !(*this == str);
}

bool AdString::operator!=(LPCTSTR cPtr) const
{
	return !(*this == cPtr);
}

bool AdString::operator!=(const AdString& str) const
{
	return !(*this == str);
}

bool AdString::operator<(const AdString& str) const
{
	CheckAndReadapt(str.currentType);

	return str.currentType == StrType::Regular ? strPair.first < str.strPair.first : strPair.second < str.strPair.second;
}

std::string AdString::operator+(const std::string& str) const
{
	CheckAndReadapt(StrType::Regular);

	return strPair.first + str;
}

std::string AdString::operator+(const char* cStr) const
{
	CheckAndReadapt(StrType::Regular);

	return strPair.first + cStr;
}

std::string AdString::operator+(char c) const
{
	CheckAndReadapt(StrType::Regular);

	return strPair.first + c;
}

CString AdString::operator+(wchar_t c) const
{
	CheckAndReadapt(StrType::Wide);

	return strPair.second + c;
}

CString AdString::operator+(const CString& str) const
{
	CheckAndReadapt(StrType::Wide);

	return strPair.second + str;
}

CString AdString::operator+(LPCTSTR str) const
{
	CheckAndReadapt(StrType::Wide);

	return strPair.second + str;
}

AdString AdString::operator+(const AdString& str) const
{
	CheckAndReadapt(str.currentType);

	return str.currentType == StrType::Regular ?
		AdString(strPair.first + str.strPair.first) :
		AdString(strPair.second + str.strPair.second);
}

AdString& AdString::operator+=(char c)
{
	CheckAndReadapt(StrType::Regular);

	strPair.first += c;

	return *this;
}

AdString& AdString::operator+=(const std::string& str)
{
	CheckAndReadapt(StrType::Regular);

	strPair.first += str;

	return *this;
}

AdString& AdString::operator+=(wchar_t c)
{
	CheckAndReadapt(StrType::Wide);

	strPair.second += c;

	return *this;
}

AdString& AdString::operator+=(const CString& str)
{
	CheckAndReadapt(StrType::Wide);

	strPair.second += str;

	return *this;
}

AdString::operator std::string() const
{
	CheckAndReadapt(StrType::Regular);

	return strPair.first;
}

AdString::operator CString() const
{
	CheckAndReadapt(StrType::Wide);

	return strPair.second;
}

AdString::operator std::string& ()
{
	CheckAndReadapt(StrType::Regular);

	return strPair.first;
}

AdString::operator LPCTSTR() const
{
	CheckAndReadapt(StrType::Wide);

	return strPair.second;
}

AdString::operator CString& ()
{
	CheckAndReadapt(StrType::Wide);

	return strPair.second;
}

bool operator==(const std::string& str1, const AdString& str2)
{
	str2.CheckAndReadapt(AdString::StrType::Regular);

	return str1 == str2.strPair.first;
}

bool operator==(const CString& str1, const AdString& str2)
{
	str2.CheckAndReadapt(AdString::StrType::Wide);

	return str1 == str2.strPair.second;
}

bool operator!=(const std::string& str1, const AdString& str2)
{
	return !(str2 == str1);
}

bool operator!=(const CString& str1, const AdString& str2)
{
	return !(str2 == str1);
}

std::string operator+(const std::string& str1, const AdString& str2)
{
	str2.CheckAndReadapt(AdString::StrType::Regular);

	return str1 + str2.strPair.first;
}

CString operator+(const CString& str1, const AdString& str2)
{
	str2.CheckAndReadapt(AdString::StrType::Wide);

	return str1 + str2.strPair.second;
}