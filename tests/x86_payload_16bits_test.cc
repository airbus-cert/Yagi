#include <gtest/gtest.h>
#include "yagiarchitecture.hh"
#include "mock_logger_test.h"
#include "mock_symbol_test.h"
#include "mock_type_test.h"
#include "mock_loader_test.h"
#include "ghidra.hh"

#define FUNC_ADDR 0xaaaaaaaa
#define FUNC_SIZE 28
#define FUNC_NAME "test"

static const uint8_t PAYLOAD_1[] = {
	0x55, 0x50, 0x53, 0x89, 0xE5, 0x8B, 0x76, 0x08,
	0xAC, 0x3C, 0x00, 0x74, 0x09, 0xB4, 0x0E, 0xBB,
	0x00, 0x00, 0xCD, 0x10, 0xEB, 0xF2, 0x89, 0xEC,
	0x5B, 0x58, 0x5D, 0xC3, 0x55, 0x50, 0x53, 0x52,
	0x89, 0xE5, 0xB4, 0x03, 0xBB, 0x00, 0x00, 0xCD,
	0x10, 0xFE, 0xC6
};


// Demonstrate some basic assertions.
TEST(TestDecompilationPayload_x86_16, Decompile) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"x86:LE:16:Real Mode",
		std::make_unique<MockLoaderFactory>([](uint1* ptr, int4 size, const Address& addr) {
			memcpy(ptr, PAYLOAD_1 + addr.getOffset() - FUNC_ADDR, size);
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
		"__fastcall"
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
	
	ASSERT_STREQ(ss.str().c_str(), "\n__uint16 test(char *param_1)\n\n{\n  char *pcVar1;\n  code *pcVar2;\n  __uint16 in_AX;\n  __uint16 unaff_DS;\n  \n  while( true ) {\n    pcVar1 = param_1;\n    param_1 = param_1 + 1;\n    if (*pcVar1 == '\\0') break;\n    pcVar2 = (code *)swi(0x10);\n    (*pcVar2)();\n  }\n  return in_AX;\n}\n");
}
