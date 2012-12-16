/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2006 - 2011, RetroShare Team
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

#ifndef _MESSAGEWIDGET_H
#define _MESSAGEWIDGET_H

#include <QWidget>
#include "ui_MessageWidget.h"

class QToolButton;
class QAction;
class QTextEdit;

class MessageWidget : public QWidget
{
	Q_OBJECT

public:
	enum enumActionType {
		ACTION_REMOVE,
		ACTION_REPLY,
		ACTION_REPLY_ALL,
		ACTION_FORWARD,
		ACTION_PRINT,
		ACTION_PRINT_PREVIEW,
		ACTION_SAVE_AS
	};

public:
	MessageWidget(bool controlled, QWidget *parent = 0, Qt::WFlags flags = 0);
	~MessageWidget();

	static MessageWidget *openMsg(const std::string &msgId, bool window);

    std::string msgId() { return currMsgId; }
	void connectAction(enumActionType actionType, QToolButton* button);
	void connectAction(enumActionType actionType, QAction* action);

	void fill(const std::string &msgId);
	void processSettings(const QString &settingsGroup, bool load);

	QString subject(bool noEmpty);

private slots:
	void reply();
	void replyAll();
	void forward();
	void remove();
	void print();
	void printPreview();
	void saveAs();

	void msgfilelistWidgetCostumPopupMenu(QPoint);
	void messagesTagsChanged();
	void messagesChanged();

	void togglefileview();
	void getcurrentrecommended();
	void getallrecommended();

	void anchorClicked(const QUrl &url);

private:
	void clearTagLabels();
	void showTagLabels();

	bool isControlled;
	bool isWindow;
	std::string currMsgId;
	unsigned int currMsgFlags;

	QList<QLabel*> tagLabels;

	/** Qt Designer generated object */
	Ui::MessageWidget ui;
};

#endif
