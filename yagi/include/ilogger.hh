#ifndef __YAGI_ILOGGER__
#define __YAGI_ILOGGER__

#include <string>

namespace yagi
{
	class ILogger
	{
	private:
		template<typename... Params>
		void format(const std::string& level, const std::string& message, Params... parameters)
		{
			std::stringstream ss;
			ss << "[Yagi] " << level << " : ";
			for (auto& val : { message, parameters ... })
			{
				ss << " " << val;
			}

			ss << std::endl;

			print(ss.str());
		}

		virtual void print(const std::string& message) = 0;

	public:
		

		template<typename... Params>
		void error(const std::string& message, Params... parameters) {
			this->format("ERROR", message, parameters...);
		}
		template<typename... Params>
		void info(const std::string& message, Params... parameters) {
			this->format("INFO", message, parameters...);
		}
	};
}
#endif