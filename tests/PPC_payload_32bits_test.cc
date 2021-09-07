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
	0x94, 0x21, 0xFF, 0xF0, 0x7C, 0x08, 0x02, 0xA6, 
	0x93, 0xE1, 0x00, 0x0C, 0x3F, 0xE0, 0x10, 0x01,
	0x89, 0x3F, 0x30, 0x94, 0x90, 0x01, 0x00, 0x14, 
	0x2F, 0x89, 0x00, 0x00, 0x40, 0x9E, 0x00, 0x10,
	0x4B, 0xFF, 0xFF, 0x61, 0x39, 0x20, 0x00, 0x01, 
	0x99, 0x3F, 0x30, 0x94, 0x80, 0x01, 0x00, 0x14,
	0x83, 0xE1, 0x00, 0x0C, 0x38, 0x21, 0x00, 0x10, 
	0x7C, 0x08, 0x03, 0xA6, 0x4E, 0x80, 0x00, 0x20
};

// Demonstrate some basic assertions.
TEST(TestDecompilationPayload_PPC_32, DecompileWithoutType) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"PowerPC:BE:32:default",
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
	
	ASSERT_STREQ(ss.str().c_str(), "\nvoid test(void)\n\n{\n  if (unk_0x10013094 == '\\0') {\n    func_0xaaaaaa2a();\n    unk_0x10013094 = '\\x01';\n  }\n  return;\n}\n");
}