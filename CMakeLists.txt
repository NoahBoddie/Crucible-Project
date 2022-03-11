cmake_minimum_required(VERSION 3.20)

# ---- Options ----

option(COPY_BUILD "Copy the build output to the Skyrim directory." OFF)

# ---- Cache build vars ----

macro(set_from_environment VARIABLE)
	if (NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
		set(${VARIABLE} $ENV{${VARIABLE}})
	endif ()
endmacro()

set_from_environment(Skyrim64Path)
set_from_environment(VCPKG_ROOT)
set_from_environment(CommonLibSSEPath)

if (DEFINED VCPKG_ROOT)
	#set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
	#set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
else ()
	message(
		WARNING
		"Variable VCPKG_ROOT is not set. Continuing without vcpkg."
	)
endif ()

set(Boost_USE_STATIC_RUNTIME OFF CACHE BOOL "")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")

# ---- Project ----

project(
	Crucible
	VERSION 1.0.0
	LANGUAGES CXX
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
	message(
		FATAL_ERROR
			"In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
	)
endif()

# ---- Globals ----

if (MSVC)
	add_compile_definitions(
		_UNICODE
	)

	if (NOT ${CMAKE_GENERATOR} STREQUAL "Ninja")
		add_compile_options(
			/MP	# Build with Multiple Processes
			#/MT # Static linking?
		)
	endif ()
endif ()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

set(Boost_USE_STATIC_LIBS ON)


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MTd")

# ---- Dependencies ----

#add_subdirectory("extern/CommonLibSSE" CommonLibSSE)
if (DEFINED CommonLibSSEPath AND NOT ${CommonLibSSEPath} STREQUAL "" AND IS_DIRECTORY ${CommonLibSSEPath})
	add_subdirectory(${CommonLibSSEPath} CommonLibSSE)
else ()
	message(
		FATAL_ERROR
		"Variable CommonLibSSEPath is not set."
	)
endif ()


find_package(spdlog REQUIRED CONFIG)
find_package(xbyak REQUIRED CONFIG)

set(spdlog_USE_STATIC_LIBS ON)


# ---- Add source files ----

set(SOURCES
	src/main.cpp
	src/PCH.h
	src/ActionStat.h
	src/ActionStatHandler.h
	src/ActionPointController.h
	src/ActionPointController.cpp
	src/CombatData.h 
	src/CombatData.cpp
	src/Combo.h 
	src/Combo.cpp 
	src/CollisionIgnoreHandler.h 
	src/PlayerData.h 
	src/PlayerData.cpp 
	src/Papyrus.h 
	src/Papyrus.cpp 
	src/Resources.h 
	src/Timer.h
	src/Timer.cpp
	src/TimeManager.h
	src/TimeManager.cpp
	src/CompetitiveEvent.h 
	src/StatedObject.h 
	src/ChainObject.h 
	src/CombatDataBehaviour.h 
	src/CombatDataBehaviour.cpp 
	src/WeaponSwing.h 
	src/WeaponSwing.cpp
	"src/ItemStatHandler.h"
	"src/ItemStatHandler.cpp"
	src/ItemStats.h
	src/ItemStats.cpp
	src/ItemTaskHandlers.h
	src/ItemTaskHandlers.cpp
	src/AttackStatController.h
	src/AttackStatController.cpp
	src/MotionStatController.h 
	src/MotionStatController.cpp
	src/BlockStatController.h 
	src/BlockStatController.cpp
	src/MagicStatController.h 
	src/MagicStatController.cpp
	src/Utility.h 
	src/Hooks.h 
	src/Hooks.cpp
	src/Events.h 
	src/Events.cpp
	src/EffectManager.h
	src/EquipUtilities.h
	src/EquipUtilities.cpp
	src/CombatValue.h
	src/CombatValue.cpp
	src/GlobalEffectHolder.h
	src/GlobalEffectHolder.cpp
	src/GlobalResistEffect.h
	src/GlobalStaggerEffect.h
	src/CombatValueEffect.h
	src/CombatValueEffect.cpp
	src/CombatValueOwner.h
	src/CombatValueOwner.cpp
	src/ActorCombatValueOwner.h
	src/ActorCombatValueOwner.cpp
	src/QueryContext.h
	src/QueryContext.cpp
	src/ActorOwner.h
	src/ActorOwner.cpp
	src/ModSettings.h
	src/ModSettings.cpp
	src/RecoverySettings.h
	src/StrikeSettings.h
	src/ModSettingHandler.h
	src/SerializableObject.h
	src/SerialConstructor.h
	src/SerialArgument.h
	src/SerialIterator.h
	src/SerializablePair.h
	src/SerializableList.h
	src/SerializableMap.h
	src/SimpleSerializable.h
	src/ComplexSerializable.h
	#src/TestField.h
	 "src/SpecialStrikeHandler.h" "src/CombatActorProcess.h" "src/CombatResource.h" "src/CombatResourceOwner.h" "src/CombatResourceOwner.cpp" "src/PoiseController.h" "src/PoiseController.cpp" "src/RecoveryController.cpp" "src/EventObject.h" "src/EventSystem.h" "src/ReadableValue.h" "src/ReadableValue.cpp" "src/CombatEffect.h" "src/GlobalEffects.h" "src/FormulaHandler.h" "src/FormulaHandler.cpp" "src/SpecialStrikeController.h")

source_group(
	TREE
		${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${SOURCES}
)

source_group(
	TREE
		${CMAKE_CURRENT_BINARY_DIR}
	FILES
		${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
)

# ---- Create DLL ----

add_library(
	${PROJECT_NAME}
	SHARED
	${SOURCES}
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	.clang-format
	.editorconfig
 "src/SpecialStrikeHandler.h" "src/CombatActorProcess.h" "src/CombatResource.h" "src/CombatResourceOwner.h" "src/CombatResourceOwner.cpp" "src/PoiseController.h" "src/PoiseController.cpp" "src/RecoveryController.cpp" "src/EventObject.h" "src/EventSystem.h" "src/ReadableValue.h" "src/ReadableValue.cpp" "src/CombatEffect.h" "src/GlobalEffects.h" "src/FormulaHandler.h" "src/FormulaHandler.cpp")

target_compile_features(
	${PROJECT_NAME}
	PRIVATE
		cxx_std_20
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		CommonLibSSE::CommonLibSSE
		spdlog::spdlog
		xbyak::xbyak
)

if (MSVC)
	target_compile_options(
		${PROJECT_NAME}
		PRIVATE
			/sdl	# Enable Additional Security Checks
			/utf-8	# Set Source and Executable character sets to UTF-8
			/Zi	# Debug Information Format

			/permissive-	# Standards conformance

			/Zc:alignedNew	# C++17 over-aligned allocation
			/Zc:auto	# Deduce Variable Type
			/Zc:char8_t
			/Zc:__cplusplus	# Enable updated __cplusplus macro
			/Zc:externC
			/Zc:externConstexpr	# Enable extern constexpr variables
			/Zc:forScope	# Force Conformance in for Loop Scope
			/Zc:hiddenFriend
			/Zc:implicitNoexcept	# Implicit Exception Specifiers
			/Zc:lambda
			/Zc:noexceptTypes	# C++17 noexcept rules
			/Zc:preprocessor	# Enable preprocessor conformance mode
			/Zc:referenceBinding	# Enforce reference binding rules
			/Zc:rvalueCast	# Enforce type conversion rules
			/Zc:sizedDealloc	# Enable Global Sized Deallocation Functions
			/Zc:strictStrings	# Disable string literal type conversion
			/Zc:ternary	# Enforce conditional operator rules
			/Zc:threadSafeInit	# Thread-safe Local Static Initialization
			/Zc:tlsGuards
			/Zc:trigraphs	# Trigraphs Substitution
			/Zc:wchar_t	# wchar_t Is Native Type

			/external:anglebrackets
			/external:W0

			/W4	# Warning level
			#/WX	# Warning level (warnings are errors)

			"$<$<CONFIG:DEBUG>:>"
			"$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
	)

	target_link_options(
		${PROJECT_NAME}
		PRIVATE
			#/WX	# Treat Linker Warnings as Errors

			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
	)
endif ()

target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
		src/PCH.h
)

# ---- Post build ----

if (COPY_BUILD)
	if (DEFINED Skyrim64Path)
		add_custom_command(
			TARGET ${PROJECT_NAME}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${Skyrim64Path}/Data/SKSE/Plugins/
			COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> ${Skyrim64Path}/Data/SKSE/Plugins/
		)
	else ()
		message(
			WARNING
			"Variable Skyrim64Path is not defined. Skipping post-build copy command."
		)
	endif ()
endif ()
