#include <gtest/gtest.h>
#include "yagiarchitecture.hh"
#include "mock_logger_test.h"
#include "mock_symbol_test.h"
#include "mock_type_test.h"
#include "mock_loader_test.h"
#include "ghidra.hh"

#define FUNC_ADDR 0x00
#define FUNC_SIZE 0x19
#define FUNC_NAME "test"

static const uint8_t PAYLOAD_1[] = {
	0x21, 0x00, 0x00, 0x2B, 0xCD, 0x08, 0x00, 0x76, 
	0x3E, 0x30, 0xD3, 0x01, 0x3E, 0x78, 0xD3, 0x01,
	0x4C, 0xCD, 0x19, 0x00, 0x4D, 0xCD, 0x19, 0x00, 
	0xC9
};

// Demonstrate some basic assertions.
TEST(TestDecompilationPayload_z80, DecompileWithoutType) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"z80:LE:16:default",
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
	
	ASSERT_STREQ(ss.str().c_str(), "\n/* WARNING: Possible PIC construction at 0x0004: Changing call to branch */\n/* WARNING: Removing unreachable block (ram,0x0007) */\n\nvoid test(void)\n\n{\n  __uint8 _Var1;\n  \n  _Var1 = 0xff;\n  unk_0x1 = 0x78;\n  func_0x0019(0xff,7);\n  func_0x0019(_Var1);\n  return;\n}\n");
}