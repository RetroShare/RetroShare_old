/****************************************************************
 *
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2011, RetroShare Team
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

#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QCompleter>
#include "gui/common/HashBox.h"
#include "ChatStyle.h"
#include "gui/style/RSStyle.h"

#include <retroshare/rsmsgs.h>
#include <retroshare/rsfiles.h>

class QAction;
class QTextEdit;
class QPushButton;
class ChatWidget;

namespace Ui {
class ChatWidget;
}

// a Container for the logic behind buttons in a PopupChatDialog
// Plugins can implement this interface to provide their own buttons
class ChatWidgetHolder
{
public:
	ChatWidgetHolder(ChatWidget *chatWidget) : mChatWidget(chatWidget) {}
	virtual ~ChatWidgetHolder() {}

	// status comes from notifyPeerStatusChanged
	// see rststaus.h for possible values
	virtual void updateStatus(int /*status*/) {}

protected:
	ChatWidget *mChatWidget;
};

class ChatWidget : public QWidget
{
	Q_OBJECT

public:
	enum MsgType { MSGTYPE_NORMAL, MSGTYPE_HISTORY, MSGTYPE_OFFLINE, MSGTYPE_SYSTEM };
	enum ChatType { CHATTYPE_UNKNOWN, CHATTYPE_PRIVATE, CHATTYPE_LOBBY, CHATTYPE_DISTANT };

	explicit ChatWidget(QWidget *parent = 0);
	~ChatWidget();

    void init(const RsPeerId &peerId, const QString &title);
	ChatType chatType() { return mChatType; }

	bool hasNewMessages() { return newMessages; }
	bool isTyping() { return typing; }

	void focusDialog();
	void addToParent(QWidget *newParent);
	void removeFromParent(QWidget *oldParent);

	void setWelcomeMessage(QString &text);
	void addChatMsg(bool incoming, const QString &name, const QDateTime &sendTime, const QDateTime &recvTime, const QString &message, MsgType chatType);
	void updateStatusString(const QString &statusMask, const QString &statusString);

	void addToolsAction(QAction *action);

    RsPeerId getPeerId() { return peerId; }
	QString getTitle() { return title; }
	int getPeerStatus() { return peerStatus; }
	void setName(const QString &name);

	bool setStyle();
	const RSStyle *getStyle() { return &style; }

	void addChatBarWidget(QWidget *w) ;

	bool isActive();
	void setDefaultExtraFileFlags(TransferRequestFlags f) ;
	void pasteText(const QString&);

	const QList<ChatWidgetHolder*> &chatWidgetHolderList() { return mChatWidgetHolder; }

public slots:
	void updateStatus(const QString &peer_id, int status);

private slots:
	void pasteCreateMsgLink() ;
	void clearChatHistory();
	void deleteChatHistory();
	void messageHistory();
	void resetStatusBar() ;

signals:
	void infoChanged(ChatWidget*);
	void newMessage(ChatWidget*);
	void statusChanged(int);

protected:
	bool eventFilter(QObject *obj, QEvent *event);
	virtual void showEvent(QShowEvent *event);
	virtual void resizeEvent(QResizeEvent *event);
	void updateTitle();

private slots:
	void contextMenuTextBrowser(QPoint);
	void chatCharFormatChanged();

	void fileHashingFinished(QList<HashedFile> hashedFiles);

	void smileyWidget();
	void addSmiley();

	void addExtraFile();
	void addExtraPicture();
	void on_closeInfoFrameButton_clicked();

	void chooseColor();
	void chooseFont();
	void resetFont();
	void setFont();

	void sendChat();

	void updatePeersCustomStateString(const QString& peer_id, const QString& status_string) ;

	bool fileSave();
	bool fileSaveAs();

private:
	void updateStatusTyping();
	void setCurrentFileName(const QString &fileName);

	void colorChanged();
	void fontChanged();
	void setColorAndFont();
	void processSettings(bool load);

	void completeNickname(bool reverse);
    QAbstractItemModel *modelFromPeers();

    RsPeerId peerId;
	QString title;
	QString name;
	QString completionWord;
	int completionPosition;

	QColor currentColor;
	QFont  currentFont;

	QString fileName;

	bool newMessages;
	bool typing;
	int peerStatus;
	ChatType mChatType;

	time_t lastStatusSendTime;

	ChatStyle chatStyle;
	RSStyle style;

	bool firstShow;
	bool inChatCharFormatChanged;

	TransferRequestFlags mDefaultExtraFileFlags ; // flags for extra files shared in this chat. Will be 0 by default, but might be ANONYMOUS for chat lobbies.
	QDate lastMsgDate ;

    QCompleter *completer;

	QList<ChatWidgetHolder*> mChatWidgetHolder;

	Ui::ChatWidget *ui;
};

#endif // CHATWIDGET_H
