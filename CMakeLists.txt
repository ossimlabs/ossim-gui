cmake_minimum_required(VERSION 2.8.11)

PROJECT(ossimGui)
MESSAGE("**********************Setting up ossimGui********************")
INCLUDE(OssimVersion)
INCLUDE(OssimUtilities)
INCLUDE(OssimCommonVariables)
INCLUDE(OssimQt5Macros)

# SET (QT_MIN_VERSION "4.2.2")

# set (CMAKE_PREFIX_PATH D:/drb/Qt/Qt5.7/msvc2015_64/lib/cmake)
# set (CMAKE_PREFIX_PATH "${QTDIR}/lib/cmake/Qt5Widgets")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)



# Instruct CMake to run moc automatically when needed.
# This causes duplicate symbols on linux.
#set(CMAKE_AUTOMOC ON)


# For the finds to work properly you can add the varible definitions
# To where the cmake configs reside on your installed Qt5 system
# 
# Qt5Widgets_DIR=
# Qt5Core_DIR=
# Qt5OpenGL_DIR=
#

# Find the QtWidgets library
find_package(Qt5Widgets)

# Find the QtWidgets library
find_package(Qt5Core)

# Find the QtOpenGL library
find_package(Qt5OpenGL)



# FIND_PACKAGE(Qt4 REQUIRED) # find and setup Qt4 for this project
# INCLUDE(${QT_USE_FILE})
SET(QT_USE_QTOPENGL ON)
# find_package(Qt4)

find_package(OpenThreads)
find_package(ossim)
find_package(TIFF)
IF(BUILD_OSSIM_PLANET)
	find_package(ossimPlanet)
	find_package(OpenGL)
	find_package(OSG)
ENDIF(BUILD_OSSIM_PLANET)


add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(${Qt5Widgets_DEFINITIONS})
add_definitions(${Qt5OpenGL_DEFINITIONS})
#ADD_DEFINITIONS(${QT_DEFINITIONS}
#	-DQT_CORE_LIB
#	-DQT_GUI_LIB
#	-DQT_THREAD_SUPPORT
#)

message("QT_LIBRARIES: ${QT_LIBRARIES}")
message("QT_INCLUDE: ${QT_INCLUDE}")

INCLUDE_DIRECTORIES(${${PROJECT_NAME}_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${OSSIM_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${QT_INCLUDES})
INCLUDE_DIRECTORIES(${OPENTHREADS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})

# To pick up uic generated headers.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${Qt5Core_INCLUDES})
include_directories(${Qt5Widgets_INCLUDES})
include_directories(${Qt5OpenGL_INCLUDES})

#MESSAGE(STATUS "QT_INCLUDES = ${QT_INCLUDES}")

############################################## BUILD THE LIBRARY ############################################
SET(LIB_NAME ossimGui)
FILE(GLOB OSSIMGUI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include/ossimGui/*.h")
FILE(GLOB OSSIMGUI_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/ossimGui/*.cpp" )
FILE(GLOB OSSIMGUI_UIS  "${CMAKE_CURRENT_SOURCE_DIR}/src/ossimGui/ui/*.ui")

SET(ADDITIONAL_LIBRARIES "")

IF(BUILD_OSSIM_PLANET)
#   LIST(APPEND ADDITIONAL_INCLUDES ${OSSIMPLANET_INCLUDE_DIR})
#   LIST(APPEND ADDITIONAL_INCLUDES ${OSG_INCLUDE_DIR})
   INCLUDE_DIRECTORIES(${OSSIMPLANET_INCLUDE_DIR})
   INCLUDE_DIRECTORIES(${OSG_INCLUDE_DIR})

   LIST(APPEND ADDITIONAL_LIBRARIES ${OSSIMPLANET_LIBRARY})
   LIST(APPEND ADDITIONAL_LIBRARIES ${QT_QTOPENGL_LIBRARY})

   ADD_DEFINITIONS(-DOSSIM_PLANET_ENABLED)

   MESSAGE("OSSIM_PLANET_ENABLED=true")

ELSE()

   MESSAGE("OSSIM_PLANET_ENABLED=false")

   LIST(REMOVE_ITEM OSSIMGUI_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/ossimGui/PlanetMdiSubWindow.cpp")
   LIST(REMOVE_ITEM OSSIMGUI_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/ossimGui/GlWidget.cpp")
   LIST(REMOVE_ITEM OSSIMGUI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include/ossimGui/PlanetMdiSubWindow.h")
   LIST(REMOVE_ITEM OSSIMGUI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include/ossimGui/GlWidget.h")

ENDIF()

MESSAGE("QT4_LIBRARIES === ${QT_LIBRARIES}")
message("Qt5Core_LIBRARIES: ${Qt5Core_LIBRARIES}")
message("Qt5Widgets_LIBRARIES: ${Qt5Widgets_LIBRARIES}")
message("Qt5OpenGL_LIBRARIES: ${Qt5OpenGL_LIBRARIES}")

QT5_WRAP_CPP(OSSIMGUI_MOC_SRCS ${OSSIMGUI_INCLUDES} )

MESSAGE("UI_DESTINATION_DIR ${${PROJECT_NAME}_SOURCE_DIR}/include/ossimGui")
SET(UI_DESTINATION_DIR ${${PROJECT_NAME}_SOURCE_DIR}/include/ossimGui)
# OSSIM_QT5_WRAP_UI( UI_HEADERS ${OSSIMGUI_UIS} )

qt5_wrap_ui( OSSIMGUI_UI_HEADERS ${OSSIMGUI_UIS} )
# source_group("Generated UI Headers" FILES ${OSSIMGUI_UI_HEADERS})

message("UI_HEADERS: ${OSSIMGUI_UI_HEADERS}")
# source_group("Generated UI Headers" FILES ${UI_HEADERS})


SET(INCLUDE_DIR "include/ossimGui")

SET(INSTALL_INCLUDE_DIR "include/ossimGui")

OSSIM_LINK_LIBRARY(${LIB_NAME} COMPONENT_NAME ossim TYPE "${OSSIM_USER_DEFINED_DYNAMIC_OR_STATIC}" HEADERS ${OSSIMGUI_INCLUDES} ${OSSIMGUI_UI_HEADERS} SOURCE_FILES ${OSSIMGUI_SRCS}  ${OSSIMGUI_MOC_SRCS}
                   LIBRARIES ${OSSIM_LIBRARY} ${OSG_LIBRARY}  ${OPENTHREADS_LIBRARY} ${JPEG_LIBRARY} ${ZLIB_LIBRARY} ${Qt5Core_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5OpenGL_LIBRARIES} ${TIFF_LIBRARIES}  ${ADDITIONAL_LIBRARIES}
                   INSTALL_LIB INSTALL_HEADERS VERSION_SYMLINKS ADDITIONAL_COMPILE_FLAGS "-DOSSIMGUI_LIBRARY")



############################################# BUILD THE APPLICATIONS #################################################

QT5_ADD_RESOURCES(OSSIMGUI_RC "${PROJECT_SOURCE_DIR}/images/images.qrc")
set(TARGET_COMMON_LIBRARIES ossimGui)
FILE(GLOB SOURCE_FILES "${PROJECT_SOURCE_DIR}/apps/ossim-geocell/ossim-geocell.cpp")
SET(OSSIM_BUILD_APPLICATION_BUNDLES ON)
MESSAGE(STATUS "IMAGELINKER SOURCE FILES = ${SOURCE_FILES}")
OSSIM_SETUP_APPLICATION(ossim-geocell INSTALL REQUIRE_WINMAIN_FLAG COMPONENT_NAME ossim SOURCE_FILES ${SOURCE_FILES} ${OSSIMGUI_RC})

# Use the Widgets module from Qt 5.
qt5_use_modules(ossim-geocell Widgets)

########################### SETUP UNINSTALL ############################
OSSIM_ADD_COMMON_MAKE_UNINSTALL()
