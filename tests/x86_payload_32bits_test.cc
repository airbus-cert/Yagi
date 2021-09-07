#include <gtest/gtest.h>
#include "yagiarchitecture.hh"
#include "mock_logger_test.h"
#include "mock_symbol_test.h"
#include "mock_type_test.h"
#include "mock_loader_test.h"
#include "ghidra.hh"

#define FUNC_ADDR 0xaaaaaaaa
#define FUNC_SIZE 20
#define FUNC_NAME "test"

static const uint8_t PAYLOAD[] = {
	0x53, 0x6A, 0x09, 0x8B, 0xD9, 0xE8, 0x3E, 0x58, 
	0x03, 0x00, 0x83, 0xC4, 0x04, 0x85, 0xC0, 0x74,
	0x14, 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC7, 
	0x40, 0x04, 0x01, 0x00, 0x00, 0x00, 0x83, 0xC0,
	0x08, 0x88, 0x18, 0x5B, 0xC3, 0x33, 0xC0, 0x5B, 
	0xC3, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC
};

// Demonstrate some basic assertions.
TEST(TestDecompilationPayload_x86_32, DecompileWithoutType) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"x86:LE:32:default:windows",
		std::make_unique<MockLoaderFactory>([](uint1* ptr, int4 size, const Address& addr) {
			memcpy(ptr, PAYLOAD + addr.getOffset() - FUNC_ADDR, size);
		}),
		std::make_unique<MockLogger>([](const std::string&) {}),
		std::make_unique<MockSymbolInfoFactory>([](uint64_t ea) -> std::optional<std::unique_ptr<yagi::SymbolInfo>> {
			if (ea == FUNC_ADDR)
			{
				return std::make_unique<MockSymbolInfo>(
					FUNC_ADDR, FUNC_NAME, FUNC_SIZE, true, false, false, false
				);
			}
			return std::nullopt; 
		}, 
		[](uint64_t func_addr) -> std::optional<std::unique_ptr<yagi::FunctionSymbolInfo>> {
			return std::make_unique<MockFunctionSymbolInfo>(
				std::make_unique<MockSymbolInfo>(
					FUNC_ADDR, FUNC_NAME, FUNC_SIZE, true, false, false, false
					)
				);
		}),
		std::make_unique<MockTypeInfoFactory>([](uint64_t) { return std::nullopt; }, [](const std::string&) { return std::nullopt; }),
		"__stdcall"
	);

	DocumentStorage store;
	arch->init(store);

	auto scope = arch->symboltab->getGlobalScope();
	auto func = scope->findFunction(
		Address(arch->getDefaultCodeSpace(), FUNC_ADDR)
	);
	arch->performActions(*func);

	arch->setPrintLanguage("c-language");

	stringstream ss;
	arch->print->setOutputStream(&ss);
	//print as C
	arch->print->docFunction(func);
	
	ASSERT_STREQ(ss.str().c_str(), "\n__uint32 * __fastcall test(__uint8 param_1)\n\n{\n  __uint32 *p_Var1;\n  \n  p_Var1 = (__uint32 *)func_0xaaae02f2(9);\n  if (p_Var1 != (__uint32 *)0x0) {\n    *p_Var1 = 0;\n    p_Var1[1] = 1;\n    *(__uint8 *)(p_Var1 + 2) = param_1;\n    return p_Var1 + 2;\n  }\n  return (__uint32 *)0x0;\n}\n");
}