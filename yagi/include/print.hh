#ifndef __YAGI_PRINT__
#define __YAGI_PRINT__

#include <printc.hh>
#include "decompiler.hh"

namespace yagi 
{

	/*!
	 * \brief	Use to declare the local print capability
	 */
	class IdaPrintCapability : public PrintLanguageCapability
	{
	private:
		/*!
		 * \brief	singleton model
		 */
		static IdaPrintCapability inst;

		/*!
		 * \brief	ctor
		 */
		explicit IdaPrintCapability();

		/*!
		 *	\brief	Copy is authorized because we only use copyable type
		 */
		IdaPrintCapability(const IdaPrintCapability&) = default;
		IdaPrintCapability& operator=(const IdaPrintCapability&) = default;

		/*!
		 *	\brief	moving is allowed
		 */
		IdaPrintCapability(IdaPrintCapability&&) noexcept = default;
		IdaPrintCapability& operator=(IdaPrintCapability&&) noexcept = default;

	public:
		/*!
		 * \brief	Build language from architecture
		 * \param	glb	current architecture
		 * \return	New print language
		 */
		PrintLanguage* buildLanguage(Architecture* glb) override;
	};

	/*!
	 * \brief	IDA emitter to follow IDA print
	 */
	class IdaEmit : public EmitPrettyPrint
	{
	protected:
		friend class EmitColorGuard;

		/*!
		 * \brief	start a color tag for each kind of token
		 */
		virtual void startColorTag(char c);

		/*!
		 * \brief	end of color tag
		 */
		virtual void endColorTag(char c);

	public:
		/*!
		 * \brief	use to clear cache
		 */
		int4 beginFunction(const Funcdata* fd) override;

		/*!
		 * \brief	Open parenthese
		 * \param	o	token use for parenthesis
		 */
		int4 openParen(char o, int4 id = 0) override;

		/*!
		 * \brief	close parenthesis
		 * \param	o	token use for parenthesis
		 */
		void closeParen(char c, int4 id) override;

		/*!
		 * \brief	when an operator (+, ->, -, ++, etc...) is printed
		 * \param	ptr	string that represent the operator
		 * \param	hl	color highlight
		 * \param	op	Associated opcode
		 */
		void tagOp(const char* ptr, syntax_highlight hl, const PcodeOp* op) override;

		/*!
		 * \brief	when a new variable is printed
		 * \param	ptr	name of the variable
		 * \param	hl	color highlight
		 * \param	vn	associated varnode (we keep tracking the address of symbol of high level)
		 * \param	op	current operation
		 */
		void tagVariable(const char* ptr, syntax_highlight hl,
			const Varnode* vn, const PcodeOp* op) override;

		/*!
		 * \brief	When the function name (header) is printed
		 * \param	ptr	name of the function
		 * \param	hl	color highlight
		 * \param	fd	function data (top of the compiler data)
		 * \param	op	current operation
		 */
		void tagFuncName(const char* ptr, syntax_highlight hl, const Funcdata* fd, const PcodeOp* op) override;

		/*!
		 * \brief	tag a field member of a struct
		 * \param	ptr	name of the field
		 * \param	hl	color highlight
		 * \param	ct	type of parent
		 * \param	off	offset of the field
		 */
		void tagField(const char* ptr, syntax_highlight hl, const Datatype* ct, int4 off) override;

		/*!
		 * \brief	tag a label
		 * \param	ptr	name of the label
		 * \param	spc
		 * \param	off	offset of the label
		 */
		void tagLabel(const char* ptr, syntax_highlight hl, const AddrSpace* spc, uintb off) override;

		/*!
		 * \print	When any print is done, emitter will call print function
		 * \param	str	string to print
		 * \param	hl	color highlight
		 */
		void print(const char* str, syntax_highlight hl = no_color) override;

		/*!
		 * \brief	When a type name is printed
		 * \param	ptr	name of the type
		 * \param	hl	color highlight
		 * \param	ct	associated datatype
		 */
		void tagType(const char* ptr, syntax_highlight hl, const Datatype* ct) override;

		/*!
		 * \brief	Return the symbol database
		 * \return	the associated map between the token name and it's associated address
		 */
		const std::map<std::string, MemoryLocation>& getSymbolAddr() const;
	};

	/*!
	 * \brief	Emit color guard use to simplify code
	 */
	class EmitColorGuard
	{
	protected:
		/*!
		 * \brief	current emitter
		 */
		IdaEmit& m_emitter;

		/*!
		 * \brief	tag color
		 */
		char m_color;
	public:
		/*!
		 * \brief	Ctor that will emit the code
		 * \param	emitter	emitter to control
		 * \param	color	color to emmit
		 */
		explicit EmitColorGuard(IdaEmit& emitter, char color);

		/*!
		 * \brief	Ctor that will emit the code
		 * \param	emitter	emitter to control
		 * \param	color	color to emmit
		 */
		explicit EmitColorGuard(IdaEmit& emitter, EmitPrettyPrint::syntax_highlight color);

		/*!
		 * \brief	destructor that will end the job
		 */
		~EmitColorGuard();
	};

	/*!
	 * \brief	Derive from c language
	 *			Use ida emit
	 */
	class IdaPrint : public PrintC
	{
	public:
		/*!
		 * \brief	ctor
		 */
		IdaPrint(Architecture* g, const string& nm);

		/*!
		 * \brief	Return the token emitter
		 * \return	Token emitter
		 */
		const IdaEmit& getEmitter() const;
	};
}

#endif