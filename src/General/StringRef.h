/*
 * StringRef.h
 *
 *  Created on: 10 Jan 2016
 *      Author: David
 */

#ifndef STRINGREF_H_
#define STRINGREF_H_

#include <cstddef>	// for size_t
#include <cstdarg>	// for va_args
#include <cstring>	// for strlen

#include "Strnlen.h"
#include "StringFunctions.h"

// Class to describe a string buffer, including its length. This saves passing buffer lengths around everywhere.
class StringRef
{
	char *p;				// pointer to the storage
	size_t len;				// number of characters in the storage, must be at least 1
public:
	StringRef(char *pp, size_t pl) noexcept : p(pp), len(pl) { }

	size_t Capacity() const noexcept { return len - 1; }
	size_t strlen() const noexcept;
	bool IsEmpty() const noexcept { return p[0] == 0; }

	const char *c_str() const noexcept { return p; }
	char *Pointer() const noexcept { return p; }						// use Pointer() only in the very care case that we need direct write access to the storage!

	char& operator[](size_t index) const noexcept { return p[index]; }

	void Clear() const noexcept { p[0] = 0; }

	int printf(const char *fmt, ...) const noexcept __attribute__ ((format (printf, 2, 3)));
	int vprintf(const char *fmt, va_list vargs) const noexcept;
	int catf(const char *fmt, ...) const noexcept __attribute__ ((format (printf, 2, 3)));
	int lcatf(const char *fmt, ...) const noexcept __attribute__ ((format (printf, 2, 3)));
	int vcatf(const char *fmt, va_list vargs) const noexcept;
	bool copy(const char* src) const noexcept;						// returns true if buffer is too small
	bool copy(const char *src, size_t maxlen) const noexcept;		// returns true if buffer is too small
	bool cat(const char *src) const noexcept;						// returns true if buffer is too small
	bool lcat(const char *src) const noexcept;						// returns true if buffer is too small
	bool catn(const char *src, size_t n) const noexcept;			// returns true if buffer is too small
	bool lcatn(const char *src, size_t n) const noexcept;			// returns true if buffer is too small
	bool cat(char c) const noexcept;								// returns true if buffer is too small
	size_t StripTrailingSpaces() const noexcept;
	bool Prepend(const char *src) const noexcept;					// returns true if buffer is too small
	void Truncate(size_t pos) const noexcept;
	void Erase(size_t pos, size_t count = 1) const noexcept;
	bool Insert(size_t pos, char c) const noexcept;					// returns true if buffer is too small
	bool Insert(size_t pos, const char *s) const noexcept;			// returns true if buffer is too small
	bool Equals(const char *s) const noexcept { return strcmp(p, s) == 0; }
	bool EqualsIgnoreCase(const char *s) const noexcept { return StringEqualsIgnoreCase(p, s); }
	int Contains(const char *s) const noexcept;
	int Contains(char c) const noexcept;

	bool Skip() noexcept;
	void Backspace() noexcept;
};

// Class to describe a string which we can get a StringRef reference to
template<size_t Len> class String
{
public:
	String() noexcept { storage[0] = 0; }

	StringRef GetRef() noexcept { return StringRef(storage, Len + 1); }
	const char *c_str() const noexcept { return storage; }
	size_t strlen() const noexcept { return Strnlen(storage, Len); }
	bool IsEmpty() const noexcept { return storage[0] == 0; }
	bool IsFull() const noexcept { return strlen() == Len; }
	char& operator[](size_t index) noexcept { return storage[index]; }
	char operator[](size_t index) const noexcept { return storage[index]; }
	constexpr size_t Capacity() const noexcept { return Len; }
	bool EndsWith(char c) const noexcept;

	void Clear() noexcept { storage[0] = 0; }
	int printf(const char *fmt, ...) noexcept __attribute__ ((format (printf, 2, 3)));
	int vprintf(const char *fmt, va_list vargs) noexcept;
	int catf(const char *fmt, ...) noexcept __attribute__ ((format (printf, 2, 3)));
	int vcatf(const char *fmt, va_list vargs) noexcept;
	bool copy(const char *src) noexcept { return GetRef().copy(src); }	// returns true if buffer is too small
	bool copy(const char *src, size_t maxlen) noexcept { return GetRef().copy(src, maxlen); }	// returns true if buffer is too small
	bool cat(const char *src) noexcept { return GetRef().cat(src); }		// returns true if buffer is too small
	bool catn(const char *src, size_t n) noexcept { return GetRef().catn(src, n); }		// returns true if buffer is too small
	bool cat(char c) noexcept { return GetRef().cat(c); }				// returns true if buffer is too small
	bool Prepend(const char *src) noexcept;								// returns true if buffer is too small

	void CopyAndPad(const char *src) noexcept;
	bool ConstantTimeEquals(String<Len> other) const noexcept;

	void Truncate(size_t len) noexcept;
	void Erase(size_t pos, size_t count = 1) noexcept;
	bool Insert(size_t pos, char c) noexcept { return GetRef().Insert(pos, c); }		// returns true if buffer is too small
	bool Insert(size_t pos, const char *s) noexcept { return GetRef().Insert(pos, s); }	// returns true if buffer is too small
	bool Equals(const char *s) const noexcept { return strcmp(storage, s) == 0; }
	bool EqualsIgnoreCase(const char *s) const noexcept { return StringEqualsIgnoreCase(storage, s); }
	int Contains(const char *s) const noexcept;
	int Contains(char c) const noexcept;

	char *Pointer() noexcept { return storage; }							// use this one only exceptionally and with great care!
	void EnsureNullTerminated() noexcept { storage[Len] = 0; }

private:
	char storage[Len + 1];
};

// Copy some text into this string and pad it with nulls so we can do a constant time compare
template<size_t Len> void String<Len>::CopyAndPad(const char* src) noexcept
{
	memset(storage, 0, Len + 1);
	copy(src);
}

// Do a constant time compare. Both this string and the other one much be padded with nulls.
template<size_t Len> bool String<Len>::ConstantTimeEquals(String<Len> other) const noexcept
{
	char rslt = 0;
	for (size_t i = 0; i < Len; ++i)
	{
		rslt |= (storage[i] ^ other.storage[i]);
	}
	return rslt == 0;
}

template<size_t Len> inline int String<Len>::vprintf(const char *fmt, va_list vargs) noexcept
{
	return GetRef().vprintf(fmt, vargs);
}

template<size_t Len> inline int String<Len>::vcatf(const char *fmt, va_list vargs) noexcept
{
	return GetRef().vcatf(fmt, vargs);
}

template<size_t Len> inline bool String<Len>::Prepend(const char *src) noexcept
{
	return GetRef().Prepend(src);
}

template<size_t Len> int String<Len>::printf(const char *fmt, ...) noexcept
{
	va_list vargs;
	va_start(vargs, fmt);
	const int ret = GetRef().vprintf(fmt, vargs);
	va_end(vargs);
	return ret;
}

template<size_t Len> int String<Len>::catf(const char *fmt, ...) noexcept
{
	va_list vargs;
	va_start(vargs, fmt);
	const int ret = GetRef().vcatf(fmt, vargs);
	va_end(vargs);
	return ret;
}

template<size_t Len> void String<Len>::Truncate(size_t len) noexcept
{
	if (len < Len)
	{
		storage[len] = 0;
	}
}

template<size_t Len> void String<Len>::Erase(size_t pos, size_t count) noexcept
{
	const size_t len = strlen();
	if (pos < len)
	{
		while (pos + count < len)
		{
			storage[pos] = storage[pos + count];
			++pos;
		}
		storage[pos] = 0;
	}
}

template<size_t Len> bool String<Len>::EndsWith(char c) const noexcept
{
	const size_t len = strlen();
	return len != 0 && storage[len - 1] == c;
}

template<size_t Len> int String<Len>::Contains(const char *s) const noexcept
{
	const char * const p = strstr(storage, s);
	return (p == nullptr) ? -1 : p - storage;
}

template<size_t Len> int String<Len>::Contains(char c) const noexcept
{
	const char * const p = strchr(storage, c);
	return (p == nullptr) ? -1 : p - storage;
}

#endif /* STRINGREF_H_ */
