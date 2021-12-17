#include <gtest/gtest.h>
#include "yagiarchitecture.hh"
#include "mock_logger_test.h"
#include "mock_symbol_test.h"
#include "mock_type_test.h"
#include "mock_loader_test.h"
#include "ghidra.hh"
#include "yagirule.hh"

#define FUNC_ADDR 0x140018A70
#define FUNC_SIZE 90
#define FUNC_NAME "test"

static const uint8_t PAYLOAD_1[] = {
	0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57,
	0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0xE9, 0x49, 0x8B, 0xF9, 0x49, 0x8B, 0xC9, 0x41, 0x8B, 0xF0,
	0x48, 0x8B, 0xDA, 0x48, 0xFF, 0x15, 0x86, 0xAB, 0x01, 0x00, 0x0F, 0x1F, 0x44, 0x00, 0x00, 0x85,
	0xC0, 0x75, 0x2A, 0x48, 0xFF, 0x15, 0x2E, 0xAB, 0x01, 0x00, 0x0F, 0x1F, 0x44, 0x00, 0x00, 0x89,
	0x05, 0xF3, 0xD0, 0x02, 0x00, 0x32, 0xC0, 0x48, 0x8B, 0x5C, 0x24, 0x30, 0x48, 0x8B, 0x6C, 0x24,
	0x38, 0x48, 0x8B, 0x74, 0x24, 0x40, 0x48, 0x83, 0xC4, 0x20, 0x5F, 0xC3, 0xCC, 0x8B, 0xD6, 0x48,
	0x8B, 0xCB, 0x48, 0x8B, 0xC5, 0xFF, 0x15, 0x9D, 0xB3, 0x01, 0x00, 0x85, 0xC0, 0x74, 0x0B, 0x83,
	0x25, 0xC2, 0xD0, 0x02, 0x00, 0x00, 0xB0, 0x01, 0xEB, 0xCD, 0x48, 0x8B, 0xD3, 0x48, 0x8B, 0xCF,
	0xEB, 0xA1, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC
};

// Demonstrate some basic assertions.
TEST(TestDecompilationPayload_x86_64, CheckControlFlowGuardPatch) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"x86:LE:64:default:windows",
		std::make_unique<MockLoaderFactory>([](uint1* ptr, int4 size, const Address& addr) {
			memcpy(ptr, PAYLOAD_1 + (addr.getOffset() - FUNC_ADDR), size);
		}),
		std::make_unique<MockLogger>([](const std::string&) {}),
		std::make_unique<MockSymbolInfoFactory>([](uint64_t ea) -> std::optional<std::unique_ptr<yagi::SymbolInfo>> {
			if (ea == FUNC_ADDR)
			{
				return std::make_unique<MockSymbolInfo>(
					FUNC_ADDR, FUNC_NAME, FUNC_SIZE, true, false, false, false
				);
			}
			// cfg wrapper
			if (ea == 0x1400335d8)
			{
				return std::make_unique<MockSymbolInfo>(
					0x1400335d8, "guard_dispatch_icall_fptr", 8, true, false, false, false
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

	// add CFG rules
	arch->extra_pool_rules.push_back(new yagi::RuleWindowsControlFlowGuard("analysis", "guard_dispatch_icall_fptr"));

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
	
	ASSERT_STREQ(ss.str().c_str(), "\n/* WARNING: Yagi : Control Flow Guard patching */\n/* WARNING: Heritage AFTER dead removal. Example location: RAX : 0x000140018aa3 */\n/* WARNING: Restarted to delay deadcode elimination for space: register */\n\n__uint64 test(__uint64 param_1,__uint64 param_2,__uint32 param_3,__uint64 param_4)\n\n{\n  code *in_RAX;\n  \n  do {\n    (*unk_0x140033620)(param_4,param_2);\n    if ((int32_t)in_RAX == 0) {\n      unk_0x140045ba8 = (*in_RAX)();\n      return 0;\n    }\n    in_RAX = (code *)(*unk_0x140033e78)(param_2,param_3);\n  } while ((int32_t)in_RAX == 0);\n  unk_0x140045ba8 = 0;\n  return 1;\n}\n");
}
