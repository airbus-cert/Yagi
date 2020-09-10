#ifndef __YAGI_PLUGIN__
#define __YAGI_PLUGIN__

#include <idp.hpp>
#include <memory>
#include <sstream>
#include "idecompile.hh"

namespace yagi {

	class Plugin : public plugmod_t {
	protected:
		std::unique_ptr<IDecompiler> m_decompiler;
	public:
		explicit Plugin(std::unique_ptr<IDecompiler> decompiler);

		Plugin(const Plugin&) = delete;
		Plugin& operator=(const Plugin&) = delete;

		Plugin(Plugin&&) noexcept = default;
		Plugin& operator=(Plugin&&) noexcept = default;

		virtual ~Plugin() = default;

		virtual bool idaapi run(size_t) override;

		void view(const std::string& name, const std::string& code) const;
	};
}

#endif