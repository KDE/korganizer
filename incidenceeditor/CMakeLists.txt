include_directories( 
    ${CMAKE_SOURCE_DIR}/korganizer/interfaces 
    ${CMAKE_SOURCE_DIR}/korganizer/incidenceeditor
    ${CMAKE_BINARY_DIR}/korganizer/incidenceeditor
    ${CMAKE_SOURCE_DIR}/kdgantt1
    ${CMAKE_SOURCE_DIR}/libkdepimdbusinterfaces
)

########### next target ###############

set( korg_incidenceeditor_LIB_SRCS 
    ${libkdgantt1_SRCS}
    koeditorconfig.cpp
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
    templatemanagementdialog.cpp
)


kde4_add_ui_files( korg_incidenceeditor_LIB_SRCS 
   koeditoralarms_base.ui
   template_management_dialog_base.ui
)

kde4_add_library(korg_incidenceeditor SHARED ${korg_incidenceeditor_LIB_SRCS})

target_link_libraries(korg_incidenceeditor
   ${ZLIB_LIBRARY}
   ${QT_QT3SUPPORT_LIBRARY}
   ${QT_QTCORE_LIBRARY}
   ${QT_QTGUI_LIBRARY}
   ${KDE4_KDEUI_LIBS}
   ${KDE4_KPARTS_LIBS}
   ${KDE4_KMIME_LIBRARY}
   ${KDE4_KDE3SUPPORT_LIBRARY}
   ${KDEPIMLIBS_KCAL_LIBS}
   kdepim
   kdepimdbusinterfaces
   akonadi-kcal
)

set_target_properties(korg_incidenceeditor PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS korg_incidenceeditor ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

set(incidenceeditorapp_SRCS main.cpp)
qt_wrap_cpp(incidenceeditorapp ${incidenceeditorapp_SRCS} main.cpp)
kde4_add_executable(incidenceeditorapp ${incidenceeditorapp_SRCS})
target_link_libraries(incidenceeditorapp  ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KCAL_LIBS} akonadi-kcal korg_incidenceeditor)
#install(TARGETS incidenceeditorapp ${INSTALL_TARGETS_DEFAULT_ARGS})
