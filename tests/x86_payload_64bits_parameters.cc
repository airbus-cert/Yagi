#include <gtest/gtest.h>
#include "yagiarchitecture.hh"
#include "mock_logger_test.h"
#include "mock_symbol_test.h"
#include "mock_type_test.h"
#include "mock_loader_test.h"
#include "ghidra.hh"

#define FUNC_ADDR 0xaaaaaaaa
#define FUNC_SIZE 27
#define FUNC_NAME "test"

static const uint8_t PAYLOAD_1[] = {
	0x48, 0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C,
	0x24, 0x08, 0x57, 0x48, 0x8B, 0x44, 0x24, 0x18,
	0xC7, 0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 0x33,
	0xC0, 0x5F, 0xC3, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC
};

TEST(TestDecompilationPayload_x86_64, DecompileWithFunctionReturnType) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"x86:LE:64:default:windows",
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
		std::make_unique<MockTypeInfoFactory>(
			[](uint64_t ea)  -> std::optional<std::unique_ptr<yagi::TypeInfo>> {
				if (ea != FUNC_ADDR)
				{
					return std::nullopt;
				}
				return std::make_unique<MockTypeInfo>(
					8, "functionName",
					MockFuncInfo(
						false, "__fastcall",
						std::vector<MockTypeInfo>{
							MockTypeInfo(8, "testUint8", true, false, false, false, false, false, false)
						},
						std::vector<std::string>{

						}
					)
				);
			},
			[](const std::string&) {
				return std::nullopt;
			}
		),
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

	ASSERT_STREQ(ss.str().c_str(), "\ntestUint8 test(void)\n\n{\n  int64_t in_RDX;\n  \n  *(__uint32 *)(in_RDX + 4) = 0;\n  return 0;\n}\n");
}

TEST(TestDecompilationPayload_x86_64, DecompileWithFunctionReturnTypeAndParameter) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"x86:LE:64:default:windows",
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
		std::make_unique<MockTypeInfoFactory>(
			[](uint64_t ea) -> std::optional<std::unique_ptr<yagi::TypeInfo>> {
				if (ea != FUNC_ADDR)
				{
					return std::nullopt;
				}
				return std::make_unique<MockTypeInfo>(
					8, "functionName",
					MockFuncInfo(
						false, "__fastcall",
						std::vector<MockTypeInfo>{
							MockTypeInfo(8, "testUint8", true, false, false, false, false, false, false),
							MockTypeInfo(8, "testUint864", true, false, false, false, false, false, false),
						},
						std::vector<std::string>{
						
						}
					)
				);
			},
			[](const std::string&) {
				return std::nullopt;
			}
		),
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

	ASSERT_STREQ(ss.str().c_str(), "\n__uint64 test(__uint64 param_1,int64_t param_2)\n\n{\n  *(__uint32 *)(param_2 + 4) = 0;\n  return 0;\n}\n");
}

TEST(TestDecompilationPayload_x86_64, DecompileWithFunctionReturnTypeAndParameterName) {

	yagi::ghidra::init(std::getenv("GHIDRADIRTEST"));

	auto arch = std::make_unique<yagi::YagiArchitecture>(
		"test",
		"x86:LE:64:default:windows",
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
		std::make_unique<MockTypeInfoFactory>(
			[](uint64_t ea)  -> std::optional<std::unique_ptr<yagi::TypeInfo>> {
				if (ea != FUNC_ADDR)
				{
					return std::nullopt;
				}
				return std::make_unique<MockTypeInfo>(
					8, "functionName",
					MockFuncInfo(
						false, "__fastcall",
						std::vector<MockTypeInfo>{
							MockTypeInfo(8, "testUint8", true, false, false, false, false, false, false),
							MockTypeInfo(8, "testUint864", true, false, false, false, false, false, false),
							MockTypeInfo(8, "testUint864", true, false, false, false, false, false, false),
						},
						std::vector<std::string>{
							"test_param_1",
							"test_param_2"
						}
					)
				);
			},
			[](const std::string&) {
				return std::nullopt;
			}
		),
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

	ASSERT_STREQ(ss.str().c_str(), "\ntestUint8 test(testUint864 test_param_1,testUint864 test_param_2)\n\n{\n  *(__uint32 *)(test_param_2 + 4) = 0;\n  return 0;\n}\n");
}