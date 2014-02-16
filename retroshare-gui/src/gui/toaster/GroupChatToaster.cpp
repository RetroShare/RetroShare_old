/*
 * RetroShare
 * Copyright (C) 2012 RetroShare Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "GroupChatToaster.h"
#include "gui/FriendsDialog.h"
#include "util/HandleRichText.h"

#include <retroshare/rspeers.h>

GroupChatToaster::GroupChatToaster(const RsPeerId &peerId, const QString &message) : QWidget(NULL)
{
	/* Invoke the Qt Designer generated object setup routine */
	ui.setupUi(this);

	this->peerId = peerId;

	connect(ui.toasterButton, SIGNAL(clicked()), SLOT(chatButtonSlot()));
	connect(ui.closeButton, SIGNAL(clicked()), SLOT(hide()));

	/* set informations */
	ui.textLabel->setText(RsHtml().formatText(NULL, message, RSHTML_FORMATTEXT_EMBED_SMILEYS | RSHTML_FORMATTEXT_EMBED_LINKS | RSHTML_FORMATTEXT_CLEANSTYLE));
	ui.toasterLabel->setText(QString::fromUtf8(rsPeers->getPeerName(peerId).c_str()));
	ui.avatarWidget->setFrameType(AvatarWidget::STATUS_FRAME);
	ui.avatarWidget->setDefaultAvatar(":/images/user/personal64.png");
	ui.avatarWidget->setId(peerId);
}

void GroupChatToaster::chatButtonSlot()
{
	FriendsDialog::groupChatActivate();
	hide();
}
