set(yagi_SRC
	src/yagiarchitecture.cc
	src/base.cc
	src/ghidradecompiler.cc
	src/exception.cc
	src/yagi.cc
	src/idatype.cc
	src/ghidra.cc
	src/idalogger.cc
	src/idasymbol.cc
	src/idaloader.cc
	src/plugin.cc
	src/print.cc
	src/scope.cc
	src/symbolinfo.cc
	src/typemanager.cc
)

set(yagi_INCLUDE
	include/yagiarchitecture.hh
	include/base.hh
	include/decompiler.hh
	include/ghidradecompiler.hh
	include/idatype.hh
	include/exception.hh
	include/ghidra.hh
	include/idalogger.hh
	include/idaloader.hh
	include/idasymbol.hh
	include/idatool.hh
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

add_library(yagi SHARED ${yagi_INCLUDE} ${yagi_SRC})
target_compile_features(yagi PRIVATE cxx_std_17)

target_include_directories(
	yagi
	INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${IDA_SDK_INCLUDE_DIRS} include
)

set_target_properties(yagi PROPERTIES 
	OUTPUT_NAME "yagi64"
	PREFIX ""
)

install(TARGETS yagi
       RUNTIME DESTINATION plugins
       LIBRARY DESTINATION plugins
)

if(MSVC)
	add_definitions(
		/wd4267
		/wd4244
		/wd4099
	)
endif()

target_link_libraries(yagi libdecomp ${IDA_SDK_LIBS})

# This is a trick for compiling on visual studio
# static initializer are stripped because libdecomp are a static library
# and no PrintLanguage are registred because singleton ctor are never called
if(MSVC)
target_link_options(yagi PRIVATE /WHOLEARCHIVE:libbase.lib)
endif()

# use to set address in 64 bits, must be set only for IDA 64 bits (sizeof ea_t)
target_compile_definitions(yagi PRIVATE __EA64__)


# Use for tests
set(yagi_TEST_SRC
	src/yagiarchitecture.cc
	src/base.cc
	src/exception.cc
	src/ghidra.cc
	src/scope.cc
	src/symbolinfo.cc
	src/typemanager.cc
)

set(yagi_TEST_INCLUDE
	include/yagiarchitecture.hh
	include/base.hh
	include/exception.hh
	include/ghidra.hh
	include/decompiler.hh
	include/loader.hh
	include/logger.hh
	include/scope.hh
	include/symbolinfo.hh
	include/typemanager.hh
	include/typeinfo.hh
)

add_library(yagi_static STATIC ${yagi_TEST_INCLUDE} ${yagi_TEST_SRC})

target_compile_features(yagi_static PRIVATE cxx_std_17)
target_compile_definitions(yagi_static PRIVATE __EA64__)

target_include_directories(
	yagi_static
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${IDA_SDK_INCLUDE_DIRS}
)
target_link_libraries(yagi_static libdecomp)
