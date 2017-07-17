#ifndef NANOSOFT_BASE64_H
#define NANOSOFT_BASE64_H

#include <string>

namespace nanosoft
{
	
	std::string base64_encode(unsigned char const* , unsigned int len);
	std::string base64_encode(const std::string &bytes);
	std::string base64_decode(std::string const& s);
	
}

#endif // NANOSOFT_BASE64_H
