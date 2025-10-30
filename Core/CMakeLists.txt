
add_library(Core INTERFACE)

target_include_directories(Core INTERFACE Inc)
target_compile_definitions(Core INTERFACE
	USE_FULL_LL_DRIVER 
	PY32F030x8 
    $<$<CONFIG:Debug>:DEBUG>
)

target_sources(Core INTERFACE
    Src/system_py32f0xx.c
    Src/main.c
    Src/py32f0xx_it.c
    # Src/sysmem.c
    # Src/syscalls.c
    Src/startup_py32f030xx.s
)

target_link_libraries(Core INTERFACE PY32_Drivers)
target_link_options(Core INTERFACE "-T" "${CMAKE_CURRENT_SOURCE_DIR}/py32f030x8.ld")
