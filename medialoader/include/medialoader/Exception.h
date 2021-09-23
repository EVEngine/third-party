#pragma once

#include <exception>
#include <cstdarg> // vararg
#include <cstdio> // vsnprintf
#include <cstring> // strncpy
#include <string>

namespace medialoader
{

/**
 * A convenient vararg-enabled exception class.
 **/
class Exception : public std::exception
{
public:

	/**
	 * Creates a new Exception according to printf-rules.
	 *
	 * See: http://www.cplusplus.com/reference/clibrary/cstdio/printf/
	 *
	 * @param fmt The format string (see printf).
	 **/
	Exception(const char *fmt, ...);
	virtual ~Exception() throw();

	/**
	 * Returns a string containing reason for the exception.
	 * @return A description of the exception.
	 **/
	inline virtual const char *what() const throw()
	{
		return message.c_str();
	}

private:

	std::string message;

}; // Exception

} // medialoader

