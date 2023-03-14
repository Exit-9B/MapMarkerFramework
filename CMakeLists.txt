cmake_minimum_required(VERSION 3.24)

project(
	MapMarkerFramework
	VERSION 2.2.0
	LANGUAGES CXX
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_IGNORE_PATH "${PROJECT_BINARY_DIR}")
list(APPEND CMAKE_MODULE_PATH
	"${PROJECT_SOURCE_DIR}/tools/SKSE-CMakeModules/Modules"
)
include(SKSEPlugin)

set(SKSE_USE_XBYAK ON)
set(SKSE_SUPPORT_VR OFF)

SKSEPlugin_Add(
	${PROJECT_NAME}
	INCLUDE_DIR include
	SOURCE_DIR src
	SOURCES
		.clang-format
		.editorconfig
		README.md
		vcpkg.json
	PRECOMPILE_HEADERS
		src/PCH/PCH.h
)

add_subdirectory("external/FlashLibSkyrim" FlashLibSkyrim EXCLUDE_FROM_ALL)

find_package(jsoncpp REQUIRED)
find_package(tsl-ordered-map CONFIG REQUIRED)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		FlashLibSkyrim
		jsoncpp_static
		tsl::ordered_map
)

set(CPACK_GENERATOR "7Z")
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION})
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
include(CPack)
