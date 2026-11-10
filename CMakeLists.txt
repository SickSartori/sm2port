cmake_minimum_required(VERSION 3.25)
project(sm2port VERSION 0.1.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static CRT everywhere: the runtime DLL must not depend on redistributables.
# Read when targets are created, so it has to come before them.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# The configuration with symbols inlines like Release, so that what is measured and tried is what ships.
string(REPLACE "/Ob1" "/Ob2" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
string(REPLACE "/Ob1" "/Ob2" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")

# The shared runtime (private submodule, external/sick-xport): the Xbox kernel, memory, threads, the NV2A's
# registers and textures, paths, logs; xportcore is its portable part (discs, XBE, kernel exports).
include(external/sick-xport/xport.cmake)

if(WIN32)
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
        message(FATAL_ERROR "The PC build must be 32-bit (x86): the game's x86 code runs natively in the process. "
                            "Configure with -A Win32 or use the 'pc-x86' preset.")
    endif()

    # Version numbers for the version resource (res/version.rc) of sm2.exe and sm2runtime.dll.
    set(SM2_VERSION_DEFINES SM2_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} SM2_VERSION_MINOR=${PROJECT_VERSION_MINOR}
                            SM2_VERSION_PATCH=${PROJECT_VERSION_PATCH})

    # Dear ImGui (MIT, third_party/imgui): the launcher's interface.
    add_library(imgui STATIC
        third_party/imgui/imgui.cpp
        third_party/imgui/imgui_draw.cpp
        third_party/imgui/imgui_tables.cpp
        third_party/imgui/imgui_widgets.cpp
        third_party/imgui/backends/imgui_impl_dx11.cpp
        third_party/imgui/backends/imgui_impl_win32.cpp
    )
    target_include_directories(imgui PUBLIC third_party/imgui)
    # The launcher polls the controllers itself (every port, not only the first): see Launcher::poll_gamepads.
    target_compile_definitions(imgui PRIVATE NOMINMAX IMGUI_IMPL_WIN32_DISABLE_GAMEPAD)

    # SDL3 (zlib): only its gamepad side, for Xbox, PlayStation, Switch and other controllers (src/pc/gamepad.cpp).
    # Downloaded at configure time, pinned by version and hash, linked statically.
    include(FetchContent)
    FetchContent_Declare(SDL3
        URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.20/SDL3-3.4.20.tar.gz
        URL_HASH SHA256=f3bb06f55bed9557227a700af2ea62e49497ebdf58037732a731a5fb3ef8581f
    )
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
    foreach(subsystem AUDIO VIDEO HAPTIC POWER SENSOR DIALOG)
        set(SDL_${subsystem} OFF CACHE BOOL "" FORCE)
    endforeach()
    FetchContent_MakeAvailable(SDL3)

    # Runtime: maps the XBE, emulates the Xbox kernel and hosts the game's threads.
    add_library(sm2runtime SHARED
        ${XPORT_RUNTIME_SOURCES}
        src/pc/audio_mixer.cpp
        src/pc/dsound_hle.cpp
        src/pc/game_disc.cpp
        src/pc/hle_patches.cpp
        src/pc/input_hle.cpp
        src/pc/kernel_io.cpp
        src/pc/kernel_threads.cpp
        src/pc/keyboard_bindings.cpp
        src/pc/launcher_ui.cpp
        src/pc/launcher_web.cpp
        src/pc/nv2a.cpp
        src/pc/nv2a_kelvin.cpp
        src/pc/renderer_d3d11.cpp
        src/pc/runtime.cpp
        src/pc/settings.cpp
        res/launcher.rc
    )
    target_link_libraries(sm2runtime PRIVATE xportcore shell32 user32 gdi32 winmm ole32 xaudio2 d3d11 dxgi d3dcompiler dbghelp dwmapi bcrypt comdlg32 winhttp windowscodecs imgui mfplat mfreadwrite mfuuid shlwapi SDL3::SDL3-static)
    target_compile_definitions(sm2runtime PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE
                               SM2PORT_VERSION="${PROJECT_VERSION}" ${SM2_VERSION_DEFINES})
    target_compile_options(sm2runtime PRIVATE /utf-8)
    target_include_directories(sm2runtime PRIVATE src res)
    xport_identity(sm2runtime sm2port SM2PORT SM2_)
    # Started by RecompLauncher, the data stays next to sm2.exe (since 0.1.2).
    target_compile_definitions(sm2runtime PRIVATE XPORT_DATA_WITH_RECOMPLAUNCHER)
    # The GPU thread's hot path crosses files (methods, physical memory, renderer).
    set_target_properties(sm2runtime PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE ON
                                                INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)

    # Launcher: linked at the XBE base (0x10000) so Windows reserves the game's address range for us.
    # It has no CRT; runtime checks and security cookies would pull one in.
    string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    add_executable(sm2 src/pc/launcher.c res/sm2.rc)
    target_compile_definitions(sm2 PRIVATE ${SM2_VERSION_DEFINES} SM2_RC_APP=1)
    target_compile_options(sm2 PRIVATE /GS-)
    target_link_libraries(sm2 PRIVATE kernel32 user32)
    target_link_options(sm2 PRIVATE
        /NODEFAULTLIB /ENTRY:LauncherEntry /SUBSYSTEM:WINDOWS
        /BASE:0x10000 /FIXED /DYNAMICBASE:NO /LARGEADDRESSAWARE
        /SAFESEH:NO /INCREMENTAL:NO
    )
    add_dependencies(sm2 sm2runtime)

    # Tests: ctest --test-dir build/pc-x86 -C Release. They need no game file; disc_real uses SM2_TEST_DISC if set.
    include(CTest)
    if(BUILD_TESTING)
        function(sm2_test name)
            add_executable(${name} tests/${name}.cpp ${ARGN})
            target_link_libraries(${name} PRIVATE xportcore)
            target_include_directories(${name} PRIVATE src tests ${XPORT_DIR}/tests)
            target_compile_definitions(${name} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
            target_compile_options(${name} PRIVATE /utf-8)
            set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
            add_test(NAME ${name} COMMAND ${name})
        endfunction()
        sm2_test(test_point_quads)
        sm2_test(test_game_disc src/pc/game_disc.cpp)
        sm2_test(test_settings src/pc/settings.cpp ${XPORT_DIR}/src/pc/log.cpp)
        xport_add_tests(sm2port SM2PORT SM2_)
        target_link_libraries(test_game_disc PRIVATE bcrypt comdlg32 shell32 user32)
        add_test(NAME disc_real COMMAND test_game_disc real)
        set_tests_properties(disc_real PROPERTIES SKIP_RETURN_CODE 77)
    endif()
endif()
