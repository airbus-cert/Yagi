#include <gtest/gtest.h>
#include "yagiarchitecture.hh"
#include "mock_logger_test.h"
#include "mock_symbol_test.h"
#include "mock_type_test.h"
#include "mock_loader_test.h"
#include "ghidra.hh"

#define FUNC_ADDR 0x0000EFD3
#define FUNC_SIZE 14
#define FUNC_NAME "test"

static const uint8_t PAYLOAD[] = {
	0xA9, 0x00, 0x85, 0x4A, 0x85, 0x4C, 0xA9, 0x08,
	0x85, 0x4B, 0xA9, 0x10, 0x85, 0x4D, 
};

// Demonstrate some basic assertions.
TEST(TestDecompilationPayload_6502, DecompileWithoutType) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"6502:LE:16:default",
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
	
	ASSERT_STREQ(ss.str().c_str(), "\nvoid test(void)\n\n{\n  unk_0x4a = 0;\n  unk_0x4c = 0;\n  unk_0x4b = 8;\n  unk_0x4d = 0x10;\n                    /* WARNING: Could not recover jumptable at 0xefe1. Too many branches */\n                    /* WARNING: Treating indirect jump as call */\n  (*unk_0xfffe)();\n  return;\n}\n");
}