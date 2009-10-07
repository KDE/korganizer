
include_directories( 
    ${CMAKE_SOURCE_DIR}/korganizer/interfaces 
    ${CMAKE_SOURCE_DIR}/korganizer
    ${CMAKE_BINARY_DIR}/korganizer
    ${CMAKE_SOURCE_DIR}/kdgantt1
)


########### next target ###############

set( korg_incidenceeditor_LIB_SRCS 
    ${libkdgantt1_SRCS}
    koincidenceeditor.cpp
    koeventeditor.cpp
    kotodoeditor.cpp
    kojournaleditor.cpp
    koeditorgeneral.cpp
    koeditorgeneralevent.cpp
    koeditorgeneraltodo.cpp
    koeditorgeneraljournal.cpp
    koeditordetails.cpp
    koeditoralarms.cpp
    koeditorrecurrence.cpp
    koeditorattachments.cpp
    koeditorfreebusy.cpp
    koattendeeeditor.cpp
)


kde4_add_ui_files( korg_incidenceeditor_LIB_SRCS 
   koeditoralarms_base.ui
)

kde4_add_library(korg_incidenceeditor SHARED ${korg_incidenceeditor_LIB_SRCS})

target_link_libraries(korg_incidenceeditor
   ${QT_QT3SUPPORT_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${QT_QTGUI_LIBRARY}
   ${KDE4_KPARTS_LIBS}
   ${KDEPIMLIBS_KCAL_LIBS}
   kdepim
)

set_target_properties(korg_incidenceeditor PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS korg_incidenceeditor ${INSTALL_TARGETS_DEFAULT_ARGS})
