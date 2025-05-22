#pragma once

#include "afxdialogex.h"
#include <string>

// Ad(aptive)String
// Amount of operator overloads is quite large
// So functionaliry will be added as needed instead of implemented at once
// Only one version (regular or wide) can be stored at once
// If opposite version is required with an operator, conversion is executed
class AdString
{
	enum class StrType
	{
		Regular,
		Wide
	};

	// Internal state, can be modified even if no changes to strings 
	mutable StrType currentType;
	mutable std::pair<std::string, CString> strPair;

	void CheckAndReadapt(StrType typeToBeUsed) const;
	void ToRegular() const;
	void ToWide() const;

	template<typename CharType>
	CharType operator[](size_t index) const;

public:
	AdString();
	AdString(const char* cPtr);
	AdString(const std::string& str);
	AdString(const CString& str);
	AdString(LPCTSTR str);

	~AdString() = default;

	// std::string-like methods
	bool empty() const;

	template<char>
	char operator[](size_t index) const;

	template<wchar_t>
	wchar_t operator[](size_t index) const;

	void operator=(const std::string& str);
	void operator=(const CString& str);

	bool operator==(const std::string& str) const;
	bool operator==(const char* cPtr) const;
	bool operator==(const CString& str) const;
	bool operator==(LPCTSTR cPtr) const;
	bool operator==(const AdString& str) const;
	friend bool operator==(const std::string& str1, const AdString& str2);
	friend bool operator==(const CString& str1, const AdString& str2);

	bool operator<(const AdString& str) const;

	std::string operator+(char c) const;
	std::string operator+(const std::string& str) const;
	std::string operator+(const char* cStr) const;
	CString operator+(wchar_t c) const;
	CString operator+(const CString& str) const;
	CString operator+(LPCTSTR cStr) const;
	AdString operator+(const AdString& str) const;
	friend std::string operator+(const std::string& str1, const AdString& str2);
	friend CString operator+(const CString& str1, const AdString& str2);

	AdString& operator+=(char c);
	AdString& operator+=(const std::string& str);
	AdString& operator+=(wchar_t c);
	AdString& operator+=(const CString& str);

	// Should be used to copy the string from AdString
	operator std::string() const;
	operator CString() const;
	operator LPCTSTR() const;

	// Should be used to modify the string
	operator std::string&();
	operator CString&();
};