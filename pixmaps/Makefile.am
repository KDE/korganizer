
picsdir = $(kde_datadir)/korganizer/pics
toolbardir = $(kde_datadir)/korganizer/toolbar

pics_DATA = smallcal.xpm 1downarrow.xpm 1leftarrow.xpm 1rightarrow.xpm \
	1uparrow.xpm 2downarrow.xpm 2leftarrow.xpm 2rightarrow.xpm \
	2uparrow.xpm 3downarrow.xpm 3leftarrow.xpm 3rightarrow.xpm \
	3uparrow.xpm checkmark.xpm checkedbox-mask.xpm checkedbox.xpm \
	emptybox-mask.xpm emptybox.xpm bell.xpm recur.xpm

toolbar_DATA = listicon.xpm 5dayicon.xpm  dayicon.xpm agenda.xpm \
	weekicon.xpm todayicon.xpm newevent.xpm newrecurevent.xpm \
	delete.xpm monthicon.xpm search.xpm checkedclipboard.xpm

EXTRA_DIST = dayview.xpm   inverse_recur.xpm korganizer.xpm \
	mini-korganizer.xpm 

icon_DATA = korganizer.xpm
icondir = $(kde_icondir)

install-data-local:
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-korganizer.xpm $(kde_minidir)/korganizer.xpm

uninstall-local:
	-rm -f $(kde_minidir)/korganizer.xpm
