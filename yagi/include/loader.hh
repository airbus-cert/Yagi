#ifndef __YAGI_LOADER__
#define __YAGI_LOADER__

class LoadImage;

namespace yagi 
{
	/*!
	 * \brief	Factory pattern
	 *			Try to split ghidra world to the rest of the worl
	 */
	class LoaderFactory
	{
	public:
		virtual ~LoaderFactory() = default;
		/*!
		 * \brief	build method
		 * \return	Compatibility with LoadImage interface
		 */
		virtual LoadImage* build() = 0;
	};


	/*!
	 * \brief	Template class just to easily declare factory type
	 */
	template<typename T>
	class LoaderFactoryDefault : public LoaderFactory
	{
	public:
		/*!
		 * \brief	build factory using default constructor
		 */
		LoadImage* build() override
		{
			return new T();
		}
	};
}

#endif