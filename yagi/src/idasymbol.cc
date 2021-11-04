#include "idasymbol.hh"
#include "base.hh"
#include "exception.hh"
#include "idatool.hh"
#include "idatype.hh"
#include <idp.hpp>
#include <frame.hpp>
#include <struct.hpp>
#include <name.hpp>
#include <sstream>

namespace yagi 
{
	/**********************************************************************/
	std::optional<std::unique_ptr<SymbolInfo>> IdaSymbolInfoFactory::find(uint64_t ea)
	{
		qstring name;
		if (get_name(&name, ea) == 0 || name.size() == 0)
		{
			return std::nullopt;
		}
		return std::make_unique<IdaSymbolInfo>(ea, name.c_str());
	}

	/**********************************************************************/
	std::optional<std::unique_ptr<FunctionSymbolInfo>> IdaSymbolInfoFactory::find_function(uint64_t ea)
	{
		auto idaFunc = get_func(ea);
		if (idaFunc == nullptr)
		{
			return std::nullopt;
		}

		qstring idaName;
		get_short_name(&idaName, idaFunc->start_ea);
		auto beginParameter = idaName.find("(");
		auto functionName = split(idaName.substr(0, beginParameter).c_str(), ' ').back();

		return std::make_unique<IdaFunctionSymbolInfo>(std::make_unique<IdaSymbolInfo>(idaFunc->start_ea, functionName));
	}

	/**********************************************************************/
	IdaSymbolInfo::IdaSymbolInfo(uint64_t ea, std::string name)
		: SymbolInfo(ea, name) 
	{}

	/**********************************************************************/
	bool IdaSymbolInfo::isFunction() const noexcept
	{
		auto idaFunc = get_func(m_ea);
		return idaFunc != nullptr && idaFunc->start_ea == m_ea;
	}

	/**********************************************************************/
	bool IdaSymbolInfo::isImport() const noexcept
	{
		std::string importName = m_name;
		if (importName.length() > 6 && importName.substr(0, 6) == IMPORT_PREFIX)
		{
			importName = importName.substr(6, importName.length() - 6);
		}

		for (uint i = 0; i < get_import_module_qty(); i++)
		{
			auto found = enum_import_names(i,
				[](ea_t ea, const char* name, uval_t ord, void* param) {
					if (name != nullptr && *static_cast<std::string*>(param) == name)
					{
						return 0;
					}
					return 1;
				}, &importName
			);

			if (found != -1 && found != 1)
			{
				return true;
			}
		}
		return false;
	}

	/**********************************************************************/
	bool IdaSymbolInfo::isLabel() const noexcept
	{
		xrefblk_t xr;
		for (bool success = xr.first_to((ea_t)m_ea, XREF_ALL); success; success = xr.next_to()) {
			if (xr.iscode == 0) {
				break;
			}
			if (xr.type != fl_JN) {
				continue;
			}
			return true;
		}

		return false;
	}

	/**********************************************************************/
	bool IdaSymbolInfo::isReadOnly() const noexcept
	{
		auto seg = getseg(m_ea);
		qstring idaName;
		if (get_segm_name(&idaName, seg))
		{
			// assuming that .data segment are read only to improve static analysis
			if (idaName == ".data")
			{
				return true;
			}
		}

		return seg->perm == SEGPERM_READ || seg->perm == SEGPERM_READ + SEGPERM_EXEC;
	}

	/**********************************************************************/
	uint64_t IdaSymbolInfo::getFunctionSize() const
	{
		auto function = get_func(m_ea);
		if (function == nullptr || function->start_ea != m_ea)
		{
			throw SymbolIsNotAFunction(m_name);
		}

		return function->end_ea - function->start_ea;
	}

	/**********************************************************************/
	std::string IdaSymbolInfo::getName() const
	{
		qstring pname;
		if (m_name.substr(0, 4) == "sub_" || !cleanup_name(&pname, m_ea, m_name.c_str()))
		{
			pname = m_name.c_str();
		}

		qstring idaName = demangle_name(pname.c_str(), 0x0EA3BE67);
		if (idaName != "")
		{
			auto pp = idaName.find('(', 0);
			if (pp == qstring::npos)
			{
				pp = idaName.size();
			}

			pname = idaName.substr(0, pp);
		}

		// Mark import symbol with IDA convention
		if (isImport()) {
			return IMPORT_PREFIX + pname.c_str();
		}

		return pname.c_str();
	}

	/**********************************************************************/
	std::optional<std::string> IdaFunctionSymbolInfo::findStackVar(uint64_t offset, uint32_t addrSize)
	{
		auto idaFunc = get_func(m_symbol->getAddress());
		auto frame = get_frame(idaFunc);
		if (frame == nullptr)
		{
			return std::nullopt;
		}

		for (uint32_t i = 0; i < frame->memqty; i++)
		{
			auto member = frame->members[i];
			auto name = std::string(get_struc_name(member.id, STRNFL_REGEX).c_str());
			auto sofset = member.get_soff() - (idaFunc->frsize + idaFunc->frregs);
			if (sofset == offset || (addrSize == 4 && ((uint32_t)sofset == (uint32_t)offset)))
			{
				auto pp = name.find(".");
				if (pp != std::string::npos)
				{
					return name.substr(pp + 1);
				}
				return name;
			}
		}
		return std::nullopt;
	}

	/**********************************************************************/
	std::optional<std::string> IdaFunctionSymbolInfo::findName(uint64_t pc, const std::string& space, uint64_t& offset)
	{
		std::stringstream ss;
		ss << "$ " << to_hex(m_symbol->getAddress()) << ".yagireg." << space << "." << to_hex(pc);
		netnode n(ss.str().c_str(), 0, true);

		qstring res;
		auto size = n.valstr(&res);

		if (res.size() == 0)
		{
			return std::nullopt;
		}

		auto pb = res.find('|');
		if (pb == qstring::npos)
		{
			return std::nullopt;
		}

		std::string name = res.substr(0, pb).c_str();
		offset = std::stoull(res.substr(pb + 1).c_str(), nullptr, 16);

		return name;
	}

	/**********************************************************************/
	void IdaFunctionSymbolInfo::saveName(const MemoryLocation& loc, const std::string& value)
	{
		for (uint64_t pc : loc.pc)
		{
			saveName(loc.offset, loc.spaceName, pc, value);
		}
	}

	/**********************************************************************/
	void IdaFunctionSymbolInfo::saveName(uint64_t address, const std::string& space, uint64_t pc, const std::string& value)
	{
		std::stringstream ss, os;
		ss << "$ " << to_hex(m_symbol->getAddress()) << ".yagireg." << space << "." << to_hex(pc);
		netnode n(ss.str().c_str(), 0, true);

		os << value << "|" << to_hex(address);
		n.set(os.str().c_str());
	}

	/**********************************************************************/
	void IdaFunctionSymbolInfo::saveType(const MemoryLocation& loc, const TypeInfo& newType)
	{
		for (uint64_t pc : loc.pc)
		{
			saveType(loc.offset, loc.spaceName, pc, newType);
		}
	}

	/**********************************************************************/
	void IdaFunctionSymbolInfo::saveType(uint64_t address, const std::string& space, uint64_t pc, const TypeInfo& newType)
	{
		std::stringstream ss, os;
		ss << "$ " << to_hex(m_symbol->getAddress()) << ".yagitype." << space << "." << to_hex(pc);
		netnode n(ss.str().c_str(), 0, true);

		os << newType.getName() << "|" << to_hex(address);
		n.set(os.str().c_str());
	}

	/**********************************************************************/
	bool IdaFunctionSymbolInfo::clearType(const MemoryLocation& loc)
	{
		bool result = false;
		for (uint64_t pc : loc.pc)
		{
			result |= clearType(loc.spaceName, pc);
		}
		return result;
	}

	/**********************************************************************/
	bool IdaFunctionSymbolInfo::clearType(const std::string& space, uint64_t pc)
	{
		std::stringstream ss, os;
		ss << "$ " << to_hex(m_symbol->getAddress()) << ".yagitype." << space << "." << to_hex(pc);
		netnode n(ss.str().c_str(), 0, true);
		return n.delvalue() == 1;
	}

	/**********************************************************************/
	std::optional<std::unique_ptr<TypeInfo>> IdaFunctionSymbolInfo::findType(uint64_t pc, const std::string& from, uint64_t& offset)
	{
		std::stringstream ss;
		ss << "$ " << to_hex(m_symbol->getAddress()) << ".yagitype." << from << "." << to_hex(pc);
		netnode n(ss.str().c_str(), 0, true);

		qstring res;
		auto size = n.valstr(&res);

		if (res.size() == 0)
		{
			return std::nullopt;
		}

		auto pb = res.find('|');
		if (pb == qstring::npos)
		{
			return std::nullopt;
		}

		std::string typeName = res.substr(0, pb).c_str();
		offset = std::stoull(res.substr(pb + 1).c_str(), nullptr, 16);

		return IdaTypeInfoFactory().build_decl(typeName);
	}
} // end of namespace yagi