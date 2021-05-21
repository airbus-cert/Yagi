#ifndef __YAGI_BASE__
#define __YAGI_BASE__

#include <vector>
#include <string>

namespace yagi 
{
	/*!
	 *	\brief Convert an address into hex string 
	 *	\param	addr	address to convert
	 *	\return	hex string
	 */
	std::string to_hex(uint64_t addr);

	/*!
	 *	\brief	Split any string using the delimiter parameter
	 *	\param	s	string to split
	 *	\param	delimiter	char use as delimiter of the string
	 */
	std::vector<std::string> split(const std::string& s, char delimiter);
}

#endif