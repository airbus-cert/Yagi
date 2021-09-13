#include "print.hh"
#include "symbolinfo.hh"
#include "varnode.hh"
#include "funcdata.hh"


#define COLOR_ON        '\1'     ///< Escape character (ON).
///< Followed by a color code (::color_t).
#define COLOR_OFF       '\2'     ///< Escape character (OFF).
								 ///< Followed by a color code (::color_t).
#define COLOR_ESC       '\3'     ///< Escape character (Quote next character).
								 ///< This is needed to output '\1' and '\2'
								 ///< characters.
#define COLOR_INV       '\4'     ///< Escape character (Inverse foreground and background colors).
								 ///< This escape character has no corresponding #COLOR_OFF.
								 ///< Its action continues until the next #COLOR_INV or end of line.

const char
COLOR_DEFAULT = '\x01',         ///< Default
COLOR_REGCMT = '\x02',         ///< Regular comment
COLOR_RPTCMT = '\x03',         ///< Repeatable comment (comment defined somewhere else)
COLOR_AUTOCMT = '\x04',         ///< Automatic comment
COLOR_INSN = '\x05',         ///< Instruction
COLOR_DATNAME = '\x06',         ///< Dummy Data Name
COLOR_DNAME = '\x07',         ///< Regular Data Name
COLOR_DEMNAME = '\x08',         ///< Demangled Name
COLOR_SYMBOL = '\x09',         ///< Punctuation
COLOR_CHAR = '\x0A',         ///< Char constant in instruction
COLOR_STRING = '\x0B',         ///< String constant in instruction
COLOR_NUMBER = '\x0C',         ///< Numeric constant in instruction
COLOR_VOIDOP = '\x0D',         ///< Void operand
COLOR_CREF = '\x0E',         ///< Code reference
COLOR_DREF = '\x0F',         ///< Data reference
COLOR_CREFTAIL = '\x10',         ///< Code reference to tail byte
COLOR_DREFTAIL = '\x11',         ///< Data reference to tail byte
COLOR_ERROR = '\x12',         ///< Error or problem
COLOR_PREFIX = '\x13',         ///< Line prefix
COLOR_BINPREF = '\x14',         ///< Binary line prefix bytes
COLOR_EXTRA = '\x15',         ///< Extra line
COLOR_ALTOP = '\x16',         ///< Alternative operand
COLOR_HIDNAME = '\x17',         ///< Hidden name
COLOR_LIBNAME = '\x18',         ///< Library function name
COLOR_LOCNAME = '\x19',         ///< Local variable name
COLOR_CODNAME = '\x1A',         ///< Dummy code name
COLOR_ASMDIR = '\x1B',         ///< Assembler directive
COLOR_MACRO = '\x1C',         ///< Macro
COLOR_DSTR = '\x1D',         ///< String constant in data directive
COLOR_DCHAR = '\x1E',         ///< Char constant in data directive
COLOR_DNUM = '\x1F',         ///< Numeric constant in data directive
COLOR_KEYWORD = '\x20',         ///< Keywords
COLOR_REG = '\x21',         ///< Register name
COLOR_IMPNAME = '\x22',         ///< Imported name
COLOR_SEGNAME = '\x23',         ///< Segment name
COLOR_UNKNAME = '\x24',         ///< Dummy unknown name
COLOR_CNAME = '\x25',         ///< Regular code name
COLOR_UNAME = '\x26',         ///< Regular unknown name
COLOR_COLLAPSED = '\x27',         ///< Collapsed line
COLOR_FG_MAX = '\x28';         ///< Max color number

namespace yagi 
{
	/**********************************************************************/
	// Constructing this registers the capability
	IdaPrintCapability IdaPrintCapability::inst;

	/**********************************************************************/
	IdaPrintCapability::IdaPrintCapability(void)
	{
		name = "yagi-c-language";
		isdefault = false;
	}

	/**********************************************************************/
	PrintLanguage* IdaPrintCapability::buildLanguage(Architecture* glb)
	{
		return new IdaPrint(glb, name);
	}

	/**********************************************************************/
	IdaPrint::IdaPrint(Architecture* g, const string& nm)
		: PrintC(g, nm)
	{
		emit = new IdaEmit();
		emit->setMaxLineSize(400);
	}

	/**********************************************************************/
	const IdaEmit& IdaPrint::getEmitter() const
	{
		return *static_cast<IdaEmit*>(emit);
	}

	/**********************************************************************/
	void IdaEmit::startColorTag(char c)
	{
		std::stringstream ss;
		ss << COLOR_ON << c;
		EmitPrettyPrint::print(ss.str().c_str());
	}

	/**********************************************************************/
	void IdaEmit::endColorTag(char c)
	{
		std::stringstream ss;
		ss << COLOR_OFF << c;
		EmitPrettyPrint::print(ss.str().c_str());
	}

	/**********************************************************************/
	int4 IdaEmit::beginFunction(const Funcdata* fd)
	{
		return EmitPrettyPrint::beginFunction(fd);
	}

	/**********************************************************************/
	int4 IdaEmit::openParen(char o, int4 id)
	{
		EmitColorGuard guard(*this, COLOR_KEYWORD);
		return EmitPrettyPrint::openParen(o, id);
	}

	/**********************************************************************/
	void IdaEmit::closeParen(char c, int4 id)
	{
		EmitColorGuard guard(*this, COLOR_KEYWORD);
		return EmitPrettyPrint::closeParen(c, id);
	}

	/**********************************************************************/
	void IdaEmit::tagOp(const char* ptr, syntax_highlight hl, const PcodeOp* op)
	{
		EmitColorGuard guard(*this, COLOR_KEYWORD);
		EmitPrettyPrint::tagOp(ptr, hl, op);
	}

	/**********************************************************************/
	void IdaEmit::tagVariable(const char* ptr, syntax_highlight hl,
		const Varnode* vn, const PcodeOp* op)
	{
		auto name = std::string(ptr);
		bool isImport = false;

		if (name.substr(0, SymbolInfo::IMPORT_PREFIX.length()) == SymbolInfo::IMPORT_PREFIX)
		{
			isImport = true;
			name = name.substr(SymbolInfo::IMPORT_PREFIX.length(), name.length() - SymbolInfo::IMPORT_PREFIX.length());
		}

		// case of variable declaration
		if (op == nullptr)
		{
			hl = syntax_highlight::keyword_color;
		}

		if (isImport)
		{
			EmitColorGuard guard(*this, COLOR_IMPNAME);
			EmitPrettyPrint::tagVariable(name.c_str(), hl, vn, op);
		}
		// Constant string
		else if (*ptr == '\"' || *ptr == '\'')
		{
			EmitColorGuard guard(*this, COLOR_DSTR);
			EmitPrettyPrint::tagVariable(name.c_str(), hl, vn, op);
		}
		// unicode string
		else if (*ptr == 'L' && ptr[1] != '\0' && (ptr[1] == '\"' || ptr[1] == '\''))
		{
			EmitColorGuard guard(*this, COLOR_DSTR);
			EmitPrettyPrint::tagVariable(name.c_str(), hl, vn, op);
		}
		else
		{
			EmitColorGuard guard(*this, hl);
			EmitPrettyPrint::tagVariable(name.c_str(), hl, vn, op);
		}
	}

	/**********************************************************************/
	void IdaEmit::tagFuncName(const char* ptr, syntax_highlight hl, const Funcdata* fd, const PcodeOp* op)
	{
		auto name = std::string(ptr);
		bool isImport = false;

		if (name.substr(0, SymbolInfo::IMPORT_PREFIX.length()) == SymbolInfo::IMPORT_PREFIX)
		{
			isImport = true;
			name = name.substr(SymbolInfo::IMPORT_PREFIX.length(), name.length() - SymbolInfo::IMPORT_PREFIX.length());
		}
		
		if (isImport)
		{
			EmitColorGuard guard(*this, COLOR_IMPNAME);
			EmitPrettyPrint::tagFuncName(name.c_str(), hl, fd, op);
		}
		else
		{
			EmitColorGuard guard(*this, hl);
			EmitPrettyPrint::tagFuncName(name.c_str(), hl, fd, op);
		}
	}

	/**********************************************************************/
	void IdaEmit::tagField(const char* ptr, syntax_highlight hl, const Datatype* ct, int4 off)
	{
		EmitColorGuard guard(*this, COLOR_KEYWORD);
		EmitPrettyPrint::tagField(ptr, hl, ct, off);
	}

	void IdaEmit::tagLabel(const char* ptr, syntax_highlight hl, const AddrSpace* spc, uintb off)
	{
		EmitColorGuard guard(*this, COLOR_KEYWORD);
		EmitPrettyPrint::tagLabel(ptr, hl, spc, off);
	}

	/**********************************************************************/
	void IdaEmit::print(const char* str, syntax_highlight hl)
	{
		// handle C synthax token
		switch (str[0])
		{
		case '{':
		case '}':
		case ';':
		case ',':
			hl = syntax_highlight::keyword_color;
		default:
			break;
		}

		EmitColorGuard guard(*this, hl);
		EmitPrettyPrint::print(str, hl);
	}

	/**********************************************************************/
	void IdaEmit::tagType(const char* ptr, syntax_highlight hl, const Datatype* ct)
	{
		EmitColorGuard guard(*this, hl);
		EmitPrettyPrint::tagType(ptr, hl, ct);
	}

	/**********************************************************************/
	EmitColorGuard::EmitColorGuard(IdaEmit& emitter, char color)
		: m_emitter(emitter), m_color(color)
	{
		m_emitter.startColorTag(m_color);
	}

	/**********************************************************************/
	EmitColorGuard::EmitColorGuard(IdaEmit& emitter, EmitPrettyPrint::syntax_highlight color)
		: m_emitter(emitter)
	{
		switch (color)
		{
		case EmitXml::keyword_color:
			m_color = COLOR_KEYWORD;
			break;
		case EmitXml::comment_color:
			m_color = COLOR_RPTCMT;
			break;
		case EmitXml::type_color:
			m_color = COLOR_RPTCMT;
			break;
		case EmitXml::funcname_color:
			m_color = COLOR_DEFAULT;
			break;
		case EmitXml::var_color:
			m_color = COLOR_DREF;
			break;
		case EmitXml::const_color:
			m_color = COLOR_KEYWORD;
			break;
		case EmitXml::param_color:
			m_color = COLOR_DEFAULT;
			break;
		case EmitXml::global_color:
			m_color = COLOR_DEFAULT;
			break;
		case EmitXml::no_color:
			m_color = COLOR_DEFAULT;
			break;
		default:
			m_color = COLOR_DEFAULT;
			break;
		}

		m_emitter.startColorTag(m_color);
	}

	/**********************************************************************/
	EmitColorGuard::~EmitColorGuard()
	{
		m_emitter.endColorTag(m_color);
	}
} // end of namespace ghidra