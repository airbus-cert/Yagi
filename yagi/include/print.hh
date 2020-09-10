#ifndef __YAGI_PRINT__
#define __YAGI_PRINT__

#include <printc.hh>

namespace yagi 
{
	class IdaPrintCapability : public PrintLanguageCapability
	{
	private:
		static IdaPrintCapability inst;
		IdaPrintCapability();

	public:
		PrintLanguage* buildLanguage(Architecture* glb) override;
	};

	class IdaPrint : public PrintC
	{
	public:
		IdaPrint(Architecture* g, const string& nm);
	};

	class IdaEmit : public EmitPrettyPrint
	{
	protected:
		friend class EmitColorGuard;
		virtual void startColorTag(char c);
		virtual void endColorTag(char c);

	public:
		int4 openParen(char o, int4 id = 0) override;
		void closeParen(char c, int4 id) override;
		void tagOp(const char* ptr, syntax_highlight hl, const PcodeOp* op) override;
		void tagVariable(const char* ptr, syntax_highlight hl,
			const Varnode* vn, const PcodeOp* op) override;
		void tagFuncName(const char* ptr, syntax_highlight hl, const Funcdata* fd, const PcodeOp* op) override;
		void print(const char* str, syntax_highlight hl = no_color) override;
		void tagType(const char* ptr, syntax_highlight hl, const Datatype* ct) override;
	};

	class EmitColorGuard
	{
	protected:
		IdaEmit& m_emitter;
		char m_color;
	public:
		explicit EmitColorGuard(IdaEmit& emitter, char color);
		explicit EmitColorGuard(IdaEmit& emitter, EmitPrettyPrint::syntax_highlight color);
		~EmitColorGuard();
	};
}

#endif