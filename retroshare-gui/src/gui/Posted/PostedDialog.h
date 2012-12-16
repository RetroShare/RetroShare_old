/*
 * Retroshare Posted Dialog
 *
 * Copyright 2012-2012 by Robert Fernie.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2.1 as published by the Free Software Foundation.
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

#ifndef MRK_POSTED_DIALOG_H
#define MRK_POSTED_DIALOG_H

#include "retroshare-gui/mainpage.h"
#include "ui_PostedDialog.h"

#include <retroshare/rsposted.h>

#include <map>

//#include "gui/Posted/PostedList.h"
//#include "gui/Posted/PostedComments.h"

//#include "gui/PhotoShare/PhotoAddDialog.h"
//#include "gui/PhotoShare/PhotoSlideShow.h"
//#include "util/TokenQueue.h"

class PostedListDialog;
class PostedComments;

class PostedDialog : public MainPage
{
  Q_OBJECT

public:
	PostedDialog(QWidget *parent = 0);

//virtual void addTab(std::string item);

private slots:

	//void OpenSlideShow();

private:

	PostedListDialog *mPostedList;
	PostedComments *mPostedComments;

	/* UI - from Designer */
	Ui::PostedDialog ui;

};

#endif

