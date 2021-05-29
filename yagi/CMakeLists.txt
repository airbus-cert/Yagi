set(YAGI_SOURCE
	src/yagiarchitecture.cc
	src/base.cc
	src/decompiler.cc
	src/exception.cc
	src/yagi.cc
	src/idatype.cc
	src/ghidra.cc
	src/idalogger.cc
	src/idasymbol.cc
	src/loader.cc
	src/plugin.cc
	src/print.cc
	src/scope.cc
	src/symbolinfo.cc
	src/typemanager.cc
)

set(YAGI_INCLUDE
	include/yagiarchitecture.hh
	include/base.hh
	include/decompiler.hh
	include/idatype.hh
	include/exception.hh
	include/ghidra.hh
	include/idalogger.hh
	include/idasymbol.hh
	include/idatool.hh
	include/idecompile.hh
	include/logger.hh
	include/loader.hh
	include/plugin.hh
	include/print.hh
	include/scope.hh
	include/symbolinfo.hh
	include/typemanager.hh
	include/typeinfo.hh
)

find_package(IdaSdk)

add_library(yagi SHARED ${YAGI_INCLUDE} ${YAGI_SOURCE})
target_include_directories(yagi PRIVATE include)
target_compile_features(yagi PRIVATE cxx_std_17)

if(MSVC)
	add_definitions(
		/wd4267
		/wd4244
		/wd4099
	)
endif()

target_include_directories(yagi PRIVATE ${IDA_SDK_INCLUDE_DIRS})
target_link_libraries(yagi libdecomp ${IDA_SDK_LIBS})

# This is a trick for compiling on visual studio
# static initializer are stripped because libdecomp are a static library
# and no PrintLanguage are registred because singleton ctor are never called
target_link_options(yagi PRIVATE /WHOLEARCHIVE:libbase.lib)

# use to set address in 64 bits, must be set only for IDA 64 bits (sizeof ea_t)
target_compile_definitions(yagi PRIVATE __EA64__)