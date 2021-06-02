#ifndef __YAGI_IDECOMPILE__
#define __YAGI_IDECOMPILE__

#include <string>

namespace yagi 
{
	/*!
	 * \brief	Compiler configuration definition
	 */
	struct Compiler {
		/*!
		 * \brief	Language definition 
		 */
		enum class Language {
			X86_WINDOWS,	// built using Windows
			X86_GCC,		// built using GCC
			X86,			// unknown compiler
			ARM,			// ARM	arch
			PPC				// PowerPC
		};

		enum class Endianess {
			BE,				// Big Endian
			LE				// Little Endian
		};

		enum class Mode {
			M32,			// 32 bits
			M64				// 64 bits
		};

		Language	language;
		Endianess	endianess;
		Mode		mode;

		/*!
		 * \brief	Constructor
		 */
		Compiler(Language language, Endianess endianess, Mode mode)
			: language {language}, endianess{endianess}, mode{mode}
		{}
	};

	/*!
	 * \brief	Decompile interface
	 */
	class Decompiler
	{
	public:
		virtual ~Decompiler() = default;

		/*!
		 * \brief	decompiler function interface
		 * \param	funcAddress	address of the function to decompile
		 * \return	decompiled source code
		 */
		virtual std::string decompile(uint64_t funcAddress) = 0;
	};
}

#endif