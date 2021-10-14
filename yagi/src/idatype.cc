#include "idatype.hh"
#include "base.hh"
#include "exception.hh"
#include "idatool.hh"

#include <algorithm>
#include <sstream>


namespace yagi 
{
	/**********************************************************************/
	IdaFuncInfo::IdaFuncInfo(IdaTypeInfo info)
		: m_info(info)
	{
	}

	/**********************************************************************/
	const TypeInfo& IdaFuncInfo::getType() const
	{
		return m_info;
	}

	/**********************************************************************/
	std::vector<std::unique_ptr<TypeInfo>> IdaFuncInfo::getFuncPrototype() const
	{
		std::vector<std::unique_ptr<TypeInfo>> result;
		func_type_data_t funcInfo;
		if (!m_info.m_type.get_func_details(&funcInfo, GTD_CALC_ARGLOCS))
		{
			// not found return empty signature
			return result;
		}

		result.push_back(std::make_unique<IdaTypeInfo>(funcInfo.rettype));

		idatool::transform<funcarg_t, std::unique_ptr<IdaTypeInfo>>(
			funcInfo, std::back_insert_iterator(result),
			[this](const funcarg_t& arg) -> std::unique_ptr<IdaTypeInfo>
			{
				return std::make_unique<IdaTypeInfo>(arg.type);
			}
		);

		return result;
	}

	/**********************************************************************/
	std::vector<std::string> IdaFuncInfo::getFuncParamName() const
	{
		std::vector<std::string> result;
		func_type_data_t funcInfo;
		if (!m_info.m_type.is_func() || !m_info.m_type.get_func_details(&funcInfo))
		{
			return result;
		}

		uint16_t index = 0;
		idatool::transform<funcarg_t, std::string>(
			funcInfo, std::back_insert_iterator(result),
			[&index](const funcarg_t& arg) -> std::string
			{
				index++;
				auto name = std::string(arg.name.c_str());
				if (name.empty())
				{
					std::stringstream ss;
					ss << "a" << index;
					return ss.str();
				}
				else
				{
					return name.c_str();
				}
			}
		);

		return result;
	}

	/**********************************************************************/
	std::string IdaFuncInfo::getCallingConv() const
	{
		func_type_data_t funcInfo;
		if (!m_info.m_type.get_func_details(&funcInfo))
		{
			throw UnknownCallingConvention(m_info.getName());
		}

		switch (funcInfo.get_cc())
		{
		case CM_CC_FASTCALL:
			return "__fastcall";
		case CM_CC_STDCALL:
			return "__stdcall";
		case CM_CC_THISCALL:
			return "__thiscall";
		case CM_CC_CDECL:
		case CM_CC_ELLIPSIS:
			return "__cdecl";
		case CM_CC_INVALID:
		case CM_CC_UNKNOWN:
		default:
			throw UnknownCallingConvention(m_info.getName());
		}
	}

	/**********************************************************************/
	std::string IdaFuncInfo::getName() const
	{
		return m_info.getName();
	}

	/**********************************************************************/
	bool IdaFuncInfo::isDotDotDot() const
	{
		return m_info.m_type.is_vararg_cc();
	}

	/**********************************************************************/
	IdaStructInfo::IdaStructInfo(IdaTypeInfo info)
		: m_info(info)
	{
	}

	/**********************************************************************/
	std::vector<TypeStructField> IdaStructInfo::getFields() const
	{
		udt_type_data_t attributes;
		m_info.m_type.get_udt_details(&attributes);
		std::vector<TypeStructField> result;

		idatool::transform<udt_member_t, TypeStructField>(
			attributes, std::back_insert_iterator(result),
			[this](const udt_member_t& arg) -> TypeStructField
			{
				return TypeStructField{ arg.offset / 8, arg.name.c_str(), std::make_unique<IdaTypeInfo>(arg.type) };
			}
		);

		return result;
	}


	/**********************************************************************/
	const TypeInfo& IdaStructInfo::getType() const
	{
		return m_info;
	}

	/**********************************************************************/
	IdaPtrInfo::IdaPtrInfo(IdaTypeInfo info)
		: m_info(info)
	{
	}

	/**********************************************************************/
	const TypeInfo& IdaPtrInfo::getType() const
	{
		return m_info;
	}

	/**********************************************************************/
	std::unique_ptr<TypeInfo> IdaPtrInfo::getPointedObject() const
	{
		return std::make_unique<IdaTypeInfo>(m_info.m_type.get_pointed_object());
	}

	/**********************************************************************/
	IdaArrayInfo::IdaArrayInfo(IdaTypeInfo info)
		: m_info(info)
	{
	}

	/**********************************************************************/
	const TypeInfo& IdaArrayInfo::getType() const
	{
		return m_info;
	}

	/**********************************************************************/
	std::unique_ptr<TypeInfo> IdaArrayInfo::getPointedObject() const
	{
		return std::make_unique<IdaTypeInfo>(m_info.m_type.get_array_element());
	}

	/**********************************************************************/
	uint64_t IdaArrayInfo::getSize() const
	{
		return m_info.m_type.get_array_nelems();
	}

	/**********************************************************************/
	IdaTypeInfo::IdaTypeInfo(tinfo_t id)
		: m_type{ id }
	{

	}

	/**********************************************************************/
	size_t IdaTypeInfo::getSize() const
	{
		return m_type.get_size();
	}

	/**********************************************************************/
	bool IdaTypeInfo::isInt() const
	{
		return m_type.is_int();
	}

	/**********************************************************************/
	std::string IdaTypeInfo::getName() const
	{
		qstring name;
		m_type.print(&name);
		return name.c_str();
	}

	/**********************************************************************/
	bool IdaTypeInfo::isBool() const
	{
		return m_type.is_bool();
	}

	/**********************************************************************/
	bool IdaTypeInfo::isFloat() const
	{
		return m_type.is_float();
	}

	/**********************************************************************/
	bool IdaTypeInfo::isVoid() const
	{
		return m_type.is_void();
	}

	/**********************************************************************/
	bool IdaTypeInfo::isConst() const
	{
		return m_type.is_const();
	}

	/**********************************************************************/
	bool IdaTypeInfo::isChar() const
	{
		return m_type.is_char();
	}

	/**********************************************************************/
	bool IdaTypeInfo::isUnicode() const
	{
		if (!isInt())
		{
			return false;
		}

		auto name = getName();
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);
		if (name.find("wchar") != std::string::npos)
		{
			return true;
		}

		if (name.find("wstr") != std::string::npos)
		{
			return true;
		}

		return false;
	}

	/**********************************************************************/
	std::optional<std::unique_ptr<FuncInfo>> IdaTypeInfo::toFunc() const
	{
		if (!m_type.is_func())
		{
			return std::nullopt;
		}

		return std::make_unique<IdaFuncInfo>(*this);
	}

	/**********************************************************************/
	std::optional<std::unique_ptr<StructInfo>> IdaTypeInfo::toStruct() const
	{
		if (!m_type.is_struct())
		{
			return std::nullopt;
		}

		return std::make_unique<IdaStructInfo>(*this);
	}

	/**********************************************************************/
	std::optional<std::unique_ptr<PtrInfo>> IdaTypeInfo::toPtr() const
	{
		if (!m_type.is_ptr())
		{
			return std::nullopt;
		}

		return std::make_unique<IdaPtrInfo>(*this);
	}

	/**********************************************************************/
	std::optional<std::unique_ptr<ArrayInfo>> IdaTypeInfo::toArray() const
	{
		if (!m_type.is_array())
		{
			return std::nullopt;
		}

		return std::make_unique<IdaArrayInfo>(*this);
	}

	/**********************************************************************/
	std::optional<std::unique_ptr<TypeInfo>> IdaTypeInfoFactory::build(const std::string& name)
	{
		tinfo_t idaTypeInfo;

		if (!idaTypeInfo.get_named_type(get_idati(), name.c_str()))
		{
			return std::nullopt;
		}

		return std::make_unique<IdaTypeInfo>(idaTypeInfo);
	}

	/**********************************************************************/
	std::optional<std::unique_ptr<TypeInfo>> IdaTypeInfoFactory::build(uint64_t ea)
	{
		tinfo_t idaTypeInfo;

		if (!get_tinfo(&idaTypeInfo, ea) && !guess_tinfo(&idaTypeInfo, ea))
		{
			return std::nullopt;
		}

		return std::make_unique<IdaTypeInfo>(idaTypeInfo);
	}

	/**********************************************************************/
	std::optional<std::unique_ptr<TypeInfo>> IdaTypeInfoFactory::build(tinfo_t info)
	{
		return std::make_unique<IdaTypeInfo>(info);
	}

	/**********************************************************************/
	std::optional<std::unique_ptr<TypeInfo>> IdaTypeInfoFactory::build_decl(const std::string& name)
	{
		tinfo_t idaTypeInfo;
		qstring parsedName;
		std::stringstream ss;
		ss << name << ";";

		if (!parse_decl(&idaTypeInfo, &parsedName, nullptr, ss.str().c_str(), PT_TYP | PT_SIL))
		{
			return std::nullopt;
		}

		return std::make_unique<IdaTypeInfo>(idaTypeInfo);
	}

} // end of namespace yagi