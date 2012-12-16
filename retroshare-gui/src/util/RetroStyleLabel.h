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

#ifndef RETROSTYLELABEL_H
#define RETROSTYLELABEL_H

#include <util/rsqtutildll.h>

#include <QtGui/QLabel>
#include <QtGui/QPixmap>
#include <QtGui/QColor>


class RSQTUTIL_API RetroStyleLabel : public QLabel {
	Q_OBJECT
public:

	enum Mode {
		Normal,
		Toggled,
	};

	RetroStyleLabel(QWidget * parent, Mode = Normal, Qt::AlignmentFlag hAlign = Qt::AlignHCenter);

	~RetroStyleLabel();

	void setPixmaps(const QPixmap & normalLeftPixmap,
			const QPixmap & normalRightPixmap,
			const QPixmap & normalFillPixmap,
			const QPixmap & pressedLeftPixmap,
			const QPixmap & pressedRightPixmap,
			const QPixmap & pressedFillPixmap);

	/*void setLeftPixmaps(const QPixmap & normalRightPixmap, const QPixmap & pressedRightPixmap);

	void setRightPixmaps(const QPixmap & normalRightPixmap, const QPixmap & pressedRightPixmap);*/

	void setTextColor(const QColor & color);

	//void setBackgroundColor(const QColor & color);

	void setTextAlignment(int alignment) {
		_alignment = alignment;
	}

	void setSelected(bool value) {
		_selected = value;
	}

public Q_SLOTS:

	void setText(const QString & text);

Q_SIGNALS:

	void clicked();

private:

	void paintEvent(QPaintEvent * event);

	void resizeEvent(QResizeEvent * event);

	void drawText(QPainter * painter);

	void mouseMoveEvent(QMouseEvent * event);

	void mousePressEvent(QMouseEvent * event);

	void mouseReleaseEvent(QMouseEvent * event);

	QPixmap _normalFillPixmap;

	QPixmap _normalLeftPixmap;

	QPixmap _normalRightPixmap;

	QPixmap _pressedFillPixmap;

	QPixmap _pressedLeftPixmap;

	QPixmap _pressedRightPixmap;

	QColor _textColor;

	QColor _backgroundColor;

	bool _pressed;

	bool _selected;

	QWidget * _parent;

	int _alignment;

	bool _toggled;

	Mode _mode;
};

#endif	//RETROSTYLELABEL_H
