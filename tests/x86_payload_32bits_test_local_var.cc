#include <gtest/gtest.h>
#include "yagiarchitecture.hh"
#include "mock_logger_test.h"
#include "mock_symbol_test.h"
#include "mock_type_test.h"
#include "mock_loader_test.h"
#include "ghidra.hh"

#define FUNC_ADDR 0x401fa0
#define FUNC_SIZE 34
#define FUNC_NAME "test"

static const uint8_t PAYLOAD[] = {
	0x56, 0x57, 0x8B, 0x7C, 0x24, 0x10, 0x57, 0xE8, 0xB2, 0x00, 0x00, 0x00, 0x8B, 0xF0, 0x8B, 0x44,
	0x24, 0x10, 0x57, 0x50, 0x46, 0xE8, 0x92, 0x04, 0x00, 0x00, 0x83, 0xC4, 0x0C, 0x8B, 0xC6, 0x5F,
	0x5E, 0xC3,
};


TEST(TestDecompilationPayload_x86_32, RenameLocalRegVar) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"x86:LE:32:default:windows",
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
			auto result = std::make_unique<MockFunctionSymbolInfo>(
					std::make_unique<MockSymbolInfo>(
						FUNC_ADDR, FUNC_NAME, FUNC_SIZE, true, false, false, false
					)
				);
			yagi::MemoryLocation loc("register", 0x0000000000000000, 4);
			loc.pc.push_back(0x0000000000401fa7);
			result->saveName(loc, "yeah");
			return result;
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
	
	ASSERT_STREQ(ss.str().c_str(), "\nint32_t test(__uint32 param_1,__uint32 param_2)\n\n{\n  int32_t yeah;\n  \n  yeah = func_0x0040205e(param_2);\n  func_0x0040244c(param_1,param_2);\n  return yeah + 1;\n}\n");
}

TEST(TestDecompilationPayload_x86_32, RetypeLocalRegVar) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"x86:LE:32:default:windows",
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
			auto result = std::make_unique<MockFunctionSymbolInfo>(
				std::make_unique<MockSymbolInfo>(
					FUNC_ADDR, FUNC_NAME, FUNC_SIZE, true, false, false, false
					)
				);
			yagi::MemoryLocation loc("register", 0, 4);
			loc.pc.push_back(0x0000000000401fa7);

			result->saveType(loc, MockTypeInfo(4, "pointer", true, false, false, false, false, false, false));
			return result;
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

	ASSERT_STREQ(ss.str().c_str(), "\nint32_t test(__uint32 param_1,__uint32 param_2)\n\n{\n  pointer pVar1;\n  \n  pVar1 = func_0x0040205e(param_2);\n  func_0x0040244c(param_1,param_2);\n  return pVar1 + 1;\n}\n");
}