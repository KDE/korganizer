
########### next target ###############

install( FILES
           calendarplugin.desktop
           calendardecoration.desktop
         DESTINATION ${SERVICETYPES_INSTALL_DIR}
       )

install( FILES
           plugin.h
           calendardecoration.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/calendar
       )
