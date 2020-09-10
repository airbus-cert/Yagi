#ifndef __YAGI_DECOMPILER__
#define __YAGI_DECOMPILER__

#include <memory>
#include <optional>

#include "idecompile.hh"
#include "typeinfo.hh"
#include "symbolfactory.hh"
#include "ilogger.hh"

namespace yagi 
{
	class YagiArchitecture;

	class GhidraDecompiler : public IDecompiler
	{
	private:
		std::unique_ptr<YagiArchitecture> m_architecture;

	public:
		explicit GhidraDecompiler(std::unique_ptr<YagiArchitecture> architecture);

		GhidraDecompiler(const GhidraDecompiler&) = delete;
		GhidraDecompiler& operator=(const GhidraDecompiler&) = delete;

		GhidraDecompiler(GhidraDecompiler&&) noexcept = default;
		GhidraDecompiler& operator=(GhidraDecompiler&&) noexcept = default;

		std::string decompile(uint64_t funcAddress) override;
		virtual ~GhidraDecompiler() = default;

		static std::optional<std::unique_ptr<IDecompiler>> build(
			std::unique_ptr<ILogger> logger, 
			std::shared_ptr<SymbolFactory> symbolDatabase, 
			std::unique_ptr<TypeInfoFactory> typeDatabase
		) noexcept;
	};
}

#endif