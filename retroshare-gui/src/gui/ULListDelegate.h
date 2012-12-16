/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2006,2007 crypton
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

#ifndef ULLISTDELEGATE_H
#define ULLISTDELEGATE_H

#include <QAbstractItemDelegate>

// Defines for upload list list columns
#define UNAME        0
#define USIZE        1
#define UTRANSFERRED 2
#define ULSPEED      3
#define UPROGRESS    4
#define USTATUS      5
#define USERNAME     6
#define UHASH        7
#define UUSERID      8



#define MAX_CHAR_TMP 128

class QModelIndex;
class QPainter;
class QStyleOptionProgressBarV2;
class QProgressBar;
class QApplication;


class ULListDelegate: public QAbstractItemDelegate {

	Q_OBJECT

	public:
		ULListDelegate(QObject *parent=0);
		~ULListDelegate();
		void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
		QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

	private:

	public slots:

	signals:
};
#endif

