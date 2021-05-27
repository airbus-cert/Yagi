set(DECOMPILER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ghidra/Ghidra/Features/Decompiler/src/decompile/cpp")

set(DECOMPILER_SOURCE_BASE_CXX
		${DECOMPILER_SOURCE_DIR}/space.cc
		${DECOMPILER_SOURCE_DIR}/float.cc
		${DECOMPILER_SOURCE_DIR}/address.cc
		${DECOMPILER_SOURCE_DIR}/pcoderaw.cc
		${DECOMPILER_SOURCE_DIR}/translate.cc
		${DECOMPILER_SOURCE_DIR}/opcodes.cc
		${DECOMPILER_SOURCE_DIR}/globalcontext.cc
		${DECOMPILER_SOURCE_DIR}/capability.cc
		${DECOMPILER_SOURCE_DIR}/architecture.cc
		${DECOMPILER_SOURCE_DIR}/options.cc
		${DECOMPILER_SOURCE_DIR}/graph.cc
		${DECOMPILER_SOURCE_DIR}/cover.cc
		${DECOMPILER_SOURCE_DIR}/block.cc
		${DECOMPILER_SOURCE_DIR}/cast.cc
		${DECOMPILER_SOURCE_DIR}/typeop.cc
		${DECOMPILER_SOURCE_DIR}/database.cc
		${DECOMPILER_SOURCE_DIR}/cpool.cc
		${DECOMPILER_SOURCE_DIR}/comment.cc
		${DECOMPILER_SOURCE_DIR}/fspec.cc
		${DECOMPILER_SOURCE_DIR}/action.cc
		${DECOMPILER_SOURCE_DIR}/loadimage.cc
		${DECOMPILER_SOURCE_DIR}/varnode.cc
		${DECOMPILER_SOURCE_DIR}/op.cc
		${DECOMPILER_SOURCE_DIR}/type.cc
		${DECOMPILER_SOURCE_DIR}/variable.cc
		${DECOMPILER_SOURCE_DIR}/varmap.cc
		${DECOMPILER_SOURCE_DIR}/jumptable.cc
		${DECOMPILER_SOURCE_DIR}/emulate.cc
		${DECOMPILER_SOURCE_DIR}/emulateutil.cc
		${DECOMPILER_SOURCE_DIR}/flow.cc
		${DECOMPILER_SOURCE_DIR}/userop.cc
		${DECOMPILER_SOURCE_DIR}/funcdata.cc
		${DECOMPILER_SOURCE_DIR}/funcdata_block.cc
		${DECOMPILER_SOURCE_DIR}/funcdata_varnode.cc
		${DECOMPILER_SOURCE_DIR}/funcdata_op.cc
		${DECOMPILER_SOURCE_DIR}/pcodeinject.cc
		${DECOMPILER_SOURCE_DIR}/heritage.cc
		${DECOMPILER_SOURCE_DIR}/prefersplit.cc
		${DECOMPILER_SOURCE_DIR}/rangeutil.cc
		${DECOMPILER_SOURCE_DIR}/ruleaction.cc
		${DECOMPILER_SOURCE_DIR}/subflow.cc
		${DECOMPILER_SOURCE_DIR}/blockaction.cc
		${DECOMPILER_SOURCE_DIR}/merge.cc
		${DECOMPILER_SOURCE_DIR}/double.cc
		${DECOMPILER_SOURCE_DIR}/coreaction.cc
		${DECOMPILER_SOURCE_DIR}/condexe.cc
		${DECOMPILER_SOURCE_DIR}/override.cc
		${DECOMPILER_SOURCE_DIR}/dynamic.cc
		${DECOMPILER_SOURCE_DIR}/crc32.cc
		${DECOMPILER_SOURCE_DIR}/prettyprint.cc
		${DECOMPILER_SOURCE_DIR}/printlanguage.cc
		${DECOMPILER_SOURCE_DIR}/printc.cc
		${DECOMPILER_SOURCE_DIR}/printjava.cc
		${DECOMPILER_SOURCE_DIR}/memstate.cc
		${DECOMPILER_SOURCE_DIR}/opbehavior.cc
		${DECOMPILER_SOURCE_DIR}/paramid.cc
		${DECOMPILER_SOURCE_DIR}/transform.cc
		${DECOMPILER_SOURCE_DIR}/string_ghidra.cc
		${DECOMPILER_SOURCE_DIR}/stringmanage.cc
		# ${DECOMPILER_SOURCE_DIR}/callgraph.cc			// uncomment for debug
		# ${DECOMPILER_SOURCE_DIR}/ifacedecomp.cc		// uncomment for debug
		# ${DECOMPILER_SOURCE_DIR}/ifaceterm.cc			// uncomment for debug
		# ${DECOMPILER_SOURCE_DIR}/interface.cc			// uncomment for debug
		)

set(DECOMPILER_SOURCE_LIBDECOMP_CXX
		${DECOMPILER_SOURCE_DIR}/libdecomp.cc)

set(SLEIGH_COMPILER_LIB
		${DECOMPILER_SOURCE_DIR}/sleigh_arch.cc
		${DECOMPILER_SOURCE_DIR}/sleigh.cc
		${DECOMPILER_SOURCE_DIR}/inject_sleigh.cc
		${DECOMPILER_SOURCE_DIR}/filemanage.cc
		${DECOMPILER_SOURCE_DIR}/semantics.cc
		${DECOMPILER_SOURCE_DIR}/slghsymbol.cc
		${DECOMPILER_SOURCE_DIR}/context.cc
		${DECOMPILER_SOURCE_DIR}/sleighbase.cc
		${DECOMPILER_SOURCE_DIR}/slghpatexpress.cc
		${DECOMPILER_SOURCE_DIR}/slghpattern.cc
		${DECOMPILER_SOURCE_DIR}/pcodecompile.cc)
		
set(SLEIGH_COMPILER_SOURCE
		${DECOMPILER_SOURCE_DIR}/slgh_compile.cc
		${DECOMPILER_SOURCE_DIR}/slgh_compile.hh)
	

find_package(BISON REQUIRED)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bison")
find_package(FLEX REQUIRED)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/flex")

if(MSVC)
	add_definitions(
		/wd4099
		/wd4101
		/wd4838
		/wd4267
		/wd4146
		/wd4996
		/wd4005
		/wd4334
		/wd4293
	)
endif()

# Common static library for lib decompiler and sleigh compiler
BISON_TARGET(XML "${DECOMPILER_SOURCE_DIR}/xml.y"
			"${CMAKE_CURRENT_BINARY_DIR}/bison/xml.cpp"
			COMPILE_FLAGS  "-Dapi.prefix={xml}")
add_library(libbase STATIC ${DECOMPILER_SOURCE_BASE_CXX} ${BISON_XML_OUTPUTS})		
target_include_directories(libbase PUBLIC "${DECOMPILER_SOURCE_DIR}" )
set_property(TARGET libbase PROPERTY FOLDER "ghidra")

# Sleigh decompiler
BISON_TARGET(PCODE "${DECOMPILER_SOURCE_DIR}/pcodeparse.y"
			"${CMAKE_CURRENT_BINARY_DIR}/bison/pcodeparse.cpp"
			COMPILE_FLAGS  "-Dapi.prefix={pcodeparse}")
			
BISON_TARGET(GRAMMAR "${DECOMPILER_SOURCE_DIR}/grammar.y"
			"${CMAKE_CURRENT_BINARY_DIR}/bison/grammar.cpp"
			COMPILE_FLAGS  "-Dapi.prefix={grammar}")
			
BISON_TARGET(SLGH "${DECOMPILER_SOURCE_DIR}/slghparse.y"
			"${CMAKE_CURRENT_BINARY_DIR}/bison/slghparse.cpp"
			COMPILE_FLAGS  "-Dapi.prefix={yy}")
			
FLEX_TARGET(SLGHSCAN "${DECOMPILER_SOURCE_DIR}/slghscan.l" 
			"${CMAKE_CURRENT_BINARY_DIR}/flex/slghscan.cpp")

# Hack for dependency
ADD_FLEX_BISON_DEPENDENCY(SLGHSCAN SLGH)			
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/flex/slghparse.tab.hh" "#include \"../bison/slghparse.hpp\"")

add_library(libsleigh STATIC ${SLEIGH_COMPILER_LIB} ${BISON_PCODE_OUTPUTS} ${BISON_GRAMMAR_OUTPUTS} ${BISON_SLGH_OUTPUTS} ${FLEX_SLGHSCAN_OUTPUTS})
target_link_libraries(libsleigh libbase)
target_include_directories(libsleigh PUBLIC "${DECOMPILER_SOURCE_DIR}")
set_property(TARGET libsleigh PROPERTY FOLDER "ghidra")

add_executable(sleighc ${SLEIGH_COMPILER_SOURCE})
target_link_libraries(sleighc libbase libsleigh)
target_include_directories(sleighc PRIVATE "${DECOMPILER_SOURCE_DIR}")
set_property(TARGET sleighc PROPERTY FOLDER "ghidra")


# Static library decompiler
add_library(libdecomp STATIC ${DECOMPILER_SOURCE_LIBDECOMP_CXX})
target_link_libraries(libdecomp libbase libsleigh)
target_include_directories(libdecomp PUBLIC "${DECOMPILER_SOURCE_DIR}")
set_property(TARGET libdecomp PROPERTY FOLDER "ghidra")

include(CmakeGenerateSLA)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/sleigh")
set(SLEIGH_DIR "${CMAKE_CURRENT_BINARY_DIR}/sleigh")

# Build processor specification
# Install all source file
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ghidra/Ghidra/Processors/" DESTINATION "Processors")

# compile file for x86
SLA_TARGET(sla_x86 "${CMAKE_CURRENT_SOURCE_DIR}/ghidra/Ghidra/Processors/x86/data/languages/x86.slaspec" "${SLEIGH_DIR}/x86.sla")
set_property(TARGET sla_x86 PROPERTY FOLDER "sleigh")
install(FILES "${SLEIGH_DIR}/x86.sla" DESTINATION "Processors/x86/data/languages/")

# compile file for x86_64
SLA_TARGET(sla_x86_64 "${CMAKE_CURRENT_SOURCE_DIR}/ghidra/Ghidra/Processors/x86/data/languages/x86-64.slaspec" "${SLEIGH_DIR}/x86-64.sla")
set_property(TARGET sla_x86_64 PROPERTY FOLDER "sleigh")
install(FILES "${SLEIGH_DIR}/x86-64.sla" DESTINATION "Processors/x86/data/languages/")
