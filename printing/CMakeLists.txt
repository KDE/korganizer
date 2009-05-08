
include_directories( 
    ${CMAKE_SOURCE_DIR}/korganizer/interfaces 
    ${CMAKE_SOURCE_DIR}/korganizer
    ${CMAKE_BINARY_DIR}/korganizer
)


########### next target ###############

set(korg_stdprinting_LIB_SRCS 
   calprinter.cpp 
   calprintpluginbase.cpp 
   calprintdefaultplugins.cpp)

kde4_add_ui_files(korg_stdprinting_LIB_SRCS 
   calprintincidenceconfig_base.ui
   calprintdayconfig_base.ui 
   calprintmonthconfig_base.ui 
   calprinttodoconfig_base.ui 
   calprintweekconfig_base.ui)

kde4_add_library(korg_stdprinting SHARED ${korg_stdprinting_LIB_SRCS})

target_link_libraries(korg_stdprinting ${KDE4_KUTILS_LIBS} korganizer_core kdepim ${KDEPIMLIBS_KCAL_LIBS} ${QT_QT3SUPPORT_LIBRARY})

set_target_properties(korg_stdprinting PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS korg_stdprinting ${INSTALL_TARGETS_DEFAULT_ARGS})
