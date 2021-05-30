#ifndef __YAGI_MOCK_LOGGER_TEST__
#define __YAGI_MOCK_LOGGER_TEST__

#include "logger.hh"
#include <functional>
#include <string>

using MockLoggerCallback = std::function<void(const std::string&)>;

class MockLogger : public yagi::Logger
{
protected:
	MockLoggerCallback m_callback;
public:
	explicit MockLogger(MockLoggerCallback callback)
		: m_callback{ callback }
	{}
	void print(const std::string& message) override
	{
		m_callback(message);
	}
};

#endif
