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
	0xF3, 0x0F, 0x1E, 0xFA, 0x8B, 0x05, 0x96, 0xA4, 0x01, 0x00, 0x41, 0x54, 0x41, 0x89, 0xFC, 0x85,
	0xC0, 0x75, 0x35, 0x48, 0x8B, 0x05, 0x96, 0x58, 0x01, 0x00, 0x48, 0x8D, 0x15, 0x1E, 0x9A, 0x01,
	0x00, 0x48, 0x39, 0xD0, 0x73, 0x3A, 0x48, 0x8D, 0x50, 0x01, 0x44, 0x88, 0x20, 0x44, 0x89, 0xE0,
	0x41, 0x5C, 0x48, 0x89, 0x15, 0x77, 0x58, 0x01, 0x00, 0xC7, 0x05, 0x65, 0xA4, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xC3, 0x0F, 0x1F, 0x40, 0x00, 0xC7, 0x05, 0x4E, 0xA4, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xE8, 0xC9, 0xF7, 0xFE, 0xFF, 0xEB, 0xBA, 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00,
	0xE8, 0x3B, 0xFF, 0xFF, 0xFF, 0x48, 0x8B, 0x05, 0x44, 0x58, 0x01, 0x00, 0xEB, 0xB8, 0x66, 0x90,
	0xF3, 0x0F, 0x1E, 0xFA, 0x41, 0x54, 0x55, 0x53, 0x48, 0x83, 0xEC, 0x10, 0x8B, 0x1D, 0x46, 0xA4
};

// Demonstrate some basic assertions.
TEST(TestDecompilationPayload_x86_64_gcc, DecompileWithoutType) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"x86:LE:64:default:gcc",
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
	
	ASSERT_STREQ(ss.str().c_str(), "\n__uint32 test(__uint32 param_1)\n\n{\n  if (unk_0xaaac4f4a != 0) {\n    unk_0xaaac4f4a = 0;\n    func_0xaaa9a2ca();\n  }\n  if ((__uint8 *)0xaaac44e8 < unk_0xaaac035a) {\n    func_0xaaaaaa4a();\n  }\n  *unk_0xaaac035a = (char)param_1;\n  unk_0xaaac035a = unk_0xaaac035a + 1;\n  unk_0xaaac4f52 = 0;\n  return param_1;\n}\n");
}