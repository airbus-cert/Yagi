#ifndef __YAGI_IDECOMPILE__
#define __YAGI_IDECOMPILE__

#include <string>

namespace yagi 
{
	struct Compiler {
		enum class Language {
			X86_WINDOWS
		};

		enum class Endianess {
			BE,
			LE
		};

		enum class Mode {
			M32,
			M64
		};

		Language	language;
		Endianess	endianess;
		Mode		mode;

		Compiler(Language language, Endianess endianess, Mode mode)
			: language {language}, endianess{endianess}, mode{mode}
		{}
	};

	class IDecompiler
	{
	public:
		virtual std::string decompile(uint64_t funcAddress) = 0;
		virtual ~IDecompiler() = default;
	};
}

#endif