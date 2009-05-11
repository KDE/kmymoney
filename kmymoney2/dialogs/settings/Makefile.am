noinst_LIBRARIES = libsettings.a
KDE_OPTIONS = noautodist

INCLUDES = $(all_includes)  -I$(top_srcdir) -I.

libsettings_a_METASOURCES = AUTO

libsettings_a_SOURCES = ksettingsgeneraldecl.ui ksettingsgeneral.cpp \
	ksettingsregisterdecl.ui ksettingsregister.cpp \
	ksettingsgpgdecl.ui ksettingsgpg.cpp \
	ksettingscolorsdecl.ui ksettingscolors.cpp \
	ksettingsfontsdecl.ui ksettingsfonts.cpp \
	ksettingsschedulesdecl.ui ksettingsschedules.cpp \
	ksettingsonlinequotesdecl.ui ksettingsonlinequotes.cpp \
	ksettingshomedecl.ui ksettingshome.cpp  \
	ksettingsforecastdecl.ui ksettingsforecast.cpp  \
	ksettingsplugins.cpp

EXTRA_DIST = ksettingsgeneraldecl.ui ksettingsregisterdecl.ui ksettingsgpgdecl.ui ksettingscolorsdecl.ui ksettingsfontsdecl.ui ksettingsschedulesdecl.ui ksettingsonlinequotesdecl.ui ksettingshomedecl.ui ksettingsforecastdecl.ui

DISTCLEANFILES= ksettingsgeneraldecl.cpp ksettingsgeneraldecl.h ksettingsregisterdecl.cpp ksettingsregisterdecl.h ksettingsgpgdecl.cpp ksettingsgpgdecl.h ksettingscolorsdecl.cpp ksettingscolorsdecl.h ksettingsfontsdecl.cpp ksettingsfontsdecl.h ksettingsschedulesdecl.cpp ksettingsschedulesdecl.h ksettingsonlinequotesdecl.cpp ksettingsonlinequotesdecl.h ksettingshomedecl.cpp ksettingshomedecl.h ksettingsforecastdecl.cpp ksettingsforecastdecl.h

noinst_HEADERS = ksettingsgeneral.h ksettingsregister.h ksettingsgpg.h ksettingscolors.h ksettingsfonts.h ksettingsschedules.h ksettingsonlinequotes.h ksettingshome.h ksettingsforecast.h ksettingsplugins.h

messages: rc.cpp
