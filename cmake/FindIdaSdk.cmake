

if(IDA_SDK_SOURCE_DIR)
	find_path(IDA_SDK_SOURCE_ROOT
	NAMES include/ida.hpp
	PATHS "${IDA_SDK_SOURCE_DIR}"
	NO_DEFAULT_PATH)
else()
	set(IDA_SDK_SOURCE_ROOT IDA_SDK_SOURCE_ROOT-NOTFOUND)
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    IDA_SDK
	REQUIRED_VARS IDA_SDK_SOURCE_ROOT
    FAIL_MESSAGE "
#######################################################
Could not find IDA SDK headers. Make sure IDA_SDK_SOURCE_DIR is set to the root of the IDA SDK source repository.
#######################################################
")

mark_as_advanced(IDA_SDK_MAIN_HEADER)

if(IDA_SDK_FOUND)
	set(IDA_SDK_INCLUDE_DIRS "${IDA_SDK_SOURCE_ROOT}/include")
	set(IDA_SDK_LIBS "${IDA_SDK_SOURCE_ROOT}/lib/x64_win_vc_64/ida.lib")
endif()