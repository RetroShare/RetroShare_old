
/*
 * "$Id: retroshare.cc,v 1.4 2007-04-21 19:08:51 rmf24 Exp $"
 *
 * RetroShare C++ Interface.
 *
 * Copyright 2004-2006 by Robert Fernie.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "retroshare@lunamutt.com".
 *
 */


#include <retroshare/rsiface.h>   /* definition of iface */
#include <retroshare/rsinit.h>   /* definition of iface */

#include "notifytxt.h"

#include <unistd.h>
#include <iostream>
#ifdef WINDOWS_SYS
#include <winsock2.h>
#endif
                                
#ifdef RS_INTRO_SERVER
#include "introserver.h"
#endif

/* Basic instructions for running libretroshare as background thread.
 * ******************************************************************* *
 * This allows your program to communicate with authenticated peers. 
 *
 * libretroshare's interfaces are defined in libretroshare/src/rsiface.
 * This should be the only headers that you need to include.
 *
 * The startup routine's are defined in rsiface.h
 */

int main(int argc, char **argv)
{
	/* Retroshare startup is configured using an RsInit object.
	 * This is an opaque class, which the user cannot directly tweak
	 * If you want to peek at whats happening underneath look in
	 * libretroshare/src/rsserver/p3face-startup.cc
	 *
	 * You create it with InitRsConfig(), and delete with CleanupRsConfig()
	 * InitRetroshare(argv, argc, config) parses the command line options, 
	 * and initialises the config paths.
	 *
	 * *** There are several functions that I should add to modify 
	 * **** the config the moment these can only be set via the commandline 
	 *   - RsConfigDirectory(...) is probably the most useful.
	 *   - RsConfigNetAddr(...) for setting port, etc.
	 *   - RsConfigOutput(...) for logging and debugging.
	 *
	 * Next you need to worry about loading your certificate, or making
	 * a new one:
	 *
	 *   RsGenerateCertificate(...) To create a new key, certificate 
	 *   LoadPassword(...) set password for existing certificate.
	 **/

	RsInit::InitRsConfig();
	int initResult = RsInit::InitRetroShare(argc, argv);

	if (initResult < 0) {
		/* Error occured */
		switch (initResult) {
		case RS_INIT_AUTH_FAILED:
			std::cerr << "RsInit::InitRetroShare AuthGPG::InitAuth failed" << std::endl;
			break;
		default:
			/* Unexpected return code */
			std::cerr << "RsInit::InitRetroShare unexpected return code " << initResult << std::endl;
			break;
		}
		return 1;
	}

	 /* load password should be called at this point: LoadPassword()
	  * otherwise loaded from commandline.
	  */


	/* Now setup the libretroshare interface objs 
	 * You will need to create you own NotifyXXX class
	 * if you want to receive notifications of events */

	NotifyTxt *notify = new NotifyTxt();
	RsIface *iface = createRsIface(*notify);
	RsControl *rsServer = createRsControl(*iface, *notify);
	rsicontrol = rsServer ;

	notify->setRsIface(iface);

	std::string preferredId, gpgId, gpgName, gpgEmail, sslName;
	RsInit::getPreferedAccountId(preferredId);

	if (RsInit::getAccountDetails(preferredId, gpgId, gpgName, gpgEmail, sslName))
	{
		RsInit::SelectGPGAccount(gpgId);
	}

	/* Key + Certificate are loaded into libretroshare */

	std::string error_string ;
	int retVal = RsInit::LockAndLoadCertificates(false,error_string);
	switch(retVal)
	{
		case 0:	break;
		case 1:	std::cerr << "Error: another instance of retroshare is already using this profile" << std::endl;
				return 1;
		case 2: std::cerr << "An unexpected error occurred while locking the profile" << std::endl;
				return 1;
		case 3: std::cerr << "An error occurred while login with the profile" << std::endl;
				return 1;
		default: std::cerr << "Main: Unexpected switch value " << retVal << std::endl;
				return 1;
	}

	/* Start-up libretroshare server threads */
	rsServer -> StartupRetroShare();

#ifdef RS_INTRO_SERVER
	RsIntroServer rsIS;
#endif
	
	/* pass control to the GUI */
	while(1)
	{
		std::cerr << "GUI Tick()" << std::endl;
#ifndef WINDOWS_SYS
		sleep(1);
#else
		Sleep(1000);
#endif

#ifdef RS_INTRO_SERVER
		rsIS.tick();
#endif
	}
	return 1;
}
