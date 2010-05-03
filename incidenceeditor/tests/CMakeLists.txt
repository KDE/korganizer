########### next target ###############

set(incidenceeditorapp_SRCS incidenceeditorapp.cpp)
qt_wrap_cpp(incidenceeditorapp ${incidenceeditorapp_SRCS} incidenceeditor.cpp)
kde4_add_executable(incidenceeditorapp ${incidenceeditorapp_SRCS})
target_link_libraries(incidenceeditorapp
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KCAL_LIBS}
  ${KDEPIMLIBS_AKONADI_KCAL_LIBRARY}
  korg_incidenceeditor
  akonadi-kcal_next
)
#install(TARGETS incidenceeditorapp ${INSTALL_TARGETS_DEFAULT_ARGS})
