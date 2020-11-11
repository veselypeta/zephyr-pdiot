# Find Zephyr. This also loads Zephyr's build system.
cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr)
project(pdiot)

set(CMAKE_CPP_COMPILER g++)
set(BOARD thingy52_nrf52832)

# Add your source file to the "app" target. This must come after
# find_package(Zephyr) which defines the target.
target_sources(app PRIVATE src/main.cpp src/gyro_lite_model.cpp src/neural_network.cpp src/accelerometer.cpp )


# --- TFLITE --- 

zephyr_include_directories(src)


# Set the target and BOARD
set(TARGET ${BOARD})
set(TARGET_ARCH cortex-m4)


set(TF_SRC_DIR  /Users/petrvesely/app/tensorflow)
set(TF_MAKE_DIR ${TF_SRC_DIR}/tensorflow/lite/micro/tools/make)
set(TF_LIB_DIR ${TF_MAKE_DIR}/gen/${TARGET}_${TARGET_ARCH}/lib)
set(extra_project_flags "-mcpu=${TARGET_ARCH} -mthumb -mno-thumb-interwork -mfpu=fpv5-sp-d16")


zephyr_get_include_directories_for_lang_as_string(       C C_includes)
zephyr_get_system_include_directories_for_lang_as_string(C C_system_includes)
zephyr_get_compile_definitions_for_lang_as_string(       C C_definitions)
zephyr_get_compile_options_for_lang_as_string(           C C_options)

set(external_project_cflags
  "${C_includes} ${C_definitions} ${optC_optionsions} ${C_system_includes} ${extra_project_flags}"
)

zephyr_get_include_directories_for_lang_as_string(       CXX CXX_includes)
zephyr_get_system_include_directories_for_lang_as_string(CXX CXX_system_includes)
zephyr_get_compile_definitions_for_lang_as_string(       CXX CXX_definitions)
zephyr_get_compile_options_for_lang_as_string(           CXX CXX_options)

set(external_project_cxxflags
  "${CXX_includes} ${CXX_definitions} ${CXX_options} ${CXX_system_includes} ${extra_project_flags}"
)

include(ExternalProject)


if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
# https://www.gnu.org/software/make/manual/html_node/MAKE-Variable.html
set(submake "$(MAKE)")
else() # Obviously no MAKEFLAGS. Let's hope a "make" can be found somewhere.
set(submake "make")
endif()


ExternalProject_Add(
  tf_project                 # Name for custom target
  SOURCE_DIR ${TF_SRC_DIR}
  BINARY_DIR ${TF_SRC_DIR} # This particular build system is invoked from the root
  CONFIGURE_COMMAND ""    # Skip configuring the project, e.g. with autoconf
  BUILD_COMMAND ${submake} -f tensorflow/lite/micro/tools/make/Makefile
  TARGET=${TARGET}
  TARGET_ARCH=${TARGET_ARCH}
  TARGET_TOOLCHAIN_ROOT=${GNUARMEMB_TOOLCHAIN_PATH}/bin/
  TARGET_TOOLCHAIN_PREFIX=arm-none-eabi-
  CC=${CMAKE_C_COMPILER}
  CXX=${CMAKE_CXX_COMPILER}
  AR=${CMAKE_AR}
  CCFLAGS=${external_project_cflags} 
  CXXFLAGS=${external_project_cxxflags} 
  microlite
  INSTALL_COMMAND ""      # This particular build system has no install command
  BUILD_BYPRODUCTS ${TF_LIB_DIR}/libtensorflow-microlite.a
  )

# Create a wrapper CMake library that our app can link with
add_library(tf_lib STATIC IMPORTED GLOBAL)

add_dependencies(
  tf_lib
  tf_project
  )
set_target_properties(tf_lib PROPERTIES IMPORTED_LOCATION             ${TF_LIB_DIR}/libtensorflow-microlite.a)
set_target_properties(tf_lib PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${TF_SRC_DIR};${TF_SRC_DIR}/tensorflow/lite/micro;${TF_MAKE_DIR}/downloads/flatbuffers/include") 

target_link_libraries(app PUBLIC tf_lib)