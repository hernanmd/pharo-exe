cmake_minimum_required(VERSION 3.0.0)
project(PharoExe VERSION 0.1.0)

include(CTest)
enable_testing()

#---------------------------------------------------------------------------------------------#
# Compiler settings
#---------------------------------------------------------------------------------------------#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Set some compiller flags: just to include debugging symbols. 
set(CMAKE_C_FLAGS "-g")
set(CMAKE_CXX_FLAGS "-g")

#---------------------------------------------------------------------------------------------#
# Generate the image
#---------------------------------------------------------------------------------------------#
make_directory(${CMAKE_CURRENT_BINARY_DIR}/build/vm)
# Resources will contain the Pharo.image
make_directory(${CMAKE_CURRENT_BINARY_DIR}/build/resources)

#---------------------------------------------------------------------------------------------#
# Download images and vm
#---------------------------------------------------------------------------------------------#
# Adding a task to download the vm
add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/build/vm/pharo
	COMMAND wget -O - https://get.pharo.org/64/vmHeadlessLatest90 | bash
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build/vm
	COMMENT "Downloading Pharo 90 VM (headless)")

# Adding a task to download the image
add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/build/resources/Pharo.image
	COMMAND wget -O - https://get.pharo.org/64/110 | bash
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build/resources
	COMMENT "Downloading Pharo 11 Image")

# Download the header files and to extract the libs from the VM. 
# /${OS_NAME}-${OS_PLATFORM}/include....
  add_custom_command(
    OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/build/include/pharovm/pharo.h
    COMMAND wget http://files.pharo.org/vm/pharo-spur64-headless/Darwin-arm64/include/latest10.zip
    COMMAND unzip latest10.zip
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build
    COMMENT "Downloading Pharo 110 VM Headers")

# Extract the libs from the VM. 
add_custom_command(
	OUTPUT  
		# ${CMAKE_CURRENT_BINARY_DIR}/build/lib/libPharoVMCore.a
    ${CMAKE_CURRENT_BINARY_DIR}/build/output/libPharoVMCore.dylib	
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/build/lib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/build/output
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/build/vm/pharo-vm/*.dylib ${CMAKE_CURRENT_BINARY_DIR}/build/lib
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/build/vm/pharo-vm/*.dylib ${CMAKE_CURRENT_BINARY_DIR}/build/output
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/build/vm/pharo-vm/*.a ${CMAKE_CURRENT_BINARY_DIR}/build/lib	
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build
    DEPENDS 
		${CMAKE_CURRENT_BINARY_DIR}/build/vm/pharo
		${CMAKE_CURRENT_BINARY_DIR}/build/include/pharovm/pharo.h
	COMMENT "Extracting Libs")

#Setting the include directories
# - The includes in this project
# - The includes from the VM code
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/build/include/)

# This are the files to compile in our small project.
set(CLIENT_DEPS
    # ${CMAKE_CURRENT_BINARY_DIR}/build/lib/libPharoVMCore.a
    ${CMAKE_CURRENT_BINARY_DIR}/build/output/libPharoVMCore.dylib
    # ${CMAKE_CURRENT_BINARY_DIR}/build/resources/codeLoaded.txt
)

set(CLIENT_SOURCES main.cpp)

#---------------------------------------------------------------------------------------------#
# Download Include Binary header library
#---------------------------------------------------------------------------------------------#
macro(Download_Single_Headerlib FILE URL)
  file(DOWNLOAD ${URL} ${CMAKE_SOURCE_DIR}/include/${FILE})
  IF(NOT Download_Single_Headerlib_flag)
    include_directories(${CMAKE_SOURCE_DIR}/include)
    set(Download_Single_Headerlib_flag TRUE)
  ENDIF()
endmacro()

Download_Single_Headerlib(
  incbin.h
  "https://raw.githubusercontent.com/graphitemaster/incbin/8cefe46d5380bf5ae4b4d87832d811f6692aae44/incbin.h"
  )
include_directories(${CMAKE_CURRENT_LIST_DIR})
#--------- Main Target -------------#

add_executable(PharoExe ${CLIENT_DEPS} ${CLIENT_SOURCES})

# Link the executable with the VM library.
target_link_directories(PharoExe PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/build/lib/)
target_link_libraries(PharoExe PharoVMCore)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
