#ifndef __YAGI_LOADER__
#define __YAGI_LOADER__

class LoadImage;
namespace yagi 
{

	class LoaderFactory
	{
	public:
		virtual LoadImage* build() = 0;
	};


	template<typename T>
	class LoaderFactoryDefault : public LoaderFactory
	{
	public:
		LoadImage* build() override
		{
			return new T();
		}
	};
}

#endif