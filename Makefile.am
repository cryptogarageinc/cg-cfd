cfd_all:
	cd src && make cfd_all

#	cd test && make cfd_all

cfd_clean:
	cd src && make cfd_clean

#	cd test && make cfd_clean

install-data-hook:
	cd include && $(MAKE) install

uninstall-hook:
	cd include && $(MAKE) uninstall

# build for src
SUBDIRS = src
DIST_SUBDIRS = $(SUBDIRS) include

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = pc/cfd.pc
