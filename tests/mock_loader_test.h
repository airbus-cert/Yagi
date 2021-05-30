#ifndef __YAGI_MOCK_LOADER_TEST__
#define __YAGI_MOCK_LOADER_TEST__

#include "loader.hh"
#include "libdecomp.hh"
#include <functional>
#include <string>

using MockLoaderCallback = std::function<void(uint1*, int4, const Address&)>;

class MockLoader : public LoadImage
{
protected:
	MockLoaderCallback m_callback;

public:

	explicit MockLoader(MockLoaderCallback callback)
		: LoadImage("mock"), m_callback{ callback }
	{}

	std::string getArchType(void) const override
	{
		return "test";
	}

	void loadFill(uint1* ptr, int4 size, const Address& addr) override
	{
		m_callback(ptr, size, addr);
	}

	void adjustVma(long adjust)
	{

	}
};

class MockLoaderFactory : public yagi::LoaderFactory
{
protected:
	MockLoaderCallback m_callback;
public:
	explicit MockLoaderFactory(MockLoaderCallback callback)
		: m_callback{ callback }
	{}

	LoadImage* build() override
	{
		return new MockLoader(m_callback);
	}
};

#endif
