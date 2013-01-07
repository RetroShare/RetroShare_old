/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2008 Robert Fernie
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


#ifndef _POSTED_GROUP_DIALOG_H
#define _POSTED_GROUP_DIALOG_H

#include "gui/gxs/GxsGroupDialog.h"
#include "retroshare/rsposted.h"

class PostedGroupDialog : public GxsGroupDialog
{
	Q_OBJECT

public:

    /*!
     * This constructs a create dialog
     */
    PostedGroupDialog(TokenQueue* tokenQueue, RsPosted* posted, QWidget *parent = NULL);

    /*!
     * This constructs a show dialog which displays an already existing group
     */
    PostedGroupDialog(const RsPostedGroup& grp, Mode mode, QWidget *parent = NULL);

protected:
    virtual void initUi();
    virtual QPixmap serviceImage();
    bool service_CreateGroup(uint32_t &token, const RsGroupMetaData &meta);

private:
    RsPostedGroup mGrp;
    RsPosted* mPosted;
};

#endif
