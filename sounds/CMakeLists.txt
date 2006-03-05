kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/korganizer/sounds FILES alert.wav icemag.wav lightmag.wav onscreen.wav  	spinout.wav )

kde4_footer()



#original Makefile.am contents follow:

#
#soundsdir = $(kde_datadir)/korganizer/sounds
#
#sounds_DATA = alert.wav icemag.wav lightmag.wav onscreen.wav \
#	spinout.wav
#
#
