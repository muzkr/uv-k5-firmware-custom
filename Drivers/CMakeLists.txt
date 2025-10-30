
add_library(PY32_Drivers INTERFACE)

target_include_directories(PY32_Drivers SYSTEM INTERFACE
    ${CMAKE_SOURCE_DIR}/PY32F0xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/CMSIS/Device/PY32F0xx/Include
    ${CMAKE_SOURCE_DIR}/CMSIS/Include
)

target_sources(PY32_Drivers INTERFACE
    ${CMAKE_SOURCE_DIR}/PY32F0xx_HAL_Driver/Src/py32f0xx_ll_utils.c
    ${CMAKE_SOURCE_DIR}/PY32F0xx_HAL_Driver/Src/py32f0xx_ll_exti.c
    ${CMAKE_SOURCE_DIR}/PY32F0xx_HAL_Driver/Src/py32f0xx_ll_gpio.c
    ${CMAKE_SOURCE_DIR}/PY32F0xx_HAL_Driver/Src/py32f0xx_ll_adc.c
    ${CMAKE_SOURCE_DIR}/PY32F0xx_HAL_Driver/Src/py32f0xx_ll_dma.c
    ${CMAKE_SOURCE_DIR}/PY32F0xx_HAL_Driver/Src/py32f0xx_ll_rcc.c
    ${CMAKE_SOURCE_DIR}/PY32F0xx_HAL_Driver/Src/py32f0xx_ll_crc.c
    ${CMAKE_SOURCE_DIR}/PY32F0xx_HAL_Driver/Src/py32f0xx_ll_usart.c
    ${CMAKE_SOURCE_DIR}/PY32F0xx_HAL_Driver/Src/py32f0xx_ll_tim.c
)
