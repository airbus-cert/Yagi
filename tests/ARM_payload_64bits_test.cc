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
	0x3D, 0x20, 0x10, 0x01, 0x3C, 0x60, 0x10, 0x01, 
	0x39, 0x29, 0x30, 0x8C, 0x38, 0x63, 0x30, 0x8C,
	0x39, 0x29, 0x00, 0x03, 0x7D, 0x23, 0x48, 0x50, 
	0x2B, 0x89, 0x00, 0x06, 0x4C, 0x9D, 0x00, 0x20,
	0x3D, 0x20, 0x00, 0x00, 0x39, 0x29, 0x00, 0x00, 
	0x2F, 0x89, 0x00, 0x00, 0x4D, 0x9E, 0x00, 0x20,
	0x7D, 0x29, 0x03, 0xA6, 0x4E, 0x80, 0x04, 0x20, 
	0x60, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00
};

// Demonstrate some basic assertions.
TEST(TestDecompilationPayload_ARM_64, DecompileWithoutType) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"AARCH64:BE:32:default",
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