enable_testing()
find_package(GTest REQUIRED)

add_executable(foo foo.cc)
target_link_libraries(foo GTest::GTest GTest::Main)

add_test(AllTestsInFoo foo)
