/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2006, crypton
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


#pragma once

#include <QDialog>

#include "ui_PGPKeyDialog.h"
#include <retroshare/rstypes.h>

class PGPKeyDialog : public QDialog
{
    Q_OBJECT

public:
    enum enumPage { PageDetails=0x0 };

    template<class ID_CLASS> static void showIt(const ID_CLASS& id, enumPage page)
    {
        PGPKeyDialog *confdialog = instance(id);

        switch (page) {
        case PageDetails:
            confdialog->ui.stabWidget->setCurrentIndex(0);
            break;
        }

        confdialog->load();
        confdialog->show();
        confdialog->raise();
        confdialog->activateWindow();

        /* window will destroy itself! */
    }
    static void loadAll();

signals:
    void configChanged();

private:
    /** Default constructor */
    PGPKeyDialog(const RsPeerId &id,const RsPgpId& pgp_id, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    /** Default destructor */
    ~PGPKeyDialog();

    static PGPKeyDialog *instance(const RsPgpId &pgp_id);

    void load();

private slots:
    void applyDialog();
    void makeFriend();
    void denyFriend();
    void signGPGKey();
    void loadKeyPage();
    //void setServiceFlags();

    void showHelpDialog();
    /** Called when a child window requests the given help <b>topic</b>. */
    void showHelpDialog(const QString &topic);

private:
    RsPeerId peerId;
    RsPgpId  pgpId;

    /** Qt Designer generated object */
    Ui::PGPKeyDialog ui;
};

