#ifndef __YAGI_ILOGGER__
#define __YAGI_ILOGGER__

#include <string>

namespace yagi
{
	class ILogger
	{
	public:
		virtual void print(const std::string& message) = 0;

		template<typename... Params>
		void error(const std::string& message, Params... parameters)
		{
			std::stringstream ss;
			ss << "[ERROR] : ";
			for (auto& val : { message, parameters ... })
			{
				ss << " " << val;
			}

			ss << std::endl;
			
			print(ss.str());
		}
	};
}
#endif