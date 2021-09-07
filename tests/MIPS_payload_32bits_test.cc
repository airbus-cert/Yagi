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
	0x3C, 0x1C, 0x00, 0x03, 0x27, 0x9C, 0x55, 0x68, 0x03, 0x99, 0xE0, 0x21, 0x27, 0xBD, 0xFF, 0xE0, 0x8F, 0x99, 0x81, 0x0C, 0xAF, 0xB0, 0x00, 0x18,
	0x00, 0x80, 0x80, 0x25, 0x24, 0x04, 0x00, 0x04, 0xAF, 0xBC, 0x00, 0x10, 0xAF, 0xBF, 0x00, 0x1C,
	0x04, 0x11, 0xD4, 0xEF, 0x02, 0x00, 0x28, 0x25, 0x1A, 0x00, 0x00, 0x09, 0x8F, 0xBF, 0x00, 0x1C,
	0x00, 0x10, 0x20, 0x80, 0x24, 0x05, 0xFF, 0xFF, 0x00, 0x82, 0x20, 0x21, 0x00, 0x40, 0x18, 0x25,
	0xAC, 0x65, 0x00, 0x00, 0x24, 0x63, 0x00, 0x04, 0x14, 0x64, 0xFF, 0xFD, 0x8F, 0xBF, 0x00, 0x1C,
	0x8F, 0xB0, 0x00, 0x18, 0x03, 0xE0, 0x00, 0x08, 0x27, 0xBD, 0x00, 0x20, 0x3C, 0x1C, 0x00, 0x03
};

// Demonstrate some basic assertions.
TEST(TestDecompilationPayload_MIPS_32, DecompileWithoutType) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"MIPS:BE:32:default",
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
	
	ASSERT_STREQ(ss.str().c_str(), "\nvoid test(int32_t param_1,__uint32 param_2,__uint32 param_3,__uint32 param_4)\n\n{\n  __uint32 *p_Var1;\n  __uint32 *p_Var2;\n  int32_t in_t9;\n  \n  p_Var1 = (__uint32 *)func_0xaaa9fe92(4,param_1,param_3,param_4,in_t9 + 0x35568);\n  if (0 < param_1) {\n    p_Var2 = p_Var1 + param_1;\n    do {\n      *p_Var1 = 0xffffffff;\n      p_Var1 = p_Var1 + 1;\n    } while (p_Var1 != p_Var2);\n  }\n  return;\n}\n");
}