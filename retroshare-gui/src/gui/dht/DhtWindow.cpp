/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2011 Robert Fernie
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#include "DhtWindow.h"
#include "ui_DhtWindow.h"
#include <QTimer>
#include <QDateTime>

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <time.h>

#include "retroshare-gui/RsAutoUpdatePage.h"
#include "retroshare/rsdht.h"
#include "retroshare/rsconfig.h"
#include "retroshare/rspeers.h"

/********************************************** STATIC WINDOW *************************************/
DhtWindow * DhtWindow::mInstance = NULL;

void DhtWindow::showYourself()
{
    if (mInstance == NULL) {
        mInstance = new DhtWindow();
    }

    mInstance->show();
    mInstance->activateWindow();
}

DhtWindow* DhtWindow::getInstance()
{
    return mInstance;
}

void DhtWindow::releaseInstance()
{
    if (mInstance) {
        delete mInstance;
    }
}

/********************************************** STATIC WINDOW *************************************/



DhtWindow::DhtWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DhtWindow)
{
    ui->setupUi(this);

    setAttribute ( Qt::WA_DeleteOnClose, true );

	// tick for gui update.
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000);
}

DhtWindow::~DhtWindow()
{
    delete ui;
    mInstance = NULL;
}

void DhtWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void DhtWindow::update()
{
	if (!isVisible())
	{
#ifdef DEBUG_DHTWINDOW
		//std::cerr << "DhtWindow::update() !Visible" << std::endl;
#endif
		return;
	}

	/* do nothing if locked, or not visible */
	if (RsAutoUpdatePage::eventsLocked() == true) 
	{
#ifdef DEBUG_DHTWINDOW
		std::cerr << "DhtWindow::update() events Are Locked" << std::endl;
#endif
		return;
    	}

	if (!rsDht)
	{
#ifdef DEBUG_DHTWINDOW
		std::cerr << "DhtWindow::update rsDht NOT Set" << std::endl;
#endif
		return;
	}

	RsAutoUpdatePage::lockAllEvents();

	//std::cerr << "DhtWindow::update()" << std::endl;
	updateNetStatus();
	updateNetPeers();
	updateDhtPeers();
	updateRelays();

	RsAutoUpdatePage::unlockAllEvents() ;
}


void DhtWindow::updateNetStatus()
{

		QString status;
	QString oldstatus;

#if 0
		status = QString::fromStdString(mPeerNet->getPeerStatusString());
	oldstatus = ui->peerLine->text();
	if (oldstatus != status)
	{
		ui->peerLine->setText(status);
	}
#endif

		status = QString::fromStdString(rsDht->getUdpAddressString());
	oldstatus = ui->peerAddressLabel->text();
	if (oldstatus != status)
	{
		ui->peerAddressLabel->setText(status);
	}

	uint32_t netMode = rsConfig->getNetworkMode();

	QLabel *label = ui->networkLabel;
	switch(netMode)
	{
		case RSNET_NETWORK_UNKNOWN:
			label->setText("Unknown NetState");
			break;
		case RSNET_NETWORK_OFFLINE:
			label->setText("Offline");
			break;
		case RSNET_NETWORK_LOCALNET:
			label->setText("Local Net");
			break;
		case RSNET_NETWORK_BEHINDNAT:
			label->setText("Behind NAT");
			break;
		case RSNET_NETWORK_EXTERNALIP:
			label->setText("External IP");
			break;
	}

	label = ui->natTypeLabel;

	uint32_t natType = rsConfig->getNatTypeMode();
	switch(natType)
	{
		case RSNET_NATTYPE_UNKNOWN:
			label->setText("UNKNOWN NAT STATE");
			break;
		case RSNET_NATTYPE_SYMMETRIC:
			label->setText("SYMMETRIC NAT");
			break;
		case RSNET_NATTYPE_DETERM_SYM:
			label->setText("DETERMINISTIC SYM NAT");
			break;
		case RSNET_NATTYPE_RESTRICTED_CONE:
			label->setText("RESTRICTED CONE NAT");
			break;
		case RSNET_NATTYPE_FULL_CONE:
			label->setText("FULL CONE NAT");
			break;
		case RSNET_NATTYPE_OTHER:
			label->setText("OTHER NAT");
			break;
		case RSNET_NATTYPE_NONE:
			label->setText("NO NAT");
			break;
	}



	label = ui->natHoleLabel;
	uint32_t natHole = rsConfig->getNatHoleMode();

	switch(natHole)
	{
		case RSNET_NATHOLE_UNKNOWN:
			label->setText("UNKNOWN NAT HOLE STATUS");
			break;
		case RSNET_NATHOLE_NONE:
			label->setText("NO NAT HOLE");
			break;
		case RSNET_NATHOLE_UPNP:
			label->setText("UPNP FORWARD");
			break;
		case RSNET_NATHOLE_NATPMP:
			label->setText("NATPMP FORWARD");
			break;
		case RSNET_NATHOLE_FORWARDED:
			label->setText("MANUAL FORWARD");
			break;
	}

	uint32_t connect = rsConfig->getConnectModes();

	label = ui->connectLabel;
	QString connOut;
	if (connect & RSNET_CONNECT_OUTGOING_TCP)
	{
		connOut += "TCP_OUT ";
	}
	if (connect & RSNET_CONNECT_ACCEPT_TCP)
	{
		connOut += "TCP_IN ";
	}
	if (connect & RSNET_CONNECT_DIRECT_UDP)
	{
		connOut += "DIRECT_UDP ";
	}
	if (connect & RSNET_CONNECT_PROXY_UDP)
	{
		connOut += "PROXY_UDP ";
	}
	if (connect & RSNET_CONNECT_RELAY_UDP)
	{
		connOut += "RELAY_UDP ";
	}

	label->setText(connOut);

	uint32_t netState = rsConfig->getNetState();

	label = ui->netStatusLabel;
	switch(netState)
	{
		case RSNET_NETSTATE_BAD_UNKNOWN:
			label->setText("NET BAD: Unknown State");
			break;
		case RSNET_NETSTATE_BAD_OFFLINE:
			label->setText("NET BAD: Offline");
			break;
		case RSNET_NETSTATE_BAD_NATSYM:
			label->setText("NET BAD: Behind Symmetric NAT");
			break;
		case RSNET_NETSTATE_BAD_NODHT_NAT:
			label->setText("NET BAD: Behind NAT & No DHT");
			break;
		case RSNET_NETSTATE_WARNING_RESTART:
			label->setText("NET WARNING: NET Restart");
			break;
		case RSNET_NETSTATE_WARNING_NATTED:
			label->setText("NET WARNING: Behind NAT");
			break;
		case RSNET_NETSTATE_WARNING_NODHT:
			label->setText("NET WARNING: No DHT");
			break;
		case RSNET_NETSTATE_GOOD:
			label->setText("NET STATE GOOD!");
			break;
		case RSNET_NETSTATE_ADV_FORWARD:
			label->setText("CAUTION: UNVERIFABLE FORWARD!");
			break;
		case RSNET_NETSTATE_ADV_DARK_FORWARD:
			label->setText("CAUTION: UNVERIFABLE FORWARD & NO DHT");
			break;
	}
}

void DhtWindow::updateNetPeers()
{
	QTreeWidget *peerTreeWidget = ui->peerTreeWidget;

	std::list<std::string> peerIds;
	std::list<std::string>::iterator it;

	rsDht->getNetPeerList(peerIds);

	/* collate peer stats */
	int nPeers = peerIds.size();

	// from DHT peers
	int nOnlinePeers = 0;
	int nUnreachablePeers = 0;
	int nOfflinePeers = 0;

	// Connect States.
	int nDisconnPeers = 0;
	int nDirectPeers = 0;
	int nProxyPeers = 0;
	int nRelayPeers = 0;


#define PTW_COL_RSNAME			0
#define PTW_COL_PEERID			1
#define PTW_COL_DHT_STATUS		2
	
#define PTW_COL_PEER_CONNECTLOGIC	3

#define PTW_COL_PEER_CONNECT_STATUS	4
#define PTW_COL_PEER_CONNECT_MODE	5
#define PTW_COL_PEER_REQ_STATUS		6
	
#define PTW_COL_PEER_CB_MSG		7
#define PTW_COL_RSID			8

#if 0
	/* clear old entries */
	int itemCount = peerTreeWidget->topLevelItemCount();
	for (int nIndex = 0; nIndex < itemCount;) 
	{
		QTreeWidgetItem *tmp_item = peerTreeWidget->topLevelItem(nIndex);
		std::string tmpid = tmp_item->data(PTW_COL_PEERID, Qt::DisplayRole).toString().toStdString();
		if (peerIds.end() == std::find(peerIds.begin(), peerIds.end(), tmpid))
		{
			peerTreeWidget->removeItemWidget(tmp_item, 0);
			/* remove it! */
			itemCount--;
		}
		else
		{
			nIndex++;
		}
	}
#endif
	peerTreeWidget->clear();

	for(it = peerIds.begin(); it != peerIds.end(); it++)
	{
		/* find the entry */
		QTreeWidgetItem *peer_item = NULL;
#if 0
		QString qpeerid = QString::fromStdString(*it);
		int itemCount = peerTreeWidget->topLevelItemCount();
		for (int nIndex = 0; nIndex < itemCount; nIndex++) 
		{
			QTreeWidgetItem *tmp_item = peerTreeWidget->topLevelItem(nIndex);
			if (tmp_item->data(PTW_COL_PEERID, Qt::DisplayRole).toString() == qpeerid) 
			{
				peer_item = tmp_item;
				break;
			}
		}
#endif

		if (!peer_item)
		{
			/* insert */
			peer_item = new QTreeWidgetItem();
			peerTreeWidget->addTopLevelItem(peer_item);
		}

		/* update the data */
		RsDhtNetPeer status;
		rsDht->getNetPeerStatus(*it, status);

		std::string name = rsPeers->getPeerName(*it);

		peer_item -> setData(PTW_COL_PEERID, Qt::DisplayRole, QString::fromStdString(status.mDhtId));
		peer_item -> setData(PTW_COL_RSNAME, Qt::DisplayRole, QString::fromStdString(name));
		peer_item -> setData(PTW_COL_RSID, Qt::DisplayRole, QString::fromStdString(status.mRsId));

		QString dhtstate;
		switch(status.mDhtState)
		{
			default:
			case RSDHT_PEERDHT_NOT_ACTIVE:
				dhtstate = "Not Active (Maybe Connected!)";
				break;
			case RSDHT_PEERDHT_SEARCHING:
				dhtstate = "Searching";
				break;
			case RSDHT_PEERDHT_FAILURE:
				dhtstate = "Failed";
				break;
			case RSDHT_PEERDHT_OFFLINE:
				dhtstate = "offline";
				nOfflinePeers++;
				break;
			case RSDHT_PEERDHT_UNREACHABLE:
				dhtstate = "Unreachable";
				nUnreachablePeers++;
				break;
			case RSDHT_PEERDHT_ONLINE:
				dhtstate = "ONLINE";
				nOnlinePeers++;
				break;
		}
			
		peer_item -> setData(PTW_COL_DHT_STATUS, Qt::DisplayRole, dhtstate);

		// NOW CONNECT STATE
		QString cpmstr;
		switch(status.mPeerConnectMode)
		{
			case RSDHT_TOU_MODE_DIRECT:
				cpmstr = "Direct";
				break;
			case RSDHT_TOU_MODE_PROXY:
				cpmstr = "Proxy VIA " + QString::fromStdString(status.mPeerConnectProxyId);
				break;
			case RSDHT_TOU_MODE_RELAY:
				cpmstr = "Relay VIA " + QString::fromStdString(status.mPeerConnectProxyId);
				break;
			default:
				cpmstr = "None";
				break;
		}


		QString cpsstr;
		switch(status.mPeerConnectState)
		{
			default:
			case RSDHT_PEERCONN_DISCONNECTED:
				cpsstr = "Disconnected";
				nDisconnPeers++;
				break;
			case RSDHT_PEERCONN_UDP_STARTED:
				cpsstr = "Udp Started";
				break;
			case RSDHT_PEERCONN_CONNECTED:
			{
				cpsstr = "Connected";
				break;
				switch(status.mPeerConnectMode)
				{
					default:
					case RSDHT_TOU_MODE_DIRECT:
						nDirectPeers++;
						break;
					case RSDHT_TOU_MODE_PROXY:
						nProxyPeers++;
						break;
					case RSDHT_TOU_MODE_RELAY:
						nRelayPeers++;
						break;
				}
			}
				break;
		}

		peer_item -> setData(PTW_COL_PEER_CONNECT_STATUS, Qt::DisplayRole, cpsstr);
		
		if (status.mPeerConnectState == RSDHT_PEERCONN_DISCONNECTED)
		{
			peer_item -> setData(PTW_COL_PEER_CONNECT_MODE, Qt::DisplayRole, "");
		}
		else 
		{
			peer_item -> setData(PTW_COL_PEER_CONNECT_MODE, Qt::DisplayRole, cpmstr);
		}
	
		// NOW REQ STATE.
		QString reqstr;
		if (status.mExclusiveProxyLock)
		{
				reqstr = "(E) ";
		}
		switch(status.mPeerReqState)
		{
			case RSDHT_PEERREQ_RUNNING:
				reqstr += "Request Active";
				break;
			case RSDHT_PEERREQ_STOPPED:
				reqstr += "No Request";
				break;
			default:
				reqstr += "Unknown";
				break;
		}
		peer_item -> setData(PTW_COL_PEER_REQ_STATUS, Qt::DisplayRole, reqstr);

		peer_item -> setData(PTW_COL_PEER_CB_MSG, Qt::DisplayRole, QString::fromStdString(status.mCbPeerMsg));
		peer_item -> setData(PTW_COL_PEER_CONNECTLOGIC, Qt::DisplayRole, 
						QString::fromStdString(status.mConnectState));
	}


	QString connstr;
	connstr =  "#Peers: " + QString::number(nPeers);
	connstr += " DHT: (#off:" + QString::number(nOfflinePeers);
	connstr += ",unreach:" + QString::number(nUnreachablePeers);
	connstr += ",online:" + QString::number(nOnlinePeers);
	connstr += ") Connections: (#dis:" + QString::number(nDisconnPeers);
	connstr += ",#dir:" + QString::number(nDirectPeers);
	connstr += ",#proxy:" + QString::number(nProxyPeers);
	connstr += ",#relay:" + QString::number(nRelayPeers);
	connstr += ")";

	ui->peerSummaryLabel->setText(connstr);
}



void DhtWindow::updateRelays()
{

	QTreeWidget *relayTreeWidget = ui->relayTreeWidget;

	std::list<RsDhtRelayEnd> relayEnds;
	std::list<RsDhtRelayProxy> relayProxies;

	std::list<RsDhtRelayEnd>::iterator reit;
	std::list<RsDhtRelayProxy>::iterator rpit;

	rsDht->getRelayEnds(relayEnds);
	rsDht->getRelayProxies(relayProxies);


#define RTW_COL_TYPE		0
#define RTW_COL_SRC		1
#define RTW_COL_PROXY		2
#define RTW_COL_DEST		3
#define RTW_COL_CLASS		4
#define RTW_COL_AGE		5
#define RTW_COL_LASTSEND	6
#define RTW_COL_BANDWIDTH	7

	relayTreeWidget->clear();
	time_t now = time(NULL);

	for(reit = relayEnds.begin(); reit != relayEnds.end(); reit++)
	{
		/* find the entry */
		QTreeWidgetItem *item = new QTreeWidgetItem();
		relayTreeWidget->addTopLevelItem(item);

		QString typestr = "RELAY END";
		QString srcstr = "Yourself";
		QString proxystr = QString::fromStdString(reit->mProxyAddr);
		QString deststr = QString::fromStdString(reit->mRemoteAddr);
		QString agestr = "unknown";
		QString lastsendstr = "unknown";
		QString bandwidthstr = "unlimited";
		QString classstr = "Own Relay";

		//std::ostringstream dhtupdatestr;
		//dhtupdatestr << now - status.mDhtUpdateTS << " secs ago";

		item -> setData(RTW_COL_TYPE, Qt::DisplayRole, typestr);
		item -> setData(RTW_COL_SRC, Qt::DisplayRole, srcstr);
		item -> setData(RTW_COL_PROXY, Qt::DisplayRole, proxystr);
		item -> setData(RTW_COL_DEST, Qt::DisplayRole, deststr);
		item -> setData(RTW_COL_CLASS, Qt::DisplayRole, classstr);
		item -> setData(RTW_COL_AGE, Qt::DisplayRole, agestr);
		item -> setData(RTW_COL_LASTSEND, Qt::DisplayRole, lastsendstr);
		item -> setData(RTW_COL_BANDWIDTH, Qt::DisplayRole, bandwidthstr);

	}


	for(rpit = relayProxies.begin(); rpit != relayProxies.end(); rpit++)
	{
		/* find the entry */
		QTreeWidgetItem *item = new QTreeWidgetItem();
		relayTreeWidget->addTopLevelItem(item);

		QString typestr = "RELAY PROXY";
		QString srcstr = QString::fromStdString(rpit->mSrcAddr);
		QString proxystr = "Yourself";
		QString deststr = QString::fromStdString(rpit->mDestAddr);
		QString agestr = QString("%1 secs ago").arg(now - rpit->mCreateTS);
		QString lastsendstr = QString("%1 secs ago").arg(now - rpit->mLastTS);
		QString bandwidthstr = QString("%1B/s").arg(QString::number(rpit->mBandwidth));
		QString classstr = QString::number(rpit->mRelayClass);

		item -> setData(RTW_COL_TYPE, Qt::DisplayRole, typestr);
		item -> setData(RTW_COL_SRC, Qt::DisplayRole, srcstr);
		item -> setData(RTW_COL_PROXY, Qt::DisplayRole, proxystr);
		item -> setData(RTW_COL_DEST, Qt::DisplayRole, deststr);
		item -> setData(RTW_COL_CLASS, Qt::DisplayRole, classstr);
		item -> setData(RTW_COL_AGE, Qt::DisplayRole, agestr);
		item -> setData(RTW_COL_LASTSEND, Qt::DisplayRole, lastsendstr);
		item -> setData(RTW_COL_BANDWIDTH, Qt::DisplayRole, bandwidthstr);

	}
}



/****************************/


#define DTW_COL_BUCKET	0
#define DTW_COL_IPADDR	1
#define DTW_COL_PEERID	2
#define DTW_COL_FLAGS	3
#define DTW_COL_FOUND	4
#define DTW_COL_SEND	5
#define DTW_COL_RECV	6

class DhtTreeWidgetItem : public QTreeWidgetItem
    {
public:
    virtual bool operator<(const QTreeWidgetItem &other) const
	{
	  int column = treeWidget()->sortColumn();
	  if (column == DTW_COL_RECV || column == DTW_COL_SEND
	      || column == DTW_COL_BUCKET)
	      {
	      QString t1 = text(column);
	      QString t2 = other.text(column);
	      t1 = t1.left(t1.indexOf(' '));
	      t2 = t2.left(t2.indexOf(' '));
	      return t1.toLong() < t2.toLong();
	      }
	  return text(column) < other.text(column);
	}
    };

void DhtWindow::updateDhtPeers()
{

	/* Hackish display of all Dht peers, should be split into buckets (as children) */
        //QString status = QString::fromStdString(mPeerNet->getDhtStatusString());
	//ui->dhtLabel->setText(status);
	
	std::list<RsDhtPeer> allpeers;
	std::list<RsDhtPeer>::iterator it;
	int i;
	for(i = 0; i < 160; i++)
	{
		std::list<RsDhtPeer> peers;
        	rsDht->getDhtPeers(i, peers);

		for(it = peers.begin(); it != peers.end(); it++)
		{
			allpeers.push_back(*it);
		}
	}

	QTreeWidget *dhtTreeWidget = ui->dhtTreeWidget;

	dhtTreeWidget->clear();

	time_t now = time(NULL);
	for(it = allpeers.begin(); it != allpeers.end(); it++)
	{
		/* find the entry */
		QTreeWidgetItem *dht_item = NULL;

		/* insert */
		dht_item = new DhtTreeWidgetItem();

		QString buckstr = QString::number(it->mBucket);
		QString ipstr = QString::fromStdString(it->mAddr);
		QString idstr = QString::fromStdString(it->mDhtId);
		QString flagsstr = QString("0x%1 EX:0x%2").arg(it->mPeerFlags, 0, 16, QChar('0')).arg(it->mExtraFlags, 0, 16, QChar('0'));
		QString foundstr = QString("%1 secs ago").arg(now - it->mFoundTime);

		QString lastsendstr;
		if (it->mLastSendTime == 0)
		{
			lastsendstr = "never";
		}
		else
		{
			lastsendstr = QString ("%1 secs ago").arg(now - it->mLastSendTime);
		}

		QString lastrecvstr = QString ("%1 secs ago").arg(now - it->mLastRecvTime);

		dht_item -> setData(DTW_COL_BUCKET, Qt::DisplayRole, buckstr);
		dht_item -> setData(DTW_COL_IPADDR, Qt::DisplayRole, ipstr);
		dht_item -> setData(DTW_COL_PEERID, Qt::DisplayRole, idstr);
		dht_item -> setData(DTW_COL_FLAGS, Qt::DisplayRole, flagsstr);

		dht_item -> setData(DTW_COL_FOUND, Qt::DisplayRole, foundstr);
		dht_item -> setData(DTW_COL_SEND, Qt::DisplayRole, lastsendstr);
		dht_item -> setData(DTW_COL_RECV, Qt::DisplayRole, lastrecvstr);

		dhtTreeWidget->addTopLevelItem(dht_item);
	}

}


