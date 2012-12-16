#ifndef RSBWCTRL_WINDOW_H
#define RSBWCTRL_WINDOW_H

/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2012 Robert Fernie
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

#include <QMainWindow>

namespace Ui {
    class BwCtrlWindow;
}

class BwCtrlWindow : public QMainWindow {
    Q_OBJECT
public:

    static void showYourself ();
    static BwCtrlWindow* getInstance();
    static void releaseInstance();


    BwCtrlWindow(QWidget *parent = 0);
    ~BwCtrlWindow();

	void updateBandwidth();

public slots:
	void update();
	
protected:
    void changeEvent(QEvent *e);

private:
    Ui::BwCtrlWindow *ui;

    static BwCtrlWindow *mInstance;

};

#endif // RSBWCTRL_WINDOW_H

