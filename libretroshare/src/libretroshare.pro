TEMPLATE = lib
CONFIG += staticlib bitdht
CONFIG -= qt
TARGET = retroshare

CONFIG += test_voip 

# GXS Stuff.
# This should be disabled for releases until further notice.
#CONFIG += gxs

# Beware: All data of the stripped services are lost
DEFINES *= PQI_DISABLE_TUNNEL
#ENABLE_CACHE_OPT

profiling {
	QMAKE_CXXFLAGS -= -fomit-frame-pointer
	QMAKE_CXXFLAGS *= -pg -g -fno-omit-frame-pointer
}

# treat warnings as error for better removing
#QMAKE_CFLAGS += -Werror
#QMAKE_CXXFLAGS += -Werror

#CONFIG += debug
debug {
#	DEFINES *= DEBUG
#	DEFINES *= OPENDHT_DEBUG DHT_DEBUG CONN_DEBUG DEBUG_UDP_SORTER P3DISC_DEBUG DEBUG_UDP_LAYER FT_DEBUG EXTADDRSEARCH_DEBUG
#	DEFINES *= CONTROL_DEBUG FT_DEBUG DEBUG_FTCHUNK P3TURTLE_DEBUG
#	DEFINES *= P3TURTLE_DEBUG 
#	DEFINES *= NET_DEBUG
#	DEFINES *= DISTRIB_DEBUG
#	DEFINES *= P3TURTLE_DEBUG FT_DEBUG DEBUG_FTCHUNK MPLEX_DEBUG
#	DEFINES *= STATUS_DEBUG SERV_DEBUG RSSERIAL_DEBUG #CONN_DEBUG 

        QMAKE_CXXFLAGS -= -O2 -fomit-frame-pointer
        QMAKE_CXXFLAGS *= -g -fno-omit-frame-pointer
}


bitdht {

HEADERS +=	dht/p3bitdht.h \
		dht/connectstatebox.h \
		dht/stunaddrassist.h

SOURCES +=	dht/p3bitdht.cc  \
		dht/p3bitdht_interface.cc \
		dht/p3bitdht_peers.cc \
		dht/p3bitdht_peernet.cc \
		dht/p3bitdht_relay.cc \
		dht/connectstatebox.cc

HEADERS +=	tcponudp/udppeer.h \
		tcponudp/bio_tou.h \
		tcponudp/tcppacket.h \
		tcponudp/tcpstream.h \
		tcponudp/tou.h \
		tcponudp/udpstunner.h \
		tcponudp/udprelay.h \

SOURCES +=	tcponudp/udppeer.cc \
		tcponudp/tcppacket.cc \
		tcponudp/tcpstream.cc \
		tcponudp/tou.cc \
		tcponudp/bss_tou.c \
		tcponudp/udpstunner.cc \
		tcponudp/udprelay.cc \


        BITDHT_DIR = ../../libbitdht/src
	INCLUDEPATH += . $${BITDHT_DIR}
	# The next line is for compliance with debian packages. Keep it!
	INCLUDEPATH += ../libbitdht
	DEFINES *= RS_USE_BITDHT
}




PUBLIC_HEADERS =	retroshare/rsblogs.h \
					retroshare/rschannels.h \
					retroshare/rsdisc.h \
					retroshare/rsdistrib.h \
					retroshare/rsexpr.h \
					retroshare/rsfiles.h \
					retroshare/rsforums.h \
					retroshare/rshistory.h \
					retroshare/rsiface.h \
					retroshare/rsinit.h \
					retroshare/rsplugin.h \
					retroshare/rsloginhandler.h \
					retroshare/rsmsgs.h \
					retroshare/rsnotify.h \
					retroshare/rspeers.h \
					retroshare/rsrank.h \
					retroshare/rsstatus.h \
					retroshare/rsturtle.h \
					retroshare/rstypes.h \
					retroshare/rsdht.h \
					retroshare/rsdsdv.h \
					retroshare/rsconfig.h

HEADERS += plugins/pluginmanager.h \
		plugins/dlfcn_win32.h \
		serialiser/rspluginitems.h

HEADERS += $$PUBLIC_HEADERS

# public headers to be...
HEADERS +=	retroshare/rsgame.h \
		retroshare/rsphoto.h

################################# Linux ##########################################
linux-* {
	isEmpty(PREFIX)  { PREFIX = /usr }
	isEmpty(INC_DIR) { INC_DIR = $${PREFIX}/include/retroshare/ }
	isEmpty(LIB_DIR) { LIB_DIR = $${PREFIX}/lib/ }

	# These two lines fixe compilation on ubuntu natty. Probably a ubuntu packaging error.
	INCLUDEPATH += $$system(pkg-config --cflags glib-2.0 | sed -e "s/-I//g")

	OPENPGPSDK_DIR = ../../openpgpsdk/src
	INCLUDEPATH *= $${OPENPGPSDK_DIR} ../openpgpsdk

	DESTDIR = lib
	QMAKE_CXXFLAGS *= -Wall -D_FILE_OFFSET_BITS=64
	QMAKE_CC = g++

	SSL_DIR = /usr/include/openssl
	UPNP_DIR = /usr/include/upnp
	INCLUDEPATH += . $${SSL_DIR} $${UPNP_DIR}

	# where to put the shared library itself
	target.path = $$LIB_DIR
	INSTALLS *= target

	# where to put the librarys interface
	include_rsiface.path = $${INC_DIR}
	include_rsiface.files = $$PUBLIC_HEADERS
	INSTALLS += include_rsiface

	#CONFIG += version_detail_bash_script


	# linux/bsd can use either - libupnp is more complete and packaged.
	#CONFIG += upnp_miniupnpc 
	CONFIG += upnp_libupnp

	# Check if the systems libupnp has been Debian-patched
	system(grep -E 'char[[:space:]]+PublisherUrl' $${UPNP_DIR}/upnp.h >/dev/null 2>&1) {
		# Normal libupnp
	} else {
		# Patched libupnp or new unreleased version
		DEFINES *= PATCHED_LIBUPNP
	}

	DEFINES *= UBUNTU
	INCLUDEPATH += /usr/include/glib-2.0/ /usr/lib/glib-2.0/include
	LIBS *= -lgnome-keyring
}

linux-g++ {
	OBJECTS_DIR = temp/linux-g++/obj
}

linux-g++-64 {
	OBJECTS_DIR = temp/linux-g++-64/obj
}

version_detail_bash_script {
    QMAKE_EXTRA_TARGETS += write_version_detail
    PRE_TARGETDEPS = write_version_detail
    write_version_detail.commands = ./version_detail.sh
}

#################### Cross compilation for windows under Linux ####################

win32-x-g++ {	
	OBJECTS_DIR = temp/win32xgcc/obj
	DESTDIR = lib.win32xgcc
	DEFINES *= WINDOWS_SYS WIN32 WIN_CROSS_UBUNTU
	QMAKE_CXXFLAGS *= -Wmissing-include-dirs
	QMAKE_CC = i586-mingw32msvc-g++
	QMAKE_LIB = i586-mingw32msvc-ar
	QMAKE_AR = i586-mingw32msvc-ar
	DEFINES *= STATICLIB WIN32

	CONFIG += upnp_miniupnpc

        SSL_DIR=../../../../openssl
	UPNPC_DIR = ../../../../miniupnpc-1.3
	GPG_ERROR_DIR = ../../../../libgpg-error-1.7
	GPGME_DIR  = ../../../../gpgme-1.1.8

	INCLUDEPATH *= /usr/i586-mingw32msvc/include ${HOME}/.wine/drive_c/pthreads/include/
}
################################# Windows ##########################################


win32 {
	QMAKE_CC = g++
	OBJECTS_DIR = temp/obj
	MOC_DIR = temp/moc
	DEFINES *= WINDOWS_SYS WIN32 STATICLIB MINGW
	DEFINES *= MINIUPNPC_VERSION=13
	DESTDIR = lib

	# Switch on extra warnings
	QMAKE_CFLAGS += -Wextra
	QMAKE_CXXFLAGS += -Wextra

	# Switch off optimization for release version
	QMAKE_CXXFLAGS_RELEASE -= -O2
	QMAKE_CXXFLAGS_RELEASE += -O0
	QMAKE_CFLAGS_RELEASE -= -O2
	QMAKE_CFLAGS_RELEASE += -O0

	# Switch on optimization for debug version
	#QMAKE_CXXFLAGS_DEBUG += -O2
	#QMAKE_CFLAGS_DEBUG += -O2

	DEFINES += USE_CMD_ARGS

	CONFIG += upnp_miniupnpc

	UPNPC_DIR = ../../../miniupnpc-1.3

	PTHREADS_DIR = ../../../pthreads-w32-2-8-0-release
	ZLIB_DIR = ../../../zlib-1.2.3
	SSL_DIR = ../../../openssl-1.0.1c
	OPENPGPSDK_DIR = ../../openpgpsdk/src

	INCLUDEPATH += . $${SSL_DIR}/include $${UPNPC_DIR} $${PTHREADS_DIR} $${ZLIB_DIR} $${OPENPGPSDK_DIR}

	# SQLite include path is required to compile GXS.
	gxs {
		SQLITE_DIR = ../../../sqlite-autoconf-3070900
		INCLUDEPATH += $${SQLITE_DIR}
	}

}


################################# MacOSX ##########################################

mac {
		QMAKE_CC = g++
		OBJECTS_DIR = temp/obj
		MOC_DIR = temp/moc
		#DEFINES = WINDOWS_SYS WIN32 STATICLIB MINGW
                #DEFINES *= MINIUPNPC_VERSION=13
		DESTDIR = lib

		CONFIG += upnp_miniupnpc

		# zeroconf disabled at the end of libretroshare.pro (but need the code)
		CONFIG += zeroconf
		CONFIG += zcnatassist

		# Beautiful Hack to fix 64bit file access.
                QMAKE_CXXFLAGS *= -Dfseeko64=fseeko -Dftello64=ftello -Dfopen64=fopen -Dvstatfs64=vstatfs

                UPNPC_DIR = ../../../miniupnpc-1.0
		#GPG_ERROR_DIR = ../../../../libgpg-error-1.7
		#GPGME_DIR  = ../../../../gpgme-1.1.8

		OPENPGPSDK_DIR = ../../openpgpsdk/src

		INCLUDEPATH += . $${UPNPC_DIR} 
		INCLUDEPATH += $${OPENPGPSDK_DIR} 

		#../openpgpsdk
		#INCLUDEPATH += . $${UPNPC_DIR} $${GPGME_DIR}/src $${GPG_ERROR_DIR}/src
}

################################# FreeBSD ##########################################

freebsd-* {
	INCLUDEPATH *= /usr/local/include/gpgme
	INCLUDEPATH *= /usr/local/include/glib-2.0

	QMAKE_CXXFLAGS *= -Dfseeko64=fseeko -Dftello64=ftello -Dstat64=stat -Dstatvfs64=statvfs -Dfopen64=fopen

	# linux/bsd can use either - libupnp is more complete and packaged.
	#CONFIG += upnp_miniupnpc 
	CONFIG += upnp_libupnp

	DESTDIR = lib
}

################################### COMMON stuff ##################################

HEADERS +=	dbase/cachestrapper.h \
			dbase/fimonitor.h \
			dbase/findex.h \
			dbase/fistore.h

#HEADERS +=	dht/p3bitdht.h \

HEADERS +=	ft/ftchunkmap.h \
			ft/ftcontroller.h \
			ft/ftdata.h \
			ft/ftdatamultiplex.h \
			ft/ftdbase.h \
			ft/ftextralist.h \
			ft/ftfilecreator.h \
			ft/ftfileprovider.h \
			ft/ftfilesearch.h \
			ft/ftsearch.h \
			ft/ftserver.h \
			ft/fttransfermodule.h

HEADERS +=	pqi/authssl.h \
			pqi/authgpg.h \
			pgp/pgphandler.h \
			pgp/pgpkeyutil.h \
			pgp/rscertificate.h \
			pqi/p3cfgmgr.h \
			pqi/p3peermgr.h \
			pqi/p3linkmgr.h \
			pqi/p3netmgr.h \
			pqi/p3dhtmgr.h \
			pqi/p3notify.h \
			pqi/p3upnpmgr.h \
			pqi/pqiqos.h \
			pqi/pqi.h \
			pqi/pqi_base.h \
			pqi/pqiarchive.h \
			pqi/pqiassist.h \
			pqi/pqibin.h \
			pqi/pqihandler.h \
			pqi/pqihash.h \
			pqi/p3historymgr.h \
			pqi/pqiindic.h \
			pqi/pqiipset.h \
			pqi/pqilistener.h \
			pqi/pqiloopback.h \
			pqi/pqimonitor.h \
			pqi/pqinetwork.h \
			pqi/pqinotify.h \
			pqi/pqiperson.h \
			pqi/pqipersongrp.h \
			pqi/pqisecurity.h \
			pqi/pqiservice.h \
			pqi/pqissl.h \
			pqi/pqissllistener.h \
			pqi/pqisslpersongrp.h \
			pqi/pqissltunnel.h \
			pqi/pqissludp.h \
			pqi/pqistore.h \
			pqi/pqistreamer.h \
			pqi/pqiqosstreamer.h \
			pqi/sslfns.h \
			pqi/pqinetstatebox.h 

HEADERS +=	rsserver/p3discovery.h \
			rsserver/p3face.h \
			rsserver/p3history.h \
			rsserver/p3msgs.h \
			rsserver/p3peers.h \
			rsserver/p3status.h \
			rsserver/p3serverconfig.h

HEADERS +=	serialiser/rsbaseitems.h \
			serialiser/rsbaseserial.h \
			serialiser/rsblogitems.h \
			serialiser/rschannelitems.h \
			serialiser/rsconfigitems.h \
			serialiser/rsdiscitems.h \
			serialiser/rsdistribitems.h \
			serialiser/rsforumitems.h \
			serialiser/rsgameitems.h \
			serialiser/rshistoryitems.h \
			serialiser/rsmsgitems.h \
			serialiser/rsserial.h \
			serialiser/rsserviceids.h \
			serialiser/rsserviceitems.h \
			serialiser/rsstatusitems.h \
			serialiser/rstlvaddrs.h \
			serialiser/rstlvbase.h \
			serialiser/rstlvkeys.h \
			serialiser/rstlvkvwide.h \
			serialiser/rstlvtypes.h \
			serialiser/rstlvutil.h \
			serialiser/rstlvdsdv.h \
			serialiser/rsdsdvitems.h \
			serialiser/rstlvbanlist.h \
			serialiser/rsbanlistitems.h \
			serialiser/rsbwctrlitems.h \
			serialiser/rstunnelitems.h

HEADERS +=	services/p3channels.h \
			services/p3chatservice.h \
			services/p3disc.h \
			services/p3forums.h \
			services/p3gamelauncher.h \
			services/p3gameservice.h \
			services/p3msgservice.h \
			services/p3service.h \
			services/p3statusservice.h \
			services/p3dsdv.h \
			services/p3banlist.h \
			services/p3bwctrl.h \
			services/p3tunnel.h
		
HEADERS +=	distrib/p3distrib.h \
			distrib/p3distribsecurity.h 
#	services/p3blogs.h \

HEADERS +=	turtle/p3turtle.h \
			turtle/rsturtleitem.h \
			turtle/turtletypes.h


HEADERS +=	util/folderiterator.h \
			util/rsdebug.h \
			util/smallobject.h \
			util/rsdir.h \
			util/rsdiscspace.h \
			util/rsnet.h \
			util/extaddrfinder.h \
			util/dnsresolver.h \
			util/rsprint.h \
			util/rsstring.h \
			util/rsthreads.h \
			util/rsversion.h \
			util/rswin.h \
			util/rsrandom.h \
			util/radix64.h \
			util/pugiconfig.h \  
			util/rsmemcache.h \
			util/rstickevent.h \

SOURCES +=	dbase/cachestrapper.cc \
			dbase/fimonitor.cc \
			dbase/findex.cc \
			dbase/fistore.cc \
			dbase/rsexpr.cc


SOURCES +=	ft/ftchunkmap.cc \
			ft/ftcontroller.cc \
			ft/ftdatamultiplex.cc \
			ft/ftdbase.cc \
			ft/ftextralist.cc \
			ft/ftfilecreator.cc \
			ft/ftfileprovider.cc \
			ft/ftfilesearch.cc \
			ft/ftserver.cc \
			ft/fttransfermodule.cc \

SOURCES +=	pqi/authgpg.cc \
			pqi/authssl.cc \
			pgp/pgphandler.cc \
			pgp/pgpkeyutil.cc \
			pgp/rscertificate.cc \
			pqi/p3cfgmgr.cc \
			pqi/p3peermgr.cc \
			pqi/p3linkmgr.cc \
			pqi/p3netmgr.cc \
			pqi/p3dhtmgr.cc \
			pqi/p3notify.cc \
			pqi/pqiqos.cc \
			pqi/pqiarchive.cc \
			pqi/pqibin.cc \
			pqi/pqihandler.cc \
			pqi/p3historymgr.cc \
			pqi/pqiipset.cc \
			pqi/pqiloopback.cc \
			pqi/pqimonitor.cc \
			pqi/pqinetwork.cc \
			pqi/pqiperson.cc \
			pqi/pqipersongrp.cc \
			pqi/pqisecurity.cc \
			pqi/pqiservice.cc \
			pqi/pqissl.cc \
			pqi/pqissllistener.cc \
			pqi/pqisslpersongrp.cc \
			pqi/pqissltunnel.cc \
			pqi/pqissludp.cc \
			pqi/pqistore.cc \
			pqi/pqistreamer.cc \
			pqi/pqiqosstreamer.cc \
			pqi/sslfns.cc \
			pqi/pqinetstatebox.cc 

SOURCES +=	rsserver/p3discovery.cc \
			rsserver/p3face-config.cc \
			rsserver/p3face-msgs.cc \
			rsserver/p3face-server.cc \
			rsserver/p3history.cc \
			rsserver/p3msgs.cc \
			rsserver/p3peers.cc \
			rsserver/p3status.cc \
			rsserver/rsiface.cc \
			rsserver/rsinit.cc \
			rsserver/rsloginhandler.cc \
			rsserver/rstypes.cc \
			rsserver/p3serverconfig.cc

SOURCES += plugins/pluginmanager.cc \
				plugins/dlfcn_win32.cc \
				serialiser/rspluginitems.cc

SOURCES +=	serialiser/rsbaseitems.cc \
			serialiser/rsbaseserial.cc \
			serialiser/rsblogitems.cc \
			serialiser/rschannelitems.cc \
			serialiser/rsconfigitems.cc \
			serialiser/rsdiscitems.cc \
			serialiser/rsdistribitems.cc \
			serialiser/rsforumitems.cc \
			serialiser/rsgameitems.cc \
			serialiser/rshistoryitems.cc \
			serialiser/rsmsgitems.cc \
			serialiser/rsserial.cc \
			serialiser/rsstatusitems.cc \
			serialiser/rstlvaddrs.cc \
			serialiser/rstlvbase.cc \
			serialiser/rstlvfileitem.cc \
			serialiser/rstlvimage.cc \
			serialiser/rstlvkeys.cc \
			serialiser/rstlvkvwide.cc \
			serialiser/rstlvtypes.cc \
			serialiser/rstlvutil.cc \
			serialiser/rstlvdsdv.cc \
			serialiser/rsdsdvitems.cc \
			serialiser/rstlvbanlist.cc \
			serialiser/rsbanlistitems.cc \
			serialiser/rsbwctrlitems.cc \
			serialiser/rstunnelitems.cc

SOURCES +=	services/p3channels.cc \
			services/p3chatservice.cc \
			services/p3disc.cc \
			services/p3forums.cc \
			services/p3gamelauncher.cc \
			services/p3msgservice.cc \
			services/p3service.cc \
			services/p3statusservice.cc \
			services/p3dsdv.cc \
			services/p3banlist.cc \
			services/p3bwctrl.cc \

# removed because getPeer() doesn t exist			services/p3tunnel.cc

SOURCES += distrib/p3distrib.cc \
		   distrib/p3distribsecurity.cc 
 
SOURCES +=	turtle/p3turtle.cc \
				turtle/rsturtleitem.cc 
#				turtle/turtlerouting.cc \
#				turtle/turtlesearch.cc \
#				turtle/turtletunnels.cc


SOURCES +=	util/folderiterator.cc \
			util/rsdebug.cc \
			util/smallobject.cc \
			util/rsdir.cc \
			util/rsdiscspace.cc \
			util/rsnet.cc \
			util/extaddrfinder.cc \
			util/dnsresolver.cc \
			util/rsprint.cc \
			util/rsstring.cc \
			util/rsthreads.cc \
			util/rsversion.cc \
			util/rswin.cc \
			util/rsrandom.cc \
			util/rstickevent.cc \


upnp_miniupnpc {
	HEADERS += upnp/upnputil.h upnp/upnphandler_miniupnp.h
	SOURCES += upnp/upnputil.c upnp/upnphandler_miniupnp.cc
}

upnp_libupnp {
	HEADERS += upnp/UPnPBase.h  upnp/upnphandler_linux.h
	SOURCES += upnp/UPnPBase.cpp upnp/upnphandler_linux.cc
	DEFINES *= RS_USE_LIBUPNP
}



zeroconf {

HEADERS +=	zeroconf/p3zeroconf.h \

SOURCES +=	zeroconf/p3zeroconf.cc  \

# Disable Zeroconf (we still need the code for zcnatassist
#	DEFINES *= RS_ENABLE_ZEROCONF

}

# This is seperated from the above for windows/linux platforms.
# It is acceptable to build in zeroconf and have it not work, 
# but unacceptable to rely on Apple's libraries for Upnp when we have alternatives. '

zcnatassist {

HEADERS +=	zeroconf/p3zcnatassist.h \

SOURCES +=	zeroconf/p3zcnatassist.cc \

	DEFINES *= RS_ENABLE_ZCNATASSIST

}

# new gxs cache system
# this should be disabled for releases until further notice.
gxs {
	DEFINES *= RS_ENABLE_GXS

	#DEFINES *= GXS_DEV_TESTNET
	#DEFINES *= GXS_ENABLE_SYNC_MSGS

	HEADERS += serialiser/rsnxsitems.h \
		gxs/rsgds.h \
		gxs/rsgxs.h \
		gxs/rsdataservice.h \
		gxs/rsgxsnetservice.h \
		gxs/rsgxsflags.h \
		gxs/rsgenexchange.h \
		gxs/rsnxsobserver.h \
		gxs/rsgxsdata.h \
		gxs/rstokenservice.h \
		gxs/rsgxsdataaccess.h \
		retroshare/rsgxsservice.h \
		serialiser/rsgxsitems.h \
		util/retrodb.h \
		util/contentvalue.h \
		gxs/gxscoreserver.h \
		gxs/gxssecurity.h \
		gxs/rsgxsifaceimpl.h \
		gxs/gxstokenqueue.h \


	SOURCES += serialiser/rsnxsitems.cc \
		gxs/rsdataservice.cc \
		gxs/rsgenexchange.cc \
		gxs/rsgxsnetservice.cc \
		gxs/rsgxsdata.cc \
		serialiser/rsgxsitems.cc \
		gxs/rsgxsdataaccess.cc \
		util/retrodb.cc \
		util/contentvalue.cc \
		gxs/gxscoreserver.cc \
		gxs/gxssecurity.cc \
		gxs/rsgxsifaceimpl.cc \
		gxs/gxstokenqueue.cc \

	# Identity Service
	HEADERS += retroshare/rsidentity.h \
		gxs/rsgixs.h \
		services/p3idservice.h \
		serialiser/rsgxsiditems.h

	SOURCES += services/p3idservice.cc \
		serialiser/rsgxsiditems.cc \

	# GxsCircles Service
	HEADERS += services/p3gxscircles.h \
		serialiser/rsgxscircleitems.h \
		retroshare/rsgxscircles.h \

	SOURCES += services/p3gxscircles.cc \
		serialiser/rsgxscircleitems.cc \

	# GxsForums Service
	HEADERS += retroshare/rsgxsforums.h \
		services/p3gxsforums.h \
		serialiser/rsgxsforumitems.h

	SOURCES += services/p3gxsforums.cc \
		serialiser/rsgxsforumitems.cc \

	# Wiki Service
	HEADERS += retroshare/rswiki.h \
		services/p3wiki.h \
		serialiser/rswikiitems.h

	SOURCES += services/p3wiki.cc \
		serialiser/rswikiitems.cc \

	# Wiki Service
	HEADERS += retroshare/rswire.h \
		services/p3wire.h \
		serialiser/rswireitems.h

	SOURCES += services/p3wire.cc \
		serialiser/rswireitems.cc \

	# Posted Service
	HEADERS += services/p3posted.h \
		retroshare/rsposted.h \
		serialiser/rsposteditems.h

	SOURCES +=  services/p3posted.cc \
		serialiser/rsposteditems.cc

	#Photo Service
	HEADERS += services/p3photoservice.h \
		retroshare/rsphoto.h \
		serialiser/rsphotoitems.h \

	SOURCES += services/p3photoservice.cc \
		serialiser/rsphotoitems.cc \
}






###########################################################################################################
# OLD CONFIG OPTIONS.
# Not used much - but might be useful one day.
#

testnetwork {
	# used in rsserver/rsinit.cc Enabled Port Restrictions, and makes Proxy Port next to Dht Port.
	DEFINES *= LOCALNET_TESTING  

	# used in tcponudp/udprelay.cc Debugging Info for Relays.
	DEFINES *= DEBUG_UDP_RELAY

	# used in tcponudp/udpstunner.[h | cc] enables local stun (careful - modifies class variables).
	DEFINES *= UDPSTUN_ALLOW_LOCALNET

	# used in pqi/p3linkmgr.cc prints out extra debug.
	DEFINES *= LINKMGR_DEBUG_LINKTYPE

	# used in dht/connectstatebox to reduce connection times and display debug.
	# DEFINES *= TESTING_PERIODS
	# DEFINES *= DEBUG_CONNECTBOX
}


test_bitdht {
	# DISABLE TCP CONNECTIONS...
	DEFINES *= P3CONNMGR_NO_TCP_CONNECTIONS 

	# NO AUTO CONNECTIONS??? FOR TESTING DHT STATUS.
	DEFINES *= P3CONNMGR_NO_AUTO_CONNECTION 

	# ENABLED UDP NOW.
}




use_blogs {

	HEADERS +=	services/p3blogs.h
	SOURCES +=	services/p3blogs.cc 

	DEFINES *= RS_USE_BLOGS
}

