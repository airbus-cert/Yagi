#include "architecture.hh"
#include "yagiaction.hh"
#include "loader.hh"
#include "scope.hh"
#include "typemanager.hh"

namespace yagi 
{
	/**********************************************************************/
	YagiArchitecture::YagiArchitecture(
		const std::string& name,
		const std::string& sleighId,
		std::unique_ptr<LoaderFactory> loaderFactory,
		std::unique_ptr<Logger> logger,
		std::unique_ptr<SymbolInfoFactory> symbols,
		std::unique_ptr<TypeInfoFactory> type,
		std::string defaultCC
	) : SleighArchitecture(name, sleighId, &m_err),
		m_loaderFactory{ std::move(loaderFactory)},
		m_logger{ std::move(logger) }, 
		m_symbols{ std::move(symbols) }, 
		m_type{ std::move(type) },
		m_defaultCC { defaultCC },
		m_customAction(Action::rule_onceperfunc, "yagiactiongroup")
	{
	}

	/**********************************************************************/
	void YagiArchitecture::buildLoader(DocumentStorage& store)
	{
		std::stringstream error;
		collectSpecFiles(error);
		if (error.str().length() > 0) {
			m_logger->error("spec files loading", error.str());
		}
		loader = m_loaderFactory->build();
	}

	/**********************************************************************/
	void YagiArchitecture::buildTypegrp(DocumentStorage& store)
	{
		types = new TypeManager(this);
		types->setCoreType("void", 1, TYPE_VOID, false);
		types->setCoreType("bool", 1, TYPE_BOOL, false);
		types->setCoreType("uint8_t", 1, TYPE_UINT, false);
		types->setCoreType("uint16_t", 2, TYPE_UINT, false);
		types->setCoreType("uint32_t", 4, TYPE_UINT, false);
		types->setCoreType("uint64_t", 8, TYPE_UINT, false);
		types->setCoreType("char", 1, TYPE_INT, true);
		types->setCoreType("int8_t", 1, TYPE_INT, false);
		types->setCoreType("int16_t", 2, TYPE_INT, false);
		types->setCoreType("int32_t", 4, TYPE_INT, false);
		types->setCoreType("int64_t", 8, TYPE_INT, false);
		types->setCoreType("float", 4, TYPE_FLOAT, false);
		types->setCoreType("double", 8, TYPE_FLOAT, false);
		types->setCoreType("float16", 16, TYPE_FLOAT, false);
		types->setCoreType("__uint8", 1, TYPE_UNKNOWN, false);
		types->setCoreType("__uint16", 2, TYPE_UNKNOWN, false);
		types->setCoreType("__uint32", 4, TYPE_UNKNOWN, false);
		types->setCoreType("__uint64", 8, TYPE_UNKNOWN, false);
		types->setCoreType("code", 1, TYPE_CODE, false);
		types->cacheCoreTypes();
	}

	/**********************************************************************/
	Scope* YagiArchitecture::buildDatabase(DocumentStorage& store)
	{
		symboltab = new Database(this, false);
		Scope* globscope = new YagiScope(0, this);
		symboltab->attachScope(globscope, nullptr);
		return globscope;
	}

	/**********************************************************************/
	Translate* YagiArchitecture::buildTranslator(DocumentStorage& store)
	{
		m_translate = SleighArchitecture::buildTranslator(store);
		return m_translate;
	}

	/**********************************************************************/
	void YagiArchitecture::buildAction(DocumentStorage& store)
	{
		SleighArchitecture::buildAction(store);
		m_customAction.addAction(new ActionRenameStackVar("yagiidasyncstackvar"));
	}

	/**********************************************************************/
	int4 YagiArchitecture::performActions(Funcdata& data)
	{
		allacts.getCurrent()->reset(data);
		m_customAction.reset(data);

		auto res = allacts.getCurrent()->perform(data);
		if (res < 0)
		{
			return res;
		}

		return res + m_customAction.perform(data);
	}

	/**********************************************************************/
	SymbolInfoFactory& YagiArchitecture::getSymbolDatabase() const
	{
		return *m_symbols.get();
	}

	/**********************************************************************/
	TypeInfoFactory& YagiArchitecture::getTypeInfoFactory() const
	{
		return *m_type.get();
	}

	/**********************************************************************/
	Logger& YagiArchitecture::getLogger() const
	{
		return *m_logger.get();
	}

	/**********************************************************************/
	const std::string& YagiArchitecture::getDefaultCC() const
	{
		return m_defaultCC;
	}

	/**********************************************************************/
	void YagiArchitecture::addInjection(std::string functionName, std::string injection)
	{
		m_injectionMap.emplace(functionName, injection);
	}

	/**********************************************************************/
	std::optional<std::string> YagiArchitecture::findInjection(const std::string& functionName)
	{
		auto iter = m_injectionMap.find(functionName);
		if (iter == m_injectionMap.end())
		{
			return nullopt;
		}
		return iter->second;
	}

} // end of namespace yagi