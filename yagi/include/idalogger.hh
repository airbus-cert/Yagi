#ifndef __YAGI_IDALOG__
#define __YAGI_IDALOG__

#include "logger.hh"

namespace yagi 
{
	/*!
	 * \brief	The IDA implementation of the Yagi logger
	 */
	class IdaLogger : public Logger
	{
	public:
		/*!
		 * \brief	Core print for IDA
		 * \param	message	message to print on console
		 */
		void print(const std::string& message) override;
	};
}

#endif