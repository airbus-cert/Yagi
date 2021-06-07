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
	0x60, 0x01, 0x00, 0xF0, 0x00, 0x98, 0x43, 0xF9, 
	0x01, 0x00, 0x40, 0x39, 0x61, 0x00, 0x00, 0x35,
	0xC0, 0x03, 0x5F, 0xD6, 0x1F, 0x20, 0x03, 0xD5, 
	0x41, 0x01, 0x00, 0xF0, 0x42, 0x01, 0x00, 0xF0,
	0x21, 0xAC, 0x46, 0xF9, 0x42, 0xB0, 0x47, 0xF9, 
	0x21, 0x00, 0x40, 0xB9, 0xF9, 0xF7, 0xFF, 0x17
};

// Demonstrate some basic assertions.
TEST(TestDecompilationPayload_ARM_64, DecompileWithoutType) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"AARCH64:LE:64:v8A:default",
		std::make_unique<MockLoaderFactory>([](uint1* ptr, int4 size, const Address& addr) {
			memcpy(ptr, PAYLOAD + addr.getOffset() - FUNC_ADDR, size);
		}),
		std::make_unique<MockLogger>([](const std::string&) {}),
		std::make_unique<MockSymbolInfoFactory>([](uint64_t ea) -> std::optional<std::unique_ptr<yagi::SymbolInfo>> {
			if (ea == FUNC_ADDR)
			{
				return std::make_unique<MockFunctionSymbolInfo>(
					FUNC_ADDR, FUNC_NAME, FUNC_SIZE, true, false, false, false
				);
			}
			return std::nullopt; 
		}, 
		[](uint64_t func_addr) -> std::optional<std::unique_ptr<yagi::SymbolInfo>> { 
			return std::nullopt;
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
	arch->allacts.getCurrent()->perform(*func);

	arch->setPrintLanguage("c-language");

	stringstream ss;
	arch->print->setOutputStream(&ss);
	//print as C
	arch->print->docFunction(func);
	
	ASSERT_STREQ(ss.str().c_str(), "\nvoid test(void)\n\n{\n  if (*unk_0xaaaaaac2 == '\\0') {\n    *unk_0xaaaaaac2 = '\\x01';\n  }\n  return;\n}\n");
}