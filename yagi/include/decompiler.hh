#ifndef __YAGI_IDECOMPILE__
#define __YAGI_IDECOMPILE__

#include <string>
#include <map>
#include <optional>
#include <vector>

namespace yagi 
{
	/*!
	 * \brief Memory location
	 */
	struct MemoryLocation
	{

		/*!
		 * \brief	type of memory 
		 */
		std::string spaceName;

		/*!
		 * \brief	offset in memory type range 
		 */
		uint64_t offset;

		/*!
		 *	\brief	Size of address 
		 */
		uint32_t addrSize;

		/*!
		 * \brief	pcode address definition
		 */
		std::vector<uint64_t> pc;


		MemoryLocation(const std::string& spaceName, uint64_t offset, uint32_t addrSize)
			: spaceName{ spaceName }, offset{ offset }, addrSize{ addrSize }
		{}
	};

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
			PPC,			// PowerPC
			MIPS,			// MIPS
			SPARC,			// Sparc
			ATMEL,			// ATMEL
			P6502,			// 6502
			Z80,			// 8085 Z80
			eBPF			// eBpf loader from https://github.com/cylance/eBPF_processor
		};

		enum class Endianess {
			BE,				// Big Endian
			LE				// Little Endian
		};

		enum class Mode {
			M16,			// 16 bits
			M24,			// 24 bits
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

		/*!
		 * \brief	result of the decompiler 
		 */
		struct Result
		{
			/*!
			 * \brief	raw code formated with correct synthax
			 */
			std::string	cCode;

			/*!
			 * \brief	name of the function
			 */
			std::string name;

			/*!
			 * \brief	address of function
			 */
			uint64_t ea;

			/*!
			 * \brief	a mapping between token symbol and address
			 */
			std::map<std::string, MemoryLocation> symbolAddress;

			/*!
			 * \brief	ctor
			 */
			Result(std::string name, uint64_t ea, std::string cCode, std::map<std::string, MemoryLocation> symbolAddress)
				: name{ name }, ea { ea }, cCode{ cCode }, symbolAddress{ symbolAddress }
			{}
		};

		virtual ~Decompiler() = default;

		/*!
		 * \brief	decompiler function interface
		 * \param	funcAddress	address of the function to decompile
		 * \return	decompiled source code
		 */
		virtual std::optional<Result> decompile(uint64_t funcAddress) = 0;
	};
}

#endif