/****************************************************************
 *
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2011, csoler  
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

#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>

#include "ChatLobbyDialog.h"
#include "gui/ChatLobbyWidget.h"
#include "ChatTabWidget.h"
#include "gui/settings/rsharesettings.h"
#include "gui/settings/RsharePeerSettings.h"
#include "gui/MainWindow.h"
#include "gui/FriendsDialog.h"
#include <gui/common/html.h>
#include "gui/common/RSTreeWidgetItem.h"
#include "gui/common/FriendSelectionDialog.h"
#include "gui/gxs/GxsIdTreeWidgetItem.h"
#include "util/HandleRichText.h"

#include <retroshare/rsnotify.h>

#include <time.h>

#define COLUMN_ICON  0
#define COLUMN_NAME  1
#define COLUMN_ACTIVITY  2
#define COLUMN_COUNT     3

/** Default constructor */
ChatLobbyDialog::ChatLobbyDialog(const ChatLobbyId& lid, QWidget *parent, Qt::WindowFlags flags)
	: ChatDialog(parent, flags), lobbyId(lid)
{
	/* Invoke Qt Designer generated QObject setup routine */
	ui.setupUi(this);

	connect(ui.participantsFrameButton, SIGNAL(toggled(bool)), this, SLOT(showParticipantsFrame(bool)));
    //connect(ui.actionChangeNickname, SIGNAL(triggered()), this, SLOT(changeNickname()));
	connect(ui.participantsList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(participantsTreeWidgetCustomPopupMenu(QPoint)));
	connect(ui.participantsList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(participantsTreeWidgetDoubleClicked(QTreeWidgetItem*,int)));

	ui.participantsList->setColumnCount(COLUMN_COUNT);
	ui.participantsList->setColumnWidth(COLUMN_ICON, 20);
    ui.participantsList->setColumnHidden(COLUMN_ACTIVITY,true);

	muteAct = new QAction(QIcon(), tr("Mute participant"), this);
	connect(muteAct, SIGNAL(triggered()), this, SLOT(changePartipationState()));

	// Add a button to invite friends.
	//
	inviteFriendsButton = new QToolButton ;
	inviteFriendsButton->setMinimumSize(QSize(28,28)) ;
	inviteFriendsButton->setMaximumSize(QSize(28,28)) ;
	inviteFriendsButton->setText(QString()) ;
	inviteFriendsButton->setAutoRaise(true) ;
	inviteFriendsButton->setToolTip(tr("Invite friends to this lobby"));

        mParticipantCompareRole = new RSTreeWidgetItemCompareRole;
        mParticipantCompareRole->setRole(0, Qt::UserRole);

	{
	QIcon icon ;
	icon.addPixmap(QPixmap(":/images/edit_add24.png")) ;
	inviteFriendsButton->setIcon(icon) ;
	inviteFriendsButton->setIconSize(QSize(22,22)) ;
	}

	connect(inviteFriendsButton, SIGNAL(clicked()), this , SLOT(inviteFriends()));

	getChatWidget()->addChatBarWidget(inviteFriendsButton) ;

	unsubscribeButton = new QToolButton ;
	unsubscribeButton->setMinimumSize(QSize(28,28)) ;
	unsubscribeButton->setMaximumSize(QSize(28,28)) ;
	unsubscribeButton->setText(QString()) ;
	unsubscribeButton->setAutoRaise(true) ;
	unsubscribeButton->setToolTip(tr("Leave this lobby (Unsubscribe)"));

	{
	QIcon icon ;
	icon.addPixmap(QPixmap(":/images/door_in.png")) ;
	unsubscribeButton->setIcon(icon) ;
	unsubscribeButton->setIconSize(QSize(24,24)) ;
	}

	/* Initialize splitter */
	ui.splitter->setStretchFactor(0, 1);
	ui.splitter->setStretchFactor(1, 0);

	connect(unsubscribeButton, SIGNAL(clicked()), this , SLOT(leaveLobby()));

	getChatWidget()->addChatBarWidget(unsubscribeButton) ;
}

void ChatLobbyDialog::leaveLobby()
{
	emit lobbyLeave(id()) ;
}
void ChatLobbyDialog::inviteFriends()
{
	std::cerr << "Inviting friends" << std::endl;

    std::list<RsPeerId> ids = FriendSelectionDialog::selectFriends_SSL(NULL,tr("Invite friends"),tr("Select friends to invite:")) ;

	std::cerr << "Inviting these friends:" << std::endl;

    if (!mChatId.isLobbyId())
		return ;

    for(std::list<RsPeerId>::const_iterator it(ids.begin());it!=ids.end();++it)
	{
		std::cerr << "    " << *it  << std::endl;

        rsMsgs->invitePeerToLobby(mChatId.toLobbyId(),*it) ;
	}
}

void ChatLobbyDialog::participantsTreeWidgetCustomPopupMenu(QPoint)
{
	QList<QTreeWidgetItem*> selectedItems = ui.participantsList->selectedItems();

	QMenu contextMnu(this);

	contextMnu.addAction(muteAct);
	muteAct->setCheckable(true);
	muteAct->setEnabled(false);
	muteAct->setChecked(false);
	if (selectedItems.size()) {

        RsGxsId nickName;
        rsMsgs->getIdentityForChatLobby(lobbyId, nickName);

        if(selectedItems.count()>1 || (RsGxsId(selectedItems.at(0)->text(COLUMN_NAME).toStdString())!=nickName))
        {
		muteAct->setEnabled(true);

		QList<QTreeWidgetItem*>::iterator item;
        for (item = selectedItems.begin(); item != selectedItems.end(); ++item) {

            RsGxsId gxsid ;
            if ( dynamic_cast<GxsIdRSTreeWidgetItem*>(*item)->getId(gxsid) && isParticipantMuted(gxsid))
            {
				muteAct->setChecked(true);
				break;
			}
		}
	}
    }

	contextMnu.exec(QCursor::pos());
}

void ChatLobbyDialog::init()
{
    ChatLobbyInfo linfo ;

    QString title;

    std::list<ChatLobbyInfo>::const_iterator lobbyIt;

    if(rsMsgs->getChatLobbyInfo(lobbyId,linfo))
    {
        title = QString::fromUtf8(linfo.lobby_name.c_str());

        QString msg = tr("Welcome to lobby %1").arg(RsHtml::plainText(linfo.lobby_name));
        _lobby_name = QString::fromUtf8(linfo.lobby_name.c_str()) ;
        if (!linfo.lobby_topic.empty()) {
            msg += "\n" + tr("Topic: %1").arg(RsHtml::plainText(linfo.lobby_topic));
        }
        ui.chatWidget->setWelcomeMessage(msg);
    }

    ChatDialog::init(ChatId(lobbyId), title);

    RsGxsId gxs_id;
    rsMsgs->getIdentityForChatLobby(lobbyId, gxs_id);

    RsIdentityDetails details ;
    rsIdentity->getIdDetails(gxs_id,details) ;

    ui.chatWidget->setName(QString::fromUtf8(details.mNickname.c_str()));
    ui.chatWidget->addToolsAction(ui.actionChangeNickname);
    ui.chatWidget->setDefaultExtraFileFlags(RS_FILE_REQ_ANONYMOUS_ROUTING);

    lastUpdateListTime = 0;

    /* Hide or show the participants frames */
    showParticipantsFrame(PeerSettings->getShowParticipantsFrame(ChatId(lobbyId)));

    // add to window

    dynamic_cast<ChatLobbyWidget*>(MainWindow::getPage(MainWindow::ChatLobby))->addChatPage(this) ;

    /** List of muted Participants */
    mutedParticipants.clear() ;

    // load settings
    processSettings(true);
}

/** Destructor. */
ChatLobbyDialog::~ChatLobbyDialog()
{
	// announce leaving of lobby

	// check that the lobby still exists.
    if (mChatId.isLobbyId()) {
        rsMsgs->unsubscribeChatLobby(mChatId.toLobbyId());
	}

	// save settings
	processSettings(false);
}

ChatWidget *ChatLobbyDialog::getChatWidget()
{
	return ui.chatWidget;
}

bool ChatLobbyDialog::notifyBlink()
{
	return (Settings->getChatLobbyFlags() & RS_CHATLOBBY_BLINK);
}

void ChatLobbyDialog::processSettings(bool load)
{
	Settings->beginGroup(QString("ChatLobbyDialog"));

	if (load) {
		// load settings
	} else {
		// save settings
	}

	Settings->endGroup();
}

/**
 * Change your Nickname
 * 
 * - send a Message to all Members => later: send hidden message to clients, so they can actualize there mutedParticipants list
 */
void ChatLobbyDialog::setIdentity(const RsGxsId& gxs_id)
{
    rsMsgs->setIdentityForChatLobby(lobbyId, gxs_id) ;

	// get new nick name
    RsGxsId newid;

    if (rsMsgs->getIdentityForChatLobby(lobbyId, newid))
    {
        RsIdentityDetails details ;
        rsIdentity->getIdDetails(gxs_id,details) ;

        ui.chatWidget->setName(QString::fromUtf8(details.mNickname.c_str()));
	}
}

/**
 * Dialog: Change your Nickname in the ChatLobby
 */
//void ChatLobbyDialog::changeNickname()
//{
//	QInputDialog dialog;
//	dialog.setWindowTitle(tr("Change nick name"));
//	dialog.setLabelText(tr("Please enter your new nick name"));
//
//	std::string nickName;
//	rsMsgs->getNickNameForChatLobby(lobbyId, nickName);
//	dialog.setTextValue(QString::fromUtf8(nickName.c_str()));
//
//	if (dialog.exec() == QDialog::Accepted && !dialog.textValue().isEmpty()) {
//		setNickname(dialog.textValue());
//	}
//}

/**
 * We get a new Message from a chat participant
 * 
 * - Ignore Messages from muted chat participants
 */
void ChatLobbyDialog::addChatMsg(const ChatMessage& msg)
{
    QDateTime sendTime = QDateTime::fromTime_t(msg.sendTime);
    QDateTime recvTime = QDateTime::fromTime_t(msg.recvTime);
    QString message = QString::fromUtf8(msg.msg.c_str());
    RsGxsId gxs_id = msg.lobby_peer_gxs_id ;
	
    if(!isParticipantMuted(gxs_id)) {
#warning name conversion missing for gxs_id
      ui.chatWidget->addChatMsg(msg.incoming, QString::fromStdString(gxs_id.toStdString()), sendTime, recvTime, message, ChatWidget::MSGTYPE_NORMAL);
		emit messageReceived(id()) ;
	}
	
	// This is a trick to translate HTML into text.
	QTextEdit editor;
	editor.setHtml(message);
    QString notifyMsg = QString::fromStdString(gxs_id.toStdString()) + ": " + editor.toPlainText();

	if(notifyMsg.length() > 30)
		MainWindow::displayLobbySystrayMsg(tr("Lobby chat") + ": " + _lobby_name, notifyMsg.left(30) + QString("..."));
	else
		MainWindow::displayLobbySystrayMsg(tr("Lobby chat") + ": " + _lobby_name, notifyMsg);

	// also update peer list.

	time_t now = time(NULL);

#warning check that addChatMessage() correctly updates participant activity
   QList<QTreeWidgetItem*>  qlFoundParticipants=ui.participantsList->findItems(QString::fromStdString(gxs_id.toStdString()),Qt::MatchExactly,COLUMN_NAME);
    if (qlFoundParticipants.count()!=0) qlFoundParticipants.at(0)->setText(COLUMN_ACTIVITY,QString::number(now));

	if (now > lastUpdateListTime) {
		lastUpdateListTime = now;
		updateParticipantsList();
	}
}

/**
 * Regenerate the QTreeWidget participant list of a Chat Lobby
 * 
 * Show yellow icon for muted Participants
 */
void ChatLobbyDialog::updateParticipantsList()
{
    ChatLobbyInfo linfo;

    if(rsMsgs->getChatLobbyInfo(lobbyId,linfo))
    {
        ChatLobbyInfo cliInfo=linfo;
        QList<QTreeWidgetItem*>  qlOldParticipants=ui.participantsList->findItems("*",Qt::MatchWildcard,COLUMN_NAME);

        foreach(QTreeWidgetItem *qtwiCur,qlOldParticipants){
            QString qsOldParticipant = qtwiCur->text(COLUMN_NAME);
            QByteArray qbaPart=qsOldParticipant.toUtf8();
            std::string strTemp=std::string(qbaPart.begin(),qbaPart.end());

            std::map<RsGxsId,time_t>::iterator itFound(cliInfo.gxs_ids.find(RsGxsId(strTemp))) ;

            if(itFound == cliInfo.gxs_ids.end())
            {
                //Old Participant go out, remove it
                int index = ui.participantsList->indexOfTopLevelItem(qtwiCur);
                delete ui.participantsList->takeTopLevelItem(index);
            }
        }



    for (std::map<RsGxsId,time_t>::const_iterator it2(linfo.gxs_ids.begin()); it2 != linfo.gxs_ids.end(); ++it2)
    {
        QString participant = QString::fromUtf8( (it2->first).toStdString().c_str() );

        QList<QTreeWidgetItem*>  qlFoundParticipants=ui.participantsList->findItems(participant,Qt::MatchExactly,COLUMN_NAME);
        GxsIdRSTreeWidgetItem *widgetitem;

        if (qlFoundParticipants.count()==0)
        {
            // TE: Add Wigdet to participantsList with Checkbox, to mute Participant

            widgetitem = new GxsIdRSTreeWidgetItem(mParticipantCompareRole);
            widgetitem->setId(it2->first,0) ;
            //widgetitem->setText(COLUMN_NAME, participant);
            //widgetitem->setText(COLUMN_ACTIVITY,QString::number(time(NULL)));
            ui.participantsList->addTopLevelItem(widgetitem);
        }
        else
            widgetitem = dynamic_cast<GxsIdRSTreeWidgetItem*>(qlFoundParticipants.at(0));

#warning finish updating the colors of GXS ids in participant list
        if (isParticipantMuted(it2->first)) {
            //widgetitem->setIcon(COLUMN_ICON, QIcon(":/images/redled.png"));
            widgetitem->setTextColor(COLUMN_NAME,QColor(255,0,0));
        } else {
            //widgetitem->setIcon(COLUMN_ICON, QIcon(":/images/greenled.png"));
            widgetitem->setTextColor(COLUMN_NAME,ui.participantsList->palette().color(QPalette::Active, QPalette::Text));
        }

        time_t tLastAct=widgetitem->text(COLUMN_ACTIVITY).toInt();
        time_t now = time(NULL);
        if (tLastAct<now-60*30)
            widgetitem->setIcon(COLUMN_ICON, QIcon(isParticipantMuted(it2->first)?":/images/ledoff1.png":":/images/grayled.png"));

        RsGxsId gxs_id;
        rsMsgs->getIdentityForChatLobby(lobbyId, gxs_id);

        if (RsGxsId(participant.toStdString()) == gxs_id) widgetitem->setIcon(COLUMN_ICON, QIcon(":/images/yellowled.png"));


        QTime qtLastAct=QTime(0,0,0).addSecs(now-tLastAct);
        widgetitem->setToolTip(COLUMN_NAME,tr("Right click to mute/unmute participants<br/>Double click to address this person<br/>")
                               +tr("This participant is not active since:")
                               +qtLastAct.toString()
                               +tr(" seconds")
                               );
    }
	}
	ui.participantsList->setSortingEnabled(true);
	ui.participantsList->sortItems(COLUMN_NAME, Qt::AscendingOrder);
}

/**
 * Called when a Participant in QTree get Clicked / Changed
 * 
 * Check if the Checkbox altered and Mute User
 * 
 * @todo auf rsid
 * 
 * @param QTreeWidgetItem Participant to check
 */
void ChatLobbyDialog::changePartipationState()
{
    QList<QTreeWidgetItem*> selectedItems = ui.participantsList->selectedItems();

	if (selectedItems.isEmpty()) {
		return;
	}

	QList<QTreeWidgetItem*>::iterator item;
    for (item = selectedItems.begin(); item != selectedItems.end(); ++item) {

        RsGxsId gxs_id ;
        dynamic_cast<GxsIdRSTreeWidgetItem*>(*item)->getId(gxs_id) ;

        std::cerr << "check Partipation status for '" << gxs_id << std::endl;

		if (muteAct->isChecked()) {
            muteParticipant(gxs_id);
		} else {
            unMuteParticipant(gxs_id);
		}
	}

	updateParticipantsList();
}

void ChatLobbyDialog::participantsTreeWidgetDoubleClicked(QTreeWidgetItem *item, int column)
{
	if (!item) {
		return;
	}

	if(column == COLUMN_NAME)
	{
		getChatWidget()->pasteText("@" + RsHtml::plainText(item->text(COLUMN_NAME))) ;
		return ;
	}

//	if (column == COLUMN_ICON) {
//		return;
//	}
//
//	QString nickname = item->text(COLUMN_NAME);
//	if (isParticipantMuted(nickname)) {
//		unMuteParticipant(nickname);
//	} else {
//		muteParticipant(nickname);
//	}
//
//	mutedParticipants->removeDuplicates();
//
//	updateParticipantsList();
}

void ChatLobbyDialog::muteParticipant(const RsGxsId& nickname)
{
    std::cerr << " Mute " << std::endl;

    RsGxsId gxs_id;
    rsMsgs->getIdentityForChatLobby(lobbyId, gxs_id);

    if (gxs_id!=nickname)
        mutedParticipants.insert(nickname);
}

void ChatLobbyDialog::unMuteParticipant(const RsGxsId& id)
{
    std::cerr << " UnMute " << std::endl;
    mutedParticipants.erase(id);
}

/**
 * Is this nickName already known in the lobby
 */
bool ChatLobbyDialog::isNicknameInLobby(const RsGxsId& nickname)
{
    ChatLobbyInfo clinfo;

    if(! rsMsgs->getChatLobbyInfo(lobbyId,clinfo))
        return false ;

    return clinfo.gxs_ids.find(nickname) != clinfo.gxs_ids.end() ;
}

/** 
 * Should Messages from this Nickname be muted?
 * 
 * At the moment it is not possible to 100% know which peer sendet the message, and only
 * the nickname is available. So this couldn't work for 100%. So, for example,  if a peer 
 * change his name to the name of a other peer, we couldn't block him. A real implementation 
 * will be possible if we transfer a temporary Session ID from the sending Retroshare client
 * version 0.6
 * 
 * @param QString nickname to check
 */
bool ChatLobbyDialog::isParticipantMuted(const RsGxsId& participant)
{
 	// nickname in Mute list
    return mutedParticipants.find(participant) != mutedParticipants.end();
}

void ChatLobbyDialog::displayLobbyEvent(int event_type, const RsGxsId& gxs_id, const QString& str)
{
    QString qsParticipant="";
    switch (event_type)
    {
    case RS_CHAT_LOBBY_EVENT_PEER_LEFT:
        qsParticipant=str;
        ui.chatWidget->addChatMsg(true, tr("Lobby management"), QDateTime::currentDateTime(), QDateTime::currentDateTime(), tr("%1 has left the lobby.").arg(RsHtml::plainText(str)), ChatWidget::MSGTYPE_SYSTEM);
        emit peerLeft(id()) ;
        break;
    case RS_CHAT_LOBBY_EVENT_PEER_JOINED:
        qsParticipant=str;
        ui.chatWidget->addChatMsg(true, tr("Lobby management"), QDateTime::currentDateTime(), QDateTime::currentDateTime(), tr("%1 joined the lobby.").arg(RsHtml::plainText(str)), ChatWidget::MSGTYPE_SYSTEM);
        emit peerJoined(id()) ;
        break;
    case RS_CHAT_LOBBY_EVENT_PEER_STATUS:
#warning peer name not handled yet here.
        qsParticipant=QString::fromStdString(gxs_id.toStdString());
        ui.chatWidget->updateStatusString(RsHtml::plainText(QString::fromStdString(gxs_id.toStdString())) + " %1", RsHtml::plainText(str));

        if (!isParticipantMuted(gxs_id))
            emit typingEventReceived(id()) ;

        break;
    case RS_CHAT_LOBBY_EVENT_PEER_CHANGE_NICKNAME:
        qsParticipant=str;
        ui.chatWidget->addChatMsg(true, tr("Lobby management"), QDateTime::currentDateTime(),
                              QDateTime::currentDateTime(),
                              tr("%1 changed his name to: %2").arg(RsHtml::plainText(QString::fromStdString(gxs_id.toStdString()))),
                              ChatWidget::MSGTYPE_SYSTEM);

        // TODO if a user was muted and changed his name, update mute list, but only, when the muted peer, dont change his name to a other peer in your chat lobby
        if (isParticipantMuted(gxs_id) && !isNicknameInLobby(RsGxsId(str.toStdString()))) {
            muteParticipant(gxs_id);
        }

        break;
    case RS_CHAT_LOBBY_EVENT_KEEP_ALIVE:
        //std::cerr << "Received keep alive packet from " << nickname.toStdString() << " in lobby " << getPeerId() << std::endl;
        break;
    default:
        std::cerr << "ChatLobbyDialog::displayLobbyEvent() Unhandled lobby event type " << event_type << std::endl;
    }

    if (qsParticipant!=""){
        QList<QTreeWidgetItem*>  qlFoundParticipants=ui.participantsList->findItems(str,Qt::MatchExactly,COLUMN_NAME);
        if (qlFoundParticipants.count()!=0) qlFoundParticipants.at(0)->setText(COLUMN_ACTIVITY,QString::number(time(NULL)));
    }

    updateParticipantsList() ;
}

bool ChatLobbyDialog::canClose()
{
	// check that the lobby still exists.
    /* TODO
	ChatLobbyId lid;
	if (!rsMsgs->isLobbyId(getPeerId(), lid)) {
		return true;
	}
    */

	if (QMessageBox::Yes == QMessageBox::question(this, tr("Unsubscribe to lobby"), tr("Do you want to unsubscribe to this chat lobby?"), QMessageBox::Yes | QMessageBox::No)) {
		return true;
	}

	return false;
}

void ChatLobbyDialog::showDialog(uint chatflags)
{
	if (chatflags & RS_CHAT_FOCUS) 
	{
		MainWindow::showWindow(MainWindow::ChatLobby);
		dynamic_cast<ChatLobbyWidget*>(MainWindow::getPage(MainWindow::ChatLobby))->setCurrentChatPage(this) ;
	}
}

void ChatLobbyDialog::showParticipantsFrame(bool show)
{
	ui.participantsFrame->setVisible(show);
	ui.participantsFrameButton->setChecked(show);

	if (show) {
		ui.participantsFrameButton->setToolTip(tr("Hide Participants"));
		ui.participantsFrameButton->setIcon(QIcon(":images/show_toolbox_frame.png"));
	} else {
		ui.participantsFrameButton->setToolTip(tr("Show Participants"));
		ui.participantsFrameButton->setIcon(QIcon(":images/hide_toolbox_frame.png"));
	}

    PeerSettings->setShowParticipantsFrame(ChatId(lobbyId), show);
}
