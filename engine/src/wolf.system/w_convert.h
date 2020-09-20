/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (https://PooyaEimandar.github.io) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/WolfEngine/Wolf.Engine/issues
	Website			 : https://WolfEngine.App
	Name			 : w_convert.h
	Description		 : Helper functions for converting types to each others
	Comment			 :
*/

#pragma once

#include <stdlib.h>
#include <vector>
#include <codecvt>
#include <string.h>

#if	defined(__WIN32) || defined(__UWP)

#include <AtlConv.h>
#include <string.h>
#include <sstream>

#elif defined(__ANDROID) || defined(__linux) || defined(__APPLE__)

#include "w_std.h"

#endif

#include "base64/chromiumbase64.h"
#include "base64/fastavx512bwbase64.h"
#include "base64/fastavxbase64.h"
#include "base64/klompavxbase64.h"
#include "base64/quicktimebase64.h"
#include "base64/scalarbase64.h"

#ifdef __linux
#include "base64/linuxbase64.h"
#endif

namespace wolf::system::convert
{
#if	defined(__WIN32) || defined(__UWP)

	inline W_RESULT chars_to_GUID(const std::wstring & pStr, GUID & pGUID)
	{
		LPOLESTR guid = W2OLE((wchar_t*)pStr.c_str());
		auto hr = W_RESULT::W_PASSED;
		CLSIDFromString(guid, (LPCLSID)& pGUID);
		return hr;
	}

	inline void ANSI_to_UNICODE(_In_z_ const std::string& pANSI, _In_z_ std::wstring& pResult, _In_ const int pSize = 1024)
	{
		LPSTR _str = const_cast<LPSTR>(pANSI.c_str());
		auto _w = new WCHAR[pSize];
		std::memset(_w, 0, pSize);

		MultiByteToWideChar(CP_ACP, 0, &_str[0], -1, _w, pSize); // ANSI to UNICODE

		pResult = _w;

		delete[] _w;
	}

	//inline void ANSI_to_UTF8(_In_z_ const std::string& pANSI, _In_z_ std::wstring& pResult, _In_ const int pSize = 1024)
	//{
	//	LPSTR _str = const_cast<LPSTR>(pANSI.c_str());
	//	auto _w = new WCHAR[pSize];
	//	std::memset(_w, 0, pSize);

	//	MultiByteToWideChar(CP_ACP, 0, &_str[0], -1, _w, pSize); // ANSI to UNICODE
	//	WideCharToMultiByte(CP_UTF8, 0, _w, -1, &_str[0], pSize, 0, 0); // UNICODE to UTF-8

	//	pResult = _w;

	//	delete[] _w;
	//}

#endif //__WIN32 || __UWP

//#if __cplusplus < 201703L

			// convert UTF-8 string to wstring
	inline std::wstring from_utf8(const std::string& pStr)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> _convert;
		return _convert.from_bytes(pStr);
	}

	// convert UTF-16 string to wstring
	inline std::wstring from_utf16(const std::string& pStr)
	{
		std::wstring_convert<std::codecvt_utf16<wchar_t>> _convert;
		return _convert.from_bytes(pStr);
	}

	// convert wstring to UTF-8 string
	inline std::string to_utf8(const std::wstring& pStr)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> _convert;
		return _convert.to_bytes(pStr);
	}

	// convert wstring to UTF-16 string
	inline std::string to_utf16(const std::wstring& pStr)
	{
		std::wstring_convert<std::codecvt_utf16<wchar_t>> _convert;
		return _convert.to_bytes(pStr);
	}
	//#endif
	inline int to_hex(const std::string& pStr)
	{
		std::stringstream _ss;
		_ss << pStr;
		int _hex;
		_ss >> std::hex >> _hex;
		_ss.clear();

		return _hex;
	}

	inline std::wstring chars_to_wstring(char* pValue)
	{
		std::string _str(pValue);
		return std::wstring(_str.begin(), _str.end());
	}

	inline std::wstring string_to_wstring(std::string pValue)
	{
		return std::wstring(pValue.begin(), pValue.end());
	}

	inline std::string wstring_to_string(std::wstring pValue)
	{
		return std::string(pValue.begin(), pValue.end());
	}

	inline std::wstring tchars_to_wstring(wchar_t* pValue, const size_t pLength)
	{
		auto _w = new wchar_t[pLength];
		for (size_t i = 0; i < pLength; ++i)
		{
			_w[i] = pValue[i];
		}
		auto _str = std::wstring(&_w[0]);

		delete[] _w;
		return _str;
	}

	inline std::string tchars_to_string(wchar_t* pValue, const size_t pLength)
	{
		auto wstr = tchars_to_wstring(pValue, pLength);
		return wstring_to_string(wstr);
	}

	//copy string to const char*
	inline char* copy_string_to_const_char_ptr(_In_z_ const std::string& pStr)
	{
		auto _size = pStr.size();
		auto _ptr = (char*)malloc((_size + 1) * sizeof(char));
		memcpy(_ptr, &pStr[0], _size * sizeof(char));
		_ptr[_size] = '\0';
		return _ptr;
	}

	inline bool has_string_start_with(_In_z_ const std::string& pString, _In_z_ const std::string pStartWith)
	{
		return strncmp(pStartWith.c_str(), pString.c_str(), pStartWith.size()) == 0;
	}

	inline bool has_wstring_start_with(_In_z_ const std::wstring& pString, _In_z_ const std::wstring pStartWith)
	{
		return wcsncmp(pStartWith.c_str(), pString.c_str(), pStartWith.size()) == 0;
	}

	inline bool has_string_end_with(_In_z_ std::string const& pStr, _In_z_ std::string const& pEnding)
	{
		if (pStr.length() >= pEnding.length())
		{
			return (0 == pStr.compare(pStr.length() - pEnding.length(), pEnding.length(), pEnding));
		}
		return false;
	}

	inline bool has_wstring_end_with(_In_z_ std::wstring const& pStr, _In_z_ std::wstring const& pEnding)
	{
		if (pStr.length() >= pEnding.length())
		{
			return (0 == pStr.compare(pStr.length() - pEnding.length(), pEnding.length(), pEnding));
		}
		return false;
	}

#pragma region sub string and convert functions

	template<class T >
	auto substr_function(const std::string& pStr, size_t pOffset, size_t pCount, _Inout_ std::vector<T>& pResult) -> typename std::enable_if<std::is_integral<T>::value, void>::type
	{
		if (std::is_same<T, int>::value || std::is_same<T, unsigned int>::value)
		{
			pResult.push_back(static_cast<int>(std::atoi(pStr.substr(pOffset, pCount).c_str())));
		}
		else if (std::is_same<T, short>::value || std::is_same<T, unsigned short>::value)
		{
			pResult.push_back(static_cast<short>(std::atoi(pStr.substr(pOffset, pCount).c_str())));
		}
		else if (std::is_same<T, long>::value || std::is_same < T, unsigned long> ::value)
		{
			pResult.push_back(std::atol(pStr.substr(pOffset, pCount).c_str()));
		}
#ifdef __WIN32
		else if (std::is_same<T, long long>::value || std::is_same < T, unsigned long long> ::value)
		{
			pResult.push_back(std::atoll(pStr.substr(pOffset, pCount).c_str()));
		}
#endif
		return;
	}

	template<class T >
	auto substr_function(const std::string& pStr, size_t pOffset, size_t pCount, _Inout_ std::vector<T>& pResult) -> typename std::enable_if<std::is_floating_point<T>::value, void>::type
	{
		pResult.push_back(std::atof(pStr.substr(pOffset, pCount).c_str()));
		return;
	}

	template<class T >
	auto substr_function(const std::string& pStr, size_t pOffset, size_t pCount, _Inout_ std::vector<T>& pResult) -> typename std::enable_if<std::is_same<T, std::string>::value, void>::type
	{
		pResult.push_back(pStr.substr(pOffset, pCount));
		return;
	}

	template<class T>
	inline void split_string_then_convert_to(const std::string& pStr, const std::string& pSplit, _Inout_ std::vector<T>& pResult)
	{
		using namespace std;

		size_t _last = 0, _next = 0;
		const auto _size = pSplit.size();
		while ((_next = pStr.find(pSplit, _last)) != string::npos)
		{
			substr_function(pStr, _last, _next - _last, pResult);
			_last = _next + _size;
		}

		//still we have string, add the finals
		auto _str_size = pStr.size();
		if (_last < _str_size)
		{
			substr_function(pStr, _last, _next - _last, pResult);
		}
	}

	inline void split_string(const std::string& pStr, const std::string& pSplit, _Inout_ std::vector<std::string>& pResult)
	{
		split_string_then_convert_to<std::string>(pStr, pSplit, pResult);
	}

	template<class T>
	inline void find_all_numbers_then_convert_to(const std::string& pStr, _Inout_ std::vector<T>& pResult)
	{
		using namespace std;

		std::string _number;
		for (size_t i = 0; i < pStr.size(); ++i)
		{
			if ((pStr[i] >= '0' && pStr[i] <= '9') || pStr[i] == '.' || pStr[i] == '+' || pStr[i] == '-' || pStr[i] == 'e')
			{
				_number += pStr[i];
			}
			else
			{
				if (_number.size())
				{
					substr_function(_number, 0, _number.size(), pResult);
					_number.clear();
				}
				continue;
			}
		}

		if (_number.size())
		{
			substr_function(_number, 0, _number.size(), pResult);
			_number.clear();
		}
	}

#pragma endregion

#pragma region sub wstring convert functions

#ifdef __WIN32
	template<class T >
	auto subwstr_function(const std::wstring& pStr, size_t pOffset, size_t pCount, _Inout_ std::vector<T>& pResult) -> typename std::enable_if<std::is_integral<T>::value, void>::type
	{
		if (std::is_same<T, int>::value)
		{
			pResult.push_back(_wtoi(pStr.substr(pOffset, pCount).c_str()));
		}
		else if (std::is_same<T, short>::value)
		{
			pResult.push_back(static_cast<short>(_wtoi(pStr.substr(pOffset, pCount).c_str())));
		}
		else if (std::is_same<T, long>::value)
		{
			pResult.push_back(_wtol(pStr.substr(pOffset, pCount).c_str()));
		}

		else if (std::is_same<T, long long>::value)
		{
			pResult.push_back(_wtoll(pStr.substr(pOffset, pCount).c_str()));
		}
		return;
	}

	template<class T >
	auto subwstr_function(const std::wstring& pStr, size_t pOffset, size_t pCount, _Inout_ std::vector<T>& pResult) -> typename std::enable_if<std::is_floating_point<T>::value, void>::type
	{
		pResult.push_back(_wtof(pStr.substr(pOffset, pCount).c_str()));
		return;
	}

	template<class T >
	auto subwstr_function(const std::wstring& pStr, size_t pOffset, size_t pCount, _Inout_ std::vector<T>& pResult) -> typename std::enable_if<std::is_same<T, std::wstring>::value, void>::type
	{
		pResult.push_back(pStr.substr(pOffset, pCount));
		return;
	}

	template<class T>
	inline void split_wstring_then_convert_to(const std::wstring& pStr, const std::wstring& pSplit, _Inout_ std::vector<T>& pResult)
	{
		using namespace std;

		size_t _last = 0, _next = 0;
		const auto _size = pSplit.size();
		while ((_next = pStr.find(pSplit, _last)) != wstring::npos)
		{
			subwstr_function(pStr, _last, _next - _last, pResult);
			_last = _next + _size;
		}

		//still we have string, add the finals
		auto _str_size = pStr.size();
		if (_last < _str_size)
		{
			subwstr_function(pStr, _last, _next - _last, pResult);
		}
	}

	inline void split_wstring(const std::wstring& pStr, const std::wstring& pSplit, _Inout_ std::vector<std::wstring>& pResult)
	{
		split_wstring_then_convert_to<std::wstring>(pStr, pSplit, pResult);
	}

	template<class T>
	inline void find_all_numbers_then_convert_toW(const std::wstring& pStr, _Inout_ std::vector<T>& pResult)
	{
		using namespace std;

		std::string _number;
		for (size_t i = 0; i < pStr.size(); ++i)
		{
			if ((pStr[i] >= L'0' && pStr[i] <= L'9') || pStr[i] == L'.' || pStr[i] == L'+' || pStr[i] == L'-')
			{
				_number += pStr[i];
			}
			else
			{
				if (_number.size())
				{
					substr_function(_number, 0, _number.size(), pResult);
					_number.clear();
				}
				continue;
			}
		}

		if (_number.size())
		{
			substr_function(_number, 0, _number.size(), pResult);
			_number.clear();
		}
	}

#endif

#pragma endregion

#pragma region base64

	enum base_64_mode { chromium, klomp_avx, fast_avx, fast_avx512, quick_time, scalar };
	inline size_t to_base_64(
		_Inout_z_ char* pDestinationBuffer,
		_In_z_ char* pSourceBuffer,
		_In_z_ const size_t& pSourceBufferLenght,
		_In_ const base_64_mode& pEncodeMode)
	{
		size_t _encoded_size = 0;
		switch (pEncodeMode)
		{
		case chromium:
			_encoded_size = chromium_base64_encode(
				pDestinationBuffer,
				pSourceBuffer,
				pSourceBufferLenght);
			break;
		case klomp_avx:
			klomp_avx2_base64_encode(
				pSourceBuffer,
				pSourceBufferLenght,
				pDestinationBuffer,
				&_encoded_size);
			break;
		case fast_avx:
			_encoded_size = fast_avx2_base64_encode(
				pDestinationBuffer,
				pSourceBuffer,
				pSourceBufferLenght);
			break;
#if USE_AVX512 != 0 && ((defined(_MSC_VER) && _MSC_VER >= 1911) || (defined(__INTEL_COMPILER) && __INTEL_COMPILER >= 1600) || (defined(__clang__) && __clang_major__ >= 4) || (defined(__GNUC__) && __GNUC__ >= 5))
		case fast_avx512:
			_encoded_size = fast_avx512bw_base64_encode(
				pDestinationBuffer,
				pSourceBuffer,
				pSourceBufferLenght);
			break;
#endif
		case quick_time:
			_encoded_size = static_cast<size_t>(quicktime_base64_encode(
				pDestinationBuffer,
				pSourceBuffer,
				(int)pSourceBufferLenght));
			break;
		case scalar:
			scalar_base64_encode(
				pSourceBuffer,
				pSourceBufferLenght,
				pDestinationBuffer,
				&_encoded_size);
			break;
		}
		return _encoded_size;
	}

#pragma endregion
}