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

#include <QTimer>

#include "BlogMsgItem.h"

#include "FeedHolder.h"
#include "SubFileItem.h"

/****
 * #define DEBUG_ITEM 1
 ****/

/** Constructor */
BlogMsgItem::BlogMsgItem(FeedHolder *parent, uint32_t feedId, std::string peerId, std::string msgId, bool isHome)
:QWidget(NULL), mParent(parent), mFeedId(feedId),
	mPeerId(peerId), mMsgId(msgId), mIsHome(isHome)
{
  /* Invoke the Qt Designer generated object setup routine */
  setupUi(this);

  setAttribute ( Qt::WA_DeleteOnClose, true );

  /* general ones */
  connect( expandButton, SIGNAL( clicked( void ) ), this, SLOT( toggle ( void ) ) );
  connect( clearButton, SIGNAL( clicked( void ) ), this, SLOT( removeItem ( void ) ) );
  //connect( gotoButton, SIGNAL( clicked( void ) ), this, SLOT( gotoHome ( void ) ) );

  /* specific ones */
  connect( playButton, SIGNAL( clicked( void ) ), this, SLOT( playMedia ( void ) ) );

  small();
  updateItemStatic();
  updateItem();
}


void BlogMsgItem::updateItemStatic()
{
	/* fill in */
#ifdef DEBUG_ITEM
	std::cerr << "BlogMsgItem::updateItemStatic()";
	std::cerr << std::endl;
#endif

	msgLabel->setText("What did you expect? \nThe Blogs will be up and running shortly!");
	titleLabel->setText("Blog Feed: Jacki @ Friday");
	subjectLabel->setText("Brand new exciting Blog stuff");

	/* add Files */
	int total = (int) (10.0 * (rand() / (RAND_MAX + 1.0)));
	int i;

	total = 0;

	for(i = 0; i < total; i++)
	{
		/* add file */
		SubFileItem *fi = new SubFileItem("dummyHash", "dummy_File", "", 1283918, SFI_STATE_REMOTE, mPeerId);
		mFileItems.push_back(fi);

		QLayout *layout = expandFrame->layout();
		layout->addWidget(fi);
	}

	playButton->setEnabled(false);
	
	if (mIsHome)
	{
		/* disable buttons */
		clearButton->setEnabled(false);
		//gotoButton->setEnabled(false);
		
		clearButton->hide();
	}
}


void BlogMsgItem::updateItem()
{
	/* fill in */
#ifdef DEBUG_ITEM
	std::cerr << "BlogMsgItem::updateItem()";
	std::cerr << std::endl;
#endif
	int msec_rate = 10000;

	/* Very slow Tick to check when all files are downloaded */
	std::list<SubFileItem *>::iterator it;
	for(it = mFileItems.begin(); it != mFileItems.end(); it++)
	{
		if (!(*it)->done())
		{
			/* loop again */
	  		QTimer::singleShot( msec_rate, this, SLOT(updateItem( void ) ));
			return;
		}
	}
	if (mFileItems.size() > 0)
	{
		playButton->setEnabled(true);
	}
}


void BlogMsgItem::small()
{
	expandFrame->hide();
}

void BlogMsgItem::toggle()
{
	mParent->lockLayout(this, true);

	if (expandFrame->isHidden())
	{
		expandFrame->show();
	    expandButton->setIcon(QIcon(QString(":/images/edit_remove24.png")));
	    expandButton->setToolTip(tr("Hide"));
	}
	else
	{
		expandFrame->hide();
	    expandButton->setIcon(QIcon(QString(":/images/edit_add24.png")));
	    expandButton->setToolTip(tr("Expand"));
	}

	mParent->lockLayout(this, false);
}


void BlogMsgItem::removeItem()
{
#ifdef DEBUG_ITEM
	std::cerr << "BlogMsgItem::removeItem()";
	std::cerr << std::endl;
#endif

	mParent->lockLayout(this, true);
	hide();
	mParent->lockLayout(this, false);

	if (mParent)
	{
		mParent->deleteFeedItem(this, mFeedId);
	}
}


void BlogMsgItem::gotoHome()
{
#ifdef DEBUG_ITEM
	std::cerr << "BlogMsgItem::gotoHome()";
	std::cerr << std::endl;
#endif
}

/*********** SPECIFIC FUNCTIOSN ***********************/


void BlogMsgItem::playMedia()
{
#ifdef DEBUG_ITEM
	std::cerr << "BlogMsgItem::playMedia()";
	std::cerr << std::endl;
#endif
}



