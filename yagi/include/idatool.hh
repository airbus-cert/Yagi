#ifndef __YAGI_IDATOOL__
#define __YAGI_IDATOOL__

#include <idp.hpp>
#include <vector>
#include <functional>
#include <optional>
#include <iterator>

namespace yagi 
{
	namespace idatool
	{
		/*!
		 * \brief	Tool function use to create a transform function
		 *			Applicable to a qvector
		 * \param	origin	Original qvector to transform
		 * \param	it	std iterator
		 * \param	callback	callback use to transform from qvector to std vector
		 */
		template<typename T, typename R, typename U>
		void transform(const qvector<T>& origin, U it, std::function<R(const T&)> callback)
		{
			for (size_t i = 0; i < origin.size(); i++)
			{
				*it = callback(origin.at(i));
			}
		}
	}
}

#endif