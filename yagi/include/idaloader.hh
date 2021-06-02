#ifndef __YAGI_IDALOADER__
#define __YAGI_IDALOADER__

#include "loader.hh"
#include <libdecomp.hh>

namespace yagi 
{
	/*!
	 * \brief	Implement the LoadImage interface of Ghidra
	 */
	class IdaLoader : public LoadImage
	{
	public:
		/*!
		 * \brief	constructor
		 */
		explicit IdaLoader();

		/*!
		 *	\brief	Copy is authorized because we only use copyable type
		 */
		IdaLoader(const IdaLoader&) = default;
		IdaLoader& operator=(const IdaLoader&) = default;

		/*!
		 *	\brief	moving is allowed
		 */
		IdaLoader(IdaLoader&&) noexcept = default;
		IdaLoader& operator=(IdaLoader&&) noexcept = default;

		/*!
		 * \brief	Return the name of the IDA arch (actually it's IDA)s
		 * \return	name of the current arch
		 */
		std::string getArchType(void) const override;

		/*!
		 * \brief	Load data of the current file using IDA API
		 * \param	ptr	buffer pointer
		 * \param	size	size of expected data
		 * \param	addr	address of the payload
		 */
		void loadFill(uint1* ptr, int4 size, const Address& addr) override;

		/*!
		 * \brief	Adjust VMA
		 * \param	adjust
		 */
		void adjustVma(long adjust);
	};

	/*!
	 * \brief	The factory interface
	 */
	using IdaLoaderFactory = LoaderFactoryDefault<IdaLoader>;
}

#endif