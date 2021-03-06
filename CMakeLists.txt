cmake_minimum_required(VERSION 3.0.2)
project(CarnaDICOM)

set(MAJOR_VERSION   1)
set(MINOR_VERSION   0)
set(PATCH_VERSION   1)

############################################

set(CMAKE_DEBUG_POSTFIX d)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(FULL_VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})
set(TARGET_NAME ${PROJECT_NAME}-${FULL_VERSION})
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_CAPS)

############################################
# Set default options for this build
############################################

option(BUILD_DOC    "Build and install the API documentation" OFF)
option(BUILD_TEST   "Build the unit tests" OFF)
option(BUILD_DEMO	"Build interactive demos" OFF)

############################################
# Macro that sets variable to default value
# only when the variable isn't defined yet
############################################

macro( option_default_to var_name default_val var_type doc_string )
    if( NOT DEFINED ${var_name} )
        set(${var_name} ${default_val})
    endif()
    set(${var_name} ${${var_name}} CACHE ${var_type} ${doc_string} FORCE)
endmacro()

############################################
# Locate Find<ModuleName>.cmake scripts
############################################

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/misc/CMake-Modules)

############################################
# Run MediTEC compatiblity script
############################################
SET(CMAKE_INSTALL_PREFIX "$ENV{MEDITEC_LIBS}")
  MESSAGE("********************************************************")
  MESSAGE(${CMAKE_INSTALL_PREFIX})
add_subdirectory(misc/MediTEC)

############################################
# Define default paths for the installation
############################################

# set default library and header destinations (relative to CMAKE_INSTALL_PREFIX)
option_default_to(INSTALL_LIBRARY_DIR "lib"                       String "Installation directory for libraries")
option_default_to(INSTALL_HEADERS_DIR "include"                   String "Installation directory for headers")
option_default_to(INSTALL_DOC_DIR     "share/doc/${PROJECT_NAME}" String "Installation directory for API documentation")

# set default destination for CMake modules
option_default_to(INSTALL_CMAKE_DIR "${CMAKE_ROOT}/Modules" String "Installation directory for CMake files")

############################################
# Normalize installation paths
# (get rid of Windows-style delimiters)
############################################

file(   TO_CMAKE_PATH   ${INSTALL_LIBRARY_DIR}  INSTALL_LIBRARY_DIR )
file(   TO_CMAKE_PATH   ${INSTALL_HEADERS_DIR}  INSTALL_HEADERS_DIR )
file(   TO_CMAKE_PATH   ${INSTALL_CMAKE_DIR}    INSTALL_CMAKE_DIR )

############################################

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/misc/Version.h.in
                ${CMAKE_CURRENT_SOURCE_DIR}/include/Carna/dicom/Version.h @ONLY)

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/src/doc/Doxyfile.in
                ${CMAKE_CURRENT_SOURCE_DIR}/src/doc/Doxyfile @ONLY)

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/misc/Find${PROJECT_NAME}.cmake.in
                ${CMAKE_CURRENT_BINARY_DIR}/misc/Find${PROJECT_NAME}.cmake @ONLY)

############################################
# Find required dependencies
############################################

# Qt
find_package( Qt5Core QUIET )
find_package( Qt5Widgets QUIET )
find_package( Qt5Gui QUIET )
find_package( Qt5Xml QUIET )
if(Qt5Core_FOUND AND Qt5Widgets_FOUND AND Qt5Gui_FOUND AND Qt5Xml_FOUND)
    set(Qt5_FOUND TRUE)
    set(QT_FOUND TRUE)
    set(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Xml)
    include_directories(
            ${Qt5Core_INCLUDE_DIRS}
            ${Qt5Widgets_INCLUDE_DIRS}
            ${Qt5Gui_INCLUDE_DIRS}
            ${Qt5Xml_INCLUDE_DIRS}
        )
else()
    find_package( Qt4 4.8.0 COMPONENTS QtCore QtGui QtXml )
    include( ${QT_USE_FILE} )
    add_definitions( ${QT_DEFINITIONS} )
endif()

# Eigen
include( $ENV{MEDITEC_LIBS}/Eigen/3.2.8/configure.cmake ) 
#find_package( Eigen3 REQUIRED )
#include_directories( ${EIGEN3_INCLUDE_DIR} )

# GDCM
find_package( GDCM 2.4.4 REQUIRED )
include_directories( ${GDCM_INCLUDE_DIR} )

# Carna
find_package( Carna 3.0.2 REQUIRED )
include_directories( ${CARNA_INCLUDE_DIR} )

# CarnaQt
find_package( CarnaQt 1.0.1 )
include_directories( ${CARNAQT_INCLUDE_DIR} )

############################################
# Enable/disable CarnaQt support
############################################

if( QT_FOUND AND CARNAQT_FOUND )
    set( CARNAQT_ENABLED 1 )
else()
    set( CARNAQT_ENABLED 0 )
endif()
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/misc/CarnaDICOM.h.in
                ${CMAKE_CURRENT_SOURCE_DIR}/include/Carna/dicom/CarnaDICOM.h @ONLY)

############################################
# Project
############################################

include_directories(${CMAKE_PROJECT_DIR}include)
set( PUBLIC_QOBJECT_HEADERS
        include/Carna/dicom/DICOMBrowser.h
)
set( PUBLIC_HEADERS
        ${PUBLIC_QOBJECT_HEADERS}
		include/Carna/dicom/CarnaDICOM.h
		include/Carna/dicom/Series.h
		include/Carna/dicom/SeriesElement.h
		include/Carna/dicom/Patient.h
		include/Carna/dicom/Study.h
		include/Carna/dicom/Directory.h
        include/Carna/dicom/VolumeGridFactory.h
    )
include_directories(${CMAKE_PROJECT_DIR}src/include)
set( PRIVATE_QOBJECT_HEADERS
        src/include/Carna/dicom/AsyncDirectory.h
        src/include/Carna/dicom/AsyncVolumeGridFactory.h
        src/include/Carna/dicom/SeriesView.h
        src/include/Carna/dicom/SeriesPreview.h
        src/include/Carna/dicom/ToggleSeriesPreview.h
        src/include/Carna/dicom/DICOMBrowserDetails.h
)
set( PRIVATE_HEADERS
        ${PRIVATE_QOBJECT_HEADERS}
        src/include/Carna/dicom/DICOMExtractionSettings.h
        src/include/Carna/dicom/IndexFile.h
        src/include/Carna/dicom/IndexFileReader.h
        src/include/Carna/dicom/IndexFileWriter.h
		src/include/dicom-interface/dicomImage.h
		src/include/dicom-interface/dicomSeries.h
		src/include/msvcpp/dirent.h
    )
set( SRC
		src/dicom/AsyncDirectory.cpp
		src/dicom/AsyncVolumeGridFactory.cpp
		src/dicom/Patient.cpp
		src/dicom/Series.cpp
		src/dicom/SeriesElement.cpp
		src/dicom/Study.cpp
		src/dicom/Directory.cpp
		src/dicom/SeriesView.cpp
		src/dicom/SeriesPreview.cpp
		src/dicom/ToggleSeriesPreview.cpp
		src/dicom/DICOMBrowser.cpp
		src/dicom/VolumeGridFactory.cpp
		src/dicom/IndexFile.cpp
		src/dicom/IndexFileReader.cpp
		src/dicom/IndexFileWriter.cpp
		src/dicom-interface/dicomImage.cpp
		src/dicom-interface/dicomSeries.cpp
    )
set( FORMS
        ""
    )
set( RESOURCES
		""
	)
set( DOC_SRC
        src/doc/Doxyfile.in
        src/doc/doc_extra.css
        src/doc/doc_main.dox
        src/doc/doc_version_log.dox
    )
set( MISC
        misc/${PROJECT_NAME}.h.in
        misc/Version.h.in
    )

############################################

if( QT_FOUND )
    if(Qt5_FOUND)
        QT5_WRAP_CPP( PUBLIC_HEADERS_MOC ${PUBLIC_QOBJECT_HEADERS} )
        QT5_WRAP_CPP( PRIVATE_HEADERS_MOC ${PRIVATE_QOBJECT_HEADERS} )
        QT5_WRAP_UI( FORMS_HEADERS ${FORMS} )
        QT5_ADD_RESOURCES( RESOURCES_RCC ${RESOURCES} )
    else(Qt5_FOUND)
        QT4_WRAP_CPP( PUBLIC_HEADERS_MOC ${PUBLIC_QOBJECT_HEADERS} )
        QT4_WRAP_CPP( PRIVATE_HEADERS_MOC ${PRIVATE_QOBJECT_HEADERS} )
        QT4_WRAP_UI( FORMS_HEADERS ${FORMS} )
        QT4_ADD_RESOURCES( RESOURCES_RCC ${RESOURCES} )
    endif(Qt5_FOUND)
    
    include_directories(${CMAKE_PROJECT_DIR}src/include/Carna/dicom)
    list( APPEND PRIVATE_HEADERS src/include/Carna/dicom/FlowLayout.h )
    list( APPEND SRC src/dicom/FlowLayout.cpp )
else()
    set( PUBLIC_HEADERS_MOC "" )
    set( PRIVATE_HEADERS_MOC "" )
    set( FORMS_HEADERS "" )
    set( RESOURCES_RCC "" )
    set( CARNAQT_LIBRARIES "" )
    set( QT_LIBRARIES "" )
endif()

############################################
# Setup Visual Studio filters
############################################

SOURCE_GROUP( "Headers"
    REGULAR_EXPRESSION .*include/Carna/qt/[^/]*\\.h )

SOURCE_GROUP( "Sources"
    REGULAR_EXPRESSION src/qt/[^/]*\\.cpp )

SOURCE_GROUP( "Miscellaneous"
    REGULAR_EXPRESSION .*\\.\(\(in\)|\(txt\)|\(qrc\)\) )

SOURCE_GROUP( "Miscellaneous\\Documentation"
    REGULAR_EXPRESSION .*\\.\(\(css\)|\(dox\)\) )

SOURCE_GROUP( "Miscellaneous\\Generated Files"
    REGULAR_EXPRESSION build/\(\(.*\\.\(\(cxx\)|\(cpp\)\)\)|\(.*/res/.*\)\) )

############################################
# Exclude all source files from build,
# since we are using unity build.
############################################

set_source_files_properties(    ${SRC}
                                ${PUBLIC_HEADERS_MOC}
                                ${PRIVATE_HEADERS_MOC}
                                ${FORMS_HEADERS}
                                ${RESOURCES_RCC}
                                ${DOC_SRC}
								${MISC}
    PROPERTIES
    HEADER_FILE_ONLY TRUE )

############################################
# Write unity build file
############################################

set( UNITY_BUILD_FILE   ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-unitybuild.cpp )

file( REMOVE    ${UNITY_BUILD_FILE} )
file( WRITE     ${UNITY_BUILD_FILE} "// This file is automatically generated by CMake.\n\n" )

foreach( SOURCE_FILE ${SRC} )
    file( APPEND    ${UNITY_BUILD_FILE} "#include \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\"\n" )
endforeach( SOURCE_FILE )

if( QT_FOUND )
	foreach( QT_GENERATED_SOURCE_FILE ${PUBLIC_HEADERS_MOC} ${PRIVATE_HEADERS_MOC} ${FORMS_HEADERS} ${RESOURCES_RCC} )
		file( APPEND	${UNITY_BUILD_FILE}	"#include \"${QT_GENERATED_SOURCE_FILE}\"\n" )
	endforeach( QT_GENERATED_SOURCE_FILE )
endif()

############################################
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
############################################

add_library( ${TARGET_NAME} SHARED
            ${SRC}
            ${UNITY_BUILD_FILE}
            ${PUBLIC_HEADERS}
            ${PRIVATE_HEADERS}
            ${PUBLIC_HEADERS_MOC}
            ${PRIVATE_HEADERS_MOC}
            ${FORMS_HEADERS}
            ${RESOURCES_RCC}
            ${DOC_SRC}
            ${MISC}
        )

add_definitions( -D${PROJECT_NAME_CAPS}_EXPORT -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS )

############################################
# Add dependencies to the linker
############################################

include( "misc/compiler_specific.cmake" )

target_link_libraries( ${TARGET_NAME}
            ${CARNA_LIBRARIES}
            ${CARNAQT_LIBRARIES}
            ${QT_LIBRARIES}
			${GDCM_LIBRARIES}
        )

############################################
# Define installation routines
############################################

install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION ${INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION ${INSTALL_LIBRARY_DIR}
        LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR})

# install headers
install(DIRECTORY ${CMAKE_PROJECT_DIR}include/Carna
        DESTINATION ${INSTALL_HEADERS_DIR})

# also install '.PDB' files on Windows
if( WIN32 )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bin/${TARGET_NAME}${CMAKE_DEBUG_POSTFIX}.pdb
            DESTINATION ${INSTALL_LIBRARY_DIR}
            OPTIONAL)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bin/${TARGET_NAME}${CMAKE_DEBUG_POSTFIX}.pdb
            DESTINATION ${INSTALL_LIBRARY_DIR}
            OPTIONAL)
endif( WIN32 )

# install CMake Find<Module> file
install(    FILES ${CMAKE_CURRENT_BINARY_DIR}/misc/Find${PROJECT_NAME}.cmake
            DESTINATION ${INSTALL_CMAKE_DIR})

# install MediTEC-specific files
add_meditec_install_routines()

############################################
# Build interactive demos
############################################

if( BUILD_DEMO )
	add_subdirectory( test/Demo )
endif()

############################################
# Process unit tests
############################################

if( BUILD_TEST )
    add_subdirectory( test )
endif()

############################################
# Doxygen API documentation
############################################

if( BUILD_DOC )
    add_subdirectory( src/doc )
endif()

############################################

