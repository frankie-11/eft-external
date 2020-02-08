#pragma once
#include <string>
#include <utility>
#include <array>


#ifdef _DEBUG

template<typename _chartype, size_t _length>
class XorStr
{
public:
	constexpr  XorStr(const _chartype(&str)[_length]) : XorStr(str, std::make_index_sequence<length>()) {}

	// returns pointer to unencrypted string 
	// caution: invalid if object gets destructed
	operator const _chartype* ()
	{
		return data;
	}

	// length in characters (not including 0-terminator)
	static constexpr size_t length = _length - 1;

private:
	template<size_t... _indices>
	constexpr XorStr(const _chartype(&str)[_length], std::index_sequence<_indices...>) : data{ str[_indices]... }, encrypted(false) {}

	_chartype data[_length];
	bool encrypted;
};

template<typename _chartype, size_t _length>
constexpr auto _xor_(_chartype const (&str)[_length])
{
	return XorStr<_chartype, _length>(str);
}


#else


template<typename _chartype, size_t _length>
class XorStr
{
public:
	constexpr  XorStr(const _chartype(&str)[_length]) : XorStr(str, std::make_index_sequence<_length>()) {}

	~XorStr() { for (size_t t = 0; t < _length; t++) data[t] = 0; }

	// returns pointer to unencrypted string 
	// cation: invalid if object gets destructed
	operator const _chartype* ()
	{
		decrypt();
		return data;
	}

	// length in characters (not including 0-terminator)
	static constexpr size_t length = _length - 1;

private:
	template<size_t... _indices>
	constexpr XorStr(const _chartype(&str)[_length], std::index_sequence<_indices...>) : data{ crypt(str[_indices], _indices)... }, encrypted(true) {}

	static constexpr auto XOR_KEY = static_cast<_chartype>(
		(__TIME__[7] - '0') +
		(__TIME__[6] - '0') * 10 +
		(__TIME__[4] - '0') * 60 +
		(__TIME__[3] - '0') * 600 +
		(__TIME__[1] - '0') * 3600 +
		(__TIME__[0] - '0') * 36000
		);

	static constexpr auto crypt(_chartype c, size_t i)
	{
		return static_cast<_chartype>(c ^ (XOR_KEY + i));
	}

	inline void decrypt()
	{
		if (encrypted)
		{
			for (size_t t = 0; t < _length; t++)
			{
				data[t] = crypt(data[t], t);
			}
			encrypted = false;
		}
	}

	_chartype data[_length];
	bool encrypted;
};


template<typename _chartype, size_t _length>
constexpr auto _xor_(_chartype const (&str)[_length])
{
	return XorStr<_chartype, _length>(str);
}

#endif