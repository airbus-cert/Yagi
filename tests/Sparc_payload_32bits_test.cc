#include <gtest/gtest.h>
#include "yagiarchitecture.hh"
#include "mock_logger_test.h"
#include "mock_symbol_test.h"
#include "mock_type_test.h"
#include "mock_loader_test.h"
#include "ghidra.hh"

#define FUNC_ADDR 0x000111A8
#define FUNC_SIZE 20
#define FUNC_NAME "test"

static const uint8_t PAYLOAD[] = {
	0x9D, 0xE3, 0xBF, 0xA0, 0xF0, 0x27, 0xA0, 0x44,
0x40, 0x00, 0x45, 0x8B, 0x90, 0x16, 0x00, 0x00, 0x40, 0x00, 0x45, 0x83, 0x90, 0x10, 0x20, 0x01,
0x81, 0xC7, 0xE0, 0x08, 0x81, 0xE8, 0x00, 0x00, 0x81, 0xC7, 0xE0, 0x08
	};

// Demonstrate some basic assertions.
TEST(TestDecompilationPayload_sparc_32, DecompileWithoutType) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"sparc:BE:32:default",
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
	
	ASSERT_STREQ(ss.str().c_str(), "\n/* WARNING: Removing unreachable block (ram,0x000111b8) */\n/* WARNING: Removing unreachable block (ram,0x000111a8) */\n/* WARNING: Removing unreachable block (ram,0x000111b0) */\n\n__uint32 test(__uint32 param_1)\n\n{\n  func_0x000227dc(param_1);\n  func_0x000227c4(1);\n  return param_1;\n}\n");
}