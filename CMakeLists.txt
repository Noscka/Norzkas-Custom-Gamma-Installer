cmake_minimum_required(VERSION 3.8)
project ("NCGI" VERSION 1.0.0 LANGUAGES CXX)

# Set Options and Properties
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

set(BIT7Z_AUTO_FORMAT ON CACHE BOOL "")
set(BIT7Z_REGEX_MATCHING ON CACHE BOOL "")
set(BIT7Z_GENERATE_PIC ON CACHE BOOL "")
set(BIT7Z_STATIC_RUNTIME ON CACHE BOOL "")
set(BIT7Z_USE_NATIVE_STRING ON CACHE BOOL "")
set(BIT7Z_AUTO_PREFIX_LONG_PATHS  ON CACHE BOOL "")

# Run Library CMakes
add_subdirectory(External)

# Find QT Lib
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
)
qt_standard_project_setup()

# Get All Project Files
file(GLOB_RECURSE ProjectFiles CONFIGURE_DEPENDS "Headers/*" "Source/*" "CustomWidgets/*" "InstallerWindow/*")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    enable_language("RC")
    set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/Resource/Icon.rc")
endif()

qt_add_executable(${PROJECT_NAME} "main.cpp" ${ProjectFiles} ${app_icon_resource_windows})

qt_add_resources(${PROJECT_NAME} "Icon"
    FILES
       "Resource/icon.ico"
)

file(GLOB_RECURSE ThemeFiles RELATIVE "${PROJECT_SOURCE_DIR}" CONFIGURE_DEPENDS
    "Assets/Theme/vs15/*"    
)

qt_add_resources(${PROJECT_NAME} "Assets"
BASE "Assets/"
    FILES
       "Assets/Images/Logo.png"
       "Assets/Theme/vs15 Dark-Red.qss"
       ${ThemeFiles}
)

# Make console invisible
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

# Link Libraries
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
)
target_link_libraries(${PROJECT_NAME} PRIVATE -static NosLib)
target_link_libraries(${PROJECT_NAME} PRIVATE -static bit7z)

# Add Compiler Definitions
add_compile_definitions(CPPHTTPLIB_OPENSSL_SUPPORT)
add_compile_definitions(WIN32_LEAN_AND_MEAN)

# Use Unicode
if (MSVC)
     target_compile_definitions(${PROJECT_NAME} PUBLIC UNICODE _UNICODE)
endif()

# Out Build Path
set(OUTDIR ${PROJECT_SOURCE_DIR}/Out/)

message(STATUS "POST BUILD OUT PATH: ${OUTDIR}")

add_custom_command(
   TARGET ${PROJECT_NAME} POST_BUILD 
   COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTDIR}
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${OUTDIR}
)

if (CMAKE_IMPORT_LIBRARY_SUFFIX)
  add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t ${OUTDIR} "$<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>"
  COMMAND_EXPAND_LISTS
  )

  add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND "windeployqt" --release --compiler-runtime "${OUTDIR}/NCGI.exe"
  )
endif ()