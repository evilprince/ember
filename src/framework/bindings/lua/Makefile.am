SUBDIRS = atlas eris varconf

SUFFIXES: .cxx .pkg .lo .la .cpp .o .obj

.pkg.cxx:
	cd $(srcdir) && TOLUAXX=${TOLUAXX} $(abs_top_srcdir)/scripts/update_lua_bindings.sh `basename $@ .cxx` `basename $@ .cxx`.pkg $(abs_builddir)/`basename $@` $<

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(srcdir) -I$(top_builddir)/src -DPREFIX=\"@prefix@\"

noinst_LIBRARIES = liblua_Framework.a
liblua_Framework_a_SOURCES = Framework.cxx

CLEANFILES = Framework.cxx
TOLUA_PKGS = AttributeObserver.pkg ConsoleBackend.pkg Framework.pkg IScriptingProvider.pkg Log.pkg LoggingInstance.pkg stdlib.pkg Tokeniser.pkg MainLoopController.pkg EntityExporter.pkg EntityImporter.pkg AtlasObjectDecoder.pkg
EXTRA_DIST = $(TOLUA_PKGS)
Framework.cxx: $(TOLUA_PKGS)
noinst_HEADERS = required.h

bindings: clean all

check:
	@set -e; cd $(srcdir) && for filename in *.pkg ; do \
		if [[ "$(TOLUA_PKGS)" != *$${filename}* ]] ;\
		then \
			echo "The file $${filename} exists in $(srcdir) but is not referenced in the Makefile.am. Either remove it if it's not used, or update the Makefile.am"; \
		  	exit 1; \
		fi \
	done;