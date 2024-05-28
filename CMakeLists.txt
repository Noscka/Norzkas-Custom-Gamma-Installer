cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project ("NCGI" VERSION 1.0.0 LANGUAGES CXX)

set(BIT7Z_AUTO_FORMAT ON CACHE BOOL "")
set(BIT7Z_REGEX_MATCHING ON CACHE BOOL "")
set(BIT7Z_GENERATE_PIC ON CACHE BOOL "")
set(BIT7Z_STATIC_RUNTIME ON CACHE BOOL "")
set(BIT7Z_USE_NATIVE_STRING ON CACHE BOOL "")
set(BIT7Z_AUTO_PREFIX_LONG_PATHS  ON CACHE BOOL "")

add_subdirectory(External)

file(GLOB_RECURSE ProjectFiles CONFIGURE_DEPENDS "Headers/*")

add_executable (${PROJECT_NAME} "main.cpp" ${ProjectFiles})

target_link_libraries(${PROJECT_NAME} PRIVATE -static NosLib)
target_link_libraries(${PROJECT_NAME} PRIVATE -static bit7z)

add_compile_definitions(CPPHTTPLIB_OPENSSL_SUPPORT)
add_compile_definitions(WIN32_LEAN_AND_MEAN)

if (MSVC)
     target_compile_definitions(${PROJECT_NAME} PUBLIC UNICODE _UNICODE)
 endif()

set(OUTDIR ${PROJECT_SOURCE_DIR}/Out/)

message(STATUS "POST BUILD OUT PATH: ${OUTDIR}")

add_custom_command(
   TARGET ${PROJECT_NAME} POST_BUILD 
   COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTDIR}
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${OUTDIR}
)

#if (CMAKE_IMPORT_LIBRARY_SUFFIX)
#  add_custom_command(
#    TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy -t ${OUTDIR} "$<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>"
#  COMMAND_EXPAND_LISTS
#  )
#
#  add_custom_command(
#    TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND "windeployqt" "${OUTDIR}/CarSearch.exe"
#  )
#endif ()