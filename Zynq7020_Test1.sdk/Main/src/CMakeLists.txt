
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/lscript.ld)
file(GLOB_RECURSE SOURCES ./*.c ./*.h)
add_executable(Main.elf ${SOURCES} ${LINKER_SCRIPT})

target_include_directories(Main.elf PUBLIC
        "Drivers"
        "ThirdParty/LVGL"
        "ThirdParty/LVGL/src/font"
        "ThirdParty/LVGL/src/widgets"
        "ThirdParty/CJSON"
        "LVGL_App"
        )

target_compile_definitions(Main.elf PUBLIC
        __USE_RTOS
        LV_LVGL_H_INCLUDE_SIMPLE
        LV_DEMO_CONF_INCLUDE_SIMPLE
        TFTP_MAX_MODE_LEN=32
        TFTP_MAX_FILENAME_LEN=512
        IN_CLION)

target_link_libraries(Main.elf PUBLIC MainBsp c gcc m arm_math)
target_link_directories(Main.elf PUBLIC ${CMAKE_SOURCE_DIR}/Main_bsp/ps7_cortexa9_0/lib cmake-build-debug-mingw-arm-none-eabi-gcc/Main_bsp)
target_link_options(Main.elf PUBLIC
        --specs=Xilinx.spec
        -Wl,-gc-sections,--print-memory-usage,-Map=Main.map
        -T ${LINKER_SCRIPT}
        )


set(HEX_FILE ${PROJECT_BINARY_DIR}/Main.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/Main.bin)

add_custom_command(TARGET Main.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:Main.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:Main.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")
