/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2006 - 2009, RetroShare Team
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


#ifndef _PROFILEWIDGET_H
#define _PROFILEWIDGET_H

#include <QWidget>

#include "ui_ProfileWidget.h"

class ProfileWidget : public QWidget
{
    Q_OBJECT

public:
    /** Default constructor */
    ProfileWidget(QWidget *parent = 0, Qt::WFlags flags = 0);

private slots:
    void showEvent ( QShowEvent * event );
    void statusmessagedlg();
    void copyCert();
    void profilemanager();

private:
    /** Qt Designer generated object */
    Ui::ProfileWidget ui;
};

#endif

