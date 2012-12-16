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

#ifndef _CREATE_BLOG_DIALOG_H
#define _CREATE_BLOG_DIALOG_H

#include "ui_CreateBlog.h"

class CreateBlog : public QDialog
{
  Q_OBJECT

public:
  CreateBlog(QWidget *parent = 0, bool isForum = true);

void  newBlog(); /* cleanup */

  /** Qt Designer generated object */
  Ui::CreateBlog ui;

  QPixmap picture;

public slots:
  /** Overloaded QWidget.show */
  void show();

private slots:

	/* actions to take.... */
void  createBlog();
void  cancelBlog();
void addBlogLogo();


private:


  bool mIsForum;
};

#endif

