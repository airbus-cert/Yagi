enable_testing()

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(yagi_TEST_INCLUDE
	mock_logger_test.h
	mock_symbol_test.h
	mock_loader_test.h
	mock_type_test.h
	mock_type_test.cc
)

add_executable(
  x86_payload_1_test
  x86_payload_1_test.cc
  ${yagi_TEST_INCLUDE}
)

target_link_libraries(
  x86_payload_1_test
  gtest_main
  yagi_static
)

target_compile_features(x86_payload_1_test PRIVATE cxx_std_17)

# This is a trick for compiling on visual studio
# static initializer are stripped because libdecomp are a static library
# and no PrintLanguage are registred because singleton ctor are never called
target_link_options(x86_payload_1_test PRIVATE /WHOLEARCHIVE:libbase.lib)

include(GoogleTest)
gtest_discover_tests(x86_payload_1_test)