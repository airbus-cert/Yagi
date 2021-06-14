#ifndef __YAGI_ILOGGER__
#define __YAGI_ILOGGER__

#include <string>
#include <sstream>
#include <ostream>

namespace yagi
{
	/*!
	 * \brief	A base class for every logger 
	 */
	class Logger
	{
	private:
		/*!
		 * \brief	Format any message from logger implementation 
		 */
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

		/*!
		 * \brief	Core implementation of the logger
		 *			Must be reimplemented by any subclasses
		 */
		virtual void print(const std::string& message) = 0;

	public:
		
		/*!
		 *	\brief	An error message
		 *			This will be prefixed with the ERROR keyword
		 *  \param	message main message
		 *  \param	parameters	convenient format parameters
		 */
		template<typename... Params>
		void error(const std::string& message, Params... parameters) {
			this->format("ERROR", message, parameters...);
		}

		/*!
		 * \brief	Write an informations log
		 *			This will prefix all message with the INFI prefix
		 */
		template<typename... Params>
		void info(const std::string& message, Params... parameters) {
			this->format("INFO", message, parameters...);
		}
	};
}
#endif
