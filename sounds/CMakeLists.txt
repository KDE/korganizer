kde4_header()




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
