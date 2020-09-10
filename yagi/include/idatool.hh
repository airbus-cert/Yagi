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
		template<typename T, typename R, typename U>
		void transform(const qvector<T>& origin, U it, std::function<R(const T&)> callback)
		{

			for (auto i = 0; i < origin.size(); i++)
			{
				*it = callback(origin.at(i));
			}
		}
	}
}

#endif